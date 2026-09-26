/*
 * pcie-feb.c
 *
 * PCIe Feb turn-on/off functions
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include "pcie-kport.h"
#include "pcie-kport-common.h"

#define CRGPERIPH_PCIECTRL0 0x800
#define CRGPERIPH_PCIECTRL1 0x804
#define CRGPERIPH_PEREN12 0x470
#define CRGPERIPH_PERDIS12 0x474

#define IP_PRST_PCIE_PHY (0x1 << 27)

#define SCTRL_SCISODIS 0x44
#define PCIE0PHYISODIS (0x1 << 5)
/* ES register */
#define ES_SOC_PCIE0PHY_ADDR 0xfc040000
#define ES_SOC_PCIE0PHY_SIZE 0x40000
#define ES_SOC_PCIEPHY_CTRL16 0x40
#define ES_SOC_PCIEPHY_CTRL17 0x44
#define ES_SOC_PCIEPHY_STAT16 0x440

#define GT_CLK_PCIE_HP (0x1 << 15)
#define GT_CLK_PCIE_DEBOUNCE (0x1 << 12)
#define GT_CLK_PCIE1_HP (0x1 << 19)
#define GT_CLK_PCIE1_DEBOUNCE (0x1 << 16)

/* cs register */
#define CS_SC_MMC1PLL_CTRL0 0xc00
#define CS_SC_MMC1PLL_CTRL1 0xc04
#define CS_SC_MMC1PLL_CTRL16 0xC40
#define CS_SC_MMC1PLL_CTRL17 0xC44
#define CS_SC_MMC1PLL_CTRL20 0xC50
#define CS_SC_MMC1PLL_CTRL21 0xC54
#define CS_SC_MMC1PLL_STAT0 0xE00

/* define ie,oe cfg */
#define IO_IE_POLAR (0x1 << 29)
#define IO_IE_EN_SOFT (0x1 << 28)
#define IO_IE_EN_HARD_BYPASS (0x1 << 27)
#define IO_OE_EN_HARD_BYPASS (0x1 << 11)
#define IO_HARD_CTRL_DEBOUNCE_BYPASS (0x1 << 10)
#define IO_OE_POLAR (0x1 << 9)
#define IO_OE_GT_MODE (0x2 << 7)
#define IO_OE_EN_SOFT (0x1 << 6)
#define DEBOUNCE_WAITCFG_IN (0xf << 20)
#define DEBOUNCE_WAITCFG_OUT (0xf << 13)

static int pcie_clk_ctrl(struct clk *clk, int clk_on)
{
	int ret = 0;

	if (clk_on) {
		ret = clk_prepare_enable(clk);
		if (ret)
			return ret;
	} else {
		clk_disable_unprepare(clk);
	}

	return ret;
}

/* Init fnpll,VCO 3G; */
static void pcie_cs_fnpll_init(struct pcie_kport *pcie)
{
	u32 val;

	/* choose FNPLL */
	val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_CTRL1);
	val |= (0x1 << 27);
	pcie_apb_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL1);

	val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_CTRL16);
	val &= 0xF000FFFF;
	/* fnpll fbdiv : 0xD0 */
	val |= (0xd0 << 16);
	pcie_apb_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL16);

	val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_CTRL17);
	val &= 0xFF000000;
	/* fnpll fracdiv : 0x555555 */
	val |= (0x555555 << 0);
	pcie_apb_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL17);

	val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_CTRL20);
	val &= 0xF5FF88FF;
	/* fnpll dll_en : 0x1 */
	val |= (0x1 << 27);
	/* fnpll postdiv1 : 0x5 */
	val |= (0x5 << 8);
	/* fnpll postdiv2 : 0x4 */
	val |= (0x4 << 12);
	/* fnpll pll_mode : 0x0 */
	val &= ~(0x1 << 25);
	pcie_apb_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL20);

	pcie_apb_phy_writel(pcie, 0x20, CS_SC_MMC1PLL_CTRL21);
}

static int pcie_cs_fnpll_ctrl(struct pcie_kport *pcie, int enable)
{
	u32 val;
	int time = 200;  /* pll_lock timeout */

	if (!enable) {
		/* disable pll: 1 */
		val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_CTRL16);
		val |= (0x1 << 0);
		pcie_apb_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL16);

		/* pciepll_bp : 1 */
		val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_CTRL20);
		val |= (0x1 << 16);
		pcie_apb_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL20);

		return 0;
	}

	/* enable pll: 0 */
	val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_CTRL16);
	val &= ~(0x1 << 0);
	pcie_apb_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL16);

	if (pcie->dtsinfo.board_type != BOARD_FPGA) {
		val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_STAT0);
		/* PLL lock? */
		while (!(val & 0x10)) {
			if (!time) {
				PCIE_PR_E("wait for pll_lock timeout");
				return -1;
			}
			time--;
			udelay(1);
			val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_STAT0);
		}
	}
	/* pciepll_bp : 0 */
	val = pcie_apb_phy_readl(pcie, CS_SC_MMC1PLL_CTRL20);
	val &= ~(0x1 << 16);
	pcie_apb_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL20);

	return 0;
}

static void pcie_phyref_gt(struct pcie_kport *pcie, int open)
{
	u32 val;

	if (open) {
		/* en hard gt mode */
		val = readl(pcie->crg_base + CRGPERIPH_PCIECTRL0);
		val &= ~(0x1 << 1);
		writel(val, pcie->crg_base + CRGPERIPH_PCIECTRL0);

		/* disable soft gt mode */
		val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
		val |= (0x1 << 14);
		writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
	} else {
		/* disable hard gt mode */
		val = readl(pcie->crg_base + CRGPERIPH_PCIECTRL0);
		val |= (0x1 << 1);
		writel(val, pcie->crg_base + CRGPERIPH_PCIECTRL0);

		/* disable soft gt mode */
		val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
		val |= (0x1 << 14);
		writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
	}
}

static void pcie_hp_debounce_gt(struct pcie_kport *pcie, int open)
{
	u32 val;

	if (open) {
		/* gt_clk_pcie_hp/gt_clk_pcie_debounce open */
		val = readl(pcie->crg_base + CRGPERIPH_PEREN12);
		val |= (GT_CLK_PCIE_HP | GT_CLK_PCIE_DEBOUNCE);
		writel(val, pcie->crg_base + CRGPERIPH_PEREN12);
	} else {
		/* gt_clk_pcie_hp/gt_clk_pcie_debounce close */
		val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
		val |= (GT_CLK_PCIE_HP | GT_CLK_PCIE_DEBOUNCE);
		writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
	}
}

static void pcie_oe_ctrl(struct pcie_kport *pcie, int en_flag)
{
	u32 val;

	val = readl(pcie->crg_base + CRGPERIPH_PCIECTRL0);

	/* set ie cfg */
	val |= IO_IE_EN_HARD_BYPASS;

	/* set oe cfg */
	val &= ~IO_HARD_CTRL_DEBOUNCE_BYPASS;

	/* set phy_debounce in&out time */
	val |= (DEBOUNCE_WAITCFG_IN | DEBOUNCE_WAITCFG_OUT);

	/* select oe_gt_mode */
	val |= IO_OE_GT_MODE;

	if (en_flag)
		val &= ~IO_OE_EN_HARD_BYPASS;
	else
		val |= IO_OE_EN_HARD_BYPASS;

	writel(val, pcie->crg_base + CRGPERIPH_PCIECTRL0);
}

static void pcie_ioref_gt(struct pcie_kport *pcie, int open)
{
	u32 val, crg_pciectrl_offset, crg_perdis12_val;

	if (pcie->rc_id == PCIE_RC0) {
		crg_pciectrl_offset = CRGPERIPH_PCIECTRL0;
		/* crg_perdis_bit:bit13 */
		crg_perdis12_val = (0x1 << 13);
	} else {
		crg_pciectrl_offset = CRGPERIPH_PCIECTRL1;
		/* crg_perdis_bit:bit17 */
		crg_perdis12_val = (0x1 << 17);
	}

	if (open) {
		pcie_oe_ctrl(pcie, ENABLE);

		/* en hard gt mode */
		val = readl(pcie->crg_base + crg_pciectrl_offset);
		val &= ~(0x1 << 0);
		writel(val, pcie->crg_base + crg_pciectrl_offset);

		/* disable soft gt mode */
		val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
		val |= crg_perdis12_val;
		writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
	} else {
		/* disable hard gt mode */
		val = readl(pcie->crg_base + crg_pciectrl_offset);
		val |= (0x1 << 0);
		writel(val, pcie->crg_base + crg_pciectrl_offset);

		/* disable soft gt mode */
		val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
		val |= crg_perdis12_val;
		writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
		pcie_oe_ctrl(pcie, DISABLE);
	}
}

static int pcie_allclk_ctrl(struct pcie_kport *pcie, int clk_on)
{
	u32 val;

	if (clk_on) {
		/* choose 100MHz clk src: Bit[8] : 1 pad, Bit[8] : 0 pll */
		val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
		val &= ~(0x1 << 8);
		pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

		if ((pcie->rc_id == PCIE_RC0) &&
			(pcie->dtsinfo.chip_type == CHIP_TYPE_ES)) {
			/* pull up phy_ref_use_cio_pad */
			val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
			val |= (0x1 << 14);
			pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);
		}

		pcie_cs_fnpll_init(pcie);
		if (pcie_cs_fnpll_ctrl(pcie, ENABLE)) {
			PCIE_PR_E("Failed to enable pll");
			return -1;
		}

		pcie_hp_debounce_gt(pcie, ENABLE);
		pcie_phyref_gt(pcie, ENABLE);
		pcie_ioref_gt(pcie, ENABLE);

		if (pcie->dtsinfo.chip_type == CHIP_TYPE_CS) {
			if (clk_set_rate(pcie->pcie_aclk, AXI_CLK_FREQ)) {
				PCIE_PR_E("Failed to set rate");
				goto GT_CLOSE;
			}
		}

		if (pcie_clk_ctrl(pcie->pcie_aclk, ENABLE)) {
			PCIE_PR_E("Failed to enable pcie_aclk");
			goto GT_CLOSE;
		}
		PCIE_PR_I("aclk on Done");

		if (pcie_clk_ctrl(pcie->pcie_aux_clk, ENABLE)) {
			PCIE_PR_E("Failed to enable pcie_aux_clk");
			goto AUX_CLK_FAIL;
		}
	} else {
		pcie_clk_ctrl(pcie->pcie_aux_clk, DISABLE);
		pcie_clk_ctrl(pcie->pcie_aclk, DISABLE);

		pcie_ioref_gt(pcie, DISABLE);
		pcie_phyref_gt(pcie, DISABLE);
		pcie_hp_debounce_gt(pcie, DISABLE);

		pcie_cs_fnpll_ctrl(pcie, DISABLE);
	}

	return 0;

AUX_CLK_FAIL:
	pcie_clk_ctrl(pcie->pcie_aclk, DISABLE);

GT_CLOSE:
	pcie_ioref_gt(pcie, DISABLE);
	pcie_phyref_gt(pcie, DISABLE);
	pcie_hp_debounce_gt(pcie, DISABLE);

	pcie_cs_fnpll_ctrl(pcie, DISABLE);

	return -1;
}

static int pcie_turn_on(struct pcie_kport *pcie,
			      enum rc_power_status on_flag)
{
	u32 val;
	int ret;

	PCIE_PR_I("+%s+", __func__);

	mutex_lock(&pcie->power_lock);

	if (atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_I("PCIe%u already power on", pcie->rc_id);
		ret = 0;
		goto MUTEX_UNLOCK;
	}

	/* pull down phy ISO */
	pcie_iso_ctrl(pcie, DISABLE);
	PCIE_PR_I("iso disable Done");

	/* enable pcie_phy_apb_pclk,pcie_ctrl_apb_pclk */
	if (pcie_clk_ctrl(pcie->apb_phy_clk, ENABLE)) {
		PCIE_PR_E("Failed to enable apb_phy_clk");
		ret = -1;
		goto MUTEX_UNLOCK;
	}

	if (pcie_clk_ctrl(pcie->apb_sys_clk, ENABLE)) {
		PCIE_PR_E("Failed to enable apb_sys_clk");
		goto APBPHY_CLK_CLOSE;
	}

	PCIE_PR_I("pciectrl/pciephy pclk open Done");

	/* unset module */
	pcie_reset_ctrl(pcie, RST_DISABLE);
	PCIE_PR_I("module unreset Done");

	pcie_io_adjust(pcie);

	/* change 2p mem_ctrl */
	pcie_apb_ctrl_writel(pcie, 0x02605550, SOC_PCIECTRL_CTRL20_ADDR);

	pcie_natural_cfg(pcie);
	PCIE_PR_I("natural cfg Done");

	/* fix chip bug: TxDetectRx fail */
	val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL38_ADDR);
	val |= (0x1 << 2);
	pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL38_ADDR);

	if (pcie_allclk_ctrl(pcie, ENABLE))
		goto APBSYS_CLK_CLOSE;

	if (pcie_port_phy_init(pcie))
		goto ALLCLK_CLOSE;

	/* unrst EP */
	pcie_perst_cfg(pcie, ENABLE);

	if (!is_pipe_clk_stable(pcie)) {
		PCIE_PR_E("PIPE clk is not stable");
		goto GPIO_DISABLE;
	}

	set_phy_eye_param(pcie);

	if (pcie_noc_power(pcie, DISABLE))
		goto GPIO_DISABLE;

	atomic_set(&(pcie->is_power_on), 1);

	PCIE_PR_I("-%s-", __func__);
	ret = 0;
	goto MUTEX_UNLOCK;

GPIO_DISABLE:
	pcie_perst_cfg(pcie, DISABLE);
ALLCLK_CLOSE:
	pcie_allclk_ctrl(pcie, DISABLE);
APBSYS_CLK_CLOSE:
	pcie_reset_ctrl(pcie, RST_ENABLE);
	pcie_clk_ctrl(pcie->apb_sys_clk, DISABLE);
APBPHY_CLK_CLOSE:
	pcie_clk_ctrl(pcie->apb_phy_clk, DISABLE);
	ret = -1;
MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

static int pcie_turn_off(struct pcie_kport *pcie,
			       enum rc_power_status on_flag)
{
	u32 val;
	int ret = 0;

	PCIE_PR_I("+%s+", __func__);

	mutex_lock(&pcie->power_lock);

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_I("PCIe%u already power off", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}

	atomic_set(&(pcie->is_power_on), 0);

	if (pcie_noc_power(pcie, ENABLE)) {
		ret = -1;
		goto MUTEX_UNLOCK;
	}

	pcie_perst_cfg(pcie, DISABLE);

	/* rst controller perst_n */
	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val &= ~(0x1 << 2);
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	pcie_allclk_ctrl(pcie, DISABLE);
	pcie_reset_ctrl(pcie, RST_ENABLE);

	pcie_clk_ctrl(pcie->apb_sys_clk, DISABLE);
	pcie_clk_ctrl(pcie->apb_phy_clk, DISABLE);

	if (pcie->dtsinfo.chip_type == CHIP_TYPE_CS)
		pcie_iso_ctrl(pcie, ENABLE);

	PCIE_PR_I("-%s-", __func__);

MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

struct pcie_platform_ops plat_ops = {
	.sram_ext_load = NULL,
	.plat_on = pcie_turn_on,
	.plat_off = pcie_turn_off,
};

int pcie_plat_init(struct platform_device *pdev, struct pcie_kport *pcie)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
	if (!np) {
		PCIE_PR_E("Failed to get crgctrl node");
		return -1;
	}
	pcie->crg_base = of_iomap(np, 0);
	if (!pcie->crg_base) {
		PCIE_PR_E("Failed to iomap crg_base iomap");
		return -1;
	}

	pcie->plat_ops = &plat_ops;

	return 0;
}
