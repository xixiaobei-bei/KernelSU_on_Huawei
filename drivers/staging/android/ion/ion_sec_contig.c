/* Copyright (c) Hisilicon Technologies Co., Ltd. 2001-2019. All rights reserved.
 * FileName: ion_sec_contig.c
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
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/cma.h>
#include <linux/sizes.h>
#include <linux/list.h>
#include <linux/version.h>
#include <linux/hisi/hisi_cma.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi/hisi_mm.h>

#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#include "ion.h"
#include "hisi/ion_tee_op.h"
#include "hisi/ion_sec_contig.h"
#include "hisi_ion_priv.h"


static inline void free_alloc_list(struct list_head *head)
{
	struct alloc_list *pos = NULL;

	while (!list_empty(head)) {
		pos = list_first_entry(head, struct alloc_list, list);
		if (pos->addr || pos->size)
			pr_err("in %s %pK %x failed\n", __func__,
			       (void *)pos->addr, pos->size);

		list_del(&pos->list);
		kfree(pos);
	}
}

static u32 count_list_nr(struct ion_seccg_heap *seccg_heap)
{
	u32 nr = 0;
	struct list_head *head = &seccg_heap->allocate_head;
	struct list_head *pos = NULL;

	list_for_each(pos, head)
		nr++;
	return nr;
}

static struct page *__seccg_cma_alloc(struct ion_seccg_heap *seccg_heap,
				      size_t count, u64 size,
				      unsigned int align)
{
	unsigned long offset = 0;

	if (!seccg_heap->static_cma_region)
		return cma_alloc(seccg_heap->cma, count, align, false);

	offset = gen_pool_alloc(seccg_heap->static_cma_region,
				size);
	if (!offset) {
		pr_err("__cma_alloc failed!size:0x%llx\n", size);
		return NULL;
	}

	return pfn_to_page(PFN_DOWN(offset));
}

static bool __seccg_cma_release(struct ion_seccg_heap *seccg_heap,
				const struct page *pages,
				unsigned int count,
				u64 size)
{
	if (!seccg_heap->static_cma_region)
		return cma_release(seccg_heap->cma, pages, count);

	gen_pool_free(seccg_heap->static_cma_region, page_to_phys(pages),
		      size);

	return 0;
}

static int cons_phys_struct(struct ion_seccg_heap *seccg_heap, u32 nents,
			    struct list_head *head, u32 cmd_to_ta)
{
	struct tz_pageinfo *pageinfo = NULL;
	struct mem_chunk_list mcl;
	struct list_head *pos = head->prev;
	struct alloc_list *tmp_list = NULL;
	unsigned long size = nents * sizeof(*pageinfo);
	u32 i;
	int ret = 0;

	if (!seccg_heap->TA_init) {
		pr_err("[%s] TA not inited.\n", __func__);
		return -EINVAL;
	}

	pageinfo = kzalloc(size, GFP_KERNEL);
	if (!pageinfo)
		return -ENOMEM;

	for (i = 0; (i < nents) && (pos != head); i++) {
		tmp_list = list_entry(pos, struct alloc_list, list);
		pageinfo[i].addr = tmp_list->addr;
		pageinfo[i].nr_pages = tmp_list->size / PAGE_SIZE;
		pos = pos->prev;
	}

	if (i < nents) {
		pr_err("[%s], invalid nents(%d) or head!\n", __func__, nents);
		ret = -EINVAL;
		goto out;
	}

	/*Call TZ Driver here*/
	mcl.nents = nents;
	mcl.phys_addr = (void *)pageinfo;
	mcl.protect_id = SEC_TASK_SEC;
	mcl.size = nents * sizeof(struct tz_pageinfo);
	ret = secmem_tee_exec_cmd(seccg_heap->session, &mcl, cmd_to_ta);
out:
	kfree(pageinfo);
	return ret;
}

static int alloc_range_update(struct page *pg, u64 size,
			struct ion_seccg_heap *seccg_heap, unsigned int count)
{
	int ret = 0;
	struct alloc_list *alloc = NULL;
	unsigned long virt;

	alloc = kzalloc(sizeof(*alloc), GFP_KERNEL);
	if (!alloc) {
		ret = -ENOMEM;
		goto err_out1;
	}
	alloc->addr = page_to_phys(pg);
	alloc->size = (unsigned int)size;
	list_add_tail(&alloc->list, &seccg_heap->allocate_head);

	if (seccg_heap->heap_attr == HEAP_SECURE_TEE) {
		virt = (unsigned long)__va(alloc->addr);
#ifdef CONFIG_NEED_CHANGE_MAPPING
		change_secpage_range(alloc->addr, virt, size,
				     __pgprot(PROT_DEVICE_nGnRE));
		flush_tlb_all();
#endif
		ion_flush_all_cpus_caches();
		if (cons_phys_struct(seccg_heap, 1,
				     &seccg_heap->allocate_head,
				     ION_SEC_CMD_TABLE_SET)) {
			pr_err("cons_phys_struct failed\n");
			ret = -EINVAL;
			goto err_out2;
		}
	}

	seccg_heap->cma_alloc_size +=  size;
	gen_pool_free(seccg_heap->pool, page_to_phys(pg), size);
	sec_debug("out %s %llu MB memory(ret = %d).\n",
		    __func__, size / SZ_1M, ret);
	return 0;
err_out2:
#ifdef CONFIG_NEED_CHANGE_MAPPING
	change_secpage_range(alloc->addr, virt, size,
			     PAGE_KERNEL);
	flush_tlb_all();
#endif
	list_del(&alloc->list);
	kfree(alloc);
err_out1:
	__seccg_cma_release(seccg_heap, pg, count, size);
	return ret;
}

static int __add_cma_to_pool(struct ion_seccg_heap *seccg_heap,
			   unsigned long user_alloc_size)
{
	int ret = 0;
	unsigned long size_remain;
	unsigned long allocated_size;
	unsigned long cma_remain;
	u64 size = seccg_heap->per_alloc_sz;
	u64 per_bit_sz = seccg_heap->per_bit_sz;
	u64 cma_size;
	/* Add for TEEOS ++, per_alloc_size = 64M */
	struct page *pg = NULL;
	unsigned int count = 0;
	/* Add for TEEOS -- */
#ifdef CONFIG_HISI_KERNELDUMP
	unsigned int k;
	struct page *tmp_page = NULL;
#endif

	sec_debug("into %s\n", __func__);
	/* add 64M for every times
	 * per_alloc_sz = 64M, per_bit_sz = 16M(the original min_size)
	 */
	allocated_size = seccg_heap->alloc_size;
	size_remain = gen_pool_avail(seccg_heap->pool);
	cma_size = cma_get_size(seccg_heap->cma);
	cma_remain = cma_size - (allocated_size + size_remain);
	if (seccg_heap->heap_size <= (allocated_size + size_remain)) {
		pr_err("heap full! allocated_size(0x%lx), remain_size(0x%lx),"
		       " heap_size(0x%lx), cma_remain(0x%lx)\n", allocated_size,
		       size_remain, seccg_heap->heap_size, cma_remain);
		return -ENOMEM;
	}

	/* we allocate as much contiguous memory as we can. */
	count = (unsigned int)size >> PAGE_SHIFT;

	pg = __seccg_cma_alloc(seccg_heap, (size_t)count, size,
			       (u32)get_order((u32)per_bit_sz));

	if (!pg) {
		size = ALIGN(user_alloc_size, per_bit_sz);
		count = (unsigned int)size >> PAGE_SHIFT;
		pg = __seccg_cma_alloc(seccg_heap, (size_t)count, size,
				       (u32)get_order((u32)per_bit_sz));
		if (!pg) {
			pr_err("out of memory,cma:0x%llx,type:%u,usesize:0x%lx\n",
			       cma_size, seccg_heap->cma_type, allocated_size);
			return -ENOMEM;
		}
	}

#ifdef CONFIG_HISI_KERNELDUMP
	tmp_page = pg;
	for (k = 0; k < count; k++) {
		SetPageMemDump(tmp_page);
		tmp_page++;
	}
#endif
	ret = alloc_range_update(pg, size, seccg_heap, count);
	if (ret)
		return ret;
	return 0;
}

void seccg_pre_alloc_wk_func(struct work_struct *work)
{
	struct ion_seccg_heap *seccg_heap = NULL;
	int ret = 0;

	seccg_heap = container_of(work, struct ion_seccg_heap, pre_alloc_work);
	mutex_lock(&seccg_heap->pre_alloc_mutex);
	/*
	 * For HEAP_SECURE_TEE attr, we want to pre-allocate every time.
	 */
	if (seccg_heap->cma_alloc_size < seccg_heap->heap_size) {
		ret = __add_cma_to_pool(seccg_heap, seccg_heap->per_alloc_sz);
		if (ret)
			pr_err("pre alloc cma to fill heap pool failed!"
			"cma allocated sz(0x%lx), per alloc sz(0x%llx)\n",
			seccg_heap->cma_alloc_size, seccg_heap->per_alloc_sz);
	}
	mutex_unlock(&seccg_heap->pre_alloc_mutex);
}

static int add_cma_to_pool(struct ion_seccg_heap *seccg_heap,
			   unsigned long size)
{
	int ret = 0;

	if (seccg_heap->pre_alloc_attr) {
		mutex_lock(&seccg_heap->pre_alloc_mutex);
		if (gen_pool_avail(seccg_heap->pool) < size &&
			seccg_heap->cma_alloc_size < seccg_heap->heap_size)
			ret = __add_cma_to_pool(seccg_heap, size);
		mutex_unlock(&seccg_heap->pre_alloc_mutex);
	} else {
		ret = __add_cma_to_pool(seccg_heap, size);
	}

	return ret;
}

static bool gen_pool_bulk_free(struct gen_pool *pool, u32 size)
{
	u32 i;
	unsigned long offset = 0;

	for (i = 0; i < (size / PAGE_SIZE); i++) {
		offset = gen_pool_alloc(pool, PAGE_SIZE);
		if (!offset) {
			pr_err("%s:%d:gen_pool_alloc failed!\n",
			       __func__, __LINE__);
			return false;
		}
	}
	return true;
}

static void __seccg_pool_release(struct ion_seccg_heap *seccg_heap)
{
	u32 nents;
	u64 addr;
	u32 size;
	unsigned long virt;
	unsigned long size_remain = 0;
	unsigned long offset = 0;
	struct alloc_list *pos = NULL;

	if (seccg_heap->heap_attr == HEAP_SECURE_TEE) {
		nents = count_list_nr(seccg_heap);
		if (nents &&
		    cons_phys_struct(seccg_heap, nents,
				     &seccg_heap->allocate_head,
				     ION_SEC_CMD_TABLE_CLEAN)) {
			pr_err("heap_type:(%u)unconfig failed!!!\n",
			       seccg_heap->heap_attr);
			goto out;
		}
	}

	if (!list_empty(&seccg_heap->allocate_head)) {
		list_for_each_entry(pos, &seccg_heap->allocate_head, list) {
			addr = pos->addr;
			size = pos->size;
			offset = gen_pool_alloc(seccg_heap->pool, size);
			if (!offset) {
				pr_err("%s:%d:gen_pool_alloc failed! %llx %x\n",
				       __func__, __LINE__, addr, size);
				continue;
			}
			virt = (unsigned long)__va(addr);
#ifdef CONFIG_NEED_CHANGE_MAPPING
			if (seccg_heap->flag & ION_FLAG_SECURE_BUFFER) {
				change_secpage_range(addr, virt, size,
						     PAGE_KERNEL);
				flush_tlb_all();
			}
#endif

			__seccg_cma_release(seccg_heap, phys_to_page(addr),
					    size >> PAGE_SHIFT, size);
			pos->addr = 0;
			pos->size = 0;
		}
	}

	if (!list_empty(&seccg_heap->allocate_head)) {
		list_for_each_entry(pos, &seccg_heap->allocate_head, list) {
			addr = pos->addr;
			size = pos->size;
			if (!addr || !size)
				continue;

			if (unlikely(!gen_pool_bulk_free(seccg_heap->pool,
							 size))) {
				pr_err("%s:%d:bulk_free failed! %llx %x\n",
				       __func__, __LINE__, addr, size);
				continue;
			}

			virt = (unsigned long)__va(addr);
#ifdef CONFIG_NEED_CHANGE_MAPPING
			if (seccg_heap->flag & ION_FLAG_SECURE_BUFFER) {
				change_secpage_range(addr, virt, size,
						     PAGE_KERNEL);
				flush_tlb_all();
			}
#endif
			__seccg_cma_release(seccg_heap, phys_to_page(addr),
					    size >> PAGE_SHIFT, size);
			pos->addr = 0;
			pos->size = 0;
		}
		free_alloc_list(&seccg_heap->allocate_head);
	}
#ifdef CONFIG_NEED_CHANGE_MAPPING
	flush_tlb_all();
#endif
out:
	size_remain = gen_pool_avail(seccg_heap->pool);
	if (size_remain)
		pr_err("out %s, size_remain = 0x%lx(0x%lx)\n",
		       __func__, size_remain, offset);
}

int __seccg_alloc_contig(struct ion_seccg_heap *seccg_heap,
			 struct ion_buffer *buffer, unsigned long size)
{
	int ret = 0;
	unsigned long offset = 0;
	struct sg_table *table = NULL;
	struct page *page = NULL;

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	if (sg_alloc_table(table, 1, GFP_KERNEL)) {
		pr_err("[%s] sg_alloc_table failed .\n", __func__);
		ret = -ENOMEM;
		goto err_out1;
	}
	/*align size*/
	offset = gen_pool_alloc(seccg_heap->pool, size);
	if (!offset) {
		ret = add_cma_to_pool(seccg_heap, size);
		if (ret)
			goto err_out2;
		offset = gen_pool_alloc(seccg_heap->pool, size);
		if (!offset) {
			ret = -ENOMEM;
			pr_err("line %d, gen_pool_alloc failed!\n", __LINE__);
			goto err_out2;
		}
	}

	if (seccg_heap->pre_alloc_attr &&
		    seccg_heap->cma_alloc_size < seccg_heap->heap_size)
		schedule_work(&seccg_heap->pre_alloc_work);

	page = pfn_to_page(PFN_DOWN(offset));
	if (seccg_heap->heap_attr == HEAP_NORMAL) {
		(void)ion_heap_pages_zero(page, size,
					  pgprot_writecombine(PAGE_KERNEL));
		ion_flush_all_cpus_caches();
	}
	sg_set_page(table->sgl, page, (unsigned int)size, 0);
	buffer->sg_table = table;
	if (seccg_heap->heap_attr != HEAP_NORMAL)
		pr_info("__seccg_alloc sec buffer phys %lx, size %lx\n",
			offset, size);

	sec_debug(" out [%s].\n", __func__);
	return ret;
err_out2:
	sg_free_table(table);
err_out1:
	kfree(table);
	return ret;
}

static void __seccg_free_pool(struct ion_seccg_heap *seccg_heap,
			      struct sg_table *table,
			      struct ion_buffer *buffer)
{
	struct page *page = sg_page(table->sgl);
	phys_addr_t paddr = PFN_PHYS(page_to_pfn(page));
	struct platform_device *hisi_ion_dev = get_hisi_ion_platform_device();

	if (!(buffer->flags & ION_FLAG_SECURE_BUFFER)) {
		(void)ion_heap_buffer_zero(buffer);
		if (buffer->flags & ION_FLAG_CACHED)
			dma_sync_sg_for_device(&hisi_ion_dev->dev, table->sgl,
					       (int)table->nents,
					       DMA_BIDIRECTIONAL);
	}
	gen_pool_free(seccg_heap->pool, paddr, buffer->size);
	if (seccg_heap->heap_attr != HEAP_NORMAL)
		pr_info("__seccg_free sec buffer phys %lx, size %zx\n",
			(unsigned long)paddr, buffer->size);

	sg_free_table(table);
	kfree(table);

	sec_debug("out %s\n", __func__);
}

int __seccg_fill_watermark(struct ion_seccg_heap *seccg_heap)
{
	struct page *pg = NULL;
	u64 size = seccg_heap->water_mark;
	u64 per_bit_sz = seccg_heap->per_bit_sz;
	struct alloc_list *alloc = NULL;
	u32 count = (u32)size >> PAGE_SHIFT;
	u32 align = (u32)get_order((u32)per_bit_sz);
#ifdef CONFIG_HISI_KERNELDUMP
	u32 k;
	struct page *tmp_page = NULL;
#endif

	if (!size || size > seccg_heap->per_alloc_sz)
		return -EINVAL;

	alloc = kzalloc(sizeof(*alloc), GFP_KERNEL);
	if (!alloc)
		return -ENOMEM;

	pg = __seccg_cma_alloc(seccg_heap, (size_t)count, size, align);
	if (!pg) {
		pr_err("%s:alloc cma fail\n", __func__);
		kfree(alloc);
		return -ENOMEM;
	}

#ifdef CONFIG_HISI_KERNELDUMP
	tmp_page = pg;
	for (k = 0; k < count; k++) {
		SetPageMemDump(tmp_page);
		tmp_page++;
	}
#endif

	alloc->addr = page_to_phys(pg);
	alloc->size = (unsigned int)size;
	list_add_tail(&alloc->list, &seccg_heap->allocate_head);

	memset(page_address(pg), 0x0, size);/* unsafe_function_ignore: memset */
	gen_pool_free(seccg_heap->pool, page_to_phys(pg), size);
	sec_debug("out %s %llu MB memory.\n",
		    __func__, (size) / SZ_1M);
	return 0;
}

void __seccg_free_contig(struct ion_seccg_heap *seccg_heap,
			 struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;

	__seccg_free_pool(seccg_heap, table, buffer);
	WARN_ON(seccg_heap->alloc_size < buffer->size);
	seccg_heap->alloc_size -= buffer->size;
	if (!seccg_heap->alloc_size) {
		if (seccg_heap->pre_alloc_attr)
			cancel_work_sync(&seccg_heap->pre_alloc_work);
		__seccg_pool_release(seccg_heap);
		seccg_heap->cma_alloc_size = 0;
		if (seccg_heap->water_mark &&
		    __seccg_fill_watermark(seccg_heap))
			pr_err("__seccg_fill_watermark failed!\n");
	}
}
