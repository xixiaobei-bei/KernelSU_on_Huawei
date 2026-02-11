/*
 * hi6403_route.c
 *
 * hi6403 route map for codec driver
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#include "hi6403_route.h"

#include <sound/core.h>

#include <linux/hisi/audio_log.h>

#include "hi6403_type.h"

static const struct snd_soc_dapm_route route_map[] = {
	/* keep for pll test */
	{ "PLL OUT",             NULL,                "PLL48K SWITCH" },
	{ "PLL OUT",             NULL,                "PLL44K1 SWITCH" },
	{ "PLL OUT",             NULL,                "PLLMAD SWITCH" },
	{ "PLL48K SWITCH",       "SWITCH",            "PLL IN" },
	{ "PLL44K1 SWITCH",      "SWITCH",            "PLL IN" },
	{ "PLLMAD SWITCH",       "SWITCH",            "PLL IN" },
	/* end for pll test */

	{ "S1 IL PGA",           NULL,                "S1L IN" },
	{ "S1 IR PGA",           NULL,                "S1R IN" },
	{ "S2 IL PGA",           NULL,                "S2L IN" },
	{ "S2 IR PGA",           NULL,                "S2R IN" },
	{ "S3 IL PGA",           NULL,                "S3L IN" },
	{ "S3 IR PGA",           NULL,                "S3R IN" },


	{ "AUDIO OUT",           NULL,                "PLAY48K SWITCH" },
	{ "AUDIO OUT",           NULL,                "PLAY96K SWITCH" },
	{ "AUDIO OUT",           NULL,                "PLAY192K SWITCH" },
	{ "AUDIO OUT",           NULL,                "LOWLATENCY SWITCH" },
	{ "PLAY48K SWITCH",      "SWITCH",            "AUDIO IN" },
	{ "PLAY96K SWITCH",      "SWITCH",            "AUDIO IN" },
	{ "PLAY192K SWITCH",     "SWITCH",            "AUDIO IN" },
	{ "LOWLATENCY SWITCH",   "SWITCH",            "AUDIO IN" },

	{ "MAD OUT",             NULL,                "MAD SWITCH" },


	{ "S1L OUT",             NULL,                "U1 DRV" },
	{ "S1R OUT",             NULL,                "U2 DRV" },
	{ "U3L OUT",             NULL,                "U3 DRV" },
	{ "U4R OUT",             NULL,                "U4 DRV" },
	{ "S2L OUT",             NULL,                "S2 TX" },
	{ "S2R OUT",             NULL,                "S2 TX" },
	{ "S3L OUT",             NULL,                "U5 DRV" },
	{ "S3R OUT",             NULL,                "U6 DRV" },
	{ "S4L OUT",             NULL,                "S4 TX" },
	{ "S4R OUT",             NULL,                "S4 TX" },

	{ "ECL OUT",             NULL,                "U7 DRV" },
	{ "ECR OUT",             NULL,                "U8 DRV" },

	{ "AUDIO OUT",           NULL,                "AUDIOUP SWITCH" },
	{ "AUDIO OUT",           NULL,                "AUDIOUP 4MIC SWITCH" },
	{ "VOICE OUT",           NULL,                "VOICE8K SWITCH" },
	{ "AUDIOUP SWITCH",      "SWITCH",            "AUDIO IN" },
	{ "AUDIOUP 4MIC SWITCH", "SWITCH",            "AUDIO IN" },
	{ "VOICE8K SWITCH",      "SWITCH",	         "VOICE IN" },

	{ "VOICE OUT",           NULL,                "VOICE32K SWITCH" },
	{ "VOICE OUT",           NULL,                "VOICE16K SWITCH" },
	{ "VOICE32K SWITCH",     "SWITCH",            "VOICE IN" },
	{ "VOICE16K SWITCH",     "SWITCH",            "VOICE IN" },

	{ "HP HIGH OUT",         NULL,                "HPHIGHLEVEL SWITCH" },
	{ "HPHIGHLEVEL SWITCH",  "SWITCH",            "HP HIGH IN" },

	#ifdef CONFIG_BUCKBOOST
	{ "AUXMIC PWM OUT",               NULL,       "AUXMICPWMPOWER SWITCH" },
	{ "AUXMICPWMPOWER SWITCH",        "SWITCH",   "AUXMIC PWM IN" },
	#endif

	{ "VOICE OUT",           NULL,                "SOUNDTRIGGER ONEMIC SWITCH" },
	{ "VOICE OUT",           NULL,                "SOUNDTRIGGER DUALMIC SWITCH" },
	{ "VOICE OUT",           NULL,                "SOUNDTRIGGER MULTIMIC SWITCH" },
	{ "SOUNDTRIGGER ONEMIC SWITCH",   "SWITCH",           "VOICE IN" },
	{ "SOUNDTRIGGER DUALMIC SWITCH",  "SWITCH",           "VOICE IN" },
	{ "SOUNDTRIGGER MULTIMIC SWITCH", "SWITCH",           "VOICE IN" },

	{ "VOICE8K SWITCH",      NULL,                "PLL CLK" },
	{ "VOICE16K SWITCH",     NULL,                "PLL CLK" },
	{ "VOICE32K SWITCH",     NULL,                "PLL CLK" },
	{ "AUDIOUP 4MIC SWITCH", NULL,                "PLL CLK" },
	{ "AUDIOUP SWITCH",      NULL,                "PLL CLK" },

	{ "PLAY48K SWITCH",      NULL,                "PLL CLK" },
	{ "PLAY96K SWITCH",      NULL,                "PLL CLK" },
	{ "PLAY192K SWITCH",     NULL,                "PLL CLK" },
	{ "LOWLATENCY SWITCH",   NULL,                "PLL CLK" },

	{ "SOUNDTRIGGER ONEMIC SWITCH",   NULL,       "PLL CLK" },
	{ "SOUNDTRIGGER DUALMIC SWITCH",  NULL,       "PLL CLK" },
	{ "SOUNDTRIGGER MULTIMIC SWITCH", NULL,       "PLL CLK" },

	{ "U7 DRV",              NULL,                "PLL CLK" },

	{ "U1 DRV",              NULL,                "S1 OL SWITCH" },
	{ "U2 DRV",              NULL,                "S1 OR SWITCH" },
	{ "S2 TX",               NULL,                "S2 OL SWITCH" },
	{ "S2 TX",               NULL,                "S2 OR SWITCH" },
	{ "U3 DRV",              NULL,                "U3 OL SWITCH" },
	{ "U4 DRV",              NULL,                "U4 OR SWITCH" },
	{ "U5 DRV",              NULL,                "S3 OL SWITCH" },
	{ "U6 DRV",              NULL,                "S3 OR SWITCH" },
	{ "S4 TX",               NULL,                "S4 OL SWITCH" },
	{ "S4 TX",               NULL,                "S4 OR SWITCH" },

	{ "U7 DRV",              NULL,                "S4 OL SWITCH" },
	{ "U8 DRV",              NULL,                "S4 OR SWITCH" },

	/* CLK SUPPLY */
	{ "S1L IN",              NULL,                "DP CLK" },
	{ "S1R IN",              NULL,                "DP CLK" },
	{ "S1L IN",              NULL,                "PLL CLK" },
	{ "S1R IN",              NULL,                "PLL CLK" },

	{ "IR TX IN",            NULL,                "DP CLK" },
	{ "IR TX IN",            NULL,                "PLL CLK" },
	{ "IR TX IN",            NULL,                "IR TX CLK" },

	{ "S2L IN",              NULL,                "S2 RX" },
	{ "S2R IN",              NULL,                "S2 RX" },
	{ "S2 RX",               NULL,                "DP CLK" },
	{ "S2 RX",               NULL,                "S2 IF CLK" },
	{ "S2 TX",               NULL,                "S2 IF CLK" },
	{ "S2 RX",               NULL,                "PLL CLK" },

	{ "S3L IN",              NULL,                "DP CLK" },
	{ "S3R IN",              NULL,                "DP CLK" },
	{ "S3L IN",              NULL,                "PLL CLK" },
	{ "S3R IN",              NULL,                "PLL CLK" },

	{ "S4L IN",              NULL,                "S4 RX" },
	{ "S4R IN",              NULL,                "S4 RX" },
	{ "S4L OUT",             NULL,                "S4 RX" },
	{ "S4R OUT",             NULL,                "S4 RX" },
	{ "S4 RX",               NULL,                "DP CLK" },
	{ "S4 RX",               NULL,                "PLL CLK" },

	{ "S1L OUT",             NULL,                "DP CLK" },
	{ "S1R OUT",             NULL,                "DP CLK" },
	{ "S2L OUT",             NULL,                "DP CLK" },
	{ "S2R OUT",             NULL,                "DP CLK" },
	{ "S3L OUT",             NULL,                "DP CLK" },
	{ "S3R OUT",             NULL,                "DP CLK" },
	{ "S4L OUT",             NULL,                "DP CLK" },
	{ "S4R OUT",             NULL,                "DP CLK" },
	{ "U3L OUT",             NULL,                "DP CLK" },
	{ "U4R OUT",             NULL,                "DP CLK" },

	{ "MAD PGA",             NULL,                "MAD CLK" },
	{ "MAIN PGA",            NULL,                "PLL CLK" },
	{ "AUX PGA",             NULL,                "PLL CLK" },
	{ "LINEINL PGA",         NULL,                "PLL CLK" },
	{ "LINEINL PGA",         NULL,                "DP CLK" },
	{ "LINEINR PGA",         NULL,                "PLL CLK" },
	{ "LINEINR PGA",         NULL,                "DP CLK" },

	{ "MAD OUT",             NULL,                "MAD CLK" },
	{ "MAD OUT",             NULL,                "DP CLK" },

	{ "ECL OUT",             NULL,                "DP CLK" },
	{ "ECR OUT",             NULL,                "DP CLK" },
	{ "ECL OUT",             NULL,                "PLL CLK" },
	{ "ECR OUT",             NULL,                "PLL CLK" },

	{ "MAD SWITCH",          "SWITCH",            "MIC1 MUX" },
	{ "S1 OL SWITCH",        "SWITCH",            "MIC1 MUX" },
	{ "S1 OR SWITCH",        "SWITCH",            "MIC2 MUX" },
	{ "S2 OL SWITCH",        "SWITCH",            "BTL MUX" },
	{ "S2 OR SWITCH",        "SWITCH",            "BTR MUX" },
	{ "S3 OL SWITCH",        "SWITCH",            "MIC1 MUX" },
	{ "S3 OR SWITCH",        "SWITCH",            "MIC2 MUX" },
	{ "S4 OL SWITCH",        "SWITCH",            "M1L MUX" },
	{ "S4 OR SWITCH",        "SWITCH",            "M1R MUX" },

	{ "U3 OL SWITCH",        "SWITCH",            "MIC3 MUX" },
	{ "U4 OR SWITCH",        "SWITCH",            "MIC4 MUX" },


	/* DAC MIXER */
	{ "DACL MIXER",          "S1L SWITCH",        "S1 IL PGA" },
	{ "DACL MIXER",          "S2L SWITCH",        "S2 IL PGA" },
	{ "DACL MIXER",          "MDM SWITCH",        "MDM MUX" },
	{ "DACL MIXER",          "S1R SWITCH",        "S1 IR PGA" },

	{ "DACR MIXER",          "S1R SWITCH",        "S1 IR PGA" },
	{ "DACR MIXER",          "S2R SWITCH",        "S2 IR PGA" },
	{ "DACR MIXER",          "MDM SWITCH",        "MDM MUX" },
	{ "DACR MIXER",          "S1L SWITCH",        "S1 IL PGA" },

	/* DAC SRC MIXER MUX */
	{ "DACL SRC MUX",        "DACL",              "DACL MIXER" },
	{ "DACL SRC MUX",        "DSP IF8 OL",        "IF8 TEST MUX" },

	{ "DACR SRC MUX",        "DSP IF8 OR",        "IF8 TEST MUX" },
	{ "DACR SRC MUX",        "DACR",              "DACR MIXER" },

	/* DSP IF8 MUX */

	/* DAC SRC MIXER */
	{ "DACL SRC MIXER",      "DSP IF8 SWITCH",    "DACL SRC MUX" },
	{ "DACL SRC MIXER",      "SIDETONE SWITCH",   "SIDETONE" },

	{ "DACR SRC MIXER",      "SIDETONE SWITCH",   "SIDETONE" },
	{ "DACR SRC MIXER",      "DSP IF8 SWITCH",    "DACR SRC MUX" },

	/* DACS MIXER */
	{ "DACSL MIXER",         "DACL SWITCH",       "DACL MIXER" },
	{ "DACSL MIXER",         "SIDETONE SWITCH",   "SIDETONE" },
	{ "DACSL MIXER",         "DSP IF8 OL SWITCH", "DACL MIXER" },
	{ "DACSL MIXER",         "MDM SWITCH",        "MDM MUX" },

	{ "DACSR MIXER",         "DACR SWITCH",       "DACR MIXER" },
	{ "DACSR MIXER",         "SIDETONE SWITCH",   "SIDETONE" },
	{ "DACSR MIXER",         "DSP IF8 OR SWITCH", "DACR MIXER" },
	{ "DACSR MIXER",         "MDM SWITCH",        "MDM MUX" },

	/* DAC FLT MIXER */
	{ "DACL FLT MIXER",      "DACL SRC SWITCH",   "DACL SRC MIXER" },
	{ "DACL FLT MIXER",      "S1L SWITCH",        "S1 IL PGA" },

	{ "DACR FLT MIXER",      "DACR SRC SWITCH",   "DACR SRC MIXER" },
	{ "DACR FLT MIXER",      "S1R SWITCH",        "S1 IR PGA" },

	/* DAC PGA */
	{ "DACL FLT PGA",        NULL,                "DACL FLT MIXER" },
	{ "DACR FLT PGA",        NULL,                "DACR FLT MIXER" },

	{ "DACSL PGA",           NULL,                "DACSL MIXER" },
	{ "DACSR PGA",           NULL,                "DACSR MIXER" },

	/* DACR PGA MUX */
	{ "DACL PGA MUX",        "DACL_384",          "DACL FLT PGA" },
	{ "DACL PGA MUX",        "ADC1L",             "ADC1L" },

	/* DACR PGA MUX */
	{ "DACR PGA MUX",        "DACR_384",          "DACR FLT PGA" },
	{ "DACR PGA MUX",        "ADC1R",             "ADC1R" },

	{ "DACL PGA",            NULL,                "DACL PGA MUX" },
	{ "DACR PGA",            NULL,                "DACR PGA MUX" },

	{ "DACL FLT PGA",	    NULL,                "DP CLK" },
	{ "DACR FLT PGA",	    NULL,                "DP CLK" },
	{ "DACL PGA",            NULL,                "DP CLK" },
	{ "DACR PGA",            NULL,                "DP CLK" },
	{ "DACSL PGA",           NULL,                "DP CLK" },
	{ "DACSR PGA",           NULL,                "DP CLK" },
	{ "DACL FLT PGA",        NULL,                "PLL CLK" },
	{ "DACR FLT PGA",	    NULL,                "PLL CLK" },
	{ "DACL PGA",            NULL,                "PLL CLK" },
	{ "DACR PGA",            NULL,                "PLL CLK" },
	{ "DACSL PGA",           NULL,                "PLL CLK" },
	{ "DACSR PGA",           NULL,                "PLL CLK" },

	/* HPL MUX */
	{ "DAC HPL MUX",         "DACL SDM45",        "DACL PGA" },
	{ "DAC HPL MUX",         "DACR SDM45",        "DACR PGA" },

	/* HPR MUX */
	{ "DAC HPR MUX",         "DACL SDM45",        "DACL PGA" },
	{ "DAC HPR MUX",         "DACR SDM45",        "DACR PGA" },

	/* EP MUX */
	{ "DAC EP MUX",          "DACL SDM",          "DACL PGA" },
	{ "DAC EP MUX",          "DACSL",             "DACSL PGA" },
	{ "DAC EP MUX",          "DACR SDM",          "DACR PGA" },
	{ "DAC EP MUX",          "DACSR",             "DACSR PGA" },
	{ "DAC EP MUX",          "DSP_IF8",           "IR TX IN" },

	/* LOL MUX */
	{ "DAC LINEOUTL MUX",    "DACL SDM",          "DACL PGA" },
	{ "DAC LINEOUTL MUX",    "DACSL",             "DACSL PGA" },
	{ "DAC LINEOUTL MUX",    "DACR SDM",          "DACR PGA" },
	{ "DAC LINEOUTL MUX",    "DACSR",             "DACSR PGA" },

	/* LOR MUX */
	{ "DAC LINEOUTR MUX",    "DACL SDM",          "DACL PGA" },
	{ "DAC LINEOUTR MUX",    "DACSL",             "DACSL PGA" },
	{ "DAC LINEOUTR MUX",    "DACR SDM",          "DACR PGA" },
	{ "DAC LINEOUTR MUX",    "DACSR",             "DACSR PGA" },

	/*PDM MUX */

	/* OUT DRIVER */
	{ "HPL DRV",             NULL,                "DAC HPL MUX" },
	{ "HPR DRV",             NULL,                "DAC HPR MUX" },

	{ "EP DRV",              NULL,                "DAC EP MUX" },

	{ "LOL DRV",             NULL,                "DAC LINEOUTL MUX" },
	{ "LOR DRV",             NULL,                "DAC LINEOUTR MUX" },

	/* OUTPUT */
	{ "EAR OUT",             NULL,                "EP DRV" },
	{ "HPL OUT",             NULL,                "HPL DRV" },
	{ "HPR OUT",             NULL,                "HPR DRV" },
	{ "LINEOUT L",           NULL,                "LOL DRV" },
	{ "LINEOUT R",           NULL,                "LOR DRV" },

	{ "HPL DRV",             NULL,                "CP2" },
	{ "HPR DRV",             NULL,                "CP2" },
	{ "EP DRV",              NULL,                "CP1" },

	{ "CP2",                 NULL,                "CP1" },
	{ "CP1",                 NULL,                "PLL CLK" },

	/* SIDETONE MUX */
	{ "SIDETONE MUX",        "S1L",               "S1 IL PGA" },
	{ "SIDETONE MUX",        "S2L",               "S2 IL PGA" },
	{ "SIDETONE MUX",        "ADC0L",             "ADC0L" },
	{ "SIDETONE MUX",        "ADC0R",             "ADC0R" },
	{ "SIDETONE MUX",        "ADC1L",             "ADC1L" },
	{ "SIDETONE MUX",        "ADC1R",             "ADC1R" },

	/* MDM MUX */
	{ "MDM MUX",             "S3L",               "S3 IL PGA" },

	/* SIDE PGA */
	{ "SIDETONE",            NULL,                "SIDETONE MUX" },

	/* MIC1 MUX */
	{ "MIC1 MUX",            "ADC0L",             "ADC0L" },
	{ "MIC1 MUX",            "ADC1L",             "ADC1L" },
	{ "MIC1 MUX",            "ADC0R",             "ADC0R" },
	{ "MIC1 MUX",            "ADC1R",             "ADC1R" },
	{ "MIC1 MUX",            "S2L",               "S2 IL PGA" },
	{ "MIC1 MUX",            "S3L",               "S3 IL PGA" },
	{ "MIC1 MUX",            "S4L",               "S4L IN" },
	{ "MIC1 MUX",            "DACL_48",           "DACL SRC MIXER" },

	/* MIC2 MUX */
	{ "MIC2 MUX",            "ADC0L",             "ADC0L" },
	{ "MIC2 MUX",            "ADC1L",             "ADC1L" },
	{ "MIC2 MUX",            "ADC0R",             "ADC0R" },
	{ "MIC2 MUX",            "ADC1R",             "ADC1R" },
	{ "MIC2 MUX",            "S2R",               "S2 IR PGA" },
	{ "MIC2 MUX",            "S3R",               "S3 IR PGA" },
	{ "MIC2 MUX",            "S4R",               "S4R IN" },
	{ "MIC2 MUX",            "DACR_48",           "DACR SRC MIXER" },

	/* MIC3 MUX */
	{ "MIC3 MUX",            "ADC0L",             "ADC0L" },
	{ "MIC3 MUX",            "ADC1L",             "ADC1L" },
	{ "MIC3 MUX",            "ADC0R",             "ADC0R" },
	{ "MIC3 MUX",            "ADC1R",             "ADC1R" },
	{ "MIC3 MUX",            "S1L",               "S1 IL PGA" },
	{ "MIC3 MUX",            "S2L",               "S2 IL PGA" },
	{ "MIC3 MUX",            "S4L",               "S4L IN" },
	{ "MIC3 MUX",            "DACL_48",           "DACR SRC MIXER" },

	/* MIC4 MUX */
	{ "MIC4 MUX",            "ADC0L",             "ADC0L" },
	{ "MIC4 MUX",            "ADC1L",             "ADC1L" },
	{ "MIC4 MUX",            "ADC0R",             "ADC0R" },
	{ "MIC4 MUX",            "ADC1R",             "ADC1R" },
	{ "MIC4 MUX",            "S1R",               "S1 IR PGA" },
	{ "MIC4 MUX",            "S2R",               "S2 IR PGA" },
	{ "MIC4 MUX",            "S4R",               "S4R IN" },
	{ "MIC4 MUX",            "DACR_48",           "DACR SRC MIXER" },

	/* BTL MUX */
	{ "BTL MUX",             "ADC0L",             "ADC0L" },
	{ "BTL MUX",             "ADC1L",             "ADC1L" },
	{ "BTL MUX",             "DACL_48",           "DACL SRC MIXER" },
	{ "BTL MUX",             "S1L",               "S1 IL PGA" },
	{ "BTL MUX",             "S3L",               "S3 IL PGA" },
	{ "BTL MUX",             "S4L",               "S4L IN" },

	/* BTR MUX */
	{ "BTR MUX",             "ADC0R",             "ADC0R" },
	{ "BTR MUX",             "ADC1R",             "ADC1R" },
	{ "BTR MUX",             "DACR_48",           "DACR SRC MIXER" },
	{ "BTR MUX",             "S1R",               "S1 IR PGA" },
	{ "BTR MUX",             "S3R",               "S3 IR PGA" },
	{ "BTR MUX",             "S4R",               "S4L IN" },

	/* M1L MUX */
	{ "M1L MUX",             "ADC0L",             "ADC0L" },
	{ "M1L MUX",             "ADC1L",             "ADC1L" },
	{ "M1L MUX",             "DACL_48",           "DACL SRC MIXER" },
	{ "M1L MUX",             "DACL_48",           "ECL IN" },
	{ "M1L MUX",             "DACSL",             "DACSL MIXER" },
	{ "M1L MUX",             "S1L",               "S1 IL PGA" },
	{ "M1L MUX",             "S2L",               "S2 IL PGA" },
	{ "M1L MUX",             "S3L",               "S3 IL PGA" },

	/* M1R MUX */
	{ "M1R MUX",             "ADC0R",             "ADC0R" },
	{ "M1R MUX",             "ADC1R",             "ADC1R" },
	{ "M1R MUX",             "DACR_48",           "DACR SRC MIXER" },
	{ "M1R MUX",             "DACR_48",           "ECR IN" },
	{ "M1R MUX",             "DACSR",             "DACSR MIXER" },
	{ "M1R MUX",             "S1R",               "S1 IR PGA" },
	{ "M1R MUX",             "S2R",               "S2 IR PGA" },
	{ "M1R MUX",             "S3R",               "S3 IR PGA" },

	/* ADC */
	{ "ADC0L",               NULL,                "ADC0L MUX" },
	{ "ADC0R",               NULL,                "ADC0R MUX" },
	{ "ADC1L",               NULL,                "ADC1L MUX" },
	{ "ADC1R",               NULL,                "ADC1R MUX" },

	/* ADC MUX */
	{ "ADC0L MUX",           "MAINMIC",           "MAIN PGA" },
	{ "ADC0L MUX",           "AUXMIC",            "AUX PGA" },
	{ "ADC0L MUX",           "LINEINL",           "LINEINL PGA" },
	{ "ADC0L MUX",           "LINEINR",           "LINEINR PGA" },
	{ "ADC0L MUX",           "DMIC0L",            "DIGMIC0" },
	{ "ADC0L MUX",           "DMIC0R",            "DIGMIC0" },
	{ "ADC0L MUX",           "DMIC1L",            "DIGMIC1" },
	{ "ADC0L MUX",           "DMIC1R",            "DIGMIC1" },
	{ "ADC0L MUX",           "MAD",               "MAD PGA" },

	{ "ADC0R MUX",           "MAINMIC",           "MAIN PGA" },
	{ "ADC0R MUX",           "AUXMIC",            "AUX PGA" },
	{ "ADC0R MUX",           "LINEINL",           "LINEINL PGA" },
	{ "ADC0R MUX",           "LINEINR",           "LINEINR PGA" },
	{ "ADC0R MUX",           "DMIC0L",            "DIGMIC0" },
	{ "ADC0R MUX",           "DMIC0R",            "DIGMIC0" },
	{ "ADC0R MUX",           "DMIC1L",            "DIGMIC1" },
	{ "ADC0R MUX",           "DMIC1R",            "DIGMIC1" },
	{ "ADC0R MUX",           "MAD",               "MAD PGA" },

	{ "ADC1L MUX",           "MAINMIC",           "MAIN PGA" },
	{ "ADC1L MUX",           "AUXMIC",            "AUX PGA" },
	{ "ADC1L MUX",           "LINEINL",           "LINEINL PGA" },
	{ "ADC1L MUX",           "LINEINR",           "LINEINR PGA" },
	{ "ADC1L MUX",           "DMIC0L",            "DIGMIC0" },
	{ "ADC1L MUX",           "DMIC0R",            "DIGMIC0" },
	{ "ADC1L MUX",           "DMIC1L",            "DIGMIC1" },
	{ "ADC1L MUX",           "DMIC1R",            "DIGMIC1" },
	{ "ADC1L MUX",           "MAD",               "MAD PGA" },

	{ "ADC1R MUX",           "MAINMIC",           "MAIN PGA" },
	{ "ADC1R MUX",           "AUXMIC",            "AUX PGA" },
	{ "ADC1R MUX",           "LINEINL",           "LINEINL PGA" },
	{ "ADC1R MUX",           "LINEINR",           "LINEINR PGA" },
	{ "ADC1R MUX",           "DMIC0L",            "DIGMIC0" },
	{ "ADC1R MUX",           "DMIC0R",            "DIGMIC0" },
	{ "ADC1R MUX",           "DMIC1L",            "DIGMIC1" },
	{ "ADC1R MUX",           "DMIC1R",            "DIGMIC1" },
	{ "ADC1R MUX",           "MAD",               "MAD PGA" },

	{ "MAD PGA",             NULL,                "MAD PGA MIXER" },
	{ "MAIN PGA",            NULL,                "MAINMIC MUX" },
	{ "AUX PGA",             NULL,                "AUXMIC" },
	{ "LINEINL PGA",         NULL,                "LINEIN L MUX" },
	{ "LINEINR PGA",         NULL,                "LINEIN R MUX" },

	{ "MAD PGA MIXER",       "MIC4 SWITCH",       "MIC4" },
	{ "MAD PGA MIXER",       "MIC3 SWITCH",       "MIC3" },
	{ "MAD PGA MIXER",       "AUXMIC SWITCH",     "AUXMIC" },
	{ "MAD PGA MIXER",       "MAINMIC SWITCH",    "MAINMIC" },
	{ "MAD PGA MIXER",       "HPMIC SWITCH",      "HSMIC" },

	{ "MAINMIC MUX",         "MAINMIC",           "MAINMIC" },
	{ "MAINMIC MUX",         "HSMIC",             "HSMIC" },

	{ "LINEIN L MUX",        "LINEIN1",           "LINEINL" },
	{ "LINEIN L MUX",        "LINEIN2",           "LINEINR" },
	{ "LINEIN L MUX",        "LINEOUT",           "LINEOUTL LOOP" },
	{ "LINEIN L MUX",        "MIC3",              "MIC3" },

	{ "LINEIN R MUX",        "LINEIN1",           "LINEINL" },
	{ "LINEIN R MUX",        "LINEIN2",           "LINEINR" },
	{ "LINEIN R MUX",        "LINEOUT",           "LINEOUTR LOOP" },
	{ "LINEIN R MUX",        "MIC4",              "MIC4" },

	/* ANA IN */
	{ "MAINMIC",             NULL,                "MAIN MIC" },
	{ "HSMIC",               NULL,                "HS MIC" },
	{ "AUXMIC",              NULL,                "AUX MIC" },
	{ "MIC3",                NULL,                "MIC3 IN" },
	{ "MIC4",                NULL,                "MIC4 IN" },

	/* ANC INPUT DATA */
	{ "ANC CORE OUT",        NULL,                "ANC FAREND MUX" },
	{ "ANC CORE OUT",        NULL,                "ERROR MIC MUX" },
	{ "ANC CORE OUT",        NULL,                "REF MIC MUX" },
	/* ANC FAREND MUX */
	{ "ANC FAREND MUX",      "DACSL_384",         "DACSL PGA" },
	{ "ANC FAREND MUX",      "DACSR_384",         "DACSR PGA" },

	/* ERROR MIC MUX */
	{ "ERROR MIC MUX",       "L0",                "ANC CORE IN" },
	{ "ERROR MIC MUX",       "R0",                "ANC CORE IN" },
	{ "ERROR MIC MUX",       "L1",                "ANC CORE IN" },
	{ "ERROR MIC MUX",       "R1",                "ANC CORE IN" },

	/* REF MIC MUX */
	{ "REF MIC MUX",         "L0",                "ANC CORE IN" },
	{ "REF MIC MUX",         "R0",                "ANC CORE IN" },
	{ "REF MIC MUX",         "L1",                "ANC CORE IN" },
	{ "REF MIC MUX",         "R1",                "ANC CORE IN" },

	{ "IR STUDY ENV SWITCH", "SWITCH",            "ADC1L" },
	{ "IR STUDY ENV SWITCH", NULL,                "DP CLK" },
	{ "IR RX8 OUT",          NULL,                "IR STUDY ENV SWITCH" },

	/* I2S2 BLUETOOTH LOOP */
	{ "I2S2 BLUETOOTH LOOP SWITCH",  "SWITCH",    "I2S2 BLUETOOTH LOOP IN" },
	{ "I2S2 BLUETOOTH LOOP OUT",     NULL,        "I2S2 BLUETOOTH LOOP SWITCH" },
	{ "I2S2 BLUETOOTH LOOP SWITCH",  NULL,        "PLL CLK" },
	{ "I2S2 BLUETOOTH LOOP SWITCH",  NULL,        "DP CLK" },
};

int hi6403_add_routes(struct snd_soc_codec *codec)
{
	struct snd_soc_dapm_context *dapm;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_codec_get_dapm(codec);
	return snd_soc_dapm_add_routes(dapm, route_map,
		ARRAY_SIZE(route_map));
}

