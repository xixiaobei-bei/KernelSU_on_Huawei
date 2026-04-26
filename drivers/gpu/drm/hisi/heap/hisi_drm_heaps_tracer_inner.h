/*
 * hisi_drm_heaps_tracer_inner.h
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

#ifndef HISI_HEAP_HISI_DRM_HEAPS_TRACER_INNER_H
#define HISI_HEAP_HISI_DRM_HEAPS_TRACER_INNER_H

#include "hisi_drm_heaps_manager.h"
#include "hisi_drm_gem.h"

#ifdef CONFIG_DRM_GEM_TRACE
void hisi_drm_gem_info_show(struct hisi_drm_gem *hisi_gem);
void hisi_drm_heaps_gem_trace(struct hisi_drm_heap_context *ctx);
void hisi_drm_heaps_remove_trace(struct hisi_drm_heap_context *ctx);
int hisi_drm_heaps_tracer_init(void);
#else
static inline void hisi_drm_gem_info_show(struct hisi_drm_gem *hisi_gem)
{

}

static inline void hisi_drm_heaps_gem_trace(struct hisi_drm_heap_context *ctx)
{

}

static inline void hisi_drm_heaps_remove_trace(struct hisi_drm_heap_context *ctx)
{

}

static inline int hisi_drm_heaps_tracer_init(void)
{
	return 0;
}
#endif

#if defined(CONFIG_DRM_GEM_TRACE) && defined(CONFIG_HISI_DEBUG_FS)
void hisi_drm_heaps_init_heap_debugfs(struct hisi_drm_heap *heap);
#else
static inline void hisi_drm_heaps_init_heap_debugfs(struct hisi_drm_heap *heap)
{
}
#endif

#endif
