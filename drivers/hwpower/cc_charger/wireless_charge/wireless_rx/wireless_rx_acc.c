/*
 * wireless_rx_acc.c
 *
 * accessory(tx,cable,adapter etc.) for wireless charging
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

#include <linux/kernel.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_acc_types.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_ui_ne.h>

#define HWLOG_TAG wireless_rx_acc
HWLOG_REGIST();

static struct tx_dft_prop g_dtf_tx_prop[] = {
	{ TX_TYPE_SDP,     "SDP",      5000, 475  },
	{ TX_TYPE_CDP,     "CDP",      5000, 1000 },
	{ TX_TYPE_NON_STD, "NON-STD",  5000, 1000 },
	{ TX_TYPE_DCP,     "DCP",      5000, 1000 },
	{ TX_TYPE_FCP,     "FCP",      9000, 2000 },
	{ TX_TYPE_SCP,     "SCP",      5000, 1000 },
	{ TX_TYPE_PD,      "PD",       5000, 1000 },
	{ TX_TYPE_QC,      "QC",       5000, 1000 },
	{ TX_TYPE_OTG_A,   "OTG_A",    5000, 475  },
	{ TX_TYPE_OTG_B,   "OTG_B",    5000, 475  },
	{ TX_TYPE_ERR,     "ERR",      5000, 1000 },
};

int wlrx_redef_tx_type(int type)
{
	if ((type >= TX_TYPE_FAC_BASE) && (type <= TX_TYPE_FAC_MAX))
		type %= TX_TYPE_FAC_BASE;
	else if ((type >= TX_TYPE_CAR_BASE) && (type <= TX_TYPE_CAR_MAX))
		type %= TX_TYPE_CAR_BASE;
	else if ((type >= TX_TYPE_PWR_BANK_BASE) && (type <= TX_TYPE_PWR_BANK_MAX))
		type %= TX_TYPE_PWR_BANK_BASE;

	return type;
}

struct tx_dft_prop *wlrx_get_dft_tx_prop(int type)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_dtf_tx_prop); i++) {
		if (type == g_dtf_tx_prop[i].type)
			return &g_dtf_tx_prop[i];
	}

	return NULL;
}

bool wlrx_is_err_tx(int type)
{
	return type == TX_TYPE_ERR;
}

bool wlrx_is_fac_tx(int type)
{
	return (type >= TX_TYPE_FAC_BASE) && (type <= TX_TYPE_FAC_MAX);
}

bool wlrx_is_car_tx(int type)
{
	return (type >= TX_TYPE_CAR_BASE) && (type <= TX_TYPE_CAR_MAX);
}

bool wlrx_is_rvs_tx(int type)
{
	return (type == TX_TYPE_OTG_A) || (type == TX_TYPE_OTG_B);
}

static void wlrx_acc_det_notify(int min_acc_pwr)
{
	int adapter_vout = 0;
	int adapter_iout = 0;
	int cable_type = 0;
	int cable_iout = 0;
	int acc_det_status;
	int ret;

	ret = wireless_get_tx_adapter_capability(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		&adapter_vout, &adapter_iout);
	ret += wireless_get_tx_cable_type(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		&cable_type, &cable_iout);
	if (ret)
		acc_det_status = WLRX_ACC_DET_ADAPTER_OR_CABLE_MISMATCH;
	else if (adapter_vout * adapter_iout / MVOLT_PER_VOLT < min_acc_pwr)
		acc_det_status = WLRX_ACC_DET_ADAPTER_MISMATCH;
	else if ((adapter_vout > 0) && (cable_iout < min_acc_pwr * MVOLT_PER_VOLT / adapter_vout))
		acc_det_status = WLRX_ACC_DET_CABLE_MISMATCH;
	else
		return;

	power_ui_event_notify(POWER_UI_NE_WL_ACC_STATUS, &acc_det_status);
}

void wlrx_acc_det(struct wlrx_acc_det_para *acc)
{
	int min_acc_pwr;
	int tx_max_pwr = 0;

	hwlog_info("acc_det_pwr=%d, tx_pwr=%d, standard_tx=%d\n", acc->acc_det_pwr,
		acc->tx_pwr, acc->standard_tx);

	if ((acc->acc_det_pwr <= 0) || (acc->tx_pwr <= 0)) {
		hwlog_err("invalid param\n");
		return;
	}

	min_acc_pwr = acc->acc_det_pwr * POWER_PERCENT / WLRX_ACC_TX_PWR_RATIO;
	if ((acc->standard_tx == 0) || (acc->tx_pwr >= min_acc_pwr))
		return;

	if (wireless_get_tx_max_power(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, &tx_max_pwr))
		return;
	if (tx_max_pwr < acc->acc_det_pwr)
		return;

	wlrx_acc_det_notify(min_acc_pwr);
}
