/* SPDX-License-Identifier: GPL-2.0 */
/*
 * client_writeback.c
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: yi.zhang@huawei.com
 * Create: 2021-1-6
 *
 */

#include <linux/backing-dev.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/page-flags.h>
#include <linux/pagemap.h>
#include <linux/pagevec.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>

#include "hmdfs.h"
#include "hmdfs_trace.h"

/* 200ms */
#define HMDFS_MAX_PAUSE			max((HZ / 5), 1)
#define HMDFS_BANDWIDTH_INTERVAL	max((HZ / 5), 1)
/* Dirty type */
#define HMDFS_DIRTY_FS			0
#define HMDFS_DIRTY_FILE		1
/* Exceed flags */
#define HMDFS_FS_EXCEED			(1 << HMDFS_DIRTY_FS)
#define HMDFS_FILE_EXCEED		(1 << HMDFS_DIRTY_FILE)
/* Ratelimit calculate shift */
#define HMDFS_LIMIT_SHIFT		10

void hmdfs_writeback_inodes_sb_handler(struct work_struct *work)
{
	struct hmdfs_writeback *hwb = container_of(
		work, struct hmdfs_writeback, dirty_sb_writeback_work.work);

	try_to_writeback_inodes_sb(hwb->sbi->sb, WB_REASON_FS_FREE_SPACE);
}

void hmdfs_writeback_inode_handler(struct work_struct *work)
{
	struct hmdfs_inode_info *info = NULL;
	struct inode *inode = NULL;
	struct hmdfs_writeback *hwb = container_of(
		work, struct hmdfs_writeback, dirty_inode_writeback_work.work);

	spin_lock(&hwb->inode_list_lock);
	while (likely(!list_empty(&hwb->inode_list_head))) {
		info = list_first_entry(&hwb->inode_list_head,
					struct hmdfs_inode_info, wb_list);
		list_del_init(&info->wb_list);
		spin_unlock(&hwb->inode_list_lock);

		inode = &info->vfs_inode;
		write_inode_now(inode, 0);
		iput(inode);
		spin_lock(&hwb->inode_list_lock);
	}
	spin_unlock(&hwb->inode_list_lock);
}

static void hmdfs_writeback_inodes_sb_delayed(struct super_block *sb,
					      unsigned int delay)
{
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	unsigned long timeout;

	timeout = msecs_to_jiffies(delay);
	if (!timeout || !work_busy(&sbi->h_wb->dirty_sb_writeback_work.work))
		mod_delayed_work(sbi->h_wb->dirty_sb_writeback_wq,
				 &sbi->h_wb->dirty_sb_writeback_work, timeout);
}

static inline void hmdfs_writeback_inodes_sb(struct super_block *sb)
{
	hmdfs_writeback_inodes_sb_delayed(sb, 0);
}

static void hmdfs_writeback_inode(struct super_block *sb, struct inode *inode)
{
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	struct hmdfs_writeback *hwb = sbi->h_wb;
	struct hmdfs_inode_info *info = hmdfs_i(inode);

	spin_lock(&hwb->inode_list_lock);
	if (list_empty(&info->wb_list)) {
		ihold(inode);
		list_add_tail(&info->wb_list, &hwb->inode_list_head);
		queue_delayed_work(hwb->dirty_inode_writeback_wq,
				   &hwb->dirty_inode_writeback_work, 0);
	}
	spin_unlock(&hwb->inode_list_lock);
}

static unsigned long hmdfs_idirty_pages(struct inode *inode, int tag)
{
	struct pagevec pvec;
	unsigned long nr_dirty_pages = 0;
	pgoff_t index = 0;

#if KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE
	pagevec_init(&pvec);
#else
	pagevec_init(&pvec, 0);
#endif
	while (pagevec_lookup_tag(&pvec, inode->i_mapping, &index, tag)) {
		nr_dirty_pages += pagevec_count(&pvec);
		pagevec_release(&pvec);
		cond_resched();
	}
	return nr_dirty_pages;
}

static inline unsigned long hmdfs_ratio_thresh(unsigned long ratio,
					       unsigned long thresh)
{
	unsigned long ret = (ratio * thresh) >> HMDFS_LIMIT_SHIFT;

	return (ret == 0) ? 1 : ret;
}

static inline unsigned long hmdfs_thresh_ratio(unsigned long base,
					       unsigned long thresh)
{
	unsigned long ratio = (base << HMDFS_LIMIT_SHIFT) / thresh;

	return (ratio == 0) ? 1 : ratio;
}

void hmdfs_calculate_dirty_thresh(struct hmdfs_writeback *hwb)
{
	hwb->dirty_fs_thresh = DIV_ROUND_UP(hwb->dirty_fs_bytes, PAGE_SIZE);
	hwb->dirty_file_thresh = DIV_ROUND_UP(hwb->dirty_file_bytes, PAGE_SIZE);
	hwb->dirty_fs_bg_thresh =
		DIV_ROUND_UP(hwb->dirty_fs_bg_bytes, PAGE_SIZE);
	hwb->dirty_file_bg_thresh =
		DIV_ROUND_UP(hwb->dirty_file_bg_bytes, PAGE_SIZE);

	hwb->fs_bg_ratio = hmdfs_thresh_ratio(hwb->dirty_fs_bg_thresh,
					      hwb->dirty_fs_thresh);
	hwb->file_bg_ratio = hmdfs_thresh_ratio(hwb->dirty_file_bg_thresh,
						hwb->dirty_file_thresh);
	hwb->fs_file_ratio = hmdfs_thresh_ratio(hwb->dirty_file_thresh,
						hwb->dirty_fs_thresh);
}

static void hmdfs_init_dirty_limit(struct hmdfs_dirty_throttle_control *hdtc)
{
	struct hmdfs_writeback *hwb = hdtc->hwb;

	hdtc->fs_thresh = hdtc->hwb->dirty_fs_thresh;
	hdtc->file_thresh = hdtc->hwb->dirty_file_thresh;
	hdtc->fs_bg_thresh = hdtc->hwb->dirty_fs_bg_thresh;
	hdtc->file_bg_thresh = hdtc->hwb->dirty_file_bg_thresh;

	if (!hwb->dirty_auto_threshold)
		return;

	/*
	 * Init thresh according the previous bandwidth adjusted thresh,
	 * thresh should be no more than setting thresh.
	 */
	if (hwb->bw_fs_thresh < hdtc->fs_thresh) {
		hdtc->fs_thresh = hwb->bw_fs_thresh;
		hdtc->fs_bg_thresh = hmdfs_ratio_thresh(hwb->fs_bg_ratio,
							hdtc->fs_thresh);
	}
	if (hwb->bw_file_thresh < hdtc->file_thresh) {
		hdtc->file_thresh = hwb->bw_file_thresh;
		hdtc->file_bg_thresh = hmdfs_ratio_thresh(hwb->file_bg_ratio,
							  hdtc->file_thresh);
	}
	/*
	 * The thresh should be updated in the first time of dirty pages
	 * exceed the freerun ceiling.
	 */
	hdtc->thresh_time_stamp = jiffies - HMDFS_BANDWIDTH_INTERVAL - 1;
}

static void hmdfs_update_dirty_limit(struct hmdfs_dirty_throttle_control *hdtc)
{
	struct hmdfs_writeback *hwb = hdtc->hwb;
	struct bdi_writeback *wb = hwb->wb;
	unsigned int time_limit = hwb->writeback_timelimit;
	unsigned long bw = wb->avg_write_bandwidth;
	unsigned long thresh;

	if (!hwb->dirty_auto_threshold)
		return;

	spin_lock(&hwb->write_bandwidth_lock);
	if (bw > hwb->max_write_bandwidth)
		hwb->max_write_bandwidth = bw;

	if (bw < hwb->min_write_bandwidth)
		hwb->min_write_bandwidth = bw;
	hwb->avg_write_bandwidth = bw;
	spin_unlock(&hwb->write_bandwidth_lock);

	/*
	 * If the bandwidth is lower than the lower limit, it may propably
	 * offline, there is meaningless to set such a lower thresh.
	 */
	bw = max(bw, hwb->bw_thresh_lowerlimit);
	thresh = bw * time_limit / roundup_pow_of_two(HZ);
	if (thresh >= hwb->dirty_fs_thresh) {
		hdtc->fs_thresh = hwb->dirty_fs_thresh;
		hdtc->file_thresh = hwb->dirty_file_thresh;
		hdtc->fs_bg_thresh = hwb->dirty_fs_bg_thresh;
		hdtc->file_bg_thresh = hwb->dirty_file_bg_thresh;
	} else {
		/* Adjust thresh according to current bandwidth */
		hdtc->fs_thresh = thresh;
		hdtc->fs_bg_thresh = hmdfs_ratio_thresh(hwb->fs_bg_ratio,
							hdtc->fs_thresh);
		hdtc->file_thresh = hmdfs_ratio_thresh(hwb->fs_file_ratio,
						       hdtc->fs_thresh);
		hdtc->file_bg_thresh = hmdfs_ratio_thresh(hwb->file_bg_ratio,
							  hdtc->file_thresh);
	}
	/* Save bandwidth adjusted thresh */
	hwb->bw_fs_thresh = hdtc->fs_thresh;
	hwb->bw_file_thresh = hdtc->file_thresh;
	/* Update time stamp */
	hdtc->thresh_time_stamp = jiffies;
}

void hmdfs_update_ratelimit(struct hmdfs_writeback *hwb)
{
	struct hmdfs_dirty_throttle_control hdtc = {.hwb = hwb};

	hmdfs_init_dirty_limit(&hdtc);

	/* hdtc.file_bg_thresh should be the lowest thresh */
	hwb->ratelimit_pages = hdtc.file_bg_thresh /
			       (num_online_cpus() * HMDFS_RATELIMIT_PAGES_GAP);
	if (hwb->ratelimit_pages < HMDFS_MIN_RATELIMIT_PAGES)
		hwb->ratelimit_pages = HMDFS_MIN_RATELIMIT_PAGES;
}

/* This is a copy of wb_max_pause() */
static unsigned long hmdfs_wb_pause(struct bdi_writeback *wb,
					unsigned long wb_dirty)
{
	unsigned long bw = wb->avg_write_bandwidth;
	unsigned long t;

	/*
	 * Limit pause time for small memory systems. If sleeping for too long
	 * time, a small pool of dirty/writeback pages may go empty and disk go
	 * idle.
	 *
	 * 8 serves as the safety ratio.
	 */
	t = wb_dirty / (1 + bw / roundup_pow_of_two(1 + HZ / 8));
	t++;

	return min_t(unsigned long, t, HMDFS_MAX_PAUSE);
}

static unsigned long
hmdfs_dirty_freerun_ceiling(struct hmdfs_dirty_throttle_control *hdtc,
			    unsigned int type)
{
	if (type == HMDFS_DIRTY_FS)
		return (hdtc->fs_thresh + hdtc->fs_bg_thresh) / 2;
	else /* HMDFS_DIRTY_FILE_TYPE */
		return (hdtc->file_thresh + hdtc->file_bg_thresh) / 2;
}

/* This is a copy of dirty_poll_interval() */
static inline unsigned long hmdfs_dirty_intv(unsigned long dirty,
					     unsigned long thresh)
{
	if (thresh > dirty)
		return 1UL << (ilog2(thresh - dirty) >> 1);
	return 1;
}

static void hmdfs_balance_dirty_pages(struct address_space *mapping)
{
	struct inode *inode = mapping->host;
	struct super_block *sb = inode->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	struct hmdfs_writeback *hwb = sbi->h_wb;
	struct bdi_writeback *wb = &inode_to_bdi(inode)->wb;
	struct hmdfs_dirty_throttle_control hdtc = {.hwb = hwb};
	unsigned int dirty_exceeded = 0;
	unsigned long start_time = jiffies;
	unsigned long pause = 0;

	/* Add delay work to trigger timeout writeback */
	if (hwb->dirty_writeback_interval != 0)
		hmdfs_writeback_inodes_sb_delayed(
			sb, hwb->dirty_writeback_interval * 10);

	hmdfs_init_dirty_limit(&hdtc);

	while (1) {
		unsigned long exceed = 0;
		unsigned long diff;

		/* Per-filesystem overbalance writeback */
		hdtc.fs_nr_dirty = wb_stat_sum(wb, WB_RECLAIMABLE);
		hdtc.fs_nr_reclaimable =
			hdtc.fs_nr_dirty + wb_stat_sum(wb, WB_WRITEBACK);
		if (hdtc.fs_nr_reclaimable < hdtc.file_bg_thresh) {
			diff = hmdfs_dirty_intv(hdtc.fs_nr_reclaimable,
						hdtc.file_thresh);
			goto free_running;
		}

		/* Per-file overbalance writeback */
		hdtc.file_nr_dirty =
			hmdfs_idirty_pages(inode, PAGECACHE_TAG_DIRTY);
		hdtc.file_nr_reclaimable =
			hmdfs_idirty_pages(inode, PAGECACHE_TAG_WRITEBACK) +
			hdtc.file_nr_dirty;
		if ((hdtc.fs_nr_reclaimable <
		     hmdfs_dirty_freerun_ceiling(&hdtc, HMDFS_DIRTY_FS)) &&
		    (hdtc.file_nr_reclaimable <
		     hmdfs_dirty_freerun_ceiling(&hdtc, HMDFS_DIRTY_FILE))) {
			unsigned long fs_intv, file_intv;

			fs_intv = hmdfs_dirty_intv(hdtc.fs_nr_reclaimable,
						   hdtc.fs_thresh);
			file_intv = hmdfs_dirty_intv(hdtc.file_nr_reclaimable,
						     hdtc.file_thresh);
			diff = min(fs_intv, file_intv);
free_running:
			current->nr_dirtied_pause = diff;
			current->nr_dirtied = 0;
			break;
		}

		if (hdtc.fs_nr_reclaimable >=
		    hmdfs_dirty_freerun_ceiling(&hdtc, HMDFS_DIRTY_FS)) {
			if (unlikely(!writeback_in_progress(wb)))
				hmdfs_writeback_inodes_sb(sb);
		} else {
			hmdfs_writeback_inode(sb, inode);
		}

		/*
		 * If dirty_auto_threshold is enabled, recalculate writeback
		 * thresh according to current bandwidth. Update bandwidth
		 * could be better if possible, but wb_update_bandwidth() is
		 * not exported, so we cannot update bandwidth here, so the
		 * bandwidth' update will be delayed if writing a lot to a
		 * single file.
		 */
		if (hwb->dirty_auto_threshold &&
		    time_is_before_jiffies(hdtc.thresh_time_stamp +
					   HMDFS_BANDWIDTH_INTERVAL))
			hmdfs_update_dirty_limit(&hdtc);

		if (unlikely(hdtc.fs_nr_reclaimable >= hdtc.fs_thresh))
			exceed |= HMDFS_FS_EXCEED;
		if (unlikely(hdtc.file_nr_reclaimable >= hdtc.file_thresh))
			exceed |= HMDFS_FILE_EXCEED;

		if (!exceed) {
			trace_hmdfs_balance_dirty_pages(sbi, wb, &hdtc,
							0UL, start_time);
			current->nr_dirtied = 0;
			break;
		}
		/*
		 * Per-file or per-fs reclaimable pages exceed throttle limit,
		 * sleep pause time and check again.
		 */
		dirty_exceeded |= exceed;
		if (dirty_exceeded && !hwb->dirty_exceeded)
			hwb->dirty_exceeded = true;

		/* Pause */
		pause = hmdfs_wb_pause(wb, hdtc.fs_nr_reclaimable);

		trace_hmdfs_balance_dirty_pages(sbi, wb, &hdtc, pause,
						start_time);

		__set_current_state(TASK_KILLABLE);
		io_schedule_timeout(pause);

		if (fatal_signal_pending(current))
			break;
	}

	if (!dirty_exceeded && hwb->dirty_exceeded)
		hwb->dirty_exceeded = false;

	if (hdtc.fs_nr_reclaimable >= hdtc.fs_bg_thresh) {
		if (unlikely(!writeback_in_progress(wb)))
			hmdfs_writeback_inodes_sb(sb);
	} else if (hdtc.file_nr_reclaimable >= hdtc.file_bg_thresh) {
		hmdfs_writeback_inode(sb, inode);
	}
}

void hmdfs_balance_dirty_pages_ratelimited(struct address_space *mapping)
{
	struct hmdfs_sb_info *sbi = mapping->host->i_sb->s_fs_info;
	struct hmdfs_writeback *hwb = sbi->h_wb;
	int *bdp_ratelimits = NULL;
	int ratelimit;

	if (!hwb->dirty_writeback_control)
		return;

	/* Add delay work to trigger timeout writeback */
	if (hwb->dirty_writeback_interval != 0)
		hmdfs_writeback_inodes_sb_delayed(
			mapping->host->i_sb,
			hwb->dirty_writeback_interval * 10);

	ratelimit = current->nr_dirtied_pause;
	if (hwb->dirty_exceeded)
		ratelimit = min(ratelimit, HMDFS_DIRTY_EXCEED_RATELIMIT);

	/*
	 * This prevents one CPU to accumulate too many dirtied pages
	 * without calling into hmdfs_balance_dirty_pages(), which can
	 * happen when there are 1000+ tasks, all of them start dirtying
	 * pages at exactly the same time, hence all honoured too large
	 * initial task->nr_dirtied_pause.
	 */
	preempt_disable();
	bdp_ratelimits = this_cpu_ptr(hwb->bdp_ratelimits);

	trace_hmdfs_balance_dirty_pages_ratelimited(sbi, hwb, *bdp_ratelimits);

	if (unlikely(current->nr_dirtied >= ratelimit)) {
		*bdp_ratelimits = 0;
	} else if (unlikely(*bdp_ratelimits >= hwb->ratelimit_pages)) {
		*bdp_ratelimits = 0;
		ratelimit = 0;
	}
	preempt_enable();

	if (unlikely(current->nr_dirtied >= ratelimit))
		hmdfs_balance_dirty_pages(mapping);
}

void hmdfs_destroy_writeback(struct hmdfs_sb_info *sbi)
{
	if (!sbi->h_wb)
		return;

	flush_delayed_work(&sbi->h_wb->dirty_sb_writeback_work);
	flush_delayed_work(&sbi->h_wb->dirty_inode_writeback_work);
	destroy_workqueue(sbi->h_wb->dirty_sb_writeback_wq);
	destroy_workqueue(sbi->h_wb->dirty_inode_writeback_wq);
	free_percpu(sbi->h_wb->bdp_ratelimits);
	kfree(sbi->h_wb);
	sbi->h_wb = NULL;
}

int hmdfs_init_writeback(struct hmdfs_sb_info *sbi)
{
	struct hmdfs_writeback *hwb;
	char name[HMDFS_WQ_NAME_LEN];
	int ret = -ENOMEM;

	hwb = kzalloc(sizeof(struct hmdfs_writeback), GFP_KERNEL);
	if (!hwb)
		return ret;

	hwb->sbi = sbi;
	hwb->wb = &sbi->sb->s_bdi->wb;
	hwb->dirty_writeback_control = true;
	hwb->dirty_writeback_interval = HM_DEFAULT_WRITEBACK_INTERVAL;
	hwb->dirty_file_bg_bytes = HMDFS_FILE_BG_WB_BYTES;
	hwb->dirty_fs_bg_bytes = HMDFS_FS_BG_WB_BYTES;
	hwb->dirty_file_bytes = HMDFS_FILE_WB_BYTES;
	hwb->dirty_fs_bytes = HMDFS_FS_WB_BYTES;
	hmdfs_calculate_dirty_thresh(hwb);
	hwb->bw_file_thresh = hwb->dirty_file_thresh;
	hwb->bw_fs_thresh = hwb->dirty_fs_thresh;
	spin_lock_init(&hwb->inode_list_lock);
	INIT_LIST_HEAD(&hwb->inode_list_head);
	hwb->dirty_exceeded = false;
	hwb->ratelimit_pages = HMDFS_DEF_RATELIMIT_PAGES;
	hwb->dirty_auto_threshold = true;
	hwb->writeback_timelimit = HMDFS_DEF_WB_TIMELIMIT;
	hwb->bw_thresh_lowerlimit = HMDFS_BW_THRESH_DEF_LIMIT;
	spin_lock_init(&hwb->write_bandwidth_lock);
	hwb->avg_write_bandwidth = 0;
	hwb->max_write_bandwidth = 0;
	hwb->min_write_bandwidth = ULONG_MAX;
	hwb->bdp_ratelimits = alloc_percpu(int);
	if (!hwb->bdp_ratelimits)
		goto free_hwb;

	snprintf(name, sizeof(name), "dfs_ino_wb%u", sbi->seq);
	hwb->dirty_inode_writeback_wq = create_singlethread_workqueue(name);
	if (!hwb->dirty_inode_writeback_wq) {
		hmdfs_err("Failed to create inode writeback workqueue!");
		goto free_bdp;
	}
	snprintf(name, sizeof(name), "dfs_sb_wb%u", sbi->seq);
	hwb->dirty_sb_writeback_wq = create_singlethread_workqueue(name);
	if (!hwb->dirty_sb_writeback_wq) {
		hmdfs_err("Failed to create filesystem writeback workqueue!");
		goto free_i_wq;
	}
	INIT_DELAYED_WORK(&hwb->dirty_sb_writeback_work,
			  hmdfs_writeback_inodes_sb_handler);
	INIT_DELAYED_WORK(&hwb->dirty_inode_writeback_work,
			  hmdfs_writeback_inode_handler);
	sbi->h_wb = hwb;
	return 0;
free_i_wq:
	destroy_workqueue(hwb->dirty_inode_writeback_wq);
free_bdp:
	free_percpu(sbi->h_wb->bdp_ratelimits);
free_hwb:
	kfree(hwb);
	return ret;
}
