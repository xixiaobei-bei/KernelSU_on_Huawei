/*
 * hiusbc_core.c -- Driver for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/property.h>
#include "hiusbc_debug.h"
#include "hiusbc_drd.h"
#include "hiusbc_debugfs.h"
#include "hiusbc_gadget.h"
#include "hiusbc_host.h"
#include "hiusbc_core.h"

int hiusbc_init_phy(struct hiusbc *hiusbc)
{
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	/* turn on U2 and U3 phy. */
	reg = hiusbc_readl(hiusbc->com_regs,
		MPI_APP_COM_REG_PHY_SEL_MODE_OFFSET);
	reg &= ~MPI_APP_COM_REG_PHY_SEL_MODE_MASK;
	reg |= hiusbc_phy_set(HIUSBC_PHY_SET_BOTH);
	hiusbc_writel(reg, hiusbc->com_regs,
		MPI_APP_COM_REG_PHY_SEL_MODE_OFFSET);

	reg = hiusbc_readl(hiusbc->com_regs,
		MPI_APP_COM_REG_U3_CONTINUE_MODE_OFFSET);
	reg |= MPI_APP_COM_REG_USBC_U2_EN_MASK |
		MPI_APP_COM_REG_USBC_U3_EN_MASK;
	hiusbc_writel(reg, hiusbc->com_regs,
		MPI_APP_COM_REG_U3_CONTINUE_MODE_OFFSET);

	/* enable U2 HS. */
	reg = hiusbc_readl(hiusbc->u2_piu_regs,
		MPI_U2_PM_UIU_REG_CSR_DEV_PRTRST_HS_HANDSHAKE_EN_OFFSET);
	reg |= MPI_U2_PM_UIU_REG_CSR_DEV_PRTRST_HS_HANDSHAKE_EN_MASK;
	hiusbc_writel(reg, hiusbc->u2_piu_regs,
		MPI_U2_PM_UIU_REG_CSR_DEV_PRTRST_HS_HANDSHAKE_EN_OFFSET);

	/* use U3 vbus */
	reg = hiusbc_readl(hiusbc->com_regs,
		MPI_APP_COM_REG_VBUS_SEL_MODE_OFFSET);
	reg |= MPI_APP_COM_REG_VBUS_SEL_MODE_MASK;
	hiusbc_writel(reg, hiusbc->com_regs,
		MPI_APP_COM_REG_VBUS_SEL_MODE_OFFSET);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return 0;
}

int hiusbc_get_irq(struct hiusbc *hiusbc)
{
	struct platform_device *hiusbc_pdev = to_platform_device(hiusbc->dev);
	int irq;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");
	irq = platform_get_irq(hiusbc_pdev, 0);
	if (irq > 0)
		goto out;

	if (irq != -EPROBE_DEFER)
		hiusbc_dbg(HIUSBC_DEBUG_SYS, "missing irq\n");

	if (!irq)
		irq = -EINVAL;

out:
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "- irq = %d\n", irq);
	return irq;
}

int hiusbc_reset_controller(struct hiusbc *hiusbc)
{
	u32 reg;
	u32 drd_mode_backup;
	u32 timeout = 1000;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	/* we need to switch to device mode before dev_rst. */
	drd_mode_backup = hiusbc_readl(hiusbc->com_regs,
	MPI_APP_COM_REG_DRD_MODE_OFFSET);
	if (hiusbc_get_drd_mode(drd_mode_backup) == HIUSBC_DRD_MODE_HOST)
		hiusbc_writel(
			hiusbc_drd_mode(HIUSBC_DRD_MODE_DEVICE),
			hiusbc->com_regs, MPI_APP_COM_REG_DRD_MODE_OFFSET);


	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_RST_OFFSET);
	reg |= MPI_APP_DEVICE_REG_DEV_RST_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_RST_OFFSET);

	do {
		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_RST_OFFSET);
		reg &= MPI_APP_DEVICE_REG_DEV_RST_MASK;
		udelay(1);
	} while (reg && --timeout);

	if (reg && !timeout) {
		dev_err(hiusbc->dev, "Timeout to reset controller!\n");
		ret = -ETIMEDOUT;
	}

	if (hiusbc_get_drd_mode(drd_mode_backup) == HIUSBC_DRD_MODE_HOST)
		hiusbc_writel(drd_mode_backup,
			hiusbc->com_regs, MPI_APP_COM_REG_DRD_MODE_OFFSET);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return ret;
}

void hiusbc_set_speed(struct hiusbc *hiusbc,
		enum usb_device_speed speed)
{
	bool u3_en = true;
	bool hs_en = true;
	bool ssp_en = false;
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+ speed = %d\n", speed);

	switch (speed) {
	case USB_SPEED_LOW:
	case USB_SPEED_FULL:
		/* fall-throung */
		hs_en = false;
	case USB_SPEED_HIGH:
		u3_en = false;
		break;

	case USB_SPEED_SUPER:
	case USB_SPEED_SUPER_PLUS:
		/* fall-through */
		break;

	default:
		dev_err(hiusbc->dev, "invalid speed = %d.\n", speed);
	}

	reg = hiusbc_readl(hiusbc->com_regs,
	MPI_APP_COM_REG_U3_CONTINUE_MODE_OFFSET);
	reg &= ~MPI_APP_COM_REG_U3_CONTINUE_MODE_MASK;
	reg |= MPI_APP_COM_REG_USBC_U2_EN_MASK;
	if (u3_en)
		reg |= MPI_APP_COM_REG_USBC_U3_EN_MASK;
	else
		reg &= ~MPI_APP_COM_REG_USBC_U3_EN_MASK;
	hiusbc_writel(reg, hiusbc->com_regs,
	MPI_APP_COM_REG_U3_CONTINUE_MODE_OFFSET);

	reg = hiusbc_readl(hiusbc->u2_piu_regs,
		MPI_U2_PM_UIU_REG_CSR_DEV_PRTRST_HS_HANDSHAKE_EN_OFFSET);
	if (hs_en)
		reg |= MPI_U2_PM_UIU_REG_CSR_DEV_PRTRST_HS_HANDSHAKE_EN_MASK;
	else
		reg &= ~MPI_U2_PM_UIU_REG_CSR_DEV_PRTRST_HS_HANDSHAKE_EN_MASK;
	hiusbc_writel(reg, hiusbc->u2_piu_regs,
		MPI_U2_PM_UIU_REG_CSR_DEV_PRTRST_HS_HANDSHAKE_EN_OFFSET);

	reg = hiusbc_readl(hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_TARGET_SPEED_OFFSET);
	reg &= ~MPI_U3_LINK_REG_CSR_TARGET_SPEED_MASK;
	if (ssp_en)
		reg |= (1 << MPI_U3_LINK_REG_CSR_TARGET_SPEED_SHIFT);
	hiusbc_writel(reg, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_TARGET_SPEED_OFFSET);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}

static int hiusbc_init_dr_mode(struct hiusbc *hiusbc)
{
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	hiusbc->current_dr_mode = HIUSBC_DR_MODE_UNKNOWN;

	switch (hiusbc->support_dr_mode) {
	case HIUSBC_DR_MODE_HOST:
	case HIUSBC_DR_MODE_DEVICE:
		hiusbc_set_mode(hiusbc, hiusbc->support_dr_mode);
		break;

	case HIUSBC_DR_MODE_BOTH:
		ret = hiusbc_drd_init(hiusbc);
		break;

	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static void hiusbc_exit_current_mode(struct hiusbc *hiusbc)
{
	unsigned long flags;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	switch (hiusbc->current_dr_mode) {
	case HIUSBC_DR_MODE_DEVICE:
		hiusbc_gadget_exit(hiusbc);
		break;
	case HIUSBC_DR_MODE_HOST:
		hiusbc_host_exit(hiusbc);
		break;
	default:
		break;
	}

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->current_dr_mode = HIUSBC_DR_MODE_UNKNOWN;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void hiusbc_enter_desired_mode(struct hiusbc *hiusbc)
{
	unsigned long flags;
	int ret = 0;
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->current_dr_mode = hiusbc->desired_dr_mode;
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	switch (hiusbc->desired_dr_mode) {
	case HIUSBC_DR_MODE_DEVICE:
		ret = hiusbc_run_stop(hiusbc, 0);
		if (ret) {
			dev_err(hiusbc->dev, "failed to stop controller!\n");
			break;
		}

		/* switch clk */
		reg = hiusbc_readl(hiusbc->hub_regs,
			USBC_HUB_REG_USBC_SOFT_RST_OFFSET);
		reg |= (1 << USBC_HUB_REG_RST_SOFT_TRPU_DEV_N_SHIFT);
		hiusbc_writel(reg, hiusbc->hub_regs,
			USBC_HUB_REG_USBC_SOFT_RST_OFFSET);

		ret = hiusbc_reset_controller(hiusbc);
		if (ret) {
			dev_err(hiusbc->dev, "failed to reset controlle!\n");
			break;
		}

		hiusbc_writel(hiusbc_drd_mode(HIUSBC_DRD_MODE_DEVICE),
			hiusbc->com_regs, MPI_APP_COM_REG_DRD_MODE_OFFSET);

		reg = hiusbc_readl(hiusbc->hub_regs,
			USBC_HUB_REG_USBC_SOFT_RST_OFFSET);
		reg &= ~USBC_HUB_REG_RST_SOFT_TRPU_HOST_N_MASK;
		hiusbc_writel(reg, hiusbc->hub_regs,
			USBC_HUB_REG_USBC_SOFT_RST_OFFSET);

		ret = hiusbc_gadget_init(hiusbc);
		if (ret)
			dev_err(hiusbc->dev, "failed to init device mode!\n");
		break;
	case HIUSBC_DR_MODE_HOST:
		ret = hiusbc_run_stop(hiusbc, 0);
		if (ret < 0) {
			dev_err(hiusbc->dev, "failed to stop controller!\n");
			break;
		}

		/* switch clk */
		reg = hiusbc_readl(hiusbc->hub_regs,
			USBC_HUB_REG_USBC_SOFT_RST_OFFSET);
		reg |= (1 << USBC_HUB_REG_RST_SOFT_TRPU_HOST_N_SHIFT);
		hiusbc_writel(reg, hiusbc->hub_regs,
			USBC_HUB_REG_USBC_SOFT_RST_OFFSET);

		ret = hiusbc_reset_controller(hiusbc);
		if (ret) {
			dev_err(hiusbc->dev, "failed to reset controller!\n");
			break;
		}

		hiusbc_writel(hiusbc_drd_mode(HIUSBC_DRD_MODE_HOST),
			hiusbc->com_regs, MPI_APP_COM_REG_DRD_MODE_OFFSET);

		reg = hiusbc_readl(hiusbc->hub_regs,
			USBC_HUB_REG_USBC_SOFT_RST_OFFSET);
		reg &= ~USBC_HUB_REG_RST_SOFT_TRPU_DEV_N_MASK;
		hiusbc_writel(reg, hiusbc->hub_regs,
			USBC_HUB_REG_USBC_SOFT_RST_OFFSET);

		ret = hiusbc_host_init(hiusbc);
		if (ret)
			dev_err(hiusbc->dev, "failed to init host mode!\n");

		break;
	default:
		break;
	}

	if (ret) {
		spin_lock_irqsave(&hiusbc->lock, flags);
		hiusbc->current_dr_mode = HIUSBC_DR_MODE_UNKNOWN;
		spin_unlock_irqrestore(&hiusbc->lock, flags);
		dev_err(hiusbc->dev, "Set current_dr_mode to _UNKNOWN!\n");
	}
}

static void hiusbc_exit_dr_mode(struct hiusbc *hiusbc)
{
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	switch (hiusbc->support_dr_mode) {
	case HIUSBC_DR_MODE_HOST:
	case HIUSBC_DR_MODE_DEVICE:
		hiusbc_exit_current_mode(hiusbc);
		break;
	case HIUSBC_DR_MODE_BOTH:
		hiusbc_drd_exit(hiusbc);
		break;
	default:
		break;
	}
}

void hiusbc_set_mode(struct hiusbc *hiusbc,
			enum hiusbc_dr_mode desired_mode)
{
	enum hiusbc_dr_mode current_mode = hiusbc->current_dr_mode;
	unsigned long flags;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "current_mode = %d, desired_mode = %d\n",
		current_mode, desired_mode);

	if (desired_mode == current_mode ||
		desired_mode == HIUSBC_DR_MODE_BOTH)
		return;

	hiusbc_exit_current_mode(hiusbc);

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->desired_dr_mode = desired_mode;
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	hiusbc_enter_desired_mode(hiusbc);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}

static void hiusbc_get_properties(struct hiusbc *hiusbc)
{
	struct device *dev = hiusbc->dev;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	hiusbc->support_dr_mode = HIUSBC_DR_MODE_BOTH;

	if (device_property_read_bool(dev, "linux,sysdev_is_parent"))
		hiusbc->sysdev = dev->parent;
	else
		hiusbc->sysdev = dev;

	hiusbc->max_speed = usb_get_maximum_speed(dev);
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "max_speed = %d\n", hiusbc->max_speed);
	if (hiusbc->max_speed == USB_SPEED_UNKNOWN)
		hiusbc->max_speed = USB_SPEED_SUPER;

	hiusbc->hird_suspend = 12;

	hiusbc->usb3_dev_lpm_capable = 1;
	hiusbc->usb3_dev_lpm_u1_accept = 1;
	hiusbc->usb3_dev_lpm_u2_accept = 1;
	hiusbc->usb3_dev_lpm_u1_initiate = 1;
	hiusbc->usb3_dev_lpm_u2_initiate = 1;
	hiusbc->usb3_dev_lpm_ux_exit = 0;

	hiusbc->usb3_host_lpm_capable = 1;
	pr_err("hiusbc->usb3_host_lpm_capable = %u\n",
			hiusbc->usb3_host_lpm_capable);

#ifdef CONFIG_HIUSBC_EDA_TEST_CASE
	hiusbc->force_maxp = 1;
#endif
}

static int hiusbc_init_regs(struct hiusbc *hiusbc, struct resource *res)
{
	void __iomem *regs = NULL;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	res->start += HIUSBC_DEVICE_MODE_REG_BASE;

	regs = devm_ioremap_resource(hiusbc->dev, res);
	if (IS_ERR(regs)) {
		pr_err("hiusbc ioremap fail.\n");
		ret = PTR_ERR(regs);
		goto exit;
	}

	hiusbc->regs = regs;
	hiusbc->regs_size = resource_size(res);

	hiusbc->host_regs = regs + MPI_APP_HOST_REG_BASE_ADDR;
	hiusbc->dev_regs = regs + MPI_APP_DEVICE_REG_BASE_ADDR;
	hiusbc->com_regs = regs + MPI_APP_COM_REG_BASE_ADDR;
	hiusbc->hub_regs = regs + USBC_HUB_REG_BASE_ADDR;

	hiusbc->lmi_regs = regs + MPI_MAU_LMI_REG_BASE_ADDR;

	hiusbc->u2_pl_regs = regs + MPI_U2_PL_REG_BASE_ADDR;
	hiusbc->u2_piu_regs = regs + MPI_U2_PM_UIU_REG_BASE_ADDR;
	hiusbc->u3_pl_regs = regs + MPI_U3PL_REG_BASE_ADDR;
	hiusbc->u3_link_regs = regs + MPI_U3_LINK_REG_BASE_ADDR;

	hiusbc_dbg(HIUSBC_DEBUG_ERR,
		"base address = 0x%p, dev_regs = 0x%p, u3_link_regs = 0x%p\n",
		regs, hiusbc->dev_regs,
		hiusbc->u3_link_regs);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return ret;

exit:
	res->start -= HIUSBC_DEVICE_MODE_REG_BASE;
	return ret;
}

static void hiusbc_clear_regs(struct hiusbc *hiusbc, struct resource *res)
{
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	res->start -= HIUSBC_DEVICE_MODE_REG_BASE;
	hiusbc->regs	= NULL;
	hiusbc->regs_size	= 0;

	hiusbc->host_regs = NULL;
	hiusbc->dev_regs = NULL;
	hiusbc->com_regs = NULL;
	hiusbc->hub_regs = NULL;
	hiusbc->lmi_regs = NULL;
	hiusbc->u2_pl_regs = NULL;
	hiusbc->u2_piu_regs = NULL;
	hiusbc->u3_pl_regs = NULL;
	hiusbc->u3_link_regs = NULL;
}

static void hiusbc_fpga_init(struct hiusbc *hiusbc)
{
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	/* Set MPI delay for FPGA platform. */
	reg = 0x00100005;
	/* APP 0x13ff8 = 0x00100005 */
	hiusbc_writel(reg, hiusbc->com_regs,
		MPI_APP_COM_REG_MPI_RDY_NUM_OFFSET);
	/* LMI 0x171fc = 0x00100005 */
	hiusbc_writel(reg, hiusbc->lmi_regs,
		MPI_MAU_LMI_REG_MPI_RDY_NUM_OFFSET);
	/* U2_PL 0x20000 = 0x00100005 */
	hiusbc_writel(reg, hiusbc->u2_pl_regs,
		MPI_U2_PL_REG_MPI_RDY_NUM_OFFSET);
	/* U2_UIU 0X28000 = 0x00100005 */
	hiusbc_writel(reg, hiusbc->u2_piu_regs,
		MPI_U2_PM_UIU_REG_MPI_RDY_NUM_OFFSET);
	/* U3_LINK 0X38000 = 0x00100005 */
	hiusbc_writel(reg, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_MPI_RDY_NUM_OFFSET);
	/* U3_PL 0x30c00 = 0x00100005 */
	hiusbc_writel(reg, hiusbc->u3_pl_regs,
		MPI_U3PL_REG_PL_MPI_CTRL_OFFSET);

	/*
	 * Force phy don't go to low power. Because FPGA can't switch clk.
	 * 0x28838 = 0
	 */
	hiusbc_writel(0, hiusbc->u2_piu_regs,
		MPI_U2_PM_UIU_REG_CSR_HST_U2_U3_PHY_LP_EN_OFFSET);
	/* 0x28860 = 0 */
	hiusbc_writel(0, hiusbc->u2_piu_regs,
		MPI_U2_PM_UIU_REG_CSR_HST_POWERED_OFF_PHY_LP_EN_OFFSET);
	/* 0x28a04 = 0 */
	hiusbc_writel(0, hiusbc->u2_piu_regs,
		MPI_U2_PM_UIU_REG_CSR_DEV_LP_CFG_OFFSET);

	/* Force U3 P2 */
	hiusbc_writel(2, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_RXD_PD1_OFFSET);
	hiusbc_writel(2, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_RXD_PD2_OFFSET);

	/* DELTA_CFG_PN clk hz *100 */
	reg = hiusbc_readl(hiusbc->com_regs,
		MPI_APP_COM_REG_DELTA_CFG_PN_OFFSET);
	reg &= ~MPI_APP_COM_REG_DELTA_PN_GEN1_MASK;
	reg |= (0x0000186a << MPI_APP_COM_REG_DELTA_PN_GEN1_SHIFT);
	hiusbc_writel(reg, hiusbc->com_regs,
		MPI_APP_COM_REG_DELTA_CFG_PN_OFFSET);

	/* PM_TIMER_UNIT: pm_1us_unit_gen1 */
	reg = hiusbc_readl(hiusbc->u3_pl_regs,
		MPI_U3PL_REG_PM_TIMER_UNIT_OFFSET);
	reg &= ~MPI_U3PL_REG_PM_1US_UNIT_GEN1_MASK;
	reg |= (0x0000003e << MPI_U3PL_REG_PM_1US_UNIT_GEN1_SHIFT);
	hiusbc_writel(reg, hiusbc->u3_pl_regs,
		MPI_U3PL_REG_PM_TIMER_UNIT_OFFSET);

	/* MAU_CLKEN_CNT_PERIOD_L */
	hiusbc_writel(0x3e, hiusbc->com_regs,
		MPI_APP_COM_REG_MAU_CLKEN_CNT_PERIOD_L_OFFSET);

	/* ERPU_INT_CNT_PARA */
	hiusbc_writel(0x10, hiusbc->com_regs,
		MPI_APP_COM_REG_ERPU_INT_CNT_PARA_OFFSET);

	/* hiusbc FPGA Debug: Match FPGA 20M clk. */
	hiusbc_writel(10000, hiusbc->host_regs,
		MPI_APP_HOST_REG_CRPU_BW_SYSBUS_OFFSET);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}

static int hiusbc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res = NULL;
	struct hiusbc *hiusbc = NULL;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_ERR, "+\n");

	hiusbc = devm_kzalloc(dev, sizeof(*hiusbc), GFP_KERNEL);
	if (!hiusbc) {
		ret = -ENOMEM;
		goto exit;
	}

	hiusbc->dev = dev;
	dev_set_drvdata(dev, hiusbc);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "Can't get memory resource of hiusbc.\n");
		ret = -ENODEV;
		goto exit;
	}

	ret = hiusbc_init_regs(hiusbc, res);
	if (ret) {
		dev_err(dev, "Failed to initial registers of hiusbc.\n");
		goto exit;
	}

	hiusbc_get_properties(hiusbc);

	platform_set_drvdata(pdev, hiusbc);

	spin_lock_init(&hiusbc->lock);

	hiusbc_fpga_init(hiusbc);

	pm_runtime_set_active(dev);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_set_autosuspend_delay(dev, HIUSBC_AUTOSUSPEND_DELAY);
	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0)
		goto pm_err;

	pm_runtime_forbid(dev);

	ret = hiusbc_init_phy(hiusbc);
	if (ret) {
		dev_err(dev, "Failed to initial phy of hiusbc.\n");
		goto phy_err;
	}

	ret = hiusbc_init_dr_mode(hiusbc);
	if (ret) {
		dev_err(dev, "Failed to initial dr mode of hiusbc.\n");
		goto dr_mode_err;
	}

	hiusbc_debugfs_init(hiusbc);

	pm_runtime_put(dev);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
	return 0;

dr_mode_err:


phy_err:
	pm_runtime_allow(dev);

pm_err:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);
	hiusbc_clear_regs(hiusbc, res);

exit:
	hiusbc_dbg(HIUSBC_DEBUG_ERR, "-: ret = %d\n", ret);
	return ret;
}

static int hiusbc_remove(struct platform_device *pdev)
{
	struct hiusbc *hiusbc = platform_get_drvdata(pdev);
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct device *dev = &pdev->dev;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	pm_runtime_get_sync(dev);

	hiusbc_debugfs_exit(hiusbc);

	hiusbc_exit_dr_mode(hiusbc);

	pm_runtime_allow(dev);

	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);
	hiusbc_clear_regs(hiusbc, res);

	return 0;
}

static int hiusbc_common_suspend(struct hiusbc *hiusbc)
{
	unsigned long flags;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);
	if (hiusbc->current_dr_mode == HIUSBC_DR_MODE_DEVICE) {
		if (!hiusbc->gadget_driver)
			goto exit;

		hiusbc_run_stop(hiusbc, 0);
		hiusbc_gadget_disconnect(hiusbc);
		hiusbc_stop(hiusbc);
		hiusbc_gadget_exit_interrupt(hiusbc);
	}

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "-\n");

exit:
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return 0;
}

static int hiusbc_common_resume(struct hiusbc *hiusbc)
{
	unsigned long flags;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);

	if (hiusbc->current_dr_mode == HIUSBC_DR_MODE_DEVICE) {
		if (!hiusbc->gadget_driver)
			goto exit;

		ret = hiusbc_init_phy(hiusbc);
		if (ret) {
			dev_err(hiusbc->dev,
				"failed to init phy during %s.\n", __func__);
			goto exit;
		}

		ret = hiusbc_run_stop(hiusbc, 1);
		if (ret) {
			hiusbc_run_stop(hiusbc, 0);
			goto exit;
		}
	}

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "-\n");

exit:
	spin_unlock_irqrestore(&hiusbc->lock, flags);
	return ret;
}

static int hiusbc_runtime_suspend(struct device *dev)
{
	struct hiusbc *hiusbc = dev_get_drvdata(dev);
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	if (hiusbc->current_dr_mode == HIUSBC_DR_MODE_DEVICE &&
		hiusbc->connected) {
		return -EBUSY;
	}

	ret = hiusbc_common_suspend(hiusbc);
	if (ret)
		return ret;

	device_init_wakeup(dev, true);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return ret;
}

static int hiusbc_runtime_resume(struct device *dev)
{
	struct hiusbc *hiusbc = dev_get_drvdata(dev);
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	device_init_wakeup(dev, false);

	ret = hiusbc_common_resume(hiusbc);
	if (ret)
		return ret;

	pm_runtime_mark_last_busy(dev);
	pm_runtime_put(dev);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return ret;
}

static int hiusbc_runtime_idle(struct device *dev)
{
	struct hiusbc *hiusbc = dev_get_drvdata(dev);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	if (hiusbc->current_dr_mode == HIUSBC_DR_MODE_DEVICE &&
		hiusbc->connected)
		return -EBUSY;

	pm_runtime_mark_last_busy(dev);
	pm_runtime_autosuspend(dev);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return 0;
}

static int hiusbc_suspend(struct device *dev)
{
	struct hiusbc *hiusbc = dev_get_drvdata(dev);
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	ret = hiusbc_common_suspend(hiusbc);
	if (ret)
		return ret;

	pinctrl_pm_select_sleep_state(dev);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return ret;
}

static int hiusbc_resume(struct device *dev)
{
	struct hiusbc *hiusbc = dev_get_drvdata(dev);
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	pinctrl_pm_select_default_state(dev);

	ret = hiusbc_common_resume(hiusbc);
	if (ret)
		return ret;

	pm_runtime_disable(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return 0;
}

static const struct dev_pm_ops hiusbc_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(hiusbc_suspend, hiusbc_resume)
	SET_RUNTIME_PM_OPS(hiusbc_runtime_suspend, hiusbc_runtime_resume,
			hiusbc_runtime_idle)
};

static const struct of_device_id of_hiusbc_match[] = {
	{ .compatible = "hisilicon,hiusbc" },
	{ .compatible = "hisi,hiusbc" },
};

static struct platform_driver hiusbc_driver = {
	.probe		= hiusbc_probe,
	.remove		= hiusbc_remove,
	.driver		= {
		.name	= "hiusbc",
		.pm	= &hiusbc_dev_pm_ops,
		.of_match_table	= of_hiusbc_match,
	},
};

module_platform_driver(hiusbc_driver);

MODULE_DESCRIPTION("Driver for the Hisilicon USB Controller");
MODULE_ALIAS("platform:hiusbc");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Wu Kaijie <wukaijie@hisilicon.com>");
