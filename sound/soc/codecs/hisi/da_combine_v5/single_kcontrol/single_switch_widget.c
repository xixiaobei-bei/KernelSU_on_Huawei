/*
 * single_switch_widget.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "single_switch_widget.h"

#include <sound/core.h>
#include <sound/tlv.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_v5.h"
#include "linux/hisi/da_combine/da_combine_v5_regs.h"
#include "linux/hisi/da_combine/da_combine_v5_type.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include <linux/hisi/da_combine/da_combine_mbhc_rear_jack.h>

#include "slimbus.h"
#include "slimbus_da_combine_v5.h"

#include "switch_widget.h"
#include "switch_widget_utils.h"
#include "path_widget.h"
#include "resource_widget.h"
#include "resource_widget.h"
#include "pga_widget.h"


#define GPIO_LEVEL_HIGH        1
#define GPIO_LEVEL_LOW         0
#define PA_2_IV_PARAM_CHANNEL  2

static const struct snd_kcontrol_new dapm_speaker_playbcak_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, SPEAKER_PLAYBACK_SW_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_headset_playback_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, HEADSET_PLAYBACK_SW_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_headset_mic_capture_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, HEADSET_MIC_CAPTURE_SW_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_mic_capture_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, MIC_CAPTURE_SW_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_bt_playback_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, BT_PLAYBACK_SW_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_bt_playback_wb_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, BT_PLAYBACK_WB_SW_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_bt_capture_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, BT_CAPTURE_SW_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_bt_capture_wb_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, BT_CAPTURE_WB_SW_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_rear_linein_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, REAR_LINE_IN_CAPTURE_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_rear_head_mic_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, REAR_HEAD_MIC_CAPTURE_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_rear_headphone_switch_controls =
	SOC_DAPM_SINGLE("Switch", VIRTUAL_PC_DOWN_REG, REAR_HEADPHONE_PLAYBACK_BIT, 1, 0);

static void speaker_playback_enable(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	/* "M1_L_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL32_REG,
		MASK_ON_BIT(M1L_DATA_SEL_LEN, M1L_DATA_SEL_OFFSET),
		0x3 << M1L_DATA_SEL_OFFSET);
	/* "M1_R_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL32_REG,
		MASK_ON_BIT(M1R_DATA_SEL_LEN, M1R_DATA_SEL_OFFSET),
		0x3 << M1R_DATA_SEL_OFFSET);
	/* DSP IV */
	iv_dspif_switch_enable(codec);
	/* "S4_IF_I2S_FS" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL24_REG,
		MASK_ON_BIT(FS_S4_LEN, FS_S4_OFFSET),
		0x4 << FS_S4_OFFSET);
	/* "S4_TX_CLK_SEL" */
	da_combine_update_bits(codec, SC_S4_IF_H_REG,
		MASK_ON_BIT(S4_TX_CLK_SEL_LEN, S4_TX_CLK_SEL_OFFSET),
		BIT(S4_TX_CLK_SEL_OFFSET));
	/* "S4_FS_TDM" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL24_REG,
		MASK_ON_BIT(S4_FS_TDM_LEN, S4_FS_TDM_OFFSET),
		0x2 << S4_FS_TDM_OFFSET);

	platform_data->route_state |= SPK_ROUTE;

	AUDIO_LOGD("end");
	return;
}

static void speaker_playback_disable(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	/* "M1_L_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL32_REG,
		MASK_ON_BIT(M1L_DATA_SEL_LEN, M1L_DATA_SEL_OFFSET), 0x0);
	/* "M1_R_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL32_REG,
		MASK_ON_BIT(M1R_DATA_SEL_LEN, M1R_DATA_SEL_OFFSET), 0x0);

	/* DSP IV */
	iv_dspif_switch_disable(codec);

	/* "S4_IF_I2S_FS" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL24_REG,
		MASK_ON_BIT(FS_S4_LEN, FS_S4_OFFSET), 0x0);
	/* "S4_TX_CLK_SEL" */
	da_combine_update_bits(codec, SC_S4_IF_H_REG,
		MASK_ON_BIT(S4_TX_CLK_SEL_LEN, S4_TX_CLK_SEL_OFFSET), 0x0);
	/* "S4_FS_TDM" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL24_REG,
		MASK_ON_BIT(S4_FS_TDM_LEN, S4_FS_TDM_OFFSET), 0x0);

	platform_data->route_state &= ~SPK_ROUTE;

	AUDIO_LOGD("end");
	return;
}


static int speaker_playback_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		speaker_playback_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		speaker_playback_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static void headset_playback_enable(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	/* "DACL_POST_MIXER" */
	da_combine_update_bits(codec, DACL_POST_MIXER2_CTRL0_REG,
		MASK_ON_BIT(DACL_POST_MIXER2_IN1_MUTE_LEN, DACL_POST_MIXER2_IN1_MUTE_OFFSET), 0x0);
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(DACL_POST_MIXER2_CLK_EN_LEN, DACL_POST_MIXER2_CLK_EN_OFFSET),
		BIT(DACL_POST_MIXER2_CLK_EN_OFFSET));
	dacl_post_mixer_enable(codec);
	/* "DACR_POST_MIXER" */
	da_combine_update_bits(codec, DACR_POST_MIXER2_CTRL0_REG,
		MASK_ON_BIT(DACR_POST_MIXER2_IN1_MUTE_LEN, DACR_POST_MIXER2_IN1_MUTE_OFFSET), 0x0);
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(DACR_POST_MIXER2_CLK_EN_LEN, DACR_POST_MIXER2_CLK_EN_OFFSET),
		BIT(DACR_POST_MIXER2_CLK_EN_OFFSET));
	dacr_post_mixer_enable(codec);
	/* "DACL_PGA_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL12_REG,
		MASK_ON_BIT(DACL_PRE_PGA_DIN_SEL_LEN, DACL_PRE_PGA_DIN_SEL_OFFSET), 0x0);
	dacl_pga_mux_enable(codec);
	/* "DACR_PGA_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL12_REG,
		MASK_ON_BIT(DACR_PRE_PGA_DIN_SEL_LEN, DACR_PRE_PGA_DIN_SEL_OFFSET), 0x0);
	dacr_pga_mux_enable(codec);
	/* "HP_L_SDM_MUX" */
	da_combine_update_bits(codec, DAC_DP_CLK_EN_2_REG,
		MASK_ON_BIT(HP_SDM_L_CLK_EN_LEN, HP_SDM_L_CLK_EN_OFFSET),
		BIT(HP_SDM_L_CLK_EN_OFFSET));
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL13_REG,
		MASK_ON_BIT(HP_SDM_L_DIN_SEL_LEN, HP_SDM_L_DIN_SEL_OFFSET), 0x0);
	/* "HP_R_SDM_MUX" */
	da_combine_update_bits(codec, DAC_DP_CLK_EN_2_REG,
		MASK_ON_BIT(HP_SDM_R_CLK_EN_LEN, HP_SDM_R_CLK_EN_OFFSET),
		BIT(HP_SDM_R_CLK_EN_OFFSET));
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL13_REG,
		MASK_ON_BIT(HP_SDM_R_DIN_SEL_LEN, HP_SDM_R_DIN_SEL_OFFSET), 0x0);
	/* "HP_DAC_L_MUX" */
	da_combine_update_bits(codec, DAC_DATA_SEL_CTRL0_REG,
		MASK_ON_BIT(DAC0_DATA_SEL_LEN, DAC0_DATA_SEL_OFFSET), 0x0);
	/* "HP_DAC_R_MUX" */
	da_combine_update_bits(codec, DAC_DATA_SEL_CTRL0_REG,
		MASK_ON_BIT(DAC1_DATA_SEL_LEN, DAC1_DATA_SEL_OFFSET), 0x0);
	/* "HP_HIGH_LEVEL" */
	da_combine_update_bits(codec, VIRTUAL_DOWN_REG, BIT(HP_HIGH_BIT), BIT(HP_HIGH_BIT));
	hp_high_level_enable(codec);

	platform_data->route_state |= HEADSET_PLAY_ROUTE;

	AUDIO_LOGD("end");
}

static void headset_playback_disable(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	/* "DACL_POST_MIXER" */
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(DACL_POST_MIXER2_CLK_EN_LEN, DACL_POST_MIXER2_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, DACL_POST_MIXER2_CTRL0_REG,
		MASK_ON_BIT(DACL_POST_MIXER2_IN1_MUTE_LEN, DACL_POST_MIXER2_IN1_MUTE_OFFSET),
		BIT(DACL_POST_MIXER2_IN1_MUTE_OFFSET));
	dacl_post_mixer_disable(codec);
	/* "DACR_POST_MIXER" */
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(DACR_POST_MIXER2_CLK_EN_LEN, DACR_POST_MIXER2_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, DACR_POST_MIXER2_CTRL0_REG,
		MASK_ON_BIT(DACR_POST_MIXER2_IN1_MUTE_LEN, DACR_POST_MIXER2_IN1_MUTE_OFFSET),
		BIT(DACR_POST_MIXER2_IN1_MUTE_OFFSET));
	dacr_post_mixer_disable(codec);
	/* "DACL_PGA_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL12_REG,
		MASK_ON_BIT(DACR_PRE_PGA_DIN_SEL_LEN, DACR_PRE_PGA_DIN_SEL_OFFSET), 0x0);
	dacl_pga_mux_disable(codec);
	/* "DACR_PGA_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL12_REG,
		MASK_ON_BIT(DACR_PRE_PGA_DIN_SEL_LEN, DACR_PRE_PGA_DIN_SEL_OFFSET), 0x0);
	dacr_pga_mux_disable(codec);
	/* "HP_L_SDM_MUX" */
	da_combine_update_bits(codec, DAC_DP_CLK_EN_2_REG,
		MASK_ON_BIT(HP_SDM_L_CLK_EN_LEN, HP_SDM_L_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL13_REG,
		MASK_ON_BIT(HP_SDM_L_DIN_SEL_LEN, HP_SDM_L_DIN_SEL_OFFSET), 0x0);
	/* "HP_R_SDM_MUX" */
	da_combine_update_bits(codec, DAC_DP_CLK_EN_2_REG,
		MASK_ON_BIT(HP_SDM_R_CLK_EN_LEN, HP_SDM_R_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL13_REG,
		MASK_ON_BIT(HP_SDM_L_DIN_SEL_LEN, HP_SDM_L_DIN_SEL_OFFSET), 0x0);
	/* "HP_DAC_L_MUX" */
	da_combine_update_bits(codec, DAC_DATA_SEL_CTRL0_REG,
		MASK_ON_BIT(DAC0_DATA_SEL_LEN, DAC0_DATA_SEL_OFFSET), 0x0);
	/* "HP_DAC_R_MUX" */
	da_combine_update_bits(codec, DAC_DATA_SEL_CTRL0_REG,
		MASK_ON_BIT(DAC1_DATA_SEL_LEN, DAC1_DATA_SEL_OFFSET), 0x0);

	/* "HP_HIGH_LEVEL" */
	da_combine_update_bits(codec, VIRTUAL_DOWN_REG, BIT(HP_HIGH_BIT), 0x0);
	hp_high_level_disable(codec);

	platform_data->route_state &= ~HEADSET_PLAY_ROUTE;

	AUDIO_LOGD("end");
}

static int headset_playback_state_change(struct snd_soc_component *codec, int event)
{
	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		headset_playback_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		headset_playback_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static int headset_playback_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	if (event == SND_SOC_DAPM_PRE_PMU)
		da_combine_set_headphone_switch_gpio_value(0);

	return headset_playback_state_change(codec, event);
}

static int rear_headphone_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	if (event == SND_SOC_DAPM_PRE_PMU)
		da_combine_set_headphone_switch_gpio_value(1);

	return headset_playback_state_change(codec, event);
}

static void headset_mic_capture_enable(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = NULL;

	AUDIO_LOGD("begin");

	if (codec == NULL) {
		AUDIO_LOGE("snd soc parameter is NULL");
		return;
	}

	platform_data = snd_soc_component_get_drvdata(codec);

	/* "HSMIC_PGA_MUX" */
	da_combine_update_bits(codec, ANA_HSMIC_MUX_AND_PGA,
		BIT(ANA_HSMIC_MUX_BIT), BIT(ANA_HSMIC_MUX_BIT));
	/* "HSMIC_PGA_GAIN" */
	da_combine_update_bits(codec, ANA_HSMIC_MUX_AND_PGA, 0xF,
		platform_data->board_config.hs_capture_gain);
	/* "ADCL0_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
		MASK_ON_BIT(ADC0L_DIN_SEL_LEN, ADC0L_DIN_SEL_OFFSET), 0x0);
	/* "ADCR0_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
		MASK_ON_BIT(ADC0R_DIN_SEL_LEN, ADC0R_DIN_SEL_OFFSET), 0x0);

	platform_data->route_state |= HEADSET_MIC_ROUTE;

	AUDIO_LOGD("end");
}

static void headset_mic_capture_disable(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = NULL;

	AUDIO_LOGD("begin");

	if (codec == NULL) {
		AUDIO_LOGE("snd soc parameter is NULL");
		return;
	}

	platform_data = snd_soc_component_get_drvdata(codec);

	/* "HSMIC_PGA_MUX" */
	da_combine_update_bits(codec, ANA_HSMIC_MUX_AND_PGA,
		BIT(ANA_HSMIC_MUX_BIT), 0x0);
	/* "HSMIC_PGA_GAIN" */
	da_combine_update_bits(codec, ANA_HSMIC_MUX_AND_PGA, 0xF, 0x4);

	platform_data->route_state &= ~HEADSET_MIC_ROUTE;

	AUDIO_LOGD("end");
}

static int headset_mic_capture_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		headset_mic_capture_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		headset_mic_capture_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static void dmic_capture_enable(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	platform_data->route_state |= DMIC_ROUTE;
	/* "DMIC0 CLK" */
	da_combine_update_bits(codec, DMIC_CLK_EN_REG,
		MASK_ON_BIT(DMIC0_CLK_EN_LEN, DMIC0_CLK_EN_OFFSET) |
		MASK_ON_BIT(FS_DMIC0_LEN, FS_DMIC0_OFFSET),
		BIT(DMIC0_CLK_EN_OFFSET) | BIT(FS_DMIC0_OFFSET));
	/* "DMIC1 CLK" */
	da_combine_update_bits(codec, DMIC_CLK_EN_REG,
		MASK_ON_BIT(DMIC1_CLK_EN_LEN, DMIC1_CLK_EN_OFFSET),
		BIT(DMIC1_CLK_EN_OFFSET));
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(FS_DMIC1_LEN, FS_DMIC1_OFFSET),
		BIT(FS_DMIC1_OFFSET));
	/* "ADCL0_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
		MASK_ON_BIT(ADC0L_DIN_SEL_LEN, ADC0L_DIN_SEL_OFFSET),
		0x5 << ADC0L_DIN_SEL_OFFSET);
	/* "ADCR0_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
		MASK_ON_BIT(ADC0R_DIN_SEL_LEN, ADC0R_DIN_SEL_OFFSET),
		0x6 << ADC0R_DIN_SEL_OFFSET);
	/* "ADCL1_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL16_REG,
		MASK_ON_BIT(ADC1L_DIN_SEL_LEN, ADC1L_DIN_SEL_OFFSET),
		0x7 << ADC1L_DIN_SEL_OFFSET);
	/* "ADCR1_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL16_REG,
		MASK_ON_BIT(ADC1R_DIN_SEL_LEN, ADC1R_DIN_SEL_OFFSET),
		0x8 << ADC1R_DIN_SEL_OFFSET);
	/* "MIC3_MUX "ADCL1" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL28_REG,
		MASK_ON_BIT(MIC_3_SEL_LEN, MIC_3_SEL_OFFSET),
		BIT(MIC_3_SEL_OFFSET));
	/* "MIC4_MUX "ADCR1" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL28_REG,
		MASK_ON_BIT(MIC_4_SEL_LEN, MIC_4_SEL_OFFSET),
		BIT(MIC_4_SEL_OFFSET));
}

static void dmic_capture_disable(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	platform_data->route_state &= ~DMIC_ROUTE;
	/* "DMIC0 CLK" */
	da_combine_update_bits(codec, DMIC_CLK_EN_REG,
		MASK_ON_BIT(DMIC0_CLK_EN_LEN, DMIC0_CLK_EN_OFFSET) |
		MASK_ON_BIT(FS_DMIC0_LEN, FS_DMIC0_OFFSET), 0);
	/* "DMIC1 CLK" */
	da_combine_update_bits(codec, DMIC_CLK_EN_REG,
		MASK_ON_BIT(DMIC1_CLK_EN_LEN, DMIC1_CLK_EN_OFFSET), 0);
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(FS_DMIC1_LEN, FS_DMIC1_OFFSET), 0);
	/* "ADCL0_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
		MASK_ON_BIT(ADC0L_DIN_SEL_LEN, ADC0L_DIN_SEL_OFFSET), 0x0);
	/* "ADCR0_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
		MASK_ON_BIT(ADC0R_DIN_SEL_LEN, ADC0R_DIN_SEL_OFFSET), 0x0);
	/* "ADCL1_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL16_REG,
		MASK_ON_BIT(ADC1L_DIN_SEL_LEN, ADC1L_DIN_SEL_OFFSET), 0x0);
	/* "ADCR1_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL16_REG,
		MASK_ON_BIT(ADC1R_DIN_SEL_LEN, ADC1R_DIN_SEL_OFFSET), 0x0);
	/* "MIC3_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL28_REG,
		MASK_ON_BIT(MIC_3_SEL_LEN, MIC_3_SEL_OFFSET), 0x0);
	/* "MIC4_MUX */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL28_REG,
		MASK_ON_BIT(MIC_4_SEL_LEN, MIC_4_SEL_OFFSET), 0x0);
}

static int mic_capture_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		dmic_capture_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		dmic_capture_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int bt_playback_common_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* "BT_L_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL31_REG,
			MASK_ON_BIT(BTL_DATA_SEL_LEN, BTL_DATA_SEL_OFFSET),
			0x2 << BTL_DATA_SEL_OFFSET);
		/* "BT_R_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL31_REG,
			MASK_ON_BIT(BTR_DATA_SEL_LEN, BTR_DATA_SEL_OFFSET),
			0x2 << BTR_DATA_SEL_OFFSET);
		/* "S2_DIRECT_MODE" */
		da_combine_update_bits(codec, SC_S2_IF_L_REG,
			MASK_ON_BIT(S2_DIRECT_LOOP_LEN, S2_DIRECT_LOOP_OFFSET), 0x0);
		/* "S2_IF_TX_ENA" */
		da_combine_update_bits(codec, SC_S2_IF_L_REG,
			MASK_ON_BIT(S2_IF_TX_ENA_LEN, S2_IF_TX_ENA_OFFSET),
			BIT(S2_IF_TX_ENA_OFFSET));
		/* "S2_OL_SRC_IN_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL6_REG,
			MASK_ON_BIT(S2_OL_SRC_DIN_VLD_SEL_LEN, S2_OL_SRC_DIN_VLD_SEL_OFFSET),
			BIT(S2_OL_SRC_DIN_VLD_SEL_OFFSET));
		/* "S2_OR_SRC_IN_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL6_REG,
			MASK_ON_BIT(S2_OR_SRC_DIN_VLD_SEL_LEN, S2_OR_SRC_DIN_VLD_SEL_OFFSET),
			BIT(S2_OR_SRC_DIN_VLD_SEL_OFFSET));
		s2up_power_event(w, kcontrol, event);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* "BT_L_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL31_REG,
			MASK_ON_BIT(BTL_DATA_SEL_LEN, BTL_DATA_SEL_OFFSET), 0x0);
		/* "BT_R_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL31_REG,
			MASK_ON_BIT(BTR_DATA_SEL_LEN, BTR_DATA_SEL_OFFSET), 0x0);
		/* "S2_DIRECT_MODE" */
		da_combine_update_bits(codec, SC_S2_IF_L_REG,
			MASK_ON_BIT(S2_DIRECT_LOOP_LEN, S2_DIRECT_LOOP_OFFSET), 0x0);
		/* "S2_IF_TX_ENA" */
		da_combine_update_bits(codec, SC_S2_IF_L_REG,
			MASK_ON_BIT(S2_IF_TX_ENA_LEN, S2_IF_TX_ENA_OFFSET), 0x0);
		/* "S2_OL_SRC_IN_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL6_REG,
			MASK_ON_BIT(S2_OL_SRC_DIN_VLD_SEL_LEN, S2_OL_SRC_DIN_VLD_SEL_OFFSET),
			0x0);
		/* "S2_OR_SRC_IN_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL6_REG,
			MASK_ON_BIT(S2_OR_SRC_DIN_VLD_SEL_LEN, S2_OR_SRC_DIN_VLD_SEL_OFFSET),
			0x0);
		s2up_power_event(w, kcontrol, event);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;

}

static int bt_playback_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		bt_playback_common_power_event(w, kcontrol, event);
		/* "S2_OL_SRC_MODE" */
		da_combine_update_bits(codec, S2_OL_SRC_CTRL_REG,
			MASK_ON_BIT(S2_OL_SRC_MODE_LEN, S2_OL_SRC_MODE_OFFSET),
			BIT(S2_OL_SRC_MODE_OFFSET));
		/* "S2_OR_SRC_MODE" */
		da_combine_update_bits(codec, S2_OR_SRC_CTRL_REG,
			MASK_ON_BIT(S2_OR_SRC_MODE_LEN, S2_OR_SRC_MODE_OFFSET),
			BIT(S2_OR_SRC_MODE_OFFSET));
		/* "S2_O_SRC_OUT_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL6_REG,
			MASK_ON_BIT(S2_O_SRC_DOUT_VLD_SEL_LEN, S2_O_SRC_DOUT_VLD_SEL_OFFSET),
			0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		bt_playback_common_power_event(w, kcontrol, event);
		/* "S2_OL_SRC_MODE" */
		da_combine_update_bits(codec, S2_OL_SRC_CTRL_REG,
			MASK_ON_BIT(S2_OL_SRC_MODE_LEN, S2_OL_SRC_MODE_OFFSET), 0x0);
		/* "S2_OR_SRC_MODE" */
		da_combine_update_bits(codec, S2_OR_SRC_CTRL_REG,
			MASK_ON_BIT(S2_OR_SRC_MODE_LEN, S2_OR_SRC_MODE_OFFSET), 0x0);
		/* "S2_O_SRC_OUT_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL6_REG,
			MASK_ON_BIT(S2_O_SRC_DOUT_VLD_SEL_LEN, S2_O_SRC_DOUT_VLD_SEL_OFFSET),
			0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int bt_playback_wb_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		bt_playback_common_power_event(w, kcontrol, event);
		/* "S2_OL_SRC_MODE" */
		da_combine_update_bits(codec, S2_OL_SRC_CTRL_REG,
			MASK_ON_BIT(S2_OL_SRC_MODE_LEN, S2_OL_SRC_MODE_OFFSET),
			0x2 << S2_OL_SRC_MODE_OFFSET);
		/* "S2_OR_SRC_MODE" */
		da_combine_update_bits(codec, S2_OR_SRC_CTRL_REG,
			MASK_ON_BIT(S2_OR_SRC_MODE_LEN, S2_OR_SRC_MODE_OFFSET),
			0x2 << S2_OL_SRC_MODE_OFFSET);
		/* "S2_O_SRC_OUT_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL6_REG,
			MASK_ON_BIT(S2_O_SRC_DOUT_VLD_SEL_LEN, S2_O_SRC_DOUT_VLD_SEL_OFFSET),
			BIT(S2_O_SRC_DOUT_VLD_SEL_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		bt_playback_common_power_event(w, kcontrol, event);
		/* "S2_OL_SRC_MODE" */
		da_combine_update_bits(codec, S2_OL_SRC_CTRL_REG,
			MASK_ON_BIT(S2_OL_SRC_MODE_LEN, S2_OL_SRC_MODE_OFFSET), 0x0);
		/* "S2_OR_SRC_MODE" */
		da_combine_update_bits(codec, S2_OR_SRC_CTRL_REG,
			MASK_ON_BIT(S2_OR_SRC_MODE_LEN, S2_OR_SRC_MODE_OFFSET), 0x0);
		/* "S2_O_SRC_OUT_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL6_REG,
			MASK_ON_BIT(S2_O_SRC_DOUT_VLD_SEL_LEN, S2_O_SRC_DOUT_VLD_SEL_OFFSET),
			0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int bt_capture_common_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* "MIC1_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL29_REG,
			MASK_ON_BIT(MIC_1_SEL_LEN, MIC_1_SEL_OFFSET), 0x3 << MIC_1_SEL_OFFSET);
		/* "MIC2_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL29_REG,
			MASK_ON_BIT(MIC_2_SEL_LEN, MIC_2_SEL_OFFSET), 0x3 << MIC_2_SEL_OFFSET);
		/* "S2_IL_SRC_OUT_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL0_REG,
			MASK_ON_BIT(S2_IL_SRC_DOUT_VLD_SEL_LEN, S2_IL_SRC_DOUT_VLD_SEL_OFFSET),
			BIT(S2_IL_SRC_DOUT_VLD_SEL_OFFSET));
		/* "S2_IR_SRC_OUT_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL0_REG,
			MASK_ON_BIT(S2_IR_SRC_DOUT_VLD_SEL_LEN, S2_IR_SRC_DOUT_VLD_SEL_OFFSET),
			BIT(S2_IR_SRC_DOUT_VLD_SEL_OFFSET));
		s2_rx_power_event(w, kcontrol, event);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* "MIC1_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL29_REG,
			MASK_ON_BIT(MIC_1_SEL_LEN, MIC_1_SEL_OFFSET), 0x0);
		/* "MIC2_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL29_REG,
			MASK_ON_BIT(MIC_2_SEL_LEN, MIC_2_SEL_OFFSET), 0x0);
		/* "S2_IL_SRC_OUT_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL0_REG,
			MASK_ON_BIT(S2_IL_SRC_DOUT_VLD_SEL_LEN, S2_IL_SRC_DOUT_VLD_SEL_OFFSET),
			0x0);
		/* "S2_IR_SRC_OUT_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL0_REG,
			MASK_ON_BIT(S2_IR_SRC_DOUT_VLD_SEL_LEN, S2_IR_SRC_DOUT_VLD_SEL_OFFSET),
			0x0);
		s2_rx_power_event(w, kcontrol, event);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;

}

static int bt_capture_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		bt_capture_common_power_event(w, kcontrol, event);
		/* "S2_IL_SRC_MODE" */
		da_combine_update_bits(codec, S2_IL_SRC_CTRL_REG,
			MASK_ON_BIT(S2_IL_SRC_MODE_LEN, S2_IL_SRC_MODE_OFFSET),
			0x7 << S2_IL_SRC_MODE_OFFSET);
		/* "S2_IR_SRC_MODE" */
		da_combine_update_bits(codec, S2_IR_SRC_CTRL_REG,
			MASK_ON_BIT(S2_IR_SRC_MODE_LEN, S2_IR_SRC_MODE_OFFSET),
			0x7 << S2_IR_SRC_MODE_OFFSET);
		/* "S2_I_SRC_IN_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL0_REG,
			MASK_ON_BIT(S2_I_SRC_DIN_VLD_SEL_LEN, S2_I_SRC_DIN_VLD_SEL_OFFSET),
			0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		bt_capture_common_power_event(w, kcontrol, event);
		/* "S2_IL_SRC_MODE" */
		da_combine_update_bits(codec, S2_IL_SRC_CTRL_REG,
			MASK_ON_BIT(S2_IL_SRC_MODE_LEN, S2_IL_SRC_MODE_OFFSET), 0x0);
		/* "S2_IR_SRC_MODE" */
		da_combine_update_bits(codec, S2_IR_SRC_CTRL_REG,
			MASK_ON_BIT(S2_IR_SRC_MODE_LEN, S2_IR_SRC_MODE_OFFSET), 0x0);
		/* "S2_I_SRC_IN_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL0_REG,
			MASK_ON_BIT(S2_I_SRC_DIN_VLD_SEL_LEN, S2_I_SRC_DIN_VLD_SEL_OFFSET),
			0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int bt_capture_wb_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		bt_capture_common_power_event(w, kcontrol, event);
		/* "S2_IL_SRC_MODE" */
		da_combine_update_bits(codec, S2_IL_SRC_CTRL_REG,
			MASK_ON_BIT(S2_IL_SRC_MODE_LEN, S2_IL_SRC_MODE_OFFSET),
			0x5 << S2_IL_SRC_MODE_OFFSET);
		/* "S2_IR_SRC_MODE" */
		da_combine_update_bits(codec, S2_IR_SRC_CTRL_REG,
			MASK_ON_BIT(S2_IR_SRC_MODE_LEN, S2_IR_SRC_MODE_OFFSET),
			0x5 << S2_IR_SRC_MODE_OFFSET);
		/* "S2_I_SRC_IN_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL0_REG,
			MASK_ON_BIT(S2_I_SRC_DIN_VLD_SEL_LEN, S2_I_SRC_DIN_VLD_SEL_OFFSET),
			BIT(S2_I_SRC_DIN_VLD_SEL_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		bt_capture_common_power_event(w, kcontrol, event);
		/* "S2_IL_SRC_MODE" */
		da_combine_update_bits(codec, S2_IL_SRC_CTRL_REG,
			MASK_ON_BIT(S2_IL_SRC_MODE_LEN, S2_IL_SRC_MODE_OFFSET), 0x0);
		/* "S2_IR_SRC_MODE" */
		da_combine_update_bits(codec, S2_IR_SRC_CTRL_REG,
			MASK_ON_BIT(S2_IR_SRC_MODE_LEN, S2_IR_SRC_MODE_OFFSET), 0x0);
		/* "S2_I_SRC_IN_FS" */
		da_combine_update_bits(codec, SRC_VLD_CTRL0_REG,
			MASK_ON_BIT(S2_I_SRC_DIN_VLD_SEL_LEN, S2_I_SRC_DIN_VLD_SEL_OFFSET),
			0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int rear_capture_state_change(struct snd_soc_component *codec, int event)
{
	AUDIO_LOGI("power event: %d", event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* "HSMIC_PGA_MUX" */
		da_combine_update_bits(codec, ANA_HSMIC_MUX_AND_PGA, 0x7 << ANA_HSMIC_MUX_BIT, 0x4 << ANA_HSMIC_MUX_BIT);
		/* "AUXMIC_PGA_MUX" */
		da_combine_update_bits(codec, ANA_AUXMIC_MUX_AND_PGA, 0x7 << ANA_HSMIC_MUX_BIT, BIT(ANA_AUXMIC_MUX_BIT));
		/* "ADCL0_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
			MASK_ON_BIT(ADC0L_DIN_SEL_LEN, ADC0L_DIN_SEL_OFFSET), 0x0);
		/* "ADCR0_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
			MASK_ON_BIT(ADC0R_DIN_SEL_LEN, ADC0R_DIN_SEL_OFFSET), BIT(ADC0R_DIN_SEL_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* "HSMIC_PGA_MUX" */
		da_combine_update_bits(codec, ANA_HSMIC_MUX_AND_PGA, 0x7 << ANA_HSMIC_MUX_BIT, 0x0);
		/* "AUXMIC_PGA_MUX" */
		da_combine_update_bits(codec, ANA_AUXMIC_MUX_AND_PGA, 0x7 << ANA_HSMIC_MUX_BIT, 0x0);
		/* "ADCL0_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
			MASK_ON_BIT(ADC0L_DIN_SEL_LEN, ADC0L_DIN_SEL_OFFSET), 0x0);
		/* "ADCR0_MUX" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL15_REG,
			MASK_ON_BIT(ADC0R_DIN_SEL_LEN, ADC0R_DIN_SEL_OFFSET), 0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int rear_linein_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	if (event == SND_SOC_DAPM_PRE_PMU) {
		da_combine_set_head_mic_switch_gpio_value(1);
		/* "HSMIC_PGA_GAIN" */
		da_combine_update_bits(codec, ANA_HSMIC_MUX_AND_PGA,
			MASK_ON_BIT(ANA_HSMIC_PGA_SEL_LEN, ANA_HSMIC_PGA_BIT), 0x3);
		/* "AUXMIC_PGA_GAIN" */
		da_combine_update_bits(codec, ANA_AUXMIC_MUX_AND_PGA,
			MASK_ON_BIT(ANA_AUXMIC_PGA_SEL_LEN, ANA_AUXMIC_PGA_BIT), 0x3);
	}

	return rear_capture_state_change(codec, event);
}

static int rear_head_mic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	if (event == SND_SOC_DAPM_PRE_PMU) {
		da_combine_set_head_mic_switch_gpio_value(0);
		/* "HSMIC_PGA_GAIN" */
		da_combine_update_bits(codec, ANA_HSMIC_MUX_AND_PGA,
			MASK_ON_BIT(ANA_HSMIC_PGA_SEL_LEN, ANA_HSMIC_PGA_BIT), 0xF);
		/* "AUXMIC_PGA_GAIN" */
		da_combine_update_bits(codec, ANA_AUXMIC_MUX_AND_PGA,
			MASK_ON_BIT(ANA_AUXMIC_PGA_SEL_LEN, ANA_AUXMIC_PGA_BIT), 0xF);
	}

	return rear_capture_state_change(codec, event);
}

#define SINGLE_SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("Speaker Playback", \
		SND_SOC_NOPM, 0, 0, &dapm_speaker_playbcak_switch_controls, \
		speaker_playback_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("Headset Playback", \
		SND_SOC_NOPM, 0, 0, &dapm_headset_playback_switch_controls, \
		headset_playback_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("Headset Mic Capture", \
		SND_SOC_NOPM, 0, 0, &dapm_headset_mic_capture_switch_controls, \
		headset_mic_capture_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("Mic Capture", \
		SND_SOC_NOPM, 0, 0, &dapm_mic_capture_switch_controls, \
		mic_capture_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("Bt Playback", \
		SND_SOC_NOPM, 0, 0, &dapm_bt_playback_switch_controls, \
		bt_playback_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("Bt Playback Wb", \
		SND_SOC_NOPM, 0, 0, &dapm_bt_playback_wb_switch_controls, \
		bt_playback_wb_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("Bt Uplink", \
		SND_SOC_NOPM, 0, 0, &dapm_bt_capture_switch_controls, \
		bt_capture_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("Bt Uplink Wb", \
		SND_SOC_NOPM, 0, 0, &dapm_bt_capture_wb_switch_controls, \
		bt_capture_wb_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("LineIn Capture", \
		SND_SOC_NOPM, 0, 0, &dapm_rear_linein_switch_controls, \
		rear_linein_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("Rear Mic Capture", \
		SND_SOC_NOPM, 0, 0, &dapm_rear_head_mic_switch_controls, \
		rear_head_mic_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("Rear Headphone Playback", \
		SND_SOC_NOPM, 0, 0, &dapm_rear_headphone_switch_controls, \
		rear_headphone_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

static const struct snd_soc_dapm_widget single_switch_widgets[] = {
	SINGLE_SWITCH_WIDGET
};

int da_combine_v5_add_single_switch_widgets(struct snd_soc_component *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	return snd_soc_dapm_new_controls(dapm, single_switch_widgets,
		ARRAY_SIZE(single_switch_widgets));
}

