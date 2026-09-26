/*
 * hwevext_codec_interface.c
 *
 * analog headset interface
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG hwevext_interface
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

#define EXTERN_CODEC_NAME  "hwevext_codec"
#define DEFAULT_MCLK_FERQ 19200000
#define MAX_DAILINK_NUM 200

static unsigned int g_mclk;

static int hwevext_codec_init(struct snd_soc_pcm_runtime *rtd)
{
	int ret;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;

	IN_FUNCTION;

	/* set the codec mclk */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, g_mclk, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		hwlog_err("%s : set codec system clock failed %d\n",
			__FUNCTION__, ret);
		return ret;
	}

	return 0;
}

static int hwevext_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned int fmt = 0;
	int ret = 0;

	IN_FUNCTION;
	switch (params_channels(params)) {
	case 2: /* Stereo I2S mode */
		fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS;
		break;
	default:
		return -EINVAL;
	}

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, fmt);
	if (ret < 0) {
		hwlog_err("%s : set codec DAI configuration failed %d\n",
			__FUNCTION__, ret);
		return ret;
	}

	/* set the codec mclk */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, g_mclk, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		hwlog_err("%s : set codec system clock failed %d\n",
			__FUNCTION__, ret);
		return ret;
	}

	return 0;
}

static struct snd_soc_ops hwevext_ops = {
	.hw_params = hwevext_hw_params,
};

static struct snd_soc_dai_link hwevext_dai_link[] = {
	{
		.name = "hwevext_codec",
		.stream_name = "hwevext_codec",
		.cpu_dai_name = "hifi-vir-dai",
		.platform_name = "asp-pcm-hifi",
		.codec_name = "hwevext_codec",
		.codec_dai_name = "HWEVEXT HiFi",
		.init = &hwevext_codec_init,
		.ops = &hwevext_ops,
	},
};

struct snd_soc_card snd_soc_hwevext = {
	.name = "HI6250_HI6555c_HWEVEXT_CARD",
	.owner = THIS_MODULE,
};

static int hwevext_codec_interface_params_check(struct platform_device *pdev,
	struct snd_soc_card **card,
	struct snd_soc_dai_link *dai_link,
	unsigned int links_num)
{
	if (pdev == NULL || card == NULL || dai_link == NULL ||
		links_num > MAX_DAILINK_NUM) {
		hwlog_err("%s: params is invaild\n", __func__);
		return -EINVAL;
	}

	return 0;
}

void hwevext_codec_register_in_machine_driver(struct platform_device *pdev,
	struct snd_soc_card **card,
	struct snd_soc_dai_link *dai_link, unsigned int links_num)
{
	int total_links = 0;
	int ret;
	const char *extern_codec_type = "huawei,extern_codec_type";
	const char *ptr = NULL;
	struct snd_soc_dai_link *hwevext_dai_links = NULL;
	unsigned int size;

	if (hwevext_codec_interface_params_check(pdev, card,
		dai_link, links_num) < 0) {
		hwlog_err("%s: params is invaild\n", __func__);
		return;
	}

	ret = of_property_read_string(pdev->dev.of_node,
		extern_codec_type, &ptr);
	if (ret)
		return;

	hwlog_info("%s: extern_codec_type: %s in dt node\n", __func__, ptr);
	if (strncmp(ptr, EXTERN_CODEC_NAME, strlen(EXTERN_CODEC_NAME)))
		return;

	ret = of_property_read_u32(pdev->dev.of_node, "extern_codec_clk", &g_mclk);
	if (ret < 0)
		g_mclk = DEFAULT_MCLK_FERQ;

	hwlog_info("%s: externcodec g_mclk is %u\n", __func__, g_mclk);
	size = (links_num + ARRAY_SIZE(hwevext_dai_link)) *
		sizeof(struct snd_soc_dai_link);
	hwevext_dai_links = kzalloc(size, GFP_KERNEL);
	if (hwevext_dai_links == NULL) {
		pr_err("dai_links is NULL\n");
		return;
	}

	memcpy(hwevext_dai_links + total_links,
		dai_link, links_num * sizeof(struct snd_soc_dai_link));
	total_links += links_num;
	memcpy(hwevext_dai_links + total_links,
		hwevext_dai_link,
		sizeof(hwevext_dai_link));
	total_links += ARRAY_SIZE(hwevext_dai_link);

	*card = &snd_soc_hwevext;
	(*card)->dai_link = hwevext_dai_links;
	(*card)->num_links = total_links;
}
EXPORT_SYMBOL_GPL(hwevext_codec_register_in_machine_driver);

