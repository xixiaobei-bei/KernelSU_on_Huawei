/*
 * Hisilicon clock driver
 *
 * Copyright (c) 2018 Hisilicon Limited.
 *
 *Author:
 *
 */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/clk-provider.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
#include <linux/clk-private.h>
#endif
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/clk.h>

#include "dvfs/peri_volt_poll.h"
#include "clk-kirin-common.h"

#ifdef CONFIG_HISI_CLK_DEBUG
#include "../hisi-clk-debug.h"
#endif

#ifndef CONFIG_HISI_CLK_ALWAYS_ON
/*lint -e750 -esym(750,*)*/
#define hi3xxx_CLK_GATE_DISABLE_OFFSET		0x4
#endif

#define hi3xxx_CLK_GATE_STATUS_OFFSET		0x8
#define hi3xxx_RST_DISABLE_REG_OFFSET		0x4

#define CLOCK_GATE_SYNC_MAX			10 /*us*/

struct hi3xxx_periclk {
	struct clk_hw	hw;
	void __iomem	*enable;	/* enable register */
	void __iomem	*reset;		/* reset register */
	u32		ebits;		/* bits in enable/disable register */
	u32		rbits;		/* bits in reset/unreset register */
	void __iomem	*sctrl;		/*sysctrl addr*/
	void __iomem	*pmctrl;	/*pmctrl addr*/
	const char 	*friend;
	spinlock_t	*lock;
	u32		flags;
	struct hwspinlock	*clk_hwlock;
	u32		peri_dvfs_sensitive;/*0:non,1:direct avs,rate(HZ):sensitive rate*/
	u32		freq_table[DVFS_MAX_FREQ_NUM];
	u32		volt_table[DVFS_MAX_FREQ_NUM+1];
	u32		sensitive_level;
	u32		perivolt_poll_id;
	u32		sensitive_pll;
	u32		always_on;
	u32		gate_abandon_enable;
	u32		sync_time;
	u32		clock_id;
	int		pmu_clk_enable;
};

#ifdef CONFIG_HISI_CLK
extern int IS_FPGA(void);
#endif
extern int __clk_enable(struct clk *clk);
extern void __clk_disable(struct clk *clk);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
extern int __clk_prepare(struct clk *clk);
extern void __clk_unprepare(struct clk *clk);
#else
extern int clk_core_prepare(struct clk_core *clk_core);
extern void clk_core_unprepare(struct clk_core *clk_core);
#endif

#ifdef CONFIG_HISI_PERIDVFS
static int peri_dvfs_set_volt(u32 poll_id, u32 volt_level)
{
	struct peri_volt_poll *pvp = NULL;
	int ret = 0;
	unsigned int volt = 0;
	int loop = PERI_AVS_LOOP_MAX;


	pvp = peri_volt_poll_get(poll_id, NULL);
	if (!pvp) {
		pr_err("pvp get failed!\n");
		return -EINVAL;
	}
	ret = peri_set_volt(pvp, volt_level);
	if (ret < 0) {
		pr_err("[%s]set volt failed ret=%d!\n", __func__, ret);
		return ret;
	}
	volt = peri_get_volt(pvp);
	if(volt > DVFS_MAX_VOLT){
		pr_err("[%s]get volt illegal volt=%u!\n", __func__, volt);
		return -EINVAL;
	}
	if (volt_level > volt) {
		do {
			volt = peri_get_volt(pvp);
			if(volt > DVFS_MAX_VOLT){
				pr_err("[%s]get volt illegal volt=%u!\n", __func__, volt);
				return -EINVAL;
			}
			if (volt < volt_level) {
				loop--;
				usleep_range(150, 300);
			}
		} while (volt < volt_level && loop > 0);
		if (volt < volt_level) {
			pr_err("[%s] fail to updata volt, ret = %d!\n",
				__func__, volt);
			/*after peri avs ok,then open behind*/
			return -EINVAL;
		}
	}
	return ret;
}

static int hisi_peri_dvfs_prepare(struct hi3xxx_periclk *pclk)
{
	unsigned long cur_rate = 0;
	int ret = 0;
	unsigned int i = 0;
	unsigned int level = pclk->sensitive_level;
	if (!pclk->peri_dvfs_sensitive)
		return 0;
	if(pclk->freq_table[0] > 0){
		cur_rate = clk_get_rate(pclk->hw.clk);
		if (!cur_rate)
			pr_err("[%s]soft rate:[%s] must not be 0,please check!\n", __func__, pclk->hw.init->name);

		for(i = 0; i < level; i++){
			if(cur_rate > pclk->freq_table[i] * 1000)//lint !e647
				continue;
			else{
				ret = peri_dvfs_set_volt(pclk->perivolt_poll_id, pclk->volt_table[i]);
				if(ret < 0){
					pr_err("[%s]pvp set volt failed ret =%d!\n", __func__, ret);
				}
				return ret;
			}
		}
		if( i == level){
			ret = peri_dvfs_set_volt(pclk->perivolt_poll_id, pclk->volt_table[i]);
			if(ret < 0){
				pr_err("[%s]pvp set volt failed ret =%d!\n", __func__, ret);
			}
		}
	}else if(0 == pclk->freq_table[0]){
		ret = peri_dvfs_set_volt(pclk->perivolt_poll_id, pclk->volt_table[level]);
		if(ret < 0){
			pr_err("[%s]pvp up volt failed ret =%d!\n", __func__, ret);
		}
	}else{
		pr_err("[%s]soft level:[%s] freq must not be less than 0,please check!\n", __func__, pclk->hw.init->name);
		return -EINVAL;
	}
	return ret;
}

static void hisi_peri_dvfs_unprepare(struct hi3xxx_periclk *pclk)
{
	int ret = 0;
	if (!pclk->peri_dvfs_sensitive)
		return ;
	ret = peri_dvfs_set_volt(pclk->perivolt_poll_id, PERI_VOLT_0);
	if(ret < 0){
		pr_err("[%s]peri dvfs set volt failed ret =%d!\n", __func__, ret);
	}
}
#endif


static int hi3xxx_clkgate_prepare(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk = NULL;
	struct clk *friend_clk;
	int ret = 0;

	pclk = container_of(hw, struct hi3xxx_periclk, hw);

	/*if friend clk exist,enable it*/
	if (pclk->friend) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk)) {
			pr_err("[%s]%s get failed!\n", __func__, pclk->friend);
			return -1;
		}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
		ret = __clk_prepare(friend_clk);
#else
		ret = clk_core_prepare(friend_clk->core);
#endif
		if (ret) {
			pr_err("[%s], friend clock prepare faild!", __func__);
			return ret;
		}
	}
#ifdef CONFIG_HISI_PERIDVFS
	ret = hisi_peri_dvfs_prepare(pclk);
	if(ret < 0)
		pr_err("[%s]set volt failed ret=%d tar=%s!\n", __func__, ret, hw->init->name);
#endif
	return ret;
}

static int hi3xxx_clkgate_enable(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk;
	struct clk *friend_clk;
	int ret = 0;

	pclk = container_of(hw, struct hi3xxx_periclk, hw);

	/*gate sync*/
	if (pclk->sync_time > 0)
		udelay(pclk->sync_time);

	/*sft give up enable*/
	if (pclk->gate_abandon_enable)
		return 0;

	/*enable clock*/
	if (pclk->enable)
		writel(pclk->ebits, pclk->enable);

	/* disable reset register */
	if (pclk->reset)
		writel(pclk->rbits, pclk->reset + hi3xxx_RST_DISABLE_REG_OFFSET);

	/*if friend clk exist,enable it*/
	if (pclk->friend) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk)) {
			pr_err("[%s]%s get failed!\n", __func__ ,pclk->friend);
			return -1;
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
	struct hi3xxx_periclk *pclk;
	struct clk *friend_clk;
	pclk = container_of(hw, struct hi3xxx_periclk, hw);

	/* reset the ip, then disalbe clk */
	if (pclk->reset)
		writel(pclk->rbits, pclk->reset);

#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	if (pclk->enable) {
		if (!pclk->always_on)
			writel(pclk->ebits, pclk->enable + hi3xxx_CLK_GATE_DISABLE_OFFSET);
	}
	if (pclk->sync_time > 0)
		udelay(pclk->sync_time);
	/*if friend clk exist, disable it .*/
	if (pclk->friend) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk)) {
			pr_err("[%s]%s get failed!\n", __func__ ,pclk->friend);
		}
		__clk_disable(friend_clk);
	}
#endif
}

static void hi3xxx_clkgate_unprepare(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk = NULL;
	struct clk *friend_clk;

	pclk = container_of(hw, struct hi3xxx_periclk, hw);

#ifdef CONFIG_HISI_PERIDVFS
	hisi_peri_dvfs_unprepare(pclk);
#endif
#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	if (pclk->friend) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk)) {
			pr_err("%s get failed!\n", pclk->friend);
            return;
		}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
		__clk_unprepare(friend_clk);
#else
		clk_core_unprepare(friend_clk->core);
#endif
	}
#endif
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hi3xxx_clkgate_is_enabled(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk;
	u32 reg = 0;

	pclk = container_of(hw, struct hi3xxx_periclk, hw);

	if (pclk->enable) {
#ifdef CONFIG_HISI_HI6250_CLK
		if ((!strcmp(__clk_get_name(hw->clk), "clk_dss_axi_mm"))
				|| (!strcmp(__clk_get_name(hw->clk), "pclk_mmbuf"))
				|| (!strcmp(__clk_get_name(hw->clk), "aclk_mmbuf")))
			reg = readl(pclk->enable + 0x18);
		else{
			reg = readl(pclk->enable + hi3xxx_CLK_GATE_STATUS_OFFSET);
		}
#else
		reg = readl(pclk->enable + hi3xxx_CLK_GATE_STATUS_OFFSET);
#endif
	} else
		return 2;

	reg &= pclk->ebits;

	return reg ? 1 : 0;
}

static void __iomem *hi3xxx_clkgate_get_reg(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk;
	void __iomem	*ret = NULL;
	u32 val = 0;

	pclk = container_of(hw, struct hi3xxx_periclk, hw);

	if (pclk->enable) {
		ret = pclk->enable + hi3xxx_CLK_GATE_STATUS_OFFSET;
		val = readl(ret);
		val &= pclk->ebits;
		pr_info("\n[%s]: reg = 0x%pK, bits = 0x%x, regval = 0x%x\n",
			__clk_get_name(hw->clk), ret, pclk->ebits, val);
	}

	return ret;
}

static int hi3xxx_dumpgate(struct clk_hw *hw, char* buf, struct seq_file *s)
{
	struct hi3xxx_periclk *pclk;
	void __iomem	*ret = NULL;
	long unsigned int clk_base_addr = 0;
	unsigned int clk_bit = 0;
	u32 index = 0;
	u32 val = 0;

	pclk = container_of(hw, struct hi3xxx_periclk, hw);

	if (pclk->enable && buf && !s) {
		ret = pclk->enable + hi3xxx_CLK_GATE_STATUS_OFFSET;
		val = readl(ret);
		snprintf(buf, DUMP_CLKBUFF_MAX_SIZE, "[%s] : regAddress = 0x%pK, regval = 0x%x\n", \
			__clk_get_name(hw->clk), ret, val);
	}

	if(!buf && s) {
		if(pclk->enable) {
			clk_base_addr = (uintptr_t)pclk->enable & CLK_ADDR_HIGH_MASK;
			clk_bit = (uintptr_t)pclk->enable & CLK_ADDR_LOW_MASK;
			val = pclk->ebits;
			while(val) {
				val = val >> 1;
				if(val)
					index++;
			}
			seq_printf(s, "    %-15s    %-15s    0x%03X    bit-%-2d", \
				hs_base_addr_transfer(clk_base_addr), "gate", clk_bit, index);
		} else {
			seq_printf(s, "    %-15s    %-15s", "NONE", "fixed-gate");
		}
	}

	return 0;
}
#endif

#ifdef CONFIG_HISI_CLK
static int clk_gate_get_source(struct clk_hw *hw)
{
	struct hi3xxx_periclk *pclk;

	pclk = container_of(hw, struct hi3xxx_periclk, hw);

	return pclk->sensitive_pll;
}
#endif

static struct clk_ops hi3xxx_clkgate_ops = {
	.prepare        = hi3xxx_clkgate_prepare,
	.unprepare      = hi3xxx_clkgate_unprepare,
	.enable		= hi3xxx_clkgate_enable,
	.disable	= hi3xxx_clkgate_disable,
#ifdef CONFIG_HISI_CLK
	.get_source = clk_gate_get_source,
#endif
#ifdef CONFIG_HISI_CLK_DEBUG
	.is_enabled = hi3xxx_clkgate_is_enabled,
	.get_reg  = hi3xxx_clkgate_get_reg,
	.dump_reg = hi3xxx_dumpgate,
#endif
};

static void __init hi3xxx_clkgate_setup(struct device_node *np)
{
	struct hi3xxx_periclk *pclk;
	struct clk_init_data *init;
	struct clk *clk;
	const char *clk_name, *name, *clk_friend, *parent_names;
	void __iomem *reg_base;
	u32 rdata[2] = {0};
	u32 gdata[2] = {0};
	u32 freq_table[DVFS_MAX_FREQ_NUM] = {0};
	u32 volt_table[DVFS_MAX_FREQ_NUM+1] = {0};
	u32 sensitive_level = 0;
	u32 sync_time = 0;
	u32 clock_id = 0;
	u32 lock_id = 0;
	u32 sensitive_pll = 0;
	int i = 0;

	reg_base = hs_clk_get_base(np);
	if (!reg_base) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			 __func__, np->name);
		return;
	}
	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clkgate",
				       &gdata[0], 2)) {
		pr_err("[%s] %s node doesn't have hi3xxx-clkgate property!\n",
			 __func__, np->name);
		return;
	}

	if (of_property_read_string(np, "clock-friend-names", &clk_friend))
		clk_friend = NULL;
	if (of_property_read_bool(np, "clock-id")) {
		if (of_property_read_u32_array(np, "clock-id", &clock_id, 1)) {
			pr_err("[%s] %s clock-id property is null\n",
				 __func__, np->name);
		}
	}
	if (of_property_read_bool(np, "sensitive_pll")) {
		if (of_property_read_u32(np, "sensitive_pll", &sensitive_pll)) {
			pr_err("[%s] %s node doesn't have sensitive_pll property!\n",
				 __func__, np->name);
			return;
		}
	}
	if (NULL != of_find_property(np, "hwspinlock-id", NULL)) {
		if (of_property_read_u32_array(np, "hwspinlock-id", &lock_id, 1)) {
			pr_err("[%s] %s node doesn't have hwspinliock-id property!\n",
				__func__, np->name);
			return;
		}
	}

	if (IS_FPGA()) {
		if (NULL != of_find_property(np, "clock-fpga", NULL)) {
			if (of_property_read_string(np, "clock-fpga", &parent_names)) {
				pr_err("[%s] %s node clock-fpga value is NULL!\n",
					__func__, np->name);
				return;
			}
		} else {
			 parent_names = of_clk_get_parent_name(np, 0);
		}
	} else {
		parent_names = of_clk_get_parent_name(np, 0);
	}

	if (NULL != of_find_property(np, "hisilicon,clk-dvfs-level", NULL)) {
		if (of_property_read_u32(np, "hisilicon,clk-dvfs-level", &sensitive_level)) {
			pr_err("[%s] %s node doesn't have dvfs-level property!\n",
				__func__, np->name);
			return;
		}
	}

	if (NULL != of_find_property(np, "hisilicon,sensitive-freq", NULL)) {
		if (of_property_read_u32_array(np, "hisilicon,sensitive-freq",
				       &freq_table[0], sensitive_level)) {
			pr_err("[%s] %s node doesn't have sensitive-freq property!\n",
                __func__, np->name);
			return;
		}
	}
	if(sensitive_level > DVFS_MAX_FREQ_NUM){
		pr_err("[%s] sensitive level can not bigger than DVFS_MAX_FREQ_NUM!\n",
			__func__);
		return;
	}
	if (NULL != of_find_property(np, "hisilicon,sensitive-volt", NULL)) {
		if (of_property_read_u32_array(np, "hisilicon,sensitive-volt",
				       &volt_table[0], sensitive_level+1)) {
			pr_err("[%s] %s node doesn't have sensitive-volt property!\n",
                __func__, np->name);
			return;
		}
	}

	pclk = kzalloc(sizeof(struct hi3xxx_periclk), GFP_KERNEL);
	if (!pclk) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		return;
	}

	init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
	if (!init) {
		pr_err("[%s] fail to alloc init!\n", __func__);
		goto err_init;
	}
	init->name = kstrdup(clk_name, GFP_KERNEL);
	init->ops = &hi3xxx_clkgate_ops;
	init->flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
	init->parent_names = &parent_names;
	init->num_parents = 1;

	if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clkreset",
				       &rdata[0], 2)) {
		pclk->reset = NULL;
		pclk->rbits = 0;
	} else {
		pclk->reset = reg_base + rdata[0];
		pclk->rbits = rdata[1];
	}

	pclk->peri_dvfs_sensitive = 0;
	if (of_property_read_bool(np, "peri_dvfs_sensitive")) {
		pclk->peri_dvfs_sensitive = 1;
	}

	if (of_property_read_bool(np, "always_on"))
		pclk->always_on = 1;
	else
		pclk->always_on = 0;
	if (of_property_read_bool(np, "gate_abandon_enable"))
		pclk->gate_abandon_enable = 1;
	else
		pclk->gate_abandon_enable = 0;
	if (of_property_read_u32_array(np, "gate_sync_time",
				       &sync_time, 1)) {
		pclk->sync_time = 0;
	} else {
		WARN_ON(sync_time > CLOCK_GATE_SYNC_MAX);
		pclk->sync_time = sync_time;
	}

	/* if gdata[1] is 0, represents the enable reg is fake */
	if (gdata[1] == 0)
			pclk->enable = NULL;
	else
			pclk->enable = reg_base + gdata[0];
	pclk->ebits = gdata[1];
	pclk->lock = &hs_clk.lock;
	pclk->hw.init = init;
	pclk->friend = clk_friend;
	pclk->flags = lock_id;
	pclk->clock_id = clock_id;
	pclk->perivolt_poll_id = clock_id;
	pclk->sensitive_pll = sensitive_pll;
	pclk->clk_hwlock = NULL;
	pclk->sctrl = NULL;
	pclk->pmctrl = hs_clk_base(HS_PMCTRL);
	pclk->pmu_clk_enable = 0;
	pclk->sensitive_level = sensitive_level;
	for (i = 0; i < DVFS_MAX_FREQ_NUM; i++){
		pclk->freq_table[i] = freq_table[i];
		pclk->volt_table[i] = volt_table[i];
	}
	pclk->volt_table[i] = volt_table[i];
	clk = clk_register(NULL, &pclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, clk_name);
		goto err_clk;
	}
	if (!of_property_read_string(np, "clock-output-names", &name))
		clk_register_clkdev(clk, name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	return;/*lint !e429*/
err_clk:
	kfree(init);
	init = NULL;
err_init:
	kfree(pclk);
	pclk = NULL;
	return;
}

/*lint -save -e611*/
CLK_OF_DECLARE(hi3xxx_gate, "hisilicon,hi3xxx-clk-gate", hi3xxx_clkgate_setup);
/*lint -restore*/
