/*
 * npu_task.h
 *
 * about npu task
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_TASK_H
#define __NPU_TASK_H

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/list.h>

struct devdrv_task_info {
	u32 id;
	u32 devid;
	struct list_head list;
};

int devdrv_task_list_init(u8 dev_ctx_id);

struct devdrv_task_info *devdrv_alloc_task(u8 dev_ctx_id);

int devdrv_free_task_id(u8 dev_ctx_id, u32 model_id);

int devdrv_task_list_destroy(u8 dev_ctx_id);

#endif
