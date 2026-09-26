/*
 * hisi_drm_gem_heap_helper.h
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

#ifndef HISI_DRM_GEM_HEAP_HELPER_H
#define HISI_DRM_GEM_HEAP_HELPER_H

#include <linux/types.h>
#include "hisi_drm_gem.h"

int hisi_drm_gem_heap_alloc(struct hisi_drm_gem *hisi_gem);
void hisi_drm_gem_heap_free(struct hisi_drm_gem *hisi_gem);
bool hisi_drm_gem_check_flags(struct hisi_drm_gem *hisi_gem);
void *hisi_drm_gem_heap_kmap(struct hisi_drm_gem *hisi_gem);
void hisi_drm_gem_heap_kunmap(struct hisi_drm_gem *hisi_gem, const void *vaddr);
int hisi_drm_gem_heap_mmap(struct hisi_drm_gem *hisi_gem,
			      struct vm_area_struct *vma);
unsigned int hisi_drm_gem_parse_flags_to_heap_id(unsigned int flags);
#endif
