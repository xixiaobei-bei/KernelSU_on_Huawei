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
#include <linux/f2fs_fs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/timer.h>
#include <linux/blkdev.h>
#include <linux/sched/signal.h>
#include <linux/version.h>

#include "f2fs.h"
#include "node.h"
#include "segment.h"
#include "gc.h"
#include <trace/events/f2fs.h>

#ifdef CONFIG_F2FS_TURBO_ZONE
#include "turbo_zone.h"
#define MIGRATE_FILE_MAX_SCHEDULE 5000
#endif

#define IDLE_WT	1000
#define MIN_WT	1000
#define DEF_GC_BALANCE_MIN_SLEEP_TIME	10000 /* milliseconds */
#define DEF_GC_FRAG_MIN_SLEEP_TIME	1000	/* milliseconds */
#define GC_URGENT_DISABLE_BLKS		(32 << 18)	/* 32G */
#define GC_URGENT_SPACE			(10 << 18)	/* 10G */
#define GC_URGENT_INTERVAL		(10 * 60 * 1000)	/* 10 mins */
#define BLOCK_COUNT_TYPE		 10   /* 10 kinds of successive blocks */
#define GC_THRESHOLD			30  /* gc threshold 30% percents */

extern int find_next_free_extent(const unsigned long *addr,
			  unsigned long size, unsigned long *offset);

static bool __is_frag_urgent(struct f2fs_sb_info *sbi)
{
	unsigned int total_segs =
			le32_to_cpu(sbi->raw_super->segment_count_main);
	unsigned int i;
	unsigned int block_count[BLOCK_COUNT_TYPE];
	unsigned int tot_blocks = 0;
	u64 total_blocks = le64_to_cpu(sbi->raw_super->block_count);
	unsigned int valid_blocks = sbi->total_valid_block_count;

	if(total_blocks < GC_URGENT_DISABLE_BLKS)
		return false;

	if(total_blocks - valid_blocks > GC_URGENT_SPACE)
		return false;

	memset(block_count, 0, sizeof(block_count));
	for (i = 0; i < total_segs; i++) {
		struct seg_entry *se = get_seg_entry(sbi, i);
		unsigned long start = 0;
		int blocks, index;

		if (se->valid_blocks == 0) {
			block_count[9] += sbi->blocks_per_seg;
			continue;
		} else if (se->valid_blocks == sbi->blocks_per_seg) {
			continue;
		}

		while (start < sbi->blocks_per_seg) {
			blocks = find_next_free_extent((unsigned long *)se->cur_valid_map,
						       sbi->blocks_per_seg,
						       &start);
			if (unlikely(blocks < 0))
				break;

			index = ilog2(blocks);
			if (unlikely(index >= BLOCK_COUNT_TYPE)) {
				index = 9;
				set_sbi_flag(sbi, SBI_NEED_FSCK);
			}

			block_count[index] += blocks;
			tot_blocks += blocks;
		}
		cond_resched();
	}
	for (i = 0; i < BLOCK_COUNT_TYPE; i++)
		f2fs_msg(sbi->sb, KERN_NOTICE,
				"block_cnt[%d]: %d\n", i, block_count[i]);

	/*
	 * It's a highly fragmented state of fs space if
	 * (4k + 8k) extent free blocks > 1/2 free blocks in dirty segments
	 * or 4k extent free blocks > 1/4 free blocks in dirty segments,
	 * and GC is urgently needed.
	 */
	if ((block_count[0] + block_count[1]) >= (tot_blocks >> 1) ||
			block_count[0] >= (tot_blocks >> 2))
		return true;

	return false;
}

static bool is_frag_urgent(struct f2fs_sb_info *sbi)
{
	unsigned long next_check = sbi->last_urgent_check +
					msecs_to_jiffies(GC_URGENT_INTERVAL);
	if (time_after(jiffies, next_check)) {
		sbi->last_urgent_check = jiffies;
		sbi->is_frag = __is_frag_urgent(sbi);
	}
	return sbi->is_frag;
}

/* if invaild blocks in dirty segments is more then 10% of total free space */
static inline int is_reclaimable_dirty_blocks_enough(struct f2fs_sb_info *sbi)
{
	s64 total_free_blocks;
	s64 total_dirty_blocks;

	total_free_blocks = sbi->user_block_count - written_block_count(sbi);
	total_dirty_blocks = total_free_blocks - free_user_blocks(sbi);

	if (total_dirty_blocks <= 0)
		return 0;

	if ((total_dirty_blocks * 10) <= total_free_blocks)
		return 0;

	return 1;
}

/*
 * GC tuning ratio [0, 100] in performance mode
 */
static inline int gc_perf_ratio(struct f2fs_sb_info *sbi)
{
	block_t reclaimable_user_blocks = sbi->user_block_count -
		written_block_count(sbi);
	return reclaimable_user_blocks == 0 ? 100 :
		(100ULL * free_user_blocks(sbi) / reclaimable_user_blocks);
}

static inline int __gc_thread_wait_timeout(struct f2fs_sb_info *sbi, struct f2fs_gc_kthread *gc_th,
       int timeout)
{
	wait_queue_head_t *wq = &gc_th->gc_wait_queue_head;
	wait_queue_head_t *fg_gc_wait = &gc_th->fg_gc_wait;

	return wait_event_interruptible_timeout(*wq,
		gc_th->gc_wake || freezing(current) ||
		kthread_should_stop() || waitqueue_active(fg_gc_wait) ||
		atomic_read(&sbi->need_ssr_gc), timeout);
}

static int gc_thread_func(void *data)
{
	struct f2fs_sb_info *sbi = data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	unsigned int wait_ms;

	wait_ms = gc_th->min_sleep_time;
	current->flags |= PF_MUTEX_GC;
	set_freezable();
	do {
		int ret;

		/* lint -save -e574 -e666 */
		if (!is_gc_test_set(sbi, GC_TEST_DISABLE_FRAG_URGENT) &&
			free_segments(sbi) < 3 * overprovision_segments(sbi) &&
			is_reclaimable_dirty_blocks_enough(sbi) &&
			is_frag_urgent(sbi))
			gc_th->gc_preference = GC_FRAG;
		else if (100ULL * written_block_count(sbi) / sbi->user_block_count > 90)
			gc_th->gc_preference = GC_LIFETIME;
		else if (gc_perf_ratio(sbi) < 10 && free_segments(sbi) <
			3 * overprovision_segments(sbi))
			gc_th->gc_preference = GC_PERF;
		else
			gc_th->gc_preference = GC_BALANCE;

		if (gc_th->gc_preference == GC_PERF)
			wait_ms = max(DEF_GC_BALANCE_MIN_SLEEP_TIME *
				gc_perf_ratio(sbi) / 100, MIN_WT);
		else if (gc_th->gc_preference == GC_BALANCE)
			gc_th->min_sleep_time = DEF_GC_BALANCE_MIN_SLEEP_TIME;
		else if (gc_th->gc_preference == GC_FRAG)
			wait_ms = DEF_GC_FRAG_MIN_SLEEP_TIME;
		else
			gc_th->min_sleep_time = DEF_GC_THREAD_MIN_SLEEP_TIME;

		if (time_to_inject(sbi, FAULT_CHECKPOINT)) {
			f2fs_show_injection_info(FAULT_CHECKPOINT);
			f2fs_stop_checkpoint(sbi, false);
		}

		ret = __gc_thread_wait_timeout(sbi, gc_th,
				msecs_to_jiffies(wait_ms));

		if (gc_th->gc_wake)
			gc_th->gc_wake = 0;

		if (!sb_start_write_trylock(sbi->sb)) {
			if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
				stat_other_skip_bggc_count(sbi);
			continue;
		}

		if (!ret) {
			if (sbi->sb->s_writers.frozen >= SB_FREEZE_WRITE) {
				increase_sleep_time(gc_th, &wait_ms);
				if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
					stat_other_skip_bggc_count(sbi);
				goto next;
			}

			if (!mutex_trylock(&sbi->gc_mutex)) {
				if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
					stat_other_skip_bggc_count(sbi);
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
			f2fs_msg(sbi->sb, KERN_ERR, "f2fs-gc: some signals have been received...\n");
			goto next;
		} else {
			int ssr_gc_count;
			ssr_gc_count = atomic_read(&sbi->need_ssr_gc);
			if (ssr_gc_count) {
				mutex_lock(&sbi->gc_mutex);
#ifdef CONFIG_F2FS_TURBO_ZONE
				f2fs_gc(sbi, true, false, false, NULL_SEGNO);
#else
				f2fs_gc(sbi, true, false, NULL_SEGNO);
#endif
				atomic_sub(ssr_gc_count, &sbi->need_ssr_gc);
			}
			if (!has_not_enough_free_secs(sbi, 0, 0)) {
				wake_up_all(&gc_th->fg_gc_wait);
				goto next;
			}

			/* run into FG_GC
			   we must wait & take sbi->gc_mutex before FG_GC */
			mutex_lock(&sbi->gc_mutex);
#ifdef CONFIG_F2FS_TURBO_ZONE
			f2fs_gc(sbi, false, false, false, NULL_SEGNO);
#else
			f2fs_gc(sbi, false, false, NULL_SEGNO);
#endif
			wake_up_all(&gc_th->fg_gc_wait);
			goto next;
		}

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
			wait_ms = gc_th->urgent_sleep_time;
			goto do_gc;
		}

#ifdef CONFIG_MAS_BLK
		if (!gc_th->block_idle &&
			!is_gc_test_set(sbi, GC_TEST_DISABLE_IO_AWARE)) {
#else
		if (!is_idle(sbi, GC_TIME) &&
			!is_gc_test_set(sbi, GC_TEST_DISABLE_IO_AWARE)) {
#endif
			increase_sleep_time(gc_th, &wait_ms);
			if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
				stat_io_skip_bggc_count(sbi);
			mutex_unlock(&sbi->gc_mutex);
			goto next;
		}

		if (has_enough_invalid_blocks(sbi))
			decrease_sleep_time(gc_th, &wait_ms);
		else
			increase_sleep_time(gc_th, &wait_ms);
do_gc:
		stat_inc_bggc_count(sbi);

#ifdef CONFIG_F2FS_STAT_FS
		f2fs_msg(sbi->sb, KERN_NOTICE,
			"BG_GC: Size=%lluMB,Free=%lluMB,count=%d,free_sec=%u,reserved_sec=%u,node_secs=%d,dent_secs=%d\n",
			((u64)(sbi->user_block_count) * sbi->blocksize) / 1024 / 1024,
			((u64)(sbi->user_block_count - valid_user_blocks(sbi)) * sbi->blocksize) / 1024 / 1024,
			sbi->bg_gc, free_sections(sbi), reserved_sections(sbi),
			get_blocktype_secs(sbi, F2FS_DIRTY_NODES), get_blocktype_secs(sbi, F2FS_DIRTY_DENTS));
#endif

		/* if return value is not zero, no victim was selected */
#ifdef CONFIG_F2FS_TURBO_ZONE
		if (f2fs_gc(sbi, test_opt(sbi, FORCE_FG_GC), true,
							false, NULL_SEGNO))
#else
		if (f2fs_gc(sbi, test_opt(sbi, FORCE_FG_GC), true, NULL_SEGNO))
#endif
			wait_ms = gc_th->no_gc_sleep_time;

		trace_f2fs_background_gc(sbi->sb, wait_ms,
				prefree_segments(sbi), free_segments(sbi));

		/* balancing f2fs's metadata periodically */
		f2fs_balance_fs_bg(sbi);
next:
		sb_end_write(sbi->sb);
	} while (!kthread_should_stop());
	return 0;
}

#ifdef CONFIG_MAS_BLK
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static void set_block_idle(struct timer_list *timer)
{
	struct f2fs_gc_kthread *gc_th = (struct f2fs_gc_kthread *)
		from_timer(gc_th, timer, nb_timer);

	gc_th->block_idle = true;
}
#else
static void set_block_idle(unsigned long data)
{
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;

	gc_th->block_idle = true;
}
#endif

static enum blk_busyidle_callback_ret gc_io_busyidle_notify_handler(
			struct blk_busyidle_event_node *event_node,
			enum blk_idle_notify_state state)
{
	enum blk_busyidle_callback_ret ret = BUSYIDLE_NO_IO;
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)event_node->param_data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;

	if (gc_th->f2fs_gc_task == NULL)
		return ret;
	switch (state) {
	case BLK_IDLE_NOTIFY:
		mod_timer(&gc_th->nb_timer, jiffies + msecs_to_jiffies(IDLE_WT));
		ret = BUSYIDLE_NO_IO;
		break;
	case BLK_BUSY_NOTIFY:
		del_timer_sync(&gc_th->nb_timer);
		gc_th->block_idle = false;
		ret = BUSYIDLE_NO_IO;
		break;
	}

	return ret;
}
#endif

void f2fs_init_gc_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;

	init_waitqueue_head(&gc_th->gc_wait_queue_head);
	init_waitqueue_head(&gc_th->fg_gc_wait);
}

int f2fs_start_gc_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	struct task_struct *f2fs_gc_task;
	dev_t dev = sbi->sb->s_bdev->bd_dev;
	int err = 0;

	gc_th->f2fs_gc_task = NULL;

	gc_th->urgent_sleep_time = DEF_GC_THREAD_URGENT_SLEEP_TIME;
	gc_th->min_sleep_time = DEF_GC_BALANCE_MIN_SLEEP_TIME;
	gc_th->max_sleep_time = DEF_GC_THREAD_MAX_SLEEP_TIME;
	gc_th->no_gc_sleep_time = DEF_GC_THREAD_NOGC_SLEEP_TIME;

	gc_th->gc_wake= 0;
	gc_th->gc_preference = GC_BALANCE;

#ifdef CONFIG_MAS_BLK
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	timer_setup(&gc_th->nb_timer, set_block_idle, 0);
#else
	setup_timer(&gc_th->nb_timer, set_block_idle, (unsigned long)sbi);
#endif
	gc_th->block_idle = false;
	strncpy(gc_th->gc_event_node.subscriber, "f2fs_gc", SUBSCRIBER_NAME_LEN);
	gc_th->gc_event_node.busyidle_callback = gc_io_busyidle_notify_handler;
	gc_th->gc_event_node.param_data = sbi;
	err = blk_busyidle_event_subscribe(sbi->sb->s_bdev, &gc_th->gc_event_node);
#endif

	f2fs_gc_task = kthread_run(gc_thread_func, sbi,
		"f2fs_gc-%u:%u", MAJOR(dev), MINOR(dev));
	if (IS_ERR(f2fs_gc_task))
		err = PTR_ERR(f2fs_gc_task);
        else
		gc_th->f2fs_gc_task = f2fs_gc_task;

	return err;
}

void f2fs_stop_gc_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	if (gc_th->f2fs_gc_task != NULL) {
		kthread_stop(gc_th->f2fs_gc_task);
		gc_th->f2fs_gc_task = NULL;
		wake_up_all(&gc_th->fg_gc_wait);
#ifdef CONFIG_MAS_BLK
		del_timer_sync(&gc_th->nb_timer);
retry:
		if (blk_busyidle_event_unsubscribe(&gc_th->gc_event_node))
			goto retry;
#endif
	}
}

#ifdef CONFIG_F2FS_TURBO_ZONE
/* Copied from gc_thread_func() */
static int gc_thread_turbo_func(void *data)
{
	struct f2fs_sb_info *sbi = data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_turbo_thread;
	unsigned int wait_ms;
	struct f2fs_tz_info *tz_info = &sbi->tz_info;
	int ret;
	int ssr_gc_count;

	wait_ms = gc_th->min_sleep_time;

	current->flags |= PF_MUTEX_GC;

	set_freezable();

	do {
		if (!sb_start_write_trylock(sbi->sb))
			continue;

		/*lint -save -e454 -e456 -e666*/
		ret = __gc_thread_wait_timeout(sbi, gc_th,
					msecs_to_jiffies(wait_ms));
		if (gc_th->gc_wake)
			gc_th->gc_wake = 0;

		if (!ret) {
			if (sbi->sb->s_writers.frozen >= SB_FREEZE_WRITE) {
				increase_sleep_time(gc_th, &wait_ms);
				goto next;
			}

			if (!mutex_trylock(&sbi->gc_mutex))
				goto next;

		} else if (try_to_freeze()) {
			goto next;
		} else if (kthread_should_stop()) {
			sb_end_write(sbi->sb);
			break;
		} else if (ret < 0) {
			pr_err("f2fs-gc-turbo: some signals received\n");
			goto next;
		} else {
			ssr_gc_count = atomic_read(&sbi->need_ssr_gc);
			if (ssr_gc_count) {
				mutex_lock(&sbi->gc_mutex);
				f2fs_gc(sbi, true, false, true, NULL_SEGNO);
				atomic_sub(ssr_gc_count, &sbi->need_ssr_gc);
			}
			if (!has_not_enough_free_secs(sbi, 0, 0)) {
				wake_up_all(&gc_th->fg_gc_wait);
				goto next;
			}

			/* run into FG_GC
			 * we must wait & take sbi->gc_mutex before FG_GC
			 */
			mutex_lock(&sbi->gc_mutex);

			f2fs_gc(sbi, false, false, true, NULL_SEGNO);
			wake_up_all(&gc_th->fg_gc_wait);
			goto next;
		}

		if (gc_th->gc_urgent) {
			wait_ms = gc_th->urgent_sleep_time;
			goto do_gc;
		}

#ifdef CONFIG_MAS_BLK
		if (!gc_th->block_idle) {
#else
		if (!is_idle(sbi, GC_TIME)) {
#endif
			increase_sleep_time(gc_th, &wait_ms);
			mutex_unlock(&sbi->gc_mutex);
			goto next;
		}

		if (has_enough_tz_invalid_blocks(sbi))
			decrease_sleep_time(gc_th, &wait_ms);
		else
			increase_sleep_time(gc_th, &wait_ms);
do_gc:
		stat_inc_bggc_count(sbi);
		tz_info->turbo_bg_gc++;

		/* if return value is not zero, no victim was selected */
		if (f2fs_gc(sbi, test_opt(sbi, FORCE_FG_GC), true, true,
								NULL_SEGNO))
			wait_ms = gc_th->no_gc_sleep_time;

		trace_f2fs_background_turbo_gc(sbi->sb, wait_ms,
				prefree_segments(sbi), free_segments(sbi),
				tz_info->free_segs);

		/* balancing f2fs's metadata periodically */
		f2fs_balance_fs_bg(sbi);

		recovery_turbo_init(sbi, true);
next:
		sb_end_write(sbi->sb);

		/*lint -restore*/
	} while (!kthread_should_stop());

	return 0;
}

#ifdef CONFIG_MAS_BLK
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static void set_block_idle_turbo(unsigned long data)
{
	struct f2fs_gc_kthread *gc_th = (struct f2fs_gc_kthread *)
		from_timer(gc_th, timer, nb_timer);

	gc_th->block_idle = true;
}
#else
static void set_block_idle_turbo(unsigned long data)
{
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_turbo_thread;

	gc_th->block_idle = true;
}
#endif

static enum blk_busyidle_callback_ret gc_io_busyidle_notify_handler_turbo(
			struct blk_busyidle_event_node *nb,
			enum blk_idle_notify_state state)
{
	enum blk_busyidle_callback_ret ret = BUSYIDLE_NO_IO;
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)nb->param_data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_turbo_thread;

	if (gc_th->f2fs_gc_task == NULL)
		return ret;
	switch (state) {
	case BLK_IDLE_NOTIFY:
		mod_timer(&gc_th->nb_timer,
				jiffies + msecs_to_jiffies(IDLE_WT));
		ret = BUSYIDLE_NO_IO;
		break;
	case BLK_BUSY_NOTIFY:
		del_timer_sync(&gc_th->nb_timer);
		gc_th->block_idle = false;
		ret = BUSYIDLE_NO_IO;
		break;
	}

	return ret;
}
#endif

int f2fs_start_gc_turbo_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_turbo_thread;
	struct task_struct *f2fs_gc_task;
	dev_t dev = sbi->sb->s_bdev->bd_dev;
	int err = 0;

	WRITE_ONCE(gc_th->f2fs_gc_task, NULL);

	if (is_tz_closed(&sbi->tz_info))
		return 0;
	gc_th->urgent_sleep_time = DEF_GC_THREAD_URGENT_SLEEP_TIME;
	gc_th->min_sleep_time = DEF_GC_THREAD_MIN_SLEEP_TIME;
	gc_th->max_sleep_time = DEF_GC_THREAD_MAX_SLEEP_TIME;
	gc_th->no_gc_sleep_time = DEF_GC_THREAD_NOGC_SLEEP_TIME;

	gc_th->gc_wake = 0;
	gc_th->gc_preference = GC_BALANCE;

#ifdef CONFIG_MAS_BLK
	gc_th->block_idle = false;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	timer_setup(&gc_th->nb_timer, set_block_idle_turbo, 0);
#else
	setup_timer(&gc_th->nb_timer, set_block_idle_turbo, (unsigned long)sbi);
#endif
	strncpy(gc_th->gc_event_node.subscriber, "f2fs_gc_turbo", SUBSCRIBER_NAME_LEN);
	gc_th->gc_event_node.blk_busyidle_callback =
				gc_io_busyidle_notify_handler_turbo;
	gc_th->gc_event_node.param_data = sbi;
	err = blk_busyidle_event_subscribe(sbi->sb->s_bdev, &gc_th->gc_event_node);
#endif

	init_waitqueue_head(&gc_th->gc_wait_queue_head);
	init_waitqueue_head(&gc_th->fg_gc_wait);
	f2fs_gc_task = kthread_run(gc_thread_turbo_func, sbi,
		"f2fs_gc_turbo-%u:%u", MAJOR(dev), MINOR(dev));
	if (IS_ERR(f2fs_gc_task))
		err = PTR_ERR(f2fs_gc_task);
	else
		WRITE_ONCE(gc_th->f2fs_gc_task, f2fs_gc_task);
	return err;
}

void f2fs_stop_gc_turbo_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_turbo_thread;

	if (gc_th->f2fs_gc_task != NULL) {
		kthread_stop(gc_th->f2fs_gc_task);
		WRITE_ONCE(gc_th->f2fs_gc_task, NULL);
		wake_up_all(&gc_th->fg_gc_wait);
#ifdef CONFIG_MAS_BLK
		del_timer_sync(&gc_th->nb_timer);
retry:
		if (blk_busyidle_event_unsubscribe(&gc_th->gc_event_node) &&
			retry_cnt > 0) {
			retry_cnt--;
			goto retry;
		}
#endif
	}
}
#endif

static int select_gc_type(struct f2fs_sb_info *sbi, int gc_type)
{
	int gc_mode = (gc_type == BG_GC) ? GC_CB : GC_GREEDY;

	switch (sbi->gc_mode) {
	case GC_IDLE_CB:
		gc_mode = GC_CB;
		break;
	case GC_IDLE_GREEDY:
	case GC_URGENT:
		gc_mode = GC_GREEDY;
		break;
	}
	return gc_mode;
}

static void select_policy(struct f2fs_sb_info *sbi, int gc_type,
			int type, struct victim_sel_policy *p)
{
	struct dirty_seglist_info *dirty_i = DIRTY_I(sbi);
	int dirty_type = type;

#ifdef CONFIG_F2FS_TURBO_ZONE
	if (dirty_type == CURSEG_TURBO_DATA)
		dirty_type = CURSEG_WARM_DATA;
#endif
	if (p->alloc_mode == SSR) {
		p->gc_mode = GC_GREEDY;
		p->dirty_segmap = dirty_i->dirty_segmap[dirty_type];
		p->max_search = dirty_i->nr_dirty[dirty_type];
		p->ofs_unit = 1;
	} else {
		p->gc_mode = select_gc_type(sbi, gc_type);
		p->dirty_segmap = dirty_i->dirty_segmap[DIRTY];
		p->max_search = dirty_i->nr_dirty[DIRTY];
		p->ofs_unit = sbi->segs_per_sec;
	}

	/* we need to check every dirty segments in the FG_GC case */
	if (gc_type != FG_GC &&
			(sbi->gc_mode != GC_URGENT) &&
			p->max_search > sbi->max_victim_search)
		p->max_search = sbi->max_victim_search;

	/* let's select beginning hot/small space first in no_heap mode*/
#ifdef CONFIG_F2FS_TURBO_ZONE
	if (test_opt(sbi, NOHEAP) &&
		(type == CURSEG_HOT_DATA || IS_NODESEG(type) ||
			type == CURSEG_TURBO_DATA))
#else
	if (test_opt(sbi, NOHEAP) &&
		(type == CURSEG_HOT_DATA || IS_NODESEG(type)))
#endif
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
	if (p->gc_mode == GC_GREEDY)
		return 2 * sbi->blocks_per_seg * p->ofs_unit;
	else if (p->gc_mode == GC_CB)
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

static unsigned int get_cb_cost(struct f2fs_sb_info *sbi, unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	unsigned int secno = GET_SEC_FROM_SEG(sbi, segno);
	unsigned int start = GET_SEG_FROM_SEC(sbi, secno);
	unsigned long long mtime = 0;
	unsigned int vblocks;
	unsigned int max_age;
	unsigned char age = 0;
	unsigned char u;
	unsigned int i;

	for (i = 0; i < sbi->segs_per_sec; i++)
		mtime += get_seg_entry(sbi, start + i)->mtime;
	vblocks = get_valid_blocks(sbi, segno, true);

	mtime = div_u64(mtime, sbi->segs_per_sec);
	vblocks = div_u64(vblocks, sbi->segs_per_sec);

	u = (vblocks * 100) >> sbi->log_blocks_per_seg;

	/* Handle if the system time has changed by the user */
	if (mtime < sit_i->min_mtime)
		sit_i->min_mtime = mtime;
	if (mtime > sit_i->max_mtime)
		sit_i->max_mtime = mtime;
	/* lint -save -e613 -e666 */
	/* Reduce the cost weight of age when free blocks less than 10% */
	max_age = (gc_th && gc_th->gc_preference != GC_LIFETIME &&
		gc_perf_ratio(sbi) < 10) ? max(10 * gc_perf_ratio(sbi), 1) : 100;
	/* lint -restore */
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
	else
		return get_cb_cost(sbi, segno);
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

#ifdef CONFIG_F2FS_TURBO_ZONE
static void get_victim_area(struct f2fs_sb_info *sbi, int type,
			unsigned int *start, unsigned int *end)
{
	if (type == CURSEG_TURBO_DATA) {
		*start = sbi->tz_info.start_seg;
		*end = sbi->tz_info.end_seg;
		return;
	}

	if (!is_tz_existed(sbi) ||
		(!sbi->tz_info.enabled && sbi->tz_info.switchable))
		return;
	/* switchable configure force turn off for factory version */
	if (is_tz_existed(sbi) &&
		!sbi->tz_info.enabled && !sbi->tz_info.switchable)
		return;

	get_nz_area(sbi, start, end);
}
#endif

/*
 * This function is called from two paths.
 * One is garbage collection and the other is SSR segment selection.
 * When it is called during GC, it just gets a victim segment
 * and it does not remove it from dirty seglist.
 * When it is called from SSR segment selection, it finds a segment
 * which has minimum valid blocks and removes it from dirty seglist.
 */
static int get_victim_by_default(struct f2fs_sb_info *sbi,
		unsigned int *result, int gc_type, int type, char alloc_mode)
{
	struct dirty_seglist_info *dirty_i = DIRTY_I(sbi);
	struct sit_info *sm = SIT_I(sbi);
	struct victim_sel_policy p;
	unsigned int secno, last_victim;
	unsigned int last_segment;
	unsigned int nsearched = 0;
#ifdef CONFIG_F2FS_TURBO_ZONE
	unsigned int start_segno = 0;
	unsigned int end_segno = last_segment;
#endif

	mutex_lock(&dirty_i->seglist_lock);
	last_segment = MAIN_SECS(sbi) * sbi->segs_per_sec;

	p.alloc_mode = alloc_mode;
	select_policy(sbi, gc_type, type, &p);

	p.min_segno = NULL_SEGNO;
	p.min_cost = get_max_cost(sbi, &p);

	if (*result != NULL_SEGNO) {
		if (get_valid_blocks(sbi, *result, false) &&
			!sec_usage_check(sbi, GET_SEC_FROM_SEG(sbi, *result)))
			p.min_segno = *result;
		goto out;
	}

	if (p.max_search == 0)
		goto out;

	if (__is_large_section(sbi) && p.alloc_mode == LFS) {
		if (sbi->next_victim_seg[BG_GC] != NULL_SEGNO) {
			p.min_segno = sbi->next_victim_seg[BG_GC];
			*result = p.min_segno;
			sbi->next_victim_seg[BG_GC] = NULL_SEGNO;
			goto got_result;
		}
		if (gc_type == FG_GC &&
				sbi->next_victim_seg[FG_GC] != NULL_SEGNO) {
			p.min_segno = sbi->next_victim_seg[FG_GC];
			*result = p.min_segno;
			sbi->next_victim_seg[FG_GC] = NULL_SEGNO;
			goto got_result;
		}
	}

	last_victim = sm->last_victim[p.gc_mode];
	if (p.alloc_mode == LFS && gc_type == FG_GC) {
		p.min_segno = check_bg_victims(sbi);
		if (p.min_segno != NULL_SEGNO)
			goto got_it;
	}

#ifdef CONFIG_F2FS_TURBO_ZONE
	get_victim_area(sbi, type, &start_segno, &end_segno);
#endif

	while (1) {
		unsigned long cost;
		unsigned int segno;

#ifdef CONFIG_F2FS_TURBO_ZONE
		if (p.offset < start_segno)
			p.offset = start_segno;
		if (last_segment > end_segno)
			last_segment = end_segno;
#endif

		segno = find_next_bit(p.dirty_segmap, last_segment, p.offset);
		if (segno >= last_segment) {
#ifdef CONFIG_F2FS_TURBO_ZONE
			if (type == CURSEG_TURBO_DATA)
				break;
#endif
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
		if (p.ofs_unit > 1) {
			p.offset -= segno % p.ofs_unit;
			nsearched += count_bits(p.dirty_segmap,
						p.offset - p.ofs_unit,
						p.ofs_unit);
		} else {
			nsearched++;
		}

#ifdef CONFIG_F2FS_CHECK_FS
		/*
		 * skip selecting the invalid segno (that is failed due to block
		 * validity check failure during GC) to avoid endless GC loop in
		 * such cases.
		 */
		if (test_bit(segno, sm->invalid_segmap))
			goto next;
#endif

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
			sbi->gc_loop.segmap && test_bit(segno, sbi->gc_loop.segmap))
			goto next;

		cost = get_gc_cost(sbi, segno, &p);

		if (p.min_cost > cost) {
			p.min_segno = segno;
			p.min_cost = cost;
		}
next:
		if (nsearched >= p.max_search) {
#ifdef CONFIG_F2FS_TURBO_ZONE
			if (type == CURSEG_TURBO_DATA)
				break;
#endif

			if (!sm->last_victim[p.gc_mode] && segno <= last_victim)
				sm->last_victim[p.gc_mode] = last_victim + 1;
			else
				sm->last_victim[p.gc_mode] = segno + 1;
			sm->last_victim[p.gc_mode] %=
				(MAIN_SECS(sbi) * sbi->segs_per_sec);
			break;
		}
	}
	if (p.min_segno != NULL_SEGNO) {
got_it:
		*result = (p.min_segno / p.ofs_unit) * p.ofs_unit;
got_result:
		if (p.alloc_mode == LFS) {
			secno = GET_SEC_FROM_SEG(sbi, p.min_segno);
			if (gc_type == FG_GC)
				sbi->cur_victim_sec = secno;
			else
				set_bit(secno, dirty_i->victim_secmap);
		}

	}
out:
	if (p.min_segno != NULL_SEGNO)
		trace_f2fs_get_victim(sbi->sb, type, gc_type, &p,
				sbi->cur_victim_sec,
				prefree_segments(sbi), free_segments(sbi));
	mutex_unlock(&dirty_i->seglist_lock);

	return (p.min_segno == NULL_SEGNO) ? 0 : 1;
}

static const struct victim_selection default_v_ops = {
	.get_victim = get_victim_by_default,
};

static struct inode *find_gc_inode(struct gc_inode_list *gc_list, nid_t ino)
{
	struct inode_entry *ie;

	ie = radix_tree_lookup(&gc_list->iroot, ino);
	if (ie)
		return ie->inode;
	return NULL;
}

static void add_gc_inode(struct gc_inode_list *gc_list, struct inode *inode)
{
	struct inode_entry *new_ie;

	if (inode == find_gc_inode(gc_list, inode->i_ino)) {
		iput(inode);
		return;
	}
	new_ie = f2fs_kmem_cache_alloc(f2fs_inode_entry_slab, GFP_NOFS);
	new_ie->inode = inode;

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
		kmem_cache_free(f2fs_inode_entry_slab, ie);
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
	int phase = 0;
	int gc_cnt = 0;
	bool fggc = (gc_type == FG_GC);
	int submitted = 0;
#ifdef CONFIG_F2FS_TURBO_ZONE
	bool in_turbo_zone = false;

	if (sbi->tz_info.enabled && is_in_turbo_zone(sbi, segno))
		in_turbo_zone = true;
#endif
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
			f2fs_ra_meta_pages(sbi, NAT_BLOCK_OFFSET(nid), 1,
							META_NAT, true);
			continue;
		}

		if (phase == 1) {
			f2fs_ra_node_page(sbi, nid);
			continue;
		}

		/* phase == 2 */
		node_page = f2fs_get_node_page(sbi, nid);
		if (IS_ERR(node_page)) {
			f2fs_gc_loop_debug(sbi);
			continue;
		}

		/* block may become invalid during f2fs_get_node_page */
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
			f2fs_gc_loop_debug(sbi);
			continue;
		}

		err = f2fs_move_node_page(node_page, gc_type);
		if (!err && gc_type == FG_GC)
			submitted++;
		if (!err)
			gc_cnt++;
		stat_inc_node_blk_count(sbi, 1, gc_type);
#ifdef CONFIG_F2FS_TURBO_ZONE
		if (in_turbo_zone && gc_type == BG_GC)
			stat_inc_turbo_bg_gc_tot_blk_count(F2FS_STAT(sbi), 1);
#endif
	}

	if (++phase < 3)
		goto next_step;

	if (fggc)
		atomic_dec(&sbi->wb_sync_req[NODE]);

	bd_mutex_lock(&sbi->bd_mutex);
	inc_bd_array_val(sbi, gc_node_blk_cnt, gc_type, gc_cnt);
	bd_mutex_unlock(&sbi->bd_mutex);
	return submitted;
}

/*
 * Calculate start block index indicating the given node offset.
 * Be careful, caller should give this node offset only indicating direct node
 * blocks. If any node offsets, which point the other types of node blocks such
 * as indirect or double indirect node blocks, are given, it must be a caller's
 * bug.
 */
block_t f2fs_start_bidx_of_node(unsigned int node_ofs, struct inode *inode)
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

	node_page = f2fs_get_node_page(sbi, nid);
	if (IS_ERR(node_page))
		return false;

	if (f2fs_get_node_info(sbi, nid, dni)) {
		f2fs_put_page(node_page, 1);
		return false;
	}

	if (sum->version != dni->version) {
		f2fs_msg(sbi->sb, KERN_WARNING,
				"%s: valid data with mismatched node version.",
				__func__);
		set_sbi_flag(sbi, SBI_NEED_FSCK);
		f2fs_set_need_fsck_report();
	}

	*nofs = ofs_of_node(node_page);
	source_blkaddr = datablock_addr(NULL, node_page, ofs_in_node);
	f2fs_put_page(node_page, 1);

	if (source_blkaddr != blkaddr) {
#ifdef CONFIG_F2FS_CHECK_FS
		unsigned int segno = GET_SEGNO(sbi, blkaddr);
		unsigned long offset = GET_BLKOFF_FROM_SEG0(sbi, blkaddr);

		if (unlikely(check_valid_map(sbi, segno, offset))) {
			if (!test_and_set_bit(segno, SIT_I(sbi)->invalid_segmap)) {
				f2fs_msg(sbi->sb, KERN_ERR, "mismatched blkaddr %u (source_blkaddr %u) in seg %u\n",
						blkaddr, source_blkaddr, segno);
				f2fs_bug_on(sbi, 1);
			}
		}
#endif
		return false;
	}
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

	if (f2fs_lookup_extent_cache(inode, index, &ei)) {
		dn.data_blkaddr = ei.blk + index - ei.fofs;
		goto got_it;
	}

	set_new_dnode(&dn, inode, NULL, NULL, 0);
	err = f2fs_get_dnode_of_data(&dn, index, LOOKUP_NODE);
	if (err)
		goto put_page;
	f2fs_put_dnode(&dn);

	if (unlikely(!f2fs_is_valid_blkaddr(sbi, dn.data_blkaddr,
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
	f2fs_wait_on_page_writeback(page, DATA, true, true);

	f2fs_wait_on_block_writeback(inode, dn.data_blkaddr);

	fio.encrypted_page = f2fs_pagecache_get_page(META_MAPPING(sbi),
					dn.data_blkaddr,
					FGP_LOCK | FGP_CREAT, GFP_NOFS);
	if (!fio.encrypted_page) {
		err = -ENOMEM;
		goto put_page;
	}

	err = f2fs_submit_page_bio(&fio);
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
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
			int gc_type, unsigned int segno, int off, bool key_flag)
#else
			int gc_type, unsigned int segno, int off)
#endif
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
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
		.flags = key_flag ? F2FS_IO_KEY_FLAG : 0,
#endif
	};
	struct dnode_of_data dn;
	struct f2fs_summary sum;
	struct node_info ni;
	struct page *page, *mpage;
	block_t newaddr = 0;
	int err = 0;
	bool lfs_mode = test_opt(fio.sbi, LFS);
	int type = CURSEG_COLD_DATA;

#ifdef CONFIG_F2FS_TURBO_ZONE
	if ((fio.sbi->tz_info.enabled) && is_tz_flag_set(inode, FI_TZ_KEY_FILE))
		type = CURSEG_TURBO_DATA;
#endif
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (f2fs_is_enabled_tz_v2(fio.sbi) &&
		is_tz_flag_set(inode, FI_TZ_KEY_FILE))
		type = CURSEG_COLD_DATA;
#endif

	/* do not read out */
	page = f2fs_grab_cache_page(inode->i_mapping, bidx, false);
	if (!page) {
		f2fs_gc_loop_debug(F2FS_I_SB(inode));
		return -ENOMEM;
	}

	if (!check_valid_map(F2FS_I_SB(inode), segno, off)) {
		err = -ENOENT;
		goto out;
	}

	if (f2fs_is_atomic_file(inode)) {
		F2FS_I(inode)->i_gc_failures[GC_FAILURE_ATOMIC]++;
		F2FS_I_SB(inode)->skipped_atomic_files[gc_type]++;
		f2fs_gc_loop_debug(F2FS_I_SB(inode));
		err = -EAGAIN;
		goto out;
	}

	if (f2fs_is_pinned_file(inode)) {
		f2fs_pin_file_control(inode, true);
		err = -EAGAIN;
		goto out;
	}

	set_new_dnode(&dn, inode, NULL, NULL, 0);
	err = f2fs_get_dnode_of_data(&dn, bidx, LOOKUP_NODE);
	if (err) {
		f2fs_gc_loop_debug(F2FS_I_SB(inode));
		goto out;
	}

	if (unlikely(dn.data_blkaddr == NULL_ADDR)) {
		ClearPageUptodate(page);
		f2fs_gc_loop_debug(F2FS_I_SB(inode));
		err = -ENOENT;
		goto put_out;
	}

	/*
	 * don't cache encrypted data into meta inode until previous dirty
	 * data were writebacked to avoid racing between GC and flush.
	 */
	f2fs_wait_on_page_writeback(page, DATA, true, true);

	f2fs_wait_on_block_writeback(inode, dn.data_blkaddr);

	err = f2fs_get_node_info(fio.sbi, dn.nid, &ni);
	if (err)
		goto put_out;

	set_summary(&sum, dn.nid, dn.ofs_in_node, ni.version);

	/* read page */
	fio.page = page;
	fio.new_blkaddr = fio.old_blkaddr = dn.data_blkaddr;

	if (lfs_mode)
		down_write(&fio.sbi->io_order_lock);

	f2fs_allocate_data_block(fio.sbi, NULL, fio.old_blkaddr, &newaddr,
				&sum, type, NULL, false, SEQ_NONE);

	fio.encrypted_page = f2fs_pagecache_get_page(META_MAPPING(fio.sbi),
				newaddr, FGP_LOCK | FGP_CREAT, GFP_NOFS);
	if (!fio.encrypted_page) {
		err = -ENOMEM;
		f2fs_gc_loop_debug(F2FS_I_SB(inode));
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

	err = f2fs_submit_page_bio(&fio);
	if (err) {
		f2fs_gc_loop_debug(F2FS_I_SB(inode));
		goto put_page_out;
	}

	/* write page */
	lock_page(fio.encrypted_page);

	if (unlikely(fio.encrypted_page->mapping != META_MAPPING(fio.sbi))) {
		err = -EIO;
		f2fs_gc_loop_debug(F2FS_I_SB(inode));
		goto put_page_out;
	}
	if (unlikely(!PageUptodate(fio.encrypted_page))) {
		err = -EIO;
		goto put_page_out;
	}

write_page:
	f2fs_wait_on_page_writeback(fio.encrypted_page, DATA, true, true);
	set_page_dirty(fio.encrypted_page);
	if (clear_page_dirty_for_io(fio.encrypted_page))
		dec_page_count(fio.sbi, F2FS_DIRTY_META);

	set_page_writeback(fio.encrypted_page);
	ClearPageError(page);

	/* allocate block address */
	f2fs_wait_on_page_writeback(dn.node_page, NODE, true, true);

	fio.op = REQ_OP_WRITE;
	fio.op_flags = REQ_SYNC;
	fio.new_blkaddr = newaddr;
	f2fs_submit_page_write(&fio);
	if (fio.retry) {
		err = -EAGAIN;
		if (PageWriteback(fio.encrypted_page))
			end_page_writeback(fio.encrypted_page);
		goto put_page_out;
	}

	f2fs_update_iostat(fio.sbi, FS_GC_DATA_IO, F2FS_BLKSIZE);

	f2fs_update_data_blkaddr(&dn, newaddr);
	set_inode_flag(inode, FI_APPEND_WRITE);
	if (page->index == 0)
		set_inode_flag(inode, FI_FIRST_BLOCK_WRITTEN);
put_page_out:
	f2fs_put_page(fio.encrypted_page, 1);
recover_block:
	if (lfs_mode)
		up_write(&fio.sbi->io_order_lock);
	if (err)
		f2fs_do_replace_block(fio.sbi, &sum, newaddr, fio.old_blkaddr,
								true, true);
put_out:
	f2fs_put_dnode(&dn);
out:
	f2fs_put_page(page, 1);
	return err;
}

static int move_data_page(struct inode *inode, block_t bidx, int gc_type,
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
				unsigned int segno, int off, bool key_flag)
#else
				unsigned int segno, int off)
#endif
{
	struct page *page;
	int err = 0;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	page = f2fs_get_lock_data_page(inode, bidx, true);
	if (IS_ERR(page)) {
		f2fs_gc_loop_debug(sbi);
		return PTR_ERR(page);
	}

	if (!check_valid_map(F2FS_I_SB(inode), segno, off)) {
		err = -ENOENT;
		goto out;
	}

	if (f2fs_is_atomic_file(inode)) {
		F2FS_I(inode)->i_gc_failures[GC_FAILURE_ATOMIC]++;
		F2FS_I_SB(inode)->skipped_atomic_files[gc_type]++;
		f2fs_gc_loop_debug(sbi);
		err = -EAGAIN;
		goto out;
	}
	if (f2fs_is_pinned_file(inode)) {
		if (gc_type == FG_GC)
			f2fs_pin_file_control(inode, true);
		err = -EAGAIN;
		goto out;
	}

	if (gc_type == BG_GC) {
		if (PageWriteback(page)) {
			f2fs_gc_loop_debug(sbi);
			err = -EAGAIN;
			goto out;
		}
		set_page_dirty(page);
		set_cold_data(page);
	} else {
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
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
			.flags = key_flag ? F2FS_IO_KEY_FLAG : 0,
#endif
		};
		bool is_dirty = PageDirty(page);
		unsigned long cnt = 0;

retry:
		f2fs_wait_on_page_writeback(page, DATA, true, true);

		set_page_dirty(page);
		if (clear_page_dirty_for_io(page)) {
			inode_dec_dirty_pages(inode);
			f2fs_remove_dirty_inode(inode);
		}

		set_cold_data(page);

		err = f2fs_do_write_data_page(&fio);
		if (err) {
			clear_cold_data(page);
			if (err == -ENOMEM) {
				cnt++;
				if (!(cnt % F2FS_GC_LOOP_NOMEM_MOD))
					f2fs_msg(sbi->sb, KERN_ERR,
					"f2fs_gc_loop nomem retry:%lu in %s:%d\n",
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
 * This function tries to get parent node of victim data block, and identifies
 * data block validity. If the block is valid, copy that with cold status and
 * modify parent node.
 * If the parent node is not valid or the data block address is different,
 * the victim data block is ignored.
 */
static int gc_data_segment(struct f2fs_sb_info *sbi, struct f2fs_summary *sum,
		struct gc_inode_list *gc_list, unsigned int segno, int gc_type)
{
	struct super_block *sb = sbi->sb;
	struct f2fs_summary *entry;
	block_t start_addr;
	int off;
	int phase = 0;
	int gc_cnt = 0;
	int submitted = 0;
#ifdef CONFIG_F2FS_TURBO_ZONE
	bool in_turbo_zone = false;

	if (sbi->tz_info.enabled && is_in_turbo_zone(sbi, segno))
		in_turbo_zone = true;
#endif
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
			f2fs_ra_meta_pages(sbi, NAT_BLOCK_OFFSET(nid), 1,
							META_NAT, true);
			continue;
		}

		if (phase == 1) {
			f2fs_ra_node_page(sbi, nid);
			continue;
		}

		/* Get an inode by ino with checking validity */
		if (!is_alive(sbi, entry, &dni, start_addr + off, &nofs)) {
			f2fs_gc_loop_debug(sbi);
			continue;
		}

		if (phase == 2) {
			f2fs_ra_node_page(sbi, dni.ino);
			continue;
		}

		ofs_in_node = le16_to_cpu(entry->ofs_in_node);

		if (phase == 3) {
			inode = f2fs_iget(sb, dni.ino);
			if (IS_ERR(inode) || is_bad_inode(inode)) {
				f2fs_gc_loop_debug(sbi);
				continue;
			}

			if (!down_write_trylock(
				&F2FS_I(inode)->i_gc_rwsem[WRITE])) {
				iput(inode);
				sbi->skipped_gc_rwsem++;
				continue;
			}

			start_bidx = f2fs_start_bidx_of_node(nofs, inode) +
								ofs_in_node;

			if (f2fs_post_read_required(inode)) {
				int err = ra_data_block(inode, start_bidx);

				up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
				if (err) {
					iput(inode);
					f2fs_gc_loop_debug(sbi);
					continue;
				}
				add_gc_inode(gc_list, inode);
				continue;
			}

			data_page = f2fs_get_read_data_page(inode,
						start_bidx, REQ_RAHEAD, true);
			up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
			if (IS_ERR(data_page)) {
				iput(inode);
				continue;
			}

			f2fs_put_page(data_page, 0);
			add_gc_inode(gc_list, inode);
			continue;
		}

		/* phase 4 */
		inode = find_gc_inode(gc_list, dni.ino);
		if (inode) {
			struct f2fs_inode_info *fi = F2FS_I(inode);
			bool locked = false;
			int err;

			if (S_ISREG(inode->i_mode)) {
				if (!down_write_trylock(&fi->i_gc_rwsem[READ])) {
					f2fs_gc_loop_debug(sbi);
					continue;
				}
				if (!down_write_trylock(
						&fi->i_gc_rwsem[WRITE])) {
					sbi->skipped_gc_rwsem++;
					up_write(&fi->i_gc_rwsem[READ]);
					f2fs_gc_loop_debug(sbi);
					continue;
				}
				locked = true;

				/* wait for all inflight aio data */
				inode_dio_wait(inode);
			}

			start_bidx = f2fs_start_bidx_of_node(nofs, inode)
								+ ofs_in_node;
			if (f2fs_post_read_required(inode))
				err = move_data_block(inode, start_bidx,
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
						gc_type, segno, off,
						f2fs_is_tz_key_io(sbi, inode));
#else
						gc_type, segno, off);
#endif
			else
				err = move_data_page(inode, start_bidx, gc_type,
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
						segno, off,
						f2fs_is_tz_key_io(sbi, inode));
#else
						segno, off);
#endif

			if (!err && (gc_type == FG_GC ||
					f2fs_post_read_required(inode)))
				submitted++;

			if (locked) {
				up_write(&fi->i_gc_rwsem[WRITE]);
				up_write(&fi->i_gc_rwsem[READ]);
			}

			stat_inc_data_blk_count(sbi, 1, gc_type);
#ifdef CONFIG_F2FS_TURBO_ZONE
			if (in_turbo_zone && gc_type == BG_GC)
				stat_inc_turbo_bg_gc_tot_blk_count(
							F2FS_STAT(sbi), 1);
#endif
			if (!err)
				gc_cnt++;
		} else {
			f2fs_gc_loop_debug(sbi);
		}
	}

	if (++phase < 5)
		goto next_step;

	bd_mutex_lock(&sbi->bd_mutex);
	inc_bd_array_val(sbi, gc_data_blk_cnt, gc_type, gc_cnt);
	inc_bd_array_val(sbi, hotcold_cnt, HC_GC_COLD_DATA, gc_cnt);
	bd_mutex_unlock(&sbi->bd_mutex);
	return submitted;
}

#ifdef CONFIG_F2FS_TURBO_ZONE
static int __get_victim(struct f2fs_sb_info *sbi, unsigned int *victim,
			int gc_type, bool turbo)
#else
static int __get_victim(struct f2fs_sb_info *sbi, unsigned int *victim,
			int gc_type)
#endif
{
	struct sit_info *sit_i = SIT_I(sbi);
	int ret;

	down_write(&sit_i->sentry_lock);
#ifdef CONFIG_F2FS_TURBO_ZONE
	ret = DIRTY_I(sbi)->v_ops->get_victim(sbi, victim, gc_type,
			turbo ? CURSEG_TURBO_DATA : NO_CHECK_TYPE, LFS);
#else
	ret = DIRTY_I(sbi)->v_ops->get_victim(sbi, victim, gc_type,
					      NO_CHECK_TYPE, LFS);
#endif
	up_write(&sit_i->sentry_lock);
#ifdef CONFIG_F2FS_TURBO_ZONE
	if (turbo)
		f2fs_msg(sbi->sb, KERN_INFO, "GC_TURBO: __get_victim: "
				"*victim %d, gc_type %d, turbo %d, ret %d",
					*victim, gc_type, turbo, ret);
#endif
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
	unsigned int end_segno = start_segno + sbi->segs_per_sec;
	int seg_freed = 0, migrated = 0;
	int hotcold_type = get_seg_entry(sbi, segno)->type;
	unsigned char type = IS_DATASEG(get_seg_entry(sbi, segno)->type) ?
						SUM_TYPE_DATA : SUM_TYPE_NODE;
	int submitted = 0;

	if (__is_large_section(sbi))
		end_segno = rounddown(end_segno, sbi->segs_per_sec);

	/* readahead multi ssa blocks those have contiguous address */
	if (__is_large_section(sbi))
		f2fs_ra_meta_pages(sbi, GET_SUM_BLOCK(sbi, segno),
					end_segno - segno, META_SSA, true);

	/* reference all summary page */
	while (segno < end_segno) {
		sum_page = f2fs_get_sum_page(sbi, segno++);
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

		if (get_valid_blocks(sbi, segno, false) == 0)
			goto freed;
		if (__is_large_section(sbi) &&
				migrated >= sbi->migration_granularity)
			goto skip;
		if (!PageUptodate(sum_page) || unlikely(f2fs_cp_error(sbi)))
			goto skip;

		sum = page_address(sum_page);
		if (type != GET_SUM_TYPE((&sum->footer))) {
			f2fs_msg(sbi->sb, KERN_ERR, "Inconsistent segment (%u) "
				"type [%d, %d] in SSA and SIT",
				segno, type, GET_SUM_TYPE((&sum->footer)));
			set_sbi_flag(sbi, SBI_NEED_FSCK);
			f2fs_set_need_fsck_report();
			goto skip;
		}

		/*
		 * this is to avoid deadlock:
		 * - lock_page(sum_page)         - f2fs_replace_block
		 *  - check_valid_map()            - down_write(sentry_lock)
		 *   - down_read(sentry_lock)     - change_curseg()
		 *                                  - lock_page(sum_page)
		 */
		if (type == SUM_TYPE_NODE)
			submitted += gc_node_segment(sbi, sum->entries, segno,
								gc_type);
		else
			submitted += gc_data_segment(sbi, sum->entries, gc_list,
							segno, gc_type);

		stat_inc_seg_count(sbi, type, gc_type);

freed:
		if (gc_type == FG_GC &&
				get_valid_blocks(sbi, segno, false) == 0)
			seg_freed++;
		migrated++;
		bd_mutex_lock(&sbi->bd_mutex);
		if (gc_type == BG_GC || get_valid_blocks(sbi, segno, 1) == 0) {
			if (type == SUM_TYPE_NODE)
				inc_bd_array_val(sbi, gc_node_seg_cnt,
					gc_type, 1);
			else
				inc_bd_array_val(sbi, gc_data_seg_cnt,
					gc_type, 1);
			inc_bd_array_val(sbi, hotcold_gc_seg_cnt,
				hotcold_type + 1, 1UL); /* lint !e679 */
		}
		inc_bd_array_val(sbi, hotcold_gc_blk_cnt, hotcold_type + 1,
			(unsigned long)get_valid_blocks(sbi,
				segno, 1)); /* lint !e679 */
		bd_mutex_unlock(&sbi->bd_mutex);

		if (__is_large_section(sbi) && segno + 1 < end_segno)
			sbi->next_victim_seg[gc_type] = segno + 1;
skip:
		f2fs_put_page(sum_page, 0);
	}

	if (submitted)
		f2fs_submit_merged_write(sbi,
				(type == SUM_TYPE_NODE) ? NODE : DATA);

	blk_finish_plug(&plug);

	stat_inc_call_count(sbi->stat_info);

	return seg_freed;
}

#ifdef CONFIG_F2FS_TURBO_ZONE
int f2fs_gc(struct f2fs_sb_info *sbi, bool sync,
			bool background, bool turbo, unsigned int segno)
#else
int f2fs_gc(struct f2fs_sb_info *sbi, bool sync,
			bool background, unsigned int segno)
#endif
{
	int gc_type = sync ? FG_GC : BG_GC;
	int sec_freed = 0, seg_freed = 0, total_freed = 0;
	int ret = 0;
	struct cp_control cpc;
	unsigned int init_segno = segno;
	struct gc_inode_list gc_list = {
		.ilist = LIST_HEAD_INIT(gc_list.ilist),
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
		.iroot = RADIX_TREE_INIT(GFP_NOFS),
#else
		.iroot = RADIX_TREE_INIT(gc_list.iroot, GFP_NOFS),
#endif
	};
	int gc_completed = 0;
	u64 fggc_begin = 0;
	u64 fggc_end;
	unsigned long long last_skipped = sbi->skipped_atomic_files[FG_GC];
	unsigned long long first_skipped;
	unsigned int skipped_round = 0, round = 0;

	fggc_begin = local_clock();
	trace_f2fs_gc_begin(sbi->sb, sync, background,
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
		if (prefree_segments(sbi) &&
				!is_sbi_flag_set(sbi, SBI_CP_DISABLED)) {
			ret = f2fs_write_checkpoint(sbi, &cpc);
			if (ret)
				goto stop;
		}
		if (has_not_enough_free_secs(sbi, 0, 0))
			gc_type = FG_GC;
	}

	/* f2fs_balance_fs doesn't need to do BG_GC in critical path. */
	if (gc_type == BG_GC && !background) {
		ret = -EINVAL;
		goto stop;
	}

#ifdef CONFIG_F2FS_JOURNAL_APPEND
	if (write_opt && (gc_type == BG_GC)
			&& (100 * get_max_reclaimable_segments(sbi) /
					free_segments(sbi)) < GC_THRESHOLD) {
		stat_dec_bggc_count(sbi);
		ret = -EINVAL;
		goto stop;
	}
#endif

#ifdef CONFIG_F2FS_TURBO_ZONE
	if (!__get_victim(sbi, &segno, gc_type, turbo)) {
#else
	if (!__get_victim(sbi, &segno, gc_type)) {
#endif
		ret = -ENODATA;
		goto stop;
	}

	if (unlikely(sbi->gc_loop.check && segno != sbi->gc_loop.segno))
		init_f2fs_gc_loop(sbi);
	seg_freed = do_garbage_collect(sbi, segno, &gc_list, gc_type);
	if (gc_type == FG_GC && seg_freed == sbi->segs_per_sec)
		sec_freed++;
	else if (unlikely(!seg_freed && gc_type == FG_GC)) {
		if (!sbi->gc_loop.check) {
			sbi->gc_loop.check = true;
			sbi->gc_loop.count = 1;
			sbi->gc_loop.segno = segno;
		}
		if (!(sbi->gc_loop.count % F2FS_GC_LOOP_MOD))
			f2fs_msg(sbi->sb, KERN_ERR, "f2fs_gc_loop same victim retry:%lu in %s:%d "
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
					kvzalloc(f2fs_bitmap_size(MAIN_SEGS(sbi)), GFP_KERNEL);
			if (sbi->gc_loop.segmap)
				set_bit(segno, sbi->gc_loop.segmap);
		}
	}
	total_freed += seg_freed;
	gc_completed = 1;

	if (gc_type == FG_GC) {
		if (sbi->skipped_atomic_files[FG_GC] > last_skipped ||
						sbi->skipped_gc_rwsem)
			skipped_round++;
		last_skipped = sbi->skipped_atomic_files[FG_GC];
		round++;
	}

	if (gc_type == FG_GC && seg_freed)
		sbi->cur_victim_sec = NULL_SEGNO;

	if (sync)
		goto stop;

	if (has_not_enough_free_secs(sbi, sec_freed, 0)) {
		if (skipped_round <= MAX_SKIP_GC_COUNT ||
					skipped_round * 2 < round) {
			segno = NULL_SEGNO;
			if (prefree_segments(sbi) &&
				has_not_enough_free_secs(sbi,
				reserved_sections(sbi), 0)) {
				ret = f2fs_write_checkpoint(sbi, &cpc);
				if (ret)
					goto stop;
				sec_freed = 0;
			}
			goto gc_more;
		}

		if (first_skipped < last_skipped &&
				(last_skipped - first_skipped) >
						sbi->skipped_gc_rwsem) {
			f2fs_drop_inmem_pages_all(sbi, true);
			segno = NULL_SEGNO;
			if (prefree_segments(sbi) &&
				has_not_enough_free_secs(sbi,
				reserved_sections(sbi), 0)) {
				ret = f2fs_write_checkpoint(sbi, &cpc);
				if (ret)
					goto stop;
				sec_freed = 0;
			}
			goto gc_more;
		}
		if (gc_type == FG_GC && !is_sbi_flag_set(sbi, SBI_CP_DISABLED))
			ret = f2fs_write_checkpoint(sbi, &cpc);
	}
stop:
	SIT_I(sbi)->last_victim[ALLOC_NEXT] = 0;
	SIT_I(sbi)->last_victim[FLUSH_DEVICE] = init_segno;

	trace_f2fs_gc_end(sbi->sb, ret, total_freed, sec_freed,
				get_pages(sbi, F2FS_DIRTY_NODES),
				get_pages(sbi, F2FS_DIRTY_DENTS),
				get_pages(sbi, F2FS_DIRTY_IMETA),
				free_sections(sbi),
				free_segments(sbi),
				reserved_segments(sbi),
				prefree_segments(sbi));

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
#ifdef CONFIG_F2FS_TURBO_ZONE
		if (turbo && gc_type == BG_GC) {
			inc_bd_val(sbi, tz_bg_gc_cnt, 1);
			if (ret)
				inc_bd_val(sbi, tz_bg_gc_fail_cnt, 1);
		}
#endif
		bd_mutex_unlock(&sbi->bd_mutex);
	}

	put_gc_inode(&gc_list);

	if (sync && !ret)
		ret = sec_freed ? 0 : -EAGAIN;
	if (unlikely(sbi->gc_loop.segmap)) {
		kvfree(sbi->gc_loop.segmap);
		sbi->gc_loop.segmap = NULL;
	}
	if (unlikely(sbi->gc_loop.check))
		init_f2fs_gc_loop(sbi);
	return ret;
}

void f2fs_build_gc_manager(struct f2fs_sb_info *sbi)
{
	DIRTY_I(sbi)->v_ops = &default_v_ops;

	sbi->gc_pin_file_threshold = DEF_GC_FAILED_PINNED_FILES;

	/* give warm/cold data area from slower device */
	if (f2fs_is_multi_device(sbi) && sbi->segs_per_sec == 1)
		SIT_I(sbi)->last_victim[ALLOC_NEXT] =
				GET_SEGNO(sbi, FDEV(0).end_blk) + 1;
}

static int free_segment_range(struct f2fs_sb_info *sbi,
				unsigned int secs, bool gc_only)
{
	unsigned int segno, next_inuse, start, end;
	struct cp_control cpc = { CP_RESIZE, 0, 0, 0 };
	int gc_mode, gc_type;
	int err = 0;
	int type;

	/* Force block allocation for GC */
	MAIN_SECS(sbi) -= secs;
	start = MAIN_SECS(sbi) * sbi->segs_per_sec;
	end = MAIN_SEGS(sbi) - 1;

	mutex_lock(&DIRTY_I(sbi)->seglist_lock);
	for (gc_mode = 0; gc_mode < MAX_GC_POLICY; gc_mode++)
		if (SIT_I(sbi)->last_victim[gc_mode] >= start)
			SIT_I(sbi)->last_victim[gc_mode] = 0;

	for (gc_type = BG_GC; gc_type <= FG_GC; gc_type++)
		if (sbi->next_victim_seg[gc_type] >= start)
			sbi->next_victim_seg[gc_type] = NULL_SEGNO;
	mutex_unlock(&DIRTY_I(sbi)->seglist_lock);

	/* Move out cursegs from the target range */
	for (type = CURSEG_HOT_DATA; type < NR_CURSEG_TYPE; type++)
		allocate_segment_for_resize(sbi, type, start, end);

	/* do GC to move out valid blocks in the range */
	for (segno = start; segno <= end; segno += sbi->segs_per_sec) {
		struct gc_inode_list gc_list = {
			.ilist = LIST_HEAD_INIT(gc_list.ilist),
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
			.iroot = RADIX_TREE_INIT(GFP_NOFS),
#else
			.iroot = RADIX_TREE_INIT(gc_list.iroot, GFP_NOFS),
#endif
		};

		do_garbage_collect(sbi, segno, &gc_list, FG_GC);
		put_gc_inode(&gc_list);

		if (!gc_only && get_valid_blocks(sbi, segno, true)) {
			err = -EAGAIN;
			goto out;
		}
		if (fatal_signal_pending(current)) {
			err = -ERESTARTSYS;
			goto out;
		}
	}
	if (gc_only)
		goto out;

	err = f2fs_write_checkpoint(sbi, &cpc);
	if (err)
		goto out;

	next_inuse = find_next_inuse(FREE_I(sbi), end + 1, start);
	if (next_inuse <= end) {
		f2fs_msg(sbi->sb, KERN_ERR,
			 "segno %u should be free but still inuse!", next_inuse);
		f2fs_bug_on(sbi, 1);
	}
out:
	MAIN_SECS(sbi) += secs;
	return err;
}

static void update_sb_metadata(struct f2fs_sb_info *sbi, int secs)
{
	struct f2fs_super_block *raw_sb = F2FS_RAW_SUPER(sbi);
	int section_count;
	int segment_count;
	int segment_count_main;
	long long block_count;
	int segs = secs * sbi->segs_per_sec;

	down_write(&sbi->sb_lock);

	section_count = le32_to_cpu(raw_sb->section_count);
	segment_count = le32_to_cpu(raw_sb->segment_count);
	segment_count_main = le32_to_cpu(raw_sb->segment_count_main);
	block_count = le64_to_cpu(raw_sb->block_count);

	raw_sb->section_count = cpu_to_le32(section_count + secs);
	raw_sb->segment_count = cpu_to_le32(segment_count + segs);
	raw_sb->segment_count_main = cpu_to_le32(segment_count_main + segs);
	raw_sb->block_count = cpu_to_le64(block_count +
					(long long)segs * sbi->blocks_per_seg);
	if (f2fs_is_multi_device(sbi)) {
		int last_dev = sbi->s_ndevs - 1;
		int dev_segs =
			le32_to_cpu(raw_sb->devs[last_dev].total_segments);

		raw_sb->devs[last_dev].total_segments =
						cpu_to_le32(dev_segs + segs);
	}

	up_write(&sbi->sb_lock);
}

static void update_fs_metadata(struct f2fs_sb_info *sbi, int secs)
{
	int segs = secs * sbi->segs_per_sec;
	long long blks = (long long)segs * sbi->blocks_per_seg;
	long long user_block_count =
				le64_to_cpu(F2FS_CKPT(sbi)->user_block_count);

	SM_I(sbi)->segment_count = (int)SM_I(sbi)->segment_count + segs;
	MAIN_SEGS(sbi) = (int)MAIN_SEGS(sbi) + segs;
	MAIN_SECS(sbi) += secs;
	FREE_I(sbi)->free_sections = (int)FREE_I(sbi)->free_sections + secs;
	FREE_I(sbi)->free_segments = (int)FREE_I(sbi)->free_segments + segs;
	F2FS_CKPT(sbi)->user_block_count = cpu_to_le64(user_block_count + blks);

	if (f2fs_is_multi_device(sbi)) {
		int last_dev = sbi->s_ndevs - 1;

		FDEV(last_dev).total_segments =
				(int)FDEV(last_dev).total_segments + segs;
		FDEV(last_dev).end_blk =
				(long long)FDEV(last_dev).end_blk + blks;
#ifdef CONFIG_BLK_DEV_ZONED
		FDEV(last_dev).nr_blkz = (int)FDEV(last_dev).nr_blkz +
					(int)(blks >> sbi->log_blocks_per_blkz);
#endif
	}
}

int f2fs_resize_fs(struct f2fs_sb_info *sbi, __u64 resize_len)
{
	unsigned int section_size = F2FS_BLKSIZE * BLKS_PER_SEC(sbi);
	unsigned int secs = (resize_len + section_size - 1) / section_size;
	__u64 shrunk_blocks = (__u64)secs * BLKS_PER_SEC(sbi);
	struct cp_control cpc = { CP_RESIZE, 0, 0, 0 };
	int err = 0;
	__u64 old_block_count = le64_to_cpu(F2FS_RAW_SUPER(sbi)->block_count);

	if (shrunk_blocks > old_block_count)
		return -EINVAL;

	if (f2fs_is_multi_device(sbi)) {
		int last_dev = sbi->s_ndevs - 1;
		__u64 last_segs = FDEV(last_dev).total_segments;

		if (last_segs * sbi->blocks_per_seg <= shrunk_blocks)
			return -EINVAL;
	}

	if (shrunk_blocks == 0)
		return 0;

	if (is_sbi_flag_set(sbi, SBI_NEED_FSCK)) {
		f2fs_msg(sbi->sb, KERN_ERR, "Should run fsck to repair first.");
		return -EFSCORRUPTED;
	}

	if (test_opt(sbi, DISABLE_CHECKPOINT)) {
		f2fs_msg(sbi->sb, KERN_ERR, "Checkpoint should be enabled.");
		return -EINVAL;
	}

	secs = div_u64(shrunk_blocks, BLKS_PER_SEC(sbi));

	/* stop other GC */
	if (!mutex_trylock(&sbi->gc_mutex))
		return -EAGAIN;

	/* should skip valid block addr check in some place */
	sbi->resized = true;
	/* stop CP to protect MAIN_SEC in free_segment_range */
	f2fs_lock_op(sbi);
	err = free_segment_range(sbi, secs, true);
	f2fs_unlock_op(sbi);
	mutex_unlock(&sbi->gc_mutex);
	if (err)
		goto out;

	set_sbi_flag(sbi, SBI_IS_RESIZEFS);

	freeze_super(sbi->sb);
	mutex_lock(&sbi->gc_mutex);
	mutex_lock(&sbi->cp_mutex);

	spin_lock(&sbi->stat_lock);
	if (shrunk_blocks + valid_user_blocks(sbi) +
		sbi->current_reserved_blocks + sbi->unusable_block_count +
		F2FS_OPTION(sbi).root_reserved_blocks > sbi->user_block_count)
		err = -ENOSPC;
	else
		sbi->user_block_count -= shrunk_blocks;
	spin_unlock(&sbi->stat_lock);
	if (err)
		goto out_err;

	err = free_segment_range(sbi, secs, false);
	if (err)
		goto recover_out;

	update_sb_metadata(sbi, -secs);

	err = f2fs_commit_super(sbi, false);
	if (err) {
		update_sb_metadata(sbi, secs);
		goto recover_out;
	}

	update_fs_metadata(sbi, -secs);
	clear_sbi_flag(sbi, SBI_IS_RESIZEFS);
	set_sbi_flag(sbi, SBI_IS_DIRTY);

	err = f2fs_write_checkpoint(sbi, &cpc);
	if (err) {
		update_fs_metadata(sbi, secs);
		update_sb_metadata(sbi, secs);
		f2fs_commit_super(sbi, false);
	}
recover_out:
	if (err) {
		set_sbi_flag(sbi, SBI_NEED_FSCK);
		f2fs_msg(sbi->sb, KERN_ERR, "resize_fs failed, should run fsck to repair!");

		spin_lock(&sbi->stat_lock);
		sbi->user_block_count += shrunk_blocks;
		spin_unlock(&sbi->stat_lock);
	}
out_err:
	mutex_unlock(&sbi->cp_mutex);
	mutex_unlock(&sbi->gc_mutex);
	thaw_super(sbi->sb);
	clear_sbi_flag(sbi, SBI_IS_RESIZEFS);
out:
	if (err)
		sbi->resized = false;
	return err;
}

#ifdef CONFIG_F2FS_TURBO_ZONE
static int check_data_blocks_in_tz(struct inode *inode, bool turbo,
							bool do_move)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_map_blocks map = { 0 };
	block_t off, blkaddr, segoff;
	block_t file_blocks = F2FS_BLK_ALIGN(i_size_read(inode));
	int submitted = 0, err = 0;
	unsigned int segno;
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_TURBO_DATA);

	f2fs_msg(sbi->sb, KERN_DEBUG,
		"%s:file_blocks %lu, turbo %d, do_move %d",
		TZ_LOG_TAG, file_blocks, turbo ? 1 : 0, do_move ? 1 : 0);

	while (map.m_lblk < file_blocks) {
		map.m_len = file_blocks - map.m_lblk;
		err = f2fs_map_blocks(inode, &map, 0, F2FS_GET_BLOCK_FIEMAP);
		if (err) {
			f2fs_msg(sbi->sb, KERN_WARNING,
				"%s%s:map err %d lblk %u, pblk %u, len %u",
				TZ_LOG_TAG, __func__, err, map.m_lblk,
				map.m_pblk, map.m_len);
			return err;
		}

		if (!is_valid_data_blkaddr(sbi, map.m_pblk))
			goto skip;

		for (off = 0; off < map.m_len; off++) {
			blkaddr = map.m_pblk + off;
			segno = GET_SEGNO(sbi, blkaddr);
			segoff = blkaddr - START_BLOCK(sbi, segno);

			if (is_in_turbo_zone(sbi, segno) == turbo)
				continue;

			if (!do_move)
				return -EAGAIN;

			/* every 5000 offset should be resched except 0 */
			if ((off + 1) % MIGRATE_FILE_MAX_SCHEDULE == 0)
				cond_resched();

			if (turbo && !curseg->inited) {
				recovery_turbo_init(sbi, true);
				if (!curseg->inited) {
					f2fs_msg(sbi->sb, KERN_WARNING,
						"Turbo zone space is full, stop migrate\n");
					return do_move ? submitted : 0;
				}
			}

			if (!turbo && sbi->tz_info.enabled == false) {
				f2fs_msg(sbi->sb, KERN_WARNING,
						"Normal zone space is full, stop migrate\n");
				return do_move ? submitted : 0;
			}

			mutex_lock(&sbi->gc_mutex);

			if (f2fs_encrypted_file(inode))
				err = move_data_block(inode, map.m_lblk + off,
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
						FG_GC, segno, segoff, false);
#else
						FG_GC, segno, segoff);
#endif
			else
				err = move_data_page(inode, map.m_lblk + off,
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
						FG_GC, segno, segoff, false);
#else
						FG_GC, segno, segoff);
#endif
			mutex_unlock(&sbi->gc_mutex);

			if (err) {
				f2fs_msg(sbi->sb, KERN_WARNING,
					"%s:migrate move bidx %u, err %d",
					TZ_LOG_TAG, map.m_lblk + off, err);
				inc_bd_ioctl_migrate_block(sbi, turbo, submitted);
				return err;
			}

			submitted++;

			if (fatal_signal_pending(current)) {
				f2fs_msg(sbi->sb, KERN_WARNING,
					"%s:migrate: signal received when move bidx %u, submitted %d",
					TZ_LOG_TAG, map.m_lblk + off, submitted);
				goto out;
			}
		}
skip:
		map.m_lblk += (map.m_len ? map.m_len : 1);
	}
out:
	if (do_move)
		inc_bd_ioctl_migrate_block(sbi, turbo, submitted);
	return do_move ? submitted : 0;
}

static bool need_sync(struct f2fs_sb_info *sbi) {
	//we define dirty_sentries_threshold 1 / 8 of main segment count
	int dirty_sentries_threshold = F2FS_RAW_SUPER(sbi)->segment_count_main >> 3;
	int free_sec_remain = FREE_SEC_REMAIN;
	if (sbi->tz_info.enabled) {
		int unavailable_segs = FDEV(F2FS_TURBO_DEV).total_segments -
			sbi->tz_info.total_segs;
		free_sec_remain =  unavailable_segs / sbi->segs_per_sec + FREE_SEC_REMAIN;
	}
	if (has_not_enough_free_secs(sbi, 0, free_sec_remain) &&
		(SIT_I(sbi)->dirty_sentries > dirty_sentries_threshold ||
		prefree_segments(sbi) > PREFREE_THRESHOLD))
		return true;
	return false;
}

int f2fs_migrate_file(struct inode *inode, bool turbo, bool sync)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	int ret;

	if (f2fs_has_inline_data(inode))
		return -EINVAL;

	if (turbo) {
		set_inode_flag(inode, FI_TZ_KEY_FILE);
		fi->i_flags |= F2FS_TZ_KEY_FL;
	} else {
		clear_inode_flag(inode, FI_TZ_KEY_FILE);
		fi->i_flags &= ~F2FS_TZ_KEY_FL;
	}

	f2fs_set_inode_flags(inode);
	f2fs_mark_inode_dirty_sync(inode, true);

	down_write(&fi->i_gc_rwsem[READ]);
	down_write(&fi->i_gc_rwsem[WRITE]);
	inode_dio_wait(inode);

	ret = check_data_blocks_in_tz(inode, turbo, true);

	up_write(&fi->i_gc_rwsem[WRITE]);
	up_write(&fi->i_gc_rwsem[READ]);
	if (need_sync(sbi))
		f2fs_sync_fs(sbi->sb, true);
	if (ret < 0)
		return ret;

	if (ret > 0)
		f2fs_submit_merged_write(sbi, DATA);

	if (sync) {
		ret = filemap_write_and_wait_range(inode->i_mapping,
						0, i_size_read(inode));
		if (ret < 0) {
			f2fs_msg(sbi->sb, KERN_ERR,
				"%s%s:sync filemap write err, turbo %d, ret %d",
				TZ_LOG_TAG, __func__, turbo ? 1 : 0, ret);
			return ret;
		}

		ret = check_data_blocks_in_tz(inode, turbo, false);
		if (ret < 0)
			return ret;
	}

	recovery_turbo_init(sbi, true);

	return 0;
}

#endif

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
#define TZ_BLK_BUF_SIZE 256
#define TZ_BLK_INVALID_LBA 0xFF
static int f2fs_do_move_data(struct inode *inode,
				const struct f2fs_map_blocks *map,
				block_t offset, bool turbo)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	block_t blkaddr, segoff;
	unsigned int segno;
	int err;

	blkaddr = map->m_pblk + offset;
	segno = GET_SEGNO(sbi, blkaddr);
	segoff = blkaddr - START_BLOCK(sbi, segno);

	mutex_lock(&sbi->gc_mutex);
	if (f2fs_encrypted_file(inode))
		err = move_data_block(inode, map->m_lblk + offset,
				FG_GC, segno, segoff, turbo);
	else
		err = move_data_page(inode, map->m_lblk + offset,
				FG_GC, segno, segoff, turbo);
	mutex_unlock(&sbi->gc_mutex);

	if (err)
		f2fs_msg(sbi->sb, KERN_WARNING,
				"%s:migrate move bidx %u, err %d",
				TZ_LOG_TAG, map->m_lblk + offset, err);

	return err;
}

static int f2fs_do_migrate(struct inode *inode,
				const struct f2fs_map_blocks *map,
				struct tz_blk_info *blk_info,
				unsigned int *submitted,
				bool turbo)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	block_t off;
	unsigned int loop, i, j;
	unsigned short len;
	int err = 0;

	loop = (map->m_len - 1) / TZ_BLK_BUF_SIZE + 1;
	for (i = 1; i <= loop; i++) {
		if (i == loop) {
			len = map->m_len % TZ_BLK_BUF_SIZE;
			if (len == 0)
				len = TZ_BLK_BUF_SIZE;
		} else {
			len = TZ_BLK_BUF_SIZE;
		}

		off = (i - 1) * TZ_BLK_BUF_SIZE;
		err = f2fs_query_tz_blks(sbi, blk_info, map->m_pblk + off, len);
		if (err)
			goto out;
		for (j = 0; j < len; j++) {
			block_t offset = off + j;

			if (blk_info->flags[j] == turbo) {
				continue;
			} else if (blk_info->flags[j] == TZ_BLK_INVALID_LBA) {
				f2fs_msg(sbi->sb, KERN_WARNING,
					"%s:send invalid qry blocks bidx %u, pblk %u",
					TZ_LOG_TAG, map->m_lblk + offset,
					map->m_lblk + offset);
				err = -EAGAIN;
				goto out;
			}

			err = f2fs_do_move_data(inode, map, offset, turbo);
			if (err)
				goto out;

			(*submitted)++;
			if (fatal_signal_pending(current)) {
				f2fs_msg(sbi->sb, KERN_WARNING,
					"%s:migrate: signal received when move bidx %u, submitted %d",
					TZ_LOG_TAG, map->m_lblk + offset,
					*submitted);
				err = -EINTR;
				goto out;
			}
		}
	}
out:
	return err;
}

static int f2fs_do_migrate_file(struct inode *inode, bool turbo,
				unsigned int *submitted)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_map_blocks map = { 0 };
	block_t file_blocks = F2FS_BLK_ALIGN(i_size_read(inode));
	int err = 0;
	struct tz_blk_info blk_info;

	blk_info.flags = kzalloc(TZ_BLK_BUF_SIZE, GFP_F2FS_ZERO);
	if (!blk_info.flags)
		return -ENOMEM;

	/*
	 * hisi scsi use 4 byte blk_info's buf to fill lun id,
	 * so we should make sure buf is greater than 4 bytes.
	 */
	blk_info.buf_len = TZ_BLK_BUF_SIZE;
	f2fs_msg(sbi->sb, KERN_DEBUG,
		"%s:file_blocks %lu, turbo %d",
		TZ_LOG_TAG, file_blocks, turbo ? 1 : 0);

	while (map.m_lblk < file_blocks) {
		map.m_len = file_blocks - map.m_lblk;
		err = f2fs_map_blocks(inode, &map, 0, F2FS_GET_BLOCK_FIEMAP);
		if (err) {
			f2fs_msg(sbi->sb, KERN_WARNING,
				"%s%s:map err %d lblk %u, pblk %u, len %u",
				TZ_LOG_TAG, __func__, err, map.m_lblk,
				map.m_pblk, map.m_len);
			goto out;
		}

		if (!is_valid_data_blkaddr(sbi, map.m_pblk) || !map.m_len)
			goto skip;
		err = f2fs_do_migrate(inode, &map, &blk_info, submitted, turbo);
		if (err)
			goto out;
		cond_resched();
skip:
		map.m_lblk += (map.m_len ? map.m_len : 1);
	}

out:
	kfree(blk_info.flags);
	inc_bd_ioctl_migrate_block(sbi, turbo, *submitted);
	return err ? err : *submitted;
}

int f2fs_migrate_file_v2(struct inode *inode, bool turbo, bool sync)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	unsigned int submitted = 0;
	int ret;

	if (f2fs_has_inline_data(inode))
		return -EINVAL;

	if (turbo) {
		set_inode_flag(inode, FI_TZ_KEY_FILE);
		fi->i_flags |= F2FS_TZ_KEY_FL;
		f2fs_set_inode_flags(inode);
		f2fs_mark_inode_dirty_sync(inode, true);
	}

	down_write(&fi->i_gc_rwsem[READ]);
	down_write(&fi->i_gc_rwsem[WRITE]);
	inode_dio_wait(inode);

	ret = f2fs_do_migrate_file(inode, turbo, &submitted);

	up_write(&fi->i_gc_rwsem[WRITE]);
	up_write(&fi->i_gc_rwsem[READ]);
	if (need_sync(sbi))
		f2fs_sync_fs(sbi->sb, true);
	if (ret < 0)
		return ret;

	if (ret > 0)
		f2fs_submit_merged_write(sbi, DATA);

	if (sync) {
		ret = filemap_write_and_wait_range(inode->i_mapping,
						0, i_size_read(inode));
		if (ret < 0) {
			f2fs_msg(sbi->sb, KERN_ERR,
				"%s%s:sync filemap write err, turbo %d, ret %d",
				TZ_LOG_TAG, __func__, turbo ? 1 : 0, ret);
			goto out;
		}
	}

	if (!turbo) {
		clear_inode_flag(inode, FI_TZ_KEY_FILE);
		fi->i_flags &= ~F2FS_TZ_KEY_FL;
		f2fs_set_inode_flags(inode);
		f2fs_mark_inode_dirty_sync(inode, true);
	}
out:
	return ret < 0 ? ret : submitted;
}
#endif
