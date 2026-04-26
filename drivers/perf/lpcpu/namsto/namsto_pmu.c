/*
 * hisi_namsto_pmu.c
 *
 * Namsto Performance Monitor Unit driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#define PMUNAME		"hisi_namsto"
#define DRVNAME		PMUNAME "_pmu"
#undef pr_fmt
#define pr_fmt(fmt)     "namsto_pmu: " fmt

#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/version.h>
#include <linux/bug.h>
#include <linux/cpuhotplug.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/cpu_pm.h>
#include <securec.h>
#include <namsto_pmu_plat.h>
#include "namsto_pmu.h"

/* All event counters are 32bit, with a 64bit Cycle counter */
#define namsto_pmu_counter_width(idx)	\
	(((idx) == NAMSTO_PMU_IDX_CYCLE_COUNTER) ? 64 : 32)

#define namsto_pmu_counter_mask(idx)	\
	GENMASK_ULL((namsto_pmu_counter_width((idx)) - 1), 0)

static unsigned long namsto_pmu_cpuhp_state;

/*
 * Select the counter register offset using the counter index
 */
static inline u32 get_counter_offset(int cntr_idx)
{
	return (u32)(NAMSTO_PMEVCNTR0_OFFSET + (cntr_idx * PMEVCNTR_STEP));
}

static inline u64 __namsto_pmu_read_counter(struct namsto_pmu *namsto_pmu,
					    int idx)
{
	u64 val;

	val = readl(namsto_pmu->base + get_counter_offset(idx));
	return val;
}

static u64 __namsto_pmu_read_pmccntr(struct namsto_pmu *namsto_pmu)
{
	u64 val_l;
	u64 val_h;
	u64 val64;

	val_l = readl(namsto_pmu->base + NAMSTO_PMCCNTR_L_OFFSET);
	val_h = readl(namsto_pmu->base + NAMSTO_PMCCNTR_H_OFFSET);
	val64 = val_h << 32 | val_l;

	return val64;
}

static u64 namsto_pmu_read_counter(struct perf_event *event)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(event->pmu);
	struct hw_perf_event *hwc = &event->hw;
	int idx = hwc->idx;
	unsigned long flags;
	u64 val;

	if (WARN_ON(!cpumask_test_cpu(smp_processor_id(),
				      &namsto_pmu->associated_cpus)))
		return 0;

	if (!namsto_pmu_counter_valid(namsto_pmu, idx)) {
		dev_err(namsto_pmu->dev, "Unsupported event index:%d\n", idx);
		return 0;
	}

	raw_spin_lock_irqsave(&namsto_pmu->pmu_lock, flags);
	if (idx == NAMSTO_PMU_IDX_CYCLE_COUNTER)
		val = __namsto_pmu_read_pmccntr(namsto_pmu);
	else
		val = __namsto_pmu_read_counter(namsto_pmu, idx);
	raw_spin_unlock_irqrestore(&namsto_pmu->pmu_lock, flags);

	return val;
}

static inline void __namsto_pmu_write_counter(struct namsto_pmu *namsto_pmu,
					      int idx, u64 val)
{
	writel(val, namsto_pmu->base + get_counter_offset(idx));
}

#define U32_MAX_MASK			    ((1UL << 32) - 1)
static void __namsto_pmu_write_pmccntr(struct namsto_pmu *namsto_pmu,
				       u64 val)
{
	u32 val_l = val & U32_MAX_MASK;
	u32 val_h = (val >> 32) & U32_MAX_MASK;

	writel(val_l, namsto_pmu->base + NAMSTO_PMCCNTR_L_OFFSET);
	writel(val_h, namsto_pmu->base + NAMSTO_PMCCNTR_H_OFFSET);
}

static void namsto_pmu_write_counter(struct perf_event *event, u64 val)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(event->pmu);
	struct hw_perf_event *hwc = &event->hw;
	int idx = hwc->idx;
	unsigned long flags;

	if (WARN_ON(!cpumask_test_cpu(smp_processor_id(),
				      &namsto_pmu->associated_cpus)))
		return;

	if (!namsto_pmu_counter_valid(namsto_pmu, idx)) {
		dev_err(namsto_pmu->dev, "Unsupported event index:%d\n", idx);
		return;
	}

	raw_spin_lock_irqsave(&namsto_pmu->pmu_lock, flags);
	if (idx == NAMSTO_PMU_IDX_CYCLE_COUNTER)
		__namsto_pmu_write_pmccntr(namsto_pmu, val);
	else
		__namsto_pmu_write_counter(namsto_pmu, idx, val);
	raw_spin_unlock_irqrestore(&namsto_pmu->pmu_lock, flags);
}

static void namsto_pmu_enable_counter(struct namsto_pmu *namsto_pmu,
				      struct hw_perf_event *hwc)
{
	/* Enable counter index in NAMSTO_ENSET register */
	writel(BIT(hwc->idx), namsto_pmu->base + NAMSTO_PMCNTENSET_OFFSET);
}

static void namsto_pmu_disable_counter(struct namsto_pmu *namsto_pmu,
				       struct hw_perf_event *hwc)
{
	/* Clear counter index in NAMSTO_ENCLR register */
	writel(BIT(hwc->idx), namsto_pmu->base + NAMSTO_PMCNTENCLR_OFFSET);
}

#ifdef CONFIG_NAMSTO_PMU_DEBUG
static struct attribute *namsto_pmu_format_attrs[] = {
	NAMSTO_FORMAT_ATTR(event, "config:0-31"),
	NULL,
};

static const struct attribute_group namsto_pmu_format_group = {
	.name = "format",
	.attrs = namsto_pmu_format_attrs,
};

static umode_t namsto_pmu_event_attr_is_visible(struct kobject *kobj,
						struct attribute *attr,
						int unused)
{
	return attr->mode;
}

static const struct attribute_group namsto_pmu_events_group = {
	.name = "events",
	.attrs = namsto_pmu_events_attrs,
	.is_visible = namsto_pmu_event_attr_is_visible,
};

static struct attribute *namsto_pmu_cpumask_attrs[] = {
	NAMSTO_CPUMASK_ATTR(cpumask, NAMSTO_ACTIVE_CPU_MASK),
	NAMSTO_CPUMASK_ATTR(associated_cpus, NAMSTO_ASSOCIATED_CPU_MASK),
	NULL,
};

static const struct attribute_group namsto_pmu_cpumask_attr_group = {
	.attrs = namsto_pmu_cpumask_attrs,
};

static const struct attribute_group *namsto_pmu_attr_groups[] = {
	&namsto_pmu_cpumask_attr_group,
	&namsto_pmu_events_group,
	&namsto_pmu_format_group,
	NULL,
};

/*
 * PMU event attributes
 */
ssize_t namsto_pmu_sysfs_event_show(struct device *dev,
				    struct device_attribute *attr,
				    char *page)
{
	struct dev_ext_attribute *eattr = NULL;

	eattr = container_of(attr, struct dev_ext_attribute, attr);

	return snprintf_s(page, PAGE_SIZE, PAGE_SIZE - 1,
			  "config=0x%lx\n", (uintptr_t)eattr->var);
}

/*
 * PMU format attributes
 */
ssize_t namsto_pmu_sysfs_format_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct dev_ext_attribute *eattr = NULL;

	eattr = container_of(attr, struct dev_ext_attribute, attr);

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%s\n", (char *)eattr->var);
}

ssize_t namsto_pmu_cpumask_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(dev_get_drvdata(dev));
	struct dev_ext_attribute *eattr = container_of(attr,
						       struct dev_ext_attribute,
						       attr);
	unsigned long mask_id = (unsigned long)eattr->var;
	const cpumask_t *cpumask = NULL;

	switch (mask_id) {
	case NAMSTO_ACTIVE_CPU_MASK:
		cpumask = &namsto_pmu->active_cpu;
		break;
	case NAMSTO_ASSOCIATED_CPU_MASK:
		cpumask = &namsto_pmu->associated_cpus;
		break;
	default:
		return 0;
	}
	return cpumap_print_to_pagebuf(true, buf, cpumask);
}
#endif

bool namsto_pmu_counter_valid(struct namsto_pmu *namsto_pmu, int idx)
{
	return (idx >= 0 && idx < namsto_pmu->num_counters) ||
		(idx == NAMSTO_PMU_IDX_CYCLE_COUNTER);
}

static int namsto_pmu_get_event_idx(struct perf_event *event)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(event->pmu);
	unsigned long *used_mask = namsto_pmu->hw_events.used_mask;
	int num_counters = namsto_pmu->num_counters;
	unsigned long evtype = event->attr.config;
	int idx;

	if (evtype == NAMSTO_PMU_EVT_CYCLES) {
		if (test_and_set_bit(NAMSTO_PMU_IDX_CYCLE_COUNTER, used_mask))
			return -EAGAIN;
		return NAMSTO_PMU_IDX_CYCLE_COUNTER;
	}

	idx = find_first_zero_bit(used_mask, num_counters);
	if (idx >= num_counters) {
		dev_err(namsto_pmu->dev,
			"no idle counter to use idx = %d\n", idx);
		return -EAGAIN;
	}
	set_bit(idx, used_mask);
	return idx;
}

static void namsto_pmu_clear_event_idx(struct namsto_pmu *namsto_pmu, int idx)
{
	if (!namsto_pmu_counter_valid(namsto_pmu, idx)) {
		dev_err(namsto_pmu->dev, "Unsupported event index:%d\n", idx);
		return;
	}

	clear_bit(idx, namsto_pmu->hw_events.used_mask);
}

static bool namsto_pmu_validate_event(struct pmu *pmu,
				      struct namsto_hw_events *hw_events,
				      struct perf_event *event)
{
	if (is_software_event(event))
		return true;

	if (event->pmu != pmu)
		return false;

	return namsto_pmu_get_event_idx(event) >= 0;
}

/*
 * Make sure the group of events can be scheduled at once
 * on the PMU
 */
static bool namsto_pmu_validate_group(struct perf_event *event)
{
	struct perf_event *sibling = NULL;
	struct perf_event *leader = event->group_leader;
	struct namsto_hw_events fake_hw;
	int ret;

	if (event->group_leader == event)
		return true;

	ret = memset_s(fake_hw.used_mask,
		       sizeof(fake_hw.used_mask),
		       0,
		       sizeof(fake_hw.used_mask));
	if (ret != EOK) {
		pr_err("namsto_pmu: memset_s failed\n");
		return false;
	}

	if (!namsto_pmu_validate_event(event->pmu, &fake_hw, leader))
		return false;

	list_for_each_entry(sibling, &leader->sibling_list, group_entry) {
		if (!namsto_pmu_validate_event(event->pmu, &fake_hw, sibling))
			return false;
	}

	return namsto_pmu_validate_event(event->pmu, &fake_hw, event);
}

static int namsto_pmu_event_init(struct perf_event *event)
{
	struct namsto_pmu *namsto_pmu = NULL;
	struct hw_perf_event *hwc = NULL;

	if (event == NULL)
		return -EINVAL;

	namsto_pmu = to_namsto_pmu(event->pmu);
	hwc = &event->hw;

	if (event->attr.type != event->pmu->type)
		return -ENOENT;

	/* We don't support sampling */
	if (is_sampling_event(event)) {
		dev_err(namsto_pmu->pmu.dev, "Can't support sampling events\n");
		return -EOPNOTSUPP;
	}

	/* We cannot support task bound events */
	if (event->cpu < 0 || (event->attach_state & PERF_ATTACH_TASK) != 0) {
		dev_err(namsto_pmu->pmu.dev, "Can't support per-task counters\n");
		return -EINVAL;
	}

	/* counters do not have these bits */
	if (has_branch_stack(event)	||
	    event->attr.exclude_user	||
	    event->attr.exclude_kernel	||
	    event->attr.exclude_hv	||
	    event->attr.exclude_idle	||
	    event->attr.exclude_host	||
	    event->attr.exclude_guest) {
		dev_err(namsto_pmu->pmu.dev, "Can't support filtering\n");
		return -EINVAL;
	}

	if (cpumask_test_cpu(event->cpu, &namsto_pmu->associated_cpus) == 0) {
		dev_err(namsto_pmu->pmu.dev,
			"Requested cpu is not associated with the namsto\n");
		return -EINVAL;
	}

	if (event->attr.config > namsto_pmu->check_event) {
		dev_err(namsto_pmu->pmu.dev, "invalid attr.config=0x%lx\n",
			event->attr.config);
		return -EINVAL;
	}

	/*
	 * Choose the current active CPU to read the events. We don't want
	 * to migrate the event contexts, irq handling etc to the requested
	 * CPU. As long as the requested CPU is within the same namsto, we
	 * are fine.
	 */
	event->cpu = cpumask_first(&namsto_pmu->active_cpu);
	if ((unsigned int)event->cpu >= nr_cpu_ids) {
		dev_err(namsto_pmu->pmu.dev, "invalid event->cpu=%d\n",
			event->cpu);
		return -EINVAL;
	}

	if (!namsto_pmu_validate_group(event))
		return -EINVAL;

	hwc->config_base = event->attr.config;

	return 0;
}

static void namsto_pmu_counter_interrupt_enable(struct namsto_pmu *namsto_pmu,
						struct hw_perf_event *hwc)
{
	writel(BIT(hwc->idx), namsto_pmu->base + NAMSTO_PMINTENSET_OFFSET);
}

static void namsto_pmu_counter_interrupt_disable(struct namsto_pmu *namsto_pmu,
						 struct hw_perf_event *hwc)
{
	writel(BIT(hwc->idx), namsto_pmu->base + NAMSTO_PMINTENCLR_OFFSET);
}

/*
 * Set the counter to count the event that we're interested in,
 * and enable interrupt and counter.
 */
static void namsto_pmu_enable_event(struct perf_event *event)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(event->pmu);
	struct hw_perf_event *hwc = &event->hw;

	namsto_pmu_counter_interrupt_enable(namsto_pmu, hwc);
	namsto_pmu_enable_counter(namsto_pmu, hwc);
}

/*
 * Disable counter and interrupt.
 */
static void namsto_pmu_disable_event(struct perf_event *event)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(event->pmu);
	struct hw_perf_event *hwc = &event->hw;

	namsto_pmu_disable_counter(namsto_pmu, hwc);
	namsto_pmu_counter_interrupt_disable(namsto_pmu, hwc);
}

/*
 * namsto_pmu_set_event_period: Set the period for the counter.
 *
 * All NAMSTO PMU event counters, except the cycle counter are 32bit
 * counters. To handle cases of extreme interrupt latency, we program
 * the counter with half of the max count for the counters.
 */
static void namsto_pmu_set_event_period(struct perf_event *event)
{
	int idx = event->hw.idx;
	u64 val = namsto_pmu_counter_mask(idx) >> 1;

	local64_set(&event->hw.prev_count, val);
	/* Write start value to the hardware event counter */
	namsto_pmu_write_counter(event, val);
}

static void namsto_pmu_event_update(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	u64 delta, prev_raw_count, new_raw_count;

	do {
		/* Read the count from the counter register */
		new_raw_count = namsto_pmu_read_counter(event);
		prev_raw_count = local64_read(&hwc->prev_count);
	} while (local64_cmpxchg(&hwc->prev_count, prev_raw_count,
				 new_raw_count) != prev_raw_count);
	/*
	 * compute the delta
	 */
	delta = (new_raw_count - prev_raw_count) &
		namsto_pmu_counter_mask(hwc->idx);

	local64_add(delta, &event->count);
}

static u32 namsto_pmu_get_reset_overflow(struct namsto_pmu *namsto_pmu)
{
	u32 val;

	val = readl(namsto_pmu->base + NAMSTO_PMOVSCLR_OFFSET);
	/* Clear the bit */
	writel(val, namsto_pmu->base + NAMSTO_PMOVSCLR_OFFSET);

	return val;
}

static irqreturn_t namsto_pmu_handle_irq(int irq_num, void *dev)
{
	int i;
	bool handled = false;
	struct namsto_pmu *namsto_pmu = dev;
	struct namsto_hw_events *hw_events = &namsto_pmu->hw_events;
	unsigned long overflow;

	overflow = namsto_pmu_get_reset_overflow(namsto_pmu);
	if (overflow == 0)
		return IRQ_NONE;

	for_each_set_bit(i, &overflow, NAMSTO_PMU_MAX_HW_CNTRS) {
		struct perf_event *event = hw_events->events[i];

		if (event == NULL)
			continue;

		namsto_pmu_event_update(event);
		namsto_pmu_set_event_period(event);
		handled = true;
	}

	return IRQ_RETVAL(handled);
}

static void namsto_pmu_set_event(struct namsto_pmu *namsto_pmu,
				 struct perf_event *event)
{
	int idx = event->hw.idx;

	if (!namsto_pmu_counter_valid(namsto_pmu, idx)) {
		dev_err(namsto_pmu->dev, "Unsupported event index:%d\n", idx);
		return;
	}

	writel(pmu_get_eventid(event),
	       namsto_pmu->base + NAMSTO_PMEVTYPER0_OFFSET + idx * PMEVTYPER_STEP);
}

static void namsto_pmu_start(struct perf_event *event, int pmu_flags)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(event->pmu);
	struct hw_perf_event *hwc = &event->hw;
	unsigned long flags;

	 /* We always reprogram the counter */
	if ((unsigned int)pmu_flags & PERF_EF_RELOAD)
		WARN_ON(!(event->hw.state & PERF_HES_UPTODATE));

	namsto_pmu_set_event_period(event);
	hwc->state = 0;

	raw_spin_lock_irqsave(&namsto_pmu->pmu_lock, flags);
	if (hwc->idx != NAMSTO_PMU_IDX_CYCLE_COUNTER)
		namsto_pmu_set_event(namsto_pmu, event);
	namsto_pmu_enable_event(event);
	raw_spin_unlock_irqrestore(&namsto_pmu->pmu_lock, flags);
}

static void namsto_pmu_stop(struct perf_event *event, int pmu_flags)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(event->pmu);
	unsigned long flags;

	if (event->hw.state & PERF_HES_STOPPED)
		return;

	raw_spin_lock_irqsave(&namsto_pmu->pmu_lock, flags);
	namsto_pmu_disable_event(event);
	raw_spin_unlock_irqrestore(&namsto_pmu->pmu_lock, flags);

	namsto_pmu_event_update(event);
	event->hw.state |= PERF_HES_STOPPED | PERF_HES_UPTODATE;
}

static int namsto_pmu_add(struct perf_event *event, int pmu_flags)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(event->pmu);
	struct hw_perf_event *hwc = &event->hw;
	int idx;

	if (WARN_ON_ONCE(!cpumask_test_cpu(smp_processor_id(),
					   &namsto_pmu->associated_cpus)))
		return -ENOENT;

	/* Get an available counter index for counting */
	idx = namsto_pmu_get_event_idx(event);
	if (idx < 0)
		return idx;

	event->hw.idx = idx;
	namsto_pmu->hw_events.events[idx] = event;
	hwc->state = PERF_HES_STOPPED | PERF_HES_UPTODATE;

	if ((unsigned int)pmu_flags & PERF_EF_START)
		namsto_pmu_start(event, PERF_EF_RELOAD);

	perf_event_update_userpage(event);
	return 0;
}

static void namsto_pmu_del(struct perf_event *event, int pmu_flags)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(event->pmu);
	struct hw_perf_event *hwc = &event->hw;
	int idx = hwc->idx;

	namsto_pmu_stop(event, PERF_EF_UPDATE);
	namsto_pmu->hw_events.events[idx] = NULL;
	namsto_pmu_clear_event_idx(namsto_pmu, idx);
	perf_event_update_userpage(event);
}

static void namsto_pmu_read(struct perf_event *event)
{
	/* Read hardware counter and update the perf counter statistics */
	namsto_pmu_event_update(event);
}

static void namsto_pmu_enable(struct pmu *pmu)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(pmu);
	unsigned long flags;
	int enabled;
	u32 pmcr;

	enabled = bitmap_empty(namsto_pmu->hw_events.used_mask,
			       NAMSTO_PMU_MAX_HW_CNTRS);
	if (enabled != 0)
		return;

	/*
	 * Set enable bit in NAMSTO_PMCR register to start counting
	 * for all enabled counters.
	 */
	raw_spin_lock_irqsave(&namsto_pmu->pmu_lock, flags);
	pmcr = readl(namsto_pmu->base + NAMSTO_PMCR_OFFSET);
	pmcr |= NAMSTO_PMCR_EN;
	writel(pmcr, namsto_pmu->base + NAMSTO_PMCR_OFFSET);
	raw_spin_unlock_irqrestore(&namsto_pmu->pmu_lock, flags);
}

static void namsto_pmu_disable(struct pmu *pmu)
{
	struct namsto_pmu *namsto_pmu = to_namsto_pmu(pmu);
	unsigned long flags;
	u32 pmcr;

	/*
	 * Clear enable bit in NAMSTO_PMCR register to stop counting
	 * for all enabled counters.
	 */
	raw_spin_lock_irqsave(&namsto_pmu->pmu_lock, flags);
	pmcr = readl(namsto_pmu->base + NAMSTO_PMCR_OFFSET);
	pmcr &= ~(NAMSTO_PMCR_EN);
	writel(pmcr, namsto_pmu->base + NAMSTO_PMCR_OFFSET);
	raw_spin_unlock_irqrestore(&namsto_pmu->pmu_lock, flags);
}

struct cpu_pm_namsto_pmu_args {
	struct namsto_pmu *namsto_pmu;
	unsigned long cmd;
	int cpu;
	int ret;
};

#ifdef CONFIG_CPU_PM
static void cpu_pm_namsto_pmu_setup(struct namsto_pmu *namsto_pmu,
				    unsigned long cmd)
{
	struct namsto_hw_events *hw_events = &namsto_pmu->hw_events;
	struct perf_event *event = NULL;
	int idx;

	for (idx = 0; idx < namsto_pmu->num_counters; idx++) {
		/*
		 * If the counter is not used skip it, there is no
		 * need of stopping/restarting it.
		 */
		if (!test_bit(idx, hw_events->used_mask))
			continue;

		event = hw_events->events[idx];
		if (event == NULL)
			continue;

		/*
		 * Check if an attempt was made to free this event during
		 * the CPU went offline.
		 */
		if (event->state != PERF_EVENT_STATE_ACTIVE)
			continue;

		switch (cmd) {
		case CPU_PM_ENTER:
			/*
			 * Stop and update the counter
			 */
			namsto_pmu_stop(event, PERF_EF_UPDATE);
			break;
		case CPU_PM_EXIT:
		case CPU_PM_ENTER_FAILED:
			 /*
			  * Restore and enable the counter.
			  * namsto_pmu_start() indirectly calls
			  *
			  * perf_event_update_userpage()
			  *
			  * that requires RCU read locking to be functional,
			  * wrap the call within RCU_NONIDLE to make the
			  * RCU subsystem aware this cpu is not idle from
			  * an RCU perspective for the namsto_pmu_start() call
			  * duration.
			  */
			RCU_NONIDLE(namsto_pmu_start(event, PERF_EF_RELOAD));
			break;
		default:
			break;
		}
	}
}

static void cpu_pm_namsto_pmu_common(void *info)
{
	struct cpu_pm_namsto_pmu_args *data = info;
	struct namsto_pmu *namsto_pmu = data->namsto_pmu;
	unsigned long cmd = data->cmd;
	int cpu = data->cpu;
	struct namsto_hw_events *hw_events = &namsto_pmu->hw_events;
	int enabled = bitmap_weight(hw_events->used_mask, namsto_pmu->num_counters);
	bool fcm_pwrdn = 0;

	if (cpumask_test_cpu(cpu, &namsto_pmu->associated_cpus) == 0) {
		data->ret = NOTIFY_DONE;
		return;
	}

	if (enabled == 0) {
		data->ret = NOTIFY_OK;
		return;
	}

	data->ret = NOTIFY_OK;

	switch (cmd) {
	case CPU_PM_ENTER:
		spin_lock(&namsto_pmu->fcm_idle_lock);
		fcm_pwrdn = lpcpu_fcm_cluster_pwrdn();
		if (fcm_pwrdn && !namsto_pmu->fcm_idle) {
			namsto_pmu->fcm_idle = true;
			namsto_pmu_disable(&namsto_pmu->pmu);
			cpu_pm_namsto_pmu_setup(namsto_pmu, cmd);
		}
		spin_unlock(&namsto_pmu->fcm_idle_lock);
		break;
	case CPU_PM_EXIT:
	case CPU_PM_ENTER_FAILED:
		spin_lock(&namsto_pmu->fcm_idle_lock);
		if (namsto_pmu->fcm_idle) {
			namsto_pmu->fcm_idle = false;
			cpu_pm_namsto_pmu_setup(namsto_pmu, cmd);
			namsto_pmu_enable(&namsto_pmu->pmu);
		}
		spin_unlock(&namsto_pmu->fcm_idle_lock);
		break;
	default:
		data->ret = NOTIFY_DONE;
		break;
	}
}

static int cpu_pm_namsto_pmu_notify(struct notifier_block *b,
				    unsigned long cmd, void *v)
{
	struct cpu_pm_namsto_pmu_args data = {
		.namsto_pmu = container_of(b, struct namsto_pmu, cpu_pm_nb),
		.cmd = cmd,
		.cpu = smp_processor_id(),
	};

	cpu_pm_namsto_pmu_common(&data);
	return data.ret;
}

static int cpu_pm_namsto_pmu_register(struct namsto_pmu *namsto_pmu)
{
	namsto_pmu->cpu_pm_nb.notifier_call = cpu_pm_namsto_pmu_notify;
	return cpu_pm_register_notifier(&namsto_pmu->cpu_pm_nb);
}

static void cpu_pm_namsto_pmu_unregister(struct namsto_pmu *namsto_pmu)
{
	cpu_pm_unregister_notifier(&namsto_pmu->cpu_pm_nb);
}

#else
static inline int cpu_pm_namsto_pmu_register(struct namsto_pmu *namsto_pmu) { return 0; }
static inline void cpu_pm_namsto_pmu_unregister(struct namsto_pmu *namsto_pmu) { }
static void cpu_pm_namsto_pmu_common(void *info) { }
#endif

static int namsto_pmu_dev_init(struct platform_device *pdev,
			       struct namsto_pmu *namsto_pmu)
{
	struct device *dev = &pdev->dev;

	namsto_pmu->base = of_iomap(dev->of_node, 0);
	if (namsto_pmu->base == NULL) {
		dev_err(&pdev->dev, "ioremap failed for namsto_pmu\n");
		return -ENOMEM;
	}

	raw_spin_lock_init(&namsto_pmu->pmu_lock);
	spin_lock_init(&namsto_pmu->fcm_idle_lock);
	cpumask_copy(&namsto_pmu->associated_cpus, cpu_online_mask);

	namsto_pmu->dev = &pdev->dev;
	namsto_pmu->check_event = NAMSTO_EV_ID_MAX;
	namsto_pmu->num_counters = -1;
	namsto_pmu->fcm_idle = false;

	return 0;
}

static int namsto_pmu_probe(struct platform_device *pdev)
{
	struct namsto_pmu *namsto_pmu = NULL;
	struct device *dev = &pdev->dev;
	char *name = NULL;
	int ret;
	int irq;

	dev_err(dev, "%s enter\n", __func__);
	namsto_pmu = devm_kzalloc(dev, sizeof(*namsto_pmu), GFP_KERNEL);
	if (namsto_pmu == NULL)
		return -ENOMEM;

	platform_set_drvdata(pdev, namsto_pmu);
	dev_set_name(dev, "namsto_pmu");

	name = devm_kasprintf(dev, GFP_KERNEL, "%s", DRVNAME);
	if (name == NULL)
		return -ENOMEM;

	ret = namsto_pmu_dev_init(pdev, namsto_pmu);
	if (ret != 0)
		return ret;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "failed to find namsto pmu IRQ\n");
		ret = -EINVAL;
		goto err;
	}

	ret = devm_request_irq(dev, irq, namsto_pmu_handle_irq,
			       IRQF_NOBALANCING, name, namsto_pmu);
	if (ret != 0) {
		dev_err(dev, "failed to request irq\n");
		goto err;
	}
	namsto_pmu->irq = irq;

	ret = cpuhp_state_add_instance(namsto_pmu_cpuhp_state,
				       &namsto_pmu->cpuhp_node);
	if (ret != 0) {
		dev_err(dev, "error %d registering hotplug\n", ret);
		goto cpuhp;
	}

	namsto_pmu->pmu = (struct pmu) {
		.task_ctx_nr		= perf_invalid_context,
		.module			= THIS_MODULE,
		.event_init		= namsto_pmu_event_init,
		.pmu_enable		= namsto_pmu_enable,
		.pmu_disable		= namsto_pmu_disable,
		.add			= namsto_pmu_add,
		.del			= namsto_pmu_del,
		.start			= namsto_pmu_start,
		.stop			= namsto_pmu_stop,
		.read			= namsto_pmu_read,
#ifdef CONFIG_NAMSTO_PMU_DEBUG
		.attr_groups	= namsto_pmu_attr_groups,
#endif
	};

	ret = cpu_pm_namsto_pmu_register(namsto_pmu);
	if (ret != 0) {
		dev_err(dev, "cpu_pm register failed\n");
		goto cpu_pm;
	}

	ret = perf_pmu_register(&namsto_pmu->pmu, name, PERF_TYPE_NAMSTO);
	if (ret != 0) {
		dev_err(dev, "namsto pmu register failed\n");
		goto pmu_reg;
	}

	dev_err(dev, "%s exit\n", __func__);
	return ret;

pmu_reg:
	cpu_pm_namsto_pmu_unregister(namsto_pmu);
cpu_pm:
	cpuhp_state_remove_instance(namsto_pmu_cpuhp_state,
				    &namsto_pmu->cpuhp_node);
cpuhp:
	irq_set_affinity_hint(namsto_pmu->irq, NULL);
err:
	iounmap(namsto_pmu->base);
	return ret;
}

static int namsto_pmu_remove(struct platform_device *pdev)
{
	struct namsto_pmu *namsto_pmu = platform_get_drvdata(pdev);

	perf_pmu_unregister(&namsto_pmu->pmu);
	cpu_pm_namsto_pmu_unregister(namsto_pmu);
	cpuhp_state_remove_instance(namsto_pmu_cpuhp_state,
				    &namsto_pmu->cpuhp_node);
	irq_set_affinity_hint(namsto_pmu->irq, NULL);
	iounmap(namsto_pmu->base);

	return 0;
}

static const struct of_device_id namsto_pmu_match[] = {
	{ .compatible = "hisi,namsto_pmu", },
	{},
};

MODULE_DEVICE_TABLE(of, namsto_pmu_match);

static struct platform_driver namsto_pmu_driver = {
	.driver = {
		.name = DRVNAME,
		.of_match_table = namsto_pmu_match,
	},
	.probe = namsto_pmu_probe,
	.remove = namsto_pmu_remove,
};

static void namsto_pmu_set_active_cpu(int cpu, struct namsto_pmu *namsto_pmu)
{
	cpumask_set_cpu(cpu, &namsto_pmu->active_cpu);
	if (irq_set_affinity_hint(namsto_pmu->irq, &namsto_pmu->active_cpu))
		pr_err("namsto_pmu: failed to set irq affinity to %d\n", cpu);
}

static void namsto_pmu_probe_pmu(struct namsto_pmu *namsto_pmu)
{
	struct device *dev = namsto_pmu->dev;
	u32 num_counters;
	u32 cpmceid[2];
	u32 value;

	value = readl(namsto_pmu->base + NAMSTO_PMCR_OFFSET);
	num_counters = value >> NAMSTO_PMCR_N_SHIFT & NAMSTO_PMCR_N_MASK;
	dev_dbg(dev, "num_counters = %u\n", num_counters);

	if (WARN_ON(num_counters > 31))
		num_counters = 31;

	namsto_pmu->num_counters = num_counters;
	if (num_counters == 0)
		return;

	cpmceid[0] = readl(namsto_pmu->base + NAMSTO_PMCEID0_OFFSET);
	cpmceid[1] = readl(namsto_pmu->base + NAMSTO_PMCEID1_OFFSET);
	dev_dbg(dev, "cpmceid[0] = 0x%x\n", cpmceid[0]);
	dev_dbg(dev, "cpmceid[1] = 0x%x\n", cpmceid[1]);

	bitmap_from_u32array(namsto_pmu->cpmceid_bitmap,
			     NAMSTO_PMU_MAX_COMMON_EVENTS,
			     cpmceid,
			     ARRAY_SIZE(cpmceid));
}

static void namsto_pmu_init_pmu(struct namsto_pmu *namsto_pmu)
{
	if (namsto_pmu->num_counters == -1)
		namsto_pmu_probe_pmu(namsto_pmu);

	namsto_pmu_get_reset_overflow(namsto_pmu);
}

static int namsto_pmu_cpu_online(unsigned int cpu, struct hlist_node *node)
{
	struct namsto_pmu *namsto_pmu = hlist_entry_safe(node,
							 struct namsto_pmu,
							 cpuhp_node);

	if (namsto_pmu == NULL ||
	    cpumask_test_cpu(cpu, &namsto_pmu->associated_cpus) == 0)
		return 0;

	/* If another CPU is already managing this PMU, simply return. */
	if (!cpumask_empty(&namsto_pmu->active_cpu))
		return 0;

	namsto_pmu_init_pmu(namsto_pmu);
	namsto_pmu_set_active_cpu(cpu, namsto_pmu);

	return 0;
}

static int get_online_cpu_any_but(struct namsto_pmu *namsto_pmu, int cpu)
{
	struct cpumask online_supported;

	cpumask_and(&online_supported,
		    &namsto_pmu->associated_cpus, cpu_online_mask);
	return cpumask_any_but(&online_supported, cpu);
}

static int namsto_pmu_cpu_offline(unsigned int cpu, struct hlist_node *node)
{
	struct namsto_pmu *namsto_pmu = hlist_entry_safe(node,
							 struct namsto_pmu,
							 cpuhp_node);
	unsigned int target;

	if (namsto_pmu == NULL ||
	    cpumask_test_and_clear_cpu(cpu, &namsto_pmu->active_cpu) == 0)
		return 0;

	target = get_online_cpu_any_but(namsto_pmu, cpu);
	if (target >= nr_cpu_ids) {
		pr_err("namsto_pmu: target %u is invalid\n", target);
		irq_set_affinity_hint(namsto_pmu->irq, NULL);
		return 0;
	}

	perf_pmu_migrate_context(&namsto_pmu->pmu, cpu, target);
	namsto_pmu_set_active_cpu(target, namsto_pmu);

	return 0;
}

static int __init namsto_pmu_module_init(void)
{
	int ret;

	ret = cpuhp_setup_state_multi(CPUHP_AP_ONLINE_DYN,
				      DRVNAME,
				      namsto_pmu_cpu_online,
				      namsto_pmu_cpu_offline);
	if (ret < 0) {
		pr_err("NAMSTO PMU: Error setup hotplug, ret = %d\n", ret);
		return ret;
	}

	namsto_pmu_cpuhp_state = ret;
	ret = platform_driver_register(&namsto_pmu_driver);
	if (ret != 0)
		cpuhp_remove_multi_state(namsto_pmu_cpuhp_state);

	return ret;
}
module_init(namsto_pmu_module_init);

static void __exit namsto_pmu_module_exit(void)
{
	platform_driver_unregister(&namsto_pmu_driver);
	cpuhp_remove_multi_state(namsto_pmu_cpuhp_state);
}
module_exit(namsto_pmu_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HISI namsto pmu driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
