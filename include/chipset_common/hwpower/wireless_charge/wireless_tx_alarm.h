/*
 * wireless_tx_alarm.h
 *
 * tx alarm head file for wireless reverse charging
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

#ifndef _WIRELESS_TX_ALARM_H_
#define _WIRELESS_TX_ALARM_H_

#define TX_ALARM_SEND_RETRY_CNT   3

struct wltx_alarm {
	u32 src_state;
	u32 plim;
	u32 vlim;
	u32 reserved;
};

struct wltx_alarm_time_para {
	int time_th;
	struct wltx_alarm alarm;
};

struct wltx_alarm_tbatt_para {
	int tbatt_lth;
	int tbatt_hth;
	int tbatt_back;
	struct wltx_alarm alarm;
};

struct wltx_alarm_fod_para {
	int ploss_id;
	int ploss_lth;
	int ploss_hth;
	int delay;
	struct wltx_alarm alarm;
};

#ifdef CONFIG_WIRELESS_CHARGER
void wltx_reset_alarm_data(void);
void wltx_update_alarm_data(struct wltx_alarm *alarm, u8 src);
#else
static inline void wltx_reset_alarm_data(void)
{
}

static inline void wltx_update_alarm_data(struct wltx_alarm *alarm, u8 src)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_ALARM_H_ */
