/*
 * wireless_power_supply.h
 *
 * common interface, variables, definition etc for wireless power supply
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

#ifndef _WIRELESS_POWER_SUPPLY_H_
#define _WIRELESS_POWER_SUPPLY_H_

enum wlps_ctrl_scene {
	WLPS_MIN = 0,
	WLPS_SYSFS_EN_PWR,
	WLPS_PROC_OTP_PWR,
	WLPS_RECOVER_OTP_PWR,
	WLPS_RX_BOOST_5V,
	WLPS_TX_BOOST_5V,
	WLPS_RX_EXT_PWR,
	WLPS_TX_PWR_SW,
	WLPS_RX_SW,
	WLPS_RX_SW_AUX,
	WLPS_TX_SW,
	WLPS_SC_SW2,
	WLPS_MAX,
};

enum wlps_chip_pwr_type {
	WLPS_CHIP_PWR_NULL = 0,
	WLPS_CHIP_PWR_RX,
	WLPS_CHIP_PWR_TX,
};

#ifdef CONFIG_WIRELESS_CHARGER
void wlps_control(unsigned int ic_type, int scene, bool flag);
#else
static void wlps_control(unsigned int ic_type, int scene, bool flag)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_POWER_SUPPLY_H_ */
