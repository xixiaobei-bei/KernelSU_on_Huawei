/*
 * vm_copy.c
 *
 * Copyright(C) 2020 Huawei Technologies Co., Ltd. All rights reserved.
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

#define pr_fmt(fmt) "[VM_COPY:]" fmt

#include <linux/mm.h>
#include <linux/mm_inline.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/mmu_notifier.h>
#include <linux/rwsem.h>
#include <linux/rmap.h>
#include <linux/string.h>
#include <linux/swap.h>
#include <linux/swapops.h>

#include <asm/pgalloc.h>
#include <asm/system_misc.h>

#include "internal.h"

struct vm_copy_struct {
	struct mm_struct *mm;
	struct vm_area_struct *src_vma;
	struct vm_area_struct *dst_vma;
	unsigned long src;
};

static void update_rss(struct mm_struct *mm, int *rss, int num)
{
	int i;

	if (num != NR_MM_COUNTERS) {
		pr_err("rss length must be NR_MM_COUNTERS\n");
		return;
	}

	sync_mm_rss(mm);
	for (i = 0; i < NR_MM_COUNTERS; i++)
		if (rss[i])
			add_mm_counter(mm, i, rss[i]);
}

static pte_t *find_pte_map_lock(struct mm_struct *mm, unsigned long addr,
				spinlock_t **addr_ptl)
{
	pgd_t *pgd = NULL;
	p4d_t *p4d = NULL;
	pud_t *pud = NULL;
	pmd_t *pmd = NULL;
	pte_t *pte = NULL;

	pgd = pgd_offset(mm, addr);
	if (pgd_none_or_clear_bad(pgd)) {
		pr_err("bad pgd\n");
		return NULL;
	}

	p4d = p4d_offset(pgd, addr);
	if (p4d_none_or_clear_bad(p4d)) {
		pr_err("bad p4d\n");
		return NULL;
	}

	pud = pud_offset(p4d, addr);
	if (pud_none_or_clear_bad(pud)) {
		pr_err("bad pud\n");
		return NULL;
	}

	pmd = pmd_offset(pud, addr);
	if (pmd_none_or_clear_bad(pmd)) {
		pr_err("bad pmd\n");
		return NULL;
	}

	pte = pte_offset_map_lock(mm, pmd, addr, addr_ptl);
	if (pte_none(*pte)) {
		pr_err("bad pte\n");
		pte_unmap_unlock(pte, *addr_ptl);
		return NULL;
	}

	return pte;
}

static void vm_copy_del_page_from_lru(struct page *page)
{
	int ret = -EBUSY;
	unsigned long flags = 0;
	struct zone *zone = page_zone(page);

	spin_lock_irqsave(zone_lru_lock(zone), flags);
	if (PageLRU(page)) {
		struct lruvec *lruvec;

		lruvec = mem_cgroup_page_lruvec(page, zone->zone_pgdat);
		get_page(page);
		ClearPageLRU(page);
		del_page_from_lru_list(page, lruvec, page_lru(page));
		ret = 0;
	}
	spin_unlock_irqrestore(zone_lru_lock(zone), flags);

	if (!ret) {
		put_page(page);
		mem_cgroup_uncharge(page);
	}

	return ret;
}

static void vm_copy_free_mem(struct vm_area_struct *vma, pte_t *pte,
				unsigned long addr, int *rss)
{
	struct page *page = NULL;

	if (pte_present(*pte)) {
		pte_t pteval = ptep_clear_flush(vma, addr, pte);

		page = vm_normal_page(vma, addr, pteval);
		if (page) {
			rss[mm_counter(page)]--;
			page_remove_rmap(page, false);
			put_page(page);
		} else if (pte_devmap(pteval)) {
			page = pte_page(pteval);
			if (is_device_public_page(page)) {
				rss[mm_counter(page)]--;
				page_remove_rmap(page, false);
				put_page(page);
			}
		}
	}
}

static unsigned long __copy_pte_single(struct vm_copy_struct *vms,
			pte_t *src_pte, pte_t *dst_pte,
			unsigned long src, unsigned long dst)
{
	unsigned long vm_flags = vms->src_vma->vm_flags;
	pte_t pte = *src_pte;
	struct mm_struct *mm = vms->mm;
	struct page *page = NULL;
	int rss[NR_MM_COUNTERS] = {0};

	/*
	 * If it's a COW mapping, write protect it both
	 * in the parent and the child
	 */
	if (is_cow_mapping(vm_flags)) {
		ptep_set_wrprotect(mm, src, src_pte);
		pte = pte_wrprotect(pte);
	}

	/*
	 * If it's a shared mapping, mark it clean in
	 * the child
	 */
	if (vm_flags & VM_SHARED)
		pte = pte_mkclean(pte);
	pte = pte_mkold(pte);

	page = vm_normal_page(vms->src_vma, src, pte);
	if (page) {
		get_page(page);
		page_dup_rmap(page, false);
		rss[mm_counter(page)]++;
		if ((vms->dst_vma->ext_flags & VM_COPY_COW) &&
					!PageVMcpy(page)) {
			SetPageVMcpy(page);
			vm_copy_del_page_from_lru(page);
		}
	} else if (pte_devmap(pte)) {
		page = pte_page(pte);
		/*
		 * Cache coherent device memory behave like regular page and
		 * not like persistent memory page. For more information see
		 * MEMORY_DEVICE_CACHE_COHERENT in memory_hotplug.h
		 */
		if (is_device_public_page(page)) {
			get_page(page);
			page_dup_rmap(page, false);
			rss[mm_counter(page)]++;
		}
	}

	if (vms->dst_vma->ext_flags & VM_COPY_COW)
		mmu_notifier_change_pte_vmcpy(mm, vms->dst_vma, dst, pte);

	if (!pte_none(*dst_pte))
		vm_copy_free_mem(vms->dst_vma, dst_pte, dst, rss);

	set_pte_at(mm, dst, dst_pte, pte);

	update_rss(mm, rss, NR_MM_COUNTERS);

	return 0;
}

static int vm_copy_pte(struct vm_copy_struct *vms, pmd_t *dst_pmd,
			unsigned long dst, unsigned long dst_end)
{
	pte_t *src_pte = NULL;
	pte_t *dst_pte = NULL;
	pte_t *orig_src_pte = NULL;
	pte_t *orig_dst_pte = NULL;
	spinlock_t *ptl = NULL;
	spinlock_t *src_ptl = NULL;
	unsigned long src_addr = vms->src;
	unsigned long dst_addr = dst;
	swp_entry_t entry = (swp_entry_t){0};
	int ret = 0;

	dst_pte = pte_alloc_map_lock(vms->mm, dst_pmd, dst, &ptl);
	if (!dst_pte)
		return -ENOMEM;

	orig_dst_pte = dst_pte;
	arch_enter_lazy_mmu_mode();

	do {
		src_pte = find_pte_map_lock(vms->mm, src_addr, &src_ptl);
		orig_src_pte = src_pte;

		if (!src_pte) {
			pr_err("src_pte is none\n");
			ret =  -ENOMEM;
			break;
		}

		if (unlikely(!pte_present(*dst_pte) && !pte_none(*dst_pte))) {
			spin_unlock(ptl);
			handle_mm_fault(vms->dst_vma, dst_addr, 0);
			spin_lock(ptl);
		}

		if (unlikely(!pte_present(*src_pte))) {
			spin_unlock(src_ptl);
			handle_mm_fault(vms->src_vma, src_addr, 0);
			spin_lock(src_ptl);
		}

		entry.val = __copy_pte_single(vms, src_pte, dst_pte,
					src_addr, dst_addr);
		if (entry.val) {
			pte_unmap_unlock(orig_src_pte, src_ptl);
			break;
		}

		pte_unmap_unlock(orig_src_pte, src_ptl);
	} while (dst_pte++, dst_addr += PAGE_SIZE, src_addr += PAGE_SIZE,
		dst_addr < dst_end);

	vms->src = src_addr;
	arch_leave_lazy_mmu_mode();
	pte_unmap_unlock(orig_dst_pte, ptl);
	cond_resched();

	if (entry.val) {
		if (add_swap_count_continuation(entry, GFP_KERNEL) < 0)
			return -ENOMEM;
	}

	flush_cache_range(vms->dst_vma, dst, dst_end);
	return ret;
}

static int vm_copy_pmd(struct vm_copy_struct *vms, pud_t *dst_pud,
			unsigned long dst, unsigned long dst_end)
{
	pmd_t *dst_pmd = NULL;
	unsigned long dst_addr = dst;
	unsigned long dst_next;

	dst_pmd = pmd_alloc(vms->mm, dst_pud, dst);
	if (!dst_pmd)
		return -ENOMEM;

	do {
		dst_next = pmd_addr_end(dst_addr, dst_end);

		if (vm_copy_pte(vms, dst_pmd, dst_addr, dst_next))
			return -ENOMEM;
	} while (dst_pmd++, dst_addr = dst_next, dst_addr < dst_end);

	return 0;
}

static int vm_copy_pud(struct vm_copy_struct *vms, p4d_t *dst_p4d,
			unsigned long dst, unsigned long dst_end)
{
	pud_t *dst_pud = NULL;
	unsigned long dst_addr = dst;
	unsigned long dst_next;

	dst_pud = pud_alloc(vms->mm, dst_p4d, dst);
	if (!dst_pud)
		return -ENOMEM;

	do {
		dst_next = pud_addr_end(dst_addr, dst_end);

		if (vm_copy_pmd(vms, dst_pud, dst_addr, dst_next))
			return -ENOMEM;

	} while (dst_pud++, dst_addr = dst_next, dst_addr < dst_end);

	return 0;
}

static int vm_copy_p4d(struct vm_copy_struct *vms, pgd_t *dst_pgd,
			unsigned long dst, unsigned long dst_end)
{

	p4d_t *dst_p4d = NULL;
	unsigned long dst_addr = dst;
	unsigned long dst_next;

	dst_p4d = p4d_alloc(vms->mm, dst_pgd, dst);
	if (!dst_p4d)
		return -ENOMEM;

	do {
		dst_next = p4d_addr_end(dst_addr, dst_end);

		if (vm_copy_pud(vms, dst_p4d, dst_addr, dst_next))
			return -ENOMEM;

	} while (dst_p4d++, dst_addr = dst_next, dst_addr < dst_end);

	return 0;
}

static int vm_copy_pgd(struct vm_copy_struct *vms, unsigned long dst,
			unsigned long dst_end)
{
	pgd_t *dst_pgd = pgd_offset(vms->mm, dst);
	unsigned long dst_addr = dst;
	unsigned long dst_next;

	do {
		dst_next = pgd_addr_end(dst_addr, dst_end);

		if (unlikely(vm_copy_p4d(vms, dst_pgd, dst_addr, dst_next)))
			return -ENOMEM;
	} while (dst_pgd++, dst_addr = dst_next, dst_addr < dst_end);

	return 0;
}

void mask_vma_ext_flags(struct vm_area_struct *vma, unsigned long flags)
{
	vma->ext_flags |= flags;
}

unsigned long vm_copy(struct mm_struct *mm, unsigned long src,
		unsigned long dst, unsigned long size)
{
	struct vm_area_struct *src_vma = NULL;
	struct vm_area_struct *dst_vma = NULL;
	struct vm_copy_struct vms;
	unsigned long src_end, dst_end;
	unsigned long ret;
	bool is_cow = false;

	if (!mm || !size) {
		pr_err("vm_copy paras error\n");
		return -EINVAL; /*lint !e570 */
	}

	size = PAGE_ALIGN(size);
	src_end = src + size;
	dst_end = dst + size;

	if (down_write_killable(&mm->mmap_sem))
		return -EINTR; /*lint !e570 */

	src_vma = find_extend_vma(mm, src);
	dst_vma = find_extend_vma(mm, dst);
	if (!src_vma || !dst_vma) {
		pr_err("can not find vma\n");
		ret = -EINVAL; /*lint !e570 */
		goto out;
	}

	if (dst_vma->ext_flags & NO_VM_COPY) {
		pr_err("bypass vm_copy\n");
		ret = -EINVAL; /*lint !e570 */
		goto out;
	}

	if (src_vma->ext_flags & VM_COPY_COW) {
		pr_err("src vma ext_flags mark\n");
		ret = -EINVAL; /*lint !e570 */
		goto out;
	}

	if ((dst_vma->ext_flags & VM_COPY_COW) &&
			((src_vma->ext_flags & VM_COPY_CMA) ||
			(dst_vma->ext_flags & VM_COPY_CMA))) {
		pr_err("src or dst page contains CMA page\n");
		ret = -EINVAL; /*lint !e570 */
		goto out;
	}

	if (src_vma->vm_end - src_vma->vm_start < size) {
		pr_err("size too large(src)\n");
		ret = -EINVAL; /*lint !e570 */
		goto out;
	}

	if (dst_vma->vm_end - dst_vma->vm_start < size) {
		pr_err("size too large(dst)\n");
		ret = -EINVAL; /*lint !e570 */
		goto out;
	}

	vms.mm = mm;
	vms.src_vma = src_vma;
	vms.dst_vma = dst_vma;
	mask_vma_ext_flags(vms.dst_vma, VM_COPY_DONE);
	vms.src = src;
#ifdef CONFIG_SPECULATIVE_PAGE_FAULT
	vm_write_begin(src_vma);
	vm_write_begin(dst_vma);
#endif
	is_cow = is_cow_mapping(src_vma->vm_flags);
	if (is_cow)
		mmu_notifier_invalidate_range_start(mm, src, src_end);

	if (vms.dst_vma->ext_flags & VM_COPY_COW)
		mmu_notifier_invalidate_range_start_vmcpy(mm,
					dst_vma, dst, dst_end);

	flush_cache_range(src_vma, src, src_end);

	ret = vm_copy_pgd(&vms, dst, dst_end);
	if (ret) {
		ret = -ENOMEM; /*lint !e570 */
		goto out_notifier_end;
	}

	ret = dst;
	count_vm_events(VM_COPY_TOTAL_PAGE, size >> PAGE_SHIFT);
out_notifier_end:
	if (vms.dst_vma->ext_flags & VM_COPY_COW)
		mmu_notifier_invalidate_range_end_vmcpy(mm,
					dst_vma, dst, dst_end);

	if (is_cow)
		mmu_notifier_invalidate_range_end(mm, src, src_end);
#ifdef CONFIG_SPECULATIVE_PAGE_FAULT
	vm_write_end(src_vma);
	vm_write_end(dst_vma);
#endif
out:
	up_write(&mm->mmap_sem);
	flush_tlb_mm(mm);

	return ret;
}
