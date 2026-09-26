// SPDX-License-Identifier: GPL-2.0
/*
 * fs/f2fs/gc.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 */
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/backing-dev.h>
#include <linux/init.h>
#include <linux/hmfs_fs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/timer.h>
#include <linux/blkdev.h>
#include <linux/sched/signal.h>
#include <linux/mas_bkops_core.h>
#include <linux/radix-tree.h>

#include "hmfs.h"
#include "node.h"
#include "segment.h"
#include "gc.h"
#include <trace/events/hmfs.h>

static struct kmem_cache *victim_entry_slab;
static struct kmem_cache *hmfs_inode_gc_entry_slab;

#define GC_THREAD_LOOP_SCHEDULE 1000
#define GC_THREAD_LOOP_PRINT_TIME 10000
#define IDLE_WT 50
#define IDLE_WT_WITH_MANUAL_GC 1000
#define MANUAL_GC_START_WT 50
#define MIN_WT 1000
#define DEF_CP_LONG_TIME_IN_GC   10000   /* milliseconds*/
#define MS_TO_NS   1000000
#define GC_URGENT_DISABLE_BLKS	       (32<<18)        /* 32G */
#define GC_URGENT_SPACE			       (10<<18)        /* 10G */
#define GC_IOLIMIT_STRONG 8
#define GC_IOLIMIT_WEAK 4
#define GC_SKIP_ROUND 200
#define FREE_SEC_UNVERIFY 2

#define DM_MAX_BLKS 768		/* max blocks moved per datamove */
#define DM_REPEAT_BLKS 512	/* align 2M per datamove if the size needed longer than DM_MAX_BLKS */
#define DM_RETRY_TIMES 5	/* max retry times when datamove repeat mode fails */

#define MIN_SEGS 768	/* 1.5G space for normal use, unit:segment */
#define MIN_SECS 4	/* min sections alloceted in meantime for node and data streams */
#define EXTRA_SEGS_IN_OVP 2048	/* extra between op and level3, unit: segment */
#define CONCURRENT_SECS 3	/* max concurrent write sections */

extern int hmfs_find_next_free_extent(const unsigned long *addr,
			unsigned long size, unsigned long *offset);
static unsigned int count_bits(const unsigned long *addr,
			unsigned int offset, unsigned int len);

/*
 * GC tuning ratio [0, 100] in performance mode
 */
static inline int gc_perf_ratio(struct f2fs_sb_info *sbi)
{
	block_t reclaimable_user_blocks = sbi->user_block_count -
						written_block_count(sbi);
	return reclaimable_user_blocks == 0 ? 100 :
			100ULL * free_user_blocks(sbi) / reclaimable_user_blocks;
}

static inline int __gc_thread_wait_timeout(struct f2fs_sb_info *sbi, struct hmfs_gc_kthread *gc_th,
       int timeout)
{
	wait_queue_head_t *wq = &gc_th->gc_wait_queue_head;
	wait_queue_head_t *fg_gc_wait = &gc_th->fg_gc_wait;

	return wait_event_interruptible_timeout(*wq,
		gc_th->gc_wake || freezing(current) ||
		kthread_should_stop() || waitqueue_active(fg_gc_wait) || atomic_read(&sbi->need_ssr_gc),
		timeout);
}

/* if invaild blocks in dirty segments is less 10% of total free space */
static inline int is_reclaimable_dirty_blocks_enough(struct f2fs_sb_info *sbi)
{
	s64 total_free_blocks;
	s64 total_dirty_blocks;
	total_free_blocks = sbi->user_block_count - written_block_count(sbi);
	total_dirty_blocks = total_free_blocks - free_user_blocks(sbi);

	if (total_dirty_blocks <= 0)
		return 0;
	do_div(total_free_blocks, 10);
	if (total_dirty_blocks <= total_free_blocks)
		return 0;
	return 1;
}

#ifdef CONFIG_HMFS_STAT_FS
static void hmfs_update_gc_stat(struct f2fs_sb_info *sbi, int level)
{
	struct gc_stat *sec_stat = &(sbi->gc_stat);

	if (has_not_enough_free_secs(sbi, 0, 0))
		level = FG_GC_LEVEL;

	if (level < 0 || level >= ALL_GC_LEVELS)
		return;

	(sec_stat->times)[level]++;
}
#endif

block_t hmfs_get_free_block_count(struct f2fs_sb_info *sbi)
{
	block_t  free_block_count;

	free_block_count = sbi->user_block_count - valid_user_blocks(sbi) -
						sbi->current_reserved_blocks;

	free_block_count += overprovision_segments(sbi) * sbi->blocks_per_seg;

	if (unlikely(free_block_count <= sbi->unusable_block_count))
		free_block_count = 0;
	else
		free_block_count -= sbi->unusable_block_count;

	if (free_block_count > F2FS_OPTION(sbi).root_reserved_blocks)
		free_block_count -= F2FS_OPTION(sbi).root_reserved_blocks;
	else
		free_block_count = 0;

	return free_block_count;
}

static inline void hmfs_set_gc_control_info(struct f2fs_sb_info *sbi,
						int iolimit, bool is_greedy, bool enable_idle)
{
	sbi->gc_control_info.iolimit = 0;
	sbi->gc_control_info.is_greedy = is_greedy;
	sbi->gc_control_info.idle_enabled = enable_idle;
}

static unsigned int hmfs_get_gc_policy(struct f2fs_sb_info *sbi, int *level_ret)
{
	unsigned int wait_ms;
	unsigned int free_secs = free_sections(sbi);
	/* # of free blocks in main area, include op area */
	block_t free_block_count = hmfs_get_free_block_count(sbi);
	int *level = sbi->gc_stat.level;

	if (free_secs > level[BG_GC_LEVEL1]) {
		wait_ms = MIN_WT * 120;
		hmfs_set_gc_control_info(sbi, 0, false, true);
		*level_ret = BG_GC_LEVEL1;
	} else if (free_secs > level[BG_GC_LEVEL2]) {
		if (hmfs_disk_is_frag(sbi, free_block_count, free_secs)) {
			wait_ms = MIN_WT * 5;
			hmfs_set_gc_control_info(sbi, 0, false, true);
		} else {
			wait_ms = MIN_WT * 60;
			hmfs_set_gc_control_info(sbi, 0, false, true);
		}
		*level_ret = BG_GC_LEVEL2;
	} else if (free_secs > level[BG_GC_LEVEL3]) {
		if (hmfs_disk_is_frag(sbi, free_block_count, free_secs)) {
			wait_ms = MIN_WT;
			hmfs_set_gc_control_info(sbi,
				GC_IOLIMIT_WEAK, true, false);
		} else {
			wait_ms = MIN_WT * 30;
			hmfs_set_gc_control_info(sbi, 0, false, true);
		}
		*level_ret = BG_GC_LEVEL3;
	} else if (free_secs > level[BG_GC_LEVEL4]) {
		if (hmfs_disk_is_frag(sbi, free_block_count, free_secs)) {
			wait_ms = 0;
			hmfs_set_gc_control_info(sbi,
				GC_IOLIMIT_STRONG, true, false);
		} else {
			wait_ms = MIN_WT * 5;
			hmfs_set_gc_control_info(sbi,
				GC_IOLIMIT_STRONG, true, true);
		}
		*level_ret = BG_GC_LEVEL4;
	} else if (free_secs > level[BG_GC_LEVEL5]) {
		wait_ms = 0;
		hmfs_set_gc_control_info(sbi, 0, true, true);
		*level_ret = BG_GC_LEVEL5;
	} else {
		wait_ms = 0;
		hmfs_set_gc_control_info(sbi, 0, true, false);
		*level_ret = BG_GC_LEVEL6;
	}

	return  wait_ms;
}

static int gc_thread_func(void *data)
{
	struct f2fs_sb_info *sbi = data;
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	unsigned int wait_ms;
	bool idle = false;
	int level = 0;

	current->flags |= PF_MUTEX_GC;

	set_freezable();
	do {
		int ret;

		wait_ms = hmfs_get_gc_policy(sbi, &level);
		if (!idle && wait_ms == 0)
			wait_ms = 10;

		/*lint -save -e574 -e666 */
		if (time_to_inject(sbi, FAULT_CHECKPOINT)) {
			f2fs_show_injection_info(FAULT_CHECKPOINT);
			hmfs_stop_checkpoint(sbi, false);
		}

		if (!sb_start_write_trylock(sbi->sb)) {
			if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
				stat_other_skip_bggc_count(sbi);
			continue;
		}

		/*lint -save -e454 -e456 -e666*/
		ret = __gc_thread_wait_timeout(sbi, gc_th,
					msecs_to_jiffies(wait_ms));
		if (gc_th->gc_wake)
			gc_th->gc_wake = 0;

		if (!ret) {
			if (sbi->sb->s_writers.frozen >= SB_FREEZE_WRITE) {
				if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
					stat_other_skip_bggc_count(sbi);
				goto next;
			}

			if (!mutex_trylock(&sbi->gc_mutex)) {
				if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
					stat_other_skip_bggc_count(sbi);
				idle = false;
				goto next;

			}
		} else if (try_to_freeze()) {
			if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
				stat_other_skip_bggc_count(sbi);
			goto next;
		} else if (kthread_should_stop()) {
			sb_end_write(sbi->sb);
			break;
		} else if (ret < 0) {
			pr_err("hmfs-gc: some signals have been received...\n");
			goto next;
		} else {
			int ssr_gc_count;
			ssr_gc_count = atomic_read(&sbi->need_ssr_gc);
			if (ssr_gc_count) {
				mutex_lock(&sbi->gc_mutex);
				hmfs_gc(sbi, true, false, NULL_SEGNO);
				atomic_sub(ssr_gc_count, &sbi->need_ssr_gc);
			}
			if (!has_not_enough_free_secs(sbi, 0, 0)) {
				wake_up_all(&gc_th->fg_gc_wait);
				goto next;
			}

			/* run into FG_GC
			   we must wait & take sbi->gc_mutex before FG_GC */
			mutex_lock(&sbi->gc_mutex);

			trace_hmfs_gc_policy(sbi, wait_ms, free_sections(sbi), 1);
			hmfs_gc(sbi, false, false, NULL_SEGNO);
			wake_up_all(&gc_th->fg_gc_wait);
			level = FG_GC_LEVEL;
			goto next;
		}
		idle = true;

		/*
		 * [GC triggering condition]
		 * 0. GC is not conducted currently.
		 * 1. There are enough dirty segments.
		 * 2. IO subsystem is idle by checking the # of writeback pages.
		 * 3. IO subsystem is idle by checking the # of requests in
		 *    bdev's request list.
		 *
		 * Note) We have to avoid triggering GCs frequently.
		 * Because it is possible that some segments can be
		 * invalidated soon after by user update or deletion.
		 * So, I'd like to wait some time to collect dirty segments.
		 */
		if (sbi->gc_mode == GC_URGENT && !is_gc_test_set(sbi, GC_TEST_DISABLE_GC_URGENT)) {
			goto do_gc;
		}

#ifdef CONFIG_MAS_BLK
		if (!gc_th->block_idle &&
		    !is_gc_test_set(sbi, GC_TEST_DISABLE_IO_AWARE) &&
		    sbi->gc_control_info.idle_enabled) {
#else
		if (!is_idle(sbi, GC_TIME) &&
		    !is_gc_test_set(sbi, GC_TEST_DISABLE_IO_AWARE) &&
		    sbi->gc_control_info.idle_enabled) {
#endif
			if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
				stat_io_skip_bggc_count(sbi);
			mutex_unlock(&sbi->gc_mutex);
			stat_io_skip_bggc_count(sbi);
			idle = false;
			goto next;
		}

do_gc:
		stat_inc_bggc_count(sbi);

#ifdef CONFIG_HMFS_STAT_FS
	{
		static DEFINE_RATELIMIT_STATE(bg_gc_rs, F2FS_GC_DSM_INTERVAL, F2FS_GC_DSM_BURST);
		unsigned long long total_size, free_size;

		total_size = blks_to_mb(sbi->user_block_count, sbi->blocksize);
		free_size = blks_to_mb(sbi->user_block_count -
				valid_user_blocks(sbi),
				sbi->blocksize);

		if (unlikely(__ratelimit(&bg_gc_rs))) {
			hmfs_msg(sbi->sb, KERN_NOTICE,
				"BG_GC: Size=%lluMB,Free=%lluMB,count=%d,free_sec=%u,reserved_sec=%u,node_secs=%d,dent_secs=%d\n",
				total_size, free_size,
				sbi->bg_gc, free_sections(sbi), reserved_sections(sbi),
				get_blocktype_secs(sbi, F2FS_DIRTY_NODES), get_blocktype_secs(sbi, F2FS_DIRTY_DENTS));
		}
	}
#endif
		trace_hmfs_gc_policy(sbi, wait_ms, free_sections(sbi), 0);
		hmfs_gc(sbi, test_opt(sbi, FORCE_FG_GC), true, NULL_SEGNO);

		trace_hmfs_background_gc(sbi->sb, wait_ms,
				prefree_segments(sbi), free_segments(sbi));

		/* balancing f2fs's metadata periodically */
		hmfs_balance_fs_bg(sbi);
next:
#ifdef CONFIG_HMFS_STAT_FS
		hmfs_update_gc_stat(sbi, level);
#endif
		sb_end_write(sbi->sb);

		/*
		 * Cpu need to be release if getting no victim repeats over 1000 times.
		 * or it may be hungtasked.
		 */
		if (sbi->gc_control_info.loop_cnt >
				GC_THREAD_LOOP_SCHEDULE) {
			msleep(MIN_WT);
			sbi->gc_control_info.loop_cnt = 0;
		}

		/*
		 * In case firmware has no time to do gc moving data for
		 * SLC cache to TLC while we are doing fs-gc, sleep 1s
		 * per 200 times to give firmware the chance to do
		 * device-gc. It can be removed if all data is being gced
		 * by datamove.
		 */
		if (sbi->gc_control_info.gc_skip_count > GC_SKIP_ROUND) {
			sbi->gc_control_info.gc_skip_count = 0;
			msleep(MIN_WT);
		}

		/*lint -restore*/
	} while (!kthread_should_stop());
	return 0;
}

#ifdef CONFIG_MAS_BLK
static void bkops_start_func(struct work_struct *work)
{
	int bkops_ret;
	struct f2fs_sb_info *sbi = container_of((struct delayed_work *)work,
		struct f2fs_sb_info, start_bkops_work);
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;

	/* stop fs-gc and do manual device-gc if needed */
	del_timer_sync(&gc_th->nb_timer);
	gc_th->block_idle = false;

	bkops_ret = mas_bkops_work_query(sbi->sb->s_bdev);

	/*
	 * BKOPS_NEED_START: need to start manual gc and wait 1s
	 * BKOPS_ALREADY_START: already started manual gc, so wait 1s
	 * others: need to stop manual gc if blk_busy notify
	 */
	if (bkops_ret == BKOPS_NEED_START || bkops_ret == BKOPS_ALREADY_START) {
		if (bkops_ret == BKOPS_NEED_START)
			mas_bkops_work_start(sbi->sb->s_bdev);
		mod_timer(&gc_th->nb_timer, jiffies +
				msecs_to_jiffies(IDLE_WT_WITH_MANUAL_GC));
	} else {
		if (test_and_clear_bit(SBI_BKOPS_WORK_START, &sbi->s_flag))
			mas_bkops_work_stop(sbi->sb->s_bdev);
		gc_th->block_idle = true;
	}
}

static void set_block_idle(unsigned long data)
{
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)data;
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;

	mas_bkops_work_stop(sbi->sb->s_bdev);
	gc_th->block_idle = true;
}

static enum blk_busyidle_callback_ret gc_io_busy_idle_notify_handler(struct blk_busyidle_event_node *event_node,
										       enum blk_idle_notify_state state)
{
	enum blk_busyidle_callback_ret ret = BUSYIDLE_NO_IO;
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)event_node->param_data;
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;

	if (gc_th->hmfs_gc_task == NULL)
		return ret;
	switch(state) {
	case BLK_IDLE_NOTIFY:
		if (test_bit(SBI_BKOPS_WORK_START, &sbi->s_flag))
			break;
		set_bit(SBI_BKOPS_WORK_START, &sbi->s_flag);
		queue_delayed_work(sbi->need_bkops_wq, &sbi->start_bkops_work,
			msecs_to_jiffies(MANUAL_GC_START_WT));
		break;
	case BLK_BUSY_NOTIFY:
		if (test_and_clear_bit(SBI_BKOPS_WORK_START, &sbi->s_flag)) {
			cancel_delayed_work_sync(&sbi->start_bkops_work);
			mas_bkops_work_stop(sbi->sb->s_bdev);
		}
		break;
	case BLK_FG_IDLE_NOTIFY:
		if (test_bit(SBI_BKOPS_WORK_START, &sbi->s_flag))
			break;
		mod_timer(&gc_th->nb_timer,
			jiffies + msecs_to_jiffies(IDLE_WT));
		break;
	case BLK_FG_BUSY_NOTIFY:
		del_timer_sync(&gc_th->nb_timer);
		gc_th->block_idle = false;
		break;
	default:
		break;
	}

	return ret;
}
#endif

int hmfs_start_gc_thread(struct f2fs_sb_info *sbi)
{
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	struct task_struct *hmfs_gc_task;
	dev_t dev = sbi->sb->s_bdev->bd_dev;
	int err = 0;

	ACCESS_ONCE(gc_th->hmfs_gc_task) = NULL;

	gc_th->urgent_sleep_time = DEF_GC_THREAD_URGENT_SLEEP_TIME;
	gc_th->min_sleep_time = DEF_GC_THREAD_MIN_SLEEP_TIME;
	gc_th->max_sleep_time = DEF_GC_THREAD_MAX_SLEEP_TIME;
	gc_th->no_gc_sleep_time = DEF_GC_THREAD_NOGC_SLEEP_TIME;

	gc_th->gc_wake= 0;
	gc_th->gc_preference = GC_BALANCE;

#ifdef CONFIG_MAS_BLK
	setup_timer(&gc_th->nb_timer, set_block_idle, (unsigned long)sbi);
	gc_th->block_idle = false;
	strncpy(gc_th->gc_event_node.subscriber, "hmfs_gc", SUBSCRIBER_NAME_LEN);
	gc_th->gc_event_node.busyidle_callback = gc_io_busy_idle_notify_handler;
	gc_th->gc_event_node.param_data = sbi;
	err = blk_busyidle_event_subscribe(sbi->sb->s_bdev, &gc_th->gc_event_node);
#endif

	gc_th->root = RB_ROOT;
	INIT_LIST_HEAD(&gc_th->victim_list);
	gc_th->victim_count = 0;

	gc_th->age_threshold = DEF_GC_THREAD_AGE_THRESHOLD;
	gc_th->dirty_rate_threshold = DEF_GC_THREAD_DIRTY_RATE_THRESHOLD;
	gc_th->dirty_count_threshold = DEF_GC_THREAD_DIRTY_COUNT_THRESHOLD;
	gc_th->age_weight = DEF_GC_THREAD_AGE_WEIGHT;

	init_waitqueue_head(&gc_th->gc_wait_queue_head);
	init_waitqueue_head(&gc_th->fg_gc_wait);
	hmfs_gc_task = kthread_run(gc_thread_func, sbi,
		"hmfs_gc-%u:%u", MAJOR(dev), MINOR(dev));
	if (IS_ERR(hmfs_gc_task))
		err = PTR_ERR(hmfs_gc_task);
	else
		ACCESS_ONCE(gc_th->hmfs_gc_task) = hmfs_gc_task;
	return err;
}

void hmfs_stop_gc_thread(struct f2fs_sb_info *sbi)
{
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	if (gc_th->hmfs_gc_task != NULL) {
		kthread_stop(gc_th->hmfs_gc_task);
		ACCESS_ONCE(gc_th->hmfs_gc_task) = NULL;
		wake_up_all(&gc_th->fg_gc_wait);
#ifdef CONFIG_MAS_BLK
retry:
		if (blk_busyidle_event_unsubscribe(&gc_th->gc_event_node))
			goto retry;

		del_timer_sync(&gc_th->nb_timer);
#endif
	}
}


static int select_gc_type(struct f2fs_sb_info *sbi, int gc_type)
{
	int gc_mode = gc_type == BG_GC? GC_AT:GC_GREEDY;

	switch (sbi->gc_mode) {
	case GC_IDLE_CB:
		gc_mode = GC_CB;
		break;
	case GC_IDLE_GREEDY:
	case GC_URGENT:
		gc_mode = GC_GREEDY;
		break;
	case GC_IDLE_AT:
		gc_mode = GC_AT;
		break;
	}
	return gc_mode;
}

static void select_policy(struct f2fs_sb_info *sbi, int gc_type,
			int type, struct victim_sel_policy *p)
{
	struct dirty_seglist_info *dirty_i = DIRTY_I(sbi);
	int dirty_type = type;

	if (p->alloc_mode == SSR) {
		p->gc_mode = GC_GREEDY;
		p->dirty_bitmap = dirty_i->dirty_segmap[dirty_type];
		p->max_search = dirty_i->nr_dirty[dirty_type];
		p->ofs_unit = 1;
	} else if (p->alloc_mode == ASSR) {
		p->gc_mode = GC_GREEDY;
		p->dirty_bitmap = dirty_i->dirty_segmap[dirty_type];
		p->max_search = dirty_i->nr_dirty[dirty_type];
		p->ofs_unit = 1;
	} else {
		p->gc_mode = select_gc_type(sbi, gc_type);
		p->ofs_unit = sbi->segs_per_sec;
		if (IS_MULTI_SEGS_IN_SEC(sbi)) {
			p->dirty_bitmap = dirty_i->dirty_secmap;
			p->max_search = count_bits(p->dirty_bitmap,
					0, MAIN_SECS(sbi));
		} else {
			p->dirty_bitmap = dirty_i->dirty_segmap[DIRTY];
			p->max_search = dirty_i->nr_dirty[DIRTY];
		}
	}

	/* we need to check every dirty segments in the FG_GC case */
	if (gc_type != FG_GC &&
			p->gc_mode != GC_AT &&
			p->alloc_mode != ASSR &&
			(sbi->gc_mode != GC_URGENT) &&
			p->max_search > sbi->max_victim_search)
		p->max_search = sbi->max_victim_search;

	/* let's select beginning hot/small space first in no_heap mode*/
	if (test_opt(sbi, NOHEAP) &&
		(type == CURSEG_HOT_DATA || IS_NODESEG(type)))
		p->offset = 0;
	else
		p->offset = SIT_I(sbi)->last_victim[p->gc_mode];
}

static unsigned int get_max_cost(struct f2fs_sb_info *sbi,
				struct victim_sel_policy *p)
{
	/* SSR allocates in a segment unit */
	if (p->alloc_mode == SSR)
		return sbi->blocks_per_seg;
	else if (p->alloc_mode == ASSR)
		return UINT_MAX;

	/* LFS */
	if (p->gc_mode == GC_GREEDY)
		return 2 * sbi->blocks_per_seg * p->ofs_unit;
	else if (p->gc_mode == GC_CB)
		return UINT_MAX;
	else if (p->gc_mode == GC_AT)
		return UINT_MAX;
	else /* No other gc_mode */
		return 0;
}

static unsigned int check_bg_victims(struct f2fs_sb_info *sbi)
{
	struct dirty_seglist_info *dirty_i = DIRTY_I(sbi);
	unsigned int secno;

	/*
	 * If the gc_type is FG_GC, we can select victim segments
	 * selected by background GC before.
	 * Those segments guarantee they have small valid blocks.
	 */
	for_each_set_bit(secno, dirty_i->victim_secmap, MAIN_SECS(sbi)) {
		if (sec_usage_check(sbi, secno))
			continue;
		if (sbi->gc_loop.segmap &&
			test_bit(secno * sbi->segs_per_sec, sbi->gc_loop.segmap))
			continue;

		clear_bit(secno, dirty_i->victim_secmap);
		return GET_SEG_FROM_SEC(sbi, secno);
	}
	return NULL_SEGNO;
}

/*
 * The section mtime is the mtime of last valid segmemt
 * in one section.
 */
static unsigned long long get_section_mtime(
		struct f2fs_sb_info *sbi, unsigned int segno)
{
	unsigned int start_segno;
	unsigned int end_segno;
	unsigned int valid_blocks;

	start_segno = (segno / sbi->segs_per_sec) * sbi->segs_per_sec;

	end_segno = start_segno + sbi->segs_per_sec - 1;

	while (end_segno > start_segno) {
		valid_blocks = get_valid_blocks(sbi, end_segno, false);
		if (valid_blocks > 0)
			break;

		end_segno--;
	}

	return get_seg_entry(sbi, end_segno)->mtime;
}

static unsigned int get_cb_cost(struct f2fs_sb_info *sbi, unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	unsigned long long mtime = 0;
	unsigned int vblocks;
	unsigned int max_age;
	unsigned char age = 0;
	unsigned char u;

	vblocks = get_valid_blocks(sbi, segno, true);
	vblocks = div_u64(vblocks, sbi->segs_per_sec);
	mtime = get_section_mtime(sbi, segno);

	u = (vblocks * 100) >> sbi->log_blocks_per_seg;

	/* Handle if the system time has changed by the user */
	if (mtime < sit_i->min_mtime)
		sit_i->min_mtime = mtime;
	if (mtime > sit_i->max_mtime)
		sit_i->max_mtime = mtime;
	/*lint -save -e613 -e666 */
	/* Reduce the cost weight of age when free blocks less than 10% */
	max_age = (gc_th && gc_th->gc_preference != GC_LIFETIME &&
		gc_perf_ratio(sbi) < 10) ? max(10 * gc_perf_ratio(sbi), 1) : 100;
	/*lint -restore*/
	if (sit_i->max_mtime != sit_i->min_mtime)
		age = max_age - div64_u64(max_age * (mtime - sit_i->min_mtime),
				sit_i->max_mtime - sit_i->min_mtime);

	return UINT_MAX - ((100 * (100 - u) * age) / (100 + u));
}

static inline unsigned int get_gc_cost(struct f2fs_sb_info *sbi,
			unsigned int segno, struct victim_sel_policy *p)
{
	if (p->alloc_mode == SSR)
		return get_seg_entry(sbi, segno)->ckpt_valid_blocks;

	/* alloc_mode == LFS */
	if (p->gc_mode == GC_GREEDY)
		return get_valid_blocks(sbi, segno, true);
	else if (p->gc_mode == GC_CB)
		return get_cb_cost(sbi, segno);
	else
		f2fs_bug_on(sbi, 1);
	return get_cb_cost(sbi, segno); /*lint !e527*/
}

static unsigned int count_bits(const unsigned long *addr,
				unsigned int offset, unsigned int len)
{
	unsigned int end = offset + len, sum = 0;

	while (offset < end) {
		if (test_bit(offset++, addr))
			++sum;
	}
	return sum;
}

static struct victim_entry *attach_victim_entry(struct f2fs_sb_info *sbi,
				unsigned long long mtime, unsigned int segno,
				struct rb_node *parent, struct rb_node **p)
{
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	struct victim_entry *ve;

	ve =  f2fs_kmem_cache_alloc(victim_entry_slab, GFP_NOFS);

	ve->mtime = mtime;
	ve->segno = segno;

	rb_link_node(&ve->rb_node, parent, p);
	rb_insert_color(&ve->rb_node, &gc_th->root);

	list_add_tail(&ve->list, &gc_th->victim_list);

	gc_th->victim_count++;

	return ve;
}

static void insert_victim_entry(struct f2fs_sb_info *sbi,
				unsigned long long mtime, unsigned int segno)
{
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	struct rb_node **p;
	struct rb_node *parent = NULL;
	struct victim_entry *ve = NULL;

	p = __hmfs_lookup_rb_tree_ext(sbi, &gc_th->root, &parent, mtime);
	ve = attach_victim_entry(sbi, mtime, segno, parent, p);
}

static void record_victim_entry(struct f2fs_sb_info *sbi,
			struct victim_sel_policy *p, unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned long long mtime = 0;

	mtime = get_section_mtime(sbi, segno);

	/* Handle if the system time has changed by the user */
	if (mtime < sit_i->dirty_min_mtime)
		sit_i->dirty_min_mtime = mtime;
	if (mtime > sit_i->dirty_max_mtime)
		sit_i->dirty_max_mtime = mtime;
	if (mtime < sit_i->min_mtime)
		sit_i->min_mtime = mtime;
	if (mtime > sit_i->max_mtime)
		sit_i->max_mtime = mtime;
	/* don't choose young section as candidate */
	if (sit_i->dirty_max_mtime - mtime < p->age_threshold)
		return;

	insert_victim_entry(sbi, mtime, segno);
}

static struct rb_node *lookup_central_victim(struct f2fs_sb_info *sbi,
						struct victim_sel_policy *p)
{
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	struct rb_node *parent = NULL;

	__hmfs_lookup_rb_tree_ext(sbi, &gc_th->root, &parent, p->age);

	return parent;
}

static void lookup_victim_atgc(struct f2fs_sb_info *sbi,
						struct victim_sel_policy *p)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	struct rb_root *root = &gc_th->root;
	struct rb_node *node;
	struct rb_entry *re;
	struct victim_entry *ve;
	unsigned long long total_time;
	unsigned long long age, u, accu;
	unsigned long long max_mtime = sit_i->dirty_max_mtime;
	unsigned long long min_mtime = sit_i->dirty_min_mtime;
	unsigned int sec_blocks = sbi->segs_per_sec * sbi->blocks_per_seg;
	unsigned int vblocks;
	unsigned int dirty_threshold = max(gc_th->dirty_count_threshold,
					gc_th->dirty_rate_threshold *
					gc_th->victim_count / 100);
	unsigned int age_weight = gc_th->age_weight;
	unsigned int cost;
	unsigned int iter = 0;

	if (max_mtime < min_mtime)
		return;

	max_mtime += 1;
	total_time = max_mtime - min_mtime;
	accu = min_t(unsigned long long,
			ULLONG_MAX / total_time / 100,
			DEFAULT_ACCURACY_CLASS);

	node = rb_first(root);
next:
	re = rb_entry_safe(node, struct rb_entry, rb_node);
	if (!re)
		return;

	ve = (struct victim_entry *)re;

	if (ve->mtime >= max_mtime || ve->mtime < min_mtime)
		goto skip;

	/* age = 10000 * x% * 60 */
	age = div64_u64(accu * (max_mtime - ve->mtime), total_time) *
								age_weight;

	vblocks = get_valid_blocks(sbi, ve->segno, true);
	if (!vblocks || vblocks == sec_blocks) {
		hmfs_msg(sbi->sb, KERN_ERR,
				"%s: segno[%u] vblocks[%u] gc[%u][%u]",
				__func__, ve->segno, vblocks,
				p->gc_mode, gc_th->victim_count);
		f2fs_bug_on(sbi, 1);
	}

	/* u = 10000 * x% * 40 */
	u = div64_u64(accu * (sec_blocks - vblocks), sec_blocks) *
							(100 - age_weight);

	f2fs_bug_on(sbi, age + u >= UINT_MAX);

	cost = UINT_MAX - (age + u);
	iter++;

	if (cost < p->min_cost ||
			(cost == p->min_cost && age > p->oldest_age)) {
		p->min_cost = cost;
		p->oldest_age = age;
		p->min_segno = ve->segno;
	}
skip:
	if (iter < dirty_threshold) {
		node = rb_next(node);
		goto next;
	}
}

/*
 * select candidates around source section in range of
 * [target - dirty_threshold, target + dirty_threshold]
 */
static void lookup_victim_assr(struct f2fs_sb_info *sbi,
						struct victim_sel_policy *p)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	struct rb_node *node;
	struct rb_entry *re;
	struct victim_entry *ve;
	unsigned long long total_time;
	unsigned long long age;
	unsigned long long max_mtime = sit_i->dirty_max_mtime;
	unsigned long long min_mtime = sit_i->dirty_min_mtime;
	unsigned int seg_blocks = sbi->blocks_per_seg;
	unsigned int vblocks;
	unsigned int dirty_threshold = max(gc_th->dirty_count_threshold,
					gc_th->dirty_rate_threshold *
					gc_th->victim_count / 100);
	unsigned int cost;
	unsigned int iter = 0;
	int stage = 0;

	if (max_mtime < min_mtime)
		return;

	max_mtime += 1;
	total_time = max_mtime - min_mtime;
next_stage:
	node = lookup_central_victim(sbi, p);
next_node:
	re = rb_entry_safe(node, struct rb_entry, rb_node);
	if (!re) {
		if (stage == 0)
			goto skip_stage;
		return;
	}

	ve = (struct victim_entry *)re;

	if (ve->mtime >= max_mtime || ve->mtime < min_mtime)
		goto skip_node;

	age = max_mtime - ve->mtime;

	vblocks = get_seg_entry(sbi, ve->segno)->ckpt_valid_blocks;
	f2fs_bug_on(sbi, !vblocks);

	/* rare case */
	if (vblocks == seg_blocks)
		goto skip_node;

	iter++;

	age = max_mtime - abs(p->age - age);
	cost = UINT_MAX - vblocks;

	if (cost < p->min_cost ||
			(cost == p->min_cost && age > p->oldest_age)) {
		p->min_cost = cost;
		p->oldest_age = age;
		p->min_segno = ve->segno;
	}
skip_node:
	if (iter < dirty_threshold) {
		if (stage == 0)
			node = rb_prev(node);
		else if (stage == 1)
			node = rb_next(node);
		goto next_node;
	}
skip_stage:
	if (stage < 1) {
		stage++;
		iter = 0;
		goto next_stage;
	}
}

bool hmfs_check_rb_tree_consistence(struct f2fs_sb_info *sbi,
						struct rb_root *root)
{
#ifdef CONFIG_HMFS_CHECK_FS
	struct rb_node *cur = rb_first(root), *next;
	struct rb_entry *cur_re, *next_re;

	if (!cur)
		return true;

	while (cur) {
		next = rb_next(cur);
		if (!next)
			return true;

		cur_re = rb_entry(cur, struct rb_entry, rb_node);
		next_re = rb_entry(next, struct rb_entry, rb_node);

		if (cur_re->key > next_re->key) {
			hmfs_msg(sbi->sb, KERN_ERR, "inconsistent rbtree, "
				"cur(%llu) next(%llu)",
				cur_re->key,
				next_re->key);
			return false;
		}

		cur = next;
	}
#endif
	return true;
}

static void lookup_victim_by_time(struct f2fs_sb_info *sbi,
						struct victim_sel_policy *p)
{
	bool consistent = hmfs_check_rb_tree_consistence(sbi, &sbi->gc_thread.root);

	WARN_ON(!consistent);

	if (p->gc_mode == GC_AT)
		lookup_victim_atgc(sbi, p);
	else if (p->alloc_mode == ASSR)
		lookup_victim_assr(sbi, p);
	else
		f2fs_bug_on(sbi, 1);
}

void hmfs_release_victim_entry(struct f2fs_sb_info *sbi)
{
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	struct victim_entry *ve, *tmp;

	list_for_each_entry_safe(ve, tmp, &gc_th->victim_list, list) {
		list_del(&ve->list);
		kmem_cache_free(victim_entry_slab, ve);
		gc_th->victim_count--;
	}

	gc_th->root = RB_ROOT;

	f2fs_bug_on(sbi, gc_th->victim_count);
	f2fs_bug_on(sbi, !list_empty(&gc_th->victim_list));
}


/*
 * This function is called from two paths.
 * One is garbage collection and the other is SSR segment selection.
 * When it is called during GC, it just gets a victim segment
 * and it does not remove it from dirty seglist.
 * When it is called from SSR segment selection, it finds a segment
 * which has minimum valid blocks and removes it from dirty seglist.
 */
static int get_victim_by_default(struct f2fs_sb_info *sbi,
			unsigned int *result, int gc_type, int type,
			char alloc_mode, unsigned long long age)
{
	struct dirty_seglist_info *dirty_i = DIRTY_I(sbi);
	struct sit_info *sm = SIT_I(sbi);
	struct victim_sel_policy p;
	unsigned int secno, last_victim;
	unsigned int last_segment;
	unsigned int nsearched;
	bool is_atgc = false;
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;
	struct task_struct *hmfs_gc_task = gc_th->hmfs_gc_task;

	mutex_lock(&dirty_i->seglist_lock);
	last_segment = MAIN_SECS(sbi) * sbi->segs_per_sec;

	p.alloc_mode = alloc_mode;
	p.age = age;
	if (hmfs_gc_task)
		p.age_threshold = sbi->gc_thread.age_threshold;

retry:
	select_policy(sbi, gc_type, type, &p);
	p.min_segno = NULL_SEGNO;
	p.oldest_age = 0;
	p.min_cost = get_max_cost(sbi, &p);
	if (hmfs_gc_task)
		is_atgc = (p.gc_mode == GC_AT || p.alloc_mode == ASSR);
	else
		is_atgc = false;
	nsearched = 0;

	if (is_atgc)
		SIT_I(sbi)->dirty_min_mtime = ULLONG_MAX;

	if (*result != NULL_SEGNO) {
		if (IS_DATASEG(get_seg_entry(sbi, *result)->type) &&
			get_valid_blocks(sbi, *result, false) &&
			!sec_usage_check(sbi, GET_SEC_FROM_SEG(sbi, *result)))
			p.min_segno = *result;
		goto out;
	}

	if (p.max_search == 0)
		goto out;

	last_victim = sm->last_victim[p.gc_mode];
	if (p.alloc_mode == LFS && gc_type == FG_GC) {
		p.min_segno = check_bg_victims(sbi);
		if (p.min_segno != NULL_SEGNO)
			goto got_it;
	}


	while (1) {
		unsigned long cost, *dirty_bitmap;
		unsigned int unit_no, segno;

		dirty_bitmap = p.dirty_bitmap;
		unit_no = find_next_bit(dirty_bitmap,
				last_segment / p.ofs_unit,
				p.offset / p.ofs_unit);
		segno = unit_no * p.ofs_unit;
		if (test_bit(unit_no, dirty_bitmap) == 0) {
			hmfs_msg(sbi->sb, KERN_NOTICE,
					"victim segno %u, offset %u, last %u, bit[%u]",
					segno, p.offset, last_segment,
					test_bit(unit_no, dirty_bitmap));
		}

		if (segno >= last_segment) {
			if (sm->last_victim[p.gc_mode]) {
				last_segment =
					sm->last_victim[p.gc_mode];
				sm->last_victim[p.gc_mode] = 0;
				p.offset = 0;
				continue;
			}
			break;
		}

		p.offset = segno + p.ofs_unit;
		nsearched++;

		secno = GET_SEC_FROM_SEG(sbi, segno);

		if (sec_usage_check(sbi, secno))
			goto next;
		/* Don't touch checkpointed data */
		if (unlikely(is_sbi_flag_set(sbi, SBI_CP_DISABLED) &&
					get_ckpt_valid_blocks(sbi, segno)))
			goto next;
		if (gc_type == BG_GC && test_bit(secno, dirty_i->victim_secmap))
			goto next;
		if (gc_type == FG_GC && p.alloc_mode == LFS &&
		    sbi->gc_loop.segmap && test_bit((segno / p.ofs_unit) *
		    p.ofs_unit, sbi->gc_loop.segmap))
			goto next;

		if (is_atgc) {
			record_victim_entry(sbi, &p, segno);
			goto next;
		}

		cost = get_gc_cost(sbi, segno, &p);

		if (p.min_cost > cost) {
			p.min_segno = segno;
			p.min_cost = cost;
		}
next:
		if (nsearched >= p.max_search) {

			if (!sm->last_victim[p.gc_mode] && segno <= last_victim)
				sm->last_victim[p.gc_mode] =
					last_victim + p.ofs_unit;
			else
				sm->last_victim[p.gc_mode] = segno + p.ofs_unit;
			sm->last_victim[p.gc_mode] %=
				(MAIN_SECS(sbi) * sbi->segs_per_sec);
			break;
		}
	}

	/* get victim for GC_AT/ASSR */
	if (is_atgc) {
		lookup_victim_by_time(sbi, &p);
		hmfs_release_victim_entry(sbi);
	}

	if (is_atgc && p.min_segno == NULL_SEGNO &&
		sm->dirty_max_mtime - sm->dirty_min_mtime < p.age_threshold) {
		/* set temp age threshold to get some victims */
		p.age_threshold = 0;
		goto retry;
	}
	if (p.min_segno != NULL_SEGNO) {
got_it:
		if (p.alloc_mode == LFS) {
			secno = GET_SEC_FROM_SEG(sbi, p.min_segno);
			if (gc_type == FG_GC)
				sbi->cur_victim_sec = secno;
			else
				set_bit(secno, dirty_i->victim_secmap);
		}
		*result = (p.min_segno / p.ofs_unit) * p.ofs_unit;

		trace_hmfs_get_victim(sbi->sb, type, gc_type, &p,
				sbi->cur_victim_sec,
				prefree_segments(sbi), free_segments(sbi));
	}
out:
	mutex_unlock(&dirty_i->seglist_lock);

	return (p.min_segno == NULL_SEGNO) ? 0 : 1;
}

static const struct victim_selection default_v_ops = {
	.get_victim = get_victim_by_default,
};

static void add_gc_offset(struct inode_entry *ie, int offset)
{
	struct inode_gc_entry *ge;

	ge = f2fs_kmem_cache_alloc(hmfs_inode_gc_entry_slab, GFP_NOFS);
	ge->offset = offset;
	list_add_tail(&ge->list, &ie->offset_list);
}

static struct inode *find_gc_inode(struct gc_inode_list *gc_list, nid_t ino,
			struct inode_entry **ret_ie)
{
	struct inode_entry *ie;

	ie = radix_tree_lookup(&gc_list->iroot, ino);
	if (ie) {
		*ret_ie = ie;
		return ie->inode;
	}
	return NULL;
}

static void add_gc_block(struct gc_inode_list *gc_list, struct inode *inode,
					int offset)
{
	struct inode_entry *new_ie, *ie;

	if (inode == find_gc_inode(gc_list, inode->i_ino, &ie)) {
		add_gc_offset(ie, offset);
		iput(inode);
		return;
	}
	new_ie = f2fs_kmem_cache_alloc(hmfs_inode_entry_slab, GFP_NOFS);
	new_ie->inode = inode;
	INIT_LIST_HEAD(&new_ie->offset_list);
	add_gc_offset(new_ie, offset);

	f2fs_radix_tree_insert(&gc_list->iroot, inode->i_ino, new_ie);
	list_add_tail(&new_ie->list, &gc_list->ilist);
}

static void put_gc_inode(struct gc_inode_list *gc_list)
{
	struct inode_entry *ie, *next_ie;
	list_for_each_entry_safe(ie, next_ie, &gc_list->ilist, list) {
		radix_tree_delete(&gc_list->iroot, ie->inode->i_ino);
		iput(ie->inode);
		list_del(&ie->list);
		kmem_cache_free(hmfs_inode_entry_slab, ie);
	}
}

static void del_gc_offset(struct f2fs_sb_info *sbi,
				struct gc_inode_list *gc_list)
{
	struct inode_entry *ie, *next_ie;
	struct inode_gc_entry *ge, *next_ge;
	struct inode *inode;

	list_for_each_entry_safe(ie, next_ie, &gc_list->ilist, list) {
		inode = ie->inode;
		if (!inode) {
			hmfs_gc_loop_debug(sbi);
			continue;
		}

		list_for_each_entry_safe(ge, next_ge, &ie->offset_list, list) {
			list_del(&ge->list);
			kmem_cache_free(hmfs_inode_gc_entry_slab, ge);
		}
	}
}

static int check_valid_map(struct f2fs_sb_info *sbi,
				unsigned int segno, int offset)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct seg_entry *sentry;
	int ret;

	down_read(&sit_i->sentry_lock);
	sentry = get_seg_entry(sbi, segno);
	ret = f2fs_test_bit(offset, sentry->cur_valid_map);
	up_read(&sit_i->sentry_lock);
	return ret;
}

/*
 * This function compares node address got in summary with that in NAT.
 * On validity, copy that node with cold status, otherwise (invalid node)
 * ignore that.
 */
static int gc_node_segment(struct f2fs_sb_info *sbi,
		struct f2fs_summary *sum, unsigned int segno, int gc_type)
{
	struct f2fs_summary *entry;
	block_t start_addr;
	int off;
	int phase = 0, gc_cnt = 0;
	bool fggc = (gc_type == FG_GC);
	int submitted = 0;

	start_addr = START_BLOCK(sbi, segno);

next_step:
	entry = sum;

	if (fggc && phase == 2)
		atomic_inc(&sbi->wb_sync_req[NODE]);

	for (off = 0; off < sbi->blocks_per_seg; off++, entry++) {
		nid_t nid = le32_to_cpu(entry->nid);
		struct page *node_page;
		struct node_info ni;
		int err;

		/* stop BG_GC if there is not enough free sections. */
		if (gc_type == BG_GC && has_not_enough_free_secs(sbi, 0, 0)) {
			bd_mutex_lock(&sbi->bd_mutex);
			inc_bd_array_val(sbi, gc_node_blk_cnt, gc_type, gc_cnt);
			bd_mutex_unlock(&sbi->bd_mutex);
			return submitted;
		}
		if (check_valid_map(sbi, segno, off) == 0)
			continue;

		if (phase == 0) {
			hmfs_ra_meta_pages(sbi, NAT_BLOCK_OFFSET(nid), 1,
							META_NAT, true);
			continue;
		}

		if (phase == 1) {
			hmfs_ra_node_page(sbi, nid);
			continue;
		}

		/* phase == 2 */
		node_page = hmfs_get_node_page(sbi, nid);
		if (IS_ERR(node_page)) {
			hmfs_gc_loop_debug(sbi);
			continue;
		}

		/* block may become invalid during hmfs_get_node_page */
		if (check_valid_map(sbi, segno, off) == 0) {
			f2fs_put_page(node_page, 1);
			continue;
		}

		if (f2fs_get_node_info(sbi, nid, &ni)) {
			f2fs_put_page(node_page, 1);
			continue;
		}

		if (ni.blk_addr != start_addr + off) {
			f2fs_put_page(node_page, 1);
			hmfs_gc_loop_debug(sbi);
			continue;
		}

		err = hmfs_move_node_page(node_page, gc_type);
		if (!err && gc_type == FG_GC)
			submitted++;
		if (!err)
			gc_cnt++;
		stat_inc_node_blk_count(sbi, 1, gc_type);
		sbi->blocks_gc_written++;
	}

	if (++phase < 3)
		goto next_step;

	bd_mutex_lock(&sbi->bd_mutex);
	inc_bd_array_val(sbi, gc_node_blk_cnt, gc_type, gc_cnt);
	bd_mutex_unlock(&sbi->bd_mutex);
	if (fggc)
		atomic_dec(&sbi->wb_sync_req[NODE]);
	return submitted;
}

/*
 * Calculate start block index indicating the given node offset.
 * Be careful, caller should give this node offset only indicating direct node
 * blocks. If any node offsets, which point the other types of node blocks such
 * as indirect or double indirect node blocks, are given, it must be a caller's
 * bug.
 */
block_t hmfs_start_bidx_of_node(unsigned int node_ofs, struct inode *inode)
{
	unsigned int indirect_blks = 2 * NIDS_PER_BLOCK + 4;
	unsigned int bidx;

	if (node_ofs == 0)
		return 0;

	if (node_ofs <= 2) {
		bidx = node_ofs - 1;
	} else if (node_ofs <= indirect_blks) {
		int dec = (node_ofs - 4) / (NIDS_PER_BLOCK + 1);
		bidx = node_ofs - 2 - dec;
	} else {
		int dec = (node_ofs - indirect_blks - 3) / (NIDS_PER_BLOCK + 1);
		bidx = node_ofs - 5 - dec;
	}
	return bidx * ADDRS_PER_BLOCK + ADDRS_PER_INODE(inode);
}

static bool is_alive(struct f2fs_sb_info *sbi, struct f2fs_summary *sum,
		struct node_info *dni, block_t blkaddr, unsigned int *nofs)
{
	struct page *node_page;
	nid_t nid;
	unsigned int ofs_in_node;
	block_t source_blkaddr;

	nid = le32_to_cpu(sum->nid);
	ofs_in_node = le16_to_cpu(sum->ofs_in_node);

	node_page = hmfs_get_node_page(sbi, nid);
	if (IS_ERR(node_page))
		return false;

	if (f2fs_get_node_info(sbi, nid, dni)) {
		f2fs_put_page(node_page, 1);
		return false;
	}

	if (sum->version != dni->version) {
		hmfs_msg(sbi->sb, KERN_WARNING,
				"%s: valid data with mismatched node version.",
				__func__);
		set_sbi_flag(sbi, SBI_NEED_FSCK);
		hmfs_set_need_fsck_report();
	}

	*nofs = ofs_of_node(node_page);
	source_blkaddr = datablock_addr(NULL, node_page, ofs_in_node);
	f2fs_put_page(node_page, 1);

	if (source_blkaddr != blkaddr)
		return false;
	return true;
}

static int ra_data_block(struct inode *inode, pgoff_t index)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct address_space *mapping = inode->i_mapping;
	struct dnode_of_data dn;
	struct page *page;
	struct extent_info ei = {0, 0, 0};
	struct f2fs_io_info fio = {
		.sbi = sbi,
		.ino = inode->i_ino,
		.type = DATA,
		.temp = COLD,
		.op = REQ_OP_READ,
		.op_flags = 0,
		.encrypted_page = NULL,
		.in_list = false,
		.retry = false,
	};
	int err;

	page = f2fs_grab_cache_page(mapping, index, true);
	if (!page)
		return -ENOMEM;

	if (hmfs_lookup_extent_cache(inode, index, &ei)) {
		dn.data_blkaddr = ei.blk + index - ei.fofs;
		goto got_it;
	}

	set_new_dnode(&dn, inode, NULL, NULL, 0);
	err = hmfs_get_dnode_of_data(&dn, index, LOOKUP_NODE);
	if (err)
		goto put_page;
	f2fs_put_dnode(&dn);

	if (unlikely(!hmfs_is_valid_blkaddr(sbi, dn.data_blkaddr,
						DATA_GENERIC))) {
		err = -EFAULT;
		goto put_page;
	}
got_it:
	/* read page */
	fio.page = page;
	fio.new_blkaddr = fio.old_blkaddr = dn.data_blkaddr;

	/*
	 * don't cache encrypted data into meta inode until previous dirty
	 * data were writebacked to avoid racing between GC and flush.
	 */
	hmfs_wait_on_page_writeback(page, DATA, true);

	hmfs_wait_on_block_writeback(inode, dn.data_blkaddr);

	fio.encrypted_page = f2fs_pagecache_get_page(META_MAPPING(sbi),
					dn.data_blkaddr,
					FGP_LOCK | FGP_CREAT, GFP_NOFS);
	if (!fio.encrypted_page) {
		err = -ENOMEM;
		goto put_page;
	}

	err = hmfs_submit_page_bio(&fio);
	if (err)
		goto put_encrypted_page;
	f2fs_put_page(fio.encrypted_page, 0);
	f2fs_put_page(page, 1);
	return 0;
put_encrypted_page:
	f2fs_put_page(fio.encrypted_page, 1);
put_page:
	f2fs_put_page(page, 1);
	return err;
}

/*
 * Move data block via META_MAPPING while keeping locked data page.
 * This can be used to move blocks, aka LBAs, directly on disk.
 */
static int move_data_block(struct inode *inode, block_t bidx,
				int gc_type, unsigned int segno, int off)
{
	struct f2fs_io_info fio = {
		.sbi = F2FS_I_SB(inode),
		.ino = inode->i_ino,
		.type = DATA,
		.temp = COLD,
		.op = REQ_OP_READ,
		.op_flags = 0,
		.encrypted_page = NULL,
		.in_list = false,
		.retry = false,
		.mem_control = 0,
	};
	struct dnode_of_data dn;
	struct f2fs_summary sum;
	struct node_info ni;
	struct page *page, *mpage;
	block_t newaddr = 0;
	int err = 0;
	bool lfs_mode = test_opt(fio.sbi, LFS);
	int type = fio.sbi->gc_thread.atgc_enabled ?
			CURSEG_FRAGMENT_DATA : CURSEG_COLD_DATA;

	/* do not read out */
	page = f2fs_grab_cache_page(inode->i_mapping, bidx, false);
	if (!page) {
		hmfs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		return -ENOMEM;
	}

	if (!check_valid_map(F2FS_I_SB(inode), segno, off)) {
		err = -ENOENT;
		goto out;
	}

	if (f2fs_is_atomic_file(inode)) {
		F2FS_I(inode)->i_gc_failures[GC_FAILURE_ATOMIC]++;
		F2FS_I_SB(inode)->skipped_atomic_files[gc_type]++;
		hmfs_gc_loop_debug(F2FS_I_SB(inode));
		err = -EAGAIN;
		goto out;
	}

	if (f2fs_is_pinned_file(inode)) {
		hmfs_pin_file_control(inode, true);
		err = -EAGAIN;
		goto out;
	}

	set_new_dnode(&dn, inode, NULL, NULL, 0);
	err = hmfs_get_dnode_of_data(&dn, bidx, LOOKUP_NODE);
	if (err) {
		hmfs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto out;
	}

	if (unlikely(dn.data_blkaddr == NULL_ADDR)) {
		ClearPageUptodate(page);
		hmfs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		err = -ENOENT;
		goto put_out;
	}

	/*
	 * don't cache encrypted data into meta inode until previous dirty
	 * data were writebacked to avoid racing between GC and flush.
	 */
	hmfs_wait_on_page_writeback(page, DATA, true);

	hmfs_wait_on_block_writeback(inode, dn.data_blkaddr);

	err = f2fs_get_node_info(fio.sbi, dn.nid, &ni);
	if (err)
		goto put_out;

	set_summary(&sum, dn.nid, dn.ofs_in_node, ni.version);

	/* read page */
	fio.page = page;
	fio.new_blkaddr = fio.old_blkaddr = dn.data_blkaddr;

	if (lfs_mode)
		down_write(&fio.sbi->io_order_lock);

	hmfs_allocate_data_block(fio.sbi, NULL, fio.old_blkaddr, &newaddr,
					&sum, type, NULL, false, SEQ_NONE);

	fio.encrypted_page = f2fs_pagecache_get_page(META_MAPPING(fio.sbi),
				newaddr, FGP_LOCK | FGP_CREAT, GFP_NOFS);
	if (!fio.encrypted_page) {
		err = -ENOMEM;
		hmfs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto recover_block;
	}

	mpage = f2fs_pagecache_get_page(META_MAPPING(fio.sbi),
					fio.old_blkaddr, FGP_LOCK, GFP_NOFS);
	if (mpage) {
		bool updated = false;

		if (PageUptodate(mpage)) {
			memcpy(page_address(fio.encrypted_page),
					page_address(mpage), PAGE_SIZE);
			updated = true;
		}
		f2fs_put_page(mpage, 1);
		invalidate_mapping_pages(META_MAPPING(fio.sbi),
					fio.old_blkaddr, fio.old_blkaddr);
		if (updated)
			goto write_page;
	}

	err = hmfs_submit_page_bio(&fio);
	if (err) {
		hmfs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_page_out;
	}

	/* write page */
	lock_page(fio.encrypted_page);

	if (unlikely(fio.encrypted_page->mapping != META_MAPPING(fio.sbi))) {
		err = -EIO;
		hmfs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_page_out;
	}
	if (unlikely(!PageUptodate(fio.encrypted_page))) {
		err = -EIO;
		hmfs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_page_out;
	}

write_page:
	hmfs_wait_on_page_writeback(fio.encrypted_page, DATA, true);
	set_page_dirty(fio.encrypted_page);
	if (clear_page_dirty_for_io(fio.encrypted_page))
		dec_page_count(fio.sbi, F2FS_DIRTY_META);

	set_page_writeback(fio.encrypted_page);
	ClearPageError(page);

	/* allocate block address */
	hmfs_wait_on_page_writeback(dn.node_page, NODE, true);

	fio.op = REQ_OP_WRITE;
	fio.op_flags = REQ_SYNC;
	fio.new_blkaddr = newaddr;
	hmfs_submit_page_write(&fio);
	if (fio.retry) {
		err = -EAGAIN;
		if (PageWriteback(fio.encrypted_page))
			end_page_writeback(fio.encrypted_page);
		goto put_page_out;
	}

	f2fs_update_iostat(fio.sbi, FS_GC_DATA_IO, F2FS_BLKSIZE);

	hmfs_update_data_blkaddr(&dn, newaddr);
	set_inode_flag(inode, FI_APPEND_WRITE);
	if (page->index == 0)
		set_inode_flag(inode, FI_FIRST_BLOCK_WRITTEN);
put_page_out:
	f2fs_put_page(fio.encrypted_page, 1);
recover_block:
	if (lfs_mode)
		up_write(&fio.sbi->io_order_lock);
	if (err)
		hmfs_do_replace_block(fio.sbi, &sum, newaddr, fio.old_blkaddr,
							true, true, false, true);
put_out:
	f2fs_put_dnode(&dn);
out:
	f2fs_put_page(page, 1);
	return err;
}

static int move_data_page(struct inode *inode, block_t bidx, int gc_type,
							unsigned int segno, int off)
{
	struct page *page;
	int err = 0;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	page = hmfs_get_lock_data_page(inode, bidx, true);
	if (IS_ERR(page)) {
		hmfs_gc_loop_debug(sbi);
		return PTR_ERR(page);
	}

	if (!check_valid_map(F2FS_I_SB(inode), segno, off)) {
		err = -ENOENT;
		goto out;
	}

	if (f2fs_is_atomic_file(inode)) {
		F2FS_I(inode)->i_gc_failures[GC_FAILURE_ATOMIC]++;
		F2FS_I_SB(inode)->skipped_atomic_files[gc_type]++;
		hmfs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		err = -EAGAIN;
		goto out;
	}
	if (f2fs_is_pinned_file(inode)) {
		if (gc_type == FG_GC)
			hmfs_pin_file_control(inode, true);
		err = -EAGAIN;
		goto out;
	}

	if (gc_type == BG_GC) {
		if (PageWriteback(page)) {
			hmfs_gc_loop_debug(sbi);
			err = -EAGAIN;
			goto out;
		}
		set_page_dirty(page);
		set_cold_data(page);
	} else {
		/*lint -save -e446*/
		struct f2fs_io_info fio = {
			.sbi = F2FS_I_SB(inode),
			.ino = inode->i_ino,
			.type = DATA,
			.temp = COLD,
			.op = REQ_OP_WRITE,
			.op_flags = REQ_SYNC,
			.old_blkaddr = NULL_ADDR,
			.page = page,
			.encrypted_page = NULL,
			.need_lock = LOCK_REQ,
			.io_type = FS_GC_DATA_IO,
		};
		bool is_dirty = PageDirty(page);
		unsigned long cnt = 0;

retry:
		hmfs_wait_on_page_writeback(page, DATA, true);

		set_page_dirty(page);
		if (clear_page_dirty_for_io(page)) {
			inode_dec_dirty_pages(inode);
			hmfs_remove_dirty_inode(inode);
		}

		set_cold_data(page);

		err = hmfs_do_write_data_page(&fio);
		if (err) {
			clear_cold_data(page);
			if (err == -ENOMEM) {
			       cnt++;
			       if (!(cnt % F2FS_GC_LOOP_NOMEM_MOD))
					hmfs_msg(sbi->sb, KERN_ERR,
					"hmfs_gc_loop nomem retry:%lu in %s:%d\n",
					cnt, __func__, __LINE__);
				congestion_wait(BLK_RW_ASYNC, HZ/50);
				goto retry;
			}
			if (is_dirty)
				set_page_dirty(page);
		}
	}
out:
	f2fs_put_page(page, 1);
	return err;
}

/*
 * Offload moving data to device, device can move data from origin TLC block
 * to new TLC block. Host do not need to reading data to page cache.
 */
static int move_data_by_datamove(struct inode *inode, block_t bidx,
				 int gc_type, unsigned int segno, int off)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct dnode_of_data dn;
	struct f2fs_summary sum;
	struct node_info ni;
	struct page *page;
	block_t newaddr = 0;
	int err = 0;

	page = f2fs_grab_cache_page(inode->i_mapping, bidx, false);
	if (!page) {
		hmfs_gc_loop_debug(sbi);
		return -ENOMEM;
	}

	/*
	 * in case the page has been written to other place or
	 * truncated before grabed.
	 */
	if (!check_valid_map(sbi, segno, off)) {
		err = -ENOENT;
		goto put_page;
	}

	if (f2fs_is_atomic_file(inode)) {
		F2FS_I(inode)->i_gc_failures[GC_FAILURE_ATOMIC]++;
		sbi->skipped_atomic_files[gc_type]++;
		hmfs_gc_loop_debug(sbi);
		err = -EAGAIN;
		goto put_page;
	}

	if (f2fs_is_pinned_file(inode)) {
		hmfs_pin_file_control(inode, true);
		err = -EAGAIN;
		goto put_page;
	}

	set_new_dnode(&dn, inode, NULL, NULL, 0);
	err = hmfs_get_dnode_of_data(&dn, bidx, LOOKUP_NODE);
	if (err) {
		hmfs_gc_loop_debug(sbi);
		goto put_page;
	}

	if (unlikely(dn.data_blkaddr == NULL_ADDR)) {
		ClearPageUptodate(page);
		hmfs_gc_loop_debug(sbi);
		err = -ENOENT;
		goto put_dnode;
	}

	/*
	 * If the page is on the writeback way, don't move data until
	 * it completed to avoid racing between datamove and normal
	 * write, otherwise obsolete data may be got in the later
	 * reading routine.
	 */
	hmfs_wait_on_page_writeback(page, DATA, true);

	err = f2fs_get_node_info(sbi, dn.nid, &ni);
	if (err)
		goto put_dnode;

	set_summary(&sum, dn.nid, dn.ofs_in_node, ni.version);
	hmfs_allocate_data_block(sbi, NULL, dn.data_blkaddr, &newaddr,
					&sum, CURSEG_DATA_MOVE1,
					NULL, false, SEQ_NONE);

	hmfs_datamove_add_entry(sbi, dn.data_blkaddr,
			newaddr, DATA, STREAM_DATA_MOVE1);

	hmfs_update_data_blkaddr(&dn, newaddr);
	f2fs_update_iostat(sbi, FS_GC_DATA_IO, F2FS_BLKSIZE);

put_dnode:
	f2fs_put_dnode(&dn);
put_page:
	f2fs_put_page(page, 1);
	return err;
}

static int move_data_by_file(struct f2fs_sb_info *sbi,
		struct f2fs_summary *sum, struct gc_inode_list *gc_list,
		unsigned int segno, int gc_type, bool datamove_gc, int *gc_cnt)
{
	struct inode *inode;
	struct inode_entry *ie, *next_ie;
	struct inode_gc_entry *ge, *next_ge;
	struct f2fs_summary *entry;
	block_t start_addr = START_BLOCK(sbi, segno);
	struct f2fs_inode_info *fi;
	int submitted = 0;

	list_for_each_entry_safe(ie, next_ie, &gc_list->ilist, list) {
		inode = ie->inode;
		if (!inode) {
			hmfs_gc_loop_debug(sbi);
			continue;
		}
		fi = F2FS_I(inode);
		list_for_each_entry_safe(ge, next_ge, &ie->offset_list, list) {
			struct node_info dni; /* dnode info for the data */
			unsigned int ofs_in_node, nofs;
			block_t start_bidx;
			bool locked = false;
			int err;

			f2fs_bug_on(sbi, ge->offset >= sbi->blocks_per_seg);
			entry = sum + ge->offset;

			/* stop BG_GC if there is not enough free sections. */
			if (gc_type == BG_GC && has_not_enough_free_secs(sbi, 0, 0)) {
				bd_mutex_lock(&sbi->bd_mutex);
				inc_bd_array_val(sbi, gc_data_blk_cnt, gc_type, *gc_cnt);
				inc_bd_array_val(sbi, hotcold_cnt, HC_GC_COLD_DATA, *gc_cnt);
				bd_mutex_unlock(&sbi->bd_mutex);
				return submitted;
			}

			if (check_valid_map(sbi, segno, ge->offset) == 0) {
				continue;
			}

			/* Get an inode by ino with checking validity */
			if (!is_alive(sbi, entry, &dni, start_addr + ge->offset, &nofs)) {
				hmfs_gc_loop_debug(sbi);
				continue;
			}

			ofs_in_node = le16_to_cpu(entry->ofs_in_node);

			if (S_ISREG(inode->i_mode)) {
				if (!down_write_trylock(&fi->i_gc_rwsem[READ])) {
					hmfs_gc_loop_debug(sbi);
					continue;
				}
				if (!down_write_trylock(
						&fi->i_gc_rwsem[WRITE])) {
					sbi->skipped_gc_rwsem++;
					up_write(&fi->i_gc_rwsem[READ]);
					hmfs_gc_loop_debug(sbi);
					continue;
				}
				locked = true;

				/* wait for all inflight aio data */
				inode_dio_wait(inode);
			}

			start_bidx = hmfs_start_bidx_of_node(nofs, inode)
								+ ofs_in_node;

			if (datamove_gc)
				err = move_data_by_datamove(inode, start_bidx,
							gc_type, segno, ge->offset);
			else if (f2fs_post_read_required(inode))
				err = move_data_block(inode, start_bidx,
							gc_type, segno, ge->offset);
			else
				err = move_data_page(inode, start_bidx, gc_type,
							segno, ge->offset);

			if (!err && (gc_type == FG_GC || datamove_gc ||
						f2fs_post_read_required(inode)))
				submitted++;

			if (locked) {
				up_write(&fi->i_gc_rwsem[WRITE]);
				up_write(&fi->i_gc_rwsem[READ]);
			}

			stat_inc_data_blk_count(sbi, 1, gc_type);
			sbi->blocks_gc_written++;
			if (!err)
				*gc_cnt += 1;

		}
	}
	return submitted;
}

/*
 * This function tries to get parent node of victim data block, and identifies
 * data block validity. If the block is valid, copy that with cold status and
 * modify parent node.
 * If the parent node is not valid or the data block address is different,
 * the victim data block is ignored.
 */
static int hmfs_gc_data_segment(struct f2fs_sb_info *sbi,
		struct f2fs_summary *sum, struct gc_inode_list *gc_list,
		unsigned int segno, int gc_type, bool datamove)
{
	struct super_block *sb = sbi->sb;
	struct f2fs_summary *entry;
	block_t start_addr;
	int off;
	int phase = 0, gc_cnt = 0;
	int submitted = 0;

	start_addr = START_BLOCK(sbi, segno);

next_step:
	entry = sum;

	for (off = 0; off < sbi->blocks_per_seg; off++, entry++) {
		struct page *data_page;
		struct inode *inode;
		struct node_info dni; /* dnode info for the data */
		unsigned int ofs_in_node, nofs;
		block_t start_bidx;
		nid_t nid = le32_to_cpu(entry->nid);

		/* stop BG_GC if there is not enough free sections. */
		if (gc_type == BG_GC && has_not_enough_free_secs(sbi, 0, 0)) {
			bd_mutex_lock(&sbi->bd_mutex);
			inc_bd_array_val(sbi, gc_data_blk_cnt, gc_type, gc_cnt);
			inc_bd_array_val(sbi, hotcold_cnt, HC_GC_COLD_DATA, gc_cnt);
			bd_mutex_unlock(&sbi->bd_mutex);
			return submitted;
		}

		if (check_valid_map(sbi, segno, off) == 0)
			continue;

		if (phase == 0) {
			hmfs_ra_meta_pages(sbi, NAT_BLOCK_OFFSET(nid), 1,
							META_NAT, true);
			continue;
		}

		if (phase == 1) {
			hmfs_ra_node_page(sbi, nid);
			continue;
		}

		/* Get an inode by ino with checking validity */
		if (!is_alive(sbi, entry, &dni, start_addr + off, &nofs)) {
			hmfs_gc_loop_debug(sbi);
			continue;
		}

		if (phase == 2) {
			hmfs_ra_node_page(sbi, dni.ino);
			continue;
		}

		ofs_in_node = le16_to_cpu(entry->ofs_in_node);

		if (phase == 3) {
			inode = hmfs_iget(sb, dni.ino);
			if (IS_ERR(inode) || is_bad_inode(inode)) {
				hmfs_gc_loop_debug(sbi);
				continue;
			}

			/* no need to do readahead for datamove */
			if (datamove) {
				add_gc_block(gc_list, inode, off);
				continue;
			}

			if (!down_write_trylock(
				&F2FS_I(inode)->i_gc_rwsem[WRITE])) {
				iput(inode);
				sbi->skipped_gc_rwsem++;
				continue;
			}

			start_bidx = hmfs_start_bidx_of_node(nofs, inode) +
								ofs_in_node;

			if (f2fs_post_read_required(inode)) {
				int err = ra_data_block(inode, start_bidx);

				up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
				if (err) {
					iput(inode);
					hmfs_gc_loop_debug(sbi);
					continue;
				}
				add_gc_block(gc_list, inode, off);
				continue;
			}

			data_page = hmfs_get_read_data_page(inode,
						start_bidx, REQ_RAHEAD, true);
			up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
			if (IS_ERR(data_page)) {
				iput(inode);
				continue;
			}

			f2fs_put_page(data_page, 0);
			add_gc_block(gc_list, inode, off);
			continue;
		}
	}

	if (++phase < 4)
		goto next_step;

	submitted = move_data_by_file(sbi, sum, gc_list, segno, gc_type,
				datamove, &gc_cnt);
	del_gc_offset(sbi, gc_list);

	bd_mutex_lock(&sbi->bd_mutex);
	inc_bd_array_val(sbi, gc_data_blk_cnt, gc_type, gc_cnt);
	inc_bd_array_val(sbi, hotcold_cnt, HC_GC_COLD_DATA, gc_cnt);
	bd_mutex_unlock(&sbi->bd_mutex);
	return submitted;
}

static int __get_victim(struct f2fs_sb_info *sbi, unsigned int *victim,
			int gc_type)
{
	struct sit_info *sit_i = SIT_I(sbi);
	int ret;
	unsigned int segno;
	struct rescue_seg_entry *rs_entry;

retry:
	/* should gc rescue segments reported by ufs first */
	if (!list_empty(&sbi->gc_control_info.rescue_segment_list)) {
		rs_entry = list_first_entry(&(sbi->gc_control_info.rescue_segment_list),
				struct rescue_seg_entry, list);
		segno = rs_entry->segno;

		if (get_valid_blocks(sbi, segno, false) == 0 ||
			IS_CURSEG(sbi, segno)) {
			list_del(&rs_entry->list);
			hmfs_msg(sbi->sb, KERN_NOTICE,
					 "rescue segment %u has been gced", segno);
			goto retry;
		}

		sbi->next_victim_seg = segno;
		hmfs_msg(sbi->sb, KERN_NOTICE, "try to gc rescue segment %u", segno);
	}

	if (sbi->next_victim_seg != NULL_SEGNO) {
		*victim = sbi->next_victim_seg;
		return 1;
	}

	down_write(&sit_i->sentry_lock);
	ret = DIRTY_I(sbi)->v_ops->get_victim(sbi, victim, gc_type,
					      NO_CHECK_TYPE, LFS, 0);
	up_write(&sit_i->sentry_lock);
	sbi->next_victim_seg = *victim;

	return ret;
}

static int do_garbage_collect(struct f2fs_sb_info *sbi,
				unsigned int start_segno,
				struct gc_inode_list *gc_list, int gc_type)
{
	struct page *sum_page;
	struct f2fs_summary_block *sum;
	struct blk_plug plug;
	unsigned int segno = start_segno;
	unsigned int sec_end_segno = (start_segno / sbi->segs_per_sec) *
		sbi->segs_per_sec + sbi->segs_per_sec;
	/* only gc one segment each time, or it may impact foreground io */
	unsigned int end_segno = segno + 1;
	int seg_freed = 0;
	int hotcold_type = get_seg_entry(sbi, segno)->type;
	unsigned char type = (IS_DATASEG(hotcold_type) ||
				IS_DMGCSEG(hotcold_type)) ?
				SUM_TYPE_DATA : SUM_TYPE_NODE;
	int submitted = 0;
	bool datamove = false;

	/* reference all summary page */
	while (segno < end_segno) {
		sum_page = hmfs_get_sum_page(sbi, segno++);
		if (IS_ERR(sum_page)) {
			int err = PTR_ERR(sum_page);

			end_segno = segno - 1;
			for (segno = start_segno; segno < end_segno; segno++) {
				sum_page = find_get_page(META_MAPPING(sbi),
						GET_SUM_BLOCK(sbi, segno));
				f2fs_put_page(sum_page, 0);
				f2fs_put_page(sum_page, 0);
			}
			return err;
		}
		unlock_page(sum_page);
	}

	blk_start_plug(&plug);

	for (segno = start_segno; segno < end_segno; segno++) {

		/* find segment summary of victim */
		sum_page = find_get_page(META_MAPPING(sbi),
					GET_SUM_BLOCK(sbi, segno));
		f2fs_put_page(sum_page, 0);

		if (get_valid_blocks(sbi, segno, false) == 0 ||
				!PageUptodate(sum_page) ||
				unlikely(f2fs_cp_error(sbi)))
			goto next;

		sum = page_address(sum_page);
		if (type != GET_SUM_TYPE((&sum->footer))) {
			hmfs_msg(sbi->sb, KERN_ERR, "Inconsistent segment (%u) "
				"type [%d, %d] in SSA and SIT",
				segno, type, GET_SUM_TYPE((&sum->footer)));
			set_sbi_flag(sbi, SBI_NEED_FSCK);
			hmfs_set_need_fsck_report();
			goto next;
		}

		if (hmfs_datamove_on(sbi))
			datamove = true;

		/*
		 * this is to avoid deadlock:
		 * - lock_page(sum_page)	 - hmfs_replace_block
		 *  - check_valid_map()		   - down_write(sentry_lock)
		 *   - down_read(sentry_lock)	  - change_curseg()
		 *				    - lock_page(sum_page)
		 */
		if (type == SUM_TYPE_NODE)
			submitted += gc_node_segment(sbi, sum->entries, segno,
								gc_type);
		else
			submitted += hmfs_gc_data_segment(sbi, sum->entries, gc_list,
						segno, gc_type, datamove);

		stat_inc_seg_count(sbi, type, gc_type);

		if (gc_type == FG_GC &&
				get_valid_blocks(sbi, segno, false) == 0)
			seg_freed++;
		bd_mutex_lock(&sbi->bd_mutex);
		if (gc_type == BG_GC || get_valid_blocks(sbi, segno, 1) == 0) {
			if (type == SUM_TYPE_NODE)
				inc_bd_array_val(sbi, gc_node_seg_cnt, gc_type, 1);
			else
				inc_bd_array_val(sbi, gc_data_seg_cnt, gc_type, 1);
			inc_bd_array_val(sbi, hotcold_gc_seg_cnt, hotcold_type + 1, 1UL);/*lint !e679*/
		}
		inc_bd_array_val(sbi, hotcold_gc_blk_cnt, hotcold_type + 1,
					(unsigned long)get_valid_blocks(sbi, segno, 1));/*lint !e679*/
		bd_mutex_unlock(&sbi->bd_mutex);
next:
		f2fs_put_page(sum_page, 0);

		sbi->next_victim_seg++;
	}

	if (submitted)
		hmfs_submit_merged_write(sbi,
				(type == SUM_TYPE_NODE) ? NODE : DATA);

	blk_finish_plug(&plug);

	stat_inc_call_count(sbi->stat_info);
	sbi->gc_control_info.gc_skip_count++;

	if (sbi->next_victim_seg == sec_end_segno)
		sbi->next_victim_seg = NULL_SEGNO;

	return seg_freed;
}

int hmfs_gc(struct f2fs_sb_info *sbi, bool sync,
			bool background, unsigned int segno)
{
	int gc_type = sync ? FG_GC : BG_GC;
	int sec_freed = 0, seg_freed = 0, total_freed = 0;
	int ret = 0;
	struct cp_control cpc;
	unsigned int init_segno = segno;
	struct gc_inode_list gc_list = {
		.ilist = LIST_HEAD_INIT(gc_list.ilist),
		.iroot = RADIX_TREE_INIT(GFP_NOFS),
	};
	unsigned long long last_skipped = sbi->skipped_atomic_files[FG_GC];
	unsigned long long first_skipped;
	unsigned int skipped_round = 0, round = 0;
	int gc_completed = 0;
	u64 fggc_begin = 0, fggc_end;
	u64 cp_begin, cp_end;
	u64 cp_time = 0;
	unsigned int total_rounds = 0;

	fggc_begin = local_clock();

	trace_hmfs_gc_begin(sbi->sb, sync, background,
				get_pages(sbi, F2FS_DIRTY_NODES),
				get_pages(sbi, F2FS_DIRTY_DENTS),
				get_pages(sbi, F2FS_DIRTY_IMETA),
				free_sections(sbi),
				free_segments(sbi),
				reserved_segments(sbi),
				prefree_segments(sbi));

	cpc.reason = __get_cp_reason(sbi);
	sbi->skipped_gc_rwsem = 0;
	first_skipped = last_skipped;
gc_more:
	total_rounds++;
	if ((cp_time / MS_TO_NS) > DEF_CP_LONG_TIME_IN_GC) {
		hmfs_msg(sbi->sb, KERN_ERR, "Long cp_time in gc, "
				"over 10s. total_rounds=%d, iolimit=%d",
				total_rounds, sbi->gc_control_info.iolimit);
		cp_time = 0;
		total_rounds = 0;
	}
	if (unlikely(!(sbi->sb->s_flags & MS_ACTIVE))) {
		ret = -EINVAL;
		goto stop;
	}
	if (unlikely(f2fs_cp_error(sbi))) {
		ret = -EIO;
		goto stop;
	}

	if (gc_type == BG_GC && has_not_enough_free_secs(sbi, 0, 0)) {
		/*
		 * For example, if there are many prefree_segments below given
		 * threshold, we can make them free by checkpoint. Then, we
		 * secure free segments which doesn't need fggc any more.
		 */
		if ((prefree_segments(sbi) > sbi->segs_per_sec) &&
				!is_sbi_flag_set(sbi, SBI_CP_DISABLED)) {
			ret = hmfs_write_checkpoint(sbi, &cpc);
			if (ret)
				goto stop;
		}
		if (has_not_enough_free_secs(sbi, 0, 0))
			gc_type = FG_GC;
	}

	/* hmfs_balance_fs doesn't need to do BG_GC in critical path. */
	if (gc_type == BG_GC && !background) {
		ret = -EINVAL;
		goto stop;
	}

	if (sbi->gc_control_info.is_greedy)
		gc_type = FG_GC;

	if (!__get_victim(sbi, &segno, gc_type)) {
		sbi->gc_control_info.loop_cnt++;
		ret = -ENODATA;
		goto stop;
	}
	if (sbi->gc_control_info.iolimit > 0) {
		gc_type = FG_GC;
		if (sbi->gc_loop.count &&
			!(sbi->gc_loop.count % GC_THREAD_LOOP_PRINT_TIME))
			hmfs_msg(sbi->sb, KERN_NOTICE,
				"victim_segno=%u, loop_count=%lu\n",
				segno, sbi->gc_loop.count);
	}

	if (unlikely(sbi->gc_loop.check && (GET_SEG_FROM_SEC(sbi,
		GET_SEC_FROM_SEG(sbi, segno)) != sbi->gc_loop.segno))) {
		hmfs_msg(sbi->sb, KERN_NOTICE,
			"init gc_loop :segno=%u, gc_loop.segno=%u, "
			"round=%u, total_freed=%d\n",
			segno, sbi->gc_loop.segno, round, total_freed);
		init_hmfs_gc_loop(sbi);
	}
	seg_freed = do_garbage_collect(sbi, segno, &gc_list, gc_type);
	if (gc_type == FG_GC && seg_freed == sbi->segs_per_sec)
		sec_freed++;
	else if (unlikely(!seg_freed && gc_type == FG_GC)) {
		if (!sbi->gc_loop.check) {
			sbi->gc_loop.check = true;
			sbi->gc_loop.count = 1;
			sbi->gc_loop.segno = GET_SEG_FROM_SEC(sbi,
					GET_SEC_FROM_SEG(sbi, segno));
		}
		if (!(sbi->gc_loop.count % F2FS_GC_LOOP_MOD))
			hmfs_msg(sbi->sb, KERN_ERR,
				"hmfs_gc_loop same victim retry:%lu in %s:%d "
				"segno:%u type:%d blocks:%u "
				"free:%u prefree:%u rsvd:%u\n",
				sbi->gc_loop.count, __func__, __LINE__,
				segno, get_seg_entry(sbi, segno)->type,
				get_valid_blocks(sbi, segno, sbi->segs_per_sec),
				free_segments(sbi), prefree_segments(sbi),
				reserved_segments(sbi));
		sbi->gc_loop.count++;
		if (sbi->gc_loop.count > F2FS_GC_LOOP_MAX) {
			if (!sbi->gc_loop.segmap)
				sbi->gc_loop.segmap =
					kvzalloc(f2fs_bitmap_size(MAIN_SEGS(sbi)), GFP_NOFS);
			if (sbi->gc_loop.segmap)
				set_bit(GET_SEG_FROM_SEC(sbi,
					GET_SEC_FROM_SEG(sbi, segno)),
					sbi->gc_loop.segmap);
		}
	}
	total_freed += seg_freed;
	gc_completed = 1;

	/* update atomic file statistics one time per section */
	if (gc_type == FG_GC && sbi->next_victim_seg == NULL_SEGNO) {
		if (sbi->skipped_atomic_files[FG_GC] > last_skipped ||
						sbi->skipped_gc_rwsem)
			skipped_round++;
		last_skipped = sbi->skipped_atomic_files[FG_GC];
		round++;
	}

	if (gc_type == FG_GC)
		sbi->cur_victim_sec = NULL_SEGNO;

	if (sync)
		goto stop;

	/*
	 * When free sections is low, we'll write checkpoint to free some
	 * sections when prefree sections reach threshold. And the freed
	 * sections can help to improved io performance.
	 */
	if (sbi->prefree_sec_threshold > 0 &&
		prefree_sections(sbi) > sbi->prefree_sec_threshold &&
		free_sections(sbi) < sbi->gc_stat.level[BG_GC_LEVEL3]) {

		ret = hmfs_write_checkpoint(sbi, &cpc);
		if (ret)
			goto stop;
	}

	if (has_not_enough_free_secs(sbi, sec_freed, 0)) {
		if (skipped_round <= MAX_SKIP_GC_COUNT ||
					skipped_round * 2 < round) {
			segno = NULL_SEGNO;
			if (prefree_segments(sbi) > sbi->segs_per_sec) {
				cp_begin = local_clock();
				ret = hmfs_write_checkpoint(sbi, &cpc);
				if (ret)
					goto stop;
				cp_end = local_clock();
				cp_time += cp_end - cp_begin;
				sec_freed = 0;
				if (!has_not_enough_free_secs(sbi, 0, 0))
					goto stop;
			}
			goto gc_more;
		}

		if (first_skipped < last_skipped &&
				(last_skipped - first_skipped) >
						sbi->skipped_gc_rwsem) {
			hmfs_drop_inmem_pages_all(sbi, true);
			segno = NULL_SEGNO;
			if (prefree_segments(sbi) > sbi->segs_per_sec) {
				cp_begin = local_clock();
				ret = hmfs_write_checkpoint(sbi, &cpc);
				if (ret)
					goto stop;
				cp_end = local_clock();
				cp_time += cp_end - cp_begin;
				sec_freed = 0;
				if (!has_not_enough_free_secs(sbi, 0, 0))
					goto stop;
			}
			goto gc_more;
		}
		if (gc_type == FG_GC && !is_sbi_flag_set(sbi, SBI_CP_DISABLED))
			ret = hmfs_write_checkpoint(sbi, &cpc);
	}
stop:
	SIT_I(sbi)->last_victim[ALLOC_NEXT] = 0;
	SIT_I(sbi)->last_victim[FLUSH_DEVICE] = init_segno;

	trace_hmfs_gc_end(sbi->sb, ret, total_freed, sec_freed,
				get_pages(sbi, F2FS_DIRTY_NODES),
				get_pages(sbi, F2FS_DIRTY_DENTS),
				get_pages(sbi, F2FS_DIRTY_IMETA),
				free_sections(sbi),
				free_segments(sbi),
				reserved_segments(sbi),
				prefree_segments(sbi));

	if (unlikely(sbi->gc_loop.segmap)) {
		kvfree(sbi->gc_loop.segmap);
		sbi->gc_loop.segmap = NULL;
	}
	if (unlikely(sbi->gc_loop.check))
		init_hmfs_gc_loop(sbi);

	mutex_unlock(&sbi->gc_mutex);
	if (gc_completed) {
		bd_mutex_lock(&sbi->bd_mutex);
		if (gc_type == FG_GC && fggc_begin) {
			fggc_end = local_clock();
			inc_bd_val(sbi, fggc_time, fggc_end - fggc_begin);
		}
		inc_bd_array_val(sbi, gc_cnt, gc_type, 1);
		if (ret)
			inc_bd_array_val(sbi, gc_fail_cnt, gc_type, 1);
		bd_mutex_unlock(&sbi->bd_mutex);
	}

	put_gc_inode(&gc_list);

	if (sync && !ret)
		ret = sec_freed ? 0 : -EAGAIN;
	return ret;
}

int __init hmfs_create_garbage_collection_cache(void)
{
	victim_entry_slab = f2fs_kmem_cache_create("victim_entry",
					sizeof(struct victim_entry));
	if (!victim_entry_slab)
		return -ENOMEM;

	hmfs_inode_gc_entry_slab = f2fs_kmem_cache_create("hmfs_inode_gc_entry",
					sizeof(struct inode_gc_entry));
	if (!hmfs_inode_gc_entry_slab) {
		kmem_cache_destroy(victim_entry_slab);
		return -ENOMEM;
	}

	return 0;
}

void hmfs_destroy_garbage_collection_cache(void)
{
	kmem_cache_destroy(victim_entry_slab);
	kmem_cache_destroy(hmfs_inode_gc_entry_slab);
}


void hmfs_build_gc_manager(struct f2fs_sb_info *sbi)
{
	DIRTY_I(sbi)->v_ops = &default_v_ops;

	sbi->gc_pin_file_threshold = DEF_GC_FAILED_PINNED_FILES;

	/* give warm/cold data area from slower device */
	if (sbi->s_ndevs && sbi->segs_per_sec == 1)
		SIT_I(sbi)->last_victim[ALLOC_NEXT] =
				GET_SEGNO(sbi, FDEV(0).end_blk) + 1;
}

static int free_segment_range(struct f2fs_sb_info *sbi, unsigned int start,
		unsigned int end)
{
	int type;
	unsigned int segno, next_inuse;
	struct seg_entry *se;
	int err = 0;

	/* Move out cursegs from the target range */
	for (type = CURSEG_HOT_DATA; type < NO_CHECK_TYPE; type++)
		hmfs_allocate_segment_for_resize(sbi, type, start, end);

	/* do GC to move out valid blocks in the range */
	for (segno = start; segno <= end; segno += sbi->segs_per_sec) {
		struct gc_inode_list gc_list = {
			.ilist = LIST_HEAD_INIT(gc_list.ilist),
			.iroot = RADIX_TREE_INIT(GFP_NOFS),
		};

		mutex_lock(&sbi->gc_mutex);
		sbi->next_victim_seg = segno;
		do_garbage_collect(sbi, segno, &gc_list, FG_GC);
		mutex_unlock(&sbi->gc_mutex);
		put_gc_inode(&gc_list);

		if (get_valid_blocks(sbi, segno, true))
			return -EAGAIN;
	}

	err = hmfs_sync_fs(sbi->sb, 1);
	if (err)
		return err;

	next_inuse = find_next_inuse(FREE_I(sbi), end + 1, start);
	if (next_inuse <= end) {
		se = get_seg_entry(sbi, next_inuse);
		hmfs_msg(sbi->sb, KERN_ERR,
			"segno %u should be free but still inuse! type %u, vblks %u, ckpt_vblks %u, IS_CURSEG %d",
			next_inuse, se->type, se->valid_blocks,
			se->ckpt_valid_blocks, IS_CURSEG(sbi, next_inuse));
		err = -EAGAIN;
	}
	return err;
}

static int read_and_check_mapping(struct f2fs_sb_info *sbi,
					struct stor_dev_mapping_partition *mp,
					unsigned int secs) {
	int err = 0;

	err = mas_blk_device_read_mapping_partition(sbi->sb->s_bdev, mp);
	if (err) {
		hmfs_msg(sbi->sb, KERN_ERR,
				"Can not read mapping partitions.");
		goto out;
	}

	if (sbi->s_ndevs > 1) {
		unsigned int secs_in_last_dev =
			FDEV(sbi->s_ndevs - 1).total_segments /
			sbi->segs_per_sec;
		if (secs_in_last_dev < secs ||
			mp->partion_size[PARTITION_TYPE_USER0] < secs ||
			(mp->partion_size[PARTITION_TYPE_USER1] == 0 &&
			mp->partion_start[PARTITION_TYPE_USER1] == 0)) {
			/*
			 * Either one of following conditions:
			 *
			 * 1. last device size belows shrunk secs.
			 * 2. empty(not used by hmfs) 2M-mapping region on
			 *    last device belows shrunk secs.
			 * 3. there is no 2M-mapping region on last device,
			 *    because the whole device has already been used
			 *    as kdump area.
			 */
			hmfs_msg(sbi->sb, KERN_ERR,
				"Not enougn empty space on last device.");
			err = -EINVAL;
			goto out;
		}
	}

out:
	return err;
}

static int config_and_validate_mapping(struct f2fs_sb_info *sbi,
					struct stor_dev_mapping_partition *mp,
					unsigned int secs) {
	int err = 0;

	/* Set shrunk area as 4K-mapping */
	if (sbi->s_ndevs > 1) {
		/*
		 * Multi-devices mode:
		 *
		 *  sdd73 |           sdd74             |     sdd75
		 * ____________________________________________________
		 * system |      main_2     | old_kdump | meta | main_1
		 * ----------------------------------------------------
		 *   4K   |        2M       |       4K         |   2M
		 *  meta0 |       user0     |      meta1       |  user1
		 *                          :
		 * After resize:            :
		 *                 ----------
		 *                 :
		 *  sdd73 |        ↓  sdd74             |     sdd75
		 * ____________________________________________________
		 * system | main_2 |     new_kdump      | meta | main_1
		 * ----------------------------------------------------
		 *   4K   |   2M   |             4K            |   2M
		 *  meta0 |  user0 |            meta1          | user1
		 */
		mp->partion_size[PARTITION_TYPE_USER0] -= secs;
		if (mp->partion_size[PARTITION_TYPE_USER0] == 0) {
			/* Region user0 will entirely be set as 4K-mapping. */

			/*
			 * Merge old regions meta0, user0 and meta1 as new
			 * region meta0
			 */
			mp->partion_size[PARTITION_TYPE_META0] +=
				mp->partion_size[PARTITION_TYPE_META1] + secs;
			mp->partion_start[PARTITION_TYPE_META1] = 0;
			mp->partion_size[PARTITION_TYPE_META1] = 0;
			/* Set region user1 as user0 */
			mp->partion_start[PARTITION_TYPE_USER0] =
					mp->partion_start[PARTITION_TYPE_USER1];
			mp->partion_size[PARTITION_TYPE_USER0] =
					mp->partion_size[PARTITION_TYPE_USER1];
			/* Invalidate region user1 */
			mp->partion_start[PARTITION_TYPE_USER1] = 0;
			mp->partion_size[PARTITION_TYPE_USER1] = 0;

			/*
			 * Just make sure last 2M-mapping region is consistent
			 * with main area, because we don't know the size of
			 * kdump area.
			 */
			if (mp->partion_size[PARTITION_TYPE_USER0] !=
							MAIN_SECS(sbi)) {
				err = -EINVAL;
				hmfs_msg(sbi->sb, KERN_ERR,
				"inconsistent user0 size %u, expect %u.",
					mp->partion_size[PARTITION_TYPE_USER0],
					MAIN_SECS(sbi));
				goto out;
			}
		} else {
			mp->partion_start[PARTITION_TYPE_META1] -= secs;
			mp->partion_size[PARTITION_TYPE_META1] += secs;

			/*
			 * Just make sure last 2M-mapping region is consistent
			 * with main area, because we don't know the size of
			 * kdump area.
			 */
			if (mp->partion_size[PARTITION_TYPE_USER0] +
				    mp->partion_size[PARTITION_TYPE_USER1] !=
				    MAIN_SECS(sbi)) {
				err = -EINVAL;
				hmfs_msg(sbi->sb, KERN_ERR,
				"inconsistent user0+user1 size %u, expect %u.",
					mp->partion_size[PARTITION_TYPE_USER0] +
					mp->partion_size[PARTITION_TYPE_USER1],
					MAIN_SECS(sbi));
				goto out;
			}
		}
	} else if (mp->partion_start[PARTITION_TYPE_META1] == 0 &&
			mp->partion_size[PARTITION_TYPE_META1] == 0){
		/*
		 * Single-device mode:
		 * Situation 1)
		 *
		 *  sdd74 |         sdd75
		 * _____________________________________
		 * system | meta |          main
		 * -------------------------------------
		 *       4K      |	     2M
		 *      meta0    |          user0       :
		 *					:
		 * After resize:			:
		 *                      ----------------
		 *                      :
		 *  sdd74 |     sdd75   ↓
		 * _____________________________________
		 * system | meta | main |     kdump
		 * -------------------------------------
		 *       4K      |  2M  |       4K
		 *      meta0    | user0|      meta1
		 */
		mp->partion_size[PARTITION_TYPE_USER0] -= secs;
		mp->partion_start[PARTITION_TYPE_META1] =
			mp->partion_start[PARTITION_TYPE_USER0] +
			mp->partion_size[PARTITION_TYPE_USER0];
		mp->partion_size[PARTITION_TYPE_META1] = secs;
	} else {
		/*
		 * Single-device mode:
		 * Situation 2)
		 *
		 *  sdd74 |         sdd75
		 * _____________________________________
		 * system | meta |   main    | old_kdump
		 * -------------------------------------
		 *       4K      |    2M     |    4K
		 *      meta0    |   user0   |   meta1
		 *                           :
		 * After resize:             :
		 *                      ------
		 *                      :
		 *  sdd74 |     sdd75   ↓
		 * _____________________________________
		 * system | meta | main |   new_kdump
		 * -------------------------------------
		 *       4K      |  2M  |       4K
		 *      meta0    | user0|      meta1
		 */
		mp->partion_size[PARTITION_TYPE_USER0] -= secs;
		mp->partion_start[PARTITION_TYPE_META1] -= secs;
		mp->partion_size[PARTITION_TYPE_META1] += secs;
	}

	if (sbi->s_ndevs <= 1) {
		/*
		 * Just make sure 2M-mapping region is consistent with main
		 * area, because we don't know the size of kdump area.
		 */
		if (mp->partion_size[PARTITION_TYPE_USER0] != MAIN_SECS(sbi)) {
			err = -EINVAL;
			hmfs_msg(sbi->sb, KERN_ERR,
				"inconsistent user0 size %u, expect %u.",
				mp->partion_size[PARTITION_TYPE_USER0],
				MAIN_SECS(sbi));
			goto out;
		}
		/* Make sure there is only one 2M-mapping region. */
		if (mp->partion_start[PARTITION_TYPE_USER1] != 0 ||
				mp->partion_size[PARTITION_TYPE_USER1] != 0) {
			err = -EINVAL;
			hmfs_msg(sbi->sb, KERN_ERR,
				"inconsistent user1 %u-%u, expect %u-%u.",
				mp->partion_start[PARTITION_TYPE_USER1],
				mp->partion_size[PARTITION_TYPE_USER1], 0, 0);
			goto out;
		}
	}

	err = mas_blk_device_config_mapping_partition(sbi->sb->s_bdev, mp);
	if (err) {
		hmfs_msg(sbi->sb, KERN_ERR,
				"Can not map %u:%u as 4K partition.",
				mp->partion_start[PARTITION_TYPE_META1],
				mp->partion_size[PARTITION_TYPE_META1]);
		goto out;
	}

out:
	return err;
}

static void update_sb_metadata(struct f2fs_sb_info *sbi, int secs)
{
	struct f2fs_super_block *raw_sb = F2FS_RAW_SUPER(sbi);
	int section_count = le32_to_cpu(raw_sb->section_count);
	int segment_count = le32_to_cpu(raw_sb->segment_count);
	int segment_count_main = le32_to_cpu(raw_sb->segment_count_main);
	long long block_count = le64_to_cpu(raw_sb->block_count);
	int segs = secs * sbi->segs_per_sec;

	raw_sb->section_count = cpu_to_le32(section_count + secs);
	raw_sb->segment_count = cpu_to_le32(segment_count + segs);
	raw_sb->segment_count_main = cpu_to_le32(segment_count_main + segs);
	raw_sb->block_count = cpu_to_le64(block_count +
					(long long)segs * sbi->blocks_per_seg);
}

static void update_fs_metadata(struct f2fs_sb_info *sbi, int secs)
{
	int segs = secs * sbi->segs_per_sec;
	long long user_block_count =
				le64_to_cpu(F2FS_CKPT(sbi)->user_block_count);

	SM_I(sbi)->segment_count = (int)SM_I(sbi)->segment_count + segs;
	MAIN_SEGS(sbi) = (int)MAIN_SEGS(sbi) + segs;
	FREE_I(sbi)->free_sections = (int)FREE_I(sbi)->free_sections + secs;
	FREE_I(sbi)->free_segments = (int)FREE_I(sbi)->free_segments + segs;
	F2FS_CKPT(sbi)->user_block_count = cpu_to_le64(user_block_count +
					(long long)segs * sbi->blocks_per_seg);
}

int hmfs_resize_fs(struct f2fs_sb_info *sbi, size_t resize_len)
{
	struct stor_dev_mapping_partition mp;
	unsigned int section_size = F2FS_BLKSIZE * BLKS_PER_SEC(sbi);
	unsigned int secs = (resize_len + section_size - 1) / section_size;
	__u64 shrunk_blocks = (__u64)secs * BLKS_PER_SEC(sbi);
	int gc_mode;
	int err = 0;

	if (is_sbi_flag_set(sbi, SBI_NEED_FSCK)) {
		hmfs_msg(sbi->sb, KERN_ERR,
			"Should run fsck to repair first.");
		return -EINVAL;
	}

	if (test_opt(sbi, DISABLE_CHECKPOINT)) {
		hmfs_msg(sbi->sb, KERN_ERR,
			"Checkpoint should be enabled.");
		return -EINVAL;
	}

	err = read_and_check_mapping(sbi, &mp, secs);
	if (err)
		return err;

	spin_lock(&sbi->stat_lock);
	if (shrunk_blocks + valid_user_blocks(sbi) +
			sbi->current_reserved_blocks +
			sbi->unusable_block_count +
			F2FS_OPTION(sbi).root_reserved_blocks >
			sbi->user_block_count)
		err = -ENOSPC;
	else
		sbi->user_block_count -= shrunk_blocks;
	spin_unlock(&sbi->stat_lock);
	if (err)
		return err;

	mutex_lock(&sbi->resize_mutex);
	set_sbi_flag(sbi, SBI_IS_RESIZEFS);
	sbi->resized = true;

	mutex_lock(&DIRTY_I(sbi)->seglist_lock);
	MAIN_SECS(sbi) -= secs;
	for (gc_mode = 0; gc_mode < MAX_GC_POLICY; gc_mode++)
		if (SIT_I(sbi)->last_victim[gc_mode] >=
				MAIN_SECS(sbi) * sbi->segs_per_sec)
			SIT_I(sbi)->last_victim[gc_mode] = 0;
	mutex_unlock(&DIRTY_I(sbi)->seglist_lock);

	err = free_segment_range(sbi, MAIN_SECS(sbi) * sbi->segs_per_sec,
			MAIN_SEGS(sbi) - 1);
	if (err) {
		sbi->resized = false;
		goto out;
	}

	err = config_and_validate_mapping(sbi, &mp, secs);
	if (err)
		goto out;

	update_sb_metadata(sbi, -secs);

	err = hmfs_commit_super(sbi, false);
	if (err) {
		update_sb_metadata(sbi, secs);
		goto out;
	}

	mutex_lock(&sbi->cp_mutex);
	update_fs_metadata(sbi, -secs);
	clear_sbi_flag(sbi, SBI_IS_RESIZEFS);
	/* Force sync cp without CP_RESIZEFS_FLAG flag */
	set_sbi_flag(sbi, SBI_IS_DIRTY);
	mutex_unlock(&sbi->cp_mutex);

	err = hmfs_sync_fs(sbi->sb, 1);
	if (err) {
		mutex_lock(&sbi->cp_mutex);
		update_fs_metadata(sbi, secs);
		mutex_unlock(&sbi->cp_mutex);
		update_sb_metadata(sbi, secs);
		hmfs_commit_super(sbi, false);
	}
out:
	if (err) {
		set_sbi_flag(sbi, SBI_NEED_FSCK);
		hmfs_set_need_fsck_report();
		hmfs_msg(sbi->sb, KERN_ERR,
				"resize_fs failed, should run fsck to repair!");

		MAIN_SECS(sbi) += secs;
		spin_lock(&sbi->stat_lock);
		sbi->user_block_count += shrunk_blocks;
		spin_unlock(&sbi->stat_lock);
	}
	clear_sbi_flag(sbi, SBI_IS_RESIZEFS);
	mutex_unlock(&sbi->resize_mutex);
	return err;
}

/*
 * Get every level interval.
 * max=1:  max(min_segs, min_secs)
 * max=0:  min(min_segs, min_secs)
 */
static int get_level_interval(struct f2fs_sb_info *sbi,
		unsigned int min_segs, unsigned int min_secs, bool max)
{
	int result;
	int sec_count;

	sec_count = (min_segs + sbi->segs_per_sec - 1) / sbi->segs_per_sec;

	if (max)
		result = sec_count < min_secs ? min_secs : sec_count;
	else
		result = sec_count < min_secs ? sec_count : min_secs;

	return result;
}

/*
 * GC use different gc policy in every level. Level becomes less
 * and policy becomes stronger from level1 to level6.
 */
static void hmfs_adjust_gc_levels(struct f2fs_sb_info *sbi)
{
	int *level = sbi->gc_stat.level;
	int extra;

	/*
	 * LEVEL1: Soft GC1 space, for free secs are enough.
	 *         Formula: main_secs / 4
	 * LEVEL2: Soft GC2 space, gc stronger a little than soft GC1.
	 *         Formula: main_secs / 8
	 * LEVEL3: Space used for middle level GC, avoid that GC suddenly speed up.
	 *         Formula: min(768 segs, 4 secs)
	 * LEVEL4: Fast GC1 space, FG IO can interrupt gc in this level.
	 *         Formula: 3 + max(768 segs, 2 secs).
	 *         "3" represents max concurrent write stream. 768 segs represents
	 *         min disk space for normal use.
	 * LEVEL5: Fast GC2 space, avoid that performance reduces suddenly.
	 *         FG IO can't interrupt gc in this level.
	 *         Formula: max(0, 4 secs)
	 * LEVEL6: FG_GC space.  Formula: # of reserve section
	 */
	level[BG_GC_LEVEL6] = reserved_sections(sbi);
	level[BG_GC_LEVEL5] = level[BG_GC_LEVEL6] +
		get_level_interval(sbi, 0, MIN_SECS, 1);
	level[BG_GC_LEVEL4] = level[BG_GC_LEVEL5] + CONCURRENT_SECS +
		get_level_interval(sbi, MIN_SEGS, 2, 1);
	level[BG_GC_LEVEL3] = level[BG_GC_LEVEL4] +
		get_level_interval(sbi, MIN_SEGS, MIN_SECS, 0);
	if ((hmfs_overprovision_sections(sbi) > level[BG_GC_LEVEL3]) &&
			(hmfs_overprovision_sections(sbi) - level[BG_GC_LEVEL3]) *
			sbi->segs_per_sec > EXTRA_SEGS_IN_OVP) {
		extra = (hmfs_overprovision_sections(sbi) - level[BG_GC_LEVEL3]) / 4;
		level[BG_GC_LEVEL4] += extra;
		level[BG_GC_LEVEL3] += extra;
	}
	level[BG_GC_LEVEL2] = MAIN_SECS(sbi) / 8;
	level[BG_GC_LEVEL1] = MAIN_SECS(sbi) / 4;

	if (level[BG_GC_LEVEL3] >= hmfs_overprovision_sections(sbi))
		level[BG_GC_LEVEL3] = hmfs_overprovision_sections(sbi);
	if (level[BG_GC_LEVEL4] >= hmfs_overprovision_sections(sbi))
		level[BG_GC_LEVEL4] = hmfs_overprovision_sections(sbi);
	if (level[BG_GC_LEVEL5] >= hmfs_overprovision_sections(sbi))
		level[BG_GC_LEVEL5] = hmfs_overprovision_sections(sbi);
	if (level[BG_GC_LEVEL6] >= hmfs_overprovision_sections(sbi)) {
		hmfs_msg(sbi->sb, KERN_ERR, "bad sections too much, num:%d\n",
				level[BG_GC_LEVEL6] -
				SM_I(sbi)->cp_reserved_segments / sbi->segs_per_sec);
		f2fs_bug_on(sbi, 1);
	}
}

static int hmfs_init_gc_levels(struct f2fs_sb_info *sbi)
{
	int i;
	unsigned int num = 0;
	int ret = 0;
	struct stor_dev_bad_block_info bad_blk_info;

	/* get bad section number from fw */
	ret = mas_blk_get_bad_block_info(sbi->sb->s_bdev, &bad_blk_info);
	if (!ret) {
		num = (unsigned int)(bad_blk_info.tlc_bad_block_num);
		if (num >= hmfs_overprovision_sections(sbi)) {
			hmfs_msg(sbi->sb, KERN_ERR, "get bad sections from"
					"fw error, num:%d\n", num);
			f2fs_bug_on(sbi, 1);
		}
	} else {
		hmfs_msg(sbi->sb, KERN_ERR, "mas_blk_get_bad_block_info "
				"return err, ret=%d\n", ret);
		return -EINVAL;
	}

	SM_I(sbi)->reserved_segments = SM_I(sbi)->cp_reserved_segments +
			num * sbi->segs_per_sec;
	hmfs_adjust_gc_levels(sbi);

	sbi->gc_stat_enable = 0;
	for (i = 0; i < ALL_GC_LEVELS; i++)
		sbi->gc_stat.times[i] = 0;

	return 0;
}

static void change_levels_by_bad_block(
		struct stor_dev_bad_block_info bad_blk_info, void *data)
{
	unsigned int num = (unsigned int)(bad_blk_info.tlc_bad_block_num);
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info*)data;

	if (!sbi)
		return;

	if (num >= hmfs_overprovision_sections(sbi)) {
		hmfs_msg(sbi->sb, KERN_ERR, "get bad sections from fw "
				"error, num:%d\n", num);
		f2fs_bug_on(sbi, 1);
	}

	SM_I(sbi)->reserved_segments = SM_I(sbi)->cp_reserved_segments +
			num * sbi->segs_per_sec;
	hmfs_adjust_gc_levels(sbi);
}

int hmfs_build_gc_control_info(struct f2fs_sb_info *sbi)
{
	int err = 0;
	atomic_set(&(sbi->gc_control_info.inflight), 0);
	init_waitqueue_head(&(sbi->gc_control_info.wait));
	sbi->gc_control_info.iolimit = 0;
	sbi->gc_control_info.loop_cnt = 0;
	sbi->gc_control_info.gc_skip_count = 0;
	sbi->gc_control_info.is_greedy = false;
	sbi->gc_control_info.idle_enabled = false;

	INIT_LIST_HEAD(&sbi->gc_control_info.rescue_segment_list);
	sbi->gc_control_info.rs_entry = NULL;

	err = hmfs_init_gc_levels(sbi);
	if(err)
		goto out;
	mas_blk_bad_block_notify_register(sbi->sb->s_bdev,
			change_levels_by_bad_block, (void*)sbi);
	sbi->need_bkops_wq = alloc_workqueue("hmfs_bkops_workqueue",
					WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
	if (!sbi->need_bkops_wq) {
		err = -ENOMEM;
		goto out;
	}

	INIT_DELAYED_WORK(&sbi->start_bkops_work, bkops_start_func);

	/*
	 * During small file tail covered write test, we found performance
	 * can be improved while decreasing this value from 100 to 5, and
	 * stay the same since then. So, we initialize this value to 5.
	 */
	sbi->prefree_sec_threshold = 5;
out:
	return err;
}

void hmfs_destroy_gc_control_info(struct f2fs_sb_info *sbi)
{
	struct hmfs_gc_kthread *gc_th = &sbi->gc_thread;

	cancel_delayed_work_sync(&sbi->start_bkops_work);
	flush_workqueue(sbi->need_bkops_wq);
	destroy_workqueue(sbi->need_bkops_wq);
#ifdef CONFIG_MAS_BLK
	del_timer_sync(&gc_th->nb_timer);
#endif

	if (sbi->gc_control_info.rs_entry != NULL) {
		kfree(sbi->gc_control_info.rs_entry);
		sbi->gc_control_info.rs_entry = NULL;
	}
}


static inline void __hmfs_remove_datamove_entry(struct hmfs_dm_manager *dm,
		block_t blkaddr, struct hmfs_dm_entry *e)
{
	radix_tree_delete(&dm->root, blkaddr);
	kfree(e);
	atomic_dec(&dm->count);
}

int hmfs_datamove_init(struct f2fs_sb_info *sbi)
{
	int i;
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);

	atomic_set(&dm->count, 0);
	INIT_RADIX_TREE(&dm->root, GFP_ATOMIC);
	init_rwsem(&dm->rw_sem);
	init_waitqueue_head(&dm->wait);
	atomic_set(&dm->refs, 0);

	dm->source_addrs = kvzalloc(sizeof(struct stor_dev_data_move_source_addr) *
					HMFS_DATAMOVE_PU_SIZE(sbi), GFP_KERNEL);
	if (!dm->source_addrs)
		return -ENOMEM;

	dm->source_inodes = kvzalloc(sizeof(struct stor_dev_data_move_source_inode) *
					HMFS_DATAMOVE_PU_SIZE(sbi), GFP_KERNEL);
	if (!dm->source_inodes) {
		kvfree(dm->source_addrs);
		return -ENOMEM;
	}

	for (i = 0; i < NR_CURSEG_DM_TYPE; i++) {
		struct hmfs_dm_info *dm_info = &dm->dm_info[i];

		dm_info->entries = kvzalloc(sizeof(struct hmfs_dm_entry) *
				HMFS_DATAMOVE_PU_SIZE(sbi), GFP_KERNEL);
		if (!dm_info->entries) {
			kvfree(dm->source_addrs);
			kvfree(dm->source_inodes);
			return -ENOMEM;
		}

		dm_info->index = 0;
		dm_info->verified_blkaddr = 0;
		dm_info->next_blkaddr = 0;
		dm_info->cached_last_blkaddr = 0;

		dm_info->rescue_index = 0;
		dm_info->rescue_entries = NULL;
	}

	if (test_hw_opt(sbi, DATAMOVE) &&
			is_set_ckpt_flags(sbi, CP_DATAMOVE_FLAG))
		set_sbi_flag(sbi, SBI_DATAMOVE_GC);

	return 0;
}

void hmfs_datamove_destroy(struct f2fs_sb_info *sbi)
{
	int i;
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_info *dm_info;

	if (dm->source_addrs)
		kvfree(dm->source_addrs);
	if (dm->source_inodes)
		kvfree(dm->source_inodes);

	dm->source_addrs = NULL;
	dm->source_inodes = NULL;

	for (i = 0; i < NR_CURSEG_DM_TYPE; i++) {
		dm_info = &dm->dm_info[i];
		if (dm_info->entries)
			kvfree(dm_info->entries);
		dm_info->entries = NULL;
	}
}

/*
 * get datamove submit array from radix tree.
 * dm->rw_sem is holden before call this function;
 */
void hmfs_datamove_fill_array(struct f2fs_sb_info *sbi,
		block_t next_submit_blkaddr, block_t cache_last_blkaddr,
		int dm_stream_id)
{
	struct hmfs_dm_entry *tmp, *dm_entry;
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_info *dm_info = &dm->dm_info[dm_stream_id];
	int index = dm_info->index;
	bool cached_newsec = false;

	if (!cache_last_blkaddr)
		return;

	if (next_submit_blkaddr) {
		if (DATA_BLOCK_IN_SAME_SEC(sbi,
				next_submit_blkaddr, cache_last_blkaddr) &&
				next_submit_blkaddr > cache_last_blkaddr)
			return;

		if (IS_LAST_DATA_BLOCK_IN_SEC(sbi,
				next_submit_blkaddr - 1, DATA))
			cached_newsec = true;
	} else {
		cached_newsec = true;
	}

	if (cached_newsec)
		next_submit_blkaddr = START_BLOCK(sbi, GET_SEG_FROM_SEC(sbi,
			GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, cache_last_blkaddr))));

	/* refill dm submit array */
	f2fs_bug_on(sbi, index != 0);
	while (index < HMFS_DATAMOVE_PU_SIZE(sbi) &&
			next_submit_blkaddr <= cache_last_blkaddr) {
		rcu_read_lock();
		tmp = radix_tree_lookup(&dm->root, next_submit_blkaddr++);
		rcu_read_unlock();
		/* can always find unverified entry */
		f2fs_bug_on(sbi, !tmp);

		dm_entry = &dm_info->entries[index++];
		dm_entry->src_blkaddr = tmp->src_blkaddr;
		dm_entry->dst_blkaddr = tmp->dst_blkaddr;
		dm_entry->ino = tmp->ino;
		dm_entry->offset = tmp->offset;
	}
	dm_info->index = index;
}

/*
 * NOTICE: hold dm->rw_sem before call this function;
 */
static bool hmfs_datamove_need_force_flush(struct f2fs_sb_info *sbi,
		unsigned int cur_sec)
{
	unsigned int free_secs = free_sections(sbi);
	unsigned int cur_dst_sec, tmp_src_sec;
	unsigned int cur_src_sec = NULL_SECNO;
	struct hmfs_dm_entry *entries[16];
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	unsigned long first_index = 0;
	int sec_count = 0;
	int i, ret;

	if (free_secs > hmfs_overprovision_sections(sbi))
		return false;

	while (1) {
		ret = radix_tree_gang_lookup(&dm->root, (void **)entries,
					first_index, ARRAY_SIZE(entries));
		if (!ret)
			break;

		first_index = entries[ret - 1]->dst_blkaddr + 1;

		for (i = 0; i < ret; i++) {
			cur_dst_sec = GET_SEC_FROM_SEG(sbi,
				GET_SEGNO(sbi, entries[i]->dst_blkaddr));
			tmp_src_sec = GET_SEC_FROM_SEG(sbi,
				GET_SEGNO(sbi, entries[i]->src_blkaddr));
			if (cur_sec == cur_dst_sec &&
					tmp_src_sec != cur_src_sec) {
				sec_count++;
				cur_src_sec = tmp_src_sec;
			}
			if (sec_count >= FREE_SEC_UNVERIFY)
				return true;
		}
	}

	return false;
}

/*
 * for data move
 * @verified_blkaddr:start block to be verified
 * @next_blkaddr: start block to write
 * ret: 0 success, others fail
 */
static int __hmfs_datamove_submit(struct f2fs_sb_info *sbi,
		block_t *verified_blkaddr, block_t *next_blkaddr,
		int dm_stream_id, bool force_flush, bool rescue,
		int *verify_fail)
{
	struct stor_dev_data_move_info data_move_info;
	struct stor_dev_data_move_source_addr *source_addrs;
	struct stor_dev_data_move_source_inode *source_inodes;
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_info *dm_info = &dm->dm_info[dm_stream_id];
	unsigned int array_size, i;
	struct hmfs_dm_entry *entries;
	struct curseg_info *curseg =
				CURSEG_I(sbi, CURSEG_DATA_MOVE1 + dm_stream_id);
	struct block_device *bdev;
	block_t dst_blkaddr;
	int ret = 0;

	f2fs_bug_on(sbi, dm_stream_id >= NR_CURSEG_DM_TYPE);
	f2fs_bug_on(sbi, force_flush && rescue);

	if (rescue) {
		f2fs_bug_on(sbi, dm_info->rescue_index == 0);
		array_size = dm_info->rescue_index;
		entries = dm_info->rescue_entries;
		source_addrs = f2fs_vzalloc(sizeof(struct stor_dev_data_move_source_addr) *
					array_size, GFP_NOFS);
		if (!source_addrs)
			return -ENOMEM;

		source_inodes = f2fs_vzalloc(sizeof(struct stor_dev_data_move_source_inode) *
					array_size, GFP_NOFS);
		if (!source_inodes) {
			vfree(source_addrs);
			return -ENOMEM;
		}
	} else {
		array_size = dm_info->index;
		entries = dm_info->entries;
		source_addrs = dm->source_addrs;
		source_inodes = dm->source_inodes;
	}

	dst_blkaddr = array_size ? entries[0].dst_blkaddr :
				NEXT_FREE_BLKADDR(sbi, curseg);
	bdev = hmfs_target_device(sbi, dst_blkaddr, NULL);

	for (i = 0; i < array_size; i++) {
		(source_addrs + i)->data_move_source_addr = entries[i].src_blkaddr;
		(source_addrs + i)->source_length = 1;
		(source_addrs + i)->src_lun = 0;
		(source_inodes + i)->data_move_source_inode = entries[i].ino;
		(source_inodes + i)->data_move_source_offset = entries[i].offset;
	}

	data_move_info.source_addr_num = array_size;
	data_move_info.source_inode_num = array_size;
	data_move_info.data_move_total_length = array_size;

	data_move_info.dest_4k_lba = dst_blkaddr;
	data_move_info.dest_stream_id = dm_stream_id;

	data_move_info.source_addr = source_addrs;
	data_move_info.source_inode = source_inodes;
	data_move_info.repeat_option = rescue ? 1 : 0;
	data_move_info.error_injection = 0;

	data_move_info.dest_blk_mode = TLC_MODE;
	data_move_info.dest_lun_info = 0;
	data_move_info.force_flush_option = force_flush ? 1 : 0;

	memset(&(data_move_info.verify_info), 0,
			sizeof(struct stor_dev_verify_info));

	ret = mas_blk_data_move(bdev, &data_move_info);
	if (ret) {
		hmfs_msg(sbi->sb, KERN_ERR, "gc mas_blk_data_move ret %d "
			"failed, verified_blkaddr %u, next_blkaddr %u\n", ret,
			data_move_info.verify_info.next_to_be_verify_4k_lba,
			data_move_info.verify_info.next_available_write_4k_lba);
		goto out;
	}

	*next_blkaddr =  data_move_info.verify_info.next_available_write_4k_lba;
	*verified_blkaddr = data_move_info.verify_info.next_to_be_verify_4k_lba;
	*verify_fail = data_move_info.verify_info.verify_done_status;
	if (*verify_fail)
		hmfs_msg(sbi->sb, KERN_ERR, "gc mas_blk_data_move verify "
				"failed fail_reason (%d)\n",
				data_move_info.verify_info.verify_fail_reason);
out:
	if (rescue) {
		vfree(source_addrs);
		vfree(source_inodes);
	}
	return ret;
}

/*
 * GC cannot be interrupted in one segment, so every dm cmd must
 * executes successfully. Retry is provided in __hmfs_datamove to
 * ensure that.
 */
static void __hmfs_datamove(struct f2fs_sb_info *sbi,
		block_t *verified_blkaddr, block_t *next_blkaddr,
		int dm_stream_id, bool force_flush, bool rescue,
		int *verify_fail)
{
	int fail_times = 0;
	int ret;

dm_fail:
	ret = __hmfs_datamove_submit(sbi, verified_blkaddr, next_blkaddr,
			dm_stream_id, force_flush, rescue, verify_fail);

	if (ret || (rescue && *verify_fail)) {
		fail_times++;
		if (fail_times > DM_RETRY_TIMES) {
			hmfs_stop_checkpoint(sbi, true);
			hmfs_msg(sbi->sb, KERN_ERR,
				"hmfs reboot for dm submit error! ret:%d rescue:%d vfail:%d\n",
				ret, rescue, *verify_fail);
#ifdef CONFIG_HUAWEI_HMFS_DSM
			if (hmfs_dclient && !dsm_client_ocuppy(hmfs_dclient)) {
				dsm_client_record(hmfs_dclient, "HMFS reboot: %s:%d [%d %d %d]\n",
						__func__, __LINE__, ret, rescue, *verify_fail);
				dsm_client_notify(hmfs_dclient, DSM_HMFS_NEED_FSCK);
			}
#endif
			f2fs_restart(); /* force restarting */
		}
		msleep(MIN_WT);
		goto dm_fail;
	}
}

void hmfs_datamove_drop_verified_entries(struct f2fs_sb_info *sbi,
			block_t verified_blkaddr, bool force)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_entry *entries[16];
	unsigned long first_index = 0;
	unsigned int secno, cur_sec;
	int i, ret;

	/*
	 * The address before verified_blkaddr has been verified.
	 * When it's at the end of one section (last block address + 1),
	 * it means this section has been verified. So we should minus 1
	 * to get current section.
	 */
	verified_blkaddr--;
	cur_sec = (verified_blkaddr < MAIN_BLKADDR(sbi)) ?
		GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, MAIN_BLKADDR(sbi))) :
		GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, verified_blkaddr));

	while (1) {
		ret = radix_tree_gang_lookup(&dm->root, (void **)entries,
					first_index, ARRAY_SIZE(entries));
		if (!ret)
			break;

		first_index = entries[ret - 1]->dst_blkaddr + 1;

		for (i = 0; i < ret; i++) {
			secno = GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, entries[i]->dst_blkaddr));

			/*
			 * Force drop will delete all entries of 2 DM streams. If use
			 * DM stream 2, we must check current section of DM stream.
			 * Delete entries by current section.
			 */
			if (force || (secno == cur_sec &&
					entries[i]->dst_blkaddr <= verified_blkaddr))
				__hmfs_remove_datamove_entry(dm, entries[i]->dst_blkaddr, entries[i]);
		}
	}
}

static inline void hmfs_check_blkaddr(struct f2fs_sb_info *sbi, block_t blkaddr)
{
	if ((blkaddr < MAIN_BLKADDR(sbi)) || (blkaddr > MAX_BLKADDR(sbi))) {
		hmfs_msg(sbi->sb, KERN_ERR, "dm illegal blkaddr %u\n", blkaddr);
		f2fs_bug_on(sbi, 1);
	}
}

void hmfs_datamove(struct f2fs_sb_info *sbi, int dm_stream_id,
					bool force_flush)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_info *dm_info = &dm->dm_info[dm_stream_id];
	struct hmfs_dm_entry *entries = dm_info->entries;
	block_t verified_blkaddr, next_blkaddr;
	block_t next_submit_blkaddr = 0;
	struct curseg_info *curseg =
				CURSEG_I(sbi, CURSEG_DATA_MOVE1 + dm_stream_id);
	unsigned int old_segno = curseg->segno;
	unsigned int old_blkoff = curseg->next_blkoff;
	int array_size = dm_info->index;
	int verify_fail;

	f2fs_bug_on(sbi, !force_flush &&
				array_size != HMFS_DATAMOVE_PU_SIZE(sbi));
	if (array_size != 0)
		next_submit_blkaddr = entries[array_size - 1].dst_blkaddr + 1;

	__hmfs_datamove(sbi, &verified_blkaddr, &next_blkaddr,
				dm_stream_id, force_flush, 0, &verify_fail);
	trace_hmfs_datamove(verified_blkaddr, next_blkaddr,
						array_size, force_flush);

	hmfs_check_blkaddr(sbi, verified_blkaddr);
	hmfs_check_blkaddr(sbi, next_blkaddr);
	dm_info->verified_blkaddr = verified_blkaddr;
	dm_info->next_blkaddr = next_blkaddr;
	hmfs_datamove_drop_verified_entries(sbi, dm_info->verified_blkaddr, 0);
	dm_info->index = 0;

	if (!verify_fail) {
		if (next_submit_blkaddr != next_blkaddr && !force_flush) {
			hmfs_msg(sbi->sb, KERN_ERR, "next_submit %u, "
				"next_blkaddr %u, verified_blkaddr %u, "
				"force flush %d, array size:%d\n",
				next_submit_blkaddr, next_blkaddr,
				verified_blkaddr, force_flush, array_size);
			f2fs_bug_on(sbi, 1);
		}
	} else {
		/*
		 * If dm return fail, fs will change current pos. Next_blkaddr
		 * is returned by dm. IF next_blkaddr == end of section, fs will
		 * use new section. IF next_blkaddr != end of section, fs will
		 * align with storage(in same sec).
		 */
		hmfs_msg(sbi->sb, KERN_INFO, "dm verify failed: "
				"stream id %d, verify_fail %ld, "
				"verified_blkaddr %u, next_blkaddr %u, "
				"cache_last %u, next_submit %u, dm_index %d\n",
				dm_stream_id, verify_fail, verified_blkaddr,
				next_blkaddr, dm_info->cached_last_blkaddr,
				next_submit_blkaddr, dm_info->index);

		hmfs_datamove_rescue(sbi, dm_stream_id, next_submit_blkaddr - 1);
	}

	if (force_flush || IS_LAST_DATA_BLOCK_IN_SEC(sbi,
			dm_info->verified_blkaddr - 1, DATA))
		dm_info->cached_last_blkaddr = 0;

	if (next_blkaddr != NEXT_FREE_BLKADDR(sbi, curseg)) {
		down_read(&SM_I(sbi)->curseg_lock);
		mutex_lock(&curseg->curseg_mutex);
		down_write(&SIT_I(sbi)->sentry_lock);
		/* align curseg next write addr with device */
		hmfs_datamove_change_curseg(sbi, curseg,
			CURSEG_DATA_MOVE1 + dm_stream_id, next_blkaddr);
		trace_hmfs_datamove_align(dm_stream_id,
			old_segno, old_blkoff, curseg->segno, curseg->next_blkoff);
		up_write(&SIT_I(sbi)->sentry_lock);
		mutex_unlock(&curseg->curseg_mutex);
		up_read(&SM_I(sbi)->curseg_lock);
	}
}

static void hmfs_datamove_rescue_fill_array(struct f2fs_sb_info *sbi,
		block_t next_submit_blkaddr, block_t cache_last_blkaddr, int dm_stream_id)
{
	struct hmfs_dm_entry *tmp, *dm_entry;
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_info *dm_info = &dm->dm_info[dm_stream_id];
	int index = 0;

	if (!cache_last_blkaddr)
		return;

	/* refill dm rescue submit array */
	while (index < DM_MAX_BLKS &&
			next_submit_blkaddr <= cache_last_blkaddr) {
		rcu_read_lock();
		tmp = radix_tree_lookup(&dm->root, next_submit_blkaddr++);
		rcu_read_unlock();
		/* can always find unverified entry */
		f2fs_bug_on(sbi, !tmp);

		dm_entry = &dm_info->rescue_entries[index++];
		dm_entry->src_blkaddr = tmp->src_blkaddr;
		dm_entry->dst_blkaddr = tmp->dst_blkaddr;
		dm_entry->ino = tmp->ino;
		dm_entry->offset = tmp->offset;
	}
	dm_info->rescue_index = index;
}

/*
 * Add rescue segments into rescue list, these rescue segments must
 * be moved by gc at first. "start" and "end" indicate lba range
 * of rescue segments.
 */
static void add_rescue_segs_list(struct f2fs_sb_info *sbi,
			block_t start, block_t end)
{
	struct rescue_seg_entry *rs_entry = NULL;
	int start_seg, end_seg;
	int i;

	start_seg = GET_SEGNO(sbi, start);
	end_seg = GET_SEGNO(sbi, end);

	for (i = start_seg; i <= end_seg; i++) {
		rs_entry = f2fs_kvmalloc(sbi, sizeof(struct rescue_seg_entry),
				GFP_NOFS | __GFP_NOFAIL);
		rs_entry->segno = i;
		list_add(&(rs_entry->list),
				&(sbi->gc_control_info.rescue_segment_list));
	}
}

void hmfs_datamove_rescue(struct f2fs_sb_info *sbi, int dm_stream_id,
							block_t last_addr)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_info *dm_info = &dm->dm_info[dm_stream_id];
	block_t next_submit_blkaddr, verified_blkaddr, next_blkaddr;
	int array_size, need_datamove_size;
	bool submit_once;
	int verify_fail;

	if ((DATA_BLOCK_IN_SAME_SEC(sbi, dm_info->verified_blkaddr, last_addr) ||
		 DATA_BLOCK_IN_SAME_SEC(sbi, dm_info->verified_blkaddr - 1, last_addr)) &&
		 dm_info->verified_blkaddr > last_addr) {
		hmfs_datamove_drop_verified_entries(sbi, dm_info->verified_blkaddr, 1);
		return;
	}

	if (IS_LAST_DATA_BLOCK_IN_SEC(sbi, dm_info->verified_blkaddr - 1, DATA))
		next_submit_blkaddr = START_BLOCK(sbi, GET_SEG_FROM_SEC(sbi,
			GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, last_addr))));
	else
		next_submit_blkaddr = dm_info->verified_blkaddr;

	need_datamove_size = last_addr - next_submit_blkaddr + 1;
	if (need_datamove_size <= DM_MAX_BLKS) {
		array_size = need_datamove_size;
		submit_once = true;
	} else {
		array_size = DM_REPEAT_BLKS;
		submit_once = false;
	}
	hmfs_msg(sbi->sb, KERN_INFO, "dm rescue: "
			"verified_blkaddr:%u, last_addr:%u, "
			"next_submit_blkaddr:%u, need_datamove_size:%u, "
			"array_size:%u", dm_info->verified_blkaddr,
			last_addr, next_submit_blkaddr,
			need_datamove_size, array_size);

	dm_info->rescue_entries = f2fs_kvzalloc(sbi,
			sizeof(struct hmfs_dm_entry) * array_size,
			GFP_NOFS | __GFP_NOFAIL);

	if (!submit_once)
		array_size = sbi->blocks_per_seg -
				GET_BLKOFF_FROM_SEG0(sbi, next_submit_blkaddr);

dm_retry:
	if (need_datamove_size == 0)
		goto dm_done;

	hmfs_datamove_rescue_fill_array(sbi, next_submit_blkaddr,
			next_submit_blkaddr + array_size - 1, dm_stream_id);
	f2fs_bug_on(sbi, array_size != dm_info->rescue_index);

	__hmfs_datamove(sbi, &verified_blkaddr, &next_blkaddr,
			dm_stream_id, 0, 1, &verify_fail);
	add_rescue_segs_list(sbi, next_submit_blkaddr,
			next_submit_blkaddr + array_size - 1);

	/* in recsue mode, all data has been verified */
	next_submit_blkaddr =
			dm_info->rescue_entries[array_size - 1].dst_blkaddr + 1;
	dm_info->verified_blkaddr = next_submit_blkaddr;
	dm_info->rescue_index = 0;
	need_datamove_size = need_datamove_size - array_size;

	if (need_datamove_size <= DM_REPEAT_BLKS)
		array_size = need_datamove_size;
	else
		array_size = DM_REPEAT_BLKS;
	hmfs_msg(sbi->sb, KERN_INFO, "dm rescue: need_datamove_size:%u, "
			"array_size:%u", need_datamove_size, array_size);
	if (need_datamove_size)
		goto dm_retry;

dm_done:
	/*
	 * After repeat mode, all addr in tree must be verified,
	 * so we clear out tree.
	 */
	dm_info->index = 0;
	dm_info->rescue_index = 0;
	hmfs_datamove_drop_verified_entries(sbi, dm_info->verified_blkaddr, 1);
	dm_info->cached_last_blkaddr = 0;
	kvfree(dm_info->rescue_entries);
	dm_info->rescue_entries = NULL;
}

void hmfs_datamove_force_flush(struct f2fs_sb_info *sbi,
		struct cp_control *cpc)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_info *dm_info;
	block_t verified_blkaddr;
	bool force_flush = false;
	unsigned int cur_sec;
	int i;

	if (!is_set_ckpt_flags(sbi, CP_DATAMOVE_FLAG))
		return;

	if (!hmfs_datamove_on(sbi))
		return;

	down_write(&dm->rw_sem);
	for (i = 0; i < NR_CURSEG_DM_TYPE; i++) {
		if (cpc->reason & CP_UMOUNT) {
			force_flush = true;
		} else {
			dm_info = &dm->dm_info[i];
			verified_blkaddr = dm_info->verified_blkaddr;
			cur_sec = (verified_blkaddr >= MAX_BLKADDR(sbi)) ?
					GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, MAX_BLKADDR(sbi) - 1)) :
					GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, verified_blkaddr));
			force_flush = hmfs_datamove_need_force_flush(sbi, cur_sec);
		}

		if (force_flush)
			hmfs_datamove(sbi, i, true);
	}
	up_write(&dm->rw_sem);
}

void hmfs_datamove_add_tree_entry(struct f2fs_sb_info *sbi, block_t src,
			block_t dst, unsigned int ino, unsigned int offset)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_entry *e;
	struct hmfs_dm_entry *orig = NULL;
	int ret;

	e = kmalloc(sizeof(struct hmfs_dm_entry),
			GFP_NOFS | __GFP_NOFAIL);
	if (!e) {
		hmfs_msg(sbi->sb, KERN_ERR, "kmalloc fail");
		f2fs_bug_on(sbi, 1);
	}

	ret = radix_tree_preload(GFP_NOFS | __GFP_NOFAIL);
	if (ret) {
		hmfs_msg(sbi->sb, KERN_ERR, "radix_tree_preload fail");
		f2fs_bug_on(sbi, 1);
	}

	orig = radix_tree_lookup(&dm->root, (unsigned long)src);
	if (orig) {
		hmfs_msg(sbi->sb, KERN_ERR, "radix_tree_lookup: src %u, "
			"dst %u, verify %u, next %u, cplast %u",
			src, dst, dm->dm_info[0].verified_blkaddr,
			dm->dm_info[0].next_blkaddr,
			dm->dm_info[0].cached_last_blkaddr);
		f2fs_bug_on(sbi, 1);
	}

	e->src_blkaddr = src;
	e->dst_blkaddr = dst;

	/*
	 * It should not fail here because we already allocate memory for
	 * node insertion and dst block is a new block, it is impossible
	 * that it is in the tree.
	 */
	ret = radix_tree_insert(&dm->root, dst, e);
	if (ret) {
		hmfs_msg(sbi->sb, KERN_ERR, "radix_tree_insert: src %u, "
			"dst %u, verify %u, next %u, cplast %u",
			src, dst, dm->dm_info[0].verified_blkaddr,
			dm->dm_info[0].next_blkaddr,
			dm->dm_info[0].cached_last_blkaddr);
		f2fs_bug_on(sbi, 1);
	}

	atomic_inc(&dm->count);
	radix_tree_preload_end();
}

bool hmfs_datamove_check_discard(struct f2fs_sb_info *sbi,
			unsigned int start_seg, unsigned int end_seg,
			unsigned long *bitmap)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_entry *entries[16];
	unsigned long first_index = 0;
	block_t start_blkaddr, end_blkaddr;
	bool flag = false;
	unsigned int set_segno;
	int i, ret;

	if (!hmfs_datamove_on(sbi))
		return false;

	start_blkaddr = START_BLOCK(sbi, start_seg);
	end_blkaddr = start_blkaddr +
		((end_seg - start_seg) << sbi->log_blocks_per_seg);

	down_read(&dm->rw_sem);
	while (1) {
		ret = radix_tree_gang_lookup(&dm->root, (void **)entries,
					first_index, ARRAY_SIZE(entries));
		if (!ret)
			break;

		first_index = entries[ret - 1]->dst_blkaddr + 1;

		for (i = 0; i < ret; i++) {
			if (entries[i]->src_blkaddr >= start_blkaddr &&
						entries[i]->src_blkaddr < end_blkaddr) {
				flag = true;
				set_segno = GET_SEGNO(sbi,
					entries[i]->src_blkaddr) - start_seg;

				if (set_segno >= 0 && set_segno <
							end_seg - start_seg && bitmap)
					set_bit(set_segno, bitmap);
			}
		}
	}
	up_read(&dm->rw_sem);

	return flag;
}

void __hmfs_datamove_add_entry(struct f2fs_sb_info *sbi, block_t src,
		block_t dst, enum page_type type, int dm_stream_id)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_info *dm_info = &dm->dm_info[dm_stream_id];
	struct hmfs_dm_entry *dm_array = dm_info->entries;
	struct hmfs_dm_entry *e;

	f2fs_bug_on(sbi, !is_set_ckpt_flags(sbi, CP_DATAMOVE_FLAG));
	down_write(&dm->rw_sem);
	hmfs_datamove_add_tree_entry(sbi, src, dst, 0, 0);
	if (dst > dm_info->cached_last_blkaddr ||
			!DATA_BLOCK_IN_SAME_SEC(sbi, dst, dm_info->cached_last_blkaddr)) {
		dm_info->cached_last_blkaddr = dst;
	}
	f2fs_bug_on(sbi, dm_info->index > HMFS_DATAMOVE_PU_SIZE(sbi));

	if (dm_info->index < HMFS_DATAMOVE_PU_SIZE(sbi)) {
		e = dm_array + dm_info->index;
		e->src_blkaddr = src;
		e->dst_blkaddr = dst;
		dm_info->index++;
	}

	if (dm_info->index >= HMFS_DATAMOVE_PU_SIZE(sbi))
		hmfs_datamove(sbi, dm_stream_id, false);

	up_write(&dm->rw_sem);
}

void __hmfs_datamove_remove_entry(struct f2fs_sb_info *sbi,
		block_t blkaddr)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_entry *e;

	rcu_read_lock();
	e = radix_tree_lookup(&dm->root, blkaddr);
	rcu_read_unlock();
	if (!e)
		return;

	down_write(&dm->rw_sem);
	e = radix_tree_lookup(&dm->root, blkaddr);
	if (e)
		__hmfs_remove_datamove_entry(dm, blkaddr, e);

	up_write(&dm->rw_sem);
}

bool hmfs_datamove_get_mapped_blk(struct f2fs_sb_info *sbi,
		block_t dst_blk, block_t *src_blk)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_entry *e;

	if (!hmfs_datamove_on(sbi))
		return false;

	if (dst_blk < MAIN_BLKADDR(sbi))
		return false;

	rcu_read_lock();
	e = radix_tree_lookup(&dm->root, dst_blk);
	rcu_read_unlock();
	if (!e)
		return false;

	down_read(&dm->rw_sem);
	e = radix_tree_lookup(&dm->root, dst_blk);
	if (!e) {
		up_read(&dm->rw_sem);
		return false;
	}

	f2fs_bug_on(sbi, dst_blk != e->dst_blkaddr);

	if (src_blk)
		*src_blk = e->src_blkaddr;
	atomic_inc(&dm->refs);
	up_read(&dm->rw_sem);
	return true;
}

void hmfs_datamove_put_mapped_blk(struct f2fs_sb_info *sbi)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);

	if (likely(!atomic_dec_and_test(&dm->refs)))
		return;

	wake_up(&dm->wait);
}

void hmfs_datamove_add_to_bio(struct f2fs_sb_info *sbi, struct bio *bio)
{
	struct hmfs_bio *hio = HMFS_BIO(bio);

	if (!(hio->flags & HMFS_BIO_DM_REF)) {
		hio->flags |= HMFS_BIO_DM_REF;
		hio->sbi = sbi;
	}

	hio->mapped_blks++;
}

void hmfs_datamove_check_mapped_blk(struct f2fs_sb_info *sbi,
		struct bio *bio, block_t dst_blk)
{
	block_t src_blk = dst_blk;
	bool blk_mapped;

	if (!hmfs_datamove_on(sbi))
		return;

	blk_mapped = hmfs_datamove_get_mapped_blk(sbi, dst_blk, &src_blk);
	if (likely(!blk_mapped))
		return;

	hmfs_target_device(sbi, src_blk, bio);
	hmfs_datamove_add_to_bio(sbi, bio);
}

static int hmfs_write_datamove_entry(struct f2fs_sb_info *sbi,
		struct page *page, block_t blkaddr)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	unsigned int offset;
	struct hmfs_datamove_entry *entry;
	struct hmfs_dm_entry *entries[16];
	unsigned long first_index = 0;
	unsigned char *kaddr;
	int i, ret;
	int nblock = 0;

	kaddr = (unsigned char *)page_address(page);
	offset = DM_HEAD_RESV_SIZE;

	down_read(&dm->rw_sem);
	while (1) {
		ret = radix_tree_gang_lookup(&dm->root, (void **)entries,
					first_index, ARRAY_SIZE(entries));
		if (!ret)
			break;

		first_index = entries[ret - 1]->dst_blkaddr + 1;

		for (i = 0; i < ret; i++) {
			if (!page) {
				page = hmfs_grab_meta_page(sbi, blkaddr++);
				nblock++;
				kaddr = (unsigned char *)page_address(page);
				memset(kaddr, 0, PAGE_SIZE);
				offset = 0;
			}
			entry = (struct hmfs_datamove_entry *)(kaddr + offset);
			entry->src_blkaddr = cpu_to_le32(entries[i]->src_blkaddr);
			entry->dst_blkaddr = cpu_to_le32(entries[i]->dst_blkaddr);
			offset += sizeof(struct hmfs_datamove_entry);
			if (offset + sizeof(struct hmfs_datamove_entry) <= PAGE_SIZE)
				continue;

			set_page_dirty(page);
			f2fs_put_page(page, 1);
			page = NULL;
		}
	}
	up_read(&dm->rw_sem);

	if (page) {
		set_page_dirty(page);
		f2fs_put_page(page, 1);
	}

	return nblock;
}

static void hmfs_fill_datamove_info(struct f2fs_sb_info *sbi,
		struct page *page)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_datamove_block *dm_block = NULL;
	unsigned char *kaddr = NULL;
	struct hmfs_datamove_info *datamove_info = NULL;
	struct hmfs_dm_info *dm_info = NULL;
	int i;

	kaddr = (unsigned char *)page_address(page);
	memset(kaddr, 0, PAGE_SIZE);
	dm_block = (struct hmfs_datamove_block *)(kaddr);
	dm_block->head.channels = cpu_to_le32(NR_CURSEG_DM_TYPE);
	dm_block->head.count = cpu_to_le32(atomic_read(&dm->count));

	for (i = 0 ; i < NR_CURSEG_DM_TYPE; i++) {
		dm_info = &dm->dm_info[i];
		datamove_info = &(dm_block->head.dm_info[i]);
		datamove_info->verified_blkaddr = cpu_to_le32(dm_info->verified_blkaddr);
		datamove_info->next_blkaddr = cpu_to_le32(dm_info->next_blkaddr);
		datamove_info->cached_last_blkaddr = cpu_to_le32(dm_info->cached_last_blkaddr);
	}
}

int hmfs_datamove_write_entry(struct f2fs_sb_info *sbi, block_t blkaddr)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_info *dm_info0 = &dm->dm_info[0];
	struct hmfs_dm_info *dm_info1 = &dm->dm_info[1];
	struct page *page = NULL;
	int nblock = 1;

	if (!is_set_ckpt_flags(sbi, CP_DATAMOVE_FLAG))
		return 0;

	if (!hmfs_datamove_on(sbi))
		return 0;

	page = hmfs_grab_meta_page(sbi, blkaddr++);
	if (!page)
		return 0;

	hmfs_fill_datamove_info(sbi, page);
	nblock += hmfs_write_datamove_entry(sbi, page, blkaddr);

	trace_hmfs_datamove_write_cp(free_sections(sbi),
			atomic_read(&dm->count),
			nblock,
			dm_info0->verified_blkaddr,
			dm_info1->verified_blkaddr,
			dm_info0->next_blkaddr,
			dm_info1->next_blkaddr,
			dm_info0->cached_last_blkaddr,
			dm_info1->cached_last_blkaddr);

	return nblock;
}


void hmfs_datamove_tree_print_info(struct f2fs_sb_info *sbi,
					unsigned int *unverify_sec,
					unsigned int *unverify_free_sec)
{
	struct hmfs_dm_manager *dm = HMFS_DM(sbi);
	struct hmfs_dm_entry *entries[16];
	unsigned long first_index = 0;
	unsigned int set_secno;
	unsigned int tmp_secno = NULL_SECNO;
	unsigned int used_sec = 0;
	unsigned int free_sec = 0;
	unsigned int valid_blocks;
	int i, ret;

	while (1) {
		ret = radix_tree_gang_lookup(&dm->root, (void **)entries,
					first_index, ARRAY_SIZE(entries));
		if (!ret)
			break;

		first_index = entries[ret - 1]->dst_blkaddr + 1;

		for (i = 0; i < ret; i++) {
			set_secno = GET_SEC_FROM_SEG(sbi,
				GET_SEGNO(sbi, entries[i]->src_blkaddr));
			if (set_secno != tmp_secno) {
				used_sec++;
				valid_blocks = get_valid_blocks(sbi,
						GET_SEGNO(sbi, entries[i]->src_blkaddr), true);
				if (!valid_blocks)
					free_sec++;
				tmp_secno = set_secno;
			}
		}
	}

	if (unverify_sec)
		*unverify_sec = used_sec;
	if (unverify_free_sec)
		*unverify_free_sec = free_sec;

	hmfs_msg(sbi->sb, KERN_INFO,
		"unverify_used_sec = %d  entry_count=%d\n",
		used_sec, atomic_read(&dm->count));
}
