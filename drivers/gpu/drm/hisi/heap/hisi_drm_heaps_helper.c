/*
 * hisi_drm_heaps_helper.c
 *
 * Provide a unified external interface for hisi drm manager
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_drm_heaps_helper.h"

#include <linux/err.h>
#include <linux/printk.h>
#include <linux/hisi-iommu.h>
#include <drm/drm_device.h>

#include "hisi_drm.h"
#include "hisi_drm_gem.h"
#include "hisi_drm_drv.h"
#include "hisi_drm_heaps_defs.h"
#include "hisi_drm_heaps_tracer_inner.h"

/* default alloc limit 1500ms */
static int hisi_drm_alloc_max_latency_us = 1500 * 1000;

module_param_named(hisi_drm_heap_alloc_limit_time_us,
		   hisi_drm_alloc_max_latency_us, int, 0640);
MODULE_PARM_DESC(hisi_drm_heap_alloc_limit_time_us,
		 "hisi drm heap alloc limit time, unit is us");

static int __do_alloc(struct hisi_drm_heap *heap,
		      struct hisi_drm_heap_context *ctx)
{
	int ret = heap->ops->alloc_buf(heap, ctx);

	if (ret)
		return ret;

	hisi_drm_heaps_gem_trace(ctx);
	return 0;
}

/*
 * Some task may exit without free kvaddr and iova, so we will check iova and
 * kvaddr, if not free, free it first. And then,  unregister trace node.
 * Finally, free memory by invoke free_buf hook func.
 */
static void __do_free(struct hisi_drm_heap *heap,
		      struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_gem *hisi_gem = ctx->priv;
	struct drm_device *dev = hisi_gem->base.dev;
	int ret;

	if (hisi_gem->vaddr) {
		pr_warn_once("%s the kmap vaddr is still in mapped\n",
			     __func__);

		heap->ops->unmap_kernel(heap, ctx, hisi_gem->vaddr);
		hisi_gem->vaddr = NULL;
	}

	if (hisi_gem->iova) {
		ret = hisi_iommu_unmap_sg(to_dma_dev(dev), hisi_gem->sgt->sgl,
					  hisi_gem->iova);
		if (ret) {
			pr_heaps_err("failed to unmap iova\n");
			hisi_drm_gem_info_show(hisi_gem);
		} else {
			hisi_gem->iova = 0;
		}
	}

	hisi_drm_heaps_remove_trace(ctx);

	heap->ops->free_buf(heap, ctx);
}

static void *hisi_drm_heaps_kmap_get(struct hisi_drm_heap *heap,
				     struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_gem *hisi_gem = ctx->priv;
	void *vaddr = NULL;

	mutex_lock(&hisi_gem->hisi_gem_lock);

	if (hisi_gem->kmap_cnt) {
		++hisi_gem->kmap_cnt;
		vaddr = hisi_gem->vaddr;
		goto get_and_unlock;
	}

	vaddr = heap->ops->map_kernel(heap, ctx);
	if (IS_ERR(vaddr)) {
		/* for dma_buf_kmap, error number is NULL */
		vaddr = NULL;
		goto err_unlock;
	}

	hisi_gem->kmap_cnt = 1;

get_and_unlock:
err_unlock:
	mutex_unlock(&hisi_gem->hisi_gem_lock);

	return vaddr;
}

static void hisi_drm_heaps_kmap_put(struct hisi_drm_heap *heap,
				    struct hisi_drm_heap_context *ctx,
				    const void *addr)
{
	struct hisi_drm_gem *hisi_gem = ctx->priv;

	mutex_lock(&hisi_gem->hisi_gem_lock);
	--hisi_gem->kmap_cnt;
	if (hisi_gem->kmap_cnt == 0) {
		heap->ops->unmap_kernel(heap, ctx, addr);
		hisi_gem->vaddr = NULL;
	}
	mutex_unlock(&hisi_gem->hisi_gem_lock);
}

static int __do_map_user(struct hisi_drm_heap *heap,
			 struct hisi_drm_heap_context *ctx,
			 struct vm_area_struct *vma)
{
	return heap->ops->map_user(heap, ctx, vma);
}

static void __set_dma_addr_for_sg(struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_gem *hisi_gem = ctx->priv;
	struct sg_table *sgt = hisi_gem->sgt;
	struct scatterlist *sgl = NULL;
	unsigned int i;

	for_each_sg (sgt->sgl, sgl, sgt->nents, i) {
		sg_dma_address(sgl) = sg_phys(sgl);
		sg_dma_len(sgl) = sgl->length;
	}
}

static void __invalid_cache_for_access(struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_gem *hisi_gem = ctx->priv;
	struct sg_table *sgt = hisi_gem->sgt;
	struct drm_device *drm_drv = hisi_gem->base.dev;

	dma_sync_sg_for_cpu(to_dma_dev(drm_drv), sgt->sgl, (int)sgt->nents,
			    DMA_FROM_DEVICE);
}

int hisi_drm_heap_alloc(struct hisi_drm_heap_context *ctx)
{
	ktime_t _stime, _etime;
	s64 _alloc_time;
	struct hisi_drm_heap *heap = NULL;
	unsigned int heap_id;
	int ret;

	if (!ctx || !ctx->priv)
		return -EINVAL;

	heap_id = ctx->head_id;
	heap = hisi_drm_get_heap(heap_id);
	if (IS_ERR_OR_NULL(heap)) {
		pr_heaps_err("can't find heap by id = %#x\n", heap_id);
		return PTR_ERR(heap);
	}

	_stime = ktime_get();
	ret = __do_alloc(heap, ctx);
	_etime = ktime_get();
	if (ret)
		return ret;

	_alloc_time = ktime_us_delta(_etime, _stime);
	if (_alloc_time > hisi_drm_alloc_max_latency_us)
		pr_heaps_warning(
			"alloc size = %#lx, heap_id = %#x, time cos=%lld us\n",
			ctx->size, ctx->head_id, _alloc_time);

	/* If we alloc without cache, we should invalid cache for it first. */
	__set_dma_addr_for_sg(ctx);
	if (!(ctx->flags & HISI_BO_CACHABLE))
		__invalid_cache_for_access(ctx);

	return 0;
}

void hisi_drm_heap_free(struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_heap *heap = NULL;
	unsigned int heap_id;

	if (!ctx || !ctx->priv) {
		pr_heaps_err("ctx is null\n");
		return;
	}

	heap_id = ctx->head_id;
	heap = hisi_drm_get_heap(heap_id);
	if (IS_ERR_OR_NULL(heap)) {
		pr_heaps_err("can't find heap by id = %#x\n", heap_id);
		return;
	}

	__do_free(heap, ctx);
}

void *hisi_drm_heap_map_kernel(struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_heap *heap = NULL;
	unsigned int heap_id;

	if (!ctx || !ctx->priv)
		return ERR_PTR(-EINVAL);

	heap_id = ctx->head_id;
	heap = hisi_drm_get_heap(heap_id);
	if (IS_ERR_OR_NULL(heap)) {
		pr_heaps_err("can't find heap by id = %#x\n", heap_id);
		return heap;
	}

	if (!heap->ops->map_kernel)
		return NULL;

	return hisi_drm_heaps_kmap_get(heap, ctx);
}

void hisi_drm_heap_unmap_kernel(struct hisi_drm_heap_context *ctx,
				const void *vaddr)
{
	struct hisi_drm_heap *heap = NULL;
	unsigned int heap_id;

	if (!ctx || !ctx->priv || !vaddr) {
		pr_heaps_err("ctx is null\n");
		return;
	}

	heap_id = ctx->head_id;
	heap = hisi_drm_get_heap(heap_id);
	if (IS_ERR_OR_NULL(heap)) {
		pr_heaps_err("can't find heap by id = %#x\n", heap_id);
		return;
	}

	if (!heap->ops->unmap_kernel)
		return;

	hisi_drm_heaps_kmap_put(heap, ctx, vaddr);
}

int hisi_drm_heap_map_user(struct hisi_drm_heap_context *ctx,
			   struct vm_area_struct *vma)
{
	struct hisi_drm_heap *heap = NULL;
	unsigned int heap_id;

	if (!ctx || !ctx->priv || !vma)
		return -EINVAL;

	heap_id = ctx->head_id;
	heap = hisi_drm_get_heap(heap_id);
	if (IS_ERR_OR_NULL(heap)) {
		pr_heaps_err("can't find heap by id = %#x\n", heap_id);
		return PTR_ERR(heap);
	}

	if (!heap->ops->map_user)
		return -ENOENT;

	return __do_map_user(heap, ctx, vma);
}
