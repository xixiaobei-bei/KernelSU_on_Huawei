/*
 * da_separate_mad_path.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "da_separate_mad_path.h"
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

#include "linux/hisi/audio_log.h"
#include "da_separate_utils.h"
#include "da_separate_type.h"
#include "soc_mad.h"
#include "soc_sctrl_interface.h"
#include "soc_acpu_baseaddr_interface.h"

#define SOC_SCTRL_SCPERCLKEN0_ADDR_SIZE     0x4

#define LOG_TAG "DA_separate_mad"

static const struct snd_kcontrol_new mad_controls[] = {
	SOC_SINGLE("MAD_MIC_PGA_GAIN",    CODEC_ANA_RW59_REG,  PGA_OFFSET,   0x1ff, 0),
	SOC_SINGLE("MAD_CIC_GAIN",    SIF_MAD_CIC_GAIN_REG,  MAD_CIC_GAIN_OFFSET,   0x3f, 0),
};

static const char * const mad_mic_mux_texts[] = {
	"NULL",
	"MAIN_MIC",
	"AUX_MIC",
	"NULL",
	"MIC3",
};

static const struct soc_enum mad_mic_mux_enum =
	SOC_ENUM_SINGLE(CODEC_ANA_RW58_REG, MAD_PGA_SEL_OFFSET,
		ARRAY_SIZE(mad_mic_mux_texts), mad_mic_mux_texts);

static const struct snd_kcontrol_new mad_mic_mux_controls =
	SOC_DAPM_ENUM("Mux", mad_mic_mux_enum);

static const struct snd_kcontrol_new sif_mad_switch =
	SOC_DAPM_SINGLE("ENABLE", VIR1_ADDR, REG_BIT5, 1, 0);

static int sif_mad_mic_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	unsigned int read_data;
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct platform_data *priv = snd_soc_component_get_drvdata(codec);
	void __iomem *remap_addr = ioremap(SOC_SCTRL_SCPERCLKEN0_ADDR(SOC_ACPU_SCTRL_BASE_ADDR),
			SOC_SCTRL_SCPERCLKEN0_ADDR_SIZE);

	if (!remap_addr) {
		AUDIO_LOGE("remap_addr is NULL");
		return 0;
	}

	read_data = readl(remap_addr) & BIT(SOC_SCTRL_SCPEREN0_gt_clk_mad_acpu_START);
	iounmap(remap_addr);
	remap_addr = NULL;

	if (read_data != BIT(SOC_SCTRL_SCPEREN0_gt_clk_mad_acpu_START)) {
		AUDIO_LOGE("asp mad not ready");
		return 0;
	}

	AUDIO_LOGI("power mode event: %d", event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		soc_mad_select_din(LOW_FREQ_MODE);
		if (priv->board_type == UDP_BOARD) {
			soc_mad_set_pinctrl_state(LOW_FREQ_MODE);
		} else {
			snd_soc_component_write(codec, 0xFA89C788, 0x3);
			snd_soc_component_write(codec, 0xFA89C78C, 0x3);
			snd_soc_component_write(codec, 0xFA89C794, 0x3);
		}
		snd_soc_component_write(codec, CTRL_REG_DIG_IO_DS_SEL1_CODEC_CFG_REG, 0x1);
		break;
	case SND_SOC_DAPM_POST_PMD:
		soc_mad_select_din(HIGH_FREQ_MODE);
		if (priv->board_type == UDP_BOARD) {
			soc_mad_set_pinctrl_state(HIGH_FREQ_MODE);
		} else {
			snd_soc_component_write(codec, 0xFA89C788, 0x1);
			snd_soc_component_write(codec, 0xFA89C78C, 0x1);
			snd_soc_component_write(codec, 0xFA89C794, 0x1);
		}
		snd_soc_component_write(codec, CTRL_REG_DIG_IO_DS_SEL1_CODEC_CFG_REG, 0x0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	return 0;
}

static int mad_mic_mux_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGI("power mode event: %d", event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_write(codec, CODEC_ANA_RW60_REG, 0x3F);
		snd_soc_component_update_bits(codec, CODEC_ANA_RW1_REG, 0x3 << AVREF_SEL_OFFSET, 0x2);
		snd_soc_component_update_bits(codec, CODEC_ANA_RW62_REG, BIT(DIN_MAD_MIC_OFFSET),
			BIT(DIN_MAD_MIC_OFFSET));
		snd_soc_component_update_bits(codec, CODEC_ANA_RW58_REG, BIT(VCM_MAD_LP_EN_OFFSET),
			BIT(VCM_MAD_LP_EN_OFFSET));

		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, CODEC_ANA_RW62_REG, BIT(DIN_MAD_MIC_OFFSET), 0x0);
		snd_soc_component_update_bits(codec, CODEC_ANA_RW58_REG, BIT(VCM_MAD_LP_EN_OFFSET), 0x0);

		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	return 0;
}

static int sif_mad_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGI("power mode event: %d", event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_write(codec, CODEC_ANA_RW60_REG, 0x02);
		usleep_range(10, 11);
		snd_soc_component_update_bits(codec, CODEC_ANA_RW60_REG, BIT(PD_MUTE_MAD_OFFSET), 0);
		usleep_range(200, 220);
		snd_soc_component_update_bits(codec, SIF_MAD_REG, BIT(MAD_CIC_CLKEN_OFFSET) | BIT(SIF_MAD_ADC_EN_OFFSET) | BIT(SIF_MAD_EN_OFFSET),
			BIT(MAD_CIC_CLKEN_OFFSET) | BIT(SIF_MAD_ADC_EN_OFFSET) | BIT(SIF_MAD_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, CODEC_ANA_RW60_REG, BIT(PD_MUTE_MAD_OFFSET), BIT(PD_MUTE_MAD_OFFSET));
		snd_soc_component_update_bits(codec, SIF_MAD_REG, BIT(MAD_CIC_CLKEN_OFFSET) | BIT(SIF_MAD_ADC_EN_OFFSET) | BIT(SIF_MAD_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	return 0;
}

static int mad_drv_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGI("power mode event: %d", event);
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		snd_soc_component_update_bits(codec, CODEC_ANA_RW1_REG, 0x3 << AVREF_SEL_OFFSET, 0x1);
		usleep_range(200, 220);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	return 0;
}

static const struct snd_soc_dapm_widget mad_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("MAD INPUT"),
	SND_SOC_DAPM_INPUT("MAD_AUX INPUT"),
	SND_SOC_DAPM_MIC("SIF_MAD_MIC", sif_mad_mic_event),
	SND_SOC_DAPM_MUX_E("MAD_MIC_MUX", SND_SOC_NOPM, 0, 0, &mad_mic_mux_controls, \
		mad_mic_mux_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SWITCH_E("SIF_MAD_SWITCH", SND_SOC_NOPM, 0, 0, &sif_mad_switch, \
		sif_mad_switch_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_OUT_DRV_E("MAD_ADC1 DRV", CODEC_ANA_RW3_REG, ADCL_MUTE_OFFSET, 1, NULL, 0, \
		mad_drv_power_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_OUTPUT("MAD OUTPUT"),
	SND_SOC_DAPM_OUTPUT("MAD_AUX OUTPUT"),
};

#define SIF_MAD_HIGH_FREQ \
	{ "MAD INPUT", 			NULL, 				"MICBIAS1 MIC"}, \
	{ "ANA_MUX3 MUX", 		"ADC1", 			"MAD INPUT"}, \
	{ "ANA_SIF_EN MIXER", 	"ANA_SIF_MAD_EN", 	"ANA_MUX3 MUX" }, \
	{ "MAD OUTPUT", 		NULL, 				"ANA_SIF_EN MIXER" }, \

#define SIF_MAD_HIGH_FREQ_SOC \
	{ "ADCL_FILTER",			NULL,				"ADC1 INPUT" }, \
	{ "SIF_EN MIXER",			"SIF_ADCL_EN",		"ADCL_FILTER" }, \
	{ "DA_AD_LOOP MUX",			"NORMAL",			"SIF_EN MIXER" }, \
	{ "ADC1 MUX",				"ADC1",				"DA_AD_LOOP MUX" }, \
	{ "MAD OUTPUT",				NULL,				"ADC1 MUX" }, \
	{ "ADCL_FILTER",			NULL,				"PLL" }, \

#define SIF_MAD_LOW_FREQ \
	{ "SIF_MAD_MIC",				NULL, 				"MAD INPUT" }, \
	{ "MAD_MIC_MUX",				"MAIN_MIC",			"SIF_MAD_MIC" }, \
	{ "MAD_MIC_MUX",				"AUX_MIC",			"SIF_MAD_MIC" }, \
	{ "MAD_MIC_MUX",				"MIC3",				"SIF_MAD_MIC" }, \
	{ "SIF_MAD_SWITCH",				"ENABLE",			"MAD_MIC_MUX" }, \
	{ "MAD OUTPUT", 				NULL, 				"SIF_MAD_SWITCH"}, \

#define SIF_MAD_LOW_FREQ_AUX \
	{ "MAD_AUX INPUT",				NULL,				"MICBIAS1 MIC" }, \
	{ "ANA_SIF_EN MIXER",			"ANA_SIF_MAD_EN",	"MAD_AUX INPUT"}, \
	{ "MAD_AUX OUTPUT",				NULL,				"ANA_SIF_EN MIXER" }, \

static const struct snd_soc_dapm_route mad_route_map[] = {
	SIF_MAD_HIGH_FREQ
	SIF_MAD_HIGH_FREQ_SOC
	SIF_MAD_LOW_FREQ
	SIF_MAD_LOW_FREQ_AUX
};

static struct snd_codec_resource mad_resource = {
	mad_controls,
	ARRAY_SIZE(mad_controls),
	mad_dapm_widgets,
	ARRAY_SIZE(mad_dapm_widgets),
	mad_route_map,
	ARRAY_SIZE(mad_route_map),
};

int add_mad(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	return da_separate_add_codec_resource(codec, &mad_resource);
}

