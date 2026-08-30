/*
 *	slimbus is a kernel driver which is used to manager SLIMbus devices
 *	Copyright (C) 2014	Hisilicon

 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.

 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.

 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>

#include "slimbus_utils.h"
#include "csmi.h"
#include "slimbus_drv.h"
#include "slimbus.h"
#include "slimbus_64xx.h"
#include "slimbus_6405.h"

#include <dsm/dsm_pub.h>

/*lint -e750 -e730 -e785 -e574*/
#define LOG_TAG "Slimbus_6405"

/*lint -e838 -e730 -e747 -e774 -e826 -e529 -e438 -e485 -e785 -e651 -e64 -e527 -e570*/
slimbus_channel_property_t audio_playback_6405[SLIMBUS_AUDIO_PLAYBACK_CHANNELS] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t audio_capture_6405[SLIMBUS_AUDIO_CAPTURE_MULTI_MIC_CHANNELS] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t voice_down_6405[SLIMBUS_VOICE_DOWN_CHANNELS] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t voice_up_6405[SLIMBUS_VOICE_UP_CHANNELS] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t ec_ref_6405[SLIMBUS_ECREF_1CH] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t sound_trigger_6405[SLIMBUS_SOUND_TRIGGER_CHANNELS] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t audio_debug_6405[SLIMBUS_DEBUG_CHANNELS] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t audio_direct_playback_6405[SLIMBUS_AUDIO_PLAYBACK_CHANNELS] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t audio_soc_2pa_iv_6405[SLIMBUS_AUDIO_PLAYBACK_CHANNELS] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t audio_soc_4pa_iv_6405[SLIMBUS_AUDIO_PLAYBACK_MULTI_PA_CHANNELS] = {{0,{0,},{{0,},},},};
slimbus_channel_property_t audio_soc_playback_d3d4_6405[SLIMBUS_AUDIO_PLAYBACK_CHANNELS] = {{0, {0,}, {{0,},},},};

slimbus_track_config_t track_config_6405_table[SLIMBUS_6405_TRACK_MAX] = {
	/* play back */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_playback_6405[0],
	},
	/* capture */
	{
		.params.channels = SLIMBUS_AUDIO_CAPTURE_MULTI_MIC_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_capture_6405[0],
	},
	/* voice down */
	{
		.params.channels = SLIMBUS_VOICE_DOWN_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_8K,
		.params.callback = NULL,
		.channel_pro = &voice_down_6405[0],
	},
	/* voice up */
	{
		.params.channels = SLIMBUS_VOICE_UP_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_8K,
		.params.callback = NULL,
		.channel_pro = &voice_up_6405[0],
	},
	/* ec */
	{
		.params.channels = SLIMBUS_ECREF_1CH,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &ec_ref_6405[0],
	},
	/* sound trigger */
	{
		.params.channels = SLIMBUS_SOUND_TRIGGER_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_192K,
		.params.callback = NULL,
		.channel_pro = &sound_trigger_6405[0],
	},
	/* debug */
	{
		.params.channels = SLIMBUS_DEBUG_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_192K,
		.params.callback = NULL,
		.channel_pro = &audio_debug_6405[0],
	},
	/* direct play */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_192K,
		.params.callback = NULL,
		.channel_pro = &audio_direct_playback_6405[0],
	},
	/* 2pa play in soc */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_soc_2pa_iv_6405[0],
	},
	/* 4pa play in soc */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_MULTI_PA_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_soc_4pa_iv_6405[0],
	},
	/* d3d4 playback */
	{
		.params.channels = SLIMBUS_AUDIO_PLAYBACK_CHANNELS,
		.params.rate = SLIMBUS_SAMPLE_RATE_48K,
		.params.callback = NULL,
		.channel_pro = &audio_soc_playback_d3d4_6405[0],
	},
};

enum {
	SLIMBUS_6405_TRACK_PLAY_ON                   = 1 << SLIMBUS_6405_TRACK_AUDIO_PLAY,
	SLIMBUS_6405_TRACK_CAPTURE_ON                = 1 << SLIMBUS_6405_TRACK_AUDIO_CAPTURE,
	SLIMBUS_6405_TRACK_VOICE_DOWN_ON             = 1 << SLIMBUS_6405_TRACK_VOICE_DOWN,
	SLIMBUS_6405_TRACK_VOICE_UP_ON               = 1 << SLIMBUS_6405_TRACK_VOICE_UP,
	SLIMBUS_6405_TRACK_EC_ON                     = 1 << SLIMBUS_6405_TRACK_ECREF,
	SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON          = 1 << SLIMBUS_6405_TRACK_SOUND_TRIGGER,
	SLIMBUS_6405_TRACK_DEBUG_ON                  = 1 << SLIMBUS_6405_TRACK_DEBUG,
	SLIMBUS_6405_TRACK_DIRECT_PLAY_ON            = 1 << SLIMBUS_6405_TRACK_DIRECT_PLAY,
	SLIMBUS_6405_TRACK_2PA_IV_ON                 = 1 << SLIMBUS_6405_TRACK_2PA_IV,
	SLIMBUS_6405_TRACK_4PA_IV_ON                 = 1 << SLIMBUS_6405_TRACK_4PA_IV,
	SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON        = 1 << SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4,
};

/* segment length */
static uint16_t sl_table[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_6405_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			/* 6.144M FPGA*/
			{
				{6, 6},                                         /* audio playback */
				{4, 4},                                         /* audio capture */
				{4, 4},                                         /* voice down */
				{4, 4, 4, 4},                                   /* voice up */
				{0},                                            /* EC_REF */
				{0},                                            /* sound trigger */
				{0},                                            /* debug */
				{0, 0},                                         /* direct play */
				{0, 0},                                         /* 2PA IV */
				{0, 0, 0, 0},                                   /* 4PA IV */
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 6.144M play*/
			{
				{6, 6},                                         /* audio playback */
				{0, 0},                                         /* audio capture */
				{0, 0},                                         /* voice down */
				{0, 0, 0, 0},                                   /* voice up */
				{8},                                            /* EC_REF */
				{0},                                            /* sound trigger */
				{0},                                            /* debug */
				{0, 0},                                         /* direct play*/
				{8, 8},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 6.144M call*/
			{
				{0, 0},                                         /* audio playback */
				{0, 0},                                         /* audio capture */
				{6, 6},                                         /* voice down */
				{6, 6, 6, 6},                                   /* voice up */
				{0},                                            /* EC_REF */
				{0},                                            /* sound trigger */
				{0},                                            /* debug */
				{0, 0},                                         /* direct play*/
				{0, 0},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 24.576M normal*/
			{
				{6, 6},                                         /* audio playback */
				{6, 6, 6, 6},                                   /* audio capture */
				{6, 6},                                         /* voice down */
				{6, 6, 6, 6},                                   /* voice up */
				{8},                                            /* EC_REF */
				{8},                                            /* sound trigger */
				{8},                                            /* debug */
				{0, 0},                                         /* direct play*/
				{8, 8},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{4, 4},                                         /* PLAY D3D4 */
			},
			/* 24.576M direct play*/
			{
				{0, 0},                                         /* audio playback */
				{6, 6, 6, 6},                                   /* audio capture */
				{0, 0},                                         /* voice down */
				{0, 0, 0, 0},                                   /* voice up */
				{0},                                            /* EC_REF */
				{8},                                            /* sound trigger */
				{0},                                            /* debug */
				{6, 6},                                         /* direct play */
				{0, 0},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 12.288M call*/
			{
				{6, 6},                                         /* audio playback */
				{0, 0},                                         /* audio capture */
				{6, 6},                                         /* voice down */
				{6, 6, 6, 6},                                   /* voice up */
				{8},                                            /* EC_REF */
				{0},                                            /* sound trigger */
				{0},                                            /* debug */
				{0, 0},                                         /* direct play*/
				{8, 8},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{4, 4},                                         /* PLAY D3D4 */
			},
			/* 6.144M soundtrigger enhance*/
			{
				{6, 6},                                         /* audio playback */
				{0, 0},                                         /* audio capture */
				{0, 0},                                         /* voice down */
				{0, 0, 0, 0},                                   /* voice up */
				{8},                                            /* EC_REF */
				{4, 4},                                         /* sound trigger */
				{0},                                            /* debug */
				{0, 0},                                         /* direct play*/
				{0, 0},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 4PA play in soc */
			{
				{4, 4},                                         /* audio playback */
				{6, 6, 6, 6},                                   /* audio capture */
				{4, 4},                                         /* voice down */
				{0, 0, 0, 0},                                   /* voice up */
				{0},                                            /* EC_REF */
				{0, 0},                                         /* sound trigger */
				{8},                                            /* debug */
				{0, 0},                                         /* direct play*/
				{0, 0},                                         /* 2PA IV*/
				{8, 8, 8, 8},                                   /* 4PA IV*/
				{4, 4},                                         /* PLAY D3D4 */
			},
};

static uint16_t sl_soundtrigger_16k[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0},                                                    /* 6.144M fpga */
			{0},                                                    /* 6.144M play */
			{0},                                                    /* 6.144M call */
			{4, 4},                                                 /* 24.576M normal */
			{4, 4},                                                 /* 24.576M direct play */
			{0},                                                    /* 12.288M call */
			{4, 4},                                                 /* 6.144M soundtrigger enhance */
};

/* presence rate */
static slimbus_presence_rate_t pr_table[SLIMBUS_6405_TRACK_MAX] = {
			SLIMBUS_PR_48K, SLIMBUS_PR_48K,
			SLIMBUS_PR_8K, SLIMBUS_PR_8K,
			SLIMBUS_PR_48K, SLIMBUS_PR_192K,
			SLIMBUS_PR_192K, SLIMBUS_PR_192K,
			SLIMBUS_PR_48K, SLIMBUS_PR_48K,
			SLIMBUS_PR_48K,
};

/* channel index */
static uint8_t cn_table[SLIMBUS_6405_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			{AUDIO_PLAY_CHANNEL_LEFT, AUDIO_PLAY_CHANNEL_RIGHT},
			{AUDIO_CAPTURE_CHANNEL_LEFT, AUDIO_CAPTURE_CHANNEL_RIGHT, VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4},
			{VOICE_DOWN_CHANNEL_LEFT, VOICE_DOWN_CHANNEL_RIGHT},
			{VOICE_UP_CHANNEL_MIC1, VOICE_UP_CHANNEL_MIC2, VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4},
			{VOICE_ECREF},
			{SOUND_TRIGGER_CHANNEL_LEFT, SOUND_TRIGGER_CHANNEL_RIGHT},
			{DEBUG_LEFT, DEBUG_RIGHT},
			{AUDIO_PLAY_CHANNEL_LEFT, AUDIO_PLAY_CHANNEL_RIGHT},
			{VOICE_ECREF, AUDIO_ECREF},
			{VOICE_ECREF, AUDIO_ECREF, VOICE_UP_CHANNEL_MIC1, VOICE_UP_CHANNEL_MIC2},
			{AUDIO_PLAY_CHANNEL_D3, AUDIO_PLAY_CHANNEL_D4},
};

/* source port number */
static uint8_t source_pn_table[SLIMBUS_6405_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			{AUDIO_PLAY_SOC_LEFT_PORT, AUDIO_PLAY_SOC_RIGHT_PORT},
			{AUDIO_CAPTURE_64XX_LEFT_PORT, AUDIO_CAPTURE_64XX_RIGHT_PORT, VOICE_UP_64XX_MIC3_PORT, VOICE_UP_64XX_MIC4_PORT},
			{VOICE_DOWN_SOC_LEFT_PORT, VOICE_DOWN_SOC_RIGHT_PORT},
			{VOICE_UP_64XX_MIC1_PORT, VOICE_UP_64XX_MIC2_PORT, VOICE_UP_64XX_MIC3_PORT, VOICE_UP_64XX_MIC4_PORT},
			{VOICE_64XX_ECREF_PORT},
			{VOICE_UP_64XX_MIC1_PORT, VOICE_UP_64XX_MIC2_PORT},
			{BT_CAPTURE_64XX_LEFT_PORT, BT_CAPTURE_64XX_RIGHT_PORT},
			{AUDIO_PLAY_SOC_LEFT_PORT, AUDIO_PLAY_SOC_RIGHT_PORT},
			{VOICE_64XX_ECREF_PORT, AUDIO_64XX_ECREF_PORT},
			{VOICE_64XX_ECREF_PORT, AUDIO_64XX_ECREF_PORT, VOICE_UP_64XX_MIC1_PORT, VOICE_UP_64XX_MIC2_PORT},
			{AUDIO_PLAY_SOC_D3_PORT, AUDIO_PLAY_SOC_D4_PORT},
};

/* sink port number */
static uint8_t sink_pn_table[SLIMBUS_6405_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			{AUDIO_PLAY_64XX_LEFT_PORT, AUDIO_PLAY_64XX_RIGHT_PORT},
			{AUDIO_CAPTURE_SOC_LEFT_PORT, AUDIO_CAPTURE_SOC_RIGHT_PORT, VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT},
			{VOICE_DOWN_64XX_LEFT_PORT, VOICE_DOWN_64XX_RIGHT_PORT},
			{VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT, VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT},
			{VOICE_SOC_ECREF_PORT},
			{VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT},
			{BT_CAPTURE_SOC_LEFT_PORT, BT_CAPTURE_SOC_RIGHT_PORT},
			{AUDIO_PLAY_64XX_LEFT_PORT, AUDIO_PLAY_64XX_RIGHT_PORT},
			{VOICE_SOC_ECREF_PORT, AUDIO_SOC_ECREF_PORT},
			{VOICE_SOC_ECREF_PORT, AUDIO_SOC_ECREF_PORT, VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT},
			{AUDIO_PLAY_64XX_D3_PORT, AUDIO_PLAY_64XX_D4_PORT},
};

/* segment distribution */
static uint16_t sd_table[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_6405_TRACK_MAX][SLIMBUS_CHANNELS_MAX] ={
			/* 6.144M FPGA*/
			{
				{0xc24, 0xc2a},                                 /* audio playback */
				{0xc30, 0xc34},                                 /* audio capture */
				{0x58, 0x5c},                                   /* voice down 8k */
				{0x78, 0x7c, 0x458, 0x45C},                     /* voice up 8k */
				{0},                                            /* EC_REF */
				{0},                                            /* sound trigger 192K*/
				{0},                                            /* debug 192K*/
				{0},                                            /* direct play 192k*/
				{0, 0},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 6.144M play */
			{
				{0xC24, 0xC2A},                                 /* audio playback */
				{0, 0},                                         /* audio capture */
				{0, 0},                                         /* voice down 8k */
				{0, 0, 0, 0},                                   /* voice up 8k */
				{0xC30},                                        /* EC_REF */
				{0},                                            /* sound trigger 192K*/
				{0},                                            /* debug 192K*/
				{0},                                            /* direct play 192k*/
				{0xC30, 0xC38},                                 /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 6.144M call*/
			{
				{0, 0},                                         /* audio playback */
				{0, 0},                                         /* audio capture */
				{0x46, 0x5E},                                   /* voice down 8k */
				{0x4C, 0x52, 0x64, 0x6A},                       /* voice up 8k */
				{0},                                            /* EC_REF */
				{0},                                            /* sound trigger 192K*/
				{0},                                            /* debug 192K*/
				{0},                                            /* direct play 192k*/
				{0, 0},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 24.576M normal */
			{
				{0xCC6, 0xCE6},                                 /* audio playback */
				{0xC8C, 0xCAC, 0xCCC, 0xCEC},                   /* audio capture */
				{0x112, 0x132},                                 /* voice down 8k */
				{0x152, 0x172, 0x192, 0x1B2},                   /* voice up 8k */
				{0xC84},                                        /* EC_REF */
				{0xC38},                                        /* sound trigger 192K*/
				{0xC38},                                        /* debug 192K*/
				{0},                                            /* direct play 192k*/
				{0xc84, 0xca4},                                 /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0xCC2, 0xCE2},                                 /* PLAY D3D4 */
			},
			/*24.576M direct play*/
			{
				{0, 0},                                         /* audio playback */
				{0xC8A, 0xCAA, 0xCCA, 0xCEA},                   /* audio capture */
				{0, 0},                                         /* voice down 8k */
				{0, 0, 0, 0},                                   /* voice up 8k */
				{0},                                            /* EC_REF */
				{0xC22},                                        /* sound trigger 192K */
				{0},                                            /* debug 192K */
				{0xC30,0xC36},                                  /* direct play 192k */
				{0, 0},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 12.288M call*/
			{
				{0xC46, 0xC66},                                 /* audio playback */
				{0, 0},                                         /* audio capture */
				{0xCC, 0xD2},                                   /* voice down 8k */
				{0x8C, 0x92, 0xAC, 0xB2},                       /* voice up 8k */
				{0xC58},                                        /* EC_REF */
				{0},                                            /* sound trigger 192K*/
				{0},                                            /* debug 192K*/
				{0},                                            /* direct play 192k*/
				{0xC58, 0xC78},                                 /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0xC42, 0xC62},                                 /* PLAY D3D4 */
			},
			/* 6.144M soundtrigger enhance*/
			{
				{0xC28, 0xC2E},                                 /* audio playback */
				{0, 0},                                         /* audio capture */
				{0, 0},                                         /* voice down */
				{0, 0, 0, 0},                                   /* voice up */
				{0xC34},                                        /* EC_REF */
				{0x24, 0x424},                                  /* sound trigger */
				{0},                                            /* debug */
				{0, 0},                                         /* direct play*/
				{0, 0},                                         /* 2PA IV*/
				{0, 0, 0, 0},                                   /* 4PA IV*/
				{0, 0},                                         /* PLAY D3D4 */
			},
			/* 4PA play in soc */
			{
				{0xC94, 0xCB4},                                 /* audio playback */
				{0xC8E, 0xCAE, 0xCCE, 0xCEE},                   /* audio capture */
				{0xCD4, 0xCF4},                                 /* voice down 48k */
				{0, 0, 0, 0},                                   /* voice up 8k */
				{0},                                            /* EC_REF */
				{0},                                            /* sound trigger 192K*/
				{0xC38},                                        /* debug 192K*/
				{0},                                            /* direct play 192k*/
				{0, 0},                                         /* 2PA IV*/
				{0xC86, 0xCA6, 0xCC6, 0xCE6},                   /* 4PA IV*/
				{0xCC2, 0xCE2},                                 /* PLAY D3D4 */
			},
};

static uint16_t sd_voice_down_6405_16k[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0x38, 0x3C},                                       /* 6.144M FPGA voice down 16k */
			{0, 0},                                             /* 6.144M play */
			{0x26, 0x3E},                                       /* 6.144M call */
			{0x92, 0xB2},                                       /* 24.576M normal */
			{0, 0},                                             /* 24.576M direct play */
			{0x44C, 0x452},                                     /* 12.288M call */
			{0, 0},                                             /* 6.144M soundtrigger enhance */
			{0, 0}                                              /* 24.579M PA IV */
};

static uint16_t sd_voice_up_6405_16k[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0x438, 0x43c, 0x838, 0x83c},                       /* 6.144M FPGA voice up 16k */
			{0, 0, 0, 0},                                       /* 6.144M play */
			{0x2C, 0x32, 0x424, 0x42A},                         /* 6.144M call */
			{0xD2, 0xF2, 0x492, 0x4B2},                         /* 24.576M normal */
			{0, 0, 0, 0},                                       /* 24.576M direct play */
			{0x4C, 0x52, 0x6C, 0x72},                           /* 12.288M call */
			{0, 0, 0, 0},                                       /* 6.144M soundtrigger enhance */
			{0, 0, 0, 0}                                        /* 24.579M 4PA IV */
};

static uint16_t sd_voice_down_6405_32k[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0, 0},                                             /* 6.144M FPGA voice down 32k */
			{0, 0},                                             /* 6.144M play */
			{0x16, 0x41E},                                      /* 6.144M call */
			{0x52, 0x72},                                       /* 24.576M normal */
			{0, 0},                                             /* 24.576M direct play */
			{0x82C, 0x832},                                     /* 12.288M call */
			{0, 0},                                             /* 6.144M soundtrigger enhance */
			{0, 0}                                              /* 24.579M 4PA IV */
};

static uint16_t sd_voice_up_6405_32k[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0, 0, 0, 0},                                       /* 6.144M FPGA voice up 32k*/
			{0, 0, 0, 0},                                       /* 6.144M PLAY */
			{0x1C, 0x412, 0x814, 0x81A},                        /* 6.144M call */
			{0x452, 0x472, 0x852, 0x872},                       /* 24.576M normal */
			{0, 0, 0, 0},                                       /* 24.576M direct play */
			{0x2C, 0x32, 0x42C, 0x432},                         /* 12.288M call */
			{0, 0, 0, 0},                                       /* 6.144M soundtrigger enhance */
			{0, 0, 0 , 0}                                       /* 24.579M 4PA IV */
};

static uint16_t sd_soundtrigger_48k[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0},                                                /* 6.144M FPGA */
			{0},                                                /* 6.144M play */
			{0},                                                /* 6.144M call */
			{0xC98},                                            /* 24.576M normal */
			{0xC82},                                            /* 24.576M direct play */
			{0},                                                /* 12.288M call */
			{0},                                                /* 6.144M soundtrigger enhance */
			{0, 0}                                              /* 24.579M PA IV */
};

static uint16_t sd_soundtrigger_16k[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0},                                                /* 6.144M fpga */
			{0},                                                /* 6.144M play */
			{0},                                                /* 6.144M call */
			{0x98, 0xB8},                                       /* 24.576M normal */
			{0x82, 0xA2},                                       /* 24.576M direct play */
			{0},                                                /* 12.288M call */
			{0x24, 0x424},                                      /* 6.144M soundtrigger enhance */
			{0, 0}                                              /* 24.579M PA IV */
};

static uint16_t sd_direct_play_96k[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0, 0},                                             /* 6.144M FPGA voice down 16k */
			{0, 0},                                             /* 6.144M play */
			{0, 0},                                             /* 6.144M call */
			{0, 0},                                             /* 24.576M normal */
			{0xC50, 0xC56},                                     /* 24.576M direct play */
			{0, 0},                                             /* 12.288M call */
			{0, 0},                                             /* 6.144M soundtrigger enhance */
			{0, 0}                                              /* 24.579M PA IV */
};
static uint16_t sd_direct_play_48k[SLIMBUS_6405_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{0, 0},                                             /* 6.144M FPGA voice down 16k */
			{0, 0},                                             /* 6.144M play */
			{0, 0},                                             /* 6.144M call */
			{0, 0},                                             /* 24.576M normal */
			{0xC90, 0xC96},                                     /* 24.576M direct play */
			{0, 0},                                             /* 12.288M call */
			{0, 0},                                             /* 6.144M soundtrigger enhance */
			{0, 0}                                              /* 24.579M PA IV */
};

static void slimbus_hi6405_set_direct_play_sd(
					uint32_t scene_type,
					uint32_t ch,
					slimbus_presence_rate_t pr,
					uint16_t *sd)
{
	if (pr == SLIMBUS_PR_96K) {
		*sd = sd_direct_play_96k[scene_type][ch];
	} else if (pr == SLIMBUS_PR_48K) {
		*sd = sd_direct_play_48k[scene_type][ch];
	}
}

static void slimbus_hi6405_set_para_sd(
					uint32_t  track_type,
					uint32_t scene_type,
					uint32_t ch,
					slimbus_presence_rate_t pr,
					uint16_t *sd)
{
	*sd = sd_table[scene_type][track_type][ch];

	switch(track_type) {
	case SLIMBUS_6405_TRACK_VOICE_DOWN:
		if (pr == SLIMBUS_PR_16K) {
			*sd = sd_voice_down_6405_16k[scene_type][ch];
		}
		if (pr == SLIMBUS_PR_32K) {
			*sd = sd_voice_down_6405_32k[scene_type][ch];
		}
		break;
	case SLIMBUS_6405_TRACK_VOICE_UP:
		if (pr == SLIMBUS_PR_16K) {
			*sd = sd_voice_up_6405_16k[scene_type][ch];
		}
		if (pr == SLIMBUS_PR_32K) {
			*sd = sd_voice_up_6405_32k[scene_type][ch];
		}
		break;
	case SLIMBUS_6405_TRACK_SOUND_TRIGGER:
		if (pr == SLIMBUS_PR_16K) {
			*sd = sd_soundtrigger_16k[scene_type][ch];
		}
		if (pr == SLIMBUS_PR_48K) {
			*sd = sd_soundtrigger_48k[scene_type][ch];
		}
		break;
	case SLIMBUS_6405_TRACK_DIRECT_PLAY:
		slimbus_hi6405_set_direct_play_sd(scene_type, ch, pr, sd);
		break;
	default:
		break;
	}

	return;
}

static void slimbus_hi6405_param_update_sl(
				int   track_type,
				slimbus_scene_config_type_t scene_type,
				uint32_t ch,
				slimbus_presence_rate_t pr,
				uint16_t *sl)
{
	if (track_type >= (int)SLIMBUS_6405_TRACK_MAX) {
		pr_err("track type is invalid, track_type:%d\n", track_type);
		return;
	}

	*sl = sl_table[scene_type][track_type][ch];

	switch(track_type) {
	case SLIMBUS_6405_TRACK_SOUND_TRIGGER:
		if (pr == SLIMBUS_PR_16K) {
			*sl = sl_soundtrigger_16k[scene_type][ch];
		}
		break;
	default:
		break;
	}

	return;
}

static void slimbus_hi6405_get_params_la(int track_type, uint8_t *source_la,
			uint8_t *sink_la, slimbus_transport_protocol_t *tp)
{
	switch (track_type) {
	case SLIMBUS_6405_TRACK_AUDIO_PLAY:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY:
	case SLIMBUS_6405_TRACK_VOICE_DOWN:
	case SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4:
		break;

	case SLIMBUS_6405_TRACK_AUDIO_CAPTURE:
	case SLIMBUS_6405_TRACK_VOICE_UP:
	case SLIMBUS_6405_TRACK_ECREF:
	case SLIMBUS_6405_TRACK_SOUND_TRIGGER:
	case SLIMBUS_6405_TRACK_DEBUG:
	case SLIMBUS_6405_TRACK_2PA_IV:
	case SLIMBUS_6405_TRACK_4PA_IV:
		*source_la = HI64XX_LA_GENERIC_DEVICE;
		*sink_la = SOC_LA_GENERIC_DEVICE;
		break;

	default:
		pr_err("[%s:%d] track type is invalid: %d\n", __FUNCTION__, __LINE__, track_type);
		return;
	}

	return;
}

/*
 * create hi6405 slimbus device
 * @device, pointer to created instance
 *
 * return 0, if success
 */
int create_hi6405_slimbus_device(slimbus_device_info_t **device)
{
	struct slimbus_device_info *dev_info = NULL;

	dev_info = kzalloc(sizeof(struct slimbus_device_info), GFP_KERNEL);
	if (!dev_info) {
		pr_err("[%s:%d] malloc slimbus failed!\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}
	dev_info->slimbus_64xx_para = kzalloc(sizeof(struct slimbus_device_info), GFP_KERNEL);
	if (!dev_info->slimbus_64xx_para) {
		pr_err("[%s:%d] malloc slimbus failed!\n", __FUNCTION__, __LINE__);
		kfree(dev_info);
		dev_info = NULL;
		return -ENOMEM;
	}

	dev_info->generic_la = HI64XX_LA_GENERIC_DEVICE;
	dev_info->voice_up_chnum = SLIMBUS_VOICE_UP_2CH;;
	dev_info->audio_up_chnum = SLIMBUS_AUDIO_CAPTURE_MULTI_MIC_CHANNELS;//default is 4mic
	dev_info->page_sel_addr = 1;
	dev_info->sm = SLIMBUS_SM_1_CSW_32_SL;
	dev_info->cg = SLIMBUS_CG_10;
	dev_info->scene_config_type = SLIMBUS_6405_SCENE_CONFIG_NORMAL;

	dev_info->slimbus_64xx_para->slimbus_track_max = SLIMBUS_6405_TRACK_MAX;
	dev_info->slimbus_64xx_para->track_config_table = track_config_6405_table;

	mutex_init(&dev_info->rw_mutex);
	mutex_init(&dev_info->track_mutex);

	*device = dev_info;

	return 0;
}

void slimbus_hi6405_param_init(slimbus_device_info_t *dev)
{
	uint32_t track = 0;
	uint32_t ch = 0;
	int i = 0;
	slimbus_transport_protocol_t tp = SLIMBUS_TP_ISOCHRONOUS;
	uint8_t source_la = SOC_LA_GENERIC_DEVICE;
	uint8_t sink_la = HI64XX_LA_GENERIC_DEVICE;
	uint8_t (*chnum_table)[SLIMBUS_CHANNELS_MAX];
	uint8_t (*source_pn)[SLIMBUS_CHANNELS_MAX];
	uint8_t (*sink_pn)[SLIMBUS_CHANNELS_MAX];
	slimbus_channel_property_t *pchprop = NULL;
	uint32_t scene_type;

	if (NULL == dev) {
		pr_err("dev is null \n");
		return;
	}
	if (dev->scene_config_type >= SLIMBUS_6405_SCENE_CONFIG_MAX) {
		pr_err("[%s:%d] track type is invalid, scene type:%d\n", __FUNCTION__, __LINE__,
			dev->scene_config_type);
		return;
	}

	scene_type = dev->scene_config_type;
	chnum_table = cn_table;
	source_pn = source_pn_table;
	sink_pn = sink_pn_table;

	for (track = 0; track < SLIMBUS_6405_TRACK_MAX; track++) {
		tp = SLIMBUS_TP_ISOCHRONOUS;
		source_la = SOC_LA_GENERIC_DEVICE;
		sink_la = HI64XX_LA_GENERIC_DEVICE;

		slimbus_hi6405_get_params_la(track, &source_la, &sink_la, &tp);

		pchprop = track_config_6405_table[track].channel_pro;

		for (ch = 0; ch < track_config_6405_table[track].params.channels; ch++) {
			memset(pchprop, 0, sizeof(slimbus_channel_property_t));

			pchprop->cn = *(*(chnum_table + track) + ch);
			pchprop->source.la = source_la;
			pchprop->source.pn = *(*(source_pn + track) + ch);
			pchprop->sink_num = 1;
			for (i = 0; i < pchprop->sink_num; i++) {
				pchprop->sinks[i].la = sink_la;
				pchprop->sinks[i].pn = *(*(sink_pn + track) + ch);
			}
			if (SLIMBUS_6405_TRACK_ECREF == track) {
				pchprop->sink_num = 2;
				pchprop->sinks[1].la = sink_la;
				pchprop->sinks[1].pn = BT_CAPTURE_SOC_RIGHT_PORT;

				if (!ch) {
					pchprop->sinks[1].pn  = IMAGE_DOWNLOAD_SOC_PORT;
				}
			}

			pchprop->tp = tp;
			pchprop->fl = 0;
			pchprop->af = SLIMBUS_AF_NOT_APPLICABLE;
			pchprop->dt = SLIMBUS_DF_NOT_INDICATED;
			pchprop->cl = 0;
			pchprop->pr = pr_table[track];

			pchprop->sl = sl_table[scene_type][track][ch];

			slimbus_hi6405_set_para_sd(track,
				scene_type, ch, pchprop->pr, &(pchprop->sd));
			slimbus_hi6405_param_update_sl(track,
				scene_type, ch, pchprop->pr, &(pchprop->sl));

			if (SLIMBUS_TP_PUSHED == tp) {
				pchprop->dl = pchprop->sl - 1;
			} else {
				pchprop->dl = pchprop->sl;
			}

			pchprop++;
		}
	}

	return;
}

int slimbus_hi6405_param_set(
			slimbus_device_info_t *dev,
			uint32_t track_type,
			slimbus_track_param_t *params)
{
	slimbus_channel_property_t *pchprop = NULL;
	uint32_t scene_type = SLIMBUS_6405_SCENE_CONFIG_MAX;
	slimbus_track_param_t slimbus_param;
	unsigned int ch = 0;

	if (!dev) {
		pr_err("[%s:%d] dev is NULL\n", __FUNCTION__, __LINE__);
		return -EINVAL;
	}
	if (track_type >= SLIMBUS_6405_TRACK_MAX) {
		pr_err("[%s:%d] track type is invalid, track_type:%d\n", __FUNCTION__, __LINE__,track_type);
		return -EINVAL;
	}
	if (dev->scene_config_type >= SLIMBUS_6405_SCENE_CONFIG_MAX) {
		pr_err("[%s:%d] scene type is invalid, scene type:%d\n", __FUNCTION__, __LINE__,
			dev->scene_config_type);
		return -EINVAL;
	}

	scene_type = dev->scene_config_type;

	if (!params) {
		memcpy(&slimbus_param, &(track_config_6405_table[track_type].params), sizeof(slimbus_param));
		params = &slimbus_param;
	}
	pr_info("[%s:%d] track type:%d, rate:%d, chnum:%d\n", __FUNCTION__, __LINE__, track_type, params->rate, params->channels);

	track_config_6405_table[track_type].params.channels = params->channels;
	track_config_6405_table[track_type].params.rate = params->rate;
	track_config_6405_table[track_type].params.callback = params->callback;

	pchprop = track_config_6405_table[track_type].channel_pro;/*lint!e838*/

	if (SLIMBUS_6405_TRACK_VOICE_UP == track_type) {
		dev->voice_up_chnum = params->channels;
	} else if (SLIMBUS_6405_TRACK_AUDIO_CAPTURE == track_type) {
		dev->audio_up_chnum = params->channels;
	} else if (SLIMBUS_6405_TRACK_VOICE_DOWN == track_type) {
		track_config_6405_table[track_type].params.channels = 2;
	} else {
		//do nothing
	}

	if (pchprop) {
		for (ch = 0; ch < track_config_6405_table[track_type].params.channels; ch++) {

			slimbus_hi64xx_set_para_pr(pr_table, track_type, params);

			pchprop->pr = pr_table[track_type];

			slimbus_hi6405_set_para_sd(track_type, scene_type,
				ch, pchprop->pr, &(pchprop->sd));
			slimbus_hi6405_param_update_sl(track_type,scene_type,
				ch, pchprop->pr, &(pchprop->sl));
			pchprop++;
		}
	}

	return 0;
}

void slimbus_hi6405_param_update(
			slimbus_device_info_t *dev,
			uint32_t track_type,
			slimbus_track_param_t *params)
{
	if (!dev) {
		pr_err("[%s:%d] dev is NULL\n", __FUNCTION__, __LINE__);
		return;
	}
	if (track_type >= SLIMBUS_6405_TRACK_MAX) {
		pr_err("[%s:%d] track type is invalid, track_type:%d\n", __FUNCTION__, __LINE__,track_type);
		return;
	}

	slimbus_hi64xx_set_para_pr(pr_table, track_type, params);

	slimbus_hi6405_param_init(dev);

	return;
}

static int process_play_and_debug_conflict(slimbus_6405_track_type_t track, int *need_callback)
{
	int ret = 0;

	if (track_state_is_on(SLIMBUS_6405_TRACK_DIRECT_PLAY)
		&& (track == SLIMBUS_6405_TRACK_DEBUG)) {
		pr_info("[%s:%d] debug conflict\n", __FUNCTION__, __LINE__);
		return -EPERM;
	}
	if (track_state_is_on(SLIMBUS_6405_TRACK_DEBUG)) {
		*need_callback = 1;
		ret = slimbus_drv_track_deactivate(track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].channel_pro,
						track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].params.channels);

		slimbus_trackstate_set(SLIMBUS_6405_TRACK_DEBUG, false);
		track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].params.channels = 1;
	}

	return ret;
}

static int process_soundtrigger_params_conflict(slimbus_6405_track_type_t track)
{
	int ret = 0;

	bool is_fast_track = ((track_config_6405_table[SLIMBUS_6405_TRACK_SOUND_TRIGGER].params.channels == 2)
		&& (track_config_6405_table[SLIMBUS_6405_TRACK_SOUND_TRIGGER].params.rate == SLIMBUS_SAMPLE_RATE_192K));

	if ((track == SLIMBUS_6405_TRACK_SOUND_TRIGGER) && is_fast_track) {
		track_config_6405_table[track].params.channels = 1;
		pr_info("[%s:%d] soundtrigger conflict\n", __FUNCTION__, __LINE__);
		return -EPERM;
	}

	return ret;
}

static int process_normal_scene_conflict(slimbus_6405_track_type_t track, bool track_enable, int *need_callback)
{
	int ret = 0;

	if (track_state_is_on(SLIMBUS_6405_TRACK_VOICE_UP) && (track == SLIMBUS_6405_TRACK_SOUND_TRIGGER) && track_enable) {
		pr_info("[%s:%d] st conflict\n", __FUNCTION__, __LINE__);
		/*return -EPERM;*///fixme
	}

	if ((track == SLIMBUS_6405_TRACK_DIRECT_PLAY) && track_enable) {
		pr_info("[%s:%d] direct play conflict\n", __FUNCTION__, __LINE__);
		return -EPERM;
	}
	if ((track == SLIMBUS_6405_TRACK_DEBUG) && (track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].params.channels == 2)) {
		track_config_6405_table[track].params.channels = 1;
		pr_info("[%s:%d] debug conflict\n", __FUNCTION__, __LINE__);
		return -EPERM;
	}
	if (track_state_is_on(SLIMBUS_6405_TRACK_DIRECT_PLAY)) {
		ret = slimbus_drv_track_deactivate(track_config_6405_table[SLIMBUS_6405_TRACK_DIRECT_PLAY].channel_pro,
						track_config_6405_table[SLIMBUS_6405_TRACK_DIRECT_PLAY].params.channels);

		slimbus_trackstate_set(SLIMBUS_6405_TRACK_DIRECT_PLAY, false);
	}
	if (track_state_is_on(SLIMBUS_6405_TRACK_DEBUG) && (track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].params.channels == 2)) {
		*need_callback = 1;
		ret = slimbus_drv_track_deactivate(track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].channel_pro,
						track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].params.channels);

		slimbus_trackstate_set(SLIMBUS_6405_TRACK_DEBUG, false);
		track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].params.channels = 1;
	}
	if (track_state_is_on(SLIMBUS_6405_TRACK_SOUND_TRIGGER)
		&& (track == SLIMBUS_6405_TRACK_DEBUG)) {
		pr_info("[%s:%d] st and debug conflict\n", __FUNCTION__, __LINE__);
		return -EPERM;
	}
	if (track_state_is_on(SLIMBUS_6405_TRACK_DEBUG)
		&& (track == SLIMBUS_6405_TRACK_SOUND_TRIGGER)) {
		*need_callback = 1;
		ret = slimbus_drv_track_deactivate(track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].channel_pro,
				track_config_6405_table[SLIMBUS_6405_TRACK_DEBUG].params.channels);

		slimbus_trackstate_set(SLIMBUS_6405_TRACK_DEBUG, false);
	}
	ret = process_soundtrigger_params_conflict(track);

	return ret;
}

static int process_other_scenes_conflict(slimbus_6405_track_type_t track, int *need_callback)
{
	int ret = 0;

	if (process_play_and_debug_conflict(track, need_callback)) {
		pr_info("[%s:%d] debug conflict\n", __FUNCTION__, __LINE__);
		return -EPERM;
	}

	ret = process_soundtrigger_params_conflict(track);

	return ret;
}

 int slimbus_hi6405_check_scenes(
				uint32_t track,
				uint32_t scenes,
				bool track_enable)
{
	unsigned int i = 0;
	int ret = 0;
	int need_callback = 0;

	if (track >= SLIMBUS_6405_TRACK_MAX || scenes >= SLIMBUS_6405_SCENE_CONFIG_MAX) {
		pr_err("param is invalid, track:%d, scenes:%d\n", track, scenes);
		return -EINVAL;
	}

	switch (scenes) {
	case SLIMBUS_6405_SCENE_CONFIG_DIRECT_PLAY:
		ret = process_other_scenes_conflict(track, &need_callback);
		break;
	case SLIMBUS_6405_SCENE_CONFIG_NORMAL:
		ret = process_normal_scene_conflict(track, track_enable, &need_callback);
		break;
	default:
		break;
	}

	for (i = 0; i < SLIMBUS_6405_TRACK_MAX; i++) {
		if ((track_state_is_on(i) || (i == SLIMBUS_6405_TRACK_DEBUG))
			&& track_config_6405_table[i].params.callback && need_callback) {
			track_config_6405_table[i].params.callback(i, (void *)&(track_config_6405_table[i].params));
		}
	}

	return ret;
}

static bool is_play_scene(uint32_t active_tracks)
{
	switch (active_tracks) {
	case SLIMBUS_6405_TRACK_PLAY_ON:
	case SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_EC_ON:
	case SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_2PA_IV_ON:
	case SLIMBUS_6405_TRACK_EC_ON:
	case SLIMBUS_6405_TRACK_2PA_IV_ON:
		return true;
	default:
		break;
	}

	return false;
}

static bool is_call_only_scene(uint32_t active_tracks)
{
	switch (active_tracks) {
	case SLIMBUS_6405_TRACK_VOICE_UP_ON:
	case SLIMBUS_6405_TRACK_VOICE_DOWN_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON:
		return true;
	default:
		break;
	}

	return false;
}

static bool is_call_12288_scene(uint32_t active_tracks)
{
	if (track_state_is_on(SLIMBUS_6405_TRACK_4PA_IV))
		return false;

	switch (active_tracks) {
	case SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_PLAY_ON:
	case SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_PLAY_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_PLAY_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_EC_ON:
	case SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_EC_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_EC_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_EC_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_2PA_IV_ON:
	case SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_2PA_IV_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_2PA_IV_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_2PA_IV_ON:
	/* choose 1 from 4 with D3D4 */
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_2PA_IV_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	/* choose 2 from 4 with D3D4 */
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_2PA_IV_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_2PA_IV_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_2PA_IV_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	/* choose 3 from 4 with D3D4 */
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_2PA_IV_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_2PA_IV_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_2PA_IV_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:
	/* choose 4 with D3D4 */
	case SLIMBUS_6405_TRACK_VOICE_UP_ON | SLIMBUS_6405_TRACK_VOICE_DOWN_ON \
		 | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_2PA_IV_ON | SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4_ON:

		return true;
	default:
		break;
	}

	return false;
}

static bool is_enhance_soundtrigger_6144_scene(uint32_t active_tracks)
{
	switch (active_tracks) {
	case SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON | SLIMBUS_6405_TRACK_PLAY_ON:
	case SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON | SLIMBUS_6405_TRACK_EC_ON:
	case SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_EC_ON:
		if (track_config_6405_table[SLIMBUS_6405_TRACK_SOUND_TRIGGER].params.rate == SLIMBUS_SAMPLE_RATE_192K) {
			pr_info("[%s:%d] not enhance st scene", __FUNCTION__, __LINE__);
			return false;
		}
		pr_info("[%s:%d] enhance st scene", __FUNCTION__, __LINE__);
		return true;
	default:
		break;
	}

	return false;
}

static bool is_direct_play_scene(uint32_t active_tracks)
{
	uint32_t tmp = 0;

	switch (active_tracks) {
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_CAPTURE_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_EC_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_PLAY_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_CAPTURE_ON | SLIMBUS_6405_TRACK_EC_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_CAPTURE_ON | SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_EC_ON | SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_EC_ON | SLIMBUS_6405_TRACK_PLAY_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_CAPTURE_ON | SLIMBUS_6405_TRACK_PLAY_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_CAPTURE_ON | SLIMBUS_6405_TRACK_EC_ON | SLIMBUS_6405_TRACK_PLAY_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_EC_ON | SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_CAPTURE_ON | SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_CAPTURE_ON | SLIMBUS_6405_TRACK_EC_ON | SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON:
	case SLIMBUS_6405_TRACK_DIRECT_PLAY_ON | SLIMBUS_6405_TRACK_PLAY_ON | SLIMBUS_6405_TRACK_CAPTURE_ON | SLIMBUS_6405_TRACK_EC_ON | SLIMBUS_6405_TRACK_SOUND_TRIGGER_ON:
		return true;
	default:
		tmp = (SLIMBUS_6405_TRACK_VOICE_DOWN_ON | SLIMBUS_6405_TRACK_VOICE_UP_ON);
		if ((active_tracks & tmp) == tmp) {
			return false;
		}

		tmp = (SLIMBUS_6405_TRACK_DEBUG_ON | SLIMBUS_6405_TRACK_DIRECT_PLAY_ON);
		if ((active_tracks & tmp) == tmp ) {
			return true;
		}

		break;
	}

	return false;
}

static bool is_soc_4pa_play_scene(uint32_t active_tracks)
{
	if ((active_tracks & SLIMBUS_6405_TRACK_4PA_IV_ON) != 0) {
		return true;
	} else {
		return false;
	}
}

int slimbus_hi6405_select_scenes(
				struct slimbus_device_info *dev,
				uint32_t track,
				slimbus_track_param_t *params,
				bool track_enable)
{
	slimbus_scene_config_6405_type_t scene_config_type = SLIMBUS_6405_SCENE_CONFIG_NORMAL;
	slimbus_subframe_mode_t sm = SLIMBUS_SM_1_CSW_32_SL;
	slimbus_clock_gear_t cg = SLIMBUS_CG_10;
	uint32_t active_tracks = 0;
	int ret = 0;

	if (track >= SLIMBUS_6405_TRACK_MAX) {
		pr_err("track is invalid, track:%d\n", track);
		return -EINVAL;
	}

	active_tracks = slimbus_trackstate_get();

	if (track_enable) {
		active_tracks |= (1<<track);
	}

	if (is_play_scene(active_tracks)) {
		scene_config_type = SLIMBUS_6405_SCENE_CONFIG_PLAY;
		cg = SLIMBUS_CG_8;
		sm = SLIMBUS_SM_4_CSW_32_SL;
	} else if (is_call_only_scene(active_tracks)) {
		scene_config_type = SLIMBUS_6405_SCENE_CONFIG_CALL;
		cg = SLIMBUS_CG_8;
		sm = SLIMBUS_SM_6_CSW_24_SL;
	} else if (is_call_12288_scene(active_tracks)) {
		scene_config_type = SLIMBUS_6405_SCENE_CONFIG_CALL_12288;
		cg = SLIMBUS_CG_9;
		sm = SLIMBUS_SM_2_CSW_32_SL;
	} else if (is_enhance_soundtrigger_6144_scene(active_tracks)) {
		scene_config_type = SLIMBUS_6405_SCENE_CONFIG_ENHANCE_ST_6144;
		cg = SLIMBUS_CG_8;
		sm = SLIMBUS_SM_4_CSW_32_SL;
	} else if (is_direct_play_scene(active_tracks)) {
		scene_config_type = SLIMBUS_6405_SCENE_CONFIG_DIRECT_PLAY;
		cg = SLIMBUS_CG_10;
		sm = SLIMBUS_SM_2_CSW_32_SL;
	} else if (is_soc_4pa_play_scene(active_tracks)){
		scene_config_type = SLIMBUS_6405_SCENE_CONFIG_4PA_MOVEUP;
		cg = SLIMBUS_CG_10;
		sm = SLIMBUS_SM_2_CSW_32_SL;
	} else {
		scene_config_type = SLIMBUS_6405_SCENE_CONFIG_NORMAL;
		cg = SLIMBUS_CG_10;
		sm = SLIMBUS_SM_2_CSW_32_SL;
	}

	ret = slimbus_hi6405_check_scenes(track, scene_config_type, track_enable);
	if (ret) {
		pr_info("[%s:%d] ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	if (dev->scene_config_type != scene_config_type) {
		pr_info("[%s:%d] scene changed from %d to %d\n", __FUNCTION__, __LINE__, dev->scene_config_type, scene_config_type);
		dev->cg = cg;
		dev->sm = sm;
		dev->scene_config_type = scene_config_type;
	}

	slimbus_hi6405_param_update(dev, track, params);

	return 0;
}

void slimbus_hi6405_get_st_params(slimbus_sound_trigger_params_t *params)
{
	if (params != NULL) {
		params->sample_rate = SLIMBUS_SAMPLE_RATE_48K;
		params->channels = 2;
		params->track_type = SLIMBUS_6405_TRACK_AUDIO_CAPTURE;
	}

	return;
}

int slimbus_hi6405_track_soundtrigger_activate(
						uint32_t track,
						bool slimbus_dynamic_freq_enable,
						struct slimbus_device_info *dev,
						slimbus_track_param_t	*params)
{
	int ret = 0;
	slimbus_sound_trigger_params_t  st_params;
	slimbus_track_param_t st_normal_params;

	if (SLIMBUS_6405_TRACK_SOUND_TRIGGER != track) {
		pr_err("[%s:%d] track %d deactive is not soundtriger\n", __FUNCTION__, __LINE__, track);
		return -1;
	}

	memset(&st_params, 0, sizeof(st_params));
	memset(&st_normal_params, 0, sizeof(st_normal_params));

	slimbus_hi6405_get_st_params(&st_params);
	st_normal_params.channels = st_params.channels;
	st_normal_params.rate = st_params.sample_rate;

	ret = slimbus_hi6405_param_set(dev, st_params.track_type,
				&st_normal_params);
	ret += slimbus_hi6405_param_set(dev, track,
				params);
	if (ret) {
		pr_err("slimbus device param set failed, ret = %d\n", ret);
		return ret;
	}

	/*  request soc slimbus clk to 24.576M */
	slimbus_freq_request();

	if (slimbus_dynamic_freq_enable) {
		ret = slimbus_drv_track_update(dev->cg, dev->sm, track, dev,
				track_config_6405_table[track].params.channels, track_config_6405_table[track].channel_pro);
		ret += slimbus_drv_track_update(dev->cg, dev->sm, st_params.track_type, dev,
				SLIMBUS_VOICE_UP_SOUNDTRIGGER, track_config_6405_table[st_params.track_type].channel_pro);
	} else {
		ret = slimbus_drv_track_activate(track_config_6405_table[track].channel_pro, track_config_6405_table[track].params.channels);
		ret += slimbus_drv_track_activate(track_config_6405_table[st_params.track_type].channel_pro, SLIMBUS_VOICE_UP_SOUNDTRIGGER);
	}

	return ret;

}

int slimbus_hi6405_track_soundtrigger_deactivate(uint32_t track)
{
	int ret = 0;
	slimbus_sound_trigger_params_t  st_params;

	if (SLIMBUS_6405_TRACK_SOUND_TRIGGER != track) {
		pr_err("[%s:%d] track %d deactive is not soundtriger\n", __FUNCTION__, __LINE__, track);
		return -1;
	}

	memset(&st_params, 0, sizeof(st_params));

	slimbus_hi6405_get_st_params(&st_params);

	/*  release soc slimbus clk to 21.777M */
	slimbus_freq_release();
	ret = slimbus_drv_track_deactivate(track_config_6405_table[track].channel_pro, track_config_6405_table[track].params.channels);
	ret += slimbus_drv_track_deactivate(track_config_6405_table[st_params.track_type].channel_pro, SLIMBUS_VOICE_UP_SOUNDTRIGGER);

	return ret;
}

bool slimbus_hi6405_track_is_fast_soundtrigger(uint32_t track)
{
	if (SLIMBUS_6405_TRACK_SOUND_TRIGGER != track) {
		return false;
	}

	return ((SLIMBUS_6405_TRACK_SOUND_TRIGGER == track)
		&& (track_config_6405_table[track].params.rate == SLIMBUS_SAMPLE_RATE_192K));
}

void slimbus_hi6405_check_st_conflict(uint32_t track, slimbus_track_param_t *params)
{
	int res = 0;
	bool is_fast_track_on = (track_state_is_on(SLIMBUS_6405_TRACK_SOUND_TRIGGER)
		&& (track_config_table[SLIMBUS_6405_TRACK_SOUND_TRIGGER].params.rate == SLIMBUS_SAMPLE_RATE_192K));

	if ((SLIMBUS_6405_TRACK_SOUND_TRIGGER == track)
		&& params
		&& (params->rate == SLIMBUS_SAMPLE_RATE_16K)
		&& is_fast_track_on) {
		pr_info("st conflict, so stop codec st\n");

		res = slimbus_drv_track_deactivate(track_config_table[SLIMBUS_6405_TRACK_SOUND_TRIGGER].channel_pro,
						track_config_table[SLIMBUS_6405_TRACK_SOUND_TRIGGER].params.channels);

		slimbus_trackstate_set(SLIMBUS_6405_TRACK_SOUND_TRIGGER, false);
	}

	if (res != 0) {
		pr_info("start soc st ,stop codec st fail\n");
	}
}
EXPORT_SYMBOL(slimbus_hi6405_check_st_conflict);
