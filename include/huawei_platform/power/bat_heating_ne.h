/*
 * bat_heating_ne.h
 *
 * notifier event of battery heating
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

#ifndef _BAT_HEATING_NE_H_
#define _BAT_HEATING_NE_H_

/*
 * define notifier event for battery heating
 * NE is simplified identifier with notifier event
 */
enum bat_heating_ne_list {
	BAT_HEATING_NE_STOP,
	BAT_HEATING_NE_START,
};

#ifdef CONFIG_HUAWEI_BAT_HEATING
void bat_heating_event_notify(enum bat_heating_ne_list event);
#else
static inline void bat_heating_event_notify(enum bat_heating_ne_list event)
{
}
#endif /* CONFIG_HUAWEI_BAT_HEATING */

#endif /* _BAT_HEATING_NE_H_ */
