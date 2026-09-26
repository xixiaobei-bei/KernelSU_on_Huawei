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

#ifndef __SLIMBUS_DA_COMBINE_V5_H__
#define __SLIMBUS_DA_COMBINE_V5_H__

#include "slimbus_types.h"
#include "slimbus.h"

enum slimbus_scene_config_da_combine_v5 {
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_6144_FPGA          = 0,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_PLAY               = 1,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_CALL               = 2,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_NORMAL             = 3,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_DIRECT_PLAY        = 4,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_CALL_12288         = 5,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_ENHANCE_ST_6144    = 6,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_4PA_MOVEUP         = 7,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_DIRECT_PLAY_441    = 8,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_4PA_CALL           = 9,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_BT_SCO             = 10,
	SLIMBUS_DA_COMBINE_V5_SCENE_CONFIG_MAX,
};

enum slimbus_da_combine_v5_track {
	SLIMBUS_DA_COMBINE_V5_TRACK_BEGIN = 0,
	SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY = SLIMBUS_DA_COMBINE_V5_TRACK_BEGIN,
	SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_CAPTURE,
	SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_DOWN,
	SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP,
	SLIMBUS_DA_COMBINE_V5_TRACK_ECREF,
	SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER,
	SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG,
	SLIMBUS_DA_COMBINE_V5_TRACK_DIRECT_PLAY,
	SLIMBUS_DA_COMBINE_V5_TRACK_2PA_IV,
	SLIMBUS_DA_COMBINE_V5_TRACK_4PA_IV,
	SLIMBUS_DA_COMBINE_V5_TRACK_AUDIO_PLAY_D3D4,
	SLIMBUS_DA_COMBINE_V5_TRACK_KWS,
	SLIMBUS_DA_COMBINE_V5_TRACK_5MIC_CAPTURE,
	SLIMBUS_DA_COMBINE_V5_TRACK_VOICE_UP_4PA,
	SLIMBUS_DA_COMBINE_V5_TRACK_BT_DOWN,
	SLIMBUS_DA_COMBINE_V5_TRACK_BT_UP,
	SLIMBUS_DA_COMBINE_V5_TRACK_ULTRA_UP,
	SLIMBUS_DA_COMBINE_V5_TRACK_MAX,
};

void slimbus_da_combine_v5_callback_register(struct slimbus_device_ops *dev_ops,
	struct slimbus_private_data *pd);

#endif

