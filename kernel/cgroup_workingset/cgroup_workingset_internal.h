/*
 * cgroup_workingset_internal.h
 *
 * control group workingset subsystem
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef CGROUP_WORKINGSET_INTERNAL_H
#define CGROUP_WORKINGSET_INTERNAL_H

#include <linux/cgroup.h>
#include <linux/fs.h>
#include <linux/mm_types.h>
#include <linux/mutex.h>
#include <linux/rbtree.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/wait.h>

#define CGROUP_WORKINGSET_VERSION	(12)

#define FILE_PAGESEQ_BITS				16
#define PAGE_STAGE_NUM_BITS			3
#define PAGE_MAJOR_BITS				1
#define PAGE_RANGE_HEAD_BITS			1
#define FILE_SEQNUM_BITS				10
#define FILE_OFFSET_BITS	\
	(8 * sizeof(uint32_t) - FILE_SEQNUM_BITS \
	- PAGE_RANGE_HEAD_BITS - PAGE_MAJOR_BITS \
	- PAGE_STAGE_NUM_BITS)
#define PAGE_STAGE_NUM_SHIFT	\
	(FILE_SEQNUM_BITS + FILE_OFFSET_BITS	\
	+ PAGE_RANGE_HEAD_BITS + PAGE_MAJOR_BITS)
#define PAGE_STAGE_NUM_MASK	\
	((1U << PAGE_STAGE_NUM_BITS) - 1)
#define PAGE_MAJOR_SHIFT	\
	(FILE_SEQNUM_BITS + FILE_OFFSET_BITS + PAGE_RANGE_HEAD_BITS)
#define PAGE_MAJOR_MASK	\
	((1U << PAGE_MAJOR_BITS) - 1)
#define PAGE_RANGE_HEAD_SHIFT		\
	(FILE_SEQNUM_BITS + FILE_OFFSET_BITS)
#define PAGE_RANGE_HEAD_MASK	\
	((1U << PAGE_RANGE_HEAD_BITS) - 1)
#define PAGE_RANGE_HEAD_BIT_MASK	\
	(PAGE_RANGE_HEAD_MASK << PAGE_RANGE_HEAD_SHIFT)
#define FILE_IDX_AND_OFFSET_MASK	\
	((1U << PAGE_RANGE_HEAD_SHIFT) - 1)
#define MAX_TOUCHED_FILES_COUNT	\
	((1U << FILE_SEQNUM_BITS) - 1)
#define MAX_TOUCHED_FILE_OFFSET	\
	((1U << FILE_OFFSET_BITS) - 1)
#define MAX_TOUCHED_PAGES_COUNT	\
	((1ULL << FILE_PAGESEQ_BITS) - 1)
#define CONTIGUOUS_PAGE_COUNT_BITS	\
	(8 * sizeof(uint32_t) - PAGE_STAGE_NUM_BITS - PAGE_MAJOR_BITS)

#define FILPS_PER_PAGE	\
	(PAGE_SIZE / sizeof(struct page *))
#define FILP_PAGES_COUNT	\
	((MAX_TOUCHED_FILES_COUNT + FILPS_PER_PAGE - 1) / FILPS_PER_PAGE)

#define COLLECTOR_CACHE_SIZE_ORDER			(4)
#define COLLECTOR_CACHE_SIZE	\
	(PAGE_SIZE << COLLECTOR_CACHE_SIZE_ORDER)
#define COLLECTOR_BATCH_COUNT		(64)
#define COLLECTOR_REMAIN_CACHE_LOW_WATER	\
	(COLLECTOR_BATCH_COUNT << 4)

#define PAGECACHEINFO_PER_PAGE	\
	(PAGE_SIZE / sizeof(struct s_pagecache_info))
#define PAGECACHE_INVALID_OFFSET			(~0U)

#define WORKINGSET_RECORD_MAGIC			(0x2b3c5d8e)
#define PATH_MAX_CHAR					256
#define OWNER_MAX_CHAR					256
#define MAX_CRCOMP_STRM_COUNT			2

#define CARE_BLKIO_MIN_THRESHOLD				20
#define BLKIO_PERCENTAGE_THRESHOLD_FOR_UPDATE	2
#define BLKIO_MULTIPLE_FOR_UPDATE				2
#define CACHE_MISSED_THRESHOLD_FOR_BLKIO		20
#define AF_CLEARED_BLKIO_SCALE					2

#define MULTI_PAGES_SEQ_WORDS	2
#define SINGLE_PAGE_SEQ_WORDS	1
#define ONE_HUNDRED				100
#define S_IRWUSR				0600

#define TOTAL_RAM_PAGES_1G	(1 * (1 << 18))
#define TOTAL_RAM_PAGES_2G	(2 * (1 << 18))
#define TOTAL_RAM_PAGES_3G	(3 * (1 << 18))
#define MAX_RECORD_COUNT_ON_1G	5
#define MAX_RECORD_COUNT_ON_2G	10
#define MAX_RECORD_COUNT_ON_3G	20
#define MAX_RECORD_COUNT_ON_4G	40

#define STATE_ONLINE_SHIFT	7
#define STATE_MONITORING_SHIFT	6
/* The commands what sends to workingset */
enum ws_monitor_states {
	E_MONITOR_STATE_OUTOFWORK,
	E_MONITOR_STATE_INWORKING,
	E_MONITOR_STATE_PAUSED,
	E_MONITOR_STATE_STOP,
	E_MONITOR_STATE_ABORT,
	E_MONITOR_STATE_PREREAD,
	E_MONITOR_STATE_BACKUP,
	E_MONITOR_STATE_CLEARYOUNG,
	E_MONITOR_STATE_MAX
};


/* The states of workingset */
enum ws_cgroup_states {
	E_CGROUP_STATE_OFFLINE = 0,
	E_CGROUP_STATE_ONLINE = (1 << STATE_ONLINE_SHIFT),
	E_CGROUP_STATE_MONITOR_BITMASK	= (E_CGROUP_STATE_ONLINE - 1),
	E_CGROUP_STATE_MONITORING = (1 << STATE_MONITORING_SHIFT),
	E_CGROUP_STATE_MONITOR_OUTOFWORK =
		(E_CGROUP_STATE_ONLINE | E_MONITOR_STATE_OUTOFWORK),
	E_CGROUP_STATE_MONITOR_INWORKING =
		(E_CGROUP_STATE_ONLINE |
		E_CGROUP_STATE_MONITORING | E_MONITOR_STATE_INWORKING),
	E_CGROUP_STATE_MONITOR_PAUSED =
		(E_CGROUP_STATE_ONLINE | E_MONITOR_STATE_PAUSED),
	E_CGROUP_STATE_MONITOR_STOP =
		(E_CGROUP_STATE_ONLINE | E_MONITOR_STATE_STOP),
	E_CGROUP_STATE_MONITOR_ABORT =
		(E_CGROUP_STATE_ONLINE | E_MONITOR_STATE_ABORT),
	E_CGROUP_STATE_MONITOR_PREREAD =
		(E_CGROUP_STATE_ONLINE | E_MONITOR_STATE_PREREAD),
	E_CGROUP_STATE_MONITOR_BACKUP =
		(E_CGROUP_STATE_ONLINE | E_MONITOR_STATE_BACKUP),
	E_CGROUP_STATE_MONITOR_CLEARYOUNG =
		(E_CGROUP_STATE_ONLINE | E_MONITOR_STATE_CLEARYOUNG),
	E_CGROUP_STATE_MAX,
};

/* The events what collector waiting for */
enum collector_wait_flags {
	F_NONE,
	F_COLLECT_PENDING,
	F_RECORD_PENDING,
};

/* The states or flags of records */
enum ws_record_state {
	E_RECORD_STATE_UNUSED = 0x00,
	E_RECORD_STATE_USED = 0x01,
	/* The flag indicate writeback the record */
	E_RECORD_STATE_DIRTY = 0x02,
	E_RECORD_STATE_COLLECTING = 0x04,
	E_RECORD_STATE_PREREADING = 0x08,
	E_RECORD_STATE_PAUSE = 0x10,
	/* The flag indicate paths of files are cached in external buffers */
	E_RECORD_STATE_EXTERNAL_FILEPATH = 0x20,
	/* The flag indicate the blkio count that first time on prereading */
	E_RECORD_STATE_UPDATE_BASE_BLKIO = 0x40,
	/* The flag indicate write only record header to disk */
	E_RECORD_STATE_UPDATE_HEADER_ONLY = 0x80,
};

enum ws_self_read_state {
	E_SELFREAD_NONE,
	E_SELFREAD_INIT,
	E_SELFREAD_WAIT,
	E_SELFREAD_DOING,
};

struct s_path_node {
	/* The hash code of this path */
	unsigned int hashcode;
	unsigned int pathlen;
	uid_t owner_uid;
	char *path;
};

struct s_range {
	unsigned int start;
	unsigned int end;
};

struct s_filp_list {
	struct file *filp;
	struct s_filp_list *next;
};

struct s_file_info {
	/* List to the workingset file list */
	struct list_head list;
	/* The list of pointer of struct file */
	struct s_filp_list *filp_list;
	/* The path info of file */
	struct s_path_node path_node;
	/*
	 * The count of page sequences belong to this owner,
	 * the range including single page occupy one pageseq,
	 * the range including multi pages occupy two pageseqs.
	 */
	unsigned int pageseq_count;
	/* The root of page cache tree */
	struct rb_root rbroot;
};

struct s_pagecache_info {
	struct rb_node rbnode;
	/* The offset range of file */
	struct s_range offset_range;
};

struct s_ws_owner {
	unsigned int uid;
	/* The pid of leader thread */
	int pid;
	char *name;
	/* The path of record file */
	char *record_path;
};

struct s_ws_data {
	unsigned int file_cnt;
	/*
	 * The count of page sequences belong to this owner,
	 * the range including single page occupy one pageseq,
	 * the range including multi pages occupy two pageseqs.
	 */
	unsigned int pageseq_cnt;
	/* Sum of file pages this owner accessed */
	unsigned int page_sum;
	/* The size of pages array */
	unsigned int array_page_cnt;
	/*
	 * The pages array that caching the path informations
	 * and file offset range informations
	 */
	struct page **page_array;
	/* The file array */
	struct s_path_node *file_array;
	/* The file cache array */
	unsigned int *cacheseq;
};

struct s_ws_record {
	/* List to the global workingset record list */
	struct list_head list;
	struct s_ws_owner owner;
	struct s_ws_data data;
	struct mutex mutex;
	/* The state of a record */
	unsigned int state;
	uint8_t is_af_cleared;
	/* The blkio count of main thread when first time on prereading */
	unsigned short leader_blkio_cnt;
	/* Tell us if or not need collect again */
	unsigned short need_update;
	/* Pages for caching struct files that be opened on prereading */
	struct page *filp_pages[FILP_PAGES_COUNT];
};

struct s_ws_backup_record_header {
	unsigned int magic;
	unsigned int header_crc;
	/*
	 * Version of the record file,
	 * it must be equal version of this linux module.
	 */
	unsigned int record_version;
	/* The count of the file */
	unsigned int file_cnt;
	/*
	 * The count of page sequences belong to this owner,
	 * the range including single page occupy one pageseq,
	 * the range including multi pages occupy two pageseqs.
	 */
	unsigned int pageseq_cnt;
	/* Sum of accessed file pages */
	unsigned int page_sum;
	/* The size of the playload data */
	unsigned int playload_length;
	/* The checksum of the playload data */
	unsigned int playload_checksum;
	/* The blkio count of main thread when first time on prereading */
	unsigned short leader_blkio_cnt;
	/* Tell us if or not need collect again */
	unsigned short need_update;
	uint8_t is_af_cleared;
};

struct s_workingset {
	struct cgroup_subsys_state css;
	struct mutex mutex;
	/* The owner which workingset is working for */
	struct s_ws_owner owner;
	unsigned long repeated_count;
	unsigned int page_sum;
	unsigned int stage_num;
	/* The state of workingset */
	unsigned int state;
	/* The blkio count of main thread */
	unsigned short leader_blkio_cnt;
	__u64 leader_blkio_base;
	unsigned int file_count;
	unsigned int pageseq_count;
	/* The alloc index of page_cache array */
	unsigned int alloc_index;
	bool clear_young;
	bool shrinker_enabled;
	struct shrinker shrinker;
	struct list_head file_list;
	/* Pages for caching page offset range information */
	struct s_pagecache_info *cache_pages[0];
};

struct s_cachepage_info {
	struct file *filp;
	unsigned int offset;
	/* The count of contiguous page_cache */
	unsigned int count:CONTIGUOUS_PAGE_COUNT_BITS;
	unsigned int stage:PAGE_STAGE_NUM_BITS;
	unsigned int is_major:PAGE_MAJOR_BITS;
};

struct s_ws_collector {
	spinlock_t lock;
	struct task_struct *collector_thread;
	wait_queue_head_t collect_wait;
	enum collector_wait_flags wait_flag;
	/* The workingset that collector working for */
	struct s_workingset *monitor;
	unsigned long discard_count;
	/* The read position of circle buffer */
	unsigned int read_pos;
	/* The write position of circle buffer */
	unsigned int write_pos;
	/* The address of circle buffer */
	void *circle_buffer;
};

struct s_probe_context {
	struct rb_root *root;
	struct rb_node **link;
	struct rb_node *parent;
	struct s_pagecache_info *merged_entry;
	struct s_pagecache_info *cur_entry;
	bool probe_left;
	bool probe_right;
	bool look_side;
	unsigned int start;
	unsigned int end;
	unsigned int stage;
	unsigned int repeat;
	int major;
	int delta;
};

struct s_readpages_control {
	struct file *filp;
	struct address_space *mapping;
	/* The file offset that will be read */
	pgoff_t offset;
	/* The count that file pages was readed */
	unsigned long nr_to_read;
	/* The count that lru pages was moved */
	unsigned int nr_adjusted;
};

struct s_clear_param {
	struct vm_area_struct *vma;
	unsigned long nr_cleared;
};

struct s_read_control {
	struct s_ws_data *data;
	struct file **filpp;
	unsigned int stage_end;
	unsigned int present_pages_cnt;
	unsigned int read_pages_cnt;
	unsigned int move_lru_cnt;
};

struct s_find_file_by_path {
	struct file *file;
	const char *filepath;
	unsigned int hashcode;
};

extern bool ws_debug_enable __read_mostly;
#define ws_dbg(x...) do { if (ws_debug_enable) pr_info(x); } while (0)

const char *workingset_state_strs(unsigned int state);
void workingset_collector_reset(const struct s_workingset *ws);

/* Api of the collect part */
int workingset_collect_kworkthread(void *p);
int workingset_clear_record(char *owner_string);
struct s_ws_record *workingset_get_available_record(void);
void workingset_destroy_data(struct s_workingset *ws, bool is_locked);

/* Api of the backup part */
int workingset_crc32_init(void);
void workingset_crc32_deinit(void);
unsigned int workingset_crc32c(
	unsigned int crc, const void *address, unsigned int length);
void workingset_record_list_init(void);
void workingset_insert_record_to_list_head(struct s_ws_record *record);
void workingset_insert_record_to_list_tail(struct s_ws_record *record);
struct s_ws_record *workingset_get_existed_record_wslocked(
	const struct s_ws_owner *owner, bool onlycache);
void workingset_writeback_all_records(void);
void workingset_writeback_last_record_if_need(void);
int workingset_prepare_record_space_wsrcrdlocked(
	const struct s_ws_owner *owner, struct s_ws_record *record,
	bool is_exist, unsigned int playload_size, unsigned int **pplayload);

/* Api of the preread part */
void workingset_do_preread_work_rcrdlocked(struct s_ws_record *record);
void workingset_preread_by_self_internal(struct s_ws_record *record);
void workingset_preread_qos_wsrcrdlocked(
	struct s_workingset *ws, struct s_ws_record *record);
void workingset_preread_permmit(void);
void workingset_preread_force_stop(void);
enum ws_self_read_state workingset_get_preread_state(void);
void workingset_set_preread_state(enum ws_self_read_state state);
#endif /* CGROUP_WORKINGSET_INTERNAL_H */
