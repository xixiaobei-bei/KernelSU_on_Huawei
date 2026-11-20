/*
 * Based on arch/arm/include/asm/pgalloc.h
 *
 * Copyright (C) 2000-2001 Russell King
 * Copyright (C) 2012 ARM Ltd.
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
#ifndef __ASM_PGALLOC_H
#define __ASM_PGALLOC_H

#include <asm/pgtable-hwdef.h>
#include <asm/processor.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#ifdef CONFIG_HKIP_PRMEM
#include <linux/hisi/prmem.h>
#endif

#define check_pgt_cache()		do { } while (0)

#define PGALLOC_GFP	(GFP_KERNEL | __GFP_ZERO)
#define PGD_SIZE	(PTRS_PER_PGD * sizeof(pgd_t))

#if CONFIG_PGTABLE_LEVELS > 2

static inline pmd_t *pmd_alloc_one(struct mm_struct *mm, unsigned long addr)
{
#ifdef CONFIG_HKIP_PRMEM
	if (unlikely(is_pmalloc((void *)addr, 1)))
		return (pmd_t *)pmalloc_get_page_table_page();
#endif
	return (pmd_t *)__get_free_page(PGALLOC_GFP);
}

static inline void pmd_free(struct mm_struct *mm, pmd_t *pmd)
{
	BUG_ON((unsigned long)pmd & (PAGE_SIZE-1));
#ifdef CONFIG_HKIP_PRMEM
	if (unlikely(is_pmalloc_page_table_page((unsigned long)pmd))) {
		pmalloc_put_page_table_page((unsigned long)pmd);
		return;
	}
#endif
	free_page((unsigned long)pmd);
}

static inline void __pud_populate(pud_t *pud, phys_addr_t pmd, pudval_t prot)
{
	set_pud(pud, __pud(pmd | prot));
}

static inline void pud_populate(struct mm_struct *mm, pud_t *pud, pmd_t *pmd)
{
	__pud_populate(pud, __pa(pmd), PMD_TYPE_TABLE);
}
#else
static inline void __pud_populate(pud_t *pud, phys_addr_t pmd, pudval_t prot)
{
	BUILD_BUG();
}
#endif	/* CONFIG_PGTABLE_LEVELS > 2 */

#if CONFIG_PGTABLE_LEVELS > 3

static inline pud_t *pud_alloc_one(struct mm_struct *mm, unsigned long addr)
{
#ifdef CONFIG_HKIP_PRMEM
	if (unlikely(is_pmalloc((void *)addr, 1)))
		return (pud_t *)pmalloc_get_page_table_page();
#endif
	return (pud_t *)__get_free_page(PGALLOC_GFP);
}

static inline void pud_free(struct mm_struct *mm, pud_t *pud)
{
	BUG_ON((unsigned long)pud & (PAGE_SIZE-1));
#ifdef CONFIG_HKIP_PRMEM
	if (unlikely(is_pmalloc_page_table_page((unsigned long)pud))) {
		pmalloc_put_page_table_page((unsigned long)pud);
		return;
	}
#endif
	free_page((unsigned long)pud);
}

static inline void __pgd_populate(pgd_t *pgdp, phys_addr_t pud, pgdval_t prot)
{
	set_pgd(pgdp, __pgd(pud | prot));
}

static inline void pgd_populate(struct mm_struct *mm, pgd_t *pgd, pud_t *pud)
{
	__pgd_populate(pgd, __pa(pud), PUD_TYPE_TABLE);
}
#else
static inline void __pgd_populate(pgd_t *pgdp, phys_addr_t pud, pgdval_t prot)
{
	BUILD_BUG();
}
#endif	/* CONFIG_PGTABLE_LEVELS > 3 */

extern pgd_t *pgd_alloc(struct mm_struct *mm);
extern void pgd_free(struct mm_struct *mm, pgd_t *pgd);

static inline pte_t *
pte_alloc_one_kernel(struct mm_struct *mm, unsigned long addr)
{
#ifdef CONFIG_HKIP_PRMEM
	if (unlikely(is_pmalloc((void *)addr, 1)))
		return (pte_t *)pmalloc_get_page_table_page();
#endif
	return (pte_t *)__get_free_page(PGALLOC_GFP);
}

#ifdef CONFIG_HKIP_PRMEM
static inline pgtable_t
pmalloc_pte_alloc_one(struct mm_struct *mm, unsigned long addr)
{
	struct page *pte = NULL;
	void *p = NULL;

	p = (void *)pmalloc_get_page_table_page();
	if (unlikely(p == NULL))
		return NULL;
	pte = virt_to_page(p);
	if (!pgtable_page_ctor(pte)) {
		pmalloc_put_page_table_page((unsigned long)p);
		return NULL;
	}
	return pte;
}
#endif

static inline pgtable_t
pte_alloc_one(struct mm_struct *mm, unsigned long addr)
{
	struct page *pte;

#ifdef CONFIG_HKIP_PRMEM
	if (unlikely(is_pmalloc((void *)addr, 1)))
		return pmalloc_pte_alloc_one(mm, addr);
#endif
	pte = alloc_pages(PGALLOC_GFP, 0);
	if (!pte)
		return NULL;
	if (!pgtable_page_ctor(pte)) {
		__free_page(pte);
		return NULL;
	}
	return pte;
}

/*
 * Free a PTE table.
 */
static inline void pte_free_kernel(struct mm_struct *mm, pte_t *pte)
{
#ifdef CONFIG_HKIP_PRMEM
	if (unlikely(pte &&
		     is_pmalloc_page_table_page((unsigned long)pte))) {
		pmalloc_put_page_table_page((unsigned long)pte);
		return;
	}
#endif
	if (pte)
		free_page((unsigned long)pte);
}

static inline void pte_free(struct mm_struct *mm, pgtable_t pte)
{
#ifdef CONFIG_HKIP_PRMEM
	unsigned long addr;
#endif

	pgtable_page_dtor(pte);
#ifdef CONFIG_HKIP_PRMEM
	addr = (unsigned long)page_to_virt(pte);
	if (unlikely(is_pmalloc_page_table_page(addr))) {
		pmalloc_put_page_table_page(addr);
		return;
	}
#endif
	__free_page(pte);
}

static inline void __pmd_populate(pmd_t *pmdp, phys_addr_t pte,
				  pmdval_t prot)
{
	set_pmd(pmdp, __pmd(pte | prot));
}

/*
 * Populate the pmdp entry with a pointer to the pte.  This pmd is part
 * of the mm address space.
 */
static inline void
pmd_populate_kernel(struct mm_struct *mm, pmd_t *pmdp, pte_t *ptep)
{
	/*
	 * The pmd must be loaded with the physical address of the PTE table
	 */
	__pmd_populate(pmdp, __pa(ptep), PMD_TYPE_TABLE);
}

static inline void
pmd_populate(struct mm_struct *mm, pmd_t *pmdp, pgtable_t ptep)
{
	__pmd_populate(pmdp, page_to_phys(ptep), PMD_TYPE_TABLE);
}
#define pmd_pgtable(pmd) pmd_page(pmd)

#endif
