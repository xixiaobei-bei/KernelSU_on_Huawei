/*
 * cps4029_dts.c
 *
 * cps4029 dts driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "cps4029.h"

#define HWLOG_TAG wireless_cps4029_dts
HWLOG_REGIST();

static void cps4029_parse_tx_fod(struct device_node *np,
	struct cps4029_dev_info *di)
{
	unsigned int val = 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th0", &val, CPS4029_TX_PLOSS_TH0_VAL);
	di->tx_fod.ploss_th0 = (u16)val;
	(void)power_dts_read_u8(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_cnt", &di->tx_fod.ploss_cnt, CPS4029_TX_PLOSS_CNT_VAL);
}

int cps4029_parse_dts(struct device_node *np, struct cps4029_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_max_fop", (u32 *)&di->tx_fop.tx_max_fop, CPS4029_TX_MAX_FOP);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_min_fop", (u32 *)&di->tx_fop.tx_min_fop, CPS4029_TX_MIN_FOP);

	cps4029_parse_tx_fod(np, di);

	return 0;
}
