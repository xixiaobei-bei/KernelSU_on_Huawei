/*
 * hisi_drm_heaps_manager.h
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

#ifndef HISI_DRM_HEAPS_MANAGER_H
#define HISI_DRM_HEAPS_MANAGER_H

#include <linux/pfn.h>
#include <linux/atomic.h>
#include <linux/shrinker.h>
#include <linux/mm_types.h>
#include <linux/seq_file.h>

#include "hisi_drm_heaps.h"

/**
 * This is the platform heap, used for register hisi drm heap.
 * It pass the necessary information for the heap who need to be
 * initialized.
 */
struct hisi_drm_platform_heap {
	enum hisi_drm_heap_type type;
	unsigned int id;
	const char *name;
	struct device_node *node;
	size_t size;
	phys_addr_t base;
	void *priv;
};

/**
 * This is the core struct for heaps ops, used for pass necessary information
 * when invoke the heap's hook func
 */
struct hisi_drm_heap_context {
	size_t size;
	unsigned int flags;
	unsigned int head_id;

	/* in normal, it's the hisi_drm_gem structure */
	void *priv;
};

/**
 * This structure defines the generic parts for Heap, include hook func
 */
struct hisi_drm_heap {
	enum hisi_drm_heap_id heap_id;

	struct list_head heap_list;

	struct shrinker shrinker;

	struct hisi_drm_heap_ops *ops;

	const char *name;
};

/**
 * This structure defines the core function which the hisi drm heap can offer.
 * Heaps must implement the alloc_buf/free_buf.
 */
struct hisi_drm_heap_ops {
	int (*alloc_buf)(struct hisi_drm_heap *heap,
			 struct hisi_drm_heap_context *ctx);

	void (*free_buf)(struct hisi_drm_heap *heap,
			 struct hisi_drm_heap_context *ctx);

	/**
	 * @map_kernel: map dmabuf in kernel space
	 *
	 * this is the dmabuf map's hook, implement by heap
	 */
	void *(*map_kernel)(struct hisi_drm_heap *heap,
			    struct hisi_drm_heap_context *ctx);

	/**
	 * @unmap_kernel: unmap dmabuf in kernel space
	 *
	 * this is the dmabuf unmap's hook, implement by heap
	 */
	void (*unmap_kernel)(struct hisi_drm_heap *heap,
			     struct hisi_drm_heap_context *ctx, const void *vaddr);

	/**
	 * @map_user: map dmabuf in user space
	 *
	 * this is the dmabuf mmap's hook, implement by heap
	 */
	int (*map_user)(struct hisi_drm_heap *heap,
			struct hisi_drm_heap_context *ctx,
			struct vm_area_struct *vma);

	int (*shrink)(struct hisi_drm_heap *heap, gfp_t gfp_mask,
		      int nr_to_scan);

	int (*debug_show)(struct hisi_drm_heap *heap, struct seq_file *,
			  void *);

	void (*set_water_mark)(struct hisi_drm_heap *heap, u64 wt_mark);
	u64 (*get_water_mark)(struct hisi_drm_heap *heap);
};

struct hisi_drm_heap *hisi_drm_get_heap(unsigned int heap_id);

#endif
