/*
 * hisi-kirin-fast-dvfs.c
 *
 * Hisilicon clock driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "hisi-kirin-fast-dvfs.h"
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <securec.h>

#ifndef CLK_FAST_DVFS_MDEBUG_LEVEL
#define CLK_FAST_DVFS_MDEBUG_LEVEL 1
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CLK"

#if CLK_FAST_DVFS_MDEBUG_LEVEL
#define clk_log(fmt, ...) \
	pr_err("[%s]%s: " fmt "\n", LOG_TAG, __func__, ##__VA_ARGS__)
#else
#define clk_log(fmt, ...)
#endif

static int get_rate_level(struct clk_hw *hw, unsigned long rate)
{
	struct hi3xxx_fastclk *fclk = NULL;
	int level;

	fclk = container_of(hw, struct hi3xxx_fastclk, hw);
	for (level = PROFILE_CNT - 1; level >= 0; level--) {
		if (rate >= fclk->p_value[level] * FREQ_CONVERSION)
			return level;
	}
	level = 0;

	return level;
}

/* return the closest rate and parent clk/rate */
static int fast_dvfs_clk_determine_rate(struct clk_hw *hw,
	struct clk_rate_request *req)
{
	struct hi3xxx_fastclk *fclk = NULL;
	int level;
	int ret = 0;

	fclk = container_of(hw, struct hi3xxx_fastclk, hw);
	level = get_rate_level(hw, req->rate);

	/* if req rate smaller than all array value, set to profile[0] */
	req->rate = fclk->p_value[level] * FREQ_CONVERSION;

	return ret;
}

/* return cur freq, used in clk_recalc by clk_change_rate */
static unsigned long fast_dvfs_clk_recalc_rate(struct clk_hw *hw,
	unsigned long parent_rate)
{
	struct hi3xxx_fastclk *fclk = NULL;
	void __iomem *div_addr = NULL;
	void __iomem *sw_addr = NULL;
	unsigned int div_value, sw_value, val;
	int i;

	fclk = container_of(hw, struct hi3xxx_fastclk, hw);

	div_addr = fclk->base_addr + fclk->clkdiv_offset[CFG_OFFSET];
	div_value = (unsigned int)readl(div_addr);
	div_value = (div_value & fclk->clkdiv_offset[CFG_MASK]) >> fclk->clkdiv_offset[SHIFT];
	div_value++;

	sw_addr = fclk->base_addr + fclk->clksw_offset[CFG_OFFSET];
	sw_value = (unsigned int)readl(sw_addr);
	sw_value = (sw_value & fclk->clksw_offset[CFG_MASK]) >> fclk->clksw_offset[SHIFT];

	val = sw_value >> 1;
	for (i = 0; val != 0; i++)
		val >>= 1;
	if (i >= PLL_CNT) {
		pr_err("[%s]%s sw value is illegal,  sw_value = 0x%x, i = %d!\n",
			__func__, hw->core->name, sw_value, i);
		return 0;
	}

	/* div_value will not be 0 */
	fclk->rate = (((unsigned long)fclk->pll_profile[i]) * FREQ_CONVERSION) / div_value;

	return fclk->rate;
}

static int set_rate_before(struct hi3xxx_fastclk *fclk,
	int cur_level, int tar_level)
{
	struct clk *tar_pll = NULL;
	struct clk *cur_pll = NULL;
	const char *tar_pll_name = NULL;
	const char *cur_pll_name = NULL;
	int ret;

	tar_pll_name = pll_name[fclk->pll_name_id[tar_level]];
	cur_pll_name = pll_name[fclk->pll_name_id[cur_level]];

	tar_pll = __clk_lookup(tar_pll_name);
	if (IS_ERR_OR_NULL(tar_pll)) {
		pr_err("[%s] %s get failed!\n", __func__, tar_pll_name);
		return -ENODEV;
	}

	cur_pll = __clk_lookup(cur_pll_name);
	if (IS_ERR_OR_NULL(cur_pll)) {
		pr_err("[%s] %s get failed!\n", __func__, cur_pll_name);
		return -ENODEV;
	}

	ret = clk_core_prepare(tar_pll->core);
	if (ret) {
		pr_err("[%s] %s clk_core_prepare fail!\n", __func__, tar_pll_name);
		return -ENODEV;
	}
	ret = clk_enable(tar_pll);
	if (ret) {
		pr_err("[%s] %s clk enable fail!\n", __func__, tar_pll_name);
		goto err_tar_en;
	}

	ret = clk_core_prepare(cur_pll->core);
	if (ret) {
		pr_err("[%s] %s clk_core_prepare fail!\n", __func__, tar_pll_name);
		goto err_cur_pre;
	}
	ret = clk_enable(cur_pll);
	if (ret) {
		pr_err("[%s] %s clk enable fail!\n", __func__, cur_pll_name);
		goto err_cur_en;
	}

	return ret;
err_cur_en:
	clk_core_unprepare(cur_pll->core);
err_cur_pre:
	clk_disable(tar_pll);
err_tar_en:
	clk_core_unprepare(tar_pll->core);
	return -ENODEV;
}

static int set_rate_after(struct hi3xxx_fastclk *fclk,
	int cur_level, int tar_level)
{
	struct clk *tar_pll = NULL;
	struct clk *cur_pll = NULL;
	const char *tar_pll_name = NULL;
	const char *cur_pll_name = NULL;

	tar_pll_name = pll_name[fclk->pll_name_id[tar_level]];
	cur_pll_name = pll_name[fclk->pll_name_id[cur_level]];

	tar_pll = __clk_lookup(tar_pll_name);
	if (IS_ERR_OR_NULL(tar_pll)) {
		pr_err("[%s] %s get failed!\n", __func__, tar_pll_name);
		return -ENODEV;
	}

	cur_pll = __clk_lookup(cur_pll_name);
	if (IS_ERR_OR_NULL(cur_pll)) {
		pr_err("[%s] %s get failed!\n", __func__, cur_pll_name);
		return -ENODEV;
	}

	clk_disable(cur_pll);
	clk_core_unprepare(cur_pll->core);

	if (fclk->en_count) {
		clk_disable(cur_pll);
		clk_core_unprepare(cur_pll->core);
	} else {
		clk_disable(tar_pll);
		clk_core_unprepare(tar_pll->core);
	}

	return 0;
}

static int fast_dvfs_clk_set_rate(struct clk_hw *hw,
	unsigned long rate, unsigned long parent_rate)
{
	struct hi3xxx_fastclk *fclk = NULL;
	unsigned long cur_rate, tar_rate;
	int cur_level, tar_level;
	int ret;

	fclk = container_of(hw, struct hi3xxx_fastclk, hw);
	cur_rate = __clk_get_rate(hw->clk);
	tar_rate = rate;

	cur_level = get_rate_level(hw, cur_rate);
	tar_level = get_rate_level(hw, tar_rate);

	ret = set_rate_before(fclk, cur_level, tar_level);
	if (ret) {
		pr_err("[%s]%s set rate before fail,  tar_level = %d,  cur_level = %d!\n",
			__func__, hw->core->name, tar_level, cur_level);
		return -EINVAL;
	}
	/* Intermediate frequency div set */
	writel(fclk->div_cfg,
		fclk->clkdiv_offset[CFG_OFFSET] + fclk->base_addr);

	/* target sw pll choose set */
	writel(fclk->p_sw_cfg[tar_level],
		fclk->clksw_offset[CFG_OFFSET] + fclk->base_addr);

	/* target div set */
	writel(fclk->p_div_cfg[tar_level],
		fclk->clkdiv_offset[CFG_OFFSET] + fclk->base_addr);

	ret = set_rate_after(fclk, cur_level, tar_level);
	if (ret) {
		pr_err("[%s]%s set rate after fail,  tar_level = %d,  cur_level = %d!\n",
			__func__, hw->core->name, tar_level, cur_level);
		ret = -EINVAL;
	}

	fclk->rate = rate;
	return ret;
}

static int hi3xxx_clkfast_dvfs_enable(struct clk_hw *hw)
{
	struct hi3xxx_fastclk *fclk = container_of(hw, struct hi3xxx_fastclk, hw);
	const char *cur_pll_name = NULL;
	struct clk *cur_pll = NULL;
	int cur_level, ret;

	cur_level = get_rate_level(hw, fclk->rate);
	/* get pll of cur profile */
	cur_pll_name = pll_name[fclk->pll_name_id[cur_level]];
	cur_pll = __clk_lookup(cur_pll_name);
	if (IS_ERR_OR_NULL(cur_pll)) {
		pr_err("[%s] %s get failed!\n", __func__, cur_pll_name);
		return -ENODEV;
	}

	ret = clk_core_prepare(cur_pll->core);
	if (ret) {
		pr_err("[%s] %s clk prepare fail!\n", __func__, cur_pll_name);
		return -ENODEV;
	}

	ret = __clk_enable(cur_pll);
	if (ret) {
		pr_err("[%s] %s enable fail!\n", __func__, cur_pll_name);
		goto err_cur_en;
	}

	/* enable andgt */
	writel(himask_set(fclk->clkgt_cfg[CFG_MASK]),
		fclk->clkgt_cfg[CFG_OFFSET] + fclk->base_addr);

	/* enable gate */
	writel(BIT(fclk->clkgate_cfg[CFG_MASK]),
		fclk->clkgate_cfg[CFG_OFFSET] + fclk->base_addr);

	fclk->en_count++;

	return 0;
err_cur_en:
	clk_core_unprepare(cur_pll->core);
	return -ENODEV;
}

static void hi3xxx_clkfast_dvfs_disable(struct clk_hw *hw)
{
#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	struct hi3xxx_fastclk *fclk = container_of(hw, struct hi3xxx_fastclk, hw);
	const char *cur_pll_name = NULL;
	struct clk *cur_pll = NULL;
	int cur_level;

	cur_level = get_rate_level(hw, fclk->rate);
	/* get pll of cur profile */
	cur_pll_name = pll_name[fclk->pll_name_id[cur_level]];
	cur_pll = __clk_lookup(cur_pll_name);
	if (IS_ERR_OR_NULL(cur_pll)) {
		pr_err("[%s] %s get failed!\n", __func__, cur_pll_name);
		return;
	}

	if (!fclk->always_on)
		writel(BIT(fclk->clkgate_cfg[CFG_MASK]),
			fclk->clkgate_cfg[CFG_OFFSET] + fclk->base_addr +
			HISI_CLK_GATE_DISABLE_OFFSET);

	if (!fclk->always_on)
		writel(himask_unset(fclk->clkgt_cfg[CFG_MASK]),
			fclk->clkgt_cfg[CFG_OFFSET] + fclk->base_addr);

	__clk_disable(cur_pll);
	clk_core_unprepare(cur_pll->core);

	fclk->en_count--;
#endif
}

static const struct clk_ops hi3xxx_clkfast_dvfs_ops = {
	.recalc_rate = fast_dvfs_clk_recalc_rate,
	.set_rate = fast_dvfs_clk_set_rate,
	.determine_rate = fast_dvfs_clk_determine_rate,
	.enable = hi3xxx_clkfast_dvfs_enable,
	.disable = hi3xxx_clkfast_dvfs_disable,
};

static int fastclk_sw_div_init(struct device_node *np, struct hi3xxx_fastclk *fastclk)
{
	int i;
	u32 div_cfg = 0;
	u32 clksw_offset[SW_DIV_CFG_CNT] = {0};
	u32 clkdiv_offset[SW_DIV_CFG_CNT] = {0};

	if (of_property_read_u32(np, "hisilicon,hi3xxx-clk-transition-div", &div_cfg)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-clk-transition-div property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clk-sw-reg", &clksw_offset[0],
		SW_DIV_CFG_CNT)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-clk-sw-reg property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clk-div-reg", &clkdiv_offset[0],
		SW_DIV_CFG_CNT)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-clk-div-reg property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	fastclk->div_cfg = div_cfg;
	for (i = 0; i < SW_DIV_CFG_CNT; i++) {
		fastclk->clksw_offset[i] = clksw_offset[i];
		fastclk->clkdiv_offset[i] = clkdiv_offset[i];
	}

	return 0;
}

static int fastclk_gate_init(struct device_node *np, struct hi3xxx_fastclk *fastclk)
{
	int i;
	u32 clkgt_cfg[GATE_CFG_CNT] = {0};
	u32 clkgate_cfg[GATE_CFG_CNT] = {0};

	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clkgt", &clkgt_cfg[0],
		GATE_CFG_CNT)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-clkgt property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clkgate", &clkgate_cfg[0],
		GATE_CFG_CNT)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-clkgate property!\n",
			__func__, np->name);
		return -ENODEV;
	}

	if (of_property_read_bool(np, "always_on"))
		fastclk->always_on = 1;
	else
		fastclk->always_on = 0;

	for (i = 0; i < GATE_CFG_CNT; i++) {
		fastclk->clkgt_cfg[i] = clkgt_cfg[i];
		fastclk->clkgate_cfg[i] = clkgate_cfg[i];
	}

	return 0;
}

static int fastclk_profile_init(struct device_node *np, struct hi3xxx_fastclk *fastclk)
{
	int i;
	u32 pll_name_id[PROFILE_CNT] = {0};
	u32 p_value[PROFILE_CNT] = {0};
	u32 p_sw_cfg[PROFILE_CNT] = {0};
	u32 p_div_cfg[PROFILE_CNT] = {0};

	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-pll-name-id", &pll_name_id[0],
		PROFILE_CNT)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-pll-name-id property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clkprofile", &p_value[0],
		PROFILE_CNT)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-clkprofile property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-sw-reg", &p_sw_cfg[0],
		PROFILE_CNT)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-sw-reg property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-div-reg", &p_div_cfg[0],
		PROFILE_CNT)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-div-reg property!\n",
			__func__, np->name);
		return -ENODEV;
	}

	for (i = 0; i < PROFILE_CNT; i++) {
		fastclk->p_value[i] = p_value[i];
		fastclk->p_sw_cfg[i] = p_sw_cfg[i];
		fastclk->p_div_cfg[i] = p_div_cfg[i];
		/* ensure pll_name[fclk->pll_name_id[cur_level]] is legal */
		if (pll_name_id[i] < MAX_PLL_NUM) {
			fastclk->pll_name_id[i] = pll_name_id[i];
		} else {
			pr_err("[%s] pll name id %u illelage frome dts, please check!\n",
				__func__, pll_name_id[i]);
			return -ENODEV;
		}
	}

	return 0;
}

static int fastclk_init(struct device_node *np, struct hi3xxx_fastclk *fastclk)
{
	int i;
	u32 pll_profile[PLL_CNT] = {0};

	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clk-pll-rate", &pll_profile[0],
		PLL_CNT)) {
		pr_err("[%s] node %s doesn't have hisilicon,hi3xxx-clk-pll-rate property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (fastclk_sw_div_init(np, fastclk)) {
		pr_err("[%s] node %s sw div init fail!\n", __func__, np->name);
		return -ENODEV;
	}
	if (fastclk_profile_init(np, fastclk)) {
		pr_err("[%s] node %s profile init fail!\n", __func__, np->name);
		return -ENODEV;
	}
	if (fastclk_gate_init(np, fastclk)) {
		pr_err("[%s] node %s gate init fail!\n", __func__, np->name);
		return -ENODEV;
	}

	for (i = 0; i < PLL_CNT; i++)
		fastclk->pll_profile[i] = pll_profile[i];

	fastclk->en_count = 0;

	return 0;
}

/* mux can't go to this process which only support sw */
static void __init kirin_fast_dvfs_setup(struct device_node *np)
{
	struct clk_init_data *init = NULL;
	struct clk *clk = NULL;
	const char *clk_name = NULL;
	struct hi3xxx_fastclk *fastclk = NULL;
	void __iomem *reg_base = NULL;

	reg_base = hs_clk_get_base(np);
	if (reg_base == NULL) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			__func__, np->name);
		return;
	}

	fastclk = kzalloc(sizeof(*fastclk), GFP_KERNEL);
	if (fastclk == NULL)
		return;

	init = hisi_clk_init_data_alloc(clk_name);
	if (init == NULL)
		goto err_init;

	init->ops = &hi3xxx_clkfast_dvfs_ops;
	init->flags = CLK_IS_ROOT;
	init->parent_names = NULL;
	init->num_parents = 0;

	fastclk->hw.init = init;
	fastclk->base_addr = reg_base;

	if (fastclk_init(np, fastclk)) {
		pr_err("[%s] fastclk init error!\n", __func__);
		goto err_clk;
	}

	clk = clk_register(NULL, &fastclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register devfreq_clk %s!\n",
			__func__, clk_name);
		goto err_clk;
	}

	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	clk_register_clkdev(clk, clk_name, NULL);

	return;
err_clk:
	kfree(init->name);
	init->name = NULL;
	kfree(init);
err_init:
	kfree(fastclk);
	pr_err("[%s] clk register fail!\n", __func__);
}

CLK_OF_DECLARE(kirin_fast_dvfs, "hisilicon,hi3xxx-fast-dvfs", kirin_fast_dvfs_setup);
