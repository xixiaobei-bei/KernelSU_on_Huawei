/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_ramp.h
 *
 * Device driver for PMU RAMP interrupts
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _PMIC_RAMP_H_
#define _PMIC_RAMP_H_

#include <linux/types.h>

#define PMU0_RAMP_COMP_STR "hisilicon-pmic-ramp"
#define PMU1_RAMP_COMP_STR "hisilicon-sub-pmic-ramp"
#define RAMP_NAME_MAX	20

struct ramp_irq_info {
	int irq;
	char *irq_name;
	u32 reset;
};

struct pmic_ramp {
	struct device *dev;
	int irq_num;
	struct ramp_irq_info *ramp_irqs;
	int event_reg_num;
	u32 *event_regs;
};

#endif /* _PMIC_RAMP_H_ */
