/*
 * asp_hdmi_dma.c
 *
 * asp dma driver module for hdmi.
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#include "asp_hdmi_dma.h"

#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/bits.h>

#include "audio_log.h"

#define LOG_TAG "asp_hdmi_dma"

/* channel info:channel_num, reg_value */
const unsigned int g_asp_hdmi_channel_num[CHANNEL_NUM_MAX][2] = {
	[CHANNEL_NUM_1] = { 1, 0x0 },
	[CHANNEL_NUM_2] = { 2, 0x1 },
	[CHANNEL_NUM_4] = { 4, 0x3 },
	[CHANNEL_NUM_6] = { 6, 0x5 },
	[CHANNEL_NUM_8] = { 8, 0x7 }
};

/* bit_width info:bit_width, reg_value */
const unsigned int g_asp_hdmi_bit_width[BIT_WIDTH_MAX][2] = {
	[BIT_WIDTH_16] = { 16, 0x0 },
	[BIT_WIDTH_17] = { 17, 0x1 },
	[BIT_WIDTH_18] = { 18, 0x2 },
	[BIT_WIDTH_19] = { 19, 0x3 },
	[BIT_WIDTH_20] = { 20, 0x4 },
	[BIT_WIDTH_21] = { 21, 0x5 },
	[BIT_WIDTH_22] = { 22, 0x6 },
	[BIT_WIDTH_23] = { 23, 0x7 },
	[BIT_WIDTH_24] = { 24, 0x8 }
};

/* align type:align type, reg_value */
const unsigned int g_asp_hdmi_align_type[ALIGN_MAX][2] = {
	[ALIGN_16] = { 16, 0x1 },
	[ALIGN_32] = { 32, 0x0 }
};

/* sample rate info: sample rate,reg_value */
const unsigned int g_asp_hdmi_sio_sample_rate[SAMPLE_RATE_MAX][2] = {
	[SAMPLE_RATE_32] = { 32, 0x03 },
	[SAMPLE_RATE_44] = { 44, 0x00 },
	[SAMPLE_RATE_48] = { 48, 0x02 },
	[SAMPLE_RATE_88] = { 88, 0x08 },
	[SAMPLE_RATE_96] = { 96, 0x0A },
	[SAMPLE_RATE_176] = { 176, 0x0C },
	[SAMPLE_RATE_192] = { 192, 0x0E }
};

struct asp_hdmi_dma_priv {
	struct device *dev;
	spinlock_t lock;
	struct resource *res;
	void __iomem *asp_dma_reg_base_addr;
};

static struct asp_hdmi_dma_priv *g_asp_hdmi_dma_priv;

static unsigned int dmac_reg_read(unsigned int reg)
{
	struct asp_hdmi_dma_priv *priv = g_asp_hdmi_dma_priv;
	unsigned int ret;
	unsigned long flag_sft = 0;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return 0;
	}

	spin_lock_irqsave(&priv->lock, flag_sft);
	ret = readl(priv->asp_dma_reg_base_addr + reg);
	spin_unlock_irqrestore(&priv->lock, flag_sft);

	return ret;
}

static void dmac_reg_write(unsigned int reg, unsigned int value)
{
	struct asp_hdmi_dma_priv *priv = g_asp_hdmi_dma_priv;
	unsigned long flag_sft = 0;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return;
	}

	spin_lock_irqsave(&priv->lock, flag_sft);
	writel(value, priv->asp_dma_reg_base_addr + reg);
	spin_unlock_irqrestore(&priv->lock, flag_sft);
}

static void dmac_reg_set_bit(unsigned int reg, unsigned int offset)
{
	struct asp_hdmi_dma_priv *priv = g_asp_hdmi_dma_priv;
	unsigned int value;
	unsigned long flag_sft = 0;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return;
	}

	spin_lock_irqsave(&priv->lock, flag_sft);

	value = readl(priv->asp_dma_reg_base_addr + reg);
	value |= BIT(offset);
	writel(value, priv->asp_dma_reg_base_addr + reg);

	spin_unlock_irqrestore(&priv->lock, flag_sft);
}

static void dmac_reg_clr_bit(unsigned int reg, unsigned int offset)
{
	struct asp_hdmi_dma_priv *priv = g_asp_hdmi_dma_priv;
	unsigned int value;
	unsigned long flag_sft = 0;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return;
	}

	spin_lock_irqsave(&priv->lock, flag_sft);

	value = readl(priv->asp_dma_reg_base_addr + reg);
	value &= ~BIT(offset);
	writel(value, priv->asp_dma_reg_base_addr + reg);

	spin_unlock_irqrestore(&priv->lock, flag_sft);
}

#ifdef ASP_HDMI_DMA_DEBUG
static void dmac_dump(void)
{
	AUDIO_LOGD("reg hdmi tx3: 0x%x", dmac_reg_read(ASP_HDMI_TX3));
	AUDIO_LOGD("reg asp hdmi dma en: 0x%x", dmac_reg_read(ASP_HDMI_DMA_EN));
	AUDIO_LOGD("reg asp hdmi int en: 0x%x", dmac_reg_read(ASP_HDMI_INT_EN));
	AUDIO_LOGD("reg a add: %pK", (void *)(uintptr_t)dmac_reg_read(ASP_HDMI_A_ADDR));
	AUDIO_LOGD("reg a len: 0x%x", dmac_reg_read(ASP_HDMI_A_LEN));
	AUDIO_LOGD("reg b add: %pK", (void *)(uintptr_t)dmac_reg_read(ASP_HDMI_B_ADDR));
	AUDIO_LOGD("reg b len: 0x%x", dmac_reg_read(ASP_HDMI_B_LEN));
}
#endif

static int transform_to_tx3_reg_value(
	struct tx3_config_parameters parameters, unsigned int *reg_value)
{
	unsigned int value = 0x0;

	if (parameters.bit_width > (BIT_WIDTH_MAX - 1)) {
		AUDIO_LOGE("not support parame: bit width %u", parameters.bit_width);
		return -EPERM;
	}

	if ((parameters.channel_num > (CHANNEL_NUM_MAX - 1)) ||
		(parameters.channel_num == CHANNEL_NUM_1 &&
		parameters.bit_width != BIT_WIDTH_16)) {
		AUDIO_LOGE("not support parame: channel %u and bit width %u",
			parameters.channel_num, parameters.bit_width);
		return -EPERM;
	}

	if ((parameters.align_type != ALIGN_16) && (parameters.align_type != ALIGN_32)) {
		AUDIO_LOGE("not support parame: align type %u", parameters.align_type);
		return -EPERM;
	}

	/**
	 * 16bit align is the opposite of 32bit align, need to configure switch.
	 * If not configured, the channel will be reversed.
	 */
	if (parameters.align_type == ALIGN_16)
		value |= (HDMI_PCM_SWITCH_ORDE_MASK << HDMI_PCM_SWITCH_ORDE_BIT);

	value |= (g_asp_hdmi_channel_num[parameters.channel_num][1] << 8) |
		(g_asp_hdmi_align_type[parameters.align_type][1] << 7) |
		(g_asp_hdmi_bit_width[parameters.bit_width][1] << 3) |
		HDMI_TX3_EN_MASK;

	*reg_value = value;

#ifdef ASP_HDMI_DMA_DEBUG
	AUDIO_LOGD("parameters bit width: 0x%x", parameters.bit_width);
	AUDIO_LOGD("parameters channel num: 0x%x", parameters.channel_num);
	AUDIO_LOGD("parameters align type: 0x%x", parameters.align_type);
	AUDIO_LOGD("value: 0x%x", value);
#endif

	return 0;
}

static unsigned int io_config_get_channel_num(unsigned int tx3_channel_num)
{
	unsigned int channel_num;

	switch (tx3_channel_num) {
	case CHANNEL_NUM_1:
		channel_num = 1;
		break;
	case CHANNEL_NUM_2:
		channel_num = 2;
		break;
	case CHANNEL_NUM_4:
		channel_num = 4;
		break;
	case CHANNEL_NUM_6:
		channel_num = 6;
		break;
	case CHANNEL_NUM_8:
		channel_num = 8;
		break;
	default:
		channel_num = 2;
		break;
	}

	return channel_num;
}

static void io_config_channel_type(unsigned int channel_num)
{
	unsigned int io_ch;
	unsigned int i;

	for (i = 0; i < channel_num; i += 2) {
		io_ch = i / 2;
		dmac_reg_set_bit(asp_hdmi_io_ch_status1_l(io_ch), HDMI_SIO_CHANNEL_TYPE_BIT);
		dmac_reg_set_bit(asp_hdmi_io_ch_status1_r(io_ch), HDMI_SIO_CHANNEL_TYPE_BIT);
	}
}

static void io_config_sample_rate(unsigned int channel_num, unsigned int sample_rate)
{
	unsigned int io_ch;
	unsigned int i;
	unsigned int reg_value;

	if (sample_rate >= SAMPLE_RATE_NO_SUPPORT) {
		AUDIO_LOGE("sample rate %u is no support, and will use default io config",sample_rate);
		return;
	}
	reg_value = dmac_reg_read(ASP_HDMI_IO_CH0_STATUS1_L);
	reg_value &= ~HDMI_SPDIF_SAMPLE_RATE_MASK;
	reg_value |= (g_asp_hdmi_sio_sample_rate[sample_rate][1]) << HDMI_SPDIF_SAMPLE_RATE_SHIFT;

	for (i = 0; i < channel_num; i += 2) {
		io_ch = i / 2;
		dmac_reg_write(asp_hdmi_io_ch_status1_l(io_ch), reg_value);
		dmac_reg_write(asp_hdmi_io_ch_status1_r(io_ch), reg_value);
	}
	AUDIO_LOGI("set io sample rate %u", sample_rate);
}

static void io_config_copy_right(unsigned int channel_num, bool is_hdcp)
{
	unsigned int io_ch;
	unsigned int i;

	if (is_hdcp) {
		for (i = 0; i < channel_num; i += 2) {
			io_ch = i / 2;
			dmac_reg_set_bit(asp_hdmi_io_ch_status1_l(io_ch), HDMI_SIO_HDCP_BIT);
			dmac_reg_set_bit(asp_hdmi_io_ch_status1_r(io_ch), HDMI_SIO_HDCP_BIT);
		}
		AUDIO_LOGI("set io hdcp bit to 1");
	}
}

static void io_config_bit_width(unsigned int channel_num, unsigned int bit_width)
{
	unsigned int io_ch;
	unsigned int i;
	unsigned int reg_value;

	if (bit_width >= BIT_WIDTH_MAX) {
		AUDIO_LOGE("bitwidth %u is no support, and will use default io bitwidth config",
			bit_width);
			return;
	}
	reg_value = dmac_reg_read(ASP_HDMI_IO_CH0_STATUS2_L);
	reg_value &= ~HDMI_SPDIF_BITWIDTH_MASK;
	if (bit_width == BIT_WIDTH_16)
		reg_value |=  HDMI_SPDIF_BITWIDTH_16BIT;
	else
		reg_value |=  HDMI_SPDIF_BITWIDTH_24BIT;

	for (i = 0; i < channel_num; i += 2) {
		io_ch = i / 2;
		dmac_reg_write(asp_hdmi_io_ch_status2_l(io_ch), reg_value);
		dmac_reg_write(asp_hdmi_io_ch_status2_r(io_ch), reg_value);
	}
	AUDIO_LOGI("set io bitwidth %u", bit_width);
}

static void io_config_dump(unsigned int channel_num)
{
	unsigned int io_ch;
	unsigned int i;
	unsigned int reg_value;

	for (i = 0; i < channel_num; i += 2) {

		io_ch = i / 2;

		reg_value = dmac_reg_read(asp_hdmi_io_ch_status1_l(io_ch));
		AUDIO_LOGI("set asp hdmi io ch %u status1 l vaule: 0x%x",
			io_ch, reg_value);
		reg_value = dmac_reg_read(asp_hdmi_io_ch_status2_l(io_ch));
		AUDIO_LOGI("set asp hdmi io ch %u status2 l vaule: 0x%x",
			io_ch, reg_value);
		reg_value = dmac_reg_read(asp_hdmi_io_ch_status1_r(io_ch));
		AUDIO_LOGI("set asp hdmi io ch %u status1 r vaule: 0x%x",
			io_ch, reg_value);
		reg_value = dmac_reg_read(asp_hdmi_io_ch_status2_r(io_ch));
		AUDIO_LOGI("set asp hdmi io ch %u status2 r vaule: 0x%x",
			io_ch, reg_value);
	}

}

void asp_hdmi_dma_enable(void)
{
	dmac_reg_write(ASP_HDMI_DMA_EN, HDMI_DMA_EN_MASK);
}

unsigned int asp_hdmi_reg_read_irsr(void)
{
	return dmac_reg_read(ASP_HDMI_INT_STATE);
}

int asp_hdmi_tx3_config(struct tx3_config_parameters parameters)
{
	unsigned int reg_value = 0x0;
	int ret;

	ret = transform_to_tx3_reg_value(parameters, &reg_value);
	if (ret != 0) {
		AUDIO_LOGE("fail");
		return -EPERM;
	}

	dmac_reg_write(ASP_HDMI_TX3, reg_value);

#ifdef ASP_HDMI_DMA_DEBUG
	dmac_dump();
#endif

	AUDIO_LOGI("success");

	return 0;
}

void asp_hdmi_tx3_enable(void)
{
	dmac_reg_set_bit(ASP_HDMI_TX3, HDMI_TX3_EN_BIT);
	AUDIO_LOGI("end");
}

void asp_hdmi_tx3_disable(void)
{
	if ((dmac_reg_read(ASP_HDMI_TX3) & 0x1) == 1) {
		dmac_reg_clr_bit(ASP_HDMI_TX3, HDMI_TX3_EN_BIT);
		AUDIO_LOGI("success");
	} else {
		AUDIO_LOGI("do nothing");
	}

}

void asp_hdmi_io_config(struct sio_config_parameters parameters)
{
	unsigned int channel_num;

	channel_num = io_config_get_channel_num(parameters.tx3_conf->channel_num);

	io_config_channel_type(channel_num);

	io_config_sample_rate(channel_num, parameters.sample_rate);

	io_config_copy_right(channel_num, parameters.is_hdcp);

	io_config_bit_width(channel_num, parameters.tx3_conf->bit_width);

	io_config_dump(channel_num);
}

bool asp_hdmi_dma_is_stop(void)
{
	unsigned int dma_en;
	unsigned int int_mask_flag;

	dma_en = dmac_reg_read(ASP_HDMI_DMA_EN);
	int_mask_flag = dmac_reg_read(ASP_HDMI_INT_MSK_STATE);

	if ((dma_en | int_mask_flag) & HDMI_DMA_EN_MASK)
		return false;

	return true;
}

int asp_hdmi_dma_config(unsigned int addr, unsigned int size)
{
	if (((addr & HDMI_DMA_ADD_VALID_MASK) != 0) ||
		((size & HDMI_DMA_ADDLEN_VALID_MASK) != 0)) {
		AUDIO_LOGE("parame invalid, addr: %pK, size: %u",
			(void *)(uintptr_t)addr, size);
		return -EINVAL;
	}
#ifdef DP_AUDIO_ASP_HDMI_I2S
	/* clean up i2s configuration */
	dmac_reg_write(ASP_HDMI_I2S_CLR, HDMI_I2S_CLR_MASK);

	/* select i2s interface as ASP_SPDIFSEL */
	dmac_reg_clr_bit(ASP_HDMI_SPDIF_SEL, 0);
#else
	/* select spdif interface as ASP_SPDIFSEL */
	dmac_reg_set_bit(ASP_HDMI_SPDIF_SEL, 0);
#endif

	/* set PCMA src addr and buffer */
	dmac_reg_write(ASP_HDMI_A_ADDR, addr);
	dmac_reg_write(ASP_HDMI_A_LEN, size);

	/* set PCMB src addr and buffer */
	dmac_reg_write(ASP_HDMI_B_ADDR, addr + size);
	dmac_reg_write(ASP_HDMI_B_LEN, size);

	/* clean up the irq */
	dmac_reg_write(ASP_HDMI_INT_CLR, HDMI_INT_MASK);

	AUDIO_LOGI("dma config success, select %s interface",
		(dmac_reg_read(ASP_HDMI_SPDIF_SEL) & 0x1) ? "spdif" : "i2s");

	return 0;
}

void asp_hdmi_dma_clear_interrupt(unsigned int value)
{
	dmac_reg_write(ASP_HDMI_INT_CLR, value);
}

int asp_hdmi_dma_start(void)
{
	/* dma enable */
	dmac_reg_write(ASP_HDMI_DMA_EN, HDMI_DMA_EN_MASK);

	/* dma interrupt enable */
	dmac_reg_write(ASP_HDMI_INT_EN, HDMI_INT_MASK);

#ifdef DP_AUDIO_ASP_HDMI_I2S
	/* set i2s */
	dmac_reg_write(ASP_HDMI_I2S_SET, HDMI_I2S_SET_MASK);
#else
	/* set spdif */
	dmac_reg_write(ASP_HDMI_SPDIF_CTRL, HDMI_SPDIF_SET_MASK);
#endif
#ifdef ASP_HDMI_DMA_DEBUG
	dmac_dump();
#endif

	AUDIO_LOGI("hdmi dma start success");

	return 0;
}

void asp_hdmi_dma_stop(void)
{
	/* dma interrupt disable */
	dmac_reg_clr_bit(ASP_HDMI_INT_EN, 0);
	dmac_reg_clr_bit(ASP_HDMI_INT_EN, 1);
	dmac_reg_clr_bit(ASP_HDMI_INT_EN, 2);

	/* dma  disable */
	dmac_reg_write(ASP_HDMI_DMA_EN, HDMI_DMA_DISABLE_MASK);

	AUDIO_LOGI("hdmi dma stop success");
}

static int asp_hdmi_dma_probe(struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct asp_hdmi_dma_priv *priv = NULL;

	if (!pdev) {
		AUDIO_LOGE("pdev is null");
		return -EINVAL;
	}

	dev = &pdev->dev;
	AUDIO_LOGI("probe begin");

	priv = devm_kzalloc(dev, sizeof(struct asp_hdmi_dma_priv), GFP_KERNEL);
	if (!priv) {
		AUDIO_LOGE("malloc failed");
		return -ENOMEM;
	}

	priv->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!priv->res) {
		AUDIO_LOGE("get resource failed");
		return -ENOENT;
	}

	priv->asp_dma_reg_base_addr = devm_ioremap(dev, priv->res->start, resource_size(priv->res));
	if (!priv->asp_dma_reg_base_addr) {
		AUDIO_LOGE("asp dma reg addr ioremap failed");
		return -ENOMEM;
	}

#ifdef ASP_HDMI_DMA_DEBUG
	AUDIO_LOGD("res start: %pK", (void *)(uintptr_t)priv->res->start);
	AUDIO_LOGD("asp dma reg base addr: %pK", (void *)priv->asp_dma_reg_base_addr);
#endif

	spin_lock_init(&priv->lock);
	priv->dev = dev;
	platform_set_drvdata(pdev, priv);
	g_asp_hdmi_dma_priv = priv;
	AUDIO_LOGI("probe end");

	return 0;
}

static int asp_hdmi_dma_remove(struct platform_device *pdev)
{
	struct asp_hdmi_dma_priv *priv =
		(struct asp_hdmi_dma_priv *)platform_get_drvdata(pdev);

	if (!priv)
		return 0;

	g_asp_hdmi_dma_priv = NULL;

	AUDIO_LOGI("asp dma driver remove success");

	return 0;
}

static const struct of_device_id of_asp_hdmi_dma_match[] = {
	{ .compatible = "hisilicon,asp-hdmi-dma", },
	{},
};

MODULE_DEVICE_TABLE(of, of_asp_hdmi_dma_match);

static struct platform_driver asp_hdmi_dma_driver = {
	.driver = {
		.name = "asp_hdmi_dma_drv",
		.owner = THIS_MODULE,
		.of_match_table = of_asp_hdmi_dma_match,
	},
	.probe  = asp_hdmi_dma_probe,
	.remove = asp_hdmi_dma_remove,
};

static int __init asp_hdmi_dma_init(void)
{
	return platform_driver_register(&asp_hdmi_dma_driver);
}
module_init(asp_hdmi_dma_init);

static void __exit asp_hdmi_dma_exit(void)
{
	platform_driver_unregister(&asp_hdmi_dma_driver);
}
module_exit(asp_hdmi_dma_exit);

MODULE_DESCRIPTION("ASP HDMI DMA Driver");
MODULE_LICENSE("GPL v2");
