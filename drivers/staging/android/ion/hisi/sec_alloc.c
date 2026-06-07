/* Copyright (c) Hisilicon Technologies Co., Ltd. 2001-2019. All rights reserved.
 * FileName: sec_alloc.c
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#define pr_fmt(fmt) "sec_alloc: " fmt

#include <linux/cma.h>
#include <linux/err.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/sizes.h>
#include <linux/hisi/hisi_ion.h>

#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#include "hisi_ion_priv.h"
#include "sec_alloc.h"

struct svc_cap {
	const char *name;
	unsigned long align;
	bool is_contig;
};

static struct cma *sec_cmas[SEC_SVC_MAX];

static struct svc_cap svc_table[] = {
	{"TUI", SZ_2M, true},
	{"eID", SZ_2M, true},
	{"tiny", SZ_8M, false},
	{"faceid", SZ_2M, false},
	{"faceid-3d", SZ_2M, false},
	{"drm_tee", SZ_2M, false},
};

static atomic_long_t svc_total_size[SEC_SVC_MAX];

unsigned long get_svc_total_size(int id)
{
	return (unsigned long)atomic_long_read(&svc_total_size[id]);
}

int hisi_sec_cma_reserve(struct reserved_mem *rmem)
{
	phys_addr_t align = PAGE_SIZE << max(MAX_ORDER - 1, pageblock_order);
	phys_addr_t mask = align - 1;
	unsigned long node = rmem->fdt_node;
	struct cma *cma = NULL;
	u64 *val = NULL;
	u64 svc_id;
	int err;

	val = (u64 *)of_get_flat_dt_prop(node, "svc-id", NULL);
	if (!val) {
		pr_err("can't get svc-id compatible\n");
		return -EINVAL;
	}

	svc_id = be64_to_cpu(*val);

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

	err = cma_init_reserved_mem(rmem->base, rmem->size, 0,
				    rmem->name, &cma);
	if (err) {
		pr_err("Reserved memory: unable to setup CMA region\n");
		return err;
	}

	set_svc_cma(svc_id, cma);

	pr_err("%s init cma:rmem->base:0x%llx, size:0x%llx\n",
	       __func__, rmem->base, rmem->size);

	return 0;
}
RESERVEDMEM_OF_DECLARE(hisi_dynamic_cma,
		       "tiny-cma-pool",
		       hisi_sec_cma_reserve);

void set_svc_cma(u64 svc_id, struct cma *cma)
{
	unsigned int i;

	for (i = 0; i < SEC_SVC_MAX; i++) {
		if (svc_id & 1UL << i) {
			sec_cmas[i] = cma;
			pr_err("%s %d: cma[%d] %pK\n",
				__func__, __LINE__, i, sec_cmas[i]);
		}
	}
}

struct cma *get_svc_cma(int id)
{
	return sec_cmas[id];
}

static struct sg_table *secmem_alloc_contig(struct cma *cma, unsigned long size,
					    unsigned long align)
{
	struct sg_table *table = NULL;
	struct page *page = NULL;
	u32 count;
#ifdef CONFIG_HISI_KERNELDUMP
	u32 k;
	struct page *tmp_page = NULL;
#endif

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return NULL;

	if (sg_alloc_table(table, 1, GFP_KERNEL)) {
		pr_err("[%s] sg_alloc_table failed .\n", __func__);
		goto err;
	}

	count = (u32)size / PAGE_SIZE;

	page = cma_alloc(cma, count, (u32)get_order((u32)align), false);
	if (!page)
		goto err2;

#ifdef CONFIG_NEED_CHANGE_MAPPING
	/*
	 * Before set memory in secure region, we need change kernel
	 * pgtable from normal to device to avoid big CPU Speculative
	 * read.
	 */
	change_secpage_range(page_to_phys(page),
			     (unsigned long)page_address(page),
			     size, __pgprot(PROT_DEVICE_nGnRE));
	flush_tlb_all();
#endif
	ion_flush_all_cpus_caches();

#ifdef CONFIG_HISI_KERNELDUMP
	tmp_page = page;
	for (k = 0; k < count; k++) {
		SetPageMemDump(tmp_page);
		tmp_page++;
	}
#endif

	sg_set_page(table->sgl, page, (unsigned int)size, 0);

	return table;

err2:
	sg_free_table(table);
err:
	kfree(table);

	return NULL;
}

static void do_free_pages(u32 i, struct sg_table *table,
				struct scatterlist *sg, struct page *page,
				struct cma *cma)
{
	unsigned long nents;

	nents = i;
	pr_err("free %ld pages in err runtime\n", nents);
	for_each_sg(table->sgl, sg, nents, i) {
		page = sg_page(sg);
#ifdef CONFIG_NEED_CHANGE_MAPPING
		change_secpage_range(page_to_phys(page),
				     (unsigned long)page_address(page),
				     sg->length, PAGE_KERNEL);
		flush_tlb_all();
#endif
		cma_release(cma, page, sg->length / PAGE_SIZE);
	}
#ifdef CONFIG_NEED_CHANGE_MAPPING
	flush_tlb_all();
#endif
	sg_free_table(table);
}

static struct sg_table *secmem_alloc_sg(struct cma *cma, unsigned long size,
					unsigned long align)
{
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	unsigned long size_remaining = size;
	unsigned long nents;
	unsigned long alloc_size;
	unsigned long count;
	u32 i = 0;
#ifdef CONFIG_HISI_KERNELDUMP
	unsigned int k;
	struct page *tmp_page = NULL;
#endif

	if (!align)
		return NULL;

	nents = size / align;

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return NULL;

	if (sg_alloc_table(table, (unsigned int)nents, GFP_KERNEL))
		goto free_table;

	sg = table->sgl;
	while (size_remaining) {
		if (size_remaining > align)
			alloc_size = align;
		else
			alloc_size = size_remaining;

		count = alloc_size / PAGE_SIZE;
		page = cma_alloc(cma, count, (u32)get_order((u32)align), false);
		if (!page) {
			pr_err("alloc fail, alloc_size:0x%lx\n", alloc_size);
			goto free_pages;
		}

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
#ifdef CONFIG_HISI_KERNELDUMP
		tmp_page = page;
		for (k = 0; k < count; k++) {
			SetPageMemDump(tmp_page);
			tmp_page++;
		}
#endif

		size_remaining -= alloc_size;
		sg_set_page(sg, page, (unsigned int)alloc_size, 0);
		sg = sg_next(sg);
		i++;
	}

	/*
	 * After change the pgtable prot, we need flush TLB and cache.
	 */
#ifdef CONFIG_NEED_CHANGE_MAPPING
	flush_tlb_all();
#endif
	ion_flush_all_cpus_caches();

	return table;

free_pages:
	do_free_pages(i, table, sg, page, cma);
free_table:
	kfree(table);

	return NULL;
}

struct sg_table *hisi_secmem_alloc(int id, unsigned long size)
{
	struct cma *cma = NULL;
	struct sg_table *table = NULL;
	const char *name = NULL;
	unsigned long align;
	unsigned long cma_size;
	bool is_contig = true;

	if (id < 0 || id >= SEC_SVC_MAX || !size) {
		pr_err("invalid input:id %d size %lx\n", id, size);
		return NULL;
	}

	name = svc_table[id].name;

	cma = get_svc_cma(id);
	if (!cma) {
		pr_err("svc %s has no cma pool\n", name);
		return NULL;
	}

	align = svc_table[id].align;
	is_contig = svc_table[id].is_contig;
	size = ALIGN(size ,align);
	cma_size = cma_get_size(cma);

	if (size > cma_size) {
		pr_err("svc %s alloc size over cma pool size\n", name);
		return NULL;
	}

	if (is_contig)
		table = secmem_alloc_contig(cma, size, align);
	else
		table = secmem_alloc_sg(cma, size, align);

	if(table)
		atomic_long_add(size, &svc_total_size[id]);

	return table;
}

void hisi_secmem_free(int id, struct sg_table *table)
{
	struct cma *cma = NULL;
	struct page *page = NULL;
	struct scatterlist *sg = NULL;
	const char *name = NULL;
	u32 i;
	unsigned long size = 0;

	if (id < 0 || id >= SEC_SVC_MAX || !table) {
		pr_err("invalid input:id %d size %pK\n", id, table);
		return;
	}

	name = svc_table[id].name;
	cma = get_svc_cma(id);
	if (!cma) {
		pr_err("svc %s has no cma pool\n", name);
		return;
	}

	for_each_sg(table->sgl, sg, table->nents, i) {
		page = sg_page(sg);
#ifdef CONFIG_NEED_CHANGE_MAPPING
		change_secpage_range(page_to_phys(page),
				     (unsigned long)page_address(page),
				     sg->length, PAGE_KERNEL);
		flush_tlb_all();
#endif
		cma_release(cma, page, sg->length / PAGE_SIZE);
		size += sg->length;
	}
#ifdef CONFIG_NEED_CHANGE_MAPPING
	flush_tlb_all();
#endif
	atomic_long_sub(size, &svc_total_size[id]);
	sg_free_table(table);
	kfree(table);
}

void sec_alloc_init(struct platform_device *pdev)
{
	const struct device_node *dt_node = pdev->dev.of_node;
	u32 alloc_prot = 0;
	u32 i;
	int ret;

	ret = of_property_read_u32(dt_node, "sec_alloc_sc_prot",
				   &alloc_prot);
	if (ret) {
		pr_err("cant't get scatter_alloc_prot\n");
		return;
	}
	pr_err("alloc_prot %x\n", alloc_prot);

	for (i = 0; i < SEC_SVC_MAX; i++) {
		if (alloc_prot & 1UL << i) {
			svc_table[i].is_contig = false;
			pr_err("%s is scatter alloc\n", svc_table[i].name);
		}
	}
}

void hisi_svc_secmem_info(void)
{
	int i;
	const char *name = NULL;

	for (i = 0; i < SEC_SVC_MAX; i++) {
		name = svc_table[i].name;
		pr_err("svc[%s]: %lu\n", name, get_svc_total_size(i));
	}
}

#ifdef CONFIG_SECMEM_TEST
void test_secmem(void)
{
	int i;
	struct sg_table *table = NULL;
	const char *name = NULL;
	unsigned long size = SZ_32M;

	for (i = 0; i < SEC_SVC_MAX; i++) {
		name = svc_table[i].name;
		table = hisi_secmem_alloc(i, size);
		if (!table) {
			pr_err("test svc[%s] secmem alloc fail\n", name);
			continue;
		}
		pr_err("test svc[%s] secmem alloc succ!table nents %u\n",
			name, table->nents);
		pr_err("svc[%s]: %lu\n", name, get_svc_total_size(i));
		hisi_secmem_free(i, table);
	}
}
#endif
