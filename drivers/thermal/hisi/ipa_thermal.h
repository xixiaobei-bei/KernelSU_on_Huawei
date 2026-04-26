/*
 * ipa_thermal.h
 *
 * header of thermal ipa framework
 *
 * Copyright (C) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __IPA_THERMAL_H
#define __IPA_THERMAL_H
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/types.h>

#define IPA_SENSOR	"tsens_max"
#define IPA_SENSOR_SYSTEM_H	"system_h"
#define IPA_SENSOR_MAXID	255
/* 254 used by IPA_SENSOR_SHELLID */
#define IPA_SENSOR_VIRTUAL	"virtual_sensor"
#define IPA_SENSOR_VIRTUALID	253
#define IPA_SENSOR_VIRTUALTEMP	45000
#define IPA_INIT_OK	0x05a5a5a5b
#define IPA_SOC_INIT_TEMP	85000
#ifdef CONFIG_THERMAL_SPM
#define MAX_SHOW_STR	5
#endif
#ifdef CONFIG_THERMAL_SHELL
#define NUM_SENSORS	4
#else
#define NUM_SENSORS	3
#endif
#define NUM_TEMP_SCALE_CAPS	5
#define NUM_TZD	2
#define NUM_BOARD_CDEV	3
#define FREQ_BUF_LEN	11UL
#define DECIMAL	10
#define NUM_CAPACITANCES	5
#define MAX_DEV_NUM	40

#ifdef CONFIG_THERMAL_SPM
struct spm_t {
	struct device *device;
	bool spm_mode;
	bool vr_mode;
};
#endif

enum {
	SOC = 0,
	BOARD
};

enum {
	HOTPLUG_NONE = 0,
	HOTPLUG_NORMAL,
	HOTPLUG_CRITICAL
};

enum {
	PROFILE_SPM = 0,
	PROFILE_VR,
	PROFILE_MIN
};

/*
 * the num of ipa cpufreq table equals cluster num , but
 * cluster num is a variable. So define a larger arrays in advance.
 */
#define CAPACITY_OF_ARRAY	10
struct capacitances {
	u32 cluster_dyn_capacitance[CAPACITY_OF_ARRAY];
	u32 cluster_static_capacitance[CAPACITY_OF_ARRAY];
	u32 cache_capacitance[CAPACITY_OF_ARRAY];
	const char *temperature_scale_capacitance[NUM_TEMP_SCALE_CAPS];
	bool initialized;
};

struct ipa_sensor {
	u32 sensor_id;
	s32 prev_temp;
	int alpha;
};

struct ipa_thermal {
	struct ipa_sensor ipa_sensor;
	struct thermal_zone_device *tzd;
	int cdevs_num;
	struct thermal_cooling_device **cdevs;
	struct capacitances *caps;
	int init_flag;
};

#ifdef CONFIG_THERMAL_HOTPLUG
#define MAX_MODE_LEN	15
struct hotplug_t {
	struct device *device;
	int cpu_down_threshold;
	int cpu_up_threshold;
	int critical_cpu_down_threshold;
	int critical_cpu_up_threshold;
	int hotplug_status;
	unsigned int cpu_hotplug_mask;
	unsigned int critical_cpu_hotplug_mask;
	unsigned int control_mask; /* diff mask */
	bool need_down;
	bool need_up;
	long current_temp;
#ifdef CONFIG_HOTPLUG_EMULATION
	int emul_temp;
#endif
	bool cpu_downed;
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
	bool gpu_need_down;
	bool gpu_need_up;
	bool gpu_downed;
	int gpu_down_threshold;
	int gpu_up_threshold;
	unsigned int gpu_limit_cores;
	unsigned int gpu_total_cores;
	long gpu_current_temp;
#endif
#ifdef CONFIG_GPU_HOTPLUG_EMULATION
	int gpu_emul_temp;
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
	bool npu_need_down;
	bool npu_need_up;
	bool npu_downed;
	int npu_down_threshold;
	int npu_up_threshold;
	unsigned int npu_limit_cores;
	unsigned int npu_total_cores;
	long npu_current_temp;
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
	int npu_emul_temp;
#endif
	struct task_struct *hotplug_task;
	struct delayed_work poll_work;
	u32 polling_delay;
	u32 polling_delay_passive;
	spinlock_t hotplug_lock;
	bool disabled;
	bool initialized;
};
#endif

struct thermal {
#if defined(CONFIG_THERMAL_SPM) || defined(CONFIG_THERMAL_HOTPLUG)
	struct class *thermal_class;
#endif
	struct ipa_thermal ipa_thermal[NUM_TZD];
#ifdef CONFIG_THERMAL_SPM
	struct spm_t spm;
#endif
#ifdef CONFIG_THERMAL_HOTPLUG
	struct hotplug_t hotplug;
#endif
};

#ifdef CONFIG_THERMAL_NPU_HOTPLUG
#define NPU_DEV_ID	0
#endif
#endif

