/*
 * hisi_drm_page_pool.c
 *
 * page pool's common implement
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
#include "hisi_drm_page_pool.h"

#include <linux/list.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/vmstat.h>

#include "hisi_defs.h"

static inline struct page *
hisi_drm_page_pool_alloc_pages(struct hisi_drm_page_pool *pool)
{
	return alloc_pages(pool->gfp_mask, pool->order);
}

static void hisi_drm_page_pool_free_pages(struct hisi_drm_page_pool *pool,
					  struct page *page)
{
	__free_pages(page, pool->order);
}

static void hisi_drm_page_pool_add(struct hisi_drm_page_pool *pool,
				   struct page *page)
{
	mutex_lock(&pool->mutex);
	if (PageHighMem(page)) {
		list_add_tail(&page->lru, &pool->high_items);
		++pool->high_count;
	} else {
		list_add_tail(&page->lru, &pool->low_items);
		++pool->low_count;
	}
	mutex_unlock(&pool->mutex);
}

static struct page *hisi_drm_page_pool_remove(struct hisi_drm_page_pool *pool,
					      bool high)
{
	struct page *page = NULL;

	if (high) {
		page = list_first_entry(&pool->high_items, struct page, lru);
		--pool->high_count;
	} else {
		page = list_first_entry(&pool->low_items, struct page, lru);
		--pool->low_count;
	}


	list_del(&page->lru);
	return page;
}

/**
 * Alloc page from page pool.
 * If page in highmem is avaliable, get page from here.
 * If both low and high page pool do not have enough  page caches,
 * get page from buddy systeam.
 * @pool page pool
 * @is_from_pool used to count pool's usage.
 * true -- alloc from pool, count down pool's cache num
 * false -- no nothing
 */
struct page *hisi_drm_page_pool_alloc(struct hisi_drm_page_pool *pool,
				      bool *is_from_pool)
{
	struct page *page = NULL;

	if (!pool)
		return ERR_PTR(-EINVAL);

	mutex_lock(&pool->mutex);
	if (pool->high_count)
		page = hisi_drm_page_pool_remove(pool, true);
	else if (pool->low_count)
		page = hisi_drm_page_pool_remove(pool, false);
	mutex_unlock(&pool->mutex);

	if (!page) {
		*is_from_pool = false;
		page = hisi_drm_page_pool_alloc_pages(pool);

	}

	return page;
}

/**
 * add page into page pool
 * @pool page pool that order is page's order
 * @page page
 */
void hisi_drm_page_pool_free(struct hisi_drm_page_pool *pool, struct page *page)
{
	BUG_ON(pool->order != compound_order(page));

	hisi_drm_page_pool_add(pool, page);
}

/**
 * free page into buddy system
 * @pool page pool that order is page's order
 * @page page
 */
void hisi_drm_page_pool_free_immediate(struct hisi_drm_page_pool *pool,
				       struct page *page)
{
	hisi_drm_page_pool_free_pages(pool, page);
}

/**
 * count pool's pgcache count
 * @pool page pool
 * @high if true, count high's item
 * @return pool's cached pages's count
 */
unsigned int hisi_drm_page_pool_total(struct hisi_drm_page_pool *pool,
				      bool high)
{
	unsigned int count = pool->low_count;

	if (high)
		count += pool->high_count;

	return count << pool->order;
}

/**
 * Page pool's shrink interface, can use this count pool's page count and
 * shrink page cache
 * @pool page pool
 * @gfp_mask operate mask
 * @nr_to_scan
 * 0 -- just count cached pages count
 * > 0 -- need shrink and fit this number.
 * @return total cached pages number or reclaimed count
 */
int hisi_drm_page_pool_shrink(struct hisi_drm_page_pool *pool, gfp_t gfp_mask,
			      int nr_to_scan)
{
	int freed = 0;
	bool high = false;
	struct page *page = NULL;

	if (current_is_kswapd())
		high = true;
	else
		high = !!(gfp_mask & __GFP_HIGHMEM);

	if (nr_to_scan == 0)
		return (int)hisi_drm_page_pool_total(pool, high);

	while (freed < nr_to_scan) {

		mutex_lock(&pool->mutex);
		if (pool->low_count) {
			page = hisi_drm_page_pool_remove(pool, false);
		} else if (high && pool->high_count) {
			page = hisi_drm_page_pool_remove(pool, true);
		} else {
			mutex_unlock(&pool->mutex);
			break;
		}
		mutex_unlock(&pool->mutex);
		hisi_drm_page_pool_free_pages(pool, page);
		freed += (1 << pool->order);
	}

	return freed;
}

struct hisi_drm_page_pool *hisi_drm_page_pool_create(gfp_t gfp_mask,
						     unsigned int order)
{
	struct hisi_drm_page_pool *pool = kzalloc(sizeof(*pool), GFP_KERNEL);

	if (!pool)
		return NULL;

	INIT_LIST_HEAD(&pool->low_items);
	INIT_LIST_HEAD(&pool->high_items);
	pool->gfp_mask = gfp_mask | __GFP_COMP;
	pool->order = order;
	mutex_init(&pool->mutex);

	return pool;
}

void hisi_drm_page_pool_destroy(struct hisi_drm_page_pool *pool)
{
	struct page *page = NULL;

	mutex_lock(&pool->mutex);
	while (pool->high_count) {
		page = hisi_drm_page_pool_remove(pool, true);
		hisi_drm_page_pool_free_pages(pool, page);
	}

	while (pool->low_count) {
		page = hisi_drm_page_pool_remove(pool, false);
		hisi_drm_page_pool_free_pages(pool, page);
	}
	mutex_unlock(&pool->mutex);

	mutex_destroy(&pool->mutex);
	kfree(pool);
}
