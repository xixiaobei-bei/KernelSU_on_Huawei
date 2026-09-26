/*
 * pcie-mar.c
 *
 * PCIe mar turn-on/off functions
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

#include "pcie-kport.h"
#include "pcie-kport-common.h"
#include "pcie-phy-firmware-mar.h"

/* MMC1 sysctrl registers */
#define MMC1_PLL_CTRL0_SYSCTRL		0x010
#define MMC1_PLL_CTRL1_SYSCTRL		0x014
#define MMC1_PCLECTRL0_SYSCTRL		0x200
#define MMC1_PEREN0_SYSCTRL		0x300
#define MMC1_PERDIS0_SYSCTRL		0x304
#define MMC1_PCIEPLL_STAT_SYSCTRL	0x0A0

#define GT_CLK_PCIE_HP			(0x1 << 6)
#define GT_CLK_PCIE_DEBOUNCE		(0x1 << 8)

#define OE_SOFT_VOLT			(0x1 << 6)
#define OE_POLAR			(0x1 << 9)
#define OE_HW_BYPASS			(0x1 << 11)
#define IE_HW_BYPASS			(0x1 << 27)
#define IE_SOFT_VOLT			(0x1 << 28)
#define IE_POLAR			(0x1 << 29)

#define PLL_UNLOCK_DETECT_INTR_BYPSS	0x924
#define FNPLL_UNLOCK_INTR_BYPASS	(0x1 << 16)

#define PHY_REF_USE_CIO_PAD  (0x1 << 14)
#define PHY_REF_USE_PAD      (0x1 << 8)

/* Load FW for PHY Fix */
static int pcie_phy_fw_fix_mar(void *data)
{
	struct pcie_kport *pcie = (struct pcie_kport *)data;

	return pcie_phy_fw_update(pcie, g_pcie_phy_data_mar, ARRAY_SIZE(g_pcie_phy_data_mar));
}

#define PLL_FN_SEL		(0x1 << 27)
#define FNPLL_EN_CFG_BIT	(0x1 << 0)
#define FNPLL_BP_CFG_BIT	(0x1 << 1)
#define FNPLL_LOCK_STAT_BIT	(0x1 << 0)
#define FNPLL_LOCK_TIMEOUT	200
static void pcie_fnpll_reg_config(struct pcie_kport *pcie)
{
	u32 reg_val;

	reg_val = readl(pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);
	/* refdiv1[7:2]--0x1 */
	reg_val &= ~(0x3F << 2);
	reg_val |= (0x1 << 2);
	/* fbdiv[19:8]--0x3e */
	reg_val &= ~(0xFFF << 8);
	reg_val |= (0x3e << 8);
	/* postdiv1[22:20]--0x4 */
	reg_val &= ~(0x7 << 20);
	reg_val |= (0x4 << 20);
	/* postdiv2[25:23]--0x6 */
	reg_val &= ~(0x7 << 23);
	reg_val |= (0x6 << 23);
	/* Foutpostdivpd[26]--0x0 */
	reg_val &= ~(0x1 << 26);
	/* Fout2xpd[27]--0x1 */
	reg_val &= ~(0x1 << 27);
	reg_val |= (0x1 << 27);
	/* Fout4phasepd[28]--0x1 */
	reg_val &= ~(0x1 << 28);
	reg_val |= (0x1 << 28);
	/* Dll_en[29]--0x1 */
	reg_val &= ~(0x1 << 29);
	reg_val |= (0x1 << 29);
	/* Pll_mode[30]--0x0 */
	reg_val &= ~(0x1 << 30);
	writel(reg_val, pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);

	/* fracdiv[23:0]-0x800000 */
	reg_val = readl(pcie->crg_base + MMC1_PLL_CTRL1_SYSCTRL);
	reg_val &= ~0xFFFFFF;
	reg_val |= 0x800000;
	writel(reg_val, pcie->crg_base + MMC1_PLL_CTRL1_SYSCTRL);
	udelay(5);

	/* Set en bit */
	reg_val = readl(pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);
	reg_val |= FNPLL_EN_CFG_BIT;
	writel(reg_val, pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);
}

static int pcie_pll_ctrl(struct pcie_kport *pcie, bool enable)
{
	u32 reg_val;
	u32 index = FNPLL_LOCK_TIMEOUT;

	PCIE_PR_I("fnpll init");

	/* choose fnpll */
	reg_val = readl(pcie->crg_base + MMC1_PLL_CTRL1_SYSCTRL);
	reg_val &= ~PLL_FN_SEL;
	writel(reg_val, pcie->crg_base + MMC1_PLL_CTRL1_SYSCTRL);

	/* set Bypass bit */
	reg_val = readl(pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);
	reg_val |= FNPLL_BP_CFG_BIT;
	writel(reg_val, pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);

	/* clear en bit */
	reg_val = readl(pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);
	reg_val &= ~FNPLL_EN_CFG_BIT;
	writel(reg_val, pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);

	if (enable) {
		pcie_fnpll_reg_config(pcie);

		reg_val = readl(pcie->crg_base + MMC1_PCIEPLL_STAT_SYSCTRL);
		while (index) {
			if (reg_val & FNPLL_LOCK_STAT_BIT) {
				PCIE_PR_I("fnpll lock in %u us", (FNPLL_LOCK_TIMEOUT - index));

				/* clear Bypass bit */
				reg_val = readl(pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);
				reg_val &= ~FNPLL_BP_CFG_BIT;
				writel(reg_val, pcie->crg_base + MMC1_PLL_CTRL0_SYSCTRL);

				return 0;
			}
			index--;
			udelay(1);
			reg_val = readl(pcie->crg_base + MMC1_PCIEPLL_STAT_SYSCTRL);
		}

		PCIE_PR_E("Failed to wait fnpll lock %d us", FNPLL_LOCK_TIMEOUT);
		return -1;
	}

	return 0;
}

static void pcie_hp_debounce_gt(struct pcie_kport *pcie, bool enable)
{
	u32 reg_val;
	u32 reg_offset;

	if (enable)
		reg_offset = MMC1_PEREN0_SYSCTRL;
	else
		reg_offset = MMC1_PERDIS0_SYSCTRL;

	reg_val = readl(pcie->crg_base + reg_offset);
	reg_val |= GT_CLK_PCIE_HP | GT_CLK_PCIE_DEBOUNCE;
	writel(reg_val, pcie->crg_base + reg_offset);
}

static void pcie_phyref_gt(struct pcie_kport *pcie, bool enable)
{
	u32 reg_val;

	/* HW bypass cfg */
	reg_val = readl(pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);
	/* gt_clk_pciephy_ref_hard_bypass : 1 - bypass, 0 - enable */
	if (enable)
		reg_val &= ~(0x1 << 1);
	else
		reg_val |= (0x1 << 1);
	writel(reg_val, pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);

	/* ST volt disable */
	reg_val = readl(pcie->crg_base + MMC1_PERDIS0_SYSCTRL);
	reg_val |= (0x1 << 5); /* gt_clk_pciephy_ref_soft */
	writel(reg_val, pcie->crg_base + MMC1_PERDIS0_SYSCTRL);
}

static void pcie_oe_cfg(struct pcie_kport *pcie, bool enable)
{
	u32 reg_val;

	reg_val = readl(pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);

	/* oe_en_HW_bypass cfg */
	if (enable)
		reg_val &= ~OE_HW_BYPASS;
	else
		reg_val |= OE_HW_BYPASS;

	reg_val &= ~OE_SOFT_VOLT;
	reg_val &= ~OE_POLAR;

	/* gt_mode: oe-->gt(open), gt-->oe(close) */
	reg_val |= (0x2 << 7);

	/* debounce use default, adjust if need */
	writel(reg_val, pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);
}

static void pcie_ie_cfg(struct pcie_kport *pcie, bool enable)
{
	u32 reg_val;

	reg_val = readl(pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);

	/* oe_en_HW_bypass cfg */
	if (enable)
		reg_val |= IE_HW_BYPASS;
	else
		reg_val &= ~IE_HW_BYPASS;

	reg_val &= ~IE_SOFT_VOLT;
	reg_val |= IE_POLAR;

	writel(reg_val, pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);
}

#define IO_CLK_SEL_CLEAR	(0x3 << 17)
#define IO_CLK_FROM_PLL		(0x0 << 17)
#define IO_CLK_FROM_PHY		(0x1 << 17)
#define IO_HW_BYPASS		(0x1 << 0)
#define IO_SOFT_VOLT		(0x1 << 7)
static void pcie_ioref_gt(struct pcie_kport *pcie, bool enable)
{
	u32 reg_val;

	PCIE_PR_I("IO refclk from PLL");

	/* set mux to PLL */
	reg_val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL21_ADDR);
	reg_val &= ~IO_CLK_SEL_CLEAR;
	reg_val |= IO_CLK_FROM_PLL;
	pcie_apb_ctrl_writel(pcie, reg_val, SOC_PCIECTRL_CTRL21_ADDR);

	/* HW bypass: clear when poweron, set when poweroff */
	reg_val = readl(pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);
	if (enable)
		reg_val &= ~IO_HW_BYPASS;
	else
		reg_val |= IO_HW_BYPASS;
	writel(reg_val, pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);

	/* soft volt disable */
	reg_val = readl(pcie->crg_base + MMC1_PERDIS0_SYSCTRL);
	reg_val |= IO_SOFT_VOLT;
	writel(reg_val, pcie->crg_base + MMC1_PERDIS0_SYSCTRL);

	/*
	 * RC: close IE forever, open/close OE by clk-req,
	 * EP: close OE forever, open/close IE by clk-req
	 */
	if (unlikely(pcie->dtsinfo.ep_flag)) {
		pcie_oe_cfg(pcie, DISABLE);
		pcie_ie_cfg(pcie, enable);
	} else {
		pcie_oe_cfg(pcie, enable);
		pcie_ie_cfg(pcie, DISABLE);
	}
}

/*
 * 100MHz clk src: Bit[8]==1 pad, Bit[8]==0 pll
 *                 Bit[14] == 1 from pll for RC,
 *                 Bit[14] == 0 from IO for EP.
 */
static int pcie_refclk_ctrl(struct pcie_kport *pcie, bool clk_on)
{
	u32 val;

	if (clk_on) {
		/* ref_use_pad */
		val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
		if (pcie->dtsinfo.ep_flag)
			val |= PHY_REF_USE_PAD;
		else
			val &= ~PHY_REF_USE_PAD;
		pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

		/* phy_ref_use_cio_pad */
		val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
		if (pcie->dtsinfo.ep_flag)
			val |= PHY_REF_USE_CIO_PAD;
		else
			val &= ~PHY_REF_USE_CIO_PAD;
		pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

		if (pcie_pll_ctrl(pcie, ENABLE)) {
			PCIE_PR_E("Failed to enable pll");
			return -1;
		}

		/* set debounce, and refclk auto-get to HW(clkreq) */
		pcie_hp_debounce_gt(pcie, ENABLE);
		pcie_phyref_gt(pcie, ENABLE);
		pcie_ioref_gt(pcie, ENABLE);
	} else {
		pcie_hp_debounce_gt(pcie, DISABLE);
		pcie_ioref_gt(pcie, DISABLE);
		pcie_phyref_gt(pcie, DISABLE);

		/* close and uninit PLL */
		pcie_pll_ctrl(pcie, DISABLE);
	}

	return 0;
}

static int pcie_clk_config(struct pcie_kport *pcie, enum pcie_clk_type clk_type, u32 enable)
{
	int ret = 0;

	switch (clk_type) {
	case PCIE_INTERNAL_CLK:
		if (enable) {
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
		} else {
			clk_disable_unprepare(pcie->apb_sys_clk);
			clk_disable_unprepare(pcie->apb_phy_clk);
		}
		break;
	case PCIE_EXTERNAL_CLK:
		if (enable) {
			/* axi_aclk */
			ret = clk_set_rate(pcie->pcie_aclk, AXI_CLK_FREQ);
			if (ret) {
				PCIE_PR_E("Failed to set rate");
				break;
			}

			ret = clk_prepare_enable(pcie->pcie_aclk);
			if (ret) {
				PCIE_PR_E("Failed to enable axi_aclk");
				break;
			}

			/* ctrl_aux_clk */
			ret = clk_prepare_enable(pcie->pcie_aux_clk);
			if (ret) {
				clk_disable_unprepare(pcie->pcie_aclk);
				PCIE_PR_E("Failed to enable aux_clk");
				break;
			}
		} else {
			clk_disable_unprepare(pcie->pcie_aux_clk);
			clk_disable_unprepare(pcie->pcie_aclk);
		}
		break;
	default:
		PCIE_PR_E("Invalid input parameters");
		ret = -EINVAL;
	}

	return ret;
}

static int pcie_turn_on(struct pcie_kport *pcie, enum rc_power_status on_flag)
{
	int ret = 0;

	PCIE_PR_I("+%s+", __func__);

	mutex_lock(&pcie->power_lock);

	if (atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_I("PCIe%u already power on", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}

	/* pull down phy ISO */
	pcie_iso_ctrl(pcie, DISABLE);

	/* rst pcie_phy_apb_presetn pcie_ctrl_apb_presetn pcie_ctrl_por_n */
	pcie_reset_ctrl(pcie, RST_ENABLE);

	/* pclk for phy & ctrl */
	ret = pcie_clk_config(pcie, PCIE_INTERNAL_CLK, ENABLE);
	if (ret)
		goto INTERNAL_CLK;

	/* unset module */
	pcie_reset_ctrl(pcie, RST_DISABLE);

	pcie_io_adjust(pcie);

	/* sys_aux_pwr_det, perst */
	pcie_natural_cfg(pcie);

	ret = pcie_refclk_ctrl(pcie, ENABLE);
	if (ret) {
		PCIE_PR_E("Failed to enable 100MHz ref_clk");
		goto REF_CLK;
	}
	PCIE_PR_I("100MHz refclks enable Done");

	ret = pcie_clk_config(pcie, PCIE_EXTERNAL_CLK, ENABLE);
	if (ret)
		goto EXTERNAL_CLK;

	ret = pcie_port_phy_init(pcie);
	if (ret) {
		PCIE_PR_E("PHY init Failed");
		goto PHY_INIT;
	}
	PCIE_PR_I("PHY init Done");

	/* unrst EP */
	PCIE_PR_I("Device +");
	if (pcie->callback_poweron && pcie->callback_poweron(pcie->callback_data)) {
		PCIE_PR_E("Failed: Device callback");
		ret = -1;
		goto PHY_INIT;
	}
	PCIE_PR_I("Device -");

	if (!is_pipe_clk_stable(pcie)) {
		ret = -1;
		PCIE_PR_E("PIPE clk is not stable");
		goto GPIO_DISABLE;
	}
	PCIE_PR_I("PIPE_clk is stable");

	set_phy_eye_param(pcie);

	ret = pcie_noc_power(pcie, DISABLE);
	if (ret)
		goto GPIO_DISABLE;

	atomic_add(1, &(pcie->is_power_on));

	PCIE_PR_I("-%s-", __func__);
	goto MUTEX_UNLOCK;

GPIO_DISABLE:
	pcie_perst_cfg(pcie, DISABLE);
PHY_INIT:
	(void)pcie_clk_config(pcie, PCIE_EXTERNAL_CLK, DISABLE);
EXTERNAL_CLK:
	pcie_refclk_ctrl(pcie, DISABLE);
REF_CLK:
	pcie_reset_ctrl(pcie, RST_ENABLE);
	(void)pcie_clk_config(pcie, PCIE_INTERNAL_CLK, DISABLE);
INTERNAL_CLK:
	pcie_iso_ctrl(pcie, ENABLE);
	PCIE_PR_E("Failed to PowerOn");
MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

static int pcie_turn_off(struct pcie_kport *pcie, enum rc_power_status on_flag)
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

	ret = pcie_noc_power(pcie, ENABLE);
	if (ret)
		PCIE_PR_E("Failed to set noc idle");

	PCIE_PR_I("Device +");
	if (pcie->callback_poweroff && pcie->callback_poweroff(pcie->callback_data))
		PCIE_PR_E("Failed: Device callback");
	PCIE_PR_I("Device -");

	/* rst controller perst_n */
	/* SOC_PCIECTRL_CTRL12[3:2]--pcie_perst_in_n_ctrl */
	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val &= ~(0x1 << 2);
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	/* close SIGDET modules */
	/* RAWAONLANEN_DIG_RX_OVRD_OUT_3[5:0]--0x2A */
	val = pcie_natural_phy_readl(pcie, 0x4035);
	val &= ~0x3F;
	val |= 0x2A;
	pcie_natural_phy_writel(pcie, val, 0x4035);

	/* pull up phy_test_powerdown signal */
	val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	val |= PHY_TEST_POWERDOWN;
	pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

	clk_disable_unprepare(pcie->pcie_aux_clk);
	clk_disable_unprepare(pcie->pcie_aclk);
	pcie_refclk_ctrl(pcie, DISABLE);

	clk_disable_unprepare(pcie->apb_phy_clk);
	clk_disable_unprepare(pcie->apb_sys_clk);

	pcie_iso_ctrl(pcie, ENABLE);

	PCIE_PR_I("-%s-", __func__);

MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

struct pcie_platform_ops plat_ops = {
	.sram_ext_load = pcie_phy_fw_fix_mar,
	.plat_on = pcie_turn_on,
	.plat_off = pcie_turn_off,
};

int pcie_plat_init(struct platform_device *pdev, struct pcie_kport *pcie)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,mmc1_sysctrl");
	if (!np) {
		PCIE_PR_E("Failed to get mmc1_ctrl node");
		return -1;
	}
	pcie->crg_base = of_iomap(np, 0);
	if (!pcie->crg_base) {
		PCIE_PR_E("Failed to iomap mmc1_base iomap");
		return -1;
	}

	pcie->plat_ops = &plat_ops;

	return 0;
}
