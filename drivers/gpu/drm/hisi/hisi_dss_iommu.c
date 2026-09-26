/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
/*lint -e86 -e559 -e598*/

#include "hisi_dss_iommu.h"

#include "drm/drmP.h"
#ifdef CONFIG_DRM_SMMU_V3
#include "hisi_drm_smmuv3.h"
#endif

struct dss_iova_info {
	struct list_head list_node;
	struct dma_buf *dmabuf;
	struct iova_info iova_info;
};

static struct dss_mm_info g_mm_list;

static struct dma_buf *hisi_dss_get_dmabuf(int sharefd)
{
	struct dma_buf *buf = NULL;

	/* dim layer share fd -1 */
	if (sharefd < 0) {
		HISI_DRM_ERR("invalid file sharefd %d!", sharefd);
		return NULL;
	}

	buf = dma_buf_get(sharefd);
	if (IS_ERR(buf)) {
		HISI_DRM_ERR("failed to dma_buf_get, sharefd %d!", sharefd);
		return NULL;
	}

	return buf;
}

static void hisi_dss_put_dmabuf(struct dma_buf *buf)
{
	if (!buf) {
		HISI_DRM_ERR("buf is nullptr!");
		return;
	}

	dma_buf_put(buf);
}

static struct dma_buf *hisi_dss_get_buffer_by_sharefd(
	struct device *dev,
	uint64_t *iova,
	int fd,
	uint32_t size)
{
	unsigned long buf_size = 0;
	struct dma_buf *buf = NULL;

	buf = hisi_dss_get_dmabuf(fd);
	if (!buf) {
		HISI_DRM_ERR("invalid file, shared_fd %d!", fd);
		return NULL;
	}

	*iova = hisi_iommu_map_dmabuf(dev, buf, 0, &buf_size);
	if ((*iova == 0) || (buf_size < size)) {
		HISI_DRM_ERR("get iova_size(0x%llx) smaller then size(0x%lx)!",
			buf_size, size);
		if (*iova) {
			hisi_iommu_unmap_dmabuf(dev, buf, *iova);
			*iova = 0;
		}

		return NULL;
	}

	return buf;
}

static void hisi_dss_put_buffer_by_dmabuf(
	struct device *dev,
	uint64_t iova,
	struct dma_buf *dmabuf)
{
	hisi_iommu_unmap_dmabuf(dev, dmabuf, iova);
	hisi_dss_put_dmabuf(dmabuf);
}

int hisi_dss_iommu_enable(struct device *dev)
{
	phys_addr_t ttbr = 0;
	struct dss_mm_info *mm_list = &g_mm_list;
	if (!dev) {
		HISI_DRM_ERR("dev is nullptr!");
		return -EINVAL;
	}
	ttbr = hisi_domain_get_ttbr(dev);

	dma_set_mask_and_coherent(dev, DMA_BIT_MASK(64));

	spin_lock_init(&mm_list->map_lock);
	INIT_LIST_HEAD(&mm_list->mm_list);

#ifdef CONFIG_DRM_SMMU_V3
	/* only one time be called */
	hisi_dss_smmuv3_init();
#endif

	return 0;
}

phys_addr_t hisi_dss_domain_get_ttbr(struct platform_device *pdev)
{
	if (!pdev) {
		HISI_DRM_ERR("pdev is nullptr");
		return 0;
	}
	return hisi_domain_get_ttbr(&pdev->dev);
}

int hisi_dss_buffer_map_iova(
	struct device *dev,
	struct fb_info *info,
	void __user *arg)
{
	struct iova_info map_data;
	struct dss_iova_info *node = NULL;
	struct dss_mm_info *mm_list = &g_mm_list;
	node = kzalloc(sizeof(struct dss_iova_info), GFP_KERNEL);
	if (node == NULL) {
		HISI_DRM_ERR("failed to alloc dss_iova_info!");
		return -ENOMEM;
	}

	if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
		HISI_DRM_ERR("failed to copy_from_user!");
		goto err_out;
	}

	node->iova_info.share_fd = map_data.share_fd;
	node->iova_info.calling_pid = map_data.calling_pid;
	node->iova_info.size = map_data.size;
	node->dmabuf = hisi_dss_get_buffer_by_sharefd(dev, &map_data.iova,
		map_data.share_fd, map_data.size);
	if (!node->dmabuf) {
		HISI_DRM_ERR("failed to map dma buf, share_fd(%d)!",
			map_data.share_fd);
		goto err_out;
	}
	node->iova_info.iova = map_data.iova;

	if (copy_to_user((void __user *)arg, &map_data, sizeof(map_data))) {
		HISI_DRM_ERR("failed to copy_to_user!");
		goto err_out;
	}

	/* save map list */
	spin_lock(&mm_list->map_lock);
	list_add_tail(&node->list_node, &mm_list->mm_list);
	spin_unlock(&mm_list->map_lock);

	return 0;

err_out:
	if (node) {
		if (node->dmabuf) {
			hisi_dss_put_buffer_by_dmabuf(dev,
				node->iova_info.iova, node->dmabuf);
		}

		kfree(node);
		node = NULL;
	}

	return -EFAULT;
}

int hisi_dss_buffer_unmap_iova(
	struct fb_info *info,
	void __user *arg)
{
	struct iova_info umap_data;
	struct dma_buf *dmabuf = NULL;
	struct dss_iova_info *node = NULL;
	struct dss_iova_info *_node_ = NULL;
	struct dss_mm_info *mm_list = &g_mm_list;
	if (copy_from_user(&umap_data, (void __user *)arg,
		sizeof(umap_data))) {
		HISI_DRM_ERR("failed to copy_from_user!");
		return -EFAULT;
	}

	dmabuf = hisi_dss_get_dmabuf(umap_data.share_fd);

	spin_lock(&mm_list->map_lock);
	list_for_each_entry_safe(node, _node_, &mm_list->mm_list, list_node) {
		if (!node)
			continue;

		if (node->dmabuf == dmabuf) {
			list_del(&node->list_node);
			/* already map, need put twice */
			hisi_dss_put_dmabuf(node->dmabuf);
			/* iova would be unmapped by dmabuf put */
			kfree(node);
		}
	}
	spin_unlock(&mm_list->map_lock);

	hisi_dss_put_dmabuf(dmabuf);

	return 0;
}

/*lint +e86 +e559 +e598*/
