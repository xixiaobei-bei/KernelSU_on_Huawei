/*
 * Hisilicon Kirin camera driver source file
 *
 * Copyright (C) Huawei Technology Co., Ltd.
 *
 * Author:	wenjianyue
 * Email:	wenjianyue@huawei.com
 * Date:	2018-11-28
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <hicam_buf.h>
#include <cam_log.h>

#include <linux/hisi-iommu.h>
#include <linux/device.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/dma-buf.h>
#include <linux/kernel.h>

struct sgtable_node {
	struct kref ref;
	struct list_head list;
	struct sg_table *sgt;
	struct dma_buf *buf;
	struct dma_buf_attachment *attachment;
};

struct priv_ion_t {
	struct device *dev;

	struct mutex sg_mutex;
	struct list_head sg_nodes;
};

void* hicam_internal_map_kernel(struct device *dev, int fd)
{
	int rc;
	void *kaddr = NULL;
	struct dma_buf *dmabuf;

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		cam_err("%s: fail to get dma dmabuf.", __func__);
		return NULL;
	}
	/* map ion buffer into kernel address space, with invalidate side effect. */
	rc = dma_buf_begin_cpu_access(dmabuf, DMA_FROM_DEVICE);
	if (rc < 0) {
		cam_err("%s: fail to map kernel.", __func__);
		goto err_map_kernel;
	}
	/* dma_buf_kmap will always succeed. */
	kaddr = dma_buf_kmap(dmabuf, /* offset in PAGE granularity */ 0);

err_map_kernel:
	dma_buf_put(dmabuf);
	return kaddr;
}

void hicam_internal_unmap_kernel(struct device *dev, int fd)
{
	int rc = 0;
	struct dma_buf *dmabuf = NULL;

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		cam_err("%s: fail to get dma dmabuf.", __func__);
		return ;
	}

	dma_buf_kunmap(dmabuf, 0, NULL); /* third param is not used. */
	/* unmap ion buffer from kernel address space, with clean side effect. */
	rc = dma_buf_end_cpu_access(dmabuf, DMA_TO_DEVICE);
	if (rc < 0) {
		cam_err("%s: failed.", __func__);
	}

	dma_buf_put(dmabuf);
}

int hicam_internal_map_iommu(struct device *dev,
		int fd, struct iommu_format *fmt)
{
	int rc = 0;
	struct dma_buf *dmabuf = NULL;
	unsigned long iova_addr, iova_size = 0;
	struct priv_ion_t *ion = dev_get_drvdata(dev);

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		cam_err("%s: fail to get dma dmabuf.", __func__);
		return -ENOENT;
	}

	iova_addr = hisi_iommu_map_dmabuf(ion->dev, dmabuf, fmt->prot, &iova_size);
	if (!iova_addr) {
		cam_err("%s: fail to map iommu.", __func__);
		rc = -ENOMEM;
		goto err_map_iommu;
	}

	cam_debug("%s: fd:%d, dmabuf:%pK iova:%#lx, size:%#lx.", __func__,
			fd, dmabuf, iova_addr, iova_size);
	fmt->iova = iova_addr;
	fmt->size = iova_size;

err_map_iommu:
	dma_buf_put(dmabuf);
	return rc;
}

void hicam_internal_unmap_iommu(struct device *dev,
		int fd, struct iommu_format *fmt)
{
	int rc;
	struct dma_buf *dmabuf = NULL;
	struct priv_ion_t *ion = dev_get_drvdata(dev);

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		cam_err("%s: fail to get dma dmabuf.", __func__);
		return ;
	}

	rc = hisi_iommu_unmap_dmabuf(ion->dev, dmabuf, fmt->iova);
	if (rc < 0) {
		cam_err("%s: failed.", __func__);
	}

	cam_debug("%s: fd:%d, dmabuf:%pK.", __func__, fd, dmabuf);
	dma_buf_put(dmabuf);
}

struct sgtable_node* find_sgtable_node_by_fd(struct priv_ion_t *ion, int fd)
{
	struct dma_buf *dmabuf = NULL;
	struct sgtable_node *node = NULL;
	struct sgtable_node *ret_node = ERR_PTR(-ENOENT);

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		cam_err("%s: fail to get dma buf.", __func__);
		return ret_node;
	}

	list_for_each_entry(node, &ion->sg_nodes, list) {
		if (node->buf == dmabuf) {
			ret_node = node;
			break;
		}
	}

	dma_buf_put(dmabuf);
	return ret_node;
}

struct sgtable_node* find_sgtable_node_by_sg(struct priv_ion_t *ion,
		struct sg_table *sgt)
{
	struct sgtable_node *node = NULL;

	list_for_each_entry(node, &ion->sg_nodes, list) {
		if (node->sgt == sgt) {
			return node;
		}
	}

	return ERR_PTR(-ENOENT);
}

struct sgtable_node* create_sgtable_node(struct priv_ion_t *ion, int fd)
{
	struct sgtable_node *node;

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node) {
		cam_err("%s: fail to alloc sgtable_node.", __func__);
		return ERR_PTR(-ENOMEM);
	}

	node->buf = dma_buf_get(fd);
	if (IS_ERR(node->buf)) {
		cam_err("%s: fail to get dma buf.", __func__);
		goto err_get_buf;
	}

	node->attachment = dma_buf_attach(node->buf, ion->dev);
	if (IS_ERR(node->attachment)) {
		cam_err("%s: fail to attach dma buf.", __func__);
		goto err_attach_buf;
	}

	node->sgt = dma_buf_map_attachment(node->attachment, DMA_BIDIRECTIONAL);
	if (IS_ERR(node->sgt)) {
		cam_err("%s: fail to map attachment.", __func__);
		goto err_map_buf;
	}

	kref_init(&node->ref);
	return node;

err_map_buf:
	dma_buf_detach(node->buf, node->attachment);
err_attach_buf:
	dma_buf_put(node->buf);
err_get_buf:
	kfree(node);
	return ERR_PTR(-ENOENT);
}

struct sg_table* hicam_internal_get_sgtable(struct device *dev, int fd)
{
	struct sgtable_node *node = NULL;
	struct priv_ion_t *ion = dev_get_drvdata(dev);

	mutex_lock(&ion->sg_mutex);
	node = find_sgtable_node_by_fd(ion, fd);
	if (!IS_ERR(node)) {
		kref_get(&node->ref);
		mutex_unlock(&ion->sg_mutex);
		return node->sgt;
	}

	node = create_sgtable_node(ion, fd);
	if (!IS_ERR(node)) {
		list_add(&node->list, &ion->sg_nodes);
		mutex_unlock(&ion->sg_mutex);
		return node->sgt;
	}
	mutex_unlock(&ion->sg_mutex);
	return ERR_PTR(-ENODEV);
}

static void hicam_sgtable_deletor(struct kref *ref)
{
	struct sgtable_node *node = container_of(ref, struct sgtable_node, ref);

	/* release sgtable things we saved. */
	dma_buf_unmap_attachment(node->attachment, node->sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(node->buf, node->attachment);
	dma_buf_put(node->buf);

	list_del(&node->list);
	kfree(node);
}

void hicam_internal_put_sgtable(struct device *dev, struct sg_table *sgt)
{
	struct sgtable_node *node = NULL;
	struct priv_ion_t *ion = dev_get_drvdata(dev);
	mutex_lock(&ion->sg_mutex);
	node = find_sgtable_node_by_sg(ion, sgt);
	if (IS_ERR(node)) {
		cam_err("%s: putting non-exist sg_table:%pK.", __func__, sgt);
		goto err_out;
	}
	kref_put(&node->ref, hicam_sgtable_deletor);
err_out:
	mutex_unlock(&ion->sg_mutex);
}

int hicam_internal_get_phys(struct device *dev, int fd, struct phys_format *fmt)
{
	struct sg_table *sgt = hicam_internal_get_sgtable(dev, fd);
	if (IS_ERR(sgt)) {
		return PTR_ERR(sgt);
	}

	fmt->phys = sg_phys(sgt->sgl);
	hicam_internal_put_sgtable(dev, sgt);
	return 0;
}

phys_addr_t hicam_internal_get_pgd_base(struct device *dev)
{
	return hisi_domain_get_ttbr(dev);
}

int hicam_internal_sync(int fd, struct sync_format *fmt)
{
	int ret;
	struct dma_buf *dmabuf;

	dmabuf = dma_buf_get(fd);
	if (unlikely(IS_ERR(dmabuf))) {
		cam_err("%s: fail to get dmabuf.", __func__);
		return -EINVAL;
	}

	if (fmt->direction & HICAM_BUF_SYNC_READ) {
		/* invalidate transfers ownership of memory to cpu,
		 * means the begin of cpu access. */
		ret = dma_buf_begin_cpu_access(dmabuf, DMA_FROM_DEVICE);
	} else if (fmt->direction & HICAM_BUF_SYNC_WRITE) {
		/* clean transfers ownership of memory to device,
		 * means the end of cpu access. */
		ret = dma_buf_end_cpu_access(dmabuf, DMA_TO_DEVICE);
	} else {
		ret = -EINVAL;
	}

	dma_buf_put(dmabuf);
	return ret;
}

int hicam_internal_local_sync(int fd, struct local_sync_format *fmt)
{
	int rc, cmd;
	struct dma_buf *dmabuf;

	dmabuf = dma_buf_get(fd);
	if (unlikely(IS_ERR(dmabuf))) {
		cam_err("%s: fail to get dmabuf.", __func__);
		return -EINVAL;
	}
	cmd = fmt->direction == HICAM_BUF_SYNC_READ ?
		ION_HISI_INV_CACHES : ION_HISI_CLEAN_CACHES;
	rc = hisi_ion_cache_operate(fd, fmt->apva, fmt->offset, fmt->length, cmd);
	if (rc < 0) {
		cam_err("%s: sync failed, rc:%d", __func__, rc);
	}

	dma_buf_put(dmabuf);
	return 0;
}

void hicam_internal_dump_debug_info(struct device *dev)
{
	struct sgtable_node *node = NULL;
	struct priv_ion_t *ion = dev_get_drvdata(dev);
	cam_info("%s: dumping.....%pK", __func__, ion);

	list_for_each_entry(node, &ion->sg_nodes, list) {
		cam_info("%s: pending sg_table:%pK for dmabuf:%pK",
				__func__, node->sgt, node->buf);
	}
}

int hicam_internal_init(struct device *dev)
{
	struct priv_ion_t *ion = devm_kzalloc(dev,
			sizeof(struct priv_ion_t), GFP_KERNEL);
	if (!ion) {
		cam_err("%s: failed to allocate internal data.", __func__);
		return -ENOMEM;
	}

	ion->dev = dev;
	mutex_init(&ion->sg_mutex);
	INIT_LIST_HEAD(&ion->sg_nodes);
	dev_set_drvdata(dev, ion);
	return 0;
}

int hicam_internal_deinit(struct device *dev)
{
	struct priv_ion_t *ion = dev_get_drvdata(dev);
	if (!ion) {
		cam_err("%s: deinit before init.", __func__);
		return -EINVAL;
	}

	hicam_internal_dump_debug_info(ion->dev);
	devm_kfree(dev, ion);
	return 0;
}
