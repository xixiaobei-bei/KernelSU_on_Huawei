/*
 * da_separate_mbhc_custom.c -- da_separate mbhc custom driver
 *
 * Copyright (c) 2020 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "da_separate_mbhc_custom.h"
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/version.h>

#include "asoc_adapter.h"
#include "hs_auto_calib.h"
#include "da_separate_reg.h"
#if (defined(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V5)) || (defined(CONFIG_SND_SOC_CODEC_DA_SEPARATE_V6L))
#include "da_separate_utils.h"
#include "da_separate_v5.h"
#endif

#include "audio_log.h"

#define LOG_TAG "da_separate_mbhc_custom"

#define MIC_SELECT_MASK                0x30
#define HEADSET_MIC_SELECTED           0x20
#define MIC_MUTE_TIME_MS               1300

static bool g_hs_mic_mute;

void da_separate_hs_mic_mute(struct snd_soc_component *codec)
{
	unsigned int hs_mic_select;
	WARN_ON(!codec);

	if (!g_hs_mic_mute) {
		AUDIO_LOGI("headset mic mute not support\n");
		return;
	}

	/* mute uplink ADCL&ADCR reg when headset plug out
	 * and headset-mic is using,
	 * unmute uplink ADCL&ADCR reg after delay 1300ms.
	 */
	hs_mic_select = snd_soc_component_read32(codec,
		CODEC_ANA_RW9_REG) & MIC_SELECT_MASK;
	if (hs_mic_select != HEADSET_MIC_SELECTED)
		return;
}

bool da_separate_get_mic_mute_status(void)
{
	return g_hs_mic_mute;
}

void da_separate_mbhc_custom_init(struct device_node *np)
{
	WARN_ON(!np);
	g_hs_mic_mute = false;
	if (of_property_read_bool(np, "hs_mic_mute"))
		g_hs_mic_mute = true;
}
