/*
 * hisi_drm_page_pool_heap.c
 *
 * page pool heap, manager page pool and impl heap's ops
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

#include "hisi_drm_page_pool_heap.h"

#include <linux/mm.h>
#include <linux/hisi-iommu.h>

#include "hisi_drm_heaps_manager.h"
#include "hisi_drm_heap_ops_helper.h"
#include "hisi_drm_page_pool.h"
#include "hisi_drm_heaps_defs.h"

#define DEFAULT_PAGE_POOL_WATER_MARK 838860800

#define MAX_ALLOWED_PAGES (totalram_pages / 2)

static int hisi_drm_page_pool_debug_show(struct hisi_drm_heap *heap,
					 struct seq_file *s, void *unused)
{
	struct hisi_drm_page_pool_heap *page_pool =
		container_of(heap, struct hisi_drm_page_pool_heap, heap);
	unsigned int i;
	struct hisi_drm_page_pool *pool = NULL;

	seq_printf(s, "%16s %16s %16s\n", "order", "low count", "high count");
	for (i = 0; i < NUM_ORDERS; ++i) {
		pool = page_pool->pools[i];
		seq_printf(s, "%16u %#16x %#16x\n", orders[i], pool->low_count,
			   pool->high_count);
	}

	seq_puts(
		s,
		"------------------------------------------------------------------------\n");
	seq_printf(s, "total page pool cached memory size=%#lx\n",
		   PAGE_SIZE * atomic_read(&page_pool->total_pages_nr));

	return 0;
}

static void parse_heap_watermark(struct hisi_drm_page_pool_heap *heap,
				 struct device_node *node)
{
	unsigned int wt_mk;
	unsigned int wt_pages;
	int ret;

	ret = of_property_read_u32(node, "water_mark", &wt_mk);
	if (ret)
		wt_mk = DEFAULT_PAGE_POOL_WATER_MARK;

	wt_pages = PAGE_ALIGN(wt_mk) >> PAGE_SHIFT;
	if (wt_pages < MAX_ALLOWED_PAGES) {
		heap->pool_watermark = wt_mk;
	} else {
		pr_heaps_warning(
			"the water mark %u is too high, max allow is %lu pages",
			wt_mk, MAX_ALLOWED_PAGES);
		heap->pool_watermark = DEFAULT_PAGE_POOL_WATER_MARK;
	}

	pr_heaps_debug("the page pool watermark is %#llx\n",
		       heap->pool_watermark);
}

static int order_to_index(unsigned int order)
{
	unsigned int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (order == orders[i])
			return (int)i;
	}

	return -1;
}

static struct hisi_drm_page_pool *get_pool_by_order(
	struct hisi_drm_page_pool_heap *pool_heap,unsigned int order)
{
	int index = order_to_index(order);

	if (index >= 0)
		return pool_heap->pools[index];

	return NULL;
}

static struct page *alloc_buffer_page(struct hisi_drm_page_pool_heap *pool_heap,
				      unsigned int order)
{
	struct hisi_drm_page_pool *pool = NULL;
	struct page *page = NULL;
	bool is_from_pool = true;

	pool = get_pool_by_order(pool_heap, order);
	if (unlikely(!pool)) {
		pr_heaps_err("failed to find pool by order, order=%u\n", order);
		return NULL;
	}

	page = hisi_drm_page_pool_alloc(pool, &is_from_pool);
	if (!page) {
		pr_heaps_debug("alloc page failed, order=%d\n", pool->order);
		return NULL;
	}

	if (is_from_pool)
		atomic_sub(1 << pool->order, &pool_heap->total_pages_nr);

#ifdef CONFIG_HISI_KERNELDUMP
	if (page)
		SetPageMemDump(page);
#endif

	return page;
}


/**
 * Due to system have per cpu page cache in page order is 0,
 * we don't need to cache this page. So, if page order is 0, just
 * free immediately.
 * If total page caches is more than pool watermark, also, just free
 * immediately.
 * For others, we will cache it into page pool.
 */
static void free_buffer_page(struct hisi_drm_page_pool_heap *pool_heap,
			     struct page *page)
{
	struct hisi_drm_page_pool *pool = NULL;
	unsigned int order = compound_order(page);
	unsigned int nr_pages;

	pool = get_pool_by_order(pool_heap, order);
	if (unlikely(!pool)) {
		pr_heaps_err("failed to find pool by order, order=%u\n", order);
		return;
	}

	if (order == 0) {
		hisi_drm_page_pool_free_immediate(pool, page);
		return;
	}

	nr_pages = atomic_read(&pool_heap->total_pages_nr);
	if (nr_pages * PAGE_SIZE >= pool_heap->pool_watermark) {
		hisi_drm_page_pool_free_immediate(pool, page);
		return;
	}

	hisi_drm_page_pool_free(pool, page);
	atomic_add(1 << pool->order, &pool_heap->total_pages_nr);
}

/**
 * Alloc pages largest available in page pool, we will try get page from
 * max order pool. If not meet or alloc faild, just try request page from
 * lower page pool.
 * @max_order the max allowed request page order from page pool.
 */
static struct page *
alloc_largest_available(struct hisi_drm_page_pool_heap *pool_heap,
			unsigned long size, unsigned int max_order)
{
	struct page *page = NULL;
	unsigned int i = 0;

	for (; i < NUM_ORDERS; ++i) {
		if (size < (PAGE_SIZE << orders[i]))
			continue;
		if (max_order < orders[i])
			continue;

		page = alloc_buffer_page(pool_heap, orders[i]);

		/*
		 * page order is 8, 4 maybe can't get,
		 * so just try lower order item
		 */
		if (!page)
			continue;

		return page;
	}

	return NULL;
}

static struct sg_table *
do_alloc_memory(struct hisi_drm_page_pool_heap *pool_heap,
		unsigned long need_size, unsigned int max_order)
{
	struct page *page = NULL;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *tmp_page = NULL;
	unsigned long total_page_nr;
	struct list_head pages;
	int i = 0;

	total_page_nr = atomic_read(&pool_heap->total_pages_nr);
	pr_heaps_debug("need alloc size=%#lx, now pool size=%#lx\n", need_size,
		       PAGE_SIZE * total_page_nr);
	INIT_LIST_HEAD(&pages);
	while (need_size > 0) {
		page = alloc_largest_available(pool_heap, need_size, max_order);
		if (IS_ERR_OR_NULL(page))
			goto free_pages;
		list_add_tail(&page->lru, &pages);
		need_size -= PAGE_SIZE << compound_order(page);
		max_order = compound_order(page);
		++i;
	}

	total_page_nr = atomic_read(&pool_heap->total_pages_nr);
	pr_heaps_debug("alloc success, now pool size=%#lx\n",
		       PAGE_SIZE * total_page_nr);

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		goto free_pages;

	if (sg_alloc_table(table, i, GFP_KERNEL))
		goto free_table;

	sg = table->sgl;
	list_for_each_entry_safe (page, tmp_page, &pages, lru) {
		sg_set_page(sg, page, PAGE_SIZE << compound_order(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}

	return table;

free_table:
	kfree(table);

free_pages:
	list_for_each_entry_safe (page, tmp_page, &pages, lru)
		free_buffer_page(pool_heap, page);

	return ERR_PTR(-ENOMEM);
}

static int hisi_drm_page_pool_heap_allocate(struct hisi_drm_heap *heap,
					    struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_gem *hisi_gem = NULL;
	struct hisi_drm_page_pool_heap *pool_heap = NULL;
	struct sg_table *table = NULL;
	unsigned long size;
	unsigned long size_remaining;
	unsigned int page_nr;

	if (!ctx || !ctx->priv) {
		pr_heaps_err("invalid input args\n");
		return -EINVAL;
	}

	if (!heap)
		return -EINVAL;

	pool_heap = container_of(heap, struct hisi_drm_page_pool_heap, heap);

	hisi_gem = ctx->priv;
	size = hisi_gem->size;
	size_remaining = PAGE_ALIGN(size);
	page_nr = size_remaining >> PAGE_SHIFT;

	if (page_nr > MAX_ALLOWED_PAGES) {
		pr_heaps_err(
			"the request pages num = %#x above the max allowed\n",
			page_nr);
		return -ENOMEM;
	}

	table = do_alloc_memory(pool_heap, size_remaining, orders[0]);
	if (IS_ERR(table)) {
		pr_heaps_err("failed to alloc memory\n");
		return PTR_ERR(table);
	}

	hisi_gem->sgt = table;

	return 0;
}

/*
 * we will cached pages if total page cache not above the watermark
 */
static void hisi_drm_page_pool_heap_free(struct hisi_drm_heap *heap,
					 struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_page_pool_heap *page_pool_heap = NULL;
	struct hisi_drm_gem *hisi_gem = NULL;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	unsigned int i;

	if (!ctx || !ctx->priv) {
		pr_heaps_err("invalid context arg\n");
		return;
	}

	if (!heap) {
		pr_heaps_err("invalid heap arg\n");
		return;
	}

	page_pool_heap =
		container_of(heap, struct hisi_drm_page_pool_heap, heap);

	hisi_gem = ctx->priv;

	table = hisi_gem->sgt;
	if (!table)
		return;

	for_each_sg (table->sgl, sg, table->nents, i)
		free_buffer_page(page_pool_heap, sg_page(sg));

	pr_heaps_debug("total free %#lx, now pool total pages=%#x\n",
		       hisi_gem->size,
		       atomic_read(&page_pool_heap->total_pages_nr));

	sg_free_table(table);
	kfree(table);
}

void hisi_drm_destroy_pools(struct hisi_drm_page_pool_heap *pool_heap)
{
	unsigned int i = 0;
	struct hisi_drm_page_pool *pool = NULL;

	for (; i < NUM_ORDERS; ++i) {
		pool = pool_heap->pools[i];
		if (pool)
			hisi_drm_page_pool_destroy(pool);
	}
}

static int hisi_drm_create_pools(struct hisi_drm_page_pool_heap *page_pool_heap,
				 struct hisi_drm_page_pool **pools)
{
	unsigned int i;
	struct hisi_drm_page_pool *pool = NULL;

	for (i = 0; i < NUM_ORDERS; ++i) { /*lint !e574*/

		gfp_t gfp_flags = low_order_gfp_flags;

		if (orders[i] >= MID_ORDER)
			gfp_flags = high_order_gfp_flags;

		if (orders[i] == HIGH_ORDER)
			gfp_flags = high_order_gfp_flags & ~__GFP_RECLAIMABLE;

		pool = hisi_drm_page_pool_create(gfp_flags, orders[i]);
		if (!pool)
			goto err_create_pool;
		pools[i] = pool;
	}
	return 0;

err_create_pool:
	hisi_drm_destroy_pools(page_pool_heap);
	return -ENOMEM;
}

static int hisi_drm_page_pool_heap_shrink(struct hisi_drm_heap *heap,
					  gfp_t gfp_mask, int nr_to_scan)
{
	struct hisi_drm_page_pool *pool = NULL;
	struct hisi_drm_page_pool_heap *page_pool_heap = NULL;
	int nr_total = 0;
	unsigned i;
	int nr_freed;
	int only_scan = 0;

	if (!heap)
		return -EINVAL;

	page_pool_heap =
		container_of(heap, struct hisi_drm_page_pool_heap, heap);

	if (!nr_to_scan)
		only_scan = 1;

	for (i = 0; i < NUM_ORDERS; i++) {
		pool = page_pool_heap->pools[i];

		if (only_scan) {
			nr_total += hisi_drm_page_pool_shrink(pool, gfp_mask,
							      nr_to_scan);
		} else {
			nr_freed = hisi_drm_page_pool_shrink(pool, gfp_mask,
							     nr_to_scan);
			atomic_sub(nr_freed, &page_pool_heap->total_pages_nr);

			pr_heaps_debug(
				"remove %#x nr_pages from pool, now total nr_pages=%#x\n",
				nr_freed,
				atomic_read(&page_pool_heap->total_pages_nr));

			nr_to_scan -= nr_freed;
			nr_total += nr_freed;
			if (nr_to_scan <= 0)
				break;
		}
	}

	return nr_total;
}

static void hisi_drm_page_pool_set_water_mark(struct hisi_drm_heap *heap,
					      u64 wt_mark)
{
	struct hisi_drm_page_pool_heap *page_pool_heap =
		container_of(heap, struct hisi_drm_page_pool_heap, heap);
	u64 wt_pages;

	wt_mark = PAGE_ALIGN(wt_mark);
	wt_pages = wt_mark >> PAGE_SHIFT;

	if (wt_pages < MAX_ALLOWED_PAGES)
		page_pool_heap->pool_watermark = wt_mark;
	else
		pr_heaps_err(
			"want to set %llu pages as water mark, is to high, max is %lu pages\n",
			wt_pages, MAX_ALLOWED_PAGES);
}

static u64 hisi_drm_page_pool_get_water_mark(struct hisi_drm_heap *heap)
{
	struct hisi_drm_page_pool_heap *page_pool_heap =
		container_of(heap, struct hisi_drm_page_pool_heap, heap);

	return page_pool_heap->pool_watermark;
}

static struct hisi_drm_heap_ops drm_mem_page_pool_ops = {
	.alloc_buf = hisi_drm_page_pool_heap_allocate,
	.free_buf = hisi_drm_page_pool_heap_free,
	.map_kernel = hisi_drm_map_kernel_helper,
	.map_user = hisi_drm_map_user_helper,
	.unmap_kernel = hisi_drm_unmap_kernel_helper,
	.shrink = hisi_drm_page_pool_heap_shrink,
	.debug_show = hisi_drm_page_pool_debug_show,
	.set_water_mark = hisi_drm_page_pool_set_water_mark,
	.get_water_mark = hisi_drm_page_pool_get_water_mark,
};


struct hisi_drm_heap *
hisi_drm_page_pool_heap_create(struct hisi_drm_platform_heap *heap_data)
{
	struct hisi_drm_page_pool_heap *heap = NULL;
	int ret;

	if (!heap_data || !heap_data->node)
		return ERR_PTR(-EINVAL);

	pr_heaps_debug("begin to create page pool heap\n");
	heap = kzalloc(sizeof(*heap), GFP_KERNEL);
	if (!heap)
		return ERR_PTR(-ENOMEM);

	heap->heap.ops = &drm_mem_page_pool_ops;

	ret = hisi_drm_create_pools(heap, heap->pools);
	if (ret) {
		pr_heaps_err("err create pools, ret = %d\n", ret);
		goto free_heap;
	}

	atomic_set(&heap->total_pages_nr, 0);

	parse_heap_watermark(heap, heap_data->node);

	pr_heaps_debug("success to create page pool heap\n");

	return &heap->heap;

free_heap:
	kfree(heap);
	return ERR_PTR(ret);
}
