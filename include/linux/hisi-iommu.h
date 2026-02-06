/*
 * Copyright(C) 2004-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _HISI_IOMMU_H
#define _HISI_IOMMU_H

#include <linux/dma-buf.h>
#include <linux/types.h>
#include <linux/iommu.h>
#include <linux/platform_device.h>

#define IOMMU_DEVICE (1 << 7)
#define IOMMU_SEC    (1 << 8)
#define IOMMU_EXEC   (1 << 9)
#ifdef CONFIG_HISI_LB
#define IOMMU_PORT_SHIFT	(12)
#define IOMMU_PORT_MASK	(0xFF << IOMMU_PORT_SHIFT)
#endif

extern int of_get_iova_info(struct device_node *np, unsigned long *iova_start,
			unsigned long *iova_size, unsigned long *iova_align);

#ifdef CONFIG_HISI_IOMMU
void dmabuf_release_iommu(struct dma_buf *dmabuf);
phys_addr_t hisi_domain_get_ttbr(struct device *dev);
unsigned long hisi_iommu_map_sg(struct device *dev, struct scatterlist *sgl,
				int prot, unsigned long *out_size);
int hisi_iommu_unmap_sg(struct device *dev, struct scatterlist *sgl,
			unsigned long iova);
unsigned long mm_drm_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				    int prot, unsigned long *out_size);
int mm_drm_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			    unsigned long iova);
unsigned long hisi_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				    int prot, unsigned long *out_size);
int hisi_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			    unsigned long iova);
unsigned long hisi_iommu_map(struct device *dev, phys_addr_t paddr,
			    size_t size, int prot);
int hisi_iommu_unmap(struct device *dev, unsigned long iova,
			    size_t size);
unsigned long hisi_iommu_idle_display_map(struct device *dev, u32 policy_id,
			struct dma_buf *dmabuf, size_t allsize,
			size_t l3size, size_t lbsize);
int hisi_iommu_idle_display_unmap(struct device *dev, unsigned long iova,
			size_t size, u32 policy_id, struct dma_buf *dmabuf);
int hisi_iommu_dev_flush_tlb(struct device *dev, unsigned int ssid);
size_t hisi_iommu_unmap_fast(struct device *dev,
			unsigned long iova, size_t size);
#else
static inline void dmabuf_release_iommu(struct dma_buf *dmabuf) {}

static inline phys_addr_t hisi_domain_get_ttbr(struct device *dev)
{
	return 0;
}

static inline unsigned long hisi_iommu_map_sg(struct device *dev,
					      struct scatterlist *sgl,
					      int prot,
					      unsigned long *out_size)
{
	return 0;
}

static inline int hisi_iommu_unmap_sg(struct device *dev,
				      struct scatterlist *sgl,
				      unsigned long iova)
{
	return -EINVAL;
}

static inline unsigned long mm_drm_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				    int prot, unsigned long *out_size)
{
	return 0;
}

static inline int mm_drm_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			    unsigned long iova)
{
	return -EINVAL;
}

static inline unsigned long hisi_iommu_map_dmabuf(struct device *dev,
						  struct dma_buf *dmabuf,
						  int prot,
						  unsigned long *out_size)
{
	return 0;
}

static inline int hisi_iommu_unmap_dmabuf(struct device *dev,
					  struct dma_buf *dmabuf,
					  unsigned long iova)
{
	return -EINVAL;
}

static inline unsigned long hisi_iommu_map(struct device *dev,
					   phys_addr_t paddr,
					   size_t size, int prot)
{
	return 0;
}

static inline int hisi_iommu_unmap(struct device *dev, unsigned long iova,
				   size_t size)
{
	return -EINVAL;
}

static inline unsigned long hisi_iommu_idle_display_map(struct device *dev,
					u32 policy_id,
					struct dma_buf *dmabuf,
					size_t allsize, size_t l3size,
					size_t lbsize)
{
	return 0;
}

static inline int hisi_iommu_idle_display_unmap(struct device *dev,
					unsigned long iova, size_t size,
					u32 policy_id, struct dma_buf *dmabuf)
{
	return 0;
}

static inline int hisi_iommu_dev_flush_tlb(struct device *dev,
				unsigned int ssid)
{
	return 0;
}

static inline size_t hisi_iommu_unmap_fast(struct device *dev,
			     unsigned long iova, size_t size)
{
	return 0;
}
#endif /* CONFIG_HISI_IOMMU */

#ifdef CONFIG_HISI_IOMMU_LAST_PAGE
unsigned long hisi_iommu_map_padding_dmabuf(struct device *dev,
		struct dma_buf *dmabuf, unsigned long padding_len,
		int prot, unsigned long *out_size);
int hisi_iommu_unmap_padding_dmabuf(struct device *dev, struct dma_buf *dmabuf,
		unsigned long padding_len, unsigned long iova);
#else
static inline unsigned long hisi_iommu_map_padding_dmabuf(struct device *dev,
		struct dma_buf *dmabuf, unsigned long padding_len,
		int prot, unsigned long *out_size)
{
	return 0;
}

static inline int hisi_iommu_unmap_padding_dmabuf(struct device *dev,
	struct dma_buf *dmabuf, unsigned long padding_len, unsigned long iova)
{
	return 0;
}
#endif /* CONFIG_HISI_IOMMU_LAST_PAGE */

#ifdef CONFIG_HISI_IOMMU_TEST
void mm_smmu_show_pte(struct device *dev, unsigned long iova,
			unsigned long size);
void mm_print_iova_dom(struct device *dev);
#else
static inline void mm_smmu_show_pte(struct device *dev, unsigned long iova,
			unsigned long size)
{
}

static inline void mm_print_iova_dom(struct device *dev)
{
}
#endif

#endif
