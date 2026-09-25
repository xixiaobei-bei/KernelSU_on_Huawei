/*
 * hi6405_kcontrol.c -- hi6405 codec driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "hi6405_kcontrol.h"

#include <linux/gpio.h>

#include <sound/core.h>
#include <sound/tlv.h>

#include "linux/hisi/audio_log.h"
#include "linux/hisi/hi64xx/hi6405.h"
#include "linux/hisi/hi64xx/hi6405_regs.h"
#include "linux/hisi/hi64xx/hi6405_type.h"
#include "linux/hisi/hi64xx/hi64xx_utils.h"
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
#include "huawei_platform/audio/high_resistance_hs_det.h"
#endif

/*
 * HSMIC PGA GAIN volume control:
 * from 0 to 30 dB in 2 dB steps
 * MAX VALUE is 15
 */
static DECLARE_TLV_DB_SCALE(hsmic_pga_tlv, 0, 200, 0);

/*
 * AUX PGA GAIN volume control:
 * from 0 to 30 dB in 2 dB steps
 * MAX VALUE is 15
 */
static DECLARE_TLV_DB_SCALE(auxmic_pga_tlv, 0, 200, 0);

/*
 * MIC3 PGA GAIN volume control:
 * from 0 to 30 dB in 2 dB steps
 * MAX VALUE is 15
 */
static DECLARE_TLV_DB_SCALE(mic3_pga_tlv, 0, 200, 0);

/*
 * MIC4 MIC GAIN volume control:
 * from 0 to 30 dB in 2 dB steps
 * MAX VALUE is 15
 */
static DECLARE_TLV_DB_SCALE(mic4_pga_tlv, 0, 200, 0);

/*
 * MIC5 PGA GAIN volume control:
 * from 0 to 30 dB in 2 dB steps
 * MAX VALUE is 15
 */
static DECLARE_TLV_DB_SCALE(mic5_pga_tlv, 0, 200, 0);

/*
 * MAD PGA GAIN volume control:
 * from 0 to 24 dB in 2 dB steps
 * MAX VALUE is 12
 */

static DECLARE_TLV_DB_SCALE(mad_pga_tlv, 0, 200, 0);

#ifdef CONFIG_HAC_SUPPORT
/* hac status */
#define HAC_ENABLE                   1
#define HAC_DISABLE                  0

static const char * const hac_switch_text[] = { "OFF", "ON" };

static const struct soc_enum hac_switch_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(hac_switch_text), hac_switch_text),
};

static int hac_status_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct hi6405_platform_data *priv = NULL;
	struct snd_soc_codec *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -1;
	}
	codec = snd_soc_kcontrol_codec(kcontrol);
	priv = snd_soc_codec_get_drvdata(codec);

	if (!gpio_is_valid(priv->board_config.hac_gpio)) {
		AUDIO_LOGE("hac gpio = %d is invalid",
			priv->board_config.hac_gpio);
		return -1;
	}
	ret = gpio_get_value(priv->board_config.hac_gpio);
	AUDIO_LOGI("hac gpio = %d, value = %d", priv->board_config.hac_gpio, ret);
	ucontrol->value.integer.value[0] = ret;

	return 0;
}

static int hac_status_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct hi6405_platform_data *priv = NULL;
	struct snd_soc_codec *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -1;
	}
	codec = snd_soc_kcontrol_codec(kcontrol);
	priv = snd_soc_codec_get_drvdata(codec);

	if (!gpio_is_valid(priv->board_config.hac_gpio)) {
		AUDIO_LOGE("hac gpio = %d is invalid",
			priv->board_config.hac_gpio);
		return -1;
	}
	ret = ucontrol->value.integer.value[0];
	if (ret == HAC_ENABLE)
		gpio_set_value(priv->board_config.hac_gpio, HAC_ENABLE);
	else
		gpio_set_value(priv->board_config.hac_gpio, HAC_DISABLE);

	return ret;
}

#define GPIO_HAC_KCONTROLS \
	SOC_ENUM_EXT("HAC", hac_switch_enum[0], \
		hi6405_hac_status_get, hac_status_set),\

#endif

#define S1_CFG_KCONTROLS \
	SOC_SINGLE("S1_IF_I2S_FS", \
		SC_CODEC_MUX_CTRL23_REG, FS_S1_OFFSET, \
		MAX_VAL_ON_BIT(FS_S1_LEN), 0), \
	SOC_SINGLE("S1_FUNC_MODE", \
		SC_S1_IF_L_REG, S1_FUNC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S1_FUNC_MODE_LEN), 0), \
	SOC_SINGLE("S1_FRAME_MODE", \
		SC_S1_IF_H_REG, S1_FRAME_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S1_FRAME_MODE_LEN), 0), \
	SOC_SINGLE("S1_RX_CLK_SEL", \
		SC_S1_IF_H_REG, S1_RX_CLK_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S1_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S1_TX_CLK_SEL", \
		SC_S1_IF_H_REG, S1_TX_CLK_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S1_TX_CLK_SEL_LEN), 0), \

#define S2_CFG_KCONTROLS \
	SOC_SINGLE("S2_IF_I2S_FS", \
		SC_CODEC_MUX_CTRL23_REG, FS_S2_OFFSET, \
		MAX_VAL_ON_BIT(FS_S2_LEN), 0), \
	SOC_SINGLE("S2_FUNC_MODE", \
		SC_S2_IF_L_REG, S2_FUNC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S2_FUNC_MODE_LEN), 0), \
	SOC_SINGLE("S2_DIRECT_MODE", \
		SC_S2_IF_L_REG, S2_DIRECT_LOOP_OFFSET, \
		MAX_VAL_ON_BIT(S2_DIRECT_LOOP_LEN), 0), \
	SOC_SINGLE("S2_FRAME_MODE", \
		SC_S2_IF_H_REG, S2_FRAME_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S2_FRAME_MODE_LEN), 0), \
	SOC_SINGLE("S2_RX_CLK_SEL", \
		SC_S2_IF_H_REG, S2_RX_CLK_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S2_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S2_TX_CLK_SEL", \
		SC_S2_IF_H_REG, S2_TX_CLK_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S2_TX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S2_FS_TDM", \
		SC_CODEC_MUX_CTRL24_REG, S2_FS_TDM_OFFSET, \
		MAX_VAL_ON_BIT(S2_FS_TDM_LEN), 0), \
	SOC_SINGLE("S2_TDM_FRAME_MODE", \
		S2_TDM_CTRL0_REG, S2_TDM_FRAME_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S2_TDM_FRAME_MODE_LEN), 0), \

#define S4_CFG_KCONTROLS \
	SOC_SINGLE("S4_IF_I2S_FS", \
		SC_CODEC_MUX_CTRL24_REG, FS_S4_OFFSET, \
		MAX_VAL_ON_BIT(FS_S4_LEN), 0), \
	SOC_SINGLE("S4_FUNC_MODE", \
		SC_S4_IF_L_REG, S4_FUNC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S4_FUNC_MODE_LEN), 0), \
	SOC_SINGLE("S4_FRAME_MODE", \
		SC_S4_IF_H_REG, S4_FRAME_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S4_FRAME_MODE_LEN), 0), \
	SOC_SINGLE("S4_DIRECT_MODE", \
		SC_S4_IF_L_REG, S4_DIRECT_LOOP_OFFSET, \
		MAX_VAL_ON_BIT(S4_DIRECT_LOOP_LEN), 0), \
	SOC_SINGLE("S4_RX_CLK_SEL", \
		SC_S4_IF_H_REG, S4_RX_CLK_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S4_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S4_TX_CLK_SEL", \
		SC_S4_IF_H_REG, S4_TX_CLK_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S4_TX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_IF_EN", \
		S4_TDM_CTRL0_REG, S4_TDM_IF_EN_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_IF_EN_LEN), 0), \
	SOC_SINGLE("S4_FS_TDM", \
		SC_CODEC_MUX_CTRL24_REG, S4_FS_TDM_OFFSET, \
		MAX_VAL_ON_BIT(S4_FS_TDM_LEN), 0), \
	SOC_SINGLE("S4_TDM_FRAME_MODE", \
		S4_TDM_CTRL0_REG, S4_TDM_FRAME_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_FRAME_MODE_LEN), 0), \

#define ANA_PGA_GAIN_KCONTROLS \
	SOC_SINGLE_TLV("HSMIC_PGA_GAIN", \
		ANA_HSMIC_MUX_AND_PGA, ANA_HSMIC_PGA_BIT, 15, 0, hsmic_pga_tlv), \
	SOC_SINGLE_TLV("AUXMIC_PGA_GAIN", \
		ANA_AUXMIC_MUX_AND_PGA, ANA_AUXMIC_PGA_BIT, 15, 0, auxmic_pga_tlv), \
	SOC_SINGLE_TLV("MIC3_PGA_GAIN", \
		ANA_MIC3_MUX_AND_PGA, ANA_MIC3_PGA_BIT, 15, 0, mic3_pga_tlv), \
	SOC_SINGLE_TLV("MIC4_PGA_GAIN", \
		ANA_MIC4_MUX_AND_PGA, ANA_MIC4_PGA_BIT, 15, 0, mic4_pga_tlv), \
	SOC_SINGLE_TLV("MIC5_PGA_GAIN", \
		ANA_MIC5_MUX_AND_PGA, ANA_MIC5_PGA_BIT, 15, 0, mic5_pga_tlv), \
	SOC_SINGLE_TLV("MAD_PGA_GAIN", \
		ANA_MAD_PGA, ANA_MAD_PGA_BIT, 12, 0, mad_pga_tlv), \
	SOC_SINGLE("EP_GAIN", \
		CODEC_ANA_RWREG_050, CODEC_ANA_EP_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(CODEC_ANA_EP_GAIN_LEN), 0), \

#define DAC_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("DACL_MIXER_S1L_GAIN", \
		DACL_MIXER4_CTRL2_REG, DACL_MIXER4_GAIN1_OFFSET, \
		MAX_VAL_ON_BIT(DACL_MIXER4_GAIN1_LEN), 0), \
	SOC_SINGLE("DACL_MIXER_S2L_GAIN", \
		DACL_MIXER4_CTRL2_REG, DACL_MIXER4_GAIN2_OFFSET, \
		MAX_VAL_ON_BIT(DACL_MIXER4_GAIN2_LEN), 0), \
	SOC_SINGLE("DACL_MIXER_S3L_GAIN", \
		DACL_MIXER4_CTRL2_REG, DACL_MIXER4_GAIN3_OFFSET, \
		MAX_VAL_ON_BIT(DACL_MIXER4_GAIN3_LEN), 0), \
	SOC_SINGLE("DACL_MIXER_S1R_GAIN", \
		DACL_MIXER4_CTRL2_REG, DACL_MIXER4_GAIN4_OFFSET, \
		MAX_VAL_ON_BIT(DACL_MIXER4_GAIN4_LEN), 0), \
	SOC_SINGLE("DACR_MIXER_S1R_GAIN", \
		DACR_MIXER4_CTRL2_REG, DACR_MIXER4_GAIN1_OFFSET, \
		MAX_VAL_ON_BIT(DACR_MIXER4_GAIN1_LEN), 0), \
	SOC_SINGLE("DACR_MIXER_S2R_GAIN", \
		DACR_MIXER4_CTRL2_REG, DACR_MIXER4_GAIN2_OFFSET, \
		MAX_VAL_ON_BIT(DACR_MIXER4_GAIN2_LEN), 0), \
	SOC_SINGLE("DACR_MIXER_MDM_GAIN", \
		DACR_MIXER4_CTRL2_REG, DACR_MIXER4_GAIN3_OFFSET, \
		MAX_VAL_ON_BIT(DACR_MIXER4_GAIN3_LEN), 0), \
	SOC_SINGLE("DACR_MIXER_S1L_GAIN", \
		DACR_MIXER4_CTRL2_REG, DACR_MIXER4_GAIN4_OFFSET, \
		MAX_VAL_ON_BIT(DACR_MIXER4_GAIN4_LEN), 0), \
	SOC_SINGLE("DACL_PRE_MIXER_MUX9_GAIN", \
		DACL_PRE_MIXER2_CTRL1_REG, DACL_PRE_MIXER2_GAIN1_OFFSET, \
		MAX_VAL_ON_BIT(DACL_PRE_MIXER2_GAIN1_LEN), 0), \
	SOC_SINGLE("DACL_PRE_MIXER_SIDETONE_GAIN", \
		DACL_PRE_MIXER2_CTRL1_REG, DACL_PRE_MIXER2_GAIN2_OFFSET, \
		MAX_VAL_ON_BIT(DACL_PRE_MIXER2_GAIN2_LEN), 0), \
	SOC_SINGLE("DACR_PRE_MIXER_MUX10_GAIN", \
		DACR_PRE_MIXER2_CTRL1_REG, DACR_PRE_MIXER2_GAIN1_OFFSET, \
		MAX_VAL_ON_BIT(DACR_PRE_MIXER2_GAIN1_LEN), 0), \
	SOC_SINGLE("DACR_PRE_MIXER_SIDETONE_GAIN", \
		DACR_PRE_MIXER2_CTRL1_REG, DACR_PRE_MIXER2_GAIN2_OFFSET, \
		MAX_VAL_ON_BIT(DACR_PRE_MIXER2_GAIN2_LEN), 0), \
	SOC_SINGLE("DACL_POST_MIXER_DAC_GAIN", \
		DACL_POST_MIXER2_CTRL1_REG, DACL_POST_MIXER2_GAIN1_OFFSET, \
		MAX_VAL_ON_BIT(DACL_POST_MIXER2_GAIN1_LEN), 0), \
	SOC_SINGLE("DACL_POST_MIXER_S1L_GAIN", \
		DACL_POST_MIXER2_CTRL1_REG, DACL_POST_MIXER2_GAIN2_OFFSET, \
		MAX_VAL_ON_BIT(DACL_POST_MIXER2_GAIN2_LEN), 0), \
	SOC_SINGLE("DACR_POST_MIXER_DAC_GAIN", \
		DACR_POST_MIXER2_CTRL1_REG, DACR_POST_MIXER2_GAIN1_OFFSET, \
		MAX_VAL_ON_BIT(DACR_POST_MIXER2_GAIN1_LEN), 0), \
	SOC_SINGLE("DACR_POST_MIXER_S1R_GAIN", \
		DACR_POST_MIXER2_CTRL1_REG, DACR_POST_MIXER2_GAIN2_OFFSET, \
		MAX_VAL_ON_BIT(DACR_POST_MIXER2_GAIN2_LEN), 0), \
	SOC_SINGLE("DACSL_MIXER_UTW_GAIN", \
		DACSL_MIXER4_CTRL2_REG, DACSL_MIXER4_GAIN1_OFFSET, \
		MAX_VAL_ON_BIT(DACSL_MIXER4_GAIN1_LEN), 0), \
	SOC_SINGLE("DACSL_MIXER_PGA_GAIN", \
		DACSL_MIXER4_CTRL2_REG, DACSL_MIXER4_GAIN2_OFFSET, \
		MAX_VAL_ON_BIT(DACSL_MIXER4_GAIN2_LEN), 0), \
	SOC_SINGLE("DACSL_MIXER_MDM_GAIN", \
		DACSL_MIXER4_CTRL2_REG, DACSL_MIXER4_GAIN3_OFFSET, \
		MAX_VAL_ON_BIT(DACSL_MIXER4_GAIN3_LEN), 0), \
	SOC_SINGLE("DACSL_MIXER_SIDETONE_GAIN", \
		DACSL_MIXER4_CTRL2_REG, DACSL_MIXER4_GAIN4_OFFSET, \
		MAX_VAL_ON_BIT(DACSL_MIXER4_GAIN4_LEN), 0), \
	SOC_SINGLE("DACL_PRE_PGA_GAIN", \
		DACL_PRE_PGA_CTRL1_REG, DACL_PRE_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(DACL_PRE_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("DACR_PRE_PGA_GAIN", \
		DACR_PRE_PGA_CTRL1_REG, DACR_PRE_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(DACR_PRE_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("DACL_POST_PGA_GAIN", \
		DACL_POST_PGA_CTRL1_REG, DACL_POST_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(DACL_POST_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("DACR_POST_PGA_GAIN", \
		DACR_POST_PGA_CTRL1_REG, DACR_POST_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(DACR_POST_PGA_GAIN_LEN), 0), \

#define S_1_4_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("S1_IL_PGA_GAIN", \
		S1_IL_PGA_CTRL1_REG, S1_IL_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(S1_IL_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S1_IR_PGA_GAIN", \
		S1_IR_PGA_CTRL1_REG, S1_IR_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(S1_IR_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S2_IL_PGA_GAIN", \
		S2_IL_PGA_CTRL1_REG, S2_IL_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(S2_IL_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S2_IR_PGA_GAIN", \
		S2_IR_PGA_CTRL1_REG, S2_IR_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(S2_IR_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S3_IL_PGA_GAIN", \
		S3_IL_PGA_CTRL1_REG, S3_IL_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(S3_IL_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S3_IR_PGA_GAIN", \
		S3_IR_PGA_CTRL1_REG, S3_IR_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(S3_IR_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S2_OL_PGA_GAIN", \
		S2_OL_PGA_CTRL1_REG, S2_OL_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(S2_OL_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S2_OR_PGA_GAIN", \
		S2_OR_PGA_CTRL1_REG, S2_OR_PGA_GAIN_OFFSET, \
		MAX_VAL_ON_BIT(S2_OR_PGA_GAIN_LEN), 0), \

#define ADC_BOOST_KCONTROLS \
	SOC_SINGLE("HSMIC_ADC_BOOST", \
		CODEC_ANA_RWREG_020, CODEC_ANA_HSMIC_ADC_BOOST, 3, 0), \
	SOC_SINGLE("AUXMIC_ADC_BOOST", \
		CODEC_ANA_RWREG_024, CODEC_ANA_AUXMIC_ADC_BOOST, 3, 0), \
	SOC_SINGLE("MIC3_ADC_BOOST", \
		CODEC_ANA_RWREG_028, CODEC_ANA_MIC3_ADC_BOOST, 3, 0), \
	SOC_SINGLE("MIC4_ADC_BOOST", \
		CODEC_ANA_RWREG_032, CODEC_ANA_MIC4_ADC_BOOST, 3, 0), \
	SOC_SINGLE("MIC5_ADC_BOOST", \
		CODEC_ANA_RWREG_036, CODEC_ANA_MIC5_ADC_BOOST, 3, 0), \
	SOC_SINGLE("MAD_ADC_BOOST", \
		ANA_MAD_PGA, ANA_MAD_BOOST_18_OFFSET, 1, 0), \

#define SRC_KCONTROLS \
	SOC_SINGLE("S1_IL_SRC_EN", \
		S1_DP_CLK_EN_REG, S1_IL_SRC_CLK_EN_OFFSET, \
		MAX_VAL_ON_BIT(S1_IL_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_IR_SRC_EN", \
		S1_DP_CLK_EN_REG, S1_IR_SRC_CLK_EN_OFFSET, \
		MAX_VAL_ON_BIT(S1_IR_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_MIC1_SRC_EN", \
		S1_DP_CLK_EN_REG, S1_MIC1_SRC_CLK_EN_OFFSET, \
		MAX_VAL_ON_BIT(S1_MIC1_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_MIC2_SRC_EN", \
		S1_DP_CLK_EN_REG, S1_MIC2_SRC_CLK_EN_OFFSET, \
		MAX_VAL_ON_BIT(S1_MIC2_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_MIC1_SRC_MODE", \
		MIC1_SRC_CTRL_REG, MIC1_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(MIC1_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S1_MIC2_SRC_MODE", \
		MIC2_SRC_CTRL_REG, MIC2_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(MIC2_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S1_MIC1_SRC_FS", \
		S1_DP_CLK_EN_REG, S1_MIC1_SRC_CLK_EN_OFFSET, \
		MAX_VAL_ON_BIT(S1_MIC1_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_MIC2_SRC_FS", \
		S1_DP_CLK_EN_REG, S1_MIC2_SRC_CLK_EN_OFFSET, \
		MAX_VAL_ON_BIT(S1_MIC2_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S2_IL_SRC_MODE", \
		S2_IL_SRC_CTRL_REG, S2_IL_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S2_IL_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S2_IR_SRC_MODE", \
		S2_IR_SRC_CTRL_REG, S2_IR_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S2_IR_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S2_I_SRC_IN_FS", \
		SRC_VLD_CTRL0_REG, S2_I_SRC_DIN_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S2_I_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_IL_SRC_OUT_FS", \
		SRC_VLD_CTRL0_REG, S2_IL_SRC_DOUT_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S2_IL_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_IR_SRC_OUT_FS", \
		SRC_VLD_CTRL0_REG, S2_IR_SRC_DOUT_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S2_IR_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_OL_SRC_MODE", \
		S2_OL_SRC_CTRL_REG, S2_OL_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S2_OL_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S2_OR_SRC_MODE", \
		S2_OR_SRC_CTRL_REG, S2_OR_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S2_OR_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S2_O_SRC_OUT_FS", \
		SRC_VLD_CTRL6_REG, S2_O_SRC_DOUT_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S2_O_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_OL_SRC_IN_FS", \
		SRC_VLD_CTRL6_REG, S2_OL_SRC_DIN_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S2_OL_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_OR_SRC_IN_FS", \
		SRC_VLD_CTRL6_REG, S2_OR_SRC_DIN_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S2_OR_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_IL_SRC_MODE", \
		S3_IL_SRC_CTRL_REG, S3_IL_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S3_IL_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S3_IR_SRC_MODE", \
		S2_IR_SRC_CTRL_REG, S3_IR_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S3_IR_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S3_I_SRC_IN_FS", \
		SRC_VLD_CTRL1_REG, S3_I_SRC_DIN_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S3_I_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_IL_SRC_OUT_FS", \
		SRC_VLD_CTRL1_REG, S3_IL_SRC_DOUT_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S3_IL_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_IR_SRC_OUT_FS", \
		SRC_VLD_CTRL1_REG, S3_IR_SRC_DOUT_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S3_IR_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_OL_SRC_MODE", \
		S3_OL_SRC_CTRL_REG, S3_OL_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S3_OL_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S3_OR_SRC_MODE", \
		S3_OR_SRC_CTRL_REG, S3_OR_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S3_OR_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S3_O_SRC_OUT_FS", \
		SRC_VLD_CTRL7_REG, S3_O_SRC_DOUT_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S3_O_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_OL_SRC_IN_FS", \
		SRC_VLD_CTRL7_REG, S3_OL_SRC_DIN_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S3_OL_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_OR_SRC_IN_FS", \
		SRC_VLD_CTRL7_REG, S3_OR_SRC_DIN_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S3_OR_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("SPA_OL_SRC_MODE", \
		SPA_OL_SRC_CTRL_REG, SPA_OL_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(SPA_OL_SRC_MODE_OFFSET), 0), \
	SOC_SINGLE("SPA_OR_SRC_MODE", \
		SPA_OR_SRC_CTRL_REG, SPA_OR_SRC_MODE_OFFSET, \
		MAX_VAL_ON_BIT(SPA_OR_SRC_MODE_OFFSET), 0), \
	SOC_SINGLE("SPA_OL_SRC_IN_FS", \
		SRC_VLD_CTRL9_REG, SPA_OL_SRC_DIN_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(SPA_OL_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("SPA_OR_SRC_IN_FS", \
		SRC_VLD_CTRL10_REG, SPA_OR_SRC_DIN_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(SPA_OR_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("SPA_OL_SRC_OUT_FS", \
		SRC_VLD_CTRL9_REG, SPA_OL_SRC_DOUT_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(SPA_OL_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("SPA_OR_SRC_OUT_FS", \
		SRC_VLD_CTRL10_REG, SPA_OR_SRC_DOUT_VLD_SEL_OFFSET, \
		MAX_VAL_ON_BIT(SPA_OR_SRC_DOUT_VLD_SEL_LEN), 0), \

#define PGA_BYPASS_KCONTROLS \
	SOC_SINGLE("S1_IL_PGA_BYPASS", \
		S1_IL_PGA_CTRL0_REG, S1_IL_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(S1_IL_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S1_IR_PGA_BYPASS", \
		S1_IR_PGA_CTRL0_REG, S1_IR_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(S1_IR_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S2_IL_PGA_BYPASS", \
		S2_IL_PGA_CTRL0_REG, S2_IL_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(S2_IL_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S2_IR_PGA_BYPASS", \
		S2_IR_PGA_CTRL0_REG, S2_IR_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(S2_IR_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S2_OL_PGA_BYPASS", \
		S2_OL_PGA_CTRL0_REG, S2_OL_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(S2_OL_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S2_OR_PGA_BYPASS", \
		S2_OR_PGA_CTRL0_REG, S2_OR_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(S2_OR_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S3_IL_PGA_BYPASS", \
		S3_IL_PGA_CTRL0_REG, S3_IL_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(S3_IL_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S3_IR_PGA_BYPASS", \
		S3_IR_PGA_CTRL0_REG, S3_IR_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(S3_IR_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("DACL_PRE_PGA_BYPASS", \
		DACL_PRE_PGA_CTRL0_REG, DACL_PRE_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(DACL_PRE_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("DACR_PRE_PGA_BYPASS", \
		DACR_PRE_PGA_CTRL0_REG, DACR_PRE_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(DACR_PRE_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("DACL_POST_PGA_BYPASS", \
		DACL_POST_PGA_CTRL0_REG, DACL_POST_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(DACL_POST_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("DACR_POST_PGA_BYPASS", \
		DACR_POST_PGA_CTRL0_REG, DACR_POST_PGA_BYPASS_OFFSET, \
		MAX_VAL_ON_BIT(DACR_POST_PGA_BYPASS_LEN), 0), \

#define SIDETONE_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("SIDETONE_SRC_CLK_EN", \
		DAC_DP_CLK_EN_2_REG, SIDE_SRC_CLK_EN_OFFSET, 1, 0), \
	SOC_SINGLE("SIDETONE_PGA_CLK_EN", \
		DAC_MIXER_CLK_EN_REG, SIDE_PGA_CLK_EN_OFFSET, 1, 0), \
	SOC_SINGLE("SIDETONE_PGA_GAIN", \
		SIDE_PGA_CTRL1_REG, SIDE_PGA_GAIN_OFFSET, 0xff, 0), \

#define SLIMBUS_DN_PORT_FS_KCONTROLS \
	SOC_SINGLE("SLIMBUS_DN3_FS", \
		SC_FS_SLIM_CTRL_1_REG, FS_SLIM_DN3_OFFSET, \
		MAX_VAL_ON_BIT(FS_SLIM_DN3_LEN), 0), \
	SOC_SINGLE("SLIMBUS_DN4_FS", \
		SC_FS_SLIM_CTRL_1_REG, FS_SLIM_DN4_OFFSET, \
		MAX_VAL_ON_BIT(FS_SLIM_DN4_LEN), 0), \
	SOC_SINGLE("SLIMBUS_DN5_FS", \
		SC_FS_SLIM_CTRL_2_REG, FS_SLIM_DN5_OFFSET, \
		MAX_VAL_ON_BIT(FS_SLIM_DN5_LEN), 0), \
	SOC_SINGLE("SLIMBUS_DN6_FS", \
		SC_FS_SLIM_CTRL_2_REG, FS_SLIM_DN6_OFFSET, \
		MAX_VAL_ON_BIT(FS_SLIM_DN6_LEN), 0), \

#define S4_TDM_CFG_KCONTROLS \
	SOC_SINGLE("S4_TDM_TX_CH0_SEL", \
		S4_TDM_CTRL1_REG, S4_TDM_TX_CH0_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_TX_CH0_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_TX_CH1_SEL", \
		S4_TDM_CTRL1_REG, S4_TDM_TX_CH1_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_TX_CH1_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_TX_CH2_SEL", \
		S4_TDM_CTRL1_REG, S4_TDM_TX_CH2_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_TX_CH2_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_TX_CH3_SEL", \
		S4_TDM_CTRL1_REG, S4_TDM_TX_CH3_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_TX_CH3_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_CLK_SEL", \
		S4_TDM_CTRL2_REG, S4_TDM_RX_CLK_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_TX_CLK_SEL", \
		S4_TDM_CTRL2_REG, S4_TDM_TX_CLK_SEL_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_TX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_I0_SEL", \
		S4_TDM_CTRL2_REG, S4_TDM_RX_SLOT_SEL_I0_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_RX_SLOT_SEL_I0_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_V0_SEL", \
		S4_TDM_CTRL2_REG, S4_TDM_RX_SLOT_SEL_V0_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_RX_SLOT_SEL_V0_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_I1_SEL", \
		S4_TDM_CTRL3_REG, S4_TDM_RX_SLOT_SEL_I1_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_RX_SLOT_SEL_I1_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_V1_SEL", \
		S4_TDM_CTRL3_REG, S4_TDM_RX_SLOT_SEL_V1_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_RX_SLOT_SEL_V1_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_I2_SEL", \
		S4_TDM_CTRL4_REG, S4_TDM_RX_SLOT_SEL_I2_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_RX_SLOT_SEL_I2_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_V2_SEL", \
		S4_TDM_CTRL4_REG, S4_TDM_RX_SLOT_SEL_V2_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_RX_SLOT_SEL_V2_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_I3_SEL", \
		S4_TDM_CTRL5_REG, S4_TDM_RX_SLOT_SEL_I3_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_RX_SLOT_SEL_I3_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_V3_SEL", \
		S4_TDM_CTRL5_REG, S4_TDM_RX_SLOT_SEL_V3_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_RX_SLOT_SEL_V3_LEN), 0), \
	SOC_SINGLE("S4_I2S_TDM_MODE", \
		S4_TDM_CTRL6_REG, S4_I2S_TDM_MODE_OFFSET, \
		MAX_VAL_ON_BIT(S4_I2S_TDM_MODE_LEN), 0), \
	SOC_SINGLE("S4_TDM_MST_SLV_SEL", \
		S4_TDM_CTRL6_REG, S4_TDM_MST_SLV_OFFSET, \
		MAX_VAL_ON_BIT(S4_TDM_MST_SLV_LEN), 0), \

/* mic fixed kcontrols names don't represent the actual meaning */
#define MIC_CALIB_KCONTROLS \
	SOC_SINGLE("ADC0L 05PGA GAIN", VIRTUAL_MAIN_MIC_CALIB_REG, \
		VIRTUAL_MAIN_MIC_CALIB_SHFIT_BIT, MIC_CALIB_MAX_VALUE, 0), \
	SOC_SINGLE("ADC0R 05PGA GAIN", VIRTUAL_MAIN_MIC2_CALIB_REG, \
		VIRTUAL_MAIN_MIC2_CALIB_SHFIT_BIT, MIC_CALIB_MAX_VALUE, 0), \
	SOC_SINGLE("ADC1L 05PGA GAIN", VIRTUAL_SUB_MIC_CALIB_REG, \
		VIRTUAL_SUB_MIC_CALIB_SHFIT_BIT, MIC_CALIB_MAX_VALUE, 0), \
	SOC_SINGLE("ADC1R 05PGA GAIN", VIRTUAL_SUB_MIC2_CALIB_REG, \
		VIRTUAL_SUB_MIC2_CALIB_SHFIT_BIT, MIC_CALIB_MAX_VALUE, 0), \

#ifdef AUDIO_FACTORY_MODE
static const char * const micbias_text[] = { "OFF", "ON" };

static const struct soc_enum micbias_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micbias_text), micbias_text),
};

static int main_micbias_status_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct hi6405_platform_data *priv = NULL;
	struct snd_soc_codec *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -1;
	}

	codec = snd_soc_kcontrol_codec(kcontrol);
	if (codec == NULL) {
		AUDIO_LOGE("codec pointer is null");
		return -1;
	}

	priv = snd_soc_codec_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv pointer is null");
		return -1;
	}

	AUDIO_LOGI("main micbias val:%d", priv->mainmicbias_val);
	ucontrol->value.integer.value[0] = priv->mainmicbias_val;

	return 0;
}

static int main_micbias_status_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct hi6405_platform_data *priv = NULL;
	struct snd_soc_codec *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -1;
	}

	codec = snd_soc_kcontrol_codec(kcontrol);
	if (codec == NULL) {
		AUDIO_LOGE("codec pointer is null");
		return -1;
	}

	priv = snd_soc_codec_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv pointer is null");
		return -1;
	}

	priv->mainmicbias_val = ucontrol->value.integer.value[0];
	if (priv->mainmicbias_val == 1) { /* 1 mean ON define in micbias_text */
		AUDIO_LOGI("mainmic bias on");
		hi64xx_resmgr_request_micbias(priv->resmgr);
		hi64xx_update_bits(codec, ANA_MICBIAS1,
			MASK_ON_BIT(ANA_MICBIAS1_ADJ_LEN,
				ANA_MICBIAS1_ADJ_OFFSET) |
				BIT(ANA_MICBIAS1_DSCHG_OFFSET),
				ANA_MICBIAS1_ADJ_2_7V);
		hi64xx_update_bits(codec, ANA_MICBIAS_ENABLE,
			BIT(ANA_MICBIAS1_PD_BIT), 0);
	} else { /* 0 mean OFF define in micbias_text */
		AUDIO_LOGI("mainmic bias off");
		hi64xx_update_bits(codec, ANA_MICBIAS_ENABLE,
			BIT(ANA_MICBIAS1_PD_BIT), BIT(ANA_MICBIAS1_PD_BIT));
		hi64xx_update_bits(codec, ANA_MICBIAS1,
			MASK_ON_BIT(ANA_MICBIAS1_ADJ_LEN,
				ANA_MICBIAS1_ADJ_OFFSET) |
				BIT(ANA_MICBIAS1_DSCHG_OFFSET),
				BIT(ANA_MICBIAS1_DSCHG_OFFSET));
		usleep_range(10000, 11000);
		/* set MICBIAS1 set output 0V */
		hi64xx_update_bits(codec, ANA_MICBIAS1,
			MASK_ON_BIT(ANA_MICBIAS1_ADJ_LEN,
				ANA_MICBIAS1_ADJ_OFFSET) |
				BIT(ANA_MICBIAS1_DSCHG_OFFSET),
				0);
		hi64xx_resmgr_release_micbias(priv->resmgr);
	}

	return 0;
}

#define MAIN_MICBIAS_KCONTROLS \
	SOC_ENUM_EXT("MAIN MICBIAS", micbias_enum[0], \
		main_micbias_status_get, \
		main_micbias_status_put), \

#endif

#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
static const char * const high_res_hs_text[] = {
	"0", "1", "2", "3", "4", "5", "6", "7", "8",
	"9", "10", "255", "254", "253", "252", "251",
}; /* gain array */

static const struct soc_enum high_res_hs_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(high_res_hs_text), high_res_hs_text),
};

static int high_res_hs_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct hi6405_platform_data *priv = NULL;
	struct snd_soc_codec *codec = NULL;

	if ((kcontrol == NULL) || (ucontrol == NULL)) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}
	codec = snd_soc_kcontrol_codec(kcontrol);
	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);
	if (check_high_res_hs_det_support())
		ret = get_high_res_data(HIGH_RES_GET_HS_STATE);

	AUDIO_LOGI("high_res_hs_status is %d", ret);
	ucontrol->value.integer.value[0] = ret;
	return 0;
}

static int high_res_hs_set(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	unsigned int gain;
	unsigned int i; /* index of high_res_hs_text */
	struct hi6405_platform_data *priv = NULL;
	struct snd_soc_codec *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}

	codec = snd_soc_kcontrol_codec(kcontrol);
	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	if (!check_high_res_hs_det_support()) {
		AUDIO_LOGI("not support hs res hs det");
		return 0;
	}

	if (get_high_res_data(HIGH_RES_GET_HS_STATE) != HIGH_RES_HS) {
		AUDIO_LOGI("high_res_hs_status is %d", NORMAL_HS);
		return 0;
	}
	i = ucontrol->value.integer.value[0];
	if (i >= ARRAY_SIZE(high_res_hs_text)) {
		AUDIO_LOGI("invalid index of gain array");
		i = 0;
	}
	/* 10-str will be parsed as a decimal */
	ret = kstrtouint(high_res_hs_text[i], 10, &gain);
	if (ret < 0) {
		AUDIO_LOGI("str convert to int fail");
		gain = 0;
	}
	AUDIO_LOGI("index of gain array is %u, gain is %u", i, gain);
	hi64xx_update_bits(codec, S1_IL_PGA_CTRL1_REG, 0xff, gain);
	hi64xx_update_bits(codec, S1_IR_PGA_CTRL1_REG, 0xff, gain);
	return 0;
}

#define HI6405_HIGH_RES_HS_KCONTROLS \
	SOC_ENUM_EXT("HIGH_RES_HS_GAIN", high_res_hs_enum[0], \
		high_res_hs_get, high_res_hs_set), \

#endif /* CONFIG_HIGH_RESISTANCE_HS_DET */


static const struct snd_kcontrol_new snd_controls[] = {
	S1_CFG_KCONTROLS
	S2_CFG_KCONTROLS
	S4_CFG_KCONTROLS
	SRC_KCONTROLS
	ANA_PGA_GAIN_KCONTROLS
	DAC_PGA_GAIN_KCONTROLS
	S_1_4_PGA_GAIN_KCONTROLS
	ADC_BOOST_KCONTROLS
	PGA_BYPASS_KCONTROLS
	SIDETONE_PGA_GAIN_KCONTROLS
	MIC_CALIB_KCONTROLS
#ifdef CONFIG_HAC_SUPPORT
	GPIO_HAC_KCONTROLS
#endif
	SLIMBUS_DN_PORT_FS_KCONTROLS
	S4_TDM_CFG_KCONTROLS
#ifdef AUDIO_FACTORY_MODE
	MAIN_MICBIAS_KCONTROLS
#endif
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
	HI6405_HIGH_RES_HS_KCONTROLS
#endif
};


int hi6405_add_kcontrol(struct snd_soc_codec *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	return snd_soc_add_codec_controls(codec, snd_controls,
		ARRAY_SIZE(snd_controls));
}

