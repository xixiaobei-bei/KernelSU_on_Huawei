/*
 * linux/vm_copy.h
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2020. All rights reserved.
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

#ifndef _VM_COPY_H
#define _VM_COPY_H

#include <linux/mm.h>

#ifdef CONFIG_VM_COPY
unsigned long vm_copy(struct mm_struct *mm, unsigned long src,
			unsigned long dst, unsigned long size);
void mask_vma_ext_flags(struct vm_area_struct *vma, unsigned long flags);
#else
static inline unsigned long vm_copy(struct mm_struct *mm, unsigned long src,
			unsigned long dst, unsigned long size)
{
	return -EINVAL; /*lint !e570 */
}
static inline void mask_vma_ext_flags(struct vm_area_struct *vma, unsigned long flags)
{
}
#endif /* CONFIG_VM_COPY END */

#endif /* _VM_COPY_H */
