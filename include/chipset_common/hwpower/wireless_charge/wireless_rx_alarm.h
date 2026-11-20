/*
 * wireless_rx_alarm.h
 *
 * rx_alarm head file for wireless charging
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

#ifndef _WIRELESS_RX_ALARM_H_
#define _WIRELESS_RX_ALARM_H_

#include <chipset_common/hwpower/protocol/wireless_protocol.h>

#ifdef CONFIG_WIRELESS_CHARGER
void wlrx_reset_fsk_alarm(void);
void wlrx_handle_fsk_alarm(struct wireless_protocol_tx_alarm *tx_alarm);
int wlrx_get_alarm_plim(void);
int wlrx_get_alarm_vlim(void);
int wlrx_get_alarm_ilim(int vbase);
#else
static inline void wlrx_reset_fsk_alarm(void)
{
}

static inline void wlrx_handle_fsk_alarm(struct wireless_protocol_tx_alarm *tx_alarm)
{
}

static inline int wlrx_get_alarm_plim(void)
{
	return 0;
}

static inline int wlrx_get_alarm_vlim(void)
{
	return 0;
}

static inline int wlrx_get_alarm_ilim(int vbase)
{
	return 0;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_ALARM_H_ */
