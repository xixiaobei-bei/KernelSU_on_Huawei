/*
 * hwcfs_mutex.c
 *
 * mutex schedule implementation
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#ifdef CONFIG_HW_VIP_THREAD
/*lint -save -e578 -e695 -e571*/
#include <chipset_common/hwcfs/hwcfs_mutex.h>

#include <linux/version.h>
#include <linux/list.h>
#include <chipset_common/hwcfs/hwcfs_common.h>

static void mutex_list_add_vip(struct list_head *entry, struct list_head *head)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct mutex_waiter *waiter = NULL;

	list_for_each_safe(pos, n, head) {
		waiter = list_entry(pos, struct mutex_waiter, list);
		if (!test_task_vip(waiter->task)) {
			list_add(entry, waiter->list.prev);
			return;
		}
	}
	if (pos == head)
		list_add_tail(entry, head);
}

void mutex_list_add(struct task_struct *task,
	struct list_head *entry, struct list_head *head, struct mutex *lock)
{
	bool is_vip = test_set_dynamic_vip(task);

	if (!entry || !head || !lock)
		return;

	if (is_vip && !lock->vip_dep_task)
		mutex_list_add_vip(entry, head);
	else
		list_add_tail(entry, head);
}

void mutex_dynamic_vip_enqueue(struct mutex *lock, struct task_struct *task)
{
	bool is_vip = false;
	struct task_struct *owner = NULL;

	if (!lock)
		return;

	is_vip = test_set_dynamic_vip(task);
#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
	owner = __mutex_owner(lock);
#else
	owner = lock->owner;
#endif
	if (is_vip && !lock->vip_dep_task && owner && !test_task_vip(owner)) {
		dynamic_vip_enqueue(owner, DYNAMIC_VIP_MUTEX, task->vip_depth);
		lock->vip_dep_task = owner;
	}
}

void mutex_dynamic_vip_dequeue(struct mutex *lock, struct task_struct *task)
{
	if (lock && lock->vip_dep_task == task) {
		dynamic_vip_dequeue(task, DYNAMIC_VIP_MUTEX);
		lock->vip_dep_task = NULL;
	}
}

/*lint -restore*/
#endif
