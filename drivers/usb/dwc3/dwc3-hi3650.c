#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <pmic_interface.h>
#include <linux/mfd/hisi_pmic.h>
#include "dwc3-hisi.h"

/* other defs */
#define USB3OTG_ACLK_FREQ		240000000

static int usb3_regu_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	if (0 != hisi_dwc3->is_regu_on) {
		usb_dbg("ldo already opened!\n");
		return 0;
	}

	if (0 == hisi_dwc3->fpga_flag) {
		ret = regulator_set_mode(hisi_dwc3->usb_regu, REGULATOR_MODE_NORMAL);
		if (ret) {
			usb_err("set usb regu to normal mode failed (ret %d)\n", ret);
			return ret;
		}
	}

	if (hisi_dwc3->phy_ops->subsys_regu) {
		ret = regulator_enable(hisi_dwc3->phy_ops->subsys_regu);
		if (ret) {
			usb_err("enable regulator otgdebugsubsys failed (ret %d)\n", ret);
			return ret;
		}
	}

	hisi_dwc3->is_regu_on = 1;

	return 0;
}

static int usb3_regu_shutdown(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	if (0 == hisi_dwc3->is_regu_on) {
		usb_dbg("regu already closed!\n");
		return 0;
	}

	if (hisi_dwc3->phy_ops->subsys_regu) {
		ret = regulator_disable(hisi_dwc3->phy_ops->subsys_regu);
		if (ret) {
			usb_err("disable regulator otgdebugsubsys failed (ret %d)\n", ret);
			return ret;
		}
	}

	if (0 == hisi_dwc3->fpga_flag) {
		ret = regulator_set_mode(hisi_dwc3->usb_regu, REGULATOR_MODE_IDLE);
		if (ret) {
			usb_err("set usb regu to idle mode failed (ret %d)\n", ret);
			return ret;
		}
	}

	hisi_dwc3->is_regu_on = 0;

	return 0;
}

static int usb3_clk_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	uint32_t temp;
	void __iomem *pctrl_base = hisi_dwc3->pctrl_reg_base;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;

	/* set usb aclk 240MHz to improve performance */
	ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, USB3OTG_ACLK_FREQ);
	if (ret) {
		usb_err("clk_set_rate failed\n");
		return ret;
	}

	/* disable modem iso */
	temp = readl(pericfg_base + PERI_CRG_ISOSTAT);
	if (temp & PERI_CRG_ISOSTAT_MODEMSUBSYSISOEN) {
		writel(PERI_CRG_ISODIS_MODEMSUBSYSISOEN,
				pericfg_base + PERI_CRG_ISODIS);
	}

	/* config and switch on abb clk */
	temp = readl(pctrl_base + PCTRL_PERI_CTRL24);
	temp &= ~(1 << 28);
	temp &= ~(3 << 25);
	temp |= (2 << 25);
	temp |= (1 << 24);
	writel(temp, pctrl_base + PCTRL_PERI_CTRL24);

	ret = clk_prepare_enable(hisi_dwc3->clk);
	if (ret) {
		usb_err("clk_prepare_enable failed\n");
		return ret;
	}

	return 0;
}

static void usb3_clk_shutdown(struct hisi_dwc3_device *hisi_dwc3)
{
	uint32_t temp;
	void __iomem *pctrl_base = hisi_dwc3->pctrl_reg_base;

	/* switch off abb clk */
	temp = readl(pctrl_base + PCTRL_PERI_CTRL24);
	temp &= ~(1 << 24);
	writel(temp, pctrl_base + PCTRL_PERI_CTRL24);

	clk_disable_unprepare(hisi_dwc3->clk);
}

static void dwc3_release(struct hisi_dwc3_device *hisi_dwc3)
{
	uint32_t temp;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;
	void __iomem *otg_bc_base = hisi_dwc3->otg_bc_reg_base;

	/* reset controller ahbif */
	writel(IP_RST_USB3OTG_MUX | IP_RST_USB3OTG_AHBIF | IP_RST_USB3OTG_32K,
			pericfg_base + PERI_CRG_RSTEN4);
	/* delay 100us */
	udelay(100);

	/* unreset controller ahbif */
	writel(IP_RST_USB3OTG_MUX | IP_RST_USB3OTG_AHBIF | IP_RST_USB3OTG_32K,
			pericfg_base + PERI_CRG_RSTDIS4);

	/* clear powerdown_hsp and powerdown_ssp */
	temp = readl(otg_bc_base + USBOTG3_CTRL2);
	temp &= ~(USBOTG3CTRL2_POWERDOWN_HSP | USBOTG3CTRL2_POWERDOWN_SSP);
	writel(temp, otg_bc_base + USBOTG3_CTRL2);

	/* reset phy */
	writel(IP_RST_USB3OTGPHY_POR | IP_RST_USB3OTG, pericfg_base + PERI_CRG_RSTEN4);
	udelay(100);

	/* set ref_ssp_en: Reference Clock Enable for SS function. */
	temp = readl(otg_bc_base + USBOTG3_CTRL7);
	writel((temp | USBOTG3_CTRL7_REF_SSP_EN), otg_bc_base + USBOTG3_CTRL7);
	udelay(100);

	/* unreset phy */
	writel(IP_RST_USB3OTGPHY_POR, pericfg_base + PERI_CRG_RSTDIS4);
	udelay(100);

	/* unreset controller */
	writel(IP_RST_USB3OTG, pericfg_base + PERI_CRG_RSTDIS4);
	msleep(10);

	/* fake vbus valid signal */
	temp = readl(otg_bc_base + USBOTG3_CTRL3);
	temp |= (USBOTG3_CTRL3_VBUSVLDEXT | USBOTG3_CTRL3_VBUSVLDEXTSEL);
	writel(temp, otg_bc_base + USBOTG3_CTRL3);
}

static void dwc3_reset(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;
	void __iomem *otg_bc_base = hisi_dwc3->otg_bc_reg_base;

	/* reset phy */
	writel(IP_RST_USB3OTG, pericfg_base + PERI_CRG_RSTEN4);
	writel(IP_RST_USB3OTGPHY_POR, pericfg_base + PERI_CRG_RSTEN4);
	/* dealy 100us */
	udelay(100);

	/* power down high speed and super speed */
	writel(USBOTG3CTRL2_POWERDOWN_HSP | USBOTG3CTRL2_POWERDOWN_SSP,
			otg_bc_base + USBOTG3_CTRL2);
	/* delay 100us */
	udelay(100);
}

static int hi3650_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	usb_dbg("+\n");

	ret = usb3_regu_init(hisi_dwc3);
	if (ret)
		return ret;

	ret = usb3_clk_init(hisi_dwc3);
	if (ret)
		return ret;

	dwc3_release(hisi_dwc3);
	config_femtophy_param(hisi_dwc3);

	set_hisi_dwc3_power_flag(1);

	usb_dbg("-\n");

	return 0;
}

static int hi3650_usb3phy_shutdown(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	usb_dbg("+\n");

	set_hisi_dwc3_power_flag(0);

	dwc3_reset(hisi_dwc3);
	usb3_clk_shutdown(hisi_dwc3);

	ret = usb3_regu_shutdown(hisi_dwc3);
	if (ret)
		return ret;

	usb_dbg("-\n");

	return 0;
}


static int hi3650_get_dts_resource(struct hisi_dwc3_device *hisi_dwc3)
{
        struct device *dev = &hisi_dwc3->pdev->dev;

         /* get abb clk handler */
        hisi_dwc3->clk = devm_clk_get(dev, "clk_usb3phy_ref");
        if (IS_ERR_OR_NULL(hisi_dwc3->clk)) {
                dev_err(dev, "get usb3phy ref clk failed\n");
                return -EINVAL;
        }

        /* get a clk handler */
        hisi_dwc3->gt_aclk_usb3otg = devm_clk_get(dev, "aclk_usb3otg");
        if (IS_ERR_OR_NULL(hisi_dwc3->gt_aclk_usb3otg)) {
                dev_err(dev, "get aclk_usb3otg failed\n");
                return -EINVAL;
        }

        if (hisi_dwc3->fpga_flag == 0) {
                hisi_dwc3->usb_regu = devm_regulator_get(dev, "usbldo10");
                if (IS_ERR(hisi_dwc3->usb_regu)) {
                        dev_err(dev, "couldn't get regulator usb regu\n");
                        return -EINVAL;
                }
        }

        return 0;
}

static struct usb3_phy_ops hi3650_phy_ops = {
	.init		= hi3650_usb3phy_init,
	.shutdown	= hi3650_usb3phy_shutdown,
	.get_dts_resource = hi3650_get_dts_resource,
};

static int dwc3_hi3650_probe(struct platform_device *pdev)
{
	int ret = 0;

	hi3650_phy_ops.subsys_regu = devm_regulator_get(&pdev->dev,
		"otgdebugsubsys");
	if (IS_ERR(hi3650_phy_ops.subsys_regu)) {
		dev_err(&pdev->dev, "couldn't get regulator otgdebugsubsys\n");
		hi3650_phy_ops.subsys_regu = NULL;
		return -EINVAL;
	}

	ret = hisi_dwc3_probe(pdev, &hi3650_phy_ops);
	if (ret)
		usb_err("probe failed, ret=[%d]\n", ret);

	return ret;
}

static int dwc3_hi3650_remove(struct platform_device *pdev)
{
	int ret = 0;

	ret = hisi_dwc3_remove(pdev);
	if (ret)
		usb_err("hisi_dwc3_remove failed, ret=[%d]\n", ret);

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id dwc3_hi3650_match[] = {
	{ .compatible = "hisilicon,hi3650-dwc3" },
	{},
};
MODULE_DEVICE_TABLE(of, dwc3_hi3650_match);
#else
#define dwc3_hi3650_match NULL
#endif

static struct platform_driver dwc3_hi3650_driver = {
	.probe		= dwc3_hi3650_probe,
	.remove		= dwc3_hi3650_remove,
	.driver		= {
		.name	= "usb3-hi3650",
		.of_match_table = of_match_ptr(dwc3_hi3650_match),
		.pm	= HISI_DWC3_PM_OPS,
	},
};

module_platform_driver(dwc3_hi3650_driver);
MODULE_LICENSE("GPL");
