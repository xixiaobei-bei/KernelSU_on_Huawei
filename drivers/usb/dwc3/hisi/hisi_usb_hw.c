// SPDX-License-Identifier: GPL-2.0
/*
 * Hardware config for Hisilicon USB
 *
 * Authors: Yu Chen <chenyu56@huawei.com>
 */

#include <linux/clk.h>
#include <linux/phy/phy.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/usb/hisi_usb_helper.h>
#include <linux/hisi/usb/hisi_usb_phy.h>
#include <linux/hisi/usb/hisi_usb_reg_cfg.h>
#include "dwc3-hisi.h"
#include "hisi_usb_bc12.h"

int hisi_usb2_phy_init(struct phy *usb2_phy, unsigned int host_mode)
{
	int ret;

	if (!usb2_phy)
		return -EINVAL;

	ret = phy_power_on(usb2_phy);
	if (ret) {
		usb_err("usb2 phy poweron failed\n");
		return ret;
	}

	if (host_mode)
		ret = phy_set_mode(usb2_phy, PHY_MODE_USB_HOST);
	else
		ret = phy_set_mode(usb2_phy, PHY_MODE_USB_DEVICE);
	if (ret) {
		usb_err("usb2 phy set mode failed\n");
		if (phy_power_off(usb2_phy))
			usb_err("usb2 phy poweroff failed\n");
		return ret;
	}

	ret = phy_init(usb2_phy);
	if (ret) {
		usb_err("usb2 phy init failed\n");
		if (phy_power_off(usb2_phy))
			usb_err("usb2 phy poweroff failed\n");
	}

	return ret;
}

int hisi_usb2_phy_exit(struct phy *usb2_phy)
{
	int ret;

	if (!usb2_phy)
		return -EINVAL;

	ret = phy_exit(usb2_phy);
	if (ret) {
		usb_err("usb2 phy exit failed\n");
		return ret;
	}

	ret = phy_power_off(usb2_phy);
	if (ret) {
		usb_err("usb2 phy poweroff failed\n");
		if (phy_init(usb2_phy))
			usb_err("usb2 phy init failed\n");
	}

	return ret;
}

int hisi_usb2_shared_phy_init(struct phy *usb2_phy)
{
	int ret;
	struct hisi_usb2_phy *hisi_usb2_phy = NULL;

	if (!usb2_phy)
		return -EINVAL;

	hisi_usb2_phy = (struct hisi_usb2_phy *)phy_get_drvdata(usb2_phy);
	if (!hisi_usb2_phy || !hisi_usb2_phy->shared_phy_init)
		return -EINVAL;

	if (usb2_phy->pwr && regulator_is_enabled(usb2_phy->pwr) == 0) {
		usb_info("enable usb2 phy 3.2v power\n");
		ret = phy_power_on(usb2_phy);
		if (ret) {
			usb_err("usb2 phy poweron failed\n");
			return ret;
		}
	}

	ret = phy_set_mode(usb2_phy, PHY_MODE_USB_HOST);
	if (ret) {
		usb_err("usb2 phy set mode failed\n");
		if (phy_power_off(usb2_phy))
			usb_err("usb2 phy poweroff failed\n");
		return ret;
	}

	ret = hisi_usb2_phy->shared_phy_init(hisi_usb2_phy);
	if (ret) {
		usb_err("usb2 shard phy init failed\n");
		if (phy_power_off(usb2_phy))
			usb_err("usb2 phy poweroff failed\n");
	}

	return ret;
}

int hisi_usb2_shared_phy_exit(struct phy *usb2_phy, unsigned int keep_power)
{
	int ret;
	struct hisi_usb2_phy *hisi_usb2_phy = NULL;

	if (!usb2_phy)
		return -EINVAL;

	hisi_usb2_phy = (struct hisi_usb2_phy *)phy_get_drvdata(usb2_phy);
	if (!hisi_usb2_phy || !hisi_usb2_phy->shared_phy_exit)
		return -EINVAL;

	ret = hisi_usb2_phy->shared_phy_exit(hisi_usb2_phy);
	if (ret) {
		usb_err("usb2 shared phy exit failed\n");
		return ret;
	}

	if (keep_power == 0) {
		usb_info("disable usb2 phy 3.2v power\n");
		ret = phy_power_off(usb2_phy);
		if (ret) {
			usb_err("usb2 shared phy poweroff failed\n");
			if (hisi_usb2_phy->shared_phy_init &&
					hisi_usb2_phy->shared_phy_init(
						hisi_usb2_phy))
				usb_err("usb2 shared phy init failed\n");
		}
	}
	return ret;
}

static int hisi_usb_get_controller_res(struct hisi_dwc3_device *hisi_usb,
		struct device *dev)
{
	int ret;

	ret = hisi_usb_get_clks(dev, &hisi_usb->controller_clks,
			&hisi_usb->num_controller_clks);
	if (ret)
		return ret;

	hisi_usb->controller_reset =
		of_get_hisi_usb_reg_cfg_by_name(dev->of_node,
				"controller-reset");
	if (!hisi_usb->controller_reset)
		return -ENOENT;

	hisi_usb->controller_unreset =
		of_get_hisi_usb_reg_cfg_by_name(dev->of_node,
				"controller-unreset");
	if (!hisi_usb->controller_unreset) {
		of_remove_hisi_usb_reg_cfg(hisi_usb->controller_reset);
		return -ENOENT;
	}

	return 0;
}

int hisi_usb_get_hw_res(struct hisi_dwc3_device *hisi_usb, struct device *dev)
{
	int ret;

	if (!hisi_usb || !dev)
		return -EINVAL;

	hisi_usb->usb2_phy = devm_phy_get(dev, "usb2-phy");
	if (IS_ERR(hisi_usb->usb2_phy)) {
		usb_err("no usb2 phy configured\n");
		return -EPROBE_DEFER;
	}

	hisi_usb->usb3_phy = devm_phy_get(dev, "usb3-phy");
	if (IS_ERR(hisi_usb->usb3_phy)) {
		ret = PTR_ERR(hisi_usb->usb3_phy);
		if (ret == -ENODEV) {
			hisi_usb->usb3_phy = NULL;
		} else if (ret == -EPROBE_DEFER) {
			return ret;
		} else {
			usb_err("no usb3 phy configured\n");
			return ret;
		}
	}

	return hisi_usb_get_controller_res(hisi_usb, dev);
}

static void hisi_usb_put_controller_res(struct hisi_dwc3_device *hisi_usb)
{
	of_remove_hisi_usb_reg_cfg(hisi_usb->controller_reset);
	of_remove_hisi_usb_reg_cfg(hisi_usb->controller_unreset);
}

void hisi_usb_put_hw_res(struct hisi_dwc3_device *hisi_usb)
{
	hisi_usb_put_controller_res(hisi_usb);
}

enum hisi_charger_type hisi_usb_detect_charger_type(
		struct hisi_dwc3_device *hisi_dwc3)
{
	struct hisi_usb2_phy *hisi_usb2_phy = NULL;

	if (!hisi_dwc3)
		return CHARGER_TYPE_NONE;

	if (hisi_dwc3->fpga_flag) {
		usb_dbg("this is fpga platform, charger is SDP\n");
		return CHARGER_TYPE_SDP;
	}

	if (hisi_dwc3->fake_charger_type != CHARGER_TYPE_NONE) {
		usb_dbg("fake type: %d\n", hisi_dwc3->fake_charger_type);
		return hisi_dwc3->fake_charger_type;
	}

	if (hisi_dwc3->use_new_frame) {
		if (!hisi_dwc3->usb2_phy)
			return CHARGER_TYPE_NONE;

		hisi_usb2_phy = (struct hisi_usb2_phy *)
			phy_get_drvdata(hisi_dwc3->usb2_phy);
		if (!hisi_usb2_phy || !hisi_usb2_phy->detect_charger_type)
			return CHARGER_TYPE_NONE;

		return hisi_usb2_phy->detect_charger_type(hisi_usb2_phy);
	} else {
		return detect_charger_type(hisi_dwc3);
	}
}

void hisi_usb_dpdm_pulldown(struct hisi_dwc3_device *hisi_dwc3)
{
	struct hisi_usb2_phy *hisi_usb2_phy = NULL;

	usb_dbg("+\n");
	if (!hisi_dwc3)
		return;

	if (hisi_dwc3->use_new_frame) {
		if (!hisi_dwc3->usb2_phy)
			return;

		hisi_usb2_phy = (struct hisi_usb2_phy *)
			phy_get_drvdata(hisi_dwc3->usb2_phy);
		if (!hisi_usb2_phy || !hisi_usb2_phy->set_dpdm_pulldown)
			return;

		hisi_usb2_phy->set_dpdm_pulldown(hisi_usb2_phy, true);
	} else {
		hisi_bc_dplus_pulldown(hisi_dwc3);
	}
	usb_dbg("-\n");
}

void hisi_usb_dpdm_pullup(struct hisi_dwc3_device *hisi_dwc3)
{
	struct hisi_usb2_phy *hisi_usb2_phy = NULL;

	usb_dbg("+\n");
	if (!hisi_dwc3)
		return;

	if (hisi_dwc3->use_new_frame) {
		if (!hisi_dwc3->usb2_phy)
			return;

		hisi_usb2_phy = (struct hisi_usb2_phy *)
			phy_get_drvdata(hisi_dwc3->usb2_phy);
		if (!hisi_usb2_phy || !hisi_usb2_phy->set_dpdm_pulldown)
			return;

		hisi_usb2_phy->set_dpdm_pulldown(hisi_usb2_phy, false);
	} else {
		hisi_bc_dplus_pullup(hisi_dwc3);
	}
	usb_dbg("-\n");
}

void hisi_usb_disable_vdp_src(struct hisi_dwc3_device *hisi_dwc3)
{
	usb_dbg("+\n");

	if (!hisi_dwc3)
		return;

	if (hisi_dwc3->use_new_frame) {
		struct hisi_usb2_phy *hisi_usb2_phy = NULL;

		if (!hisi_dwc3->usb2_phy)
			return;

		hisi_usb2_phy = (struct hisi_usb2_phy *)
			phy_get_drvdata(hisi_dwc3->usb2_phy);
		if (!hisi_usb2_phy || !hisi_usb2_phy->set_vdp_src)
			return;

		hisi_usb2_phy->set_vdp_src(hisi_usb2_phy, false);

		usb_dbg("-\n");
	} else {
		unsigned long flags;

		spin_lock_irqsave(&hisi_dwc3->bc_again_lock, flags);
		hisi_bc_disable_vdp_src(hisi_dwc3);
		spin_unlock_irqrestore(&hisi_dwc3->bc_again_lock, flags);
	}
}

void hisi_usb_enable_vdp_src(struct hisi_dwc3_device *hisi_dwc3)
{
	usb_dbg("+\n");

	if (!hisi_dwc3)
		return;

	if (hisi_dwc3->use_new_frame) {
		struct hisi_usb2_phy *hisi_usb2_phy = NULL;

		if (!hisi_dwc3->usb2_phy)
			return;

		hisi_usb2_phy = (struct hisi_usb2_phy *)
			phy_get_drvdata(hisi_dwc3->usb2_phy);
		if (!hisi_usb2_phy || !hisi_usb2_phy->set_vdp_src)
			return;

		hisi_usb2_phy->set_vdp_src(hisi_usb2_phy, true);

		usb_dbg("-\n");
	} else {
		unsigned long flags;

		spin_lock_irqsave(&hisi_dwc3->bc_again_lock, flags);
		hisi_bc_enable_vdp_src(hisi_dwc3);
		spin_unlock_irqrestore(&hisi_dwc3->bc_again_lock, flags);
	}
}

int hisi_usb_controller_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret, i;

	if (!hisi_dwc3)
		return -EINVAL;

	if (hisi_dwc3->num_controller_clks && !hisi_dwc3->controller_clks)
		return -EINVAL;

	for (i = 0; i < hisi_dwc3->num_controller_clks; i++) {
		ret = clk_prepare_enable(hisi_dwc3->controller_clks[i]);
		if (ret) {
			usb_err("clk enable failed\n");
			while (--i >= 0)
				clk_disable_unprepare(
						hisi_dwc3->controller_clks[i]);
			return ret;
		}
	}

	if (hisi_dwc3->controller_unreset)
		return hisi_usb_reg_write(hisi_dwc3->controller_unreset);

	/* require to delay 10ms */
	mdelay(10);

	return 0;
}

int hisi_usb_controller_exit(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret, i;

	if (!hisi_dwc3)
		return -EINVAL;

	if (hisi_dwc3->num_controller_clks && !hisi_dwc3->controller_clks)
		return -EINVAL;

	if (hisi_dwc3->controller_reset) {
		ret = hisi_usb_reg_write(hisi_dwc3->controller_reset);
		if (ret)
			return ret;
	}

	for (i = 0; i < hisi_dwc3->num_controller_clks; i++)
		clk_disable_unprepare(hisi_dwc3->controller_clks[i]);

	return 0;
}
