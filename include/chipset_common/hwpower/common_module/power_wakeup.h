/*
 * power_wakeup.h
 *
 * wakeup for power module
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

#ifndef _POWER_WAKEUP_H_
#define _POWER_WAKEUP_H_

#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/version.h>

struct wakeup_source *power_wakeup_source_register(struct device *dev,
	const char *name);
void power_wakeup_source_unregister(struct wakeup_source *ws);
void power_wakeup_lock(struct wakeup_source *ws);
void power_wakeup_unlock(struct wakeup_source *ws);

#endif /* _POWER_WAKEUP_H_ */
