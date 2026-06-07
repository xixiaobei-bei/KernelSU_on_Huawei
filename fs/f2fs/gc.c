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

#include "f2fs.h"
#include "node.h"
#include "segment.h"
#include "gc.h"
#include <trace/events/f2fs.h>

#ifdef CONFIG_F2FS_TURBO_ZONE
#include "turbo_zone.h"
#endif

static struct kmem_cache *victim_entry_slab;

#define IDLE_WT 1000
#define MIN_WT 1000
#define DEF_GC_BALANCE_MIN_SLEEP_TIME  10000   /* milliseconds */
#define DEF_GC_FRAG_MIN_SLEEP_TIME     1000    /* milliseconds */
#define GC_URGENT_DISABLE_BLKS	       (32<<18)        /* 32G */
#define GC_URGENT_SPACE			       (10<<18)        /* 10G */

extern int find_next_free_extent(const unsigned long *addr,
			 unsigned long size, unsigned long *offset);

static bool __is_frag_urgent(struct f2fs_sb_info *sbi)
{
	unsigned int total_segs =
			le32_to_cpu(sbi->raw_super->segment_count_main);
	unsigned int i;
	unsigned int block_count[10];
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
			if (unlikely(index >= 10))
				index = 9;

			block_count[index] += blocks;
			tot_blocks += blocks;
		}
		cond_resched();
	}
	for (i=0; i < 10; i++)
		printk("block_cnt[%d]: %d\n", i, block_count[i]);

	if ((block_count[0] + block_count[1]) >= (tot_blocks >> 1))
		return true;

	return false;
}

static bool is_frag_urgent(struct f2fs_sb_info *sbi)
{
	unsigned long next_check = sbi->last_urgent_check + msecs_to_jiffies(10*60*1000);
	if (time_after(jiffies, next_check)) {
		sbi->last_urgent_check = jiffies;
		sbi->is_frag = __is_frag_urgent(sbi);
	}
	return sbi->is_frag;
}

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

static inline int __gc_thread_wait_timeout(struct f2fs_sb_info *sbi, struct f2fs_gc_kthread *gc_th,
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

		/*lint -save -e574 -e666 */
		if (!is_gc_test_set(sbi, GC_TEST_DISABLE_FRAG_URGENT) &&
			is_frag_urgent(sbi) &&
			free_segments(sbi) < 3 * overprovision_segments(sbi) &&
			is_reclaimable_dirty_blocks_enough(sbi))
			gc_th->gc_preference = GC_FRAG;
		else if (100 * written_block_count(sbi) / sbi->user_block_count > 90)
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
			pr_err("f2fs-gc: some signals have been received...\n");
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


#ifdef CONFIG_HISI_BLK
		if (!gc_th->block_idle && !is_gc_test_set(sbi, GC_TEST_DISABLE_IO_AWARE)) {
#else
		if (!is_idle(sbi, GC_TIME) && !is_gc_test_set(sbi, GC_TEST_DISABLE_IO_AWARE)) {
#endif
			increase_sleep_time(gc_th, &wait_ms);
			if (is_gc_test_set(sbi, GC_TEST_ENABLE_GC_STAT))
				stat_io_skip_bggc_count(sbi);
			mutex_unlock(&sbi->gc_mutex);
			stat_io_skip_bggc_count(sbi);
			goto next;
		}

		if (has_enough_invalid_blocks(sbi))
			decrease_sleep_time(gc_th, &wait_ms);
		else
			increase_sleep_time(gc_th, &wait_ms);
do_gc:
		stat_inc_bggc_count(sbi);

#ifdef CONFIG_F2FS_STAT_FS
	{
		static DEFINE_RATELIMIT_STATE(bg_gc_rs, F2FS_GC_DSM_INTERVAL, F2FS_GC_DSM_BURST);
		unsigned long long total_size, free_size;

		total_size = blks_to_mb(sbi->user_block_count, sbi->blocksize);
		free_size = blks_to_mb(sbi->user_block_count -
				valid_user_blocks(sbi),
				sbi->blocksize);

		if (unlikely(__ratelimit(&bg_gc_rs))) {
			f2fs_msg(sbi->sb, KERN_NOTICE,
				"BG_GC: Size=%lluMB,Free=%lluMB,count=%d,free_sec=%u,reserved_sec=%u,node_secs=%d,dent_secs=%d\n",
				total_size, free_size,
				sbi->bg_gc, free_sections(sbi), reserved_sections(sbi),
				get_blocktype_secs(sbi, F2FS_DIRTY_NODES), get_blocktype_secs(sbi, F2FS_DIRTY_DENTS));
		}
	}
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

		/*lint -restore*/
	} while (!kthread_should_stop());
	return 0;
}

#ifdef CONFIG_HISI_BLK
static void set_block_idle(unsigned long data)
{
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	gc_th->block_idle = true;
}

static enum blk_busy_idle_callback_return gc_io_busy_idle_notify_handler(struct blk_busy_idle_event_node *event_node,
										       enum blk_idle_notify_state state)
{
	enum blk_busy_idle_callback_return ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)event_node->param_data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;

	if (gc_th->f2fs_gc_task == NULL)
		return ret;
	switch(state) {

	case BLK_IDLE_NOTIFY:
		mod_timer(&gc_th->nb_timer, jiffies + msecs_to_jiffies(IDLE_WT));
		ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
		break;
	case BLK_BUSY_NOTIFY:
		del_timer_sync(&gc_th->nb_timer);
		gc_th->block_idle = false;
		ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
		break;
	}

	return ret;
}
#endif

int f2fs_start_gc_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	struct task_struct *f2fs_gc_task;
	dev_t dev = sbi->sb->s_bdev->bd_dev;
	int err = 0;

	ACCESS_ONCE(gc_th->f2fs_gc_task) = NULL;

	gc_th->urgent_sleep_time = DEF_GC_THREAD_URGENT_SLEEP_TIME;
	gc_th->min_sleep_time = DEF_GC_THREAD_MIN_SLEEP_TIME;
	gc_th->max_sleep_time = DEF_GC_THREAD_MAX_SLEEP_TIME;
	gc_th->no_gc_sleep_time = DEF_GC_THREAD_NOGC_SLEEP_TIME;

	gc_th->gc_wake= 0;
	gc_th->gc_preference = GC_BALANCE;

#ifdef CONFIG_HISI_BLK
	setup_timer(&gc_th->nb_timer, set_block_idle, (unsigned long)sbi);
	gc_th->block_idle = false;
	strncpy(gc_th->gc_event_node.subscriber_name, "f2fs_gc", SUBSCRIBER_NAME_LEN);
	gc_th->gc_event_node.blk_busy_idle_notifier_callback = gc_io_busy_idle_notify_handler;
	gc_th->gc_event_node.param_data = sbi;
	err = blk_busy_idle_event_subscriber(sbi->sb->s_bdev, &gc_th->gc_event_node);
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
	f2fs_gc_task = kthread_run(gc_thread_func, sbi,
		"f2fs_gc-%u:%u", MAJOR(dev), MINOR(dev));
	if (IS_ERR(f2fs_gc_task))
		err = PTR_ERR(f2fs_gc_task);
	else
		ACCESS_ONCE(gc_th->f2fs_gc_task) = f2fs_gc_task;
	return err;
}

void f2fs_stop_gc_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	if (gc_th->f2fs_gc_task != NULL) {
		kthread_stop(gc_th->f2fs_gc_task);
		ACCESS_ONCE(gc_th->f2fs_gc_task) = NULL;
		wake_up_all(&gc_th->fg_gc_wait);
#ifdef CONFIG_HISI_BLK
		del_timer_sync(&gc_th->nb_timer);
retry:
		if (blk_busy_idle_event_unsubscriber(&gc_th->gc_event_node))
			goto retry;
#endif
	}
}

#ifdef CONFIG_F2FS_TURBO_ZONE
static void recovery_turbo_init(struct f2fs_sb_info *sbi);

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
		if (!tz_info->enabled) {
			f2fs_msg(sbi->sb, KERN_WARNING,
					"f2fs-gc-turbo: turbo gc stopped\n");
			break;
		}

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
				continue;
			}

			if (!mutex_trylock(&sbi->gc_mutex))
				continue;

		} else if (try_to_freeze()) {
			continue;
		} else if (kthread_should_stop()) {
			sb_end_write(sbi->sb);
			break;
		} else if (ret < 0) {
			pr_err("f2fs-gc-turbo: some signals received\n");
			continue;
		} else {
			ssr_gc_count = atomic_read(&sbi->need_ssr_gc);
			if (ssr_gc_count) {
				mutex_lock(&sbi->gc_mutex);
				f2fs_gc(sbi, true, false, true, NULL_SEGNO);
				atomic_sub(ssr_gc_count, &sbi->need_ssr_gc);
			}
			if (!has_not_enough_free_secs(sbi, 0, 0)) {
				wake_up_all(&gc_th->fg_gc_wait);
				continue;
			}

			/* run into FG_GC
			 * we must wait & take sbi->gc_mutex before FG_GC
			 */
			mutex_lock(&sbi->gc_mutex);

			f2fs_gc(sbi, false, false, true, NULL_SEGNO);
			wake_up_all(&gc_th->fg_gc_wait);
			continue;
		}

		if (gc_th->gc_urgent) {
			wait_ms = gc_th->urgent_sleep_time;
			goto do_gc;
		}

#ifdef CONFIG_HISI_BLK
		if (!gc_th->block_idle) {
#else
		if (!is_idle(sbi)) {
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

		recovery_turbo_init(sbi);
next:
		sb_end_write(sbi->sb);

		/*lint -restore*/
	} while (!kthread_should_stop());

	while (!kthread_should_stop())
		cond_resched();

	return 0;
}

#ifdef CONFIG_HISI_BLK
static void set_block_idle_turbo(unsigned long data)
{
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_turbo_thread;

	gc_th->block_idle = true;
}

static enum blk_busy_idle_callback_return
gc_io_busy_idle_notify_handler_turbo(struct blk_busy_idle_event_node *nb,
					enum blk_idle_notify_state state)
{
	enum blk_busy_idle_callback_return ret =
				BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)nb->param_data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_turbo_thread;

	if (gc_th->f2fs_gc_task == NULL)
		return ret;
	switch (state) {

	case BLK_IDLE_NOTIFY:
		mod_timer(&gc_th->nb_timer,
				jiffies + msecs_to_jiffies(IDLE_WT));
		ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
		break;
	case BLK_BUSY_NOTIFY:
		del_timer_sync(&gc_th->nb_timer);
		gc_th->block_idle = false;
		ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
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

	if (!sbi->tz_info.enabled)
		return 0;
	gc_th->urgent_sleep_time = DEF_GC_THREAD_URGENT_SLEEP_TIME;
	gc_th->min_sleep_time = DEF_GC_THREAD_MIN_SLEEP_TIME;
	gc_th->max_sleep_time = DEF_GC_THREAD_MAX_SLEEP_TIME;
	gc_th->no_gc_sleep_time = DEF_GC_THREAD_NOGC_SLEEP_TIME;

	gc_th->gc_wake = 0;
	gc_th->gc_preference = GC_BALANCE;

#ifdef CONFIG_HISI_BLK
	gc_th->block_idle = false;
	setup_timer(&gc_th->nb_timer, set_block_idle_turbo, (unsigned long)sbi);
	strncpy(gc_th->gc_event_node.subscriber_name, "f2fs_gc_turbo", SUBSCRIBER_NAME_LEN);
	gc_th->gc_event_node.blk_busy_idle_notifier_callback =
				gc_io_busy_idle_notify_handler_turbo;
	gc_th->gc_event_node.param_data = sbi;
	err = blk_busy_idle_event_subscriber(sbi->sb->s_bdev, &gc_th->gc_event_node);
#endif

	/* disable atgc for turbo gc */
	gc_th->atgc_enabled = false;
	gc_th->root = RB_ROOT;
	INIT_LIST_HEAD(&gc_th->victim_list);
	gc_th->victim_count = 0;

	gc_th->age_threshold = DEF_GC_THREAD_AGE_THRESHOLD;
	gc_th->dirty_rate_threshold = DEF_GC_THREAD_DIRTY_RATE_THRESHOLD;
	gc_th->dirty_count_threshold = DEF_GC_THREAD_DIRTY_COUNT_THRESHOLD;
	gc_th->age_weight = DEF_GC_THREAD_AGE_WEIGHT;

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
#ifdef CONFIG_HISI_BLK
		del_timer_sync(&gc_th->nb_timer);
retry:
		if (blk_busy_idle_event_unsubscriber(&gc_th->gc_event_node))
			goto retry;
#endif
	}
}
#endif

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

#ifdef CONFIG_F2FS_TURBO_ZONE
	if (dirty_type == CURSEG_TURBO_DATA)
		dirty_type = CURSEG_WARM_DATA;
#endif
	if (p->alloc_mode == SSR) {
		p->gc_mode = GC_GREEDY;
		p->dirty_segmap = dirty_i->dirty_segmap[dirty_type];
		p->max_search = dirty_i->nr_dirty[dirty_type];
		p->ofs_unit = 1;
	} else if (p->alloc_mode == ASSR) {
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
			p->gc_mode != GC_AT &&
			p->alloc_mode != ASSR &&
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
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
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
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	struct rb_node **p;
	struct rb_node *parent = NULL;
	struct victim_entry *ve = NULL;

	p = __lookup_rb_tree_ext(sbi, &gc_th->root, &parent, mtime);
	ve = attach_victim_entry(sbi, mtime, segno, parent, p);
}

static void record_victim_entry(struct f2fs_sb_info *sbi,
			struct victim_sel_policy *p, unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int secno = segno / sbi->segs_per_sec;
	unsigned int start = secno * sbi->segs_per_sec;
	unsigned long long mtime = 0;
	unsigned int i;

	for (i = 0; i < sbi->segs_per_sec; i++)
		mtime += get_seg_entry(sbi, start + i)->mtime;
	mtime = div_u64(mtime, sbi->segs_per_sec);

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
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	struct rb_node *parent = NULL;

	__lookup_rb_tree_ext(sbi, &gc_th->root, &parent, p->age);

	return parent;
}

static void lookup_victim_atgc(struct f2fs_sb_info *sbi,
						struct victim_sel_policy *p)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
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
	f2fs_bug_on(sbi, !vblocks || vblocks == sec_blocks);

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
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
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

bool check_rb_tree_consistence(struct f2fs_sb_info *sbi,
						struct rb_root *root)
{
#ifdef CONFIG_F2FS_CHECK_FS
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
			f2fs_msg(sbi->sb, KERN_ERR, "inconsistent rbtree, "
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
	bool consistent = check_rb_tree_consistence(sbi, &sbi->gc_thread.root);

	WARN_ON(!consistent);

	if (p->gc_mode == GC_AT)
		lookup_victim_atgc(sbi, p);
	else if (p->alloc_mode == ASSR)
		lookup_victim_assr(sbi, p);
	else
		f2fs_bug_on(sbi, 1);
}

void release_victim_entry(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
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
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	struct task_struct *f2fs_gc_task = gc_th->f2fs_gc_task;
#ifdef CONFIG_F2FS_TURBO_ZONE
	unsigned int start_segno = 0;
	unsigned int end_segno;
#endif

	mutex_lock(&dirty_i->seglist_lock);
	last_segment = MAIN_SECS(sbi) * sbi->segs_per_sec;
#ifdef CONFIG_F2FS_TURBO_ZONE
	end_segno = last_segment;
#endif

	p.alloc_mode = alloc_mode;
	p.age = age;
	if (f2fs_gc_task)
		p.age_threshold = sbi->gc_thread.age_threshold;

retry:
	select_policy(sbi, gc_type, type, &p);
	p.min_segno = NULL_SEGNO;
	p.oldest_age = 0;
	p.min_cost = get_max_cost(sbi, &p);
	if (f2fs_gc_task)
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

	/* get victim for GC_AT/ASSR */
	if (is_atgc) {
		lookup_victim_by_time(sbi, &p);
		release_victim_entry(sbi);
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

		trace_f2fs_get_victim(sbi->sb, type, gc_type, &p,
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
	f2fs_wait_on_page_writeback(page, DATA, true);

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

#ifdef CONFIG_F2FS_TURBO_ZONE
	if (is_tz_flag_set(inode, FI_TZ_KEY_FILE))
		type = CURSEG_TURBO_DATA;
#endif
	/* do not read out */
	page = f2fs_grab_cache_page(inode->i_mapping, bidx, false);
	if (!page) {
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
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
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto out;
	}

	if (unlikely(dn.data_blkaddr == NULL_ADDR)) {
		ClearPageUptodate(page);
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		err = -ENOENT;
		goto put_out;
	}

	/*
	 * don't cache encrypted data into meta inode until previous dirty
	 * data were writebacked to avoid racing between GC and flush.
	 */
	f2fs_wait_on_page_writeback(page, DATA, true);

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
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
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
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_page_out;
	}

	/* write page */
	lock_page(fio.encrypted_page);

	if (unlikely(fio.encrypted_page->mapping != META_MAPPING(fio.sbi))) {
		err = -EIO;
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_page_out;
	}
	if (unlikely(!PageUptodate(fio.encrypted_page))) {
		err = -EIO;
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_page_out;
	}

write_page:
	set_page_dirty(fio.encrypted_page);
	f2fs_wait_on_page_writeback(fio.encrypted_page, DATA, true);
	if (clear_page_dirty_for_io(fio.encrypted_page))
		dec_page_count(fio.sbi, F2FS_DIRTY_META);

	set_page_writeback(fio.encrypted_page);
	ClearPageError(page);

	/* allocate block address */
	f2fs_wait_on_page_writeback(dn.node_page, NODE, true);

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
							true, true, true);
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
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
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
		set_page_dirty(page);
		f2fs_wait_on_page_writeback(page, DATA, true);
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
							gc_type, segno, off);
			else
				err = move_data_page(inode, start_bidx, gc_type,
								segno, off);

			if (!err && (gc_type == FG_GC ||
					f2fs_post_read_required(inode)))
				submitted++;

			if (locked) {
				up_write(&fi->i_gc_rwsem[WRITE]);
				up_write(&fi->i_gc_rwsem[READ]);
			}

			stat_inc_data_blk_count(sbi, 1, gc_type);
			if (!err)
				gc_cnt++;
	       } else
		       f2fs_gc_loop_debug(sbi);
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
			turbo ? CURSEG_TURBO_DATA : NO_CHECK_TYPE, LFS, 0);
#else
	ret = DIRTY_I(sbi)->v_ops->get_victim(sbi, victim, gc_type,
					      NO_CHECK_TYPE, LFS, 0);
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
	int seg_freed = 0;
	int hotcold_type = get_seg_entry(sbi, segno)->type;
	unsigned char type = IS_DATASEG(hotcold_type) ?
						SUM_TYPE_DATA : SUM_TYPE_NODE;
	int submitted = 0;

	/* readahead multi ssa blocks those have contiguous address */
	if (sbi->segs_per_sec > 1)
		f2fs_ra_meta_pages(sbi, GET_SUM_BLOCK(sbi, segno),
					sbi->segs_per_sec, META_SSA, true);

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

		if (get_valid_blocks(sbi, segno, false) == 0 ||
				!PageUptodate(sum_page) ||
				unlikely(f2fs_cp_error(sbi)))
			goto next;

		sum = page_address(sum_page);
		if (type != GET_SUM_TYPE((&sum->footer))) {
			f2fs_msg(sbi->sb, KERN_ERR, "Inconsistent segment (%u) "
				"type [%d, %d] in SSA and SIT",
				segno, type, GET_SUM_TYPE((&sum->footer)));
			set_sbi_flag(sbi, SBI_NEED_FSCK);
			goto next;
		}

		/*
		 * this is to avoid deadlock:
		 * - lock_page(sum_page)	 - f2fs_replace_block
		 *  - check_valid_map()		   - down_write(sentry_lock)
		 *   - down_read(sentry_lock)	  - change_curseg()
		 *				    - lock_page(sum_page)
		 */
		if (type == SUM_TYPE_NODE)
			submitted += gc_node_segment(sbi, sum->entries, segno,
								gc_type);
		else
			submitted += gc_data_segment(sbi, sum->entries, gc_list,
							segno, gc_type);

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
		.iroot = RADIX_TREE_INIT(GFP_NOFS),
	};
	unsigned long long last_skipped = sbi->skipped_atomic_files[FG_GC];
	unsigned long long first_skipped;
	unsigned int skipped_round = 0, round = 0;
	int gc_completed = 0;
	u64 fggc_begin = 0, fggc_end;

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
			f2fs_msg(sbi->sb, KERN_ERR,
				"f2fs_gc_loop same victim retry:%lu in %s:%d "
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

	if (gc_type == FG_GC)
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

	if (unlikely(sbi->gc_loop.segmap)) {
		kvfree(sbi->gc_loop.segmap);
		sbi->gc_loop.segmap = NULL;
	}
	if (unlikely(sbi->gc_loop.check))
		init_f2fs_gc_loop(sbi);

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

int __init create_garbage_collection_cache(void)
{
	victim_entry_slab = f2fs_kmem_cache_create("victim_entry",
					sizeof(struct victim_entry));
	if (!victim_entry_slab)
		return -ENOMEM;
	return 0;
}

void destroy_garbage_collection_cache(void)
{
	kmem_cache_destroy(victim_entry_slab);
}


void f2fs_build_gc_manager(struct f2fs_sb_info *sbi)
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
		allocate_segment_for_resize(sbi, type, start, end);

	/* do GC to move out valid blocks in the range */
	for (segno = start; segno <= end; segno += sbi->segs_per_sec) {
		struct gc_inode_list gc_list = {
			.ilist = LIST_HEAD_INIT(gc_list.ilist),
			.iroot = RADIX_TREE_INIT(GFP_NOFS),
		};

		mutex_lock(&sbi->gc_mutex);
		do_garbage_collect(sbi, segno, &gc_list, FG_GC);
		mutex_unlock(&sbi->gc_mutex);
		put_gc_inode(&gc_list);

		if (get_valid_blocks(sbi, segno, true))
			return -EAGAIN;
	}

	err = f2fs_sync_fs(sbi->sb, 1);
	if (err)
		return err;

	next_inuse = find_next_inuse(FREE_I(sbi), end + 1, start);
	if (next_inuse <= end) {
		se = get_seg_entry(sbi, next_inuse);
		f2fs_msg(sbi->sb, KERN_ERR,
			"segno %u should be free but still inuse! type %u, vblks %u, ckpt_vblks %u, IS_CURSEG %d",
			next_inuse, se->type, se->valid_blocks,
			se->ckpt_valid_blocks, IS_CURSEG(sbi, next_inuse));
		err = -EAGAIN;
	}
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

int f2fs_resize_fs(struct f2fs_sb_info *sbi, size_t resize_len)
{
	unsigned int section_size = F2FS_BLKSIZE * BLKS_PER_SEC(sbi);
	unsigned int secs = (resize_len + section_size - 1) / section_size;
	__u64 shrunk_blocks = (__u64)secs * BLKS_PER_SEC(sbi);
	int gc_mode;
	int err = 0;

	if (is_sbi_flag_set(sbi, SBI_NEED_FSCK)) {
		f2fs_msg(sbi->sb, KERN_ERR,
			"Should run fsck to repair first.");
		return -EINVAL;
	}

	if (test_opt(sbi, DISABLE_CHECKPOINT)) {
		f2fs_msg(sbi->sb, KERN_ERR,
			"Checkpoint should be enabled.");
		return -EINVAL;
	}

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

	update_sb_metadata(sbi, -secs);

	err = f2fs_commit_super(sbi, false);
	if (err) {
		update_sb_metadata(sbi, secs);
		goto out;
	}

	update_fs_metadata(sbi, -secs);
	clear_sbi_flag(sbi, SBI_IS_RESIZEFS);
	err = f2fs_sync_fs(sbi->sb, 1);
	if (err) {
		update_fs_metadata(sbi, secs);
		update_sb_metadata(sbi, secs);
		f2fs_commit_super(sbi, false);
	}
out:
	if (err) {
		set_sbi_flag(sbi, SBI_NEED_FSCK);
		f2fs_msg(sbi->sb, KERN_ERR,
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

	while (map.m_lblk < file_blocks) {
		map.m_len = file_blocks - map.m_lblk;
		err = f2fs_map_blocks(inode, &map, 0, F2FS_GET_BLOCK_FIEMAP);
		f2fs_msg(sbi->sb, KERN_INFO,
			"migrate: map_blocks err %d: lblk %u, pblk %u, len %u",
				err, map.m_lblk, map.m_pblk, map.m_len);

		if (err)
			return err;

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

			if (f2fs_encrypted_file(inode))
				err = move_data_block(inode, map.m_lblk + off,
							FG_GC, segno, segoff);
			else
				err = move_data_page(inode, map.m_lblk + off,
							FG_GC, segno, segoff);
			if (err) {
				f2fs_msg(sbi->sb, KERN_WARNING,
						"migrate: move bidx %u, err %d",
						map.m_lblk + off, err);
				return err;
			}

			submitted++;

			if (fatal_signal_pending(current)) {
				f2fs_msg(sbi->sb, KERN_WARNING,
						"migrate: signal received when move bidx %u, submitted %d",
						map.m_lblk + off, err);
				return do_move ? submitted : 0;
			}
		}
skip:
		map.m_lblk += (map.m_len ? map.m_len : 1);
	}
	return do_move ? submitted : 0;
}

static void recovery_turbo_init(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_TURBO_DATA);

	if (!curseg->inited) {
		if (f2fs_sync_fs(sbi->sb, 1))
			return;
		init_turbo_curseg(sbi);
	}
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

	if (ret < 0)
		return ret;

	if (ret > 0)
		f2fs_submit_merged_write(sbi, DATA);

	if (sync) {
		ret = filemap_write_and_wait_range(inode->i_mapping,
						0, i_size_read(inode));
		if (ret < 0)
			return ret;
		ret = check_data_blocks_in_tz(inode, turbo, false);
		if (ret < 0)
			return ret;
	}

	recovery_turbo_init(sbi);

	return 0;
}
#endif
