/*
 * direct_charge_work.h
 *
 * direct charge work module
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

#ifndef _DIRECT_CHARGE_WORK_H_
#define _DIRECT_CHARGE_WORK_H_

#include <linux/hrtimer.h>
#include <linux/workqueue.h>

#ifdef CONFIG_DIRECT_CHARGER
void dc_control_work(struct work_struct *work);
void dc_calc_thld_work(struct work_struct *work);
void dc_kick_wtd_work(struct work_struct *work);
enum hrtimer_restart dc_calc_thld_timer_func(struct hrtimer *timer);
enum hrtimer_restart dc_control_timer_func(struct hrtimer *timer);
enum hrtimer_restart dc_kick_wtd_timer_func(struct hrtimer *timer);
#else
static inline void dc_control_work(struct work_struct *work)
{
}

static inline void dc_calc_thld_work(struct work_struct *work)
{
}

static inline void dc_kick_wtd_work(struct work_struct *work)
{
}

static inline enum hrtimer_restart dc_calc_thld_timer_func(
	struct hrtimer *timer)
{
	return HRTIMER_NORESTART;
}

static inline enum hrtimer_restart dc_control_timer_func(
	struct hrtimer *timer)
{
	return HRTIMER_NORESTART;
}

static inline enum hrtimer_restart dc_kick_wtd_timer_func(
	struct hrtimer *timer)
{
	return HRTIMER_NORESTART;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_WORK_H_ */
