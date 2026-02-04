/*
 *  slimbus is a kernel driver which is used to manager SLIMbus devices
 *  Copyright (C) 2014  Hisilicon

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/version.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <linux/hisi/hilog.h>

#define  AK4376_MCLK_FERQ     (19200000)
#define  EXTERN_HIFI_CODEC_AK4376_NAME  "ak4376"

static struct snd_soc_dai_link hi3650_hi6402_dai_link[] = { 
    {
        /* dai link name*/
        .name       = "hi3650_hi6402_pb_normal",
        /* stream name same as name */
        .stream_name    = "hi3650_hi6402_pb_normal",
        /* codec(hi6401) device name ,see in hi6401.c */
        .codec_name = "hi6402-codec",
        /* cpu(k3v3:asp) dai name(device name), see in hi3650-pcm.c */
        .cpu_dai_name   = "slimbus-dai",
        /* codec dai name, see in struct snd_soc_dai_driver in hi6401.c */
        .codec_dai_name = "hi6402-audio-dai",
        /* platform(k3v3:asp) device name, see in hi3650-pcm.c */
        .platform_name  = "hi6210-hifi",
    },
    {
        /* dai link name*/
        .name       = "hi3650_voice",
        /* stream name same as name */
        .stream_name    = "hi3650_voice",
        /* codec(hi6402) device name ,see in hi6402.c */
        .codec_name = "hi6402-codec",
        /* cpu(k3v3:asp) dai name(device name), see in hi3650-pcm.c */
        .cpu_dai_name   = "slimbus-dai",
        /* codec dai name, see in struct snd_soc_dai_driver in hi6402.c */
        .codec_dai_name = "hi6402-voice-dai",
        /* platform(k3v3:asp) device name, see in hi3650-pcm.c */
        .platform_name  = "snd-soc-dummy",
    },
    {
        /* dai link name*/
        .name       = "hi3650_fm1",
        /* stream name same as name */
        .stream_name    = "hi3650_fm1",
        /* codec(hi6401) device name ,see in hi6401.c */
        .codec_name = "hi6402-codec",
        /* cpu(k3v3:asp) dai name(device name), see in hi3650-pcm.c */
        .cpu_dai_name   = "slimbus-dai",
        /* codec dai name, see in struct snd_soc_dai_driver in hi6401.c */
        .codec_dai_name = "hi6402-fm-dai",
        /* platform(k3v3:asp) device name, see in hi3650-pcm.c */
        .platform_name  = "snd-soc-dummy",
    },      
    {
        /* dai link name*/
        .name       = "hi3650_fm2",
        /* stream name same as name */
        .stream_name    = "hi3650_fm2",
        /* codec(hi6401) device name ,see in hi6401.c */
        .codec_name = "hi6402-codec",
        /* cpu(k3v3:asp) dai name(device name), see in hi3650-pcm.c */
        .cpu_dai_name   = "slimbus-dai",
        /* codec dai name, see in struct snd_soc_dai_driver in hi6401.c */
        .codec_dai_name = "hi6402-fm-dai",
        /* platform(k3v3:asp) device name, see in hi3650-pcm.c */
        .platform_name  = "snd-soc-dummy",
    },
    {
        /* dai link name*/
        .name       = "hi3650_hi6402_pb_dsp",
        /* stream name same as name */
        .stream_name    = "hi3650_hi6402_pb_dsp",
        /* codec(hi6402) device name ,see in hi6402.c */
        .codec_name = "hi6402-codec",
        /* cpu(k3v3:asp) dai name(device name), see in hi3650-pcm.c */
        .cpu_dai_name   = "slimbus-dai",
        /* codec dai name, see in struct snd_soc_dai_driver in hi6402.c */
        .codec_dai_name = "hi6402-audio-dai",
        /* platform(k3v3:asp) device name, see in hi3650-pcm.c */
        .platform_name  = "hi6210-hifi",
    },
};


static int hi3650_ak4376_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned int fmt;
	int ret;

	switch (params_channels(params)) {
		case 2: /* Stereo I2S mode */
			fmt =	SND_SOC_DAIFMT_I2S |
				SND_SOC_DAIFMT_CBS_CFS;
			break;
		default:
			return -EINVAL;
	}

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, fmt);
	if (ret < 0) {
		pr_err("%s : set codec DAI configuration failed %d\n", __FUNCTION__, ret);
		return ret;
	}

	/* set the codec mclk */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, AK4376_MCLK_FERQ,
				SND_SOC_CLOCK_IN);
	if (ret < 0) {
		pr_err("%s : set codec system clock failed %d\n", __FUNCTION__, ret);
		return ret;
	}

	return 0;
}


static struct snd_soc_ops hi3650_ak4376_ops = {
	.hw_params = hi3650_ak4376_hw_params,
};


static struct snd_soc_dai_link ak4376_dai_link[] = {

	{
		 .name = "AK4376_PB_OUTPUT",
		 .stream_name = "Playback",
		 .codec_name = "ak4376-codec",
		 .cpu_dai_name = "hifi-vir-dai",
		 .platform_name = "hi6210-hifi",
		 .codec_dai_name = "ak4376-AIF1",
		 .ops = &hi3650_ak4376_ops,
	},
};

static struct snd_soc_dai_link hi3650_hi6402_ak4376_dai_links[
				ARRAY_SIZE(hi3650_hi6402_dai_link) +
				ARRAY_SIZE(ak4376_dai_link)];

/* Audio machine driver */
static struct snd_soc_card hi3650_hi6402_card = {
	/* sound card name, can see all sound cards in /proc/asound/cards */
	.name = "hi3650_HI6402_CARD",
	.owner = THIS_MODULE,
	.dai_link = hi3650_hi6402_dai_link,
	.num_links = ARRAY_SIZE(hi3650_hi6402_dai_link),
};

struct snd_soc_card hi3650_hi6402_ak4376_card = {
	.name = "hi3650_HI6402_AK4376_CARD",
	.owner = THIS_MODULE,
	.dai_link = hi3650_hi6402_ak4376_dai_links,
	.num_links = ARRAY_SIZE(hi3650_hi6402_ak4376_dai_links),
};

static int populate_extern_snd_card_dailinks(struct platform_device *pdev)
{
	if (of_property_read_bool(pdev->dev.of_node, "huawei,ak4376-codec")) {

		pr_info("%s: Audio : hi3650_hi6402_ak4376_probe \n", __func__);

		memcpy(hi3650_hi6402_ak4376_dai_links, hi3650_hi6402_dai_link,
				sizeof(hi3650_hi6402_dai_link));
		memcpy(hi3650_hi6402_ak4376_dai_links + ARRAY_SIZE(hi3650_hi6402_dai_link),
			ak4376_dai_link, sizeof(ak4376_dai_link));

		return 0;
	}

	return -EINVAL;

}

static int hi3650_hi6402_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct snd_soc_card *card = &hi3650_hi6402_card;
	const char *extern_codec_type = "huawei,extern_codec_type";
	const char *ptr = NULL;

	pr_info("Audio : hi3650_hi6402_probe \n");

	ret = of_property_read_string(pdev->dev.of_node, extern_codec_type, &ptr);
	if (!ret) {
		pr_info("%s: extern_codec_type: %s in dt node\n", __func__, extern_codec_type);
		if (!strncmp(ptr, EXTERN_HIFI_CODEC_AK4376_NAME, strlen(EXTERN_HIFI_CODEC_AK4376_NAME))) {
			if(!populate_extern_snd_card_dailinks(pdev)){
				pr_info("Audio : set  hi3650_hi6402_ak4376_card\n");
				card = &hi3650_hi6402_ak4376_card;
			}else{
				card = &hi3650_hi6402_card;
			}
		} else {
			card = &hi3650_hi6402_card;
		}
	}else{
		card = &hi3650_hi6402_card;
	}
	card->dev = &pdev->dev;

	ret = snd_soc_register_card(card);
	if (ret) {
		pr_err("%s : register failed %d\n", __FUNCTION__, ret);
		HiLOGE("audio", "Hi3650_hi6402", "%s : register failed %d\n", __FUNCTION__, ret);
	}

	return ret;
}

static int hi3650_hi6402_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	if (NULL != card)
		snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id hi3650_hi6402_of_match[] = {
	{.compatible = "hisilicon,hi3xxx-hi6402", },
	{ },
};
MODULE_DEVICE_TABLE(of, hi3650_hi6402_of_match);

static struct platform_driver hi3650_hi6402_driver = {
	.driver	= {
		.name = "hi3650_hi6402",
		.owner = THIS_MODULE,
		.of_match_table = hi3650_hi6402_of_match,
	},
	.probe	= hi3650_hi6402_probe,
	.remove	= hi3650_hi6402_remove,
};
//module_platform_driver(hi3650_hi6402_driver);

static int __init hi3650_init(void)
{
	pr_info("Audio : hi3xxx-hi6402 init \n");
    return platform_driver_register(&hi3650_hi6402_driver);
}

late_initcall(hi3650_init);

static void __exit hi3650_exit(void)
{
    //remove_proc_entry("status", audio_pcm_dir);

    platform_driver_unregister(&hi3650_hi6402_driver);
}
module_exit(hi3650_exit);

/* Module information */
MODULE_AUTHOR("chengong <apollo.chengong@huawei.com>");
MODULE_DESCRIPTION("ALSA SoC for Hisilicon hi3650 with hi6402 codec");
MODULE_LICENSE("GPL");
MODULE_ALIAS("machine driver:hi3650-hi6402");
