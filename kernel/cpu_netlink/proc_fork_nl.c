/*
 * proc_fork_nl.c
 *
 * iaware fork connector implementation
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#include <cpu_netlink/cpu_netlink.h>
#define MAX_FORK_TIME 30

#ifdef CONFIG_HW_RTG
#include <linux/sched/hw_rtg/proc_state.h>
#elif (defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_MTK_RTG_SCHED))
#include "hwrtg/proc_state.h"
#endif

static void send_vip_msg(int pid, int tgid)
{
	int dt[SEND_DATA_LEN];

	dt[0] = pid;
	dt[1] = tgid;
	send_to_user(PROC_FORK, SEND_DATA_LEN, dt);
}

#if (defined(CONFIG_HW_VIP_THREAD) && !defined(CONFIG_DISABLE_VIP_FORK_MSG))
static int check_vip_status(int cur_pid, int cur_tgid, struct task_struct *task)
{
	pid_t pid = cur_pid;
	pid_t tgid = cur_tgid;
	struct task_struct *parent = NULL;
	struct task_struct *temp_task = task;
	struct task_struct *tg_task = NULL;
	int time = 0;
	int vip = 0;

	if (!task)
		return vip;

	rcu_read_lock();
	do {
		if (pid != tgid) {
			tg_task = find_task_by_vpid(tgid);
			if (!tg_task)
				break;
			vip = tg_task->static_vip;
			if (vip == 1)
				break;
		}

		parent = rcu_dereference(
			temp_task->real_parent); /*lint !e1058 !e64*/
		if (!parent)
			break;
		vip = parent->static_vip;
		pid = parent->pid;
		tgid = parent->tgid;

		if (vip == 1)
			break;
		temp_task = parent;
	} while (pid > 1 && time++ < MAX_FORK_TIME);
	rcu_read_unlock();

	return vip;
}
#else
static int check_vip_status(int cur_pid, int cur_tgid, struct task_struct *task)
{
	return 0;
}
#endif

void iaware_proc_fork_connector(struct task_struct *task)
{
	pid_t cur_pid;
	pid_t cur_tgid;

	if (!task)
		return;

	cur_pid = task->pid;
	cur_tgid = task->tgid;

#if defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_RTG) || defined(CONFIG_HW_MTK_RTG_SCHED)
	get_task_struct(task);
	if (is_key_aux_comm(task, task->comm))
		send_thread_comm_msg(PROC_AUX_COMM_FORK, cur_pid, cur_tgid);
	put_task_struct(task);
#endif

	if (check_vip_status(cur_pid, cur_tgid, task) == 1)
		send_vip_msg(cur_pid, cur_tgid);
}
