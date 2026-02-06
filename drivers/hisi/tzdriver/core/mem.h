/*
 * mem.h
 *
 * memory operation for gp sharedmem.
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
 */
#ifndef MEM_H
#define MEM_H
#include <linux/types.h>
#include "teek_ns_client.h"

#define PRE_ALLOCATE_SIZE (1024*1024)
#define MEM_POOL_ELEMENT_SIZE (64*1024)
#define MEM_POOL_ELEMENT_NR 8
#define MEM_POOL_ELEMENT_ORDER 4

struct tc_ns_shared_mem *tc_mem_allocate(size_t len);
void tc_mem_free(struct tc_ns_shared_mem *shared_mem);

static inline void get_sharemem_struct(struct tc_ns_shared_mem *sharemem)
{
	if (sharemem != NULL)
		atomic_inc(&sharemem->usage);
}

static inline void put_sharemem_struct(struct tc_ns_shared_mem *sharemem)
{
	if (sharemem != NULL) {
		if (atomic_dec_and_test(&sharemem->usage))
			tc_mem_free(sharemem);
	}
}

#endif
