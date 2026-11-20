
/*
 * dpm_hwmon_v1.h
 *
 * dpm hwmon 1st version
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

#ifndef __DPM_HWMON_V1_H
#define __DPM_HWMON_V1_H

#include <linux/hisi/dpm_hwmon.h>
#include <linux/kernel.h>
#include <linux/mutex.h>

unsigned long long get_dpm_power(struct dpm_hwmon_ops *pos);
void dpm_enable_module(struct dpm_hwmon_ops *pos, bool dpm_switch);
bool get_dpm_enabled(struct dpm_hwmon_ops *pos);
void dpm_sample(struct dpm_hwmon_ops *pos);

extern struct list_head g_dpm_hwmon_ops_list;
extern struct mutex g_dpm_hwmon_ops_list_lock;
#endif
