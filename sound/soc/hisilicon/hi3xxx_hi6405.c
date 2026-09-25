/*
 * hi3xxx_hi6405 is the machine driver of alsa which is used to registe sound card
 * Copyright (C) 2014	Hisilicon

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>

/*lint -e785*/
static struct snd_soc_dai_link hi3xxx_hi6405_dai_link[] = {
	{
		.name = "hi3xxx_hi6405_pb_normal",
		.stream_name = "hi3xxx_hi6405_pb_normal",
		.codec_name = "hi6405-codec",
		.cpu_dai_name = "slimbus-dai",
		.codec_dai_name = "hi6405-audio-dai",
		.platform_name = "hi6210-hifi",
	},
	{
		.name = "hi3xxx_voice",
		.stream_name = "hi3xxx_voice",
		.codec_name = "hi6405-codec",
		.cpu_dai_name = "slimbus-dai",
		.codec_dai_name = "hi6405-voice-dai",
		.platform_name = "snd-soc-dummy",
	},
	{
		.name = "hi3xxx_fm1",
		.stream_name = "hi3xxx_fm1",
		.codec_name = "hi6405-codec",
		.cpu_dai_name = "slimbus-dai",
		.codec_dai_name = "hi6405-fm-dai",
		.platform_name = "snd-soc-dummy",
	},
	{
		.name = "hi3xxx_hi6405_pb_dsp",
		.stream_name = "hi3xxx_hi6405_pb_dsp",
		.codec_name = "hi6405-codec",
		.cpu_dai_name = "slimbus-dai",
		.codec_dai_name = "hi6405-audio-dai",
		.platform_name = "hi6210-hifi",
	},
	{
		.name = "hi3xxx_hi6405_pb_direct",
		.stream_name = "hi3xxx_hi6405_pb_direct",
		.codec_name = "hi6405-codec",
		.cpu_dai_name = "slimbus-dai",
		.codec_dai_name = "hi6405-audio-dai",
		.platform_name = "hi6210-hifi",
	},
	{
		.name = "hi3xxx_hi6405_lowlatency",
		.stream_name = "hi3xxx_hi6405_lowlatency",
		.codec_name = "hi6405-codec",
		.cpu_dai_name = "slimbus-dai",
		.codec_dai_name = "hi6405-audio-dai",
#ifdef AUDIO_LOW_LATENCY_LEGACY
		.platform_name = "hi3xxx-pcm-asp-dma",
#else
		.platform_name = "hi6210-hifi",
#endif
	},
	{
		.name = "hi3xxx_hi6405_mmap",
		.stream_name = "hi3xxx_hi6405_mmap",
		.codec_name = "hi6405-codec",
		.cpu_dai_name = "slimbus-dai",
		.codec_dai_name = "hi6405-audio-dai",
		.platform_name = "hi6210-hifi",
	},
};

static struct snd_soc_card hi3xxx_hi6405_card = {
	.name = "hi3xxx_hi6405_card",
	.owner = THIS_MODULE,
	.dai_link = hi3xxx_hi6405_dai_link,
	.num_links = ARRAY_SIZE(hi3xxx_hi6405_dai_link),
};

static int hi3xxx_hi6405_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct snd_soc_card *card = &hi3xxx_hi6405_card;

	if (NULL == pdev) {
		pr_err("%s : pdev is null, fail\n", __FUNCTION__);
		return -EINVAL;
	}

	pr_info("Audio : hi3xxx-hi6405 probe\n");

	card->dev = &pdev->dev;

	ret = snd_soc_register_card(card);
	if (ret) {
		pr_err("%s : sound card register failed %d\n", __FUNCTION__, ret);
	}

	return ret;
}

static int hi3xxx_hi6405_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	if (NULL != card)
		snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id hi3xxx_hi6405_of_match[] = {
	{.compatible = "hisilicon,hi3xxx-hi6405", },
	{ },
};
MODULE_DEVICE_TABLE(of, hi3xxx_hi6405_of_match);

static struct platform_driver hi3xxx_hi6405_driver = {
	.driver	= {
		.name = "hi3xxx_hi6405",
		.owner = THIS_MODULE,
		.of_match_table = hi3xxx_hi6405_of_match,
	},
	.probe	= hi3xxx_hi6405_probe,
	.remove	= hi3xxx_hi6405_remove,
};

static int __init hi3xxx_hi6405_init(void)
{
	pr_info("Audio : hi3xxx-hi6405 init\n");
	return platform_driver_register(&hi3xxx_hi6405_driver);
}

late_initcall(hi3xxx_hi6405_init);

static void __exit hi3xxx_hi6405_exit(void)
{
	platform_driver_unregister(&hi3xxx_hi6405_driver);
}
module_exit(hi3xxx_hi6405_exit);

MODULE_DESCRIPTION("ALSA SoC for Hisilicon hi3xxx with hi6405 codec");
MODULE_LICENSE("GPL");
MODULE_ALIAS("machine driver:hi3xxx-hi6405");
