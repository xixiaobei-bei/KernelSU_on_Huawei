/*
 * antenna_board_adc_match.h
 *
 * Antenna boardid detect driver header file which use adc.
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

#ifndef KERNEL_INCLUDE_ANTENNA_BOARD_ADC_MATCH
#define KERNEL_INCLUDE_ANTENNA_BOARD_ADC_MATCH
#include <linux/device.h>

#define DEFAULT_ANTENNA_BOARD_ADC_CHANNEL (7)

struct antenna_adc_match_info {
	struct device *dev;
};

#endif