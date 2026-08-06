/*
 * rotation.c
 *
 * sched running task rotation
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <linux/cpumask.h>
#include <linux/topology.h>
#include <linux/percpu.h>
#include <linux/math64.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <uapi/linux/sched/types.h>
#include <trace/events/sched.h>

#include "sched.h"


struct rotate_work {
	struct work_struct w;
	struct task_struct *src_task;
	struct task_struct *dst_task;
	int src_cpu;
	int dst_cpu;
};

static DEFINE_PER_CPU(struct rotate_work, rotate_works);

unsigned int sysctl_sched_rotate_big_tasks = 1;
unsigned int rotation_enabled;

static void rotate_work_func(struct work_struct *work)
{
	struct rotate_work *wr = container_of(work,
				struct rotate_work, w);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	migrate_swap(wr->src_task, wr->dst_task, wr->dst_cpu, wr->src_cpu);
#else
	migrate_swap(wr->src_task, wr->dst_task);
#endif

	put_task_struct(wr->src_task);
	put_task_struct(wr->dst_task);

	clear_reserved(wr->src_cpu);
	clear_reserved(wr->dst_cpu);
}

void rotate_work_init(int cpu)
{
	struct rotate_work *wr = &per_cpu(rotate_works, cpu);

	INIT_WORK(&wr->w, rotate_work_func);
}

void rotation_checkpoint(bool sched_avg_updated)
{
	unsigned int nr_big;

	if (!sysctl_sched_rotate_big_tasks ||
	    num_online_cpus() <= 1) {
		rotation_enabled = 0;
		return;
	}

	if (!sched_avg_updated)
		return;

	nr_big = sched_get_cpus_real_big_tasks(cpu_possible_mask);
	rotation_enabled = nr_big >= num_possible_cpus();

	trace_rotation_checkpoint(nr_big, rotation_enabled);
}

#define ROTATION_THRESHOLD_NS	19000000
void check_for_rotation(struct rq *src_rq)
{
	u64 wallclock, wait, run;
	u64 max_wait = 0;
	u64 max_run = 0;
	int deserved_cpu = nr_cpu_ids;
	int dst_cpu = nr_cpu_ids;
	int i, src_cpu = cpu_of(src_rq);
	struct rq *dst_rq = NULL;
	struct rotate_work *wr = NULL;
	struct cpumask slow_cpus;

	if (!rotation_enabled)
		return;

	hisi_get_slow_cpus(&slow_cpus);

	if (!cpumask_test_cpu(src_cpu, &slow_cpus))
		return;

	wallclock = walt_ktime_clock();

	for_each_cpu(i, &slow_cpus) {
		struct rq *rq = cpu_rq(i);

		if (is_reserved(i))
			continue;

		if (!rq->misfit_task_load ||
		    rq->curr->sched_class != &fair_sched_class)
			continue;

		wait = wallclock - rq->curr->last_enqueued_ts;
		if (wait > max_wait) {
			max_wait = wait;
			deserved_cpu = i;
		}
	}

	if (deserved_cpu != src_cpu)
		return;

	for_each_cpu_not(i, &slow_cpus) {
		struct rq *rq = cpu_rq(i);

		if (is_reserved(i))
			continue;

		if (rq->curr->sched_class != &fair_sched_class)
			continue;

		if (rq->nr_running > 1)
			continue;

		run = wallclock - rq->curr->last_enqueued_ts;

		if (run < ROTATION_THRESHOLD_NS)
			continue;

		if (run > max_run) {
			max_run = run;
			dst_cpu = i;
		}
	}

	if (dst_cpu == nr_cpu_ids)
		return;

	dst_rq = cpu_rq(dst_cpu);

	double_rq_lock(src_rq, dst_rq);
	if (dst_rq->curr->sched_class == &fair_sched_class) {
		get_task_struct(src_rq->curr);
		get_task_struct(dst_rq->curr);

		mark_reserved(src_cpu);
		mark_reserved(dst_cpu);
		wr = &per_cpu(rotate_works, src_cpu);

		wr->src_task = src_rq->curr;
		wr->dst_task = dst_rq->curr;

		wr->src_cpu = src_cpu;
		wr->dst_cpu = dst_cpu;
	}
	double_rq_unlock(src_rq, dst_rq);

	if (wr)
		queue_work_on(src_cpu, system_highpri_wq, &wr->w);
}
