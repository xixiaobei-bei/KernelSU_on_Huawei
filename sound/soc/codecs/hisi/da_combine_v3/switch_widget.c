/*
 * switch_widget.c
 *
 * switch widget for codec driver
 *
 * Copyright (c) 2015 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "switch_widget.h"

#include <sound/core.h>
#include <sound/tlv.h>

#include <linux/hisi/audio_log.h>
#include <linux/hisi/da_combine/da_combine_resmgr.h>

#include "slimbus.h"
#include "slimbus_da_combine_v3.h"

#include "linux/hisi/da_combine/da_combine_v3_regs.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "path_widget.h"
#include "type.h"

struct voice_fs_config {
	unsigned int s3_clk_cfg;
	unsigned int s3_ctrl_cfg;
	unsigned int s3_dspif_cfg;
	unsigned int slim2_clk_cfg;
	unsigned int slim5_clk_cfg;
	unsigned int u3_src_mode;
	unsigned int slim4_clk_cfg;
	uint32_t slimbus_rate;
};

struct voice_fs_config voice_fs_val[] = {
	{0x0, 0xa, 0x12, 0x0, 0x0, 0x2, 0x0, SLIMBUS_SAMPLE_RATE_8K},
	{0x11, 0x2, 0x0, 0x11, 0x11, 0x0, 0x11, SLIMBUS_SAMPLE_RATE_16K},
	{0x22, 0x4, 0x24, 0x22, 0x22, 0x4, 0x22, SLIMBUS_SAMPLE_RATE_32K},
};

enum voice_fs {
	VOICE_8K_FREQ,
	VOICE_16K_FREQ,
	VOICE_32K_FREQ,
};

/* SWITCH CONTROL */
/* MAD SWITCH */
static const struct snd_kcontrol_new dapm_mad_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_S1_PORT_CLKEN_REG, DA_COMBINE_V3_S1_OL_SRC_CLKEN_BIT, 1, 0);
/* S1 OL SWITCH */
static const struct snd_kcontrol_new dapm_s1_ol_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_S1_OL_SRC_EN_MM_BIT, 1, 0);
/* S1 OR SWITCH */
static const struct snd_kcontrol_new dapm_s1_or_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_S1_OR_SRC_EN_MM_BIT, 1, 0);
/* S2 OL SWITCH */
static const struct snd_kcontrol_new dapm_s2_ol_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_S2_PORT_CLKEN_REG, DA_COMBINE_V3_S2_OL_PGA_CLKEN_BIT, 1, 0);
/* S2 OR SWITCH */
static const struct snd_kcontrol_new dapm_s2_or_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_S2_PORT_CLKEN_REG, DA_COMBINE_V3_S2_OR_PGA_CLKEN_BIT, 1, 0);
/* S3 OL SWITCH */
static const struct snd_kcontrol_new dapm_s3_ol_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_S3_OL_SRC_EN_MM_BIT, 1, 0);
/* S3 OR SWITCH */
static const struct snd_kcontrol_new dapm_s3_or_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_S3_OR_SRC_EN_MM_BIT, 1, 0);
/* S4 OL SWITCH */
static const struct snd_kcontrol_new dapm_s4_ol_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_S4_OL_SRC_EN_MM_BIT, 1, 0);
/* S4 OR SWITCH */
static const struct snd_kcontrol_new dapm_s4_or_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_S4_OR_SRC_EN_MM_BIT, 1, 0);
/* U3 SRC SWITCH */
static const struct snd_kcontrol_new dapm_u3_ol_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_U3_OL_SRC_EN_MM_BIT, 1, 0);
/* U4 SRC SWITCH */
static const struct snd_kcontrol_new dapm_u4_or_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_U4_OR_SRC_EN_MM_BIT, 1, 0);
/* AUDIOUP SWITCH */
static const struct snd_kcontrol_new dapm_audioup_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_AUDIOUP_BIT, 1, 0);
/* AUDIODOWN SWITCH */
static const struct snd_kcontrol_new dapm_play48k_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_PLAY48K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_play96k_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_PLAY96K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_play192k_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_PLAY192K_BIT, 1, 0);
/* Lowlatency SWITCH */
static const struct snd_kcontrol_new dapm_lowlatency_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_LOWLANTENCY_BIT, 1, 0);
/* AUDIOUP 4MIC SWITCH */
static const struct snd_kcontrol_new dapm_audioup_4mic_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_AUDIOUP_4MIC_BIT, 1, 0);
/* VOICEUP SWITCH */
static const struct snd_kcontrol_new dapm_voice8k_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_VOICE8K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice16k_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_VOICE16K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice32k_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_VOICE32K_BIT, 1, 0);
/* HPCLASSH SWITCH */
static const struct snd_kcontrol_new dapm_hp_high_level_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_HPCLASSH_BIT, 1, 0);
/* MAX77813 PWM SWITCH */
#ifdef CONFIG_BUCKBOOST
static const struct snd_kcontrol_new dapm_auxmic_pwm_power_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_AUXMICPWMPOWER_BIT, 1, 0);
#endif
/* PLL48K SWITCH */
static const struct snd_kcontrol_new dapm_pll48k_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_PLL48K_BIT, 1, 0);
/* PLL44K1 SWITCH */
static const struct snd_kcontrol_new dapm_pll44k1_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_PLL44K1_BIT, 1, 0);
/* PLLMAD SWITCH */
static const struct snd_kcontrol_new dapm_pllmad_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_PLLMAD_BIT, 1, 0);
/* ir_env_study SWITCH*/
static const struct snd_kcontrol_new dapm_ir_env_study_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_IR_ENV_STUDY_BIT, 1, 0);

/* I2S2 bluetooth LOOP SWITCH*/
static const struct snd_kcontrol_new dapm_i2s2_bluetooth_loop_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_I2S2_BLUETOOTH_LOOP_BIT, 1, 0);

/* soundtrigger mic switch*/
static const struct snd_kcontrol_new dapm_soundtrigger_onemic_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_SOUNDTRIGGER_ONE_MIC_EN_BIT, 1, 0);

static const struct snd_kcontrol_new dapm_soundtrigger_dualmic_switch_controls =
	SOC_DAPM_SINGLE("SWITCH",
		DA_COMBINE_V3_VIRTUAL_REG, DA_COMBINE_V3_SOUNDTRIGGER_DUAL_MIC_EN_BIT, 1, 0);

static void u12_select_dspif(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, DA_COMBINE_V3_SLIM_UP_EN2,
		0x1 << DA_COMBINE_V3_SLIMBUS_UP12_DATA_SEL,
		0x1 << DA_COMBINE_V3_SLIMBUS_UP12_DATA_SEL);
}

static void u12_select_pga(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, DA_COMBINE_V3_SLIM_UP_EN2,
		0x1 << DA_COMBINE_V3_SLIMBUS_UP12_DATA_SEL,
		0x0);
}

static void u56_select_dspif(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, DA_COMBINE_V3_SLIM_UP_EN2,
		0x1 << DA_COMBINE_V3_SLIMBUS_UP56_DATA_SEL,
		0x1 << DA_COMBINE_V3_SLIMBUS_UP56_DATA_SEL);
}

static void u56_select_pga(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, DA_COMBINE_V3_SLIM_UP_EN2,
		0x1 << DA_COMBINE_V3_SLIMBUS_UP56_DATA_SEL,
		0x0);
}

static int madswitch_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		u12_select_dspif(codec);
		u56_select_dspif(codec);
		/* mad src enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_S4_PORT_CLKEN_REG,
			1 << DA_COMBINE_V3_S1_MAD_SRC_CLKEN_BIT,
			1 << DA_COMBINE_V3_S1_MAD_SRC_CLKEN_BIT);
		/* s1_o_dsp_if_din_sel->mad_buffer out */
		da_combine_update_bits(codec, DA_COMBINE_V3_SC_CODEC_MUX_SEL1_3, 0x3, 0x2);
		break;
	case SND_SOC_DAPM_POST_PMD:
		u12_select_pga(codec);
		u56_select_pga(codec);
		/* s1_o_dsp_if_din_sel->mad_buffer out */
		da_combine_update_bits(codec, DA_COMBINE_V3_SC_CODEC_MUX_SEL1_3, 0x3, 0x0);
		/* mad src disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_S4_PORT_CLKEN_REG,
			1 << DA_COMBINE_V3_S1_MAD_SRC_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int s2up_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			1 << DA_COMBINE_V3_S2_OR_SRC_CLKEN_BIT,
			1 << DA_COMBINE_V3_S2_OR_SRC_CLKEN_BIT);
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			1 << DA_COMBINE_V3_S2_OL_SRC_CLKEN_BIT,
			1 << DA_COMBINE_V3_S2_OL_SRC_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			1 << DA_COMBINE_V3_S2_OR_SRC_CLKEN_BIT, 0);
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			1 << DA_COMBINE_V3_S2_OL_SRC_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int audioup_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		u12_select_pga(codec);
		priv->capture_params.channels = 2;
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			SLIMBUS_TRACK_AUDIO_CAPTURE, &priv->capture_params);
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (priv->audioup_4mic_state == TRACK_FREE) {
			u12_select_dspif(codec);
			ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
				SLIMBUS_TRACK_AUDIO_CAPTURE, NULL);
		}
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int audioup_4mic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	if ((priv->voiceup_state == 1) && (priv->voice_params.channels == 4))
		priv->capture_params.channels = 2;
	else
		priv->capture_params.channels = 4;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (priv->capture_params.channels == 4)
			da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
				0x1 << DA_COMBINE_V3_AUDIO_UP_RST_BIT,
				0x1 << DA_COMBINE_V3_AUDIO_UP_RST_BIT);

		snd_soc_component_write(codec, DA_COMBINE_V3_SLIM3_CLK_CFG_REG, 0x44);
		if (priv->capture_params.channels == 4)
			snd_soc_component_write(codec, DA_COMBINE_V3_SLIM4_CLK_CFG_REG, 0x44);

		u12_select_pga(codec);
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			SLIMBUS_TRACK_AUDIO_CAPTURE, &priv->capture_params);
		priv->audioup_4mic_state = TRACK_STARTUP;

		if (priv->capture_params.channels == 4)
			da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
				0x1 << DA_COMBINE_V3_AUDIO_UP_RST_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		u12_select_dspif(codec);
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			SLIMBUS_TRACK_AUDIO_CAPTURE, &priv->capture_params);
		priv->audioup_4mic_state = TRACK_FREE;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int voice_deactive(struct snd_soc_component *codec,
		struct da_combine_v3_platform_data *platform_data)
{
	int voice_up_ret;
	int voice_down_ret;

	voice_up_ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
		SLIMBUS_TRACK_VOICE_UP, NULL);
	if (voice_up_ret)
		AUDIO_LOGW("deactive voice up error : %d", voice_up_ret);

	voice_down_ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
		SLIMBUS_TRACK_VOICE_DOWN, NULL);
	if (voice_down_ret)
		AUDIO_LOGW("deactive voice down error : %d", voice_down_ret);

	platform_data->voiceup_state = TRACK_FREE;
	u56_select_dspif(codec);

	return (voice_up_ret + voice_down_ret);
}

static void set_voice_fs(struct snd_soc_dapm_widget *w,
	struct voice_fs_config *fs_val)
{
	struct snd_soc_component *codec =
		snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	/* fs config & src power on */
	snd_soc_component_write(codec, DA_COMBINE_V3_S3_CLK_CFG_REG1,
		fs_val->s3_clk_cfg);
	da_combine_update_bits(codec, DA_COMBINE_V3_S3_PORT_CLKEN_REG, 0xC4, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_S3_CONTROL_REG, 0xE,
		fs_val->s3_ctrl_cfg);
	da_combine_update_bits(codec, DA_COMBINE_V3_S3_DSPIF_CONTROL_REG, 0x3F,
		fs_val->s3_dspif_cfg);
	da_combine_update_bits(codec, DA_COMBINE_V3_S3_PORT_CLKEN_REG, 0xC4, 0xC4);
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM2_CLK_CFG_REG,
		fs_val->slim2_clk_cfg);
	snd_soc_component_write(codec, DA_COMBINE_V3_SLIM5_CLK_CFG_REG,
		fs_val->slim5_clk_cfg);

	if (priv->voice_params.channels == 4) {
		da_combine_update_bits(codec, DA_COMBINE_V3_S4_PORT_CLKEN_REG,
			0x3, 0x0);
		da_combine_update_bits(codec, DA_COMBINE_V3_U3_SRC_MODE_CFG_REG, 0x7,
			fs_val->u3_src_mode);
		if (fs_val->slimbus_rate == SLIMBUS_SAMPLE_RATE_32K)
			da_combine_update_bits(codec, DA_COMBINE_V3_U4_SRC_MODE_CFG_REG,
			0x7, 0x4);
		da_combine_update_bits(codec, DA_COMBINE_V3_S4_PORT_CLKEN_REG,
			0x3, 0x3);
		snd_soc_component_write(codec, DA_COMBINE_V3_SLIM4_CLK_CFG_REG,
			fs_val->slim4_clk_cfg);
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			0x1 << DA_COMBINE_V3_VOICE_UP_RST_BIT,
			0x1 << DA_COMBINE_V3_VOICE_UP_RST_BIT);
	}
}

static int voice_fs_para_pass(struct snd_soc_dapm_widget *w,
	int event, struct voice_fs_config *fs_val)
{
	int ret = 0;
	struct slimbus_track_param voice_down_param;
	struct snd_soc_component *codec =
		snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	memset(&voice_down_param, 0, sizeof(voice_down_param)); /* unsafe_function_ignore: memset */
	priv->voice_params.rate = fs_val->slimbus_rate;
	voice_down_param.rate = priv->voice_params.rate;
	voice_down_param.channels = 2;
	voice_down_param.callback = priv->voice_params.callback;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		set_voice_fs(w, fs_val);
		u56_select_pga(codec);
		/* slimbus voice active */
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			SLIMBUS_TRACK_VOICE_UP, &priv->voice_params);
		ret += slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			SLIMBUS_TRACK_VOICE_DOWN, &voice_down_param);
		priv->voiceup_state = TRACK_STARTUP;

		if (priv->voice_params.channels == 4)
			da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
				0x1 << DA_COMBINE_V3_VOICE_UP_RST_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		priv->cdc_ctrl->ssi_check_enable = SSI_STATE_CHECK_ENABLE;
		ret = voice_deactive(codec, priv);
		/* src power off */
		da_combine_update_bits(codec, DA_COMBINE_V3_S3_PORT_CLKEN_REG,
			0xC4, 0x0);
		if (priv->voice_params.channels == 4)
			da_combine_update_bits(codec, DA_COMBINE_V3_S4_PORT_CLKEN_REG,
				0x3, 0x0);
		priv->cdc_ctrl->ssi_check_enable = SSI_STATE_CHECK_DISABLE;
		break;
	default:
		ret = -1;
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int voice8k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return voice_fs_para_pass(w, event, &voice_fs_val[VOICE_8K_FREQ]);
}

static int voice16k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return voice_fs_para_pass(w, event, &voice_fs_val[VOICE_16K_FREQ]);
}

static int voice32k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return voice_fs_para_pass(w, event, &voice_fs_val[VOICE_32K_FREQ]);
}

static int slimbus_param_pass(struct snd_soc_component *codec,
	enum slimbus_track_type track, struct slimbus_track_param *params, int event)
{
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (params == NULL) {
			snd_soc_component_write(codec, DA_COMBINE_V3_SLIM0_CLK_CFG_REG, 0x44);
			snd_soc_component_write(codec, DA_COMBINE_V3_DACL_PGA_MUX_SEL, 0x0);
			snd_soc_component_write(codec, DA_COMBINE_V3_DACR_PGA_MUX_SEL, 0x0);
		} else {
			if (params->rate == SLIMBUS_SAMPLE_RATE_96K) {
				snd_soc_component_write(codec,
					DA_COMBINE_V3_SLIM0_CLK_CFG_REG, 0x55);
				snd_soc_component_write(codec,
				DA_COMBINE_V3_DACL_PGA_MUX_SEL, 0x0);
				snd_soc_component_write(codec,
					DA_COMBINE_V3_DACR_PGA_MUX_SEL, 0x0);
			} else if (params->rate == SLIMBUS_SAMPLE_RATE_192K) {
				snd_soc_component_write(codec,
					DA_COMBINE_V3_SLIM0_CLK_CFG_REG, 0x66);
				snd_soc_component_write(codec,
					DA_COMBINE_V3_DACL_PGA_MUX_SEL, 0x02);
				snd_soc_component_write(codec,
					DA_COMBINE_V3_DACR_PGA_MUX_SEL, 0x02);
			} else {
				snd_soc_component_write(codec,
					DA_COMBINE_V3_SLIM0_CLK_CFG_REG, 0x44);
				snd_soc_component_write(codec,
					DA_COMBINE_V3_DACL_PGA_MUX_SEL, 0x0);
				snd_soc_component_write(codec,
					DA_COMBINE_V3_DACR_PGA_MUX_SEL, 0x0);
			}
		}

		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			track, params);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			track, params);
		snd_soc_component_write(codec, DA_COMBINE_V3_SLIM0_CLK_CFG_REG, 0x44);
		snd_soc_component_write(codec, DA_COMBINE_V3_DACL_PGA_MUX_SEL, 0x0);
		snd_soc_component_write(codec, DA_COMBINE_V3_DACR_PGA_MUX_SEL, 0x0);
		break;
	default:
		ret = -1;
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static int play48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	return slimbus_param_pass(codec,
		SLIMBUS_TRACK_AUDIO_PLAY, NULL, event);
}

static int play96k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	priv->play_params.rate = SLIMBUS_SAMPLE_RATE_96K;

	return slimbus_param_pass(codec, SLIMBUS_TRACK_DIRECT_PLAY,
		&priv->play_params, event);
}

static int play192k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	priv->play_params.rate = SLIMBUS_SAMPLE_RATE_192K;

	return slimbus_param_pass(codec, SLIMBUS_TRACK_DIRECT_PLAY,
		&priv->play_params, event);
}

static int lowlatency_play_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			SLIMBUS_TRACK_FAST_PLAY, NULL);
		snd_soc_component_write(codec, DA_COMBINE_V3_I2S_SLIM_SEL_REG, 0x3F);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			SLIMBUS_TRACK_FAST_PLAY, NULL);
		snd_soc_component_write(codec, DA_COMBINE_V3_I2S_SLIM_SEL_REG, 0x33);
		break;
	default:
		ret = -1;
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

static void headphone_low_to_high_ib05_current_config(struct snd_soc_component *codec)
{
	/* headphone feed back mode config */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
		0x1 << DA_COMBINE_V3_HP_FB_STB_CTRL_BIT,
		0x1 << DA_COMBINE_V3_HP_FB_STB_CTRL_BIT);
	/* headphone feed back enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
		0x1 << DA_COMBINE_V3_HP_FB_EN_BIT,
		0x1 << DA_COMBINE_V3_HP_FB_EN_BIT);
	/* headphone mode select high mode */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
		0x1 << DA_COMBINE_V3_HP_SRB_CTRL_BIT,
		0x1 << DA_COMBINE_V3_HP_SRB_CTRL_BIT);

	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x33);
	usleep_range(10000, 11000);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x44);
	usleep_range(10000, 11000);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x55);
	usleep_range(10000, 11000);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x66);
}

static void headphone_high_to_low_ib05_current_config(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x55);
	usleep_range(10000, 11000);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x44);
	usleep_range(10000, 11000);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x33);
	usleep_range(10000, 11000);
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x22);

	/* headphone mode select low mode */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
		0x1 << DA_COMBINE_V3_HP_SRB_CTRL_BIT,
		0 << DA_COMBINE_V3_HP_SRB_CTRL_BIT);
	/* headphone feed back disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
		0x1 << DA_COMBINE_V3_HP_FB_EN_BIT,
		0 << DA_COMBINE_V3_HP_FB_EN_BIT);
	/* headphone feed back mode config */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
		0x1 << DA_COMBINE_V3_HP_FB_STB_CTRL_BIT,
		0 << DA_COMBINE_V3_HP_FB_STB_CTRL_BIT);
}

void headphone_high_mode_shadow_config(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *platform_data =
		snd_soc_component_get_drvdata(codec);

	/* reg pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_CLASSH_CTRL4, 0x0);
	/* shadow dacl pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL6,
		platform_data->hs_high_pga_gain);
	/* shadow dacr pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL7,
		platform_data->hs_high_pga_gain);
	/* dacl pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_DACL_PGA_GAIN_CFG_REG,
		platform_data->hs_high_pga_gain);
	/* dacr pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_DACR_PGA_GAIN_CFG_REG,
		platform_data->hs_high_pga_gain);

	/* shadow headphone res select */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG38_SHADOW,
		0x7 << DA_COMBINE_V3_HP_RES_CT_SHADOW_BIT,
		0x2 << DA_COMBINE_V3_HP_RES_CT_SHADOW_BIT);
	/* headphone res select */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG38,
		0x7 << DA_COMBINE_V3_HP_RES_CT_BIT,
		0x2 << DA_COMBINE_V3_HP_RES_CT_BIT);
	/* shadow headphone cell sel enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_HP_SDM45_CONFIG_SHADOW_REG0,
		0x3 << DA_COMBINE_V3_HP_SDM_L_CELL_SEL_EN_SHADOW_BIT,
		0x2 << DA_COMBINE_V3_HP_SDM_L_CELL_SEL_EN_SHADOW_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_HP_SDM45_CONFIG_SHADOW_REG0,
		0x3 << DA_COMBINE_V3_HP_SDM_R_CELL_SEL_EN_SHADOW_BIT,
		0x2 << DA_COMBINE_V3_HP_SDM_R_CELL_SEL_EN_SHADOW_BIT);
	/* headphone cell sel enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_HPL_SDM45_CONFIG_REG0,
		0x3 << DA_COMBINE_V3_HP_SDM_L_CELL_SEL_EN_BIT,
		0x2 << DA_COMBINE_V3_HP_SDM_L_CELL_SEL_EN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_HPR_SDM45_CONFIG_REG0,
		0x3 << DA_COMBINE_V3_HP_SDM_R_CELL_SEL_EN_BIT,
		0x2 << DA_COMBINE_V3_HP_SDM_R_CELL_SEL_EN_BIT);
}

void headphone_low_mode_shadow_config(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *platform_data =
		snd_soc_component_get_drvdata(codec);

	/* reg pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_CLASSH_CTRL4, 0x19);
	/* shadow dacl pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL6,
		platform_data->hs_low_pga_gain);
	/* shadow dacr pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_PERF_SWITCH_CTRL7,
		platform_data->hs_low_pga_gain);
	/* dacl pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_DACL_PGA_GAIN_CFG_REG,
		platform_data->hs_low_pga_gain);
	/* dacr pga gain */
	snd_soc_component_write(codec, DA_COMBINE_V3_DACR_PGA_GAIN_CFG_REG,
		platform_data->hs_low_pga_gain);

	/* shadow headphone res select */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG38_SHADOW,
		0x7 << DA_COMBINE_V3_HP_RES_CT_SHADOW_BIT,
		0x5 << DA_COMBINE_V3_HP_RES_CT_SHADOW_BIT);
	/* headphone res select */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG38,
		0x7 << DA_COMBINE_V3_HP_RES_CT_BIT, 0x5 << DA_COMBINE_V3_HP_RES_CT_BIT);
	/* shadow headphone cell sel disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_HP_SDM45_CONFIG_SHADOW_REG0,
		0x3 << DA_COMBINE_V3_HP_SDM_L_CELL_SEL_EN_SHADOW_BIT,
		0x1 << DA_COMBINE_V3_HP_SDM_L_CELL_SEL_EN_SHADOW_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_HP_SDM45_CONFIG_SHADOW_REG0,
		0x3 << DA_COMBINE_V3_HP_SDM_R_CELL_SEL_EN_SHADOW_BIT,
		0x1 << DA_COMBINE_V3_HP_SDM_R_CELL_SEL_EN_SHADOW_BIT);
	/* headphone cell sel disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_HPL_SDM45_CONFIG_REG0,
		0x3 << DA_COMBINE_V3_HP_SDM_L_CELL_SEL_EN_BIT,
		0x1 << DA_COMBINE_V3_HP_SDM_L_CELL_SEL_EN_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_HPR_SDM45_CONFIG_REG0,
		0x3 << DA_COMBINE_V3_HP_SDM_R_CELL_SEL_EN_BIT,
		0x1 << DA_COMBINE_V3_HP_SDM_R_CELL_SEL_EN_BIT);
}


static int hp_high_level_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* hpdac high performance */
		priv->rcv_hp_classh_state =
			(unsigned int)priv->rcv_hp_classh_state &
			(~HP_CLASSH_STATE);
		set_classh_config(codec, priv->rcv_hp_classh_state);

		if (priv->board_config.hp_high_low_change_enable == true) {
			/* disable the trigger config */
			da_combine_update_bits(codec, DA_COMBINE_V3_HP_SW_CTRL10_REG,
				0x1 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT,
				0x0 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT);
			/* high mode shadow config  */
			headphone_high_mode_shadow_config(codec);
			/* headphone ib05 current config */
			headphone_low_to_high_ib05_current_config(codec);
			/* enable the trigger config */
			da_combine_update_bits(codec, DA_COMBINE_V3_HP_SW_CTRL10_REG,
				0x1 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT,
				0x1 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT);
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* hpdac lower power */
		priv->rcv_hp_classh_state =
			(unsigned int)priv->rcv_hp_classh_state |
			HP_CLASSH_STATE;
		set_classh_config(codec, priv->rcv_hp_classh_state);

		if (priv->board_config.hp_high_low_change_enable == true) {
			/* disable the trigger config */
			da_combine_update_bits(codec, DA_COMBINE_V3_HP_SW_CTRL10_REG,
				0x1 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT,
				0x0 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT);
			/* low mode shadow config  */
			headphone_low_mode_shadow_config(codec);
			/* headphone ib05 current config */
			headphone_high_to_low_ib05_current_config(codec);
			/* enable the trigger config */
			da_combine_update_bits(codec, DA_COMBINE_V3_HP_SW_CTRL10_REG,
				0x1 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT,
				0x1 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT);
		}
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

#ifdef CONFIG_BUCKBOOST
static int auxmic_pwm_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ret = buck_boost_set_pwm_enable(1);
		if (ret < 0)
			AUDIO_LOGW("set pwm enable fail: %d", event);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = buck_boost_set_pwm_enable(0);
		if (ret < 0)
			AUDIO_LOGW("set pwm disable fail: %d", event);
		break;
	default:
		AUDIO_LOGW("power mode event err: %d", event);
		break;
	}

	return 0;
}
#endif

static int pll_param_pass(struct snd_soc_dapm_widget *w,
	enum da_combine_pll_type pll_type, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *platform_data =
		snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_resmgr_request_pll(platform_data->resmgr, pll_type);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_resmgr_release_pll(platform_data->resmgr, pll_type);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int pll48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	pll_param_pass(w, PLL_HIGH, event);

	return 0;
}

static int pll44k1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	pll_param_pass(w, PLL_44_1, event);

	return 0;
}

static int pllmad_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	pll_param_pass(w, PLL_LOW, event);

	return 0;
}

static int i2s2_bluetooth_loop_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* s2 func mode  PCM STD */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			7 << DA_COMBINE_V3_S2_MODE_CFG_BIT,
			2 << DA_COMBINE_V3_S2_MODE_CFG_BIT);
		/* s2 direct loop Sdin->Sdout */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			3 << DA_COMBINE_V3_S2_DRIECT_CFG_BIT,
			1 << DA_COMBINE_V3_S2_DRIECT_CFG_BIT);
		/* s2 mater mode */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			1 << DA_COMBINE_V3_S2_MST_SLV_SEL_BIT,
			0 << DA_COMBINE_V3_S2_MST_SLV_SEL_BIT);
		/* s2 if rx en */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			1 << DA_COMBINE_V3_S2_RX_EN_BIT, 1 << DA_COMBINE_V3_S2_RX_EN_BIT);
		/* s2 if tx en */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			1 << DA_COMBINE_V3_S2_TX_EN_BIT, 1 << DA_COMBINE_V3_S2_TX_EN_BIT);
		/* s2 clk if en */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CLK_CFG_REG,
			1 << DA_COMBINE_V3_S2_CLKEN_BIT, 1 << DA_COMBINE_V3_S2_CLKEN_BIT);
		/* s2 freq */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CLK_CFG_REG,
			7 << DA_COMBINE_V3_S2_CLK_SEL_BIT, 0 << DA_COMBINE_V3_S2_CLK_SEL_BIT);
		/* s2 frame mode */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG_H,
			1 << DA_COMBINE_V3_S2_FRAME_MODE_BIT,
			0 << DA_COMBINE_V3_S2_FRAME_MODE_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* s2 func mode  PCM STD */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			7 << DA_COMBINE_V3_S2_MODE_CFG_BIT, 0);
		/* s2 direct loop Sdin->Sdout */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			3 << DA_COMBINE_V3_S2_DRIECT_CFG_BIT, 0);
		/* s2 mater mode */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			1 << DA_COMBINE_V3_S2_MST_SLV_SEL_BIT, 0);
		/* s2 if rx en */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			1 << DA_COMBINE_V3_S2_RX_EN_BIT, 0);
		/* s2 if tx en */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			1 << DA_COMBINE_V3_S2_TX_EN_BIT, 0);
		/* s2 clk if en */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CLK_CFG_REG,
			1 << DA_COMBINE_V3_S2_CLKEN_BIT, 0);
		/* s2 freq */
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CLK_CFG_REG,
			7 << DA_COMBINE_V3_S2_CLK_SEL_BIT, 0);
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
		da_combine_update_bits(codec, DA_COMBINE_V3_U3_SRC_MODE_CFG_REG, 0x7, 0x0);
		snd_soc_component_write(codec, DA_COMBINE_V3_SLIM4_CLK_CFG_REG, 0x11);

		/* slimbus soundtrigger active */
		ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			SLIMBUS_TRACK_KWS, params);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* slimbus soundtrigger deactive */
		ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V3,
			SLIMBUS_TRACK_KWS, NULL);
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
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	priv->soundtrigger_params.channels = 1;

	return soundtrigger_u3u4_power_event(codec,
		&priv->soundtrigger_params, event);
}

static int soundtrigger_dualmic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	priv->soundtrigger_params.channels = 2;

	return soundtrigger_u3u4_power_event(codec,
		&priv->soundtrigger_params, event);
}

#define SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("MAD SWITCH", \
		DA_COMBINE_V3_MAD_CFG_REG, DA_COMBINE_V3_MAD_EN_BIT, 0, \
		&dapm_mad_switch_controls, \
		madswitch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("S1 OL SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s1_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("S1 OR SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s1_or_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("S2 OL SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s2_ol_switch_controls, \
		s2up_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("S2 OR SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s2_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("S3 OL SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s3_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("S3 OR SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s3_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("S4 OL SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s4_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("S4 OR SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_s4_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("U3 OL SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_u3_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("U4 OR SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_u4_or_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("AUDIOUP SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_audioup_switch_controls, \
		audioup_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIOUP 4MIC SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_audioup_4mic_switch_controls, \
		audioup_4mic_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE8K SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice8k_switch_controls, \
		voice8k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE16K SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice16k_switch_controls, \
		voice16k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE32K SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_voice32k_switch_controls, \
		voice32k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY48K SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play48k_switch_controls, \
		play48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY96K SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play96k_switch_controls, \
		play96k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY192K SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_play192k_switch_controls, \
		play192k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("LOWLATENCY SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_lowlatency_switch_controls, \
		lowlatency_play_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("HPHIGHLEVEL SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_hp_high_level_switch_controls, \
		hp_high_level_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLL48K SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_pll48k_switch_controls, \
		pll48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLL44K1 SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_pll44k1_switch_controls, \
		pll44k1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLLMAD SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_pllmad_switch_controls, \
		pllmad_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("IR STUDY ENV SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_ir_env_study_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("I2S2 BLUETOOTH LOOP SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_i2s2_bluetooth_loop_switch_controls, \
		i2s2_bluetooth_loop_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("SOUNDTRIGGER ONEMIC SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_soundtrigger_onemic_switch_controls, \
		soundtrigger_onemic_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("SOUNDTRIGGER DUALMIC SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_soundtrigger_dualmic_switch_controls, \
		soundtrigger_dualmic_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

static const struct snd_soc_dapm_widget switch_widgets[] = {
	SWITCH_WIDGET
#ifdef CONFIG_BUCKBOOST
	SND_SOC_DAPM_SWITCH_E("AUXMICPWMPOWER SWITCH",
		SND_SOC_NOPM, 0, 0,
		&dapm_auxmic_pwm_power_switch_controls,
		auxmic_pwm_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
#endif
};

int add_switch_widgets(struct snd_soc_component *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	return snd_soc_dapm_new_controls(dapm, switch_widgets,
		ARRAY_SIZE(switch_widgets));
}

