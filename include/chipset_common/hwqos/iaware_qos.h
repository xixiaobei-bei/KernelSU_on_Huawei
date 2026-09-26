/*
 * iaware_qos.h
 *
 * Qos schedule declaration
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef IAWARE_QOS_H
#define IAWARE_QOS_H

#include <linux/sched/task.h>
#include <linux/types.h>
#include <chipset_common/hwqos/hwqos_common.h>

#define QOS_CTRL_MAGIC 'q'

enum {
	GET_QOS_STAT = 1,
	SET_QOS_STAT_THREAD,
	GET_QOS_WHOLE,
	SET_QOS_STAT_PROC,
	SET_TASK_VIP_PRIO,
	SET_TASK_MIN_UTIL,
	CTRL_MAX_NR,
};

#define QOS_CTRL_GET_QOS_STAT \
	_IOR(QOS_CTRL_MAGIC, GET_QOS_STAT, struct task_config)
#define QOS_CTRL_SET_QOS_STAT_THREAD \
	_IOWR(QOS_CTRL_MAGIC, SET_QOS_STAT_THREAD, struct task_config)
#define QOS_CTRL_SET_QOS_STAT_PROC \
	_IOWR(QOS_CTRL_MAGIC, SET_QOS_STAT_PROC, struct task_config)
#define QOS_CTRL_GET_QOS_WHOLE \
	_IOR(QOS_CTRL_MAGIC, GET_QOS_WHOLE, struct qos_whole)
#define CTRL_SET_TASK_VIP_PRIO \
		_IOW(QOS_CTRL_MAGIC, SET_TASK_VIP_PRIO, struct task_config)
#define CTRL_SET_TASK_MIN_UTIL \
		_IOW(QOS_CTRL_MAGIC, SET_TASK_MIN_UTIL, struct task_config)

struct qos_whole {
	pid_t pid;
	int proc_qos;
	int proc_usage;
	int thread_qos;
	int thread_usage;
	int trans_flags;
};

struct task_config {
	pid_t pid;
	int value;
};

extern struct trans_qos_allow g_qos_trans_allows[QOS_TRANS_THREADS_NUM];
extern spinlock_t g_trans_qos_lock;

#endif
