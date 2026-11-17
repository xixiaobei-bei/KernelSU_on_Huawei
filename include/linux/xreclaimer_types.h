/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: xreclaimer header file
 * Author: Miao Xie <miaoxie@huawei.com>
 * Create: 2020-07-01
 */
#ifndef _LINUX_XRECLAIMER_TYPES_H
#define _LINUX_XRECLAIMER_TYPES_H

struct xreclaimer_mm {
	/* Reuse page_table_lock to protect xrmm_ntasks. */
	int			xrmm_ntasks;
	unsigned long		xrmm_flags;
	struct task_struct	*xrmm_owner;
	struct mm_struct	*xrmm_next_mm;
};
#endif
