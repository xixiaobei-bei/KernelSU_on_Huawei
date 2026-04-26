/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef HISI_DSS_ION_H
#define HISI_DSS_ION_H

#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/of_reserved_mem.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
#include <linux/ion.h>
#endif
#include <linux/fb.h>
#include <linux/version.h>
#include <linux/hisi-iommu.h>

void *hisifb_iommu_map_kernel(
	struct sg_table *sg_table,
	size_t size);

#endif /* HISI_DSS_ION_H */
