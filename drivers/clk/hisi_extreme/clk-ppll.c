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

#include "clk-ppll.h"
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
#ifdef CONFIG_HISI_PLL_VOTE_SEC
#include "clk-smc.h"
#endif
#ifdef CONFIG_HISI_PLL_LOCK_RETRY
#include <linux/notifier.h>
#include <linux/timer.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <linux/hisi/rdr_hisi_platform.h>

struct pll_lock_info g_pll_lock_info;
struct timer_list g_dmd_timer;
unsigned int g_pll_debug_set;
#endif

#ifdef CONFIG_BALT_IP_PLATFORM
u32 ppll2_en_vote_vol;
u32 ppll2_retry_flag;

static void read_ppll2_en_vote(struct hi3xxx_ppll_clk *ppll_clk)
{
	ppll2_en_vote_vol = readl(ppll_clk->addr + ppll_clk->en_ctrl[0]);
}
#endif

#ifndef CONFIG_HISI_PLL_VOTE_SEC
static int plat_ppll_enable_open(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
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
#ifdef CONFIG_BALT_IP_PLATFORM
	if (ppll == PPLL2)
		read_ppll2_en_vote(ppll_clk);
#endif
	return 0;
}

static void plat_ppll_nogate(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
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
#endif

#ifdef CONFIG_HISI_PLL_LOCK_RETRY
static int wait_ap_pll_lock(const void __iomem *lock_base, int ppll, unsigned int lock_bit)
{
	u32 reg_value, lock_state;
	u32 timeout = 0;
	u32 lockerr_flag = 0;
	u32 pll_lock_state = 0;
	u32 zero_flag = 0;

	do {
		reg_value = readl(lock_base);
		lock_state = (reg_value & BIT(lock_bit)) >> lock_bit;
		/* record lock state 1 to 0 */
		if ((pll_lock_state & 0x1) == 1 && lock_state == 0)
			lockerr_flag = 1;
		pll_lock_state = (pll_lock_state << 1) | lock_state;
		zero_flag |= lock_state;
		udelay(1);
		timeout++;
	} while (timeout < MAX_CHECK_TIME && ((pll_lock_state & PLL_LOCK_STATUS) != PLL_LOCK_STATUS));

	if ((pll_lock_state & PLL_LOCK_STATUS) == PLL_LOCK_STATUS)
		return 0;
	if (lockerr_flag)
		pr_err("[%s]: PPLL-%d[0x%x] lock unstabitily!\n", __func__,
			ppll, pll_lock_state);
	if (!zero_flag)
		pr_err("[%s]: PPLL-%d lock timeout!\n", __func__, ppll);

	return -1;
}
#else
static int wait_ap_pll_lock(const void __iomem *lock_base, int ppll, unsigned int lock_bit)
{
	u32 val;
	u32 timeout = 0;

	do {
		val = readl(lock_base);
		val &= BIT(lock_bit);
		timeout++;
		udelay(1);
		if (timeout > AP_PPLL_STABLE_TIME) {
			pr_err("%s: ppll-%d enable is timeout\n", __func__, ppll);
			break;
		}
	} while (!val);

	return 0;
}
#endif

#ifndef CONFIG_HISI_PLL_VOTE_SEC
static int plat_ppll_enable_ready(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	void __iomem *lock_base = NULL;
	int ret_value;

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
	case AUPLL:
		lock_base = ppll_clk->endisable_addr + ppll_clk->st_ctrl[0];
		ret_value = wait_ap_pll_lock(lock_base, ppll, ppll_clk->st_ctrl[1]);
		if (ret_value)
			return ret_value;
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_ready\n", __func__, ppll);
		break;
	}
	plat_ppll_nogate(ppll_clk, ppll);

	return 0;
}

static void plat_ppll_disable(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
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
	/* wait to retry */
	udelay(1);
#ifdef CONFIG_BALT_IP_PLATFORM
	if (ppll == PPLL2)
		read_ppll2_en_vote(ppll_clk);
#endif
}

static int get_ppll_state(struct hi3xxx_ppll_clk *ppll_clk)
{
	int ppll;
	u32 val, ret;

	ppll = ppll_clk->pll_id;

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
#endif

#ifdef CONFIG_HISI_PLL_VOTE_SEC
static int multicore_ppll_enable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = container_of(hw,
		struct hi3xxx_ppll_clk, hw);

	/* enable count */
	ppll_clk->ref_cnt++;

	if (ppll_clk->pll_id == PPLL0)
		return 0;
	if (ppll_clk->ref_cnt == 1) {
#ifdef CONFIG_HISI_PLL_LOCK_RETRY
		u32 pll_en_count = g_pll_lock_info.pll_en_count[ppll_clk->pll_id];
#else
		u32 pll_en_count = 0;
#endif
		u32 retry_num = 0;
		struct smc_ppll clk_smc_ppll;
		struct smc_ppll atfd_clk_smc_ppll = {
			.smc_fid = 0,
			.arg1 = CLK_PLL_ENABLE,
			.arg2 = ppll_clk->pll_id,
			.arg3 = pll_en_count,
			.arg4 = retry_num,
		};

		clk_smc_ppll = atfd_ppll_clk_smc(atfd_clk_smc_ppll);
#ifdef CONFIG_HISI_PLL_LOCK_RETRY
		g_pll_lock_info.pll_en_count[ppll_clk->pll_id] = clk_smc_ppll.arg3;
		retry_num = clk_smc_ppll.arg4;
		if (retry_num >= MAX_RETRY_NUM) {
			pr_err("[%s]: PPLL-%d retry five count lock err!\n", __func__,
				ppll_clk->pll_id);
#ifdef CONFIG_HISI_CLK_DEBUG
			rdr_syserr_process_for_ap(MODID_AP_S_PANIC_PLL_UNLOCK,
				0ULL, 0ULL);
#endif
			return -1;
		}
		if (!clk_smc_ppll.smc_fid)
			g_pll_lock_info.pll_retry_count[ppll_clk->pll_id][retry_num]++;
#endif
	}
	return 0;
}
#else
static int multicore_ppll_enable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);
	u32 ret;
#ifdef CONFIG_HISI_PLL_LOCK_RETRY
	u32 i;
#endif

	/* enable count */
	ppll_clk->ref_cnt++;

	if (ppll_clk->pll_id == PPLL0)
		return 0;
	if (ppll_clk->ref_cnt == 1) {
		ret = get_ppll_state(ppll_clk);
		if (ret)
			return 0;
#ifdef CONFIG_HISI_PLL_LOCK_RETRY
		for (i = 0; i < MAX_RETRY_NUM; i++) {
			plat_ppll_enable_open(ppll_clk, ppll_clk->pll_id);
			g_pll_lock_info.pll_en_count[ppll_clk->pll_id]++;
			/* wait to lock */
			udelay(20);
			ret = plat_ppll_enable_ready(ppll_clk, ppll_clk->pll_id);
			if (!ret)
				break;
			plat_ppll_disable(ppll_clk, ppll_clk->pll_id);
		}
		if (i == MAX_RETRY_NUM) {
			pr_err("[%s]: PPLL-%d retry five count lock err!\n", __func__,
				ppll_clk->pll_id);
#ifdef CONFIG_BALT_IP_PLATFORM
			if (ppll_clk->pll_id == PPLL2)
				ppll2_retry_flag = MAX_RETRY_NUM;
#endif
			ppll_clk->ref_cnt--;
#ifdef CONFIG_HISI_DEBUG_FS
			rdr_syserr_process_for_ap(MODID_AP_S_PANIC_PLL_UNLOCK, 0ULL, 0ULL);
#endif
			return -1;
		}
		g_pll_lock_info.pll_retry_count[ppll_clk->pll_id][i]++;
#else
		plat_ppll_enable_open(ppll_clk, ppll_clk->pll_id);
		plat_ppll_enable_ready(ppll_clk, ppll_clk->pll_id);
#endif
	}
	return 0;
}
#endif

static void multicore_ppll_disable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);

	/* enable count */
	ppll_clk->ref_cnt--;
	if (ppll_clk->pll_id == PPLL0)
		return;
#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	if (!ppll_clk->ref_cnt) {
#ifdef CONFIG_HISI_PLL_VOTE_SEC
		atfd_service_clk_smc(CLK_PLL_DISABLE, ppll_clk->pll_id, 0, 0);
#else
		plat_ppll_disable(ppll_clk, ppll_clk->pll_id);
#endif
#endif
	}
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int pll_dumpgate(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;
	unsigned long  int clk_base_addr = 0;

	ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);
	if (buf == NULL && s != NULL) {
		clk_base_addr = (uintptr_t)ppll_clk->addr & CLK_ADDR_HIGH_MASK;
		seq_printf(s, "    %-15s    %-15s    en[0x%03X-%u]    gt[0x%03X-%u]    bp[0x%03X-%u]    st[0x%03X-%u]",
			hs_base_addr_transfer(clk_base_addr), "pll", ppll_clk->en_ctrl[0],
			ppll_clk->en_ctrl[1], ppll_clk->gt_ctrl[0], ppll_clk->gt_ctrl[1],
			ppll_clk->bypass_ctrl[0], ppll_clk->bypass_ctrl[1],
			ppll_clk->st_ctrl[0], ppll_clk->st_ctrl[1]);
	}

	return 0;
}
#endif

static const struct clk_ops ppll_ops = {
	.enable = multicore_ppll_enable,
	.disable = multicore_ppll_disable,
#ifdef CONFIG_HISI_CLK_DEBUG
	.dump_reg = pll_dumpgate,
#endif
};

static void set_pll_reg_bit(void __iomem *addr, unsigned int reg_bit)
{
	u32 val;

	val = readl(addr);
	val |= BIT(reg_bit);
	writel(val, addr);
}

static void clear_pll_reg_bit(void __iomem *addr, unsigned int reg_bit)
{
	u32 val;

	val = readl(addr);
	val &= (~BIT(reg_bit));
	writel(val, addr);
}

static void hsdt_ppll_enable_open(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	/*
	 * due to chip design,PCIE0PLL and PCIE1PLL have
	 * no pll gate,no need to config gt bit
	 */
	switch (ppll) {
	case SCPLL:
		/* set gt b0 */
		clear_pll_reg_bit(ppll_clk->addr + ppll_clk->gt_ctrl[0], ppll_clk->gt_ctrl[1]);
		/* set en b1 */
		set_pll_reg_bit(ppll_clk->addr + ppll_clk->en_ctrl[0], ppll_clk->en_ctrl[1]);
		break;
	case PCIE0PLL:
	case PCIE1PLL:
		/* set en b1 */
		set_pll_reg_bit(ppll_clk->addr + ppll_clk->en_ctrl[0], ppll_clk->en_ctrl[1]);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_open\n", __func__, ppll);
		break;
	}
}

static int ppll_enable_open(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	/* en */
	switch (ppll) {
	case SCPLL:
	case PCIE0PLL:
	case PCIE1PLL:
		hsdt_ppll_enable_open(ppll_clk, ppll);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_open\n", __func__, ppll);
		break;
	}

	return 0;
}

static void hsdt_ppll_nogate(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	/*
	 * due to chip design,PCIE0PLL and PCIE1PLL have
	 * no pll gate,no need to config gt bit
	 */
	switch (ppll) {
	case SCPLL:
		/* set bypass b0 */
		clear_pll_reg_bit(ppll_clk->addr + ppll_clk->bypass_ctrl[0],
			ppll_clk->bypass_ctrl[1]);
		/* set gt b1 */
		set_pll_reg_bit(ppll_clk->addr + ppll_clk->gt_ctrl[0], ppll_clk->gt_ctrl[1]);
		break;
	case PCIE0PLL:
	case PCIE1PLL:
		/* set bypass b0 */
		clear_pll_reg_bit(ppll_clk->addr + ppll_clk->bypass_ctrl[0],
			ppll_clk->bypass_ctrl[1]);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_ready\n", __func__, ppll);
		break;
	}
}

static void ppll_nogate(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	/* output clock not gate */
	switch (ppll) {
	case SCPLL:
	case PCIE0PLL:
	case PCIE1PLL:
		hsdt_ppll_nogate(ppll_clk, ppll);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is enable_ready\n", __func__, ppll);
		break;
	}
}

static int ppll_enable_ready(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	int ret_value;

	/* waiting lock */
	switch (ppll) {
	case SCPLL:
	case PCIE0PLL:
	case PCIE1PLL:
		ret_value = wait_ap_pll_lock(ppll_clk->addr + ppll_clk->st_ctrl[0],
			ppll, ppll_clk->st_ctrl[1]);
		break;
	default:
		ret_value = -1;
		pr_err("[%s]: A wrong PPLL-%d is enable_ready\n", __func__, ppll);
		break;
	}
	if (ret_value)
		return ret_value;

	ppll_nogate(ppll_clk, ppll);
	return 0;
}

static void ppll_disable_set_gt_b1(struct hi3xxx_ppll_clk *ppll_clk)
{
	u32 val;

	val = (unsigned int)readl(ppll_clk->addr + ppll_clk->gt_ctrl[0]);
	val |= BIT(ppll_clk->gt_ctrl[1]);
	writel(val, ppll_clk->addr + ppll_clk->gt_ctrl[0]);
}

static void ppll_disable_set_bypass_b1(struct hi3xxx_ppll_clk *ppll_clk)
{
	int ppll = ppll_clk->pll_id;

	switch (ppll) {
	case SCPLL:
	case PCIE0PLL:
	case PCIE1PLL:
		set_pll_reg_bit(ppll_clk->addr + ppll_clk->bypass_ctrl[0],
			ppll_clk->bypass_ctrl[1]);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is set_bypass\n", __func__, ppll);
		break;
	}
}

static void ppll_disable_set_en_b0(struct hi3xxx_ppll_clk *ppll_clk)
{
	int ppll = ppll_clk->pll_id;

	switch (ppll) {
	case SCPLL:
	case PCIE0PLL:
	case PCIE1PLL:
		clear_pll_reg_bit(ppll_clk->addr + ppll_clk->en_ctrl[0], ppll_clk->en_ctrl[1]);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is set_bypass\n", __func__, ppll);
		break;
	}
}

static void ppll_disable_set_gt_b0(struct hi3xxx_ppll_clk *ppll_clk)
{
	u32 val;

	val = (unsigned int)readl(ppll_clk->addr + ppll_clk->gt_ctrl[0]);
	val &= (~BIT(ppll_clk->gt_ctrl[1]));
	writel(val, ppll_clk->addr + ppll_clk->gt_ctrl[0]);
}

static void hsdt_ppll_disable(struct hi3xxx_ppll_clk *ppll_clk, int ppll)
{
	/*
	 * due to chip design,PCIE0PLL and PCIE1PLL have
	 * no pll gate,no need to config gt bit
	 */
	switch (ppll) {
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
	case PCIE0PLL:
	case PCIE1PLL:
		/* set bypass b1 */
		ppll_disable_set_bypass_b1(ppll_clk);
		/* set en b0 */
		ppll_disable_set_en_b0(ppll_clk);
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
	case SCPLL:
	case PCIE0PLL:
	case PCIE1PLL:
		hsdt_ppll_disable(ppll_clk, ppll);
		break;
	default:
		pr_err("[%s]: A wrong PPLL-%d is disable\n", __func__, ppll);
		break;
	}
}

#ifdef CONFIG_HISI_PLL_LOCK_RETRY
static int hi3xxx_multicore_ppll_enable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;
	u32 ret;
	int i;

	ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);
	/* enable count */
	ppll_clk->ref_cnt++;
	if (ppll_clk->pll_id == PPLL0)
		return 0;
	if (ppll_clk->ref_cnt == 1) {
		for (i = 0; i < MAX_RETRY_NUM; i++) {
			ppll_enable_open(ppll_clk, ppll_clk->pll_id);
			g_pll_lock_info.pll_en_count[ppll_clk->pll_id]++;
			/* wait to lock */
			udelay(20);
			ret = ppll_enable_ready(ppll_clk, ppll_clk->pll_id);
			if (!ret)
				break;
			ppll_disable(ppll_clk, ppll_clk->pll_id);
		}
		if (i == MAX_RETRY_NUM) {
			pr_err("[%s]: PPLL-%d retry five count lock err!\n",
				__func__, ppll_clk->pll_id);
			ppll_clk->ref_cnt--;
#ifdef CONFIG_HISI_DEBUG_FS
			rdr_syserr_process_for_ap(MODID_AP_S_PANIC_PLL_UNLOCK, 0ULL, 0ULL);
#endif
			return -1;
		}
		g_pll_lock_info.pll_retry_count[ppll_clk->pll_id][i]++;
	}
	return 0;
}
#else
static int hi3xxx_multicore_ppll_enable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;

	ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);
	/* enable count */
	ppll_clk->ref_cnt++;
	if (ppll_clk->pll_id == PPLL0)
		return 0;
	if (ppll_clk->ref_cnt == 1) {
		ppll_enable_open(ppll_clk, ppll_clk->pll_id);
		ppll_enable_ready(ppll_clk, ppll_clk->pll_id);
	}
	return 0;
}
#endif

static void hi3xxx_multicore_ppll_disable(struct clk_hw *hw)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;

	ppll_clk = container_of(hw, struct hi3xxx_ppll_clk, hw);
	/* enable count */
	ppll_clk->ref_cnt--;
	if (ppll_clk->pll_id == PPLL0)
		return;
#ifndef CONFIG_HISI_CLK_ALWAYS_ON
	if (!ppll_clk->ref_cnt)
		ppll_disable(ppll_clk, ppll_clk->pll_id);
#endif
}

static const struct clk_ops hi3xxx_ppll_ops = {
	.enable = hi3xxx_multicore_ppll_enable,
	.disable = hi3xxx_multicore_ppll_disable,
};

static struct clk *__clk_register_pll(const struct pll_clock *pll,
	struct clock_data *data)
{
	struct hi3xxx_ppll_clk *ppll_clk = NULL;
	struct clk_init_data init;
	struct clk *clk = NULL;
	unsigned int i;
	struct hs_clk *hs_clk = get_hs_clk_info();

	ppll_clk = kzalloc(sizeof(struct hi3xxx_ppll_clk), GFP_KERNEL);
	if (ppll_clk == NULL) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		return clk;
	}
	/* initialize the reference count */
	ppll_clk->ref_cnt = 0;

	init.name = pll->name;
	if (!pll->pll_en_method)
		init.ops = &ppll_ops;
	else
		init.ops = &hi3xxx_ppll_ops;
	init.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
	init.parent_names = &(pll->parent_name);
	init.num_parents = 1;

	ppll_clk->pll_id = pll->pll_id;
	ppll_clk->lock = &hs_clk->lock;
	ppll_clk->hw.init = &init;
	/* PLL gt,en,by reg_base */
	ppll_clk->addr = data->base;
#ifdef CONFIG_HISI_CLK_DEBUG
	ppll_clk->en_method = pll->pll_en_method;
#endif
	/* PLL state reg_base */
	ppll_clk->endisable_addr = hs_clk_base(pll->pll_st_addr);
	for (i = 0; i < PLL_REG_NUM; i++) {
		ppll_clk->en_ctrl[i] = pll->en_ctrl[i];
		ppll_clk->gt_ctrl[i] = pll->gt_ctrl[i];
		ppll_clk->bypass_ctrl[i] = pll->bypass_ctrl[i];
		ppll_clk->st_ctrl[i] = pll->st_ctrl[i];
	}

	clk = clk_register(NULL, &ppll_clk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, pll->name);
		goto err_register;
	}

	/* init is local variable, need set NULL before exit func */
	ppll_clk->hw.init = NULL;

	return clk;
err_register:
	kfree(ppll_clk);
	return clk;
}

#ifdef CONFIG_HISI_PLL_LOCK_RETRY
#ifdef CONFIG_HISI_DEBUG_FS
#define MAX_PLL_INFO_SIZE 100
static int pll_lock_panic_handler(struct notifier_block *nb,
	unsigned long action, void *data)
{
	/* 0x72AC: print pll id */
	unsigned int print_pll_lock_mask = 0x72AC;
	unsigned int i, j, k;
	unsigned char pll_retry_info[MAX_PLL_INFO_SIZE] = {0};
	int ret;

	pr_err("[%s] A panic happened!\n", __func__);
	for (i = 0; i < PPLLMAX; i++) {
		if (!(print_pll_lock_mask & BIT(i)))
			continue;
		k = 0;
		ret = snprintf_s(pll_retry_info, sizeof(pll_retry_info),
			sizeof(pll_retry_info) - 1, "PPLL-%u:", i);
		if (ret < 0) {
			pr_err("[%s] snprintf_s is error!\n", __func__);
			return ret;
		}
		k += ret;
		ret = snprintf_s(pll_retry_info + k, sizeof(pll_retry_info) - k,
			sizeof(pll_retry_info) - k - 1, "[0x%02x]",
			g_pll_lock_info.pll_en_count[i]);
		if (ret < 0) {
			pr_err("[%s] snprintf_s is error!\n", __func__);
			return ret;
		}
		k += ret;
		for (j = 0; j < MAX_RETRY_NUM; j++) {
			ret = snprintf_s(pll_retry_info + k, sizeof(pll_retry_info) - k,
				sizeof(pll_retry_info) - k - 1,
				"[0x%02x]", g_pll_lock_info.pll_retry_count[i][j]);
			if (ret < 0) {
				pr_err("[%s] snprintf_s is error!\n", __func__);
				return ret;
			}
			k += ret;
		}
		pr_err("%s\n", pll_retry_info);
	}
	return 0;
}

static struct notifier_block panic_pll_lock = {
	.notifier_call = pll_lock_panic_handler,
};
static void ppll_debug_init(void)
{
	if (g_pll_debug_set)
		return;
	g_pll_debug_set++;
	atomic_notifier_chain_register(&panic_notifier_list, &panic_pll_lock);
}

#else /* CONFIG_HISI_DEBUG_FS */

/* The timer needs to be triggered every 24 hours */
#define PLL_DMD_REPORT_TIME (24 * 60 * 60)
static void pll_lock_info_clear(unsigned int ppll)
{
	unsigned int i;

	g_pll_lock_info.pll_en_count[ppll] = 0;
	for (i = 0; i < MAX_RETRY_NUM; i++)
		g_pll_lock_info.pll_retry_count[ppll][i] = 0;
}
static void timer_print_info(unsigned long args)
{
	unsigned int i, j, pll_retry_count;

	(void)args;
	for (i = 0; i < PPLLMAX; i++) {
		pll_retry_count = 0;
		for (j = 0; j < MAX_RETRY_NUM; j++)
			pll_retry_count += g_pll_lock_info.pll_retry_count[i][j] * j;
		if (pll_retry_count > 0) {
			bbox_diaginfo_record(CLOCK_PLL_AP, NULL, "ppll-%u %u %u", i,
				g_pll_lock_info.pll_en_count[i],
				pll_retry_count);
			pll_lock_info_clear(i);
			pr_err("[%s] ppll-%u DMD Task Happened!\n", __func__, i);
		}
	}
	mod_timer(&g_dmd_timer, jiffies + HZ * PLL_DMD_REPORT_TIME);
}

static void pll_dmd_timer_set(void)
{
	init_timer(&g_dmd_timer);
	g_dmd_timer.function = timer_print_info;
	add_timer(&g_dmd_timer);
	mod_timer(&g_dmd_timer, jiffies + HZ * PLL_DMD_REPORT_TIME);
}
static void ppll_debug_init(void)
{
	if (g_pll_debug_set)
		return;
	g_pll_debug_set++;
	pll_dmd_timer_set();
}
#endif /* CONFIG_HISI_DEBUG_FS */

#else /* CONFIG_PLL_LOCK_RETRY */

static void ppll_debug_init(void)
{
}
#endif /* CONFIG_PLL_LOCK_RETRY */

void plat_clk_register_pll(const struct pll_clock *clks,
	int nums, struct clock_data *data)
{
	struct clk *clk = NULL;
	int i;

	for (i = 0; i < nums; i++) {
		clk = __clk_register_pll(&clks[i], data);
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}

#ifdef CONFIG_HISI_CLK_DEBUG
		debug_clk_add(clk, CLOCK_PPLL);
#endif

		clk_log_dbg("clks id %d, nums %d, clks name = %s!\n",
			clks[i].id, nums, clks[i].name);

		clk_data_init(clk, clks[i].alias, clks[i].id, data);
	}
	ppll_debug_init();
}

