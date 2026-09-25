/*
 * npu_model.h
 *
 * about npu model
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
