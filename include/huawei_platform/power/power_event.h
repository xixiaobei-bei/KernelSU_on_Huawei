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

enum power_event_sysfs_type {
	POWER_EVENT_SYSFS_BEGIN = 0,
	POWER_EVENT_SYSFS_CONNECT_STATE = POWER_EVENT_SYSFS_BEGIN,
	POWER_EVENT_SYSFS_END,
};

enum power_event_ne_list {
	POWER_EVENT_NE_BEGIN = 0,
	POWER_EVENT_NE_USB_DISCONNECT = POWER_EVENT_NE_BEGIN,
	POWER_EVENT_NE_USB_CONNECT,
	POWER_EVENT_NE_WIRELESS_DISCONNECT,
	POWER_EVENT_NE_WIRELESS_CONNECT,
	POWER_EVENT_NE_END,
};

enum power_event_connect_state {
	POWER_EVENT_DISCONNECT,
	POWER_EVENT_CONNECT,
};

struct power_event_dev {
	struct device *dev;
	struct notifier_block nb;
	struct kobject *sysfs_ne;
	int connect_state;
};

void power_event_notify(unsigned long event, void *data);

#endif /* _POWER_EVENT_H_ */
