/*
 * set_rtg.c
 *
 * set rtg sched
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

#include "include/set_rtg.h"

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/types.h>
#include <../kernel/sched/sched.h>

int set_rtg_sched(struct task_struct *task, bool is_rtg, int grp_id, int prio)
{
	int err = -1;
	int policy = SCHED_NORMAL;
	int grpid = DEFAULT_RTG_GRP_ID;
	struct sched_param sp = {0};

	if (!task)
		return err;

	if (is_rtg && (prio < 0 || prio > MAX_USER_RT_PRIO - 1))
		return err;

	if (is_rtg && task->sched_class != &fair_sched_class)
		return err;

	if (in_interrupt()) {
		pr_err("[AWARE_RTG]: %s is in interrupt\n", __func__);
		return err;
	}

	if (is_rtg) {
		policy = SCHED_FIFO | SCHED_RESET_ON_FORK;
		sp.sched_priority = MAX_USER_RT_PRIO - 1 - prio;
		grpid = grp_id;
	}

	err = sched_setscheduler_nocheck(task, policy, &sp);
	if (err < 0) {
		pr_err("[AWARE_RTG]: %s task:%d  setscheduler err:%d\n",
				__func__, task->pid, err);
		return err;
	}

	err = _sched_set_group_id(task, grpid); //lint !e732
	if (err < 0) {
		pr_err("[AWARE_RTG]: %s task:%d set_group_id err:%d\n",
				__func__, task->pid, err);
		if (is_rtg) {
			policy = SCHED_NORMAL;
			sp.sched_priority = 0;
			sched_setscheduler_nocheck(task, policy, &sp);
		}
	}
	return err;
}

int set_rtg_grp(pid_t tid, bool is_rtg, int grp_id, int prio)
{
	struct task_struct *task;
	int err = -1;

	if (grp_id < DEFAULT_RTG_GRP_ID || grp_id >= MAX_NUM_CGROUP_COLOC_ID)
		return err;

	if (is_rtg && (prio < 0 || prio > MAX_USER_RT_PRIO - 1))
		return err;

	if (tid <= 0)
		return err;
	rcu_read_lock();
	task = find_task_by_vpid(tid);
	if (!task) {
		pr_err("[AWARE_RTG] %s tid:%d task is null\n", __func__, tid);
		rcu_read_unlock();
		return err;
	}
	get_task_struct(task);
	rcu_read_unlock();

	if (set_rtg_sched(task, is_rtg, grp_id, prio) < 0) {
		put_task_struct(task);
		return err;
	}

	if (is_rtg)
		task->rtg_depth = STATIC_RTG_DEPTH;
	else
		task->rtg_depth = 0;

	put_task_struct(task);
	return 0;
}


void set_frame_rtg_thread(struct task_struct *task, bool is_rtg)
{
	if (!task)
		return;

	if (set_rtg_sched(task, is_rtg, DEFAULT_RT_FRAME_ID, DEFAULT_RT_PRIO) < 0)
		return;

	if (is_rtg)
		task->rtg_depth = STATIC_RTG_DEPTH;
	else
		task->rtg_depth = 0;
}
