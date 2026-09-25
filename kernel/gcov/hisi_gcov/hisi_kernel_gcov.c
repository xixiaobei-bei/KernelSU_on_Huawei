/*
 * hisi_kernel_gcov.c
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * Author: nobody
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/hisi/mntn_dump.h>
#include <gcov.h>

MODULE_LICENSE("GPL");

extern noinline int atfd_hisi_service_freqdump_smc(u64, u64, u64, u64);
extern void gcov_iter_new_gcov_get_panic_gcda(struct gcov_info *);
extern size_t convert_to_gcda(char *buffer, struct gcov_info *);

void *gcov_gcda_malloc_ptr;
void *gcov_gcda_malloc_ptr_phy;
void *gcov_gcda_malloc_ptr_curr;
unsigned int g_count_gcda;
#define GCDA_TOTAL_SIZE_OF_4M  (4*1024*1024)

noinline void gcov_freqdump_atf(void)
{
	pr_err("%s: call start!\n", __func__);
	atfd_hisi_service_freqdump_smc(0xC700EE0A, 0, 0, 0);
	pr_err("%s: call end!\n", __func__);
	return;
}

void hisi_kernel_gcov_init(void)
{
	struct page * page_ptr = NULL;
	page_ptr = alloc_pages(GFP_KERNEL | __GFP_ZERO, 10);
	if (page_ptr) {
		gcov_gcda_malloc_ptr = page_to_virt(page_ptr);
		gcov_gcda_malloc_ptr_curr = gcov_gcda_malloc_ptr;
		gcov_gcda_malloc_ptr_phy = page_to_phys(page_ptr);
		pr_err("gcov_gcda_malloc_ptr phy: %lx,virt = %lx\n", gcov_gcda_malloc_ptr_phy, gcov_gcda_malloc_ptr);
	}
}

static void mntn_dump_gcov_data(unsigned int size)
{
	int ret;
	struct mdump_gcov *head;

	if (!gcov_gcda_malloc_ptr_phy)
		return;

	ret = register_mntn_dump(MNTN_DUMP_GCOV,
		(unsigned int)sizeof(struct mdump_gcov), (void **)&head);
	if (ret) {
		pr_err("register gcda buf fail\n");
		return;
	}

	if (!head) {
		pr_err("%s, head is NULL!\n", __func__);
		return;
	}

	head->gcda_addr = gcov_gcda_malloc_ptr_phy;
	head->gcda_size = size;
}

void gcov_get_gcda(void)
{
	struct gcov_info *info = NULL;

	if (NULL == gcov_gcda_malloc_ptr) {
		pr_err("gcov_gcda_malloc_ptr  NULL\n");
		return ;
	}
	mutex_lock(&gcov_lock);
	g_count_gcda = 0;
	gcov_gcda_malloc_ptr_curr += 4;
	/* Perform event callback for previously registered entries. */
	while ((info = gcov_info_next(info))) {
		if(gcov_gcda_malloc_ptr_curr - gcov_gcda_malloc_ptr > GCDA_TOTAL_SIZE_OF_4M) {
			pr_err("sorry ,no space to store gcda\n");
			break;
		}
		gcov_iter_new_gcov_get_panic_gcda(info);
		cond_resched();
	}
	*(unsigned int*)(gcov_gcda_malloc_ptr) = g_count_gcda;

	mntn_dump_gcov_data((unsigned int)(gcov_gcda_malloc_ptr_curr - gcov_gcda_malloc_ptr));

	pr_err("g_count_gcda = %d,gcov_gcda_malloc_ptr_phy = %lx\n",g_count_gcda,gcov_gcda_malloc_ptr_phy);

	mutex_unlock(&gcov_lock);
}
