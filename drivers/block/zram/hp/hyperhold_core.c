/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold implement
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
 * Author:	He Biao <hebiao6@huawei.com>
 *		Wang Cheng Ke <wangchengke2@huawei.com>
 *		Wang Fa <fa.wang@huawei.com>
 *
 * Create: 2020-4-16
 *
 */
#define pr_fmt(fmt) "[HYPERHOLD]" fmt

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sched/task.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/memcontrol.h>
#include <linux/memcg_policy.h>
#include <linux/hyperhold_inf.h>

#include "zram_drv.h"
#include "hyperhold.h"
#include "hyperhold_internal.h"

struct async_req {
	struct mem_cgroup *mcg;
	unsigned long size;
	struct work_struct work;
	int nice;
	bool preload;
};

struct io_priv {
	struct zram *zram;
	enum hyperhold_scenario scenario;
	struct hyperhold_page_pool page_pool;
};

struct schedule_para {
	void *io_handler;
	struct hyperhold_entry *io_entry;
	struct hyperhold_buffer io_buf;
	struct io_priv priv;
	struct hyperhold_key_point_record record;
};

#ifdef CONFIG_HYPERHOLD
static void hyperhold_memcg_iter(
	int (*iter)(struct mem_cgroup *, void *), void *data)
{
	struct mem_cgroup *mcg = get_next_memcg(NULL);

	while (mcg) {
		if (iter(mcg, data)) {
			get_next_memcg_break(mcg);
			return;
		}
		mcg = get_next_memcg(mcg);
	}
}

#else
static void hyperhold_memcg_iter(
	int (*iter)(struct mem_cgroup *, void *), void *data)
{
}

#endif

/*
 * This interface will be called when anon page is added to ZRAM.
 * Hyperhold should trace this ZRAM in memcg LRU list.
 *
 */
void hyperhold_track(struct zram *zram, u32 index,
				struct mem_cgroup *memcg)
{
	if (!hyperhold_enable())
		return;

	/*
	 * 1. Add ZRAM obj into LRU.
	 * 2. Updata the stored size in memcg and ZRAM.
	 * 3. ZRAM capcity pressure check & report
	 *
	 */
	if (unlikely(!memcg))
		return;

	if (unlikely(!memcg->zram)) {
		spin_lock(&memcg->zram_init_lock);
		if (!memcg->zram)
			hyperhold_manager_memcg_init(memcg, zram);
		spin_unlock(&memcg->zram_init_lock);
	}

	hyperhold_zram_lru_add(zram, index, memcg);

#ifdef CONFIG_ZSWAPD
	zram_slot_unlock(zram, index);
	if (!zram_watermark_ok())
		wake_all_zswapd();
	zram_slot_lock(zram, index);
#endif
}

/*
 * This interface will be called when anon page in ZRAM is freed.
 *
 */
void hyperhold_untrack(struct zram *zram, u32 index)
{
	/*
	 * 1. When the ZS object in the writeback or swapin,
	 * it can't be untrack.
	 * 2. Updata the stored size in memcg and ZRAM.
	 * 3. Remove ZRAM obj from LRU.
	 *
	 */
	if (!hyperhold_enable())
		return;

	while (zram_test_flag(zram, index, ZRAM_UNDER_WB) ||
		zram_test_flag(zram, index, ZRAM_BATCHING_OUT)) {
		zram_slot_unlock(zram, index);
		udelay(50);
		zram_slot_lock(zram, index);
	}

	hyperhold_zram_lru_del(zram, index);
}

static unsigned long memcg_reclaim_size(struct mem_cgroup *memcg)
{
	unsigned long zram_size = atomic64_read(&memcg->zram_stored_size);
	unsigned long cur_size = atomic64_read(&memcg->hyperhold_stored_size);
	unsigned long new_size = (zram_size + cur_size) *
		atomic_read(&memcg->memcg_reclaimed.ub_zram2ufs_ratio) / 100;

	if (memcg->force_swapout)
		return zram_size;

	if (task_hyperhold_idle_reclaim(current))
		return atomic64_read(&memcg->zram_idle_size);

	return (new_size > cur_size) ? (new_size - cur_size) : 0;
}

static int hyperhold_permcg_sz(struct mem_cgroup *memcg, void *data)
{
	unsigned long *out_size = (unsigned long *)data;

	*out_size += memcg_reclaim_size(memcg);
	return 0;
}

static void hyperhold_flush_cb(enum hyperhold_scenario scenario, void *pri)
{
	switch (scenario) {
	case HYPERHOLD_FAULT_OUT:
	case HYPERHOLD_PRE_OUT:
	case HYPERHOLD_BATCH_OUT:
		hyperhold_extent_destroy(pri, scenario);
		break;
	case HYPERHOLD_RECLAIM_IN:
		hyperhold_extent_register(pri);
		break;
	default:
		break;
	}
}

static void hyperhold_flush_done(struct hyperhold_entry *io_entry, int err)
{
	struct io_priv *priv = (struct io_priv *)(io_entry->private);

	if (likely(!err)) {
		hyperhold_flush_cb(priv->scenario,
				io_entry->manager_private);
#ifdef CONFIG_ZSWAPD
		if (!zram_watermark_ok())
			wake_all_zswapd();
#endif

	} else {
		hyperhold_extent_exception(priv->scenario,
				io_entry->manager_private);
	}
	hyperhold_free(io_entry);
}

static void hyperhold_free_pagepool(struct schedule_para *sched)
{
	struct page *free_page = NULL;

	spin_lock(&sched->priv.page_pool.page_pool_lock);
	while (!list_empty(&sched->priv.page_pool.page_pool_list)) {
		free_page = list_first_entry(
			&sched->priv.page_pool.page_pool_list,
				struct page, lru);
		list_del_init(&free_page->lru);
		__free_page(free_page);
	}
	spin_unlock(&sched->priv.page_pool.page_pool_lock);
}

static void hyperhold_plug_complete(void *data)
{
	struct schedule_para *sched  = (struct schedule_para *)data;

	hyperhold_free_pagepool(sched);

	hyperhold_perf_end(&sched->record);

	hyperhold_free(sched);
}

static void *hyperhold_init_plug(struct zram *zram,
				enum hyperhold_scenario scenario,
				struct schedule_para *sched)
{
	struct hyperhold_io io_para;

	io_para.bdev = zram->bdev;
	io_para.scenario = scenario;
	io_para.private = (void *)sched;
	io_para.record = &sched->record;
	INIT_LIST_HEAD(&sched->priv.page_pool.page_pool_list);
	spin_lock_init(&sched->priv.page_pool.page_pool_lock);
	io_para.done_callback = hyperhold_flush_done;
	switch (io_para.scenario) {
	case HYPERHOLD_RECLAIM_IN:
		io_para.complete_notify = hyperhold_plug_complete;
		sched->io_buf.pool = NULL;
		break;
	case HYPERHOLD_BATCH_OUT:
	case HYPERHOLD_PRE_OUT:
		io_para.complete_notify = hyperhold_plug_complete;
		sched->io_buf.pool = &sched->priv.page_pool;
		break;
	case HYPERHOLD_FAULT_OUT:
		io_para.complete_notify = NULL;
		sched->io_buf.pool = NULL;
		break;
	default:
		break;
	}
	sched->io_buf.zram = zram;
	sched->priv.zram = zram;
	sched->priv.scenario = io_para.scenario;
	return hyperhold_plug_start(&io_para);
}

static void hyperhold_fill_entry(struct hyperhold_entry *io_entry,
					struct hyperhold_buffer *io_buf,
					void *private)
{
	io_entry->addr = io_entry->ext_id * EXTENT_SECTOR_SIZE +
		hyperhold_get_start_sector();
	io_entry->dest_pages = io_buf->dest_pages;
	io_entry->pages_sz = EXTENT_PG_CNT;
	io_entry->private = private;
}

static int hyperhold_reclaim_check(struct mem_cgroup *memcg,
					unsigned long *require_size,
					unsigned long *mcg_reclaim_sz)
{
	if (unlikely(!memcg->zram))
		return -EAGAIN;
	if (unlikely(memcg->in_swapin))
		return -EAGAIN;
	if (!memcg->force_swapout && *require_size < MIN_RECLAIM_ZRAM_SZ)
		return -EIO;

	*mcg_reclaim_sz = memcg_reclaim_size(memcg);

	if (!memcg->force_swapout && *mcg_reclaim_sz < MIN_RECLAIM_ZRAM_SZ)
		return -EAGAIN;

	return 0;
}

static int hyperhold_update_reclaim_sz(unsigned long *require_size,
						unsigned long *mcg_reclaim_sz,
						unsigned long reclaim_size)
{
	if (*require_size > reclaim_size)
		*require_size -= reclaim_size;
	else
		*require_size = 0;
	if (*mcg_reclaim_sz > reclaim_size)
		*mcg_reclaim_sz -= reclaim_size;
	else
		return -EINVAL;

	if (*mcg_reclaim_sz < MIN_RECLAIM_ZRAM_SZ)
		return -EAGAIN;
	return 0;
}

static void hyperhold_stat_alloc_fail(enum hyperhold_scenario scenario,
	int err)
{
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat || (err != -ENOMEM) || (scenario >= HYPERHOLD_SCENARIO_BUTT))
		return;

	atomic64_inc(&stat->alloc_fail_cnt[scenario]);
}

static int hyperhold_reclaim_extent(struct mem_cgroup *memcg,
					struct schedule_para *sched,
					unsigned long *require_size,
					unsigned long *mcg_reclaim_sz,
					int *io_err)
{
	int ret;
	unsigned long reclaim_size;

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_IOENTRY_ALLOC);
	sched->io_entry = hyperhold_malloc(
		sizeof(struct hyperhold_entry), false, true);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_IOENTRY_ALLOC);
	if (unlikely(!sched->io_entry)) {
		hh_print(HHLOG_ERR, "alloc io entry failed!\n");
		*require_size = 0;
		*io_err = -ENOMEM;
		hyperhold_stat_alloc_fail(HYPERHOLD_RECLAIM_IN, -ENOMEM);

		return *io_err;
	}

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_FIND_EXTENT);
	reclaim_size = hyperhold_extent_create(
			memcg, &sched->io_entry->ext_id,
			&sched->io_buf,
			&sched->io_entry->manager_private);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_FIND_EXTENT);
	if (unlikely(!reclaim_size)) {
		if (sched->io_entry->ext_id != -ENOENT)
			*require_size = 0;
		hyperhold_free(sched->io_entry);
		return -EAGAIN;
	}

	hyperhold_fill_entry(sched->io_entry, &sched->io_buf,
				(void *)(&sched->priv));

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_IO_EXTENT);
	ret = hyperhold_write_extent(sched->io_handler,
						sched->io_entry);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_IO_EXTENT);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR,
			"hyperhold write failed! %d\n", ret);
		*require_size = 0;
		*io_err = ret;
		hyperhold_stat_alloc_fail(HYPERHOLD_RECLAIM_IN, ret);

		return *io_err;
	}

	ret = hyperhold_update_reclaim_sz(require_size,
				mcg_reclaim_sz, reclaim_size);
	if (memcg->force_swapout)
		return 0;

	return ret;
}

static int hyperhold_permcg_reclaim(
			struct mem_cgroup *memcg, void *data)
{
	/*
	 * 1. Recaim one extent from ZRAM in cur memcg.
	 * 2. If extent exist, dispatch it to UFS.
	 *
	 */
	int ret, extcnt;
	int io_err = 0;
	unsigned long *require_size = (unsigned long *)data;
	unsigned long mcg_reclaim_sz;
	struct schedule_para *sched = NULL;
	ktime_t start = ktime_get();
	unsigned long long start_ravg_sum = hyperhold_get_ravg_sum();

	ret = hyperhold_reclaim_check(memcg, require_size, &mcg_reclaim_sz);
	if (ret)
		return ret == -EAGAIN ? 0 : ret;

	sched = hyperhold_malloc(sizeof(struct schedule_para), false, true);
	if (unlikely(!sched)) {
		hh_print(HHLOG_ERR, "alloc sched failed!\n");
		hyperhold_stat_alloc_fail(HYPERHOLD_RECLAIM_IN, -ENOMEM);

		return -ENOMEM;
	}

	hyperhold_perf_start(&sched->record, start, start_ravg_sum,
		HYPERHOLD_RECLAIM_IN);

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_INIT);
	sched->io_handler = hyperhold_init_plug(memcg->zram,
				HYPERHOLD_RECLAIM_IN, sched);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_INIT);
	if (unlikely(!sched->io_handler)) {
		hh_print(HHLOG_ERR, "plug start failed!\n");
		hyperhold_perf_end(&sched->record);
		hyperhold_free(sched);
		hyperhold_stat_alloc_fail(HYPERHOLD_RECLAIM_IN, -ENOMEM);
		ret = -EIO;
		goto out;
	}

	while (*require_size) {
		if (hyperhold_reclaim_extent(memcg, sched,
			require_size, &mcg_reclaim_sz, &io_err))
			break;
		atomic64_inc(&memcg->hyperhold_outextcnt);
		extcnt = atomic_inc_return(&memcg->hyperhold_extcnt);
		if (extcnt > atomic_read(&memcg->hyperhold_peakextcnt))
			atomic_set(&memcg->hyperhold_peakextcnt, extcnt);
	}
	ret = hyperhold_plug_finish(sched->io_handler);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "hyperhold write flush failed! %d\n", ret);
		hyperhold_stat_alloc_fail(HYPERHOLD_RECLAIM_IN, ret);
		*require_size = 0;
	} else {
		ret = io_err;
	}
	atomic64_inc(&memcg->hyperhold_outcnt);
out:
	return ret;
}

static void hyperhold_reclaimin_inc(void)
{
	struct hyperhold_stat *stat;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat))
		return;
	atomic64_inc(&stat->reclaimin_infight);
}

static void hyperhold_reclaimin_dec(void)
{
	struct hyperhold_stat *stat;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat))
		return;
	atomic64_dec(&stat->reclaimin_infight);
}

static void hyperhold_reclaim_work(struct work_struct *work)
{
	/*
	 * 1. Set process prio.
	 * 2. Reclaim object from each memcg.
	 *
	 */
	struct async_req *rq = container_of(work, struct async_req, work);
	int old_nice = task_nice(current);

	set_user_nice(current, rq->nice);
	hyperhold_reclaimin_inc();

	hyperhold_memcg_iter(hyperhold_permcg_reclaim, &rq->size);

	hyperhold_reclaimin_dec();
	set_user_nice(current, old_nice);
	hyperhold_free(rq);
}

/*
 * This interface is for anon mem reclaim.
 *
 */
unsigned long hyperhold_reclaim_in(unsigned long size)
{
	/*
	 * 1. Estimate push in size.
	 * 2. Wakeup push in worker.
	 *
	 */
	struct async_req *rq = NULL;
	unsigned long out_size = 0;

	if (!hyperhold_enable() || !hyperhold_reclaim_in_enable())
		return 0;

	hyperhold_memcg_iter(hyperhold_permcg_sz, &out_size);

	rq = hyperhold_malloc(sizeof(struct async_req), false, true);
	if (unlikely(!rq)) {
		hh_print(HHLOG_ERR, "alloc async req fail!\n");
		hyperhold_stat_alloc_fail(HYPERHOLD_RECLAIM_IN, -ENOMEM);

		return 0;
	}
	rq->size = size;
	rq->nice = task_nice(current);
	INIT_WORK(&rq->work, hyperhold_reclaim_work);
	queue_work(hyperhold_get_reclaim_workqueue(), &rq->work);

	return out_size > size ? size : out_size;
}

static int hyperhold_batch_out_extent(struct schedule_para *sched,
						struct mem_cgroup *mcg,
						bool preload,
						int *io_err)
{
	int ret;

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_IOENTRY_ALLOC);
	sched->io_entry = hyperhold_malloc(
		sizeof(struct hyperhold_entry), !preload, preload);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_IOENTRY_ALLOC);
	if (unlikely(!sched->io_entry)) {
		hh_print(HHLOG_ERR, "alloc io entry failed!\n");
		*io_err = -ENOMEM;
		hyperhold_stat_alloc_fail(HYPERHOLD_BATCH_OUT, -ENOMEM);

		return *io_err;
	}

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_FIND_EXTENT);
	sched->io_entry->ext_id = hyperhold_find_extent_by_memcg(
				mcg, &sched->io_buf,
				&sched->io_entry->manager_private);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_FIND_EXTENT);
	if (sched->io_entry->ext_id < 0) {
		hyperhold_stat_alloc_fail(HYPERHOLD_BATCH_OUT,
			sched->io_entry->ext_id);
		hyperhold_free(sched->io_entry);
		return -EAGAIN;
	}

	hyperhold_fill_entry(sched->io_entry, &sched->io_buf,
				(void *)(&sched->priv));

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_IO_EXTENT);
	ret = hyperhold_read_extent(sched->io_handler, sched->io_entry);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_IO_EXTENT);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR,
				"hyperhold read failed! %d\n", ret);
		hyperhold_stat_alloc_fail(HYPERHOLD_BATCH_OUT, ret);
		*io_err = ret;

		return *io_err;
	}

	return 0;
}

static int hyperhold_do_batch_out_init(struct schedule_para **out_sched,
	struct mem_cgroup *mcg, bool preload)
{
	struct schedule_para *sched = NULL;
	ktime_t start = ktime_get();
	unsigned long long start_ravg_sum = hyperhold_get_ravg_sum();

	sched = hyperhold_malloc(sizeof(struct schedule_para),
						!preload, preload);
	if (unlikely(!sched)) {
		hh_print(HHLOG_ERR, "alloc sched failed!\n");
		hyperhold_stat_alloc_fail(HYPERHOLD_BATCH_OUT, -ENOMEM);

		return -ENOMEM;
	}

	hyperhold_perf_start(&sched->record, start, start_ravg_sum,
		preload ? HYPERHOLD_PRE_OUT : HYPERHOLD_BATCH_OUT);

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_INIT);
	sched->io_handler = hyperhold_init_plug(mcg->zram,
		preload ? HYPERHOLD_PRE_OUT : HYPERHOLD_BATCH_OUT,
		sched);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_INIT);
	if (unlikely(!sched->io_handler)) {
		hh_print(HHLOG_ERR, "plug start failed!\n");
		hyperhold_perf_end(&sched->record);
		hyperhold_free(sched);
		hyperhold_stat_alloc_fail(HYPERHOLD_BATCH_OUT, -ENOMEM);

		return -EIO;
	}

	*out_sched = sched;

	return 0;
}

/*
 * This interface is for app warm start.
 *
 */
static int hyperhold_do_batch_out(struct mem_cgroup *mcg,
				unsigned long size, bool preload)
{
	/*
	 * 1. Get all the extent ids from the memcg.
	 * 2. Send extents to UFS one by one.
	 *
	 */
	int ret = 0;
	int io_err = 0;
	struct schedule_para *sched = NULL;

	if (unlikely(!mcg || !mcg->zram)) {
		hh_print(HHLOG_ERR, "no zram in mcg!\n");
		ret = -EINVAL;
		goto out;
	}

	ret = hyperhold_do_batch_out_init(&sched, mcg, preload);
	if (unlikely(ret))
		goto out;

	mcg->in_swapin = true;

	if (!size)
		size = mcg->zram->nr_pages << PAGE_SHIFT;
	while (size >= EXTENT_SIZE) {
		if (hyperhold_batch_out_extent(sched, mcg, preload, &io_err))
			break;
		size -= EXTENT_SIZE;
		atomic64_inc(&mcg->hyperhold_inextcnt);
		atomic_dec(&mcg->hyperhold_extcnt);
	}

	ret = hyperhold_plug_finish(sched->io_handler);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR,
			"hyperhold read flush failed! %d\n", ret);
		hyperhold_stat_alloc_fail(HYPERHOLD_BATCH_OUT, ret);
	} else {
		ret = io_err;
	}

	atomic64_inc(&mcg->hyperhold_incnt);
	mcg->in_swapin = false;
out:
	return ret;
}

static void hyperhold_batchout_inc(void)
{
	struct hyperhold_stat *stat;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat))
		return;
	atomic64_inc(&stat->batchout_inflight);
}

static void hyperhold_batchout_dec(void)
{
	struct hyperhold_stat *stat;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat))
		return;
	atomic64_dec(&stat->batchout_inflight);
}

int hyperhold_batch_out(struct mem_cgroup *mcg,
				unsigned long size, bool preload)
{
	int ret;

	if (!hyperhold_enable())
		return 0;

	hyperhold_batchout_inc();
	ret = hyperhold_do_batch_out(mcg, size, preload);
	hyperhold_batchout_dec();

	return ret;
}

static void hyperhold_fault_stat(struct zram *zram, u32 index)
{
	struct mem_cgroup *mcg = NULL;
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (unlikely(!stat))
		return;

	atomic64_inc(&stat->fault_cnt);

	mcg = hyperhold_zram_get_memcg(zram, index);
	if (mcg)
		atomic64_inc(&mcg->hyperhold_allfaultcnt);
}

static void hyperhold_fault2_stat(struct zram *zram, u32 index)
{
	struct mem_cgroup *mcg = NULL;
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (unlikely(!stat))
		return;

	atomic64_inc(&stat->hyperhold_fault_cnt);

	mcg = hyperhold_zram_get_memcg(zram, index);
	if (mcg)
		atomic64_inc(&mcg->hyperhold_faultcnt);
}

static bool hyperhold_fault_out_check(struct zram *zram,
					u32 index, unsigned long *zentry)
{
	if (!hyperhold_enable())
		return false;

	hyperhold_fault_stat(zram, index);

	if (!zram_test_flag(zram, index, ZRAM_WB))
		return false;

	zram_set_flag(zram, index, ZRAM_BATCHING_OUT);
#ifdef CONFIG_ZRAM_DEDUP
	*zentry = zram_get_direct_handle(zram, index);
#else
	*zentry = zram_get_handle(zram, index);
#endif
	zram_slot_unlock(zram, index);
	return true;
}

static int hyperhold_fault_out_get_extent(struct zram *zram,
						struct schedule_para *sched,
						unsigned long zentry,
						u32 index)
{
	sched->io_buf.zram = zram;
	sched->priv.zram = zram;
	sched->io_buf.pool = NULL;
	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_FIND_EXTENT);
	sched->io_entry->ext_id = hyperhold_find_extent_by_idx(zentry,
			&sched->io_buf, &sched->io_entry->manager_private);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_FIND_EXTENT);
	if (unlikely(sched->io_entry->ext_id == -EBUSY)) {
		/* The extent maybe in unexpected case, wait here */
		while (1) {
			/* The extent doesn't exist in hyperhold */
			zram_slot_lock(zram, index);
			if (!zram_test_flag(zram, index, ZRAM_WB)) {
				zram_slot_unlock(zram, index);
				hyperhold_free(sched->io_entry);
				return -EAGAIN;
			}
			zram_slot_unlock(zram, index);

			/* Get extent again */
			hyperhold_perf_lat_start(&sched->record,
				HYPERHOLD_FIND_EXTENT);
			sched->io_entry->ext_id =
			hyperhold_find_extent_by_idx(zentry,
				&sched->io_buf,
				&sched->io_entry->manager_private);
			hyperhold_perf_lat_end(&sched->record,
				HYPERHOLD_FIND_EXTENT);
			if (likely(sched->io_entry->ext_id != -EBUSY))
				break;
			udelay(50);
		}
	}
	if (sched->io_entry->ext_id < 0) {
		hyperhold_stat_alloc_fail(HYPERHOLD_FAULT_OUT,
			sched->io_entry->ext_id);

		return sched->io_entry->ext_id;
	}
	hyperhold_fault2_stat(zram, index);
	hyperhold_fill_entry(sched->io_entry, &sched->io_buf,
					(void *)(&sched->priv));
	return 0;
}

static int hyperhold_fault_out_exit_check(struct zram *zram,
						u32 index, int ret)
{
	zram_slot_lock(zram, index);
	if (likely(!ret)) {
		if (unlikely(zram_test_flag(zram, index, ZRAM_WB))) {
			hh_print(HHLOG_ERR, "still in WB status!\n");
			ret = -EIO;
		}
	}
	zram_clear_flag(zram, index, ZRAM_BATCHING_OUT);

	return ret;
}

static int hyperhold_fault_out_extent(struct zram *zram, u32 index,
	struct schedule_para *sched, unsigned long zentry)
{
	int ret;

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_IOENTRY_ALLOC);
	sched->io_entry = hyperhold_malloc(sizeof(struct hyperhold_entry),
						true, true);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_IOENTRY_ALLOC);
	if (unlikely(!sched->io_entry)) {
		hh_print(HHLOG_ERR, "alloc io entry failed!\n");
		hyperhold_stat_alloc_fail(HYPERHOLD_FAULT_OUT, -ENOMEM);
		hyperhold_fail_record(HYPERHOLD_FAULT_OUT_ENTRY_ALLOC_FAIL,
			index, 0, sched->record.task_comm);
		return -ENOMEM;
	}

	ret = hyperhold_fault_out_get_extent(zram, sched, zentry, index);
	if (ret)
		return ret;

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_IO_EXTENT);
	ret = hyperhold_read_extent(sched->io_handler, sched->io_entry);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_IO_EXTENT);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "hyperhold read failed! %d\n", ret);
		hyperhold_stat_alloc_fail(HYPERHOLD_FAULT_OUT, ret);
	}

	return ret;
}

/*
 * This interface will be called when ZRAM is read.
 * Hyperhold should be searched before ZRAM is read.
 * This function require ZRAM slot lock being held.
 *
 */
int hyperhold_fault_out(struct zram *zram, u32 index)
{
	/*
	 * 1. Find the extent in ZRAM by the index.
	 * 2. if extent exist, dispatch it to UFS.
	 *
	 */
	int ret = 0;
	int io_err;
	struct schedule_para sched;
	unsigned long zentry;
	ktime_t start = ktime_get();
	unsigned long long start_ravg_sum = hyperhold_get_ravg_sum();

	if (!hyperhold_fault_out_check(zram, index, &zentry))
		return ret;

	memset(&sched.record, 0, sizeof(struct hyperhold_key_point_record));
	hyperhold_perf_start(&sched.record, start, start_ravg_sum,
		HYPERHOLD_FAULT_OUT);

	hyperhold_perf_lat_start(&sched.record, HYPERHOLD_INIT);
	sched.io_handler = hyperhold_init_plug(zram,
				HYPERHOLD_FAULT_OUT, &sched);
	hyperhold_perf_lat_end(&sched.record, HYPERHOLD_INIT);
	if (unlikely(!sched.io_handler)) {
		hh_print(HHLOG_ERR, "plug start failed!\n");
		hyperhold_stat_alloc_fail(HYPERHOLD_FAULT_OUT, -ENOMEM);
		ret = -EIO;
		hyperhold_fail_record(HYPERHOLD_FAULT_OUT_INIT_FAIL,
			index, 0, sched.record.task_comm);

		goto out;
	}

	io_err = hyperhold_fault_out_extent(zram, index, &sched, zentry);
	ret = hyperhold_plug_finish(sched.io_handler);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "hyperhold flush failed! %d\n", ret);
		hyperhold_stat_alloc_fail(HYPERHOLD_FAULT_OUT, ret);
	} else {
		ret = (io_err != -EAGAIN) ? io_err : 0;
	}
out:
	hyperhold_perf_lat_start(&sched.record, HYPERHOLD_ZRAM_LOCK);
	ret = hyperhold_fault_out_exit_check(zram, index, ret);
	hyperhold_perf_lat_end(&sched.record, HYPERHOLD_ZRAM_LOCK);
	hyperhold_perf_end(&sched.record);

	return ret;
}

/*
 * This interface will be called when ZRAM is freed.
 * Hyperhold should be deleted before ZRAM is freed.
 * If obj can be deleted from ZRAM,
 * return true, otherwise return false.
 *
 */
bool hyperhold_delete(struct zram *zram, u32 index)
{
	/*
	 * 1. Find the extent in ZRAM by the index.
	 * 2. Delete the zs Object in the extent.
	 * 3. If extent is empty, free the extent.
	 *
	 */

	if (!hyperhold_enable())
		return true;

	if (zram_test_flag(zram, index, ZRAM_UNDER_WB)
		|| zram_test_flag(zram, index, ZRAM_BATCHING_OUT)) {
		struct hyperhold_stat *stat = hyperhold_get_stat_obj();

		if (stat)
			atomic64_inc(&stat->miss_free);
		return false;
	}

	if (!zram_test_flag(zram, index, ZRAM_WB))
		return true;

	hyperhold_extent_objs_del(zram, index);

	return true;
}

void hyperhold_mem_cgroup_remove(struct mem_cgroup *memcg)
{
	if (!hyperhold_enable())
		return;
	hyperhold_manager_memcg_deinit(memcg);
}

void hyperhold_force_reclaim(struct mem_cgroup *mcg)
{
	unsigned long size;

	if (!mcg || !mcg->zram)
		return;

	if (!hyperhold_enable() || !hyperhold_reclaim_in_enable())
		return;

	size = atomic64_read(&mcg->zram_stored_size);

	mcg->force_swapout = true;
	hyperhold_permcg_reclaim(mcg, &size);
	mcg->force_swapout = false;
}

void hyperhold_idle_inc(struct zram *zram, u32 index)
{
	if (!hyperhold_enable())
		return;

	if (zram_test_flag(zram, index, ZRAM_WB))
		return;

	if (!zram_test_flag(zram, index, ZRAM_IDLE))
		memcg_idle_inc(zram, index);
}

void hyperhold_idle_dec(struct zram *zram, u32 index)
{
	if (!hyperhold_enable())
		return;

	if (zram_test_flag(zram, index, ZRAM_WB))
		return;

	if (zram_test_flag(zram, index, ZRAM_IDLE))
		memcg_idle_dec(zram, index);
}

void hyperhold_idle_reclaim(void)
{
	struct mem_cgroup *memcg = NULL;
	unsigned long idle_size;

	if (!hyperhold_enable() || !hyperhold_reclaim_in_enable())
		return;

	hh_print(HHLOG_DEBUG, "idle reclaim start.\n");
	task_set_hyperhold_idle_reclaim(current);
	for (memcg = get_next_memcg(NULL); memcg; memcg = get_next_memcg(memcg))
		if (memcg == root_mem_cgroup ||
				!strcmp(memcg->name, "activityApp")) {
			idle_size = atomic64_read(&memcg->zram_idle_size);
			hyperhold_permcg_reclaim(memcg, &idle_size);
		}
	task_clear_hyperhold_idle_reclaim(current);
	hh_print(HHLOG_DEBUG, "idle reclaim end.\n");
}

#ifdef CONFIG_HISI_DEBUG_FS
static int hyperhold_in_proc_ft(struct file *file)
{
	struct task_struct *task = NULL;
	int ret;
	unsigned long size = FT_RECLAIM_SZ * MIN_RECLAIM_ZRAM_SZ;

	task = get_task_from_proc(file->f_path.dentry->d_inode);
	if (unlikely(!task)) {
		hh_print(HHLOG_ERR, "get_proc_task failed!\n");
		return -ESRCH;
	}

	if (!hyperhold_reclaim_in_enable()) {
		hh_print(HHLOG_ERR, "reclaim_in disable!\n");
		return -ESRCH;
	}

	ret = hyperhold_permcg_reclaim(mem_cgroup_from_task(task), &size);
	put_task_struct(task);
	return ret;
}

static int hyperhold_out_proc_ft(struct file *file, bool preload)
{
	struct task_struct *task = NULL;
	int ret;

	task = get_task_from_proc(file->f_path.dentry->d_inode);
	if (unlikely(!task)) {
		hh_print(HHLOG_ERR, "get_proc_task failed!\n");
		return -ESRCH;
	}
	ret = hyperhold_batch_out(mem_cgroup_from_task(task),
			FT_RECLAIM_SZ * MIN_RECLAIM_ZRAM_SZ, preload);
	put_task_struct(task);
	return ret;
}

static ssize_t hyperhold_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	char buffer[200];
	int ret;
	unsigned long val;

	if (!hyperhold_enable())
		return count;

	memset(buffer, 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;

	if (unlikely(copy_from_user(buffer, buf, count)))
		return -EFAULT;
	ret = kstrtoul(buffer, 0, &val);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "val is error!\n");
		return -EINVAL;
	}

	switch (val) {
	/* reclaim in simulate */
	case 1:
		ret = hyperhold_in_proc_ft(file);
		break;
	/* batch out simulate */
	case 2:
		ret = hyperhold_out_proc_ft(file, false);
		break;
	/* pre out simulate */
	case 3:
		ret = hyperhold_out_proc_ft(file, true);
		break;
	default:
		break;
	}

	return ret ? ret : count;
}

const struct file_operations proc_hyperhold_operations = {
	.write		= hyperhold_write,
	.llseek		= noop_llseek,
};
#endif
