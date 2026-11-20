#include "dubai_cpu_stats_common.h"

#include <linux/cpufreq.h>
#include <linux/cpuidle.h>
#include <linux/slab.h>

#include "../utils/dubai_utils.h"

static struct dubai_cpu_config_info *g_cpuinfo = NULL;

static inline size_t dubai_get_cpuinfo_size(void)
{
	return nr_cpu_ids * sizeof(struct dubai_cpu_config_info);
}

static struct dubai_cpu_config_info *dubai_alloc_cpuinfo(void)
{
	struct dubai_cpu_config_info *info = NULL;

	info = kzalloc(dubai_get_cpuinfo_size(), GFP_KERNEL);
	if (!info)
		dubai_err("Failed to allocate memory");

	return info;
}

int dubai_get_cpu_configs(void __user *argp)
{
	if (!g_cpuinfo)
		return -EINVAL;

	if (copy_to_user(argp, g_cpuinfo, dubai_get_cpuinfo_size()))
		return -EFAULT;

	return 0;
}

int dubai_cpu_configs_init(void)
{
	int freq_count, cpu;
	struct dubai_cpu_config_info *info = NULL;
	struct cpufreq_policy *policy = NULL;
	struct cpufreq_frequency_table *pos = NULL, *table = NULL;
	struct cpuidle_device *idle_dev = NULL;
	struct cpuidle_driver *idle_drv = NULL;

	info = dubai_alloc_cpuinfo();
	if (!info) {
		dubai_err("Failed to allocate cpu info");
		return -ENOMEM;
	}

	for_each_possible_cpu(cpu) {
		policy = cpufreq_cpu_get(cpu);
		idle_dev = per_cpu(cpuidle_devices, cpu);
		idle_drv = cpuidle_get_cpu_driver(idle_dev);
		if (!policy || !policy->freq_table || !idle_drv) {
			dubai_err("Invalid cpufreq policy or idle device: %d", cpu);
			goto err_exit;
		}

		freq_count = cpufreq_table_count_valid_entries(policy);
		if ((freq_count <= 0) || (freq_count > MAX_CPU_FREQ_NR)) {
			dubai_err("Invalid cpu freq count: %d", freq_count);
			goto err_exit;
		}

		info[cpu].freq_policy_cpu = policy->cpu;
		info[cpu].idle_nr = idle_drv->state_count;
		info[cpu].freq_nr = freq_count;
		table = policy->freq_table;
		cpufreq_for_each_valid_entry(pos, table)
			info[cpu].avail_freqs[pos - table] = pos->frequency;
	}
	g_cpuinfo = info;

	return 0;
err_exit:
	kfree(info);

	return -EFAULT;
}

void dubai_cpu_configs_exit(void)
{
	if (g_cpuinfo)
		kfree(g_cpuinfo);
	g_cpuinfo = NULL;
}
