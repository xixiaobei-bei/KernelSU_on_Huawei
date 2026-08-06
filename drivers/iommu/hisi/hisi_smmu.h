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

#ifndef _HISI_SMMU_H
#define _HISI_SMMU_H
#include <linux/genalloc.h>
#include <linux/hisi-iommu.h>
#include <linux/iommu.h>
#include <linux/rbtree.h>

/*
 * #define IOMMU_DEBUG
 */
#ifdef IOMMU_DEBUG
#define smmu_err(format, arg...) (pr_err("[iommu]" format, ##arg); )
#else
#define smmu_err(format, arg...)
#endif

#define SMMU_PHY_PTRS_PER_PTE 256
#define SMMU_PTRS_PER_PGD 4
#define SMMU_PTRS_PER_PMD 512
#define SMMU_PTRS_PER_PTE 512
#define SMMU_PAGE_SHIFT 12

#define PAGE_TABLE_ADDR_MASK (UL(0xFFFFFFF) << SMMU_PAGE_SHIFT)

#define SMMU_PAGE_SIZE BIT(SMMU_PAGE_SHIFT)
#define SMMU_PAGE_MASK (~(SMMU_PAGE_SIZE - 1))

#define SMMU_PGDIR_SHIFT 30
#define SMMU_PGDIR_SIZE BIT(SMMU_PGDIR_SHIFT)
#define SMMU_PGDIR_MASK (~(SMMU_PGDIR_SIZE - 1))

#define SMMU_PMDIR_SHIFT 21
#define SMMU_PMDIR_SIZE BIT(SMMU_PMDIR_SHIFT)
#define SMMU_PMDIR_MASK (~(SMMU_PMDIR_SIZE - 1))
#define SMMU_PGD_TYPE (BIT(0) | BIT(1))
#define SMMU_PMD_TYPE (BIT(0) | BIT(1))
#define SMMU_PTE_TYPE (BIT(0) | BIT(1))

#define SMMU_PGD_NS BIT(63)
#define SMMU_PMD_NS BIT(63)
#define SMMU_PTE_NS BIT(5)

#define SMMU_PTE_PXN BIT(53) /* Privileged XN */
#define SMMU_PTE_UXN BIT(54) /* User XN */
#define SMMU_PTE_USER BIT(6) /* AP[1] */
#define SMMU_PTE_RDONLY BIT(7) /* AP[2] */
#define SMMU_PTE_SHARED (BIT(8) | BIT(9)) /* SH[1:0], inner shareable */
#define SMMU_PTE_AF BIT(10) /* Access Flag */
#define SMMU_PTE_NG BIT(11) /* nG */
#define SMMU_PTE_ATTRINDX(t) ((t) << 2)
/*
 * Memory types available.
 * USED BY A7
 */
#define MM_MT_NORMAL 0
#define MM_MT_NORMAL_CACHE 4
#define MM_MT_NORMAL_NC 3
#define MM_MT_DEVICE_NGNRE 6

#define SMMU_PAGE_DEFAULT (SMMU_PTE_TYPE | SMMU_PTE_AF | SMMU_PTE_SHARED)

#define SMMU_PROT_DEVICE_NGNRE                                                \
	(SMMU_PAGE_DEFAULT | SMMU_PTE_PXN | SMMU_PTE_UXN |                     \
	 SMMU_PTE_ATTRINDX(MM_MT_DEVICE_NGNRE))
#define SMMU_PROT_NORMAL_CACHE                                                 \
	(SMMU_PAGE_DEFAULT | SMMU_PTE_PXN | SMMU_PTE_UXN |                     \
	 SMMU_PTE_ATTRINDX(MM_MT_NORMAL_CACHE))
#define SMMU_PROT_NORMAL_NC                                                    \
	(SMMU_PAGE_DEFAULT | SMMU_PTE_PXN | SMMU_PTE_UXN |                     \
	 SMMU_PTE_ATTRINDX(MM_MT_NORMAL_NC))
#define SMMU_PROT_NORMAL                                                       \
	(SMMU_PAGE_DEFAULT | SMMU_PTE_PXN | SMMU_PTE_UXN |                     \
	 SMMU_PTE_ATTRINDX(MM_MT_NORMAL))

#define SMMU_PAGE_READWRITE                                                    \
	(SMMU_PAGE_DEFAULT | SMMU_PTE_USER | SMMU_PTE_NG | SMMU_PTE_PXN |      \
	 SMMU_PTE_UXN)
#define SMMU_PAGE_READONLY                                                     \
	(SMMU_PAGE_DEFAULT | SMMU_PTE_USER | SMMU_PTE_RDONLY | SMMU_PTE_NG |   \
	 SMMU_PTE_PXN | SMMU_PTE_UXN)
#define SMMU_PAGE_READONLY_EXEC                                                \
	(SMMU_PAGE_DEFAULT | SMMU_PTE_USER | SMMU_PTE_NG)

#define smmu_pte_index(addr)                                                   \
	(((addr) >> SMMU_PAGE_SHIFT) & (SMMU_PTRS_PER_PTE - 1))
#define smmu_pmd_index(addr)                                                   \
	(((addr) >> SMMU_PMDIR_SHIFT) & (SMMU_PTRS_PER_PMD - 1))
#define smmu_pgd_index(addr)                                                   \
	(((addr) >> SMMU_PGDIR_SHIFT) & (SMMU_PTRS_PER_PGD - 1))
#define SMMU_PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)

#define PINGPONG_SHIFT 31
#define PINGPONG_MASK (1UL <<  PINGPONG_SHIFT)
#define IOVA_SIZE_MASK (PINGPONG_MASK - 1)

typedef u64 smmu_pgd_t;
typedef u64 smmu_pmd_t;
typedef u64 smmu_pte_t;

enum iova_free_type {
	IMME_FREE, /* Immediately free */
	LAZY_FREE, /* lazy free */
};

struct iommu_domain_data {
	unsigned long iova_start;
	unsigned long iova_size;
	unsigned long iova_align;
	unsigned int iova_free;
};

struct iova_dom {
	struct rb_node node;
	unsigned long iova;
	unsigned long size;
	unsigned long pad_size;
	atomic_long_t ref;
	u64 key;
	struct device *dev;
};

struct mm_iova_lazy_free {
	u32 *free_size;
	u32 pingpong;
	unsigned long pages;
	unsigned long waterline;
	bool end;
	wait_queue_head_t wait_q;
	struct task_struct *task;
	struct mutex mutex;
	spinlock_t lock;
};

struct mm_dom_cookie {
	spinlock_t iova_lock;
	struct rb_root iova_root;
	struct gen_pool *iova_pool;
	struct iommu_domain *domain;
	struct iommu_domain_data iova;
	struct mm_iova_lazy_free *lazy_free;
};

struct mm_domain {
	struct iommu_domain domain;
	struct device *dev;
	spinlock_t lock; /* spinlock */
	smmu_pgd_t *va_pgtable_addr;
	smmu_pgd_t *va_pgtable_addr_orig;
	phys_addr_t pa_pgtable_addr;
	struct list_head list;
};

/* smmu device object */
struct mm_smmu_device_lpae {
	struct iommu_device iommu;
	struct device *dev;
	struct iommu_group *group;
};

extern struct list_head domain_list;
extern struct list_head smmuv3_domain_list;

static inline unsigned int smmu_pgd_none_lpae(smmu_pgd_t pgd)
{
	return !(pgd ? pgd : 0);
}

static inline unsigned int smmu_pmd_none_lpae(smmu_pmd_t pmd)
{
	return !(pmd ? pmd : 0);
}

static inline unsigned int smmu_pte_none_lpae(smmu_pte_t pte)
{
	return !(pte ? pte : 0);
}

static inline unsigned int pte_is_valid_lpae(smmu_pte_t *ptep)
{
	return (unsigned int)((*(ptep) & SMMU_PTE_TYPE) ? 1 : 0);
}

/* Find an entry in the second-level page table.. */
static inline void *smmu_pmd_page_vaddr_lpae(smmu_pmd_t *pgd)
{
	return phys_to_virt(*pgd & PAGE_TABLE_ADDR_MASK);
}

/* Find an entry in the third-level page table.. */
static inline void *smmu_pte_page_vaddr_lpae(smmu_pmd_t *pmd)
{
	return phys_to_virt(*pmd & PAGE_TABLE_ADDR_MASK);
}

/* fill the pgd entry, pgd value must be 64bit */
static inline void smmu_set_pgd_lpae(smmu_pgd_t *pgdp, u64 pgd)
{
	*pgdp = pgd;
	dsb(ishst);
	isb();
}

/* fill the pmd entry, pgd value must be 64bit */
static inline void smmu_set_pmd_lpae(smmu_pgd_t *pmdp, u64 pmd)
{
	smmu_err("%s: pmd = 0x%llx\n", __func__, pmd);
	*pmdp = pmd;
	dsb(ishst);
	isb();
}

static inline void smmu_pmd_populate_lpae(smmu_pmd_t *pmdp, pgtable_t ptep,
					  pgdval_t prot)
{
	smmu_set_pmd_lpae(pmdp, (u64)(page_to_phys(ptep) | prot));
}

static inline void smmu_pgd_populate_lpae(smmu_pgd_t *pgdp, pgtable_t pmdp,
					  pgdval_t prot)
{
	smmu_set_pgd_lpae(pgdp, (u64)(page_to_phys(pmdp) | prot));
}

static inline unsigned long smmu_pgd_addr_end_lpae(unsigned long addr,
						   unsigned long end)
{
	unsigned long boundary = (addr + SMMU_PGDIR_SIZE) & SMMU_PGDIR_MASK;

	return (boundary - 1 < end - 1) ? boundary : end;
}

static inline unsigned long smmu_pmd_addr_end_lpae(unsigned long addr,
						   unsigned long end)
{
	unsigned long boundary = (addr + SMMU_PMDIR_SIZE) & SMMU_PMDIR_MASK;

	return (boundary - 1 < end - 1) ? boundary : end;
}

int mm_smmu_handle_mapping_lpae(struct iommu_domain *domain,
				  unsigned long iova, phys_addr_t paddr,
				  size_t size, int prot);

size_t mm_smmu_handle_unmapping_lpae(struct iommu_domain *domain,
				       unsigned long iova, size_t size);

struct mm_domain *to_mm_domain(struct iommu_domain *dom);
struct gen_pool *iova_pool_setup(unsigned long start, unsigned long size,
				 unsigned long align);
void iova_pool_destroy(struct gen_pool *pool);
void __dmabuf_release_iommu(struct dma_buf *dmabuf, struct iommu_domain domain);
#ifdef CONFIG_ARM_SMMU_V3
void arm_smmu_dmabuf_release_iommu(struct dma_buf *dmabuf);
#else
static inline void arm_smmu_dmabuf_release_iommu(struct dma_buf *dmabuf)
{
}
#endif
void mm_iova_dom_info(struct device *dev);
int mm_smmu_tcu_node_status(int smmuid);
int mm_smmu_dmss_status(int smmuid);
int mm_flush_pgtbl_cache(pgd_t *raw_pgd, unsigned long addr, size_t size);
#endif
