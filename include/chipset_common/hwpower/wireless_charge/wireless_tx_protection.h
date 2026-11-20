/*
 * wireless_tx_protection.h
 *
 * tx protection head file for wireless reverse charging
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

#ifndef _WIRELESS_TX_PROTECTION_H_
#define _WIRELESS_TX_PROTECTION_H_

#include <chipset_common/hwpower/wireless_charge/wireless_tx_alarm.h>

#define WLTX_PROTECT_MON_DELAY       15000 /* ms */
#define WLTX_PROTECT_MON_INTERVAL    1000 /* ms */

#define WLTX_TBATT_ALARM_ROW         3
#define WLTX_TBATT_ALARM_COL         7

struct wltx_protect_cfg {
	int tbatt_alarm_level;
	struct wltx_alarm_tbatt_para tbatt_alarm[WLTX_TBATT_ALARM_ROW];
};

struct wltx_protect_dev_info {
	bool need_monitor;
	bool first_monitor;
	struct delayed_work mon_work;
	struct wltx_protect_cfg cfg;
};

#ifdef CONFIG_WIRELESS_CHARGER
void wltx_start_monitor_protection(struct wltx_protect_cfg *cfg,
	const unsigned int delay);
void wltx_stop_monitor_protection(void);
#else
static inline void wltx_start_monitor_protection(
	struct wltx_protect_cfg *cfg, const unsigned int delay)
{
}

static inline void wltx_stop_monitor_protection(void)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_PROTECTION_H_ */
