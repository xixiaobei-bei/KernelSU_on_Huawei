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

#ifndef __SLIMBUS_TYPES_H__
#define __SLIMBUS_TYPES_H__

#include <linux/types.h>
#include <linux/mutex.h>

/* max channel number supported */
#define SLIMBUS_MAX_CHANNELS 16
#define SLIMBUS_SLINK_NUM 16

#define SLIMBUS_AUDIO_PLAYBACK_CHANNELS                 2
#define SLIMBUS_AUDIO_PLAYBACK_MULTI_PA_CHANNELS        4
#define SLIMBUS_AUDIO_CAPTURE_CHANNELS                  2
#define SLIMBUS_AUDIO_CAPTURE_MULTI_MIC_CHANNELS        4
#define SLIMBUS_AUDIO_CAPTURE_5MIC_CHANNELS             5
#define SLIMBUS_VOICE_DOWN_CHANNELS                     2
#define SLIMBUS_VOICE_UP_CHANNELS                       4
#define SLIMBUS_VOICE_UP_2CH                            2
#define SLIMBUS_IMAGE_DOWNLOAD_CHANNELS                 1
#define SLIMBUS_ECREF_CHANNELS                          2
#define SLIMBUS_ECREF_1CH                               1
#define SLIMBUS_SOUND_TRIGGER_CHANNELS                  2
#define SLIMBUS_VOICE_UP_SOUNDTRIGGER                   1
#define SLIMBUS_DEBUG_CHANNELS                          1
#define SLIMBUS_KWS_CHANNELS                            2

#define SLIMBUS_CHANNELS_MAX                            5
#define SLIMBUS_PORT_VERSION_MAX                        4
#define SLIMBUS_SAMPLE_RATE_8K                          8000
#define SLIMBUS_SAMPLE_RATE_16K                         16000
#define SLIMBUS_SAMPLE_RATE_32K                         32000
#define SLIMBUS_SAMPLE_RATE_48K                         48000
#define SLIMBUS_SAMPLE_RATE_96K                         96000
#define SLIMBUS_SAMPLE_RATE_192K                        192000
#define SLIMBUS_SAMPLE_RATE_384K                        384000
#define SLIMBUS_SAMPLE_RATE_768K                        768000
#define SLIMBUS_SAMPLE_RATE_44K1                        44100
#define SLIMBUS_SAMPLE_RATE_88K2                        88200
#define SLIMBUS_SAMPLE_RATE_176K4                       176400



/* user value / information elements slice sizes */
enum slimbus_slice_size {
	SLIMBUS_SS_1_BYTE      = 0, /* slice size is 1 byte. */
	SLIMBUS_SS_2_BYTES     = 1, /* slice size is 2 bytes. */
	SLIMBUS_SS_3_BYTES     = 2, /* slice size is 3 bytes. */
	SLIMBUS_SS_4_BYTES     = 3, /* slice size is 4 bytes. */
	SLIMBUS_SS_6_BYTES     = 4, /* slice size is 6 bytes. */
	SLIMBUS_SS_8_BYTES     = 5, /* slice size is 8 bytes. */
	SLIMBUS_SS_12_BYTES    = 6, /* slice size is 12 bytes. */
	SLIMBUS_SS_16_BYTES    = 7, /* slice size is 16 bytes. */
	SLIMBUS_SS_SLICE_BUT   = 8,
};

/* transport protocols */
enum slimbus_transport_protocol {
	SLIMBUS_TP_ISOCHRONOUS             = 0, /* isochronous protocol (multicast). */
	SLIMBUS_TP_PUSHED                  = 1, /* pushed protocol (multicast). */
	SLIMBUS_TP_PULLED                  = 2, /* pulled protocol (unicast). */
	SLIMBUS_TP_LOCKED                  = 3, /* locked protocol (multicast). */
	SLIMBUS_TP_ASYNC_SIMPLEX           = 4, /* asynchronous protocol - simplex (unicast). */
	SLIMBUS_TP_ASYNC_HALF_DUPLEX       = 5, /* asynchronous protocol - half-duplex (unicast). */
	SLIMBUS_TP_EXT_ASYNC_SIMPLEX       = 6, /* extended asynchronous protocol - simplex (unicast). */
	SLIMBUS_TP_EXT_ASYNC_HALF_DUPLEX   = 7, /* extended asynchronous protocol - half-duplex (unicast). */
	SLIMBUS_TP_USER_DEFINED_1          = 14, /* user defined 1. */
	SLIMBUS_TP_USER_DEFINED_2          = 15, /* user defined 2. */
};

/* presence rates */
enum slimbus_presence_rate {
	SLIMBUS_PR_12K      = 1,
	SLIMBUS_PR_24K      = 2,
	SLIMBUS_PR_48K      = 3,
	SLIMBUS_PR_96K      = 4,
	SLIMBUS_PR_192K     = 5,
	SLIMBUS_PR_384K     = 6,
	SLIMBUS_PR_768K     = 7,
	SLIMBUS_PR_11025    = 9,
	SLIMBUS_PR_22050    = 10,
	SLIMBUS_PR_44100    = 11,
	SLIMBUS_PR_88200    = 12,
	SLIMBUS_PR_176400   = 13,
	SLIMBUS_PR_352800   = 14,
	SLIMBUS_PR_705600   = 15,
	SLIMBUS_PR_4K       = 16,
	SLIMBUS_PR_8K       = 17,
	SLIMBUS_PR_16K      = 18,
	SLIMBUS_PR_32K      = 19,
	SLIMBUS_PR_64K      = 20,
	SLIMBUS_PR_128K     = 21,
	SLIMBUS_PR_256K     = 22,
	SLIMBUS_PR_512K     = 23,
};

/* data types */
enum slimbus_data_type {
	SLIMBUS_DF_NOT_INDICATED     = 0, /* not indicated. */
	SLIMBUS_DF_LPCM              = 1, /* LPCM audio. */
	SLIMBUS_DF_IEC61937          = 2, /* IEC61937 compressed audio. */
	SLIMBUS_DF_PACKED_PDM_AUDIO  = 3, /* packed PDM audio. */
	SLIMBUS_DF_USER_DEFINED_1    = 14, /* user defined 1. */
	SLIMBUS_DF_USER_DEFINED_2    = 15, /* user defined 2. */
};

/* auxilary field formats formats */
enum slimbus_aux_field_format {
	SLIMBUS_AF_NOT_APPLICABLE   = 0, /* not applicable. */
	SLIMBUS_AF_ZCUV             = 1, /* ZCUV for tunneling IEC60958. */
	SLIMBUS_AF_USER_DEFINED     = 11, /* user defined. */
};

/* subframe mode codings. CSW - control space width (slots), sl - subframe length (slots) */
enum slimbus_subframe_mode {
	SLIMBUS_SM_24_CSW_32_SL    = 31,
	SLIMBUS_SM_16_CSW_32_SL    = 29,
	SLIMBUS_SM_16_CSW_24_SL    = 28,
	SLIMBUS_SM_12_CSW_32_SL    = 27,
	SLIMBUS_SM_12_CSW_24_SL    = 26,
	SLIMBUS_SM_8_CSW_32_SL     = 25,
	SLIMBUS_SM_8_CSW_24_SL     = 24,
	SLIMBUS_SM_6_CSW_32_SL     = 23,
	SLIMBUS_SM_6_CSW_24_SL     = 22,
	SLIMBUS_SM_6_CSW_8_SL      = 21,
	SLIMBUS_SM_4_CSW_32_SL     = 19,
	SLIMBUS_SM_4_CSW_24_SL     = 18,
	SLIMBUS_SM_4_CSW_8_SL      = 17,
	SLIMBUS_SM_4_CSW_6_SL      = 16,
	SLIMBUS_SM_3_CSW_32_SL     = 15,
	SLIMBUS_SM_3_CSW_24_SL     = 14,
	SLIMBUS_SM_3_CSW_8_SL      = 13,
	SLIMBUS_SM_3_CSW_6_SL      = 12,
	SLIMBUS_SM_2_CSW_32_SL     = 11,
	SLIMBUS_SM_2_CSW_24_SL     = 10,
	SLIMBUS_SM_2_CSW_8_SL      = 9,
	SLIMBUS_SM_2_CSW_6_SL      = 8,
	SLIMBUS_SM_1_CSW_32_SL     = 7,
	SLIMBUS_SM_1_CSW_24_SL     = 6,
	SLIMBUS_SM_1_CSW_8_SL      = 5,
	SLIMBUS_SM_1_CSW_6_SL      = 4,
	SLIMBUS_SM_8_CSW_8_SL      = 0, /* 100% control space, 0% data space */
};

/** slimbus Frequencies and Clock Gear Codings. */
enum slimbus_clock_gear {
	SLIMBUS_CG_0 = 0,   /* not Indicated, Min: 0 MHz, max: 28.8 MHz */
	SLIMBUS_CG_1 = 1,   /* min: 0.025 MHz, max: 0.05625 MHz */
	SLIMBUS_CG_2 = 2,   /* min: 0.05 MHz, max: 0.1125 MHz */
	SLIMBUS_CG_3 = 3,   /* min: 0.1 MHz, max: 0.225 MHz */
	SLIMBUS_CG_4 = 4,   /* min: 0.2 MHz, max: 0.45 MHz */
	SLIMBUS_CG_5 = 5,   /* min: 0.4 MHz, max: 0.9 MHz */
	SLIMBUS_CG_6 = 6,   /* min: 0.8 MHz, max: 1.8 MHz */
	SLIMBUS_CG_7 = 7,   /* min: 1.6 MHz, max: 3.6 MHz */
	SLIMBUS_CG_8 = 8,   /* min: 3.2 MHz, max: 7.2 MHz */
	SLIMBUS_CG_9 = 9,   /* min: 6.4 MHz, max: 14.4 MHz */
	SLIMBUS_CG_10 = 10, /* min: 12.8 MHz, max: 28.8 MHz */
};


/** slimbus Root Frequency (RF) and Phase Modulus (PM) Codings. */
enum slimbus_root_frequency {
	SLIMBUS_RF_0 = 0,    /* rf: not indicated, pm: 160 */
	SLIMBUS_RF_1 = 1,    /* rf: 24.576 MHz, pm: 160 */
	SLIMBUS_RF_2 = 2,    /* rf: 22.5792 MHz, pm: 147 */
	SLIMBUS_RF_3 = 3,    /* rf: 15.36 MHz, pm: 100 */
	SLIMBUS_RF_4 = 4,    /* rf: 16.8 MHz, pm: 875 */
	SLIMBUS_RF_5 = 5,    /* rf: 19.2 MHz, pm: 125 */
	SLIMBUS_RF_6 = 6,    /* rf: 24 MHz, pm: 625 */
	SLIMBUS_RF_7 = 7,    /* rf: 25 MHz, pm: 15625 */
	SLIMBUS_RF_8 = 8,    /* rf: 26 MHz, pm: 8125 */
	SLIMBUS_RF_9 = 9,    /* rf: 27 MHz, pm: 5625 */
};

/* root frequency type */
enum slimbus_rf_type {
	SLIMBUS_RF_24576 = 0,
	SLIMBUS_RF_6144 = 1,
	SLIMBUS_RF_MAX
};

/* platform type */
enum platform_type {
	PLATFORM_PHONE = 0,
	PLATFORM_UDP = 1,
	PLATFORM_FPGA = 2,
};

/* a track is a combination of channels */
enum slimbus_track_type {
	SLIMBUS_TRACK_BEGIN = 0,
	SLIMBUS_TRACK_AUDIO_PLAY = SLIMBUS_TRACK_BEGIN,
	SLIMBUS_TRACK_AUDIO_CAPTURE,
	SLIMBUS_TRACK_VOICE_DOWN,
	SLIMBUS_TRACK_VOICE_UP,
	SLIMBUS_TRACK_IMAGE_LOAD,
	SLIMBUS_TRACK_ECREF,
	SLIMBUS_TRACK_SOUND_TRIGGER,
	SLIMBUS_TRACK_DEBUG,
	SLIMBUS_TRACK_DIRECT_PLAY,
	SLIMBUS_TRACK_FAST_PLAY,
	SLIMBUS_TRACK_KWS,
	SLIMBUS_TRACK_MAX,
};

/* slimbus channel index definition */
enum slimbus_channel_index {
	AUDIO_PLAY_CHANNEL_LEFT        = 1,
	AUDIO_PLAY_CHANNEL_RIGHT       = 2,
	AUDIO_CAPTURE_CHANNEL_LEFT     = 3,
	AUDIO_CAPTURE_CHANNEL_RIGHT    = 4,
	VOICE_DOWN_CHANNEL_LEFT        = 5,
	VOICE_DOWN_CHANNEL_RIGHT       = 6,
	VOICE_UP_CHANNEL_MIC1          = 7,
	VOICE_UP_CHANNEL_MIC2          = 8,
	VOICE_UP_CHANNEL_MIC3          = 9,
	VOICE_UP_CHANNEL_MIC4          = 10,

	IMAGE_DOWNLOAD                 = 11,
	VOICE_ECREF                    = 12,
	AUDIO_ECREF                    = 13,
	SOUND_TRIGGER_CHANNEL_LEFT     = 14,
	SOUND_TRIGGER_CHANNEL_RIGHT    = 15,
	AUDIO_CAPTURE_CHANNEL_MIC3     = 16,
	AUDIO_CAPTURE_CHANNEL_MIC4     = 17,
	DEBUG_LEFT                     = 18,
	DEBUG_RIGHT                    = 19,
	AUDIO_CAPTURE_CHANNEL_MIC5     = 19,
	AUDIO_PLAY_CHANNEL_D3          = 20,
	AUDIO_PLAY_CHANNEL_D4          = 21,
};


/* restart Time Values. */
enum slimbus_restart_time {
	/* after a restart request, the active Framer shall resume toggling the CLK line within four cycles of the
	 * CLK line frequency (as indicated by the Clock Gear and Root Frequency) used for the upcoming Frame. Optional.
	 */
	SLIMBUS_RT_FAST_RECOVERY = 0,
	/* after the restart request, the active Framer shall resume toggling the CLK line so
	 * the duration of the Pause is an integer number of Superframes in the upcoming Clock Gear. Optional.
	 */
	SLIMBUS_RT_CONSTANT_PHASE_RECOVERY = 1,
	/* after a restart request, the active Framer shall resume toggling the CLK line
	 * after an unspecified delay. Mandatory
	 */
	SLIMBUS_RT_UNSPECIFIED_DELAY = 2,
};

enum slimbus_dump_state {
	SLIMBUS_DUMP_IRQ       = 0,
	SLIMBUS_DUMP_FRAMER    = 1,
	SLIMBUS_DUMP_LOSTMS    = 2,
	SLIMBUS_DUMP_ALL       = 3,
	SLIMBUS_DUMP_MAX
};

enum slimbus_scene_config_type {
	SLIMBUS_SCENE_CONFIG_6144_FPGA          = 0,
	SLIMBUS_SCENE_CONFIG_PLAY               = 1,
	SLIMBUS_SCENE_CONFIG_CALL               = 2,
	SLIMBUS_SCENE_CONFIG_NORMAL             = 3,
	SLIMBUS_SCENE_CONFIG_ANC_CALL           = 4,
	SLIMBUS_SCENE_CONFIG_DIRECT_PLAY        = 5,
	SLIMBUS_SCENE_CONFIG_IMAGE_LOAD         = 6,
	SLIMBUS_SCENE_CONFIG_FAST_PLAY          = 7,
	SLIMBUS_SCENE_CONFIG_FAST_PLAY_AND_REC  = 8,
	SLIMBUS_SCENE_CONFIG_FAST_PLAY_AND_ST   = 9,
	SLIMBUS_SCENE_CONFIG_CALL_12288         = 10,
	SLIMBUS_SCENE_CONFIG_ENHANCE_ST_6144    = 11,
	SLIMBUS_SCENE_CONFIG_MAX                = 12,
};

struct slimbus_device_info {
	uint8_t generic_la; /* generic device logical address  */
	uint8_t voice_up_chnum; /* channel numbers of voice up */
	uint8_t audio_up_chnum; /* channel numbers of audio up */
	uint32_t page_sel_addr;
	struct mutex rw_mutex;

	enum slimbus_rf_type rf; /* root frequency */
	uint32_t slimbusclk_drv;
	uint32_t slimbusdata_drv;
	uint32_t slimbusclk_offset;
	uint32_t slimbusdata_offset;
	uint32_t slimbusclk_cfg_offset;
	uint32_t slimbusdata_cfg_offset;
	int32_t sm; /* subframe mode */
	int32_t cg; /* clock gear */
	uint32_t scene_config_type;
	struct mutex track_mutex;

	struct slimbus_64xx_config_para *slimbus_64xx_para;
	uint32_t voice_up_port_version;
	uint32_t audio_up_port_version;
};

/*
 * Data Channel Endpoints
 */
struct slimbus_port {
	uint8_t la; /* logical address */
	uint8_t pn; /* port number */
};

/*
 * Data Channel properties
 */
struct slimbus_channel_property {
	uint32_t cn; /* channel number */

	/* payload of CONNECT_SOURCE/SINK message */
	struct slimbus_port source;
	struct slimbus_port sinks[SLIMBUS_SLINK_NUM];
	uint32_t sink_num;

	/* payload of NEXT_DEFINE_CHANNEL message */
	enum slimbus_transport_protocol tp; /* transport protocol */
	uint16_t sd; /* segment_distribution */
	uint16_t sl; /* segment_length */

	/* payload of NEXT_DEFINE_CONTENT message */
	uint32_t fl; /* frequency locked_bit */
	enum slimbus_presence_rate pr; /* presence_rate */
	enum slimbus_aux_field_format af; /* auxiliary bit format */
	enum slimbus_data_type dt; /* dataType */
	uint8_t cl; /* channel link */
	uint8_t dl; /* data length */
};

typedef int32_t (*track_callback_t)(uint32_t int_type, const void *param);

/* configuration for a stream */
struct slimbus_track_param {
	uint32_t channels;
	uint32_t rate;
	track_callback_t callback;
};

struct slimbus_track_config {
	struct slimbus_track_param params;
	struct slimbus_channel_property *channel_pro;
};

struct slimbus_64xx_config_para {
	uint32_t slimbus_track_max;
	struct slimbus_track_config *track_config_table;
};

struct slimbus_bus_config {
	enum slimbus_subframe_mode sm; /* subframe mode */
	enum slimbus_clock_gear cg; /* clock gear */
	enum slimbus_root_frequency rf; /* root frequency */
};

struct select_scene_node {
	uint32_t scene_type;
	enum slimbus_clock_gear cg;
	enum slimbus_subframe_mode subframe_mode;
	bool (*is_scene_matched)(uint32_t);
};

#endif /* __SLIMBUS_TYPES_H__ */

