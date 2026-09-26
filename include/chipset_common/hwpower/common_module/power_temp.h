/*
 * power_temp.h
 *
 * temperature interface for power module
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

#ifndef _POWER_TEMP_H_
#define _POWER_TEMP_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/thermal.h>

#define POWER_TEMP_RD_BUF_SIZE     16
#define POWER_TEMP_RETRYS          3
#define POWER_TEMP_SAMPLES         3
#define POWER_TEMP_UNIT            1000
#define POWER_TEMP_VALID           0
#define POWER_TEMP_INVALID         1
#define POWER_TEMP_INVALID_TEMP    (-40000)
#define POWER_TEMP_DIFF_THLD       10000

#define POWER_TEMP_USB_PORT        "usb_port"
#define POWER_TEMP_SERIAL_BAT_0    "bat_0"
#define POWER_TEMP_SERIAL_BAT_1    "bat_1"
#define POWER_TEMP_BTB_NTC_AUX     "btb_ntc_aux"
#define POWER_TEMP_RF_BOARD        "rfboard"

struct power_temp_dev {
	struct device *dev;
};

int power_temp_get_raw_value(const char *sensor_name);
int power_temp_get_average_value(const char *sensor_name);

#endif /* _POWER_TEMP_H_ */
