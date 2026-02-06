/*
 * single_pga_widget.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "single_pga_widget.h"

#include <sound/core.h>

#include "slimbus_da_combine_v5.h"
#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_v5.h"
#include "linux/hisi/da_combine/da_combine_v5_regs.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "linux/hisi/da_combine/da_combine_v5_type.h"


static void bt_play_pga_enable(struct snd_soc_component *codec)
{
	AUDIO_LOGI("begin");

	if (codec == NULL) {
		AUDIO_LOGE("snd soc codec is NULL");
		return;
	}

	/* "DACL_MIXER" */
	da_combine_update_bits(codec, DACL_MIXER4_CTRL1_REG,
		MASK_ON_BIT(DACL_MIXER4_IN3_MUTE_LEN, DACL_MIXER4_IN3_MUTE_OFFSET), 0x0);
	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		MASK_ON_BIT(DACL_MIXER4_CLK_EN_LEN, DACL_MIXER4_CLK_EN_OFFSET),
		BIT(DACL_MIXER4_CLK_EN_OFFSET));
	/* "DACR_MIXER" */
	da_combine_update_bits(codec, DACR_MIXER4_CTRL1_REG,
		MASK_ON_BIT(DACR_MIXER4_IN3_MUTE_LEN, DACR_MIXER4_IN3_MUTE_OFFSET), 0x0);
	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		MASK_ON_BIT(DACR_MIXER4_CLK_EN_LEN, DACR_MIXER4_CLK_EN_OFFSET),
		BIT(DACR_MIXER4_CLK_EN_OFFSET));
	/* SRC */
	da_combine_update_bits(codec, S3_DP_CLK_EN_REG,
		MASK_ON_BIT(S3_IL_PGA_CLK_EN_LEN, S3_IL_PGA_CLK_EN_OFFSET),
		BIT(S3_IL_PGA_CLK_EN_OFFSET));
	da_combine_update_bits(codec, S3_DP_CLK_EN_REG,
		MASK_ON_BIT(S3_IR_PGA_CLK_EN_LEN, S3_IR_PGA_CLK_EN_OFFSET),
		BIT(S3_IR_PGA_CLK_EN_OFFSET));
	da_combine_update_bits(codec, S3_DP_CLK_EN_REG,
		MASK_ON_BIT(S3_IL_SRC_CLK_EN_LEN, S3_IL_SRC_CLK_EN_OFFSET),
		BIT(S3_IL_SRC_CLK_EN_OFFSET));
	da_combine_update_bits(codec, S3_DP_CLK_EN_REG,
		MASK_ON_BIT(S3_IR_SRC_CLK_EN_LEN, S3_IR_SRC_CLK_EN_OFFSET),
		BIT(S3_IR_SRC_CLK_EN_OFFSET));
	/* MDM MUX */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL14_REG,
		MASK_ON_BIT(MDM_SEL_LEN, MDM_SEL_OFFSET),
		BIT(MDM_SEL_OFFSET));
	/* config d5 d6 48k */
	da_combine_update_bits(codec, SC_FS_SLIM_CTRL_2_REG,
		MASK_ON_BIT(FS_SLIM_DN5_LEN, FS_SLIM_DN5_OFFSET),
		0x4);
	da_combine_update_bits(codec, SC_FS_SLIM_CTRL_2_REG,
		MASK_ON_BIT(FS_SLIM_DN6_LEN, FS_SLIM_DN6_OFFSET),
		0x4 << FS_SLIM_DN6_OFFSET);
	/* src rate */
	da_combine_update_bits(codec, SRC_VLD_CTRL1_REG,
		MASK_ON_BIT(S3_IL_SRC_DOUT_VLD_SEL_LEN, S3_IL_SRC_DOUT_VLD_SEL_OFFSET),
		BIT(S3_IL_SRC_DOUT_VLD_SEL_OFFSET));
	da_combine_update_bits(codec, SRC_VLD_CTRL1_REG,
		MASK_ON_BIT(S3_IR_SRC_DOUT_VLD_SEL_LEN, S3_IR_SRC_DOUT_VLD_SEL_OFFSET),
		BIT(S3_IR_SRC_DOUT_VLD_SEL_OFFSET));
	da_combine_update_bits(codec, SRC_VLD_CTRL1_REG,
		MASK_ON_BIT(S3_I_SRC_DIN_VLD_SEL_LEN, S3_I_SRC_DIN_VLD_SEL_OFFSET),
		0x4 << S3_I_SRC_DIN_VLD_SEL_OFFSET);
	da_combine_update_bits(codec, S3_IL_SRC_CTRL_REG,
		MASK_ON_BIT(S3_IL_SRC_MODE_LEN, S3_IL_SRC_MODE_OFFSET),
		0x4 << S3_IL_SRC_MODE_OFFSET);
	da_combine_update_bits(codec, S3_IR_SRC_CTRL_REG,
		MASK_ON_BIT(S3_IR_SRC_MODE_LEN, S3_IR_SRC_MODE_OFFSET),
		0x4 << S3_IR_SRC_MODE_OFFSET);

	AUDIO_LOGI("end");
}

static void bt_play_pga_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGI("begin");

	if (codec == NULL) {
		AUDIO_LOGE("snd soc codec is NULL");
		return;
	}

	/* "DACL_MIXER" */
	da_combine_update_bits(codec, DACL_MIXER4_CTRL1_REG,
		MASK_ON_BIT(DACL_MIXER4_IN3_MUTE_LEN, DACL_MIXER4_IN3_MUTE_OFFSET), BIT(DACL_MIXER4_IN3_MUTE_OFFSET));
	/* "DACR_MIXER" */
	da_combine_update_bits(codec, DACR_MIXER4_CTRL1_REG,
		MASK_ON_BIT(DACR_MIXER4_IN3_MUTE_LEN, DACR_MIXER4_IN3_MUTE_OFFSET), BIT(DACL_MIXER4_IN3_MUTE_OFFSET));
	/* SRC */
	da_combine_update_bits(codec, S3_DP_CLK_EN_REG,
		MASK_ON_BIT(S3_IL_PGA_CLK_EN_LEN, S3_IL_PGA_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, S3_DP_CLK_EN_REG,
		MASK_ON_BIT(S3_IR_PGA_CLK_EN_LEN, S3_IR_PGA_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, S3_DP_CLK_EN_REG,
		MASK_ON_BIT(S3_IL_SRC_CLK_EN_LEN, S3_IL_SRC_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, S3_DP_CLK_EN_REG,
		MASK_ON_BIT(S3_IR_SRC_CLK_EN_LEN, S3_IR_SRC_CLK_EN_OFFSET), 0x0);
	/* MDM MUX */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL14_REG,
		MASK_ON_BIT(MDM_SEL_LEN, MDM_SEL_OFFSET),
		0);
	/* src rate */
	da_combine_update_bits(codec, SRC_VLD_CTRL1_REG,
		MASK_ON_BIT(S3_IL_SRC_DOUT_VLD_SEL_LEN, S3_IL_SRC_DOUT_VLD_SEL_OFFSET),
		0x0 << S3_IL_SRC_DOUT_VLD_SEL_OFFSET);
	da_combine_update_bits(codec, SRC_VLD_CTRL1_REG,
		MASK_ON_BIT(S3_IR_SRC_DOUT_VLD_SEL_LEN, S3_IR_SRC_DOUT_VLD_SEL_OFFSET),
		0x0 << S3_IR_SRC_DOUT_VLD_SEL_OFFSET);
	da_combine_update_bits(codec, SRC_VLD_CTRL1_REG,
		MASK_ON_BIT(S3_I_SRC_DIN_VLD_SEL_LEN, S3_I_SRC_DIN_VLD_SEL_OFFSET),
		0x0 << S3_I_SRC_DIN_VLD_SEL_OFFSET);
	da_combine_update_bits(codec, S3_IL_SRC_CTRL_REG,
		MASK_ON_BIT(S3_IL_SRC_MODE_LEN, S3_IL_SRC_MODE_OFFSET),
		0x0 << S3_IL_SRC_MODE_OFFSET);
	da_combine_update_bits(codec, S3_IR_SRC_CTRL_REG,
		MASK_ON_BIT(S3_IR_SRC_MODE_LEN, S3_IR_SRC_MODE_OFFSET),
		0x0 << S3_IR_SRC_MODE_OFFSET);

	AUDIO_LOGI("end");
}

static int bt_play_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		bt_play_pga_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		bt_play_pga_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static void audio_play_pga_enable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	/* "DACL_MIXER" */
	da_combine_update_bits(codec, DACL_MIXER4_CTRL1_REG,
		MASK_ON_BIT(DACL_MIXER4_IN1_MUTE_LEN, DACL_MIXER4_IN1_MUTE_OFFSET), 0x0);
	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		MASK_ON_BIT(DACL_MIXER4_CLK_EN_LEN, DACL_MIXER4_CLK_EN_OFFSET),
		BIT(DACL_MIXER4_CLK_EN_OFFSET));
	/* "DACR_MIXER" */
	da_combine_update_bits(codec, DACR_MIXER4_CTRL1_REG,
		MASK_ON_BIT(DACR_MIXER4_IN1_MUTE_LEN, DACR_MIXER4_IN1_MUTE_OFFSET), 0x0);
	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		MASK_ON_BIT(DACR_MIXER4_CLK_EN_LEN, DACR_MIXER4_CLK_EN_OFFSET),
		BIT(DACR_MIXER4_CLK_EN_OFFSET));
	/* "DACL_PRE_MIXER" */
	da_combine_update_bits(codec, DACL_PRE_MIXER2_CTRL0_REG,
		MASK_ON_BIT(DACL_PRE_MIXER2_IN1_MUTE_LEN, DACL_PRE_MIXER2_IN1_MUTE_OFFSET), 0x0);
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(DACL_PRE_MIXER2_CLK_EN_LEN, DACL_PRE_MIXER2_CLK_EN_OFFSET),
		BIT(DACL_PRE_MIXER2_CLK_EN_OFFSET));
	/* "DACR_PRE_MIXER" */
	da_combine_update_bits(codec, DACR_PRE_MIXER2_CTRL0_REG,
		MASK_ON_BIT(DACR_PRE_MIXER2_IN1_MUTE_LEN, DACR_PRE_MIXER2_IN1_MUTE_OFFSET), 0x0);
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(DACR_PRE_MIXER2_CLK_EN_LEN, DACR_PRE_MIXER2_CLK_EN_OFFSET),
		BIT(DACR_PRE_MIXER2_CLK_EN_OFFSET));
	/* "S1_IL_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL0_REG,
		MASK_ON_BIT(S1_IL_PGA_DIN_SEL_LEN, S1_IL_PGA_DIN_SEL_OFFSET),
		BIT(S1_IL_PGA_DIN_SEL_OFFSET));
	/* "S1_IR_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL0_REG,
		MASK_ON_BIT(S1_IR_PGA_DIN_SEL_LEN, S1_IR_PGA_DIN_SEL_OFFSET),
		BIT(S1_IR_PGA_DIN_SEL_OFFSET));
	/* "DACL_PRE_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
		MASK_ON_BIT(DACL_PRE_MIXER2_IN1_SEL_LEN, DACL_PRE_MIXER2_IN1_SEL_OFFSET),
		BIT(DACL_PRE_MIXER2_IN1_SEL_OFFSET));
	/* "DACR_PRE_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
		MASK_ON_BIT(DACR_PRE_MIXER2_IN1_SEL_LEN, DACR_PRE_MIXER2_IN1_SEL_OFFSET),
		BIT(DACR_PRE_MIXER2_IN1_SEL_OFFSET));
	/* "PGA_BYPASS" */
	da_combine_update_bits(codec, DACL_PRE_PGA_CTRL0_REG,
		MASK_ON_BIT(DACL_PRE_PGA_BYPASS_LEN, DACL_PRE_PGA_BYPASS_OFFSET),
		BIT(DACL_PRE_PGA_BYPASS_OFFSET));
	da_combine_update_bits(codec, DACR_PRE_PGA_CTRL0_REG,
		MASK_ON_BIT(DACR_PRE_PGA_BYPASS_LEN, DACR_PRE_PGA_BYPASS_OFFSET),
		BIT(DACR_PRE_PGA_BYPASS_OFFSET));
	da_combine_update_bits(codec, DACL_POST_PGA_CTRL0_REG,
		MASK_ON_BIT(DACL_POST_PGA_BYPASS_LEN, DACL_POST_PGA_BYPASS_OFFSET),
		BIT(DACL_POST_PGA_BYPASS_OFFSET));
	da_combine_update_bits(codec, DACR_POST_PGA_CTRL0_REG,
		MASK_ON_BIT(DACR_POST_PGA_BYPASS_LEN, DACR_POST_PGA_BYPASS_OFFSET),
		BIT(DACR_POST_PGA_BYPASS_OFFSET));

	AUDIO_LOGD("end");
}

void audio_play_pga_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	/* "DACL_MXIER" */
	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		MASK_ON_BIT(DACL_MIXER4_CLK_EN_LEN, DACL_MIXER4_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, DACL_MIXER4_CTRL1_REG,
		MASK_ON_BIT(DACL_MIXER4_IN1_MUTE_LEN, DACL_MIXER4_IN1_MUTE_OFFSET),
		BIT(DACL_MIXER4_IN1_MUTE_OFFSET));
	/* "DACR_MIXER" */
	da_combine_update_bits(codec, DAC_DP_CLK_EN_1_REG,
		MASK_ON_BIT(DACR_MIXER4_CLK_EN_LEN, DACR_MIXER4_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, DACR_MIXER4_CTRL1_REG,
		MASK_ON_BIT(DACR_MIXER4_IN1_MUTE_LEN, DACR_MIXER4_IN1_MUTE_OFFSET),
		BIT(DACR_MIXER4_IN1_MUTE_OFFSET));
	/* "DACL_PRE_MIXER" */
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(DACL_PRE_MIXER2_CLK_EN_LEN, DACL_PRE_MIXER2_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, DACL_PRE_MIXER2_CTRL0_REG,
		MASK_ON_BIT(DACL_PRE_MIXER2_IN1_MUTE_LEN, DACL_PRE_MIXER2_IN1_MUTE_OFFSET),
		BIT(DACL_PRE_MIXER2_IN1_MUTE_OFFSET));
	/* "DACR_PRE_MIXER" */
	da_combine_update_bits(codec, DAC_MIXER_CLK_EN_REG,
		MASK_ON_BIT(DACR_PRE_MIXER2_CLK_EN_LEN, DACR_PRE_MIXER2_CLK_EN_OFFSET), 0x0);
	da_combine_update_bits(codec, DACR_PRE_MIXER2_CTRL0_REG,
		MASK_ON_BIT(DACR_PRE_MIXER2_IN1_MUTE_LEN, DACR_PRE_MIXER2_IN1_MUTE_OFFSET),
		BIT(DACR_PRE_MIXER2_IN1_MUTE_OFFSET));
	/* "S1_IL_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL0_REG,
		MASK_ON_BIT(S1_IL_PGA_DIN_SEL_LEN, S1_IL_PGA_DIN_SEL_OFFSET), 0x0);
	/* "S1_IR_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL0_REG,
		MASK_ON_BIT(S1_IR_PGA_DIN_SEL_LEN, S1_IR_PGA_DIN_SEL_OFFSET), 0x0);
	/* "DACL_PRE_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
		MASK_ON_BIT(DACL_PRE_MIXER2_IN1_SEL_LEN, DACL_PRE_MIXER2_IN1_SEL_OFFSET), 0x0);
	/* "DACR_PRE_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL9_REG,
		MASK_ON_BIT(DACR_PRE_MIXER2_IN1_SEL_LEN, DACR_PRE_MIXER2_IN1_SEL_OFFSET), 0x0);
	/* "PGA_BYPASS" */
	da_combine_update_bits(codec, DACL_PRE_PGA_CTRL0_REG,
		MASK_ON_BIT(DACL_PRE_PGA_BYPASS_LEN, DACL_PRE_PGA_BYPASS_OFFSET), 0x0);
	da_combine_update_bits(codec, DACR_PRE_PGA_CTRL0_REG,
		MASK_ON_BIT(DACR_PRE_PGA_BYPASS_LEN, DACR_PRE_PGA_BYPASS_OFFSET), 0x0);
	da_combine_update_bits(codec, DACL_POST_PGA_CTRL0_REG,
		MASK_ON_BIT(DACL_POST_PGA_BYPASS_LEN, DACL_POST_PGA_BYPASS_OFFSET), 0x0);
	da_combine_update_bits(codec, DACR_POST_PGA_CTRL0_REG,
		MASK_ON_BIT(DACR_POST_PGA_BYPASS_LEN, DACR_POST_PGA_BYPASS_OFFSET), 0x0);

	AUDIO_LOGD("end");
}

static int audio_play_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		audio_play_pga_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		audio_play_pga_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static void audio_capture_pga_enable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	/* "MIC1_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL29_REG,
		MASK_ON_BIT(MIC_1_SEL_LEN, MIC_1_SEL_OFFSET), 0x0);
	/* "MIC2_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL29_REG,
		MASK_ON_BIT(MIC_2_SEL_LEN, MIC_2_SEL_OFFSET), 0X0);
	/* "MIC3_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL28_REG,
		MASK_ON_BIT(MIC_3_SEL_LEN, MIC_3_SEL_OFFSET), BIT(MIC_3_SEL_OFFSET));
	/* "MIC4_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL28_REG,
		MASK_ON_BIT(MIC_4_SEL_LEN, MIC_4_SEL_OFFSET), BIT(MIC_4_SEL_OFFSET));

	AUDIO_LOGD("end");
}

static void audio_capture_pga_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	/* "MIC1_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL29_REG,
		MASK_ON_BIT(MIC_1_SEL_LEN, MIC_1_SEL_OFFSET), 0x0);
	/* "MIC2_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL29_REG,
		MASK_ON_BIT(MIC_2_SEL_LEN, MIC_2_SEL_OFFSET), 0X0);
	/* "MIC3_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL28_REG,
		MASK_ON_BIT(MIC_3_SEL_LEN, MIC_3_SEL_OFFSET), BIT(MIC_3_SEL_OFFSET));
	/* "MIC4_MUX" */
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL28_REG,
		MASK_ON_BIT(MIC_4_SEL_LEN, MIC_4_SEL_OFFSET), BIT(MIC_4_SEL_OFFSET));

	AUDIO_LOGD("end");
}

static int audio_capture_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		audio_capture_pga_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		audio_capture_pga_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static int i2s2_common_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* "S2_FRAME_MODE" */
		da_combine_update_bits(codec, SC_S2_IF_H_REG,
			MASK_ON_BIT(S2_FRAME_MODE_LEN, S2_FRAME_MODE_OFFSET), 0x0);
		/* "S2_RX_CLK_SEL" */
		da_combine_update_bits(codec, SC_S2_IF_H_REG,
			MASK_ON_BIT(S2_RX_CLK_SEL_LEN, S2_RX_CLK_SEL_OFFSET),
			BIT(S2_RX_CLK_SEL_OFFSET));
		/* "S2_TX_CLK_SEL" */
		da_combine_update_bits(codec, SC_S2_IF_H_REG,
			MASK_ON_BIT(S2_TX_CLK_SEL_LEN, S2_TX_CLK_SEL_OFFSET), 0x0);
		/* "S2_FUNC_MODE" */
		da_combine_update_bits(codec, SC_S2_IF_L_REG,
			MASK_ON_BIT(S2_FUNC_MODE_LEN, S2_FUNC_MODE_OFFSET),
			0x2 << S2_FUNC_MODE_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* "S2_FRAME_MODE" */
		da_combine_update_bits(codec, SC_S2_IF_H_REG,
			MASK_ON_BIT(S2_FRAME_MODE_LEN, S2_FRAME_MODE_OFFSET), 0x0);
		/* "S2_RX_CLK_SEL" */
		da_combine_update_bits(codec, SC_S2_IF_H_REG,
			MASK_ON_BIT(S2_RX_CLK_SEL_LEN, S2_RX_CLK_SEL_OFFSET), 0x0);
		/* "S2_TX_CLK_SEL" */
		da_combine_update_bits(codec, SC_S2_IF_H_REG,
			MASK_ON_BIT(S2_TX_CLK_SEL_LEN, S2_TX_CLK_SEL_OFFSET),
			BIT(S2_TX_CLK_SEL_OFFSET));
		/* "S2_FUNC_MODE" */
		da_combine_update_bits(codec, SC_S2_IF_L_REG,
			MASK_ON_BIT(S2_FUNC_MODE_LEN, S2_FUNC_MODE_OFFSET), 0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int i2s2_nb_pga_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		i2s2_common_power_event(w, kcontrol, event);
		/* "S2_IF_I2S_FS" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL23_REG,
			MASK_ON_BIT(FS_S2_LEN, FS_S2_OFFSET), 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		i2s2_common_power_event(w, kcontrol, event);
		/* "S2_IF_I2S_FS" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL23_REG,
			MASK_ON_BIT(FS_S2_LEN, FS_S2_OFFSET), 0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int i2s2_wb_pga_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		i2s2_common_power_event(w, kcontrol, event);
		/* "S2_IF_I2S_FS" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL23_REG,
			MASK_ON_BIT(FS_S2_LEN, FS_S2_OFFSET), BIT(FS_S2_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		i2s2_common_power_event(w, kcontrol, event);
		/* "S2_IF_I2S_FS" */
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL23_REG,
			MASK_ON_BIT(FS_S2_LEN, FS_S2_OFFSET), 0x0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

#define SINGLE_PGA_WIDGET \
	SND_SOC_DAPM_PGA_S("AUDIO_PLAY_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		audio_play_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("AUDIO_CAPTURE_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		audio_capture_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("BT_I2S2_NB_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		i2s2_nb_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("BT_I2S2_WB_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		i2s2_wb_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("BT_PLAY_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		bt_play_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

static const struct snd_soc_dapm_widget single_pga_widgets[] = {
	SINGLE_PGA_WIDGET
};

int da_combine_v5_add_single_pga_widgets(struct snd_soc_component *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	return snd_soc_dapm_new_controls(dapm, single_pga_widgets,
		ARRAY_SIZE(single_pga_widgets));
}

