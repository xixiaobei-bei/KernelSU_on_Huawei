/*
 * hisi_cpufreq_lowtemp.h
 *
 * hisi cpufreq lowtemp head file
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef __HISI_CPUFREQ_LOWTEMP_H__
#define __HISI_CPUFREQ_LOWTEMP_H__

#ifdef CONFIG_LPCPU_CPUFREQ_LOWTEMP
/* return true if low temperature, false if not */
bool is_low_temprature(void);

#else
static inline bool is_low_temprature(void) { return false; }
#endif

#endif /* __HISI_CPUFREQ_LOWTEMP_H__ */
