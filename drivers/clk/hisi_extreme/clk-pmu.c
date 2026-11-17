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
#include "clk-pmu.h"

static struct hwspinlock *clk_hwlock_9;

static int hi3xxx_multicore_clkgate_prepare(struct clk_hw *hw)
{
	return 0;
}

static int hi3xxx_multicore_clkgate_enable(struct clk_hw *hw)
{
	struct hi3xxx_clkpmu *pclk = container_of(hw, struct hi3xxx_clkpmu, hw);
	unsigned int val;
	unsigned long flags = 0;

	if (pclk->lock)
		spin_lock_irqsave(pclk->lock, flags);

	val = (*pclk->clk_pmic_read)(pclk->pmu_clk_enable);
	val |= pclk->ebits;
	(*pclk->clk_pmic_write)(pclk->pmu_clk_enable, val);

	if (pclk->lock)
		spin_unlock_irqrestore(pclk->lock, flags);
	return 0;
}

static void hi3xxx_multicore_clkgate_disable(struct clk_hw *hw)
{
#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	struct hi3xxx_clkpmu *pclk = container_of(hw, struct hi3xxx_clkpmu, hw);
	unsigned int val;
	unsigned long flags = 0;

	if (pclk->lock)
		spin_lock_irqsave(pclk->lock, flags);

	if (!pclk->always_on) {
		val = (*pclk->clk_pmic_read)(pclk->pmu_clk_enable);
		val &= (~pclk->ebits);
		(*pclk->clk_pmic_write)(pclk->pmu_clk_enable, val);
	}

	if (pclk->lock)
		spin_unlock_irqrestore(pclk->lock, flags);

	return;
#endif
}

static void hi3xxx_multicore_clkgate_unprepare(struct clk_hw *hw)
{
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hi3xxx_dump_pmuclk(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	struct hi3xxx_clkpmu *pclk = container_of(hw, struct hi3xxx_clkpmu, hw);

	if ((buf == NULL) && (s != NULL))
		seq_printf(s, "    %-15s    %-15s    0x%03X", "PMUCTRL",
			"pmu-clk", pclk->pmu_clk_enable);
	return 0;
}
#endif

static const struct clk_ops hi3xxx_pmu_clkgate_ops = {
	.prepare = hi3xxx_multicore_clkgate_prepare,
	.unprepare = hi3xxx_multicore_clkgate_unprepare,
	.enable = hi3xxx_multicore_clkgate_enable,
	.disable = hi3xxx_multicore_clkgate_disable,
#ifdef CONFIG_HISI_CLK_DEBUG
	.dump_reg = hi3xxx_dump_pmuclk,
#endif
};

static void __hi3xxx_abb_clk_hwunlock(struct hi3xxx_clkpmu *pclk)
{
	hwspin_unlock(pclk->clk_hwlock);
}

static int __hi3xxx_abb_clk_hwlock(struct hi3xxx_clkpmu *pclk)
{
	if (clk_hwlock_9 == NULL) {
		clk_hwlock_9 = hwspin_lock_request_specific(pclk->flags);
		if (clk_hwlock_9 == NULL) {
			pr_err("abb clk request hwspin lock[9] failed !\n");
			return -ENODEV;
		}
	}
	pclk->clk_hwlock = clk_hwlock_9;
	if (hwspin_lock_timeout(pclk->clk_hwlock, CLK_HWLOCK_TIMEOUT)) {
		pr_err("abb clk enable hwspinlock timout!\n");
		return -ENOENT;
	}
	return 0;
}

static int hi3xxx_multicore_abb_clkgate_prepare(struct clk_hw *hw)
{
	struct hi3xxx_clkpmu *pclk = container_of(hw, struct hi3xxx_clkpmu, hw);
	u32 val;
	int ret;

	ret = __hi3xxx_abb_clk_hwlock(pclk);
	if (ret)
		return ret;
	val = readl(pclk->sctrl);
	if ((val & BIT(AP_ABB_EN)) == 0) {
		if ((val & BIT(LPM3_ABB_EN)) == 0) {
			/* open abb clk */
			val = (*pclk->clk_pmic_read)(pclk->pmu_clk_enable);
			val |= pclk->ebits;
			(*pclk->clk_pmic_write)(pclk->pmu_clk_enable, val);
		}
		/* write 0x43C register */
		val = readl(pclk->sctrl);
		val |= BIT(AP_ABB_EN);
		writel(val, pclk->sctrl);
	}
	__hi3xxx_abb_clk_hwunlock(pclk);

	mdelay(1);
	return 0;
}

static void hi3xxx_multicore_abb_clkgate_unprepare(struct clk_hw *hw)
{
#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	struct hi3xxx_clkpmu *pclk = container_of(hw, struct hi3xxx_clkpmu, hw);
	u32 val;

	if (hwspin_lock_timeout(pclk->clk_hwlock, CLK_HWLOCK_TIMEOUT)) {
		pr_err("abb clk disable hwspinlock timout!\n");
		return;
	}

	val = readl(pclk->sctrl);
	if ((val & BIT(AP_ABB_EN)) == 1) {
		if ((val & BIT(LPM3_ABB_EN)) == 0) {
			if (!pclk->always_on) {
				/* close abb clk */
				val = (*pclk->clk_pmic_read)(pclk->pmu_clk_enable);
				val &= (~pclk->ebits);
				(*pclk->clk_pmic_write)(pclk->pmu_clk_enable, val);
			}
		}
		/* write 0x43C register */
		val = readl(pclk->sctrl);
		val &= (~BIT(AP_ABB_EN));
		writel(val, pclk->sctrl);
	}
	hwspin_unlock(pclk->clk_hwlock);
#endif
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hi3xxx_dump_abbclk(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	struct hi3xxx_clkpmu *pclk = container_of(hw, struct hi3xxx_clkpmu, hw);
	u32 val;
	int s_ret, ret;

	ret = __hi3xxx_abb_clk_hwlock(pclk);
	if (ret)
		return ret;
	if ((buf != NULL) && (s == NULL) && (buf_length > 0)) {
		val = (*pclk->clk_pmic_read)(pclk->pmu_clk_enable);
		s_ret = snprintf_s(buf, buf_length, buf_length - 1,
			"[%s] : regAddress = 0x%x, regval = 0x%x\n",
			__clk_get_name(hw->clk), pclk->pmu_clk_enable, val);
		if (s_ret == -1)
			pr_err("%s snprintf_s failed!\n", __func__);
	}
	if ((buf == NULL) && (s != NULL))
		seq_printf(s, "    %-15s    %-15s    0x%03X", "PMUCTRL",
			"abb-clk", pclk->pmu_clk_enable);

	__hi3xxx_abb_clk_hwunlock(pclk);

	return 0;
}
#endif

static const struct clk_ops hi3xxx_abb_clkgate_ops = {
	.prepare = hi3xxx_multicore_abb_clkgate_prepare,
	.unprepare = hi3xxx_multicore_abb_clkgate_unprepare,
#ifdef CONFIG_HISI_CLK_DEBUG
	.dump_reg = hi3xxx_dump_abbclk,
#endif
};

static int __hi3xxx_pmu_clkgate_pclk_init(const struct pmu_clock *clkpmu,
	struct hi3xxx_clkpmu *pclk)
{
	struct device_node *np = NULL;
	char mmw_name[MAX_CMP_LEN] = "hisilicon,mmw_";
	u32 clkgata_val[CLKGATE_COUNT] = {0};
	int ret;

	ret = strncat_s(mmw_name, MAX_CMP_LEN, clkpmu->alias, strlen(clkpmu->alias));
	if (ret) {
		pr_err("[%s]mmw string strncat_s fail!\n", __func__);
		return ret;
	}
	np = of_find_compatible_node(NULL, NULL, mmw_name);
	if (np) {
		if (of_property_read_u32_array(np, "hisilicon,clkgate", &clkgata_val[0], CLKGATE_COUNT)) {
			pr_err("[%s] %s node doesn't have clkgate property!\n",
				__func__, np->name);
			return -ENODEV;
		}
		pclk->pmu_clk_enable = clkgata_val[0];
		pclk->ebits = BIT(clkgata_val[1]);
		pclk->clk_pmic_read = mmw_pmic_reg_read;
		pclk->clk_pmic_write = mmw_pmic_reg_write;
	} else {
		pclk->pmu_clk_enable = clkpmu->offset;
		pclk->ebits = BIT(clkpmu->bit_mask);
		pclk->clk_pmic_read = pmic_read_reg;
		pclk->clk_pmic_write = pmic_write_reg;
	}

	return 0;
}

static struct clk *__clk_register_clkpmu(const struct pmu_clock *clkpmu,
	struct clock_data *data)
{
	struct clk *clk = NULL;
	struct hi3xxx_clkpmu *pclk = NULL;
	struct clk_init_data init;
	int ret;
	struct hs_clk *hs_clk = get_hs_clk_info();

	pclk = kzalloc(sizeof(*pclk), GFP_KERNEL);
	if (pclk == NULL)
		return clk;

	ret = __hi3xxx_pmu_clkgate_pclk_init(clkpmu, pclk);
	if (ret)
		goto err_init;

	init.name = clkpmu->name;

	if (!strcmp(clkpmu->name, "clk_abb_192")) {
		init.ops = &hi3xxx_abb_clkgate_ops;
	} else {
		init.ops = &hi3xxx_pmu_clkgate_ops;
	}
	init.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
	init.parent_names = &(clkpmu->parent_name);
	init.num_parents = 1;

	pclk->always_on = clkpmu->always_on;
	pclk->sctrl = ABB_SCBAKDATA(data->base); /* only for abb clk */
	pclk->lock = &hs_clk->lock;
	pclk->hw.init = &init;
	pclk->flags = clkpmu->hwlock_id;
	pclk->clk_hwlock = NULL;

	clk = clk_register(NULL, &pclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, clkpmu->name);
		goto err_init;
	}
	/* init is local variable, need set NULL before func */
	pclk->hw.init = NULL;
	return clk;

err_init:
	kfree(pclk);
	return clk;
}

void plat_clk_register_clkpmu(const struct pmu_clock *clks,
	int nums, struct clock_data *data)
{
	struct clk *clk = NULL;
	int i;

	for (i = 0; i < nums; i++) {
		clk = __clk_register_clkpmu(&clks[i], data);
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}

#ifdef CONFIG_HISI_CLK_DEBUG
		debug_clk_add(clk, CLOCK_PMU);
#endif

		clk_log_dbg("clks id %d, nums %d, clks name = %s!\n",
			clks[i].id, nums, clks[i].name);

		clk_data_init(clk, clks[i].alias, clks[i].id, data);
	}
}

