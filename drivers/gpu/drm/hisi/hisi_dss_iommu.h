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
#ifndef HISI_DSS_IOMMU_H
#define HISI_DSS_IOMMU_H

#include <linux/iommu.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/hisi-iommu.h>

#include "hisi_dpe.h"
#include "hisi_overlay_cmdlist_utils.h"


struct iova_info {
	uint64_t iova;
	uint64_t size;
	int share_fd;
	int calling_pid;
};

struct dss_mm_info {
	struct list_head mm_list;
	spinlock_t map_lock;
};

int hisi_dss_buffer_map_iova(
	struct device *dev,
	struct fb_info *info,
	void __user *arg);
int hisi_dss_buffer_unmap_iova(
	struct fb_info *info,
	void __user *arg);

int hisi_dss_iommu_enable(
	struct device *pdev);
phys_addr_t hisi_dss_domain_get_ttbr(
	struct platform_device *pdev);


#endif /* HISI_DSS_IOMMU_H */
