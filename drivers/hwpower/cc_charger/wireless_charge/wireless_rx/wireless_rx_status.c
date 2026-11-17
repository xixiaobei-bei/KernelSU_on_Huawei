/*
 * wireless_rx_status.c
 *
 * charge status (stage, channel status etc.) for wireless charging
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
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>

#define HWLOG_TAG wireless_rx_status
HWLOG_REGIST();

static int g_wlrx_stage;
static int g_wldc_stage;
static int g_wired_channel_state;
static int g_wireless_channel_state;

void wlrx_set_charge_stage(int stage)
{
	static const char *const stage_str[] = {
		[WLRX_STAGE_DEFAULT]       = "DEFAULT",
		[WLRX_STAGE_HANDSHAKE]     = "HANDSHAKE",
		[WLRX_STAGE_GET_TX_CAP]    = "GET_TX_CAP",
		[WLRX_STAGE_CABLE_DET]     = "CABLE_DETECT",
		[WLRX_STAGE_AUTH]          = "AUTHENTICATION",
		[WLRX_STAGE_FW_UPDATE]     = "FW_UPDATE",
		[WLRX_STAGE_CHARGING]      = "CHARGING",
		[WLRX_STAGE_REGULATION]    = "REGULATION",
		[WLRX_STAGE_REGULATION_DC] = "REGULATION_DC",
	};

	if ((stage < WLRX_STAGE_BEGIN) || (stage >= WLRX_STAGE_END))
		return;

	g_wlrx_stage = stage;
	hwlog_info("[wireless_rx_stage] set to %s\n", stage_str[stage]);
}

int wlrx_get_charge_stage(void)
{
	return g_wlrx_stage;
}

void wldc_set_charge_stage(int stage)
{
	static const char *const stage_str[] = {
		[WLDC_STAGE_DEFAULT]       = "DEFAULT",
		[WLDC_STAGE_CHECK]         = "CHECK",
		[WLDC_STAGE_SUCCESS]       = "SUCCESS",
		[WLDC_STAGE_CHARGING]      = "CHARGING",
		[WLDC_STAGE_CHARGE_DONE]   = "CHARGE_DONE",
		[WLDC_STAGE_STOP_CHARGING] = "STOP_CHARGING",
	};

	if ((stage < WLDC_STAGE_BEGIN) || (stage >= WLDC_STAGE_END))
		return;

	g_wldc_stage = stage;
	hwlog_info("[wireless_dc_stage] set to %s\n", stage_str[stage]);
}

int wldc_get_charge_stage(void)
{
	return g_wldc_stage;
}

void wlrx_set_wired_channel_state(int state)
{
	g_wired_channel_state = state;
	hwlog_info("[set_wired_channel_state] %d\n", state);
}

int wlrx_get_wired_channel_state(void)
{
	return g_wired_channel_state;
}

void wlrx_set_wireless_channel_state(int state)
{
	g_wireless_channel_state = state;
	hwlog_info("[set_wireless_channel_state] %d\n", state);
}

int wlrx_get_wireless_channel_state(void)
{
	return g_wireless_channel_state;
}
