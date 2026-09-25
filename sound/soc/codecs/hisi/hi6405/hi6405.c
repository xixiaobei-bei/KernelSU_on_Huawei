/*
 * hi6405.c -- hi6405 codec driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "linux/hisi/hi64xx/hi6405.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/gpio.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#include "linux/hisi/audio_log.h"
#include "slimbus.h"
#include "slimbus_6405.h"
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "hi6405_debug.h"
#endif
#include "linux/hisi/hi64xx/hi64xx_compat.h"
#include "linux/hisi/hi64xx/hi64xx_utils.h"
#include "linux/hisi/hi64xx/hi64xx_resmgr.h"
#include "linux/hisi/hi64xx/hi64xx_vad.h"
#include "linux/hisi/hi64xx/hi64xx_mbhc.h"
#include "linux/hisi/hi64xx/hi6405_regs.h"
#include "linux/hisi/hi64xx/hi6405_type.h"
#include <linux/hisi/hi64xx_dsp/hi64xx_dsp_misc.h>
#include "hi6405_dsp_config.h"
#include "hi6405_kcontrol.h"
#include "hi6405_resource_widget.h"
#include "hi6405_path_widget.h"
#include "hi6405_switch_widget.h"
#include "hi6405_route.h"
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
#include "hi6405_high_res_cfg.h"
#include "huawei_platform/audio/high_resistance_hs_det.h"
#endif

#define MBHC_VOLTAGE_COEFFICIENT_MIN 1600
#define MBHC_VOLTAGE_COEFFICIENT_DEFAULT 2700
#define MBHC_VOLTAGE_COEFFICIENT_MAX 2800
#define ADC_PGA_GAIN_DEFAULT 0x78
#define DACLR_MIXER_PGA_GAIN_DEFAULT 0xFF

#ifdef CONFIG_HISI_DIEID
#define CODEC_DIEID_BUF 60
#define CODEC_DIEID_TEM_BUF 4
#endif

#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
#define HS_DET_R_RATIO         2
#define HS_DET_I_RATIO         16
#define HS_DET_MICBIAS         1800 /* mv */
#define HS_DET_DEFAULT_VPEAK   200  /* mv */
#define HS_DET_INNER_RES       2200 /* ohm */
#define MAX_SARADC_VAL         4096
#define MAX_DAC_POST_PGA_GAIN  0x78
#endif

#define PLL_DATA_ALL_NUM     128
#define PLL_DATA_GROUP_NUM   8
#define PLL_DATA_BUF_SIZE    10

struct reg_page {
	unsigned int offest;
	unsigned int begin;
	unsigned int end;
};

static const struct reg_page hi6405_reg_info[] = {
	{ CODEC_BASE_ADDR_PAGE_IO, CODEC_ADDR_PAGE_IO_START, CODEC_ADDR_PAGE_IO_END },
	{ CODEC_BASE_ADDR_PAGE_CFG, CODEC_ADDR_PAGE_CFG_START, CODEC_ADDR_PAGE_CFG_END },
	{ CODEC_BASE_ADDR_PAGE_ANA, ADDR_ANA_OFFSET_START, ADDR_ANA_OFFSET_END },
	{ CODEC_BASE_ADDR_PAGE_DIG, ADDR_DIG_OFFSET_START, ADDR_DIG_OFFSET_END },

	{ BASE_ADDR_PAGE_VIRTUAL, VIR_UP, VIR_CNT - 1 },
	{ ADDR_OCRAM_BASE, ADDR_OCRAM_START, ADDR_OCRAM_END },
	{ ADDR_TCM_BASE, ADDR_TCM_START, ADDR_TCM_END },

	{ BASE_ADDR_PAGE_WDOG, ADDR_WDOG_OFFSET_START, ADDR_WDOG_OFFSET_END },
	{ BASE_ADDR_PAGE_SCTRL, ADDR_SCTRL_OFFSET_START, ADDR_SCTRL_OFFSET_END },
	{ BASE_ADDR_PAGE_UART, ADDR_UART_OFFSET_START, ADDR_UART_OFFSET_END },
	{ BASE_ADDR_PAGE_TIMER0, ADDR_TIMER0_OFFSET_START, ADDR_TIMER0_OFFSET_END },
	{ BASE_ADDR_PAGE_TIMER1, ADDR_TIMER1_OFFSET_START, ADDR_TIMER1_OFFSET_END },

	{ BASE_ADDR_PAGE_GPIO0, ADDR_GPIO0_OFFSET_START, ADDR_GPIO0_OFFSET_END },
	{ BASE_ADDR_PAGE_GPIO1, ADDR_GPIO1_OFFSET_START, ADDR_GPIO1_OFFSET_END },
	{ BASE_ADDR_PAGE_GPIO2, ADDR_GPIO2_OFFSET_START, ADDR_GPIO2_OFFSET_END },

	{ BASE_ADDR_PAGE_TIMER32K, ADDR_TIMER32K_OFFSET_START, ADDR_TIMER32K_OFFSET_END },
	{ BASE_ADDR_PAGE_I2C_MST, ADDR_I2C_MST_OFFSET_START, ADDR_I2C_MST_OFFSET_END },
	{ BASE_ADDR_PAGE_USB, ADDR_USB_OFFSET_START, ADDR_USB_OFFSET_END },
	{ BASE_ADDR_PAGE_EDMA, ADDR_EDMA_OFFSET_START, ADDR_EDMA_OFFSET_END },
	{ BASE_ADDR_PAGE_PLL_TEST, ADDR_PLL_TEST_OFFSET_START, ADDR_PLL_TEST_OFFSET_END },
	{ BASE_ADDR_PAGE_DSPIF, ADDR_DSPIF_OFFSET_START, ADDR_DSPIF_OFFSET_END },
};

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
static struct hicodec_dump_reg_entry hi6405_dump_table[] = {
	{ "PAGE IO", DBG_PAGE_IO_CODEC_START, DBG_PAGE_IO_CODEC_END, 4 },
	{ "PAGE CFG", DBG_PAGE_CFG_CODEC_START, DBG_PAGE_CFG_CODEC_END, 1 },
	{ "PAGE ANA", DBG_PAGE_ANA_CODEC_START, DBG_PAGE_ANA_CODEC_END, 1 },
	{ "PAGE DIG", DBG_PAGE_DIG_CODEC_START, DBG_PAGE_DIG_CODEC_END, 1 },
};

static struct hicodec_dump_reg_info dump_info = {
	.entry = hi6405_dump_table,
	.count = sizeof(hi6405_dump_table) / sizeof(struct hicodec_dump_reg_entry),
};
#endif

static const struct of_device_id hi6405_platform_match[] = {
	{ .compatible = "hisilicon,hi6405-codec", },
	{ },
};

static struct snd_soc_codec *hi6405_codec;

void write_reg_array(struct snd_soc_codec *codec,
	const struct reg_config *reg_array, size_t len)
{
	unsigned int i;

	if (codec == NULL) {
		AUDIO_LOGE("codec is null");
		return;
	}

	if (reg_array == NULL) {
		AUDIO_LOGE("reg array is null");
		return;
	}

	for (i = 0; i < len; i++) {
		if (reg_array[i].update_bits)
			hi64xx_update_bits(codec, reg_array[i].addr,
				reg_array[i].mask, reg_array[i].val);
		else
			snd_soc_write(codec, reg_array[i].addr,
				reg_array[i].val);
	}
}

void write_reg_seq_array(struct snd_soc_codec *codec,
	const struct reg_seq_config *reg_seq_array, size_t len)
{
	unsigned int i;

	if (codec == NULL) {
		AUDIO_LOGE("codec is null");
		return;
	}

	if (reg_seq_array == NULL) {
		AUDIO_LOGE("reg array is null");
		return;
	}

	for (i = 0; i < len; i++) {
		if (reg_seq_array[i].cfg.update_bits)
			hi64xx_update_bits(codec, reg_seq_array[i].cfg.addr,
				reg_seq_array[i].cfg.mask, reg_seq_array[i].cfg.val);
		else
			 snd_soc_write(codec, reg_seq_array[i].cfg.addr,
				reg_seq_array[i].cfg.val);

		switch (reg_seq_array[i].type) {
		case RANGE_SLEEP:
			usleep_range(reg_seq_array[i].us,
				reg_seq_array[i].us + reg_seq_array[i].us / 10);
			break;
		case MSLEEP:
			msleep(reg_seq_array[i].us / 1000);
			break;
		case MDELAY:
			mdelay((unsigned long)(reg_seq_array[i].us / 1000));
			break;
		default:
			break;
		}
	}
}

static unsigned int virtual_reg_read(struct hi6405_platform_data *platform_data,
	unsigned int addr)
{
	unsigned int ret = 0;
	unsigned long flag;
	unsigned int *map = platform_data->board_config.mic_map;

	spin_lock_irqsave(&platform_data->v_rw_lock, flag);

	switch (addr) {
	case VIRTUAL_DOWN_REG:
		ret = platform_data->virtual_reg[VIR_UP];
		break;
	case VIRTUAL_UP_REG:
		ret = platform_data->virtual_reg[VIR_DOWN];
		break;
	case VIRTUAL_EXTERN_REG:
		ret = platform_data->virtual_reg[VIR_EXTERN];
		break;
	case VIRTUAL_MAIN_MIC_CALIB_REG:
		ret = platform_data->mic_calib_value[map[MAIN_MIC1]];
		break;
	case VIRTUAL_MAIN_MIC2_CALIB_REG:
		ret = platform_data->mic_calib_value[map[MAIN_MIC2]];
		break;
	case VIRTUAL_SUB_MIC_CALIB_REG:
		ret = platform_data->mic_calib_value[map[SUB_MIC1]];
		break;
	case VIRTUAL_SUB_MIC2_CALIB_REG:
		ret = platform_data->mic_calib_value[map[SUB_MIC2]];
		break;
	default:
		AUDIO_LOGE("read failed: virtual reg addr is not existed\n");
		break;
	}

	spin_unlock_irqrestore(&platform_data->v_rw_lock, flag);

	return ret;
}

static bool is_reg_valid(unsigned int reg)
{
	unsigned int val = reg & (~CODEC_BASE_ADDR);
	size_t len = ARRAY_SIZE(hi6405_reg_info);
	unsigned int begin;
	unsigned int end;
	unsigned int i;

	for (i = 0; i < len; i++) {
		begin = hi6405_reg_info[i].begin + hi6405_reg_info[i].offest;
		end = hi6405_reg_info[i].end + hi6405_reg_info[i].offest;
		if (val >= begin && val <= end)
			return true;
	}

	AUDIO_LOGE("invalid reg: 0x%x, begin: 0x%x, end: 0x%x\n",
		val, begin, end);
	return false;
}

static unsigned int hi6405_reg_read(struct snd_soc_codec *codec,
	unsigned int reg)
{
	unsigned int ret = 0;
	unsigned int reg_mask;
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	if (!is_reg_valid(reg)) {
		AUDIO_LOGE("invalid reg: 0x%x", reg);
		return 0;
	}

	reg_mask = reg & 0xFFFFF000;
	if (reg_mask == BASE_ADDR_PAGE_CFG || reg_mask == BASE_ADDR_PAGE_IO) {
		reg = reg | CODEC_BASE_ADDR;
	} else if (reg_mask == BASE_ADDR_PAGE_VIRTUAL) {
		ret = virtual_reg_read(platform_data, reg);
		return ret;
	}

	if (platform_data->resmgr == NULL) {
		AUDIO_LOGE("platform_data->resmgr is null");
		return 0;
	}

	hi64xx_resmgr_request_reg_access(platform_data->resmgr, reg);
	ret = hi_cdcctrl_reg_read(platform_data->cdc_ctrl, reg);
	hi64xx_resmgr_release_reg_access(platform_data->resmgr, reg);

	return ret;
}

static void virtual_reg_write(struct hi6405_platform_data *platform_data,
	unsigned int addr, unsigned int value)
{
	unsigned long flag;
	unsigned int *map = platform_data->board_config.mic_map;

	spin_lock_irqsave(&platform_data->v_rw_lock, flag);

	switch (addr) {
	case VIRTUAL_DOWN_REG:
		platform_data->virtual_reg[VIR_UP] = value;
		break;
	case VIRTUAL_UP_REG:
		platform_data->virtual_reg[VIR_DOWN] = value;
		break;
	case VIRTUAL_EXTERN_REG:
		platform_data->virtual_reg[VIR_EXTERN] = value;
		break;
	case VIRTUAL_MAIN_MIC_CALIB_REG:
		platform_data->mic_calib_value[map[MAIN_MIC1]] = value;
		break;
	case VIRTUAL_MAIN_MIC2_CALIB_REG:
		platform_data->mic_calib_value[map[MAIN_MIC2]] = value;
		break;
	case VIRTUAL_SUB_MIC_CALIB_REG:
		platform_data->mic_calib_value[map[SUB_MIC1]] = value;
		break;
	case VIRTUAL_SUB_MIC2_CALIB_REG:
		platform_data->mic_calib_value[map[SUB_MIC2]] = value;
		break;
	default:
		AUDIO_LOGE("write failed: virtual reg addr is not existed\n");
		break;
	}

	spin_unlock_irqrestore(&platform_data->v_rw_lock, flag);
}

static int hi6405_reg_write(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value)
{
	int ret;
	unsigned int reg_mask;
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	if (!is_reg_valid(reg)) {
		AUDIO_LOGE("invalid reg: 0x%x", reg);
		return -EINVAL;
	}

	reg_mask = reg & 0xFFFFF000;
	if (reg_mask == BASE_ADDR_PAGE_CFG || reg_mask == BASE_ADDR_PAGE_IO) {
		reg = reg | CODEC_BASE_ADDR;
	} else if (reg_mask == BASE_ADDR_PAGE_VIRTUAL) {
		virtual_reg_write(platform_data, reg, value);
		return 0;
	}

	if (platform_data->resmgr == NULL) {
		AUDIO_LOGE("platform_data->resmgr null");
		return -EFAULT;
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_reg_rw_cache(reg, value, HICODEC_DEBUG_FLAG_WRITE);
#endif

	hi64xx_resmgr_request_reg_access(platform_data->resmgr, reg);
	ret = hi_cdcctrl_reg_write(platform_data->cdc_ctrl, reg, value);
	hi64xx_resmgr_release_reg_access(platform_data->resmgr, reg);

	return ret;
}

static const struct reg_seq_config ioshare_slimbus_init_regs[] = {
	{ { IOS_MF_CTRL1_REG, 0, 0x1 << IOS_MF_CTRL1_OFFSET, false }, 0, 0 },
	{ { IOS_MF_CTRL3_REG, 0, 0x2 << IOS_MF_CTRL3_OFFSET, false }, 0, 0 },
	{ { IOS_MF_CTRL4_REG, 0, 0x2 << IOS_MF_CTRL4_OFFSET, false }, 0, 0 },
	{ { IOS_IOM_CTRL7_REG, 0, 0x114, false }, 0, 0 },
	{ { IOS_IOM_CTRL8_REG, 0, 0x115, false }, 0, 0 },

	/* enable hi6405 slim framer */
	{ { SLIM_CTRL1_REG, 0x1 << SLIM_CLK_EN_OFFSET,
		0x1 << SLIM_CLK_EN_OFFSET, true }, 0, 0 },
	{ { SLIM_CTRL1_REG, 0x1 << SLIM_SWIRE_DIV_EN_OFFSET,
		0x1 << SLIM_SWIRE_DIV_EN_OFFSET, true }, 0, 0 },
};

static const struct reg_seq_config ioshare_init_regs[] = {
	/* ssi ioshare config */
	{ { IOS_IOM_CTRL5_REG, 0, 0x10D, false }, 0, 0 },
	{ { IOS_MF_CTRL1_REG, 0, 0x1 << IOS_MF_CTRL1_OFFSET, false }, 0, 0 },

	/* slimbus ioshare config */
	{ { IOS_MF_CTRL3_REG, 0, 0x2 << IOS_MF_CTRL3_OFFSET, false }, 0, 0 },
	{ { IOS_MF_CTRL4_REG, 0, 0x2 << IOS_MF_CTRL4_OFFSET, false }, 0, 0 },

	/* I2S2 ioshare config */
	{ { IOS_MF_CTRL7_REG, 0, 0x01, false }, 0, 0 },
	{ { IOS_MF_CTRL8_REG, 0, 0x01, false }, 0, 0 },
	{ { IOS_MF_CTRL9_REG, 0, 0x01, false }, 0, 0 },
	{ { IOS_MF_CTRL10_REG, 0, 0x01, false }, 0, 0 },

	/* I2S4 ioshare config */
	{ { IOS_MF_CTRL11_REG, 0, 0x01, false }, 0, 0 },
	{ { IOS_MF_CTRL12_REG, 0, 0x01, false }, 0, 0 },
	{ { IOS_MF_CTRL13_REG, 0, 0x01, false }, 0, 0 },
	{ { IOS_MF_CTRL14_REG, 0, 0x01, false }, 0, 0 },
};

static void ioshare_init(struct snd_soc_codec *codec,
	struct hi6405_platform_data *platform_data)
{
	if (platform_data->cdc_ctrl->bus_sel == BUSTYPE_SELECT_SLIMBUS)
		write_reg_seq_array(codec, ioshare_slimbus_init_regs,
			ARRAY_SIZE(ioshare_slimbus_init_regs));

	write_reg_seq_array(codec, ioshare_init_regs,
		ARRAY_SIZE(ioshare_init_regs));
}

static void slimbus_init(struct snd_soc_codec *codec,
	struct hi6405_platform_data *platform_data)
{
	/* slim&ssi mclk enable */
	hi64xx_update_bits(codec, CFG_CLK_CTRL_REG,
		0x1 << INTF_SSI_CLK_EN_OFFSET,
		0x1 << INTF_SSI_CLK_EN_OFFSET);
	hi64xx_update_bits(codec, CFG_CLK_CTRL_REG,
		0x1 << INTF_SLIM_CLK_EN_OFFSET,
		0x1 << INTF_SLIM_CLK_EN_OFFSET);

	snd_soc_write(codec, SLIM_CTRL3_REG, 0xBF);

	/* slimbus clk schmitt config */
	hi64xx_update_bits(codec, IOS_IOM_CTRL8_REG,
		0x3 << ST_OFFSET, 0x1 << ST_OFFSET);
	/* slimbus pin disable pd */
	hi64xx_update_bits(codec, IOS_IOM_CTRL7_REG,
		0x1 << PD_OFFSET, 0x0 << PD_OFFSET);
	hi64xx_update_bits(codec, IOS_IOM_CTRL8_REG,
		0x1 << PD_OFFSET, 0x0 << PD_OFFSET);

	/* slimbus drv codec side */
	hi64xx_update_bits(codec, IOS_IOM_CTRL7_REG, 0x7 << DS_OFFSET,
		platform_data->cdc_ctrl->slimbusdata_cdc_drv << DS_OFFSET);

	hi64xx_update_bits(codec, IOS_IOM_CTRL8_REG, 0x7 << DS_OFFSET,
		platform_data->cdc_ctrl->slimbusclk_cdc_drv << DS_OFFSET);

	/* slimbus frame config */
	snd_soc_write(codec, SLIM_CTRL0_REG, 0x6);

	/* slimbus up1&2 port fs config */
	snd_soc_write(codec, SC_FS_SLIM_CTRL_3_REG, 0x44);
	/* slimbus up3&4 port fs config */
	snd_soc_write(codec, SC_FS_SLIM_CTRL_4_REG, 0x44);
	/* slimbus up7&8 port fs config */
	snd_soc_write(codec, SC_FS_SLIM_CTRL_6_REG, 0x44);
	/* slimbus up9&10 port fs config */
	snd_soc_write(codec, SC_FS_SLIM_CTRL_7_REG, 0x44);

	/* slimbus dn1&dn2 port fs config */
	snd_soc_write(codec, SC_FS_SLIM_CTRL_0_REG, 0x44);
	/* slimbus dn5&dn6 port fs config */
	snd_soc_write(codec, SC_FS_SLIM_CTRL_2_REG, 0x44);
}

static void efuse_init(struct snd_soc_codec *codec)
{
	unsigned int inf_ate_ctrl;
	unsigned int inf_trim_ctrl;
	unsigned int bgr_ate;
	unsigned int die_id0;
	unsigned int die_id1;

	/* enable efuse */
	hi64xx_update_bits(codec, DIE_ID_CFG1_REG,
		0x1 << EFUSE_MODE_SEL_OFFSET, 0x1 << EFUSE_MODE_SEL_OFFSET);
	hi64xx_update_bits(codec, CFG_CLK_CTRL_REG,
		0x1 << EFUSE_CLK_EN_OFFSET, 0x1 << EFUSE_CLK_EN_OFFSET);
	hi64xx_update_bits(codec, DIE_ID_CFG1_REG,
		0x1 << EFUSE_READ_EN_OFFSET, 0x1 << EFUSE_READ_EN_OFFSET);

	usleep_range(5000, 5500);

	/* default para set */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_052,
		0x1 << CODEC_OTPREG_SEL_FIR_OFFSET,
		0x0 << CODEC_OTPREG_SEL_FIR_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_053,
		0x1 << CODEC_OTPREG_SEL_INF_OFFSET,
		0x0 << CODEC_OTPREG_SEL_INF_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_133,
		0x1 << CODEC_OTPREG_SEL_BGR_OFFSET,
		0x0 << CODEC_OTPREG_SEL_BGR_OFFSET);

	die_id0 = snd_soc_read(codec, DIE_ID_OUT_DATA0_REG);
	die_id1 = snd_soc_read(codec, DIE_ID_OUT_DATA1_REG);
	bgr_ate = snd_soc_read(codec, DIE_ID_OUT_DATA2_REG);

	inf_ate_ctrl = die_id0 & 0xf;
	inf_trim_ctrl = ((die_id0 & 0xf0) >> 0x4) | ((die_id1 & 0x7) << 0x4);

	AUDIO_LOGI("efuse inf ate: 0x%x, inf trim: 0x%x, bgr ate0x%x",
			inf_ate_ctrl, inf_trim_ctrl, bgr_ate);

}

static int slim_enumerate(struct hi6405_platform_data *data)
{
	int ret = 0;

	if (!data->cdc_ctrl->pm_runtime_support)
		return ret;

	/* open codec pll and asp clk to make sure codec framer be enumerated */
	ret = hi64xx_resmgr_request_pll(data->resmgr, PLL_HIGH);
	if (ret != 0) {
		AUDIO_LOGE("request pll failed");
		return ret;
	}
	ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
		SLIMBUS_6405_TRACK_AUDIO_PLAY, NULL);
	if (ret != 0)
		AUDIO_LOGE("slimbus_activate_track audio play failed");
	usleep_range(1000, 1100);
	ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
		SLIMBUS_6405_TRACK_AUDIO_PLAY, NULL);
	if (ret != 0)
		AUDIO_LOGE("deactivate audio play failed");
	ret = hi64xx_resmgr_release_pll(data->resmgr, PLL_HIGH);
	if (ret != 0)
		AUDIO_LOGE("release pll failed");
	return ret;
}

static int utils_init(struct hi6405_platform_data *platform_data)
{
	int ret;
	struct utils_config cfg;

	cfg.hi64xx_dump_reg = NULL;
	ret = hi64xx_utils_init(platform_data->codec, platform_data->cdc_ctrl,
		&cfg, platform_data->resmgr, HI64XX_CODEC_TYPE_6405);

	return ret;
}

static void set_mad_param(struct snd_soc_codec *codec,
	struct hi6405_board_cfg *board_cfg)
{
	/* auto active time */
	snd_soc_write(codec, MAD_AUTO_ACT_TIME_REG, 0x0);

	/* pll time */
	snd_soc_write(codec, MAD_PLL_TIME_L_REG, 0x1);

	/* adc time */
	snd_soc_write(codec, MAD_ADC_TIME_H_REG, 0x0);
	snd_soc_write(codec, MAD_ADC_TIME_L_REG, 0x3);

	/* mad_ana_time */
	snd_soc_write(codec, MAD_ANA_TIME_H_REG, 0x0);
	snd_soc_write(codec, MAD_ANA_TIME_L_REG, 0x5);

	/* omt */
	snd_soc_write(codec, MAD_OMIT_SAMP_REG, 0x20);

	/* mad_vad_time */
	snd_soc_write(codec, MAD_VAD_TIME_H_REG, 0x0);
	snd_soc_write(codec, MAD_VAD_TIME_L_REG, 0xa0);

	/* mad_sleep_time */
	snd_soc_write(codec, MAD_SLEEP_TIME_L_REG, 0x0);

	/* mad_buffer_fifo_thre */
	if (board_cfg->wakeup_hisi_algo_support)
		snd_soc_write(codec, MAD_BUFFER_CTRL0_REG, 0x3f);
	else
		snd_soc_write(codec, MAD_BUFFER_CTRL0_REG, 0x7f);
	hi64xx_update_bits(codec, MAD_BUFFER_CTRL1_REG, 0x1f, 0x1f);

	/* mad_cnt_thre,vad delay cnt */
	snd_soc_write(codec, MAD_CNT_THRE_REG, 0x2);

	/* mad_snr_thre */
	snd_soc_write(codec, MAD_SNR_THRE_SUM_REG, 0x32);
	snd_soc_write(codec, MAD_SNR_THRE_REG, 0x20);

	/* mad_min_chan_eng */
	snd_soc_write(codec, MAD_MIN_CHAN_ENG_REG, 0x14);

	/* mad_ine */
	snd_soc_write(codec, MAD_INE_REG, 0x14);
	/* mad_band_thre */
	snd_soc_write(codec, MAD_BAND_THRE_REG, 0x8);
	/* mad_scale */
	snd_soc_write(codec, MAD_SCALE_REG, 0x3);

	/* mad_vad_num */
	snd_soc_write(codec, MAD_VAD_NUM_REG, 0x1);
	/* mad_alpha_en1 */
	snd_soc_write(codec, MAD_ALPHA_EN1_REG, 0xc);

	/* mad_vad_ao ->en, mad_irq_en->en, mad_en->en, mad_wind_sel */
	snd_soc_write(codec, MAD_CTRL_REG, 0x63);

	/* mad capless config */
	snd_soc_write(codec, ANA_MAD_CAPLESS_MIXER, 0x0);
	snd_soc_write(codec, ANA_MAD_PGA_CAPLESS_MIXER, 0x0);
}

static void set_dsp_if_bypass(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL22_REG,
		0x1 << S3_O_DSP_BYPASS_OFFSET | 0x1 << S2_O_DSP_BYPASS_OFFSET |
		0x1 << S1_O_DSP_BYPASS_OFFSET,
		0x1 << S3_O_DSP_BYPASS_OFFSET | 0x1 << S2_O_DSP_BYPASS_OFFSET |
		0x1 << S1_O_DSP_BYPASS_OFFSET);
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL8_REG,
		0x1 << S4_I_DSP_BYPASS_OFFSET | 0x1 << S3_I_DSP_BYPASS_OFFSET |
		0x1 << S2_I_DSP_BYPASS_OFFSET | 0x1 << S1_I_DSP_BYPASS_OFFSET,
		0x1 << S4_I_DSP_BYPASS_OFFSET | 0x1 << S3_I_DSP_BYPASS_OFFSET |
		0x1 << S2_I_DSP_BYPASS_OFFSET | 0x1 << S1_I_DSP_BYPASS_OFFSET);
}

static const struct reg_seq_config pga_fade_regs[] = {
	{ { DACL_MIXER4_CTRL1_REG, 0x1 << DACL_MIXER4_FADE_EN_OFFSET,
		0x1 << DACL_MIXER4_FADE_EN_OFFSET, true }, 0, 0 },
	{ { DACL_MIXER4_CTRL3_REG, MASK_ON_BIT(DACL_MIXER4_FADE_IN_LEN,
		DACL_MIXER4_FADE_IN_OFFSET), 0xf << DACL_MIXER4_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACL_MIXER4_CTRL4_REG, MASK_ON_BIT(DACL_MIXER4_FADE_OUT_LEN,
		DACL_MIXER4_FADE_OUT_OFFSET), 0xa << DACL_MIXER4_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACR_MIXER4_CTRL1_REG, 0x1 << DACR_MIXER4_FADE_EN_OFFSET,
		0x1 << DACR_MIXER4_FADE_EN_OFFSET, true }, 0, 0 },
	{ { DACR_MIXER4_CTRL3_REG, MASK_ON_BIT(DACR_MIXER4_FADE_IN_LEN,
		DACR_MIXER4_FADE_IN_OFFSET), 0xf << DACR_MIXER4_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACR_MIXER4_CTRL4_REG, MASK_ON_BIT(DACR_MIXER4_FADE_OUT_LEN,
		DACR_MIXER4_FADE_OUT_OFFSET), 0xa << DACR_MIXER4_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACL_PRE_MIXER2_CTRL1_REG, 0x1 << DACL_PRE_MIXER2_FADE_EN_OFFSET,
		0x1 << DACL_PRE_MIXER2_FADE_EN_OFFSET, true }, 0, 0 },
	{ { DACL_PRE_MIXER2_CTRL2_REG, MASK_ON_BIT(DACL_PRE_MIXER2_FADE_IN_LEN,
		DACL_PRE_MIXER2_FADE_IN_OFFSET),
		0xf << DACL_PRE_MIXER2_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACL_PRE_MIXER2_CTRL3_REG, MASK_ON_BIT(DACL_PRE_MIXER2_FADE_OUT_LEN,
		DACL_PRE_MIXER2_FADE_OUT_OFFSET),
		0xa << DACL_PRE_MIXER2_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACR_PRE_MIXER2_CTRL1_REG, 0x1 << DACR_PRE_MIXER2_FADE_EN_OFFSET,
		0x1 << DACR_PRE_MIXER2_FADE_EN_OFFSET, true }, 0, 0 },
	{ { DACR_PRE_MIXER2_CTRL2_REG, MASK_ON_BIT(DACR_PRE_MIXER2_FADE_IN_LEN,
		DACR_PRE_MIXER2_FADE_IN_OFFSET),
		0xf << DACR_PRE_MIXER2_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACR_PRE_MIXER2_CTRL3_REG, MASK_ON_BIT(DACR_PRE_MIXER2_FADE_OUT_LEN,
		DACR_PRE_MIXER2_FADE_OUT_OFFSET),
		0xa << DACR_PRE_MIXER2_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACL_POST_MIXER2_CTRL1_REG, 0x1 << DACL_POST_MIXER2_FADE_EN_OFFSET,
		0x1 << DACL_POST_MIXER2_FADE_EN_OFFSET, true }, 0, 0 },
	{ { DACL_POST_MIXER2_CTRL2_REG, MASK_ON_BIT(DACL_POST_MIXER2_FADE_IN_LEN,
		DACL_POST_MIXER2_FADE_IN_OFFSET),
		0xf << DACL_POST_MIXER2_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACL_POST_MIXER2_CTRL3_REG, MASK_ON_BIT(DACL_POST_MIXER2_FADE_OUT_LEN,
		DACL_POST_MIXER2_FADE_OUT_OFFSET),
		0xa << DACL_POST_MIXER2_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACR_POST_MIXER2_CTRL1_REG, 0x1 << DACR_POST_MIXER2_FADE_EN_OFFSET,
		0x1 << DACR_POST_MIXER2_FADE_EN_OFFSET, true }, 0, 0 },
	{ { DACR_POST_MIXER2_CTRL2_REG, MASK_ON_BIT(DACR_POST_MIXER2_FADE_IN_LEN,
		DACR_POST_MIXER2_FADE_IN_OFFSET),
		0xf << DACR_POST_MIXER2_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACR_POST_MIXER2_CTRL3_REG, MASK_ON_BIT(DACR_POST_MIXER2_FADE_OUT_LEN,
		DACR_POST_MIXER2_FADE_OUT_OFFSET),
		0xa << DACR_POST_MIXER2_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACSL_MIXER4_CTRL1_REG, 0x1 << DACSL_MIXER4_FADE_EN_OFFSET,
		0x1 << DACSL_MIXER4_FADE_EN_OFFSET, true }, 0, 0 },
	{ { DACSL_MIXER4_CTRL3_REG, MASK_ON_BIT(DACSL_MIXER4_FADE_IN_LEN,
		DACSL_MIXER4_FADE_IN_OFFSET),
		0xf << DACSL_MIXER4_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACSL_MIXER4_CTRL4_REG, MASK_ON_BIT(DACSL_MIXER4_FADE_OUT_LEN,
		DACSL_MIXER4_FADE_OUT_OFFSET),
		0xa << DACSL_MIXER4_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { S2_IL_PGA_CTRL0_REG, 0x0 << S2_IL_PGA_FADE_EN_OFFSET,
		0x1 << S2_IL_PGA_FADE_EN_OFFSET, true }, 0, 0 },
	{ { S2_IL_PGA_CTRL2_REG, MASK_ON_BIT(S2_IL_PGA_FADE_IN_LEN,
		S2_IL_PGA_FADE_IN_OFFSET), 0xc << S2_IL_PGA_FADE_IN_OFFSET, true }, 0, 0 },
	{ { S2_IL_PGA_CTRL3_REG, MASK_ON_BIT(S2_IL_PGA_FADE_OUT_LEN,
		S2_IL_PGA_FADE_OUT_OFFSET), 0xc << S2_IL_PGA_FADE_OUT_OFFSET, true }, 0, 0 },
};

static void pga_fade_init(struct snd_soc_codec *codec)
{
	write_reg_seq_array(codec, pga_fade_regs, ARRAY_SIZE(pga_fade_regs));
}

static void mic_pga_gain_init(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, ANA_HSMIC_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, ANA_AUXMIC_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, ANA_MIC3_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, ANA_MIC4_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, ANA_MIC5_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, ANA_MAD_PGA, 0xf, 0x4);
}

static void adc_pga_gain_init(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, ADC0L_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
	snd_soc_write(codec, ADC0R_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
	snd_soc_write(codec, ADC1L_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
	snd_soc_write(codec, ADC1R_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
	snd_soc_write(codec, ADC2L_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
}

static void mixer_pga_gain_init(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, DACL_MIXER4_CTRL2_REG,
		DACLR_MIXER_PGA_GAIN_DEFAULT);
	snd_soc_write(codec, DACR_MIXER4_CTRL2_REG,
		DACLR_MIXER_PGA_GAIN_DEFAULT);
	hi64xx_update_bits(codec, DACL_PRE_MIXER2_CTRL1_REG, 0x1E, 0xff);
	hi64xx_update_bits(codec, DACR_PRE_MIXER2_CTRL1_REG, 0x1E, 0xff);
	hi64xx_update_bits(codec, DACL_POST_MIXER2_CTRL1_REG, 0x1E, 0xff);
	hi64xx_update_bits(codec, DACR_POST_MIXER2_CTRL1_REG, 0x1E, 0xff);
	snd_soc_write(codec, DACSL_MIXER4_CTRL2_REG,
		DACLR_MIXER_PGA_GAIN_DEFAULT);
	snd_soc_write(codec, DACSR_MIXER4_CTRL2_REG,
		DACLR_MIXER_PGA_GAIN_DEFAULT);
}

static void dac_pga_gain_init(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, DACL_PRE_PGA_CTRL1_REG, 0x6E);/* -5db */
	snd_soc_write(codec, DACR_PRE_PGA_CTRL1_REG, 0x6E);
	snd_soc_write(codec, DACL_POST_PGA_CTRL1_REG, 0x6E);
	snd_soc_write(codec, DACR_POST_PGA_CTRL1_REG, 0x6E);
}

static void adc_init(struct snd_soc_codec *codec)
{
	/* adc source select */
	snd_soc_write(codec, SC_ADC_ANA_SEL_REG, 0x3f);
}

static void hsd_cfg_init(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_078,
		0x3 << HSD_VL_SEL_BIT, 0x2 << HSD_VL_SEL_BIT);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_078,
		0xf << HSD_VH_SEL_BIT, 0x8 << HSD_VH_SEL_BIT);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_079,
		0x1 << HSD_VTH_SEL_BIT, 0x1 << HSD_VTH_SEL_BIT);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_079,
		MASK_ON_BIT(HSD_VTH_MICL_CFG_LEN, HSD_VTH_MICL_CFG_OFFSET),
		HSD_VTH_MICL_CFG_800MV << HSD_VTH_MICL_CFG_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_079,
		MASK_ON_BIT(HSD_VTH_MICH_CFG_LEN, HSD_VTH_MICH_CFG_OFFSET),
		HSD_VTH_MICH_CFG_95 << HSD_VTH_MICH_CFG_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_080,
		MASK_ON_BIT(MBHD_VTH_ECO_CFG_LEN, MBHD_VTH_ECO_CFG_OFFSET),
		MBHD_VTH_ECO_CFG_125MV << MBHD_VTH_ECO_CFG_OFFSET);
}

static void classH_init(struct snd_soc_codec *codec,
	struct hi6405_platform_data *platform_data)
{
	/* broadconfig just controle rcv classH state */
	if (platform_data->board_config.classh_rcv_hp_switch)
		platform_data->rcv_hp_classH_state =
			(unsigned int)platform_data->rcv_hp_classH_state | RCV_CLASSH_STATE;
	else
		platform_data->rcv_hp_classH_state =
			(unsigned int)platform_data->rcv_hp_classH_state & (~RCV_CLASSH_STATE);/*lint !e64*/
	/* headphone default:classH */
	platform_data->rcv_hp_classH_state =
		(unsigned int)platform_data->rcv_hp_classH_state | HP_CLASSH_STATE;
	hi6405_set_classH_config(codec, platform_data->rcv_hp_classH_state);
}

static void mux_init(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
		MASK_ON_BIT(SPA_OL_SRC_DIN_SEL_LEN, SPA_OL_SRC_DIN_SEL_OFFSET) |
		MASK_ON_BIT(SPA_OR_SRC_DIN_SEL_LEN, SPA_OR_SRC_DIN_SEL_OFFSET),
		0x1 << SPA_OL_SRC_DIN_SEL_OFFSET |
		0x1 << SPA_OR_SRC_DIN_SEL_OFFSET);

	/* set dacsl to 96k */
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL38_REG,
		0x1 << DACSL_MIXER4_DVLD_SEL_OFFSET,
		0x1 << DACSL_MIXER4_DVLD_SEL_OFFSET);
}

static const struct reg_seq_config key_gpio_init_regs[] = {
#ifdef CONFIG_VIRTUAL_BTN_SUPPORT
	{ { APB_CLK_CFG_REG, 0x1 << GPIO_PCLK_EN_OFFSET,
		0x1 << GPIO_PCLK_EN_OFFSET, true }, 0, 0 },
	/* GPIO19---KEY_INT */
	{ { IOS_MF_CTRL19_REG, 0x1F << IOS_MF_CTRL19_OFFSET,
		0x8 << IOS_MF_CTRL19_OFFSET, true }, 0, 0 },
	{ { IOS_IOM_CTRL23_REG, 0x3 << PU_OFFSET, 0x0 << PU_OFFSET, true }, 0, 0 },
	{ { IOS_IOM_CTRL23_REG, 0x1 << ST_OFFSET, 0x1 << ST_OFFSET, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2DIR_REG, 0x1 << GPIO2_19_OFFSET,
		0x0 << GPIO2_19_OFFSET, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2IS_REG, 0x1 << GPIO2_19_OFFSET,
		0x0 << GPIO2_19_OFFSET, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2IBE_REG, 0x1 << GPIO2_19_OFFSET,
		0x0 << GPIO2_19_OFFSET, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2IEV_REG, 0x1 << GPIO2_19_OFFSET,
		0x1 << GPIO2_19_OFFSET, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2IE_REG, 0x1 << GPIO2_19_OFFSET,
		0x0 << GPIO2_19_OFFSET, true }, 0, 0 },
	/* GPIO18---PWM_SMT */
	{ { IOS_MF_CTRL18_REG, 0x1F << IOS_MF_CTRL18_OFFSET,
		0x8 << IOS_MF_CTRL18_OFFSET, true }, 0, 0 },
	{ { IOS_IOM_CTRL22_REG, 0x3 << PU_OFFSET, 0x0 << PU_OFFSET, true }, 0, 0 },
	{ { IOS_IOM_CTRL22_REG, 0x7 << DS_OFFSET, 0x1 << DS_OFFSET, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2DIR_REG, 0x1 << GPIO2_18_OFFSET,
		0x1 << GPIO2_18_OFFSET, true }, 0, 0 },
	/* GPIO15---AP_AI */
	{ { IOS_MF_CTRL15_REG, 0x1F << IOS_MF_CTRL15_OFFSET,
		0x8 << IOS_MF_CTRL15_OFFSET, true }, 0, 0 },
	{ { IOS_IOM_CTRL19_REG, 0x3 << PU_OFFSET, 0x0 << PU_OFFSET, true }, 0, 0 },
	{ { IOS_IOM_CTRL19_REG, 0x7 << DS_OFFSET, 0x1 << DS_OFFSET, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO1DIR_REG, 0x1 << GPIO2_15_OFFSET,
		0x1 << GPIO2_15_OFFSET, true }, 0, 0 },
#endif
	{ { ANA_MICBIAS1, 0x1 << ANA_MICBIAS1_DSCHG_OFFSET,
		0x0 << ANA_MICBIAS1_DSCHG_OFFSET, true }, 0, 0 },
	{ { ANA_MICBIAS2, 0x1 << ANA_MICBIAS2_DSCHG_OFFSET,
		0x0 << ANA_MICBIAS2_DSCHG_OFFSET, true }, 0, 0 },
	{ { ANA_HSMICBIAS, 0x1 << ANA_HSMIC_DSCHG_OFFSET,
		0x0 << ANA_HSMIC_DSCHG_OFFSET, true }, 0, 0 },
};

static void key_gpio_init(struct snd_soc_codec *codec)
{
	write_reg_seq_array(codec, key_gpio_init_regs,
		ARRAY_SIZE(key_gpio_init_regs));
}

static void ir_calibration_operation(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_EN_CAL_MT_OFFSET), BIT(CODEC_EN_CAL_MT_OFFSET));
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_053,
		MASK_ON_BIT(CODEC_INF_TRIM_CTRL_REG_LEN,
		CODEC_INF_TRIM_CTRL_REG_OFFSET), 0);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_053,
		BIT(CODEC_OTPREG_SEL_INF_OFFSET), BIT(CODEC_OTPREG_SEL_INF_OFFSET));
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_01,
		BIT(CODEC_PD_INFCAL_CLK_OFFSET), BIT(CODEC_PD_INFCAL_CLK_OFFSET));
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_138,
		BIT(CODEC_RX_CHOP_BPS_OFFSET), 0);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_CLK_CALIB_CFG_OFFSET) |
		MASK_ON_BIT(CODEC_INF_IBCT_CAL_LEN, CODEC_INF_IBCT_CAL_OFFSET), 0);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_RST_CLK_CAL_OFFSET) | BIT(CODEC_RST_CAL_OFFSET),
		BIT(CODEC_RST_CLK_CAL_OFFSET) | BIT(CODEC_RST_CAL_OFFSET));
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_RST_CLK_CAL_OFFSET), 0);
	usleep_range(1100, 1200);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_EN_CAL_MT_OFFSET) | BIT(CODEC_RST_CAL_OFFSET), 0);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_EN_CAL_OFFSET), BIT(CODEC_EN_CAL_OFFSET));
	usleep_range(10000, 11000);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_EN_CAL_OFFSET), 0);
}

static void hp_config(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	if (platform_data == NULL) {
		AUDIO_LOGE("hp config get hi6405 platform data failed");
		return;
	}

	if (!platform_data->board_config.cap_1nf_enable)
		return;

	hi64xx_update_bits(codec, CODEC_ANA_RWREG_056,
		BIT(CODEC_ANA_STB_1N_CT_OFFSET) | BIT(CODEC_ANA_STB_1N_CAP_OFFSET),
		BIT(CODEC_ANA_STB_1N_CT_OFFSET) | BIT(CODEC_ANA_STB_1N_CAP_OFFSET));
}


static void chip_init(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	efuse_init(codec);
	ioshare_init(codec, platform_data);
	hi6405_supply_pll_init(codec);

	slimbus_init(codec, platform_data);
	set_mad_param(codec, &platform_data->board_config);
	set_dsp_if_bypass(codec);
	adc_init(codec);
	pga_fade_init(codec);
	mic_pga_gain_init(codec);
	adc_pga_gain_init(codec);
	mixer_pga_gain_init(codec);
	dac_pga_gain_init(codec);
	hsd_cfg_init(codec);
	classH_init(codec, platform_data);
	key_gpio_init(codec);
	mux_init(codec);
	hp_config(codec);

	OUT_FUNCTION;
}

static unsigned int get_saradc_value(struct snd_soc_codec *codec)
{
	int retry = 3;
	unsigned int sar_value = 0;

	/* sar rst and work */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_078,
		BIT(RST_SAR_BIT), BIT(RST_SAR_BIT));
	udelay(50);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_078,
		BIT(RST_SAR_BIT), 0);
	udelay(50);
	/* saradc on */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_012,
		0x1 << MBHD_SAR_PD_BIT, 0);
	/* start saradc */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_012,
		0x1 << SARADC_START_BIT, 0x1 << SARADC_START_BIT);

	while (retry--) {
		usleep_range(1000, 1100);
		if (hi64xx_check_saradc_ready_detect(codec)) {
			sar_value = snd_soc_read(codec, CODEC_ANA_ROREG_000);
			sar_value = snd_soc_read(codec,
				CODEC_ANA_ROREG_001) + (sar_value << 0x8);
			AUDIO_LOGI("saradc value is %#x", sar_value);

			break;
		}
	}
	if (retry < 0)
		AUDIO_LOGE("get saradc err");

	/* end saradc */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_012,
		0x1 << SARADC_START_BIT, 0x0 << SARADC_START_BIT);
	/* saradc pd */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_012,
		0x1 << MBHD_SAR_PD_BIT, 0x1 << MBHD_SAR_PD_BIT);

	return sar_value;
}

static unsigned int get_voltage_value(struct snd_soc_codec *codec,
	unsigned int voltage_coefficient)
{
	unsigned int sar_value;
	unsigned int voltage_value;

	sar_value = get_saradc_value(codec);
	voltage_value = sar_value * voltage_coefficient / 0xFFF;

	return voltage_value;
}

static void hs_mbhc_on(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	if (platform_data == NULL) {
		AUDIO_LOGE("get hi6405 platform data failed");
		return;
	}

	hi64xx_irq_mask_btn_irqs(platform_data->mbhc);

	/* sar clk use clk32_sys */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_077,
		0x3 << CLK_SAR_SEL_BIT, 0x1 << CLK_SAR_SEL_BIT);
	/* saradc tsmp set 4 * Tclk */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_077,
		0x3 << SAR_TSMP_CFG_BIT, 0x3 << SAR_TSMP_CFG_BIT);
	/* cmp fast mode en */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_077,
		0x3 << SAR_INPUT_SEL_BIT, 0);
	msleep(30);

	hi64xx_irq_unmask_btn_irqs(platform_data->mbhc);

	msleep(120);
}

static void hs_mbhc_off(struct snd_soc_codec *codec)
{
	/* eco off */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_080,
		0x1 << MBHD_ECO_EN_BIT, 0);
	AUDIO_LOGI("eco disable");
}

static void hs_enable_hsdet(struct snd_soc_codec *codec,
	struct hi64xx_mbhc_config mbhc_config)
{
	unsigned int voltage_coefficent;

	if (mbhc_config.coefficient < MBHC_VOLTAGE_COEFFICIENT_MIN ||
		mbhc_config.coefficient > MBHC_VOLTAGE_COEFFICIENT_MAX) {
		/* default set coefficent 2700mv */
		voltage_coefficent = (MBHC_VOLTAGE_COEFFICIENT_DEFAULT -
			MBHC_VOLTAGE_COEFFICIENT_MIN) / 100;
	} else {
		voltage_coefficent = (mbhc_config.coefficient -
			MBHC_VOLTAGE_COEFFICIENT_MIN) / 100;
	}

	hi64xx_update_bits(codec, CODEC_ANA_RWREG_073, 0xF << HSMICB_ADJ,
		voltage_coefficent << HSMICB_ADJ);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_012,
		0x1 << MBHD_PD_MBHD_VTN, 0);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_080, 0x1 << MBHD_HSD_EN_BIT,
		0x1 << MBHD_HSD_EN_BIT);
}

#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
static void hs_res_det_enable(struct snd_soc_codec *codec, bool enable)
{
	if (enable) {
		write_reg_seq_array(codec, enable_res_det_table,
			ARRAY_SIZE(enable_res_det_table));
		hs_mbhc_on(codec);
		AUDIO_LOGI("hi6405_headphone_resdet enable");
	} else {
		hs_mbhc_off(codec);
		write_reg_seq_array(codec, disable_res_det_table,
			ARRAY_SIZE(disable_res_det_table));
		AUDIO_LOGI("hi6405_headphone_resdet disable");
	}
}

static unsigned int get_resvalue(struct snd_soc_codec *codec)
{
	unsigned int res_value;
	unsigned int saradc_value;
	unsigned int min_saradc_value = MAX_SARADC_VAL;
	unsigned int retry = 5;
	unsigned int i;
	unsigned int fb_val; /* feedback res reg val */
	unsigned int vpeak_val; /* vpeak reg val */
	unsigned int r_mir; /* ohm */
	unsigned int vpeak_det; /* mv */

	IN_FUNCTION;
	hs_res_det_enable(codec, true);
	write_reg_seq_array(codec, enable_get_res_table,
		ARRAY_SIZE(enable_get_res_table));

	fb_val = get_high_res_data(HIGH_RES_GET_FB_VAL);
	vpeak_val = get_high_res_data(HIGH_RES_GET_OUTPUT_AMP);
	AUDIO_LOGI("fb_val = 0x%x, vpeak_val = 0x%x", fb_val, vpeak_val);
	/* hs det res cfg */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_065,
		0x3 << CODEC_ANA_HP_IMPDET_RES_CFG_OFFSET,
		fb_val << CODEC_ANA_HP_IMPDET_RES_CFG_OFFSET);
	snd_soc_write(codec, DACL_PRE_PGA_CTRL1_REG, vpeak_val);

	hi6405_headphone_pop_on(codec);
	for (i = 0; i < MAX_DAC_POST_PGA_GAIN + 1; i += 2) {
		snd_soc_write(codec, DACL_POST_PGA_CTRL1_REG, i);
		udelay(100);
	}
	write_reg_seq_array(codec, hp_impdet_vpout_table,
		ARRAY_SIZE(hp_impdet_vpout_table));
	/* wait for stable saradc_value */
	msleep(50);
	for (i = 0; i < retry; i++) {
		mdelay(2);
		saradc_value = get_saradc_value(codec);
		if (min_saradc_value > saradc_value)
			min_saradc_value = saradc_value;
	}

	/* r_mir=res_calib*2/2^r_fb */
	r_mir = (get_high_res_data(HIGH_RES_GET_CALIB_VAL)) *
		HS_DET_R_RATIO / (1 << fb_val);
	/* vpeak_det=(saradc/sar_max_val)*micbias */
	vpeak_det = min_saradc_value * HS_DET_MICBIAS / MAX_SARADC_VAL;
	/* RL=R_MIR*Vpeak/(Vpeak_det+Vpeak)/Iratio */
	res_value = r_mir * HS_DET_DEFAULT_VPEAK /
		(vpeak_det + HS_DET_DEFAULT_VPEAK) / HS_DET_I_RATIO;

	AUDIO_LOGI("feedback_det_val = %u, vpeak_det = %u, res_value = %u",
		r_mir, vpeak_det, res_value);
#ifdef CONFIG_HUAWEI_DSM
	(void)audio_dsm_report_info(AUDIO_CODEC, DSM_HIFI_AK4376_CODEC_PROBE_ERR,
		"feedback_det_val = %u, saradc = %x, res_value = %u",
		r_mir, min_saradc_value, res_value);
#endif
	for (i = 0; i < MAX_DAC_POST_PGA_GAIN + 1; i += 2) {
		snd_soc_write(codec, DACL_POST_PGA_CTRL1_REG, MAX_DAC_POST_PGA_GAIN - i);
		udelay(100);
	}
	write_reg_seq_array(codec, disable_get_res_table,
		ARRAY_SIZE(disable_get_res_table));
	hs_res_det_enable(codec, false);
	OUT_FUNCTION;
	return res_value;
}

void imp_path_enable(struct snd_soc_codec *codec, bool enable)
{
	if (enable) {
		write_reg_seq_array(codec, enable_path_table,
			ARRAY_SIZE(enable_path_table));
		hi6405_set_classH_config(codec, HP_POWER_STATE & (~HP_CLASSH_STATE));
		AUDIO_LOGI("hi6405_imp_path enable");
	} else {
		hi6405_headphone_pop_off(codec);
		hi6405_set_classH_config(codec, HP_POWER_STATE | HP_CLASSH_STATE);
		write_reg_seq_array(codec, disable_path_table,
			ARRAY_SIZE(disable_path_table));
		AUDIO_LOGI("hi6405_imp_path disable");
	}
}
#endif /* CONFIG_HIGH_RESISTANCE_HS_DET */

static void hs_path_enable(struct snd_soc_codec *codec)
{
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;
	hi64xx_resmgr_request_pll(priv->resmgr, PLL_HIGH);
	request_cp_dp_clk(codec);
	imp_path_enable(codec, true);
	OUT_FUNCTION;
#endif
}

static void hs_res_detect(struct snd_soc_codec *codec)
{
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
	unsigned int res_val;
	unsigned int hs_status;

	IN_FUNCTION;
	if (!check_high_res_hs_det_support()) {
		AUDIO_LOGI("not support hs res hs det");
		return;
	}
	res_val = get_resvalue(codec);
	if (res_val < (unsigned int)get_high_res_data(HIGH_RES_GET_MIN_THRESHOLD)) {
		AUDIO_LOGI("normal headset");
		hs_status = NORMAL_HS;
	} else if (res_val < (unsigned int)get_high_res_data(HIGH_RES_GET_MAX_THRESHOLD)) {
		AUDIO_LOGI("set as normal headset");
		hs_status = NORMAL_HS;
	} else {
		AUDIO_LOGI("high res headset");
		hs_status = HIGH_RES_HS;
	}
	AUDIO_LOGI("high resistance headset status is %u", hs_status);
	set_high_res_data(HIGH_RES_SET_HS_STATE, hs_status);
	OUT_FUNCTION;
#endif
}

static void hs_path_disable(struct snd_soc_codec *codec)
{
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;
	if (priv->hsl_power_on == false || priv->hsr_power_on == false) {
		imp_path_enable(codec, false);
		AUDIO_LOGI("headset path is open, no need to close");
	}
	release_cp_dp_clk(codec);
	hi64xx_resmgr_release_pll(priv->resmgr, PLL_HIGH);
	OUT_FUNCTION;
#endif
}

static void hs_res_calib(struct snd_soc_codec *codec)
{
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
	unsigned int res_calib_status;
	unsigned int saradc_value;
	unsigned int res_calib_val;

	IN_FUNCTION;
	if (!check_high_res_hs_det_support()) {
		AUDIO_LOGI("not support hs res hs det");
		return;
	}
	res_calib_status = get_high_res_data(HIGH_RES_GET_CALIB_STATE);
	if (res_calib_status == RES_NOT_CALIBRATED) {
		hi6405_headphone_pop_on(codec);
		hs_res_det_enable(codec, true);
		AUDIO_LOGI("hs resistance need calibration");
		write_reg_seq_array(codec, enable_res_calib_table,
			ARRAY_SIZE(enable_res_calib_table));
		/* wait for stable saradc_value */
		msleep(10);
		saradc_value = get_saradc_value(codec);
		AUDIO_LOGI("saradc_value = %u", saradc_value);
		/* res_calib=inner_res*saradc/(saradc_maxval-saradc) */
		res_calib_val = (HS_DET_INNER_RES * saradc_value) /
			(MAX_SARADC_VAL - saradc_value);
		AUDIO_LOGI("res_calib_val = %u", res_calib_val);
		set_high_res_data(HIGH_RES_SET_CALIB_VAL, res_calib_val);
		write_reg_seq_array(codec, disable_res_calib_table,
			ARRAY_SIZE(disable_res_calib_table));
		set_high_res_data(HIGH_RES_SET_CALIB_STATE, RES_CALIBRATED);
		hs_res_det_enable(codec, false);
	}
	AUDIO_LOGI("end");
	OUT_FUNCTION;
#endif
}

static struct hs_mbhc_reg hs_mbhc_reg = {
	.irq_source_reg = CODEC_BASE_ADDR + CODEC_ANA_IRQ_SRC_STAT_REG,
	.irq_mbhc_2_reg = IRQ_REG2_REG,
};

static struct hs_mbhc_func hs_mbhc_func = {
	.hs_mbhc_on =  hs_mbhc_on,
	.hs_get_voltage = get_voltage_value,
	.hs_enable_hsdet = hs_enable_hsdet,
	.hs_mbhc_off =  hs_mbhc_off,
};

static struct hs_res_detect_func hs_res_detect_func = {
	.hs_res_detect = hs_res_detect,
	.hs_path_enable = hs_path_enable,
	.hs_path_disable = hs_path_disable,
	.hs_res_calibration = hs_res_calib,
};

static struct hs_res_detect_func hs_res_detect_func_null = {
	.hs_res_detect = NULL,
	.hs_path_enable = NULL,
	.hs_path_disable = NULL,
	.hs_res_calibration = NULL,
};

static struct hi64xx_hs_cfg hs_cfg = {
	.mbhc_reg = &hs_mbhc_reg,
	.mbhc_func = &hs_mbhc_func,
	.res_detect_func = &hs_res_detect_func_null,
};

static int codec_add_driver_resource(struct snd_soc_codec *codec)
{
	int ret = hi6405_add_kcontrol(codec);

	if (ret != 0) {
		AUDIO_LOGE("hi6405 add kcontrols failed, ret = %d", ret);
		goto exit;
	}

	ret = hi6405_add_resource_widgets(codec);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 add resource widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = hi6405_add_path_widgets(codec);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 add path widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = hi6405_add_switch_widgets(codec);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 add switch widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = hi6405_add_routes(codec);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 add route map failed, ret = %d", ret);
		goto exit;
	}

exit:
	return ret;
}

static int codec_init(struct snd_soc_codec *codec,
	struct hi6405_platform_data *data)
{
	int ret = hi64xx_mbhc_init(codec, data->node, &hs_cfg,
		data->resmgr, data->irqmgr, &data->mbhc);
	if (ret != 0) {
		AUDIO_LOGE("hifi config fail: 0x%x", ret);
		goto mbhc_init_failed;
	}

	ret = hi6405_dsp_config_init(codec, data->resmgr,
		data->irqmgr, data->cdc_ctrl->bus_sel);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 dsp init failed:0x%x", ret);
		goto misc_init_failed;
	}

	ret = utils_init(data);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 utils init failed:0x%x", ret);
		goto utils_init_failed;
	}

	ret = hi64xx_vad_init(codec, data->irqmgr);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 vad init failed:0x%x", ret);
		goto vad_init_failed;
	}

	ret = slim_enumerate(data);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 slim enumerate failed:0x%x", ret);
		goto slim_enumerate_failed;
	}

	return ret;

slim_enumerate_failed:
	hi64xx_vad_deinit();
vad_init_failed:
	hi64xx_utils_deinit();
utils_init_failed:
	hi6405_dsp_config_deinit();
misc_init_failed:
	hi64xx_mbhc_deinit(data->mbhc);
mbhc_init_failed:
	return ret;
}

static void codec_deinit(struct hi6405_platform_data *data)
{
	hi64xx_vad_deinit();
	hi64xx_utils_deinit();
	hi6405_dsp_config_deinit();
	hi64xx_mbhc_deinit(data->mbhc);
}

static void ir_calibration(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);
	hi64xx_resmgr_request_pll(priv->resmgr, PLL_HIGH);
	/* dp clk enable */
	request_dp_clk(codec, true);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_052,
		BIT(CODEC_PD_INF_LRN_OFFSET), BIT(CODEC_PD_INF_LRN_OFFSET));
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_052,
		BIT(CODEC_PD_INF_EMS_OFFSET), 0);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_051,
		MASK_ON_BIT(CODEC_FIR_OUT_CTRL_LEN, CODEC_FIR_OUT_CTRL_OFFSET) |
		MASK_ON_BIT(CODEC_FIR_ATE_CTRL_LEN, CODEC_FIR_ATE_CTRL_OFFSET),
		CODEC_FIR_OUT_X15 << CODEC_FIR_OUT_CTRL_OFFSET |
		CODEC_FIR_ATE_XF << CODEC_FIR_ATE_CTRL_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_LEAK_CTRL_OFFSET), BIT(CODEC_LEAK_CTRL_OFFSET));

	ir_calibration_operation(codec);

	hi64xx_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_LEAK_CTRL_OFFSET), 0);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_051,
		MASK_ON_BIT(CODEC_FIR_OUT_CTRL_LEN, CODEC_FIR_OUT_CTRL_OFFSET) |
		MASK_ON_BIT(CODEC_FIR_ATE_CTRL_LEN, CODEC_FIR_ATE_CTRL_OFFSET),
		CODEC_FIR_OUT_NON << CODEC_FIR_OUT_CTRL_OFFSET |
		CODEC_FIR_ATE_X1 << CODEC_FIR_ATE_CTRL_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_052,
		BIT(CODEC_PD_INF_EMS_OFFSET), BIT(CODEC_PD_INF_EMS_OFFSET));
	/* dp clk disable */
	request_dp_clk(codec, false);
	hi64xx_resmgr_release_pll(priv->resmgr, PLL_HIGH);
	AUDIO_LOGI("hi6405 ir calibration end");
}

static int hi6405_codec_probe(struct snd_soc_codec *codec)
{
	int ret;
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;
	if (platform_data == NULL) {
		AUDIO_LOGE("get hi6405 platform data failed");
		return -ENOENT;
	}

	snd_soc_codec_set_drvdata(codec, platform_data);
	platform_data->codec = codec;
	hi6405_codec = codec;

	ret = hi6405_resmgr_init(platform_data);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 resmgr init failed:0x%x", ret);
		return -ENOMEM;
	}

	AUDIO_LOGI("hi6405 version:0x%x", snd_soc_read(codec, VERSION_REG));

	chip_init(codec);

	if (platform_data->board_config.hp_res_detect_enable) {
		AUDIO_LOGI("hs res detect support");
		hs_cfg.res_detect_func = &hs_res_detect_func;
	}

	ret = codec_init(codec, platform_data);
	if (ret != 0) {
		hi64xx_resmgr_deinit(platform_data->resmgr);
		AUDIO_LOGE("hi6405 codec probe failed");
		return ret;
	}


#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	ret = hicodec_debug_init(codec, &dump_info);
	if (ret != 0)
		AUDIO_LOGI("codec debug init failed: 0x%x", ret);
#endif

	ret = codec_add_driver_resource(codec);
	if (ret != 0) {
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
		hicodec_debug_uninit(codec);
#endif
		codec_deinit(platform_data);
		hi64xx_resmgr_deinit(platform_data->resmgr);
		AUDIO_LOGE("hi6405 add codec driver resource fail: %d", ret);
		return ret;
	}

	ir_calibration(codec);
	AUDIO_LOGI("hi6405 codec probe ok");

	return ret;
}

static int hi6405_codec_remove(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	if (platform_data == NULL) {
		AUDIO_LOGE("get hi6405 platform data failed");
		return -ENOENT;
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif

	codec_deinit(platform_data);
	hi64xx_resmgr_deinit(platform_data->resmgr);

	return 0;
}

static int hi6405_audio_hw_params(struct snd_pcm_substream *substream,
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

static int hi6405_audio_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

struct snd_soc_dai_ops hi6405_audio_dai_ops = {
	.hw_params = hi6405_audio_hw_params,
	.hw_free = hi6405_audio_hw_free,
};

static int hi6405_voice_hw_params(struct snd_pcm_substream *substream,
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

static int hi6405_voice_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

struct snd_soc_dai_ops hi6405_voice_dai_ops = {
	.hw_params = hi6405_voice_hw_params,
	.hw_free = hi6405_voice_hw_free,
};

struct snd_soc_dai_driver hi6405_dai[] = {
	{
		.name = "hi6405-audio-dai",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 2,
			.channels_max = 4,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS },
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 9,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS },
		.ops = &hi6405_audio_dai_ops,
	},
	{
		.name = "hi6405-voice-dai",
		.playback = {
			.stream_name = "Down",
			.channels_min = 1,
			.channels_max = 2,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS },
		.capture = {
			.stream_name = "Up",
			.channels_min = 1,
			.channels_max = 6,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS },
		.ops = &hi6405_voice_dai_ops,
	},
	{
		.name = "hi6405-fm-dai",
		.playback = {
			.stream_name = "FM",
			.channels_min = 1,
			.channels_max = 2,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS },
	},
};

static struct snd_soc_codec_driver hi6405_codec_driver = {
	.probe = hi6405_codec_probe,
	.remove = hi6405_codec_remove,
	.read = hi6405_reg_read,
	.write = hi6405_reg_write,
};

static bool check_card_valid(struct hi6405_platform_data *platform_data)
{
	unsigned int val = hi_cdcctrl_reg_read(platform_data->cdc_ctrl,
		HI64XX_VERSION_REG);

	if (val != HI64XX_VERSION_CS) {
		AUDIO_LOGE("read hi6405 version failed:0x%x", val);
		return false;
	}

	return true;
}

static void get_board_micnum(struct device_node *node,
	struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;

	if (!of_property_read_u32(node, "hisilicon,mic_num", &val))
		board_cfg->mic_num = val;
	else
		board_cfg->mic_num = 2;
}

static void get_board_hpswitch(struct device_node *node,
	struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;

	if (!of_property_read_u32(node, "hisilicon,classh_rcv_hp_switch", &val)) {
		if (val != 0)
			board_cfg->classh_rcv_hp_switch = true;
		else
			board_cfg->classh_rcv_hp_switch = false;
	} else {
		board_cfg->classh_rcv_hp_switch = false;
	}
}

static void get_board_micmap(struct device_node *node,
	struct hi6405_board_cfg *board_cfg)
{
	int ret;
	unsigned int i;
	unsigned int *map = board_cfg->mic_map;

	ret = of_property_read_u32_array(node, "hisilicon,mic_map",
		map, TOTAL_MACHINE_CALIB_MIC);
	if (ret) {
		AUDIO_LOGE("mic map read error\n");
		goto mic_map_err;
	}
	/* check the validation */
	for (i = 0; i < TOTAL_MACHINE_CALIB_MIC; i++) {
		if (map[i] >= TOTAL_CODEC_CALIB_MIC) {
			AUDIO_LOGE("mic map values are not valid\n");
			goto mic_map_err;
		}
	}
	return;
mic_map_err:
	for (i = 0; i < TOTAL_MACHINE_CALIB_MIC; i++)
		*(map++) = i;
}

#ifdef CONFIG_HAC_SUPPORT
int hac_gpio_init(int hac_gpio)
{
	if (!gpio_is_valid(hac_gpio)) {
		AUDIO_LOGE("hac Value is not valid");
		return -1;
	}
	if (gpio_request(hac_gpio, "hac_en_gpio")) {
		AUDIO_LOGE("hac gpio request failed");
		return -1;
	}
	if (gpio_direction_output(hac_gpio, 0)) {
		AUDIO_LOGE("hac gpio set output failed");
		return -1;
	}

	return 0;
}

static void get_board_hac(struct device_node *node,
	struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;
	int ret = 0;

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

static void get_board_wakeup_hisi_algo_support(struct device_node *node,
	struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;

	board_cfg->wakeup_hisi_algo_support = false;
	if (!of_property_read_u32(node, "hisilicon,wakeup_hisi_algo_support", &val)) {
		if (val != 0)
			board_cfg->wakeup_hisi_algo_support = true;
	}
}

static void get_board_hp_res_detect(struct device_node *node,
			struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;

	board_cfg->hp_res_detect_enable = false;
	if (of_property_read_u32(node, "hisilicon,hp_res_detect_enable", &val))
		return;

	if (val != 0)
		board_cfg->hp_res_detect_enable = true;

}

static void get_board_mic_control_sc_frequency(struct device_node *node,
	struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;

	board_cfg->mic_control_sc_freq_enable = false;
	if (of_property_read_u32(node, "mic_control_sc_frequency", &val))
		return;

	if (val != 0)
		board_cfg->mic_control_sc_freq_enable = true;
}

static void get_board_cap_1nf_enable(struct device_node *node,
	struct hi6405_board_cfg *board_cfg)
{
	board_cfg->cap_1nf_enable = false;
	if (of_property_read_bool(node, "cap_1nf_enable"))
		board_cfg->cap_1nf_enable = true;
}

static void get_board_headphone_pop_on_delay(struct device_node *node,
	struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;

	board_cfg->headphone_pop_on_delay_enable = false;
	if (of_property_read_u32(node, "headphone_pop_on_delay", &val))
		return;

	if (val != 0)
		board_cfg->headphone_pop_on_delay_enable = true;
}

static void get_board_cfg(struct device_node *node,
	struct hi6405_board_cfg *board_cfg)
{
	get_board_micnum(node, board_cfg);
	get_board_hpswitch(node, board_cfg);
	get_board_micmap(node, board_cfg);
#ifdef CONFIG_HAC_SUPPORT
	get_board_hac(node, board_cfg);
	AUDIO_LOGI("hac_gpio %d", board_cfg->hac_gpio);
#endif
	AUDIO_LOGI("mic_num %d", board_cfg->mic_num);
	get_board_wakeup_hisi_algo_support(node, board_cfg);
	AUDIO_LOGI("wakeup_hisi_algo_support %d",
		board_cfg->wakeup_hisi_algo_support);
	get_board_hp_res_detect(node, board_cfg);
	AUDIO_LOGI("hp_res_detect %d", board_cfg->hp_res_detect_enable);
	get_board_mic_control_sc_frequency(node, board_cfg);
	AUDIO_LOGI("mic control sc frequency %d",
		board_cfg->mic_control_sc_freq_enable);
	get_board_cap_1nf_enable(node, board_cfg);
	AUDIO_LOGI("cap 1nf enable %d",
		board_cfg->cap_1nf_enable);
	get_board_headphone_pop_on_delay(node, board_cfg);
	AUDIO_LOGI("headphone pop on delay %d",
		board_cfg->headphone_pop_on_delay_enable);
}

static int init_platform_data(struct platform_device *pdev,
	struct hi6405_platform_data *platform_data)
{
	const struct of_device_id *match = NULL;
	int i;

	platform_data->irqmgr = (struct hi64xx_irq *)dev_get_drvdata(pdev->dev.parent);
	platform_data->cdc_ctrl = (struct hi_cdc_ctrl *)dev_get_drvdata(pdev->dev.parent->parent);

	if (!check_card_valid(platform_data))
		return -ENODEV;

	match = of_match_device(hi6405_platform_match, &pdev->dev);
	if (match == NULL) {
		AUDIO_LOGE("get device info err");
		return -ENOENT;
	}

	platform_data->cdc_ctrl->ssi_check_enable = SSI_STATE_CHECK_DISABLE;
	platform_data->node = pdev->dev.of_node;
	get_board_cfg(platform_data->node, &platform_data->board_config);
	platform_data->voice_up_params.channels = 2;
	/* set channel as 4, when mic num exceeds 2 */
	if (platform_data->board_config.mic_num > 2)
		platform_data->voice_up_params.channels = 4;
	platform_data->voice_down_params.channels = 2;
	platform_data->capture_params.channels = 2;
	platform_data->capture_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	platform_data->soundtrigger_params.rate = SLIMBUS_SAMPLE_RATE_16K;
	platform_data->soundtrigger_params.channels = 1;
	platform_data->voiceup_state = TRACK_FREE;
	platform_data->voiceup_v2_state = TRACK_FREE;
	platform_data->audioup_4mic_state = TRACK_FREE;
	platform_data->audioup_4mic_v2_state = TRACK_FREE;
	platform_data->audioup_5mic_state = TRACK_FREE;
	platform_data->play_params.channels = 2;
	platform_data->play_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	platform_data->pa_iv_params.channels = 4;
	platform_data->pa_iv_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	for (i = 0; i < TOTAL_CODEC_CALIB_MIC; i++)
		platform_data->mic_calib_value[i] = MIC_CALIB_NONE_VALUE;

	spin_lock_init(&platform_data->v_rw_lock);
	mutex_init(&platform_data->impdet_dapm_mutex);

	platform_set_drvdata(pdev, platform_data);
	dev_set_name(&pdev->dev, "hi6405-codec");

	platform_data->is_madswitch_on = false;
	platform_data->is_callswitch_on = false;
	if (platform_data->board_config.headphone_pop_on_delay_enable)
		INIT_DELAYED_WORK(&(platform_data->headphone_pop_on_delay),
			headphone_pop_on_delay_work);

	return 0;
}

static inline void deinit_platform_data(struct hi6405_platform_data *platform_data)
{
	mutex_destroy(&platform_data->impdet_dapm_mutex);
}

static struct hi64xx_irq_map irqs = {
	{ IRQ_REG0_REG, IRQ_REG1_REG, IRQ_REG2_REG, IRQ_REG3_REG,
		IRQ_REG4_REG, IRQ_REG5_REG, IRQ_REG6_REG },
	{ IRQM_REG0_REG, IRQM_REG1_REG, IRQM_REG2_REG, IRQM_REG3_REG,
		IRQM_REG4_REG, IRQM_REG5_REG, IRQM_REG6_REG },
	IRQ_NUM,
};

static inline void dsm_report(int dsm_type, char *str_error)
{
#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, dsm_type, str_error);
#endif
}

static void dsp_power_down(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_DSP_CTRL0_REG,
		0x1 << SC_DSP_SFT_RUNSTALL_OFFSET | 0x1 << SC_DSP_EN_OFFSET |
		0x1 << SC_HIFI_CLK_EN_OFFSET | 0x1 << SC_HIFI_ACLK_EN_OFFSET,
		0x1 << SC_DSP_SFT_RUNSTALL_OFFSET | 0x0 << SC_DSP_EN_OFFSET |
		0x0 << SC_HIFI_CLK_EN_OFFSET | 0x0 << SC_HIFI_ACLK_EN_OFFSET);
	hi64xx_update_bits(codec, APB_CLK_CFG_REG,
		0x1 << APB_PD_PCLK_EN_OFFSET, 0x0 << APB_PD_PCLK_EN_OFFSET);
	snd_soc_write(codec, DSP_IF_CLK_EN, 0x0);
	hi64xx_update_bits(codec, SW_RST_REQ_REG,
		0x1 << DSP_PD_SRST_REQ_OFFSET, 0x1 << DSP_PD_SRST_REQ_OFFSET);
	hi64xx_update_bits(codec, DSP_LP_CTRL0_REG,
		0x1 << DSP_TOP_ISO_CTRL_OFFSET | 0x1 << DSP_TOP_MTCMOS_CTRL_OFFSET,
		0x1 << DSP_TOP_ISO_CTRL_OFFSET | 0x1 << DSP_TOP_MTCMOS_CTRL_OFFSET);
}

#ifdef CONFIG_HUAWEI_DSM
static void irq_handler(char *irq_name, unsigned int reg,
	unsigned int reg_offset, int dsm_type, void *data)
{
	struct hi6405_platform_data *platform_data = (struct hi6405_platform_data *)(data);
	struct snd_soc_codec *codec = platform_data->codec;

	if (codec != NULL) {
		AUDIO_LOGW("hi6405: %s irq receive", irq_name);
		snd_soc_write(codec, reg, 0x1 << reg_offset);
		dsm_report(dsm_type, irq_name);
	}
}
#else
static void irq_handler(const char *irq_name, unsigned int reg,
	unsigned int reg_offset, int dsm_type, void *data)
{

}
#endif

static irqreturn_t bunk1_ocp_handler(int irq, void *data)
{
	irq_handler("64xx codec bunk1_ocp", IRQ_REG5_REG,
		PMU_BUNK1_OCP_IRQ_OFFSET, DSM_CODEC_BUNK1_OCP, data);
	return IRQ_HANDLED;
}

static irqreturn_t bunk1_scp_handler(int irq, void *data)
{
	irq_handler("64xx codec bunk1_scp", IRQ_REG5_REG,
		PMU_BUNK1_SCP_IRQ_OFFSET, DSM_CODEC_BUNK1_SCP, data);
	return IRQ_HANDLED;
}

static irqreturn_t ldo_avdd18_ocp_handler(int irq, void *data)
{
	irq_handler("64xx codec ldo_avdd18_ocp", IRQ_REG5_REG,
		PMU_LDO_AVDD18_OCP_IRQ_OFFSET, DSM_CODEC_LDO_AVDD18_OCP, data);
	return IRQ_HANDLED;
}

static irqreturn_t ldop_ocp_handler(int irq, void *data)
{
	irq_handler("64xx codec ldop_ocp", IRQ_REG5_REG,
		PMU_LDOP_OCP_IRQ_OFFSET, DSM_CODEC_LDOP_OCP, data);
	return IRQ_HANDLED;
}

static irqreturn_t ldon_ocp_handler(int irq, void *data)
{
	irq_handler("64xx codec ldon_ocp", IRQ_REG5_REG,
		PMU_LDON_OCP_IRQ_OFFSET, DSM_CODEC_LDON_OCP, data);
	return IRQ_HANDLED;
}

static irqreturn_t cp1_short_handler(int irq, void *data)
{
	irq_handler("64xx codec cp1_short", IRQ_REG5_REG,
		PMU_CP1_SHORT_IRQ_OFFSET, DSM_CODEC_CP1_SHORT, data);
	return IRQ_HANDLED;
}

static irqreturn_t cp2_short_handler(int irq, void *data)
{
	irq_handler("64xx codec cp2_short", IRQ_REG5_REG,
		PMU_CP2_SHORT_IRQ_OFFSET, DSM_CODEC_CP2_SHORT, data);
	return IRQ_HANDLED;
}

static const struct reg_seq_config pll_unlock_regs[] = {
	{ { PLL_TEST_CTRL1_REG, 0x1 << PLL_FIFO_CLK_EN_OFFSET,
		0x1 << PLL_FIFO_CLK_EN_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_193, 0x1 << CODEC_ANA_TEST_REF_CLK_CG_EN_OFFSET,
		0x1 << CODEC_ANA_TEST_REF_CLK_CG_EN_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_200, 0x1 << CODEC_ANA_TEST_SOFT_RST_N_OFFSET,
		0x1 << CODEC_ANA_TEST_SOFT_RST_N_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_193, 0x1 << CODEC_ANA_TEST_PLL_SEL_OFFSET |
		0x1 << CODEC_ANA_TEST_DCO_OPEN_EN_OFFSET |
		0x1 << CODEC_ANA_TEST_PLL_CLOSE_EN_OFFSET |
		0x7 << CODEC_ANA_TEST_PLL_CLOSE_CAPTUTE_MODE_OFFSET,
		0x0 << CODEC_ANA_TEST_PLL_SEL_OFFSET |
		0x1 << CODEC_ANA_TEST_DCO_OPEN_EN_OFFSET |
		0x1 << CODEC_ANA_TEST_PLL_CLOSE_EN_OFFSET |
		0x1 << CODEC_ANA_TEST_PLL_CLOSE_CAPTUTE_MODE_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_194, 0, 0x07, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_195, 0, 0x07, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_196, 0x1 << CODEC_ANA_PVT_SEL_OFFSET |
		0x7 << CODEC_ANA_TEST_LOOP_I_OFFSET |
		0xf << CODEC_ANA_TEST_FREQ_CALC_CNT_TIME_OFFSET,
		0x1 << CODEC_ANA_PVT_SEL_OFFSET |
		0x1 << CODEC_ANA_TEST_LOOP_I_OFFSET |
		0xf << CODEC_ANA_TEST_FREQ_CALC_CNT_TIME_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_197, 0, 0xff, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_198, 0, 0xf, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_199, 0, 0xa0, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_200, 0x3 << CODEC_ANA_TEST_SCAN_CNT_TIME_OFFSET,
		0x2 << CODEC_ANA_TEST_SCAN_CNT_TIME_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_201, 0, 0x0, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_202, 0, 0xff, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_167, 0x7f << CODEC_ANA_MAIN1_TEST_TUNE_FINE_OFFSET,
		0x40 << CODEC_ANA_TEST_SCAN_CNT_TIME_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_168, 0x3f << CODEC_ANA_MAIN1_TEST_TUNE_PVT_OFFSET,
		0x20 << CODEC_ANA_MAIN1_TEST_TUNE_PVT_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_190, 0x7f << CODEC_ANA_MAIN2_TEST_TUNE_FINE_OFFSET,
		0x40 << CODEC_ANA_MAIN2_TEST_TUNE_FINE_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_191, 0x3f << CODEC_ANA_MAIN2_TEST_TUNE_PVT_OFFSET,
		0x20 << CODEC_ANA_MAIN2_TEST_TUNE_PVT_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_193, 0x1 << CODEC_ANA_TEST_MODE_EN_OFFSET,
		0x0 << CODEC_ANA_TEST_MODE_EN_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_193, 0x1 << CODEC_ANA_TEST_MODE_EN_OFFSET,
		0x1 << CODEC_ANA_TEST_MODE_EN_OFFSET, true }, 0, 0 },
};

static irqreturn_t pll_unlock_handler(int irq, void *data)
{
	struct hi6405_platform_data *platform_data = (struct hi6405_platform_data *)(data);
	struct snd_soc_codec *codec = platform_data->codec;
	unsigned char output_str[PLL_DATA_ALL_NUM] = {0};
	unsigned char buf[PLL_DATA_BUF_SIZE] = {0};
	unsigned int i;
	unsigned int j;

	if (codec != NULL) {
		AUDIO_LOGW("pll unlock irq received");
		snd_soc_write(codec, IRQ_REG2_REG,
			0x1 << PLL_48K_UNLOCK_F_IRQ_OFFSET);
#ifdef CONFIG_HUAWEI_DSM
		dsm_report(DSM_HI6402_PLL_UNLOCK, "64xx codec pll_unlock\n");
#endif
		write_reg_seq_array(codec, pll_unlock_regs,
			ARRAY_SIZE(pll_unlock_regs));

		msleep(100);
		dsp_power_down(codec);
		for (i = 0; i < PLL_DATA_ALL_NUM; i = i + PLL_DATA_GROUP_NUM) {
			memset(output_str, 0, sizeof(output_str));
			memset(buf, 0, sizeof(buf));
			for (j = 0; j < PLL_DATA_GROUP_NUM; ++j) {
				snprintf(buf, sizeof(buf), "%9x",
					snd_soc_read(codec, PLL_FIFO));
				strncat(output_str, buf, strlen(buf));
			}
			AUDIO_LOGI("pll main1 PLL data:%s", output_str);
		}
		hi64xx_wtdog_send_event();
	}
	return IRQ_HANDLED;
}

static const struct reg_seq_config pll44k1_unlock_regs[] = {
	{ { PLL_TEST_CTRL1_REG, 0x1 << PLL_FIFO_CLK_EN_OFFSET,
		0x1 << PLL_FIFO_CLK_EN_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_193, 0x1 << CODEC_ANA_TEST_REF_CLK_CG_EN_OFFSET,
		0x1 << CODEC_ANA_TEST_REF_CLK_CG_EN_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_200, 0x1 << CODEC_ANA_TEST_SOFT_RST_N_OFFSET,
		0x1 << CODEC_ANA_TEST_SOFT_RST_N_OFFSET, 1, }, 0, 0 },
	{ { CODEC_ANA_RWREG_193, 0x1 << CODEC_ANA_TEST_PLL_SEL_OFFSET |
		0x1 << CODEC_ANA_TEST_DCO_OPEN_EN_OFFSET |
		0x1 << CODEC_ANA_TEST_PLL_CLOSE_EN_OFFSET |
		0x7 << CODEC_ANA_TEST_PLL_CLOSE_CAPTUTE_MODE_OFFSET,
		0x1 << CODEC_ANA_TEST_PLL_SEL_OFFSET |
		0x1 << CODEC_ANA_TEST_DCO_OPEN_EN_OFFSET |
		0x1 << CODEC_ANA_TEST_PLL_CLOSE_EN_OFFSET |
		0x1 << CODEC_ANA_TEST_PLL_CLOSE_CAPTUTE_MODE_OFFSET, 1, }, 0, 0 },
	{ { CODEC_ANA_RWREG_194, 0, 0x7, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_195, 0, 0x7, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_196, 0x1 << CODEC_ANA_PVT_SEL_OFFSET |
		0x7 << CODEC_ANA_TEST_LOOP_I_OFFSET |
		0xf << CODEC_ANA_TEST_FREQ_CALC_CNT_TIME_OFFSET,
		0x1 << CODEC_ANA_PVT_SEL_OFFSET |
		0x1 << CODEC_ANA_TEST_LOOP_I_OFFSET |
		0xf << CODEC_ANA_TEST_FREQ_CALC_CNT_TIME_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_197, 0, 0xff, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_198, 0, 0xf, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_199, 0, 0xa0, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_200,
		0x3 << CODEC_ANA_TEST_SCAN_CNT_TIME_OFFSET,
		0x2 << CODEC_ANA_TEST_SCAN_CNT_TIME_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_201, 0, 0x0, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_202, 0, 0xff, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_167, 0x7f << CODEC_ANA_MAIN1_TEST_TUNE_FINE_OFFSET,
		0x40 << CODEC_ANA_MAIN1_TEST_TUNE_FINE_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_168, 0x3f << CODEC_ANA_MAIN1_TEST_TUNE_PVT_OFFSET,
		0x20 << CODEC_ANA_MAIN1_TEST_TUNE_PVT_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_190, 0x7f << CODEC_ANA_MAIN2_TEST_TUNE_FINE_OFFSET,
		0x40 << CODEC_ANA_MAIN2_TEST_TUNE_FINE_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_191, 0x3f << CODEC_ANA_MAIN2_TEST_TUNE_PVT_OFFSET,
		0x20 << CODEC_ANA_MAIN2_TEST_TUNE_PVT_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_193, 0x1 << CODEC_ANA_TEST_MODE_EN_OFFSET,
		0x0 << CODEC_ANA_TEST_MODE_EN_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_193, 0x1 << CODEC_ANA_TEST_MODE_EN_OFFSET,
		0x1 << CODEC_ANA_TEST_MODE_EN_OFFSET, true }, 0, 0 },
};

static irqreturn_t pll44k1_unlock_handler(int irq, void *data)
{
	struct hi6405_platform_data *platform_data = (struct hi6405_platform_data *)(data);
	struct snd_soc_codec *codec = platform_data->codec;
	unsigned char output_str[PLL_DATA_ALL_NUM] = {0};
	unsigned char buf[PLL_DATA_BUF_SIZE] = {0};
	unsigned int i;
	unsigned int j;

	if (codec != NULL) {
		AUDIO_LOGW("pll44k1 unlock irq received");
		snd_soc_write(codec, IRQ_REG4_REG,
			0x1 << PLL_44P1K_UNLOCK_F_IRQ_OFFSET);
#ifdef CONFIG_HUAWEI_DSM
		dsm_report(DSM_HI6402_PLL_UNLOCK, "64xx codec pll44k1_unlock\n");
#endif
		write_reg_seq_array(codec, pll44k1_unlock_regs,
			ARRAY_SIZE(pll44k1_unlock_regs));

		msleep(100);
		dsp_power_down(codec);
		for (i = 0; i < PLL_DATA_ALL_NUM; i = i + PLL_DATA_GROUP_NUM) {
			memset(output_str, 0, sizeof(output_str));
			memset(buf, 0, sizeof(buf));
			for (j = 0; j < PLL_DATA_GROUP_NUM; ++j) {
				snprintf(buf, sizeof(buf), "%9x",
					snd_soc_read(codec, PLL_FIFO));
				strncat(output_str, buf, strlen(buf));
			}
			AUDIO_LOGI("pll main2 PLL data:%s", output_str);
		}
		hi64xx_wtdog_send_event();
	}
	return IRQ_HANDLED;
}

static irqreturn_t pllmad_unlock_handler(int irq, void *data)
{
	irq_handler("64xx codec pllmad_unlock", IRQ_REG4_REG,
		PLL_MAD_UNLOCK_F_IRQ_OFFSET, DSM_HI6402_PLL_UNLOCK, data);
	return IRQ_HANDLED;
}

static const struct irq_config codec_irqs[] = {
	{ HI6405_IRQ_BUNK1_OCP, bunk1_ocp_handler, "bunk1_ocp", 1 },
	{ HI6405_IRQ_BUNK1_SCP, bunk1_scp_handler, "bunk1_scp", 1 },
	{ HI6405_IRQ_CP1_SHORT, cp1_short_handler, "cp1_short", 1 },
	{ HI6405_IRQ_CP2_SHORT, cp2_short_handler, "cp2_short", 1 },
	{ HI6405_IRQ_LDO_AVDD18_OCP, ldo_avdd18_ocp_handler, "ldo_avdd18_ocp", 1 },
	{ HI6405_IRQ_LDOP_OCP, ldop_ocp_handler, "ldop_ocp", 1 },
	{ HI6405_IRQ_LDON_OCP, ldon_ocp_handler, "ldon_ocp", 1 },
	{ HI6405_IRQ_PLL_UNLOCK, pll_unlock_handler, "pll_unlock", 0 },
	{ HI6405_IRQ_PLL44K1_UNLOCK, pll44k1_unlock_handler, "pll44k1_unlock", 0 },
	{ HI6405_IRQ_PLLMAD_UNLOCK, pllmad_unlock_handler, "pllmad_unlock", 0 },
};

static void codec_free_irq(struct hi6405_platform_data *platform_data)
{
	size_t len = ARRAY_SIZE(codec_irqs);
	unsigned int i;

	for (i = 0; i < len; i++)
		hi64xx_irq_free_irq(platform_data->irqmgr,
			codec_irqs[i].type, platform_data);
}

static int codec_request_irq(struct hi6405_platform_data *platform_data)
{
	size_t len = ARRAY_SIZE(codec_irqs);
	int ret = 0;
	unsigned int i, j;

	for (i = 0; i < len; i++) {
		ret = hi64xx_irq_request_irq(platform_data->irqmgr,
			codec_irqs[i].type, codec_irqs[i].handler,
			codec_irqs[i].name, platform_data);
		if (ret != 0) {
			AUDIO_LOGE("request irq failed, irq type is %d, irq name is %s",
				codec_irqs[i].type, codec_irqs[i].name);
			for (j = 0; j < i; j++) {
				hi64xx_irq_free_irq(platform_data->irqmgr,
					codec_irqs[i - 1 - j].type,
					platform_data);
			}
			break;
		}

		if (!codec_irqs[i].enabled)
			hi64xx_irq_disable_irq(platform_data->irqmgr,
				codec_irqs[i].type);
	}

	return ret;
}

#ifdef CONFIG_HISI_DIEID
int hi6405_codec_get_dieid(char *dieid, unsigned int len)
{
	unsigned int dieid_value;
	unsigned int reg_value;
	unsigned int length;
	char dieid_buf[CODEC_DIEID_BUF] = {0};
	char buf[CODEC_DIEID_TEM_BUF] = {0};
	int ret;

	if (dieid == NULL) {
		AUDIO_LOGE("dieid is NULL");
		return -EINVAL;
	}

	if (hi6405_codec == NULL) {
		AUDIO_LOGW("codec is NULL");
		return -EINVAL;
	}

	ret = snprintf(dieid_buf, sizeof(dieid_buf), "%s%s%s",
		"\r\n", "CODEC", ":0x");/* [false alarm]:snprintf is safe */
	if (ret < 0) {
		AUDIO_LOGE("snprintf main codec dieid head fail");
		return ret;
	}

	/* enable efuse */
	hi64xx_update_bits(hi6405_codec, DIE_ID_CFG1_REG,
			0x1 << EFUSE_MODE_SEL_OFFSET,
			0x1 << EFUSE_MODE_SEL_OFFSET);
	hi64xx_update_bits(hi6405_codec, CFG_CLK_CTRL_REG,
			0x1 << EFUSE_CLK_EN_OFFSET, 0x1 << EFUSE_CLK_EN_OFFSET);
	hi64xx_update_bits(hi6405_codec, DIE_ID_CFG1_REG,
			0x1 << EFUSE_READ_EN_OFFSET, 0x1 << EFUSE_READ_EN_OFFSET);
	usleep_range(5000, 5500);

	for (reg_value = DIE_ID_OUT_DATA0_REG; reg_value <= DIE_ID_OUT_DATA15_REG; reg_value++) {
		dieid_value = snd_soc_read(hi6405_codec, reg_value);
		ret = snprintf(buf, sizeof(buf), "%02x", dieid_value); /* [false alarm]:snprintf is safe */
		if (ret < 0) {
			AUDIO_LOGE("snprintf codec dieid fail");
			return ret;
		}
		strncat(dieid_buf, buf, strlen(buf));
	}
	strncat(dieid_buf, "\r\n", strlen("\r\n"));

	length = strlen(dieid_buf);
	if (len > length) {
		strncpy(dieid, dieid_buf, length);
		dieid[length] = '\0';
	} else {
		AUDIO_LOGE("dieid buf length = %u is not enough", length);
		return -ENOMEM;
	}

	return 0;
}
#endif

static int hi6405_platform_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct hi6405_platform_data *platform_data = devm_kzalloc(dev, sizeof(*platform_data), GFP_KERNEL);

	if (platform_data == NULL) {
		AUDIO_LOGE("malloc hi6405 platform data failed");
		return -ENOMEM;
	}

	ret = init_platform_data(pdev, platform_data);
	if (ret != 0) {
		AUDIO_LOGE("platform data initialization failed:0x%x", ret);
		goto free_platform_data;
	}

	ret = hi64xx_irq_init_irq(platform_data->irqmgr, &irqs);
	if (ret != 0) {
		AUDIO_LOGE("hi6405 irq init failed:0x%x", ret);
		goto irq_init_err_exit;
	}

	ret = codec_request_irq(platform_data);
	if (ret != 0) {
		AUDIO_LOGE("request irq failed:0x%x", ret);
		goto irq_request_err_exit;
	}

	ret = hi64xx_compat_init(platform_data->cdc_ctrl, platform_data->irqmgr);
	if (ret != 0) {
		AUDIO_LOGE("hi64xx compat init failed:0x%x", ret);
		goto compat_init_err_exit;
	}

	ret = snd_soc_register_codec(dev, &hi6405_codec_driver, hi6405_dai, ARRAY_SIZE(hi6405_dai));
	if (ret != 0) {
		AUDIO_LOGE("registe codec driver failed:0x%x", ret);
		goto codec_register_err_exit;
	}

	return ret;

codec_register_err_exit:
	hi64xx_compat_deinit();
compat_init_err_exit:
	codec_free_irq(platform_data);
irq_request_err_exit:
	hi64xx_irq_deinit_irq(platform_data->irqmgr);
irq_init_err_exit:
	deinit_platform_data(platform_data);
free_platform_data:
	if (platform_data != NULL)
		devm_kfree(dev, platform_data);

	AUDIO_LOGE("platform probe init failed");

	return ret;
}

static int hi6405_platform_remove(struct platform_device *pdev)
{
	struct hi6405_platform_data *platform_data = platform_get_drvdata(pdev);

#ifdef CONFIG_HAC_SUPPORT
	if (gpio_is_valid(platform_data->board_config.hac_gpio))
		gpio_free(platform_data->board_config.hac_gpio);
#endif

	snd_soc_unregister_codec(&pdev->dev);

	hi64xx_compat_deinit();

	codec_free_irq(platform_data);

	hi64xx_irq_deinit_irq(platform_data->irqmgr);

	deinit_platform_data(platform_data);

	return 0;
}

static void hi6405_platform_shutdown(struct platform_device *pdev)
{
	struct hi6405_platform_data *platform_data = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = platform_data->codec;

	if (codec != NULL)
		hi6405_headphone_pop_off(codec);
}

static struct platform_driver hi6405_platform_driver = {
	.probe = hi6405_platform_probe,
	.remove = hi6405_platform_remove,
	.shutdown = hi6405_platform_shutdown,
	.driver = {
		.owner = THIS_MODULE,
		.name = "hi6405-codec",
		.of_match_table = of_match_ptr(hi6405_platform_match),
	},
};

static int __init hi6405_platform_init(void)
{
	return platform_driver_register(&hi6405_platform_driver);
}
module_init(hi6405_platform_init);

static void __exit hi6405_platform_exit(void)
{
	platform_driver_unregister(&hi6405_platform_driver);
}
module_exit(hi6405_platform_exit);

MODULE_DESCRIPTION("ASoC hi6405 codec driver");
MODULE_LICENSE("GPL");

