/*
 * hw_futex_pi.c
 *
 * Description: customize futex pi and support qos and vip_prio inheritance
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
#include <chipset_common/linux/hw_pi.h>
#include <trace/events/sched.h>
#ifdef CONFIG_HW_QOS_THREAD
#include <chipset_common/hwqos/hwqos_common.h>
#endif

unsigned int g_hw_futex_pi_enabled;

#ifdef CONFIG_HW_QOS_THREAD
static int rt_mutex_adjust_and_return_qos(struct task_struct *p,
	struct task_struct *pi_task)
{
	int pi_qos;
	int oldqos = get_task_qos(p);
	int qos = get_task_normal_qos(p);

	if (pi_task) {
		pi_qos = get_task_qos(pi_task);
		qos = max(qos, ((pi_qos < MIN_INHERIT_QOS) ? 0 : pi_qos));
	}

	if (qos > oldqos && pi_task) {
		trace_sched_pi_setqos(p, oldqos, qos);
		dynamic_qos_enqueue(p, pi_task, DYNAMIC_QOS_FUTEX);
	} else if (qos < oldqos) {
		trace_sched_pi_setqos(p, oldqos, qos);
		dynamic_qos_dequeue(p, DYNAMIC_QOS_FUTEX);
	}
	return qos;
}
#endif

unsigned int rt_mutex_calculate_vip_prio(struct task_struct *p,
	struct task_struct *pi_task)
{
	unsigned int vip_prio = 0;
#ifdef CONFIG_HW_QOS_THREAD
	int qos;
#endif

	if (unlikely(!p))
		return 0;
	if (unlikely(!is_hw_futex_pi_enabled()))
#ifdef CONFIG_HUAWEI_SCHED_VIP
		return p->vip_prio;
#else
		return 0;
#endif

#ifdef CONFIG_HW_QOS_THREAD
	qos = rt_mutex_adjust_and_return_qos(p, pi_task);
#endif
#ifdef CONFIG_HUAWEI_SCHED_VIP
	if (pi_task)
		vip_prio = max(p->normal_vip_prio, pi_task->vip_prio);
	else
		vip_prio = p->normal_vip_prio;

#ifdef CONFIG_HW_QOS_THREAD
	if (qos == VALUE_QOS_CRITICAL)
		vip_prio = max(vip_prio, MIN_INHERIT_VIP_PRIO);
#endif
#endif
	return vip_prio;
}

#ifdef CONFIG_SECCOMP_FILTER
bool can_skip_filter(int this_syscall)
{
	bool is_futex = false;

	if (unlikely(READ_ONCE(current->seccomp.filter) == NULL))
		return false;
#ifdef CONFIG_COMPAT
	#define __NR_FUTEX32    240
	is_futex = ((this_syscall == __NR_FUTEX32) &&
	    test_thread_flag(TIF_32BIT)) ||
	    ((this_syscall == __NR_futex) &&
	    !test_thread_flag(TIF_32BIT));
#else
	is_futex = (this_syscall == __NR_futex);
#endif
	if (likely(!is_futex))
		return false;

	switch (task_pt_regs(current)->regs[1] & FUTEX_CMD_MASK) {
	case FUTEX_LOCK_PI:
	case FUTEX_UNLOCK_PI:
		if (likely(is_hw_futex_pi_enabled()))
			return true;
		break;
	default:
		break;
	}
	return false;
}
#endif
