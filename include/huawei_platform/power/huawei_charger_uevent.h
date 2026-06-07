/*
 * huawei_charger_uevent.h
 *
 * charger uevent driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

extern struct blocking_notifier_head charger_event_notify_head;

void charge_event_notify(int event);
void charge_send_uevent(int input_events);
void direct_charge_connect_send_uevent(void);
void direct_charge_disconnect_send_uevent(void);
void wireless_charge_connect_send_uevent(void);

#endif /* _HUAWEI_CHARGER_UEVENT_H_ */
