/*
 * slimbus is a kernel driver which is used to manager slimbus devices
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#include "slimbus_da_combine_v5.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <dsm/dsm_pub.h>

#include "linux/hisi/audio_log.h"
#include "slimbus_utils.h"
#include "csmi.h"
#include "slimbus.h"
#include "slimbus_da_combine.h"
#include "slimbus_utils.h"
#include "slimbus_drv.h"

/*lint -e750 -e730 -e785 -e574*/
#define LOG_TAG "slimbus_DA_combine_v5"

/*lint -e838 -e730 -e747 -e774 -e826 -e529 -e438 -e485 -e785 -e651 -e64 -e527 -e570*/

static struct slimbus_channel_property audio_playback_da_combine_v5[SLIMBUS_AUDIO_PLAYBACK_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property audio_capture_da_combine_v5[SLIMBUS_AUDIO_CAPTURE_MULTI_MIC_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property voice_down_da_combine_v5[SLIMBUS_VOICE_DOWN_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property voice_up_da_combine_v5[SLIMBUS_VOICE_UP_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property ec_ref_da_combine_v5[SLIMBUS_ECREF_1CH] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property sound_trigger_da_combine_v5[SLIMBUS_SOUND_TRIGGER_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property audio_debug_da_combine_v5[SLIMBUS_DEBUG_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property audio_direct_playback_da_combine_v5[SLIMBUS_AUDIO_PLAYBACK_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property audio_soc_2pa_iv_da_combine_v5[SLIMBUS_AUDIO_PLAYBACK_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property audio_soc_4pa_iv_da_combine_v5[SLIMBUS_AUDIO_PLAYBACK_MULTI_PA_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property audio_soc_playback_d3d4_da_combine_v5[SLIMBUS_AUDIO_PLAYBACK_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property audio_kws_da_combine_v5[SLIMBUS_KWS_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property audio_capture_5mic_da_combine_v5[SLIMBUS_AUDIO_CAPTURE_5MIC_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property voice_up_4pa_da_combine_v5[SLIMBUS_VOICE_UP_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property bt_down_da_combine_v5[SLIMBUS_VOICE_DOWN_CHANNELS] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property bt_up_da_combine_v5[SLIMBUS_VOICE_UP_2CH] = {{0, {0,}, {{0,},},},};
static struct slimbus_channel_property ultra_up_da_combine_v5[SLIMBUS_ECREF_1CH] = {{0, {0,}, {{0,},},},};


static struct slimbus_track_config track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_MAX] = {
	/* play back */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_playback_da_combine_v5[0],
	},
	/* capture */
	{
		.params.channels = SLIMBUS_AUDIO_CAPTURE_MULTI_MIC_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_capture_da_combine_v5[0],
	},
	/* voice down */
	{
		.params.channels = SLIMBUS_VOICE_DOWN_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_8K,
		.params.callback = NULL,
		.channel_pro = &voice_down_da_combine_v5[0],
	},
	/* voice up */
	{
		.params.channels = SLIMBUS_VOICE_UP_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_8K,
		.params.callback = NULL,
		.channel_pro = &voice_up_da_combine_v5[0],
	},
	/* ec */
	{
		.params.channels = SLIMBUS_ECREF_1CH,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &ec_ref_da_combine_v5[0],
	},
	/* sound trigger */
	{
		.params.channels = SLIMBUS_SOUND_TRIGGER_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_192K,
		.params.callback = NULL,
		.channel_pro = &sound_trigger_da_combine_v5[0],
	},
	/* debug */
	{
		.params.channels = SLIMBUS_DEBUG_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_192K,
		.params.callback = NULL,
		.channel_pro = &audio_debug_da_combine_v5[0],
	},
	/* direct play */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_192K,
		.params.callback = NULL,
		.channel_pro = &audio_direct_playback_da_combine_v5[0],
	},
	/* 2pa play in soc */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_soc_2pa_iv_da_combine_v5[0],
	},
	/* 4pa play in soc */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_MULTI_PA_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_soc_4pa_iv_da_combine_v5[0],
	},
	/* d3d4 playback */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_soc_playback_d3d4_da_combine_v5[0],
	},
	/* KWS */
	{
		.params.channels = SLIMBUS_KWS_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_16K,
		.params.callback = NULL,
		.channel_pro = &audio_kws_da_combine_v5[0],
	},
	/* 5mic */
	{
		.params.channels = SLIMBUS_AUDIO_CAPTURE_5MIC_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_capture_5mic_da_combine_v5[0],
	},
	/* 4pa call in soc */
	{
		.params.channels = SLIMBUS_VOICE_UP_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_8K,
		.params.callback = NULL,
		.channel_pro = &voice_up_4pa_da_combine_v5[0],
	},
	/* bt down */
	{
		.params.channels = SLIMBUS_VOICE_DOWN_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &bt_down_da_combine_v5[0],
	},
	/* bt up */
	{
		.params.channels = SLIMBUS_VOICE_UP_2CH,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &bt_up_da_combine_v5[0],
	},
	/* ultra up */
	{
		.params.channels = SLIMBUS_ECREF_1CH,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &ultra_up_da_combine_v5[0],
	},
};

enum {
	SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON             = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY),
	SLIMBUS_DA_COMBINE_V5_TRACK_CAPTURE_ON          = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE),
	SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON       = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN),
	SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON         = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP),
	SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON               = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_ECREF),
	SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER_ON    = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER),
	SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG_ON            = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG),
	SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON      = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY),
	SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON           = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV),
	SLIMBUS_DA_COMBINE_V5_TRACK_4PA_IV_ON           = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_4PA_IV),
	SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON  = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4),
	SLIMBUS_DA_COMBINE_V5_TRACK_KWS_ON              = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_KWS),
	SLIMBUS_DA_COMBINE_V5_TRACK_5MIC_CAPTURE_ON     = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_5MIC_CAPTURE),
	SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA_ON     = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA),
	SLIMBUS_DA_COMBINE_V5_TRACK_BT_DOWN_ON          = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_BT_DOWN),
	SLIMBUS_DA_COMBINE_V5_TRACK_BT_UP_ON            = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_BT_UP),
	SLIMBUS_DA_COMBINE_V5_TRACK_ULTRA_UP_ON         = BIT(SLIMBUS_DA_COMBINE_V5_TRACK_ULTRA_UP),
};

/* segment length */
static uint16_t sl_table[SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX][SLIMBUS_DA_COMBINE_V5_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
	/* 6.144M FPGA */
	{
		{ 6, 6 },                           /* audio playback */
		{ 4, 4 },                           /* audio capture */
		{ 4, 4 },                           /* voice down */
		{ 4, 4, 4, 4 },                     /* voice up */
		{ 0 },                              /* EC_REF */
		{ 0 },                              /* sound trigger */
		{ 0 },                              /* debug */
		{ 0, 0 },                           /* direct play */
		{ 0, 0 },                           /* 2PA IV */
		{ 0, 0, 0, 0 },                     /* 4PA IV */
		{ 0, 0 },                           /* PLAY D3D4 */
		{ 0, 0 },                           /* kws */
		{ 0, 0, 0, 0, 0 },                  /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                     /* voice up 4pa */
		{ 0, 0 },                           /* bt down */
		{ 0, 0 },                           /* bt up */
		{ 0 },                              /* ultra_up */
	},
	/* 6.144M play */
	{
		{ 6, 6 },                           /* audio playback */
		{ 0, 0 },                           /* audio capture */
		{ 0, 0 },                           /* voice down */
		{ 0, 0, 0, 0 },                     /* voice up */
		{ 8 },                              /* EC_REF */
		{ 0 },                              /* sound trigger */
		{ 0 },                              /* debug */
		{ 0, 0 },                           /* direct play */
		{ 8, 8 },                           /* 2PA IV */
		{ 0, 0, 0, 0 },                     /* 4PA IV */
		{ 0, 0 },                           /* PLAY D3D4 */
		{ 0, 0 },                           /* kws */
		{ 0, 0, 0, 0, 0 },                  /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                     /* voice up 4pa */
		{ 0, 0 },                           /* bt down */
		{ 0, 0 },                           /* bt up */
		{ 0 },                              /* ultra_up */
	},
	/* 6.144M call */
	{
		{ 0, 0 },                           /* audio playback */
		{ 0, 0 },                           /* audio capture */
		{ 6, 6 },                           /* voice down */
		{ 6, 6, 6, 6 },                     /* voice up */
		{ 0 },                              /* EC_REF */
		{ 0 },                              /* sound trigger */
		{ 0 },                              /* debug */
		{ 0, 0 },                           /* direct play */
		{ 0, 0 },                           /* 2PA IV */
		{ 0, 0, 0, 0 },                     /* 4PA IV */
		{ 0, 0 },                           /* PLAY D3D4 */
		{ 0, 0 },                           /* kws */
		{ 0, 0, 0, 0, 0 },                  /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                     /* voice up 4pa */
		{ 0, 0 },                           /* bt down */
		{ 0, 0 },                           /* bt up */
		{ 0 },                              /* ultra_up */
	},
	/* 24.576M normal */
	{
		{ 6, 6 },                          /* audio playback */
		{ 6, 6, 6, 6 },                    /* audio capture */
		{ 0, 0 },                          /* voice down */
		{ 6, 6, 6, 6 },                    /* voice up */
		{ 8 },                             /* EC_REF */
		{ 8 },                             /* sound trigger */
		{ 8 },                             /* debug */
		{ 0, 0 },                          /* direct play */
		{ 8, 8 },                          /* 2PA IV */
		{ 0, 0, 0, 0 },                    /* 4PA IV */
		{ 4, 4 },                          /* PLAY D3D4 */
		{ 4, 4 },                          /* kws */
		{ 0, 0, 0, 0, 0 },                 /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                    /* voice up 4pa */
		{ 0, 0 },                          /* bt down */
		{ 0, 0 },                          /* bt up */
		{ 6 },                             /* ultra_up */
	},
	/* 24.576M direct play */
	{
		{ 0, 0 },                          /* audio playback */
		{ 6, 6, 6, 6 },                    /* audio capture */
		{ 0, 0 },                          /* voice down */
		{ 0, 0, 0, 0 },                    /* voice up */
		{ 0 },                             /* EC_REF */
		{ 8 },                             /* sound trigger */
		{ 0 },                             /* debug */
		{ 6, 6 },                          /* direct play */
		{ 0, 0 },                          /* 2PA IV */
		{ 0, 0, 0, 0 },                    /* 4PA IV */
		{ 0, 0 },                          /* PLAY D3D4 */
		{ 4, 4 },                          /* kws */
		{ 0, 0, 0, 0, 0 },                 /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                    /* voice up 4pa */
		{ 0, 0 },                          /* bt down */
		{ 0, 0 },                          /* bt up */
		{ 0 },                             /* ultra_up */
	},
	/* 12.288M call */
	{
		{ 6, 6 },                           /* audio playback */
		{ 0, 0 },                           /* audio capture */
		{ 6, 6 },                           /* voice down */
		{ 6, 6, 6, 6 },                     /* voice up */
		{ 8 },                              /* EC_REF */
		{ 0 },                              /* sound trigger */
		{ 0 },                              /* debug */
		{ 0, 0 },                           /* direct play*/
		{ 8, 8 },                           /* 2PA IV */
		{ 0, 0, 0, 0 },                     /* 4PA IV */
		{ 4, 4 },                           /* PLAY D3D4 */
		{ 0, 0 },                           /* kws */
		{ 0, 0, 0, 0, 0 },                  /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                     /* voice up 4pa */
		{ 0, 0 },                           /* bt down */
		{ 0, 0 },                           /* bt up */
		{ 0 },                              /* ultra_up */
	},
	/* 6.144M soundtrigger enhance */
	{
		{ 6, 6 },                            /* audio playback */
		{ 0, 0 },                            /* audio capture */
		{ 0, 0 },                            /* voice down */
		{ 0, 0, 0, 0 },                      /* voice up */
		{ 8 },                               /* EC_REF */
		{ 0, 0 },                            /* sound trigger */
		{ 0 },                               /* debug */
		{ 0, 0 },                            /* direct play */
		{ 0, 0 },                            /* 2PA IV */
		{ 0, 0, 0, 0 },                      /* 4PA IV */
		{ 0, 0 },                            /* PLAY D3D4 */
		{ 4, 4 },                            /* kws */
		{ 0, 0, 0, 0, 0 },                   /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                      /* voice up 4pa */
		{ 0, 0 },                            /* bt down */
		{ 0, 0 },                            /* bt up */
		{ 0 },                               /* ultra_up */
	},
	/* 4PA play in soc */
	{
		{ 4, 4 },                                         /* audio playback */
		{ 6, 6, 6, 6 },                                   /* audio capture */
		{ 4, 4 },                                         /* voice down */
		{ 6, 6, 6, 6 },                                   /* voice up */
		{ 0 },                                            /* EC_REF */
		{ 8 },                                            /* sound trigger */
		{ 0 },                                            /* debug */
		{ 0, 0 },                                         /* direct play*/
		{ 0, 0 },                                         /* 2PA IV*/
		{ 8, 8, 8, 8 },                                   /* 4PA IV*/
		{ 4, 4 },                                         /* PLAY D3D4 */
		{ 4, 4 },                                         /* kws */
		{ 4, 4, 4, 4, 4 },                                /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                                   /* voice up 4pa */
		{ 0, 0 },                                         /* bt down */
		{ 0, 0 },                                         /* bt up */
		{ 0 },                                            /* ultra_up */
	},
	/* 24.576M direct play-44.1k */
	{
		{ 0, 0 },                            /* audio playback */
		{ 6, 6, 6, 6 },                      /* audio capture */
		{ 0, 0 },                            /* voice down */
		{ 0, 0, 0, 0 },                      /* voice up */
		{ 0 },                               /* EC_REF */
		{ 8 },                               /* sound trigger */
		{ 0 },                               /* debug */
		{ 6, 6 },                            /* direct play */
		{ 0, 0 },                            /* 2PA IV */
		{ 0, 0, 0, 0 },                      /* 4PA IV */
		{ 0, 0 },                            /* PLAY D3D4 */
		{ 4, 4 },                            /* kws */
		{ 0, 0, 0, 0, 0 },                   /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                      /* voice up 4pa */
		{ 0, 0 },                            /* bt down */
		{ 0, 0 },                            /* bt up */
		{ 0 },                               /* ultra_up */
	},
	/* 4PA call */
	{
		{ 4, 4 },                                         /* audio playback */
		{ 6, 6, 6, 6 },                                   /* audio capture */
		{ 4, 4 },                                         /* voice down */
		{ 6, 6, 6, 6 },                                   /* voice up */
		{ 0 },                                            /* EC_REF */
		{ 8 },                                            /* sound trigger */
		{ 0 },                                            /* debug */
		{ 0, 0 },                                         /* direct play*/
		{ 0, 0 },                                         /* 2PA IV*/
		{ 8, 8, 8, 8 },                                   /* 4PA IV*/
		{ 4, 4 },                                         /* PLAY D3D4 */
		{ 0, 0 },                                         /* kws */
		{ 4, 4, 4, 4, 4 },                                /* 5MIC CAPTURE */
		{ 6, 6, 6, 6 },                                   /* voice up 4pa */
		{ 0, 0 },                                         /* bt down */
		{ 0, 0 },                                         /* bt up */
		{ 0 },                                            /* ultra_up */
	},
	/* 24.576M normal with bt sco */
	{
		{ 6, 6 },                          /* audio playback */
		{ 6, 6, 6, 6 },                    /* audio capture */
		{ 0, 0 },                          /* voice down */
		{ 0, 0, 0, 0 },                    /* voice up */
		{ 8 },                             /* EC_REF */
		{ 8 },                             /* sound trigger */
		{ 8 },                             /* debug */
		{ 0, 0 },                          /* direct play */
		{ 8, 8 },                          /* 2PA IV */
		{ 0, 0, 0, 0 },                    /* 4PA IV */
		{ 4, 4 },                          /* PLAY D3D4 */
		{ 4, 4 },                          /* kws */
		{ 0, 0, 0, 0, 0 },                 /* 5MIC CAPTURE */
		{ 0, 0, 0, 0 },                    /* voice up 4pa */
		{ 6, 6 },                          /* bt down */
		{ 6, 6 },                          /* bt up */
		{ 0 },                             /* ultra_up */
	},
};

/* presence rate */
static enum slimbus_presence_rate pr_table[SLIMBUS_DA_COMBINE_V5_TRACK_MAX] = {
			SLIMBUS_PR_48K, SLIMBUS_PR_48K,
			SLIMBUS_PR_8K, SLIMBUS_PR_8K,
			SLIMBUS_PR_48K, SLIMBUS_PR_192K,
			SLIMBUS_PR_192K, SLIMBUS_PR_192K,
			SLIMBUS_PR_48K, SLIMBUS_PR_48K,
			SLIMBUS_PR_48K, SLIMBUS_PR_16K,
			SLIMBUS_PR_48K, SLIMBUS_PR_8K,
			SLIMBUS_PR_48K, SLIMBUS_PR_48K,
			SLIMBUS_PR_48K,
};

static uint8_t cn_table_voice_up[SLIMBUS_PORT_VERSION_MAX][SLIMBUS_CHANNELS_MAX] = {
	{VOICE_UP_CHANNEL_MIC1, VOICE_UP_CHANNEL_MIC2, VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4},
	{VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4, DEBUG_LEFT, DEBUG_RIGHT},
};
static uint8_t cn_table_audio_up[SLIMBUS_PORT_VERSION_MAX][SLIMBUS_CHANNELS_MAX] = {
	{AUDIO_CAPTURE_CHANNEL_LEFT, AUDIO_CAPTURE_CHANNEL_RIGHT, VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4},
	{AUDIO_CAPTURE_CHANNEL_LEFT, AUDIO_CAPTURE_CHANNEL_RIGHT, DEBUG_RIGHT},
};

/* channel index */
static uint8_t cn_table[SLIMBUS_DA_COMBINE_V5_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ AUDIO_PLAY_CHANNEL_LEFT, AUDIO_PLAY_CHANNEL_RIGHT},
			{ AUDIO_CAPTURE_CHANNEL_LEFT, AUDIO_CAPTURE_CHANNEL_RIGHT,
				VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4},
			{ VOICE_DOWN_CHANNEL_LEFT, VOICE_DOWN_CHANNEL_RIGHT},
			{ VOICE_UP_CHANNEL_MIC1, VOICE_UP_CHANNEL_MIC2,
				VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4},
			{ VOICE_ECREF },
			{ SOUND_TRIGGER_CHANNEL_LEFT, SOUND_TRIGGER_CHANNEL_RIGHT },
			{ DEBUG_LEFT, DEBUG_RIGHT },
			{ AUDIO_PLAY_CHANNEL_LEFT, AUDIO_PLAY_CHANNEL_RIGHT },
			{ VOICE_ECREF, AUDIO_ECREF},
			{ VOICE_ECREF, AUDIO_ECREF, VOICE_UP_CHANNEL_MIC1, VOICE_UP_CHANNEL_MIC2 },
			{ AUDIO_PLAY_CHANNEL_D3, AUDIO_PLAY_CHANNEL_D4 },
			{ VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4 },
			{ AUDIO_CAPTURE_CHANNEL_LEFT, AUDIO_CAPTURE_CHANNEL_RIGHT,
				VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4, AUDIO_CAPTURE_CHANNEL_MIC5 },
			{ VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4, DEBUG_LEFT, DEBUG_RIGHT },
			{ VOICE_DOWN_CHANNEL_LEFT, VOICE_DOWN_CHANNEL_RIGHT },
			{ VOICE_UP_CHANNEL_MIC1, VOICE_UP_CHANNEL_MIC2 },
			{ DEBUG_LEFT },
};

static uint8_t source_pn_table_voice_up[SLIMBUS_PORT_VERSION_MAX][SLIMBUS_CHANNELS_MAX] = {
	{VOICE_UP_DA_COMBINE_MIC1_PORT, VOICE_UP_DA_COMBINE_MIC2_PORT, VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT},
	{VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT, BT_CAPTURE_DA_COMBINE_LEFT_PORT, BT_CAPTURE_DA_COMBINE_RIGHT_PORT},
};
static uint8_t source_pn_table_audio_up[SLIMBUS_PORT_VERSION_MAX][SLIMBUS_CHANNELS_MAX] = {
	{AUDIO_CAPTURE_DA_COMBINE_LEFT_PORT, AUDIO_CAPTURE_DA_COMBINE_RIGHT_PORT, VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT},
	{AUDIO_CAPTURE_DA_COMBINE_LEFT_PORT, AUDIO_CAPTURE_DA_COMBINE_RIGHT_PORT, BT_CAPTURE_DA_COMBINE_RIGHT_PORT},
};

/* source port number */
static uint8_t source_pn_table[SLIMBUS_DA_COMBINE_V5_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ AUDIO_PLAY_SOC_LEFT_PORT, AUDIO_PLAY_SOC_RIGHT_PORT },
			{ AUDIO_CAPTURE_DA_COMBINE_LEFT_PORT, AUDIO_CAPTURE_DA_COMBINE_RIGHT_PORT,
				VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT },
			{ VOICE_DOWN_SOC_LEFT_PORT, VOICE_DOWN_SOC_RIGHT_PORT },
			{ VOICE_UP_DA_COMBINE_MIC1_PORT, VOICE_UP_DA_COMBINE_MIC2_PORT,
				VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT },
			{ VOICE_DA_COMBINE_ECREF_PORT},
			{ VOICE_UP_DA_COMBINE_MIC1_PORT, VOICE_UP_DA_COMBINE_MIC2_PORT },
			{ BT_CAPTURE_DA_COMBINE_LEFT_PORT, BT_CAPTURE_DA_COMBINE_RIGHT_PORT },
			{ AUDIO_PLAY_SOC_LEFT_PORT, AUDIO_PLAY_SOC_RIGHT_PORT },
			{ VOICE_DA_COMBINE_ECREF_PORT, AUDIO_DA_COMBINE_ECREF_PORT },
			{ VOICE_DA_COMBINE_ECREF_PORT, AUDIO_DA_COMBINE_ECREF_PORT,
				VOICE_UP_DA_COMBINE_MIC1_PORT, VOICE_UP_DA_COMBINE_MIC2_PORT },
			{ AUDIO_PLAY_SOC_D3_PORT, AUDIO_PLAY_SOC_D4_PORT },
			{ VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT },
			{ AUDIO_CAPTURE_DA_COMBINE_LEFT_PORT, AUDIO_CAPTURE_DA_COMBINE_RIGHT_PORT,
				VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT, MIC5_CAPTURE_DA_COMBINE_RIGHT_PORT },
			{ VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT,
				BT_CAPTURE_DA_COMBINE_LEFT_PORT, BT_CAPTURE_DA_COMBINE_RIGHT_PORT },
			{ VOICE_DOWN_SOC_LEFT_PORT, VOICE_DOWN_SOC_RIGHT_PORT },
			{ VOICE_UP_DA_COMBINE_MIC1_PORT, VOICE_UP_DA_COMBINE_MIC2_PORT },
			{ BT_CAPTURE_DA_COMBINE_LEFT_PORT },
};

static uint8_t sink_pn_table_voice_up[SLIMBUS_PORT_VERSION_MAX][SLIMBUS_CHANNELS_MAX] = {
	{VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT, VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT},
	{VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT, BT_CAPTURE_SOC_LEFT_PORT, BT_CAPTURE_SOC_RIGHT_PORT},
};
static uint8_t sink_pn_table_audio_up[SLIMBUS_PORT_VERSION_MAX][SLIMBUS_CHANNELS_MAX] = {
	{AUDIO_CAPTURE_SOC_LEFT_PORT, AUDIO_CAPTURE_SOC_RIGHT_PORT, VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT},
	{AUDIO_CAPTURE_SOC_LEFT_PORT, AUDIO_CAPTURE_SOC_RIGHT_PORT, BT_CAPTURE_SOC_RIGHT_PORT},
};

/* sink port number */
static uint8_t sink_pn_table[SLIMBUS_DA_COMBINE_V5_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ AUDIO_PLAY_DA_COMBINE_LEFT_PORT, AUDIO_PLAY_DA_COMBINE_RIGHT_PORT },
			{ AUDIO_CAPTURE_SOC_LEFT_PORT, AUDIO_CAPTURE_SOC_RIGHT_PORT,
				VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT },
			{ VOICE_DOWN_DA_COMBINE_LEFT_PORT, VOICE_DOWN_DA_COMBINE_RIGHT_PORT },
			{ VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT,
				VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT },
			{ VOICE_SOC_ECREF_PORT },
			{ VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT},
			{ BT_CAPTURE_SOC_LEFT_PORT, BT_CAPTURE_SOC_RIGHT_PORT },
			{ AUDIO_PLAY_DA_COMBINE_LEFT_PORT, AUDIO_PLAY_DA_COMBINE_RIGHT_PORT },
			{ VOICE_SOC_ECREF_PORT, AUDIO_SOC_ECREF_PORT},
			{ VOICE_SOC_ECREF_PORT, AUDIO_SOC_ECREF_PORT,
				VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT },
			{ AUDIO_PLAY_DA_COMBINE_D3_PORT, AUDIO_PLAY_DA_COMBINE_D4_PORT },
			{ VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT },
			{ AUDIO_CAPTURE_SOC_LEFT_PORT, AUDIO_CAPTURE_SOC_RIGHT_PORT,
				VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT, MIC5_CAPTURE_SOC_RIGHT_PORT },
			{ VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT,
				BT_CAPTURE_SOC_LEFT_PORT, BT_CAPTURE_SOC_RIGHT_PORT },
			{ VOICE_DOWN_DA_COMBINE_LEFT_PORT, VOICE_DOWN_DA_COMBINE_RIGHT_PORT },
			{ VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT },
			{ BT_CAPTURE_SOC_LEFT_PORT },

};

/* segment distribution */
static uint16_t sd_table[SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX][SLIMBUS_DA_COMBINE_V5_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			/* 6.144M FPGA */
			{
				{ 0xc24, 0xc2a },                         /* audio playback */
				{ 0xc30, 0xc34 },                         /* audio capture */
				{ 0x58, 0x5c},                            /* voice down 8k */
				{ 0x78, 0x7c, 0x458, 0x45C },             /* voice up 8k */
				{ 0 },                                    /* EC_REF */
				{ 0 },                                    /* sound trigger 192K */
				{ 0 },                                    /* debug 192K */
				{ 0 },                                    /* direct play 192k */
				{ 0, 0 },                                 /* 2PA IV */
				{ 0, 0, 0, 0 },                           /* 4PA IV */
				{ 0, 0 },                                 /* PLAY D3D4 */
				{ 0, 0 },                                 /* kws */
				{ 0, 0, 0, 0, 0 },                        /* 5mic capture */
				{ 0, 0, 0, 0 },                           /* voice up 4pa 8k */
				{ 0, 0 },                                 /* bt down 48k */
				{ 0, 0 },                                 /* bt up 48k */
				{ 0 },                                    /* ultra_up 48k */
			},
			/* 6.144M play */
			{
				{ 0xC24, 0xC2A },                         /* audio playback */
				{ 0, 0 },                                 /* audio capture */
				{ 0, 0 },                                 /* voice down 8k */
				{ 0, 0, 0, 0 },                           /* voice up 8k */
				{ 0xC30 },                                /* EC_REF */
				{ 0 },                                    /* sound trigger 192K */
				{ 0 },                                    /* debug 192K */
				{ 0 },                                    /* direct play 192k */
				{ 0xC30, 0xC38 },                         /* 2PA IV */
				{ 0, 0, 0, 0 },                           /* 4PA IV */
				{ 0, 0 },                                 /* PLAY D3D4 */
				{ 0, 0 },                                 /* kws */
				{ 0, 0, 0, 0, 0 },                        /* 5mic capture */
				{ 0, 0, 0, 0 },                           /* voice up 4pa 8k */
				{ 0, 0 },                                 /* bt down 48k */
				{ 0, 0 },                                 /* bt up 48k */
				{ 0 },                                    /* ultra_up 48k */
			},
			/* 6.144M call */
			{
				{ 0, 0 },                                 /* audio playback */
				{ 0, 0 },                                 /* audio capture */
				{ 0x46, 0x5E },                           /* voice down 8k */
				{ 0x4C, 0x52, 0x64, 0x6A },               /* voice up 8k */
				{ 0 },                                    /* EC_REF */
				{ 0 },                                    /* sound trigger 192K */
				{ 0 },                                    /* debug 192K */
				{ 0 },                                    /* direct play 192k */
				{ 0, 0 },                                 /* 2PA IV */
				{ 0, 0, 0, 0 },                           /* 4PA IV */
				{ 0, 0 },                                 /* PLAY D3D4 */
				{ 0, 0 },                                 /* kws */
				{ 0, 0, 0, 0, 0 },                        /* 5mic capture */
				{ 0, 0, 0, 0 },                           /* voice up 4pa 8k */
				{ 0, 0 },                                 /* bt down 48k */
				{ 0, 0 },                                 /* bt up 48k */
				{ 0 },                                    /* ultra_up 48k */
			},
			/* 24.576M normal */
			{
				{ 0xCC6, 0xCE6 },                          /* audio playback */
				{ 0xC8C, 0xCAC, 0xCCC, 0xCEC },            /* audio capture */
				{ 0x112, 0x132 },                          /* voice down 8k */
				{ 0x152, 0x172, 0x192, 0x1B2 },            /* voice up 8k */
				{ 0xC84 },                                 /* EC_REF */
				{ 0xC38 },                                 /* sound trigger 192K */
				{ 0xC38 },                                 /* debug 192K */
				{ 0 },                                     /* direct play 192k */
				{ 0xc84, 0xca4 },                          /* 2PA IV */
				{ 0, 0, 0, 0 },                            /* 4PA IV */
				{ 0xCC2, 0xCE2 },                          /* PLAY D3D4 */
				{ 0x92, 0xB2 },                            /* kws */
				{ 0, 0, 0, 0, 0 },                         /* 5mic capture */
				{ 0, 0, 0, 0 },                            /* voice up 4pa 8k */
				{ 0, 0 },                                  /* bt down 48k */
				{ 0, 0 },                                  /* bt up 48k */
				{ 0xC86 },                                 /* ultra_up 48k */
			},
			/* 24.576M direct play */
			{
				{ 0, 0 },                                /* audio playback */
				{ 0xC8A, 0xCAA, 0xCCA, 0xCEA },          /* audio capture */
				{ 0, 0 },                                /* voice down 8k */
				{ 0, 0, 0, 0 },                          /* voice up 8k */
				{ 0 },                                   /* EC_REF */
				{ 0xC22 },                               /* sound trigger 192K */
				{ 0},                                    /* debug 192K */
				{ 0xC30, 0xC36 },                        /* direct play 192k */
				{ 0, 0 },                                /* 2PA IV */
				{ 0, 0, 0, 0 },                          /* 4PA IV */
				{ 0, 0 },                                /* PLAY D3D4 */
				{ 0x9C, 0xBC },                          /* kws */
				{ 0, 0, 0, 0, 0 },                       /* 5mic capture */
				{ 0, 0, 0, 0 },                          /* voice up 4pa 8k */
				{ 0, 0 },                                /* bt down 48k */
				{ 0, 0 },                                /* bt up 48k */
				{ 0 },                                   /* ultra_up 48k */
			},
			/* 12.288M call */
			{
				{ 0xC46, 0xC66 },                        /* audio playback */
				{ 0, 0 },                                /* audio capture */
				{ 0xCC, 0xD2 },                          /* voice down 8k */
				{ 0x8C, 0x92, 0xAC, 0xB2 },              /* voice up 8k */
				{ 0xC58 },                               /* EC_REF */
				{ 0 },                                   /* sound trigger 192K */
				{ 0 },                                   /* debug 192K */
				{ 0 },                                   /* direct play 192k */
				{ 0xC58, 0xC78 },                        /* 2PA IV */
				{ 0, 0, 0, 0 },                          /* 4PA IV */
				{ 0xC42, 0xC62 },                        /* PLAY D3D4 */
				{ 0, 0 },                                /* kws */
				{ 0, 0, 0, 0, 0 },                       /* 5mic capture */
				{ 0, 0, 0, 0 },                          /* voice up 4pa 8k */
				{ 0, 0 },                                /* bt down 48k */
				{ 0, 0 },                                /* bt up 48k */
				{ 0 },                                   /* ultra_up 48k */
			},
			/* 6.144M soundtrigger enhance */
			{
				{ 0xC28, 0xC2E },                         /* audio playback */
				{ 0, 0 },                                 /* audio capture */
				{ 0, 0 },                                 /* voice down */
				{ 0, 0, 0, 0 },                           /* voice up */
				{ 0xC34 },                                /* EC_REF */
				{ 0, 0 },                                 /* sound trigger */
				{ 0 },                                    /* debug */
				{ 0, 0 },                                 /* direct play */
				{ 0, 0 },                                 /* 2PA IV */
				{ 0, 0, 0, 0 },                           /* 4PA IV */
				{ 0, 0 },                                 /* PLAY D3D4 */
				{ 0x24, 0x424 },                          /* kws */
				{ 0, 0, 0, 0, 0 },                        /* 5mic capture */
				{ 0, 0, 0, 0 },                           /* voice up 4pa 8k */
				{ 0, 0 },                                 /* bt down 48k */
				{ 0, 0 },                                 /* bt up 48k */
				{ 0 },                                    /* ultra_up 48k */
			},
			/* 4PA play in soc */
			{
				{ 0xC94, 0xCB4 },                                 /* audio playback */
				{ 0xC8E, 0xCAE, 0xCCE, 0xCEE },                   /* audio capture */
				{ 0xCD4, 0xCF4 },                                 /* voice down 48k */
				{ 0, 0, 0, 0 },                                   /* voice up 8k */
				{ 0 },                                            /* EC_REF */
				{ 0xC38 },                                        /* sound trigger 192K*/
				{ 0 },                                            /* debug 192K*/
				{ 0 },                                            /* direct play 192k*/
				{ 0, 0 },                                         /* 2PA IV*/
				{ 0xC86, 0xCA6, 0xCC6, 0xCE6 },                   /* 4PA IV*/
				{ 0xCC2, 0xCE2 },                                 /* PLAY D3D4 */
				{ 0xA2, 0x4A2 },                                  /* kws */
				{ 0xC8E, 0xCAE, 0xCCE, 0xCEE, 0xc82 },            /* 5mic capture */
				{ 0, 0, 0, 0 },                                   /* voice up 4pa 8k */
				{ 0, 0 },                                         /* bt down 48k */
				{ 0, 0 },                                         /* bt up 48k */
				{ 0 },                                            /* ultra_up 48k */
			},
			/*24.576M direct play-44.1k */
			{
				{ 0, 0 },                                /* audio playback */
				{ 0xC8B, 0xCAB, 0xCCB, 0xCEB },          /* audio capture */
				{ 0, 0 },                                /* voice down 8k */
				{ 0, 0, 0, 0 },                          /* voice up 8k */
				{ 0 },                                   /* EC_REF */
				{ 0xC22 },                               /* sound trigger 192K */
				{ 0},                                    /* debug 192K */
				{ 0xC32, 0xC39 },                        /* direct play 192k */
				{ 0, 0 },                                /* 2PA IV */
				{ 0, 0, 0, 0 },                          /* 4PA IV */
				{ 0, 0 },                                /* PLAY D3D4 */
				{ 0xCB, 0xEB },                          /* kws */
				{ 0, 0 },                                /* 5mic capture */
				{ 0, 0, 0, 0 },                          /* voice up 4pa 8k */
				{ 0, 0 },                                /* bt down 48k */
				{ 0, 0 },                                /* bt up 48k */
				{ 0 },                                   /* ultra_up 48k */
			},
			/* 24.576M 4PA call */
			{
				{ 0xC90, 0xCB0 },                                 /* audio playback */
				{ 0xC8A, 0xCAA, 0xCCA, 0xCEA },                   /* audio capture */
				{ 0xCD0, 0xCF0 },                                 /* voice down 48k */
				{ 0x154, 0x174, 0x194, 0x1B4 },                   /* voice up 8k */
				{ 0 },                                            /* EC_REF */
				{ 0 },                                            /* sound trigger 192K*/
				{ 0 },                                            /* debug 192K*/
				{ 0 },                                            /* direct play 192k*/
				{ 0, 0 },                                         /* 2PA IV*/
				{ 0xC82, 0xCA2, 0xCC2, 0xCE2 },                   /* 4PA IV*/
				{ 0xCDA, 0xCBA },                                 /* 4PA D3D4 DOWN 48k */
				{ 0, 0 },                                         /* kws */
				{ 0xC8A, 0xCAA, 0xCCA, 0xCEA, 0xC9A },            /* 5mic capture */
				{ 0x154, 0x174, 0x194, 0x1B4 },                   /* voice up 4pa 8k */
				{ 0, 0 },                                         /* bt down 48k */
				{ 0, 0 },                                         /* bt up 48k */
				{ 0 },                                            /* ultra_up 48k */
			},
			/* 24.576M normal with bt sco */
			{
				{ 0xCC6, 0xCE6 },                          /* audio playback */
				{ 0xC8C, 0xCAC, 0xCCC, 0xCEC },            /* audio capture */
				{ 0, 0 },                          /* voice down 8k */
				{ 0, 0, 0, 0 },            /* voice up 8k */
				{ 0xC84 },                                 /* EC_REF */
				{ 0xC38 },                                 /* sound trigger 192K */
				{ 0xC38 },                                 /* debug 192K */
				{ 0 },                                     /* direct play 192k */
				{ 0xc84, 0xca4 },                          /* 2PA IV */
				{ 0, 0, 0, 0 },                            /* 4PA IV */
				{ 0xCC2, 0xCE2 },                          /* PLAY D3D4 */
				{ 0x92, 0xB2 },                            /* kws */
				{ 0, 0, 0, 0, 0 },                         /* 5mic capture */
				{ 0, 0, 0, 0 },                            /* voice up 4pa 8k */
				{ 0xC92, 0xCB2 },                          /* bt down 48k */
				{ 0xCD2, 0xCF2 },                          /* bt up 48k */
				{ 0 },                                     /* ultra_up 48k */
			},
};

static uint16_t sd_voice_down_da_combine_v5_16k[SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0x38, 0x3C },                                       /* 6.144M FPGA voice down 16k */
			{ 0, 0 },                                             /* 6.144M play */
			{ 0x26, 0x3E },                                       /* 6.144M call */
			{ 0x92, 0xB2 },                                       /* 24.576M normal */
			{ 0, 0 },                                             /* 24.576M direct play */
			{ 0x44C, 0x452 },                                     /* 12.288M call */
			{ 0, 0 },                                             /* 6.144M soundtrigger enhance */
			{ 0, 0 },                                             /* 24.579M PA IV */
			{ 0, 0 },                                             /* 24.576M direct play-44.1k */
			{ 0x94, 0xB4 },                                       /* 24.579M 4PA call */
};

static uint16_t sd_voice_up_da_combine_v5_16k[SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0x438, 0x43c, 0x838, 0x83c },                       /* 6.144M FPGA voice up 16k */
			{ 0, 0, 0, 0 },                                       /* 6.144M play */
			{ 0x2C, 0x32, 0x424, 0x42A },                         /* 6.144M call */
			{ 0xD2, 0xF2, 0x492, 0x4B2 },                         /* 24.576M normal */
			{ 0, 0, 0, 0 },                                       /* 24.576M direct play */
			{ 0x4C, 0x52, 0x6C, 0x72 },                           /* 12.288M call */
			{ 0, 0, 0, 0 },                                       /* 6.144M soundtrigger enhance */
			{ 0, 0, 0, 0 },                                       /* 24.579M 4PA IV */
			{ 0, 0, 0, 0 },                                       /* 24.576M direct play-44.1k */
			{ 0xD4, 0xF4, 0x494, 0x4B4 },                         /* 24.579M 4PA CALL */
};

static uint16_t sd_voice_down_da_combine_v5_32k[SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0, 0 },                                             /* 6.144M FPGA voice down 32k */
			{ 0, 0 },                                             /* 6.144M play */
			{ 0x16, 0x41E },                                      /* 6.144M call */
			{ 0x52, 0x72 },                                       /* 24.576M normal */
			{ 0, 0 },                                             /* 24.576M direct play */
			{ 0x82C, 0x832 },                                     /* 12.288M call */
			{ 0, 0 },                                             /* 6.144M soundtrigger enhance */
			{ 0, 0 },                                             /* 24.579M 4PA IV */
			{ 0, 0 },                                             /* 24.576M direct play-44.1k */
			{ 0x54, 0x74 },                                       /* 24.579M 4PA CALL */
};

static uint16_t sd_voice_up_da_combine_v5_32k[SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0, 0, 0, 0 },                                       /* 6.144M FPGA voice up 32k*/
			{ 0, 0, 0, 0 },                                       /* 6.144M PLAY */
			{ 0x1C, 0x412, 0x814, 0x81A },                        /* 6.144M call */
			{ 0x452, 0x472, 0x852, 0x872 },                       /* 24.576M normal */
			{ 0, 0, 0, 0 },                                       /* 24.576M direct play */
			{ 0x2C, 0x32, 0x42C, 0x432 },                         /* 12.288M call */
			{ 0, 0, 0, 0 },                                       /* 6.144M soundtrigger enhance */
			{ 0, 0, 0, 0 },                                       /* 24.579M 4PA IV */
			{ 0, 0, 0, 0 },                                       /* 24.576M direct play-44.1k */
			{ 0x454, 0x474, 0x854, 0x874 },                       /* 24.579M 4PA CALL */
};

static uint16_t sd_soundtrigger_48k[SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0},                                                /* 6.144M FPGA */
			{0},                                                /* 6.144M play */
			{0},                                                /* 6.144M call */
			{0xC98},                                            /* 24.576M normal */
			{0xC82},                                            /* 24.576M direct play */
			{0},                                                /* 12.288M call */
			{0},                                                /* 6.144M soundtrigger enhance */
			{ 0, 0 },                                             /* 24.579M 4PA play in soc */
			{0XC82},                                            /* 24.576M direct play-44.1k */
			{0, 0},                                             /* 24.579M 4PA CALL */
};

static uint16_t sd_direct_play_96k[SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0, 0 },                                             /* 6.144M FPGA voice down 16k */
			{ 0, 0 },                                             /* 6.144M play */
			{ 0, 0 },                                             /* 6.144M call */
			{ 0, 0 },                                             /* 24.576M normal */
			{ 0xC50, 0xC56 },                                     /* 24.576M direct play */
			{ 0, 0 },                                             /* 12.288M call */
			{ 0, 0 },                                             /* 6.144M soundtrigger enhance */
			{ 0, 0 },                                             /* 24.579M 4PA play in soc */
			{ 0XC52, 0XC59 },                                     /* 24.576M direct play-44.1k */
			{ 0, 0 },                                             /* 24.579M 4PA CALL */
};
static uint16_t sd_direct_play_48k[SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0, 0 },                                             /* 6.144M FPGA voice down 16k */
			{ 0, 0 },                                             /* 6.144M play */
			{ 0, 0 },                                             /* 6.144M call */
			{ 0, 0 },                                             /* 24.576M normal */
			{ 0xC90, 0xC96 },                                     /* 24.576M direct play */
			{ 0, 0 },                                             /* 12.288M call */
			{ 0, 0 },                                             /* 6.144M soundtrigger enhance */
			{ 0, 0 },                                             /* 24.579M 4PA play in soc */
			{ 0XC92, 0XC99 },                                     /* 24.576M direct play-44.1k */
			{ 0, 0 },                                             /* 24.579M 4PA CALL */
};

static void slimbus_da_combine_v5_set_direct_play_sd(uint32_t scene_type, uint32_t ch,
	enum slimbus_presence_rate pr, uint16_t *sd)
{
	if (pr == SLIMBUS_PR_96K || pr == SLIMBUS_PR_88200)
		*sd = sd_direct_play_96k[scene_type][ch];
	else if (pr == SLIMBUS_PR_48K || pr == SLIMBUS_PR_44100)
		*sd = sd_direct_play_48k[scene_type][ch];
}

static void slimbus_da_combine_v5_set_para_sd(uint32_t track_type, uint32_t scene_type,
	uint32_t ch, enum slimbus_presence_rate pr, uint16_t *sd)
{
	*sd = sd_table[scene_type][track_type][ch];

	switch (track_type) {
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN:
		if (pr == SLIMBUS_PR_16K)
			*sd = sd_voice_down_da_combine_v5_16k[scene_type][ch];
		if (pr == SLIMBUS_PR_32K)
			*sd = sd_voice_down_da_combine_v5_32k[scene_type][ch];
		break;
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA:
		if (pr == SLIMBUS_PR_16K)
			*sd = sd_voice_up_da_combine_v5_16k[scene_type][ch];
		if (pr == SLIMBUS_PR_32K)
			*sd = sd_voice_up_da_combine_v5_32k[scene_type][ch];
		break;
	case SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER:
		if (pr == SLIMBUS_PR_48K)
			*sd = sd_soundtrigger_48k[scene_type][ch];
		break;
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY:
		slimbus_da_combine_v5_set_direct_play_sd(scene_type, ch, pr, sd);
		break;
	default:
		break;
	}
}

static void slimbus_da_combine_v5_get_params_la(int32_t track_type, uint8_t *source_la,
	uint8_t *sink_la, enum slimbus_transport_protocol *tp)
{
	switch (track_type) {
	case SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY:
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN:
	case SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4:
	case SLIMBUS_DA_COMBINE_V5_TRACK_BT_DOWN:
		break;

	case SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP:
	case SLIMBUS_DA_COMBINE_V5_TRACK_ECREF:
	case SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER:
	case SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG:
	case SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV:
	case SLIMBUS_DA_COMBINE_V5_TRACK_4PA_IV:
	case SLIMBUS_DA_COMBINE_V5_TRACK_KWS:
	case SLIMBUS_DA_COMBINE_V5_TRACK_5MIC_CAPTURE:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA:
	case SLIMBUS_DA_COMBINE_V5_TRACK_BT_UP:
	case SLIMBUS_DA_COMBINE_V5_TRACK_ULTRA_UP:
		*source_la = DA_COMBINE_LA_GENERIC_DEVICE;
		*sink_la = SOC_LA_GENERIC_DEVICE;
		break;

	default:
		AUDIO_LOGE("track type is invalid: %d", track_type);
		return;
	}
}

/*
 * create da_combine_v5 slimbus device
 * @device, pointer to created instance
 *
 * return 0, if success
 */
static int32_t slimbus_da_combine_v5_create_device(struct slimbus_device_info **device)
{
	struct slimbus_device_info *dev_info = NULL;

	if (!device) {
		AUDIO_LOGE("device is invalid");
		return -EINVAL;
	}

	dev_info = kzalloc(sizeof(struct slimbus_device_info), GFP_KERNEL);
	if (!dev_info) {
		AUDIO_LOGE("kzalloc slimbus failed");
		return -ENOMEM;
	}
	dev_info->slimbus_da_combine_para = kzalloc(sizeof(struct slimbus_device_info), GFP_KERNEL);
	if (!dev_info->slimbus_da_combine_para) {
		AUDIO_LOGE("kzalloc slimbus failed");
		kfree(dev_info);
		return -ENOMEM;
	}

	dev_info->generic_la = DA_COMBINE_LA_GENERIC_DEVICE;
	dev_info->voice_up_chnum = SLIMBUS_VOICE_UP_2CH;
	dev_info->audio_up_chnum = SLIMBUS_AUDIO_CAPTURE_MULTI_MIC_CHANNELS; //default is 4mic
	dev_info->page_sel_addr = 1;
	dev_info->sm = SLIMBUS_SM_1_CSW_32_SL;
	dev_info->cg = SLIMBUS_CG_10;
	dev_info->scene_config_type = SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_NORMAL;

	dev_info->slimbus_da_combine_para->slimbus_track_max = SLIMBUS_DA_COMBINE_V5_TRACK_MAX;
	dev_info->slimbus_da_combine_para->track_config_table = track_config_da_combine_v5_table;

	mutex_init(&dev_info->rw_mutex);
	mutex_init(&dev_info->track_mutex);

	*device = dev_info;

	return 0;
}

static void slimbus_port_table_update(const struct slimbus_device_info *dev)
{
	uint32_t i;
	uint32_t voice_up_port_version;
	uint32_t audio_up_port_version;

	voice_up_port_version = dev->voice_up_port_version;
	if (voice_up_port_version >= SLIMBUS_PORT_VERSION_MAX) {
		AUDIO_LOGE("voice up port version error: %d", voice_up_port_version);
		voice_up_port_version = 0;
	}

	audio_up_port_version = dev->audio_up_port_version;
	if (audio_up_port_version >= SLIMBUS_PORT_VERSION_MAX) {
		AUDIO_LOGE("audio up port version error: %d", audio_up_port_version);
		audio_up_port_version = 0;
	}

	AUDIO_LOGI("voice, audio up port version is %d, %d",
		voice_up_port_version,
		audio_up_port_version);

	for (i = 0; i < SLIMBUS_CHANNELS_MAX; i++) {
		cn_table[SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP][i] =
			cn_table_voice_up[voice_up_port_version][i];
		source_pn_table[SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP][i] =
			source_pn_table_voice_up[voice_up_port_version][i];
		sink_pn_table[SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP][i] =
			sink_pn_table_voice_up[voice_up_port_version][i];
	}

	for (i = 0; i < SLIMBUS_CHANNELS_MAX; i++) {
		cn_table[SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE][i] =
			cn_table_audio_up[audio_up_port_version][i];
		source_pn_table[SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE][i] =
			source_pn_table_audio_up[audio_up_port_version][i];
		sink_pn_table[SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE][i] =
			sink_pn_table_audio_up[audio_up_port_version][i];
	}
}

static void config_param(uint32_t track, uint32_t scene_type,
	uint8_t source_la, uint8_t sink_la, enum slimbus_transport_protocol tp)
{
	int32_t i;
	uint32_t ch;
	uint8_t (*chnum_table)[SLIMBUS_CHANNELS_MAX] = cn_table;
	uint8_t (*source_pn)[SLIMBUS_CHANNELS_MAX] = source_pn_table;
	uint8_t (*sink_pn)[SLIMBUS_CHANNELS_MAX] = sink_pn_table;
	struct slimbus_channel_property *pchprop = track_config_da_combine_v5_table[track].channel_pro;

	for (ch = 0; ch < track_config_da_combine_v5_table[track].params.channels; ch++) {
		memset(pchprop, 0, sizeof(struct slimbus_channel_property));

		pchprop->cn = *(*(chnum_table + track) + ch);
		pchprop->source.la = source_la;
		pchprop->source.pn = *(*(source_pn + track) + ch);
		pchprop->sink_num = 1;
		for (i = 0; i < pchprop->sink_num; i++) {
			pchprop->sinks[i].la = sink_la;
			pchprop->sinks[i].pn = *(*(sink_pn + track) + ch);
		}
		if (track == SLIMBUS_DA_COMBINE_V5_TRACK_ECREF) {
			pchprop->sink_num = 2;
			pchprop->sinks[1].la = sink_la;
			pchprop->sinks[1].pn = BT_CAPTURE_SOC_RIGHT_PORT;

			if (!ch)
				pchprop->sinks[1].pn  = IMAGE_DOWNLOAD_SOC_PORT;
		}

		pchprop->tp = tp;
		pchprop->fl = 0;
		pchprop->af = SLIMBUS_AF_NOT_APPLICABLE;
		pchprop->dt = SLIMBUS_DF_NOT_INDICATED;
		pchprop->cl = 0;
		pchprop->pr = pr_table[track];

		if (track == SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER && pchprop->pr == SLIMBUS_PR_192K) {
			pchprop->source.pn = SLIMBUS_WAKEUP2MIC_DA_COMBINE_LEFT_PORT;
			pchprop->sinks[0].pn = SLIMBUS_WAKEUP2MIC_SOC_LEFT_PORT;
		}

		pchprop->sl = sl_table[scene_type][track][ch];

		slimbus_da_combine_v5_set_para_sd(track,
			scene_type, ch, pchprop->pr, &(pchprop->sd));

		pchprop->dl = pchprop->sl;

		pchprop++;
	}
}

static void slimbus_da_combine_v5_param_init(const struct slimbus_device_info *dev)
{
	uint32_t track;
	enum slimbus_transport_protocol tp = SLIMBUS_TP_ISOCHRONOUS;
	uint8_t source_la = SOC_LA_GENERIC_DEVICE;
	uint8_t sink_la = DA_COMBINE_LA_GENERIC_DEVICE;
	uint32_t scene_type;

	if (dev == NULL) {
		AUDIO_LOGE("dev is null");
		return;
	}
	if (dev->scene_config_type >= SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX) {
		AUDIO_LOGE("track type is invalid, scene type: %d", dev->scene_config_type);
		return;
	}

	slimbus_port_table_update(dev);

	scene_type = dev->scene_config_type;

	for (track = 0; track < SLIMBUS_DA_COMBINE_V5_TRACK_MAX; track++) {
		if (scene_type == SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_DIRECT_PLAY_441)
			tp = SLIMBUS_TP_PUSHED;
		else
			tp = SLIMBUS_TP_ISOCHRONOUS;

		source_la = SOC_LA_GENERIC_DEVICE;
		sink_la = DA_COMBINE_LA_GENERIC_DEVICE;

		slimbus_da_combine_v5_get_params_la(track, &source_la, &sink_la, &tp);
		config_param(track, scene_type, source_la, sink_la, tp);
	}
}

static int32_t slimbus_da_combine_v5_param_set(struct slimbus_device_info *dev,
	uint32_t track_type, struct slimbus_track_param *params)
{
	struct slimbus_channel_property *pchprop = NULL;
	uint32_t scene_type = SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX;
	struct slimbus_track_param slimbus_param;
	uint32_t ch;

	if (!dev) {
		AUDIO_LOGE("dev is NULL");
		return -EINVAL;
	}

	if (track_type >= SLIMBUS_DA_COMBINE_V5_TRACK_MAX) {
		AUDIO_LOGE("track type is invalid, track_type: %d", track_type);
		return -EINVAL;
	}
	if (dev->scene_config_type >= SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX) {
		AUDIO_LOGE("scene type is invalid, scene type: %d", dev->scene_config_type);
		return -EINVAL;
	}

	scene_type = dev->scene_config_type;

	if (!params) {
		memcpy(&slimbus_param, &(track_config_da_combine_v5_table[track_type].params), sizeof(slimbus_param));
		params = &slimbus_param;
	}
	AUDIO_LOGI("track type: %d, rate: %d, chnum: %d", track_type, params->rate, params->channels);

	track_config_da_combine_v5_table[track_type].params.channels = params->channels;
	track_config_da_combine_v5_table[track_type].params.rate = params->rate;
	track_config_da_combine_v5_table[track_type].params.callback = params->callback;

	pchprop = track_config_da_combine_v5_table[track_type].channel_pro; /*lint!e838*/

	if (track_type == SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP)
		dev->voice_up_chnum = params->channels;
	else if (track_type == SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA)
		dev->voice_up_chnum = params->channels;
	else if (track_type == SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE)
		dev->audio_up_chnum = params->channels;
	else if (track_type == SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN)
		track_config_da_combine_v5_table[track_type].params.channels = 2;

	if (pchprop) {
		for (ch = 0; ch < track_config_da_combine_v5_table[track_type].params.channels; ch++) {

			slimbus_da_combine_set_para_pr(pr_table, track_type, params);

			pchprop->pr = pr_table[track_type];

			slimbus_da_combine_v5_set_para_sd(track_type, scene_type, ch, pchprop->pr, &(pchprop->sd));
			if (pchprop->tp == SLIMBUS_TP_PUSHED)
				pchprop->sl = pchprop->sl + 1;
			pchprop++;
		}
	}

	return 0;
}

static void slimbus_da_combine_v5_param_update(const struct slimbus_device_info *dev,
	uint32_t track_type, const struct slimbus_track_param *params)
{
	if (track_type >= SLIMBUS_DA_COMBINE_V5_TRACK_MAX) {
		AUDIO_LOGE("track type is invalid, track_type: %d", track_type);
		return;
	}

	slimbus_da_combine_set_para_pr(pr_table, track_type, params);
	slimbus_da_combine_v5_param_init(dev);
}

static int32_t process_play_and_debug_conflict(enum slimbus_da_combine_v5_track track, int32_t *need_callback)
{
	int32_t ret = 0;

	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY) &&
		(track == SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG)) {
		AUDIO_LOGI("debug conflict");
		return -EPERM;
	}
	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG)) {
		*need_callback = 1;
		ret = slimbus_drv_track_deactivate(track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].channel_pro,
				track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].params.channels);

		slimbus_trackstate_set(SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG, false);
		track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].params.channels = 1;
	}

	return ret;
}

static int32_t process_soundtrigger_params_conflict(enum slimbus_da_combine_v5_track track)
{
	int32_t ret = 0;

	bool is_fast_track = ((track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER].params.channels == 2) &&
		(track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER].params.rate == SLIMBUS_SAMPLE_RATE_192K));

	if ((track == SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER) && is_fast_track) {
		track_config_da_combine_v5_table[track].params.channels = 1;
		AUDIO_LOGI("soundtrigger conflict");
		return -EPERM;
	}

	return ret;
}

static int32_t normal_scene_conflict_deactivate(int32_t *need_callback)
{
	int32_t ret = 0;

	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY)) {
		ret = slimbus_drv_track_deactivate(track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY].channel_pro,
					track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY].params.channels);

		slimbus_trackstate_set(SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY, false);
	}
	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG) &&
		(track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].params.channels == 2)) {
		*need_callback = 1;
		ret = slimbus_drv_track_deactivate(track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].channel_pro,
				track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].params.channels);

		slimbus_trackstate_set(SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG, false);
		track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].params.channels = 1;
	}

	return ret;
}

static int32_t process_normal_scene_conflict(enum slimbus_da_combine_v5_track track,
	bool track_enable, int32_t *need_callback)
{
	int32_t ret;

	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP) && (track == SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER) && track_enable)
		AUDIO_LOGI("st conflict");

	if ((track == SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY) && track_enable) {
		AUDIO_LOGI("direct play conflict");
		return -EPERM;
	}
	if ((track == SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG) && (track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].params.channels == 2)) {
		track_config_da_combine_v5_table[track].params.channels = 1;
		AUDIO_LOGI("debug conflict");
		return -EPERM;
	}

	ret = normal_scene_conflict_deactivate(need_callback);

	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER) &&
		(track == SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG)) {
		AUDIO_LOGI("st and debug conflict");
		return -EPERM;
	}
	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA) &&
		(track == SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG)) {
		AUDIO_LOGI("st and debug conflict");
		return -EPERM;
	}
	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG) && (track == SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA)) {
		AUDIO_LOGI("st and 4pa call conflict");
		return -EPERM;
	}
	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA) && (track == SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER)) {
		AUDIO_LOGI("st and sound trigger conflict");
		return -EPERM;
	}
	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER) && (track == SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA)) {
		AUDIO_LOGI("st and 4pa conflict");
		return -EPERM;
	}

	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG) && (track == SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER)) {
		*need_callback = 1;
		ret = slimbus_drv_track_deactivate(track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].channel_pro,
				track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG].params.channels);

		slimbus_trackstate_set(SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG, false);
	}
	ret = process_soundtrigger_params_conflict(track);

	return ret;
}

static int32_t process_other_scenes_conflict(enum slimbus_da_combine_v5_track track, int32_t *need_callback)
{
	int32_t ret;

	if (process_play_and_debug_conflict(track, need_callback)) {
		AUDIO_LOGI("debug conflict");
		return -EPERM;
	}

	ret = process_soundtrigger_params_conflict(track);

	return ret;
}

static int32_t slimbus_da_combine_v5_check_scenes(uint32_t track, uint32_t scenes, bool track_enable)
{
	uint32_t i;
	int32_t ret = 0;
	int32_t need_callback = 0;

	if (track >= SLIMBUS_DA_COMBINE_V5_TRACK_MAX || scenes >= SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX) {
		AUDIO_LOGE("param is invalid, track: %d, scenes: %d", track, scenes);
		return -EINVAL;
	}

	switch (scenes) {
	case SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_DIRECT_PLAY:
	case SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_DIRECT_PLAY_441:
		ret = process_other_scenes_conflict(track, &need_callback);
		break;
	case SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_NORMAL:
		ret = process_normal_scene_conflict(track, track_enable, &need_callback);
		break;
	default:
		break;
	}

	for (i = 0; i < SLIMBUS_DA_COMBINE_V5_TRACK_MAX; i++) {
		if ((slimbus_track_state_is_on(i) || (i == SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG)) &&
			track_config_da_combine_v5_table[i].params.callback && need_callback)
			track_config_da_combine_v5_table[i].params.callback(i, (void *)&(track_config_da_combine_v5_table[i].params));
	}

	return ret;
}

static bool is_play_scene(uint32_t active_tracks)
{
	switch (active_tracks) {
	case SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON:
		return true;
	default:
		break;
	}

	return false;
}

static bool is_call_only_scene(uint32_t active_tracks)
{
	switch (active_tracks) {
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON:
		return true;
	default:
		break;
	}

	return false;
}

static bool is_call_12288_scene(uint32_t active_tracks)
{
	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_4PA_IV))
		return false;

	switch (active_tracks) {
	case SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON |
		SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON |
		SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON:
	/* choose 1 from 4 with D3D4 */
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	/* choose 2 from 4 with D3D4 */
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	/* choose 3 from 4 with D3D4 */
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON |
		SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON |
		SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON |
		SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON |
		SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:
	/* choose 4 with D3D4 */
	case SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON |
		SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV_ON | SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4_ON:

		return true;
	default:
		break;
	}

	return false;
}

static bool is_enhance_soundtrigger_6144_scene(uint32_t active_tracks)
{
	switch (active_tracks) {
	case SLIMBUS_DA_COMBINE_V5_TRACK_KWS_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_KWS_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_KWS_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_KWS_ON | SLIMBUS_DA_COMBINE_V5_TRACK_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
		if (track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_KWS].params.rate != SLIMBUS_SAMPLE_RATE_16K) {
			AUDIO_LOGI("not enhance st scene");
			return false;
		}
		AUDIO_LOGI("enhance st scene");
		return true;
	default:
		break;
	}

	return false;
}

static bool is_direct_play(uint32_t active_tracks)
{
	uint32_t tmp;

	switch (active_tracks) {
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_CAPTURE_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_CAPTURE_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_CAPTURE_ON | SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON | SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_CAPTURE_ON |
	SLIMBUS_DA_COMBINE_V5_TRACK_EC_ON | SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER_ON:
		return true;
	default:
		tmp = (SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN_ON | SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_ON);
		if ((active_tracks & tmp) == tmp)
			return false;

		tmp = (SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG_ON | SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON);
		if ((active_tracks & tmp) == tmp)
			return true;

		tmp = (SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON | SLIMBUS_DA_COMBINE_V5_TRACK_KWS_ON);
		if ((active_tracks & tmp) == tmp)
			return true;

		break;
	}

	return false;
}

static bool is_soc_4pa_play_scene(uint32_t active_tracks)
{
	if (slimbus_track_state_is_on(SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA))
		return false;

	if (((active_tracks & SLIMBUS_DA_COMBINE_V5_TRACK_4PA_IV_ON) != 0) ||
		((active_tracks & SLIMBUS_DA_COMBINE_V5_TRACK_5MIC_CAPTURE_ON) != 0))
		return true;
	else
		return false;
}

static bool is_bt_sco_scene(uint32_t active_tracks)
{
	AUDIO_LOGI("bt sco scene: %u", active_tracks);

	if (((active_tracks & SLIMBUS_DA_COMBINE_V5_TRACK_BT_DOWN_ON) != 0) ||
		((active_tracks & SLIMBUS_DA_COMBINE_V5_TRACK_BT_UP_ON) != 0))
		return true;
	else
		return false;
}

static bool is_has_direct(uint32_t active_tracks)
{
	return ((active_tracks & SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON) == SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY_ON);
}

static bool is_direct_play_scene_44k1(uint32_t active_tracks)
{
	uint32_t rate = track_config_da_combine_v5_table[SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY].params.rate;

	if (!is_has_direct(active_tracks))
		return false;

	if (is_direct_play(active_tracks)) {
		if (rate == SLIMBUS_SAMPLE_RATE_44K1 || rate == SLIMBUS_SAMPLE_RATE_88K2 ||
			rate == SLIMBUS_SAMPLE_RATE_176K4) {
			return true;
		}
	}

	return false;
}

static bool is_direct_play_scene(uint32_t active_tracks)
{
	if (!is_has_direct(active_tracks))
		return false;

	if (is_direct_play(active_tracks))
		return true;

	return false;
}

static bool is_call_4pa_scene(uint32_t active_tracks)
{
	if ((active_tracks & SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA_ON) != 0)
		return true;
	else
		return false;
}

static struct select_scene_node scene_table_da_combine_v5[] = {
	{ SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_PLAY, SLIMBUS_CG_8, SLIMBUS_SM_4_CSW_32_SL, is_play_scene },
	{ SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_CALL, SLIMBUS_CG_8, SLIMBUS_SM_6_CSW_24_SL, is_call_only_scene },
	{ SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_CALL_12288, SLIMBUS_CG_9, SLIMBUS_SM_2_CSW_32_SL, is_call_12288_scene },
	{ SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_ENHANCE_ST_6144, SLIMBUS_CG_8, SLIMBUS_SM_4_CSW_32_SL, is_enhance_soundtrigger_6144_scene },
	{ SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_DIRECT_PLAY_441, SLIMBUS_CG_10, SLIMBUS_SM_2_CSW_32_SL, is_direct_play_scene_44k1 },
	{ SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_DIRECT_PLAY, SLIMBUS_CG_10, SLIMBUS_SM_2_CSW_32_SL, is_direct_play_scene },
	{ SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_4PA_CALL, SLIMBUS_CG_10, SLIMBUS_SM_2_CSW_32_SL, is_call_4pa_scene },
	{ SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_4PA_MOVEUP, SLIMBUS_CG_10, SLIMBUS_SM_2_CSW_32_SL, is_soc_4pa_play_scene },
	{ SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_BT_SCO, SLIMBUS_CG_10, SLIMBUS_SM_2_CSW_32_SL, is_bt_sco_scene },
};

static int32_t slimbus_da_combine_v5_select_scenes(struct slimbus_device_info *dev, uint32_t track,
		const struct slimbus_track_param *params, bool track_enable)
{
	enum slimbus_scene_config_da_combine_v5 scene_config_type = SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_NORMAL;
	enum slimbus_subframe_mode sm = SLIMBUS_SM_2_CSW_32_SL;
	enum slimbus_clock_gear cg = SLIMBUS_CG_10;
	uint32_t len = ARRAY_SIZE(scene_table_da_combine_v5);
	uint32_t active_tracks;
	int32_t ret;
	uint32_t i;

	if (!dev) {
		AUDIO_LOGE("dev address is null");
		return -EINVAL;
	}

	if (track >= SLIMBUS_DA_COMBINE_V5_TRACK_MAX) {
		AUDIO_LOGE("track is invalid, track: %d", track);
		return -EINVAL;
	}

	active_tracks = slimbus_trackstate_get();
	if (track_enable)
		active_tracks |= (1 << track);

	for (i = 0; i < len; i++) {
		if (scene_table_da_combine_v5[i].is_scene_matched(active_tracks)) {
			scene_config_type = scene_table_da_combine_v5[i].scene_type;
			cg = scene_table_da_combine_v5[i].cg;
			sm = scene_table_da_combine_v5[i].subframe_mode;
			break;
		}
	}

	ret = slimbus_da_combine_v5_check_scenes(track, scene_config_type, track_enable);
	if (ret) {
		AUDIO_LOGI("ret %d", ret);
		return ret;
	}

	if (dev->scene_config_type != scene_config_type) {
		AUDIO_LOGI("scene changed from %d to %d", dev->scene_config_type, scene_config_type);
		dev->cg = cg;
		dev->sm = sm;
		dev->scene_config_type = scene_config_type;
	}

	slimbus_da_combine_v5_param_update(dev, track, params);

	return 0;
}

static void slimbus_da_combine_v5_get_st_params(struct slimbus_sound_trigger_params *params)
{
	if (params != NULL) {
		params->sample_rate = SLIMBUS_SAMPLE_RATE_48K;
		params->channels = 2;
		params->track_type = SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE;
	}
}

static int32_t slimbus_da_combine_v5_track_soundtrigger_activate(uint32_t track, bool slimbus_dynamic_freq_enable,
	struct slimbus_device_info *dev, struct slimbus_track_param	*params)
{
	int32_t ret;
	struct slimbus_sound_trigger_params st_params;
	struct slimbus_track_param st_normal_params;

	if (track != SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER) {
		AUDIO_LOGE("track %d deactive is not soundtriger", track);
		return -EINVAL;
	}

	if (!dev) {
		AUDIO_LOGE("dev address is null");
		return -EINVAL;
	}

	memset(&st_params, 0, sizeof(st_params));
	memset(&st_normal_params, 0, sizeof(st_normal_params));

	slimbus_da_combine_v5_get_st_params(&st_params);
	st_normal_params.channels = st_params.channels;
	st_normal_params.rate = st_params.sample_rate;

	ret = slimbus_da_combine_v5_param_set(dev, st_params.track_type, &st_normal_params);
	ret += slimbus_da_combine_v5_param_set(dev, track, params);

	if (ret) {
		AUDIO_LOGE("slimbus device param set failed, ret: %d", ret);
		return ret;
	}

	/* request soc slimbus clk to 24.576M */
	slimbus_utils_freq_request();

	if (slimbus_dynamic_freq_enable) {
		ret = slimbus_drv_track_update(dev->cg, dev->sm, track, dev,
				track_config_da_combine_v5_table[track].params.channels, track_config_da_combine_v5_table[track].channel_pro);
		ret += slimbus_drv_track_update(dev->cg, dev->sm, st_params.track_type, dev,
				SLIMBUS_VOICE_UP_SOUNDTRIGGER, track_config_da_combine_v5_table[st_params.track_type].channel_pro);
	} else {
		ret = slimbus_drv_track_activate(track_config_da_combine_v5_table[track].channel_pro,
				track_config_da_combine_v5_table[track].params.channels);
		ret += slimbus_drv_track_activate(track_config_da_combine_v5_table[st_params.track_type].channel_pro,
				SLIMBUS_VOICE_UP_SOUNDTRIGGER);
	}

	return ret;

}

static int32_t slimbus_da_combine_v5_track_soundtrigger_deactivate(uint32_t track)
{
	int32_t ret;
	struct slimbus_sound_trigger_params st_params;

	if (track != SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER) {
		AUDIO_LOGE("track %d deactive is not soundtriger", track);
		return -1;
	}

	memset(&st_params, 0, sizeof(st_params));

	slimbus_da_combine_v5_get_st_params(&st_params);

	/* release soc slimbus clk to 21.777M */
	slimbus_utils_freq_release();
	ret = slimbus_drv_track_deactivate(track_config_da_combine_v5_table[track].channel_pro,
			track_config_da_combine_v5_table[track].params.channels);
	ret += slimbus_drv_track_deactivate(track_config_da_combine_v5_table[st_params.track_type].channel_pro,
			SLIMBUS_VOICE_UP_SOUNDTRIGGER);

	return ret;
}

static bool slimbus_da_combine_v5_track_is_fast_soundtrigger(uint32_t track)
{
	if (track != SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER)
		return false;

	return ((track == SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER) &&
		(track_config_da_combine_v5_table[track].params.rate == SLIMBUS_SAMPLE_RATE_192K));
}

static struct slimbus_track_config *slimbus_da_combine_v5_get_track_config_table(void)
{
	return track_config_da_combine_v5_table;
}

void slimbus_da_combine_v5_callback_register(struct slimbus_device_ops *dev_ops,
	struct slimbus_private_data *pd)
{
	if (dev_ops == NULL || pd == NULL) {
		AUDIO_LOGE("input param is null");
		return;
	}

	dev_ops->create_slimbus_device = slimbus_da_combine_v5_create_device;
	dev_ops->slimbus_device_param_init = slimbus_da_combine_v5_param_init;
	dev_ops->slimbus_device_param_update = slimbus_da_combine_v5_param_update;
	dev_ops->slimbus_device_param_set = slimbus_da_combine_v5_param_set;
	dev_ops->slimbus_track_soundtrigger_activate = slimbus_da_combine_v5_track_soundtrigger_activate;
	dev_ops->slimbus_track_soundtrigger_deactivate = slimbus_da_combine_v5_track_soundtrigger_deactivate;
	dev_ops->slimbus_track_is_fast_soundtrigger = slimbus_da_combine_v5_track_is_fast_soundtrigger;
	dev_ops->slimbus_get_soundtrigger_params = slimbus_da_combine_v5_get_st_params;
	dev_ops->slimbus_check_scenes = slimbus_da_combine_v5_check_scenes;
	dev_ops->slimbus_select_scenes = slimbus_da_combine_v5_select_scenes;

	pd->track_config_table = slimbus_da_combine_v5_get_track_config_table();
	pd->slimbus_track_max = (uint32_t)SLIMBUS_DA_COMBINE_V5_TRACK_MAX;
}

