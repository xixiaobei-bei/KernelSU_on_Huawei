/*
 * workingset_cgroup.h
 *
 * Provide external call interfaces of the workingset cgrpup.
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef CGROUP_WORKINGSET_H_INCLUDED
#define CGROUP_WORKINGSET_H_INCLUDED

#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/cgroup.h>
#include <linux/pagemap.h>

#define page_off(pos)	(((unsigned long)(pos)) >> PAGE_SHIFT)
#define index_off(index, pos)	((index) + ((pos) >> PAGE_SHIFT))

int workingset_preread_by_self(void);
void workingset_pagecache_record(
	struct file *file, pgoff_t start_offset,
	unsigned int count, bool is_pagefault);

#ifdef CONFIG_HISI_LB
static inline bool workingset_pagecache_skip_ptefault(struct vm_fault *vmf)
{
	return likely(!(current->ext_flags & PF_EXT_WSCG_MONITOR)) || !vmf ||
		(vmf->flags & FAULT_FLAG_WRITE) || pte_gid(vmf->orig_pte) ||
		PageSwapBacked(pte_page(vmf->orig_pte));
}
#else
static inline bool workingset_pagecache_skip_ptefault(struct vm_fault *vmf)
{
	return likely(!(current->ext_flags & PF_EXT_WSCG_MONITOR)) || !vmf ||
		(vmf->flags & FAULT_FLAG_WRITE) ||
		PageSwapBacked(pte_page(vmf->orig_pte));
}
#endif

static inline void workingset_pagecache_on_ptefault(struct vm_fault *vmf)
{
	if (!workingset_pagecache_skip_ptefault(vmf))
		workingset_pagecache_record(vmf->vma->vm_file,
			linear_page_index(vmf->vma, vmf->address), 1, true);
}

static inline void workingset_pagecache_on_pagefault(
	struct file *file, pgoff_t start_offset)
{
	if (likely(!(current->ext_flags & PF_EXT_WSCG_MONITOR)))
		return;

	workingset_pagecache_record(file, start_offset, 1, true);
}

static inline bool workingset_pagecache_skip_readfile(
	loff_t *pos, pgoff_t index, unsigned long offset)
{
	return likely(!(current->ext_flags & PF_EXT_WSCG_MONITOR)) ||
		!pos || (*pos >= ((loff_t)index << PAGE_SHIFT) + offset);
}

static inline void workingset_pagecache_on_readfile(
	struct file *file, loff_t *pos, pgoff_t index, unsigned long offset)
{
	if (!workingset_pagecache_skip_readfile(pos, index, offset))
		workingset_pagecache_record(file, page_off(*pos),
			index_off(index, offset) - page_off(*pos) + 1, false);
}
#endif /* CGROUP_WORKINGSET_H_INCLUDED */
