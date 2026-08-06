/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ship_mode.h
 *
 * ship mode control driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _SHIP_MODE_H_
#define _SHIP_MODE_H_

#include <linux/errno.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define SHIP_MODE_RW_BUF_SIZE                16
#define SHIP_MODE_DEFAULT_ENTRY_TIME_USER    15 /* s */
#define SHIP_MODE_DEFAULT_ENTRY_TIME_FAC     5 /* s */
#define SHIP_MODE_DEFAULT_DELAY_TIME         3 /* ms */

enum ship_mode_sysfs_type {
	SHIP_MODE_SYSFS_BEGIN = 0,
	SHIP_MODE_SYSFS_DELAY_TIME = SHIP_MODE_SYSFS_BEGIN,
	SHIP_MODE_SYSFS_ENTRY_TIME,
	SHIP_MODE_SYSFS_WORK_MODE,
	SHIP_MODE_SYSFS_END,
};

enum ship_mode_op_user {
	SHIP_MODE_OP_USER_BEGIN = 0,
	SHIP_MODE_OP_USER_SHELL = SHIP_MODE_OP_USER_BEGIN, /* for shell */
	SHIP_MODE_OP_USER_ATCMD, /* for atcmd or diag daemon */
	SHIP_MODE_OP_USER_HIDL, /* for hidl interface */
	SHIP_MODE_OP_USER_END,
};

enum ship_mode_work_mode {
	SHIP_MODE_NOT_IN_SHIP = 0,
	SHIP_MODE_IN_SHIP,
	SHIP_MODE_IN_SHUTDOWN_SHIP,
};

struct ship_mode_para {
	unsigned int delay_time;
	unsigned int entry_time;
	unsigned int work_mode;
};

struct ship_mode_ops {
	const char *ops_name;
	void *dev_data;
	void (*set_entry_time)(unsigned int time, void *dev_data);
	void (*set_work_mode)(unsigned int mode, void *dev_data);
};

struct ship_mode_dev {
	struct device *dev;
	struct ship_mode_ops *ops;
	struct ship_mode_para para;
};

#ifdef CONFIG_HUAWEI_SHIP_MODE
int ship_mode_ops_register(struct ship_mode_ops *ops);
int ship_mode_entry(const struct ship_mode_para *para);
#else
static inline int ship_mode_ops_register(struct ship_mode_ops *ops)
{
	return -EPERM;
}

static inline int ship_mode_entry(const struct ship_mode_para *para)
{
	return -EINVAL;
}
#endif /* CONFIG_HUAWEI_SHIP_MODE */

#endif /* _SHIP_MODE_H_ */
