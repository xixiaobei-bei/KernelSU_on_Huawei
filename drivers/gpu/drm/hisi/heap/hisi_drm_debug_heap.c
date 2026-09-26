/*
 * hisi_drm_debug_heap.c
 *
 * Debug heap, can used for debug in reserved memory.
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#include "hisi_drm_debug_heap.h"

#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <asm/memory.h>
#include <linux/scatterlist.h>

#include "hisi_drm_gem.h"
#include "hisi_drm_heap_ops_helper.h"
#include "hisi_drm_heaps_manager.h"
#include "hisi_drm_heaps_defs.h"

static struct hisi_drm_debug_pool g_mem_pool;
static struct hisi_drm_debug_heap *reserved_heap;

static phys_addr_t find_and_get_paddr(unsigned long size)
{
	return gen_pool_alloc(reserved_heap->pool, size);
}

static void free_paddr(phys_addr_t paddr, unsigned long size)
{
	gen_pool_free(reserved_heap->pool, paddr, size);
}

static int hisi_drm_debug_heap_alloc(struct hisi_drm_heap *heap,
				     struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_gem *hisi_gem = NULL;

	struct sg_table *table = NULL;

	unsigned long size;

	phys_addr_t paddr = 0;
	int ret = 0;

	hisi_gem = ctx->priv;
	if (!hisi_gem) {
		pr_heaps_err("invalid hisi_gem args\n");
		return -EINVAL;
	}

	size = PAGE_ALIGN(hisi_gem->size);
	if (size > g_mem_pool.pool_size) {
		pr_heaps_err("request size %#lx, above allowed max size %#x\n",
			     size, g_mem_pool.pool_size);
		return -EINVAL;
	}

	paddr = find_and_get_paddr(size);
	if (!paddr) {
		pr_heaps_err("failed to alloc pages from pool\n");
		return -ENOMEM;
	}

	table = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!table) {
		ret = -ENOMEM;
		goto free_addr;
	}

	ret = sg_alloc_table(table, 1, GFP_KERNEL);
	if (ret) {
		pr_heaps_err("[%s] failed to alloc sg table\n", __func__);
		ret = -ENOMEM;
		goto free_sgt;
	}

	sg_set_page(table->sgl, pfn_to_page(PFN_DOWN(paddr)), size, 0);
	pr_heaps_debug("alloc from debug success, request size=%#lx\n", size);

	hisi_gem->sgt = table;

	return ret;
free_sgt:
	kfree(table);
free_addr:
	free_paddr(paddr, hisi_gem->size);
	return ret;
}

static void hisi_drm_debug_heap_free(struct hisi_drm_heap *heap,
				     struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_gem *hisi_gem = NULL;
	unsigned long size;
	phys_addr_t paddr;
	struct page *tmp_page = NULL;

	if (!ctx || !ctx->priv) {
		pr_heaps_err("invalid input args, can't not find hisi_gem\n");
		return;
	}

	hisi_gem = ctx->priv;
	size = hisi_gem->size;

	if (!hisi_gem->sgt)
		return;

	tmp_page = sg_page(hisi_gem->sgt->sgl);
	if (!tmp_page) {
		pr_heaps_err("invalid args, can not get page\n");
		return;
	}
	paddr = page_to_phys(tmp_page);
	pr_heaps_debug("free size=%#lx\n", size);
	gen_pool_free(reserved_heap->pool, paddr, size);

	sg_free_table(hisi_gem->sgt);
	kfree(hisi_gem->sgt);
}

static int hisi_drm_debug_pool_setup(struct reserved_mem *rmem)
{
	g_mem_pool.phys_base = rmem->base;
	g_mem_pool.pool_size = rmem->size;

	return 0;
}

int hisi_drm_debug_heap_pool_init(struct hisi_drm_debug_heap *heap,
				  int order_per_bit)
{
	struct gen_pool *pool = NULL;
	int ret;

	pool = gen_pool_create(order_per_bit, 0);
	if (!pool) {
		pr_heaps_err("failed to alloc gen pool\n");
		return -ENOMEM;
	}

	ret = gen_pool_add(pool, g_mem_pool.phys_base, g_mem_pool.pool_size, 0);
	if (ret) {
		pr_heaps_err("failed to add reserved mem into gen pool\n");
		return ret;
	}

	heap->pool = pool;
	pr_heaps_debug("init reserved pool complete\n");

	return 0;
}

static struct hisi_drm_heap_ops hisi_drm_debug_heap_ops = {
	.alloc_buf = hisi_drm_debug_heap_alloc,
	.free_buf = hisi_drm_debug_heap_free,
	.map_kernel = hisi_drm_map_kernel_helper,
	.unmap_kernel = hisi_drm_unmap_kernel_helper,
	.map_user = hisi_drm_map_user_helper,
};

struct hisi_drm_heap *
hisi_drm_debug_heap_create(struct hisi_drm_platform_heap *heap_data)
{
	struct hisi_drm_debug_heap *heap = NULL;
	int ret;

	pr_heaps_info("begin to init drm reserved memory\n");

	if (!heap_data)
		return ERR_PTR(-EINVAL);

	if (g_mem_pool.pool_size == 0) {
		pr_heaps_err(
			"can't create debug pool, due to no reserved memory\n");
		return ERR_PTR(-ENOENT);
	}

	heap = kzalloc(sizeof(*heap), GFP_KERNEL);
	if (!heap)
		return ERR_PTR(-ENOMEM);

	heap->heap.ops = &hisi_drm_debug_heap_ops;

	ret = hisi_drm_debug_heap_pool_init(heap, PAGE_SHIFT);
	if (ret) {
		kfree(heap);
		return ERR_PTR(ret);
	}

	reserved_heap = heap;

	pr_heaps_info("init drm reserved memory success\n");

	return &heap->heap;
}

RESERVEDMEM_OF_DECLARE(mem_pool, "hisi-drm-debug-pool",
		       hisi_drm_debug_pool_setup);
