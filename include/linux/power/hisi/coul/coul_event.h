/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: notifier interface for coul module
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

#ifndef _COUL_EVENT_H
#define _COUL_EVENT_H

#include <linux/notifier.h>
#include <linux/time.h>

enum batt_coul_event {
	/* event below just processed in the orign flow */
	BATT_SOH_ACR = 0,
	BATT_SOH_DCR = 1,
	BATT_EEPROM_CYC = 2,
	BATT_OCV_UPDATE = 3,
	BATT_EIS_FREQ = 4,

	/* event below should be processed as atom in the orign flow
	 * and event name should be end with _ATOM
	 */
};

struct ocv_update_data {
	time_t sample_time_sec;
	int ocv_volt_uv; /* uV */
	int ocv_soc_uah; /* uAh */
	s64 cc_value; /* uAh */
	int tbatt;
	int pc;
	int ocv_level;
	int batt_chargecycles;
};

#ifdef CONFIG_COUL_DRV
int coul_register_atomic_notifier(struct notifier_block *nb);
int coul_unregister_atomic_notifier(struct notifier_block *nb);
int call_coul_atomic_notifiers(unsigned long val, void *v);
int coul_register_blocking_notifier(struct notifier_block *nb);
int coul_unregister_blocking_notifier(struct notifier_block *nb);
int call_coul_blocking_notifiers(unsigned long val, void *v);
#else
static inline int coul_register_atomic_notifier(struct notifier_block *nb)
{
	return 0;
}
static inline int coul_unregister_atomic_notifier(struct notifier_block *nb)
{
	return 0;
}
static inline int call_coul_atomic_notifiers(unsigned long val, void *v)
{
	return 0;
}
static inline int coul_register_blocking_notifier(struct notifier_block *nb)
{
	return 0;
}
static inline int coul_unregister_blocking_notifier(struct notifier_block *nb)
{
	return 0;
}
static inline int call_coul_blocking_notifiers(unsigned long val, void *v)
{
	return 0;
}
#endif
#endif
