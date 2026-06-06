/*
 *  Copyright (C) 2017 Hisilicon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
#include <hisi_npu_pm.h>
#include "hisi_npu_pm_private.h"

#include "peri_volt_poll.h"

struct npn_clk_list {
	struct list_head list; /* kernel's list structure */
	const char *clock_name;
	struct clk *clock;
};
static struct npn_clk_list g_npn_clk_list;

struct hisi_npu_dvfs_data g_dvfs_data = {
	.module_profile = NULL,
	.module_count = 0,
	.main_module  = NULL,
	.dvfs_enable  = false,
	.temp_stat    = TEMP_NORMAL,
	.test         = 0,
	.pvp          = NULL,
};

#define CONFIG_USE_SOFT_CURR_FREQ	1

#define HISI_NPU_TSENSOR_NAME		("gpu")
#define LOW_TEMP_THRESHOLD	(5)/*5 degree*/

#define NPU_PERI_VOL_VOTE_ID	62

extern int ipa_get_tsensor_id(const char *name);
extern int ipa_get_sensor_value(u32 sensor, int *val);

bool hisi_npu_is_low_temperature(void)
{
	int temp = 0;
	int ret;


	if (g_dvfs_data.tsensor >= 0) {
		ret = ipa_get_sensor_value(g_dvfs_data.tsensor, &temp);
		if ((0 == ret) && (temp < LOW_TEMP_THRESHOLD)) {
			return true;
		}
	}

	return false;
}

int _hal_reg_write(struct clk_div_regs *cfg, unsigned int val)
{
	int shift, width;
	u32 mask;

	shift = ffs(cfg->rd_mask);
	if (shift <= 0) {
		return -EPERM;
	}
	shift -= 1;
	width = fls(cfg->rd_mask) - shift;
	mask  = (1 << (unsigned int)width) - 1;

	if (val > mask) {
		val = mask;
	}
	writel(cfg->wr_mask | ((val & mask) << (unsigned int)shift), cfg->cfg_reg);

	udelay(1);

	return 0;
}

int _hal_reg_read(struct clk_div_regs *cfg, u32 *val)
{
	int shift;

	shift = ffs(cfg->rd_mask);
	if (shift <= 0) {
		return -EPERM;
	}
	shift -= 1;
	*val = ((unsigned int)readl(cfg->stat_reg) & cfg->rd_mask) >> (unsigned int)shift;

	return 0;
}

int hisi_npu_get_module_voltage(struct hisi_npu_module_profile *module_profile)
{
	struct peri_volt_poll *pvp = NULL;
	int voltage_uV = 0;
	unsigned int vote_id;

	if (strcmp(module_profile->module_name, g_dvfs_data.main_module->module_name) == 0) {
		pvp = peri_volt_poll_get(NPU_PERI_VOL_VOTE_ID, NULL);
		if (IS_ERR_OR_NULL(pvp)) {
			pr_err("[npu dvfs]%s get pvp failed!\n", __func__);
			return -EINVAL;
		}
		vote_id = peri_get_volt(pvp);
		if (vote_id < (unsigned int)module_profile->profile_count) {
			voltage_uV = module_profile->profile_table[vote_id].voltage;
		} else {
			pr_err("[npu dvfs]%s Failed to read peri voltage, vote_id=%d\n", __func__, vote_id);
		}

		return voltage_uV;
	}
	return 0;
}

int hisi_npu_set_module_voltage(struct hisi_npu_module_profile *module_profile, unsigned int voltage)
{
	int ret;
	struct regulator *buck;

	buck = module_profile->buck;
	if (IS_ERR_OR_NULL(buck)) {
		return 0;
	}

	ret = regulator_set_voltage(buck, voltage, voltage);
	if (ret) {
		return ret;
	}
	udelay(500);

	return 0;
}

void dt2clkdiv(struct clk_div_regs_dt *clk_div_dt, struct clk_div_regs *clk_div_regs, void __iomem *base)
{
	clk_div_regs->cfg_reg  = clk_div_dt->cfg_offset + base;
	clk_div_regs->stat_reg = clk_div_dt->stat_offset + base;
	clk_div_regs->rd_mask  = clk_div_dt->rd_mask;
	clk_div_regs->wr_mask  = clk_div_dt->wr_mask;
}

void dt2clkgate(struct clk_gate_regs_dt *clk_gate_dt, struct clk_gate_regs *clk_gate_regs, void __iomem *base)
{
	clk_gate_regs->en_reg   = clk_gate_dt->en_offset + base;
	clk_gate_regs->dis_reg  = clk_gate_dt->dis_offset + base;
	clk_gate_regs->stat_reg = clk_gate_dt->stat_offset + base;
	clk_gate_regs->mask     = clk_gate_dt->mask;
}

void _hal_clk_gate_enable(struct clk_gate_regs *cfg)
{
	writel(cfg->mask, cfg->en_reg);
}

void _hal_clk_gate_disable(struct clk_gate_regs *cfg)
{
	writel(cfg->mask, cfg->dis_reg);
}

int hisi_npu_find_profile_id(struct hisi_npu_module_profile *module_profile, unsigned long *freq)
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

static struct clk *
hisi_npu_get_cur_clk(struct hisi_npu_module_profile *module_profile)
{
	struct module_clk_list *clk_node;
	u32 clk_switch;
	int ret;

	ret = _hal_reg_read(&(module_profile->clk_switch), &clk_switch);
	if (ret) {
		pr_err("[npu dvfs]%s get clk switch failed\n", __func__);
		return NULL;
	}

	list_for_each_entry(clk_node, &(module_profile->clk_list.list), list) {
		if (clk_node->clk_sw == clk_switch) {
			return clk_node->clk;
		}
	}

	return NULL;
}

unsigned long
hisi_npu_get_cur_clk_rate(struct hisi_npu_module_profile *module_profile)
{
	u32 clk_div;
	struct clk *cur_clk;
	int ret;

	cur_clk = hisi_npu_get_cur_clk(module_profile);
	if (IS_ERR_OR_NULL(cur_clk)) {
		pr_err("[npu dvfs]%s get pll failed\n", __func__);
		return 0;
	}

	ret = _hal_reg_read(&(module_profile->clk_div), &clk_div);
	if (ret) {
		pr_err("[npu dvfs]%s get clk div failed\n", __func__);
		return 0;
	}

	return (clk_get_rate(cur_clk) / (clk_div + 1));
}

int hisi_npu_find_absolute_profile_id(struct hisi_npu_module_profile *module_profile, unsigned int freq)
{
	int id;

	for (id = 0; id < module_profile->profile_count; id++) {
		if (freq == module_profile->profile_table[id].freq) {
			return id;
		}
	}

	pr_err("[npu dvfs] freq%u not exist in profile(%s)\n", freq, module_profile->module_name);
	return -EINVAL;
}

int hisi_npu_find_cur_profile_id_by_hw(struct hisi_npu_module_profile *module_profile)
{
	return 0;
}

int hisi_npu_update_temp_stat(void)
{
	bool is_low_temp;
	int stat = TEMP_ERROR;

	is_low_temp = hisi_npu_is_low_temperature();
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

void hisi_npu_clr_temp_stat(void)
{
	g_dvfs_data.temp_stat = TEMP_NORMAL;
}

static int hisi_npu_vote_peri_voltage(int tar_pid)
{
	int ret = 0;
	int vote_timeout = 5000;

	if (IS_ERR_OR_NULL(g_dvfs_data.pvp)) {
		pr_err("[npu dvfs]%s global pvp is NULL!\n", __func__);
		return -EINVAL;
	}

	ret = peri_set_volt(g_dvfs_data.pvp, tar_pid);
	if (ret) {
		pr_err("[npu dvfs]%s Vote tar_pid=%d failed, ret = %d\n", __func__, tar_pid, ret);
		return -EINVAL;
	}

	while(vote_timeout) {
		/* case 1. vote voltage up: Wait here untill the voltage readout >= the vote voltage
		 * case 2. vote voltage down: Other IP may vote a high voltage, only vote and no need to wait
		 * */
		if ((unsigned int)tar_pid <= peri_get_volt(g_dvfs_data.pvp)) {
			/* Success vote the voltage */
			break;
		} else {
			vote_timeout--;
		}
		udelay(10);
	}

	if (vote_timeout == 0) {
		/*Failed to vote the voltage*/
		pr_err("[npu dvfs]%s Vote tar_pid=%d timeout\n", __func__, tar_pid);
		return -EINVAL;
	}

	ret = peri_wait_avs_update(g_dvfs_data.pvp);
	if (ret) {
		pr_err("[npu dvfs]%s wait avs status update failed, ret = %d\n", __func__, ret);
	}

	return ret;
}

int hisi_npu_voltage_step_up(int cur_pid, int tar_pid, int temp_stat)
{
	int ret = 0;
	if (cur_pid < tar_pid) {
		ret = hisi_npu_vote_peri_voltage(tar_pid);
	}
	return ret;
}

int hisi_npu_voltage_step_down(int cur_pid, int tar_pid, int temp_stat)
{
	int ret = 0;
	if (cur_pid > tar_pid) {
		ret = hisi_npu_vote_peri_voltage(tar_pid);
	}
	return ret;
}

int hisi_npu_voltage_update(int cur_pid, int tar_pid, int temp_stat)
{
	struct hisi_npu_module_profile *module_profile;
	struct regulator *buck;
	unsigned int tar_voltage;
	unsigned int last_voltage;
	unsigned int buck_step_time;
	int module;
	int ret = 0;

	for (module = 0; module < g_dvfs_data.module_count; module++) {
		module_profile = &(g_dvfs_data.module_profile[module]);

		npu_update_svfd_code(module_profile, tar_pid);

		buck = module_profile->buck;
		if (IS_ERR_OR_NULL(buck)) {
			continue;
		}

		if (TEMP_LOW_ENTER == temp_stat) {
			tar_voltage = module_profile->voltage_lt;
		} else if (TEMP_LOW_EXIT == temp_stat) {
			tar_voltage = module_profile->profile_table[tar_pid].voltage;
		} else {
			continue;
		}
		last_voltage = module_profile->last_voltage;
		buck_step_time = module_profile->buck_step_time;

		module_profile->last_voltage = tar_voltage;
		ret = regulator_set_voltage(buck, tar_voltage, tar_voltage);
		if (ret) {
			return ret;
		}
		if (last_voltage > tar_voltage) {
			udelay((last_voltage - tar_voltage) / buck_step_time + module_profile->buck_hold_time);
		} else {
			udelay((tar_voltage - last_voltage) / buck_step_time + module_profile->buck_hold_time);
		}
	}

	return ret;
}

int hisi_npu_dvfs_module_clk_cfg(int cur_pid, int tar_pid)
{
	struct hisi_npu_module_profile *module_profile;
	unsigned int cur_div;
	unsigned int tar_div;
	unsigned int tar_pll;
	int module;
	int ret;

	for (module = 0; module < g_dvfs_data.module_count; module++) {
		module_profile = &(g_dvfs_data.module_profile[module]);

		cur_div = module_profile->profile_table[cur_pid].div;
		tar_div = module_profile->profile_table[tar_pid].div;
		tar_pll = module_profile->profile_table[tar_pid].pll;
		/* enable target pll */
		ret = clk_prepare_enable(module_profile->profile_table[tar_pid].pll_clk);
		if (ret) {
			pr_err("[npu dvfs] %s %d clk enable fail%d\n", __func__, __LINE__, ret);
			return ret;
		}
		module_profile->cur_clk = module_profile->profile_table[tar_pid].pll_clk;

		if (tar_div <= cur_div) {
			/*
			 * switch pll
			 * config div with
			 */
			ret = _hal_reg_write(&(module_profile->clk_switch), tar_pll);
			if (ret) {
				pr_err("[npu dvfs] %s %d pll switch fail\n", __func__, __LINE__);
				return -EINVAL;
			}
			ret = _hal_reg_write(&(module_profile->clk_div), tar_div);
			if (ret) {
				pr_err("[npu dvfs] %s %d div fail\n", __func__, __LINE__);
				return -EINVAL;
			}
		} else {
			/*
			 * config div with
			 * switch pll
			 */
			ret = _hal_reg_write(&(module_profile->clk_div), tar_div);
			if (ret) {
				pr_err("[npu dvfs] %s %d div fail\n", __func__, __LINE__);
				return -EINVAL;
			}
			ret = _hal_reg_write(&(module_profile->clk_switch), tar_pll);
			if (ret) {
				pr_err("[npu dvfs] %s %d pll switch fail\n", __func__, __LINE__);
				return -EINVAL;
			}
		}

		/*disable source pll*/
		clk_disable_unprepare(module_profile->profile_table[cur_pid].pll_clk);
	}

	return 0;
}

int hisi_npu_dvfs_hal(unsigned long target_freq)
{
	unsigned int last_freq;
	int tar_pid;
	int cur_pid;
	int ret;
	int temp_stat;


	if (!g_dvfs_data.dvfs_enable) {
		return 0;
	}

	temp_stat = hisi_npu_update_temp_stat();

	target_freq = target_freq / 1000;

	if ((TEMP_LOW_ENTER == temp_stat) || (TEMP_LOW == temp_stat)) {
		if (target_freq > g_dvfs_data.main_module->lt_freq_uplimit) {
			target_freq = g_dvfs_data.main_module->lt_freq_uplimit;
		}
	}

	tar_pid = hisi_npu_find_profile_id(g_dvfs_data.main_module, &target_freq);
	last_freq = g_dvfs_data.current_freq;
	g_dvfs_data.current_freq = target_freq;

	cur_pid = hisi_npu_find_absolute_profile_id(g_dvfs_data.main_module, last_freq);
	if (cur_pid < 0) {
		pr_err("[npu dvfs] failed to find current profile id\n");
		return -EINVAL;
	}

	if (cur_pid == tar_pid) {
		ret = hisi_npu_voltage_update(cur_pid, tar_pid, temp_stat);
		if (ret) {
			pr_err("[npu dvfs] failed to update voltage\n");
		}
		return ret;
	}
	pr_debug("[npu dvfs] %s cur_pid%d tar_pid%d(%lu)\n", __func__, cur_pid, tar_pid, target_freq);
	pr_debug("[npu dvfs] %s temp_stat %d\n", __func__, temp_stat);

	hisi_npu_svfd_switch(g_dvfs_data.main_module, tar_pid, 0);

	ret = hisi_npu_voltage_step_up(cur_pid, tar_pid, temp_stat);
	if (ret) {
		pr_err("[npu dvfs] %s voltage step-up fail\n", __func__);
		return ret;
	}

	ret = hisi_npu_dvfs_module_clk_cfg(cur_pid, tar_pid);
	if (ret) {
		pr_err("[npu dvfs] %s clk cfg fail\n", __func__);
		return ret;
	}

	ret = hisi_npu_voltage_step_down(cur_pid, tar_pid, temp_stat);
	if (ret) {
		pr_err("[npu dvfs] %s voltage step-down fail\n", __func__);
		return ret;
	}

	hisi_npu_svfd_switch(g_dvfs_data.main_module, tar_pid, 1);

	return 0;
}

int hisi_npu_voltage_init(struct hisi_npu_module_profile *module_profile, int tar_pid, int temp_stat)
{
	int ret = 0;

	/* Only set the voltage for main module */
	if (!strncmp(module_profile->module_name, g_dvfs_data.main_module->module_name, strlen(module_profile->module_name))) {
		ret = peri_set_avs(g_dvfs_data.pvp, PERI_AVS_ENABLE);
		if (ret) {
			pr_err("[npu dvfs] %s set avs failed, ret = %d\n", __func__, ret);
			return ret;
		}
		ret = hisi_npu_vote_peri_voltage(tar_pid);
	}

	return ret;
}

int hisi_npu_profile_hal_init(unsigned long target_freq)
{
	struct hisi_npu_module_profile *module_profile;
	struct clk *cur_pll_clk;
	struct clk *tar_pll_clk;
	unsigned int tar_div;
	unsigned int tar_pll;
	int tar_pid;
	int ret;
	int module;
	int temp_stat;

	temp_stat = hisi_npu_update_temp_stat();

	target_freq = target_freq / 1000;

	if ((TEMP_LOW_ENTER == temp_stat) || (TEMP_LOW == temp_stat)) {
		if (target_freq > g_dvfs_data.main_module->lt_freq_uplimit) {
			target_freq = g_dvfs_data.main_module->lt_freq_uplimit;
		}
	}

	tar_pid = hisi_npu_find_profile_id(g_dvfs_data.main_module, &target_freq);
	g_dvfs_data.current_freq = target_freq;

	for (module = 0; module < g_dvfs_data.module_count; module++) {
		module_profile = &(g_dvfs_data.module_profile[module]);
		tar_div = module_profile->profile_table[tar_pid].div;
		tar_pll = module_profile->profile_table[tar_pid].pll;

		/* find clk by cur_pll */
		cur_pll_clk = hisi_npu_get_cur_clk(module_profile);
		tar_pll_clk = module_profile->profile_table[tar_pid].pll_clk;
		if (IS_ERR_OR_NULL(cur_pll_clk) || IS_ERR_OR_NULL(tar_pll_clk)) {
			return -EINVAL;
		}

		/*
		 * enable div gate
		 * set target div
		 * open current&target pll
		 * clk switch
		 * disable last pll
		 */
		ret = _hal_reg_write(&(module_profile->clk_div_gate), 1);
		if (ret) {
			pr_err("[npu dvfs] enable div gate failed%d\n", ret);
			goto err_hand3;
		}
		ret = _hal_reg_write(&(module_profile->clk_div), tar_div);
		if (ret) {
			pr_err("[npu dvfs] config div failed%d\n", ret);
			goto err_hand3;
		}
		ret = clk_prepare_enable(cur_pll_clk);
		if (ret) {
			pr_err("[npu dvfs] enable src clk failed%d\n", ret);
			goto err_hand3;
		}
		ret = clk_prepare_enable(tar_pll_clk);
		if (ret) {
			pr_err("[npu dvfs] enable target clk failed%d\n", ret);
			goto err_hand2;
		}
		ret = _hal_reg_write(&(module_profile->clk_switch), tar_pll);
		if (ret) {
			pr_err("[npu dvfs] %s %d clk switch failed%d\n", __func__, __LINE__, ret);
			goto err_hand1;
		}
		clk_disable_unprepare(cur_pll_clk);

		module_profile->cur_clk = tar_pll_clk;

		/* set target voltage & open buck */
		ret = hisi_npu_voltage_init(module_profile, tar_pid, temp_stat);
		if (ret) {
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
	struct regulator *buck;
	struct clk *clk;
	int ret;

	for (module = 0; module < g_dvfs_data.module_count; module++) {
		/* disable div gate */
		ret = _hal_reg_write(&(g_dvfs_data.module_profile[module].clk_div_gate), 0);
		if (ret) {
			pr_err("[npu dvfs] disable div gate failed%d\n", ret);
		}

		/* buck disable */
		buck = g_dvfs_data.module_profile[module].buck;
		if (!IS_ERR_OR_NULL(buck)) {
			ret = regulator_disable(buck);
			if (ret) {
				pr_err("[npu dvfs] disable buck failed%d\n", ret);
			}
		}

		/* clk disable */
		clk = g_dvfs_data.module_profile[module].cur_clk;
		if (!IS_ERR_OR_NULL(clk)) {
			clk_disable_unprepare(clk);
		}
	}

	ret = hisi_npu_vote_peri_voltage(0);
	if (ret) {
		pr_err("[npu dvfs] disable buck failed%d\n", ret);
	}
	ret = peri_set_avs(g_dvfs_data.pvp, PERI_AVS_DISABLE);
	if (ret) {
		pr_err("[npu dvfs] disable avs failed, ret=%d\n", ret);
	}

	hisi_npu_clr_temp_stat();
}

int hisi_npu_dt_buck_data_parse(struct device *dev,
                                struct device_node *module_np,
                                struct hisi_npu_module_profile *module_profile)
{
	struct regulator *buck;
	const char *buck_name = NULL;
	unsigned int time = 0;
	int ret;

	ret = of_property_read_string(module_np, "buck_name", &buck_name);
	if (ret) {
		pr_debug("module [%s] has no buck\n", module_profile->module_name);
	} else {
		buck = devm_regulator_get(dev, buck_name);
		if (IS_ERR_OR_NULL(buck)) {
			dev_err(dev, "[%s] buck[%s] not exist\n", __func__, buck_name);
			return -EINVAL;
		}
		module_profile->buck = buck;

		ret = of_property_read_u32(module_np, "buck_step_time", &time);
		if (ret) {
			dev_err(dev, "[%s]parse buck_step_time failed%d\n", __func__, ret);
			return ret;
		}
		module_profile->buck_step_time = time;

		ret = of_property_read_u32(module_np, "buck_hold_time", &time);
		if (ret) {
			dev_err(dev, "[%s]parse buck_hold_time failed%d\n", __func__, ret);
			return ret;
		}
		module_profile->buck_hold_time = time;
	}

	return 0;
}

int hisi_npu_dt_low_temp_data_parse(struct device *dev,
                                struct device_node *module_np,
                                struct hisi_npu_module_profile *module_profile)
{
	unsigned int frequency = 0;
	unsigned int voltage = 0;
	int ret;

	ret = of_property_read_u32(module_np, "low_temp_frequency", &frequency);
	if (ret) {
		dev_err(dev, "[%s]parse low_temp_frequency failed%d\n", __func__, ret);
		return ret;
	}
	module_profile->lt_freq_uplimit = frequency;

	ret = of_property_read_u32(module_np, "low_temp_voltage", &voltage);
	if (ret) {
		dev_err(dev, "[%s]parse low_temp_voltage failed%d\n", __func__, ret);
		return ret;
	}
	module_profile->voltage_lt = voltage;

	return 0;
}

int hisi_npu_dt_reg_data_parse(struct device *dev,
                               struct device_node *module_np,
                               struct hisi_npu_module_profile *module_profile)
{
	void __iomem *base;
	struct clk_div_regs_dt clk_div_data = { 0 };
	struct clk_gate_regs_dt clk_gate_data = { 0 };
	int ret;

	base = module_profile->base;

	ret = of_property_read_u32_array(module_np, "clock_switch_reg", (u32 *)(&clk_div_data), CLK_DIV_ELEM_NUM);
	if (ret) {
		dev_err(dev, "[%s] get clock switch from dt failed(%d)\n", __func__, ret);
		return -EINVAL;
	}
	dt2clkdiv(&clk_div_data, &(module_profile->clk_switch), base);

	ret = of_property_read_u32_array(module_np, "clock_div_reg", (u32 *)(&clk_div_data), CLK_DIV_ELEM_NUM);
	if (ret) {
		dev_err(dev, "[%s] get clock divider from dt failed(%d)\n", __func__, ret);
		return -EINVAL;
	}
	dt2clkdiv(&clk_div_data, &(module_profile->clk_div), base);

	ret = of_property_read_u32_array(module_np, "clock_div_gate_reg", (u32 *)(&clk_div_data), CLK_DIV_ELEM_NUM);
	if (ret) {
		dev_err(dev, "[%s] get clock div gate from dt failed(%d)\n", __func__, ret);
		return -EINVAL;
	}
	dt2clkdiv(&clk_div_data, &(module_profile->clk_div_gate), base);

	ret = of_property_read_u32_array(module_np, "clock_gate_reg", (u32 *)(&clk_gate_data), CLK_GATE_ELEM_NUM);
	if (ret) {
		dev_err(dev, "[%s] get clock gate from dt failed(%d)\n", __func__, ret);
		return -EINVAL;
	}
	dt2clkgate(&clk_gate_data, &(module_profile->clk_gate), base);

	return 0;
}

void hisi_npu_clk_list_release(struct device *dev)
{
	struct npn_clk_list *tmp, *node;

	list_for_each_entry_safe(node, tmp, &(g_npn_clk_list.list), list) {
		list_del(&node->list);
	}
}

/*lint -e429*/
int hisi_npu_clock_init(struct device *dev)
{
	struct device_node *np = dev->of_node;
	const char *clock_name = NULL;
	struct clk *clock;
	int clock_count;
	struct npn_clk_list *list_node, *tmp;
	int index;
	int ret;

	INIT_LIST_HEAD(&(g_npn_clk_list.list));

	clock_count = of_property_count_strings(np, "clock-names");
	if (clock_count <= 0) {
		dev_err(dev, "clock names is null\n");
		return -EINVAL;
	}

	for (index = 0; index < clock_count; index++) {
		ret = of_property_read_string_index(np, "clock-names", index, &clock_name);
		if (ret) {
			dev_err(dev, "failed to find clock name(index=%d, ret=%d)\n", index, ret);
			goto err_handle;
		}
		clock = devm_clk_get(dev, clock_name);
		if (IS_ERR(clock)) {
			ret = PTR_ERR(clock);
			goto err_handle;
		}

		list_node = devm_kzalloc(dev, sizeof(struct npn_clk_list), GFP_KERNEL);
		if (!list_node) {
			dev_err(dev, "Failed to allocate clock node\n");
			ret = -ENOMEM;
			goto err_handle;
		}
		list_node->clock_name = clock_name;
		list_node->clock      = clock;
		list_add_tail(&(list_node->list), &(g_npn_clk_list.list));
	}

	return 0;
err_handle:
	list_for_each_entry_safe(list_node, tmp, &(g_npn_clk_list.list), list) {
		list_del(&list_node->list);
	}

	return ret;
}
/*lint +e429*/

struct clk *hisi_npu_find_clock(const char *clock_name)
{
	struct npn_clk_list *list_node;

	if (IS_ERR_OR_NULL(clock_name)) {
		pr_err("Clock name is NULL\n");
		return NULL;
	}

	list_for_each_entry(list_node, &(g_npn_clk_list.list), list) {
		if (!strcmp(clock_name, list_node->clock_name)) {
			return list_node->clock;
		}
	}

	pr_err("Failed to get clk res for %s\n", clock_name);
	return NULL;
}

static int hisi_npu_dt_module_parse(struct device *dev,
                                    struct device_node *module_np,
                                    struct hisi_npu_module_profile *module_profile)
{
	struct device_node *child;
	struct device_node *parent;
	struct hisi_npu_profile *profile_table;
	struct hisi_npu_profile *profile;
	int profile_count;
	const char *clk_name = NULL;
	int ret;
	int i;

	parent = of_get_child_by_name(module_np, "profile");
	if (!parent) {
		dev_err(dev, "[%s] Failed to find profile node\n", __func__);
		return -EINVAL;
	}

	profile_count = of_get_child_count(parent);
	if (!profile_count) {
		dev_err(dev, "[%s] no any profile\n", __func__);
		return -EINVAL;
	}

	profile_table = devm_kzalloc(dev, sizeof(struct hisi_npu_profile) * profile_count, GFP_KERNEL);
	if (!profile_table) {
		dev_err(dev, "Failed to allocate npu profile_table\n");
		return -ENOMEM;
	}

	profile = profile_table;
	for_each_child_of_node(parent, child) {
		ret = of_property_read_u32(child, "frequency", (u32 *)(&(profile->freq)));
		if (ret) {
			dev_err(dev, "[%s]parse %s frequency failed%d!\n", __func__, child->name, ret);
			goto err_out;
		}

		ret = of_property_read_u32(child, "voltage", (u32 *)(&(profile->voltage)));
		if (ret) {
			dev_err(dev, "[%s]parse %s voltage failed%d!\n", __func__, child->name, ret);
			goto err_out;
		}

		ret = of_property_read_string(child, "clock-names", &clk_name);
		if (ret) {
			dev_err(dev, "[%s]parse %s clock failed%d!\n", __func__, child->name, ret);
			goto err_out;
		}

		profile->pll_clk = hisi_npu_find_clock(clk_name);
		if (IS_ERR_OR_NULL(profile->pll_clk)) {
			dev_err(dev, "[%s] Failed to get clock: %s\n", __func__, clk_name);
			goto err_out;
		}

		ret = of_property_read_u32(child, "clock_sw", (u32 *)(&(profile->pll)));
		if (ret) {
			dev_err(dev, "[%s]parse %s clock_sw failed%d!\n", __func__, child->name, ret);
			goto err_out;
		}

		ret = of_property_read_u32(child, "clock_div", (u32 *)(&(profile->div)));
		if (ret) {
			dev_err(dev, "[%s]parse %s clock_div failed%d!\n", __func__, child->name, ret);
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
		if (!IS_ERR_OR_NULL(profile->pll_clk)) {
			devm_clk_put(dev, profile->pll_clk);
		}
		profile++;
	}

	return ret;
}

/*lint -e429*/
int hisi_npu_dt_profile_parse(struct device *dev,
                              struct device_node *module_np,
                              struct hisi_npu_module_profile *module_profile)
{
	struct module_clk_list *list_node, *tmp;//lint !e429
	int clock_count;
	int index;
	const char *clk_name = NULL;
	unsigned int clk_switch;
	int ret;

	INIT_LIST_HEAD(&(module_profile->clk_list.list));

	clock_count = of_property_count_strings(module_np, "clock-names");
	if (clock_count <= 0) {
		dev_err(dev, "clock names is null\n");
		return -EINVAL;
	}
	if (clock_count != of_property_count_u32_elems(module_np, "clock_switch")) {
		dev_err(dev, "clock sw count not match clock-names in %s\n", module_np->name);
		return -EINVAL;
	}

	for (index = 0; index < clock_count; index++) {
		ret = of_property_read_string_index(module_np, "clock-names", index, &clk_name);
		if (ret) {
			dev_err(dev, "failed to find clock name(index=%d, ret=%d)\n", index, ret);
			return ret;
		}

		ret = of_property_read_u32_index(module_np, "clock_switch", index, &clk_switch);
		if (ret) {
			dev_err(dev, "failed to find clock switch(index=%d, ret=%d)\n", index, ret);
			return ret;
		}

		list_node = devm_kzalloc(dev, sizeof(struct module_clk_list), GFP_KERNEL);
		if (!list_node) {
			dev_err(dev, "Failed to allocate module_clk_list node\n");
			return -ENOMEM;
		}
		list_node->name   = clk_name;
		list_node->clk    = hisi_npu_find_clock(clk_name);
		list_node->clk_sw = clk_switch;
		list_add_tail(&(list_node->list), &(module_profile->clk_list.list));
	}

	ret = hisi_npu_dt_module_parse(dev, module_np, module_profile);
	if (ret) {
		list_for_each_entry_safe(list_node, tmp, &(module_profile->clk_list.list), list) {
			list_del(&list_node->list);
		}
	}

	return ret;
}
/*lint +e429*/

int hisi_npu_profile_data_init(struct device *dev)
{
	struct device_node *root = dev->of_node;
	struct device_node *module_np = root;
	struct hisi_npu_module_profile *module_profile;
	const char *main_module_name = NULL;
	int module_count = 0;
	int ret = 0;
	void __iomem *base;

	base = of_iomap(root, 0);
	if (!base) {
		dev_err(dev, "[%s] base iomap fail %ld\n", __func__, PTR_ERR(base));
		return -ENOMEM;
	}

	for_each_node_by_type(module_np, "npu-dvfs-module") {
		module_count++;
	}

	g_dvfs_data.module_profile = devm_kzalloc(dev, sizeof(struct hisi_npu_module_profile) * module_count, GFP_KERNEL);
	if (!(g_dvfs_data.module_profile)) {
		dev_err(dev, "[%s] kzalloc module profile(%d) failed %ld\n", __func__, module_count, PTR_ERR(g_dvfs_data.module_profile));
		return -ENOMEM;
	}

	g_dvfs_data.module_count = module_count;

	module_profile = g_dvfs_data.module_profile;

	ret = of_property_read_string(root, "main-dvfs-module", &main_module_name);
	if (ret) {
		dev_err(dev, "[%s]parse main module failed%d\n", __func__, ret);
		goto out;
	}

	/*
	 *  Parse device tree for npu subsys
	 */
	module_np = root;
	for_each_node_by_type(module_np, "npu-dvfs-module") {
		module_profile->module_name = module_np->name;
		module_profile->base = base;

		ret = hisi_npu_dt_buck_data_parse(dev, module_np, module_profile);
		if (ret) {
			dev_err(dev, "%s dts buck parse failed\n", __func__);
			goto out;
		}

		ret = hisi_npu_dt_low_temp_data_parse(dev, module_np, module_profile);
		if (ret) {
			dev_err(dev, "%s dts low temperature parse failed\n", __func__);
			goto out;
		}

		ret = hisi_npu_dt_reg_data_parse(dev, module_np, module_profile);
		if (ret) {
			dev_err(dev, "%s dts regs parse failed\n", __func__);
			goto out;
		}

		ret = hisi_npu_dt_profile_parse(dev, module_np, module_profile);
		if (ret) {
			dev_err(dev, "%s dts profile data parse failed\n", __func__);
			goto out;
		}

		/* avs info init must behind profile data init */
		ret = hisi_npu_dt_avs_info_parse(dev, module_np, module_profile, main_module_name);
		if (ret) {
			dev_err(dev, "%s dts profile avs data parse failed\n", __func__);
			goto out;
		}

		if (!strncmp(main_module_name, module_profile->module_name, strlen(main_module_name))) {
			g_dvfs_data.main_module = module_profile;
		}

		module_profile++;
	}

	if (!g_dvfs_data.main_module) {
		g_dvfs_data.main_module = &g_dvfs_data.module_profile[0];
	}

	g_dvfs_data.current_freq = g_dvfs_data.main_module->profile_table[0].freq;

out:

	return ret;
}

int hisi_npu_profile_release(struct npu_pm_device *pmdev)
{
	return 0;
}

int hisi_npu_thermal_init(struct device *dev)
{
	g_dvfs_data.tsensor = ipa_get_tsensor_id(HISI_NPU_TSENSOR_NAME);
	if (g_dvfs_data.tsensor < 0) {
		dev_err(dev, "get tsensor fail(%d)\n", g_dvfs_data.tsensor);
		return -EINVAL;
	}

	return 0;
}

int hisi_npu_dvfs_init(struct npu_pm_device *pmdev)
{
	struct device *dev = pmdev->dev;
	int ret;

	g_dvfs_data.current_freq = pmdev->last_freq / KHz;

	ret = hisi_npu_thermal_init(dev);
	if (ret) {
		dev_err(dev, "%s get tsensor id failed\n", __func__);
	}

	ret = hisi_npu_clock_init(dev);
	if (ret) {
		dev_err(dev, "%s npu clk node parse failed\n", __func__);
		return ret;
	}

	ret = hisi_npu_profile_data_init(dev);
	if (ret) {
		dev_err(dev, "%s npu profile data parse failed\n", __func__);
		return ret;
	}

	g_dvfs_data.pvp = peri_volt_poll_get(NPU_PERI_VOL_VOTE_ID, NULL);
	if (IS_ERR_OR_NULL(g_dvfs_data.pvp)) {
		dev_err(dev, "[npu dvfs]%s get pvp failed!\n", __func__);
		return -EINVAL;
	}

	pmdev->dvfs_data = &g_dvfs_data;

	return ret;
}

void hisi_npu_dvfs_exit(struct npu_pm_device *pmdev)
{
	hisi_npu_profile_release(pmdev);
}
