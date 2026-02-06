/*
 * vbus_monitor.h
 *
 * vbus monitor driver
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

#ifndef _VBUS_MONITOR_H_
#define _VBUS_MONITOR_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>

#define VBUS_MONITOR_VOTE_OBJECT   "vbus_monitor"

#define VBUS_ABSENT_INIT_TIME      4000 /* 4s */
#define VBUS_ABSENT_CHECK_TIME     2000 /* 2s */
#define VBUS_ABSENT_MAX_CNTS       5

enum vbus_sysfs_type {
	VBUS_SYSFS_BEGIN = 0,
	VBUS_SYSFS_ABSENT_STATE = VBUS_SYSFS_BEGIN,
	VBUS_SYSFS_CONNECT_STATE,
	VBUS_SYSFS_END,
};

enum vbus_absent_state {
	VBUS_STATE_PRESENT = 0,
	VBUS_STATE_ABSENT = 1,
};

enum vbus_connect_state {
	VBUS_DEFAULT_CONNECT_STATE = -1,
	VBUS_STATE_DISCONNECT = 0,
	VBUS_STATE_CONNECT = 1,
};

struct vbus_dev {
	struct device *dev;
	struct notifier_block nb;
	u32 absent_monitor_enabled;
	struct delayed_work absent_monitor_work;
	int absent_cnt;
	int absent_state;
	int connect_state;
};

#endif /* _VBUS_MONITOR_H_ */
