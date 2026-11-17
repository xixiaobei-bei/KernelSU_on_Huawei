/*
 * da_combine_v3.c
 *
 * da_combine_v3 codec driver
 *
 * Copyright (c) 2015 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>

#include <linux/hisi/audio_log.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif

#include "asoc_adapter.h"
#include "slimbus.h"
#include "slimbus_da_combine_v3.h"
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "debug.h"
#endif
#include "linux/hisi/da_combine/da_combine_compat.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "linux/hisi/da_combine/da_combine_resmgr.h"
#include "linux/hisi/da_combine/da_combine_vad.h"
#include "linux/hisi/da_combine/da_combine_mbhc.h"
#include "linux/hisi/da_combine/da_combine_v3_regs.h"
#include "type.h"
#include "da_combine_v3_dsp_config.h"
#include "kcontrol.h"
#include "resource_widget.h"
#include "path_widget.h"
#include "switch_widget.h"
#include "route.h"
#include "headset_config.h"

#define DA_COMBINE_V3_CODEC_NAME "da_combine_v3-codec"

#define MBHC_VOLTAGE_COEFFICIENT_MIN 1600
#define MBHC_VOLTAGE_COEFFICIENT_DEFAULT 2700
#define MBHC_VOLTAGE_COEFFICIENT_MAX 2800
#define ADC_PGA_GAIN_DEFAULT 0x78
#define DACLR_MIXER_PGA_GAIN_DEFAULT 0xFF
#define HS_RES_DEFAULT 300
#define VOICE_HW_FREE_DATA 0xC4
#define MIC_FIXED_VAL_DEFAULT 0x78

#define PLL_DATA_ALL_NUM     128
#define PLL_DATA_GROUP_NUM   8
#define PLL_DATA_BUF_SIZE    10

#define DA_COMBINE_V3_IRQ_NUM               48

#define DAI_LINK_CODEC_NAME            "da_combine_v3"
#define DAI_LINK_CODEC_DAI_NAME_AUDIO  "da_combine_v3-audio-dai"
#define DAI_LINK_CODEC_DAI_NAME_VOICE  "da_combine_v3-voice-dai"
#define DAI_LINK_CODEC_DAI_NAME_FM     "da_combine_v3-fm-dai"

struct irq_config {
	enum da_combine_irq_type type;
	irq_handler_t handler;
	const char *name;
	bool enabled;
};

struct reg_page {
	unsigned int offest;
	unsigned int begin;
	unsigned int end;
};

static const struct reg_page reg_info[] = {
	{ BASE_ADDR_PAGE_IO, DA_COMBINE_V3_PAGE_IO_START, DA_COMBINE_V3_PAGE_IO_END },
	{ BASE_ADDR_PAGE_CFG, DA_COMBINE_V3_PAGE_CFG_START, DA_COMBINE_V3_PAGE_CFG_END },
	{ BASE_ADDR_PAGE_ANA, DA_COMBINE_V3_PAGE_ANA_OFFSET_START, DA_COMBINE_V3_PAGE_ANA_OFFSET_END },
	{ BASE_ADDR_PAGE_DIG, DA_COMBINE_V3_PAGE_DIG_OFFSET_START, DA_COMBINE_V3_PAGE_DIG_OFFSET_END },

	{ DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_PAGE_VIRTUAL_START, DA_COMBINE_V3_PAGE_VIRTUAL_END },
	{ BASE_ADDR_PAGE_OCRAM, DA_COMBINE_V3_PAGE_OCRAM_START, DA_COMBINE_V3_PAGE_OCRAM_END },
	{ BASE_ADDR_PAGE_TCM, DA_COMBINE_V3_PAGE_TCM_START, DA_COMBINE_V3_PAGE_TCM_END },

	{ BASE_ADDR_PAGE_WDOG, DA_COMBINE_V3_PAGE_WDOG_OFFSET_START, DA_COMBINE_V3_PAGE_WDOG_OFFSET_END },
	{ BASE_ADDR_PAGE_SCTRL, DA_COMBINE_V3_PAGE_SCTRL_OFFSET_START, DA_COMBINE_V3_PAGE_SCTRL_OFFSET_END },
	{ BASE_ADDR_PAGE_UART, DA_COMBINE_V3_PAGE_UART_OFFSET_START, DA_COMBINE_V3_PAGE_UART_OFFSET_END },
	{ BASE_ADDR_PAGE_TIMER0, DA_COMBINE_V3_PAGE_TIMER0_OFFSET_START, DA_COMBINE_V3_PAGE_TIMER0_OFFSET_END },
	{ BASE_ADDR_PAGE_TIMER1, DA_COMBINE_V3_PAGE_TIMER1_OFFSET_START, DA_COMBINE_V3_PAGE_TIMER1_OFFSET_END },

	{ BASE_ADDR_PAGE_GPIO0, DA_COMBINE_V3_PAGE_GPIO0_OFFSET_START, DA_COMBINE_V3_PAGE_GPIO0_OFFSET_END },
	{ BASE_ADDR_PAGE_GPIO1, DA_COMBINE_V3_PAGE_GPIO1_OFFSET_START, DA_COMBINE_V3_PAGE_GPIO1_OFFSET_END },
	{ BASE_ADDR_PAGE_GPIO2, DA_COMBINE_V3_PAGE_GPIO2_OFFSET_START, DA_COMBINE_V3_PAGE_GPIO2_OFFSET_END },
	{ BASE_ADDR_PAGE_GPIO3, DA_COMBINE_V3_PAGE_GPIO3_OFFSET_START, DA_COMBINE_V3_PAGE_GPIO3_OFFSET_END },

	{ BASE_ADDR_PAGE_I2C_MST, DA_COMBINE_V3_PAGE_I2C_MST_OFFSET_START, DA_COMBINE_V3_PAGE_I2C_MST_OFFSET_END },
	{ BASE_ADDR_PAGE_EDMA, DA_COMBINE_V3_PAGE_EDMA_OFFSET_START, DA_COMBINE_V3_PAGE_EDMA_OFFSET_END },
	{ BASE_ADDR_PAGE_DSPIF, DA_COMBINE_V3_PAGE_DSPIF_OFFSET_START, DA_COMBINE_V3_PAGE_DSPIF_OFFSET_END },
};

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
static struct hicodec_dump_reg_entry dump_table[] = {
	{ "PAGE IO", DA_COMBINE_V3_DBG_PAGE_IO_CODEC_START, DA_COMBINE_V3_DBG_PAGE_IO_CODEC_END, 4 },
	{ "PAGE CFG", DA_COMBINE_V3_DBG_PAGE_CFG_CODEC_START, DA_COMBINE_V3_DBG_PAGE_CFG_CODEC_END, 1 },
	{ "PAGE ANA", DA_COMBINE_V3_DBG_PAGE_ANA_CODEC_START, DA_COMBINE_V3_DBG_PAGE_ANA_CODEC_END, 1 },
	{ "PAGE DIG", DA_COMBINE_V3_DBG_PAGE_DIG_CODEC_START, DA_COMBINE_V3_DBG_PAGE_DIG_CODEC_END, 1 },
};

static struct hicodec_dump_reg_info dump_info = {
	.entry = dump_table,
	.count = sizeof(dump_table) / sizeof(struct hicodec_dump_reg_entry),
};
#endif

static const struct of_device_id platform_match[] = {
	{ .compatible = "hisilicon,da_combine_v3-codec", },
	{ },
};

static bool is_reg_valid(unsigned int reg)
{
	unsigned int val = reg & (~CODEC_BASE_ADDR);
	size_t len = ARRAY_SIZE(reg_info);
	unsigned int begin;
	unsigned int end;
	unsigned int i;

	for (i = 0; i < len; i++) {
		begin = reg_info[i].begin + reg_info[i].offest;
		end = reg_info[i].end + reg_info[i].offest;
		if (val >= begin && val <= end)
			return true;
	}

	AUDIO_LOGE("invalid reg: 0x%x, begin: 0x%x, end: 0x%x\n", val, begin, end);
	return false;
}

static unsigned int virtual_reg_read(struct da_combine_v3_platform_data *platform_data,
	unsigned int addr)
{
	unsigned int ret;

	mutex_lock(&platform_data->rw_mutex);
	switch (addr) {
	case DA_COMBINE_V3_VIRTUAL_REG:
		ret = platform_data->mm_reg;
		break;
	case DA_COMBINE_V3_DACL_PGA_GAIN_VIRTUAL_REG:
		ret = platform_data->virtul_dacl_gain;
		break;
	case DA_COMBINE_V3_DACR_PGA_GAIN_VIRTUAL_REG:
		ret = platform_data->virtul_dacr_gain;
		break;
	case DA_COMBINE_V3_ADC0L_05PGA_GAIN_VIRTUAL_REG:
		ret = platform_data->mic_fixed_value[0];
		break;
	case DA_COMBINE_V3_ADC0R_05PGA_GAIN_VIRTUAL_REG:
		ret = platform_data->mic_fixed_value[1];
		break;
	case DA_COMBINE_V3_ADC1L_05PGA_GAIN_VIRTUAL_REG:
		ret = platform_data->mic_fixed_value[2];
		break;
	case DA_COMBINE_V3_ADC1R_05PGA_GAIN_VIRTUAL_REG:
		ret = platform_data->mic_fixed_value[3];
		break;
	default:
		ret = 0;
		AUDIO_LOGE("virtual reg addr err, addr = 0x%pK",
			(void *)(uintptr_t)addr);
		break;
	}
	mutex_unlock(&platform_data->rw_mutex);

	return ret;
}

static unsigned int reg_read(struct snd_soc_component *codec,
	unsigned int reg)
{
	unsigned int ret = 0;
	unsigned int reg_mask;
	struct da_combine_v3_platform_data *platform_data =
		snd_soc_component_get_drvdata(codec);

	if (!is_reg_valid(reg)) {
		AUDIO_LOGE("invalid reg");
		return 0;
	}

	reg_mask = reg & 0xFFFFF000;
	if (reg_mask == BASE_ADDR_PAGE_CFG || reg_mask == BASE_ADDR_PAGE_IO) {
		reg = reg | CODEC_BASE_ADDR;
	} else if (reg_mask == BASE_ADDR_PAGE_MM) {
		ret = virtual_reg_read(platform_data, reg);
		return ret;
	}

	da_combine_resmgr_request_reg_access(platform_data->resmgr, reg);
	ret = hi_cdcctrl_reg_read(platform_data->cdc_ctrl, reg);
	da_combine_resmgr_release_reg_access(platform_data->resmgr, reg);

	return ret;
}

static unsigned int virtual_reg_write(struct da_combine_v3_platform_data *platform_data,
	unsigned int addr, unsigned int value)
{
	unsigned int ret = 0;

	mutex_lock(&platform_data->rw_mutex);
	switch (addr) {
	case DA_COMBINE_V3_VIRTUAL_REG:
		platform_data->mm_reg = value;
		break;
	case DA_COMBINE_V3_DACL_PGA_GAIN_VIRTUAL_REG:
		platform_data->virtul_dacl_gain = value;
		if (platform_data->hsl_power_on && platform_data->hsr_power_on)
			ret = DA_COMBINE_V3_DACL_PGA_GAIN_CFG_REG | CODEC_BASE_ADDR;
		break;
	case DA_COMBINE_V3_DACR_PGA_GAIN_VIRTUAL_REG:
		platform_data->virtul_dacr_gain = value;
		if (platform_data->hsl_power_on && platform_data->hsr_power_on)
			ret = DA_COMBINE_V3_DACR_PGA_GAIN_CFG_REG | CODEC_BASE_ADDR;
		break;
	case DA_COMBINE_V3_ADC0L_05PGA_GAIN_VIRTUAL_REG:
		platform_data->mic_fixed_value[0] = value;
		ret = DA_COMBINE_V3_ADC0L_05PGA_GAIN_REG | CODEC_BASE_ADDR;
		break;
	case DA_COMBINE_V3_ADC0R_05PGA_GAIN_VIRTUAL_REG:
		platform_data->mic_fixed_value[1] = value;
		ret = DA_COMBINE_V3_ADC0R_05PGA_GAIN_REG | CODEC_BASE_ADDR;
		break;
	case DA_COMBINE_V3_ADC1L_05PGA_GAIN_VIRTUAL_REG:
		platform_data->mic_fixed_value[2] = value;
		ret = DA_COMBINE_V3_ADC1L_05PGA_GAIN_REG | CODEC_BASE_ADDR;
		break;
	case DA_COMBINE_V3_ADC1R_05PGA_GAIN_VIRTUAL_REG:
		platform_data->mic_fixed_value[3] = value;
		ret = DA_COMBINE_V3_ADC1R_05PGA_GAIN_REG | CODEC_BASE_ADDR;
		break;
	default:
		AUDIO_LOGE("virtual reg addr err, addr = 0x%pK",
			(void *)(uintptr_t)addr);
		break;
	}
	mutex_unlock(&platform_data->rw_mutex);

	return ret;
}

static int reg_write(struct snd_soc_component *codec,
	unsigned int reg, unsigned int value)
{
	int ret = 0;
	unsigned int reg_mask;
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	if (!is_reg_valid(reg)) {
		AUDIO_LOGE("invalid reg");
		return 0;
	}

	reg_mask = reg & 0xFFFFF000;
	if (reg_mask == BASE_ADDR_PAGE_CFG || reg_mask == BASE_ADDR_PAGE_IO) {
		reg = reg | CODEC_BASE_ADDR;
	} else if (reg_mask == BASE_ADDR_PAGE_MM) {
		reg = virtual_reg_write(priv, reg, value);
		if (reg == 0)
			return ret;
	} else {
		/* for pclint */
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_reg_rw_cache(reg, value, HICODEC_DEBUG_FLAG_WRITE);
#endif

	da_combine_resmgr_request_reg_access(priv->resmgr, reg);
	ret = hi_cdcctrl_reg_write(priv->cdc_ctrl, reg, value);
	da_combine_resmgr_release_reg_access(priv->resmgr, reg);

	return ret;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
static unsigned int reg_read_by_codec(struct snd_soc_codec *codec,
	unsigned int reg)
{
	return reg_read(&codec->component, reg);
}

static int reg_write_by_codec(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value)
{
	return reg_write(&codec->component, reg, value);
}
#endif

static void ioshare_cdc_ctrl_slimbus_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	if (platform_data->cdc_ctrl->bus_sel == BUSTYPE_SELECT_SLIMBUS) {
		snd_soc_component_write(codec, DA_COMBINE_V3_SSI_DATA_IOSHARE_REG, 0x1);
		snd_soc_component_write(codec, DA_COMBINE_V3_I2S1_SYNC_IOSHARE_REG, 0x2);
		snd_soc_component_write(codec, DA_COMBINE_V3_I2S1_CLK_IOSHARE_REG, 0x2);
		snd_soc_component_write(codec, DA_COMBINE_V3_SLIM_DATA_IOCONFIG_REG, 0x114);
		snd_soc_component_write(codec, DA_COMBINE_V3_SLIM_CLK_IOCONFIG_REG, 0x115);
		/* enable da_combine_v3 slim framer */
		snd_soc_component_write(codec, DA_COMBINE_V3_SLIM_CLK_DIV_REG, 0x53);
		/* slim&ssi mclk enable */
		snd_soc_component_write(codec, DA_COMBINE_V3_CFG_CLK_CTRL, 0x3);
	}
	/* slimbus dport ack share */
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM_CTRL3, 0xBF);
}

static void ioshare_ssi_init(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_SSI_DATA_IOS_IOM_CTRL, 0x109);
	snd_soc_component_write(codec, DA_COMBINE_V3_SSI_DATA_IOSHARE_REG,
		1 << DA_COMBINE_V3_SSI_DATA_EN_BIT);
}

static void ioshare_irq_init(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_IRQ_IOSHARE_REG, 1 << DA_COMBINE_V3_IRQ_EN_BIT);
	snd_soc_component_write(codec, DA_COMBINE_V3_IRQN_IOCONFIG_REG, 0x84);
}

static void ioshare_i2s_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	/* slimbus ioshare config */
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S1_SYNC_IOSHARE_REG,
		1 << DA_COMBINE_V3_SLIM_DATA_EN_BIT);
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S1_CLK_IOSHARE_REG,
		1 << DA_COMBINE_V3_SLIM_CLK_EN_BIT);
	/* I2S2 ioshare config */
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S2_SYNC_IOSHARE_REG,
		1 << DA_COMBINE_V3_I2S2_SYNC_EN_BIT);
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S2_CLK_IOSHARE_REG,
		1 << DA_COMBINE_V3_I2S2_CLK_EN_BIT);
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S2_SDI_IOSHARE_REG,
		1 << DA_COMBINE_V3_I2S2_SDI_EN_BIT);
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S2_SDO_IOSHARE_REG,
		1 << DA_COMBINE_V3_I2S2_SDO_EN_BIT);
	/* I2S3 ioshare config */
	if (platform_data->board_config.extern_hs_hifi_ak4376_I2S3) {
		snd_soc_component_write(codec, DA_COMBINE_V3_I2S3_SYNC_IOSHARE_REG,
			1 << DA_COMBINE_V3_I2S3_SYNC_EN_BIT);
		snd_soc_component_write(codec, DA_COMBINE_V3_I2S3_CLK_IOSHARE_REG,
			1 << DA_COMBINE_V3_I2S3_CLK_EN_BIT);
		snd_soc_component_write(codec, DA_COMBINE_V3_I2S3_SDI_IOSHARE_REG,
			1 << DA_COMBINE_V3_I2S3_SDI_EN_BIT);
		snd_soc_component_write(codec, DA_COMBINE_V3_I2S3_SDO_IOSHARE_REG,
			1 << DA_COMBINE_V3_I2S3_SDO_EN_BIT);
	}
	/* I2S4 ioshare config */
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S4_SYNC_IOSHARE_REG,
		1 << DA_COMBINE_V3_I2S4_SYNC_EN_BIT);
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S4_CLK_IOSHARE_REG,
		1 << DA_COMBINE_V3_I2S4_CLK_EN_BIT);
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S4_SDI_IOSHARE_REG,
		1 << DA_COMBINE_V3_I2S4_SDI_EN_BIT);
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S4_SDO_IOSHARE_REG,
		1 << DA_COMBINE_V3_I2S4_SDO_EN_BIT);
	/* I2S2&I2S4 sdo config */
	da_combine_update_bits(codec, DA_COMBINE_V3_I2S2_SDO_IOCONFIG_REG,
		(1 << DA_COMBINE_V3_I2S_SDO_INPUT_EN_BIT) | (1 << DA_COMBINE_V3_I2S_SDO_OUTPUT_EN_BIT),
		(0 << DA_COMBINE_V3_I2S_SDO_INPUT_EN_BIT) | (1 << DA_COMBINE_V3_I2S_SDO_OUTPUT_EN_BIT));
	da_combine_update_bits(codec, DA_COMBINE_V3_I2S4_SDO_IOCONFIG_REG,
		(1 << DA_COMBINE_V3_I2S_SDO_INPUT_EN_BIT) | (1 << DA_COMBINE_V3_I2S_SDO_OUTPUT_EN_BIT),
		(0 << DA_COMBINE_V3_I2S_SDO_INPUT_EN_BIT) | (1 << DA_COMBINE_V3_I2S_SDO_OUTPUT_EN_BIT));
}

static void ioshare_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	ioshare_cdc_ctrl_slimbus_init(codec, platform_data);
	ioshare_ssi_init(codec);
	ioshare_irq_init(codec);
	ioshare_i2s_init(codec, platform_data);
}

static void slimbus_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	/* slimbus clk schmitt config */
	da_combine_update_bits(codec, DA_COMBINE_V3_SLIM_CLK_IOCONFIG_REG,
		0x3 << DA_COMBINE_V3_SLIM_SCHMITT_BIT, 0x1 << DA_COMBINE_V3_SLIM_SCHMITT_BIT);
	/* slimbus pin disable pd */
	da_combine_update_bits(codec, DA_COMBINE_V3_SLIM_DATA_IOCONFIG_REG,
		0x1 << DA_COMBINE_V3_IOSHARE_PIN_PD, 0x0 << DA_COMBINE_V3_IOSHARE_PIN_PD);
	da_combine_update_bits(codec, DA_COMBINE_V3_SLIM_CLK_IOCONFIG_REG,
		0x1 << DA_COMBINE_V3_IOSHARE_PIN_PD, 0x0 << DA_COMBINE_V3_IOSHARE_PIN_PD);
	/* slimbus drv codec side */
	da_combine_update_bits(codec, DA_COMBINE_V3_SLIM_DATA_IOCONFIG_REG,
		0x7 << DA_COMBINE_V3_SLIM_DATA_IO_CURRENT_BIT,
		platform_data->cdc_ctrl->slimbusdata_cdc_drv << DA_COMBINE_V3_SLIM_DATA_IO_CURRENT_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_SLIM_CLK_IOCONFIG_REG,
		0x7 << DA_COMBINE_V3_SLIM_CLK_IO_CURRENT_BIT,
		platform_data->cdc_ctrl->slimbusclk_cdc_drv << DA_COMBINE_V3_SLIM_CLK_IO_CURRENT_BIT);
}

static void gpio_pd_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	if (platform_data->board_config.gpio_pd_enable) {
		/* GPIO0 pd enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_GPIO0_IOCONFIG_REG,
			0x40, 0x40);
		/* 12.228M GPIO pd enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_12M288_IOCONFIG_REG,
			0x40, 0x40);
	}
}

static void dsd_init(struct snd_soc_component *codec)
{
	/* dsd clk disable */
	snd_soc_component_write(codec, DA_COMBINE_V3_DSD_CLK_CFG_REG0, 0xC0);
	snd_soc_component_write(codec, DA_COMBINE_V3_DSD_CLK_CFG_REG1, 0x80);
}

static void mic_voltage_init(struct snd_soc_component *codec)
{
	/* SPK CLK GPIO pd disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_SPK_CLK_IOCONFIG_REG, 0x40, 0x0);

	/* mic voltage config */
	snd_soc_component_write(codec, DA_COMBINE_V3_MIC_3_4_CONFIG_REG, 0xBB);
	snd_soc_component_write(codec, DA_COMBINE_V3_MIC_1_2_CONFIG_REG, 0xBB);
	snd_soc_component_write(codec, DA_COMBINE_V3_HSMIC_CONFIG_REG, 0xB);
}

static void headphone_pop_init(struct snd_soc_component *codec)
{
	/* headphone irq ctrl -> reg */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG43,
		0x1 << DA_COMBINE_V3_HSD_IRQ_LINK_DIS_BIT,
		0x1 << DA_COMBINE_V3_HSD_IRQ_LINK_DIS_BIT);
	/* headphone pop pull set not 0 */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG43,
		0x3F << DA_COMBINE_V3_HPL_POP_PULL_2_0_BIT,
		0x8 << DA_COMBINE_V3_HPL_POP_PULL_2_0_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG44,
		0x3F << DA_COMBINE_V3_HPR_POP_PULL_2_0_BIT,
		0x8 << DA_COMBINE_V3_HPR_POP_PULL_2_0_BIT);

	/* headphone pop config */
	snd_soc_component_write(codec, DA_COMBINE_V3_HPL_POP_CFG_REG, 0x7F);
	snd_soc_component_write(codec, DA_COMBINE_V3_HPR_POP_CFG_REG, 0x7F);
	/* pop auto enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG47,
		1 << DA_COMBINE_V3_HP_POP_AUTO_EN_BIT, 1 << DA_COMBINE_V3_HP_POP_AUTO_EN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG47, 0x3, 0x2);
	/* pop clk power on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG15,
		1 << DA_COMBINE_V3_POP_CLK_PD_BIT, 0);
	/* pop clk rst */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG47,
		1 << DA_COMBINE_V3_HP_POP_RST_BIT, 0);
	/* headphone dac mute disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG2,
		0x3 << DA_COMBINE_V3_DAC_HPL_MUTE_BIT, 0);
}

static void cp_bunk_init(struct snd_soc_component *codec)
{
	/* ISO config, bit1 change with 1do8 */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG97,
		0x3 << DA_COMBINE_V3_ISO_A33_BIT, 0x3 << DA_COMBINE_V3_ISO_A33_BIT);
	/* cp1 catch buck1 */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG108,
		1 << DA_COMBINE_V3_CP1_EN_SEL_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG130,
		1 << DA_COMBINE_V3_BUCK1_PD_SINGLE_BIT, 0);
	/* cp2 catch buck2 */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG121,
		1 << DA_COMBINE_V3_CP2_EN_SEL_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG130,
		1 << DA_COMBINE_V3_BUCK2_PD_SINGLE_BIT, 0);
}

static void set_freq_init(struct snd_soc_component *codec)
{
	/* I2S4 config (fs init 48k) */
	snd_soc_component_write(codec, DA_COMBINE_V3_S4_CFG_REG_H, 0x70);
	snd_soc_component_write(codec, DA_COMBINE_V3_S4_CLK_CFG_REG, 0x4);
	snd_soc_component_write(codec, DA_COMBINE_V3_S4_CLK_CFG_REG1, 0x44);
	/* slimbus down1&2 port fs config */
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM0_CLK_CFG_REG, 0x44);
	/* slimbus down3&4port fs config */
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM1_CLK_CFG_REG, 0x44);
	/* slimbus up1&2 port fs config */
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM3_CLK_CFG_REG, 0x44);
	/* slimbus up3&4 port fs config */
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM4_CLK_CFG_REG, 0x44);
	/* slimbus up5&6 port fs config */
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM5_CLK_CFG_REG, 0x11);
	/* slimbus up7&8 port fs config */
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM6_CLK_CFG_REG, 0x44);
	/* I2S/SLIM SEL */
	snd_soc_component_write(codec, DA_COMBINE_V3_I2S_SLIM_SEL_REG, 0x33);
	/* adc source select */
	snd_soc_component_write(codec, DA_COMBINE_V3_ADC_SOURCE_SEL, 0x1f);
	/* mixer fs config */
	da_combine_update_bits(codec, DA_COMBINE_V3_DACL_FLT_MIXER_SEL,
		0x1 << DA_COMBINE_V3_DACL_FLT_MIXER_FS_CFG_BIT,
		0x1 << DA_COMBINE_V3_DACL_FLT_MIXER_FS_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACR_FLT_MIXER_SEL,
		0x1 << DA_COMBINE_V3_DACR_FLT_MIXER_FS_CFG_BIT,
		0x1 << DA_COMBINE_V3_DACR_FLT_MIXER_FS_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACSL_MIXER_SEL_1,
		0x1 << DA_COMBINE_V3_DACSL_MIXER_FS_BIT,
		0x1 << DA_COMBINE_V3_DACSL_MIXER_FS_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACSR_MIXER_SEL_1,
		0x1 << DA_COMBINE_V3_DACSR_MIXER_FS_BIT,
		0x1 << DA_COMBINE_V3_DACSR_MIXER_FS_BIT);
}

static void dsp_if_config_init(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, DA_COMBINE_V3_S1_DSPIF_CONTROL_REG,
		0x3 << DA_COMBINE_V3_S1_DSPIF_IN_BIT, 0x3 << DA_COMBINE_V3_S1_DSPIF_IN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_S2_DSPIF_CONTROL_REG,
		0x3 << DA_COMBINE_V3_S2_DSPIF_IN_BIT, 0x3 << DA_COMBINE_V3_S2_DSPIF_IN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_S3_DSPIF_CONTROL_REG,
		0x3 << DA_COMBINE_V3_S3_DSPIF_IN_BIT, 0x3 << DA_COMBINE_V3_S3_DSPIF_IN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_S4_DSPIF_CONTROL_REG,
		0x3 << DA_COMBINE_V3_S4_DSPIF_IN_BIT, 0x3 << DA_COMBINE_V3_S4_DSPIF_IN_BIT);
	/* slimbus frame config */
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM_CLK_DIV_REG, 0x53);
}

static void dac_mux_init(struct snd_soc_component *codec)
{
	/* dac src mux init */
	snd_soc_component_write(codec, DA_COMBINE_V3_DAC_SRC_MUX_SEL, 0x88);
	/* mux for sdm init */
	snd_soc_component_write(codec, DA_COMBINE_V3_DAC_HP_MUX_SEL, 0x1B);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACS_MUX_SEL, 0x36);
	snd_soc_component_write(codec, DA_COMBINE_V3_DAC_LTR_MUX_SEL, 0x6);
}

static void headphone_conifg_init(struct snd_soc_component *codec)
{
	/* headphone detect config */
	snd_soc_component_write(codec, DA_COMBINE_V3_HP_DET_CFG_REG, 0x9);
	/* headphone dac VB current select -> 80uA */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG38_SHADOW,
		0x3 << DA_COMBINE_V3_DAC_HP_VBR_SEL_SHADOW_BIT,
		0x1 << DA_COMBINE_V3_DAC_HP_VBR_SEL_SHADOW_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG38,
		0x3 << DA_COMBINE_V3_DAC_HP_VBR_SEL_BIT,
		0x1 << DA_COMBINE_V3_DAC_HP_VBR_SEL_BIT);
	/* hp_ibct -> 5uA */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
		0x11 << DA_COMBINE_V3_HP_IBCT_BIT, 0x0 << DA_COMBINE_V3_HP_IBCT_BIT);
	/* headphone feed back res select -> 1.25k */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG49,
		0x7 << DA_COMBINE_V3_HP_FB_RES_CT_BIT, 0x2 << DA_COMBINE_V3_HP_FB_RES_CT_BIT);
	/* dac_hp_high_celloff enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG40,
		0x3 << DA_COMBINE_V3_DAC_HPR_HI_CELLOFF_EN,
		0x0 << DA_COMBINE_V3_DAC_HPR_HI_CELLOFF_EN);
	/* headphone ICELL VB enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG39,
		0x1 << DA_COMBINE_V3_DAC_HP_VBSWEN_BIT,
		0x0 << DA_COMBINE_V3_DAC_HP_VBSWEN_BIT);
}

static void mixer_pga_gain_init(struct snd_soc_component *codec)
{
	/* mixer gian config */
	snd_soc_component_write(codec, DA_COMBINE_V3_DACL_MIXER_GAIN_CFG_REG, 0xFF);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACR_MIXER_GAIN_CFG_REG, 0xFF);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACSL_MIXER_GAIN_CFG_REG, 0xFF);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACSR_MIXER_GAIN_CFG_REG, 0xFF);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACL_SRC_MIXER_GAIN_REG, 0xF);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACR_SRC_MIXER_GAIN_REG, 0xF);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACL_FLT_MIXER_GAIN_CFG_REG, 0xF, 0xF);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACR_FLT_MIXER_GAIN_CFG_REG, 0xF, 0xF);
}

static void classh_init(struct snd_soc_component *codec)
{
	/* class A/B config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CLASS_H_CFG_REG0, 0x13);
	snd_soc_component_write(codec, DA_COMBINE_V3_CLASS_H_CFG_REG1, 0x37);
}

static void sdm_config_init(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, DA_COMBINE_V3_HPL_SDM45_CONFIG_REG0, 0x3F, 0x28);
	da_combine_update_bits(codec, DA_COMBINE_V3_HPR_SDM45_CONFIG_REG0, 0x3F, 0x28);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACL_SDM45_MUX_SEL, 0xFC, 0x0);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACR_SDM45_MUX_SEL, 0xFC, 0x0);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACL_SDM_MUX_SEL, 0x8, 0x8);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACR_SDM_MUX_SEL, 0x8, 0x8);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACSL_SDM_MUX_SEL, 0x8, 0x8);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACSR_SDM_MUX_SEL, 0x8, 0x8);
	snd_soc_component_write(codec, DA_COMBINE_V3_HPL_SDM_NEW_CTRL5_REG, 0x69);
	snd_soc_component_write(codec, DA_COMBINE_V3_HPR_SDM_NEW_CTRL5_REG, 0x69);
}

static void mic_pga_gain_init(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG23,
		0x1F << DA_COMBINE_V3_MAIN_PGA_GAIN_BIT,
		0x11 << DA_COMBINE_V3_MAIN_PGA_GAIN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG22,
		0x1F << DA_COMBINE_V3_AUX_PGA_GAIN_BIT,
		0x11 << DA_COMBINE_V3_AUX_PGA_GAIN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG21,
		0x1F << DA_COMBINE_V3_LINEINR_PGA_GAIN_BIT,
		0x11 << DA_COMBINE_V3_LINEINR_PGA_GAIN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG20,
		0x1F << DA_COMBINE_V3_LINEINL_PGA_GAIN_BIT,
		0x11 << DA_COMBINE_V3_LINEINL_PGA_GAIN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG84,
		0x1F << DA_COMBINE_V3_MAD_PGA_GAIN_BIT,
		0x11 << DA_COMBINE_V3_MAD_PGA_GAIN_BIT);
}

static void pga05_gain_init(struct snd_soc_component *codec)
{
	/* 0.5dB step pga gain init -> 0dB */
	snd_soc_component_write(codec, DA_COMBINE_V3_DACL_PGA_GAIN_CFG_REG, 0x6E);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACR_PGA_GAIN_CFG_REG, 0x6E);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACSL_PGA_GAIN_CFG_REG, 0x78);
	snd_soc_component_write(codec, DA_COMBINE_V3_ADC0L_05PGA_GAIN_REG, 0x78);
	snd_soc_component_write(codec, DA_COMBINE_V3_ADC0R_05PGA_GAIN_REG, 0x78);
	snd_soc_component_write(codec, DA_COMBINE_V3_ADC1L_05PGA_GAIN_REG, 0x78);
	snd_soc_component_write(codec, DA_COMBINE_V3_ADC1R_05PGA_GAIN_REG, 0x78);
}

static void digital_pga_gain_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	/* digital pga fade mode */
	if (platform_data->board_config.fm_enable)
		snd_soc_component_write(codec, DA_COMBINE_V3_DIG_PGA_FADE_MODE_CFG_REG, 0xF3);
	else
		snd_soc_component_write(codec, DA_COMBINE_V3_DIG_PGA_FADE_MODE_CFG_REG, 0xFF);
}

static void cp_bunk_config_init(struct snd_soc_component *codec)
{
	/* I2S1 config fs init 48k */
	da_combine_update_bits(codec, DA_COMBINE_V3_S1_CLK_CFG_REG0, 0x7, 0x4);
	/* S1 DSPIF */
	snd_soc_component_write(codec, DA_COMBINE_V3_S1_CLK_CFG_REG1, 0x44);
	/* disable i2c master clk */
	da_combine_update_bits(codec, DA_COMBINE_V3_DSP_PERI_CLKEN1,
		0x1 << DA_COMBINE_V3_I2C_MST_CLK_EN_BIT, 0);
	/* buck pll dig clk select 19M2 */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG200, 0x40, 0x0);
	/* buck1 ramp config */
	da_combine_update_bits(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_107,
		0x3 << DA_COMBINE_V3_BUCK1_REG_R, 0x0 << DA_COMBINE_V3_BUCK1_REG_R);
	/* config buck1 dmd */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_102, 0x61);
	/* buck1 bandwidth config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_99, 0x2F);
	/* buck1 config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_100, 0x18);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_101, 0xDE);
	/* buck2 ramp config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_109, 0x22);
	/* config buck2 dmd */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_116, 0xD5);
	/* buck2 bandwidth config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_113, 0xFC);
	/* buck2 config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_114, 0x9A);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_115, 0xA2);
	/* buck1 Vout config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_137, 0x76);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_138, 0x41);
	/* buck1 clk div select */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_139, 0x86);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_140, 0x14);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_141, 0x12);
	/* buck1 mos config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_142, 0x66);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_143, 0xBB);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_144, 0x66);
	/* cp1 Vout config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_145, 0xA7);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_146, 0x42);
	/* cp1 config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_147, 0xA9);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_148, 0x3A);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_132, 0x23);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_133, 0x1C);
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_134, 0x0E);
	/* buck2 config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_135, 0x07);
	/* cp2 config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_136, 0x07);
	/* buck1 pull down disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG111, 0x80, 0x80);
	da_combine_update_bits(codec, DA_COMBINE_V3_HPL_SDM45_CONFIG_REG1, 0xF, 0xF);
	snd_soc_component_write(codec, DA_COMBINE_V3_HPL_SDM45_CONFIG_REG2, 0xC4);
	da_combine_update_bits(codec, DA_COMBINE_V3_HPR_SDM45_CONFIG_REG1, 0xF, 0xF);
	snd_soc_component_write(codec, DA_COMBINE_V3_HPR_SDM45_CONFIG_REG2, 0xC4);
}

static void ib05_config_init(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG70, 0x66);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG71, 0x66);
	if (snd_soc_component_read32(codec, DA_COMBINE_V3_EC_VERSION_REG) & 0x1)
		snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG72, 0x62);
	else
		snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG72, 0x6F);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x66);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG74, 0x66);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG75, 0x66);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG76, 0x66);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG77, 0x66);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG78, 0x62);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG79, 0x22);

	/* ib05 config */
	snd_soc_component_write(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_80, 0x66);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG81, 0x66);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG82, 0x1);

	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG30, 0x38);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG32, 0x38);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG33, 0x38);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG35, 0x38);

}

static void classh_config_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_CLASSH_HP_EP_CTRL, 0x30);

	if (platform_data->board_config.classh_rcv_hp_switch != 0)
		platform_data->rcv_hp_classh_state =
			(unsigned int)platform_data->rcv_hp_classh_state | RCV_CLASSH_STATE;
	else
		platform_data->rcv_hp_classh_state =
			(unsigned int)platform_data->rcv_hp_classh_state & (~RCV_CLASSH_STATE);

	set_classh_config(codec, platform_data->rcv_hp_classh_state);
	/* g2_hp -> g2 */
	da_combine_update_bits(codec, DA_COMBINE_V3_CLASSH_CTRL4,
		0x1 << DA_COMBINE_V3_CLASSH_SELD_BIT, 0x0 << DA_COMBINE_V3_CLASSH_SELD_BIT);
	/* classH fall delay enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_CLASSH_CTRL12,
		0x1 << DA_COMBINE_V3_CLASSH_FALL_DLY_EN_BIT,
		0x1 << DA_COMBINE_V3_CLASSH_FALL_DLY_EN_BIT);
	/* classH sync enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_CLASSH_SYNC_CTRL_REG,
		0x1 << DA_COMBINE_V3_CLASSH_SYNC_CTRL_BIT,
		0x1 << DA_COMBINE_V3_CLASSH_SYNC_CTRL_BIT);
	/* classA/B -> classH  */
	da_combine_update_bits(codec, DA_COMBINE_V3_CLASS_H_CFG_REG1,
		0x1 << DA_COMBINE_V3_CLASS_H_SEL_BIT, 0x1 << DA_COMBINE_V3_CLASS_H_SEL_BIT);
	/* hp typeb ctrl config */
	da_combine_update_bits(codec, DA_COMBINE_V3_HP_SW_CTRL10_REG,
		0x1 << DA_COMBINE_V3_HP_SW_TYPEB_CTRL_BIT,
		0x1 << DA_COMBINE_V3_HP_SW_TYPEB_CTRL_BIT);
	/* hp sw config */
	snd_soc_component_write(codec, DA_COMBINE_V3_HP_SW_CTRL1_REG, 0x4);
	snd_soc_component_write(codec, DA_COMBINE_V3_HP_SW_CTRL4_REG, 0xC);

	platform_data->rcv_hp_classh_state =
		(unsigned int)platform_data->rcv_hp_classh_state | HP_CLASSH_STATE;
}

static void dac_pga_gain_init(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_DACL_PGA_GAIN_CFG_REG, 0xBA);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACR_PGA_GAIN_CFG_REG, 0xBA);

	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG13,
		0x3 << DA_COMBINE_V3_DAC_CHOP_CLK_SEL_BIT,
		0x0 << DA_COMBINE_V3_DAC_CHOP_CLK_SEL_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG14,
		0x3F << DA_COMBINE_V3_CHOPPER_BIT, 0x3F << DA_COMBINE_V3_CHOPPER_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG15,
		0x1 << DA_COMBINE_V3_DAC_LOUT_CHOP_BPS_BIT,
		0x1 << DA_COMBINE_V3_DAC_LOUT_CHOP_BPS_BIT);
	/* imp_det enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_IMP_DET_CTRL_REG,
		0x1F << DA_COMBINE_V3_IMP_SAMP_CFG, 0x6 << DA_COMBINE_V3_IMP_SAMP_CFG);
	/* detres sel -> 300 */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG50,
		0x1 << DA_COMBINE_V3_DETRES_SEL_BIT, 0x1 << DA_COMBINE_V3_DETRES_SEL_BIT);
	/* mic dischg disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
		0x1F << DA_COMBINE_V3_MIC4_DISCHG_EN_BIT, 0);
	/* dac pga fade enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DACL_PGA_CFG_REG,
		0x1 << DA_COMBINE_V3_DACL_PGA_FD_EN_BIT,
		0x1 << DA_COMBINE_V3_DACL_PGA_FD_EN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACR_PGA_CFG_REG,
		0x1 << DA_COMBINE_V3_DACR_PGA_FD_EN_BIT,
		0x1 << DA_COMBINE_V3_DACR_PGA_FD_EN_BIT);
	/* s3 IL pga fade enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_S3_PGA_IL_CFG_REG,
		0x1 << DA_COMBINE_V3_S3_PGA_IL_FD_EN_BIT,
		0x1 << DA_COMBINE_V3_S3_PGA_IL_FD_EN_BIT);
	/* zero cross config for classH */
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL3, 0x00);
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL2, 0x00);
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL1, 0x80);
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL0, 0x00);
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL5, 0x00);
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL4, 0x80);
	/*digital pga bypass config*/
	snd_soc_component_write(codec, DA_COMBINE_V3_PGA_BYPASS_REG0, 0x33);
	snd_soc_component_write(codec, DA_COMBINE_V3_PGA_BYPASS_REG1, 0x30);
	snd_soc_component_write(codec, DA_COMBINE_V3_PGA_BYPASS_REG2, 0x00);
}

static void hsd_cfg_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	/* set hsd_vth -> 1.65v */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG98,
		0x3 << DA_COMBINE_V3_HSD_VTH_SEL_BIT, 0x2 << DA_COMBINE_V3_HSD_VTH_SEL_BIT);

	if (platform_data->board_config.hs_det_trigger_func == SCHMIT_FUNC)
		/* set hsd -> schmitt */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG98,
			0x1 << DA_COMBINE_V3_HSD_SEL_BIT, 0x0 << DA_COMBINE_V3_HSD_SEL_BIT);
	else
		/* set hsd -> CMP */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG98,
			0x1 << DA_COMBINE_V3_HSD_SEL_BIT, 0x1 << DA_COMBINE_V3_HSD_SEL_BIT);

	/* hsd cfg */
	if (platform_data->board_config.hsd_cfg_enable != 0)
		snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG98,
			platform_data->board_config.hsd_cfg_value);
}

static void pga_zero_cross_init(struct snd_soc_component *codec)
{
	/* init config for cmp */
	snd_soc_component_write(codec, DA_COMBINE_V3_MBHD_COMP_CFG_REG, 0x1E);

	/* digital pga zc point -> 0 */
	da_combine_update_bits(codec, DA_COMBINE_V3_PGA_ZC_CFG_REG_0,
		0x1F << DA_COMBINE_V3_PGA_ZC_CFG_BIT, 0x0 << DA_COMBINE_V3_PGA_ZC_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_PGA_ZC_CFG_REG_1,
		0x1F << DA_COMBINE_V3_PGA_ZC_CFG_BIT, 0x0 << DA_COMBINE_V3_PGA_ZC_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_PGA_ZC_CFG_REG_2,
		0x1F << DA_COMBINE_V3_PGA_ZC_CFG_BIT, 0x0 << DA_COMBINE_V3_PGA_ZC_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_PGA_ZC_CFG_REG_3,
		0x1F << DA_COMBINE_V3_PGA_ZC_CFG_BIT, 0x0 << DA_COMBINE_V3_PGA_ZC_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_PGA_ZC_CFG_REG_4,
		0x1F << DA_COMBINE_V3_PGA_ZC_CFG_BIT, 0x0 << DA_COMBINE_V3_PGA_ZC_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_PGA_ZC_CFG_REG_5,
		0x1F << DA_COMBINE_V3_PGA_ZC_CFG_BIT, 0x0 << DA_COMBINE_V3_PGA_ZC_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_PGA_ZC_CFG_REG_6,
		0x1F << DA_COMBINE_V3_PGA_ZC_CFG_BIT, 0x0 << DA_COMBINE_V3_PGA_ZC_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_PGA_ZC_CFG_REG_7,
		0x1F << DA_COMBINE_V3_PGA_ZC_CFG_BIT, 0x0 << DA_COMBINE_V3_PGA_ZC_CFG_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_PGA_ZC_CFG_REG_8,
		0x1F << DA_COMBINE_V3_PGA_ZC_CFG_BIT, 0x0 << DA_COMBINE_V3_PGA_ZC_CFG_BIT);
}

static void chip_config(struct snd_soc_component *codec)
{
	/* chip init, no need deint; donnot apply resource */
	struct da_combine_v3_platform_data *platform_data =
		snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

	ioshare_init(codec, platform_data);
	slimbus_init(codec, platform_data);
	gpio_pd_init(codec, platform_data);
	dsd_init(codec);
	mic_voltage_init(codec);
	supply_pll_init(codec);
	headphone_pop_init(codec);
	cp_bunk_init(codec);
	set_freq_init(codec);
	dsp_if_config_init(codec);
	dac_mux_init(codec);
	headphone_conifg_init(codec);
	mixer_pga_gain_init(codec);
	classh_init(codec);
	sdm_config_init(codec);
	mic_pga_gain_init(codec);
	pga05_gain_init(codec);
	digital_pga_gain_init(codec, platform_data);
	cp_bunk_config_init(codec);
	ib05_config_init(codec);
	classh_config_init(codec, platform_data);
	hp_classh_init(codec, platform_data);
	dac_pga_gain_init(codec);
	hsd_cfg_init(codec, platform_data);
	pga_zero_cross_init(codec);

	OUT_FUNCTION;
}

static void set_mad_param(struct snd_soc_component *codec,
	struct board_cfg *board_cfg)
{
	/* auto active time */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_AUTO_ACT_TIME_L, 0x0);

	/* pll time */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_TIME_L, 0x1);

	/* adc time */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_ADC_TIME_H, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_ADC_TIME_L, 0x3);

	/* mad_ana_time */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_ANA_TIME_H, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_ANA_TIME, 0x5);

	/* omt */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_OMIT_SAMP, 0x20);

	/* mad_vad_time */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_VAD_TIME_H, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_VAD_TIME_L, 0xa0);

	/* mad_sleep_time */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_SLEEP_TIME_L, 0x0);

	/* mad_buffer_fifo_thre */
	if (board_cfg->wakeup_audio_algo_support)
		snd_soc_component_write(codec, DA_COMBINE_V3_MAD_BUFFER_CTRL0, 0x3f);
	else
		snd_soc_component_write(codec, DA_COMBINE_V3_MAD_BUFFER_CTRL0, 0x7f);

	da_combine_update_bits(codec, DA_COMBINE_V3_MAD_BUFFER_CTRL1, 0x1f, 0x1f);

	/* mad_cnt_thre,vad delay cnt */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_CNT_THRE, 0x2);

	/* mad_snr_thre */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_SNR_THRE_SUM, 0x32);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_SNR_THRE, 0x20);

	/* mad_min_chan_eng */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_MIN_CHAN_ENG, 0x14);

	/* mad_ine */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_INE, 0x14);
	/* mad_band_thre */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_BAND_THRE, 0x8);
	/* mad_scale */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_SCALE, 0x3);

	/* mad_vad_num */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_VAD_NUM, 0x1);
	/* mad_alpha_en1 */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_ALPHA_EN1, 0xc);

	/* mad_vad_ao ->en, mad_irq_en->en, mad_en->en, mad_wind_sel */
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_CTRL, 0x63);

	/* clear mic1_src_fifo */
	snd_soc_component_write(codec, DA_COMBINE_V3_MIC1_SRCDN_CTRL, 0x8);
	snd_soc_component_write(codec, DA_COMBINE_V3_MIC1_SRCDN_CTRL, 0x0);
	/* mic2_src_fifo_clr */
	snd_soc_component_write(codec, DA_COMBINE_V3_MIC2_SRCDN_CTRL, 0x8);
	snd_soc_component_write(codec, DA_COMBINE_V3_MIC2_SRCDN_CTRL, 0x0);
}

static int utils_init(struct da_combine_v3_platform_data *platform_data)
{
	int ret;
	struct utils_config cfg;

	cfg.da_combine_dump_reg = NULL;
	ret = da_combine_utils_init(platform_data->codec, platform_data->cdc_ctrl,
		&cfg, platform_data->resmgr, DA_COMBINE_CODEC_TYPE_V3);

	return ret;
}

static int slim_enumerate(struct da_combine_v3_platform_data *data)
{
	int ret = 0;

	if (!data->cdc_ctrl->pm_runtime_support)
		return ret;

	/* open codec pll and asp clk to make sure codec framer be enumerated */
	ret = da_combine_resmgr_request_pll(data->resmgr, PLL_HIGH);
	if (ret != 0) {
		AUDIO_LOGE("request pll failed");
		return ret;
	}
	ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
		SLIMBUS_TRACK_AUDIO_PLAY, NULL);
	if (ret != 0)
		AUDIO_LOGE("slimbus_activate_track audio play failed");
	usleep_range(1000, 1100);
	ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
		SLIMBUS_TRACK_AUDIO_PLAY, NULL);
	if (ret != 0)
		AUDIO_LOGE("deactivate audio play failed");
	ret = da_combine_resmgr_release_pll(data->resmgr, PLL_HIGH);
	if (ret != 0)
		AUDIO_LOGE("release pll failed");
	return ret;
}

static int codec_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *data)
{
	int ret;

	chip_config(codec);

	ret = hs_cfg_init(codec, data);
	if (ret != 0) {
		AUDIO_LOGE("hs cfg init failed:0x%x", ret);
		goto hs_cfg_init_failed;
	}

	set_mad_param(codec, &data->board_config);

	ret = dsp_config_init(codec, data->resmgr,
		data->irqmgr, data->cdc_ctrl->bus_sel);
	if (ret != 0) {
		AUDIO_LOGE("init failed:0x%x", ret);
		goto misc_init_failed;
	}

	ret = utils_init(data);
	if (ret != 0) {
		AUDIO_LOGE("utils init failed:0x%x", ret);
		goto utils_init_failed;
	}

	ret = da_combine_vad_init(codec, data->irqmgr);
	if (ret != 0) {
		AUDIO_LOGE("vad init failed:0x%x", ret);
		goto vad_init_failed;
	}

	ret = slim_enumerate(data);
	if (ret != 0) {
		AUDIO_LOGE("slim enumerate failed:0x%x", ret);
		goto slim_enumerate_failed;
	}

	return ret;

slim_enumerate_failed:
	da_combine_vad_deinit(data->node);
vad_init_failed:
	da_combine_utils_deinit();
utils_init_failed:
	hifi_config_deinit();
misc_init_failed:
	hs_cfg_deinit(data->mbhc);
hs_cfg_init_failed:
	return ret;
}

static int codec_add_driver_resource(struct snd_soc_component *codec)
{
	int ret = add_kcontrol(codec);

	if (ret != 0) {
		AUDIO_LOGE("add kcontrols failed, ret = %d", ret);
		return ret;
	}

	ret = add_resource_widgets(codec);
	if (ret != 0) {
		AUDIO_LOGE("add resource widgets failed, ret = %d", ret);
		return ret;
	}

	ret = add_path_widgets(codec);
	if (ret != 0) {
		AUDIO_LOGE("add path widgets failed, ret = %d", ret);
		return ret;
	}

	ret = add_switch_widgets(codec);
	if (ret != 0) {
		AUDIO_LOGE("add switch widgets failed, ret = %d", ret);
		return ret;
	}

	ret = add_routes(codec);
	if (ret != 0) {
		AUDIO_LOGE("add route map failed, ret = %d", ret);
		return ret;
	}

	return ret;
}

static void codec_deinit(struct da_combine_v3_platform_data *data)
{
	da_combine_vad_deinit(data->node);
	da_combine_utils_deinit();
	hifi_config_deinit();
	da_combine_mbhc_deinit(data->mbhc);
}

static int codec_probe(struct snd_soc_component *codec)
{
	int ret;
	struct da_combine_v3_platform_data *platform_data =
		snd_soc_component_get_drvdata(codec);

	if (platform_data == NULL) {
		AUDIO_LOGE("get platform data failed");
		return -ENOENT;
	}

	snd_soc_component_set_drvdata(codec, platform_data);
	platform_data->codec = codec;

	mutex_init(&platform_data->rw_mutex);
	mutex_init(&platform_data->impdet_dapm_mutex);

	platform_data->dp_clk_num = 0;
	platform_data->cp1_num = 0;
	platform_data->cp2_num = 0;

	ret = resmgr_init(platform_data);
	if (ret != 0) {
		AUDIO_LOGE("resmgr init failed:0x%x", ret);
		goto resmgr_init_err_exit;
	}

	AUDIO_LOGI("version:%#x", snd_soc_component_read32(codec, DA_COMBINE_V3_VERSION_REG));

	ret = codec_init(codec, platform_data);
	if (ret != 0) {
		AUDIO_LOGE("probe failed");
		goto codec_init_err_exit;
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	ret = hicodec_debug_init(codec, &dump_info);
	if (ret != 0)
		AUDIO_LOGI("debug init failed: 0x%x", ret);
#endif

	ret = codec_add_driver_resource(codec);
	if (ret != 0) {
		AUDIO_LOGE("add driver resource fail: %d", ret);
		goto driver_resource_init_err_exit;
	}

	AUDIO_LOGI("probe ok");
	return ret;

driver_resource_init_err_exit:
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif
	codec_deinit(platform_data);
codec_init_err_exit:
	da_combine_resmgr_deinit(platform_data->resmgr);
resmgr_init_err_exit:
	mutex_destroy(&platform_data->rw_mutex);
	mutex_destroy(&platform_data->impdet_dapm_mutex);

	AUDIO_LOGE("probe init fail");

	return ret;
}

static void codec_remove(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *platform_data =
		snd_soc_component_get_drvdata(codec);

	if (platform_data == NULL) {
		AUDIO_LOGE("get platform data failed");
		return;
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif

	codec_deinit(platform_data);
	da_combine_resmgr_deinit(platform_data->resmgr);
	mutex_destroy(&platform_data->rw_mutex);
	mutex_destroy(&platform_data->impdet_dapm_mutex);

	return;
}

static int audio_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	int ret = 0;
	int rate;

	if (params == NULL) {
		AUDIO_LOGE("pcm params is null");
		return -EINVAL;
	}

	if (dai == NULL) {
		AUDIO_LOGE("soc dai is null");
		return -EINVAL;
	}

	rate = params_rate(params);
	switch (rate) {
	case 8000:
	case 11025:
	case 16000:
	case 22050:
	case 32000:
	case 44100:
	case 48000:
	case 88200:
	case 96000:
	case 176400:
	case 192000:
		break;
	case 384000:
		AUDIO_LOGE("rate: %d", rate);
		break;
	default:
		AUDIO_LOGE("unknown rate: %d", rate);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int audio_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

struct snd_soc_dai_ops audio_dai_ops = {
	.hw_params = audio_hw_params,
	.hw_free = audio_hw_free,
};

static int voice_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	int ret = 0;
	int rate;

	if (params == NULL) {
		AUDIO_LOGE("pcm params is null");
		return -EINVAL;
	}

	if (dai == NULL) {
		AUDIO_LOGE("soc dai is null");
		return -EINVAL;
	}

	rate = params_rate(params);
	switch (rate) {
	case 8000:
	case 16000:
	case 32000:
		break;
	default:
		AUDIO_LOGE("unknown rate: %d", rate);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int voice_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct snd_soc_component *codec = dai->component;

	da_combine_update_bits(codec, DA_COMBINE_V3_S3_CLK_CFG_REG1,
		VOICE_HW_FREE_DATA, 0);

	AUDIO_LOGI("free voice");

	return 0;
}

struct snd_soc_dai_ops voice_dai_ops = {
	.hw_params = voice_hw_params,
	.hw_free = voice_hw_free,
};

struct snd_soc_dai_driver dai[] = {
	{
		.name = DAI_LINK_CODEC_DAI_NAME_AUDIO,
		.playback = {
			.stream_name = "Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = RATES,
			.formats = FORMATS },
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 6,
			.rates = RATES,
			.formats = FORMATS },
		.ops = &audio_dai_ops,
	},
	{
		.name = DAI_LINK_CODEC_DAI_NAME_VOICE,
		.playback = {
			.stream_name = "Down",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RATES,
			.formats = FORMATS },
		.capture = {
			.stream_name = "Up",
			.channels_min = 1,
			.channels_max = 6,
			.rates = RATES,
			.formats = FORMATS },
		.ops = &voice_dai_ops,
	},
	{
		.name = DAI_LINK_CODEC_DAI_NAME_FM,
		.playback = {
			.stream_name = "FM",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RATES,
			.formats = FORMATS },
	},
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static struct snd_soc_component_driver codec_driver = {
	.read = reg_read,
	.write = reg_write,
	.name = DA_COMBINE_V3_CODEC_NAME,
	.probe = codec_probe,
	.remove = codec_remove,
};
#else
static struct snd_soc_codec_driver codec_driver = {
	.read = reg_read_by_codec,
	.write = reg_write_by_codec,
	.component_driver.probe = codec_probe,
	.component_driver.remove = codec_remove,
	.component_driver.name = DA_COMBINE_V3_CODEC_NAME,
};
#endif

static bool check_codec_valid(struct hi_cdc_ctrl *cdc_ctrl)
{
	unsigned int val = hi_cdcctrl_reg_read(cdc_ctrl,
		DA_COMBINE_V3_VERSION_REG);

	if (val != DA_COMBINE_VERSION_CS) {
		AUDIO_LOGE("version failed:0x%x", val);
		return false;
	}

	return true;
}

static void get_board_ear_ir_gpio_id(struct device_node *node,
	struct board_cfg *board_cfg)
{
	int val = of_get_named_gpio(node, "hisilicon,ear_ir_gpio", 0);

	if (val > 0) {
		board_cfg->ear_ir_gpio_id = (unsigned int)val;
	} else {
		board_cfg->ear_ir_gpio_id = 0;
		AUDIO_LOGE("can not get ear ir gpio config");
	}
}

static void get_board_ir_gpio_id(struct device_node *node,
	struct board_cfg *board_cfg)
{
	int val = of_get_named_gpio(node, "hisilicon,ir_gpio", 0);

	if (val > 0) {
		board_cfg->ir_gpio_id = (unsigned int)val;
	} else {
		board_cfg->ir_gpio_id = 0;
		AUDIO_LOGE("can not get ir gpio config");
	}
}

static void get_board_micnum(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "hisilicon,mic_num", &val))
		board_cfg->mic_num = val;
	else
		board_cfg->mic_num = 2;

	AUDIO_LOGI("mic_num %d", board_cfg->mic_num);
}

static void get_board_pa(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "use_stereo_smartpa", &val)) {
		if (val != 0)
			board_cfg->use_stereo_smartpa = true;
		else
			board_cfg->use_stereo_smartpa = false;
	} else {
		board_cfg->use_stereo_smartpa = false;
	}
}

static void get_board_hpswitch(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "hisilicon,classh_rcv_hp_switch", &val)) {
		if (val != 0)
			board_cfg->classh_rcv_hp_switch = true;
		else
			board_cfg->classh_rcv_hp_switch = false;
	} else {
		board_cfg->classh_rcv_hp_switch = false;
	}
}

static void get_board_high_low_change(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "hisilicon,hp_high_low_change_enable", &val)) {
		if (val != 0)
			board_cfg->hp_high_low_change_enable = true;
		else
			board_cfg->hp_high_low_change_enable = false;
	} else {
		board_cfg->hp_high_low_change_enable = false;
	}
}

static void get_board_hp_res_detect(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "hisilicon,hp_res_detect_enable", &val)) {
		if (val != 0)
			board_cfg->hp_res_detect_enable = true;
		else
			board_cfg->hp_res_detect_enable = false;
	} else {
		board_cfg->hp_res_detect_enable = false;
	}
}

static void get_extern_hs_hifi_ak4376_i2s3(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "extern_hs_hifi_ak4376_I2S3", &val)) {
		if (val != 0)
			board_cfg->extern_hs_hifi_ak4376_I2S3 = true;
		else
			board_cfg->extern_hs_hifi_ak4376_I2S3 = false;
	} else {
		board_cfg->extern_hs_hifi_ak4376_I2S3 = false;
	}
}

static void get_board_gpio_pd_status(struct device_node *node,
	struct board_cfg *board_cfg)
{
	board_cfg->gpio_pd_enable =
		of_property_read_bool(node, "gpio_pd_enable");
}

static void get_board_hsd_cfg_enable(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "hsd_cfg_enable", &val)) {
		if (val != 0)
			board_cfg->hsd_cfg_enable = true;
		else
			board_cfg->hsd_cfg_enable = false;
	} else {
		board_cfg->hsd_cfg_enable = false;
	}
}

static void get_board_hsd_cfg_val(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "hsd_cfg_value", &val))
		board_cfg->hsd_cfg_value = val;
	else
		board_cfg->hsd_cfg_value = HSD_CFG_DEFAULT_VALUE;
}

static int bt_tri_gpio_init(int bt_tri_gpio)
{
	if (!gpio_is_valid(bt_tri_gpio)) {
		AUDIO_LOGE("bt tri is not support");
		return -EINVAL;
	}
	if (gpio_request(bt_tri_gpio, "bt_tri_gpio")) {
		AUDIO_LOGE("bt tri gpio request failed");
		return -EINVAL;
	}
	if (gpio_direction_output(bt_tri_gpio, BT_TRI_PA)) {
		AUDIO_LOGE("bt tri gpio set output failed");
		return -EINVAL;
	}

	return 0;
}

static void get_board_bt_tri(struct device_node *node,
	struct board_cfg *board_cfg)
{
	int val;
	int ret;

	AUDIO_LOGI("get board bt tri\n");

	val = of_get_named_gpio(node, "hisilicon,bt_tri_gpio", 0);
	if (val > 0) {
		board_cfg->bt_tri_gpio = (unsigned int)val;
		AUDIO_LOGI("get board bt tri, gpio = %d",
			board_cfg->bt_tri_gpio);

		ret = bt_tri_gpio_init(board_cfg->bt_tri_gpio);
		if (ret != 0)
			AUDIO_LOGE("gpio resource init fail, ret = %d", ret);
	} else {
		board_cfg->bt_tri_gpio = -1;
		AUDIO_LOGI("bt tri not supported, gpio = %d",
			board_cfg->bt_tri_gpio);
	}
}

static void get_board_hsdetfunc(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	board_cfg->hs_det_trigger_func = CMP_FUNC;
	if (!of_property_read_u32(node, "hisilicon,hs_det_trigger", &val)) {
		if (val != 0)
			board_cfg->hs_det_trigger_func = SCHMIT_FUNC;
	}
}

#ifdef CONFIG_HAC_SUPPORT
int hac_gpio_init(int hac_gpio)
{
	if (!gpio_is_valid(hac_gpio)) {
		AUDIO_LOGE("hac Value is not valid");
		return -EINVAL;
	}
	if (gpio_request(hac_gpio, "hac_en_gpio")) {
		AUDIO_LOGE("hac gpio request failed");
		return -EINVAL;
	}
	if (gpio_direction_output(hac_gpio, 0)) {
		AUDIO_LOGE("hac gpio set output failed");
		return -EINVAL;
	}

	return 0;
}

static void get_board_hac(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;
	int ret;

	if (!of_property_read_u32(node, "hisilicon,hac_gpio", &val)) {
		board_cfg->hac_gpio = val;
		ret = hac_gpio_init(board_cfg->hac_gpio);
		if (ret != 0)
			AUDIO_LOGE("gpio resource init fail, ret = %d", ret);
	} else {
		board_cfg->hac_gpio = -1;
	}
}
#endif

static void get_board_fm_enable(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "fm_enable", &val)) {
		if (val != 0)
			board_cfg->fm_enable = true;
		else
			board_cfg->fm_enable = false;
	} else {
		board_cfg->fm_enable = false;
	}
}

static void get_board_mclk_addr(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int addr;

	board_cfg->pmu_mclk_addr = 0;
	if (!of_property_read_u32(node, "hisilicon,pmu_mclk_addr", &addr)) {
		if (addr < 0xff)
			board_cfg->pmu_mclk_addr = addr;
	}
}

static void get_board_micbias_modify(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	if (!of_property_read_u32(node, "micbias_modify", &val)) {
		if (val != 0)
			board_cfg->micbias_modify = true;
		else
			board_cfg->micbias_modify = false;
	} else {
		board_cfg->micbias_modify = false;
	}
}

#ifdef CONFIG_RCV_TDD_SUPPORT
static int rcv_tdd_gpio_init(int rcv_tdd_gpio)
{
	if (!gpio_is_valid(rcv_tdd_gpio)) {
		AUDIO_LOGE("rcv tdd is is not valid");
		return -EINVAL;
	}
	if (gpio_request(rcv_tdd_gpio, "rcv_tdd_en_gpio")) {
		AUDIO_LOGE("rcv tdd gpio request failed");
		return -EINVAL;
	}
	if (gpio_direction_output(rcv_tdd_gpio, 0)) {
		AUDIO_LOGE("rcv tdd gpio set output failed");
		return -EINVAL;
	}

	return 0;
}

static void get_board_rcv_tdd(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;
	int ret;

	if (!of_property_read_u32(node, "hisilicon,rcv_tdd_gpio", &val)) {
		board_cfg->rcv_tdd_gpio = val;

		ret = rcv_tdd_gpio_init(board_cfg->rcv_tdd_gpio);
		if (ret != 0) {
			AUDIO_LOGE("gpio resource init fail, ret = %d", ret);
			board_cfg->rcv_tdd_gpio = -1;
		}
	} else {
		board_cfg->rcv_tdd_gpio = -1;
	}
}
#endif

static void get_board_wakeup_audio_algo_support(struct device_node *node,
	struct board_cfg *board_cfg)
{
	unsigned int val;

	board_cfg->wakeup_audio_algo_support = false;
	if (!of_property_read_u32(node, "hisilicon,wakeup_audio_algo_support", &val)) {
		if (val != 0)
			board_cfg->wakeup_audio_algo_support = true;
	}
}

static void get_board_cfg(struct device_node *node,
	struct board_cfg *board_cfg)
{
	get_board_ear_ir_gpio_id(node, board_cfg);
	get_board_ir_gpio_id(node, board_cfg);
	get_board_micnum(node, board_cfg);
	get_board_pa(node, board_cfg);
	get_board_hpswitch(node, board_cfg);
	get_board_high_low_change(node, board_cfg);
	get_board_hp_res_detect(node, board_cfg);
	get_extern_hs_hifi_ak4376_i2s3(node, board_cfg);
	get_board_gpio_pd_status(node, board_cfg);
	get_board_hsd_cfg_enable(node, board_cfg);
	get_board_hsd_cfg_val(node, board_cfg);
	get_board_bt_tri(node, board_cfg);
	get_board_hsdetfunc(node, board_cfg);
#ifdef CONFIG_HAC_SUPPORT
	get_board_hac(node, board_cfg);
#endif
	get_board_fm_enable(node, board_cfg);
	get_board_mclk_addr(node, board_cfg);
	get_board_micbias_modify(node, board_cfg);
#ifdef CONFIG_RCV_TDD_SUPPORT
	get_board_rcv_tdd(node, board_cfg);
#endif
	get_board_wakeup_audio_algo_support(node, board_cfg);
}

static void headphone_off(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	headphone_pop_off(codec);

	priv->rcv_hp_classh_state =
		(unsigned int)priv->rcv_hp_classh_state &
		(~HP_POWER_STATE);
	set_classh_config(codec, priv->rcv_hp_classh_state);
}

static void show_board_cfg(struct board_cfg board_config)
{
	AUDIO_LOGI("mic_num %d , use_stereo_smartpa %d, classh_rcv_hp_switch %d,"
		" hp_high_low_change_enable %d, hp_res_detect_enable %d,"
		" extern_hs_hifi_ak4376_I2S3 %d, gpio_pd_enable %d, ir_gpio_id %u,"
		" ear_ir_gpio_id %u, hsd_cfg_enable %d, bt_tri_gpio %d, fm_enable %d,"
		" micbias_modify %d, wakeup_audio_algo_support %d",
		board_config.mic_num,
		board_config.use_stereo_smartpa,
		board_config.classh_rcv_hp_switch,
		board_config.hp_high_low_change_enable,
		board_config.hp_res_detect_enable,
		board_config.extern_hs_hifi_ak4376_I2S3,
		board_config.gpio_pd_enable,
		board_config.ir_gpio_id,
		board_config.ear_ir_gpio_id,
		board_config.hsd_cfg_enable,
		board_config.bt_tri_gpio,
		board_config.fm_enable,
		board_config.micbias_modify,
		board_config.wakeup_audio_algo_support);
#ifdef CONFIG_HAC_SUPPORT
	AUDIO_LOGI("hac_gpio %d", board_config.hac_gpio);
#endif
#ifdef CONFIG_RCV_TDD_SUPPORT
	AUDIO_LOGI("rcv_tdd_gpio %d", board_config.rcv_tdd_gpio);
#endif
}

static void platform_data_cfg(struct platform_device *pdev,
	struct da_combine_v3_platform_data *platform_data)
{
	unsigned int i;

	platform_data->node = pdev->dev.of_node;

	get_board_cfg(platform_data->node, &platform_data->board_config);
	show_board_cfg(platform_data->board_config);

	platform_data->irqmgr =
		(struct da_combine_irq *)dev_get_drvdata(pdev->dev.parent);
	platform_data->cdc_ctrl =
		(struct hi_cdc_ctrl *)dev_get_drvdata(pdev->dev.parent->parent);
	platform_data->cdc_ctrl->ssi_check_enable = SSI_STATE_CHECK_DISABLE;
	platform_data->voice_params.channels = 2;
	platform_data->voice_params.rate = SLIMBUS_SAMPLE_RATE_16K;
	platform_data->capture_params.channels = 2;
	platform_data->capture_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	platform_data->play_params.channels = 2;
	platform_data->play_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	platform_data->soundtrigger_params.rate = SLIMBUS_SAMPLE_RATE_16K;
	platform_data->soundtrigger_params.channels = 1;
	platform_data->res_value = HS_RES_DEFAULT;
	platform_data->hs_high_pga_gain = HS_HIGH_0_GAIN;
	platform_data->hs_low_pga_gain = HS_LOW_0_GAIN;
	platform_data->voiceup_state = TRACK_FREE;
	platform_data->audioup_4mic_state = TRACK_FREE;

	for (i = 0; i < ARRAY_SIZE(platform_data->mic_fixed_value); i++)
		platform_data->mic_fixed_value[i] = MIC_FIXED_VAL_DEFAULT;

	if (platform_data->board_config.mic_num > 2)
		platform_data->voice_params.channels = 4;

	platform_set_drvdata(pdev, platform_data);
	dev_set_name(&pdev->dev, DAI_LINK_CODEC_NAME);
}

static struct da_combine_irq_map irqs = {
	{ DA_COMBINE_V3_IRQ_REG0_REG, DA_COMBINE_V3_IRQ_REG1_REG, DA_COMBINE_V3_IRQ_REG2_REG,
		DA_COMBINE_V3_IRQ_REG3_REG, DA_COMBINE_V3_IRQ_REG4_REG, DA_COMBINE_V3_IRQ_REG5_REG },
	{ DA_COMBINE_V3_IRQM_REG0_REG, DA_COMBINE_V3_IRQM_REG1_REG, DA_COMBINE_V3_IRQM_REG2_REG,
		DA_COMBINE_V3_IRQM_REG3_REG, DA_COMBINE_V3_IRQM_REG4_REG, DA_COMBINE_V3_IRQM_REG5_REG },
	DA_COMBINE_V3_IRQ_NUM,
};

static irqreturn_t bunk1_ocp_handler(int irq, void *data)
{
	AUDIO_LOGW("bunk1 ocp irq received");

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_CODEC_BUNK1_OCP, "da_combine codec bunk1 ocp\n");
#endif

	return IRQ_HANDLED;
}

static irqreturn_t bunk1_scp_handler(int irq, void *data)
{
	AUDIO_LOGW("bunk1 scp irq received");

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_CODEC_BUNK1_SCP, "da_combine codec bunk1 scp\n");
#endif

	return IRQ_HANDLED;
}

static irqreturn_t bunk2_ocp_handler(int irq, void *data)
{
	AUDIO_LOGW("bunk2 ocp irq received");

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_CODEC_BUNK2_OCP, "da_combine codec bunk2 ocp\n");
#endif

	return IRQ_HANDLED;
}

static irqreturn_t bunk2_scp_handler(int irq, void *data)
{
	AUDIO_LOGW("bunk2 scp irq received");

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_CODEC_BUNK2_SCP, "da_combine codec bunk2 scp\n");
#endif

	return IRQ_HANDLED;
}

static irqreturn_t cp1_short_handler(int irq, void *data)
{
	AUDIO_LOGW("cp1 short irq received");

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_CODEC_CP1_SHORT, "da_combine codec cp1 short\n");
#endif

	return IRQ_HANDLED;
}

static irqreturn_t cp2_short_handler(int irq, void *data)
{
	AUDIO_LOGW("cp2 short irq received");

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_CODEC_CP2_SHORT, "da_combine codec cp2 short\n");
#endif

	return IRQ_HANDLED;
}

static irqreturn_t pll_unlock_handler(int irq, void *data)
{
	AUDIO_LOGW("pll unlock irq received");

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_HI6402_PLL_UNLOCK, "da_combine codec pll unlock\n");
#endif

	return IRQ_HANDLED;
}

static irqreturn_t pll44k1_unlock_handler(int irq, void *data)
{
	AUDIO_LOGW("pll44k1 unlock irq received");

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_HI6402_PLL_UNLOCK, "da_combine codec pll 44k1 unlock\n");
#endif

	return IRQ_HANDLED;
}

static irqreturn_t pllmad_unlock_handler(int irq, void *data)
{
	AUDIO_LOGW("pllmad unlock irq received");

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_HI6402_PLL_UNLOCK, "da_combine codec pll mad unlock\n");
#endif

	return IRQ_HANDLED;
}

static const struct irq_config exception_irqs[] = {
	{ IRQ_BUNK1_OCP, bunk1_ocp_handler, "bunk1_ocp", 1 },
	{ IRQ_BUNK1_SCP, bunk1_scp_handler, "bunk1_scp", 1 },
	{ IRQ_BUNK2_OCP, bunk2_ocp_handler, "bunk2_ocp", 1 },
	{ IRQ_BUNK2_SCP, bunk2_scp_handler, "bunk2_scp", 1 },
	{ IRQ_CP1_SHORT, cp1_short_handler, "cp1_short", 1 },
	{ IRQ_CP2_SHORT, cp2_short_handler, "cp2_short", 1 },
	{ IRQ_PLL_UNLOCK, pll_unlock_handler, "pll_unlock", 0 },
	{ IRQ_PLL44K1_UNLOCK, pll44k1_unlock_handler, "pll44k1_unlock", 0 },
	{ IRQ_PLLMAD_UNLOCK, pllmad_unlock_handler, "pllmad_unlock", 0 },
};

static void exception_irq_free_irq(struct da_combine_v3_platform_data *platform_data)
{
	size_t len = ARRAY_SIZE(exception_irqs);
	unsigned int i;

	for (i = 0; i < len; i++)
		da_combine_irq_free_irq(platform_data->irqmgr,
			exception_irqs[i].type, platform_data);
}

static int exception_irq_init_irq(struct da_combine_v3_platform_data *platform_data)
{
	size_t len = ARRAY_SIZE(exception_irqs);
	int ret = 0;
	unsigned int i, j;

	for (i = 0; i < len; i++) {
		ret = da_combine_irq_request_irq(platform_data->irqmgr,
			exception_irqs[i].type, exception_irqs[i].handler,
			exception_irqs[i].name, platform_data);
		if (ret != 0) {
			AUDIO_LOGE("request irq failed, irq type is %d, irq name is %s",
				exception_irqs[i].type, exception_irqs[i].name);
			for (j = 0; j < i; j++) {
				da_combine_irq_free_irq(platform_data->irqmgr,
					exception_irqs[i - 1 - j].type,
					platform_data);
			}
			break;
		}

		if (!exception_irqs[i].enabled)
			da_combine_irq_disable_irq(platform_data->irqmgr,
				exception_irqs[i].type);
	}

	return ret;
}

static int platform_irq_init(struct da_combine_v3_platform_data *platform_data)
{
	int ret;

	ret = da_combine_irq_init_irq(platform_data->irqmgr, &irqs);
	if (ret != 0) {
		AUDIO_LOGE("irq init failed:0x%x", ret);
		return ret;
	}

	ret = exception_irq_init_irq(platform_data);
	if (ret != 0) {
		AUDIO_LOGE("exception irq init failed:0x%x", ret);
		da_combine_irq_deinit_irq(platform_data->irqmgr);
		return ret;
	}

	return 0;
}

static void platform_irq_deinit(struct da_combine_v3_platform_data *platform_data)
{
	exception_irq_free_irq(platform_data);
	da_combine_irq_deinit_irq(platform_data->irqmgr);
}

static void platform_free_gpio(struct board_cfg *board_config)
{
	if (gpio_is_valid(board_config->bt_tri_gpio))
		gpio_free(board_config->bt_tri_gpio);

#ifdef CONFIG_HAC_SUPPORT
	if (gpio_is_valid(board_config->hac_gpio))
		gpio_free(board_config->hac_gpio);
#endif

#ifdef CONFIG_RCV_TDD_SUPPORT
	if (gpio_is_valid(board_config->rcv_tdd_gpio))
		gpio_free(board_config->rcv_tdd_gpio);
#endif
}

static int platform_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct da_combine_v3_platform_data *platform_data = NULL;
	const struct of_device_id *match = NULL;

	platform_data = devm_kzalloc(dev, sizeof(*platform_data), GFP_KERNEL);
	if (platform_data == NULL) {
		AUDIO_LOGE("malloc platform data failed");
		return -ENOMEM;
	}

	match = of_match_device(platform_match, &pdev->dev);
	if (match == NULL) {
		AUDIO_LOGE("get device info err");
		ret = -ENOENT;
		goto free_platform_data;
	}

	platform_data_cfg(pdev, platform_data);

	if (!check_codec_valid(platform_data->cdc_ctrl)) {
		AUDIO_LOGE("check valid error");
		ret = -ENODEV;
		goto free_platform_gpio;
	}

	ret = platform_irq_init(platform_data);
	if (ret != 0) {
		AUDIO_LOGE("platform irq init failed:0x%x", ret);
		goto free_platform_gpio;
	}

	ret = da_combine_compat_init(platform_data->cdc_ctrl,
		platform_data->irqmgr);
	if (ret != 0) {
		AUDIO_LOGE("compat init failed:0x%x", ret);
		goto compat_init_err_exit;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	ret = devm_snd_soc_register_component(dev, &codec_driver, dai,
		ARRAY_SIZE(dai));
#else
	ret = snd_soc_register_codec(dev, &codec_driver, dai,
			ARRAY_SIZE(dai));
#endif
	if (ret != 0) {
		AUDIO_LOGE("registe driver failed:0x%x", ret);
		goto codec_register_err_exit;
	}

	return ret;

codec_register_err_exit:
	da_combine_compat_deinit();
compat_init_err_exit:
	platform_irq_deinit(platform_data);
free_platform_gpio:
	platform_free_gpio(&platform_data->board_config);
free_platform_data:
	AUDIO_LOGE("platform probe init failed");

	return ret;
}

static int platform_remove(struct platform_device *pdev)
{
	struct da_combine_v3_platform_data *platform_data = platform_get_drvdata(pdev);

	IN_FUNCTION;

	platform_free_gpio(&platform_data->board_config);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
	snd_soc_unregister_codec(&pdev->dev);
#endif

	da_combine_compat_deinit();

	platform_irq_deinit(platform_data);

	OUT_FUNCTION;

	return 0;
}

static void platform_shutdown(struct platform_device *pdev)
{
	struct da_combine_v3_platform_data *platform_data = platform_get_drvdata(pdev);
	struct snd_soc_component *codec = platform_data->codec;

	if (codec != NULL)
		headphone_off(codec);
}

static struct platform_driver platform_driver = {
	.probe = platform_probe,
	.remove = platform_remove,
	.shutdown = platform_shutdown,
	.driver = {
		.owner = THIS_MODULE,
		.name = DAI_LINK_CODEC_NAME,
		.of_match_table = of_match_ptr(platform_match),
	},
};

static int __init da_combine_v3_platform_init(void)
{
	return platform_driver_register(&platform_driver);
}

static void __exit da_combine_v3_platform_exit(void)
{
	platform_driver_unregister(&platform_driver);
}

fs_initcall_sync(da_combine_v3_platform_init);
module_exit(da_combine_v3_platform_exit);

MODULE_DESCRIPTION("ASoC DA combine v3 codec driver");
MODULE_LICENSE("GPL");


