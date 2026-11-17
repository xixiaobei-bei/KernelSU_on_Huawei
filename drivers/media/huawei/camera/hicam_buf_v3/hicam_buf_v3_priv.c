/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Implement of hicamera buffer v3 priv.
 * Author: yancong
 * Create: 2019-07-30
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <hicam_buf.h>
#include <cam_log.h>

#include <linux/hisi-iommu.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/dma-buf.h>

#define MAX_PADDING_SIZE (150 * 1024 * 1024)

int hicam_v3_internal_map_iommu(struct device *dev,
	int fd, struct iommu_format *fmt, int padding_support)
{
	int rc = 0;
	struct dma_buf *dmabuf = NULL;
	unsigned long iova_addr = 0;
	unsigned long iova_size = 0;
	if (IS_ERR_OR_NULL(dev)) {
		cam_err("%s: fail to get dev", __FUNCTION__);
		return -ENOENT;
	}

	if (fmt->size > MAX_PADDING_SIZE) {
		cam_err("%s: padding size is oversize", __func__);
		return -ENOENT;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		cam_err("%s: fail to get dma dmabuf", __FUNCTION__);
		return -ENOENT;
	}

	if (padding_support && fmt->size != 0)
		iova_addr = hisi_iommu_map_padding_dmabuf(dev, dmabuf,
			fmt->size, fmt->prot, &iova_size);
	else
		iova_addr = hisi_iommu_map_dmabuf(dev, dmabuf, fmt->prot,
			&iova_size);

	if (!iova_addr) {
		cam_err("%s: fail to map iommu", __FUNCTION__);
		rc = -ENOMEM;
		goto err_map_iommu;
	}

	cam_info("%s: fd:%d, dmabuf:%pK iova:%#lx, maped size:%#lx, "
		"padding_support: %d, fmt size: %#lx",
		__FUNCTION__, fd, dmabuf, iova_addr, iova_size, padding_support,
		fmt->size);
	fmt->iova = iova_addr;
	fmt->size = iova_size;

err_map_iommu:
	dma_buf_put(dmabuf);
	return rc;
}

void hicam_v3_internal_unmap_iommu(struct device *dev,
	int fd, struct iommu_format *fmt, int padding_support)
{
	int rc;
	struct dma_buf *dmabuf = NULL;
	if (IS_ERR_OR_NULL(dev)) {
		cam_err("%s: fail to get dev", __FUNCTION__);
		return;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		cam_err("%s: fail to get dma dmabuf", __FUNCTION__);
		return;
	}

	if (padding_support && fmt->size != 0)
		rc = hisi_iommu_unmap_padding_dmabuf(dev, dmabuf,
			fmt->size, fmt->iova);
	else
		rc = hisi_iommu_unmap_dmabuf(dev, dmabuf, fmt->iova);

	if (rc < 0)
		cam_err("%s: failed", __FUNCTION__);

	cam_info("%s: fd:%d, dmabuf:%pK, unmaped iova:0x%016lx, padding_support: %d, fmt size: %#lx",
		__FUNCTION__, fd, dmabuf, fmt->iova, padding_support,
		fmt->size);
	dma_buf_put(dmabuf);
}
