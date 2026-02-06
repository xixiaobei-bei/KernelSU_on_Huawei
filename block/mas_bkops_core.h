/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: bkops core framework
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef MAS_BKOPS_CORE_H
#define MAS_BKOPS_CORE_H

#include <linux/workqueue.h>
#include <linux/blkdev.h>

enum bkops_operation {
	BKOPS_STOP = 0,
	BKOPS_START,
};

#define BKOPS_START_STATUS 0
#define BKOPS_QUERY_NEEDED BIT(1)
#define BKOPS_CHK_ACCU_WRITE BIT(2)
#define BKOPS_CHK_ACCU_DISCARD BIT(3)
#define BKOPS_CHK_TIME_INTERVAL BIT(4)
#define BKOPS_ASYNC_WORK_STARTED 5
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
#define BKOPS_IDLE_STATE 6
#define BKOPS_FS_STARTED 7
#endif

#ifdef CONFIG_MAS_DEBUG_FS
struct bkops_debug_ops {
	bool sim_bkops_start_fail;
	bool sim_bkops_stop_fail;
	bool sim_bkops_query_fail;
	bool sim_critical_bkops;
	bool sim_bkops_abort;
	u32 sim_bkops_stop_delay; /* in ms */
	bool skip_bkops_stop;
	bool disable_bkops;
	bool bkops_force_query;
	u32 sim_bkops_query_delay; /* in ms */
};

#define BKOPS_DUR_100MS 100
#define BKOPS_DUR_500MS 500
#define BKOPS_DUR_1000MS 1000
#define BKOPS_DUR_2000MS 2000
#define BKOPS_DUR_5000MS 5000
enum bkops_dur_enum {
	BKOPS_DUR_IDX_100MS = 0,
	BKOPS_DUR_IDX_500MS,
	BKOPS_DUR_IDX_1000MS,
	BKOPS_DUR_IDX_2000MS,
	BKOPS_DUR_IDX_5000MS,
	BKOPS_DUR_IDX_FOR_AGES,
	BKOPS_DUR_IDX_DUR_NUM,
};

#define	BKOPS_STATUS_NUM_MAX	10
struct bkops_stats {
	u32 bkops_retry_count;
	u32 bkops_start_count;
	u32 bkops_stop_count;
	/* how many manual bkops has been stopped by bkops core framework */
	u32 bkops_core_stop_count;
	u32 bkops_abort_count; /* bkops was interrupted before completed */
	u32 bkops_actual_query_count;
	u32 bkops_idle_work_canceled_count;
	u32 bkops_idle_work_waited_count;
	u32 bkops_time_query_count;
	u32 bkops_write_query_count;
	u32 bkops_discard_query_count;
	u32 bkops_query_fail_count;
	u32 bkops_start_fail_count;
	u32 bkops_stop_fail_count;
	u64 bkops_start_time; /* last bkops start time in ns */

	u64 bkops_max_query_time; /* in ns */
	u64 bkops_avrg_query_time; /* in ns */
	u64 bkops_max_start_time; /* in ns */
	u64 bkops_avrg_start_time; /* in ns */
	u64 bkops_max_stop_time; /* in ns */
	u64 bkops_avrg_stop_time; /* in ns */
	u64 max_bkops_duration; /* in ns */
	u64 bkops_avrg_exe_time; /* in ns */

	u32 bkops_dur[BKOPS_DUR_IDX_DUR_NUM]; /* in ms */
	u32 bkops_status[BKOPS_STATUS_NUM_MAX];
	u32 bkops_status_max;
	const char **bkops_status_str;

#ifdef CONFIG_MAS_UNISTORE_PRESERVE
	u32 bkops_fs_work_start_idle_count;
	u32 bkops_fs_work_start_not_idle_count;
	u32 bkops_fs_work_start_already_start_count;
	u32 bkops_fs_work_start_no_need_count;
	u32 bkops_fs_work_query_count;
	u32 bkops_fs_work_query_not_idle_count;
	u32 bkops_fs_work_query_already_start_count;
	u32 bkops_fs_work_query_no_need_count;
	u32 bkops_fs_work_stop_not_start_count;
	u32 bkops_fs_work_stop_count;
	u32 bkops_idle_fs_work_start_count;
	u32 bkops_idle_fs_work_not_start_count;
	u32 bkops_fs_work_end_count;
#endif
};
#endif /* CONFIG_MAS_DEBUG_FS */

enum bkops_dev_type {
	BKOPS_DEV_NONE = 0,
	BKOPS_DEV_MMC,
	BKOPS_DEV_UFS_1861,
	BKOPS_DEV_UFS_HYNIX,
	BKOPS_DEV_TYPE_MAX,
};

struct mas_bkops {
	enum bkops_dev_type dev_type;
	struct bkops_ops *bkops_ops;
	void *bkops_data;
	struct request_queue *q;
	struct delayed_work bkops_idle_work;
	unsigned long bkops_flag;
	unsigned long bkops_idle_delay_ms;
	struct blk_busyidle_event_node busyidle_event_node;

	u32 bkops_status; /* bkops status of last query */
	unsigned int en_bkops_retry;
	long bkops_check_interval; /* in seconds */
	long last_bkops_query_time; /* in seconds */

	unsigned long bkops_check_discard_len; /* in bytes */
	unsigned long last_discard_len; /* in bytes */

	unsigned long bkops_check_write_len; /* in bytes */
	unsigned long last_write_len; /* in bytes */

#ifdef CONFIG_MAS_DEBUG_FS
	struct bkops_stats bkops_stats;
	struct dentry *bkops_root;
	struct bkops_debug_ops bkops_debug_ops;
#endif
};

typedef int (bkops_start_stop_fn)(void *bkops_data, int start);
typedef int (bkops_status_query_fn)(void *bkops_data, u32 *status);
struct bkops_ops {
	bkops_start_stop_fn *bkops_start_stop;
	bkops_status_query_fn *bkops_status_query;
};

#define BKOPS_DEF_IDLE_DELAY	1000 /* in ms */
#define BKOPS_DEF_CHECK_INTERVAL	(60 * 60) /* in seconds */
#define BKOPS_DEF_DISCARD_LEN	(512 * 1024 * 1024) /* in bytes */
#define BKOPS_DEF_WRITE_LEN		(512 * 1024 * 1024) /* in bytes */

struct mas_bkops *mas_bkops_alloc(void);
void mas_bkops_set_status_str(
	struct mas_bkops *bkops, u32 bkops_stat_max,
	const char **bkops_stat_str);
int mas_bkops_enable(
	struct request_queue *q, struct mas_bkops *bkops, struct dentry *pdir);
int mas_bkops_add_debugfs(
	struct mas_bkops *bkops, const struct dentry *bkops_root);
#ifdef CONFIG_MAS_DEBUG_FS
void mas_bkops_update_dur(struct bkops_stats *bkops_stats_p);
#endif
void mas_bkops_remove_debugfs(struct mas_bkops *bkops);
#endif /* MAS_BKOPS_CORE_H */
