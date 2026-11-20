/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef DKMD_DPU_DVFS_H
#define DKMD_DPU_DVFS_H

#include <linux/types.h>
#include <linux/semaphore.h>

struct dpu_dvfs {
	struct semaphore sem;

};

void dpu_rm_register_dvfs(struct list_head *res_head);

#endif