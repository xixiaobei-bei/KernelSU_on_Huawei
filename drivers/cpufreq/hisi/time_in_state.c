/*
 * time_in_state.c
 *
 * counting different C-state time in each freq
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

#include <linux/types.h>
#include <linux/cpu.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/percpu-defs.h>
#include <linux/spinlock.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/cpuidle.h>

struct time_in_state {
	spinlock_t lock;
	ktime_t last_update;
	unsigned int *freq_table;
	u64 *time;
	unsigned int max_freq_state;
	unsigned int max_idle_state;
	/* 0=active 1..idle states */
	unsigned int cur_idle_idx;
	/* 0~freq_num-1 */
	unsigned int cur_freq_idx;
	bool initialized;
};

static DEFINE_PER_CPU(struct time_in_state, stats_info);

/* called with icpu->lock held */
static void update_cur_state(struct time_in_state *icpu)
{
	ktime_t now;
	u64 delta_time;
	unsigned int pos;

	now = ktime_get();
	delta_time = ktime_us_delta(now, icpu->last_update);
	pos = icpu->cur_idle_idx * icpu->max_freq_state + icpu->cur_freq_idx;
	icpu->time[pos] += delta_time;
	icpu->last_update = now;
}

ssize_t hisi_time_in_state_show(int cpu, char *buf)
{
	unsigned int i, j, pos;
	ssize_t len = 0;
	struct time_in_state *icpu = NULL;

	if (cpu < 0 || cpu >= nr_cpu_ids || buf == NULL) //lint !e574
		return -EINVAL;

	icpu = per_cpu_ptr(&stats_info, cpu);
	if (!icpu->initialized)
		return -ENODEV;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
	spin_unlock(&icpu->lock);

	for (i = 0; i < icpu->max_freq_state; i++) {
		len += scnprintf(buf + len, PAGE_SIZE - len, "%u", icpu->freq_table[i]);
		for (j = 0; j < icpu->max_idle_state; j++) {
			pos = j * icpu->max_freq_state + i;
			len += scnprintf(buf + len, PAGE_SIZE - len, " %llu", icpu->time[pos]);
		}

		len += scnprintf(buf + len, PAGE_SIZE - len, "\n");
	}

	return len;
}

int hisi_time_in_freq_get(int cpu, u64 *freqs, int freqs_len)
{
	unsigned int i, j, pos;
	struct time_in_state *icpu = NULL;

	if (cpu < 0 || cpu >= nr_cpu_ids || freqs == NULL)
		return -EINVAL;

	icpu = per_cpu_ptr(&stats_info, cpu);
	if (!icpu->initialized)
		return -ENODEV;

	if (freqs_len != icpu->max_freq_state)
		return -EINVAL;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
	spin_unlock(&icpu->lock);
	for (i = 0; i < freqs_len; i++) {
		freqs[i] = 0;
		for (j = 0; j < icpu->max_idle_state; j++) {
			pos = j * icpu->max_freq_state + i;
			freqs[i] += icpu->time[pos];
		}
	}

	return 0;
}

void time_in_state_update_idle(int cpu, unsigned int new_idle_index)
{
	struct time_in_state *icpu = NULL;

	if (cpu < 0 || cpu >= nr_cpu_ids) //lint !e574
		return;

	icpu = per_cpu_ptr(&stats_info, cpu);
	if (!icpu->initialized || new_idle_index >= icpu->max_idle_state)
		return;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
	icpu->cur_idle_idx = new_idle_index;
	spin_unlock(&icpu->lock);
}

/* update the time of old freq when freq change */
void time_in_state_update_freq(struct cpumask *cpus, unsigned int new_freq_index)
{
	int cpu;
	struct time_in_state *icpu = NULL;

	if (cpus == NULL || cpumask_empty(cpus))
		return;

	for_each_cpu(cpu, cpus) { //lint !e574
		icpu = per_cpu_ptr(&stats_info, cpu);

		if (!icpu->initialized || new_freq_index >= icpu->max_freq_state)
			continue;

		spin_lock(&icpu->lock);
		update_cur_state(icpu);
		icpu->cur_freq_idx = new_freq_index;
		spin_unlock(&icpu->lock);
	}
}

static int __init time_in_state_init(void)
{
	int cpu, ret, freq_index;
	struct time_in_state *icpu = NULL;
	struct cpuidle_device *dev = NULL;
	struct cpuidle_driver *drv = NULL;
	struct cpufreq_policy policy;
	unsigned int alloc_size, i;
	struct cpufreq_frequency_table *pos = NULL;

	for_each_online_cpu(cpu) { //lint !e574
		icpu = per_cpu_ptr(&stats_info, cpu);

		if (icpu->initialized)
			continue;

		/* init idle */
		dev = per_cpu(cpuidle_devices, cpu);
		drv = cpuidle_get_cpu_driver(dev);
		/* all idle states plus active state */
		icpu->max_idle_state = (drv != NULL) ? drv->state_count + 1 : 2;
		icpu->cur_idle_idx = 0;

		/* init freq */
		ret = cpufreq_get_policy(&policy, cpu);
		if (ret != 0)
			continue;

		i = 0;
		cpufreq_for_each_valid_entry(pos, policy.freq_table)
			i++;
		if (i == 0)
			continue;

		icpu->max_freq_state = i;
		alloc_size = i * sizeof(int) + (icpu->max_idle_state * i * sizeof(u64)); //lint !e647
		icpu->freq_table = kzalloc(alloc_size, GFP_KERNEL);
		if (icpu->freq_table == NULL) {
			pr_err("time in stats alloc fail for cpu:%d\n", cpu);
			goto err_out;
		}
		icpu->time = (u64 *)(icpu->freq_table + icpu->max_freq_state);

		i = 0;
		cpufreq_for_each_valid_entry(pos, policy.freq_table)
			icpu->freq_table[i++] = pos->frequency;

		freq_index = cpufreq_frequency_table_target(&policy, policy.cur, CPUFREQ_RELATION_C);
		if (freq_index < 0)
			continue;
		icpu->cur_freq_idx = freq_index;

		spin_lock_init(&icpu->lock);
		icpu->last_update = ktime_get();
		icpu->initialized = true;
	}

	return 0;

err_out:
	for_each_online_cpu(cpu) { //lint !e574
		icpu = per_cpu_ptr(&stats_info, cpu);

		if (!icpu->initialized)
			continue;

		icpu->initialized = false;
		kfree(icpu->freq_table);
		icpu->freq_table = NULL;
	}

	return 0;
}
late_initcall(time_in_state_init);
