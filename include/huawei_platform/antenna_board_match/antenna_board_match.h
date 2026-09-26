/*
 * antenna_board_match.h
 *
 * Check the antenna board match status header file.
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef KERNEL_INCLUDE_ANTENNA_BOARD_MATCH
#define KERNEL_INCLUDE_ANTENNA_BOARD_MATCH
#include <linux/device.h>

struct antenna_device_ops {
	int (*get_antenna_match_status)(void);
	int (*get_antenna_board_voltage)(void);
};

enum antenna_sysfs_type {
	ANTENNA_BOARD_MATCH = 0,
	ANTENNA_BOARD_VOLTAGE,
};

struct antenna_device_info {
	struct device *dev;
	struct antenna_device_ops *ops;
};

int antenna_match_ops_register(struct antenna_device_ops *ops);
#endif

