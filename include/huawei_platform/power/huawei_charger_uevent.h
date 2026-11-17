/*
 * huawei_charger_uevent.h
 *
 * charger uevent driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef _HUAWEI_CHARGER_UEVENT_H_
#define _HUAWEI_CHARGER_UEVENT_H_

#include <huawei_platform/power/huawei_charger.h>

#ifdef CONFIG_HUAWEI_CHARGER
void charge_send_uevent(int input_events);
void charge_send_icon_uevent(int icon_type);
void wired_connect_send_icon_uevent(int icon_type);
void wired_disconnect_send_icon_uevent(void);
void wireless_connect_send_icon_uevent(int icon_type);
#else
static inline void charge_send_uevent(int input_events) {}
static inline void charge_send_icon_uevent(int icon_type) {}
static inline void wired_connect_send_icon_uevent(int icon_type) {}
static inline void wired_disconnect_send_icon_uevent(void) {}
static inline void wireless_connect_send_icon_uevent(int icon_type) {}
#endif /* CONFIG_HUAWEI_CHARGER */

#endif /* _HUAWEI_CHARGER_UEVENT_H_ */
