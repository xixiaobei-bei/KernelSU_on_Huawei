/*
 * power_log.h
 *
 * log for power module
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

#ifndef _POWER_LOG_H_
#define _POWER_LOG_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define POWER_LOG_INVAID_OP                 (-22)
#define POWER_LOG_MAX_SIZE                  4096
#define POWER_LOG_RESERVED_SIZE             16
#define POWER_LOG_RD_BUF_SIZE               32
#define POWER_LOG_WR_BUF_SIZE               32

enum power_log_device_id {
	POWER_LOG_DEVICE_ID_BEGIN = 0,
	POWER_LOG_DEVICE_ID_SERIES_BATT = POWER_LOG_DEVICE_ID_BEGIN,
	POWER_LOG_DEVICE_ID_SERIES_CAP,
	POWER_LOG_DEVICE_ID_BATT_INFO, /* for pmic */
	POWER_LOG_DEVICE_ID_MULTI_BTB, /* for multi btb temp */
	POWER_LOG_DEVICE_ID_BD99954, /* for bd99954 */
	POWER_LOG_DEVICE_ID_BQ2419X, /* for bq2419x */
	POWER_LOG_DEVICE_ID_BQ2429X, /* for bq2429x */
	POWER_LOG_DEVICE_ID_BQ2560X, /* for bq2560x */
	POWER_LOG_DEVICE_ID_BQ25882, /* for bq25882 */
	POWER_LOG_DEVICE_ID_BQ25892, /* for bq25892 */
	POWER_LOG_DEVICE_ID_ETA6937, /* for eta6937 */
	POWER_LOG_DEVICE_ID_HL7019, /* for hl7019 */
	POWER_LOG_DEVICE_ID_RT9466, /* for rt9466 */
	POWER_LOG_DEVICE_ID_RT9471, /* for rt9471 */
	POWER_LOG_DEVICE_ID_BQ25970, /* for bq25970 */
	POWER_LOG_DEVICE_ID_RT9759, /* for rt9759 */
	POWER_LOG_DEVICE_ID_BQ25970_AUX, /* for bq25970_aux */
	POWER_LOG_DEVICE_ID_RT9759_AUX, /* for rt9759_aux */
	POWER_LOG_DEVICE_ID_AW32280, /* for aw32280 */
	POWER_LOG_DEVICE_ID_AW32280_AUX, /* for aw32280_aux */
	POWER_LOG_DEVICE_ID_HI6522, /* for hi6522 */
	POWER_LOG_DEVICE_ID_HI6523, /* for hi6523 */
	POWER_LOG_DEVICE_ID_HI6526, /* for hi6526 */
	POWER_LOG_DEVICE_ID_HI6526_AUX, /* for hi6526_aux */
	POWER_LOG_DEVICE_ID_SM5450, /* for sm5450 */
	POWER_LOG_DEVICE_ID_SM5450_AUX, /* for sm5450_aux */
	POWER_LOG_DEVICE_ID_SGM41511H, /* for sgm41511h */
	POWER_LOG_DEVICE_ID_MT6360, /* for mt6360 */
	POWER_LOG_DEVICE_ID_RT9426, /* for rt9426 */
	POWER_LOG_DEVICE_ID_SC8545, /* for sc8545 */
	POWER_LOG_DEVICE_ID_SC8545_AUX, /* for sc8545_aux */
	POWER_LOG_DEVICE_ID_BQ27Z561, /* for bq27z561 */
	POWER_LOG_DEVICE_ID_BQ40Z50, /* for bq40z50 */
	POWER_LOG_DEVICE_ID_BQ25713, /* for bq25713 */
	POWER_LOG_DEVICE_ID_END,
};

enum power_log_sysfs_type {
	POWER_LOG_SYSFS_BEGIN = 0,
	POWER_LOG_SYSFS_DEV_ID = POWER_LOG_SYSFS_BEGIN,
	POWER_LOG_SYSFS_HEAD,
	POWER_LOG_SYSFS_HEAD_ALL,
	POWER_LOG_SYSFS_CONTENT,
	POWER_LOG_SYSFS_CONTENT_ALL,
	POWER_LOG_SYSFS_END,
};

enum power_log_type {
	POWER_LOG_TYPE_BEGIN = 0,
	POWER_LOG_DUMP_LOG_HEAD = POWER_LOG_TYPE_BEGIN,
	POWER_LOG_DUMP_LOG_CONTENT,
	POWER_LOG_TYPE_END,
};

struct power_log_ops {
	const char *dev_name;
	void *dev_data;
	int (*dump_log_head)(char *, int, void *);
	int (*dump_log_content)(char *, int, void *);
};

struct power_log_dev {
	struct device *dev;
	int dev_id;
	struct mutex log_lock;
	struct mutex devid_lock;
	char log_buf[POWER_LOG_MAX_SIZE];
	unsigned int total_ops;
	struct power_log_ops *ops[POWER_LOG_DEVICE_ID_END];
};

int power_log_ops_register(struct power_log_ops *ops);

#endif /* _POWER_LOG_H_ */
