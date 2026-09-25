/*
 * hisi-clkgate.c
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
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/clkdev.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <securec.h>

#include "dvfs/peri_volt_poll.h"
#include "clk-kirin-common.h"

#ifdef CONFIG_HISI_CLK_DEBUG
#include "debug/hisi-clk-debug.h"
#endif

#ifndef CONFIG_HISI_CLK_ALWAYS_ON

#define HISI_CLK_GATE_DISABLE_OFFSET		0x4
#endif

#define HISI_CLK_GATE_STATUS_OFFSET		0x8

#define CLOCK_GATE_SYNC_MAX			10 /* us */

#ifdef CONFIG_HISI_PERIDVFS
#if defined(CONFIG_HISI_HW_PERI_DVS)
static int peridvs_channel_valid_set(struct hi3xxx_periclk * pclk, bool flag)
{
	struct peri_volt_poll *pvp = NULL;
	int ret;

	if (!pclk)
		return -EINVAL;

	pvp = peri_volt_poll_get(pclk->perivolt_poll_id, NULL);
	if (!pvp) /* if no pvp found, just return ok */
		return 0;

	ret = peri_set_avs(pvp, flag);
	if (ret < 0) {
		pr_err("[%s] pvp dev_id %u valid set failed!\n", __func__,
			pclk->perivolt_poll_id);
		return ret;
	}
	return 0;
}
#endif

static int peri_dvfs_set_volt(u32 poll_id, u32 volt_level)
{
	struct peri_volt_poll *pvp = NULL;
	unsigned int volt;
	int loop = PERI_AVS_LOOP_MAX;
	int ret;

	pvp = peri_volt_poll_get(poll_id, NULL);
	if (pvp == NULL) {
		pr_err("pvp get failed!\n");
		return -EINVAL;
	}
	ret = peri_set_volt(pvp, volt_level);
	if (ret < 0) {
		pr_err("[%s]set volt failed ret = %d!\n", __func__, ret);
		return ret;
	}
	volt = peri_get_volt(pvp);
	if (volt > DVFS_MAX_VOLT) {
		pr_err("[%s]get volt illegal volt = %u!\n", __func__, volt);
		return -EINVAL;
	}
	if (volt_level > volt) {
		do {
			volt = peri_get_volt(pvp);
			if (volt > DVFS_MAX_VOLT) {
				pr_err("[%s]get volt illegal volt = %u!\n", __func__, volt);
				return -EINVAL;
			}
			if (volt < volt_level) {
				loop--;
				/* AVS complete timeout is about 150us * 400 ~ 300us * 400 */
				usleep_range(150, 300);
			}
		} while (volt < volt_level && loop > 0);
		if (volt < volt_level) {
			pr_err("[%s] fail to updata volt, ret = %u!\n",
				__func__, volt);
			/* after peri avs ok,then open behind */
			return -EINVAL;
		}
	}
	return ret;
}

static int __hisi_peri_dvfs_set_volt(struct hi3xxx_periclk *pclk, unsigned int level)
{
	int ret = 0;
	int i;
	unsigned long cur_rate;

	cur_rate = clk_get_rate(pclk->hw.clk);
	if (!cur_rate)
		pr_err("[%s]soft rate:[%s] must not be 0,please check!\n",
			__func__, pclk->hw.init->name);

	for (i = 0; i < level; i++) {
		/* 1000: freq conversion */
		if (cur_rate > pclk->freq_table[i] * 1000)
			continue;
		ret = peri_dvfs_set_volt(pclk->perivolt_poll_id, pclk->volt_table[i]);
		if (ret < 0)
			pr_err("[%s]pvp set volt failed ret = %d!\n", __func__, ret);
		return ret;
	}
	if (i == level) {
		ret = peri_dvfs_set_volt(pclk->perivolt_poll_id, pclk->volt_table[i]);
		if (ret < 0)
			pr_err("[%s]pvp set volt failed ret = %d!\n", __func__, ret);
	}
	return ret;
}

static int hisi_peri_dvfs_prepare(struct hi3xxx_periclk *pclk)
{
	int ret = 0;
	unsigned int level = pclk->sensitive_level;

	if (!pclk->peri_dvfs_sensitive)
		return 0;
	if (pclk->freq_table[0] > 0) {
		ret = __hisi_peri_dvfs_set_volt(pclk, level);
	} else if (pclk->freq_table[0] == 0) {
		ret = peri_dvfs_set_volt(pclk->perivolt_poll_id, pclk->volt_table[level]);
		if (ret < 0)
			pr_err("[%s]pvp up volt failed ret = %d!\n", __func__, ret);
	} else {
		pr_err("[%s]soft level:[%s] freq must not be less than 0,please check!\n",
			__func__, pclk->hw.init->name);
		return -EINVAL;
	}

	return ret;
}

static void hisi_peri_dvfs_unprepare(struct hi3xxx_periclk *pclk)
{
	int ret;

	if (!pclk->peri_dvfs_sensitive)
		return;
	ret = peri_dvfs_set_volt(pclk->perivolt_poll_id, PERI_VOLT_0);
	if (ret < 0)
		pr_err("[%s]peri dvfs set volt failed ret = %d!\n", __func__, ret);
}
#endif


static int hi3xxx_clkgate_prepare(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
	struct clk *friend_clk = NULL;
	int ret = 0;

	/* if friend clk exist,enable it */
	if (pclk->friend != NULL) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk)) {
			pr_err("[%s]%s get failed!\n", __func__, pclk->friend);
			return -EINVAL;
		}
		ret = clk_core_prepare(friend_clk->core);
		if (ret) {
			pr_err("[%s], friend clock prepare faild!", __func__);
			return ret;
		}
	}
#ifdef CONFIG_HISI_PERIDVFS
#if defined(CONFIG_HISI_HW_PERI_DVS)
	ret = peridvs_channel_valid_set(pclk, AVS_ENABLE_PLL);
	if (ret < 0)
		pr_err("[%s] set peridvs channel failed ret=%d!\n", __func__, ret);
#endif
	ret = hisi_peri_dvfs_prepare(pclk);
	if (ret < 0)
		pr_err("[%s]set volt failed ret = %d tar = %s!\n", __func__, ret, hw->init->name);
#endif

	return ret;
}

static int hi3xxx_clkgate_enable(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
	struct clk *friend_clk = NULL;
	int ret;

	/* gate sync */
	if (pclk->sync_time > 0)
		udelay(pclk->sync_time);

	/* enable clock */
	if (pclk->enable != NULL)
		writel(pclk->ebits, pclk->enable);

	/* if friend clk exist,enable it */
	if (pclk->friend != NULL) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk)) {
			pr_err("[%s]%s get failed!\n", __func__, pclk->friend);
			return -EINVAL;
		}
		ret = __clk_enable(friend_clk);
		if (ret) {
			pr_err("[%s], friend clock:%s enable faild!", __func__, pclk->friend);
			return ret;
		}
	}

	if (pclk->sync_time > 0)
		udelay(pclk->sync_time);

	return 0;
}

static void hi3xxx_clkgate_disable(struct clk_hw *hw)
{
#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
	struct clk *friend_clk = NULL;

	if (pclk->enable != NULL) {
		if (!pclk->always_on)
			writel(pclk->ebits, pclk->enable + HISI_CLK_GATE_DISABLE_OFFSET);
	}
	if (pclk->sync_time > 0)
		udelay(pclk->sync_time);
	/* if friend clk exist, disable it . */
	if (pclk->friend != NULL) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk))
			pr_err("[%s]%s get failed!\n", __func__, pclk->friend);
		__clk_disable(friend_clk);
	}
#endif
}

static void hi3xxx_clkgate_unprepare(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
	struct clk *friend_clk = NULL;

#ifdef CONFIG_HISI_PERIDVFS
#if defined(CONFIG_HISI_HW_PERI_DVS)
	if (peridvs_channel_valid_set(pclk, AVS_DISABLE_PLL) < 0)
		pr_err("[%s] disable peridvs channel failed!\n", __func__);
#endif /* CONFIG_HISI_HW_PERI_DVS */
	hisi_peri_dvfs_unprepare(pclk);
#endif /* CONFIG_HISI_PERIDVFS */

#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	if (pclk->friend != NULL) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk)) {
			pr_err("%s get failed!\n", pclk->friend);
			return;
		}
		clk_core_unprepare(friend_clk->core);
	}

#endif
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hi3xxx_clkgate_is_enabled(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
	u32 reg;

	if (pclk->enable == NULL)
		return 2;
	reg = readl(pclk->enable + HISI_CLK_GATE_STATUS_OFFSET);
	reg &= pclk->ebits;

	/* 0:disable 1:enable 2:other  */
	return reg ? 1 : 0;
}

static void __iomem *hi3xxx_clkgate_get_reg(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
	void __iomem *ret = NULL;
	u32 val;

	if (pclk->enable != NULL) {
		ret = pclk->enable + HISI_CLK_GATE_STATUS_OFFSET;
		val = readl(ret);
		val &= pclk->ebits;
		pr_info("\n[%s]: reg = 0x%pK, bits = 0x%x, regval = 0x%x\n",
			__clk_get_name(hw->clk), ret, pclk->ebits, val);
	}

	return ret;
}

static int hi3xxx_dumpgate(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
	void __iomem *ret = NULL;
	unsigned long int clk_base_addr = 0;
	unsigned int clk_bit;
	u32 index = 0;
	u32 val;
	int s_ret;

	if ((pclk->enable != NULL) && (buf != NULL) && (s == NULL) && (buf_length > 0)) {
		ret = pclk->enable + HISI_CLK_GATE_STATUS_OFFSET;
		val = readl(ret);
		s_ret = snprintf_s(buf, buf_length, buf_length - 1,
			"[%s] : regAddress = 0x%pK, regval = 0x%x\n",
			__clk_get_name(hw->clk), ret, val);
		if (s_ret == -1)
			pr_err("%s snprintf_s failed!\n", __func__);
	}

	if ((pclk->enable != NULL) && (buf == NULL) && (s != NULL)) {
		clk_base_addr = (uintptr_t)pclk->enable & CLK_ADDR_HIGH_MASK;
		clk_bit = (uintptr_t)pclk->enable & CLK_ADDR_LOW_MASK;
		val = pclk->ebits;
		while (val) {
			val = val >> 1;
			if (val)
				index++;
		}
		seq_printf(s, "    %-15s    %-15s    0x%03X    bit-%-2u",
			hs_base_addr_transfer(clk_base_addr), "gate", clk_bit, index);
	}

	if ((pclk->enable == NULL) && (buf == NULL) && (s != NULL))
		seq_printf(s, "    %-15s    %-15s", "NONE", "fixed-gate");

	return 0;
}
#endif

#ifdef CONFIG_HISI_CLK
static int clk_gate_get_source(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);

	return pclk->sensitive_pll;
}
#endif

static const struct clk_ops hi3xxx_clkgate_ops = {
	.prepare        = hi3xxx_clkgate_prepare,
	.unprepare      = hi3xxx_clkgate_unprepare,
	.enable         = hi3xxx_clkgate_enable,
	.disable        = hi3xxx_clkgate_disable,
#ifdef CONFIG_HISI_CLK
	.get_source = clk_gate_get_source,
#endif
#ifdef CONFIG_HISI_CLK_DEBUG
	.is_enabled = hi3xxx_clkgate_is_enabled,
	.get_reg    = hi3xxx_clkgate_get_reg,
	.dump_reg   = hi3xxx_dumpgate,
#endif
};

static int __hi3xxx_clkgate_dvfs_setup(struct device_node *np, struct hi3xxx_periclk *pclk)
{
	u32 sensitive_level = 0;
	u32 freq_table[DVFS_MAX_FREQ_NUM] = {0};
	u32 volt_table[DVFS_MAX_VOLT_NUM] = {0};
	int i;

	if (of_find_property(np, "hisilicon,clk-dvfs-level", NULL) != NULL) {
		if (of_property_read_u32(np, "hisilicon,clk-dvfs-level", &sensitive_level)) {
			pr_err("[%s] %s node doesn't have dvfs-level property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}

	if (of_find_property(np, "hisilicon,sensitive-freq", NULL) != NULL) {
		if (of_property_read_u32_array(np, "hisilicon,sensitive-freq",
			&freq_table[0], sensitive_level)) {
			pr_err("[%s] %s node doesn't have sensitive-freq property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}
	if (sensitive_level > DVFS_MAX_FREQ_NUM) {
		pr_err("[%s] sensitive level can not bigger than DVFS_MAX_FREQ_NUM!\n", __func__);
		return -EINVAL;
	}
	if (of_find_property(np, "hisilicon,sensitive-volt", NULL) != NULL) {
		if (of_property_read_u32_array(np, "hisilicon,sensitive-volt",
			&volt_table[0], sensitive_level + 1)) {
			pr_err("[%s] %s node doesn't have sensitive-volt property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}

	pclk->peri_dvfs_sensitive = 0;
	if (of_property_read_bool(np, "peri_dvfs_sensitive"))
		pclk->peri_dvfs_sensitive = 1;

	pclk->sensitive_level = sensitive_level;

	for (i = 0; i < DVFS_MAX_FREQ_NUM; i++) {
		pclk->freq_table[i] = freq_table[i];
		pclk->volt_table[i] = volt_table[i];
	}
	pclk->volt_table[i] = volt_table[i];

	return 0;
}

static int __hi3xxx_clkgate_clk_register(struct device_node *np, struct hi3xxx_periclk *pclk,
	const char *clk_name)
{
	struct clk *clk = NULL;
	const char *name = NULL;

	clk = clk_register(NULL, &pclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n", __func__, clk_name);
		return -EINVAL;
	}
	if (!of_property_read_string(np, "clock-output-names", &name))
		clk_register_clkdev(clk, name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);

	return 0;
}

static int __hi3xxx_clkgate_pclk_setup(struct device_node *np, struct hi3xxx_periclk *pclk,
	void __iomem *reg_base)
{
	const char *clk_friend = NULL;
	/* 2:offset and efficient bit 0:offset 1:efficient bit */
	u32 gdata[2] = {0};
	u32 clock_id = 0;
	u32 sync_time = 0;
	u32 sensitive_pll = 0;
	u32 lock_id = 0;

	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clkgate", &gdata[0], 2)) {
		pr_err("[%s] %s node doesn't have hi3xxx-clkgate property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_string(np, "clock-friend-names", &clk_friend))
		clk_friend = NULL;

	if (of_property_read_bool(np, "clock-id")) {
		if (of_property_read_u32_array(np, "clock-id", &clock_id, 1))
			pr_err("[%s] %s clock-id property is null\n", __func__, np->name);
	}

	if (of_property_read_bool(np, "sensitive_pll")) {
		if (of_property_read_u32(np, "sensitive_pll", &sensitive_pll)) {
			pr_err("[%s] %s node doesn't have sensitive_pll property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}
	if (of_find_property(np, "hwspinlock-id", NULL) != NULL) {
		if (of_property_read_u32_array(np, "hwspinlock-id", &lock_id, 1)) {
			pr_err("[%s] %s node doesn't have hwspinliock-id property!\n",
				__func__, np->name);
			return -ENODEV;
		}
	}
	if (of_property_read_u32_array(np, "gate_sync_time", &sync_time, 1)) {
		pclk->sync_time = 0;
	} else {
		WARN_ON(sync_time > CLOCK_GATE_SYNC_MAX);
		pclk->sync_time = sync_time;
	}
	if (of_property_read_bool(np, "always_on"))
		pclk->always_on = 1;
	else
		pclk->always_on = 0;
	/* if gdata[1] is 0, represents the enable reg is fake */
	if (gdata[1] == 0)
		pclk->enable = NULL;
	else
		pclk->enable = reg_base + gdata[0];
	pclk->ebits = gdata[1];
	pclk->friend = clk_friend;
	pclk->clock_id = clock_id;
	pclk->perivolt_poll_id = clock_id;
	pclk->sensitive_pll = sensitive_pll;
	pclk->flags = lock_id;
	pclk->pmu_clk_enable = 0;
	pclk->clk_hwlock = NULL;
	pclk->sctrl = NULL;

	return 0;
}

static void __init hi3xxx_clkgate_setup(struct device_node *np)
{
	struct hi3xxx_periclk *pclk = NULL;
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

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			__func__, np->name);
		return;
	}

	parent_names = __hi3xxx_clk_get_parent_name(np);
	if (parent_names == NULL)
		return;

	pclk = kzalloc(sizeof(*pclk), GFP_KERNEL);
	if (pclk == NULL) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		return;
	}

	init = hisi_clk_init_data_alloc(clk_name);
	if (init == NULL)
		goto err_init;
	init->ops = &hi3xxx_clkgate_ops;
	init->flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
	init->parent_names = &parent_names;
	init->num_parents = 1;

	ret = __hi3xxx_clkgate_dvfs_setup(np, pclk);
	if (ret)
		goto err_clk;

	ret = __hi3xxx_clkgate_pclk_setup(np, pclk, reg_base);
	if (ret)
		goto err_clk;

	pclk->lock = &hs_clk.lock;
	pclk->hw.init = init;
	pclk->pmctrl = hs_clk_base(HS_PMCTRL);

	ret = __hi3xxx_clkgate_clk_register(np, pclk, clk_name);
	if (ret)
		goto err_iomap;

	return;
err_iomap:
	iounmap(pclk->pmctrl);
err_clk:
	kfree(init->name);
	kfree(init);
err_init:
	kfree(pclk);
}


CLK_OF_DECLARE(hi3xxx_gate, "hisilicon,hi3xxx-clk-gate", hi3xxx_clkgate_setup);
