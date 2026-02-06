/*
 * btb_check.h
 *
 * btb abnormal monitor driver
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

#ifndef _BTB_CHECK_H_
#define _BTB_CHECK_H_

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

#define MAIN_BAT_BTB_ERR           BIT(0)
#define AUX_BAT_BTB_ERR            BIT(1)
#define BTB_BAT_DIFF_ERR           BIT(2)

#define BTB_CK_WORK_DELAY_TIME     30

enum btb_ck_type {
	BTB_CK_TYPE_BEGIN = 0,
	BTB_VOLT_CHECK = BTB_CK_TYPE_BEGIN,
	BTB_TEMP_CHECK,
	BTB_CK_TYPE_END,
};

enum btb_ck_sysfs_type {
	BTB_CK_SYSFS_TYPE_BEGIN = 0,
	BTB_CK_SYSFS_VOLT_RESULT = BTB_CK_SYSFS_TYPE_BEGIN,
	BTB_CK_SYSFS_TEMP_RESULT,
	BTB_CK_SYSFS_TYPE_END,
};

enum btb_ck_para_type {
	BTB_CK_ENABLE = 0,
	BTB_CK_IS_MULTI_BTB,
	BTB_CK_MIN_TH,
	BTB_CK_MAX_TH,
	BTB_CK_DIFF_TH,
	BTB_CK_TIMES,
	BTB_CK_DMD_NO,
	BTB_CK_PARA_TOTAL,
};

struct btb_ck_para {
	int enable;
	int is_multi_btb;
	int min_th;
	int max_th;
	int diff_th;
	int times;
	int dmd_no;
};

struct btb_ck_ops {
	int (*get_bat_btb_temp)(int *temp);
};

struct btb_ck_dev {
	struct device *dev;
	struct notifier_block nb;
	struct delayed_work volt_ck_work;
	struct delayed_work temp_ck_work;
	struct btb_ck_para volt_ck_para;
	struct btb_ck_para temp_ck_para;
	unsigned int volt_ck_result;
	unsigned int temp_ck_result;
};

#ifdef CONFIG_HUAWEI_BTB_CHECK
unsigned int btb_ck_get_result(int type);
void btb_ck_get_result_now(int type, unsigned int *result);
#else
static inline unsigned int btb_ck_get_result(int type)
{
	return 0;
}

static inline void btb_ck_get_result_now(int type, unsigned int *result)
{
}
#endif /* CONFIG_HUAWEI_BTB_CHECK */

#endif /* _BTB_CHECK_H_ */
