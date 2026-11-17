/*
 * wireless_rx_interfere.h
 *
 * common interface, variables, definition etc of wireless_rx_interfere.c
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

#ifndef _WIRELESS_RX_INTERFERE_H_
#define _WIRELESS_RX_INTERFERE_H_

#define NORMAL_FOP_MAX             150
#define VOUT_9V_STAGE_MAX          10500
#define VOUT_9V_STAGE_MIN          7500

#define WLRX_INTFR_DFT_FIXED_FOP   (-1) /* no need fix fop */

struct wlrx_intfr {
	u32 src_open;
	u32 src_close;
	int fixed_fop;
	int vtx;
	int vrx;
	int irx;
};

struct wlrx_pctrl;

#ifdef CONFIG_WIRELESS_CHARGER
u8 wlrx_get_intfr_src(void);
int wlrx_get_intfr_fixed_fop(void);
int wlrx_get_intfr_vtx(void);
int wlrx_get_intfr_vrx(void);
int wlrx_get_intfr_irx(void);
void wlrx_handle_intfr_settings(u8 src_state);
void wlrx_update_intfr_pctrl(struct wlrx_pctrl *pctrl);
void wlrx_clear_intfr_settings(void);
#else
static inline u8 wlrx_get_intfr_src(void)
{
	return 0;
}

static inline int wlrx_get_intfr_fixed_fop(void)
{
	return WLRX_INTFR_DFT_FIXED_FOP;
}

static inline int wlrx_get_intfr_vtx(void)
{
	return 0;
}

static inline int wlrx_get_intfr_vrx(void)
{
	return 0;
}

static inline int wlrx_get_intfr_imax(void)
{
	return 0;
}

static inline void wlrx_handle_intfr_settings(u8 src_states)
{
}

static inline void wlrx_update_intfr_pctrl(struct wlrx_pctrl *pctrl)
{
}

static inline void wlrx_clear_intfr_settings(void)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_INTERFERE_H_ */
