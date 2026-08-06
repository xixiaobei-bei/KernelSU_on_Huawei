/**
 * @file cmdlist_mem.c
 * @brief Cmdlist memory management interface
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

#include "cmdlist_drv.h"
#include "cmdlist_dev.h"

int cmdlist_dev_mmap(struct vm_area_struct *vma)
{
	int ret = 0;
	unsigned long size;
	unsigned long addr;
	struct cmdlist_private *priv = &g_cmdlist_priv;

	addr = vma->vm_start;
	size = vma->vm_end - vma->vm_start;
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	pr_info("mmap --> phy:%#x, addr:%#x, size:%#x\n", priv->pool_phy_addr, addr, size);

	if (size > priv->sum_pool_size)
		return -EINVAL;

	ret = remap_pfn_range(vma, addr, (priv->pool_phy_addr >> PAGE_SHIFT) + vma->vm_pgoff, size, vma->vm_page_prot);
	if (ret != 0) {
		pr_err("failed to remap_pfn_range! ret=%d\n", ret);
		return ret;
	}

	return ret;
}

void *cmdlist_mem_alloc(uint32_t size, dma_addr_t *phy_addr, uint32_t *out_buffer_size)
{
	void *vir_addr = NULL;
	uint32_t alloc_size;
	struct cmdlist_private *priv = &g_cmdlist_priv;

	if (!phy_addr || !out_buffer_size) {
		pr_err("check input param error!\n");
		return NULL;
	}
	*out_buffer_size = 0;

	if (IS_ERR_OR_NULL(priv->memory_pool)) {
		pr_err("check memory_pool failed!\n");
		return NULL;
	}

	alloc_size = roundup(size, PAGE_SIZE);
	vir_addr = (void *)(uintptr_t)gen_pool_dma_alloc(priv->memory_pool, alloc_size, phy_addr);
	if (!vir_addr) {
		pr_err("gen pool alloc failed!\n");
		return NULL;
	}
	memset(vir_addr, 0, alloc_size);

	*out_buffer_size = alloc_size;

	pr_info("mem_alloc --> vir_addr:%#x, size:%#x\n", vir_addr, alloc_size);

	return vir_addr;
}

void cmdlist_mem_free(void *vir_addr, uint32_t size)
{
	struct cmdlist_private *priv = &g_cmdlist_priv;

	if (IS_ERR_OR_NULL(priv->memory_pool)) {
		pr_err("check memory_pool failed!\n");
		return;
	}

	gen_pool_free(priv->memory_pool, (unsigned long)(uintptr_t)vir_addr, size);

	pr_info("mem_free --> vir_addr:%#x, size:%#x\n", vir_addr, size);

	vir_addr = NULL;
}



