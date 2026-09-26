/*
 * pm.c
 *
 * suspend
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
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

#include <linux/version.h>
#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/err.h>
#include <asm/suspend.h>
#include <linux/platform_device.h>
#include <linux/cpu_pm.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <asm/cpu_ops.h>
#include <linux/psci.h>
#include <asm/cputype.h>
#include <soc_sctrl_interface.h>
#include <soc_crgperiph_interface.h>

#include <soc_acpu_baseaddr_interface.h>
#include <pm_def.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/wakeup_reason.h>
#ifdef CONFIG_SR_DEBUG
#include <linux/mfd/hisi_pmic.h>
#include "lpregs.h"
#endif

#ifdef CONFIG_HUAWEI_DUBAI
#include <huawei_platform/power/dubai/dubai_wakeup_stats.h>
#endif


#include <soc_gpio_interface.h>
#include <securec.h>

#define BUFFER_LEN		40

#define REG_SCBAKDATA8_OFFSET		SOC_SCTRL_SCBAKDATA8_ADDR(0)
#define REG_SCBAKDATA9_OFFSET		SOC_SCTRL_SCBAKDATA9_ADDR(0)

#ifdef CONFIG_PM_FCM
#define cpuidle_flag_reg(cluster)		REG_SCBAKDATA8_OFFSET
#else
#define cpuidle_flag_reg(cluster)	(((cluster) == 0) ? REG_SCBAKDATA8_OFFSET : \
					 REG_SCBAKDATA9_OFFSET)
#endif

#define AP_SUSPEND_FLAG		BIT(16)

#define IRQ_GROUP_MAX		13
#define IRQ_NUM_PER_WORD		32
#define NO_SEQFILE		0

#define FPGA_FLAG		0x1
#define LITTLE_CLUSTER		0x0
#define PM_GIC_START		2
#define PM_GIC_STEP		4
#define AO_GPIO_GROUP_STEP		8
#define CPU_POWERON_FLAG		0x6
#define CPU_POWERDOWN_FLAG		0x0
#define BOOT_CORE_BIT_START		11
#define CLK_32K_SEC_COUNT		32768
#define BBPRX1_SIZE		0x4
#define AO_GPIO_GROUP_MAX		8
#define AP_RESUME_FLAG		1

#ifdef CONFIG_SR_TICK
#define pmu_write_sr_tick(offset, pos)		pmic_write_reg(offset, pos)
#else
#define pmu_write_sr_tick(offset, pos)
#endif

#define MPIDR_AFFINITY_BITS		8
#ifdef CONFIG_PM_FCM
#define LITTLE_CLUSTER_ID		0
#define MID_CLUSTER_ID		1
#define BIG_CLUSTER_ID		2

#define MPIDR_AFFLVL_MASK		0xff
#define MPIDR_CLUSTER_MASK		(MPIDR_AFFLVL_MASK << MPIDR_AFFINITY_BITS)

#define PLATFORM_CLUSTER0_CORE_COUNT		4
#define PLATFORM_CLUSTER1_CORE_COUNT		2
#define PLATFORM_CLUSTER2_CORE_COUNT		2

#define LITTLE_CLUSTER_BASE		0
#define MID_CLUSTER_BASE	PLATFORM_CLUSTER0_CORE_COUNT
#define BIG_CLUSTER_BASE	(PLATFORM_CLUSTER0_CORE_COUNT + PLATFORM_CLUSTER1_CORE_COUNT)

#define cluster_id(cpuid)	((cpuid) < MID_CLUSTER_BASE ? LITTLE_CLUSTER_ID : \
				 ((cpuid) < BIG_CLUSTER_BASE ? MID_CLUSTER_ID : BIG_CLUSTER_ID))
#else
#define CLUSTER_STEP		4
#define MPIDR_CLUSTER_MASK		0xff
#define MPIDR_CORE_MASK		0xff
#endif

static void __iomem *g_bbpdrx1_base;
static void __iomem *g_sysctrl_base;
static void __iomem *g_enable_base;
static void __iomem *g_pending_base;
static void __iomem *g_crgctrl_base_addr;
static unsigned int g_enable_value[IRQ_GROUP_MAX];
static unsigned int g_ap_irq_num;
static unsigned int g_pm_fpga_flag;
static unsigned int g_ap_suspend_flag;
static unsigned int g_tickmark_s;
static unsigned int g_tickmark_r;
static unsigned int g_resume_time_ms;
const char **g_ap_irq_name;

static int g_ao_gpio_grp_num;
struct ao_gpio_info {
	void __iomem *ao_gpio_base;
	u32 ao_gpio_irq;
	u32 group_idx;
	u32 sec_flag;
};
static struct ao_gpio_info *g_ao_gpio_info_p;

static int pm_ao_gpio_irq_dump(unsigned int irq_num)
{
	int i, index;
	unsigned int data;

	if (g_ao_gpio_grp_num == 0 || g_ao_gpio_info_p == NULL) {
		pr_err("%s: grp num is %d or NULL pointer.\n",
		       __func__, g_ao_gpio_grp_num);
		return -EINVAL;
	}

	for (index = 0; index < g_ao_gpio_grp_num; index++) {
		if (g_ao_gpio_info_p[index].ao_gpio_irq == irq_num)
			break;
		if (index == g_ao_gpio_grp_num - 1) {
			pr_err("%s: irq num %d does not match with dtsi.\n",
			       __func__, irq_num);
			return -EINVAL;
		}
	}
	if (g_ao_gpio_info_p[index].sec_flag == 0) {
		data = readl(g_ao_gpio_info_p[index].ao_gpio_base +
			     SOC_GPIO_GPIOIE_ADDR(0)) &
		       readl(g_ao_gpio_info_p[index].ao_gpio_base +
			     SOC_GPIO_GPIOMIS_ADDR(0));
		for (i = 0; i < AO_GPIO_GROUP_MAX; i++)
			if ((data & BIT((u32)i)) != 0)
				return (int)(g_ao_gpio_info_p[index].group_idx *
					     AO_GPIO_GROUP_STEP) + i;
	}
	return  -EINVAL;
}


static void pm_gic_dump(void)
{
	unsigned int i;

	for (i = PM_GIC_START; i < IRQ_GROUP_MAX; i++)
		g_enable_value[i] = readl(g_enable_base + i * PM_GIC_STEP);
}

static void pm_gic_pending_dump(void)
{
	unsigned int i, j;
	unsigned int value;
	unsigned int irq;
	int gpio;

	for (i = PM_GIC_START; i < IRQ_GROUP_MAX; i++) {
		value = readl(g_pending_base + i * PM_GIC_STEP);

		for (j = 0; j < IRQ_NUM_PER_WORD; j++) {
			if ((value & BIT_MASK(j)) != 0 &&
			    (value & BIT_MASK(j)) ==
			    (g_enable_value[i] & BIT_MASK(j))) {
				irq = i * IRQ_NUM_PER_WORD + j;
				if (irq < g_ap_irq_num) {
					pr_info("wake up irq num: %d, irq name: %s",
						irq, g_ap_irq_name[irq]);
					log_wakeup_reason((int)irq);
				} else {
					pr_info("wake up irq num: %d, irq name: no name!",
						irq);
				}
				gpio = pm_ao_gpio_irq_dump(irq);
				if (gpio >= 0)
					pr_info("(gpio-%d)", gpio);
#ifdef CONFIG_HUAWEI_DUBAI
				if (irq < g_ap_irq_num)
					dubai_log_irq_wakeup(DUBAI_IRQ_WAKEUP_TYPE_AP, g_ap_irq_name[irq], gpio);
#endif
				pr_info("\n");
			}
		}
	}
}

static void set_ap_suspend_flag(unsigned int cluster)
{
	unsigned int val;

	/* do not need lock, as the core is only one now. */
	val = readl(g_sysctrl_base + cpuidle_flag_reg(cluster));
	val |= AP_SUSPEND_FLAG;
	writel(val, g_sysctrl_base + cpuidle_flag_reg(cluster));
}

static void clear_ap_suspend_flag(unsigned int cluster)
{
	unsigned int val;

	/* do not need lock, as the core is only one now. */
	val = readl(g_sysctrl_base + cpuidle_flag_reg(cluster));
	val &= ~AP_SUSPEND_FLAG;
	writel(val, g_sysctrl_base + cpuidle_flag_reg(cluster));
}

static int test_pwrdn_othercores(unsigned int cluster, unsigned int core)
{
	unsigned int pwrack_stat;
	unsigned int mask;
	unsigned int a53_power_stat, maia_power_stat;

	pwrack_stat = readl(SOC_CRGPERIPH_PERPWRACK_ADDR(g_crgctrl_base_addr));
	if (g_pm_fpga_flag == FPGA_FLAG) {
		a53_power_stat = readl(SOC_CRGPERIPH_A53_COREPOWERSTAT_ADDR(g_crgctrl_base_addr));
		maia_power_stat = readl(SOC_CRGPERIPH_MAIA_COREPOWERSTAT_ADDR(g_crgctrl_base_addr));
		pr_err("[%s-cluster%d] Only For fpga: don't check cores' power state in sr!",
		       __func__, cluster);
		pr_err("A53_COREPOWERSTAT:%x, MAIA_COREPOWERSTAT:%x, PERPWRACK:%x\n",
		       a53_power_stat, maia_power_stat, pwrack_stat);

		if (cluster == LITTLE_CLUSTER)
			return (a53_power_stat != CPU_POWERON_FLAG ||
				maia_power_stat != CPU_POWERDOWN_FLAG);
		else
			return (a53_power_stat != CPU_POWERDOWN_FLAG ||
				maia_power_stat != CPU_POWERON_FLAG);

	} else {
		/* boot core mask */
#ifdef CONFIG_PM_FCM
		mask = BIT(BOOT_CORE_BIT_START + core);
#else
		mask = BIT(BOOT_CORE_BIT_START + cluster * CLUSTER_STEP + core);
#endif
		/* non boot core mask */
		mask = (unsigned int)COREPWRACK_MASK & (~mask);
		pwrack_stat &= mask;

		return pwrack_stat;
	}
}

#define SR_POWER_STATE_SUSPEND		0x01010000

static int sr_psci_suspend(unsigned long index)
{
	return psci_ops.cpu_suspend(SR_POWER_STATE_SUSPEND,
				    virt_to_phys(cpu_resume));
}

static void pm_cpu_suspend(void)
{
	cpu_suspend(0, sr_psci_suspend);
}

static int hisi_pm_enter(suspend_state_t state)
{
	unsigned int cluster;
	unsigned int core;
	unsigned long mpidr = read_cpuid_mpidr();

	pmu_write_sr_tick(PMUOFFSET_SR_TICK, KERNEL_SUSPEND_IN);
	pr_err("%s ++\n", __func__);
#ifdef CONFIG_PM_FCM
	core = (mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;
	cluster = cluster_id(core);
#else
	cluster = (mpidr >> MPIDR_AFFINITY_BITS) & MPIDR_CLUSTER_MASK;
	core = mpidr & MPIDR_CORE_MASK;
#endif

	pr_debug("%s: mpidr is 0x%lx, cluster = %d, core = %d.\n",
		 __func__, mpidr, cluster, core);

	pm_gic_dump();
#ifdef CONFIG_SR_DEBUG
	get_ip_regulator_state();
	pmclk_monitor_enable();
#endif
	while (test_pwrdn_othercores(cluster, core) != 0)
		;
	pmu_write_sr_tick(PMUOFFSET_SR_TICK, KERNEL_SUSPEND_SETFLAG);
	g_ap_suspend_flag = 1;
	set_ap_suspend_flag(cluster);
	cpu_cluster_pm_enter();
	pm_cpu_suspend();
	g_tickmark_s = readl(g_bbpdrx1_base);
	cpu_cluster_pm_exit();
	clear_ap_suspend_flag(cluster);
#ifdef CONFIG_SR_DEBUG
	debuguart_reinit();
	pr_info("%s tick: 0x%x, 0x%x, 0x%x\n",
		__func__, g_tickmark_s, readl(g_bbpdrx1_base),
		readl(g_bbpdrx1_base) - g_tickmark_s);
	pm_gic_pending_dump();
	pm_status_show(NO_SEQFILE);
#endif
	pr_err("%s --\n", __func__);
	pmu_write_sr_tick(PMUOFFSET_SR_TICK, KERNEL_RESUME);

	return 0;
}

static const struct platform_suspend_ops pm_ops = {
	.enter = hisi_pm_enter,
	.valid = suspend_valid_only_mem,
};

static int get_gic_base(void)
{
	void __iomem *gic_dist_base = NULL;
	struct device_node *node = NULL;
	u32 enable_offset = 0;
	u32 pending_offset = 0;
	int ret;

	node = of_find_compatible_node(NULL, NULL, "arm,lp_gic");
	if (node == NULL) {
		pr_err("%s: gic No compatible node found\n", __func__);
		return -ENODEV;
	}

	ret = of_property_read_u32(node, "enable-offset", &enable_offset);
	if (ret != 0) {
		pr_err("%s: no enable-offset!\n", __func__);
		goto err_put_node;
	}

	ret = of_property_read_u32(node, "pending-offset", &pending_offset);
	if (ret != 0)  {
		pr_err("%s: no pending-offset!\n", __func__);
		goto err_put_node;
	}

	gic_dist_base = of_iomap(node, 0);
	if (gic_dist_base == NULL) {
		of_node_put(node);
		pr_err("%s: gic_dist_base is NULL\n", __func__);
		return -ENODEV;
	}

	of_node_put(node);

	g_enable_base = gic_dist_base + enable_offset;
	g_pending_base = gic_dist_base + pending_offset;

	pr_info("g_enable_base = %pK, g_pending_base = %pK.\n",
		g_enable_base, g_pending_base);

	return 0;

err_put_node:
	of_node_put(node);

	pr_err("%s failed.\n", __func__);
	return ret;
}

static int init_lowpm_table(struct device_node *np)
{
	int ret;
	u32 i;

	ret = of_property_count_strings(np, "ap-irq-table");
	if (ret < 0) {
		pr_err("%s, not find ap-irq-table property!\n", __func__);
		goto err;
	}
	g_ap_irq_num = ret;
	pr_info("%s, ap-irq-table num: %d!\n", __func__, g_ap_irq_num);

	g_ap_irq_name = kzalloc(g_ap_irq_num * sizeof(char *), GFP_KERNEL);
	if (g_ap_irq_name == NULL) {
		pr_err("%s:%d kzalloc err!!\n", __func__, __LINE__);
		ret = -ENOMEM;
		goto err;
	}

	for (i = 0; i < g_ap_irq_num; i++) {
		ret = of_property_read_string_index(np, "ap-irq-table",
						    i, &g_ap_irq_name[i]);
		if (ret != 0) {
			pr_err("%s, no ap-irq-table %d!\n", __func__, i);
			goto err_free;
		}
	}

	pr_info("%s: init lowpm table success.\n", __func__);
	return ret;

err_free:
	kfree(g_ap_irq_name);
	g_ap_irq_name = NULL;
err:
	return ret;
}

static int init_ao_gpio(struct device_node *np)
{
	int i;
	int ret_rd_sec_flag;
	int ret = 0;
	struct device_node *dn = NULL;
	char *io_buffer = NULL;
	int err;

	g_ao_gpio_grp_num = of_property_count_u32_elems(np, "ao-gpio-irq");
	if (g_ao_gpio_grp_num < 0) {
		pr_err("%s[%d], no ao gpio irq!\n", __func__, __LINE__);
		ret = -ENODEV;
		goto err;
	}
	pr_info("%s: g_ao_gpio_grp_num is %d.\n", __func__, g_ao_gpio_grp_num);

	io_buffer = kzalloc(BUFFER_LEN * sizeof(char), GFP_KERNEL);
	if (io_buffer == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	g_ao_gpio_info_p = kzalloc((unsigned long)(g_ao_gpio_grp_num * sizeof(struct ao_gpio_info)),
				   GFP_KERNEL);
	if (g_ao_gpio_info_p == NULL) {
		ret = -ENOMEM;
		goto err_free_buffer;
	}

	for (i = 0; i < g_ao_gpio_grp_num; i++) {
		ret = of_property_read_u32_index(np, "ao-gpio-irq",
						 (unsigned int)i,
						 &(g_ao_gpio_info_p[i].ao_gpio_irq));
		if (ret != 0) {
			pr_err("%s, no ao-gpio-irq, %d.\n", __func__, i);
			goto err_free;
		}
	}

	for (i = 0; i < g_ao_gpio_grp_num; i++) {
		ret = of_property_read_u32_index(np, "ao-gpio-group-idx",
						 (unsigned int)i,
						 &(g_ao_gpio_info_p[i].group_idx));
		if (ret != 0) {
			pr_err("%s, no ao-gpio-group-idx, %d\n", __func__, i);
			goto err_free;
		}
	}

	for (i = 0; i < g_ao_gpio_grp_num; i++) {
		err = memset_s(io_buffer, BUFFER_LEN * sizeof(char),
			       0, BUFFER_LEN * sizeof(char));
		if (err != EOK) {
			pr_err("[%s]memset_s fail[%d]\n", __func__, ret);
			ret = err;
			goto err_free;
		}
		err = snprintf_s(io_buffer, BUFFER_LEN * sizeof(char),
				 (BUFFER_LEN - 1) * sizeof(char),
				 "arm,primecell%u", g_ao_gpio_info_p[i].group_idx);
		if (err < 0) {
			pr_err("[%s]snprintf_s fail[%d]\n", __func__, ret);
			ret = err;
			goto err_free;
		}
		dn = of_find_compatible_node(NULL, NULL, io_buffer);
		if (dn == NULL) {
			pr_err("%s: primecell%d No compatible node found\n",
			       __func__, g_ao_gpio_info_p[i].group_idx);
			ret = -ENODEV;
			goto err_free;
		}
		ret_rd_sec_flag = of_property_read_u32(dn, "secure-mode",
						       &g_ao_gpio_info_p[i].sec_flag);
		if (ret_rd_sec_flag != 0)
			g_ao_gpio_info_p[i].sec_flag = 0;
		else
			pr_info("sec gpio group no: %d\n", i);

		g_ao_gpio_info_p[i].ao_gpio_base = of_iomap(dn, 0);
		if (g_ao_gpio_info_p[i].ao_gpio_base == NULL) {
			ret = -EINVAL;
			of_node_put(dn);
			goto err_free;
		}
		of_node_put(dn);
	}

	kfree(io_buffer);
	pr_info("%s: init ao gpio success.\n", __func__);
	return ret;

err_free:
	kfree(g_ao_gpio_info_p);
	g_ao_gpio_info_p = NULL;
err_free_buffer:
	kfree(io_buffer);
err:
	return ret;
}

static int sr_tick_pm_notify(struct notifier_block *nb,
			     unsigned long mode, void *_unused)
{
	switch (mode) {
	case PM_SUSPEND_PREPARE:
		pmu_write_sr_tick(PMUOFFSET_SR_TICK, KERNEL_SUSPEND_PREPARE);
		break;
	case PM_POST_SUSPEND:
		pmu_write_sr_tick(PMUOFFSET_SR_TICK, KERNEL_RESUME_OUT);
		if (g_ap_suspend_flag == AP_SUSPEND_FLAG) {
			g_tickmark_r = readl(g_bbpdrx1_base);
			g_resume_time_ms = (g_tickmark_r - g_tickmark_s) *
					   1000 / CLK_32K_SEC_COUNT;
		} else {
			g_resume_time_ms = 0;
		}
		g_ap_suspend_flag = 0;
		break;
	default:
		break;
	}

	return 0;
}

static struct notifier_block sr_tick_pm_nb = {
	.notifier_call = sr_tick_pm_notify,
};

static int pm_drvinit(void)
{
	struct device_node *np = NULL;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,lowpm_func");
	if (np == NULL) {
		pr_err("%s: lowpm_func No compatible node found\n",
		       __func__);
		return -ENODEV;
	}

	g_pm_fpga_flag = 0;
	ret = of_property_read_u32_array(np, "fpga_flag", &g_pm_fpga_flag, 1);
	if (ret != 0)
		pr_err("%s , no fpga_flag.\n", __func__);

	ret = init_ao_gpio(np);
	if (ret != 0) {
		of_node_put(np);
		pr_err("%s, init ao GPIO failed!\n", __func__);
		return -ENODEV;
	}

	ret = init_lowpm_table(np);
	if (ret != 0) {
		of_node_put(np);
		pr_err("%s, init lowpm_table err!\n", __func__);
		return -ENODEV;
	}

	of_node_put(np);

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (np == NULL) {
		pr_err("%s: sysctrl No compatible node found\n",
		       __func__);
		return -ENODEV;
	}

	g_sysctrl_base = of_iomap(np, 0);
	if (g_sysctrl_base == NULL) {
		of_node_put(np);
		pr_err("%s: g_sysctrl_base is NULL\n", __func__);
		return -ENODEV;
	}

	of_node_put(np);

	np = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
	if (np == NULL) {
		pr_info("%s: crgctrl No compatible node found\n",
			__func__);
		return -ENODEV;
	}
	g_crgctrl_base_addr = of_iomap(np, 0);
	if (g_crgctrl_base_addr == NULL) {
		of_node_put(np);
		pr_err("%s: g_crgctrl_base_addr is NULL\n", __func__);
		return -ENODEV;
	}

	of_node_put(np);

	if (get_gic_base() != 0) {
		pr_err("%s: get gic base failed!\n", __func__);
		return -ENODEV;
	}
	/*lint -e446*/
	g_bbpdrx1_base = ioremap(SOC_SCTRL_SCBBPDRXSTAT1_ADDR(SOC_ACPU_SCTRL_BASE_ADDR),
				 BBPRX1_SIZE);
	/*lint +e446*/
	if (g_bbpdrx1_base == NULL) {
		pr_err("%s: get SCBBPDRXSTAT1_ADDR failed!\n", __func__);
		return -ENODEV;
	}

	ret = register_pm_notifier(&sr_tick_pm_nb);
	if (ret != 0) {
		pr_err("%s: register_pm_notifier failed!\n", __func__);
		return -ENODEV;
	}

	suspend_set_ops(&pm_ops);

	return 0;
}

static __init int pm_init(void)
{
	int ret;

	ret = pm_drvinit();
	if (ret != 0) {
		/* if not suspend_set_ops ok, don't support suspend to s2idle */
		mem_sleep_current = PM_SUSPEND_ON;
		pr_err("%s: pm_drvinit failed!\n", __func__);
		return -ENODEV;
	}

	return 0;
}

arch_initcall(pm_init);
