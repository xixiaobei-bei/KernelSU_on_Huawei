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

#ifndef _SLIMBUS_6405_H_
#define _SLIMBUS_6405_H_


#include "slimbus_types.h"

typedef enum {
	SLIMBUS_6405_SCENE_CONFIG_6144_FPGA          = 0,
	SLIMBUS_6405_SCENE_CONFIG_PLAY               = 1,
	SLIMBUS_6405_SCENE_CONFIG_CALL               = 2,
	SLIMBUS_6405_SCENE_CONFIG_NORMAL             = 3,
	SLIMBUS_6405_SCENE_CONFIG_DIRECT_PLAY        = 4,
	SLIMBUS_6405_SCENE_CONFIG_CALL_12288         = 5,
	SLIMBUS_6405_SCENE_CONFIG_ENHANCE_ST_6144    = 6,
	SLIMBUS_6405_SCENE_CONFIG_4PA_MOVEUP          = 7,
	SLIMBUS_6405_SCENE_CONFIG_MAX,
} slimbus_scene_config_6405_type_t;

typedef enum {
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
	SLIMBUS_6405_TRACK_MAX,
} slimbus_6405_track_type_t;

extern slimbus_track_config_t track_config_6405_table[];

extern int create_hi6405_slimbus_device(slimbus_device_info_t **device);

extern void slimbus_hi6405_param_init(slimbus_device_info_t *dev);

extern void slimbus_hi6405_param_update(
			slimbus_device_info_t *dev,
			uint32_t track_type,
			slimbus_track_param_t *params);

extern int slimbus_hi6405_param_set(
			slimbus_device_info_t *dev,
			uint32_t track_type,
			slimbus_track_param_t *params);

extern int slimbus_hi6405_check_scenes(
				uint32_t track,
				uint32_t scenes,
				bool track_enable);

extern int slimbus_hi6405_select_scenes(
				struct slimbus_device_info *dev,
				uint32_t track,
				slimbus_track_param_t *params,
				bool track_enable);

extern int slimbus_hi6405_track_soundtrigger_activate(
						uint32_t track,
						bool slimbus_dynamic_freq_enable,
						struct slimbus_device_info *dev,
						slimbus_track_param_t *params);

extern int slimbus_hi6405_track_soundtrigger_deactivate(uint32_t track);

extern bool slimbus_hi6405_track_is_fast_soundtrigger(uint32_t track);

extern void slimbus_hi6405_check_st_conflict(uint32_t track, slimbus_track_param_t *params);

extern void slimbus_hi6405_get_st_params(slimbus_sound_trigger_params_t *params);

#endif

