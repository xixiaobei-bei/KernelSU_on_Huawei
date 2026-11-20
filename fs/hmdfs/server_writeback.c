/* SPDX-License-Identifier: GPL-2.0 */
/*
 * server_writeback.c
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: chengzhihao1@huawei.com
 * Create: 2021-1-7
 *
 */
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/backing-dev.h>

#include "hmdfs.h"
#include "hmdfs_trace.h"
#include "server_writeback.h"

#define HMDFS_SRV_WB_DEF_DIRTY_THRESH	50UL

static void hmdfs_srv_wb_handler(struct work_struct *work)
{
	struct hmdfs_server_writeback *hswb = container_of(work,
					      struct hmdfs_server_writeback,
					      dirty_sb_writeback_work);
	struct super_block *lower_sb = hswb->sbi->lower_sb;
	int dirty_pages;

	if (writeback_in_progress(&lower_sb->s_bdi->wb) ||
	    !down_read_trylock(&lower_sb->s_umount))
		return;

	dirty_pages = hswb->dirty_nr_pages_to_wb;
	writeback_inodes_sb_nr(lower_sb, dirty_pages, WB_REASON_FS_FREE_SPACE);
	up_read(&lower_sb->s_umount);

	trace_hmdfs_start_srv_wb(hswb->sbi, dirty_pages, hswb->dirty_thresh_pg);
}

void hmdfs_server_check_writeback(struct hmdfs_server_writeback *hswb)
{
	unsigned long old_time, now;
	int dirty_nr_pages;

	old_time = hswb->last_reset_time;
	now = jiffies;
	dirty_nr_pages = atomic_inc_return(&hswb->dirty_nr_pages);
	if (time_after(now, old_time + HZ) &&
	    cmpxchg(&hswb->last_reset_time, old_time, now) == old_time) {
		/*
		 * We calculate the speed of page dirting to handle
		 * following situations:
		 *
		 * 1. Dense writing, average page writing speed
		 *    exceeds @hswb->dirty_thresh_pg:
		 *    0-1s 100MB
		 * 2. Sporadic writing, average page writing speed
		 *    belows @hswb->dirty_thresh_pg:
		 *    0-0.1s	40MB
		 *    3.1-3.2	20MB
		 */
		unsigned int writepage_speed;

		writepage_speed = dirty_nr_pages / ((now - old_time) / HZ);
		if (writepage_speed >= hswb->dirty_thresh_pg) {
			/*
			 * Writeback @hswb->dirty_nr_pages_to_wb pages in
			 * server-writeback work. If work is delayed after
			 * 1s, @hswb->dirty_nr_pages_to_wb could be assigned
			 * another new value (eg. 60MB), the old value (eg.
			 * 80MB) will be overwritten, which means 80MB data
			 * will be ommitted to writeback. We can tolerate this
			 * situation, The writeback pressure is too high if
			 * the previous work is not completed, so it's
			 * meaningless to continue subsequent work.
			 */
			hswb->dirty_nr_pages_to_wb = dirty_nr_pages;
			/*
			 * There are 3 conditions to trigger queuing work:
			 *
			 * A. Server successfully handles writepage for client
			 * B. Every 1 second interval
			 * C. Speed for page dirting exceeds @dirty_thresh_pg
			 */
			queue_work(hswb->dirty_writeback_wq,
				   &hswb->dirty_sb_writeback_work);
		}

		/*
		 * There is no need to account the number of dirty pages
		 * from remote client very accurately. Allow the missing
		 * count to increase by other process in the gap between
		 * increment and zero out.
		 */
		atomic_set(&hswb->dirty_nr_pages, 0);
	}
}

void hmdfs_destroy_server_writeback(struct hmdfs_sb_info *sbi)
{
	if (!sbi->h_swb)
		return;

	flush_work(&sbi->h_swb->dirty_sb_writeback_work);
	destroy_workqueue(sbi->h_swb->dirty_writeback_wq);
	kfree(sbi->h_swb);
	sbi->h_swb = NULL;
}

int hmdfs_init_server_writeback(struct hmdfs_sb_info *sbi)
{
	struct hmdfs_server_writeback *hswb;
	char name[HMDFS_WQ_NAME_LEN];

	hswb = kzalloc(sizeof(struct hmdfs_server_writeback), GFP_KERNEL);
	if (!hswb)
		return -ENOMEM;

	hswb->sbi = sbi;
	hswb->dirty_writeback_control = true;
	hswb->dirty_thresh_pg = HMDFS_SRV_WB_DEF_DIRTY_THRESH <<
				HMDFS_MB_TO_PAGE_SHIFT;
	atomic_set(&hswb->dirty_nr_pages, 0);
	hswb->last_reset_time = jiffies;

	snprintf(name, sizeof(name), "dfs_srv_wb%u", sbi->seq);
	hswb->dirty_writeback_wq = create_singlethread_workqueue(name);
	if (!hswb->dirty_writeback_wq) {
		hmdfs_err("Failed to create server writeback workqueue!");
		kfree(hswb);
		return -ENOMEM;
	}
	INIT_WORK(&hswb->dirty_sb_writeback_work, hmdfs_srv_wb_handler);
	sbi->h_swb = hswb;

	return 0;
}

