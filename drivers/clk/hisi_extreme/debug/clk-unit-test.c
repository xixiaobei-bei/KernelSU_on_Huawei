/*
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifdef CONFIG_HISI_CLK_STUB
#include "clk-unit-test.h"

#include <linux/clk-provider.h>
#include <linux/clk/clk-conf.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/hwspinlock.h>
#include <linux/of_address.h>
#include <linux/clk.h>

#include <soc_pmctrl_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_crgperiph_interface.h>
#include <soc_media1_crg_interface.h>
#include <soc_media2_crg_interface.h>
#include <soc_crgperiph_interface.h>
#include <soc_iomcu_interface.h>
#include <soc_pctrl_interface.h>
#include <pmic_interface.h>

#include "clk.h"

#define WIDTH_TO_MASK(width)			((1 << (width)) - 1)

#define CLK_GATE_STATUS_OFFSET			0x8


#define CLK_ON					1
#define CLK_OFF					0

static DEFINE_MUTEX(clock_list_lock);
#define to_clk_gate(_hw) container_of(_hw, struct clk_gate, hw)
#define to_clk_mux(_hw) container_of(_hw, struct clk_mux, hw)

static int clock_enable_emulator(const char *name)
{
	struct clk_core *clk = NULL;
	char *clk_name = NULL;
	int err = 0;
	int ret;

	mutex_lock(&clock_list_lock);

	clk_name = name;

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, prepare and enable it.
	 */
	list_for_each_entry(clk, &clocks, node) {
		if (clk == NULL) {
			err = -EINVAL;
			goto out;
		}
		if (strcmp(clk->name, clk_name) == 0) {
			pr_err("[old]:enable_refcnt = %u\n", clk->enable_count);

			ret = clk_prepare_enable(clk->hw->clk);
			if (ret) {
				err = -EINVAL;
				goto out;
			}
			pr_err("[new]:enable_refcnt = %u\n", clk->enable_count);
			goto out;
		}
	}

	pr_err("clk name error!\n");

out:
	mutex_unlock(&clock_list_lock);
	return err;
}

static int clock_disable_emulator(const char *name)
{
	struct clk_core *clk = NULL;
	char *clk_name = NULL;
	int err = 0;

	mutex_lock(&clock_list_lock);

	/* copy clock name from user space */
	clk_name = name;
	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, disable and unprepare it.
	 */
	list_for_each_entry(clk, &clocks, node) {
		if (clk == NULL) {
			err = -EINVAL;
			goto out;
		}
		if (strcmp(clk->name, clk_name) == 0) {
			pr_err("[old]:enable_refcnt = %u\n", clk->enable_count);

			clk_disable_unprepare(clk->hw->clk);

			pr_err("[new]:enable_refcnt = %u\n", clk->enable_count);
			goto out;
		}
	}
	pr_err("clk name error!\n");

out:
	mutex_unlock(&clock_list_lock);
	return err;
}

static int clock_setrate_emulator(const char *name, unsigned long setrate)
{
	struct clk_core *clk = NULL;
	char *clk_name = NULL;
	unsigned long rate;
	int err = 0;
	int ret;

	clk_name = name;
	mutex_lock(&clock_list_lock);

	rate = setrate;

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, set rate of this clock.
	 */
	list_for_each_entry(clk, &clocks, node) {
		if (clk == NULL)
			goto out;
		if (strcmp(clk->name, clk_name) == 0) {
			pr_err("\n\n");
			pr_err("[%s]: old rate = %lu , target rate = %lu\n",
				clk->name, clk->rate, rate);
			ret = clk_set_rate(clk->hw->clk, rate);
			rate = clk_get_rate(clk->hw->clk);
			pr_err("ret = %d, rate = %lu\n", ret, rate);
			if (ret)
				err = -EINVAL;
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error */
	pr_err("clk name error!\n\n");

out:
	mutex_unlock(&clock_list_lock);
	return err;
}

static unsigned int get_table_div(const struct clk_div_table *table,
	unsigned int val)
{
	const struct clk_div_table *clkt = NULL;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->val == val)
			return clkt->div;

	return 0;
}

static bool clkgate_check(struct hi3xxx_periclk *pclk, u32 val, u32 rdata0, u32 offset, u32 flag)
{
	bool err = false;

	if (flag == 1) {
		if (pclk->enable && val && (rdata0 == offset))
			err = true;
	} else {
		if (!val && (rdata0 == offset))
			err = true;
	}

	return err;
}

static void clkgate_offsetbit_check(const char *name, u32 clk_offset,
	unsigned int clk_bit, u32 flag)
{
	struct clk_core *clk = NULL;
	struct hi3xxx_periclk *pclk = NULL;
	struct device_node *np = NULL;
	char *clk_name = NULL;
	bool err = false;
	u32 val;
	/* offset and efficient bits */
	u32 rdata[2] = {0};

	clk_name = name;
	mutex_lock(&clock_list_lock);

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, set rate of this clock.
	 */
	list_for_each_entry(clk, &clocks, node) {
		if (clk == NULL)
			goto out;
		if (strcmp(clk->name, clk_name) == 0) {
			pclk = container_of(clk->hw, struct hi3xxx_periclk, hw);
			val = readl(pclk->enable + CLK_GATE_STATUS_OFFSET);
			val &= BIT(clk_bit);
			np = of_find_node_by_name(NULL, clk_name);
			if (np == NULL) {
				pr_err("[%s] fail to find np!\n", __func__);
				goto out;
			}
			if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clkgate",
				&rdata[0], 2)) {
				pr_err("[%s] node doesn't have clkgate-reg property!\n",
					__func__);
				goto out;
			}
			err = clkgate_check(pclk, val, rdata[0], clk_offset, flag);
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error */
	pr_err("clk name error!\n\n");

out:
	mutex_unlock(&clock_list_lock);
	if (!err)
		pr_err("Fail: [%s] fail to gate!\n", name);
	else
		pr_err("Succ: [%s] succ to gate!\n", name);
}

static void clkmux_offsetbit_check(const char *name, u32 clk_offset, u32 shift, u32 clk_bit)
{
	struct clk_core *clk = NULL;
	struct clk_mux *mux = NULL;
	char *clk_name = NULL;
	struct device_node *np = NULL;
	bool err = false;
	u32 val;
	/* offset and efficient bits */
	u32 rdata[2] = {0};

	clk_name = name;
	mutex_lock(&clock_list_lock);

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, set rate of this clock.
	 */
	list_for_each_entry(clk, &clocks, node) {
		if (clk == NULL)
			goto out;

		if (strcmp(clk->name, clk_name) == 0) {
			mux = to_clk_mux(clk->hw);
			val = readl(mux->reg) >> ((mux->shift));
			val &= mux->mask;
			val ^= clk_bit;
			np = of_find_node_by_name(NULL, clk_name);
			if (np == NULL) {
				pr_err("[%s] fail to find np!\n", __func__);
				goto out;
			}
			if (of_property_read_u32_array(np, "hisilicon,clkmux-reg",
				&rdata[0], 2)) {
				pr_err("[%s] node doesn't have clkmux-reg property!\n",
					__func__);
				goto out;
			}
			if (!val && ((clk_offset == rdata[0])) && mux->shift == shift)
				err = true;
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error */
	pr_err("clk name error!\n\n");

out:
	mutex_unlock(&clock_list_lock);
	if (!err)
		pr_err("Fail: [%s] fail to mux!\n", name);
	else
		pr_err("Succ: [%s] succ to mux!\n", name);
}

static void clkdiv_offsetbit_check(const char *name, u32 clk_offset,
	unsigned int div, u32 div_start, u32 div_end)
{
	struct clk_core *clk = NULL;
	struct hi3xxx_divclk *dclk = NULL;
	struct device_node *np = NULL;
	char *clk_name = NULL;
	bool err = false;
	u32 val, rdiv, width;
	/* offset and efficient bits */
	u32 rdata[2] = {0};

	clk_name = name;
	mutex_lock(&clock_list_lock);

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, set rate of this clock.
	 * if clk wasn't in the clocks list, clock name is error
	 */
	list_for_each_entry(clk, &clocks, node) {
		if (clk == NULL)
			goto out;
		if (strcmp(clk->name, clk_name) == 0) {
			dclk = container_of(clk->hw, struct hi3xxx_divclk, hw);
			width = div_end - div_start + 1;
			val = readl(dclk->reg) >> dclk->shift;
			val &= WIDTH_TO_MASK(dclk->width);
			rdiv = get_table_div(dclk->table, val);
			np = of_find_node_by_name(NULL, clk_name);
			if (np == NULL) {
				pr_err("[%s] fail to find np!\n", __func__);
				goto out;
			}

			if (of_property_read_u32_array(np, "hisilicon,clkdiv",
				&rdata[0], 2)) {
				pr_err("[%s] node doesn't have clkdiv-reg property!\n",
					__func__);
				goto out;
			}

			if (rdiv == div && dclk->shift == div_start &&
			dclk->width == width && (clk_offset == rdata[0]))
				err = true;
			goto out;
		}
	}

out:
	mutex_unlock(&clock_list_lock);
	if (!err)
		pr_err("Fail: [%s] fail to div!\n", name);
	else
		pr_err("Succ: [%s] succ to div!\n", name);
}

static bool clkmaskgate_check(u32 val, u32 rdata0, u32 offset, u32 flag)
{
	bool err = false;

	if (flag == 1) {
		if (val && (rdata0 == offset))
			err = true;
	} else {
		if (!val && (rdata0 == offset))
			err = true;
	}

	return err;
}

static void clkmaskgate_offsetbit_check(const char *name, unsigned int clk_offset,
	unsigned int clk_bit, u32 flag)
{
	struct clk_core *clk = NULL;
	struct clk_gate *gate = NULL;
	struct device_node *np = NULL;
	char *clk_name = NULL;
	bool err = false;
	u32 val;
	/* offset and efficient bits */
	u32 rdata[2] = {0};

	clk_name = name;
	mutex_lock(&clock_list_lock);

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, set rate of this clock.
	 */
	list_for_each_entry(clk, &clocks, node) {
		if (clk == NULL)
			goto out;

		if (strcmp(clk->name, clk_name) == 0) {
			gate = to_clk_gate(clk->hw);
			val = readl(gate->reg);
			val &= BIT(clk_bit);

			np = of_find_node_by_name(NULL, clk_name);
			if (np == NULL) {
				pr_err("[%s] fail to find np!\n", __func__);
				goto out;
			}
			if (of_property_read_u32_array(np, "hisilicon,clkgate",
				&rdata[0], 2)) {
				pr_err("[%s] node doesn't have clkgate property!\n",
					__func__);
				goto out;
			}
			err = clkmaskgate_check(val, rdata[0], clk_offset, flag);
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error */
	pr_err("clk name error!\n\n");

out:
	mutex_unlock(&clock_list_lock);
	if (!err)
		pr_err("Fail: [%s] fail to andgate!\n", name);
	else
		pr_err("Succ: [%s] succ to andgate!\n", name);
}

int clk_test_check(void)
{
	pr_err("********************************\n");
	pr_err("\n\n\n");
	pr_err("START CLOCK UNIT TEST\n");
	pr_err("\n\n\n");
	pr_err("********************************\n");

	const char *name = "pclk_dsi0";
	int ret;

	pr_err("\n");
	ret = clock_enable_emulator(name);
	if (ret)
		return -EINVAL;
	/* 0x50:clk offset */
	clkgate_offsetbit_check("pclk_dsi0", 0x50,
		SOC_CRGPERIPH_PEREN5_gt_pclk_dsi0_START, CLK_ON);
	/* set rate 232M */
	ret = clock_setrate_emulator(name, 232000000);
	if (ret)
		pr_err("[%s] fail to setrate!\n", name);
	/* 0xEC:clk offset 1:div 0:div start 1:div end */
	clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, 0, 1);

	/* set rate 580M */
	ret = clock_setrate_emulator(name, 58000000);
	if (ret)
		pr_err("[%s] fail to setrate!\n", name);
	/* 0xEC:clk offset 4:div 0:div start 1:div end */
	clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, 0, 1);

	ret = clock_disable_emulator(name);
	if (ret)
		return -EINVAL;
	/* 0x50:clk offset */
	clkgate_offsetbit_check("pclk_dsi0", 0x50,
		SOC_CRGPERIPH_PEREN5_gt_pclk_dsi0_START, CLK_OFF);

	return 0;
}
#endif
