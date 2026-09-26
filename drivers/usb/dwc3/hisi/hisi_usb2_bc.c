/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Charger type detect based on BC1.2
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#include <linux/io.h>
#include <linux/hisi/usb/hisi_usb_helper.h>
#include "hisi_usb2_bc.h"
#include "dwc3-hisi.h"

void chip_bc_dplus_pulldown(struct chip_dwc3_device *chip_dwc)
{
	struct usb_ahbif_registers *ahbif =
		(struct usb_ahbif_registers *)chip_dwc->usb_phy->otg_bc_reg_base;
	union bc_ctrl4 bc_ctrl4;

	usb_dbg("+\n");
	/* enable BC */
	writel(1, &ahbif->bc_ctrl3);

	bc_ctrl4.reg = readl(&ahbif->bc_ctrl4);
	bc_ctrl4.bits.bc_dmpulldown = 1;
	bc_ctrl4.bits.bc_dppulldown = 1;
	writel(bc_ctrl4.reg, &ahbif->bc_ctrl4);

	usb_dbg("-\n");
}

void chip_bc_dplus_pullup(struct chip_dwc3_device *chip_dwc)
{
	struct usb_ahbif_registers *ahbif =
		(struct usb_ahbif_registers *)chip_dwc->usb_phy->otg_bc_reg_base;
	union bc_ctrl4 bc_ctrl4;

	usb_dbg("+\n");

	bc_ctrl4.reg = readl(&ahbif->bc_ctrl4);
	bc_ctrl4.bits.bc_dmpulldown = 0;
	bc_ctrl4.bits.bc_dppulldown = 0;
	writel(bc_ctrl4.reg, &ahbif->bc_ctrl4);
	/* disable BC */
	writel(0, &ahbif->bc_ctrl3);

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
	struct usb_ahbif_registers *ahbif =
		(struct usb_ahbif_registers *)chip_dwc3->usb_phy->otg_bc_reg_base;
	union bc_ctrl4 bc_ctrl4;
	union bc_ctrl5 bc_ctrl5;

	usb_dbg("+\n");
	if (chip_dwc3->vdp_src_enable == 0)
		return;
	chip_dwc3->vdp_src_enable = 0;

	bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
	bc_ctrl5.bits.bc_chrg_sel = 0;
	bc_ctrl5.bits.bc_vdat_src_en = 0;
	bc_ctrl5.bits.bc_vdat_det_en = 0;
	writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);

	/* bc_suspend = 1, nomal mode */
	bc_ctrl4.reg = readl(&ahbif->bc_ctrl4);
	bc_ctrl4.bits.bc_suspendm = 1;
	writel(bc_ctrl4.reg, &ahbif->bc_ctrl4);

	/* disable BC */
	writel(0, &ahbif->bc_ctrl3);
	usb_dbg("-\n");
}

void chip_bc_enable_vdp_src(struct chip_dwc3_device *chip_dwc3)
{
	struct usb_ahbif_registers *ahbif =
		(struct usb_ahbif_registers *)chip_dwc3->usb_phy->otg_bc_reg_base;
	union bc_ctrl5 bc_ctrl5;

	usb_dbg("+\n");
	if (chip_dwc3->vdp_src_enable == 1)
		return;
	chip_dwc3->vdp_src_enable = 1;

	bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
	bc_ctrl5.bits.bc_chrg_sel = 0;
	bc_ctrl5.bits.bc_vdat_src_en = 1;
	bc_ctrl5.bits.bc_vdat_det_en = 1;
	writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);
	usb_dbg("-\n");
}

static void usb2_bc_dcd(struct usb_ahbif_registers *ahbif, enum hisi_charger_type *type)
{
	union bc_ctrl4 bc_ctrl4;
	union bc_ctrl5 bc_ctrl5;
	unsigned long jiffies_expire;
	union bc_sts2 bc_sts2;
	int i = 0;
	/* phy suspend */
	bc_ctrl4.reg = readl(&ahbif->bc_ctrl4);
	bc_ctrl4.bits.bc_suspendm = 0;
	bc_ctrl4.bits.bc_dmpulldown = 1;
	writel(bc_ctrl4.reg, &ahbif->bc_ctrl4);

	/* enable DCD */
	bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
	bc_ctrl5.bits.bc_dcd_en = 1;
	writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);

	jiffies_expire = jiffies + msecs_to_jiffies(900); /* jiffies number */
	msleep(50); /* msleep time */
	while (i < 10) { /* loop number */
		bc_sts2.reg = readl(&ahbif->bc_sts2);
		if (bc_sts2.bits.bc_fs_vplus == 0) {
			i++;
		} else if (i) {
			usb_info("USB D+ D- not connected!\n");
			i = 0;
		}

		msleep(1);

		if (time_after(jiffies, jiffies_expire)) {
			usb_info("DCD timeout!\n");
			*type = CHARGER_TYPE_UNKNOWN;
			break;
		}
	}
	/* disable dcd */
	bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
	bc_ctrl5.bits.bc_dcd_en = 0;
	writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);
}

static void usb2_bc_detect_exit(struct chip_dwc3_device *chip_dwc3,
					enum hisi_charger_type type)
{
	/* If a PD detects that D+ is greater than VDAT_REF, it knows that it is
	 * attached to a DCP. It is then required to enable VDP_SRC or pull D+
	 * to VDP_UP through RDP_UP
	 */
	union bc_ctrl4 bc_ctrl4;
	struct usb_ahbif_registers *ahbif =
		(struct usb_ahbif_registers *)chip_dwc3->usb_phy->otg_bc_reg_base;
	if (type == CHARGER_TYPE_DCP) {
		usb_info("charger is DCP, enable VDP_SRC\n");
		chip_bc_enable_vdp_src(chip_dwc3);
	} else {
		/* bc_suspend = 1, nomal mode */
		bc_ctrl4.reg = readl(&ahbif->bc_ctrl4);
		bc_ctrl4.bits.bc_suspendm = 1;
		writel(bc_ctrl4.reg, &ahbif->bc_ctrl4);

		msleep(10);

		/* disable BC */
		writel(0, &ahbif->bc_ctrl3);
	}

	if (type == CHARGER_TYPE_CDP) {
		usb_info("it needs enable VDP_SRC while detect CDP!\n");
		chip_bc_enable_vdp_src(chip_dwc3);
	}
}

static void usb2_bc_primary_detect(struct usb_ahbif_registers *ahbif, enum hisi_charger_type *type)
{
	union bc_sts2 bc_sts2;
	union bc_ctrl5 bc_ctrl5;

	if (*type == CHARGER_TYPE_NONE) {
		/* enable vdect */
		bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
		bc_ctrl5.bits.bc_chrg_sel = 0;
		bc_ctrl5.bits.bc_vdat_src_en = 1;
		bc_ctrl5.bits.bc_vdat_det_en = 1;
		writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);

		msleep(40);

		/* we can detect sdp or cdp dcp */
		bc_sts2.reg = readl(&ahbif->bc_sts2);
		if (bc_sts2.bits.bc_chg_det == 0)
			*type = CHARGER_TYPE_SDP;

		/* disable vdect */
		bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
		bc_ctrl5.bits.bc_vdat_src_en = 0;
		bc_ctrl5.bits.bc_vdat_det_en = 0;
		writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);
	}
}

static void usb2_bc_second_detect(struct usb_ahbif_registers *ahbif, enum hisi_charger_type *type)
{
	union bc_sts2 bc_sts2;
	union bc_ctrl5 bc_ctrl5;

	if (*type == CHARGER_TYPE_NONE) {
		/* enable vdect */
		bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
		bc_ctrl5.bits.bc_chrg_sel = 1;
		bc_ctrl5.bits.bc_vdat_src_en = 1;
		bc_ctrl5.bits.bc_vdat_det_en = 1;
		writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);

		msleep(40);

		/* we can detect sdp or cdp dcp */
		bc_sts2.reg = readl(&ahbif->bc_sts2);
		if (bc_sts2.bits.bc_chg_det == 0)
			*type = CHARGER_TYPE_CDP;
		else
			*type = CHARGER_TYPE_DCP;

		/* disable vdect */
		bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
		bc_ctrl5.bits.bc_chrg_sel = 0;
		bc_ctrl5.bits.bc_vdat_src_en = 0;
		bc_ctrl5.bits.bc_vdat_det_en = 0;
		writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);
	}
}

enum hisi_charger_type detect_charger_type(struct chip_dwc3_device *chip_dwc3)
{
	struct usb_ahbif_registers *ahbif =
		(struct usb_ahbif_registers *)chip_dwc3->usb_phy->otg_bc_reg_base;
	enum hisi_charger_type type = CHARGER_TYPE_NONE;

	usb_info("+\n");

	/* enable BC */
	writel(1, &ahbif->bc_ctrl3);
	/* USB2_DCD */
	usb2_bc_dcd(ahbif, &type);
	usb_info("DCD done\n");
	usb2_bc_primary_detect(ahbif, &type);
	usb_info("Primary Detection done\n");
	usb2_bc_second_detect(ahbif, &type);
	usb_info("Secondary Detection done\n");

	usb2_bc_detect_exit(chip_dwc3, type);
	chip_dwc3->charger_type = type;
	usb_info("charger type: %s\n", charger_type_string(type));
	usb_info("-\n");
	return type;
}
