/*
 * audio_pcm_dp.c
 *
 * alsa soc audio pcm dp driver
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/hisi/audio_log.h>

#include "audio_pcm_dp.h"
#include "asp_hdmi_dma.h"
#include "asp_cfg.h"
#include "dsp_misc.h"
#include "drv_mailbox_platform.h"

#define LOG_TAG "audio_pcm_dp"
#define AUDIO_DP_PCM_NAME "audio-pcm-dp"

#define DSP_DP_SAMPLE_RATE_NUM  13
#define PERIOD_BYTES_MIN        32
#define PERIOD_BYTES_MAX        (256 * 1024)
#define PERIODS_MIN             2
#define PERIODS_MAX             PERIOD_BYTES_MIN
#define BUFFER_BYTES_MAX        PERIOD_BYTES_MAX
#define SUPPORT_MONO            1
#define SUPPORT_STEREO          2
#define SUPPORT_8CH             8
#define STREAM_COUNT            2

static DEFINE_SPINLOCK(g_substream_spinlock);
static struct snd_pcm_substream *g_audio_pcm_dp_substream;
static unsigned long long g_audio_pcm_dmamask = DMA_BIT_MASK(32);

static const unsigned int g_freq[DSP_DP_SAMPLE_RATE_NUM][2] = {
	{ 8000, SAMPLE_RATE_NO_SUPPORT },
	{ 11025, SAMPLE_RATE_NO_SUPPORT },
	{ 12000, SAMPLE_RATE_NO_SUPPORT },
	{ 16000, SAMPLE_RATE_NO_SUPPORT },
	{ 22050, SAMPLE_RATE_NO_SUPPORT },
	{ 24000, SAMPLE_RATE_NO_SUPPORT },
	{ 32000, SAMPLE_RATE_NO_SUPPORT },
	{ 44100, SAMPLE_RATE_NO_SUPPORT },
	{ 48000, SAMPLE_RATE_48 },
	{ 88200, SAMPLE_RATE_NO_SUPPORT },
	{ 96000, SAMPLE_RATE_96 },
	{ 176400, SAMPLE_RATE_NO_SUPPORT },
	{ 192000, SAMPLE_RATE_192 }
};

static const unsigned int g_div_clk[SAMPLE_RATE_MAX][2] = {
	{ SAMPLE_RATE_32, AUDIO_DIV_CLK_NO_SUPPORT },
	{ SAMPLE_RATE_44, AUDIO_DIV_CLK_NO_SUPPORT },
	{ SAMPLE_RATE_48, AUDIO_DIV_CLK_48 },
	{ SAMPLE_RATE_88, AUDIO_DIV_CLK_NO_SUPPORT },
	{ SAMPLE_RATE_96, AUDIO_DIV_CLK_96 },
	{ SAMPLE_RATE_176, AUDIO_DIV_CLK_NO_SUPPORT },
	{ SAMPLE_RATE_192, AUDIO_DIV_CLK_192 }
};

static struct snd_pcm_hardware g_audio_pcm_dp_hardware = {
	.info = SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_INTERLEAVED,
	.formats = SNDRV_PCM_FMTBIT_S16_LE |
		SNDRV_PCM_FMTBIT_S16_BE |
		SNDRV_PCM_FMTBIT_S24_LE |
		SNDRV_PCM_FMTBIT_S24_BE |
		SNDRV_PCM_FMTBIT_S32_LE |
		SNDRV_PCM_FMTBIT_S32_BE,
	.period_bytes_min = PERIOD_BYTES_MIN,
	.period_bytes_max = PERIOD_BYTES_MAX,
	.periods_min = PERIODS_MIN,
	.periods_max = PERIODS_MAX,
	.buffer_bytes_max = BUFFER_BYTES_MAX,
};

static const struct of_device_id g_audio_pcm_dp_match[] = {
	{
		.compatible = "hisilicon,audio-pcm-dp",
	},
	{},
};

MODULE_DEVICE_TABLE(of, g_audio_pcm_dp_match);

static int set_tx3_parameters(struct snd_pcm_hw_params *params,
	struct tx3_config_parameters *tx3_parameters)
{
	unsigned int params_value;

	/* check support channels : mono or stereo or 8ch */
	params_value = params_channels(params);
	AUDIO_LOGI("set channel num: %u", params_value);
	switch (params_value) {
	case SUPPORT_MONO:
		tx3_parameters->channel_num = CHANNEL_NUM_1;
		break;
	case SUPPORT_STEREO:
		tx3_parameters->channel_num = CHANNEL_NUM_2;
		break;
	case SUPPORT_8CH:
		tx3_parameters->channel_num = CHANNEL_NUM_8;
		break;
	default:
		AUDIO_LOGE("DAC not support %u channels", params_value);
		return -EINVAL;
	}

	params_value = params_format(params);
	AUDIO_LOGI("set format: %u", params_value);
	switch (params_value) {
	case SNDRV_PCM_FORMAT_S16_LE:
		tx3_parameters->bit_width  = BIT_WIDTH_16;
		tx3_parameters->align_type = ALIGN_16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		tx3_parameters->bit_width  = BIT_WIDTH_24;
		tx3_parameters->align_type = ALIGN_32;
		break;
	/* use align_32 to avoid Channel interchange problem */
	case SNDRV_PCM_FORMAT_S32_LE:
		tx3_parameters->bit_width  = BIT_WIDTH_16;
		tx3_parameters->align_type = ALIGN_32;
		break;
	default:
		AUDIO_LOGE("format err: %u", params_value);
		return -EINVAL;
	}

	return 0;
}

static int set_sio_parameters(struct snd_pcm_hw_params *params,
	struct sio_config_parameters *sio_parameters)
{
	unsigned int params_value;
	unsigned int infreq_index;
	unsigned int clk_index;

	params_value = params_rate(params);
	AUDIO_LOGI("set rate: %u", params_value);

	for (infreq_index = 0; infreq_index < DSP_DP_SAMPLE_RATE_NUM; infreq_index++) {
		if (params_value == g_freq[infreq_index][0])
			break;
	}
	if (infreq_index >= DSP_DP_SAMPLE_RATE_NUM) {
		AUDIO_LOGE("set rate: %u error", params_value);
		return -EINVAL;
	}

	sio_parameters->sample_rate = g_freq[infreq_index][1];
	AUDIO_LOGI("sample rate[%u]: %d", sio_parameters->sample_rate, g_freq[infreq_index][0]);

	for (clk_index = 0; clk_index < SAMPLE_RATE_MAX; clk_index++) {
		if (sio_parameters->sample_rate == g_div_clk[clk_index][0])
			break;
	}
	if (clk_index >= SAMPLE_RATE_MAX) {
		AUDIO_LOGE("set clk: %u error", sio_parameters->sample_rate);
		return -EINVAL;
	}

	sio_parameters->div_clk = g_div_clk[clk_index][1];
	AUDIO_LOGI("div clk: 0x%x", sio_parameters->div_clk);

	return 0;
}

static int dp_clk_enable(struct audio_dp_data *prtd)
{
	struct clk *dp_audio_pll_clk = NULL;
	struct clk *asp_subsys_clk = NULL;
	int ret;

	if (!prtd) {
		AUDIO_LOGE("prtd is null pointer");
		return -EINVAL;
	}

	asp_subsys_clk = prtd->asp_subsys_clk;
	if (asp_subsys_clk) {
		ret = clk_prepare_enable(asp_subsys_clk);
		if (ret != 0) {
			AUDIO_LOGE("asp subsys clk enable fail, error: %d", ret);
			return -EFAULT;
		}
	} else {
		AUDIO_LOGE("asp subsys clk is null");
	}

	dp_audio_pll_clk = prtd->dp_audio_pll_clk;
	if (dp_audio_pll_clk) {
		ret = clk_set_rate(dp_audio_pll_clk, AUDIO_PLL6_RATE_48);
		AUDIO_LOGI("set pll6 rate: %d", AUDIO_PLL6_RATE_48);
		if (ret != 0) {
			AUDIO_LOGE("set pll6 rate failed. ret: %d", ret);
			if (asp_subsys_clk)
				clk_disable_unprepare(asp_subsys_clk);

			return ret;
		}

		ret = clk_prepare_enable(dp_audio_pll_clk);
		if (ret != 0) {
			AUDIO_LOGE("clk dp audio pll enable fail, error: %d", ret);
			if (asp_subsys_clk)
				clk_disable_unprepare(asp_subsys_clk);

			return -EINVAL;
		}
	} else {
		AUDIO_LOGE("dp audio pll clk is null");
	}

	return 0;
}

static int dp_clk_disable(struct audio_dp_data *prtd)
{
	struct clk *dp_audio_pll_clk = NULL;
	struct clk *asp_subsys_clk = NULL;
	int ret;

	if (!prtd) {
		AUDIO_LOGE("prtd is null pointer");
		return -EINVAL;
	}

	dp_audio_pll_clk = prtd->dp_audio_pll_clk;
	if (dp_audio_pll_clk) {
		ret = clk_set_rate(dp_audio_pll_clk, AUDIO_PLL6_RATE_DEFAULT);
		if (ret != 0)
			AUDIO_LOGE("set pll6 rate to default value failed. ret: %d", ret);
		clk_disable_unprepare(dp_audio_pll_clk);
	}

	asp_subsys_clk = prtd->asp_subsys_clk;
	if (asp_subsys_clk)
		clk_disable_unprepare(asp_subsys_clk);

	return 0;
}

static bool check_irq_status(const struct audio_dp_runtime_data *prtd,
	unsigned int irs)
{
	if (prtd->status == STATUS_HDMI_STOP) {
		AUDIO_LOGD("stop dma, irs: %#x", irs);
		return true;
	}

	if (((1 << HDMI_DMA_A) & irs) != 0 || ((1 << HDMI_DMA_B) & irs) != 0)
		return false;

	AUDIO_LOGE("irq error, irs: %#x", irs);

	return true;
}

static void time_interval_monitor(struct audio_dp_data *pdata)
{
	unsigned int cur_time;
	unsigned int delay_time;

	cur_time = (unsigned int)mailbox_get_timestamp();
	delay_time = cur_time - pdata->pre_time;
	pdata->pre_time = cur_time;
	if (delay_time > (AUDIO_WORK_DELAY_1MS * 30))
		AUDIO_LOGE("dp irp timeout: %u", delay_time);
}

static void playback_hw_available_monitor(struct snd_pcm_runtime *runtime)
{
	unsigned int rt_period_size;
	snd_pcm_uframes_t avail;

	rt_period_size = runtime->period_size;

	avail = snd_pcm_playback_hw_avail(runtime);
	if (avail < rt_period_size)
		AUDIO_LOGI("there is no avail data avail[%d] rt period size[%u]",
			(int)avail, rt_period_size);
}

static irqreturn_t dp_dma_irq_handler(int irq, void *data)
{
	struct snd_pcm *pcm = data;
	struct snd_pcm_substream *substream = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	struct audio_dp_runtime_data *prtd = NULL;
	struct audio_dp_data *pdata = NULL;
	unsigned int num_periods;
	unsigned int irs;
	uint32_t asp_int;

	asp_int = asp_cfg_get_irq_value();
	if (!(asp_int & 0x1))
		return IRQ_HANDLED;

	if (!pcm) {
		AUDIO_LOGE("pcm is null");
		return IRQ_HANDLED;
	}

	substream = pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream;
	if (!substream) {
		AUDIO_LOGE("substream is null");
		return IRQ_HANDLED;
	}

	runtime = substream->runtime;
	prtd = runtime->private_data;
	pdata = prtd->pdata;
	num_periods = runtime->periods;

	time_interval_monitor(pdata);

	irs = asp_hdmi_reg_read_irsr();
	if (irs == 0) {
		AUDIO_LOGW("not hdmi dma irq");
		return IRQ_HANDLED;
	}

	asp_hdmi_dma_clear_interrupt(irs);

	if (check_irq_status(prtd, irs))
		return IRQ_HANDLED;

	spin_lock(&prtd->lock);
	prtd->period_cur = (prtd->period_cur + 1) % num_periods;
	spin_unlock(&prtd->lock);

	snd_pcm_period_elapsed(substream);

	spin_lock(&prtd->lock);

	playback_hw_available_monitor(runtime);

	asp_hdmi_dma_enable();

	spin_unlock(&prtd->lock);

	return IRQ_HANDLED;
}

int stop_pcm_dp_substream(void)
{
	struct snd_pcm_substream *substream = NULL;
	int ret = 0;

	substream = g_audio_pcm_dp_substream;
	if (substream != NULL) {
		ret = snd_pcm_stop_xrun(substream);
		AUDIO_LOGI("pcm dp substream stop: %d", ret);
	}

	return ret;
}

static void init_private_data(struct snd_pcm_substream *substream,
	struct audio_dp_data *pdata, struct audio_dp_runtime_data *prtd)
{
	mutex_init(&prtd->mutex);
	spin_lock_init(&prtd->lock);

	WARN_ON(!pdata);

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_get_sync(pdata->dev);
#endif

	prtd->pdata = pdata;

	substream->runtime->private_data = prtd;
}

static int enable_regulator(struct audio_dp_data *pdata)
{
	int ret;

	ret = regulator_bulk_enable(1, &pdata->regu);
	if (ret != 0) {
		AUDIO_LOGE("couldn't enable regulators, ret: %d", ret);
		return ret;
	}

	return ret;
}
static void disable_regulator(struct audio_dp_runtime_data *prtd)
{
	if (regulator_bulk_disable(1, &prtd->pdata->regu))
		AUDIO_LOGE("fail to disable regulator");
}

static void deinit_private_data(struct audio_dp_runtime_data *prtd)
{
#ifdef CONFIG_PM_RUNTIME
	pm_runtime_mark_last_busy(prtd->pdata->dev);
	pm_runtime_put_autosuspend(prtd->pdata->dev);
#endif
	mutex_destroy(&prtd->mutex);
}

static void enable_hdmi(void)
{
	asp_cfg_hdmi_module_enable();

	/* selete clk_audio_pll */
	asp_cfg_hdmi_clk_sel(ASP_CFG_HDMI_CLK_SEL_I2S);

	asp_cfg_enable_hdmi_interrupeter();
}

static void disable_hdmi(void)
{
	asp_cfg_disable_hdmi_interrupeter();
	asp_cfg_hdmi_module_disable();
}

static int audio_pcm_dp_open(struct snd_pcm_substream *substream)
{
	struct audio_dp_runtime_data *prtd = NULL;
	struct snd_soc_pcm_runtime *rtd = NULL;
	struct audio_dp_data *pdata = NULL;
	struct snd_soc_component *component = NULL;
	struct snd_pcm *pcm = substream->pcm;
	unsigned long flags;
	int ret;

	WARN_ON(!pcm);

	AUDIO_LOGI("begin");

	prtd = kzalloc(sizeof(*prtd), GFP_KERNEL);
	if (!prtd) {
		AUDIO_LOGE("kzalloc audio dp runtime data error");
		return -ENOMEM;
	}

	rtd = (struct snd_soc_pcm_runtime *)substream->private_data;
	component = snd_soc_rtdcom_lookup(rtd, AUDIO_DP_PCM_NAME);
	if (!component) {
		AUDIO_LOGE("componet is null");
		ret = -ENOMEM;
		goto err_rtdcom_lookup;
	}

	pdata = (struct audio_dp_data *)snd_soc_component_get_drvdata(component);
	init_private_data(substream, pdata, prtd);

	ret = enable_regulator(pdata);
	if (ret != 0)
		goto err_enable_regulator;

	ret = request_irq(pdata->irq, dp_dma_irq_handler,
		IRQF_TRIGGER_HIGH | IRQF_SHARED | IRQF_NO_SUSPEND, "asp_hdmi_dma", pcm);
	if (ret != 0) {
		AUDIO_LOGE("request asp dma irq failed");
		goto err_irq;
	}

	ret = dp_clk_enable(pdata);
	if (ret != 0) {
		AUDIO_LOGE("dp clk enable failed");
		goto err_clk;
	}

	enable_hdmi();

	ret = snd_soc_set_runtime_hwparams(substream, &g_audio_pcm_dp_hardware);
	if (ret != 0) {
		AUDIO_LOGE("set hwparams failed");
		goto err_set_hw_param;
	}

	spin_lock_irqsave(&g_substream_spinlock, flags);
	g_audio_pcm_dp_substream = substream;
	spin_unlock_irqrestore(&g_substream_spinlock, flags);

	return ret;

err_set_hw_param:
	disable_hdmi();
	dp_clk_disable(prtd->pdata);
err_clk:
	free_irq(pdata->irq, pcm);
err_irq:
	disable_regulator(prtd);
	deinit_private_data(prtd);
err_enable_regulator:
err_rtdcom_lookup:
	kfree(prtd);

	return ret;
}

static int audio_pcm_dp_close(struct snd_pcm_substream *substream)
{
	struct audio_dp_runtime_data *prtd = substream->runtime->private_data;
	unsigned long flags;

	AUDIO_LOGI("begin");

	if (!prtd) {
		AUDIO_LOGE("prtd is null");
		return -ENOMEM;
	}

	if (!prtd->pdata) {
		AUDIO_LOGE("pdata is null");
		return -ENOMEM;
	}

	free_irq(prtd->pdata->irq, substream->pcm);

	asp_cfg_disable_hdmi_interrupeter();

	asp_hdmi_tx3_disable();

	asp_cfg_hdmi_module_disable();

	if (dp_clk_disable(prtd->pdata))
		AUDIO_LOGE("dp clk disable error");

	disable_regulator(prtd);

	deinit_private_data(prtd);

	spin_lock_irqsave(&g_substream_spinlock, flags);
	kfree(prtd);
	substream->runtime->private_data = NULL;
	g_audio_pcm_dp_substream = NULL;
	spin_unlock_irqrestore(&g_substream_spinlock, flags);

	return 0;
}

static int audio_pcm_dp_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	int ret;
	unsigned long bytes = params_buffer_bytes(params);
	struct audio_dp_runtime_data *prtd = substream->runtime->private_data;
	struct tx3_config_parameters tx3_parameters = { CHANNEL_NUM_2, BIT_WIDTH_16, ALIGN_16 };
	struct sio_config_parameters sio_parameters = {
		&tx3_parameters, SAMPLE_RATE_48, AUDIO_DIV_CLK_48, false
	};

	if (!prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	AUDIO_LOGI("begin");

	if (substream->stream != SNDRV_PCM_STREAM_PLAYBACK)
		return 0;
	ret = snd_pcm_lib_malloc_pages(substream, bytes);
	if (ret < 0) {
		AUDIO_LOGE("malloc pages failed. ret: %d", ret);
		return ret;
	}

	ret = set_tx3_parameters(params, &tx3_parameters);
	if (ret != 0) {
		AUDIO_LOGE("set tx3 parameters failed. ret: %d", ret);
		goto err_out;
	}

	ret = set_sio_parameters(params, &sio_parameters);
	if (ret != 0) {
		AUDIO_LOGE("set sio parameters failed. ret: %d", ret);
		goto err_out;
	}

	if (sio_parameters.div_clk != AUDIO_DIV_CLK_NO_SUPPORT)
		asp_cfg_div_clk(sio_parameters.div_clk);
	else
		AUDIO_LOGE("div clk: %u is no support,and will use defaulte div_clk config",
			sio_parameters.div_clk);

	mutex_lock(&prtd->mutex);
	prtd->period_size = params_period_bytes(params);
	mutex_unlock(&prtd->mutex);

	ret = asp_hdmi_tx3_config(tx3_parameters);
	if (ret != 0) {
		AUDIO_LOGE("tx3 config failed. ret: %d", ret);
		goto err_out;
	}

	asp_hdmi_io_config(sio_parameters);

	return 0;
err_out:
	AUDIO_LOGE("hw params error, ret: %d", ret);
	snd_pcm_lib_free_pages(substream);

	return ret;
}

static int audio_pcm_dp_hw_free(struct snd_pcm_substream *substream)
{
	struct audio_dp_runtime_data *prtd = substream->runtime->private_data;
	int ret = 0;

	if (!prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		asp_hdmi_dma_clear_interrupt(HDMI_IRQ_MASK);

		ret = snd_pcm_lib_free_pages(substream);
	}

	return ret;
}

static int audio_pcm_dp_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct audio_dp_runtime_data *prtd = substream->runtime->private_data;

	if (!prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}
	AUDIO_LOGI("mode: %d", substream->stream);

	spin_lock(&prtd->lock);
	prtd->status = STATUS_HDMI_STOP;
	prtd->period_next = 0;
	prtd->period_cur = 0;
	prtd->dma_addr = (unsigned int)((unsigned long)runtime->dma_addr);
	spin_unlock(&prtd->lock);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* config dma ch_a ch_b */
		asp_hdmi_dma_config(prtd->dma_addr, prtd->period_size);

		asp_hdmi_dma_clear_interrupt(HDMI_IRQ_MASK);
	}

	return 0;
}

static int audio_pcm_dp_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct audio_dp_runtime_data *prtd = substream->runtime->private_data;
	unsigned long flags;
	int ret = 0;

	if (!prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	AUDIO_LOGI("mode %d trigger %d", substream->stream, cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			spin_lock_irqsave(&g_substream_spinlock, flags);
			prtd->status = STATUS_HDMI_RUNNING;
			prtd->pdata->pre_time = (unsigned int)mailbox_get_timestamp();
			asp_cfg_dp_module_enable();
			asp_hdmi_tx3_enable();
			asp_hdmi_dma_start();
			spin_unlock_irqrestore(&g_substream_spinlock, flags);
		}
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			spin_lock_irqsave(&g_substream_spinlock, flags);
			prtd->status = STATUS_HDMI_STOP;
			asp_hdmi_dma_stop();
			asp_hdmi_tx3_disable();
			asp_cfg_dp_module_disable();
			spin_unlock_irqrestore(&g_substream_spinlock, flags);
		}
		break;

	default:
		AUDIO_LOGE("cmd is error and trigger cmd: %d", cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static snd_pcm_uframes_t audio_pcm_dp_pointer(struct snd_pcm_substream *substream)
{
	snd_pcm_uframes_t frame;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct audio_dp_runtime_data *prtd = substream->runtime->private_data;

	unsigned int period_cur;
	unsigned int period_size;

	if (!prtd) {
		AUDIO_LOGE("prtd is null");
		return 0;
	}

	spin_lock(&prtd->lock);
	period_cur = prtd->period_cur;
	period_size = prtd->period_size;
	spin_unlock(&prtd->lock);

	frame = bytes_to_frames(runtime, period_cur * period_size);
	if (frame >= runtime->buffer_size)
		frame = 0;

	return frame;
}

static int audio_dp_normal_mmap(struct snd_pcm_substream *substream,
	struct vm_area_struct *vma)
{
	int ret;
	struct snd_pcm_runtime *runtime = NULL;

	runtime = substream->runtime;
	if (!runtime) {
		AUDIO_LOGE("runtime is invalid");
		return -ENOMEM;
	}

	ret = dma_mmap_writecombine(substream->pcm->card->dev,
		vma, runtime->dma_area, runtime->dma_addr, runtime->dma_bytes);

	return ret;
}

int audio_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = NULL;
	size_t size = g_audio_pcm_dp_hardware.buffer_bytes_max;

	if (!substream) {
		AUDIO_LOGE("substream is null");
		return  -ENOMEM;
	}

	buf = &substream->dma_buffer;

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->addr = AUDIO_DP_DATA_LOCATION;

	buf->area = ioremap_wc(buf->addr, size);

	if (!buf->area) {
		AUDIO_LOGE("dma alloc combine error");
		return -ENOMEM;
	}

	buf->bytes = size;

	return 0;
}

static void audio_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream = NULL;
	struct snd_dma_buffer *buf = NULL;
	int stream;

	for (stream = 0; stream < STREAM_COUNT; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		iounmap(buf->area);

		buf->area = NULL;
		buf->addr = 0;
	}
}

static struct snd_pcm_ops g_audio_pcm_dp_ops = {
	.open = audio_pcm_dp_open,
	.close = audio_pcm_dp_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = audio_pcm_dp_hw_params,
	.hw_free = audio_pcm_dp_hw_free,
	.prepare = audio_pcm_dp_prepare,
	.trigger = audio_pcm_dp_trigger,
	.pointer = audio_pcm_dp_pointer,
	.mmap = audio_dp_normal_mmap,
};

static int audio_pcm_dp_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	int ret = 0;

	if (!pcm) {
		AUDIO_LOGE("pcm is null");
		return -EINVAL;
	}

	if (!card || !card->dev) {
		AUDIO_LOGE("pcm runtime has no card or card->dev is null");
		return -ENOENT;
	}

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &g_audio_pcm_dmamask;

	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = audio_pcm_preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_PLAYBACK);
		if (ret != 0) {
			AUDIO_LOGE("preallocate dma buffer error, error No.: %d", ret);
			return ret;
		}
	}

	return ret;
}

static void audio_pcm_dp_free(struct snd_pcm *pcm)
{
	if (!pcm) {
		AUDIO_LOGE("pcm is null");
		return;
	}

	audio_pcm_free_dma_buffers(pcm);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static struct snd_soc_component_driver g_audio_pcm_dp_platform = {
	.name       = AUDIO_DP_PCM_NAME,
	.ops = &g_audio_pcm_dp_ops,
	.pcm_new = audio_pcm_dp_new,
	.pcm_free = audio_pcm_dp_free,
};
#else
static struct snd_soc_platform_driver g_audio_pcm_dp_platform = {
	.ops = &g_audio_pcm_dp_ops,
	.pcm_new = audio_pcm_dp_new,
	.pcm_free = audio_pcm_dp_free,
	.component_driver.name = AUDIO_DP_PCM_NAME,
};
#endif

static int get_dp_cfg(struct platform_device *pdev,
	struct device *dev, struct audio_dp_data *pdata)
{
	int ret;

	pdata->regu.supply = "dp-pcm";
	ret = devm_regulator_bulk_get(dev, 1, &(pdata->regu));
	if (ret != 0) {
		AUDIO_LOGE("couldn't get regulators, ret: %d", ret);
		return -ENOMEM;
	}

	pdata->dev = dev;

	pdata->dp_audio_pll_clk = devm_clk_get(dev, "clk_dp_audio_pll");
	if (IS_ERR_OR_NULL(pdata->dp_audio_pll_clk)) {
		AUDIO_LOGE("devm clk get: dp audio pll clk not found");
		return -EINVAL;
	}

	pdata->asp_subsys_clk = devm_clk_get(dev, "clk_asp_subsys");
	if (IS_ERR_OR_NULL(pdata->asp_subsys_clk)) {
		AUDIO_LOGE("devm clk get: clk asp subsys not found");
		return -EFAULT;
	}

	pdata->irq = platform_get_irq_byname(pdev, "asp_hdmi_dma");
	if (pdata->irq < 0) {
		AUDIO_LOGE("cannot get irq");
		return -ENOENT;
	}

	return ret;
}

static int audio_pcm_dp_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct audio_dp_data *pdata = NULL;
	struct device_node *node = pdev->dev.of_node;

	if (!dev) {
		AUDIO_LOGE("platform device has no device");
		return -ENOENT;
	}

	AUDIO_LOGI("probe begin");

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		AUDIO_LOGE("cannot allocate audio pcm dp platform data");
		return -ENOMEM;
	}

	ret = get_dp_cfg(pdev, dev, pdata);
	if (ret != 0) {
		AUDIO_LOGE("audio dp data get fail");
		return ret;
	}

	if (of_property_read_bool(node, "no_support_mmap"))
		g_audio_pcm_dp_hardware.info = SNDRV_PCM_INFO_INTERLEAVED;

	AUDIO_LOGI("pcm dp hardware info: %u", g_audio_pcm_dp_hardware.info);

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_set_autosuspend_delay(dev, 100); /* 100ms */
	pm_runtime_use_autosuspend(dev);

	pm_runtime_enable(dev);
#endif

	platform_set_drvdata(pdev, pdata);

	dev_set_name(dev, AUDIO_DP_PCM_NAME);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	ret = devm_snd_soc_register_component(dev, &g_audio_pcm_dp_platform, NULL, 0);
#else
	ret = devm_snd_soc_register_platform(dev, &g_audio_pcm_dp_platform);
#endif
	if (ret != 0) {
		AUDIO_LOGE("snd soc register platform return: %d", ret);
		goto reg_platform_failed;
	}

	AUDIO_LOGI("probe end");

	return 0;

reg_platform_failed:
#ifdef CONFIG_PM_RUNTIME
	pm_runtime_disable(&pdev->dev);
#endif

	return ret;
}

static int audio_pcm_dp_remove(struct platform_device *pdev)
{
#ifdef CONFIG_PM_RUNTIME
	pm_runtime_disable(&pdev->dev);
#endif

	return 0;
}

#ifdef CONFIG_PM_RUNTIME
int audio_dp_runtime_suspend(struct device *dev)
{
	struct audio_dp_data *pdata = dev_get_drvdata(dev);
	int ret;

	WARN_ON(!pdata);

	AUDIO_LOGI("begin");

	ret = regulator_bulk_disable(1, &pdata->regu);
	if (ret != 0)
		AUDIO_LOGE("fail to disable regulator, ret: %d", ret);

	AUDIO_LOGI("end");

	return 0;
}

int audio_dp_runtime_resume(struct device *dev)
{
	struct audio_dp_data *pdata = dev_get_drvdata(dev);
	int ret;

	WARN_ON(!pdata);

	AUDIO_LOGI("begin");

	ret = regulator_bulk_enable(1, &pdata->regu);
	if (ret != 0)
		AUDIO_LOGE("couldn't enable regulators,ret: %d", ret);

	AUDIO_LOGI("end");

	return ret;
}
#endif

static bool check_hdmi_status(void)
{
	struct audio_dp_runtime_data *prtd = NULL;

	if (g_audio_pcm_dp_substream == NULL ||
		g_audio_pcm_dp_substream->runtime == NULL ||
		g_audio_pcm_dp_substream->runtime->private_data == NULL) {
		AUDIO_LOGW("params is NULL");
		return false;
	}

	prtd = g_audio_pcm_dp_substream->runtime->private_data;
	if (prtd->status == STATUS_HDMI_STOP) {
		AUDIO_LOGI("hdmi status is stop");
		return false;
	}

	return true;
}

static int audio_dp_suspend(struct device *dev)
{
	unsigned long flags;
	bool ret = false;

	AUDIO_LOGI("begin");

	spin_lock_irqsave(&g_substream_spinlock, flags);
	ret = check_hdmi_status();
	if (!ret) {
		spin_unlock_irqrestore(&g_substream_spinlock, flags);
		AUDIO_LOGI("not need suspend hdmi");
		return 0;
	}

	asp_hdmi_dma_stop();
	asp_hdmi_dma_clear_interrupt(HDMI_IRQ_MASK);
	asp_hdmi_tx3_disable();
	asp_cfg_dp_module_disable();
	spin_unlock_irqrestore(&g_substream_spinlock, flags);

	AUDIO_LOGI("end");
	return 0;
}

static int audio_dp_resume(struct device *dev)
{
	struct audio_dp_runtime_data *prtd = NULL;
	unsigned long flags;
	bool ret = false;

	AUDIO_LOGI("begin");

	spin_lock_irqsave(&g_substream_spinlock, flags);
	ret = check_hdmi_status();
	if (!ret) {
		spin_unlock_irqrestore(&g_substream_spinlock, flags);
		AUDIO_LOGI("not need resume hdmi");
		return 0;
	}

	prtd = g_audio_pcm_dp_substream->runtime->private_data;
	prtd->period_cur = 0;

	asp_hdmi_dma_clear_interrupt(HDMI_IRQ_MASK);
	asp_cfg_dp_module_enable();
	asp_hdmi_tx3_enable();
	asp_hdmi_dma_start();
	spin_unlock_irqrestore(&g_substream_spinlock, flags);

	AUDIO_LOGI("end");
	return 0;
}

const struct dev_pm_ops g_audio_dp_pm_ops = {
	.suspend = audio_dp_suspend,
	.resume = audio_dp_resume,
#ifdef CONFIG_PM_RUNTIME
	.runtime_suspend = audio_dp_runtime_suspend,
	.runtime_resume = audio_dp_runtime_resume,
#endif
};

static struct platform_driver g_audio_pcm_dp_driver = {
	.probe = audio_pcm_dp_probe,
	.remove = audio_pcm_dp_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = AUDIO_DP_PCM_NAME,
		.of_match_table = of_match_ptr(g_audio_pcm_dp_match),
		.pm = &g_audio_dp_pm_ops,
	},
};

module_platform_driver(g_audio_pcm_dp_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ASoC audio pcm dp driver");
