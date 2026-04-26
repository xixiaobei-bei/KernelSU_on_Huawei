/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: utilityies for dwc3.
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <pmic_interface.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/hisi/usb/dwc3_usb_interface.h>
#include <linux/hisi/usb/hisi_usb_helper.h>
#include <linux/hisi/usb/hisi_usb_interface.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include "dwc3-hisi.h"
#include "common.h"
#include "vregbypass.h"


struct plat_chip_usb_phy {
	struct chip_usb_phy phy;
	void __iomem *pericfg_reg_base;
	void __iomem *sctrl_reg_base;
	void __iomem *mmc0crg_reg_base;
	void __iomem *pmctrl_reg_base;
	void __iomem *sys_bus_reg_base;

	struct clk *clk;
	struct clk *gt_clk_usb2phy_ref;
	struct clk *gt_clk_usb2_drd_32k;
	struct clk *gt_hclk_usb3otg;

	unsigned int usb3otg_qos_value;
};

static struct plat_chip_usb_phy *_chip_usb_phy;

#define CLK_FREQ_19_2M			19200000
#define CLK_FREQ_96_M			9600000

/* SCTRL register */
/* 0x8 */
#define SCTRL_SCDEEPSLEEPED					0x8
/* bit20 */
#define USB_INIT_CLK_SEL					(1 << 20)
/* bit7 */
#define USB_20PHY_POWER_SEL				(1 << 7)

/* PERI CRG */
#define PERI_CRG_PEREN4				0x40
#define PERI_CRG_PERDIS4			0x44
#define PERI_CRG_PERSTAT4			0x4c
#define GT_CLK_MMC_USBDP			(1 << 1)
#define GT_CLK_USB2DRD_REF			(1 << 19)

#define PERI_CRG_CLKDIV29			0x70c
#define SC_GT_CLK_MMC_USBDP		(1 << 2)
#define PERI_CRG_CLKDIV28_MSK_START		16

/* MMC0 CRG */
#define MMC0_CRG_PEREN0			0x0
#define MMC0_CRG_PERSTAT0			0xC
#define GT_HCLK_USB2DRD						(1 << 5)
#define GT_ACLK_NOC_USB2DRD_ASYNCBRG		(1 << 8)

#define MMC0_CRG_PERRSTEN0		0x20
#define MMC0_CRG_PERRSTDIS0		0x24
#define MMC0_CRG_PERRSTSTAT0		0x28

#define IP_RST_USB2DRD_PHY			(1 << 3)
#define IP_RST_USB3OTG_32K			(1 << 6)
#define IP_RST_USB2DRD_MUX			(1 << 7)
#define IP_RST_USB2DRD_AHBIF			(1 << 8)

/* PMCTRL */
#define PMCTRL_NOC_POWER_IDLEREQ_0	0x380
#define PMCTRL_NOC_POWER_IDLEACK_0	0x384
#define PMCTRL_NOC_POWER_IDLE_0		0x388

#define NOC_USB_POWER_IDLEREQ		(1 << 6)
#define NOC_POWER_IDLEREQ_0_MSK_START		16

#define NOC_USB_POWER_IDLEACK			(1 << 6)
#define NOC_USB_POWER_IDLE			(1 << 6)

/* USB BC register */
#define USBOTG2_CTRL2					0x08
/* bit 14 */
#define USB2PHY_POR_N					14
/* bit 12 */
#define USB3C_RESET_N					12
/* bit 3 */
#define USB2_VBUSVLDEXT				3
/* bit 2 */
#define USB2_VBUSVLDSEL				2

#define USBOTG2_CTRL3					0x0C

#define USBOTG2_CTRL4					0x10
/* bit 1 */
#define USB2_VREGBYPASS					1
/* bit 0 */
#define USB2_SIDDQ							0

#define USB3OTG_QOS_REG_OFFSET				0x0108
#define USB3OTG_QOS_PRI_3				0x0303
#define USB3OTG_QOS_PRI_DEF				0x0
#define USB_ASYNBRG_INIT_TIMEOUT		100

static uint32_t is_abb_clk_selected(void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & (USB_INIT_CLK_SEL)) != 0)
		return 1;

	return 0;
}

static int usb2_otg_bc_is_unreset(void __iomem *mmc0_base)
{
	volatile uint32_t regval;

	regval = (uint32_t)readl(MMC0_CRG_PERRSTSTAT0 + mmc0_base);
	return !((IP_RST_USB2DRD_MUX | IP_RST_USB2DRD_AHBIF) & regval);
}

int usb3_hclk_asyncbrgclk_is_open(void __iomem *mmc0_base)
{
	volatile uint32_t hclk_asyncbrgclk;
	uint32_t hclk_asyncbrgclk_mask = GT_HCLK_USB2DRD | GT_ACLK_NOC_USB2DRD_ASYNCBRG;

	hclk_asyncbrgclk = readl(MMC0_CRG_PERSTAT0 + mmc0_base);

	return ((hclk_asyncbrgclk_mask & hclk_asyncbrgclk) == hclk_asyncbrgclk_mask);
}

int usb3_periasyncbrgclk_is_open(void __iomem *pericfg_base)
{
	volatile uint32_t peri_asyncbrgclk;
	uint32_t peri_asyncbrgclk_mask = GT_CLK_MMC_USBDP;

	peri_asyncbrgclk = readl(PERI_CRG_PERSTAT4 + pericfg_base);
	return ((peri_asyncbrgclk_mask & peri_asyncbrgclk) == peri_asyncbrgclk_mask);
}

static int config_usb_clk(const struct plat_chip_usb_phy *usb_phy)
{
	void __iomem *sctrl_base = NULL;
	int ret;

	sctrl_base = usb_phy->sctrl_reg_base;

	if (is_abb_clk_selected(sctrl_base)) {
		/* enable USB20 PHY 19.2M clk gt_clk_usb2phy_ref */
		ret = clk_set_rate(usb_phy->gt_clk_usb2phy_ref, CLK_FREQ_96_M);
		if (ret) {
			usb_err("usb2phy_ref set 96M rate failed, ret=%d\n", ret);
			return ret;
		}
	} else {
		/* enable USB20 PHY 19.2MHz clk gt_clk_usb2phy_ref */
		ret = clk_set_rate(usb_phy->gt_clk_usb2phy_ref, CLK_FREQ_19_2M);
		if (ret) {
			usb_err("usb2phy_ref set 19.2M rate failed, ret=%d\n", ret);
			return ret;
		}
	}

	ret = clk_prepare_enable(usb_phy->gt_clk_usb2phy_ref);
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
	volatile uint32_t clk;
	uint32_t clk_mask = GT_CLK_USB2DRD_REF;

	clk = (uint32_t)readl(PERI_CRG_PERSTAT4 + pericfg_base);

	return ((clk_mask & clk) == clk_mask);
}

static void usb20phy_init_vregbypass(const void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;
	bool need_init = false;

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & USB_20PHY_POWER_SEL) != 0)
		need_init = false;
	else
		need_init = true;

	pr_info("%s: usb20phy vregbypass %s init\n",
		__func__, need_init ? "need" : "not need");

	if (need_init)
		usb20phy_vregbypass_init();
}

static void config_usbphy_power(void __iomem *otg_bc_reg_base,
		void __iomem *sctrl_base)
{
	usb3_rw_reg_clrbit(USB2_VREGBYPASS, otg_bc_reg_base, USBOTG2_CTRL4);
	usb_dbg("clear usb vregbypass\n");
}

static int sep_asyncbrgclk_init(struct plat_chip_usb_phy *usb_phy)
{
	void __iomem *pmctrl_base = NULL;
	void __iomem *pericfgl_base = NULL;
	volatile uint32_t idleack;
	volatile uint32_t idle;
	int i = 0;

	pmctrl_base = usb_phy->pmctrl_reg_base;
	pericfgl_base = usb_phy->pericfg_reg_base;

	/* enable mmco noc asynchronous bridge clk */
	/* WO */
	writel(SC_GT_CLK_MMC_USBDP | (SC_GT_CLK_MMC_USBDP << PERI_CRG_CLKDIV28_MSK_START),
		PERI_CRG_CLKDIV29 + pericfgl_base);

	/* WO */
	writel(GT_CLK_MMC_USBDP, PERI_CRG_PEREN4 + pericfgl_base);

	/* enable mmc0 noc usb exit low power */
	writel(NOC_USB_POWER_IDLEREQ << NOC_POWER_IDLEREQ_0_MSK_START,
		PMCTRL_NOC_POWER_IDLEREQ_0 + pmctrl_base);
	do {
		udelay(100); /* udelay time */

		idleack = readl(PMCTRL_NOC_POWER_IDLEACK_0 + pmctrl_base);
		idle = readl(PMCTRL_NOC_POWER_IDLE_0 + pmctrl_base);
		i++;
	} while ((((idleack & NOC_USB_POWER_IDLEACK) != 0) ||
			((idle & NOC_USB_POWER_IDLE) != 0)) &&
			(i < USB_ASYNBRG_INIT_TIMEOUT));
	if (i == USB_ASYNBRG_INIT_TIMEOUT) {
		usb_err("wait noc power idle state timeout\n");
		return -EINVAL;
	}
	return 0;
}

static int dwc3_release(struct plat_chip_usb_phy *usb_phy)
{
	int ret;
	void __iomem *sctrl_base = NULL;
	void __iomem *mmc0_base = NULL;

	sctrl_base = usb_phy->sctrl_reg_base;
	mmc0_base = usb_phy->mmc0crg_reg_base;

	usb_dbg("+\n");

	/* init asynchronous bridge */
	ret = sep_asyncbrgclk_init(usb_phy);
	if (ret != 0)
		return ret;
	/* if asynchronous bridge init succeed, config Qos */
	writel(usb_phy->usb3otg_qos_value, usb_phy->sys_bus_reg_base + USB3OTG_QOS_REG_OFFSET);

	/* make sure at reset status */
	/* WO */
	writel(IP_RST_USB3OTG_32K | IP_RST_USB2DRD_MUX | IP_RST_USB2DRD_AHBIF,
				mmc0_base + MMC0_CRG_PERRSTEN0);

	udelay(100); /* udelay time */

	/* enable gt_hclk_usb2drd */
	ret = clk_prepare_enable(usb_phy->gt_hclk_usb3otg);
	if (ret) {
		usb_err("clk_prepare_enable gt_hclk_usb3otg failed\n");
		return ret;
	}

	/* enable gt_clk_usb2drd_ref */
	ret = clk_prepare_enable(usb_phy->clk);
	if (ret) {
		clk_disable_unprepare(usb_phy->gt_hclk_usb3otg);
		usb_err("clk_prepare_enable clk failed\n");
		return ret;
	}

	/* enable gt_clk_usb2_drd_32k */
	ret = clk_prepare_enable(usb_phy->gt_clk_usb2_drd_32k);
	if (ret) {
		clk_disable_unprepare(usb_phy->gt_hclk_usb3otg);
		clk_disable_unprepare(usb_phy->clk);
		usb_err("clk_prepare_enable gt_clk_usb2_drd_32k failed\n");
		return ret;
	}

	/* dis-reset usb misc ctrl module */
	/* WO */
	writel(IP_RST_USB3OTG_32K | IP_RST_USB2DRD_MUX | IP_RST_USB2DRD_AHBIF,
				mmc0_base + MMC0_CRG_PERRSTDIS0);

	config_usbphy_power(usb_phy->phy.otg_bc_reg_base, sctrl_base);

	ret = config_usb_clk(usb_phy);
	if (ret) {
		clk_disable_unprepare(usb_phy->gt_hclk_usb3otg);
		clk_disable_unprepare(usb_phy->clk);
		clk_disable_unprepare(usb_phy->gt_clk_usb2_drd_32k);
		usb_err("config_usb_clk failed\n");
		return ret;
	}

	config_usb_phy_controller(usb_phy->phy.otg_bc_reg_base);

	/* unreset phy */
	usb3_rw_reg_setbit(USB2PHY_POR_N, usb_phy->phy.otg_bc_reg_base, USBOTG2_CTRL2);

	udelay(100); /* udelay time */

	chip_usb_unreset_phy_if_fpga();

	/* unreset phy reset sync unit */
	/* WO */
	writel(IP_RST_USB2DRD_PHY, mmc0_base + MMC0_CRG_PERRSTDIS0);

	udelay(100); /* udelay time */

	/* unreset controller */
	usb3_rw_reg_setbit(USB3C_RESET_N, usb_phy->phy.otg_bc_reg_base, USBOTG2_CTRL2);

	udelay(100); /* udelay time */

	/* set vbus valid */
	usb3_rw_reg_setvalue(BIT(USB2_VBUSVLDSEL) | BIT(USB2_VBUSVLDEXT),
			usb_phy->phy.otg_bc_reg_base, USBOTG2_CTRL2);
	usb_dbg("-\n");

	mdelay(10); /* mdelay time */

	return ret;
}

static void dwc3_reset(struct plat_chip_usb_phy *usb_phy)
{
	void __iomem *pericfg_base = usb_phy->pericfg_reg_base;
	void __iomem *mmc0_base = usb_phy->mmc0crg_reg_base;
	void __iomem *pmctrl_base = usb_phy->pmctrl_reg_base;
	volatile uint32_t idleack;
	volatile uint32_t idle;
	int i = 0;

	usb_dbg("+\n");
	if (usb2_otg_bc_is_unreset(mmc0_base) && usb3_hclk_asyncbrgclk_is_open(mmc0_base)
			&& usb_clk_is_open(pericfg_base) && usb3_periasyncbrgclk_is_open(pericfg_base)) {
		/* reset controller */
		usb3_rw_reg_clrbit(USB3C_RESET_N, usb_phy->phy.otg_bc_reg_base,
				USBOTG2_CTRL2);

		/* reset phy */
		usb3_rw_reg_clrbit(USB2PHY_POR_N, usb_phy->phy.otg_bc_reg_base,
				USBOTG2_CTRL2);

		/* close phy 19.2M ref clk gt_clk_usb2phy_ref */
		clk_disable_unprepare(usb_phy->gt_clk_usb2phy_ref);
	}

	/* reset usb misc ctrl module */
	/* WO */
	writel(IP_RST_USB2DRD_PHY | IP_RST_USB3OTG_32K | IP_RST_USB2DRD_MUX | IP_RST_USB2DRD_AHBIF,
			mmc0_base + MMC0_CRG_PERRSTEN0);

	clk_disable_unprepare(usb_phy->gt_hclk_usb3otg);
	clk_disable_unprepare(usb_phy->clk);
	clk_disable_unprepare(usb_phy->gt_clk_usb2_drd_32k);

	/* disable mmc0 noc usb exit low power */
	writel(NOC_USB_POWER_IDLEREQ | (NOC_USB_POWER_IDLEREQ << NOC_POWER_IDLEREQ_0_MSK_START),
		PMCTRL_NOC_POWER_IDLEREQ_0 + pmctrl_base);

	do {
		udelay(100); /* udelay time */

		idleack = readl(pmctrl_base + PMCTRL_NOC_POWER_IDLEACK_0);
		idle = readl(pmctrl_base + PMCTRL_NOC_POWER_IDLE_0);
		i++;
	} while ((((idleack & NOC_USB_POWER_IDLEACK) == 0) ||
			((idle & NOC_USB_POWER_IDLE) == 0)) &&
			(i < USB_ASYNBRG_INIT_TIMEOUT));
	if (i == USB_ASYNBRG_INIT_TIMEOUT)
		usb_err("wait noc power enter idle state timeout\n");

	/* WO */
	writel(GT_CLK_MMC_USBDP, PERI_CRG_PERDIS4 + pericfg_base);

	/* WO */
	writel(SC_GT_CLK_MMC_USBDP << PERI_CRG_CLKDIV28_MSK_START, PERI_CRG_CLKDIV29 + pericfg_base);

	usb_dbg("-\n");
}

void set_usb2_eye_diagram_param(struct plat_chip_usb_phy *usb_phy,
		unsigned int eye_diagram_param)
{
	void __iomem *otg_bc_base = usb_phy->phy.otg_bc_reg_base;

	if (!otg_bc_base)
		return;

	if (chip_dwc3_is_fpga())
		return;

	/* set high speed phy parameter */
	writel(eye_diagram_param, otg_bc_base + USBOTG2_CTRL3);
	usb_dbg("set hs phy param 0x%x for host\n",
			readl(otg_bc_base + USBOTG2_CTRL3));
}

static int sep_usb3phy_shutdown(unsigned int support_dp)
{
	usb_dbg("+\n");
	if (!_chip_usb_phy)
		return -ENODEV;

	if (chip_dwc3_is_powerdown()) {
		usb_dbg("already shutdown, just return!\n");
		return 0;
	}
	set_chip_dwc3_power_flag(USB_POWER_OFF);

	dwc3_reset(_chip_usb_phy);

	usb_dbg("-\n");

	return 0;
}

static int sep_get_dts_resource(struct device *dev, struct plat_chip_usb_phy *usb_phy)
{
	void __iomem *regs = NULL;

	/* get hclk handler */
	usb_phy->gt_hclk_usb3otg = devm_clk_get(dev, "hclk_usb2drd");
	/* get clk handler */
	usb_phy->clk = devm_clk_get(dev, "clk_usb2drd_ref");
	if ((IS_ERR_OR_NULL(usb_phy->gt_hclk_usb3otg)) ||
		(IS_ERR_OR_NULL(usb_phy->clk))) {
		dev_err(dev, "get hclk_usb3otg or clk failed\n");
		return -EINVAL;
	}

	/* get gt_clk_usb2_drd_32k handler */
	usb_phy->gt_clk_usb2_drd_32k = devm_clk_get(dev, "clk_usb2_drd_32k");
	/* get usb2phy ref clk handler */
	usb_phy->gt_clk_usb2phy_ref = devm_clk_get(dev, "clk_usb2phy_ref");
	if ((IS_ERR_OR_NULL(usb_phy->gt_clk_usb2_drd_32k)) ||
		(IS_ERR_OR_NULL(usb_phy->gt_clk_usb2phy_ref))) {
		dev_err(dev, "get gt_clk_usb2_drd_32k or gt_clk_usb2phy_ref failed\n");
		return -EINVAL;
	}

	/* get otg bc */
	usb_phy->phy.otg_bc_reg_base = of_iomap(dev->of_node, 0);
	if (!usb_phy->phy.otg_bc_reg_base) {
		dev_err(dev, "ioremap res 0 failed\n");
		return -ENOMEM;
	}

	usb_phy->pericfg_reg_base = of_devm_ioremap(dev, "hisilicon,crgctrl");
	if (IS_ERR(usb_phy->pericfg_reg_base)) {
		dev_err(dev, "iomap pericfg_reg_base failed!\n");
		return -EINVAL;
	}

	regs = of_devm_ioremap(dev, "hisilicon,sysctrl");
	if (IS_ERR(regs)) {
		usb_err("iomap sctrl_reg_base failed!\n");
		return PTR_ERR(regs);
	}
	usb_phy->sctrl_reg_base = regs;

	usb_phy->mmc0crg_reg_base = of_devm_ioremap(dev, "hisilicon,mmc0crg");
	if (IS_ERR(usb_phy->mmc0crg_reg_base)) {
		dev_err(dev, "iomap mmc0crg_reg_base failed!\n");
		return -EINVAL;
	}

	usb_phy->pmctrl_reg_base = of_devm_ioremap(dev, "hisilicon,pmctrl");
	if (IS_ERR(usb_phy->pmctrl_reg_base)) {
		dev_err(dev, "iomap pmctrl failed!\n");
		return -EINVAL;
	}

	usb_phy->sys_bus_reg_base = of_devm_ioremap(dev, "hisilicon,sys_bus");
	if (IS_ERR(usb_phy->sys_bus_reg_base)) {
		dev_err(dev, "iomap sys_bus_reg_base failed!\n");
		return -EINVAL;
	}

	return 0;
}

static int sep_usb3phy_init(unsigned int support_dp,
		unsigned int eye_diagram_param,
		unsigned int usb3_phy_tx_vboost_lvl)
{
	int ret;

	usb_dbg("+\n");
	if (!_chip_usb_phy)
		return -ENODEV;

	if (!chip_dwc3_is_powerdown()) {
		usb_dbg("already release, just return!\n");
		return 0;
	}

	ret = dwc3_release(_chip_usb_phy);
	if (ret) {
		usb_err("[RELEASE.ERR] release error, need check clk!\n");
		return ret;
	}

	set_usb2_eye_diagram_param(_chip_usb_phy, eye_diagram_param);

	set_chip_dwc3_power_flag(USB_POWER_ON);

	usb_dbg("-\n");

	return 0;
}

static int sep_shared_phy_init(unsigned int support_dp, unsigned int eye_diagram_param,
		unsigned int combophy_flag)
{
	if (!_chip_usb_phy)
		return -ENODEV;

	/* switch to hifi,demote usb's priority */
	_chip_usb_phy->usb3otg_qos_value = USB3OTG_QOS_PRI_3;

	return sep_usb3phy_init(0, eye_diagram_param, 0);
}

static int sep_shared_phy_shutdown(unsigned int support_dp, unsigned int combophy_flag, unsigned int keep_power)
{
	if (!_chip_usb_phy)
		return -ENODEV;

	/* shutdown hifi,resume usb's priority */
	_chip_usb_phy->usb3otg_qos_value = USB3OTG_QOS_PRI_DEF;

	return sep_usb3phy_shutdown(0);
}

static int dwc3_sep_probe(struct platform_device *pdev)
{
	int ret;

	_chip_usb_phy = devm_kzalloc(&pdev->dev, sizeof(*_chip_usb_phy),
			GFP_KERNEL);
	if (!_chip_usb_phy)
		return -ENOMEM;

	ret = sep_get_dts_resource(&pdev->dev, _chip_usb_phy);
	if (ret) {
		usb_err("get_dts_resource failed\n");
		goto err_out;
	}

	usb20phy_init_vregbypass(_chip_usb_phy->sctrl_reg_base);
	_chip_usb_phy->phy.init = sep_usb3phy_init;
	_chip_usb_phy->phy.shutdown = sep_usb3phy_shutdown;
	_chip_usb_phy->phy.shared_phy_init = sep_shared_phy_init;
	_chip_usb_phy->phy.shared_phy_shutdown = sep_shared_phy_shutdown;

	ret = chip_usb_dwc3_register_phy(&_chip_usb_phy->phy);
	if (ret) {
		usb_err("usb_dwc3_register_phy failed\n");
		goto err_out;
	}

	return 0;
err_out:
	_chip_usb_phy = NULL;
	return ret;
}

static int dwc3_sep_remove(struct platform_device *pdev)
{
	int ret;

	ret = chip_usb_dwc3_unregister_phy(&_chip_usb_phy->phy);
	_chip_usb_phy = NULL;

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id dwc3_sep_match[] = {
	{ .compatible = "hisilicon,sep-dwc3" },
	{},
};
MODULE_DEVICE_TABLE(of, dwc3_sep_match);
#else
#define dwc3_sep_match NULL
#endif

static struct platform_driver dwc3_sep_driver = {
	.probe		= dwc3_sep_probe,
	.remove		= dwc3_sep_remove,
	.driver		= {
		.name	= "usb3-sep",
		.of_match_table = of_match_ptr(dwc3_sep_match),
	},
};

module_platform_driver(dwc3_sep_driver);

MODULE_LICENSE("GPL");

