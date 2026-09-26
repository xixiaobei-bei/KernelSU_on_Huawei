/*
 * pcie-jun.c
 *
 * PCIe jun turn-on/off functions
 *
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

#include "pcie-kport.h"
#include "pcie-kport-common.h"
#include "pcie-kport-idle.h"

#include <bl31_smc.h>
#include <linux/hisi/hisi_svm.h>

static atomic_t tcu_urst_cnt = ATOMIC_INIT(0);

/* PCIE CTRL Register bit definition */
#define PCIE_PERST_CONFIG_MASK			0x4

/* PMC register offset */
#define NOC_POWER_IDLEREQ			0x380
#define NOC_POWER_IDLE				0x384

/* PMC register bit definition */
#define NOC_PCIE0_POWER				(0x1 << 9)
#define NOC_PCIE1_POWER				(0x1 << 10)
#define NOC_PCIE0_POWER_MASK			(0x1 << 25)
#define NOC_PCIE1_POWER_MASK			(0x1 << 26)

/* HSDT register offset */
#define HSDTCRG_PEREN0				0x0
#define HSDTCRG_PERDIS0				0x4
#define HSDTCRG_PEREN1				0x10
#define HSDTCRG_PERDIS1				0x14
#define HSDTCRG_PERRSTEN0			0x60
#define HSDTCRG_PERRSTDIS0			0x64
#define HSDTPCIEPLL_STATE			0x204
#define HSDT1PCIEPLL_STATE			0x208
#define HSDTCRG_PCIECTRL0			0x300
#define HSDTCRG_PCIECTRL1			0x304
#define PCIE1SCTRL_AWMMUS			0x604
#define PCIE1SCTRL_ARMMUS			0x608
#define PCIE0SCTRL_AWMMUS			0x610
#define PCIE0SCTRL_ARMMUS			0x614

/* HSDT register bit definition */
#define PCIE0_CLK_HP_GATE			(0x1 << 1)
#define PCIE0_CLK_DEBUNCE_GATE			(0x1 << 2)
#define PCIE0_CLK_PHYREF_GATE			(0x1 << 6)
#define PCIE0_CLK_IO_GATE			(0x1 << 7)
#define PCIE1_CLK_HP_GATE			(0x1 << 6)
#define PCIE1_CLK_DEBUNCE_GATE			(0x1 << 7)
#define PCIE1_CLK_PHYREF_GATE			(0x1 << 4)
#define PCIE1_CLK_IO_GATE			(0x1 << 5)
#define PCIEIO_HW_BYPASS			(0x1 << 0)
#define PCIEPHY_REF_HW_BYPASS			(0x1 << 1)
#define PCIEIO_OE_EN_SOFT			(0x1 << 6)
#define PCIEIO_OE_POLAR				(0x1 << 9)
#define PCIEIO_OE_EN_HARD_BYPASS		(0x1 << 11)
#define PCIEIO_IE_EN_HARD_BYPASS		(0x1 << 27)
#define PCIEIO_IE_EN_SOFT			(0x1 << 28)
#define PCIEIO_IE_POLAR				(0x1 << 29)
#define PCIE_CTRL_POR_N			(0x1 << 7)
#define PCIE1_CTRL_POR_N			(0x1 << 21)
#define PCIE_RST_HSDT_TCU			(0x1 << 3)
#define PCIE_RST_HSDT_TBU			(0x1 << 5)
#define PCIE1_RST_HSDT_TBU			(0x1 << 25)
#define PCIE_RST_MCU				(0x1 << 15)
#define PCIE_RST_MCU_32K			(0x1 << 12)
#define PCIE_RST_MCU_19P2			(0x1 << 13)
#define PCIE_RST_MCU_BUS			(0x1 << 14)
#define PCIE1_RST_MCU				(0x1 << 19)
#define PCIE1_RST_MCU_32K			(0x1 << 16)
#define PCIE1_RST_MCU_19P2			(0x1 << 17)
#define PCIE1_RST_MCU_BUS			(0x1 << 18)
#define PCIE_AWRMMUSSIDV			(0x1 << 24)
#define PCIE_AWRMMUSID			(0xE << 8)
#define PCIE1_AWRMMUSID			(0xF << 8)

/* APB PHY register definition */
#define PHY_REF_USE_PAD				(0x1 << 8)
#define PHY_REF_USE_CIO_PAD			(0x1 << 14)

/* pll Bit */
#define FNPLL_EN_BIT				(0x1 << 0)
#define FNPLL_BP_BIT				(0x1 << 1)
#define FNPLL_LP_LOCK_BIT			(0x1 << 0)
#define FNPLL_LOCK_BIT				(0x1 << 4)
#define PLL_SRC_FNPLL				(0x1 << 27)

#define IO_CLK_SEL_CLEAR			(0x3 << 17)
#define IO_CLK_FROM_CIO				(0x0 << 17)

#define NOC_TIMEOUT_VAL				1000
#define FNPLL_LOCK_TIMEOUT			200
#define TBU_CONFIG_TIMEOUT			100

#define GEN3_RELATED_OFF			0x890
#define GEN3_ZRXDC_NONCOMPL			(0x1 << 0)

#define PCIE0_TBU_BASE			0xEE080000
#define PCIE1_TBU_BASE			0xEE180000
#define PCIE_TBU_SIZE			0x2000
#define PCIE_TBU_CR			0x000
#define PCIE_TBU_CRACK			0x004
#define PCIE_TBU_SCR			0x1000
#define PCIE_TBU_EN_REQ_BIT		(0x1 << 0)
#define PCIE_TBU_EN_ACK_BIT		(0x1 << 0)
#define PCIE_TBU_CONNCT_STATUS_BIT	(0x1 << 1)
#define PCIE_TBU_TOK_TRANS_MSK	(0xFF << 8)
#define PCIE_TBU_TOK_TRANS		0x8
#define PCIE_TBU_DEFALUT_TOK_TRANS	0xF
#define PICE0_SMMUID			0x3
#define PICE1_SMMUID			0x4

static u32 hsdt_crg_reg_read(struct pcie_kport *pcie, u32 reg)
{
	return readl(pcie->crg_base + reg);
}

static void hsdt_crg_reg_write(struct pcie_kport *pcie, u32 val, u32 reg)
{
	writel(val, pcie->crg_base + reg);
}

/*
 * exit or enter noc power idle
 * exit: 1, exit noc power idle
 *       0, enter noc power idle
 */
static int pcie_noc_power_jun(struct pcie_kport *pcie, u32 exit)
{
	u32 mask_bits, val_bits, val;
	u32 time = NOC_TIMEOUT_VAL;

	if (pcie->rc_id == PCIE_RC0) {
		val_bits = NOC_PCIE0_POWER;
		mask_bits = NOC_PCIE0_POWER_MASK;
	} else {
		val_bits = NOC_PCIE1_POWER;
		mask_bits = NOC_PCIE1_POWER_MASK;
	}

	/*
	 * bits in mask_bits set to write the bit
	 * if bit in val_bits is 0, exit noc power idle
	 * or enter noc power idle
	 */
	if (exit) {
		writel(mask_bits, pcie->pmctrl_base + NOC_POWER_IDLEREQ);
		val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
		while (val & val_bits) {
			udelay(1);
			if (!time) {
				PCIE_PR_E("Exit failed :%u", val);
				return -1;
			}
			time--;
			val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
		}
	} else {
		writel(mask_bits | val_bits, pcie->pmctrl_base + NOC_POWER_IDLEREQ);
		val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
		while ((val & val_bits) != val_bits) {
			udelay(1);
			if (!time) {
				PCIE_PR_E("Enter failed :%u", val);
				return -1;
			}
			time--;
			val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
		}
	}

	return 0;
}

/* config fnpll_1v2 */
static int pcie_fnpll_lp_ctrl(struct pcie_kport *pcie, u32 enable)
{
	int ret = 0;

	if (enable) {
		udelay(5);

		ret = clk_enable(pcie->pcie_refclk);
	} else {
		clk_disable(pcie->pcie_refclk);
	}

	return ret;
}

static void pcie_phy_por_n(struct pcie_kport *pcie)
{
	u32 val;

	val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val |= PCIEPHY_POR_N_RESET_BIT;
	pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);
}

static void pcie_ctrl_por_n_rst(struct pcie_kport *pcie, u32 enable)
{
	u32 reg, mask;

	if (enable)
		reg = HSDTCRG_PERRSTEN0;
	else
		reg = HSDTCRG_PERRSTDIS0;

	if (pcie->rc_id == PCIE_RC0)
		mask = PCIE_CTRL_POR_N;
	else
		mask = PCIE1_CTRL_POR_N;

	hsdt_crg_reg_write(pcie, mask, reg);
}

static void pcie_en_rx_term(struct pcie_kport *pcie)
{
	u32 val;

	val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL39_ADDR);
	val |= PCIEPHY_RX_TERMINATION_BIT;
	pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL39_ADDR);
}

/*
 * enable:ENABLE  -- enable clkreq control phyio clk
 *        DISABLE -- disable clkreq control phyio clk
 */
static void pcie_phyio_hard_bypass(struct pcie_kport *pcie, bool enable)
{
	u32 val, reg_addr;

	if (pcie->rc_id == PCIE_RC0)
		reg_addr = HSDTCRG_PCIECTRL0;
	else
		reg_addr = HSDTCRG_PCIECTRL1;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	if (enable)
		val &= ~PCIEIO_HW_BYPASS;
	else
		val |= PCIEIO_HW_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

/*
 * enable:ENABLE--enable clkreq control phyref clk
 *        DISABLE--disable clkreq control phyref clk
 */
static void pcie_phyref_hard_bypass(struct pcie_kport *pcie, bool enable)
{
	u32 val, reg_addr;

	if (pcie->rc_id == PCIE_RC0)
		reg_addr = HSDTCRG_PCIECTRL0;
	else
		reg_addr = HSDTCRG_PCIECTRL1;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	if (enable)
		val &= ~PCIEPHY_REF_HW_BYPASS;
	else
		val |= PCIEPHY_REF_HW_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

/*
 * Config gt_clk_pciephy_ref_inuse
 * enable: ENABLE--controlled by ~pcie_clkreq_in
 *         FALSE--clock down
 */
static void pcie_phy_ref_clk_gt(struct pcie_kport *pcie, u32 enable)
{
	u32  mask, reg;

	/* HW bypass cfg */
	pcie_phyref_hard_bypass(pcie, enable);

	/* soft ref cfg,Always disable SW control */
	if (pcie->rc_id == PCIE_RC0) {
		mask = PCIE0_CLK_PHYREF_GATE;
		reg = HSDTCRG_PERDIS1;
	} else {
		mask = PCIE1_CLK_PHYREF_GATE;
		reg = HSDTCRG_PERDIS0;
	}
	hsdt_crg_reg_write(pcie, mask, reg);
}

/*
 * enable: ENABLE  -- control by pcieio_oe_mux
 *         DISABLE -- close
 */
static void pcie_oe_config(struct pcie_kport *pcie, bool enable)
{
	u32 val, reg_addr;

	if (pcie->rc_id == PCIE_RC0)
		reg_addr = HSDTCRG_PCIECTRL0;
	else
		reg_addr = HSDTCRG_PCIECTRL1;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	val &= ~PCIEIO_OE_POLAR;
	val &= ~PCIEIO_OE_EN_SOFT;
	if (enable)
		val &= ~PCIEIO_OE_EN_HARD_BYPASS;
	else
		val |= PCIEIO_OE_EN_HARD_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

/*
 * enable: ENABLE  -- control by pcie_clkreq_in_n
 *         DISABLE -- close
 */
static void pcie_ie_config(struct pcie_kport *pcie, bool enable)
{
	u32 val, reg_addr;

	if (pcie->rc_id == PCIE_RC0)
		reg_addr = HSDTCRG_PCIECTRL0;
	else
		reg_addr = HSDTCRG_PCIECTRL1;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	val &= ~PCIEIO_IE_POLAR;
	val &= ~PCIEIO_IE_EN_SOFT;
	if (enable)
		val &= ~PCIEIO_IE_EN_HARD_BYPASS;
	else
		val |= PCIEIO_IE_EN_HARD_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

static void pcie_ioref_gt(struct pcie_kport *pcie, u32 enable)
{
	u32 val, mask, reg;

	/* selcet cio */
	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL21_ADDR);
	val &= ~IO_CLK_SEL_CLEAR;
	val |= IO_CLK_FROM_CIO;
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL21_ADDR);

	/*
	 * HW bypass: DISABLE:HW don't control
	 *            ENABLE:clkreq_n is one of controller
	 */
	pcie_phyio_hard_bypass(pcie, ENABLE);

	/* disable SW control */
	if (pcie->rc_id == PCIE_RC0) {
		mask = PCIE0_CLK_IO_GATE;
		reg = HSDTCRG_PERDIS1;
	} else {
		mask = PCIE1_CLK_IO_GATE;
		reg = HSDTCRG_PERDIS0;
	}
	hsdt_crg_reg_write(pcie, mask, reg);

	/* enable/disable ie/oe according mode */
	if (unlikely(pcie->dtsinfo.ep_flag)) {
		pcie_oe_config(pcie, DISABLE);
		pcie_ie_config(pcie, enable);
	} else {
		pcie_oe_config(pcie, enable);
		pcie_ie_config(pcie, DISABLE);
	}
}

/* enable/disable hp&debounce clk */
static void pcie_hp_debounce_clk_gt(struct pcie_kport *pcie, u32 enable)
{
	u32 mask, reg;

	if (pcie->rc_id == PCIE_RC0) {
		mask = PCIE0_CLK_HP_GATE | PCIE0_CLK_DEBUNCE_GATE;
		if (enable)
			reg = HSDTCRG_PEREN1;
		else
			reg = HSDTCRG_PERDIS1;
	} else {
		mask = PCIE1_CLK_HP_GATE | PCIE1_CLK_DEBUNCE_GATE;
		if (enable)
			reg = HSDTCRG_PEREN0;
		else
			reg = HSDTCRG_PERDIS0;
	}

	hsdt_crg_reg_write(pcie, mask, reg);
}

/*
 * For RC, select FNPLL
 * For EP, select CIO
 * Select FNPLL
 */
static int pcie_refclk_on(struct pcie_kport *pcie)
{
	u32 val;
	int ret;

	ret = pcie_fnpll_lp_ctrl(pcie, ENABLE);
	if (ret) {
		PCIE_PR_E("Failed to enable 100MHz ref_clk");
		return ret;
	}

	val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val &= ~PHY_REF_USE_PAD;
	pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

	val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	if (pcie->dtsinfo.ep_flag)
		val |= PHY_REF_USE_CIO_PAD;
	else
		val &= ~PHY_REF_USE_CIO_PAD;
	pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

	/* enable pcie hp&debounce clk */
	pcie_hp_debounce_clk_gt(pcie, ENABLE);
	/* gate pciephy clk */
	pcie_phy_ref_clk_gt(pcie, ENABLE);
	/* gate pcieio clk */
	pcie_ioref_gt(pcie, ENABLE);

	PCIE_PR_I("100MHz refclks enable Done");
	return 0;
}

static void pcie_refclk_off(struct pcie_kport *pcie)
{
	pcie_hp_debounce_clk_gt(pcie, DISABLE);
	pcie_ioref_gt(pcie, DISABLE);
	pcie_phy_ref_clk_gt(pcie, DISABLE);

	pcie_fnpll_lp_ctrl(pcie, DISABLE);
}

static int pcie_refclk_ctrl(struct pcie_kport *pcie, bool clk_on)
{
	int ret;

	if (clk_on) {
		ret = clk_prepare(pcie->pcie_refclk);
		if (ret) {
			PCIE_PR_E("Failed to prepare pcie_refclk");
			return ret;
		}

		ret = pcie_refclk_on(pcie);
		if (ret) {
			PCIE_PR_E("Failed to enable pcie_refclk");
			goto REF_CLK;
		}
	} else {
		pcie_refclk_off(pcie);
		clk_unprepare(pcie->pcie_refclk);
	}

	return 0;

REF_CLK:
	clk_unprepare(pcie->pcie_refclk);

	return ret;
}

static void pcie_l1ss_fixup(struct pcie_kport *pcie)
{
	u32 val;
	/* fix l1ss exit issue */
	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, GEN3_RELATED_OFF, 0x4);
	val &= ~GEN3_ZRXDC_NONCOMPL;
	pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, GEN3_RELATED_OFF, 0x4, val);
}

static int pcie_clk_config(struct pcie_kport *pcie, enum pcie_clk_type clk_type, u32 enable)
{
	int ret = 0;

	switch (clk_type) {
	case PCIE_INTERNAL_CLK:
		if (!enable) {
			clk_disable_unprepare(pcie->apb_sys_clk);
			clk_disable_unprepare(pcie->apb_phy_clk);
			break;
		}

		/* pclk for phy */
		ret = clk_prepare_enable(pcie->apb_phy_clk);
		if (ret) {
			PCIE_PR_E("Failed to enable apb_phy_clk");
			break;
		}

		/* pclk for ctrl */
		ret = clk_prepare_enable(pcie->apb_sys_clk);
		if (ret) {
			clk_disable_unprepare(pcie->apb_phy_clk);
			PCIE_PR_E("Failed to enable apb_sys_clk");
			break;
		}
		break;
	case PCIE_EXTERNAL_CLK:
		if (!enable) {
			clk_disable_unprepare(pcie->pcie_aux_clk);
			clk_disable_unprepare(pcie->pcie_tbu_clk);
			clk_disable_unprepare(pcie->pcie_tcu_clk);
			clk_disable_unprepare(pcie->pcie_aclk);
			break;
		}

		/* Enable pcie axi clk */
		ret = clk_prepare_enable(pcie->pcie_aclk);
		if (ret) {
			PCIE_PR_E("Failed to enable axi_aclk");
			break;
		}

		/* Enable pcie tcu clk */
		ret = clk_prepare_enable(pcie->pcie_tcu_clk);
		if (ret) {
			clk_disable_unprepare(pcie->pcie_aclk);
			PCIE_PR_E("Failed to enable tcu_clk");
			break;
		}

		/* Enable pcie tbu clk */
		ret = clk_prepare_enable(pcie->pcie_tbu_clk);
		if (ret) {
			clk_disable_unprepare(pcie->pcie_tcu_clk);
			clk_disable_unprepare(pcie->pcie_aclk);
			PCIE_PR_E("Failed to enable tbu_clk");
			break;
		}

		/* enable pcie aux clk */
		ret = clk_prepare_enable(pcie->pcie_aux_clk);
		if (ret) {
			clk_disable_unprepare(pcie->pcie_tbu_clk);
			clk_disable_unprepare(pcie->pcie_tcu_clk);
			clk_disable_unprepare(pcie->pcie_aclk);
			PCIE_PR_E("Failed to enable aux_clk");
			break;
		}
		break;
	default:
		PCIE_PR_E("Invalid input parameters");
		ret = -EINVAL;
	}

	return ret;
}

static void pcie_tcu_rst(struct pcie_kport *pcie)
{
	if (atomic_sub_return(1, &tcu_urst_cnt) == 0)
		hsdt_crg_reg_write(pcie, PCIE_RST_HSDT_TCU, HSDTCRG_PERRSTEN0);
}

static void pcie_tcu_urst(struct pcie_kport *pcie)
{
	if (atomic_add_return(1, &tcu_urst_cnt) == 1)
		hsdt_crg_reg_write(pcie, PCIE_RST_HSDT_TCU, HSDTCRG_PERRSTDIS0);
}

static void pcie_tcu_tbu_rst_gt(struct pcie_kport *pcie, u32 enable)
{
	u32 mask, reg;

	if (enable)
		pcie_tcu_rst(pcie);
	else
		pcie_tcu_urst(pcie);

	if (enable)
		reg = HSDTCRG_PERRSTEN0;
	else
		reg = HSDTCRG_PERRSTDIS0;

	if (pcie->rc_id == PCIE_RC0)
		mask = PCIE_RST_HSDT_TBU;
	else
		mask = PCIE1_RST_HSDT_TBU;

	hsdt_crg_reg_write(pcie, mask, reg);
}

static noinline uint64_t pcie_tbu_bypass(struct pcie_kport *pcie)
{
	register uint64_t r0 __asm__("x0") = (uint64_t)FID_PCIE_SET_TBU_BYPASS;
	register uint64_t r1 __asm__("x1") = (uint64_t)pcie->rc_id;

	__asm__ volatile(__asmeq("%0", "x0")
			 __asmeq("%1", "x1")
			 "smc #0\n"
			 : "+r"(r0)
			 : "r"(r1));

	PCIE_PR_I("Bypass TBU");
	return r0;
}

static noinline uint64_t pcie_mem_unsec(struct pcie_kport *pcie)
{
	register uint64_t r0 __asm__("x0") = (uint64_t)FID_PCIE_SET_MEM_UNSEC;
	register uint64_t r1 __asm__("x1") = (uint64_t)pcie->rc_id;

	__asm__ volatile(__asmeq("%0", "x0")
			 __asmeq("%1", "x1")
			 "smc #0\n"
			 : "+r"(r0)
			 : "r"(r1));

	PCIE_PR_I("Set PCIe mem unsec");
	return r0;
}

static int pcie_smmu_init(struct pcie_kport *pcie, u32 enable)
{
	if (enable) {
		if (pcie_tbu_bypass(pcie)) {
			PCIE_PR_E("Bypass TBU failed");
			return -1;
		}

		if (pcie_mem_unsec(pcie)) {
			PCIE_PR_E("Set pcie_mem_unsec failed");
			return -1;
		}
	}

	return 0;
}

static void pcie_en_dbi_ep_splt(struct pcie_kport *pcie)
{
	u32 val;

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL30_ADDR);
	if (pcie->rc_id == PCIE_RC0)
		val &= ~PCIE_DBI_EP_SPLT_BIT;
	else
		val |= PCIE_DBI_EP_SPLT_BIT;
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL30_ADDR);
}

#ifdef XW_PHY_MCU
static void mcu_rst_gt(struct pcie_kport *pcie, u32 enable)
{
	u32 mask, reg;

	if (enable)
		reg = HSDTCRG_PERRSTEN0;
	else
		reg = HSDTCRG_PERRSTDIS0;

	if (pcie->rc_id == PCIE_RC0)
		mask = PCIE_RST_MCU | PCIE_RST_MCU_32K |
			PCIE_RST_MCU_19P2 | PCIE_RST_MCU_BUS;
	else
		mask = PCIE1_RST_MCU | PCIE1_RST_MCU_32K |
			PCIE1_RST_MCU_19P2 | PCIE1_RST_MCU_BUS;

	hsdt_crg_reg_write(pcie, mask, reg);
}

static int32_t phy_core_poweron(struct pcie_kport *pcie)
{
	int ret;

	I("+\n");
	ret = pcie_phy_core_clk_cfg(pcie, ENABLE);
	if (ret)
		return ret;

	mcu_rst_gt(pcie, DISABLE);
	I("-\n");

	return 0;
}

static int32_t phy_core_poweroff(struct pcie_kport *pcie)
{
	int ret;

	mcu_rst_gt(pcie, ENABLE);

	ret = pcie_phy_core_clk_cfg(pcie, DISABLE);

	return ret;
}

static int32_t phy_core_on(struct pcie_kport *pcie)
{
	int32_t ret;

	I("+\n");
	ret = phy_core_poweron(pcie);
	if (ret) {
		PCIE_PR_E("phy core power");
		return ret;
	}

	ret = pcie_phy_core_start(pcie);
	if (ret) {
		PCIE_PR_E("core start");
		return ret;
	}

	PCIE_PR_I("Phy Core running");
	return ret;
}

static int32_t phy_core_off(struct pcie_kport *pcie)
{
	return phy_core_poweroff(pcie);
}
#endif

static int pcie_turn_on(struct pcie_kport *pcie, enum rc_power_status on_flag)
{
	int ret = 0;
	unsigned long flags = 0;

	PCIE_PR_I("+%s+", __func__);

	mutex_lock(&pcie->power_lock);

	if (atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_I("PCIe%u already power on", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}

	/* pclk for phy & ctrl */
	ret = pcie_clk_config(pcie, PCIE_INTERNAL_CLK, ENABLE);
	if (ret)
		goto INTERNAL_CLK;

	/* unrst pcie_phy_apb_presetn pcie_ctrl_apb_presetn */
	pcie_reset_ctrl(pcie, RST_DISABLE);

	/* adjust output refclk amplitude, currently no adjust */
	pcie_io_adjust(pcie);

	pcie_config_axi_timeout(pcie);

	pcie_phy_por_n(pcie);

	/* sys_aux_pwr_det, perst */
	pcie_natural_cfg(pcie);

	ret = pcie_refclk_ctrl(pcie, ENABLE);
	if (ret)
		goto REF_CLK;

	ret = pcie_clk_config(pcie, PCIE_EXTERNAL_CLK, ENABLE);
	if (ret)
		goto EXTERNAL_CLK;

	ret = pcie_port_phy_init(pcie);
	if (ret) {
		PCIE_PR_E("PHY init Failed");
		goto PHY_INIT;
	}
	PCIE_PR_I("PHY init Done");

	pcie_ctrl_por_n_rst(pcie, DISABLE);

	/* Call EP poweron callback */
	PCIE_PR_I("Device +");
	if (pcie->callback_poweron && pcie->callback_poweron(pcie->callback_data)) {
		PCIE_PR_E("Failed: Device callback");
		ret = -1;
		goto DEV_EP_ON;
	}
	PCIE_PR_I("Device -");

	/* unrst tcu&tbu */
	pcie_tcu_tbu_rst_gt(pcie, DISABLE);

	if (!is_pipe_clk_stable(pcie)) {
		ret = -1;
		PCIE_PR_E("PIPE clk is not stable");
		pcie_phy_state(pcie);
		goto GPIO_DISABLE;
	}
	PCIE_PR_I("PIPE_clk is stable");

	if (!pcie_phy_ready(pcie)) {
		ret = -1;
		goto GPIO_DISABLE;
	}

	pcie_en_rx_term(pcie);

	ret = pcie_noc_power_jun(pcie, 1);
	if (ret) {
		PCIE_PR_E("Fail to exit noc idle");
		goto GPIO_DISABLE;
	}

	pcie_en_dbi_ep_splt(pcie);

	ret = pcie_smmu_init(pcie, ENABLE);
	if (ret) {
		PCIE_PR_E("SMMU init Failed");
		goto SMMU_INIT_FAIL;
	}
	PCIE_PR_I("SMMU init Done");

	flags = pcie_idle_spin_lock(pcie->idle_sleep, flags);
	atomic_add(1, &(pcie->is_power_on));
	pcie_idle_spin_unlock(pcie->idle_sleep, flags);

	pcie_l1ss_fixup(pcie);

	pcie_phy_irq_init(pcie);

#ifdef XW_PHY_MCU
	phy_core_on(pcie);
	udelay(50);
#endif

	/* pcie 32K idle */
	pcie_32k_idle_init(pcie->idle_sleep);

	PCIE_PR_I("-%s-", __func__);
	goto MUTEX_UNLOCK;

SMMU_INIT_FAIL:
	ret = pcie_noc_power_jun(pcie, 0);
	if (ret)
		PCIE_PR_E("Fail to enter noc idle");
GPIO_DISABLE:
	pcie_tcu_tbu_rst_gt(pcie, ENABLE);
	pcie_perst_cfg(pcie, DISABLE);
DEV_EP_ON:
	pcie_ctrl_por_n_rst(pcie, ENABLE);
PHY_INIT:
	(void)pcie_clk_config(pcie, PCIE_EXTERNAL_CLK, DISABLE);
EXTERNAL_CLK:
	(void)pcie_refclk_ctrl(pcie, DISABLE);
REF_CLK:
	pcie_reset_ctrl(pcie, RST_ENABLE);
	(void)pcie_clk_config(pcie, PCIE_INTERNAL_CLK, DISABLE);
INTERNAL_CLK:
	PCIE_PR_E("Failed to PowerOn");
MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

static int pcie_turn_off(struct pcie_kport *pcie, enum rc_power_status on_flag)
{
	int ret = 0;
	u32 val;
	unsigned long flags = 0;

	PCIE_PR_I("+%s+", __func__);

	mutex_lock(&pcie->power_lock);
	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_I("PCIe%u already power off", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}

	flags = pcie_idle_spin_lock(pcie->idle_sleep, flags);
	atomic_set(&(pcie->is_power_on), 0);
	pcie_idle_spin_unlock(pcie->idle_sleep, flags);

	/* pcie 32K idle */
	pcie_32k_idle_deinit(pcie->idle_sleep);

	pcie_phy_irq_deinit(pcie);

#ifdef XW_PHY_MCU
	phy_core_off(pcie);
#endif

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL10_ADDR);
	val |= AXI_TIMEOUT_MASK_BIT;
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL10_ADDR);

	ret = pcie_smmu_init(pcie,  DISABLE);
	if (ret)
		PCIE_PR_E("SMMU config Failed");

	/* Enter NOC Power Idle */
	ret = pcie_noc_power_jun(pcie, 0);
	if (ret)
		PCIE_PR_E("Fail to enter noc idle");

	pcie_tcu_tbu_rst_gt(pcie, ENABLE);

	PCIE_PR_I("Device +");
	if (pcie->callback_poweroff && pcie->callback_poweroff(pcie->callback_data))
		PCIE_PR_E("Failed: Device callback");
	PCIE_PR_I("Device -");

	pcie_ctrl_por_n_rst(pcie, ENABLE);

	/* rst controller perst_n */
	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val &= ~PCIE_PERST_CONFIG_MASK;
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	clk_disable_unprepare(pcie->pcie_aux_clk);
	clk_disable_unprepare(pcie->pcie_tbu_clk);
	clk_disable_unprepare(pcie->pcie_tcu_clk);
	clk_disable_unprepare(pcie->pcie_aclk);
	(void)pcie_refclk_ctrl(pcie, DISABLE);

	/* rst pcie_phy_apb_presetn pcie_ctrl_apb_presetn */
	pcie_reset_ctrl(pcie, RST_ENABLE);
	clk_disable_unprepare(pcie->apb_sys_clk);
	clk_disable_unprepare(pcie->apb_phy_clk);

	PCIE_PR_I("-%s-", __func__);
MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

#ifdef CONFIG_PCIE_KPORT_IDLE
static void pcie_ref_clk_on(struct pcie_kport *pcie)
{
	if (!atomic_read(&pcie->is_power_on))
		return;

	pcie_refclk_on(pcie);
}

static void pcie_ref_clk_off(struct pcie_kport *pcie)
{
	if (!atomic_read(&pcie->is_power_on))
		return;

	pcie_refclk_off(pcie);
}

static void pcie_pll_gate_status(struct pcie_kport *pcie)
{
	uint32_t pll_gate_reg = HSDT1PCIEPLL_STATE;

	if (pcie->rc_id == 0)
		pll_gate_reg = HSDTPCIEPLL_STATE;

	PCIE_PR_I("[PCIe%u] PCIEPLL STATE 0x%-8x: %8x",
		pcie->rc_id,
		pll_gate_reg,
		hsdt_crg_reg_read(pcie, pll_gate_reg));
}
#endif

struct pcie_platform_ops plat_ops = {
	.sram_ext_load = NULL,
	.plat_on = pcie_turn_on,
	.plat_off = pcie_turn_off,
#ifdef CONFIG_PCIE_KPORT_IDLE
	.ref_clk_on    = pcie_ref_clk_on,
	.ref_clk_off   = pcie_ref_clk_off,
	.pll_status     = pcie_pll_gate_status,
#endif
};

static int pcie_get_clk(struct platform_device *pdev, struct pcie_kport *pcie)
{
	pcie->pcie_tcu_clk = devm_clk_get(&pdev->dev, "pcie_tcu_clk");
	if (IS_ERR(pcie->pcie_tcu_clk)) {
		PCIE_PR_E("Failed to get pcie_tcu_clk");
		return PTR_ERR(pcie->pcie_tcu_clk);
	}

	pcie->pcie_tbu_clk = devm_clk_get(&pdev->dev, "pcie_tbu_clk");
	if (IS_ERR(pcie->pcie_tbu_clk)) {
		PCIE_PR_E("Failed to get pcie_tbu_clk");
		return PTR_ERR(pcie->pcie_tbu_clk);
	}

	pcie->pcie_refclk = devm_clk_get(&pdev->dev, "pcie_refclk");
	if (IS_ERR(pcie->pcie_refclk)) {
		PCIE_PR_E("Failed to get pcie_refclk");
		return PTR_ERR(pcie->pcie_refclk);
	}

	return 0;
}

int pcie_plat_init(struct platform_device *pdev, struct pcie_kport *pcie)
{
	struct device_node *np = NULL;

	if (pcie_get_clk(pdev, pcie))
		return -1;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,hsdt_crg");
	if (!np) {
		PCIE_PR_E("Failed to get hsdt-crg Node");
		return -1;
	}
	pcie->crg_base = of_iomap(np, 0);
	if (!pcie->crg_base) {
		PCIE_PR_E("Failed to iomap hsdt_base");
		return -1;
	}

	if (pcie->rc_id == PCIE_RC0)
		pcie->tbu_base = ioremap(PCIE0_TBU_BASE, PCIE_TBU_SIZE);
	else
		pcie->tbu_base = ioremap(PCIE1_TBU_BASE, PCIE_TBU_SIZE);

	if (!pcie->tbu_base) {
		PCIE_PR_E("Failed to iomap tbu_base");
		goto CRG_BASE_UNMAP;
	}

	pcie->plat_ops = &plat_ops;

	return 0;

CRG_BASE_UNMAP:
	iounmap(pcie->crg_base);
	return -1;
}
