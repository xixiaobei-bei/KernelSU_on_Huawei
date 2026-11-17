/*
 * lpcpu_idle_sleep.h
 *
 * information about cpuidle
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LPCPU_IDLE_SLEEP_H__
#define __LPCPU_IDLE_SLEEP_H__

enum {
	ID_MODEM = 0,
	ID_IOMCU,
	ID_GPS,
	ID_UFS,
	ID_WIFI,
	ID_MUX,
};

#ifdef CONFIG_LPCPU_MULTIDRV_CPUIDLE
u32 lpcpu_get_idle_cpumask(void);
#else
static inline u32 lpcpu_get_idle_cpumask(void) { return 0; }
#endif

#ifdef CONFIG_LPCPU_IDLE_SLEEP
s32 lpcpu_idle_sleep_vote(u32 modid, u32 val);
u32 lpcpu_idle_sleep_getval(void);
#else
static inline s32 lpcpu_idle_sleep_vote(u32 modid, u32 val) { return 0; }
static inline u32 lpcpu_idle_sleep_getval(void) { return 0; }
#endif

#endif
