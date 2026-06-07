/* Copyright (c) 2012-2014 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __DPM_HWMON_H__
#define __DPM_HWMON_H__

#include <linux/list.h>

#define DPM_MODULE_NAME_LENGTH 20
#define DPM_BUFFER_SIZE 128
#define UINT64_MAX 0xffffffffffffffff

struct dpm_hwmon_ops {
	const char *name;
	unsigned long long *dpm_counter_table;
	int *dpm_fitting_table;
	int init_flag;
	unsigned int dpm_cnt_tab_len;
	unsigned int dpm_fit_tab_len;
	struct list_head ops_list;
	int (*hi_dpm_update_counter)(void);
#ifdef CONFIG_HISI_DPM_HWMON_DEBUG
	int (*hi_dpm_get_counter_for_fitting)(void);
#endif
};

extern int dpm_hwmon_register(struct dpm_hwmon_ops *dpm_ops);
extern int dpm_hwmon_unregister(struct dpm_hwmon_ops *dpm_ops);
extern void dpm_monitor_enable(void __iomem * module_dpm_addr, unsigned int dpmonitor_signal_mode);
extern void dpm_monitor_disable(void __iomem * module_dpm_addr);
extern long long int get_dpm_chdmod_power(const char *name);

#ifdef CONFIG_HISI_DPM_HWMON_DEBUG
extern unsigned int g_dpm_buffer_for_fitting[DPM_BUFFER_SIZE];
#endif

#endif
