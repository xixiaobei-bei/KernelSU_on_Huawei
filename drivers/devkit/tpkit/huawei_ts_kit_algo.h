/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __HUAWEI_TOUCHSCREEN_ALGO_H_
#define __HUAWEI_TOUCHSCREEN_ALGO_H_

#include "huawei_ts_kit.h"
#define PEN_MOV_LENGTH 120 /* move length (pixels) */

/*
 * the time pen checked after finger released
 * shouldn't less than this value(ms)
 */
#define FINGER_REL_TIME 300

#define AFT_EDGE 1
#define NOT_AFT_EDGE 0

#define FINGER_PRESS_TIME_MIN_ALGO1 (20 * 1000) /* mil second */
#define FINGER_PRESS_TIME_MIN_ALGO3 (20 * 1000) /* mil second */
#define FINGER_PRESS_TIMES_ALGO1 5
#define FINGER_PRESS_TIMES_ALGO2 5
#define FINGER_PRESS_TIMES_ALGO3 3
int ts_kit_register_algo_func(struct ts_kit_device_data *chip_data);
#endif
