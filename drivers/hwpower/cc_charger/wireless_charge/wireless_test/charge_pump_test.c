/*
 * charge_pump_test.c
 *
 * charge pump hardware test driver
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
#include <linux/kernel.h>
#include <linux/delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_test_hw.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>

#define HWLOG_TAG wireless_cp_hw_test
HWLOG_REGIST();

#define CHK_RVS_CP_RETRY_TIMES        3
#define CHK_RVS_CP_DELAY_FOR_BST      100
#define CHK_RVS_CP_VMIN               9000
#define CHK_RVS_CP_VMAX               11000

static int cp_hw_test_reserve_cp(void)
{
	int cnt;
	int ret;
	u16 tx_vin = 0;

	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, true);
	ret = charge_pump_reverse_cp_chip_init(CP_TYPE_MAIN);
	if (ret) {
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
		hwlog_err("test_reverse_cp: set cp failed\n");
		return ret;
	}
	hwlog_info("[test_reverse_cp] begin\n");
	msleep(CHK_RVS_CP_DELAY_FOR_BST);

	ret = wireless_hw_test_reverse_cp_prepare(CHK_RVS_CP_PREV_PREPARE);
	if (ret) {
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
		return ret;
	}
	wltx_ic_chip_enable(WLTRX_IC_MAIN, true);
	for (cnt = 0; cnt < CHK_RVS_CP_RETRY_TIMES; cnt++) {
		(void)wltx_ic_get_vin(WLTRX_IC_MAIN, &tx_vin);
		hwlog_info("[test_reverse_cp] vin=%u\n", tx_vin);
		if ((tx_vin > CHK_RVS_CP_VMIN) && (tx_vin < CHK_RVS_CP_VMAX)) {
			hwlog_info("[test_reverse_cp] succ\n");
			(void)wireless_hw_test_reverse_cp_prepare(CHK_RVS_CP_POST_PREPARE);
			wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
			return 0;
		}
		msleep(CHK_RVS_CP_DELAY_FOR_BST);
	}
	wireless_hw_test_reverse_cp_prepare(CHK_RVS_CP_POST_PREPARE);
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
	hwlog_err("test_reverse_cp: fail\n");
	return -1;
};

static struct wireless_hw_test_ops g_hw_cp_test_ops = {
	.chk_reverse_cp = cp_hw_test_reserve_cp,
};

static int __init cp_hw_test_init(void)
{
	return wireless_hw_test_ops_register(&g_hw_cp_test_ops);
}

fs_initcall(cp_hw_test_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless cp test module");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
