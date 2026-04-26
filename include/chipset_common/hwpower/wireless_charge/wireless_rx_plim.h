/*
 * wireless_rx_plim.h
 *
 * power limit for wireless charging
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

#ifndef _WIRELESS_RX_PLIM_H_
#define _WIRELESS_RX_PLIM_H_

enum wlrx_plim_src {
	WLRX_PLIM_SRC_BEGIN = 0,
	WLRX_PLIM_SRC_OTG = WLRX_PLIM_SRC_BEGIN,
	WLRX_PLIM_SRC_RPP,
	WLRX_PLIM_SRC_FAN,
	WLRX_PLIM_SRC_VOUT_ERR,
	WLRX_PLIM_SRC_TX_ALARM,
	WLRX_PLIM_SRC_TX_BST_ERR,
	WLRX_PLIM_SRC_KB,
	WLRX_PLIM_SRC_THERMAL,
	WLRX_PLIM_SRC_FIX_FOP,
	WLRX_PLIM_SRC_END,
};

struct wlrx_plim_para {
	int src_id;
	const char *src_name;
	bool need_rst; /* reset para when recharging */
	int vtx; /* mV */
	int vrx; /* mV */
	int irx; /* mA */
};

struct wlrx_plim {
	unsigned long src;
};

struct wlrx_pctrl;

#ifdef CONFIG_WIRELESS_CHARGER
int wlrx_get_plim_src(void);
void wlrx_set_plim_src(int src_id);
void wlrx_clear_plim_src(int src_id);
void wlrx_reset_plim_para(void);
void wlrx_update_plim_pctrl(struct wlrx_pctrl *pctrl);
#else
static inline int wlrx_get_plim_src(void)
{
	return 0;
}

static inline void wlrx_set_plim_src(int src_id)
{
}

static inline void wlrx_clear_plim_src(int src_id)
{
}

static inline void wlrx_reset_plim_para(void)
{
}

static inline void wlrx_update_plim_pctrl(struct wlrx_pctrl *pctrl)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_PLIM_H_ */
