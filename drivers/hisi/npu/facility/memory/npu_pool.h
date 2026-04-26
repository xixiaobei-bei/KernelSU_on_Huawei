/*
 * npu_pool.h
 *
 * about npu pool
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
#ifndef _NPU_POOL_H
#define _NPU_POOL_H

#include "npu_common.h"

typedef enum {
	NPU_MEM_POOL_TYPE_SQ = 0,
	NPU_MEM_POOL_TYPE_CQ = 1,
	NPU_MEM_POOL_TYPE_HWTS_SQ = 2,
	NPU_MEM_POOL_TYPE_LONG_HWTS_SQ = 3,
	NPU_MEM_POOL_TYPE_HWTS_CQ = 4,
	NPU_MEM_POOL_TYPE_MAX
} tag_npu_mem_pool_type;

typedef struct npu_entity_info {
	struct list_head list;
	u32 index;
	vir_addr_t vir_addr;
	unsigned long iova;
} npu_entity_info_t;

typedef int (*alloc_pool_func)(u32 len, vir_addr_t *virt_addr,
	unsigned long *iova);

typedef void (*free_pool_func)(vir_addr_t virt_addr, unsigned long iova);

int npu_pool_regist(u8 dev_id, tag_npu_mem_pool_type type, u32 start_id,
	u32 entity_num, u32 entity_size,
	u8 pool_num, alloc_pool_func alloc_pool, free_pool_func free_pool);

int npu_pool_unregist(u8 dev_id, tag_npu_mem_pool_type type);

struct npu_entity_info *npu_alloc_entity(u8 dev_id,
	tag_npu_mem_pool_type type);

int npu_free_entity(u8 dev_id, tag_npu_mem_pool_type type, u32 index);

#endif
