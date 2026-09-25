/*
 * aux_info.h
 *
 * aux grp info header
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "include/aux_info.h"
#include <trace/events/sched.h>
#include "include/aux.h"
#include "include/proc_state.h"
#include "include/set_rtg.h"
#include "../kernel/sched/sched.h"
#include <linux/sched/frame.h>

#define RTG_FRAME_PRIO 1
#define MAX_RT_AUX_THREADS 5

atomic_t g_aux_thread_num = ATOMIC_INIT(0);
atomic_t g_rt_aux_thread_num = ATOMIC_INIT(0);

static unsigned long g_frame_cluster_update_jiffies;
static unsigned long g_aux_cluster_update_jiffies;

static struct sched_cluster *g_big_cluster;
static struct sched_cluster *g_middle_cluster;

struct aux_task_list {
	struct list_head list;
	struct task_struct *task;
};

static inline struct related_thread_group *aux_rtg(void)
{
	return lookup_related_thread_group(DEFAULT_AUX_ID);
}

static inline struct aux_info *__rtg_aux_info(struct related_thread_group *grp)
{
	return (struct aux_info *) grp->private_data;
}

static struct aux_info *rtg_aux_info(void)
{
	struct aux_info *aux_info = NULL;
	struct related_thread_group *grp = aux_rtg();

	if (!grp)
		return NULL;

	aux_info = __rtg_aux_info(grp);
	return aux_info;
}

static bool is_rt_task(struct task_struct *task)
{
	bool ret = false;
	if (!task) {
		return ret;
	}
	ret = (task->prio < MAX_RT_PRIO) && (task->rtg_depth == STATIC_RTG_DEPTH);
	return ret;
}

static int get_rt_thread_num()
{
	int ret = 0;
	struct task_struct *p = NULL;
	struct related_thread_group *grp = NULL;
	unsigned long flags;

	grp = aux_rtg();
	if (!grp)
		return ret;

	raw_spin_lock_irqsave(&grp->lock, flags);
	if (list_empty(&grp->tasks)) {
		raw_spin_unlock_irqrestore(&grp->lock, flags);
		return ret;
	}

	list_for_each_entry(p, &grp->tasks, grp_list) {
		if (is_rt_task(p))
			++ret;
	}
	raw_spin_unlock_irqrestore(&grp->lock, flags);
	return ret;
}

int sched_rtg_aux(int tid, int enable, const struct aux_info *info)
{
	int err;

	if (!info)
		return -INVALID_ARG;

	if ((info->prio < 0) || (info->prio + 2 > MAX_RT_PRIO - DEFAULT_RT_PRIO))
		return -INVALID_ARG;

	if (enable < 0 || enable > 1)
		return -INVALID_ARG;

	if ((enable == 1) && (atomic_read(&g_rt_aux_thread_num) == MAX_RT_AUX_THREADS))
		err = set_rtg_grp(tid, enable == 1, DEFAULT_AUX_ID, NOT_RT_PRIO, info->min_util);
	else
		err = set_rtg_grp(tid, enable == 1, DEFAULT_AUX_ID, DEFAULT_RT_PRIO + info->prio, info->min_util);

	if (err != 0)
		return -ERR_SET_AUX_RTG;

	if ((enable == 1) && (atomic_read(&g_rt_aux_thread_num) < MAX_RT_AUX_THREADS))
		atomic_inc(&g_rt_aux_thread_num);
	else
		atomic_set(&g_rt_aux_thread_num, get_rt_thread_num());

	trace_rtg_frame_sched("AUX_SCHED_ENABLE", atomic_read(&g_rt_aux_thread_num));
	return SUCC;
}

int set_aux_boost_util(int util)
{
	struct aux_info *aux_info = rtg_aux_info();

	if (!aux_info)
		return -INVALID_ARG;

	if (util < 0 || util > DEFAULT_MAX_UTIL)
		return -INVALID_ARG;

	aux_info->boost_util = util;
	return SUCC;
}

void stop_aux_sched(void)
{
	struct task_struct *p = NULL;
	struct related_thread_group *grp = NULL;
	unsigned long flags;
	struct list_head task_list_head;
	struct aux_task_list *aux_task = NULL;
	struct aux_task_list *pos = NULL;
	struct aux_task_list *n = NULL;

	grp = aux_rtg();
	if (!grp)
		return;

	raw_spin_lock_irqsave(&grp->lock, flags);
	if (list_empty(&grp->tasks)) {
		raw_spin_unlock_irqrestore(&grp->lock, flags);
		return;
	}

	INIT_LIST_HEAD(&task_list_head);
	list_for_each_entry(p, &grp->tasks, grp_list) {
		if (!p)
			continue;
		get_task_struct(p);
		aux_task = kzalloc(sizeof(*aux_task), GFP_KERNEL);
		if (aux_task == NULL) {
			put_task_struct(p);
			continue;
		}
		aux_task->task = p;
		list_add_tail(&aux_task->list, &task_list_head);
	}
	raw_spin_unlock_irqrestore(&grp->lock, flags);

	list_for_each_entry_safe(pos, n, &task_list_head, list) {
		set_rtg_sched(pos->task, false, DEFAULT_AUX_ID, DEFAULT_RT_PRIO);
		put_task_struct(pos->task);
		list_del(&pos->list);
		kfree(pos);
	}

	atomic_set(&g_aux_thread_num, 0);
	trace_rtg_frame_sched("AUX_SCHED_ENABLE", 0);
}

static struct aux_info g_aux_info;
static int __init init_aux_info(void)
{
	struct related_thread_group *grp = NULL;
	struct aux_info *aux_info = NULL;
	unsigned long flags;

	g_frame_cluster_update_jiffies = 0;
	g_aux_cluster_update_jiffies = 0;
	g_big_cluster = 0;
	g_middle_cluster = 0;

	aux_info = &g_aux_info;
	memset(aux_info, 0, sizeof(struct aux_info));

	grp = aux_rtg();

	raw_spin_lock_irqsave(&grp->lock, flags);
	grp->private_data = aux_info;
	grp->rtg_class = NULL;
	raw_spin_unlock_irqrestore(&grp->lock, flags);

	if (!g_big_cluster) {
		g_big_cluster = max_cap_cluster();

		// littile, middle, big
		if (g_big_cluster && g_big_cluster->id == 2)
			g_middle_cluster = list_prev_entry(g_big_cluster, list);

		// middle id is illegal
		if (g_middle_cluster && g_middle_cluster->id != 1)
			g_middle_cluster = NULL;
	}
	return 0;
}
late_initcall(init_aux_info);
