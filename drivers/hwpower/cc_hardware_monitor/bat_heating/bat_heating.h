/*
 * bat_heating.h
 *
 * battery heating of low temperature control driver
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

#ifndef _BAT_HEATING_H_
#define _BAT_HEATING_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fb.h>

#define BAT_HEATING_RW_BUF_SIZE              32

#define DELAY_TIME_FOR_INIT_MONITOR_WORK     10000 /* 10s */
#define DELAY_TIME_FOR_MONITOR_WORK          5000 /* 5s */
#define DELAY_TIME_FOR_COUNT_WORK            10000 /* 10s */
#define DELAY_TIME_FOR_DMD_WORK              3000 /* 3s */

#define DEFAULT_LOW_TEMP_MIN_THLD            (-10)
#define DEFAULT_LOW_TEMP_MAX_THLD            5
#define DEFAULT_LOW_TEMP_HYSTERESIS          35
#define DEFAULT_LOW_TEMP_MIN_IBAT            (-400)
#define DEFAULT_BUCK_IIN_LIMIT               1300 /* ma */

#define BAT_HEATING_DMD_REPORT_COUNTS        5
#define BAT_HEATING_FULL_SOC                 99
#define BAT_HEATING_USB_TEMP_THLD            15
#define BAT_HEATING_DC_TEMP_THLD             10
#define BAT_HEATING_OVERLOAD_THLD            20

enum bat_heating_sysfs_type {
	BAT_HEATING_SYSFS_BEGIN = 0,
	BAT_HEATING_SYSFS_COUNT = BAT_HEATING_SYSFS_BEGIN,
	BAT_HEATING_SYSFS_TRIGGER,
	BAT_HEATING_SYSFS_END,
};

enum bat_heating_op_user {
	BAT_HEATING_OP_USER_BEGIN = 0,
	BAT_HEATING_OP_USER_SHELL = BAT_HEATING_OP_USER_BEGIN, /* for shell */
	BAT_HEATING_OP_USER_BMS_HEATING, /* for bms_heating daemon */
	BAT_HEATING_OP_USER_END,
};

enum bat_heating_uevent_type {
	BAT_HEATING_UEVENT_BEGIN = 0,
	BAT_HEATING_START_HEAT_UP = BAT_HEATING_UEVENT_BEGIN,
	BAT_HEATING_STOP_HEAT_UP,
	BAT_HEATING_POPUP_UI_MSG,
	BAT_HEATING_REMOVE_UI_MSG,
	BAT_HEATING_UEVENT_END,
};

enum bat_heating_usb_state {
	BAT_HEATING_USB_DISCONNECT,
	BAT_HEATING_USB_CONNECT,
};

enum bat_heating_screen_state {
	BAT_HEATING_SCREEN_ON,
	BAT_HEATING_SCREEN_OFF,
};

#define BAT_HEATING_TEMP_LEVEL    8

enum bat_heating_temp_info {
	BAT_HEATING_TEMP_MIN = 0,
	BAT_HEATING_TEMP_MAX,
	BAT_HEATING_TEMP_ICHG,
	BAT_HEATING_TEMP_TOTAL,
};

struct bat_heating_temp_para {
	int temp_min;
	int temp_max;
	int temp_ichg;
};

struct bat_heating_dev {
	struct device *dev;
	struct notifier_block nb;
	struct notifier_block fb_nb;
	struct delayed_work monitor_work;
	struct delayed_work rd_count_work;
	struct delayed_work wr_count_work;
	struct delayed_work rpt_dmd_work;
	int dmd_count;
	int hysteresis;
	int count;
	int trigger;
	int bat_temp;
	int usb_temp;
	int usb_state;
	int screen_state;
	int low_temp_min_thld;
	int low_temp_max_thld;
	int low_temp_hysteresis;
	int low_temp_min_ibat;
	int buck_iin_limit;
	struct bat_heating_temp_para temp_para[BAT_HEATING_TEMP_LEVEL];
	bool heat_up_flag;
	bool ui_msg_flag;
	int overload_count;
};

#endif /* _BAT_HEATING_H_ */
