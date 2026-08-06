/*
 * cpufreq_perf_ctrl.h
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
#ifndef __CPUFREQ_PERF_CTRL_H
#define __CPUFREQ_PERF_CTRL_H

struct cpu_busy_time {
	int cpu_count;
	u64 time_adj_freq[NR_CPUS];
};

#if defined(CONFIG_PERF_CTRL) && defined(CONFIG_HISI_FREQ_STATS_COUNTING_IDLE)
int perf_ctrl_get_cpu_busy_time(void __user *uarg);
#else
static inline int perf_ctrl_get_cpu_busy_time(void __user *uarg)
{
	return -ENODEV;
}
#endif

#endif
