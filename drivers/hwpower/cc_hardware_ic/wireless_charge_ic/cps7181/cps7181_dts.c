/*
 * cps7181_dts.c
 *
 * cps7181 dts driver
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

#include "cps7181.h"

#define HWLOG_TAG wireless_cps7181_dts
HWLOG_REGIST();

static int cps7181_parse_fod(struct device_node *np,
	struct cps7181_dev_info *di)
{
	if (di->ic_type == WLTRX_IC_AUX)
		return 0;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_5v", di->rx_fod_5v, CPS7181_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_9v", di->rx_fod_9v, CPS7181_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_12v", di->rx_fod_12v, CPS7181_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_15v", di->rx_fod_15v, CPS7181_RX_FOD_LEN))
		return -EINVAL;

	return 0;
}

static int cps7181_parse_ldo_cfg(struct device_node *np,
	struct cps7181_dev_info *di)
{
	if (di->ic_type == WLTRX_IC_AUX)
		return 0;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_5v", di->rx_ldo_cfg_5v, CPS7181_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_9v", di->rx_ldo_cfg_9v, CPS7181_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_12v", di->rx_ldo_cfg_12v, CPS7181_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_sc", di->rx_ldo_cfg_sc, CPS7181_RX_LDO_CFG_LEN))
		return -EINVAL;

	return 0;
}

int cps7181_parse_dts(struct device_node *np, struct cps7181_dev_info *di)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth,
		CPS7181_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val,
		WLTRX_IC_EN_ENABLE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_interval", (u32 *)&di->tx_ping_interval,
		CPS7181_TX_PING_INTERVAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_max_fop", (u32 *)&di->tx_max_fop,
		CPS7181_TX_MAX_FOP);

	ret = cps7181_parse_fod(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse fod para failed\n");
		return ret;
	}
	ret = cps7181_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse ldo cfg failed\n");
		return ret;
	}

	return 0;
}