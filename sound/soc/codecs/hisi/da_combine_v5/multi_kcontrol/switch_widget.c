/*
 * switch_widget.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "switch_widget.h"

#include <linux/version.h>
#include <sound/core.h>
#include <sound/tlv.h>

#include "asoc_adapter.h"
#include "slimbus.h"
#include "slimbus_da_combine_v5.h"
#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_v5.h"
#include "linux/hisi/da_combine/da_combine_v5_regs.h"
#include "linux/hisi/da_combine/da_combine_v5_type.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "path_widget.h"
#include "switch_widget_utils.h"

#include "huawei_platform/power/vsys_switch/vsys_switch.h"

/* SWITCH - AUDIODOWN */
static const struct snd_kcontrol_new dapm_play44k1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLAY44K1_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_play88k2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLAY88K2_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_play176k4_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLAY176K4_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_play48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLAY48K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_play96k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLAY96K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_play192k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLAY192K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_play384k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLAY384K_BIT, 1, 0);

static const struct snd_kcontrol_new dapm_hp_high_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, HP_HIGH_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_hp_concurrency_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, HP_CONCURRENCY_BIT, 1, 0);

static const struct snd_kcontrol_new dapm_play44k1_gain_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLAY44K1_GAIN_BIT, 1, 0);


/* SWITCH - S1-S4 */
static const struct snd_kcontrol_new dapm_s2_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, S2_OL_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s2_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, S2_OR_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s4_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, S4_OL_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s4_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, S4_OR_BIT, 1, 0);

/* SWITCH - VIR */
static const struct snd_kcontrol_new dapm_play384k_vir_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLAY384K_VIR_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_iv_dspif_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, IV_DSPIF_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_iv_2pa_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, IV_2PA_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_iv_4pa_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, IV_4PA_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_ir_emission_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, IR_EMISSION_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_ec_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, EC_BIT, 1, 0);

static const struct snd_kcontrol_new dapm_mad_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, MAD_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_ultrasonic_up_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, ULTRASONIC_UP_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s1_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, S1_OL_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s1_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, S1_OR_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s3_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, S3_OL_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s3_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, S3_OR_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_u3_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, U3_OL_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_u4_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, U4_OR_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_u5_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, U5_OL_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_us_r1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, US_R1_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_us_r2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, US_R2_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_audioup_2mic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, AUDIOUP_2MIC_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_audioup_4mic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, AUDIOUP_4MIC_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_audioup_4mic_v2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, AUDIOUP_4MIC_V2_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_audioup_5mic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, AUDIOUP_5MIC_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_ultra_up9_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_EXTERN_REG, ULTRA_UP9_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice8k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VOICE8K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice16k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VOICE16K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice32k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VOICE32K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice8k_v2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_EXTERN_REG, VOICE8K_V2_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice16k_v2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_EXTERN_REG, VOICE16K_V2_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice32k_v2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_EXTERN_REG, VOICE32K_V2_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice8k_4pa_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VOICE8K_4PA_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice16k_4pa_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VOICE16K_4PA_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice32k_4pa_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VOICE32K_4PA_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_ir_env_study_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, IR_ENV_STUDY_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_soundtrigger_onemic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, SOUNDTRIGGER_ONE_MIC_EN_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_soundtrigger_dualmic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, SOUNDTRIGGER_DUAL_MIC_EN_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_auxmic_hsmicbias_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, AUXMIC_HSMICBIAS_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_auxmic_micbias2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, AUXMIC_MICBIAS2_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_auxmic_micbias1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, AUXMIC_MICBIAS1_EN_MM_BIT, 1, 0);

/* SWITCH - VIR EXTERN */
static const struct snd_kcontrol_new dapm_dacl96k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_EXTERN_REG, DACL96K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_mic4_micbias1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_EXTERN_REG, MIC4_MICBIAS1_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_mic5_micbias1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_EXTERN_REG, MIC5_MICBIAS1_EN_MM_BIT, 1, 0);

/* I2S2 bluetooth LOOP ENABLE */
static const struct snd_kcontrol_new dapm_i2s2_bluetooth_loop_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, I2S2_BLUETOOTH_LOOP_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_moveup_2pa_tdm_iv_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, MOVEUP_2PA_TDM_IV_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_moveup_4pa_tdm_iv_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, MOVEUP_4PA_TDM_IV_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_tdm_audio_pa_down_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, TDM_AUDIO_PA_TDM_DOWN_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_tdm_audio_pa_down_loop_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, TDM_AUDIO_PA_TDM_DN_LOOP_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_fm_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, FM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_fm_hifi_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, FM_HIFI_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_virtbtn_active_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VIRTUAL_BTN_ACTIVE_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_virtbtn_passive_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VIRTUAL_BTN_PASSIVE_BIT, 1, 0);



static int play44k1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	return play_config_power_event(SAMPLE_RATE_INDEX_44K1, codec, event);
}

static int play88k2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	return play_config_power_event(SAMPLE_RATE_INDEX_88K2, codec, event);
}

static int play176k4_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	return play_config_power_event(SAMPLE_RATE_INDEX_176K4, codec, event);
}

static int play48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	return play_config_power_event(SAMPLE_RATE_INDEX_48K, codec, event);
}

static int play96k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	return play_config_power_event(SAMPLE_RATE_INDEX_96K, codec, event);
}

static int play192k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	return play_config_power_event(SAMPLE_RATE_INDEX_192K, codec, event);
}

static int play384k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	return play_config_power_event(SAMPLE_RATE_INDEX_384K, codec, event);

}


static int hp_high_level_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hp_high_level_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hp_high_level_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static int hp_concurrency_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *priv = snd_soc_component_get_drvdata(codec);
	int ret = 0;

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			1 << S2_IL_SRC_CLK_EN_OFFSET | 1 << S2_IR_SRC_CLK_EN_OFFSET,
			1 << S2_IL_SRC_CLK_EN_OFFSET | 1 << S2_IR_SRC_CLK_EN_OFFSET);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4, &priv->play_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus track activate 4pa iv err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4, &priv->play_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus track activate 4pa iv err: %d", ret);
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			1 << S2_IL_SRC_CLK_EN_OFFSET | 1 << S2_IR_SRC_CLK_EN_OFFSET,
			0x0);
		break;
	default:
		AUDIO_LOGW("power event err:%d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

int s2up_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			BIT(S2_OL_SRC_CLK_EN_OFFSET) | BIT(S2_OR_SRC_CLK_EN_OFFSET),
			BIT(S2_OL_SRC_CLK_EN_OFFSET) | BIT(S2_OR_SRC_CLK_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			BIT(S2_OL_SRC_CLK_EN_OFFSET) | BIT(S2_OR_SRC_CLK_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}


/* for codec path of smartpa in codec dsp */
static int iv_dspif_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		iv_dspif_switch_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		iv_dspif_switch_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

/* for codec path of smartpa alg in soc */
int iv_2pa_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		iv_2pa_switch_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		iv_2pa_switch_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}


static int fm_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			BIT(S2_IL_SRC_CLK_EN_OFFSET) | BIT(S2_IR_SRC_CLK_EN_OFFSET),
			BIT(S2_IL_SRC_CLK_EN_OFFSET) | BIT(S2_IR_SRC_CLK_EN_OFFSET));
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
			BIT(DAC_MIXER4_VLD_SEL_OFFSET) | BIT(DAC_PRE_MIXER2_VLD_SEL_OFFSET),
			BIT(DAC_MIXER4_VLD_SEL_OFFSET) | BIT(DAC_PRE_MIXER2_VLD_SEL_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
			BIT(DAC_MIXER4_VLD_SEL_OFFSET)  | BIT(DAC_PRE_MIXER2_VLD_SEL_OFFSET), 0);
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			BIT(S2_IL_SRC_CLK_EN_OFFSET) | BIT(S2_IR_SRC_CLK_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int fm_hifi_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	int ret;

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			BIT(S2_IL_SRC_CLK_EN_OFFSET) | BIT(S2_IR_SRC_CLK_EN_OFFSET),
			BIT(S2_IL_SRC_CLK_EN_OFFSET) | BIT(S2_IR_SRC_CLK_EN_OFFSET));
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5, SLIMBUS_DA_COMBINE_V5_TRACK_ECREF, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus activate track fm err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5, SLIMBUS_DA_COMBINE_V5_TRACK_ECREF, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus deactivate track fm err: %d", ret);
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			BIT(S2_IL_SRC_CLK_EN_OFFSET) | BIT(S2_IR_SRC_CLK_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

int audioup_2mic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ret = audioup_2mic_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = audioup_2mic_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;

}


static const struct reg_seq_config audioup_4mic_up_v2_regs[] = {
	{ { S1_MIXER_EQ_CLK_EN_REG, BIT(RST_5MIC_S1_ACCESS_IRQ_OFFSET),
		BIT(RST_5MIC_S1_ACCESS_IRQ_OFFSET), true }, 0, 0 },
	{ { SC_FS_SLIM_CTRL_3_REG, 0, 0x44, false }, 0, 0 },
	/* src in && out sample rate */
	{ { SRC_VLD_CTRL2_REG, 0, 0x1C, false }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, (BIT(S1_MIC1_SRC_CLK_EN_OFFSET)) |
		(BIT(S1_MIC2_SRC_CLK_EN_OFFSET)), 0x0, true }, 0, 0 },
	/* src down 2 mode */
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC1_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC2_SRC_MODE_OFFSET, true }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, (1 << S1_MIC1_SRC_CLK_EN_OFFSET) |
		(1 << S1_MIC2_SRC_CLK_EN_OFFSET), (1 << S1_MIC1_SRC_CLK_EN_OFFSET) |
		(1 << S1_MIC2_SRC_CLK_EN_OFFSET), true }, 0, 0 },

	{ { SC_FS_SLIM_CTRL_7_REG,
		MASK_ON_BIT(FS_SLIM_UP10_LEN, FS_SLIM_UP10_OFFSET),
		SAMPLE_RATE_REG_CFG_48K << FS_SLIM_UP10_OFFSET, true }, 0, 0 },

	/* for Mic5 */
	{ { SRC_VLD_CTRL5_REG, 0, 0xC, false }, 0, 0 },
	{ { S4_DP_CLK_EN_REG, BIT(S1_MIC5_SRC_CLK_EN_OFFSET), 0x0,
		true }, 0, 0 },
	{ { MIC5_SRC_CTRL_REG, 0x7 << MIC5_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC5_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { S4_DP_CLK_EN_REG, BIT(S1_MIC5_SRC_CLK_EN_OFFSET),
		BIT(S1_MIC5_SRC_CLK_EN_OFFSET), true }, 0, 0 },
	/* 0x20007033 0x0, Mic5-Port7, Port6 and Port7 not use the same ACK */
	{ { SLIM_CTRL3_REG, 1 << 3, 0 << 3, true }, 0, 0 },
};


static const struct reg_seq_config audioup_4mic_dn_v2_regs[] = {
	/* src power off */
	{ { S1_DP_CLK_EN_REG, 1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL2_REG, 0, 0x0, false }, 0, 0 },

	{ { S4_DP_CLK_EN_REG, BIT(S1_MIC5_SRC_CLK_EN_OFFSET), 0x0,
		true }, 0, 0 },

	/* 0x20007033  Mic5-Port7, Port6 and Port7 revert to use the same ACK */
	{ { SLIM_CTRL3_REG, 1 << 3, 1 << 3, true }, 0, 0 },
	{ { MIC5_SRC_CTRL_REG, 0x7 << MIC5_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL5_REG, 0, 0x0, false }, 0, 0 },
};

int audioup_4mic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ret = audioup_4mic_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = audioup_4mic_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;

}

static int audioup_4mic_v2_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	int ret = 0;

	/* back_mic capture use u1 u2 u10, data in hifi will be 3ch to 4ch */
	platform_data->capture_params.channels = 3;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		write_reg_seq_array(codec, audioup_4mic_up_v2_regs,
			ARRAY_SIZE(audioup_4mic_up_v2_regs));

		u10_select_mic_src(codec);
		u12_select_pga(codec);

		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE,
			&platform_data->capture_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track capture v2 err: %d", ret);

		da_combine_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			BIT(RST_5MIC_S1_ACCESS_IRQ_OFFSET), 0);

		platform_data->audioup_4mic_v2_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		u12_select_dspif(codec);
		u10_select_dspif(codec);

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE,
			&platform_data->capture_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track capture v2 err: %d", ret);

		write_reg_seq_array(codec, audioup_4mic_dn_v2_regs,
			ARRAY_SIZE(audioup_4mic_dn_v2_regs));

		platform_data->audioup_4mic_v2_state = TRACK_FREE;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static const struct reg_seq_config audioup_5mic_up_regs[] = {
	{ { S1_MIXER_EQ_CLK_EN_REG, BIT(RST_5MIC_S1_ACCESS_IRQ_OFFSET),
		BIT(RST_5MIC_S1_ACCESS_IRQ_OFFSET), true }, 0, 0 },
	{ { SC_FS_SLIM_CTRL_3_REG, 0, 0x44, false }, 0, 0 },
	/* src in && out sample rate */
	{ { SRC_VLD_CTRL2_REG, 0, 0x1C, false }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, (BIT(S1_MIC1_SRC_CLK_EN_OFFSET)) |
		(BIT(S1_MIC2_SRC_CLK_EN_OFFSET)), 0x0, true }, 0, 0 },
	/* src down 2 mode */
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC1_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC2_SRC_MODE_OFFSET, true }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, (1 << S1_MIC1_SRC_CLK_EN_OFFSET) |
		(1 << S1_MIC2_SRC_CLK_EN_OFFSET), (1 << S1_MIC1_SRC_CLK_EN_OFFSET) |
		(1 << S1_MIC2_SRC_CLK_EN_OFFSET), true }, 0, 0 },
	{ { SC_FS_SLIM_CTRL_4_REG,
		(MASK_ON_BIT(FS_SLIM_UP4_LEN, FS_SLIM_UP4_OFFSET) |
		MASK_ON_BIT(FS_SLIM_UP3_LEN, FS_SLIM_UP3_OFFSET)),
		(0x4 << FS_SLIM_UP4_OFFSET) |
		(0x4 << FS_SLIM_UP3_OFFSET), true }, 0, 0 },
	{ { SRC_VLD_CTRL3_REG, 0, 0xC, false }, 0, 0 },
	{ { SRC_VLD_CTRL4_REG, 0, 0xC, false }, 0, 0 },
	{ { S1_DP_CLK_EN_REG, (BIT(S1_MIC3_SRC_CLK_EN_OFFSET)) |
		(BIT(S1_MIC4_SRC_CLK_EN_OFFSET)), 0x0, true }, 0, 0 },
	{ { MIC3_SRC_CTRL_REG, 0x7 << MIC3_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC3_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { MIC4_SRC_CTRL_REG, 0x7 << MIC4_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC4_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { S1_DP_CLK_EN_REG, (1 << S1_MIC3_SRC_CLK_EN_OFFSET) |
		(1 << S1_MIC4_SRC_CLK_EN_OFFSET), (1 << S1_MIC3_SRC_CLK_EN_OFFSET) |
		(1 << S1_MIC4_SRC_CLK_EN_OFFSET), true }, 0, 0 },
	{ { SC_FS_SLIM_CTRL_7_REG,
		MASK_ON_BIT(FS_SLIM_UP10_LEN, FS_SLIM_UP10_OFFSET),
		0x4 << FS_SLIM_UP10_OFFSET, true }, 0, 0 },
	/* for Mic5 */
	{ { SRC_VLD_CTRL5_REG, 0, 0xC, false }, 0, 0 },
	{ { S4_DP_CLK_EN_REG, BIT(S1_MIC5_SRC_CLK_EN_OFFSET), 0x0,
		true }, 0, 0 },
	{ { MIC5_SRC_CTRL_REG, 0x7 << MIC5_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC5_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { S4_DP_CLK_EN_REG, BIT(S1_MIC5_SRC_CLK_EN_OFFSET),
		BIT(S1_MIC5_SRC_CLK_EN_OFFSET), true }, 0, 0 },
	/* 0x20007033 0x0, Mic5-Port7, Port6 and Port7 not use the same ACK */
	{ { SLIM_CTRL3_REG, 1 << 3, 0 << 3, true }, 0, 0 },
};

static const struct reg_seq_config audioup_5_2mic_up_regs[] = {
	{ { SC_FS_SLIM_CTRL_3_REG, 0, 0x44, false }, 0, 0 },
	/* src in && out sample rate */
	{ { SRC_VLD_CTRL2_REG, 0, 0x1C, false }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, BIT(S1_MIC1_SRC_CLK_EN_OFFSET) |
		BIT(S1_MIC2_SRC_CLK_EN_OFFSET), 0x0, true }, 0, 0 },
	/* src down 2 mode */
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC1_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC2_SRC_MODE_OFFSET, true }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, 1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		1 << S1_MIC2_SRC_CLK_EN_OFFSET, 1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		1 << S1_MIC2_SRC_CLK_EN_OFFSET, true }, 0, 0 },
};

static const struct reg_seq_config audioup_5mic_dn_regs[] = {
	/* src power off */
	{ { S1_DP_CLK_EN_REG, 1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL2_REG, 0, 0x0, false }, 0, 0 },
	{ { S1_DP_CLK_EN_REG, 1 << S1_MIC3_SRC_CLK_EN_OFFSET |
		1 << S1_MIC4_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC3_SRC_CTRL_REG, 0x7 << MIC3_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC4_SRC_CTRL_REG, 0x7 << MIC4_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL3_REG, 0, 0x0, false }, 0, 0 },
	{ { SRC_VLD_CTRL4_REG, 0, 0x0, false }, 0, 0 },
	{ { S4_DP_CLK_EN_REG, BIT(S1_MIC5_SRC_CLK_EN_OFFSET), 0x0,
		true }, 0, 0 },
	/* 0x20007033  Mic5-Port7, Port6 and Port7 revert to use the same ACK */
	{ { SLIM_CTRL3_REG, 1 << 3, 1 << 3, true }, 0, 0 },
	{ { MIC5_SRC_CTRL_REG, 0x7 << MIC5_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL5_REG, 0, 0x0, false }, 0, 0 },
};

static const struct reg_seq_config audioup_5_2mic_dn_regs[] = {
	/* src power off */
	{ { S1_DP_CLK_EN_REG, 1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL2_REG, 0, 0x0, false }, 0, 0 },
};


static int audioup_5mic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = NULL;
	int ret = 0;

	AUDIO_LOGD("begin");

	if (codec == NULL) {
		AUDIO_LOGE("nul, err");
		AUDIO_LOGD("end");
		return ret;
	}

	platform_data = snd_soc_component_get_drvdata(codec);
	if (platform_data == NULL) {
		AUDIO_LOGE("platform_data is nul, err");
		AUDIO_LOGD("end");
		return ret;
	}

	/* Numbers means the number of channels */
	if (platform_data->voiceup_state &&
		(platform_data->voice_up_params.channels == 4))
		platform_data->capture_params.channels = 2;
	else
		platform_data->capture_params.channels = 5;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (platform_data->capture_params.channels == 5)
			write_reg_seq_array(codec, audioup_5mic_up_regs,
				ARRAY_SIZE(audioup_5mic_up_regs));
		else if (platform_data->capture_params.channels == 2)
			write_reg_seq_array(codec, audioup_5_2mic_up_regs,
				ARRAY_SIZE(audioup_5_2mic_up_regs));

		u10_select_mic_src(codec);
		u12_select_pga(codec);

		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
				SLIMBUS_DA_COMBINE_V5_TRACK_5MIC_CAPTURE,
				&platform_data->capture_params);
		if (ret)
			AUDIO_LOGE("slimbus_activate_track capture err");

		if (platform_data->capture_params.channels == 5)
			da_combine_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
				0x1<<RST_5MIC_S1_ACCESS_IRQ_OFFSET, 0);

		platform_data->audioup_5mic_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		u12_select_dspif(codec);
		u10_select_dspif(codec);

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
				SLIMBUS_DA_COMBINE_V5_TRACK_5MIC_CAPTURE,
				&platform_data->capture_params);
		if (ret)
			AUDIO_LOGE("slimbus_deactivate_track capture err");

		if (platform_data->capture_params.channels == 5)
			write_reg_seq_array(codec, audioup_5mic_dn_regs,
				ARRAY_SIZE(audioup_5mic_dn_regs));
		else if (platform_data->capture_params.channels == 2)
			write_reg_seq_array(codec, audioup_5_2mic_dn_regs,
				ARRAY_SIZE(audioup_5_2mic_dn_regs));

		platform_data->audioup_5mic_state = TRACK_FREE;
		break;
	default:
		AUDIO_LOGW("power event err : %d", event);
		break;

	}

	AUDIO_LOGD("end");

	return ret;
}

static void ultrasonic_pll_turn_on(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	if (platform_data->pll_track == NULL) {
		AUDIO_LOGE("ultra pll track workqueue is null");
		return;
	}

	da_combine_irq_disable_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK);
	snd_soc_component_write(codec, IOS_MF_CTRL7_REG, 0x01);
	if (platform_data->board_config.ultra_pll_sync_i2s_id == 0)   /* 0:i2s2, 1:i2s3 */
		snd_soc_component_write(codec, PLL_CTRL0_REG, 0x50);
	else
		snd_soc_component_write(codec, PLL_CTRL0_REG, 0x60);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_160, 0x1C);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_161, 0);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_169, 0x02);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_153, BIT(MAIN1_FCM_EN_OFFSET), 0);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_162, 0x75);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_149,
		BIT(MAIN1_TRK_EN_OFFSET), BIT(MAIN1_TRK_EN_OFFSET));

	if (!queue_work(platform_data->pll_track, &platform_data->pll_track_work))
		AUDIO_LOGE("ultra pll track, this work was already on the queue");
}

static void ultrasonic_pll_turn_off(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	if (platform_data->pll_track == NULL) {
		AUDIO_LOGE("ultra pll track workqueue is null");
		return;
	}

	da_combine_update_bits(codec, CODEC_ANA_RWREG_149, BIT(MAIN1_TRK_EN_OFFSET), 0);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_152,
		BIT(MAIN1_DPLL_LOCK_LOSE_RST_OFFSET), BIT(MAIN1_DPLL_LOCK_LOSE_RST_OFFSET));
	udelay(200);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_152, BIT(MAIN1_DPLL_LOCK_LOSE_RST_OFFSET), 0);
	flush_workqueue(platform_data->pll_track);
	AUDIO_LOGI("pll track turn off");
}

static int ultra_up9_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	int ret = 0;

	platform_data->ultrasonic_up_params.channels = 1;
	platform_data->ultrasonic_up_params.rate = SLIMBUS_SAMPLE_RATE_48K;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG,
			BIT(S2_OL_SRC_CLK_EN_OFFSET) | BIT(S2_OL_PGA_CLK_EN_OFFSET),
			BIT(S2_OL_SRC_CLK_EN_OFFSET) | BIT(S2_OL_PGA_CLK_EN_OFFSET));
		ultrasonic_pll_turn_on(codec);
		/* 0x20007033 0x0, u9-Port6, Port6 and Port7 not use the same ACK */
		da_combine_update_bits(codec, SLIM_CTRL3_REG, BIT(SLIM_DPORT6_DPORT7_ACK_OFFSET), 0);
		da_combine_update_bits(codec, SC_FS_SLIM_CTRL_7_REG,
			MASK_ON_BIT(FS_SLIM_UP9_LEN, FS_SLIM_UP9_OFFSET),
			SAMPLE_RATE_REG_CFG_48K << FS_SLIM_UP9_OFFSET);
		/* ultrasonic up ADC2L FIR2D bypass enable */
		da_combine_update_bits(codec, ADC2L_CTRL0_REG,
			BIT(ADC2L_IR2D_BYPASS_EN_OFFSET), BIT(ADC2L_IR2D_BYPASS_EN_OFFSET));
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL25_REG,
			MASK_ON_BIT(SLIM_UP9_DATA_SEL_LEN, SLIM_UP9_DATA_SEL_OFFSET), 0);

		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_ULTRA_UP, &platform_data->ultrasonic_up_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track ultrasonic up err: %d", ret);

		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_ULTRA_UP, &platform_data->ultrasonic_up_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track ultrasonic up err: %d", ret);

		/* ultrasonic up ADC2L FIR2D bypass disable */
		da_combine_update_bits(codec, ADC2L_CTRL0_REG,
			BIT(ADC2L_IR2D_BYPASS_EN_OFFSET), 0);
		ultrasonic_pll_turn_off(codec);
		/* 0x20007033  u9-Port6, Port6 and Port7 revert to use the same ACK */
		da_combine_update_bits(codec, SLIM_CTRL3_REG, BIT(SLIM_DPORT6_DPORT7_ACK_OFFSET),
			BIT(SLIM_DPORT6_DPORT7_ACK_OFFSET));
		da_combine_update_bits(codec, S2_DP_CLK_EN_REG, BIT(S2_OL_SRC_CLK_EN_OFFSET) |
			BIT(S2_OL_PGA_CLK_EN_OFFSET), 0);

		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int voice8k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	int ret = 0;

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_8K;
	platform_data->voice_down_params.rate = SLIMBUS_SAMPLE_RATE_8K;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu(codec, platform_data->voice_up_params.channels,
			SAMPLE_RATE_REG_CFG_8K, SRC_MODE_12);
		ret = voice_slimbus_active(codec, platform_data);
		if (ret != 0)
			AUDIO_LOGE("voice_slimbus_active voice err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = voice_slimbus_deactive(codec, platform_data);
		if (ret != 0)
			AUDIO_LOGE("voice_slimbus_deactive voice err: %d", ret);
		up_src_post_pmu(codec,
			platform_data->voice_up_params.channels);

		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
	update_callswitch_state(event, platform_data);

	return ret;
}

static int voice16k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	int ret = 0;

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_16K;
	platform_data->voice_down_params.rate = SLIMBUS_SAMPLE_RATE_16K;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu(codec, platform_data->voice_up_params.channels,
			SAMPLE_RATE_REG_CFG_16K, SRC_MODE_6);
		ret = voice_slimbus_active(codec, platform_data);
		if (ret != 0)
			AUDIO_LOGE("voice_slimbus_active voice err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = voice_slimbus_deactive(codec, platform_data);
		if (ret != 0)
			AUDIO_LOGE("voice_slimbus_deactive voice err: %d", ret);
		up_src_post_pmu(codec,
			platform_data->voice_up_params.channels);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
	update_callswitch_state(event, platform_data);

	return ret;
}

static int voice32k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	int ret = 0;

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_32K;
	platform_data->voice_down_params.rate = SLIMBUS_SAMPLE_RATE_32K;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu(codec, platform_data->voice_up_params.channels,
			SAMPLE_RATE_REG_CFG_32K, SRC_MODE_3);
		ret = voice_slimbus_active(codec, platform_data);
		if (ret != 0)
			AUDIO_LOGE("voice_slimbus_active voice err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		platform_data->cdc_ctrl->ssi_check_enable = SSI_STATE_CHECK_ENABLE;
		ret = voice_slimbus_deactive(codec, platform_data);
		if (ret != 0)
			AUDIO_LOGE("voice_slimbus_deactive voice err: %d", ret);
		up_src_post_pmu(codec,
			platform_data->voice_up_params.channels);
		platform_data->cdc_ctrl->ssi_check_enable = SSI_STATE_CHECK_DISABLE;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
	update_callswitch_state(event, platform_data);

	return ret;
}


static int voice8k_v2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return voice_v2_power_event(w, event, SLIMBUS_SAMPLE_RATE_8K);
}

static int voice16k_v2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return voice_v2_power_event(w, event, SLIMBUS_SAMPLE_RATE_16K);
}

static int voice32k_v2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return voice_v2_power_event(w, event, SLIMBUS_SAMPLE_RATE_32K);
}

static int voice8k_4pa_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	int ret = 0;
	int channel_num = platform_data->board_config.backmic_record_enable ? 2 : 4;

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_8K;
	platform_data->voice_up_params.channels = channel_num;
	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu_v2(codec, channel_num, SAMPLE_RATE_REG_CFG_8K, SRC_MODE_12);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5, SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA,
			&platform_data->voice_up_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track voice err: %d", ret);
		platform_data->voiceup_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track voice err: %d", ret);
		up_src_post_pmu_v2(codec, channel_num);
		platform_data->voiceup_state = TRACK_FREE;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int voice16k_4pa_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	int ret = 0;
	int channel_num = platform_data->board_config.backmic_record_enable ? 2 : 4;

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_16K;
	platform_data->voice_up_params.channels = channel_num;
	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu_v2(codec, channel_num, SAMPLE_RATE_REG_CFG_16K, SRC_MODE_6);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5, SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA,
			&platform_data->voice_up_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track voice err: %d", ret);
		platform_data->voiceup_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track voice err: %d", ret);
		up_src_post_pmu_v2(codec, channel_num);
		platform_data->voiceup_state = TRACK_FREE;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int voice32k_4pa_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	int ret = 0;
	int channel_num = platform_data->board_config.backmic_record_enable ? 2 : 4;

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_32K;
	platform_data->voice_up_params.channels = channel_num;
	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu_v2(codec, channel_num, SAMPLE_RATE_REG_CFG_32K, SRC_MODE_3);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5, SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA,
			&platform_data->voice_up_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track voice err: %d", ret);
		platform_data->voiceup_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5, SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track voice err: %d", ret);
		up_src_post_pmu_v2(codec, channel_num);
		platform_data->voiceup_state = TRACK_FREE;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static void update_madswitch_state(int event,
	struct da_combine_v5_platform_data *platform_data)
{
	struct da_combine_v5_board_cfg board_cfg = platform_data->board_config;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		platform_data->is_madswitch_on = true;
		if (board_cfg.mic_control_sc_freq_enable) {
			vsys_switch_set_sc_frequency_mode(1); /* auto mode */
			AUDIO_LOGI("madswitch event update, set sc frequency mode 1");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		platform_data->is_madswitch_on = false;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
}

static int madswitch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* mad src enable */
		da_combine_update_bits(codec, S4_DP_CLK_EN_REG,
			BIT(S1_MAD_SRC_CLK_EN_OFFSET),
			BIT(S1_MAD_SRC_CLK_EN_OFFSET));
		/* s1_o_dsp_if_din_sel->mad_buffer out */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL21_REG,
			0x3 << S1_O_DSP_IF_DIN_SEL_OFFSET,
			0x2 << S1_O_DSP_IF_DIN_SEL_OFFSET);
		u12_select_dspif(codec);
		u9_select_dspif(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* s1_o_dsp_if_din_sel->mad_buffer out */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL21_REG,
			0x3 << S1_O_DSP_IF_DIN_SEL_OFFSET, 0x0);
		u12_select_pga(codec);
		u9_select_pga(codec);
		/* mad src disable */
		da_combine_update_bits(codec, S4_DP_CLK_EN_REG,
			BIT(S1_MAD_SRC_CLK_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
	update_madswitch_state(event, platform_data);

	return 0;
}

static int ultrasonic_up_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* set adc2l_pga bypass */
		da_combine_update_bits(codec, ADC2L_PGA_CTRL0_REG,
			BIT(ADC2L_PGA_BYPASS_OFFSET),
			BIT(ADC2L_PGA_BYPASS_OFFSET));
		/* ultrasonic up ADC2L FIR2D bypass enable */
		da_combine_update_bits(codec, ADC2L_CTRL0_REG,
			BIT(ADC2L_IR2D_BYPASS_EN_OFFSET),
			BIT(ADC2L_IR2D_BYPASS_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* ultrasonic up ADC2L FIR2D bypass disable */
		da_combine_update_bits(codec, ADC2L_CTRL0_REG,
			BIT(ADC2L_IR2D_BYPASS_EN_OFFSET), 0);
		/* set adc2l_pga default */
		da_combine_update_bits(codec, ADC2L_PGA_CTRL0_REG,
			BIT(ADC2L_PGA_BYPASS_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int virtbtn_active_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		AUDIO_LOGI("virtbtn active switch power event: %d", event);
		da_combine_update_bits(codec, DSPIF_VLD_CTRL4_REG,
			0x7 << DSPIF8_DIN_VLD_SEL_OFFSET,
			0x4 << DSPIF8_DIN_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		AUDIO_LOGI("virtbtn active switch power event: %d", event);
		da_combine_update_bits(codec, DSPIF_VLD_CTRL4_REG,
			0x7 << DSPIF8_DIN_VLD_SEL_OFFSET,
			0x0 << DSPIF8_DIN_VLD_SEL_OFFSET);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int virtbtn_passive_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec;

	codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		AUDIO_LOGI("virtbtn passive switch power event: %d", event);
		da_combine_update_bits(codec, APB_CLK_CFG_REG,
			BIT(GPIO_PCLK_EN_OFFSET),
			BIT(GPIO_PCLK_EN_OFFSET));
		da_combine_update_bits(codec, DSPIF_VLD_CTRL5_REG,
			0x7 << DSPIF9_DIN_VLD_SEL_OFFSET,
			0x0 << DSPIF9_DIN_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		AUDIO_LOGI("virtbtn passive switch power event: %d", event);
		da_combine_update_bits(codec, DSPIF_VLD_CTRL5_REG,
			0x7 << DSPIF9_DIN_VLD_SEL_OFFSET,
			0x0 << DSPIF9_DIN_VLD_SEL_OFFSET);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int ir_study_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
			BIT(CODEC_PD_INF_EMS_OFFSET),
			BIT(CODEC_PD_INF_EMS_OFFSET));
		da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
			BIT(CODEC_PD_INF_LRN_OFFSET),
			0x0 << CODEC_PD_INF_LRN_OFFSET);

		da_combine_update_bits(codec, ANA_IR_EN,
			0x3 << ANA_IR_INF_SEL_OFFSET,
			BIT(ANA_IR_INF_SEL_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, ANA_IR_EN,
			0x3 << ANA_IR_INF_SEL_OFFSET, 0);
		da_combine_update_bits(codec, CODEC_ANA_RWREG_052,
			BIT(CODEC_PD_INF_LRN_OFFSET),
			BIT(CODEC_PD_INF_LRN_OFFSET));
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int soundtrigger_u3u4_power_event(struct snd_soc_component *codec,
	struct slimbus_track_param *params, int event)
{
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* mic3/4 in&&out sample rate */
		da_combine_update_bits(codec, SRC_VLD_CTRL3_REG,
			BIT(MIC3_SRC_DIN_VLD_SEL_OFFSET) | 0x7 << MIC3_SRC_DOUT_VLD_SEL_OFFSET,
			BIT(MIC3_SRC_DIN_VLD_SEL_OFFSET) | SAMPLE_RATE_REG_CFG_16K << MIC3_SRC_DOUT_VLD_SEL_OFFSET);
		da_combine_update_bits(codec, SRC_VLD_CTRL4_REG,
			BIT(MIC4_SRC_DIN_VLD_SEL_OFFSET) | 0x7 << MIC4_SRC_DOUT_VLD_SEL_OFFSET,
			BIT(MIC4_SRC_DIN_VLD_SEL_OFFSET) | SAMPLE_RATE_REG_CFG_16K << MIC4_SRC_DOUT_VLD_SEL_OFFSET);
		/* u3/u4 clk clk */
		da_combine_update_bits(codec, S1_DP_CLK_EN_REG,
			BIT(S1_MIC4_SRC_CLK_EN_OFFSET) | BIT(S1_MIC3_SRC_CLK_EN_OFFSET), 0x0);
		/* u3/u4 src mode */
		da_combine_update_bits(codec, MIC3_SRC_CTRL_REG,
			0x7 << MIC3_SRC_MODE_OFFSET,
			SRC_MODE_6 << MIC3_SRC_MODE_OFFSET);
		da_combine_update_bits(codec, MIC4_SRC_CTRL_REG,
			0x7 << MIC4_SRC_MODE_OFFSET,
			SRC_MODE_6 << MIC4_SRC_MODE_OFFSET);
		/* u3/u4 clk clk */
		da_combine_update_bits(codec, S1_DP_CLK_EN_REG,
			BIT(S1_MIC4_SRC_CLK_EN_OFFSET) | BIT(S1_MIC3_SRC_CLK_EN_OFFSET),
			BIT(S1_MIC4_SRC_CLK_EN_OFFSET) | BIT(S1_MIC3_SRC_CLK_EN_OFFSET));

		/* u3/u4 slimbus sample rate */
		snd_soc_component_write(codec, SC_FS_SLIM_CTRL_4_REG,
			SAMPLE_RATE_REG_CFG_16K << FS_SLIM_UP4_OFFSET | SAMPLE_RATE_REG_CFG_16K << FS_SLIM_UP3_OFFSET);

		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_KWS, params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track kws err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* slimbus soundtrigger deactive */
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_KWS, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track kws err: %d", ret);

		da_combine_update_bits(codec, S1_DP_CLK_EN_REG,
			BIT(S1_MIC4_SRC_CLK_EN_OFFSET) | BIT(S1_MIC3_SRC_CLK_EN_OFFSET), 0x0);
		snd_soc_component_write(codec, SC_FS_SLIM_CTRL_4_REG,
			0x4 << FS_SLIM_UP4_OFFSET | 0x4 << FS_SLIM_UP3_OFFSET);
		da_combine_update_bits(codec, SRC_VLD_CTRL3_REG,
			BIT(MIC3_SRC_DIN_VLD_SEL_OFFSET) | 0x7 << MIC3_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
		da_combine_update_bits(codec, SRC_VLD_CTRL4_REG,
			BIT(MIC4_SRC_DIN_VLD_SEL_OFFSET) | 0x7 << MIC4_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
		da_combine_update_bits(codec, MIC3_SRC_CTRL_REG,
			0x7 << MIC3_SRC_MODE_OFFSET, 0x0);
		da_combine_update_bits(codec, MIC4_SRC_CTRL_REG,
			0x7 << MIC4_SRC_MODE_OFFSET, 0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int soundtrigger_onemic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	platform_data->soundtrigger_params.channels = 1;

	return soundtrigger_u3u4_power_event(codec,
		&platform_data->soundtrigger_params, event);
}

static int soundtrigger_dualmic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	platform_data->soundtrigger_params.channels = 2;

	return soundtrigger_u3u4_power_event(codec,
		&platform_data->soundtrigger_params, event);
}



int ec_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ec_switch_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ec_switch_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}


static const struct reg_seq_config bluetooth_loop_up_regs[] = {
	/* s2 func mode  PCM STD */
	{ { SC_S2_IF_L_REG, 0x7 << S2_FUNC_MODE_OFFSET,
		0x2 << S2_FUNC_MODE_OFFSET, true }, 0, 0 },
	/* s2 direct loop Sdin->Sdout */
	{ { SC_S2_IF_L_REG, 0x3 << S2_DIRECT_LOOP_OFFSET,
		BIT(S2_DIRECT_LOOP_OFFSET), true }, 0, 0 },
	/* s2 mater mode */
	{ { SC_S2_IF_L_REG, BIT(S2_MST_SLV_OFFSET),
		0x0 << S2_MST_SLV_OFFSET, true }, 0, 0 },
	/* s2 if rx en */
	{ { SC_S2_IF_L_REG, BIT(S2_IF_RX_ENA_OFFSET),
		BIT(S2_IF_RX_ENA_OFFSET), true }, 0, 0 },
	/* s2 if tx en */
	{ { SC_S2_IF_L_REG, BIT(S2_IF_TX_ENA_OFFSET),
		BIT(S2_IF_TX_ENA_OFFSET), true }, 0, 0 },
	/* s2 clk if en */
	{ { SC_CODEC_MUX_CTRL26_REG, BIT(S2_CLK_IF_EN_OFFSET),
		BIT(S2_CLK_IF_EN_OFFSET), true }, 0, 0 },
	/* s2 freq */
	{ { SC_CODEC_MUX_CTRL23_REG, 0x7 << FS_S2_OFFSET,
		0x0 << FS_S2_OFFSET, true }, 0, 0 },
	/* s2 frame mode */
	{ { SC_S2_IF_H_REG, BIT(S2_FRAME_MODE_OFFSET),
		0x0 << S2_FRAME_MODE_OFFSET, true }, 0, 0 },
	/* s2_clk_if_txrx_en */
	{ { SC_CODEC_MUX_CTRL26_REG, BIT(S2_CLK_IF_TXRX_EN_OFFSET),
		BIT(S2_CLK_IF_TXRX_EN_OFFSET), true }, 0, 0 },
};

static const struct reg_seq_config bluetooth_loop_dn_regs[] = {
	/* s2_clk_if_txrx_en */
	{ { SC_CODEC_MUX_CTRL26_REG, BIT(S2_CLK_IF_TXRX_EN_OFFSET), 0, true }, 0, 0 },
	/* s2 frame mode */
	{ { SC_S2_IF_H_REG, BIT(S2_FRAME_MODE_OFFSET), 0, true }, 0, 0 },
	/* s2 freq */
	{ { SC_CODEC_MUX_CTRL23_REG, 0x7 << FS_S2_OFFSET, 0, true }, 0, 0 },
	/* s2 clk if en */
	{ { SC_CODEC_MUX_CTRL26_REG, BIT(S2_CLK_IF_EN_OFFSET), 0, true }, 0, 0 },
	/* s2 if tx en */
	{ { SC_S2_IF_L_REG, BIT(S2_IF_TX_ENA_OFFSET), 0, true }, 0, 0 },
	/* s2 if rx en */
	{ { SC_S2_IF_L_REG, BIT(S2_IF_RX_ENA_OFFSET), 0, true }, 0, 0 },
	/* s2 mater mode */
	{ { SC_S2_IF_L_REG, BIT(S2_MST_SLV_OFFSET), 0, true }, 0, 0 },
	/* s2 direct loop Sdin->Sdout */
	{ { SC_S2_IF_L_REG, 0x3 << S2_DIRECT_LOOP_OFFSET, 0, true }, 0, 0 },
	/* s2 func mode  PCM STD */
	{ { SC_S2_IF_L_REG, 0x7 << S2_FUNC_MODE_OFFSET, 0, true }, 0, 0 },
};

static int i2s2_bluetooth_loop_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		write_reg_seq_array(codec, bluetooth_loop_up_regs,
			ARRAY_SIZE(bluetooth_loop_up_regs));
		break;
	case SND_SOC_DAPM_POST_PMD:
		write_reg_seq_array(codec, bluetooth_loop_dn_regs,
			ARRAY_SIZE(bluetooth_loop_dn_regs));
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static const struct reg_seq_config moveup_2pa_up_regs[] = {
	/* bypass S4 UP bit match */
	{ { SC_CODEC_MUX_CTRL1_REG, MASK_ON_BIT(S4_O_BITMATCH_BYP_EN_LEN,
		S4_O_BITMATCH_BYP_EN_OFFSET), BIT(S4_O_BITMATCH_BYP_EN_OFFSET), true }, 0, 0 },
	/* close u7 src clk */
	{ { S4_DP_CLK_EN_REG, MASK_ON_BIT(S4_SPA_R_SRC_CLK_EN_LEN,
		S4_SPA_R_SRC_CLK_EN_OFFSET), 0x0, true }, 0, 0 },
	/* sel u7 data source */
	{ { SC_CODEC_MUX_CTRL37_REG, MASK_ON_BIT(SLIM_SW_UP7_DATA_SEL_LEN,
		SLIM_SW_UP7_DATA_SEL_OFFSET), 0x2 << SLIM_SW_UP7_DATA_SEL_OFFSET, true }, 0, 0 },
	/* config u7 48k */
	{ { SC_FS_SLIM_CTRL_6_REG, MASK_ON_BIT(FS_SLIM_UP7_LEN, FS_SLIM_UP7_OFFSET),
		0x4 << FS_SLIM_UP7_OFFSET, true }, 0, 0 },
	/* close u8 src clk */
	{ { S4_DP_CLK_EN_REG, MASK_ON_BIT(S4_SPA_L_SRC_CLK_EN_LEN,
		S4_SPA_L_SRC_CLK_EN_OFFSET), 0x0, true }, 0, 0 },
	/* sel u8 data source */
	{ { SC_CODEC_MUX_CTRL37_REG, MASK_ON_BIT(SLIM_SW_UP8_DATA_SEL_LEN,
		SLIM_SW_UP8_DATA_SEL_OFFSET), 0x2 << SLIM_SW_UP8_DATA_SEL_OFFSET, true }, 0, 0 },
	/* config u8 48k */
	{ { SC_FS_SLIM_CTRL_6_REG, MASK_ON_BIT(FS_SLIM_UP8_LEN, FS_SLIM_UP8_OFFSET),
		0x4 << FS_SLIM_UP8_OFFSET, true }, 0, 0 },
	/* 32bit */
	{ { SC_S4_IF_H_REG, MASK_ON_BIT(S4_CODEC_IO_WORDLENGTH_LEN,
		S4_CODEC_IO_WORDLENGTH_OFFSET),
		0x3 << S4_CODEC_IO_WORDLENGTH_OFFSET, true }, 0, 0 },
};

static const struct reg_seq_config moveup_2pa_dn_regs[] = {
	{ { SC_CODEC_MUX_CTRL37_REG, MASK_ON_BIT(SLIM_SW_UP7_DATA_SEL_LEN,
		SLIM_SW_UP7_DATA_SEL_OFFSET), 0x0, true }, 0, 0 },
	{ { SC_CODEC_MUX_CTRL37_REG, MASK_ON_BIT(SLIM_SW_UP8_DATA_SEL_LEN,
		SLIM_SW_UP8_DATA_SEL_OFFSET), 0x0, true }, 0, 0 },
	{ { SC_S4_IF_H_REG, MASK_ON_BIT(S4_CODEC_IO_WORDLENGTH_LEN,
		S4_CODEC_IO_WORDLENGTH_OFFSET), 0x0, true }, 0, 0 },
};

static int moveup_2pa_tdm_iv_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *priv = snd_soc_component_get_drvdata(codec);
	int ret = 0;

	priv->pa_iv_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->pa_iv_params.channels = 2;
	priv->play_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->play_params.channels = 2;
	AUDIO_LOGI("power event: %d\n", event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		write_reg_seq_array(codec, moveup_2pa_up_regs,
			ARRAY_SIZE(moveup_2pa_up_regs));

		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV, &priv->pa_iv_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track 4pa iv err: %d", ret);
		/* 2PA need active D1\D2 ,it contol by PLAY48K_SWITCH */
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* sel reg default config */
		write_reg_seq_array(codec, moveup_2pa_dn_regs,
			ARRAY_SIZE(moveup_2pa_dn_regs));

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV, &priv->pa_iv_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track 4pa iv err: %d", ret);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static const struct reg_seq_config moveup_4pa_up_regs[] = {
	/* sel u5 data source */
	{ { SC_CODEC_MUX_CTRL25_REG, MASK_ON_BIT(SLIM_UP5_DATA_SEL_LEN,
		SLIM_UP5_DATA_SEL_OFFSET), 0x2 << SLIM_UP5_DATA_SEL_OFFSET, true }, 0, 0 },
	/* config u5 48k */
	{ { SC_FS_SLIM_CTRL_5_REG, MASK_ON_BIT(FS_SLIM_UP5_LEN,
		FS_SLIM_UP5_OFFSET), 0x4 << FS_SLIM_UP5_OFFSET, true }, 0, 0 },
	/* sel u6 data source */
	{ { SC_CODEC_MUX_CTRL22_REG, MASK_ON_BIT(SLIM_UP6_DATA_SEL_LEN,
		SLIM_UP6_DATA_SEL_OFFSET), 0x2 << SLIM_UP6_DATA_SEL_OFFSET, true }, 0, 0 },
	/* config u6 48k */
	{ { SC_FS_SLIM_CTRL_5_REG, MASK_ON_BIT(FS_SLIM_UP6_LEN,
		FS_SLIM_UP6_OFFSET), 0x4 << FS_SLIM_UP6_OFFSET, true }, 0, 0 },
	/* bypass S4 UP bit match */
	{ { SC_CODEC_MUX_CTRL1_REG, MASK_ON_BIT(S4_O_BITMATCH_BYP_EN_LEN,
		S4_O_BITMATCH_BYP_EN_OFFSET), BIT(S4_O_BITMATCH_BYP_EN_OFFSET), true }, 0, 0 },
	/* close u7 src clk */
	{ { S4_DP_CLK_EN_REG, MASK_ON_BIT(S4_SPA_R_SRC_CLK_EN_LEN,
		S4_SPA_R_SRC_CLK_EN_OFFSET), 0x0, true }, 0, 0 },
	/* sel u7 data source */
	{ { SC_CODEC_MUX_CTRL37_REG, MASK_ON_BIT(SLIM_SW_UP7_DATA_SEL_LEN,
		SLIM_SW_UP7_DATA_SEL_OFFSET), 0x2 << SLIM_SW_UP7_DATA_SEL_OFFSET, true }, 0, 0 },
	/* config u7 48k */
	{ { SC_FS_SLIM_CTRL_6_REG, MASK_ON_BIT(FS_SLIM_UP7_LEN,
		FS_SLIM_UP7_OFFSET), 0x4 << FS_SLIM_UP7_OFFSET, true }, 0, 0 },
	/* close u8 src clk */
	{ { S4_DP_CLK_EN_REG, MASK_ON_BIT(S4_SPA_L_SRC_CLK_EN_LEN,
		S4_SPA_L_SRC_CLK_EN_OFFSET), 0x0, true }, 0, 0 },
	/* sel u8 data source */
	{ { SC_CODEC_MUX_CTRL37_REG, MASK_ON_BIT(SLIM_SW_UP8_DATA_SEL_LEN,
		SLIM_SW_UP8_DATA_SEL_OFFSET), 0x2 << SLIM_SW_UP8_DATA_SEL_OFFSET, true }, 0, 0 },
	/* config u8 48k */
	{ { SC_FS_SLIM_CTRL_6_REG, MASK_ON_BIT(FS_SLIM_UP8_LEN,
		FS_SLIM_UP8_OFFSET), 0x4 << FS_SLIM_UP8_OFFSET, true }, 0, 0 },
	/* 32bit */
	{ { SC_S4_IF_H_REG, MASK_ON_BIT(S4_CODEC_IO_WORDLENGTH_LEN,
		S4_CODEC_IO_WORDLENGTH_OFFSET), 0x3 << S4_CODEC_IO_WORDLENGTH_OFFSET, true }, 0, 0 },
};

static const struct reg_seq_config moveup_4pa_dn_regs[] = {
	/* sel reg default config */
	{ { SC_CODEC_MUX_CTRL25_REG, MASK_ON_BIT(SLIM_UP5_DATA_SEL_LEN,
		SLIM_UP5_DATA_SEL_OFFSET), 0x0, true }, 0, 0 },
	{ { SC_CODEC_MUX_CTRL22_REG, MASK_ON_BIT(SLIM_UP6_DATA_SEL_LEN,
		SLIM_UP6_DATA_SEL_OFFSET), 0x0, true }, 0, 0 },
	{ { SC_CODEC_MUX_CTRL37_REG, MASK_ON_BIT(SLIM_SW_UP7_DATA_SEL_LEN,
		SLIM_SW_UP7_DATA_SEL_OFFSET), 0x0, true }, 0, 0 },
	{ { SC_CODEC_MUX_CTRL37_REG, MASK_ON_BIT(SLIM_SW_UP8_DATA_SEL_LEN,
		SLIM_SW_UP8_DATA_SEL_OFFSET), 0x0, true }, 0, 0 },
	{ { SC_S4_IF_H_REG, MASK_ON_BIT(S4_CODEC_IO_WORDLENGTH_LEN,
		S4_CODEC_IO_WORDLENGTH_OFFSET), 0x0, true }, 0, 0 },
};

static int moveup_4pa_tdm_iv_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *priv = snd_soc_component_get_drvdata(codec);
	int ret = 0;

	priv->pa_iv_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->pa_iv_params.channels = 4;
	priv->play_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->play_params.channels = 2;
	AUDIO_LOGI("power event: %d\n", event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		write_reg_seq_array(codec, moveup_4pa_up_regs,
			ARRAY_SIZE(moveup_4pa_up_regs));

		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_4PA_IV, &priv->pa_iv_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track 4pa iv err: %d", ret);
		/* D1/D2 active by PLAY48K_SWITCH, D5/D6 is active here */
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN, &priv->play_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track D5 D6 err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		write_reg_seq_array(codec, moveup_4pa_dn_regs,
			ARRAY_SIZE(moveup_4pa_dn_regs));

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_4PA_IV, &priv->pa_iv_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track 4pa iv err: %d", ret);
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
			SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN, &priv->play_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track D5 D6 err: %d", ret);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int tdm_audio_pa_down_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	int ret = 0;

	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* config SPA_OL/OR(if7) select dn1 dn2,it is defalut value */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
			MASK_ON_BIT(SPA_OL_SRC_DIN_SEL_LEN, SPA_OL_SRC_DIN_SEL_OFFSET) |
			MASK_ON_BIT(SPA_OR_SRC_DIN_SEL_LEN, SPA_OR_SRC_DIN_SEL_OFFSET),
			0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int tdm_audio_pa_down_loop_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	int ret = 0;

	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* config SPA_OL/OR(if7) select dspif out put */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
			MASK_ON_BIT(SPA_OL_SRC_DIN_SEL_LEN, SPA_OL_SRC_DIN_SEL_OFFSET) |
			MASK_ON_BIT(SPA_OR_SRC_DIN_SEL_LEN, SPA_OR_SRC_DIN_SEL_OFFSET),
			0x2 << SPA_OR_SRC_DIN_SEL_OFFSET |
			0x2 << SPA_OL_SRC_DIN_SEL_OFFSET);
		/* dspif7_tdm_trans clk on */
		da_combine_update_bits(codec, DMIC_CLK_EN_REG,
			MASK_ON_BIT(DSPIF7_TDM_TRANS_CLK_EN_LEN,
			DSPIF7_TDM_TRANS_CLK_EN_OFFSET),
			BIT(DSPIF7_TDM_TRANS_CLK_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* config SPA_OL/OR(if7) ,select dn1 dn2,it is defalut value */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
			MASK_ON_BIT(SPA_OL_SRC_DIN_SEL_LEN, SPA_OL_SRC_DIN_SEL_OFFSET) |
			MASK_ON_BIT(SPA_OR_SRC_DIN_SEL_LEN, SPA_OR_SRC_DIN_SEL_OFFSET), 0x0);
		/* dspif7_tdm_trans clk close */
		da_combine_update_bits(codec, DMIC_CLK_EN_REG,
			MASK_ON_BIT(DSPIF7_TDM_TRANS_CLK_EN_LEN, DSPIF7_TDM_TRANS_CLK_EN_OFFSET), 0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}
static int play44k1_gain_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	int ret = 0;
	unsigned int val;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		val = snd_soc_component_read32(codec, DACL_MIXER4_CTRL2_REG);
		snd_soc_component_write(codec, DACL_MIXER4_CTRL2_REG, val);
		val = snd_soc_component_read32(codec, DACR_MIXER4_CTRL2_REG);
		snd_soc_component_write(codec, DACR_MIXER4_CTRL2_REG, val);
		val = snd_soc_component_read32(codec, DACL_PRE_MIXER2_CTRL1_REG);
		snd_soc_component_write(codec, DACL_PRE_MIXER2_CTRL1_REG, val);
		val = snd_soc_component_read32(codec, DACR_PRE_MIXER2_CTRL1_REG);
		snd_soc_component_write(codec, DACR_PRE_MIXER2_CTRL1_REG, val);
		val = snd_soc_component_read32(codec, DACL_POST_MIXER2_CTRL1_REG);
		snd_soc_component_write(codec, DACL_POST_MIXER2_CTRL1_REG, val);
		val = snd_soc_component_read32(codec, DACR_POST_MIXER2_CTRL1_REG);
		snd_soc_component_write(codec, DACR_POST_MIXER2_CTRL1_REG, val);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

#define SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("PLAY44K1_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play44k1_switch_controls, \
		play44k1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY88K2_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play88k2_switch_controls, \
		play88k2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY176K4_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play176k4_switch_controls, \
		play176k4_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY48K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play48k_switch_controls, \
		play48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY96K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play96k_switch_controls, \
		play96k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY192K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play192k_switch_controls, \
		play192k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY384K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play384k_switch_controls, \
		play384k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("HP_HIGHLEVEL_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_hp_high_switch_controls, \
		hp_high_level_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("HP_CONCURRENCY_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_hp_concurrency_switch_controls, \
		hp_concurrency_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("S2_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_s2_ol_switch_controls, \
		s2up_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("S2_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s2_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("S4_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s4_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("S4_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s4_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("PLAY384K_VIR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_play384k_vir_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("IV_DSPIF_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_iv_dspif_switch_controls, \
		iv_dspif_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("IV_2PA_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_iv_2pa_switch_controls, \
		iv_2pa_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("IV_4PA_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_iv_4pa_switch_controls), \
	SND_SOC_DAPM_SWITCH("IR_EMISSION_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_ir_emission_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("AUDIOUP_2MIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_audioup_2mic_switch_controls, \
		audioup_2mic_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIOUP_4MIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_audioup_4mic_switch_controls, \
		audioup_4mic_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIOUP_4MIC_V2_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_audioup_4mic_v2_switch_controls, \
		audioup_4mic_v2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIOUP_5MIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_audioup_5mic_switch_controls, \
		audioup_5mic_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("ULTRA_UP9_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_ultra_up9_switch_controls, \
		ultra_up9_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE8K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice8k_switch_controls, \
		voice8k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE16K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice16k_switch_controls, \
		voice16k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE32K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice32k_switch_controls, \
		voice32k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE8K_V2_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice8k_v2_switch_controls, \
		voice8k_v2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE16K_V2_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice16k_v2_switch_controls, \
		voice16k_v2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE32K_V2_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice32k_v2_switch_controls, \
		voice32k_v2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE8K_4PA_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice8k_4pa_switch_controls, \
		voice8k_4pa_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE16K_4PA_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice16k_4pa_switch_controls, \
		voice16k_4pa_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE32K_4PA_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice32k_4pa_switch_controls, \
		voice32k_4pa_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("MOVEUP_2PA_TDM_IV_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_moveup_2pa_tdm_iv_switch_controls, \
		moveup_2pa_tdm_iv_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("MOVEUP_4PA_TDM_IV_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_moveup_4pa_tdm_iv_switch_controls, \
		moveup_4pa_tdm_iv_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("MAD_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_mad_switch_controls, \
		madswitch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("ULTRASONIC_UP_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_ultrasonic_up_switch_controls, \
		ultrasonic_up_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("S1_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s1_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("S1_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s1_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("S3_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s3_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("S3_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s3_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("U3_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_u3_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("U4_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_u4_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("U5_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_u5_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("US_R1_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_us_r1_switch_controls), \
	SND_SOC_DAPM_SWITCH("US_R2_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_us_r2_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("IR_STUDY_ENV_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_ir_env_study_switch_controls, \
		ir_study_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("SOUNDTRIGGER_ONEMIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_soundtrigger_onemic_switch_controls, \
		soundtrigger_onemic_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("SOUNDTRIGGER_DUALMIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_soundtrigger_dualmic_switch_controls, \
		soundtrigger_dualmic_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("U7_EC_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_ec_switch_controls, \
		ec_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("AUXMIC_HSMICBIAS_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_auxmic_hsmicbias_switch_controls), \
	SND_SOC_DAPM_SWITCH("AUXMIC_MICBIAS2_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_auxmic_micbias2_switch_controls), \
	SND_SOC_DAPM_SWITCH("AUXMIC_MICBIAS1_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_auxmic_micbias1_switch_controls), \
	SND_SOC_DAPM_SWITCH("MIC4_MICBIAS1_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic4_micbias1_switch_controls), \
	SND_SOC_DAPM_SWITCH("MIC5_MICBIAS1_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic5_micbias1_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("TDM_AUDIO_PA_DOWN_SWITCH",\
		SND_SOC_NOPM, 0, 0, &dapm_tdm_audio_pa_down_switch_controls,\
		tdm_audio_pa_down_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),\
	SND_SOC_DAPM_SWITCH_E("TDM_AUDIO_PA_DN_LOOP_SWITCH",\
		SND_SOC_NOPM, 0, 0, &dapm_tdm_audio_pa_down_loop_switch_controls,\
		tdm_audio_pa_down_loop_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),\
	SND_SOC_DAPM_SWITCH_E("FM_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_fm_switch_controls, \
		fm_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("FM_HIFI_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_fm_hifi_switch_controls, \
		fm_hifi_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VIRTUAL_BTN_ACTIVE_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_virtbtn_active_switch_controls, \
		virtbtn_active_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VIRTUAL_BTN_PASSIVE_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_virtbtn_passive_switch_controls, \
		virtbtn_passive_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY44K1_GAIN_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play44k1_gain_switch_controls, \
		play44k1_gain_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

#define I2S2_BT_LOOP_SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("I2S2_BLUETOOTH_LOOP_SWITCH",\
		SND_SOC_NOPM, 0, 0, &dapm_i2s2_bluetooth_loop_switch_controls,\
		i2s2_bluetooth_loop_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),\

static const struct snd_soc_dapm_widget switch_widgets[] = {
	SWITCH_WIDGET
	I2S2_BT_LOOP_SWITCH_WIDGET
};

static const struct snd_soc_dapm_widget switch_widgets_for_single_kcontrol[] = {
	I2S2_BT_LOOP_SWITCH_WIDGET
};

int da_combine_v5_add_switch_widgets(struct snd_soc_component *codec, bool single_kcontrol)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	if (single_kcontrol)
		return snd_soc_dapm_new_controls(dapm, switch_widgets_for_single_kcontrol,
			ARRAY_SIZE(switch_widgets_for_single_kcontrol));
	else
		return snd_soc_dapm_new_controls(dapm, switch_widgets,
			ARRAY_SIZE(switch_widgets));
}


