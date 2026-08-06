/*
 * DP audio codec driver.
 *
 * Copyright (c) 2015 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <sound/pcm.h>
#include <sound/soc.h>

#define DP_AUDIO_MIN_CHANNELS ( 1 )
#define DP_AUDIO_MAX_CHANNELS ( 8 )
#define DP_AUDIO_FORMATS    	( SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S16_BE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_S24_BE | \
			SNDRV_PCM_FMTBIT_S32_LE | \
			SNDRV_PCM_FMTBIT_S32_BE )

#define DP_AUDIO_RATES    (SNDRV_PCM_RATE_8000_192000)

static int dp_audio_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	return 0;
}

static int dp_audio_hw_free(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	return 0;
}

struct snd_soc_dai_ops dp_audio_dai_ops = {
	.hw_params  = dp_audio_hw_params,
	.hw_free = dp_audio_hw_free,
};

struct snd_soc_dai_driver dp_audio_dai[] = {
	{
		.name = "dp-audio-dai",
		.playback = {
			.stream_name    = "dp-playback",
			.channels_min   = DP_AUDIO_MIN_CHANNELS,
			.channels_max   = DP_AUDIO_MAX_CHANNELS,
			.rates          = DP_AUDIO_RATES,
			.formats        = DP_AUDIO_FORMATS,
		},
		.ops = &dp_audio_dai_ops,
	},
};

static const struct of_device_id dp_audio_of_match[] = {
	{
		.compatible = "hisilicon,dp-audio",
	},
	{ },
};

MODULE_DEVICE_TABLE(of, dp_audio_of_match);

static const struct snd_soc_component_driver dp_audio_component = {
	.name		= "dp-audio",
};

static int dp_audio_codec_probe(struct snd_soc_codec *codec)
{
	return 0;
}

static int dp_audio_codec_remove(struct snd_soc_codec *codec)
{
	return 0;
}

static struct snd_soc_codec_driver dp_audio_dev = {
	.probe		= dp_audio_codec_probe,
	.remove		= dp_audio_codec_remove,
};

static int dp_audio_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;

	pr_info("[%s:%d], dp_audio_probe \n", __FUNCTION__, __LINE__);

	dev_set_name(dev, "dp-audio");
	ret = snd_soc_register_codec(dev, &dp_audio_dev,
			dp_audio_dai, ARRAY_SIZE(dp_audio_dai));
	if (0 != ret) {
		pr_err("[%s:%d]: snd_soc_register_component failed\n", __FUNCTION__, __LINE__);
	}

	return ret;
}

static int dp_audio_remove(struct platform_device *pdev)
{
	pr_info("[%s:%d], dp_audio_remove \n", __FUNCTION__, __LINE__);

	snd_soc_unregister_codec(&pdev->dev);

	return 0;
}

static struct platform_driver dp_audio_driver = {
	.driver = {
		.name  = "dp-audio",
		.owner = THIS_MODULE,
		.of_match_table = dp_audio_of_match,
	},
	.probe  = dp_audio_probe,
	.remove = dp_audio_remove,
};

module_platform_driver(dp_audio_driver);

MODULE_DESCRIPTION("ASoC dp audio driver");
MODULE_AUTHOR("wushengyang <wushengyang1@hisilicon.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("codec:dp_audio");
