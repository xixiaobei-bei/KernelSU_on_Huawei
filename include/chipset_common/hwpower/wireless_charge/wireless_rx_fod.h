/*
 * wireless_rx_fod.h
 *
 * rx fod head file for wireless charging
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_RX_FOD_H_
#define _WIRELESS_RX_FOD_H_

#include <linux/errno.h>

struct device_node;

#ifdef CONFIG_WIRELESS_CHARGER
int wlrx_fod_init(unsigned int drv_type, const struct device_node *np);
void wlrx_fod_deinit(unsigned int drv_type);
void wlrx_send_fod_status(unsigned int drv_type, unsigned int prot_type);
#else
static inline int wlrx_fod_init(unsigned int drv_type, const struct device_node *np)
{
	return -ENOTSUPP;
}

static inline void wlrx_fod_deinit(unsigned int drv_type)
{
}

static inline void wlrx_send_fod_status(unsigned int drv_type, unsigned int prot_type)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_FOD_H_ */
