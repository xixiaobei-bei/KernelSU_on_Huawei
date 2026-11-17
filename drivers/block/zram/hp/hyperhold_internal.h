/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold header file
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

#ifndef HYPERHOLD_INTERNAL_H
#define HYPERHOLD_INTERNAL_H

#define EXTENT_SHIFT		15
#define EXTENT_SIZE		(1UL << EXTENT_SHIFT)
#define EXTENT_PG_CNT		(EXTENT_SIZE >> PAGE_SHIFT)
#define EXTENT_SECTOR_SIZE	(EXTENT_PG_CNT << 3)

#define MAX_FAIL_RECORD_NUM 	10

#define BLOCK_SECTOR_SHIFT	3

#define MIN_RECLAIM_ZRAM_SZ	(1024 * 1024)
#define FT_RECLAIM_SZ		500

enum {
	HHLOG_ERR = 0,
	HHLOG_WARN,
	HHLOG_INFO,
	HHLOG_DEBUG,
	HHLOG_MAX
};

static inline void pr_none(void) {}

#define pt(f, ...)	pr_err("<%s>:"f, __func__, ##__VA_ARGS__)
#define cur_lvl()	hyperhold_loglevel()

#define hh_print(l, f, ...) \
	(l <= cur_lvl() ? pt(f, ##__VA_ARGS__) : pr_none())

int hyperhold_loglevel(void);

enum hyperhold_scenario {
	HYPERHOLD_RECLAIM_IN = 0,
	HYPERHOLD_FAULT_OUT,
	HYPERHOLD_BATCH_OUT,
	HYPERHOLD_PRE_OUT,
	HYPERHOLD_SCENARIO_BUTT
};

enum hyperhold_key_point {
	HYPERHOLD_START = 0,
	HYPERHOLD_INIT,
	HYPERHOLD_IOENTRY_ALLOC,
	HYPERHOLD_FIND_EXTENT,
	HYPERHOLD_IO_EXTENT,
	HYPERHOLD_SEGMENT_ALLOC,
	HYPERHOLD_BIO_ALLOC,
	HYPERHOLD_SUBMIT_BIO,
	HYPERHOLD_END_IO,
	HYPERHOLD_SCHED_WORK,
	HYPERHOLD_END_WORK,
	HYPERHOLD_CALL_BACK,
	HYPERHOLD_WAKE_UP,
	HYPERHOLD_ZRAM_LOCK,
	HYPERHOLD_DONE,
	HYPERHOLD_KYE_POINT_BUTT
};

enum hyperhold_fail_point {
	HYPERHOLD_FAULT_OUT_INIT_FAIL = 0,
	HYPERHOLD_FAULT_OUT_ENTRY_ALLOC_FAIL,
	HYPERHOLD_FAULT_OUT_IO_ENTRY_PARA_FAIL,
	HYPERHOLD_FAULT_OUT_SEGMENT_ALLOC_FAIL,
	HYPERHOLD_FAULT_OUT_BIO_ALLOC_FAIL,
	HYPERHOLD_FAULT_OUT_BIO_ADD_FAIL,
	HYPERHOLD_FAULT_OUT_IO_FAIL,
	HYPERHOLD_FAIL_POINT_BUTT
};

struct hyperhold_fail_record {
	unsigned char task_comm[TASK_COMM_LEN];
	enum hyperhold_fail_point point;
	ktime_t time;
	u32 index;
	int ext_id;
};

struct hyperhold_fail_record_info {
	int num;
	spinlock_t lock;
	struct hyperhold_fail_record record[MAX_FAIL_RECORD_NUM];
};

struct hyperhold_key_point_info {
	unsigned int record_cnt;
	unsigned int end_cnt;
	ktime_t first_time;
	ktime_t last_time;
	s64 proc_total_time;
	s64 proc_max_time;
	unsigned long long last_ravg_sum;
	unsigned long long proc_ravg_sum;
	spinlock_t time_lock;
};

struct hyperhold_key_point_record {
	struct timer_list lat_monitor;
	unsigned long warning_threshold;
	int page_cnt;
	int segment_cnt;
	int nice;
	bool timeout_flag;
	unsigned char task_comm[TASK_COMM_LEN];
	struct task_struct *task;
	enum hyperhold_scenario scenario;
	struct hyperhold_key_point_info key_point[HYPERHOLD_KYE_POINT_BUTT];
};

struct hyperhold_lat_stat {
	atomic64_t total_lat;
	atomic64_t max_lat;
	atomic64_t timeout_cnt;
};

struct hyperhold_stat {
	atomic64_t reclaimin_cnt;
	atomic64_t reclaimin_bytes;
	atomic64_t reclaimin_pages;
	atomic64_t reclaimin_infight;
	atomic64_t batchout_cnt;
	atomic64_t batchout_bytes;
	atomic64_t batchout_pages;
	atomic64_t batchout_inflight;
	atomic64_t fault_cnt;
	atomic64_t hyperhold_fault_cnt;
	atomic64_t reout_pages;
	atomic64_t reout_bytes;
	atomic64_t zram_stored_pages;
	atomic64_t zram_stored_size;
	atomic64_t stored_pages;
	atomic64_t stored_size;
	atomic64_t notify_free;
	atomic64_t frag_cnt;
	atomic64_t mcg_cnt;
	atomic64_t ext_cnt;
	atomic64_t miss_free;
	atomic64_t mcgid_clear;
	atomic64_t io_fail_cnt[HYPERHOLD_SCENARIO_BUTT];
	atomic64_t alloc_fail_cnt[HYPERHOLD_SCENARIO_BUTT];
	struct hyperhold_lat_stat lat[HYPERHOLD_SCENARIO_BUTT];
	struct hyperhold_fail_record_info record;
};

struct hyperhold_page_pool {
	struct list_head page_pool_list;
	spinlock_t page_pool_lock;
};

struct hyperhold_buffer {
	struct zram *zram;
	struct hyperhold_page_pool *pool;
	struct page **dest_pages;
};

struct hyperhold_entry {
	int ext_id;
	sector_t addr;
	struct page **dest_pages;
	int pages_sz;
	struct list_head list;
	void *private;
	void *manager_private;
};

struct hyperhold_io {
	struct block_device *bdev;
	enum hyperhold_scenario scenario;
	void (*done_callback)(struct hyperhold_entry *, int);
	void (*complete_notify)(void *);
	void *private;
	struct hyperhold_key_point_record *record;
};

void *hyperhold_malloc(size_t size, bool fast, bool nofail);

void hyperhold_free(const void *mem);

unsigned long hyperhold_zsmalloc(struct zs_pool *zs_pool,
			size_t size, struct hyperhold_page_pool *pool);

struct page *hyperhold_alloc_page(
		struct hyperhold_page_pool *pool, gfp_t gfp,
		bool fast, bool nofail);

void hyperhold_page_recycle(struct page *page,
		struct hyperhold_page_pool *pool);

struct hyperhold_stat *hyperhold_get_stat_obj(void);

int hyperhold_manager_init(struct zram *zram);

void hyperhold_manager_memcg_init(struct mem_cgroup *memcg, struct zram *zram);

void hyperhold_manager_memcg_deinit(struct mem_cgroup *mcg);

void hyperhold_zram_lru_add(struct zram *zram, u32 index,
					struct mem_cgroup *memcg);

void hyperhold_zram_lru_del(struct zram *zram, u32 index);

unsigned long hyperhold_extent_create(struct mem_cgroup *memcg,
					int *ext_id,
					struct hyperhold_buffer *dest_buf,
					void **private);

void hyperhold_extent_register(void *private);

void hyperhold_extent_objs_del(struct zram *zram, u32 index);

int hyperhold_find_extent_by_idx(
	unsigned long eswpentry, struct hyperhold_buffer *buf, void **private);

int hyperhold_find_extent_by_memcg(
			struct mem_cgroup *mcg,
			struct hyperhold_buffer *dest_buf, void **private);

void hyperhold_extent_destroy(void *private, enum hyperhold_scenario scenario);

void hyperhold_extent_exception(enum hyperhold_scenario scenario,
					void *private);

void hyperhold_manager_deinit(struct zram *zram);

struct mem_cgroup *hyperhold_zram_get_memcg(struct zram *zram, u32 index);

int hyperhold_schedule_init(void);

void *hyperhold_plug_start(struct hyperhold_io *io_para);

int hyperhold_read_extent(void *io_handler,
				struct hyperhold_entry *io_entry);

int hyperhold_write_extent(void *io_handler,
				struct hyperhold_entry *io_entry);

int hyperhold_plug_finish(void *io_handler);

void hyperhold_perf_start(
	struct hyperhold_key_point_record *record,
	ktime_t stsrt, unsigned long long start_ravg_sum,
	enum hyperhold_scenario scenario);

void hyperhold_perf_end(struct hyperhold_key_point_record *record);

void hyperhold_perf_lat_start(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type);

void hyperhold_perf_lat_end(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type);

void hyperhold_perf_lat_point(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type);

void hyperhold_perf_async_perf(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type, ktime_t start,
	unsigned long long start_ravg_sum);

void hyperhold_perf_io_stat(
	struct hyperhold_key_point_record *record, int page_cnt,
	int segment_cnt);

static inline unsigned long long hyperhold_get_ravg_sum(void)
{
#ifdef CONFIG_TASK_RAVG_SUM
	return current->ravg.ravg_sum;
#else
	return 0;
#endif
}

void hyperhold_fail_record(enum hyperhold_fail_point point,
	u32 index, int ext_id, unsigned char *task_comm);

bool hyperhold_reclaim_in_enable(void);
struct workqueue_struct *hyperhold_get_reclaim_workqueue(void);

#endif

void memcg_idle_count(struct zram *zram);
void memcg_idle_inc(struct zram *zram, u32 index);
void memcg_idle_dec(struct zram *zram, u32 index);

sector_t hyperhold_get_start_sector(void);
