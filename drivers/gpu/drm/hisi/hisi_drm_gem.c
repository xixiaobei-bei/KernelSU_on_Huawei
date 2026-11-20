/*
 * hisi_drm_gem.c
 *
 * Implement DRM GEM feature
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
/*lint -e559 -e574*/
#include "hisi_defs.h"
#include "hisi_drm_gem.h"

#include <linux/hisi-iommu.h>
#include <drm/drmP.h>
#include <drm/drm_prime.h>
#include <hisi_drm_drv.h>
#include "hisi_drm.h"
#include "hisi_dss_ion.h"
#include "hisi_drm_gem_heap_helper.h"

static int hisi_drm_alloc_buf(struct hisi_drm_gem *hisi_gem)
{
	struct drm_device *dev = NULL;
	unsigned long buf_size = 0;
	int ret;

	if (!hisi_gem) {
		HISI_DRM_ERR("hisi_gem is nullptr!");
		return -EINVAL;
	}

	dev = hisi_gem->base.dev;
	if (!dev) {
		HISI_DRM_ERR("dev is nullptr!");
		return -EINVAL;
	}

	ret = hisi_drm_gem_heap_alloc(hisi_gem);
	if (ret) {
		HISI_DRM_ERR(" alloc buf failed\n");
		return ret;
	}

	if (!hisi_gem->sgt) {
		HISI_DRM_ERR("failed to alloc hisifb dma memory!");
		return -ENOMEM;
	}

	HISI_DRM_DEBUG("alloc drm bufsuccess, size=0x%lx\n", hisi_gem->size);

	if (!(hisi_gem->flags & HISI_BO_ALLOC_WITH_MAP_IOMMU))
		return 0;

	hisi_gem->iova = hisi_iommu_map_sg(to_dma_dev(dev), hisi_gem->sgt->sgl,
					   0, &buf_size);
	if (!hisi_gem->iova) {
		HISI_DRM_ERR("failed to iommu map sg!");
		hisi_drm_gem_heap_free(hisi_gem);
		return -ENOMEM;
	}

	HISI_DRM_DEBUG("success to map iova\n");

	return 0;
}

static struct hisi_drm_gem *hisi_drm_gem_init(struct drm_device *dev,
					      unsigned long size)
{
	struct hisi_drm_gem *hisi_gem = NULL;
	struct drm_gem_object *obj = NULL;

	hisi_gem = kzalloc(sizeof(*hisi_gem), GFP_KERNEL);
	if (!hisi_gem) {
		HISI_DRM_ERR("failed to alloc hisi_gem!");
		return ERR_PTR(-ENOMEM);
	}

	hisi_gem->size = size;
	obj = &hisi_gem->base;

	mutex_init(&hisi_gem->hisi_gem_lock);

	drm_gem_private_object_init(dev, obj, size);

	return hisi_gem;
}

struct hisi_drm_gem *hisi_drm_gem_create(struct drm_device *dev,
					 unsigned int flags, unsigned long size)
{
	int ret;
	struct hisi_drm_gem *hisi_gem = NULL;

	if (!dev) {
		HISI_DRM_ERR("dev is nullptr!");
		return ERR_PTR(-ENODEV);
	}

	if (!size) {
		HISI_DRM_ERR("invalid GEM buffer size: %lu!", size);
		return ERR_PTR(-EINVAL);
	}

	size = roundup(size, PAGE_SIZE);

	hisi_gem = hisi_drm_gem_init(dev, size);
	if (IS_ERR(hisi_gem)) {
		HISI_DRM_ERR("hisi_gem is nullptr!");
		return hisi_gem;
	}

	/* set memory type and cache attribute from user side. */
	hisi_gem->flags = flags;

	hisi_gem->heap_id =
		hisi_drm_gem_parse_flags_to_heap_id(hisi_gem->flags);
	if (!hisi_drm_gem_check_flags(hisi_gem))
		return ERR_PTR(-EINVAL);

	ret = hisi_drm_alloc_buf(hisi_gem);
	if (ret) {
		hisi_drm_gem_heap_free(hisi_gem);
		drm_gem_object_release(&hisi_gem->base);
		kfree(hisi_gem);
		return ERR_PTR(ret);
	}

	return hisi_gem;
}

static void hisi_drm_free_buf(struct hisi_drm_gem *hisi_gem)
{
	struct drm_device *dev = NULL;

	if (!hisi_gem) {
		HISI_DRM_ERR("hisi_gem is nullptr!");
		return;
	}

	dev = hisi_gem->base.dev;
	if (!dev) {
		HISI_DRM_ERR("invalid args for no device");
		return;
	}

	HISI_DRM_DEBUG("free size(0x%lx)\n", hisi_gem->size);
	hisi_drm_gem_heap_free(hisi_gem);
}

static int hisi_drm_gem_handle_create(struct drm_gem_object *obj,
				      struct drm_file *file_priv,
				      unsigned int *handle)
{
	int ret;

	/*
	 * allocate a id of idr table where the obj is registered
	 * and handle has the id what user can see.
	 */
	ret = drm_gem_handle_create(file_priv, obj, handle);
	if (ret) {
		HISI_DRM_ERR("failed to drm_gem_handle_create! ret=%d!", ret);
		return ret;
	}

	/* drop reference from allocate - handle holds it now. */
	drm_gem_object_unreference_unlocked(obj);

	return 0;
}

void hisi_drm_gem_destroy(struct hisi_drm_gem *hisi_gem)
{
	struct drm_gem_object *obj = NULL;

	if (!hisi_gem) {
		HISI_DRM_ERR("hisi_gem is nullptr!");
		return;
	}

	obj = &hisi_gem->base;
	if (!obj) {
		HISI_DRM_ERR("obj is nullptr!");
		return;
	}

	/*
	 * do not release memory region from exporter.
	 *
	 * the region will be released by exporter
	 * once dmabuf's refcount becomes 0.
	 */
	if (obj->import_attach)
		drm_prime_gem_destroy(obj, hisi_gem->sgt);
	else
		hisi_drm_free_buf(hisi_gem);

	/* release file pointer to gem object. */
	drm_gem_object_release(obj);

	kfree(hisi_gem);
}

void hisi_drm_gem_free_object(struct drm_gem_object *obj)
{
	hisi_drm_gem_destroy(to_hisi_gem(obj));
}

int hisi_drm_gem_dumb_create(struct drm_file *file_priv, struct drm_device *dev,
			     struct drm_mode_create_dumb *args)
{
	int ret;
	struct hisi_drm_gem *hisi_gem = NULL;
	unsigned int flags = 0;

	/*
	 * allocate memory to be used for framebuffer.
	 * - this callback would be called by user application
	 *	with DRM_IOCTL_MODE_CREATE_DUMB command.
	 */

	args->pitch = args->width * ((args->bpp + 7) / 8);
	args->size = ((__u64)args->pitch) * args->height;

	/* we are used flag's high 8 bit */
	flags = HISI_BO_NONCONTIG | HISI_BO_WC | HISI_BO_NONCACHABLE |
		args->flags;

	hisi_gem = hisi_drm_gem_create(dev, flags, args->size);
	if (IS_ERR(hisi_gem)) {
		HISI_DRM_ERR("failed to create hisi drm gem!");
		return PTR_ERR(hisi_gem);
	}

	ret = hisi_drm_gem_handle_create(&hisi_gem->base, file_priv,
					 &args->handle);
	if (ret) {
		HISI_DRM_ERR("failed to create hisi drm gem! ret = %d\n", ret);
		hisi_drm_gem_destroy(hisi_gem);
		return ret;
	}

	return 0;
}

struct drm_gem_object *hisi_drm_gem_prime_import(struct drm_device *dev,
						 struct dma_buf *dma_buf)
{
	struct hisi_drm_private *priv = NULL;

	if (!dev) {
		HISI_DRM_ERR("dev is nullptr!");
		return NULL;
	}

	priv = dev->dev_private;
	if (!priv) {
		HISI_DRM_ERR("priv is nullptr!");
		return NULL;
	}

	return drm_gem_prime_import_dev(dev, dma_buf, priv->iommu_dev);
}

static int hisi_drm_gem_mmap_obj(struct drm_gem_object *obj,
				 struct vm_area_struct *vma)
{
	int ret;
	struct hisi_drm_gem *hisi_gem = NULL;

	if (!obj) {
		HISI_DRM_ERR("obj is nullptr!");
		return -EINVAL;
	}

	hisi_gem = to_hisi_gem(obj);
	if (!hisi_gem) {
		HISI_DRM_ERR("hisi_gem is nullptr!");
		return -EINVAL;
	}

	ret = hisi_drm_gem_heap_mmap(hisi_gem, vma);
	if (ret) {
		HISI_DRM_ERR("failed to drm_gem_mmap_buffer! ret=%d!", ret);
		goto err_close_vm;
	}

	return 0;

err_close_vm:
	drm_gem_vm_close(vma);

	return ret;
}

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	int ret;
	int i;
	struct sg_table *new_table = NULL;
	struct scatterlist *sg = NULL;
	struct scatterlist *new_sg = NULL;

	if (!table) {
		HISI_DRM_ERR("table is nullptr!");
		return NULL;
	}

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table) {
		HISI_DRM_ERR("new_table is nullptr!");
		return ERR_PTR(-ENOMEM);
	}

	ret = sg_alloc_table(new_table, table->nents, GFP_KERNEL);
	if (ret) {
		HISI_DRM_ERR("failed to sg_alloc_table! ret=%d!", ret);
		kfree(new_table);
		return ERR_PTR(-ENOMEM);
	}

	new_sg = new_table->sgl;
	for_each_sg (table->sgl, sg, table->nents, i) {
		memcpy(new_sg, sg, sizeof(*sg));
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

struct sg_table *hisi_drm_gem_prime_get_sg_table(struct drm_gem_object *obj)
{
	struct hisi_drm_gem *hisi_gem = NULL;

	if (!obj) {
		HISI_DRM_ERR("obj is nullptr!");
		return NULL;
	}

	hisi_gem = to_hisi_gem(obj);
	if (!hisi_gem) {
		HISI_DRM_ERR("hisi_gem is nullptr!");
		return NULL;
	}

	if (!hisi_gem->sgt) {
		HISI_DRM_ERR("sgt is nullptr!");
		return NULL;
	}

	return dup_sg_table(hisi_gem->sgt);
}

struct drm_gem_object *
hisi_drm_gem_prime_import_sg_table(struct drm_device *dev,
				   struct dma_buf_attachment *attach,
				   struct sg_table *sgt)
{
	struct hisi_drm_gem *hisi_gem = NULL;
	unsigned long buf_size = 0;
	int ret;

	if (!dev) {
		HISI_DRM_ERR("dev is nullptr!");
		return NULL;
	}

	if (!attach) {
		HISI_DRM_ERR("attach is nullptr!");
		return NULL;
	}

	if (!sgt) {
		HISI_DRM_ERR("sgt is nullptr!");
		return NULL;
	}

	hisi_gem = hisi_drm_gem_init(dev, attach->dmabuf->size);
	if (!hisi_gem) {
		HISI_DRM_ERR("failed to hisi_drm_gem_init!");
		return NULL;
	}

	hisi_gem->sgt = sgt;

	hisi_gem->iova = hisi_iommu_map_dmabuf(to_dma_dev(dev), attach->dmabuf,
					       0, &buf_size);
	if ((hisi_gem->iova == 0) || (buf_size < hisi_gem->size)) {
		if (hisi_gem->iova) {
			ret = hisi_iommu_unmap_dmabuf(to_dma_dev(dev),
						      attach->dmabuf,
						      hisi_gem->iova);
			if (ret)
				HISI_DRM_ERR("failed to unmap dmabuf\n");
			hisi_gem->iova = 0;
		}

		HISI_DRM_ERR("failed to map iova! size=%#lx\n",
			     attach->dmabuf->size);
	}

	return &hisi_gem->base;
}

void *hisi_drm_gem_prime_vmap(struct drm_gem_object *obj)
{
	UNUSED(obj);

	return NULL;
}

void hisi_drm_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr)
{
	UNUSED(obj);
	UNUSED(vaddr);
}

int hisi_drm_gem_prime_mmap(struct drm_gem_object *obj,
			    struct vm_area_struct *vma)
{
	int ret;
	struct hisi_drm_gem *hisi_gem = NULL;

	if (!obj) {
		HISI_DRM_ERR("obj is nullptr!");
		return -EINVAL;
	}

	if (!vma) {
		HISI_DRM_ERR("vma is nullptr!");
		return -EINVAL;
	}

	hisi_gem = to_hisi_gem(obj);
	if (!hisi_gem) {
		HISI_DRM_ERR("hisi_gem is nullptr!");
		return -EINVAL;
	}

	ret = drm_gem_mmap_obj(obj, obj->size, vma);
	if (ret < 0) {
		HISI_DRM_ERR("failed to drm_gem_mmap_obj!");
		return ret;
	}

	return hisi_drm_gem_mmap_obj(obj, vma);
}

/**
 * used by user space mmap with drm_file. need user use MAP_DUMB ioctl to earn
 * fake offset, so we can use this offset find the drm_gem_object.
 */
int hisi_drm_gem_mmap(struct file *file_priv, struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = NULL;
	int ret;

	/* set vm_area_struct. */
	ret = drm_gem_mmap(file_priv, vma);
	if (ret < 0) {
		DRM_ERROR("failed to mmap.\n");
		return ret;
	}

	obj = vma->vm_private_data;
	if (obj->import_attach)
		return dma_buf_mmap(obj->dma_buf, vma, 0);

	vma->vm_pgoff = 0;

	return hisi_drm_gem_mmap_obj(obj, vma);
}

int hisi_drm_gem_create_ioctl(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	int ret;
	struct drm_hisi_gem_create *args = NULL;
	struct hisi_drm_gem *hisi_gem = NULL;

	if (!dev) {
		HISI_DRM_ERR("dev is nullptr!");
		return -EINVAL;
	}

	if (!data) {
		HISI_DRM_ERR("data is nullptr!");
		return -EINVAL;
	}

	if (!file_priv) {
		HISI_DRM_ERR("file_priv is nullptr!");
		return -EINVAL;
	}

	args = (struct drm_hisi_gem_create *)data;
	hisi_gem = hisi_drm_gem_create(dev, args->flags, args->size);
	if (IS_ERR(hisi_gem)) {
		HISI_DRM_ERR("failed to drm gem create!");
		return PTR_ERR(hisi_gem);
	}

	ret = hisi_drm_gem_handle_create(&hisi_gem->base, file_priv,
					 &args->handle);
	if (ret) {
		HISI_DRM_ERR("failed to drm_gem_handle_create! ret=%d!", ret);
		hisi_drm_gem_destroy(hisi_gem);
		return ret;
	}

	return 0;
}

/*lint +e559 +e574*/
