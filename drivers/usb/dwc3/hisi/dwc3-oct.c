#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <pmic_interface.h>
#include <linux/mfd/hisi_pmic.h>
#include "dwc3-hisi.h"
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/hisi/usb/hisi_usb_interface.h>
#include <linux/hisi/usb/dwc3_usb_interface.h>
#include <linux/jiffies.h>
#include "common.h"


/*lint -e750 -esym(750,*)*/
#define CLK_FREQ_19_2M			19200000
#define CLK_FREQ_96_M			9600000

/* SCTRL register */
/* 0x8 */
#define SCTRL_SCDEEPSLEEPED                         (0x8)
/*bit20 */
#define USB_INIT_CLK_SEL					(1 << 20)
/* bit7 */
#define USB_20PHY_POWER_SEL				(1 << 7)

/* 0x8B4 */
#define SCTRL_MODEM_CTRL_DIS		(0x8B4)
/* bit0 */
#define ABB_DIG2ANA_ISO_EN			(1 << 0)


/* CRGPERIPH register */
/* 0x4c */
#define PERI_CRG_PERSTAT4					(0x4c)
/* bit 1 */
#define GT_HCLK_USB2DRD					(1 << 1)
/* bit 0 */
#define GT_CLK_USB2DRD_REF				(1 << 0)

/* 0x90 */
#define PERI_CRG_PERRSTEN4				(0x90)
/* 0x94 */
#define PERI_CRG_PERRSTDIS4				(0x94)
/* 0x98 */
#define PERI_CRG_PERRSTSTAT4				(0x98)
/* bit 23*/
#define IP_RST_USB2DRD_PHY			 (1 << 23)
/* bit 16*/
#define IP_HRST_USB2DRD_AHBIF			 (1 << 16)
/* bit 15*/
#define IP_HRST_USB2DRD_MUX 			 (1 << 15)
/* bit 6*/
#define IP_RST_USB2DRD_ADP				(1 << 6)

/* 0xfc */
#define PERI_CRG_CLKDIV21                               (0xfc)
/* bit 8*/
#define SEL_ABB_BACKUP                                  (1 << 8)
#define CLKDIV21_MSK_START                              (16)

/* bit for PERI_CRG_ISODIS register */
/* bit 25 */
#define USB_REFCLK_ISO_EN				(1 << 25)


/* PCTRL register */
/* bit for PCTRL_PERI_CTRL24 register */
/* bit 25 */
#define SC_CLK_USB3PHY_3MUX1_SEL		(1 << 25)


/* USB BC register */
#define USBOTG2_CTRL2					(0x08)
/* bit 14 */
#define USB2PHY_POR_N					(14)
/* bit 12 */
#define USB3C_RESET_N					(12)
/* bit 3 */
#define USB2_VBUSVLDEXT				(3)
/* bit 2 */
#define USB2_VBUSVLDSEL				(2)

#define USBOTG2_CTRL3					(0x0C)

#define USBOTG2_CTRL4					(0x10)
/* bit 1 */
#define USB2_VREGBYPASS					(1)
/* bit 0 */
#define USB2_SIDDQ							(0)


#define USB3OTG_QOS_REG_OFFSET		(0x0088)
#define USB3OTG_QOS_PRI_3				(0x0303)


extern struct hisi_dwc3_device *hisi_dwc3_dev;
static uint32_t is_abb_clk_selected(void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & (USB_INIT_CLK_SEL)) != 0)
		return 1;

	return 0;
}

static int usb2_otg_bc_is_unreset(void __iomem *pericfg_base)
{
	volatile uint32_t regval;
	regval = (uint32_t)readl(PERI_CRG_PERRSTSTAT4 + pericfg_base);
	return !((IP_HRST_USB2DRD_AHBIF | IP_HRST_USB2DRD_MUX) & regval);
}

static int config_usb_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *sctrl_base;
	int ret;

	sctrl_base = hisi_dwc3->sctrl_reg_base;

	if (is_abb_clk_selected(sctrl_base)) {
		writel(ABB_DIG2ANA_ISO_EN, SCTRL_MODEM_CTRL_DIS + sctrl_base);

		/* enable USB20 PHY 19.2M clk gt_clk_usb2phy_ref*/
		ret = clk_set_rate(hisi_dwc3->gt_clk_usb2phy_ref, CLK_FREQ_96_M);
		if (ret) {
			usb_err("usb2phy_ref set 96M rate failed, ret=%d\n", ret);
			return ret;
		}
	}else{
		/* enable USB20 PHY 19.2MHz clk gt_clk_usb2phy_ref*/
		ret = clk_set_rate(hisi_dwc3->gt_clk_usb2phy_ref, CLK_FREQ_19_2M);
		if (ret) {
			usb_err("usb2phy_ref set 19.2M rate failed, ret=%d\n", ret);
			return ret;
		}
	}

	ret = clk_prepare_enable(hisi_dwc3->gt_clk_usb2phy_ref);
	if (ret) {
		usb_err("clk_prepare_enable gt_clk_usb2phy_ref failed\n");
		return ret;
	}

	return 0;
}

static void config_usb_phy_controller(void __iomem *otg_bc_reg_base)
{
	/* Release USB20 PHY out of IDDQ mode */
	usb3_rw_reg_clrbit(USB2_SIDDQ, otg_bc_reg_base, USBOTG2_CTRL4);
}

int usb_clk_is_open(void __iomem *pericfg_base)
{
	volatile uint32_t hclk;
	uint32_t hclk_mask = (GT_CLK_USB2DRD_REF | GT_HCLK_USB2DRD);

	hclk = (uint32_t)readl(PERI_CRG_PERSTAT4 + pericfg_base);

	return ((hclk_mask & hclk) == hclk_mask);
}

static void config_usbphy_power(void __iomem * otg_bc_reg_base,
		void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & USB_20PHY_POWER_SEL) != 0)
		usb3_rw_reg_setbit(USB2_VREGBYPASS, otg_bc_reg_base, USBOTG2_CTRL4);
}

static int dwc3_release(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	void __iomem *pericfg_reg_base;
	void __iomem *sctrl_base;

	pericfg_reg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */
	sctrl_base = hisi_dwc3->sctrl_reg_base;/*lint !e529 */

	usb_dbg("+\n");

	/* make sure at reset status */
	/* WO */
	writel(IP_HRST_USB2DRD_AHBIF | IP_HRST_USB2DRD_MUX | IP_RST_USB2DRD_ADP,
				pericfg_reg_base + PERI_CRG_PERRSTEN4);

	udelay(100);

       /* enable gt_hclk_usb2drd*/
	ret = clk_prepare_enable(hisi_dwc3->gt_hclk_usb3otg);
	if (ret) {
		usb_err("clk_prepare_enable gt_hclk_usb3otg failed\n");
		return ret;
	}

       /* enable gt_clk_usb2drd_ref*/
	ret = clk_prepare_enable(hisi_dwc3->clk);
	if (ret) {
		usb_err("clk_prepare_enable clk failed\n");
		return ret;
	}

       /* enable gt_clk_usb2_drd_32k*/
	ret = clk_prepare_enable(hisi_dwc3->gt_clk_usb2_drd_32k);
	if (ret) {
		usb_err("clk_prepare_enable gt_clk_usb2_drd_32k failed\n");
		return ret;
	}

	/* dis-reset usb misc ctrl module */
	/* WO */
	writel(IP_HRST_USB2DRD_AHBIF | IP_HRST_USB2DRD_MUX | IP_RST_USB2DRD_ADP,
				pericfg_reg_base + PERI_CRG_PERRSTDIS4);

	config_usbphy_power(hisi_dwc3->otg_bc_reg_base, sctrl_base);

	ret = config_usb_clk(hisi_dwc3);
	if (ret) {
		usb_err("config_usb_clk failed\n");
		return ret;
	}

	config_usb_phy_controller(hisi_dwc3->otg_bc_reg_base);

	udelay(100);

	/* unreset phy */
	usb3_rw_reg_setbit(USB2PHY_POR_N, hisi_dwc3->otg_bc_reg_base, USBOTG2_CTRL2);

	udelay(100);

	hisi_usb_unreset_phy_if_fpga();

	/* unreset phy reset sync unit*/
	/* WO */
	writel(IP_RST_USB2DRD_PHY, pericfg_reg_base + PERI_CRG_PERRSTDIS4);

	udelay(100);

	/* unreset controller */
	usb3_rw_reg_setbit(USB3C_RESET_N, hisi_dwc3->otg_bc_reg_base, USBOTG2_CTRL2);

	udelay(100);	

	/* set vbus valid */
	usb3_rw_reg_setvalue(BIT(USB2_VBUSVLDSEL) | BIT(USB2_VBUSVLDEXT),
			hisi_dwc3->otg_bc_reg_base, USBOTG2_CTRL2);
	usb_dbg("-\n");

	return ret;
}

static void dwc3_reset(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_reg_base = hisi_dwc3->pericfg_reg_base;
	void __iomem *sctrl_base = hisi_dwc3->sctrl_reg_base;
/*lint -e438 -esym(438,*)*/
/*lint -e529 -esym(529,*)*/

	usb_dbg("+\n");
	if(usb2_otg_bc_is_unreset(pericfg_reg_base) && usb_clk_is_open(pericfg_reg_base)) {
		/* reset controller */
		usb3_rw_reg_clrbit(USB3C_RESET_N, hisi_dwc3->otg_bc_reg_base,
				USBOTG2_CTRL2);

		/* reset phy */
		usb3_rw_reg_clrbit(USB2PHY_POR_N, hisi_dwc3->otg_bc_reg_base,
				USBOTG2_CTRL2);

		if (is_abb_clk_selected(sctrl_base)) {
			writel(ABB_DIG2ANA_ISO_EN, SCTRL_MODEM_CTRL_DIS + sctrl_base);
		}

		/* close phy 19.2M ref clk gt_clk_usb2phy_ref*/
		clk_disable_unprepare(hisi_dwc3->gt_clk_usb2phy_ref);
	}

	/* reset usb misc ctrl module */
	/* WO */
	writel(IP_HRST_USB2DRD_AHBIF | IP_HRST_USB2DRD_MUX | IP_RST_USB2DRD_ADP | IP_RST_USB2DRD_PHY, 
			pericfg_reg_base + PERI_CRG_PERRSTEN4);

	clk_disable_unprepare(hisi_dwc3->gt_hclk_usb3otg);
	clk_disable_unprepare(hisi_dwc3->clk);	
	clk_disable_unprepare(hisi_dwc3->gt_clk_usb2_drd_32k);	

	usb_dbg("-\n");
}

void set_usb2_eye_diagram_param(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *otg_bc_base = hisi_dwc3->otg_bc_reg_base;

	if (!otg_bc_base)
		return;

	if (hisi_dwc3->fpga_flag != 0)
		return;

	/* set high speed phy parameter */
	if (hisi_dwc3->host_flag) {
		writel(hisi_dwc3->eye_diagram_host_param, otg_bc_base + USBOTG2_CTRL3);
		usb_dbg("set hs phy param 0x%x for host\n",
				readl(otg_bc_base + USBOTG2_CTRL3));
	} else {
		writel(hisi_dwc3->eye_diagram_param, otg_bc_base + USBOTG2_CTRL3);
		usb_dbg("set hs phy param 0x%x for device\n",
				readl(otg_bc_base + USBOTG2_CTRL3));
	}
}

/*lint -e438 +esym(438,*)*/
/*lint -e529 +esym(529,*)*/
static int oct_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3);

static int oct_usb3phy_shutdown(struct hisi_dwc3_device *hisi_dwc3)
{
	usb_dbg("+\n");

	if (hisi_dwc3_is_powerdown()) {
		usb_dbg("already shutdown, just return!\n");
		return 0;
	}
	set_hisi_dwc3_power_flag(USB_POWER_OFF);

	dwc3_reset(hisi_dwc3);

	usb_dbg("-\n");

	return 0;
}

static int oct_get_dts_resource(struct hisi_dwc3_device *hisi_dwc3)
{
	struct device *dev = &hisi_dwc3->pdev->dev;
	struct device_node *np;

	/* get h clk handler */
	hisi_dwc3->gt_hclk_usb3otg = devm_clk_get(&hisi_dwc3->pdev->dev, "hclk_usb2drd");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_hclk_usb3otg)) {
		dev_err(dev, "get hclk_usb3otg failed\n");
		return -EINVAL;
	}

	/* get clk handler */
	hisi_dwc3->clk = devm_clk_get(&hisi_dwc3->pdev->dev, "clk_usb2drd_ref");
	if (IS_ERR_OR_NULL(hisi_dwc3->clk)) {
		dev_err(dev, "get clk failed\n");
		return -EINVAL;
	}

	/* get gt_clk_usb2_drd_32k handler */
	hisi_dwc3->gt_clk_usb2_drd_32k = devm_clk_get(&hisi_dwc3->pdev->dev, "clk_usb2_drd_32k");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_clk_usb2_drd_32k)) {
		dev_err(dev, "get gt_clk_usb2_drd_32k failed\n");
		return -EINVAL;
	}

	/* get usb2phy ref clk handler */
	hisi_dwc3->gt_clk_usb2phy_ref = devm_clk_get(&hisi_dwc3->pdev->dev, "clk_usb2phy_ref");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_clk_usb2phy_ref)) {
		dev_err(dev, "get gt_clk_usb2phy_ref failed\n");
		return -EINVAL;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sys_bus");
	if (!np) {
		dev_err(dev, "get sys_bus node failed!\n");
		return -EINVAL;
	}
	hisi_dwc3->sys_bus_reg_base = of_iomap(np, 0);
	if (!hisi_dwc3->sys_bus_reg_base) {
		dev_err(dev, "iomap sys_bus_reg_base failed!\n");
		return -EINVAL;
	}

	return 0;
}

static int oct_shared_phy_init(struct hisi_dwc3_device *hisi_dwc3, unsigned int combophy_flag)
{
	void __iomem *sys_bus_reg_base = hisi_dwc3_dev->sys_bus_reg_base;

	writel(USB3OTG_QOS_PRI_3, sys_bus_reg_base + USB3OTG_QOS_REG_OFFSET);

	return oct_usb3phy_init(hisi_dwc3);
}

static int oct_shared_phy_shutdown(struct hisi_dwc3_device *hisi_dwc3, unsigned int combophy_flag, unsigned int keep_power)
{
	return oct_usb3phy_shutdown(hisi_dwc3);
}

static void dwc3_hisi_lscdtimer_set(void)
{
	if (!hisi_dwc3_dev || !hisi_dwc3_dev->core_ops) {
		usb_err("[USB.CP0] usb driver not setup!\n");
		return;
	}
	hisi_dwc3_dev->core_ops->lscdtimer_set();
}

static struct usb3_phy_ops oct_phy_ops = {
	.init		= oct_usb3phy_init,
	.shutdown	= oct_usb3phy_shutdown,
	.shared_phy_init	= oct_shared_phy_init,
	.shared_phy_shutdown	= oct_shared_phy_shutdown,
	.lscdtimer_set  = dwc3_hisi_lscdtimer_set,
	.get_dts_resource = oct_get_dts_resource,
};

static int oct_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	usb_dbg("+\n");

	if (!hisi_dwc3_is_powerdown()) {
		usb_dbg("already release, just return!\n");
		return 0;
	}

	ret = dwc3_release(hisi_dwc3);
	if (ret) {
		usb_err("[RELEASE.ERR] release error, need check clk!\n");
		return ret;
	}

	set_usb2_eye_diagram_param(hisi_dwc3);

	set_hisi_dwc3_power_flag(USB_POWER_ON);

	usb_dbg("-\n");

	return 0;
}

static int dwc3_oct_probe(struct platform_device *pdev)
{
	int ret;

	ret = hisi_dwc3_probe(pdev, &oct_phy_ops);
	if (ret)
		usb_err("probe failed, ret=[%d]\n", ret);

	return ret;
}

static int dwc3_oct_remove(struct platform_device *pdev)
{
	int ret;

	ret = hisi_dwc3_remove(pdev);
	if (ret)
		usb_err("hisi_dwc3_remove failed, ret=[%d]\n", ret);

	return ret;
}
#ifdef CONFIG_OF
static const struct of_device_id dwc3_oct_match[] = {
	{ .compatible = "hisilicon,oct-dwc3" },
	{},
};
MODULE_DEVICE_TABLE(of, dwc3_oct_match);
#else
#define dwc3_oct_match NULL
#endif

static struct platform_driver dwc3_oct_driver = {
	.probe		= dwc3_oct_probe,
	.remove		= dwc3_oct_remove,
	.driver		= {
		.name	= "usb3-oct",
		.of_match_table = of_match_ptr(dwc3_oct_match),
		.pm	= HISI_DWC3_PM_OPS,
	},
};
/*lint +e715 +e716 +e717 +e835 +e838 +e845 +e533 +e835 +e778 +e774 +e747 +e785 +e438 +e529*/
/*lint -e64 -esym(64,*)*/
/*lint -e528 -esym(528,*)*/
module_platform_driver(dwc3_oct_driver);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL");
/*lint -e753 +esym(753,*)*/
/*lint -e64 +esym(64,*)*/
