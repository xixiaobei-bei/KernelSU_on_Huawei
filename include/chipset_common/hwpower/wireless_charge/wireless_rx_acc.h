/*
 * wireless_rx_acc.h
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

#ifndef _WIRELESS_RX_ACC_H_
#define _WIRELESS_RX_ACC_H_

#define WLRX_ACC_TX_PWR_RATIO              75

struct tx_dft_prop {
	int type;
	const char *name;
	int vout;
	int iout;
};

enum wlrx_acc_det_status {
	WLRX_ACC_DET_DEFAULT,
	WLRX_ACC_DET_ADAPTER_OR_CABLE_MISMATCH,
	WLRX_ACC_DET_ADAPTER_MISMATCH,
	WLRX_ACC_DET_CABLE_MISMATCH,
	WLRX_ACC_DET_ADAPTER_AND_CABLE_MISMATCH,
};

struct wlrx_acc_det_para {
	int acc_det_pwr;
	int tx_pwr;
	int standard_tx;
};

#ifdef CONFIG_WIRELESS_CHARGER
struct tx_dft_prop *wlrx_get_dft_tx_prop(int type);
int wlrx_redef_tx_type(int type);
bool wlrx_is_err_tx(int type);
bool wlrx_is_fac_tx(int type);
bool wlrx_is_car_tx(int type);
bool wlrx_is_rvs_tx(int type);
void wlrx_acc_det(struct wlrx_acc_det_para *acc);
#else
static inline struct tx_dft_prop *wlrx_get_dft_tx_prop(int type)
{
	return NULL;
}

static inline int wlrx_redef_tx_type(int type)
{
	return 0;
}

static inline bool wlrx_is_err_tx(int type)
{
	return true;
}

static inline bool wlrx_is_fac_tx(int type)
{
	return false;
}

static inline bool wlrx_is_car_tx(int type)
{
	return false;
}

static inline bool wlrx_is_rvs_tx(int type)
{
	return false;
}

static inline void wlrx_acc_det(struct wlrx_acc_det_para *acc)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_ACC_ */
