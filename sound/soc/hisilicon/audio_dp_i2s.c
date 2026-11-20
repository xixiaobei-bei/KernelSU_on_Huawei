/*
 * audio_dp_i2s.c
 *
 * alsa soc audio dp i2s dai driver
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

#include <linux/module.h>
#include <linux/device.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <linux/hisi/audio_log.h>

#define LOG_TAG "audio_dp_i2s"

#define AUDIO_DP_MIN_CHANNELS 1
#define AUDIO_DP_MAX_CHANNELS 8

#define AUDIO_DP_PCM_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
	SNDRV_PCM_FMTBIT_S16_BE | \
	SNDRV_PCM_FMTBIT_S24_LE | \
	SNDRV_PCM_FMTBIT_S24_BE | \
	SNDRV_PCM_FMTBIT_S32_LE | \
	SNDRV_PCM_FMTBIT_S32_BE)

#define AUDIO_DP_PCM_RATES (SNDRV_PCM_RATE_8000_192000)

struct audio_dp_i2s_platform_data {
	void __iomem *reg_base_addr;
	struct resource *res;
};

static int audio_dp_i2s_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	return 0;
}

static int audio_dp_i2s_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static int audio_dp_i2s_probe(struct snd_soc_dai *dai)
{
	return 0;
}

static int audio_dp_i2s_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static const struct snd_soc_dai_ops g_audio_dp_i2s_ops = {
	.hw_params = audio_dp_i2s_hw_params,
	.hw_free = audio_dp_i2s_hw_free,
};

static struct snd_soc_dai_driver g_audio_dp_i2s_dai = {
	.probe = audio_dp_i2s_probe,
	.remove = audio_dp_i2s_remove,
	.playback = {
		.channels_min = AUDIO_DP_MIN_CHANNELS,
		.channels_max = AUDIO_DP_MAX_CHANNELS,
		.rates = AUDIO_DP_PCM_RATES,
		.formats = AUDIO_DP_PCM_FORMATS,
	},
	.ops = &g_audio_dp_i2s_ops,
};

static const struct snd_soc_component_driver g_audio_dp_i2s_component = {
	.name = "audio-dp-i2s",
};

static const struct of_device_id g_audio_dp_i2s_of_match[] = {
	{
		.compatible = "hisilicon,audio-dp-i2s",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, g_audio_dp_i2s_of_match);

static int dp_i2s_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct audio_dp_i2s_platform_data *pdata = NULL;

	AUDIO_LOGI("in");

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		AUDIO_LOGE("cannot allocate dp i2s platform data");
		return -ENOMEM;
	}

	/* rename device name */
	dev_set_name(dev, "audio-dp-i2s");
	platform_set_drvdata(pdev, pdata);

	AUDIO_LOGI("out");

	return devm_snd_soc_register_component(&pdev->dev, &g_audio_dp_i2s_component,
					  &g_audio_dp_i2s_dai, 1);
}

static int dp_i2s_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver g_audio_dp_i2s_driver = {
	.driver = {
		.name = "audio-dp-i2s",
		.owner = THIS_MODULE,
		.of_match_table = g_audio_dp_i2s_of_match,
	},
	.probe = dp_i2s_probe,
	.remove = dp_i2s_remove,
};
module_platform_driver(g_audio_dp_i2s_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("AUDIO DP I2S");
