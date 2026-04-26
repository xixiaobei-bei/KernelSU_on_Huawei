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
 */

#ifndef HYPERHOLD_INF_H
#define HYPERHOLD_INF_H

enum hyperhold_mcg_member {
	MCG_ZRAM_STORED_SZ = 0,
	MCG_ZRAM_PG_SZ,
	MCG_DISK_STORED_SZ,
	MCG_DISK_STORED_PG_SZ,
	MCG_ANON_FAULT_CNT,
	MCG_DISK_FAULT_CNT,
	MCG_SWAPOUT_CNT,
	MCG_SWAPOUT_SZ,
	MCG_SWAPIN_CNT,
	MCG_SWAPIN_SZ,
	MCG_DISK_SPACE,
	MCG_DISK_SPACE_PEAK,
};

#ifdef CONFIG_HP_CORE
#ifdef CONFIG_HISI_DEBUG_FS
extern const struct file_operations proc_hyperhold_operations;
#endif
extern int hyperhold_batch_out(struct mem_cgroup *mcg,
				unsigned long size, bool preload);

extern unsigned long hyperhold_reclaim_in(unsigned long size);

extern unsigned long hyperhold_get_zram_used_pages(void);
extern unsigned long hyperhold_get_eswap_used_pages(void);
extern unsigned long long hyperhold_get_zram_pagefault(void);

extern bool hyperhold_reclaim_work_running(void);

extern void hyperhold_mem_cgroup_remove(struct mem_cgroup *memcg);

extern void hyperhold_psi_show(struct seq_file *m);

extern unsigned long long hyperhold_read_mcg_stats(
	struct mem_cgroup *mcg, enum hyperhold_mcg_member mcg_member);

extern bool hyperhold_enable(void);

static inline void inc_memcg_zram_swapout_cnt(struct mem_cgroup *memcg)
{
	return atomic64_inc(&memcg->zram_swapout_cnt);
}

static inline void inc_memcg_zram_swapin_cnt(struct mem_cgroup *memcg)
{
	return atomic64_inc(&memcg->zram_swapin_cnt);
}
extern void hyperhold_force_reclaim(struct mem_cgroup *mcg);
#else
static inline int hyperhold_batch_out(struct mem_cgroup *mcg,
				unsigned long size, bool preload)
{
	return 0;
}

static inline unsigned long hyperhold_reclaim_in(unsigned long size)
{
	return 0;
}

static inline unsigned long hyperhold_get_zram_used_pages(void)
{
	return 0;
}

static inline unsigned long hyperhold_get_eswap_used_pages(void)
{
	return 0;
}

static inline unsigned long long hyperhold_get_zram_pagefault(void)
{
	return 0;
}

static inline bool hyperhold_reclaim_work_running(void)
{
	return false;
}

static inline void hyperhold_mem_cgroup_remove(struct mem_cgroup *memcg) {}
static inline void hyperhold_psi_show(struct seq_file *m) {}

static inline unsigned long long hyperhold_read_mcg_stats(
	struct mem_cgroup *mcg, enum hyperhold_mcg_member mcg_member)
{
	return 0;
}

static inline bool hyperhold_enable(void)
{
	return 0;
}

static inline void inc_memcg_zram_swapout_cnt(struct mem_cgroup *memcg)
{
	return;
}

static inline void inc_memcg_zram_swapin_cnt(struct mem_cgroup *memcg)
{
	return;
}
static inline void hyperhold_force_reclaim(struct mem_cgroup *mcg) {}
#endif

#endif
