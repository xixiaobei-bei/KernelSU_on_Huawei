/* SPDX-License-Identifier: GPL-2.0 */
/*
 * client_writeback.h
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: yi.zhang@huawei.com
 * Create: 2021-01-11
 *
 */

#ifndef CLIENT_WRITEBACK_H
#define CLIENT_WRITEBACK_H

#include "hmdfs.h"

/*
 * HM_DEFAULT_WRITEBACK_INTERVAL - centiseconds
 * HMDFS_FILE_BG_WB_BYTES - background per-file threshold 10M
 * HMDFS_FS_BG_WB_BYTES - background per-fs threshold 50M
 * HMDFS_FILE_WB_BYTES - per-file throttle threshold
 * HMDFS_FS_WB_BYTES - per-fs throttle threshold
 */
#define HM_DEFAULT_WRITEBACK_INTERVAL	500
#define HMDFS_FILE_BG_WB_BYTES		(10 * 1024 * 1024)
#define HMDFS_FS_BG_WB_BYTES		(50 * 1024 * 1024)
#define HMDFS_FILE_WB_BYTES		(HMDFS_FILE_BG_WB_BYTES << 1)
#define HMDFS_FS_WB_BYTES		(HMDFS_FS_BG_WB_BYTES << 1)

/* writeback time limit (default 5s) */
#define HMDFS_DEF_WB_TIMELIMIT		(5 * HZ)
#define HMDFS_MAX_WB_TIMELIMIT		(30 * HZ)

/* bandwidth adjusted lower limit (default 1MB/s) */
#define HMDFS_BW_THRESH_MIN_LIMIT	(1 << (20 - PAGE_SHIFT))
#define HMDFS_BW_THRESH_MAX_LIMIT	(100 << (20 - PAGE_SHIFT))
#define HMDFS_BW_THRESH_DEF_LIMIT	HMDFS_BW_THRESH_MIN_LIMIT

#define HMDFS_DIRTY_EXCEED_RATELIMIT	(32 >> (PAGE_SHIFT - 10))
#define HMDFS_RATELIMIT_PAGES_GAP	16
#define HMDFS_DEF_RATELIMIT_PAGES	32
#define HMDFS_MIN_RATELIMIT_PAGES	1

struct hmdfs_dirty_throttle_control {
	struct hmdfs_writeback *hwb;
	/* last time threshes are updated */
	unsigned long thresh_time_stamp;

	unsigned long file_bg_thresh;
	unsigned long fs_bg_thresh;
	unsigned long file_thresh;
	unsigned long fs_thresh;

	unsigned long file_nr_dirty;
	unsigned long fs_nr_dirty;
	unsigned long file_nr_reclaimable;
	unsigned long fs_nr_reclaimable;
};

struct hmdfs_writeback {
	struct hmdfs_sb_info *sbi;
	struct bdi_writeback *wb;
	/* enable hmdfs dirty writeback control */
	bool dirty_writeback_control;

	/* writeback per-file inode list */
	struct list_head inode_list_head;
	spinlock_t inode_list_lock;

	/* centiseconds */
	unsigned int dirty_writeback_interval;
	/* per-file background threshold */
	unsigned long dirty_file_bg_bytes;
	unsigned long dirty_file_bg_thresh;
	/* per-fs background threshold */
	unsigned long dirty_fs_bg_bytes;
	unsigned long dirty_fs_bg_thresh;
	/* per-file throttle threshold */
	unsigned long dirty_file_bytes;
	unsigned long dirty_file_thresh;
	/* per-fs throttle threshold */
	unsigned long dirty_fs_bytes;
	unsigned long dirty_fs_thresh;
	/* ratio between background thresh and throttle thresh */
	unsigned long fs_bg_ratio;
	unsigned long file_bg_ratio;
	/* ratio between file and fs throttle thresh */
	unsigned long fs_file_ratio;

	/*
	 * Enable auto-thresh. If enabled, the background and throttle
	 * thresh are nolonger a fixed value storeed in dirty_*_bytes,
	 * they are determined by the bandwidth of the network and the
	 * writeback timelimit.
	 */
	bool dirty_auto_threshold;
	unsigned int writeback_timelimit;
	/* bandwitdh adjusted filesystem throttle thresh */
	unsigned long bw_fs_thresh;
	/* bandwidth adjusted per-file throttle thresh */
	unsigned long bw_file_thresh;
	/* bandwidth adjusted thresh lower limit */
	unsigned long bw_thresh_lowerlimit;

	/* reclaimable pages exceed throttle thresh */
	bool dirty_exceeded;
	/* percpu dirty pages ratelimit */
	long ratelimit_pages;
	/* count percpu dirty pages */
	int __percpu *bdp_ratelimits;

	/* per-fs writeback work */
	struct workqueue_struct *dirty_sb_writeback_wq;
	struct delayed_work dirty_sb_writeback_work;
	/* per-file writeback work */
	struct workqueue_struct *dirty_inode_writeback_wq;
	struct delayed_work dirty_inode_writeback_work;

	/* per-fs writeback bandwidth */
	spinlock_t write_bandwidth_lock;
	unsigned long max_write_bandwidth;
	unsigned long min_write_bandwidth;
	unsigned long avg_write_bandwidth;
};

void hmdfs_writeback_inodes_sb_handler(struct work_struct *work);

void hmdfs_writeback_inode_handler(struct work_struct *work);

void hmdfs_calculate_dirty_thresh(struct hmdfs_writeback *hwb);

void hmdfs_update_ratelimit(struct hmdfs_writeback *hwb);

void hmdfs_balance_dirty_pages_ratelimited(struct address_space *mapping);

void hmdfs_destroy_writeback(struct hmdfs_sb_info *sbi);

int hmdfs_init_writeback(struct hmdfs_sb_info *sbi);

#endif
