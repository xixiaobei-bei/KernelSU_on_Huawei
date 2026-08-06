/*
 * wireless_power_supply.c
 *
 * power supply for wireless charging and reverse charging
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/boost_5v.h>
#include <huawei_platform/power/wireless_power_supply.h>

#define HWLOG_TAG wireless_ps
HWLOG_REGIST();

static struct wlps_dev_info *g_wlps_di;

static void wlps_rxsw_ctrl(struct wlps_dev_info *di, int flag)
{
	if (di->gpio_rxsw <= 0)
		return;

	if (flag == WLPS_CTRL_ON)
		gpio_set_value(di->gpio_rxsw, di->gpio_rxsw_valid_val);
	else
		gpio_set_value(di->gpio_rxsw, !di->gpio_rxsw_valid_val);

	hwlog_info("[%s] %s\n",
		__func__, (flag == WLPS_CTRL_ON) ? "on" : "off");
}

static void wlps_txsw_ctrl(struct wlps_dev_info *di, int flag)
{
	if (di->gpio_txsw <= 0)
		return;

	if (flag == WLPS_CTRL_ON)
		gpio_set_value(di->gpio_txsw, di->gpio_txsw_valid_val);
	else
		gpio_set_value(di->gpio_txsw, !di->gpio_txsw_valid_val);

	hwlog_info("[%s] %s\n",
		__func__, (flag == WLPS_CTRL_ON) ? "on" : "off");
}

static void wlps_rx_ext_pwr_ctrl(struct wlps_dev_info *di, int flag)
{
	if (di->gpio_ext_pwr_sw <= 0)
		return;

	if (flag == WLPS_CTRL_ON) {
		boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_WLC);
		gpio_set_value(di->gpio_ext_pwr_sw,
			di->gpio_ext_pwr_sw_valid_val);
	} else {
		gpio_set_value(di->gpio_ext_pwr_sw,
			!di->gpio_ext_pwr_sw_valid_val);
		boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_WLC);
	}

	hwlog_info("[%s] %s\n",
		__func__, (flag == WLPS_CTRL_ON) ? "on" : "off");
}

static void wlps_tx_sp_pwr_ctrl(struct wlps_dev_info *di, int flag)
{
	if (di->gpio_tx_sppwr_en <= 0)
		return;

	if (flag == WLPS_CTRL_ON) {
		gpio_set_value(di->gpio_tx_sppwr_en,
			di->gpio_tx_sppwr_en_valid_val);
		gpio_set_value(di->gpio_txsw, di->gpio_txsw_valid_val);
	} else {
		gpio_set_value(di->gpio_txsw, !di->gpio_txsw_valid_val);
		gpio_set_value(di->gpio_tx_sppwr_en,
			!di->gpio_tx_sppwr_en_valid_val);
	}

	hwlog_info("[%s] %s\n",
		__func__, (flag == WLPS_CTRL_ON) ? "on" : "off");
}

static void wlps_proc_otp_pwr_ctrl(struct wlps_dev_info *di, int flag)
{
	switch (di->proc_otp_pwr) {
	case WLPS_CHIP_PWR_NULL:
		wlps_rxsw_ctrl(di, flag);
		break;
	case WLPS_CHIP_PWR_RX:
		wlps_rx_ext_pwr_ctrl(di, flag);
		break;
	case WLPS_CHIP_PWR_TX:
		wlps_tx_sp_pwr_ctrl(di, flag);
		break;
	default:
		hwlog_err("%s: err scene(0x%x)", __func__, di->proc_otp_pwr);
		break;
	}
}

static void wlps_sysfs_en_pwr_ctrl(struct wlps_dev_info *di, int flag)
{
	switch (di->sysfs_en_pwr) {
	case WLPS_CHIP_PWR_NULL:
		wlps_rxsw_ctrl(di, flag);
		break;
	case WLPS_CHIP_PWR_RX:
		wlps_rx_ext_pwr_ctrl(di, flag);
		break;
	case WLPS_CHIP_PWR_TX:
		wlps_tx_sp_pwr_ctrl(di, flag);
		break;
	default:
		hwlog_err("%s: err scene(0x%x)", __func__, di->sysfs_en_pwr);
		break;
	}
}

void wlps_control(enum wlps_ctrl_scene scene, int ctrl_flag)
{
	struct wlps_dev_info *di = g_wlps_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	switch (scene) {
	case WLPS_PROBE_PWR:
		break;
	case WLPS_SYSFS_EN_PWR:
		wlps_sysfs_en_pwr_ctrl(di, ctrl_flag);
		break;
	case WLPS_PROC_OTP_PWR:
		wlps_proc_otp_pwr_ctrl(di, ctrl_flag);
		break;
	case WLPS_RX_EXT_PWR:
		wlps_rx_ext_pwr_ctrl(di, ctrl_flag);
		break;
	case WLPS_TX_PWR_SW:
		wlps_tx_sp_pwr_ctrl(di, ctrl_flag);
		break;
	case WLPS_RX_SW:
		wlps_rxsw_ctrl(di, ctrl_flag);
		break;
	case WLPS_TX_SW:
		wlps_txsw_ctrl(di, ctrl_flag);
		break;
	default:
		hwlog_err("%s: err scene(0x%x)", __func__, scene);
		break;
	}
}

static int wlps_tx_sw_gpio_init(struct device_node *np,
		struct wlps_dev_info *di)
{
	int ret;

	di->gpio_txsw = of_get_named_gpio(np, "gpio_txsw", 0);
	hwlog_info("[%s] gpio_txsw = %d\n", __func__, di->gpio_txsw);
	if (di->gpio_txsw < 0)
		return 0;

	if (!gpio_is_valid(di->gpio_txsw)) {
		hwlog_err("%s: gpio_txsw is not valid\n", __func__);
		di->gpio_txsw = 0;
		return -EINVAL;
	}
	ret = gpio_request(di->gpio_txsw, "wireless_txsw");
	if (ret) {
		hwlog_err("%s: could not request gpio_txsw\n", __func__);
		return -ENOMEM;
	}

	ret = of_property_read_u32(np, "gpio_txsw_valid_val",
			&di->gpio_txsw_valid_val);
	if (ret)
		di->gpio_txsw_valid_val = 1;  /* high valid */
	hwlog_info("[%s] gpio_txsw_valid_val = %d\n",
		__func__, di->gpio_txsw_valid_val);

	ret = gpio_direction_output(di->gpio_txsw, !di->gpio_txsw_valid_val);
	if (ret)
		hwlog_err("%s: gpio_dir_output fail\n", __func__);

	return 0;
}

static int wlps_tx_sppwr_gpio_init(struct device_node *np,
		struct wlps_dev_info *di)
{
	int ret;

	di->gpio_tx_sppwr_en = of_get_named_gpio(np, "gpio_tx_sppwr_en", 0);
	hwlog_info("[%s] gpio_tx_sppwr_en = %d\n",
		__func__, di->gpio_tx_sppwr_en);
	if (di->gpio_tx_sppwr_en < 0)
		return 0;

	if (!gpio_is_valid(di->gpio_tx_sppwr_en)) {
		hwlog_err("%s: gpio_tx_sppwr_en is not valid\n", __func__);
		di->gpio_tx_sppwr_en = 0;
		return -EINVAL;
	}
	ret = gpio_request(di->gpio_tx_sppwr_en, "wireless_tx_sppwr_en");
	if (ret) {
		hwlog_err("%s: could not request gpio_tx_sppwr_en\n", __func__);
		return -ENOMEM;
	}

	ret = of_property_read_u32(np, "gpio_tx_sppwr_en_valid_val",
			&di->gpio_tx_sppwr_en_valid_val);
	if (ret)
		di->gpio_tx_sppwr_en_valid_val = 1;  /* high valid */
	hwlog_info("[%s] gpio_txpwr_valid_val = %d\n",
		__func__, di->gpio_tx_sppwr_en_valid_val);

	ret = gpio_direction_output(di->gpio_tx_sppwr_en,
		!di->gpio_tx_sppwr_en_valid_val);
	if (ret)
		hwlog_err("%s: gpio_dir_output fail\n", __func__);

	return 0;
}

static int wlps_rx_sw_gpio_init(struct device_node *np,
	struct wlps_dev_info *di)
{
	int ret;

	di->gpio_rxsw = of_get_named_gpio(np, "gpio_rxsw", 0);
	hwlog_info("%s gpio_rxsw = %d\n", __func__, di->gpio_rxsw);
	if (di->gpio_rxsw < 0)
		return 0;
	if (!gpio_is_valid(di->gpio_rxsw)) {
		hwlog_err("gpio_rxsw is not valid\n");
		di->gpio_rxsw = 0;
		return -EINVAL;
	}
	ret = gpio_request(di->gpio_rxsw, "wireless_rxsw");
	if (ret) {
		hwlog_err("%s: could not request gpio_rxsw\n", __func__);
		return -ENOMEM;
	}

	ret = of_property_read_u32(np, "gpio_rxsw_valid_val",
			&di->gpio_rxsw_valid_val);
	if (ret)
		di->gpio_rxsw_valid_val = 0;  /* low valid */
	hwlog_info("[%s] gpio_rxsw_valid_val = %d\n",
		__func__, di->gpio_rxsw_valid_val);

	ret = gpio_direction_output(di->gpio_rxsw, di->gpio_rxsw_valid_val);
	if (ret)
		hwlog_err("%s: gpio_dir_output fail\n", __func__);

	return 0;
}

static int wlps_ext_pwr_sw_gpio_init(struct device_node *np,
		struct wlps_dev_info *di)
{
	int ret;

	di->gpio_ext_pwr_sw = of_get_named_gpio(np, "gpio_ext_pwr_sw", 0);
	hwlog_info("%s gpio_ext_pwr_sw = %d\n", __func__, di->gpio_ext_pwr_sw);
	if (di->gpio_ext_pwr_sw < 0)
		return 0;

	if (!gpio_is_valid(di->gpio_ext_pwr_sw)) {
		hwlog_err("%s: gpio_ext_pwr_sw is not valid\n", __func__);
		di->gpio_ext_pwr_sw = 0;
		return -EINVAL;
	}
	ret = gpio_request(di->gpio_ext_pwr_sw, "wireless_ext_pwr_sw");
	if (ret) {
		hwlog_err("%s: could not request gpio_ext_pwr_sw\n", __func__);
		return -ENOMEM;
	}

	ret = of_property_read_u32(np, "gpio_ext_pwr_sw_valid_val",
			&di->gpio_ext_pwr_sw_valid_val);
	if (ret)
		di->gpio_ext_pwr_sw_valid_val = 1;  /* high valid */
	hwlog_info("[%s] gpio_ext_pwr_sw_valid_val = %d\n",
		__func__, di->gpio_ext_pwr_sw_valid_val);

	ret = gpio_direction_output(di->gpio_ext_pwr_sw,
		!di->gpio_ext_pwr_sw_valid_val);
	if (ret)
		hwlog_err("%s: gpio_dir_output fail\n", __func__);

	return 0;
}

static void wlps_parse_dts(struct device_node *np,
		struct wlps_dev_info *di)
{
	int ret;

	ret = of_property_read_u32(np, "sysfs_en_pwr",
			(u32 *)&di->sysfs_en_pwr);
	if (ret)
		di->sysfs_en_pwr = WLPS_CHIP_PWR_NULL;
	hwlog_info("[%s] sysfs_en_pwr = %d\n", __func__, di->sysfs_en_pwr);

	ret = of_property_read_u32(np, "proc_otp_pwr",
			(u32 *)&di->proc_otp_pwr);
	if (ret)
		di->proc_otp_pwr = WLPS_CHIP_PWR_NULL;
	hwlog_info("[%s] proc_otp_pwr = %d\n", __func__, di->proc_otp_pwr);
}

static int wireless_ps_probe(struct platform_device *pdev)
{
	int ret;
	struct wlps_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_wlps_di = di;
	di->dev = &pdev->dev;
	np = pdev->dev.of_node;

	wlps_parse_dts(np, di);

	ret = wlps_tx_sw_gpio_init(np, di);
	if (ret)
		goto tx_sw_gpio_init_fail;
	ret = wlps_tx_sppwr_gpio_init(np, di);
	if (ret)
		goto tx_sppwr_gpio_init_fail;
	ret = wlps_rx_sw_gpio_init(np, di);
	if (ret)
		goto rx_sw_gpio_init_fail;
	ret = wlps_ext_pwr_sw_gpio_init(np, di);
	if (ret)
		goto ext_pwr_sw_gpio_init_fail;

	hwlog_info("wireless_tx probe ok\n");
	return 0;

ext_pwr_sw_gpio_init_fail:
	if (di->gpio_rxsw > 0)
		gpio_free(di->gpio_rxsw);
rx_sw_gpio_init_fail:
	if (di->gpio_tx_sppwr_en > 0)
		gpio_free(di->gpio_tx_sppwr_en);
tx_sppwr_gpio_init_fail:
	if (di->gpio_txsw > 0)
		gpio_free(di->gpio_txsw);
tx_sw_gpio_init_fail:
	kfree(di);
	di = NULL;
	g_wlps_di = NULL;
	hwlog_info("wireless_ps probe fail\n");
	return ret;
}

static void wireless_ps_shutdown(struct platform_device *pdev)
{
	wlps_control(WLPS_TX_PWR_SW, WLPS_CTRL_OFF);
	wlps_control(WLPS_RX_SW, WLPS_CTRL_OFF);
}

static const struct of_device_id wlps_match_table[] = {
	{
		.compatible = "huawei,wireless_ps",
		.data = NULL,
	},
	{},
};

static struct platform_driver wlps_driver = {
	.probe = wireless_ps_probe,
	.shutdown = wireless_ps_shutdown,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_ps",
		.of_match_table = of_match_ptr(wlps_match_table),
	},
};

static int __init wireless_ps_init(void)
{
	return platform_driver_register(&wlps_driver);
}

static void __exit wireless_ps_exit(void)
{
	platform_driver_unregister(&wlps_driver);
}

fs_initcall(wireless_ps_init);
module_exit(wireless_ps_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless power supply module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
