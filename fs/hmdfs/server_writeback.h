/* SPDX-License-Identifier: GPL-2.0 */
/*
 * server_writeback.h
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: chengzhihao1@huawei.com
 * Create: 2021-1-13
 *
 */

#ifndef SERVER_WRITEBACK_H
#define SERVER_WRITEBACK_H

#include "hmdfs.h"

#define HMDFS_MB_TO_PAGE_SHIFT	(20 - HMDFS_PAGE_OFFSET)

struct hmdfs_server_writeback {
	struct hmdfs_sb_info *sbi;
	/* Enable hmdfs server dirty writeback control */
	bool dirty_writeback_control;

	/* Current # of dirty pages from remote client in recent 1s */
	atomic_t dirty_nr_pages;
	/* Current # of dirty pages to writeback */
	int dirty_nr_pages_to_wb;
	/* Dirty thresh(Dirty data pages in 1s) to trigger wb */
	unsigned int dirty_thresh_pg;
	/* Last reset timestamp(in jiffies) for @dirty_nr_pages */
	unsigned long last_reset_time;

	struct workqueue_struct *dirty_writeback_wq;
	/* Per-fs pages from client writeback work */
	struct work_struct dirty_sb_writeback_work;
};

void hmdfs_server_check_writeback(struct hmdfs_server_writeback *hswb);

void hmdfs_destroy_server_writeback(struct hmdfs_sb_info *sbi);

int hmdfs_init_server_writeback(struct hmdfs_sb_info *sbi);

#endif
