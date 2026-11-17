/*
 * wireless_tx_chrg_curve.h
 *
 * tx charging curve head file for wireless reverse charging
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

#ifndef _WIRELESS_TX_CHRG_CURVE_H_
#define _WIRELESS_TX_CHRG_CURVE_H_

#include <chipset_common/hwpower/wireless_charge/wireless_tx_alarm.h>

#define WLTX_CC_MON_DELAY       15000 /* ms */
#define WLTX_CC_MON_INTERVAL    10000 /* ms */

#define WLTX_TIME_ALARM_ROW       3
#define WLTX_TIME_ALARM_COL       5

struct wltx_cc_cfg {
	int time_alarm_level;
	struct wltx_alarm_time_para time_alarm[WLTX_TIME_ALARM_ROW];
};

struct wltx_cc_dev_info {
	bool need_monitor;
	unsigned int start_time;
	struct delayed_work mon_work;
	struct wltx_cc_cfg cfg;
};

#ifdef CONFIG_WIRELESS_CHARGER
void wltx_start_monitor_chrg_curve(struct wltx_cc_cfg *cfg,
	const unsigned int delay);
void wltx_stop_monitor_chrg_curve(void);
#else
static inline void wltx_start_monitor_chrg_curve(
	struct wltx_cc_cfg *cfg, const unsigned int delay)
{
}

static inline void wltx_stop_monitor_chrg_curve(void)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_CHRG_CURVE_H_ */
