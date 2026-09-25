/*
 * hi6405_switch_widget.c -- hi6405 codec driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "hi6405_switch_widget.h"

#include <sound/core.h>
#include <sound/tlv.h>

#include "slimbus.h"
#include "slimbus_6405.h"
#include "linux/hisi/audio_log.h"
#include "linux/hisi/hi64xx/hi6405.h"
#include "linux/hisi/hi64xx/hi6405_regs.h"
#include "linux/hisi/hi64xx/hi6405_type.h"
#include "linux/hisi/hi64xx/hi64xx_utils.h"
#include "hi6405_path_widget.h"
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
static const struct snd_kcontrol_new dapm_virtbtn_active_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VIRTUAL_BTN_ACTIVE_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_virtbtn_passive_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_UP_REG, VIRTUAL_BTN_PASSIVE_BIT, 1, 0);

static void u12_select_pga(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL25_REG,
		0x3 << SLIM_UP12_DATA_SEL_OFFSET,
		0x0 << SLIM_UP12_DATA_SEL_OFFSET);
}

static void u12_select_dspif(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL25_REG,
		0x3 << SLIM_UP12_DATA_SEL_OFFSET,
		0x1 << SLIM_UP12_DATA_SEL_OFFSET);
}

static void u56_select_dspif(struct snd_soc_codec *codec)
{
	/* U5 U6 selesct dspif for wakeup app may died */
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL22_REG,
		0x3 << SLIM_UP6_DATA_SEL_OFFSET,
		0x1 << SLIM_UP6_DATA_SEL_OFFSET);
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL25_REG,
		0x3 << SLIM_UP5_DATA_SEL_OFFSET,
		0x1 << SLIM_UP5_DATA_SEL_OFFSET);
}

static void u56_select_pga(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL22_REG,
		0x3 << SLIM_UP6_DATA_SEL_OFFSET,
		0x0 << SLIM_UP6_DATA_SEL_OFFSET);
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL25_REG,
		0x3 << SLIM_UP5_DATA_SEL_OFFSET,
		0x0 << SLIM_UP5_DATA_SEL_OFFSET);
}

static void u10_select_mic_src(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL25_REG,
		0x3 << SLIM_UP10_DATA_SEL_OFFSET,
		0x3 << SLIM_UP10_DATA_SEL_OFFSET);
}

static void u10_select_dspif(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL25_REG,
		0x3 << SLIM_UP10_DATA_SEL_OFFSET,
		0x1 << SLIM_UP10_DATA_SEL_OFFSET);
}

static int voice_slimbus_active(struct snd_soc_codec *codec,
	struct hi6405_platform_data *platform_data)
{
	int ret;

	u56_select_pga(codec);
	if (platform_data->voice_up_params.channels == 4) {
		hi64xx_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			0x1 << RST_5MIC_S3_ACCESS_IRQ_OFFSET,
			0x1 << RST_5MIC_S3_ACCESS_IRQ_OFFSET);
	}

	/* slimbus voice active */
	ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
		SLIMBUS_6405_TRACK_VOICE_UP, &platform_data->voice_up_params);
	if (ret != 0)
		AUDIO_LOGE("slimbus_activate_track voice up err: %d", ret);

	if (platform_data->voice_up_params.channels == 4) {
		hi64xx_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			0x1 << RST_5MIC_S3_ACCESS_IRQ_OFFSET, 0);
	}

	platform_data->voiceup_state = TRACK_STARTUP;

	return ret;
}

static int voice_slimbus_deactive(struct snd_soc_codec *codec,
	struct hi6405_platform_data *platform_data)
{
	int ret;

	/* slimbus voice deactive */
	ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
		SLIMBUS_6405_TRACK_VOICE_UP, NULL);
	if (ret != 0)
		AUDIO_LOGE("slimbus_deactivate_track voice up err: %d", ret);

	platform_data->voiceup_state = TRACK_FREE;
	u56_select_dspif(codec);

	return ret;
}

static int slimbus_param_pass(struct snd_soc_codec *codec,
	enum slimbus_6405_track track, struct slimbus_track_param *params, int event)
{
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (params == NULL) {
			snd_soc_write(codec, SC_FS_SLIM_CTRL_0_REG, 0x44);
		} else {
			if (params->rate == SLIMBUS_SAMPLE_RATE_96K || params->rate == SLIMBUS_SAMPLE_RATE_88K2)
				snd_soc_write(codec, SC_FS_SLIM_CTRL_0_REG, 0x55);
			else if (params->rate == SLIMBUS_SAMPLE_RATE_192K || params->rate == SLIMBUS_SAMPLE_RATE_176K4)
				snd_soc_write(codec, SC_FS_SLIM_CTRL_0_REG, 0x66);
			else if (params->rate == SLIMBUS_SAMPLE_RATE_384K)
				snd_soc_write(codec, SC_FS_SLIM_CTRL_0_REG, 0x77);
			else
				snd_soc_write(codec, SC_FS_SLIM_CTRL_0_REG, 0x44);
		}
		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			track, params);
		if (ret != 0)
			AUDIO_LOGE("slimbus activate err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			track, params);
		if (ret != 0)
			AUDIO_LOGE("slimbus deactivate err: %d", ret);
		snd_soc_write(codec, SC_FS_SLIM_CTRL_0_REG, 0x44);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		ret = -1;
		break;
	}

	return ret;
}

static int play48k_path_config(struct snd_soc_codec *codec, int event)
{
	int ret = 0;

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
			0x1 << S1_IL_SRC_CLK_EN_OFFSET | 0x1 << S1_IR_SRC_CLK_EN_OFFSET,
			0x1 << S1_IL_SRC_CLK_EN_OFFSET | 0x1 << S1_IR_SRC_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
			0x1 << DAC_MIXER4_VLD_SEL_OFFSET | 0x1 << DAC_PRE_MIXER2_VLD_SEL_OFFSET,
			0x1 << DAC_MIXER4_VLD_SEL_OFFSET | 0x1 << DAC_PRE_MIXER2_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
			0x1 << DAC_MIXER4_VLD_SEL_OFFSET  | 0x1 << DAC_PRE_MIXER2_VLD_SEL_OFFSET, 0);
		hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
			0x1 << S1_IL_SRC_CLK_EN_OFFSET | 0x1 << S1_IR_SRC_CLK_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return ret;
}

static void bypass_dac_fir21(struct snd_soc_codec *codec, bool flag)
{
	if (flag) {
		hi64xx_update_bits(codec, DACL_CTRL_REG,
			BIT(DACL_IR2I_BYPASS_EN_OFFSET), BIT(DACL_IR2I_BYPASS_EN_OFFSET));
		hi64xx_update_bits(codec, DACR_CTRL_REG,
			BIT(DACR_IR2I_BYPASS_EN_OFFSET), BIT(DACR_IR2I_BYPASS_EN_OFFSET));
	} else {
		hi64xx_update_bits(codec, DACL_CTRL_REG,
			BIT(DACL_IR2I_BYPASS_EN_OFFSET), 0);
		hi64xx_update_bits(codec, DACR_CTRL_REG,
			BIT(DACR_IR2I_BYPASS_EN_OFFSET), 0);
	}
}

static int play96k_path_config(struct snd_soc_codec *codec, int event)
{
	int ret = 0;

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
			0x1 << DAC_MIXER4_VLD_SEL_OFFSET | 0x1 << DAC_PRE_MIXER2_VLD_SEL_OFFSET,
			0x1 << DAC_MIXER4_VLD_SEL_OFFSET | 0x1 << DAC_PRE_MIXER2_VLD_SEL_OFFSET);
		bypass_dac_fir21(codec, true);
		break;
	case SND_SOC_DAPM_POST_PMD:
		bypass_dac_fir21(codec, false);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
			0x1 << DAC_MIXER4_VLD_SEL_OFFSET | 0x1 << DAC_PRE_MIXER2_VLD_SEL_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return ret;
}

static int play192k_path_config(struct snd_soc_codec *codec, int event)
{
	int ret = 0;

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL34_REG,
			0x1 << DACL_POST_MIXER2_DVLD_SEL_OFFSET | 0x1 << DACR_POST_MIXER2_DVLD_SEL_OFFSET,
			0x1 << DACL_POST_MIXER2_DVLD_SEL_OFFSET | 0x1 << DACR_POST_MIXER2_DVLD_SEL_OFFSET);
		bypass_dac_fir21(codec, true);
		break;
	case SND_SOC_DAPM_POST_PMD:
		bypass_dac_fir21(codec, false);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL34_REG,
			0x1 << DACL_POST_MIXER2_DVLD_SEL_OFFSET | 0x1 << DACR_POST_MIXER2_DVLD_SEL_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return ret;
}

static void set_slimbus_fifo_for_44k1(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SLIM_SYNC_CTRL2_REG,
		MASK_ON_BIT(SLIM_SYNC_FIFO_AEMPTY_TH_LEN, SLIM_SYNC_FIFO_AEMPTY_TH_OFFSET),
		0x2c);
	hi64xx_update_bits(codec, SLIM_SYNC_CTRL1_REG,
		MASK_ON_BIT(SLIM_SYNC_FIFO_AF0_TH_LEN, SLIM_SYNC_FIFO_AF0_TH_OFFSET),
		0x3c);
}

static void set_slimbus_fifo_for_default(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SLIM_SYNC_CTRL2_REG,
		MASK_ON_BIT(SLIM_SYNC_FIFO_AEMPTY_TH_LEN, SLIM_SYNC_FIFO_AEMPTY_TH_OFFSET),
		0x1);
	hi64xx_update_bits(codec, SLIM_SYNC_CTRL1_REG,
		MASK_ON_BIT(SLIM_SYNC_FIFO_AF0_TH_LEN, SLIM_SYNC_FIFO_AF0_TH_OFFSET),
		0x18);
}

struct play_power_event_config{
	unsigned int rate;
	unsigned int slimbus_rate;
	unsigned int slimbus_track;
	int (*play_path_config)(struct snd_soc_codec *codec, int event);
	void (*set_slimbus_fifo)(struct snd_soc_codec *codec);
};

static const struct play_power_event_config play_power_event_config_list[SAMPLE_RATE_INDEX_MAX] = {
	{ SAMPLE_RATE_INDEX_48K, SLIMBUS_SAMPLE_RATE_48K, SLIMBUS_6405_TRACK_AUDIO_PLAY,
		play48k_path_config, set_slimbus_fifo_for_default },
	{ SAMPLE_RATE_INDEX_96K, SLIMBUS_SAMPLE_RATE_96K, SLIMBUS_6405_TRACK_DIRECT_PLAY,
		play96k_path_config, set_slimbus_fifo_for_default },
	{ SAMPLE_RATE_INDEX_192K, SLIMBUS_SAMPLE_RATE_192K, SLIMBUS_6405_TRACK_DIRECT_PLAY,
		play192k_path_config, set_slimbus_fifo_for_default },
	{ SAMPLE_RATE_INDEX_384K, SLIMBUS_SAMPLE_RATE_384K, SLIMBUS_6405_TRACK_DIRECT_PLAY,
		play192k_path_config, set_slimbus_fifo_for_default },
	{ SAMPLE_RATE_INDEX_44K1, SLIMBUS_SAMPLE_RATE_44K1, SLIMBUS_6405_TRACK_DIRECT_PLAY,
		play48k_path_config, set_slimbus_fifo_for_44k1 },
	{ SAMPLE_RATE_INDEX_88K2, SLIMBUS_SAMPLE_RATE_88K2, SLIMBUS_6405_TRACK_DIRECT_PLAY,
		play96k_path_config, set_slimbus_fifo_for_44k1 },
	{ SAMPLE_RATE_INDEX_176K4, SLIMBUS_SAMPLE_RATE_176K4, SLIMBUS_6405_TRACK_DIRECT_PLAY,
		play192k_path_config, set_slimbus_fifo_for_44k1 },
};


static int play_config_power_event(unsigned int rate, struct snd_soc_codec *codec, int event)
{
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);

	unsigned int i;
	int ret;

	IN_FUNCTION;

	for (i = 0; i < SAMPLE_RATE_INDEX_MAX; i++) {
		if (rate == play_power_event_config_list[i].rate)
			break;
	}

	if (i == SAMPLE_RATE_INDEX_MAX) {
		AUDIO_LOGW("error sample rate: %d", rate);
		return -EINVAL;
	}
	AUDIO_LOGI("sample rate idx: %d, rate: %u",
				i, play_power_event_config_list[i].rate);

	if (play_power_event_config_list[i].play_path_config) {
		ret = play_power_event_config_list[i].play_path_config(codec, event);
		if (ret != 0) {
			AUDIO_LOGW("path config err: %d", ret);
			return ret;
		}
	}

	if (play_power_event_config_list[i].set_slimbus_fifo)
		play_power_event_config_list[i].set_slimbus_fifo(codec);

	priv->play_params.rate = play_power_event_config_list[i].slimbus_rate;

	ret = slimbus_param_pass(codec,
		play_power_event_config_list[i].slimbus_track, &priv->play_params, event);

	OUT_FUNCTION;

	return ret;

}

static int play44k1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	return play_config_power_event(SAMPLE_RATE_INDEX_44K1, codec, event);
}

static int play88k2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	return play_config_power_event(SAMPLE_RATE_INDEX_88K2, codec, event);
}

static int play176k4_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	return play_config_power_event(SAMPLE_RATE_INDEX_176K4, codec, event);
}

static int play48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	return play_config_power_event(SAMPLE_RATE_INDEX_48K, codec, event);
}

static int play96k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	return play_config_power_event(SAMPLE_RATE_INDEX_96K, codec, event);
}

static int play192k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	return play_config_power_event(SAMPLE_RATE_INDEX_192K, codec, event);
}

static int play384k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	return play_config_power_event(SAMPLE_RATE_INDEX_384K, codec, event);

}

static int hp_high_level_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* hpdac high performance */
		priv->rcv_hp_classH_state = (unsigned int)priv->rcv_hp_classH_state & (~HP_CLASSH_STATE);
		hi6405_set_classH_config(codec, priv->rcv_hp_classH_state);
		break;

	case SND_SOC_DAPM_POST_PMD:
		/* hpdac lower power */
		priv->rcv_hp_classH_state = (unsigned int)priv->rcv_hp_classH_state | HP_CLASSH_STATE;
		hi6405_set_classH_config(codec, priv->rcv_hp_classH_state);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hp_concurrency_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);
	int ret = 0;

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			1 << S2_IL_SRC_CLK_EN_OFFSET | 1 << S2_IR_SRC_CLK_EN_OFFSET,
			1 << S2_IL_SRC_CLK_EN_OFFSET | 1 << S2_IR_SRC_CLK_EN_OFFSET);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4, &priv->play_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus track activate 4pa iv err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4, &priv->play_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus track activate 4pa iv err: %d", ret);
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			1 << S2_IL_SRC_CLK_EN_OFFSET | 1 << S2_IR_SRC_CLK_EN_OFFSET,
			0x0);
		break;
	default:
		AUDIO_LOGW("power event err:%d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s2up_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_OL_SRC_CLK_EN_OFFSET | 0x1 << S2_OR_SRC_CLK_EN_OFFSET,
			0x1 << S2_OL_SRC_CLK_EN_OFFSET | 0x1 << S2_OR_SRC_CLK_EN_OFFSET );
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_OL_SRC_CLK_EN_OFFSET | 0x1 << S2_OR_SRC_CLK_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int iv_dspif_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* on bit match */
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL1_REG,
			0x1 << S4_O_BITMATCH_BYP_EN_OFFSET, 0);

		/* src 96k-48k */
		hi64xx_update_bits(codec, S4_DP_CLK_EN_REG,
			0x1 << S4_OL_SRC_CLK_EN_OFFSET | 0x1 << S4_OR_SRC_CLK_EN_OFFSET,
			0x1 << S4_OL_SRC_CLK_EN_OFFSET | 0x1 << S4_OR_SRC_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, SRC_VLD_CTRL8_REG,
			0x1 << S4_OL_SRC_DIN_VLD_SEL_OFFSET | 0x1 << S4_OR_SRC_DIN_VLD_SEL_OFFSET,
			0x1 << S4_OL_SRC_DIN_VLD_SEL_OFFSET | 0x1 << S4_OR_SRC_DIN_VLD_SEL_OFFSET);

		/* 32bit */
		hi64xx_update_bits(codec, SC_S4_IF_H_REG,
			MASK_ON_BIT(S4_CODEC_IO_WORDLENGTH_LEN, S4_CODEC_IO_WORDLENGTH_OFFSET), 0xff);

		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
			MASK_ON_BIT(SPA_OL_SRC_DIN_SEL_LEN, SPA_OL_SRC_DIN_SEL_OFFSET) |
			MASK_ON_BIT(SPA_OR_SRC_DIN_SEL_LEN, SPA_OR_SRC_DIN_SEL_OFFSET),
			0x1 << SPA_OL_SRC_DIN_SEL_OFFSET | 0x1 << SPA_OR_SRC_DIN_SEL_OFFSET);

		hi64xx_update_bits(codec, SC_S4_IF_H_REG,
			MASK_ON_BIT(S4_CODEC_IO_WORDLENGTH_LEN, S4_CODEC_IO_WORDLENGTH_OFFSET), 0x0);

		hi64xx_update_bits(codec, S4_DP_CLK_EN_REG,
			0x1 << S4_OL_SRC_CLK_EN_OFFSET | 0x1 << S4_OR_SRC_CLK_EN_OFFSET, 0x0);
		hi64xx_update_bits(codec, SRC_VLD_CTRL8_REG,
			0x1 << S4_OL_SRC_DIN_VLD_SEL_OFFSET | 0x1 << S4_OR_SRC_DIN_VLD_SEL_OFFSET, 0x0);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL1_REG,
			0x1 << S4_O_BITMATCH_BYP_EN_OFFSET,
			0x1 << S4_O_BITMATCH_BYP_EN_OFFSET);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static const struct reg_seq_config iv_2pa_up_regs[] = {
	/* on bit match */
	{ { SC_CODEC_MUX_CTRL1_REG, 0x1 << S4_O_BITMATCH_BYP_EN_OFFSET, 0x0, true }, 0, 0 },
	/* src 96k-48k */
	{ { S4_DP_CLK_EN_REG, 0x1 << S4_OL_SRC_CLK_EN_OFFSET |
		0x1 << S4_OR_SRC_CLK_EN_OFFSET,
		0x1 << S4_OL_SRC_CLK_EN_OFFSET |
		0x1 << S4_OR_SRC_CLK_EN_OFFSET, true }, 0, 0 },
	{ { SRC_VLD_CTRL8_REG, 0x1 << S4_OL_SRC_DIN_VLD_SEL_OFFSET |
		0x1 << S4_OR_SRC_DIN_VLD_SEL_OFFSET,
		0x1 << S4_OL_SRC_DIN_VLD_SEL_OFFSET |
		0x1 << S4_OR_SRC_DIN_VLD_SEL_OFFSET, true }, 0, 0 },
	/* 32bit */
	{ { SC_S4_IF_H_REG, MASK_ON_BIT(S4_CODEC_IO_WORDLENGTH_LEN,
		S4_CODEC_IO_WORDLENGTH_OFFSET),
		0x3 << S4_CODEC_IO_WORDLENGTH_OFFSET, true }, 0, 0 },
	/* sel iv */
	{ { SC_CODEC_MUX_CTRL37_REG, 0, 0xA5, 0 }, 0, 0 },
};

static const struct reg_seq_config iv_2pa_dn_regs[] = {
	{ { SC_CODEC_MUX_CTRL37_REG, 0, 0x5, 0 }, 0, 0 },
	{ { SC_S4_IF_H_REG, MASK_ON_BIT(S4_CODEC_IO_WORDLENGTH_LEN,
		S4_CODEC_IO_WORDLENGTH_OFFSET), 0x0, true }, 0, 0 },
	{ { S4_DP_CLK_EN_REG, 0x1 << S4_OL_SRC_CLK_EN_OFFSET |
		0x1 << S4_OR_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL8_REG, 0x1 << S4_OL_SRC_DIN_VLD_SEL_OFFSET |
		0x1 << S4_OR_SRC_DIN_VLD_SEL_OFFSET, 0x0, true }, 0, 0 },
	{ { SC_CODEC_MUX_CTRL1_REG, 0x1 << S4_O_BITMATCH_BYP_EN_OFFSET,
		0x1 << S4_O_BITMATCH_BYP_EN_OFFSET, true }, 0, 0 },
};

static int iv_2pa_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);
	int ret = 0;

	IN_FUNCTION;

	priv->pa_iv_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->pa_iv_params.channels = 2;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		write_reg_seq_array(codec,
			iv_2pa_up_regs, ARRAY_SIZE(iv_2pa_up_regs));

		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_2PA_IV, &priv->pa_iv_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track 4pa iv err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		write_reg_seq_array(codec,
			iv_2pa_dn_regs, ARRAY_SIZE(iv_2pa_dn_regs));

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_2PA_IV, &priv->pa_iv_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track 4pa iv err: %d", ret);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int fm_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IL_SRC_CLK_EN_OFFSET | 0x1 << S2_IR_SRC_CLK_EN_OFFSET,
			0x1 << S2_IL_SRC_CLK_EN_OFFSET | 0x1 << S2_IR_SRC_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
			0x1 << DAC_MIXER4_VLD_SEL_OFFSET | 0x1 << DAC_PRE_MIXER2_VLD_SEL_OFFSET,
			0x1 << DAC_MIXER4_VLD_SEL_OFFSET | 0x1 << DAC_PRE_MIXER2_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
			0x1 << DAC_MIXER4_VLD_SEL_OFFSET  | 0x1 << DAC_PRE_MIXER2_VLD_SEL_OFFSET, 0);
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IL_SRC_CLK_EN_OFFSET | 0x1 << S2_IR_SRC_CLK_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static const struct reg_seq_config audioup_2mic_up_regs[] = {
	/* src in && out sample rate */
	{ { SRC_VLD_CTRL2_REG, 0, 0x1C, 0 }, 0, 0 },
	/* src disable clk */
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC2_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET,
		0x0, true }, 0, 0 },
	/* src down 2 mode */
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC1_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC2_SRC_MODE_OFFSET, true }, 0, 0 },
	/* src enable clk */
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC2_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET,
		0x1 << S1_MIC2_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET, true }, 0, 0 },
};

static const struct reg_seq_config audioup_2mic_dn_regs[] = {
	/* src power off */
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC2_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL2_REG, 0, 0x0, false }, 0, 0 },
};

static int audioup_2mic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		platform_data->capture_params.channels = 2;
		write_reg_seq_array(codec, audioup_2mic_up_regs,
			ARRAY_SIZE(audioup_2mic_up_regs));

		u12_select_pga(codec);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_AUDIO_CAPTURE,
			&platform_data->capture_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track capture err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (platform_data->audioup_4mic_state == TRACK_FREE ||
			platform_data->audioup_5mic_state == TRACK_FREE) {
			u12_select_dspif(codec);
			ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
				SLIMBUS_6405_TRACK_AUDIO_CAPTURE, NULL);
			if (ret != 0)
				AUDIO_LOGE("slimbus_deactivate_track capture err: %d", ret);
			write_reg_seq_array(codec, audioup_2mic_dn_regs,
				ARRAY_SIZE(audioup_2mic_dn_regs));
		}
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static const struct reg_seq_config audioup_4mic_up_regs[] = {
	{ { S1_MIXER_EQ_CLK_EN_REG, 0x1 << RST_5MIC_S1_ACCESS_IRQ_OFFSET,
		0x1 << RST_5MIC_S1_ACCESS_IRQ_OFFSET, true }, 0, 0 },
	{ { SC_FS_SLIM_CTRL_3_REG, 0, 0x44, false }, 0, 0 },
	/* src in && out sample rate */
	{ { SRC_VLD_CTRL2_REG, 0, 0x1C, false }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	/* src down 2 mode */
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC1_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC2_SRC_MODE_OFFSET, true }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC2_SRC_CLK_EN_OFFSET, true }, 0, 0 },
	{ { SC_FS_SLIM_CTRL_4_REG, 0, 0x44, false }, 0, 0 },
	{ { SRC_VLD_CTRL3_REG, 0, 0xC, false }, 0, 0 },
	{ { SRC_VLD_CTRL4_REG, 0, 0xC, false }, 0, 0 },
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC4_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC3_SRC_CTRL_REG, 0x7 << MIC3_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC3_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { MIC4_SRC_CTRL_REG, 0x7 << MIC4_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC4_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC4_SRC_CLK_EN_OFFSET, 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC4_SRC_CLK_EN_OFFSET, true }, 0, 0 },
};

static const struct reg_seq_config audioup_4_2mic_up_regs[] = {
	{ { SC_FS_SLIM_CTRL_3_REG, 0, 0x44, false }, 0, 0 },
	/* src in && out sample rate */
	{ { SRC_VLD_CTRL2_REG, 0, 0x1C, false }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	/* src down 2 mode */
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC1_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC2_SRC_MODE_OFFSET, true }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC2_SRC_CLK_EN_OFFSET, true }, 0, 0 },
};

static const struct reg_seq_config audioup_4mic_up_v2_regs[] = {
	{ { S1_MIXER_EQ_CLK_EN_REG, 0x1 << RST_5MIC_S1_ACCESS_IRQ_OFFSET,
		0x1 << RST_5MIC_S1_ACCESS_IRQ_OFFSET, true }, 0, 0 },
	{ { SC_FS_SLIM_CTRL_3_REG, 0, 0x44, false }, 0, 0 },
	/* src in && out sample rate */
	{ { SRC_VLD_CTRL2_REG, 0, 0x1C, false }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, (0x1 << S1_MIC1_SRC_CLK_EN_OFFSET) |
		(0x1 << S1_MIC2_SRC_CLK_EN_OFFSET), 0x0, true }, 0, 0 },
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
		0x4 << FS_SLIM_UP10_OFFSET, true }, 0, 0 },

	/* for Mic5 */
	{ { SRC_VLD_CTRL5_REG, 0, 0xC, false }, 0, 0 },
	{ { S4_DP_CLK_EN_REG, 0x1 << S1_MIC5_SRC_CLK_EN_OFFSET, 0x0,
		true }, 0, 0 },
	{ { MIC5_SRC_CTRL_REG, 0x7 << MIC5_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC5_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { S4_DP_CLK_EN_REG, 0x1 << S1_MIC5_SRC_CLK_EN_OFFSET,
		0x1 << S1_MIC5_SRC_CLK_EN_OFFSET, true }, 0, 0 },
	/* 0x20007033 0x0, Mic5-Port7, Port6 and Port7 not use the same ACK */
	{ { SLIM_CTRL3_REG, 1 << 3, 0 << 3, true }, 0, 0 },
};

static const struct reg_seq_config audioup_4mic_dn_regs[] = {
	/* src power off */
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL2_REG, 0, 0x0, false }, 0, 0 },
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC4_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC3_SRC_CTRL_REG, 0x7 << MIC3_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC4_SRC_CTRL_REG, 0x7 << MIC4_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL3_REG, 0, 0x0, false }, 0, 0 },
	{ { SRC_VLD_CTRL4_REG, 0, 0x0, false }, 0, 0 },
};

static const struct reg_seq_config audioup_4_2mic_dn_regs[] = {
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC4_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC3_SRC_CTRL_REG, 0x7 << MIC3_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC4_SRC_CTRL_REG, 0x7 << MIC4_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL3_REG, 0, 0x0, false }, 0, 0 },
	{ { SRC_VLD_CTRL4_REG, 0, 0x0, false }, 0, 0 },
};

static const struct reg_seq_config audioup_4mic_dn_v2_regs[] = {
	/* src power off */
	{ { S1_DP_CLK_EN_REG, 1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC1_SRC_CTRL_REG, 0x7 << MIC1_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { MIC2_SRC_CTRL_REG, 0x7 << MIC2_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL2_REG, 0, 0x0, false }, 0, 0 },

	{ { S4_DP_CLK_EN_REG, 0x1 << S1_MIC5_SRC_CLK_EN_OFFSET, 0x0,
		true }, 0, 0 },

	/* 0x20007033  Mic5-Port7, Port6 and Port7 revert to use the same ACK */
	{ { SLIM_CTRL3_REG, 1 << 3, 1 << 3, true }, 0, 0 },
	{ { MIC5_SRC_CTRL_REG, 0x7 << MIC5_SRC_MODE_OFFSET, 0x0, true }, 0, 0 },
	{ { SRC_VLD_CTRL5_REG, 0, 0x0, false }, 0, 0 },
};

static int audioup_4mic_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	int ret = 0;

	/* When calling and recording are concurrent, channels drops from 4 to 2. */
	if (platform_data->voiceup_state && (platform_data->voice_up_params.channels == 4))
		platform_data->capture_params.channels = 2;
	else
		platform_data->capture_params.channels = 4;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (platform_data->capture_params.channels == 4)
			write_reg_seq_array(codec, audioup_4mic_up_regs,
				ARRAY_SIZE(audioup_4mic_up_regs));
		else if (platform_data->capture_params.channels == 2)
			write_reg_seq_array(codec, audioup_4_2mic_up_regs,
				ARRAY_SIZE(audioup_4_2mic_up_regs));

		u12_select_pga(codec);

		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_AUDIO_CAPTURE,
			&platform_data->capture_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track capture err: %d", ret);
		if (platform_data->capture_params.channels == 4)
			hi64xx_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
				0x1 << RST_5MIC_S1_ACCESS_IRQ_OFFSET, 0);

		platform_data->audioup_4mic_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		u12_select_dspif(codec);

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_AUDIO_CAPTURE,
			&platform_data->capture_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track capture err: %d", ret);
		if (platform_data->capture_params.channels == 4)
			write_reg_seq_array(codec, audioup_4mic_dn_regs,
				ARRAY_SIZE(audioup_4mic_dn_regs));
		else if (platform_data->capture_params.channels == 2)
			write_reg_seq_array(codec, audioup_4_2mic_dn_regs,
				ARRAY_SIZE(audioup_4_2mic_dn_regs));

		platform_data->audioup_4mic_state = TRACK_FREE;
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	int ret = 0;

	platform_data->capture_params.channels = 4;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		write_reg_seq_array(codec, audioup_4mic_up_v2_regs,
			ARRAY_SIZE(audioup_4mic_up_v2_regs));

		u10_select_mic_src(codec);
		u12_select_pga(codec);

		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_AUDIO_CAPTURE,
			&platform_data->capture_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track capture v2 err: %d", ret);

		hi64xx_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			0x1 << RST_5MIC_S1_ACCESS_IRQ_OFFSET, 0);

		platform_data->audioup_4mic_v2_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		u12_select_dspif(codec);
		u10_select_dspif(codec);

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_AUDIO_CAPTURE,
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
	{ { S1_MIXER_EQ_CLK_EN_REG, 0x1 << RST_5MIC_S1_ACCESS_IRQ_OFFSET,
		0x1 << RST_5MIC_S1_ACCESS_IRQ_OFFSET, true }, 0, 0 },
	{ { SC_FS_SLIM_CTRL_3_REG, 0, 0x44, false }, 0, 0 },
	/* src in && out sample rate */
	{ { SRC_VLD_CTRL2_REG, 0, 0x1C, false }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, (0x1 << S1_MIC1_SRC_CLK_EN_OFFSET) |
		(0x1 << S1_MIC2_SRC_CLK_EN_OFFSET), 0x0, true }, 0, 0 },
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
	{ { S1_DP_CLK_EN_REG, (0x1 << S1_MIC3_SRC_CLK_EN_OFFSET) |
		(0x1 << S1_MIC4_SRC_CLK_EN_OFFSET), 0x0, true }, 0, 0 },
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
	{ { S4_DP_CLK_EN_REG, 0x1 << S1_MIC5_SRC_CLK_EN_OFFSET, 0x0,
		true }, 0, 0 },
	{ { MIC5_SRC_CTRL_REG, 0x7 << MIC5_SRC_MODE_OFFSET,
		SRC_MODE_2 << MIC5_SRC_MODE_OFFSET, true }, 0, 0 },
	{ { S4_DP_CLK_EN_REG, 0x1 << S1_MIC5_SRC_CLK_EN_OFFSET,
		0x1 << S1_MIC5_SRC_CLK_EN_OFFSET, true }, 0, 0 },
	/* 0x20007033 0x0, Mic5-Port7, Port6 and Port7 not use the same ACK */
	{ { SLIM_CTRL3_REG, 1 << 3, 0 << 3, true }, 0, 0 },
};

static const struct reg_seq_config audioup_5_2mic_up_regs[] = {
	{ { SC_FS_SLIM_CTRL_3_REG, 0, 0x44, false }, 0, 0 },
	/* src in && out sample rate */
	{ { SRC_VLD_CTRL2_REG, 0, 0x1C, false }, 0, 0 },
	/* src clk */
	{ { S1_DP_CLK_EN_REG, 0x1 << S1_MIC1_SRC_CLK_EN_OFFSET |
		0x1 << S1_MIC2_SRC_CLK_EN_OFFSET, 0x0, true }, 0, 0 },
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
	{ { S4_DP_CLK_EN_REG, 0x1 << S1_MIC5_SRC_CLK_EN_OFFSET, 0x0,
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = NULL;
	int ret = 0;

	IN_FUNCTION;

	if (codec == NULL) {
		AUDIO_LOGE("codec is nul, err");
		OUT_FUNCTION;
		return ret;
	}

	platform_data = snd_soc_codec_get_drvdata(codec);
	if (platform_data == NULL) {
		AUDIO_LOGE("platform_data is nul, err");
		OUT_FUNCTION;
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

		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
				SLIMBUS_6405_TRACK_5MIC_CAPTURE,
				&platform_data->capture_params);
		if (ret)
			AUDIO_LOGE("slimbus_activate_track capture err");

		if (platform_data->capture_params.channels == 5)
			hi64xx_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
				0x1<<RST_5MIC_S1_ACCESS_IRQ_OFFSET, 0);

		platform_data->audioup_5mic_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		u12_select_dspif(codec);
		u10_select_dspif(codec);

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
				SLIMBUS_6405_TRACK_5MIC_CAPTURE,
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

	OUT_FUNCTION;

	return ret;
}

static void up_src_pre_pmu(struct snd_soc_codec *codec, unsigned int channel,
	unsigned int sample_rate, unsigned int src_mode)
{
	/* src in && out sample rate */
	hi64xx_update_bits(codec, SRC_VLD_CTRL7_REG,
		0x1 << S3_OR_SRC_DIN_VLD_SEL_OFFSET | 0x1 << S3_OL_SRC_DIN_VLD_SEL_OFFSET,
		0x1 << S3_OR_SRC_DIN_VLD_SEL_OFFSET | 0x1 << S3_OL_SRC_DIN_VLD_SEL_OFFSET);
	hi64xx_update_bits(codec, SRC_VLD_CTRL7_REG,
		0x7 << S3_O_SRC_DOUT_VLD_SEL_OFFSET,
		sample_rate << S3_O_SRC_DOUT_VLD_SEL_OFFSET);
	/* src clk */
	hi64xx_update_bits(codec, S3_DP_CLK_EN_REG,
		0x1 << S3_OL_SRC_CLK_EN_OFFSET | 0x1 << S3_OR_SRC_CLK_EN_OFFSET, 0x0);
	/* src down xx mode */
	hi64xx_update_bits(codec, S3_OL_SRC_CTRL_REG,
		0x7 << S3_OL_SRC_MODE_OFFSET, src_mode << S3_OL_SRC_MODE_OFFSET);
	hi64xx_update_bits(codec, S3_OR_SRC_CTRL_REG,
		0x7 << S3_OR_SRC_MODE_OFFSET, src_mode << S3_OR_SRC_MODE_OFFSET);
	/* src clk */
	hi64xx_update_bits(codec, S3_DP_CLK_EN_REG,
		0x1 << S3_OL_SRC_CLK_EN_OFFSET | 0x1 << S3_OR_SRC_CLK_EN_OFFSET,
		0x1 << S3_OL_SRC_CLK_EN_OFFSET | 0x1 << S3_OR_SRC_CLK_EN_OFFSET);
	/* slimbus sample rate */
	snd_soc_write(codec, SC_FS_SLIM_CTRL_5_REG,
		sample_rate << FS_SLIM_UP6_OFFSET | sample_rate << FS_SLIM_UP5_OFFSET);
	if (channel == 4) {
		/* mic3/4 in&&out sample rate */
		hi64xx_update_bits(codec, SRC_VLD_CTRL3_REG,
			0x1 << MIC3_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC3_SRC_DOUT_VLD_SEL_OFFSET,
			0x1 << MIC3_SRC_DIN_VLD_SEL_OFFSET | sample_rate << MIC3_SRC_DOUT_VLD_SEL_OFFSET);
		hi64xx_update_bits(codec, SRC_VLD_CTRL4_REG,
			0x1 << MIC4_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC4_SRC_DOUT_VLD_SEL_OFFSET,
			0x1 << MIC4_SRC_DIN_VLD_SEL_OFFSET | sample_rate << MIC4_SRC_DOUT_VLD_SEL_OFFSET);
		/* u3/u4 clk clk */
		hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
			0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET, 0x0);
		/* u3/u4 src mode */
		hi64xx_update_bits(codec, MIC3_SRC_CTRL_REG,
			0x7 << MIC3_SRC_MODE_OFFSET,
			src_mode << MIC3_SRC_MODE_OFFSET);
		hi64xx_update_bits(codec, MIC4_SRC_CTRL_REG,
			0x7 << MIC4_SRC_MODE_OFFSET,
			src_mode << MIC4_SRC_MODE_OFFSET);
		/* u3/u4 clk clk */
		hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
			0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET,
			0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET);

		/* u3/u4 slimbus sample rate */
		snd_soc_write(codec, SC_FS_SLIM_CTRL_4_REG,
			sample_rate << FS_SLIM_UP4_OFFSET | sample_rate << FS_SLIM_UP3_OFFSET);
	}
}

static void up_src_post_pmu(struct snd_soc_codec *codec, unsigned int channel)
{
	/* slimbus sample rate */
	snd_soc_write(codec, SC_FS_SLIM_CTRL_5_REG,
		0x0 << FS_SLIM_UP6_OFFSET | 0x0 << FS_SLIM_UP5_OFFSET);
	/* src power off */
	hi64xx_update_bits(codec, S3_DP_CLK_EN_REG,
		0x1 << S3_OL_SRC_CLK_EN_OFFSET | 0x1 << S3_OR_SRC_CLK_EN_OFFSET, 0x0);
	hi64xx_update_bits(codec, S3_OL_SRC_CTRL_REG,
		0x7 << S3_OL_SRC_MODE_OFFSET, 0x0);
	hi64xx_update_bits(codec, S3_OR_SRC_CTRL_REG,
		0x7 << S3_OR_SRC_MODE_OFFSET, 0x0);
	hi64xx_update_bits(codec, SRC_VLD_CTRL7_REG,
		0x7 << S3_O_SRC_DOUT_VLD_SEL_OFFSET, 0);
	hi64xx_update_bits(codec, SRC_VLD_CTRL7_REG,
		0x1 << S3_OR_SRC_DIN_VLD_SEL_OFFSET | 0x1 << S3_OL_SRC_DIN_VLD_SEL_OFFSET, 0x0);
	if (channel == 4) {
		hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
			0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET, 0x0);
		snd_soc_write(codec, SC_FS_SLIM_CTRL_4_REG,
			0x4 << FS_SLIM_UP4_OFFSET | 0x4 << FS_SLIM_UP3_OFFSET);
		hi64xx_update_bits(codec, SRC_VLD_CTRL3_REG,
			0x1 << MIC3_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC3_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
		hi64xx_update_bits(codec, SRC_VLD_CTRL4_REG,
			0x1 << MIC4_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC4_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
		hi64xx_update_bits(codec, MIC3_SRC_CTRL_REG,
			0x7 << MIC3_SRC_MODE_OFFSET, 0x0);
		hi64xx_update_bits(codec, MIC4_SRC_CTRL_REG,
			0x7 << MIC4_SRC_MODE_OFFSET, 0x0);
	}
}

static void up_src_pre_pmu_v2(struct snd_soc_codec *codec, unsigned int channel,
	unsigned int sample_rate, unsigned int src_mode)
{
	/* mic3 in&&out sample rate */
	hi64xx_update_bits(codec, SRC_VLD_CTRL3_REG,
		0x1 << MIC3_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC3_SRC_DOUT_VLD_SEL_OFFSET,
		0x1 << MIC3_SRC_DIN_VLD_SEL_OFFSET | sample_rate << MIC3_SRC_DOUT_VLD_SEL_OFFSET);
	/* mic4 in&&out sample rate */
	hi64xx_update_bits(codec, SRC_VLD_CTRL4_REG,
		0x1 << MIC4_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC4_SRC_DOUT_VLD_SEL_OFFSET,
		0x1 << MIC4_SRC_DIN_VLD_SEL_OFFSET | sample_rate << MIC4_SRC_DOUT_VLD_SEL_OFFSET);
	/* s1_mic3/mic4 src clk close */
	hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
		0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET, 0x0);
	/* mic3/mic4 src mode */
	hi64xx_update_bits(codec, MIC3_SRC_CTRL_REG, 0x7 << MIC3_SRC_MODE_OFFSET,
		src_mode << MIC3_SRC_MODE_OFFSET);
	hi64xx_update_bits(codec, MIC4_SRC_CTRL_REG, 0x7 << MIC4_SRC_MODE_OFFSET,
		src_mode << MIC4_SRC_MODE_OFFSET);
	/* s1_mic3/mic4 src clk open */
	hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
		0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET,
		0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET);
	/* u3/u4 slimbus sample rate */
	hi64xx_update_bits(codec, SC_FS_SLIM_CTRL_4_REG,
		0x7 << FS_SLIM_UP4_OFFSET | 0x7 << FS_SLIM_UP3_OFFSET,
		sample_rate << FS_SLIM_UP4_OFFSET | sample_rate << FS_SLIM_UP3_OFFSET);

	if (channel == 4) {
		/* u10 data source: mic_src */
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL25_REG, 0x3 << SLIM_UP10_DATA_SEL_OFFSET,
			0x3 << SLIM_UP10_DATA_SEL_OFFSET);
		/* mic5 in&&out sample rate */
		hi64xx_update_bits(codec, SRC_VLD_CTRL5_REG,
			0x1 << MIC5_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC5_SRC_DOUT_VLD_SEL_OFFSET,
			0x1 << MIC5_SRC_DIN_VLD_SEL_OFFSET | sample_rate << MIC5_SRC_DOUT_VLD_SEL_OFFSET);
		/* mic5 src clk close */
		hi64xx_update_bits(codec, S4_DP_CLK_EN_REG, 0x1 << S1_MIC5_SRC_CLK_EN_OFFSET,
			0x0 << S1_MIC5_SRC_CLK_EN_OFFSET);
		/* mic5 src mode */
		hi64xx_update_bits(codec, MIC5_SRC_CTRL_REG, 0x7 << MIC5_SRC_MODE_OFFSET,
			src_mode << MIC5_SRC_MODE_OFFSET);
		/* mic5 src clk open */
		hi64xx_update_bits(codec, S4_DP_CLK_EN_REG, 0x1 << S1_MIC5_SRC_CLK_EN_OFFSET,
			0x1 << S1_MIC5_SRC_CLK_EN_OFFSET);
		/* 0x20007033 0x0, Mic5-Port7, Port6 and Port7 not use the same ACK */
		hi64xx_update_bits(codec, SLIM_CTRL3_REG, 0x1 << SLIM_DPORT6_DPORT7_ACK_OFFSET,
			0 << SLIM_DPORT6_DPORT7_ACK_OFFSET);
		/* u10 slimbus sample rate */
		hi64xx_update_bits(codec, SC_FS_SLIM_CTRL_7_REG, 0x7 << FS_SLIM_UP10_OFFSET,
			sample_rate << FS_SLIM_UP10_OFFSET);
	}
}

static void up_src_post_pmu_v2(struct snd_soc_codec *codec, unsigned int channel)
{
	/* u3/u4 slimbus sample rate revert to init value 48k */
	hi64xx_update_bits(codec, SC_FS_SLIM_CTRL_4_REG,
		0x7 << FS_SLIM_UP4_OFFSET | 0x7 << FS_SLIM_UP3_OFFSET,
		0x4 << FS_SLIM_UP4_OFFSET | 0x4 << FS_SLIM_UP3_OFFSET);
	/* s1_mic3/mic4 src clk close */
	hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
		0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET, 0x0);
	/* mic3 in&&out sample rate revert to default value in: 48k, out: 8k */
	hi64xx_update_bits(codec, SRC_VLD_CTRL3_REG,
		0x1 << MIC3_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC3_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
	/* mic4 in&&out sample rate revert to default value in: 48k, out: 8k */
	hi64xx_update_bits(codec, SRC_VLD_CTRL4_REG,
		0x1 << MIC4_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC4_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
	/* mic3 src mode: 3times */
	hi64xx_update_bits(codec, MIC3_SRC_CTRL_REG, 0x7 << MIC3_SRC_MODE_OFFSET, 0x0);
	hi64xx_update_bits(codec, MIC4_SRC_CTRL_REG, 0x7 << MIC4_SRC_MODE_OFFSET, 0x0);

	if (channel == 4) {
		/* u10 slimbus sample rate revert to init value 48k */
		hi64xx_update_bits(codec, SC_FS_SLIM_CTRL_7_REG, 0x7 << FS_SLIM_UP10_OFFSET,
			0x4 << FS_SLIM_UP10_OFFSET);
		/* mic5 src clk close */
		hi64xx_update_bits(codec, S4_DP_CLK_EN_REG, 0x1 << S1_MIC5_SRC_CLK_EN_OFFSET,
			0x0 << S1_MIC5_SRC_CLK_EN_OFFSET);
		/* 0x20007033  Mic5-Port7, Port6 and Port7 revert to use the same ACK */
		hi64xx_update_bits(codec, SLIM_CTRL3_REG, 0x1 << SLIM_DPORT6_DPORT7_ACK_OFFSET,
			0x1 << SLIM_DPORT6_DPORT7_ACK_OFFSET);
		/* mic5 in&&out sample rate revert to default value in: 48k, out: 8k */
		hi64xx_update_bits(codec, SRC_VLD_CTRL5_REG,
			0x1 << MIC5_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC5_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
		/* mic5 src mode: 3times */
		hi64xx_update_bits(codec, MIC5_SRC_CTRL_REG, 0x7 << MIC5_SRC_MODE_OFFSET, 0x0);
	}
}

static void update_callswitch_state(int event,
	struct hi6405_platform_data *platform_data)
{
	struct hi6405_board_cfg board_cfg = platform_data->board_config;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		platform_data->is_callswitch_on = true;
		if (board_cfg.mic_control_sc_freq_enable) {
			vsys_switch_set_sc_frequency_mode(1); /* auto mode */
			AUDIO_LOGI("callswitch event update, set sc frequency auto mode");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		platform_data->is_callswitch_on = false;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
}

static int voice8k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
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

static int voice_v2_power_event(struct snd_soc_dapm_widget *w,
	int event, uint32_t rate)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	uint32_t rate_cfg;
	uint32_t rate_mode;
	int ret = 0;

	/* update channels to 2 */
	platform_data->voice_up_params.channels = 2;

	platform_data->voice_up_params.rate = rate;
	if (rate == SLIMBUS_SAMPLE_RATE_8K) {
		rate_cfg = SAMPLE_RATE_REG_CFG_8K;
		rate_mode = SRC_MODE_12;
	} else if (rate == SLIMBUS_SAMPLE_RATE_16K) {
		rate_cfg = SAMPLE_RATE_REG_CFG_16K;
		rate_mode = SRC_MODE_6;
	} else {
		rate_cfg = SAMPLE_RATE_REG_CFG_32K;
		rate_mode = SRC_MODE_3;
	}

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu_v2(codec, 2, rate_cfg, rate_mode);

		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_VOICE_UP, &platform_data->voice_up_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track voice up v2 err: %d", ret);

		platform_data->voiceup_v2_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_VOICE_UP, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track voice up v2 err: %d", ret);

		platform_data->voiceup_v2_state = TRACK_FREE;

		up_src_post_pmu_v2(codec, 2);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	int ret = 0;

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_8K;
	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu_v2(codec, 4, SAMPLE_RATE_REG_CFG_8K, SRC_MODE_12);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_UP_4PA,
			&platform_data->voice_up_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track voice err: %d", ret);
		platform_data->voiceup_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_VOICE_UP_4PA, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track voice err: %d", ret);
		up_src_post_pmu_v2(codec, 4);
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	int ret = 0;

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_16K;
	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu_v2(codec, 4, SAMPLE_RATE_REG_CFG_16K, SRC_MODE_6);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_UP_4PA,
			&platform_data->voice_up_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track voice err: %d", ret);
		platform_data->voiceup_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_VOICE_UP_4PA, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track voice err: %d", ret);
		up_src_post_pmu_v2(codec, 4);
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	int ret = 0;

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_32K;
	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		up_src_pre_pmu_v2(codec, 4, SAMPLE_RATE_REG_CFG_32K, SRC_MODE_3);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_UP_4PA,
			&platform_data->voice_up_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track voice err: %d", ret);
		platform_data->voiceup_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_UP_4PA, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track voice err: %d", ret);
		up_src_post_pmu_v2(codec, 4);
		platform_data->voiceup_state = TRACK_FREE;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static void u9_select_dspif(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL25_REG,
		0x3 << SLIM_UP9_DATA_SEL_OFFSET,
		0x1 << SLIM_UP9_DATA_SEL_OFFSET);
}

static void u9_select_pga(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL25_REG,
		0x3 << SLIM_UP9_DATA_SEL_OFFSET,
		0x0 << SLIM_UP9_DATA_SEL_OFFSET);
}

static void update_madswitch_state(int event,
	struct hi6405_platform_data *platform_data)
{
	struct hi6405_board_cfg board_cfg = platform_data->board_config;

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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* mad src enable */
		hi64xx_update_bits(codec, S4_DP_CLK_EN_REG,
			0x1 << S1_MAD_SRC_CLK_EN_OFFSET,
			0x1 << S1_MAD_SRC_CLK_EN_OFFSET);
		/* s1_o_dsp_if_din_sel->mad_buffer out */
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL21_REG,
			0x3 << S1_O_DSP_IF_DIN_SEL_OFFSET,
			0x2 << S1_O_DSP_IF_DIN_SEL_OFFSET);
		u12_select_dspif(codec);
		u9_select_dspif(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* s1_o_dsp_if_din_sel->mad_buffer out */
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL21_REG,
			0x3 << S1_O_DSP_IF_DIN_SEL_OFFSET, 0x0);
		u12_select_pga(codec);
		u9_select_pga(codec);
		/* mad src disable */
		hi64xx_update_bits(codec, S4_DP_CLK_EN_REG,
			0x1 << S1_MAD_SRC_CLK_EN_OFFSET, 0);
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* set adc2l_pga bypass */
		hi64xx_update_bits(codec, ADC2L_PGA_CTRL0_REG,
			0x1 << ADC2L_PGA_BYPASS_OFFSET,
			0x1 << ADC2L_PGA_BYPASS_OFFSET);
		/* ultrasonic up ADC2L FIR2D bypass enable */
		hi64xx_update_bits(codec, ADC2L_CTRL0_REG,
			0x1 << ADC2L_IR2D_BYPASS_EN_OFFSET,
			0x1 << ADC2L_IR2D_BYPASS_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* ultrasonic up ADC2L FIR2D bypass disable */
		hi64xx_update_bits(codec, ADC2L_CTRL0_REG,
			0x1 << ADC2L_IR2D_BYPASS_EN_OFFSET, 0);
		/* set adc2l_pga default */
		hi64xx_update_bits(codec, ADC2L_PGA_CTRL0_REG,
			0x1 << ADC2L_PGA_BYPASS_OFFSET, 0);
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		AUDIO_LOGW("virtbtn_active_switch_power_event 1");
		hi64xx_update_bits(codec, DSPIF_VLD_CTRL4_REG,
			0x7 << DSPIF8_DIN_VLD_SEL_OFFSET,
			0x4 << DSPIF8_DIN_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		AUDIO_LOGW("virtbtn_active_switch_power_event 0");
		hi64xx_update_bits(codec, DSPIF_VLD_CTRL4_REG,
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
	struct snd_soc_codec *codec;

	codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		AUDIO_LOGW("virtbtn_passive_switch_power_event 1");
		hi64xx_update_bits(codec, APB_CLK_CFG_REG,
			0x1 << GPIO_PCLK_EN_OFFSET,
			0x1 << GPIO_PCLK_EN_OFFSET);
		hi64xx_update_bits(codec, DSPIF_VLD_CTRL5_REG,
			0x7 << DSPIF9_DIN_VLD_SEL_OFFSET,
			0x0 << DSPIF9_DIN_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		AUDIO_LOGW("virtbtn_passive_switch_power_event 0");
		hi64xx_update_bits(codec, DSPIF_VLD_CTRL5_REG,
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_052,
			0x1 << CODEC_PD_INF_EMS_OFFSET,
			0x1 << CODEC_PD_INF_EMS_OFFSET);
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_052,
			0x1 << CODEC_PD_INF_LRN_OFFSET,
			0x0 << CODEC_PD_INF_LRN_OFFSET);

		hi64xx_update_bits(codec, ANA_IR_EN,
			0x3 << ANA_IR_INF_SEL_OFFSET,
			0x1 << ANA_IR_INF_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, ANA_IR_EN,
			0x3 << ANA_IR_INF_SEL_OFFSET, 0);
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_052,
			0x1 << CODEC_PD_INF_LRN_OFFSET,
			0x1 << CODEC_PD_INF_LRN_OFFSET);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int soundtrigger_u3u4_power_event(struct snd_soc_codec *codec,
	struct slimbus_track_param *params, int event)
{
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* mic3/4 in&&out sample rate */
		hi64xx_update_bits(codec, SRC_VLD_CTRL3_REG,
			0x1 << MIC3_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC3_SRC_DOUT_VLD_SEL_OFFSET,
			0x1 << MIC3_SRC_DIN_VLD_SEL_OFFSET | SAMPLE_RATE_REG_CFG_16K << MIC3_SRC_DOUT_VLD_SEL_OFFSET);
		hi64xx_update_bits(codec, SRC_VLD_CTRL4_REG,
			0x1 << MIC4_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC4_SRC_DOUT_VLD_SEL_OFFSET,
			0x1 << MIC4_SRC_DIN_VLD_SEL_OFFSET | SAMPLE_RATE_REG_CFG_16K << MIC4_SRC_DOUT_VLD_SEL_OFFSET);
		/* u3/u4 clk clk */
		hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
			0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET, 0x0);
		/* u3/u4 src mode */
		hi64xx_update_bits(codec, MIC3_SRC_CTRL_REG,
			0x7 << MIC3_SRC_MODE_OFFSET,
			SRC_MODE_6 << MIC3_SRC_MODE_OFFSET);
		hi64xx_update_bits(codec, MIC4_SRC_CTRL_REG,
			0x7 << MIC4_SRC_MODE_OFFSET,
			SRC_MODE_6 << MIC4_SRC_MODE_OFFSET);
		/* u3/u4 clk clk */
		hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
			0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET,
			0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET);

		/* u3/u4 slimbus sample rate */
		snd_soc_write(codec, SC_FS_SLIM_CTRL_4_REG,
			SAMPLE_RATE_REG_CFG_16K << FS_SLIM_UP4_OFFSET | SAMPLE_RATE_REG_CFG_16K << FS_SLIM_UP3_OFFSET);

		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_KWS, params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track kws err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* slimbus soundtrigger deactive */
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_KWS, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track kws err: %d", ret);

		hi64xx_update_bits(codec, S1_DP_CLK_EN_REG,
			0x1 << S1_MIC4_SRC_CLK_EN_OFFSET | 0x1 << S1_MIC3_SRC_CLK_EN_OFFSET, 0x0);
		snd_soc_write(codec, SC_FS_SLIM_CTRL_4_REG,
			0x4 << FS_SLIM_UP4_OFFSET | 0x4 << FS_SLIM_UP3_OFFSET);
		hi64xx_update_bits(codec, SRC_VLD_CTRL3_REG,
			0x1 << MIC3_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC3_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
		hi64xx_update_bits(codec, SRC_VLD_CTRL4_REG,
			0x1 << MIC4_SRC_DIN_VLD_SEL_OFFSET | 0x7 << MIC4_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
		hi64xx_update_bits(codec, MIC3_SRC_CTRL_REG,
			0x7 << MIC3_SRC_MODE_OFFSET, 0x0);
		hi64xx_update_bits(codec, MIC4_SRC_CTRL_REG,
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	platform_data->soundtrigger_params.channels = 1;

	return soundtrigger_u3u4_power_event(codec,
		&platform_data->soundtrigger_params, event);
}

static int soundtrigger_dualmic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	platform_data->soundtrigger_params.channels = 2;

	return soundtrigger_u3u4_power_event(codec,
		&platform_data->soundtrigger_params, event);
}


static int ec_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* close u7 src clk */
		hi64xx_update_bits(codec, S4_DP_CLK_EN_REG,
			0x1 << S4_SPA_R_SRC_CLK_EN_OFFSET, 0x0);

		/* sel u7 data source */
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
			0x3 << SLIM_SW_UP7_DATA_SEL_OFFSET, 0x0);

		/* config u7 48k */
		hi64xx_update_bits(codec, SC_FS_SLIM_CTRL_6_REG,
			0x7 << FS_SLIM_UP7_OFFSET, 0x4),

		/* active ec */
		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_ECREF, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track ec err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_ECREF, NULL);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track ec err: %d", ret);

		/* sel reg default config */
		hi64xx_update_bits(codec, SC_FS_SLIM_CTRL_6_REG,
			0x7 << FS_SLIM_UP7_OFFSET, 0x4),

		/* sel reg default config */
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
			0x3 << SLIM_SW_UP7_DATA_SEL_OFFSET, 0x0);

		/* sel reg default config */
		hi64xx_update_bits(codec, S4_DP_CLK_EN_REG,
			0x1 <<  S4_SPA_R_SRC_CLK_EN_OFFSET, 0x0);
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
		0x1 << S2_DIRECT_LOOP_OFFSET, true }, 0, 0 },
	/* s2 mater mode */
	{ { SC_S2_IF_L_REG, 0x1 << S2_MST_SLV_OFFSET,
		0x0 << S2_MST_SLV_OFFSET, true }, 0, 0 },
	/* s2 if rx en */
	{ { SC_S2_IF_L_REG, 0x1 << S2_IF_RX_ENA_OFFSET,
		0x1 << S2_IF_RX_ENA_OFFSET, true }, 0, 0 },
	/* s2 if tx en */
	{ { SC_S2_IF_L_REG, 0x1 << S2_IF_TX_ENA_OFFSET,
		0x1 << S2_IF_TX_ENA_OFFSET, true }, 0, 0 },
	/* s2 clk if en */
	{ { SC_CODEC_MUX_CTRL26_REG, 0x1 << S2_CLK_IF_EN_OFFSET,
		0x1 << S2_CLK_IF_EN_OFFSET, true }, 0, 0 },
	/* s2 freq */
	{ { SC_CODEC_MUX_CTRL23_REG, 0x7 << FS_S2_OFFSET,
		0x0 << FS_S2_OFFSET, true }, 0, 0 },
	/* s2 frame mode */
	{ { SC_S2_IF_H_REG, 0x1 << S2_FRAME_MODE_OFFSET,
		0x0 << S2_FRAME_MODE_OFFSET, true }, 0, 0 },
	/* s2_clk_if_txrx_en */
	{ { SC_CODEC_MUX_CTRL26_REG, 0x1 << S2_CLK_IF_TXRX_EN_OFFSET,
		0x1 << S2_CLK_IF_TXRX_EN_OFFSET, true }, 0, 0 },
};

static const struct reg_seq_config bluetooth_loop_dn_regs[] = {
	/* s2_clk_if_txrx_en */
	{ { SC_CODEC_MUX_CTRL26_REG, 0x1 << S2_CLK_IF_TXRX_EN_OFFSET, 0, true }, 0, 0 },
	/* s2 frame mode */
	{ { SC_S2_IF_H_REG, 0x1 << S2_FRAME_MODE_OFFSET, 0, true }, 0, 0 },
	/* s2 freq */
	{ { SC_CODEC_MUX_CTRL23_REG, 0x7 << FS_S2_OFFSET, 0, true }, 0, 0 },
	/* s2 clk if en */
	{ { SC_CODEC_MUX_CTRL26_REG, 0x1 << S2_CLK_IF_EN_OFFSET, 0, true }, 0, 0 },
	/* s2 if tx en */
	{ { SC_S2_IF_L_REG, 0x1 << S2_IF_TX_ENA_OFFSET, 0, true }, 0, 0 },
	/* s2 if rx en */
	{ { SC_S2_IF_L_REG, 0x1 << S2_IF_RX_ENA_OFFSET, 0, true }, 0, 0 },
	/* s2 mater mode */
	{ { SC_S2_IF_L_REG, 0x1 << S2_MST_SLV_OFFSET, 0, true }, 0, 0 },
	/* s2 direct loop Sdin->Sdout */
	{ { SC_S2_IF_L_REG, 0x3 << S2_DIRECT_LOOP_OFFSET, 0, true }, 0, 0 },
	/* s2 func mode  PCM STD */
	{ { SC_S2_IF_L_REG, 0x7 << S2_FUNC_MODE_OFFSET, 0, true }, 0, 0 },
};

static int i2s2_bluetooth_loop_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

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
		S4_O_BITMATCH_BYP_EN_OFFSET), 0x1 << S4_O_BITMATCH_BYP_EN_OFFSET, true }, 0, 0 },
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);
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

		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_2PA_IV, &priv->pa_iv_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track 4pa iv err: %d", ret);
		/* 2PA need active D1\D2 ,it contol by PLAY48K_SWITCH */
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* sel reg default config */
		write_reg_seq_array(codec, moveup_2pa_dn_regs,
			ARRAY_SIZE(moveup_2pa_dn_regs));

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_2PA_IV, &priv->pa_iv_params);
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
		S4_O_BITMATCH_BYP_EN_OFFSET), 0x1 << S4_O_BITMATCH_BYP_EN_OFFSET, true }, 0, 0 },
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);
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

		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_4PA_IV, &priv->pa_iv_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track 4pa iv err: %d", ret);
		/* D1/D2 active by PLAY48K_SWITCH, D5/D6 is active here */
		ret = slimbus_activate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_VOICE_DOWN, &priv->play_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_activate_track D5 D6 err: %d", ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		write_reg_seq_array(codec, moveup_4pa_dn_regs,
			ARRAY_SIZE(moveup_4pa_dn_regs));

		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_4PA_IV, &priv->pa_iv_params);
		if (ret != 0)
			AUDIO_LOGE("slimbus_deactivate_track 4pa iv err: %d", ret);
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_VOICE_DOWN, &priv->play_params);
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	int ret = 0;

	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* config SPA_OL/OR(if7) select dn1 dn2,it is defalut value */
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	int ret = 0;

	AUDIO_LOGI("power event: %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* config SPA_OL/OR(if7) select dspif out put */
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
			MASK_ON_BIT(SPA_OL_SRC_DIN_SEL_LEN, SPA_OL_SRC_DIN_SEL_OFFSET) |
			MASK_ON_BIT(SPA_OR_SRC_DIN_SEL_LEN, SPA_OR_SRC_DIN_SEL_OFFSET),
			0x2 << SPA_OR_SRC_DIN_SEL_OFFSET |
			0x2 << SPA_OL_SRC_DIN_SEL_OFFSET);
		/* dspif7_tdm_trans clk on */
		hi64xx_update_bits(codec, DMIC_CLK_EN_REG,
			MASK_ON_BIT(DSPIF7_TDM_TRANS_CLK_EN_LEN,
			DSPIF7_TDM_TRANS_CLK_EN_OFFSET),
			0x1 << DSPIF7_TDM_TRANS_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* config SPA_OL/OR(if7) ,select dn1 dn2,it is defalut value */
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL37_REG,
			MASK_ON_BIT(SPA_OL_SRC_DIN_SEL_LEN, SPA_OL_SRC_DIN_SEL_OFFSET) |
			MASK_ON_BIT(SPA_OR_SRC_DIN_SEL_LEN, SPA_OR_SRC_DIN_SEL_OFFSET), 0x0);
		/* dspif7_tdm_trans clk close */
		hi64xx_update_bits(codec, DMIC_CLK_EN_REG,
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
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	int ret = 0;
	unsigned int val;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		val = snd_soc_read(codec, DACL_MIXER4_CTRL2_REG);
		snd_soc_write(codec, DACL_MIXER4_CTRL2_REG, val);
		val = snd_soc_read(codec, DACR_MIXER4_CTRL2_REG);
		snd_soc_write(codec, DACR_MIXER4_CTRL2_REG, val);
		val = snd_soc_read(codec, DACL_PRE_MIXER2_CTRL1_REG);
		snd_soc_write(codec, DACL_PRE_MIXER2_CTRL1_REG, val);
		val = snd_soc_read(codec, DACR_PRE_MIXER2_CTRL1_REG);
		snd_soc_write(codec, DACR_PRE_MIXER2_CTRL1_REG, val);
		val = snd_soc_read(codec, DACL_POST_MIXER2_CTRL1_REG);
		snd_soc_write(codec, DACL_POST_MIXER2_CTRL1_REG, val);
		val = snd_soc_read(codec, DACR_POST_MIXER2_CTRL1_REG);
		snd_soc_write(codec, DACR_POST_MIXER2_CTRL1_REG, val);
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
	SND_SOC_DAPM_SWITCH_E("I2S2_BLUETOOTH_LOOP_SWITCH",\
		SND_SOC_NOPM, 0, 0, &dapm_i2s2_bluetooth_loop_switch_controls,\
		i2s2_bluetooth_loop_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),\
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


static const struct snd_soc_dapm_widget switch_widgets[] = {
	SWITCH_WIDGET
};

int hi6405_add_switch_widgets(struct snd_soc_codec *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_codec_get_dapm(codec);
	return snd_soc_dapm_new_controls(dapm, switch_widgets,
		ARRAY_SIZE(switch_widgets));
}

