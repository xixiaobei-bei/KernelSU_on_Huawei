/*
 *  reserve memory for key process
 *
 *  Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 *  Authors:
 *  liang hui <lianghuiliang.lianghui@huawei.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */

#ifdef CONFIG_HISI_QOS_MEM_OPT

#include <linux/hisi/qos_mem.h>
#include <linux/mmzone.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/mm_types.h>
#include <linux/module.h>

#include "internal.h"

#ifdef CONFIG_HISI_SLOW_PATH_COUNT
#include "slowpath_count.h"
#endif

#define MAX_RESERVED_ORDER                                    4
#define MAX_RESERVED_SIZE                                     10//10M
#define MIN_QOS_MEM_LEVEL                                     2 //min qos val

#define CMS_MTK_NAME                                          "Camera"
#define CMS_HISI_NAME                                         "camera"

struct qos_reserved_mem {
	spinlock_t lock;/*lock for each order*/
	unsigned int order;
	unsigned int water_mark;//max page to reserve
	unsigned int total_page;//current pool pages
	struct list_head free_list;
};

struct qos_reserved_mem g_reserved_mem;

static unsigned int zero;
static unsigned int one = 1;
static unsigned int max_reserved_size = MAX_RESERVED_SIZE;
static unsigned int qos_mem_watermark[MAX_RESERVED_ORDER + 1] = {0, 0, 1};

static unsigned int qos_mem_enable;
static unsigned int stack_water_mark;

static void drop_reserved_stack_memory(void)
{
	struct page *page;
	unsigned long flags;
	struct qos_reserved_mem *rsv_mem = &g_reserved_mem;

	spin_lock_irqsave(&rsv_mem->lock, flags);
	if (!rsv_mem->total_page) {
		spin_unlock_irqrestore(&rsv_mem->lock, flags);
		return;
	}

	while (rsv_mem->total_page) {
		page = list_first_entry_or_null(&rsv_mem->free_list, struct page, lru);
		if (page) {
			list_del(&page->lru);
			__free_pages(page, rsv_mem->order);
		}
		rsv_mem->total_page--;
	}
	spin_unlock_irqrestore(&rsv_mem->lock, flags);
}

static int qos_mem_watermark_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret = proc_dointvec_minmax(table, write, buffer, length, ppos);

	if (ret)
		return ret;

	if (write) {
		unsigned int order;
		unsigned int water_mark;
		unsigned int order_pages;

		for (order = 0; order < MAX_RESERVED_ORDER + 1; order++) {
			order_pages = SZ_1M / PAGE_SIZE;
			order_pages /= (unsigned int)(1 << order);
			water_mark = qos_mem_watermark[order] * order_pages;
			if (order == THREAD_SIZE_ORDER) {
				g_reserved_mem.water_mark = water_mark;
				stack_water_mark = water_mark;
				if (!water_mark)
					drop_reserved_stack_memory();
			}
		}
	}
	return ret;
}

static int qos_mem_enable_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret = proc_dointvec_minmax(table, write, buffer, length, ppos);

	if (ret)
		return ret;

	if (write) {
		if (!qos_mem_enable)//disable here free all pages
			drop_reserved_stack_memory();
	}

	return ret;
}

struct ctl_table qos_mem_ctl_table[] = {
	{
		.procname       = "watermark",
		.data           = &qos_mem_watermark,
		.maxlen         = sizeof(qos_mem_watermark),
		.mode           = 0640,
		.proc_handler   = qos_mem_watermark_handler,
		.extra1         = &zero,
		.extra2         = &max_reserved_size,
	},
	{
		.procname       = "enable",
		.data           = &qos_mem_enable,
		.maxlen	        = sizeof(unsigned int),
		.mode           = 0640,
		.proc_handler   = qos_mem_enable_handler,
		.extra1         = &zero,
		.extra2         = &one,
	},
	{ }
};

static inline bool is_vip_thread(void)
{
#ifdef CONFIG_HW_VIP_THREAD
	struct task_struct *leader = current->group_leader;
	return (leader->static_vip || atomic64_read(&leader->dynamic_vip));
#else
	return false;
#endif
}

static inline bool is_camera_thread(void)
{
	if (current->default_vip < 0) {
		if (strstr(current->group_leader->comm, CMS_MTK_NAME) ||
			strstr(current->group_leader->comm, CMS_HISI_NAME))
			current->default_vip = true;
		else
			current->default_vip = false;
	}

	return current->default_vip;
}

bool test_vip_alloc_thread(void)
{
	int qos = 0;

#ifdef CONFIG_HW_QOS_THREAD
	qos = get_task_qos(current);
#else
	if (is_vip_thread())
		qos = MIN_QOS_MEM_LEVEL;
#endif

	if (qos < MIN_QOS_MEM_LEVEL && is_camera_thread())
		qos = MIN_QOS_MEM_LEVEL;

	return (qos >= MIN_QOS_MEM_LEVEL);
}

unsigned int get_qos_cached_stack_size(void)
{
	unsigned int ret = 0;
	if (stack_water_mark)
		ret = g_reserved_mem.total_page;
	return ret << THREAD_SIZE_ORDER;
}

struct page *get_stack_page_from_pool(int order)
{
	unsigned long flags;
	struct page *page = NULL;
	struct qos_reserved_mem *rsv_mem;

	if (!qos_mem_enable || !stack_water_mark)
		return NULL;

	if (unlikely(order != THREAD_SIZE_ORDER))
		return NULL;

	if (!test_vip_alloc_thread())
		return NULL;

	rsv_mem = &g_reserved_mem;

	spin_lock_irqsave(&rsv_mem->lock, flags);
	if (rsv_mem->total_page) {
		page = list_first_entry_or_null(&rsv_mem->free_list, struct page, lru);
		if (page)
			list_del(&page->lru);
		else
			pr_info("stack alloc fail O:%d T:%d why?\n", order, rsv_mem->total_page);
		rsv_mem->total_page--;
	}
	spin_unlock_irqrestore(&rsv_mem->lock, flags);

#ifdef CONFIG_HISI_SLOW_PATH_COUNT
	if (page)
		qos_alloc_count_inc();
#endif

	return page;
}

bool free_stack_page_to_pool(struct page *page, unsigned int order)
{
	unsigned long flags;
	struct qos_reserved_mem *rsv_mem = &g_reserved_mem;

	if (unlikely(!page || order != THREAD_SIZE_ORDER))
		return 1;

	if (!qos_mem_enable || !stack_water_mark)
		return 1;

	if (rsv_mem->total_page >= rsv_mem->water_mark)
		return 1;// no lock for better performance

	spin_lock_irqsave(&rsv_mem->lock, flags);
	list_add(&page->lru, &rsv_mem->free_list);
	rsv_mem->total_page++;
	spin_unlock_irqrestore(&rsv_mem->lock, flags);

	return 0;
}

static int __init qos_mem_init(void)
{
	unsigned int order;
	unsigned int order_pages;
	unsigned int water_marks;
	struct qos_reserved_mem *rsv_mem;

	for (order = 0; order < MAX_RESERVED_ORDER + 1; order++) {
		order_pages = SZ_1M / PAGE_SIZE;
		order_pages /= (unsigned int)(1 << order);
		water_marks = qos_mem_watermark[order] * order_pages;
		if (order == THREAD_SIZE_ORDER) {
			rsv_mem = &g_reserved_mem;
			spin_lock_init(&rsv_mem->lock);
			rsv_mem->order = order;
			rsv_mem->total_page = 0;
			rsv_mem->water_mark = water_marks;
			stack_water_mark = rsv_mem->water_mark;
			INIT_LIST_HEAD(&rsv_mem->free_list);
		}
	}

	qos_mem_enable = false;
	pr_info("mem_qos inited\n");
	return 0;
}

module_init(qos_mem_init);
#endif
