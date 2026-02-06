/*
 * single_switch_route.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "single_switch_route.h"

#include <sound/core.h>

#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_v5.h"

#define SINGLE_DP_SUPPLY_ROUTE \
	{ "U1_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U2_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U3_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U4_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "D1_D2_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "S2_RX_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "S2_TX_OUTPUT",                 NULL,              "DP_CLK_SUPPLY" }, \
	{ "S4_TX_OUTPUT",                 NULL,              "DP_CLK_SUPPLY" }, \
	{ "HP_L_OUTPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "HP_R_OUTPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "I2S2_BLUETOOTH_LOOP_SWITCH",   NULL,              "DP_CLK_SUPPLY" }, \

#define SINGLE_PLL_SUPPLY_ROUTE \
	{ "U1_OUTPUT",                    NULL,              "PLL_CLK_SUPPLY" }, \
	{ "U2_OUTPUT",                    NULL,              "PLL_CLK_SUPPLY" }, \
	{ "U3_OUTPUT",                    NULL,              "PLL_CLK_SUPPLY" }, \
	{ "U4_OUTPUT",                    NULL,              "PLL_CLK_SUPPLY" }, \
	{ "D1_D2_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "S2_RX_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "S2_TX_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY" }, \
	{ "S4_TX_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY" }, \
	{ "HSMIC_PGA",                    NULL,              "PLL_CLK_SUPPLY" }, \
	{ "I2S2_BLUETOOTH_LOOP_SWITCH",   NULL,              "PLL_CLK_SUPPLY" }, \

#define SINGLE_I2S_SUPPLY_ROUTE \
	{ "S2_RX_INPUT",                  NULL,              "S2_IF_CLK_SUPPLY" }, \
	{ "S2_TX_OUTPUT",                 NULL,              "S2_IF_CLK_SUPPLY" }, \
	{ "S4_RX_INPUT",                  NULL,              "S4_IF_CLK_SUPPLY" }, \
	{ "S4_TX_OUTPUT",                 NULL,              "S4_IF_CLK_SUPPLY" }, \
	{ "S4_RX_INPUT",                  NULL,              "S4_RX_SUPPLY" }, \

#define SINGLE_CP1_SUPPLY_ROUTE \
	{ "CP2_SUPPLY",                   NULL,              "CP1_SUPPLY" }, \

#define SINGLE_CP2_SUPPLY_ROUTE \
	{ "HP_L_OUTPUT",                  NULL,              "CP2_SUPPLY" }, \
	{ "HP_R_OUTPUT",                  NULL,              "CP2_SUPPLY" }, \

/* spk_route */
#define SINGLE_PLAY_SPK_ROUTE \
	{ "AUDIO_PLAY_PGA",               NULL,              "D1_D2_INPUT" }, \
	{ "S1_IL_PGA",                    NULL,              "AUDIO_PLAY_PGA" }, \
	{ "S1_IR_PGA",                    NULL,              "AUDIO_PLAY_PGA" }, \
	{ "AUDIO_PLAY_DRV",               NULL,              "S1_IL_PGA" }, \
	{ "AUDIO_PLAY_DRV",               NULL,              "S1_IR_PGA" }, \
	{ "Speaker Playback",             "Switch",          "AUDIO_PLAY_DRV" }, \
	{ "S4_TX_DRV",                    NULL,              "Speaker Playback" }, \
	{ "S4_TX_OUTPUT",                 NULL,              "S4_TX_DRV" }, \
	{ "Speaker Playback",             "Switch",          "S4_RX_INPUT" }, \
	{ "IV_DSPIF_OUTPUT",              NULL,              "Speaker Playback" }, \

/* hp_route */
#define SINGLE_PLAY_HP_ROUTE \
	{ "Headset Playback",             "Switch",          "AUDIO_PLAY_DRV" }, \
	{ "HP_L_DRV",                     NULL,              "Headset Playback" }, \
	{ "HP_R_DRV",                     NULL,              "Headset Playback" }, \
	{ "HP_L_OUTPUT",                  NULL,              "HP_L_DRV" }, \
	{ "HP_R_OUTPUT",                  NULL,              "HP_R_DRV" }, \

#define SINGLE_HEADSET_MIC_CAPTURE_ROUTE \
	{ "HSMIC_INPUT",                  NULL,              "HSMICBIAS"}, \
	{ "HSMIC_PGA",                    NULL,              "HSMIC_INPUT" }, \
	{ "Headset Mic Capture",          "Switch",          "HSMIC_PGA" }, \
	{ "AUDIO_CAPTURE_PGA",            NULL,              "Headset Mic Capture" }, \

#define SINGLE_MIC_CAPTURE_ROUTE \
	{ "Mic Capture",                  "Switch",          "MIC_INPUT" }, \
	{ "AUDIO_CAPTURE_PGA",            NULL,              "Mic Capture" }, \

#define SINGLE_CAPTURE_ROUTE \
	{ "AUDIO_CAPTURE_DRV",            NULL,              "AUDIO_CAPTURE_PGA" }, \
	{ "ADCL0",                        NULL,              "AUDIO_CAPTURE_DRV" }, \
	{ "ADCR0",                        NULL,              "AUDIO_CAPTURE_DRV" }, \
	{ "ADCL1",                        NULL,              "AUDIO_CAPTURE_DRV" }, \
	{ "ADCR1",                        NULL,              "AUDIO_CAPTURE_DRV" }, \
	{ "U1_DRV",                       NULL,              "ADCL0" }, \
	{ "U2_DRV",                       NULL,              "ADCR0" }, \
	{ "U3_DRV",                       NULL,              "ADCL1" }, \
	{ "U4_DRV",                       NULL,              "ADCR1" }, \
	{ "U1_OUTPUT",                    NULL,              "U1_DRV" }, \
	{ "U2_OUTPUT",                    NULL,              "U2_DRV" }, \
	{ "U3_OUTPUT",                    NULL,              "U3_DRV" }, \
	{ "U4_OUTPUT",                    NULL,              "U4_DRV" }, \

#define SINGLE_EC_ROUTE \
	{ "Headset Mic Capture",          "Switch",          "U7_EC_INPUT" }, \
	{ "U7_DRV",                       NULL,              "Headset Mic Capture" }, \
	{ "Mic Capture",                  "Switch",          "U7_EC_INPUT" }, \
	{ "U7_DRV",                       NULL,              "Mic Capture" }, \
	{ "EC_DRV",                       NULL,              "U7_DRV" }, \
	{ "U7_EC_OUTPUT",                 NULL,              "EC_DRV" }, \

#define SINGLE_SOURCE_SELECT_ROUTE \
	{ "S1_IL_MUX",                    "D1",              "D1_D2_INPUT" }, \
	{ "S1_IR_MUX",                    "D2",              "D1_D2_INPUT" }, \
	{ "ADCL0_MUX",                    "DMIC0L",          "MIC_INPUT" }, \
	{ "ADCR0_MUX",                    "DMIC1L",          "MIC_INPUT" }, \
	{ "ADCL1_MUX",                    "DMIC0R",          "MIC_INPUT" }, \
	{ "ADCR1_MUX",                    "DMIC1R",          "MIC_INPUT" }, \

#define SINGLE_BT_LOOP_ROUTE \
	{ "I2S2_BLUETOOTH_LOOP_SWITCH",   "ENABLE",          "I2S2_BLUETOOTH_LOOP_INPUT" }, \
	{ "I2S2_BLUETOOTH_LOOP_OUTPUT",   NULL,              "I2S2_BLUETOOTH_LOOP_SWITCH" }, \

#define SINGLE_BT_PLAYBACK_ROUTE \
	{ "AUDIO_PLAY_PGA",               NULL,              "D5_D6_INPUT" }, \
	{ "BT_I2S2_NB_PGA",               NULL,              "AUDIO_PLAY_PGA" }, \
	{ "Bt Playback",                  "Switch",          "BT_I2S2_NB_PGA" }, \
	{ "BT_PLAY_PGA",                  NULL,              "Bt Playback" }, \
	{ "BT_PLAY_DRV",                  NULL,              "BT_PLAY_PGA" }, \
	{ "S2_TX_DRV",                    NULL,              "BT_PLAY_DRV" }, \
	{ "S2_TX_OUTPUT",                 NULL,              "S2_TX_DRV" }, \

#define SINGLE_BT_CAPTURE_ROUTE \
	{ "S2_IL_PGA",                    NULL,              "S2_RX_INPUT" }, \
	{ "S2_IR_PGA",                    NULL,              "S2_RX_INPUT" }, \
	{ "BT_I2S2_NB_PGA",               NULL,              "S2_IL_PGA" }, \
	{ "BT_I2S2_NB_PGA",               NULL,              "S2_IR_PGA" }, \
	{ "Bt Uplink",                    "Switch",          "BT_I2S2_NB_PGA" }, \
	{ "BT_CAPTURE_DRV",               NULL,              "Bt Uplink" }, \
	{ "U5_DRV",                       NULL,              "BT_CAPTURE_DRV" }, \
	{ "U6_DRV",                       NULL,              "BT_CAPTURE_DRV" }, \
	{ "U5_OUTPUT",                    NULL,              "U5_DRV" }, \
	{ "U6_OUTPUT",                    NULL,              "U6_DRV" }, \

#define SINGLE_BT_PLAYBACK_ROUTE_WB \
	{ "BT_I2S2_WB_PGA",               NULL,              "AUDIO_PLAY_DRV" }, \
	{ "Bt Playback Wb",               "Switch",          "BT_I2S2_WB_PGA" }, \
	{ "BT_PLAY_PGA",                  NULL,              "Bt Playback Wb" }, \

#define SINGLE_BT_CAPTURE_ROUTE_WB \
	{ "BT_I2S2_WB_PGA",               NULL,              "S2_IL_PGA" }, \
	{ "BT_I2S2_WB_PGA",               NULL,              "S2_IR_PGA" }, \
	{ "Bt Uplink Wb",                 "Switch",          "BT_I2S2_WB_PGA" }, \
	{ "BT_CAPTURE_DRV",               NULL,              "Bt Uplink Wb" }, \

#define SINGLE_REAR_HEAD_MIC_ROUTE \
	{ "MAINMIC_INPUT",                NULL,              "MICBIAS1_MIC" }, \
	{ "AUXMIC_INPUT",                 NULL,              "MICBIAS1_MIC" }, \
	{ "HSMIC_PGA",                    NULL,              "MAINMIC_INPUT" }, \
	{ "AUXMIC_PGA",                   NULL,              "AUXMIC_INPUT" }, \
	{ "Rear Mic Capture",             "Switch",          "HSMIC_PGA" }, \
	{ "Rear Mic Capture",             "Switch",          "AUXMIC_PGA" }, \
	{ "AUDIO_CAPTURE_PGA",            NULL,              "Rear Mic Capture" }, \

#define SINGLE_REAR_LINE_IN_ROUTE\
	{ "LineIn Capture",               "Switch",          "HSMIC_PGA" }, \
	{ "LineIn Capture",               "Switch",          "AUXMIC_PGA" }, \
	{ "AUDIO_CAPTURE_PGA",            NULL,              "LineIn Capture" }, \

#define SINGLE_REAR_PLAY_HP_ROUTE \
	{ "Rear Headphone Playback",      "Switch",          "AUDIO_PLAY_DRV" }, \
	{ "HP_L_DRV",                     NULL,              "Rear Headphone Playback" }, \
	{ "HP_R_DRV",                     NULL,              "Rear Headphone Playback" }, \

static const struct snd_soc_dapm_route single_route_map[] = {
	SINGLE_DP_SUPPLY_ROUTE
	SINGLE_PLL_SUPPLY_ROUTE
	SINGLE_I2S_SUPPLY_ROUTE
	SINGLE_CP1_SUPPLY_ROUTE
	SINGLE_CP2_SUPPLY_ROUTE
	SINGLE_PLAY_SPK_ROUTE
	SINGLE_PLAY_HP_ROUTE
	SINGLE_HEADSET_MIC_CAPTURE_ROUTE
	SINGLE_MIC_CAPTURE_ROUTE
	SINGLE_SOURCE_SELECT_ROUTE
	SINGLE_BT_LOOP_ROUTE
	SINGLE_BT_PLAYBACK_ROUTE
	SINGLE_BT_CAPTURE_ROUTE
	SINGLE_BT_PLAYBACK_ROUTE_WB
	SINGLE_BT_CAPTURE_ROUTE_WB
	SINGLE_EC_ROUTE
	SINGLE_CAPTURE_ROUTE
};

static const struct snd_soc_dapm_route single_route_map_rear_jack[] = {
	SINGLE_REAR_LINE_IN_ROUTE
	SINGLE_REAR_HEAD_MIC_ROUTE
	SINGLE_REAR_PLAY_HP_ROUTE
};

int da_combine_v5_add_single_switch_route(struct snd_soc_component *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;
	struct device_node *node = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}
	node = codec->dev->of_node;
	dapm = snd_soc_component_get_dapm(codec);

	if (of_property_read_bool(node, "single_kcontrol_route_rear_jack"))
		snd_soc_dapm_add_routes(dapm, single_route_map_rear_jack, ARRAY_SIZE(single_route_map_rear_jack));

	return snd_soc_dapm_add_routes(dapm, single_route_map, ARRAY_SIZE(single_route_map));
}

