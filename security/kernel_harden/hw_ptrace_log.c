/*
 * hw_ptrace_log.c
 *
 * Huawei Kernel Harden, ptrace log upload
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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
#include <linux/capability.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/ptrace.h>
#include <linux/security.h>
#include <linux/signal.h>
#include <linux/uio.h>
#include <linux/audit.h>
#include <linux/pid_namespace.h>
#include <chipset_common/security/saudit.h>

void record_ptrace_info_before_return(long request, struct task_struct *child)
{
	struct task_struct *tracer = NULL;
	static unsigned int ptrace_log_counter = 1;
	char comm_child[TASK_COMM_LEN] = {0};
	char comm_tracer[TASK_COMM_LEN] = {0};

	if (!child)
		return;

	/* only 100 log upload since power on */
	if (ptrace_log_counter > 100)
		return;
	ptrace_log_counter++;

	(void)get_task_comm(comm_child, child);
	rcu_read_lock();
	tracer = ptrace_parent(child);
	if (tracer)
		(void)get_task_comm(comm_tracer, tracer);
	else
		(void)strncpy(comm_tracer, "unknown", sizeof("unknown"));

	rcu_read_unlock();

	saudit_log(PTRACE, STP_RISK, 0, "child=%s,tracer=%s,", comm_child, comm_tracer);

}
