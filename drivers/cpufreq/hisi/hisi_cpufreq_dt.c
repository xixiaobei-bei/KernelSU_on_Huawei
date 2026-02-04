/*
 * Hisilicon Platforms CPUFREQ-DT support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/cpufreq.h>
#include <linux/of_platform.h>
#include <linux/pm_opp.h>
#include <linux/cpumask.h>

#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/hisi/hifreq_hotplug.h>
#include <linux/hisi/hisi_efuse.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif


#define EFUSE_TABLE_LEN		4
#define EFUSE_ADJUST_LEN	1

enum HW_VERSION {
	HW_VERSION_0,
	HW_VERSION_1,
	HW_VERSION_MAX,
};

#define HW_VERSION_DEFAULT	HW_VERSION_0
#define VERSION_ELEMENTS	1

static unsigned int cpufreq_dt_version = 0;
static unsigned int efuse_hw_version_table[EFUSE_TABLE_LEN] = {
	HW_VERSION_0,
	HW_VERSION_0,
	HW_VERSION_1,
	HW_VERSION_0,
};

static int get_efuse_hw_version(unsigned int *efuse_prof)
{
	int max_prof = 0;/* 1718-1721 */
	int max_prof_adjust = 0;/* 1714-1717 */
	int adjust_symbol = 0;
	int ret;
	unsigned int efuse_val = 0;

	ret = get_efuse_freq_value((unsigned char *)(&efuse_val), 4);
	if (ret) {
		pr_err("%s:read efuse fail, %d.\n", __func__, ret);
		return ret;
	}

	pr_debug("%s:read efuse ok,val:0x%x.\n", __func__, efuse_val);

	/* efuse 0, set to default */
	if (!efuse_val) {
		pr_err("%s:no efuse info\n", __func__);
		*efuse_prof = HW_VERSION_DEFAULT;
		return ret;
	}

	max_prof = (efuse_val >> 4) & 0xf;
	max_prof_adjust = efuse_val & 0x7;
	adjust_symbol = (efuse_val >> 3) & 0x1;
	adjust_symbol = adjust_symbol * (-2) + 1;

	pr_info("max:%d,adjust:%d,symbol:%d.\n",
		max_prof, max_prof_adjust, adjust_symbol);

	if (max_prof >= EFUSE_TABLE_LEN) {
		*efuse_prof = HW_VERSION_DEFAULT;
		pr_err("%s:ATE efuse:%d err.\n", __func__, max_prof);
	} else {
		*efuse_prof = efuse_hw_version_table[max_prof];
	}

	pr_info("origin:%d.\n", *efuse_prof);

	/*adjust limit to 1 level*/
	if (max_prof_adjust > EFUSE_ADJUST_LEN) {
		pr_err("%s:ATE efuse adjust:%d err.\n",
		       __func__, max_prof_adjust);
		return -EINVAL;
	}

	*efuse_prof += adjust_symbol * max_prof_adjust;
	if (*efuse_prof < HW_VERSION_0 ||
	    *efuse_prof >= HW_VERSION_MAX) {
		pr_err("%s:adjust fail %d,set to default.\n",
		       __func__, *efuse_prof);
		*efuse_prof = HW_VERSION_DEFAULT;
	}

	pr_info("adjusted:%d.\n", *efuse_prof);

	return ret;
}

static void check_efuse_hw_version(int *dt_hw_version)
{
	int efuse_hw_version;

	if (get_efuse_hw_version(&efuse_hw_version))
		return;

	if (*dt_hw_version > efuse_hw_version) {
		*dt_hw_version = efuse_hw_version;
		pr_err("%s:soc cap not match dt\n", __func__);
		return;
	}

	pr_info("%s:check success:%d,%d\n", __func__,
		*dt_hw_version, efuse_hw_version);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_CPU_CHIP);
#endif
}


#ifdef CONFIG_HISI_L2_DYNAMIC_RETENTION
struct l2_retention_ctrl {
	u64 l2_retention_backup;
	u64 l2_retention_dis_mask;
	u64 l2_retention_dis_value;
	u32 l2_retention_dis_cluster;
	u32 l2_retention_dis_freq;
};
static struct l2_retention_ctrl *l2_ret_ctrl = NULL;

u64 l2_retention_read(void)
{
	u64 cfg;

	asm volatile ("MRS %0,S3_1_C11_C0_3\n" \
			: "=r"(cfg) \
			: \
			: "memory");

	return cfg;
}

void l2_retention_write(u64 cfg)
{
	asm volatile ("MSR S3_1_C11_C0_3,%0\n" \
			: \
			: "r"(cfg) \
			: "memory");
}

void l2_dynamic_retention_ctrl(int cluster, unsigned int freq)
{
	u64 cfg;

	if (IS_ERR_OR_NULL(l2_ret_ctrl)) {
		pr_err("%s l2_ret_ctrl not init\n", __func__);
		return;
	}

	if (cluster != l2_ret_ctrl->l2_retention_dis_cluster) {
		return;
	}

	if (freq == l2_ret_ctrl->l2_retention_dis_freq) {
		l2_ret_ctrl->l2_retention_backup = l2_retention_read();
		cfg = l2_ret_ctrl->l2_retention_backup & (~(l2_ret_ctrl->l2_retention_dis_mask));
		cfg |= l2_ret_ctrl->l2_retention_dis_value & l2_ret_ctrl->l2_retention_dis_mask;
		l2_retention_write(cfg);
	} else {
		l2_retention_write(l2_ret_ctrl->l2_retention_backup);
	}
}

int l2_dynamic_retention_init(void)
{
	struct device_node *np;
	int ret = -ENODEV;

	l2_ret_ctrl = kzalloc(sizeof(struct l2_retention_ctrl), GFP_KERNEL);
	if (!l2_ret_ctrl) {
		pr_err("%s: alloc l2_retention_ctrl err\n", __func__);
		ret = -ENOMEM;
		goto err_out;
	}

	np = of_find_compatible_node(NULL, NULL, "hisi,l2-retention-dis-freq");
	if (!np) {
		pr_err("[%s] doesn't have hisi,l2-retention-dis-freq node!\n", __func__);
		goto err_out_free;
	}

	ret = of_property_read_u32(np, "dis_retention_cluster", &(l2_ret_ctrl->l2_retention_dis_cluster));
	if (ret) {
		pr_err("[%s]parse dis_retention_cluster fail!\n", __func__);
		goto err_out_free;
	}

	ret = of_property_read_u32(np, "dis_retention_freq", &(l2_ret_ctrl->l2_retention_dis_freq));
	if (ret) {
		pr_err("[%s]parse dis_retention_freq fail!\n", __func__);
		goto err_out_free;
	}

	ret = of_property_read_u64(np, "dis_retention_mask", &(l2_ret_ctrl->l2_retention_dis_mask));
	if (ret) {
		pr_err("[%s]parse dis_retention_mask fail!\n", __func__);
		goto err_out_free;
	}

	ret = of_property_read_u64(np, "dis_retention_value", &(l2_ret_ctrl->l2_retention_dis_value));
	if (ret) {
		pr_err("[%s]parse dis_retention_value fail!\n", __func__);
		goto err_out_free;
	}

	l2_ret_ctrl->l2_retention_backup = l2_retention_read();

	return 0;
err_out_free:
	kfree(l2_ret_ctrl);
	l2_ret_ctrl = NULL;
	of_node_put(np);
err_out:
	return ret;
}
#endif

#ifdef CONFIG_HISI_HW_VOTE_CPU_FREQ
enum hw_vote_ret_type {
	REG_PMCTRL_TYPE,
};

struct hw_vote_cpu_reg_info {
	unsigned int reg_type;
	unsigned int vote_reg_offset;
	unsigned int vote_bits_mask;
	unsigned int vote_wr_mask;
	unsigned int result_reg_offset;
	unsigned int result_rd_mask;
};

static struct hw_vote_cpu_reg_info *cpu_vote_reg = NULL;
static unsigned int cpu_ch_num = 0;
static void __iomem *pmctrl_base = NULL;

#endif


int hisi_cpufreq_set_supported_hw(struct cpufreq_policy *policy)
{
	int ret, cpu;
	struct device *cpu_dev;

	/* find first cpu of policy->cpus */
	cpu = cpumask_any(policy->cpus);
	cpu_dev = get_cpu_device(cpu);
	if (!cpu_dev) {
		pr_err("%s Failed to get cpu %d device!\n", __func__, cpu);
		return -ENODEV;
	}

	ret = dev_pm_opp_set_supported_hw(cpu_dev, &cpufreq_dt_version, VERSION_ELEMENTS);
	if (ret)
		pr_err("%s Failed to set supported hardware\n", __func__);

	return ret;
}

void hisi_cpufreq_put_supported_hw(struct cpufreq_policy *policy)
{
	int cpu, j;
	struct device *cpu_dev;

	/* find last cpu of policy->related_cpus */
	for_each_cpu(j, policy->related_cpus) {
		cpu = j;
	}
	cpu_dev = get_cpu_device(cpu);
	if (!cpu_dev) {
		pr_err("%s Failed to get cpu %d device!\n", __func__, cpu);
		return;
	}

	dev_pm_opp_put_supported_hw(cpu_dev);
}

static int hisi_cpufreq_get_dt_version(void)
{
	const char *target_cpu;
	int ret, index;
	struct device_node *np;
	bool support_efuse = false;

	np = of_find_compatible_node(NULL, NULL, "hisi,targetcpu");
	if (!np) {
		pr_err("%s Failed to find compatible node:targetcpu\n", __func__);
		return -ENODEV;
	}

	ret = of_property_read_string(np, "target_cpu", &target_cpu);
	if (ret) {
		pr_err("%s Failed to read target_cpu\n", __func__);
		of_node_put(np);
		return ret;
	}
	of_node_put(np);

	np = of_find_compatible_node(NULL, NULL, "hisi,supportedtarget");
	if (!np) {
		pr_err("%s Failed to find compatible node:supportedtarget\n", __func__);
		return -ENODEV;
	}

	ret = of_property_match_string(np, "support_name", target_cpu);
	if (ret < 0) {
		pr_err("%s Failed to get support_name\n", __func__);
		of_node_put(np);
		return ret;
	}
	index = ret;

	support_efuse = of_property_read_bool(np, "support_efuse");
	if (support_efuse)
		check_efuse_hw_version(&index);

	of_node_put(np);

	cpufreq_dt_version = BIT(index);

	return 0;
}

void hisi_cpufreq_get_suspend_freq(struct cpufreq_policy *policy)
{
	struct device_node *np;
	unsigned int value;
	int cluster, ret;

	np = of_find_compatible_node(NULL, NULL, "hisi,suspend-freq");
	if (!np)
		return;

	cluster = topology_physical_package_id(policy->cpu);
	ret = of_property_read_u32_index(np, "suspend_freq", cluster, &value);
	of_node_put(np);

	/* This overides the suspend opp */
	if (!ret)
		policy->suspend_freq = value;
}

#ifdef CONFIG_HISI_HW_VOTE_CPU_FREQ

#define FREQ_VOTE_RATIO		(16U)
#define KHz		(1000)

int hisi_hw_vote_target(struct cpufreq_policy *policy, unsigned int index)
{
	int cluster;
	unsigned int freq_cfg;
	void __iomem *reg;
	int shift, width;
	unsigned int mask;

	if (NULL == policy) {
		return -EINVAL;
	}

	cluster = topology_physical_package_id(policy->cpu);
	if ((unsigned int)cluster >= cpu_ch_num || NULL == cpu_vote_reg) {
		pr_err("%s: hw vote cpu info error\n", __func__);
		return -EINVAL;
	}

	if (0 == cpu_vote_reg[cluster].vote_reg_offset) {
		pr_err("%s: pmctrl vote reg not remap\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_HISI_L2_DYNAMIC_RETENTION
	l2_dynamic_retention_ctrl(cluster, policy->freq_table[index].frequency);
#endif

	freq_cfg = (policy->freq_table[index].frequency / KHz) / FREQ_VOTE_RATIO;

	if (REG_PMCTRL_TYPE == cpu_vote_reg[cluster].reg_type) {
		reg   = pmctrl_base + cpu_vote_reg[cluster].vote_reg_offset;
		shift = ffs(cpu_vote_reg[cluster].vote_bits_mask);
		if (shift <= 0) {
			pr_err("%s: cluster%d result reg mask error\n", __func__, cluster);
			return -EINVAL;
		}
		shift -= 1;
		width = fls(cpu_vote_reg[cluster].vote_bits_mask) - shift;
		mask  = (1 << width) - 1;
		writel(cpu_vote_reg[cluster].vote_wr_mask | ((freq_cfg & mask) << shift), reg);
	}

	return 0;
}

unsigned int hisi_cpufreq_get(unsigned int cpu)
{
	struct cpufreq_policy *policy = cpufreq_cpu_get_raw(cpu);

	if (!policy) {
		pr_err("%s: No policy associated to cpu: %d\n", __func__, cpu);
		return 0;
	}

	return policy->cur;
}

int hisi_cpufreq_policy_cur_init(struct cpufreq_policy *policy)
{
	int cluster;
	unsigned int target_freq = 0, index;
	int ret;
	void __iomem *reg;
	unsigned int shift;

	if (!policy) {
		pr_err("%s: policy is NULL\n", __func__);
		return -EINVAL;
	}

	cluster = topology_physical_package_id(policy->cpu);
	if ((unsigned int)cluster >= cpu_ch_num || NULL == cpu_vote_reg) {
		pr_err("%s: hw vote cpu info error\n", __func__);
		policy->cur = policy->freq_table[0].frequency;
		return -EINVAL;
	}

	if (0 == cpu_vote_reg[cluster].result_reg_offset) {
		policy->cur = policy->freq_table[0].frequency;
		pr_err("%s: pmctrl result reg not remap\n", __func__);
		return -EINVAL;
	}

	if (REG_PMCTRL_TYPE == cpu_vote_reg[cluster].reg_type) {
		reg   = pmctrl_base + cpu_vote_reg[cluster].result_reg_offset;
		shift = ffs(cpu_vote_reg[cluster].result_rd_mask);
		if (shift <= 0) {
			policy->cur = policy->freq_table[0].frequency;
			pr_err("%s: cluster%d result reg mask error\n", __func__, cluster);
			return -EINVAL;
		}
		shift -= 1;
		target_freq = (readl(reg) & cpu_vote_reg[cluster].result_rd_mask) >> shift;
	}

	target_freq = target_freq * KHz * FREQ_VOTE_RATIO;

	ret = cpufreq_frequency_table_target(policy, policy->freq_table, target_freq, CPUFREQ_RELATION_C, &index);

	if (0 == ret) {
		policy->cur = policy->freq_table[index].frequency;
	} else {
		pr_err("%s: find freq fail%d\n", __func__, ret);
		policy->cur = policy->freq_table[0].frequency;
	}

	return ret;
}

static int hw_vote_cpu_reg_remap(void)
{
	struct device_node *np;
	int ret;
	int elem_num;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	if (!np) {
		pr_err("[%s] doesn't have pmctrl node!\n", __func__);
		return -ENODEV;
	}

	pmctrl_base = of_iomap(np, 0);
	if (NULL == pmctrl_base) {
		pr_err("[%s]: ioremap fail!\n", __func__);
		return -EINVAL;
	}

	np = of_find_compatible_node(NULL, NULL, "hisi,freq-hw-vote-cpu");
	if (!np) {
		pr_err("[%s] doesn't have freq-hw-vote-cpu node!\n", __func__);
		return -ENODEV;
	}

	ret = of_property_read_u32_array(np, "vote_channel_num", &cpu_ch_num, 1);
	if (ret) {
		pr_err("[%s]parse vote_channel_num fail!\n", __func__);
		return -ENODEV;
	}

	elem_num = of_property_count_elems_of_size(np, "vote_reg_info", sizeof(unsigned int));
	if ((cpu_ch_num * (sizeof(struct hw_vote_cpu_reg_info) / sizeof(unsigned int))) != elem_num) {
		pr_err("[%s]vote_reg_info & struct un-match\n", __func__);
		return -ENODEV;
	}

	cpu_vote_reg = kzalloc(sizeof(struct hw_vote_cpu_reg_info) * cpu_ch_num, GFP_KERNEL);
	if (NULL == cpu_vote_reg) {
		pr_err("[%s]kzalloc hw_vote_cpu_reg_info buffer fail!\n", __func__);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "vote_reg_info", (unsigned int*)cpu_vote_reg, elem_num);
	if (ret) {
		pr_err("[%s]parse vote_reg_info fail!\n", __func__);
		kfree(cpu_vote_reg);
		cpu_vote_reg = NULL;
		return -ENODEV;
	}

	return 0;
}
#endif

static int hisi_cpufreq_init(void)
{
	int ret = 0;
	struct platform_device *pdev;

	if (!of_find_compatible_node(NULL, NULL, "arm,generic-bL-cpufreq"))
		return -ENODEV;

#ifdef CONFIG_HISI_L2_DYNAMIC_RETENTION
	l2_dynamic_retention_init();
#endif

#ifdef CONFIG_HISI_HW_VOTE_CPU_FREQ
	ret = hw_vote_cpu_reg_remap();
	if (ret)
		return ret;
#endif

	ret = hisi_cpufreq_get_dt_version();
	if (ret)
		return -EINVAL;

#ifdef CONFIG_HISI_BIG_MAXFREQ_HOTPLUG
	bL_hifreq_hotplug_init();
#endif

	pdev = platform_device_register_simple("cpufreq-dt", -1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	return ret;
}
module_init(hisi_cpufreq_init);
