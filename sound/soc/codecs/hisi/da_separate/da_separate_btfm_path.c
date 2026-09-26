/*
 * da_separate_btfm_path.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "da_separate_btfm_path.h"
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

#include "linux/hisi/audio_log.h"
#include "da_separate_type.h"
#include "da_separate_i2s.h"
#include "da_separate_utils.h"

#define LOG_TAG "DA_separate_btfm"

static const struct snd_kcontrol_new btfm_controls[] = {
	SOC_SINGLE("FS_I2S2",                   I2S2_PCM_CTRL_REG,     FS_I2S2_OFFSET,                   0x7, 0),
	SOC_SINGLE("I2S2_TX_CLK_SEL",           I2S2_PCM_CTRL_REG,     I2S2_TX_CLK_SEL_OFFSET,           0x1, 0),
	SOC_SINGLE("I2S2_RX_CLK_SEL",           I2S2_PCM_CTRL_REG,     I2S2_RX_CLK_SEL_OFFSET,           0x1, 0),
	SOC_SINGLE("I2S2_MST_SLV",              I2S2_PCM_CTRL_REG,     I2S2_MST_SLV_OFFSET,              0x1, 0),
	SOC_SINGLE("I2S2_FUNC_MODE",            I2S2_PCM_CTRL_REG,     I2S2_FUNC_MODE_OFFSET,            0x7, 0),
	SOC_SINGLE("I2S2_FRAME_MODE",           I2S2_PCM_CTRL_REG,     I2S2_FRAME_MODE_OFFSET,           0x1, 0),
	SOC_SINGLE("I2S2_DIRECT_LOOP",          I2S2_PCM_CTRL_REG,     I2S2_DIRECT_LOOP_OFFSET,          0x3, 0),
	SOC_SINGLE("I2S2_CODEC_IO_WORDLENGTH",  I2S2_PCM_CTRL_REG,     I2S2_CODEC_IO_WORDLENGTH_OFFSET,  0x3, 0),
	SOC_SINGLE("BT_L_RX_SRCDN_IN_SRC14",    FS_CTRL2_REG,          FS_BT_L_RX_SRCDN_IN_OFFSET,       0x7, 0),
	SOC_SINGLE("BT_R_RX_SRCUP_OUT_SRC14",   FS_CTRL2_REG,          FS_BT_L_RX_SRCDN_OUT_OFFSET,      0x7, 0),
	SOC_SINGLE("BT_R_RX_SRCDN_IN_SRC15",    FS_CTRL2_REG,          FS_BT_R_RX_SRCDN_IN_OFFSET,       0x7, 0),
	SOC_SINGLE("BT_R_RX_SRCDN_OUT_SRC15",   FS_CTRL2_REG,          FS_BT_R_RX_SRCDN_OUT_OFFSET,      0x7, 0),
	SOC_SINGLE("BT_L_TX_SRCDN_SRC16_MODE",  SRCDN_CTRL_REG,        BT_L_TX_SRCDN_SRC_MODE_OFFSET,    0x7, 0),
	SOC_SINGLE("BT_L_TX_SRCDN_OUT_SRC16",   FS_CTRL3_REG,          FS_I2S2_TX_L_SRCDN_DOUT_OFFSET,   0x7, 0),
	SOC_SINGLE("BT_R_TX_SRCDN_SRC17_MODE",  SRCDN_CTRL1_REG,       BT_TX_R_SRCDN_SRC_MODE_OFFSET,    0x7, 0),
	SOC_SINGLE("BT_R_TX_SRCDN_IN_SRC17",    FS_CTRL3_REG,          FS_I2S2_TX_R_SRCDN_DIN_OFFSET,    0x1, 0),
	SOC_SINGLE("BT_R_TX_SRCDN_OUT_SRC17",   FS_CTRL3_REG,          FS_I2S2_TX_R_SRCDN_DOUT_OFFSET,   0x7, 0),
	SOC_SINGLE("BT_L_RX_SRCUP_SRC12_MODE",  SRCUP_CTRL_REG,        BT_L_RX_SRCUP_SRC_MODE_OFFSET,    0x7, 0),
	SOC_SINGLE("BT_L_RX_SRCUP_IN_SRC12",    FS_CTRL1_REG,          FS_BT_L_RX_SRCUP_IN_OFFSET,       0x7, 0),
	SOC_SINGLE("BT_L_RX_SRCUP_OUT_SRC12",   FS_CTRL1_REG,          FS_BT_L_RX_SRCUP_OUT_OFFSET,      0x7, 0),
	SOC_SINGLE("BT_R_RX_SRCDN_SRC13_MODE",  SRCUP_CTRL_REG,        BT_R_RX_SRCUP_SRC_MODE_OFFSET,    0x7, 0),
	SOC_SINGLE("BT_R_RX_SRCUP_IN_SRC13",    FS_CTRL1_REG,          FS_BT_R_RX_SRCUP_IN_OFFSET,       0x7, 0),
	SOC_SINGLE("BT_R_RX_SRCUP_OUT_SRC13",   FS_CTRL1_REG,          FS_BT_R_RX_SRCUP_OUT_OFFSET,      0x7, 0),
	SOC_SINGLE("BT_L_RX_SRCDN_SRC14_MODE",  SRCDN_CTRL_REG,        BT_L_RX_SRCDN_SRC_MODE_OFFSET,    0x7, 0),
	SOC_SINGLE("BT_R_RX_SRCDN_SRC15_MODE",  SRCDN_CTRL_REG,        BT_R_RX_SRCDN_SRC_MODE_OFFSET,    0x7, 0),
	SOC_SINGLE("BT_L_RX_PGA_BYPASS",        BT_L_RX_PGA_CTRL_REG,  BT_L_RX_PGA_BYPASS_OFFSET,        0x1, 0),
	SOC_SINGLE("BT_R_RX_PGA_BYPASS",        BT_R_RX_PGA_CTRL_REG,  BT_R_RX_PGA_BYPASS_OFFSET,        0x1, 0),
	SOC_SINGLE("BT_L_RX_PGA_GAIN",          BT_L_RX_PGA_CTRL_REG,  BT_L_RX_PGA_GAIN_OFFSET,          0xff, 0),
	SOC_SINGLE("BT_R_RX_PGA_GAIN",          BT_R_RX_PGA_CTRL_REG,  BT_R_RX_PGA_GAIN_OFFSET,          0xff, 0),
	SOC_SINGLE("TX_MIXER2_GAIN1_PGA",       BT_TX_MIXER2_CTRL_REG, BT_TX_MIXER2_GAIN1_OFFSET,        0x3, 0),
	SOC_SINGLE("TX_MIXER2_GAIN2_PGA",       BT_TX_MIXER2_CTRL_REG, BT_TX_MIXER2_GAIN2_OFFSET,        0x3, 0),
	SOC_SINGLE("IN1_S2_L_PGA",              DACL_MIXER4_CTRL0_REG, DACL_MIXER4_GAIN1_OFFSET,         0x3, 0),
	SOC_SINGLE("IN1_S2_R_PGA",              DACR_MIXER4_CTRL0_REG, DACR_MIXER4_GAIN1_OFFSET,         0x3, 0),
};

static int i2s2_rx_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable s2 lr uplink pga clk */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(BT_L_RX_PGA_CLKEN_OFFSET) | BIT(BT_R_RX_PGA_CLKEN_OFFSET),
			BIT(BT_L_RX_PGA_CLKEN_OFFSET) | BIT(BT_R_RX_PGA_CLKEN_OFFSET));

		/* enable i2s2 rx */
		snd_soc_component_update_bits(codec, I2S2_PCM_CTRL_REG,
			BIT(I2S2_IF_RX_ENA_OFFSET), BIT(I2S2_IF_RX_ENA_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/* disable i2s2 rx */
		snd_soc_component_update_bits(codec, I2S2_PCM_CTRL_REG,
			BIT(I2S2_IF_RX_ENA_OFFSET), 0);

		/* disable s2 lr uplink pga clk */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(BT_L_RX_PGA_CLKEN_OFFSET) |
			BIT(BT_R_RX_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("i2s2_src_pga_event event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s2_src_i2s2_tx_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable s2 src downlink */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(BT_TX_L_SRCDN_CLKEN_OFFSET), BIT(BT_TX_L_SRCDN_CLKEN_OFFSET));

		/* enable i2s2 tx */
		snd_soc_component_update_bits(codec, I2S2_PCM_CTRL_REG,
			BIT(I2S2_IF_TX_ENA_OFFSET), BIT(I2S2_IF_TX_ENA_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/* disable i2s2 tx */
		snd_soc_component_update_bits(codec, I2S2_PCM_CTRL_REG,
			BIT(I2S2_IF_TX_ENA_OFFSET), 0);

		/* disable s2 src downlink */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(BT_TX_L_SRCDN_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("s2_src_i2s2_clk_tx_switch event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s2_src12_13_rx_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable s2 src12 and src13 uplink clk */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(BT_L_RX_SRCUP_CLKEN_OFFSET) |
			BIT(BT_R_RX_SRCUP_CLKEN_OFFSET),
			BIT(BT_L_RX_SRCUP_CLKEN_OFFSET) |
			BIT(BT_R_RX_SRCUP_CLKEN_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/* disable s2 src12 and src13 uplink clk */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(BT_L_RX_SRCUP_CLKEN_OFFSET) |
			BIT(BT_R_RX_SRCUP_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("i2s2_src12_13_rx_switch_event event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s2_src_rx_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable s2 lr uplink src clk */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(BT_L_RX_SRCDN_CLKEN_OFFSET) |
			BIT(BT_R_RX_SRCDN_CLKEN_OFFSET) |
			BIT(BT_L_RX_SRCUP_CLKEN_OFFSET) |
			BIT(BT_R_RX_SRCUP_CLKEN_OFFSET),
			BIT(BT_L_RX_SRCDN_CLKEN_OFFSET) |
			BIT(BT_R_RX_SRCDN_CLKEN_OFFSET) |
			BIT(BT_L_RX_SRCUP_CLKEN_OFFSET) |
			BIT(BT_R_RX_SRCUP_CLKEN_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/* disable s2 lr uplink src clk */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(BT_L_RX_SRCDN_CLKEN_OFFSET) |
			BIT(BT_R_RX_SRCDN_CLKEN_OFFSET) |
			BIT(BT_L_RX_SRCUP_CLKEN_OFFSET) |
			BIT(BT_R_RX_SRCUP_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("i2s2_src_rx_event event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int i2s2_clk_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

	if (priv->board_type == FPGA_BOARD)
		return 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_separate_i2s_set_pinctrl_default_for_i2s2();
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_separate_i2s_set_pinctrl_idle_for_i2s2();
		break;
	default:
		AUDIO_LOGE("i2s2_clk_switch_event event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static const struct snd_kcontrol_new s2_mixer[] = {
	SOC_DAPM_SINGLE("S2_DACL_MIXER",
		BT_TX_MIXER2_CTRL_REG, BT_TX_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S2_DACR_MIXER",
		BT_TX_MIXER2_CTRL_REG, BT_TX_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const char * const i2s2_txl_mux_texts[] = {
	"S2_MIXER",
	"DACL_MIXER",
};

static const struct soc_enum i2s2_txl_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S2_TX_L_SRCDN_DIN_SEL_OFFSET,
		ARRAY_SIZE(i2s2_txl_mux_texts), i2s2_txl_mux_texts);

static const struct snd_kcontrol_new i2s2_txl_mux =
	SOC_DAPM_ENUM("Mux", i2s2_txl_mux_enum);

static const char * const i2s2_txr_mux_texts[] = {
	"S2_TXL",
	"DACR_MIXER",
};

static const struct soc_enum i2s2_txr_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S2_TX_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(i2s2_txr_mux_texts), i2s2_txr_mux_texts);

static const struct snd_kcontrol_new i2s2_txr_mux =
	SOC_DAPM_ENUM("Mux", i2s2_txr_mux_enum);

static const struct snd_kcontrol_new s2_src_i2s2_tx_switch[] = {
	SOC_DAPM_SINGLE("I2S2_SWITCH_EN", VIR0_ADDR, REG_BIT23, 1, 0),
};

static const struct snd_kcontrol_new s2_src12_13_rx_switch[] = {
	SOC_DAPM_SINGLE("I2S2_SRC12_13_RX_EN", VIR0_ADDR, REG_BIT24, 1, 0),
};

static const struct snd_kcontrol_new s2_src_rx_switch[] = {
	SOC_DAPM_SINGLE("I2S2_SRC_RX_EN", VIR0_ADDR, REG_BIT27, 1, 0),
};


static const struct snd_kcontrol_new i2s2_clk_switch[] = {
	SOC_DAPM_SINGLE("I2S2_CLK_EN", CODEC_CLK_EN1_REG,
		I2S2_PCM_CLKEN_OFFSET, 1, 0),
};

static const struct snd_soc_dapm_widget btfm_dapm_widgets[] = {
	SND_SOC_DAPM_PGA_E("I2S2_RX PGA", SND_SOC_NOPM, 0, 0, NULL, 0,
		i2s2_rx_pga_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_MIXER("S2_DOWNLINK_EN MIXER",
		CODEC_CLK_EN0_REG, BT_TX_MIXER2_CLKEN_OFFSET, 0,
		s2_mixer, ARRAY_SIZE(s2_mixer)),
	SND_SOC_DAPM_MUX("I2S2_TXL MUX", SND_SOC_NOPM, 0, 0, &i2s2_txl_mux),
	SND_SOC_DAPM_MUX("I2S2_TXR MUX", CODEC_CLK_EN3_REG,
		BT_TX_R_SRCDN_CLKEN_OFFSET, 0, &i2s2_txr_mux),
	SND_SOC_DAPM_SWITCH_E("S2_SRC_I2S2_TX SWITCH",
		SND_SOC_NOPM, 0, 0, s2_src_i2s2_tx_switch, s2_src_i2s2_tx_switch_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SWITCH_E("S2_SRC12_13_RX SWITCH",
		SND_SOC_NOPM, 0, 0, s2_src12_13_rx_switch, s2_src12_13_rx_switch_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SWITCH_E("S2_SRC_RX SWITCH",
		SND_SOC_NOPM, 0, 0, s2_src_rx_switch, s2_src_rx_switch_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SWITCH_E("I2S2_CLK SWITCH",
		SND_SOC_NOPM, 0, 0, i2s2_clk_switch, i2s2_clk_switch_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
};

#define I2S2_DOWNLINK \
	{ "S2_DOWNLINK_EN MIXER",    "S2_DACL_MIXER",    "DAC_DL MIXER" }, \
	{ "S2_DOWNLINK_EN MIXER",    "S2_DACR_MIXER",    "DAC_DR MIXER" }, \
	{ "I2S2_TXL MUX",            "S2_MIXER",         "S2_DOWNLINK_EN MIXER" }, \
	{ "I2S2_TXL MUX",            "DACL_MIXER",       "DAC_DL MIXER" }, \
	{ "I2S2_TXR MUX",            "DACR_MIXER",       "DAC_DR MIXER" }, \
	{ "I2S2_TXR MUX",            "S2_TXL",           "I2S2_TXL MUX" }, \
	{ "S2_SRC_I2S2_TX SWITCH",   "I2S2_SWITCH_EN",   "I2S2_TXL MUX" }, \
	{ "S2_SRC_I2S2_TX SWITCH",   "I2S2_SWITCH_EN",   "I2S2_TXR MUX" }, \
	{ "I2S2_CLK SWITCH",         "I2S2_CLK_EN",      "S2_SRC_I2S2_TX SWITCH" }, \
	{ "I2S1_2_3 OUTPUT",         NULL,               "I2S2_CLK SWITCH" }, \

#define I2S2_UPLINK \
	{ "I2S2_CLK SWITCH",         "I2S2_CLK_EN",      "I2S1_2_3 INPUT" }, \
	{ "I2S2_RX PGA",             NULL,               "I2S2_CLK SWITCH" }, \
	{ "VOICE_UL MUX",            "S2_L",             "I2S2_RX PGA" }, \
	{ "VOICE_UR MUX",            "S2_R",             "I2S2_RX PGA" }, \
	{ "DAC_DL MUX",              "S2_L",             "I2S2_RX PGA" }, \
	{ "DAC_DR MUX",              "S2_R",             "I2S2_RX PGA" }, \
	{ "DAC_DL MIXER",            "IN1_S2_L",         "I2S2_RX PGA" }, \
	{ "DAC_DR MIXER",            "IN1_S2_R",         "I2S2_RX PGA" }, \
	{ "AUDIO_UP_SRC5_EN PGA",    NULL,               "I2S2_RX PGA" }, \
	{ "AUDIO_UP_SRC6_EN PGA",    NULL,               "I2S2_RX PGA" }, \
	{ "AUDIO_UL MUX",            "S2_L_SRC",         "AUDIO_UP_SRC5_EN PGA" }, \
	{ "AUDIO_UR MUX",            "S2_R_SRC",         "AUDIO_UP_SRC6_EN PGA" }, \
	{ "S2_SRC12_13_RX SWITCH",   "I2S2_SRC12_13_RX_EN",   "I2S2_RX PGA" }, \
	{ "S2_SRC_RX SWITCH",        "I2S2_SRC_RX_EN",   "I2S2_RX PGA" }, \
	{ "VOICE_UL MUX",            "S2_L",             "S2_SRC_RX SWITCH" }, \
	{ "VOICE_UR MUX",            "S2_R",             "S2_SRC_RX SWITCH" }, \
	{ "DAC_DL MUX",              "S2_L",             "S2_SRC_RX SWITCH" }, \
	{ "DAC_DR MUX",              "S2_R",             "S2_SRC_RX SWITCH" }, \
	{ "DAC_DL MIXER",            "IN1_S2_L",         "S2_SRC_RX SWITCH" }, \
	{ "DAC_DR MIXER",            "IN1_S2_R",         "S2_SRC_RX SWITCH" }, \
	{ "AUDIO_UP_SRC5_EN PGA",    NULL,               "S2_SRC12_13_RX SWITCH" }, \
	{ "AUDIO_UP_SRC6_EN PGA",    NULL,               "S2_SRC12_13_RX SWITCH" }, \
	{ "AUDIO_UP_SRC5_EN PGA",    NULL,               "S2_SRC_RX SWITCH" }, \
	{ "AUDIO_UP_SRC6_EN PGA",    NULL,               "S2_SRC_RX SWITCH" }, \
	{ "AUDIO_UL MUX",            "S2_L_SRC",         "AUDIO_UP_SRC5_EN PGA" }, \
	{ "AUDIO_UR MUX",            "S2_R_SRC",         "AUDIO_UP_SRC6_EN PGA" }, \

static const struct snd_soc_dapm_route btfm_route_map[] = {
	I2S2_DOWNLINK
	I2S2_UPLINK
};

static struct snd_codec_resource btfm_resource = {
	btfm_controls,
	ARRAY_SIZE(btfm_controls),
	btfm_dapm_widgets,
	ARRAY_SIZE(btfm_dapm_widgets),
	btfm_route_map,
	ARRAY_SIZE(btfm_route_map),
};

int add_btfm(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	return da_separate_add_codec_resource(codec, &btfm_resource);
}
