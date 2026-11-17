/*
 * hwcfs_common.c
 *
 * hwcfs schedule implementation
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

#ifdef CONFIG_HW_VIP_THREAD
/*lint -save -e578 -e695 -e571*/
#include <chipset_common/hwcfs/hwcfs_common.h>

#include <linux/version.h>
#include <linux/list.h>
#include <linux/jiffies.h>
#include <trace/events/sched.h>
#include <../kernel/sched/sched.h>

#define DYNAMIC_VIP_SEC_WIDTH 8
#define DYNAMIC_VIP_MASK_BASE 0x00000000ff

#define MS_TO_NS 1000000
#define dynamic_vip_offset_of(type) (type * DYNAMIC_VIP_SEC_WIDTH)
#define dynamic_vip_mask_of(type) \
	((u64)(DYNAMIC_VIP_MASK_BASE) << (dynamic_vip_offset_of(type)))
#define dynamic_vip_get_bits(value, type) \
	((value & dynamic_vip_mask_of(type)) >> dynamic_vip_offset_of(type))
#define dynamic_vip_one(type) ((u64)1 << dynamic_vip_offset_of(type))

#ifdef CONFIG_VIP_SCHED_OPT
#include <../kernel/sched/tune.h>
#include <../kernel/sched/walt.h>
#include <../../drivers/misc/mediatek/sched/sched_ctl.h>
#include <../kernel/sched/eas_plus.h>
#endif

#ifdef CONFIG_HW_EAS_SCHED
#include <linux/hw/topology_hw.h>
#endif

int vip_min_sched_delay_granularity; /* vip thread delay upper bound(ms) */
int vip_max_dynamic_granularity = 32; /* vip dynamic max exist time(ms) */
int vip_min_migration_delay = 10; /* vip min migration delay time(ms) */

#define S2NS_T 1000000

#ifdef CONFIG_VIP_SCHED_OPT
#define VIP_TASK_LIMIT_RUNNABLE_NS 50000000 /* 50ms */
#define VIP_TASK_LIMIT_ITER_NUM 20 /* max 20 for an rq */

#ifdef CONFIG_FAIR_GROUP_SCHED
#define entity_is_task(se)	(!se->my_q)
#else
#define entity_is_task(se)	1
#endif

static void update_rq_highest_prio(struct rq *rq)
{
	int highest_prio = 0;
	int task_cnt = 0;
	struct task_struct *task = NULL;

	list_for_each_entry(task, &rq->vip_thread_list, vip_entry) {
		highest_prio = max(highest_prio, task->vip_prio);

		if (++task_cnt >= VIP_TASK_LIMIT_ITER_NUM)
			break;
	}

	rq->highest_vip_prio = highest_prio;
}
#endif

static int entity_before(struct sched_entity *a,
	struct sched_entity *b)
{
	return (s64)(a->vruntime - b->vruntime) < 0;
}

void enqueue_vip_thread(struct rq *rq, struct task_struct *p)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	bool exist = false;

	if (!rq || !p || !list_empty(&p->vip_entry))
		return;

	p->enqueue_time = rq->clock;
	if (p->static_vip || atomic64_read(&p->dynamic_vip)) {
		list_for_each_safe(pos, n, &rq->vip_thread_list) {
			if (pos == &p->vip_entry) {
				exist = true;
				break;
			}
		}
		if (!exist) {
			list_add_tail(&p->vip_entry, &rq->vip_thread_list);
			get_task_struct(p);
			trace_sched_vip_queue_op(p, "vip_enqueue_succ");
		}
#ifdef CONFIG_VIP_SCHED_OPT
		if (p->vip_prio > rq->highest_vip_prio)
			rq->highest_vip_prio = p->vip_prio;
#endif
	}
}

void dequeue_vip_thread(struct rq *rq, struct task_struct *p)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	u64 now = jiffies_to_nsecs(jiffies);
	u64 interval = (u64)vip_max_dynamic_granularity * MS_TO_NS;

	if (!rq || !p)
		return;

	p->enqueue_time = 0;
	if (!list_empty(&p->vip_entry)) {
		list_for_each_safe(pos, n, &rq->vip_thread_list) {
			if (atomic64_read(&p->dynamic_vip) &&
				(now - p->dynamic_vip_start) > interval)
				atomic64_set(&p->dynamic_vip, 0);
		}
		list_for_each_safe(pos, n, &rq->vip_thread_list) {
			if (pos == &p->vip_entry) {
				list_del_init(&p->vip_entry);
				put_task_struct(p);
				trace_sched_vip_queue_op(p, "vip_dequeue_succ");
				break;
			}
		}
	}
#ifdef CONFIG_VIP_SCHED_OPT
	if (p->vip_prio != 0 && p->vip_prio >= rq->highest_vip_prio)
		update_rq_highest_prio(rq);
#endif
}

#ifdef CONFIG_VIP_SCHED_OPT
static struct task_struct *
pick_first_vip_thread(struct rq *rq)
{
	struct task_struct *task = NULL;
	struct task_struct *leftmost_task = NULL;
	struct task_struct *n = NULL;
	unsigned int task_cnt = 0;

	list_for_each_entry_safe(task, n, &rq->vip_thread_list, vip_entry) {
		if (unlikely(task_rq(task) != rq)) {
			pr_warn("task %d on another rq\n", task->pid);
			list_del_init(&task->vip_entry);
			continue;
		}

		if (unlikely(!task_on_rq_queued(task))) {
			pr_warn("task %d not queued\n", task->pid);
			list_del_init(&task->vip_entry);
			continue;
		}

		if (!leftmost_task ||
			task->vip_prio > leftmost_task->vip_prio ||
			(task->vip_prio == leftmost_task->vip_prio &&
			entity_before(&task->se, &leftmost_task->se)))
			leftmost_task = task;

		if (++task_cnt >= VIP_TASK_LIMIT_ITER_NUM)
			break;
	}

	if (leftmost_task) {
		s64 enqueue_time;

		trace_sched_pick_next_vip(leftmost_task);
		/*
		 * Waking someone with WF_SYNC.
		 * Let the wakee have a chance to preempt and run.
		 */
		if (test_tsk_thread_flag(leftmost_task, TIF_WAKE_SYNC))
			return NULL;

		/*
		 * Skip picking the vip task if it has been enqueued too long
		 * to prevent starving or even livelock.
		 */
		enqueue_time = rq_clock(rq) - leftmost_task->enqueue_time;
		if (unlikely(enqueue_time > VIP_TASK_LIMIT_RUNNABLE_NS))
			return NULL;
	}

	return leftmost_task;
}

void pick_vip_thread(struct rq *rq, struct task_struct **p,
	struct sched_entity **se)
{
	struct task_struct *key_task = NULL;
	struct sched_entity *key_se = NULL;

	if (!rq || !p || !se)
		return;

	if (!list_empty(&rq->vip_thread_list)) {
		key_task = pick_first_vip_thread(rq);
		if (key_task) {
			key_se = &key_task->se;
			if (key_se) {
				trace_sched_vip_sched(key_task, "pick_vip");
				*p = key_task;
				*se = key_se;
			}
		}
	}
}

#ifndef CONFIG_HW_EAS_SCHED
struct task_struct *get_heaviest_vip_task(struct sched_entity *se,
	struct task_struct *p, int cpu)
{
	int loop = 5; // max loop time: 5
	unsigned long max_util = task_util_est(p);
	unsigned long max_preferred_util = 0;
	struct task_struct *tsk = NULL;
	struct task_struct *max_preferred_tsk = NULL;
	struct task_struct *max_util_task = p;

	while (loop && se) {
		if (!entity_is_task(se)) {
			se = __pick_next_entity(se);
			loop--;
			continue;
		}
		tsk = task_of(se);
		if (cpumask_test_cpu(cpu, &tsk->cpus_allowed)) {
			bool boosted = schedtune_task_boost(tsk) > 0;
			bool prefer_idle = schedtune_prefer_idle(tsk) > 0;
			unsigned long util = boosted_task_util(tsk);

			/* prefer vip task */
			if (tsk->vip_prio)
				return tsk;

			if ((boosted || prefer_idle) && util > max_preferred_util) {
				max_preferred_util = util;
				max_preferred_tsk = tsk;
			} else if (!boosted && !prefer_idle && util > max_util) {
				max_util = util;
				max_util_task = tsk;
			}
		}

		se = __pick_next_entity(se);
		loop--;
	}

	return max_preferred_tsk ? max_preferred_tsk : max_util_task;
}
#endif

#else

static struct task_struct *pick_first_vip_thread(struct rq *rq)
{
	struct list_head *vip_thread_list = &rq->vip_thread_list;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct task_struct *temp = NULL;
	struct task_struct *left_most_task = NULL;

	list_for_each_safe(pos, n, vip_thread_list) {
		temp = list_entry(pos, struct task_struct, vip_entry);
		/* ensure vip task in current rq cpu otherwise delete it */
		if (unlikely(task_cpu(temp) != rq->cpu)) {
			printk(KERN_WARNING "task(%s,%d,%d) does not belong to cpu%d",
				temp->comm, task_cpu(temp),
				temp->policy, rq->cpu);
			list_del_init(&temp->vip_entry);
			continue;
		}
		if (left_most_task == NULL)
			left_most_task = temp;
		else if (entity_before(&temp->se, &left_most_task->se))
			left_most_task = temp;
	}

	return left_most_task;
}

void pick_vip_thread(struct rq *rq,
	struct task_struct **p, struct sched_entity **se)
{
	struct task_struct *ori_p = NULL;
	struct task_struct *key_task = NULL;
	struct sched_entity *key_se = NULL;
	u64 interval = (u64)vip_min_sched_delay_granularity * MS_TO_NS;

	if (!rq || !p || !se)
		return;

	ori_p = *p;
	if (ori_p && !ori_p->static_vip &&
		!atomic64_read(&ori_p->dynamic_vip)) {
		if (list_empty(&rq->vip_thread_list))
			return;
		key_task = pick_first_vip_thread(rq);
		if (!key_task)
			return;
		key_se = &key_task->se;
		if (key_se && (rq->clock >= key_task->enqueue_time) &&
			(rq->clock - key_task->enqueue_time >= interval)) {
			trace_sched_vip_sched(key_task, "pick_vip");
			*p = key_task;
			*se = key_se;
		}
	}
}
#endif

#ifdef CONFIG_VIP_SCHED_OPT
static int find_lowest_vip_cpu(struct task_struct *p,
			struct cpumask *search_cpus)
{
	int i;
	int target_cpu = -1;
	unsigned int lowest_prio = p->vip_prio;
	unsigned long max_cap = 0;
	unsigned long min_util = ULONG_MAX;

	for_each_cpu(i, search_cpus) {
		unsigned int prio = cpu_rq(i)->highest_vip_prio;
		unsigned long cap = capacity_orig_of(i);
		unsigned long util = cpu_util_without(i, p);

		if (p->state == TASK_RUNNING && is_reserved(i))
			continue;

		if (prio > lowest_prio)
			continue;

		if (prio == lowest_prio) {
			if (cap < max_cap)
				continue;
			if (cap == max_cap &&
				util >= min_util)
				continue;
		}

		lowest_prio = prio;
		max_cap = cap;
		min_util = util;
		target_cpu = i;
	}

	return target_cpu;
}

#ifndef CONFIG_HW_EAS_SCHED
static inline bool task_fits(struct task_struct *p, int cpu)
{
	unsigned long capacity = capacity_of(cpu);
	unsigned long util = boosted_task_util(p);

	return (capacity << SCHED_CAPACITY_SHIFT) > (util * capacity_orig_of(cpu));
}

bool task_fits_max(struct task_struct *p, int cpu)
{
	unsigned long capacity_orig = capacity_orig_of(cpu);
	unsigned long max_capacity = cpu_rq(cpu)->rd->max_cpu_capacity.val;
	cpumask_t allowed_cpus;
	int allowed_cpu;

	if (capacity_orig == max_capacity)
		return true;

	if (task_fits(p, cpu))
		return true;

	/* Now task does not fit. Check if there's a better one. */
	cpumask_and(&allowed_cpus, &p->cpus_allowed, cpu_online_mask);
	for_each_cpu(allowed_cpu, &allowed_cpus) {
		if (capacity_orig_of(allowed_cpu) > capacity_orig)
			return false; /* Misfit */
	}

	/* Already largest capacity in allowed cpus. */
	return true;
}
#endif

static int get_vip_target_cpu(struct task_struct *p, cpumask_t search_cpus,
	unsigned long target_capacity, bool favor_larger_capacity)
{
	int i;
	int target_cpu = -1;
	unsigned long capacity_orig;
	unsigned long wake_util;
	int idle_idx;
	int best_idle_cstate = INT_MAX;
	unsigned long min_wake_util = ULONG_MAX;

	for_each_cpu(i, &search_cpus) {
		trace_sched_find_vip_cpu_each(
			p, i, !list_empty(&cpu_rq(i)->vip_thread_list),
			cpu_rq(i)->highest_vip_prio, is_reserved(i),
			walt_cpu_high_irqload(i), capacity_orig_of(i),
			cpu_util_without(i, p)
		);

		if (!list_empty(&cpu_rq(i)->vip_thread_list))
			continue;

		if (is_reserved(i))
			continue;

		if (walt_cpu_high_irqload(i))
			continue;

		capacity_orig = capacity_orig_of(i);
		wake_util = cpu_util_without(i, p);
		idle_idx = idle_cpu(i) ? idle_get_state_idx(cpu_rq(i))
					: INT_MAX;

		if (!task_fits_max(p, i))
			continue;

		/* Favor larger/smaller capacity cpu when possible. */
		if (favor_larger_capacity) {
			if (capacity_orig < target_capacity)
				continue;
		} else if (capacity_orig > target_capacity) {
			continue;
		}

		if (capacity_orig == target_capacity) {
			/*
			 * In same cluster, favor lowest wake util
			 * cpu or shallowest idle cpu.
			 */
			if (wake_util > min_wake_util)
				continue;

			if (idle_idx >= best_idle_cstate)
				continue;
		}

		target_cpu = i;
		target_capacity = capacity_orig;
		min_wake_util = wake_util;
		best_idle_cstate = idle_idx;
	}
	return target_cpu;
}

int find_vip_cpu(struct task_struct *p)
{
	int target_cpu;
	cpumask_t search_cpus = CPU_MASK_NONE;
	bool favor_larger_capacity = false;
	unsigned long target_capacity = ULONG_MAX;

#ifdef CONFIG_MTK_SCHED_BOOST
	if (cpu_prefer(p) == SCHED_PREFER_BIG)
		favor_larger_capacity = true;
#endif

	if (favor_larger_capacity)
		target_capacity = 0;

	cpumask_and(&search_cpus, &p->cpus_allowed, cpu_online_mask);
	target_cpu = get_vip_target_cpu(p, search_cpus, target_capacity,
		favor_larger_capacity);
	/*
	 * If we didn't find a suitable non-vip cpu, try to find a cpu
	 * with lowest vip prio and do not consider power efficiency.
	 */
	if (target_cpu == -1)
		target_cpu = find_lowest_vip_cpu(p, &search_cpus);

	trace_sched_find_vip_cpu(p, favor_larger_capacity, target_cpu);
	return target_cpu;
}

enum vip_preempt_type vip_preempt_classify(struct rq *rq,
			struct task_struct *prev, struct task_struct *next)
{
	/* Switching to a normal(not vip) cfs task or idle task. */
	if (!next->vip_prio && next->sched_class != &rt_sched_class)
		return NO_VIP_PREEMPT;

	/*
	 * Switching to a vip/rt task.
	 * There might be a runnable task starving so give load balancer
	 * a hint.
	 */
	if (prev->on_rq) {
		/*
		 * Mark witch type of task is preempted.
		 */
		if (prev->vip_prio)
			return VIP_PREEMPT_VIP;
		if (schedtune_prefer_idle(prev))
			return VIP_PREEMPT_TOPAPP;
		if (prev->sched_class == &fair_sched_class)
			return VIP_PREEMPT_OTHER;
	} else {
		/*
		 * No preemption.
		 * Check if there are runnable tasks(prev task
		 * has been deactivated at this point).
		 */
		return rq->nr_running > 1
			? VIP_PREEMPT_OTHER
			: NO_VIP_PREEMPT;
	}

	/*
	 * Keep it unchanged.
	 * Note that cpu_overutilized_for_lb() will update it if necessary.
	 */
	return rq->vip_preempt_type;
}

void fixup_vip_preempt_type(struct rq *rq)
{
	if (rq->vip_preempt_type &&
		rq->nr_running <= 1) {
		rq->vip_preempt_type = NO_VIP_PREEMPT;
		return;
	}

	if (!rq->vip_preempt_type &&
	    rq->highest_vip_prio &&
	    rq->nr_running > 1) {
		rq->vip_preempt_type = VIP_PREEMPT_OTHER;
		return;
	}
}
unsigned int select_allowed_cpu(struct task_struct *p,
	struct cpumask *allowed_cpus)
{
	int cpu;

	if (p->vip_prio) {
		cpu = find_lowest_vip_cpu(p, allowed_cpus);
		if (cpu != -1)
			return (unsigned int)cpu;
	}
	return cpumask_any(allowed_cpus);
}
#endif // CONFIG_VIP_SCHED_OPT

bool test_dynamic_vip(struct task_struct *task, int type)
{
	u64 dynamic_vip;

	if (!task)
		return false;

	dynamic_vip = atomic64_read(&task->dynamic_vip);
	return dynamic_vip_get_bits(dynamic_vip, (u32)type) > 0;
}

static bool test_task_exist(struct task_struct *task, struct list_head *head)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	list_for_each_safe(pos, n, head) {
		if (pos == &task->vip_entry)
			return true;
	}
	return false;
}

static inline void dynamic_vip_dec(struct task_struct *task, int type)
{
	atomic64_sub(dynamic_vip_one((u32)type), &task->dynamic_vip);
}

static inline void dynamic_vip_inc(struct task_struct *task, int type)
{
	atomic64_add(dynamic_vip_one((u32)type), &task->dynamic_vip);
}

static void __dynamic_vip_dequeue(struct task_struct *task, int type)
{
#if KERNEL_VERSION(4, 9, 0) > LINUX_VERSION_CODE
	unsigned long flags;
#else
	struct rq_flags flags;
#endif
	bool exist = false;
	struct rq *rq = NULL;
	u64 dynamic_vip;

	rq = task_rq_lock(task, &flags);
	dynamic_vip = atomic64_read(&task->dynamic_vip);
	if (dynamic_vip <= 0) {
		task_rq_unlock(rq, task, &flags);
		return;
	}
	dynamic_vip_dec(task, type);
	dynamic_vip = atomic64_read(&task->dynamic_vip);
	if (dynamic_vip > 0) {
		task_rq_unlock(rq, task, &flags);
		return;
	}
	task->vip_depth = 0;

	exist = test_task_exist(task, &rq->vip_thread_list);
	if (exist) {
		trace_sched_vip_queue_op(task, "dynamic_vip_dequeue_succ");
		list_del_init(&task->vip_entry);
		put_task_struct(task);
	}
	task_rq_unlock(rq, task, &flags);
}

void dynamic_vip_dequeue(struct task_struct *task, int type)
{
	if (!task || type >= DYNAMIC_VIP_MAX)
		return;

	__dynamic_vip_dequeue(task, type);
}

extern const struct sched_class fair_sched_class;
static void __dynamic_vip_enqueue(struct task_struct *task, int type, int depth)
{
#if KERNEL_VERSION(4, 9, 0) > LINUX_VERSION_CODE
	unsigned long flags;
#else
	struct rq_flags flags;
#endif
	bool exist = false;
	struct rq *rq;

	rq = task_rq_lock(task, &flags);
	if (task->sched_class != &fair_sched_class) {
		task_rq_unlock(rq, task, &flags);
		return;
	}
	if (unlikely(!list_empty(&task->vip_entry))) {
		printk(KERN_WARNING "task(%s,%d,%d) is already in another list",
			task->comm, task->pid, task->policy);
		task_rq_unlock(rq, task, &flags);
		return;
	}

	dynamic_vip_inc(task, type);
	task->dynamic_vip_start = jiffies_to_nsecs(jiffies);
	task->vip_depth =
		task->vip_depth > (depth + 1) ? task->vip_depth : (depth + 1);
	if (task->state == TASK_RUNNING) {
		exist = test_task_exist(task, &rq->vip_thread_list);
		if (!exist) {
			get_task_struct(task);
			list_add_tail(&task->vip_entry, &rq->vip_thread_list);
			trace_sched_vip_queue_op(
				task, "dynamic_vip_enqueue_succ");
		}
	}
	task_rq_unlock(rq, task, &flags);
}

void dynamic_vip_enqueue(struct task_struct *task, int type, int depth)
{
	if (!task || type >= DYNAMIC_VIP_MAX)
		return;

	__dynamic_vip_enqueue(task, type, depth);
}

inline bool test_task_vip(struct task_struct *task)
{
	return task && (task->static_vip || atomic64_read(&task->dynamic_vip));
}

inline bool test_task_vip_depth(int vip_depth)
{
	return vip_depth < VIP_DEPTH_MAX;
}

inline bool test_set_dynamic_vip(struct task_struct *tsk)
{
	return test_task_vip(tsk) && test_task_vip_depth(tsk->vip_depth);
}

static struct task_struct *check_vip_delayed(struct rq *rq)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct task_struct *tsk = NULL;
	struct list_head *vip_thread_list = NULL;
	u64 interval = (u64)vip_min_migration_delay * MS_TO_NS;

	vip_thread_list = &rq->vip_thread_list;
	list_for_each_safe(pos, n, vip_thread_list) {
		tsk = list_entry(pos, struct task_struct, vip_entry);
		if (tsk && (rq->clock - tsk->enqueue_time) >= interval)
			return tsk;
	}
	return NULL;
}

static int vip_task_hot(struct task_struct *p,
	struct rq *src_rq, struct rq *dst_rq)
{
	s64 delta;

	lockdep_assert_held(&src_rq->lock);

	if (p->sched_class != &fair_sched_class)
		return 0;

	if (unlikely(p->policy == SCHED_IDLE))
		return 0;

	if (sched_feat(CACHE_HOT_BUDDY) && dst_rq->nr_running &&
		(&p->se == src_rq->cfs.next || &p->se == src_rq->cfs.last))
		return 1;

	if (sysctl_sched_migration_cost == (unsigned int)-1)
		return 1;
	if (sysctl_sched_migration_cost == 0)
		return 0;

	delta = src_rq->clock_task - p->se.exec_start;
	return delta < (s64)sysctl_sched_migration_cost;
}

static void detach_task(struct task_struct *p,
	struct rq *src_rq, struct rq *dst_rq)
{
	trace_sched_vip_queue_op(p, "detach_task");
	lockdep_assert_held(&src_rq->lock);
	deactivate_task(src_rq, p, 0);
	p->on_rq = TASK_ON_RQ_MIGRATING;
	double_lock_balance(src_rq, dst_rq);
	set_task_cpu(p, dst_rq->cpu);
	double_unlock_balance(src_rq, dst_rq);
}

static void attach_task(struct rq *dst_rq, struct task_struct *p)
{
	trace_sched_vip_queue_op(p, "attach_task");
	raw_spin_lock(&dst_rq->lock);
	if (task_rq(p) != dst_rq) { /*lint !e58*/
		raw_spin_unlock(&dst_rq->lock);
		return;
	}
	p->on_rq = TASK_ON_RQ_QUEUED;
	activate_task(dst_rq, p, 0);
	check_preempt_curr(dst_rq, p, 0);
	raw_spin_unlock(&dst_rq->lock);
}

static int vip_can_migrate(struct task_struct *p,
	struct rq *src_rq, struct rq *dst_rq)
{
	if (task_running(src_rq, p))
		return 0;
	if (vip_task_hot(p, src_rq, dst_rq))
		return 0;
	if (task_rq(p) != src_rq) /*lint !e58*/
		return 0;
	if (!test_task_exist(p, &src_rq->vip_thread_list))
		return 0;
	return 1;
}

#ifdef CONFIG_HISI_EAS_SCHED
static struct cpumask slow_cpu_mask;
#endif

static void migrate_task(struct rq *src_rq,
	struct rq *dst_rq, struct task_struct *task, int cpu_id)
{
	struct task_struct *p = task;
	bool is_mig = false;

	if (!src_rq)
		return;

	/* move task from src_rq to dst_rq cpu */
	raw_spin_lock(&src_rq->lock);
	if (cpu_id != nr_cpu_ids && p != NULL && dst_rq != NULL) {
		if (vip_can_migrate(p, src_rq, dst_rq)) {
			detach_task(p, src_rq, dst_rq);
			is_mig = true;
		}
	}
	src_rq->active_vip_balance = 0;
	raw_spin_unlock(&src_rq->lock);
	if (is_mig)
		attach_task(dst_rq, p);
}

static struct rq *get_dest_rq(int src_cpu,
	struct cpumask *cpus_allowed, int *cpu_id)
{
	int i;
	struct rq *dst_rq = NULL;

#if defined(CONFIG_HW_EAS_SCHED) || defined(CONFIG_HISI_EAS_SCHED)
	for (i = 0; i < nr_cpu_ids; i++) {
		if (i == src_cpu)
			continue;
		if (cpumask_test_cpu(i, cpus_allowed)) {
			dst_rq = cpu_rq(i);
			raw_spin_lock(&dst_rq->lock);
			if (!dst_rq->rt.rt_nr_running ||
				list_empty(&dst_rq->vip_thread_list)) {
				raw_spin_unlock(&dst_rq->lock);
				break;
			}
			raw_spin_unlock(&dst_rq->lock);
		}
	}
#else
	for_each_cpu_and(i, cpus_allowed, cpu_coregroup_mask(src_cpu)) {
		if (i == src_cpu || !cpu_online(i))
			continue;
		dst_rq = cpu_rq(i);
		raw_spin_lock(&dst_rq->lock);
		if (!dst_rq->rt.rt_nr_running ||
			list_empty(&dst_rq->vip_thread_list)) {
			raw_spin_unlock(&dst_rq->lock);
			break;
		}
		raw_spin_unlock(&dst_rq->lock);
	}
#endif
	*cpu_id = i;
	return dst_rq;
}

static int __do_vip_balance(void *data)
{
	struct rq *src_rq = data;
	struct rq *dst_rq = NULL;
	int src_cpu = cpu_of(src_rq);
	int i;
	struct task_struct *p = NULL;
#if defined(CONFIG_HW_EAS_SCHED) || defined(CONFIG_HISI_EAS_SCHED)
	struct cpumask cluster = CPU_MASK_NONE;
	struct cpumask cpus_allowed = CPU_MASK_NONE;
#endif

	/* find a delayed vip task */
	raw_spin_lock_irq(&src_rq->lock);
	if (unlikely(src_cpu != smp_processor_id() ||
		!src_rq->active_vip_balance)) {
		src_rq->active_vip_balance = 0;
		raw_spin_unlock_irq(&src_rq->lock);
		return 0;
	}
	p = check_vip_delayed(src_rq);
	if (!p) {
		src_rq->active_vip_balance = 0;
		raw_spin_unlock_irq(&src_rq->lock);
		return 0;
	}

	/* find a free-cpu */
#ifdef CONFIG_HW_EAS_SCHED
	hw_get_fast_cpus(&cluster);
	hw_get_slow_cpus(&slow_cpu_mask);
	if (cpumask_test_cpu(src_cpu, &cluster))
		cpumask_and(&cpus_allowed, &cluster, &(p->cpus_allowed));
	else
		cpumask_and(&cpus_allowed,
			&slow_cpu_mask, &(p->cpus_allowed));
	raw_spin_unlock(&src_rq->lock);
	dst_rq = get_dest_rq(src_cpu, &cpus_allowed, &i);
#elif defined(CONFIG_HISI_EAS_SCHED)
	hisi_get_fast_cpus(&cluster);
	hisi_get_slow_cpus(&slow_cpu_mask);
	if (cpumask_test_cpu(src_cpu, &cluster))
		cpumask_and(&cpus_allowed, &cluster, &(p->cpus_allowed));
	else
		cpumask_and(&cpus_allowed,
			&slow_cpu_mask, &(p->cpus_allowed));
	raw_spin_unlock(&src_rq->lock);
	dst_rq = get_dest_rq(src_cpu, &cpus_allowed, &i);
#else
	raw_spin_unlock(&src_rq->lock);
	dst_rq = get_dest_rq(src_cpu, &(p->cpus_allowed), &i);
#endif
	migrate_task(src_rq, dst_rq, p, i);

	local_irq_enable();
	return 0;
}

void trigger_vip_balance(struct rq *rq)
{
	struct task_struct *task = NULL;
	int active_vip_balance = 0;

	if (!rq)
		return;

	raw_spin_lock(&rq->lock);
	task = check_vip_delayed(rq);
	/*
	 * active_vip_balance synchronized accesss to vip_balance_work
	 * 1 means vip_balance_work is ongoing, and reset to 0 after
	 * vip_balance_work is done
	 */
	if (!rq->active_vip_balance && task) {
		active_vip_balance = 1;
		rq->active_vip_balance = 1;
	}
	raw_spin_unlock(&rq->lock);
	if (active_vip_balance)
		stop_one_cpu_nowait(cpu_of(rq),
			__do_vip_balance, rq, &rq->vip_balance_work);
}

void vip_init_rq_data(struct rq *rq)
{
	if (!rq)
		return;

	rq->active_vip_balance = 0;
	INIT_LIST_HEAD(&rq->vip_thread_list);
#ifdef CONFIG_VIP_SCHED_OPT
	rq->vip_preempt_type = NO_VIP_PREEMPT;
	rq->highest_vip_prio = 0;
#endif
}

#ifdef CONFIG_VIP_SCHED_OPT
void set_thread_vip_prio(struct task_struct *p, unsigned int prio)
{
	struct rq_flags rf;
	struct rq *rq = NULL;
	bool queued = false;

	if (p == NULL || p->vip_prio == prio)
		return;

	if (p->sched_class == &fair_sched_class) {
		rq = task_rq_lock(p, &rf);
		queued = task_on_rq_queued(p);
		if (queued) {
			update_rq_clock(rq);
			dequeue_vip_thread(rq, p);
		}

		get_task_struct(p);
		p->vip_prio = prio;
		put_task_struct(p);

		if (queued)
			enqueue_vip_thread(rq, p);
		task_rq_unlock(rq, p, &rf);
	} else {
		get_task_struct(p);
		p->vip_prio = prio;
		put_task_struct(p);
	}
}
#endif
/*lint -restore*/
#endif
