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
#include <../kernel/sched/sched.h>
#include <linux/kernel.h>

int set_rtg_sched(struct task_struct *task, bool is_rtg)
{
	int err = -1;
	int policy = SCHED_NORMAL;
	int grpid = DEFAULT_RTG_GRP_ID;
	struct sched_param sp = {0};

	if (task == NULL)
		return err;

	if (is_rtg && task->sched_class != &fair_sched_class)
		return err;

	if (is_rtg) {
		policy = SCHED_FIFO | SCHED_RESET_ON_FORK;
		sp.sched_priority = 1;
		grpid = DEFAULT_RT_FRAME_ID;
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
