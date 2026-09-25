/*
 * hi6555_codec.c -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "hi6555_codec.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/pm_runtime.h>
#include <sound/pcm.h>

#include <linux/mfd/hisi_pmic.h>
#include "hi6xxx_mbhc.h"
#include "linux/hisi/audio_log.h"
#include "hi6555_type.h"
#include "hi6555_utils.h"
#include "hi6555_store.h"

#define LOG_TAG "hi6555_codec"

#define PB_MIN_CHANNELS 2
#define PB_MAX_CHANNELS 2
#define CP_MIN_CHANNELS 1
#define CP_MAX_CHANNELS 6
#define FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)
#define RATES SNDRV_PCM_RATE_8000_384000

static int hi6555_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static int hi6555_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	return 0;
}

struct snd_soc_dai_ops hi6555_dai_ops = {
	.startup    = hi6555_startup,
	.hw_params  = hi6555_hw_params,
};

struct snd_soc_dai_driver hi6555_dai[] = {
	{
		.name = "hi6555c-dai",
		.playback = {
			.stream_name = "Playback",
			.channels_min = PB_MIN_CHANNELS,
			.channels_max = PB_MAX_CHANNELS,
			.rates = RATES,
			.formats = FORMATS
		},
		.capture = {
			.stream_name = "Capture",
			.channels_min = CP_MIN_CHANNELS,
			.channels_max = CP_MAX_CHANNELS,
			.rates = RATES,
			.formats = FORMATS
		},
		.ops = &hi6555_dai_ops,
	},
};

static struct platform_data hi6555_data;

static int hi6555_codec_probe(struct snd_soc_codec *codec)
{
	int ret;
	struct platform_data *priv = &hi6555_data;
	struct snd_soc_dapm_context *dapm = snd_soc_codec_get_dapm(codec);

	AUDIO_LOGI("Begin");
	if (priv->ops.codec_init == NULL || priv->ops.codec_deinit == NULL) {
		AUDIO_LOGE("invalid codec callbacks, codec_init: 0x%pK, codec_deinit: 0x%pK",
			priv->ops.codec_init, priv->ops.codec_deinit);
		ret = -EINVAL;
		goto end;
	}

	snd_soc_codec_set_drvdata(codec, priv);

	ret = priv->ops.codec_init(codec);
	if (ret) {
		AUDIO_LOGE("codec init err, ret = %d", ret);
		goto end;
	}

	ret = hi6xxx_mbhc_init(codec, &priv->mbhc, priv->ops.mbhc_ops);
	if (ret) {
		AUDIO_LOGE("hi6xxx_mbhc_init err, ret = %d", ret);
		goto mbhc_init_err;
	}

	ret = snd_soc_dapm_sync(dapm);
	if (ret) {
		AUDIO_LOGE("dapm sync error, errornum = %d", ret);
		goto dapm_sync_err;
	}
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_init(codec, hi6555_reg_get_dump_info());
#endif

	ret = hi6555_store_subscribe_path(codec, priv->path_name, priv->path_name_num);
	if (ret) {
		AUDIO_LOGE("subscribe path resource fail, ret: %d", ret);
		goto subscribe_path_err;
	}
	goto end;

subscribe_path_err:
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif

dapm_sync_err:
	hi6xxx_mbhc_deinit(priv->mbhc);

mbhc_init_err:
	priv->ops.codec_deinit(codec);
end:
	AUDIO_LOGI("End");

	return ret;
}

static int hi6555_codec_remove(struct snd_soc_codec *codec)
{
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif

	hi6xxx_mbhc_deinit(priv->mbhc);
	priv->ops.codec_deinit(codec);

	OUT_FUNCTION;

	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_hi6555 = {
	.probe    = hi6555_codec_probe,
	.remove  = hi6555_codec_remove,
	.read      = hi6555_reg_read,
	.write     = hi6555_reg_write,
	.idle_bias_off = true
};

static int hi6555_probe(struct platform_device *pdev)
{
	int ret;

	AUDIO_LOGI("Begin");

	ret = hi6555_reg_addr_map(pdev);
	if (ret) {
		AUDIO_LOGE("hi6555_reg_addr_map failed! err code %d", ret);
		hi6555_reg_addr_unmap();
		return ret;
	}

	ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_hi6555, hi6555_dai,
		ARRAY_SIZE(hi6555_dai));
	if (ret) {
		AUDIO_LOGE("snd_soc_register_codec failed! err code %d", ret);
		hi6555_reg_addr_unmap();
	}

	AUDIO_LOGI("End");

	return ret;
}

static int hi6555_remove(struct platform_device *pdev)
{
	AUDIO_LOGI("Begin");

	snd_soc_unregister_codec(&pdev->dev);
	hi6555_reg_addr_unmap();
	AUDIO_LOGI("End");

	return 0;
}

static const struct of_device_id hi6555_codec_match[] = {
	{ .compatible = "hisilicon,hi6555-codec", },
	{},
};

#ifdef CONFIG_PM
int hi6555_runtime_suspend(struct device *dev)
{
	int ret;
	struct platform_data *priv = NULL;

	AUDIO_LOGI("begin");

	if (dev == NULL) {
		AUDIO_LOGE("device is null");
		return -EINVAL;
	}

	priv = dev_get_drvdata(dev);

	if (priv == NULL) {
		AUDIO_LOGE("get drvdata failed");
		return -EINVAL;
	}

	if (priv->board_type == FPGA_BOARD)
		return 0;

	ret = pinctrl_select_state(priv->resource.pin.pctrl, priv->resource.pin.pin_idle);
	if (ret) {
		AUDIO_LOGE("could not set pins to idle state, ret = %d", ret);
		return ret;
	}

	hi6555_asp_clk_disable(priv->codec);

	AUDIO_LOGI("end");

	return ret;
}

int hi6555_runtime_resume(struct device *dev)
{
	int ret;
	struct platform_data *priv = NULL;

	AUDIO_LOGI("begin");

	if (dev == NULL) {
		AUDIO_LOGE("device is null");
		return -EINVAL;
	}

	priv = dev_get_drvdata(dev);

	if (priv->board_type == FPGA_BOARD)
		return 0;

	ret = hi6555_asp_clk_enable(priv->codec);
	if (ret) {
		AUDIO_LOGE("could not enable asp resource, ret = %d", ret);
		return ret;
	}

	ret = pinctrl_select_state(priv->resource.pin.pctrl, priv->resource.pin.pin_default);
	if (ret) {
		AUDIO_LOGE("could not set pins to default state, ret = %d", ret);
		return ret;
	}

	AUDIO_LOGI("end");

	return ret;
}
#endif

#ifdef CONFIG_PM_SLEEP
static int hi6555_suspend(struct device *dev)
{
	int ret;
	struct platform_data *priv = NULL;

	AUDIO_LOGI("begin");

	if (dev == NULL) {
		AUDIO_LOGE("device is null");
		return -EINVAL;
	}

	priv = dev_get_drvdata(dev);
	if (priv == NULL) {
		AUDIO_LOGE("get drvdata failed");
		return -EINVAL;
	}

	if (priv->board_type == FPGA_BOARD)
		return 0;

	if (priv->pm.pm_runtime_support) {
		ret = pm_runtime_get_sync(dev);
		AUDIO_LOGD("pm suspend ret: %d", ret);
		if (ret < 0) {
			AUDIO_LOGE("pm suspend error, ret: %d", ret);
			return ret;
		}
	}

	AUDIO_LOGD("suspend usage_count: 0x%x status: 0x%x disable_depth: %d asp_subsys_clk: %d, asp_codec_clk: %d",
			atomic_read(&(dev->power.usage_count)),
			dev->power.runtime_status,
			dev->power.disable_depth,
			clk_get_enable_count(priv->resource.asp_subsys_clk),
			clk_get_enable_count(priv->resource.asp_codec_clk));


	if (!priv->pm.have_dapm) {
		/* set pin to low power mode */
		ret = pinctrl_select_state(priv->resource.pin.pctrl, priv->resource.pin.pin_idle);
		if (ret) {
			AUDIO_LOGE("could not set pins to idle state, ret: %d", ret);
			return ret;
		}

		hi6555_asp_resource_disable(priv->codec);
		priv->pm.asp_pd = true;

		AUDIO_LOGI("suspend without dapm");
	}

	AUDIO_LOGI("end");

	return 0;
}

static int hi6555_resume(struct device *dev)
{
	int ret;
	struct platform_data *priv = NULL;

	AUDIO_LOGI("begin");

	if (dev == NULL) {
		AUDIO_LOGE("device is null");
		return -EINVAL;
	}

	priv = dev_get_drvdata(dev);

	if (priv == NULL) {
		AUDIO_LOGE("get drvdata failed");
		return -EINVAL;
	}

	if (priv->board_type == FPGA_BOARD)
		return 0;

	if (priv->pm.asp_pd) {
		ret = hi6555_asp_resource_enable(priv->codec);
		if (ret) {
			AUDIO_LOGE("asp resource enable error, ret: %d", ret);
			return ret;
		}

		priv->pm.asp_pd = false;
		priv->ops.pm_asp_reg_init(priv->codec);
		ret = pinctrl_select_state(priv->resource.pin.pctrl, priv->resource.pin.pin_default);
		if (ret) {
			AUDIO_LOGE("could not set pins to default state, ret: %d", ret);
			return ret;
		}
		AUDIO_LOGI("resume from asp pd");
	}

	if (priv->pm.pm_runtime_support) {
		pm_runtime_mark_last_busy(dev);
		pm_runtime_put_autosuspend(dev);

		pm_runtime_disable(dev);
		pm_runtime_set_active(dev);
		pm_runtime_enable(dev);
	}

	AUDIO_LOGD("resume usage_count: 0x%x status: 0x%x disable_depth: %d asp_subsys_clk: %d, asp_codec_clk: %d",
			atomic_read(&(dev->power.usage_count)),
			dev->power.runtime_status,
			dev->power.disable_depth,
			clk_get_enable_count(priv->resource.asp_subsys_clk),
			clk_get_enable_count(priv->resource.asp_codec_clk));

	AUDIO_LOGI("resume end");

	return 0;
}
#endif

const struct dev_pm_ops hi6555_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(hi6555_suspend, hi6555_resume)
	SET_RUNTIME_PM_OPS(hi6555_runtime_suspend, hi6555_runtime_resume, NULL)
};

static struct platform_driver hi6555_codec_driver = {
	.driver = {
		.name  = "hi6555-codec",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hi6555_codec_match),
		.pm = &hi6555_pm_ops,
	},
	.probe  = hi6555_probe,
	.remove = hi6555_remove,
};

void hi6555_codec_driver_init(const struct codec_ops *ops)
{
	if (ops == NULL) {
		AUDIO_LOGE("priv data is null");
		return;
	}

	memcpy(&hi6555_data.ops, ops, sizeof(hi6555_data.ops));
	hi6555_codec_driver.shutdown = ops->shutdown;
}

static int __init hi6555_codec_init(void)
{
	AUDIO_LOGI("Begin");

	return platform_driver_register(&hi6555_codec_driver);
}
module_init(hi6555_codec_init);

static void __exit hi6555_codec_exit(void)
{
	AUDIO_LOGI("Begin");
	platform_driver_unregister(&hi6555_codec_driver);
}
module_exit(hi6555_codec_exit);

MODULE_DESCRIPTION("ASoC hi6555x serise driver");
MODULE_AUTHOR("xieliyin <xieliyin@huawei.com>");
MODULE_LICENSE("GPL");

