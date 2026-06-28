/*
 * hisi_dp_i2s.c -- ALSA SoC HISI DP I2S DAI driver
 *
 * Copyright (c) 2013 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <sound/core.h>
#include <sound/dmaengine_pcm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>


/*lint -e429*/
#define HISI_DP_MIN_CHANNELS 	 (1)
#define HISI_DP_MAX_CHANNELS	 (8)

#define HISI_DP_PCM_FORMATS		(SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S16_BE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_S24_BE | \
			SNDRV_PCM_FMTBIT_S32_LE | \
			SNDRV_PCM_FMTBIT_S32_BE )

#define HISI_DP_PCM_RATES    (SNDRV_PCM_RATE_8000_192000)

struct hisi_dp_i2s_platform_data {
	void		__iomem *reg_base_addr;
	struct resource	*res;
};

static int hisi_dp_i2s_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params,
			     struct snd_soc_dai *dai)
{
	return 0;
}

static int hisi_dp_i2s_hw_free(struct snd_pcm_substream *substream,
			   struct snd_soc_dai *dai)
{
	return 0;
}

static int hisi_dp_i2s_probe(struct snd_soc_dai *dai)
{
	/* do nothing */
	return 0;
}

static int hisi_dp_i2s_remove(struct snd_soc_dai *dai)
{
	/* do nothing */
	return 0;
}

static const struct snd_soc_dai_ops hisi_dp_i2s_ops = {
	.hw_params	= hisi_dp_i2s_hw_params,
	.hw_free	= hisi_dp_i2s_hw_free,
};

static struct snd_soc_dai_driver hisi_dp_i2s_dai = {
	.probe = hisi_dp_i2s_probe,
	.remove = hisi_dp_i2s_remove,
	.playback = {
		.channels_min = HISI_DP_MIN_CHANNELS,
		.channels_max = HISI_DP_MAX_CHANNELS,
		.rates = HISI_DP_PCM_RATES,
		.formats = HISI_DP_PCM_FORMATS,
	},
	.ops = &hisi_dp_i2s_ops,
};

static const struct snd_soc_component_driver hisi_dp_i2s_component = {
	.name		= "hisi-dp-i2s",
};

static const struct of_device_id hisi_dp_i2s_of_match[] = {
	{
		.compatible = "hisilicon,hisi-dp-i2s",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, hisi_dp_i2s_of_match);

static int dp_i2s_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hisi_dp_i2s_platform_data *pdata = NULL;

	pr_info("[%s:%d], dp_i2s_probe!\n", __func__, __LINE__);

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (NULL == pdata) {
		dev_err(dev, "%s, cannot allocate dp i2s platform data\n", __func__);
		return -ENOMEM;
	}

	/* rename device name */
	dev_set_name(dev, "hisi-dp-i2s");
	platform_set_drvdata(pdev, pdata);

	return snd_soc_register_component(&pdev->dev, &hisi_dp_i2s_component,
					  &hisi_dp_i2s_dai, 1);
}

static int dp_i2s_remove(struct platform_device *pdev)
{
	pr_info("[%s:%d], dp_i2s_remove in\n", __func__, __LINE__);

	snd_soc_unregister_component(&pdev->dev);

	return 0;
}

static struct platform_driver hisi_dp_i2s_driver = {
	.driver = {
		.name	= "hisi-dp-i2s",
		.owner	= THIS_MODULE,
		.of_match_table = hisi_dp_i2s_of_match,
	},
	.probe	= dp_i2s_probe,
	.remove	= dp_i2s_remove,
};
module_platform_driver(hisi_dp_i2s_driver);

MODULE_AUTHOR("wushengyang <wushengyang1@hisilicon.com>");
MODULE_DESCRIPTION("HISI DP I2S");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:hisi_dp_i2s");
