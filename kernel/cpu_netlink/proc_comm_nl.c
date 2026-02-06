/*
 * proc_comm_nl.c
 *
 * iaware comm connector implementation
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
#define BINDER_THREAD_NAME "Binder:"

#ifdef CONFIG_HW_RTG
#include <linux/sched/hw_rtg/proc_state.h>
#elif (defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_MTK_RTG_SCHED))
#include "hwrtg/proc_state.h"
#endif

#if defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_RTG) || defined(CONFIG_HW_MTK_RTG_SCHED)
#define MALI_THREAD_NAME "mali-cmar-backe"
#endif

#define IAWARE_COMM_MSG_LEN 2

void send_thread_comm_msg(int num, int pid, int tgid)
{
	int dt[IAWARE_COMM_MSG_LEN];

	dt[0] = pid;
	dt[1] = tgid;
	send_to_user(num, IAWARE_COMM_MSG_LEN, dt);
}

int iaware_proc_comm_connector(struct task_struct *task, const char *comm)
{
	int sock_num = 0;

	if (!task || !comm)
		return sock_num;

	if (strstr(comm, BINDER_THREAD_NAME))
		sock_num = PROC_COMM;
#if defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_RTG) || defined(CONFIG_HW_MTK_RTG_SCHED)
	else if (strstr(comm, MALI_THREAD_NAME))
		sock_num = PROC_MALI_COMM;
	else if (is_key_aux_comm(task, comm))
		sock_num = PROC_AUX_COMM;
	else if (is_key_aux_comm(task, task->comm))
		sock_num = PROC_AUX_COMM_REMOVE;
#endif

	return sock_num;
}

void iaware_send_comm_msg(struct task_struct *task, int sock_num)
{
	int pid;
	int tgid;

	if (!task || !sock_num)
		return;

	get_task_struct(task);
	pid = task->pid;
	tgid = task->tgid;
	put_task_struct(task);
	send_thread_comm_msg(sock_num, pid, tgid);
}
