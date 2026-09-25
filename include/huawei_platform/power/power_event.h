/*
 * power_event.h
 *
 * event for power module
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_EVENT_H_
#define _POWER_EVENT_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define POWER_EVENT_NOTIFY_SIZE 128
#define POWER_EVENT_NOTIFY_NUM  2

#define POWER_EVENT_INVAID      (-1)
#define VBUS_CHECK_WORK_TIME    3000 /* 3s */
#define VBUS_ABSENT_CNTS        2

enum power_event_sysfs_type {
	POWER_EVENT_SYSFS_BEGIN = 0,
	POWER_EVENT_SYSFS_CONNECT_STATE = POWER_EVENT_SYSFS_BEGIN,
	POWER_EVENT_SYSFS_VBUS_STATE,
	POWER_EVENT_SYSFS_END,
};

enum power_event_ne_list {
	POWER_EVENT_NE_BEGIN = 0,
	POWER_EVENT_NE_USB_DISCONNECT = POWER_EVENT_NE_BEGIN,
	POWER_EVENT_NE_USB_CONNECT,
	POWER_EVENT_NE_WIRELESS_DISCONNECT,
	POWER_EVENT_NE_WIRELESS_CONNECT,
	POWER_EVENT_NE_VBUS_CHECK,
	POWER_EVENT_NE_AUTH_DC_SC,
	POWER_EVENT_NE_AUTH_WL_SC,
	POWER_EVENT_NE_AUTH_WL_SC_UVDM,
	POWER_EVENT_NE_FW_UPDATE_WIRELESS,
	POWER_EVENT_NE_FW_UPDATE_ADAPTER,
	POWER_EVENT_NE_UI_CABLE_TYPE,
	POWER_EVENT_NE_UI_MAX_POWER,
	POWER_EVENT_NE_UI_WL_OFF_POS,
	POWER_EVENT_NE_UI_WL_FAN_STATUS,
	POWER_EVENT_NE_UI_WL_COVER_STATUS,
	POWER_EVENT_NE_UI_WATER_STATUS,
	POWER_EVENT_NE_UI_HEATING_STATUS,
	POWER_EVENT_NE_END,
};

enum power_event_connect_state {
	POWER_EVENT_DISCONNECT,
	POWER_EVENT_CONNECT,
};

enum power_event_vbus_state {
	POWER_EVENT_PRESENT,
	POWER_EVENT_ABSENT,
};

struct power_event_notify_data {
	const char *event;
	unsigned char event_len;
};

struct power_event_dev {
	struct device *dev;
	struct notifier_block nb;
	struct kobject *sysfs_ne;
	int connect_state;
	struct delayed_work vbus_check_work;
	int vbus_absent_cnt;
	int vbus_state;
};

void power_event_notify(unsigned long event, void *data);

#endif /* _POWER_EVENT_H_ */
