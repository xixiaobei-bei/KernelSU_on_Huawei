/*
 * wireless_tx_cap.c
 *
 * tx capability for wireless reverse charging
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

#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_acc_types.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>

#define HWLOG_TAG wireless_tx_cap
HWLOG_REGIST();

u8 wltx_get_tx_cap_type(void)
{
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return -EPERM;

	return di->tx_cap.cap_para[di->tx_cap.exp_id].type;
}

void wltx_set_exp_cap_id(int exp_id)
{
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return;

	di->tx_cap.exp_id = exp_id;
	hwlog_info("[set_exp_cap_id] id=%d\n", di->tx_cap.exp_id);
}

static void wltx_cap_set_tx_type(u8 *tx_cap, u8 type)
{
	if ((type != TX_TYPE_OTG_A) && (type != TX_TYPE_OTG_B)) {
		tx_cap[WLC_TX_CAP_TYPE] = type;
	} else {
		if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON)
			tx_cap[WLC_TX_CAP_TYPE] = TX_TYPE_OTG_B;
		else
			tx_cap[WLC_TX_CAP_TYPE] = TX_TYPE_OTG_A;
	}

	hwlog_info("[tx_cap_type] type:0x%x\n", tx_cap[WLC_TX_CAP_TYPE]);
}

static void wltx_cap_set_tx_vmax(u8 *tx_cap, int vmax)
{
	/* 100mV in unit according to private qi protocol */
	tx_cap[WLC_TX_CAP_VMAX] = (u8)(vmax / 100);

	hwlog_info("[tx_cap_vmax] vmax:%dmV\n", vmax);
}

static void wltx_cap_set_tx_imax(u8 *tx_cap, int imax)
{
	enum wltx_pwr_type pwr_type;
	const char *src_name = NULL;

	pwr_type = wltx_get_pwr_type();
	src_name = wltx_get_pwr_src_name(wltx_get_cur_pwr_src());
	if ((pwr_type == WL_TX_PWR_5VBST_VBUS_OTG_CP) &&
		!strstr(src_name, "CP"))
		imax = 500; /* 500mA imax for 5vbst/vbus/otg pwr_src */

	/* 100mA in unit according to private qi protocol */
	tx_cap[WLC_TX_CAP_IMAX] = (u8)(imax / 100);

	hwlog_info("[tx_cap_imax] imax:%dmA\n", imax);
}

static void wltx_cap_set_tx_attr(u8 *tx_cap, u8 attr)
{
	tx_cap[WLC_TX_CAP_ATTR] = attr;

	hwlog_info("[tx_cap_attr] attr:0x%x\n", attr);
}

static void wltx_set_tx_cap(u8 *tx_cap, struct wltx_cap *cfg_cap)
{
	wltx_cap_set_tx_type(tx_cap, cfg_cap->type);
	wltx_cap_set_tx_vmax(tx_cap, cfg_cap->vout);
	wltx_cap_set_tx_imax(tx_cap, cfg_cap->iout);
	wltx_cap_set_tx_attr(tx_cap, cfg_cap->attr);
}

static void wltx_set_high_pwr_cap_id(struct wltx_dev_info *di)
{
	struct wltx_pwr_ctrl_info *info = wltx_get_pwr_ctrl_info();

	if (!info) {
		wltx_set_exp_cap_id(WLTX_DFLT_CAP);
		return;
	}

	if (info->charger_type == WLTX_SC_HI_PWR_CHARGER)
		wltx_set_exp_cap_id(WLTX_HIGH_PWR_CAP);
	else if (info->charger_type == WLTX_SC_HI_PWR2_CHARGER)
		wltx_set_exp_cap_id(WLTX_HIGH_PWR2_CAP);
	else
		wltx_set_exp_cap_id(WLTX_DFLT_CAP);
}

void wltx_send_tx_cap(void)
{
	int cap_id;
	struct wltx_dev_info *di = wltx_get_dev_info();
	u8 tx_cap[WLC_TX_CAP_TOTAL] = { TX_TYPE_OTG_A, 0 };

	if (!di)
		return;

	cap_id = di->tx_cap.exp_id;
	if ((cap_id < 0) || (cap_id >= WLTX_TOTAL_CAP))
		return;

	wltx_set_tx_cap(tx_cap, &di->tx_cap.cap_para[cap_id]);
	hwlog_info("[send_tx_cap] type=0x%x vmax=0x%x imax=0x%x attr=0x%x\n",
		tx_cap[WLC_TX_CAP_TYPE], tx_cap[WLC_TX_CAP_VMAX],
		tx_cap[WLC_TX_CAP_IMAX], tx_cap[WLC_TX_CAP_ATTR]);
	(void)wireless_send_tx_capability(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		tx_cap, WLC_TX_CAP_TOTAL);
	di->tx_cap.cur_id = di->tx_cap.exp_id;
}

static void wltx_check_high_pwr_cap(struct wltx_dev_info *di)
{
	int soc;

	soc = coul_drv_battery_capacity();
	if (soc < di->tx_high_pwr_soc) {
		wltx_set_exp_cap_id(WLTX_DFLT_CAP);
		return;
	}

	if (di->pwr_type != WL_TX_PWR_5VBST_VBUS_OTG_CP) {
		wltx_set_exp_cap_id(WLTX_HIGH_PWR_CAP);
		return;
	}

	wltx_set_high_pwr_cap_id(di);
}

void wltx_reset_exp_tx_cap_id(void)
{
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return;

	if (wltx_is_tx_open_by_hall()) {
		wltx_set_exp_cap_id(WLTX_DFLT_CAP);
		return;
	}

	if (di->tx_cap.cap_level <= 1) { /* only default cap, 5v */
		wltx_set_exp_cap_id(WLTX_DFLT_CAP);
		return;
	}

	wltx_check_high_pwr_cap(di);
}
