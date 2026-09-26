/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: This is for smmuv3 dma driver.
 * Create: 2019-12-01
 */

#ifndef _HISI_IOMMU_DMA_H
#define _HISI_IOMMU_DMA_H

#include <linux/device.h>
#include <linux/dma-mapping.h>

#ifdef CONFIG_HISI_IOMMU_DMA
void mm_iommu_setup_dma_ops(struct device *dev);
#else
static inline void mm_iommu_setup_dma_ops(struct device *dev) {}
#endif

#endif
