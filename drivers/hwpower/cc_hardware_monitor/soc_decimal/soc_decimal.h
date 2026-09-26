/*
 * soc_decimal.h
 *
 * calculate soc with decimal driver
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

#ifndef _SOC_DECIMAL_H_
#define _SOC_DECIMAL_H_

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
#include <linux/delay.h>

#define SOC_DECIMAL_RD_BUF_SIZE        64
#define SOC_DECIMAL_WR_BUF_SIZE        32
#define SOC_DECIMAL_PARA_LEVEL         8
#define SOC_DECIMAL_DEFAULT_LEVEL      (-1)
#define SOC_DECIMAL_DEFAULT_BASE       100
#define SOC_DECIMAL_DEFAULT_SAMPLES    85
#define SOC_DECIMAL_DEFAULT_INTERVAL   140 /* 0.14 seconds */
#define SOC_DECIMAL_WINDOW_LEN         20
#define SOC_DECIMAL_WAIT_TIME          10

enum soc_decimal_sysfs_type {
	SOC_DECIMAL_SYSFS_BEGIN = 0,
	SOC_DECIMAL_SYSFS_START = SOC_DECIMAL_SYSFS_BEGIN,
	SOC_DECIMAL_SYSFS_SOC,
	SOC_DECIMAL_SYSFS_LEVEL,
	SOC_DECIMAL_SYSFS_PARA,
	SOC_DECIMAL_SYSFS_END,
};

enum soc_decimal_op_user {
	SOC_DECIMAL_OP_USER_BEGIN = 0,
	SOC_DECIMAL_OP_USER_SYSTEM_UI = SOC_DECIMAL_OP_USER_BEGIN,
	SOC_DECIMAL_OP_USER_END,
};

enum soc_decimal_para_type {
	SOC_DECIMAL_TYPE_DC = 0,
	SOC_DECIMAL_TYPE_WL_DC,
};

enum soc_decimal_para_item {
	SOC_DECIMAL_ITEM_TYPE = 0,
	SOC_DECIMAL_ITEM_MIN_PWR,
	SOC_DECIMAL_ITEM_MAX_PWR,
	SOC_DECIMAL_ITEM_BASE,
	SOC_DECIMAL_ITEM_SAMPLES,
	SOC_DECIMAL_ITEM_INTERVAL,
	SOC_DECIMAL_ITEM_TOTAL,
};

struct soc_decimal_para_data {
	u32 type;
	u32 min_pwr;
	u32 max_pwr;
	u32 base;
	u32 samples;
	u32 interval;
};

struct soc_decimal_fifo_data {
	u32 filter[SOC_DECIMAL_WINDOW_LEN];
	u32 sum; /* sum of filter data */
	u32 index; /* index of filter data */
	u32 curr_samples;
	u32 init_soc;
};

struct soc_decimal_calc_data {
	int base;
	int samples;
	int curr_samples;
	int init_soc;
	int curr_soc;
	int rep_soc;
	int round_soc;
	int remain_soc;
};

struct soc_decimal_info {
	bool start;
	u32 soc;
	int level;
	struct soc_decimal_fifo_data fifo;
	struct soc_decimal_para_data para;
};

struct soc_decimal_dev {
	struct device *dev;
	struct notifier_block nb;
	struct hrtimer timer;
	int para_level;
	struct soc_decimal_para_data para[SOC_DECIMAL_PARA_LEVEL];
	u32 ui_offset;
	struct soc_decimal_info info;
};

#endif /* _SOC_DECIMAL_H_ */
