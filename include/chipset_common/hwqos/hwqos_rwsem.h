/*
 * hwqos_rwsem.h
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

#ifndef HWQOS_RWSEM_H
#define HWQOS_RWSEM_H

struct task_struct;

void rwsem_dynamic_qos_enqueue(struct task_struct *owner,
	struct task_struct *waiter_task);
void rwsem_dynamic_qos_dequeue(struct task_struct *task);
#endif
