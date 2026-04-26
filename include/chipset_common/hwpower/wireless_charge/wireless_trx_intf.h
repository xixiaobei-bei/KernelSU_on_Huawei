/*
 * wireless_trx_intf.h
 *
 * common interface, variables, definition etc for wireless_trx_intf
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_TRX_INTF_H_
#define _WIRELESS_TRX_INTF_H_

#define WIRELESS_RX                   0
#define WIRELESS_TX                   1

enum wireless_drv_type {
	WLTRX_DRV_MIN = 0,
	WLTRX_DRV_MAIN = WLTRX_DRV_MIN,
	WLTRX_DRV_AUX,
	WLTRX_DRV_MAX,
};

static inline bool wltrx_is_drv_type_valid(unsigned int drv_type)
{
	return (drv_type >= WLTRX_DRV_MIN) && (drv_type < WLTRX_DRV_MAX);
}

#endif /* _WIRELESS_TRX_INTF_H_ */
