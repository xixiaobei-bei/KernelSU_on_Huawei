#include <linux/io.h>

#include "hisi_usb2_bc.h"
#include "dwc3-hisi.h"

void hisi_bc_dplus_pulldown(struct hisi_dwc3_device *hisi_dwc)
{
	struct usb_ahbif_registers *ahbif
		= (struct usb_ahbif_registers *)hisi_dwc->otg_bc_reg_base;
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

void hisi_bc_dplus_pullup(struct hisi_dwc3_device *hisi_dwc)
{
	struct usb_ahbif_registers *ahbif
		= (struct usb_ahbif_registers *)hisi_dwc->otg_bc_reg_base;
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

/* BC1.2 Spec:
 * If a PD detects that D+ is greater than VDAT_REF, it knows that it is
 * attached to a DCP. It is then required to enable VDP_SRC or pull D+
 * to VDP_UP through RDP_UP */
void hisi_bc_disable_vdp_src(struct hisi_dwc3_device *hisi_dwc3)
{
	struct usb_ahbif_registers *ahbif
		= (struct usb_ahbif_registers *)hisi_dwc3->otg_bc_reg_base;
	union bc_ctrl4 bc_ctrl4;
	union bc_ctrl5 bc_ctrl5;

	usb_dbg("+\n");
	if (hisi_dwc3->vdp_src_enable == 0)
		return;
	hisi_dwc3->vdp_src_enable = 0;

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

void hisi_bc_enable_vdp_src(struct hisi_dwc3_device *hisi_dwc3)
{
	struct usb_ahbif_registers *ahbif
		= (struct usb_ahbif_registers *)hisi_dwc3->otg_bc_reg_base;
	union bc_ctrl5 bc_ctrl5;

	usb_dbg("+\n");
	if (hisi_dwc3->vdp_src_enable == 1)
		return;
	hisi_dwc3->vdp_src_enable = 1;

	bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
	bc_ctrl5.bits.bc_chrg_sel = 0;
	bc_ctrl5.bits.bc_vdat_src_en = 1;
	bc_ctrl5.bits.bc_vdat_det_en = 1;
	writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);
	usb_dbg("-\n");
}

enum hisi_charger_type detect_charger_type(struct hisi_dwc3_device *hisi_dwc3)
{
	struct usb_ahbif_registers *ahbif
		= (struct usb_ahbif_registers *)hisi_dwc3->otg_bc_reg_base;
	enum hisi_charger_type type = CHARGER_TYPE_NONE;
	union bc_ctrl4 bc_ctrl4;
	union bc_ctrl5 bc_ctrl5;
	union bc_sts2 bc_sts2;
	unsigned long jiffies_expire;
	int i = 0;

	usb_dbg("+\n");

	if (hisi_dwc3->fpga_flag) {
		usb_dbg("this is fpga platform, charger is SDP\n");
		return CHARGER_TYPE_SDP;
	}

	if (hisi_dwc3->fake_charger_type != CHARGER_TYPE_NONE) {
		usb_dbg("fake type: %d\n", hisi_dwc3->fake_charger_type);
		hisi_dwc3->charger_type = hisi_dwc3->fake_charger_type;
		return hisi_dwc3->fake_charger_type;
	}

	/* enable BC */
	writel(1, &ahbif->bc_ctrl3);

	/* phy suspend */
	bc_ctrl4.reg = readl(&ahbif->bc_ctrl4);
	bc_ctrl4.bits.bc_suspendm = 0;
	bc_ctrl4.bits.bc_dmpulldown = 1;
	writel(bc_ctrl4.reg, &ahbif->bc_ctrl4);

	/* enable DCD */
	bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
	bc_ctrl5.bits.bc_dcd_en = 1;
	writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);

	jiffies_expire = jiffies + msecs_to_jiffies(900);
	msleep(50);
	while (i < 10) {
		bc_sts2.reg = readl(&ahbif->bc_sts2);
		if (bc_sts2.bits.bc_fs_vplus == 0) {
			i++;
		} else if (i) {
			usb_dbg("USB D+ D- not connected!\n");
			i = 0;
		}

		msleep(1);

		if (time_after(jiffies, jiffies_expire)) {
			usb_dbg("DCD timeout!\n");
			type = CHARGER_TYPE_UNKNOWN;
			break;
		}
	}

	/* disable DCD */
	bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
	bc_ctrl5.bits.bc_dcd_en = 0;
	writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);

	usb_dbg("DCD done\n");

	if (type == CHARGER_TYPE_NONE) {
		/* enable vdect */
		bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
		bc_ctrl5.bits.bc_chrg_sel = 0;
		bc_ctrl5.bits.bc_vdat_src_en = 1;
		bc_ctrl5.bits.bc_vdat_det_en = 1;
		writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);

		msleep(40);

		/* we can detect sdp or cdp dcp */
		bc_sts2.reg = readl(&ahbif->bc_sts2);
		if (bc_sts2.bits.bc_chg_det == 0) {
			type = CHARGER_TYPE_SDP;
		}

		/* disable vdect */
		bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
		bc_ctrl5.bits.bc_vdat_src_en = 0;
		bc_ctrl5.bits.bc_vdat_det_en = 0;
		writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);
	}

	usb_dbg("Primary Detection done\n");

	if (type == CHARGER_TYPE_NONE) {
		/* enable vdect */
		bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
		bc_ctrl5.bits.bc_chrg_sel = 1;
		bc_ctrl5.bits.bc_vdat_src_en = 1;
		bc_ctrl5.bits.bc_vdat_det_en = 1;
		writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);

		msleep(40);

		/* we can detect sdp or cdp dcp */
		bc_sts2.reg = readl(&ahbif->bc_sts2);
		if (bc_sts2.bits.bc_chg_det == 0) {
			type = CHARGER_TYPE_CDP;
		} else {
			type = CHARGER_TYPE_DCP;
		}

		/* disable vdect */
		bc_ctrl5.reg = readl(&ahbif->bc_ctrl5);
		bc_ctrl5.bits.bc_chrg_sel = 0;
		bc_ctrl5.bits.bc_vdat_src_en = 0;
		bc_ctrl5.bits.bc_vdat_det_en = 0;
		writel(bc_ctrl5.reg, &ahbif->bc_ctrl5);
	}

	usb_dbg("Secondary Detection done\n");

	/* If a PD detects that D+ is greater than VDAT_REF, it knows that it is
	 * attached to a DCP. It is then required to enable VDP_SRC or pull D+
	 * to VDP_UP through RDP_UP */
	if (type == CHARGER_TYPE_DCP) {
		usb_dbg("charger is DCP, enable VDP_SRC\n");
		hisi_bc_enable_vdp_src(hisi_dwc3);
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
		usb_dbg("it needs enable VDP_SRC while detect CDP!\n");
		hisi_bc_enable_vdp_src(hisi_dwc3);
	}

	hisi_dwc3->charger_type = type;
	usb_dbg("charger type: %s\n", charger_type_string(type));
	usb_dbg("-\n");
	return type;
}

