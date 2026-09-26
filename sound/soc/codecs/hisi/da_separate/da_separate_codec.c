/*
 * da_separate_codec.c -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "da_separate_codec.h"

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
#include <linux/version.h>

#include <linux/mfd/hisi_pmic.h>
#include "da_separate_mbhc.h"
#include "linux/hisi/audio_log.h"
#include "da_separate_type.h"
#include "da_separate_utils.h"
#include "da_separate_store.h"

#define DA_SEPARATE_CODEC_NAME "da_separate-codec"

#define LOG_TAG "DA_separate"

#define PB_MIN_CHANNELS 2
#define PB_MAX_CHANNELS 2
#define CP_MIN_CHANNELS 1
#define CP_MAX_CHANNELS 6
#define FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)
#define RATES SNDRV_PCM_RATE_8000_384000

static int da_separate_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static int da_separate_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	return 0;
}

struct snd_soc_dai_ops da_separate_dai_ops = {
	.startup    = da_separate_startup,
	.hw_params  = da_separate_hw_params,
};

struct snd_soc_dai_driver da_separate_dai[] = {
	{
		.name = "da_separate-dai",
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
		.ops = &da_separate_dai_ops,
	},
};

static struct platform_data da_separate_data;

static int da_separate_codec_probe(struct snd_soc_component *codec)
{
	int ret;
	struct platform_data *priv = &da_separate_data;
	struct snd_soc_dapm_context *dapm = snd_soc_component_get_dapm(codec);

	AUDIO_LOGI("Begin");
	if (priv->ops.codec_init == NULL || priv->ops.codec_deinit == NULL) {
		AUDIO_LOGE("invalid callbacks, init: 0x%pK, deinit: 0x%pK",
			priv->ops.codec_init, priv->ops.codec_deinit);
		ret = -EINVAL;
		goto end;
	}

	snd_soc_component_set_drvdata(codec, priv);

	ret = priv->ops.codec_init(codec);
	if (ret) {
		AUDIO_LOGE("init err, ret = %d", ret);
		goto end;
	}

	ret = da_separate_mbhc_init(codec, &priv->mbhc, priv->ops.mbhc_ops);
	if (ret) {
		AUDIO_LOGE("mbhc init err, ret: %d", ret);
		goto mbhc_init_err;
	}

	ret = snd_soc_dapm_sync(dapm);
	if (ret) {
		AUDIO_LOGE("dapm sync error, errornum: %d", ret);
		goto dapm_sync_err;
	}
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_init(codec, da_separate_reg_get_dump_info());
#endif

	ret = da_separate_store_subscribe_path(codec, priv->path_name, priv->path_name_num);
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
	da_separate_mbhc_deinit(priv->mbhc);

mbhc_init_err:
	priv->ops.codec_deinit(codec);
end:
	AUDIO_LOGI("End");

	return ret;
}

static void da_separate_codec_remove(struct snd_soc_component *codec)
{
	struct platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif

	da_separate_mbhc_deinit(priv->mbhc);
	priv->ops.codec_deinit(codec);

	OUT_FUNCTION;

	return;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static struct snd_soc_component_driver soc_codec_dev_da_separate = {
	.name     = DA_SEPARATE_CODEC_NAME,
	.probe    = da_separate_codec_probe,
	.remove  = da_separate_codec_remove,
	.read      = da_separate_reg_read,
	.write     = da_separate_reg_write,
	.idle_bias_off = true
};
#else
static struct snd_soc_codec_driver soc_codec_dev_da_separate = {
	.read      = da_separate_reg_read_by_codec,
	.write     = da_separate_reg_write_by_codec,
	.component_driver.name  = DA_SEPARATE_CODEC_NAME,
	.component_driver.probe = da_separate_codec_probe,
	.component_driver.remove = da_separate_codec_remove,
	.idle_bias_off = true
};
#endif

static int da_separate_probe(struct platform_device *pdev)
{
	int ret;

	AUDIO_LOGI("Begin");

	ret = da_separate_reg_addr_map(pdev);
	if (ret) {
		AUDIO_LOGE("reg addr map failed! err code %d", ret);
		da_separate_reg_addr_unmap();
		return ret;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	ret = devm_snd_soc_register_component(&pdev->dev, &soc_codec_dev_da_separate, da_separate_dai,
		ARRAY_SIZE(da_separate_dai));
#else
	ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_da_separate, da_separate_dai,
		ARRAY_SIZE(da_separate_dai));
#endif
	if (ret) {
		AUDIO_LOGE("register failed! err code %d", ret);
		da_separate_reg_addr_unmap();
	}

	AUDIO_LOGI("End");

	return ret;
}

static int da_separate_remove(struct platform_device *pdev)
{
	AUDIO_LOGI("Begin");

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
	snd_soc_unregister_codec(&pdev->dev);
#endif
	da_separate_reg_addr_unmap();

	AUDIO_LOGI("End");

	return 0;
}

static const struct of_device_id da_separate_codec_match[] = {
	{ .compatible = "hisilicon,da_separate-codec", },
	{},
};

#ifdef CONFIG_PM
int da_separate_runtime_suspend(struct device *dev)
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

	da_separate_asp_clk_disable(priv->codec);

	AUDIO_LOGI("end");

	return ret;
}

int da_separate_runtime_resume(struct device *dev)
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

	ret = da_separate_asp_clk_enable(priv->codec);
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
static int da_separate_suspend(struct device *dev)
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

		da_separate_asp_resource_disable(priv->codec);
		priv->pm.asp_pd = true;

		AUDIO_LOGI("suspend without dapm");
	}

	AUDIO_LOGI("end");

	return 0;
}

static int da_separate_resume(struct device *dev)
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
		ret = da_separate_asp_resource_enable(priv->codec);
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

const struct dev_pm_ops da_separate_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(da_separate_suspend, da_separate_resume)
	SET_RUNTIME_PM_OPS(da_separate_runtime_suspend, da_separate_runtime_resume, NULL)
};

static struct platform_driver da_separate_codec_driver = {
	.driver = {
		.name  = "da_separate-codec",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(da_separate_codec_match),
		.pm = &da_separate_pm_ops,
	},
	.probe  = da_separate_probe,
	.remove = da_separate_remove,
};

void da_separate_codec_driver_init(const struct codec_ops *ops)
{
	if (ops == NULL) {
		AUDIO_LOGE("priv data is null");
		return;
	}

	memcpy(&da_separate_data.ops, ops, sizeof(da_separate_data.ops));
	da_separate_codec_driver.shutdown = ops->shutdown;
}

static int __init da_separate_codec_init(void)
{
	AUDIO_LOGI("Begin");

	return platform_driver_register(&da_separate_codec_driver);
}
module_init(da_separate_codec_init);

static void __exit da_separate_codec_exit(void)
{
	AUDIO_LOGI("Begin");
	platform_driver_unregister(&da_separate_codec_driver);
}
module_exit(da_separate_codec_exit);

MODULE_DESCRIPTION("ASoC da separate serise driver");
MODULE_LICENSE("GPL");

