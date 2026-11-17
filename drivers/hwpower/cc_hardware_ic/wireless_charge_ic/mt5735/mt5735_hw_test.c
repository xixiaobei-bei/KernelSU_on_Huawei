/*
 * mt5735_hw_test.c
 *
 * mt5735 hardware test driver
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

#include "mt5735.h"

#define HWLOG_TAG wireless_mt5735_hw_test
HWLOG_REGIST();

static int mt5735_hw_test_pwr_good_gpio(void)
{
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, true);
	power_usleep(DT_USLEEP_10MS);
	if (!mt5735_is_pwr_good()) {
		hwlog_err("pwr_good_gpio: failed\n");
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, false);
		return -EINVAL;
	}
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, false);

	hwlog_info("[pwr_good_gpio] succ\n");
	return 0;
};

static int mt5735_hw_test_reverse_cp(int type)
{
	int ret;

	if (type == CHK_RVS_CP_PREV_PREPARE) {
		ret = wltx_ic_set_open_flag(WLTRX_IC_MAIN, true);
		ret += wltx_ic_activate_chip(WLTRX_IC_MAIN);
	} else {
		ret = wltx_ic_set_open_flag(WLTRX_IC_MAIN, false);
	}

	return ret;
}

static struct wireless_hw_test_ops g_mt5735_hw_test_ops = {
	.chk_pwr_good_gpio = mt5735_hw_test_pwr_good_gpio,
	.chk_reverse_cp_prepare = mt5735_hw_test_reverse_cp,
};

int mt5735_hw_test_ops_register(void)
{
	return wireless_hw_test_ops_register(&g_mt5735_hw_test_ops);
};
