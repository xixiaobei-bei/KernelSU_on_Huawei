/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Hardware config for USB.
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include "hisi_usb_hw.h"
#include <securec.h>
#include <linux/clk.h>
#include <linux/hisi/usb/hisi_usb_helper.h>
#include <linux/hisi/usb/hisi_usb_reg_cfg.h>
#include <linux/hisi/usb/tca.h>
#include <linux/of_platform.h>
#include "hisi_usb_bc12.h"
#include "combophy_core.h"

#define USBC_NUM_MAX 20 /* usb controller num limit */

void chip_usb3_phy_dump_info(struct chip_dwc3_device *chip_dwc)
{
	usb_info("+\n");
	if (!chip_dwc || !chip_dwc->usb3_phy)
		return;

	combophy_regdump(chip_dwc->usb3_phy);
	usb_info("-\n");
}

int chip_usb3_phy_init(struct phy *phy,
		       enum tcpc_mux_ctrl_type mode_type,
		       enum typec_plug_orien_e typec_orien)
{
	int ret;

	usb_info("+\n");
	/* usb3 phy is optional */
	if (!phy)
		return 0;

	ret = combophy_set_mode(phy, mode_type, typec_orien);
	if (ret) {
		usb_err("combophy set mode failed\n");
		return ret;
	}

	ret = combophy_init(phy);
	if (ret) {
		usb_err("combophy init failed\n");
		return ret;
	}
	usb_info("-\n");

	return 0;
}

int chip_usb3_phy_exit(struct phy *phy)
{
	int ret;

	usb_info("+\n");
	/* usb3 phy is optional */
	if (!phy)
		return 0;

	ret = combophy_exit(phy);
	if (ret) {
		usb_err("combophy exit failed\n");
		return ret;
	}

	ret = combophy_set_mode(phy, TCPC_NC, TYPEC_ORIEN_POSITIVE);
	if (ret) {
		usb_err("combophy set mode failed\n");
		return ret;
	}

	usb_info("-\n");
	return 0;
}

int chip_usb2_phy_init(struct phy *usb2_phy, bool host_mode)
{
	int ret;

	usb_info("+\n");
	if (!usb2_phy)
		return -EINVAL;

	if (usb2_phy->power_count == 0) {
		ret = phy_power_on(usb2_phy);
		if (ret) {
			usb_err("usb2 phy poweron failed\n");
			return ret;
		}
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

	usb_info("-\n");
	return ret;
}

int chip_usb2_phy_exit(struct phy *usb2_phy)
{
	int ret;

	usb_info("+\n");
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

	usb_info("-\n");
	return ret;
}

static int chip_usb_get_controller_clks(struct device_node *node,
			struct usb_controller *c)
{
	if (c->stub_clk)
		return 0;

	return chip_usb_get_clks(node, &c->clks, &c->num_clks);
}

static void chip_usb_put_controller_clks(struct usb_controller *c)
{
	if (c->stub_clk)
		return;

	chip_usb_put_clks(c->clks, c->num_clks);
}

static int chip_usb_get_controller_ops(struct device_node *node,
			struct usb_controller *c)
{
	c->reset = of_get_chip_usb_reg_cfg(node, "reset");
	if (!c->reset) {
		usb_err("controller reset fail\n");
		goto exit;
	}

	c->unreset = of_get_chip_usb_reg_cfg(node, "unreset");
	if (!c->unreset) {
		usb_err("controller unreset fail\n");
		goto unreset_err;
	}

	c->sel_self = of_get_chip_usb_reg_cfg(node, "sel-self");
	if (!c->sel_self)
		usb_err("dont support multi controller\n");

	return 0;

unreset_err:
	of_remove_chip_usb_reg_cfg(c->reset);
exit:
	return -ENOENT;
}

static void chip_usb_put_controller_ops(struct usb_controller *c)
{
	of_remove_chip_usb_reg_cfg(c->reset);

	of_remove_chip_usb_reg_cfg(c->unreset);

	of_remove_chip_usb_reg_cfg(c->sel_self);
}

static int chip_usb_get_controller_property(
	struct chip_dwc3_device *chip_usb,
	struct device *dev)
{
	int ret = 0;
	u32 i;
	errno_t m_ret;
	struct property *prop = NULL;
	const char *name = NULL;
	struct device_node *child = NULL;
	struct of_device_id _match[2];
	struct of_device_id *match = &_match[0];

	m_ret = memset_s(_match, sizeof(_match), 0, sizeof(_match));
	if (m_ret != EOK)
		return -ENOMEM;

	i = 0;
	of_property_for_each_string(dev->of_node, "controller-list", prop, name) {
		chip_usb->usbc_sets[i] = devm_kmalloc(dev,
			sizeof(*chip_usb->usbc_sets[i]), GFP_KERNEL);
		if (!chip_usb->usbc_sets[i]) {
			ret = -ENOMEM;
			goto malloc_err;
		}

		m_ret = strncpy_s(match->compatible, sizeof(match->compatible),
			name, strlen(name));
		if (m_ret != EOK)
			goto get_clk_err;

		for_each_child_of_node(dev->of_node, child) {
			if (!of_match_node(match, child))
				continue;

			usb_info("get %s prop\n", match->compatible);
			chip_usb->usbc_sets[i]->node = child;
			chip_usb->usbc_sets[i]->stub_clk =
				of_property_read_bool(child, "stub-clk");

			ret = chip_usb_get_controller_clks(child,
				chip_usb->usbc_sets[i]);
			if (ret) {
				usb_err("get controller clks failed %d\n", ret);
				goto get_clk_err;
			}

			ret = chip_usb_get_controller_ops(child,
				chip_usb->usbc_sets[i]);
			if (ret) {
				usb_err("get controller ops failed %d\n", ret);
				goto get_ops_err;
			}
		}
		i++;
	}
	return ret;

get_ops_err:
	chip_usb_put_controller_clks(chip_usb->usbc_sets[i]);
get_clk_err:
	devm_kfree(dev, chip_usb->usbc_sets[i]);
	chip_usb->usbc_sets[i] = NULL;
malloc_err:
	if (i == 0)
		return ret;
	chip_usb->usbc_nums = i;
	return 0;
}

static int chip_usb_get_controller_res(struct chip_dwc3_device *chip_usb,
			struct device *dev)
{
	int ret;

	if (of_property_read_u32(dev->of_node, "usbc_idx", &chip_usb->usbc_idx))
		chip_usb->usbc_idx = 0;

	chip_usb->usbc_working_idx = chip_usb->usbc_idx;
	chip_usb->usbc_nums = of_property_count_strings(dev->of_node,
		"controller-list");
	if (!chip_usb->usbc_nums) {
		usb_err("one usb controller at least\n");
		return -ENODEV;
	}

	if (chip_usb->usbc_nums >= USBC_NUM_MAX) {
		usb_err("usbc index err\n");
		chip_usb->usbc_nums = 0;
		return -EINVAL;
	}

	chip_usb->usbc_sets = devm_kmalloc(dev, sizeof(*chip_usb->usbc_sets) * chip_usb->usbc_nums,
		GFP_KERNEL);

	if (!chip_usb->usbc_sets) {
		usb_err("fail to malloc usbc_sets\n");
		return -ENOMEM;
	}

	ret =  chip_usb_get_controller_property(chip_usb, dev);
	if (ret) {
		usb_err("fail to get property\n");
		devm_kfree(dev, chip_usb->usbc_sets);
		chip_usb->usbc_sets = NULL;
		chip_usb->usbc_nums = 0;
	}
	return ret;
}

static int chip_usb_get_phy_res(struct chip_dwc3_device *chip_usb,
			struct device *dev)
{
	struct device_node *np = NULL;

	chip_usb->usb2_phy = devm_phy_get(dev, "usb2-phy");
	if (IS_ERR(chip_usb->usb2_phy)) {
		usb_err("no usb2 phy configured\n");
		return -EPROBE_DEFER;
	}

	np = of_get_child_by_name(dev->of_node, "combophy_func");
	if (np && of_device_is_available(np)) {
		chip_usb->usb3_phy = devm_phy_get(dev, "usb3-phy");
		if (IS_ERR(chip_usb->usb3_phy)) {
			usb_err("no usb3 phy configured\n");
			return -EPROBE_DEFER;
		}
	} else {
		chip_usb->usb3_phy = NULL;
	}
	if (np)
		of_node_put(np);

	return 0;
}

int chip_usb_get_hw_res(struct chip_dwc3_device *chip_usb, struct device *dev)
{
	int ret;

	if (!chip_usb || !dev)
		return -EINVAL;

	ret = chip_usb_get_phy_res(chip_usb, dev);
	if (ret)
		return ret;

	return  chip_usb_get_controller_res(chip_usb, dev);
}

static void chip_usb_put_controller_res(struct chip_dwc3_device *chip_usb)
{
	u32 i;

	for (i = 0; i < chip_usb->usbc_nums; i++) {
		if (!chip_usb->usbc_sets[i])
			continue;

		chip_usb_put_controller_ops(chip_usb->usbc_sets[i]);

		chip_usb_put_controller_clks(chip_usb->usbc_sets[i]);

		devm_kfree(&chip_usb->pdev->dev, chip_usb->usbc_sets[i]);
		chip_usb->usbc_sets[i] = NULL;
	}
	devm_kfree(&chip_usb->pdev->dev, chip_usb->usbc_sets);
	chip_usb->usbc_sets = NULL;
}

void chip_usb_put_hw_res(struct chip_dwc3_device *chip_usb)
{
	chip_usb_put_controller_res(chip_usb);
}

int chip_usb_register_hw_debugfs(struct chip_dwc3_device *chip_usb)
{
	if (!chip_usb)
		return -EINVAL;

	if (!chip_usb->debug_root)
		return 0;

	if (chip_usb->usb3_phy)
		return combophy_register_debugfs(chip_usb->usb3_phy,
			chip_usb->debug_root);

	return 0;
}

enum hisi_charger_type chip_usb_detect_charger_type(
		struct chip_dwc3_device *chip_dwc3)
{
	struct chip_usb2_phy *chip_usb2_phy = NULL;

	if (!chip_dwc3)
		return CHARGER_TYPE_NONE;

	if (chip_dwc3->fpga_flag) {
		usb_dbg("this is fpga platform, charger is SDP\n");
		return CHARGER_TYPE_SDP;
	}

	if (chip_dwc3->fake_charger_type != CHARGER_TYPE_NONE) {
		usb_dbg("fake type: %d\n", chip_dwc3->fake_charger_type);
		return chip_dwc3->fake_charger_type;
	}

	if (chip_dwc3->use_new_frame) {
		if (!chip_dwc3->usb2_phy)
			return CHARGER_TYPE_NONE;

		chip_usb2_phy = (struct chip_usb2_phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!chip_usb2_phy || !chip_usb2_phy->detect_charger_type)
			return CHARGER_TYPE_NONE;

		return chip_usb2_phy->detect_charger_type(chip_usb2_phy);
	} else {
		return detect_charger_type(chip_dwc3);
	}
}

void chip_usb_dpdm_pulldown(struct chip_dwc3_device *chip_dwc3)
{
	struct chip_usb2_phy *chip_usb2_phy = NULL;

	usb_dbg("+\n");
	if (!chip_dwc3)
		return;

	if (chip_dwc3->use_new_frame) {
		if (!chip_dwc3->usb2_phy)
			return;

		chip_usb2_phy = (struct chip_usb2_phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!chip_usb2_phy || !chip_usb2_phy->set_dpdm_pulldown)
			return;

		chip_usb2_phy->set_dpdm_pulldown(chip_usb2_phy, true);
	} else {
		chip_bc_dplus_pulldown(chip_dwc3);
	}
	usb_dbg("-\n");
}

void chip_usb_dpdm_pullup(struct chip_dwc3_device *chip_dwc3)
{
	struct chip_usb2_phy *chip_usb2_phy = NULL;

	usb_dbg("+\n");
	if (!chip_dwc3)
		return;

	if (chip_dwc3->use_new_frame) {
		if (!chip_dwc3->usb2_phy)
			return;

		chip_usb2_phy = (struct chip_usb2_phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!chip_usb2_phy || !chip_usb2_phy->set_dpdm_pulldown)
			return;

		chip_usb2_phy->set_dpdm_pulldown(chip_usb2_phy, false);
	} else {
		chip_bc_dplus_pullup(chip_dwc3);
	}
	usb_dbg("-\n");
}

void chip_usb_disable_vdp_src(struct chip_dwc3_device *chip_dwc3)
{
	usb_dbg("+\n");

	if (!chip_dwc3)
		return;

	if (chip_dwc3->use_new_frame) {
		struct chip_usb2_phy *chip_usb2_phy = NULL;

		if (!chip_dwc3->usb2_phy)
			return;

		chip_usb2_phy = (struct chip_usb2_phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!chip_usb2_phy || !chip_usb2_phy->set_vdp_src)
			return;

		chip_usb2_phy->set_vdp_src(chip_usb2_phy, false);

		usb_dbg("-\n");
	} else {
		unsigned long flags;

		spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
		chip_bc_disable_vdp_src(chip_dwc3);
		spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);
	}
}

void chip_usb_enable_vdp_src(struct chip_dwc3_device *chip_dwc3)
{
	usb_dbg("+\n");

	if (!chip_dwc3)
		return;

	if (chip_dwc3->use_new_frame) {
		struct chip_usb2_phy *chip_usb2_phy = NULL;

		if (!chip_dwc3->usb2_phy)
			return;

		chip_usb2_phy = (struct chip_usb2_phy *)
			phy_get_drvdata(chip_dwc3->usb2_phy);
		if (!chip_usb2_phy || !chip_usb2_phy->set_vdp_src)
			return;

		chip_usb2_phy->set_vdp_src(chip_usb2_phy, true);

		usb_dbg("-\n");
	} else {
		unsigned long flags;

		spin_lock_irqsave(&chip_dwc3->bc_again_lock, flags);
		chip_bc_enable_vdp_src(chip_dwc3);
		spin_unlock_irqrestore(&chip_dwc3->bc_again_lock, flags);
	}
}

static int usb_controller_init(struct usb_controller *usbc)
{
	int ret;

	if (usbc->sel_self) {
		ret = chip_usb_reg_write(usbc->sel_self);
		if (ret) {
			usb_err("sel self failed %d\n", ret);
			return ret;
		}
	}

	if (!usbc->stub_clk) {
		ret = chip_usb_init_clks(usbc->clks, usbc->num_clks);
		if (ret) {
			usb_err("clks init failed %d\n", ret);
			return ret;
		}
	}

	if (usbc->unreset) {
		ret = chip_usb_reg_write(usbc->unreset);
		if (ret) {
			usb_err("unreset failed %d\n", ret);
			return ret;
		}
	}
	return 0;
}

static int usb_controller_exit(struct usb_controller *usbc)
{
	int ret;

	if (usbc->reset) {
		ret = chip_usb_reg_write(usbc->reset);
		if (ret) {
			usb_err("reset fail %d\n", ret);
			return ret;
		}
	}

	if (!usbc->stub_clk)
		chip_usb_shutdown_clks(usbc->clks, usbc->num_clks);

	return 0;
}

int chip_usb_controller_init(struct chip_dwc3_device *chip_dwc3)
{
	int ret;

	if (!chip_dwc3)
		return -EINVAL;

	if (!chip_dwc3->usbc_sets) {
		usb_err("usbc sets null\n");
		return -EINVAL;
	}

	if (chip_dwc3->usbc_nums <= chip_dwc3->usbc_idx) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	chip_dwc3->usbc_working_idx = chip_dwc3->usbc_idx;
	ret = usb_controller_init(chip_dwc3->usbc_sets[chip_dwc3->usbc_idx]);
	if (ret) {
		usb_err("controller init fail %d\n", ret);
		return ret;
	}
	/* require to delay 10ms */
	mdelay(10);

	return 0;
}

int chip_usb_controller_exit(struct chip_dwc3_device *chip_dwc3)
{
	if (!chip_dwc3)
		return -EINVAL;

	if (!chip_dwc3->usbc_sets) {
		usb_err("usbc sets null\n");
		return -EINVAL;
	}

	if (chip_dwc3->usbc_nums <= chip_dwc3->usbc_working_idx) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	return usb_controller_exit(chip_dwc3->usbc_sets[chip_dwc3->usbc_working_idx]);
}

/* multi controller */
int chip_usb_controller_probe(struct chip_dwc3_device *chip_dwc3)
{
	struct platform_device *pdev = NULL;
	struct device *parent = NULL;
	struct device_node *child = NULL;

	if (!chip_dwc3)
		return -ENAVAIL;

	if (!chip_dwc3->usbc_sets) {
		usb_err("usbc sets null\n");
		return -EINVAL;
	}

	if (chip_dwc3->usbc_nums <= chip_dwc3->usbc_idx) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	parent = &chip_dwc3->pdev->dev;
	child = chip_dwc3->usbc_sets[chip_dwc3->usbc_idx]->node;
	if (!child) {
		usb_err("usbc node err\n");
		return -EINVAL;
	}

	pdev = of_platform_device_create(child, NULL, parent);
	if (!pdev) {
		usb_err("create usbc dev failed!\n");
		return -ENODEV;
	}
	chip_dwc3->usbc_sets[chip_dwc3->usbc_idx]->pdev = pdev;
	return 0;
}

int chip_usb_controller_destroy(struct chip_dwc3_device *chip_dwc3)
{
	int ret;
	struct device *parent = NULL;
	struct platform_device *child = NULL;

	if (!chip_dwc3)
		return -ENAVAIL;

	if (!chip_dwc3->usbc_sets) {
		usb_err("usbc sets null\n");
		return -EINVAL;
	}

	if (chip_dwc3->usbc_nums <= chip_dwc3->usbc_working_idx) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	parent = &chip_dwc3->pdev->dev;
	child = chip_dwc3->usbc_sets[chip_dwc3->usbc_working_idx]->pdev;
	if (!child) {
		usb_err("usbc index err\n");
		return -EINVAL;
	}

	ret = of_platform_device_destroy(&child->dev, NULL);
	if (ret)
		return ret;

	chip_dwc3->usbc_sets[chip_dwc3->usbc_working_idx]->pdev = NULL;
	return 0;
}
