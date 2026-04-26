/*
 * hisi_drm_heap_ops_helper.h
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

#ifndef HISI_DRM_HEAP_OPS_HELPER_H
#define HISI_DRM_HEAP_OPS_HELPER_H

#include "hisi_drm_gem.h"
#include "hisi_drm_heaps_manager.h"

void *hisi_drm_map_kernel_helper(struct hisi_drm_heap *heap,
				 struct hisi_drm_heap_context *ctx);
void hisi_drm_unmap_kernel_helper(struct hisi_drm_heap *heap,
				  struct hisi_drm_heap_context *ctx,
				  const void *vaddr);
int hisi_drm_map_user_helper(struct hisi_drm_heap *heap,
			     struct hisi_drm_heap_context *ctx,
			     struct vm_area_struct *vma);
#endif
