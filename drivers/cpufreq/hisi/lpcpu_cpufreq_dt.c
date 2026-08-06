/*
 * lpcpu_cpufreq_dt.c
 *
 * cpufreq module about device tree
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
 *
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
#include <linux/hisi/hisi_cpufreq_req.h>
#include <linux/version.h>

#ifdef CONFIG_LPCPU_HW_VOTE_CPU_FREQ
#include <linux/hisi/hw_vote.h>
#endif

#define CREATE_TRACE_POINTS
#include <trace/events/hisi_cpufreq_req.h>

#define VERSION_ELEMENTS	1
static unsigned int g_cpufreq_dt_version = 0;

static int cpufreq_freq_req_policy_notifier(struct notifier_block *nb,
					    unsigned long event, void *data)
{
	struct cpufreq_req *req = container_of(nb, struct cpufreq_req, nb);
	struct cpufreq_policy *policy = (struct cpufreq_policy *)data;

	if (event != CPUFREQ_ADJUST ||
	    !cpumask_test_cpu(req->cpu, policy->cpus))
		return NOTIFY_DONE;

	trace_cpufreq_req_notify(req->cpu, (void *)req, req->freq,
				 policy->min, policy->max);

	cpufreq_verify_within_limits(policy, req->freq,
				     policy->max);

	return NOTIFY_OK;
}

int hisi_cpufreq_init_req(struct cpufreq_req *req, int cpu)
{
	if (unlikely(IS_ERR_OR_NULL(req)))
		return -EINVAL;

	req->cpu = cpu;
	req->freq = 0;
	req->nb.priority = 100;
	req->nb.notifier_call = cpufreq_freq_req_policy_notifier;
	return cpufreq_register_notifier(&req->nb, CPUFREQ_POLICY_NOTIFIER);
}
EXPORT_SYMBOL(hisi_cpufreq_init_req);

void hisi_cpufreq_update_req(struct cpufreq_req *req, unsigned int freq)
{
	if (unlikely(IS_ERR_OR_NULL(req)))
		return;

	req->freq = freq;
	trace_cpufreq_req_update(req->cpu, (void *)req, freq);
	cpufreq_update_policy(req->cpu);
}
EXPORT_SYMBOL(hisi_cpufreq_update_req);

void hisi_cpufreq_exit_req(struct cpufreq_req *req)
{
	if (unlikely(IS_ERR_OR_NULL(req)))
		return;

	cpufreq_unregister_notifier(&req->nb, CPUFREQ_POLICY_NOTIFIER);
}
EXPORT_SYMBOL(hisi_cpufreq_exit_req);

#ifdef CONFIG_LPCPU_L2_DYNAMIC_RETENTION
struct l2_retention_ctrl {
	u64 l2_retention_backup;
	u64 l2_retention_dis_mask;
	u64 l2_retention_dis_value;
	u32 l2_retention_dis_cluster;
	u32 l2_retention_dis_freq;
};

static struct l2_retention_ctrl *g_l2_ret_ctrl = NULL;

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

void l2_dynamic_retention_ctrl(struct cpufreq_policy *policy, unsigned int freq)
{
	u64 cfg;
	int cluster;

	if (IS_ERR_OR_NULL(policy)) {
		pr_err("%s policy not found\n", __func__);
		return;
	}
	if (IS_ERR_OR_NULL(g_l2_ret_ctrl)) {
		pr_err("%s g_l2_ret_ctrl not init\n", __func__);
		return;
	}

	cluster = topology_physical_package_id(policy->cpu);
	if (cluster != g_l2_ret_ctrl->l2_retention_dis_cluster)
		return;

	if (freq == g_l2_ret_ctrl->l2_retention_dis_freq) {
		g_l2_ret_ctrl->l2_retention_backup = l2_retention_read();
		cfg = g_l2_ret_ctrl->l2_retention_backup &
		      (~(g_l2_ret_ctrl->l2_retention_dis_mask));
		cfg |= g_l2_ret_ctrl->l2_retention_dis_value &
		       g_l2_ret_ctrl->l2_retention_dis_mask;
		l2_retention_write(cfg);
	} else {
		l2_retention_write(g_l2_ret_ctrl->l2_retention_backup);
	}
}

int l2_dynamic_retention_init(void)
{
	struct device_node *np = NULL;
	int ret = -ENODEV;

	g_l2_ret_ctrl = kzalloc(sizeof(struct l2_retention_ctrl), GFP_KERNEL);
	if (g_l2_ret_ctrl == NULL) {
		pr_err("%s: alloc l2_retention_ctrl err\n", __func__);
		return -ENOMEM;
	}

	np = of_find_compatible_node(NULL, NULL, "hisi,l2-retention-dis-freq");
	if (np == NULL) {
		pr_err("[%s] doesn't have hisi,l2-retention-dis-freq node!\n",
		       __func__);
		goto err_out_free;
	}

	ret = of_property_read_u32(np, "dis_retention_cluster",
				   &g_l2_ret_ctrl->l2_retention_dis_cluster);
	if (ret != 0) {
		pr_err("[%s]parse dis_retention_cluster fail!\n", __func__);
		goto err_node_put;
	}

	ret = of_property_read_u32(np, "dis_retention_freq",
				   &g_l2_ret_ctrl->l2_retention_dis_freq);
	if (ret != 0) {
		pr_err("[%s]parse dis_retention_freq fail!\n", __func__);
		goto err_node_put;
	}

	ret = of_property_read_u64(np, "dis_retention_mask",
				   &g_l2_ret_ctrl->l2_retention_dis_mask);
	if (ret != 0) {
		pr_err("[%s]parse dis_retention_mask fail!\n", __func__);
		goto err_node_put;
	}

	ret = of_property_read_u64(np, "dis_retention_value",
				   &g_l2_ret_ctrl->l2_retention_dis_value);
	if (ret != 0) {
		pr_err("[%s]parse dis_retention_value fail!\n", __func__);
		goto err_node_put;
	}

	g_l2_ret_ctrl->l2_retention_backup = l2_retention_read();

	return 0;
err_node_put:
	of_node_put(np);
err_out_free:
	kfree(g_l2_ret_ctrl);
	g_l2_ret_ctrl = NULL;
	return ret;
}
#endif

struct opp_table *lpcpu_cpufreq_set_supported_hw(struct device *cpu_dev)
{
	if (IS_ERR_OR_NULL(cpu_dev)) {
		pr_err("%s: cpu_dev is null\n", __func__);
		return ERR_PTR(-ENODEV);
	}

	return dev_pm_opp_set_supported_hw(cpu_dev, &g_cpufreq_dt_version,
					   VERSION_ELEMENTS);
}

void lpcpu_cpufreq_put_supported_hw(struct opp_table *opp_table)
{
	if (IS_ERR_OR_NULL(opp_table)) {
		pr_err("%s: opp_table is null\n", __func__);
		return;
	}

	dev_pm_opp_put_supported_hw(opp_table);
}

static int lpcpu_cpufreq_get_dt_version(void)
{
	const char *target_cpu = NULL;
	int ret, index;
	struct device_node *np = of_find_compatible_node(NULL, NULL,
							 "hisi,targetcpu");

	if (np == NULL) {
		pr_err("%s Failed to find compatible node:targetcpu\n",
		       __func__);
		return -ENODEV;
	}

	ret = of_property_read_string(np, "target_cpu", &target_cpu);
	if (ret != 0) {
		pr_err("%s Failed to read target_cpu\n", __func__);
		of_node_put(np);
		return ret;
	}
	of_node_put(np);

	np = of_find_compatible_node(NULL, NULL, "hisi,supportedtarget");
	if (np == NULL) {
		pr_err("%s Failed to find compatible node:supportedtarget\n",
		       __func__);
		return -ENODEV;
	}

	ret = of_property_match_string(np, "support_name", target_cpu);
	if (ret < 0) {
		pr_err("%s Failed to get support_name\n", __func__);
		of_node_put(np);
		return ret;
	}
	of_node_put(np);

	index = ret;
	g_cpufreq_dt_version = BIT((unsigned int)index);

	return 0;
}

void lpcpu_cpufreq_get_suspend_freq(struct cpufreq_policy *policy)
{
	unsigned int value;
	int cluster, ret;
	struct device_node *np = NULL;

	if (IS_ERR_OR_NULL(policy)) {
		pr_err("%s: policy is null\n", __func__);
		return;
	}

	np = of_find_compatible_node(NULL, NULL, "hisi,suspend-freq");
	if (np == NULL)
		return;

	cluster = topology_physical_package_id(policy->cpu);
	ret = of_property_read_u32_index(np, "suspend_freq", cluster, &value);
	of_node_put(np);

	/* This overides the suspend opp */
	if (ret == 0)
		policy->suspend_freq = value;
}

#ifdef CONFIG_LPCPU_HW_VOTE_CPU_FREQ
struct hvdev *lpcpu_cpufreq_hv_init(struct device *cpu_dev)
{
	struct device_node *np = NULL;
	struct hvdev *cpu_hvdev = NULL;
	const char *ch_name = NULL;
	const char *vsrc = NULL;
	int ret;

	if (IS_ERR_OR_NULL(cpu_dev)) {
		pr_err("%s: cpu_dev is null!\n", __func__);
		goto err_out;
	}

	np = cpu_dev->of_node;
	if (IS_ERR_OR_NULL(np)) {
		pr_err("%s: cpu_dev no dt node!\n", __func__);
		goto err_out;
	}

	ret = of_property_read_string_index(np, "freq-vote-channel", 0,
					    &ch_name);
	if (ret != 0) {
		pr_err("[%s]parse freq-vote-channel fail!\n", __func__);
		goto err_out;
	}

	ret = of_property_read_string_index(np, "freq-vote-channel", 1, &vsrc);
	if (ret != 0) {
		pr_err("[%s]parse vote src fail!\n", __func__);
		goto err_out;
	}

	cpu_hvdev = hvdev_register(cpu_dev, ch_name, vsrc);
	if (IS_ERR_OR_NULL(cpu_hvdev))
		pr_err("%s: cpu_hvdev register fail!\n", __func__);

err_out:
	return cpu_hvdev;
}

void lpcpu_cpufreq_hv_exit(struct hvdev *cpu_hvdev, unsigned int cpu)
{
	if (hvdev_remove(cpu_hvdev) != 0)
		pr_err("cpu%u unregister hvdev fail\n", cpu);
}

int lpcpu_cpufreq_set(struct hvdev *cpu_hvdev, unsigned int freq)
{
	if (IS_ERR_OR_NULL(cpu_hvdev))
		return -ENODEV;

	return hv_set_freq(cpu_hvdev, freq);
}

unsigned int lpcpu_cpufreq_get(unsigned int cpu)
{
	struct cpufreq_policy *policy = cpufreq_cpu_get_raw(cpu);

	if (policy == NULL) {
		pr_err("%s: No policy associated to cpu: %u\n", __func__, cpu);
		return 0;
	}

	return policy->cur;
}

void lpcpu_cpufreq_policy_cur_init(struct hvdev *cpu_hvdev,
				   struct cpufreq_policy *policy)
{
	int ret;
	unsigned int freq_khz = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	unsigned int index = 0;
#endif

	if (IS_ERR_OR_NULL(policy)) {
		pr_err("%s: policy is null\n", __func__);
		return;
	}

	ret = hv_get_result(cpu_hvdev, &freq_khz);
	if (ret != 0)
		goto exception;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	/*
	 * On kernel 4.19, policy->min/max hasn't be initialized until now,
	 * thus, cpufreq_frequency_table_target() cannot be called. Policy->cur
	 * will be corrected with the CPUFREQ_NEED_INITIAL_FREQ_CHECK flag set.
	 */
	policy->cur = freq_khz;
#else
	index = cpufreq_frequency_table_target(policy, freq_khz,
					       CPUFREQ_RELATION_C);

	policy->cur = policy->freq_table[index].frequency;
	/* update last freq in hv driver */
	(void)hv_set_freq(cpu_hvdev, policy->cur);
#endif

	return;
exception:
	pr_err("%s:find freq%u fail: %d\n", __func__, freq_khz, ret);
	policy->cur = policy->freq_table[0].frequency;
	/* update last freq in hv driver */
	(void)hv_set_freq(cpu_hvdev, policy->cur);
}
#endif

int lpcpu_cpufreq_init(void)
{
	int ret;
	struct device_node *np = of_find_compatible_node(NULL, NULL,
							 "arm,generic-bL-cpufreq");

	if (np == NULL)
		return -ENODEV;

#ifdef CONFIG_LPCPU_L2_DYNAMIC_RETENTION
	l2_dynamic_retention_init();
#endif

	ret = lpcpu_cpufreq_get_dt_version();

	of_node_put(np);
	return ret;
}
