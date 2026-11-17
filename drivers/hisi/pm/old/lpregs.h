/*
 * lpregs.h
 *
 * io debugging, and could checkout unexpected
 * status of all io/clk/pmu before system into SUSPEND.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
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

#ifndef __LPREGS_H
#define __LPREGS_H

#include <linux/seq_file.h>

#define PERI_EN_CLOSE		BIT(0)
#define PERI_EN_ECO		BIT(1)
#define LPM3_DIS		BIT(2)
#define AP_DIS		BIT(3)
#define CLK_EN_CLOSE		BIT(4)
#define NULL_OWNER		BIT(5)
#define SOFT_ECO		BIT(6)

struct pmuregs {
	unsigned int ucoffset;
	unsigned int cval;
	unsigned int old_val;
	unsigned int cmask;
};

struct pmu_idx {
	const char *name;
	const char *module;
	unsigned int status;
	const char *status_name;
};

void pm_status_show(struct seq_file *s);
void pm_nvic_pending_dump(void);
void io_status_show(struct seq_file *s);
void pmu_status_show(struct seq_file *s);
void clk_status_show(struct seq_file *s);
void debuguart_reinit(void);
int get_console_index(void);
#endif
