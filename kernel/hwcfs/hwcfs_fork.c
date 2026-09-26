/*
 * hwcfs_fork.c
 *
 * hwcfs fork interface implementation
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwcfs/hwcfs_fork.h>

void init_task_vip_info(struct task_struct *task)
{
	if (!task)
		return;
	task->static_vip = 0;
	atomic64_set(&(task->dynamic_vip), 0);
	INIT_LIST_HEAD(&task->vip_entry);
	task->vip_depth = 0;
	task->enqueue_time = 0;
	task->dynamic_vip_start = 0;
}
#endif
