/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
 * rtg ioctl entry header
 */

#ifndef IAWARE_RTG_H
#define IAWARE_RTG_H

#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#ifdef CONFIG_HW_RTG_SCHED
#include <linux/render_rt.h>
#endif

#define RTG_SCHED_IPC_MAGIC 0XAB

#define CMD_ID_SET_CONFIG \
	_IOWR(RTG_SCHED_IPC_MAGIC, SET_CONFIG, struct rtg_str_data)
#define CMD_ID_SET_RTG_THREAD \
	_IOWR(RTG_SCHED_IPC_MAGIC, SET_RTG_THREAD, struct rtg_str_data)
#define CMD_ID_SET_ENABLE \
	_IOWR(RTG_SCHED_IPC_MAGIC, SET_ENABLE, struct rtg_enable_data)
#define CMD_ID_SET_RTG_ATTR \
	_IOWR(RTG_SCHED_IPC_MAGIC, SET_RTG_ATTR, struct rtg_str_data)
#define CMD_ID_SET_RTG_CFS_THREAD \
	_IOWR(RTG_SCHED_IPC_MAGIC, SET_RTG_CFS_THREAD, struct rtg_str_data)
#define CMD_ID_GET_QOS_CLASS \
	_IOR(RTG_SCHED_IPC_MAGIC, GET_QOS_CLASS, struct rtg_qos_data)
#define CMD_ID_BEGIN_FRAME_FREQ \
	_IOWR(RTG_SCHED_IPC_MAGIC, BEGIN_FRAME_FREQ, struct proc_state_data)
#define CMD_ID_END_FRAME_FREQ \
	_IOWR(RTG_SCHED_IPC_MAGIC, END_FRAME_FREQ, struct proc_state_data)
#define CMD_ID_BEGIN_ACTIVITY_FREQ \
	_IOWR(RTG_SCHED_IPC_MAGIC, BEGIN_ACTIVITY_FREQ, struct proc_state_data)
#define CMD_ID_END_ACTIVITY_FREQ \
	_IOWR(RTG_SCHED_IPC_MAGIC, END_ACTIVITY_FREQ, struct proc_state_data)
#define CMD_ID_END_FREQ \
	_IOWR(RTG_SCHED_IPC_MAGIC, END_FREQ, struct proc_state_data)
#define CMD_ID_ENABLE_RTG_BOOST \
	_IOWR(RTG_SCHED_IPC_MAGIC, ENABLE_RTG_BOOST, struct rtg_boost_data)

#define CMD_ID_SET_MIN_UTIL \
	_IOWR(RTG_SCHED_IPC_MAGIC, SET_MIN_UTIL, struct proc_state_data)
#define CMD_ID_SET_MARGIN \
	_IOWR(RTG_SCHED_IPC_MAGIC, SET_MARGIN, struct proc_state_data)
#define CMD_ID_SET_MIM_UTIL_AND_MARGIN \
	_IOWR(RTG_SCHED_IPC_MAGIC, SET_MIM_UTIL_AND_MARGIN, \
		struct min_util_margin_data)

#define CMD_ID_SET_RME_MARGIN \
	_IOWR(RTG_SCHED_IPC_MAGIC, SET_RME_MARGIN, struct rme_fps_data)
#define CMD_ID_GET_RME_MARGIN \
	_IOWR(RTG_SCHED_IPC_MAGIC, GET_RME_MARGIN, struct rme_fps_data)

#ifdef CONFIG_HW_RTG_SCHED
#define RTG_CMD_RT_CTRL_INIT_RENDER_PID \
	_IOW(RTG_SCHED_IPC_MAGIC, RT_CTRL_INIT_RENDER_PID, struct render_init_paras)
#define RTG_CMD_RT_CTRL_GET_RENDER_RT \
	_IOWR(RTG_SCHED_IPC_MAGIC, RT_CTRL_GET_RENDER_RT, struct render_rt)
#define RTG_CMD_RT_CTRL_STOP_RENDER_RT \
	_IOW(RTG_SCHED_IPC_MAGIC, RT_CTRL_STOP_RENDER_RT, struct render_stop)
#define RTG_CMD_RT_CTRL_GET_RENDER_HT \
	_IOWR(RTG_SCHED_IPC_MAGIC, RT_CTRL_GET_RENDER_HT, struct render_ht)
#define RTG_CMD_RT_CTRL_DESTROY_RENDER_RT \
	_IOW(RTG_SCHED_IPC_MAGIC, RT_CTRL_DESTROY_RENDER_RT, pid_t)
#define RTG_CMD_RT_CTRL_GET_DEV_CAP \
	_IOR(RTG_SCHED_IPC_MAGIC, RT_CTRL_GET_DEV_CAP, unsigned long)
#endif

enum rtg_sched_cmdid {
	SET_CONFIG = 1,
	SET_RTG_THREAD,
	SET_ENABLE,
	SET_RTG_ATTR,
	SET_RTG_CFS_THREAD = 5,

	GET_QOS_CLASS = 10,

	BEGIN_FRAME_FREQ = 20,
	END_FRAME_FREQ,

	BEGIN_ACTIVITY_FREQ,
	END_ACTIVITY_FREQ,
	END_FREQ,

	SET_MIN_UTIL = 30,
	SET_MARGIN,
	SET_MIM_UTIL_AND_MARGIN,
	SET_RME_MARGIN,
	GET_RME_MARGIN,
#ifdef CONFIG_HW_RTG_SCHED
	RT_CTRL_INIT_RENDER_PID,
	RT_CTRL_GET_RENDER_RT,
	RT_CTRL_STOP_RENDER_RT,
	RT_CTRL_GET_RENDER_HT,
	RT_CTRL_DESTROY_RENDER_RT,
	RT_CTRL_GET_DEV_CAP,
#endif

	ENABLE_RTG_BOOST = 100,

	CMD_ID_MAX,
};

enum ioctl_abi_format {
	IOCTL_ABI_ARM32,
	IOCTL_ABI_AARCH64,
};

struct rtg_qos_data {
	unsigned int is_rtg;
};

bool is_rtg_enable(void);
int proc_rtg_open(struct inode *inode, struct file *filp);
long proc_rtg_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#ifdef CONFIG_COMPAT
long proc_rtg_compat_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg);
#endif

#ifdef CONFIG_HW_MTK_RTG_SCHED
enum enable_type {
	ALL_ENABLE = 1,
	TRANS_ENABLE = 2,
	ENABLE_MAX
};

int get_enable_type(void);
#endif

#endif
