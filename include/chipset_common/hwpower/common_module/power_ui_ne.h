/*
 * power_ui_ne.h
 *
 * notifier event for ui display
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

#ifndef _POWER_UI_NE_H_
#define _POWER_UI_NE_H_

enum power_ui_ne_list {
	POWER_UI_NE_BEGIN = 0,
	POWER_UI_NE_CABLE_TYPE = POWER_UI_NE_BEGIN,
	POWER_UI_NE_MAX_POWER, /* maximum power of adapter */
	POWER_UI_NE_WL_OFF_POS, /* off position of wireless rx */
	POWER_UI_NE_WL_FAN_STATUS,
	POWER_UI_NE_WL_COVER_STATUS,
	POWER_UI_NE_WL_ACC_STATUS,
	POWER_UI_NE_WATER_STATUS,
	POWER_UI_NE_HEATING_STATUS,
	POWER_UI_NE_ICON_TYPE,
	POWER_UI_NE_DEFAULT,
	POWER_UI_NE_END,
};

enum power_ui_icon_type {
	ICON_TYPE_BEGIN = 0,
	ICON_TYPE_INVALID = ICON_TYPE_BEGIN,
	ICON_TYPE_NORMAL,
	ICON_TYPE_QUICK,
	ICON_TYPE_SUPER,
	ICON_TYPE_WIRELESS_NORMAL,
	ICON_TYPE_WIRELESS_QUICK,
	ICON_TYPE_WIRELESS_SUPER,
	ICON_TYPE_END,
};

void power_ui_event_notify(unsigned long event, const void *data);

#endif /* _POWER_UI_NE_H_ */
