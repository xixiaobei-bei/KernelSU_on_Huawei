/*
 * hwcxext_codec_interface.c
 *
 * analog headset interface
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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
#include <securec.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG hwcxext_interface
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

#define EXTERN_CODEC_NAME  "hwcxext_codec"
#define DEFAULT_MCLK_FERQ 19200000
#define MAX_DAILINK_NUM 100

static unsigned int g_mclk;

static int hwcxext_codec_init(struct snd_soc_pcm_runtime *rtd)
{
	int ret;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;

	IN_FUNCTION;

	/* set the codec mclk */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, g_mclk, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		hwlog_err("%s : set codec system clock failed %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static int hwcxext_hw_params(struct snd_pcm_substream *substream,
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
			__func__, ret);
		return ret;
	}

	/* set the codec mclk */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, g_mclk, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		hwlog_err("%s : set codec system clock failed %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static struct snd_soc_ops hwcxext_ops = {
	.hw_params = hwcxext_hw_params,
};

static struct snd_soc_dai_link hwcxext_dai_link[] = {
	{
		.name = "hwcxext_codec",
		.stream_name = "hwcxext_codec",
		.cpu_dai_name = "hifi-vir-dai",
		.platform_name = "asp-pcm-hifi",
		.codec_name = "hwcxext_codec",
		.codec_dai_name = "HWCXEXT HiFi",
		.init = &hwcxext_codec_init,
		.ops = &hwcxext_ops,
	},
};

struct snd_soc_card snd_soc_hwcxext = {
	.name = "HI6250_HI6555c_HWCXEXT_CARD",
	.owner = THIS_MODULE,
};

static int hwcxext_codec_interface_params_check(struct platform_device *pdev,
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

void hwcxext_codec_register_in_machine_driver(struct platform_device *pdev,
	struct snd_soc_card **card,
	struct snd_soc_dai_link *dai_link, unsigned int links_num)
{
	unsigned int total_links = 0;
	int ret;
	const char *extern_codec_type = "huawei,extern_codec_type";
	const char *ptr = NULL;
	struct snd_soc_dai_link *hwcxext_dai_links = NULL;
	unsigned int size;

	if (hwcxext_codec_interface_params_check(pdev, card,
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
	size = (links_num + ARRAY_SIZE(hwcxext_dai_link)) *
		sizeof(struct snd_soc_dai_link);
	hwcxext_dai_links = kzalloc(size, GFP_KERNEL);
	if (hwcxext_dai_links == NULL) {
		pr_err("dai_links is NULL\n");
		return;
	}

	if (memcpy_s(hwcxext_dai_links + total_links,
		links_num * sizeof(struct snd_soc_dai_link), dai_link,
		links_num * sizeof(struct snd_soc_dai_link)) != EOK)
		hwlog_err("%s: memcpy_s is failed\n", __func__);

	total_links += links_num;
	if (memcpy_s(hwcxext_dai_links + total_links,
		sizeof(hwcxext_dai_link), hwcxext_dai_link,
		sizeof(hwcxext_dai_link)) != EOK)
		hwlog_err("%s: memcpy_s is failed\n", __func__);

	total_links += ARRAY_SIZE(hwcxext_dai_link);

	*card = &snd_soc_hwcxext;
	(*card)->dai_link = hwcxext_dai_links;
	(*card)->num_links = (int)total_links;
}
EXPORT_SYMBOL_GPL(hwcxext_codec_register_in_machine_driver);

