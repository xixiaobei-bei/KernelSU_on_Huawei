/*
 * lga_check.h
 *
 * lga board abnormal monitor driver
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

#ifndef _LGA_CHECK_H_
#define _LGA_CHECK_H_

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
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>

#define LGA_CK_WORK_DELAY_TIME          30000 /* 30s */
#define LGA_CK_DMD_DELAY_TIME           3000 /* 3s */
#define LGA_CK_ADC_MAX_RETRYS           3
#define LGA_CK_MAX_DMD_REPORT_TIME      5
#define LGA_CK_FRACTURE_FOUND           (-1)
#define LGA_CK_FRACTURE_FREE            0
#define LGA_CK_PARA_LEVEL               8
#define LGA_CK_NAME_SIZE                16

enum lga_ck_mode {
	LGA_CK_MODE_BEGIN = 0,
	LGA_CK_MODE_ADC,
	LGA_CK_MODE_GPIO,
	LGA_CK_MODE_IRQ,
	LGA_CK_MODE_END,
};

enum lga_ck_sysfs_type {
	LGA_CK_SYSFS_TYPE_BEGIN = 0,
	LGA_CK_SYSFS_STATUS,
	LGA_CK_SYSFS_TYPE_END,
};

enum lga_ck_para_type {
	LGA_CK_TYPE = 0,
	LGA_CK_NAME,
	LGA_CK_ABNORMAL_THLD,
	LGA_CK_DMD_NO,
	LGA_CK_DMD_SWITCH,
	LGA_CK_PARA_TOTAL,
};

struct lga_ck_para_info {
	int type;
	char name[LGA_CK_NAME_SIZE];
	int threshold;
	int dmd_no;
	int dmd_switch;
	u32 adc_no;
	int adc_vol;
	int gpio_no;
	int gpio_val;
	int irq_gpio_no;
	int irq_int;
	int irq_val;
	int status;
};

struct lga_ck_para_data {
	int total_type;
	struct lga_ck_para_info para[LGA_CK_PARA_LEVEL];
};

struct lga_ck_dev {
	struct device *dev;
	struct notifier_block nb;
	struct delayed_work detect_work;
	struct lga_ck_para_data data;
	int abnormal_time;
};

#endif /* _LGA_CHECK_H_ */
