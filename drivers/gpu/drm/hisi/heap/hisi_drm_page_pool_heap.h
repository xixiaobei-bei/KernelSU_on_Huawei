/*
 * hisi_drm_page_pool_heap.h
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

#ifndef HISI_DRM_PAGE_POOL_HEAP_H
#define HISI_DRM_PAGE_POOL_HEAP_H

#include <linux/mm_types.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "hisi_drm_gem_heap_helper.h"
#include "hisi_drm_heaps_manager.h"

#define LOW_ORDER 0
#define MID_ORDER 4
#define HIGH_ORDER 8

static gfp_t high_order_gfp_flags =
	(GFP_HIGHUSER | __GFP_ZERO | __GFP_NOWARN | __GFP_NORETRY) &
	~__GFP_RECLAIM;
static gfp_t low_order_gfp_flags = GFP_HIGHUSER | __GFP_ZERO;
static const unsigned int orders[] = { HIGH_ORDER, MID_ORDER, LOW_ORDER };

#define NUM_ORDERS ARRAY_SIZE(orders)

struct hisi_drm_page_pool_heap {
	struct hisi_drm_heap heap;
	struct hisi_drm_page_pool *pools[NUM_ORDERS];
	u64 pool_watermark;
	atomic_t total_pages_nr;
};

struct hisi_drm_heap *
hisi_drm_page_pool_heap_create(struct hisi_drm_platform_heap *heap_data);
#endif
