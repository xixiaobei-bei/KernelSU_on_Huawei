/*
 * hwcfs_common.h
 *
 * hwcfs schedule declaration
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#ifndef _HWCFS_COMMON_H_
#define _HWCFS_COMMON_H_

#include <linux/sched.h>

struct rq;

void enqueue_vip_thread(struct rq *rq, struct task_struct *p);
void dequeue_vip_thread(struct rq *rq, struct task_struct *p);
void pick_vip_thread(struct rq *rq, struct task_struct **p,
	struct sched_entity **se);
bool test_dynamic_vip(struct task_struct *task, int type);
void dynamic_vip_dequeue(struct task_struct *task, int type);
void dynamic_vip_enqueue(struct task_struct *task, int type, int depth);
bool test_task_vip(struct task_struct *task);
bool test_task_vip_depth(int vip_depth);
bool test_set_dynamic_vip(struct task_struct *tsk);
void trigger_vip_balance(struct rq *rq);
void vip_init_rq_data(struct rq *rq);
#ifdef CONFIG_VIP_SCHED_OPT
extern int find_vip_cpu(struct task_struct *p);
extern void set_thread_vip_prio(struct task_struct *p, unsigned int prio);
extern unsigned int select_allowed_cpu(struct task_struct *p,
	struct cpumask *allowed_cpus);
extern enum vip_preempt_type vip_preempt_classify(struct rq *rq,
	struct task_struct *prev, struct task_struct *next);
extern void fixup_vip_preempt_type(struct rq *rq);
#ifndef CONFIG_HW_EAS_SCHED
struct task_struct *get_heaviest_vip_task(struct sched_entity *se,
	struct task_struct *p, int cpu);
#endif
#endif
#endif
