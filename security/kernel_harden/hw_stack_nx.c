/*
 * hw_stack_nx.c
 *
 * Huawei Kernel Harden, stack nx
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/cacheflush.h>

void __init set_init_stack_nx(struct thread_info *ti)
{
	set_memory_nx((unsigned long)ti, THREAD_SIZE/PAGE_SIZE);
}

void set_task_stack_nx(struct thread_info *ti)
{
	set_memory_nx((unsigned long)ti, THREAD_SIZE/PAGE_SIZE);
}
