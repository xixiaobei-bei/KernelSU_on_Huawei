/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Charger type detect based on BC1.2
 * Create: 2019-6-16
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/hisi/usb/hisi_usb_helper.h>

#include "dwc3-hisi.h"
#include "hisi_usb_bc12.h"

int chip_bc_is_bcmode_on(void)
{
	void __iomem *base = NULL;

	volatile u32 reg;

	if (!chip_dwc3_dev || !chip_dwc3_dev->usb_phy)
		return false;

	base = chip_dwc3_dev->usb_phy->otg_bc_reg_base;
	reg = readl(base + BC_CTRL1);
	return (reg & BC_CTRL1_BC_MODE) != 0;
}

void __iomem *chip_bc_get_bc_ctrl(struct chip_dwc3_device *chip_dwc)
{
	if (!chip_dwc3_dev->usb_phy->get_bc_ctrl_reg)
		return NULL;

	return chip_dwc3_dev->usb_phy->get_bc_ctrl_reg();
}

void chip_bc_dplus_pulldown(struct chip_dwc3_device *chip_dwc)
{
	void __iomem *base = NULL;
	volatile u32 reg;

	usb_dbg("+\n");
	if (!chip_dwc || !chip_dwc->usb_phy)
		return;

	base = chip_dwc->usb_phy->otg_bc_reg_base;

	/* enable BC */
	writel(BC_CTRL1_BC_MODE, base + BC_CTRL1);
	reg = readl(base + BC_CTRL0);
	/* bc reg enable, move 7 bit, move 8 bit */
	reg |= ((1u << 7) | (1u << 8));
	writel(reg, base + BC_CTRL0);

	usb_dbg("-\n");
}

void chip_bc_dplus_pullup(struct chip_dwc3_device *chip_dwc)
{
	void __iomem *base = NULL;
	volatile u32 reg;

	usb_dbg("+\n");
	if (!chip_dwc || !chip_dwc->usb_phy)
		return;

	base = chip_dwc->usb_phy->otg_bc_reg_base;
	reg = readl(base + BC_CTRL0);
	/* bc ctrl0 reg, move 7 bit, move 8 bit */
	reg &= (~((1u << 7) | (1u << 8)));
	writel(reg, base + BC_CTRL0);
	/* disable BC */
	writel((readl(base + BC_CTRL1) & ~BC_CTRL1_BC_MODE), base + BC_CTRL1);

	usb_dbg("-\n");
}

/*
 * BC1.2 Spec:
 * If a PD detects that D+ is greater than VDAT_REF, it knows that it is
 * attached to a DCP. It is then required to enable VDP_SRC or pull D+
 * to VDP_UP through RDP_UP
 */
void chip_bc_disable_vdp_src(struct chip_dwc3_device *chip_dwc3)
{
	void __iomem *base = NULL;
	void __iomem *bc_ctrl2 = NULL;

	u32 reg;

	usb_dbg("+\n");

	if (!chip_dwc3 || !chip_dwc3->usb_phy)
		return;

	if (chip_dwc3->vdp_src_enable == 0)
		return;
	chip_dwc3->vdp_src_enable = 0;

	base = chip_dwc3->usb_phy->otg_bc_reg_base;
	bc_ctrl2 = chip_bc_get_bc_ctrl(chip_dwc3);
	if (!bc_ctrl2) {
		usb_err("bc_ctrl2 is null\n");
		return;
	}
	usb_dbg("diaable VDP_SRC\n");

	reg = readl(bc_ctrl2);
	reg &= ~(BC_CTRL2_BC_PHY_VDATARCENB | BC_CTRL2_BC_PHY_VDATDETENB);
	writel(reg, bc_ctrl2);

	reg = readl(base + BC_CTRL0);
	reg |= BC_CTRL0_BC_SUSPEND_N;
	writel(reg, base + BC_CTRL0);

	writel((readl(base + BC_CTRL1) & ~BC_CTRL1_BC_MODE), base + BC_CTRL1);
	usb_dbg("-\n");
}

void chip_bc_enable_vdp_src(struct chip_dwc3_device *chip_dwc3)
{
	void __iomem *bc_ctrl2 = NULL;
	u32 reg;

	usb_dbg("+\n");

	if (!chip_dwc3 || !chip_dwc3->usb_phy)
		return;

	if (chip_dwc3->vdp_src_enable != 0)
		return;
	chip_dwc3->vdp_src_enable = 1;

	bc_ctrl2 = chip_bc_get_bc_ctrl(chip_dwc3);
	if (!bc_ctrl2) {
		usb_err("bc_ctrl2 is null\n");
		return;
	}
	usb_dbg("enable VDP_SRC\n");
	reg = readl(bc_ctrl2);
	reg &= ~BC_CTRL2_BC_PHY_CHRGSEL;
	reg |= (BC_CTRL2_BC_PHY_VDATARCENB | BC_CTRL2_BC_PHY_VDATDETENB);
	writel(reg, bc_ctrl2);
	usb_dbg("-\n");
}

static int is_dcd_timeout(const void __iomem *base)
{
	unsigned long jiffies_expire;
	u32 reg;
	int ret = 0;
	int i = 0;

	jiffies_expire = jiffies + msecs_to_jiffies(900);
	msleep(50);
	while (1) {
		reg = readl(base + BC_STS0);
		if ((reg & BC_STS0_BC_PHY_FSVPLUS) == 0) {
			i++;
			if (i >= 10)
				break;
		} else {
			i = 0;
		}

		msleep(10);

		if (time_after(jiffies, jiffies_expire)) {
			usb_dbg("DCD timeout!\n");
			ret = -1;
			break;
		}
	}

	return ret;
}

static void usb3_bc_dcd(struct chip_dwc3_device *chip_dwc3, void __iomem *bc_ctrl2,
	enum hisi_charger_type *type)
{
	void __iomem *base = NULL;
	u32 reg;
	unsigned long flags;

	base = chip_dwc3->usb_phy->otg_bc_reg_base;
	reg = readl(base + BC_CTRL0);
	reg &= ~BC_CTRL0_BC_SUSPEND_N;
	writel(reg, base + BC_CTRL0);

	spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
	/* enable DCD */
	reg = readl(bc_ctrl2);
	reg |= BC_CTRL2_BC_PHY_DCDENB;
	writel(reg, bc_ctrl2);
	spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);

	reg = readl(base + BC_CTRL0);
	reg |= BC_CTRL0_BC_DMPULLDOWN;
	writel(reg, base + BC_CTRL0);

	if (is_dcd_timeout(base))
		*type = CHARGER_TYPE_UNKNOWN;

	reg = readl(base + BC_CTRL0);
	reg &= ~BC_CTRL0_BC_DMPULLDOWN;
	writel(reg, base + BC_CTRL0);

	spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
	/* disable DCD */
	reg = readl(bc_ctrl2);
	reg &= ~BC_CTRL2_BC_PHY_DCDENB;
	writel(reg, bc_ctrl2);
	spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);
}

static void usb3_bc_primary_detect(struct chip_dwc3_device *chip_dwc3, void __iomem *bc_ctrl2,
	enum hisi_charger_type *type)
{
	void __iomem *base = NULL;
	u32 reg;
	unsigned long flags;

	base = chip_dwc3->usb_phy->otg_bc_reg_base;
	if (*type == CHARGER_TYPE_NONE) {
		/* enable vdect */
		spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
		reg = readl(bc_ctrl2);
		reg &= ~BC_CTRL2_BC_PHY_CHRGSEL;
		reg |= (BC_CTRL2_BC_PHY_VDATARCENB | BC_CTRL2_BC_PHY_VDATDETENB);
		writel(reg, bc_ctrl2);
		spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);

		msleep(40);

		/* we can detect sdp or cdp dcp */
		reg = readl(base + BC_STS0);
		if ((reg & BC_STS0_BC_PHY_CHGDET) == 0)
			*type = CHARGER_TYPE_SDP;

		spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
		/* disable vdect */
		reg = readl(bc_ctrl2);
		reg &= ~(BC_CTRL2_BC_PHY_VDATARCENB | BC_CTRL2_BC_PHY_VDATDETENB);
		writel(reg, bc_ctrl2);
		spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);
	}
}

static void usb3_bc_second_detect(struct chip_dwc3_device *chip_dwc3, void __iomem *bc_ctrl2,
	enum hisi_charger_type *type)
{
	void __iomem *base = NULL;
	u32 reg;
	unsigned long flags;

	base = chip_dwc3->usb_phy->otg_bc_reg_base;
	if (*type == CHARGER_TYPE_NONE) {
		/* enable vdect */
		spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
		reg = readl(bc_ctrl2);
		reg |= (BC_CTRL2_BC_PHY_VDATARCENB | BC_CTRL2_BC_PHY_VDATDETENB |
		BC_CTRL2_BC_PHY_CHRGSEL);
		writel(reg, bc_ctrl2);
		spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);

		msleep(40);

		/* we can detect sdp or cdp dcp */
		reg = readl(base + BC_STS0);
		if ((reg & BC_STS0_BC_PHY_CHGDET) == 0)
			*type = CHARGER_TYPE_CDP;
		else
			*type = CHARGER_TYPE_DCP;

		spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
		/* disable vdect */
		reg = readl(bc_ctrl2);
		reg &= ~(BC_CTRL2_BC_PHY_VDATARCENB | BC_CTRL2_BC_PHY_VDATDETENB |
			BC_CTRL2_BC_PHY_CHRGSEL);
		writel(reg, bc_ctrl2);
		spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);
	}
}

static void usb3_bc_detect_exit(struct chip_dwc3_device *chip_dwc3, enum hisi_charger_type type)
{
	/*
	 * If a PD detects that D+ is greater than VDAT_REF, it knows that it is
	 * attached to a DCP. It is then required to enable VDP_SRC or pull D+
	 * to VDP_UP through RDP_UP
	 */
	void __iomem *base = NULL;
	unsigned long flags;
	u32 reg;

	base = chip_dwc3->usb_phy->otg_bc_reg_base;
	if (type == CHARGER_TYPE_DCP) {
		usb_dbg("charger is DCP, enable VDP_SRC\n");
		/* customize if keep D+ and D- short after bc1.2 */
		if (!chip_dwc3->vdp_src_disable) {
			spin_lock_irqsave(&chip_dwc3->bc_again_lock,
				flags);
			chip_bc_enable_vdp_src(chip_dwc3);
			spin_unlock_irqrestore(&chip_dwc3->bc_again_lock,
				flags);
		}
	} else {
		/* bc_suspend = 1, nomal mode */
		reg = readl(base + BC_CTRL0);
		reg |= BC_CTRL0_BC_SUSPEND_N;
		writel(reg, base + BC_CTRL0);

		msleep(10);

		/* disable BC */
		writel((readl(base + BC_CTRL1) & ~BC_CTRL1_BC_MODE), base + BC_CTRL1);
	}

	if (type == CHARGER_TYPE_CDP) {
		usb_info("it needs enable VDP_SRC while detect CDP!\n");
		spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
		chip_bc_enable_vdp_src(chip_dwc3);
		spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);
	}
}

enum hisi_charger_type detect_charger_type(struct chip_dwc3_device *chip_dwc3)
{
	enum hisi_charger_type type = CHARGER_TYPE_NONE;
	void __iomem *base = NULL;
	void __iomem *bc_ctrl2 = NULL;

	usb_info("+\n");

	if (!chip_dwc3 || !chip_dwc3->usb_phy)
		return CHARGER_TYPE_NONE;

	base = chip_dwc3->usb_phy->otg_bc_reg_base;
	bc_ctrl2 = chip_bc_get_bc_ctrl(chip_dwc3);
	if (!bc_ctrl2) {
		usb_err("bc_ctrl2 is null\n");
		return CHARGER_TYPE_NONE;
	}
	writel(BC_CTRL1_BC_MODE, base + BC_CTRL1); /* Enable BC */

	usb3_bc_dcd(chip_dwc3, bc_ctrl2, &type);
	usb_info("DCD done\n");
	usb3_bc_primary_detect(chip_dwc3, bc_ctrl2, &type);
	usb_info("Primary Detection done\n");
	usb3_bc_second_detect(chip_dwc3, bc_ctrl2, &type);
	usb_info("Secondary Detection done\n");

	usb3_bc_detect_exit(chip_dwc3, type);
	usb_info("charger type: %s\n", charger_type_string(type));
	usb_info("-\n");
	return type;
}
