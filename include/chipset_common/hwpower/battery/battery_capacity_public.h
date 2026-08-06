/*
 * battery_capacity_public.h
 *
 * huawei battery capacity public interface
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

#ifndef _BATTERY_CAPACITY_PUBLIC_H_
#define _BATTERY_CAPACITY_PUBLIC_H_

#ifdef CONFIG_HUAWEI_BATTERY_UI_CAPACTIY
int bat_ui_capacity_get_filter_sum(int base);
void bat_ui_capacity_sync_filter(int rep_soc, int round_soc, int base);
void bat_ui_capacity_cancle_work(void);
void bat_ui_capacity_restart_work(void);
#else
static inline int bat_ui_capacity_get_filter_sum(int base)
{
	return -EPERM;
}

static inline void bat_ui_capacity_sync_filter(
	int rep_soc, int round_soc, int base)
{
}

static inline void bat_ui_capacity_cancle_work(void)
{
}

static inline void bat_ui_capacity_restart_work(void)
{
}
#endif /* CONFIG_HUAWEI_BATTERY_UI_CAPACTIY */

#endif /* _BATTERY_CAPACITY_PUBLIC_H_ */
