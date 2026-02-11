/*
 * huawei_charger_uevent.c
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

#include <huawei_platform/power/huawei_charger_uevent.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/battery_soh.h>

#define HWLOG_TAG huawei_charger_uevent
HWLOG_REGIST();

BLOCKING_NOTIFIER_HEAD(charger_event_notify_head);

void charge_event_notify(int event)
{
	blocking_notifier_call_chain(&charger_event_notify_head, event, NULL);
}

void charge_send_uevent(int input_events)
{
	enum charge_status_event events;
	int charger_online = get_charger_online_flag();
	struct charge_device_info *di = huawei_charge_get_di();

	if (!di)
		return;

	if (input_events == NO_EVENT) {
		if (di->charger_source == POWER_SUPPLY_TYPE_MAINS) {
			events = VCHRG_START_AC_CHARGING_EVENT;
			charge_event_notify(CHARGER_START_CHARGING_EVENT);
		} else if (di->charger_source == POWER_SUPPLY_TYPE_USB) {
			events = VCHRG_START_USB_CHARGING_EVENT;
			charge_event_notify(CHARGER_START_CHARGING_EVENT);
		} else if (di->charger_source == POWER_SUPPLY_TYPE_BATTERY) {
			events = VCHRG_STOP_CHARGING_EVENT;
			charge_event_notify(CHARGER_STOP_CHARGING_EVENT);
			di->current_full_status = 0;
			charge_reset_hiz_state();
#ifdef CONFIG_DIRECT_CHARGER
			direct_charge_set_disable_flags(DC_CLEAR_DISABLE_FLAGS,
				DC_DISABLE_WIRELESS_TX);
#endif
		} else {
			events = NO_EVENT;
			hwlog_err("error charge source\n");
		}
#ifdef CONFIG_WIRELESS_CHARGER
		wireless_charge_wired_vbus_handler();
#endif
	} else {
		events = input_events;
	}

	/* avoid passing the wrong charging state */
	if (!pmic_vbus_irq_is_enabled() &&
		di->charger_type != CHARGER_TYPE_POGOPIN) {
		if ((charger_online &&
			events == VCHRG_STOP_CHARGING_EVENT) ||
			(!charger_online &&
			(events == VCHRG_START_AC_CHARGING_EVENT ||
			events == VCHRG_START_USB_CHARGING_EVENT))) {
			hwlog_err("status error, cc online = %d, events = %d\n",
				charger_online, events);
			events = NO_EVENT;
		}
	}

	/* valid events need to be sent to hisi_bci */
	if (events != NO_EVENT) {
		hisi_coul_charger_event_rcv(events);
		bsoh_event_rcv(events);
	}
}

void direct_charge_connect_send_uevent(void)
{
	struct charge_device_info *di = huawei_charge_get_di();

	if (!di)
		return;

	if (get_cancel_work_flag()) {
		hwlog_info("charge already stop\n");
		return;
	}

	if ((di->charger_type == CHARGER_REMOVED) ||
		(di->charger_source == POWER_SUPPLY_TYPE_BATTERY)) {
		hwlog_info("charge already plugged out\n");
		return;
	}

	di->charger_type = CHARGER_TYPE_STANDARD;
	di->charger_source = POWER_SUPPLY_TYPE_MAINS;
	charge_send_uevent(NO_EVENT);
}

void direct_charge_disconnect_send_uevent(void)
{
	struct charge_device_info *di = huawei_charge_get_di();

	if (!di)
		return;

	charge_send_uevent(NO_EVENT);
}

void wireless_charge_connect_send_uevent(void)
{
	struct charge_device_info *di = huawei_charge_get_di();

	if (!di)
		return;

	di->charger_type = CHARGER_TYPE_WIRELESS;
	di->charger_source = POWER_SUPPLY_TYPE_MAINS;
	charge_send_uevent(NO_EVENT);
}
