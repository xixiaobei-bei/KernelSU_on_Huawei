/*
 * cps4057_hw_test.c
 *
 * cps4057 hardware test driver
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

#define HWLOG_TAG wireless_cps4057_hw_test
HWLOG_REGIST();

static int cps4057_hw_test_pwr_good_gpio(void)
{
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, true);
	power_usleep(DT_USLEEP_10MS);
	if (!cps4057_is_pwr_good()) {
		hwlog_err("pwr_good_gpio: failed\n");
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, false);
		return -EINVAL;
	}
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, false);

	hwlog_info("[pwr_good_gpio] succ\n");
	return 0;
};

static int cps4057_hw_test_reverse_cp(int type)
{
	return 0;
}

static struct wireless_hw_test_ops g_cps4057_hw_test_ops = {
	.chk_pwr_good_gpio = cps4057_hw_test_pwr_good_gpio,
	.chk_reverse_cp_prepare = cps4057_hw_test_reverse_cp,
};

int cps4057_hw_test_ops_register(void)
{
	return wireless_hw_test_ops_register(&g_cps4057_hw_test_ops);
};
