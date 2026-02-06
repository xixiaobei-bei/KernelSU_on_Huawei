/*
 * hwqos_common.c
 *
 * Qos schedule implementation
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwqos/hwqos_common.h>

#include <../kernel/sched/sched.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/slab.h>
#include <trace/events/sched.h>

#ifdef CONFIG_HW_RTG_FRAME
#include <linux/sched/hw_rtg/trans_rtg.h>
#endif
#ifdef CONFIG_HW_RTG_SCHED
#include "hwrtg/trans_rtg.h"
#endif
#if defined(CONFIG_HW_MTK_RTG_SCHED) && !defined(CONFIG_HW_RTG)
#include "mtkrtg/trans_rtg.h"
#endif
#include <chipset_common/hwqos/iaware_qos.h>

#define BASE_FLAG 0x00000001

#define get_trans_type(flags, type) \
	(((unsigned int)(flags)) & (BASE_FLAG << (type)))
#define set_trans_type(flags, type) \
	(((unsigned int)(flags)) | (BASE_FLAG << (type)))
#define remove_trans_type(flags, type) \
	(((unsigned int)(flags)) & (~(BASE_FLAG << (type))))
#define SCHED_VIP_PRIO_HIGH 5

// 10 1000 0001
// bit0: qos set, enable by default
// bit7: hisi vip sched set, enable by default
// bit9: minUtil set, enable by default
unsigned int g_sysctl_qos_sched = 0x281;

static inline struct transact_qos *get_transact_qos(unsigned int type,
	struct task_struct *task)
{
	return &(task->trans_qos[type]);
}

static inline bool is_qos_trans_type_valid(unsigned int type)
{
	return ((type >= DYNAMIC_QOS_BINDER) || (type < DYNAMIC_QOS_TYPE_MAX));
}

static int get_task_set_qos_inner(struct task_struct *task)
{
	int qos;
	int usage_proc;
	int usage_thread;

	if (unlikely(!task->proc_qos))
		return VALUE_QOS_INVALID;
	usage_proc = atomic_read(&task->proc_qos->usage);
	usage_thread = atomic_read(&task->thread_qos.usage);
	if (usage_proc > usage_thread) {
		qos = atomic_read(&task->proc_qos->dynamic_qos);
	} else if (usage_proc == usage_thread) {
		qos = atomic_read(&task->thread_qos.dynamic_qos);
	} else {
		// It's error for usage_thread is not bigger than usage_proc,
		// and we should repair the usage of proc and thread now
		if (unlikely(usage_proc <= 0)) {
			usage_proc = 1;
			atomic_set(&task->proc_qos->usage, usage_proc);
		}
		atomic_set(&task->thread_qos.usage, (usage_proc - 1));
		qos = atomic_read(&task->proc_qos->dynamic_qos);
	}
	if (unlikely((qos < VALUE_QOS_LOW) || (qos > VALUE_QOS_CRITICAL)))
		qos = VALUE_QOS_INVALID;
	return qos;
}

static void register_qos_trans_allows(struct task_struct *task, int qos)
{
	int i;
	unsigned long flags = 0;

	// Avoid to register_qos_trans_allows multiply with same task
	// which qos is VALUE_QOS_CRITICAL
	if (unlikely(get_task_set_qos_inner(task) == VALUE_QOS_CRITICAL))
		return;

	spin_lock_irqsave(&g_trans_qos_lock, flags);
	for (i = 0; i < QOS_TRANS_THREADS_NUM; i++) {
		if (g_qos_trans_allows[i].allow_pid == 0) {
			g_qos_trans_allows[i].allow_pid = task->pid;
			g_qos_trans_allows[i].allow_tgid = task->tgid;
			task->trans_allowed = &g_qos_trans_allows[i];
			break;
		}
	}
	if (unlikely(i == QOS_TRANS_THREADS_NUM)) {
		pr_warn("There is no available g_qos_trans_allows, and start to reinit now\n");
		g_qos_trans_allows[0].allow_pid = task->pid;
		g_qos_trans_allows[0].allow_tgid = task->tgid;
		task->trans_allowed = &g_qos_trans_allows[0];
		for (i = 1; i < QOS_TRANS_THREADS_NUM; i++)
			memset(&g_qos_trans_allows[i],
				0,
				sizeof(g_qos_trans_allows[i]));
	}
	spin_unlock_irqrestore(&g_trans_qos_lock, flags);
}

static void unregister_qos_trans_allows(struct task_struct *task)
{
	int i;
	unsigned long flags = 0;

	if (!task->trans_allowed)
		return;
	spin_lock_irqsave(&g_trans_qos_lock, flags);
	for (i = 0; i < QOS_TRANS_THREADS_NUM; i++) {
		if (g_qos_trans_allows[i].allow_tgid != task->tgid)
			continue;
		memset(&g_qos_trans_allows[i],
			0,
			sizeof(g_qos_trans_allows[i]));
	}
	task->trans_allowed = NULL;
	spin_unlock_irqrestore(&g_trans_qos_lock, flags);
}

void set_task_qos_by_tid(struct task_struct *task, int qos)
{
	if (unlikely((!task) || (!task->proc_qos)))
		return;
	if (qos == VALUE_QOS_CRITICAL)
		register_qos_trans_allows(task, qos);
	else
		unregister_qos_trans_allows(task);
	atomic_set(&task->thread_qos.dynamic_qos, qos);
	atomic_set(&task->thread_qos.usage,
		atomic_read(&task->proc_qos->usage));
	trace_sched_qos(task, NULL, OPERATION_QOS_SET_THREAD);
}

void set_task_qos_by_pid(struct task_struct *task, int qos)
{
	if (unlikely((!task) || (!task->proc_qos)))
		return;
	if (unlikely(!thread_group_leader(task))) {
		pr_warn("Failed to set qos, for it is not a group leader, pid=%d, tgid=%d\n",
			task->pid, task->tgid);
		return;
	}
	if (qos == VALUE_QOS_CRITICAL)
		register_qos_trans_allows(task, qos);
	else
		unregister_qos_trans_allows(task);
	atomic_set(&task->proc_qos->dynamic_qos, qos);
	if (atomic_add_negative(1, &task->proc_qos->usage))
		atomic_set(&task->proc_qos->usage, 1);
	trace_sched_qos(task, NULL, OPERATION_QOS_SET_PROCESS);
}

int get_task_set_qos(struct task_struct *task)
{
	int qos = VALUE_QOS_INVALID;

	if (unlikely(!task))
		return qos;
	qos = get_task_set_qos_inner(task);
	return qos;
}

int get_task_trans_qos(struct transact_qos *tq)
{
	struct trans_qos_allow *trans = NULL;

	if (unlikely(!tq))
		return VALUE_QOS_INVALID;
	trans = tq->trans_from;
	if (!trans)
		return VALUE_QOS_INVALID;
	if ((trans->allow_pid == tq->trans_pid) && (trans->allow_pid != 0))
		return VALUE_QOS_CRITICAL;
	return VALUE_QOS_INVALID;
}

static int get_trans_qos_by_type(struct task_struct *task, unsigned int type)
{
	struct transact_qos *tq = NULL;
	int flags = atomic_read(&task->trans_flags);

	if (!get_trans_type(flags, type))
		return VALUE_QOS_INVALID;
	tq = get_transact_qos(type, task);
	return get_task_trans_qos(tq);
}

int get_task_qos(struct task_struct *task)
{
	unsigned int i;
	int qos_trans;
	int qos;

	if (unlikely(!QOS_SCHED_SET_ENABLE))
		return VALUE_QOS_INVALID;
	if (unlikely(!task))
		return VALUE_QOS_INVALID;
	qos = get_task_set_qos_inner(task);
	if (qos == VALUE_QOS_CRITICAL)
		return qos;
	for (i = DYNAMIC_QOS_BINDER; i < DYNAMIC_QOS_TYPE_MAX; i++) {
		qos_trans = get_trans_qos_by_type(task, i);
		qos = (qos > qos_trans) ? qos : qos_trans;
		if (qos == VALUE_QOS_CRITICAL)
			break;
	}
	return qos;
}

#ifdef CONFIG_HW_FUTEX_PI
int get_task_normal_qos(struct task_struct *task)
{
	unsigned int i;
	int qos_trans;
	int qos;

	if (unlikely(!QOS_SCHED_SET_ENABLE))
		return VALUE_QOS_INVALID;
	if (unlikely(!task))
		return VALUE_QOS_INVALID;
	qos = get_task_set_qos_inner(task);
	if (qos == VALUE_QOS_CRITICAL)
		return qos;
	for (i = DYNAMIC_QOS_BINDER; i < DYNAMIC_QOS_TYPE_MAX; i++) {
		if (i == DYNAMIC_QOS_FUTEX)
			continue;
		qos_trans = get_trans_qos_by_type(task, i);
		qos = (qos > qos_trans) ? qos : qos_trans;
		if (qos == VALUE_QOS_CRITICAL)
			break;
	}
	return qos;
}
#endif

#ifdef CONFIG_HUAWEI_SCHED_VIP
static void dynamic_vip_prio_enqueue(struct task_struct *task,
	struct task_struct *from, unsigned int type)
{
	unsigned int vip_prio_desired;

	if (!VIP_PRIO_TRANS_ENABLE || type != DYNAMIC_QOS_BINDER)
		return;

	if (task->vip_params.trans_flag)
		return;
	if (task->sched_class != &fair_sched_class)
		return;
	if (task->vip_prio < SCHED_VIP_PRIO_HIGH &&
		get_task_set_qos_inner(from) == VALUE_QOS_HIGH)
		vip_prio_desired = SCHED_VIP_PRIO_HIGH;
	else if (from->vip_prio > task->vip_prio)
		vip_prio_desired = from->vip_prio;
	else
		return;
	task->vip_params.value = task->vip_prio;
	set_vip_prio(task, vip_prio_desired);
	task->vip_params.trans_flag = true;
}

static void dynamic_vip_prio_dequeue(struct task_struct *task, unsigned int type)
{
	if (!VIP_PRIO_TRANS_ENABLE || type != DYNAMIC_QOS_BINDER)
		return;

	if (!task->vip_params.trans_flag)
		return;
	set_vip_prio(task, task->vip_params.value);
	task->vip_params.value = 0;
	task->vip_params.trans_flag = false;
}
#endif

#ifdef CONFIG_SCHED_HISI_TASK_MIN_UTIL
static void dynamic_min_util_enqueue(struct task_struct *task,
	struct task_struct *from, unsigned int type)
{
	unsigned int min_util_desired;

	if (!MIN_UTIL_TRANS_ENABLE || type != DYNAMIC_QOS_BINDER)
		return;

	if (task->min_util_params.trans_flag)
		return;
	min_util_desired = from->uclamp.min_util;
	if (min_util_desired == 0 || min_util_desired <= task->uclamp.min_util)
		return;
	task->min_util_params.value = task->uclamp.min_util;
	set_task_min_util(task, min_util_desired);
	task->min_util_params.trans_flag = true;
}

static void dynamic_min_util_dequeue(struct task_struct *task, unsigned int type)
{
	if (!MIN_UTIL_TRANS_ENABLE || type != DYNAMIC_QOS_BINDER)
		return;

	if (!task->min_util_params.trans_flag)
		return;
	set_task_min_util(task, task->min_util_params.value);
	task->min_util_params.value = 0;
	task->min_util_params.trans_flag = false;
}
#endif

static bool dynamic_qos_enqueue_inner(struct task_struct *task,
	struct task_struct *from, unsigned int type)
{
	bool ret = false;
	unsigned int i;
	struct trans_qos_allow *trans_qos = NULL;
	int qos = get_task_set_qos_inner(from);

	if (qos == VALUE_QOS_CRITICAL) {
		trans_qos = from->trans_allowed;
	} else {
		for (i = DYNAMIC_QOS_BINDER; i < DYNAMIC_QOS_TYPE_MAX; i++) {
			if (get_trans_qos_by_type(from, i) ==
				VALUE_QOS_CRITICAL) {
				trans_qos =
					get_transact_qos(i, from)->trans_from;
				break;
			}
		}
	}
	if (trans_qos) {
		int flags;
		struct transact_qos *tq = get_transact_qos(type, task);

		tq->trans_from = trans_qos;
		tq->trans_pid = trans_qos->allow_pid;
		tq->trans_type = type;
#ifdef CONFIG_HW_RTG_SCHED
		if (RTG_TRANS_ENABLE && (type == DYNAMIC_QOS_BINDER))
			add_trans_thread(task, from);
#endif
#if defined(CONFIG_HW_RTG_FRAME) || defined(CONFIG_HW_MTK_RTG_SCHED)
		if (RTG_TRANS_ENABLE && (type == DYNAMIC_QOS_BINDER))
			trans_rtg_sched_enqueue(task, from, type);
#endif
		flags = set_trans_type(atomic_read(&task->trans_flags), type);
		atomic_set(&task->trans_flags, flags); /*lint !e446 !e734*/
#ifdef CONFIG_SCHED_HWSTATUS
		sched_hwstatus_qos_enqueue(task, type);
#endif
		trace_sched_qos(task, tq, OPERATION_QOS_ENQUEUE);
		ret = true;
	}
	return ret;
}

bool dynamic_qos_enqueue(struct task_struct *task,
	struct task_struct *from, unsigned int type)
{
	bool ret = false;

	if (unlikely(!QOS_SCHED_ENQUEUE_ENABLE))
		return ret;
	if (unlikely(!is_qos_trans_type_valid(type)))
		return ret;
	if (unlikely(!task))
		return ret;
	if (unlikely(!from))
		return ret;
#ifdef CONFIG_HUAWEI_SCHED_VIP
	dynamic_vip_prio_enqueue(task, from, type);
#endif
#ifdef CONFIG_SCHED_HISI_TASK_MIN_UTIL
	dynamic_min_util_enqueue(task, from, type);
#endif
	if (get_trans_qos_by_type(task, type) == VALUE_QOS_CRITICAL)
		return ret;
	return dynamic_qos_enqueue_inner(task, from, type);
}

/*lint -save -e502 -e446 -e734*/
void dynamic_qos_dequeue(struct task_struct *task, unsigned int type)
{
	int flags;
	struct transact_qos *tq = NULL;

	if (unlikely(!QOS_SCHED_ENQUEUE_ENABLE))
		return;
	if (unlikely(!is_qos_trans_type_valid(type)))
		return;
	if (unlikely(!task))
		return;

#ifdef CONFIG_HUAWEI_SCHED_VIP
	dynamic_vip_prio_dequeue(task, type);
#endif
#ifdef CONFIG_SCHED_HISI_TASK_MIN_UTIL
	dynamic_min_util_dequeue(task, type);
#endif

	flags = atomic_read(&task->trans_flags);
	if (!get_trans_type(flags, type))
		return;
#ifdef CONFIG_SCHED_HWSTATUS
	sched_hwstatus_qos_dequeue(task, type);
#endif
	tq = get_transact_qos(type, task);
	tq->trans_from = NULL;
	flags = remove_trans_type(atomic_read(&task->trans_flags), type);
	atomic_set(&task->trans_flags, flags);
#ifdef CONFIG_HW_RTG_SCHED
	if (RTG_TRANS_ENABLE && (type == DYNAMIC_QOS_BINDER))
		remove_trans_thread(task);
#endif
#if defined(CONFIG_HW_RTG_FRAME) || defined(CONFIG_HW_MTK_RTG_SCHED)
	if (RTG_TRANS_ENABLE && (type == DYNAMIC_QOS_BINDER))
		trans_rtg_sched_dequeue(task, type);
#endif
	trace_sched_qos(task, tq, OPERATION_QOS_DEQUEUE);
}
/*lint -restore*/

void iaware_proc_fork_inherit(struct task_struct *task,
	unsigned long clone_flags)
{
	int qos;
	int usage_thread;

	if (unlikely(!QOS_SCHED_SET_ENABLE))
		return;
	if (unlikely((!task) || (!task->proc_qos) || (!current->proc_qos)))
		return;
	if (clone_flags & CLONE_THREAD) {
		task->proc_qos = current->proc_qos;
	} else {
		task->proc_qos = kzalloc(sizeof(*task->proc_qos), GFP_KERNEL);
		if (task->proc_qos) {
			atomic_set(&task->proc_qos->dynamic_qos,
				atomic_read(&current->proc_qos->dynamic_qos));
			atomic_set(&task->proc_qos->usage,
				atomic_read(&current->proc_qos->usage));
		}
	}
	qos = atomic_read(&current->thread_qos.dynamic_qos);
	usage_thread = atomic_read(&current->thread_qos.usage);
	if (qos == VALUE_QOS_CRITICAL)
		qos = VALUE_QOS_HIGH;
	atomic_set(&task->thread_qos.dynamic_qos, qos);
	atomic_set(&task->thread_qos.usage, usage_thread);
	trace_sched_qos(task, NULL, OPERATION_QOS_SET_THREAD);
}

bool should_binder_do_set_priority(struct task_struct *task, int desired_prio, bool verify)
{
	if (unlikely(!BINDER_SET_PRIO_ENABLE))
		return true;

	// false indicates that it's called by binder_restore_priority()
	if (!verify)
		return true;

	return (task->normal_prio > desired_prio);
}
