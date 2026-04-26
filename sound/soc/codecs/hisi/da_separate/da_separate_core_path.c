/*
 * da_separate_core_path.c codec driver.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "da_separate_core_path.h"

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include "linux/hisi/audio_log.h"
#include "da_separate_utils.h"
#include "da_separate_type.h"

#define LOG_TAG "DA_separate_core"

static const struct snd_kcontrol_new dac_dl_mixer[] = {
	SOC_DAPM_SINGLE("IN1_S2_L", DACL_MIXER4_CTRL0_REG,
		DACL_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("IN2_VOICE_DL", DACL_MIXER4_CTRL0_REG,
		DACL_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("IN3_AUDIO_DL", DACL_MIXER4_CTRL0_REG,
		DACL_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("IN4_SIDE_TONE_L", DACL_MIXER4_CTRL0_REG,
		DACL_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dac_dr_mixer[] = {
	SOC_DAPM_SINGLE("IN1_S2_R", DACR_MIXER4_CTRL0_REG,
		DACR_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("IN2_VOICE_DR", DACR_MIXER4_CTRL0_REG,
		DACR_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("IN3_AUDIO_DR", DACR_MIXER4_CTRL0_REG,
		DACR_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("IN4_SIDE_TONE_R", DACR_MIXER4_CTRL0_REG,
		DACR_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new sif_en_mixer[] = {
	SOC_DAPM_SINGLE("SIF_DACL_EN",    VIR0_ADDR, REG_BIT11, 1, 0),
	SOC_DAPM_SINGLE("SIF_DACR_EN",    VIR0_ADDR, REG_BIT12, 1, 0),
	SOC_DAPM_SINGLE("SIF_DACUL_EN",   VIR1_ADDR, REG_BIT2, 1, 0),
	SOC_DAPM_SINGLE("SIF_DAC_SPA_EN", CODEC_CLK_EN3_REG, CLASSH_CLKEN_OFFSET, 1, 0),
#if (defined(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V6B)) || (defined(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V6L))
	SOC_DAPM_SINGLE("SIF_DAC_SPA2_EN", VIR1_ADDR, REG_BIT3, 1, 0),
	SOC_DAPM_SINGLE("SIF_DAC_SPA3_EN", VIR1_ADDR, REG_BIT4, 1, 0),
	SOC_DAPM_SINGLE("SIF_ADC_SPA2_EN", VIR1_ADDR, REG_BIT12, 1, 0),
	SOC_DAPM_SINGLE("SIF_ADC_SPA3_EN", VIR1_ADDR, REG_BIT13, 1, 0),
#endif
	SOC_DAPM_SINGLE("SIF_ADCL_EN",    VIR0_ADDR, REG_BIT13, 1, 0),
	SOC_DAPM_SINGLE("SIF_ADCR_EN",    VIR0_ADDR, REG_BIT14, 1, 0),
	SOC_DAPM_SINGLE("SIF_ADC3_EN",    VIR0_ADDR, REG_BIT15, 1, 0),
	SOC_DAPM_SINGLE("SIF_ADC_SPA_EN", VIR1_ADDR, REG_BIT1, 1, 0),
};

static const char *const dacl_mux_texts[] = {
	"DACL_MIXER",
	"S2_L",
	"S1_L",
};

static const struct soc_enum dacl_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, DACL_SRCUP_DIN_SEL_OFFSET,
		ARRAY_SIZE(dacl_mux_texts), dacl_mux_texts);

static const struct snd_kcontrol_new dacl_mux =
	SOC_DAPM_ENUM("Mux", dacl_mux_enum);

static const char *const dacr_mux_texts[] = {
	"DACR_MIXER",
	"S2_R",
	"S1_R",
};

static const struct soc_enum dacr_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, DACR_SRCUP_DIN_SEL_OFFSET,
		ARRAY_SIZE(dacr_mux_texts), dacr_mux_texts);

static const struct snd_kcontrol_new dacr_mux =
	SOC_DAPM_ENUM("Mux", dacr_mux_enum);

static const char *const ad_da_loopback_mux_texts[] = {
	"NORMAL",
	"AD_DA_LOOP",
	"MIC3_DA_LOOP",
};

static const struct soc_enum ad_da_loopback_mux_enum =
	SOC_ENUM_SINGLE(DAC_FILTER_CTRL_REG, SIF_MST_P2S_LOOPBACK_OFFSET,
		ARRAY_SIZE(ad_da_loopback_mux_texts), ad_da_loopback_mux_texts);

static const struct snd_kcontrol_new ad_da_loopback_mux =
	SOC_DAPM_ENUM("Mux", ad_da_loopback_mux_enum);

/* sif da_ad loopback mux */
static const char *const da_ad_loopback_mux_texts[] = {
	"NORMAL",
	"DALR_ADLR_LOOP",
	"DA_SOUT_AD_SIN_LOOP",
	"DA_SOUT_MIC3_SIN_LOOP",
};

static const struct soc_enum da_ad_loopback_mux_enum =
	SOC_ENUM_SINGLE(DAC_FILTER_CTRL_REG, SIF_MST_S2P_LOOPBACK_OFFSET,
		ARRAY_SIZE(da_ad_loopback_mux_texts), da_ad_loopback_mux_texts);

static const struct snd_kcontrol_new da_ad_loopback_mux =
	SOC_DAPM_ENUM("Mux", da_ad_loopback_mux_enum);

static const char *const adc_loopback_mux_texts[] = {
	"NORMAL",
	"DA_AD_LOOP",
	"DA_MIC3_LOOP",
};

static const struct soc_enum adc_loopback_mux_enum =
	SOC_ENUM_SINGLE(SIF_LOOPBACK_CFG_REG, ADC_LOOPBACK_OFFSET,
		ARRAY_SIZE(adc_loopback_mux_texts), adc_loopback_mux_texts);

static const struct snd_kcontrol_new adc_loopback_mux =
	SOC_DAPM_ENUM("Mux", adc_loopback_mux_enum);

/* dac_loopback mux */
static const char *const dac_loopback_mux_texts[] = {
	"NORMAL",
	"DOUT_SDIN",
	"MIC3_SDIN",
};

static const struct soc_enum dac_loopback_mux_enum =
	SOC_ENUM_SINGLE(SIF_LOOPBACK_CFG_REG, DAC_LOOPBACK_OFFSET,
		ARRAY_SIZE(dac_loopback_mux_texts), dac_loopback_mux_texts);

static const struct snd_kcontrol_new dac_loopback_mux =
	SOC_DAPM_ENUM("Mux", dac_loopback_mux_enum);

static void ibias_work_enable(struct snd_soc_component *codec, bool enable)
{
	struct platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

	mutex_lock(&priv->ibias.ibias_mutex);
	if (enable) {
		if (priv->ibias.ibias_work == 0)
			priv->ibias.set_ibias(codec, true);

		WARN_ON(priv->ibias.ibias_work == MAX_INT32);
		++priv->ibias.ibias_work;
	} else {
		if (priv->ibias.ibias_work <= 0) {
			AUDIO_LOGE("ibias_work is %d, fail to disable ibias", priv->ibias.ibias_work);
			mutex_unlock(&priv->ibias.ibias_mutex);
			return;
		}

		--priv->ibias.ibias_work;

		if (priv->ibias.ibias_work == 0)
			priv->ibias.set_ibias(codec, false);
	}
	mutex_unlock(&priv->ibias.ibias_mutex);

	OUT_FUNCTION;
}

static int smt_ibias_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ibias_work_enable(codec, true);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ibias_work_enable(codec, false);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int pll_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

	if (priv->board_type == UDP_BOARD) {
		switch (event) {
		case SND_SOC_DAPM_PRE_PMU:
			priv->pm.have_dapm = true;
			break;
		case SND_SOC_DAPM_POST_PMD:
			priv->pm.have_dapm = false;
			break;
		default:
			AUDIO_LOGE("power mode event err: %d", event);
			break;
		}
	} else {
		AUDIO_LOGI("fpga clk is always on");
	}

	OUT_FUNCTION;

	return 0;
}

static int classd_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (priv->resource.set_classd_supply)
			priv->resource.set_classd_supply(codec, true);
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (priv->resource.set_classd_supply)
			priv->resource.set_classd_supply(codec, false);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

#define SUPPLY_WIDGET \
	SND_SOC_DAPM_SUPPLY("SMT_IBIAS SUPPLY", SND_SOC_NOPM, 0, 0, \
		smt_ibias_supply_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY("PLL", SND_SOC_NOPM, 0, 0, \
		pll_supply_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY("CLASSD SUPPLY", SND_SOC_NOPM, 0, 0, \
		classd_supply_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

#define CORE_MIXER_WIDGET \
	SND_SOC_DAPM_MIXER("DAC_DL MIXER", CODEC_CLK_EN0_REG, DACL_MIXER4_CLKEN_OFFSET, 0, \
		dac_dl_mixer, ARRAY_SIZE(dac_dl_mixer)), \
	SND_SOC_DAPM_MIXER("DAC_DR MIXER", CODEC_CLK_EN0_REG, DACR_MIXER4_CLKEN_OFFSET, 0, \
		dac_dr_mixer, ARRAY_SIZE(dac_dr_mixer)), \
	SND_SOC_DAPM_MIXER("SIF_EN MIXER", CODEC_CLK_EN1_REG, SIF_MST_CLKEN_OFFSET, 0, \
		sif_en_mixer, ARRAY_SIZE(sif_en_mixer)), \

#define CORE_MUX_WIDGET \
	SND_SOC_DAPM_MUX("DAC_DL MUX", SND_SOC_NOPM, 0, 0, &dacl_mux), \
	SND_SOC_DAPM_MUX("DAC_DR MUX", SND_SOC_NOPM, 0, 0, &dacr_mux), \
	SND_SOC_DAPM_MUX("AD_DA_LOOP MUX", SND_SOC_NOPM, 0, 0, &ad_da_loopback_mux), \
	SND_SOC_DAPM_MUX("DA_AD_LOOP MUX", SND_SOC_NOPM, 0, 0, &da_ad_loopback_mux), \
	SND_SOC_DAPM_MUX("ADC_LOOP MUX", SND_SOC_NOPM, 0, 0, &adc_loopback_mux), \
	SND_SOC_DAPM_MUX("DAC_LOOPBACK MUX", SND_SOC_NOPM, 0, 0, &dac_loopback_mux), \


static const struct snd_soc_dapm_widget core_dapm_widgets[] = {
	SUPPLY_WIDGET
	CORE_MIXER_WIDGET
	CORE_MUX_WIDGET
};

static struct snd_codec_resource core_resource = {
	NULL,
	0,
	core_dapm_widgets,
	ARRAY_SIZE(core_dapm_widgets),
	NULL,
	0,
};

int add_core(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	return da_separate_add_codec_resource(codec, &core_resource);
}

