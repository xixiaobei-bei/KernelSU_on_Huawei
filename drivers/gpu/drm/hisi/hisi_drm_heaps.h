/*
 * hisi_drm_heaps.h
 *
 * Define DRM GEM Heap initial properties
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

#ifndef HISI_DRM_HEAPS_H
#define HISI_DRM_HEAPS_H

enum hisi_drm_heap_type {
	HISI_DRM_PAGE_POOL,
	HISI_DRM_DEBUG_POOL,
};

enum hisi_drm_heap_id {
	HISI_DRM_PAGE_POOL_HEAP_ID = 0,
	HISI_DRM_DEBUG_HEAP_ID = 1,
};

#endif
