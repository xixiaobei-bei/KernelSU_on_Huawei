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

#ifndef __SLIMBUS_DA_COMBINE_H__
#define __SLIMBUS_DA_COMBINE_H__

#include "slimbus_types.h"

/* slimbus soc side port definition */
enum slimbus_soc_port {
	AUDIO_PLAY_SOC_LEFT_PORT           = 0,
	AUDIO_PLAY_SOC_RIGHT_PORT          = 1,
	AUDIO_CAPTURE_SOC_LEFT_PORT        = 2,
	AUDIO_CAPTURE_SOC_RIGHT_PORT       = 3,
	IMAGE_DOWNLOAD_SOC_PORT            = 4,
	AUDIO_PLAY_SOC_D3_PORT             = 4,
	SUPER_PLAY_SOC_RIGHT_PORT          = 5,
	AUDIO_PLAY_SOC_D4_PORT             = 5,
	SLIMBUS_WAKEUP2MIC_SOC_LEFT_PORT   = 6,
	BT_CAPTURE_SOC_LEFT_PORT           = 6,
	BT_CAPTURE_SOC_RIGHT_PORT          = 7,
	MIC5_CAPTURE_SOC_RIGHT_PORT        = 7,
	VOICE_DOWN_SOC_LEFT_PORT           = 8,
	VOICE_DOWN_SOC_RIGHT_PORT          = 9,
	VOICE_UP_SOC_MIC1_PORT             = 10,
	VOICE_UP_SOC_MIC2_PORT             = 11,
	VOICE_UP_SOC_MIC3_PORT             = 12,
	VOICE_UP_SOC_MIC4_PORT             = 13,
	VOICE_SOC_ECREF_PORT               = 14,
	AUDIO_SOC_ECREF_PORT               = 15,
};

/* slimbus device side port definition */
enum slimbus_da_combine_port {
	AUDIO_PLAY_DA_COMBINE_LEFT_PORT          = 0,
	AUDIO_PLAY_DA_COMBINE_RIGHT_PORT         = 1,
	AUDIO_CAPTURE_DA_COMBINE_LEFT_PORT       = 2,
	AUDIO_CAPTURE_DA_COMBINE_RIGHT_PORT      = 3,
	IMAGE_DOWNLOAD_DA_COMBINE_PORT           = 4,
	AUDIO_PLAY_DA_COMBINE_D3_PORT            = 4,
	SUPER_PLAY_DA_COMBINE_PORT               = 5,
	AUDIO_PLAY_DA_COMBINE_D4_PORT            = 5,
	SLIMBUS_WAKEUP2MIC_DA_COMBINE_LEFT_PORT  = 6,
	BT_CAPTURE_DA_COMBINE_LEFT_PORT          = 6,
	BT_CAPTURE_DA_COMBINE_RIGHT_PORT         = 7,
	MIC5_CAPTURE_DA_COMBINE_RIGHT_PORT       = 7,
	VOICE_DOWN_DA_COMBINE_LEFT_PORT          = 8,
	VOICE_DOWN_DA_COMBINE_RIGHT_PORT         = 9,
	VOICE_UP_DA_COMBINE_MIC1_PORT            = 10,
	VOICE_UP_DA_COMBINE_MIC2_PORT            = 11,
	VOICE_UP_DA_COMBINE_MIC3_PORT            = 12,
	VOICE_UP_DA_COMBINE_MIC4_PORT            = 13,
	VOICE_DA_COMBINE_ECREF_PORT              = 14,
	AUDIO_DA_COMBINE_ECREF_PORT              = 15,
};

extern void slimbus_da_combine_release_device(struct slimbus_device_info *dev);

extern void slimbus_da_combine_set_para_pr(enum slimbus_presence_rate *pr_table,
	uint32_t track_type, const struct slimbus_track_param *params);

#endif

