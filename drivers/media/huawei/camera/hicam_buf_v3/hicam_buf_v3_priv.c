

#include <hicam_buf.h>
#include <cam_log.h>

#include <linux/hisi-iommu.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/dma-buf.h>

int hicam_v3_internal_map_iommu(struct device *dev,
		int fd, struct iommu_format *fmt, int padding_support)
{
	int rc = 0;
	struct dma_buf *dmabuf = NULL;
	unsigned long iova_addr, iova_size = 0;
	if (!dev) {
		cam_err("%s: fail to get dev", __func__);
		return -ENOENT;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		cam_err("%s: fail to get dma dmabuf", __func__);
		return -ENOENT;
	}

	if (padding_support && fmt->size != 0) {
		iova_addr = hisi_iommu_map_padding_dmabuf(dev, dmabuf,
				fmt->size, fmt->prot, &iova_size);
	} else {
		iova_addr = hisi_iommu_map_dmabuf(dev, dmabuf, fmt->prot,
				&iova_size);
	}

	if (!iova_addr) {
		cam_err("%s: fail to map iommu", __func__);
		rc = -ENOMEM;
		goto err_map_iommu;
	}

	cam_info("%s: fd:%d, dmabuf:%pK iova:%#lx, maped size:%#lx, padding_support: %d, fmt size: %#lx",
			__func__, fd, dmabuf, iova_addr, iova_size, padding_support, fmt->size);
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
	if (!dev) {
		cam_err("%s: fail to get dev", __func__);
		return;
	}

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		cam_err("%s: fail to get dma dmabuf", __func__);
		return ;
	}

	if (padding_support && fmt->size != 0) {
		rc = hisi_iommu_unmap_padding_dmabuf(dev, dmabuf,
				fmt->size, fmt->iova);
	} else {
		rc = hisi_iommu_unmap_dmabuf(dev, dmabuf, fmt->iova);
	}

	if (rc < 0)
		cam_err("%s: failed", __func__);

	cam_info("%s: fd:%d, dmabuf:%pK, unmaped iova: 0x%0x, padding_support: %d, fmt size: %#lx",
			__func__, fd, dmabuf, fmt->iova, padding_support, fmt->size);
	dma_buf_put(dmabuf);
}

