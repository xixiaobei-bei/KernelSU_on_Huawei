/*
 * sched_avg.c
 *
 * sched running average statistics
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
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/topology.h>
#include <linux/kthread.h>
#include <linux/percpu.h>
#include <linux/math64.h>
#include <uapi/linux/sched/types.h>

#include "sched.h"

static DEFINE_PER_CPU(u64, nr_prod_sum);
static DEFINE_PER_CPU(u64, nr_big_prod_sum);
static DEFINE_PER_CPU(u64, iowait_prod_sum);
static DEFINE_PER_CPU(u64, last_time);
static DEFINE_PER_CPU(u64, nr);
static DEFINE_PER_CPU(u64, nr_max);
static DEFINE_PER_CPU(struct sched_nr_stat, stat);
static DEFINE_PER_CPU(spinlock_t, nr_lock) = __SPIN_LOCK_UNLOCKED(nr_lock);

static unsigned int g_running_avg_update_interval_ms = UPDATE_INIT_20MS;
static s64 g_running_avg_update_timestamp_ms;
static u64 g_last_get_time;

void request_running_avg_update_ms(unsigned int update_ms)
{
	g_running_avg_update_interval_ms = update_ms;
}

bool sched_update_running_avg(void)
{
	int cpu;
	s64 now;
	u64 diff, curr_time, period;
	u64 tmp_avg, tmp_iowait, tmp_big_avg;

	now = ktime_to_ms(ktime_get());
	if (now < g_running_avg_update_timestamp_ms + g_running_avg_update_interval_ms)
		return false;

	g_running_avg_update_timestamp_ms = now;

	curr_time = sched_clock();
	period = curr_time - g_last_get_time;
	if (!period)
		return true;

	/* read and reset nr_running counts */
	for_each_possible_cpu(cpu) {
		unsigned long flags;
		struct sched_nr_stat *cpu_stat = &per_cpu(stat, cpu);

		spin_lock_irqsave(&per_cpu(nr_lock, cpu), flags);
		curr_time = sched_clock();
		diff = curr_time - per_cpu(last_time, cpu);
		if ((s64)diff < 0)
			diff = 0;

		tmp_avg = per_cpu(nr_prod_sum, cpu);
		tmp_avg += per_cpu(nr, cpu) * diff;

		tmp_big_avg = per_cpu(nr_big_prod_sum, cpu);
		tmp_big_avg += cpu_rq(cpu)->nr_heavy_running * diff;

		tmp_iowait = per_cpu(iowait_prod_sum, cpu);
		tmp_iowait += nr_iowait_cpu(cpu) * diff;

		cpu_stat->avg = (unsigned int)div64_u64(tmp_avg * NR_AVG_AMP, period);
		cpu_stat->big_avg = (unsigned int)div64_u64(tmp_big_avg * NR_AVG_AMP, period);
		cpu_stat->iowait_avg = (unsigned int)div64_u64(tmp_iowait * NR_AVG_AMP,
							   period);
		cpu_stat->nr_max = (unsigned int)per_cpu(nr_max, cpu);

		per_cpu(last_time, cpu) = curr_time;

		per_cpu(nr_prod_sum, cpu) = 0;
		per_cpu(nr_big_prod_sum, cpu) = 0;
		per_cpu(iowait_prod_sum, cpu) = 0;
		per_cpu(nr_max, cpu) = per_cpu(nr, cpu);

		spin_unlock_irqrestore(&per_cpu(nr_lock, cpu), flags);
	}
	g_last_get_time = curr_time;

	return true;
}

/*
 * sched_get_cpus_running_avg
 * @return: Average nr_running, iowait and nr_big_tasks value since last poll.
 *	    Returns the avg * NR_AVG_AMP to return up to two decimal points
 *	    of accuracy.
 *
 * Obtains the average nr_running value since the last poll.
 * This function may not be called concurrently with itself
 */
void sched_get_cpus_running_avg(const struct cpumask *cpus,
				struct sched_nr_stat *nr_stat)
{
	int cpu;
	unsigned int tmp_avg = 0;
	unsigned int tmp_iowait = 0;
	unsigned int tmp_big_avg = 0;
	unsigned int tmp_nr_max = 0;

	if (!cpus || !nr_stat)
		return;

	for_each_cpu(cpu, cpus) {
		struct sched_nr_stat *cpu_stat = &per_cpu(stat, cpu);

		if (tmp_nr_max < cpu_stat->nr_max)
			tmp_nr_max = cpu_stat->nr_max;

		tmp_avg += cpu_stat->avg;
		tmp_iowait += cpu_stat->iowait_avg;
		tmp_big_avg += cpu_stat->big_avg;
	}

	nr_stat->avg = tmp_avg;
	nr_stat->big_avg = tmp_big_avg;
	nr_stat->iowait_avg = tmp_iowait;
	nr_stat->nr_max = tmp_nr_max;
}

/*
 * sched_update_nr_prod
 * @updated_rq: The updated runqueue.
 * @delta: Adjust nr by 'delta' amount
 * @inc: Whether we are increasing or decreasing the count
 * @return: N/A
 *
 * Update average with latest nr_running value for CPU
 */
void sched_update_nr_prod(struct rq *updated_rq)
{
	int cpu = cpu_of(updated_rq);
	u64 diff;
	u64 curr_time;
	unsigned long flags;
	unsigned long prev_nr_running;

	spin_lock_irqsave(&per_cpu(nr_lock, cpu), flags);

	prev_nr_running = per_cpu(nr, cpu);
	per_cpu(nr, cpu) = updated_rq->nr_running;
	curr_time = sched_clock();
	diff = curr_time - per_cpu(last_time, cpu);
	per_cpu(last_time, cpu) = curr_time;

	if (per_cpu(nr, cpu) > per_cpu(nr_max, cpu))
		per_cpu(nr_max, cpu) = per_cpu(nr, cpu);

	if ((s64)diff > 0) {
		per_cpu(nr_prod_sum, cpu) += prev_nr_running * diff;
		per_cpu(nr_big_prod_sum, cpu) +=
			updated_rq->nr_heavy_running * diff;
		per_cpu(iowait_prod_sum, cpu) += nr_iowait_cpu(cpu) * diff;
	}

	spin_unlock_irqrestore(&per_cpu(nr_lock, cpu), flags);
}
EXPORT_SYMBOL(sched_update_nr_prod);

unsigned int sched_get_cpus_real_big_tasks(const struct cpumask *cpus)
{
	int cpu;
	unsigned int nr_big = 0;
	unsigned int nr_cpus = 0;

	if (!cpus)
		return 0;

	for_each_cpu(cpu, cpus) {
		struct sched_nr_stat *cpu_stat = &per_cpu(stat, cpu);

		if (test_slow_cpu(cpu))
			nr_big += cpu_stat->big_avg;
		else
			nr_big += cpu_stat->avg;

		nr_cpus++;
	}

	nr_big = DIV_ROUND_CLOSEST(nr_big, NR_AVG_AMP);
	return min(nr_big, nr_cpus);
}
