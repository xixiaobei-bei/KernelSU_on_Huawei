/*
 * ak8987.h
 *
 * hall_ak8987 driver
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

#ifndef AK8789_H
#define AK8789_H

#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/pm_wakeup.h>
#include <linux/time.h>
#include <linux/gpio.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/irq.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#include "hall_sensor.h"

#ifdef CONFIG_HUAWEI_HALL_INPUTHUB
#define HALL_DATA_REPORT_INPUTHUB
#endif

#ifdef CONFIG_HUAWEI_HALL_INPUTDEV
#define HALL_DATA_REPORT_INPUTDEV
#endif

#if defined(CONFIG_INPUTHUB) || defined(CONFIG_INPUTHUB_20)
#define HALL_INPUTHUB_ROUTE
#endif

#if defined(CONFIG_HUAWEI_DSM) && defined(DSM_HALL_ERROR_NO)
#define HALL_DSM_CONFIG
#endif

/* dts name */
#define HALL_SINGLE_N_POLE       "huawei,single-n-pole"
#define HALL_SINGLE_S_POLE       "huawei,single-s-pole"
#define HALL_DOUBLE_POLE         "huawei,double-pole"
#define HALL_ID                  "huawei,id"
#define HALL_TYPE                "huawei,type"
#define HALL_HW_WAKEUP_SUPPORT   "huawei,hw-wakeup-support"
#define HALL_WAKEUP_FLAG         "huawei,wakeup-flag"
#define HALL_DEFAULT_STATE       "huawei,default-state"
#define HALL_X_COORDINATE        "huawei,x-coordinate"
#define HALL_Y_COORDINATE        "huawei,y-coordinate"
#define HALL_AUXILIARY_ID        "huawei,auxiliary-id"
#define HALL_AUXILIARY_N         "huawei,auxiliary-north"
#define HALL_AUXILIARY_S         "huawei,auxiliary-south"
#define HALL_GPIO_TYPE           "huawei,gpio-type"
#define HALL_INT_DELAY           "huawei,int-delay-time"
#define HALL_SENSOR_NAME         "hall,sensor"
#define NORTH_POLE_NAME          "hall_north_pole"
#define SOUTH_POLE_NAME          "hall_south_pole"
#define X_COR                    "x_coordinate"
#define Y_COR                    "y_coordinate"

#define HALL_MAX_STRING_LEN      16
#define HALL_MXA_INFO_LEN        256

#define GPIO_HIGH_VOLTAGE        1
#define GPIO_LOW_VOLTAGE         0
#define HALL_DEFAULT_INT_DELAY   50

#define HALL_IRQ_ABNORMAL_TIME   50000L

/* hall_type bite-0 instand north, bit-1 instand sourh */
#define support_type_north(type) ((((unsigned int)(type)) & 0x01) != 0)
#define support_type_south(type) ((((unsigned int)(type)) & 0x02) != 0)

struct hall_info {
	unsigned int h_type;
	unsigned int gpio[MAX_TYPE_NUM];
	int irq[MAX_TYPE_NUM];
	int trigger_style[MAX_TYPE_NUM];
	unsigned int auxiliary_io[MAX_TYPE_NUM];
	char name[MAX_TYPE_NUM][HALL_MAX_STRING_LEN];

#ifdef HALL_TEST
	atomic_t irq_err_count[MAX_TYPE_NUM];
	long irq_time;
	struct timeval last_time[MAX_TYPE_NUM];
	struct timeval now_time[MAX_TYPE_NUM];
#endif
};

struct hall_device {
	struct platform_device *pdev;
	unsigned int hall_id;
	unsigned int hall_type;
	unsigned int hall_wakeup_flag;
	int hall_x_coordinate;
	int hall_y_coordinate;
	int hall_auxiliary_flag;
	int hall_hw_wakeup_support;
	int default_state;
	unsigned int hall_gpio_type;
	int hall_int_delay;
	struct list_head list;
	struct delayed_work h_delayed_work;
	char name[HALL_MAX_STRING_LEN];
	struct hall_info h_info;
};

struct ak8789_data {
	struct hall_cdev cdev;
	struct list_head head;
	struct platform_device *pdev;
	int count;
	int state;
	struct workqueue_struct *hall_wq;
	struct wakeup_source wakelock;
	spinlock_t spinlock;
	struct work_struct inter_work;

#ifdef HALL_DATA_REPORT_INPUTDEV
	struct input_dev *input_dev;
#endif

#ifdef HALL_TEST
	int enable;
	int report_count;
	packet_data value;
	int flag;
	int period;
#ifdef HALL_DSM_CONFIG
	struct dsm_client *hall_dclient;
#endif
#endif
};

#ifdef HALL_DATA_REPORT_INPUTDEV
int ak8789_input_register(struct ak8789_data *data);
void ak8789_input_unregister(struct ak8789_data *data);
#endif
int ak8789_report_data(void *data, packet_data pdata);
#endif
