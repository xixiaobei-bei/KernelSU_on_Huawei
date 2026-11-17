/*
 * acr_detect.h
 *
 * acr abnormal monitor driver
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

#ifndef _ACR_DETECT_H_
#define _ACR_DETECT_H_

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>

#define ACR_DET_NOT_RUNNING        0
#define ACR_DET_RUNNING            1
#define ACR_DET_NOT_SUPPORT        0
#define ACR_DET_RETRY_TIMES        50
#define ACR_DET_MIN_CURRENT        10 /* 10ma */
#define ACR_DET_DEFAULT_THLD       100
#define ACR_DET_DEFAULT_MIN_FMD    40
#define ACR_DET_DEFAULT_MAX_FMD    90
/* for bigdata report */
#define ACR_DET_DEFAULT_VERSION    1
#define ACR_DET_ITEM_ID            703023001
#define ACR_DET_FMD_FAIL           "fail"
#define ACR_DET_FMD_PASS           "pass"
#define ACR_DET_NA                 "NA"
#define ACR_DET_DEVICE_NAME        "battery"
#define ACR_DET_TEST_NAME          "BATT_ACR_VAL"

enum acr_det_sysfs_type {
	ACR_DET_SYSFS_BEGIN = 0,
	ACR_DET_SYSFS_RT_SUPPORT = ACR_DET_SYSFS_BEGIN,
	ACR_DET_SYSFS_RT_DETECT,
	ACR_DET_SYSFS_END,
};

enum acr_det_status_type {
	ACR_DET_STATUS_INIT = 0,
	ACR_DET_STATUS_FAIL,
	ACR_DET_STATUS_SUCC,
};

struct acr_det_dev {
	struct device *dev;
	struct delayed_work rt_work;
	int rt_run;
	u32 rt_support;
	int rt_status;
	int rt_threshold;
	u32 rt_fmd_min;
	u32 rt_fmd_max;
};

#endif /* _ACR_DETECT_H_ */
