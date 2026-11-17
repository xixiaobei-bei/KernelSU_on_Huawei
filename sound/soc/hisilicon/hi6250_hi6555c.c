/*
 *  hi6250_hi6555c.c
 *  ALSA SoC
 *  cpu-dai   : Hi6250
 *  codec-dai : hi6555c
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

#include "hi6250_log.h"

#ifdef CONFIG_AK4376_KERNEL_4_1
#define  AK4376_MCLK_FERQ               (19200000)
#define  EXTERN_HIFI_CODEC_AK4376_NAME  "ak4376"
#endif

static struct platform_device *hi6250_hi6555c_snd_device;

static struct snd_soc_dai_link hi6250_hi6555c_dai_link[] =
{
    {
        /* dai link name*/
        .name           = "HI6210_hi6555c",
        /* stream name same as name */
        .stream_name    = "HI6210_hi6555c",
        /* cpu(v9r1:hifi) dai name(device name), see in hi6250-pcm.c */
        .cpu_dai_name   = "hi6210-mm",
        /* codec dai name, see in struct snd_soc_dai_driver in hi6555c.c */
        .codec_dai_name = "hi6555c-dai",
        /* platform(v9r1:hifi) device name, see in hi6250-pcm.c */
        .platform_name  = "hi6210-hifi",
    },
    {
        .name           = "MODEM_hi6555c",
        .stream_name    = "MODEM_hi6555c",
        .cpu_dai_name   = "hi6210-modem",
        .codec_dai_name = "hi6555c-dai",
        .platform_name  = "hi6210-hifi",
    },
    {
        .name           = "FM_hi6555c",
        .stream_name    = "FM_hi6555c",
        .cpu_dai_name   = "hi6210-fm",
        .codec_dai_name = "hi6555c-dai",
        .platform_name  = "hi6210-hifi",
    },
    {
        .name           = "LPP_hi6555c",
        .stream_name    = "LPP_hi6555c",
        .cpu_dai_name   = "hi6210-lpp",
        .codec_dai_name = "hi6555c-dai",
        .platform_name  = "hi6210-hifi",
    },
	{
		.name           = "direct_hi6555c",
		.stream_name    = "hi3660_hi6403_pb_direct",
		.cpu_dai_name   = "hi6210-direct",
		.codec_dai_name = "hi6555c-dai",
		.platform_name  = "hi6210-hifi",
	},
	{
		.name           = "lowlatency_hi6555c",
		.stream_name    = "lowlatency_hi6555c",
		.cpu_dai_name   = "hi6210-fast",
		.codec_dai_name = "hi6555c-dai",
		.platform_name  = "hi6210-hifi",
	},
	{
		.name           = "mmap_hi6555c",
		.stream_name    = "mmap_hi6555c",
		.cpu_dai_name   = "hisi-pcm-mmap",
		.codec_dai_name = "hi6555c-dai",
		.platform_name  = "hi6210-hifi",
	},
};

#ifdef CONFIG_AK4376_KERNEL_4_1
static int hi6250_ak4376_hw_params(struct snd_pcm_substream *substream,
                                   struct snd_pcm_hw_params *params)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *codec_dai = rtd->codec_dai;
    unsigned int fmt = 0;
    int ret = 0;

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
        pr_err("%s : set codec DAI configuration failed %d\n", __FUNCTION__, ret);
        return ret;
    }

    /* set the codec mclk */
    ret = snd_soc_dai_set_sysclk(codec_dai, 0, AK4376_MCLK_FERQ, SND_SOC_CLOCK_IN);
    if (ret < 0) {
        pr_err("%s : set codec system clock failed %d\n", __FUNCTION__, ret);
        return ret;
    }

    return 0;
}

static struct snd_soc_ops hi6250_ak4376_ops = {
    .hw_params = hi6250_ak4376_hw_params,
};

static struct snd_soc_dai_link ak4376_dai_link[] = {
    /* RX for headset/headphone with audio mode */
    {
        .name = "AK4376_PB_OUTPUT",
        .stream_name = "Audio HIFI",
        .codec_name = "ak4376-codec",
        .cpu_dai_name = "hifi-vir-dai",
        .platform_name = "hi6210-hifi",
        .codec_dai_name = "ak4376-AIF1",
        .ops = &hi6250_ak4376_ops,
    },
};

static struct snd_soc_dai_link hi6250_hi6555c_ak4376_dai_links[
                ARRAY_SIZE(hi6250_hi6555c_dai_link) +
                ARRAY_SIZE(ak4376_dai_link)];
#endif
/* Audio machine driver */
static struct snd_soc_card snd_soc_hi6250_hi6555c =
{
    /* sound card name, can see all sound cards in /proc/asound/cards */
    .name       = "HI6250_hi6555c_CARD",
    .dai_link   = hi6250_hi6555c_dai_link,
    .num_links  = ARRAY_SIZE(hi6250_hi6555c_dai_link),
};

#ifdef CONFIG_AK4376_KERNEL_4_1
struct snd_soc_card snd_soc_hi6250_hi6555c_ak4376 = {
    .name = "HI6250_HI6555c_AK4376_CARD",
    .owner = THIS_MODULE,
    .dai_link = hi6250_hi6555c_ak4376_dai_links,
    .num_links = ARRAY_SIZE(hi6250_hi6555c_ak4376_dai_links),
};

static int populate_extern_snd_card_dailinks(struct platform_device *pdev)
{
    pr_info("%s: Audio : hi6250_hi6555c_ak4376_probe \n", __func__);

    memcpy(hi6250_hi6555c_ak4376_dai_links, hi6250_hi6555c_dai_link,
            sizeof(hi6250_hi6555c_dai_link));
    memcpy(hi6250_hi6555c_ak4376_dai_links + ARRAY_SIZE(hi6250_hi6555c_dai_link),
            ak4376_dai_link, sizeof(ak4376_dai_link));

    return 0;
}
#endif

static int hi6250_hi6555c_probe(struct platform_device *pdev)
{
    int ret                             = 0;
    struct device_node  *codec_np       = NULL;
    struct snd_soc_card *card           = &snd_soc_hi6250_hi6555c;
    struct device_node  *np             = NULL;
	uint32_t i = 0;
#ifdef CONFIG_AK4376_KERNEL_4_1
    const char *extern_codec_type       = "huawei,extern_codec_type";
    const char *ptr = NULL;
#endif
    if (NULL == pdev) {
        pr_err("%s : enter with null pointer, fail!\n", __FUNCTION__);
        return -EINVAL;
    }

    logi("%s Begin\n",__FUNCTION__);

    np = pdev->dev.of_node;

    /* set codec node in dai link*/
    codec_np = of_parse_phandle(np, "codec-handle", 0);
    if (!codec_np) {
        loge( "could not find codec node\n");
        return -EINVAL;
    }

	for (i = 0; i < ARRAY_SIZE(hi6250_hi6555c_dai_link); i++) {
		hi6250_hi6555c_dai_link[i].codec_of_node = codec_np;
	}

#ifdef CONFIG_AK4376_KERNEL_4_1
    /* add akm4376 devices */
    ret = of_property_read_string(pdev->dev.of_node, extern_codec_type, &ptr);
    if (!ret) {
        pr_info("%s: extern_codec_type: %s in dt node\n", __func__, extern_codec_type);
        if (!strncmp(ptr, EXTERN_HIFI_CODEC_AK4376_NAME, strlen(EXTERN_HIFI_CODEC_AK4376_NAME))) {
            populate_extern_snd_card_dailinks(pdev);
            pr_info("Audio : set hi6250_hi6555c_ak4376_card\n");
            card = &snd_soc_hi6250_hi6555c_ak4376;
        } else {
            card = &snd_soc_hi6250_hi6555c;
        }
    } else {
        card = &snd_soc_hi6250_hi6555c;
    }
#endif

    /* register card to soc core */
    card->dev = &pdev->dev;
    ret = snd_soc_register_card(card);
    if (ret){
        loge("%s : register failed %d\n", __FUNCTION__, ret);
    }
    logi("%s end\n",__FUNCTION__);
    return ret;
}
static int hi6250_hi6555c_remove(struct platform_device *pdev)
{
    logi("%s\n",__FUNCTION__);

    if( snd_soc_hi6250_hi6555c.dev != &pdev->dev )
    {
        logi("hi6250_hi6555c_remove dev error\n");
        return -ENODEV;
    }

    platform_device_unregister(hi6250_hi6555c_snd_device);

    return 0;
}

static const struct of_device_id hi6250_hi6555c_of_match[] =
{
    {.compatible = "hi6250_hi6555c", },
    { },
};
static struct platform_driver hi6250_hi6555c_driver =
{
    .probe  = hi6250_hi6555c_probe,
    .remove = hi6250_hi6555c_remove,
    .driver =
    {
        .name           = "hi6250_hi6555c",
        .owner          = THIS_MODULE,
        .of_match_table = hi6250_hi6555c_of_match,
    },
};

static int __init hi6250_hi6555c_soc_init(void)
{
    logi("%s\n",__FUNCTION__);
    return platform_driver_register(&hi6250_hi6555c_driver);
}

static void __exit hi6250_hi6555c_soc_exit(void)
{
    logi("%s\n",__FUNCTION__);
    platform_driver_unregister(&hi6250_hi6555c_driver);
}

module_init(hi6250_hi6555c_soc_init);
module_exit(hi6250_hi6555c_soc_exit);

/* Module information */
MODULE_AUTHOR("hisilicon");
MODULE_DESCRIPTION("Hi6250_hi6555c ALSA SoC audio driver");
MODULE_LICENSE("GPL");
