/*
 * wireless_rx_pctrl.h
 *
 * power control for wireless charging
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

#ifndef _WIRELESS_RX_PCTRL_H_
#define _WIRELESS_RX_PCTRL_H_

struct wlrx_pctrl {
	int vtx;
	int vrx;
	int irx;
};

struct wlrx_soc_para {
	int soc_min;
	int soc_max;
	int vtx;
	int vrx;
	int irx;
};

#ifdef CONFIG_WIRELESS_CHARGER
void wlrx_update_pctrl(struct wlrx_pctrl *pctrl);
#else
static inline void wlrx_update_pctrl(struct wlrx_pctrl *pctrl)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_PCTRL_H_ */
