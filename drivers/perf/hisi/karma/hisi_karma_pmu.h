/*
 * HiSilicon SoC Hardware event counters support
 *
 * Copyright (C) 2018 Hisilicon Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __HISI_KARMA_PMU_H__
#define __HISI_KARMA_PMU_H__

#include <linux/cpumask.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/perf_event.h>
#include <linux/types.h>

/* include cycle counter */
#define KARMA_PMU_MAX_HW_CNTRS	32

/* KARMA has 6-counters */
#define KARMA_NR_COUNTERS				0x6

/*
 * We use the index of the counters as they appear in the counter
 * bit maps in the PMU registers (e.g CLUSTERPMSELR).
 * i.e,
 *	counter 0	- Bit 0
 *	counter 1	- Bit 1
 *	...
 *	Cycle counter	- Bit 31
 */
#define KARMA_PMU_IDX_CYCLE_COUNTER	31

#define KARMA_ACTIVE_CPU_MASK			0x0
#define KARMA_ASSOCIATED_CPU_MASK		0x1


#define to_hisi_pmu(p)	(container_of(p, struct karma_pmu, pmu))

#define PMU_GET_EVENTID(ev) (ev->hw.config_base & 0xfff)

#define PMU_MAX_PERIOD(nr) (BIT_ULL(nr) - 1)

#ifdef CONFIG_HISI_KARMA_PMU_DEBUG
#define KARMA_EXT_ATTR(_name, _func, _config)		\
	(&((struct dev_ext_attribute[]) {				\
		{							\
			.attr = __ATTR(_name, 0440, _func, NULL),	\
			.var = (void *)_config				\
		}							\
	})[0].attr.attr)

#define KARMA_EVENT_ATTR(_name, _config)		\
	KARMA_EXT_ATTR(_name, karma_pmu_sysfs_event_show, (unsigned long)_config)

#define KARMA_FORMAT_ATTR(_name, _config)		\
	KARMA_EXT_ATTR(_name, karma_pmu_sysfs_format_show, (char *)_config)

#define KARMA_CPUMASK_ATTR(_name, _config)	\
	KARMA_EXT_ATTR(_name, karma_pmu_cpumask_show, (unsigned long)_config)
#endif

#define for_each_sibling_event(sibling, event)			\
	if ((event)->group_leader == (event))			\
		list_for_each_entry((sibling), &(event)->sibling_list, sibling_list)

struct karma_pmu_hwevents {
	struct perf_event *hw_events[KARMA_PMU_MAX_HW_CNTRS];
	DECLARE_BITMAP(used_mask, KARMA_PMU_MAX_HW_CNTRS);
};

/* Generic pmu struct for different pmu types */
struct karma_pmu {
	struct pmu pmu;
	struct karma_pmu_hwevents pmu_events;
	/* associated_cpus: All CPUs associated with the PMU */
	cpumask_t associated_cpus;
	/* CPU used for counting */
	cpumask_t active_cpu;
	//int irq;
	struct device *dev;
	raw_spinlock_t pmu_lock;
	struct hlist_node node;
	void __iomem *base;
	/* the ID of the PMU modules */
	u32 index_id;
	int num_counters;
	int counter_bits;
	struct notifier_block cpu_pm_nb;
	bool fcm_idle;
	spinlock_t fcm_idle_lock;
	/* check event code range */
	u64 check_event;
};



int karma_pmu_counter_valid(struct karma_pmu *karma_pmu, int idx);
#ifdef CONFIG_HISI_KARMA_PMU_DEBUG
ssize_t karma_pmu_sysfs_event_show(struct device *dev,
			      struct device_attribute *attr, char *buf);
ssize_t karma_pmu_sysfs_format_show(struct device *dev,
			       struct device_attribute *attr, char *buf);
ssize_t karma_pmu_cpumask_show(struct device *dev,
				struct device_attribute *attr, char *buf);
#endif





#endif /* __HISI_KARMA_PMU_H__ */
