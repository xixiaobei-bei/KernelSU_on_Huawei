/*
 * npu_dvfs.c
 *
 * npu dvfs
 *
 * Copyright (C) 2017-2020 Huawei Technologies Co., Ltd.
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

#include <linux/version.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/devfreq.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/pm_opp.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <npu_pm.h>
#include "npu_pm_private.h"

#ifdef CONFIG_THERMAL_TSENSOR
#include <linux/lpm_thermal.h>
#endif
struct npn_clk_list {
	struct list_head list; /* kernel's list structure */
	const char *clock_name;
	struct clk *clock;
};

static struct npn_clk_list g_npn_clk_list;

struct npu_pm_dvfs_data g_dvfs_data = {
	.module_profile = NULL,
	.module_count = 0,
	.main_module = NULL,
	.dvfs_enable = false,
	.temp_stat = TEMP_NORMAL,
	.test = 0,
	.ppll = {
			.base = NULL,
			.clk_name = NULL,
		},
};

#define CONFIG_USE_SOFT_CURR_FREQ	1

#define HISI_NPU_TSENSOR_NAME		"gpu"
#define LOW_TEMP_THRESHOLD	5 /* 5 degree */

#define NPU_PERI_VOL_VOTE_ID	62

#define K_RATIO			1000

static bool npu_is_low_temperature(void)
{
#ifdef CONFIG_THERMAL_TSENSOR
	int temp = 0;
	int ret;
#endif


#ifdef CONFIG_THERMAL_TSENSOR
	if (g_dvfs_data.tsensor >= 0) {
		ret = ipa_get_sensor_value(g_dvfs_data.tsensor, &temp);
		if (ret == 0 && temp < LOW_TEMP_THRESHOLD)
			return true;
	}
#endif

	return false;
}

static int _hal_reg_write(struct clk_div_regs *cfg, unsigned int val)
{
	int shift, width;
	u32 mask;

	shift = ffs(cfg->rd_mask);
	if (shift <= 0)
		return -EPERM;
	shift -= 1;
	width = fls(cfg->rd_mask) - shift;
	mask  = (1 << (unsigned int)width) - 1;
	if (val > mask)
		val = mask;

	writel(cfg->wr_mask | ((val & mask) << (unsigned int)shift),
	       cfg->cfg_reg);

	udelay(1);

	return 0;
}

int _hal_reg_read(const struct clk_div_regs *cfg, u32 *val)
{
	int shift;

	if (cfg == NULL)
		return -EINVAL;

	shift = ffs(cfg->rd_mask);
	if (shift <= 0)
		return -EPERM;
	shift -= 1;
	*val = ((unsigned int)readl(cfg->stat_reg) & cfg->rd_mask) >>
		(unsigned int)shift;

	return 0;
}

/*
 * Extern function used to get NPU voltage for NPU DPM
 * return: mv
 */
int hisi_npu_get_voltage(void)
{
	struct hisi_npu_module_profile *module_profile =
		g_dvfs_data.main_module;

	if (!IS_ERR_OR_NULL(module_profile->buck))
		/* regulator_get_voltage fun unit uv, 1000uv = 1mv */
		return regulator_get_voltage(module_profile->buck) / K_RATIO;
	return 0;
}

int hisi_npu_get_module_voltage(const struct hisi_npu_module_profile *module_profile)
{
	if (!IS_ERR_OR_NULL(module_profile->buck))
		return regulator_get_voltage(module_profile->buck);

	return 0;
}

#define NPU_SET_VOLTAGE_STABLE_TIME		500 /* us */
int hisi_npu_set_module_voltage(const struct hisi_npu_module_profile *module_profile,
				unsigned int voltage)
{
	int ret;
	struct regulator *buck = NULL;

	if (module_profile == NULL) {
		pr_err("[npu dvfs]%s invalid param\n", __func__);
		return 0;
	}

	buck = module_profile->buck;
	if (IS_ERR_OR_NULL(buck))
		return 0;

	ret = regulator_set_voltage(buck, voltage, voltage);
	if (ret != 0)
		return ret;
	udelay(NPU_SET_VOLTAGE_STABLE_TIME);

	return 0;
}

static void dt2clkdiv(const struct clk_div_regs_dt *clk_div_dt,
		      struct clk_div_regs *clk_div_regs,
		      void __iomem *base)
{
	clk_div_regs->cfg_reg = clk_div_dt->cfg_offset + base;
	clk_div_regs->stat_reg = clk_div_dt->stat_offset + base;
	clk_div_regs->rd_mask = clk_div_dt->rd_mask;
	clk_div_regs->wr_mask = clk_div_dt->wr_mask;
}

static void dt2clkgate(const struct clk_gate_regs_dt *clk_gate_dt,
		       struct clk_gate_regs *clk_gate_regs,
		       void __iomem *base)
{
	clk_gate_regs->en_reg = clk_gate_dt->en_offset + base;
	clk_gate_regs->dis_reg = clk_gate_dt->dis_offset + base;
	clk_gate_regs->stat_reg = clk_gate_dt->stat_offset + base;
	clk_gate_regs->mask = clk_gate_dt->mask;
}

int hisi_npu_find_profile_id(const struct hisi_npu_module_profile *module_profile,
			     unsigned long *freq)
{
	int id;

	for (id = 0; id < module_profile->profile_count; id++) {
		if (module_profile->profile_table[id].freq >= *freq) {
			*freq = module_profile->profile_table[id].freq;
			return id;
		}
	}

	id = module_profile->profile_count - 1;
	*freq = module_profile->profile_table[id].freq;

	return id;
}

static struct clk *hisi_npu_get_cur_clk(const struct hisi_npu_module_profile *module_profile)
{
	struct module_clk_list *clk_node = NULL;
	u32 clk_switch = 0;
	int ret;

	ret = _hal_reg_read(&module_profile->clk_switch, &clk_switch);
	if (ret != 0) {
		pr_err("[npu dvfs]%s get clk switch failed\n", __func__);
		return NULL;
	}

	list_for_each_entry(clk_node, &module_profile->clk_list.list, list)
		if (clk_node->clk_sw == clk_switch)
			return clk_node->clk;

	return NULL;
}

unsigned long hisi_npu_get_cur_clk_rate(const struct hisi_npu_module_profile *module_profile)
{
	u32 clk_div;
	struct clk *cur_clk = NULL;
	int ret;
	unsigned int clk_freq_get;

	if (module_profile == NULL) {
		pr_err("[npu dvfs]%s invalid param\n", __func__);
		return 0;
	}

	cur_clk = hisi_npu_get_cur_clk(module_profile);
	if (IS_ERR_OR_NULL(cur_clk)) {
		pr_err("[npu dvfs]%s get pll failed\n", __func__);
		return 0;
	}

	ret = _hal_reg_read(&module_profile->clk_div, &clk_div);
	if (ret != 0) {
		pr_err("[npu dvfs]%s get clk div failed\n", __func__);
		return 0;
	}
	if (g_dvfs_data.ppll.clk_name != NULL &&
	    strcmp(module_profile->profile_table[module_profile->cur_pid].clk_name,
		   g_dvfs_data.ppll.clk_name) == 0)
		clk_freq_get = g_dvfs_data.ppll.cur_freq * KHZ;
	else
		clk_freq_get = clk_get_rate(cur_clk);

	return (clk_freq_get / (clk_div + 1));
}

static int hisi_npu_find_absolute_profile_id(const struct hisi_npu_module_profile *module_profile,
					     unsigned int freq)
{
	int id;

	for (id = 0; id < module_profile->profile_count; id++)
		if (freq == module_profile->profile_table[id].freq)
			return id;

	pr_err("[npu dvfs] freq%u not exist in profile %s\n",
	       freq, module_profile->module_name);

	return -EINVAL;
}

static int hisi_npu_update_temp_stat(void)
{
	bool is_low_temp;
	int stat = TEMP_ERROR;

	is_low_temp = npu_is_low_temperature();
	if (is_low_temp) {
		switch (g_dvfs_data.temp_stat) {
		case TEMP_NORMAL:
			stat = TEMP_LOW_ENTER;
			break;
		case TEMP_LOW:
			stat = TEMP_LOW;
			break;
		default:
			break;
		}
		g_dvfs_data.temp_stat = TEMP_LOW;
	} else {
		switch (g_dvfs_data.temp_stat) {
		case TEMP_NORMAL:
			stat = TEMP_NORMAL;
			break;
		case TEMP_LOW:
			stat = TEMP_LOW_EXIT;
			break;
		default:
			break;
		}
		g_dvfs_data.temp_stat = TEMP_NORMAL;
	}

	return stat;
}

static void hisi_npu_clr_temp_stat(void)
{
	g_dvfs_data.temp_stat = TEMP_NORMAL;
}


static unsigned int get_avs_voltage(struct hisi_npu_module_profile *module_profile,
				    int cur_pid,
				    int tar_pid)
{
#ifdef CONFIG_NPU_PM_AVS_VDM
	unsigned int temp_idx;
	unsigned int pa_volt;
	unsigned int pa_sw = 0;
#endif
	unsigned int tar_voltage;
	struct hisi_npu_profile *profile = module_profile->profile_table;;

#ifdef CONFIG_NPU_PM_AVS_VDM
	if ((profile[tar_pid].func_sw & PROF_AVS) > 0) {
		if (((unsigned int)cur_pid) >= module_profile->pa_ctrl.pa_safe_prof)
			pa_sw = 1;
		pa_volt = npu_cluster_avs(g_dvfs_data.main_module, pa_sw);
		temp_idx = update_avs_temperature_info();
		tar_voltage = npu_volt_cfg2vol(profile[tar_pid].avsvolt[temp_idx]) + pa_volt;
		pr_debug("[npu pasnsor] pa %u, cur%d sw%u\n", pa_volt, cur_pid, pa_sw);
	} else {
		tar_voltage = profile[tar_pid].voltage;
	}
#else
		tar_voltage = profile[tar_pid].voltage;
#endif
	return tar_voltage;
}

int hisi_npu_voltage_step_up(int cur_pid, int tar_pid, int temp_stat)
{
	int ret = 0;
	struct hisi_npu_module_profile *module_profile = NULL;
	struct regulator *buck = NULL;
	unsigned int tar_voltage;
	unsigned int last_voltage;
	unsigned int buck_delay_time;
	int module;

	for (module = 0; module < g_dvfs_data.module_count; module++) {
		module_profile = &g_dvfs_data.module_profile[module];

		npu_update_svfd_code(module_profile, tar_pid);

		buck = module_profile->buck;
		if (IS_ERR_OR_NULL(buck))
			continue;

		if (temp_stat == TEMP_LOW_ENTER)
			tar_voltage = module_profile->voltage_lt;
		else if (temp_stat == TEMP_NORMAL || temp_stat == TEMP_LOW_EXIT)
			tar_voltage = get_avs_voltage(module_profile, cur_pid, tar_pid);
		else
			continue;

		last_voltage = module_profile->last_voltage;
		if (tar_voltage > last_voltage) {
			module_profile->last_voltage = tar_voltage;
			/* up voltage */
			ret = regulator_set_voltage(buck, tar_voltage,
						    tar_voltage);
			if (ret) {
				pr_err("Set tar_voltage=%u failed, ret = %d\n",
				       tar_voltage, ret);
				return ret;
			}
			buck_delay_time =
				(tar_voltage - last_voltage) / module_profile->buck_step_time +
				module_profile->buck_hold_time;

			pr_debug("[npu dvfs]volt up tar%u, cur%u, time%u(us)\n",
				 tar_voltage, last_voltage, buck_delay_time);
			udelay(buck_delay_time);
		}
	}

	return ret;
}

int hisi_npu_voltage_step_down(int cur_pid, int tar_pid, int temp_stat)
{
	int ret = 0;
	struct hisi_npu_module_profile *module_profile = NULL;
	struct regulator *buck = NULL;
	unsigned int tar_voltage;
	unsigned int last_voltage;
	unsigned int buck_delay_time;
	int module;

	for (module = 0; module < g_dvfs_data.module_count; module++) {
		module_profile = &g_dvfs_data.module_profile[module];

		buck = module_profile->buck;
		if (IS_ERR_OR_NULL(buck))
			continue;

		if (temp_stat == TEMP_LOW_ENTER)
			tar_voltage = module_profile->voltage_lt;
		else if (temp_stat == TEMP_NORMAL || temp_stat == TEMP_LOW_EXIT)
			tar_voltage = get_avs_voltage(module_profile, cur_pid, tar_pid);
		else
			continue;

		last_voltage = module_profile->last_voltage;
		if (tar_voltage < last_voltage) {
			module_profile->last_voltage = tar_voltage;
			/* down voltage */
			ret = regulator_set_voltage(buck, tar_voltage,
						    tar_voltage);
			if (ret != 0) {
				pr_err("Set tar_voltage=%u failed, ret = %d\n",
				       tar_voltage, ret);
				return ret;
			}
			buck_delay_time =
				(last_voltage - tar_voltage) / module_profile->buck_step_time +
				module_profile->buck_hold_time;
			pr_debug("[npu dvfs]volt dn:tar%u, cur%d time%u(us)\n",
				 tar_voltage, last_voltage, buck_delay_time);
			udelay(buck_delay_time);
		}
	}

	return ret;
}

int hisi_npu_voltage_update(int cur_pid, int tar_pid, int temp_stat)
{
	struct hisi_npu_module_profile *module_profile = NULL;
	struct regulator *buck = NULL;
	unsigned int tar_voltage;
	unsigned int last_voltage;
	int module;
	int ret = 0;

	for (module = 0; module < g_dvfs_data.module_count; module++) {
		module_profile = &g_dvfs_data.module_profile[module];

		npu_update_svfd_code(module_profile, tar_pid);

		buck = module_profile->buck;
		if (IS_ERR_OR_NULL(buck))
			continue;

		if (temp_stat == TEMP_LOW_ENTER)
			tar_voltage = module_profile->voltage_lt;
		else if (temp_stat == TEMP_LOW_EXIT)
			tar_voltage = get_avs_voltage(module_profile, cur_pid, tar_pid);
		else
			continue;

		last_voltage = module_profile->last_voltage;

		module_profile->last_voltage = tar_voltage;
		ret = regulator_set_voltage(buck, tar_voltage, tar_voltage);
		if (ret != 0)
			return ret;

		if (last_voltage > tar_voltage)
			udelay((last_voltage - tar_voltage) / module_profile->buck_step_time +
			       module_profile->buck_hold_time);
		else
			udelay((tar_voltage - last_voltage) / module_profile->buck_step_time +
			       module_profile->buck_hold_time);
	}

	return ret;
}
static void hisi_npu_set_clk_freq(const struct hisi_npu_module_profile *module_profile,
				  int tar_pid)
{
	unsigned int clk_freq;
	unsigned int delta;
	const char *clk_name = NULL;
	unsigned int clk_count;
	struct hisi_npu_profile *profile = module_profile->profile_table;
	struct ppll_conf *ppll = &g_dvfs_data.ppll;

	clk_name = profile[tar_pid].clk_name;
	if (ppll->base == NULL || ppll->clk_name == NULL)
		return;

	if (strcmp(clk_name, ppll->clk_name) == 0) {
		/* get the clock freq tar_pid need */
		clk_freq = profile[tar_pid].freq * (profile[tar_pid].div + 1);
		delta = clk_freq > ppll->cur_freq ?
			(clk_freq - ppll->cur_freq) :
			(ppll->cur_freq - clk_freq);
		/*
		 * cur clock freq and tar freq calculate may have 10KHz err gap,
		 * only greater than 10KHz means need config clock frequency
		 */
		if (delta > 10 * KHZ) {
			clk_count = clk_get_enable_count(profile[tar_pid].pll_clk);
			if (clk_count > 0) {
				pr_err("[npu dvfs] %s is not free and failed to set %uKHz, clk_count[%u] [%d]->[%d]",
				       clk_name, clk_freq, clk_count,
				       module_profile->cur_pid, tar_pid);
				return;
			}

			if (ppll->cur_freq != ppll->para[0][0]) {
				writel(ppll->para[0][1],
				       ppll->base + ppll->ctrl_offset[0]);
				writel(ppll->para[0][2],
				       ppll->base + ppll->ctrl_offset[1]);
				ppll->cur_freq = ppll->para[0][0];
			} else {
				writel(ppll->para[1][1],
				       ppll->base + ppll->ctrl_offset[0]);
				writel(ppll->para[1][2],
				       ppll->base + ppll->ctrl_offset[1]);
				ppll->cur_freq = ppll->para[1][0];
			}
			pr_debug("[npu dvfs] Set %s clk%s to %u MHz, Tar%d\n",
				 module_profile->module_name, clk_name,
				 ppll->cur_freq / KHZ, tar_pid);
		}
	}
}

/*
 * This function used to check two module clk whether conflict
 * for specified profile.
 * module_a and module_b can be same, and return different profile
 * clk conflict status for this module
 */
static bool hisi_npu_check_clk_freq_conflict(const struct hisi_npu_module_profile *module_a,
					     int pid_a,
					     const struct hisi_npu_module_profile *module_b,
					     int pid_b)
{
	unsigned int clk_freq_a; /* KHz */
	unsigned int clk_freq_b; /* KHz */
	unsigned int delta;

	/* If two module use different clk for specified prof, not conflict */
	if (module_a->profile_table[pid_a].pll_clk !=
	    module_b->profile_table[pid_b].pll_clk)
		return false;

	clk_freq_a = module_a->profile_table[pid_a].freq *
		     (module_a->profile_table[pid_a].div + 1);
	clk_freq_b = module_b->profile_table[pid_b].freq *
		     (module_b->profile_table[pid_b].div + 1);
	delta = clk_freq_b > clk_freq_a ?
		(clk_freq_b - clk_freq_a) : (clk_freq_a - clk_freq_b);
	/*
	 * If delta < 10KHz, means clock not conflict
	 * for module_a cur_pid and module_b tar_pid
	 */
	if (delta < 10 * KHZ)
		return false;

	/* Clock conflict for two module */
	return true;
}

static int hisi_npu_set_module_profile(struct hisi_npu_module_profile *module_profile,
				       int cur_pid, int tar_pid)
{
	unsigned int cur_div;
	unsigned int tar_div;
	unsigned int tar_pll;
	int ret;

	/* Check whether already config tar profile done */
	if (module_profile->cur_pid == tar_pid)
		return 0;

	if (hisi_npu_check_clk_freq_conflict(module_profile, cur_pid,
					     module_profile, tar_pid)) {
		pr_err("[npu dvfs]module[%s] clk conflict[%d]->[%d] set fail\n",
		       module_profile->module_name, cur_pid, tar_pid);
		return -EINVAL;
	}

	cur_div = module_profile->profile_table[cur_pid].div;
	tar_div = module_profile->profile_table[tar_pid].div;
	tar_pll = module_profile->profile_table[tar_pid].pll;

	/* enable target pll */
	ret = clk_prepare_enable(module_profile->profile_table[tar_pid].pll_clk);
	if (ret != 0) {
		pr_err("[npu dvfs] %s %d clk enable fail%d\n",
		       __func__, __LINE__, ret);
		return ret;
	}
	module_profile->cur_clk =
		module_profile->profile_table[tar_pid].pll_clk;

	if (tar_div <= cur_div) {
		/*
		 * switch pll
		 * config div with
		 */
		ret = _hal_reg_write(&module_profile->clk_switch, tar_pll);
		if (ret != 0) {
			pr_err("[npu dvfs] %s %d pll switch fail\n",
			       __func__, __LINE__);
			return -EINVAL;
		}
		ret = _hal_reg_write(&module_profile->clk_div, tar_div);
		if (ret != 0) {
			pr_err("[npu dvfs] %s %d div fail\n",
			       __func__, __LINE__);
			return -EINVAL;
		}
	} else {
		/*
		 * config div with
		 * switch pll
		 */
		ret = _hal_reg_write(&module_profile->clk_div, tar_div);
		if (ret != 0) {
			pr_err("[npu dvfs] %s %d div fail\n",
			       __func__, __LINE__);
			return -EINVAL;
		}
		ret = _hal_reg_write(&module_profile->clk_switch, tar_pll);
		if (ret != 0) {
			pr_err("[npu dvfs] %s %d pll switch fail\n",
			       __func__, __LINE__);
			return -EINVAL;
		}
	}

	/* disable source pll */
	clk_disable_unprepare(module_profile->profile_table[cur_pid].pll_clk);

	/* This module set tar profile done */
	module_profile->cur_pid = tar_pid;

	return 0;
}

static int hisi_npu_same_module_clk_conflict_handle(struct hisi_npu_module_profile *module_profile,
						    int tar_pid)
{
	int i;
	int ret;
	int max_pid;
	const char *clk_name = NULL;

	/* Make sure current voltage support the temporaty profile we choose */
	max_pid = module_profile->cur_pid > tar_pid ?
		  module_profile->cur_pid : tar_pid;

	for (i = 0; i < max_pid; i++) {
		/* Check profile table which use common clock */
		clk_name = module_profile->profile_table[i].clk_name;
		if (strcmp(clk_name, g_dvfs_data.ppll.clk_name) != 0)
			break;
	}
	if (i == max_pid) {
		pr_err("[npu dvfs] Module[%s]Clk conflict can't fix, cur%d tar%d\n",
		       module_profile->module_name,
		       module_profile->cur_pid,
		       tar_pid);
		return -EINVAL;
	}
	/*
	 * config Module to temporary pofile wich use common clock,
	 * and set to target profile after relock pll frequency
	 */
	ret = hisi_npu_set_module_profile(module_profile,
					  module_profile->cur_pid, i);
	if (ret != 0) {
		pr_err("[npu dvfs] Failed set [%d] -> [%d]\n",
		       module_profile->cur_pid, i);
		return ret;
	}

	return 0;
}

static int hisi_npu_dvfs_module_clk_cfg(int cur_pid, int tar_pid)
{
	int ret;
	int index_a;
	int index_b;
	struct hisi_npu_module_profile *module_a = NULL;
	struct hisi_npu_module_profile *module_b = NULL;

	for (index_a = 0; index_a < g_dvfs_data.module_count; index_a++) {
		module_a = &g_dvfs_data.module_profile[index_a];
		/* This module has set tar_profile done, check next */
		if (module_a->cur_pid == tar_pid)
			continue;

		/*
		 * module_a has not set profile to tar_profile,
		 * check clock conflict with other modules
		 */
		for (index_b = index_a; index_b < g_dvfs_data.module_count; index_b++) {
			module_b = &g_dvfs_data.module_profile[index_b];
			/* This module has set tar_profile done, check next */
			if (module_b->cur_pid  == tar_pid)
				continue;

			/*
			 * If module_a target clk and module_b current clk
			 * not conflict, check next module
			 */
			if (!hisi_npu_check_clk_freq_conflict(module_a,
							      tar_pid,
							      module_b,
							      cur_pid))
				continue;

			/* Check Same module clock conflict */
			if (index_a == index_b) {
				ret = hisi_npu_same_module_clk_conflict_handle(module_a,
									       tar_pid);
				if (ret != 0)
					return ret;
				continue;
			}

			/* clk conflict, first config Module b to tar_profile */
			ret = hisi_npu_set_module_profile(module_b,
							  module_b->cur_pid,
							  tar_pid);
			if (ret != 0) {
				pr_err("[npu dvfs] Failed set %s [%d] -> [%d]\n",
				       module_b->module_name, cur_pid, tar_pid);
				return ret;
			}
		}
		/* module_a tar_clock is free, check need config freq */
		hisi_npu_set_clk_freq(module_a, tar_pid);
		ret = hisi_npu_set_module_profile(module_a,
						  module_a->cur_pid,
						  tar_pid);
		if (ret != 0) {
			pr_err("[npu dvfs] Failed set %s [%d] -> [%d]\n",
			       module_a->module_name, cur_pid, tar_pid);
			return ret;
		}
	}
	/* All module have set profile done */
	return 0;
}

int npu_pm_dvfs_hal(unsigned long target_freq)
{
	unsigned int last_freq;
	int tar_pid;
	int cur_pid;
	int ret;
	int temp_stat;
	ktime_t in_ktime;
	unsigned long delta_time;

	if (!g_dvfs_data.dvfs_enable)
		return 0;

	in_ktime = ktime_get();

	temp_stat = hisi_npu_update_temp_stat();

	target_freq = target_freq / KHZ;

	if (temp_stat == TEMP_LOW_ENTER || temp_stat == TEMP_LOW)
		if (target_freq > g_dvfs_data.main_module->lt_freq_uplimit)
			target_freq = g_dvfs_data.main_module->lt_freq_uplimit;

	tar_pid = hisi_npu_find_profile_id(g_dvfs_data.main_module,
					   &target_freq);
	last_freq = g_dvfs_data.current_freq;
	g_dvfs_data.current_freq = target_freq;

	cur_pid = hisi_npu_find_absolute_profile_id(g_dvfs_data.main_module,
						    last_freq);
	if (cur_pid < 0) {
		pr_err("[npu dvfs] failed to find current profile id\n");
		return -EINVAL;
	}

	if (cur_pid == tar_pid) {
		ret = hisi_npu_voltage_update(cur_pid, tar_pid, temp_stat);
		if (ret != 0)
			pr_err("[npu dvfs] failed to update voltage\n");
		return ret;
	}
	pr_debug("[npu dvfs] %s cur_pid%d tar_pid%d(%lu)\n",
		 __func__, cur_pid,
		 tar_pid, target_freq);
	pr_debug("[npu dvfs] %s temp_stat %d\n", __func__, temp_stat);

	ret = hisi_npu_voltage_step_up(cur_pid, tar_pid, temp_stat);
	if (ret != 0) {
		pr_err("[npu dvfs] %s voltage step-up fail\n", __func__);
		return ret;
	}

	ret = hisi_npu_dvfs_module_clk_cfg(cur_pid, tar_pid);
	if (ret != 0) {
		pr_err("[npu dvfs] %s clk cfg fail\n", __func__);
		return ret;
	}

	ret = hisi_npu_voltage_step_down(cur_pid, tar_pid, temp_stat);
	if (ret != 0) {
		pr_err("[npu dvfs] %s voltage step-down fail\n", __func__);
		return ret;
	}

	delta_time = ktime_to_ns(ktime_sub(ktime_get(), in_ktime));
	if (delta_time > g_dvfs_data.max_dvfs_time)
		g_dvfs_data.max_dvfs_time = delta_time;

	return 0;
}

static int hisi_npu_module_buck_on(struct hisi_npu_module_profile *module_profile,
				   int tar_pid, int temp_stat)
{
	struct hisi_npu_profile *profile = NULL;
	struct regulator *buck = NULL;
	unsigned int tar_voltage;
	int ret;
#ifdef CONFIG_NPU_PM_AVS_VDM
	unsigned int pa_volt;
	unsigned int temp_idx;
#endif

	buck = module_profile->buck;
	if (IS_ERR_OR_NULL(buck))
		return 0;

	if (temp_stat == TEMP_LOW_ENTER || temp_stat == TEMP_LOW) {
		tar_voltage = module_profile->voltage_lt;
	} else {
		profile = module_profile->profile_table;
#ifdef CONFIG_NPU_PM_AVS_VDM
		if ((profile[tar_pid].func_sw & PROF_AVS) > 0) {
			pa_volt = npu_cluster_avs(g_dvfs_data.main_module, 0);
			temp_idx = update_avs_temperature_info();
			tar_voltage = npu_volt_cfg2vol(profile[tar_pid].avsvolt[temp_idx]) +
					  pa_volt;
			pr_debug("[npu pasnsor]  pa init is %u ,tar %u, origin %u\n",
				 pa_volt, tar_voltage,
				 profile[tar_pid].voltage);
		} else {
			tar_voltage = profile[tar_pid].voltage;
		}
#else
		tar_voltage = profile[tar_pid].voltage;
#endif
	}
	module_profile->last_voltage = tar_voltage;
	/* init voltage */
	ret = regulator_set_voltage(buck, tar_voltage, tar_voltage);
	if (ret != 0) {
		pr_err("[npu dvfs] %s %d set voltage failed%d\n",
			   __func__, __LINE__, ret);
		return ret;
	}

	ret = regulator_enable(buck);
	if (ret != 0) {
		pr_err("[npu dvfs] enable buck failed%d\n", ret);
		return ret;
	}

	return 0;
}

static int hisi_npu_voltage_init(struct hisi_npu_module_profile *module_profile,
				 int tar_pid, int temp_stat)
{
	return hisi_npu_module_buck_on(module_profile, tar_pid, temp_stat);
}

int hisi_npu_profile_hal_init(unsigned long target_freq)
{
	struct hisi_npu_module_profile *module_profile = NULL;
	struct clk *cur_pll_clk = NULL;
	struct clk *tar_pll_clk = NULL;
	unsigned int tar_div;
	unsigned int tar_pll;
	int tar_pid;
	int ret;
	int module;
	int temp_stat;

	temp_stat = hisi_npu_update_temp_stat();

	target_freq = target_freq / KHZ;

	if (temp_stat == TEMP_LOW_ENTER || temp_stat == TEMP_LOW)
		if (target_freq > g_dvfs_data.main_module->lt_freq_uplimit)
			target_freq = g_dvfs_data.main_module->lt_freq_uplimit;

	tar_pid = hisi_npu_find_profile_id(g_dvfs_data.main_module,
					   &target_freq);
	g_dvfs_data.current_freq = target_freq;

	for (module = 0; module < g_dvfs_data.module_count; module++) {
		module_profile = &g_dvfs_data.module_profile[module];
		tar_div = module_profile->profile_table[tar_pid].div;
		tar_pll = module_profile->profile_table[tar_pid].pll;

		/* find clk by cur_pll */
		cur_pll_clk = hisi_npu_get_cur_clk(module_profile);
		tar_pll_clk = module_profile->profile_table[tar_pid].pll_clk;
		if (IS_ERR_OR_NULL(cur_pll_clk) || IS_ERR_OR_NULL(tar_pll_clk))
			return -EINVAL;

		/* check and set ppll to tar freq if need */
		hisi_npu_set_clk_freq(module_profile, tar_pid);

		/*
		 * enable div gate
		 * set target div
		 * open current&target pll
		 * clk switch
		 * disable last pll
		 */
		ret = _hal_reg_write(&module_profile->clk_div_gate, 1);
		if (ret != 0) {
			pr_err("[npu dvfs] enable div gate failed%d\n", ret);
			goto err_hand3;
		}
		ret = _hal_reg_write(&module_profile->clk_div, tar_div);
		if (ret != 0) {
			pr_err("[npu dvfs] config div failed%d\n", ret);
			goto err_hand3;
		}
		ret = clk_prepare_enable(cur_pll_clk);
		if (ret != 0) {
			pr_err("[npu dvfs] enable src clk failed%d\n", ret);
			goto err_hand3;
		}
		ret = clk_prepare_enable(tar_pll_clk);
		if (ret != 0) {
			pr_err("[npu dvfs] enable target clk failed%d\n", ret);
			goto err_hand2;
		}
		ret = _hal_reg_write(&module_profile->clk_switch, tar_pll);
		if (ret != 0) {
			pr_err("[npu dvfs] %s %d clk switch failed%d\n",
			       __func__, __LINE__, ret);
			goto err_hand1;
		}
		clk_disable_unprepare(cur_pll_clk);

		module_profile->cur_clk = tar_pll_clk;
		module_profile->cur_pid = tar_pid;

		/* set target voltage & open buck */
		ret = hisi_npu_voltage_init(module_profile, tar_pid, temp_stat);
		if (ret != 0) {
			pr_err("[npu dvfs] enable buck failed%d\n", ret);
			goto err_hand1;
		}
	}
	return 0;

err_hand1:
	clk_disable_unprepare(tar_pll_clk);
err_hand2:
	clk_disable_unprepare(cur_pll_clk);
err_hand3:
	return ret;
}

void hisi_npu_profile_hal_exit(void)
{
	int module;
	struct regulator *buck = NULL;
	struct clk *clk = NULL;
	int ret;

	for (module = 0; module < g_dvfs_data.module_count; module++) {
		/* disable div gate */
		ret = _hal_reg_write(&g_dvfs_data.module_profile[module].clk_div_gate, 0);
		if (ret != 0)
			pr_err("[npu dvfs] disable div gate failed%d\n", ret);

		/* buck disable */
		buck = g_dvfs_data.module_profile[module].buck;
		if (!IS_ERR_OR_NULL(buck)) {
			ret = regulator_disable(buck);
			if (ret != 0)
				pr_err("[npu dvfs] disable buck failed%d\n",
				       ret);
		}

		/* clk disable */
		clk = g_dvfs_data.module_profile[module].cur_clk;
		if (IS_ERR_OR_NULL(clk) == false)
			clk_disable_unprepare(clk);
	}


	hisi_npu_clr_temp_stat();
}

int hisi_npu_dt_buck_data_parse(struct device *dev,
				struct device_node *module_np,
				struct hisi_npu_module_profile *module_profile)
{
	struct regulator *buck = NULL;
	const char *buck_name = NULL;
	unsigned int time = 0;
	int ret;

	ret = of_property_read_string(module_np, "buck_name", &buck_name);
	if (ret != 0) {
		pr_debug("module [%s] has no buck\n",
			 module_profile->module_name);
	} else {
		buck = devm_regulator_get(dev, buck_name);
		if (IS_ERR_OR_NULL(buck)) {
			dev_err(dev, "[%s] buck[%s] not exist\n",
				__func__, buck_name);
			return -EINVAL;
		}
		module_profile->buck = buck;

		ret = of_property_read_u32(module_np, "buck_step_time", &time);
		if (ret != 0) {
			dev_err(dev, "[%s]parse buck_step_time failed%d\n",
				__func__, ret);
			return ret;
		}
		module_profile->buck_step_time = time;

		ret = of_property_read_u32(module_np, "buck_hold_time", &time);
		if (ret != 0) {
			dev_err(dev, "[%s]parse buck_hold_time failed%d\n",
				__func__, ret);
			return ret;
		}
		module_profile->buck_hold_time = time;
	}

	return 0;
}

static int hisi_npu_dt_low_temp_data_parse(struct device *dev,
					   struct device_node *module_np,
					   struct hisi_npu_module_profile *module_profile)
{
	unsigned int frequency = 0;
	unsigned int voltage = 0;
	int ret;

	ret = of_property_read_u32(module_np, "low_temp_frequency", &frequency);
	if (ret != 0) {
		dev_err(dev, "[%s]parse low_temp_frequency failed%d\n",
			__func__, ret);
		return ret;
	}
	module_profile->lt_freq_uplimit = frequency;

	ret = of_property_read_u32(module_np, "low_temp_voltage", &voltage);
	if (ret != 0) {
		dev_err(dev, "[%s]parse low_temp_voltage failed%d\n",
			__func__, ret);
		return ret;
	}
	module_profile->voltage_lt = voltage;

	return 0;
}

static int hisi_npu_dt_reg_data_parse(struct device *dev,
				      struct device_node *module_np,
				      struct hisi_npu_module_profile *module_profile)
{
	void __iomem *base = NULL;
	struct clk_div_regs_dt clk_div_data = { 0 };
	struct clk_gate_regs_dt clk_gate_data = { 0 };
	int ret;

	base = module_profile->base;

	ret = of_property_read_u32_array(module_np, "clock_switch_reg",
					 (u32 *)(&clk_div_data),
					 CLK_DIV_ELEM_NUM);
	if (ret != 0) {
		dev_err(dev, "[%s] get clock switch from dt failed(%d)\n",
			__func__, ret);
		return -EINVAL;
	}
	dt2clkdiv(&clk_div_data, &module_profile->clk_switch, base);

	ret = of_property_read_u32_array(module_np, "clock_div_reg",
					 (u32 *)(&clk_div_data),
					 CLK_DIV_ELEM_NUM);
	if (ret != 0) {
		dev_err(dev, "[%s] get clock divider from dt failed(%d)\n",
			__func__, ret);
		return -EINVAL;
	}
	dt2clkdiv(&clk_div_data, &module_profile->clk_div, base);

	ret = of_property_read_u32_array(module_np, "clock_div_gate_reg",
					 (u32 *)(&clk_div_data),
					 CLK_DIV_ELEM_NUM);
	if (ret != 0) {
		dev_err(dev, "[%s] get clock div gate from dt failed(%d)\n",
			__func__, ret);
		return -EINVAL;
	}
	dt2clkdiv(&clk_div_data, &module_profile->clk_div_gate, base);

	ret = of_property_read_u32_array(module_np, "clock_gate_reg",
					 (u32 *)(&clk_gate_data),
					 CLK_GATE_ELEM_NUM);
	if (ret != 0) {
		dev_err(dev, "[%s] get clock gate from dt failed(%d)\n",
			__func__, ret);
		return -EINVAL;
	}
	dt2clkgate(&clk_gate_data, &module_profile->clk_gate, base);

	return 0;
}

int hisi_npu_clock_init(struct device *dev)
{
	struct device_node *np = dev->of_node;
	const char *clock_name = NULL;
	struct clk *clock = NULL;
	int clock_count;
	struct npn_clk_list *list_node = NULL;
	struct npn_clk_list *tmp = NULL;
	int index;
	int ret;

	INIT_LIST_HEAD(&g_npn_clk_list.list);

	clock_count = of_property_count_strings(np, "clock-names");
	if (clock_count <= 0) {
		dev_err(dev, "clock names is null\n");
		return -EINVAL;
	}

	for (index = 0; index < clock_count; index++) {
		ret = of_property_read_string_index(np, "clock-names",
						    index, &clock_name);
		if (ret != 0) {
			dev_err(dev, "failed to find clock name(index=%d, ret=%d)\n",
				index, ret);
			goto err_handle;
		}
		clock = devm_clk_get(dev, clock_name);
		if (IS_ERR_OR_NULL(clock)) {
			ret = PTR_ERR(clock);
			goto err_handle;
		}

		list_node = devm_kzalloc(dev, sizeof(struct npn_clk_list),
					 GFP_KERNEL);
		if (list_node == NULL) {
			dev_err(dev, "Failed to allocate clock node\n");
			ret = -ENOMEM;
			goto err_handle;
		}
		list_node->clock_name = clock_name;
		list_node->clock      = clock;
		list_add_tail(&list_node->list, &g_npn_clk_list.list);
	}

	return 0;
err_handle:
	list_for_each_entry_safe(list_node, tmp, &g_npn_clk_list.list, list)
		list_del(&list_node->list);

	return ret;
}

struct clk *hisi_npu_find_clock(const char *clock_name)
{
	struct npn_clk_list *list_node = NULL;

	if (IS_ERR_OR_NULL(clock_name)) {
		pr_err("Clock name is NULL\n");
		return NULL;
	}

	list_for_each_entry(list_node, &g_npn_clk_list.list, list)
		if (strcmp(clock_name, list_node->clock_name) == 0)
			return list_node->clock;

	pr_err("Failed to get clk res for %s\n", clock_name);

	return NULL;
}

static int hisi_npu_dt_module_parse(struct device *dev,
				    struct device_node *module_np,
				    struct hisi_npu_module_profile *module_profile)
{
	struct device_node *child = NULL;
	struct device_node *parent = NULL;
	struct hisi_npu_profile *profile_table = NULL;
	struct hisi_npu_profile *profile = NULL;
	int profile_count;
	const char *clk_name = NULL;
	int ret;
	int i;

	parent = of_get_child_by_name(module_np, "profile");
	if (parent == NULL) {
		dev_err(dev, "[%s] Failed to find profile node\n", __func__);
		return -EINVAL;
	}

	profile_count = of_get_child_count(parent);
	if (profile_count == 0) {
		dev_err(dev, "[%s] no any profile\n", __func__);
		return -EINVAL;
	}

	profile_table = devm_kzalloc(dev,
				     sizeof(struct hisi_npu_profile) *
				     profile_count,
				     GFP_KERNEL);
	if (profile_table == NULL) {
		dev_err(dev, "Failed to allocate npu profile_table\n");
		return -ENOMEM;
	}

	profile = profile_table;
	for_each_child_of_node(parent, child) {
		ret = of_property_read_u32(child, "frequency",
					   (u32 *)(&profile->freq));
		if (ret != 0) {
			dev_err(dev, "[%s]parse %s frequency failed%d!\n",
				__func__, child->name, ret);
			goto err_out;
		}

		ret = of_property_read_u32(child, "voltage",
					   (u32 *)(&profile->voltage));
		if (ret != 0) {
			dev_err(dev, "[%s]parse %s voltage failed%d!\n",
				__func__, child->name, ret);
			goto err_out;
		}

		ret = of_property_read_string(child, "clock-names", &clk_name);
		if (ret != 0) {
			dev_err(dev, "[%s]parse %s clock failed%d!\n",
				__func__, child->name, ret);
			goto err_out;
		}

		profile->clk_name = clk_name;
		profile->pll_clk = hisi_npu_find_clock(clk_name);
		if (IS_ERR_OR_NULL(profile->pll_clk)) {
			dev_err(dev, "[%s] Failed to get clock: %s\n",
				__func__, clk_name);
			goto err_out;
		}

		ret = of_property_read_u32(child, "clock_sw",
					   (u32 *)(&profile->pll));
		if (ret != 0) {
			dev_err(dev, "[%s]parse %s clock_sw failed%d!\n",
				__func__, child->name, ret);
			goto err_out;
		}

		ret = of_property_read_u32(child, "clock_div",
					   (u32 *)(&profile->div));
		if (ret != 0) {
			dev_err(dev, "[%s]parse %s clock_div failed%d!\n",
				__func__, child->name, ret);
			goto err_out;
		}

		profile++;
	}

	module_profile->profile_table = profile_table;
	module_profile->profile_count = profile_count;

	return 0;

err_out:
	profile = profile_table;
	for (i = 0; i < profile_count; i++) {
		if (!IS_ERR_OR_NULL(profile->pll_clk))
			devm_clk_put(dev, profile->pll_clk);

		profile++;
	}

	return ret;
}

static int hisi_npu_dt_profile_parse(struct device *dev,
				     struct device_node *module_np,
				     struct hisi_npu_module_profile *module_profile)
{
	struct module_clk_list *list_node = NULL;
	struct module_clk_list *tmp = NULL;
	int clock_count;
	int index;
	const char *clk_name = NULL;
	unsigned int clk_switch;
	int ret;

	INIT_LIST_HEAD(&module_profile->clk_list.list);

	clock_count = of_property_count_strings(module_np, "clock-names");
	if (clock_count <= 0) {
		dev_err(dev, "clock names is null\n");
		return -EINVAL;
	}
	if (clock_count != of_property_count_u32_elems(module_np,
						       "clock_switch")) {
		dev_err(dev, "clock sw count not match clock-names in %s\n",
			module_np->name);
		return -EINVAL;
	}

	for (index = 0; index < clock_count; index++) {
		ret = of_property_read_string_index(module_np, "clock-names",
						    index, &clk_name);
		if (ret != 0) {
			dev_err(dev, "fail to find clk name(index%d, ret%d)\n",
				index, ret);
			return ret;
		}

		ret = of_property_read_u32_index(module_np, "clock_switch",
						 index, &clk_switch);
		if (ret != 0) {
			dev_err(dev, "fail to find clk_sw(index%d, ret%d)\n",
				index, ret);
			return ret;
		}

		list_node = devm_kzalloc(dev, sizeof(struct module_clk_list),
					 GFP_KERNEL);
		if (list_node == NULL) {
			dev_err(dev, "Fail to allocate module_clk_list node\n");
			return -ENOMEM;
		}
		list_node->name   = clk_name;
		list_node->clk    = hisi_npu_find_clock(clk_name);
		list_node->clk_sw = clk_switch;
		list_add_tail(&list_node->list, &module_profile->clk_list.list);
	}

	ret = hisi_npu_dt_module_parse(dev, module_np, module_profile);
	if (ret != 0)
		list_for_each_entry_safe(list_node, tmp,
					 &module_profile->clk_list.list, list)
			list_del(&list_node->list);

	return ret;
}

void hisi_npu_dt_ppll_parse(struct device_node *root)
{
	int ret;

	g_dvfs_data.ppll.base = of_iomap(root, 2);
	if (g_dvfs_data.ppll.base == NULL) {
		pr_debug("[%s] Not support ppll config\n", __func__);
		return;
	}
	ret = of_property_read_string(root, "conflict-clock-name",
				      &g_dvfs_data.ppll.clk_name);
	if (ret != 0) {
		pr_debug("[%s] Parse conflict-clock-name dt failed, maybe not support %d\n",
			 __func__, ret);
		goto error_out;
	}

	ret = of_property_read_u32_array(root, "clock-conf-reg",
					 (u32 *)(g_dvfs_data.ppll.ctrl_offset),
					 sizeof(g_dvfs_data.ppll.ctrl_offset) /
					 sizeof(u32));
	if (ret != 0) {
		pr_err("[%s] Parse clock-conf-reg dt failed%d\n",
		       __func__, ret);
		goto error_out;
	}
	ret = of_property_read_u32_array(root, "clock-conf-para",
					 (u32 *)(g_dvfs_data.ppll.para),
					 sizeof(g_dvfs_data.ppll.para) /
					 sizeof(u32));
	if (ret != 0) {
		pr_err("[%s] Parse clock-conf-para dt failed%d\n",
		       __func__, ret);
		goto error_out;
	}
	g_dvfs_data.ppll.cur_freq = g_dvfs_data.ppll.para[0][0]; /* KHz */
	return;

error_out:
	iounmap(g_dvfs_data.ppll.base);
	g_dvfs_data.ppll.base = NULL;
	g_dvfs_data.ppll.clk_name = NULL;
}

int hisi_npu_profile_data_init(struct device *dev)
{
	struct device_node *root = dev->of_node;
	struct device_node *module_np = root;
	struct hisi_npu_module_profile *module_profile = NULL;
	const char *main_module_name = NULL;
	int module_count = 0;
	int ret = 0;
	int i;
	void __iomem *base = NULL;

	base = of_iomap(root, 0);
	if (base == NULL) {
		dev_err(dev, "[%s] base iomap fail %ld\n",
			__func__, PTR_ERR(base));
		return -ENOMEM;
	}

	for_each_node_by_type(module_np, "npu-dvfs-module")
		module_count++;

	g_dvfs_data.module_profile = devm_kzalloc(dev,
						  sizeof(struct hisi_npu_module_profile) *
						  module_count,
						  GFP_KERNEL);
	if (g_dvfs_data.module_profile == NULL) {
		iounmap(base);
		dev_err(dev, "[%s] kzalloc module profile(%d) failed %ld\n",
			__func__, module_count,
			PTR_ERR(g_dvfs_data.module_profile));
		return -ENOMEM;
	}

	g_dvfs_data.module_count = module_count;

	module_profile = g_dvfs_data.module_profile;

	ret = of_property_read_string(root, "main-dvfs-module",
				      &main_module_name);
	if (ret != 0) {
		iounmap(base);
		dev_err(dev, "[%s]parse main module failed%d\n", __func__, ret);
		return ret;
	}

	/* Parse device tree for npu subsys */
	module_np = root;
	for_each_node_by_type(module_np, "npu-dvfs-module") {
		module_profile->module_name = module_np->name;
		module_profile->base = base;

		ret = hisi_npu_dt_buck_data_parse(dev, module_np,
						  module_profile);
		if (ret != 0) {
			dev_err(dev, "%s dts buck parse failed\n", __func__);
			goto out;
		}

		ret = hisi_npu_dt_low_temp_data_parse(dev, module_np,
						      module_profile);
		if (ret != 0) {
			dev_err(dev, "%s dts low temperature parse failed\n",
				__func__);
			goto out;
		}

		ret = hisi_npu_dt_reg_data_parse(dev, module_np,
						 module_profile);
		if (ret != 0) {
			dev_err(dev, "%s dts regs parse failed\n",
				__func__);
			goto out;
		}

		ret = hisi_npu_dt_profile_parse(dev, module_np,
						module_profile);
		if (ret != 0) {
			dev_err(dev, "%s dts profile data parse failed\n",
				__func__);
			goto out;
		}

		/* avs info init must behind profile data init */
		ret = hisi_npu_dt_avs_info_parse(dev, module_np,
						 module_profile,
						 main_module_name);
		if (ret != 0) {
			dev_err(dev, "%s dts profile avs data parse failed\n",
				__func__);
			goto out;
		}

		if (strncmp(main_module_name,
			    module_profile->module_name,
			    strlen(main_module_name)) == 0)
			g_dvfs_data.main_module = module_profile;

		module_profile++;
	}

	/* Parse NPU private ppll para if need config freq dynamically */
	hisi_npu_dt_ppll_parse(root);

	if (g_dvfs_data.main_module == NULL)
		g_dvfs_data.main_module = &g_dvfs_data.module_profile[0];

	g_dvfs_data.current_freq =
		g_dvfs_data.main_module->profile_table[0].freq;

	return 0;
out:
	iounmap(base);
	module_profile = g_dvfs_data.module_profile;
	for (i = 0; i < module_count; i++) {
		module_profile->base = NULL;
		module_profile++;
	}

	return ret;
}

static int hisi_npu_thermal_init(struct device *dev)
{
#ifdef CONFIG_THERMAL_TSENSOR
	g_dvfs_data.tsensor = ipa_get_tsensor_id(HISI_NPU_TSENSOR_NAME);
	if (g_dvfs_data.tsensor < 0) {
		dev_err(dev, "get tsensor fail %d\n", g_dvfs_data.tsensor);
		return -EINVAL;
	}
#endif

	return 0;
}

int npu_pm_dvfs_init(struct npu_pm_device *pmdev)
{
	struct device *dev = pmdev->dev;
	int ret;

	g_dvfs_data.current_freq = pmdev->last_freq / KHZ;

	ret = hisi_npu_thermal_init(dev);
	if (ret != 0)
		dev_err(dev, "%s get tsensor id failed\n", __func__);

	ret = hisi_npu_clock_init(dev);
	if (ret != 0) {
		dev_err(dev, "%s npu clk node parse failed\n", __func__);
		return ret;
	}

	ret = hisi_npu_profile_data_init(dev);
	if (ret != 0) {
		dev_err(dev, "%s npu profile data parse failed\n", __func__);
		return ret;
	}


	pmdev->dvfs_data = &g_dvfs_data;

	return ret;
}
