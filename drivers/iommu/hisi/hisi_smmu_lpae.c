/*
 * hisi_smmu_lpae.c -- 3 layer pagetable
 *
 * Copyright (C) 2019 Hisilicon. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/pgalloc.h>
#include <linux/bitops.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dma-iommu.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/iommu.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_iommu.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/sizes.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/hisi-iommu.h>
#include <linux/hisi/rdr_hisi_ap_hook.h>
#include "hisi_smmu.h"

#ifdef CONFIG_HISI_LB
#include <linux/hisi/hisi_lb.h>
#endif

LIST_HEAD(domain_list);
static struct iommu_ops hisi_smmu_ops;

struct hisi_domain *to_hisi_domain(struct iommu_domain *dom)
{
	if (!dom) {
		pr_err("the iommu domain is invalid\n");
		return NULL;
	}
	return container_of(dom, struct hisi_domain, domain);
}

/* transfer 64bit pte table pointer to struct page */
static pgtable_t smmu_pgd_to_pte_lpae(unsigned int ppte_table)
{
	unsigned long page_table_addr;

	if (!ppte_table) {
		smmu_err("error: the pointer of pte_table is NULL\n");
		return NULL;
	}
	page_table_addr = (unsigned long)ppte_table;
	return phys_to_page(page_table_addr);
}

/* transfer 64bit pte table pointer to struct page */
static pgtable_t smmu_pmd_to_pte_lpae(unsigned long ppte_table)
{
	struct page *table = NULL;

	if (!ppte_table) {
		smmu_err("error: the pointer of pte_table is NULL\n");
		return NULL;
	}
	table = phys_to_page(ppte_table);
	return table;
}

int of_get_iova_info(struct device_node *np, unsigned long *iova_start,
		     unsigned long *iova_size, unsigned long *iova_align)
{
	struct device_node *node = NULL;
	int ret = 0;

	if (!np)
		return -ENODEV;

	node = of_get_child_by_name(np, "iova_info");
	if (!node) {
		pr_err("find iommu_info node error\n");
		return -ENODEV;
	}
	ret = of_property_read_u64(node, "start-addr", (u64 *)iova_start);
	if (ret) {
		pr_err("read iova start address error\n");
		return -EINVAL;
	}
	ret = of_property_read_u64(node, "size", (u64 *)iova_size);
	if (ret) {
		pr_err("read iova size error\n");
		return -EINVAL;
	}
	ret = of_property_read_u64(node, "iova-align", (u64 *)iova_align);
	if (ret)
		*iova_align = SZ_256K;

	pr_err("%s:start_addr 0x%lx, size 0x%lx align 0x%lx\n", __func__,
	       *iova_start, *iova_size, *iova_align);

	return 0;
}

static struct iommu_domain *
hisi_smmu_domain_alloc_lpae(unsigned int iommu_domain_type)
{
	struct hisi_domain *hisi_domain = NULL;

	if (iommu_domain_type != IOMMU_DOMAIN_UNMANAGED &&
	    iommu_domain_type != IOMMU_DOMAIN_DMA)
		return NULL;

	hisi_domain = kzalloc(sizeof(*hisi_domain), GFP_KERNEL);
	if (!hisi_domain)
		return NULL;

#ifdef CONFIG_IOMMU_DMA
	if (iommu_domain_type == IOMMU_DOMAIN_DMA &&
	    iommu_get_dma_cookie(&hisi_domain->domain)) {
		goto err_smmu_pgd;
	}
#endif

	hisi_domain->va_pgtable_addr_orig =
		kzalloc(SZ_4K, GFP_KERNEL | __GFP_DMA);
	if (!hisi_domain->va_pgtable_addr_orig)
		goto err_smmu_pgd;

	hisi_domain->va_pgtable_addr = (smmu_pgd_t *)(ALIGN(
		(uintptr_t)(hisi_domain->va_pgtable_addr_orig), SZ_512));

	hisi_domain->pa_pgtable_addr =
		virt_to_phys(hisi_domain->va_pgtable_addr);
	spin_lock_init(&hisi_domain->lock);
	spin_lock_init(&hisi_domain->iova_lock);
	hisi_domain->iova_root = RB_ROOT;
	list_add_tail(&hisi_domain->list, &domain_list);

	return &hisi_domain->domain; /*lint !e429 */

err_smmu_pgd:
	kfree(hisi_domain);
	return NULL;
}

static void hisi_smmu_flush_pgtable_lpae(void *addr, size_t size)
{
	__flush_dcache_area(addr, size);
}

static void hisi_smmu_free_ptes_lpae(smmu_pgd_t pmd)
{
	pgtable_t table = smmu_pgd_to_pte_lpae(pmd);

	if (!table) {
		smmu_err("pte table is null\n");
		return;
	}
	__free_page(table);
	smmu_set_pmd_lpae(&pmd, 0);
}

static void hisi_smmu_free_pmds_lpae(smmu_pgd_t pgd)
{
	pgtable_t table = smmu_pmd_to_pte_lpae(pgd);

	if (!table) {
		smmu_err("pte table is null\n");
		return;
	}
	__free_page(table);
	smmu_set_pgd_lpae(&pgd, 0);
}

static void hisi_smmu_free_pgtables_lpae(struct iommu_domain *domain,
					 unsigned long *page_table_addr)
{
	int i, j;
	smmu_pgd_t *pgd = NULL;
	smmu_pmd_t *pmd = NULL;
	unsigned long flags;
	struct hisi_domain *hisi_domain = to_hisi_domain(domain);

	pgd = (smmu_pgd_t *)page_table_addr;
	pmd = (smmu_pmd_t *)page_table_addr;

	spin_lock_irqsave(&hisi_domain->lock, flags);
	for (i = 0; i < SMMU_PTRS_PER_PGD; ++i) {
		if ((smmu_pgd_none_lpae(*pgd)) & (smmu_pmd_none_lpae(*pmd)))
			continue;
		for (j = 0; j < SMMU_PTRS_PER_PMD; ++j) {
			hisi_smmu_free_pmds_lpae(*pgd);
			pmd++;
		}
		hisi_smmu_free_ptes_lpae(*pmd);
		pgd++;
	}
	memset((void *)page_table_addr, 0, PAGE_SIZE);
	/* unsafe_function_ignore: memset */
	spin_unlock_irqrestore(&hisi_domain->lock, flags);
}

static void hisi_smmu_domain_free_lpae(struct iommu_domain *domain)
{
	struct hisi_domain *hisi_domain = to_hisi_domain(domain);

	if (!hisi_domain) {
		pr_err("the hisi_domain is invalid!\n");
		return;
	}

	hisi_smmu_free_pgtables_lpae(
		domain, (unsigned long *)hisi_domain->va_pgtable_addr);
	list_del(&hisi_domain->list);
	kfree(hisi_domain->va_pgtable_addr_orig);
	kfree(hisi_domain);
}

static u64 hisi_smmu_pte_ready(u64 prot)
{
	u64 pteval = SMMU_PTE_TYPE;

	if (!prot) {
		pteval |= SMMU_PROT_NORMAL;
		pteval |= SMMU_PTE_NS;
	} else {
		if (prot & IOMMU_DEVICE) {
			pteval |= SMMU_PROT_DEVICE_NGNRE;
		} else {
			if (prot & IOMMU_CACHE)
				pteval |= SMMU_PROT_NORMAL_CACHE;
			else
				pteval |= SMMU_PROT_NORMAL_NC;

			if ((prot & IOMMU_READ) && (prot & IOMMU_WRITE))
				pteval |= SMMU_PAGE_READWRITE;
			else if ((prot & IOMMU_READ) && !(prot & IOMMU_WRITE))
				pteval |= SMMU_PAGE_READONLY;
			else
				WARN_ON(1);

			if (prot & IOMMU_EXEC) {
				pteval |= SMMU_PAGE_READONLY_EXEC;
				pteval &= ~(SMMU_PTE_PXN | SMMU_PTE_UXN);
			}
		}
		if (prot & IOMMU_SEC)
			pteval &= (~SMMU_PTE_NS);
		else
			pteval |= SMMU_PTE_NS;
	}
	return pteval;
}

static int hisi_smmu_alloc_init_pte_lpae(struct iommu_domain *domain,
					 smmu_pmd_t *ppmd, unsigned long addr,
					 unsigned long end, unsigned long pfn,
					 u64 prot, unsigned long *flags)
{
#ifdef CONFIG_HISI_LB
	u32 pid;
#endif
	smmu_pte_t *pte = NULL;
	smmu_pte_t *start = NULL;
	pgtable_t table;
	u64 pteval;
	struct hisi_domain *hisi_domain = to_hisi_domain(domain);

	if (!smmu_pmd_none_lpae(*ppmd))
		goto pte_ready;

	/* Allocate a new set of tables */
	table = alloc_page(GFP_KERNEL | __GFP_ZERO | __GFP_DMA);
	if (!table) {
		smmu_err("%s: alloc page fail\n", __func__);
		return -ENOMEM;
	}
	spin_lock_irqsave(&hisi_domain->lock, *flags);

	if (smmu_pmd_none_lpae(*ppmd)) {
		hisi_smmu_flush_pgtable_lpae(page_address(table),
					     SMMU_PAGE_SIZE);
		smmu_pmd_populate_lpae(ppmd, table,
				       SMMU_PMD_TYPE | SMMU_PMD_NS);
		hisi_smmu_flush_pgtable_lpae(ppmd, sizeof(*ppmd));
	} else {
		__free_page(table);
	}
	spin_unlock_irqrestore(&hisi_domain->lock, *flags);

pte_ready:
	if (prot & IOMMU_SEC)
		*ppmd &= (~SMMU_PMD_NS);

	start = (smmu_pte_t *)smmu_pte_page_vaddr_lpae(ppmd) +
		smmu_pte_index(addr);
	pte = start;
	pteval = hisi_smmu_pte_ready(prot);

#ifdef CONFIG_HISI_LB
	pid = (prot & IOMMU_PORT_MASK) >> IOMMU_PORT_SHIFT;
	pteval |= !pid ? 0 : lb_pid_to_gidphys(pid);
#endif
	do {
		if (!pte_is_valid_lpae(pte))
			*pte = (u64)(__pfn_to_phys(pfn) | pteval);
		else
			WARN_ONCE(1, "map to same VA more times!\n"); /*lint !e146 !e665 */
		pte++;
		pfn++;
		addr += SMMU_PAGE_SIZE;
	} while (addr < end);

	hisi_smmu_flush_pgtable_lpae(start, sizeof(*pte) * (pte - start));
	return 0;
}

static int hisi_smmu_alloc_init_pmd_lpae(struct iommu_domain *domain,
					 smmu_pgd_t *ppgd, unsigned long addr,
					 unsigned long end, unsigned long paddr,
					 int prot, unsigned long *flags)
{
	int ret = 0;
	smmu_pmd_t *ppmd = NULL;
	smmu_pmd_t *start = NULL;
	u64 next;
	pgtable_t table;
	struct hisi_domain *hisi_domain = to_hisi_domain(domain);

	if (!smmu_pgd_none_lpae(*ppgd))
		goto pmd_ready;

	/* Allocate a new set of tables */
	table = alloc_page(GFP_KERNEL | __GFP_ZERO | __GFP_DMA);
	if (!table) {
		smmu_err("%s: alloc page fail\n", __func__);
		return -ENOMEM;
	}
	spin_lock_irqsave(&hisi_domain->lock, *flags);
	if (smmu_pgd_none_lpae(*ppgd)) {
		hisi_smmu_flush_pgtable_lpae(page_address(table),
					     SMMU_PAGE_SIZE);
		smmu_pgd_populate_lpae(ppgd, table,
				       SMMU_PGD_TYPE | SMMU_PGD_NS);
		hisi_smmu_flush_pgtable_lpae(ppgd, sizeof(*ppgd));
	} else {
		__free_page(table);
	}
	spin_unlock_irqrestore(&hisi_domain->lock, *flags);

pmd_ready:
	if ((unsigned int)prot & IOMMU_SEC)
		*ppgd &= (~SMMU_PGD_NS);
	start = (smmu_pmd_t *)smmu_pmd_page_vaddr_lpae(ppgd) +
		smmu_pmd_index(addr);
	ppmd = start;

	do {
		next = smmu_pmd_addr_end_lpae(addr, end);
		ret = hisi_smmu_alloc_init_pte_lpae(domain, ppmd, addr, next,
						    __phys_to_pfn(paddr), prot,
						    flags);
		if (ret)
			goto error;
		paddr += (next - addr);
		addr = next;
	} while (ppmd++, addr < end);
error:
	return ret;
}

int hisi_smmu_handle_mapping_lpae(struct iommu_domain *domain,
				  unsigned long iova, phys_addr_t paddr,
				  size_t size, int prot)
{
	int ret;
	unsigned long end;
	unsigned long next;
	unsigned long flags;

	struct hisi_domain *hisi_domain = to_hisi_domain(domain);
	smmu_pgd_t *pgd = (smmu_pgd_t *)hisi_domain->va_pgtable_addr;

	if (!pgd) {
		smmu_err("pgd is null\n");
		return -EINVAL;
	}
	iova = ALIGN(iova, SMMU_PAGE_SIZE);
	size = ALIGN(size, SMMU_PAGE_SIZE);
	pgd += smmu_pgd_index(iova);
	end = iova + size;
	do {
		next = smmu_pgd_addr_end_lpae(iova, end);
		ret = hisi_smmu_alloc_init_pmd_lpae(domain, pgd, iova, next,
						    paddr, prot, &flags);
		if (ret)
			goto out_unlock;
		paddr += next - iova;
		iova = next;
	} while (pgd++, iova < end);
out_unlock:
	smmu_trace_hook(MEM_ALLOC, iova, paddr, (unsigned int)size);
	return ret;
}

static int hisi_smmu_map_lpae(struct iommu_domain *domain, unsigned long iova,
			      phys_addr_t paddr, size_t size, int prot)
{
	if (!domain) {
		smmu_err("domain is null\n");
		return -ENODEV;
	}
	return hisi_smmu_handle_mapping_lpae(domain, iova, paddr, size, prot);
}

static unsigned long hisi_smmu_clear_pte_lpae(smmu_pgd_t *pmdp,
					      unsigned long iova,
					      unsigned long end)
{
	smmu_pte_t *ptep = NULL;
	smmu_pte_t *ppte = NULL;
	unsigned long size = end - iova;

	ptep = smmu_pte_page_vaddr_lpae(pmdp);
	ppte = ptep + smmu_pte_index(iova);

	if (size)
		memset(ppte, 0x0,
		       (size / SMMU_PAGE_SIZE) *
			sizeof(*ppte)); /* unsafe_function_ignore: memset */

	return size;
}

static unsigned long hisi_smmu_clear_pmd_lpae(smmu_pgd_t *pgdp,
					      unsigned long iova,
					      unsigned long end)
{
	smmu_pmd_t *pmdp = NULL;
	smmu_pmd_t *ppmd = NULL;
	unsigned long next = 0;
	unsigned long size = end - iova;

	pmdp = smmu_pmd_page_vaddr_lpae(pgdp);
	ppmd = pmdp + smmu_pmd_index(iova);
	do {
		next = smmu_pmd_addr_end_lpae(iova, end);
		hisi_smmu_clear_pte_lpae(ppmd, iova, next);
		iova = next;
		smmu_err("%s: iova=0x%lx, end=0x%lx\n", __func__, iova, end);
	} while (ppmd++, iova < end);

	return size;
}

size_t hisi_smmu_handle_unmapping_lpae(struct iommu_domain *domain,
				       unsigned long iova, size_t size)
{
	smmu_pgd_t *pgdp = NULL;
	unsigned long end = 0;
	unsigned long next = 0;
	unsigned long unmap_size = 0;
	struct hisi_domain *hisi_domain = to_hisi_domain(domain);

	iova = SMMU_PAGE_ALIGN(iova);
	size = SMMU_PAGE_ALIGN(size);
	pgdp = (smmu_pgd_t *)hisi_domain->va_pgtable_addr;
	end = iova + size;
	pgdp += smmu_pgd_index(iova);
	do {
		next = smmu_pgd_addr_end_lpae(iova, end);
		unmap_size += hisi_smmu_clear_pmd_lpae(pgdp, iova, next);
		iova = next;
		smmu_err("%s: pgdp=%pK, iova=0x%lx\n", __func__, pgdp, iova);
	} while (pgdp++, iova < end);

	smmu_trace_hook(MEM_FREE, iova, 0, unmap_size);
	return (size_t)unmap_size;
}

static size_t hisi_smmu_unmap_lpae(struct iommu_domain *domain,
				   unsigned long iova, size_t size)
{
	size_t unmap_size = 0;

	if (!domain) {
		smmu_err("domain is null\n");
		return 0;
	}
	/* caculate the max io virtual address */
	/* unmapping the range of iova */
	unmap_size = hisi_smmu_handle_unmapping_lpae(domain, iova, size);
	if (unmap_size == size) {
		smmu_err("%s:unmap size:0x%x\n", __func__, (unsigned int)size);
		return size;
	}

	return 0;
}

static phys_addr_t hisi_smmu_iova_to_phys_lpae(struct iommu_domain *domain,
					       dma_addr_t iova)
{
	smmu_pgd_t *pgdp = NULL;
	smmu_pgd_t pgd;

	smmu_pmd_t pmd;
	pte_t smmu_pte;
	struct hisi_domain *hisi_domain = to_hisi_domain(domain);

	pgdp = (smmu_pgd_t *)hisi_domain->va_pgtable_addr;
	if (!pgdp)
		return 0;

	pgd = *(pgdp + smmu_pgd_index(iova));
	if (smmu_pgd_none_lpae(pgd))
		return 0;

	pmd = *((smmu_pmd_t *)smmu_pmd_page_vaddr_lpae(&pgd) +
		smmu_pmd_index(iova));
	if (smmu_pmd_none_lpae(pmd))
		return 0;

	smmu_pte.pte =
		*((u64 *)smmu_pte_page_vaddr_lpae(&pmd) + smmu_pte_index(iova));
	if (smmu_pte_none_lpae(smmu_pte.pte))
		return 0;

	return __pfn_to_phys(pte_pfn(smmu_pte)) | (iova & ~SMMU_PAGE_MASK);
}

static int hisi_attach_dev_lpae(struct iommu_domain *domain, struct device *dev)
{
	struct device_node *np = NULL;
	struct iommu_domain_data *domain_data = NULL;
	struct hisi_smmu_device_lpae *hisi_smmu = NULL;
	struct hisi_domain *hisi_domain = to_hisi_domain(domain);
	int ret = 0;

	if (!dev->iommu_fwspec || dev->iommu_fwspec->ops != &hisi_smmu_ops)
		return -ENODEV; /* Not a iommu client device */

	hisi_smmu = dev->iommu_fwspec->iommu_priv;
	if (!hisi_smmu) {
		pr_err("Device (%pK %s) have no hisi smmu for add to\n", dev,
		       dev_name(dev));
		return -ENODEV;
	}

	if (hisi_domain->domain_data)
		return 0;

	domain_data = kzalloc(sizeof(*domain_data), GFP_KERNEL);
	if (!domain_data)
		return -ENOMEM;

	np = hisi_smmu->dev->of_node;
	ret = of_get_iova_info(np, &domain_data->iova_start,
			       &domain_data->iova_size,
			       &domain_data->iova_align);
	if (ret) {
		pr_err("get dev(%s) iova info fail\n", dev_name(dev));
		kfree(domain_data);
		return ret;
	}

	hisi_domain->iova_pool =
		iova_pool_setup(domain_data->iova_start, domain_data->iova_size,
				domain_data->iova_align);
	if (!hisi_domain->iova_pool) {
		pr_err("setup dev(%s) iova pool fail\n", dev_name(dev));
		kfree(domain_data);
		return -ENOMEM;
	}

	hisi_domain->domain_data = domain_data;
	hisi_domain->dev = hisi_smmu->dev;

	return 0;
}

static void hisi_detach_dev_lpae(struct iommu_domain *domain,
				 struct device *dev)
{
	struct hisi_domain *hisi_domain = to_hisi_domain(domain);
	struct iommu_domain_data *data = hisi_domain->domain_data;

	if (hisi_domain->iova_pool)
		iova_pool_destroy(hisi_domain->iova_pool);

	if (!data)
		smmu_err("%s:error! data entry has been delected\n", __func__);
	else
		kfree(data);

	hisi_domain->domain_data = NULL;
}

static size_t hisi_iommu_map_sg_lpae(struct iommu_domain *domain,
				     unsigned long iova, struct scatterlist *sg,
				     unsigned int nents, int prot)
{
	struct scatterlist *s = NULL;
	size_t mapped = 0;
	unsigned int i, min_pagesz;
	int ret;

	if (domain->ops->pgsize_bitmap == 0UL)
		return 0;

	min_pagesz = 1 << __ffs(domain->ops->pgsize_bitmap);

	for_each_sg(sg, s, nents, i) {
		phys_addr_t phys = page_to_phys(sg_page(s)) + s->offset;

		/*
		 * We are mapping on IOMMU page boundaries, so offset within
		 * the page must be 0. However, the IOMMU may support pages
		 * smaller than PAGE_SIZE, so s->offset may still represent
		 * an offset of that boundary within the CPU page.
		 */
		if (!IS_ALIGNED(s->offset, min_pagesz))
			goto out_err;

		ret = hisi_smmu_map_lpae(domain, iova + mapped, phys,
					 (size_t)s->length, prot);
		if (ret)
			goto out_err;
		mapped += s->length;
	}

	return mapped;

out_err:
	/* undo mappings already done */
	hisi_smmu_unmap_lpae(domain, iova, mapped);

	return 0;
}

static int hisi_smmu_add_device(struct device *dev)
{
	struct iommu_group *group = NULL;
	struct hisi_smmu_device_lpae *hisi_smmu = NULL;

	if (!dev->iommu_fwspec || dev->iommu_fwspec->ops != &hisi_smmu_ops)
		return -ENODEV; /* Not a iommu client device */

	hisi_smmu = dev->iommu_fwspec->iommu_priv;
	if (!hisi_smmu) {
		pr_err("Device (%pK %s) have no hisi smmu for add to\n", dev,
		       dev_name(dev));
		return -ENODEV;
	}

	group = iommu_group_get_for_dev(dev);
	if (IS_ERR(group)) {
		pr_err("%s %d:get group fail\n", __func__, __LINE__);
		return PTR_ERR(group);
	}

	iommu_group_put(group);
	iommu_device_link(&hisi_smmu->iommu, dev);

	return 0;
}

static void hisi_smmu_remove_device(struct device *dev)
{
	struct hisi_smmu_device_lpae *hisi_smmu = dev->iommu_fwspec->iommu_priv;

	if (!hisi_smmu)
		return;

	iommu_group_remove_device(dev);
	iommu_device_unlink(&hisi_smmu->iommu, dev);
	iommu_fwspec_free(dev);
}

static struct iommu_group *hisi_smmu_device_group(struct device *dev)
{
	struct iommu_group *group = NULL;
	struct hisi_smmu_device_lpae *hisi_smmu = dev->iommu_fwspec->iommu_priv;

	if (!hisi_smmu)
		return ERR_PTR(-ENODEV); /* Not a iommu client device */

	if (!hisi_smmu->group) {
		group = iommu_group_alloc();
		if (IS_ERR(group)) {
			dev_err(dev, "Failed to allocate SMMU group\n");
			return group;
		}
		hisi_smmu->group = group;
	}

	return hisi_smmu->group;
}

static int hisi_smmu_of_xlate(struct device *dev, struct of_phandle_args *args)
{
	struct platform_device *pdev = NULL;
	struct hisi_smmu_device_lpae *hisi_smmu = NULL;

	pdev = of_find_device_by_node(args->np);
	if (WARN_ON(!pdev)) /*lint !e146 !e665 */
		return -EINVAL;

	hisi_smmu = platform_get_drvdata(pdev);
	if (!dev->iommu_fwspec->iommu_priv)
		dev->iommu_fwspec->iommu_priv = hisi_smmu;
	else if (WARN_ON(hisi_smmu !=
			dev->iommu_fwspec->iommu_priv)) /*lint !e146 !e665 */
		return -EINVAL;

	return 0;
}

static struct iommu_ops hisi_smmu_ops = { /*lint !e31 */
	.domain_alloc = hisi_smmu_domain_alloc_lpae,
	.domain_free = hisi_smmu_domain_free_lpae,
	.map = hisi_smmu_map_lpae,
	.unmap = hisi_smmu_unmap_lpae,
	.map_sg = hisi_iommu_map_sg_lpae,
	.attach_dev = hisi_attach_dev_lpae,
	.detach_dev = hisi_detach_dev_lpae,
	.add_device = hisi_smmu_add_device,
	.remove_device = hisi_smmu_remove_device,
	.device_group = hisi_smmu_device_group,
	.of_xlate = hisi_smmu_of_xlate,
	.iova_to_phys = hisi_smmu_iova_to_phys_lpae,
	.pgsize_bitmap = SMMU_PAGE_SIZE,
};

static int hisi_smmu_probe_lpae(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hisi_smmu_device_lpae *hisi_smmu = NULL;
	int ret = 0;

	smmu_err("enter %s\n", __func__);

	hisi_smmu = devm_kzalloc(dev, sizeof(*hisi_smmu), GFP_KERNEL);
	if (!hisi_smmu)
		return -ENOMEM;

	hisi_smmu->dev = dev;
	platform_set_drvdata(pdev, hisi_smmu);

	ret = iommu_device_sysfs_add(&hisi_smmu->iommu, dev, NULL,
				     dev_name(dev));
	if (ret) {
		pr_err("Failed to register iommu in sysfs\n");
		return ret; /*lint !e429 */
	}

	iommu_device_set_ops(&hisi_smmu->iommu, &hisi_smmu_ops);
	iommu_device_set_fwnode(&hisi_smmu->iommu, dev->fwnode);

	ret = iommu_device_register(&hisi_smmu->iommu);
	if (ret) {
		pr_err("Failed to register iommu device\n");
		return ret; /*lint !e429 */
	}

	bus_set_iommu(&platform_bus_type, &hisi_smmu_ops);

	return 0; /*lint !e429 */
}

static int hisi_smmu_remove_lpae(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id hisi_smmu_of_match_lpae[] = {
	{.compatible = "hisi,hisi-smmu-lpae" },
	{},
};
MODULE_DEVICE_TABLE(of, hisi_smmu_of_match_lpae);

static struct platform_driver hisi_smmu_driver_lpae = {
	.driver = {
			.owner = THIS_MODULE,
			.name = "hisi-smmu-lpae",
			.of_match_table = of_match_ptr(hisi_smmu_of_match_lpae),
		},
	.probe = hisi_smmu_probe_lpae,
	.remove = hisi_smmu_remove_lpae,
};

static int __init hisi_smmu_init_lpae(void)
{
	int ret = 0;

	ret = platform_driver_register(&hisi_smmu_driver_lpae);

	return ret;
}

static void __exit hisi_smmu_exit_lpae(void)
{
	platform_driver_unregister(&hisi_smmu_driver_lpae);
}

subsys_initcall(hisi_smmu_init_lpae);
module_exit(hisi_smmu_exit_lpae);

IOMMU_OF_DECLARE(hisi_smmu, "hisi,hisi-smmu-lpae", NULL);

MODULE_DESCRIPTION("IOMMU API for Hisi SMMU implementations");
MODULE_AUTHOR("Huawei Hisilicon Company");
MODULE_LICENSE("GPL v2");
