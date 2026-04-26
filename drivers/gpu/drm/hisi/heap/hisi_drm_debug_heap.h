/*
 * hisi_drm_debug_heap.h
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

#ifndef HISI_DRM_DEBUG_HEAP_H
#define HISI_DRM_DEBUG_HEAP_H

#include <linux/genalloc.h>

#include "hisi_drm_heaps_manager.h"

struct hisi_drm_debug_heap {
	struct hisi_drm_heap heap;
	struct gen_pool *pool;
};

struct hisi_drm_debug_pool {
	phys_addr_t phys_base;
	unsigned int pool_size;
};

struct hisi_drm_heap *
hisi_drm_debug_heap_create(struct hisi_drm_platform_heap *heap_data);

#endif
