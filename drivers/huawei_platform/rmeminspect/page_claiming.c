/* page_claiming.c
 *
 * provides the user space direct access to physical memory
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

#include "page_claiming.h"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/page-flags.h>
#include <linux/pageblock-flags.h>
#include <linux/page-isolation.h>
#include <linux/gfp.h>
#include <linux/memory_hotplug.h>
#include <linux/vmstat.h>
#include <linux/page_ext.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/log2.h>
#include <linux/gfp.h>
#include <securec.h>

#include "phys_mem.h"

#define pfn(address) page_to_pfn(virt_to_page(((void *)(address))))

unsigned int g_conti_nr_pfn;
unsigned int g_conti_order;
static int alloc_conti_pages_section(struct phys_mem_session *session,
	struct phys_mem_request *request);

int handle_mark_page_poison(struct phys_mem_session *session,
	const struct mark_page_poison *request)
{
	int ret = 0;

	if ((session == NULL) || (request == NULL))
		return -EINVAL;

	if (down_interruptible(&session->sem))
		return -ERESTARTSYS;

	if (unlikely((GET_STATE(session) != SESSION_STATE_MAPPED) &&
		(GET_STATE(session) != SESSION_STATE_CONFIGURED))) {
		pr_err("Session %llu: invalid, IOCTL never appear in state %i\n",
			session->session_id, GET_STATE(session));
		ret = -EINVAL;
		up(&session->sem);
		return ret;
	}

	SetPageHWPoison(pfn_to_page(request->bad_pfn));
	up(&session->sem);

	return ret;
}

int handle_request_pages(struct phys_mem_session *session,
	struct phys_mem_request *request)
{
	int ret = 0;

	if ((session == NULL) || (request == NULL))
		return -EINVAL;

	if (down_interruptible(&session->sem))
		return -ERESTARTSYS;

	if (unlikely((GET_STATE(session) != SESSION_STATE_OPEN) &&
		(GET_STATE(session) != SESSION_STATE_CONFIGURED))) {
		if (unlikely(GET_STATE(session) == SESSION_STATE_CONFIGURING))
			pr_err("Session %llu: The Request never appear in state %i\n",
				session->session_id, GET_STATE(session));
		else
			pr_err("Session %llu: The state should never appear in state %i\n",
				session->session_id, GET_STATE(session));
		ret = -EINVAL;
		up(&session->sem);
		return ret;
	}

	if (GET_STATE(session) == SESSION_STATE_CONFIGURED)
		free_page_stati(session);

	SET_STATE(session, SESSION_STATE_CONFIGURING);

	/*
	 * request.req	points to an array of  request.num_requests many
	 * struct phys_mem_frame_request(s) in userspace.
	 */
	if (request->num_requests == 0) {
		SET_STATE(session, SESSION_STATE_OPEN);
		up(&session->sem);
		return ret;
	}

	session->frame_stati =
		SESSION_ALLOC_NUM_FRAME_STATI(request->num_requests);
	if (session->frame_stati == NULL) {
		ret = -ENOMEM;
		goto out_to_open;
	}

	if (memset_s(session->frame_stati,
		SESSION_FRAME_STATI_SIZE(request->num_requests), 0,
		SESSION_FRAME_STATI_SIZE(request->num_requests)) != EOK) {
		pr_err("%s memset_s failed\n", __func__);
		goto out_to_open;
	}

	session->num_frame_stati = request->num_requests;

	/* Handle all requests */
	g_conti_nr_pfn = request->conti_pageframe_cnt;
	if ((!is_power_of_2(g_conti_nr_pfn)) || (g_conti_nr_pfn == 0))
		goto out_to_open;

	g_conti_order = ilog2(g_conti_nr_pfn);

	/*
	 * The VMA maps all successfully mapped pages in the same order as
	 * they appear here. The relative offset of the frames
	 * gets returned in vma_offset_of_first_byte.
	 */
	alloc_conti_pages_section(session, request);

	SET_STATE(session, SESSION_STATE_CONFIGURED);

	up(&session->sem);
	return ret;

out_to_open:
	pr_notice("The Request IOCTL could not be completed!\n");
	free_page_stati(session);

	SET_STATE(session, SESSION_STATE_OPEN);

	up(&session->sem);

	return ret;
}

/* alloc conti pfn like alloc_gigantic_page */
static bool pfn_range_valid(struct zone *z,
	unsigned long start_pfn, unsigned long nr_pages)
{
	unsigned long i;
	unsigned long end_pfn = start_pfn + nr_pages;
	struct page *page = NULL;

	if (z == NULL)
		return -EINVAL;

	for (i = start_pfn; i < end_pfn; i++) {
		if (!pfn_valid(i))
			return false;

		page = pfn_to_page(i);

		if (page_zone(page) != z)
			return false;

		if (PageReserved(page))
			return false;

		if (page_count(page) > 0)
			return false;

		if (PageHuge(page))
			return false;
	}

	return true;
}

int alloc_conti_pages(unsigned long start_pfn, unsigned long nr_pages)
{
	unsigned long ret;
	unsigned long flags;
	struct zone *z = NULL;

	z = page_zone(pfn_to_page(start_pfn));
	spin_lock_irqsave(&z->lock, flags);

	if (pfn_range_valid(z, start_pfn, nr_pages)) {
		spin_unlock_irqrestore(&z->lock, flags);

		ret = alloc_contig_range(start_pfn, start_pfn + nr_pages,
			MIGRATE_MOVABLE, GFP_KERNEL | __GFP_NOWARN);
		if (!ret)
			return 0;
		spin_lock_irqsave(&z->lock, flags);
	}
	spin_unlock_irqrestore(&z->lock, flags);

	return 1;
}

static int alloc_conti_pages_section(struct phys_mem_session *session,
	struct phys_mem_request *request)
{
	int ret;
	int i;
	int j;
	int pfn_invalid_flag = 0;
	unsigned long current_offset_in_vma = 0;
	unsigned long start_pfn = 0;

	if ((session == NULL) || (request == NULL))
		return -EINVAL;

	for (i = 0; i < request->num_requests; i++) {
		struct phys_mem_frame_request __user *current_pfn_request =
						&request->req[i];
		struct phys_mem_frame_status __kernel *current_pfn_status =
						&session->frame_stati[i];

		if (copy_from_user(&current_pfn_status->request,
			current_pfn_request,
			sizeof(struct phys_mem_frame_request))) {
			return CLAIMED_TRY_NEXT;
		}

		if ((current_pfn_status->request.requested_pfn) %
			g_conti_nr_pfn != 0) {
			return CLAIMED_TRY_NEXT;
		}

		start_pfn = current_pfn_status->request.requested_pfn;
		pfn_invalid_flag = 0;
		for (j = 0; j < g_conti_nr_pfn; j++) {
			if (unlikely(!pfn_valid(start_pfn + j)))
				pfn_invalid_flag++;
		}
		if (pfn_invalid_flag != 0)
			continue;

		if (unlikely(!(current_pfn_status->request.allowed_sources &
			SOURCE_FREE_CONTI_PAGE)))
			continue;

		ret = alloc_conti_pages(start_pfn, g_conti_nr_pfn);

		if (ret == 0) {
			current_pfn_status->pfn = start_pfn;
			current_pfn_status->page = pfn_to_page(start_pfn);
			current_pfn_status->actual_source =
				SOURCE_FREE_CONTI_PAGE;
			current_pfn_status->vma_offset_of_first_byte =
			current_offset_in_vma;
			current_offset_in_vma += PAGE_SIZE*g_conti_nr_pfn;
		} else {
			current_pfn_status->pfn = 0;
			current_pfn_status->page = NULL;
			current_pfn_status->actual_source = 0;
			current_pfn_status->vma_offset_of_first_byte = 0;
		}
	}

	return CLAIMED_SUCCESSFULLY;
}
