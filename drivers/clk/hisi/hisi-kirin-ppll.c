/*
 * hisi-kirin-ppll.c
 *
 * Hisilicon clock driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include "hisi-kirin-ppll.h"

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

#ifdef CONFIG_HISI_CLK_DEBUG
#include "debug/hisi-clk-debug.h"
#endif

static int kirin_ppll_enable_open(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	u32 val;

	/* en */
	switch (ppll) {
	case PPLL2:
	case PPLL2_B:
	case PPLL3:
	case PPLL5:
	case PPLL6:
	case PPLL7:
	case FNPLL1:
	case FNPLL4:
	case AUPLL:
		val = BIT(ppll_clk->gt_ctrl[1] + PLL_MASK_OFFSET);
		writel(val, ppll_clk->addr + ppll_clk->gt_ctrl[0]); /* gt 1'b0 */
		val = BIT(ppll_clk->en_ctrl[1] + PLL_MASK_OFFSET);
		val |= BIT(ppll_clk->en_ctrl[1]);
		writel(val, ppll_clk->addr + ppll_clk->en_ctrl[0]); /* en 1'b1 */
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_open\n", __func__, ppll);
		break;
	}
	return 0;
}

static void kirin_ppll_nogate(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	u32 val;

	/* output clock not gate */
	switch (ppll) {
	case PPLL2:
	case PPLL2_B:
	case PPLL3:
	case PPLL5:
	case PPLL6:
	case PPLL7:
	case FNPLL1:
	case FNPLL4:
	case AUPLL:
		val = BIT(ppll_clk->bypass_ctrl[1] + PLL_MASK_OFFSET);
		writel(val, ppll_clk->addr + ppll_clk->bypass_ctrl[0]); /* bypass  1'b0 */
		val = BIT(ppll_clk->gt_ctrl[1] + PLL_MASK_OFFSET);
		val |= BIT(ppll_clk->gt_ctrl[1]);
		writel(val, ppll_clk->addr + ppll_clk->gt_ctrl[0]); /* gt 1'b1 */
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_ready\n", __func__, ppll);
		break;
	}
}

static int kirin_ppll_enable_ready(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	u32 val;
	u32 timeout = 0;

	/* waiting lock */
	switch (ppll) {
	case PPLL2:
	case PPLL2_B:
	case PPLL3:
	case PPLL5:
	case PPLL6:
	case PPLL7:
	case FNPLL1:
	case FNPLL4:
		do {
			val = readl(ppll_clk->endisable_addr + ppll_clk->pll_ctrl0);
			val &= BIT(PPLLCTRL0_LOCK);
			timeout++;
			if (timeout > AP_PPLL_STABLE_TIME) {
				pr_err("%s: ppll-%d enable is timeout\n", __func__, ppll);
				break;
			}
		} while (!val);
		break;
	case AUPLL:
		do {
			val = readl(AUPLL_LOCK_ADDR(ppll_clk->endisable_addr));
			val &= BIT(AUPLL_LOCK);
			timeout++;
			if (timeout > AP_PPLL_STABLE_TIME) {
				pr_err("%s: ppll-%d enable is timeout!\n", __func__, ppll);
				break;
			}
		} while (!val);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_ready\n", __func__, ppll);
		break;
	}
	kirin_ppll_nogate(ppll_clk, ppll);

	return 0;
}

static void kirin_ppll_disable(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	u32 val;

	/* output clock gate */
	switch (ppll) {
	case PPLL2:
	case PPLL2_B:
	case PPLL3:
	case PPLL5:
	case PPLL6:
	case PPLL7:
	case FNPLL1:
	case FNPLL4:
	case AUPLL:
		/* output clock gate */
		val = BIT(ppll_clk->gt_ctrl[1] + PLL_MASK_OFFSET);
		writel(val, ppll_clk->addr + ppll_clk->gt_ctrl[0]); /* gt 1'b0 */

		/* bypass 1'b1 */
		val = BIT(ppll_clk->bypass_ctrl[1] + PLL_MASK_OFFSET);
		val |= BIT(ppll_clk->bypass_ctrl[1]);
		writel(val, ppll_clk->addr + ppll_clk->bypass_ctrl[0]); /* bypass 1'b1 */

		/* ~en */
		val = BIT(ppll_clk->en_ctrl[1] + PLL_MASK_OFFSET);
		writel(val, ppll_clk->addr + ppll_clk->en_ctrl[0]); /* en 1'b0 */

		/* output clock nogate */
		val = BIT(ppll_clk->gt_ctrl[1] + PLL_MASK_OFFSET);
		val |= BIT(ppll_clk->gt_ctrl[1]);
		writel(val, ppll_clk->addr + ppll_clk->gt_ctrl[0]); /* gt 1'b1 */
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is disable\n", __func__, ppll);
		break;
	}
}

static int get_ppll_state(struct hi3xxx_ppll_clk *ppll_clk)
{
	int ppll;
	u32 val, ret;

	ppll = ppll_clk->en_cmd[1];

	switch (ppll) {
	case PPLL2:
	case PPLL2_B:
	case PPLL3:
	case PPLL5:
	case PPLL6:
	case PPLL7:
	case FNPLL1:
	case FNPLL4:
	case AUPLL:
		/* en need 1 */
		val = readl(ppll_clk->addr + ppll_clk->en_ctrl[0]);
		if (!(val & BIT(ppll_clk->en_ctrl[1])))
			return 0;
		/* gt need 1 */
		val = readl(ppll_clk->addr + ppll_clk->gt_ctrl[0]);
		if (!(val & BIT(ppll_clk->gt_ctrl[1])))
			return 0;
		/* bypass need 0 */
		val = readl(ppll_clk->addr + ppll_clk->bypass_ctrl[0]);
		if (val & BIT(ppll_clk->bypass_ctrl[1]))
			return 0;
		ret = 1;
		break;
	default:
		ret = 0;
		pr_info("[%s]: A wrong ppll[%d] !\n", __func__, ppll);
		break;
	}

	return ret;
}

static int kirin_multicore_ppll_enable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);
	u32 ret;

	/* for debug */
	ppll_clk->ref_cnt++;

	if (ppll_clk->en_cmd[1] == PPLL0)
		return 0;
	if (ppll_clk->ref_cnt == 1) {
		ret = get_ppll_state(ppll_clk);
		if (ret)
			return 0;
		kirin_ppll_enable_open(ppll_clk, ppll_clk->en_cmd[1]);
		kirin_ppll_enable_ready(ppll_clk, ppll_clk->en_cmd[1]);
	}

	return 0;
}

static void kirin_multicore_ppll_disable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);

	/* for debug */
	ppll_clk->ref_cnt--;
	if (ppll_clk->dis_cmd[1] == PPLL0)
		return;
#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	if (!ppll_clk->ref_cnt)
		kirin_ppll_disable(ppll_clk, ppll_clk->dis_cmd[1]);
#endif
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int kirin_pll_dumpgate(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;
	unsigned long  int clk_base_addr = 0;

	ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);
	if (buf == NULL && s != NULL) {
		clk_base_addr = (uintptr_t)ppll_clk->addr & CLK_ADDR_HIGH_MASK;
		seq_printf(s, "    %-15s    %-15s    en[0x%03X-%u]    gt[0x%03X-%u]    bp[0x%03X-%u]    ctrl0[0x%03X]",
			hs_base_addr_transfer(clk_base_addr), "pll", ppll_clk->en_ctrl[0],
			ppll_clk->en_ctrl[1], ppll_clk->gt_ctrl[0], ppll_clk->gt_ctrl[1],
			ppll_clk->bypass_ctrl[0], ppll_clk->bypass_ctrl[1], ppll_clk->pll_ctrl0);
	}

	return 0;
}
#endif

static const struct clk_ops kirin_ppll_ops = {
	.enable 	= kirin_multicore_ppll_enable,
	.disable	= kirin_multicore_ppll_disable,
#ifdef CONFIG_HISI_CLK_DEBUG
	.dump_reg = kirin_pll_dumpgate,
#endif
};

static int __kirin_ppll_clk_register(struct device_node *np, struct hi3xxx_ppll_clk *ppll_clk,
	const char *clk_name)
{
	struct clk *clk = NULL;

	clk = clk_register(NULL, &ppll_clk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n", __func__, clk_name);
		return -1;
	}
	clk_register_clkdev(clk, clk_name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);

	return 0;
}

static int __kirin_lpm3_cmd_setup(struct device_node *np, struct hi3xxx_ppll_clk *ppll_clk)
{
	u32 en_cmd[LPM3_CMD_LEN] = {0};
	u32 dis_cmd[LPM3_CMD_LEN] = {0};
	int i;

	if (of_property_read_u32_array(np, "hisilicon,ipc-lpm3-cmd-en",
		&en_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] %s node doesn't have hisilicon,ipc-lpm3-cmd property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32_array(np, "hisilicon,ipc-lpm3-cmd-dis",
		&dis_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] %s node doesn't have hisilicon,ipc-lpm3-cmd property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	for (i = 0; i < LPM3_CMD_LEN; i++) {
		ppll_clk->en_cmd[i] = en_cmd[i];
		ppll_clk->dis_cmd[i] = dis_cmd[i];
	}

	return 0;
}

static int __kirin_pll_reg_setup(struct device_node *np, struct hi3xxx_ppll_clk *ppll_clk)
{
	u32 en_ctrl[PLL_REG_NUM] = {0};
	u32 gt_ctrl[PLL_REG_NUM] = {0};
	u32 bypass_ctrl[PLL_REG_NUM] = {0};
	u32 pllctrl0_base = HS_PMCTRL;
	u32 pll_ctrl0 = 0;
	int i;

	if (of_property_read_bool(np, "hisilicon,pll-en-reg")) {
		if (of_property_read_u32_array(np, "hisilicon,pll-en-reg",
			&en_ctrl[0], PLL_REG_NUM)) {
			pr_err("[%s] %s node doesn't have hisilicon,pll-en-reg property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}
	if (of_property_read_bool(np, "hisilicon,pll-gt-reg")) {
		if (of_property_read_u32_array(np, "hisilicon,pll-gt-reg",
			&gt_ctrl[0], PLL_REG_NUM)) {
			pr_err("[%s] %s node doesn't have hisilicon,pll-gt-reg property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}
	if (of_property_read_bool(np, "hisilicon,pll-bypass-reg")) {
		if (of_property_read_u32_array(np, "hisilicon,pll-bypass-reg",
			&bypass_ctrl[0], PLL_REG_NUM)) {
			pr_err("[%s] %s node doesn't have hisilicon,pll-bypass-reg property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}
	if (of_property_read_bool(np, "hisilicon,pll-ctrl0-reg")) {
		if (of_property_read_u32(np, "hisilicon,pll-ctrl0-reg", &pll_ctrl0)) {
			pr_err("[%s] node %s doesn't pll-ctrl0-reg property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}
	if (of_property_read_bool(np, "hisilicon,pll-ctrl0-base-reg")) {
		if (of_property_read_u32(np, "hisilicon,pll-ctrl0-base-reg", &pllctrl0_base)) {
			pr_err("[%s] node %s doesn't pll-ctrl0-base-reg property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}

	for (i = 0; i < PLL_REG_NUM; i++) {
		ppll_clk->en_ctrl[i] = en_ctrl[i];
		ppll_clk->gt_ctrl[i] = gt_ctrl[i];
		ppll_clk->bypass_ctrl[i] = bypass_ctrl[i];
	}
	/* PLL ctrl0 reg_base */
	ppll_clk->flags = 0;
	ppll_clk->endisable_addr = hs_clk_base(pllctrl0_base);
	ppll_clk->pll_ctrl0 = pll_ctrl0;

	return 0;
}

static const char *__kirin_ppll_clock_name_read(struct device_node *np)
{
	const char *clk_name = NULL;

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			__func__, np->name);
		return NULL;
	}

	return clk_name;
}

static void __init kirin_ppll_setup(struct device_node *np)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;
	struct clk_init_data *init = NULL;
	const char *clk_name = NULL;
	const char *parent_names = NULL;
	void __iomem *reg_base = NULL;
	int ret;

	reg_base = hs_clk_get_base(np);
	if (reg_base == NULL) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}
	clk_name = __kirin_ppll_clock_name_read(np);
	if (clk_name == NULL)
		return;

	parent_names = of_clk_get_parent_name(np, 0);

	ppll_clk = kzalloc(sizeof(*ppll_clk), GFP_KERNEL);
	if (ppll_clk == NULL) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		goto err_ppll_clk;
	}
	/* initialize the reference count */
	ppll_clk->ref_cnt = 0;

	ret = __kirin_pll_reg_setup(np, ppll_clk);
	if (ret)
		goto err_init;

	ret = __kirin_lpm3_cmd_setup(np, ppll_clk);
	if (ret)
		goto err_init;

	init = hisi_clk_init_data_alloc(clk_name);
	if (init == NULL)
		goto err_init;
	init->ops = &kirin_ppll_ops;
	init->flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
	init->num_parents = 1;
	init->parent_names = &parent_names;

	ppll_clk->lock = &hs_clk.lock;
	ppll_clk->hw.init = init;
	/* PLL gt,en,by reg_base */
	ppll_clk->addr = reg_base;
	ppll_clk->sctrl = NULL;
	ret = __kirin_ppll_clk_register(np, ppll_clk, clk_name);
	if (ret)
		goto err_clk;

	return;
err_clk:
	kfree(init->name);
	kfree(init);
err_init:
	kfree(ppll_clk);
err_ppll_clk:
	return;
}

/* suit plat before boston */
static int ppll_enable_open(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	u32 val;

	/* en */
	switch (ppll) {
	case PPLL3:
		val = readl(PPLL3_EN_ACPU_ADDR(ppll_clk->endisable_addr));
		val |= BIT(PPLL3_EN_ACPU);
		writel(val, PPLL3_EN_ACPU_ADDR(ppll_clk->endisable_addr));
		break;
	case PPLL2:
		val = readl(PPLL2_EN_ACPU_ADDR(ppll_clk->endisable_addr));
		val |= BIT(PPLL2_EN_ACPU);
		writel(val, PPLL2_EN_ACPU_ADDR(ppll_clk->endisable_addr));
		break;
	case PPLL4:
		val = readl(PPLL4_EN_ACPU_ADDR(ppll_clk->addr));
		val |= BIT(PPLL4_EN);
		writel(val, PPLL4_EN_ACPU_ADDR(ppll_clk->addr));
		break;
	case PPLL6:
		val = readl(PPLL6_EN_ACPU_ADDR(ppll_clk->addr));
		val |= BIT(PPLL6_EN);
		writel(val, PPLL6_EN_ACPU_ADDR(ppll_clk->addr));
		break;
	case PPLL7:
		val = readl(PPLL7_EN_ACPU_ADDR(ppll_clk->addr));
		val |= BIT(PPLL7_EN);
		writel(val, PPLL7_EN_ACPU_ADDR(ppll_clk->addr));
		break;
	case SCPLL:
		/* set gt b0 */
		val = (unsigned int)readl(SCPLL_GT_ACPU_ADDR(ppll_clk->addr));
		val &= (~BIT(SCPLL_GT));
		writel(val, SCPLL_GT_ACPU_ADDR(ppll_clk->addr));
		/* set en b1 */
		val = readl(SCPLL_EN_ACPU_ADDR(ppll_clk->addr));
		val |= BIT(SCPLL_EN);
		writel(val, SCPLL_EN_ACPU_ADDR(ppll_clk->addr));
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_open\n", __func__, ppll);
		break;
	}

	return 0;
}

static void ppll_nogate(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	u32 val;

	/* output clock not gate */
	switch (ppll) {
	case PPLL3:
		val = readl(PPLL3_GT_ACPU_ADDR(ppll_clk->endisable_addr));
		val |= BIT(PPLL3_GT_ACPU);
		writel(val, PPLL3_GT_ACPU_ADDR(ppll_clk->endisable_addr));
		break;
	case PPLL2:
		val = readl(PPLL2_GT_ACPU_ADDR(ppll_clk->endisable_addr));
		val |= BIT(PPLL2_GT_ACPU);
		writel(val, PPLL2_GT_ACPU_ADDR(ppll_clk->endisable_addr));
		break;
	case PPLL4:
		val = (unsigned int)readl(PPLL4_GT_ACPU_ADDR(ppll_clk->addr));
		val |= BIT(PPLL4_GT);
		writel(val, PPLL4_GT_ACPU_ADDR(ppll_clk->addr));
		break;
	case PPLL6:
		val = (unsigned int)readl(PPLL6_GT_ACPU_ADDR(ppll_clk->addr));
		val |= BIT(PPLL6_GT);
		writel(val, PPLL6_GT_ACPU_ADDR(ppll_clk->addr));
		break;
	case PPLL7:
		val = (unsigned int)readl(PPLL7_GT_ACPU_ADDR(ppll_clk->addr));
		val |= BIT(PPLL7_GT);
		writel(val, PPLL7_GT_ACPU_ADDR(ppll_clk->addr));
		break;
	case SCPLL:
		/* set bypass b0 */
		val = (unsigned int)readl(SCPLL_BP_ACPU_ADDR(ppll_clk->addr));
		val &= (~BIT(SCPLL_BP));
		writel(val, SCPLL_BP_ACPU_ADDR(ppll_clk->addr));
		/* set gt b1 */
		val = (unsigned int)readl(SCPLL_GT_ACPU_ADDR(ppll_clk->addr));
		val |= BIT(SCPLL_GT);
		writel(val, SCPLL_GT_ACPU_ADDR(ppll_clk->addr));
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_ready\n", __func__, ppll);
		break;
	}
}

static void ppll_check_lock(const void __iomem *addr, int ppll, int lock_bit)
{
	u32 timeout = 0;
	u32 val;

	do {
		val = readl(addr);
		val &= BIT((unsigned int)lock_bit);
		timeout++;
		if (timeout > AP_PPLL_STABLE_TIME) {
			pr_err("%s: ppll-%d enable is timeout\n", __func__, ppll);
			break;
		}
	} while (!val);
}

static int ppll_enable_ready(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	void __iomem *addr_temp = NULL;

	/* waiting lock */
	switch (ppll) {
	case PPLL2:
	case PPLL3:
		addr_temp = ppll_clk->addr + PPLLCTRL0((unsigned int)ppll);
		ppll_check_lock(addr_temp, ppll, PPLLCTRL0_LOCK);
		break;
	case PPLL6:
		ppll_check_lock(PPLL6CTRL0(ppll_clk->addr), ppll, PPLLCTRL0_LOCK);
		break;
	case PPLL4:
		ppll_check_lock(PPLL4CTRL0(ppll_clk->addr), ppll, PPLLCTRL0_LOCK);
		break;
	case PPLL7:
		ppll_check_lock(PPLL7CTRL0(ppll_clk->addr), ppll, PPLLCTRL0_LOCK);
		break;
	case SCPLL:
		ppll_check_lock(SCPLL_LOCK_STAT(ppll_clk->addr), ppll, SCPLL_LOCK_BIT);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_ready\n", __func__, ppll);
		break;
	}
	ppll_nogate(ppll_clk, ppll);

	return 0;
}

static void ppll_disable_set_gt_b1(struct hi3xxx_ppll_clk *ppll_clk)
{
	u32 val;

	val = (unsigned int)readl(SCPLL_GT_ACPU_ADDR(ppll_clk->addr));
	val |= BIT(SCPLL_GT);
	writel(val, SCPLL_GT_ACPU_ADDR(ppll_clk->addr));
}

static void ppll_disable_set_bypass_b1(struct hi3xxx_ppll_clk *ppll_clk)
{
	u32 val;

	val = (unsigned int)readl(SCPLL_BP_ACPU_ADDR(ppll_clk->addr));
	val |= BIT(SCPLL_BP);
	writel(val, SCPLL_BP_ACPU_ADDR(ppll_clk->addr));
}

static void ppll_disable_set_en_b0(struct hi3xxx_ppll_clk *ppll_clk)
{
	u32 val;

	val = (unsigned int)readl(SCPLL_EN_ACPU_ADDR(ppll_clk->addr));
	val &= (~BIT(SCPLL_EN));
	writel(val, SCPLL_EN_ACPU_ADDR(ppll_clk->addr));
}

static void ppll_disable_set_gt_b0(struct hi3xxx_ppll_clk *ppll_clk)
{
	u32 val;

	val = (unsigned int)readl(SCPLL_GT_ACPU_ADDR(ppll_clk->addr));
	val &= (~BIT(SCPLL_GT));
	writel(val, SCPLL_GT_ACPU_ADDR(ppll_clk->addr));
}

static void ppll_disable_set_en_vote(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	u32 val;

	switch (ppll) {
	case PPLL3:
		val = readl(PPLL3_DIS_ACPU_ADDR(ppll_clk->endisable_addr));
		val |= BIT(PPLL3_EN_ACPU);
		writel(val, PPLL3_DIS_ACPU_ADDR(ppll_clk->endisable_addr));
		break;
	case PPLL2:
		val = readl(PPLL2_DIS_ACPU_ADDR(ppll_clk->endisable_addr));
		val |= BIT(PPLL2_EN_ACPU);
		writel(val, PPLL2_DIS_ACPU_ADDR(ppll_clk->endisable_addr));
		break;
	case PPLL4:
		val = (unsigned int)readl(PPLL4_EN_ACPU_ADDR(ppll_clk->addr));
		val &= (~BIT(PPLL4_EN));
		writel(val, PPLL4_EN_ACPU_ADDR(ppll_clk->addr));
		break;
	case PPLL6:
		val = (unsigned int)readl(PPLL6_EN_ACPU_ADDR(ppll_clk->addr));
		val &= (~BIT(PPLL6_EN));
		writel(val, PPLL6_EN_ACPU_ADDR(ppll_clk->addr));
		break;
	case PPLL7:
		val = (unsigned int)readl(PPLL7_EN_ACPU_ADDR(ppll_clk->addr));
		val &= (~BIT(PPLL7_EN));
		writel(val, PPLL7_EN_ACPU_ADDR(ppll_clk->addr));
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is disable\n", __func__, ppll);
		break;
	}
}

static void ppll_disable_output_clock_gate(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	u32 val;

	switch (ppll) {
	case PPLL3:
		val = readl(PPLL3_DISGT_ACPU(ppll_clk->endisable_addr));
		val |= BIT(PPLL3_GT_ACPU);
		writel(val, PPLL3_DISGT_ACPU(ppll_clk->endisable_addr));
		break;
	case PPLL2:
		val = readl(PPLL2_DISGT_ACPU_ADDR(ppll_clk->endisable_addr));
		val |= BIT(PPLL2_GT_ACPU);
		writel(val, PPLL2_DISGT_ACPU_ADDR(ppll_clk->endisable_addr));
		break;
	case PPLL4:
		val = (unsigned int)readl(PPLL4_GT_ACPU_ADDR(ppll_clk->addr));
		val &= (~BIT(PPLL4_GT));
		writel(val, PPLL4_GT_ACPU_ADDR(ppll_clk->addr));
		break;
	case PPLL6:
		val = (unsigned int)readl(PPLL6_GT_ACPU_ADDR(ppll_clk->addr));
		val &= (~BIT(PPLL6_GT));
		writel(val, PPLL6_GT_ACPU_ADDR(ppll_clk->addr));
		break;
	case PPLL7:
		val = (unsigned int)readl(PPLL7_GT_ACPU_ADDR(ppll_clk->addr));
		val &= (~BIT(PPLL7_GT));
		writel(val, PPLL7_GT_ACPU_ADDR(ppll_clk->addr));
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is disable\n", __func__, ppll);
		break;
	}
}

static void ppll_disable(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	/* output clock gate */
	switch (ppll) {
	case PPLL3:
		/* output clock gate */
		ppll_disable_output_clock_gate(ppll_clk, ppll);
		/* ~en */
		ppll_disable_set_en_vote(ppll_clk, ppll);
		break;
	case PPLL2:
		/* output gate */
		ppll_disable_output_clock_gate(ppll_clk, ppll);
		/* ~en */
		ppll_disable_set_en_vote(ppll_clk, ppll);
		break;
	case PPLL4:
		/* output gate */
		ppll_disable_output_clock_gate(ppll_clk, ppll);
		/* ~en */
		ppll_disable_set_en_vote(ppll_clk, ppll);
		break;
	case PPLL6:
		/* output gate */
		ppll_disable_output_clock_gate(ppll_clk, ppll);
		/* ~en */
		ppll_disable_set_en_vote(ppll_clk, ppll);
		break;
	case PPLL7:
		/* output gate */
		ppll_disable_output_clock_gate(ppll_clk, ppll);
		/* ~en */
		ppll_disable_set_en_vote(ppll_clk, ppll);
		break;
	case SCPLL:
		/* set gt b0 */
		ppll_disable_set_gt_b0(ppll_clk);
		/* set bypass b1 */
		ppll_disable_set_bypass_b1(ppll_clk);
		/* set en b0 */
		ppll_disable_set_en_b0(ppll_clk);
		/* set gt b1 */
		ppll_disable_set_gt_b1(ppll_clk);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is disable\n", __func__, ppll);
		break;
	}
}

static int hi3xxx_multicore_ppll_enable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;

	ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);
	/* for debug */
	ppll_clk->ref_cnt++;
	if (ppll_clk->en_cmd[1] == PPLL0)
		return 0;
	if (ppll_clk->ref_cnt == 1) {
		ppll_enable_open(ppll_clk, ppll_clk->en_cmd[1]);
		ppll_enable_ready(ppll_clk, ppll_clk->en_cmd[1]);
	}
	return 0;
}

static void hi3xxx_multicore_ppll_disable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;

	ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);
	/* for debug */
	ppll_clk->ref_cnt--;
	if (ppll_clk->dis_cmd[1] == PPLL0)
		return;
#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	if (!ppll_clk->ref_cnt)
		ppll_disable(ppll_clk, ppll_clk->dis_cmd[1]);
#endif
}

static const struct clk_ops hi3xxx_ppll_ops = {
	.enable = hi3xxx_multicore_ppll_enable,
	.disable = hi3xxx_multicore_ppll_disable,
};

static void __iomem *__hi3xxx_ppll_reg_base(struct device_node *np)
{
	void __iomem *reg_base = NULL;

	reg_base = hs_clk_get_base(np);
	if (reg_base == NULL)
		pr_err("[%s] fail to get reg_base!\n", __func__);

	return reg_base;
}

static void __init hi3xxx_ppll_setup(struct device_node *np)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;
	struct clk_init_data *init = NULL;
	const char *clk_name = NULL;
	const char *parent_names = NULL;
	void __iomem *reg_base = NULL;
	u32 crg_type;
	int ret;

	reg_base = __hi3xxx_ppll_reg_base(np);
	if (reg_base == NULL)
		return;
	clk_name = __kirin_ppll_clock_name_read(np);
	if (clk_name == NULL)
		return;
	parent_names = of_clk_get_parent_name(np, 0);

	ppll_clk = kzalloc(sizeof(*ppll_clk), GFP_KERNEL);
	if (ppll_clk == NULL) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		goto err_ppll_clk;
	}
	/* initialize the reference count */
	ppll_clk->ref_cnt = 0;

	init = hisi_clk_init_data_alloc(clk_name);
	if (init == NULL)
		goto err_init;
	init->ops = &hi3xxx_ppll_ops;
	init->flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
	init->parent_names = &parent_names;
	init->num_parents = 1;

	ret = __kirin_lpm3_cmd_setup(np, ppll_clk);
	if (ret)
		goto err_clk;

	ppll_clk->lock = &hs_clk.lock;
	ppll_clk->hw.init = init;
	ppll_clk->addr = reg_base;
	ppll_clk->sctrl = NULL;
	crg_type = hisi_clk_crg_type_get(np, HS_CRGCTRL);
	ppll_clk->endisable_addr = hs_clk_base(crg_type);
	ppll_clk->flags = 0;

	ret = __kirin_ppll_clk_register(np, ppll_clk, clk_name);
	if (ret)
		goto err_iomap;

	return;
err_iomap:
	iounmap(ppll_clk->endisable_addr);
err_clk:
	kfree(init->name);
	kfree(init);
err_init:
	kfree(ppll_clk);
err_ppll_clk:
	return;
}

CLK_OF_DECLARE(kirin_ppll, "hisilicon,kirin-ppll-ctrl", kirin_ppll_setup);
CLK_OF_DECLARE(hi3xxx_ppll, "hisilicon,ppll-ctrl", hi3xxx_ppll_setup);
