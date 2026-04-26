/*
 * path_widget.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "path_widget.h"

#include <sound/core.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/jiffies.h>

#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_v5.h"
#include "linux/hisi/da_combine/da_combine_v5_regs.h"
#include "linux/hisi/da_combine/da_combine_utils.h"

#define INPUT_WIDGET \
	SND_SOC_DAPM_INPUT("MAINMIC_INPUT"), \
	SND_SOC_DAPM_INPUT("AUXMIC_INPUT"), \
	SND_SOC_DAPM_INPUT("MIC3_INPUT"), \
	SND_SOC_DAPM_INPUT("MIC4_INPUT"), \
	SND_SOC_DAPM_INPUT("MIC5_INPUT"), \
	SND_SOC_DAPM_INPUT("IR_STUDY_INPUT"), \
	SND_SOC_DAPM_INPUT("HSMIC_INPUT"), \
	SND_SOC_DAPM_INPUT("AUDIOUP_INPUT"), \
	SND_SOC_DAPM_INPUT("VOICE_INPUT"), \
	SND_SOC_DAPM_INPUT("DSD_L_INPUT"), \
	SND_SOC_DAPM_INPUT("DSD_R_INPUT"), \
	SND_SOC_DAPM_INPUT("D1_D2_INPUT"), \
	SND_SOC_DAPM_INPUT("D3_D4_INPUT"), \
	SND_SOC_DAPM_INPUT("D5_D6_INPUT"), \
	SND_SOC_DAPM_INPUT("S1_RX_INPUT"), \
	SND_SOC_DAPM_INPUT("S2_RX_INPUT"), \
	SND_SOC_DAPM_INPUT("S4_RX_INPUT"), \
	SND_SOC_DAPM_INPUT("DMIC1_INPUT"), \
	SND_SOC_DAPM_INPUT("DMIC2_INPUT"), \
	SND_SOC_DAPM_INPUT("MIC_INPUT"), \
	SND_SOC_DAPM_INPUT("IR_TX_INPUT"), \
	SND_SOC_DAPM_INPUT("AUDIO_DOWN_INPUT"), \
	SND_SOC_DAPM_INPUT("U7_EC_INPUT"), \
	SND_SOC_DAPM_INPUT("PLL_TEST_INPUT"), \
	SND_SOC_DAPM_INPUT("HP_HIGHLEVEL_INPUT"), \
	SND_SOC_DAPM_INPUT("I2S2_BLUETOOTH_LOOP_INPUT"), \
	SND_SOC_DAPM_INPUT("TDM_AUDIO_PA_DOWN_INPUT"), \
	SND_SOC_DAPM_INPUT("FM_INPUT"), \
	SND_SOC_DAPM_INPUT("HP_CONCURRENCY_INPUT"),\
	SND_SOC_DAPM_INPUT("EP_ULTRASONIC_DOWN_INPUT"),\

#define OUTPUT_WIDGET \
	SND_SOC_DAPM_OUTPUT("HP_L_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("HP_R_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("EP_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("DSD_L_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("DSD_R_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S1_TX_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S2_TX_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S4_TX_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("IR_LED_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO_DOWN_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U7_EC_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("PLL_TEST_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("HP_HIGHLEVEL_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("IV_DSPIF_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U1_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U2_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U3_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U4_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U5_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U6_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U7_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U8_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U9_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U10_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S2L_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S2R_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S4L_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S4R_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("MAD_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("VIRTUAL_BTN_ACTIVE_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("VIRTUAL_BTN_PASSIVE_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("IR_STUDY_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIOUP_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("VOICE_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("I2S2_BLUETOOTH_LOOP_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("TDM_AUDIO_PA_DOWN_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("FM_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("HP_CONCURRENCY_OUTPUT"),\
	SND_SOC_DAPM_OUTPUT("AUXMIC_ULTRASONIC_UP_OUTPUT"),\

static const struct snd_kcontrol_new mad_pga_mixer_controls[] = {
	SOC_DAPM_SINGLE("MAINMIC_ENABLE",
		ANA_MAD_PGA_MIXER, ANA_MAD_PGA_MAINMIC_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("AUXMIC_ENABLE",
		ANA_MAD_PGA_MIXER, ANA_MAD_PGA_AUXMIC_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("MIC3_ENABLE",
		ANA_MAD_PGA_MIXER, ANA_MAD_PGA_MIC3_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("MIC4_ENABLE",
		ANA_MAD_PGA_MIXER, ANA_MAD_PGA_MIC4_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("MIC5_ENABLE",
		ANA_MAD_PGA_MIXER, ANA_MAD_PGA_MIC5_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("HSMIC_ENABLE",
		ANA_MAD_PGA_MIXER, ANA_MAD_PGA_HSMIC_SEL_BIT, 1, 0),
};

/* MIXER */
static const struct snd_kcontrol_new dsdl_mixer_controls[] = {
	SOC_DAPM_SINGLE("PGA",
		DSDL_MIXER2_CTRL1_REG, DSDL_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("DACL_384K",
		DSDL_MIXER2_CTRL1_REG, DSDL_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dsdr_mixer_controls[] = {
	SOC_DAPM_SINGLE("PGA",
		DSDR_MIXER2_CTRL1_REG, DSDR_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("DACR_384K",
		DSDR_MIXER2_CTRL1_REG, DSDR_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dacl_mixer_controls[] = {
	SOC_DAPM_SINGLE("S1_L",
		DACL_MIXER4_CTRL1_REG, DACL_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S2_L",
		DACL_MIXER4_CTRL1_REG, DACL_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S3_L",
		DACL_MIXER4_CTRL1_REG, DACL_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S1_R",
		DACL_MIXER4_CTRL1_REG, DACL_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dacr_mixer_controls[] = {
	SOC_DAPM_SINGLE("S1_R",
		DACR_MIXER4_CTRL1_REG, DACR_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S2_R",
		DACR_MIXER4_CTRL1_REG, DACR_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("MDM",
		DACR_MIXER4_CTRL1_REG, DACR_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S1_L",
		DACR_MIXER4_CTRL1_REG, DACR_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dacl_pre_mixer_controls[] = {
	SOC_DAPM_SINGLE("MUX9",
		DACL_PRE_MIXER2_CTRL0_REG, DACL_PRE_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE",
		DACL_PRE_MIXER2_CTRL0_REG, DACL_PRE_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dacr_pre_mixer_controls[] = {
	SOC_DAPM_SINGLE("MUX10",
		DACR_PRE_MIXER2_CTRL0_REG, DACR_PRE_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE",
		DACR_PRE_MIXER2_CTRL0_REG, DACR_PRE_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dacl_post_mixer_controls[] = {
	SOC_DAPM_SINGLE("DACLSRC",
		DACL_POST_MIXER2_CTRL0_REG, DACL_POST_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S1_L",
		DACL_POST_MIXER2_CTRL0_REG, DACL_POST_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dacr_post_mixer_controls[] = {
	SOC_DAPM_SINGLE("DACRSRC",
		DACR_POST_MIXER2_CTRL0_REG, DACR_POST_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S1_R",
		DACR_POST_MIXER2_CTRL0_REG, DACR_POST_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dacsl_mixer_controls[] = {
	SOC_DAPM_SINGLE("UTW",
		DACSL_MIXER4_CTRL1_REG, DACSL_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("DACSL_PGA",
		DACSL_MIXER4_CTRL1_REG, DACSL_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("MDM",
		DACSL_MIXER4_CTRL1_REG, DACSL_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE",
		DACSL_MIXER4_CTRL1_REG, DACSL_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dacsr_mixer_controls[] = {
	SOC_DAPM_SINGLE("UTW",
		DACSR_MIXER4_CTRL1_REG, DACSR_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("DACSR_PGA",
		DACSR_MIXER4_CTRL1_REG, DACSR_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("MDM",
		DACSR_MIXER4_CTRL1_REG, DACSR_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE",
		DACSR_MIXER4_CTRL1_REG, DACSR_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

void dacl_post_mixer_enable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		BIT(CLK_DACL_EN_OFFSET), BIT(CLK_DACL_EN_OFFSET));
}

void dacl_post_mixer_disable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		BIT(CLK_DACL_EN_OFFSET), 0);
}

static int dacl_post_mixer_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		dacl_post_mixer_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		dacl_post_mixer_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

void dacr_post_mixer_enable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		BIT(CLK_DACR_EN_OFFSET), BIT(CLK_DACR_EN_OFFSET));
}

void dacr_post_mixer_disable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		BIT(CLK_DACR_EN_OFFSET), 0);
}

static int dacr_post_mixer_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		dacr_post_mixer_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		dacr_post_mixer_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static int dacsl_mixer_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* filter clk */
		da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
			BIT(CLK_DACSL_EN_OFFSET), BIT(CLK_DACSL_EN_OFFSET));
		/* hbf1i_bypass_en */
		da_combine_update_bits(codec, DACSL_CTRL_REG,
			BIT(DACSL_HBF1I_BYPASS_EN_OFFSET),
			BIT(DACSL_HBF1I_BYPASS_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DACSL_CTRL_REG,
			BIT(DACSL_HBF1I_BYPASS_EN_OFFSET), 0);
		da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
			BIT(CLK_DACSL_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int dacsr_mixer_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		break;
	case SND_SOC_DAPM_POST_PMD:
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

#define MIXER_WIDGET \
	SND_SOC_DAPM_MIXER("MAD_PGA_MIXER", \
		SND_SOC_NOPM, 0, 0, \
		mad_pga_mixer_controls, \
		ARRAY_SIZE(mad_pga_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DSDL_MIXER", \
		SND_SOC_NOPM, 0, 0, \
		dsdl_mixer_controls, \
		ARRAY_SIZE(dsdl_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DSDR_MIXER", \
		SND_SOC_NOPM, 0, 0, \
		dsdr_mixer_controls, \
		ARRAY_SIZE(dsdr_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACL_MIXER", \
		DAC_DP_CLK_EN_1_REG, DACL_MIXER4_CLK_EN_OFFSET, 0, \
		dacl_mixer_controls, \
		ARRAY_SIZE(dacl_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACR_MIXER", \
		DAC_DP_CLK_EN_1_REG, DACR_MIXER4_CLK_EN_OFFSET, 0, \
		dacr_mixer_controls, \
		ARRAY_SIZE(dacr_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACL_PRE_MIXER", \
		DAC_MIXER_CLK_EN_REG, DACL_PRE_MIXER2_CLK_EN_OFFSET, 0, \
		dacl_pre_mixer_controls, \
		ARRAY_SIZE(dacl_pre_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACR_PRE_MIXER", \
		DAC_MIXER_CLK_EN_REG, DACR_PRE_MIXER2_CLK_EN_OFFSET, 0, \
		dacr_pre_mixer_controls, \
		ARRAY_SIZE(dacr_pre_mixer_controls)), \
	SND_SOC_DAPM_MIXER_E("DACL_POST_MIXER", \
		DAC_MIXER_CLK_EN_REG, DACL_POST_MIXER2_CLK_EN_OFFSET, 0, \
		dacl_post_mixer_controls, \
		ARRAY_SIZE(dacl_post_mixer_controls), \
		dacl_post_mixer_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MIXER_E("DACR_POST_MIXER", \
		DAC_MIXER_CLK_EN_REG, DACR_POST_MIXER2_CLK_EN_OFFSET, 0, \
		dacr_post_mixer_controls, \
		ARRAY_SIZE(dacr_post_mixer_controls), \
		dacr_post_mixer_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MIXER_E("DACSL_MIXER", \
		DAC_DP_CLK_EN_1_REG, DACL_S_MIXER4_CLK_EN_OFFSET, 0, \
		dacsl_mixer_controls, \
		ARRAY_SIZE(dacsl_mixer_controls), \
		dacsl_mixer_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MIXER_E("DACSR_MIXER", \
		DAC_DP_CLK_EN_1_REG, DACR_S_MIXER4_CLK_EN_OFFSET, 0, \
		dacsr_mixer_controls, \
		ARRAY_SIZE(dacsr_mixer_controls), \
		dacsr_mixer_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

static const char * const hsmic_pga_mux_texts[] = {
	"RESERVE0",
	"HSMIC",
	"HSMIC_CPLS",
	"RESERVE3",
	"MAINMIC",
	"RESERVE5",
	"RESERVE6",
	"RESERVE7",
	"MAINMIC_CPLS",
};

static const struct soc_enum hsmic_pga_mux_enum =
	SOC_ENUM_SINGLE(ANA_HSMIC_MUX_AND_PGA, ANA_HSMIC_MUX_BIT,
		ARRAY_SIZE(hsmic_pga_mux_texts), hsmic_pga_mux_texts);

static const struct snd_kcontrol_new dapm_hsmic_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", hsmic_pga_mux_enum);

static const char * const auxmic_pga_mux_texts[] = {
	"RESERVE0",
	"AUXMIC",
	"AUXMIC_CPLS",
	"RESERVE3",
	"MAINMIC",
	"RESERVE5",
	"RESERVE6",
	"RESERVE7",
	"MAINMIC_CPLS",
};

static const struct soc_enum auxmic_pga_mux_enum =
	SOC_ENUM_SINGLE(ANA_AUXMIC_MUX_AND_PGA, ANA_AUXMIC_MUX_BIT,
		ARRAY_SIZE(auxmic_pga_mux_texts), auxmic_pga_mux_texts);

static const struct snd_kcontrol_new dapm_auxmic_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", auxmic_pga_mux_enum);

static const char * const mic3_pga_texts[] = {
	"RESERVE0",
	"MIC3",
	"MIC3_CPLS",
	"RESERVE3",
	"MIC4",
	"RESERVE5",
	"RESERVE6",
	"RESERVE7",
	"MIC4_CPLS",
};

static const char * const mic4_pga_texts[] = {
	"RESERVE0",
	"MIC4",
	"MIC4_CPLS",
	"RESERVE3",
	"MIC3",
	"RESERVE5",
	"RESERVE6",
	"RESERVE7",
	"MIC3_CPLS",
};

static const struct soc_enum mic3_pga_mux_enum =
	SOC_ENUM_SINGLE(ANA_MIC3_MUX_AND_PGA, ANA_MIC3_MUX_BIT,
		ARRAY_SIZE(mic3_pga_texts), mic3_pga_texts);

static const struct snd_kcontrol_new dapm_mic3_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", mic3_pga_mux_enum);

static const struct soc_enum mic4_pga_mux_enum =
	SOC_ENUM_SINGLE(ANA_MIC4_MUX_AND_PGA, ANA_MIC4_MUX_BIT,
		ARRAY_SIZE(mic4_pga_texts), mic4_pga_texts);

static const struct snd_kcontrol_new dapm_mic4_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", mic4_pga_mux_enum);

static const char * const mic5_pga_texts[] = {
	"IR",
	"MIC5",
	"MIC5_CPLS",
};

static const struct soc_enum mic5_pga_mux_enum =
	SOC_ENUM_SINGLE(ANA_MIC5_MUX_AND_PGA, ANA_MIC5_MUX_BIT,
		ARRAY_SIZE(mic5_pga_texts), mic5_pga_texts);

static const struct snd_kcontrol_new dapm_mic5_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", mic5_pga_mux_enum);

static const char * const adc_mux_texts[] = {
	"ADC0",
	"ADC1",
	"ADC2",
	"ADC3",
	"ADC4",
	"DMIC0L",
	"DMIC0R",
	"DMIC1L",
	"DMIC1R",
	"ADC_MAD",
};

static const struct soc_enum adcl0_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL15_REG, ADC0L_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc_mux_texts), adc_mux_texts);

static const struct snd_kcontrol_new dapm_adcl0_mux_controls =
	SOC_DAPM_ENUM("Mux", adcl0_mux_enum);

static const struct soc_enum adcr0_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL15_REG, ADC0R_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc_mux_texts), adc_mux_texts);

static const struct snd_kcontrol_new dapm_adcr0_mux_controls =
	SOC_DAPM_ENUM("Mux", adcr0_mux_enum);

static const struct soc_enum adcl1_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL16_REG, ADC1L_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc_mux_texts), adc_mux_texts);

static const struct snd_kcontrol_new dapm_adcl1_mux_controls =
	SOC_DAPM_ENUM("Mux", adcl1_mux_enum);

static const struct soc_enum adcr1_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL16_REG, ADC1R_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc_mux_texts), adc_mux_texts);

static const struct snd_kcontrol_new dapm_adcr1_mux_controls =
	SOC_DAPM_ENUM("Mux", adcr1_mux_enum);

static const struct soc_enum adcl2_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL17_REG, ADC2L_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc_mux_texts), adc_mux_texts);

static const struct snd_kcontrol_new dapm_adcl2_mux_controls =
	SOC_DAPM_ENUM("Mux", adcl2_mux_enum);

static const char * const mic1_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"DACL_48",
	"S2_L",
	"S3_L",
	"S4_L",
	"ADCR0",
	"ADCR1",
	"ADCL2",
};

static const struct soc_enum mic1_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL29_REG, MIC_1_SEL_OFFSET,
		ARRAY_SIZE(mic1_mux_texts), mic1_mux_texts);

static const struct snd_kcontrol_new dapm_mic1_mux_controls =
	SOC_DAPM_ENUM("Mux", mic1_mux_enum);

static const char * const mic2_mux_texts[] = {
	"ADCR0",
	"ADCR1",
	"DACR_48",
	"S2_R",
	"S3_R",
	"S4_R",
	"ADCL0",
	"ADCL1",
	"ADCL2",
};

static const struct soc_enum mic2_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL29_REG, MIC_2_SEL_OFFSET,
		ARRAY_SIZE(mic2_mux_texts), mic2_mux_texts);

static const struct snd_kcontrol_new dapm_mic2_mux_controls =
	SOC_DAPM_ENUM("Mux", mic2_mux_enum);

static const char * const mic3_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"DACL_48",
	"S1_L",
	"S2_L",
	"S4_L",
	"ADCR0",
	"ADCR1",
	"ADCL2",
};

static const struct soc_enum mic3_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL28_REG, MIC_3_SEL_OFFSET,
		ARRAY_SIZE(mic3_mux_texts), mic3_mux_texts);

static const struct snd_kcontrol_new dapm_mic3_mux_controls =
	SOC_DAPM_ENUM("Mux", mic3_mux_enum);

static const char * const mic4_mux_texts[] = {
	"ADCR0",
	"ADCR1",
	"DACR_48",
	"S1_R",
	"S2_R",
	"S4_R",
	"ADCL0",
	"ADCL1",
	"ADCL2",
};

static const struct soc_enum mic4_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL28_REG, MIC_4_SEL_OFFSET,
		ARRAY_SIZE(mic4_mux_texts), mic4_mux_texts);

static const struct snd_kcontrol_new dapm_mic4_mux_controls =
	SOC_DAPM_ENUM("Mux", mic4_mux_enum);

static const char * const mic5_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"ADCR0",
	"ADCR1",
	"ADCL2",
	"DACL_S",
	"DACR_S",
};

static const struct soc_enum mic5_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL27_REG, MIC_5_SEL_OFFSET,
		ARRAY_SIZE(mic5_mux_texts), mic5_mux_texts);

static const struct snd_kcontrol_new dapm_mic5_mux_controls =
	SOC_DAPM_ENUM("Mux", mic5_mux_enum);

static const char * const sidetone_mux_texts[] = {
	"S1_L",
	"S2_L",
	"ADCL0",
	"ADCR0",
	"ADCL1",
	"ADCR1",
	"ADCL2"
};

static const struct soc_enum sidetone_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL14_REG, SIDE_SRC_DIN_SEL_OFFSET,
		ARRAY_SIZE(sidetone_mux_texts), sidetone_mux_texts);

static const struct snd_kcontrol_new dapm_sidetone_mux_controls =
	SOC_DAPM_ENUM("Mux", sidetone_mux_enum);

static const char * const m1_l_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"DACL_S",
	"DACL_48",
	"S1_L",
	"S2_L",
	"S3_L",
	"ADCL2",
};

static const struct soc_enum m1_l_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL32_REG, M1L_DATA_SEL_OFFSET,
		ARRAY_SIZE(m1_l_mux_texts), m1_l_mux_texts);

static const struct snd_kcontrol_new dapm_m1_l_mux_controls =
	SOC_DAPM_ENUM("Mux", m1_l_mux_enum);

static const char * const m1_r_mux_texts[] = {
	"ADCR0",
	"ADCR1",
	"DACR_S",
	"DACR_48",
	"S1_R",
	"S2_R",
	"S3_R",
	"ADCL2",
};

static const struct soc_enum m1_r_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL32_REG, M1R_DATA_SEL_OFFSET,
		ARRAY_SIZE(m1_r_mux_texts), m1_r_mux_texts);

static const struct snd_kcontrol_new dapm_m1_r_mux_controls =
	SOC_DAPM_ENUM("Mux", m1_r_mux_enum);

static const char * const bt_l_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"DACL_48",
	"S1_L",
	"S3_L",
	"S4_L",
	"ADCL2",
};

static const struct soc_enum bt_l_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL31_REG, BTL_DATA_SEL_OFFSET,
		ARRAY_SIZE(bt_l_mux_texts), bt_l_mux_texts);

static const struct snd_kcontrol_new dapm_bt_l_mux_controls =
	SOC_DAPM_ENUM("Mux", bt_l_mux_enum);

static const char * const bt_r_mux_texts[] = {
	"ADCR0",
	"ADCR1",
	"DACR_48",
	"S1_R",
	"S3_R",
	"S4_R",
	"ADCL2",
};

static const struct soc_enum bt_r_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL31_REG, BTR_DATA_SEL_OFFSET,
		ARRAY_SIZE(bt_r_mux_texts), bt_r_mux_texts);

static const struct snd_kcontrol_new dapm_bt_r_mux_controls =
	SOC_DAPM_ENUM("Mux", bt_r_mux_enum);

static const char * const us_r1_mux_texts[] = {
	"ADCL0",
	"ADCR0",
	"ADCL1",
	"ADCR1",
	"ADCL2",
	"DACL_S",
	"DACR_S",
};

static const struct soc_enum us_r1_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL30_REG, ULTRASONIC_RX_MUX1_SEL_OFFSET,
		ARRAY_SIZE(us_r1_mux_texts), us_r1_mux_texts);

static const struct snd_kcontrol_new dapm_us_r1_mux_controls =
	SOC_DAPM_ENUM("Mux", us_r1_mux_enum);

static const char * const us_r2_mux_texts[] = {
	"ADCL0",
	"ADCR0",
	"ADCL1",
	"ADCR1",
	"ADCL2",
	"DACL_S",
	"DACR_S",
};

static const struct soc_enum us_r2_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL30_REG, ULTRASONIC_RX_MUX2_SEL_OFFSET,
		ARRAY_SIZE(us_r2_mux_texts), us_r2_mux_texts);

static const struct snd_kcontrol_new dapm_us_r2_mux_controls =
	SOC_DAPM_ENUM("Mux", us_r2_mux_enum);

/* S1_IL_MUX */
static const char * const dapm_s1_il_mux_texts[] = {
	"S1_INL",
	"D1",
};
static const struct soc_enum dapm_s1_il_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL0_REG, S1_IL_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_s1_il_mux_texts),
		dapm_s1_il_mux_texts);
static const struct snd_kcontrol_new dapm_s1_il_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_s1_il_mux_enum);

/* S1_IR_MUX */
static const char * const dapm_s1_ir_mux_texts[] = {
	"S1_INR",
	"D2",
};
static const struct soc_enum dapm_s1_ir_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL0_REG, S1_IR_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_s1_ir_mux_texts),
		dapm_s1_ir_mux_texts);
static const struct snd_kcontrol_new dapm_s1_ir_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_s1_ir_mux_enum);

/* S2_IL_MUX */
static const char * const dapm_s2_il_mux_texts[] = {
	"S2_INL",
	"D3",
};
static const struct soc_enum dapm_s2_il_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL0_REG, S2_IL_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_s2_il_mux_texts),
		dapm_s2_il_mux_texts);
static const struct snd_kcontrol_new dapm_s2_il_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_s2_il_mux_enum);

/* S2_IR_MUX */
static const char * const dapm_s2_ir_mux_texts[] = {
	"S2_INR",
	"D4",
};
static const struct soc_enum dapm_s2_ir_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL0_REG, S2_IR_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_s2_ir_mux_texts),
		dapm_s2_ir_mux_texts);
static const struct snd_kcontrol_new dapm_s2_ir_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_s2_ir_mux_enum);

static const char * const dapm_dsp_if8_mux_texts[] = {
	"DAC_MIXER",
	"ADC",
	"HP_SDM_L",
	"EP_SDM_L",
	"CLASSH_DEBUG",
	"DSDL",
};
static const struct soc_enum dapm_dsp_if8_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL11_REG, DSPIF8_TEST_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_dsp_if8_mux_texts), dapm_dsp_if8_mux_texts);
static const struct snd_kcontrol_new dapm_dsp_if8_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_dsp_if8_mux_enum);

static const char * const dapm_dacl_pre_mux_texts[] = {
	"DSP_IF8_OL",
	"DACL_MIXER",
};
static const struct soc_enum dapm_dacl_pre_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL9_REG, DACL_PRE_MIXER2_IN1_SEL_OFFSET,
		ARRAY_SIZE(dapm_dacl_pre_mux_texts), dapm_dacl_pre_mux_texts);
static const struct snd_kcontrol_new dapm_dacl_pre_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_dacl_pre_mux_enum);

static const char * const dapm_dacr_pre_mux_texts[] = {
	"DSP_IF8_OR",
	"DACR_MIXER",
};
static const struct soc_enum dapm_dacr_pre_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL9_REG, DACR_PRE_MIXER2_IN1_SEL_OFFSET,
		ARRAY_SIZE(dapm_dacr_pre_mux_texts), dapm_dacr_pre_mux_texts);
static const struct snd_kcontrol_new dapm_dacr_pre_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_dacr_pre_mux_enum);

static const char * const dapm_dacl_pga_mux_texts[] = {
	"DACL_384K",
	"ADCL1",
	"HP_RES_DET",
	"S1_L",
};
static const struct soc_enum dapm_dacl_pga_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL12_REG, DACL_PRE_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_dacl_pga_mux_texts), dapm_dacl_pga_mux_texts);
static const struct snd_kcontrol_new dapm_dacl_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_dacl_pga_mux_enum);

static const char * const dapm_dacr_pga_mux_texts[] = {
	"DACR_384K",
	"ADCR1",
	"HP_RES_DET",
	"S1_R",
};
static const struct soc_enum dapm_dacr_pga_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL12_REG, DACR_PRE_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_dacr_pga_mux_texts), dapm_dacr_pga_mux_texts);
static const struct snd_kcontrol_new dapm_dacr_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_dacr_pga_mux_enum);

static const char * const dapm_hp_l_sdm_mux_texts[] = {
	"DACL_UP16",
	"DSD_L",
	"ANC_L",
	"IO_TEST",
};
static const struct soc_enum dapm_hp_l_sdm_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL13_REG, HP_SDM_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_hp_l_sdm_mux_texts), dapm_hp_l_sdm_mux_texts);
static const struct snd_kcontrol_new dapm_hp_l_sdm_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_hp_l_sdm_mux_enum);

static const char * const dapm_hp_r_sdm_mux_texts[] = {
	"DACR_UP16",
	"DSD_R",
	"ANC_R",
	"IO_TEST",
};
static const struct soc_enum dapm_hp_r_sdm_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL13_REG, HP_SDM_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_hp_r_sdm_mux_texts), dapm_hp_r_sdm_mux_texts);
static const struct snd_kcontrol_new dapm_hp_r_sdm_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_hp_r_sdm_mux_enum);

static const char * const dapm_ep_l_sdm_mux_texts[] = {
	"DACSL_UP16",
	"ANC_L",
	"ANC_R",
	"IO_TEST",
};
static const struct soc_enum dapm_ep_l_sdm_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL13_REG, EP_SDM_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_ep_l_sdm_mux_texts), dapm_ep_l_sdm_mux_texts);
static const struct snd_kcontrol_new dapm_ep_l_sdm_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_ep_l_sdm_mux_enum);

static const char * const dapm_dacsl_pag_mux_texts[] = {
	"DACSL_384K",
	"HP_RES_DET",
};
static const struct soc_enum dapm_dacsl_pag_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL38_REG, DACSL_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(dapm_dacsl_pag_mux_texts), dapm_dacsl_pag_mux_texts);
static const struct snd_kcontrol_new dapm_dacsl_pag_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_dacsl_pag_mux_enum);

static const char * const dapm_hp_dac_l_mux_texts[] = {
	"HP_L_SDM",
	"HP_R_SDM",
	"HP_ICELL_CAB",
	"HP_SDM_MUX",
	"DSPIF8",
};

static const struct soc_enum dapm_hp_dac_l_mux_enum =
	SOC_ENUM_SINGLE(DAC_DATA_SEL_CTRL0_REG, DAC0_DATA_SEL_OFFSET,
		ARRAY_SIZE(dapm_hp_dac_l_mux_texts), dapm_hp_dac_l_mux_texts);
static const struct snd_kcontrol_new dapm_hp_dac_l_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_hp_dac_l_mux_enum);

static const char * const dapm_hp_dac_r_mux_texts[] = {
	"HP_R_SDM",
	"HP_L_SDM",
	"HP_ICELL_CAB",
	"HP_SDM_MUX",
	"DSPIF8",
};

static const struct soc_enum dapm_hp_dac_r_mux_enum =
	SOC_ENUM_SINGLE(DAC_DATA_SEL_CTRL0_REG, DAC1_DATA_SEL_OFFSET,
		ARRAY_SIZE(dapm_hp_dac_r_mux_texts), dapm_hp_dac_r_mux_texts);
static const struct snd_kcontrol_new dapm_hp_dac_r_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_hp_dac_r_mux_enum);

static const char * const dapm_ep_dac_mux_texts[] = {
	"EP_L_SDM",
	"AP_ICELL_CAB",
	"EP_SDM_MUX",
	"DSPIF8",
};

static const struct soc_enum dapm_ep_dac_mux_enum =
	SOC_ENUM_SINGLE(DAC_DATA_SEL_CTRL1_REG, DAC2_DATA_SEL_OFFSET,
		ARRAY_SIZE(dapm_ep_dac_mux_texts), dapm_ep_dac_mux_texts);
static const struct snd_kcontrol_new dapm_ep_dac_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_ep_dac_mux_enum);

static const char * const dapm_ir_mux_texts[] = {
	"DSPIF8",
	"IR_REG_CTRL",
	"IO_TEST_IN",
	"OTHERS",
};

static const struct soc_enum dapm_ir_mux_enum =
	SOC_ENUM_SINGLE(IR_SOURCE_CTRL_REG, IR_SOURCE_SEL_OFFSET,
		ARRAY_SIZE(dapm_ir_mux_texts), dapm_ir_mux_texts);
static const struct snd_kcontrol_new dapm_ir_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_ir_mux_enum);

static const char * const dapm_mdm_mux_texts[] = {
	"S3_L",
	"S3_R",
};

static const struct soc_enum dapm_mdm_mux_enum =
	SOC_ENUM_SINGLE(SC_CODEC_MUX_CTRL14_REG, MDM_SEL_OFFSET,
		ARRAY_SIZE(dapm_mdm_mux_texts), dapm_mdm_mux_texts);

static const struct snd_kcontrol_new dapm_mdm_mux_controls =
	SOC_DAPM_ENUM("Mux", dapm_mdm_mux_enum);

/* virtual mux for S3_L*/
static const char * const s3_l_mux_texts[] = {
	"NORMAL",
	"ULTRASONIC",
	"S3_IL_PGA",
};

static const struct soc_enum s3_l_mux_enum =
	SOC_ENUM_SINGLE(VIRTUAL_EXTERN_REG, ULTRASONIC_DOWN_OFFSET,
		ARRAY_SIZE(s3_l_mux_texts), s3_l_mux_texts);

static const struct snd_kcontrol_new dapm_s3_l_mux_controls =
	SOC_DAPM_ENUM("Mux", s3_l_mux_enum);

static const struct reg_seq_config dacl_pga_mux_up_regs[] = {
	/* pga */
	{ { MISC_CLK_EN_REG, BIT(DACL_PRE_PGA_CLK_EN_OFFSET),
		BIT(DACL_PRE_PGA_CLK_EN_OFFSET), true }, 0, 0 },
	{ { MISC_CLK_EN_REG, BIT(DACL_POST_PGA_CLK_EN_OFFSET),
		BIT(DACL_POST_PGA_CLK_EN_OFFSET), true }, 0, 0 },
	/* src */
	{ { S1_MIXER_EQ_CLK_EN_REG, BIT(CLK_DACL_UP16_EN_OFFSET),
		BIT(CLK_DACL_UP16_EN_OFFSET), true }, 0, 0 },
	/* sdm valid */
	{ { HP_SDM_L_CTRL0_REG, BIT(HP_SDM_L_DEM_DSEN_OFFSET),
		BIT(HP_SDM_L_DEM_DSEN_OFFSET), true }, 0, 0 },
	{ { HP_SDM_L_CTRL7_REG, HP_SDM_L_UP16_DELAY_MASK,
		HP_SDM_L_UP16_DELAY_VAULE, true }, 0, 0 },
	{ { HP_SDM_L_CTRL8_REG, HP_SDM_L_DELAY_MASK,
		HP_SDM_L_DELAY_VALUE, true }, 0, 0 },
	{ { HP_SDM_L_CTRL9_REG,
		MASK_ON_BIT(HP_SDM_L_CELLSEL_MODE_LEN, HP_SDM_L_CELLSEL_MODE_OFFSET),
		HP_SDM_L_CELLSEL_MODE_2 << HP_SDM_L_CELLSEL_MODE_OFFSET, true }, 0, 0 },
	{ { HP_SDM_L_CTRL12_REG, BIT(HP_SDM_L_CALT_VLD_OFFSET),
		BIT(HP_SDM_L_CALT_VLD_OFFSET), true }, 0, 0 },
	/* dacl dc */
	{ { DACL_DC_M_REG, DACL_DC_M_REG_MASK, DACL_DC_M_REG_VALUE, true }, 0, 0 },
};

static const struct reg_seq_config dacl_pga_mux_dn_regs[] = {
	/* dacl dc */
	{ { DACL_DC_M_REG, DACL_DC_M_REG_MASK, 0, true }, 0, 0 },
	/* sdm default */
	{ { HP_SDM_L_CTRL12_REG, BIT(HP_SDM_L_CALT_VLD_OFFSET), 0, true }, 0, 0 },
	{ { HP_SDM_L_CTRL9_REG,
		MASK_ON_BIT(HP_SDM_L_CELLSEL_MODE_LEN, HP_SDM_L_CELLSEL_MODE_OFFSET),
		0, true }, 0, 0 },
	{ { HP_SDM_L_CTRL8_REG, HP_SDM_L_DELAY_MASK, 0, true }, 0, 0 },
	{ { HP_SDM_L_CTRL7_REG, HP_SDM_L_UP16_DELAY_MASK, 0, true }, 0, 0 },
	{ { HP_SDM_L_CTRL0_REG,
		BIT(HP_SDM_L_DEM_DSEN_OFFSET), BIT(HP_SDM_L_DEM_DSEN_OFFSET),
		true }, 0, 0 },
	/* src */
	{ { S1_MIXER_EQ_CLK_EN_REG, BIT(CLK_DACL_UP16_EN_OFFSET), 0, true }, 0, 0 },
	/* pga */
	{ { MISC_CLK_EN_REG, BIT(DACL_POST_PGA_CLK_EN_OFFSET), 0, true }, 0, 0 },
	{ { MISC_CLK_EN_REG, BIT(DACL_PRE_PGA_CLK_EN_OFFSET), 0, true }, 0, 0 },
};

static const struct reg_seq_config dacr_pga_mux_up_regs[] = {
	/* pga */
	{ { MISC_CLK_EN_REG, BIT(DACR_PRE_PGA_CLK_EN_OFFSET),
		BIT(DACR_PRE_PGA_CLK_EN_OFFSET), true }, 0, 0 },
	{ { MISC_CLK_EN_REG, BIT(DACR_POST_PGA_CLK_EN_OFFSET),
		BIT(DACR_POST_PGA_CLK_EN_OFFSET), true }, 0, 0 },
	/* src */
	{ { S1_MIXER_EQ_CLK_EN_REG, BIT(CLK_DACR_UP16_EN_OFFSET),
		BIT(CLK_DACR_UP16_EN_OFFSET), true }, 0, 0 },
	/* sdm */
	{ { HP_SDM_R_CTRL0_REG, BIT(HP_SDM_R_DEM_DSEN_OFFSET),
		BIT(HP_SDM_R_DEM_DSEN_OFFSET), true }, 0, 0 },
	{ { HP_SDM_R_CTRL7_REG, HP_SDM_R_UP16_DELAY_MASK,
		HP_SDM_R_UP16_DELAY_VAULE, true }, 0, 0 },
	{ { HP_SDM_R_CTRL8_REG, HP_SDM_R_DELAY_MASK, HP_SDM_R_DELAY_VALUE, true }, 0, 0 },
	{ { HP_SDM_R_CTRL9_REG,
		MASK_ON_BIT(HP_SDM_R_CELLSEL_MODE_LEN, HP_SDM_R_CELLSEL_MODE_OFFSET),
		HP_SDM_R_CELLSEL_MODE_2 << HP_SDM_R_CELLSEL_MODE_OFFSET, true }, 0, 0 },
	{ { HP_SDM_R_CTRL12_REG, BIT(HP_SDM_R_CALT_VLD_OFFSET),
		BIT(HP_SDM_R_CALT_VLD_OFFSET), true }, 0, 0 },
	/* dacr dc */
	{ { DACR_DC_M_REG, DACR_DC_M_REG_MASK, DACR_DC_M_REG_VALUE, true }, 0, 0 },
};

static const struct reg_seq_config dacr_pga_mux_dn_regs[] = {
	{ { DACR_DC_M_REG, DACR_DC_M_REG_MASK, 0, true }, 0, 0 },
	/* sdm */
	{ { HP_SDM_R_CTRL12_REG, BIT(HP_SDM_R_CALT_VLD_OFFSET), 0, true }, 0, 0 },
	{ { HP_SDM_R_CTRL9_REG,
		MASK_ON_BIT(HP_SDM_R_CELLSEL_MODE_LEN, HP_SDM_R_CELLSEL_MODE_OFFSET), 0, true }, 0, 0 },
	{ { HP_SDM_R_CTRL8_REG, HP_SDM_R_DELAY_MASK, 0, true }, 0, 0 },
	{ { HP_SDM_R_CTRL7_REG, HP_SDM_R_UP16_DELAY_MASK, 0, true }, 0, 0 },
	{ { HP_SDM_R_CTRL0_REG, BIT(HP_SDM_R_DEM_DSEN_OFFSET),
		BIT(HP_SDM_R_DEM_DSEN_OFFSET), true }, 0, 0 },
	/* src */
	{ { S1_MIXER_EQ_CLK_EN_REG, BIT(CLK_DACR_UP16_EN_OFFSET), 0, true }, 0, 0 },
	/* pga */
	{ { MISC_CLK_EN_REG, BIT(DACR_POST_PGA_CLK_EN_OFFSET), 0, true }, 0, 0 },
	{ { MISC_CLK_EN_REG, BIT(DACR_PRE_PGA_CLK_EN_OFFSET), 0, true }, 0, 0 },
};


void dacl_pga_mux_enable(struct snd_soc_component *codec)
{
	write_reg_seq_array(codec, dacl_pga_mux_up_regs,
		ARRAY_SIZE(dacl_pga_mux_up_regs));
}

void dacl_pga_mux_disable(struct snd_soc_component *codec)
{
	write_reg_seq_array(codec, dacl_pga_mux_dn_regs,
		ARRAY_SIZE(dacl_pga_mux_dn_regs));
}

static int dacl_pga_mux_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		dacl_pga_mux_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		dacl_pga_mux_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

void dacr_pga_mux_enable(struct snd_soc_component *codec)
{
	write_reg_seq_array(codec, dacr_pga_mux_up_regs,
		ARRAY_SIZE(dacr_pga_mux_up_regs));
}

void dacr_pga_mux_disable(struct snd_soc_component *codec)
{
	write_reg_seq_array(codec, dacr_pga_mux_dn_regs,
		ARRAY_SIZE(dacr_pga_mux_dn_regs));
}

static int dacr_pga_mux_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		dacr_pga_mux_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		dacr_pga_mux_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static int dacsl_pga_mux_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* pga */
		da_combine_update_bits(codec, DMIC_CLK_EN_REG,
			BIT(DACL_S_PGA_CLK_EN_OFFSET),
			BIT(DACL_S_PGA_CLK_EN_OFFSET));
		da_combine_update_bits(codec, DACSL_PGA_CTRL0_REG,
			BIT(DACSL_PGA_BYPASS_OFFSET),
			BIT(DACSL_PGA_BYPASS_OFFSET));
		/* src */
		da_combine_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			BIT(CLK_DACL_S_UP16_EN_OFFSET),
			BIT(CLK_DACL_S_UP16_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			BIT(CLK_DACL_S_UP16_EN_OFFSET), 0);
		da_combine_update_bits(codec, DACSL_PGA_CTRL0_REG,
			BIT(DACSL_PGA_BYPASS_OFFSET), 0);
		da_combine_update_bits(codec, DMIC_CLK_EN_REG,
			BIT(DACL_S_PGA_CLK_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int ep_l_sdm_mux_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* sdm CAL en */
		da_combine_update_bits(codec, EP_SDM_L_CTRL1_REG,
			MASK_ON_BIT(EP_SDM_L_DEMMD_SEL_LEN, EP_SDM_L_DEMMD_SEL_OFFSET),
			EP_SDM_L_DEMMD_SEL_CAL << EP_SDM_L_DEMMD_SEL_OFFSET);
		da_combine_update_bits(codec, EP_SDM_L_CTRL1_REG,
			MASK_ON_BIT(EP_SDM_L_DEMHI_SEL_LEN, EP_SDM_L_DEMHI_SEL_OFFSET),
			EP_SDM_L_DEMHI_SEL_CAL << EP_SDM_L_DEMHI_SEL_OFFSET);

		da_combine_update_bits(codec, EP_SDM_L_CTRL2_REG,
			MASK_ON_BIT(EP_SDM_L_DEMLO_SEL_LEN, EP_SDM_L_DEMLO_SEL_OFFSET),
			EP_SDM_L_DEMHO_SEL_CAL << EP_SDM_L_DEMLO_SEL_OFFSET);
		/* sdm valid */
		da_combine_update_bits(codec, EP_SDM_L_CTRL3_REG,
			BIT(EP_SDM_L_CALT_VLD_OFFSET),
			BIT(EP_SDM_L_CALT_VLD_OFFSET));

		/* bypass pga */
		da_combine_update_bits(codec, DACSL_MIXER4_PGA_CTRL0_REG,
			BIT(DACSL_MIXER4_PGA_BYPASS_OFFSET),
			BIT(DACSL_MIXER4_PGA_BYPASS_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DACSL_MIXER4_PGA_CTRL0_REG,
			BIT(DACSL_MIXER4_PGA_BYPASS_OFFSET), 0);

		da_combine_update_bits(codec, EP_SDM_L_CTRL3_REG,
			BIT(EP_SDM_L_CALT_VLD_OFFSET), 0);
		da_combine_update_bits(codec, EP_SDM_L_CTRL2_REG,
			MASK_ON_BIT(EP_SDM_L_DEMLO_SEL_LEN, EP_SDM_L_DEMLO_SEL_OFFSET),
			EP_SDM_L_DEMHO_SEL_DEFAULT << EP_SDM_L_DEMLO_SEL_OFFSET);

		da_combine_update_bits(codec, EP_SDM_L_CTRL1_REG,
			MASK_ON_BIT(EP_SDM_L_DEMHI_SEL_LEN, EP_SDM_L_DEMHI_SEL_OFFSET),
			EP_SDM_L_DEMHI_SEL_DEFAULT << EP_SDM_L_DEMHI_SEL_OFFSET);
		da_combine_update_bits(codec, EP_SDM_L_CTRL1_REG,
			MASK_ON_BIT(EP_SDM_L_DEMMD_SEL_LEN, EP_SDM_L_DEMMD_SEL_OFFSET),
			EP_SDM_L_DEMMD_SEL_DEFAULT << EP_SDM_L_DEMMD_SEL_OFFSET);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

#define MUX_PART1_WIDGET \
	SND_SOC_DAPM_MUX("HSMIC_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_hsmic_pga_mux_controls), \
	SND_SOC_DAPM_MUX("AUXMIC_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_auxmic_pga_mux_controls), \
	SND_SOC_DAPM_MUX("MIC3_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic3_pga_mux_controls), \
	SND_SOC_DAPM_MUX("MIC4_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic4_pga_mux_controls), \
	SND_SOC_DAPM_MUX("MIC5_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic5_pga_mux_controls), \
	SND_SOC_DAPM_MUX("ADCL2_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_adcl2_mux_controls), \
	SND_SOC_DAPM_MUX("MIC1_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic1_mux_controls), \
	SND_SOC_DAPM_MUX("MIC2_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic2_mux_controls), \
	SND_SOC_DAPM_MUX("MIC3_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic3_mux_controls), \
	SND_SOC_DAPM_MUX("MIC4_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic4_mux_controls), \
	SND_SOC_DAPM_MUX("MIC5_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic5_mux_controls), \
	SND_SOC_DAPM_MUX("SIDETONE_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_sidetone_mux_controls), \
	SND_SOC_DAPM_MUX("M1_L_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_m1_l_mux_controls), \
	SND_SOC_DAPM_MUX("M1_R_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_m1_r_mux_controls), \
	SND_SOC_DAPM_MUX("BT_L_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_bt_l_mux_controls), \
	SND_SOC_DAPM_MUX("BT_R_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_bt_r_mux_controls), \
	SND_SOC_DAPM_MUX("US_R1_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_us_r1_mux_controls), \
	SND_SOC_DAPM_MUX("US_R2_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_us_r2_mux_controls), \
	SND_SOC_DAPM_MUX("S2_IL_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s2_il_mux_controls), \
	SND_SOC_DAPM_MUX("S2_IR_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s2_ir_mux_controls), \
	SND_SOC_DAPM_MUX("DSP_IF8_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dsp_if8_mux_controls), \
	SND_SOC_DAPM_MUX("DACL_PRE_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacl_pre_mux_controls), \
	SND_SOC_DAPM_MUX("DACR_PRE_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacr_pre_mux_controls), \
	SND_SOC_DAPM_MUX_E("DACL_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacl_pga_mux_controls, \
		dacl_pga_mux_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MUX_E("DACR_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacr_pga_mux_controls, \
		dacr_pga_mux_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MUX("HP_L_SDM_MUX", \
		DAC_DP_CLK_EN_2_REG, HP_SDM_L_CLK_EN_OFFSET, 0, \
		&dapm_hp_l_sdm_mux_controls), \
	SND_SOC_DAPM_MUX("HP_R_SDM_MUX", \
		DAC_DP_CLK_EN_2_REG, HP_SDM_R_CLK_EN_OFFSET, 0, \
		&dapm_hp_r_sdm_mux_controls), \
	SND_SOC_DAPM_MUX_E("EP_L_SDM_MUX", \
		DAC_DP_CLK_EN_2_REG, EP_SDM_L_CLK_EN_OFFSET, 0, \
		&dapm_ep_l_sdm_mux_controls, \
		ep_l_sdm_mux_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MUX_E("DACSL_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacsl_pag_mux_controls, \
		dacsl_pga_mux_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MUX("HP_DAC_L_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_hp_dac_l_mux_controls), \
	SND_SOC_DAPM_MUX("HP_DAC_R_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_hp_dac_r_mux_controls), \
	SND_SOC_DAPM_MUX("EP_DAC_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_ep_dac_mux_controls), \
	SND_SOC_DAPM_MUX("IR_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_ir_mux_controls), \
	SND_SOC_DAPM_MUX("MDM_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mdm_mux_controls), \
	SND_SOC_DAPM_MUX("S3L_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s3_l_mux_controls), \

#define S1_MUX_WIDGET \
	SND_SOC_DAPM_MUX("S1_IL_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s1_il_mux_controls), \
	SND_SOC_DAPM_MUX("S1_IR_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s1_ir_mux_controls), \

#define ADC01_MUX_WIDGET \
	SND_SOC_DAPM_MUX("ADCL0_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_adcl0_mux_controls), \
	SND_SOC_DAPM_MUX("ADCR0_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_adcr0_mux_controls), \
	SND_SOC_DAPM_MUX("ADCL1_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_adcl1_mux_controls), \
	SND_SOC_DAPM_MUX("ADCR1_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_adcr1_mux_controls), \

static void classH_headphone_select(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	/* classH clk en */
	da_combine_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
		BIT(CLK_CLASSH_EN_OFFSET), BIT(CLK_CLASSH_EN_OFFSET));
	/* classA/B -> classH */
	da_combine_update_bits(codec, CLASSH_CTRL6_REG,
		BIT(CLASSH_EN_OFFSET), BIT(CLASSH_EN_OFFSET));

	/* set dacl/r_up16_din_sel=classh */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL33_REG,
		BIT(DACL_UP16_DIN_SEL_OFFSET) | BIT(DACR_UP16_DIN_SEL_OFFSET),
		BIT(DACL_UP16_DIN_SEL_OFFSET) | BIT(DACR_UP16_DIN_SEL_OFFSET));
	/* classh_hp_en */
	da_combine_update_bits(codec, CLASSH_CTRL1_REG,
		BIT(CLASSH_HP_EN_OFFSET), BIT(CLASSH_HP_EN_OFFSET));
	/* classh_ng_en */
	da_combine_update_bits(codec, CLASSH_CTRL8_REG,
		BIT(CLASSH_NG_EN_OFFSET), BIT(CLASSH_NG_EN_OFFSET));
	/* set dacl/r_up16_din_sel=classh */
	da_combine_update_bits(codec, CLASSH_CTRL8_REG,
		BIT(CLASSH_NG_EN_OFFSET), BIT(CLASSH_NG_EN_OFFSET));
	/* cp_pop_clk_pd */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_140,
		BIT(CODEC_ANA_CTCM_CHOP_BPS_OFFSET),
		BIT(CODEC_ANA_CTCM_CHOP_BPS_OFFSET));
	/* rst_clk_calib_pop_ng_lp */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_056,
		BIT(CODEC_ANA_RST_CLK_POP_OFFSET),
		BIT(CODEC_ANA_RST_CLK_POP_OFFSET));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_056,
		BIT(CODEC_ANA_RST_CLK_POP_OFFSET), 0);

	AUDIO_LOGD("end");
}

static void classAB_headphone_select(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	/* classH clk must en */
	da_combine_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
		BIT(CLK_CLASSH_EN_OFFSET), BIT(CLK_CLASSH_EN_OFFSET));
	/* classH -> classA/B */
	da_combine_update_bits(codec, CLASSH_CTRL6_REG,
		BIT(CLASSH_EN_OFFSET), 0x0 << CLASSH_EN_OFFSET);

	/* set dacl/r_up16_din_sel=pga */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL33_REG,
		BIT(DACL_UP16_DIN_SEL_OFFSET) | BIT(DACR_UP16_DIN_SEL_OFFSET),
		0);
	/* classh_hp_en */
	da_combine_update_bits(codec, CLASSH_CTRL1_REG,
		BIT(CLASSH_HP_EN_OFFSET), 0x0 << CLASSH_HP_EN_OFFSET);
	/* classh_ng_en */
	da_combine_update_bits(codec, CLASSH_CTRL8_REG,
		BIT(CLASSH_NG_EN_OFFSET), BIT(CLASSH_NG_EN_OFFSET));

	/* cp_pop_clk_pd */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_140,
		BIT(CODEC_ANA_CTCM_CHOP_BPS_OFFSET),
		BIT(CODEC_ANA_CTCM_CHOP_BPS_OFFSET));
	/* rst_clk_calib_pop_ng_lp */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_056,
		BIT(CODEC_ANA_RST_CLK_POP_OFFSET),
		BIT(CODEC_ANA_RST_CLK_POP_OFFSET));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_056,
		BIT(CODEC_ANA_RST_CLK_POP_OFFSET), 0);

	AUDIO_LOGD("end");
}

static void classH_earpiece_select(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	/* classH clk en */
	da_combine_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
		BIT(CLK_CLASSH_EN_OFFSET), BIT(CLK_CLASSH_EN_OFFSET));

	/* classh_ep_en */
	da_combine_update_bits(codec, CLASSH_CTRL1_REG,
		BIT(CLASSH_EP_EN_OFFSET), BIT(CLASSH_EP_EN_OFFSET));

	/* classA/B -> classH */
	da_combine_update_bits(codec, CLASSH_CTRL6_REG,
		BIT(CLASSH_EN_OFFSET), BIT(CLASSH_EN_OFFSET));

	/* dacls_up16_din_sel classh */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL33_REG,
		BIT(DACL_S_UP16_DIN_SEL_OFFSET),
		BIT(DACL_S_UP16_DIN_SEL_OFFSET));

	/* fall delay counter=16384 */
	da_combine_update_bits(codec, CLASSH_CTRL9_REG,
		MASK_ON_BIT(CLASSH_VTH_NG_CFG_LEN, CLASSH_VTH_NG_CFG_OFFSET),
		0);
	da_combine_update_bits(codec, CLASSH_CTRL9_REG,
		MASK_ON_BIT(CLASSH_DF_CFG_LEN, CLASSH_DF_CFG_OFFSET),
		CLASSH_DF_CFG_16384);

	AUDIO_LOGD("end");
}

static void classAB_earpiece_select(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	/* classH clk en */
	da_combine_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
		BIT(CLK_CLASSH_EN_OFFSET), BIT(CLK_CLASSH_EN_OFFSET));

	/* classh_ep_en */
	da_combine_update_bits(codec, CLASSH_CTRL1_REG,
		BIT(CLASSH_EP_EN_OFFSET), BIT(CLASSH_EP_EN_OFFSET));

	/* classH -> classAB */
	da_combine_update_bits(codec, CLASSH_CTRL6_REG,
		BIT(CLASSH_EN_OFFSET), 0);
	/* dacls_up16_din_sel classh */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL33_REG,
		BIT(DACL_S_UP16_DIN_SEL_OFFSET),
		0);

	/* fall delay counter=16384 */
	da_combine_update_bits(codec, CLASSH_CTRL9_REG,
		MASK_ON_BIT(CLASSH_VTH_NG_CFG_LEN, CLASSH_VTH_NG_CFG_OFFSET),
		0);
	da_combine_update_bits(codec, CLASSH_CTRL9_REG,
		MASK_ON_BIT(CLASSH_DF_CFG_LEN, CLASSH_DF_CFG_OFFSET),
		CLASSH_DF_CFG_16384);

	AUDIO_LOGD("end");
}

void da_combine_v5_set_classH_config(struct snd_soc_component *codec,
		enum classH_state classH_state_cfg)
{
	unsigned char state = (unsigned int)classH_state_cfg & 0x0F;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	AUDIO_LOGI("ClassH state is %d", state);

	if (state & HP_POWER_STATE) {
		if (state & HP_CLASSH_STATE)
			classH_headphone_select(codec);
		else
			classAB_headphone_select(codec);
	}

	if (state & RCV_POWER_STATE) {
		if (state & RCV_CLASSH_STATE)
			classH_earpiece_select(codec);
		else
			classAB_earpiece_select(codec);
	}
}

static const struct reg_seq_config headphone_pop_on_regs[] = {
	/* hp sdm clk dis */
	{ { DAC_DP_CLK_EN_2_REG, BIT(HP_SDM_L_CLK_EN_OFFSET) | BIT(HP_SDM_R_CLK_EN_OFFSET),
		0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_067, 0, 0x7A, false }, 0, 0 }, /* pop ramp */
	{ { CODEC_ANA_RWREG_141, 0x3 << CODEC_ANA_CLK_POP_SEL_OFFSET,
		0x2 << CODEC_ANA_CLK_POP_SEL_OFFSET, true }, 0, 0 }, /* tx_hp */
	{ { CODEC_ANA_RWREG_140, 0xc0, 0x80, true }, 0, 0 }, /* chop dis and cp_pop */
	{ { CODEC_ANA_RWREG_056, BIT(CODEC_ANA_RST_CLK_POP_OFFSET) |
		0x3 << CODEC_ANA_IBCT_HP_OFFSET | 0x3 << CODEC_ANA_IBCT_FB_OFFSET,
		BIT(CODEC_ANA_RST_CLK_POP_OFFSET) | 0x2 << CODEC_ANA_IBCT_HP_OFFSET |
		0x2 << CODEC_ANA_IBCT_FB_OFFSET, true }, 0, 0 }, /* tx_hp default */
	{ { CODEC_ANA_RWREG_056, BIT(CODEC_ANA_RST_CLK_POP_OFFSET), 0, true }, 0, 0 }, /* tx_hp pop work */
	{ { CODEC_ANA_RWREG_067, 0, 0x7A, false }, 0, 0 }, /* tx_hp default */
	{ { CODEC_ANA_RWREG_067, 0, 0x7A, false }, 0, 0 }, /* tx_hp default */
	{ { CODEC_ANA_RWREG_055, 0, 0xF, false }, 0, 0 }, /* TX_HP VBR_Gain:0db */

	{ { CODEC_ANA_RWREG_011, 0, 0x0, false }, 0, 0 }, /* CPGND */
	{ { CODEC_ANA_RWREG_057, 0, 0x1, false }, 0, 0 }, /* tx_hp hl default */
	{ { CODEC_ANA_RWREG_057, 0, 0x1, false }, 0, 0 }, /* tx_hp hl default */
	{ { CODEC_ANA_RWREG_058, 0, 0x10, false }, 0, 0 }, /* TX_HP_offset default */
	{ { CODEC_ANA_RWREG_058, 0, 0x18, false }, 0, 0 }, /* TX_HP_offset 768k */
	{ { CODEC_ANA_RWREG_058, 0, 0x18, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_058, 0, 0x8, false }, 0, 0 }, /* OFFSET calibration work */
	{ { CODEC_ANA_RWREG_058, 0, 0xE, false }, MDELAY, 10000 },

	{ { CODEC_ANA_RWREG_058, 0, 0x8, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_09, 0, 0x7F, false }, 0, 0 }, /* TX_HPL default */
	{ { CODEC_ANA_RWREG_010, 0, 0x3F, false }, 0, 0 }, /* TX_HPR */
	{ { CODEC_ANA_RWREG_09, 0, 0x5F, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_010, 0, 0x1F, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_066, 0, 0x89, false }, 0, 0 }, /* pop default */
	{ { CODEC_ANA_RWREG_067, 0, 0x7A, false }, 0, 0 }, /* pop ramp default */
	{ { CODEC_ANA_RWREG_066, 0, 0x89, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_066, 0, 0x9, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_066, 0, 0x1, false }, MDELAY, 20000 },

	{ { CODEC_ANA_RWREG_09, 0, 0x4B, false }, MDELAY, 10000 },
	{ { CODEC_ANA_RWREG_09, 0, 0x4F, false }, MDELAY, 20000 },
	{ { CODEC_ANA_RWREG_09, 0, 0x43, false }, RANGE_SLEEP, 200 },

	{ { CODEC_ANA_RWREG_010, 0, 0xB, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_010, 0, 0xF, false }, MDELAY, 20000 },
	{ { CODEC_ANA_RWREG_010, 0, 0x3, false }, MDELAY, 10000 },
	{ { CODEC_ANA_RWREG_067, 0, 0x1A, false }, 0, 0 },
	{ { DAC_DP_CLK_EN_2_REG, BIT(HP_SDM_L_CLK_EN_OFFSET) | BIT(HP_SDM_R_CLK_EN_OFFSET),
		BIT(HP_SDM_L_CLK_EN_OFFSET) | BIT(HP_SDM_R_CLK_EN_OFFSET), true }, 0, 0},
};

static const struct reg_seq_config headphone_pop_on_delay_regs[] = {
	/* hp sdm clk dis */
	{ { DAC_DP_CLK_EN_2_REG, BIT(HP_SDM_L_CLK_EN_OFFSET) | BIT(HP_SDM_R_CLK_EN_OFFSET),
		0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_067, 0, 0x7A, false }, 0, 0 }, /* pop ramp */
	{ { CODEC_ANA_RWREG_141, 0x3 << CODEC_ANA_CLK_POP_SEL_OFFSET,
		0x2 << CODEC_ANA_CLK_POP_SEL_OFFSET, true }, 0, 0 }, /* tx_hp */
	{ { CODEC_ANA_RWREG_140, 0xc0, 0x80, true }, 0, 0 }, /* chop dis and cp_pop */
	{ { CODEC_ANA_RWREG_056, BIT(CODEC_ANA_RST_CLK_POP_OFFSET) |
		0x3 << CODEC_ANA_IBCT_HP_OFFSET | 0x3 << CODEC_ANA_IBCT_FB_OFFSET,
		BIT(CODEC_ANA_RST_CLK_POP_OFFSET) | 0x2 << CODEC_ANA_IBCT_HP_OFFSET |
		0x2 << CODEC_ANA_IBCT_FB_OFFSET, true }, 0, 0 }, /* tx_hp default */
	{ { CODEC_ANA_RWREG_056, BIT(CODEC_ANA_RST_CLK_POP_OFFSET), 0, true }, 0, 0 }, /* tx_hp pop work */
	{ { CODEC_ANA_RWREG_067, 0, 0x7A, false }, 0, 0 }, /* tx_hp default */
	{ { CODEC_ANA_RWREG_067, 0, 0x7A, false }, 0, 0 }, /* tx_hp default */
	{ { CODEC_ANA_RWREG_055, 0, 0xF, false }, 0, 0 }, /* TX_HP VBR_Gain:0db */

	{ { CODEC_ANA_RWREG_011, 0, 0x0, false }, 0, 0 }, /* CPGND */
	{ { CODEC_ANA_RWREG_057, 0, 0x1, false }, 0, 0 }, /* tx_hp hl default */
	{ { CODEC_ANA_RWREG_057, 0, 0x1, false }, 0, 0 }, /* tx_hp hl default */
	{ { CODEC_ANA_RWREG_058, 0, 0x10, false }, 0, 0 }, /* TX_HP_offset default */
	{ { CODEC_ANA_RWREG_058, 0, 0x18, false }, 0, 0 }, /* TX_HP_offset 768k */
	{ { CODEC_ANA_RWREG_058, 0, 0x18, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_058, 0, 0x8, false }, 0, 0 }, /* OFFSET calibration work */
	{ { CODEC_ANA_RWREG_058, 0, 0xE, false }, MDELAY, 10000 }, /* delay 10ms */

	{ { CODEC_ANA_RWREG_058, 0, 0x8, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_09, 0, 0x7F, false }, 0, 0 }, /* TX_HPL default */
	{ { CODEC_ANA_RWREG_010, 0, 0x3F, false }, 0, 0 }, /* TX_HPR */
	{ { CODEC_ANA_RWREG_09, 0, 0x5F, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_010, 0, 0x1F, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_066, 0, 0x89, false }, 0, 0 }, /* pop default */
	{ { CODEC_ANA_RWREG_067, 0, 0x7A, false }, 0, 0 }, /* pop ramp default */
	{ { CODEC_ANA_RWREG_066, 0, 0x89, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_066, 0, 0x9, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_066, 0, 0x1, false }, MDELAY, 20000 }, /* delay 20ms */

	{ { CODEC_ANA_RWREG_09, 0, 0x4B, false }, MDELAY, 10000 }, /* delay 10ms */
	{ { CODEC_ANA_RWREG_09, 0, 0x4F, false }, MDELAY, 20000 }, /* delay 20ms */
	{ { CODEC_ANA_RWREG_09, 0, 0x4B, false }, RANGE_SLEEP, 200 }, /* delay 200us */

	{ { CODEC_ANA_RWREG_010, 0, 0xB, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_010, 0, 0xF, false }, MDELAY, 20000 }, /* delay 20ms */
	{ { CODEC_ANA_RWREG_010, 0, 0xB, false }, MDELAY, 10000 }, /* delay 10ms */
	{ { CODEC_ANA_RWREG_067, 0, 0x1A, false }, 0, 0 },
	{ { DAC_DP_CLK_EN_2_REG, BIT(HP_SDM_L_CLK_EN_OFFSET) | BIT(HP_SDM_R_CLK_EN_OFFSET),
		BIT(HP_SDM_L_CLK_EN_OFFSET) | BIT(HP_SDM_R_CLK_EN_OFFSET), true }, 0, 0},
};

void headphone_pop_on_delay_work(struct work_struct *work)
{
	struct da_combine_v5_platform_data *data = container_of(work,
		struct da_combine_v5_platform_data, headphone_pop_on_delay.work);
	struct reg_seq_config headphone_pop_on_amp[] = {
		{ { CODEC_ANA_RWREG_09, 0, 0x43, false }, RANGE_SLEEP, 200 }, /* delay 200us */
		{ { CODEC_ANA_RWREG_010, 0, 0x3, false }, RANGE_SLEEP, 200 }, /* delay 200us */
	};

	write_reg_seq_array(data->codec, headphone_pop_on_amp,
		ARRAY_SIZE(headphone_pop_on_amp));
	AUDIO_LOGI("%s: amp write success", __func__);
}

void da_combine_v5_headphone_pop_on(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *data = NULL;

	AUDIO_LOGD("begin");
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}
	data = snd_soc_component_get_drvdata(codec);

	if (data->board_config.headphone_pop_on_delay_enable)
		write_reg_seq_array(codec, headphone_pop_on_delay_regs,
			ARRAY_SIZE(headphone_pop_on_delay_regs));
	else
		write_reg_seq_array(codec, headphone_pop_on_regs,
			ARRAY_SIZE(headphone_pop_on_regs));

	AUDIO_LOGD("end");
}

static const struct reg_seq_config headphone_pop_off_regs[] = {
	{ { DAC_DP_CLK_EN_2_REG,
		BIT(HP_SDM_L_CLK_EN_OFFSET) | BIT(HP_SDM_R_CLK_EN_OFFSET),
		0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_067, 0, 0x7A, false }, 0, 0},
	{ { CODEC_ANA_RWREG_09, 0, 0x4B, false }, RANGE_SLEEP, 100 },
	{ { CODEC_ANA_RWREG_09, 0, 0x4F, false }, MDELAY, 10000 },
	{ { CODEC_ANA_RWREG_09, 0, 0x5F, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_010, 0, 0x0B, false }, RANGE_SLEEP, 100 },
	{ { CODEC_ANA_RWREG_010, 0, 0x0F, false }, MDELAY, 10000 },
	{ { CODEC_ANA_RWREG_010, 0, 0x1F, false }, RANGE_SLEEP, 100 },
	{ { CODEC_ANA_RWREG_09, 0, 0x7F, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_010, 0, 0x3F, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_010, 0, 0x7F, false }, 0, 0 },
};

void da_combine_v5_headphone_pop_off(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	write_reg_seq_array(codec, headphone_pop_off_regs,
		ARRAY_SIZE(headphone_pop_off_regs));

	AUDIO_LOGD("end");
}

static void headphone_power_param_pass_delay(struct da_combine_v5_platform_data *data,
	int event)
{
	int pop_on_delay_time = 25; /* delay 25ms */

	if (!data->board_config.headphone_pop_on_delay_enable)
		return;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		schedule_delayed_work(&(data->headphone_pop_on_delay),
			msecs_to_jiffies(pop_on_delay_time));
		AUDIO_LOGI("headphone pop on delay work time: %d", pop_on_delay_time);
		break;
	case SND_SOC_DAPM_POST_PMD:
		cancel_delayed_work(&(data->headphone_pop_on_delay));
		AUDIO_LOGI("headphone pop on delay work cancel");
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
}

static void headphone_power_param_pass_enable(struct snd_soc_component *codec, int hs_power_up_flag)
{
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	if (hs_power_up_flag == HEADSET_LEFT)
		data->hsl_power_on = true;
	if (hs_power_up_flag == HEADSET_RIGHT)
		data->hsr_power_on = true;

	if (data->hsl_power_on && data->hsr_power_on) {
		data->rcv_hp_classH_state =
			(unsigned int)data->rcv_hp_classH_state | HP_POWER_STATE;
		da_combine_v5_set_classH_config(codec,
			data->rcv_hp_classH_state);
		da_combine_v5_headphone_pop_on(codec);
		headphone_power_param_pass_delay(data, SND_SOC_DAPM_PRE_PMU);
	}

	AUDIO_LOGD("end");
}

static void headphone_power_param_pass_disable(struct snd_soc_component *codec, int hs_power_down_flag)
{
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	if (data->hsl_power_on && data->hsr_power_on) {
		headphone_power_param_pass_delay(data, SND_SOC_DAPM_POST_PMD);
		da_combine_v5_headphone_pop_off(codec);
		data->rcv_hp_classH_state =
			(unsigned int)data->rcv_hp_classH_state & (~HP_POWER_STATE);
		da_combine_v5_set_classH_config(codec,
			data->rcv_hp_classH_state);
	}

	if (hs_power_down_flag == HEADSET_LEFT)
		data->hsl_power_on = false;
	if (hs_power_down_flag == HEADSET_RIGHT)
		data->hsr_power_on = false;


	AUDIO_LOGD("end");
}


static int headphone_l_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		headphone_power_param_pass_enable(codec, HEADSET_LEFT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		headphone_power_param_pass_disable(codec, HEADSET_LEFT);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static int headphone_r_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		headphone_power_param_pass_enable(codec, HEADSET_RIGHT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		headphone_power_param_pass_disable(codec, HEADSET_RIGHT);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static int ir_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
			BIT(CODEC_PD_INF_LRN_OFFSET),
			BIT(CODEC_PD_INF_LRN_OFFSET));
		da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
			BIT(CODEC_PD_INF_EMS_OFFSET),
			0x0 << CODEC_PD_INF_EMS_OFFSET);
		da_combine_update_bits(codec, CODEC_ANA_RWREG_051,
			MASK_ON_BIT(CODEC_FIR_OUT_CTRL_LEN, CODEC_FIR_OUT_CTRL_OFFSET) |
			MASK_ON_BIT(CODEC_FIR_ATE_CTRL_LEN, CODEC_FIR_ATE_CTRL_OFFSET),
			CODEC_FIR_OUT_X15 << CODEC_FIR_OUT_CTRL_OFFSET |
			CODEC_FIR_ATE_XF << CODEC_FIR_ATE_CTRL_OFFSET);
		da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
			BIT(CODEC_LEAK_CTRL_OFFSET), BIT(CODEC_LEAK_CTRL_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, CODEC_ANA_RWREG_054,
			BIT(CODEC_LEAK_CTRL_OFFSET), 0);
		da_combine_update_bits(codec, CODEC_ANA_RWREG_051,
			MASK_ON_BIT(CODEC_FIR_OUT_CTRL_LEN, CODEC_FIR_OUT_CTRL_OFFSET) |
			MASK_ON_BIT(CODEC_FIR_ATE_CTRL_LEN, CODEC_FIR_ATE_CTRL_OFFSET),
			CODEC_FIR_OUT_NON << CODEC_FIR_OUT_CTRL_OFFSET |
			CODEC_FIR_ATE_X1 << CODEC_FIR_ATE_CTRL_OFFSET);
		da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
			BIT(CODEC_PD_INF_EMS_OFFSET),
			BIT(CODEC_PD_INF_EMS_OFFSET));
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
	AUDIO_LOGD("end");

	return 0;
}

static int s2_tx_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			BIT(S2_OL_PGA_CLK_EN_OFFSET) | BIT(S2_OR_PGA_CLK_EN_OFFSET),
			BIT(S2_OL_PGA_CLK_EN_OFFSET) | BIT(S2_OR_PGA_CLK_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			BIT(S2_OL_PGA_CLK_EN_OFFSET) | BIT(S2_OR_PGA_CLK_EN_OFFSET),
			0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
	AUDIO_LOGD("end");

	return 0;
}

static int earpiece_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *priv = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		priv->rcv_hp_classH_state = (unsigned int)priv->rcv_hp_classH_state | RCV_POWER_STATE;
		da_combine_v5_set_classH_config(codec, priv->rcv_hp_classH_state);
		/* earpiece dac on */
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_EPDAC_OFFSET), 0);
		/* earpiece pga on */
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_EPPGA_OFFSET), 0);
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_EPINT_OFFSET), 0);
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_EPVB_OFFSET), 0);
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_RSUB_EP_OFFSET), 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* earpiece pga off */
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_RSUB_EP_OFFSET),
			BIT(CODEC_ANA_PD_RSUB_EP_OFFSET));
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_EPVB_OFFSET),
			BIT(CODEC_ANA_PD_EPVB_OFFSET));
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_EPINT_OFFSET),
			BIT(CODEC_ANA_PD_EPINT_OFFSET));
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_EPPGA_OFFSET),
			BIT(CODEC_ANA_PD_EPPGA_OFFSET));
		/* earpiece dac pb */
		da_combine_update_bits(codec, CODEC_ANA_RWREG_014,
			BIT(CODEC_ANA_PD_EPDAC_OFFSET),
			BIT(CODEC_ANA_PD_EPDAC_OFFSET));
		priv->rcv_hp_classH_state = (unsigned int)priv->rcv_hp_classH_state & (~RCV_POWER_STATE);
		da_combine_v5_set_classH_config(codec, priv->rcv_hp_classH_state);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

#define OUT_DRV_WIDGET \
	SND_SOC_DAPM_OUT_DRV_E("HP_L_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		headphone_l_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("HP_R_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		headphone_r_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("EP_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		earpiece_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("IR_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		ir_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("S2_TX_DRV", \
		SC_S2_IF_L_REG, S2_IF_TX_ENA_OFFSET, 0, NULL, 0, \
		s2_tx_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV("S4_TX_DRV", \
		SC_S4_IF_L_REG, S4_IF_TX_ENA_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U1_DRV", \
		SLIM_UP_EN1_REG, SLIM_UP1_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U2_DRV", \
		SLIM_UP_EN1_REG, SLIM_UP2_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U3_DRV", \
		SLIM_UP_EN1_REG, SLIM_UP3_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U4_DRV", \
		SLIM_UP_EN1_REG, SLIM_UP4_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U5_DRV", \
		SLIM_UP_EN1_REG, SLIM_UP5_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U6_DRV", \
		SLIM_UP_EN1_REG, SLIM_UP6_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U7_DRV", \
		SLIM_UP_EN1_REG, SLIM_UP7_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U8_DRV", \
		SLIM_UP_EN1_REG, SLIM_UP8_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U9_DRV", \
		SLIM_UP_EN2_REG, SLIM_UP9_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U10_DRV", \
		SLIM_UP_EN2_REG, SLIM_UP10_EN_OFFSET, 0, \
		NULL, 0), \

static const struct snd_soc_dapm_widget path_widgets[] = {
	INPUT_WIDGET
	OUTPUT_WIDGET
	MIXER_WIDGET
	MUX_PART1_WIDGET
	S1_MUX_WIDGET
	ADC01_MUX_WIDGET
	OUT_DRV_WIDGET
};

static const struct snd_soc_dapm_widget path_widgets_for_single_kcontrol[] = {
	INPUT_WIDGET
	OUTPUT_WIDGET
	S1_MUX_WIDGET
	ADC01_MUX_WIDGET
	OUT_DRV_WIDGET
};

int da_combine_v5_add_path_widgets(struct snd_soc_component *codec, bool single_kcontrol)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	if (single_kcontrol)
		return snd_soc_dapm_new_controls(dapm, path_widgets_for_single_kcontrol,
			ARRAY_SIZE(path_widgets_for_single_kcontrol));
	else
		return snd_soc_dapm_new_controls(dapm, path_widgets,
			ARRAY_SIZE(path_widgets));
}


