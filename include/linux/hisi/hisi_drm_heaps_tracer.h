/*
 * hisi_drm_heaps_tracer.h
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

#ifndef HISI_HEAP_HISI_DRM_HEAPS_TRACER_H
#define HISI_HEAP_HISI_DRM_HEAPS_TRACER_H

#ifdef CONFIG_DRM_GEM_TRACE
void hisi_drm_heaps_process_show(void);
#else
static inline void hisi_drm_heaps_process_show(void)
{

}
#endif

#endif
