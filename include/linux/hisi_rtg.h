/*
 * hisi_rtg.h
 *
 * related thread grop.
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __HISI_RTG_H
#define __HISI_RTG_H

#include <linux/perf_ctrl.h>

#ifdef CONFIG_SCHED_RTG
enum rtg_freq_update_flags {
	FRAME_FORCE_UPDATE = (1 << 0),
	FRAME_NORMAL_UPDATE = (1 << 1),
	AI_FORCE_UPDATE = (1 << 2),
};

int sched_set_group_id(pid_t pid, unsigned int group_id);
struct related_thread_group *lookup_related_thread_group(unsigned int group_id);
unsigned int get_cluster_grp_running(int cluster_id);
int sched_set_preferred_cluster(unsigned int grp_id, int sched_cluster_id);
int sched_set_group_normalized_util(unsigned int grp_id, unsigned long util,
				    unsigned int flag);
int sched_set_group_window_size(unsigned int grp_id, unsigned int rate);
int sched_set_group_window_rollover(unsigned int grp_id);
int sched_set_group_freq(unsigned int grp_id, unsigned int freq);
int sched_set_group_freq_update_interval(unsigned int grp_id,
					 unsigned int interval);
int sched_set_group_util_invalid_interval(unsigned int grp_id,
					  unsigned int interval);
int sched_set_group_load_mode(struct rtg_load_mode *mode);
int sched_set_group_ed_params(struct rtg_ed_params *params);
unsigned int sched_get_group_id(struct task_struct *p);

#else /* CONFIG_SCHED_RTG */

static inline struct related_thread_group*
	lookup_related_thread_group(unsigned int group_id)
{
	return NULL;
}

static inline unsigned int get_cluster_grp_running(int cluster_id)
{
	return 0;
}

static inline int sched_set_group_id(pid_t pid, unsigned int group_id)
{
	return 0;
}

static inline int sched_set_preferred_cluster(unsigned int grp_id,
					      int sched_cluster_id)
{
	return 0;
}

static inline int sched_set_group_normalized_util(unsigned int grp_id,
						  unsigned long util,
						  unsigned int flag)
{
	return 0;
}

static inline int sched_set_group_window_size(unsigned int grp_id,
					      unsigned int rate)
{
	return 0;
}

static inline int sched_set_group_window_rollover(unsigned int grp_id)
{
	return 0;
}

static inline int sched_set_group_freq(unsigned int grp_id, unsigned int freq)
{
	return 0;
}

static inline int sched_set_group_freq_update_interval(unsigned int grp_id,
						       unsigned int interval)
{
	return 0;
}

static inline int sched_set_group_util_invalid_interval(unsigned int grp_id,
							unsigned int interval)
{
	return 0;
}

static inline int sched_set_group_load_mode(struct rtg_load_mode *mode)
{
	return 0;
}

static inline int sched_set_group_ed_params(struct rtg_ed_params *params)
{
	return 0;
}

static inline unsigned int sched_get_group_id(struct task_struct *p)
{
	return 0;
}

#endif

#endif
