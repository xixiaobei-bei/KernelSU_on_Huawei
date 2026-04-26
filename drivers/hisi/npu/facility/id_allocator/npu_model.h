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
#include "npu_id_allocator.h"

struct npu_id_entity *npu_alloc_model(u8 dev_ctx_id);

int npu_free_model_id(u8 dev_ctx_id, int model_id);

#endif
