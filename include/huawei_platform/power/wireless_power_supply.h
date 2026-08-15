/*
 * wireless_power_supply.h
 *
 * driver for wireless power supply
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

#ifndef _WIRELESS_POWER_SUPPLY_H_
#define _WIRELESS_POWER_SUPPLY_H_

#define WLPS_CTRL_ON         1
#define WLPS_CTRL_OFF        0

enum wlps_ctrl_scene {
	WLPS_PROBE_PWR = 0,
	WLPS_SYSFS_EN_PWR,
	WLPS_PROC_OTP_PWR,
	WLPS_RX_EXT_PWR,
	WLPS_TX_PWR_SW,
	WLPS_RX_SW,
	WLPS_TX_SW,
	WLPS_MAX,
};

enum wlps_chip_pwr_type {
	WLPS_CHIP_PWR_NULL = 0,
	WLPS_CHIP_PWR_RX,
	WLPS_CHIP_PWR_TX,
};

struct wlps_dev_info {
	struct device *dev;
	int gpio_tx_sppwr_en;
	int gpio_tx_sppwr_en_valid_val;
	int gpio_txsw;
	int gpio_txsw_valid_val;
	int gpio_rxsw;
	int gpio_rxsw_valid_val;
	int gpio_ext_pwr_sw;
	int gpio_ext_pwr_sw_valid_val;
	enum wlps_chip_pwr_type sysfs_en_pwr;
	enum wlps_chip_pwr_type proc_otp_pwr;
};

void wlps_control(enum wlps_ctrl_scene scene, int ctrl_flag);

#endif  /* _WIRELESS_POWER_SUPPLY_H_ */
