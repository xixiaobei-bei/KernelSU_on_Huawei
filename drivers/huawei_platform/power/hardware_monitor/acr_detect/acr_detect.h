/*
 * acr_detect.h
 *
 * acr abnormal monitor driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#define ACR_FALSE                 0
#define ACR_TRUE                  1

#define ACR_RT_NOT_SUPPORT        0
#define ACR_RT_RETRY_TIMES        50
#define ACR_RT_CURRENT_MIN        10 /* 10ma */
#define ACR_RT_THRESHOLD_DEFAULT  100
#define ACR_RT_FMD_MIN_DEFAULT    40
#define ACR_RT_FMD_MAX_DEFAULT    90

#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
#define ACR_RT_DEFAULT_VERSION    1
#define ACR_RT_ITEM_ID            703023001
#define ACR_RT_FMD_FAIL           "fail"
#define ACR_RT_FMD_PASS           "pass"
#define ACR_RT_NA                 "NA"
#define ACR_RT_DEVICE_NAME        "battery"
#define ACR_RT_TEST_NAME          "BATT_ACR_VAL"
#endif /* CONFIG_HUAWEI_DATA_ACQUISITION */

struct acr_det_device_info {
	struct device *dev;
	struct acr_info acr_data;
	struct delayed_work rt_work;
#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
	struct message acr_msg;
#endif /* CONFIG_HUAWEI_DATA_ACQUISITION */
	int rt_support;
	int rt_status;
	int rt_threshold;
	int rt_fmd_min;
	int rt_fmd_max;
};

enum acr_det_sysfs_type {
	ACR_DET_SYSFS_RT_SUPPORT = 0,
	ACR_DET_SYSFS_RT_DETECT,
};

enum acr_det_rt_status_type {
	ACR_DET_RT_STATUS_INIT = 0,
	ACR_DET_RT_STATUS_FAIL,
	ACR_DET_RT_STATUS_SUCC,
};

#endif /* _ACR_DETECT_H_ */
