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
 * Create: 2020-5-15
 *
 */
#define pr_fmt(fmt) "[HYPERHOLD]" fmt

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/atomic.h>
#include <linux/zsmalloc.h>
#include <linux/memcontrol.h>
#include <linux/hyperhold_inf.h>

#include "hyperhold_internal.h"

#define SCENARIO_NAME_LEN 32
#define MBYTE_SHIFT 20

static char scenario_name[HYPERHOLD_SCENARIO_BUTT][SCENARIO_NAME_LEN] = {
	"reclaim_in",
	"fault_out",
	"batch_out",
	"pre_out"
};

static void hyperhold_lat_show(struct seq_file *m,
	struct hyperhold_stat *stat)
{
	int i;

	for (i = 0; i < HYPERHOLD_SCENARIO_BUTT; ++i) {
		seq_printf(m, "hyperhold_%s_total_lat:%lld\n",
			scenario_name[i],
			atomic64_read(&stat->lat[i].total_lat));
		seq_printf(m, "hyperhold_%s_max_lat:%lld\n",
			scenario_name[i],
			atomic64_read(&stat->lat[i].max_lat));
		seq_printf(m, "hyperhold_%s_timeout_cnt:%lld\n",
			scenario_name[i],
			atomic64_read(&stat->lat[i].timeout_cnt));
	}
}

static void hyperhold_stats_show(struct seq_file *m,
	struct hyperhold_stat *stat)
{
	seq_printf(m, "hyperhold_out_times:%lld\n",
		atomic64_read(&stat->reclaimin_cnt));
	seq_printf(m, "hyperhold_out_comp_size:%lld\n",
		atomic64_read(&stat->reclaimin_bytes) >> MBYTE_SHIFT);
	if (PAGE_SHIFT < MBYTE_SHIFT)
		seq_printf(m, "hyperhold_out_ori_size:%lld\n",
			atomic64_read(&stat->reclaimin_pages) >>
				(MBYTE_SHIFT - PAGE_SHIFT));
	seq_printf(m, "hyperhold_in_times:%lld\n",
		atomic64_read(&stat->batchout_cnt));
	seq_printf(m, "hyperhold_in_comp_size:%lld\n",
		atomic64_read(&stat->batchout_bytes) >> MBYTE_SHIFT);
	if (PAGE_SHIFT < MBYTE_SHIFT)
		seq_printf(m, "hyperhold_in_ori_size:%lld\n",
		atomic64_read(&stat->batchout_pages) >>
			(MBYTE_SHIFT - PAGE_SHIFT));
	seq_printf(m, "hyperhold_all_fault:%lld\n",
		atomic64_read(&stat->fault_cnt));
	seq_printf(m, "hyperhold_fault:%lld\n",
		atomic64_read(&stat->hyperhold_fault_cnt));
}

static void hyperhold_area_info_show(struct seq_file *m,
	struct hyperhold_stat *stat)
{
	seq_printf(m, "hyperhold_reout_ori_size:%lld\n",
		atomic64_read(&stat->reout_pages) >>
			(MBYTE_SHIFT - PAGE_SHIFT));
	seq_printf(m, "hyperhold_reout_comp_size:%lld\n",
		atomic64_read(&stat->reout_bytes) >> MBYTE_SHIFT);
	seq_printf(m, "hyperhold_zram_comp_size:%lld\n",
		atomic64_read(&stat->zram_stored_size) >> MBYTE_SHIFT);
	seq_printf(m, "hyperhold_zram_ori_size:%lld\n",
		atomic64_read(&stat->zram_stored_pages) >>
			(MBYTE_SHIFT - PAGE_SHIFT));
	seq_printf(m, "hyperhold_store_comp_size:%lld\n",
		atomic64_read(&stat->stored_size) >> MBYTE_SHIFT);
	seq_printf(m, "hyperhold_store_ori_size:%lld\n",
		atomic64_read(&stat->stored_pages) >>
			(MBYTE_SHIFT - PAGE_SHIFT));
	seq_printf(m, "hyperhold_notify_free_size:%lld\n",
		atomic64_read(&stat->notify_free) >>
			(MBYTE_SHIFT - EXTENT_SHIFT));
	seq_printf(m, "hyperhold_store_memcg_cnt:%lld\n",
		atomic64_read(&stat->mcg_cnt));
	seq_printf(m, "hyperhold_store_extent_cnt:%lld\n",
		atomic64_read(&stat->ext_cnt));
	seq_printf(m, "hyperhold_store_fragment_cnt:%lld\n",
		atomic64_read(&stat->frag_cnt));
}

static void hyperhold_fail_show(struct seq_file *m,
	struct hyperhold_stat *stat)
{
	int i;

	for (i = 0; i < HYPERHOLD_SCENARIO_BUTT; ++i) {
		seq_printf(m, "hyperhold_%s_io_fail_cnt:%lld\n",
			scenario_name[i],
			atomic64_read(&stat->io_fail_cnt[i]));
		seq_printf(m, "hyperhold_%s_alloc_fail_cnt:%lld\n",
			scenario_name[i],
			atomic64_read(&stat->alloc_fail_cnt[i]));
	}
}

void hyperhold_psi_show(struct seq_file *m)
{
	struct hyperhold_stat *stat = NULL;

	if (!hyperhold_enable())
		return;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat)) {
		hh_print(HHLOG_ERR, "can't get stat obj!\n");
		return;
	}

	hyperhold_stats_show(m, stat);
	hyperhold_area_info_show(m, stat);
	hyperhold_lat_show(m, stat);
	hyperhold_fail_show(m, stat);
}

unsigned long hyperhold_get_zram_used_pages(void)
{
	struct hyperhold_stat *stat = NULL;

	if (!hyperhold_enable())
		return 0;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat)) {
		hh_print(HHLOG_ERR, "can't get stat obj!\n");

		return 0;
	}

	return atomic64_read(&stat->zram_stored_pages);
}

unsigned long hyperhold_get_eswap_used_pages(void)
{
	struct hyperhold_stat *stat = NULL;

	if (!hyperhold_enable())
		return 0;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat)) {
		hh_print(HHLOG_ERR, "can't get stat obj!\n");
		return 0;
	}

	return atomic64_read(&stat->stored_pages);
}

unsigned long long hyperhold_get_zram_pagefault(void)
{
	struct hyperhold_stat *stat = NULL;

	if (!hyperhold_enable())
		return 0;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat)) {
		hh_print(HHLOG_ERR, "can't get stat obj!\n");

		return 0;
	}

	return atomic64_read(&stat->fault_cnt);
}

bool hyperhold_reclaim_work_running(void)
{
	struct hyperhold_stat *stat = NULL;

	if (!hyperhold_enable())
		return false;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat)) {
		hh_print(HHLOG_ERR, "can't get stat obj!\n");

		return 0;
	}

	return atomic64_read(&stat->reclaimin_infight) ? true : false;
}

unsigned long long hyperhold_read_mcg_stats(struct mem_cgroup *mcg,
				enum hyperhold_mcg_member mcg_member)
{
	unsigned long long val = 0;
	int extcnt;

	if (!hyperhold_enable())
		return 0;

	switch (mcg_member) {
	case MCG_ZRAM_STORED_SZ:
		val = atomic64_read(&mcg->zram_stored_size);
		break;
	case MCG_ZRAM_PG_SZ:
		val = atomic64_read(&mcg->zram_page_size);
		break;
	case MCG_DISK_STORED_SZ:
		val = atomic64_read(&mcg->hyperhold_stored_size);
		break;
	case MCG_DISK_STORED_PG_SZ:
		val = atomic64_read(&mcg->hyperhold_stored_pages);
		break;
	case MCG_ANON_FAULT_CNT:
		val = atomic64_read(&mcg->hyperhold_allfaultcnt);
		break;
	case MCG_DISK_FAULT_CNT:
		val = atomic64_read(&mcg->hyperhold_faultcnt);
		break;
	case MCG_SWAPOUT_CNT:
		val = atomic64_read(&mcg->hyperhold_outcnt);
		break;
	case MCG_SWAPOUT_SZ:
		val = atomic64_read(&mcg->hyperhold_outextcnt) << EXTENT_SHIFT;
		break;
	case MCG_SWAPIN_CNT:
		val = atomic64_read(&mcg->hyperhold_incnt);
		break;
	case MCG_SWAPIN_SZ:
		val = atomic64_read(&mcg->hyperhold_inextcnt) << EXTENT_SHIFT;
		break;
	case MCG_DISK_SPACE:
		extcnt = atomic_read(&mcg->hyperhold_extcnt);
		if (extcnt < 0)
			extcnt = 0;
		val = ((unsigned long long) extcnt) << EXTENT_SHIFT; /*lint !e571*/
		break;
	case MCG_DISK_SPACE_PEAK:
		extcnt = atomic_read(&mcg->hyperhold_peakextcnt);
		if (extcnt < 0)
			extcnt = 0;
		val = ((unsigned long long) extcnt) << EXTENT_SHIFT; /*lint !e571*/
		break;
	default:
		break;
	}

	return val;
}

void hyperhold_fail_record(enum hyperhold_fail_point point,
	u32 index, int ext_id, unsigned char *task_comm)
{
	struct hyperhold_stat *stat = NULL;
	unsigned long flags;
	unsigned int copylen = strlen(task_comm) + 1;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat)) {
		hh_print(HHLOG_ERR, "can't get stat obj!\n");
		return;
	}

	if (copylen > TASK_COMM_LEN) {
		hh_print(HHLOG_ERR, "task_comm len %d is err\n", copylen);
		return;
	}

	spin_lock_irqsave(&stat->record.lock, flags);
	if (stat->record.num < MAX_FAIL_RECORD_NUM) {
		stat->record.record[stat->record.num].point = point;
		stat->record.record[stat->record.num].index = index;
		stat->record.record[stat->record.num].ext_id = ext_id;
		stat->record.record[stat->record.num].time = ktime_get();
		memcpy(stat->record.record[stat->record.num].task_comm,
			task_comm, copylen);
		stat->record.num++;
	}
	spin_unlock_irqrestore(&stat->record.lock, flags);
}

static void hyperhold_fail_record_get(
	struct hyperhold_fail_record_info *record_info)
{
	struct hyperhold_stat *stat = NULL;
	unsigned long flags;

	if (!hyperhold_enable())
		return;

	stat = hyperhold_get_stat_obj();
	if (unlikely(!stat)) {
		hh_print(HHLOG_ERR, "can't get stat obj!\n");
		return;
	}

	spin_lock_irqsave(&stat->record.lock, flags);
	memcpy(record_info, &stat->record,
		sizeof(struct hyperhold_fail_record_info));
	stat->record.num = 0;
	spin_unlock_irqrestore(&stat->record.lock, flags);
}

static ssize_t hyperhold_fail_record_show(char *buf)
{
	int i;
	ssize_t size = 0;
	struct hyperhold_fail_record_info record_info = { 0 };

	hyperhold_fail_record_get(&record_info);

	size += scnprintf(buf + size, PAGE_SIZE,
			"hyperhold_fail_record_num: %d\n", record_info.num);
	for (i = 0; i < record_info.num; ++i)
		size += scnprintf(buf + size, PAGE_SIZE - size,
			"point[%u]time[%lld]taskname[%s]index[%u]ext_id[%d]\n",
			record_info.record[i].point,
			ktime_us_delta(ktime_get(),
				record_info.record[i].time),
			record_info.record[i].task_comm,
			record_info.record[i].index,
			record_info.record[i].ext_id);

	return size;
}

ssize_t hyperhold_report_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return hyperhold_fail_record_show(buf);
}

