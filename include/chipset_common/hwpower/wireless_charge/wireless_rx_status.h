/*
 * wireless_rx_status.h
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

#ifndef _WIRELESS_RX_STATUS_H_
#define _WIRELESS_RX_STATUS_H_

#define WIRELESS_CHANNEL_ON         1
#define WIRELESS_CHANNEL_OFF        0
#define WIRED_CHANNEL_ON            1
#define WIRED_CHANNEL_OFF           0

enum wlrx_chrg_stage {
	WLRX_STAGE_BEGIN = 0,
	WLRX_STAGE_DEFAULT = WLRX_STAGE_BEGIN,
	WLRX_STAGE_HANDSHAKE,
	WLRX_STAGE_GET_TX_CAP,
	WLRX_STAGE_CABLE_DET,
	WLRX_STAGE_AUTH,
	WLRX_STAGE_FW_UPDATE,
	WLRX_STAGE_CHARGING,
	WLRX_STAGE_REGULATION,
	WLRX_STAGE_REGULATION_DC,
	WLRX_STAGE_END,
};

enum wldc_chrg_stage {
	WLDC_STAGE_BEGIN = 0,
	WLDC_STAGE_DEFAULT = WLDC_STAGE_BEGIN,
	WLDC_STAGE_CHECK,
	WLDC_STAGE_SUCCESS,
	WLDC_STAGE_CHARGING,
	WLDC_STAGE_CHARGE_DONE,
	WLDC_STAGE_STOP_CHARGING,
	WLDC_STAGE_END,
};

#ifdef CONFIG_WIRELESS_CHARGER
void wlrx_set_charge_stage(int stage);
int wlrx_get_charge_stage(void);
void wldc_set_charge_stage(int stage);
int wldc_get_charge_stage(void);
void wlrx_set_wired_channel_state(int state);
int wlrx_get_wired_channel_state(void);
void wlrx_set_wireless_channel_state(int state);
int wlrx_get_wireless_channel_state(void);
#else
static inline void wlrx_set_charge_stage(int stage)
{
}

static inline int wlrx_get_charge_stage(void)
{
	return WLRX_STAGE_DEFAULT;
}

static inline void wldc_set_charge_stage(int stage)
{
}

static inline int wldc_get_charge_stage(void)
{
	return WLDC_STAGE_DEFAULT;
}

static inline void wlrx_set_wired_channel_state(int state)
{
}

static inline int wlrx_get_wired_channel_state(void)
{
	return WIRED_CHANNEL_OFF;
}

static inline void wlrx_set_wireless_channel_state(int state)
{
}

static inline int wlrx_get_wireless_channel_state(void)
{
	return WIRELESS_CHANNEL_OFF;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_STATUS_H_ */
