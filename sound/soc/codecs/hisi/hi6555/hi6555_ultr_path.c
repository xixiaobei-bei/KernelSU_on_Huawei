/*
 * hi6555_ultr_path.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "hi6555_ultr_path.h"
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

#include "linux/hisi/audio_log.h"
#include "hi6555_type.h"
#include "hi6555_utils.h"

#define LOG_TAG "hi6555_ultr"

static const struct snd_kcontrol_new ultr_controls[] = {
	SOC_SINGLE("FS_ULTR_UPLINK",        FS_CTRL3_REG,             FS_ULTR_UPLINK_OFFSET,     0x3, 0),
	SOC_SINGLE("FS_ULTR_DLINK",         FS_CTRL3_REG,             FS_ULTR_DLINK_OFFSET,      0x3, 0),
	SOC_SINGLE("ULTR_DN_PGA_SRC",       FS_CTRL3_REG,             FS_ULTR_DN_PGA_OFFSET,     0x3, 0),
	SOC_SINGLE("IN1_DACL_SRC_OUT_PGA",  ULTR_MIXER2_CTRL_REG,     ULTR_MIXER2_GAIN1_OFFSET,  0x3, 0),
	SOC_SINGLE("IN2_ULTR_D_PGA",        ULTR_MIXER2_CTRL_REG,     ULTR_MIXER2_GAIN2_OFFSET,  0x3, 0),
	SOC_SINGLE("ULTR_DN_PGA_BYPASS",    ULTR_DN_PGA_CTRL_REG,     ULTR_DN_PGA_BYPASS_OFFSET, 0x1, 0),
	SOC_SINGLE("ULTR_DN_PGA",           ULTR_DN_PGA_CTRL_REG,     ULTR_DN_PGA_GAIN_OFFSET,   0xff, 0),
	SOC_SINGLE("ADC_ULTR_CIC_GAIN",     ADC_ULTR_FILTER_CTRL_REG, ADC_ULTR_CIC_GAIN_OFFSET,  0x3f, 0),
};

static const struct snd_kcontrol_new ultr_mixer2[] = {
	SOC_DAPM_SINGLE("DACL_SRC_OUT",
		ULTR_MIXER2_CTRL_REG, ULTR_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("ULTR_D",
		ULTR_MIXER2_CTRL_REG, ULTR_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const char *const ultr_dl_mux_texts[] = {
	"ULTR_D_FIFO",
	"ADC_ULTR",
};

static const struct soc_enum ultr_dl_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, ULTR_DN_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(ultr_dl_mux_texts), ultr_dl_mux_texts);

static const struct snd_kcontrol_new ultr_dl_mux =
	SOC_DAPM_ENUM("Mux", ultr_dl_mux_enum);

static const char *const ultr_dac_mux_texts[] = {
	"ULTR_D",
	"ULTR_MIXER",
};

static const struct soc_enum ultr_dac_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, DACML_DIN_SEL_OFFSET,
		ARRAY_SIZE(ultr_dac_mux_texts), ultr_dac_mux_texts);

static const struct snd_kcontrol_new ultr_dac_mux =
	SOC_DAPM_ENUM("Mux", ultr_dac_mux_enum);

static const struct snd_kcontrol_new ultr_dn_fifo_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR0_ADDR, REG_BIT28, 1, 0),
};

static const struct snd_kcontrol_new ultr_up_fifo_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR0_ADDR, REG_BIT29, 1, 0),
};

static const struct snd_kcontrol_new analog_dac_ultr_switch[] = {
	SOC_DAPM_SINGLE("DAC_ULTR", VIR0_ADDR, REG_BIT31, 1, 0),
};

static int ultr_dacl_filter_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* ultr dacl configuration */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(DACML_CLKEN_OFFSET), BIT(DACML_CLKEN_OFFSET));
		snd_soc_update_bits(codec, DACML_FILTER_CTRL_REG,
			BIT(DACML_HBF1I_BYPASS_EN_OFFSET),
			BIT(DACML_HBF1I_BYPASS_EN_OFFSET));
		snd_soc_update_bits(codec, DACML_FILTER_CTRL_REG,
			BIT(DACML_HBF2I_BYPASS_EN_OFFSET) |
			BIT(DACML_COMPI_BYPASS_EN_OFFSET), 0);
		/* src ultr mux */
		snd_soc_update_bits(codec, CODEC_DIN_MUX1_REG,
			BIT(ULTR_UP16_DIN_SEL_OFFSET), 0);
		/* src up and sdm clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(DACML_UP16_CLKEN_OFFSET) | BIT(SDM_ULTR_CLKEN_OFFSET),
			BIT(DACML_UP16_CLKEN_OFFSET) | BIT(SDM_ULTR_CLKEN_OFFSET));
		/* sdm calt enable */
		snd_soc_update_bits(codec, DACML_FILTER_CTRL_REG,
			BIT(SDM_ULTR_CALT_VLD_OFFSET), BIT(SDM_ULTR_CALT_VLD_OFFSET));
		/* sdm dither disable */
		snd_soc_update_bits(codec, DACML_FILTER_CTRL_REG,
			BIT(SDM_ULTR_DITHER_OFFSET), 0);
		/* sif smt dac ultr enable */
		snd_soc_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_MST_DAC_ULTR_EN_OFFSET), BIT(SIF_MST_DAC_ULTR_EN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* ultr dacl disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(DACML_CLKEN_OFFSET), 0);
		snd_soc_update_bits(codec, DACML_FILTER_CTRL_REG,
			BIT(DACML_HBF1I_BYPASS_EN_OFFSET), 0);
		/* src up and sdm clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(DACML_UP16_CLKEN_OFFSET) | BIT(SDM_ULTR_CLKEN_OFFSET), 0);
		/* sdm calt disable */
		snd_soc_update_bits(codec, DACML_FILTER_CTRL_REG,
			BIT(SDM_ULTR_CALT_VLD_OFFSET), 0);
		/* sif smt dac ultr disable */
		snd_soc_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_MST_DAC_ULTR_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int ultr_adc_filter_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ADC_ULTR_CLKEN_OFFSET), BIT(ADC_ULTR_CLKEN_OFFSET));
		/* hbfvd filter bypass */
		snd_soc_update_bits(codec, ADC_ULTR_FILTER_CTRL_REG,
			BIT(ADC_ULTR_HBFVD_BYPASS_EN_OFFSET),
			BIT(ADC_ULTR_HBFVD_BYPASS_EN_OFFSET));
		/* hpf hbf2d filter work */
		snd_soc_update_bits(codec, ADC_ULTR_FILTER_CTRL_REG,
			BIT(ADC_ULTR_HPF_BYPASS_EN_OFFSET) |
			BIT(ADC_ULTR_HBF2D_BYPASS_EN_OFFSET), 0);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ADC_ULTR_CLKEN_OFFSET), 0);
		/* hbfvd filter work */
		snd_soc_update_bits(codec, ADC_ULTR_FILTER_CTRL_REG,
			BIT(ADC_ULTR_HBFVD_BYPASS_EN_OFFSET), 0);
		/* hpf hbf2d filter bypass */
		snd_soc_update_bits(codec, ADC_ULTR_FILTER_CTRL_REG,
			BIT(ADC_ULTR_HPF_BYPASS_EN_OFFSET) |
			BIT(ADC_ULTR_HBF2D_BYPASS_EN_OFFSET),
			BIT(ADC_ULTR_HPF_BYPASS_EN_OFFSET) |
			BIT(ADC_ULTR_HBF2D_BYPASS_EN_OFFSET));
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int ultr_dn_fifo_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ULTR_DN_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ULTR_DN_AFIFO_CLKEN_OFFSET), BIT(ULTR_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ULTR_DN_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int ultr_up_fifo_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ULTR_UP_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ULTR_UP_AFIFO_CLKEN_OFFSET), BIT(ULTR_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ULTR_UP_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int analog_dac_ultr_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		snd_soc_update_bits(codec, CLK_EN_CFG_REG,
			BIT(DAC_ULTR_EN_OFFSET), BIT(DAC_ULTR_EN_OFFSET));
		snd_soc_update_bits(codec, DAC_CHAN_CTRL_REG,
			BIT(DAC_ULTR_DWA_EN_OFFSET), BIT(DAC_ULTR_DWA_EN_OFFSET));
		snd_soc_update_bits(codec, CODEC_ANA_RW4_REG,
			BIT(DACS_PD_OFFSET), 0);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		snd_soc_update_bits(codec, CLK_EN_CFG_REG,
			BIT(DAC_ULTR_EN_OFFSET), 0);
		snd_soc_update_bits(codec, DAC_CHAN_CTRL_REG,
			BIT(DAC_ULTR_DWA_EN_OFFSET), 0);
		snd_soc_update_bits(codec, CODEC_ANA_RW4_REG,
			BIT(DACS_PD_OFFSET), BIT(DACS_PD_OFFSET));
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static const struct snd_soc_dapm_widget ultr_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("ULTR_DLINK INPUT"),
	SND_SOC_DAPM_OUTPUT("ULTR_UP OUTPUT"),
	SND_SOC_DAPM_OUTPUT("ULTR_DLINK OUTPUT"),
	SND_SOC_DAPM_PGA_E("DACL_ULTR_FILTER",
		SND_SOC_NOPM, 0, 0, NULL, 0, ultr_dacl_filter_event,
		(SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_PGA_E("ADC_ULTR_FILTER",
		SND_SOC_NOPM, 0, 0, NULL, 0, ultr_adc_filter_event,
		(SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_PGA("ULTR_DLINK PGA", CODEC_CLK_EN3_REG,
		ULTR_DN_PGA_CLKEN_OFFSET, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("ULTR_DLINK_EN MIXER",
		CODEC_CLK_EN3_REG, ULTR_MIXER2_CLKEN_OFFSET, 0,
		ultr_mixer2, ARRAY_SIZE(ultr_mixer2)),
	SND_SOC_DAPM_MUX("ULTR_DL MUX", SND_SOC_NOPM, 0, 0, &ultr_dl_mux),
	SND_SOC_DAPM_MUX("DAC_ULTR MUX", SND_SOC_NOPM, 0, 0, &ultr_dac_mux),
	SND_SOC_DAPM_SWITCH_E("ULTR_DLINK_FIFO SWITCH",
		SND_SOC_NOPM, 0, 0, ultr_dn_fifo_switch, ultr_dn_fifo_switch_event,
		(SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_SWITCH_E("ULTR_UP_FIFO SWITCH",
		SND_SOC_NOPM, 0, 0, ultr_up_fifo_switch, ultr_up_fifo_switch_event,
		(SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_SWITCH_E("ANA_DAC_ULTR SWITCH", SND_SOC_NOPM, 0, 0,
		analog_dac_ultr_switch, analog_dac_ultr_switch_event,
		(SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
};

#define ULTR_UPLINK \
	{ "SIF_EN MIXER",              "SIF_ADC3_EN",    "ADC3 INPUT" }, \
	{ "ADC_ULTR MUX",              "ADC3",           "SIF_EN MIXER" }, \
	{ "ADC_ULTR MUX",              "ADC3",           "DA_AD_LOOP MUX" }, \
	{ "ADC_ULTR_FILTER",           NULL,             "ADC_ULTR MUX" }, \
	{ "ULTR_UP_FIFO SWITCH",       "ENABLE",         "ADC_ULTR_FILTER" }, \
	{ "ULTR_UP_FIFO SWITCH",       NULL,             "PLL" }, \
	{ "ULTR_UP OUTPUT",            NULL,             "ULTR_UP_FIFO SWITCH" }, \

#define ULTR_SOC_DOWNLINK \
	{ "ULTR_DLINK_FIFO SWITCH",    "ENABLE",          "ULTR_DLINK INPUT" }, \
	{ "ULTR_DLINK_FIFO SWITCH",    NULL,              "PLL" }, \
	{ "ULTR_DL MUX",               "ULTR_D_FIFO",     "ULTR_DLINK_FIFO SWITCH" }, \
	{ "ULTR_DL MUX",               "ADC_ULTR",        "ADC_ULTR MUX" }, \
	{ "ULTR_DLINK PGA",            NULL,              "ULTR_DL MUX" }, \
	{ "ULTR_DLINK_EN MIXER",       "ULTR_D",          "ULTR_DLINK PGA" }, \
	{ "ULTR_DLINK_EN MIXER",       "DACL_SRC_OUT",    "DAC_DL MUX" }, \
	{ "DAC_ULTR MUX",              "ULTR_MIXER",      "ULTR_DLINK_EN MIXER" }, \
	{ "DAC_ULTR MUX",              "ULTR_D",          "ULTR_DLINK PGA" }, \
	{ "DACL_ULTR_FILTER",          NULL,              "DAC_ULTR MUX" }, \
	{ "SIF_EN MIXER",              "SIF_DACUL_EN",    "DACL_ULTR_FILTER" }, \
	{ "ULTR_DLINK OUTPUT",         NULL,              "SIF_EN MIXER" }, \

#define ULTR_ANALOG_DOWNLINK \
	{ "ANA_SIF_EN MIXER",        "ANA_SIF_ULTR_EN",  "SMT_ANA_DAC_SDATA INPUT" }, \
	{ "ANA_DAC_ULTR SWITCH",     "DAC_ULTR",         "ANA_SIF_EN MIXER" }, \
	{ "ANA_EAR MIXER",           "DAC_ULTR",         "ANA_DAC_ULTR SWITCH" }, \
	{ "ANA_LINEOUT MIXER",       "DAC_ULTR",         "ANA_DAC_ULTR SWITCH" }, \


static const struct snd_soc_dapm_route ultr_route_map[] = {
	ULTR_UPLINK
	ULTR_SOC_DOWNLINK
	ULTR_ANALOG_DOWNLINK
};

static struct snd_codec_resource ultr_resource = {
	ultr_controls,
	ARRAY_SIZE(ultr_controls),
	ultr_dapm_widgets,
	ARRAY_SIZE(ultr_dapm_widgets),
	ultr_route_map,
	ARRAY_SIZE(ultr_route_map),
};

int add_ultr(struct snd_soc_codec *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	return hi6555_add_codec_resource(codec, &ultr_resource);
}

