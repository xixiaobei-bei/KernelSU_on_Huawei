/*
 * wireless_power_supply.c
 *
 * power supply for wireless charging and reverse charging
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/device.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>

#define HWLOG_TAG wireless_ps
HWLOG_REGIST();

enum wlps_gpio_type {
	WLPS_GPIO_BEGIN = 0,
	WLPS_GPIO_RXSW = WLPS_GPIO_BEGIN,
	WLPS_GPIO_RXSW_AUX,
	WLPS_GPIO_TXSW,
	WLPS_GPIO_SC_SW2,
	WLPS_GPIO_EXT_PWR_SW,
	WLPS_GPIO_TX_SPPWR_EN,
	WLPS_GPIO_END,
};

struct wlps_dev_info {
	struct device *dev;
	int gpio_tx_sppwr_en;
	int gpio_tx_sppwr_en_valid_val;
	bool tx_sppwr_sw_on;
	int gpio_txsw;
	int gpio_txsw_valid_val;
	int gpio_rxsw;
	int gpio_rxsw_valid_val;
	int gpio_rxsw_aux;
	int gpio_rxsw_aux_valid_val;
	int gpio_ext_pwr_sw;
	int gpio_ext_pwr_sw_valid_val;
	int gpio_sc_sw2;
	int gpio_sc_sw2_valid_val;
	int sysfs_en_pwr;
	int proc_otp_pwr;
	int recover_otp_pwr;
};

static struct wlps_dev_info *g_wlps_di[WLTRX_IC_MAX];

static void wlps_gpio_val_check(unsigned int gpio_type, int gpio_no, int set_val)
{
	int err_no;
	int gpio_val;
	char dsm_buf[POWER_DSM_BUF_SIZE_0128] = { 0 };
	static const char * const wlps_gpio_str[] = {
		[WLPS_GPIO_RXSW]        = "rxsw_ctrl",
		[WLPS_GPIO_RXSW_AUX]    = "rxsw_aux_ctrl",
		[WLPS_GPIO_TXSW]        = "txsw_ctrl",
		[WLPS_GPIO_SC_SW2]      = "sc_sw2_ctrl",
		[WLPS_GPIO_EXT_PWR_SW]  = "rx_ext_pwr_ctrl",
		[WLPS_GPIO_TX_SPPWR_EN] = "tx_sp_pwr_ctrl",
	};

	if ((gpio_no <= 0) || (gpio_type < WLPS_GPIO_BEGIN) ||
		(gpio_type >= WLPS_GPIO_END))
		return;

	gpio_val = gpio_get_value(gpio_no);
	hwlog_info("[%s] set_val=%d, actual_val=%d\n", wlps_gpio_str[gpio_type],
		set_val, gpio_val);

	if (gpio_val == set_val)
		return;

	switch (gpio_type) {
	case WLPS_GPIO_TXSW:
		snprintf(dsm_buf, sizeof(dsm_buf), "gpio_txsw: set=%d actual=%d\n",
			set_val, gpio_val);
		err_no = DSM_WLPS_GPIO_TXSW;
		break;
	default:
		return;
	}
	power_dsm_report_dmd(POWER_DSM_BATTERY, err_no, dsm_buf);
}

static struct wlps_dev_info *wlps_get_dev_info(unsigned int ic_type)
{
	if (!wltrx_ic_is_type_valid(ic_type)) {
		hwlog_err("get_dev_info: ic_type=%u err\n", ic_type);
		return NULL;
	}

	return g_wlps_di[ic_type];
}

void wlps_boost_5v_enable(unsigned int ic_type, unsigned int role_type, bool flag)
{
	if (ic_type == WLTRX_IC_MAIN) {
		if (role_type == WIRELESS_RX)
			boost_5v_enable(flag, BOOST_CTRL_WLRX);
		else if (role_type == WIRELESS_TX)
			boost_5v_enable(flag, BOOST_CTRL_WLTX);
	} else if (ic_type == WLTRX_IC_AUX) {
		if (role_type == WIRELESS_RX)
			boost_5v_enable(flag, BOOST_CTRL_WLRX_AUX);
		else if (role_type == WIRELESS_TX)
			boost_5v_enable(flag, BOOST_CTRL_WLTX_AUX);
	}
}

static void wlps_rxsw_ctrl(unsigned int ic_type, bool flag)
{
	int set_val;
	struct wlps_dev_info *di = wlps_get_dev_info(ic_type);

	if (!di || (di->gpio_rxsw <= 0))
		return;

	if (flag) {
		if (di->tx_sppwr_sw_on) {
			hwlog_err("rxsw_ctrl: tx_pwr_sw on, ignore rx_sw on\n");
			return;
		}
		set_val = di->gpio_rxsw_valid_val;
	} else {
		set_val = !di->gpio_rxsw_valid_val;
	}
	gpio_set_value(di->gpio_rxsw, set_val);
	hwlog_info("[rxsw_ctrl] %s\n", flag ? "on" : "off");
	wlps_gpio_val_check(WLPS_GPIO_RXSW, di->gpio_rxsw, set_val);
}

static void wlps_rxsw_aux_ctrl(unsigned int ic_type, bool flag)
{
	int set_val;
	struct wlps_dev_info *di = wlps_get_dev_info(ic_type);

	if (!di || (di->gpio_rxsw_aux <= 0))
		return;

	if (flag)
		set_val = di->gpio_rxsw_aux_valid_val;
	else
		set_val = !di->gpio_rxsw_aux_valid_val;
	gpio_set_value(di->gpio_rxsw_aux, set_val);
	hwlog_info("[rxsw_aux_ctrl] %s\n", flag ? "on" : "off");
	wlps_gpio_val_check(WLPS_GPIO_RXSW_AUX, di->gpio_rxsw_aux, set_val);
}

static void wlps_txsw_ctrl(unsigned int ic_type, bool flag)
{
	int set_val;
	struct wlps_dev_info *di = wlps_get_dev_info(ic_type);

	if (!di || (di->gpio_txsw <= 0))
		return;

	if (flag)
		set_val = di->gpio_txsw_valid_val;
	else
		set_val = !di->gpio_txsw_valid_val;
	gpio_set_value(di->gpio_txsw, set_val);
	hwlog_info("[txsw_ctrl] %s\n", flag ? "on" : "off");
	wlps_gpio_val_check(WLPS_GPIO_TXSW, di->gpio_txsw, set_val);
}

static void wlps_sc_sw2_ctrl(unsigned int ic_type, bool flag)
{
	int set_val;
	struct wlps_dev_info *di = wlps_get_dev_info(ic_type);

	if (!di || (di->gpio_sc_sw2 <= 0))
		return;

	if (flag)
		set_val = di->gpio_sc_sw2_valid_val;
	else
		set_val = !di->gpio_sc_sw2_valid_val;
	gpio_set_value(di->gpio_sc_sw2, set_val);
	hwlog_info("[sc_sw2_ctrl] %s\n", flag ? "on" : "off");
	wlps_gpio_val_check(WLPS_GPIO_SC_SW2, di->gpio_sc_sw2, set_val);
}

static void wlps_rx_ext_pwr_ctrl(unsigned int ic_type, bool flag)
{
	int set_val;
	struct wlps_dev_info *di = wlps_get_dev_info(ic_type);

	if (!di || (di->gpio_ext_pwr_sw <= 0))
		return;

	if (flag) {
		wlrx_ic_ext_pwr_prev_ctrl(ic_type, true);
		wlps_boost_5v_enable(ic_type, WIRELESS_RX, true);
		power_usleep(DT_USLEEP_10MS);
		set_val = di->gpio_ext_pwr_sw_valid_val;
		gpio_set_value(di->gpio_ext_pwr_sw, set_val);
		wlrx_ic_ext_pwr_post_ctrl(ic_type, true);
	} else {
		wlrx_ic_ext_pwr_post_ctrl(ic_type, false);
		set_val = !di->gpio_ext_pwr_sw_valid_val;
		gpio_set_value(di->gpio_ext_pwr_sw, set_val);
		power_usleep(DT_USLEEP_10MS);
		wlps_boost_5v_enable(ic_type, WIRELESS_RX, false);
		wlrx_ic_ext_pwr_prev_ctrl(ic_type, false);
	}

	hwlog_info("[rx_ext_pwr_ctrl] %s\n", flag ? "on" : "off");
	wlps_gpio_val_check(WLPS_GPIO_EXT_PWR_SW, di->gpio_ext_pwr_sw, set_val);
}

static void wlps_tx_sp_pwr_ctrl(unsigned int ic_type, bool flag)
{
	int tx_sppwr_en_set_val;
	struct wlps_dev_info *di = wlps_get_dev_info(ic_type);

	if (!di || (di->gpio_tx_sppwr_en <= 0))
		return;

	if (flag) {
		di->tx_sppwr_sw_on = true;
		wlps_rxsw_ctrl(ic_type, false);
		wlps_txsw_ctrl(ic_type, flag);
		power_usleep(DT_USLEEP_10MS);
		tx_sppwr_en_set_val = di->gpio_tx_sppwr_en_valid_val;
		gpio_set_value(di->gpio_tx_sppwr_en, tx_sppwr_en_set_val);
	} else {
		di->tx_sppwr_sw_on = false;
		wlps_txsw_ctrl(ic_type, flag);
		power_usleep(DT_USLEEP_10MS);
		tx_sppwr_en_set_val = !di->gpio_tx_sppwr_en_valid_val;
		gpio_set_value(di->gpio_tx_sppwr_en, tx_sppwr_en_set_val);
	}

	hwlog_info("[tx_sp_pwr_ctrl] %s\n", flag ? "on" : "off");
	wlps_gpio_val_check(WLPS_GPIO_TX_SPPWR_EN, di->gpio_tx_sppwr_en, tx_sppwr_en_set_val);
}

static void wlps_chip_pwr_ctrl(unsigned int ic_type, int pwr_type, bool flag)
{
	switch (pwr_type) {
	case WLPS_CHIP_PWR_NULL:
		wlps_rxsw_ctrl(ic_type, flag);
		break;
	case WLPS_CHIP_PWR_RX:
		wlps_rx_ext_pwr_ctrl(ic_type, flag);
		break;
	case WLPS_CHIP_PWR_TX:
		wlps_tx_sp_pwr_ctrl(ic_type, flag);
		break;
	default:
		hwlog_err("chip_pwr_ctrl: err_type=0x%x\n", pwr_type);
		break;
	}
}

static void wlps_sysfs_en_pwr_ctrl(unsigned int ic_type, bool flag)
{
	struct wlps_dev_info *di = wlps_get_dev_info(ic_type);

	if (!di)
		return;

	wlps_chip_pwr_ctrl(ic_type, di->sysfs_en_pwr, flag);
}

static void wlps_proc_otp_pwr_ctrl(unsigned int ic_type, bool flag)
{
	struct wlps_dev_info *di = wlps_get_dev_info(ic_type);

	if (!di)
		return;

	wlps_chip_pwr_ctrl(ic_type, di->proc_otp_pwr, flag);
}

static void wlps_recover_otp_pwr_ctrl(unsigned int ic_type, bool flag)
{
	struct wlps_dev_info *di = wlps_get_dev_info(ic_type);

	if (!di)
		return;

	wlps_chip_pwr_ctrl(ic_type, di->recover_otp_pwr, flag);
}

void wlps_control(unsigned int ic_type, int scene, bool flag)
{
	switch (scene) {
	case WLPS_SYSFS_EN_PWR:
		wlps_sysfs_en_pwr_ctrl(ic_type, flag);
		break;
	case WLPS_PROC_OTP_PWR:
		wlps_proc_otp_pwr_ctrl(ic_type, flag);
		break;
	case WLPS_RECOVER_OTP_PWR:
		wlps_recover_otp_pwr_ctrl(ic_type, flag);
		break;
	case WLPS_RX_EXT_PWR:
		wlps_rx_ext_pwr_ctrl(ic_type, flag);
		break;
	case WLPS_TX_PWR_SW:
		wlps_tx_sp_pwr_ctrl(ic_type, flag);
		break;
	case WLPS_RX_SW:
		wlps_rxsw_ctrl(ic_type, flag);
		break;
	case WLPS_RX_SW_AUX:
		wlps_rxsw_aux_ctrl(ic_type, flag);
		break;
	case WLPS_TX_SW:
		wlps_txsw_ctrl(ic_type, flag);
		break;
	case WLPS_SC_SW2:
		wlps_sc_sw2_ctrl(ic_type, flag);
		break;
	default:
		hwlog_err("error scene: %d\n", scene);
		break;
	}
}

static void wlps_gpio_free(int gpio_no)
{
	if (gpio_no > 0)
		gpio_free(gpio_no);
}

static int wlps_request_tx_sw_gpio(struct device_node *np, struct wlps_dev_info *di)
{
	di->gpio_txsw = of_get_named_gpio(np, "gpio_txsw", 0);
	hwlog_info("gpio_txsw=%d\n", di->gpio_txsw);
	if (di->gpio_txsw <= 0)
		return 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_txsw_valid_val", (u32 *)&di->gpio_txsw_valid_val, 1);
	if (power_gpio_config_output(np, "gpio_txsw", "wireless_txsw",
		&di->gpio_txsw, !di->gpio_txsw_valid_val))
		return -EINVAL;

	return 0;
}

static int wlps_request_tx_sppwr_gpio(struct device_node *np, struct wlps_dev_info *di)
{
	di->gpio_tx_sppwr_en = of_get_named_gpio(np, "gpio_tx_sppwr_en", 0);
	hwlog_info("gpio_tx_sppwr_en=%d\n", di->gpio_tx_sppwr_en);
	if (di->gpio_tx_sppwr_en <= 0)
		return 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_tx_sppwr_en_valid_val", (u32 *)&di->gpio_tx_sppwr_en_valid_val, 1);
	if (power_gpio_config_output(np, "gpio_tx_sppwr_en",
		"wireless_tx_sppwr_en", &di->gpio_tx_sppwr_en,
		!di->gpio_tx_sppwr_en_valid_val))
		return -EINVAL;

	return 0;
}

static int wlps_request_rx_sw_gpio(struct device_node *np, struct wlps_dev_info *di)
{
	di->gpio_rxsw = of_get_named_gpio(np, "gpio_rxsw", 0);
	hwlog_info("gpio_rxsw=%d\n", di->gpio_rxsw);
	if (di->gpio_rxsw <= 0)
		return 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_rxsw_valid_val", (u32 *)&di->gpio_rxsw_valid_val, 0);
	if (power_gpio_config_output(np, "gpio_rxsw",
		"wireless_rxsw", &di->gpio_rxsw, di->gpio_rxsw_valid_val))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "gpio_rxsw_aux_valid_val",
		(u32 *)&di->gpio_rxsw_aux_valid_val, 0))
		return 0;
	if (power_gpio_config_output(np, "gpio_rxsw_aux", "wireless_rxsw_aux",
		&di->gpio_rxsw_aux, !di->gpio_rxsw_aux_valid_val)) {
		gpio_free(di->gpio_rxsw);
		return -EINVAL;
	}

	return 0;
}

static int wlps_request_ext_pwr_sw_gpio(struct device_node *np, struct wlps_dev_info *di)
{
	di->gpio_ext_pwr_sw = of_get_named_gpio(np, "gpio_ext_pwr_sw", 0);
	hwlog_info("gpio_ext_pwr_sw=%d\n", di->gpio_ext_pwr_sw);
	if (di->gpio_ext_pwr_sw <= 0)
		return 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_ext_pwr_sw_valid_val", (u32 *)&di->gpio_ext_pwr_sw_valid_val, 1);
	if (power_gpio_config_output(np, "gpio_ext_pwr_sw", "wireless_ext_pwr_sw",
		&di->gpio_ext_pwr_sw, !di->gpio_ext_pwr_sw_valid_val))
		return -EINVAL;

	return 0;
}

static int wlps_request_sc_sw2_gpio(struct device_node *np, struct wlps_dev_info *di)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_sc_sw2_valid_val", (u32 *)&di->gpio_sc_sw2_valid_val, 0))
		return 0;

	if (power_gpio_config_output(np, "gpio_sc_sw2", "wireless_gpio_sc_sw2",
		&di->gpio_sc_sw2, !di->gpio_sc_sw2_valid_val)) {
		di->gpio_sc_sw2 = 0;
		return -EINVAL;
	}

	return 0;
}

static int wlps_request_resources(struct device_node *np, struct wlps_dev_info *di)
{
	if (wlps_request_tx_sw_gpio(np, di))
		goto tx_sw_gpio_fail;
	if (wlps_request_tx_sppwr_gpio(np, di))
		goto tx_sppwr_gpio_fail;
	if (wlps_request_rx_sw_gpio(np, di))
		goto rx_sw_gpio_fail;
	if (wlps_request_ext_pwr_sw_gpio(np, di))
		goto ext_pwr_sw_gpio_fail;
	if (wlps_request_sc_sw2_gpio(np, di))
		goto sc_sw2_gpio_fail;

	return 0;

sc_sw2_gpio_fail:
	wlps_gpio_free(di->gpio_ext_pwr_sw);
ext_pwr_sw_gpio_fail:
	wlps_gpio_free(di->gpio_rxsw);
	wlps_gpio_free(di->gpio_rxsw_aux);
rx_sw_gpio_fail:
	wlps_gpio_free(di->gpio_tx_sppwr_en);
tx_sppwr_gpio_fail:
	wlps_gpio_free(di->gpio_txsw);
tx_sw_gpio_fail:
	return -EINVAL;
}

static void wlps_parse_dts(struct device_node *np, struct wlps_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sysfs_en_pwr", (u32 *)&di->sysfs_en_pwr, WLPS_CHIP_PWR_NULL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"proc_otp_pwr", (u32 *)&di->proc_otp_pwr, WLPS_CHIP_PWR_NULL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"recover_otp_pwr", (u32 *)&di->recover_otp_pwr, WLPS_CHIP_PWR_TX);
}

static int wireless_ps_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct wlps_dev_info *di = NULL;
	const struct platform_device_id *id = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	id = platform_get_device_id(pdev);
	if (!id || !wltrx_ic_is_type_valid(id->driver_data))
		return -ENODEV;

	hwlog_info("[probe] dev_id=%d\n", id->driver_data);
	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	wlps_parse_dts(np, di);
	if (wlps_request_resources(np, di)) {
		kfree(di);
		return -EINVAL;
	}

	g_wlps_di[id->driver_data] = di;
	hwlog_info("device[%d] probe ok\n", id->driver_data);
	return 0;
}

static void wireless_ps_shutdown(struct platform_device *pdev)
{
	const struct platform_device_id *id = NULL;

	if (!pdev)
		return;

	id = platform_get_device_id(pdev);
	if (!id || !wltrx_ic_is_type_valid(id->driver_data))
		return;

	wlps_control(id->driver_data, WLPS_TX_PWR_SW, false);
	charge_pump_chip_enable(CP_TYPE_MAIN, false);
	wlps_control(id->driver_data, WLPS_RX_SW, false);
	kfree(g_wlps_di[id->driver_data]);
	g_wlps_di[id->driver_data] = NULL;
}

static const struct platform_device_id wlps_id[] = {
	{ "wireless_ps", WLTRX_IC_MAIN },
	{ "wireless_ps_aux", WLTRX_IC_AUX },
	{},
};

static struct platform_driver wlps_driver = {
	.probe = wireless_ps_probe,
	.shutdown = wireless_ps_shutdown,
	.id_table = wlps_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_psy",
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

fs_initcall_sync(wireless_ps_init);
module_exit(wireless_ps_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless power supply module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
