/*
 * hisi_drm_heaps_helper.h
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

#ifndef HISI_DRM_HEAPS_HELPER_H
#define HISI_DRM_HEAPS_HELPER_H

#include "hisi_drm_heaps_manager.h"

int hisi_drm_heap_alloc(struct hisi_drm_heap_context *cxt);

void hisi_drm_heap_free(struct hisi_drm_heap_context *cxt);

void *hisi_drm_heap_map_kernel(struct hisi_drm_heap_context *cxt);

void hisi_drm_heap_unmap_kernel(struct hisi_drm_heap_context *cxt,
				const void *vaddr);

int hisi_drm_heap_map_user(struct hisi_drm_heap_context *cxt,
			   struct vm_area_struct *vma);

#endif
