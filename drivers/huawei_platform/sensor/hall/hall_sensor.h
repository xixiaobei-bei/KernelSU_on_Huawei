/*
 * hall_sensor.h
 *
 * hall_sensor driver
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#ifndef HALL_SENSOR_H
#define HALL_SENSOR_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/slab.h>

#define HALL_TEST

#ifdef HALL_TEST
#define HALL_REPORT_MAX_COUNT   1000
#define HALL_REPORT_MIN_COUNT   1
#define HALL_REPORT_MAX_PERIOD  1000
#define HALL_REPORT_MIN_PERIOD  50
#endif

#define MAX_INFO_LEN            256
#define MAX_BUF_SIZE            50

typedef unsigned int packet_data; // in order to avoid expose variable type

enum hall_type {
	NORTH,
	SOUTH,
	MAX_TYPE_NUM
};

struct hall_cdev {
	struct device *dev;
	int hall_count;
	int (*hall_report_debug_data)(struct hall_cdev *, packet_data);
	int (*hall_report_state)(struct hall_cdev *);
	int (*hall_get_state)(struct hall_cdev *);
	int (*hall_get_info)(struct hall_cdev *, char *, unsigned int);
#ifdef HALL_TEST
	int count;
	packet_data value;
	int flag;
	int period;
	int enable;
	void (*hall_set_report_value)(struct hall_cdev *);
#endif
};

int hall_first_report(bool enable);
int ak8789_register_report_data(int ms);
int hall_register(struct hall_cdev *cdev);
void hall_unregister(struct hall_cdev *cdev);
#endif
