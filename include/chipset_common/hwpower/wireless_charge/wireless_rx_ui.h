/*
 * wireless_rx_ui.h
 *
 * ui handler for wireless charge
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

#ifndef _WIRELESS_RX_UI_H_
#define _WIRELESS_RX_UI_H_

struct wlrx_max_pwr_revise {
	int unit_low;
	int unit_high;
	int remainder;
	int carry;
};

struct wlrx_ui_para {
	int ui_max_pwr;
	int product_max_pwr;
	int tx_max_pwr;
};

#ifdef CONFIG_WIRELESS_CHARGER
void wlrx_ui_send_max_pwr_evt(struct wlrx_ui_para *data);
void wlrx_ui_send_soc_decimal_evt(struct wlrx_ui_para *data);
#else
static inline void wlrx_ui_send_max_pwr_evt(struct wlrx_ui_para *data) {}
static inline void wlrx_ui_send_soc_decimal_evt(struct wlrx_ui_para *data) {}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_UI_H_ */
