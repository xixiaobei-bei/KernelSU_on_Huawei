/* mmap_phys.c
 *
 * Implement the mmap-interface.
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "mmap_phys.h"

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/semaphore.h>
#include <linux/printk.h>
#include <asm/pgtable.h>

#include "phys_mem.h"
#include "phys_mem_int.h"

#define ROUND_UP_TO(x, y)		(((x) + (y) - 1) / (y) * (y))
#define ROUND_UP_TO_PAGE(x)		ROUND_UP_TO((x), PAGE_SIZE)

/*
 * open and close:	keep track of how many times the device is
 * mapped, to avoid releasing it.
 */
static void phys_mem_vma_open(struct vm_area_struct *vma)
{
	struct phys_mem_session *session =
		(struct phys_mem_session *)vma->vm_private_data;

	if (vma == NULL)
		return;

	down(&session->sem);
	if (session->vmas == 0)
		SET_STATE(session, SESSION_STATE_MAPPED);

	session->vmas++;
	up(&session->sem);
}

static void phys_mem_vma_close(struct vm_area_struct *vma)
{
	struct phys_mem_session *session =
		(struct phys_mem_session *)vma->vm_private_data;

	if (vma == NULL)
		return;

	down(&session->sem);
	session->vmas--;

	if (session->vmas == 0)
		SET_STATE(session, SESSION_STATE_CONFIGURED);

	up(&session->sem);
}

const struct vm_operations_struct phys_mem_vm_ops = {
	.open	= phys_mem_vma_open,
	.close	= phys_mem_vma_close,
};

static int assemble_vma(struct phys_mem_session *session,
	struct vm_area_struct *vma)
{
	unsigned long request_iterator;
	int insert_status = 0;

	if ((session == NULL) || (vma == NULL))
		return -EINVAL;

	for (request_iterator = 0; request_iterator < session->num_frame_stati;
		request_iterator++) {
		struct phys_mem_frame_status *frame_status =
			&session->frame_stati[request_iterator];

		if (frame_status->page) {
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			insert_status = remap_pfn_range(vma, vma->vm_start +
				frame_status->vma_offset_of_first_byte,
				page_to_pfn(frame_status->page),
				PAGE_SIZE*g_conti_nr_pfn, vma->vm_page_prot);

			if (unlikely(insert_status)) {
				pr_err("Could not insert page %p into VMA! Reason: %d",
					frame_status->page, insert_status);
				frame_status->actual_source |=
					SOURCE_ERROR_NOT_MAPPABLE;
				return insert_status;
			}
		}
	}

	return insert_status;
}


int file_mmap_configured(struct file *filp, struct vm_area_struct *vma)
{
	struct phys_mem_session *session = NULL;
	int ret;
	unsigned long max_size;

	if ((filp == NULL) || (vma == NULL))
		return -EINVAL;

	session = (struct phys_mem_session *)filp->private_data;

	if (down_interruptible(&session->sem))
		return -ERESTARTSYS;

	if ((GET_STATE(session) != SESSION_STATE_CONFIGURED) &&
		(GET_STATE(session) != SESSION_STATE_MAPPED)) {
		ret = -EIO;
		pr_err("The session cannot be mmaped in state %i",
			GET_STATE(session));
		up(&session->sem);
		return ret;
	}

	max_size = ROUND_UP_TO_PAGE(g_conti_nr_pfn *
		SESSION_FRAME_STATI_SIZE(session->num_frame_stati));
	max_size <<= PAGE_SHIFT;

	if (vma->vm_end - vma->vm_start > max_size) {
		ret = -EINVAL;
		pr_err("Mmap too large:  %lx > %lx",
			vma->vm_end-vma->vm_start, max_size);
		up(&session->sem);
		return ret;
	}

	ret = assemble_vma(session, vma);
	if (ret) {
		up(&session->sem);
		return ret;
	}
	vma->vm_ops = &phys_mem_vm_ops;
	vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);
	vma->vm_flags |= VM_IO;
	vma->vm_private_data = session;

	up(&session->sem);
	phys_mem_vma_open(vma);
	return ret;
}
