/*
 * antenna_boardid_gpio_detect.h
 *
 * Antenna boardid detect driver header file which use gpio.
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

#ifndef KERNEL_INCLUDE_ANTENNA_BOARDID_GPIO_DETECT
#define KERNEL_INCLUDE_ANTENNA_BOARDID_GPIO_DETECT
#include <linux/device.h>

struct antenna_device_info {
	struct device *dev;
};

enum antenna_type {
	ANTENNA_BOARDID_GPIO_DETECT,
	ANTENNA_BOARDID_GPIO_STATUS,
};

#define ANATENNA_DETECT_SUCCEED 1
#define ANATENNA_DETECT_FAIL    0
#endif
