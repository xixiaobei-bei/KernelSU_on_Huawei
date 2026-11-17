/*
 * cps4057_dts.c
 *
 * cps4057 dts driver
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

#include "cps4057.h"

#define HWLOG_TAG wireless_cps4057_dts
HWLOG_REGIST();

#define CPS4057_CAP_CFG_LEN                3

static void cps4057_parse_tx_fod(struct device_node *np,
	struct cps4057_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th0", (u32 *)&di->tx_fod.ploss_th0,
		CPS4057_TX_PLOSS_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th1", (u32 *)&di->tx_fod.ploss_th1,
		CPS4057_TX_PLOSS_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th2", (u32 *)&di->tx_fod.ploss_th2,
		CPS4057_TX_PLOSS_TH2_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th3", (u32 *)&di->tx_fod.ploss_th3,
		CPS4057_TX_PLOSS_TH3_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th0", (u32 *)&di->tx_fod.hp_ploss_th0,
		CPS4057_TX_HP_PLOSS_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th1", (u32 *)&di->tx_fod.hp_ploss_th1,
		CPS4057_TX_HP_PLOSS_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th2", (u32 *)&di->tx_fod.hp_ploss_th2,
		CPS4057_TX_HP_PLOSS_TH2_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_fod_cur_th0", (u32 *)&di->tx_fod.hp_cur_th0,
		CPS4057_TX_HP_FOD_CUR_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_fod_cur_th1", (u32 *)&di->tx_fod.hp_cur_th1,
		CPS4057_TX_HP_FOD_CUR_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_cnt", (u32 *)&di->tx_fod.ploss_cnt,
		CPS4057_TX_PLOSS_CNT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_q_en", (u32 *)&di->tx_fod.q_en,
		CPS4057_TX_FUNC_DIS);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_q_coil_th", (u32 *)&di->tx_fod.q_coil_th,
		CPS4057_TX_Q_ONLY_COIL_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_q_th", (u32 *)&di->tx_fod.q_th,
		CPS4057_TX_Q_TH);
}

static int cps4057_parse_ldo_cfg(struct device_node *np,
	struct cps4057_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_5v", di->rx_ldo_cfg_5v, CPS4057_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_9v", di->rx_ldo_cfg_9v, CPS4057_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_sc", di->rx_ldo_cfg_sc, CPS4057_RX_LDO_CFG_LEN))
		return -EINVAL;

	return 0;
}

static void cps4057_parse_rx_ask_mod_cfg(struct device_node *np,
	struct cps4057_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_volt_cap_cfg", (u8 *)&di->mod_cfg, CPS4057_CAP_CFG_LEN)) {
		di->mod_cfg.low_volt = CPS4057_RX_CMALL_EN_VAL;
		di->mod_cfg.high_volt = CPS4057_RX_CMBC_EN_VAL;
		di->mod_cfg.fac_high_volt = CPS4057_RX_CMC_EN_VAL;
	}
}

int cps4057_parse_dts(struct device_node *np, struct cps4057_dev_info *di)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth,
		CPS4057_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val,
		WLTRX_IC_EN_ENABLE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"full_bridge_ith", (u32 *)&di->full_bridge_ith,
		CPS4057_FULL_BRIDGE_ITH);

	cps4057_parse_tx_fod(np, di);

	ret = wlrx_parse_fod_cfg(di->ic_type, np, CPS4057_RX_FOD_LEN);
	if (ret) {
		hwlog_err("parse_dts: parse rx_fod para failed\n");
		return ret;
	}
	ret = cps4057_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse ldo cfg failed\n");
		return ret;
	}

	cps4057_parse_rx_ask_mod_cfg(np, di);

	return 0;
}
