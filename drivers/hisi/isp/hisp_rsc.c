/*
 *  driver, hisp_rsc.c
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
#include <linux/dma-mapping.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/platform_data/hisp_mempool.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/scatterlist.h>
#include <linux/rproc_share.h>
#include <securec.h>

#define ISP_RSC_TABLE_SIZE (0x1000)
#define ISP_RDR_SIZE        (0x40000)

/**
 * rproc_handle_version() - handle the verison information
 * @rproc: the remote processor
 * @rsc: the trace resource descriptor
 * @avail: size of available data (for sanity checking the image)
 */
int rproc_handle_version(struct rproc *rproc, struct fw_rsc_hdr *hdr,
		int offset, int avail)
{
	struct fw_rsc_version *rsc = NULL;

	/* make sure hdr isn't NULL */
	if (hdr == NULL) {
		pr_err("%s: hdr is NULL!\n", __func__);
		return -EINVAL;
	}
	rsc = (struct fw_rsc_version *)((void *)hdr + sizeof(*hdr));

	if (!rproc || !rsc) {
		pr_err("%s: rproc or rsc is NULL!\n", __func__);
		return -ENOENT;
	}

	pr_info("[%s] Firmware_version: magic.%x, module.%s, version.%s\n",
		__func__, rsc->magic, rsc->module, rsc->version);
	pr_info("[%s] Firmware_version: build_time = %s.\n",
		__func__, rsc->build_time);

	return 0;
}

/* page order */
static const unsigned int orders[] = {8, 4, 0};
#define ISP_NUM_ORDERS ARRAY_SIZE(orders)

/* dynamic alloc page and creat sg_table */
static inline unsigned int order_to_size(int order)
{
	return PAGE_SIZE << order;
}
/* try to alloc largest page orders */
static struct page *alloc_largest_pages(gfp_t gfp_mask, unsigned long size,
				unsigned int max_order, unsigned int *order)
{
	struct page *page   = NULL;
	gfp_t gfp_flags     = 0;
	int i               = 0;

	if (order == NULL) {
		pr_err("%s: order is NULL\n", __func__);
		return NULL;
	}

	for (i = 0; i < ISP_NUM_ORDERS; i++) {/*lint !e574*/
		if (size < order_to_size(orders[i]))
			continue;
		if (max_order < orders[i])
			continue;

		if (orders[i] >= 8) {
			gfp_flags = gfp_mask & (~__GFP_RECLAIM);
			gfp_flags |= __GFP_NOWARN;
			gfp_flags |= __GFP_NORETRY;
		} else if (orders[i] >= 4) {
			gfp_flags = gfp_mask & (~__GFP_DIRECT_RECLAIM);
			gfp_flags |= __GFP_NOWARN;
			gfp_flags |= __GFP_NORETRY;
		} else {
			gfp_flags = gfp_mask;
		}

		page = alloc_pages(gfp_flags, orders[i]);
		if (page == NULL)
			continue;
		*order = orders[i];

		return page;
	}

	return NULL;
}

static int isp_mem_page_allocate(unsigned long length,
	struct list_head *pages_list, int *alloc_pages_num)
{
	struct rproc_page_info *page_info = NULL;
	unsigned int max_order       = orders[0];
	unsigned long size_remaining = PAGE_ALIGN(length);

	INIT_LIST_HEAD(pages_list);

	while (size_remaining > 0) {
		page_info = kzalloc(sizeof(struct rproc_page_info), GFP_KERNEL);
		if (page_info == NULL)
			return -1;
		page_info->page = alloc_largest_pages(GFP_KERNEL,
			size_remaining, max_order, &page_info->order);
		if (!page_info->page) {
			pr_err("%s: alloc largest pages failed!\n", __func__);
			kfree(page_info);
			return -1;
		}

		list_add_tail(&page_info->node, pages_list);
		size_remaining -= PAGE_SIZE << page_info->order;
		max_order = page_info->order;
		(*alloc_pages_num)++;
	}

	return 0;
}

/* dynamic alloc page and creat sg_table */
static struct sg_table *isp_mem_sg_table_allocate(unsigned long length)
{
	struct list_head pages_list             = { 0 };
	struct rproc_page_info *page_info       = NULL;
	struct rproc_page_info *tmp_page_info   = NULL;
	struct sg_table *table       = NULL;
	struct scatterlist *sg       = NULL;
	int alloc_pages_num          = 0;
	int ret;

	INIT_LIST_HEAD(&pages_list);

	ret = isp_mem_page_allocate(length, &pages_list, &alloc_pages_num);
	if (ret < 0) {
		pr_err("%s: isp_mem_page_allocate fail\n", __func__);
		goto free_pages;
	}

	table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (table == NULL)
		goto free_pages;

	if (sg_alloc_table(table, alloc_pages_num, GFP_KERNEL))
		goto free_table;

	sg = table->sgl;
	list_for_each_entry_safe(page_info, tmp_page_info, &pages_list, node) {
		sg_set_page(sg, page_info->page,
			PAGE_SIZE << page_info->order, 0);
		sg = sg_next(sg);
		list_del(&page_info->node);
		kfree(page_info);
	}
	pr_debug("%s: pages num = %d, length = 0x%lx\n",
		__func__, alloc_pages_num, length);
	return table;

free_table:
	kfree(table);
free_pages:
	list_for_each_entry_safe(page_info, tmp_page_info, &pages_list, node) {
		__free_pages(page_info->page, page_info->order);
		list_del(&page_info->node);
		kfree(page_info);
	}

	return NULL;
}

/* dynamic free page and sg_table */
static void isp_mem_sg_table_free(struct sg_table *table)
{
	struct scatterlist *sg  = NULL;
	int i                   = 0;

	if (table == NULL) {
		pr_err("%s: table is NULL\n", __func__);
		return;
	}
	pr_debug("%s: table = 0x%pK\n", __func__, table);
	for_each_sg(table->sgl, sg, table->nents, i) {/*lint !e574*/
		__free_pages(sg_page(sg), get_order(PAGE_ALIGN(sg->length)));
	}
	sg_free_table(table);
	kfree(table);
}

static void isp_mem_map_kernel_sub(struct sg_table *table,
	int npages, struct page **pages)
{
	int i, j;
	struct scatterlist *sg  = NULL;
	int npages_this_entry   = 0;
	struct page *page       = NULL;

	if ((table == NULL) || (pages == NULL)) {
		pr_err("%s: table or pages is NULL\n", __func__);
		return;
	}

	for_each_sg(table->sgl, sg, table->nents, i) {/*lint !e574*/
		npages_this_entry = PAGE_ALIGN(sg->length) / PAGE_SIZE;
		page = sg_page(sg);
		BUG_ON(i >= npages);
		for (j = 0; j < npages_this_entry; j++)
			*(pages++) = page++;/*lint !e613*/
	}
}

/* dynamic map kernel addr with sg_table */
static void *isp_mem_map_kernel(struct sg_table *table, unsigned long length)
{
	void *vaddr             = NULL;
	int npages = PAGE_ALIGN(length) / PAGE_SIZE;
	struct page **pages;

	pages = vmalloc(sizeof(struct page *) * npages);
	if (pages == NULL)
		return NULL;

	isp_mem_map_kernel_sub(table, npages, pages);

	vaddr = vmap(pages, npages, VM_MAP, PAGE_KERNEL);/*lint !e446 */
	vfree(pages);
	if (vaddr == NULL) {
		pr_err("%s: vmap failed.\n", __func__);
		return NULL;
	}
	return vaddr;
}
/* dynamic unmap kernel addr with sg_table */
static void isp_mem_unmap_kernel(const void *va)
{
	if (va == NULL) {
		pr_err("%s: va is NULL\n", __func__);
		return;
	}
	vunmap(va);
}
/* dynamic sg_table list_add_tail in page list */
/*lint -e429*/
static int isp_mem_add_page_list(struct rproc *rproc, struct sg_table *table)
{
	struct rproc_page *r_page    = NULL;/*lint !e429 */

	if (!table || !rproc) {
		pr_err("%s: table or rproc is NULL!\n", __func__);
		return -ENOENT;
	}

	r_page = kzalloc(sizeof(struct rproc_page), GFP_KERNEL);
	if (r_page == NULL)
		return -ENOMEM;

	r_page->table = table;

	list_add_tail(&r_page->node, &rproc->pages);
	return 0;
}
static void isp_mem_free_page_list(struct rproc *rproc)
{
	struct rproc_page *page_entry = NULL, *page_tmp = NULL;

	list_for_each_entry_safe(page_entry, page_tmp, &rproc->pages, node) {
		list_del(&page_entry->node);
		kfree(page_entry);
	}
}

static int rproc_dynamic_memory_mapping(struct rproc *rproc,
	struct fw_rsc_dynamic_memory *rsc, struct sg_table *table)
{
	int ret = -1;
	struct device *dev = NULL;
	struct rproc_mem_entry *mapping = NULL;

	if ((rproc == NULL) || (rsc == NULL) || (table == NULL)) {
		pr_err("%s: rproc.rsc.table is NULL!\n", __func__);
		return -ENOENT;
	}

	dev = &rproc->dev;

	if (rproc->domain) {
		mapping = kzalloc(sizeof(*mapping), GFP_KERNEL);
		if (mapping == NULL)
			return -ENOMEM;

		if ((IOMMU_MASK & rsc->flags) != 0) {
			dev_err(dev, "iommu attr error.0x%x\n", rsc->flags);
			ret = -ENOMEM;
			goto free_mapping;
		}

		ret = iommu_map_sg(rproc->domain, rsc->da, table->sgl,
				sg_nents(table->sgl), rsc->flags);
		if (ret != rsc->len) {
			dev_err(dev, "isp_iommu_map_range failed: ret %d len %d\n",
					ret, rsc->len);
			goto free_mapping;
		}

		/*
		 * We'll need this info later when we'll want to unmap
		 * everything (e.g. on shutdown).
		 *
		 * We can't trust the remote processor not to change the
		 * resource table, so we must maintain this info independently.
		 */
		mapping->da = rsc->da;
		mapping->len = rsc->len;
		list_add_tail(&mapping->node, &rproc->mappings);
	}

	return 0;

free_mapping:
	kfree(mapping);
	return ret;
}

static int rproc_dynamic_memory_entry(struct rproc *rproc, void *va,
	struct fw_rsc_dynamic_memory *rsc)
{
	int ret;
	struct device *dev = NULL;
	struct rproc_mem_entry *dynamic_mem = NULL;
	struct rproc_cache_entry *cache_entry = NULL;


	if ((rproc == NULL) || (rsc == NULL) || (va == NULL)) {
		pr_err("%s: rproc.rsc.va is NULL!\n", __func__);
		return -ENOENT;
	}

	dev = &rproc->dev;

	cache_entry = kzalloc(sizeof(*cache_entry), GFP_KERNEL);
	if (cache_entry == NULL)
		return -ENOMEM;

	dynamic_mem = kzalloc(sizeof(*dynamic_mem), GFP_KERNEL);
	if (dynamic_mem == NULL) {
		ret = -ENOMEM;
		goto free_cache;
	}

	dynamic_mem->va = va;
	dynamic_mem->len = rsc->len;
	dynamic_mem->dma = rsc->da;
	dynamic_mem->da = rsc->da;
	dynamic_mem->priv = rsc->name;

	list_add_tail(&dynamic_mem->node, &rproc->dynamic_mems);

	/* save cache entry */
	cache_entry->va = va;
	cache_entry->len = rsc->len;
	list_add_tail(&cache_entry->node, &rproc->caches);

	return 0;

free_cache:
	kfree(cache_entry);
	return ret;
}

static void *get_rproc_dynamic_memory_table(struct fw_rsc_dynamic_memory *rsc)
{
	struct sg_table *table = NULL;

	if (rsc == NULL) {
		pr_err("%s: rsc is NULL!\n", __func__);
		return NULL;
	}

	pr_info("%s: dynamic_mem rsc: len %x, flags %x\n", __func__,
		rsc->len, rsc->flags);

	table = isp_mem_sg_table_allocate(rsc->len);

	return table;
}

static void *get_rproc_dynamic_memory_rsc(struct fw_rsc_hdr *hdr, int avail)
{
	struct fw_rsc_dynamic_memory *rsc = NULL;

	/* make sure hdr isn't NULL */
	if (hdr == NULL) {
		pr_err("%s: hdr is NULL!\n", __func__);
		return NULL;
	}
	rsc = (struct fw_rsc_dynamic_memory *)((void *)hdr + sizeof(*hdr));

	if (rsc == NULL) {
		pr_err("%s: rsc is NULL!\n", __func__);
		return NULL;
	}

	if (sizeof(*rsc) > avail) {/*lint !e574 */
		pr_err("%s: dynamic_mem rsc is truncated\n", __func__);
		return NULL;
	}

	/* make sure reserved bytes are zeroes */
	if (rsc->reserved) {
		pr_err("%s: rsc has non zero reserved bytes\n", __func__);
		return NULL;
	}

	return rsc;
}

/**
 * rproc_handle_dynamic_memory()
 * handle phys non-contiguous memory allocation requests
 * @rproc: rproc handle
 * @rsc: the resource entry
 * @avail: size of available data (for image validation)
 *
 * This function will handle firmware requests for allocation of physically
 * contiguous memory regions.
 *
 * These request entries should come first in the firmware's resource table,
 * as other firmware entries might request placing other data objects inside
 * these memory regions (e.g. data/code segments, trace resource entries, ...).
 *
 * Allocating memory this way helps utilizing the reserved physical memory
 * (e.g. CMA) more efficiently, and also minimizes the number of TLB entries
 * needed to map it (in case @rproc is using an IOMMU). Reducing the TLB
 * pressure is important; it may have a substantial impact on performance.
 */
/*lint -save -e429*/
int rproc_handle_dynamic_memory(struct rproc *rproc,
		struct fw_rsc_hdr *hdr,	int offset, int avail)
{
	struct device *dev = NULL;
	struct fw_rsc_dynamic_memory *rsc = NULL;
	struct sg_table *table = NULL;
	void *va = NULL;
	int ret = -1;

	rsc = get_rproc_dynamic_memory_rsc(hdr, avail);
	if (!rproc || !rsc) {
		pr_err("%s: rproc or rsc is NULL!\n", __func__);
		return -ENOENT;
	}

	dev = &rproc->dev;

	table = get_rproc_dynamic_memory_table(rsc);
	if (table == NULL) {
		dev_err(dev, "%s:vaddr_to_sgl failed\n", __func__);
		return -EINVAL;
	}

	ret = isp_mem_add_page_list(rproc, table);
	if (ret < 0) {
		dev_err(dev, "isp_mem_add_page_list failed: ret %d.\n", ret);
		goto free_table;
	}

	va = isp_mem_map_kernel(table, rsc->len);
	if (va == NULL) {
		dev_err(dev, "%s:vaddr_to_sgl failed\n", __func__);
		goto free_page_list;
	}

	/* mapping */
	ret = rproc_dynamic_memory_mapping(rproc, rsc, table);
	if (ret < 0) {
		dev_err(dev, "rproc_dynamic_memory_mapping failed\n");
		goto free_map;
	}

	/*
	 * Some remote processors might need to know the pa
	 * even though they are behind an IOMMU. E.g., OMAP4's
	 * remote M3 processor needs this so it can control
	 * on-chip hardware accelerators that are not behind
	 * the IOMMU, and therefor must know the pa.
	 *
	 * Generally we don't want to expose physical addresses
	 * if we don't have to (remote processors are generally
	 * _not_ trusted), so we might want to do this only for
	 * remote processor that _must_ have this (e.g. OMAP4's
	 * dual M3 subsystem).
	 *
	 * Non-IOMMU processors might also want to have this info.
	 * In this case, the device address and the physical address
	 * are the same.
	 */

	ret = rproc_dynamic_memory_entry(rproc, va, rsc);
	if (ret < 0) {
		dev_err(dev, "rproc_dynamic_memory_entry failed\n");
		goto free_map;
	}

	return 0;

free_map:
	isp_mem_unmap_kernel(va);
free_page_list:
	isp_mem_free_page_list(rproc);
free_table:
	isp_mem_sg_table_free(table);

	return ret;
}
/*lint -restore */

/* rproc_handle_rdr_memory() - handle phys rdr memory allocation requests */
/*lint -save -e429*/
int rproc_handle_rdr_memory(struct rproc *rproc,
		struct fw_rsc_hdr *hdr,
		int offset, int avail)
{
	struct rproc_mem_entry *mapping = NULL;
	struct device *dev = NULL;
	struct fw_rsc_carveout *rsc = NULL;
	int ret = -1;
	u64 isprdr_addr;

	/* make sure hdr isn't NULL */
	if (hdr == NULL) {
		pr_err("%s: hdr is NULL!\n", __func__);
		return -EINVAL;
	}
	rsc = (struct fw_rsc_carveout *)((void *)hdr + sizeof(*hdr));

	if (!rproc || !rsc) {
		pr_err("%s: rproc or rsc is NULL!\n", __func__);
		return -ENOENT;
	}

	dev = &rproc->dev;

	if (hisp_get_smmuc3_flag() == 1) {
		void __iomem *rdr_va;

		rdr_va = get_isprdr_va();
		if (rdr_va == NULL) {
			pr_err("[%s] Failed : rdr_va is NULL\n", __func__);
		} else {
			ret = memset_s(rdr_va, ISP_RDR_SIZE, 0, ISP_RDR_SIZE);
			if (ret != 0)
				pr_err("%s: memset_s rdr to 0.%d!\n",
					__func__, ret);
		}
	}

	isprdr_addr = get_isprdr_addr();
	pr_info("ispRDR ==>> (Len.0x%x, Flag.0x%x).%s\n",
			rsc->len, rsc->flags, rsc->name);
	if (isprdr_addr == 0) {
		pr_info("%s: rdr func is off.\n", __func__);
		return 0;
	}

	if (sizeof(*rsc) > avail) {/*lint !e574 */
		dev_err(dev, "memory rsc is truncated\n");
		return -EINVAL;
	}

	/* make sure reserved bytes are zeroes */
	if (rsc->reserved) {
		dev_err(dev, "memory rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	if (rproc->domain) {
		mapping = kzalloc(sizeof(*mapping), GFP_KERNEL);
		if (mapping == NULL)
			return -ENOMEM;

		ret = iommu_map(rproc->domain, rsc->da, isprdr_addr,
					rsc->len, rsc->flags);
		if (ret) {
			dev_err(dev, "iommu_map failed: %d\n", ret);
			goto free_mapping;
		}

		mapping->da = rsc->da;
		mapping->len = rsc->len;
		list_add_tail(&mapping->node, &rproc->mappings);
	}

	return 0;

free_mapping:
	kfree(mapping);

	return ret;
}
/*lint -restore */

/*
 * rproc_handle_shared_memory()
 * handle phys shared parameters memory allocation requests
 */

/*lint -save -e429*/
static int rproc_shared_memory_entry(struct rproc *rproc, void *va,
	struct fw_rsc_carveout *rsc)
{
	struct rproc_mem_entry *reserved_mem = NULL;

	reserved_mem = kzalloc(sizeof(*reserved_mem), GFP_KERNEL);
	if (reserved_mem == NULL)
		return -ENOMEM;

	reserved_mem->va = va;
	reserved_mem->len = rsc->len;
	reserved_mem->dma = rsc->da;
	reserved_mem->da = rsc->da;
	reserved_mem->priv = rsc->name;

	list_add_tail(&reserved_mem->node, &rproc->reserved_mems);
	pr_info("[%s] name.%s, (0x%x) -> %pK\n",
		__func__, rsc->name, rsc->len, va);

	return 0;
}

static void *get_rproc_shared_memory_rsc(struct fw_rsc_hdr *hdr, int avail)
{
	struct fw_rsc_carveout *rsc = NULL;

	/* make sure hdr isn't NULL */
	if (hdr == NULL) {
		pr_err("%s: hdr is NULL!\n", __func__);
		return NULL;
	}
	rsc = (struct fw_rsc_carveout *)((void *)hdr + sizeof(*hdr));

	if (rsc == NULL) {
		pr_err("%s: rproc or rsc is NULL!\n", __func__);
		return NULL;
	}

	if (sizeof(*rsc) > avail) {/*lint !e574 */
		pr_err("%s: memory rsc is truncated\n", __func__);
		return NULL;
	}

	/* make sure reserved bytes are zeroes */
	if (rsc->reserved) {
		pr_err("%s:memory rsc has non zero reserved bytes\n", __func__);
		return NULL;
	}

	pr_info("%s: name.%s, len.%x, flags.%x\n",
		__func__, rsc->name, rsc->len, rsc->flags);

	return rsc;
}

static void *get_rproc_shared_memory_va(struct fw_rsc_carveout *rsc,
			u64 a7sharedmem_addr)
{
	void *va = NULL;

	if (use_nonsec_isp()) {
		if (rsc->len != ISP_RSC_TABLE_SIZE) {
			pr_info("%s:ioremap failed a7shadmem_addr\n", __func__);
			return NULL;
		}
		va = get_a7sharedmem_va();
	} else {
		va = ioremap_wc(a7sharedmem_addr, rsc->len);/*lint !e446 */
	}
	pr_info("[%s] va.%pK\n", __func__, va);

	return va;
}

static int rproc_shared_memory_mapping(struct rproc *rproc,
	struct fw_rsc_carveout *rsc, u64 a7sharedmem_addr)
{
	int ret = -1;
	struct device *dev = NULL;
	struct rproc_mem_entry *mapping = NULL;


	if ((rproc == NULL) || (rsc == NULL)) {
		pr_err("%s: rproc.rsc is NULL!\n", __func__);
		return -ENOENT;
	}

	dev = &rproc->dev;

	if (rproc->domain) {
		mapping = kzalloc(sizeof(*mapping), GFP_KERNEL);
		if (mapping == NULL) {
			ret = -ENOMEM;
			return ret;
		}

		ret = iommu_map(rproc->domain, rsc->da, a7sharedmem_addr,
				rsc->len, rsc->flags);
		if (ret != 0) {
			dev_err(dev, "iommu_map failed: %d\n", ret);
			goto free_mapping;
		}

		mapping->da = rsc->da;
		mapping->len = rsc->len;
		list_add_tail(&mapping->node, &rproc->mappings);
	}

	return 0;

free_mapping:
	kfree(mapping);
	return ret;
}

static void hisp_update_shared_va(struct fw_rsc_carveout *rsc, void *va)
{
	u64 mdc_pa = 0;
	int ret;

	if ((va == NULL) || (rsc == NULL)) {
		pr_err("%s: va.rsc is NULL!\n", __func__);
		return;
	}

	hisp_lock_sharedbuf();
	isp_share_para = va;
	mdc_pa = isp_share_para->mdc_pa_addr;
	ret = memset_s(isp_share_para, rsc->len, 0, rsc->len);
	if (ret != 0)
		pr_err("%s: memset_s isp_share_para to 0 fail.%d!\n",
			__func__, ret);
	isp_share_para->mdc_pa_addr = mdc_pa;
	pr_info("[%s] isp_share_para.%pK, va.%pK\n",
			__func__, isp_share_para, va);
	hisp_unlock_sharedbuf();
}

int rproc_handle_shared_memory(struct rproc *rproc,
		struct fw_rsc_hdr *hdr,
		int offset, int avail)
{
	struct device *dev = NULL;
	struct fw_rsc_carveout *rsc = NULL;
	u64 a7sharedmem_addr = 0;
	void *va = NULL;
	int ret = -1;

	pr_info("%s: entern.\n", __func__);

	rsc = get_rproc_shared_memory_rsc(hdr, avail);
	if (!rproc || !rsc) {
		pr_err("%s: rproc or rsc is NULL!\n", __func__);
		return -ENOENT;
	}

	dev = &rproc->dev;

	a7sharedmem_addr = get_a7sharedmem_addr();

	va = get_rproc_shared_memory_va(rsc, a7sharedmem_addr);
	if (va == NULL) {
		dev_err(dev, "ioremap failed a7sharedmem_addr\n");
		ret = -ENOMEM;
		return ret;
	}

	ret = rproc_shared_memory_mapping(rproc, rsc, a7sharedmem_addr);
	if (ret < 0) {
		dev_err(dev, "rproc_shared_memory_mapping failed\n");
		goto free_iomem;
	}

	ret = rproc_shared_memory_entry(rproc, va, rsc);
	if (ret < 0) {
		dev_err(dev, "rproc_shared_memory_entry failed\n");
		goto free_iomem;
	}

	hisp_update_shared_va(rsc, va);
	pr_info("[%s] -\n", __func__);

	return 0;

free_iomem:
	if (!use_nonsec_isp())
		iounmap(va);

	return ret;
}

void hisp_rproc_resource_cleanup(struct rproc *rproc)
{
	struct rproc_mem_entry *entry = NULL, *tmp = NULL;
	struct rproc_page *page_entry = NULL, *page_tmp = NULL;
	struct rproc_cache_entry *cache_entry = NULL, *cache_tmp = NULL;
	struct device *dev = NULL;

	if (rproc == NULL)
		return;

	dev = &rproc->dev;
	/* clean up reserved allocations */
	list_for_each_entry_safe(entry, tmp, &rproc->reserved_mems, node) {
		hisp_lock_sharedbuf();
		if (!use_nonsec_isp())
			iounmap(entry->va);
		/* reset the share parameter pointer */
		if (!use_sec_isp()) {
			pr_info("resd_mems : %s\n", (char *)entry->priv);
			if (!strncmp(entry->priv, "ISP_SHARED_MEM",
					strlen(entry->priv)))
				isp_share_para = NULL;
		}
		hisp_unlock_sharedbuf();
		list_del(&entry->node);
		kfree(entry);
	}

	/* clean up dynamic allocations */
	list_for_each_entry_safe(entry, tmp, &rproc->dynamic_mems, node) {
		vunmap(entry->va);
		list_del(&entry->node);
		kfree(entry);
	}

	/* free page */
	list_for_each_entry_safe(page_entry, page_tmp, &rproc->pages, node) {
		isp_mem_sg_table_free(page_entry->table);
		list_del(&page_entry->node);
		kfree(page_entry);
	}

	/* clean up cache entry */
	list_for_each_entry_safe(cache_entry, cache_tmp, &rproc->caches, node) {
		list_del(&cache_entry->node);
		kfree(cache_entry);
	}

	/*clean up sec tsctable mem*/
	if (use_sec_isp())
		free_secmem_rsctable();
	else if (is_use_loadbin())
		hisp_free_rsctable();

	/* clean dynamic mem pool */
	hisp_dynamic_mem_pool_clean();
}
