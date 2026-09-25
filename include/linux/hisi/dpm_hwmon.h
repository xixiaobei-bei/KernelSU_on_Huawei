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
#define DPM_BUFFER_SIZE 256
#define UINT64_MAX 0xffffffffffffffff
extern bool dpm_report_enabled;

struct dpm_hwmon_ops {
	const char *name;
	unsigned long long *dpm_counter_table;
	unsigned long long *last_dpm_counter;
	int *dpm_fitting_table;
	unsigned int dpm_cnt_len;
	unsigned int dpm_fit_len;
	struct list_head ops_list;
	int (*hi_dpm_update_counter)(void);
	int (*hi_dpm_fitting_coff)(void); /* mul 1000000  */
#ifdef CONFIG_HISI_DPM_TWO_LEVEL
	unsigned long long last_time;
	int compensate_coff;;
	int (*hi_dpm_get_voltage)(void); /* mv */
#endif
#ifdef CONFIG_HISI_DPM_HWMON_DEBUG
#ifdef CONFIG_HISI_DPM_TWO_LEVEL
	int (*hi_dpm_get_counter_for_fitting)(int mode);
#else
	int (*hi_dpm_get_counter_for_fitting)(void);
#endif
#endif
};

extern int dpm_hwmon_register(struct dpm_hwmon_ops *dpm_ops);
extern int dpm_hwmon_unregister(struct dpm_hwmon_ops *dpm_ops);
extern void dpm_monitor_enable(void __iomem * module_dpm_addr, unsigned int dpmonitor_signal_mode);
extern void dpm_monitor_disable(void __iomem * module_dpm_addr);
extern long long get_dpm_chdmod_power(const char *name);

#ifdef CONFIG_HISI_DPM_HWMON_DEBUG
extern unsigned long long g_dpm_buffer_for_fitting[DPM_BUFFER_SIZE];
static const char * const dpm_module_table[] = {
	"dss_disp",
	"aicore",
	"vdec",
	"venc0",
	"dpm_ipp",
	"dpm_ivp",
	"dpm_npu",
};

#endif

#endif
