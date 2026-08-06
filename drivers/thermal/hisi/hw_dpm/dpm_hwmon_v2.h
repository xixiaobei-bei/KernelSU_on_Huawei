
/*
 * hi_dpm_hwmon.c
 *
 * dpm interface for component and user
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#ifndef __DPM_HWMON_V2_H
#define __DPM_HWMON_V2_H

#include <linux/hisi/dpm_hwmon.h>
#include <linux/kernel.h>
#include <linux/mutex.h>

unsigned long long get_dpm_power(struct dpm_hwmon_ops *pos);
void dpm_enable_module(struct dpm_hwmon_ops *pos, bool dpm_switch);
bool get_dpm_enabled(struct dpm_hwmon_ops *pos);
void dpm_sample(struct dpm_hwmon_ops *pos);

void dpm_enable_peri_pcr(void);
void peri_pcr_vote(bool vote, unsigned int module_mask);
void dpm_iounmap(void);
int dpm_ioremap(void);

extern struct list_head g_dpm_hwmon_ops_list;
extern struct mutex g_dpm_hwmon_ops_list_lock;
extern unsigned int g_dpm_peri_pcr_vote;
#endif
