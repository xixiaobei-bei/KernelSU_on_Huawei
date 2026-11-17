/*
 * wireless_rx_dts.h
 *
 * parse dts for wireless charging
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_RX_DTS_H_
#define _WIRELESS_RX_DTS_H_

struct wlrx_pctrl;
struct wlrx_soc_para;
struct wlrx_intfr;
struct wlrx_pmode;
struct wlrx_vmode;

struct wlrx_dts {
	int antifake_kid;
	int ui_pmax_lth;
	int product_pmax_hth;
	int accdet_pmax_lth;
	int hvc_need_5vbst;
	int bst5v_ignore_vbus_only;
	int rx_vout_err_ratio;
	int rx_imin;
	int rx_istep;
	int pmax;
	int ignore_qval;
	u32 sample_delay_time;
	int support_high_pwr_wltx;
	int wired_sw_dflt_on;
	struct wlrx_pctrl *product_cfg;
	int soc_cfg_level;
	struct wlrx_soc_para *soc_cfg;
	int intrf_src_level;
	struct wlrx_intfr *intfr_cfg;
	int pmode_cfg_level;
	struct wlrx_pmode *pmode_cfg;
	int vmode_cfg_level;
	struct wlrx_vmode *vmode_cfg;
};

struct device_node;

#ifdef CONFIG_WIRELESS_CHARGER
struct wlrx_dts *wlrx_get_dts(void);
int wlrx_parse_dts(const struct device_node *np);
void wlrx_kfree_dts(void);
#else
static inline struct wlrx_dts *wlrx_get_dts(void)
{
	return NULL;
}

static inline int wlrx_parse_dts(const struct device_node *np)
{
	return -EINVAL;
}

static inline void wlrx_kfree_dts(void)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_DTS_H_ */
