/*
 * drv_mem.c
 *
 * This is common data type defination.
 *
 * Copyright (c) 2008-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HI_DRV_MEM_H__
#define __HI_DRV_MEM_H__

#include <linux/seq_file.h>
#include <linux/vmalloc.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/dma-buf.h>
#include <linux/slab.h>

#include "hi_type.h"
#include "drv_venc_ioctl.h"
#include "drv_venc_osal.h"

#define MAX_MEM_NAME_LEN       15
#define MAX_KMALLOC_MEM_NODE   16    /* 1 channel need 2 node ,there is have max 8 channels */
#define MAX_ION_MEM_NODE       200
#define SMMU_RWERRADDR_SIZE    128

typedef struct {
	uint64_t read_addr;
	uint64_t write_addr;
	uint32_t size;
} venc_smmu_err_add_t;

typedef struct {
	void  *start_virt_addr;
	uint64_t   start_phys_addr;
	uint32_t   size;
	uint8_t    is_mapped;
	int32_t   share_fd;
} mem_buffer_t;

typedef struct  {
	char            node_name[MAX_MEM_NAME_LEN];
	char            zone_name[MAX_MEM_NAME_LEN];
	void           *virt_addr;
	uint64_t             phys_addr;
	uint32_t             size;
	int32_t             shared_fd;
	struct ion_handle *handle;
	struct dmabuf    *dmabuf;
} venc_mem_buf_t;

struct mem_info {
	void *virt_addr;
	uint64_t iova_addr;
};

struct mem_pool {
	spinlock_t *lock;
	struct device *dev;
	vedu_osal_event_t event;

	void *start_virt_addr;
	uint64_t start_phy_addr;

	void *aligned_virt_addr;
	uint64_t aligned_iova_addr;

	uint32_t num;
	uint32_t each_size;
	uint32_t align_size;
	DECLARE_KFIFO_PTR(fifo, struct mem_info);
};

int32_t drv_mem_copy_from_user(uint32_t cmd, const void __user *user_arg, void **kernel_arg);
int32_t drv_mem_init(void);
int32_t drv_mem_exit(void);
int32_t drv_mem_kalloc(const char *bufname, const char *zone_name, mem_buffer_t *psmbuf);
int32_t drv_mem_kfree(const mem_buffer_t *psmbuf);
int32_t drv_mem_map_kernel(int32_t shared_fd, mem_buffer_t *psmbuf);
int32_t drv_mem_unmap_kernel(mem_buffer_t *psmbuf);
int32_t drv_mem_get_map_info(int32_t shared_fd, mem_buffer_t *psmbuf);
int32_t drv_mem_put_map_info(mem_buffer_t *psmbuf);
int32_t drv_mem_iommumap(venc_buffer_record_t *node, struct platform_device *pdev);
int32_t drv_mem_iommuunmap(int32_t shared_fd, int32_t phys_addr, struct platform_device *pdev);

struct mem_pool *drv_mem_create_pool(struct device *dev, uint32_t each_size, uint32_t num, uint32_t align_size);
void drv_mem_destory_pool(struct mem_pool *pool);
void *drv_mem_pool_alloc(struct mem_pool *pool, uint64_t *iova_addr);
void drv_mem_pool_free(struct mem_pool *pool, void *virt_addr, uint64_t iova_addr);

#endif

