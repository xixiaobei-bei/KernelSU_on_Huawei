/*
 * hi6555_voice_path.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "hi6555_voice_path.h"
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

#include "linux/hisi/audio_log.h"
#include "hi6555_type.h"
#include "hi6555_utils.h"

#define LOG_TAG "hi6555_voice"

static const struct snd_kcontrol_new voice_controls[] = {
	SOC_SINGLE("FS_VOICE_L_DLINK",           FS_CTRL0_REG,              FS_VOICE_L_DLINK_OFFSET,          0x3,  0),
	SOC_SINGLE("FS_VOICE_R_DLINK",           FS_CTRL2_REG,              FS_VOICE_R_DLINK_OFFSET,          0x3,  0),
	SOC_SINGLE("FS_VOICE_L_UPLINK",          FS_CTRL0_REG,              FS_VOICE_L_UPLINK_OFFSET,         0x3,  0),
	SOC_SINGLE("FS_VOICE_R_UPLINK",          FS_CTRL0_REG,              FS_VOICE_R_UPLINK_OFFSET,         0x3,  0),
	SOC_SINGLE("VOICE_L_DN_SRCUP_SRC1_DIN",  FS_CTRL2_REG,              FS_VOICE_L_DN_SRCUP_IN_OFFSET,    0x3,  0),
	SOC_SINGLE("VOICE_L_DN_SRCUP_SRC1_DOUT", FS_CTRL2_REG,              FS_VOICE_L_DN_SRCUP_DOUT_OFFSET,  0x3,  0),
	SOC_SINGLE("VOICE_L_DN_SRCUP_SRC1_MODE", SRCUP_CTRL_REG,            VOICE_L_DN_SRCUP_SRC_MODE_OFFSET, 0x7,  0),
	SOC_SINGLE("VOICE_R_DN_SRCUP_SRC3_DIN",  FS_CTRL2_REG,              FS_VOICE_R_DN_SRCUP_IN_OFFSET,    0x3,  0),
	SOC_SINGLE("VOICE_R_DN_SRCUP_SRC3_DOUT", FS_CTRL2_REG,              FS_VOICE_R_DN_SRCUP_DOUT_OFFSET,  0x3,  0),
	SOC_SINGLE("VOICE_R_DN_SRCUP_SRC3_MODE", SRCUP_CTRL_REG,            VOICE_R_DN_SRCUP_SRC_MODE_OFFSET, 0x7,  0),
	SOC_SINGLE("VOICE_L_UP_SRC7_MODE",       SRCDN_CTRL_REG,            VOICE_L_UP_SRCDN_SRC_MODE_OFFSET, 0x7,  0),
	SOC_SINGLE("VOICE_L_UP_SRC7",            FS_CTRL1_REG,              FS_VOICE_L_UP_SRCDN_OUT_OFFSET,   0x3,  0),
	SOC_SINGLE("VOICE_R_UP_SRC8_MODE",       SRCDN_CTRL_REG,            VOICE_R_UP_SRCDN_SRC_MODE_OFFSET, 0x7,  0),
	SOC_SINGLE("VOICE_R_UP_SRC8",            FS_CTRL1_REG,              FS_VOICE_R_UP_SRCDN_OUT_OFFSET,   0x3,  0),
	SOC_SINGLE("VOICE_L_DN_PGA_GAIN",        VOICE_L_DN_PGA_CTRL_REG,   VOICE_L_DN_PGA_GAIN_OFFSET,       0xff, 0),
	SOC_SINGLE("VOICE_L_DN_PGA_BYPASS",      VOICE_L_DN_PGA_CTRL_REG,   VOICE_L_DN_PGA_BYPASS_OFFSET,     0x1,  0),
	SOC_SINGLE("VOICE_R_DN_PGA_GAIN",        VOICE_R_DN_PGA_CTRL_REG,   VOICE_R_DN_PGA_GAIN_OFFSET,       0xff, 0),
	SOC_SINGLE("VOICE_R_DN_PGA_BYPASS",      VOICE_R_DN_PGA_CTRL_REG,   VOICE_R_DN_PGA_BYPASS_OFFSET,     0x1,  0),
	SOC_SINGLE("IN2_VOICE_DL_PGA",           DACL_MIXER4_CTRL0_REG,     DACL_MIXER4_GAIN2_OFFSET,         0x3,  0),
	SOC_SINGLE("IN2_VOICE_DR_PGA",           DACR_MIXER4_CTRL0_REG,     DACR_MIXER4_GAIN2_OFFSET,         0x3,  0),
};

static int voice_dl_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* pga/fifo/src clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_L_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET) |
			BIT(VOICE_L_DN_SRCUP_CLKEN_OFFSET),
			BIT(VOICE_L_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET) |
			BIT(VOICE_L_DN_SRCUP_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_L_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET) |
			BIT(VOICE_L_DN_SRCUP_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int voice_dr_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* pga/fifo/src clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(VOICE_R_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET) |
			BIT(VOICE_R_DN_SRCUP_CLKEN_OFFSET),
			BIT(VOICE_R_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET) |
			BIT(VOICE_R_DN_SRCUP_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(VOICE_R_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET) |
			BIT(VOICE_R_DN_SRCUP_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int voice_ul_fifo_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET),
			BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int voice_ur_fifo_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET),
			BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static const char *const voice_ul_texts[] = {
	"ADCL_SRC",
	"S2_L",
};

static const struct soc_enum voice_ul_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_VOICE_L_UP_DIN_SEL_OFFSET,
	ARRAY_SIZE(voice_ul_texts), voice_ul_texts);

static const struct snd_kcontrol_new voice_ul_mux =
	SOC_DAPM_ENUM("Mux", voice_ul_mux_enum);

static const char *const voice_ur_mux_texts[] = {
	"ADCR_SRC",
	"S2_R",
};

static const struct soc_enum voice_ur_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_VOICE_R_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(voice_ur_mux_texts), voice_ur_mux_texts);

static const struct snd_kcontrol_new voice_ur_mux =
	SOC_DAPM_ENUM("Mux", voice_ur_mux_enum);

static const struct snd_kcontrol_new voice_ul_fifo_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR0_ADDR, REG_BIT4, 1, 0),
};

static const struct snd_kcontrol_new voice_ur_fifo_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR0_ADDR, REG_BIT5, 1, 0),
};

static const struct snd_soc_dapm_widget voice_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("VOICE_DLINK INPUT"),
	SND_SOC_DAPM_OUTPUT("VOICE_UL OUTPUT"),
	SND_SOC_DAPM_OUTPUT("VOICE_UR OUTPUT"),
	SND_SOC_DAPM_PGA_E("VOICE_DL PGA", SND_SOC_NOPM, 0, 0, NULL, 0,
		voice_dl_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_PGA_E("VOICE_DR PGA", SND_SOC_NOPM, 0, 0, NULL, 0,
		voice_dr_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_PGA("VOICE_UP_SRC7_EN PGA", CODEC_CLK_EN1_REG,
		VOICE_L_UP_SRCDN_CLKEN_OFFSET, 0, NULL, 0),
	SND_SOC_DAPM_PGA("VOICE_UP_SRC8_EN PGA", CODEC_CLK_EN1_REG,
		VOICE_R_UP_SRCDN_CLKEN_OFFSET, 0, NULL, 0),
	SND_SOC_DAPM_MUX("VOICE_UL MUX", SND_SOC_NOPM, 0, 0, &voice_ul_mux),
	SND_SOC_DAPM_MUX("VOICE_UR MUX", SND_SOC_NOPM, 0, 0, &voice_ur_mux),
	SND_SOC_DAPM_SWITCH_E("VOICE_UL_FIFO SWITCH", SND_SOC_NOPM, 0, 0,
		voice_ul_fifo_switch, voice_ul_fifo_switch_event,
		(SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_SWITCH_E("VOICE_UR_FIFO SWITCH", SND_SOC_NOPM, 0, 0,
		voice_ur_fifo_switch, voice_ur_fifo_switch_event,
		(SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
};

#define VOICE_DOWNLINK \
	{ "VOICE_DL PGA",    NULL,            "VOICE_DLINK INPUT" }, \
	{ "VOICE_DR PGA",    NULL,            "VOICE_DLINK INPUT" }, \
	{ "DAC_DL MIXER",    "IN2_VOICE_DL",  "VOICE_DL PGA" }, \
	{ "DAC_DR MIXER",    "IN2_VOICE_DR",  "VOICE_DR PGA" }, \

#define VOICE_UPLINK \
	{ "VOICE_UP_SRC7_EN PGA",   NULL,       "ADCL_FILTER" }, \
	{ "VOICE_UP_SRC8_EN PGA",   NULL,       "ADCR_FILTER" }, \
	{ "VOICE_UL MUX",           "ADCL_SRC", "VOICE_UP_SRC7_EN PGA" }, \
	{ "VOICE_UR MUX",           "ADCR_SRC", "VOICE_UP_SRC8_EN PGA" }, \
	{ "VOICE_UL_FIFO SWITCH",   "ENABLE",   "VOICE_UL MUX" }, \
	{ "VOICE_UR_FIFO SWITCH",   "ENABLE",   "VOICE_UR MUX" }, \
	{ "VOICE_UL_FIFO SWITCH",   NULL,       "PLL" }, \
	{ "VOICE_UR_FIFO SWITCH",   NULL,       "PLL" }, \
	{ "VOICE_UL OUTPUT",        NULL,       "VOICE_UL_FIFO SWITCH" }, \
	{ "VOICE_UR OUTPUT",        NULL,       "VOICE_UR_FIFO SWITCH" }, \

static const struct snd_soc_dapm_route voice_route_map[] = {
	VOICE_DOWNLINK
	VOICE_UPLINK
};

static struct snd_codec_resource voice_resource = {
	voice_controls,
	ARRAY_SIZE(voice_controls),
	voice_dapm_widgets,
	ARRAY_SIZE(voice_dapm_widgets),
	voice_route_map,
	ARRAY_SIZE(voice_route_map),
};

int add_voice(struct snd_soc_codec *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	return hi6555_add_codec_resource(codec, &voice_resource);
}
