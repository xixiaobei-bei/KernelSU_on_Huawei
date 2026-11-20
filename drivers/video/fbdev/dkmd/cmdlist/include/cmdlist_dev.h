/**
 * @file cmdlist_dev.h
 * @brief Interface for cmdlist device function
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __CMDLIST_DEV_H__
#define __CMDLIST_DEV_H__

#include <linux/hisi-iommu.h>
#include <linux/slab.h>
#include <linux/genalloc.h>

int cmdlist_dev_mmap(struct vm_area_struct *vma);
void *cmdlist_mem_alloc(uint32_t size, dma_addr_t *phy_addr, uint32_t *out_buffer_size);
void cmdlist_mem_free(void *vir_addr, uint32_t size);

#endif