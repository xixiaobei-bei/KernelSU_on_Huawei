/*
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "clk-fast-dvfs.h"
#include <linux/clk.h>
#include <securec.h>
#include <linux/spinlock.h>

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

static const char *pll_name[MAX_PLL_NUM] = { "clk_ppll0_media", "clk_ppll2_media",
	"clk_ppll2b_media", "clk_ppll3_media" };

static DEFINE_SPINLOCK(fast_dvfs_lock);

#define min_div_cfg(v1, v2)	((v1) > (v2) ? (v2) : (v1))
#define max_div_cfg(v1, v2)	((v1) > (v2) ? (v1) : (v2))

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
		pr_err("[%s] %s tar_pll clk_core_prepare fail, ret = %d, pre_cnt = %u!\n",
			__func__, tar_pll_name, ret,
			tar_pll->core->prepare_count);
		return -ENODEV;
	}
	ret = clk_enable(tar_pll);
	if (ret) {
		pr_err("[%s] %s tar_pll clk enable fail, ret = %d!\n",
			__func__, tar_pll_name, ret);
		goto err_tar_en;
	}

	ret = clk_core_prepare(cur_pll->core);
	if (ret) {
		pr_err("[%s] %s cur_pll clk_core_prepare fail, ret = %d, pre_cnt = %u!\n",
			__func__, tar_pll_name, ret,
			cur_pll->core->prepare_count);
		goto err_cur_pre;
	}
	ret = clk_enable(cur_pll);
	if (ret) {
		pr_err("[%s] %s cur_pll clk enable fail, ret = %d!\n",
			__func__, cur_pll_name, ret);
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

static u32 get_inter_div(struct hi3xxx_fastclk *fclk, unsigned long cur_rate,
	unsigned long tar_rate, int cur_level, int tar_level)
{
	u32 min_div, max_div, pll_id;
	unsigned long cur_pll_profile, tar_pll_profile, max_rate;

	min_div = min_div_cfg(fclk->div_val[cur_level], fclk->div_val[tar_level]);
	max_div = max_div_cfg(fclk->div_val[cur_level], fclk->div_val[tar_level]);
	max_rate = max_div_cfg(cur_rate, tar_rate) + ADD_FREQ_OFFSET;

	if (min_div == 0)
		goto err_ret;

	pll_id = fclk->pll_profile_id[cur_level];
	if (pll_id >= PLL_CNT)
		goto err_ret;

	cur_pll_profile = fclk->pll_profile[pll_id];
	cur_pll_profile = cur_pll_profile * FREQ_CONVERSION;

	pll_id = fclk->pll_profile_id[tar_level];
	if (pll_id >= PLL_CNT)
		goto err_ret;

	tar_pll_profile = fclk->pll_profile[pll_id];
	tar_pll_profile = tar_pll_profile * FREQ_CONVERSION;

	for (; min_div < max_div; min_div++) {
		if ((cur_pll_profile / min_div) <= max_rate &&
			(tar_pll_profile / min_div) <= max_rate)
			return min_div;
	}

err_ret:
	return max_div;
}

static int fast_dvfs_clk_set_rate(struct clk_hw *hw,
	unsigned long rate, unsigned long parent_rate)
{
	struct hi3xxx_fastclk *fclk = NULL;
	unsigned long cur_rate, tar_rate;
	int cur_level, tar_level;
	int ret;
	u32 inter_div_cfg;
	void __iomem *div_addr = NULL;
	void __iomem *sw_addr = NULL;
	unsigned long flags;

	fclk = container_of(hw, struct hi3xxx_fastclk, hw);
	cur_rate = __clk_get_rate(hw->clk);
	tar_rate = rate;

	cur_level = get_rate_level(hw, cur_rate);
	tar_level = get_rate_level(hw, tar_rate);

	inter_div_cfg = get_inter_div(fclk, cur_rate, tar_rate, cur_level, tar_level);
	inter_div_cfg = inter_div_cfg - 1;

	inter_div_cfg = (fclk->clkdiv_offset[CFG_MASK] << HIMASKEN_SHIFT) +
		(inter_div_cfg << fclk->clkdiv_offset[SHIFT]);

	div_addr = fclk->clkdiv_offset[CFG_OFFSET] + fclk->base_addr;
	sw_addr = fclk->clksw_offset[CFG_OFFSET] + fclk->base_addr;

	ret = set_rate_before(fclk, cur_level, tar_level);
	if (ret) {
		pr_err("[%s]%s set rate before fail,  tar_level = %d,  cur_level = %d!\n",
			__func__, hw->core->name, tar_level, cur_level);
		return -EINVAL;
	}

	spin_lock_irqsave(&fast_dvfs_lock, flags);
	/* Intermediate frequency div set */
	writel(inter_div_cfg, div_addr);

	/* target sw pll choose set */
	writel(fclk->p_sw_cfg[tar_level], sw_addr);

	/* target div set */
	writel(fclk->p_div_cfg[tar_level], div_addr);
	spin_unlock_irqrestore(&fast_dvfs_lock, flags);

	ret = set_rate_after(fclk, cur_level, tar_level);
	if (ret) {
		pr_err("[%s]%s set rate after fail,  tar_level = %d,  cur_level = %d!\n",
			__func__, hw->core->name, tar_level, cur_level);
		ret = -EINVAL;
	}

	fclk->rate = rate;
	return ret;
}

static int hi3xxx_clkfast_dvfs_prepare(struct clk_hw *hw)
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

	ret = clk_prepare(cur_pll);
	if (ret) {
		pr_err("[%s] %s clk prepare fail, ret = %d, pre_cnt = %u!\n",
			__func__, cur_pll_name, ret,
			cur_pll->core->prepare_count);
		return -ENODEV;
	}

	return 0;
}

static void hi3xxx_clkfast_dvfs_unprepare(struct clk_hw *hw)
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
	clk_unprepare(cur_pll);
#endif
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

	ret = clk_enable(cur_pll);
	if (ret) {
		pr_err("[%s] %s enable fail!\n", __func__, cur_pll_name);
		return -ENODEV;
	}

	/* enable andgt */
	writel(himask_set(fclk->clkgt_cfg[CFG_MASK]),
		fclk->clkgt_cfg[CFG_OFFSET] + fclk->base_addr);

	/* enable gate */
	writel(BIT(fclk->clkgate_cfg[CFG_MASK]),
		fclk->clkgate_cfg[CFG_OFFSET] + fclk->base_addr);

	fclk->en_count++;

	return 0;
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
			CLK_GATE_DISABLE_OFFSET);

	if (!fclk->always_on)
		writel(himask_unset(fclk->clkgt_cfg[CFG_MASK]),
			fclk->clkgt_cfg[CFG_OFFSET] + fclk->base_addr);

	clk_disable(cur_pll);

	fclk->en_count--;
#endif
}

static const struct clk_ops hi3xxx_clkfast_dvfs_ops = {
	.recalc_rate = fast_dvfs_clk_recalc_rate,
	.set_rate = fast_dvfs_clk_set_rate,
	.determine_rate = fast_dvfs_clk_determine_rate,
	.prepare = hi3xxx_clkfast_dvfs_prepare,
	.unprepare = hi3xxx_clkfast_dvfs_unprepare,
	.enable = hi3xxx_clkfast_dvfs_enable,
	.disable = hi3xxx_clkfast_dvfs_disable,
};

static void fastclk_sw_div_init(const struct fast_dvfs_clock *fast_dvfs,
	struct hi3xxx_fastclk *fastclk)
{
	int i;

	for (i = 0; i < PROFILE_CNT; i++) {
		fastclk->div_val[i] = fast_dvfs->div_val[i];
		fastclk->pll_profile_id[i] = fast_dvfs->pll_profile_id[i];
	}

	for (i = 0; i < SW_DIV_CFG_CNT; i++) {
		fastclk->clksw_offset[i] = fast_dvfs->clksw_offset[i];
		fastclk->clkdiv_offset[i] = fast_dvfs->clkdiv_offset[i];
	}
}

static void fastclk_gate_init(const struct fast_dvfs_clock *fast_dvfs,
	struct hi3xxx_fastclk *fastclk)
{
	int i;

	fastclk->always_on = fast_dvfs->always_on;

	for (i = 0; i < GATE_CFG_CNT; i++) {
		fastclk->clkgt_cfg[i] = fast_dvfs->clkgt_cfg[i];
		fastclk->clkgate_cfg[i] = fast_dvfs->clkgate_cfg[i];
	}
}

static int fastclk_profile_init(const struct fast_dvfs_clock *fast_dvfs,
	struct hi3xxx_fastclk *fastclk)
{
	int i;

	for (i = 0; i < PROFILE_CNT; i++) {
		fastclk->p_value[i] = fast_dvfs->p_value[i];
		fastclk->p_sw_cfg[i] = fast_dvfs->p_sw_cfg[i];
		fastclk->p_div_cfg[i] = fast_dvfs->p_div_cfg[i];
		/* ensure pll_name[fclk->pll_name_id[cur_level]] is legal */
		if (fast_dvfs->pll_name_id[i] < MAX_PLL_NUM) {
			fastclk->pll_name_id[i] = fast_dvfs->pll_name_id[i];
		} else {
			pr_err("[%s] pll name id %u illelage frome dts, please check!\n",
				__func__, fast_dvfs->pll_name_id[i]);
			return -ENODEV;
		}
	}

	return 0;
}

static int fastclk_init(const struct fast_dvfs_clock *fast_dvfs,
	struct hi3xxx_fastclk *fastclk)
{
	int i;

	fastclk_sw_div_init(fast_dvfs, fastclk);
	if (fastclk_profile_init(fast_dvfs, fastclk)) {
		pr_err("[%s] node profile init fail!\n", __func__);
		return -ENODEV;
	}
	fastclk_gate_init(fast_dvfs, fastclk);

	for (i = 0; i < PLL_CNT; i++)
		fastclk->pll_profile[i] = fast_dvfs->pll_profile[i];

	fastclk->en_count = 0;

	return 0;
}

/* mux can't go to this process which only support sw */
static struct clk *__clk_register_fast_dvfs(const struct fast_dvfs_clock *fast_dvfs,
	struct clock_data *data)
{
	struct clk_init_data init;
	struct clk *clk = NULL;
	struct hi3xxx_fastclk *fastclk = NULL;

	fastclk = kzalloc(sizeof(*fastclk), GFP_KERNEL);
	if (fastclk == NULL)
		return clk;

	init.name = fast_dvfs->name;
	init.ops = &hi3xxx_clkfast_dvfs_ops;
	init.flags = CLK_IS_ROOT;
	init.parent_names = NULL;
	init.num_parents = 0;

	fastclk->hw.init = &init;
	fastclk->base_addr = data->base;

	if (fastclk_init(fast_dvfs, fastclk)) {
		pr_err("[%s] fastclk init error!\n", __func__);
		goto err_init;
	}

	clk = clk_register(NULL, &fastclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register devfreq_clk %s!\n",
			__func__, fast_dvfs->name);
		goto err_init;
	}

	return clk;
err_init:
	kfree(fastclk);
	pr_err("[%s] clk register fail!\n", __func__);
	return clk;
}

void plat_clk_register_fast_dvfs_clk(const struct fast_dvfs_clock *clks,
	int nums, struct clock_data *data)
{
	struct clk *clk = NULL;
	int i;

	for (i = 0; i < nums; i++) {
		clk = __clk_register_fast_dvfs(&clks[i], data);
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}

#ifdef CONFIG_HISI_CLK_DEBUG
		debug_clk_add(clk, CLOCK_FAST_CLOCK);
#endif

		clk_log_dbg("clks id %d, nums %d, clks name = %s!\n",
			clks[i].id, nums, clks[i].name);

		clk_data_init(clk, clks[i].alias, clks[i].id, data);
	}
}

