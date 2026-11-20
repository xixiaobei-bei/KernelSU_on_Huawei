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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <dsm/dsm_pub.h>
#include "linux/hisi/audio_log.h"

#include "csmi.h"
#include "slimbus_drv.h"
#include "slimbus.h"
#include "slimbus_da_combine.h"

/*lint -e750 -e730 -e785 -e574*/
#define LOG_TAG "slimbus_DA_combine_v3"

/* segment length */
static uint16_t sl_table[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
	/* 6.144M FPGA */
	{
		{ 6, 6 },                    /* audio playback */
		{ 4, 4 },                    /* audio capture */
		{ 4, 4 },                    /* voice down */
		{ 4, 4, 4, 4 },              /* voice up */
		{ 5 },                       /* image download */
		{ 0, 0 },                    /* EC_REF */
		{ 0 },                       /* sound trigger */
		{ 0 },                       /* debug */
		{ 0, 0 },                    /* direct play */
		{ 0, 0 },                    /* fast play */
		{ 0, 0 },                    /* kws */
	},
	/* 6.144M play */
	{
		{ 6, 6 },                    /* audio playback */
		{ 0, 0 },                    /* audio capture */
		{ 0, 0 },                    /* voice down */
		{ 0, 0, 0, 0 },              /* voice up */
		{ 5 },                       /* image download */
		{ 6, 6 },                    /* EC_REF */
		{ 0 },                       /* sound trigger */
		{ 0 },                       /* debug */
		{ 0, 0 },                    /* direct play */
		{ 0, 0 },                    /* fast play */
		{ 0, 0 },                    /* kws */
	},
	/* 6.144M call */
	{
		{ 0, 0 },                    /* audio playback */
		{ 0, 0 },                    /* audio capture */
		{ 6, 6 },                    /* voice down */
		{ 6, 6, 6, 6 },              /* voice up */
		{ 5 },                       /* image download */
		{ 0, 0 },                    /* EC_REF */
		{ 0 },                       /* sound trigger */
		{ 0 },                       /* debug */
		{ 0, 0 },                    /* direct play */
		{ 0, 0 },                    /* fast play */
		{ 0, 0 },                    /* kws */
	},
	/* 24.576M normal */
	{
		{ 6, 6 },                    /* audio playback */
		{ 6, 6, 6, 6 },              /* audio capture */
		{ 6, 6 },                    /* voice down */
		{ 6, 6, 6, 6 },              /* voice up */
		{ 5 },                       /* image download */
		{ 6, 6 },                    /* EC_REF */
		{ 8 },                       /* sound trigger */
		{ 8 },                       /* debug */
		{ 0, 0 },                    /* direct play */
		{ 0, 0 },                    /* fast play */
		{ 4, 4 },                    /* kws */
	},
	/* 24.576M anc call */
	{
		{ 6, 6 },                    /* audio playback */
		{ 0, 0 },                    /* audio capture */
		{ 6, 6 },                    /* voice down */
		{ 6, 6, 6, 6 },              /* voice up */
		{ 5 },                       /* image download */
		{ 0, 0 },                    /* EC_REF */
		{ 0 },                       /* sound trigger */
		{ 8, 8 },                    /* debug */
		{ 0, 0 },                    /* direct play */
		{ 0, 0 },                    /* fast play */
		{ 0, 0 },                    /* kws */
	},
	/* 24.576M hires play */
	{
		{ 0, 0 },                    /* audio playback */
		{ 6, 6, 6, 6 },              /* audio capture */
		{ 0, 0 },                    /* voice down */
		{ 0, 0, 0, 0 },              /* voice up */
		{ 5 },                       /* image download */
		{ 6, 6 },                    /* EC_REF */
		{ 8 },                       /* sound trigger */
		{ 0 },                       /* debug */
		{ 6, 6 },                    /* direct play */
		{ 0, 0 },                    /* fast play */
		{ 4, 4 },                    /* kws */
	},
	/* image load */
	{
		{ 0, 0 },                     /* audio playback */
		{ 0, 0, 0, 0 },               /* audio capture */
		{ 0, 0 },                     /* voice down */
		{ 0, 0, 0, 0 },               /* voice up */
		{ 5 },                        /* image download */
		{ 0, 0 },                     /* EC_REF */
		{ 0 },                        /* sound trigger */
		{ 0 },                        /* debug */
		{ 0, 0 },                     /* direct play */
		{ 0, 0 },                     /* fast play */
		{ 0, 0 },                     /* kws */
	},
	/* 24.576M fast play */
	{
		{ 0, 0 },                     /* audio playback */
		{ 0, 0, 0, 0 },               /* audio capture */
		{ 0, 0 },                     /* voice down */
		{ 0, 0, 0, 0 },               /* voice up */
		{ 5 },                        /* image download */
		{ 6, 6 },                     /* EC_REF */
		{ 0 },                        /* sound trigger */
		{ 0 },                        /* debug */
		{ 0, 0 },                     /* direct play */
		{ 6, 6 },                     /* fast play */
		{ 0, 0 },                     /* kws */
	},
	/* 24.576M fast play & record */
	{
		{ 6, 6 },                       /* audio playback */
		{ 6, 6, 6, 6 },                 /* audio capture */
		{ 0, 0 },                       /* voice down */
		{ 0, 0, 0, 0 },                 /* voice up */
		{ 5 },                          /* image download */
		{ 6, 6 },                       /* EC_REF */
		{ 0 },                          /* sound trigger */
		{ 0 },                          /* debug */
		{ 6, 6 },                       /* direct play */
		{ 6, 6 },                       /* fast play */
		{ 0, 0 },                       /* kws */
	},
	/* 24.576M fast play & st */
	{
		{ 6, 6 },                       /* audio playback */
		{ 0, 0, 0, 0 },                 /* audio capture */
		{ 0, 0 },                       /* voice down */
		{ 0, 0, 0, 0 },                 /* voice up */
		{ 5 },                          /* image download */
		{ 6, 6 },                       /* EC_REF */
		{ 8 },                          /* sound trigger */
		{ 0 },                          /* debug */
		{ 6, 6 },                       /* direct play */
		{ 6, 6 },                       /* fast play */
		{ 0, 0 },                       /* kws */
	},
	/* 12.288M call */
	{
		{ 6, 6 },                      /* audio playback */
		{ 0, 0 },                      /* audio capture */
		{ 6, 6 },                      /* voice down */
		{ 6, 6, 6, 6 },                /* voice up */
		{ 5 },                         /* image download */
		{ 6, 6 },                      /* EC_REF */
		{ 0 },                         /* sound trigger */
		{ 0 },                         /* debug */
		{ 0, 0 },                      /* direct play */
		{ 0, 0 },                      /* fast play */
		{ 0, 0 },                      /* kws */
	},
	/* 6.144M soundtrigger enhance */
	{
		{ 6, 6 },                      /* audio playback */
		{ 0, 0 },                      /* audio capture */
		{ 0, 0 },                      /* voice down */
		{ 0, 0, 0, 0 },                /* voice up */
		{ 5 },                         /* image download */
		{ 6, 6 },                      /* EC_REF */
		{ 0, 0 },                      /* sound trigger */
		{ 0 },                         /* debug */
		{ 0, 0 },                      /* direct play */
		{ 0, 0 },                      /* fast play */
		{ 4, 4 },                      /* kws */
	},
};

/* presence rate */
static enum slimbus_presence_rate pr_table[SLIMBUS_TRACK_MAX] = {
			SLIMBUS_PR_48K, SLIMBUS_PR_48K,
			SLIMBUS_PR_8K, SLIMBUS_PR_8K,
			SLIMBUS_PR_768K, SLIMBUS_PR_48K,
			SLIMBUS_PR_192K, SLIMBUS_PR_192K,
			SLIMBUS_PR_192K, SLIMBUS_PR_48K,
			SLIMBUS_PR_16K
};

/* channel index */
static uint8_t cn_table[SLIMBUS_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ AUDIO_PLAY_CHANNEL_LEFT, AUDIO_PLAY_CHANNEL_RIGHT },
			{ AUDIO_CAPTURE_CHANNEL_LEFT, AUDIO_CAPTURE_CHANNEL_RIGHT,
				VOICE_UP_CHANNEL_MIC1, VOICE_UP_CHANNEL_MIC2 },
			{ VOICE_DOWN_CHANNEL_LEFT, VOICE_DOWN_CHANNEL_RIGHT },
			{ VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4,
				VOICE_UP_CHANNEL_MIC1, VOICE_UP_CHANNEL_MIC2 },
			{ IMAGE_DOWNLOAD },
			{ VOICE_ECREF, AUDIO_ECREF },
			{ SOUND_TRIGGER_CHANNEL_LEFT, SOUND_TRIGGER_CHANNEL_RIGHT },
			{ DEBUG_LEFT, DEBUG_RIGHT },
			{ AUDIO_PLAY_CHANNEL_LEFT, AUDIO_PLAY_CHANNEL_RIGHT },
			{ VOICE_DOWN_CHANNEL_LEFT, VOICE_DOWN_CHANNEL_RIGHT },
			{ VOICE_UP_CHANNEL_MIC3, VOICE_UP_CHANNEL_MIC4 }, /* kws */
};


/* source port number */
static uint8_t source_pn_table[SLIMBUS_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ AUDIO_PLAY_SOC_LEFT_PORT, AUDIO_PLAY_SOC_RIGHT_PORT },
			{ AUDIO_CAPTURE_DA_COMBINE_LEFT_PORT, AUDIO_CAPTURE_DA_COMBINE_RIGHT_PORT,
				VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT },
			{ VOICE_DOWN_SOC_LEFT_PORT, VOICE_DOWN_SOC_RIGHT_PORT },
			{ VOICE_UP_DA_COMBINE_MIC1_PORT, VOICE_UP_DA_COMBINE_MIC2_PORT,
				VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT },
			{ IMAGE_DOWNLOAD_SOC_PORT },
			{ VOICE_DA_COMBINE_ECREF_PORT, AUDIO_DA_COMBINE_ECREF_PORT },
			{ VOICE_UP_DA_COMBINE_MIC1_PORT, VOICE_UP_DA_COMBINE_MIC2_PORT },
			{ BT_CAPTURE_DA_COMBINE_LEFT_PORT, BT_CAPTURE_DA_COMBINE_RIGHT_PORT },
			{ AUDIO_PLAY_SOC_LEFT_PORT, AUDIO_PLAY_SOC_RIGHT_PORT },
			{ VOICE_DOWN_SOC_LEFT_PORT, VOICE_DOWN_SOC_RIGHT_PORT },
			{ VOICE_UP_DA_COMBINE_MIC3_PORT, VOICE_UP_DA_COMBINE_MIC4_PORT }, /* kws */
};

/* sink port number */
static uint8_t sink_pn_table[SLIMBUS_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ AUDIO_PLAY_DA_COMBINE_LEFT_PORT, AUDIO_PLAY_DA_COMBINE_RIGHT_PORT },
			{ AUDIO_CAPTURE_SOC_LEFT_PORT, AUDIO_CAPTURE_SOC_RIGHT_PORT,
				VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT },
			{ VOICE_DOWN_DA_COMBINE_LEFT_PORT, VOICE_DOWN_DA_COMBINE_RIGHT_PORT },
			{ VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT,
				VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT },
			{ IMAGE_DOWNLOAD_DA_COMBINE_PORT },
			{ VOICE_SOC_ECREF_PORT, AUDIO_SOC_ECREF_PORT },
			{ VOICE_UP_SOC_MIC1_PORT, VOICE_UP_SOC_MIC2_PORT },
			{ BT_CAPTURE_SOC_LEFT_PORT, BT_CAPTURE_SOC_RIGHT_PORT },
			{ AUDIO_PLAY_DA_COMBINE_LEFT_PORT, AUDIO_PLAY_DA_COMBINE_RIGHT_PORT },
			{ IMAGE_DOWNLOAD_DA_COMBINE_PORT, SUPER_PLAY_DA_COMBINE_PORT },
			{ VOICE_UP_SOC_MIC3_PORT, VOICE_UP_SOC_MIC4_PORT }, /* kws */
};

/* segment distribution */
static uint16_t sd_table[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_TRACK_MAX][SLIMBUS_CHANNELS_MAX] = {
	/* 6.144M FPGA */
	{
		{ 0xc24, 0xc2a },                    /* audio playback */
		{ 0xc30, 0xc34 },                    /* audio capture */
		{ 0x58, 0x5c },                      /* voice down 8k */
		{ 0x78, 0x7c, 0x458, 0x45C },        /* voice up 8k */
		{ 0xC0B },                           /* image download */
		{ 0, 0 },                            /* EC_REF */
		{ 0 },                               /* sound trigger 192K */
		{ 0 },                               /* debug 192K */
		{ 0 },                               /* direct play 192k */
		{ 0 },                               /* fast play 48k */
		{ 0, 0 },                            /* kws */
	},
	/* 6.144M play */
	{
		{ 0xC28, 0xC2E },                    /* audio playback */
		{ 0, 0 },                            /* audio capture */
		{ 0, 0 },                            /* voice down 8k */
		{ 0, 0, 0, 0 },                      /* voice up 8k */
		{ 0xC0B },                           /* image download */
		{ 0xC34, 0xC3A },                    /* EC_REF */
		{ 0 },                               /* sound trigger 192K */
		{ 0 },                               /* debug 192K */
		{ 0 },                               /* direct play 192k */
		{ 0 },                               /* fast play 48k */
		{ 0, 0 },                            /* kws */
	},
	/* 6.144M call */
	{
		{ 0, 0 },                            /* audio playback */
		{ 0, 0 },                            /* audio capture */
		{ 0x46, 0x5E },                      /* voice down 8k */
		{ 0x4C, 0x52, 0x64, 0x6A },          /* voice up 8k */
		{ 0xC0B },                           /* image download */
		{ 0, 0 },                            /* EC_REF */
		{ 0 },                               /* sound trigger 192K */
		{ 0 },                               /* debug 192K */
		{ 0 },                               /* direct play 192k */
		{ 0 },                               /* fast play 48k */
		{ 0, 0 },                            /* kws */
	},
	/* 24.576M normal */
	{
		{ 0xCC1, 0xCE1 },                    /* audio playback */
		{ 0xC87, 0xCA7, 0xCC7, 0xCE7 },      /* audio capture */
		{ 0x10D, 0x12D },                    /* voice down 8k */
		{ 0x14D, 0x16D, 0x18D, 0x1AD },      /* voice up 8k */
		{ 0xC0B },                           /* image download */
		{ 0xC81, 0xCA1 },                    /* EC_REF */
		{ 0xC33 },                           /* sound trigger 192K */
		{ 0xC33 },                           /* debug 192K */
		{ 0 },                               /* direct play 192k */
		{ 0 },                               /* fast play 48k */
		{ 0x93, 0x97 },                      /* kws */
	},
	/* 24.576M anc call */
	{
		{ 0xCC1, 0xCE1 },                     /* audio playback */
		{ 0, 0, 0, 0 },                       /* audio capture */
		{ 0x107, 0x127 },                     /* voice down 8k */
		{ 0x147, 0x167, 0x187, 0x1A7 },       /* voice up 8k */
		{ 0xC0B },                            /* image download */
		{ 0, 0 },                             /* EC_REF */
		{ 0 },                                /* sound trigger 192K */
		{ 0xC2D, 0xC35 },                     /* debug 192K */
		{ 0 },                                /* direct play 192k */
		{ 0 },                                /* fast play 48k */
		{ 0, 0 },                             /* kws */
	},
	/* 24.576M direct play */
	{
		{ 0, 0 },                             /* audio playback */
		{ 0xC8A, 0xCAA, 0xCCA, 0xCEA },       /* audio capture */
		{ 0, 0 },                             /* voice down 8k */
		{ 0, 0, 0, 0 },                       /* voice up 8k */
		{ 0xC0B },                            /* image download */
		{ 0xC82, 0xCA2 },                     /* EC_REF */
		{ 0xC22 },                            /* sound trigger 192K */
		{ 0 },                                /* debug 192K */
		{ 0xC30, 0xC36 },                     /* direct play 192k */
		{ 0 },                                /* fast play 48k */
		{ 0x9C, 0xBC },                       /* kws */
	},
	/* image load */
	{
		{ 0, 0 },                             /* audio playback */
		{ 0, 0, 0, 0 },                       /* audio capture */
		{ 0, 0 },                             /* voice down 8k */
		{ 0, 0, 0, 0 },                       /* voice up 8k */
		{ 0xC0B },                            /* image download */
		{ 0, 0 },                             /* EC_REF */
		{ 0 },                                /* sound trigger 192K */
		{ 0, 0 },                             /* debug 192K */
		{ 0 },                                /* direct play 192k */
		{ 0 },                                /* fast play 48k */
		{ 0, 0 },                             /* kws */
	},
	/* 6.144M fast play */
	{
		{ 0, 0 },                             /* audio playback */
		{ 0, 0, 0, 0 },                       /* audio capture */
		{ 0, 0 },                             /* voice down 8k */
		{ 0, 0, 0, 0 },                       /* voice up 8k */
		{ 0xC0B },                            /* image download */
		{ 0xC34, 0xC3A },                     /* EC_REF */
		{ 0 },                                /* sound trigger 192K */
		{ 0 },                                /* debug 192K */
		{ 0, 0 },                             /* direct play 192k */
		{ 0xC28, 0xC2E },                     /* fast play 48k */
		{ 0, 0 },                             /* kws */
	},
	/* 24.576M fast play & rec */
	{
		{ 0xCC2, 0xCE2 },                     /* audio playback */
		{ 0xC88, 0xCA8, 0xCC8, 0xCE8 },       /* audio capture */
		{ 0, 0 },                             /* voice down 8k */
		{ 0, 0, 0, 0 },                       /* voice up 8k */
		{ 0xC0B },                            /* image download */
		{ 0xC82, 0xCA2 },                     /* EC_REF */
		{ 0 },                                /* sound trigger 192K */
		{ 0 },                                /* debug 192K */
		{ 0xC2E, 0xC34 },                     /* direct play 192k */
		{ 0xC9A, 0xCBA },                     /* fast play 48k */
		{ 0, 0 },                             /* kws */
	},
	/* 24.576M fast play & st */
	{
		{ 0xCC4, 0xCE4 },                     /* audio playback */
		{ 0, 0, 0, 0 },                       /* audio capture */
		{ 0, 0 },                             /* voice down 8k */
		{ 0, 0, 0, 0 },                       /* voice up 8k */
		{ 0xC0B },                            /* image download */
		{ 0xC84, 0xCA4 },                     /* EC_REF */
		{ 0xC38 },                            /* sound trigger 192K */
		{ 0 },                                /* debug 192K */
		{ 0xC30, 0xC36 },                     /* direct play 192k */
		{ 0xC8A, 0xCAA },                     /* fast play 48k */
		{ 0, 0 },                             /* kws */
	},
	/* 12.288M call */
	{
		{ 0xC48, 0xC68 },                     /* audio playback */
		{ 0, 0 },                             /* audio capture */
		{ 0xCE, 0xD4 },                       /* voice down 8k */
		{ 0x8E, 0x94, 0xAE, 0xB4 },           /* voice up 8k */
		{ 0xC0B },                            /* image download */
		{ 0xC5A, 0xC7A },                     /* EC_REF */
		{ 0 },                                /* sound trigger 192K */
		{ 0 },                                /* debug 192K */
		{ 0 },                                /* direct play 192k */
		{ 0 },                                /* fast play 48k */
		{ 0, 0 },                             /* kws */
	},
	/* 6.144M soundtrigger enhance */
	{
		{ 0xC28, 0xC2E },                      /* audio playback */
		{ 0, 0 },                              /* audio capture */
		{ 0, 0 },                              /* voice down */
		{ 0, 0, 0, 0 },                        /* voice up */
		{ 0xC0B },                             /* image download */
		{ 0xC34, 0xC3A },                      /* EC_REF */
		{ 0, 0 },                              /* sound trigger */
		{ 0 },                                 /* debug */
		{ 0, 0 },                              /* direct play */
		{ 0, 0 },                              /* fast play */
		{ 0x24, 0x424 },                       /* kws */
	},
};

static uint16_t sd_voice_down_16k[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0x38, 0x3C },                     /* 6.144M FPGA voice down 16k */
			{ 0, 0 },                           /* 6.144M play */
			{ 0x26, 0x3E },                     /* 6.144M call */
			{ 0x8D, 0xAD },                     /* 24.576M normal */
			{ 0x87, 0xA7 },                     /* 24.576M anc call */
			{ 0, 0 },                           /* 24.576M direct play */
			{ 0, 0 },                           /* image */
			{ 0, 0 },                           /* 6.144M fast play */
			{ 0, 0 },                           /* 24.576M fast play & rec */
			{ 0, 0 },                           /* 24.576M fast play & st */
			{ 0x44E, 0x454 },                   /* 12.288M call */
			{ 0, 0 },                           /* 6.144M soundtrigger enhance */
};

static uint16_t sd_voice_up_16k[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0x438, 0x43c, 0x838, 0x83c },      /* 6.144M FPGA voice up 16k */
			{ 0, 0, 0, 0 },                      /* 6.144M play */
			{ 0x2C, 0x32, 0x424, 0x42A },        /* 6.144M call */
			{ 0xCD, 0xED, 0x48D, 0x4AD },        /* 24.576M normal */
			{ 0xC7, 0xE7, 0x487, 0x4A7 },        /* 24.576M anc call */
			{ 0, 0, 0, 0 },                      /* 24.576M direct play */
			{ 0, 0, 0, 0 },                      /* image */
			{ 0, 0, 0, 0 },                      /* 6.144M fast play */
			{ 0, 0, 0, 0 },                      /* 24.576M fast play & rec */
			{ 0, 0, 0, 0 },                      /* 24.576M fast play & st */
			{ 0x4E, 0x34, 0x6E, 0x74 },          /* 12.288M call */
			{ 0, 0, 0, 0 },                      /* 6.144M soundtrigger enhance */
};

static uint16_t sd_voice_down_32k[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0, 0 },                            /* 6.144M FPGA voice down 32k */
			{ 0, 0 },                            /* 6.144M play */
			{ 0x16, 0x41E },                     /* 6.144M call */
			{ 0x4D, 0x6D },                      /* 24.576M normal */
			{ 0x47, 0x67 },                      /* 24.576M anc call */
			{ 0, 0 },                            /* 24.576M direct play */
			{ 0, 0 },                            /* image */
			{ 0, 0 },                            /* 6.144M fast play */
			{ 0, 0 },                            /* 24.576M fast play & rec */
			{ 0, 0 },                            /* 24.576M fast play & st */
			{ 0x82E, 0x834 },                    /* 12.288M call */
			{ 0, 0 },                            /* 6.144M soundtrigger enhance */
};

static uint16_t sd_voice_up_32k[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0, 0, 0, 0 },                      /* 6.144M FPGA voice up 32k */
			{ 0, 0, 0, 0 },                      /* 6.144M PLAY */
			{ 0x1C, 0x412, 0x814, 0x81A },       /* 6.144M call */
			{ 0x44D, 0x46D, 0x84D, 0x86D },      /* 24.576M normal */
			{ 0x447, 0x467, 0x847, 0x867 },      /* 24.576M anc call */
			{ 0, 0, 0, 0 },                      /* 24.576M direct play */
			{ 0, 0, 0, 0 },                      /* image */
			{ 0, 0, 0, 0 },                      /* 6.144M fast play */
			{ 0, 0, 0, 0 },                      /* 24.576M fast play & rec */
			{ 0, 0, 0, 0 },                      /* 24.576M fast play & st */
			{ 0x2E, 0x34, 0x42E, 0x434 },        /* 12.288M call */
			{ 0, 0, 0, 0 },                      /* 6.144M soundtrigger enhance */
};

static uint16_t sd_soundtrigger_48k[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0 },                              /* 6.144M FPGA */
			{ 0 },                              /* 6.144M play */
			{ 0 },                              /* 6.144M call */
			{ 0xC93 },                          /* 24.576M normal */
			{ 0 },                              /* 24.576M anc call */
			{ 0xC82 },                          /* 24.576M direct play */
			{ 0 },                              /* image */
			{ 0 },                              /* 6.144M fast play */
			{ 0 },                              /* 24.576M fast play & rec */
			{ 0xC98 },                          /* 24.576M fast play & st */
			{ 0 },                              /* 12.288M call */
			{ 0 },                              /* 6.144M soundtrigger enhance */
};

static uint16_t sd_debug_192k[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0 },                              /* 6.144M fpga */
			{ 0 },                              /* 6.144M play */
			{ 0 },                              /* 6.144M call */
			{ 0xC33 },                          /* 24.576M normal */
			{ 0xC2D, 0xC35 },                   /* 24.576M anc call*/
			{ 0 },                              /* 24.576M direct play */
			{ 0 },                              /* image */
			{ 0 },                              /* 6.144M fast play */
			{ 0 },                              /* 24.576M fast play & rec  */
			{ 0 },                              /* 24.576M fast play & st */
			{ 0 },                              /* 12.288M call */
			{ 0 },                              /* 6.144M soundtrigger enhance */
};
static uint16_t sd_direct_play_96k[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0, 0 },                           /* 6.144M FPGA voice down 16k */
			{ 0, 0 },                           /* 6.144M play */
			{ 0, 0 },                           /* 6.144M call */
			{ 0, 0 },                           /* 24.576M normal */
			{ 0, 0 },                           /* 24.576M anc call */
			{ 0xC50, 0xC56 },                   /* 24.576M direct play */
			{ 0, 0 },                           /* image */
			{ 0, 0 },                           /* 6.144M fast play */
			{ 0xC4E, 0xC54 },                   /* 24.576M fast play & rec */
			{ 0xC50, 0xC56 },                   /* 24.576M fast play & st */
			{ 0, 0 },                           /* 12.288M call */
			{ 0, 0 },                           /* 6.144M soundtrigger enhance */
};
static uint16_t sd_direct_play_48k[SLIMBUS_SCENE_CONFIG_MAX][SLIMBUS_CHANNELS_MAX] = {
			{ 0, 0 },                           /* 6.144M FPGA voice down 16k */
			{ 0, 0 },                           /* 6.144M play */
			{ 0, 0 },                           /* 6.144M call */
			{ 0, 0 },                           /* 24.576M normal */
			{ 0, 0 },                           /* 24.576M anc call */
			{ 0xC90, 0xC96 },                   /* 24.576M direct play */
			{ 0, 0 },                           /* image */
			{ 0, 0 },                           /* 6.144M fast play */
			{ 0xC8E, 0xC94 },                   /* 24.576M fast play & rec */
			{ 0xC90, 0xC96 },                   /* 24.576M fast play & st */
			{ 0, 0 },                           /* 12.288M call */
			{ 0, 0 },                           /* 6.144M soundtrigger enhance */
};

static void slimbus_da_combine_v3_set_direct_play_sd(uint32_t scene_type, uint32_t ch,
	enum slimbus_presence_rate pr, uint16_t *sd)
{
	if (pr == SLIMBUS_PR_96K)
		*sd = sd_direct_play_96k[scene_type][ch];
	else if (pr == SLIMBUS_PR_48K)
		*sd = sd_direct_play_48k[scene_type][ch];
}

static void slimbus_da_combine_v3_set_para_sd(uint32_t track_type, uint32_t scene_type,
	uint32_t ch, enum slimbus_presence_rate pr, uint16_t *sd)
{
	*sd = sd_table[scene_type][track_type][ch];

	switch (track_type) {
	case SLIMBUS_TRACK_VOICE_DOWN:
		if (pr == SLIMBUS_PR_16K)
			*sd = sd_voice_down_16k[scene_type][ch];
		else if (pr == SLIMBUS_PR_32K)
			*sd = sd_voice_down_32k[scene_type][ch];
		break;
	case SLIMBUS_TRACK_VOICE_UP:
		if (pr == SLIMBUS_PR_16K)
			*sd = sd_voice_up_16k[scene_type][ch];
		else if (pr == SLIMBUS_PR_32K)
			*sd = sd_voice_up_32k[scene_type][ch];
		break;
	case SLIMBUS_TRACK_SOUND_TRIGGER:
		if (pr == SLIMBUS_PR_48K)
			*sd = sd_soundtrigger_48k[scene_type][ch];
		break;
	case SLIMBUS_TRACK_DEBUG:
		*sd = sd_debug_192k[scene_type][ch];
		break;
	case SLIMBUS_TRACK_DIRECT_PLAY:
		slimbus_da_combine_v3_set_direct_play_sd(scene_type, ch, pr, sd);
		break;
	default:
		break;
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
	struct slimbus_track_config *track_cfg = slimbus_get_track_config_table();
	struct slimbus_channel_property *pchprop = track_cfg[track].channel_pro;

	for (ch = 0; ch < track_cfg[track].params.channels; ch++) {
		memset(pchprop, 0, sizeof(struct slimbus_channel_property));

		pchprop->cn = *(*(chnum_table + track) + ch);
		pchprop->source.la = source_la;
		pchprop->source.pn = *(*(source_pn + track) + ch);
		pchprop->sink_num = 1;
		for (i = 0; i < pchprop->sink_num; i++) {
			pchprop->sinks[i].la = sink_la;
			pchprop->sinks[i].pn = *(*(sink_pn + track) + ch);
		}
		if (track == SLIMBUS_TRACK_ECREF) {
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

		pchprop->sl = sl_table[scene_type][track][ch];

		slimbus_da_combine_v3_set_para_sd(track,
			scene_type, ch, pchprop->pr, &(pchprop->sd));

		if (tp == SLIMBUS_TP_PUSHED)
			pchprop->dl = pchprop->sl - 1;
		else
			pchprop->dl = pchprop->sl;

		pchprop++;
	}
}

static void slimbus_da_combine_v3_param_init(const struct slimbus_device_info *dev)
{
	uint32_t track;
	enum slimbus_transport_protocol  tp = SLIMBUS_TP_ISOCHRONOUS;
	uint8_t source_la = SOC_LA_GENERIC_DEVICE;
	uint8_t sink_la = DA_COMBINE_LA_GENERIC_DEVICE;
	uint32_t scene_type;

	if (dev == NULL) {
		AUDIO_LOGE("dev is null");
		return;
	}

	if (dev->scene_config_type >= SLIMBUS_SCENE_CONFIG_MAX) {
		AUDIO_LOGE("track type is invalid, scene type: %d", dev->scene_config_type);
		return;
	}

	scene_type = dev->scene_config_type;

	for (track = 0; track < SLIMBUS_TRACK_MAX; track++) {
		tp = SLIMBUS_TP_ISOCHRONOUS;
		source_la = SOC_LA_GENERIC_DEVICE;
		sink_la = DA_COMBINE_LA_GENERIC_DEVICE;

		slimbus_drv_get_params_la(track, &source_la, &sink_la, &tp);
		config_param(track, scene_type, source_la, sink_la, tp);
	}
}

static int32_t slimbus_da_combine_v3_param_set(struct slimbus_device_info *dev,
	uint32_t track_type, struct slimbus_track_param *params)
{
	struct slimbus_channel_property *pchprop = NULL;
	uint32_t scene_type = SLIMBUS_SCENE_CONFIG_MAX;
	struct slimbus_track_param slimbus_param;
	uint32_t ch;
	struct slimbus_track_config *track_cfg = slimbus_get_track_config_table();

	if (!dev) {
		AUDIO_LOGE("dev is NULL");
		return -EINVAL;
	}
	if (track_type >= SLIMBUS_TRACK_MAX) {
		AUDIO_LOGE("track type is invalid, track type: %d", track_type);
		return -EINVAL;
	}
	if (dev->scene_config_type >= SLIMBUS_SCENE_CONFIG_MAX) {
		AUDIO_LOGE("scene type is invalid, scene type: %d", dev->scene_config_type);
		return -EINVAL;
	}

	scene_type = dev->scene_config_type;

	if (!params) {
		memcpy(&slimbus_param, &(track_cfg[track_type].params), sizeof(slimbus_param));
		params = &slimbus_param;
	}
	AUDIO_LOGI("track type: %d, rate: %d, chnum: %d", track_type, params->rate, params->channels);

	track_cfg[track_type].params.channels = params->channels;
	track_cfg[track_type].params.rate = params->rate;
	track_cfg[track_type].params.callback = params->callback;

	pchprop = track_cfg[track_type].channel_pro; /*lint!e838*/

	if (track_type == SLIMBUS_TRACK_VOICE_UP)
		dev->voice_up_chnum = params->channels;
	else if (track_type == SLIMBUS_TRACK_AUDIO_CAPTURE)
		dev->audio_up_chnum = params->channels;
	else if (track_type == SLIMBUS_TRACK_VOICE_DOWN)
		track_cfg[track_type].params.channels = 2;

	if (pchprop) {
		for (ch = 0; ch < track_cfg[track_type].params.channels; ch++) {

			slimbus_da_combine_set_para_pr(pr_table, track_type, params);

			pchprop->pr = pr_table[track_type];

			slimbus_da_combine_v3_set_para_sd(track_type, scene_type,
				ch, pchprop->pr, &(pchprop->sd));

			pchprop++;
		}
	}

	return 0;
}

static void slimbus_da_combine_v3_param_update(const struct slimbus_device_info *dev,
	uint32_t track_type, const struct slimbus_track_param *params)
{
	if (track_type >= SLIMBUS_TRACK_MAX) {
		AUDIO_LOGE("invalid track type: %d", track_type);
		return;
	}

	slimbus_da_combine_set_para_pr(pr_table, track_type, params);
	slimbus_da_combine_v3_param_init(dev);
}

/*
 * create da_combine_v3 slimbus device
 * @device, pointer to created instance
 *
 * return 0, if success
 */
static int32_t slimbus_da_combine_v3_create_device(struct slimbus_device_info **device)
{
	struct slimbus_device_info *dev_info = NULL;

	if (!device) {
		AUDIO_LOGE("device param is null");
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
	dev_info->audio_up_chnum = SLIMBUS_AUDIO_CAPTURE_MULTI_MIC_CHANNELS; // default is 4mic
	dev_info->page_sel_addr = 1;
	dev_info->sm = SLIMBUS_SM_1_CSW_32_SL;
	dev_info->cg = SLIMBUS_CG_10;
	dev_info->scene_config_type = SLIMBUS_SCENE_CONFIG_NORMAL;

	dev_info->slimbus_da_combine_para->slimbus_track_max = SLIMBUS_TRACK_MAX;
	dev_info->slimbus_da_combine_para->track_config_table = slimbus_get_track_config_table();

	mutex_init(&dev_info->rw_mutex);
	mutex_init(&dev_info->track_mutex);

	*device = dev_info;

	return 0;
}

static void slimbus_da_combine_v3_get_st_params(struct slimbus_sound_trigger_params *params)
{
	if (params != NULL) {
		params->sample_rate = SLIMBUS_SAMPLE_RATE_48K;
		params->channels = 2;
		params->track_type = SLIMBUS_TRACK_AUDIO_CAPTURE;
	}
}

void slimbus_da_combine_v3_callback_register(struct slimbus_device_ops *dev_ops,
	struct slimbus_private_data *pd)
{
	if (dev_ops == NULL) {
		AUDIO_LOGE("input param is null");
		return;
	}

	dev_ops->create_slimbus_device = slimbus_da_combine_v3_create_device;
	dev_ops->slimbus_device_param_set = slimbus_da_combine_v3_param_set;
	dev_ops->slimbus_device_param_init = slimbus_da_combine_v3_param_init;
	dev_ops->slimbus_device_param_update = slimbus_da_combine_v3_param_update;
	dev_ops->slimbus_get_soundtrigger_params = slimbus_da_combine_v3_get_st_params;
}

