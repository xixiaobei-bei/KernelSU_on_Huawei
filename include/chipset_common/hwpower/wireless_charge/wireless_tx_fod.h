/*
 * wireless_tx_fod.h
 *
 * tx fod head file for wireless reverse charging
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

#ifndef _WIRELESS_TX_FOD_H_
#define _WIRELESS_TX_FOD_H_

#include <chipset_common/hwpower/wireless_charge/wireless_tx_alarm.h>

#define WLTX_FOD_MON_DELAY       15000 /* ms */
#define WLTX_FOD_MON_INTERVAL    500 /* ms */
#define WLTX_FOD_MON_DEBOUNCE    2000 /* ms */

#define WLTX_FOD_ALARM_ROW       3
#define WLTX_FOD_ALARM_COL       8

struct wltx_fod_cfg {
	int fod_alarm_level;
	struct wltx_alarm_fod_para fod_alarm[WLTX_FOD_ALARM_ROW];
};

struct wltx_fod_dev_info {
	int mon_delay;
	bool need_monitor;
	int alarm_id;
	struct delayed_work mon_work;
	struct wltx_fod_cfg cfg;
};

#ifdef CONFIG_WIRELESS_CHARGER
void wltx_start_monitor_fod(struct wltx_fod_cfg *cfg, const unsigned int delay);
void wltx_stop_monitor_fod(void);
#else
static inline void wltx_start_monitor_fod(struct wltx_fod_cfg *cfg,
	const unsigned int delay)
{
}

static inline void wltx_stop_monitor_fod(void)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_CHRG_CURVE_H_ */
