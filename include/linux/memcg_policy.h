/*
 * memcg_policy.h
 *
 * Copyright(C) 2020 Huawei Technologies Co., Ltd. All rights reserved.
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
 */

#ifndef _MEMCG_POLICY_H
#define _MEMCG_POLICY_H

struct mem_cgroup;
struct seqfile;
struct pg_data_t;
struct scan_control;

extern struct list_head score_head;
extern bool score_head_inited;
extern spinlock_t score_list_lock;

struct memcg_reclaim {
	atomic64_t ub_ufs2zram_ratio;
	atomic_t ub_zram2ufs_ratio;
	atomic64_t app_score;
#ifdef CONFIG_HYPERHOLD_ZSWAPD
	atomic_t ub_mem2zram_ratio;
	atomic_t refault_threshold;
	/* anon refault */
	unsigned long long reclaimed_pagefault;
#endif
};

void memcg_app_score_update(struct mem_cgroup *target);
struct mem_cgroup *get_next_memcg(struct mem_cgroup *prev);
void get_next_memcg_break(struct mem_cgroup *prev);

#ifdef CONFIG_HYPERHOLD_FILE_LRU
void shrink_anon_memcg(struct pglist_data *pgdat,
		struct mem_cgroup *memcg, struct scan_control *sc,
		unsigned long *nr);
bool shrink_node_hyperhold(pg_data_t *pgdat, struct scan_control *sc);
#endif

#ifdef CONFIG_HYPERHOLD_DEBUG
void memcg_eswap_info_show(struct seq_file *m);
#endif

#ifdef CONFIG_HYPERHOLD_ZSWAPD
extern int zswapd_run(int nid);
extern void zswapd_stop(int nid);
extern void wakeup_zswapd(pg_data_t *pgdat);
extern bool zram_watermark_ok(void);
extern void zswapd_status_show(struct seq_file *m);
extern void wake_all_zswapd(void);
#ifndef CONFIG_REFAULT_IO_VMSCAN
extern void set_snapshotd_init_flag(unsigned int val);
#endif
extern pid_t get_zswapd_pid(void);
extern u64 get_free_swap_threshold_value(void);
#else
static inline int zswapd_run(int nid)
{
	return 0;
}

static inline void zswapd_stop(int nid)
{
}

static inline void wakeup_zswapd(pg_data_t *pgdat)
{
}

static inline bool zram_watermark_ok(void)
{
	return true;
}

static inline void zswapd_status_show(struct seq_file *m)
{
}

static inline void wake_all_zswapd(void)
{
}

#ifndef CONFIG_REFAULT_IO_VMSCAN
static inline void set_snapshotd_init_flag(unsigned int val)
{
}
#endif

static inline pid_t get_zswapd_pid(void)
{
	return -EINVAL;
}

static inline u64 get_free_swap_threshold_value(void);
{
	return 0;
}
#endif

#endif/* _LINUX_MEMCG_POLICY_H */
