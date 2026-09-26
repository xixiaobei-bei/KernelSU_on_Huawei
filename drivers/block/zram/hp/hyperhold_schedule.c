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
#include <linux/kref.h>
#include <linux/bio.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/sched/task.h>
#include <linux/list.h>
#include <linux/random.h>
#include <linux/zsmalloc.h>
#include <linux/delay.h>
#include <linux/fscrypt_common.h>
#include "hyperhold_internal.h"

/* default key index is zero */
#define HYPERHOLD_KEY_INDEX 0
#define HYPERHOLD_KEY_SIZE 64
#define HYPERHOLD_KEY_INDEX_SHIFT 3

#define HYPERHOLD_MAX_INFILGHT_NUM 256

#define HYPERHOLD_SECTOR_SHIFT 9
#define HYPERHOLD_PAGE_SIZE_SECTOR (PAGE_SIZE >> HYPERHOLD_SECTOR_SHIFT)

#define HYPERHOLD_READ_TIME 10
#define HYPERHOLD_WRITE_TIME 100
#define HYPERHOLD_FAULT_OUT_TIME 10

struct hyperhold_io_req {
	struct hyperhold_io io_para;
	struct kref refcount;
	struct mutex refmutex;
	struct wait_queue_head io_wait;
	atomic_t extent_inflight;
	struct completion io_end_flag;
	struct hyperhold_segment *segment;
	bool limit_inflight_flag;
	bool wait_io_finish_flag;
	int page_cnt;
	int segment_cnt;
	int nice;
};

struct hyperhold_segment_time {
	ktime_t submit_bio;
	ktime_t end_io;
};

struct hyperhold_segment {
	sector_t segment_sector;
	int extent_cnt;
	int page_cnt;
	struct list_head io_entries;
	struct hyperhold_entry *io_entries_fifo[BIO_MAX_PAGES];
	struct work_struct endio_work;
	struct hyperhold_io_req *req;
	struct hyperhold_segment_time time;
	u32 bio_result;
};

static u8 hyperhold_io_key[HYPERHOLD_KEY_SIZE];
static u8 hyperhold_io_metadata[METADATA_BYTE_IN_KDF];

static struct workqueue_struct *hyperhold_proc_read_workqueue;
static struct workqueue_struct *hyperhold_proc_write_workqueue;
bool hyperhold_schedule_init_flag;

static void hyperhold_stat_io_bytes(struct hyperhold_io_req *req)
{
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat || !req->page_cnt)
		return;

	if (req->io_para.scenario == HYPERHOLD_RECLAIM_IN) {
		atomic64_add(req->page_cnt * PAGE_SIZE,
			&stat->reclaimin_bytes);
		atomic64_inc(&stat->reclaimin_cnt);
	} else {
		atomic64_add(req->page_cnt * PAGE_SIZE,
			&stat->batchout_bytes);
		atomic64_inc(&stat->batchout_cnt);
	}
}

static void hyperhold_key_init(void)
{
	get_random_bytes(hyperhold_io_key, HYPERHOLD_KEY_SIZE);
	get_random_bytes(hyperhold_io_metadata, METADATA_BYTE_IN_KDF);
}

static void hyperhold_bio_encryption_proc(struct bio *bio)
{
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	bio->ci_key_index = HYPERHOLD_KEY_INDEX;
	bio->ci_key_len = HYPERHOLD_KEY_SIZE;
	bio->ci_key = (void *)hyperhold_io_key;
	bio->index =
		bio->bi_iter.bi_sector >> HYPERHOLD_KEY_INDEX_SHIFT;
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
	bio->ci_metadata = hyperhold_io_metadata;
#endif
#endif
}

static void hyperhold_io_req_release(struct kref *ref)
{
	struct hyperhold_io_req *req =
		container_of(ref, struct hyperhold_io_req, refcount);

	if (req->io_para.complete_notify && req->io_para.private)
		req->io_para.complete_notify(req->io_para.private);

	kfree(req);
}

static void hyperhold_segment_free(struct hyperhold_io_req *req,
	struct hyperhold_segment *segment)
{
	int i;

	for (i = 0; i < segment->extent_cnt; ++i) {
		INIT_LIST_HEAD(&segment->io_entries_fifo[i]->list);
		req->io_para.done_callback(
				segment->io_entries_fifo[i], -EIO);
	}
	kfree(segment);
}

static void hyperhold_limit_inflight(struct hyperhold_io_req *req)
{
	int ret;

	if (!req->limit_inflight_flag)
		return;

	if (atomic_read(&req->extent_inflight) >= HYPERHOLD_MAX_INFILGHT_NUM)
		do {
			hh_print(HHLOG_DEBUG, "wait inflight start\n");
			ret = wait_event_timeout(req->io_wait, /*lint !e578*/
				atomic_read(&req->extent_inflight) <
				HYPERHOLD_MAX_INFILGHT_NUM,
				msecs_to_jiffies(100)); /*lint !e666*/
		} while (!ret);
}

static void hyperhold_wait_io_finish(struct hyperhold_io_req *req)
{
	int ret;
	unsigned int wait_time;

	if (!req->wait_io_finish_flag || !req->page_cnt)
		return;

	if (req->io_para.scenario == HYPERHOLD_FAULT_OUT) {
		hh_print(HHLOG_DEBUG, "fault out wait finish start\n");
		wait_for_completion_io(&req->io_end_flag);

		return;
	}

	wait_time = (req->io_para.scenario == HYPERHOLD_RECLAIM_IN) ?
		HYPERHOLD_WRITE_TIME : HYPERHOLD_READ_TIME;

	do {
		hh_print(HHLOG_DEBUG, "wait finish start\n");
		ret = wait_event_timeout(req->io_wait, /*lint !e578*/
			(!atomic_read(&req->extent_inflight)),
			msecs_to_jiffies(wait_time)); /*lint !e666*/
	} while (!ret);
}

static void hyperhold_inflight_inc(struct hyperhold_segment *segment)
{
	mutex_lock(&segment->req->refmutex);
	kref_get(&segment->req->refcount);
	mutex_unlock(&segment->req->refmutex);
	atomic_add(segment->page_cnt, &segment->req->extent_inflight);
}

static void hyperhold_inflight_dec(struct hyperhold_io_req *req,
	int num)
{
	if ((atomic_sub_return(num, &req->extent_inflight) <
		HYPERHOLD_MAX_INFILGHT_NUM) && req->limit_inflight_flag &&
		wq_has_sleeper(&req->io_wait))
		wake_up(&req->io_wait);
}

static void hyperhold_io_end_wake_up(struct hyperhold_io_req *req)
{
	if (req->io_para.scenario == HYPERHOLD_FAULT_OUT) {
		complete(&req->io_end_flag);
		return;
	}

	if (wq_has_sleeper(&req->io_wait))
		wake_up(&req->io_wait);
}

static void hyperhold_io_entry_proc(struct hyperhold_segment *segment)
{
	int i;
	struct hyperhold_io_req *req = segment->req;
	struct hyperhold_key_point_record *record = req->io_para.record;
	int page_num;
	ktime_t callback_start;
	unsigned long long callback_start_ravg_sum;

	for (i = 0; i < segment->extent_cnt; ++i) {
		INIT_LIST_HEAD(&segment->io_entries_fifo[i]->list);
		page_num = segment->io_entries_fifo[i]->pages_sz;
		hh_print(HHLOG_DEBUG, "extent_id[%d] %d page_num %d\n",
			i, segment->io_entries_fifo[i]->ext_id, page_num);
		callback_start = ktime_get();
		callback_start_ravg_sum = hyperhold_get_ravg_sum();
		if (req->io_para.done_callback)
			req->io_para.done_callback(segment->io_entries_fifo[i],
				0);
		hyperhold_perf_async_perf(record, HYPERHOLD_CALL_BACK,
			callback_start, callback_start_ravg_sum);
		hyperhold_inflight_dec(req, page_num);
	}
}

static void hyperhold_io_err_record(enum hyperhold_fail_point point,
	struct hyperhold_io_req *req, int ext_id)
{
	if (req->io_para.scenario == HYPERHOLD_FAULT_OUT)
		hyperhold_fail_record(point, 0, ext_id,
			req->io_para.record->task_comm);
}

static void hyperhold_stat_io_fail(enum hyperhold_scenario scenario)
{
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat || (scenario >= HYPERHOLD_SCENARIO_BUTT))
		return;

	atomic64_inc(&stat->io_fail_cnt[scenario]);
}

static void hyperhold_io_err_proc(struct hyperhold_io_req *req,
	struct hyperhold_segment *segment)
{
	hh_print(HHLOG_ERR, "segment sector 0x%llx, extent_cnt %d\n",
			segment->segment_sector, segment->extent_cnt);
	hh_print(HHLOG_ERR, "scenario %u, bio_result %u\n",
			req->io_para.scenario, segment->bio_result);
	hyperhold_stat_io_fail(req->io_para.scenario);
	hyperhold_io_err_record(HYPERHOLD_FAULT_OUT_IO_FAIL, req,
		segment->io_entries_fifo[0]->ext_id);
	hyperhold_inflight_dec(req, segment->page_cnt);
	hyperhold_io_end_wake_up(req);
	hyperhold_segment_free(req, segment);
	kref_put_mutex(&req->refcount, hyperhold_io_req_release,
		&req->refmutex);
}

static void hyperhold_io_end_work(struct work_struct *work)
{
	struct hyperhold_segment *segment =
		container_of(work, struct hyperhold_segment, endio_work);
	struct hyperhold_io_req *req = segment->req;
	struct hyperhold_key_point_record *record = req->io_para.record;
	int old_nice = task_nice(current);
	ktime_t work_start;
	unsigned long long work_start_ravg_sum;

	if (unlikely(segment->bio_result)) {
		hyperhold_io_err_proc(req, segment);

		return;
	}

	set_user_nice(current, req->nice);

	hyperhold_perf_async_perf(record, HYPERHOLD_SCHED_WORK,
		segment->time.end_io, 0);
	work_start = ktime_get();
	work_start_ravg_sum = hyperhold_get_ravg_sum();

	hyperhold_io_entry_proc(segment);

	hyperhold_perf_async_perf(record, HYPERHOLD_END_WORK, work_start,
		work_start_ravg_sum);

	hyperhold_io_end_wake_up(req);

	kref_put_mutex(&req->refcount, hyperhold_io_req_release,
		&req->refmutex);
	kfree(segment);

	set_user_nice(current, old_nice);
}

static void hyperhold_end_io(struct bio *bio)
{
	struct hyperhold_segment *segment = bio->bi_private;
	struct hyperhold_io_req *req = NULL;
	struct workqueue_struct *workqueue = NULL;
	struct hyperhold_key_point_record *record = NULL;

	if (unlikely(!segment || !(segment->req))) {
		hh_print(HHLOG_ERR, "segment or req null\n");
		bio_put(bio);

		return;
	}

	req = segment->req;
	record = req->io_para.record;

	hyperhold_perf_async_perf(record, HYPERHOLD_END_IO,
		segment->time.submit_bio, 0);

	workqueue = (req->io_para.scenario == HYPERHOLD_RECLAIM_IN) ?
		hyperhold_proc_write_workqueue : hyperhold_proc_read_workqueue;
	segment->time.end_io = ktime_get();
	segment->bio_result = bio->bi_status;

	queue_work(workqueue, &segment->endio_work);
	bio_put(bio);
}

static bool hyperhold_ext_merge_back(
	struct hyperhold_segment *segment,
	struct hyperhold_entry *io_entry)
{
	struct hyperhold_entry *tail_io_entry =
		list_last_entry(&segment->io_entries,
			struct hyperhold_entry, list);

	return ((tail_io_entry->addr +
		tail_io_entry->pages_sz * HYPERHOLD_PAGE_SIZE_SECTOR) ==
		io_entry->addr);
}

static bool hyperhold_ext_merge_front(
	struct hyperhold_segment *segment,
	struct hyperhold_entry *io_entry)
{
	struct hyperhold_entry *head_io_entry =
		list_first_entry(&segment->io_entries,
			struct hyperhold_entry, list);

	return (head_io_entry->addr ==
		(io_entry->addr +
		io_entry->pages_sz * HYPERHOLD_PAGE_SIZE_SECTOR));
}

static bool hyperhold_ext_merge(struct hyperhold_io_req *req,
	struct hyperhold_entry *io_entry)
{
	struct hyperhold_segment *segment = req->segment;

	if (segment == NULL)
		return false;

	if ((segment->page_cnt + io_entry->pages_sz) > BIO_MAX_PAGES)
		return false;

	if (hyperhold_ext_merge_front(segment, io_entry)) {
		list_add(&io_entry->list, &segment->io_entries);
		segment->io_entries_fifo[segment->extent_cnt++] = io_entry;
		segment->segment_sector = io_entry->addr;
		segment->page_cnt += io_entry->pages_sz;
		return true;
	}

	if (hyperhold_ext_merge_back(segment, io_entry)) {
		list_add_tail(&io_entry->list, &segment->io_entries);
		segment->io_entries_fifo[segment->extent_cnt++] = io_entry;
		segment->page_cnt += io_entry->pages_sz;

		return true;
	}

	return false;
}

static struct bio *hyperhold_bio_alloc(enum hyperhold_scenario scenario)
{
	gfp_t gfp = (scenario != HYPERHOLD_RECLAIM_IN) ? GFP_ATOMIC : GFP_NOIO;
	struct bio *bio = bio_alloc(gfp, BIO_MAX_PAGES);

	if (!bio && (scenario == HYPERHOLD_FAULT_OUT))
		bio = bio_alloc(GFP_NOIO, BIO_MAX_PAGES);

	return bio;
}

static int hyperhold_bio_add_page(struct bio *bio,
	struct hyperhold_segment *segment)
{
	int i;
	int k = 0;
	struct hyperhold_entry *io_entry = NULL;
	struct hyperhold_entry *tmp = NULL;

	list_for_each_entry_safe(io_entry, tmp, &segment->io_entries, list)  {
		for (i = 0; i < io_entry->pages_sz; i++) {
			io_entry->dest_pages[i]->index =
				bio->bi_iter.bi_sector + k;
			if (unlikely(!bio_add_page(bio,
				io_entry->dest_pages[i], PAGE_SIZE, 0))) {

				return -EIO;
			}
			k += HYPERHOLD_PAGE_SIZE_SECTOR;
		}
	}

	return 0;
}

static void hyperhold_set_bio_opf(struct bio *bio,
	struct hyperhold_segment *segment)
{
	if (segment->req->io_para.scenario == HYPERHOLD_RECLAIM_IN) {
		bio->bi_opf |= REQ_BACKGROUND;
		return;
	}

	bio->bi_opf |= REQ_SYNC | REQ_FG | REQ_VIP;
	if (segment->req->io_para.scenario == HYPERHOLD_FAULT_OUT)
		bio->bi_opf |= REQ_FAULT_OUT;
	else
		bio->bi_opf |= REQ_BATCH_OUT;
}

static int hyperhold_submit_bio(struct hyperhold_segment *segment)
{
	unsigned int op =
		(segment->req->io_para.scenario == HYPERHOLD_RECLAIM_IN) ?
		REQ_OP_WRITE : REQ_OP_READ;
	struct hyperhold_entry *head_io_entry =
		list_first_entry(&segment->io_entries,
			struct hyperhold_entry, list);
	struct hyperhold_key_point_record *record =
		segment->req->io_para.record;
	struct bio *bio = NULL;

	hyperhold_perf_lat_start(record, HYPERHOLD_BIO_ALLOC);
	bio = hyperhold_bio_alloc(segment->req->io_para.scenario);
	hyperhold_perf_lat_end(record, HYPERHOLD_BIO_ALLOC);
	if (unlikely(!bio)) {
		hh_print(HHLOG_ERR, "bio is null.\n");
		hyperhold_io_err_record(HYPERHOLD_FAULT_OUT_BIO_ALLOC_FAIL,
			segment->req, segment->io_entries_fifo[0]->ext_id);

		return -ENOMEM;
	}

	bio->bi_iter.bi_sector = segment->segment_sector;
	bio_set_dev(bio, segment->req->io_para.bdev);
	bio->bi_private = segment;
	bio_set_op_attrs(bio, op, 0);
	bio->bi_end_io = hyperhold_end_io;
	hyperhold_set_bio_opf(bio, segment);
	hyperhold_bio_encryption_proc(bio);

	if (unlikely(hyperhold_bio_add_page(bio, segment))) {
		bio_put(bio);
		hh_print(HHLOG_ERR, "bio_add_page fail\n");
		hyperhold_io_err_record(HYPERHOLD_FAULT_OUT_BIO_ADD_FAIL,
			segment->req, segment->io_entries_fifo[0]->ext_id);

		return -EIO;
	}

	hyperhold_inflight_inc(segment);

	hh_print(HHLOG_DEBUG, "submit bio sector %llu ext_id %d\n",
		segment->segment_sector, head_io_entry->ext_id);
	hh_print(HHLOG_DEBUG, "extent_cnt %d scenario %u\n",
		segment->extent_cnt, segment->req->io_para.scenario);

	segment->req->page_cnt += segment->page_cnt;
	segment->req->segment_cnt++;
	segment->time.submit_bio = ktime_get();

	hyperhold_perf_lat_start(record, HYPERHOLD_SUBMIT_BIO);
	submit_bio(bio);
	hyperhold_perf_lat_end(record, HYPERHOLD_SUBMIT_BIO);

	return 0;
}

static int hyperhold_new_segment_init(struct hyperhold_io_req *req,
	struct hyperhold_entry *io_entry)
{
	gfp_t gfp = (req->io_para.scenario != HYPERHOLD_RECLAIM_IN) ?
		GFP_ATOMIC : GFP_NOIO;
	struct hyperhold_segment *segment = NULL;
	struct hyperhold_key_point_record *record = req->io_para.record;

	hyperhold_perf_lat_start(record, HYPERHOLD_SEGMENT_ALLOC);
	segment = kzalloc(sizeof(struct hyperhold_segment), gfp);
	if (!segment && (req->io_para.scenario == HYPERHOLD_FAULT_OUT))
		segment = kzalloc(sizeof(struct hyperhold_segment), GFP_NOIO);
	hyperhold_perf_lat_end(record, HYPERHOLD_SEGMENT_ALLOC);
	if (unlikely(!segment)) {
		hyperhold_io_err_record(HYPERHOLD_FAULT_OUT_SEGMENT_ALLOC_FAIL,
			req, io_entry->ext_id);

		return -ENOMEM;
	}

	segment->req = req;
	INIT_LIST_HEAD(&segment->io_entries);
	list_add_tail(&io_entry->list, &segment->io_entries);
	segment->io_entries_fifo[segment->extent_cnt++] = io_entry;
	segment->page_cnt = io_entry->pages_sz;
	INIT_WORK(&segment->endio_work, hyperhold_io_end_work);
	segment->segment_sector = io_entry->addr;
	req->segment = segment;

	return 0;
}

static int hyperhold_io_submit(struct hyperhold_io_req *req,
	bool merge_flag)
{
	int ret;
	struct hyperhold_segment *segment = req->segment;

	if (!segment || ((merge_flag) && (segment->page_cnt < BIO_MAX_PAGES)))
		return 0;

	hyperhold_limit_inflight(req);

	ret = hyperhold_submit_bio(segment);
	if (unlikely(ret))
		hyperhold_segment_free(req, segment);

	req->segment = NULL;

	return ret;
}

static bool hyperhold_check_io_para_err(struct hyperhold_io *io_para)
{
	if (unlikely(!io_para)) {
		hh_print(HHLOG_ERR, "io_para null\n");

		return true;
	}

	if (unlikely(!io_para->bdev ||
		(io_para->scenario >= HYPERHOLD_SCENARIO_BUTT))) {
		hh_print(HHLOG_ERR, "io_para err, scenario %u\n",
			io_para->scenario);

		return true;
	}

	if (unlikely(!io_para->done_callback)) {
		hh_print(HHLOG_ERR, "done_callback err\n");

		return true;
	}

	return false;
}

static bool hyperhold_check_entry_err(
	struct hyperhold_entry *io_entry)
{
	int i;

	if (unlikely(!io_entry)) {
		hh_print(HHLOG_ERR, "io_entry null\n");

		return true;
	}

	if (unlikely((!io_entry->dest_pages) ||
		(io_entry->ext_id < 0) ||
		(io_entry->pages_sz > BIO_MAX_PAGES) ||
		(io_entry->pages_sz <= 0))) {
		hh_print(HHLOG_ERR, "ext_id %d, page_sz %d\n", io_entry->ext_id,
			io_entry->pages_sz);

		return true;
	}

	for (i = 0; i < io_entry->pages_sz; ++i) {
		if (!io_entry->dest_pages[i]) {
			hh_print(HHLOG_ERR, "dest_pages[%d] is null\n", i);
			return true;
		}
	}

	return false;
}

static int hyperhold_io_extent(void *io_handler,
	struct hyperhold_entry *io_entry)
{
	int ret;
	struct hyperhold_io_req *req = (struct hyperhold_io_req *)io_handler;

	if (unlikely(hyperhold_check_entry_err(io_entry))) {
		hyperhold_io_err_record(HYPERHOLD_FAULT_OUT_IO_ENTRY_PARA_FAIL,
			req, io_entry ? io_entry->ext_id : -EINVAL);
		req->io_para.done_callback(io_entry, -EIO);

		return -EFAULT;
	}

	hh_print(HHLOG_DEBUG, "ext id %d, pages_sz %d, addr %llx\n",
		io_entry->ext_id, io_entry->pages_sz,
		io_entry->addr);

	if (hyperhold_ext_merge(req, io_entry))
		return hyperhold_io_submit(req, true);

	ret = hyperhold_io_submit(req, false);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "submit fail %d\n", ret);
		req->io_para.done_callback(io_entry, -EIO);

		return ret;
	}

	ret = hyperhold_new_segment_init(req, io_entry);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "hyperhold_new_segment_init fail %d\n",
			ret);
		req->io_para.done_callback(io_entry, -EIO);

		return ret;
	}

	return 0;
}

int hyperhold_schedule_init(void)
{
	if (hyperhold_schedule_init_flag)
		return 0;

	hyperhold_proc_read_workqueue = alloc_workqueue("proc_hyperhold_read",
		WQ_HIGHPRI | WQ_UNBOUND, 0);
	if (unlikely(!hyperhold_proc_read_workqueue))
		return -EFAULT;

	hyperhold_proc_write_workqueue = alloc_workqueue("proc_hyperhold_write",
		WQ_CPU_INTENSIVE, 0);
	if (unlikely(!hyperhold_proc_write_workqueue)) {
		destroy_workqueue(hyperhold_proc_read_workqueue);

		return -EFAULT;
	}

	hyperhold_key_init();

	hyperhold_schedule_init_flag = true;

	return 0;
}

void *hyperhold_plug_start(struct hyperhold_io *io_para)
{
	gfp_t gfp;
	struct hyperhold_io_req *req = NULL;

	if (unlikely(hyperhold_check_io_para_err(io_para)))
		return NULL;

	gfp = (io_para->scenario != HYPERHOLD_RECLAIM_IN) ?
		GFP_ATOMIC : GFP_NOIO;
	req = kzalloc(sizeof(struct hyperhold_io_req), gfp);
	if (!req && (io_para->scenario == HYPERHOLD_FAULT_OUT))
		req = kzalloc(sizeof(struct hyperhold_io_req), GFP_NOIO);

	if (unlikely(!req)) {
		hh_print(HHLOG_ERR, "io_req null\n");

		return NULL;
	}

	kref_init(&req->refcount);
	mutex_init(&req->refmutex);
	atomic_set(&req->extent_inflight, 0);
	init_waitqueue_head(&req->io_wait);
	req->io_para.bdev = io_para->bdev;
	req->io_para.scenario = io_para->scenario;
	req->io_para.done_callback = io_para->done_callback;
	req->io_para.complete_notify = io_para->complete_notify;
	req->io_para.private = io_para->private;
	req->io_para.record = io_para->record;
	req->limit_inflight_flag =
		(io_para->scenario == HYPERHOLD_RECLAIM_IN) ||
		(io_para->scenario == HYPERHOLD_PRE_OUT);
	req->wait_io_finish_flag =
		(io_para->scenario == HYPERHOLD_RECLAIM_IN) ||
		(io_para->scenario == HYPERHOLD_FAULT_OUT);
	req->nice = task_nice(current);
	init_completion(&req->io_end_flag);

	return (void *)req;
}

/* io_handler validity guaranteed by the caller */
int hyperhold_read_extent(void *io_handler,
	struct hyperhold_entry *io_entry)
{
	return hyperhold_io_extent(io_handler, io_entry);
}

/* io_handler validity guaranteed by the caller */
int hyperhold_write_extent(void *io_handler,
	struct hyperhold_entry *io_entry)
{
	return hyperhold_io_extent(io_handler, io_entry);
}

/* io_handler validity guaranteed by the caller */
int hyperhold_plug_finish(void *io_handler)
{
	int ret;
	struct hyperhold_io_req *req = (struct hyperhold_io_req *)io_handler;

	hyperhold_perf_lat_start(req->io_para.record, HYPERHOLD_IO_EXTENT);
	ret = hyperhold_io_submit(req, false);
	if (unlikely(ret))
		hh_print(HHLOG_ERR, "submit fail %d\n", ret);

	hyperhold_perf_lat_end(req->io_para.record, HYPERHOLD_IO_EXTENT);
	hyperhold_wait_io_finish(req);
	hyperhold_perf_lat_point(req->io_para.record, HYPERHOLD_WAKE_UP);

	hyperhold_stat_io_bytes(req);
	hyperhold_perf_io_stat(req->io_para.record, req->page_cnt,
		req->segment_cnt);

	kref_put_mutex(&req->refcount, hyperhold_io_req_release,
		&req->refmutex);

	hh_print(HHLOG_DEBUG, "io schedule finish succ\n");

	return ret;
}

