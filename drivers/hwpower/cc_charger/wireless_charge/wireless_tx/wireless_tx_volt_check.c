/*
 * wireless_tx_volt_check.c
 *
 * tx voltage check for wireless reverse charging
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

#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>

#define HWLOG_TAG wireless_tx_vcheck
HWLOG_REGIST();

void wltx_ps_tx_high_vout_check(void)
{
	int i;
	int ret;
	u8 duty = 0;
	u16 tx_iin = 0;
	u16 fop = 0;
	static bool first_boost = true;
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di || di->high_vctrl.vctrl_level <= 0)
		return;

	if ((di->tx_vout.v_ask < WIRELESS_ADAPTER_15V) ||
		(wltx_get_cur_pwr_src() != PWR_SRC_VBUS_CP)) {
		first_boost = true;
		return;
	}

	ret = wltx_ic_get_iin(WLTRX_IC_MAIN, &tx_iin);
	ret += wltx_ic_get_fop(WLTRX_IC_MAIN, &fop);
	ret += wltx_ic_get_duty(WLTRX_IC_MAIN, &duty);
	if (ret)
		return;

	for (i = 0; i < di->high_vctrl.vctrl_level; i++) {
		if (di->high_vctrl.vctrl[i].cur_v <
			di->high_vctrl.vctrl[i].target_v) { /* boot */
			if (!first_boost &&
				(di->tx_vout.v_tx !=
				di->high_vctrl.vctrl[i].cur_v))
				continue;
			if (tx_iin <= di->high_vctrl.vctrl[i].iin_th)
				continue;
			if (fop >= di->high_vctrl.vctrl[i].fop_th)
				continue;
			if (duty < di->high_vctrl.vctrl[i].duty_th)
				continue;
			first_boost = false;
			break;
		} else if (di->high_vctrl.vctrl[i].cur_v >
			di->high_vctrl.vctrl[i].target_v) { /* reset */
			if (di->tx_vout.v_tx != di->high_vctrl.vctrl[i].cur_v)
				continue;
			if ((duty <= di->high_vctrl.vctrl[i].duty_th) ||
				((fop > di->high_vctrl.vctrl[i].fop_th) &&
				(tx_iin < di->high_vctrl.vctrl[i].iin_th)))
				break;
		}
	}

	if (i >= di->high_vctrl.vctrl_level)
		return;

	wltx_set_tx_volt(WL_TX_STAGE_REGULATION,
		di->high_vctrl.vctrl[i].target_v, false);
}
