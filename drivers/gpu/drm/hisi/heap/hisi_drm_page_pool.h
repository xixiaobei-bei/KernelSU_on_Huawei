/*
 * hisi_drm_page_pool.h
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

#ifndef HISI_DRM_PAGE_POOL_H
#define HISI_DRM_PAGE_POOL_H

#include <linux/mm_types.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/types.h>

struct hisi_drm_page_pool {
	int high_count;
	int low_count;
	struct list_head high_items;
	struct list_head low_items;
	struct mutex mutex;
	gfp_t gfp_mask;
	unsigned int order;
};

struct page *hisi_drm_page_pool_alloc(struct hisi_drm_page_pool *pool,
				      bool *is_from_pool);

void hisi_drm_page_pool_free(struct hisi_drm_page_pool *pool,
			     struct page *page);

void hisi_drm_page_pool_free_immediate(struct hisi_drm_page_pool *pool,
				       struct page *page);

unsigned int hisi_drm_page_pool_total(struct hisi_drm_page_pool *pool,
				      bool high);

int hisi_drm_page_pool_shrink(struct hisi_drm_page_pool *pool, gfp_t gfp_mask,
			      int nr_to_scan);

struct hisi_drm_page_pool *hisi_drm_page_pool_create(gfp_t gfp_mask,
						     unsigned int order);

void hisi_drm_page_pool_destroy(struct hisi_drm_page_pool *pool);

#endif
