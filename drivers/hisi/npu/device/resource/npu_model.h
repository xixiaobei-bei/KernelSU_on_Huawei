/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#ifndef __NPU_MODEL_H
#define __NPU_MODEL_H

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/list.h>

struct devdrv_model_info {
	u32 id;
	u32 devid;
	struct list_head list;
};

int devdrv_model_list_init(u8 dev_ctx_id);

struct devdrv_model_info *devdrv_alloc_model(u8 dev_ctx_id);

int devdrv_free_model_id(u8 dev_ctx_id, int model_id);

int devdrv_model_list_destroy(u8 dev_ctx_id);

int devdrv_model_software_ops_register(u8 dev_ctx_id);

#endif
