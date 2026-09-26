/*
 * slimbus_dai.c
 *
 * slimbus dai
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "audio_log.h"
#include "slimbus_types.h"

#define LOG_TAG "slimbus_dai"

/* pcm devices */
#define SLIMBUS_DEV_AUDIO           0
#define SLIMBUS_DEV_DIRECT_AUDIO    1
#define SLIMBUS_DEV_FAST_AUDIO      2
#define SLIMBUS_DEV_VOICE           3
#define SLIMBUS_DEV_BT              4
#define SLIMBUS_DEV_MODEM           5
#define SLIMBUS_DEV_DSP_PB          6
#define PLAYBACK_CHANNELS_MIN       1
#define PLAYBACK_CHANNELS_MAX       4
#define CAPTURE_CHANNELS_MIN        1
#define CAPTURE_CHANNELS_MAX        13

struct slimbus_dai_data {
	struct slimbus_track_param hw_params;
	enum slimbus_track_type track;
	struct mutex lock;
	int playback_count;
	int capture_count;
};

static int slimbus_dai_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static void slimbus_dai_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
}

static int slimbus_dai_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	return 0;
}

static int slimbus_dai_trigger(struct snd_pcm_substream *substream, int cmd,
	struct snd_soc_dai *dai)
{
	return 0;
}

static int slimbus_dai_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static const struct snd_soc_dai_ops g_slimbus_dai_ops = {
	.startup = slimbus_dai_startup,
	.shutdown = slimbus_dai_shutdown,
	.hw_params = slimbus_dai_hw_params,
	.trigger = slimbus_dai_trigger,
	.hw_free = slimbus_dai_hw_free,
};

static int slimbus_dai_probe(struct snd_soc_dai *dai)
{
	return 0;
}

static int slimbus_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}


static struct snd_soc_dai_driver g_slimbus_dai = {
	.probe = slimbus_dai_probe,
	.remove = slimbus_dai_remove,
	.playback = {
		.channels_min = PLAYBACK_CHANNELS_MIN,
		.channels_max = PLAYBACK_CHANNELS_MAX,
		.rates = SNDRV_PCM_RATE_8000_384000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S16_BE |
			SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S24_BE,
	},
	.capture = {
		.channels_min = CAPTURE_CHANNELS_MIN,
		.channels_max = CAPTURE_CHANNELS_MAX,
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
	.ops = &g_slimbus_dai_ops,
};

static const struct snd_soc_component_driver g_slimbus_dai_component = {
	.name = "slimbus-dai",
};

static int slimbus_dai_module_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct slimbus_dai_data *pdata = NULL;
	int ret;

	AUDIO_LOGI("slimbus dai probe");
	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		AUDIO_LOGE("cannot allocate platform data");
		return -ENOMEM;
	}

	mutex_init(&pdata->lock);

	platform_set_drvdata(pdev, pdata);

	dev_set_name(dev, "slimbus-dai");

	AUDIO_LOGI("slimbus %s", dev_name(dev));
	ret = devm_snd_soc_register_component(&pdev->dev, &g_slimbus_dai_component,
		&g_slimbus_dai, 1);

	return ret;
}

static int slimbus_dai_module_remove(struct platform_device *pdev)
{
	struct slimbus_dai_data *pdata = platform_get_drvdata(pdev);

	mutex_destroy(&pdata->lock);

	return 0;
}

static const struct of_device_id g_slimbus_dai_of_match[] = {
	{ .compatible = "hisilicon,slimbus-dai", },
	{ },
};

static struct platform_driver g_slimbus_dai_driver = {
	.driver = {
		.name = "slimbus-dai",
		.owner = THIS_MODULE,
		.of_match_table = g_slimbus_dai_of_match,
	},
	.probe = slimbus_dai_module_probe,
	.remove = slimbus_dai_module_remove,
};

static int __init slimbus_dai_module_init(void)
{
	platform_driver_register(&g_slimbus_dai_driver);

	return 0;
}

static void __exit slimbus_dai_module_exit(void)
{
	platform_driver_unregister(&g_slimbus_dai_driver);
}

late_initcall(slimbus_dai_module_init);
module_exit(slimbus_dai_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("SLIMBUS DAI");
