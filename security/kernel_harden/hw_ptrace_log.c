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
#include <chipset_common/security/hw_kernel_stp_interface.h>

void record_ptrace_info_before_return(long request, struct task_struct *child)
{
	struct task_struct *tracer = NULL;
	struct stp_item item;
	int ret;
	static unsigned int g_ptrace_log_counter = 1;
	char comm_child[TASK_COMM_LEN] = {0};
	char comm_tracer[TASK_COMM_LEN] = {0};
	char add_info[TASK_COMM_LEN * 2 + 1] = {0};

	if (!child)
		return;

	/* only 100 log upload since power on */
	if (g_ptrace_log_counter > 100)
		return;
	g_ptrace_log_counter++;

	(void)get_task_comm(comm_child, child);
	rcu_read_lock();
	tracer = ptrace_parent(child);
	if (tracer)
		(void)get_task_comm(comm_tracer, tracer);
	else
		(void)strncpy(comm_tracer, "unknown", sizeof("unknown"));

	rcu_read_unlock();
	(void)memset(&item, 0, sizeof(item));
	item.id = item_info[PTRACE].id;
	item.status = STP_RISK;
	item.credible = STP_CREDIBLE;
	item.version = 0;
	(void)strncpy(item.name, item_info[PTRACE].name, STP_ITEM_NAME_LEN - 1);
	item.name[STP_ITEM_NAME_LEN - 1] = '\0';
	(void)snprintf(add_info, TASK_COMM_LEN * 2 + 1, "%s%s",
		       comm_child, comm_tracer);
	ret = kernel_stp_upload(item, add_info);
	if (ret != 0)
		pr_err("fail child=%s, tracer=%s\n", comm_child, comm_tracer);
	else
		pr_err("succ child=%s, tracer=%s\n", comm_child, comm_tracer);
}
