/*
 * da_separate_third_codec_poath.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "da_separate_third_codec_path.h"
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

#include "linux/hisi/audio_log.h"
#include "da_separate_type.h"
#include "da_separate_utils.h"

#define LOG_TAG "DA_separate_third"

static const struct snd_kcontrol_new third_codec_controls[] = {
	SOC_SINGLE("FS_3RDCODEC_L_DLINK",       FS_CTRL2_REG,              FS_CODEC3_L_DLINK_OFFSET,        0x7, 0),
	SOC_SINGLE("FS_3RDCODEC_R_DLINK",       FS_CTRL2_REG,              FS_CODEC3_R_DLINK_OFFSET,        0x7, 0),
	SOC_SINGLE("FS_I2S3",                   I2S3_PCM_CTRL_REG,         FS_I2S3_OFFSET,                  0x7, 0),
	SOC_SINGLE("I2S3_TX_CLK_SEL",           I2S3_PCM_CTRL_REG,         I2S3_TX_CLK_SEL_OFFSET,          0x1, 0),
	SOC_SINGLE("I2S3_RX_CLK_SEL",           I2S3_PCM_CTRL_REG,         I2S3_RX_CLK_SEL_OFFSET,          0x1, 0),
	SOC_SINGLE("I2S3_MST_SLV",              I2S3_PCM_CTRL_REG,         I2S3_MST_SLV_OFFSET,             0x1, 0),
	SOC_SINGLE("I2S3_DIRECT_LOOP",          I2S3_PCM_CTRL_REG,         I2S3_DIRECT_LOOP_OFFSET,         0x3, 0),
	SOC_SINGLE("I2S3_FUNC_MODE",            I2S3_PCM_CTRL_REG,         I2S3_FUNC_MODE_OFFSET,           0x7, 0),
	SOC_SINGLE("I2S3_FRAME_MODE",           I2S3_PCM_CTRL_REG,         I2S3_FRAME_MODE_OFFSET,          0x1, 0),
	SOC_SINGLE("I2S3_CODEC_IO_WORDLENGTH",  I2S3_PCM_CTRL_REG,         I2S3_CODEC_IO_WORDLENGTH_OFFSET, 0x3, 0),
	SOC_SINGLE("CODEC3_L_DN_PGA_SRC",       FS_CTRL3_REG,              FS_CODEC3_L_DN_PGA_OFFSET,       0x7, 0),
	SOC_SINGLE("CODEC3_R_DN_PGA_SRC",       FS_CTRL3_REG,              FS_CODEC3_R_DN_PGA_OFFSET,       0x7, 0),
	SOC_SINGLE("CODEC3_L_DN_PGA_BYPASS",    CODEC3_L_DN_PGA_CTRL_REG,  CODEC3_L_DN_PGA_BYPASS_OFFSET,   0x1, 0),
	SOC_SINGLE("CODEC3_L_DN_PGA",           CODEC3_L_DN_PGA_CTRL_REG,  CODEC3_L_DN_PGA_GAIN_OFFSET,     0xff, 0),
	SOC_SINGLE("CODEC3_R_DN_PGA_BYPASS",    CODEC3_R_DN_PGA_CTRL_REG,  CODEC3_R_DN_PGA_BYPASS_OFFSET,   0x1, 0),
	SOC_SINGLE("CODEC3_R_DN_PGA",           CODEC3_R_DN_PGA_CTRL_REG,  CODEC3_R_DN_PGA_GAIN_OFFSET,     0xff, 0),

};

static int third_codec_dl_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable 3rd codec dlr fifo */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG, BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG, BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET), BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET));
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET), BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET));
		/* enable pga */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(CODEC3_L_DN_PGA_CLKEN_OFFSET), BIT(CODEC3_L_DN_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(CODEC3_R_DN_PGA_CLKEN_OFFSET), BIT(CODEC3_R_DN_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* disable 3rd codec dlr fifo */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG, BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG, BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET), 0);
		/* disable pga */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG, BIT(CODEC3_L_DN_PGA_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG, BIT(CODEC3_R_DN_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("3rd dlr fifo i2s3 tx switch event err: %d", event);
		break;
	}

	return 0;
}

static const char * const i2s3_s1r_mux_texts[] = {
	"THIRD_CODEC_DL",
	"ADC_ULTR",
};

static const struct soc_enum i2s3_s1r_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S3_TX_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(i2s3_s1r_mux_texts), i2s3_s1r_mux_texts);

static const struct snd_kcontrol_new i2s3_s1r_mux =
	SOC_DAPM_ENUM("Mux", i2s3_s1r_mux_enum);


static const struct snd_kcontrol_new i2s3_rx_switch[] = {
	SOC_DAPM_SINGLE("I2S3_RX_EN", I2S3_PCM_CTRL_REG,
		I2S3_IF_RX_ENA_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new i2s3_tx_switch[] = {
	SOC_DAPM_SINGLE("I2S3_TX_EN", I2S3_PCM_CTRL_REG,
		I2S3_IF_TX_ENA_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new i2s3_clk_switch[] = {
	SOC_DAPM_SINGLE("I2S3_CLK_EN", CODEC_CLK_EN1_REG,
		I2S3_PCM_CLKEN_OFFSET, 1, 0),
};

static const struct snd_soc_dapm_widget third_codec_dapm_widgets[] = {
	SND_SOC_DAPM_PGA_E("THIRD_CODEC_DL PGA", SND_SOC_NOPM, 0, 0, NULL, 0,
		third_codec_dl_pga_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_MUX("I2S3_S1R MUX", SND_SOC_NOPM, 0, 0, &i2s3_s1r_mux),
	SND_SOC_DAPM_SWITCH("I2S3_CLK SWITCH", SND_SOC_NOPM, 0, 0, i2s3_clk_switch),
	SND_SOC_DAPM_SWITCH("I2S3_RX SWITCH", SND_SOC_NOPM, 0, 0, i2s3_rx_switch),
	SND_SOC_DAPM_SWITCH("I2S3_TX SWITCH", SND_SOC_NOPM, 0, 0, i2s3_tx_switch),
};

#define THIRD_CODEC_I2S3_DOWNLINK \
	{ "THIRD_CODEC_DL PGA",    NULL,                "I2S1_2_3 INPUT" }, \
	{ "I2S3_S1R MUX",          "THIRD_CODEC_DL",    "THIRD_CODEC_DL PGA" }, \
	{ "I2S3_S1R MUX",          "ADC_ULTR",          "ADC_ULTR_FILTER" }, \
	{ "I2S3_TX SWITCH",        "I2S3_TX_EN",        "I2S3_S1R MUX" }, \
	{ "I2S3_CLK SWITCH",       "I2S3_CLK_EN",       "I2S3_TX SWITCH" }, \
	{ "I2S1_2_3 OUTPUT",       NULL,                "I2S3_CLK SWITCH" }, \

#define THIRD_CODEC_I2S1_DOWNLINK \
	{ "THIRD_CODEC_DL PGA",    NULL,               "I2S1_2_3 INPUT" }, \
	{ "AUDIO_DN PGA",          NULL,               "I2S1_2_3 INPUT" }, \
	{ "I2S1_TDM_TX SWITCH",    "TDM_SWITCH_EN",    "AUDIO_DN PGA" }, \
	{ "I2S1_TDM_TX SWITCH",    "TDM_SWITCH_EN",    "THIRD_CODEC_DL PGA" }, \

#define THIRD_CODEC_I2S3_UPLINK \
	{ "I2S3_RX SWITCH",        "I2S3_RX_EN",       "I2S1_2_3 INPUT" }, \
	{ "I2S3_CLK SWITCH",       "I2S3_CLK_EN",      "I2S3_RX SWITCH" }, \
	{ "ECHO_L MUX",            "S3_L",             "I2S3_CLK SWITCH" }, \
	{ "ECHO_R MUX",            "S3_R",             "I2S3_CLK SWITCH" }, \

static const struct snd_soc_dapm_route third_codec_route_map[] = {
	THIRD_CODEC_I2S3_DOWNLINK
	THIRD_CODEC_I2S1_DOWNLINK
	THIRD_CODEC_I2S3_UPLINK
};

static struct snd_codec_resource third_codec_resource = {
	third_codec_controls,
	ARRAY_SIZE(third_codec_controls),
	third_codec_dapm_widgets,
	ARRAY_SIZE(third_codec_dapm_widgets),
	third_codec_route_map,
	ARRAY_SIZE(third_codec_route_map),
};

int add_third_codec(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	return da_separate_add_codec_resource(codec, &third_codec_resource);
}
