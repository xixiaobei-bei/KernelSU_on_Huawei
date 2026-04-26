/*
 * lpcpu-multidrv-cpuidle.c
 *
 * information about lpcpu cpuidle register
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "CPUidle arm64: " fmt

#include <securec.h>
#include <linux/cpuidle.h>
#include <linux/cpumask.h>
#include <linux/cpu_pm.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/hisi/lpcpu_idle_sleep.h>
#include <linux/io.h>
#include <linux/version.h>
#include <linux/string.h>

#include <asm/cpuidle.h>
#include <asm/suspend.h>
#include <linux/sched.h>
#include <linux/cpu.h>
#include "dt_idle_states.h"
#ifdef CONFIG_HISI_CORESIGHT_TRACE
#include <linux/coresight.h>
#endif

#define FILENAME_LEN		64
#define CORE_FLAG_LEN		32

#ifdef CONFIG_ARCH_HISI
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))

#ifndef local_fiq_enable
#define local_fiq_enable()	asm("msr	daifclr, #1" : : : "memory")
#endif

#ifndef local_fiq_disable
#define local_fiq_disable()	asm("msr	daifset, #1" : : : "memory")
#endif

#endif
#endif

static unsigned long g_cpu_on_hotplug;
struct cpumask g_idle_cpus_mask;
#ifdef CONFIG_HISI_CPUIDLE_SKIP_ALL_CORE_DOWN
/* g_core_idle_cpus_mask - has bit 'cpu' set iff cpu is in C1 idle state */
struct cpumask g_core_idle_cpus_mask;
#endif
spinlock_t g_idle_spin_lock;
bool lpcpu_cluster_cpu_all_pwrdn(void)
{
	struct cpuidle_driver *drv = cpuidle_get_driver();
	struct cpumask cluster_cpu_mask;
	int all_pwrdn;

	if (drv == NULL)
		return false;
	cpumask_copy(&cluster_cpu_mask, drv->cpumask);

	spin_lock(&g_idle_spin_lock);
	all_pwrdn = cpumask_subset(&cluster_cpu_mask, &g_idle_cpus_mask);
	spin_unlock(&g_idle_spin_lock);

	return !!all_pwrdn;
}
EXPORT_SYMBOL(lpcpu_cluster_cpu_all_pwrdn);

bool lpcpu_fcm_cluster_pwrdn(void)
{
	int fcm_pwrdn;

	spin_lock(&g_idle_spin_lock);
	fcm_pwrdn = cpumask_subset(cpu_online_mask, &g_idle_cpus_mask);
	spin_unlock(&g_idle_spin_lock);

	return !!fcm_pwrdn;
}
EXPORT_SYMBOL(lpcpu_fcm_cluster_pwrdn);

static struct cpumask pending_idle_cpumask;

static void kick_cpu_sync(int cpu)
{
	smp_send_reschedule(cpu);
}

static void __iomem *idle_flag_addr[NR_CPUS] = {NULL};
static u32 idle_flag_bit[NR_CPUS] = {0};

/*
 * get core-idle-flag
 *
 * failed: return 0
 * success: return core-idle-flag
 */
u32 lpcpu_get_idle_cpumask(void)
{
	u32 i, tmp_flag;
	u32 core_idle_flag = 0;

	for (i = 0; i < NR_CPUS; ++i) {
		tmp_flag = 0;

		if (idle_flag_addr[i] == NULL) {
			pr_err("get pwron cpumask : flag addr%d error.\n", i);
			return 0;
		}

		tmp_flag = (u32)readl(idle_flag_addr[i]) & BIT(idle_flag_bit[i]);
		if (tmp_flag != 0)
			core_idle_flag |= BIT(i);
	}

	return core_idle_flag;
}

/*
 * Programs CPU to enter the specified state
 *
 * dev: cpuidle device
 * drv: cpuidle driver
 * idx: state index
 *
 * Called from the CPUidle framework to program the device to the
 * specified target state selected by the governor.
 */
static int enter_idle_state(struct cpuidle_device *dev,
				 struct cpuidle_driver *drv, int idx)
{
	int ret, cpu;

	if (dev == NULL || drv == NULL) {
		idx = -1;
		return idx;
	}

	if (need_resched())
		return idx;

	if (g_cpu_on_hotplug != 0)
		idx = 0;

	if (idx == 0) {
		cpu_do_idle();
		return idx;
	}

	cpu = dev->cpu;
	/* we assume the deepest state is cluster_idle */
	if (idx == (drv->state_count - 1)) {
		spin_lock(&g_idle_spin_lock);
		cpumask_set_cpu(cpu, &g_idle_cpus_mask);
		smp_wmb();
		spin_unlock(&g_idle_spin_lock);
	}
#ifdef CONFIG_HISI_CPUIDLE_SKIP_ALL_CORE_DOWN
	if (idx == (drv->state_count - 2)) {
		spin_lock(&g_idle_spin_lock);
		cpumask_set_cpu(cpu, &g_core_idle_cpus_mask);
		smp_wmb();
		spin_unlock(&g_idle_spin_lock);
	}
#endif
	ret = cpu_pm_enter();
	if (ret == 0) {
#ifdef CONFIG_ARCH_HISI
		local_fiq_disable();
#endif
		/*
		 * Pass idle state index to cpu_suspend which in turn will
		 * call the CPU ops suspend protocol with idle index as a
		 * parameter.
		 */
		ret = arm_cpuidle_suspend(idx);
#ifdef CONFIG_HISI_CORESIGHT_TRACE
		/* Restore ETM registers */
		_etm4_cpuilde_restore();
#endif
#ifdef CONFIG_ARCH_HISI
		local_fiq_enable();
#endif
		cpu_pm_exit();
	}

	if (idx == (drv->state_count - 1)) {
		spin_lock(&g_idle_spin_lock);
		cpumask_clear_cpu(cpu, &g_idle_cpus_mask);
		smp_wmb();
		spin_unlock(&g_idle_spin_lock);
	}
#ifdef CONFIG_HISI_CPUIDLE_SKIP_ALL_CORE_DOWN
	if (idx == (drv->state_count - 2)) {
		spin_lock(&g_idle_spin_lock);
		cpumask_clear_cpu(cpu, &g_core_idle_cpus_mask);
		smp_wmb();
		spin_unlock(&g_idle_spin_lock);
	}
#endif
	return ret ? -1 : idx;
}

static int enter_coupled_idle_state(struct cpuidle_device *dev,
					 struct cpuidle_driver *drv, int idx)
{
	int ret, cpu, pending_cpu;

	if (dev == NULL || drv == NULL) {
		idx = -1;
		return idx;
	}

	if (need_resched())
		return idx;

	if (g_cpu_on_hotplug != 0)
		idx = 0;

	if (idx == 0) {
		cpu_do_idle();
		return idx;
	}

	cpu = dev->cpu;
	if (idx == (drv->state_count - 1)) {
		spin_lock(&g_idle_spin_lock);
		cpumask_set_cpu(cpu, &g_idle_cpus_mask);
		smp_wmb();

		if (cpumask_subset(drv->cpumask, &g_idle_cpus_mask)) {
			pending_cpu = cpumask_first(&pending_idle_cpumask);
			if (pending_cpu < nr_cpu_ids && pending_cpu != cpu) { //lint !e574
				cpumask_clear_cpu(pending_cpu,
						  &pending_idle_cpumask);
				smp_wmb();
				kick_cpu_sync(pending_cpu);
			}
		} else {
			/* pending bit */
			cpumask_set_cpu(cpu, &pending_idle_cpumask);
			smp_wmb();
			spin_unlock(&g_idle_spin_lock);

			cpu_do_idle();

			spin_lock(&g_idle_spin_lock);
			cpumask_clear_cpu(cpu, &g_idle_cpus_mask);
			smp_wmb();
			spin_unlock(&g_idle_spin_lock);

			return idx;
		}
		spin_unlock(&g_idle_spin_lock);
	}

	ret = cpu_pm_enter();
	if (ret == 0) {
#ifdef CONFIG_ARCH_HISI
		local_fiq_disable();
#endif
		/*
		 * Pass idle state index to cpu_suspend which in turn will
		 * call the CPU ops suspend protocol with idle index as a
		 * parameter.
		 */
		ret = arm_cpuidle_suspend(idx);
#ifdef CONFIG_HISI_CORESIGHT_TRACE
		/* Restore ETM registers */
		_etm4_cpuilde_restore();
#endif
#ifdef CONFIG_ARCH_HISI
		local_fiq_enable();
#endif
		cpu_pm_exit();
	}

	if (idx == (drv->state_count - 1)) {
		spin_lock(&g_idle_spin_lock);
		cpumask_clear_cpu(cpu, &g_idle_cpus_mask);
		smp_wmb();
		spin_unlock(&g_idle_spin_lock);
	}

	return ret ? -1 : idx;
}

/*lint -e64 -e785 -e651*/

static const struct of_device_id arm64_idle_state_match[] __initconst = {
	{
		.compatible = "arm,idle-state",
		.data = enter_idle_state
	},
	{
		.compatible = "arm,coupled-idle-state",
		.data = enter_coupled_idle_state
	},
	{ },
};

/*lint +e64 +e785 +e651*/
static int __init idle_drv_cpumask_init(struct cpuidle_driver *drv,
					     int cluster_id)
{
	struct cpumask *cpumask = NULL;
	int cpu;

	cpumask = kzalloc(cpumask_size(), GFP_KERNEL);
	if (!cpumask)
		return -ENOMEM;

	for_each_possible_cpu(cpu) { //lint !e713 !e574
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
		if (cpu_topology[cpu].package_id == cluster_id)
#else
		if (cpu_topology[cpu].cluster_id == cluster_id)
#endif
			cpumask_set_cpu((unsigned int)cpu, cpumask);
	}

	drv->cpumask = cpumask;

	return 0;
}

static void __init idle_drv_cpumask_uninit(struct cpuidle_driver *drv)
{
	kfree(drv->cpumask);
}

static int __init idle_drv_init(struct cpuidle_driver *drv)
{
	int cpu, ret;

	/*
	 * Initialize idle states data, starting at index 1.
	 * This driver is DT only, if no DT idle states are detected (ret == 0)
	 * let the driver initialization fail accordingly since there is no
	 * reason to initialize the idle driver if only wfi is supported.
	 */
	ret = dt_init_idle_driver(drv, arm64_idle_state_match, 1);
	if (ret <= 0) {
		if (ret)
			pr_err("failed to initialize idle states\n");
		return ret ? : -ENODEV;
	}

	/*
	 * Call arch CPU operations in order to initialize
	 * idle states suspend back-end specific data
	 */
	for_each_possible_cpu(cpu) { //lint !e713 !e574
		ret = arm_cpuidle_init((unsigned int)cpu);
		if (ret != 0) {
			pr_err("CPU %d failed to init idle CPU ops\n", cpu);
			return ret;
		}
	}

	ret = cpuidle_register(drv, NULL);
	if (ret != 0) {
		pr_err("failed to register cpuidle driver\n");
		return ret;
	}

	return 0;
}

static int __init multidrv_idle_init(struct cpuidle_driver *drv,
					  int cluster_id)
{
	int ret;

	ret = idle_drv_cpumask_init(drv, cluster_id);
	if (ret != 0) {
		pr_err("fail to init cluster%d idle driver!\n", cluster_id);
		return ret;
	}

	ret = idle_drv_init(drv);
	if (ret != 0) {
		idle_drv_cpumask_uninit(drv);
		pr_err("fail to register cluster%d cpuidle drv.\n", cluster_id);
		return ret;
	}

	return 0;
}

static int __init get_idle_mask_init(void)
{
	int ret;
	struct device_node *np = NULL;
	u32 i, reg_base_addr;
	u32 flag_data[2];
	char filename[FILENAME_LEN];
	void __iomem *base_addr = NULL;

	np = of_find_compatible_node(NULL, NULL,
				     "hisilicon,cpu-idle-flag");//lint !e838
	if (np == NULL) {
		pr_err("idle_mask_init : get base node error.\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "idle-reg-base", &reg_base_addr);
	if (ret != 0) {
		pr_err("idle_mask_init : get node idle-reg-base error.\n");
		return -ENODEV;
	}

	base_addr = ioremap(reg_base_addr, SZ_4K);
	if (base_addr == NULL) {
		pr_err("idle_mask_init : ioremap base addr error.\n");
		return -ENOMEM;
	}

	for (i = 0; i < NR_CPUS; ++i) {
		flag_data[0] = 0;
		flag_data[1] = 0;
		ret = memset_s(filename, sizeof(filename), 0, sizeof(filename));
		if (ret != EOK)
			return -ENODEV;

		ret = snprintf_s(filename, sizeof(filename), CORE_FLAG_LEN,
				 "core-%u-flag", i);
		if (ret == -1)
			return -ENODEV;

		ret = of_property_read_u32_array(np, filename,
						 &flag_data[0], 2UL);
		if (ret != 0)
			return -ENODEV;

		idle_flag_addr[i] = base_addr + flag_data[0];
		if (idle_flag_addr[i] == NULL) {
			pr_err("pwron_mask_init : ioremap addr%d error.\n", i);
			return -ENOMEM;
		}

		idle_flag_bit[i] = flag_data[1];
	}

	return 0;
}

static int __ref cpuidle_decoup_hotplug_start(unsigned int cpu)
{
	set_bit((int)cpu, &g_cpu_on_hotplug);
	kick_all_cpus_sync();
	return 0;
}

static int __ref cpuidle_decoup_hotplug_end(unsigned int cpu)
{
	clear_bit((int)cpu, &g_cpu_on_hotplug);
	kick_all_cpus_sync();
	return 0;
}

#define CLUSTER_IDLE_DRV_NAME_LEN  32
static struct cpuidle_driver cluster_idle_driver[NR_CPUS];

static void __init cluster_wfi_state_init(struct cpuidle_state *pstate_wfi)
{
	/*
	 * State at index 0 is standby wfi and considered standard
	 * on all ARM platforms. If in some platforms simple wfi
	 * can't be used as "state 0", DT bindings must be implemented
	 * to work around this issue and allow installing a special
	 * handler for idle state index 0.
	 */
	pstate_wfi->enter = enter_idle_state;
	pstate_wfi->exit_latency = 1;
	pstate_wfi->target_residency = 1;
	pstate_wfi->power_usage = (int)UINT_MAX;
	(void)strncpy_s(pstate_wfi->name, CPUIDLE_NAME_LEN, "WFI",
			sizeof("WFI"));
	(void)strncpy_s(pstate_wfi->desc, CPUIDLE_DESC_LEN, "ARM64 WFI",
			sizeof("ARM64 WFI"));
}

static int __init cluster_idle_driver_init(void)
{
	int ret, cpu;
	int cluster_num = 0;
	char *drv_name = NULL;
	struct cpumask drv_init_cpumask;
	struct cpuidle_driver *drv = NULL;

	cpumask_clear(&drv_init_cpumask);
	ret = memset_s(cluster_idle_driver,
		       sizeof(cluster_idle_driver), 0,
		       sizeof(cluster_idle_driver));
	if (ret != EOK)
		return -ENODEV;

	for_each_possible_cpu(cpu) { //lint !e574
		if (cpumask_test_cpu(cpu, &drv_init_cpumask))
			continue;

		drv_name = kzalloc(CLUSTER_IDLE_DRV_NAME_LEN, GFP_KERNEL);
		if (drv_name != NULL) {
			ret = snprintf_s(drv_name,
					 CLUSTER_IDLE_DRV_NAME_LEN,
					 CLUSTER_IDLE_DRV_NAME_LEN - 1,
					 "cluster%d_idle_driver",
					 cluster_num);
			if (ret == -1) {
				kfree(drv_name);
				return -ENODEV;
			}
		}

		drv = &cluster_idle_driver[cluster_num++];
		drv->owner = THIS_MODULE;
		drv->name  = drv_name;
		drv_name   = NULL;

		cluster_wfi_state_init(&drv->states[0]);

		ret = multidrv_idle_init(drv,
					      topology_physical_package_id(cpu));
		if (ret != 0) {
			pr_err("fail to register cluster cpuidle drv.ret:%d\n",
			       ret);
			return ret;
		}

		cpumask_or(&drv_init_cpumask, &drv_init_cpumask, drv->cpumask);

		if (cpumask_weight(&drv_init_cpumask) >= NR_CPUS)
			break;
	}

	return 0;
}

/*lint +e785*/
/*
 * Registers the multi cpuidle driver with the cpuidle
 * framework. It relies on core code to parse the idle states
 * and initialize them using driver data structures accordingly.
 */
static int __init lpcpu_idle_init(void)
{
	int ret;

	ret = get_idle_mask_init();
	if (ret != 0)
		pr_err("pwron mask init err%d\n", ret);

	spin_lock_init(&g_idle_spin_lock);
	cpumask_clear(&g_idle_cpus_mask);
#ifdef CONFIG_HISI_CPUIDLE_SKIP_ALL_CORE_DOWN
	cpumask_clear(&g_core_idle_cpus_mask);
#endif
	cpumask_clear(&pending_idle_cpumask);

	ret = cluster_idle_driver_init();
	if (ret != 0)
		return ret;

	cpuhp_setup_state_nocalls(CPUHP_BP_MULTIDRV_NOTIFY_PREPARE,
				  "multidrv-cpuidle:notify",
				  cpuidle_decoup_hotplug_start,
				  cpuidle_decoup_hotplug_end);
	cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
				  "multidrv-cpuidle:online/offline",
				  cpuidle_decoup_hotplug_end,
				  cpuidle_decoup_hotplug_start);

	return 0;
}

/*lint -e528 -esym(528,*)*/
device_initcall(lpcpu_idle_init);
/*lint -e528 +esym(528,*)*/
