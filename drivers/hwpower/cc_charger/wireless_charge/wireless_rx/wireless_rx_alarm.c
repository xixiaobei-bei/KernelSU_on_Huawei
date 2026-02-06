/*
 * wireless_rx_alarm.c
 *
 * ask alarm for wireless charging
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

#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_alarm.h>

#define HWLOG_TAG wireless_rx_alarm
HWLOG_REGIST();

static struct {
	u8 src;
	u32 plim;
	u32 vlim;
} g_wlrx_alarm;

int wlrx_get_alarm_plim(void)
{
	return g_wlrx_alarm.plim;
}

int wlrx_get_alarm_vlim(void)
{
	return g_wlrx_alarm.vlim;
}

int wlrx_get_alarm_ilim(int vbase)
{
	if (vbase <= 0)
		return 0;

	return g_wlrx_alarm.plim * MVOLT_PER_VOLT / vbase;
}

void wlrx_reset_fsk_alarm(void)
{
	g_wlrx_alarm.src = 0;
	g_wlrx_alarm.plim = 0;
	g_wlrx_alarm.vlim = 0;
}

void wlrx_handle_fsk_alarm(struct wireless_protocol_tx_alarm *tx_alarm)
{
	if (!tx_alarm)
		return;

	g_wlrx_alarm.src = tx_alarm->src;
	g_wlrx_alarm.plim = tx_alarm->plim;
	g_wlrx_alarm.vlim = tx_alarm->vlim;
	hwlog_info("[ask_alarm] src=0x%x, plim=%umW, vlim=%umV\n",
		g_wlrx_alarm.src, g_wlrx_alarm.plim, g_wlrx_alarm.vlim);
}
