/*
 * arm-memlat-mon.c
 *
 * arm memory latency monitor driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt) "arm-memlat-mon: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/cpu_pm.h>
#include <linux/cpu.h>
#include "governor.h"
#include "governor_memlat.h"
#include <linux/perf_event.h>
#include <linux/version.h>
#include <linux/sched/cpufreq.h>

enum ev_index {
	INST_IDX,
	CYC_IDX,
	NUM_COMMON_EVS,
};

/* stop work in lazy mode */
enum mon_work_mode {
	NORMAL_MODE,
	LAZY_MODE,
	NUM_WORK_MODE,
};

#define INST_EV		0x08
#define L2DM_EV		0x17
#define CYC_EV		0x11

#define DEFAULT_UPDATE_MS 100
#define LAZY_HYST_MS 100

struct event_data {
	struct perf_event *pevent;
	unsigned long prev_count;
	unsigned long last_delta;
};

struct cpu_pmu_stats {
	struct event_data common_evs[NUM_COMMON_EVS];
	unsigned long freq;
};

struct memlat_mon {
	bool is_active;

	unsigned int miss_ev_id;
	struct event_data *miss_ev;

	struct memlat_hwmon hw;
	unsigned int requested_update_ms;
	struct cpu_grp_info *cpu_grp;

	/* link to mon_list */
	struct list_head mon_node;
};

struct cpu_grp_info {
	cpumask_t cpus;
	/* track online and pmu-inited cpus */
	cpumask_t inited_cpus;
	unsigned int common_ev_ids[NUM_COMMON_EVS];
	struct cpu_pmu_stats *cpustats;

	ktime_t last_update_ts;

	struct delayed_work work;
	unsigned int update_ms;

	struct mutex mons_lock;
	unsigned int num_active_mons;
	/* memlat monitor list who shares this cpu grp */
	struct list_head mon_list;

	enum mon_work_mode mode;
	ktime_t mon_invoke_ts;
	bool lazy_mode_enable;
};

#define to_cpustats(cpu_grp, cpu) \
	(&(cpu_grp)->cpustats[(cpu) - cpumask_first(&(cpu_grp)->cpus)])
#define to_devstats(hw, cpu) \
	(&(hw)->core_stats[(cpu) - cpumask_first(&(hw)->cpus)])
#define to_mon(hwmon) container_of((hwmon), struct memlat_mon, hw)
#define to_common_evs(cpu_grp, cpu) \
	((cpu_grp)->cpustats[(cpu) - cpumask_first(&(cpu_grp)->cpus)].common_evs)

static DEFINE_PER_CPU(struct cpu_grp_info *, grp_info);
static struct workqueue_struct *memlat_wq;

static inline unsigned long compute_freq(unsigned long cyc_cnt,
					 unsigned long diff)
{
	unsigned long freq = cyc_cnt;

	if (diff == 0)
		diff = 1;

	do_div(freq, diff);
	return freq;
}

#define MAX_COUNT_LIM 0xFFFFFFFFFFFFFFFF
static inline void read_event(struct event_data *event)
{
	unsigned long ev_count;
	u64 total, running;
	u64 enabled;

	total = perf_event_read_value(event->pevent, &enabled, &running);
	if (total >= event->prev_count)
		ev_count = total - event->prev_count;
	else
		ev_count = (MAX_COUNT_LIM - event->prev_count) + total;

	event->prev_count = total;
	event->last_delta = ev_count;
}

static void update_counts(struct cpu_grp_info *cpu_grp)
{
	int i, cpu;
	struct memlat_mon *mon = NULL;
	ktime_t now = ktime_get();
	unsigned long delta = ktime_us_delta(now, cpu_grp->last_update_ts);

	cpu_grp->last_update_ts = now;

	/* read common events */
	for_each_cpu(cpu, &cpu_grp->inited_cpus) {
		struct cpu_pmu_stats *cpustats = to_cpustats(cpu_grp, cpu);
		struct event_data *common_evs = cpustats->common_evs;

		for (i = 0; i < NUM_COMMON_EVS; i++)
			read_event(&common_evs[i]);

		cpustats->freq = compute_freq(common_evs[CYC_IDX].last_delta,
					      delta);
	}

	/* read cache miss events */
	list_for_each_entry(mon, &cpu_grp->mon_list, mon_node) {
		if (!mon->is_active)
			continue;

		for_each_cpu(cpu, &cpu_grp->inited_cpus) {
			unsigned int mon_idx =
				cpu - cpumask_first(&cpu_grp->cpus);
			read_event(&mon->miss_ev[mon_idx]);
		}
	}
}

static unsigned long get_cnt(struct memlat_hwmon *hw)
{
	struct memlat_mon *mon = to_mon(hw);
	struct cpu_grp_info *cpu_grp = mon->cpu_grp;
	int cpu;

	for_each_cpu(cpu, &cpu_grp->inited_cpus) {
		struct cpu_pmu_stats *cpustats = to_cpustats(cpu_grp, cpu);
		struct event_data *common_evs = cpustats->common_evs;
		unsigned int mon_idx =
			cpu - cpumask_first(&cpu_grp->cpus);
		struct dev_stats *devstats = to_devstats(hw, cpu);

		devstats->freq = cpustats->freq;
		devstats->inst_count = common_evs[INST_IDX].last_delta;
		devstats->mem_count = mon->miss_ev[mon_idx].last_delta;
	}

	return 0;
}

static struct perf_event_attr *alloc_attr(void)
{
	struct perf_event_attr *attr = NULL;

	attr = kzalloc(sizeof(struct perf_event_attr), GFP_KERNEL);
	if (attr == NULL)
		return attr;

	attr->type = PERF_TYPE_RAW;
	attr->size = sizeof(struct perf_event_attr);
	attr->pinned = 1;
	attr->exclude_idle = 1;

	return attr;
}

static int set_event(struct event_data *ev, int cpu, unsigned int event_id,
		     struct perf_event_attr *attr)
{
	struct perf_event *pevent = NULL;

	if (!event_id)
		return 0;

	attr->config = event_id;
	pevent = perf_event_create_kernel_counter(attr, cpu, NULL, NULL, NULL);
	if (IS_ERR(pevent))
		return PTR_ERR(pevent);

	ev->pevent = pevent;
	perf_event_enable(pevent);

	return 0;
}

static void delete_event(struct event_data *event)
{
	event->prev_count = 0;
	event->last_delta = 0;
	if (event->pevent) {
		perf_event_release_kernel(event->pevent);
		event->pevent = NULL;
	}
}

static int init_common_evs(struct cpu_grp_info *cpu_grp,
			   int cpu, struct perf_event_attr *attr)
{
	int i, j;
	int ret;
	struct event_data *common_evs = to_common_evs(cpu_grp, cpu);

	for (i = 0; i < NUM_COMMON_EVS; i++) {
		ret = set_event(&common_evs[i], cpu,
				cpu_grp->common_ev_ids[i], attr);
		if (ret)
			goto err;
	}

	return 0;

err:
	pr_err("fail to create %d event of cpu %d\n", i, cpu);
	for (j = 0; j < i; j++)
		delete_event(&common_evs[j]);

	return ret;
}

static void free_common_evs(struct cpu_grp_info *cpu_grp, int cpu)
{
	int i;
	struct event_data *common_evs = to_common_evs(cpu_grp, cpu);

	for (i = 0; i < NUM_COMMON_EVS; i++)
		delete_event(&common_evs[i]);
}

static int __ref arm_memlat_cpu_online(unsigned int cpu)
{
	int ret;
	struct memlat_mon *mon = NULL;
	struct cpu_grp_info *cpu_grp = per_cpu(grp_info, cpu);
	struct perf_event_attr *attr = NULL;
	unsigned int idx;

	if (cpu_grp == NULL)
		goto out;

	mutex_lock(&cpu_grp->mons_lock);
	if (cpumask_test_cpu(cpu, &cpu_grp->inited_cpus))
		goto unlock;

	/* all memlat mon stopped, do nothing */
	if (!cpu_grp->num_active_mons)
		goto unlock;

	attr = alloc_attr();
	if (attr == NULL)
		goto unlock;

	/* common events init */
	ret = init_common_evs(cpu_grp, cpu, attr);
	if (ret)
		goto free;

	/* all memlat monitor cache miss event init */
	idx = cpu - cpumask_first(&cpu_grp->cpus);
	list_for_each_entry(mon, &cpu_grp->mon_list, mon_node) {
		if (!mon->is_active)
			continue;

		ret = set_event(&mon->miss_ev[idx], cpu,
				mon->miss_ev_id, attr);
		if (ret) {
			free_common_evs(cpu_grp, cpu);
			goto free;
		}
	}

	cpumask_set_cpu(cpu, &cpu_grp->inited_cpus);

free:
	kfree(attr);
unlock:
	mutex_unlock(&cpu_grp->mons_lock);
out:
	/* always return 0 for not break hotplug */
	return 0;
}

static int __ref arm_memlat_cpu_offline(unsigned int cpu)
{
	struct memlat_mon *mon = NULL;
	struct cpu_grp_info *cpu_grp = per_cpu(grp_info, cpu);
	unsigned int idx;

	if (cpu_grp == NULL)
		goto out;

	mutex_lock(&cpu_grp->mons_lock);
	if (!cpumask_test_cpu(cpu, &cpu_grp->inited_cpus))
		goto unlock;

	/* all memlat mon stopped, do nothing */
	if (!cpu_grp->num_active_mons)
		goto unlock;

	/* all memlat monitor cache miss event delete */
	idx = cpu - cpumask_first(&cpu_grp->cpus);
	list_for_each_entry(mon, &cpu_grp->mon_list, mon_node) {
		struct memlat_hwmon *hw = &mon->hw;
		struct dev_stats *devstats = to_devstats(hw, cpu);

		if (!mon->is_active)
			continue;

		delete_event(&mon->miss_ev[idx]);

		/* Clear governor data of this cpu */
		devstats->inst_count = 0;
		devstats->mem_count = 0;
		devstats->freq = 0;
	}

	/* common events delete */
	free_common_evs(cpu_grp, cpu);
	cpumask_clear_cpu(cpu, &cpu_grp->inited_cpus);

unlock:
	mutex_unlock(&cpu_grp->mons_lock);
out:
	/* always return 0 for not break hotplug */
	return 0;
}

/* Called with cpu_grp->mons_lock taken */
static unsigned int get_min_request_ms(struct cpu_grp_info *cpu_grp)
{
	struct memlat_mon *mon = NULL;
	unsigned int new_update_ms = UINT_MAX;

	list_for_each_entry(mon, &cpu_grp->mon_list, mon_node) {
		if (mon->is_active && mon->requested_update_ms)
			new_update_ms =
				min(new_update_ms, mon->requested_update_ms);
	}

	return new_update_ms;
}

static void request_update_ms(struct memlat_hwmon *hw, unsigned int update_ms)
{
	struct devfreq *df = hw->df;
	struct memlat_mon *mon = to_mon(hw);
	struct cpu_grp_info *cpu_grp = mon->cpu_grp;
	unsigned int new_update_ms;

	mutex_lock(&df->lock);
	df->profile->polling_ms = update_ms;
	mutex_unlock(&df->lock);

	mutex_lock(&cpu_grp->mons_lock);
	mon->requested_update_ms = update_ms;

	new_update_ms = get_min_request_ms(cpu_grp);
	cpu_grp->update_ms = new_update_ms;
	mutex_unlock(&cpu_grp->mons_lock);
}

static inline bool should_enter_lazy_mode(struct cpu_grp_info *cpu_grp)
{
	ktime_t now = ktime_get();

	if (cpu_grp->lazy_mode_enable &&
	    cpu_grp->mode != LAZY_MODE &&
	    ktime_ms_delta(now, cpu_grp->mon_invoke_ts) > LAZY_HYST_MS)
		return true;

	return false;
}

static void memlat_monitor_work(struct work_struct *work)
{
	int err;
	struct cpu_grp_info *cpu_grp =
		container_of(work, struct cpu_grp_info, work.work);
	struct memlat_mon *mon = NULL;
	bool vote_min = true;

	mutex_lock(&cpu_grp->mons_lock);
	if (!cpu_grp->num_active_mons)
		goto unlock;

	update_counts(cpu_grp);
	list_for_each_entry(mon, &cpu_grp->mon_list, mon_node) {
		struct devfreq *df = NULL;

		if (!mon->is_active)
			continue;

		df = mon->hw.df;
		mutex_lock(&df->lock);
		err = update_devfreq(df);
		if (df->previous_freq > df->min_freq)
			vote_min = false;
		mutex_unlock(&df->lock);
		if (err)
			dev_err(mon->hw.dev, "Memlat update failed: %d\n", err);
	}

	/* check enter lazy mode by itself */
	if (vote_min) {
		if (should_enter_lazy_mode(cpu_grp)) {
			cpu_grp->mode = LAZY_MODE;

			goto unlock;
		}
	}

	queue_delayed_work(memlat_wq, &cpu_grp->work,
			   msecs_to_jiffies(cpu_grp->update_ms));

unlock:
	mutex_unlock(&cpu_grp->mons_lock);
}

static int start_hwmon(struct memlat_hwmon *hw)
{
	int cpu;
	int ret = 0;
	struct memlat_mon *mon = to_mon(hw);
	struct cpu_grp_info *cpu_grp = mon->cpu_grp;
	bool should_init_cpu_grp = false;
	struct perf_event_attr *attr = alloc_attr();

	if (attr == NULL)
		return -ENOMEM;

	get_online_cpus();
	mutex_lock(&cpu_grp->mons_lock);
	if (mon->is_active)
		goto unlock;

	should_init_cpu_grp = !cpu_grp->num_active_mons;
	cpu_grp->num_active_mons++;
	for_each_cpu(cpu, &cpu_grp->cpus) {
		unsigned int idx = cpu - cpumask_first(&cpu_grp->cpus);

		if (!cpu_online(cpu))
			continue;

		/* common events init */
		if (should_init_cpu_grp) {
			ret = init_common_evs(cpu_grp, cpu, attr);
			if (ret)
				goto unlock;
		}

		/* cache miss event init */
		ret = set_event(&mon->miss_ev[idx], cpu,
				mon->miss_ev_id, attr);
		if (ret) {
			if (should_init_cpu_grp)
				free_common_evs(cpu_grp, cpu);

			goto unlock;
		}

		cpumask_set_cpu(cpu, &cpu_grp->inited_cpus);
	}

	mon->is_active = true;

	if (should_init_cpu_grp)
		queue_delayed_work(memlat_wq, &cpu_grp->work,
				   msecs_to_jiffies(cpu_grp->update_ms));

unlock:
	mutex_unlock(&cpu_grp->mons_lock);
	put_online_cpus();

	kfree(attr);

	return ret;
}

static void stop_hwmon(struct memlat_hwmon *hw)
{
	int cpu;
	struct memlat_mon *mon = to_mon(hw);
	struct cpu_grp_info *cpu_grp = mon->cpu_grp;

	get_online_cpus();
	mutex_lock(&cpu_grp->mons_lock);
	if (!mon->is_active)
		goto unlock;

	mon->is_active = false;
	cpu_grp->num_active_mons--;

	for_each_cpu(cpu, &cpu_grp->inited_cpus) {
		unsigned int idx = cpu - cpumask_first(&cpu_grp->cpus);
		struct dev_stats *devstats = to_devstats(hw, cpu);

		delete_event(&mon->miss_ev[idx]);

		/* Clear governor data */
		devstats->inst_count = 0;
		devstats->mem_count = 0;
		devstats->freq = 0;

		if (!cpu_grp->num_active_mons)
			free_common_evs(cpu_grp, cpu);
	}

	if (!cpu_grp->num_active_mons) {
		cancel_delayed_work(&cpu_grp->work);
		cpumask_clear(&cpu_grp->inited_cpus);
	}

unlock:
	mutex_unlock(&cpu_grp->mons_lock);
	put_online_cpus();
}

static int get_mask_from_dev_handle(struct platform_device *pdev,
				    cpumask_t *mask)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *cpu_np = NULL;
	struct device_node *mon_np = NULL;
	int i, cpu;

	cpumask_clear(mask);

	for_each_possible_cpu(cpu) {
		cpu_np = of_get_cpu_node(cpu, NULL);
		if (!cpu_np) {
			pr_err("%s: failed to get cpu%d node\n",
			       __func__, cpu);
			return -ENOENT;
		}

		/* Get memlat monitor descriptor node */
		for (i = 0; ; i++) {
			mon_np = of_parse_phandle(cpu_np, "memlat-monitors", i);
			if (!mon_np)
				break;

			/* CPUs are sharing memlat monitor node */
			if (np == mon_np) {
				cpumask_set_cpu(cpu, mask);
				of_node_put(mon_np);
				break;
			}
			of_node_put(mon_np);
		}

		of_node_put(cpu_np);
	}

	if (cpumask_empty(mask))
		return -ENOENT;

	return 0;
}

static void memlat_exit_lazy_mode(struct cpu_grp_info *cpu_grp)
{
	s64 elapsed_time;

	cpu_grp->mon_invoke_ts = ktime_get();

	if (cpu_grp->mode == NORMAL_MODE)
		return;

	cpu_grp->mode = NORMAL_MODE;
	elapsed_time = ktime_ms_delta(cpu_grp->mon_invoke_ts, cpu_grp->last_update_ts);
	/* time elapsed since last work exec smaller than update_ms */
	if (elapsed_time < cpu_grp->update_ms)
		queue_delayed_work(memlat_wq, &cpu_grp->work,
				   msecs_to_jiffies(cpu_grp->update_ms - elapsed_time));
	else
		queue_delayed_work(memlat_wq, &cpu_grp->work, 0);
}

static void memlat_lazy_mode_enable(struct cpu_grp_info *cpu_grp)
{
	mutex_lock(&cpu_grp->mons_lock);
	cpu_grp->lazy_mode_enable = true;
	mutex_unlock(&cpu_grp->mons_lock);
}

static void memlat_lazy_mode_disable(struct cpu_grp_info *cpu_grp)
{
	mutex_lock(&cpu_grp->mons_lock);
	cpu_grp->lazy_mode_enable = false;
	memlat_exit_lazy_mode(cpu_grp);
	mutex_unlock(&cpu_grp->mons_lock);
}

/* invoke monitor exit from lazy mode */
static void memlat_invoke(struct cpu_grp_info *cpu_grp)
{
	mutex_lock(&cpu_grp->mons_lock);
	memlat_exit_lazy_mode(cpu_grp);
	mutex_unlock(&cpu_grp->mons_lock);
}

static int sugov_status_notify(struct notifier_block *nb,
			       unsigned long val, void *data)
{
	int cpu = *(int *)data;
	struct cpu_grp_info *cpu_grp = NULL;

	if (cpu < 0 || cpu >= nr_cpu_ids)
		goto done;

	cpu_grp = per_cpu(grp_info, cpu);
	if (!cpu_grp)
		goto done;

	switch (val) {
	case SUGOV_START:
		memlat_lazy_mode_enable(cpu_grp);
		break;
	case SUGOV_STOP:
		memlat_lazy_mode_disable(cpu_grp);
		break;
	case SUGOV_ACTIVE:
		memlat_invoke(cpu_grp);
		break;
	default:
		break;
	}
done:
	return NOTIFY_OK;
}

static struct notifier_block memlat_nb = {
	.notifier_call = sugov_status_notify
};

static struct cpu_grp_info *cpu_grp_alloc_and_init(struct device *dev,
						   cpumask_t *cpus)
{
	struct cpu_grp_info *cpu_grp = NULL;
	u32 event_id = 0;
	int ret, cpu;

	cpu_grp = per_cpu(grp_info, cpumask_first(cpus));
	/* it is unacceptable different cpu_grps having cpus overlap and not equal */
	if (cpu_grp != NULL) {
		if (!cpumask_equal(cpus, &cpu_grp->cpus)) {
			dev_err(dev, "Wrong CPU list:%*pbl\n", cpumask_pr_args(cpus));
			return NULL;
		}

		return cpu_grp;
	}

	for_each_cpu(cpu, cpus) {
		if (per_cpu(grp_info, cpu) != cpu_grp) {
			dev_err(dev, "CPU%d already registered\n", cpu);
			return NULL;
		}
	}

	cpu_grp = devm_kzalloc(dev, sizeof(*cpu_grp), GFP_KERNEL);
	if (cpu_grp == NULL)
		return NULL;

	/* common event ids init */
	cpu_grp->common_ev_ids[CYC_IDX] = CYC_EV;

	ret = of_property_read_u32(dev->of_node, "hisi,inst-ev", &event_id);
	if (ret != 0) {
		dev_dbg(dev, "Inst event not specified. Using def:0x%x\n",
			INST_EV);
		event_id = INST_EV;
	}
	cpu_grp->common_ev_ids[INST_IDX] = event_id;

	cpu_grp->update_ms = DEFAULT_UPDATE_MS;
	mutex_init(&cpu_grp->mons_lock);
	INIT_LIST_HEAD(&cpu_grp->mon_list);

	cpu_grp->cpustats = devm_kzalloc(dev, cpumask_weight(cpus) *
					 sizeof(*cpu_grp->cpustats),
					 GFP_KERNEL);
	if (cpu_grp->cpustats == NULL)
		return NULL;

	INIT_DEFERRABLE_WORK(&cpu_grp->work, &memlat_monitor_work);

	cpumask_copy(&cpu_grp->cpus, cpus);
	for_each_cpu(cpu, &cpu_grp->cpus)
		per_cpu(grp_info, cpu) = cpu_grp;

	return cpu_grp;
}

static int arm_memlat_mon_driver_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct cpu_grp_info *cpu_grp = NULL;
	struct memlat_mon *mon = NULL;
	struct memlat_hwmon *hw = NULL;
	u32 event_id = 0;
	cpumask_t cpus;
	int cpu, ret, num_cpus;

	if (get_mask_from_dev_handle(pdev, &cpus) != 0) {
		dev_err(dev, "No CPU use this mon\n");
		return -ENODEV;
	}

	cpu_grp = cpu_grp_alloc_and_init(dev, &cpus);
	if (cpu_grp == NULL)
		return -ENOMEM;

	mon = devm_kzalloc(dev, sizeof(*mon), GFP_KERNEL);
	if (mon == NULL)
		return -ENOMEM;

	hw = &mon->hw;
	hw->dev = dev;
	hw->of_node = of_parse_phandle(dev->of_node, "hisi,target-dev", 0);
	if (hw->of_node == NULL) {
		dev_err(dev, "Couldn't find a target device\n");
		return -ENODEV;
	}

	/* the hw->cpus field is used by memlat governor */
	cpumask_copy(&hw->cpus, &cpus);
	num_cpus = cpumask_weight(&cpus);
	hw->num_cores = num_cpus;
	hw->core_stats = devm_kzalloc(dev, hw->num_cores *
				      sizeof(*hw->core_stats), GFP_KERNEL);
	if (hw->core_stats == NULL)
		return -ENOMEM;

	for_each_cpu(cpu, &hw->cpus)
		to_devstats(hw, cpu)->id = cpu;

	hw->start_hwmon = &start_hwmon;
	hw->stop_hwmon = &stop_hwmon;
	hw->get_cnt = &get_cnt;
	hw->request_update_ms = &request_update_ms;

	/* cache miss event ids init */
	mon->miss_ev = devm_kzalloc(dev, num_cpus * sizeof(*mon->miss_ev),
				    GFP_KERNEL);
	if (mon->miss_ev == NULL) {
		dev_err(dev, "Couldn't t device\n");
		return -ENOMEM;
	}

	ret = of_property_read_u32(dev->of_node, "hisi,cachemiss-ev",
				   &event_id);
	if (ret != 0) {
		dev_dbg(dev, "Cache Miss event not specified. Using def:0x%x\n",
			L2DM_EV);
		event_id = L2DM_EV;
	}
	mon->miss_ev_id = event_id;
	mon->is_active = false;
	mon->cpu_grp = cpu_grp;

	mutex_lock(&cpu_grp->mons_lock);
	list_add_tail(&mon->mon_node, &cpu_grp->mon_list);
	mutex_unlock(&cpu_grp->mons_lock);

	ret = register_memlat(dev, hw);
	if (ret != 0) {
		dev_err(dev, "Mem Latency Gov register failed\n");
		return ret;
	}

	return 0;
}

static const struct of_device_id match_table[] = {
	{ .compatible = "hisi,arm-memlat-mon" },
	{}
};

static struct platform_driver arm_memlat_mon_driver = {
	.probe = arm_memlat_mon_driver_probe,
	.driver = {
		.name = "arm-memlat-mon",
		.of_match_table = match_table,
		.owner = THIS_MODULE,
	},
};

static int __init arm_memlat_mon_init(void)
{
	int ret;

	memlat_wq = create_freezable_workqueue("memlat_wq");
	if (memlat_wq == NULL) {
		pr_err("%s: Couldn't create memlat workqueue.\n", __func__);
		return -ENOMEM;
	}

	ret = platform_driver_register(&arm_memlat_mon_driver);
	if (!ret) {
		cpuhp_setup_state_nocalls(CPUHP_AP_HISI_ARM_MEMLAT_MON_NOTIFY_PREPARE,
					  "arm-memlat-mon:online",
					  arm_memlat_cpu_online, arm_memlat_cpu_offline);

		return sugov_register_notifier(&memlat_nb);
	}

	return ret;
}
module_init(arm_memlat_mon_init);

static void __exit arm_memlat_mon_exit(void)
{
	(void)sugov_unregister_notifier(&memlat_nb);
	cpuhp_remove_state_nocalls(CPUHP_AP_HISI_ARM_MEMLAT_MON_NOTIFY_PREPARE);
	platform_driver_unregister(&arm_memlat_mon_driver);
	destroy_workqueue(memlat_wq);
}
module_exit(arm_memlat_mon_exit);
