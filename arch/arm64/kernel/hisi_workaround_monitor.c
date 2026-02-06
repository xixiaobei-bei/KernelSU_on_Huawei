/*
 * hisi_workaround_monitor.c
 *
 * workaround optimization for arm cpu
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/perf_event.h>
#include <linux/perf/arm_pmu.h>
#include <linux/arm-smccc.h>
#include <linux/debugfs.h>
#include <asm/mmu.h>
#include <asm/cpu.h>
#include <asm/cputype.h>
#include <bl31_smc.h>

#ifdef CONFIG_HISI_HARDEN_BRANCH_PREDICTOR
/*
 * the type definition of cpu is not unified in kernel,
 * which will cause pclint violation. In order to satisfy
 * pclint while respecting kernel native code, we have to
 * redefine some cpu related macro here
 */
#if NR_CPUS != 1
#undef for_each_cpu
#define for_each_cpu(cpu, mask)					\
	for ((cpu) = -1;					\
		(cpu) = (int) cpumask_next((cpu), (mask)),	\
		(cpu) < (int) nr_cpu_ids;)
#endif

#define BTB_UPDATE_EV		0x11c

struct wa2_pmu_monitor {
	struct perf_event *pevent;
	bool monitor_started;
};

DEFINE_PER_CPU(struct wa2_pmu_monitor, g_wa2_monitor);
DEFINE_PER_CPU_READ_MOSTLY(u64, g_wa2_pmu_counter);
DEFINE_PER_CPU_READ_MOSTLY(u64, g_wa2_apply);

bool need_to_apply(int cpu)
{
	struct wa2_pmu_monitor *monitor = &per_cpu(g_wa2_monitor, cpu);
	u64 need_apply;

	if (!monitor->monitor_started || monitor->pevent == NULL)
		return true;

	need_apply = per_cpu(g_wa2_apply, cpu);
	if (need_apply != 0) {
		per_cpu(g_wa2_apply, cpu) = 0;
		return true;
	}

	return false;
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
	attr->config = BTB_UPDATE_EV;

	return attr;
}

static bool is_enyo(int cpu)
{
	struct cpuinfo_arm64 *info = &per_cpu(cpu_data, cpu);
	u32 midr = info->reg_midr;

	return MIDR_IMPLEMENTOR(midr) == ARM_CPU_IMP_HISI &&
	       MIDR_PARTNUM(midr) == ARM_CPU_PART_ENYO;
}

static void __init wa2_monitor_init(void)
{
	int cpu;
	struct wa2_pmu_monitor *monitor = NULL;
	struct perf_event *pevent = NULL;
	struct perf_event_attr *attr = NULL;

	attr = alloc_attr();
	if (attr == NULL) {
		pr_err("fail to alloc perf attr for wa monitor\n");
		return;
	}

	for_each_possible_cpu(cpu) {
		monitor = &per_cpu(g_wa2_monitor, cpu);

		/* only enyo need to enable this PMU event */
		if (!is_enyo(cpu))
			continue;

		pevent = perf_event_create_kernel_counter(attr, cpu, NULL,
							  NULL, NULL);
		if (IS_ERR(pevent)) {
			pr_err("fail to create wa2 pmu counter for cpu%d\n",
			       cpu);
			continue;
		}

		perf_event_enable(pevent);
		per_cpu(g_wa2_pmu_counter, cpu) = armv8pmu_get_counter(pevent);
		monitor->pevent = pevent;
		monitor->monitor_started = true;
		pr_info("success to create wa2 pmu counter for cpu%d\n", cpu);
	}

	kfree(attr);
}

static void __exit wa2_monitor_exit(void)
{
	int cpu;
	struct wa2_pmu_monitor *monitor = NULL;

	for_each_possible_cpu(cpu) {
		monitor = &per_cpu(g_wa2_monitor, cpu);
		if (!monitor->monitor_started)
			continue;

		monitor->monitor_started = false;
		perf_event_disable(monitor->pevent);
		perf_event_release_kernel(monitor->pevent);
		monitor->pevent = NULL;
	}
}
#endif /* CONFIG_HISI_HARDEN_BRANCH_PREDICTOR */

#ifdef CONFIG_HISI_DEBUG_FS

#define WA_COUNTER_SIZE 11U
static int wa_counter_show(struct seq_file *s, void *unused)
{
	struct arm_smccc_res res;
	unsigned int i;

	for (i = 0; i < WA_COUNTER_SIZE; i++) {
		arm_smccc_1_1_smc(HISI_GET_WA_COUNTER_FID_VALUE, i, &res);
		seq_printf(s, "wa_counter[%u]:%llu\n", i, res.a0);
	}

	return 0;
}

static int wa_counter_open(struct inode *inode, struct file *file)
{
	if (inode == NULL || file == NULL)
		return -EPERM;

	return single_open(file, wa_counter_show, &inode->i_private);
}

static const struct file_operations g_wa_counter_fops = {
	.owner = THIS_MODULE,
	.open = wa_counter_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct dentry *g_wa_counter_dir;
#endif /* CONFIG_HISI_DEBUG_FS */

static int __init wa_monitor_init(void)
{
#ifdef CONFIG_HISI_HARDEN_BRANCH_PREDICTOR
	wa2_monitor_init();
#endif
#ifdef CONFIG_HISI_DEBUG_FS
	g_wa_counter_dir = debugfs_create_file("wa_counter", 0440,
					       NULL, NULL,
					       &g_wa_counter_fops);
	if (g_wa_counter_dir == NULL)
		pr_err("fail to create fs for wa counter\n");
#endif
	return 0;
}

static void __exit wa_monitor_exit(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	debugfs_remove(g_wa_counter_dir);
#endif
#ifdef CONFIG_HISI_HARDEN_BRANCH_PREDICTOR
	wa2_monitor_exit();
#endif
}

late_initcall(wa_monitor_init);
module_exit(wa_monitor_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("workaround optimization for arm cpu");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
