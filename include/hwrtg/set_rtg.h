/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
 * set rtg sched header
 */

#ifndef SET_RTG_H
#define SET_RTG_H

#include <linux/sched.h>
#include <linux/types.h>

#define STATIC_RTG_DEPTH (-1)
#define INVALID_VALUE 0xffff
#define DEFAULT_RT_PRIO 97
#define NOT_RT_PRIO (-1)
#define RTG_MAX_RT_THREAD_NUM CONFIG_NR_CPUS

int set_rtg_sched(struct task_struct *task, bool is_rtg, int grp_id, int prio, bool is_trans);
int set_rtg_grp(pid_t tid, bool is_rtg, int grp_id, int prio, int task_util);
void set_frame_rtg_thread(int grp_id, struct task_struct *task, bool is_rtg, int prio);
void set_aux_task_min_util(struct task_struct *task, int min_util);
void reset_aux_task_min_util(struct task_struct *task);
bool is_rtg_rt_task(struct task_struct *task);
unsigned int get_rtg_rt_thread_num(void);
#ifdef CONFIG_HW_RTG_SCHED_RT_THREAD_LIMIT
void inc_rtg_rt_thread_num(void);
void dec_rtg_rt_thread_num(void);
int test_and_read_rtg_rt_thread_num(void);
int read_rtg_rt_thread_num(void);
#else
static inline void inc_rtg_rt_thread_num(void)
{
}
static inline void dec_rtg_rt_thread_num(void)
{
}
static inline int test_and_read_rtg_rt_thread_num(void)
{
	return 0;
}
static inline int read_rtg_rt_thread_num(void)
{
	return 0;
}
#endif
#endif
