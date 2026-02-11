#ifndef CGROUP_WORKINGSET_H_INCLUDED
#define CGROUP_WORKINGSET_H_INCLUDED

#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/cgroup.h>
#include <linux/pagemap.h>

extern void workingset_pagecache_record(struct file *file,
	pgoff_t start_offset, unsigned count, bool is_pagefault);

static inline void workingset_pagecache_on_ptefault(
	struct vm_fault *vmf)
{
	struct vm_area_struct *vma;
	pgoff_t pgoff;

	if (likely(!(current->flags & PF_WSCG_MONITOR)))
		return;
	if (!vmf || (vmf->flags & FAULT_FLAG_WRITE)
		|| PageSwapBacked(pte_page(vmf->orig_pte)))
		return;

	vma = vmf->vma;
	pgoff = linear_page_index(vma, vmf->address);
	workingset_pagecache_record(vma->vm_file, pgoff, 1, true);
}

static inline void workingset_pagecache_on_pagefault(struct file *file,
	pgoff_t start_offset)
{
	if (likely(!(current->flags & PF_WSCG_MONITOR)))
		return;

	workingset_pagecache_record(file, start_offset, 1, true);
}

static inline void workingset_pagecache_on_readfile(struct file *file,
	loff_t *pos, pgoff_t index, unsigned long offset)
{
	pgoff_t start_offset, end_offset;

	if (likely(!(current->flags & PF_WSCG_MONITOR)))
		return;

	if (!pos || *pos >= ((loff_t)index << PAGE_SHIFT) + offset)
		return;

	start_offset = ((unsigned long)*pos) >> PAGE_SHIFT;
	end_offset = index + (offset >> PAGE_SHIFT);
	workingset_pagecache_record(file, start_offset,
			end_offset - start_offset + 1, false);
}
#endif /* CGROUP_WORKINGSET_H_INCLUDED */
