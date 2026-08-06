/*
 * hw_pi.h
 *
 * Description: provide external call interfaces of hw_pi.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#ifndef HW_PI_H_INCLUDED
#define HW_PI_H_INCLUDED
#include <linux/sched.h>
#include <linux/sched/deadline.h>
#include <linux/sched/rt.h>
#include <linux/cgroup.h>
#include <linux/futex.h>
#include <../../../kernel/locking/rtmutex_common.h>
#ifdef CONFIG_HW_QOS_THREAD
#include <chipset_common/hwqos/hwqos_common.h>
#endif

#ifdef CONFIG_HUAWEI_SCHED_VIP
#define MIN_PREEMPT_VIP_PRIO	(10U)
#define MIN_INHERIT_VIP_PRIO	(10U)
#endif
#ifdef CONFIG_HW_QOS_THREAD
#define MIN_PREEMPT_QOS		(VALUE_QOS_CRITICAL)
#define MIN_INHERIT_QOS		(VALUE_QOS_CRITICAL)
#endif

#define SUPPORT_FUTEX_PI	(0x1)
#define USER_SPACE_ENABLE	(0x2)
#define FUTEX_PI_ENABLE		(SUPPORT_FUTEX_PI | USER_SPACE_ENABLE)
extern unsigned int g_hw_futex_pi_enabled;
static inline int is_hw_futex_pi_enabled(void)
{
	return (g_hw_futex_pi_enabled & FUTEX_PI_ENABLE) == FUTEX_PI_ENABLE;
}

static inline int can_all_pi(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (left && right && !left->major_only && !right->major_only);
}

#ifdef CONFIG_HUAWEI_SCHED_VIP
static inline bool vip_prio_equal(unsigned int left, unsigned int right)
{
	return (left == right) || ((left < MIN_INHERIT_VIP_PRIO) &&
	    (right < MIN_INHERIT_VIP_PRIO));
}

static inline bool prio_equal(struct task_struct *p, int prio,
	unsigned int vip_prio, struct rt_mutex_waiter *waiter)
{
	return likely(p) && ((!waiter || waiter->major_only) ?
	    vip_prio_equal(vip_prio, p->vip_prio) :
	    ((vip_prio == p->vip_prio) && (prio == p->prio)));
}
#else
static inline bool prio_equal(struct task_struct *p, int prio,
	unsigned int vip_prio, struct rt_mutex_waiter *waiter)
{
	return !waiter || waiter->major_only ||
	    (likely(p) && (prio == p->prio));
}
#endif

static inline bool mix_prio_equal(struct task_struct *p, int prio,
	unsigned int vip_prio, struct rt_mutex_waiter *w)
{
	int rt = rt_prio(prio);

	return likely(p) && ((rt && (prio == p->prio) && !dl_prio(prio)) ||
	    (!rt && !rt_prio(p->prio) && prio_equal(p, prio, vip_prio, w)));
}

static inline bool rt_mutex_mix_prio_equal(struct task_struct *p, int prio,
	unsigned int vip_prio, struct rt_mutex_waiter *waiter)
{
	return likely(p) && (likely(is_hw_futex_pi_enabled()) ?
	    mix_prio_equal(p, prio, vip_prio, waiter) :
	    (prio == p->prio && !dl_prio(prio)));
}

#ifdef CONFIG_HW_QOS_THREAD
static inline int get_preempt_qos(struct task_struct *p)
{
	int qos = likely(p) ? get_task_qos(p) : 0;

	return (qos < MIN_PREEMPT_QOS) ? 0 : qos;
}

static inline void set_preempt_qos(struct rt_mutex_waiter *waiter, int qos)
{
	if (likely(waiter))
		waiter->qos = qos;
}
#endif

#ifdef CONFIG_HUAWEI_SCHED_VIP
static inline unsigned int get_preempt_vip_prio(struct task_struct *p)
{
	return (likely(p) && (p->vip_prio >= MIN_PREEMPT_VIP_PRIO)) ?
	    p->vip_prio : 0;
}

static inline void set_preempt_vip_prio(struct rt_mutex_waiter *waiter,
	unsigned int vip_prio)
{
	if (likely(waiter))
		waiter->vip_prio = vip_prio;
}
#endif

static inline void set_preempt_policy(struct rt_mutex_waiter *waiter,
	struct task_struct *p, bool set_policy)
{
	if (likely(waiter) && set_policy)
		waiter->major_only = likely(p) ? (IS_ENABLED(CONFIG_CPUSETS) &&
		    !task_css_is_root(p, cpuset_cgrp_id)) : 0;
}

#if defined(CONFIG_HW_QOS_THREAD) && defined(CONFIG_HUAWEI_SCHED_VIP)
#define task_to_waiter(p) &(struct rt_mutex_waiter) { \
	.qos = get_preempt_qos(p), .vip_prio = get_preempt_vip_prio(p), \
	.prio = (p)->prio, .deadline = (p)->dl.deadline }

static inline void rt_mutex_waiter_fill_additional_infos(
	struct rt_mutex_waiter *waiter, struct task_struct *p, bool set_policy)
{
	set_preempt_qos(waiter, get_preempt_qos(p));
	set_preempt_vip_prio(waiter, get_preempt_vip_prio(p));
	set_preempt_policy(waiter, p, set_policy);
}

static inline int waiter_less(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (likely(left) && likely(right) && ((left->qos > right->qos) ||
	    (left->vip_prio > right->vip_prio) ||
	    (can_all_pi(left, right) && (left->prio < right->prio)))) ? 1 : 0;
}

static inline int waiter_equal(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (unlikely(!left) || unlikely(!right) ||
	    (left->qos != right->qos) || (left->vip_prio != right->vip_prio) ||
	    (can_all_pi(left, right) && (left->prio != right->prio))) ? 0 : 1;
}
#elif defined(CONFIG_HW_QOS_THREAD)
#define task_to_waiter(p) &(struct rt_mutex_waiter) { \
	.qos = get_preempt_qos(p), \
	.prio = (p)->prio, .deadline = (p)->dl.deadline }

static inline void rt_mutex_waiter_fill_additional_infos(
	struct rt_mutex_waiter *waiter, struct task_struct *p, bool set_policy)
{
	set_preempt_qos(waiter, get_preempt_qos(p));
	set_preempt_policy(waiter, p, set_policy);
}

static inline int waiter_less(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (likely(left) && likely(right) && ((left->qos > right->qos) ||
	    (can_all_pi(left, right) && (left->prio < right->prio)))) ? 1 : 0;
}

static inline int waiter_equal(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (unlikely(!left) || unlikely(!right) ||
	    (left->qos != right->qos) ||
	    (can_all_pi(left, right) && (left->prio != right->prio))) ? 0 : 1;
}
#elif defined(CONFIG_HUAWEI_SCHED_VIP)
#define task_to_waiter(p) &(struct rt_mutex_waiter) { \
	.vip_prio = get_preempt_vip_prio(p), \
	.prio = (p)->prio, .deadline = (p)->dl.deadline }

static inline void rt_mutex_waiter_fill_additional_infos(
	struct rt_mutex_waiter *waiter, struct task_struct *p, bool set_policy)
{
	set_preempt_vip_prio(waiter, get_preempt_vip_prio(p));
	set_preempt_policy(waiter, p, set_policy);
}

static inline int waiter_less(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (likely(left) && likely(right) &&
	    ((left->vip_prio > right->vip_prio) ||
	    (can_all_pi(left, right) && (left->prio < right->prio)))) ? 1 : 0;
}

static inline int waiter_equal(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (unlikely(!left) || unlikely(!right) ||
	    (left->vip_prio != right->vip_prio) ||
	    (can_all_pi(left, right) && (left->prio != right->prio))) ? 0 : 1;
}
#else
#define task_to_waiter(p) &(struct rt_mutex_waiter) { \
	.prio = (p)->prio, .deadline = (p)->dl.deadline }

static inline void rt_mutex_waiter_fill_additional_infos(
	struct rt_mutex_waiter *waiter, struct task_struct *p, bool set_policy)
{
	set_preempt_policy(waiter, p, set_policy);
}

static inline int waiter_less(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (likely(left) && likely(right) &&
	    (can_all_pi(left, right) && (left->prio < right->prio))) ? 1 : 0;
}

static inline int waiter_equal(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (unlikely(!left) || unlikely(!right) ||
	    (can_all_pi(left, right) && (left->prio != right->prio))) ? 0 : 1;
}
#endif

static inline int hw_rt_mutex_waiter_less(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (likely(is_hw_futex_pi_enabled()) && likely(left) &&
	    likely(right) && !rt_prio(left->prio) && !rt_prio(right->prio)) ?
	    waiter_less(left, right) : -1;
}

static inline int hw_rt_mutex_waiter_equal(struct rt_mutex_waiter *left,
	struct rt_mutex_waiter *right)
{
	return (likely(is_hw_futex_pi_enabled()) && likely(left) &&
	    likely(right) && !rt_prio(left->prio) && !rt_prio(right->prio)) ?
	    waiter_equal(left, right) : -1;
}

unsigned int rt_mutex_calculate_vip_prio(struct task_struct *p,
	struct task_struct *pi_task);
#ifdef CONFIG_SECCOMP_FILTER
bool can_skip_filter(int this_syscall);
#endif
#endif /* HW_PI_H_INCLUDED */
