/*
 * clk-kirin-common.c
 *
 * Hisilicon clock driver
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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
#include "clk-kirin-common.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/clk.h>

#include <linux/hisi/hisi_mailbox.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hwspinlock.h>
#include <linux/mfd/hisi_pmic.h>
#include <soc_sctrl_interface.h>
#include <soc_crgperiph_interface.h>
#include <soc_pmctrl_interface.h>
#include <securec.h>

#include "hisi-clk-mailbox.h"
#ifdef CONFIG_HISI_CLK_DEBUG
#include "debug/hisi-clk-debug.h"
#endif

#define DDR_FREQ_MASK			0x00FF0000

/* reset register offset */
#define CLK_GATE_ALWAYS_ON_MASK		0x4
#define WIDTH_TO_MASK(width)		((1 << (width)) - 1)
#define MAX_FREQ_NUM			16

#define CLK_HWLOCK_TIMEOUT		1000
#define FREQ_CONVERSION			1000

#define ABB_SCBAKDATA(BASE)		SOC_SCTRL_SCBAKDATA12_ADDR(BASE)

#define AP_ABB_EN			0
#define LPM3_ABB_EN			1
#define CLKGATE_COUNT			2
#define DDR_FREQ_ID			3
#define CLK_SET_OK			1

/*
 * The reverse of DIV_ROUND_UP: The maximum number which
 * divided by m is r
 */
#define MULT_ROUND_UP(r, m)		((r) * (m) + (m) - 1)
#define INVALID_HWSPINLOCK_ID		0xFF

#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
#define CLK_DVFS_IPC_CMD		0xC
static u32 g_count_num_dvfs;
#endif

struct hs_clk hs_clk = {
	.lock = __SPIN_LOCK_UNLOCKED(hs_clk.lock),
};

struct hi3xxx_muxclk {
	struct clk_hw hw;
	void __iomem *reg;	/* mux register */
	u8 shift;
	u8 width;
	u32 mbits;		/* mask bits in mux register */
	spinlock_t *lock;
};

struct hi3xxx_xfreq_clk {
	struct clk_hw hw;
	void __iomem *sysreg;	/* sysctrl register */
	void __iomem *pmreg;	/* pmctrl register */
	u32 id;
	u32 vote_method;
	u32 ddr_mask;
	u32 set_rate_cmd[LPM3_CMD_LEN];
	u32 get_rate_cmd[LPM3_CMD_LEN];
	u32 freq[MAX_FREQ_NUM];
	u32 volt[MAX_FREQ_NUM];
	u32 table_length;
	u32 rate;
};

struct hi3xxx_xfreq_pll {
	struct clk_hw hw;
	void __iomem *reg;	/* pll ctrl0 register */
};

struct hi3xxx_mclk {
	struct clk_hw hw;
	u32 ref_cnt;		/* reference count */
	u32 en_cmd[LPM3_CMD_LEN];
	u32 dis_cmd[LPM3_CMD_LEN];
	u32 clock_id;
	u32 always_on;
	spinlock_t *lock;
};

static struct hwspinlock *clk_hwlock_9;

void __iomem __init *hs_clk_get_base(struct device_node *np);

#ifdef CONFIG_HISI_CLK
int is_fpga(void)
{
	static int flag_diff_fpga_asic = -1;

	if (flag_diff_fpga_asic == -1) {
		if (of_find_node_by_name(NULL, "fpga"))
			flag_diff_fpga_asic = 1;
		else
			flag_diff_fpga_asic = 0;
	}
	return flag_diff_fpga_asic;
}
EXPORT_SYMBOL_GPL(is_fpga);
#endif

unsigned int mul_valid_cal(unsigned long freq, unsigned long freq_conversion)
{
	unsigned long freq_update = freq * freq_conversion;

	return (!freq) || (freq_update / freq == freq_conversion);
}

struct clk_init_data *hisi_clk_init_data_alloc(const char *clk_name)
{
	struct clk_init_data *init = NULL;

	init = kzalloc(sizeof(*init), GFP_KERNEL);
	if (init == NULL)
		return NULL;

	init->name = kstrdup(clk_name, GFP_KERNEL);
	if (init->name == NULL) {
		kfree(init);
		return NULL;
	}

	return init;
}

u32 hisi_clk_crg_type_get(struct device_node *np, const u32 default_crg_type)
{
	u32 crg_type = default_crg_type;

	if (np == NULL)
		return (u32)(-EINVAL);

	/* if dts tree has "clk-crg-type" property, then overwrite crg_type */
	if (of_property_read_bool(np, "clk-crg-type")) {
		if (of_property_read_u32(np, "clk-crg-type", &crg_type)) {
			pr_err("[%s] %s clk-crg-type property is null!\n",
			       __func__, np->name);
			return default_crg_type;
		}
	}

	return crg_type;
}

static int hi3xxx_multicore_clkgate_prepare(struct clk_hw *hw)
{
	return 0;
}

static int hi3xxx_multicore_clkgate_enable(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
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
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
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
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);

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

static void __hi3xxx_abb_clk_hwunlock(struct hi3xxx_periclk *pclk)
{
	hwspin_unlock(pclk->clk_hwlock);
}

static int __hi3xxx_abb_clk_hwlock(struct hi3xxx_periclk *pclk)
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
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
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
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
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
	struct hi3xxx_periclk *pclk = container_of(hw, struct hi3xxx_periclk, hw);
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

static int __hi3xxx_pmu_clkgate_clk_register(struct device_node *np,
	struct hi3xxx_periclk *pclk, const char *clk_name)
{
	struct clk *clk = NULL;
	const char *name = NULL;

	clk = clk_register(NULL, &pclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n", __func__, clk_name);
		return -ENODEV;
	}

	if (!of_property_read_string(np, "clock-output-names", &name))
		clk_register_clkdev(clk, name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);

	return 0;
}

static int __hi3xxx_pmu_clkgate_pclk_init(struct device_node *np, struct hi3xxx_periclk *pclk)
{
	const char *clk_friend = NULL;
	u32 clkgata_val[CLKGATE_COUNT] = {0};
	u32 lock_id = 0;
	u32 clock_id = 0;

	if (of_property_read_u32_array(np, "hisilicon,clkgate", &clkgata_val[0], CLKGATE_COUNT)) {
		pr_err("[%s] %s node doesn't have hisilicon,clkgate property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32_array(np, "hwspinlock-id", &lock_id, 1)) {
		pr_info("[%s] %s node doesn't have hwspinliock-id property!\n",
			__func__, np->name);
		lock_id = INVALID_HWSPINLOCK_ID;
	}
	if (of_property_read_bool(np, "clock-id"))
		if (of_property_read_u32_array(np, "clock-id", &clock_id, 1))
			pr_err("[%s] %s clock_id property is null!\n", __func__, np->name);

	if (of_property_read_string(np, "clock-friend-names", &clk_friend))
		clk_friend = NULL;

	if (of_property_read_bool(np, "always_on"))
		pclk->always_on = 1;
	else
		pclk->always_on = 0;

	if (of_property_read_bool(np, "hisi_mmw_flags")) {
		pclk->clk_pmic_read = hisi_mmw_pmic_reg_read;
		pclk->clk_pmic_write = hisi_mmw_pmic_reg_write;
	} else {
		pclk->clk_pmic_read = hisi_pmic_reg_read;
		pclk->clk_pmic_write = hisi_pmic_reg_write;
	}

	pclk->ebits = BIT(clkgata_val[1]);
	pclk->friend = clk_friend;
	pclk->flags = lock_id;
	pclk->clock_id = clock_id;
	pclk->pmu_clk_enable = clkgata_val[0];

	pclk->enable = NULL;
	pclk->clk_hwlock = NULL;

	return 0;
}

const char *__hi3xxx_clk_get_parent_name(struct device_node *np)
{
	const char *parent_names = NULL;

	if (np == NULL)
		return NULL;

	if (!is_fpga())
		return of_clk_get_parent_name(np, 0);

	if (of_find_property(np, "clock-fpga", NULL) != NULL) {
		if (of_property_read_string(np, "clock-fpga", &parent_names)) {
			pr_err("[%s] %s node clock-fpga value is NULL!\n", __func__, np->name);
			return NULL;
		}
	} else {
		parent_names = of_clk_get_parent_name(np, 0);
	}

	return parent_names;
}

static void __init hi3xxx_pmu_clkgate_setup(struct device_node *np)
{
	struct hi3xxx_periclk *pclk = NULL;
	struct clk_init_data *init = NULL;
	struct device_node *sctrl_np = NULL;
	const char *clk_name = NULL;
	const char *parent_names = NULL;
	int ret;

	parent_names = __hi3xxx_clk_get_parent_name(np);
	if (parent_names == NULL)
		return;
	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n", __func__, np->name);
		return;
	}
	pclk = kzalloc(sizeof(*pclk), GFP_KERNEL);
	if (pclk == NULL)
		return;

	ret = __hi3xxx_pmu_clkgate_pclk_init(np, pclk);
	if (ret)
		goto err_init;

	init = hisi_clk_init_data_alloc(clk_name);
	if (init == NULL)
		goto err_init;

	if (!strcmp(clk_name, "clk_abb_192"))
		init->ops = &hi3xxx_abb_clkgate_ops;
	else
		init->ops = &hi3xxx_pmu_clkgate_ops;
	init->flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
	init->parent_names = &parent_names;
	init->num_parents = 1;

	sctrl_np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (sctrl_np == NULL) {
		pr_err("[%s] fail to find sctrl node!\n", __func__);
		goto err_sctr;
	}
	pclk->sctrl = of_iomap(sctrl_np, 0);

	if (pclk->sctrl == NULL) {
		pr_err("[%s]failed to iomap!\n", __func__);
		goto no_iomap;
	}
	/* only for abb clk */
	pclk->sctrl = ABB_SCBAKDATA(pclk->sctrl);
	pclk->lock = &hs_clk.lock;
	pclk->hw.init = init;

	ret = __hi3xxx_pmu_clkgate_clk_register(np, pclk, clk_name);
	if (ret)
		goto err_clk;

	return;

err_clk:
	iounmap(pclk->sctrl);
no_iomap:
	of_node_put(sctrl_np);
err_sctr:
	kfree(init->name);
	kfree(init);
err_init:
	kfree(pclk);
}


static int __init hi3xxx_parse_mux(struct device_node *np, u8 *num_parents)
{
	int i, cnt;

	/* get the count of items in mux */
	for (i = 0, cnt = 0;; i++, cnt++) {
		/* parent's #clock-cells property is always 0 */
		if (!of_parse_phandle(np, "clocks", i))
			break;
	}

	for (i = 0; i < cnt; i++) {
		if (!of_clk_get_parent_name(np, i)) {
			pr_err("[%s] cannot get %dth parent_clk name!\n", __func__, i);
			return -ENOENT;
		}
	}
	*num_parents = cnt;
	return 0;
}

static void __iomem *__hi3xxx_clkmux_reg_init(struct device_node *np, void __iomem *base,
	u32 *width, u8 *shift)
{
	void __iomem *reg = NULL;
	/* 2:offset and efficient bit */
	u32 rdata[2] = {0};

	if (of_property_read_u32_array(np, "hisilicon,clkmux-reg", &rdata[0], 2)) {
		pr_err("[%s] %s node doesn't have clkmux-reg property!\n",
			__func__, np->name);
		return reg;
	}

	/* 0:offset 1:bit */
	reg = base + rdata[0];
	*shift = ffs(rdata[1]) - 1;
	*width = fls(rdata[1]) - ffs(rdata[1]) + 1;
	return reg;
}

static void __init hi3xxx_clkmux_setup(struct device_node *np)
{
	struct clk *clk = NULL;
	const char *clk_name = NULL;
	const char **parent_names = NULL;
	void __iomem *reg = NULL;
	void __iomem *base = NULL;
	u32 width = 0;
	u8 num_parents, shift;
	u8 flag = 0;
	int i, ret;

	base = hs_clk_get_base(np);
	if (base == NULL) {
		pr_err("[%s] fail to get base!\n", __func__);
		return;
	}

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			__func__, np->name);
		return;
	}

	if (of_property_read_bool(np, "hiword"))
		flag = CLK_MUX_HIWORD_MASK;
	ret = hi3xxx_parse_mux(np, &num_parents);
	if (ret) {
		pr_err("[%s] %s node cannot get num_parents!\n",
			__func__, np->name);
		return;
	}
	parent_names = kzalloc(sizeof(char *) * num_parents, GFP_KERNEL);
	if (parent_names == NULL)
		return;

	for (i = 0; i < num_parents; i++)
		parent_names[i] = of_clk_get_parent_name(np, i);

	reg = __hi3xxx_clkmux_reg_init(np, base, &width, &shift);
	if (reg == NULL)
		goto err_clk;

	clk = clk_register_mux(NULL, clk_name, parent_names, num_parents,
		CLK_SET_RATE_PARENT, reg, shift, width,
		flag, &hs_clk.lock);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register mux clk %s!\n", __func__, clk_name);
		goto err_clk;
	}
	clk_register_clkdev(clk, clk_name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);

err_clk:
	kfree(parent_names);
}

static void __init hs_clkgate_setup(struct device_node *np)
{
	struct clk *clk = NULL;
	const char *clk_name = NULL;
	const char *parent_names = NULL;
	void __iomem *reg_base = NULL;
	unsigned long flags = 0;
	u32 clkgata_val[CLKGATE_COUNT] = {0};

	reg_base = hs_clk_get_base(np);
	if (reg_base == NULL) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}
	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] node %s doesn't have clock-output-names property!\n",
			__func__, np->name);
		return;
	}
	if (of_property_read_u32_array(np, "hisilicon,clkgate", &clkgata_val[0], CLKGATE_COUNT)) {
		pr_err("[%s] node %s doesn't have clkgate property!\n",
			__func__, np->name);
		return;
	}
	if (of_property_read_bool(np, "hisilicon,clkgate-inverted"))
		flags = CLK_GATE_SET_TO_DISABLE;
	if (of_property_read_bool(np, "hiword"))
		flags |= CLK_GATE_HIWORD_MASK;
	if (of_property_read_bool(np, "always_on"))
		flags |= CLK_GATE_ALWAYS_ON_MASK;
	if (of_property_read_bool(np, "pmu32khz"))
		/* 2:efficient bit */
		clkgata_val[0] = clkgata_val[0] << 2;

	/* gate only has the fixed parent */
	parent_names = __hi3xxx_clk_get_parent_name(np);
	if (parent_names == NULL)
		return;

	clk = clk_register_gate(NULL, clk_name, parent_names,
		CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
		reg_base + clkgata_val[0], (u8) clkgata_val[1], flags,
		&hs_clk.lock);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register gate clk %s!\n",
			__func__, clk_name);
		goto err;
	}

	clk_register_clkdev(clk, clk_name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);

err:
	return;
}

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

static int hi3xxx_clkdiv_bestdiv(struct clk_hw *hw, unsigned long rate,
	unsigned long *best_parent_rate)
{
	struct hi3xxx_divclk *dclk = container_of(hw, struct hi3xxx_divclk, hw);
	struct clk *clk_parent = __clk_get_parent(hw->clk);
	int bestdiv = 0;
	unsigned long parent_rate, now, maxdiv, i;
	unsigned long best_rate = 0;

	maxdiv = hi3xxx_get_table_maxdiv(dclk->table);

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
	if (rate == 0) {
		pr_err("[%s] :rate == 0 is illegal!\n", __func__);
		return 0;
	}
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
	int div;

	if (!rate)
		rate = 1;
	div = hi3xxx_clkdiv_bestdiv(hw, rate, prate);
	if (!div) {
		pr_err("[%s] : div == 0 is illegal!\n", __func__);
		return 0;
	}
	return *prate / (unsigned long)div;
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

	if (value > WIDTH_TO_MASK(dclk->width))
		value = WIDTH_TO_MASK(dclk->width);

	if (dclk->lock)
		spin_lock_irqsave(dclk->lock, flags);

	data = readl(dclk->reg);
	data &= ~(WIDTH_TO_MASK(dclk->width) << dclk->shift);
	data |= value << dclk->shift;
	data |= dclk->mbits;
	writel(data, dclk->reg);

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

static int __hi3xxx_clkdiv_clk_register(struct device_node *np, struct hi3xxx_divclk *dclk,
	const char *clk_name)
{
	struct clk *clk = clk_register(NULL, &dclk->hw);

	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register div clk %s!\n",
			__func__, clk_name);
		return -ENODEV;
	}

	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	clk_register_clkdev(clk, clk_name, NULL);
	return 0;
}

static int __hi3xxx_clkdiv_reg_set(struct device_node *np, struct hi3xxx_divclk *dclk,
	void __iomem *reg_base)
{
	/* 2:offset and efficient bit */
	u32 data[2] = {0};

	if (of_property_read_u32_array(np, "hisilicon,clkdiv", &data[0], 2)) {
		pr_err("[%s] node %s doesn't have clkdiv property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	/* 0:offset 1:efficient bit */
	dclk->reg = reg_base + data[0];
	dclk->shift = ffs(data[1]) - 1;
	dclk->width = fls(data[1]) - ffs(data[1]) + 1;
	dclk->mbits = data[1] << 16;

	return 0;
}

static struct clk_div_table *__hi3xxx_clkdiv_table_set(struct device_node *np)
{
	struct clk_div_table *table = NULL;
	/* 2 : offset and efficient bit */
	u32 data[2] = {0};
	unsigned int table_num, min_div, max_div;
	unsigned int multiple = 1;
	int i;

	/* process the div_table */
	if (of_property_read_u32_array(np, "hisilicon,clkdiv-table", &data[0], 2)) {
		pr_err("[%s] node %s doesn't have clkdiv-table property!\n", __func__, np->name);
		return NULL;
	}
	max_div = (u8) data[0];
	min_div = (u8) data[1];
	table_num = max_div - min_div + 1;

	/* table ends with <0, 0>, so plus one to table_num */
	table = kzalloc(sizeof(*table) * (table_num + 1), GFP_KERNEL);
	if (table == NULL) {
		pr_err("[%s] fail to alloc table!\n", __func__);
		return table;
	}

	if (of_property_read_bool(np, "double_div"))
		multiple = 2;

	for (i = 0; i < table_num; i++) {
		table[i].div = (min_div + i) * multiple;
		table[i].val = i;
	}

	return table;

}

static const char *__hi3xxx_clkdiv_clock_name_read(struct device_node *np)
{
	const char *clk_name = NULL;

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] node %s doesn't have clock-output-names property!\n",
			__func__, np->name);
		return NULL;
	}
	return clk_name;
}

static void __init hi3xxx_clkdiv_setup(struct device_node *np)
{
	struct clk_init_data *init = NULL;
	struct clk_div_table *table = NULL;
	struct hi3xxx_divclk *dclk = NULL;
	void __iomem *reg_base = NULL;
	const char *clk_name = NULL;
	const char *parent_names = NULL;
	int ret;

	reg_base = hs_clk_get_base(np);
	if (reg_base == NULL) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}

	clk_name = __hi3xxx_clkdiv_clock_name_read(np);
	if (clk_name == NULL)
		return;

	table = __hi3xxx_clkdiv_table_set(np);
	if (table == NULL)
		return;

	parent_names = __hi3xxx_clk_get_parent_name(np);
	if (parent_names == NULL)
		goto err_dclk;

	dclk = kzalloc(sizeof(*dclk), GFP_KERNEL);
	if (dclk == NULL) {
		pr_err("[%s] fail to alloc dclk!\n", __func__);
		goto err_dclk;
	}

	init = hisi_clk_init_data_alloc(clk_name);
	if (init == NULL)
		goto err_init;
	init->ops = &hi3xxx_clkdiv_ops;
	init->flags = CLK_SET_RATE_PARENT;
	init->parent_names = &parent_names;
	init->num_parents = 1;

	dclk->lock = &hs_clk.lock;
	dclk->hw.init = init;
	dclk->table = table;

	ret = __hi3xxx_clkdiv_reg_set(np, dclk, reg_base);
	if (ret)
		goto err_clk;

	ret = __hi3xxx_clkdiv_clk_register(np, dclk, clk_name);
	if (ret)
		goto err_clk;

	return;
err_clk:
	kfree(init->name);
	kfree(init);
err_init:
	kfree(dclk);
err_dclk:
	kfree(table);
}


static struct device_node *of_get_clk_cpu_node(int cluster)
{
	struct device_node *np = NULL;
	struct device_node *parent = NULL;
	const u32 *mpidr = NULL;

	parent = of_find_node_by_path("/cpus");
	if (parent == NULL) {
		pr_err("failed to find OF /cpus\n");
		return NULL;
	}

	/*
	 * Get first cluster node ; big or little custer core0 must
	 * contain reg and operating-points node
	 * according to DT description, cluster is the 8-15 bit of reg +1
	 */
	for_each_child_of_node(parent, np) {
		mpidr = of_get_property(np, "reg", NULL);
		if (mpidr == NULL) {
			pr_err("%s missing reg property\n", np->full_name);
			of_node_put(np);
			np = NULL;
			break;
		} else if (((be32_to_cpup(mpidr + 1) >> 8) & 0xff) == cluster) {
			if (of_get_property(np, "operating-points-v2", NULL)) {
				pr_debug("cluster%d suppoet operating-points-v2\n", cluster);
				of_node_put(np);
				break;
			} else if (of_get_property(np, "operating-points", NULL)) {
				pr_debug("cluster%d suppoet operating-points-v1\n", cluster);
				of_node_put(np);
				break;
			}
			of_node_put(np);
			pr_err("cluster%d can not find opp v1&v2\n", cluster);
			np = NULL;

			break;
		}
	}

	of_node_put(parent);

	return np;
}

static struct device_node *of_get_xfreq_node(const char *xfreq)
{
	struct device_node *np = NULL;

	if (!strcmp(xfreq, "ddrfreq"))
		np = of_find_compatible_node(NULL, NULL, "hisilicon,ddr_devfreq");
	else if (!strcmp(xfreq, "gpufreq"))
		np = of_find_compatible_node(NULL, NULL, "arm,mali-midgard");
	else if (!strcmp(xfreq, "sysctrl"))
		np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	return np;
}

static int __xfreq_clk_table_opp_np(struct device_node *np,
	struct hi3xxx_xfreq_clk *xfreqclk, int *ret)
{
	struct device_node *opp_np = NULL;
	struct device_node *opp_np_chd = NULL;
	int count = 0;
	int k = 0;
	unsigned long long rate = 0;
	unsigned int volt = 0;

	opp_np = of_parse_phandle(np, "operating-points-v2", 0);

	if (opp_np != NULL) {
		for_each_available_child_of_node(opp_np, opp_np_chd) {
			count++;
			*ret = of_property_read_u64(opp_np_chd, "opp-hz", &rate);
			if (*ret) {
				pr_err("%s: Failed to read opp-hz, %d\n", __func__, *ret);
				return -ENODEV;
			}
			*ret = of_property_read_u32(opp_np_chd, "opp-microvolt", &volt);
			if (*ret) {
				pr_err("%s: Failed to read  opp-microvolt, %d\n", __func__, *ret);
				return -ENODEV;
			}
			if (k >= MAX_FREQ_NUM) {
				pr_err("%s: sizeof freq and volt in device tree illegal!, k = %d\n",
					__func__, k);
				return -EINVAL;
			}
			xfreqclk->freq[k] = (unsigned int)(rate / FREQ_CONVERSION);
			xfreqclk->volt[k] = volt;
			k++;
		}
		/* There should be one of more OPP defined */
		if (WARN_ON(!count)) {
			*ret = -ENOENT;
			return -ENOENT;
		}
		return -EINVAL;
	}

	return 0;
}

static int xfreq_clk_table_init(struct device_node *np, struct hi3xxx_xfreq_clk *xfreqclk)
{
	const struct property *prop = NULL;
	const __be32 *val = NULL;
	int k = 0;
	int ret = 0;
	unsigned int volt, freq;
	int nr, flag;

	flag = __xfreq_clk_table_opp_np(np, xfreqclk, &ret);
	if (flag)
		return ret;

	prop = of_find_property(np, "operating-points", NULL);
	if (prop == NULL)
		return -ENODEV;
	if (!prop->value)
		return -ENODATA;

	/*
	 * Each OPP is a set of tuples consisting of frequency and
	 * voltage like <freq-kHz vol-uV>.
	 * 2:freq-kHz, vol-uV
	 */
	nr = prop->length / sizeof(u32);
	if ((nr % 2) || (nr / 2) > MAX_FREQ_NUM) {
		pr_err("%s: Invalid OPP list\n", __func__);
		return -EINVAL;
	}

	xfreqclk->table_length = nr / 2;
	val = prop->value;
	while (nr) {
		freq = be32_to_cpup(val++);
		volt = be32_to_cpup(val++);
		xfreqclk->freq[k] = freq;
		xfreqclk->volt[k] = volt;
		pr_debug("[%s]: the OPP k %d,freq %u, volt %u\n", __func__, k, freq, volt);
		nr -= 2;
		k++;
	}

	return 0;

}

#define FREQ_INDEX_MASK		0xF
static unsigned int hi3xxx_xfreq_clk_get_freq_idx(struct hi3xxx_xfreq_clk *xfreq_clk)
{
	unsigned int sys_bak_reg;

	if (xfreq_clk->sysreg == NULL)
		return 0;

	sys_bak_reg = readl(xfreq_clk->sysreg);

	/*
	 * sysctrl SCBAKDATA4
	 * bit 0-3	LITTLE Cluster
	 * bit 4-7	BIG Cluster
	 * bit 8-11	DDR
	 * bit 12-15	GPU
	 */
	switch (xfreq_clk->id) {
	case LITTLE_CLUSRET:
		break;
	case BIG_CLUSTER:
		sys_bak_reg >>= 4;
		break;
	case GPU:
		sys_bak_reg >>= 12;
		break;
	case DDR:
		sys_bak_reg >>= 8;
		break;
	default:
		return 0;
	}

	sys_bak_reg &= FREQ_INDEX_MASK;

	if (sys_bak_reg >= xfreq_clk->table_length)
		sys_bak_reg = 0;

	return sys_bak_reg;
}

static unsigned long hi3xxx_xfreq_clk_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct hi3xxx_xfreq_clk *xfreq_clk = container_of(hw, struct hi3xxx_xfreq_clk, hw);
	unsigned int rate, freq_index, mul_ret;

	switch (xfreq_clk->id) {
	/* DDR get freq */
	case DDR_FREQ_ID:
		freq_index = hi3xxx_xfreq_clk_get_freq_idx(xfreq_clk);
		mul_ret = mul_valid_cal(xfreq_clk->freq[freq_index], FREQ_CONVERSION);
		if (!mul_ret)
			return 0;
		rate = xfreq_clk->freq[freq_index] * FREQ_CONVERSION;
		pr_debug("[%s]3 idx=%u rate=%u\n", __func__, freq_index, rate);
		break;
	/* DDR set min */
	default:
		rate = xfreq_clk->rate;
	}
	return rate;
}

static long hi3xxx_xfreq_clk_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *prate)
{
	return rate;
}

static int hi3xxx_xfreq_clk_determine_rate(struct clk_hw *hw, struct clk_rate_request *req)
{
	return 0;
}

static int hi3xxx_xfreq_clk_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	struct hi3xxx_xfreq_clk *xfreq_clk =
		container_of(hw, struct hi3xxx_xfreq_clk, hw);
	/* 1000000:freq conversion HZ_TO_MHZ */
	unsigned long new_rate = rate / 1000000;
	int ret = 0;

	pr_debug("[%s] set rate = %luMHZ\n", __func__, new_rate);
	if (xfreq_clk->vote_method == DDR_HW_VOTE) {
		switch (xfreq_clk->id) {
		case DDRC_MIN_CLK_ID:
		case DDRC_MAX_CLK_ID:
			new_rate = new_rate | xfreq_clk->ddr_mask;
			writel(new_rate, xfreq_clk->pmreg);
			break;
		default:
			pr_err("[%s]dev_id is error!\n", __func__);
		}
	} else {
		xfreq_clk->set_rate_cmd[1] = new_rate;
#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
		ret = hisi_clkmbox_send_msg(xfreq_clk->set_rate_cmd, LPM3_CMD_LEN);
		if (ret < 0) {
			pr_err("[%s]core id:%u fail to send msg to LPM3!\n", __func__, xfreq_clk->id);

			return -EINVAL;
		}
#endif
	}
	xfreq_clk->rate = rate;

	return ret;
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hi3xxx_dump_xfreq_clk(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	if ((buf == NULL) && (s != NULL))
		seq_printf(s, "    %-15s    %-15s", "NONE", "ddr-cpu-clk");

	return 0;
}
#endif

static const struct clk_ops hi3xxx_xfreq_clk_ops = {
	.recalc_rate = hi3xxx_xfreq_clk_recalc_rate,
	.determine_rate = hi3xxx_xfreq_clk_determine_rate,
	.round_rate = hi3xxx_xfreq_clk_round_rate,
	.set_rate = hi3xxx_xfreq_clk_set_rate,
#ifdef CONFIG_HISI_CLK_DEBUG
	.dump_reg = hi3xxx_dump_xfreq_clk,
#endif
};

static int __hi3xxx_xfreq_clk_register(struct device_node *np,
	struct hi3xxx_xfreq_clk *xfreqclk, const char *clk_name)
{
	struct clk *clk = NULL;

	clk = clk_register(NULL, &xfreqclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register xfreqclk %s!\n",
			__func__, clk_name);
		return -EINVAL;
	}
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	clk_register_clkdev(clk, clk_name, NULL);

	return 0;
}

static int __hi3xxx_xfreq_lpm3_cmd_setup(struct device_node *np, struct hi3xxx_xfreq_clk *xfreqclk)
{
	u32 get_rate_cmd[LPM3_CMD_LEN] = {0};
	u32 set_rate_cmd[LPM3_CMD_LEN] = {0};
	int i;

	if (of_property_read_u32_array(np, "hisilicon,get-rate-ipc-cmd",
		&get_rate_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] node %s doesn't get-rate-ipc-cmd property!\n", __func__, np->name);
		return -ENODEV;
	}

	if (of_property_read_u32_array(np, "hisilicon,set-rate-ipc-cmd",
		&set_rate_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] node %s doesn't set-rate-ipc-cmd property!\n", __func__, np->name);
		return -ENODEV;
	}
	for (i = 0; i < LPM3_CMD_LEN; i++) {
		xfreqclk->set_rate_cmd[i] = set_rate_cmd[i];
		xfreqclk->get_rate_cmd[i] = get_rate_cmd[i];
	}

	return 0;
}

static int __hi3xxx_xfreq_xfreqclk_setup(struct device_node *np, struct hi3xxx_xfreq_clk *xfreqclk)
{
	struct device_node *xfreq_np = NULL;
	u32 device_id = 0;
	u32 vote_method = 0;
	u32 scbacdata = 0;
	u32 ddr_mask = DDR_FREQ_MASK;

	if (of_property_read_u32(np, "hisilicon,hi3xxx-xfreq-devid", &device_id)) {
		pr_err("[%s] node %s doesn't have clock-output-names property!\n",
			__func__, np->name);
		return -ENODEV;
	}

	if (of_property_read_u32(np, "hisilicon,hi3xxx-xfreq-ipchw", &vote_method))
		vote_method = 0;

	if (of_property_read_u32(np, "hisilicon,hi3xxx-xfreq-mask", &ddr_mask))
		ddr_mask = DDR_FREQ_MASK;

	if (of_property_read_u32(np, "hisilicon,hi3xxx-xfreq-scbakdata", &scbacdata)) {
		pr_err("[%s] node %s doesn't hi3xxx-xfreq-scbakdata property!\n",
			__func__, np->name);
		return -ENODEV;
	}

	xfreq_np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	xfreqclk->sysreg = of_iomap(xfreq_np, 0);
	xfreq_np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	xfreqclk->pmreg = of_iomap(xfreq_np, 0);

	xfreqclk->id = device_id;
	xfreqclk->vote_method = vote_method;
	xfreqclk->ddr_mask = ddr_mask;

	if (xfreqclk->sysreg == NULL) {
		pr_err("[%s] iomap fail!\n", __func__);
		return -EINVAL;
	}

	if (xfreqclk->pmreg == NULL) {
		pr_err("[%s]pmreg iomap fail!\n", __func__);
		iounmap(xfreqclk->sysreg);
		xfreqclk->sysreg = NULL;
		return -EINVAL;
	}
	xfreqclk->sysreg += scbacdata;
	xfreqclk->pmreg += scbacdata;

	return 0;
}

static void __hi3xxx_xfreqclk_sort(struct hi3xxx_xfreq_clk *xfreqclk)
{
	int i, k, temp;

	for (i = 0; i < MAX_FREQ_NUM - 1; i++) {
		for (k = MAX_FREQ_NUM - 1; k > i; k--) {
			if (xfreqclk->freq[k] < xfreqclk->freq[k - 1]) {
				temp = xfreqclk->freq[k];
				xfreqclk->freq[k] =
				xfreqclk->freq[k - 1];
				xfreqclk->freq[k - 1] = temp;
			}
		}
	}
}

static int __hi3xxx_xfreqclk_set(struct hi3xxx_xfreq_clk *xfreqclk, u32 device_id)
{
	struct device_node *xfreq_np = NULL;
	unsigned int freq_index, mul_ret;
	int i, k, ret;

	switch (device_id) {
	case CPU_CLUSTER_0:
	case CPU_CLUSTER_1:
		xfreq_np = of_get_clk_cpu_node(device_id);
		if (!xfreq_clk_table_init(xfreq_np, xfreqclk)) {
			freq_index = hi3xxx_xfreq_clk_get_freq_idx(xfreqclk);
			mul_ret = mul_valid_cal(xfreqclk->freq[freq_index], FREQ_CONVERSION);
			if (!mul_ret)
				return -EINVAL;
			xfreqclk->rate = xfreqclk->freq[freq_index] * FREQ_CONVERSION;
		}
		return CLK_SET_OK;
	case CLK_G3D:
		xfreq_np = of_get_xfreq_node("gpufreq");
		if (!xfreq_clk_table_init(xfreq_np, xfreqclk)) {
			freq_index = hi3xxx_xfreq_clk_get_freq_idx(xfreqclk);
			mul_ret = mul_valid_cal(xfreqclk->freq[freq_index], FREQ_CONVERSION);
			if (!mul_ret)
				return -EINVAL;
			xfreqclk->rate = xfreqclk->freq[freq_index] * FREQ_CONVERSION;
		}
		return CLK_SET_OK;
	case CLK_DDRC_FREQ:
	case CLK_DDRC_MAX:
	case CLK_DDRC_MIN:
		xfreq_np = of_get_xfreq_node("ddrfreq");
		ret = xfreq_clk_table_init(xfreq_np, xfreqclk);
		if (ret)
			return ret;
		/* sort lowtohigh */
		__hi3xxx_xfreqclk_sort(xfreqclk);

		k = 0;
		for (i = 0; i < MAX_FREQ_NUM; i++) {
			pr_debug("1xfreqclk->freq[i]=%u\n", xfreqclk->freq[i]);
			if (xfreqclk->freq[i] == 0) {
				k++;
				continue;
			}
			xfreqclk->freq[i - k] = xfreqclk->freq[i];
		}
		if (k > 0) {
			for (i = MAX_FREQ_NUM - 1; i > MAX_FREQ_NUM - k - 1; i--)
				xfreqclk->freq[i] = 0;
		}
		for (i = 0; i < MAX_FREQ_NUM; i++)
			pr_debug("2xfreqclk->freq[i]=%u\n", xfreqclk->freq[i]);
		return CLK_SET_OK;
	default:
		pr_err("[%s]dev_id is error!\n", __func__);
	}

	return 0;
}

/*
 * xfreq_clk is used for cpufreq & devfreq.
 */
static void __init hi3xxx_xfreq_clk_setup(struct device_node *np)
{
	const char *clk_name = NULL;
	const char *parent_names = NULL;
	struct clk_init_data *init = NULL;
	struct hi3xxx_xfreq_clk *xfreqclk = NULL;
	void __iomem *reg_base = NULL;
	int ret;

	reg_base = hs_clk_get_base(np);
	if (reg_base == NULL) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}

	if (of_clk_get_parent_name(np, 0) != NULL)
		parent_names = of_clk_get_parent_name(np, 0);

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] node %s doesn't have clock-output-names property!\n",
			__func__, np->name);
		return;
	}

	xfreqclk = kzalloc(sizeof(*xfreqclk), GFP_KERNEL);
	if (xfreqclk == NULL)
		return;

	init = hisi_clk_init_data_alloc(clk_name);
	if (init == NULL)
		goto err_init;
	init->ops = &hi3xxx_xfreq_clk_ops;
	init->parent_names = (parent_names ? &parent_names : NULL);
	init->num_parents = (parent_names ? 1 : 0);
	init->flags = CLK_IS_ROOT | CLK_GET_RATE_NOCACHE;

	xfreqclk->hw.init = init;
	ret = __hi3xxx_xfreq_xfreqclk_setup(np, xfreqclk);
	if (ret)
		goto err_clk;

	ret = __hi3xxx_xfreq_lpm3_cmd_setup(np, xfreqclk);
	if (ret)
		goto err_clk;

	ret = __hi3xxx_xfreqclk_set(xfreqclk, xfreqclk->id);
	if (!ret)
		goto err_clk;

	ret = __hi3xxx_xfreq_clk_register(np, xfreqclk, clk_name);
	if (ret)
		goto err_clk;
	return;

err_clk:
	kfree(init->name);
	kfree(init);
err_init:
	kfree(xfreqclk);
}

static int hi3xxx_mclk_prepare(struct clk_hw *hw)
{
	struct hi3xxx_mclk *mclk = container_of(hw, struct hi3xxx_mclk, hw);
#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	s32 ret = 0;
#endif
	mclk->ref_cnt++;

#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	if (mclk->en_cmd[1] == CLK_DVFS_IPC_CMD) {
		g_count_num_dvfs++;
		if (g_count_num_dvfs == 1) {
			ret = hisi_clkmbox_send_msg_sync(&mclk->en_cmd[0], LPM3_CMD_LEN);
			if (ret)
				pr_err("[%s] fail to enable clk, ret = %d!\n", __func__, ret);
		}
	} else {
		if (mclk->ref_cnt == 1) {
			ret = hisi_clkmbox_send_msg_sync(&mclk->en_cmd[0], LPM3_CMD_LEN);
			if (ret)
				pr_err("[%s] fail to enable clk, ret = %d!\n", __func__, ret);
		}
	}
#endif

#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	return ret;
#else
	return 0;
#endif
}

static void hi3xxx_mclk_unprepare(struct clk_hw *hw)
{

	struct hi3xxx_mclk *mclk = container_of(hw, struct hi3xxx_mclk, hw);
#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	s32 ret;
#endif
	mclk->ref_cnt--;

#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	/* notify m3 when the ref_cnt of gps_clk is 0 */
	if (mclk->always_on)
		return;
	if (mclk->dis_cmd[1] == CLK_DVFS_IPC_CMD) {
		g_count_num_dvfs--;
		if (!g_count_num_dvfs) {
			ret = hisi_clkmbox_send_msg_sync(&mclk->dis_cmd[0], LPM3_CMD_LEN);
			if (ret)
				pr_err("[%s] fail to disable clk, ret = %d!\n", __func__, ret);
		}
	} else {
		if (!mclk->ref_cnt) {
			ret = hisi_clkmbox_send_msg_sync(&mclk->dis_cmd[0], LPM3_CMD_LEN);
			if (ret)
				pr_err("[%s] fail to disable clk, ret = %d!\n", __func__, ret);
		}
	}

#endif
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hi3xxx_dump_mclk(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	if ((buf == NULL) && (s != NULL))
		seq_printf(s, "    %-15s    %-15s", "NONE", "ipc-clk");

	return 0;
}
#endif

static const struct clk_ops hi3xxx_mclk_ops = {
	.prepare = hi3xxx_mclk_prepare,
	.unprepare = hi3xxx_mclk_unprepare,
#ifdef CONFIG_HISI_CLK_DEBUG
	.dump_reg = hi3xxx_dump_mclk,
#endif
};

static int __hi3xxx_mclk_setup(struct device_node *np, struct hi3xxx_mclk *mclk)
{
	u32 en_cmd[LPM3_CMD_LEN] = {0};
	u32 dis_cmd[LPM3_CMD_LEN] = {0};
	u32 clock_id = 0;
	int i;

	if (of_property_read_bool(np, "clock-id"))
		if (of_property_read_u32_array(np, "clock-id", &clock_id, 1))
			pr_err("[%s] %s clock_id property is null!\n", __func__, np->name);

	if (of_property_read_u32_array(np, "hisilicon,ipc-lpm3-cmd-en",
		&en_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] %s node doesn't have hisilicon,ipc-modem-cmd property!\n",
			__func__, np->name);
		return -ENODEV;
	}

	if (of_property_read_u32_array(np, "hisilicon,ipc-lpm3-cmd-dis",
		&dis_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] %s node doesn't have hisilicon,ipc-modem-cmd property!\n",
			__func__, np->name);
		return -ENODEV;
	}

	if (of_property_read_bool(np, "always_on"))
		mclk->always_on = 1;
	else
		mclk->always_on = 0;

	for (i = 0; i < LPM3_CMD_LEN; i++) {
		mclk->en_cmd[i] = en_cmd[i];
		mclk->dis_cmd[i] = dis_cmd[i];
	}
	mclk->clock_id = clock_id;
	/* initialize the reference count */
	mclk->ref_cnt = 0;
	mclk->lock = &hs_clk.lock;

	return 0;
}

static void __init hi3xxx_mclk_setup(struct device_node *np)
{
	struct hi3xxx_mclk *mclk = NULL;
	struct clk_init_data *init = NULL;
	struct clk *clk = NULL;
	const char *clk_name = NULL;
	const char *parent_names = NULL;
	int ret;

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			__func__, np->name);
		return;
	}

	parent_names = of_clk_get_parent_name(np, 0);

	mclk = kzalloc(sizeof(*mclk), GFP_KERNEL);
	if (mclk == NULL)
		goto err_mclk;

	init = hisi_clk_init_data_alloc(clk_name);
	if (init == NULL)
		goto err_init;
	init->ops = &hi3xxx_mclk_ops;
	init->flags = CLK_SET_RATE_PARENT;
	init->parent_names = &parent_names;
	init->num_parents = 1;

	ret = __hi3xxx_mclk_setup(np, mclk);
	if (ret)
		goto err_clk;

	mclk->hw.init = init;

	clk = clk_register(NULL, &mclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n", __func__, clk_name);
		goto err_clk;
	}

	clk_register_clkdev(clk, clk_name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);

	return;

err_clk:
	kfree(init->name);
	kfree(init);
err_init:
	kfree(mclk);
err_mclk:
	return;
}


CLK_OF_DECLARE(hi3xxx_mux, "hisilicon,hi3xxx-clk-mux", hi3xxx_clkmux_setup);
CLK_OF_DECLARE(hi3xxx_div, "hisilicon,hi3xxx-clk-div", hi3xxx_clkdiv_setup);
CLK_OF_DECLARE(hs_gate, "hisilicon,clk-gate", hs_clkgate_setup);
CLK_OF_DECLARE(hi3xxx_pmu_gate, "hisilicon,clk-pmu-gate", hi3xxx_pmu_clkgate_setup);
CLK_OF_DECLARE(hi3xxx_cpu, "hisilicon,hi3xxx-xfreq-clk", hi3xxx_xfreq_clk_setup);
CLK_OF_DECLARE(hi3xxx_mclk, "hisilicon,interactive-clk", hi3xxx_mclk_setup);

static const struct of_device_id hs_of_match[] = {
	{.compatible = "hisilicon,clk-pmctrl", .data = (void *)HS_PMCTRL,},
	{.compatible = "hisilicon,clk-sctrl", .data = (void *)HS_SYSCTRL,},
	{.compatible = "hisilicon,clk-crgctrl", .data = (void *)HS_CRGCTRL,},
	{.compatible = "hisilicon,hi6421pmic", .data = (void *)HS_PMUCTRL,},
	{.compatible = "hisilicon,clk-pctrl", .data = (void *)HS_PCTRL,},
	{.compatible = "hisilicon,media-crg", .data = (void *)HS_MEDIACRG,},
	{.compatible = "hisilicon,iomcu-crg", .data = (void *)HS_IOMCUCRG,},
	{.compatible = "hisilicon,media1-crg", .data = (void *)HS_MEDIA1CRG,},
	{.compatible = "hisilicon,media2-crg", .data = (void *)HS_MEDIA2CRG,},
	{.compatible = "hisilicon,mmc1-crg", .data = (void *)HS_MMC1CRG,},
	{.compatible = "hisilicon,hsdt-crg", .data = (void *)HS_HSDTCRG,},
	{.compatible = "hisilicon,mmc0-crg", .data = (void *)HS_MMC0CRG,},
	{.compatible = "hisilicon,hsdt1-crg", .data = (void *)HS_HSDT1CRG,},
	{},
};

void __iomem __init *hs_clk_base(u32 ctrl)
{
	struct device_node *np = NULL;
	void __iomem *ret = NULL;

	switch (ctrl) {
	case HS_SYSCTRL:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
		break;
	case HS_CRGCTRL:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
		break;
	case HS_PMCTRL:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
		break;
	case HS_MEDIACRG:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,mediactrl");
		break;
	case HS_MEDIA1CRG:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,media1ctrl");
		break;
	case HS_MEDIA2CRG:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,media2ctrl");
		break;
	case HS_MMC1CRG:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,mmc1_sysctrl");
		break;
	case HS_HSDTCRG:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,hsdt_crg");
		break;
	case HS_MMC0CRG:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,mmc0crg");
		break;
	case HS_HSDT1CRG:
		np = of_find_compatible_node(NULL, NULL, "hisilicon,hsdt1_crg");
		break;
	default:
		pr_err("[%s] ctrl err!\n", __func__);
		WARN_ON(1);
		return ret;
	}
	if (np == NULL) {
		pr_err("[%s] node doesn't have node!\n", __func__);
		return ret;
	}
	ret = of_iomap(np, 0);
	WARN_ON(!ret);

	return ret;
}

#ifdef CONFIG_HISI_CLK_DEBUG
char *hs_base_addr_transfer(unsigned long int base_addr)
{
	if (!base_addr) {
		pr_err("[%s] base_addr doesn't exist!\n", __func__);
		return NULL;
	}

	if (base_addr == (uintptr_t) hs_clk.crgctrl)
		return "PERICRG";
	else if (base_addr == (uintptr_t) hs_clk.sctrl)
		return "SCTRL";
	else if (base_addr == (uintptr_t) hs_clk.pmctrl)
		return "PMCTRL";
	else if (base_addr == (uintptr_t) hs_clk.pctrl)
		return "PCTRL";
	else if (base_addr == (uintptr_t) hs_clk.media1crg)
		return "MEDIA1CRG";
	else if (base_addr == (uintptr_t) hs_clk.media2crg)
		return "MEDIA2CRG";
	else if (base_addr == (uintptr_t) hs_clk.mmc1crg)
		return "MMC1CRG";
	else if (base_addr == (uintptr_t) hs_clk.mmc0crg)
		return "MMC0CRG";
	else if (base_addr == (uintptr_t) hs_clk.hsdtcrg)
		return "HSDCRG";
	else if (base_addr == (uintptr_t) hs_clk.hsdt1crg)
		return "HSD1CRG";
	else if (base_addr == (uintptr_t) hs_clk.iomcucrg)
		return "IOMCUCRG";
	else
		return "NONE";

}
#endif

static void __iomem *__hs_clk_get_base(void __iomem **base_addr, struct device_node *parent)
{
	void __iomem *ret = NULL;

	if ((*base_addr) == NULL) {
		ret = of_iomap(parent, 0);
		WARN_ON(!ret);
		*base_addr = ret;
	} else {
		ret = *base_addr;
	}

	return ret;
}

static void __iomem *__hs_clk_get_base_of_device(const struct of_device_id *match,
	struct device_node *parent)
{
	void __iomem *ret = NULL;

	switch ((uintptr_t) match->data) {
	case HS_PMCTRL:
		ret = __hs_clk_get_base(&hs_clk.pmctrl, parent);
		break;
	case HS_SYSCTRL:
		ret = __hs_clk_get_base(&hs_clk.sctrl, parent);
		break;
	case HS_CRGCTRL:
		ret = __hs_clk_get_base(&hs_clk.crgctrl, parent);
		break;
	case HS_PCTRL:
		ret = __hs_clk_get_base(&hs_clk.pctrl, parent);
		break;
	case HS_MEDIACRG:
		ret = __hs_clk_get_base(&hs_clk.mediacrg, parent);
		break;
	case HS_IOMCUCRG:
		ret = __hs_clk_get_base(&hs_clk.iomcucrg, parent);
		break;
	case HS_MEDIA1CRG:
		ret = __hs_clk_get_base(&hs_clk.media1crg, parent);
		break;
	case HS_MEDIA2CRG:
		ret = __hs_clk_get_base(&hs_clk.media2crg, parent);
		break;
	case HS_MMC1CRG:
		ret = __hs_clk_get_base(&hs_clk.mmc1crg, parent);
		break;
	case HS_HSDTCRG:
		ret = __hs_clk_get_base(&hs_clk.hsdtcrg, parent);
		break;
	case HS_MMC0CRG:
		ret = __hs_clk_get_base(&hs_clk.mmc0crg, parent);
		break;
	case HS_HSDT1CRG:
		ret = __hs_clk_get_base(&hs_clk.hsdt1crg, parent);
		break;
	default:
		pr_err("[%s] cannot find the match node!\n", __func__);
		ret = NULL;
	}

	return ret;
}

void __iomem __init *hs_clk_get_base(struct device_node *np)
{
	struct device_node *parent = NULL;
	const struct of_device_id *match = NULL;
	void __iomem *ret = NULL;

	if (np == NULL)
		return NULL;
	parent = of_get_parent(np);
	if (parent == NULL) {
		pr_err("[%s] node %s doesn't have parent node!\n", __func__,
			np->name);
		goto out;
	}

	match = of_match_node(hs_of_match, parent);
	if (match == NULL) {
		pr_err("[%s] parent node %s doesn't match!\n", __func__,
			parent->name);
		goto out;
	}

	ret = __hs_clk_get_base_of_device(match, parent);

out:
	return ret;
}
