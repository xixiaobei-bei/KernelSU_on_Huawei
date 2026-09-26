/*
 * wireless_protocol_sysfs.h
 *
 * sysfs of wireless protocol driver
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

#ifndef _WIRELESS_PROTOCOL_SYSFS_H_
#define _WIRELESS_PROTOCOL_SYSFS_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define WIRELESS_RD_BUF_SIZE      64
#define WIRELESS_WR_BUF_SIZE      64

enum wireless_sysfs_type {
	WIRELESS_SYSFS_BEGIN = 0,
	WIRELESS_SYSFS_TX_FWVER = WIRELESS_SYSFS_BEGIN,
	WIRELESS_SYSFS_TX_ID,
	WIRELESS_SYSFS_TX_TYPE,
	WIRELESS_SYSFS_TX_ADP_TYPE,
	WIRELESS_SYSFS_TX_BD_INFO,
	WIRELESS_SYSFS_END,
};

struct wireless_dev {
	struct device *dev;
};

#endif /* _WIRELESS_PROTOCOL_SYSFS_H_ */
