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

#ifndef __HUAWEI_TS_KIT_MISC_DEV_H_
#define __HUAWEI_TS_KIT_MISC_DEV_H_

#include <linux/ioctl.h>
#include <linux/miscdevice.h>

#define DEVICE_AFT_GET_INFO  "ts_aft_get_info"
#define DEVICE_AFT_SET_INFO  "ts_aft_set_info"

/* commands */
#define  INPUT_AFT_GET_IO_TYPE 0xBA
#define  INPUT_AFT_SET_IO_TYPE 0xBB

#define INPUT_AFT_IOCTL_CMD_GET_TS_FINGERS_INFO \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x01, struct ts_fingers)

#define INPUT_AFT_IOCTL_CMD_GET_ALGO_PARAM_INFO \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x02, struct ts_aft_algo_param)

#define INPUT_AFT_IOCTL_CMD_SET_COORDINATES \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x01, struct ts_fingers)

#endif
