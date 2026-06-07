/* Copyright (c) Hisilicon Technologies Co., Ltd. 2001-2019. All rights reserved.
 * FileName: ion_seccg_heap.c
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#define pr_fmt(fmt) "seccg: " fmt

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <linux/genalloc.h>
#include <linux/mutex.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/cma.h>
#include <linux/sizes.h>
#include <linux/memblock.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/version.h>
#include <linux/hisi/hisi_cma.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi/hisi_mm.h>

#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#include "ion.h"
#include "hisi/ion_tee_op.h"
#include "hisi/ion_sec_contig.h"
#include "hisi/sec_alloc.h"
#include "hisi_ion_priv.h"

static struct cma *hisi_cma;
static struct ion_sec_cma hisi_seccg_cmas[SEC_CG_CMA_NUM];
static unsigned int seccg_cma_num;

int hisi_sec_cma_set_up(struct reserved_mem *rmem)
{
	phys_addr_t align = PAGE_SIZE << max(MAX_ORDER - 1, pageblock_order);
	phys_addr_t mask = align - 1;
	unsigned long node = rmem->fdt_node;
	struct cma *cma = NULL;
	const char *cma_name = NULL;
	u64 *val = NULL;
	u64 svc_id = 0;
	int err;

	if (seccg_cma_num == 0)
		memset(hisi_seccg_cmas, 0,/* unsafe_function_ignore: memset */
		       sizeof(hisi_seccg_cmas));

	if (seccg_cma_num >= SEC_CG_CMA_NUM) {
		pr_err("Sec cma num overflow!seccg_cma_num:%u\n",
		       seccg_cma_num);
		return -EINVAL;
	}

	cma_name = (const char *)of_get_flat_dt_prop(node, "compatible", NULL);

	if (!cma_name) {
		pr_err("can't get compatible, id:%u\n", seccg_cma_num);
		return -EINVAL;
	}

	val = (u64 *)of_get_flat_dt_prop(node, "svc-id", NULL);
	if (val)
		svc_id = be64_to_cpu(*val);
	pr_err("cma svc-id is 0x%llx\n", svc_id);

	if (!of_get_flat_dt_prop(node, "reusable", NULL) ||
	    of_get_flat_dt_prop(node, "no-map", NULL)) {
		pr_err("%s err node\n", __func__);
		return -EINVAL;
	}

	if ((rmem->base & mask) || (rmem->size & mask)) {
		pr_err("Reserved memory: incorrect alignment of CMA region\n");
		return -EINVAL;
	}

	if (!memblock_is_memory(rmem->base)) {
		memblock_free(rmem->base, rmem->size);
		pr_err("memory is invalid(0x%llx), size(0x%llx)\n",
			rmem->base, rmem->size);
		return -EINVAL;
	}
	err = cma_init_reserved_mem(rmem->base, rmem->size,
				    0, rmem->name, &cma);
	if (err) {
		pr_err("Reserved memory: unable to setup CMA region\n");
		return err;
	}

	set_svc_cma(svc_id, cma);

	hisi_seccg_cmas[seccg_cma_num].cma_region = cma;
	hisi_seccg_cmas[seccg_cma_num].cma_name = cma_name;

	pr_err("%s init cma:rmem->base:0x%llx,size:0x%llx,cmaname:%s, seccg_cma_num:%u\n",
	       __func__, rmem->base, rmem->size, cma_name, seccg_cma_num);

	seccg_cma_num++;

	if (!strcmp(cma_name, "hisi-cma-pool")) {
		pr_err("%s,cma reg to dma_camera_cma\n", __func__);
		hisi_cma = cma;
		ion_register_dma_camera_cma((void *)cma);
	}

	return 0;
}
RESERVEDMEM_OF_DECLARE(hisi_dynamic_cma,
		       "hisi-cma-pool",
		       hisi_sec_cma_set_up);
RESERVEDMEM_OF_DECLARE(hisi_iris_static_cma,
		       "hisi-iris-sta-cma-pool",
		       hisi_sec_cma_set_up);
RESERVEDMEM_OF_DECLARE(hisi_algo_static_cma,
		       "hisi-algo-sta-cma-pool",
		       hisi_sec_cma_set_up);

struct cma *get_sec_cma(void)
{
	return hisi_cma;
}

static int __seccg_heap_input_check(struct ion_seccg_heap *seccg_heap,
				    unsigned long size, unsigned long flag)
{
	if (seccg_heap->alloc_size + size <= seccg_heap->alloc_size) {
		pr_err("size overflow! alloc_size = 0x%lx, size = 0x%lx,\n",
			seccg_heap->alloc_size, size);
		return -EINVAL;
	}

	if ((seccg_heap->alloc_size + size) > seccg_heap->heap_size) {
		pr_err("alloc size = 0x%lx, size = 0x%lx, heap size = 0x%lx\n",
			seccg_heap->alloc_size, size, seccg_heap->heap_size);
		return -EINVAL;
	}

	if (size > seccg_heap->per_alloc_sz) {
		pr_err("size too large! size 0x%lx, per_alloc_sz 0x%llx",
			size, seccg_heap->per_alloc_sz);
		return -EINVAL;
	}

	if ((seccg_heap->heap_attr == HEAP_SECURE_TEE ||
	    seccg_heap->heap_attr == HEAP_SECURE) &&
	    !(flag & ION_FLAG_SECURE_BUFFER)) {
		pr_err("alloc mem w/o sec flag in sec heap(%u) flag(%lx)\n",
		       seccg_heap->heap_attr, flag);
		return -EINVAL;
	}

	if ((seccg_heap->heap_attr == HEAP_NORMAL) &&
	    (flag & ION_FLAG_SECURE_BUFFER)) {
		pr_err("invalid allocate sec in sec heap(%u) flag(%lx)\n",
				seccg_heap->heap_attr, flag);
		return -EINVAL;
	}

	seccg_heap->flag = flag;
	return 0;
}

static int __seccg_create_static_cma_region(struct ion_seccg_heap *seccg_heap,
					    u64 cma_base, u64 cma_size)
{
	struct page *page = NULL;

	page = cma_alloc(seccg_heap->cma, cma_size >> PAGE_SHIFT, 0, false);
	if (!page) {
		pr_err("%s cma_alloc failed!cma_base 0x%llx cma_size 0x%llx\n",
		       __func__, cma_base, cma_size);
		return -ENOMEM;
	}

	pr_err("%s cma_alloc success!cma_base 0x%llx cma_size 0x%llx\n",
	       __func__, cma_base, cma_size);

	seccg_heap->static_cma_region =
		gen_pool_create((int)seccg_heap->pool_shift, -1);

	if (!seccg_heap->static_cma_region) {
		pr_err("static_cma_region create failed\n");
		cma_release(seccg_heap->cma, page, (u32)cma_size >> PAGE_SHIFT);
		return -ENOMEM;
	}

	gen_pool_set_algo(seccg_heap->static_cma_region,
			  gen_pool_best_fit, NULL);

	if (gen_pool_add(seccg_heap->static_cma_region,
			 page_to_phys(page), cma_size, -1)) {
		pr_err("static_cma_region add base 0x%llx cma_size 0x%llx failed!\n",
		       cma_base, cma_size);
		cma_release(seccg_heap->cma, page, (u32)cma_size >> PAGE_SHIFT);
		gen_pool_destroy(seccg_heap->static_cma_region);
		seccg_heap->static_cma_region = NULL;
		return -ENOMEM;
	}

	return 0;
}

static int __seccg_create_pool(struct ion_seccg_heap *seccg_heap)
{
	u64 cma_base;
	u64 cma_size;
	int ret = 0;

	sec_debug("into %s\n", __func__);
	/* Allocate on 4KB boundaries (1 << ION_PBL_SHIFT)*/
	seccg_heap->pool = gen_pool_create((int)seccg_heap->pool_shift, -1);

	if (!seccg_heap->pool) {
		pr_err("in __seccg_create_pool create failed\n");
		return -ENOMEM;
	}
	gen_pool_set_algo(seccg_heap->pool, gen_pool_best_fit, NULL);

	/* Add all memory to genpool first，one chunk only*/
	cma_base = cma_get_base(seccg_heap->cma);
	cma_size = cma_get_size(seccg_heap->cma);

	if (gen_pool_add(seccg_heap->pool, cma_base, cma_size, -1)) {
		pr_err("genpool add base 0x%llx cma_size 0x%llx\n",
		       cma_base, cma_size);
		ret = -ENOMEM;
		goto err_add;
	}

	if (!gen_pool_alloc(seccg_heap->pool, cma_size)) {
		pr_err("in __seccg_create_pool alloc failed\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	if (seccg_heap->cma_type == CMA_STATIC) {
		ret = __seccg_create_static_cma_region(seccg_heap,
						       cma_base,
						       cma_size);
		if (ret)
			goto err_cma_alloc;
	}

	return 0;
err_cma_alloc:
	gen_pool_free(seccg_heap->pool, cma_base, cma_size);
err_alloc:
	gen_pool_destroy(seccg_heap->pool);
err_add:
	seccg_heap->pool = NULL;
	return ret;
}

static int seccg_alloc(struct ion_seccg_heap *seccg_heap,
			struct ion_buffer *buffer,
			unsigned long size)
{
	int ret;

	ret = __seccg_alloc_contig(seccg_heap, buffer, size);

	return ret;
}

int ion_seccg_heap_phys(struct ion_heap *heap, struct ion_buffer *buffer,
			 phys_addr_t *addr, size_t *len)
{
	struct sg_table *table = buffer->sg_table;
	struct page *page = sg_page(table->sgl);
	struct ion_seccg_heap *seccg_heap = NULL;
	phys_addr_t paddr = 0;

	if (heap->type != ION_HEAP_TYPE_SEC_CONTIG) {
		pr_err("%s: not seccg mem!\n", __func__);
		return -EINVAL;
	}

	seccg_heap = container_of(heap, struct ion_seccg_heap, heap);

	paddr = PFN_PHYS(page_to_pfn(page));
	*addr = paddr;
	*len = buffer->size;

	return 0;
}

static void seccg_free(struct ion_seccg_heap *seccg_heap,
			struct ion_buffer *buffer)
{
	sec_debug("%s: enter, free size 0x%zx\n", __func__, buffer->size);
	__seccg_free_contig(seccg_heap, buffer);
	sec_debug("%s: exit\n", __func__);
}

static int ion_seccg_heap_allocate(struct ion_heap *heap,
				   struct ion_buffer *buffer,
				   unsigned long size,
				   unsigned long flags)
{
	struct ion_seccg_heap *seccg_heap =
		container_of(heap, struct ion_seccg_heap, heap);
	int ret = 0;

	sec_debug("enter %s  size 0x%lx heap id %u\n",
		    __func__, size, heap->id);
	mutex_lock(&seccg_heap->mutex);

	if (__seccg_heap_input_check(seccg_heap, size, flags)) {
		pr_err("input params failed\n");
		ret = -EINVAL;
		goto out;
	}

	/*init the TA conversion here*/
	if (!seccg_heap->TA_init &&
	    (seccg_heap->heap_attr == HEAP_SECURE_TEE)) {
		ret = secmem_tee_init(seccg_heap->context, seccg_heap->session);
		if (ret) {
			pr_err("[%s] TA init failed\n", __func__);
			goto out;
		}
		seccg_heap->TA_init = 1;
	}

	if (seccg_alloc(seccg_heap, buffer, size)) {
		ret = -ENOMEM;
		goto out;
	}

	seccg_heap->alloc_size += size;
	sec_debug("seccg heap alloc succ, heap all alloc_size 0x%lx\n",
		    seccg_heap->alloc_size);
	mutex_unlock(&seccg_heap->mutex);
	return 0;
out:
	pr_err("heap[%d] alloc fail, size 0x%lx, heap all alloc_size 0x%lx\n",
	       heap->id, size, seccg_heap->alloc_size);
	mutex_unlock(&seccg_heap->mutex);

	if (ret == -ENOMEM)
		hisi_ion_memory_info(true);

	return ret;
}

static void ion_seccg_heap_free(struct ion_buffer *buffer)
{
	struct ion_heap *heap = buffer->heap;
	struct ion_seccg_heap *seccg_heap =
		container_of(heap, struct ion_seccg_heap, heap);

	sec_debug("%s:enter, heap %d, free size:0x%zx,\n",
		    __func__, heap->id, buffer->size);
	mutex_lock(&seccg_heap->mutex);
	seccg_free(seccg_heap, buffer);
	sec_debug("out %s:heap remaining allocate %lx\n",
		    __func__, seccg_heap->alloc_size);
	mutex_unlock(&seccg_heap->mutex);
}

static int ion_seccg_heap_map_user(struct ion_heap *heap,
				   struct ion_buffer *buffer,
				   struct vm_area_struct *vma)
{
	struct ion_seccg_heap *seccg_heap =
		container_of(heap, struct ion_seccg_heap, heap);
	if (seccg_heap->heap_attr != HEAP_NORMAL) {
		pr_err("secure buffer, can not call %s\n", __func__);
		return -EINVAL;
	}

	return ion_heap_map_user(heap, buffer, vma);
}

static void *ion_seccg_heap_map_kernel(struct ion_heap *heap,
					struct ion_buffer *buffer)
{
	struct ion_seccg_heap *seccg_heap =
		container_of(heap, struct ion_seccg_heap, heap);
	if (seccg_heap->heap_attr != HEAP_NORMAL) {
		pr_err("secure buffer, can not call %s\n", __func__);
		return NULL;
	}

	return ion_heap_map_kernel(heap, buffer);
}

static void ion_seccg_heap_unmap_kernel(struct ion_heap *heap,
					struct ion_buffer *buffer)
{
	struct ion_seccg_heap *seccg_heap =
		container_of(heap, struct ion_seccg_heap, heap);
	if (seccg_heap->heap_attr != HEAP_NORMAL) {
		pr_err("secure buffer, can not call %s\n", __func__);
		return;
	}

	ion_heap_unmap_kernel(heap, buffer);
}

static struct ion_heap_ops seccg_heap_ops = {
	.allocate = ion_seccg_heap_allocate,
	.free = ion_seccg_heap_free,
	.map_user = ion_seccg_heap_map_user,
	.map_kernel = ion_seccg_heap_map_kernel,
	.unmap_kernel = ion_seccg_heap_unmap_kernel,
};

static void __seccg_parse_dt_cma_para(struct device_node *nd,
				      struct ion_seccg_heap *seccg_heap)
{
	const char *compatible = NULL;
	struct device_node *phandle_node = NULL;
	u32 cma_type = 0;
	int ret = 0;

	phandle_node = of_parse_phandle(nd, "cma-region", 0);
	if (phandle_node) {
		ret = of_property_read_string(phandle_node,
					      "compatible",
					      &compatible);
		if (ret) {
			pr_err("no compatible\n");
			return;
		}
		seccg_heap->cma_name = compatible;
	} else {
		seccg_heap->cma_name = "hisi-cma-pool";
		pr_err("no cma-region phandle\n");
	}
	ret = of_property_read_u32(nd, "cma-type", &cma_type);
	if (ret < 0) {
		seccg_heap->cma_type = CMA_DYNAMIC;
		pr_err("can't find prop:cma-type\n");
	} else {
		seccg_heap->cma_type = cma_type;
	}
}

static int __seccg_parse_dt(struct device *dev,
			    struct ion_platform_heap *heap_data,
			    struct ion_seccg_heap *seccg_heap)
{
	struct device_node *nd = NULL;
	u64 per_bit_sz = 0;
	u64 per_alloc_sz = 0;
	u64 water_mark = 0;
	u32 heap_attr = 0;
	u32 pool_shift = ION_PBL_SHIFT;
	u32 pre_alloc_attr = 0;
	int ret = 0;

	nd = of_get_child_by_name(dev->of_node, heap_data->name);
	if (!nd) {
		pr_err("can't of_get_child_by_name %s\n", heap_data->name);
		ret = -EINVAL;
		goto out;
	}

	__seccg_parse_dt_cma_para(nd, seccg_heap);

	ret = of_property_read_u64(nd, "per-alloc-size", &per_alloc_sz);
	if (ret < 0) {
		pr_err("can't find prop:per-alloc-size\n");
		goto out;
	}
	seccg_heap->per_alloc_sz = PAGE_ALIGN(per_alloc_sz);

	ret = of_property_read_u64(nd, "per-bit-size", &per_bit_sz);
	if (ret < 0) {
		pr_err("can't find prop:per-bit-size\n");
		goto out;
	}
	seccg_heap->per_bit_sz = PAGE_ALIGN(per_bit_sz);

	ret = of_property_read_u64(nd, "water-mark", &water_mark);
	if (ret < 0) {
		pr_err("can't find prop:water-mark\n");
		water_mark = 0;
	}
	seccg_heap->water_mark = PAGE_ALIGN(water_mark);

	ret = of_property_read_u32(nd, "pool-shift", &pool_shift);
	if (ret < 0) {
		pr_err("can not find pool-shift.\n");
		pool_shift = ION_PBL_SHIFT;
	}
	seccg_heap->pool_shift = pool_shift;

	ret = of_property_read_u32(nd, "heap-attr", &heap_attr);
	if (ret < 0) {
		pr_err("can not find heap-arrt.\n");
		heap_attr = HEAP_NORMAL;
	}
	if (heap_attr >= HEAP_MAX)
		heap_attr = HEAP_NORMAL;
	seccg_heap->heap_attr = heap_attr;

	ret = of_property_read_u32(nd, "pre-alloc-attr", &pre_alloc_attr);
	if (ret < 0) {
		pr_err("can't find prop:pre-alloc-attr.\n");
		pre_alloc_attr = 0;
		ret = 0;
	}
	seccg_heap->pre_alloc_attr = pre_alloc_attr;

out:
	return ret;
}

static struct cma *__seccg_heap_getcma(struct ion_seccg_heap *seccg_heap)
{
	int i;

	for (i = 0; i < SEC_CG_CMA_NUM; i++) {
		if (seccg_heap->cma_name &&
		    hisi_seccg_cmas[i].cma_name &&
		    (!strcmp(seccg_heap->cma_name,
			     hisi_seccg_cmas[i].cma_name)))
			return hisi_seccg_cmas[i].cma_region;
	}

	return NULL;
}

static void seccg_heap_init(struct ion_seccg_heap *seccg_heap,
						struct ion_platform_heap *heap_data)
{
	seccg_heap->pool = NULL;
	seccg_heap->heap.ops = &seccg_heap_ops;
	seccg_heap->heap.type = ION_HEAP_TYPE_SEC_CONTIG;
	seccg_heap->heap_size = heap_data->size;
	seccg_heap->alloc_size = 0;
	seccg_heap->cma_alloc_size = 0;
}

struct ion_heap *ion_seccg_heap_create(struct ion_platform_heap *heap_data)
{
	int ret;
	struct device *dev = NULL;
	struct ion_seccg_heap *seccg_heap = NULL;

	seccg_heap = kzalloc(sizeof(*seccg_heap), GFP_KERNEL);
	if (!seccg_heap)
		return ERR_PTR(-ENOMEM);

	mutex_init(&seccg_heap->mutex);
	mutex_init(&seccg_heap->pre_alloc_mutex);

	seccg_heap_init(seccg_heap, heap_data);
	dev = heap_data->priv;
	INIT_LIST_HEAD(&seccg_heap->allocate_head);

	ret = __seccg_parse_dt(dev, heap_data, seccg_heap);
	if (ret)
		goto free_heap;

	seccg_heap->cma = __seccg_heap_getcma(seccg_heap);

	if (!seccg_heap->cma) {
		pr_err("%s, can't get cma! cma_type:%u, cma_name:%s\n",
		       __func__, seccg_heap->cma_type, seccg_heap->cma_name);
		goto free_heap;
	}

	if (seccg_heap->heap_attr == HEAP_SECURE_TEE) {
		seccg_heap->context = kzalloc(sizeof(TEEC_Context), GFP_KERNEL);
		if (!seccg_heap->context)
			goto free_heap;
		seccg_heap->session = kzalloc(sizeof(TEEC_Session), GFP_KERNEL);
		if (!seccg_heap->session)
			goto free_context;
	} else {
		seccg_heap->context = NULL;
		seccg_heap->session = NULL;
	}
	seccg_heap->TA_init = 0;

	ret = __seccg_create_pool(seccg_heap);
	if (ret) {
		pr_err("[%s] pool create failed.\n", __func__);
		goto free_session;
	}

	if (seccg_heap->water_mark &&
	    __seccg_fill_watermark(seccg_heap))
		pr_err("__seccg_fill_watermark failed!\n");

	if (seccg_heap->pre_alloc_attr)
		INIT_WORK(&seccg_heap->pre_alloc_work, seccg_pre_alloc_wk_func);

	pr_err("seccg heap info %s:\n"
		  "\t\t\t\t\t\t\t heap id : %u\n"
		  "\t\t\t\t\t\t\t heap cmatype : %u\n"
		  "\t\t\t\t\t\t\t heap cmaname : %s\n"
		  "\t\t\t\t\t\t\t heap attr : %u\n"
		  "\t\t\t\t\t\t\t pre alloc attr : %u\n"
		  "\t\t\t\t\t\t\t pool shift : %u\n"
		  "\t\t\t\t\t\t\t heap size : %lu MB\n"
		  "\t\t\t\t\t\t\t per alloc size :  %llu MB\n"
		  "\t\t\t\t\t\t\t per bit size : %llu KB\n"
		  "\t\t\t\t\t\t\t water_mark size : %llu MB\n"
		  "\t\t\t\t\t\t\t cma base : 0x%llx\n"
		  "\t\t\t\t\t\t\t cma size : 0x%lx\n",
		  heap_data->name,
		  heap_data->id,
		  seccg_heap->cma_type,
		  seccg_heap->cma_name,
		  seccg_heap->heap_attr,
		  seccg_heap->pre_alloc_attr,
		  seccg_heap->pool_shift,
		  seccg_heap->heap_size / SZ_1M,
		  seccg_heap->per_alloc_sz / SZ_1M,
		  seccg_heap->per_bit_sz / SZ_1K,
		  seccg_heap->water_mark / SZ_1M,
		  cma_get_base(seccg_heap->cma),
		  cma_get_size(seccg_heap->cma));

	return &seccg_heap->heap;
free_session:
	if (seccg_heap->session)
		kfree(seccg_heap->session);
free_context:
	if (seccg_heap->context)
		kfree(seccg_heap->context);
free_heap:
	kfree(seccg_heap);
	return ERR_PTR(-ENOMEM);
}
