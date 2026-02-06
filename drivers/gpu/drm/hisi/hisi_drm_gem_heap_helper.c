/*
 * hisi_drm_gem_heap_helper.c
 *
 * Provide heaps common interface to gem, and organized request, post it into
 * heaps helper
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
#include "hisi_drm_gem_heap_helper.h"

#include <linux/types.h>
#include <linux/scatterlist.h>
#include <drm/drm_device.h>
#include <drm/drm_drv.h>

#include <linux/hisi/hisi_drm_heaps_tracer.h>
#include "heap/hisi_drm_heaps_helper.h"
#include "hisi_defs.h"
#include "hisi_drm.h"

#ifdef CONFIG_DRM_DSS_IOMMU
static bool is_support_iommu = true;
#else
static bool is_support_iommu;
#endif

unsigned int hisi_drm_gem_parse_flags_to_heap_id(unsigned int flags)
{
	if (flags & HISI_BO_PAGE_POOL_HEAP)
		return HISI_DRM_PAGE_POOL_HEAP_ID;
	else if (flags & HISI_BO_DEBUG_HEAP)
		return HISI_DRM_DEBUG_HEAP_ID;

	/* used default heap id */
	return HISI_DRM_PAGE_POOL_HEAP_ID;
}

int hisi_drm_gem_heap_alloc(struct hisi_drm_gem *hisi_gem)
{
	int ret;

	struct hisi_drm_heap_context ctx = {
		.head_id = hisi_gem->heap_id,
		.priv = hisi_gem,
		.flags = hisi_gem->flags,
	};

	ret = hisi_drm_heap_alloc(&ctx);
	if (ret)
		hisi_drm_heaps_process_show();

	return ret;
}

void hisi_drm_gem_heap_free(struct hisi_drm_gem *hisi_gem)
{
	struct hisi_drm_heap_context ctx = {
		.head_id = hisi_gem->heap_id,
		.priv = hisi_gem,
		.flags = hisi_gem->flags,
	};

	hisi_drm_heap_free(&ctx);
}

void *hisi_drm_gem_heap_kmap(struct hisi_drm_gem *hisi_gem)
{
	void *kvaddr = NULL;

	struct hisi_drm_heap_context ctx = {
		.head_id = hisi_gem->heap_id,
		.priv = hisi_gem,
		.flags = hisi_gem->flags,
	};

	kvaddr = hisi_drm_heap_map_kernel(&ctx);
	if (!kvaddr)
		hisi_drm_heaps_process_show();

	return kvaddr;
}

void hisi_drm_gem_heap_kunmap(struct hisi_drm_gem *hisi_gem, const void *vaddr)
{
	struct hisi_drm_heap_context ctx = {
		.head_id = hisi_gem->heap_id,
		.priv = hisi_gem,
		.flags = hisi_gem->flags,
	};

	hisi_drm_heap_unmap_kernel(&ctx, vaddr);
}

int hisi_drm_gem_heap_mmap(struct hisi_drm_gem *hisi_gem,
			   struct vm_area_struct *vma)
{
	int ret;
	struct hisi_drm_heap_context ctx = {
		.head_id = hisi_gem->heap_id,
		.priv = hisi_gem,
		.flags = hisi_gem->flags,
	};

	ret = hisi_drm_heap_map_user(&ctx, vma);
	if (ret)
		hisi_drm_heaps_process_show();

	return ret;
}

/**
 * check the flags, and if flags is empty, give a default flags.
 * if used PAGE POOL, we need iommu is enabled; else, check fail.
 */
bool hisi_drm_gem_check_flags(struct hisi_drm_gem *hisi_gem)
{
	unsigned int heap_id;
	unsigned int attrs;

	if (!hisi_gem)
		return false;

	heap_id = hisi_gem->heap_id;
	attrs = hisi_gem->flags;

	HISI_DRM_DEBUG("heap id = %u, attrs = %u\n", heap_id, attrs);

	if (!attrs) {
		if (is_support_iommu)
			attrs = HISI_BO_NONCONTIG | HISI_BO_NONCACHABLE |
				HISI_BO_WC;
		else
			attrs = HISI_BO_CONTIG | HISI_BO_NONCACHABLE |
				HISI_BO_WC;
	}

	hisi_gem->flags = attrs;

	if (heap_id == HISI_DRM_DEBUG_HEAP_ID)
		return true;

	if ((heap_id == HISI_DRM_PAGE_POOL_HEAP_ID) && is_support_iommu)
		return true;

	return false;
}
