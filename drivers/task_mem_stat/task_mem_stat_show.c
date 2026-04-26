/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * task_mem_stat_show.c
 *
 * Huawei memory stats for every kernel task, support Slab/Shmem/PageTable.
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

#include <task_mem_stat/task_mem_stat.h>

#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/sched/signal.h>
#include <linux/seq_file.h>

#define KILO_TRANS_BITS 10

struct task_kmem_stat_track
{
	unsigned long total_slab;
	unsigned long total_slab_reclaimable;
	unsigned long total_slab_unreclaim;
	unsigned long total_ptable;
	unsigned long total_shmem;
};

void task_kmem_stat_track_init(struct task_kmem_stat_track *track)
{
	track->total_slab = 0;
	track->total_slab_reclaimable = 0;
	track->total_slab_unreclaim = 0;
	track->total_ptable = 0;
	track->total_shmem = 0;
}

static void task_kmem_stat_print(struct seq_file *m, const struct task_kmem_stat_track *track)
{
	seq_printf(m, "\nTotal:\n"
		      "Slab:         %8lu pgs  /%lu kB\n"
		      "SReclaimable: %8lu pgs  /%lu kB\n"
		      "SUnreclaim:   %8lu pgs  /%lu kB\n"
		      "Shmem:        %8lu pgs  /%lu kB\n"
		      "PageTables:   %8lu bytes/%lu kB\n",
		      track->total_slab,
		      ((track->total_slab) << (PAGE_SHIFT - KILO_TRANS_BITS)),
		      track->total_slab_reclaimable,
		      ((track->total_slab_reclaimable) << (PAGE_SHIFT - KILO_TRANS_BITS)),
		      track->total_slab_unreclaim,
		      ((track->total_slab_unreclaim) << (PAGE_SHIFT - KILO_TRANS_BITS)),
		      track->total_shmem,
		      ((track->total_shmem) << (PAGE_SHIFT - KILO_TRANS_BITS)),
		      track->total_ptable,
		      ((track->total_ptable) >> KILO_TRANS_BITS));
}

static void add_task_slab_total(bool isReclaimable, struct task_struct *task, long value)
{
	atomic_long_add(value, (isReclaimable ? &task->slab_reclaimable : &task->slab_unreclaim));
}

static long get_task_slab_total(bool isReclaimable, const struct task_struct *task)
{
	if (!task)
		return 0;
	return atomic_long_read(isReclaimable ? &task->slab_reclaimable : &task->slab_unreclaim);
}

void add_slab_count(bool isReclaimable, long value)
{
	add_task_slab_total(isReclaimable, current->group_leader, value);
}

static int all_task_kmem_show(struct seq_file *m, void *v)
{
	struct task_struct *task = NULL;
	struct task_kmem_stat_track track;
	unsigned long slab_reclaimable;
	unsigned long slab_unreclaim;
	unsigned long ptable;
	unsigned long shmem;

	task_kmem_stat_track_init(&track);
	for_each_process(task) {
		slab_reclaimable = 0;
		slab_unreclaim = 0;
		ptable = 0;
		shmem = 0;
		if (task->group_leader == task) {
			slab_reclaimable = get_task_slab_total(true, task);
			slab_unreclaim = get_task_slab_total(false, task);
			if (slab_reclaimable < 0 || slab_unreclaim < 0)
				pr_err("kmem_total_slab_negative:%s,reclaimable=%ld,unreclaim=%ld",
				       task->comm, slab_reclaimable, slab_unreclaim);
			track.total_slab_reclaimable += slab_reclaimable;
			track.total_slab_unreclaim += slab_unreclaim;
		}
		if (task->mm) {
			ptable = (PTRS_PER_PTE * sizeof(pte_t) *
				  atomic_long_read(&task->mm->nr_ptes)) +
				  (PTRS_PER_PMD * sizeof(pmd_t) * mm_nr_pmds(task->mm));
			shmem = get_mm_counter(task->mm, MM_SHMEMPAGES);
			track.total_ptable += ptable;
			track.total_shmem += shmem;
		}
		seq_printf(m, "%d,%s,%ld,%lu,%lu,%lu,%lu\n",
			   task->pid, task->comm, task->state,
			   slab_reclaimable, slab_unreclaim, ptable, shmem);
	}
	track.total_slab = track.total_slab_reclaimable + track.total_slab_unreclaim;
	task_kmem_stat_print(m, &track);
	return 0;
}

static int task_mem_stat_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, all_task_kmem_show, NULL);
}

static const struct file_operations task_mem_stat_proc_fops = {
	.open = task_mem_stat_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init proc_task_mem_stat_init(void)
{
	proc_create("task_mem_stat", 0440, NULL, &task_mem_stat_proc_fops);
	return 0;
}
fs_initcall(proc_task_mem_stat_init);
