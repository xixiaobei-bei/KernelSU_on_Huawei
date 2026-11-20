/*
 * dpm_hwmon.h
 *
 * dpm interface header, dpm_ops structure
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
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

#ifndef __DPM_HWMON_H__
#define __DPM_HWMON_H__

#include <linux/list.h>
#ifdef CONFIG_DPM_HWMON_V2
#include <soc_dpmonitor_interface.h>
#endif

#define DPM_BUFFER_SIZE		256
#define UINT64_MAX		0xffffffffffffffff
#define DPM_NAME_SIZE		64

extern bool g_dpm_report_enabled;

enum dpm_module_type {
	DPM_PERI_MODULE = 0,
	DPM_NPU_MODULE,
	DPM_GPU_MODULE,
};

enum dpm_module_id {
	DPM_DSS_ID = 0,
	DPM_VDEC_ID,
	DPM_VENC_ID,
	DPM_ISP_ID,
	DPM_DMSS_ID,
	DPM_GPU_ID,
	DPM_NPU_ID,
	DPM_MODULE_NUM,
};

static const char* dpm_module_table[DPM_MODULE_NUM] = {
	"DSS",
	"VDEC",
	"VENC",
	"ISP",
	"DMSS",
	"GPU",
	"NPU",
};

struct dpm_hwmon_ops {
	const int dpm_module_id;
	const int dpm_type;
	unsigned long long *dpm_counter_table;
	unsigned int dpm_cnt_len;
	struct list_head ops_list;
	int (*hi_dpm_update_counter)(void);
#if defined(CONFIG_DPM_HWMON_V1)
	unsigned int dpm_fit_len;
	int *dpm_fitting_table;
	unsigned long long *last_dpm_counter;
	unsigned long long last_time;
	int compensate_coff;
	int (*hi_dpm_get_voltage)(void); /* mv */
	int (*hi_dpm_fitting_coff)(void); /* mul 1000000 */
#elif defined(CONFIG_DPM_HWMON_V2)
	bool module_enabled;
	unsigned int dpm_power_len;
	unsigned long long *dpm_power_table;
	int (*hi_dpm_update_power)(void);
#endif
#ifdef CONFIG_DPM_HWMON_DEBUG
	int (*hi_dpm_get_counter_for_fitting)(int mode);
#endif
};

extern int dpm_hwmon_register(struct dpm_hwmon_ops *dpm_ops);
extern int dpm_hwmon_unregister(struct dpm_hwmon_ops *dpm_ops);

#if defined(CONFIG_DPM_HWMON_V1)
extern void dpm_monitor_enable(void __iomem * module_dpm_addr,
			       unsigned int dpmonitor_signal_mode);
extern void dpm_monitor_disable(void __iomem * module_dpm_addr);
#elif defined(CONFIG_DPM_HWMON_V2)
extern void dpm_monitor_enable(void __iomem * module_dpm_addr,
			       unsigned int dpmonitor_signal_mode,
			       unsigned int *param, int param_num);
extern void dpm_monitor_disable(void __iomem * module_dpm_addr);
extern int dpm_hwmon_update_power(void __iomem *module_dpm_addr,
				  unsigned long long *power, int mode);
extern int dpm_hwmon_update_counter_power(void __iomem *module_dpm_addr,
					  unsigned long long *counter_table, int counter_len,
					  unsigned long long *power, int mode);
#endif

#ifdef CONFIG_DPM_HWMON_DEBUG
extern unsigned long long g_dpm_buffer_for_fitting[DPM_BUFFER_SIZE];
#endif

#endif
