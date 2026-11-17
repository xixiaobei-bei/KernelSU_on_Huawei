/*
 * hwcfs_mutex.h
 *
 * mutex schedule declaration
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

#ifndef _HWCFS_MUTEX_H_
#define _HWCFS_MUTEX_H_

#include <linux/sched.h>
#include <linux/mutex.h>

void mutex_list_add(struct task_struct *task, struct list_head *entry,
	struct list_head *head, struct mutex *lock);
void mutex_dynamic_vip_enqueue(struct mutex *lock, struct task_struct *task);
void mutex_dynamic_vip_dequeue(struct mutex *lock, struct task_struct *task);
#endif
