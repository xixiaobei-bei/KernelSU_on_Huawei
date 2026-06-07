/*
 * hwqos_common.h
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

#ifndef HWQOS_COMMON_H
#define HWQOS_COMMON_H

#include <linux/types.h>
#include <chipset_common/hwqos/hwqos_sysctl.h>

#define QOS_TRANS_THREADS_NUM 8

struct task_struct;

void set_task_qos_by_tid(struct task_struct *task, int qos);
void set_task_qos_by_pid(struct task_struct *task, int qos);
int get_task_set_qos(struct task_struct *task);
int get_task_trans_qos(struct transact_qos *tq);
int get_task_qos(struct task_struct *task);
bool dynamic_qos_enqueue(struct task_struct *task,
	struct task_struct *from, unsigned int type);
void dynamic_qos_dequeue(struct task_struct *task, unsigned int type);
void iaware_proc_fork_inherit(struct task_struct *task,
	unsigned long clone_flags);
#endif
