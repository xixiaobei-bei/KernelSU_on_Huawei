/*
 * linux/sound/soc/hisilicon/hisi-dp-card.c
 *
 * Copyright (C) 2015 Linaro, Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 */

#include <linux/module.h>
#include <sound/pcm.h>
#include <sound/soc.h>

static struct snd_soc_dai_link hisi_dp_dai_link[] = {
	{
		 /* dai link name*/
		.name = "hisi-dp-dai-link",
		/* stream name same as name */
		.stream_name = "dp",
		/* cpu  dai name(device name), see in hisi_dp_i2s.c */
		.cpu_dai_name = "hisi-dp-i2s",
		 /* codec(dp audio) device name ,see in dp-audio.c */
		.codec_name = "dp-audio",
		/* codec dai name, see in struct snd_soc_dai_driver in dp-audio.c */
		.codec_dai_name = "dp-audio-dai",
		/* platform  device name, see in hisi_pcm_dp.c */
		.platform_name = "hisi-pcm-dp",
	},
};

/* Audio dp machine driver */
static struct snd_soc_card snd_soc_hisi_dp = {
	.name = "hisi-dp",
	.owner = THIS_MODULE,
	.dai_link = hisi_dp_dai_link,
	.num_links = ARRAY_SIZE(hisi_dp_dai_link),
};

static int hisi_dp_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_hisi_dp;
	int ret = 0;

	if (NULL == pdev) {
		pr_err("%s : enter with null pointer, fail!\n", __func__);
		return -EINVAL;
	}

	pr_info("Audio: hisi_dp_probe\n");

	card->dev = &pdev->dev;

	ret = snd_soc_register_card(card);
	if (ret) {
		pr_err("%s: snd_soc_register_card failed (%d)\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int hisi_dp_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	pr_info("Audio: hisi_dp_remove\n");

	if (NULL != card)
		snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id hisi_dp_of_match[] = {
	{ .compatible = "hisilicon,hisi-dp-audio-machine" },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, hisi_dp_dt_ids);

static struct platform_driver hisi_dp_driver = {
	.driver = {
		.name = "hisi-dp-audio",
		.owner = THIS_MODULE,
		.of_match_table = hisi_dp_of_match,
	},
	.probe = hisi_dp_probe,
	.remove = hisi_dp_remove,
};

static int __init hisi_dp_init(void)
{
	pr_info("Audio : hisi_dp init\n");
	return platform_driver_register(&hisi_dp_driver);
}
late_initcall(hisi_dp_init);

static void __exit hisi_dp_exit(void)
{
	pr_info("Audio : hisi_dp exit\n");
	platform_driver_unregister(&hisi_dp_driver);
}
module_exit(hisi_dp_exit);

/* Module information */
MODULE_AUTHOR("wushengyang <wushengyang1@hisilicon.com>");
MODULE_DESCRIPTION("Hisilicon DP machine ASoC driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("machine driver:hisi-dp-audio");
