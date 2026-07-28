#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/stat.h>

#include "pcie-kirin.h"
#include "pcie-kirin-common.h"
#include "pcie_phy_firmware.h"

/*lint -e438 -e550 -e715 -e732 -e747 -e750 -e838 -esym(438,*) -esym(550,*) -esym(715,*) -esym(732,*) -esym(747,*) -esym(750,*) -esym(838,*) */

/* MMC1 sysctrl registers */
#define MMC1_PLL_CTRL0_SYSCTRL		0x010
#define MMC1_PLL_CTRL1_SYSCTRL		0x014
#define MMC1_FNPLL_CFG6_SYSCTRL		0x118
#define MMC1_FNPLL_CFG7_SYSCTRL		0x11C
#define MMC1_PCLECTRL0_SYSCTRL		0x200
#define MMC1_PEREN0_SYSCTRL		0x300
#define MMC1_PERDIS0_SYSCTRL		0x304
#define MMC1_PCIEPLL_STAT_SYSCTRL	0x0A0


#define GT_CLK_PCIE_HP			(0x1 << 6)
#define GT_CLK_PCIE_DEBOUNCE	(0x1 << 8)

#define OE_SOFT_VOLT	(0x1 << 6)
#define OE_POLAR		(0x1 << 9)
#define OE_HW_BYPASS	(0x1 << 11)
#define IE_HW_BYPASS	(0x1 << 27)
#define IE_SOFT_VOLT	(0x1 << 28)
#define IE_POLAR		(0x1 << 29)

#define PLL_UNLOCK_DETECT_INTR_BYPSS		0x924

enum {
	PLL_TYPE_FN = 1,
	PLL_TYPE_HP = 2,
};

enum {
	REFCLK_FROM_PHY = 1,
	REFCLK_FROM_PLL = 2,
};

static void kirin_pcie_voltage_ctrl(struct kirin_pcie *pcie)
{
	u32 reg_val;

	if (pcie->dtsinfo.chip_type == CHIP_TYPE_CS)
		return;

	PCIE_PR_DEBUG("Adjust PHY voltage");

	/* RAWAONLANEN_DIG_RX_ADPT_DFE_TAP4[0]--0x1 */
	reg_val = kirin_natural_phy_readl(pcie, 0x401f);
	reg_val |= 0x1;
	kirin_natural_phy_writel(pcie, reg_val, 0x401f);

	/* RAWAONLANEN_DIG_RX_ADPT_DFE_TAP3[0]--0x1 */
	reg_val = kirin_natural_phy_readl(pcie, 0x401e);
	reg_val |= 0x1;
	kirin_natural_phy_writel(pcie, reg_val, 0x401e);
}

static int pcie_phy_fw_fix(void *data)
{
	u32 index;
	u32 reg_addr, reg_val;
	struct kirin_pcie *pcie = (struct kirin_pcie *)data;

	/*lint -e679 -esym(679,*) -e661 -esym(661,*) -e662 -esym(662,*) */
	for (index = 0; index < (sizeof(g_pcie_phy_data)/sizeof(g_pcie_phy_data[0])/2); index++) {
		reg_addr = g_pcie_phy_data[2 * index];
		reg_val = g_pcie_phy_data[2 * index + 1];
		if (!reg_addr) {
			PCIE_PR_INFO("Update done: %d", index);
			break;
		}
		reg_addr -= 0xC000;
		kirin_sram_phy_writel(pcie, reg_val, reg_addr);
	}
	/*lint -e679 +esym(679,*) -e661 +esym(661,*) -e662 +esym(662,*) */

	/* adjust voltage */
	 kirin_pcie_voltage_ctrl(pcie);

	/* Vboost */
	reg_val = kirin_natural_phy_readl(pcie, 0x21);
	reg_val &= ~0xFFFF;
	reg_val |= 0xB5;
	kirin_natural_phy_writel(pcie, reg_val, 0x21);

	/* cdr_legacy_en */
	reg_val = kirin_apb_phy_readl(pcie, 0x258);
	reg_val |= 0x1;
	kirin_apb_phy_writel(pcie, reg_val, 0x258);

	pcie->plat_ops->cal_alg_adjust(pcie, false);

	return 0;
}

static void pcie_get_clk_source(struct kirin_pcie *pcie,
			struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo;
	struct device_node *np = pdev->dev.of_node;

	dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(np, "pll_source", &dtsinfo->pll_source);
	if (ret)
		dtsinfo->pll_source = PLL_TYPE_FN;

	ret = of_property_read_u32(np, "ioref_clk_source", &dtsinfo->ioref_clk_source);
	if (ret)
		dtsinfo->ioref_clk_source = REFCLK_FROM_PLL;

	PCIE_PR_DEBUG("Pll source:%s, Refclk from:%s",
			dtsinfo->pll_source == PLL_TYPE_FN ? "FN_pll": "HP_pll",
			dtsinfo->ioref_clk_source == REFCLK_FROM_PLL ? "FromPll": "FromPhy");
}

static void get_phy_cal_flag(struct kirin_pcie *pcie,
			struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;
	struct device_node *np = pdev->dev.of_node;

	ret = of_property_read_u32(np, "phy_cal", &dtsinfo->phy_cal);
	if (ret)
		dtsinfo->phy_cal = 0;

	PCIE_PR_DEBUG("Phy cal flag:%d", dtsinfo->phy_cal);
}

#define PLL_FN_SEL		(0x1 << 27)
#define FNPLL_EN_CFG_BIT	(0x1 << 0)
#define FNPLL_BP_CFG_BIT	(0x1 << 1)
#define FNPLL_LOCK_STAT_BIT	(0x1 << 0)
#define FNPLL_EN_STAT_BIT	(0x1 << 1)
static int kirin_pcie_fnpll_ctrl(struct kirin_pcie *pcie, bool enable)
{
	u32 reg_val;
	int index;

	if (pcie->dtsinfo.board_type == BOARD_FPGA)
		return 0;

	PCIE_PR_INFO("fnpll init");

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

		reg_val = readl(pcie->crg_base + MMC1_PCIEPLL_STAT_SYSCTRL);
		index = 200;
		while(index) {
			if (reg_val & FNPLL_LOCK_STAT_BIT) {
				PCIE_PR_INFO("fnpll locked(%d)", index);

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

		PCIE_PR_ERR("Failed to wait fnpll lock");
		return -1;
	}

	return 0;
}

#define HPPLL_EN_CFG_BIT	(0x1 << 0)
#define HPPLL_BP_CFG_BIT	(0x1 << 1)
#define HPPLL_LOCK_STAT_BIT	(0x1 << 4)
#define HPPLL_EN_STAT_BIT	(0x1 << 5)
static int kirin_pcie_hppll_ctrl(struct kirin_pcie *pcie, bool enable)
{
	u32 reg_val;
	int index;

	if (pcie->dtsinfo.board_type == BOARD_FPGA)
		return 0;

	PCIE_PR_INFO("hppll init(2.4-->1.8)");

	/* choose hppll */
	reg_val = readl(pcie->crg_base + MMC1_PLL_CTRL1_SYSCTRL);
	reg_val |= PLL_FN_SEL;
	writel(reg_val, pcie->crg_base + MMC1_PLL_CTRL1_SYSCTRL);

	/* set Bypass bit */
	reg_val = readl(pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);
	reg_val |= HPPLL_BP_CFG_BIT;
	writel(reg_val, pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);

	/* clear en bit */
	reg_val = readl(pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);
	reg_val &= ~HPPLL_EN_CFG_BIT;
	writel(reg_val, pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);

	if (enable) {
		reg_val = readl(pcie->crgperiph_base + PLL_UNLOCK_DETECT_INTR_BYPSS);
		reg_val |= (0x1 << 16);
		writel(reg_val, pcie->crgperiph_base + PLL_UNLOCK_DETECT_INTR_BYPSS);

		 /* config 2.4G */
		reg_val = readl(pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);
		/* refdiv[7:2]-0x1 */
		reg_val &= ~(0x3F << 2);
		reg_val |= (0x1 << 2);
		/* fbdiv[19:8]-0x3e */
		reg_val &= ~(0xFFF << 8);
		reg_val |= (0x3e << 8);
		/* postdiv1[22:20]-0x4 */
		reg_val &= ~(0x7 << 20);
		reg_val |= (0x4 << 20);
		/* postdiv2[25:23]-0x6 */
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
		writel(reg_val, pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);

		/* fracdiv[23:0]-0x800000 */
		reg_val = readl(pcie->crg_base + MMC1_FNPLL_CFG7_SYSCTRL);
		reg_val &= ~0xFFFFFF;
		reg_val |= 0x800000;
		writel(reg_val, pcie->crg_base + MMC1_FNPLL_CFG7_SYSCTRL);

		/* Set en bit */
		reg_val = readl(pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);
		reg_val |= HPPLL_EN_CFG_BIT;
		writel(reg_val, pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);
		udelay(10);

		/* config 1.8G */
		reg_val = readl(pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);
		/* refdiv[7:2]-0x1 */
		reg_val &= ~(0x3F << 2);
		reg_val |= (0x1 << 2);
		/* fbdiv[19:8]-0x2e */
		reg_val &= ~(0xFFF << 8);
		reg_val |= (0x2e << 8);
		/* postdiv1[22:20]-0x3 */
		reg_val &= ~(0x7 << 20);
		reg_val |= (0x3 << 20);
		/* postdiv2[25:23]-0x6 */
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
		writel(reg_val, pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);

		/* fracdiv[23:0]-0xe00000 */
		reg_val = readl(pcie->crg_base + MMC1_FNPLL_CFG7_SYSCTRL);
		reg_val &= ~0xFFFFFF;
		reg_val |= 0xe00000;
		writel(reg_val, pcie->crg_base + MMC1_FNPLL_CFG7_SYSCTRL);
		udelay(50);

		reg_val = readl(pcie->crgperiph_base + PLL_UNLOCK_DETECT_INTR_BYPSS);
		reg_val &= ~(0x1 << 16);
		writel(reg_val, pcie->crgperiph_base + PLL_UNLOCK_DETECT_INTR_BYPSS);
		udelay(1);

		reg_val = readl(pcie->crg_base + MMC1_PCIEPLL_STAT_SYSCTRL);
		index = 200;
		while(index) {
			if (reg_val & HPPLL_LOCK_STAT_BIT) {
				PCIE_PR_INFO("hppll locked(%d)", index);

				/* clear Bypass bit */
				reg_val = readl(pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);
				reg_val &= ~HPPLL_BP_CFG_BIT;
				writel(reg_val, pcie->crg_base + MMC1_FNPLL_CFG6_SYSCTRL);

				return 0;
			}
			index--;
			udelay(1);
			reg_val = readl(pcie->crg_base + MMC1_PCIEPLL_STAT_SYSCTRL);
		}

		PCIE_PR_ERR("Failed to wait hppll lock");
		return -1;
	}

	return 0;
}

/* fnPLL as default */
static int kirin_pcie_pll_ctrl(struct kirin_pcie *pcie, bool enable)
{
	if (pcie->dtsinfo.pll_source == PLL_TYPE_HP)
		return kirin_pcie_hppll_ctrl(pcie, enable);

	return kirin_pcie_fnpll_ctrl(pcie, enable);
}

static void kirin_pcie_hp_debounce_gt(struct kirin_pcie *pcie, bool enable)
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

static void kirin_pcie_phyref_gt(struct kirin_pcie *pcie, bool enable)
{
	u32 reg_val;

	/* HW bypass cfg */
	reg_val = readl(pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);
	if (enable)
		reg_val &= ~(0x1 << 1);
	else
		reg_val |= (0x1 << 1);
	writel(reg_val, pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);

	/* ST volt disable */
	reg_val = readl(pcie->crg_base + MMC1_PERDIS0_SYSCTRL);
	reg_val |= (0x1 << 5);
	writel(reg_val, pcie->crg_base + MMC1_PERDIS0_SYSCTRL);
}

static void kirin_pcie_oe_cfg(struct kirin_pcie *pcie, bool enable)
{
	u32 reg_val;

	reg_val = readl(pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);

	/* oe_en_HW_bypass cfg*/
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

static void kirin_pcie_ie_cfg(struct kirin_pcie *pcie, bool enable)
{
	u32 reg_val;

	reg_val = readl(pcie->crg_base + MMC1_PCLECTRL0_SYSCTRL);

	/* oe_en_HW_bypass cfg*/
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
static void ioref_from_pll(struct kirin_pcie *pcie, bool enable)
{
	u32 reg_val;

	PCIE_PR_INFO("IO refclk from PLL");

	/* set mux to PLL */
	reg_val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL21_ADDR);
	reg_val &= ~IO_CLK_SEL_CLEAR;
	reg_val |= IO_CLK_FROM_PLL;
	kirin_elb_writel(pcie, reg_val, SOC_PCIECTRL_CTRL21_ADDR);

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
		kirin_pcie_oe_cfg(pcie, DISABLE);
		kirin_pcie_ie_cfg(pcie, enable);
	} else {
		kirin_pcie_oe_cfg(pcie, enable);
		kirin_pcie_ie_cfg(pcie, DISABLE);
	}
}

#define MPLLA_DIV_CLK_offset	0x3
#define MPLLA_DIV_CLK_EN		(0x1 << 0)
#define MPLLA_DIV_MULTIPLIER	(0x19 << 1)
#define MPLLA_DIV_OVRD_EN		(0x1 << 9)
#define MPLLA_FORCE_EN			(0x1 << 0)
#define PIPE_LANE0_CLKREQ_SEL	(0x1 << 24)
static void ioref_from_phy(struct kirin_pcie *pcie, bool enable)
{
	u32 reg_val;

	PCIE_PR_INFO("IO refclk from PHY");

	/* set mux to PLL */
	reg_val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL21_ADDR);
	reg_val &= ~IO_CLK_SEL_CLEAR;
	reg_val |= IO_CLK_FROM_PHY;
	kirin_elb_writel(pcie, reg_val, SOC_PCIECTRL_CTRL21_ADDR);

	reg_val = kirin_natural_phy_readl(pcie, MPLLA_DIV_CLK_offset);
	reg_val |= (MPLLA_DIV_CLK_EN | MPLLA_DIV_MULTIPLIER | MPLLA_DIV_OVRD_EN);
	kirin_natural_phy_writel(pcie, reg_val, MPLLA_DIV_CLK_offset);

	if (enable) {
		/* mplla force en */
		reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL34_ADDR);
		reg_val |= MPLLA_FORCE_EN;
		kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL34_ADDR);

		/* link clkreq */
		reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
		reg_val |= PIPE_LANE0_CLKREQ_SEL;
		kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL0_ADDR);
	}
}

static void kirin_pcie_ioref_gt(struct kirin_pcie *pcie, bool enable)
{
	/* from PCIeIO as default */
	if (pcie->dtsinfo.ioref_clk_source == REFCLK_FROM_PHY)
		ioref_from_phy(pcie, enable);
	else
		ioref_from_pll(pcie, enable);
}

/*
*   100MHz clk src: Bit[8]==1 pad, Bit[8]==0 pll
*	Bit[14]== 1 from pll for RC,
*	Bit[14] == 0 from IO for EP.
*/
static int kirin_pcie_refclk_ctrl(struct kirin_pcie *pcie, bool clk_on)
{
	u32 val;

	if (clk_on) {
		/* ref_use_pad */
		val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
		if (pcie->dtsinfo.ep_flag)
			val |= 0x1 << 8;
		else
			val &= ~(0x1 << 8);
		kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

		/* phy_ref_use_cio_pad */
		val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
		if (pcie->dtsinfo.ep_flag)
			val |= (0x1 << 14);
		else
			val &= ~(0x1 << 14);
		kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

		if (kirin_pcie_pll_ctrl(pcie, ENABLE)) {
			PCIE_PR_ERR("Failed to enable pll");
			return -1;
		}

		/* set debounce, and refclk auto-get to HW(clkreq) */
		kirin_pcie_hp_debounce_gt(pcie, ENABLE);
		kirin_pcie_phyref_gt(pcie, ENABLE);
		kirin_pcie_ioref_gt(pcie, ENABLE);
	} else {
		kirin_pcie_hp_debounce_gt(pcie, DISABLE);
		kirin_pcie_ioref_gt(pcie, DISABLE);
		kirin_pcie_phyref_gt(pcie, DISABLE);

		/* close and uninit PLL */
		kirin_pcie_pll_ctrl(pcie, DISABLE);
	}

	return 0;
}

static int kirin_pcie_turn_on(struct pcie_port *pp, enum rc_power_status on_flag)
{
	int ret = 0;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);

	PCIE_PR_INFO("+ON+");

	mutex_lock(&pcie->power_lock);

	if (atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_INFO("PCIe%d already power on", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}

	/* pull down phy ISO */
	kirin_pcie_iso_ctrl(pcie, DISABLE);

	/* rst pcie_phy_apb_presetn pcie_ctrl_apb_presetn pcie_ctrl_por_n */
	kirin_pcie_reset_ctrl(pcie, RST_ENABLE);

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

	pcie_io_adjust(pcie);

	/* sys_aux_pwr_det, perst */
	kirin_pcie_natural_cfg(pcie);

	ret = kirin_pcie_refclk_ctrl(pcie, ENABLE);
	if (ret) {
		PCIE_PR_ERR("Failed to enable 100MHz ref_clk");
		goto REF_CLK;
	}
	PCIE_PR_INFO("100MHz refclks enable Done");

	/* axi_aclk */
	ret = clk_set_rate(pcie->pcie_aclk, AXI_CLK_FREQ);
	if (ret) {
		PCIE_PR_ERR("Failed to set rate");
		goto AXI_ACLK;
	}

	ret = clk_prepare_enable(pcie->pcie_aclk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable axi_aclk");
		goto AXI_ACLK;
	}

	/* ctrl_aux_clk */
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
	PCIE_PR_DEBUG("PHY init Done");

	/* unrst EP */
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

	ret = kirin_pcie_noc_power(pcie, DISABLE);
	if (ret) {
		goto GPIO_DISABLE;
	}

	atomic_add(1, &(pcie->is_power_on));

	PCIE_PR_INFO("-ON-");
	goto MUTEX_UNLOCK;

GPIO_DISABLE:
	kirin_pcie_perst_cfg(pcie, DISABLE);
PHY_INIT:
	clk_disable_unprepare(pcie->pcie_aux_clk);
AUX_CLK:
	clk_disable_unprepare(pcie->pcie_aclk);
AXI_ACLK:
	kirin_pcie_refclk_ctrl(pcie, DISABLE);
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

static int kirin_pcie_turn_off(struct pcie_port *pp, enum rc_power_status on_flag)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
	u32 val;
	int ret = 0;

	PCIE_PR_DEBUG("+OFF+");

	mutex_lock(&pcie->power_lock);

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_INFO("PCIe%d already power off", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}

	ret = kirin_pcie_noc_power(pcie, ENABLE);
	if (ret)
		PCIE_PR_ERR("Failed to set noc idle");

	PCIE_PR_DEBUG("Device +");
	if (pcie->callback_poweroff && pcie->callback_poweroff(pcie->callback_data)) {
		PCIE_PR_ERR("Failed: Device callback");
		ret = -1;
	}
	PCIE_PR_DEBUG("Device -");

	/* rst controller perst_n */
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val &= ~(0x1 << 2);
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	/* close SIGDET modules */
	/* RAWAONLANEN_DIG_RX_OVRD_OUT_3[5:0]--0x2A */
	val = kirin_natural_phy_readl(pcie, 0x4035);
	val &= ~0x3F;
	val |= 0x2A;
	kirin_natural_phy_writel(pcie, val, 0x4035);

	/* pull up phy_test_powerdown signal */
	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	val |= (0x1 << 22);
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

	clk_disable_unprepare(pcie->pcie_aux_clk);
	clk_disable_unprepare(pcie->pcie_aclk);
	kirin_pcie_refclk_ctrl(pcie, DISABLE);

	clk_disable_unprepare(pcie->apb_phy_clk);
	clk_disable_unprepare(pcie->apb_sys_clk);

	kirin_pcie_iso_ctrl(pcie, ENABLE);

	atomic_set(&(pcie->is_power_on), 0);
	PCIE_PR_DEBUG("-OFF-");

MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

static void phy_cal_adjust(void *data, bool restore)
{
	static u32 ori_rx_vco_val[4] = {0};
	static bool save_flag = false;
	struct kirin_pcie *pcie= (struct kirin_pcie *)data;
	u32 val = 0;
	u32 reg_val = 0;

	if (!pcie->dtsinfo.phy_cal)
		return;

	if (!restore) {
		val = kirin_natural_phy_readl(pcie, 0x1049);
		ori_rx_vco_val[0] = val;
		val |= 0x1 << 4;
		kirin_natural_phy_writel(pcie, val, 0x1049);

		val = kirin_natural_phy_readl(pcie, 0x1040);
		ori_rx_vco_val[1] = val;
		val &= ~(0x1 << 9);
		kirin_natural_phy_writel(pcie, val, 0x1040);

		val = kirin_natural_phy_readl(pcie, 0x1041);
		ori_rx_vco_val[2] = val;
		val &= ~(0x1 << 9);
		kirin_natural_phy_writel(pcie, val, 0x1041);

		val = kirin_natural_phy_readl(pcie, 0x1042);
		ori_rx_vco_val[3] = val;
		val &= ~(0x1 << 9);
		kirin_natural_phy_writel(pcie, val, 0x1042);
		save_flag = true;
		PCIE_PR_DEBUG("disable cal");
	} else {
		if (!save_flag)
			return;
		val = kirin_natural_phy_readl(pcie, 0x104D);
		val &= 0x3FF;
		reg_val = kirin_natural_phy_readl(pcie, 0x104A);
		reg_val &= ~0x3FF;
		reg_val |= val;
		reg_val |= 0x1 << 14;
		kirin_natural_phy_writel(pcie, reg_val,0x104A);

		kirin_natural_phy_writel(pcie, ori_rx_vco_val[1] ,0x1040);
		kirin_natural_phy_writel(pcie, ori_rx_vco_val[2] ,0x1041);
		kirin_natural_phy_writel(pcie, ori_rx_vco_val[3] ,0x1042);
		kirin_natural_phy_writel(pcie, ori_rx_vco_val[0] ,0x1049);
		save_flag = false;
		PCIE_PR_DEBUG("set freq and restore cal");
	}
}


struct pcie_platform_ops plat_ops = {
	.sram_ext_load = pcie_phy_fw_fix,
	.plat_on = kirin_pcie_turn_on,
	.plat_off = kirin_pcie_turn_off,
	.cal_alg_adjust = phy_cal_adjust,
};

int pcie_plat_init(struct platform_device *pdev, struct kirin_pcie *pcie)
{
	struct device_node *np;

	pcie_get_clk_source(pcie, pdev);
	get_phy_cal_flag(pcie, pdev);

	np = of_find_compatible_node(NULL, NULL, "hisilicon,mmc1_sysctrl");
	if (!np) {
		PCIE_PR_ERR("Failed to get mmc1_ctrl node");
		return -1;
	}
	pcie->crg_base = of_iomap(np, 0);
	if (!pcie->crg_base) {
		PCIE_PR_ERR("Failed to iomap mmc1_base iomap");
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
	if (!np) {
		PCIE_PR_ERR("Failed to get crgctrl node");
		return -1;
	}
	pcie->crgperiph_base = of_iomap(np, 0);
	if (!pcie->crgperiph_base) {
		PCIE_PR_ERR("Failed to iomap crgperiph_base iomap");
		return -1;
	}

	pcie->plat_ops = &plat_ops;

	return 0;
}

/*lint -e438 -e550 -e715 -e732 -e747 -e750 -e838 +esym(438,*) +esym(550,*) +esym(715,*) +esym(732,*) +esym(747,*) +esym(750,*) +esym(838,*) */
