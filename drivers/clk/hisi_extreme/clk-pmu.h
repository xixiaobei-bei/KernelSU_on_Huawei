/*
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
 */
#ifndef __CLK_PMU_H
#define __CLK_PMU_H
#include <linux/clk.h>
#include <securec.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hwspinlock.h>
#include <linux/mfd/hisi_pmic.h>
#include <soc_sctrl_interface.h>
#include "clk.h"

#define CLK_HWLOCK_TIMEOUT		1000
#define ABB_SCBAKDATA(BASE)		SOC_SCTRL_SCBAKDATA12_ADDR(BASE);
#define AP_ABB_EN			0
#define LPM3_ABB_EN			1

#define MAX_CMP_LEN			50 /* max compatible len */

struct hi3xxx_clkpmu {
	struct clk_hw	hw;
	void __iomem	*sctrl;
	u32	ebits;
	spinlock_t *lock;
	u32	always_on;
	u32	sync_time;
	struct hwspinlock *clk_hwlock;
	u32	pmu_clk_enable;
	unsigned int	(*clk_pmic_read)(int clk_enable);
	void	(*clk_pmic_write)(int clk_enable, int val);
	u32	flags;
};
#endif
