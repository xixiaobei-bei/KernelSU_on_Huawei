/*
 * meminfo_lite.c
 *
 * A fast way to read system free and aviliable meminfo
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd
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
 */
#include <linux/fs.h>
#include <linux/hugetlb.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/vmstat.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <linux/swap.h>
#include <asm/page.h>
#include <asm/pgtable.h>

#define K_PER_PAGE_SHIFT	(PAGE_SHIFT - 10)

struct mem_swapinfo {
	unsigned long freeswap;
	unsigned long active_anon;
	unsigned long inactive_anon;
};

static int meminfo_lite_proc_show(struct seq_file *m, void *v)
{
	struct sysinfo i;
	long available;

	/* Display in kilobytes. */
	si_meminfo(&i);
	available = si_mem_available();

	/* Tagged format, for easy grepping and expansion. */
	seq_printf(m, "Fr:%lu,Av:%lu\n", i.freeram << K_PER_PAGE_SHIFT,
		(unsigned long)available << K_PER_PAGE_SHIFT);

	hugetlb_report_meminfo(m);

	return 0;
}

static int meminfo_lite_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, meminfo_lite_proc_show, NULL);
}

static const struct file_operations meminfo_lite_proc_fops = {
	.open		= meminfo_lite_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};


static int meminfo_sw_proc_show(struct seq_file *m, void *v)
{
	struct sysinfo i;
	struct mem_swapinfo s;

	si_swapinfo(&i);
	s.freeswap = i.freeswap;

	s.active_anon   =
		global_node_page_state(NR_LRU_BASE + LRU_ACTIVE_ANON);
	s.inactive_anon =
		global_node_page_state(NR_LRU_BASE + LRU_INACTIVE_ANON);
#define page_to_k(x) (((x) << (PAGE_SHIFT - 10)))
	seq_printf(m, "Sf:%lu,As:%lu\n",
		   page_to_k(s.freeswap),
		   page_to_k(s.active_anon + s.inactive_anon));
	return 0;
#undef page_to_k
}

static int meminfo_sw_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, meminfo_sw_proc_show, NULL);
}

static const struct file_operations meminfo_sw_fops = {
	.open		= meminfo_sw_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_meminfo_lite_init(void)
{
	proc_create("meminfo_lite", 0444, NULL, &meminfo_lite_proc_fops);
	proc_create("meminfo_sw", 0444, NULL, &meminfo_sw_fops);
	return 0;
}

module_init(proc_meminfo_lite_init);
