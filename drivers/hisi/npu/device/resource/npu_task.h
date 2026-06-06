/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
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
