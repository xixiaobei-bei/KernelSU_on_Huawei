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
#include "include/aux.h"
#include "include/proc_state.h"
#include "include/set_rtg.h"
#include "../kernel/sched/sched.h"
#include <linux/sched/frame.h>

#define RTG_FRAME_PRIO 1

atomic_t g_aux_thread_num = ATOMIC_INIT(0);

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

static inline int get_grp_util(const struct aux_info *aux_info)
{
	return max_t(unsigned long, aux_info->min_util, aux_info->boost_util);
}

static int set_aux_param(const struct aux_info *info)
{
	struct aux_info *aux_info = rtg_aux_info();

	if (!info || !aux_info)
		return -INVALID_ARG;

	// cluster will not change, do not need update
	if (atomic_read(&g_aux_thread_num) > 0 &&
		aux_info->prio == info->prio &&
		aux_info->min_util == info->min_util)
		return -INVALID_ARG;

	if (info->min_util < 0 || info->min_util > DEFAULT_MAX_UTIL)
		return -INVALID_ARG;

	if (info->prio < 0 || info->prio == RTG_FRAME_PRIO)
		return -INVALID_ARG;

	aux_info->min_util = info->min_util;
	if (atomic_read(&g_aux_thread_num) == 0)
		aux_info->min_util = 0;

	aux_info->prio = info->prio;
	(void)sched_set_group_normalized_util(DEFAULT_AUX_ID, get_grp_util(aux_info), FRAME_NORMAL_UPDATE);

	FRAME_SYSTRACE("AUX_MIN_UTIL", get_grp_util(aux_info), smp_processor_id());
	return 0;
}

int sched_rtg_aux(int tid, int enable, const struct aux_info *info)
{
	struct aux_info *aux_info = rtg_aux_info();
	int err;

	if (!info || !aux_info)
		return -INVALID_ARG;

	if (enable < 0 || enable > 1)
		return -INVALID_ARG;

	err = set_aux_param(info);
	if (err != 0)
		return -ERR_SET_AUX_RTG;

	err = set_rtg_grp(tid, enable == 1, DEFAULT_AUX_ID, DEFAULT_RT_PRIO);
	if (err != 0)
		return -ERR_SET_AUX_RTG;

	if (enable == 1)
		atomic_inc(&g_aux_thread_num);
	else if (atomic_read(&g_aux_thread_num) > 0)
		atomic_dec(&g_aux_thread_num);
	else
		atomic_set(&g_aux_thread_num, 0);

	if (atomic_read(&g_aux_thread_num) > 0)
		FRAME_SYSTRACE("AUX_SCHED_ENABLE", 1, smp_processor_id());
	else
		FRAME_SYSTRACE("AUX_SCHED_ENABLE", 0, smp_processor_id());

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
	struct aux_task_list *aux_task;
	struct aux_task_list *pos, *n;

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
	FRAME_SYSTRACE("AUX_SCHED_ENABLE", 0, smp_processor_id());
}

void update_aux_info_tick(struct related_thread_group *grp)
{
	struct aux_info *aux_info;

	rcu_read_lock();
	aux_info = __rtg_aux_info(grp);
	rcu_read_unlock();

	if (!aux_info)
		return;

	(void)sched_set_group_normalized_util(DEFAULT_AUX_ID, get_grp_util(aux_info), FRAME_NORMAL_UPDATE);

	FRAME_SYSTRACE("AUX_MIN_UTIL", get_grp_util(aux_info), smp_processor_id());
}

const struct rtg_class aux_rtg_class = {
	.sched_update_rtg_tick = update_aux_info_tick,
};

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
	grp->rtg_class = &aux_rtg_class;
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
