/*
 * memcg_control.c
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2020. All rights reserved.
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

#include <linux/memcontrol.h>
#include <linux/cgroup.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/mutex.h>
#include <trace/events/block.h>
#include <linux/cpuset.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <uapi/linux/sched/types.h>

#ifdef CONFIG_MEMCG_PROTECT_LRU
#include <linux/protect_lru.h>
#endif

#ifdef CONFIG_HP_CORE
#include <linux/hyperhold_inf.h>
#endif

#include <linux/memcg_policy.h>
#include "memcg_policy_internal.h"
#include "internal.h"

#define MAX_APP_SCORE 1000
#define MAX_RATIO 100

struct list_head score_head;
bool score_head_inited;
atomic_t ec_app_start_flag = ATOMIC_INIT(0);
static DEFINE_MUTEX(reclaim_para_lock);
DEFINE_SPINLOCK(score_list_lock);

#ifdef CONFIG_HYPERHOLD_ZSWAPD
static DEFINE_MUTEX(pressure_event_lock);
struct eventfd_ctx *zswapd_press_efd[LEVEL_COUNT];
#define ZRAM_WM_RATIO 0
#define COMPRESS_RATIO 30
atomic64_t zram_wm_ratio = ATOMIC_LONG_INIT(ZRAM_WM_RATIO);
atomic64_t compress_ratio = ATOMIC_LONG_INIT(COMPRESS_RATIO);
struct zswapd_param {
	unsigned int min_score;
	unsigned int max_score;
	unsigned int ub_mem2zram_ratio;
	unsigned int ub_zram2ufs_ratio;
	unsigned int refault_threshold;
};
#define ZSWAPD_MAX_LEVEL_NUM 10
static struct zswapd_param zswap_param[ZSWAPD_MAX_LEVEL_NUM];
#define INACTIVE_FILE_RATIO 90
#define ACTIVE_FILE_RATIO 70
#define MAX_RECLAIM_SIZE 100
atomic_t avail_buffers = ATOMIC_INIT(0);
atomic_t min_avail_buffers = ATOMIC_INIT(0);
atomic_t high_avail_buffers = ATOMIC_INIT(0);
atomic_t max_reclaim_size = ATOMIC_INIT(MAX_RECLAIM_SIZE);
atomic64_t free_swap_threshold = ATOMIC64_INIT(0);
atomic_t inactive_file_ratio = ATOMIC_INIT(INACTIVE_FILE_RATIO);
atomic_t active_file_ratio = ATOMIC_INIT(ACTIVE_FILE_RATIO);
atomic64_t zram_crit_thres = ATOMIC_LONG_INIT(0);
#ifndef CONFIG_REFAULT_IO_VMSCAN
#define AREA_ANON_REFAULT_THRESHOLD 22000
#else
/*
 * area anon refault threshold ratio, with swappiness,
 * means anon cost: file cost=4:1
 */
#define AREA_ANON_REFAULT_THRESHOLD 80
#endif
#define ANON_REFAULT_SNAPSHOT_MIN_INTERVAL 200
#define EMPTY_ROUND_SKIP_INTERNVAL 20
#define MAX_SKIP_INTERVAL 1000
#define EMPTY_ROUND_CHECK_THRESHOLD 10
atomic64_t area_anon_refault_threshold =
	ATOMIC_LONG_INIT(AREA_ANON_REFAULT_THRESHOLD);
atomic64_t anon_refault_snapshot_min_interval =
	ATOMIC_LONG_INIT(ANON_REFAULT_SNAPSHOT_MIN_INTERVAL);
atomic64_t empty_round_skip_interval =
	ATOMIC_LONG_INIT(EMPTY_ROUND_SKIP_INTERNVAL);
atomic64_t max_skip_interval =
	ATOMIC_LONG_INIT(MAX_SKIP_INTERVAL);
atomic64_t empty_round_check_threshold =
	ATOMIC_LONG_INIT(EMPTY_ROUND_CHECK_THRESHOLD);

#endif

/**
 * get_next_memcg - iterate over memory cgroup score_list
 * @prev: previously returned memcg, NULL on first invocation
 *
 * Returns references to the next memg on score_list of @prev,
 * or %NULL after a full round-trip.
 *
 * Caller must pass the return value in @prev on subsequent
 * invocations for reference counting, or use get_next_memcg_break()
 * to cancel a walk before the round-trip is complete.
 */
struct mem_cgroup *get_next_memcg(struct mem_cgroup *prev)
{
	struct mem_cgroup *memcg = NULL;
	struct list_head *pos = NULL;
	unsigned long flags;

	if (unlikely(!score_head_inited))
		return NULL;

	spin_lock_irqsave(&score_list_lock, flags);

	if (unlikely(!prev))
		pos = &score_head;
	else
		pos = &prev->score_node;

	if (list_empty(pos)) /* deleted node */
		goto unlock;

	if (pos->next == &score_head)
		goto unlock;

	memcg = list_entry(pos->next,
			struct mem_cgroup, score_node);

	if (!css_tryget(&memcg->css))
		memcg = NULL;

unlock:
	spin_unlock_irqrestore(&score_list_lock, flags);

	if (prev)
		css_put(&prev->css);

	return memcg;
}

void get_next_memcg_break(struct mem_cgroup *memcg)
{
	if (memcg)
		css_put(&memcg->css);
}

static ssize_t mem_cgroup_force_shrink_anon(struct kernfs_open_file *of,
		char *buf, size_t nbytes, loff_t off)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(of_css(of));
	pg_data_t *pgdat = NULL;
	int nid;
	bool prelaunch = false;

	buf = strstrip(buf);
	if (strncmp(buf, "prelaunch", nbytes) == 0)
		prelaunch = true;

	for_each_online_node(nid) {
		pgdat = NODE_DATA(nid);
		if (prelaunch)
			reclaim_all_anon_memcg_prelaunch(pgdat, memcg);
		else
			reclaim_all_anon_memcg(pgdat, memcg);
	}

	return nbytes;
}

inline bool get_ec_app_start_flag_value(void)
{
	return atomic_read(&ec_app_start_flag);
}

static int memcg_total_info_per_app_show(struct seq_file *m, void *v)
{
	struct mem_cgroup *memcg = NULL;
	struct mem_cgroup_per_node *mz = NULL;
	struct lruvec *lruvec = NULL;
	unsigned long anon_size;
	unsigned long zram_compress_size;
	unsigned long eswap_compress_size;

	while ((memcg = get_next_memcg(memcg))) {
		mz = mem_cgroup_nodeinfo(memcg, 0);
		if (!mz) {
			get_next_memcg_break(memcg);
			return 0;
		}

		lruvec = &mz->lruvec;
		if (!lruvec) {
			get_next_memcg_break(memcg);
			return 0;
		}

		anon_size = lruvec_lru_size(lruvec, LRU_ACTIVE_ANON,
			MAX_NR_ZONES) + lruvec_lru_size(lruvec,
			LRU_INACTIVE_ANON, MAX_NR_ZONES);
		zram_compress_size = hyperhold_read_mcg_stats(memcg,
				MCG_ZRAM_STORED_SZ);
		eswap_compress_size = hyperhold_read_mcg_stats(memcg,
				MCG_DISK_STORED_SZ);

		anon_size *= PAGE_SIZE / SZ_1K;
		zram_compress_size /= SZ_1K;
		eswap_compress_size /= SZ_1K;
		if (!strlen(memcg->name)) /* skip root, apps, system */
			continue;
		seq_printf(m, "%s %lu %lu %lu\n", memcg->name, anon_size,
			zram_compress_size, eswap_compress_size);
	}

	return 0;
}

static int mem_cgroup_ub_ufs2zram_ratio_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(css);
	const unsigned int ratio = 100;

	if (val > ratio)
		return -EINVAL;
	atomic64_set(&memcg->memcg_reclaimed.ub_ufs2zram_ratio, val);

	return 0;
}

static int mem_cgroup_ec_app_start_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	atomic_set(&ec_app_start_flag, 1);

	return 0;
}

static int mem_cgroup_force_swapin_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(css);
	unsigned long size;
	const unsigned int ratio = 100;

	size = hyperhold_read_mcg_stats(memcg, MCG_DISK_STORED_SZ);
	size = atomic64_read(&memcg->memcg_reclaimed.ub_ufs2zram_ratio) *
			size / ratio;
#ifdef CONFIG_HP_CORE
	hyperhold_batch_out(memcg, size,
			val ? true : false);
#endif

	return 0;
}

static int mem_cgroup_force_swapout_write(struct cgroup_subsys_state *css,
					  struct cftype *cft, u64 val)
{
#ifdef CONFIG_HP_CORE
	hyperhold_force_reclaim(mem_cgroup_from_css(css));
#endif
	return 0;
}

static ssize_t mem_cgroup_name_write(struct kernfs_open_file *of, char *buf,
				size_t nbytes, loff_t off)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(of_css(of));
	const unsigned int buf_max_size = 100;

	buf = strstrip(buf);
	if (nbytes >= buf_max_size)
		return -EINVAL;

	mutex_lock(&reclaim_para_lock);
	/*lint -e421 */
	if (memcg)
		strcpy(memcg->name, buf);
	/*lint +e421 */
	mutex_unlock(&reclaim_para_lock);

	return nbytes;
}

void memcg_app_score_update(struct mem_cgroup *target)
{
	struct list_head *pos = NULL;
	unsigned long flags;

	spin_lock_irqsave(&score_list_lock, flags);
	list_for_each(pos, &score_head) {
		struct mem_cgroup *memcg = list_entry(pos,
				struct mem_cgroup, score_node);
		if (atomic64_read(&memcg->memcg_reclaimed.app_score) <
			atomic64_read(&target->memcg_reclaimed.app_score))
			break;
	}
	list_move_tail(&target->score_node, pos);
	spin_unlock_irqrestore(&score_list_lock, flags);
}

static int mem_cgroup_app_score_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(css);

	if (val > MAX_APP_SCORE)
		return -EINVAL;

	if (atomic64_read(&memcg->memcg_reclaimed.app_score) != val) {
		atomic64_set(&memcg->memcg_reclaimed.app_score, val);
		memcg_app_score_update(memcg);
	}

	return 0;
}

static int memcg_eswap_stat_show(struct seq_file *m, void *v)
{
	struct mem_cgroup *memcg = NULL;
	unsigned long swap_out_cnt;
	unsigned long swap_out_size;
	unsigned long swap_in_size;
	unsigned long swap_in_cnt;
	unsigned long page_fault_cnt;
	unsigned long cur_eswap_size;
	unsigned long max_eswap_size;

	memcg = mem_cgroup_from_css(seq_css(m));
	swap_out_cnt = hyperhold_read_mcg_stats(memcg, MCG_SWAPOUT_CNT);
	swap_out_size = hyperhold_read_mcg_stats(memcg, MCG_SWAPOUT_SZ);
	swap_in_size = hyperhold_read_mcg_stats(memcg, MCG_SWAPIN_SZ);
	swap_in_cnt = hyperhold_read_mcg_stats(memcg, MCG_SWAPIN_CNT);
	page_fault_cnt = hyperhold_read_mcg_stats(memcg, MCG_DISK_FAULT_CNT);
	cur_eswap_size = hyperhold_read_mcg_stats(memcg, MCG_DISK_SPACE);
	max_eswap_size = hyperhold_read_mcg_stats(memcg, MCG_DISK_SPACE_PEAK);

	seq_printf(m, "swapOutTotal:%lu\n", swap_out_cnt);
	seq_printf(m, "swapOutSize:%lu MB\n", swap_out_size / SZ_1M);
	seq_printf(m, "swapInSize:%lu MB\n", swap_in_size / SZ_1M);
	seq_printf(m, "swapInTotal:%lu\n", swap_in_cnt);
	seq_printf(m, "pageInTotal:%lu\n", page_fault_cnt);
	seq_printf(m, "swapSizeCur:%lu MB\n", cur_eswap_size / SZ_1M);
	seq_printf(m, "swapSizeMax:%lu MB\n", max_eswap_size / SZ_1M);

	return 0;
}

#ifdef CONFIG_HYPERHOLD_ZSWAPD
inline u64 get_zram_wm_ratio_value(void)
{
	return atomic64_read(&zram_wm_ratio);
}

inline u64 get_compress_ratio_value(void)
{
	return atomic64_read(&compress_ratio);
}

inline unsigned int get_avail_buffers_value(void)
{
	return atomic_read(&avail_buffers);
}

inline unsigned int get_min_avail_buffers_value(void)
{
	return atomic_read(&min_avail_buffers);
}

inline unsigned int get_high_avail_buffers_value(void)
{
	return atomic_read(&high_avail_buffers);
}

inline u64 get_zswapd_max_reclaim_size(void)
{
	return atomic_read(&max_reclaim_size);
}

inline u64 get_free_swap_threshold_value(void)
{
	return atomic64_read(&free_swap_threshold);
}

inline unsigned int get_inactive_file_ratio_value(void)
{
	return atomic_read(&inactive_file_ratio);
}

inline unsigned int get_active_file_ratio_value(void)
{
	return atomic_read(&active_file_ratio);
}

inline unsigned long long get_area_anon_refault_threshold_value(void)
{
	return atomic64_read(&area_anon_refault_threshold);
}

inline unsigned long get_anon_refault_snapshot_min_interval_value(void)
{
	return atomic64_read(&anon_refault_snapshot_min_interval);
}

inline unsigned long long get_empty_round_skip_interval_value(void)
{
	return atomic64_read(&empty_round_skip_interval);
}
inline unsigned long long get_max_skip_interval_value(void)
{
	return atomic64_read(&max_skip_interval);
}
inline unsigned long long get_empty_round_check_threshold_value(void)
{
	return atomic64_read(&empty_round_check_threshold);
}

inline u64 get_zram_critical_threshold_value(void)
{
	return atomic64_read(&zram_crit_thres);
}


static ssize_t avail_buffers_params_write(struct kernfs_open_file *of,
				char *buf, size_t nbytes, loff_t off)
{
	const unsigned int params_num = 4;
	unsigned int avail_buffers_value;
	unsigned int min_avail_buffers_value;
	unsigned int high_avail_buffers_value;
	u64 free_swap_threshold_value;

	buf = strstrip(buf);
	if (sscanf(buf, "%u %u %u %llu",
		&avail_buffers_value,
		&min_avail_buffers_value,
		&high_avail_buffers_value,
		&free_swap_threshold_value) != params_num)
		return -EINVAL;
	atomic_set(&avail_buffers, avail_buffers_value);
	atomic_set(&min_avail_buffers, min_avail_buffers_value);
	atomic_set(&high_avail_buffers, high_avail_buffers_value);
	atomic64_set(&free_swap_threshold,
		(free_swap_threshold_value * (SZ_1M / PAGE_SIZE)));

#ifndef CONFIG_REFAULT_IO_VMSCAN
	if (atomic_read(&min_avail_buffers) == 0)
		set_snapshotd_init_flag(0);
	else
		set_snapshotd_init_flag(1);
#endif

	wake_all_zswapd();

	return nbytes;
}

static ssize_t zswapd_max_reclaim_size_write(struct kernfs_open_file *of,
				char *buf, size_t nbytes, loff_t off)
{
	const unsigned int base = 10;
	u32 max_reclaim_size_value;
	int ret;

	buf = strstrip(buf);
	ret = kstrtouint(buf, base, &max_reclaim_size_value);
	if (ret)
		return -EINVAL;

	atomic_set(&max_reclaim_size, max_reclaim_size_value);

	return nbytes;
}

static ssize_t buffers_ratio_params_write(struct kernfs_open_file *of,
				char *buf, size_t nbytes, loff_t off)
{
	const unsigned int params_num = 2;
	unsigned int inactive_file_ratio_value;
	unsigned int active_file_ratio_value;

	buf = strstrip(buf);
	if (sscanf(buf, "%u %u",
		&inactive_file_ratio_value,
		&active_file_ratio_value) != params_num)
		return -EINVAL;

	if (inactive_file_ratio_value > MAX_RATIO ||
			active_file_ratio_value > MAX_RATIO)
		return -EINVAL;

	atomic_set(&inactive_file_ratio, inactive_file_ratio_value);
	atomic_set(&active_file_ratio, active_file_ratio_value);

	return nbytes;
}

static int area_anon_refault_threshold_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	atomic64_set(&area_anon_refault_threshold, val);

	return 0;
}

static int empty_round_skip_interval_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	atomic64_set(&empty_round_skip_interval, val);

	return 0;
}

static int max_skip_interval_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	atomic64_set(&max_skip_interval, val);

	return 0;
}

static int empty_round_check_threshold_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	atomic64_set(&empty_round_check_threshold, val);

	return 0;
}

static
int anon_refault_snapshot_min_interval_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	atomic64_set(&anon_refault_snapshot_min_interval, val);

	return 0;
}

static int zram_critical_thres_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	atomic64_set(&zram_crit_thres, val);

	return 0;
}

static ssize_t zswapd_pressure_event_control(struct kernfs_open_file *of,
				char *buf, size_t nbytes, loff_t off)
{
	const unsigned int params_num = 2;
	unsigned int efd;
	unsigned int level;
	struct fd efile;
	int ret;

	buf = strstrip(buf);
	if (sscanf(buf, "%u %u", &efd, &level) != params_num)
		return -EINVAL;

	if (level >= LEVEL_COUNT)
		return -EINVAL;

	mutex_lock(&pressure_event_lock);
	efile = fdget(efd);
	if (!efile.file) {
		ret = -EBADF;
		goto out;
	}
	zswapd_press_efd[level] = eventfd_ctx_fileget(efile.file);
	if (IS_ERR(zswapd_press_efd[level])) {
		ret = PTR_ERR(zswapd_press_efd[level]);
		goto out_put_efile;
	}
	fdput(efile);
	mutex_unlock(&pressure_event_lock);
	return nbytes;

out_put_efile:
	fdput(efile);
out:
	mutex_unlock(&pressure_event_lock);

	return ret;
}

void zswapd_pressure_report(enum zswapd_pressure_level level)
{
	int ret;

	if (zswapd_press_efd[level] == NULL)
		return;

	ret = eventfd_signal(zswapd_press_efd[level], 1);
	if (ret < 0)
		pr_err("SWAP-MM: %s : level:%u, ret:%d ", __func__, level, ret);
}

static u64 zswapd_pid_read(struct cgroup_subsys_state *css, struct cftype *cft)
{
	return get_zswapd_pid();
}

static void zswapd_memcgs_param_parse(int level_num)
{
	struct mem_cgroup *memcg = NULL;
	int i;

	while ((memcg = get_next_memcg(memcg))) {
		for (i = 0; i < level_num; ++i)
			if (atomic64_read(&memcg->memcg_reclaimed.app_score) >=
					zswap_param[i].min_score &&
			    atomic64_read(&memcg->memcg_reclaimed.app_score) <=
					zswap_param[i].max_score)
				break;

		atomic_set(&memcg->memcg_reclaimed.ub_mem2zram_ratio,
			zswap_param[i].ub_mem2zram_ratio);
		atomic_set(&memcg->memcg_reclaimed.ub_zram2ufs_ratio,
			zswap_param[i].ub_zram2ufs_ratio);
		atomic_set(&memcg->memcg_reclaimed.refault_threshold,
			zswap_param[i].refault_threshold);
	}
}

static ssize_t zswapd_memcgs_param_write(struct kernfs_open_file *of, char *buf,
				size_t nbytes, loff_t off)
{
	const char delim[] = " ";
	char *token = NULL;
	int level_num;
	int i;

	buf = strstrip(buf);
	token = strsep(&buf, delim);
	if (!token)
		return -EINVAL;

	if (kstrtoint(token, 0, &level_num))
		return -EINVAL;

	if (level_num > ZSWAPD_MAX_LEVEL_NUM)
		return -EINVAL;

	mutex_lock(&reclaim_para_lock);
	for (i = 0; i < level_num; ++i) {
		token = strsep(&buf, delim);
		if (!token)
			goto out;

		if (kstrtoint(token, 0, &zswap_param[i].min_score) ||
			zswap_param[i].min_score > MAX_APP_SCORE)
			goto out;

		token = strsep(&buf, delim);
		if (!token)
			goto out;

		if (kstrtoint(token, 0, &zswap_param[i].max_score) ||
			zswap_param[i].max_score > MAX_APP_SCORE)
			goto out;

		token = strsep(&buf, delim);
		if (!token)
			goto out;

		if (kstrtoint(token, 0, &zswap_param[i].ub_mem2zram_ratio) ||
			zswap_param[i].ub_mem2zram_ratio > MAX_RATIO)
			goto out;

		token = strsep(&buf, delim);
		if (!token)
			goto out;

		if (kstrtoint(token, 0, &zswap_param[i].ub_zram2ufs_ratio) ||
			zswap_param[i].ub_zram2ufs_ratio > MAX_RATIO)
			goto out;

		token = strsep(&buf, delim);
		if (!token)
			goto out;

		if (kstrtoint(token, 0, &zswap_param[i].refault_threshold))
			goto out;
	}

	zswapd_memcgs_param_parse(level_num);
	mutex_unlock(&reclaim_para_lock);

	return nbytes;

out:
	mutex_unlock(&reclaim_para_lock);
	return -EINVAL;
}

static ssize_t zswapd_single_memcg_param_write(struct kernfs_open_file *of,
				char *buf, size_t nbytes, loff_t off)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(of_css(of));
	unsigned int ub_mem2zram_ratio;
	unsigned int ub_zram2ufs_ratio;
	unsigned int refault_threshold;
	const unsigned int params_num = 3;

	buf = strstrip(buf);
	if (sscanf(buf, "%u %u %u",
		&ub_mem2zram_ratio,
		&ub_zram2ufs_ratio,
		&refault_threshold) != params_num)
		return -EINVAL;

	if (ub_mem2zram_ratio > MAX_RATIO || ub_zram2ufs_ratio > MAX_RATIO)
		return -EINVAL;

	atomic_set(&memcg->memcg_reclaimed.ub_mem2zram_ratio,
		ub_mem2zram_ratio);
	atomic_set(&memcg->memcg_reclaimed.ub_zram2ufs_ratio,
		ub_zram2ufs_ratio);
	atomic_set(&memcg->memcg_reclaimed.refault_threshold,
		refault_threshold);

	return nbytes;
}

static int mem_cgroup_zram_wm_ratio_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	if (val > MAX_RATIO)
		return -EINVAL;

	atomic64_set(&zram_wm_ratio, val);

	return 0;
}

static int mem_cgroup_compress_ratio_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	if (val > MAX_RATIO)
		return -EINVAL;

	atomic64_set(&compress_ratio, val);

	return 0;
}

static int zswapd_presure_show(struct seq_file *m, void *v)
{
	zswapd_status_show(m);

	return 0;
}

static int memcg_active_app_info_list_show(struct seq_file *m, void *v)
{
	struct mem_cgroup *memcg = NULL;
	struct mem_cgroup_per_node *mz = NULL;
	struct lruvec *lruvec = NULL;
	unsigned long anon_size;
	unsigned long zram_size;
	unsigned long eswap_size;

	while ((memcg = get_next_memcg(memcg))) {
		u64 score = atomic64_read(&memcg->memcg_reclaimed.app_score);

		mz = mem_cgroup_nodeinfo(memcg, 0);
		if (!mz) {
			get_next_memcg_break(memcg);
			return 0;
		}

		lruvec = &mz->lruvec;
		if (!lruvec) {
			get_next_memcg_break(memcg);
			return 0;
		}

		anon_size = lruvec_lru_size(lruvec, LRU_ACTIVE_ANON,
				MAX_NR_ZONES) + lruvec_lru_size(lruvec,
					LRU_INACTIVE_ANON, MAX_NR_ZONES);
		eswap_size = hyperhold_read_mcg_stats(memcg,
				MCG_DISK_STORED_PG_SZ);
		zram_size = hyperhold_read_mcg_stats(memcg,
				MCG_ZRAM_PG_SZ);
		if (anon_size + zram_size + eswap_size == 0)
			continue;

		if (!strlen(memcg->name))
			continue;

		anon_size *= PAGE_SIZE / SZ_1K;
		zram_size *= PAGE_SIZE / SZ_1K;
		eswap_size *= PAGE_SIZE / SZ_1K;

		seq_printf(m, "%s %llu %lu %lu %lu %llu\n", memcg->name, score,
			anon_size, zram_size, eswap_size,
			memcg->memcg_reclaimed.reclaimed_pagefault);
	}
	return 0;
}

static int report_app_info_show(struct seq_file *m, void *v)
{
	struct mem_cgroup *memcg = NULL;
	struct mem_cgroup_per_node *mz = NULL;
	struct lruvec *lruvec = NULL;
	unsigned long anon_size;
	unsigned long zram_size;
	unsigned long eswap_size;
	unsigned long swap_out_size;
	unsigned long swap_in_size;

	while ((memcg = get_next_memcg(memcg))) {
		u64 score = atomic64_read(&memcg->memcg_reclaimed.app_score);

		mz = mem_cgroup_nodeinfo(memcg, 0);
		if (!mz) {
			get_next_memcg_break(memcg);
			return 0;
		}

		lruvec = &mz->lruvec;
		if (!lruvec) {
			get_next_memcg_break(memcg);
			return 0;
		}

		anon_size = lruvec_lru_size(lruvec, LRU_ACTIVE_ANON,
				MAX_NR_ZONES) + lruvec_lru_size(lruvec,
				LRU_INACTIVE_ANON, MAX_NR_ZONES);
		eswap_size = hyperhold_read_mcg_stats(memcg,
				MCG_DISK_STORED_PG_SZ);
		zram_size = hyperhold_read_mcg_stats(memcg,
				MCG_ZRAM_PG_SZ);
		swap_out_size = hyperhold_read_mcg_stats(memcg, MCG_SWAPOUT_SZ);
		swap_in_size = hyperhold_read_mcg_stats(memcg, MCG_SWAPIN_SZ);

		if (anon_size + zram_size + eswap_size == 0)
			continue;

		anon_size *= PAGE_SIZE / SZ_1K;
		zram_size *= PAGE_SIZE / SZ_1K;
		eswap_size *= PAGE_SIZE / SZ_1K;

		seq_printf(m, "%s,%llu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",
			strlen(memcg->name) ? memcg->name : "root", score,
			anon_size, zram_size, eswap_size,
			atomic64_read(&memcg->zram_swapout_cnt),
			atomic64_read(&memcg->zram_swapin_cnt),
			swap_out_size, swap_in_size);
	}
	return 0;
}
#endif

#ifdef CONFIG_HYPERHOLD_DEBUG
static int memcg_name_show(struct seq_file *m, void *v)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(seq_css(m));

	seq_printf(m, "%s\n", memcg->name);

	return 0;
}

static void mem_health_meminfo_show(struct seq_file *m)
{
	long available;
	unsigned long pages[NR_LRU_LISTS];
	int lru;
	pg_data_t *pgdat = NULL;
	int nid;
	unsigned long file = 0;

	for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
		pages[lru] = global_node_page_state(NR_LRU_BASE + lru);

	available = si_mem_available();

	for_each_online_node(nid) {
		struct lruvec *lruvec;

		pgdat = NODE_DATA(nid);
		lruvec = node_lruvec(pgdat);
		file += lruvec_lru_size(lruvec, LRU_ACTIVE_FILE,
				MAX_NR_ZONES) + lruvec_lru_size(lruvec,
					LRU_INACTIVE_FILE, MAX_NR_ZONES);
#ifdef CONFIG_REFAULT_IO_VMSCAN
		seq_printf(m, "Node:%d anon refault cost=%lu\n", nid,
			   lruvec_page_state(lruvec, WORKINGSET_ANON_COST));
		seq_printf(m, "Node:%d file refault cost=%lu\n", nid,
			   lruvec_page_state(lruvec, WORKINGSET_FILE_COST));
#endif
	}

	seq_printf(m, "MemAvailable:%lu\n", available * PAGE_SIZE / SZ_1K);
	seq_printf(m, "MemFree:%lu\n",
		global_zone_page_state(NR_FREE_PAGES) * PAGE_SIZE / SZ_1K);
	seq_printf(m, "Inactive_file:%lu\n",
		pages[LRU_INACTIVE_FILE] * PAGE_SIZE / SZ_1K);
	seq_printf(m, "active_file:%lu\n",
		pages[LRU_ACTIVE_FILE] * PAGE_SIZE / SZ_1K);
	seq_printf(m, "Inactive_anon:%lu\n",
		pages[LRU_INACTIVE_ANON] * PAGE_SIZE / SZ_1K);
	seq_printf(m, "active_anon:%lu\n",
		pages[LRU_ACTIVE_ANON] * PAGE_SIZE / SZ_1K);
	seq_printf(m, "FileLru:%lu\n",
		file * PAGE_SIZE / SZ_1K);
#ifdef CONFIG_MEMCG_PROTECT_LRU
	seq_printf(m, "ProtectLru:%lu\n",
		get_protected_pages() * PAGE_SIZE / SZ_1K);
#endif
#ifdef CONFIG_HYPERHOLD_ZSWAPD
	zswapd_status_show(m);
#endif

}

static void mem_health_vmstat_show(struct seq_file *m)
{
#ifdef CONFIG_VM_EVENT_COUNTERS
	unsigned long *vm_buf = NULL;
	const int pgparam = 2;
	unsigned long allocstall = 0;

	vm_buf = kzalloc(sizeof(struct vm_event_state), GFP_KERNEL);
	if (!vm_buf)
		return;
	all_vm_events(vm_buf);

#ifdef CONFIG_ZONE_DMA
	allocstall += vm_buf[ALLOCSTALL_DMA];
#endif
	allocstall += vm_buf[ALLOCSTALL_MOVABLE];
	allocstall += vm_buf[ALLOCSTALL_NORMAL];

	seq_printf(m, "Pgpgin:%lu\n", vm_buf[PGPGIN] / pgparam);
	seq_printf(m, "Pgpgout:%lu\n", vm_buf[PGPGOUT] / pgparam);
	seq_printf(m, "Pswpin:%lu\n", vm_buf[PSWPIN]);
	seq_printf(m, "Pswpout:%lu\n", vm_buf[PSWPOUT]);
	seq_printf(m, "zswapd_wake_up:%lu\n", vm_buf[ZSWAPD_WAKEUP]);
	seq_printf(m, "zswapd_area_refault:%lu\n", vm_buf[ZSWAPD_REFAULT]);
	seq_printf(m, "zswapd_medium_press:%lu\n", vm_buf[ZSWAPD_MEDIUM_PRESS]);
	seq_printf(m, "zswapd_critical_press:%lu\n",
		vm_buf[ZSWAPD_CRITICAL_PRESS]);
	seq_printf(m, "zswapd_memcg_ratio_skip:%lu\n",
		vm_buf[ZSWAPD_MEMCG_RATIO_SKIP]);
	seq_printf(m, "zswapd_memcg_refault_skip:%lu\n",
		vm_buf[ZSWAPD_MEMCG_REFAULT_SKIP]);
	seq_printf(m, "zswapd_swapout:%lu\n", vm_buf[ZSWAPD_SWAPOUT]);
	seq_printf(m, "zswapd_snapshot_times:%lu\n",
		vm_buf[ZSWAPD_SNAPSHOT_TIMES]);
	seq_printf(m, "zswapd_reclaimed:%lu\n", vm_buf[ZSWAPD_RECLAIMED]);
	seq_printf(m, "zswapd_scanned:%lu\n", vm_buf[ZSWAPD_SCANNED]);
	seq_printf(m, "kswapd_wake_up:%lu\n", vm_buf[PAGEOUTRUN]);
	seq_printf(m, "kswapd_rec_anon:%lu\n", vm_buf[KSWAPD_RECLAIMED_ANON]);
	seq_printf(m, "kswapd_rec_file:%lu\n", vm_buf[KSWAPD_RECLAIMED_FILE]);
	seq_printf(m, "kswapd_scanned_anon:%lu\n", vm_buf[KSWAPD_SCAN_ANON]);
	seq_printf(m, "kswapd_scanned_file:%lu\n", vm_buf[KSWAPD_SCAN_FILE]);
	seq_printf(m, "dr_wake_up:%lu\n", allocstall);
	seq_printf(m, "dr_reclaimed_anon:%lu\n", vm_buf[DR_RECLAIMED_ANON]);
	seq_printf(m, "dr_reclaimed_file:%lu\n", vm_buf[DR_RECLAIMED_FILE]);
	seq_printf(m, "dr_scanned_anon:%lu\n", vm_buf[DR_SCAN_ANON]);
	seq_printf(m, "dr_scanned_file:%lu\n", vm_buf[DR_SCAN_FILE]);
	seq_printf(m, "freeze_wake_up:%lu\n", vm_buf[FREEZE_RECLAIM_TIMES]);
	seq_printf(m, "freeze_reclaimed:%lu\n", vm_buf[FREEZE_RECLAIMED]);

	kfree(vm_buf);
#endif
}

#ifdef CONFIG_REFAULT_IO_VMSCAN
static void mem_health_tree_stat_show(struct seq_file *m)
{
#ifdef CONFIG_HYPERHOLD_FILE_LRU
	seq_printf(m, "Workingset_refault_anon:%lu\n",
		global_node_page_state(WORKINGSET_REFAULT_ANON));
	seq_printf(m, "Workingset_refault_file:%lu\n",
		global_node_page_state(WORKINGSET_REFAULT_FILE));
#else
	unsigned long stat[MEMCG_NR_STAT];

	tree_stat(NULL, stat);
	seq_printf(m, "Workingset_refault_anon:%lu\n", stat[WORKINGSET_REFAULT_ANON]);
	seq_printf(m, "Workingset_refault:%lu\n", stat[WORKINGSET_REFAULT_FILE]);
#endif
}
#else
static void mem_health_tree_stat_show(struct seq_file *m)
{
#ifdef CONFIG_HYPERHOLD_FILE_LRU
	seq_printf(m, "Workingset_refault:%lu\n",
		global_node_page_state(WORKINGSET_REFAULT));
#else
	unsigned long stat[MEMCG_NR_STAT];

	tree_stat(NULL, stat);
	seq_printf(m, "Workingset_refault:%lu\n", stat[WORKINGSET_REFAULT]);
#endif
}
#endif

static void buddy_info_show(struct seq_file *m)
{
	pg_data_t *pgdat = NULL;
	struct zone *zone = NULL;
	int nid;
	int i;
	int order;

	for_each_online_node(nid) {
		pgdat = NODE_DATA(nid);
		for (i = 0; i < MAX_NR_ZONES; i++) {
			zone = pgdat->node_zones + i;
			if (!populated_zone(zone))
				continue;
			for (order = 0; order < MAX_ORDER; ++order) {
				seq_printf(m, "N%d_%s_order%d:%lu",
					pgdat->node_id, zone->name, order,
					zone->free_area[order].nr_free);
				seq_putc(m, '\n');
			}
		}
	}
}

static int psi_health_info_show(struct seq_file *m, void *v)
{
	/* mem info */
	mem_health_meminfo_show(m);
	mem_health_vmstat_show(m);
	mem_health_tree_stat_show(m);
	buddy_info_show(m);
#ifdef CONFIG_HP_CORE
	hyperhold_psi_show(m);
#endif
	return 0;
}

static u64 mem_cgroup_ub_ufs2zram_ratio_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(css);

	return atomic64_read(&memcg->memcg_reclaimed.ub_ufs2zram_ratio);
}

static u64 mem_cgroup_ec_app_start_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return atomic_read(&ec_app_start_flag);
}

#ifdef CONFIG_HYPERHOLD_ZSWAPD
static int avail_buffers_params_show(struct seq_file *m, void *v)
{
	seq_printf(m, "avail_buffers: %u\n",
		atomic_read(&avail_buffers));
	seq_printf(m, "min_avail_buffers: %u\n",
		atomic_read(&min_avail_buffers));
	seq_printf(m, "high_avail_buffers: %u\n",
		atomic_read(&high_avail_buffers));
	seq_printf(m, "free_swap_threshold: %llu\n",
		(atomic64_read(&free_swap_threshold) * PAGE_SIZE / SZ_1M));

	return 0;
}

static int zswapd_max_reclaim_size_show(struct seq_file *m, void *v)
{
	seq_printf(m, "zswapd_max_reclaim_size: %u\n",
		atomic_read(&max_reclaim_size));

	return 0;
}

static int buffers_ratio_params_show(struct seq_file *m, void *v)
{
	seq_printf(m, "inactive_file_ratio: %u\n",
		atomic_read(&inactive_file_ratio));
	seq_printf(m, "active_file_ratio: %u\n",
		atomic_read(&active_file_ratio));

	return 0;
}

static u64 area_anon_refault_threshold_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return atomic64_read(&area_anon_refault_threshold);
}

static u64 empty_round_skip_interval_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return atomic64_read(&empty_round_skip_interval);
}

static u64 max_skip_interval_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return atomic64_read(&max_skip_interval);
}

static u64 empty_round_check_threshold_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return atomic64_read(&empty_round_check_threshold);
}

static u64 anon_refault_snapshot_min_interval_read(
				struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return atomic64_read(&anon_refault_snapshot_min_interval);
}

static u64 zram_critical_thres_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return atomic64_read(&zram_crit_thres);
}


static int zswapd_memcgs_param_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < ZSWAPD_MAX_LEVEL_NUM; ++i) {
		seq_printf(m, "level %d min score: %u\n",
				i, zswap_param[i].min_score);
		seq_printf(m, "level %d max score: %u\n",
				i, zswap_param[i].max_score);
		seq_printf(m, "level %d ub_mem2zram_ratio: %u\n",
				i, zswap_param[i].ub_mem2zram_ratio);
		seq_printf(m, "level %d ub_zram2ufs_ratio: %u\n",
				i, zswap_param[i].ub_zram2ufs_ratio);
		seq_printf(m, "memcg %d refault_threshold: %u\n",
				i, zswap_param[i].refault_threshold);
	}

	return 0;
}

static int zswapd_single_memcg_param_show(struct seq_file *m, void *v)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(seq_css(m));

	seq_printf(m, "memcg score: %lu\n",
		atomic64_read(&memcg->memcg_reclaimed.app_score));
	seq_printf(m, "memcg ub_mem2zram_ratio: %u\n",
		atomic_read(&memcg->memcg_reclaimed.ub_mem2zram_ratio));
	seq_printf(m, "memcg ub_zram2ufs_ratio: %u\n",
		atomic_read(&memcg->memcg_reclaimed.ub_zram2ufs_ratio));
	seq_printf(m, "memcg refault_threshold: %u\n",
		atomic_read(&memcg->memcg_reclaimed.refault_threshold));

	return 0;
}

static u64 mem_cgroup_zram_wm_ratio_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return atomic64_read(&zram_wm_ratio);
}

static u64 mem_cgroup_compress_ratio_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return atomic64_read(&compress_ratio);
}
#endif

static int memcg_score_list_show(struct seq_file *m, void *v)
{
	struct mem_cgroup *memcg = NULL;

	while ((memcg = get_next_memcg(memcg)))
		seq_printf(m, "%lu %s\n",
			atomic64_read(&memcg->memcg_reclaimed.app_score),
				memcg->name);

	return 0;
}

static u64 mem_cgroup_app_score_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(css);

	return atomic64_read(&memcg->memcg_reclaimed.app_score);
}

void memcg_eswap_info_show(struct seq_file *m)
{
	struct mem_cgroup *memcg = mem_cgroup_from_css(seq_css(m));
	struct mem_cgroup_per_node *mz = NULL;
	struct lruvec *lruvec = NULL;
	unsigned long anon;
	unsigned long file;
	unsigned long zram;
	unsigned long eswap;

	mz = mem_cgroup_nodeinfo(memcg, 0);
	if (!mz)
		return;

	lruvec = &mz->lruvec;
	if (!lruvec)
		return;

	anon = lruvec_lru_size(lruvec, LRU_ACTIVE_ANON, MAX_NR_ZONES) +
		lruvec_lru_size(lruvec, LRU_INACTIVE_ANON, MAX_NR_ZONES);
	file = lruvec_lru_size(lruvec, LRU_ACTIVE_FILE, MAX_NR_ZONES) +
		lruvec_lru_size(lruvec, LRU_INACTIVE_FILE, MAX_NR_ZONES);
	zram = hyperhold_read_mcg_stats(memcg, MCG_ZRAM_PG_SZ);
	eswap = hyperhold_read_mcg_stats(memcg, MCG_DISK_STORED_PG_SZ);
	anon *= PAGE_SIZE / SZ_1K;
	file *= PAGE_SIZE / SZ_1K;
	zram *= PAGE_SIZE / SZ_1K;
	eswap *= PAGE_SIZE / SZ_1K;
	seq_printf(m,
		"Anon:\t%12lu kB\n"
		"File:\t%12lu kB\n"
		"Zram:\t%8lu kB\n"
		"Eswap:\t%8lu kB\n",
		anon,
		file,
		zram,
		eswap);
}
#endif

static struct cftype memcg_policy_files[] = {
	{
		.name = "force_shrink_anon",
		.write = mem_cgroup_force_shrink_anon,
	},
	{
		.name = "total_info_per_app",
		.seq_show = memcg_total_info_per_app_show,
	},
	{
		.name = "eswap_stat",
		.seq_show = memcg_eswap_stat_show,
	},
	{
		.name = "name",
		.write = mem_cgroup_name_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.seq_show = memcg_name_show,
#endif
	},
	{
		.name = "app_score",
		.write_u64 = mem_cgroup_app_score_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = mem_cgroup_app_score_read,
#endif
	},
	{
		.name = "ub_ufs2zram_ratio",
		.write_u64 = mem_cgroup_ub_ufs2zram_ratio_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = mem_cgroup_ub_ufs2zram_ratio_read,
#endif
	},
	{
		.name = "ec_app_start",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write_u64 = mem_cgroup_ec_app_start_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = mem_cgroup_ec_app_start_read,
#endif
	},
	{
		.name = "force_swapin",
		.write_u64 = mem_cgroup_force_swapin_write,
	},
	{
		.name = "force_swapout",
		.write_u64 = mem_cgroup_force_swapout_write,
	},
#ifdef CONFIG_HYPERHOLD_ZSWAPD
	{
		.name = "active_app_info_list",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = memcg_active_app_info_list_show,
	},
	{
		.name = "zram_wm_ratio",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write_u64 = mem_cgroup_zram_wm_ratio_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = mem_cgroup_zram_wm_ratio_read,
#endif
	},
	{
		.name = "compress_ratio",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write_u64 = mem_cgroup_compress_ratio_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = mem_cgroup_compress_ratio_read,
#endif
	},
	{
		.name = "zswapd_pressure",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write = zswapd_pressure_event_control,
	},
	{
		.name = "zswapd_pid",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.read_u64 = zswapd_pid_read,
	},
	{
		.name = "avail_buffers",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write = avail_buffers_params_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.seq_show = avail_buffers_params_show,
#endif
	},
	{
		.name = "zswapd_max_reclaim_size",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write = zswapd_max_reclaim_size_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.seq_show = zswapd_max_reclaim_size_show,
#endif
	},
	{
		.name = "area_anon_refault_threshold",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write_u64 = area_anon_refault_threshold_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = area_anon_refault_threshold_read,
#endif
	},
	{
		.name = "empty_round_skip_interval",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write_u64 = empty_round_skip_interval_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = empty_round_skip_interval_read,
#endif
	},
	{
		.name = "max_skip_interval",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write_u64 = max_skip_interval_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = max_skip_interval_read,
#endif
	},
	{
		.name = "empty_round_check_threshold",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write_u64 = empty_round_check_threshold_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = empty_round_check_threshold_read,
#endif
	},
	{
		.name = "anon_refault_snapshot_min_interval",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write_u64 = anon_refault_snapshot_min_interval_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = anon_refault_snapshot_min_interval_read,
#endif
	},
	{
		.name = "zswapd_memcgs_param",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write = zswapd_memcgs_param_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.seq_show = zswapd_memcgs_param_show,
#endif
	},
	{
		.name = "zswapd_single_memcg_param",
		.write = zswapd_single_memcg_param_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.seq_show = zswapd_single_memcg_param_show,
#endif
	},
	{
		.name = "buffer_ratio_params",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write = buffers_ratio_params_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.seq_show = buffers_ratio_params_show,
#endif
	},
	{
		.name = "zswapd_presure_show",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = zswapd_presure_show,
	},
	{
		.name = "zram_critical_threshold",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write_u64 = zram_critical_thres_write,
#ifdef CONFIG_HYPERHOLD_DEBUG
		.read_u64 = zram_critical_thres_read,
#endif
	},
#endif
#ifdef CONFIG_HYPERHOLD_DEBUG
	{
		.name = "score_list",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = memcg_score_list_show,
	},
	{
		.name = "psi_health_info",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = psi_health_info_show,
	},
	{
		.name = "report_app_info",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = report_app_info_show,
	},
#endif
	{ }, /* terminate */
};

static int __init memcg_policy_init(void)
{
	/*lint -e548 */
	if (!mem_cgroup_disabled())
		WARN_ON(cgroup_add_legacy_cftypes(&memory_cgrp_subsys,
						memcg_policy_files));
	/*lint +e548 */

	return 0;
}
subsys_initcall(memcg_policy_init);
