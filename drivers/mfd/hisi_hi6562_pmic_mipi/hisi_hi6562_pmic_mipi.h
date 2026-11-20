/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * hisi_hi6562_pmic_mipi.h
 *
 * Head file for hisi_hi6562_pmic_mipi DRIVER
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _HISI_HI6562_PMIC_MIPI
#define _HISI_HI6562_PMIC_MIPI

#include <linux/device.h>
#include <linux/regmap.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

struct hisi_mipi_rffe_dev {
		struct device *dev;
		struct regmap *regmap;

		spinlock_t buffer_lock;
		int mipi_rffe_data;
		int mipi_rffe_clk;
		u32 opt_regs_num;
		u32 *opt_regs_addr;
		u32 *opt_regs_val;
		u16 usid_addr;

		struct delayed_work	mntn_work;
		struct delayed_work	mntn_test_work;
};
#endif