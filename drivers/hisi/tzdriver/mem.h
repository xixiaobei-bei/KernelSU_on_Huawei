/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Funciton Declaration: Memory init, register for mailbox pool.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */


#ifndef _MEM_H_
#define _MEM_H_
#include <linux/types.h>
#include "teek_ns_client.h"

#define PRE_ALLOCATE_SIZE (1024*1024)
#define MEM_POOL_ELEMENT_SIZE (64*1024)
#define MEM_POOL_ELEMENT_NR (8)
#define MEM_POOL_ELEMENT_ORDER (4)

int tc_mem_init(void);
void tc_mem_destroy(void);

tc_ns_shared_mem *tc_mem_allocate(size_t len, bool from_mailbox);
void tc_mem_free(tc_ns_shared_mem *shared_mem);

static inline void get_sharemem_struct(struct tag_tc_ns_shared_mem *sharemem)
{
	if (sharemem != NULL)
		atomic_inc(&sharemem->usage);
}

static inline void put_sharemem_struct(struct tag_tc_ns_shared_mem *sharemem)
{
	if (sharemem != NULL) {
		if (atomic_dec_and_test(&sharemem->usage))
			tc_mem_free(sharemem);
	}
}

int tc_ns_register_ion_mem(void);

enum static_mem_tag {
	MEM_TAG_MIN = 0,
	PP_MEM_TAG = 1,
	PRI_PP_MEM_TAG = 2,
	PT_MEM_TAG = 3,
	MEM_TAG_MAX,
};
#endif
