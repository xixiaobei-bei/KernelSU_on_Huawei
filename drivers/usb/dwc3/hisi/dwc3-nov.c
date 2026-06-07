#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <pmic_interface.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/of_address.h>
#include <linux/hisi/usb/dwc3_usb_interface.h>
#include "dwc3-hisi.h"
#include "hisi_usb2_bc.h"
#include "hisi_usb_helper.h"

struct plat_hisi_usb_phy {
	struct hisi_usb_phy phy;
	void __iomem *pericfg_reg_base;
	void __iomem *sctrl_reg_base;
	void __iomem *pctrl_reg_base;

	struct clk *clk;
	struct clk *gt_hclk_usb3otg;
	struct clk *gt_clk_usb3_tcxo_en;
	struct clk *gt_clk_usb2phy_ref;
};

struct plat_hisi_usb_phy *_hisi_usb_phy;

/**
 * USB related register in PERICRG
 */
#define SCTRL_SCDEEPSLEEPED				(0x08)
#define PERI_CRG_PEREN0					(0x0)
#define GT_HCLK_USB2OTG					(1 << 6)
#define GT_CLK_USB2OTG_REF				(1 << 25)
#define PERI_CRG_PERDIS0				(0x04)
#define PERI_CRG_PERRSTEN0				(0x60)
#define PERI_CRG_PERRSTDIS0				(0x64)
#define IP_HRST_USB2OTG					(1 << 4)
#define IP_HRST_USB2OTG_AHBIF				(1 << 5)
#define IP_HRST_USB2OTG_MUX				(1 << 6)
#define IP_RST_USB2OTG_ADP				(1 << 25)
#define IP_RST_USB2OTG_BC				(1 << 26)
#define IP_RST_USB2OTG_PHY				(1 << 27)
#define	IP_RST_USB2PHY_POR				(1 << 28)

#define PERI_CRG_ISODIS					(0x148)
#define USB_REFCLK_ISO_EN				(1u << 25)

#define PERI_CRG_CLK_EN5				(0x50)
#define PERI_CRG_CLK_DIS5				(0x54)
#define GT_CLK_ABB_BACKUP				(1 << 22)

#define PERI_CRG_CLKDIV21				(0xFC)
#define SC_GT_CLK_ABB_SYS				(1 << 6)
#define SC_GT_CLK_ABB_PLL				(1 << 7)
#define SC_SEL_ABB_BACKUP				(1 << 8)
#define CLKDIV_MASK_START				(16)

#define PERI_CRG_CLKDIV24				(0x108)

#define PMC_PPLL3CTRL0					(0x048)
#define PPLL3_EN					(1 << 0)
#define PPLL3_BP					(1 << 1)
#define PPLL3_LOCK					(1 << 26)
#define PPLL3_FBDIV_START				(8)
#define PMC_PPLL3CTRL1					(0x04C)
#define PPLL3_INT_MOD					(1 << 24)
#define GT_CLK_PPLL3					(1 << 26)

/* PCTRL registers */
#define PCTRL_PERI_CTRL3				(0x10)
#define USB_TCXO_EN					(1 << 1)
#define PERI_CTRL3_MSK_START				(16)
#define PCTRL_PERI_CTRL24				(0x64)
#define SC_CLK_USB3PHY_3MUX1_SEL			(1 << 25)
/*lint -e750 -esym(750,*)*/

#define USBOTG3_CTRL2_VBUSVLDEXT	(1 << 3)
#define USBOTG3_CTRL2_VBUSVLDEXTSEL	(1 << 2)

#define USBOTG2_CTRL3			(0x0C)

static int is_abb_clk_selected(void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & (1 << 20)) == 0)
		return 1;

	return 0;
}

static int nov_get_dts_resource(struct device *dev, struct plat_hisi_usb_phy *usb_phy)
{
	void __iomem *regs;

	/* get hclk handler */
	usb_phy->gt_hclk_usb3otg = devm_clk_get(dev, "hclk_usb2otg");
	if (IS_ERR_OR_NULL(usb_phy->gt_hclk_usb3otg)) {
		dev_err(dev, "get hclk_usb2otg failed\n");
		return -EINVAL;
	}

	/* get abb clk handler */
	usb_phy->gt_clk_usb3_tcxo_en = devm_clk_get(dev, "clk_usb2_tcxo_en");
	if (IS_ERR_OR_NULL(usb_phy->gt_clk_usb3_tcxo_en)) {
		dev_err(dev, "get clk_usb2_tcxo_en failed\n");
		return -EINVAL;
	}
	/* get 32k usb2phy ref clk handler */
	usb_phy->gt_clk_usb2phy_ref = devm_clk_get(dev, "clk_usb2phy_ref");
	if (IS_ERR_OR_NULL(usb_phy->gt_clk_usb2phy_ref)) {
		dev_err(dev, "get clk_usb2phy_ref failed\n");
		return -EINVAL;
	}

	/* get otg bc */
	usb_phy->phy.otg_bc_reg_base = of_iomap(dev->of_node, 0);
	if (!usb_phy->phy.otg_bc_reg_base) {
		dev_err(dev, "ioremap res 0 failed\n");
		return -ENOMEM;
	}

	/*
	 * map PERI CRG region
	 */
	regs = of_devm_ioremap(dev, "hisilicon,crgctrl");
	if (IS_ERR(regs)) {
		usb_err("iomap peri cfg failed!\n");
		return PTR_ERR(regs);
	}
	usb_phy->pericfg_reg_base = regs;

	/*
	 * map SCTRL region
	 */
	regs = of_devm_ioremap(dev, "hisilicon,sysctrl");
	if (IS_ERR(regs)) {
		usb_err("iomap sctrl_reg_base failed!\n");
		return PTR_ERR(regs);
	}
	usb_phy->sctrl_reg_base = regs;

	/*
	 * map PCTRL region
	 */
	regs = of_devm_ioremap(dev, "hisilicon,pctrl");
	if (IS_ERR(regs)) {
		usb_err("iomap pctrl failed!\n");
		return PTR_ERR(regs);
	}
	usb_phy->pctrl_reg_base = regs;

	return 0;
}

static int dwc3_release(struct plat_hisi_usb_phy *usb_phy)
{
	void __iomem *pericrg_base = usb_phy->pericfg_reg_base;/*lint !e529 */
	void __iomem *otg_bc_base = usb_phy->phy.otg_bc_reg_base;
	void __iomem *pctrl_reg_base = usb_phy->pctrl_reg_base;
	void __iomem *sctrl_reg_base = usb_phy->sctrl_reg_base;
	volatile uint32_t temp;
	int ret;

	/* enable hclk  and 32k clk */
	ret = clk_prepare_enable(usb_phy->gt_hclk_usb3otg);
	if (ret) {
		usb_err("clk_prepare_enable gt_hclk_usb3otg failed\n");
		return ret;
	}

	ret = clk_prepare_enable(usb_phy->gt_clk_usb2phy_ref);
	if (ret) {
		usb_err("clk_prepare_enable gt_clk_usb2phy_ref failed\n");
		return ret;
	}

	/* unreset controller */
	writel(IP_RST_USB2OTG_ADP | IP_HRST_USB2OTG_MUX | IP_HRST_USB2OTG_AHBIF,
			pericrg_base + PERI_CRG_PERRSTDIS0);

	if (is_abb_clk_selected(sctrl_reg_base)) {
		/* open ABBISO */
		writel(USB_REFCLK_ISO_EN, pericrg_base + PERI_CRG_ISODIS);

		/* open clk */
		ret = clk_prepare_enable(usb_phy->gt_clk_usb3_tcxo_en);
		if (ret) {
			usb_err("clk_prepare_enable gt_clk_usb3_tcxo_en failed\n");
			return ret;
		}

		msleep(1);

		temp = readl(pctrl_reg_base + PCTRL_PERI_CTRL24);
		temp &= ~SC_CLK_USB3PHY_3MUX1_SEL;
		writel(temp,  pctrl_reg_base + PCTRL_PERI_CTRL24);
	} else {
		writel(GT_CLK_ABB_BACKUP, pericrg_base + PERI_CRG_CLK_DIS5);

		writel(SC_SEL_ABB_BACKUP | (SC_SEL_ABB_BACKUP << CLKDIV_MASK_START),
				pericrg_base + PERI_CRG_CLKDIV21);

		/* [15:10] set to 0x3F, with mask */
		writel(0xFC00FC00, pericrg_base + PERI_CRG_CLKDIV24);

		writel(SC_GT_CLK_ABB_PLL | (SC_GT_CLK_ABB_PLL << CLKDIV_MASK_START),
				pericrg_base + PERI_CRG_CLKDIV21);

		temp = readl(pctrl_reg_base + PCTRL_PERI_CTRL24);
		temp |= SC_CLK_USB3PHY_3MUX1_SEL;
		writel(temp, pctrl_reg_base + PCTRL_PERI_CTRL24);

		writel(GT_CLK_ABB_BACKUP, pericrg_base + PERI_CRG_CLK_EN5);
	}

	/* unreset phy */
	writel(IP_RST_USB2PHY_POR, pericrg_base + PERI_CRG_PERRSTDIS0);

	udelay(100);
	hisi_usb_unreset_phy_if_fpga();

	/* unreset phy clk domain */
	writel(IP_RST_USB2OTG_PHY, pericrg_base + PERI_CRG_PERRSTDIS0);

	udelay(100);

	/* unreset hclk domain */
	writel(IP_HRST_USB2OTG, pericrg_base + PERI_CRG_PERRSTDIS0);

	udelay(100);

	/* fake vbus valid signal */
	temp = readl(otg_bc_base + USBOTG3_CTRL2);
	temp |= (USBOTG3_CTRL2_VBUSVLDEXT | USBOTG3_CTRL2_VBUSVLDEXTSEL);
	writel(temp, otg_bc_base + USBOTG3_CTRL2);

	temp = readl(otg_bc_base + USBOTG3_CTRL0);
	temp |= (0x3 << 10);
	writel(temp, otg_bc_base + USBOTG3_CTRL0);

	msleep(1);

	return 0;

}

static void dwc3_reset(struct plat_hisi_usb_phy *usb_phy)
{
	void __iomem *pericrg_base = usb_phy->pericfg_reg_base;
	void __iomem *sctrl_reg_base = usb_phy->sctrl_reg_base;
	volatile uint32_t temp;

/*lint -e438 -esym(438,*)*/
/*lint -e529 -esym(529,*)*/
	/* reset controller */
	writel(IP_HRST_USB2OTG, pericrg_base + PERI_CRG_PERRSTEN0);

	/* reset phy */
	writel(IP_RST_USB2OTG_PHY, pericrg_base + PERI_CRG_PERRSTEN0);

	writel(IP_RST_USB2PHY_POR, pericrg_base + PERI_CRG_PERRSTEN0);

	if (is_abb_clk_selected(sctrl_reg_base)) {
		clk_disable_unprepare(usb_phy->gt_clk_usb3_tcxo_en);
	} else {
		/* close abb backup clk */
		writel(GT_CLK_ABB_BACKUP, pericrg_base + PERI_CRG_CLK_DIS5);

		temp = (SC_GT_CLK_ABB_PLL | SC_GT_CLK_ABB_SYS) << CLKDIV_MASK_START;
		writel(temp, pericrg_base + PERI_CRG_CLKDIV21);
	}

	/* reset usb controller */
	writel(IP_HRST_USB2OTG_AHBIF | IP_HRST_USB2OTG_MUX | IP_RST_USB2OTG_ADP,
			pericrg_base + PERI_CRG_PERRSTEN0);

	/* disable usb controller clk */
	clk_disable_unprepare(usb_phy->gt_hclk_usb3otg);
	clk_disable_unprepare(usb_phy->gt_clk_usb2phy_ref);

	msleep(1);
}

void set_usb2_eye_diagram_param(struct plat_hisi_usb_phy *usb_phy,
		unsigned int eye_diagram_param)
{
	void __iomem *otg_bc_base = usb_phy->phy.otg_bc_reg_base;

	if (hisi_dwc3_is_fpga())
		return;

	/* set high speed phy parameter */
	writel(eye_diagram_param, otg_bc_base + USBOTG2_CTRL3);
	usb_dbg("set hs phy param 0x%x for host\n",
			readl(otg_bc_base + USBOTG2_CTRL3));
}

/*lint -e438 +esym(438,*)*/
/*lint -e529 +esym(529,*)*/
static int nov_usb3phy_init(unsigned int support_dp,
		unsigned int eye_diagram_param,
		unsigned int usb3_phy_tx_vboost_lvl)
{
	int ret;

	usb_dbg("+\n");
	if (!_hisi_usb_phy)
		return -ENODEV;

	ret = dwc3_release(_hisi_usb_phy);
	if (ret) {
		usb_err("dwc3_release failed\n");
		return ret;
	}

	set_usb2_eye_diagram_param(_hisi_usb_phy, eye_diagram_param);

	set_hisi_dwc3_power_flag(1);

	usb_dbg("-\n");

	return 0;
}

static int nov_usb3phy_shutdown(unsigned int support_dp)
{
	usb_dbg("+\n");
	if (!_hisi_usb_phy)
		return -ENODEV;

	set_hisi_dwc3_power_flag(0);

	dwc3_reset(_hisi_usb_phy);
	usb_dbg("-\n");

	return 0;
}

static int nov_shared_phy_init(unsigned int support_dp, unsigned int eye_diagram_param,
		unsigned int combophy_flag)
{
	return nov_usb3phy_init(0, eye_diagram_param, 0);
}

static int nov_shared_phy_shutdown(unsigned int support_dp,
		unsigned int combophy_flag, unsigned int keep_power)
{
	return nov_usb3phy_shutdown(0);
}

static void chip_set_hi_impedance(void)
{
	struct usb_ahbif_registers *ahbif = NULL;
	union bc_ctrl4 bc_ctrl4;
	union usbotg2_ctrl0 usbotg2_ctrl0;

	usb_dbg("+\n");
	if (!_hisi_usb_phy)
		return;

	ahbif = (struct usb_ahbif_registers *)_hisi_usb_phy->phy.otg_bc_reg_base;

	bc_ctrl4.reg = readl(&ahbif->bc_ctrl4);
	bc_ctrl4.bits.bc_opmode = 1;
	writel(bc_ctrl4.reg, &ahbif->bc_ctrl4);

	usbotg2_ctrl0.reg = readl(&ahbif->usbotg2_ctrl0);
	usbotg2_ctrl0.bits.dppulldown = 0;
	usbotg2_ctrl0.bits.dmpulldown = 0;
	writel(usbotg2_ctrl0.reg, &ahbif->usbotg2_ctrl0);

	usb_dbg("-\n");
}

static int dwc3_nov_probe(struct platform_device *pdev)
{
	int ret;

	_hisi_usb_phy = devm_kzalloc(&pdev->dev, sizeof(*_hisi_usb_phy),
			GFP_KERNEL);
	if (!_hisi_usb_phy)
		return -ENOMEM;

	ret = nov_get_dts_resource(&pdev->dev, _hisi_usb_phy);
	if (ret) {
		usb_err("get_dts_resource failed\n");
		goto err_out;
	}

	_hisi_usb_phy->phy.init = nov_usb3phy_init;
	_hisi_usb_phy->phy.shutdown = nov_usb3phy_shutdown;
	_hisi_usb_phy->phy.shared_phy_init = nov_shared_phy_init;
	_hisi_usb_phy->phy.shared_phy_shutdown = nov_shared_phy_shutdown;
	_hisi_usb_phy->phy.set_hi_impedance = chip_set_hi_impedance;

	ret = hisi_usb_dwc3_register_phy(&_hisi_usb_phy->phy);
	if (ret) {
		usb_err("hisi_usb_dwc3_register_phy failed\n");
		goto err_out;
	}

	return 0;
err_out:
	_hisi_usb_phy = NULL;
	return ret;
}

static int dwc3_nov_remove(struct platform_device *pdev)
{
	int ret;

	ret = hisi_usb_dwc3_unregister_phy(&_hisi_usb_phy->phy);
	_hisi_usb_phy = NULL;

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id dwc3_nov_match[] = {
	{ .compatible = "hisilicon,nov-dwc3" },
	{},
};
MODULE_DEVICE_TABLE(of, dwc3_nov_match);
#else
#define dwc3_nov_match NULL
#endif

static struct platform_driver dwc3_nov_driver = {
	.probe		= dwc3_nov_probe,
	.remove		= dwc3_nov_remove,
	.driver		= {
		.name	= "usb3-nov",
		.of_match_table = of_match_ptr(dwc3_nov_match),
	},
};
/*lint +e715 +e716 +e717 +e835 +e838 +e845 +e533 +e835 +e778 +e774 +e747 +e785 +e438 +e529*/
/*lint -e64 -esym(64,*)*/
/*lint -e528 -esym(528,*)*/
module_platform_driver(dwc3_nov_driver);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL");
/*lint -e753 +esym(753,*)*/
/*lint -e64 +esym(64,*)*/
