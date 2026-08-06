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
#include <linux/timer.h>
#include <linux/zsmalloc.h>
#include <linux/spinlock.h>
#include <linux/sched/task.h>

#include "hyperhold_internal.h"

#define DUMP_BUF_LEN 512

static unsigned long warning_threshold[HYPERHOLD_SCENARIO_BUTT] = {
	0, 200, 500, 0
};

const char *key_point_name[HYPERHOLD_KYE_POINT_BUTT] = {
	"START",
	"INIT",
	"IOENTRY_ALLOC",
	"FIND_EXTENT",
	"IO_EXTENT",
	"SEGMENT_ALLOC",
	"BIO_ALLOC",
	"SUBMIT_BIO",
	"END_IO",
	"SCHED_WORK",
	"END_WORK",
	"CALL_BACK",
	"WAKE_UP",
	"ZRAM_LOCK",
	"DONE"
};

static void hyperhold_dump_point_lat(
	struct hyperhold_key_point_record *record, ktime_t start)
{
	int i;

	for (i = 0; i < HYPERHOLD_KYE_POINT_BUTT; ++i) {
		if (!record->key_point[i].record_cnt)
			continue;

		hh_print(HHLOG_ERR,
			"%s diff %lld cnt %u end %u lat %lld ravg_sum %llu\n",
			key_point_name[i],
			ktime_us_delta(record->key_point[i].first_time, start),
			record->key_point[i].record_cnt,
			record->key_point[i].end_cnt,
			record->key_point[i].proc_total_time,
			record->key_point[i].proc_ravg_sum);
	}
}

static void hyperhold_dump_no_record_point(
	struct hyperhold_key_point_record *record, char *log,
	unsigned int *count)
{
	int i;
	unsigned int point = 0;

	for (i = 0; i < HYPERHOLD_KYE_POINT_BUTT; ++i)
		if (record->key_point[i].record_cnt)
			point = i;

	point++;
	if (point < HYPERHOLD_KYE_POINT_BUTT)
		*count += snprintf(log + *count,
			(size_t)(DUMP_BUF_LEN - *count),
			" no_record_point %s", key_point_name[point]);
	else
		*count += snprintf(log + *count,
			(size_t)(DUMP_BUF_LEN - *count), " all_point_record");
}

static long long hyperhold_calc_speed(s64 page_cnt, s64 time)
{
	s64 size;

	if (!page_cnt)
		return 0;

	size = page_cnt * PAGE_SIZE * BITS_PER_BYTE;
	if (time)
		return size * USEC_PER_SEC / time;
	else
		return S64_MAX;
}

static void hyperhold_dump_lat(
	struct hyperhold_key_point_record *record, ktime_t curr_time,
	bool perf_end_flag)
{
	char log[DUMP_BUF_LEN] = { 0 };
	unsigned int count = 0;
	ktime_t start;
	s64 total_time;

	start = record->key_point[HYPERHOLD_START].first_time;
	total_time = ktime_us_delta(curr_time, start);
	count += snprintf(log + count,
		(size_t)(DUMP_BUF_LEN - count),
		"totaltime(us) %lld scenario %u task %s nice %d",
		total_time, record->scenario, record->task_comm, record->nice);

	if (perf_end_flag)
		count += snprintf(log + count, (size_t)(DUMP_BUF_LEN - count),
			" page %d segment %d speed(bps) %lld threshold %llu",
			record->page_cnt, record->segment_cnt,
			hyperhold_calc_speed(record->page_cnt, total_time),
			record->warning_threshold);
	else
		count += snprintf(log + count, (size_t)(DUMP_BUF_LEN - count),
			" state %c", task_state_to_char(record->task));

	hyperhold_dump_no_record_point(record, log, &count);

	hh_print(HHLOG_ERR, "perf end flag %u %s\n", perf_end_flag, log);

	hyperhold_dump_point_lat(record, start);
}

static unsigned long hyperhold_perf_warning_threshold(
	enum hyperhold_scenario scenario)
{
	if (unlikely(scenario >= HYPERHOLD_SCENARIO_BUTT))
		return 0;

	return warning_threshold[scenario];
}

static void hyperhold_perf_warning(unsigned long data)
{
	struct hyperhold_key_point_record *record =
		(struct hyperhold_key_point_record *)data;

	if (!record->warning_threshold)
		return;

	hyperhold_dump_lat(record, ktime_get(), false);

	if (likely(record->task))
		show_stack(record->task, NULL);
	record->warning_threshold <<= 2;
	record->timeout_flag = true;
	mod_timer(&record->lat_monitor,
		jiffies + msecs_to_jiffies(record->warning_threshold));
}

static void hyperhold_perf_init_monitor(
	struct hyperhold_key_point_record *record,
	enum hyperhold_scenario scenario)
{
	record->warning_threshold = hyperhold_perf_warning_threshold(scenario);

	if (!record->warning_threshold)
		return;

	record->task = current;
	get_task_struct(record->task);
	init_timer(&record->lat_monitor);
	record->lat_monitor.function = hyperhold_perf_warning;
	record->lat_monitor.data = (uintptr_t)record;
	mod_timer(&record->lat_monitor,
			jiffies +
			msecs_to_jiffies(record->warning_threshold));
}

static void hyperhold_perf_stop_monitor(
	struct hyperhold_key_point_record *record)
{
	if (!record->warning_threshold)
		return;

	del_timer_sync(&record->lat_monitor);
	put_task_struct(record->task);
}

static void hyperhold_perf_init(struct hyperhold_key_point_record *record,
	enum hyperhold_scenario scenario)
{
	int i;

	for (i = 0; i < HYPERHOLD_KYE_POINT_BUTT; ++i)
		spin_lock_init(&record->key_point[i].time_lock);

	record->nice = task_nice(current);
	record->scenario = scenario;
	get_task_comm(record->task_comm, current); /*lint !e514*/
	hyperhold_perf_init_monitor(record, scenario);
}

void hyperhold_perf_start_proc(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type, ktime_t curr_time,
	unsigned long long current_ravg_sum)
{
	struct hyperhold_key_point_info *key_point =
		&record->key_point[type];

	if (!key_point->record_cnt)
		key_point->first_time = curr_time;

	key_point->record_cnt++;
	key_point->last_time = curr_time;
	key_point->last_ravg_sum = current_ravg_sum;
}

void hyperhold_perf_end_proc(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type, ktime_t curr_time,
	unsigned long long current_ravg_sum)
{
	struct hyperhold_key_point_info *key_point =
		&record->key_point[type];
	s64 diff_time = ktime_us_delta(curr_time, key_point->last_time);

	key_point->proc_total_time += diff_time;
	if (diff_time > key_point->proc_max_time)
		key_point->proc_max_time = diff_time;

	key_point->proc_ravg_sum += current_ravg_sum -
		key_point->last_ravg_sum;
	key_point->end_cnt++;
	key_point->last_time = curr_time;
	key_point->last_ravg_sum = current_ravg_sum;
}

void hyperhold_perf_async_perf(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type, ktime_t start,
	unsigned long long start_ravg_sum)
{
	unsigned long long current_ravg_sum = ((type == HYPERHOLD_CALL_BACK) ||
		(type == HYPERHOLD_END_WORK)) ? hyperhold_get_ravg_sum() : 0;
	unsigned long flags;

	spin_lock_irqsave(&record->key_point[type].time_lock, flags);
	hyperhold_perf_start_proc(record, type, start, start_ravg_sum);
	hyperhold_perf_end_proc(record, type, ktime_get(),
		current_ravg_sum);
	spin_unlock_irqrestore(&record->key_point[type].time_lock, flags);
}

void hyperhold_perf_lat_point(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type)
{
	hyperhold_perf_start_proc(record, type, ktime_get(),
		hyperhold_get_ravg_sum());
	record->key_point[type].end_cnt++;
}

void hyperhold_perf_start(
	struct hyperhold_key_point_record *record,
	ktime_t stsrt, unsigned long long start_ravg_sum,
	enum hyperhold_scenario scenario)
{
	hyperhold_perf_init(record, scenario);
	hyperhold_perf_start_proc(record, HYPERHOLD_START, stsrt,
		start_ravg_sum);
	record->key_point[HYPERHOLD_START].end_cnt++;
}

void hyperhold_perf_lat_stat(
	struct hyperhold_key_point_record *record)
{
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();
	s64 curr_lat;
	/* reclaim_in: 2s, fault_out: 100ms, batch_out: 500ms, pre_out: 2s */
	s64 timeout_value[HYPERHOLD_SCENARIO_BUTT] = {
		2000000, 100000, 500000, 2000000
	};

	if (!stat || (record->scenario >= HYPERHOLD_SCENARIO_BUTT))
		return;

	curr_lat = ktime_us_delta(record->key_point[HYPERHOLD_DONE].first_time,
		record->key_point[HYPERHOLD_START].first_time);
	atomic64_add(curr_lat, &stat->lat[record->scenario].total_lat);
	if (curr_lat > atomic64_read(&stat->lat[record->scenario].max_lat))
		atomic64_set(&stat->lat[record->scenario].max_lat, curr_lat);
	if (curr_lat > timeout_value[record->scenario])
		atomic64_inc(&stat->lat[record->scenario].timeout_cnt);
}

void hyperhold_perf_end(struct hyperhold_key_point_record *record)
{
	int loglevel;

	hyperhold_perf_stop_monitor(record);
	hyperhold_perf_lat_point(record, HYPERHOLD_DONE);
	hyperhold_perf_lat_stat(record);

	loglevel = record->timeout_flag ? HHLOG_ERR : HHLOG_INFO;
	if (loglevel > hyperhold_loglevel())
		return;

	hyperhold_dump_lat(record,
		record->key_point[HYPERHOLD_DONE].first_time, true);
}

void hyperhold_perf_lat_start(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type)
{
	hyperhold_perf_start_proc(record, type, ktime_get(),
		hyperhold_get_ravg_sum());
}

void hyperhold_perf_lat_end(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type)
{
	hyperhold_perf_end_proc(record, type, ktime_get(),
		hyperhold_get_ravg_sum());
}

void hyperhold_perf_io_stat(
	struct hyperhold_key_point_record *record, int page_cnt,
	int segment_cnt)
{
	record->page_cnt = page_cnt;
	record->segment_cnt = segment_cnt;
}

