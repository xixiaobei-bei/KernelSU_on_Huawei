/*
 * static_ion_mem.h
 *
 * memory init, register for mailbox pool.
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
#ifndef STATIC_ION_MEM_H
#define STATIC_ION_MEM_H
#include <linux/types.h>

#define ION_MEM_MAX_SIZE 10
struct register_ion_mem_tag {
	uint32_t size;
	uint64_t memaddr[ION_MEM_MAX_SIZE];
	uint32_t memsize[ION_MEM_MAX_SIZE];
	uint32_t memtag[ION_MEM_MAX_SIZE];
};

enum static_mem_tag {
	MEM_TAG_MIN = 0,
	PP_MEM_TAG = 1,
	PRI_PP_MEM_TAG = 2,
	PT_MEM_TAG = 3,
	MEM_TAG_MAX,
};

#ifdef CONFIG_STATIC_ION

int tc_ns_register_ion_mem(void);

#else

static inline int tc_ns_register_ion_mem(void)
{
	return 0;
}

#endif

#if defined(CONFIG_STATIC_ION) && defined(DEF_ENG)

void secos_addr_test(bool read, uint32_t offset, uint32_t value);

#else

static inline void secos_addr_test(bool read, uint32_t offset, uint32_t value)
{
	(void)read;
	(void)offset;
	(void)value;
}

#endif

#endif
