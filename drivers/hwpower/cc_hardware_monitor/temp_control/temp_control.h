/*
 * temp_control.h
 *
 * huawei temperature control driver
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

#ifndef _TEMP_CONTROL_H_
#define _TEMP_CONTROL_H_

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
#include <chipset_common/hwpower/common_module/power_algorithm.h>

#define DELAY_TIME_FOR_SLOW_WORK         30000 /* 30s */
#define DELAY_TIME_FOR_FAST_WORK         10000 /* 10s */

#define TEMP_CTRL_PARA_LEVEL             8

enum temp_control_para_type {
	TEMP_CTRL_TEMP_LTH = 0,
	TEMP_CTRL_TEMP_HTH,
	TEMP_CTRL_TEMP_BACK,
	TEMP_CTRL_IOUT_MAX,
	TEMP_CTRL_DMD_NO,
	TEMP_CTRL_DMD_MAX_CNT,
	TEMP_CTRL_PARA_TOTAL,
};

struct temp_control_para_info {
	int iout_max;
	int dmd_no;
	int dmd_max_cnt;
	int dmd_count;
};

struct temp_control_dev {
	struct device *dev;
	struct notifier_block nb;
	struct delayed_work monitor_work;
	struct hysteresis_para usb_port_hys[TEMP_CTRL_PARA_LEVEL];
	struct temp_control_para_info usb_port_para[TEMP_CTRL_PARA_LEVEL];
	int usb_port_para_size;
	int usb_port_para_index;
};

#endif /* _TEMP_CONTROL_H_ */
