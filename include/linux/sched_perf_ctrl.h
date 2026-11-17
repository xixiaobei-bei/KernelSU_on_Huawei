/*
 * sched_perf_ctrl.h
 *
 * sched perf ctrl header file

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

#ifndef __SCHED_PERF_CTRL_H
#define __SCHED_PERF_CTRL_H

struct sched_stat {
	pid_t pid;
	unsigned long long sum_exec_runtime;
	unsigned long long run_delay;
	unsigned long pcount;
};

struct rtg_group_task {
	pid_t pid;
	unsigned int grp_id;
	bool pmu_sample_enabled;
};

struct rtg_cpus {
	unsigned int grp_id;
	int cluster_id;
};

struct rtg_freq {
	unsigned int grp_id;
	unsigned int freq;
};

struct rtg_interval {
	unsigned int grp_id;
	unsigned int interval;
};

/* rtg:load_mode */
struct rtg_load_mode {
	unsigned int grp_id;
	unsigned int freq_enabled;
	unsigned int util_enabled;
};

/* rtg: ed_params */
struct rtg_ed_params {
	unsigned int grp_id;
	unsigned int enabled;
	unsigned int running_ns;
	unsigned int waiting_ns;
	unsigned int nt_running_ns;
};

struct task_config {
	pid_t pid;
	unsigned int value;
};

#ifdef CONFIG_SCHED_INFO
int perf_ctrl_get_sched_stat(void __user *uarg);
#else
static inline int perf_ctrl_get_sched_stat(void __user *uarg)
{
	return -EFAULT;
}
#endif

static inline int perf_ctrl_set_task_util(void __user *uarg)
{
	return 0;
}
int perf_ctrl_get_related_tid(void __user *uarg);

#ifdef CONFIG_SCHED_RTG
int perf_ctrl_set_task_rtg(void __user *uarg);
int perf_ctrl_set_rtg_cpus(void __user *uarg);
int perf_ctrl_set_rtg_freq(void __user *uarg);
int perf_ctrl_set_rtg_freq_update_interval(void __user *uarg);
int perf_ctrl_set_rtg_util_invalid_interval(void __user *uarg);
int perf_ctrl_set_rtg_load_mode(void __user *uarg);
int perf_ctrl_set_rtg_ed_params(void __user *uarg);
int perf_ctrl_set_task_rtg_min_freq(void __user *uarg);
#else
static inline int perf_ctrl_set_task_rtg(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_set_rtg_cpus(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_set_rtg_freq(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_set_rtg_freq_update_interval(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_set_rtg_util_invalid_interval(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_set_rtg_load_mode(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_set_rtg_ed_params(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_set_task_rtg_min_freq(void __user *uarg)
{
	return -EFAULT;
}
#endif

#ifdef CONFIG_HUAWEI_SCHED_VIP
int perf_ctrl_set_vip_prio(void __user *uarg);
#else
static inline int perf_ctrl_set_vip_prio(void __user *uarg)
{
	return -EFAULT;
}
#endif


#ifdef CONFIG_HISI_EAS_SCHED
int perf_ctrl_set_favor_small_cap(void __user *uarg);
#else
static inline int perf_ctrl_set_favor_small_cap(void __user *uarg)
{
	return -EFAULT;
}
#endif

#ifdef CONFIG_SCHED_HISI_UTIL_CLAMP
int perf_ctrl_set_task_min_util(void __user *uarg);
int perf_ctrl_set_task_max_util(void __user *uarg);
#else
static inline int perf_ctrl_set_task_min_util(void __user *uarg)
{
	return -EFAULT;
}

static inline int perf_ctrl_set_task_max_util(void __user *uarg)
{
	return -EFAULT;
}
#endif

#ifdef CONFIG_SCHED_STAT_YIELD
int perf_ctrl_get_task_yield_time(void __user *uarg);
#else
static inline int perf_ctrl_get_task_yield_time(void __user *uarg)
{
	return -EFAULT;
}
#endif

#ifdef CONFIG_FRAME_RTG
int perf_ctrl_set_frame_rate(void __user *uarg);
int perf_ctrl_set_frame_margin(void __user *uarg);
int perf_ctrl_set_frame_status(void __user *uarg);
#else
static inline int perf_ctrl_set_frame_rate(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_set_frame_margin(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_set_frame_status(void __user *uarg)
{
	return -EFAULT;
}
#endif

#endif

