/*
 * da_separate_machine.c
 *
 * da_separate driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/clk.h>
#include <linux/kernel.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <asm/io.h>

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/hisi/audio_log.h>

#ifdef CONFIG_AK4376_KERNEL_4_1
#define  AK4376_MCLK_FERQ 19200000
#define  EXTERN_HIFI_CODEC_AK4376_NAME "ak4376"
#endif
#define LOG_TAG "da_separate_machine"

static struct snd_soc_dai_link da_separate_link[] = {
	{
		/* dai link name */
		.name = "platform_da_separate",
		/* stream name same as name */
		.stream_name = "platform_da_separate",
		/* cpu(v9r1:hifi) dai name(device name) */
		.cpu_dai_name = "asp-pcm-mm",
		/* codec dai name, see in struct snd_soc_dai_driver */
		.codec_dai_name = "da_separate-dai",
		/* platform(v9r1:hifi) device name */
		.platform_name = "asp-pcm-hifi",
	},
	{
		.name           = "MODEM_da_separate",
		.stream_name    = "MODEM_da_separate",
		.cpu_dai_name   = "asp-pcm-modem",
		.codec_dai_name = "da_separate-dai",
		.platform_name  = "asp-pcm-hifi",
	},
	{
		.name           = "FM_da_separate",
		.stream_name    = "FM_da_separate",
		.cpu_dai_name   = "asp-pcm-fm",
		.codec_dai_name = "da_separate-dai",
		.platform_name  = "asp-pcm-hifi",
	},
	{
		.name           = "LPP_da_separate",
		.stream_name    = "LPP_da_separate",
		.cpu_dai_name   = "asp-pcm-lpp",
		.codec_dai_name = "da_separate-dai",
		.platform_name  = "asp-pcm-hifi",
	},
	{
		.name           = "direct_da_separate",
		.stream_name    = "da_combine_v3_machine_pb_direct",
		.cpu_dai_name   = "asp-pcm-direct",
		.codec_dai_name = "da_separate-dai",
		.platform_name  = "asp-pcm-hifi",
	},
	{
		.name           = "lowlatency_da_separate",
		.stream_name    = "lowlatency_da_separate",
		.cpu_dai_name   = "asp-pcm-fast",
		.codec_dai_name = "da_separate-dai",
		.platform_name  = "asp-pcm-hifi",
	},
	{
		.name           = "mmap_da_separate",
		.stream_name    = "mmap_da_separate",
		.cpu_dai_name   = "audio-pcm-mmap",
		.codec_dai_name = "da_separate-dai",
		.platform_name  = "asp-pcm-hifi",
	},
};

#ifdef CONFIG_AK4376_KERNEL_4_1
static int ak4376_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned int fmt = 0;
	int ret;

	switch (params_channels(params)) {
	case 2: /* Stereo I2S mode */
		fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS;
		break;
	default:
		return -EINVAL;
	}

	/* Set codec dai configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, fmt);
	if (ret < 0) {
		AUDIO_LOGE("hw params: set DAI configuration failed %d", ret);
		return ret;
	}

	/* set the codec mclk */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, AK4376_MCLK_FERQ, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		AUDIO_LOGE("hw params: set system clock failed %d", ret);
		return ret;
	}

	return 0;
}

static struct snd_soc_ops ak4376_ops = {
	.hw_params = ak4376_hw_params,
};

static struct snd_soc_dai_link ak4376_dai_link[] = {
	/* RX for headset/headphone with audio mode */
	{
		.name = "AK4376_PB_OUTPUT",
		.stream_name = "Audio HIFI",
		.codec_name = "ak4376-codec",
		.cpu_dai_name = "hifi-vir-dai",
		.platform_name = "asp-pcm-hifi",
		.codec_dai_name = "ak4376-AIF1",
		.ops = &ak4376_ops,
	},
};

static struct snd_soc_dai_link ak4376_dai_links[ARRAY_SIZE(da_separate_link) +
	ARRAY_SIZE(ak4376_dai_link)];
#endif

/* Audio machine driver */
static struct snd_soc_card snd_soc_da_separate = {
	/* sound card name, can see all sound cards in /proc/asound/cards */
	.name = "DA_SEPARATE_MACHINE_CARD",
	.dai_link = da_separate_link,
	.num_links = ARRAY_SIZE(da_separate_link),
};

#ifdef CONFIG_AK4376_KERNEL_4_1
struct snd_soc_card snd_soc_ak4376 = {
	.name = "DA_SEPARATE_MACHINE_AK4376_CARD",
	.owner = THIS_MODULE,
	.dai_link = ak4376_dai_links,
	.num_links = ARRAY_SIZE(ak4376_dai_links),
};

static void populate_extern_snd_card_dailinks(struct platform_device *pdev)
{
	AUDIO_LOGI("Audio: probe");

	memcpy(ak4376_dai_links, da_separate_link, sizeof(da_separate_link));
	memcpy(ak4376_dai_links + ARRAY_SIZE(da_separate_link), ak4376_dai_link,
		sizeof(ak4376_dai_link));
}
#endif

static int da_separate_probe(struct platform_device *pdev)
{
	int ret;
	uint32_t i;
	struct device_node *codec_np = NULL;
	struct snd_soc_card *card = &snd_soc_da_separate;
	struct device_node *np = NULL;

#ifdef CONFIG_AK4376_KERNEL_4_1
	const char *extern_codec_type = "huawei,extern_codec_type";
	const char *ptr = NULL;
#endif
	if (!pdev) {
		AUDIO_LOGE("probe: enter with null pointer, fail");
		return -EINVAL;
	}

	AUDIO_LOGI("probe Begin");

	np = pdev->dev.of_node;

	/* set codec node in dai link*/
	codec_np = of_parse_phandle(np, "codec-handle", 0);
	if (!codec_np) {
	AUDIO_LOGE("could not find node");
	return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(da_separate_link); i++)
		da_separate_link[i].codec_of_node = codec_np;

#ifdef CONFIG_AK4376_KERNEL_4_1
	/* add akm4376 devices */
	ret = of_property_read_string(pdev->dev.of_node, extern_codec_type, &ptr);
	if (!ret) {
		AUDIO_LOGI("probe: extern type: %s in dt node", extern_codec_type);
		if (!strncmp(ptr, EXTERN_HIFI_CODEC_AK4376_NAME, strlen(EXTERN_HIFI_CODEC_AK4376_NAME))) {
			populate_extern_snd_card_dailinks(pdev);
			AUDIO_LOGI("Audio: set card");
			card = &snd_soc_ak4376;
		} else {
			card = &snd_soc_da_separate;
		}
	} else {
		card = &snd_soc_da_separate;
	}
#endif

	/* register card to soc core */
	card->dev = &pdev->dev;
	ret = snd_soc_register_card(card);
	if (ret)
		AUDIO_LOGE("probe: register failed %d", ret);

	AUDIO_LOGI("probe end");

	return ret;
}

static const struct of_device_id da_separate_of_match[] = {
	{.compatible = "hisilicon,da_separate-machine", },
	{ },
};

static struct platform_driver da_separate_driver = {
	.probe = da_separate_probe,
	.driver = {
		.name = "da_separate-machine",
		.owner = THIS_MODULE,
		.of_match_table = da_separate_of_match,
	},
};

static int __init da_separate_soc_init(void)
{
	AUDIO_LOGI("soc init");
	return platform_driver_register(&da_separate_driver);
}

static void __exit da_separate_soc_exit(void)
{
	AUDIO_LOGI("soc exit");
	platform_driver_unregister(&da_separate_driver);
}

module_init(da_separate_soc_init);
module_exit(da_separate_soc_exit);

/* Module information */
MODULE_DESCRIPTION("DA SEPARATE ALSA SoC audio driver");
MODULE_LICENSE("GPL");

