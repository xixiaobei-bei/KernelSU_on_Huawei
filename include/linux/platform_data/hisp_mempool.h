/*
 * ISP driver, isp_mempool.h
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#ifndef _PLAT_MEMPOOL_HISP_H
#define _PLAT_MEMPOOL_HISP_H

#include <linux/scatterlist.h>
#include <linux/printk.h>
#include <linux/rproc_share.h>
#include <linux/genalloc.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/platform_data/hisp.h>
#include <global_ddr_map.h>
#include <isp_ddr_map.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct mem_pool_info_list_s {
	unsigned int addr;
	unsigned int size;
	unsigned int iova; /* raw iova addr */
	unsigned int iova_size; /* raw buffer size */
	struct list_head list;
};

struct mem_pool_info_s {
	unsigned int addr;
	unsigned int size;
	unsigned int prot;
	unsigned int enable;
	unsigned int dynamic_mem_flag;
	unsigned int iova_start;
	unsigned int iova_end;
	struct mem_pool_info_list_s *node;
	struct gen_pool *isp_mem_pool;
};

struct isp_mem_pool_s {
	unsigned int count;
	struct mutex mem_pool_mutex;
	struct mem_pool_info_s isp_pool_info[ISP_MEM_POOL_NUM];
	struct mem_pool_info_s iova_pool_info;
	struct mutex hisp_map_mutex;
};

enum maptype {
	MAP_TYPE_DYNAMIC    = 0,
	MAP_TYPE_RAW2YUV,
	MAP_TYPE_STATIC,
	MAP_TYPE_STATIC_SEC,
	MAP_TYPE_DYNAMIC_CARVEOUT,
	MAP_TYPE_STATIC_ISP_SEC,
	MAP_TYPE_DYNAMIC_SEC,
	MAP_TYPE_COREDUMP   = 10,
	MAP_TYPE_MAX,
};

/* MDC reserved memory */
unsigned int hisp_mem_pool_alloc_carveout(size_t size, unsigned int type);
int hisp_mem_pool_free_carveout(unsigned int  iova, size_t size);

/* hisp map addr for ispcpu */
unsigned int hisp_alloc_cpu_map_addr(struct scatterlist *sgl,
		unsigned int prot, unsigned int size, unsigned int align);
int hisp_free_cpu_map_addr(unsigned int iova, unsigned int size);

/* hisp mempool api */
unsigned long hisp_mem_pool_alloc_iova(unsigned int size,
			unsigned int pool_num);
int hisp_mem_pool_free_iova(unsigned int pool_num,
				unsigned int va, unsigned int size);
unsigned int hisp_mem_map_setup(struct scatterlist *sgl,
				unsigned int iova, unsigned int size,
				unsigned int prot, unsigned int pool_num,
				unsigned int flag, unsigned int align);
void hisp_mem_pool_destroy(unsigned int pool_num);
void hisp_dynamic_mem_pool_clean(void);

/* hisp mempool init api */
int hisp_mem_pool_init(unsigned int addr, unsigned int size);
void hisp_mem_pool_exit(void);

#ifdef __cplusplus
}
#endif

#endif

