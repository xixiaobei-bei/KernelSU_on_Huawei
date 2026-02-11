/* Copyright (c) Hisilicon Technologies Co., Ltd. 2001-2019. All rights reserved.
 * FileName: ion_sec_contig.h
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _ION_SEC_CONTIG_H
#define _ION_SEC_CONTIG_H

#include <linux/err.h>
#include <linux/version.h>

#include "ion.h"
#include "hisi_ion_priv.h"

enum SECCG_HEAP_TYPE {
	HEAP_NORMAL = 0,
	HEAP_SECURE = 1,
	HEAP_SECURE_TEE = 2,
	HEAP_MAX,
};

enum SECCG_CMA_TYPE {
	CMA_STATIC = 0,
	CMA_DYNAMIC = 1,
	CMA_TYPE_MAX,
};

#define ION_PBL_SHIFT 12
#define ION_NORMAL_SHIFT 16

#define SEC_CG_CMA_NUM 4

struct ion_seccg_heap {
	struct ion_heap heap;
	struct cma *cma;
	struct gen_pool *pool;
	/* heap mutex */
	struct mutex mutex;
	/* heap attr: secure, normal */
	u32 heap_attr;
	u32 pool_shift;
	/* heap total size*/
	size_t heap_size;
	/* heap allocated size*/
	unsigned long alloc_size;
	/* heap alloc cma size */
	unsigned long cma_alloc_size;
	/* worker to asynchronously do cma alloc fill pool */
	struct work_struct pre_alloc_work;
	/* pre alloc attr */
	u32 pre_alloc_attr;
	/* pre alloc mutex */
	struct mutex pre_alloc_mutex;
	/* heap flag */
	u64 flag;
	u64 per_alloc_sz;
	/* align size */
	u64 per_bit_sz;
	u64 water_mark;
	struct list_head allocate_head;
	TEEC_Context *context;
	TEEC_Session *session;
	int TA_init;
	u32 cma_type;
	const char *cma_name;
	struct gen_pool *static_cma_region;
};

struct ion_sec_cma {
	struct cma *cma_region;
	const char *cma_name;
};

struct alloc_list {
	u64 addr;
	u32 size;
	struct list_head list;
};

int __seccg_alloc_contig(struct ion_seccg_heap *seccg_heap,
			 struct ion_buffer *buffer, unsigned long size);
void __seccg_free_contig(struct ion_seccg_heap *seccg_heap,
			 struct ion_buffer *buffer);
int __seccg_fill_watermark(struct ion_seccg_heap *seccg_heap);

#endif
