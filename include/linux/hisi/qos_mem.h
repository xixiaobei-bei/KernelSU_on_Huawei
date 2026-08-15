/*
 * reserve memory for key process
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * Authors:
 * liang hui <lianghuiliang.lianghui@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifdef CONFIG_HISI_QOS_MEM_OPT

#include <linux/types.h>
#include <linux/mm_types.h>
#include <linux/sysctl.h>

extern struct ctl_table qos_mem_ctl_table[];

#ifdef CONFIG_HW_QOS_THREAD
extern int get_task_qos(struct task_struct *task);
#endif

bool test_vip_alloc_thread(void);
unsigned int get_qos_cached_stack_size(void);
struct page *get_stack_page_from_pool(int order);// for fork
bool free_stack_page_to_pool(struct page *page, unsigned int order);// for fork
#endif
