/*
 * uscp.h
 *
 * usb port short circuit protect monitor driver
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

#ifndef _USCP_H_
#define _USCP_H_

#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
bool uscp_is_in_protect_mode(void);
bool uscp_is_in_rt_protect_mode(void);
bool uscp_is_in_hiz_mode(void);
#else
static inline bool uscp_is_in_protect_mode(void)
{
	return false;
}

static inline bool uscp_is_in_rt_protect_mode(void)
{
	return false;
}

static inline bool uscp_is_in_hiz_mode(void)
{
	return false;
}
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */

#endif /* _USCP_H_ */
