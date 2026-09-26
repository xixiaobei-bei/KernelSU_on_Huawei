/*
 * pga_widget.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "pga_widget.h"

#include <linux/version.h>
#include <sound/core.h>
#include <sound/tlv.h>

#include "asoc_adapter.h"
#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "linux/hisi/da_combine/da_combine_v5.h"
#include "linux/hisi/da_combine/da_combine_v5_regs.h"
#include "linux/hisi/da_combine/da_combine_v5_type.h"
#include "huawei_platform/power/vsys_switch/vsys_switch.h"

#include "switch_widget.h"


#define CP2_DELAY_TIME 5

static const struct mic_mux_node hsmic_mux_info[HSMIC_MUX_TOTAL_MIC] = {
	{
		.type = HS_MIC,
		.reg_value = 0x01
	},
	{
		.type = MAIN_MIC,
		.reg_value = 0x04
	}
};

static const struct mic_mux_node auxmic_mux_info[AUXMIC_MUX_TOTAL_MIC] = {
	{
		.type = AUX_MIC,
		.reg_value = 0x01
	},
	{
		.type = MAIN_MIC,
		.reg_value = 0x04
	}
};

static const struct mic_mux_node mic3_mux_info[MIC3_MUX_TOTAL_MIC] = {
	{
		.type = MIC3,
		.reg_value = 0x01
	},
	{
		.type = MIC4,
		.reg_value = 0x04
	}
};

static const struct mic_mux_node mic4_mux_info[MIC4_MUX_TOTAL_MIC] = {
	{
		.type = MIC4,
		.reg_value = 0x01
	},
	{
		.type = MIC3,
		.reg_value = 0x04
	}
};

static const struct mic_mux_node mic5_mux_info[MIC5_MUX_TOTAL_MIC] = {
	{
		.type = MIC5,
		.reg_value = 0x01
	}
};

static enum codec_mic_pin_type get_one_mic_mux_state(struct snd_soc_component *codec,
	unsigned int reg, unsigned int offset,
	const struct mic_mux_node *mic_mux, unsigned int total_mic)
{
	unsigned int raw_mux_reg;
	unsigned int mux_reg;
	unsigned int i;
	enum codec_mic_pin_type ret = NONE;

	raw_mux_reg = snd_soc_component_read32(codec, reg);
	mux_reg = (raw_mux_reg >> offset) & LOWEST_FOUR_BITS_MASK;
	for (i = 0; i < total_mic; i++) {
		if (mux_reg == mic_mux[i].reg_value) {
			ret = mic_mux[i].type;
			break;
		}
	}
	return ret;
}

static void get_all_mic_mux_state(struct snd_soc_component *codec,
	enum codec_mic_pin_type *mic_mux)
{
	/* HS Mic PGA MAX */
	mic_mux[ADC0] = get_one_mic_mux_state(codec,
		ANA_HSMIC_MUX_AND_PGA, HSMIC_MUX_OFFSET,
		hsmic_mux_info, HSMIC_MUX_TOTAL_MIC);
	/* Aux Mic PGA MAX */
	mic_mux[ADC1] = get_one_mic_mux_state(codec,
		ANA_AUXMIC_MUX_AND_PGA, AUXMIC_MUX_OFFSET,
		auxmic_mux_info, AUXMIC_MUX_TOTAL_MIC);
	/* Mic3 PGA MAX */
	mic_mux[ADC2] = get_one_mic_mux_state(codec,
		ANA_MIC3_MUX_AND_PGA, MIC3_MUX_OFFSET,
		mic3_mux_info, MIC3_MUX_TOTAL_MIC);
	/* Mic4 PGA MAX */
	mic_mux[ADC3] = get_one_mic_mux_state(codec,
		ANA_MIC4_MUX_AND_PGA, MIC4_MUX_OFFSET,
		mic4_mux_info, MIC4_MUX_TOTAL_MIC);
	/* Mic5 PGA MAX */
	mic_mux[ADC4] = get_one_mic_mux_state(codec,
		ANA_MIC5_MUX_AND_PGA, MIC5_MUX_OFFSET,
		mic5_mux_info, MIC5_MUX_TOTAL_MIC);
}

static unsigned int get_calib_value_by_mic_mux(
	struct da_combine_v5_platform_data *data, enum codec_mic_pin_type mic_mux)
{
	unsigned int calib_value;

	switch (mic_mux) {
	case HS_MIC:
		calib_value = MIC_CALIB_NONE_VALUE;
		break;
	case MAIN_MIC:
		calib_value = data->mic_calib_value[CODEC_CALIB_MAIN_MIC];
		break;
	case AUX_MIC:
		calib_value = data->mic_calib_value[CODEC_CALIB_AUX_MIC];
		break;
	case MIC3:
		calib_value = data->mic_calib_value[CODEC_CALIB_MIC3];
		break;
	case MIC4:
		calib_value = data->mic_calib_value[CODEC_CALIB_MIC4];
		break;
	case MIC5:
		calib_value = data->mic_calib_value[CODEC_CALIB_MIC5];
		break;
	default:
		calib_value = MIC_CALIB_NONE_VALUE;
		break;
	}

	/* in case the calib value is malicious modified */
	if (calib_value < MIC_CALIB_MIN_VALUE ||
		calib_value > MIC_CALIB_MAX_VALUE)
		calib_value = MIC_CALIB_NONE_VALUE;

	return calib_value;
}

static void set_calib_by_adc_mux(struct da_combine_v5_platform_data *data,
	unsigned int adc_type, enum codec_mic_pin_type *mic_mux,
	unsigned int reg)
{
	unsigned int calib_value;

	if (adc_type < TOTAL_MIC_ADC_TYPE) {
		calib_value = get_calib_value_by_mic_mux(data,
			mic_mux[adc_type]);
		snd_soc_component_write(data->codec, reg, calib_value);
		AUDIO_LOGI("adc %d set mic %u calib value as: %u\n",
			adc_type, mic_mux[adc_type], calib_value);
	}
}

static void set_mic_calib_value(struct da_combine_v5_platform_data *data)
{
	unsigned int adc_type;
	unsigned int adc0_mux;
	unsigned int adc1_mux;
	unsigned int adc2_mux;
	enum codec_mic_pin_type mic_mux[TOTAL_MIC_ADC_TYPE] = { NONE };

	/* ADC0L/R mux reg */
	adc0_mux = snd_soc_component_read32(data->codec, SC_CODEC_MUX_CTRL15_REG);
	/* ADC1L/R mux reg */
	adc1_mux = snd_soc_component_read32(data->codec, SC_CODEC_MUX_CTRL16_REG);
	/* ADC2L mux reg */
	adc2_mux = snd_soc_component_read32(data->codec, SC_CODEC_MUX_CTRL17_REG);

	get_all_mic_mux_state(data->codec, mic_mux);

	/* ADC0L */
	adc_type = (adc0_mux >> ADC0L_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC0L_PGA_CTRL1_REG);
	/* ADC0R */
	adc_type = (adc0_mux >> ADC0R_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC0R_PGA_CTRL1_REG);
	/* ADC1L */
	adc_type = (adc1_mux >> ADC1L_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC1L_PGA_CTRL1_REG);
	/* ADC1R */
	adc_type = (adc1_mux >> ADC1R_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC1R_PGA_CTRL1_REG);
	/* ADC2L */
	adc_type = (adc2_mux >> ADC2L_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC2L_PGA_CTRL1_REG);
}

static int mad_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, ANA_MAD_PGA_ENABLE,
			0x1 << ANA_MAD_PGA_PD | 0x1 << ANA_MAD_PGA_MUTE2 |
			0x1 << ANA_MAD_PGA_MUTE1, 0);
		da_combine_update_bits(codec, ANA_MAD_PGA_SDM,
			0x1 << ANA_MAD_PGA_PD_OUT | 0x1 << ANA_MAD_PGA_PD_IBIAS |
			0x1 << ANA_MAD_PGA_PD_SDM | 0x1 << ANA_MAD_PGA_MUTE_FLS |
			0x1 << ANA_MAD_PGA_MUTE_DWA_OUT, 0);
		da_combine_update_bits(codec, CODEC_ANA_RWREG_000,
			0x1 << PD_MAD_SDM_VREF_OFFSET, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, CODEC_ANA_RWREG_000,
			0x1 << PD_MAD_SDM_VREF_OFFSET,
			0x1 << PD_MAD_SDM_VREF_OFFSET);
		da_combine_update_bits(codec, ANA_MAD_PGA_SDM,
			0x1 << ANA_MAD_PGA_PD_OUT | 0x1 << ANA_MAD_PGA_PD_IBIAS |
			0x1 << ANA_MAD_PGA_PD_SDM | 0x1 << ANA_MAD_PGA_MUTE_FLS |
			0x1 << ANA_MAD_PGA_MUTE_DWA_OUT,
			0x1 << ANA_MAD_PGA_PD_OUT | 0x1 << ANA_MAD_PGA_PD_IBIAS |
			0x1 << ANA_MAD_PGA_PD_SDM | 0x1 << ANA_MAD_PGA_MUTE_FLS |
			0x1 << ANA_MAD_PGA_MUTE_DWA_OUT);
		da_combine_update_bits(codec, ANA_MAD_PGA_ENABLE,
			0x1 << ANA_MAD_PGA_PD | 0x1 << ANA_MAD_PGA_MUTE2 |
			0x1 << ANA_MAD_PGA_MUTE1,
			0x1 << ANA_MAD_PGA_PD | 0x1 << ANA_MAD_PGA_MUTE2 |
			0x1 << ANA_MAD_PGA_MUTE1);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static void hsmic_pga_enable(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, ANA_HSMIC_PGA_ENABLE,
		BIT(ANA_HSMIC_PGA_PD) | BIT(ANA_HSMIC_PGA_MUTE2) |
		BIT(ANA_HSMIC_PGA_MUTE1) | BIT(ANA_HSMIC_FLASH_MUTE) |
		BIT(ANA_HSMIC_PGA_SDM_PD), 0);
	da_combine_update_bits(codec, ANA_ADC_SDM,
		BIT(ANA_PD_OUT_HSMIC_OFFSET), 0x0);
	/* set mic calib values for all valid routes */
	set_mic_calib_value(data);

	AUDIO_LOGD("end");
}

static void hsmic_pga_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, ANA_ADC_SDM,
		BIT(ANA_PD_OUT_HSMIC_OFFSET),
		BIT(ANA_PD_OUT_HSMIC_OFFSET));
	da_combine_update_bits(codec, ANA_HSMIC_PGA_ENABLE,
		BIT(ANA_HSMIC_PGA_PD) | BIT(ANA_HSMIC_PGA_MUTE2) |
		BIT(ANA_HSMIC_PGA_MUTE1) | BIT(ANA_HSMIC_FLASH_MUTE) |
		BIT(ANA_HSMIC_PGA_SDM_PD),
		BIT(ANA_HSMIC_PGA_PD) | BIT(ANA_HSMIC_PGA_MUTE2) |
		BIT(ANA_HSMIC_PGA_MUTE1) | BIT(ANA_HSMIC_FLASH_MUTE) |
		BIT(ANA_HSMIC_PGA_SDM_PD));

	AUDIO_LOGD("end");
}

static int hsmic_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hsmic_pga_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hsmic_pga_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static int auxmic_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, ANA_AUXMIC_PGA_ENABLE,
			0x1 << ANA_AUXMIC_PGA_PD | 0x1 << ANA_AUXMIC_PGA_MUTE2 |
			0x1 << ANA_AUXMIC_PGA_MUTE1 | 0x1 << ANA_AUXMIC_FLASH_MUTE |
			0x1 << ANA_AUXMIC_PGA_SDM_PD, 0);
		da_combine_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_AUXMIC_OFFSET, 0x0);
		/* set mic calib values for all valid routes */
		set_mic_calib_value(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_AUXMIC_OFFSET,
			0x1 << ANA_PD_OUT_AUXMIC_OFFSET);
		da_combine_update_bits(codec, ANA_AUXMIC_PGA_ENABLE,
			0x1 << ANA_AUXMIC_PGA_PD | 0x1 << ANA_AUXMIC_PGA_MUTE2 |
			0x1 << ANA_AUXMIC_PGA_MUTE1 | 0x1 << ANA_AUXMIC_FLASH_MUTE |
			0x1 << ANA_AUXMIC_PGA_SDM_PD,
			0x1 << ANA_AUXMIC_PGA_PD | 0x1 << ANA_AUXMIC_PGA_MUTE2 |
			0x1 << ANA_AUXMIC_PGA_MUTE1 | 0x1 << ANA_AUXMIC_FLASH_MUTE |
			0x1 << ANA_AUXMIC_PGA_SDM_PD);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int mic3_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, ANA_MIC3_PGA_ENABLE,
			0x1 << ANA_MIC3_PGA_PD | 0x1 << ANA_MIC3_PGA_MUTE2 |
			0x1 << ANA_MIC3_PGA_MUTE1 | 0x1 << ANA_MIC3_FLASH_MUTE |
			0x1 << ANA_MIC3_PGA_SDM_PD, 0);
		da_combine_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC3_OFFSET, 0x0);
		/* set mic calib values for all valid routes */
		set_mic_calib_value(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC3_OFFSET,
			0x1 << ANA_PD_OUT_MIC3_OFFSET);
		da_combine_update_bits(codec, ANA_MIC3_PGA_ENABLE,
			0x1 << ANA_MIC3_PGA_PD | 0x1 << ANA_MIC3_PGA_MUTE2 |
			0x1 << ANA_MIC3_PGA_MUTE1 | 0x1 << ANA_MIC3_FLASH_MUTE |
			0x1 << ANA_MIC3_PGA_SDM_PD,
			0x1 << ANA_MIC3_PGA_PD | 0x1 << ANA_MIC3_PGA_MUTE2 |
			0x1 << ANA_MIC3_PGA_MUTE1 | 0x1 << ANA_MIC3_FLASH_MUTE |
			0x1 << ANA_MIC3_PGA_SDM_PD);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int mic4_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, ANA_MIC4_PGA_ENABLE,
			0x1 << ANA_MIC4_PGA_PD | 0x1 << ANA_MIC4_PGA_MUTE2 |
			0x1 << ANA_MIC4_PGA_MUTE1 | 0x1 << ANA_MIC4_FLASH_MUTE |
			0x1 << ANA_MIC4_PGA_SDM_PD, 0);
		da_combine_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC4_OFFSET, 0x0);
		/* set mic calib values for all valid routes */
		set_mic_calib_value(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC4_OFFSET,
			0x1 << ANA_PD_OUT_MIC4_OFFSET);
		da_combine_update_bits(codec, ANA_MIC4_PGA_ENABLE,
			0x1 << ANA_MIC4_PGA_PD | 0x1 << ANA_MIC4_PGA_MUTE2 |
			0x1 << ANA_MIC4_PGA_MUTE1 | 0x1 << ANA_MIC4_FLASH_MUTE |
			0x1 << ANA_MIC4_PGA_SDM_PD,
			0x1 << ANA_MIC4_PGA_PD | 0x1 << ANA_MIC4_PGA_MUTE2 |
			0x1 << ANA_MIC4_PGA_MUTE1 | 0x1 << ANA_MIC4_FLASH_MUTE |
			0x1 << ANA_MIC4_PGA_SDM_PD);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int mic5_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, ANA_MIC5_PGA_ENABLE,
			0x1 << ANA_MIC5_PGA_PD | 0x1 << ANA_MIC5_PGA_MUTE2 |
			0x1 << ANA_MIC5_PGA_MUTE1 | 0x1 << ANA_MIC5_FLASH_MUTE |
			0x1 << ANA_MIC5_PGA_SDM_PD, 0);
		da_combine_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC5_OFFSET, 0x0);
		/* set mic calib values for all valid routes */
		set_mic_calib_value(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC5_OFFSET,
			0x1 << ANA_PD_OUT_MIC5_OFFSET);
		da_combine_update_bits(codec, ANA_MIC5_PGA_ENABLE,
			0x1 << ANA_MIC5_PGA_PD | 0x1 << ANA_MIC5_PGA_MUTE2 |
			0x1 << ANA_MIC5_PGA_MUTE1 | 0x1 << ANA_MIC5_FLASH_MUTE |
			0x1 << ANA_MIC5_PGA_SDM_PD,
			0x1 << ANA_MIC5_PGA_PD | 0x1 << ANA_MIC5_PGA_MUTE2 |
			0x1 << ANA_MIC5_PGA_MUTE1 | 0x1 << ANA_MIC5_FLASH_MUTE |
			0x1 << ANA_MIC5_PGA_SDM_PD);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static void adcl0_enable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		0x1 << CLK_ADC0L_EN_OFFSET,
		0x1 << CLK_ADC0L_EN_OFFSET);
	da_combine_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
		0x1 << ADC0L_PGA_CLK_EN_OFFSET,
		0x1 << ADC0L_PGA_CLK_EN_OFFSET);

	AUDIO_LOGD("end");
}

static void adcl0_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
		0x1 << ADC0L_PGA_CLK_EN_OFFSET, 0);
	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		0x1 << CLK_ADC0L_EN_OFFSET, 0);

	AUDIO_LOGD("end");

}

int adcl0_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		adcl0_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		adcl0_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static void adcr0_enable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		0x1 << CLK_ADC0R_EN_OFFSET,
		0x1 << CLK_ADC0R_EN_OFFSET);
	da_combine_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
		0x1 << ADC0R_PGA_CLK_EN_OFFSET,
		0x1 << ADC0R_PGA_CLK_EN_OFFSET);

	AUDIO_LOGD("end");
}

static void adcr0_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
		0x1 << ADC0R_PGA_CLK_EN_OFFSET, 0);
	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		0x1 << CLK_ADC0R_EN_OFFSET, 0);

	AUDIO_LOGD("end");
}

int adcr0_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		adcr0_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		adcr0_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static void adcl1_enable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		0x1 << CLK_ADC1L_EN_OFFSET,
		0x1 << CLK_ADC1L_EN_OFFSET);
	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		0x1 << ADC1L_PGA_CLK_EN_OFFSET,
		0x1 << ADC1L_PGA_CLK_EN_OFFSET);

	AUDIO_LOGD("end");
}

static void adcl1_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		0x1 << ADC1L_PGA_CLK_EN_OFFSET, 0);
	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		0x1 << CLK_ADC1L_EN_OFFSET, 0);

	AUDIO_LOGD("end");
}

static int adcl1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		adcl1_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		adcl1_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static void adcr1_enable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		0x1 << CLK_ADC1R_EN_OFFSET,
		0x1 << CLK_ADC1R_EN_OFFSET);
	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		0x1 << ADC1R_PGA_CLK_EN_OFFSET,
		0x1 << ADC1R_PGA_CLK_EN_OFFSET);

	AUDIO_LOGD("end");
}

static void adcr1_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		0x1 << ADC1R_PGA_CLK_EN_OFFSET, 0);
	da_combine_update_bits(codec, ADC_DAC_CLK_EN_REG,
		0x1 << CLK_ADC1R_EN_OFFSET, 0);

	AUDIO_LOGD("end");
}

static int adcr1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		adcr1_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		adcr1_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adcl2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
			0x1 << CLK_ADC2L_EN_OFFSET,
			0x1 << CLK_ADC2L_EN_OFFSET);
		da_combine_update_bits(codec, DAC_DP_CLK_EN_2_REG,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DAC_DP_CLK_EN_2_REG,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET, 0);
		da_combine_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
			0x1 << CLK_ADC2L_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adcl2_virtbtn_ir_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		AUDIO_LOGW("adcl2_virtbtn_ir_power_event 1");
		snd_soc_component_write(codec, SC_CODEC_MUX_CTRL19_REG, 0x1);
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL39_REG,
			0x1 << DSPIF8_DIN_SEL_OFFSET,
			0x1 << DSPIF8_DIN_SEL_OFFSET);
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL39_REG,
			0x3 << DSPIF9_DIN_SEL_OFFSET,
			0x0 << DSPIF9_DIN_SEL_OFFSET);
		da_combine_update_bits(codec, ADC2L_CTRL0_REG,
			0x1 << ADC2L_HPF_BYPASS_EN_OFFSET |
			0x1 << ADC2L_IR2D_BYPASS_EN_OFFSET,
			0x1 << ADC2L_HPF_BYPASS_EN_OFFSET |
			0x1 << ADC2L_IR2D_BYPASS_EN_OFFSET);
		snd_soc_component_write(codec, SC_CODEC_MUX_CTRL11_REG, 0x1);
		da_combine_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
			0x1 << CLK_ADC2L_EN_OFFSET,
			0x1 << CLK_ADC2L_EN_OFFSET);
		da_combine_update_bits(codec, DAC_DP_CLK_EN_2_REG,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		AUDIO_LOGW("adcl2_virtbtn_ir_power_event 0");
		snd_soc_component_write(codec, SC_CODEC_MUX_CTRL11_REG, 0x0);
		da_combine_update_bits(codec, ADC2L_CTRL0_REG,
			0x1 << ADC2L_HPF_BYPASS_EN_OFFSET |
			0x1 << ADC2L_IR2D_BYPASS_EN_OFFSET, 0x0);
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL39_REG,
			0x1 << DSPIF8_DIN_SEL_OFFSET, 0x0);
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL39_REG,
			0x3 << DSPIF9_DIN_SEL_OFFSET,
			0x0 << DSPIF9_DIN_SEL_OFFSET);
		snd_soc_component_write(codec, SC_CODEC_MUX_CTRL19_REG, 0x0);
		da_combine_update_bits(codec, DAC_DP_CLK_EN_2_REG,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET, 0);
		da_combine_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
			0x1 << CLK_ADC2L_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int s2_il_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IL_PGA_CLK_EN_OFFSET | 0x1 << S2_IL_SRC_CLK_EN_OFFSET,
			0x1 << S2_IL_PGA_CLK_EN_OFFSET | 0x1 << S2_IL_SRC_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IL_PGA_CLK_EN_OFFSET | 0x1 << S2_IL_SRC_CLK_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int s2_ir_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IR_PGA_CLK_EN_OFFSET | 0x1 << S2_IR_SRC_CLK_EN_OFFSET,
			0x1 << S2_IR_PGA_CLK_EN_OFFSET | 0x1 << S2_IR_SRC_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IR_PGA_CLK_EN_OFFSET | 0x1 << S2_IR_SRC_CLK_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

#define PGA_WIDGET \
	SND_SOC_DAPM_PGA_S("MAD_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		mad_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MIC3_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		mic3_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MIC4_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		mic4_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MIC5_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		mic5_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL2", \
		1, SND_SOC_NOPM, 0, 0, adcl2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL2_VIRTBTN_IR", \
		1, SND_SOC_NOPM, 0, 0, adcl2_virtbtn_ir_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S3_IL_PGA", \
		0, S3_DP_CLK_EN_REG, S3_IL_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S3_IR_PGA", \
		0, S3_DP_CLK_EN_REG, S3_IR_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S4_IL_PGA", \
		0, VIRTUAL_DOWN_REG, S4_IL_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S4_IR_PGA", \
		0, VIRTUAL_DOWN_REG, S4_IR_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("SIDETONE_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		NULL, 0), \

#define COMMON_PGA_WIDGET \
	SND_SOC_DAPM_PGA_S("HSMIC_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		hsmic_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("AUXMIC_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		auxmic_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCR1", \
		1, SND_SOC_NOPM, 0, 0, adcr1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL1", \
		1, SND_SOC_NOPM, 0, 0, adcl1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCR0", \
		1, SND_SOC_NOPM, 0, 0, adcr0_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL0", \
		1, SND_SOC_NOPM, 0, 0, adcl0_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S1_IL_PGA", \
		0, S1_DP_CLK_EN_REG, S1_IL_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S1_IR_PGA", \
		0, S1_DP_CLK_EN_REG, S1_IR_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S2_IL_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		s2_il_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S2_IR_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		s2_ir_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

static const struct snd_soc_dapm_widget pga_widgets[] = {
	PGA_WIDGET
	COMMON_PGA_WIDGET
};

static const struct snd_soc_dapm_widget pga_widgets_for_single_kcontrol[] = {
	COMMON_PGA_WIDGET
};

int da_combine_v5_add_pga_widgets(struct snd_soc_component *codec, bool single_kcontrol)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	if (single_kcontrol)
		return snd_soc_dapm_new_controls(dapm, pga_widgets_for_single_kcontrol,
			ARRAY_SIZE(pga_widgets_for_single_kcontrol));
	else
		return snd_soc_dapm_new_controls(dapm, pga_widgets,
			ARRAY_SIZE(pga_widgets));
}


