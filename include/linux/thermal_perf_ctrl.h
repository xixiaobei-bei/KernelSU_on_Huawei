/*
 * thermal_perf_ctrl.h
 *
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

#ifndef __THERMAL_PERF_CTRL_H
#define __THERMAL_PERF_CTRL_H

enum thermal_cdev_type {
	CDEV_GPU = 0,
	CDEV_CPU_CLUSTER0,
	CDEV_CPU_CLUSTER1,
	CDEV_CPU_CLUSTER2,
	THERMAL_CDEV_MAX,
};

enum thermal_zone_type {
	SOC_THERMAL_ZONE = 0,
	BOARD_THERMAL_ZONE,
	THERMAL_ZONE_MAX,
};

struct thermal_cdev_power {
	int thermal_zone_type;
	unsigned int cdev_power[THERMAL_CDEV_MAX];
};

struct ipa_stat {
	unsigned int cluster0;
	unsigned int cluster1;
	unsigned int cluster2;
	unsigned int gpu;
	int soc_temp;
	int board_temp;
};

#ifdef CONFIG_PERF_CTRL
int perf_ctrl_get_ipa_stat(void __user *uarg);
int perf_ctrl_get_thermal_cdev_power(void __user *uarg);
#else
static inline int perf_ctrl_get_ipa_stat(void __user *uarg)
{
	return -EFAULT;
}

static inline int perf_ctrl_get_thermal_cdev_power(void __user *uarg)
{
	return -EFAULT;
}
#endif

#endif
