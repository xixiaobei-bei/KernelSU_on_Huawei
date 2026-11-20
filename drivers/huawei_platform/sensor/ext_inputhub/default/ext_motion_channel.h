/*
 * ext_sensorhub_api.h
 *
 * head file for external sensorhub channel kernel api
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef EXT_MOTION_CHANNEL_H
#define EXT_MOTION_CHANNEL_H

#define c32_b2l(x) (unsigned int)((((unsigned int)(x)&0xff000000) >> 24) | \
(((unsigned int)(x)&0x00ff0000) >> 8) | \
(((unsigned int)(x)&0x0000ff00) << 8) | (((unsigned int)(x)&0x000000ff) << 24))

#define MOTION_TYPE_WRIST_TILT 14
#define MOTION_TYPE_FINGER_GESTURE 16
#define MOTION_SENSOR_TYPE 0x48
#define MOTION_FINGER_SENSOR_TYPE 0x49

#endif /* EXT_MOTION_CHANNEL_H */
