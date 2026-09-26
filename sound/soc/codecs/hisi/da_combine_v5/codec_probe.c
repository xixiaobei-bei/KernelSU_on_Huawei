/*
 * codec_probe.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "codec_probe.h"

#include <linux/delay.h>
#include <linux/version.h>
#include <sound/soc.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#include "linux/hisi/audio_log.h"
#include "slimbus.h"
#include "slimbus_da_combine_v5.h"
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "debug.h"
#endif
#include "asoc_adapter.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "linux/hisi/da_combine/da_combine_resmgr.h"
#include "linux/hisi/da_combine/da_combine_vad.h"
#include "linux/hisi/da_combine/da_combine_mbhc.h"
#include "linux/hisi/da_combine/da_combine_v5_regs.h"
#include "linux/hisi/da_combine/da_combine_v5_type.h"
#include "linux/hisi/da_combine/da_combine_v5.h"
#include "da_combine_v5_dsp_config.h"
#include "kcontrol.h"
#include "resource_widget.h"
#include "path_widget.h"
#include "switch_widget.h"
#include "route.h"
#include "utils.h"
#include "single_drv_widget.h"
#include "single_pga_widget.h"
#include "single_switch_widget.h"
#include "pga_widget.h"
#include "single_switch_route.h"
#include "codec_pm.h"
#include "rdr_audio_codec.h"

#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
#include "high_res_cfg.h"
#include "huawei_platform/audio/high_resistance_hs_det.h"
#endif

#define ULTRA_ALL_CHECK_TIME       5
#define ULTRA_ALL_CHECK_ALL_TIME   400

#define ADC_PGA_GAIN_DEFAULT 0x78
#define DACLR_MIXER_PGA_GAIN_DEFAULT 0xFF
#define MBHC_VOLTAGE_COEFFICIENT_MIN 1600
#define MBHC_VOLTAGE_COEFFICIENT_DEFAULT 2700
#define MBHC_VOLTAGE_COEFFICIENT_MAX 2800
#define PGA_FADE_IN_CFG      0xf
#define PGA_FADE_OUT_CFG     0xa
#define S2_IL_PGA_FADE_CFG   0xc

#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
#define HS_DET_R_RATIO         2
#define HS_DET_I_RATIO         16
#define HS_DET_MICBIAS         1800 /* mv */
#define HS_DET_DEFAULT_VPEAK   200  /* mv */
#define HS_DET_INNER_RES       2200 /* ohm */
#define MAX_SARADC_VAL         4096
#define MAX_DAC_POST_PGA_GAIN  0x78
#endif

#ifdef CONFIG_HISI_DIEID
#define CODEC_DIEID_BUF 60
#define CODEC_DIEID_TEM_BUF 4
#endif

#define GET_SARADC_VAL_MAX_TIMES  3

static const char * const SND_CARD_DRV_VERSION = "1.0.0";

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
static struct hicodec_dump_reg_entry da_combine_v5_dump_table[] = {
	{ "PAGE IO", DBG_PAGE_IO_CODEC_START, DBG_PAGE_IO_CODEC_END, sizeof(unsigned int) },
	{ "PAGE CFG", DBG_PAGE_CFG_CODEC_START, DBG_PAGE_CFG_CODEC_END, sizeof(char) },
	{ "PAGE ANA", DBG_PAGE_ANA_CODEC_START, DBG_PAGE_ANA_CODEC_END, sizeof(char) },
	{ "PAGE DIG", DBG_PAGE_DIG_CODEC_START, DBG_PAGE_DIG_CODEC_END, sizeof(char) },
};

static struct hicodec_dump_reg_info dump_info = {
	.entry = da_combine_v5_dump_table,
	.count = sizeof(da_combine_v5_dump_table) / sizeof(struct hicodec_dump_reg_entry),
};
#endif

static struct snd_soc_component *da_combine_v5_codec;

struct snd_soc_component *da_combine_v5_get_codec(void)
{
	return da_combine_v5_codec;
}

static unsigned int get_saradc_value(struct snd_soc_component *codec)
{
	int retry = GET_SARADC_VAL_MAX_TIMES;
	unsigned int sar_value = 0;

	/* sar rst and work */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_078,
		BIT(RST_SAR_BIT), BIT(RST_SAR_BIT));
	udelay(50);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_078, BIT(RST_SAR_BIT), 0);
	udelay(50);
	/* saradc on */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_012, BIT(MBHD_SAR_PD_BIT), 0);
	/* start saradc */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_012,
		BIT(SARADC_START_BIT), BIT(SARADC_START_BIT));

	while (retry--) {
		usleep_range(1000, 1100);
		if (da_combine_check_saradc_ready_detection(codec)) {
			sar_value = snd_soc_component_read32(codec, CODEC_ANA_ROREG_000);
			sar_value = snd_soc_component_read32(codec,
				CODEC_ANA_ROREG_001) + (sar_value << 0x8);
			AUDIO_LOGI("saradc value is %#x", sar_value);

			break;
		}
	}
	if (retry < 0)
		AUDIO_LOGE("get saradc err");

	/* end saradc */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_012, BIT(SARADC_START_BIT), 0);
	/* saradc pd */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_012,
		BIT(MBHD_SAR_PD_BIT), BIT(MBHD_SAR_PD_BIT));

	return sar_value;
}

static unsigned int get_voltage_value(struct snd_soc_component *codec,
	unsigned int voltage_coefficient)
{
	unsigned int sar_value;
	unsigned int voltage_value;

	sar_value = get_saradc_value(codec);
	voltage_value = sar_value * voltage_coefficient / 0xFFF;

	return voltage_value;
}

static void hs_mbhc_on(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	if (platform_data == NULL) {
		AUDIO_LOGE("get platform data failed");
		return;
	}

	da_combine_irq_mask_btn_irqs(platform_data->mbhc);

	/* sar clk use clk32_sys */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_077,
		MASK_ON_BIT(CLK_SAR_SEL_REG_LEN, CLK_SAR_SEL_BIT), BIT(CLK_SAR_SEL_BIT));
	/* saradc tsmp set 4 * Tclk */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_077,
		MASK_ON_BIT(SAR_TSMP_CFG_REG_LEN, SAR_TSMP_CFG_BIT),
		MASK_ON_BIT(SAR_TSMP_CFG_REG_LEN, SAR_TSMP_CFG_BIT));
	/* cmp fast mode en */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_077,
		MASK_ON_BIT(SAR_INPUT_SEL_REG_LEN, SAR_INPUT_SEL_BIT), 0);
	msleep(30);

	da_combine_irq_unmask_btn_irqs(platform_data->mbhc);

	msleep(120);
}

static void hs_mbhc_off(struct snd_soc_component *codec)
{
	/* eco off */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_080, BIT(MBHD_ECO_EN_BIT), 0);
	AUDIO_LOGI("eco disable");
}

static void hs_enable_hsdet(struct snd_soc_component *codec,
	struct da_combine_mbhc_config mbhc_config)
{
	unsigned int voltage_coefficent;

	if (mbhc_config.hs_cfg[HS_COEFFICIENT] < MBHC_VOLTAGE_COEFFICIENT_MIN ||
		mbhc_config.hs_cfg[HS_COEFFICIENT] > MBHC_VOLTAGE_COEFFICIENT_MAX) {
		/* default set coefficent 2700mv */
		voltage_coefficent = (MBHC_VOLTAGE_COEFFICIENT_DEFAULT -
			MBHC_VOLTAGE_COEFFICIENT_MIN) / 100;
	} else {
		voltage_coefficent = (mbhc_config.hs_cfg[HS_COEFFICIENT] -
			MBHC_VOLTAGE_COEFFICIENT_MIN) / 100;
	}

	da_combine_update_bits(codec, CODEC_ANA_RWREG_073,
		MASK_ON_BIT(HSMICB_ADJ_REG_LEN, HSMICB_ADJ),
		voltage_coefficent << HSMICB_ADJ);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_012, BIT(MBHD_PD_MBHD_VTN), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_080, BIT(MBHD_HSD_EN_BIT),
		BIT(MBHD_HSD_EN_BIT));
}

#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
static void hs_res_det_enable(struct snd_soc_component *codec, bool enable)
{
	if (enable) {
		write_reg_seq_array(codec, enable_res_det_table,
			ARRAY_SIZE(enable_res_det_table));
		hs_mbhc_on(codec);
		AUDIO_LOGI("headphone resdet enable");
	} else {
		hs_mbhc_off(codec);
		write_reg_seq_array(codec, disable_res_det_table,
			ARRAY_SIZE(disable_res_det_table));
		AUDIO_LOGI("headphone resdet disable");
	}
}

static unsigned int get_resvalue_calculate(struct snd_soc_component *codec,
	unsigned int fb_val)
{
	unsigned int res_value;
	unsigned int saradc_value;
	unsigned int min_saradc_value = MAX_SARADC_VAL;
	unsigned int i;
	unsigned int r_mir; /* ohm */
	unsigned int vpeak_det; /* mv */

	da_combine_v5_headphone_pop_on(codec);
	for (i = 0; i < MAX_DAC_POST_PGA_GAIN + 1; i += 2) {
		snd_soc_component_write(codec, DACL_POST_PGA_CTRL1_REG, i);
		udelay(100);
	}
	write_reg_seq_array(codec, hp_impdet_vpout_table,
		ARRAY_SIZE(hp_impdet_vpout_table));
	/* wait for stable saradc_value */
	msleep(50);
	for (i = 0; i < 5; i++) {
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
	return res_value;
}

static unsigned int get_resvalue(struct snd_soc_component *codec)
{
	unsigned int res_value;
	unsigned int i;
	unsigned int fb_val; /* feedback res reg val */
	unsigned int vpeak_val; /* vpeak reg val */

	IN_FUNCTION;
	hs_res_det_enable(codec, true);
	write_reg_seq_array(codec, enable_get_res_table,
		ARRAY_SIZE(enable_get_res_table));

	fb_val = get_high_res_data(HIGH_RES_GET_FB_VAL);
	vpeak_val = get_high_res_data(HIGH_RES_GET_OUTPUT_AMP);
	AUDIO_LOGI("fb_val = 0x%x, vpeak_val = 0x%x", fb_val, vpeak_val);
	/* hs det res cfg */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_065,
		0x3 << CODEC_ANA_HP_IMPDET_RES_CFG_OFFSET,
		fb_val << CODEC_ANA_HP_IMPDET_RES_CFG_OFFSET);
	snd_soc_component_write(codec, DACL_PRE_PGA_CTRL1_REG, vpeak_val);

	res_value = get_resvalue_calculate(codec, fb_val);

	for (i = 0; i < MAX_DAC_POST_PGA_GAIN + 1; i += 2) {
		snd_soc_component_write(codec, DACL_POST_PGA_CTRL1_REG, MAX_DAC_POST_PGA_GAIN - i);
		udelay(100);
	}
	write_reg_seq_array(codec, disable_get_res_table,
		ARRAY_SIZE(disable_get_res_table));
	hs_res_det_enable(codec, false);
	OUT_FUNCTION;
	return res_value;
}

void imp_path_enable(struct snd_soc_component *codec, bool enable)
{
	if (enable) {
		write_reg_seq_array(codec, enable_path_table,
			ARRAY_SIZE(enable_path_table));
		da_combine_v5_set_classH_config(codec, HP_POWER_STATE & (~HP_CLASSH_STATE));
		AUDIO_LOGI("imp path enable");
	} else {
		da_combine_v5_headphone_pop_off(codec);
		da_combine_v5_set_classH_config(codec, HP_POWER_STATE | HP_CLASSH_STATE);
		write_reg_seq_array(codec, disable_path_table,
			ARRAY_SIZE(disable_path_table));
		AUDIO_LOGI("himp path disable");
	}
}
#endif /* CONFIG_HIGH_RESISTANCE_HS_DET */

static void hs_path_enable(struct snd_soc_component *codec)
{
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
	struct da_combine_v5_platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;
	da_combine_resmgr_request_pll(priv->resmgr, PLL_HIGH);
	request_cp_dp_clk(codec);
	imp_path_enable(codec, true);
	OUT_FUNCTION;
#endif
}

static void hs_res_detect(struct snd_soc_component *codec)
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

static void hs_path_disable(struct snd_soc_component *codec)
{
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
	struct da_combine_v5_platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;
	if (priv->hsl_power_on == false || priv->hsr_power_on == false) {
		imp_path_enable(codec, false);
		AUDIO_LOGI("headset path is open, no need to close");
	}
	release_cp_dp_clk(codec);
	da_combine_resmgr_release_pll(priv->resmgr, PLL_HIGH);
	OUT_FUNCTION;
#endif
}

static void hs_res_calib(struct snd_soc_component *codec)
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
		da_combine_v5_headphone_pop_on(codec);
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

static struct da_combine_hs_cfg hs_cfg = {
	.mbhc_reg = &hs_mbhc_reg,
	.mbhc_func = &hs_mbhc_func,
	.res_detect_func = &hs_res_detect_func_null,
};

static void efuse_init(struct snd_soc_component *codec)
{
	unsigned int inf_ate_ctrl;
	unsigned int inf_trim_ctrl;
	unsigned int bgr_ate;
	unsigned int die_id0;
	unsigned int die_id1;

	/* enable efuse */
	da_combine_update_bits(codec, DIE_ID_CFG1_REG,
		BIT(EFUSE_MODE_SEL_OFFSET), BIT(EFUSE_MODE_SEL_OFFSET));
	da_combine_update_bits(codec, CFG_CLK_CTRL_REG,
		BIT(EFUSE_CLK_EN_OFFSET), BIT(EFUSE_CLK_EN_OFFSET));
	da_combine_update_bits(codec, DIE_ID_CFG1_REG,
		BIT(EFUSE_READ_EN_OFFSET), BIT(EFUSE_READ_EN_OFFSET));

	usleep_range(5000, 5500);

	/* default para set */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
		BIT(CODEC_OTPREG_SEL_FIR_OFFSET), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_053,
		BIT(CODEC_OTPREG_SEL_INF_OFFSET), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_133,
		BIT(CODEC_OTPREG_SEL_BGR_OFFSET), 0);

	die_id0 = snd_soc_component_read32(codec, DIE_ID_OUT_DATA0_REG);
	die_id1 = snd_soc_component_read32(codec, DIE_ID_OUT_DATA1_REG);
	bgr_ate = snd_soc_component_read32(codec, DIE_ID_OUT_DATA2_REG);

	inf_ate_ctrl = die_id0 & 0xf;
	inf_trim_ctrl = ((die_id0 & 0xf0) >> 0x4) | ((die_id1 & 0x7) << 0x4);

	AUDIO_LOGI("efuse inf ate: 0x%x, inf trim: 0x%x, bgr ate0x%x",
			inf_ate_ctrl, inf_trim_ctrl, bgr_ate);
}

static const struct reg_seq_config ioshare_slimbus_init_regs[] = {
	{ { IOS_MF_CTRL1_REG, 0, 0x1, false }, 0, 0 },
	{ { IOS_MF_CTRL3_REG, 0, 0x2, false }, 0, 0 },
	{ { IOS_MF_CTRL4_REG, 0, 0x2, false }, 0, 0 },
	{ { IOS_IOM_CTRL7_REG, 0, 0x114, false }, 0, 0 },
	{ { IOS_IOM_CTRL8_REG, 0, 0x115, false }, 0, 0 },

	/* enable da_combine_v5 slim framer */
	{ { SLIM_CTRL1_REG, BIT(SLIM_CLK_EN_OFFSET),
		BIT(SLIM_CLK_EN_OFFSET), true }, 0, 0 },
	{ { SLIM_CTRL1_REG, BIT(SLIM_SWIRE_DIV_EN_OFFSET),
		BIT(SLIM_SWIRE_DIV_EN_OFFSET), true }, 0, 0 },
};

static const struct reg_seq_config ioshare_init_regs[] = {
	/* ssi ioshare config */
	{ { IOS_IOM_CTRL5_REG, 0, 0x10D, false }, 0, 0 },
	{ { IOS_MF_CTRL1_REG, 0, 0x1, false }, 0, 0 },

	/* slimbus ioshare config */
	{ { IOS_MF_CTRL3_REG, 0, 0x2, false }, 0, 0 },
	{ { IOS_MF_CTRL4_REG, 0, 0x2, false }, 0, 0 },

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

static void dmic_ioshare_init(struct snd_soc_component *codec,
	struct da_combine_v5_platform_data *platform_data)
{
	if (!platform_data->board_config.dmic_enable)
		return;

	snd_soc_component_write(codec, IOS_MF_CTRL15_REG, 0x02);
	snd_soc_component_write(codec, IOS_MF_CTRL16_REG, 0x02);
	snd_soc_component_write(codec, IOS_MF_CTRL18_REG, 0x02);
	snd_soc_component_write(codec, IOS_MF_CTRL19_REG, 0x02);
}

static void ioshare_init(struct snd_soc_component *codec,
	struct da_combine_v5_platform_data *platform_data)
{
	if (platform_data->cdc_ctrl->bus_sel == BUSTYPE_SELECT_SLIMBUS)
		write_reg_seq_array(codec, ioshare_slimbus_init_regs,
			ARRAY_SIZE(ioshare_slimbus_init_regs));

	write_reg_seq_array(codec, ioshare_init_regs,
		ARRAY_SIZE(ioshare_init_regs));

	dmic_ioshare_init(codec, platform_data);
}

static void slimbus_init(struct snd_soc_component *codec,
	struct da_combine_v5_platform_data *platform_data)
{
	/* slim&ssi mclk enable */
	da_combine_update_bits(codec, CFG_CLK_CTRL_REG,
		BIT(INTF_SSI_CLK_EN_OFFSET), BIT(INTF_SSI_CLK_EN_OFFSET));
	da_combine_update_bits(codec, CFG_CLK_CTRL_REG,
		BIT(INTF_SLIM_CLK_EN_OFFSET), BIT(INTF_SLIM_CLK_EN_OFFSET));

	snd_soc_component_write(codec, SLIM_CTRL3_REG, 0xBF);

	/* slimbus clk schmitt config */
	da_combine_update_bits(codec, IOS_IOM_CTRL8_REG, BIT(ST_OFFSET), BIT(ST_OFFSET));
	/* slimbus pin disable pd */
	da_combine_update_bits(codec, IOS_IOM_CTRL7_REG, BIT(PD_OFFSET), 0);
	da_combine_update_bits(codec, IOS_IOM_CTRL8_REG, BIT(PD_OFFSET), 0);

	/* slimbus drv codec side */
	da_combine_update_bits(codec, IOS_IOM_CTRL7_REG, MASK_ON_BIT(DS_LEN, DS_OFFSET),
		platform_data->cdc_ctrl->slimbusdata_cdc_drv << DS_OFFSET);

	da_combine_update_bits(codec, IOS_IOM_CTRL8_REG, MASK_ON_BIT(DS_LEN, DS_OFFSET),
		platform_data->cdc_ctrl->slimbusclk_cdc_drv << DS_OFFSET);

	/* slimbus frame config */
	snd_soc_component_write(codec, SLIM_CTRL0_REG, 0x6);

	/* slimbus up1&2 port fs config */
	snd_soc_component_write(codec, SC_FS_SLIM_CTRL_3_REG, 0x44);
	/* slimbus up3&4 port fs config */
	snd_soc_component_write(codec, SC_FS_SLIM_CTRL_4_REG, 0x44);
	/* slimbus up7&8 port fs config */
	snd_soc_component_write(codec, SC_FS_SLIM_CTRL_6_REG, 0x44);
	/* slimbus up9&10 port fs config */
	snd_soc_component_write(codec, SC_FS_SLIM_CTRL_7_REG, 0x44);

	/* slimbus dn1&dn2 port fs config */
	snd_soc_component_write(codec, SC_FS_SLIM_CTRL_0_REG, 0x44);
	/* slimbus dn5&dn6 port fs config */
	snd_soc_component_write(codec, SC_FS_SLIM_CTRL_2_REG, 0x44);
}

static void set_mad_param(struct snd_soc_component *codec,
	struct da_combine_v5_board_cfg *board_cfg)
{
	/* auto active time */
	snd_soc_component_write(codec, MAD_AUTO_ACT_TIME_REG, 0x0);

	/* pll time */
	snd_soc_component_write(codec, MAD_PLL_TIME_L_REG, 0x1);

	/* adc time */
	snd_soc_component_write(codec, MAD_ADC_TIME_H_REG, 0x0);
	snd_soc_component_write(codec, MAD_ADC_TIME_L_REG, 0x3);

	/* mad_ana_time */
	snd_soc_component_write(codec, MAD_ANA_TIME_H_REG, 0x0);
	snd_soc_component_write(codec, MAD_ANA_TIME_L_REG, 0x5);

	/* omt */
	snd_soc_component_write(codec, MAD_OMIT_SAMP_REG, 0x20);

	/* mad_vad_time */
	snd_soc_component_write(codec, MAD_VAD_TIME_H_REG, 0x0);
	snd_soc_component_write(codec, MAD_VAD_TIME_L_REG, 0xa0);

	/* mad_sleep_time */
	snd_soc_component_write(codec, MAD_SLEEP_TIME_L_REG, 0x0);

	/* mad_buffer_fifo_thre */
	if (board_cfg->wakeup_audio_algo_support)
		snd_soc_component_write(codec, MAD_BUFFER_CTRL0_REG, 0x3f);
	else
		snd_soc_component_write(codec, MAD_BUFFER_CTRL0_REG, 0x7f);
	da_combine_update_bits(codec, MAD_BUFFER_CTRL1_REG, 0x1f, 0x1f);

	/* mad_cnt_thre,vad delay cnt */
	snd_soc_component_write(codec, MAD_CNT_THRE_REG, 0x2);

	/* mad_snr_thre */
	snd_soc_component_write(codec, MAD_SNR_THRE_SUM_REG, 0x32);
	snd_soc_component_write(codec, MAD_SNR_THRE_REG, 0x20);

	/* mad_min_chan_eng */
	snd_soc_component_write(codec, MAD_MIN_CHAN_ENG_REG, 0x14);

	/* mad_ine */
	snd_soc_component_write(codec, MAD_INE_REG, 0x14);
	/* mad_band_thre */
	snd_soc_component_write(codec, MAD_BAND_THRE_REG, 0x8);
	/* mad_scale */
	snd_soc_component_write(codec, MAD_SCALE_REG, 0x3);

	/* mad_vad_num */
	snd_soc_component_write(codec, MAD_VAD_NUM_REG, 0x1);
	/* mad_alpha_en1 */
	snd_soc_component_write(codec, MAD_ALPHA_EN1_REG, 0xc);

	/* mad_vad_ao ->en, mad_irq_en->en, mad_en->en, mad_wind_sel */
	snd_soc_component_write(codec, MAD_CTRL_REG, 0x63);

	/* mad capless config */
	snd_soc_component_write(codec, ANA_MAD_CAPLESS_MIXER, 0x0);
	snd_soc_component_write(codec, ANA_MAD_PGA_CAPLESS_MIXER, 0x0);
}

static void set_dsp_if_bypass(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL22_REG,
		BIT(S3_O_DSP_BYPASS_OFFSET) | BIT(S2_O_DSP_BYPASS_OFFSET) |
		BIT(S1_O_DSP_BYPASS_OFFSET),
		BIT(S3_O_DSP_BYPASS_OFFSET) | BIT(S2_O_DSP_BYPASS_OFFSET) |
		BIT(S1_O_DSP_BYPASS_OFFSET));
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL8_REG,
		BIT(S4_I_DSP_BYPASS_OFFSET) | BIT(S3_I_DSP_BYPASS_OFFSET) |
		BIT(S2_I_DSP_BYPASS_OFFSET) | BIT(S1_I_DSP_BYPASS_OFFSET),
		BIT(S4_I_DSP_BYPASS_OFFSET) | BIT(S3_I_DSP_BYPASS_OFFSET) |
		BIT(S2_I_DSP_BYPASS_OFFSET) | BIT(S1_I_DSP_BYPASS_OFFSET));
}

static const struct reg_seq_config pga_fade_regs[] = {
	{ { DACL_MIXER4_CTRL1_REG, BIT(DACL_MIXER4_FADE_EN_OFFSET),
		BIT(DACL_MIXER4_FADE_EN_OFFSET), true }, 0, 0 },
	{ { DACL_MIXER4_CTRL3_REG, MASK_ON_BIT(DACL_MIXER4_FADE_IN_LEN,
		DACL_MIXER4_FADE_IN_OFFSET),
		PGA_FADE_IN_CFG << DACL_MIXER4_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACL_MIXER4_CTRL4_REG, MASK_ON_BIT(DACL_MIXER4_FADE_OUT_LEN,
		DACL_MIXER4_FADE_OUT_OFFSET),
		PGA_FADE_OUT_CFG << DACL_MIXER4_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACR_MIXER4_CTRL1_REG, BIT(DACR_MIXER4_FADE_EN_OFFSET),
		BIT(DACR_MIXER4_FADE_EN_OFFSET), true }, 0, 0 },
	{ { DACR_MIXER4_CTRL3_REG, MASK_ON_BIT(DACR_MIXER4_FADE_IN_LEN,
		DACR_MIXER4_FADE_IN_OFFSET),
		PGA_FADE_IN_CFG << DACR_MIXER4_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACR_MIXER4_CTRL4_REG, MASK_ON_BIT(DACR_MIXER4_FADE_OUT_LEN,
		DACR_MIXER4_FADE_OUT_OFFSET),
		PGA_FADE_OUT_CFG << DACR_MIXER4_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACL_PRE_MIXER2_CTRL1_REG, BIT(DACL_PRE_MIXER2_FADE_EN_OFFSET),
		BIT(DACL_PRE_MIXER2_FADE_EN_OFFSET), true }, 0, 0 },
	{ { DACL_PRE_MIXER2_CTRL2_REG, MASK_ON_BIT(DACL_PRE_MIXER2_FADE_IN_LEN,
		DACL_PRE_MIXER2_FADE_IN_OFFSET),
		PGA_FADE_IN_CFG << DACL_PRE_MIXER2_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACL_PRE_MIXER2_CTRL3_REG, MASK_ON_BIT(DACL_PRE_MIXER2_FADE_OUT_LEN,
		DACL_PRE_MIXER2_FADE_OUT_OFFSET),
		PGA_FADE_OUT_CFG << DACL_PRE_MIXER2_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACR_PRE_MIXER2_CTRL1_REG, BIT(DACR_PRE_MIXER2_FADE_EN_OFFSET),
		BIT(DACR_PRE_MIXER2_FADE_EN_OFFSET), true }, 0, 0 },
	{ { DACR_PRE_MIXER2_CTRL2_REG, MASK_ON_BIT(DACR_PRE_MIXER2_FADE_IN_LEN,
		DACR_PRE_MIXER2_FADE_IN_OFFSET),
		PGA_FADE_IN_CFG << DACR_PRE_MIXER2_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACR_PRE_MIXER2_CTRL3_REG, MASK_ON_BIT(DACR_PRE_MIXER2_FADE_OUT_LEN,
		DACR_PRE_MIXER2_FADE_OUT_OFFSET),
		PGA_FADE_OUT_CFG << DACR_PRE_MIXER2_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACL_POST_MIXER2_CTRL1_REG, BIT(DACL_POST_MIXER2_FADE_EN_OFFSET),
		BIT(DACL_POST_MIXER2_FADE_EN_OFFSET), true }, 0, 0 },
	{ { DACL_POST_MIXER2_CTRL2_REG, MASK_ON_BIT(DACL_POST_MIXER2_FADE_IN_LEN,
		DACL_POST_MIXER2_FADE_IN_OFFSET),
		PGA_FADE_IN_CFG << DACL_POST_MIXER2_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACL_POST_MIXER2_CTRL3_REG, MASK_ON_BIT(DACL_POST_MIXER2_FADE_OUT_LEN,
		DACL_POST_MIXER2_FADE_OUT_OFFSET),
		PGA_FADE_OUT_CFG << DACL_POST_MIXER2_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACR_POST_MIXER2_CTRL1_REG, BIT(DACR_POST_MIXER2_FADE_EN_OFFSET),
		BIT(DACR_POST_MIXER2_FADE_EN_OFFSET), true }, 0, 0 },
	{ { DACR_POST_MIXER2_CTRL2_REG, MASK_ON_BIT(DACR_POST_MIXER2_FADE_IN_LEN,
		DACR_POST_MIXER2_FADE_IN_OFFSET),
		PGA_FADE_IN_CFG << DACR_POST_MIXER2_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACR_POST_MIXER2_CTRL3_REG, MASK_ON_BIT(DACR_POST_MIXER2_FADE_OUT_LEN,
		DACR_POST_MIXER2_FADE_OUT_OFFSET),
		PGA_FADE_OUT_CFG << DACR_POST_MIXER2_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { DACSL_MIXER4_CTRL1_REG, BIT(DACSL_MIXER4_FADE_EN_OFFSET),
		BIT(DACSL_MIXER4_FADE_EN_OFFSET), true }, 0, 0 },
	{ { DACSL_MIXER4_CTRL3_REG, MASK_ON_BIT(DACSL_MIXER4_FADE_IN_LEN,
		DACSL_MIXER4_FADE_IN_OFFSET),
		PGA_FADE_IN_CFG << DACSL_MIXER4_FADE_IN_OFFSET, true }, 0, 0 },
	{ { DACSL_MIXER4_CTRL4_REG, MASK_ON_BIT(DACSL_MIXER4_FADE_OUT_LEN,
		DACSL_MIXER4_FADE_OUT_OFFSET),
		PGA_FADE_OUT_CFG << DACSL_MIXER4_FADE_OUT_OFFSET, true }, 0, 0 },
	{ { S2_IL_PGA_CTRL0_REG, BIT(S2_IL_PGA_FADE_EN_OFFSET), 0, true }, 0, 0 },
	{ { S2_IL_PGA_CTRL2_REG, MASK_ON_BIT(S2_IL_PGA_FADE_IN_LEN,
		S2_IL_PGA_FADE_IN_OFFSET),
		S2_IL_PGA_FADE_CFG << S2_IL_PGA_FADE_IN_OFFSET, true }, 0, 0 },
	{ { S2_IL_PGA_CTRL3_REG, MASK_ON_BIT(S2_IL_PGA_FADE_OUT_LEN,
		S2_IL_PGA_FADE_OUT_OFFSET),
		S2_IL_PGA_FADE_CFG << S2_IL_PGA_FADE_OUT_OFFSET, true }, 0, 0 },
};

static void pga_fade_init(struct snd_soc_component *codec)
{
	write_reg_seq_array(codec, pga_fade_regs, ARRAY_SIZE(pga_fade_regs));
}

static void mic_pga_gain_init(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, ANA_HSMIC_MUX_AND_PGA, 0xf, 0x4);
	da_combine_update_bits(codec, ANA_AUXMIC_MUX_AND_PGA, 0xf, 0x4);
	da_combine_update_bits(codec, ANA_MIC3_MUX_AND_PGA, 0xf, 0x4);
	da_combine_update_bits(codec, ANA_MIC4_MUX_AND_PGA, 0xf, 0x4);
	da_combine_update_bits(codec, ANA_MIC5_MUX_AND_PGA, 0xf, 0x4);
	da_combine_update_bits(codec, ANA_MAD_PGA, 0xf, 0x4);
}

static void adc_pga_gain_init(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, ADC0L_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
	snd_soc_component_write(codec, ADC0R_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
	snd_soc_component_write(codec, ADC1L_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
	snd_soc_component_write(codec, ADC1R_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
	snd_soc_component_write(codec, ADC2L_PGA_CTRL1_REG, ADC_PGA_GAIN_DEFAULT);
}

static void mixer_pga_gain_init(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DACL_MIXER4_CTRL2_REG,
		DACLR_MIXER_PGA_GAIN_DEFAULT);
	snd_soc_component_write(codec, DACR_MIXER4_CTRL2_REG,
		DACLR_MIXER_PGA_GAIN_DEFAULT);
	da_combine_update_bits(codec, DACL_PRE_MIXER2_CTRL1_REG, 0x1E, 0xff);
	da_combine_update_bits(codec, DACR_PRE_MIXER2_CTRL1_REG, 0x1E, 0xff);
	da_combine_update_bits(codec, DACL_POST_MIXER2_CTRL1_REG, 0x1E, 0xff);
	da_combine_update_bits(codec, DACR_POST_MIXER2_CTRL1_REG, 0x1E, 0xff);
	snd_soc_component_write(codec, DACSL_MIXER4_CTRL2_REG,
		DACLR_MIXER_PGA_GAIN_DEFAULT);
	snd_soc_component_write(codec, DACSR_MIXER4_CTRL2_REG,
		DACLR_MIXER_PGA_GAIN_DEFAULT);
}

static void dac_pga_gain_init(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DACL_PRE_PGA_CTRL1_REG, 0x6E);/* -5db */
	snd_soc_component_write(codec, DACR_PRE_PGA_CTRL1_REG, 0x6E);
	snd_soc_component_write(codec, DACL_POST_PGA_CTRL1_REG, 0x6E);
	snd_soc_component_write(codec, DACR_POST_PGA_CTRL1_REG, 0x6E);
}

static void adc_init(struct snd_soc_component *codec)
{
	/* adc source select */
	snd_soc_component_write(codec, SC_ADC_ANA_SEL_REG, 0x3f);
}

static void hsd_cfg_init(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, CODEC_ANA_RWREG_078,
		MASK_ON_BIT(HSD_VL_SEL_BIT_REG_LEN, HSD_VL_SEL_BIT),
		HSD_VTH_LO_CFG << HSD_VL_SEL_BIT);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_078,
		MASK_ON_BIT(HSD_VH_SEL_BIT_REG_LEN, HSD_VH_SEL_BIT),
		HSD_VTH_HI_CFG << HSD_VH_SEL_BIT);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_079,
		BIT(HSD_VTH_SEL_BIT), BIT(HSD_VTH_SEL_BIT));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_079,
		MASK_ON_BIT(HSD_VTH_MICL_CFG_LEN, HSD_VTH_MICL_CFG_OFFSET),
		HSD_VTH_MICL_CFG_800MV << HSD_VTH_MICL_CFG_OFFSET);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_079,
		MASK_ON_BIT(HSD_VTH_MICH_CFG_LEN, HSD_VTH_MICH_CFG_OFFSET),
		HSD_VTH_MICH_CFG_95 << HSD_VTH_MICH_CFG_OFFSET);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_080,
		MASK_ON_BIT(MBHD_VTH_ECO_CFG_LEN, MBHD_VTH_ECO_CFG_OFFSET),
		MBHD_VTH_ECO_CFG_125MV << MBHD_VTH_ECO_CFG_OFFSET);
}

static void class_h_init(struct snd_soc_component *codec,
	struct da_combine_v5_platform_data *platform_data)
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
	da_combine_v5_set_classH_config(codec, platform_data->rcv_hp_classH_state);
}

static const struct reg_seq_config key_gpio_init_regs[] = {
#ifdef CONFIG_VIRTUAL_BTN_SUPPORT
	{ { APB_CLK_CFG_REG, BIT(GPIO_PCLK_EN_OFFSET),
		BIT(GPIO_PCLK_EN_OFFSET), true }, 0, 0 },
	/* GPIO19---KEY_INT */
	{ { IOS_MF_CTRL19_REG, MASK_ON_BIT(IOS_MF_CTRL19_LEN, IOS_MF_CTRL19_OFFSET),
		IOS_MF_CTRL19_CFG << IOS_MF_CTRL19_OFFSET, true }, 0, 0 },
	{ { IOS_IOM_CTRL23_REG, MASK_ON_BIT(PU_CFG, PU_OFFSET), 0, true }, 0, 0 },
	{ { IOS_IOM_CTRL23_REG, BIT(ST_OFFSET), BIT(ST_OFFSET), true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2DIR_REG, BIT(GPIO2_19_OFFSET), 0, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2IS_REG, BIT(GPIO2_19_OFFSET), 0, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2IBE_REG, BIT(GPIO2_19_OFFSET), 0, true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2IEV_REG, BIT(GPIO2_19_OFFSET),
		BIT(GPIO2_19_OFFSET), true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2IE_REG, BIT(GPIO2_19_OFFSET), 0, true }, 0, 0 },
	/* GPIO18---PWM_SMT */
	{ { IOS_MF_CTRL18_REG, MASK_ON_BIT(IOS_MF_CTRL18_LEN, IOS_MF_CTRL18_OFFSET),
		IOS_MF_CTRL18_CFG << IOS_MF_CTRL18_OFFSET, true }, 0, 0 },
	{ { IOS_IOM_CTRL22_REG, MASK_ON_BIT(PU_CFG, PU_OFFSET), 0, true }, 0, 0 },
	{ { IOS_IOM_CTRL22_REG, MASK_ON_BIT(DS_LEN, DS_OFFSET),
		BIT(DS_OFFSET), true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO2DIR_REG, BIT(GPIO2_18_OFFSET),
		BIT(GPIO2_18_OFFSET), true }, 0, 0 },
	/* GPIO15---AP_AI */
	{ { IOS_MF_CTRL15_REG, MASK_ON_BIT(IOS_MF_CTRL15_LEN, IOS_MF_CTRL15_OFFSET),
		IOS_MF_CTRL15_CFG << IOS_MF_CTRL15_OFFSET, true }, 0, 0 },
	{ { IOS_IOM_CTRL19_REG, MASK_ON_BIT(PU_CFG, PU_OFFSET), 0, true }, 0, 0 },
	{ { IOS_IOM_CTRL19_REG, MASK_ON_BIT(DS_LEN, DS_OFFSET),
		BIT(DS_OFFSET), true }, 0, 0 },
	{ { CODEC_BASE_ADDR | GPIO1DIR_REG, BIT(GPIO2_15_OFFSET),
		BIT(GPIO2_15_OFFSET), true }, 0, 0 },
#endif
	{ { ANA_MICBIAS1, BIT(ANA_MICBIAS1_DSCHG_OFFSET), 0, true }, 0, 0 },
	{ { ANA_MICBIAS2, BIT(ANA_MICBIAS2_DSCHG_OFFSET), 0, true }, 0, 0 },
	{ { ANA_HSMICBIAS, BIT(ANA_HSMIC_DSCHG_OFFSET), 0, true }, 0, 0 },
};

static void key_gpio_init(struct snd_soc_component *codec)
{
	write_reg_seq_array(codec, key_gpio_init_regs,
		ARRAY_SIZE(key_gpio_init_regs));
}

static void mux_init(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
		MASK_ON_BIT(SPA_OL_SRC_DIN_SEL_LEN, SPA_OL_SRC_DIN_SEL_OFFSET) |
		MASK_ON_BIT(SPA_OR_SRC_DIN_SEL_LEN, SPA_OR_SRC_DIN_SEL_OFFSET),
		BIT(SPA_OL_SRC_DIN_SEL_OFFSET) | BIT(SPA_OR_SRC_DIN_SEL_OFFSET));

	/* set dacsl to 96k */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL38_REG,
		BIT(DACSL_MIXER4_DVLD_SEL_OFFSET), BIT(DACSL_MIXER4_DVLD_SEL_OFFSET));
}

static void hp_config(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	if (platform_data == NULL) {
		AUDIO_LOGE("hp config get platform data failed");
		return;
	}

	if (!platform_data->board_config.cap_1nf_enable)
		return;

	da_combine_update_bits(codec, CODEC_ANA_RWREG_056,
		BIT(CODEC_ANA_STB_1N_CT_OFFSET) | BIT(CODEC_ANA_STB_1N_CAP_OFFSET),
		BIT(CODEC_ANA_STB_1N_CT_OFFSET) | BIT(CODEC_ANA_STB_1N_CAP_OFFSET));
}

static int utils_init(struct da_combine_v5_platform_data *platform_data)
{
	struct utils_config cfg;

	cfg.da_combine_dump_reg = NULL;
	return da_combine_utils_init(platform_data->codec, platform_data->cdc_ctrl,
		&cfg, platform_data->resmgr, DA_COMBINE_CODEC_TYPE_V5);
}

static int slimbus_enumerate(struct da_combine_v5_platform_data *data)
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
	ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
		SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY, NULL);
	if (ret != 0)
		AUDIO_LOGE("slimbus_activate_track audio play failed");
	usleep_range(1000, 1100);
	ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
		SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY, NULL);
	if (ret != 0)
		AUDIO_LOGE("deactivate audio play failed");
	ret = da_combine_resmgr_release_pll(data->resmgr, PLL_HIGH);
	if (ret != 0)
		AUDIO_LOGE("release pll failed");
	return ret;
}

static bool need_reset_in_kernel(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	if (platform_data->cdc_ctrl == NULL) {
		AUDIO_LOGE("cdc ctrl is NULL");
		return false;
	}

	if (platform_data->cdc_ctrl->need_reset_in_kernel)
		return true;

	return false;
}

static void ioshare_irq_init(struct snd_soc_component *codec)
{
	/* codec irq ioshare config */
	snd_soc_component_write(codec, IOS_MF_CTRL2_REG, 0x1);
	/* dis int input */
	snd_soc_component_write(codec, IOS_IOM_CTRL6_REG, 0x4);
	/* clear all irq state */
	snd_soc_component_write(codec, IRQ_REG0_REG, 0xff);
	snd_soc_component_write(codec, IRQ_REG1_REG, 0xff);
	snd_soc_component_write(codec, IRQ_REG2_REG, 0xff);
	snd_soc_component_write(codec, IRQ_REG3_REG, 0xff);
	snd_soc_component_write(codec, IRQ_REG4_REG, 0xff);
	snd_soc_component_write(codec, IRQ_REG5_REG, 0xff);
	snd_soc_component_write(codec, IRQ_REG6_REG, 0xff);
	AUDIO_LOGI("irq ioshare init ok");
}

static void ioshare_ssi_slimbus_init(struct snd_soc_component *codec)
{
	/* ioshare config */
	snd_soc_component_write(codec, IOS_MF_CTRL1_REG, 0x1);
	snd_soc_component_write(codec, IOS_MF_CTRL3_REG, 0x2);
	snd_soc_component_write(codec, IOS_MF_CTRL4_REG, 0x2);
	snd_soc_component_write(codec, IOS_IOM_CTRL7_REG, 0x114);
	snd_soc_component_write(codec, IOS_IOM_CTRL8_REG, 0x115);
	/* enable da_combine_v5 slim framer */
	snd_soc_component_write(codec, SLIM_CTRL0_REG, 0x6);
	/* slim&ssi mclk enable */
	snd_soc_component_write(codec, SLIM_CTRL1_REG, 0x16);
	snd_soc_component_write(codec, SLIM_CTRL1_REG, 0x36);
	snd_soc_component_write(codec, CFG_CLK_CTRL_REG, 0x3);
	AUDIO_LOGI("slimbus ioshare init ok");
}

static int chip_init(struct snd_soc_component *codec)
{
	int ret = 0;
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;
	if (need_reset_in_kernel(codec)) {
		/* codec device enumerate irq coming after ioshare configure
		   call slimbus enable to make sure irq process correctly */
		ret = slimbus_get_rpm();
		if (ret < 0) {
			return ret;
		}
		ioshare_ssi_slimbus_init(codec);
		ioshare_irq_init(codec);
		ret = slimbus_enum_codec_device();
		if (ret < 0) {
			AUDIO_LOGE("slimbus enum device failed");
			slimbus_put_rpm();
			return ret;
		}
		slimbus_put_rpm();
	}
	efuse_init(codec);
	ioshare_init(codec, platform_data);
	da_combine_v5_supply_pll_init(codec);
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
	class_h_init(codec, platform_data);
	key_gpio_init(codec);
	mux_init(codec);
	hp_config(codec);

	OUT_FUNCTION;

	return ret;
}

static void generate_mbhc_cfg(struct da_combine_v5_platform_data *data,
	struct da_combine_mbhc_cfg *mbhc_cfg)
{
	mbhc_cfg->codec = data->codec;
	mbhc_cfg->irqmgr = data->irqmgr;
	mbhc_cfg->node = data->node;
	mbhc_cfg->resmgr = data->resmgr;
	mbhc_cfg->mbhc = &(data->mbhc);
}

static void pll_track_delay(struct work_struct *work)
{
	unsigned int i;
	unsigned int count = 0;
	struct da_combine_v5_platform_data *priv = snd_soc_component_get_drvdata(da_combine_v5_codec);
	bool pll_track_state = snd_soc_component_read32
		(da_combine_v5_codec, CODEC_ANA_RWREG_149) & BIT(MAIN1_TRK_EN_OFFSET);

	/*
	 * After 10ms delay, the PLL status is checked every 200us.
	 * If the PLL status is checked successfully for five consecutive times,
	 * the PLL status is tracked. wait 80ms here is enough.
	 */
	usleep_range(10000, 11000);
	for (i = 0; i < ULTRA_ALL_CHECK_TIME; i++) {
		if (count == ULTRA_ALL_CHECK_ALL_TIME || !pll_track_state)
			break;
		if (!(snd_soc_component_read32(da_combine_v5_codec, IRQ_ANA_2_REG) & (BIT(MAIN1_PLL_LOCK_TRK_OFFSET))))
			i = 0;
		usleep_range(190, 200);
		count++;
	}

	da_combine_irq_enable_irq(priv->irqmgr, IRQ_PLL_UNLOCK);
	AUDIO_LOGI("pll track check time is %d", count);
}

static int pll_track_init(struct da_combine_v5_platform_data *data)
{
	data->pll_track = create_singlethread_workqueue("pll_track");
	if (!data->pll_track) {
		AUDIO_LOGE("pll_track workqueue create failed\n");
		return -EFAULT;
	}

	INIT_WORK(&data->pll_track_work, pll_track_delay);

	return 0;
}

static void pll_track_deinit(struct da_combine_v5_platform_data *data)
{
	if (!data->pll_track)
		return;
	flush_workqueue(data->pll_track);
	destroy_workqueue(data->pll_track);
	data->pll_track = NULL;
}

static int codec_init(struct snd_soc_component *codec,
	struct da_combine_v5_platform_data *data)
{
	int ret;
	struct da_combine_mbhc_cfg mbhc;

	generate_mbhc_cfg(data, &mbhc);

	ret = da_combine_mbhc_init(&mbhc, &hs_cfg);
	if (ret != 0) {
		AUDIO_LOGE("hifi config fail: 0x%x", ret);
		goto mbhc_init_failed;
	}

	ret = da_combine_v5_dsp_config_init(codec, data);
	if (ret != 0) {
		AUDIO_LOGE("dsp init failed: 0x%x", ret);
		goto dsp_init_failed;
	}

	ret = utils_init(data);
	if (ret != 0) {
		AUDIO_LOGE("utils init failed: 0x%x", ret);
		goto utils_init_failed;
	}

	ret = da_combine_vad_init(codec, data->irqmgr);
	if (ret != 0) {
		AUDIO_LOGE("vad init failed: 0x%x", ret);
		goto vad_init_failed;
	}

	ret = slimbus_enumerate(data);
	if (ret != 0) {
		AUDIO_LOGE("slim enumerate failed: 0x%x", ret);
		goto slimbus_enumerate_failed;
	}

	if (data->board_config.pll_track_support) {
		ret = pll_track_init(data);
		if (ret != 0) {
			AUDIO_LOGE("pll track init failed: 0x%x", ret);
			pll_track_deinit(data);
			goto slimbus_enumerate_failed;
		}
	}

	return ret;

slimbus_enumerate_failed:
	da_combine_vad_deinit(data->node);
vad_init_failed:
	da_combine_utils_deinit();
utils_init_failed:
	da_combine_v5_dsp_config_deinit();
dsp_init_failed:
	da_combine_mbhc_deinit(data->mbhc);
mbhc_init_failed:
	return ret;
}

static int codec_add_driver_resource(struct snd_soc_component *codec)
{
	int ret = da_combine_v5_add_kcontrol(codec);

	if (ret != 0) {
		AUDIO_LOGE("add kcontrols failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_resource_widgets(codec, false);
	if (ret != 0) {
		AUDIO_LOGE("add resource widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_pga_widgets(codec, false);
	if (ret != 0) {
		AUDIO_LOGE("add pga widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_path_widgets(codec, false);
	if (ret != 0) {
		AUDIO_LOGE("add path widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_switch_widgets(codec, false);
	if (ret != 0) {
		AUDIO_LOGE("add switch widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_routes(codec);
	if (ret != 0) {
		AUDIO_LOGE("add route map failed, ret = %d", ret);
		goto exit;
	}

exit:
	return ret;
}

static int codec_add_driver_resource_for_single_control(struct snd_soc_component *codec)
{
	int ret = da_combine_v5_add_kcontrol(codec);

	if (ret != 0) {
		AUDIO_LOGE("add kcontrols failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_resource_widgets(codec, true);
	if (ret != 0) {
		AUDIO_LOGE("add resource widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_pga_widgets(codec, true);
	if (ret != 0) {
		AUDIO_LOGE("add pga widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_path_widgets(codec, true);
	if (ret != 0) {
		AUDIO_LOGE("add path widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_switch_widgets(codec, true);
	if (ret != 0) {
		AUDIO_LOGE("add switch widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_single_drv_widgets(codec);
	if (ret != 0) {
		AUDIO_LOGE("add single drv widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_single_pga_widgets(codec);
	if (ret != 0) {
		AUDIO_LOGE("add single pga widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_single_switch_widgets(codec);
	if (ret != 0) {
		AUDIO_LOGE("add switch widgets failed, ret = %d", ret);
		goto exit;
	}

	ret = da_combine_v5_add_single_switch_route(codec);
	if (ret != 0) {
		AUDIO_LOGE("add route map failed, ret = %d", ret);
		goto exit;
	}

exit:
	return ret;
}

static void codec_deinit(struct da_combine_v5_platform_data *data)
{
	pll_track_deinit(data);
	da_combine_vad_deinit(data->node);
	da_combine_utils_deinit();
	da_combine_v5_dsp_config_deinit();
	da_combine_mbhc_deinit(data->mbhc);
}

static void ir_calibration_operation(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_EN_CAL_MT_OFFSET), BIT(CODEC_EN_CAL_MT_OFFSET));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_053,
		MASK_ON_BIT(CODEC_INF_TRIM_CTRL_REG_LEN,
		CODEC_INF_TRIM_CTRL_REG_OFFSET), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_053,
		BIT(CODEC_OTPREG_SEL_INF_OFFSET), BIT(CODEC_OTPREG_SEL_INF_OFFSET));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_01,
		BIT(CODEC_PD_INFCAL_CLK_OFFSET), BIT(CODEC_PD_INFCAL_CLK_OFFSET));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_138,
		BIT(CODEC_RX_CHOP_BPS_OFFSET), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_CLK_CALIB_CFG_OFFSET) |
		MASK_ON_BIT(CODEC_INF_IBCT_CAL_LEN, CODEC_INF_IBCT_CAL_OFFSET), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_RST_CLK_CAL_OFFSET) | BIT(CODEC_RST_CAL_OFFSET),
		BIT(CODEC_RST_CLK_CAL_OFFSET) | BIT(CODEC_RST_CAL_OFFSET));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_RST_CLK_CAL_OFFSET), 0);
	usleep_range(1100, 1200);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_EN_CAL_MT_OFFSET) | BIT(CODEC_RST_CAL_OFFSET), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_EN_CAL_OFFSET), BIT(CODEC_EN_CAL_OFFSET));
	usleep_range(10000, 11000);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_EN_CAL_OFFSET), 0);
}

#ifdef CONFIG_HISI_DIEID
int da_combine_v5_codec_get_dieid(char *dieid, unsigned int len)
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

	if (da_combine_v5_codec == NULL) {
		AUDIO_LOGW("parameter is NULL");
		return -EINVAL;
	}

	ret = snprintf(dieid_buf, sizeof(dieid_buf), "%s%s%s",
		"\r\n", "CODEC", ":0x");/* [false alarm]:snprintf is safe */
	if (ret < 0) {
		AUDIO_LOGE("snprintf main dieid head fail");
		return ret;
	}

	/* enable efuse */
	da_combine_update_bits(da_combine_v5_codec, DIE_ID_CFG1_REG,
			BIT(EFUSE_MODE_SEL_OFFSET), BIT(EFUSE_MODE_SEL_OFFSET));
	da_combine_update_bits(da_combine_v5_codec, CFG_CLK_CTRL_REG,
			BIT(EFUSE_CLK_EN_OFFSET), BIT(EFUSE_CLK_EN_OFFSET));
	da_combine_update_bits(da_combine_v5_codec, DIE_ID_CFG1_REG,
			BIT(EFUSE_READ_EN_OFFSET), BIT(EFUSE_READ_EN_OFFSET));
	usleep_range(5000, 5500);

	for (reg_value = DIE_ID_OUT_DATA0_REG; reg_value <= DIE_ID_OUT_DATA15_REG; reg_value++) {
		dieid_value = snd_soc_component_read32(da_combine_v5_codec, reg_value);
		ret = snprintf(buf, sizeof(buf), "%02x", dieid_value); /* [false alarm]:snprintf is safe */
		if (ret < 0) {
			AUDIO_LOGE("snprintf dieid fail");
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

static void ir_calibration(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *priv = snd_soc_component_get_drvdata(codec);

	da_combine_resmgr_request_pll(priv->resmgr, PLL_HIGH);
	/* dp clk enable */
	request_dp_clk(codec, true);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
		BIT(CODEC_PD_INF_LRN_OFFSET), BIT(CODEC_PD_INF_LRN_OFFSET));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
		BIT(CODEC_PD_INF_EMS_OFFSET), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_051,
		MASK_ON_BIT(CODEC_FIR_OUT_CTRL_LEN, CODEC_FIR_OUT_CTRL_OFFSET) |
		MASK_ON_BIT(CODEC_FIR_ATE_CTRL_LEN, CODEC_FIR_ATE_CTRL_OFFSET),
		CODEC_FIR_OUT_X15 << CODEC_FIR_OUT_CTRL_OFFSET |
		CODEC_FIR_ATE_XF << CODEC_FIR_ATE_CTRL_OFFSET);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_LEAK_CTRL_OFFSET), BIT(CODEC_LEAK_CTRL_OFFSET));

	ir_calibration_operation(codec);

	da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
		BIT(CODEC_LEAK_CTRL_OFFSET), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_051,
		MASK_ON_BIT(CODEC_FIR_OUT_CTRL_LEN, CODEC_FIR_OUT_CTRL_OFFSET) |
		MASK_ON_BIT(CODEC_FIR_ATE_CTRL_LEN, CODEC_FIR_ATE_CTRL_OFFSET),
		CODEC_FIR_OUT_NON << CODEC_FIR_OUT_CTRL_OFFSET |
		CODEC_FIR_ATE_X1 << CODEC_FIR_ATE_CTRL_OFFSET);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
		BIT(CODEC_PD_INF_EMS_OFFSET), BIT(CODEC_PD_INF_EMS_OFFSET));
	/* dp clk disable */
	request_dp_clk(codec, false);
	da_combine_resmgr_release_pll(priv->resmgr, PLL_HIGH);
	AUDIO_LOGI("ir calibration end");
}

static void da_combine_print_codec_info(struct snd_info_entry *entry,
			     struct snd_info_buffer *buffer)
{
	snd_iprintf(buffer, "Name:da_combine_v5\n");
	snd_iprintf(buffer, "Vendor:HISILICON\n");
	snd_iprintf(buffer, "Model:da_combine_v5\n");
}

static void da_combine_codec_info_select(struct snd_soc_component *component)
{
	struct snd_info_entry *entry = NULL;
	int ret;

	ret = snd_card_proc_new(component->card->snd_card, "codec#0", &entry);
	if (ret < 0) {
		AUDIO_LOGE("select info failed");
		return;
	}
	snd_info_set_text_ops(entry, NULL, da_combine_print_codec_info);

	return;
}

static bool get_ap_reset_cfg(void)
{
	struct da_combine_v5_platform_data *platform_data = NULL;

	if (da_combine_v5_codec == NULL)
		return false;

	platform_data = snd_soc_component_get_drvdata(da_combine_v5_codec);
	if (platform_data->board_config.ap_reset_disable)
		return true;

	return false;
}

int da_combine_v5_codec_probe(struct snd_soc_component *codec)
{
	int ret;
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;
	if (platform_data == NULL) {
		AUDIO_LOGE("get platform data failed");
		return -ENOENT;
	}

	snd_soc_component_set_drvdata(codec, platform_data);
	platform_data->codec = codec;
	da_combine_v5_codec = codec;

	ret = da_combine_v5_resmgr_init(platform_data);
	if (ret != 0) {
		AUDIO_LOGE("resmgr init failed: 0x%x", ret);
		return -ENOMEM;
	}

	AUDIO_LOGI("version: 0x%x", snd_soc_component_read32(codec, VERSION_REG));

	ret = chip_init(codec);
	if (ret != 0) {
		da_combine_resmgr_deinit(platform_data->resmgr);
		AUDIO_LOGE("codec enumerate failed: 0x%x", ret);
		return ret;
	}

	if (platform_data->board_config.hp_res_detect_enable) {
		AUDIO_LOGI("hs res detect support");
		hs_cfg.res_detect_func = &hs_res_detect_func;
	}

	ret = codec_init(codec, platform_data);
	if (ret != 0) {
		da_combine_resmgr_deinit(platform_data->resmgr);
		AUDIO_LOGE("probe failed: 0x%x", ret);
		return ret;
	}


#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	ret = hicodec_debug_init(codec, &dump_info);
	if (ret != 0)
		AUDIO_LOGI("debug init failed: 0x%x", ret);
#endif

	if (platform_data->board_config.single_kcontrol_route_mode)
		ret = codec_add_driver_resource_for_single_control(codec);
	else
		ret = codec_add_driver_resource(codec);

	if (ret != 0) {
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
		hicodec_debug_uninit(codec);
#endif
		codec_deinit(platform_data);
		da_combine_resmgr_deinit(platform_data->resmgr);
		AUDIO_LOGE("add driver resource fail: 0x%x", ret);
		return ret;
	}

	ir_calibration(codec);

	if (platform_data->board_config.codec_extra_info_enable)
		da_combine_codec_info_select(codec);

	da_combine_v5_codec_pm_init(da_combine_v5_codec);

	rdr_audio_register_get_ap_reset_cfg_cb(get_ap_reset_cfg);

	AUDIO_LOGI("probe ok");

	return ret;
}

void da_combine_v5_codec_remove(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	if (platform_data == NULL) {
		AUDIO_LOGE("get platform data failed");
		return;
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif

	codec_deinit(platform_data);
	da_combine_resmgr_deinit(platform_data->resmgr);

	return;
}

