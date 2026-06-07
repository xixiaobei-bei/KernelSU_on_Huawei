/*
 * iaware_qos.h
 *
 * Qos schedule declaration
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <linux/types.h>

#define QOS_CTRL_MAGIC 'q'

enum {
	GET_QOS_STAT = 1,
	SET_QOS_STAT_THREAD,
	GET_QOS_WHOLE,
	SET_QOS_STAT_PROC,
	QOS_CTRL_MAX_NR,
};

#define QOS_CTRL_GET_QOS_STAT \
	_IOR(QOS_CTRL_MAGIC, GET_QOS_STAT, struct qos_stat)
#define QOS_CTRL_SET_QOS_STAT_THREAD \
	_IOWR(QOS_CTRL_MAGIC, SET_QOS_STAT_THREAD, struct qos_stat)
#define QOS_CTRL_SET_QOS_STAT_PROC \
	_IOWR(QOS_CTRL_MAGIC, SET_QOS_STAT_PROC, struct qos_stat)
#define QOS_CTRL_GET_QOS_WHOLE \
	_IOR(QOS_CTRL_MAGIC, GET_QOS_WHOLE, struct qos_whole)

struct qos_stat {
	pid_t pid;
	int qos;
};

struct qos_whole {
	pid_t pid;
	int proc_qos;
	int proc_usage;
	int thread_qos;
	int thread_usage;
	int trans_flags;
};

#endif
