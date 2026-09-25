/*
 * hi6555_smpa_path.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "hi6555_smpa_path.h"
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

#include "linux/hisi/audio_log.h"
#include "hi6555_type.h"
#include "hi6555_utils.h"

#define LOG_TAG "hi6555_smpa"
#define LUT_REG_INIT 269488144

#define I2S_KCONTROLS \
	SOC_SINGLE("FS_SPA1_UPLINK",           FS_CTRL3_REG,          FS_SPA1_UPLINK_OFFSET,            0x1, 0), \
	SOC_SINGLE("FS_SPA3_UPLINK",           FS_CTRL3_REG,          FS_SPA3_UPLINK_OFFSET,            0x1, 0), \
	SOC_SINGLE("FS_I2S1",                  I2S1_CTRL_REG,         FS_I2S1_OFFSET,                   0x7, 0), \
	SOC_SINGLE("I2S1_TX_CLK_SEL",          I2S1_CTRL_REG,         I2S1_TX_CLK_SEL_OFFSET,           0x1, 0), \
	SOC_SINGLE("I2S1_RX_CLK_SEL",          I2S1_CTRL_REG,         I2S1_RX_CLK_SEL_OFFSET,           0x1, 0), \
	SOC_SINGLE("I2S1_MST_SLV",             I2S1_CTRL_REG,         I2S1_MST_SLV_OFFSET,              0x1, 0), \
	SOC_SINGLE("I2S1_DIRECT_LOOP",         I2S1_CTRL_REG,         I2S1_DIRECT_LOOP_OFFSET,          0x3, 0), \
	SOC_SINGLE("I2S1_FUNC_MODE",           I2S1_CTRL_REG,         I2S1_FUNC_MODE_OFFSET,            0x7, 0), \
	SOC_SINGLE("I2S1_FRAME_MODE",          I2S1_CTRL_REG,         I2S1_FRAME_MODE_OFFSET,           0x1, 0), \
	SOC_SINGLE("I2S1_CODEC_IO_WORDLENGTH", I2S1_CTRL_REG,         I2S1_CODEC_IO_WORDLENGTH_OFFSET,  0x3, 0), \
	SOC_SINGLE("I2S1_TDM_TX_CLK_SEL",      I2S1_TDM_CTRL0_REG,    S1_TDM_TX_CLK_SEL_OFFSET,         0x1, 0), \
	SOC_SINGLE("I2S1_TDM_RX_CLK_SEL",      I2S1_TDM_CTRL0_REG,    S1_TDM_RX_CLK_SEL_OFFSET,         0x1, 0), \
	SOC_SINGLE("I2S1_TDM_MST_SLV",         I2S1_TDM_CTRL1_REG,    S1_TDM_MST_SLV_OFFSET,            0x1, 0), \
	SOC_SINGLE("I2S1_TDM_DIRECT_LOOP",     I2S1_TDM_CTRL0_REG,    S1_TDM_DIRECT_LOOP_OFFSET,        0x3, 0), \
	SOC_SINGLE("I2S1_TDM_FRAME_MODE",      I2S1_TDM_CTRL0_REG,    S1_TDM_FRAME_MODE_OFFSET,         0x7, 0), \

#define PGA_KCONTROLS \
	SOC_SINGLE("DACML_MIXER2_GAIN1",       DACML_MIXER2_CTRL_REG, DACML_MIXER2_GAIN1_OFFSET,        0x3,  0), \
	SOC_SINGLE("DACML_MIXER2_GAIN2",       DACML_MIXER2_CTRL_REG, DACML_MIXER2_GAIN2_OFFSET,        0x3,  0), \
	SOC_SINGLE("DACMR_MIXER2_GAIN1",       DACMR_MIXER2_CTRL_REG, DACMR_MIXER2_GAIN1_OFFSET,        0x3,  0), \
	SOC_SINGLE("DACMR_MIXER2_GAIN2",       DACMR_MIXER2_CTRL_REG, DACMR_MIXER2_GAIN2_OFFSET,        0x3,  0), \
	SOC_SINGLE("SPAV_ADC_CIC_GAIN",        SPAIV_ADC_FILTER_REG,  ADC_SPAV_CIC_GAIN_OFFSET,         0x3f, 0), \
	SOC_SINGLE("SPAI_ADC_CIC_GAIN",        SPAIV_ADC_FILTER_REG,  ADC_SPAI_CIC_GAIN_OFFSET,         0x3f, 0), \

#define CLASSH_KCONTROLS \
	SOC_SINGLE("CLASSH_BYPASS",        CLASSH_CTRL1_REG,   CLASSH_BYPASS_OFFSET,     0x1,    0), \
	SOC_SINGLE("CLASSH_FALL_DELAY",    CLASSH_CTRL1_REG,   FALL_DELAY_DF_CFG_OFFSET, 0x7fff, 0), \
	SOC_SINGLE("CLASSH_DO_CFG",        CLASSH_CTRL1_REG,   DO_CFG_OFFSET,            0xf,    0), \
	SOC_SINGLE("CLASSH_FALL_DELAY_EN", CLASSH_CTRL1_REG,   FALL_DELAY_EN_OFFSET,     0x1,    0), \
	SOC_SINGLE("CLASSH_DN_CFG",        CLASSH_CTRL1_REG,   DN_CFG_OFFSET,            0xf,    0), \
	SOC_SINGLE("CLASSH_FALL_STEP",     CLASSH_CTRL2_REG,   FALL_STEP_OFFSET,         0x3f,   0), \
	SOC_SINGLE("CLASSH_REG_VCTRL1",    CLASSH_CTRL2_REG,   REG_VCTRL1_OFFSET,        0xff,   0), \
	SOC_SINGLE("CLASSH_EN",            CLASSH_CTRL2_REG,   CLASSH_EN_OFFSET,         0x1,    0), \
	SOC_SINGLE("CLASSH_DYM_EN",        CLASSH_CTRL2_REG,   CLASSH_DYM_EN_OFFSET,     0x1,    0), \
	SOC_SINGLE("CLASSH_PGA_GAIN",      CLASSH_CTRL2_REG,   CLASSH_PGA_GAIN_OFFSET,   0x1ff,  0), \

#define LUT_KCONTROLS \
	SOC_SINGLE("LUT_REG0_3",    LUT_REG0_3_REG,     LUT_REG0_OFFSET,   0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG4_7",    LUT_REG4_7_REG,     LUT_REG4_OFFSET,   0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG8_11",   LUT_REG8_11_REG,    LUT_REG8_OFFSET,   0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG12_15",  LUT_REG12_15_REG,   LUT_REG12_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG16_19",  LUT_REG16_19_REG,   LUT_REG16_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG20_23",  LUT_REG20_23_REG,   LUT_REG20_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG24_27",  LUT_REG24_27_REG,   LUT_REG24_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG28_31",  LUT_REG28_31_REG,   LUT_REG28_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG32_35",  LUT_REG32_35_REG,   LUT_REG32_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG36_39",  LUT_REG36_39_REG,   LUT_REG36_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG40_43",  LUT_REG40_43_REG,   LUT_REG40_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG44_47",  LUT_REG44_47_REG,   LUT_REG44_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG48_51",  LUT_REG48_51_REG,   LUT_REG48_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG52_55",  LUT_REG52_55_REG,   LUT_REG52_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG56_59",  LUT_REG56_59_REG,   LUT_REG56_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG60_63",  LUT_REG60_63_REG,   LUT_REG60_OFFSET,  0x7fffffff,  0), \
	SOC_SINGLE("LUT_REG64_67",  LUT_REG64_67_REG,   LUT_REG64_OFFSET,  0x7fffffff,  0), \

void hisi_pa_asp_soc_reg_init(struct snd_soc_codec *codec)
{
	snd_soc_update_bits(codec, CLASSH_CTRL1_REG,
		BIT(CLASSH_BYPASS_OFFSET), BIT(CLASSH_BYPASS_OFFSET));
	snd_soc_update_bits(codec, CLASSH_CTRL1_REG,
		MASK_ON_BIT(DN_CFG_LEN, DN_CFG_OFFSET), 0x1);

	snd_soc_write(codec, LUT_REG0_3_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG4_7_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG8_11_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG12_15_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG16_19_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG20_23_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG24_27_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG28_31_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG32_35_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG36_39_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG40_43_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG44_47_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG48_51_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG52_55_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG56_59_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG60_63_REG, LUT_REG_INIT);
	snd_soc_write(codec, LUT_REG64_67_REG, LUT_REG_INIT);
}

static const struct snd_kcontrol_new smpa_controls[] = {
	I2S_KCONTROLS
	PGA_KCONTROLS
	CLASSH_KCONTROLS
	LUT_KCONTROLS
};

#ifdef CONFIG_SND_SOC_CODEC_HI6555V5
static int voice_dl_smpa2_pga_event(struct snd_soc_dapm_widget *w,
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
			BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET),
			BIT(VOICE_L_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(VOICE_L_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int voice_dr_smpa1_pga_event(struct snd_soc_dapm_widget *w,
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
			BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET),
			BIT(VOICE_R_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(VOICE_R_DN_PGA_CLKEN_OFFSET) |
			BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int smpa1_dacr_filter_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		snd_soc_update_bits(codec, SPA_SDM_CTRL_REG,
			BIT(SPA1_SDM_CALM_VLD_OFFSET), BIT(SPA1_SDM_CALM_VLD_OFFSET));
		snd_soc_update_bits(codec, CODEC_CLK_EN2_REG,
			BIT(SPA1_DAC_FILTER_CLKEN_OFFSET) | BIT(SPA1_UP16_CLKEN_OFFSET) | BIT(SPA1_SDM_CLKEN_OFFSET),
			BIT(SPA1_DAC_FILTER_CLKEN_OFFSET) | BIT(SPA1_UP16_CLKEN_OFFSET) | BIT(SPA1_SDM_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		snd_soc_update_bits(codec, CODEC_CLK_EN2_REG,
			BIT(SPA1_DAC_FILTER_CLKEN_OFFSET) | BIT(SPA1_UP16_CLKEN_OFFSET) | BIT(SPA1_SDM_CLKEN_OFFSET),
			0x0);
		snd_soc_update_bits(codec, SPA_SDM_CTRL_REG,
			BIT(SPA1_SDM_CALM_VLD_OFFSET), 0x0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}
#endif

static int adc_spai_filter_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ADC_SPI_CLKEN_OFFSET), BIT(ADC_SPI_CLKEN_OFFSET));
		/* ADC SPAI MUX */
		snd_soc_update_bits(codec, CODEC_DIN_MUX1_REG,
			BIT(SPAI_ADC_DIN_SEL_OFFSET), BIT(SPAI_ADC_DIN_SEL_OFFSET));
		/* SIF ADC_SPA enable */
		snd_soc_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_SPA_ADC_EN_OFFSET), BIT(SIF_SPA_ADC_EN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ADC_SPI_CLKEN_OFFSET), 0);
		/* ADC SPAI MUX */
		snd_soc_update_bits(codec, CODEC_DIN_MUX1_REG,
			BIT(SPAI_ADC_DIN_SEL_OFFSET), 0);
		/* SIF ADC_SPA disable */
		snd_soc_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_SPA_ADC_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int adc_spav_filter_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter clk enable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ADC_SPV_CLKEN_OFFSET), BIT(ADC_SPV_CLKEN_OFFSET));
		/* ADC SPAV MUX */
		snd_soc_update_bits(codec, CODEC_DIN_MUX1_REG,
			BIT(SPAV_ADC_DIN_SEL_OFFSET), 0);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter clk disable */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(ADC_SPV_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int i2s1_i2s_tx_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable i2s1 mode */
		snd_soc_update_bits(codec, I2S1_TDM_CTRL1_REG,
			BIT(S1_I2S_TDM_MODE_OFFSET), 0);
		/* enable i2s1 tx */
		snd_soc_update_bits(codec, I2S1_CTRL_REG,
			BIT(I2S1_IF_TX_ENA_OFFSET), BIT(I2S1_IF_TX_ENA_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* disable i2s1 tx */
		snd_soc_update_bits(codec, I2S1_CTRL_REG,
			BIT(I2S1_IF_TX_ENA_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("i2s1_clk_i2s_tx_switch event err: %d", event);
		break;
	}

	return 0;
}

static int i2s1_i2s_rx_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable i2s1 rx */
		snd_soc_update_bits(codec, I2S1_CTRL_REG,
			BIT(I2S1_IF_RX_ENA_OFFSET), BIT(I2S1_IF_RX_ENA_OFFSET));

		/* enable smartpa feedback SPA1 SPA3 */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(SPA1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA3_UP_AFIFO_CLKEN_OFFSET),
			BIT(SPA1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA3_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* disable smartpa feedback SPA1 SPA3 */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(SPA1_UP_AFIFO_CLKEN_OFFSET) |
			BIT(SPA3_UP_AFIFO_CLKEN_OFFSET), 0);

		/* disable i2s1 rx */
		snd_soc_update_bits(codec, I2S1_CTRL_REG,
			BIT(I2S1_IF_RX_ENA_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("i2s1_i2s_rx_switch event err: %d", event);
		break;
	}

	return 0;
}

static int i2s1_tdm_rx_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable smartpa feedback SPA1 SPA2 SPA3 SPA4 fifo switch */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(SPA1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA2_UP_AFIFO_CLKEN_OFFSET) |
			BIT(SPA3_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA4_UP_AFIFO_CLKEN_OFFSET),
			BIT(SPA1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA2_UP_AFIFO_CLKEN_OFFSET) |
			BIT(SPA3_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA4_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* disable smartpa feedback SPA1 SPA2 SPA3 SPA4 fifo switch */
		snd_soc_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(SPA1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA2_UP_AFIFO_CLKEN_OFFSET) |
			BIT(SPA3_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA4_UP_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("i2s1_i2s_rx_switch event err: %d", event);
		break;
	}

	return 0;
}

static int i2s1_tdm_tx_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable tdm mode */
		snd_soc_update_bits(codec, I2S1_TDM_CTRL1_REG,
			BIT(S1_I2S_TDM_MODE_OFFSET), BIT(S1_I2S_TDM_MODE_OFFSET));

		/* enable tdm tx */
		snd_soc_update_bits(codec, I2S1_TDM_CTRL0_REG,
			BIT(S1_TDM_IF_EN_OFFSET), BIT(S1_TDM_IF_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* disable tdm tx */
		snd_soc_update_bits(codec, I2S1_TDM_CTRL0_REG,
			BIT(S1_TDM_IF_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("i2s1_clk_tdm_tx_switch_event event err: %d", event);
		break;
	}

	return 0;
}

static const struct snd_kcontrol_new dacml_mixer2[] = {
	SOC_DAPM_SINGLE("VOICE_DL",
		DACML_MIXER2_CTRL_REG, DACML_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("ULTR_D",
		DACML_MIXER2_CTRL_REG, DACML_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dacmr_mixer2[] = {
	SOC_DAPM_SINGLE("VOICE_DR",
		DACMR_MIXER2_CTRL_REG, DACMR_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("ULTR_D",
		DACMR_MIXER2_CTRL_REG, DACMR_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const char *const i2s1_l_mux_texts[] = {
	"ADCL",
	"DACL_MIXER",
	"DACML_MIXER2",
};

static const struct soc_enum i2s1_l_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S1_PCM_TX_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(i2s1_l_mux_texts), i2s1_l_mux_texts);

static const struct snd_kcontrol_new i2s1_l_mux =
	SOC_DAPM_ENUM("Mux", i2s1_l_mux_enum);

static const char *const i2s1_r_mux_texts[] = {
	"ADCR",
	"DACR_MIXER",
	"DACMR_MIXER2",
};

static const struct soc_enum i2s1_r_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S1_PCM_TX_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(i2s1_r_mux_texts), i2s1_r_mux_texts);

static const struct snd_kcontrol_new i2s1_r_mux =
	SOC_DAPM_ENUM("Mux", i2s1_r_mux_enum);

static const struct snd_kcontrol_new i2s1_i2s_tx_switch[] = {
	SOC_DAPM_SINGLE("I2S_SWITCH_EN", VIR0_ADDR, REG_BIT21, 1, 0),
};

static const struct snd_kcontrol_new i2s1_tdm_tx_switch[] = {
	SOC_DAPM_SINGLE("TDM_SWITCH_EN", VIR0_ADDR, REG_BIT22, 1, 0),
};
static const struct snd_kcontrol_new i2s1_i2s_rx_switch[] = {
	SOC_DAPM_SINGLE("I2S1_I2S_RX_EN", VIR0_ADDR, REG_BIT25, 1, 0),
};

static const struct snd_kcontrol_new i2s1_tdm_rx_switch[] = {
	SOC_DAPM_SINGLE("I2S1_TDM_RX_EN", VIR0_ADDR, REG_BIT26, 1, 0),
};

static const struct snd_kcontrol_new i2s1_clk_switch[] = {
	SOC_DAPM_SINGLE("I2S1_CLK_EN",
		CODEC_CLK_EN1_REG, I2S1_TDM_CLKEN_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new dac_spa_switch[] = {
	SOC_DAPM_SINGLE("DAC_SPA_EN",
		DAC_FILTER_CTRL_REG, SIF_SPA_DAC_EN_OFFSET, 1, 0),
};

#ifdef CONFIG_SND_SOC_CODEC_HI6555V5
static const struct snd_kcontrol_new smpa1_virtual_switch[] = {
	SOC_DAPM_SINGLE("HISI_SMPA1_EN",
		VIR1_ADDR, REG_BIT30, 1, 0),
};

static const struct snd_kcontrol_new smpa2_virtual_switch[] = {
	SOC_DAPM_SINGLE("HISI_SMPA2_EN",
		VIR1_ADDR, REG_BIT31, 1, 0),
};
#endif

static const struct snd_soc_dapm_widget smpa_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("HISI_SMPA_DLINK INPUT"),
	SND_SOC_DAPM_INPUT("ADC_SPA INPUT"),
	SND_SOC_DAPM_INPUT("I2S1_2_3 INPUT"),
	SND_SOC_DAPM_OUTPUT("DACL_SPA OUTPUT"),
	SND_SOC_DAPM_OUTPUT("DACR_SPA OUTPUT"),
	SND_SOC_DAPM_OUTPUT("I2S1_2_3 OUTPUT"),
	SND_SOC_DAPM_PGA_E("ADC_SPAI_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,
		adc_spai_filter_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_PGA_E("ADC_SPAV_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,
		adc_spav_filter_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_MIXER("DACML_EN MIXER", CODEC_CLK_EN3_REG,
		DACML_MIXER2_CLKEN_OFFSET, 0, dacml_mixer2, ARRAY_SIZE(dacml_mixer2)),
	SND_SOC_DAPM_MIXER("DACMR_EN MIXER", CODEC_CLK_EN3_REG,
		DACMR_MIXER2_CLKEN_OFFSET, 0, dacmr_mixer2, ARRAY_SIZE(dacmr_mixer2)),
	SND_SOC_DAPM_MUX("I2S1_S1L MUX", SND_SOC_NOPM, 0, 0, &i2s1_l_mux),
	SND_SOC_DAPM_MUX("I2S1_S1R MUX", SND_SOC_NOPM, 0, 0, &i2s1_r_mux),
	SND_SOC_DAPM_SWITCH_E("I2S1_I2S_TX SWITCH",
		SND_SOC_NOPM, 0, 0, i2s1_i2s_tx_switch, i2s1_i2s_tx_switch_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SWITCH_E("I2S1_TDM_TX SWITCH",
		SND_SOC_NOPM, 0, 0, i2s1_tdm_tx_switch, i2s1_tdm_tx_switch_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SWITCH("I2S1_CLK SWITCH", SND_SOC_NOPM, 0, 0, i2s1_clk_switch),
	SND_SOC_DAPM_SWITCH_E("I2S1_I2S_RX SWITCH",
		SND_SOC_NOPM, 0, 0, i2s1_i2s_rx_switch, i2s1_i2s_rx_switch_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SWITCH_E("I2S1_TDM_RX SWITCH",
		SND_SOC_NOPM, 0, 0, i2s1_tdm_rx_switch, i2s1_tdm_rx_switch_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SWITCH("DAC_SPA SWITCH", SND_SOC_NOPM, 0, 0, dac_spa_switch),
#ifdef CONFIG_SND_SOC_CODEC_HI6555V5
	SND_SOC_DAPM_PGA_E("VOICE_DL_SMPA2 PGA", SND_SOC_NOPM, 0, 0, NULL, 0, \
		voice_dl_smpa2_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_PGA_E("VOICE_DR_SMPA1 PGA", SND_SOC_NOPM, 0, 0, NULL, 0,
		voice_dr_smpa1_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_PGA_E("HISI_SMPA1_DACR_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0, \
		smpa1_dacr_filter_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),
	SND_SOC_DAPM_SWITCH("HISI_SMPA1 SWITCH", SND_SOC_NOPM, 0, 0, smpa1_virtual_switch),
	SND_SOC_DAPM_SWITCH("HISI_SMPA2 SWITCH", SND_SOC_NOPM, 0, 0, smpa2_virtual_switch),
#endif
};

#define SMPA_ECHO_UPLINK \
	{ "SIF_EN MIXER",        "SIF_ADC_SPA_EN",  "ADC_SPA INPUT" }, \
	{ "ADC_SPAI_FILTER",     NULL,              "SIF_EN MIXER" }, \
	{ "ADC_SPAV_FILTER",     NULL,              "SIF_EN MIXER" }, \
	{ "ECHO_L MUX",          "ADC_SPAI",        "ADC_SPAI_FILTER" }, \
	{ "ECHO_R MUX",          "ADC_SPAV",        "ADC_SPAV_FILTER" }, \

#define SMPA_DOWNLINK \
	{ "DAC_SPA SWITCH",  "DAC_SPA_EN",      "DACL_FILTER" }, \
	{ "SIF_EN MIXER",    "SIF_DAC_SPA_EN",  "DAC_SPA SWITCH" }, \
	{ "DACL_SPA OUTPUT",  NULL,              "SIF_EN MIXER" }, \

#define HISI_SMPA_DOWNLINK \
	{ "VOICE_DR_SMPA1 PGA",     NULL,             "HISI_SMPA_DLINK INPUT" }, \
	{ "VOICE_DL_SMPA2 PGA",     NULL,             "HISI_SMPA_DLINK INPUT" }, \
	{ "HISI_SMPA2 SWITCH",      "HISI_SMPA2_EN",  "VOICE_DL_SMPA2 PGA" }, \
	{ "DACL_SPA OUTPUT",        NULL,             "HISI_SMPA2 SWITCH" }, \
	{ "DAC_SPA SWITCH",         "DAC_SPA_EN",     "VOICE_DR_SMPA1 PGA" }, \
	{ "SIF_EN MIXER",           "SIF_DAC_SPA_EN", "DAC_SPA SWITCH" }, \
	{ "HISI_SMPA1 SWITCH",      "HISI_SMPA1_EN",  "SIF_EN MIXER" }, \
	{ "HISI_SMPA1_DACR_FILTER", NULL,             "HISI_SMPA1 SWITCH" }, \
	{ "DACR_SPA OUTPUT",        NULL,             "HISI_SMPA1_DACR_FILTER" }, \

#define SMPA_I2S1_PATH \
	{ "I2S1_S1L MUX",         "ADCL",            "ADCL_FILTER" }, \
	{ "I2S1_S1R MUX",         "ADCR",            "ADCR_FILTER" }, \
	{ "I2S1_S1L MUX",         "DACL_MIXER",      "DAC_DL MIXER" }, \
	{ "I2S1_S1R MUX",         "DACR_MIXER",      "DAC_DR MIXER" }, \
	{ "I2S1_I2S_TX SWITCH",   "I2S_SWITCH_EN",   "I2S1_S1L MUX" }, \
	{ "I2S1_I2S_TX SWITCH",   "I2S_SWITCH_EN",   "I2S1_S1R MUX" }, \
	{ "I2S1_CLK SWITCH",      "I2S1_CLK_EN",     "I2S1_I2S_TX SWITCH" }, \
	{ "I2S1_2_3 OUTPUT",      NULL,              "I2S1_CLK SWITCH" }, \
	{ "DACML_EN MIXER",       "ULTR_D",          "ULTR_DL MUX" }, \
	{ "DACML_EN MIXER",       "VOICE_DL",        "VOICE_DL PGA" }, \
	{ "I2S1_S1L MUX",         "DACML_MIXER2",    "DACML_EN MIXER" }, \
	{ "DACMR_EN MIXER",       "ULTR_D",          "ULTR_DL MUX" }, \
	{ "DACMR_EN MIXER",       "VOICE_DR",        "VOICE_DR PGA" }, \
	{ "I2S1_S1R MUX",         "DACMR_MIXER2",    "DACMR_EN MIXER" }, \
	{ "I2S1_TDM_TX SWITCH",   "TDM_SWITCH_EN",   "I2S1_S1L MUX" }, \
	{ "I2S1_TDM_TX SWITCH",   "TDM_SWITCH_EN",   "I2S1_S1R MUX" }, \
	{ "I2S1_CLK SWITCH",      "I2S1_CLK_EN",     "I2S1_TDM_TX SWITCH" }, \
	{ "I2S1_2_3 OUTPUT",      NULL,              "I2S1_CLK SWITCH" }, \
	{ "I2S1_I2S_RX SWITCH",   "I2S1_I2S_RX_EN",  "I2S1_2_3 INPUT" }, \
	{ "I2S1_CLK SWITCH",      "I2S1_CLK_EN",     "I2S1_I2S_RX SWITCH" }, \
	{ "I2S1_2_3 OUTPUT",      NULL,              "I2S1_CLK SWITCH" }, \
	{ "I2S1_TDM_RX SWITCH",   "I2S1_TDM_RX_EN",  "I2S1_2_3 INPUT" }, \
	{ "I2S1_CLK SWITCH",      "I2S1_CLK_EN",     "I2S1_TDM_RX SWITCH" }, \
	{ "I2S1_2_3 OUTPUT",      NULL,              "I2S1_CLK SWITCH" }, \

static const struct snd_soc_dapm_route smpa_route_map[] = {
	SMPA_ECHO_UPLINK
	SMPA_DOWNLINK
	SMPA_I2S1_PATH
#ifdef CONFIG_SND_SOC_CODEC_HI6555V5
	HISI_SMPA_DOWNLINK
#endif
};

static struct snd_codec_resource smpa_resource = {
	smpa_controls,
	ARRAY_SIZE(smpa_controls),
	smpa_dapm_widgets,
	ARRAY_SIZE(smpa_dapm_widgets),
	smpa_route_map,
	ARRAY_SIZE(smpa_route_map),
};

int add_smpa(struct snd_soc_codec *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	return hi6555_add_codec_resource(codec, &smpa_resource);
}
