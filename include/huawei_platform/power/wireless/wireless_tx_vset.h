/*
 * wireless_tx_vset.h
 *
 * tx vset header file for wireless reverse charging
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

#ifndef _WIRELESS_TX_VSET_H_
#define _WIRELESS_TX_VSET_H_

#define WLTX_TX_VSET_TYPE_MAX            3
#define WLTX_TX_VSET_SOC_TH1             20
#define WLTX_TX_VSET_SOC_TH2             25
#define WLTX_TX_VSET_TBAT_TH1            40
#define WLTX_TX_VSET_TBAT_TH2            45
#define WLTX_TX_VSET_FOP_TH1             115
#define WLTX_TX_VSET_FOP_TH2             130
#define WLTX_TX_VSET_IIN_TH1             400
#define WLTX_TX_VSET_IIN_TH2             600

/* only vset>=10v can tx be in half bridge mode */
#define WLTX_HALF_BRIDGE_RATIO           75 /* half: vout/vin < 75% */
#define WLTX_HALF_BRIDGE_VTH             10000 /* mV */
#define WLTX_FULL_BRIDGE_ITH             400 /* mA */
#define WLTX_HALF_BRIDGE_ITH             150 /* mA */

enum wltx_bridge_type {
	WLTX_BRIDGE_BEGIN = 0,
	WLTX_PING_FULL_BRIDGE = WLTX_BRIDGE_BEGIN,
	WLTX_PING_HALF_BRIDGE,
	WLTX_PT_FULL_BRIDGE,
	WLTX_PT_HALF_BRIDGE,
	WLTX_BRIDGE_END
};

enum wltx_stage_vset_info {
	WLTX_TX_STAGE_VPS = 0,
	WLTX_TX_STAGE_VPING,
	WLTX_TX_STAGE_VHS,
	WLTX_TX_STAGE_VDFLT,
	WLTX_TX_STAGE_VTOTAL
};

enum wltx_vset_info {
	WLTX_RX_VSET_MIN = 0,
	WLTX_RX_VSET_MAX,
	WLTX_TX_VSET,
	WLTX_TX_VSET_LTH,
	WLTX_TX_VSET_HTH,
	WLTX_TX_PLOSS_TH,
	WLTX_TX_PLOSS_CNT,
	WLTX_TX_EXT_HDL,
	WLTX_TX_VSET_TOTAL,
};

enum wltx_stage {
	WL_TX_STAGE_DEFAULT = 0,
	WL_TX_STAGE_POWER_SUPPLY,
	WL_TX_STAGE_CHIP_INIT,
	WL_TX_STAGE_PING_RX,
	WL_TX_STAGE_REGULATION,
	WL_TX_STAGE_TOTAL,
};

/* wireless tx extend handle */
enum wltx_ext_hdl {
	WLTX_EXT_HDL_DEFAULT = 0,
	WLTX_EXT_HDL_BP2CP
};

struct wltx_vset_data {
	u32 rx_vmin;
	u32 rx_vmax;
	int vset;
	u32 lth;
	u32 hth;
	u32 pl_th;
	u8 pl_cnt;
	u8 ext_hdl;
};

struct wltx_vset_para {
	int v_ps; /* power supply */
	int v_ping;
	int v_hs; /* handshake */
	int v_dflt;
	int cur;
	int total;
	int max_vset;
	int bridge;
	struct wltx_vset_data para[WLTX_TX_VSET_TYPE_MAX];
};

struct wltx_vout_para {
	int v_ask;
	int v_adap;
	int v_tx;
};

#ifdef CONFIG_WIRELESS_CHARGER
void wltx_set_tx_volt(enum wltx_stage stg, int vtx, bool force);
void wltx_reset_vset_para(void);
#else
static inline void wltx_set_tx_volt(enum wltx_stage stg, int vtx, bool force) {}
static inline void wltx_reset_vset_para(void) {}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_VSET_H_ */
