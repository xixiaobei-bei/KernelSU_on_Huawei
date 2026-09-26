/*
 * route.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "route.h"

#include <sound/core.h>

#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_v5.h"

#define DP_SUPPLY_ROUTE \
	{ "U1_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U2_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U3_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U4_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U5_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U6_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U8_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U9_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "U10_OUTPUT",                   NULL,              "DP_CLK_SUPPLY" }, \
	{ "MAD_OUTPUT",                   NULL,              "DP_CLK_SUPPLY" }, \
	{ "VIRTUAL_BTN_ACTIVE_OUTPUT",    NULL,              "DP_CLK_SUPPLY" }, \
	{ "VIRTUAL_BTN_PASSIVE_OUTPUT",   NULL,              "DP_CLK_SUPPLY" }, \
	{ "IR_STUDY_OUTPUT",              NULL,              "DP_CLK_SUPPLY" }, \
	{ "D1_D2_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "D3_D4_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "D5_D6_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "S1_RX_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "S2_RX_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "S4_RX_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "IR_TX_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "S1_TX_OUTPUT",                 NULL,              "DP_CLK_SUPPLY" }, \
	{ "S2_TX_OUTPUT",                 NULL,              "DP_CLK_SUPPLY" }, \
	{ "S4_TX_OUTPUT",                 NULL,              "DP_CLK_SUPPLY" }, \
	{ "I2S2_BLUETOOTH_LOOP_SWITCH",   NULL,              "DP_CLK_SUPPLY" }, \
	{ "TDM_AUDIO_PA_DOWN_SWITCH",     NULL,              "DP_CLK_SUPPLY" }, \
	{ "TDM_AUDIO_PA_DN_LOOP_SWITCH",  NULL,              "DP_CLK_SUPPLY" }, \
	{ "FM_SWITCH",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "FM_HIFI_SWITCH",               NULL,              "DP_CLK_SUPPLY" }, \
	{ "EP_OUTPUT",                    NULL,              "DP_CLK_SUPPLY" }, \
	{ "HP_L_OUTPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "HP_R_OUTPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "DSD_L_OUTPUT",                 NULL,              "DP_CLK_SUPPLY" }, \
	{ "DSD_R_OUTPUT",                 NULL,              "DP_CLK_SUPPLY" }, \
	{ "EP_ULTRASONIC_DOWN_INPUT",     NULL,              "DP_CLK_SUPPLY" }, \
	{ "AUXMIC_ULTRASONIC_UP_OUTPUT",  NULL,              "DP_CLK_SUPPLY" }, \
	{ "U7_EC_INPUT",                  NULL,              "DP_CLK_SUPPLY" }, \
	{ "ULTRA_UP9_SWITCH",             NULL,              "DP_CLK_SUPPLY" }, \

#define PLL_SUPPLY_ROUTE \
	{ "44K1_CLK_SUPPLY",              NULL,              "PLL_CLK_SUPPLY" }, \
	{ "PLAY48K_SWITCH",               NULL,              "PLL_CLK_SUPPLY" }, \
	{ "PLAY96K_SWITCH",               NULL,              "PLL_CLK_SUPPLY" }, \
	{ "PLAY192K_SWITCH",              NULL,              "PLL_CLK_SUPPLY" }, \
	{ "PLAY384K_SWITCH",              NULL,              "PLL_CLK_SUPPLY" }, \
	{ "D1_D2_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "D3_D4_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "D5_D6_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "S1_RX_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "S2_RX_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "S4_RX_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "S1_TX_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY" }, \
	{ "IR_TX_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "S2_TX_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY" }, \
	{ "S4_TX_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY" }, \
	{ "SOUNDTRIGGER_ONEMIC_SWITCH",   NULL,              "PLL_CLK_SUPPLY" }, \
	{ "SOUNDTRIGGER_DUALMIC_SWITCH",  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "VOICE8K_SWITCH",               NULL,              "PLL_CLK_SUPPLY" }, \
	{ "VOICE16K_SWITCH",              NULL,              "PLL_CLK_SUPPLY" }, \
	{ "VOICE32K_SWITCH",              NULL,              "PLL_CLK_SUPPLY" }, \
	{ "VOICE8K_V2_SWITCH",            NULL,              "PLL_CLK_SUPPLY" }, \
	{ "VOICE16K_V2_SWITCH",           NULL,              "PLL_CLK_SUPPLY" }, \
	{ "VOICE32K_V2_SWITCH",           NULL,              "PLL_CLK_SUPPLY" }, \
	{ "VOICE8K_4PA_SWITCH",           NULL,              "PLL_CLK_SUPPLY" }, \
	{ "VOICE16K_4PA_SWITCH",          NULL,              "PLL_CLK_SUPPLY" }, \
	{ "VOICE32K_4PA_SWITCH",          NULL,              "PLL_CLK_SUPPLY" }, \
	{ "AUDIOUP_2MIC_SWITCH",          NULL,              "PLL_CLK_SUPPLY" }, \
	{ "AUDIOUP_4MIC_SWITCH",          NULL,              "PLL_CLK_SUPPLY" }, \
	{ "AUDIOUP_4MIC_V2_SWITCH",       NULL,              "PLL_CLK_SUPPLY" }, \
	{ "AUDIOUP_5MIC_SWITCH",          NULL,              "PLL_CLK_SUPPLY" }, \
	{ "ULTRA_UP9_SWITCH",             NULL,              "PLL_CLK_SUPPLY" }, \
	{ "HSMIC_PGA",                    NULL,              "PLL_CLK_SUPPLY" }, \
	{ "AUXMIC_PGA",                   NULL,              "PLL_CLK_SUPPLY" }, \
	{ "MIC3_PGA",                     NULL,              "PLL_CLK_SUPPLY" }, \
	{ "MIC4_PGA",                     NULL,              "PLL_CLK_SUPPLY" }, \
	{ "MIC4_PGA",                     NULL,              "PLL_CLK_SUPPLY" }, \
	{ "I2S2_BLUETOOTH_LOOP_SWITCH",   NULL,              "PLL_CLK_SUPPLY" }, \
	{ "TDM_AUDIO_PA_DOWN_SWITCH",     NULL,              "PLL_CLK_SUPPLY" }, \
	{ "TDM_AUDIO_PA_DN_LOOP_SWITCH",  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "FM_SWITCH",                    NULL,              "PLL_CLK_SUPPLY" }, \
	{ "FM_HIFI_SWITCH",               NULL,              "PLL_CLK_SUPPLY" }, \
	{ "EP_OUTPUT",                    NULL,              "PLL_CLK_SUPPLY" }, \
	{ "HP_L_OUTPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "HP_R_OUTPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "DSD_L_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY" }, \
	{ "DSD_R_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY" }, \
	{ "EP_ULTRASONIC_DOWN_INPUT",     NULL,              "PLL_CLK_SUPPLY" }, \
	{ "AUXMIC_ULTRASONIC_UP_OUTPUT",  NULL,              "PLL_CLK_SUPPLY" }, \
	{ "U7_EC_INPUT",                  NULL,              "PLL_CLK_SUPPLY" }, \

#define MAD_SUPPLY_ROUTE \
	{ "MAD_PGA",                      NULL,              "MAD_CLK_SUPPLY" }, \

#define PLAY_44K1_SUPPLY_ROUTE \
	{ "PLAY44K1_SWITCH",              NULL,              "44K1_CLK_SUPPLY" }, \
	{ "PLAY88K2_SWITCH",              NULL,              "44K1_CLK_SUPPLY" }, \
	{ "PLAY176K4_SWITCH",             NULL,              "44K1_CLK_SUPPLY" }, \
	{ "PLAY44K1_GAIN_SWITCH",         NULL,              "44K1_CLK_SUPPLY" }, \


#define I2S_SUPPLY_ROUTE \
	{ "S1_RX_INPUT",                  NULL,              "S1_IF_CLK_SUPPLY" }, \
	{ "S2_RX_INPUT",                  NULL,              "S2_IF_CLK_SUPPLY" }, \
	{ "S4_RX_INPUT",                  NULL,              "S4_IF_CLK_SUPPLY" }, \
	{ "S1_TX_OUTPUT",                 NULL,              "S1_IF_CLK_SUPPLY" }, \
	{ "S2_TX_OUTPUT",                 NULL,              "S2_IF_CLK_SUPPLY" }, \
	{ "S4_TX_OUTPUT",                 NULL,              "S4_IF_CLK_SUPPLY" }, \
	{ "S2_RX_INPUT",                  NULL,              "S2_RX_SUPPLY" }, \
	{ "S4_RX_INPUT",                  NULL,              "S4_RX_SUPPLY" }, \

#define CP1_SUPPLY_ROUTE \
	{ "CP2_SUPPLY",                   NULL,              "CP1_SUPPLY" }, \
	{ "EP_OUTPUT",                    NULL,              "CP1_SUPPLY" }, \

#define CP2_SUPPLY_ROUTE \
	{ "HP_L_OUTPUT",                  NULL,              "CP2_SUPPLY" }, \
	{ "HP_R_OUTPUT",                  NULL,              "CP2_SUPPLY" }, \

#define PLL_TEST_ROUTE \
	{ "PLL48K_TEST_SWITCH",           "ENABLE",          "PLL_TEST_INPUT" }, \
	{ "PLL44K1_TEST_SWITCH",          "ENABLE",          "PLL_TEST_INPUT" }, \
	{ "PLLMAD_TEST_SWITCH",           "ENABLE",          "PLL_TEST_INPUT" }, \
	{ "PLL_TEST_OUTPUT",              NULL,              "PLL48K_TEST_SWITCH" }, \
	{ "PLL_TEST_OUTPUT",              NULL,              "PLL44K1_TEST_SWITCH" }, \
	{ "PLL_TEST_OUTPUT",              NULL,              "PLLMAD_TEST_SWITCH" }, \

#define VOICE_SWITCH_ROUTE \
	{ "VOICE_OUTPUT",                 NULL,              "VOICE32K_SWITCH" }, \
	{ "VOICE_OUTPUT",                 NULL,              "VOICE32K_V2_SWITCH" }, \
	{ "VOICE32K_SWITCH",              "ENABLE",          "VOICE_INPUT" }, \
	{ "VOICE32K_V2_SWITCH",           "ENABLE",          "VOICE_INPUT" }, \
	{ "VOICE_OUTPUT",                 NULL,              "VOICE16K_SWITCH" }, \
	{ "VOICE_OUTPUT",                 NULL,              "VOICE16K_V2_SWITCH" }, \
	{ "VOICE16K_SWITCH",              "ENABLE",          "VOICE_INPUT" }, \
	{ "VOICE16K_V2_SWITCH",           "ENABLE",          "VOICE_INPUT" }, \
	{ "VOICE_OUTPUT",                 NULL,              "VOICE8K_SWITCH" }, \
	{ "VOICE_OUTPUT",                 NULL,              "VOICE8K_V2_SWITCH" }, \
	{ "VOICE8K_SWITCH",               "ENABLE",          "VOICE_INPUT" }, \
	{ "VOICE8K_V2_SWITCH",            "ENABLE",          "VOICE_INPUT" }, \

#define VOICE_4PA_SWITCH_ROUTE \
	{ "VOICE_OUTPUT",                 NULL,              "VOICE8K_4PA_SWITCH" }, \
	{ "VOICE8K_4PA_SWITCH",           "ENABLE",          "VOICE_INPUT" }, \
	{ "VOICE_OUTPUT",                 NULL,              "VOICE16K_4PA_SWITCH" }, \
	{ "VOICE16K_4PA_SWITCH",          "ENABLE",          "VOICE_INPUT" }, \
	{ "VOICE_OUTPUT",                 NULL,              "VOICE32K_4PA_SWITCH" }, \
	{ "VOICE32K_4PA_SWITCH",          "ENABLE",          "VOICE_INPUT" }, \

#define MOVEUP_PA_TDM_SWITCH_ROUTE \
	{ "MOVEUP_2PA_TDM_IV_SWITCH",     "ENABLE",          "S4_RX_INPUT" }, \
	{ "U7_DRV",                       NULL,              "MOVEUP_2PA_TDM_IV_SWITCH" }, \
	{ "U8_DRV",                       NULL,              "MOVEUP_2PA_TDM_IV_SWITCH" }, \
	{ "MOVEUP_4PA_TDM_IV_SWITCH",     "ENABLE",          "S4_RX_INPUT" }, \
	{ "U5_DRV",                       NULL,              "MOVEUP_4PA_TDM_IV_SWITCH" }, \
	{ "U6_DRV",                       NULL,              "MOVEUP_4PA_TDM_IV_SWITCH" }, \
	{ "U7_DRV",                       NULL,              "MOVEUP_4PA_TDM_IV_SWITCH" }, \
	{ "U8_DRV",                       NULL,              "MOVEUP_4PA_TDM_IV_SWITCH" }, \
	{ "TDM_AUDIO_PA_DOWN_SWITCH",     "ENABLE",          "TDM_AUDIO_PA_DOWN_INPUT" }, \
	{ "TDM_AUDIO_PA_DOWN_OUTPUT",     NULL,              "TDM_AUDIO_PA_DOWN_SWITCH" }, \

#define SOUNDTRIGGER_SWITCH_ROUTE \
	{ "VOICE_OUTPUT",                 NULL,              "SOUNDTRIGGER_ONEMIC_SWITCH" }, \
	{ "VOICE_OUTPUT",                 NULL,              "SOUNDTRIGGER_DUALMIC_SWITCH" }, \
	{ "SOUNDTRIGGER_ONEMIC_SWITCH",   "ENABLE",          "VOICE_INPUT" }, \
	{ "SOUNDTRIGGER_DUALMIC_SWITCH",  "ENABLE",          "VOICE_INPUT" }, \

#define ULTRASONIC_DOWN_ROUTE \
	{ "S3L_MUX",                      "ULTRASONIC",      "EP_ULTRASONIC_DOWN_INPUT" }, \

#define ULTRASONIC_UP_ROUTE \
	{ "AUXMIC_ULTRASONIC_UP_OUTPUT",  NULL,              "ULTRASONIC_UP_SWITCH" }, \

#define AUDIOUP_SWITCH_ROUTE \
	{ "AUDIOUP_2MIC_SWITCH",          "ENABLE",          "AUDIOUP_INPUT" }, \
	{ "AUDIOUP_4MIC_SWITCH",          "ENABLE",          "AUDIOUP_INPUT" }, \
	{ "AUDIOUP_4MIC_V2_SWITCH",       "ENABLE",          "AUDIOUP_INPUT" }, \
	{ "AUDIOUP_5MIC_SWITCH",          "ENABLE",          "AUDIOUP_INPUT" }, \
	{ "ULTRA_UP9_SWITCH",             "ENABLE",          "AUDIOUP_INPUT" }, \
	{ "AUDIOUP_OUTPUT",               NULL,              "AUDIOUP_2MIC_SWITCH" }, \
	{ "AUDIOUP_OUTPUT",               NULL,              "AUDIOUP_4MIC_SWITCH" }, \
	{ "AUDIOUP_OUTPUT",               NULL,              "AUDIOUP_4MIC_V2_SWITCH" }, \
	{ "AUDIOUP_OUTPUT",               NULL,              "AUDIOUP_5MIC_SWITCH" }, \
	{ "AUDIOUP_OUTPUT",               NULL,              "ULTRA_UP9_SWITCH" }, \

#define HP_HIGHLEVEL_SWITCH_ROUTE \
	{ "HP_HIGHLEVEL_SWITCH",          "ENABLE",          "HP_HIGHLEVEL_INPUT" }, \
	{ "HP_HIGHLEVEL_OUTPUT",          NULL,              "HP_HIGHLEVEL_SWITCH" }, \

#define HP_CONCURRENCY_SWITCH_ROUTE \
	{"HP_CONCURRENCY_SWITCH",        "ENABLE",          "HP_CONCURRENCY_INPUT"}, \
	{"HP_CONCURRENCY_OUTPUT",        NULL,              "HP_CONCURRENCY_SWITCH"}, \

#define TX_OUTPUT_ROUTE \
	{ "U1_DRV",                       NULL,              "S1_OL_SWITCH" }, \
	{ "U2_DRV",                       NULL,              "S1_OR_SWITCH" }, \
	{ "U3_DRV",                       NULL,              "U3_OL_SWITCH" }, \
	{ "U4_DRV",                       NULL,              "U4_OR_SWITCH" }, \
	{ "U5_DRV",                       NULL,              "S3_OL_SWITCH" }, \
	{ "U6_DRV",                       NULL,              "S3_OR_SWITCH" }, \
	{ "U8_DRV",                       NULL,              "US_R1_SWITCH" }, \
	{ "U9_DRV",                       NULL,              "US_R2_SWITCH" }, \
	{ "U9_DRV",                       NULL,              "ULTRA_UP9_SWITCH" }, \
	{ "U10_DRV",                      NULL,              "U5_OL_SWITCH" }, \
	{ "IR_STUDY_OUTPUT",              NULL,              "IR_STUDY_ENV_SWITCH" }, \
	{ "U1_OUTPUT",                    NULL,              "U1_DRV" }, \
	{ "U2_OUTPUT",                    NULL,              "U2_DRV" }, \
	{ "U3_OUTPUT",                    NULL,              "U3_DRV" }, \
	{ "U4_OUTPUT",                    NULL,              "U4_DRV" }, \
	{ "U5_OUTPUT",                    NULL,              "U5_DRV" }, \
	{ "U6_OUTPUT",                    NULL,              "U6_DRV" }, \
	{ "U7_OUTPUT",                    NULL,              "U7_DRV" }, \
	{ "U8_OUTPUT",                    NULL,              "U8_DRV" }, \
	{ "U9_OUTPUT",                    NULL,              "U9_DRV" }, \
	{ "U10_OUTPUT",                   NULL,              "U10_DRV" }, \
	{ "MAD_OUTPUT",                   NULL,              "MAD_SWITCH" }, \
	{ "VIRTUAL_BTN_ACTIVE_OUTPUT",    NULL,              "VIRTUAL_BTN_ACTIVE_SWITCH" }, \
	{ "VIRTUAL_BTN_PASSIVE_OUTPUT",   NULL,              "VIRTUAL_BTN_PASSIVE_SWITCH" }, \

#define RX_OUTPUT_ROUTE \
	{ "HP_L_DRV",                     NULL,              "HP_DAC_L_MUX" }, \
	{ "HP_R_DRV",                     NULL,              "HP_DAC_R_MUX" }, \
	{ "EP_DRV",                       NULL,              "EP_DAC_MUX" }, \
	{ "IR_DRV",                       NULL,              "IR_EMISSION_SWITCH" }, \
	{ "S2_TX_DRV",                    NULL,              "S2_OL_SWITCH" }, \
	{ "S2_TX_DRV",                    NULL,              "S2_OR_SWITCH" }, \
	{ "S4_TX_DRV",                    NULL,              "S4_OL_SWITCH" }, \
	{ "S4_TX_DRV",                    NULL,              "S4_OR_SWITCH" }, \
	{ "HP_L_OUTPUT",                  NULL,              "HP_L_DRV" }, \
	{ "HP_R_OUTPUT",                  NULL,              "HP_R_DRV" }, \
	{ "EP_OUTPUT",                    NULL,              "EP_DRV" }, \
	{ "IR_LED_OUTPUT",                NULL,              "IR_DRV" }, \
	{ "S2_TX_OUTPUT",                 NULL,              "S2_TX_DRV" }, \
	{ "S4_TX_OUTPUT",                 NULL,              "S4_TX_DRV" }, \

#define AUDIODOWN_SWITCH_ROUTE \
	{ "PLAY44K1_SWITCH",              "ENABLE",          "AUDIO_DOWN_INPUT" }, \
	{ "PLAY44K1_GAIN_SWITCH",         "ENABLE",          "AUDIO_DOWN_INPUT" }, \
	{ "PLAY48K_SWITCH",               "ENABLE",          "AUDIO_DOWN_INPUT" }, \
	{ "PLAY96K_SWITCH",               "ENABLE",          "AUDIO_DOWN_INPUT" }, \
	{ "PLAY192K_SWITCH",              "ENABLE",          "AUDIO_DOWN_INPUT" }, \
	{ "PLAY384K_SWITCH",              "ENABLE",          "AUDIO_DOWN_INPUT" }, \
	{ "PLAY88K2_SWITCH",              "ENABLE",          "AUDIO_DOWN_INPUT" }, \
	{ "PLAY176K4_SWITCH",             "ENABLE",          "AUDIO_DOWN_INPUT" }, \
	{ "AUDIO_DOWN_OUTPUT",            NULL,              "PLAY44K1_SWITCH" }, \
	{ "AUDIO_DOWN_OUTPUT",            NULL,              "PLAY44K1_GAIN_SWITCH" }, \
	{ "AUDIO_DOWN_OUTPUT",            NULL,              "PLAY48K_SWITCH" }, \
	{ "AUDIO_DOWN_OUTPUT",            NULL,              "PLAY96K_SWITCH" }, \
	{ "AUDIO_DOWN_OUTPUT",            NULL,              "PLAY192K_SWITCH" }, \
	{ "AUDIO_DOWN_OUTPUT",            NULL,              "PLAY384K_SWITCH" }, \
	{ "AUDIO_DOWN_OUTPUT",            NULL,              "PLAY88K2_SWITCH" }, \
	{ "AUDIO_DOWN_OUTPUT",            NULL,              "PLAY176K4_SWITCH" }, \
	{ "PLAY384K_VIR_SWITCH",          "ENABLE",          "S1_IL_PGA" }, \
	{ "PLAY384K_VIR_SWITCH",          "ENABLE",          "S1_IR_PGA" }, \

#define FM_SWITCH_ROUTE \
	{ "FM_SWITCH",                    "ENABLE",          "FM_INPUT" }, \
	{ "FM_OUTPUT",                    NULL,              "FM_SWITCH" }, \
	{ "FM_HIFI_SWITCH",               "ENABLE",          "FM_INPUT" }, \
	{ "FM_OUTPUT",                    NULL,              "FM_HIFI_SWITCH" }, \

#define MICBIAS_ROUTE \
	{ "MAINMIC_INPUT",                NULL,              "MICBIAS1_MIC" }, \
	{ "AUXMIC_HSMICBIAS_SWITCH",      "ENABLE",          "HSMICBIAS" }, \
	{ "AUXMIC_MICBIAS2_SWITCH",       "ENABLE",          "MICBIAS2_MIC" }, \
	{ "AUXMIC_MICBIAS1_SWITCH",       "ENABLE",          "MICBIAS1_MIC" }, \
	{ "AUXMIC_INPUT",                 NULL,              "AUXMIC_HSMICBIAS_SWITCH" }, \
	{ "AUXMIC_INPUT",                 NULL,              "AUXMIC_MICBIAS2_SWITCH" }, \
	{ "AUXMIC_INPUT",                 NULL,              "AUXMIC_MICBIAS1_SWITCH" }, \
	{ "MIC3_INPUT",                   NULL,              "MICBIAS2_MIC" }, \
	{ "MIC4_INPUT",                   NULL,              "MICBIAS2_MIC" }, \
	{ "MIC5_INPUT",                   NULL,              "MICBIAS2_MIC" }, \
	{ "MIC4_MICBIAS1_SWITCH",         "ENABLE",          "MICBIAS1_MIC" }, \
	{ "MIC4_INPUT",                   NULL,              "MIC4_MICBIAS1_SWITCH" }, \
	{ "MIC5_MICBIAS1_SWITCH",         "ENABLE",          "MICBIAS1_MIC" }, \
	{ "MIC5_INPUT",                   NULL,              "MIC5_MICBIAS1_SWITCH" }, \
	{ "HSMIC_INPUT",                  NULL,              "HSMICBIAS" }, \

#define MICPGA_ROUTE \
	{ "HSMIC_PGA_MUX",                "MAINMIC",         "MAINMIC_INPUT" }, \
	{ "HSMIC_PGA_MUX",                "HSMIC",           "HSMIC_INPUT" }, \
	{ "AUXMIC_PGA_MUX",               "MAINMIC",         "MAINMIC_INPUT" }, \
	{ "AUXMIC_PGA_MUX",               "AUXMIC",          "AUXMIC_INPUT" }, \
	{ "MIC3_PGA_MUX",                 "MIC3",            "MIC3_INPUT" }, \
	{ "MIC3_PGA_MUX",                 "MIC4",            "MIC4_INPUT" }, \
	{ "MIC4_PGA_MUX",                 "MIC3",            "MIC3_INPUT" }, \
	{ "MIC4_PGA_MUX",                 "MIC4",            "MIC4_INPUT" }, \
	{ "MIC5_PGA_MUX",                 "MIC5",            "MIC5_INPUT" }, \
	{ "MIC5_PGA_MUX",                 "IR",              "IR_STUDY_INPUT" }, \
	{ "MAD_PGA_MIXER",                "MIC5_ENABLE",     "MIC5_INPUT" }, \
	{ "MAD_PGA_MIXER",                "MIC4_ENABLE",     "MIC4_INPUT" }, \
	{ "MAD_PGA_MIXER",                "MIC3_ENABLE",     "MIC3_INPUT" }, \
	{ "MAD_PGA_MIXER",                "AUXMIC_ENABLE",   "AUXMIC_INPUT" }, \
	{ "MAD_PGA_MIXER",                "MAINMIC_ENABLE",  "MAINMIC_INPUT" }, \
	{ "MAD_PGA_MIXER",                "HSMIC_ENABLE",    "HSMIC_INPUT" }, \
	{ "MAD_PGA",                      NULL,              "MAD_PGA_MIXER" }, \
	{ "HSMIC_PGA",                    NULL,              "HSMIC_PGA_MUX" }, \
	{ "AUXMIC_PGA",                   NULL,              "AUXMIC_PGA_MUX" }, \
	{ "MIC3_PGA",                     NULL,              "MIC3_PGA_MUX" }, \
	{ "MIC4_PGA",                     NULL,              "MIC4_PGA_MUX" }, \
	{ "MIC5_PGA",                     NULL,              "MIC5_PGA_MUX" }, \

#define ADC_ROUTE \
	{ "ADCL0",                        NULL,              "ADCL0_MUX" }, \
	{ "ADCR0",                        NULL,              "ADCR0_MUX" }, \
	{ "ADCL1",                        NULL,              "ADCL1_MUX" }, \
	{ "ADCR1",                        NULL,              "ADCR1_MUX" }, \
	{ "ADCL2",                        NULL,              "ADCL2_MUX" }, \

#define ADC_MUX_ROUTE \
	{ "ADCL0_MUX",                    "ADC_MAD",         "MAD_PGA" }, \
	{ "ADCL0_MUX",                    "ADC0",            "HSMIC_PGA" }, \
	{ "ADCL0_MUX",                    "ADC1",            "AUXMIC_PGA" }, \
	{ "ADCL0_MUX",                    "ADC2",            "MIC3_PGA" }, \
	{ "ADCL0_MUX",                    "ADC3",            "MIC4_PGA" }, \
	{ "ADCL0_MUX",                    "ADC4",            "MIC5_PGA" }, \
	{ "ADCL1_MUX",                    "ADC_MAD",         "MAD_PGA" }, \
	{ "ADCL1_MUX",                    "ADC0",            "HSMIC_PGA" }, \
	{ "ADCL1_MUX",                    "ADC1",            "AUXMIC_PGA" }, \
	{ "ADCL1_MUX",                    "ADC2",            "MIC3_PGA" }, \
	{ "ADCL1_MUX",                    "ADC3",            "MIC4_PGA" }, \
	{ "ADCL1_MUX",                    "ADC4",            "MIC5_PGA" }, \
	{ "ADCR0_MUX",                    "ADC_MAD",         "MAD_PGA" }, \
	{ "ADCR0_MUX",                    "ADC0",            "HSMIC_PGA" }, \
	{ "ADCR0_MUX",                    "ADC1",            "AUXMIC_PGA" }, \
	{ "ADCR0_MUX",                    "ADC2",            "MIC3_PGA" }, \
	{ "ADCR0_MUX",                    "ADC3",            "MIC4_PGA" }, \
	{ "ADCR0_MUX",                    "ADC4",            "MIC5_PGA" }, \
	{ "ADCR1_MUX",                    "ADC_MAD",         "MAD_PGA" }, \
	{ "ADCR1_MUX",                    "ADC0",            "HSMIC_PGA" }, \
	{ "ADCR1_MUX",                    "ADC1",            "AUXMIC_PGA" }, \
	{ "ADCR1_MUX",                    "ADC2",            "MIC3_PGA" }, \
	{ "ADCR1_MUX",                    "ADC3",            "MIC4_PGA" }, \
	{ "ADCR1_MUX",                    "ADC4",            "MIC5_PGA" }, \
	{ "ADCL2_MUX",                    "ADC_MAD",         "MAD_PGA" }, \
	{ "ADCL2_MUX",                    "ADC0",            "HSMIC_PGA" }, \
	{ "ADCL2_MUX",                    "ADC1",            "AUXMIC_PGA" }, \
	{ "ADCL2_MUX",                    "ADC2",            "MIC3_PGA" }, \
	{ "ADCL2_MUX",                    "ADC3",            "MIC4_PGA" }, \
	{ "ADCL2_MUX",                    "ADC4",            "MIC5_PGA" }, \

#define TX_COMMON_MUX_ROUTE \
	{ "MIC1_MUX",                     "ADCL0",           "ADCL0" }, \
	{ "MIC1_MUX",                     "ADCL1",           "ADCL1" }, \
	{ "MIC1_MUX",                     "ADCR0",           "ADCR0" }, \
	{ "MIC1_MUX",                     "ADCR1",           "ADCR1" }, \
	{ "MIC1_MUX",                     "S2_L",            "S2_IL_PGA" }, \
	{ "MIC1_MUX",                     "S3_L",            "S3_IL_PGA" }, \
	{ "MIC1_MUX",                     "S4_L",            "S4_IL_PGA" }, \
	{ "MIC1_MUX",                     "DACL_48",         "DACL_PRE_MIXER" }, \
	{ "MIC1_MUX",                     "ADCL2",           "ADCL2" }, \
	{ "MIC2_MUX",                     "ADCL0",           "ADCL0" }, \
	{ "MIC2_MUX",                     "ADCL1",           "ADCL1" }, \
	{ "MIC2_MUX",                     "ADCR0",           "ADCR0" }, \
	{ "MIC2_MUX",                     "ADCR1",           "ADCR1" }, \
	{ "MIC2_MUX",                     "S2_R",            "S2_IR_PGA" }, \
	{ "MIC2_MUX",                     "S3_R",            "S3_IR_PGA" }, \
	{ "MIC2_MUX",                     "S4_R",            "S4_IR_PGA" }, \
	{ "MIC2_MUX",                     "DACR_48",         "DACR_PRE_MIXER" }, \
	{ "MIC2_MUX",                     "ADCL2",           "ADCL2" }, \
	{ "MIC3_MUX",                     "ADCL0",           "ADCL0" }, \
	{ "MIC3_MUX",                     "ADCL1",           "ADCL1" }, \
	{ "MIC3_MUX",                     "ADCR0",           "ADCR0" }, \
	{ "MIC3_MUX",                     "ADCR1",           "ADCR1" }, \
	{ "MIC3_MUX",                     "S1_L",            "S1_IL_PGA" }, \
	{ "MIC3_MUX",                     "S2_L",            "S2_IL_PGA" }, \
	{ "MIC3_MUX",                     "S4_L",            "S4_IL_PGA" }, \
	{ "MIC3_MUX",                     "DACL_48",         "DACL_PRE_MIXER" }, \
	{ "MIC3_MUX",                     "ADCL2",           "ADCL2" }, \
	{ "MIC4_MUX",                     "ADCL0",           "ADCL0" }, \
	{ "MIC4_MUX",                     "ADCL1",           "ADCL1" }, \
	{ "MIC4_MUX",                     "ADCR0",           "ADCR0" }, \
	{ "MIC4_MUX",                     "ADCR1",           "ADCR1" }, \
	{ "MIC4_MUX",                     "S1_R",            "S1_IR_PGA" }, \
	{ "MIC4_MUX",                     "S2_R",            "S2_IR_PGA" }, \
	{ "MIC4_MUX",                     "S4_R",            "S4_IR_PGA" }, \
	{ "MIC4_MUX",                     "DACR_48",         "DACR_PRE_MIXER" }, \
	{ "MIC4_MUX",                     "ADCL2",           "ADCL2" }, \
	{ "MIC5_MUX",                     "ADCL0",           "ADCL0" }, \
	{ "MIC5_MUX",                     "ADCL1",           "ADCL1" }, \
	{ "MIC5_MUX",                     "ADCR0",           "ADCR0" }, \
	{ "MIC5_MUX",                     "ADCR1",           "ADCR1" }, \
	{ "MIC5_MUX",                     "DACL_S",          "DACSL_MIXER" }, \
	{ "MIC5_MUX",                     "DACR_S",          "DACSR_MIXER" }, \
	{ "MIC5_MUX",                     "ADCL2",           "ADCL2" }, \
	{ "SIDETONE_MUX",                 "S1_L",            "S1_IL_PGA" }, \
	{ "SIDETONE_MUX",                 "S2_L",            "S2_IL_PGA" }, \
	{ "SIDETONE_MUX",                 "ADCL0",           "ADCL0" }, \
	{ "SIDETONE_MUX",                 "ADCR0",           "ADCR0" }, \
	{ "SIDETONE_MUX",                 "ADCL1",           "ADCL1" }, \
	{ "SIDETONE_MUX",                 "ADCR1",           "ADCR1" }, \
	{ "SIDETONE_MUX",                 "ADCL2",           "ADCL2" }, \
	{ "SIDETONE_PGA",                 NULL,              "SIDETONE_MUX" }, \
	{ "M1_L_MUX",                     "ADCL0",           "ADCL0" }, \
	{ "M1_L_MUX",                     "ADCL1",           "ADCL1" }, \
	{ "M1_L_MUX",                     "DACL_S",          "DACSL_MIXER" }, \
	{ "M1_L_MUX",                     "DACL_48",         "DACL_PRE_MIXER" }, \
	{ "M1_L_MUX",                     "S1_L",            "S1_IL_PGA" }, \
	{ "M1_L_MUX",                     "S2_L",            "S2_IL_PGA" }, \
	{ "M1_L_MUX",                     "S3_L",            "S3_IL_PGA" }, \
	{ "M1_L_MUX",                     "ADCL2",           "ADCL2" }, \
	{ "M1_R_MUX",                     "ADCR0",           "ADCR0" }, \
	{ "M1_R_MUX",                     "ADCR1",           "ADCR1" }, \
	{ "M1_R_MUX",                     "DACR_S",          "DACSR_MIXER" }, \
	{ "M1_R_MUX",                     "DACR_48",         "DACR_PRE_MIXER" }, \
	{ "M1_R_MUX",                     "S1_R",            "S1_IR_PGA" }, \
	{ "M1_R_MUX",                     "S2_R",            "S2_IR_PGA" }, \
	{ "M1_R_MUX",                     "S3_R",            "S3_IR_PGA" }, \
	{ "M1_R_MUX",                     "ADCL2",           "ADCL2" }, \
	{ "BT_L_MUX",                     "ADCL0",           "ADCL0" }, \
	{ "BT_L_MUX",                     "ADCL1",           "ADCL1" }, \
	{ "BT_L_MUX",                     "DACL_48",         "DACL_PRE_MIXER" }, \
	{ "BT_L_MUX",                     "S1_L",            "S1_IL_PGA" }, \
	{ "BT_L_MUX",                     "S3_L",            "S3_IL_PGA" }, \
	{ "BT_L_MUX",                     "S4_L",            "S4_IL_PGA" }, \
	{ "BT_L_MUX",                     "ADCL2",           "ADCL2" }, \
	{ "BT_R_MUX",                     "ADCR0",           "ADCR0" }, \
	{ "BT_R_MUX",                     "ADCR1",           "ADCR1" }, \
	{ "BT_R_MUX",                     "DACR_48",         "DACR_PRE_MIXER" }, \
	{ "BT_R_MUX",                     "S1_R",            "S1_IR_PGA" }, \
	{ "BT_R_MUX",                     "S3_R",            "S3_IR_PGA" }, \
	{ "BT_R_MUX",                     "S4_R",            "S4_IR_PGA" }, \
	{ "BT_R_MUX",                     "ADCL2",           "ADCL2" }, \
	{ "US_R1_MUX",                    "ADCL0",           "ADCL0" }, \
	{ "US_R1_MUX",                    "ADCL1",           "ADCL1" }, \
	{ "US_R1_MUX",                    "ADCR0",           "ADCR0" }, \
	{ "US_R1_MUX",                    "ADCR1",           "ADCR1" }, \
	{ "US_R1_MUX",                    "ADCL2",           "ADCL2" }, \
	{ "US_R2_MUX",                    "ADCL0",           "ADCL0" }, \
	{ "US_R2_MUX",                    "ADCL1",           "ADCL1" }, \
	{ "US_R2_MUX",                    "ADCR0",           "ADCR0" }, \
	{ "US_R2_MUX",                    "ADCR1",           "ADCR1" }, \
	{ "US_R2_MUX",                    "ADCL2",           "ADCL2" }, \

#define TX_SWITCH_ROUTE \
	{ "S1_OL_SWITCH",                 "ENABLE",          "MIC1_MUX" }, \
	{ "S1_OR_SWITCH",                 "ENABLE",          "MIC2_MUX" }, \
	{ "S2_OL_SWITCH",                 "ENABLE",          "BT_L_MUX" }, \
	{ "ULTRA_UP9_SWITCH",             "ENABLE",          "BT_L_MUX" }, \
	{ "S2_OR_SWITCH",                 "ENABLE",          "BT_R_MUX" }, \
	{ "S3_OL_SWITCH",                 "ENABLE",          "MIC1_MUX" }, \
	{ "S3_OR_SWITCH",                 "ENABLE",          "MIC2_MUX" }, \
	{ "S4_OL_SWITCH",                 "ENABLE",          "M1_L_MUX" }, \
	{ "S4_OR_SWITCH",                 "ENABLE",          "M1_R_MUX" }, \
	{ "U3_OL_SWITCH",                 "ENABLE",          "MIC3_MUX" }, \
	{ "U4_OR_SWITCH",                 "ENABLE",          "MIC4_MUX" }, \
	{ "U5_OL_SWITCH",                 "ENABLE",          "MIC5_MUX" }, \
	{ "US_R1_SWITCH",                 "ENABLE",          "US_R1_MUX" }, \
	{ "US_R2_SWITCH",                 "ENABLE",          "US_R2_MUX" }, \
	{ "MAD_SWITCH",                   "ENABLE",          "MIC1_MUX"}, \
	{ "VIRTUAL_BTN_ACTIVE_SWITCH",    "ENABLE",          "ADCL2_VIRTBTN_IR" }, \
	{ "VIRTUAL_BTN_PASSIVE_SWITCH",   "ENABLE",          "ADCL2_VIRTBTN_IR" }, \
	{ "ADCL2_VIRTBTN_IR",             NULL,              "ADCL2_MUX" }, \
	{ "ULTRASONIC_UP_SWITCH",         "ENABLE",          "ADCL2" }, \
	{ "IR_STUDY_ENV_SWITCH",          "ENABLE",          "ADCL2_VIRTBTN_IR" }, \

#define IV_ROUTE \
	{ "IV_DSPIF_SWITCH",              "ENABLE",          "S4_RX_INPUT" }, \
	{ "IV_2PA_SWITCH",                "ENABLE",          "S4_RX_INPUT" }, \
	{ "IV_4PA_SWITCH",                "ENABLE",          "S4_RX_INPUT" }, \
	{ "U7_DRV",                       NULL,              "IV_2PA_SWITCH" }, \
	{ "U8_DRV",                       NULL,              "IV_2PA_SWITCH" }, \
	{ "U5_DRV",                       NULL,              "IV_4PA_SWITCH" }, \
	{ "U6_DRV",                       NULL,              "IV_4PA_SWITCH" }, \
	{ "U7_DRV",                       NULL,              "IV_4PA_SWITCH" }, \
	{ "U8_DRV",                       NULL,              "IV_4PA_SWITCH" }, \
	{ "IV_DSPIF_OUTPUT",              NULL,              "IV_DSPIF_SWITCH" }, \

#define EC_ROUTE \
	{ "U7_EC_SWITCH",                 "ENABLE",          "U7_EC_INPUT" }, \
	{ "U7_DRV",                       NULL,              "U7_EC_SWITCH" }, \
	{ "U7_EC_OUTPUT",                 NULL,              "U7_DRV" }, \

#define DAC_MIXER_ROUTE \
	{ "DACL_MIXER",                   "S1_L",            "S1_IL_PGA" }, \
	{ "DACL_MIXER",                   "S2_L",            "S2_IL_PGA" }, \
	{ "DACL_MIXER",                   "S3_L",            "S3L_MUX" }, \
	{ "DACL_MIXER",                   "S1_R",            "S1_IR_PGA" }, \
	{ "DACR_MIXER",                   "S1_R",            "S1_IR_PGA" }, \
	{ "DACR_MIXER",                   "S2_R",            "S2_IR_PGA" }, \
	{ "DACR_MIXER",                   "MDM",             "MDM_MUX" }, \
	{ "DACR_MIXER",                   "S1_L",            "S1_IL_PGA" }, \
	{ "DACL_PRE_MIXER",               "MUX9",            "DACL_PRE_MUX" }, \
	{ "DACL_PRE_MIXER",               "SIDETONE",        "SIDETONE_PGA" }, \
	{ "DACR_PRE_MIXER",               "MUX10",           "DACR_PRE_MUX" }, \
	{ "DACR_PRE_MIXER",               "SIDETONE",        "SIDETONE_PGA" }, \
	{ "DACL_POST_MIXER",              "DACLSRC",         "DACL_PRE_MIXER" }, \
	{ "DACL_POST_MIXER",              "S1_L",            "S1_IL_PGA" }, \
	{ "DACR_POST_MIXER",              "DACRSRC",         "DACR_PRE_MIXER" }, \
	{ "DACR_POST_MIXER",              "S1_R",            "S1_IR_PGA" }, \
	{ "DACSL_MIXER",                  "UTW",             "D5_D6_INPUT" }, \
	{ "DACSL_MIXER",                  "DACSL_PGA",       "DACL_PRE_MUX" }, \
	{ "DACSL_MIXER",                  "MDM",             "MDM_MUX" }, \
	{ "DACSL_MIXER",                  "SIDETONE",        "SIDETONE_PGA" }, \
	{ "DACSR_MIXER",                  "UTW",             "D5_D6_INPUT" }, \
	{ "DACSR_MIXER",                  "DACSR_PGA",       "DACR_PRE_MUX" }, \
	{ "DACSR_MIXER",                  "MDM",             "MDM_MUX" }, \
	{ "DACSR_MIXER",                  "SIDETONE",        "SIDETONE_PGA" }, \

#define DSD_MIXER_ROUTE \
	{ "DSDL_MIXER",                   "PGA",             "DSD_L_INPUT" }, \
	{ "DSDL_MIXER",                   "DACL_384K",       "DACL_POST_MIXER" }, \
	{ "DSDR_MIXER",                   "PGA",             "DSD_R_INPUT" }, \
	{ "DSDR_MIXER",                   "DACR_384K",       "DACR_POST_MIXER" }, \

#define DAC_MUX_ROUTE \
	{ "DACL_PRE_MUX",                 "DACL_MIXER",      "DACL_MIXER" }, \
	{ "DACR_PRE_MUX",                 "DACR_MIXER",      "DACR_MIXER" }, \
	{ "DACSL_PGA_MUX",                "DACSL_384K",      "DACSL_MIXER" }, \
	{ "DACL_PGA_MUX",                 "DACL_384K",       "DACL_POST_MIXER" }, \
	{ "DACL_PGA_MUX",                 "S1_L",            "PLAY384K_VIR_SWITCH" }, \
	{ "DACL_PGA_MUX",                 "ADCL1",           "ADCL1" },\
	{ "DACR_PGA_MUX",                 "DACR_384K",       "DACR_POST_MIXER" }, \
	{ "DACR_PGA_MUX",                 "S1_R",            "PLAY384K_VIR_SWITCH" }, \
	{ "DACR_PGA_MUX",                 "ADCR1",           "ADCR1" }, \

#define RX_MUX_ROUTE \
	{ "EP_L_SDM_MUX",                 "DACSL_UP16",      "DACSL_PGA_MUX" }, \
	{ "HP_L_SDM_MUX",                 "DSD_L",           "DSDL_MIXER" }, \
	{ "HP_L_SDM_MUX",                 "DACL_UP16",       "DACL_PGA_MUX" }, \
	{ "HP_R_SDM_MUX",                 "DSD_R",           "DSDR_MIXER" }, \
	{ "HP_R_SDM_MUX",                 "DACR_UP16",       "DACR_PGA_MUX" }, \
	{ "HP_DAC_L_MUX",                 "HP_L_SDM",        "HP_L_SDM_MUX" }, \
	{ "HP_DAC_L_MUX",                 "HP_R_SDM",        "HP_R_SDM_MUX" }, \
	{ "HP_DAC_R_MUX",                 "HP_L_SDM",        "HP_L_SDM_MUX" }, \
	{ "HP_DAC_R_MUX",                 "HP_R_SDM",        "HP_R_SDM_MUX" }, \
	{ "EP_DAC_MUX",                   "EP_L_SDM",        "EP_L_SDM_MUX" }, \

#define SX_INPUT_MUX_ROUTE \
	{ "S1_IL_MUX",                    "D1",              "D1_D2_INPUT" }, \
	{ "S1_IL_MUX",                    "S1_INL",          "S1_RX_INPUT" }, \
	{ "S1_IR_MUX",                    "D2",              "D1_D2_INPUT" }, \
	{ "S1_IR_MUX",                    "S1_INR",          "S1_RX_INPUT" }, \
	{ "S2_IL_MUX",                    "D3",              "D3_D4_INPUT" }, \
	{ "S2_IL_MUX",                    "S2_INL",          "S2_RX_INPUT" }, \
	{ "S2_IR_MUX",                    "D4",              "D3_D4_INPUT" }, \
	{ "S2_IR_MUX",                    "S2_INR",          "S2_RX_INPUT" }, \

#define SX_PGA_ROUTE \
	{ "S1_IL_PGA",                    NULL,              "S1_IL_MUX" }, \
	{ "S1_IR_PGA",                    NULL,              "S1_IR_MUX" }, \
	{ "S2_IL_PGA",                    NULL,              "S2_IL_MUX" }, \
	{ "S2_IR_PGA",                    NULL,              "S2_IR_MUX" }, \
	{ "S3_IL_PGA",                    NULL,              "D5_D6_INPUT" }, \
	{ "S3_IR_PGA",                    NULL,              "D5_D6_INPUT" }, \
	{ "S4_IL_PGA",                    NULL,              "S4_RX_INPUT" }, \
	{ "S4_IR_PGA",                    NULL,              "S4_RX_INPUT" }, \

#define MDM_MUX_ROUTE \
	{ "S3L_MUX",                     "S3_IL_PGA",        "S3_IL_PGA" }, \
	{ "MDM_MUX",                     "S3_L",             "S3L_MUX" }, \
	{ "MDM_MUX",                     "S3_R",             "S3_IR_PGA" }, \

#define IR_TX_MUX_ROUTE \
	{ "IR_MUX",                      "DSPIF8",           "IR_TX_INPUT" }, \
	{ "IR_MUX",                      "IR_REG_CTRL",      "IR_TX_INPUT" }, \
	{ "IR_MUX",                      "IO_TEST_IN",       "IR_TX_INPUT" }, \
	{ "IR_EMISSION_SWITCH",          "ENABLE",           "IR_MUX" }, \

/* mmi_route */
#define MMI_ROUTE \
	{ "I2S2_BLUETOOTH_LOOP_SWITCH",   "ENABLE",          "I2S2_BLUETOOTH_LOOP_INPUT" }, \
	{ "I2S2_BLUETOOTH_LOOP_OUTPUT",   NULL,              "I2S2_BLUETOOTH_LOOP_SWITCH" }, \
	{ "TDM_AUDIO_PA_DN_LOOP_SWITCH",  "ENABLE",          "TDM_AUDIO_PA_DOWN_INPUT" }, \
	{ "TDM_AUDIO_PA_DOWN_OUTPUT",     NULL,              "TDM_AUDIO_PA_DN_LOOP_SWITCH" }, \

static const struct snd_soc_dapm_route route_map[] = {
	DP_SUPPLY_ROUTE
	PLL_SUPPLY_ROUTE
	MAD_SUPPLY_ROUTE
	PLAY_44K1_SUPPLY_ROUTE
	I2S_SUPPLY_ROUTE
	CP1_SUPPLY_ROUTE
	CP2_SUPPLY_ROUTE
	PLL_TEST_ROUTE
	VOICE_SWITCH_ROUTE
	SOUNDTRIGGER_SWITCH_ROUTE
	ULTRASONIC_DOWN_ROUTE
	ULTRASONIC_UP_ROUTE
	AUDIOUP_SWITCH_ROUTE
	HP_HIGHLEVEL_SWITCH_ROUTE
	HP_CONCURRENCY_SWITCH_ROUTE
	MICBIAS_ROUTE
	MICPGA_ROUTE
	TX_COMMON_MUX_ROUTE
	ADC_ROUTE
	ADC_MUX_ROUTE
	TX_SWITCH_ROUTE
	TX_OUTPUT_ROUTE
	AUDIODOWN_SWITCH_ROUTE
	FM_SWITCH_ROUTE
	IV_ROUTE
	EC_ROUTE
	DAC_MUX_ROUTE
	DAC_MIXER_ROUTE
	DSD_MIXER_ROUTE
	RX_MUX_ROUTE
	RX_OUTPUT_ROUTE
	SX_INPUT_MUX_ROUTE
	SX_PGA_ROUTE
	MDM_MUX_ROUTE
	IR_TX_MUX_ROUTE
	MMI_ROUTE
	MOVEUP_PA_TDM_SWITCH_ROUTE
	VOICE_4PA_SWITCH_ROUTE
};

int da_combine_v5_add_routes(struct snd_soc_component *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	return snd_soc_dapm_add_routes(dapm, route_map,
		ARRAY_SIZE(route_map));
}

