/*
 * HISI drm dmabuf ops implement
 *
 * Copyright (C) 2019 Hisilicon. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*lint -e446 -e546 -e548 -e574 -e580*/

#include <linux/export.h>
#include <linux/dma-buf.h>
#include <linux/rbtree.h>
#include <drm/drm_prime.h>
#include <drm/drm_gem.h>
#include <drm/drmP.h>

#include "hisi_gem_dmabuf.h"
#include "hisi_drm_gem.h"
#include "hisi_defs.h"
#include "hisi_drm_gem_heap_helper.h"
#include "hisi_drm_drv.h"

struct drm_prime_attachment {
	struct sg_table *sgt;
	enum dma_data_direction dir;
};

static struct sg_table *hisi_drm_gem_map_dma_buf(struct dma_buf_attachment
						 *attach,
						 enum dma_data_direction dir)
{
	struct drm_prime_attachment *prime_attach = attach->priv;
	struct drm_gem_object *obj = attach->dmabuf->priv;
	struct sg_table *sgt = NULL;

	if (dir == DMA_NONE || !prime_attach)
		return ERR_PTR(-EINVAL);

	/* return the cached mapping when possible */
	if (prime_attach->dir == dir)
		return prime_attach->sgt;

	/*
	 * two mappings with different directions for the same attachment are
	 * not allowed
	 */
	if (prime_attach->dir != DMA_NONE)
		return ERR_PTR(-EBUSY);

	sgt = obj->dev->driver->gem_prime_get_sg_table(obj);

	if (!IS_ERR(sgt)) {
		if (!dma_map_sg(attach->dev, sgt->sgl, sgt->nents, dir)) {
			sg_free_table(sgt);
			kfree(sgt);
			sgt = ERR_PTR(-ENOMEM);
		} else {
			prime_attach->sgt = sgt;
			prime_attach->dir = dir;
		}
	}

	return sgt;
}

void hisi_drm_gem_dmabuf_release(struct dma_buf *dma_buf)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	/* drop the reference on the export fd holds */
	drm_gem_object_put_unlocked(obj);

	drm_dev_unref(dev);
}
EXPORT_SYMBOL(hisi_drm_gem_dmabuf_release);

static void *hisi_drm_gem_dmabuf_kmap(struct dma_buf *dma_buf,
				      unsigned long page_num)
{
	void *vaddr = NULL;
	struct hisi_drm_gem *hisi_gem = to_hisi_gem(dma_buf->priv);

	vaddr = hisi_drm_gem_heap_kmap(hisi_gem);
	if (IS_ERR_OR_NULL(vaddr))
		return NULL;

	return vaddr + page_num * PAGE_SIZE;
}

static void hisi_drm_gem_dmabuf_kunmap(struct dma_buf *dma_buf,
				       unsigned long page_num, void *addr)
{
	struct hisi_drm_gem *hisi_gem = to_hisi_gem(dma_buf->priv);

	hisi_drm_gem_heap_kunmap(hisi_gem, addr);
}

static int hisi_drm_gem_dmabuf_mmap(struct dma_buf *dma_buf,
				    struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	if (!dev->driver->gem_prime_mmap)
		return -ENOENT;

	return dev->driver->gem_prime_mmap(obj, vma);
}

static void *hisi_drm_gem_dmabuf_vmap(struct dma_buf *dma_buf)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	return dev->driver->gem_prime_vmap(obj);
}

static void hisi_drm_gem_dmabuf_vunmap(struct dma_buf *dma_buf, void *vaddr)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	dev->driver->gem_prime_vunmap(obj, vaddr);
}

static void hisi_drm_gem_map_detach(struct dma_buf *dma_buf,
				    struct dma_buf_attachment *attach)
{
	struct drm_prime_attachment *prime_attach = attach->priv;
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;
	struct sg_table *sgt = NULL;

	if (dev->driver->gem_prime_unpin)
		dev->driver->gem_prime_unpin(obj);

	if (!prime_attach)
		return;

	sgt = prime_attach->sgt;
	if (sgt) {
		if (prime_attach->dir != DMA_NONE)
			dma_unmap_sg(attach->dev, sgt->sgl, sgt->nents,
				     prime_attach->dir);

		sg_free_table(sgt);
	}

	kfree(sgt);
	kfree(prime_attach);
	attach->priv = NULL;
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
static void *hisi_drm_gem_dmabuf_kmap_atomic(struct dma_buf *dma_buf,
					     unsigned long page_num)
{
	UNUSED(dma_buf);
	UNUSED(page_num);

	return NULL;
}

static void hisi_drm_gem_dmabuf_kunmap_atomic(struct dma_buf *dma_buf,
					      unsigned long page_num,
					      void *addr)
{
	UNUSED(dma_buf);
	UNUSED(page_num);
	UNUSED(addr);
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
static int hisi_drm_gem_map_attach(struct dma_buf *dma_buf,
				struct device *target_dev,
				struct dma_buf_attachment *attach)
#else
static int hisi_drm_gem_map_attach(struct dma_buf *dma_buf,
				struct dma_buf_attachment *attach)
#endif
{
	struct drm_prime_attachment *prime_attach;
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	prime_attach = kzalloc(sizeof(*prime_attach), GFP_KERNEL);
	if (!prime_attach)
		return -ENOMEM;

	prime_attach->dir = DMA_NONE;
	attach->priv = prime_attach;

	if (!dev->driver->gem_prime_pin)
		return 0;

	return dev->driver->gem_prime_pin(obj);
}

static void hisi_drm_gem_unmap_dma_buf(struct dma_buf_attachment *attach,
				       struct sg_table *sgt,
				       enum dma_data_direction dir)
{
	UNUSED(attach);
	UNUSED(sgt);
	UNUSED(dir);
}

static int
hisi_drm_gem_dmabuf_begin_cpu_access(struct dma_buf *dmabuf,
				     enum dma_data_direction direction)
{
	struct drm_gem_object *obj = dmabuf->priv;
	struct hisi_drm_gem *hisi_gem = to_hisi_gem(obj);
	struct drm_device *drm_drv = obj->dev;
	struct sg_table *table = hisi_gem->sgt;

	mutex_lock(&hisi_gem->hisi_gem_lock);
	dma_sync_sg_for_cpu(to_dma_dev(drm_drv), table->sgl, table->nents,
			    direction);
	mutex_unlock(&hisi_gem->hisi_gem_lock);

	return 0;
}

static int hisi_drm_gem_dmabuf_end_cpu_access(struct dma_buf *dmabuf,
					      enum dma_data_direction direction)
{
	struct drm_gem_object *obj = dmabuf->priv;
	struct hisi_drm_gem *hisi_gem = to_hisi_gem(obj);
	struct drm_device *drm_drv = obj->dev;
	struct sg_table *table = hisi_gem->sgt;

	mutex_lock(&hisi_gem->hisi_gem_lock);
	dma_sync_sg_for_device(to_dma_dev(drm_drv), table->sgl, table->nents,
			       direction);
	mutex_unlock(&hisi_gem->hisi_gem_lock);

	return 0;
}

static const struct dma_buf_ops hisi_drm_gem_prime_dmabuf_ops = {
	.attach = hisi_drm_gem_map_attach,
	.detach = hisi_drm_gem_map_detach,
	.map_dma_buf = hisi_drm_gem_map_dma_buf,
	.unmap_dma_buf = hisi_drm_gem_unmap_dma_buf,
	.release = hisi_drm_gem_dmabuf_release,
	.map = hisi_drm_gem_dmabuf_kmap,
	.unmap = hisi_drm_gem_dmabuf_kunmap,
	.mmap = hisi_drm_gem_dmabuf_mmap,
	.vmap = hisi_drm_gem_dmabuf_vmap,
	.vunmap = hisi_drm_gem_dmabuf_vunmap,
	.begin_cpu_access = hisi_drm_gem_dmabuf_begin_cpu_access,
	.end_cpu_access = hisi_drm_gem_dmabuf_end_cpu_access,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	.map_atomic = hisi_drm_gem_dmabuf_kmap_atomic,
	.unmap_atomic = hisi_drm_gem_dmabuf_kunmap_atomic,
#endif
};

struct drm_gem_object *hisi_drm_gem_prime_import_dev(struct drm_device *dev,
						     struct dma_buf *dma_buf,
						     struct device *attach_dev)
{
	struct dma_buf_attachment *attach = NULL;
	struct sg_table *sgt = NULL;
	struct drm_gem_object *obj = NULL;
	int ret;

	if (dma_buf->ops == &hisi_drm_gem_prime_dmabuf_ops) {
		obj = dma_buf->priv;
		if (obj->dev == dev) {
			/*
			 * Importing dmabuf exported from out own gem increases
			 * refcount on gem itself instead of f_count of dmabuf.
			 */
			drm_gem_object_get(obj);
			return obj;
		}
	}

	if (!dev->driver->gem_prime_import_sg_table)
		return ERR_PTR(-EINVAL);

	attach = dma_buf_attach(dma_buf, attach_dev);
	if (IS_ERR(attach))
		return ERR_CAST(attach);

	get_dma_buf(dma_buf);

	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		ret = PTR_ERR(sgt);
		goto fail_detach;
	}

	obj = dev->driver->gem_prime_import_sg_table(dev, attach, sgt);
	if (IS_ERR(obj)) {
		ret = PTR_ERR(obj);
		goto fail_unmap;
	}

	obj->import_attach = attach;

	return obj;

fail_unmap:
	dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
fail_detach:
	dma_buf_detach(dma_buf, attach);
	dma_buf_put(dma_buf);

	return ERR_PTR(ret);
}

struct dma_buf *hisi_gem_dmabuf_prime_export(struct drm_device *dev,
					     struct drm_gem_object *obj,
					     int flags)
{
	struct dma_buf_export_info exp_info = {
		.exp_name = KBUILD_MODNAME,	/* white lie for debug */
		.owner = dev->driver->fops->owner,
		.ops = &hisi_drm_gem_prime_dmabuf_ops,
		.size = obj->size,
		.flags = flags,
		.priv = obj,
	};

	return drm_gem_dmabuf_export(dev, &exp_info);
}

struct drm_gem_object *hisi_gem_dmabuf_prime_import(struct drm_device *dev,
						    struct dma_buf *dma_buf)
{
	return hisi_drm_gem_prime_import_dev(dev, dma_buf, dev->dev);
}

struct hisi_drm_gem *hisi_gem_dmabuf_to_gem(struct dma_buf *dmabuf)
{
	struct hisi_drm_gem *hisi_gem = NULL;

	if (!is_hisi_drm_dmabuf(dmabuf))
		return NULL;

	hisi_gem = to_hisi_gem(dmabuf->priv);
	return hisi_gem;
}

bool is_hisi_drm_dmabuf(struct dma_buf *dmabuf)
{
	if ((dmabuf != NULL) && (dmabuf->ops != NULL) &&
	    (dmabuf->ops == &hisi_drm_gem_prime_dmabuf_ops))
		return true;
	return false;
}

/*lint +e446 +e546 +e548 +e574 +e580*/
