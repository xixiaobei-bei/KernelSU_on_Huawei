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

#ifndef _SLIMBUS_6403_H_
#define _SLIMBUS_6403_H_

#include "slimbus_types.h"

extern int32_t slimbus_hi6403_create_device(struct slimbus_device_info **device);

extern void slimbus_hi6403_param_init(const struct slimbus_device_info *dev);

extern void slimbus_hi6403_param_update(
			const struct slimbus_device_info *dev,
			uint32_t track_type,
			const struct slimbus_track_param *params);

extern int32_t slimbus_hi6403_param_set(
			struct slimbus_device_info *dev,
			uint32_t track_type,
			struct slimbus_track_param *params);

extern void slimbus_hi6403_get_st_params(struct slimbus_sound_trigger_params *params);
#endif

