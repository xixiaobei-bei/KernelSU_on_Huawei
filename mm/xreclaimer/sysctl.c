/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: sysctl interface implementation for xreclaimer
 * Author: Miao Xie <miaoxie@huawei.com>
 * Create: 2020-06-29
 */
#include <linux/sysctl.h>
#include <linux/xreclaimer.h>
#include <linux/init.h>

static struct ctl_table_header *xreclaimer_sysctl_header;

/* Reuse old file name - boost_sigkill_free */
static struct ctl_table kern_table[] = {
	{
		.procname	= "boost_sigkill_free",
		.data		= &sysctl_xreclaimer_enable,
		.maxlen		= sizeof(sysctl_xreclaimer_enable),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{ },
};

static struct ctl_table sys_table[] = {
	{
		.procname	= "kernel",
		.mode		= 0555,
		.child		= kern_table,
	},
	{ },
};

int __init xreclaimer_init_sysctl(void)
{
	xreclaimer_sysctl_header = register_sysctl_table(sys_table);
	if (!xreclaimer_sysctl_header) {
		pr_err("Unable to register xreclaimer sysctl files.\n");
		return -ENOMEM;
	}

	return 0;
}
