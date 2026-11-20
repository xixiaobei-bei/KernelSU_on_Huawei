/*
 * perf_ctrl.h
 *
 * header file for perf_ctrl
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd.
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

#ifndef __PERF_CTRL_H__
#define __PERF_CTRL_H__

#include <linux/hisi/gpu_hook.h>
#include <linux/thermal_perf_ctrl.h>
#include <linux/hisi/ddr_perf_ctrl.h>
#include <linux/drg.h>
#include <linux/render_rt.h>
#include <linux/sched_perf_ctrl.h>
#include <linux/hisi/hisi_lb.h>
#include <linux/cpufreq_perf_ctrl.h>
#include <linux/hisi/l3cache_partition_ctrl.h>

#define PERF_CTRL_MAGIC 'x'

enum {
	GET_SCHED_STAT = 1,
	SET_TASK_UTIL,
	GET_IPA_STAT,
	GET_DDR_FLUX,
	GET_RELATED_TID,
	GET_DEV_FREQ,
	GET_THERMAL_CDEV_POWER,
	SET_FRAME_RATE,
	SET_FRAME_MARGIN,
	SET_FRAME_STATUS,
	SET_TASK_RTG,
	SET_RTG_CPUS,
	SET_RTG_FREQ,
	SET_RTG_FREQ_UPDATE_INTERVAL,
	SET_RTG_UTIL_INVALID_INTERVAL,
	GET_GPU_FENCE,
	/* render related threads */
	INIT_RENDER_PID,
	GET_RENDER_RT,
	STOP_RENDER_RT,
	GET_RENDER_HT,
	DESTROY_RENDER_RT,
	SET_RTG_LOAD_MODE,
	SET_RTG_ED_PARAMS,
	GET_DEV_CAP,
	SET_VIP_PRIO,
	SET_FAVOR_SMALL_CAP,
	SET_TASK_RTG_MIN_FREQ,
	SET_TASK_MIN_UTIL,
	GET_GPU_BUFFER_SIZE,
	SET_LB_POLICY,
	RESET_LB_POLICY,
	GET_LB_LITE_INFO,
	ENABLE_GPU_LB,
	GET_CPU_BUSY_TIME,
	SET_TASK_MAX_UTIL,
	GET_TASK_YIELD_TIME,
	SET_TASK_L3C_PART,

	/* ADD BELOW */

	PERF_CTRL_MAX_NR,
};

/* use cap to track the difference of the ioctl cmd */
enum {
	CAP_AI_SCHED_COMM_CMD = 0,
	CAP_RTG_CMD,
	CAP_RENDER_RT_CMD,
};

#define PERF_CTRL_GET_SCHED_STAT \
	_IOR(PERF_CTRL_MAGIC, GET_SCHED_STAT, struct sched_stat)
#define PERF_CTRL_GET_IPA_STAT \
	_IOR(PERF_CTRL_MAGIC, GET_IPA_STAT, struct ipa_stat)
#define PERF_CTRL_GET_DDR_FLUX \
	_IOR(PERF_CTRL_MAGIC, GET_DDR_FLUX, struct ddr_flux)
#define PERF_CTRL_GET_RELATED_TID \
	_IOWR(PERF_CTRL_MAGIC, GET_RELATED_TID, struct related_tid_info)
#define PERF_CTRL_DRG_GET_DEV_FREQ \
	_IOWR(PERF_CTRL_MAGIC, GET_DEV_FREQ, struct drg_dev_freq)
#define PERF_CTRL_GET_THERMAL_CDEV_POWER \
	_IOWR(PERF_CTRL_MAGIC, GET_THERMAL_CDEV_POWER, struct thermal_cdev_power)
#define PERF_CTRL_GET_GPU_FENCE \
	_IOWR(PERF_CTRL_MAGIC, GET_GPU_FENCE, struct kbase_fence_info)
#define PERF_CTRL_GET_GPU_BUFFER_SIZE \
	_IOR(PERF_CTRL_MAGIC, GET_GPU_BUFFER_SIZE, unsigned int)
#define PERF_CTRL_ENABLE_GPU_LB \
	_IOW(PERF_CTRL_MAGIC, ENABLE_GPU_LB, unsigned int)

#define PERF_CTRL_SET_FRAME_RATE \
	_IOWR(PERF_CTRL_MAGIC, SET_FRAME_RATE, int*)
#define PERF_CTRL_SET_FRAME_MARGIN \
	_IOWR(PERF_CTRL_MAGIC, SET_FRAME_MARGIN, int*)
#define PERF_CTRL_SET_FRAME_STATUS \
	_IOWR(PERF_CTRL_MAGIC, SET_FRAME_STATUS, int*)
#define PERF_CTRL_SET_TASK_RTG \
	_IOWR(PERF_CTRL_MAGIC, SET_TASK_RTG, struct rtg_group_task)
#define PERF_CTRL_SET_RTG_CPUS \
	_IOWR(PERF_CTRL_MAGIC, SET_RTG_CPUS, struct rtg_cpus)
#define PERF_CTRL_SET_RTG_FREQ \
	_IOWR(PERF_CTRL_MAGIC, SET_RTG_FREQ, struct rtg_freq)
#define PERF_CTRL_SET_RTG_FREQ_UPDATE_INTERVAL \
	_IOWR(PERF_CTRL_MAGIC, SET_RTG_FREQ_UPDATE_INTERVAL, struct rtg_interval)
#define PERF_CTRL_SET_RTG_UTIL_INVALID_INTERVAL \
	_IOWR(PERF_CTRL_MAGIC, SET_RTG_UTIL_INVALID_INTERVAL, struct rtg_interval)
#define PERF_CTRL_SET_RTG_LOAD_MODE \
	_IOW(PERF_CTRL_MAGIC, SET_RTG_LOAD_MODE, struct rtg_load_mode)
#define PERF_CTRL_SET_RTG_ED_PARAMS \
	_IOW(PERF_CTRL_MAGIC, SET_RTG_ED_PARAMS, struct rtg_ed_params)

#define PERF_CTRL_INIT_RENDER_PID \
	_IOW(PERF_CTRL_MAGIC, INIT_RENDER_PID, struct render_init_paras)
#define PERF_CTRL_GET_RENDER_RT \
	_IOWR(PERF_CTRL_MAGIC, GET_RENDER_RT, struct render_rt)
#define PERF_CTRL_STOP_RENDER_RT \
	_IOW(PERF_CTRL_MAGIC, STOP_RENDER_RT, struct render_stop)
#define PERF_CTRL_GET_RENDER_HT \
	_IOWR(PERF_CTRL_MAGIC, GET_RENDER_HT, struct render_ht)
#define PERF_CTRL_DESTROY_RENDER_RT \
	_IOW(PERF_CTRL_MAGIC, DESTROY_RENDER_RT, pid_t)
#define PERF_CTRL_GET_DEV_CAP \
	_IOR(PERF_CTRL_MAGIC, GET_DEV_CAP, unsigned long)

#define PERF_CTRL_SET_VIP_PRIO \
	_IOW(PERF_CTRL_MAGIC, SET_VIP_PRIO, struct task_config)
#define PERF_CTRL_SET_FAVOR_SMALL_CAP \
	_IOW(PERF_CTRL_MAGIC, SET_FAVOR_SMALL_CAP, struct task_config)
#define PERF_CTRL_SET_TASK_RTG_MIN_FREQ \
	_IOW(PERF_CTRL_MAGIC, SET_TASK_RTG_MIN_FREQ, struct task_config)
#define PERF_CTRL_SET_TASK_MIN_UTIL \
	_IOW(PERF_CTRL_MAGIC, SET_TASK_MIN_UTIL, struct task_config)
#define PERF_CTRL_SET_TASK_MAX_UTIL \
	_IOW(PERF_CTRL_MAGIC, SET_TASK_MAX_UTIL, struct task_config)
#define PERF_CTRL_GET_TASK_YIELD_TIME \
	_IOWR(PERF_CTRL_MAGIC, GET_TASK_YIELD_TIME, struct task_config)

#define PERF_CTRL_SET_LB_POLICY \
	_IOW(PERF_CTRL_MAGIC, SET_LB_POLICY, struct lb_policy_config)
#define PERF_CTRL_RESET_LB_POLICY \
	_IOW(PERF_CTRL_MAGIC, RESET_LB_POLICY, struct lb_policy_config)
#define PERF_CTRL_GET_LB_LITE_INFO \
	_IOW(PERF_CTRL_MAGIC, GET_LB_LITE_INFO, struct lb_policy_config)
#define PERF_CTRL_GET_CPU_BUSY_TIME \
	_IOW(PERF_CTRL_MAGIC, GET_CPU_BUSY_TIME, struct cpu_busy_time)


#define PERF_CTRL_SET_TASK_L3C_PART \
	_IOW(PERF_CTRL_MAGIC, SET_TASK_L3C_PART, struct task_config)

#endif /* __PERF_CTRL_H__ */
