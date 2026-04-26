/*
 * hwqos_rwsem.c
 *
 * Qos schedule implementation
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

#include <linux/sched.h>

#include <chipset_common/hwqos/hwqos_common.h>

#define RWSEM_ANONYMOUSLY_OWNED (1UL << 0)

static inline bool rwsem_owner_is_writer(struct task_struct *owner)
{
	return owner && (!(((uintptr_t)owner) & RWSEM_ANONYMOUSLY_OWNED));
}

void rwsem_dynamic_qos_enqueue(struct task_struct *owner,
	struct task_struct *waiter_task)
{
	if (rwsem_owner_is_writer(owner))
		dynamic_qos_enqueue(owner, waiter_task, DYNAMIC_QOS_RWSEM);
}

void rwsem_dynamic_qos_dequeue(struct task_struct *task)
{
	dynamic_qos_dequeue(task, DYNAMIC_QOS_RWSEM);
}
