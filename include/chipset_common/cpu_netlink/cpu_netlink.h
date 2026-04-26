/*
 * cpu_netlink.h
 *
 * iaware cpu netlink header
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

#ifndef CPU_NETLINK_H
#define CPU_NETLINK_H

#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/version.h>
#include <linux/module.h>

#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
#include <linux/sched/task.h>
#endif

#define SEND_DATA_LEN 2

enum sock_no {
	CPU_HIGH_LOAD = 1,
	PROC_FORK = 2,
	PROC_COMM = 3,
	PROC_AUX_COMM = 4,
	PROC_LOAD = 5,
	PROC_AUX_COMM_FORK = 6,
	PROC_AUX_COMM_REMOVE = 7,
	PROC_CPUMASK_BIG = 8,
	PROC_MALI_COMM = 9
};

int send_to_user(int sock_no, size_t len, const int *data);
void iaware_proc_fork_connector(struct task_struct *task);
int iaware_proc_comm_connector(struct task_struct *task, const char *comm);
void iaware_send_comm_msg(struct task_struct *task, int sock_num);
void send_thread_comm_msg(int num, int pid, int tgid);
#endif
