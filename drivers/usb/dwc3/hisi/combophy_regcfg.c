// SPDX-License-Identifier: GPL-2.0
/*
 * Register config for USB
 *
 * Authors: Yu Chen <chenyu56@huawei.com>
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/of.h>
#include "hisi_usb_reg_cfg.h"
#include "dwc3-hisi.h"

struct combophy_regcfg {
	struct hisi_usb_reg_cfg *misc_ctrl_reset_cfg;
	struct hisi_usb_reg_cfg *misc_ctrl_unreset_cfg;
	struct hisi_usb_reg_cfg *misc_ctrl_is_unreset_cfg;
	struct hisi_usb_reg_cfg *misc_ctrl_is_clk_en_cfg;
	struct hisi_usb_reg_cfg *phy_reset_cfg;
	struct hisi_usb_reg_cfg *phy_unreset_cfg;
	struct hisi_usb_reg_cfg *phy_isodis_cfg;
	struct hisi_usb_reg_cfg *exit_testpowerdown_cfg;
	struct hisi_usb_reg_cfg *power_stable_cfg;
	struct hisi_usb_reg_cfg *enter_testpowerdown_cfg;
	struct hisi_usb_reg_cfg *is_controller_ref_clk_en_cfg;
	struct hisi_usb_reg_cfg *is_controller_bus_clk_en_cfg;
};

struct combophy_regcfg *_combophy_regcfg;

void combophy_regcfg_reset_misc(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->misc_ctrl_reset_cfg)
		return;

	ret = hisi_usb_reg_write(_combophy_regcfg->misc_ctrl_reset_cfg);
	if (ret)
		usb_err("config failed\n");
}

void combophy_regcfg_unreset_misc(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->misc_ctrl_unreset_cfg)
		return;

	ret = hisi_usb_reg_write(_combophy_regcfg->misc_ctrl_unreset_cfg);
	if (ret)
		usb_err("config failed\n");
}

bool combophy_regcfg_is_misc_ctrl_unreset(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->misc_ctrl_is_unreset_cfg)
		return false;

	ret = hisi_usb_reg_test_cfg(_combophy_regcfg->misc_ctrl_is_unreset_cfg);
	if (ret < 0)
		usb_err("config failed\n");

	return ret < 0 ? false : ret;
}

bool combophy_regcfg_is_misc_ctrl_clk_en(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->misc_ctrl_is_clk_en_cfg)
		return false;

	ret = hisi_usb_reg_test_cfg(_combophy_regcfg->misc_ctrl_is_clk_en_cfg);
	if (ret < 0)
		usb_err("config failed\n");

	return ret < 0 ? false : ret;
}

void combophy_regcfg_phyreset(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->phy_reset_cfg)
		return;

	ret = hisi_usb_reg_write(_combophy_regcfg->phy_reset_cfg);
	if (ret)
		usb_err("config failed\n");
}

void combophy_regcfg_phyunreset(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->phy_unreset_cfg)
		return;

	ret = hisi_usb_reg_write(_combophy_regcfg->phy_unreset_cfg);
	if (ret)
		usb_err("config failed\n");
}

void combophy_regcfg_isodis(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->phy_isodis_cfg)
		return;

	ret = hisi_usb_reg_write(_combophy_regcfg->phy_isodis_cfg);
	if (ret)
		usb_err("config failed\n");
}

void combophy_regcfg_exit_testpowerdown(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->exit_testpowerdown_cfg)
		return;

	ret = hisi_usb_reg_write(_combophy_regcfg->exit_testpowerdown_cfg);
	if (ret)
		usb_err("config failed\n");
}

void combophy_regcfg_power_stable(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->power_stable_cfg)
		return;

	ret = hisi_usb_reg_write(_combophy_regcfg->power_stable_cfg);
	if (ret)
		usb_err("config failed\n");
}

void combophy_regcfg_enter_testpowerdown(void)
{
	int ret;

	if (!_combophy_regcfg || !_combophy_regcfg->enter_testpowerdown_cfg)
		return;

	ret = hisi_usb_reg_write(_combophy_regcfg->enter_testpowerdown_cfg);
	if (ret)
		usb_err("config failed\n");
}

bool combophy_regcfg_is_controller_ref_clk_en(void)
{
	int ret;

	if (!_combophy_regcfg ||
			!_combophy_regcfg->is_controller_ref_clk_en_cfg)
		return false;

	ret = hisi_usb_reg_test_cfg(
			_combophy_regcfg->is_controller_ref_clk_en_cfg);
	if (ret < 0)
		usb_err("config failed\n");

	return ret < 0 ? false : ret;
}

bool combophy_regcfg_is_controller_bus_clk_en(void)
{
	int ret;

	if (!_combophy_regcfg ||
			!_combophy_regcfg->is_controller_bus_clk_en_cfg)
		return false;

	ret = hisi_usb_reg_test_cfg(
			_combophy_regcfg->is_controller_bus_clk_en_cfg);
	if (ret < 0)
		usb_err("config failed\n");

	return ret < 0 ? false : ret;
}

#define combophy_get_reg_cfg(np, name) (_combophy_regcfg->name = \
		of_get_hisi_usb_reg_cfg_by_name(np, #name))

static int combophy_regcfg_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	_combophy_regcfg = devm_kzalloc(dev, sizeof(*_combophy_regcfg),
			GFP_KERNEL);
	if (!_combophy_regcfg)
		return -ENOMEM;

	combophy_get_reg_cfg(np, misc_ctrl_reset_cfg);

	combophy_get_reg_cfg(np, misc_ctrl_unreset_cfg);

	combophy_get_reg_cfg(np, misc_ctrl_is_unreset_cfg);

	combophy_get_reg_cfg(np, misc_ctrl_is_clk_en_cfg);

	combophy_get_reg_cfg(np, phy_reset_cfg);

	combophy_get_reg_cfg(np, phy_unreset_cfg);

	combophy_get_reg_cfg(np, phy_isodis_cfg);

	combophy_get_reg_cfg(np, exit_testpowerdown_cfg);

	combophy_get_reg_cfg(np, power_stable_cfg);

	combophy_get_reg_cfg(np, enter_testpowerdown_cfg);

	combophy_get_reg_cfg(np, is_controller_ref_clk_en_cfg);

	combophy_get_reg_cfg(np, is_controller_bus_clk_en_cfg);

	return 0;
}

static int combophy_regcfg_remove(struct platform_device *pdev)
{
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->misc_ctrl_reset_cfg);
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->misc_ctrl_unreset_cfg);
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->misc_ctrl_is_unreset_cfg);
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->misc_ctrl_is_clk_en_cfg);
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->phy_reset_cfg);
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->phy_unreset_cfg);
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->phy_isodis_cfg);
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->exit_testpowerdown_cfg);
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->power_stable_cfg);
	of_remove_hisi_usb_reg_cfg(_combophy_regcfg->enter_testpowerdown_cfg);
	of_remove_hisi_usb_reg_cfg(
			_combophy_regcfg->is_controller_ref_clk_en_cfg);
	of_remove_hisi_usb_reg_cfg(
			_combophy_regcfg->is_controller_bus_clk_en_cfg);
	_combophy_regcfg = NULL;

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id combophy_regcfg_match[] = {
	{ .compatible = "hisilicon,combophy_regcfg" },
	{},
};
MODULE_DEVICE_TABLE(of, combophy_regcfg_match);
#else
#define dwc3_combophy_match NULL
#endif

static struct platform_driver combophy_regcfg_driver = {
	.probe		= combophy_regcfg_probe,
	.remove		= combophy_regcfg_remove,
	.driver		= {
		.name = "combophy_regcfg",
		.of_match_table = of_match_ptr(combophy_regcfg_match),
	},
};
module_platform_driver(combophy_regcfg_driver);
