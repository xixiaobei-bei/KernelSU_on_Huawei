#ifndef _HI36XX_SMMU_H
#define _HI36XX_SMMU_H

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

#ifdef CONFIG_HISI_IOMMU_LPAE
void dmabuf_release_iommu(struct dma_buf *dmabuf);

phys_addr_t hisi_domain_get_ttbr(struct device *dev);
unsigned long hisi_iommu_map_sg(struct device *dev, struct scatterlist *sgl,
				int prot, unsigned long *out_size);
int hisi_iommu_unmap_sg(struct device *dev, struct scatterlist *sgl,
			unsigned long iova);
unsigned long hisi_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				    int prot, unsigned long *out_size);
int hisi_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			    unsigned long iova);
unsigned long hisi_iommu_map(struct device *dev, phys_addr_t paddr,
			    size_t size, int prot);
int hisi_iommu_unmap(struct device *dev, unsigned long iova,
			    size_t size);
int hisi_iommu_idle_display_map(struct device *dev, u32 policy_id, phys_addr_t paddr,
			size_t allsize, size_t l3size, size_t lbsize);
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

static inline int hisi_iommu_idle_display_map(struct device *dev, u32 pid,
					      phys_addr_t paddr,
					      size_t allsize, size_t l3size,
					      size_t lbsize)
{
	return 0;
}
#endif /* CONFIG_HISI_IOMMU_LPAE */

#ifdef CONFIG_HISI_IOMMU_TEST
void hisi_smmu_show_pte(struct device *dev, unsigned long iova,
			unsigned long size);
void hisi_print_iova_dom(struct device *dev);
#else
static inline void hisi_smmu_show_pte(struct device *dev, unsigned long iova,
			unsigned long size)
{
}

static inline void hisi_print_iova_dom(struct device *dev) { }
#endif

#endif
