/*
 * wireless_tx_pwr_ctrl.h
 *
 * power control module for wireless reverse charging
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

#ifndef _WIRELESS_TX_PWR_CTRL_H_
#define _WIRELESS_TX_PWR_CTRL_H_

#include <linux/types.h>
#include <linux/workqueue.h>
#include <huawei_platform/power/wireless/wireless_tx_pwr_src.h>

#define WLTX_UNKOWN_CHARGER        0
#define WLTX_SC_LOW_PWR_CHARGER    1
#define WLTX_SC_HI_PWR_CHARGER     2
#define WLTX_SC_HI_PWR2_CHARGER    3
#define WLTX_HIGH_PWR_SC_MIN_IOUT  4000 /* mA */
#define WLTX_MON_DC_ADP_INTERVAL   5000 /* 5s */

#define WLTX_DC_DONE_BUCK_ILIM     2000

/* Notes that new type should be appended to the last */
enum wltx_pwr_type {
	WL_TX_PWR_BEGIN = 0x0,
	WL_TX_PWR_VBUS_OTG = WL_TX_PWR_BEGIN,
	WL_TX_PWR_5VBST_OTG,
	WL_TX_PWR_SPBST, /* Specialized boost */
	WL_TX_PWR_5VBST,
	WL_TX_PWR_5VBST_VBUS_OTG_CP,
	WL_TX_PWR_END,
};

enum wltx_pwr_sw_scene {
	PWR_SW_SCN_BEGIN = 0x0,
	PWR_SW_BY_VBUS_ON = PWR_SW_SCN_BEGIN,
	PWR_SW_BY_VBUS_OFF,
	PWR_SW_BY_OTG_ON,
	PWR_SW_BY_OTG_OFF,
	PWR_SW_BY_DC_DONE,
	PWR_SW_SCN_END,
};

struct wltx_pwr_attr {
	enum wltx_pwr_sw_scene scn;
	bool need_sw;
	enum wltx_pwr_src src;
};

struct wltx_pwr_ctrl_info {
	struct delayed_work mon_dc_work;
	int charger_type;
	bool dc_done;
};

#ifdef CONFIG_WIRELESS_CHARGER
struct wltx_pwr_ctrl_info *wltx_get_pwr_ctrl_info(void);
const char *wltx_get_pwr_type_name(enum wltx_pwr_type type);
bool wltx_need_disable_wired_dc(void);
int wltx_enable_pwr_supply(void);
void wltx_disable_pwr_supply(void);
void wltx_enable_extra_pwr_supply(void);
void wltx_disable_all_pwr(void);
enum wltx_pwr_type wltx_get_pwr_type(void);
void wireless_tx_cancel_work(enum wltx_pwr_sw_scene scn);
void wireless_tx_restart_check(enum wltx_pwr_sw_scene scn);
void wltx_dc_adaptor_handler(void);
#else
static inline struct wltx_pwr_ctrl_info *wltx_get_pwr_ctrl_info(void)
{
	return NULL;
}

static inline const char *wltx_get_pwr_type_name(enum wltx_pwr_type type)
{
	return "NA";
}

static inline bool wltx_need_disable_wired_dc(void)
{
	return false;
}

static inline int wltx_enable_pwr_supply(void)
{
	return -1;
}

static inline enum wltx_pwr_type wltx_get_pwr_type(void)
{
	return WL_TX_PWR_END;
}

static inline void wltx_disable_pwr_supply(void) {}
static inline void wltx_enable_extra_pwr_supply(void) {}
static inline void wltx_disable_all_pwr(void) {}
static inline void wireless_tx_cancel_work(enum wltx_pwr_sw_scene scn) {}
static inline void wireless_tx_restart_check(enum wltx_pwr_sw_scene scn) {}
static inline void wltx_dc_adaptor_handler(void) {}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_PWR_CTRL_H_ */
