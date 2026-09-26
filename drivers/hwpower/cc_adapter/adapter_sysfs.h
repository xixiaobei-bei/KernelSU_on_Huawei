/*
 * adapter_sysfs.h
 *
 * sysfs of adapter driver
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

#ifndef _ADAPTER_SYSFS_H_
#define _ADAPTER_SYSFS_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define ADAPTER_RD_BUF_SIZE  32
#define ADAPTER_WR_BUF_SIZE  32

enum adapter_sysfs_type {
	ADAPTER_SYSFS_BEGIN = 0,
	ADAPTER_SYSFS_SUPPORT_MODE = ADAPTER_SYSFS_BEGIN,
	ADAPTER_SYSFS_CHIP_ID,
	ADAPTER_SYSFS_VENDOR_ID,
	ADAPTER_SYSFS_MODULE_ID,
	ADAPTER_SYSFS_SERIAL_NO,
	ADAPTER_SYSFS_HWVER,
	ADAPTER_SYSFS_FWVER,
	ADAPTER_SYSFS_MIN_VOLT,
	ADAPTER_SYSFS_MAX_VOLT,
	ADAPTER_SYSFS_MIN_CUR,
	ADAPTER_SYSFS_MAX_CUR,
	ADAPTER_SYSFS_ADP_TYPE,
	ADAPTER_SYSFS_TEST_RESULT,
	ADAPTER_SYSFS_END,
};

struct adapter_sysfs_dev {
	struct device *dev;
};

#endif /* _ADAPTER_SYSFS_H_ */
