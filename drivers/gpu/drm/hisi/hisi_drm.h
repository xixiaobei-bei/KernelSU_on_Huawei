/*
 * Hisilicon Kirin SoCs drm master driver
 *
 * Copyright (c) 2019 Linaro Limited.
 * Copyright (c) 2019-2020 Hisilicon Limited.
 *
 * Author:
 * <yangweijian@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef HISI_DRM_H
#define HISI_DRM_H

#include <drm/drm.h>

/**
 * User-desired buffer creation information structure.
 *
 * @size: user-desired memory allocation size.
 * - this size value would be page-aligned internally.
 * @flags: user request for setting memory type or cache attributes.
 * @handle: returned a handle to created gem object.
 * - this handle will be set by gem module of kernel side.
 */
struct drm_hisi_gem_create {
	__u64 size;
	__u32 flags;
	__u32 handle;
};

/* memory type definitions. */
enum e_drm_hisi_gem_mem_type {
	/* Physically Continuous memory and used as default */
	HISI_BO_CONTIG = 0 << 0,
	/* Physically Non-Continuous memory. */
	HISI_BO_NONCONTIG = 1 << 0,
	/* non-cachable mapping and used as default */
	HISI_BO_NONCACHABLE = 0 << 1,
	/* cachable mapping. */
	HISI_BO_CACHABLE = 1 << 1,
	/* write-combine mapping. */
	HISI_BO_WC			= 1 << 2,
	HISI_BO_MASK		= HISI_BO_NONCONTIG | HISI_BO_CACHABLE |
							HISI_BO_WC,
	/* Map iommu when alloc success */
	HISI_BO_ALLOC_WITH_MAP_IOMMU = 	1 << 24,

	/* Default Memory Heap */
	HISI_BO_PAGE_POOL_HEAP = 1 << 28,

	/* Used for debug, By default, this parameter is not used. */
	HISI_BO_DEBUG_HEAP = 1 << 29,
};

#define DRM_HISI_GEM_CREATE 0x00

#define DRM_IOCTL_HISI_GEM_CREATE DRM_IOWR(DRM_COMMAND_BASE + \
				DRM_HISI_GEM_CREATE, struct drm_hisi_gem_create)

#endif
