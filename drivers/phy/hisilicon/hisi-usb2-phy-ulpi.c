/*
 * hisi-usb2-phy-ulpi.c
 * Hisilicon Hi6502 efuse driver
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Create:2019-09-24
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/atomic.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>
#include <linux/hisi/hisi_usb_phy_chip.h>
#include <linux/hisi/hisi_usb_phy_driver.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/usb/hisi_usb_phy.h>
#include <linux/hisi/usb/hisi_usb_misc_ctrl.h>
#include <linux/hisi/usb/hisi_usb_helper.h>
#include <linux/hisi/usb/hisi_usb_reg_cfg.h>

#define info(format, arg...) pr_info("[USB2_6502][I][%s]"format, __func__, ##arg)
#define err(format, arg...) pr_err("[USB2_6502][E][%s]"format, __func__, ##arg)

#define chip_usb2_phy_to_phy_priv(pointer) \
	container_of(pointer, struct phy_priv, usb_phy)

#define DEFAULT_T28_EYE_PARAM 0x0246709C
#define DEFAULT_T28_HOST_EYE_PARAM 0x0246709E
#define DEFAULT_S28_EYE_PARAM 0xB760
#define DEFAULT_S28_HOST_EYE_PARAM 0xB760
#define DEFAULT_ULPI_IOC_DRIVING_FORCE	0x00FB004A

struct phy_priv {
	struct device *dev;
	struct chip_usb2_phy usb_phy;

	struct clk *gt_clk_ulpi_ref;
	struct chip_usb_reg_cfg **vbus_valid;
	int vbus_valid_num;
	struct chip_usb_reg_cfg **vbus_invalid;
	int vbus_invalid_num;

	enum phy_mode mode;
	bool disable_bc;
	bool ulpi_ioc_stub;
	uint32_t phy_param_t28;
	uint32_t phy_host_param_t28;
	uint32_t phy_param_s28;
	uint32_t phy_host_param_s28;
	uint32_t ulpi_data_driving_force_s28;
	uint32_t ulpi_stp_driving_force_s28;
	uint32_t ulpi_data_driving_force_t28;
	uint32_t ulpi_stp_driving_force_t28;
	struct work_struct vdp_src_work;
	atomic_t desired_vdp_src;
};

static void ioc_hsdt1_config_ulpi(struct phy_priv *priv)
{
	/* emu & fpga may not cfg hsdt1 ioc func */
	if (priv->ulpi_ioc_stub)
		return;

	upc_set_usb_ioc(
		priv->ulpi_data_driving_force_t28, priv->ulpi_stp_driving_force_t28,
		priv->ulpi_data_driving_force_s28, priv->ulpi_stp_driving_force_s28);
}

static int _usb2_phy_init(struct phy_priv *priv)
{
	int ret;

	info("+\n");

	/* usb phy chip unreset */
	upc_open();

	ioc_hsdt1_config_ulpi(priv);
	ret = misc_ctrl_init();
	if (ret)
		return ret;

	if (priv->gt_clk_ulpi_ref) {
		/* enable ULPI ref clock */
		ret = clk_prepare_enable(priv->gt_clk_ulpi_ref);
		if (ret) {
			err("clk_prepare_enable ulpi ref clk failed\n");
			goto out_exit_misc_ctrl;
		}
	}
	/* deassert ULPI phy */
	upc_phy_reset();
	udelay(100);
	upc_phy_unreset();

	/* open ulpi vbusvalid and bvalid */
	ret = chip_usb_reg_write_array(priv->vbus_valid, priv->vbus_valid_num);
	if (ret) {
		err("set vbus valid failed %d\n", ret);
		goto out_exit_misc_ctrl;
	}

	udelay(100);

	info("set phy param in %s mode\n",
			(priv->mode == PHY_MODE_USB_HOST) ? "host" : "device");
	/* set eye param */
	if (priv->mode == PHY_MODE_USB_HOST)
		upc_set_eye_diagram_param(priv->phy_host_param_t28,
				priv->phy_host_param_s28);
	else
		upc_set_eye_diagram_param(priv->phy_param_t28,
				priv->phy_param_s28);

	info("-\n");
	return 0;

out_exit_misc_ctrl:
	misc_ctrl_exit();
	err("failed to init phy ret: %d\n", ret);
	return ret;
}

static int usb2_phy_init(struct phy *phy)
{
	struct chip_usb2_phy *usb2_phy =
		(struct chip_usb2_phy *)phy_get_drvdata(phy);
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -ENODEV;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	return _usb2_phy_init(priv);
}

static int _usb2_phy_exit(struct phy_priv *priv)
{
	int ret;

	info("+\n");

	/* reset ULPI phy */
	upc_phy_reset();

	/* close ulpi vbusvalid and bvalid */
	ret = chip_usb_reg_write_array(priv->vbus_invalid,
			priv->vbus_invalid_num);
	if (ret)
		err("set vbus invalid failed %d\n", ret);

	if (priv->gt_clk_ulpi_ref)
		clk_disable_unprepare(priv->gt_clk_ulpi_ref);

	misc_ctrl_exit();
	/* usb phy chip reset */
	upc_close();
	info("-\n");
	return 0;
}

static int usb2_phy_exit(struct phy *phy)
{
	struct chip_usb2_phy *usb2_phy =
		(struct chip_usb2_phy *)phy_get_drvdata(phy);
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -ENODEV;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	return _usb2_phy_exit(priv);
}

static int usb2_phy_poweron(struct phy *phy)
{
	/* avoid plus on S28, called after regulator enable */
	info("+\n");
	upc_remove_pw_burr_enable(UPC_BURR_PW_ON, UPC_BURR_NORMAL_CFG);
	info("-\n");

	return 0;
}

static int usb2_phy_poweroff(struct phy *phy)
{
	/* avoid plus on S28, called before regulator disable */
	info("+\n");
	upc_remove_pw_burr_enable(UPC_BURR_PW_OFF, UPC_BURR_NORMAL_CFG);
	info("-\n");

	return 0;
}

static int usb2_phy_set_mode(struct phy *phy, enum phy_mode mode)
{
	struct chip_usb2_phy *usb2_phy =
		(struct chip_usb2_phy *)phy_get_drvdata(phy);
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -ENODEV;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	priv->mode = mode;

	return 0;
}

static struct phy_ops usb2_phy_ops = {
	.init		= usb2_phy_init,
	.exit		= usb2_phy_exit,
	.power_on	= usb2_phy_poweron,
	.power_off	= usb2_phy_poweroff,
	.set_mode	= usb2_phy_set_mode,
	.owner		= THIS_MODULE,
};

static enum hisi_charger_type detect_charger_type(
		struct chip_usb2_phy *usb2_phy)
{
	struct phy_priv *priv = NULL;
	struct phy *phy = NULL;
	enum hisi_charger_type charger_type;

	if (!usb2_phy)
		return CHARGER_TYPE_NONE;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	if (priv->disable_bc)
		return CHARGER_TYPE_SDP;

	phy = priv->usb_phy.phy;

	mutex_lock(&phy->mutex);
	if (phy->init_count == 0) {
		mutex_unlock(&phy->mutex);
		err("phy has not been initialized\n");
		return CHARGER_TYPE_NONE;
	}

	charger_type = upc_detect_charger_type();

	mutex_unlock(&phy->mutex);

	return charger_type;
}

static int set_dpdm_pulldown(struct chip_usb2_phy *usb2_phy, bool pulldown)
{
	struct phy_priv *priv = NULL;
	struct phy *phy = NULL;

	if (!usb2_phy)
		return -EINVAL;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	if (priv->disable_bc)
		return 0;

	phy = priv->usb_phy.phy;

	mutex_lock(&phy->mutex);
	if (phy->init_count == 0) {
		mutex_unlock(&phy->mutex);
		err("phy has not been initialized\n");
		return -ENODEV;
	}

	if (pulldown)
		upc_hisi_bc_dplus_pulldown();
	else
		upc_hisi_bc_dplus_pullup();

	mutex_unlock(&phy->mutex);

	return 0;
}

static int _set_vdp_src(struct phy_priv *priv, bool enable)
{
	struct phy *phy = priv->usb_phy.phy;

	if (priv->disable_bc)
		return 0;

	mutex_lock(&phy->mutex);
	if (phy->init_count == 0) {
		mutex_unlock(&phy->mutex);
		err("phy has not been initialized\n");
		return -ENODEV;
	}

	if (enable)
		upc_hisi_bc_enable_vdp_src();
	else
		upc_hisi_bc_disable_vdp_src();

	mutex_unlock(&phy->mutex);

	return 0;
}

static int set_vdp_src(struct chip_usb2_phy *usb2_phy, bool enable)
{
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -EINVAL;

	priv = chip_usb2_phy_to_phy_priv(usb2_phy);

	if (in_atomic() || irqs_disabled()) {
		if (enable)
			atomic_set(&priv->desired_vdp_src, 1);
		else
			atomic_set(&priv->desired_vdp_src, 0);

		if (!queue_work(system_power_efficient_wq,
					&priv->vdp_src_work)) {
			info("vdp src work already in queue\n");
			return -EAGAIN;
		}
		return 0;
	} else {
		return _set_vdp_src(priv, enable);
	}
}

static void vdp_src_work_fn(struct work_struct *work)
{
	int ret;
	struct phy_priv *priv = container_of(work,
				    struct phy_priv, vdp_src_work);

	ret = _set_vdp_src(priv, atomic_read(&priv->desired_vdp_src) == 1);
	if (ret)
		err("set vdp src failed %d\n", ret);
}

static int get_vbus_ops(struct phy_priv *priv)
{
	int ret;
	struct device *dev = priv->dev;

	ret = get_chip_usb_reg_cfg_array(dev, "vbus-valid",
				&priv->vbus_valid, &priv->vbus_valid_num);
	if (ret) {
		err("get vbus valid failed %d\n", ret);
		return ret;
	}

	ret = get_chip_usb_reg_cfg_array(dev, "vbus-invalid",
				&priv->vbus_invalid, &priv->vbus_invalid_num);
	if (ret) {
		free_chip_usb_reg_cfg_array(priv->vbus_valid,
					priv->vbus_valid_num);
		err("get vbus invalid fail %d\n", ret);
		return ret;
	}
	return ret;
}

static void put_vbus_ops(struct phy_priv *priv)
{
	free_chip_usb_reg_cfg_array(priv->vbus_invalid,
			priv->vbus_invalid_num);

	free_chip_usb_reg_cfg_array(priv->vbus_valid,
			priv->vbus_valid_num);
}

static int get_clk_resource(struct phy_priv *priv)
{
	struct device *dev = priv->dev;

	if (of_property_read_bool(dev->of_node, "clocks")) {
		/* get ulpi ref clk handler */
		priv->gt_clk_ulpi_ref = devm_clk_get(dev, "clk_ulpi_ref");
		if (IS_ERR_OR_NULL(priv->gt_clk_ulpi_ref)) {
			err("get gt_clk_ulpi_ref failed\n");
			return -EINVAL;
		}
	}
	return 0;
}

static void phy_get_phy_param_form_dt(struct phy_priv *priv)
{
	int ret;

	ret = of_property_read_u32(priv->dev->of_node, "phy-param-t28",
		&priv->phy_param_t28);
	if (ret) {
		err("read phy-param-t28 failed, ret %d\n", ret);
		priv->phy_param_t28 = DEFAULT_T28_EYE_PARAM;
	}

	ret = of_property_read_u32(priv->dev->of_node, "phy-host-param-t28",
		&priv->phy_host_param_t28);
	if (ret) {
		err("read phy-host-param-t28 failed, ret %d\n", ret);
		priv->phy_host_param_t28 = DEFAULT_T28_HOST_EYE_PARAM;
	}

	ret = of_property_read_u32(priv->dev->of_node, "phy-param-s28",
		&priv->phy_param_s28);
	if (ret) {
		err("read phy-param-s28 failed, ret %d\n", ret);
		priv->phy_param_s28 = DEFAULT_S28_EYE_PARAM;
	}

	ret = of_property_read_u32(priv->dev->of_node, "phy-host-param-s28",
		&priv->phy_host_param_s28);
	if (ret) {
		err("read phy-host-param-s28 failed, ret %d\n", ret);
		priv->phy_host_param_s28 = DEFAULT_S28_HOST_EYE_PARAM;
	}
}

static void get_io_config_form_dt(struct phy_priv *priv)
{
	int ret;

	ret = of_property_read_u32(priv->dev->of_node, "ulpi-ioc-data-driving-t28",
		&priv->ulpi_data_driving_force_t28);
	if (ret) {
		err("read ulpi-ioc-data-driving-t28 failed, ret %d\n", ret);
		priv->ulpi_data_driving_force_t28 = DEFAULT_ULPI_IOC_DRIVING_FORCE;
	}

	ret = of_property_read_u32(priv->dev->of_node, "ulpi-ioc-stp-driving-t28",
		&priv->ulpi_stp_driving_force_t28);
	if (ret) {
		err("read ulpi-ioc-stp-driving-t28 failed, ret %d\n", ret);
		priv->ulpi_stp_driving_force_t28 = DEFAULT_ULPI_IOC_DRIVING_FORCE;
	}

	ret = of_property_read_u32(priv->dev->of_node, "ulpi-ioc-data-driving-s28",
		&priv->ulpi_data_driving_force_s28);
	if (ret) {
		err("read ulpi-ioc-data-driving-s28 failed, ret %d\n", ret);
		priv->ulpi_data_driving_force_s28 = DEFAULT_ULPI_IOC_DRIVING_FORCE;
	}

	ret = of_property_read_u32(priv->dev->of_node, "ulpi-ioc-stp-driving-s28",
		&priv->ulpi_stp_driving_force_s28);
	if (ret) {
		err("read ulpi-ioc-stp-driving-s28 failed, ret %d\n", ret);
		priv->ulpi_stp_driving_force_s28 = DEFAULT_ULPI_IOC_DRIVING_FORCE;
	}
}

static int get_dts_resource(struct phy_priv *priv)
{
	if (get_clk_resource(priv)) {
		err("get clk err\n");
		return -EINVAL;
	}

	if (get_vbus_ops(priv)) {
		err("get vbus ops err\n");
		return -EINVAL;
	}

	priv->disable_bc = of_property_read_bool(priv->dev->of_node,
			"disable-bc");
	info("disable_bc %d!\n", priv->disable_bc);

	priv->ulpi_ioc_stub = of_property_read_bool(priv->dev->of_node,
			"ulpi-ioc-cfg-stub");
	info("ulpi-ioc-cfg-stub %d!\n", priv->ulpi_ioc_stub);

	/* get usb phy eye param config */
	phy_get_phy_param_form_dt(priv);

	/* get usb phy io driving force config */
	get_io_config_form_dt(priv);

	return 0;
}

static int usb2_phy_6502_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct phy_provider *phy_provider = NULL;
	struct phy *phy = NULL;
	struct phy_priv *priv = NULL;
	int ret;

	err("+\n");
	if (!misc_ctrl_is_ready()) {
		err("misc ctrl is not ready\n");
		return -EPROBE_DEFER;
	}

	if (!upc_phy_driver_is_ready()) {
		err("upc phy driver is not ready\n");
		return -EPROBE_DEFER;
	}

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_no_callbacks(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		err("pm_runtime_get_sync failed\n");
		goto err_pm_put;
	}

	pm_runtime_forbid(dev);

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto err_pm_allow;
	}

	priv->dev = dev;

	ret = get_dts_resource(priv);
	if (ret) {
		err("get_dts_resource failed\n");
		goto err_pm_allow;
	}

	phy = devm_phy_create(dev, NULL, &usb2_phy_ops);
	if (IS_ERR(phy)) {
		ret = PTR_ERR(phy);
		goto err_get_dts;
	}

	priv->usb_phy.phy = phy;
	priv->usb_phy.detect_charger_type = detect_charger_type;
	priv->usb_phy.set_dpdm_pulldown = set_dpdm_pulldown;
	priv->usb_phy.set_vdp_src = set_vdp_src;
	priv->mode = PHY_MODE_INVALID;
	phy_set_drvdata(phy, &priv->usb_phy);
	INIT_WORK(&priv->vdp_src_work, vdp_src_work_fn);
	atomic_set(&priv->desired_vdp_src, 0);

	phy_provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	if (IS_ERR(phy_provider)) {
		ret = PTR_ERR(phy_provider);
		goto err_get_dts;
	}

	platform_set_drvdata(pdev, priv);

	err("-\n");
	return 0;

err_get_dts:
	put_vbus_ops(priv);
err_pm_allow:
	pm_runtime_allow(dev);
err_pm_put:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);

	info("ret %d\n", ret);
	return ret;
}

static int usb2_phy_6502_remove(struct platform_device *pdev)
{
	struct phy_priv *priv = (struct phy_priv *)platform_get_drvdata(pdev);

	if (cancel_work_sync(&priv->vdp_src_work))
		info("vdp src work canceled\n");

	put_vbus_ops(priv);
	pm_runtime_allow(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static const struct of_device_id usb2_phy_6502_of_match[] = {
	{ .compatible = "hisilicon,usb2-phy-6502", },
	{ }
};
MODULE_DEVICE_TABLE(of, usb2_phy_6502_of_match);

static struct platform_driver usb2_phy_6502_driver = {
	.probe	= usb2_phy_6502_probe,
	.remove = usb2_phy_6502_remove,
	.driver = {
		.name	= "usb2-phy-6502",
		.of_match_table	= usb2_phy_6502_of_match,
	}
};
module_platform_driver(usb2_phy_6502_driver);

MODULE_AUTHOR("Yu Chen <chenyu56@huawei.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hilisicon USB2 PHY 6502 Driver");
