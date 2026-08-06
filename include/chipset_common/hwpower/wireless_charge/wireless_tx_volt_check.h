/*
 * wireless_tx_volt_check.h
 *
 * tx voltage check head file for wireless reverse charging
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

#ifndef _WIRELESS_TX_VOLT_CHECK_H_
#define _WIRELESS_TX_VOLT_CHECK_H_

#define WLTX_HIGH_V_PARA_ROW        6
#define WLTX_HIGH_V_PARA_COL        6

struct wltx_high_vctrl_para {
	int iin_th;
	int fop_th;
	int duty_th;
	int cur_v; /* mv */
	int target_v; /* mv */
	int delay; /* ms */
};

struct wltx_high_vctrl_data {
	int vctrl_level;
	struct wltx_high_vctrl_para vctrl[WLTX_HIGH_V_PARA_ROW];
};

#ifdef CONFIG_WIRELESS_CHARGER
void wltx_ps_tx_high_vout_check(void);
#else
static inline void wltx_ps_tx_high_vout_check(void) {}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_VOLT_CHECK_H_ */
