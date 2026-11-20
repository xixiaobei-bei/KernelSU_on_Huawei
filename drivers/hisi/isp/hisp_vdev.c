/*
 *  driver, hisp_vdev.c
 *
 * Copyright (c) 2013- ISP Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/scatterlist.h>
#include <linux/hisi-iommu.h>

/*
 * hisp viring alloc struct
 */
#ifdef CONFIG_HISP_REMOTEPROC_DMAALLOC_DEBUG
#define RPROC_NAME_SIZE         32

enum hisp_vring_e {
	VRING0          = 0x0,
	VRING1,
	VIRTQUEUE,
};
struct hisp_vring_s {
	u64 paddr;
	void *virt_addr;
	size_t size;
	u32 da;
};

struct hisp_virtio_dev {
	struct hisp_vring_s hisp_vring[HISP_VRING_NUM];
};

static struct hisp_virtio_dev g_hisp_vdev;

/*lint -save -e429*/
static int rpmsg_vdev_map_resource_sec(void)
{
	struct hisp_virtio_dev *vdev = &g_hisp_vdev;
	struct hisp_vring_s *hisp_vring = NULL;
	unsigned int i;
	int ret;

	if (sec_process_use_ca_ta())
		return 0;

	/* map vring */
	for (i = 0; i < VIRTQUEUE; i++) {
		hisp_vring = &vdev->hisp_vring[i];
		ret = hisp_mem_type_pa_init(A7VRING0 + i, hisp_vring->paddr);
		if (ret < 0) {
			pr_err("[%s] Failed : hisp_meminit.%d.(0x%x)\n",
				__func__, ret, A7VRING0 + i);
			return ret;
		}
	}

	/* map virtqueue */
	hisp_vring = &vdev->hisp_vring[i];
	ret = hisp_mem_type_pa_init(A7VQ, hisp_vring->paddr);
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_meminit.%d.(0x%x)\n",
			__func__, ret, A7VQ);
		return ret;
	}

	return 0;
}

static int rpmsg_vdev_map_resource_nonsec(struct rproc *rproc)
{
	struct hisp_virtio_dev *vdev = &g_hisp_vdev;
	struct hisp_vring_s *hisp_vring = NULL;
	struct rproc_mem_entry *mapping = NULL;
	struct device *subdev = NULL;
	unsigned int i;
	int size, ret, unmaped;

	if ((rproc == NULL) || rproc->domain == NULL) {
		pr_err("%s: rproc or domain is NULL\n", __func__);
		return -EINVAL;
	}

	subdev = get_isp_device();
	if (subdev == NULL) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < HISP_VRING_NUM; i++) {
		hisp_vring = &vdev->hisp_vring[i];
		size = PAGE_ALIGN(hisp_vring->size);/*lint !e666 */
		ret = iommu_map(rproc->domain, hisp_vring->da,
			hisp_vring->paddr, size, IOMMU_READ | IOMMU_WRITE);
		if (ret) {
			pr_err("[%s] Failed : iommu_map.%d\n", __func__, ret);
			return ret;
		}

		mapping = kzalloc(sizeof(*mapping), GFP_KERNEL);
		if (!mapping) {
			if (hisp_get_smmuc3_flag() == 1) {
				unmaped = hisi_iommu_unmap_fast(subdev,
						hisp_vring->da, size);
			} else {
				unmaped = iommu_unmap(rproc->domain,
						hisp_vring->da, size);
			}
			if (unmaped != size)
				pr_err("[%s] Failed : size.%u, unmaped.%u\n",
					__func__, size, unmaped);
			ret = -ENOMEM;
			return ret;
		}

		mapping->da = hisp_vring->da;
		mapping->len = size;
		list_add_tail(&mapping->node, &rproc->mappings);
	}

	return 0;
}

int rpmsg_vdev_map_resource(struct rproc *rproc)
{
	int ret = 0;

	if (rproc == NULL) {
		pr_err("%s: rproc is NULL\n", __func__);
		return -EINVAL;
	}

	if (use_sec_isp())
		return rpmsg_vdev_map_resource_sec();

	ret = rpmsg_vdev_map_resource_nonsec(rproc);
	if (ret < 0)
		pr_err("[%s] Failed: rpmsg_vdev_map_resource_nonsec.%d\n",
			__func__, ret);

	return ret;
}
/*lint -restore */

void *get_vring_dma_addr(u64 *dma_handle, size_t size, unsigned int index)
{
	struct hisp_virtio_dev *rproc_dev = &g_hisp_vdev;
	struct hisp_vring_s *hisp_vring = NULL;

	pr_info("[%s] : +\n", __func__);
	if (rproc_dev == NULL) {
		pr_err("%s: rproc_boot_device in NULL\n", __func__);
		return NULL;
	}

	hisp_vring = &rproc_dev->hisp_vring[index];
	if (hisp_vring == NULL) {
		pr_err("%s: hisp_vring is NULL\n", __func__);
		return NULL;
	}

	if (hisp_vring->size != size) {
		pr_err("%s: hisp_vring size not same: 0x%lx --> 0x%lx\n",
				__func__, hisp_vring->size, size);
		return NULL;
	}

	*dma_handle = hisp_vring->paddr;
	pr_info("[%s] : -\n", __func__);
	return hisp_vring->virt_addr;
}


static u32 hisp_vring_da_get(int type)
{
	unsigned int etype = 0;

	switch (type) {
	case VRING0:
		if (!sec_process_use_ca_ta())
			etype = A7VRING0;
		else
			etype = HISP_SEC_VR0;
		break;
	case VRING1:
		if (!sec_process_use_ca_ta())
			etype = A7VRING1;
		else
			etype = HISP_SEC_VR1;
		break;
	case VIRTQUEUE:
		if (!sec_process_use_ca_ta())
			etype = A7VQ;
		else
			etype = HISP_SEC_VQ;
		break;
	default:
		pr_err("%s: type is out of bound\n", __func__);
		return 0;
	}

	return hisp_get_hisp_vring_da(etype);
}

int get_vring_dma_addr_probe(struct platform_device *pdev)
{
	struct hisp_virtio_dev *rproc_dev = &g_hisp_vdev;
	struct hisp_vring_s *hisp_vring = NULL;
	int ret = 0;
	int i = 0;

	if (pdev == NULL) {
		pr_err("%s: rproc_boot_device in NULL\n", __func__);
		return -ENOMEM;
	}

	for (i = 0; i < HISP_VRING_NUM; i++) {
		hisp_vring = &rproc_dev->hisp_vring[i];
		if (hisp_vring == NULL) {
			pr_err("%s: hisp_vring is NULL\n", __func__);
			ret = -ENOMEM;
			goto out;
		}

		if (i < (HISP_VRING_NUM - 1))
			hisp_vring->size =  HISP_VRING_SIZE;
		else
			hisp_vring->size =  HISP_VQUEUE_SIZE;

		hisp_vring->virt_addr = dma_alloc_coherent(&pdev->dev,
			hisp_vring->size, &hisp_vring->paddr,
			GFP_KERNEL);
		if (!hisp_vring->virt_addr) {
			pr_err("%s: hisp_vring is NULL\n", __func__);
			ret = -ENOMEM;
			goto out;
		}

		hisp_vring->da = hisp_vring_da_get(i);
	}
	return 0;

out:
	for (; i >= 0; i--) {
		if (i >= HISP_VRING_NUM)
			continue;
		hisp_vring = &rproc_dev->hisp_vring[i];
		if (hisp_vring == NULL)
			continue;

		dma_free_coherent(&pdev->dev, hisp_vring->size,
			hisp_vring->virt_addr, hisp_vring->paddr);
	}

	return ret;
}

int get_vring_dma_addr_remove(struct platform_device *pdev)
{
	struct hisp_virtio_dev *rproc_dev = &g_hisp_vdev;
	struct hisp_vring_s *hisp_vring = NULL;
	unsigned int i = 0;

	for (i = 0; i < HISP_VRING_NUM; i++) {
		hisp_vring = &rproc_dev->hisp_vring[i];
		if (hisp_vring == NULL) {
			pr_err("%s: hisp_vring is NULL\n", __func__);
			return -ENOMEM;
		}

		dma_free_coherent(&pdev->dev, hisp_vring->size,
			hisp_vring->virt_addr, hisp_vring->paddr);
	}
	return 0;
}

bool rpmsg_mem_alloc_beforehand(struct virtio_device *vdev)
{
	struct rproc_vdev *rvdev = NULL;
	struct rproc *rproc = NULL;

	if (vdev == NULL) {
		pr_err("%s: vdev is NULL\n", __func__);
		return false;
	}

	rvdev = container_of(vdev, struct rproc_vdev, vdev);
	rproc = rvdev->rproc;

	if (strncmp(rproc->name, "ISPCPU", RPROC_NAME_SIZE))
		return false;

	return true;
}

bool rproc_mem_alloc_beforehand(struct rproc *rproc)
{
	if (rproc == NULL) {
		pr_err("%s: rproc is NULL\n", __func__);
		return false;
	}

	if (strncmp(rproc->name, "ISPCPU", RPROC_NAME_SIZE))
		return false;

	return true;
}
#endif

