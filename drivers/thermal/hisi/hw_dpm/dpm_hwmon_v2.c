
/*
 * dpm_hwmon_v2.c
 *
 * dpm interface for component and user
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#include "dpm_hwmon_v2.h"
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <soc_acpu_baseaddr_interface.h>
#include <soc_crgperiph_interface.h>
#include <soc_pcr_interface.h>
#include <soc_pctrl_interface.h>

#define PARAM_BYTE_NUM		4
#define POWER_NUM		1
#define DPMONITOR_MAX_PARAM_NUM		33
/* enable sensor 0~15 */
#define DPMMONITOR_SENSOR_EN_VAL	0xffff
#define BIT_MASK_OFFSET		16
#define PCTRL_SHUTDOWN_TPRAM		0x854

#define PERI_CRG_SIZE		0xFFF
#define PERI_PCR_SIZE		0xFFF
#define PCTRL_SIZE		0xFFF

static DEFINE_MUTEX(g_dpm_hwmon_ops_lock);
static DEFINE_MUTEX(g_dpm_hwmon_peri_lock);
static void __iomem *g_peri_crg_base;
static void __iomem *g_peri_pcr_base;
static void __iomem *g_pctrl_base;

LIST_HEAD(g_dpm_hwmon_ops_list);
DEFINE_MUTEX(g_dpm_hwmon_ops_list_lock);
unsigned int g_dpm_peri_pcr_vote;

/* common module start */
static void free_dpm_ops_mem(struct dpm_hwmon_ops *dpm_ops)
{
	if (dpm_ops->dpm_counter_table != NULL) {
		kfree(dpm_ops->dpm_counter_table);
		dpm_ops->dpm_counter_table = NULL;
	}
	if (dpm_ops->dpm_power_table != NULL) {
		kfree(dpm_ops->dpm_power_table);
		dpm_ops->dpm_power_table = NULL;
	}
}

int dpm_hwmon_register(struct dpm_hwmon_ops *dpm_ops)
{
	struct dpm_hwmon_ops *pos = NULL;
	int ret;
	const int qword_size = sizeof(unsigned long long);

	if (dpm_ops == NULL || dpm_ops->dpm_module_id < 0 ||
	    dpm_ops->dpm_module_id >= DPM_MODULE_NUM) {
		pr_err("%s LINE %d dpm_ops is invalid\n", __func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&g_dpm_hwmon_ops_list_lock);
	list_for_each_entry(pos, &g_dpm_hwmon_ops_list, ops_list) {
		if (dpm_ops->dpm_module_id == pos->dpm_module_id) {
			pr_err("%s LINE %d dpm module %d has registered\n",
			       __func__, __LINE__, dpm_ops->dpm_module_id);
			mutex_unlock(&g_dpm_hwmon_ops_list_lock);
			return -EINVAL;
		}
	}
	list_add(&dpm_ops->ops_list, &g_dpm_hwmon_ops_list);
	mutex_unlock(&g_dpm_hwmon_ops_list_lock);

	if ((dpm_ops->dpm_cnt_len > 0 && dpm_ops->dpm_cnt_len < DPM_BUFFER_SIZE) &&
	    (dpm_ops->dpm_power_len > 0 && dpm_ops->dpm_power_len < DPM_BUFFER_SIZE)) {
		dpm_ops->dpm_counter_table = kzalloc(qword_size * dpm_ops->dpm_cnt_len,
						     GFP_KERNEL);
		dpm_ops->dpm_power_table = kzalloc(qword_size * dpm_ops->dpm_power_len,
						   GFP_KERNEL);
		if (dpm_ops->dpm_counter_table == NULL ||
		    dpm_ops->dpm_power_table == NULL) {
			ret = -ENOMEM;
			goto err_handler;
		}
	} else {
		return -EINVAL;
	}
	dpm_ops->module_enabled = false;
	pr_err("dpm hwmon %d register!\n", dpm_ops->dpm_module_id);
	return 0;

err_handler:
	ret = dpm_hwmon_unregister(dpm_ops);
	if (ret < 0)
		pr_err("%s LINE %d register fail", __func__, __LINE__);
	return ret;
}

int dpm_hwmon_unregister(struct dpm_hwmon_ops *dpm_ops)
{
	struct dpm_hwmon_ops *pos = NULL;
	struct dpm_hwmon_ops *tmp = NULL;

	if (dpm_ops == NULL) {
		pr_err("%s LINE %d dpm_ops is NULL\n", __func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&g_dpm_hwmon_ops_list_lock);
	list_for_each_entry_safe(pos, tmp, &g_dpm_hwmon_ops_list, ops_list) {
		if (dpm_ops->dpm_module_id == pos->dpm_module_id) {
			free_dpm_ops_mem(pos);
			list_del_init(&pos->ops_list);
			break;
		}
	}
	mutex_unlock(&g_dpm_hwmon_ops_list_lock);
	return 0;
}
/* common module end */

static void dpm_data_process(void __iomem *dpm_addr, unsigned long long *data,
			     int mode)
{
	unsigned long long data_mem;

	data_mem = (unsigned long long)readl(dpm_addr);
	if (mode > 0)
		*data = *data + data_mem;
	else
		*data = data_mem;
}

void dpm_monitor_enable(void __iomem *module_dpm_addr, unsigned int sig_level_mode,
			unsigned int *param, int param_num)
{
	int i;

	if (param == NULL || param_num <= 0 ||
	    param_num > DPMONITOR_MAX_PARAM_NUM) {
		pr_err("%s invalid param\n", __func__);
		return;
	}
	/* step 6: sync cg off */
	writel(BIT(SOC_DPMONITOR_CTRL_EN_sync_cg_off_START) << BIT_MASK_OFFSET,
	       SOC_DPMONITOR_CTRL_EN_ADDR(module_dpm_addr));
	/* step 7 8: fill parameter */
	for (i = 0; i < param_num; i++)
		writel(param[i], SOC_DPMONITOR_POWER_PARAM0_ADDR(module_dpm_addr) +
		       i * PARAM_BYTE_NUM);
	/* step 9 sig level mode */
	writel(sig_level_mode, SOC_DPMONITOR_SIGNAL_LEVEL_MODE_ADDR(module_dpm_addr));
	/* step 12 enable monitor_senor 0~15 */
	writel(DPMMONITOR_SENSOR_EN_VAL, SOC_DPMONITOR_SENSOR_EN_ADDR(module_dpm_addr));
	/* step 13 enable monitor */
	writel(BIT(SOC_DPMONITOR_CTRL_EN_monitor_ctrl_en_START) |
	       (BIT(SOC_DPMONITOR_CTRL_EN_monitor_ctrl_en_START) << BIT_MASK_OFFSET),
	       SOC_DPMONITOR_CTRL_EN_ADDR(module_dpm_addr));
}

void dpm_monitor_disable(void __iomem *module_dpm_addr)
{
	/* step 1: disable tsensor */
	writel(BIT(SOC_DPMONITOR_CTRL_EN_monitor_ctrl_en_START) << BIT_MASK_OFFSET,
	       SOC_DPMONITOR_CTRL_EN_ADDR(module_dpm_addr));
}

int dpm_hwmon_update_power(void __iomem *module_dpm_addr,
			   unsigned long long *power, int mode)
{
	if (power == NULL) {
		pr_err("%s invalid param\n", __func__);
		return -EINVAL;
	}
	dpm_data_process(SOC_DPMONITOR_ACC_ENERGY_ADDR(module_dpm_addr), power, mode);
	return POWER_NUM;
}

int dpm_hwmon_update_counter_power(void __iomem *module_dpm_addr,
				   unsigned long long *counter_table,
				   int counter_len,
				   unsigned long long *power, int mode)
{
	int i;

	if (counter_table == NULL || power == NULL) {
		pr_err("%s invalid param\n", __func__);
		return -EINVAL;
	}
	for (i = 0; i < counter_len; i++)
		dpm_data_process(SOC_DPMONITOR_BUSY_CNT0_ADDR(module_dpm_addr) +
				 i * PARAM_BYTE_NUM, counter_table + i, mode);
	dpm_data_process(SOC_DPMONITOR_ACC_ENERGY_ADDR(module_dpm_addr), power,
			 mode);
	return counter_len + POWER_NUM;
}

unsigned long long get_dpm_power(struct dpm_hwmon_ops *pos)
{
	unsigned int i;
	unsigned long long dpm_power = 0;

	if (pos == NULL)
		return dpm_power;

	for (i = 0; i < pos->dpm_power_len; i++)
		dpm_power += pos->dpm_power_table[i];

	return dpm_power;
}

void dpm_enable_peri_pcr(void)
{
	if (g_peri_crg_base == NULL || g_peri_pcr_base == NULL)
		return;
	/* close peri pcr clk */
	writel(BIT(SOC_CRGPERIPH_PERDIS7_gt_clk_peri_pcr_START),
	       SOC_CRGPERIPH_PERDIS7_ADDR(g_peri_crg_base));
	/* disable peri pcr rst and prst */
	writel(BIT(SOC_CRGPERIPH_PERRSTDIS2_ip_rst_peripcr_START) |
	       BIT(SOC_CRGPERIPH_PERRSTDIS2_ip_prst_peripcr_START),
	       SOC_CRGPERIPH_PERRSTDIS2_ADDR(g_peri_crg_base));
	/* open peri pcr clk and pclk */
	writel(BIT(SOC_CRGPERIPH_PEREN7_gt_clk_peri_pcr_START) |
	       BIT(SOC_CRGPERIPH_PEREN7_gt_pclk_peri_pcr_START),
	       SOC_CRGPERIPH_PEREN7_ADDR(g_peri_crg_base));
	/* set pcr output: bypass, don't control frequency */
	writel(BIT(SOC_PCR_CFG_BYPASS_ff_en_bypass_START),
	       SOC_PCR_CFG_BYPASS_ADDR(g_peri_pcr_base));
	/* enable pcr */
	writel(BIT(SOC_PCR_CTRL_enable_START), SOC_PCR_CTRL_ADDR(g_peri_pcr_base));
	pr_err("open peri pcr!\n");
}

static void dpm_disable_peri_pcr(void)
{
	if (g_peri_crg_base == NULL || g_peri_pcr_base == NULL ||
	    g_pctrl_base == NULL)
		return;

	/* set pcr output: bypass, don't control frequency */
	writel(BIT(SOC_PCR_CFG_BYPASS_ff_en_bypass_START),
	       SOC_PCR_CFG_BYPASS_ADDR(g_peri_pcr_base));
	/* disable pcr */
	writel(0, SOC_PCR_CTRL_ADDR(g_peri_pcr_base));
	writel(PCTRL_SHUTDOWN_TPRAM, SOC_PCTRL_PERI_CTRL68_ADDR(g_pctrl_base));
	/* enable peri pcr rst and prst */
	writel(BIT(SOC_CRGPERIPH_PERRSTEN2_ip_rst_peripcr_START) |
	       BIT(SOC_CRGPERIPH_PERRSTEN2_ip_prst_peripcr_START),
	       SOC_CRGPERIPH_PERRSTEN2_ADDR(g_peri_crg_base));
	/* close peri pcr clk */
	writel(BIT(SOC_CRGPERIPH_PERDIS7_gt_clk_peri_pcr_START),
	       SOC_CRGPERIPH_PERDIS7_ADDR(g_peri_crg_base));
	pr_err("close peri pcr!\n");
}

void peri_pcr_vote(bool vote, unsigned int module_mask)
{
	mutex_lock(&g_dpm_hwmon_peri_lock);
	if (((g_dpm_peri_pcr_vote & module_mask) > 0) != vote) {
		if (vote) {
			/* peri pcr is closed and vote */
			if (g_dpm_peri_pcr_vote == 0)
				dpm_enable_peri_pcr();
			g_dpm_peri_pcr_vote |= module_mask;
		} else {
			g_dpm_peri_pcr_vote &= (~module_mask);
			/* both peri vote and gpu vote is disabled */
			if (g_dpm_peri_pcr_vote == 0)
				dpm_disable_peri_pcr();
		}
	}
	mutex_unlock(&g_dpm_hwmon_peri_lock);
}

void dpm_enable_module(struct dpm_hwmon_ops *pos, bool dpm_switch)
{
	if (pos != NULL)
		pos->module_enabled = dpm_switch;
}

bool get_dpm_enabled(struct dpm_hwmon_ops *pos)
{
	if (pos != NULL)
		return pos->module_enabled;
	else
		return false;
}

void dpm_sample(struct dpm_hwmon_ops *pos)
{
	if (pos == NULL)
		return;
	if (pos->hi_dpm_update_power() < 0)
		pr_err("DPM %d sample fail\n", pos->dpm_module_id);
}

void dpm_iounmap(void)
{
	if (g_peri_crg_base != NULL) {
		iounmap(g_peri_crg_base);
		g_peri_crg_base = NULL;
	}
	if (g_peri_pcr_base != NULL) {
		iounmap(g_peri_pcr_base);
		g_peri_pcr_base = NULL;
	}
	if (g_pctrl_base != NULL) {
		iounmap(g_pctrl_base);
		g_pctrl_base = NULL;
	}
}

int dpm_ioremap(void)
{
	g_peri_crg_base = ioremap(SOC_ACPU_PERI_CRG_BASE_ADDR, PERI_CRG_SIZE);
	g_peri_pcr_base = ioremap(SOC_ACPU_PERI_PCR_BASE_ADDR, PERI_PCR_SIZE);
	g_pctrl_base = ioremap(SOC_ACPU_PCTRL_BASE_ADDR, PCTRL_SIZE);

	/* peri dpm not work, gpu dpm is affected, npu dpm is ok */
	if (g_peri_crg_base == NULL || g_peri_pcr_base == NULL ||
	    g_pctrl_base == NULL) {
		dpm_iounmap();
		pr_err("dpm ioremap fail!\n");
		return -EFAULT;
	}
	return 0;
}
