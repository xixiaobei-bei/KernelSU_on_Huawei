/*
 * dynamic_mem.h
 *
 * dynamic Ion memory function declaration.
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
#ifndef DYNAMIC_MMEM_H
#define DYNAMIC_MMEM_H

#ifdef CONFIG_DYNAMIC_ION
#include <linux/version.h>
#include <linux/hisi/hisi_ion.h>
#include <securec.h>
#endif

#include "teek_ns_client.h"

struct sg_memory {
	int dyn_shared_fd;
	struct sg_table *dyn_sg_table;
	struct dma_buf *dyn_dma_buf;
	phys_addr_t ion_phys_addr;
	size_t len;
	void *ion_virt_addr;
};
struct dynamic_mem_item {
	struct list_head head;
	uint32_t configid;
	uint32_t size;
	struct sg_memory memory;
	uint32_t cafd;
	struct tc_uuid uuid;
	uint32_t ddr_sec_region;
};

struct dynamic_mem_config {
	struct tc_uuid uuid;
	uint32_t ddr_sec_region;
};

#define MAX_ION_NENTS      1024
typedef struct ion_page_info {
	phys_addr_t phys_addr;
	uint32_t npages;
} tz_page_info;

typedef struct sglist {
	uint64_t sglist_size;
	uint64_t ion_size;
	uint64_t ion_id;
	uint64_t info_length; /* page_info number */
	struct ion_page_info page_info[0];
} tz_sg_list;

#ifdef CONFIG_DYNAMIC_ION

int init_dynamic_mem(void);
int load_app_use_configid(uint32_t configid, uint32_t cafd,
	const struct tc_uuid *uuid, uint32_t size);
void kill_ion_by_cafd(unsigned int cafd);
void kill_ion_by_uuid(const struct tc_uuid *uuid);

#else

static inline int init_dynamic_mem(void)
{
	return 0;
}

static inline int load_app_use_configid(uint32_t configid, uint32_t cafd,
	const struct tc_uuid *uuid, uint32_t size)
{
	return 0;
}

static inline void kill_ion_by_cafd(unsigned int cafd)
{
	return;
}

static inline void kill_ion_by_uuid(const struct tc_uuid *uuid)
{
	return;
}

#endif

#endif
