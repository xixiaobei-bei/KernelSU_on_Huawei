/*
 * Copyright(C) 2019-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
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

#include <linux/bitops.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dma-iommu.h>
#include <linux/err.h>
#include <linux/hisi-iommu.h>
#include <linux/hisi/rdr_hisi_ap_hook.h>
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

#include <asm/pgalloc.h>

#include "hisi_smmu.h"
#ifdef CONFIG_HISI_LB
#include <linux/hisi/hisi_lb.h>
#endif

LIST_HEAD(domain_list);
static struct iommu_ops mm_smmu_ops;

struct mm_domain *to_mm_domain(struct iommu_domain *dom)
{
	if (!dom) {
		pr_err("the iommu domain is invalid\n");
		return NULL;
	}
	return container_of(dom, struct mm_domain, domain);
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

static int mm_smmu_domain_get_cookie(struct iommu_domain *domain)
{
	struct mm_dom_cookie *cookie = NULL;

	if (domain->iova_cookie)
		return -EEXIST;

	domain->iova_cookie = kzalloc(sizeof(struct mm_dom_cookie), GFP_KERNEL);
	if (!domain->iova_cookie)
		return -ENOMEM;

	cookie = domain->iova_cookie;
	spin_lock_init(&cookie->iova_lock);
	cookie->iova_root = RB_ROOT;
	cookie->domain = domain;

	return 0;
}

static struct iommu_domain *
smmu_domain_alloc_lpae(unsigned int iommu_domain_type)
{
	struct mm_domain *mm_domain = NULL;

	if (iommu_domain_type != IOMMU_DOMAIN_UNMANAGED &&
	    iommu_domain_type != IOMMU_DOMAIN_DMA)
		return NULL;

	mm_domain = kzalloc(sizeof(*mm_domain), GFP_KERNEL);
	if (!mm_domain)
		return NULL;

#ifdef CONFIG_IOMMU_DMA
	if (iommu_domain_type == IOMMU_DOMAIN_DMA &&
	    iommu_get_dma_cookie(&mm_domain->domain)) {
		goto err_smmu_pgd;
	}
#else
	if (mm_smmu_domain_get_cookie(&mm_domain->domain))
		goto err_smmu_pgd;
#endif

	mm_domain->va_pgtable_addr_orig =
		kzalloc(SZ_4K, GFP_KERNEL | __GFP_DMA);
	if (!mm_domain->va_pgtable_addr_orig)
		goto err_smmu_pgd;

	mm_domain->va_pgtable_addr = (smmu_pgd_t *)(ALIGN(
		(uintptr_t)(mm_domain->va_pgtable_addr_orig), SZ_512));

	mm_domain->pa_pgtable_addr =
		virt_to_phys(mm_domain->va_pgtable_addr);
	spin_lock_init(&mm_domain->lock);
	list_add_tail(&mm_domain->list, &domain_list);

	return &mm_domain->domain; /*lint !e429 */

err_smmu_pgd:
	kfree(mm_domain);
	return NULL;
}

static void mm_smmu_flush_pgtable_lpae(void *addr, size_t size)
{
	__flush_dcache_area(addr, size);
}

static void mm_smmu_free_ptes_lpae(smmu_pgd_t pmd)
{
	pgtable_t table = smmu_pgd_to_pte_lpae(pmd);
	if (!table) {
		smmu_err("pte table is null\n");
		return;
	}
	__free_page(table);
	smmu_set_pmd_lpae(&pmd, 0);
}

static void mm_smmu_free_pmds_lpae(smmu_pgd_t pgd)
{
	pgtable_t table = smmu_pmd_to_pte_lpae(pgd);
	if (!table) {
		smmu_err("pte table is null\n");
		return;
	}
	__free_page(table);
	smmu_set_pgd_lpae(&pgd, 0);
}

static void mm_smmu_free_pgtables_lpae(struct iommu_domain *domain,
					 unsigned long *page_table_addr)
{
	int i, j;
	smmu_pgd_t *pgd = NULL;
	smmu_pmd_t *pmd = NULL;
	unsigned long flags;
	struct mm_domain *mm_domain = to_mm_domain(domain);

	pgd = (smmu_pgd_t *)page_table_addr;
	pmd = (smmu_pmd_t *)page_table_addr;

	spin_lock_irqsave(&mm_domain->lock, flags);
	for (i = 0; i < SMMU_PTRS_PER_PGD; ++i) {
		if ((smmu_pgd_none_lpae(*pgd)) & (smmu_pmd_none_lpae(*pmd)))
			continue;
		for (j = 0; j < SMMU_PTRS_PER_PMD; ++j) {
			mm_smmu_free_pmds_lpae(*pgd);
			pmd++;
		}
		mm_smmu_free_ptes_lpae(*pmd);
		pgd++;
	}
	memset((void *)page_table_addr, 0, PAGE_SIZE);
	/* unsafe_function_ignore: memset */
	spin_unlock_irqrestore(&mm_domain->lock, flags);
}

static void mm_smmu_domain_free_lpae(struct iommu_domain *domain)
{
	struct mm_domain *mm_domain = to_mm_domain(domain);

	if (!mm_domain) {
		pr_err("the mm_domain is invalid!\n");
		return;
	}

	mm_smmu_free_pgtables_lpae(
		domain, (unsigned long *)mm_domain->va_pgtable_addr);
	list_del(&mm_domain->list);
	kfree(mm_domain->va_pgtable_addr_orig);
	kfree(mm_domain);
}

static u64 mm_smmu_pte_ready(u64 prot)
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

static int mm_smmu_alloc_init_pte_lpae(struct iommu_domain *domain,
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
	struct mm_domain *mm_domain = to_mm_domain(domain);

	if (!smmu_pmd_none_lpae(*ppmd))
		goto pte_ready;

	/* Allocate a new set of tables */
	table = alloc_page(GFP_KERNEL | __GFP_ZERO | __GFP_DMA);
	if (!table) {
		smmu_err("%s: alloc page fail\n", __func__);
		return -ENOMEM;
	}
	spin_lock_irqsave(&mm_domain->lock, *flags);

	if (smmu_pmd_none_lpae(*ppmd)) {
		mm_smmu_flush_pgtable_lpae(page_address(table),
					     SMMU_PAGE_SIZE);
		smmu_pmd_populate_lpae(ppmd, table,
				       SMMU_PMD_TYPE | SMMU_PMD_NS);
		mm_smmu_flush_pgtable_lpae(ppmd, sizeof(*ppmd));
	} else {
		__free_page(table);
	}
	spin_unlock_irqrestore(&mm_domain->lock, *flags);

pte_ready:
	if (prot & IOMMU_SEC)
		*ppmd &= (~SMMU_PMD_NS);

	start = (smmu_pte_t *)smmu_pte_page_vaddr_lpae(ppmd) +
		smmu_pte_index(addr);
	pte = start;
	pteval = mm_smmu_pte_ready(prot);

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

	mm_smmu_flush_pgtable_lpae(start, sizeof(*pte) * (pte - start));
	return 0;
}

static int mm_smmu_alloc_init_pmd_lpae(struct iommu_domain *domain,
					 smmu_pgd_t *ppgd, unsigned long addr,
					 unsigned long end, unsigned long paddr,
					 int prot, unsigned long *flags)
{
	int ret = 0;
	smmu_pmd_t *ppmd = NULL;
	smmu_pmd_t *start = NULL;
	u64 next;
	pgtable_t table;
	struct mm_domain *mm_domain = to_mm_domain(domain);

	if (!smmu_pgd_none_lpae(*ppgd))
		goto pmd_ready;

	/* Allocate a new set of tables */
	table = alloc_page(GFP_KERNEL | __GFP_ZERO | __GFP_DMA);
	if (!table) {
		smmu_err("%s: alloc page fail\n", __func__);
		return -ENOMEM;
	}
	spin_lock_irqsave(&mm_domain->lock, *flags);
	if (smmu_pgd_none_lpae(*ppgd)) {
		mm_smmu_flush_pgtable_lpae(page_address(table),
					     SMMU_PAGE_SIZE);
		smmu_pgd_populate_lpae(ppgd, table,
				       SMMU_PGD_TYPE | SMMU_PGD_NS);
		mm_smmu_flush_pgtable_lpae(ppgd, sizeof(*ppgd));
	} else {
		__free_page(table);
	}
	spin_unlock_irqrestore(&mm_domain->lock, *flags);

pmd_ready:
	if ((unsigned int)prot & IOMMU_SEC)
		*ppgd &= (~SMMU_PGD_NS);
	start = (smmu_pmd_t *)smmu_pmd_page_vaddr_lpae(ppgd) +
		smmu_pmd_index(addr);
	ppmd = start;

	do {
		next = smmu_pmd_addr_end_lpae(addr, end);
		ret = mm_smmu_alloc_init_pte_lpae(domain, ppmd, addr, next,
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

int mm_smmu_handle_mapping_lpae(struct iommu_domain *domain,
				  unsigned long iova, phys_addr_t paddr,
				  size_t size, int prot)
{
	int ret;
	unsigned long end;
	unsigned long next;
	unsigned long flags;

	struct mm_domain *mm_domain = to_mm_domain(domain);
	smmu_pgd_t *pgd = (smmu_pgd_t *)mm_domain->va_pgtable_addr;

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
		ret = mm_smmu_alloc_init_pmd_lpae(domain, pgd, iova, next,
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

static int mm_smmu_map_lpae(struct iommu_domain *domain, unsigned long iova,
			      phys_addr_t paddr, size_t size, int prot)
{
	if (!domain) {
		smmu_err("domain is null\n");
		return -ENODEV;
	}
	return mm_smmu_handle_mapping_lpae(domain, iova, paddr, size, prot);
}

static unsigned long mm_smmu_clear_pte_lpae(smmu_pgd_t *pmdp,
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

static unsigned long mm_smmu_clear_pmd_lpae(smmu_pgd_t *pgdp,
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
		mm_smmu_clear_pte_lpae(ppmd, iova, next);
		iova = next;
		smmu_err("%s: iova=0x%lx, end=0x%lx\n", __func__, iova, end);
	} while (ppmd++, iova < end);

	return size;
}

size_t mm_smmu_handle_unmapping_lpae(struct iommu_domain *domain,
				       unsigned long iova, size_t size)
{
	smmu_pgd_t *pgdp = NULL;
	unsigned long end = 0;
	unsigned long next = 0;
	unsigned long unmap_size = 0;
	struct mm_domain *mm_domain = to_mm_domain(domain);

	iova = SMMU_PAGE_ALIGN(iova);
	size = SMMU_PAGE_ALIGN(size);
	pgdp = (smmu_pgd_t *)mm_domain->va_pgtable_addr;
	end = iova + size;
	pgdp += smmu_pgd_index(iova);
	do {
		next = smmu_pgd_addr_end_lpae(iova, end);
		unmap_size += mm_smmu_clear_pmd_lpae(pgdp, iova, next);
		iova = next;
		smmu_err("%s: pgdp=%pK, iova=0x%lx\n", __func__, pgdp, iova);
	} while (pgdp++, iova < end);

	smmu_trace_hook(MEM_FREE, iova, 0, unmap_size);
	return (size_t)unmap_size;
}

static size_t mm_smmu_unmap_lpae(struct iommu_domain *domain,
				   unsigned long iova, size_t size)
{
	size_t unmap_size = 0;

	if (!domain) {
		smmu_err("domain is null\n");
		return 0;
	}
	/* caculate the max io virtual address */
	/* unmapping the range of iova */
	unmap_size = mm_smmu_handle_unmapping_lpae(domain, iova, size);
	if (unmap_size == size) {
		smmu_err("%s:unmap size:0x%x\n", __func__, (unsigned int)size);
		return size;
	}

	pr_err("%s:unmap fail: iova:0x%lx, size:0x%lx, unmapped:0x%lx\n",
		__func__, iova, size, unmap_size);
	return 0;
}

static phys_addr_t mm_smmu_iova_to_phys_lpae(struct iommu_domain *domain,
					       dma_addr_t iova)
{
	smmu_pgd_t *pgdp = NULL;
	smmu_pgd_t pgd;

	smmu_pmd_t pmd;
	pte_t smmu_pte;
	struct mm_domain *mm_domain = to_mm_domain(domain);

	pgdp = (smmu_pgd_t *)mm_domain->va_pgtable_addr;
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

static int mm_attach_dev_lpae(struct iommu_domain *domain, struct device *dev)
{
	struct device_node *np = NULL;
	struct mm_smmu_device_lpae *mm_smmu = NULL;
	struct mm_domain *mm_domain = to_mm_domain(domain);
	int ret = 0;
	struct mm_dom_cookie *cookie = NULL;

	if (!dev->iommu_fwspec || dev->iommu_fwspec->ops != &mm_smmu_ops)
		return -ENODEV; /* Not a iommu client device */

	mm_smmu = dev->iommu_fwspec->iommu_priv;
	if (!mm_smmu) {
		pr_err("Device (%pK %s) have no mm smmu for add to\n", dev,
		       dev_name(dev));
		return -ENODEV;
	}

	if (mm_domain->dev)
		return 0;

	cookie = domain->iova_cookie;
	np = mm_smmu->dev->of_node;
	ret = of_get_iova_info(np, &cookie->iova.iova_start,
			       &cookie->iova.iova_size,
			       &cookie->iova.iova_align);
	if (ret) {
		pr_err("get dev(%s) iova info fail\n", dev_name(dev));
		return ret;
	}

	cookie->iova_pool =
		iova_pool_setup(cookie->iova.iova_start, cookie->iova.iova_size,
				cookie->iova.iova_align);
	if (!cookie->iova_pool) {
		pr_err("setup dev(%s) iova pool fail\n", dev_name(dev));
		return -ENOMEM;
	}

	mm_domain->dev = mm_smmu->dev;

	return 0;
}

static void mm_detach_dev_lpae(struct iommu_domain *domain,
				 struct device *dev)
{
	struct mm_dom_cookie *cookie = domain->iova_cookie;

	if (!cookie) {
		smmu_err("%s:error! data entry has been delected\n", __func__);
		return;
	}

	if (cookie->iova_pool)
		iova_pool_destroy(cookie->iova_pool);

	kfree(cookie);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
static size_t mm_iommu_map_sg_lpae(struct iommu_domain *domain,
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

		ret = mm_smmu_map_lpae(domain, iova + mapped, phys,
					 (size_t)s->length, prot);
		if (ret)
			goto out_err;
		mapped += s->length;
	}

	return mapped;

out_err:
	/* undo mappings already done */
	mm_smmu_unmap_lpae(domain, iova, mapped);

	return 0;
}
#endif

static int mm_smmu_add_device(struct device *dev)
{
	struct iommu_group *group = NULL;
	struct mm_smmu_device_lpae *mm_smmu = NULL;

	if (!dev->iommu_fwspec || dev->iommu_fwspec->ops != &mm_smmu_ops)
		return -ENODEV; /* Not a iommu client device */

	pr_info("enter %s, dev %s\n", __func__, dev_name(dev));

	mm_smmu = dev->iommu_fwspec->iommu_priv;
	if (!mm_smmu) {
		pr_err("Device (%pK %s) have no mm smmu for add to\n", dev,
		       dev_name(dev));
		return -ENODEV;
	}

	group = iommu_group_get_for_dev(dev);
	if (IS_ERR(group)) {
		pr_err("%s %d:get group fail\n", __func__, __LINE__);
		return PTR_ERR(group);
	}

	iommu_group_put(group);
	iommu_device_link(&mm_smmu->iommu, dev);

	return 0;
}

static void mm_smmu_remove_device(struct device *dev)
{
	struct mm_smmu_device_lpae *mm_smmu = dev->iommu_fwspec->iommu_priv;

	if (!mm_smmu)
		return;

	iommu_group_remove_device(dev);
	iommu_device_unlink(&mm_smmu->iommu, dev);
	iommu_fwspec_free(dev);
}

static struct iommu_group *mm_smmu_device_group(struct device *dev)
{
	struct iommu_group *group = NULL;
	struct mm_smmu_device_lpae *mm_smmu = dev->iommu_fwspec->iommu_priv;

	if (!mm_smmu)
		return ERR_PTR(-ENODEV); /* Not a iommu client device */

	if (!mm_smmu->group) {
		group = iommu_group_alloc();
		if (IS_ERR(group)) {
			dev_err(dev, "Failed to allocate SMMU group\n");
			return group;
		}
		mm_smmu->group = group;
	}

	return mm_smmu->group;
}

static int mm_smmu_of_xlate(struct device *dev, struct of_phandle_args *args)
{
	struct platform_device *pdev = NULL;
	struct mm_smmu_device_lpae *mm_smmu = NULL;

	pdev = of_find_device_by_node(args->np);
	if (WARN_ON(!pdev)) /*lint !e146 !e665 */
		return -EINVAL;

	mm_smmu = platform_get_drvdata(pdev);
	if (!dev->iommu_fwspec->iommu_priv)
		dev->iommu_fwspec->iommu_priv = mm_smmu;
	else if (WARN_ON(mm_smmu !=
			dev->iommu_fwspec->iommu_priv)) /*lint !e146 !e665 */
		return -EINVAL;

	return 0;
}

static int mm_smmu_domain_get_attr(struct iommu_domain *domain,
				enum iommu_attr attr, void *data)
{
	struct mm_domain *mm_domain = NULL;

	mm_domain = to_mm_domain(domain);
	switch (attr) {
	case DOMAIN_ATTR_TTBR:
		*(phys_addr_t *)data = mm_domain->pa_pgtable_addr;
		return 0;
	default:
		pr_err("%s: attr not support, attr = %d", __func__, (int)attr);
		return -ENODEV;
	}
}

static struct iommu_ops mm_smmu_ops = { /*lint !e31 */
	.domain_alloc = smmu_domain_alloc_lpae,
	.domain_free = mm_smmu_domain_free_lpae,
	.map = mm_smmu_map_lpae,
	.unmap = mm_smmu_unmap_lpae,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	.map_sg = mm_iommu_map_sg_lpae,
#endif
	.attach_dev = mm_attach_dev_lpae,
	.detach_dev = mm_detach_dev_lpae,
	.add_device = mm_smmu_add_device,
	.remove_device = mm_smmu_remove_device,
	.device_group = mm_smmu_device_group,
	.domain_get_attr = mm_smmu_domain_get_attr,
	.of_xlate = mm_smmu_of_xlate,
	.iova_to_phys = mm_smmu_iova_to_phys_lpae,
	.pgsize_bitmap = SMMU_PAGE_SIZE,
};

static int mm_smmu_probe_lpae(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mm_smmu_device_lpae *mm_smmu = NULL;
	int ret = 0;

	smmu_err("enter %s: devname = [%s]\n", __func__, dev_name(dev));

	mm_smmu = devm_kzalloc(dev, sizeof(*mm_smmu), GFP_KERNEL);
	if (!mm_smmu)
		return -ENOMEM;

	mm_smmu->dev = dev;
	platform_set_drvdata(pdev, mm_smmu);

	ret = iommu_device_sysfs_add(&mm_smmu->iommu, dev, NULL,
				     dev_name(dev));
	if (ret) {
		pr_err("Failed to register iommu in sysfs\n");
		return ret; /*lint !e429 */
	}

	iommu_device_set_ops(&mm_smmu->iommu, &mm_smmu_ops);
	iommu_device_set_fwnode(&mm_smmu->iommu, dev->fwnode);

	ret = iommu_device_register(&mm_smmu->iommu);
	if (ret) {
		pr_err("Failed to register iommu device\n");
		return ret; /*lint !e429 */
	}

	(void)bus_set_iommu(&platform_bus_type, &mm_smmu_ops);

	return 0; /*lint !e429 */
}

static int mm_smmu_remove_lpae(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id mm_smmu_of_match_lpae[] = {
	{.compatible = "hisi,hisi-smmu-lpae" },
	{},
};
MODULE_DEVICE_TABLE(of, mm_smmu_of_match_lpae);

static struct platform_driver mm_smmu_driver_lpae = {
	.driver = {
			.owner = THIS_MODULE,
			.name = "hisi-smmu-lpae",
			.of_match_table = of_match_ptr(mm_smmu_of_match_lpae),
		},
	.probe = mm_smmu_probe_lpae,
	.remove = mm_smmu_remove_lpae,
};

static int __init mm_smmu_init_lpae(void)
{
	int ret = 0;

	ret = platform_driver_register(&mm_smmu_driver_lpae);

	return ret;
}

static void __exit mm_smmu_exit_lpae(void)
{
	platform_driver_unregister(&mm_smmu_driver_lpae);
}

subsys_initcall(mm_smmu_init_lpae);
module_exit(mm_smmu_exit_lpae);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
IOMMU_OF_DECLARE(hisi_smmu, "hisi,hisi-smmu-lpae", NULL);
#endif

MODULE_DESCRIPTION("IOMMU API for SMMU implementations");
MODULE_AUTHOR("Hisilicon Company");
MODULE_LICENSE("GPL v2");
