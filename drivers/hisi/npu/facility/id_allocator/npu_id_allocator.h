/*
 * npu_id_allocator.h
 *
 * about npu id allocator
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_ID_ALLOCATOR_H
#define __NPU_ID_ALLOCATOR_H

#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include "npu_common.h"

int npu_id_allocator_create(struct npu_id_allocator *id_allocator,
	u32 start_id, u32 id_num, u32 data_size);

int npu_id_allocator_destroy(struct npu_id_allocator *id_allocator);

struct npu_id_entity *npu_id_allocator_alloc(
	struct npu_id_allocator *id_allocator);

int npu_id_allocator_free(struct npu_id_allocator *id_allocator, u32 index);

#endif
