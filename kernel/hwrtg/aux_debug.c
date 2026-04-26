/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
 * aux grp debug header
 */

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <../kernel/sched/sched.h>

#include "include/aux_info.h"
#include "include/aux.h"
#include "include/set_rtg.h"

/*
 * This allows printing both to /proc/sched_debug and
 * to the console
 */
#define aux_seq_printf(sfile, info...) \
do { \
	if (sfile) \
		seq_printf(sfile, info); \
	else \
		printk(info); \
} while (0)

static void print_aux_info(const struct seq_file *sfile,
	const struct related_thread_group *grp)
{
	struct aux_info *aux_info = (struct aux_info *) grp->private_data;

	aux_seq_printf(sfile, "AUX_INFO    : MIN_UTIL:%d##BOOST_UTIL:%d##PRIO:%d\n",
		aux_info->min_util,
		aux_info->boost_util,
		aux_info->prio);
#ifdef CONFIG_HW_RTG_SCHED
	aux_seq_printf(sfile, "AUX_CLUSTER : %d\n",
		grp->preferred_cluster ? grp->preferred_cluster->id : -1);
#endif
#ifdef CONFIG_HW_RTG_SCHED_RT_THREAD_LIMIT
	aux_seq_printf(sfile, "RTG_RT_THREAD_NUM : %d/%d\n", read_rtg_rt_thread_num(), RTG_MAX_RT_THREAD_NUM);
#endif
}

static char aux_task_state_to_char(const struct task_struct *tsk)
{
	static const char state_char[] = "RSDTtXZPI";
	unsigned int tsk_state = READ_ONCE(tsk->state);
	unsigned int state = (tsk_state | tsk->exit_state) & TASK_REPORT;

	BUILD_BUG_ON_NOT_POWER_OF_2(TASK_REPORT_MAX);
	BUILD_BUG_ON(1 + ilog2(TASK_REPORT_MAX) != sizeof(state_char) - 1);

	if (tsk_state == TASK_IDLE)
		state = TASK_REPORT_IDLE;

	return state_char[fls(state)];
}

static void print_aux_task_header(const struct seq_file *sfile,
	const char *header, int run, int nr)
{
	aux_seq_printf(sfile,
		"%s   : %d/%d\n"
#ifdef CONFIG_SCHED_HISI_TASK_MIN_UTIL
		"STATE		COMM	TGID	PID	PRIO	  CPU	  UTIL\n"
#else
		"STATE		COMM	TGID	PID	PRIO	  CPU\n"
#endif
		"---------------------------------------------------------\n",
		header, run, nr);
}

static void print_aux_task(const struct seq_file *sfile,
	const struct task_struct *p)
{
#ifdef CONFIG_SCHED_HISI_TASK_MIN_UTIL
	aux_seq_printf(sfile, "%5c %15s %6d %6d %6d %5d(%*pbl) %8d\n",
#else
	aux_seq_printf(sfile, "%5c %15s %6d %6d %6d %5d(%*pbl)\n",
#endif
		aux_task_state_to_char(p), p->comm,
		p->tgid, p->pid, p->prio,
#ifdef CONFIG_SCHED_HISI_TASK_MIN_UTIL
		task_cpu(p), cpumask_pr_args(&p->cpus_allowed),
		p->uclamp.min_util);
#else
		task_cpu(p), cpumask_pr_args(&p->cpus_allowed));
#endif
}

static void print_aux_threads(const struct seq_file *sfile,
	const struct related_thread_group *grp)
{
	struct task_struct *p = NULL;
	int nr_thread = 0;

	list_for_each_entry(p, &grp->tasks, grp_list) {
		nr_thread++;
	}
	print_aux_task_header(sfile, "AUX_THREADS", grp->nr_running, nr_thread);
	list_for_each_entry(p, &grp->tasks, grp_list) {
		if (!p)
			continue;
		get_task_struct(p);
		print_aux_task(sfile, p);
		put_task_struct(p);
	}
}

static int sched_aux_debug_show(struct seq_file *sfile, void *v)
{
	struct related_thread_group *grp = NULL;
	unsigned long flags;

	grp = lookup_related_thread_group(DEFAULT_AUX_ID);
	if (!grp) {
		aux_seq_printf(sfile, "IPROVISION AUX none\n");
		return 0;
	}

	raw_spin_lock_irqsave(&grp->lock, flags);
	if (list_empty(&grp->tasks)) {
		raw_spin_unlock_irqrestore(&grp->lock, flags);
		aux_seq_printf(sfile, "IPROVISION AUX tasklist empty\n");
		return 0;
	}

	print_aux_info(sfile, grp);
	print_aux_threads(sfile, grp);
	raw_spin_unlock_irqrestore(&grp->lock, flags);

	return 0;
}

static int sched_aux_debug_release(struct inode *inode, struct file *file)
{
	seq_release(inode, file);
	return 0;
}

static int sched_aux_debug_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, sched_aux_debug_show, NULL);
}

static const struct file_operations sched_aux_debug_fops = {
	.open = sched_aux_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = sched_aux_debug_release,
};

static int __init init_aux_sched_debug_procfs(void)
{
	struct proc_dir_entry *pe = NULL;

	pe = proc_create("sched_aux_debug",
		0444, NULL, &sched_aux_debug_fops);
	if (!pe)
		return -ENOMEM;
	return 0;
}
late_initcall(init_aux_sched_debug_procfs);
