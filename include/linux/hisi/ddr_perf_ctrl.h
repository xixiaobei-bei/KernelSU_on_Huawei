/*
 * ddr_perf_ctrl.h
 *
 * ddr perf ctrl header file

 * Copyright (c) 2020, Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __DDR_PERF_CTRL_H
#define __DDR_PERF_CTRL_H

struct ddr_flux {
	unsigned long long rd_flux;
	unsigned long long wr_flux;
};

#ifdef CONFIG_HISI_DDR_PERF_CTRL
int perf_ctrl_get_ddr_flux(void __user *uarg);
#else
static inline int perf_ctrl_get_ddr_flux(void __user *uarg)
{
	return -EFAULT;
}
#endif

#endif
