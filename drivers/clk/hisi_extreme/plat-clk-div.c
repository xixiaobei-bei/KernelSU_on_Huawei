/*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include "plat-clk-div.h"

static unsigned int hi3xxx_get_table_maxdiv(const struct clk_div_table *table)
{
	unsigned int maxdiv = 0;
	const struct clk_div_table *clkt = NULL;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div > maxdiv)
			maxdiv = clkt->div;
	return maxdiv;
}

static unsigned int hi3xxx_get_table_div(const struct clk_div_table *table, unsigned int val)
{
	const struct clk_div_table *clkt = NULL;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->val == val)
			return clkt->div;
	return 0;
}

static unsigned int hi3xxx_get_table_val(const struct clk_div_table *table, unsigned int div)
{
	const struct clk_div_table *clkt = NULL;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div == div)
			return clkt->val;
	return 0;
}

static unsigned long hi3xxx_clkdiv_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct hi3xxx_divclk *dclk = container_of(hw, struct hi3xxx_divclk, hw);
	unsigned int div, val;

	val = (unsigned int)readl(dclk->reg) >> dclk->shift;
	val &= WIDTH_TO_MASK(dclk->width);

	div = hi3xxx_get_table_div(dclk->table, val);
	if (!div) {
		pr_warn("%s: Invalid divisor for clock %s\n", __func__,
			__clk_get_name(hw->clk));
		return parent_rate;
	}
	return parent_rate / div;
}

static bool hi3xxx_is_valid_table_div(const struct clk_div_table *table, unsigned int div)
{
	const struct clk_div_table *clkt = NULL;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div == div)
			return true;
	return false;
}

static unsigned long hi3xxx_clkdiv_bestdiv(struct clk_hw *hw, unsigned long rate,
	unsigned long *best_parent_rate)
{
	struct hi3xxx_divclk *dclk = container_of(hw, struct hi3xxx_divclk, hw);
	struct clk *clk_parent = __clk_get_parent(hw->clk);
	unsigned long bestdiv = 0;
	unsigned long parent_rate, now, maxdiv, i;
	unsigned long best_rate = 0;

	maxdiv = hi3xxx_get_table_maxdiv(dclk->table);

	if (rate == 0) {
		pr_err("[%s] :rate == 0 is illegal!\n", __func__);
		return 0;
	}

	if (!(__clk_get_flags(hw->clk) & CLK_SET_RATE_PARENT)) {
		parent_rate = *best_parent_rate;
		bestdiv = DIV_ROUND_UP(parent_rate, rate);
		bestdiv = bestdiv == 0 ? 1 : bestdiv;
		bestdiv = bestdiv > maxdiv ? maxdiv : bestdiv;
		return bestdiv;
	}

	/*
	 * The maximum divider we can use without overflowing
	 * unsigned long in rate * i below
	 */
	maxdiv = min(ULONG_MAX / rate, maxdiv);

	for (i = 1; i <= maxdiv; i++) {
		if (!hi3xxx_is_valid_table_div(dclk->table, i))
			continue;
		parent_rate = __clk_round_rate(clk_parent, MULT_ROUND_UP(rate, i));
		now = parent_rate / i;
		if (now <= rate && now > best_rate) {
			bestdiv = i;
			best_rate = now;
			*best_parent_rate = parent_rate;
		}
	}

	if (!bestdiv) {
		bestdiv = hi3xxx_get_table_maxdiv(dclk->table);
		*best_parent_rate = __clk_round_rate(clk_parent, 1);
	}
	return bestdiv;
}

static long hi3xxx_clkdiv_round_rate(struct clk_hw *hw, unsigned long rate,
	unsigned long *prate)
{
	unsigned long div;

	if (!rate)
		rate = 1;
	div = hi3xxx_clkdiv_bestdiv(hw, rate, prate);
	if (!div) {
		pr_err("[%s] : div == 0 is illegal!\n", __func__);
		return 0;
	}
	return *prate / div;
}

static int hi3xxx_clkdiv_set_rate(struct clk_hw *hw, unsigned long rate,
	unsigned long parent_rate)
{
	struct hi3xxx_divclk *dclk = container_of(hw, struct hi3xxx_divclk, hw);
	unsigned int div, value;
	unsigned long flags = 0;
	u32 data;

	if (rate == 0) {
		pr_err("[%s] : rate == 0 is illegal!\n", __func__);
		return 0;
	}

	div = parent_rate / rate;
	value = hi3xxx_get_table_val(dclk->table, div);

	if (value > (unsigned int)WIDTH_TO_MASK(dclk->width))
		value = WIDTH_TO_MASK(dclk->width);

	if (dclk->lock)
		spin_lock_irqsave(dclk->lock, flags);

	data = readl(dclk->reg);
	data &= ~((unsigned int)WIDTH_TO_MASK(dclk->width) << dclk->shift);
	data |= value << dclk->shift;
	data |= dclk->mbits;
	writel(data, dclk->reg);

#ifdef CONFIG_HISI_CLK_WAIT_DONE
	if ((dclk->div_done) &&
	    (dclk->hw.core->parent) &&
	    (clk_gate_is_enabled(dclk->hw.core->parent->hw))) {
		udelay(1);
		if (!(readl(dclk->div_done) & BIT(dclk->done_bit)))
			pr_err("[%s]: [%s] not complete at %lu rate!\n",
				__func__, dclk->hw.core->name, rate);
	}
#endif

	if (dclk->lock)
		spin_unlock_irqrestore(dclk->lock, flags);

	return 0;
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hi3xxx_divreg_check(struct clk_hw *hw)
{
	unsigned long rate;
	struct clk *clk = hw->clk;
	struct clk *pclk = clk_get_parent(clk);

	rate = hi3xxx_clkdiv_recalc_rate(hw, clk_get_rate(pclk));
	if (rate == clk_get_rate(clk))
		return 1;
	else
		return 0;
}

static void __iomem *hi3xxx_clkdiv_get_reg(struct clk_hw *hw)
{
	struct hi3xxx_divclk *dclk = NULL;
	void __iomem *ret = NULL;
	u32 val;

	dclk = container_of(hw, struct hi3xxx_divclk, hw);

	if (dclk->reg) {
		ret = dclk->reg;
		val = readl(ret);
		val &= dclk->mbits;
		pr_info("\n[%s]: reg = 0x%pK, bits = 0x%x, regval = 0x%x\n",
			__clk_get_name(hw->clk), ret, dclk->mbits, val);
	}

	return ret;
}

static int hi3xxx_dumpdiv(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	struct hi3xxx_divclk *dclk = container_of(hw, struct hi3xxx_divclk, hw);
	void __iomem *ret = NULL;
	unsigned long int clk_bask_addr = 0;
	unsigned int clk_bit = 0;
	u32 val;
	int s_ret;

	if ((dclk->reg != NULL) && (buf != NULL) && (s == NULL) && (buf_length > 0)) {
		ret = dclk->reg;
		val = readl(ret);
		s_ret = snprintf_s(buf, buf_length, buf_length - 1,
			"[%s] : regAddress = 0x%pK, regval = 0x%x\n",
			__clk_get_name(hw->clk), dclk->reg, val);
		if (s_ret == -1)
			pr_err("%s snprintf_s failed!\n", __func__);
	}
	if ((dclk->reg != NULL) && (buf == NULL) && (s != NULL)) {
		clk_bask_addr = (uintptr_t) dclk->reg & CLK_ADDR_HIGH_MASK;
		clk_bit = (uintptr_t) dclk->reg & CLK_ADDR_LOW_MASK;
		seq_printf(s, "    %-15s    %-15s    0x%03X    bit-%u:%u",
			hs_base_addr_transfer(clk_bask_addr), "div", clk_bit,
			dclk->shift, (dclk->shift + dclk->width - 1));
	}

	return 0;
}
#endif

static const struct clk_ops hi3xxx_clkdiv_ops = {
	.recalc_rate = hi3xxx_clkdiv_recalc_rate,
	.round_rate = hi3xxx_clkdiv_round_rate,
	.set_rate = hi3xxx_clkdiv_set_rate,
#ifdef CONFIG_HISI_CLK_DEBUG
	.check_divreg = hi3xxx_divreg_check,
	.get_reg = hi3xxx_clkdiv_get_reg,
	.dump_reg = hi3xxx_dumpdiv,
#endif
};

static struct clk_div_table *__hi3xxx_clkdiv_table_set(const struct div_clock *div)
{
	struct clk_div_table *table = NULL;
	unsigned int table_num;
	unsigned int multiple = 1;
	unsigned int i;

	table_num = div->max_div - div->min_div + 1;

	/* table ends with <0, 0>, so plus one to table_num */
	table = kzalloc(sizeof(*table) * (table_num + 1), GFP_KERNEL);
	if (table == NULL) {
		pr_err("[%s] fail to alloc table!\n", __func__);
		return table;
	}

	if (div->multiple)
		multiple = 2;
	else
		multiple = 1;

	for (i = 0; i < table_num; i++) {
		table[i].div = (div->min_div + i) * multiple;
		table[i].val = i;
	}

	return table;
}
static struct clk *__clk_register_divider(const struct div_clock *div,
	struct clock_data *data)
{
	struct clk *clk = NULL;
	struct clk_init_data init;
	struct clk_div_table *table = NULL;
	struct hi3xxx_divclk *dclk = NULL;
	struct hs_clk *hs_clk = get_hs_clk_info();

	table = __hi3xxx_clkdiv_table_set(div);
	if (table == NULL)
		return clk;

	dclk = kzalloc(sizeof(*dclk), GFP_KERNEL);
	if (dclk == NULL) {
		pr_err("[%s] fail to alloc dclk!\n", __func__);
		goto err_dclk;
	}

	init.name = div->name;
	init.ops = &hi3xxx_clkdiv_ops;
	init.flags = CLK_SET_RATE_PARENT;
	init.parent_names = &(div->parent_name);
	init.num_parents = 1;

	dclk->reg = data->base + div->offset;
	dclk->shift = ffs(div->div_mask) - 1;
	dclk->width = fls(div->div_mask) - ffs(div->div_mask) + 1;
	dclk->mbits = div->div_mask << DIV_MASK_OFFSET;
#ifdef CONFIG_HISI_CLK_WAIT_DONE
	if (div->div_done) {
		dclk->div_done = data->base + div->div_done;
		dclk->done_bit = div->done_bit;
	}
#endif
	dclk->lock = &hs_clk->lock;
	dclk->hw.init = &init;
	dclk->table = table;

	clk = clk_register(NULL, &dclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register div clk %s!\n",
				__func__, div->name);
		goto err_init;
	}
	/* init is local variable, need set NULL before func */
	dclk->hw.init = NULL;
	return clk;

err_init:
	kfree(dclk);
err_dclk:
	kfree(table);
	return clk;
}

void plat_clk_register_divider(const struct div_clock *clks,
	int nums, struct clock_data *data)
{
	struct clk *clk = NULL;
	int i;

	for (i = 0; i < nums; i++) {
		clk = __clk_register_divider(&clks[i], data);
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}

#ifdef CONFIG_HISI_CLK_DEBUG
		debug_clk_add(clk, CLOCK_DIV);
#endif

		clk_log_dbg("clks id %d, nums %d, clks name = %s!\n",
			clks[i].id, nums, clks[i].name);

		clk_data_init(clk, clks[i].alias, clks[i].id, data);
	}
}

