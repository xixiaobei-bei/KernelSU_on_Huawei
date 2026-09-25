/*
 * pcie-sep.c
 *
 * PCIe Sep turn-on/off functions
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "pcie-kirin.h"
#include "pcie-kirin-common.h"
#include "pcie-phy-firmware-sep.h"

/* PCIE CTRL Register bit definition */
#define PCIE_PERST_CONFIG_MASK			0x4

/* PMC register offset */
#define NOC_POWER_IDLEREQ			0x380
#define NOC_POWER_IDLE				0x388

/* PMC register bit definition */
#define NOC_PCIE0_POWER				(0x1 << 10)
#define NOC_PCIE0_POWER_MASK			(0x1 << 26)

/* HSDT register offset */
#define HSDTCRG_PEREN1				0x10
#define HSDTCRG_PERDIS1				0x14
#define PCIEPLL_STATE				0x208
#define PLL_CFG0				0x224
#define PLL_CFG1				0x228
#define PLL_CFG2				0x22C
#define PLL_CFG3				0x230
#define PLL_CFG4				0x234
#define PLL_CFG5				0x238
#define PLL_CFG6				0x23C
#define PLL_CFG7				0x240
#define HSDTCRG_PCIECTRL0			0x300

/* HSDT register bit definition */
#define PCIE0_CLK_HP_GATE			(0x1 << 1)
#define PCIE0_CLK_DEBUNCE_GATE			(0x1 << 2)
#define PCIE0_CLK_PHYREF_GATE			(0x1 << 6)
#define PCIE0_CLK_IO_GATE			(0x1 << 7)
#define PCIEIO_HW_BYPASS			(0x1 << 0)
#define PCIEPHY_REF_HW_BYPASS			(0x1 << 1)
#define PCIEIO_OE_EN_SOFT			(0x1 << 6)
#define PCIEIO_OE_POLAR				(0x1 << 9)
#define PCIEIO_OE_EN_HARD_BYPASS		(0x1 << 11)
#define PCIEIO_IE_EN_HARD_BYPASS		(0x1 << 27)
#define PCIEIO_IE_EN_SOFT			(0x1 << 28)
#define PCIEIO_IE_POLAR				(0x1 << 29)
#define MPLLA_FORCE_EN_MUX_SEL			(0x1 << 4)
#define PCIE_MPLLA_FORCE_EN_SEL			(0x1 << 30)

/* APB PHY register definition */
#define PHY_REF_USE_PAD				(0x1 << 8)
#define PHY_REF_USE_CIO_PAD			(0x1 << 14)

/* pll Bit */
#define FNPLL_EN_BIT				(0x1 << 0)
#define FNPLL_BP_BIT				(0x1 << 1)
#define FNPLL_LOCK_BIT				(0x1 << 4)

#define IO_CLK_SEL_CLEAR			(0x3 << 17)
#define IO_CLK_FROM_CIO				(0x0 << 17)
#define IO_CLK_FROM_PHY_MPLLA			(0x1 << 17)

#define NOC_TIMEOUT_VAL				1000
#define FNPLL_LOCK_TIMEOUT			200

#define GEN3_RELATED_OFF			0x890
#define GEN3_ZRXDC_NONCOMPL			(0x1 << 0)

static u32 hsdt_crg_reg_read(struct kirin_pcie *pcie, u32 reg)
{
	return readl(pcie->crg_base + reg);
}

static void hsdt_crg_reg_write(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	writel(val, pcie->crg_base + reg);
}

/*
 * exit or enter noc power idle
 *    exit:
 *      1 -- exit noc power idle
 *      0 -- enter noc power idle
 */
static int pcie_noc_power(struct kirin_pcie *pcie, u32 exit)
{
	u32 val;
	int timeout = NOC_TIMEOUT_VAL;
	u32 val_bits = NOC_PCIE0_POWER;
	const u32 mask_bits = NOC_PCIE0_POWER_MASK;

	/*
	 * bits in mask_bits set to write the bit
	 * if bit in val_bits is 0, exit noc power idle
	 * or enter noc power idle
	 */
	if (exit)
		val_bits = 0;

	writel(mask_bits | val_bits, pcie->pmctrl_base + NOC_POWER_IDLEREQ);
	val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
	while ((val & NOC_PCIE0_POWER) != val_bits) {
		udelay(1);
		if (timeout <= 0) {
			PCIE_PR_ERR("Failed to %s noc power idle state 0x%x",
				    (exit ? "exit" : "enter"), val);
			return -1;
		}
		timeout--;
		val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
	}

	return 0;
}

/* config fnpll */
static int pcie_fnpll_ctrl(struct kirin_pcie *pcie, u32 enable)
{
	u32 val;
	int i = 0;
	const u32 pll_cfg6 = PLL_CFG6;
	const u32 pll_cfg7 = PLL_CFG7;
	const u32 pciepll_state = PCIEPLL_STATE;

	/* PCIE FNPLL: Set bypass and clear en bit */
	val = hsdt_crg_reg_read(pcie, pll_cfg6);
	val |= FNPLL_BP_BIT;
	val &= ~FNPLL_EN_BIT;
	hsdt_crg_reg_write(pcie, val, pll_cfg6);

	/* PCIe FNPLL CFG0~CFG5 stay default set */
	if (enable) {
		/*
		 * FNPLL_CFG6            [31:0]  = 32'h3A60_3E06
		 *   fnpll_pll_mode      [30]    = 'h0 (stay default set)
		 *   fnpll_dll_en        [29]    = 'h1 (stay default set)
		 *   fnpll_fout4phasepd  [28]    = 'h1 (stay default set)
		 *   fnpll_fout2xpd      [27]    = 'h1 (stay default set)
		 *   fnpll_foutpostdivpd [26]    = 'h0 (stay default set)
		 *   fnpll_postdiv2      [25:23] = 'h4
		 *   fnpll_postdiv1      [22:20] = 'h6
		 *   fnpll_fbdiv         [19:8]  = 'h3e
		 *   fnpll_refdiv        [7:2]   = 'h1
		 *   fnpll_bp            [1]     = 'h1 (set before)
		 *   fnpll_en            [0]     = 'h0 (set before)
		 */
		val = hsdt_crg_reg_read(pcie, pll_cfg6);
		val &= 0xFC000003;
		val |= 0x02603E04;
		hsdt_crg_reg_write(pcie, val, pll_cfg6);

		/*
		 * FNPLL_CFG7            [31:0]  = 32'h0080_0000
		 *   fnpll_pll_mode      [23:0]  = 32'h80_0000
		 */
		val = 0x00800000;
		hsdt_crg_reg_write(pcie, val, pll_cfg7);

		udelay(5);

		/* enable fnpll */
		val = hsdt_crg_reg_read(pcie, pll_cfg6);
		val |= FNPLL_EN_BIT;
		hsdt_crg_reg_write(pcie, val, pll_cfg6);

		udelay(20);

		val = hsdt_crg_reg_read(pcie, pciepll_state);
		while (i < FNPLL_LOCK_TIMEOUT) {
			if (val & FNPLL_LOCK_BIT) {
				val = hsdt_crg_reg_read(pcie, pll_cfg6);
				/* clear bypass */
				val &= ~FNPLL_BP_BIT;
				hsdt_crg_reg_write(pcie, val, pll_cfg6);
				PCIE_PR_INFO("FNPLL lock in %d us", i);
				return 0;
			}
			udelay(1);
			i++;
			val = hsdt_crg_reg_read(pcie, pciepll_state);
		}

		PCIE_PR_ERR("FNPLL unlock timeout:%d us!", FNPLL_LOCK_TIMEOUT);
		return -1;
	}

	return 0;
}

/* change clkreq_n signal to low */
static void pcie_config_clkreq_low(struct kirin_pcie *pcie)
{
	u32 val;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL22_ADDR);
	val &= ~PCIE_CLKREQ_OUT_MASK;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL22_ADDR);
}

/*
 * config gt_clk_pcieio_hard_bypass
 * enable:
 *     ENABLE  -- enable clkreq control phyio clk
 *     DISABLE -- disable clkreq control phyio clk
 */
static void pcie_phyio_hard_bypass(struct kirin_pcie *pcie, bool enable)
{
	u32 val;
	const u32 reg_addr = HSDTCRG_PCIECTRL0;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	if (enable)
		val &= ~PCIEIO_HW_BYPASS;
	else
		val |= PCIEIO_HW_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

/*
 * Config gt_clk_pciephy_ref_hard_bypass
 * enable:
 *     ENABLE  -- enable clkreq control phyref clk
 *     DISABLE -- disable clkreq control phyref clk
 */
static void pcie_phyref_hard_bypass(struct kirin_pcie *pcie, bool enable)
{
	u32 val;
	const u32 reg_addr = HSDTCRG_PCIECTRL0;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	if (enable)
		val &= ~PCIEPHY_REF_HW_BYPASS;
	else
		val |= PCIEPHY_REF_HW_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

/*
 * Config gt_clk_pciephy_ref_inuse
 * enable:
 *     ENABLE -- controlled by ~pcie_clkreq_in
 *     FALSE  -- clock down
 */
static void pcie_phy_ref_clk_gt(struct kirin_pcie *pcie, u32 enable)
{
	const u32 mask = PCIE0_CLK_PHYREF_GATE;
	const u32 reg = HSDTCRG_PERDIS1;

	/* HW bypass cfg */
	pcie_phyref_hard_bypass(pcie, enable);

	/* soft ref cfg, Always disable SW control */
	hsdt_crg_reg_write(pcie, mask, reg);
}

/*
 * enable:
 *     ENABLE  -- control by pcieio_oe_mux
 *     DISABLE -- close
 */
static void pcie_oe_config(struct kirin_pcie *pcie, bool enable)
{
	u32 val;
	const u32 reg_addr = HSDTCRG_PCIECTRL0;

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
 * enable:
 *     ENABLE  -- control by pcie_clkreq_in_n
 *     DISABLE -- close
 */
static void pcie_ie_config(struct kirin_pcie *pcie, bool enable)
{
	u32 val;
	const u32 reg_addr = HSDTCRG_PCIECTRL0;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	val &= ~PCIEIO_IE_POLAR;
	val &= ~PCIEIO_IE_EN_SOFT;
	if (enable)
		val &= ~PCIEIO_IE_EN_HARD_BYPASS;
	else
		val |= PCIEIO_IE_EN_HARD_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

/* ioref clock from mplla */
static void pcie_ioclk_from_phy(struct kirin_pcie *pcie, bool enable)
{
	u32 reg_val;
	const u32 reg_addr = HSDTCRG_PCIECTRL0;

	PCIE_PR_INFO("IO refclk from PHY");

	/* select mplla */
	reg_val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL21_ADDR);
	reg_val &= ~IO_CLK_SEL_CLEAR;
	reg_val |= IO_CLK_FROM_PHY_MPLLA;
	kirin_elb_writel(pcie, reg_val, SOC_PCIECTRL_CTRL21_ADDR);

	if (enable) {
		/* mplla force en */
		reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL34_ADDR);
		reg_val |= MPLLA_FORCE_EN_MUX_SEL;
		kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL34_ADDR);

		reg_val = hsdt_crg_reg_read(pcie, reg_addr);
		reg_val |= PCIE_MPLLA_FORCE_EN_SEL;
		hsdt_crg_reg_write(pcie, reg_val, reg_addr);
	}
}

/* ioref clock from fnpll */
static void pcie_ioclk_from_pll(struct kirin_pcie *pcie, u32 enable)
{
	u32 val;
	const u32 mask = PCIE0_CLK_IO_GATE;
	const u32 reg = HSDTCRG_PERDIS1;

	/* selcet cio */
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL21_ADDR);
	val &= ~IO_CLK_SEL_CLEAR;
	val |= IO_CLK_FROM_CIO;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL21_ADDR);

	/*
	 * HW bypass:
	 *     DISABLE: HW don't control
	 *     ENABLE : clkreq_n is one of control
	 */
	pcie_phyio_hard_bypass(pcie, ENABLE);

	/* disable SW control */
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

/* phyio clk configuration */
static void kirin_pcie_ioref_gt(struct kirin_pcie *pcie, bool enable)
{
	/* from pll as default */
	if (pcie->dtsinfo.ioref_clk_source == REFCLK_FROM_PHY) {
		PCIE_PR_INFO("CIO Use MPLLA");
		pcie_ioclk_from_phy(pcie, enable);
	} else {
		PCIE_PR_INFO("CIO Use FNPLL");
		pcie_ioclk_from_pll(pcie, enable);
	}
}

/* enable/disable hp&debounce clk */
static void pcie_hp_debounce_clk_gt(struct kirin_pcie *pcie, u32 enable)
{
	u32 reg;
	const u32 mask = PCIE0_CLK_HP_GATE | PCIE0_CLK_DEBUNCE_GATE;

	if (enable)
		reg = HSDTCRG_PEREN1;
	else
		reg = HSDTCRG_PERDIS1;
	hsdt_crg_reg_write(pcie, mask, reg);
}

/*
 * For RC, select FNPLL
 * For EP, select CIO
 * Select FNPLL
 */
static int kirin_pcie_refclk_on(struct kirin_pcie *pcie)
{
	u32 val;
	int ret;

	ret = pcie_fnpll_ctrl(pcie, ENABLE);
	if (ret)
		return ret;

	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val &= ~PHY_REF_USE_PAD;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	if (pcie->dtsinfo.ep_flag)
		val |= PHY_REF_USE_CIO_PAD;
	else
		val &= ~PHY_REF_USE_CIO_PAD;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

	/* enable pcie hp&debounce clk */
	pcie_hp_debounce_clk_gt(pcie, ENABLE);
	/* gate pciephy clk */
	pcie_phy_ref_clk_gt(pcie, ENABLE);
	/* gate pcieio clk */
	kirin_pcie_ioref_gt(pcie, ENABLE);

	return 0;
}

static void kirin_pcie_refclk_off(struct kirin_pcie *pcie)
{
	pcie_hp_debounce_clk_gt(pcie, DISABLE);
	kirin_pcie_ioref_gt(pcie, DISABLE);
	pcie_phy_ref_clk_gt(pcie, DISABLE);

	pcie_fnpll_ctrl(pcie, DISABLE);
}

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
static void pcie_ref_clk_on(struct kirin_pcie *pcie)
{
	if (!atomic_read(&pcie->is_power_on))
		return;

	if (atomic_cmpxchg(&(pcie->idle_sleep->ref_clk_onoff), 0, 1))
		return;

	/* enable pcie fnpll */
	(void)pcie_fnpll_ctrl(pcie, ENABLE);

	/* gate pciephy clk */
	pcie_phy_ref_clk_gt(pcie, ENABLE);

	/* gate pcieio clk */
	pcie_oe_config(pcie, ENABLE);

#ifdef CONFIG_PCIE_KIRIN_MNTN
	/* refclk turn on total counter */
	pcie->idle_sleep->ref_clk_on_cnt++;
#endif
}

static void pcie_ref_clk_off(struct kirin_pcie *pcie)
{
	if (!atomic_read(&pcie->is_power_on))
		return;

	if (!atomic_cmpxchg(&(pcie->idle_sleep->ref_clk_onoff), 1, 0))
		return;

	/* disable PCIE OE */
	pcie_oe_config(pcie, DISABLE);

	/* disable ref clk gt */
	pcie_phy_ref_clk_gt(pcie, DISABLE);

	/* wait fnpll lock */
	(void)pcie_fnpll_ctrl(pcie, DISABLE);

#ifdef CONFIG_PCIE_KIRIN_MNTN
	/* refclk turn off total counter */
	pcie->idle_sleep->ref_clk_off_cnt++;
#endif
}

static void kirin_pcie_idle_sleep_init(struct kirin_pcie *pcie)
{
	u32 val;

	if (!pcie->idle_sleep->idle_sleep_enable)
		goto SKIP_IDLE_SLEEP_INIT;

	/* enable pcie idle timeout int */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_timeout_int_addr);
	val &= ~(BIT(pcie->idle_sleep->idle_timeout_mask_bit));
	val |= BIT(pcie->idle_sleep->idle_timeout_en_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_timeout_int_addr);

	/* enable pcie idle wake int & mask */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_wake_int_addr);
	val |= BIT(pcie->idle_sleep->idle_wake_en_bit);
	val |= BIT(pcie->idle_sleep->idle_wake_mask_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_wake_int_addr);

	/* init pcie l1ss base cnt */
	if (pcie->idle_sleep->l1ss_cnt_base)
		kirin_elb_writel(pcie,
				 pcie->idle_sleep->l1ss_cnt_base,
				 pcie->idle_sleep->idle_wake_int_addr);
#ifdef CONFIG_PCIE_KIRIN_MNTN
	/* pcie fnpll state */
	pcie->idle_sleep->pciepll_state = PCIEPLL_STATE;
#endif

SKIP_IDLE_SLEEP_INIT:
	/* set refclk state to 1 */
	atomic_set(&(pcie->idle_sleep->ref_clk_onoff), 1);

	PCIE_PR_INFO("%s: idle sleep enable state %u",
		__func__, pcie->idle_sleep->idle_sleep_enable);
	PCIE_PR_INFO("%s: idle timeout and wake int state[0x%x]: 0x%x",
		__func__, pcie->idle_sleep->idle_timeout_int_addr,
		kirin_elb_readl(pcie, pcie->idle_sleep->idle_timeout_int_addr));
	PCIE_PR_INFO("%s: L1ss cnt base[0x%x]: 0x%x",
		__func__, pcie->idle_sleep->l1ss_cnt_base_addr,
		kirin_elb_readl(pcie, pcie->idle_sleep->l1ss_cnt_base_addr));
}

static void kirin_pcie_idle_sleep_deinit(struct kirin_pcie *pcie)
{
	u32 val;

	if (!pcie->idle_sleep->idle_sleep_enable)
		goto SKIP_IDLE_SLEEP_DEINIT;

	/* clear pcie idle timeout int */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_timeout_clr_addr);
	val |= BIT(pcie->idle_sleep->idle_timeout_clr_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_timeout_clr_addr);

	/* clear pcie idle wake int */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_wake_clr_addr);
	val |= BIT(pcie->idle_sleep->idle_wake_clr_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_wake_clr_addr);

	/* PCIe L1ss idle wake interrupts mask */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_wake_int_addr);
	val |= BIT(pcie->idle_sleep->idle_wake_mask_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_wake_int_addr);

	/* PCIe L1ss idle timeout interrupts mask */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_timeout_int_addr);
	val |= BIT(pcie->idle_sleep->idle_timeout_mask_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_timeout_int_addr);

SKIP_IDLE_SLEEP_DEINIT:
	/* clear refclk vote state */
	atomic_set(&(pcie->idle_sleep->ref_clk_onoff), 0);
	atomic_set(&(pcie->idle_sleep->ref_clk_vote), 0);
	atomic_set(&(pcie->idle_sleep->ref_clk_device_vote), 0);

#ifdef CONFIG_PCIE_KIRIN_MNTN
	/* clear refclk on&off cnt */
	pcie->idle_sleep->ref_clk_on_cnt = 0;
	pcie->idle_sleep->ref_clk_off_cnt = 0;
#endif
	PCIE_PR_INFO("%s: idle timeout and wake int state[0x%x]: 0x%x",
		__func__, pcie->idle_sleep->idle_timeout_int_addr,
		kirin_elb_readl(pcie, pcie->idle_sleep->idle_timeout_int_addr));
}
#endif

static void kirin_pcie_l1ss_fixup(struct kirin_pcie *pcie)
{
	u32 val;

	/* fix l1ss exit issue */
	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, GEN3_RELATED_OFF, 0x4);
	val &= ~GEN3_ZRXDC_NONCOMPL;
	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, GEN3_RELATED_OFF, 0x4, val);
}

static void kirin_pcie_en_dbi_ep_splt(struct kirin_pcie *pcie)
{
	u32 val;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL30_ADDR);
	val &= ~PCIE_DBI_EP_SPLT_BIT;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL30_ADDR);
}

static int kirin_pcie_turn_on(struct kirin_pcie *pcie, enum rc_power_status on_flag)
{
	int ret = 0;

	PCIE_PR_INFO("+%s+", __func__);

	mutex_lock(&pcie->power_lock);

	if (atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_INFO("PCIe%u already power on", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}

	/* pull down phy ISO */
	kirin_pcie_iso_ctrl(pcie, DISABLE);

	/* pclk for phy */
	ret = clk_prepare_enable(pcie->apb_phy_clk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable apb_phy_clk");
		goto APB_PHY_CLK;
	}

	/* pclk for ctrl */
	ret = clk_prepare_enable(pcie->apb_sys_clk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable apb_sys_clk");
		goto APB_SYS_CLK;
	}

	/* unset module */
	kirin_pcie_reset_ctrl(pcie, RST_DISABLE);

	/* adjust output refclk amplitude, currently no adjust */
	pcie_io_adjust(pcie);

	/* set clkreq low */
	pcie_config_clkreq_low(pcie);
	PCIE_PR_DEBUG("set clkreq low Done");

	kirin_pcie_config_axi_timeout(pcie);
	PCIE_PR_INFO("config axi_timeout Done");

	/* sys_aux_pwr_det, perst */
	kirin_pcie_natural_cfg(pcie);

	ret = kirin_pcie_refclk_on(pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to enable 100MHz ref_clk");
		goto REF_CLK;
	}
	PCIE_PR_INFO("100MHz refclks enable Done");

	/* Enable pcie axi clk */
	ret = clk_prepare_enable(pcie->pcie_aclk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable axi_aclk");
		goto AXI_ACLK;
	}

	/* enable pcie aux clk */
	ret = clk_prepare_enable(pcie->pcie_aux_clk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable aux_clk");
		goto AUX_CLK;
	}

	ret = kirin_pcie_phy_init(pcie);
	if (ret) {
		PCIE_PR_ERR("PHY init Failed");
		goto PHY_INIT;
	}
	PCIE_PR_INFO("PHY init Done");

	/* Call EP poweron callback */
	PCIE_PR_INFO("Device +");
	if (pcie->callback_poweron && pcie->callback_poweron(pcie->callback_data)) {
		PCIE_PR_ERR("Failed: Device callback");
		ret = -1;
		goto PHY_INIT;
	}
	PCIE_PR_INFO("Device -");

	if (!is_pipe_clk_stable(pcie)) {
		ret = -1;
		PCIE_PR_ERR("PIPE clk is not stable");
		goto GPIO_DISABLE;
	}
	PCIE_PR_INFO("PIPE_clk is stable");

	set_phy_eye_param(pcie);

	ret = pcie_noc_power(pcie, 1);
	if (ret) {
		PCIE_PR_ERR("Fail to exit noc idle");
		goto GPIO_DISABLE;
	}

	kirin_pcie_en_dbi_ep_splt(pcie);

	atomic_add(1, &(pcie->is_power_on));

	kirin_pcie_l1ss_fixup(pcie);

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
	kirin_pcie_idle_sleep_init(pcie);
#endif

	PCIE_PR_INFO("-%s-", __func__);
	goto MUTEX_UNLOCK;

GPIO_DISABLE:
	kirin_pcie_perst_cfg(pcie, DISABLE);
PHY_INIT:
	clk_disable_unprepare(pcie->pcie_aux_clk);
AUX_CLK:
	clk_disable_unprepare(pcie->pcie_aclk);
AXI_ACLK:
	kirin_pcie_refclk_off(pcie);
REF_CLK:
	kirin_pcie_reset_ctrl(pcie, RST_ENABLE);
	clk_disable_unprepare(pcie->apb_sys_clk);
APB_SYS_CLK:
	clk_disable_unprepare(pcie->apb_phy_clk);
APB_PHY_CLK:
	kirin_pcie_iso_ctrl(pcie, ENABLE);
	PCIE_PR_ERR("Failed to PowerOn");
MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

static int kirin_pcie_turn_off(struct kirin_pcie *pcie, enum rc_power_status on_flag)
{
	int ret = 0;
	u32 val;

	PCIE_PR_INFO("+%s+", __func__);

	mutex_lock(&pcie->power_lock);

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_INFO("PCIe%u already power off", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}
	atomic_set(&(pcie->is_power_on), 0);

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
	kirin_pcie_idle_sleep_deinit(pcie);
#endif

	/* mask pcie_axi_timeout */
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL10_ADDR);
	val |= AXI_TIMEOUT_MASK_BIT;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL10_ADDR);

	/* Enter NOC Power Idle */
	ret = pcie_noc_power(pcie, 0);
	if (ret)
		PCIE_PR_ERR("Fail to enter noc idle");

	PCIE_PR_INFO("Device +");
	if (pcie->callback_poweroff && pcie->callback_poweroff(pcie->callback_data))
		PCIE_PR_ERR("Failed: Device callback");
	PCIE_PR_INFO("Device -");

	/* rst controller perst_n */
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val &= ~PCIE_PERST_CONFIG_MASK;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	/* pull up phy_test_powerdown signal */
	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	val |= PHY_TEST_POWERDOWN;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

	clk_disable_unprepare(pcie->pcie_aux_clk);
	clk_disable_unprepare(pcie->pcie_aclk);
	kirin_pcie_refclk_off(pcie);

	/* rst pcie_phy_apb_presetn pcie_ctrl_apb_presetn pcie_ctrl_por_n */
	kirin_pcie_reset_ctrl(pcie, RST_ENABLE);

	clk_disable_unprepare(pcie->apb_sys_clk);
	clk_disable_unprepare(pcie->apb_phy_clk);

	kirin_pcie_iso_ctrl(pcie, ENABLE);

	PCIE_PR_INFO("-%s-", __func__);
MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

/* Load FW for PHY Fix */
static int pcie_phy_fw_fix_sep(void *data)
{
	struct kirin_pcie *pcie = data;

	return pcie_phy_fw_update(pcie, g_pcie_phy_data_sep, ARRAY_SIZE(g_pcie_phy_data_sep));
}

struct pcie_platform_ops plat_ops = {
	.plat_on       = kirin_pcie_turn_on,
	.plat_off      = kirin_pcie_turn_off,
	.sram_ext_load = pcie_phy_fw_fix_sep,
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
	.ref_clk_on    = pcie_ref_clk_on,
	.ref_clk_off   = pcie_ref_clk_off,
#endif
};

/* entry */
int pcie_plat_init(struct platform_device *pdev, struct kirin_pcie *pcie)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,hsdt-crg");
	if (!np) {
		PCIE_PR_ERR("Failed to get hsdt-crg Node");
		return -1;
	}

	pcie->crg_base = of_iomap(np, 0);
	if (!pcie->crg_base) {
		PCIE_PR_ERR("Failed to iomap hsdt_base");
		return -1;
	}
	pcie->plat_ops = &plat_ops;

	return 0;
}
