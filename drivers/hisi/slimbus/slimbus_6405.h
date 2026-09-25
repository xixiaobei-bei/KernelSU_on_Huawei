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

#ifndef _SLIMBUS_6405_H_
#define _SLIMBUS_6405_H_

#include "slimbus_types.h"
#include "slimbus.h"

enum slimbus_scene_config_6405 {
	SLIMBUS_6405_SCENE_CONFIG_6144_FPGA          = 0,
	SLIMBUS_6405_SCENE_CONFIG_PLAY               = 1,
	SLIMBUS_6405_SCENE_CONFIG_CALL               = 2,
	SLIMBUS_6405_SCENE_CONFIG_NORMAL             = 3,
	SLIMBUS_6405_SCENE_CONFIG_DIRECT_PLAY        = 4,
	SLIMBUS_6405_SCENE_CONFIG_CALL_12288         = 5,
	SLIMBUS_6405_SCENE_CONFIG_ENHANCE_ST_6144    = 6,
	SLIMBUS_6405_SCENE_CONFIG_4PA_MOVEUP         = 7,
	SLIMBUS_6405_SCENE_CONFIG_DIRECT_PLAY_441    = 8,
	SLIMBUS_6405_SCENE_CONFIG_4PA_CALL           = 9,
	SLIMBUS_6405_SCENE_CONFIG_MAX,
};

enum slimbus_6405_track {
	SLIMBUS_6405_TRACK_BEGIN = 0,
	SLIMBUS_6405_TRACK_AUDIO_PLAY = SLIMBUS_6405_TRACK_BEGIN,
	SLIMBUS_6405_TRACK_AUDIO_CAPTURE,
	SLIMBUS_6405_TRACK_VOICE_DOWN,
	SLIMBUS_6405_TRACK_VOICE_UP,
	SLIMBUS_6405_TRACK_ECREF,
	SLIMBUS_6405_TRACK_SOUND_TRIGGER,
	SLIMBUS_6405_TRACK_DEBUG,
	SLIMBUS_6405_TRACK_DIRECT_PLAY,
	SLIMBUS_6405_TRACK_2PA_IV,
	SLIMBUS_6405_TRACK_4PA_IV,
	SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4,
	SLIMBUS_6405_TRACK_KWS,
	SLIMBUS_6405_TRACK_5MIC_CAPTURE,
	SLIMBUS_6405_TRACK_VOICE_UP_4PA,
	SLIMBUS_6405_TRACK_MAX,
};

extern struct slimbus_track_config *slimbus_hi6405_get_track_config_table(void);

extern int32_t slimbus_hi6405_create_device(struct slimbus_device_info **device);

extern void slimbus_hi6405_param_init(const struct slimbus_device_info *dev);

extern void slimbus_hi6405_param_update(const struct slimbus_device_info *dev,
	uint32_t track_type, const struct slimbus_track_param *params);

extern int32_t slimbus_hi6405_param_set(struct slimbus_device_info *dev,
	uint32_t track_type, struct slimbus_track_param *params);

extern int32_t slimbus_hi6405_check_scenes(uint32_t track,
	uint32_t scenes, bool track_enable);

extern int32_t slimbus_hi6405_select_scenes(struct slimbus_device_info *dev,
	uint32_t track, const struct slimbus_track_param *params, bool track_enable);

extern int32_t slimbus_hi6405_track_soundtrigger_activate(uint32_t track,
	bool slimbus_dynamic_freq_enable, struct slimbus_device_info *dev, struct slimbus_track_param *params);

extern int32_t slimbus_hi6405_track_soundtrigger_deactivate(uint32_t track);

extern bool slimbus_hi6405_track_is_fast_soundtrigger(uint32_t track);

extern void slimbus_hi6405_get_st_params(struct slimbus_sound_trigger_params *params);

#endif

