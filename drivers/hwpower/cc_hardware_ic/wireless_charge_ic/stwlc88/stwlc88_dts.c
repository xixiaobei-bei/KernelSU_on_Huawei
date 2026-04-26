/*
 * stwlc88_dts.c
 *
 * stwlc88 dts driver
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

#include "stwlc88.h"

#define HWLOG_TAG wireless_stwlc88_dts
HWLOG_REGIST();

#define STWLC88_CAP_CFG_LEN                3

static void stwlc88_parse_tx_fod(struct device_node *np,
	struct stwlc88_dev_info *di)
{
	(void)power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_para", di->tx_fod_para, ST88_TX_FOD_LEN);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_cnt", (u32 *)&di->tx_fod_cnt,
		ST88_TX_PLOSS_CNT_VAL);
}

static int stwlc88_parse_ldo_cfg(struct device_node *np,
	struct stwlc88_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_5v", di->rx_ldo_cfg_5v, ST88_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_9v", di->rx_ldo_cfg_9v, ST88_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_12v", di->rx_ldo_cfg_12v, ST88_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_sc", di->rx_ldo_cfg_sc, ST88_RX_LDO_CFG_LEN))
		return -EINVAL;

	return 0;
}

static void stwlc88_parse_rx_ask_mod_cfg(struct device_node *np,
	struct stwlc88_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_volt_cap_cfg", (u8 *)&di->mod_cfg, STWLC88_CAP_CFG_LEN)) {
		di->mod_cfg.low_volt = ST88_ALL_CAP_POSITIVE;
		di->mod_cfg.high_volt = ST88_CAP_BC_NEGATIVE;
		di->mod_cfg.fac_high_volt = ST88_CAP_BC_NEGATIVE;
	}
}

int stwlc88_parse_dts(struct device_node *np, struct stwlc88_dev_info *di)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth,
		ST88_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val,
		WLTRX_IC_EN_ENABLE);

	stwlc88_parse_tx_fod(np, di);

	ret = wlrx_parse_fod_cfg(di->ic_type, np, ST88_RX_FOD_LEN);
	if (ret) {
		hwlog_err("parse_dts: parse rx_fod para failed\n");
		return ret;
	}

	ret = stwlc88_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse ldo cfg failed\n");
		return ret;
	}

	stwlc88_parse_rx_ask_mod_cfg(np, di);

	return 0;
}
