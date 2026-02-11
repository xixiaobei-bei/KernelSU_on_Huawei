/*
 * ion_secsg_heap.c
 *
 * Copyright (C) 2018 Hisilicon, Inc.
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

#define pr_fmt(fmt) "secsg: " fmt

#include <linux/cma.h>
#include <linux/err.h>
#include <linux/genalloc.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/sizes.h>
#include <linux/version.h>
#include <linux/hisi/hisi_cma.h>
#include <linux/hisi/hisi_ion.h>

#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#include "ion.h"
#include "hisi/ion_tee_op.h"
#include "hisi/sec_alloc.h"
#include "hisi_ion_priv.h"


struct ion_secsg_heap {
	struct ion_heap heap;
	struct cma *cma;
	struct gen_pool *pool;
	/* heap mutex */
	struct mutex mutex;
	/* heap total size */
	size_t heap_size;
	/* heap allocated size */
	unsigned long alloc_size;
	/* align size: 1MB or 2MB */
	u64 per_bit_sz;
	/* each sg section size */
	u64 per_alloc_sz;
	TEEC_Context *context;
	TEEC_Session *session;
	/* heap attr: secure, protect */
	u32 heap_attr;
	u32 pool_flag;
	int TA_init;
};

#ifdef CONFIG_SECMEM_TEST
#define ION_FLAG_ALLOC_TEST (1U << 31)

static void double_alloc_test(struct ion_secsg_heap *secsg_heap,
			      struct ion_buffer *buffer, u32 cmd)
{
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	struct mem_chunk_list mcl;
	struct tz_pageinfo *pageinfo = NULL;
	unsigned int nents = table->nents;
	int ret = 0;
	u32 size = 0,i;

	pageinfo = kcalloc(nents, sizeof(*pageinfo), GFP_KERNEL);
	if (!pageinfo)
		return;

	for_each_sg(table->sgl, sg, table->nents, i) {
		page = sg_page(sg);
		pageinfo[i].addr = page_to_phys(page);
		pageinfo[i].nr_pages = sg->length / PAGE_SIZE;
		size += sg->length;
	}

	mcl.phys_addr = (void *)pageinfo;
	mcl.nents = nents;
	mcl.protect_id = SEC_TASK_DRM;
	mcl.size = size;

	ret = secmem_tee_exec_cmd(secsg_heap->session, &mcl, cmd);
	kfree(pageinfo);
	if (!ret)
		pr_err("Test double alloc Fail\n");
	else
		pr_err("Test double alloc Succ\n");
}

static void sion_test(struct ion_secsg_heap *secsg_heap,
		      struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	struct mem_chunk_list mcl;
	struct tz_pageinfo *pageinfo = NULL;
	unsigned int nents = table->nents;
	int ret = 0;
	u32 size = 0;
	u32 i;

	if (!secsg_heap->TA_init) {
		secsg_heap->context = kzalloc(sizeof(TEEC_Context), GFP_KERNEL);
		if (!secsg_heap->context)
			return;

		secsg_heap->session = kzalloc(sizeof(TEEC_Session), GFP_KERNEL);
		if (!secsg_heap->session) {
			kfree(secsg_heap->context);
			return;
		}

		ret = secmem_tee_init(secsg_heap->context, secsg_heap->session);
		if (ret) {
			pr_err("[%s] TA init failed\n", __func__);
			kfree(secsg_heap->context);
			kfree(secsg_heap->session);
			return;
		}
		secsg_heap->TA_init = 1;
	}

	pageinfo = kcalloc(nents, sizeof(*pageinfo), GFP_KERNEL);
	if (!pageinfo)
		return;

	for_each_sg(table->sgl, sg, table->nents, i) {
		page = sg_page(sg);
		pageinfo[i].addr = page_to_phys(page);
		pageinfo[i].nr_pages = sg->length / PAGE_SIZE;
		size += sg->length;
	}

	mcl.phys_addr = (void *)pageinfo;
	mcl.nents = nents;
	mcl.size = size;
	if (buffer->id) {
		mcl.protect_id = SEC_TASK_DRM;
		mcl.buff_id = buffer->id;
	} else
		mcl.protect_id = SEC_TASK_SEC;

	ret = secmem_tee_exec_cmd(secsg_heap->session, &mcl, ION_SEC_CMD_TEST);
	kfree(pageinfo);
	if (ret)
		pr_err("Test sion Fail\n");
	else
		pr_err("Test sion Succ\n");
}
#endif

static int change_scatter_prop(struct ion_secsg_heap *secsg_heap,
			       struct ion_buffer *buffer,
			       u32 cmd)
{
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	struct mem_chunk_list mcl;
	struct tz_pageinfo *pageinfo = NULL;
	unsigned int nents = table->nents;
	int ret = 0;
	u32 i;

	if (!secsg_heap->TA_init) {
		pr_err("[%s] TA not inited.\n", __func__);
		return -EINVAL;
	}

	if (cmd == ION_SEC_CMD_ALLOC) {
		pageinfo = kcalloc(nents, sizeof(*pageinfo), GFP_KERNEL);
		if (!pageinfo)
			return -ENOMEM;

		for_each_sg(table->sgl, sg, table->nents, i) {
			page = sg_page(sg);
			pageinfo[i].addr = page_to_phys(page);
			pageinfo[i].nr_pages = sg->length / PAGE_SIZE;
		}

		mcl.phys_addr = (void *)pageinfo;
		mcl.nents = nents;
		mcl.protect_id = SEC_TASK_DRM;
	} else if (cmd == ION_SEC_CMD_FREE) {
		mcl.protect_id = SEC_TASK_DRM;
		mcl.buff_id = buffer->id;
		mcl.phys_addr = NULL;
	} else {
		pr_err("%s: Error cmd\n", __func__);
		return -EINVAL;
	}

	ret = secmem_tee_exec_cmd(secsg_heap->session, &mcl, cmd);
	if (ret) {
		pr_err("%s:exec cmd[%d] fail\n", __func__, cmd);
		ret = -EINVAL;
	} else {
		if (cmd == ION_SEC_CMD_ALLOC)
			buffer->id = mcl.buff_id;
	}

	if (pageinfo)
		kfree(pageinfo);

	return ret;
}

static struct page *__secsg_alloc_page(struct ion_secsg_heap *secsg_heap,
				       unsigned long size,
				       unsigned long align)
{
	struct page *page = NULL;
	unsigned long offset = 0;
	u32 count = 0;

	if (secsg_heap->pool_flag) {
		offset = gen_pool_alloc(secsg_heap->pool, size);
		if (!offset)
			return NULL;

		page = pfn_to_page(PFN_DOWN(offset));
	} else {
		count = (u32)size / PAGE_SIZE;
		page = cma_alloc(secsg_heap->cma, count,
				 (u32)get_order((u32)align), false);
	}

	return page;
}

static void __secsg_free_page(struct ion_secsg_heap *secsg_heap,
			      struct page *page,
			      unsigned long size)
{
	if (secsg_heap->pool_flag)
		gen_pool_free(secsg_heap->pool, page_to_phys(page), size);
	else
		cma_release(secsg_heap->cma, page, (u32)size / PAGE_SIZE);
}

static struct page *__secsg_alloc_large(struct ion_secsg_heap *secsg_heap,
					unsigned long size,
					unsigned long align)
{
	struct page *page = NULL;
#ifdef CONFIG_HISI_KERNELDUMP
	struct page *tmp_page = NULL;
	u32 count = 0;
	u32 k;
#endif

	page = __secsg_alloc_page(secsg_heap, size, align);
	if (!page)
		return NULL;

	if (!secsg_heap->pool_flag)
		memset(page_to_virt((uintptr_t)page), 0, size);
#ifdef CONFIG_HISI_KERNELDUMP
	count = (u32)size / PAGE_SIZE;
	tmp_page = page;
	for (k = 0; k < count; k++) {
		SetPageMemDump(tmp_page);
		tmp_page++;
	}
#endif
	return page;
}

static int flush_tlb_cache_send_cmd(struct ion_buffer *buffer, struct sg_table *table,
					struct ion_secsg_heap *secsg_heap)
{
	int ret;

#ifdef CONFIG_NEED_CHANGE_MAPPING
	flush_tlb_all();
#endif
	ion_flush_all_cpus_caches();

	buffer->sg_table = table;

	/*
	 * Send cmd to secos change the memory form normal to protect when
	 * user is DRM, and record some information of the sg_list for secos
	 * va map.
	 * Other user don't need to do this.
	 */
	if (secsg_heap->heap_attr == SEC_DRM_TEE) {
		ret = change_scatter_prop(secsg_heap, buffer,
					  ION_SEC_CMD_ALLOC);
		if (ret)
			return -1;
	}

#ifdef CONFIG_SECMEM_TEST
	if (buffer->flags & ION_FLAG_ALLOC_TEST) {
		pr_err("sion TEST start!\n");
		if (secsg_heap->heap_attr == SEC_DRM_TEE)
			double_alloc_test(secsg_heap, buffer, ION_SEC_CMD_ALLOC);
		sion_test(secsg_heap, buffer);
		pr_err("sion TEST end!\n");
	}
#endif

	sec_debug("%s: exit\n", __func__);
	return 0;
}

int __secsg_alloc_scatter(struct ion_secsg_heap *secsg_heap,
			  struct ion_buffer *buffer, unsigned long size)
{
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	unsigned long per_bit_sz = secsg_heap->per_bit_sz;
	unsigned long per_alloc_sz = secsg_heap->per_alloc_sz;
	unsigned long size_remaining = ALIGN(size, per_bit_sz);
	unsigned long alloc_size = 0;
	unsigned long nents = ALIGN(size, per_alloc_sz) / per_alloc_sz;
	int ret = 0;
	u32 i = 0;

	sec_debug("%s: enter, ALIGN size 0x%lx\n", __func__, size_remaining);
	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	if (sg_alloc_table(table, (u32)nents, GFP_KERNEL))
		goto free_table;

	/*
	 * DRM memory alloc from CMA pool.
	 * In order to speed up the allocation, we will apply for memory
	 * in units of 2MB, and the memory portion of less than 2MB will
	 * be applied for one time.
	 */
	sg = table->sgl;
	while (size_remaining) {
		if (size_remaining > per_alloc_sz)
			alloc_size = per_alloc_sz;
		else
			alloc_size = size_remaining;

		page = __secsg_alloc_large(secsg_heap, alloc_size, per_bit_sz);
		if (!page)
			goto free_pages;

#ifdef CONFIG_NEED_CHANGE_MAPPING
		/*
		 * Before set memory in secure region, we need change kernel
		 * pgtable from normal to device to avoid big CPU Speculative
		 * read.
		 */
		change_secpage_range(page_to_phys(page),
				     (unsigned long)page_address(page),
				     alloc_size, __pgprot(PROT_DEVICE_nGnRE));
#endif
		size_remaining -= alloc_size;
		sg_set_page(sg, page, (u32)alloc_size, 0);
		sg = sg_next(sg);
		i++;
	}
	/*
	 * After change the pgtable prot, we need flush TLB and cache.
	 */
	 ret = flush_tlb_cache_send_cmd(buffer, table, secsg_heap);
	 if (ret)
		 goto free_pages;

	 return 0;
free_pages:
	nents = i;
	for_each_sg(table->sgl, sg, nents, i) {
		page = sg_page(sg);
#ifdef CONFIG_NEED_CHANGE_MAPPING
		change_secpage_range(page_to_phys(page),
				     (unsigned long)page_address(page),
				     sg->length, PAGE_KERNEL);
		flush_tlb_all();
#endif
		__secsg_free_page(secsg_heap, page, sg->length);
	}
#ifdef CONFIG_NEED_CHANGE_MAPPING
	flush_tlb_all();
#endif
	sg_free_table(table);
free_table:
	kfree(table);

	return -ENOMEM;
}

static void __secsg_free_scatter(struct ion_secsg_heap *secsg_heap,
				 struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg = NULL;
	int ret = 0;
	u32 i;

	if (secsg_heap->heap_attr == SEC_DRM_TEE) {
		ret = change_scatter_prop(secsg_heap, buffer, ION_SEC_CMD_FREE);
		if (ret) {
			pr_err("release MPU protect fail! Need check runtime\n");
			return;
		}
	}

	if (secsg_heap->pool_flag) {
		buffer->flags |= ION_FLAG_CACHED;
		ion_heap_buffer_zero(buffer);
		ion_flush_all_cpus_caches();
	}

	for_each_sg(table->sgl, sg, table->nents, i) {
#ifdef CONFIG_NEED_CHANGE_MAPPING
		change_secpage_range(page_to_phys(sg_page(sg)),
				     (unsigned long)page_address(sg_page(sg)),
				     sg->length, PAGE_KERNEL);
		flush_tlb_all();
#endif
		__secsg_free_page(secsg_heap, sg_page(sg), sg->length);
	}
#ifdef CONFIG_NEED_CHANGE_MAPPING
	flush_tlb_all();
#endif
	sg_free_table(table);
	kfree(table);
	secsg_heap->alloc_size -= buffer->size;
}

int ion_secsg_heap_phys(struct ion_heap *heap, struct ion_buffer *buffer,
			phys_addr_t *addr, size_t *len)
{
	struct ion_secsg_heap *secsg_heap = NULL;

	if (!heap || !buffer || !addr || !len) {
		pr_err("%s: invalid input para!\n", __func__);
		return -EINVAL;
	}

	if (heap->type != ION_HEAP_TYPE_SECSG) {
		pr_err("%s: not secsg mem!\n", __func__);
		return -EINVAL;
	}

	secsg_heap = container_of(heap, struct ion_secsg_heap, heap);

	if (secsg_heap->heap_attr != SEC_DRM_TEE) {
		pr_err("%s: sec buffer don't support get phys!\n", __func__);
		return -EINVAL;
	}

	*addr = buffer->id;
	*len = buffer->size;

	return 0;
}

static int __secsg_heap_input_check(struct ion_secsg_heap *secsg_heap,
				    unsigned long size, unsigned long flag)
{
	if (secsg_heap->alloc_size + size <= secsg_heap->alloc_size) {
		pr_err("size overflow! alloc_size = 0x%lx, size = 0x%lx,\n",
			secsg_heap->alloc_size, size);
		return -EINVAL;
	}

	if ((secsg_heap->alloc_size + size) > secsg_heap->heap_size) {
		pr_err("alloc size = 0x%lx, size = 0x%lx, heap size = 0x%lx\n",
			secsg_heap->alloc_size, size, secsg_heap->heap_size);
		return -EINVAL;
	}

	if (!(flag & ION_FLAG_SECURE_BUFFER)) {
		pr_err("invalid alloc flag in heap(%u) flag(%lx)\n",
		       secsg_heap->heap_attr, flag);
		return -EINVAL;
	}

	return 0;
}

static int ion_secsg_heap_allocate(struct ion_heap *heap,
				   struct ion_buffer *buffer,
				   unsigned long size,
				   unsigned long flags)
{
	struct ion_secsg_heap *secsg_heap =
		container_of(heap, struct ion_secsg_heap, heap);
	int ret = 0;

	sec_debug("enter %s  size 0x%lx heap id %u\n",
		    __func__, size, heap->id);
	mutex_lock(&secsg_heap->mutex);

	if (__secsg_heap_input_check(secsg_heap, size, flags)) {
		pr_err("input params failed\n");
		ret = -EINVAL;
		goto out;
	}

	/*init the TA conversion here*/
	if (secsg_heap->heap_attr == SEC_DRM_TEE && !secsg_heap->TA_init) {
		ret = secmem_tee_init(secsg_heap->context, secsg_heap->session);
		if (ret) {
			pr_err("[%s] TA init failed\n", __func__);
			goto out;
		}
		secsg_heap->TA_init = 1;
	}

	if (__secsg_alloc_scatter(secsg_heap, buffer, size)) {
		ret = -ENOMEM;
		goto out;
	}

	secsg_heap->alloc_size += size;
	sec_debug("secsg heap alloc succ, heap all alloc_size 0x%lx\n",
		    secsg_heap->alloc_size);
	mutex_unlock(&secsg_heap->mutex);

	return 0;
out:
	if (!(flags & ION_FLAG_ALLOC_NOWARN_BUFFER))
		pr_err("heap[%d] alloc fail, size 0x%lx, heap all alloc_size 0x%lx\n",
			heap->id, size, secsg_heap->alloc_size);
	mutex_unlock(&secsg_heap->mutex);

	if (ret == -ENOMEM && !(flags & ION_FLAG_ALLOC_NOWARN_BUFFER))
		hisi_ion_memory_info(true);

	return ret;
}

static void ion_secsg_heap_free(struct ion_buffer *buffer)
{
	struct ion_heap *heap = buffer->heap;
	struct ion_secsg_heap *secsg_heap =
		container_of(heap, struct ion_secsg_heap, heap);

	sec_debug("%s:enter, heap %d, free size:0x%zx,\n",
		    __func__, heap->id, buffer->size);
	mutex_lock(&secsg_heap->mutex);
	__secsg_free_scatter(secsg_heap, buffer);
	sec_debug("out %s:heap remaining allocate %lx\n",
		    __func__, secsg_heap->alloc_size);
	mutex_unlock(&secsg_heap->mutex);
}

static int ion_secsg_heap_map_user(struct ion_heap *heap,
				    struct ion_buffer *buffer,
				    struct vm_area_struct *vma)
{
	struct ion_secsg_heap *secsg_heap =
		container_of(heap, struct ion_secsg_heap, heap);

	if (secsg_heap->heap_attr == SEC_DRM_TEE) {
		pr_err("heap(%d) DRM don't support map user\n", heap->id);
		return -EINVAL;
	}

	return ion_heap_map_user(heap, buffer, vma);
}

static void *ion_secsg_heap_map_kernel(struct ion_heap *heap,
				       struct ion_buffer *buffer)
{
	struct ion_secsg_heap *secsg_heap =
		container_of(heap, struct ion_secsg_heap, heap);

	if (secsg_heap->heap_attr == SEC_DRM_TEE) {
		pr_err("heap(%d) DRM don't support map user\n", heap->id);
		return ERR_PTR(-EINVAL);
	}

	return ion_heap_map_kernel(heap, buffer);
}

static void ion_secsg_heap_unmap_kernel(struct ion_heap *heap,
					struct ion_buffer *buffer)
{
	struct ion_secsg_heap *secsg_heap =
		container_of(heap, struct ion_secsg_heap, heap);

	if (secsg_heap->heap_attr == SEC_DRM_TEE) {
		pr_err("heap(%d) DRM don't support map user\n", heap->id);
		return;
	}

	ion_heap_unmap_kernel(heap, buffer);
}

static struct ion_heap_ops secsg_heap_ops = {
	.allocate = ion_secsg_heap_allocate,
	.free = ion_secsg_heap_free,
	.map_kernel = ion_secsg_heap_map_kernel,
	.unmap_kernel = ion_secsg_heap_unmap_kernel,
	.map_user = ion_secsg_heap_map_user,
};

static int __secsg_parse_dt(struct device *dev,
			    struct ion_platform_heap *heap_data,
			    struct ion_secsg_heap *secsg_heap)
{
	struct device_node *nd = NULL;
	u64 per_bit_sz = 0;
	u64 per_alloc_sz = 0;
	u32 heap_attr = 0;
	u32 pool_flag = 0;
	int ret = 0;

	nd = of_get_child_by_name(dev->of_node, heap_data->name);
	if (!nd) {
		pr_err("can't of_get_child_by_name %s\n", heap_data->name);
		ret = -EINVAL;
		goto out;
	}

	ret = of_property_read_u32(nd, "heap-attr", &heap_attr);
	if (ret < 0) {
		pr_err("can not find heap-attr.\n");
		goto out;
	}

	if (heap_attr >= SEC_SVC_MAX) {
		pr_err("invalid heap-attr.\n");
		goto out;
	}

	secsg_heap->heap_attr = heap_attr;

	ret = of_property_read_u64(nd, "per-alloc-size", &per_alloc_sz);
	if (ret < 0) {
		pr_err("can't find prop:per-alloc-size\n");
		goto out;
	}
	secsg_heap->per_alloc_sz = PAGE_ALIGN(per_alloc_sz);

	ret = of_property_read_u64(nd, "per-bit-size", &per_bit_sz);
	if (ret < 0) {
		pr_err("can't find prop:per-bit-size\n");
		goto out;
	}
	secsg_heap->per_bit_sz = PAGE_ALIGN(per_bit_sz);

	ret = of_property_read_u32(nd, "use-static-pool", &pool_flag);
	if (ret < 0) {
		pr_err("can not find static-pool.\n");
		pool_flag = 0;
		ret = 0;
	}
	secsg_heap->pool_flag = pool_flag;

out:
	return ret;
}

static int secsg_create_pool(struct ion_secsg_heap *secsg_heap)
{
	struct page *page = NULL;
	u64 cma_base;
	u64 cma_size;

	cma_base = cma_get_base(secsg_heap->cma);
	cma_size = cma_get_size(secsg_heap->cma);

	secsg_heap->pool = gen_pool_create(get_order(SZ_2M), -1);
	if (!secsg_heap->pool) {
		pr_err("create pool failed\n");
		return -ENOMEM;
	}
	gen_pool_set_algo(secsg_heap->pool, gen_pool_best_fit, NULL);

	page = cma_alloc(secsg_heap->cma, cma_size >> PAGE_SHIFT, 0, false);
	if (!page) {
		pr_err("%s cma_alloc failed!cma_base 0x%llx cma_size 0x%llx\n",
			__func__, cma_base, cma_size);
		goto destory_pool;
	}

	if (gen_pool_add(secsg_heap->pool, cma_base, cma_size, -1)) {
		pr_err("genpool add base 0x%llx cma_size 0x%llx\n",
			cma_base, cma_size);
		goto free_cma;
	}

	return 0;

free_cma:
	cma_release(secsg_heap->cma, page, (u32)cma_size >> PAGE_SHIFT);
destory_pool:
	gen_pool_destroy(secsg_heap->pool);

	return -ENOMEM;
}

struct ion_heap *ion_secsg_heap_create(struct ion_platform_heap *heap_data)
{
	struct ion_secsg_heap *secsg_heap = NULL;
	struct device *dev = NULL;
	int ret;

	secsg_heap = kzalloc(sizeof(*secsg_heap), GFP_KERNEL);
	if (!secsg_heap)
		return ERR_PTR(-ENOMEM);

	mutex_init(&secsg_heap->mutex);

	secsg_heap->heap.ops = &secsg_heap_ops;
	secsg_heap->heap.type = ION_HEAP_TYPE_SECSG;
	secsg_heap->heap_size = heap_data->size;
	secsg_heap->alloc_size = 0;
	dev = heap_data->priv;

	ret = __secsg_parse_dt(dev, heap_data, secsg_heap);
	if (ret)
		goto free_heap;

	secsg_heap->cma = get_svc_cma((int)secsg_heap->heap_attr);
	if (!secsg_heap->cma) {
		pr_err("can't get heap[%u]'s cma!\n", heap_data->id);
		goto free_heap;
	}

	if (secsg_heap->pool_flag) {
		ret = secsg_create_pool(secsg_heap);
		if (ret) {
			pr_err("can't crate heap[%u]'s pool!\n", heap_data->id);
			goto free_heap;
		}
	}

	if (secsg_heap->heap_attr == SEC_DRM_TEE) {
		secsg_heap->context = kzalloc(sizeof(TEEC_Context), GFP_KERNEL);
		if (!secsg_heap->context)
			goto free_heap;

		secsg_heap->session = kzalloc(sizeof(TEEC_Session), GFP_KERNEL);
		if (!secsg_heap->session)
			goto free_context;
	}

	secsg_heap->TA_init = 0;

	pr_err("secsg heap info %s:\n"
		  "\t\t\t\t\t\t\t heap id : %u\n"
		  "\t\t\t\t\t\t\t heap attr : %u\n"
		  "\t\t\t\t\t\t\t heap size : %lu MB\n"
		  "\t\t\t\t\t\t\t per bit size : %llu KB\n"
		  "\t\t\t\t\t\t\t per alloc size : %llu KB\n"
		  "\t\t\t\t\t\t\t heap pool flag : %u\n"
		  "\t\t\t\t\t\t\t cma base : 0x%llx\n"
		  "\t\t\t\t\t\t\t cma size : 0x%lx\n",
		  heap_data->name,
		  heap_data->id,
		  secsg_heap->heap_attr,
		  secsg_heap->heap_size / SZ_1M,
		  secsg_heap->per_bit_sz / SZ_1K,
		  secsg_heap->per_alloc_sz / SZ_1K,
		  secsg_heap->pool_flag,
		  cma_get_base(secsg_heap->cma),
		  cma_get_size(secsg_heap->cma));

	return &secsg_heap->heap;

free_context:
	if (secsg_heap->context)
		kfree(secsg_heap->context);
free_heap:
	kfree(secsg_heap);
	return ERR_PTR(-ENOMEM);
}
