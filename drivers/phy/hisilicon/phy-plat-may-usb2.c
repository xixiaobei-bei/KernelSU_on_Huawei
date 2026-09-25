// SPDX-License-Identifier: GPL-2.0
/*
 * Phy provider for USB2 on HiSilicon May platform
 *
 * Copyright (C) 2017-2018 Hilisicon Electronics Co., Ltd.
 *		http://www.huawei.com
 *
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

#include <soc_usb_misc_ctrl_interface.h>

#define MAY_USB2_INFO(format, arg...) \
	pr_info("[MAY_USB2][I][%s]"format, __func__, ##arg)

#define MAY_USB2_ERR(format, arg...) \
	pr_err("[MAY_USB2][E][%s]"format, __func__, ##arg)

#define hisi_usb2_phy_to_phy_priv(pointer) \
	container_of(pointer, struct phy_priv, usb_phy)

/* MISC CTRL */
#define SC_USB20PHY_MISC_CTRL5_OFFSET		0x14
#define SC_USB20PHY_MISC_CTRL6_OFFSET		0x18
#define SC_USB20PHY_MISC_CTRL7_OFFSET		0x1c
#define SC_USB20PHY_MISC_CTRL9_OFFSET		0x24
#define SOC_USB_MISC_CTRL_USB_MISC_CFGA0_OFFSET	0xa0
/* bit of SC_USB20PHY_MISC_CTRL5 */
#define USB2_SIDDQ				SOC_USB_MISC_CTRL_sc_usb20phy_misc_ctrl5_test_iddq_START
/* bit of SC_USB20PHY_MISC_CTRL6 */
#define MISC_CTRL6_PHY_RESET			SOC_USB_MISC_CTRL_sc_usb20phy_misc_ctrl6_phy_reset_START
#define MISC_CTRL6_REPEATOR_RESET		SOC_USB_MISC_CTRL_sc_usb20phy_misc_ctrl6_repeator_reset_START
#define MISC_CTRL6_ULPI_RESET			SOC_USB_MISC_CTRL_sc_usb20phy_misc_ctrl6_ulpi_reset_START
/* bit of SC_USB20PHY_MISC_CTRL7 */
#define MISC_CTRL7_UTMI_VBUS_VALID		BIT(SOC_USB_MISC_CTRL_sc_usb20phy_misc_ctrl7_utmi_vbus_valid_ext_START)
#define MISC_CTRL7_DWC_BVALID			BIT(SOC_USB_MISC_CTRL_sc_usb20phy_misc_ctrl7_dwc_bvalid_START)
/* bit of SC_USB20PHY_MISC_CTRL9 */
#define MISC_CTRL9_BVALID_CFG			(1 << 0)
#define MISC_CTRL9_SESSEND_CFG			(1 << 1)
#define MISC_CTRL9_VBUSVALID_CFG		(1 << 2)

#define DEFAULT_T28_EYE_PARAM 0x0246709C
#define DEFAULT_T28_HOST_EYE_PARAM 0x0246709E
#define DEFAULT_S28_EYE_PARAM 0xB760
#define DEFAULT_S28_HOST_EYE_PARAM 0xB760
#define DEFAULT_ULPI_IOC_DRIVING_FORCE	0x00FB004A

struct phy_priv {
	struct device *dev;
	struct hisi_usb2_phy usb_phy;
	struct phy *phy;

	void __iomem *usb_misc_ctrl;
	void __iomem *ioc_hsdt1;
	struct clk *gt_clk_ulpi_ref;

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
	uint32_t temp;

	MAY_USB2_INFO("+\n");
	/* usb phy chip unreset */
	upc_open();

	ioc_hsdt1_config_ulpi(priv);
	ret = misc_ctrl_init();
	if (ret)
		return ret;

	/* enable ULPI ref clock */
	ret = clk_prepare_enable(priv->gt_clk_ulpi_ref);
	if (ret) {
		MAY_USB2_ERR("clk_prepare_enable ulpi ref clk failed\n");
		goto out_exit_misc_ctrl;
	}

	/* deassert ULPI phy */
	upc_phy_unreset();

	/* open ulpi vbusvalid and bvalid */
	temp = readl(priv->usb_misc_ctrl + SC_USB20PHY_MISC_CTRL9_OFFSET);
	temp &= ~(MISC_CTRL9_SESSEND_CFG);
	temp |= (MISC_CTRL9_BVALID_CFG | MISC_CTRL9_VBUSVALID_CFG);
	writel(temp, priv->usb_misc_ctrl + SC_USB20PHY_MISC_CTRL9_OFFSET);

	udelay(100);

	/* set vbus valid */
	temp = readl(priv->usb_misc_ctrl + SC_USB20PHY_MISC_CTRL7_OFFSET);
	temp |= MISC_CTRL7_UTMI_VBUS_VALID;
	writel(temp, priv->usb_misc_ctrl + SC_USB20PHY_MISC_CTRL7_OFFSET);
	udelay(100);
	temp = readl(priv->usb_misc_ctrl + SC_USB20PHY_MISC_CTRL7_OFFSET);
	temp |= MISC_CTRL7_DWC_BVALID;
	writel(temp, priv->usb_misc_ctrl + SC_USB20PHY_MISC_CTRL7_OFFSET);

	MAY_USB2_INFO("set phy param in %s mode\n",
			(priv->mode == PHY_MODE_USB_HOST) ? "host" : "device");
	/* set eye param */
	if (priv->mode == PHY_MODE_USB_HOST)
		upc_set_eye_diagram_param(priv->phy_host_param_t28,
				priv->phy_host_param_s28);
	else
		upc_set_eye_diagram_param(priv->phy_param_t28,
				priv->phy_param_s28);

	MAY_USB2_INFO("-\n");
	return 0;

out_exit_misc_ctrl:
	misc_ctrl_exit();
	MAY_USB2_ERR("failed to init phy ret: %d\n", ret);
	return ret;
}

static int usb2_phy_init(struct phy *phy)
{
	struct hisi_usb2_phy *usb2_phy =
		(struct hisi_usb2_phy *)phy_get_drvdata(phy);
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -ENODEV;

	priv = hisi_usb2_phy_to_phy_priv(usb2_phy);

	return _usb2_phy_init(priv);
}

static int _usb2_phy_exit(struct phy_priv *priv)
{
	MAY_USB2_INFO("+\n");
	/* reset ULPI phy */
	upc_phy_reset();

	clk_disable_unprepare(priv->gt_clk_ulpi_ref);

	misc_ctrl_exit();
	/* usb phy chip reset */
	upc_close();
	MAY_USB2_INFO("-\n");
	return 0;
}

static int usb2_phy_exit(struct phy *phy)
{
	struct hisi_usb2_phy *usb2_phy =
		(struct hisi_usb2_phy *)phy_get_drvdata(phy);
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -ENODEV;

	priv = hisi_usb2_phy_to_phy_priv(usb2_phy);

	return _usb2_phy_exit(priv);
}

static int usb2_phy_poweron(struct phy *phy)
{
	/* avoid plus on S28, called after regulator enable */
	MAY_USB2_INFO("+\n");
	upc_remove_pw_burr_enable(UPC_BURR_PW_ON, UPC_BURR_NORMAL_CFG);
	MAY_USB2_INFO("-\n");

	return 0;
}

static int usb2_phy_poweroff(struct phy *phy)
{
	/* avoid plus on S28, called before regulator disable */
	MAY_USB2_INFO("+\n");
	upc_remove_pw_burr_enable(UPC_BURR_PW_OFF, UPC_BURR_NORMAL_CFG);
	MAY_USB2_INFO("-\n");

	return 0;
}

static int usb2_phy_set_mode(struct phy *phy, enum phy_mode mode)
{
	struct hisi_usb2_phy *usb2_phy =
		(struct hisi_usb2_phy *)phy_get_drvdata(phy);
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -ENODEV;

	priv = hisi_usb2_phy_to_phy_priv(usb2_phy);

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

static int shared_phy_init(struct hisi_usb2_phy *usb2_phy)
{
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -EINVAL;

	priv = hisi_usb2_phy_to_phy_priv(usb2_phy);

	return _usb2_phy_init(priv);
}

static int shared_phy_exit(struct hisi_usb2_phy *usb2_phy)
{
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -EINVAL;

	priv = hisi_usb2_phy_to_phy_priv(usb2_phy);

	return _usb2_phy_exit(priv);
}

static enum hisi_charger_type detect_charger_type(
		struct hisi_usb2_phy *usb2_phy)
{
	struct phy_priv *priv = NULL;
	enum hisi_charger_type charger_type;

	if (!usb2_phy)
		return CHARGER_TYPE_NONE;

	priv = hisi_usb2_phy_to_phy_priv(usb2_phy);

	if (priv->disable_bc)
		return CHARGER_TYPE_SDP;

	mutex_lock(&priv->phy->mutex);
	if (priv->phy->init_count == 0) {
		mutex_unlock(&priv->phy->mutex);
		MAY_USB2_ERR("phy has not been initialized\n");
		return CHARGER_TYPE_NONE;
	}

	charger_type = upc_detect_charger_type();

	mutex_unlock(&priv->phy->mutex);

	return charger_type;
}

static int set_dpdm_pulldown(struct hisi_usb2_phy *usb2_phy, bool pulldown)
{
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -EINVAL;

	priv = hisi_usb2_phy_to_phy_priv(usb2_phy);

	if (priv->disable_bc)
		return 0;

	mutex_lock(&priv->phy->mutex);
	if (priv->phy->init_count == 0) {
		mutex_unlock(&priv->phy->mutex);
		MAY_USB2_ERR("phy has not been initialized\n");
		return -ENODEV;
	}

	if (pulldown)
		upc_hisi_bc_dplus_pulldown();
	else
		upc_hisi_bc_dplus_pullup();

	mutex_unlock(&priv->phy->mutex);

	return 0;
}

static int _set_vdp_src(struct phy_priv *priv, bool enable)
{
	if (priv->disable_bc)
		return 0;

	mutex_lock(&priv->phy->mutex);
	if (priv->phy->init_count == 0) {
		mutex_unlock(&priv->phy->mutex);
		MAY_USB2_ERR("phy has not been initialized\n");
		return -ENODEV;
	}

	if (enable)
		upc_hisi_bc_enable_vdp_src();
	else
		upc_hisi_bc_disable_vdp_src();

	mutex_unlock(&priv->phy->mutex);

	return 0;
}

static int set_vdp_src(struct hisi_usb2_phy *usb2_phy, bool enable)
{
	struct phy_priv *priv = NULL;

	if (!usb2_phy)
		return -EINVAL;

	priv = hisi_usb2_phy_to_phy_priv(usb2_phy);

	if (in_atomic() || irqs_disabled()) {
		if (enable)
			atomic_set(&priv->desired_vdp_src, 1);
		else
			atomic_set(&priv->desired_vdp_src, 0);

		if (!queue_work(system_power_efficient_wq,
					&priv->vdp_src_work)) {
			MAY_USB2_INFO("vdp src work already in queue\n");
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
		MAY_USB2_ERR("set vdp src failed %d\n", ret);
}

static int may_get_clk_resource(struct phy_priv *priv)
{
	struct device *dev = priv->dev;

	/* get ulpi ref clk handler */
	priv->gt_clk_ulpi_ref = devm_clk_get(dev, "clk_ulpi_ref");
	if (IS_ERR_OR_NULL(priv->gt_clk_ulpi_ref)) {
		MAY_USB2_ERR("get gt_clk_ulpi_ref failed\n");
		return -EINVAL;
	}

	return 0;
}

static void may_put_dts_resource(struct phy_priv *priv)
{
	iounmap(priv->usb_misc_ctrl);
	iounmap(priv->ioc_hsdt1);
}

static int map_regs_from_dts(struct phy_priv *priv)
{
	struct device_node *np = NULL;

	/*
	 * map USB MISC CTRL region
	 */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,usb-misc-ctrl");
	if (!np) {
		MAY_USB2_ERR("get usb misc ctrl node failed!\n");
		return -EINVAL;
	}

	priv->usb_misc_ctrl = of_iomap(np, 0);
	if (!priv->usb_misc_ctrl) {
		MAY_USB2_ERR("iomap usb misc ctrl reg_base failed!\n");
		return -ENOMEM;
	}

	/*
	 * map IOC HSDT1 region
	 */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,ioc-hsdt1-usb");
	if (!np) {
		MAY_USB2_ERR("get ioc hsdt1 node failed!\n");
		iounmap(priv->usb_misc_ctrl);
		priv->usb_misc_ctrl = NULL;
		return -EINVAL;
	}

	priv->ioc_hsdt1 = of_iomap(np, 0);
	if (!priv->ioc_hsdt1) {
		MAY_USB2_ERR("iomap ioc hsdt1 reg_base failed!\n");
		iounmap(priv->usb_misc_ctrl);
		priv->usb_misc_ctrl = NULL;
		return -ENOMEM;
	}

	return 0;
}

static void may_get_phy_param_form_dt(struct phy_priv *priv)
{
	int ret;

	ret = of_property_read_u32(priv->dev->of_node, "phy-param-t28",
		&priv->phy_param_t28);
	if (ret) {
		MAY_USB2_ERR("read phy-param-t28 failed, ret %d\n", ret);
		priv->phy_param_t28 = DEFAULT_T28_EYE_PARAM;
	}

	ret = of_property_read_u32(priv->dev->of_node, "phy-host-param-t28",
		&priv->phy_host_param_t28);
	if (ret) {
		MAY_USB2_ERR("read phy-host-param-t28 failed, ret %d\n", ret);
		priv->phy_host_param_t28 = DEFAULT_T28_HOST_EYE_PARAM;
	}

	ret = of_property_read_u32(priv->dev->of_node, "phy-param-s28",
		&priv->phy_param_s28);
	if (ret) {
		MAY_USB2_ERR("read phy-param-s28 failed, ret %d\n", ret);
		priv->phy_param_s28 = DEFAULT_S28_EYE_PARAM;
	}

	ret = of_property_read_u32(priv->dev->of_node, "phy-host-param-s28",
		&priv->phy_host_param_s28);
	if (ret) {
		MAY_USB2_ERR("read phy-host-param-s28 failed, ret %d\n", ret);
		priv->phy_host_param_s28 = DEFAULT_S28_HOST_EYE_PARAM;
	}
}

static void may_get_io_config_form_dt(struct phy_priv *priv)
{
	int ret;

	ret = of_property_read_u32(priv->dev->of_node, "ulpi-ioc-data-driving-t28",
		&priv->ulpi_data_driving_force_t28);
	if (ret) {
		MAY_USB2_ERR("read ulpi-ioc-data-driving-t28 failed, ret %d\n", ret);
		priv->ulpi_data_driving_force_t28 = DEFAULT_ULPI_IOC_DRIVING_FORCE;
	}

	ret = of_property_read_u32(priv->dev->of_node, "ulpi-ioc-stp-driving-t28",
		&priv->ulpi_stp_driving_force_t28);
	if (ret) {
		MAY_USB2_ERR("read ulpi-ioc-stp-driving-t28 failed, ret %d\n", ret);
		priv->ulpi_stp_driving_force_t28 = DEFAULT_ULPI_IOC_DRIVING_FORCE;
	}

	ret = of_property_read_u32(priv->dev->of_node, "ulpi-ioc-data-driving-s28",
		&priv->ulpi_data_driving_force_s28);
	if (ret) {
		MAY_USB2_ERR("read ulpi-ioc-data-driving-s28 failed, ret %d\n", ret);
		priv->ulpi_data_driving_force_s28 = DEFAULT_ULPI_IOC_DRIVING_FORCE;
	}

	ret = of_property_read_u32(priv->dev->of_node, "ulpi-ioc-stp-driving-s28",
		&priv->ulpi_stp_driving_force_s28);
	if (ret) {
		MAY_USB2_ERR("read ulpi-ioc-stp-driving-s28 failed, ret %d\n", ret);
		priv->ulpi_stp_driving_force_s28 = DEFAULT_ULPI_IOC_DRIVING_FORCE;
	}

}

static int may_get_dts_resource(struct phy_priv *priv)
{
	int ret;

	if (may_get_clk_resource(priv))
		return -EINVAL;

	ret = map_regs_from_dts(priv);
	if (ret) {
		MAY_USB2_ERR("map_regs_from_dts failed\n");
		return ret;
	}

	priv->disable_bc = of_property_read_bool(priv->dev->of_node,
			"disable-bc");
	MAY_USB2_INFO("disable_bc %d!\n", priv->disable_bc);

	priv->ulpi_ioc_stub = of_property_read_bool(priv->dev->of_node,
			"ulpi-ioc-cfg-stub");
	MAY_USB2_INFO("ulpi-ioc-cfg-stub %d!\n", priv->ulpi_ioc_stub);

	/* get usb phy eye param config */
	may_get_phy_param_form_dt(priv);

	/* get usb phy io driving force config */
	may_get_io_config_form_dt(priv);

	return 0;
}

static int may_usb2_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct phy_provider *phy_provider = NULL;
	struct phy *phy = NULL;
	struct phy_priv *priv = NULL;
	int ret;

	MAY_USB2_INFO("+\n");
	if (!misc_ctrl_is_ready()) {
		MAY_USB2_ERR("misc ctrl is not ready\n");
		return -EPROBE_DEFER;
	}

	if (!upc_phy_driver_is_ready()) {
		MAY_USB2_ERR("upc phy driver is not ready\n");
		return -EPROBE_DEFER;
	}

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_no_callbacks(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		MAY_USB2_ERR("pm_runtime_get_sync failed\n");
		goto err_pm_put;
	}

	pm_runtime_forbid(dev);

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto err_pm_allow;
	}

	priv->dev = dev;

	ret = may_get_dts_resource(priv);
	if (ret) {
		MAY_USB2_ERR("get_dts_resource failed\n");
		goto err_pm_allow;
	}

	phy = devm_phy_create(dev, NULL, &usb2_phy_ops);
	if (IS_ERR(phy)) {
		ret = PTR_ERR(phy);
		goto err_pm_allow;
	}

	priv->phy = phy;
	priv->usb_phy.shared_phy_init = shared_phy_init;
	priv->usb_phy.shared_phy_exit = shared_phy_exit;
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
		goto err_pm_allow;
	}

	platform_set_drvdata(pdev, priv);

	MAY_USB2_INFO("-\n");
	return 0;

err_pm_allow:
	pm_runtime_allow(dev);
err_pm_put:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);

	MAY_USB2_INFO("ret %d\n", ret);
	return ret;
}

static int may_usb2_phy_remove(struct platform_device *pdev)
{
	struct phy_priv *priv = (struct phy_priv *)platform_get_drvdata(pdev);

	if (cancel_work_sync(&priv->vdp_src_work))
		MAY_USB2_INFO("vdp src work canceled\n");

	may_put_dts_resource(priv);

	pm_runtime_allow(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static const struct of_device_id may_usb2_phy_of_match[] = {
	{.compatible = "hisilicon,may-usb2-phy",},
	{ }
};
MODULE_DEVICE_TABLE(of, may_usb2_phy_of_match);

static struct platform_driver may_usb2_phy_driver = {
	.probe	= may_usb2_phy_probe,
	.remove = may_usb2_phy_remove,
	.driver = {
		.name	= "may-usb2-phy",
		.of_match_table	= may_usb2_phy_of_match,
	}
};
module_platform_driver(may_usb2_phy_driver);

MODULE_AUTHOR("Yu Chen <chenyu56@huawei.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hilisicon USB2 PHY Driver");
