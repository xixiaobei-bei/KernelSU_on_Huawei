/*
 * ipa_thermal.c
 *
 * thermal ipa framework
 *
 * Copyright (C) 2017-2020 Huawei Technologies Co., Ltd.
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

#include "ipa_thermal.h"
#include <linux/cpu_cooling.h>
#include <linux/debugfs.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <linux/topology.h>
#include <trace/events/thermal_power_allocator.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/kthread.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
#include <linux/sched/task.h>
#endif
#include <uapi/linux/sched/types.h>
#include <securec.h>
#include <linux/of_platform.h>
#ifdef CONFIG_THERMAL_SPM
#include <linux/string.h>
#endif
#include <linux/lpm_thermal.h>

struct capacitances g_caps;
struct thermal g_ipa_thermal_info;
u32 g_cluster_num = 2;
u32 g_ipa_sensor_num = 3;
u32 g_ipa_cpufreq_table_index[CAPACITY_OF_ARRAY];
const char *g_ipa_actor_name[CAPACITY_OF_ARRAY];
u32 g_ipa_actor_index[CAPACITY_OF_ARRAY];

typedef int (*ipa_get_sensor_id_t)(const char *);
struct ipa_sensor_info {
	const char *ipa_sensor_name;
	ipa_get_sensor_id_t ipa_get_sensor_id;
};
struct ipa_sensor_info g_ipa_sensor_info[NUM_SENSORS] = {
	{
		.ipa_sensor_name = IPA_SENSOR,
		.ipa_get_sensor_id = (ipa_get_sensor_id_t)ipa_get_tsensor_id
	},
#ifdef CONFIG_THERMAL_SHELL
	{
		.ipa_sensor_name = IPA_SENSOR_SHELL,
		.ipa_get_sensor_id = (ipa_get_sensor_id_t)ipa_get_tsensor_id
	},
#endif
	{
		.ipa_sensor_name = IPA_SENSOR_SYSTEM_H,
		.ipa_get_sensor_id = (ipa_get_sensor_id_t)ipa_get_periph_id
	},
	{
		.ipa_sensor_name = IPA_SENSOR_VIRTUAL,
		.ipa_get_sensor_id = (ipa_get_sensor_id_t)ipa_get_tsensor_id
	}
};

get_power_t g_get_board_power[NUM_BOARD_CDEV] = {
	get_camera_power,
	get_backlight_power,
	get_charger_power
};

#ifdef CONFIG_HOTPLUG_EMULATION
static ssize_t
hotplug_emul_temp_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int temperature = 0;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (kstrtoint(buf, DECIMAL, &temperature))
		return -EINVAL;

	temperature = thermal_zone_temp_check(temperature);
	g_ipa_thermal_info.hotplug.emul_temp = temperature;
	pr_err("hotplug emul temp set : %d\n", temperature);

	return (long)count;
}

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
static DEVICE_ATTR(hotplug_emul_temp, S_IWUSR, NULL, hotplug_emul_temp_store);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/
#endif

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
static ssize_t
gpu_hotplug_emul_temp_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int temperature = 0;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (kstrtoint(buf, DECIMAL, &temperature))
		return -EINVAL;

	temperature = thermal_zone_temp_check(temperature);
	g_ipa_thermal_info.hotplug.gpu_emul_temp = temperature;
	pr_err("gpu hotplug emul temp set : %d\n", temperature);

	return (long)count;
}

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
static DEVICE_ATTR(gpu_hotplug_emul_temp, S_IWUSR, NULL,
		   gpu_hotplug_emul_temp_store);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/
#endif

#ifdef CONFIG_NPU_HOTPLUG_EMULATION
static ssize_t
npu_hotplug_emul_temp_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int temperature = 0;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (kstrtoint(buf, DECIMAL, &temperature))
		return -EINVAL;

	temperature = thermal_zone_temp_check(temperature);
	g_ipa_thermal_info.hotplug.npu_emul_temp = temperature;
	pr_err("npu hotplug emul temp set : %d\n", temperature);

	return (long)count;
}

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
static DEVICE_ATTR(npu_hotplug_emul_temp, S_IWUSR, NULL,
		   npu_hotplug_emul_temp_store);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/
#endif
#ifdef CONFIG_THERMAL_HOTPLUG
static ssize_t
mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	return snprintf_s(buf, (unsigned long)MAX_MODE_LEN,
			  (unsigned long)(MAX_MODE_LEN - 1), "%s\n",
			  g_ipa_thermal_info.hotplug.disabled ? "disabled" : "enabled");
}

static ssize_t
mode_store(struct device *dev, struct device_attribute *attr,
	   const char *buf, size_t count)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (strncmp(buf, "enabled", sizeof("enabled") - 1) == 0)
		g_ipa_thermal_info.hotplug.disabled = false;
	else if (strncmp(buf, "disabled", sizeof("disabled") - 1) == 0)
		g_ipa_thermal_info.hotplug.disabled = true;
	else
		return -EINVAL;

	return (ssize_t)count;
}

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
static DEVICE_ATTR(hotplug_mode, 0644, mode_show, mode_store);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/

#ifdef CONFIG_HOTPLUG_EMULATION
/* cppcheck-suppress */
#define MODE_NAME_LEN	8
#define SHOW_THRESHOLD(mode_name)					\
static ssize_t show_##mode_name						\
(struct device *dev, struct device_attribute *attr, char *buf)		\
{									\
	if (dev == NULL || attr == NULL || buf == NULL)			\
		return 0;						\
									\
	return snprintf_s(buf, MODE_NAME_LEN,  MODE_NAME_LEN - 1, "%d\n",	\
			  (int)g_ipa_thermal_info.hotplug.mode_name);	\
}

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
SHOW_THRESHOLD(gpu_down_threshold);
SHOW_THRESHOLD(gpu_up_threshold);
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
SHOW_THRESHOLD(npu_down_threshold);
SHOW_THRESHOLD(npu_up_threshold);
#endif
SHOW_THRESHOLD(cpu_down_threshold);
SHOW_THRESHOLD(cpu_up_threshold);
SHOW_THRESHOLD(critical_cpu_down_threshold);
SHOW_THRESHOLD(critical_cpu_up_threshold);
/* cppcheck-suppress */
#define STORE_THRESHOLD(mode_name)				\
static ssize_t store_##mode_name				\
(struct device *dev, struct device_attribute *attr,		\
 const char *buf, size_t count)					\
{								\
	int value = 0;						\
								\
	if (dev == NULL || attr == NULL || buf == NULL)		\
		return 0;					\
								\
	if (kstrtoint(buf, DECIMAL, &value))			\
		return -EINVAL;					\
								\
	g_ipa_thermal_info.hotplug.mode_name = value;		\
								\
	return (ssize_t)count;					\
}

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
STORE_THRESHOLD(gpu_down_threshold);
STORE_THRESHOLD(gpu_up_threshold);
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
STORE_THRESHOLD(npu_down_threshold);
STORE_THRESHOLD(npu_up_threshold);
#endif
STORE_THRESHOLD(cpu_down_threshold);
STORE_THRESHOLD(cpu_up_threshold);
STORE_THRESHOLD(critical_cpu_down_threshold);
STORE_THRESHOLD(critical_cpu_up_threshold);

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
#define THRESHOLD_ATTR_RW(mode_name)				\
static DEVICE_ATTR(mode_name, S_IWUSR | S_IRUGO,		\
		   show_##mode_name, store_##mode_name)

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
THRESHOLD_ATTR_RW(gpu_down_threshold);
THRESHOLD_ATTR_RW(gpu_up_threshold);
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
THRESHOLD_ATTR_RW(npu_down_threshold);
THRESHOLD_ATTR_RW(npu_up_threshold);
#endif
THRESHOLD_ATTR_RW(cpu_down_threshold);
THRESHOLD_ATTR_RW(cpu_up_threshold);
THRESHOLD_ATTR_RW(critical_cpu_down_threshold);
THRESHOLD_ATTR_RW(critical_cpu_up_threshold);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/
#endif
#endif

u32 get_camera_power(void)
{
	return 0;
}

u32 get_backlight_power(void)
{
	return 0;
}

u32 get_charger_power(void)
{
	return 0;
}

static int get_sensor_id_by_name(const char *name)
{
	int ret = -EINVAL;
	unsigned long i;

	if (!strncmp(name, IPA_SENSOR, strlen(IPA_SENSOR))) {
		pr_info("%s: name %s match, use id:%d\n",
			__func__, name, IPA_SENSOR_MAXID);
		return IPA_SENSOR_MAXID;
	} else if (!strncmp(name, IPA_SENSOR_VIRTUAL,
						strlen(IPA_SENSOR_VIRTUAL))) {
		pr_info("%s: name %s match, use id:%d\n",
			__func__, name, IPA_SENSOR_VIRTUALID);
		return IPA_SENSOR_VIRTUALID;
#ifdef CONFIG_THERMAL_SHELL
	} else if (!strncmp(name, IPA_SENSOR_SHELL,
						strlen(IPA_SENSOR_SHELL))) {
		pr_info("%s: name %s match, use id:%d\n",
				__func__, name, IPA_SENSOR_SHELLID);
		return IPA_SENSOR_SHELLID;
#endif
	}

	for (i = 0; i < ARRAY_SIZE(g_ipa_sensor_info); i++) {
		if (strncmp(name, g_ipa_sensor_info[i].ipa_sensor_name,
					strlen(g_ipa_sensor_info[i].ipa_sensor_name)) == 0)
			ret = g_ipa_sensor_info[i].ipa_get_sensor_id(name);
	}
	return ret;
}

#ifdef CONFIG_THERMAL_SPM
static void update_cpufreqs(void)
{
	int i;

	for (i = 0; i < (int)g_cluster_num; i++)
		cpufreq_update_policy(g_ipa_cpufreq_table_index[i]);
}

static bool setfreq_available(unsigned int idx, u32 freq)
{
	struct cpufreq_frequency_table *pos = NULL;
	struct cpufreq_frequency_table *table = NULL;
	bool ret = false;

	if (idx < g_cluster_num) {
		/* cpu core */
		table = cpufreq_frequency_get_table(g_ipa_cpufreq_table_index[idx]);
		if (table == NULL) {
			pr_err("%s: Unable to find freq table\n", __func__);
			return false;
		}
		cpufreq_for_each_valid_entry(pos, table) { /*lint !e613*/
			if (pos != NULL && freq == pos->frequency) /*lint !e613*/
				ret = true;
		}
		if (ret != true)
			pr_err("idx %d : freq %d don't match any available freq.\n ",
			       idx, freq);
	} else {
		ret = true; /* GPU */
	}

	return ret;
}

unsigned int g_powerhal_actor_num = 3;
unsigned int g_powerhal_profiles[3][CAPACITY_OF_ARRAY] = {{0}, {0}, {0}};

/* cppcheck-suppress */
#define SHOW_MODE(mode_name)					\
static ssize_t show_##mode_name					\
(struct device *dev, struct device_attribute *attr, char *buf)	\
{								\
	if (dev == NULL || attr == NULL || buf == NULL)		\
		return 0;					\
								\
	return snprintf_s(buf, (unsigned long)MAX_SHOW_STR,	\
			  (unsigned long)(MAX_SHOW_STR - 1), "%d\n",	\
			  (int)g_ipa_thermal_info.spm.mode_name);	\
}

SHOW_MODE(vr_mode);
SHOW_MODE(spm_mode);

/* cppcheck-suppress */
#define STORE_MODE(mode_name)					\
static ssize_t store_##mode_name				\
(struct device *dev, struct device_attribute *attr,		\
 const char *buf, size_t count)					\
{								\
	u32 mode_name = 0;					\
								\
	if (dev == NULL || attr == NULL || buf == NULL)		\
		return 0;					\
								\
	if (kstrtouint(buf, DECIMAL, &mode_name))		\
		return -EINVAL;					\
								\
	g_ipa_thermal_info.spm.mode_name = mode_name ? true : false;\
								\
	cpufreq_update_policies();				\
	return (ssize_t)count;					\
}

STORE_MODE(vr_mode);
STORE_MODE(spm_mode);

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
#define MODE_ATTR_RW(mode_name)				\
static DEVICE_ATTR(mode_name, S_IWUSR | S_IRUGO,	\
		   show_##mode_name, store_##mode_name)

MODE_ATTR_RW(vr_mode);
MODE_ATTR_RW(spm_mode);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/

static ssize_t
vr_freq_store(struct device *dev, struct device_attribute *attr,
	      const char *buf, size_t count)
{
	u32 freq;
	int ret;
	unsigned int i = 0;
	char *token = NULL;
	char temp_buf[MAX_DEV_NUM] = {0};
	char *s = NULL;

	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	ret = strncpy_s(temp_buf, sizeof(temp_buf), buf, sizeof(temp_buf) - 1);
	if (ret != EOK)
		return ret;
	s = temp_buf;

	for (token = strsep(&s, ","); (token != NULL) && (i < g_ipa_actor_num);
	     token = strsep(&s, ","), i++) {
		if (kstrtouint(token, DECIMAL, &freq))
			return -EINVAL;
		if (setfreq_available(i, freq))
			g_powerhal_profiles[1][i] = freq;
	}
	update_cpufreqs();
	return (ssize_t)count;
}

static ssize_t
get_freq_show(struct device *dev, struct device_attribute *devattr,
	      char *buf, int pwrhal_prf_idx)
{
	unsigned int i;
	ssize_t ret = 0;
	int rc;

	if (dev == NULL || devattr == NULL || buf == NULL)
		return 0;
	for (i = 0; i < g_ipa_actor_num; i++) {
		rc = snprintf_s(buf + ret, PAGE_SIZE - ret, FREQ_BUF_LEN, "%u%s",
				g_powerhal_profiles[pwrhal_prf_idx][i], ",");
		if (rc < 0) {
			pr_err("%s: snprintf_s error\n", __func__);
			return rc;
		}
		ret += rc;
	}

	rc = snprintf_s(buf + ret - 1, PAGE_SIZE - ret, 1UL, "\n");
	if (rc < 0) {
		pr_err("%s: snprintf_s error\n", __func__);
		return rc;
	}
	ret += rc;
	return ret;
}

static ssize_t vr_freq_show(struct device *dev,
			    struct device_attribute *devattr, char *buf)
{
	return get_freq_show(dev, devattr, buf, PROFILE_VR);
}
static DEVICE_ATTR(vr_freq, S_IWUSR | S_IRUGO, vr_freq_show, vr_freq_store);

static ssize_t spm_freq_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	u32 freq;
	unsigned int i = 0;
	char *token = NULL;
	char temp_buf[MAX_DEV_NUM] = {0};
	char *s = NULL;
	int ret;

	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	ret = strncpy_s(temp_buf, sizeof(temp_buf), buf, sizeof(temp_buf) - 1);
	if (ret != EOK)
		return ret;
	s = temp_buf;

	for (token = strsep(&s, ","); (token != NULL) && (i < g_ipa_actor_num);
	     token = strsep(&s, ","), i++) {
		if (kstrtouint(token, DECIMAL, &freq))
			return -EINVAL;
		if (setfreq_available(i, freq))
			g_powerhal_profiles[0][i] = freq;
	}
	update_cpufreqs();
	return (ssize_t)count;
}

static ssize_t
spm_freq_show(struct device *dev, struct device_attribute *devattr, char *buf)
{
	return get_freq_show(dev, devattr, buf, PROFILE_SPM);
}
static DEVICE_ATTR(spm_freq, S_IWUSR | S_IRUGO, spm_freq_show, spm_freq_store);

static ssize_t
min_freq_store(struct device *dev, struct device_attribute *attr,
	       const char *buf, size_t count)
{
	u32 freq;
	unsigned int i = 0;
	char *token = NULL;
	char temp_buf[MAX_DEV_NUM] = {0};
	char *s = NULL;
	int ret;

	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	ret = strncpy_s(temp_buf, sizeof(temp_buf), buf, sizeof(temp_buf) - 1);
	if (ret != EOK)
		return ret;
	s = temp_buf;

	for (token = strsep(&s, ","); (token != NULL) && (i < g_ipa_actor_num);
	     token = strsep(&s, ","), i++) {
		if (kstrtouint(token, DECIMAL, &freq))
			return -EINVAL;
		if (setfreq_available(i, freq))
			g_powerhal_profiles[2][i] = freq;
	}
	update_cpufreqs();
	return (ssize_t)count;
}

static ssize_t
min_freq_show(struct device *dev, struct device_attribute *devattr, char *buf)
{
	return get_freq_show(dev, devattr, buf, PROFILE_MIN);
}
static DEVICE_ATTR(min_freq, S_IWUSR | S_IRUGO, min_freq_show, min_freq_store);

bool is_spm_mode_enabled(void)
{
	return g_ipa_thermal_info.spm.spm_mode || g_ipa_thermal_info.spm.vr_mode;
}
EXPORT_SYMBOL(is_spm_mode_enabled);

int ipa_get_actor_id(const char *name)
{
	int ret = -1;
	u32 id;

	for (id = 0; id < g_ipa_sensor_num; id++) {
		if (strncmp(name, g_ipa_actor_name[id], strlen(name)) == 0) {
			ret = id;
			return ret;
		}
	}

	return ret;
}
EXPORT_SYMBOL_GPL(ipa_get_actor_id);

unsigned int get_powerhal_profile(int actor)
{
	int cur_policy = -1;
	unsigned int freq = 0;
	int gpu_id;

	if (g_ipa_thermal_info.spm.vr_mode)
		cur_policy = 1;
	else if (g_ipa_thermal_info.spm.spm_mode)
		cur_policy = 0;

	gpu_id = ipa_get_actor_id("gpu");
	if (gpu_id < 0)
		return freq;

	if (cur_policy != -1 && actor <= gpu_id)
		freq = g_powerhal_profiles[cur_policy][actor];

	return freq;
}
EXPORT_SYMBOL(get_powerhal_profile);

unsigned int get_minfreq_profile(int actor)
{
	unsigned int freq = 0;
	int gpu_id;

	gpu_id = ipa_get_actor_id("gpu");
	if (gpu_id < 0)
		return freq;

	if (actor <= gpu_id)
		freq = g_powerhal_profiles[2][actor];

	return freq;
}
EXPORT_SYMBOL(get_minfreq_profile);
#endif

int get_soc_temp(void)
{
	if (g_ipa_thermal_info.ipa_thermal[SOC].init_flag == IPA_INIT_OK &&
	    g_ipa_thermal_info.ipa_thermal[SOC].tzd != NULL) {
		if (g_ipa_thermal_info.ipa_thermal[SOC].tzd->temperature < 0)
			return 0;
		return g_ipa_thermal_info.ipa_thermal[SOC].tzd->temperature; /*lint !e571*/
	}

	return  IPA_SOC_INIT_TEMP;
}
EXPORT_SYMBOL(get_soc_temp);

static u32 get_dyn_power_coeff(int cluster, struct ipa_thermal *ipa_dev)
{
	if (ipa_dev == NULL || ipa_dev->caps == NULL) {
		pr_err("%s parm null\n", __func__);
		return 0;
	}

	return ipa_dev->caps->cluster_dyn_capacitance[cluster];
}

static u32 get_cpu_static_power_coeff(int cluster)
{
	struct capacitances *caps = &g_caps;

	return caps->cluster_static_capacitance[cluster];
}

static u32 get_cache_static_power_coeff(int cluster)
{
	struct capacitances *caps = &g_caps;

	return caps->cache_capacitance[cluster];
}

#define MAX_TEMPERATURE	145
#define MIN_TEMPERATURE	(-40)
#define MAX_CAPS	0xFFFFF
static unsigned long get_temperature_scale(int temp)
{
	int i, temperature, cap, ret;
	int t_exp = 1;
	long long t_scale = 0;
	struct capacitances *caps = &g_caps;
	int capacitance[NUM_CAPACITANCES] = {0};

	temperature = clamp_val(temp, MIN_TEMPERATURE, MAX_TEMPERATURE);
	for (i = 0; i < NUM_CAPACITANCES - 1; i++) {
		ret = kstrtoint(caps->temperature_scale_capacitance[i],
				DECIMAL, &cap);
		if (ret != 0) {
			pr_warning("%s kstortoint is failed\n", __func__);
			return 0;
		}
		capacitance[i] = clamp_val(cap, -MAX_CAPS, MAX_CAPS);
		t_scale += (long long)capacitance[i] * t_exp;
		t_exp *= temperature;
	}

	t_scale = clamp_val(t_scale, 0, UINT_MAX);
	ret = kstrtoint(caps->temperature_scale_capacitance[NUM_CAPACITANCES - 1],
			DECIMAL, &capacitance[NUM_CAPACITANCES - 1]);
	if (ret != 0) {
		pr_warning("%s kstortoint is failed\n", __func__);
		return 0;
	}

	if (capacitance[NUM_CAPACITANCES - 1] <= 0)
		return 0;

	return (unsigned long)(t_scale / capacitance[NUM_CAPACITANCES - 1]); /*lint !e571*/
}

static unsigned long get_voltage_scale(unsigned long u_volt)
{
	unsigned long m_volt = u_volt / 1000;
	unsigned long v_scale;

	v_scale = m_volt * m_volt * m_volt; /* = (m_V^3) / (900 ^ 3) =  */
	return v_scale / 1000000; /* the value returned needs to be /(1E3) */
}

/*
 * voltage in uV and temperature in mC
 * lint -e715 -esym(715,*)
 */
static int cluster_get_static_power(cpumask_t *cpumask, int interval,
					 unsigned long u_volt, u32 *static_power)
{
	int temperature;
	unsigned long t_scale, v_scale;
	u32 cpu_coeff;
	u32 nr_cpus = cpumask_weight(cpumask);
	int cluster = topology_physical_package_id(cpumask_any(cpumask));

	temperature = get_soc_temp();
	temperature /= 1000;

	cpu_coeff = get_cpu_static_power_coeff(cluster);

	t_scale = get_temperature_scale(temperature);
	v_scale = get_voltage_scale(u_volt);
	*static_power = (u32)(nr_cpus * (cpu_coeff * t_scale * v_scale) / 1000000UL);

	if (nr_cpus != 0) {
		u32 cache_coeff = get_cache_static_power_coeff(cluster);

		/* cache leakage */
		*static_power += (u32)((cache_coeff * v_scale * t_scale) / 1000000UL);
	}

	return 0;
}

/*lint -e715 +esym(715,*)*/

#ifdef CONFIG_THERMAL_HOTPLUG
static void thermal_hotplug_cpu_down(struct hotplug_t *hotplug)
{
	int cpu_num;
	struct device *cpu_dev = NULL;

	for (cpu_num = NR_CPUS - 1; cpu_num >= 0; cpu_num--) {
		if ((1UL << (unsigned int)cpu_num) & hotplug->control_mask) {
			cpu_dev = get_cpu_device((unsigned int)cpu_num);
			if (cpu_dev != NULL) {
				device_lock(cpu_dev);
				cpu_down((unsigned int)cpu_num);
				kobject_uevent(&cpu_dev->kobj, KOBJ_OFFLINE);
				cpu_dev->offline = true;
				device_unlock(cpu_dev);
			}
		}
	}
	pr_info("cpu hotplug temp = %ld, cpu_down!!!\n", hotplug->current_temp);
}

static void thermal_hotplug_cpu_up(struct hotplug_t *hotplug)
{
	unsigned int cpu_num;
	struct device *cpu_dev = NULL;

	for (cpu_num = 0; cpu_num < NR_CPUS; cpu_num++) {
		if ((1 << cpu_num) & hotplug->control_mask) {
			cpu_dev = get_cpu_device(cpu_num);
			if (cpu_dev != NULL) {
				device_lock(cpu_dev);
				cpu_up(cpu_num);
				kobject_uevent(&cpu_dev->kobj, KOBJ_ONLINE);
				cpu_dev->offline = false;
				device_unlock(cpu_dev);
			}
		}
	}
	pr_info("cpu hotplug temp = %ld, cpu_up!!!\n", hotplug->current_temp);
}

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
static void thermal_hotplug_gpu_down(struct hotplug_t *hotplug)
{
	pr_info("gpu limit to %u cores, current_temp:%ld !!!\n",
		hotplug->gpu_limit_cores, hotplug->gpu_current_temp);
	gpu_thermal_cores_control(hotplug->gpu_limit_cores);
}

static void thermal_hotplug_gpu_up(struct hotplug_t *hotplug)
{
	pr_info("gpu restore to max cores:%u, current_temp:%ld !!!\n",
		hotplug->gpu_total_cores, hotplug->gpu_current_temp);
	/* max number of cores */
	gpu_thermal_cores_control(hotplug->gpu_total_cores);
}
#endif

#ifdef CONFIG_THERMAL_NPU_HOTPLUG
static void thermal_hotplug_npu_down(struct hotplug_t *hotplug)
{
	pr_info("npu limit to %u cores, current_temp:%ld !!!\n",
		hotplug->npu_limit_cores, hotplug->npu_current_temp);
	npu_ctrl_core(NPU_DEV_ID, hotplug->npu_limit_cores);
}

static void thermal_hotplug_npu_up(struct hotplug_t *hotplug)
{
	pr_info("npu restore to max cores:%u, current_temp:%ld !!!\n",
		hotplug->npu_total_cores, hotplug->npu_current_temp);
	/* max number of cores */
	npu_ctrl_core(NPU_DEV_ID, hotplug->npu_total_cores);
}
#endif

static void trace_hotplug(struct hotplug_t *hotplug)
{
	trace_IPA_cpu_hot_plug(hotplug->cpu_downed,
			       hotplug->current_temp,
			       hotplug->cpu_up_threshold,
			       hotplug->cpu_down_threshold);
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
	trace_IPA_gpu_hot_plug(hotplug->gpu_downed,
			       hotplug->gpu_current_temp,
			       hotplug->gpu_up_threshold,
			       hotplug->gpu_down_threshold);
#endif
}

/*lint -e715*/
static int thermal_hotplug_task(void *data) /*lint !e715*/
{
	unsigned long flags;
	struct hotplug_t *hotplug = &g_ipa_thermal_info.hotplug;

	while (1) {
		spin_lock_irqsave(&hotplug->hotplug_lock, flags); /*lint !e550*/
		if (!hotplug->need_up &&
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
		    !hotplug->gpu_need_down && !hotplug->gpu_need_up &&
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
		    !hotplug->npu_need_down && !hotplug->npu_need_up &&
#endif
		    !hotplug->need_down) {
			set_current_state(TASK_INTERRUPTIBLE); /*lint !e446 !e666*/
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);

			schedule();

			if (kthread_should_stop())
				break;

			spin_lock_irqsave(&hotplug->hotplug_lock, flags); /*lint !e550*/
		}

		if (hotplug->need_down) {
			hotplug->need_down = false;
			hotplug->cpu_downed = true;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_cpu_down(hotplug);
		} else if (hotplug->need_up) {
			hotplug->need_up = false;
			if (hotplug->hotplug_status == HOTPLUG_NONE)
				hotplug->cpu_downed = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_cpu_up(hotplug);
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
		} else if (hotplug->gpu_need_down) {
			hotplug->gpu_downed = true;
			hotplug->gpu_need_down = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_gpu_down(hotplug);
		} else if (hotplug->gpu_need_up) {
			hotplug->gpu_downed = false;
			hotplug->gpu_need_up = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_gpu_up(hotplug);
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
		} else if (hotplug->npu_need_down) {
			hotplug->npu_downed = true;
			hotplug->npu_need_down = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_npu_down(hotplug);
		} else if (hotplug->npu_need_up) {
			hotplug->npu_downed = false;
			hotplug->npu_need_up = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_npu_up(hotplug);
#endif
		} else {
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		}

		trace_hotplug(hotplug);
	}

	return 0;
}
/*lint +e715*/
#endif

#define FRAC_BITS	8
#define int_to_frac(x)	((x) << FRAC_BITS)

/*
 * mul_frac() - multiply two fixed-point numbers
 * @x: first multiplicand
 * @y: second multiplicand
 *
 * Return: the result of multiplying two fixed-point numbers.  The
 * result is also a fixed-point number.
 */
/*lint -e702*/
static inline s32 mul_frac(s32 x, s32 y)
{
	return (s32)((u32)(x * y) >> FRAC_BITS);
}

/*lint +e702*/
#ifdef CONFIG_THERMAL_HOTPLUG
static bool hotplug_cpu_downed_check(struct hotplug_t *hotplug, int temp)
{
	if (temp > hotplug->critical_cpu_down_threshold) {
		if (hotplug->hotplug_status == HOTPLUG_NORMAL) {
			hotplug->need_down = true;
			hotplug->control_mask =
				hotplug->critical_cpu_hotplug_mask &
				(~hotplug->cpu_hotplug_mask);
			hotplug->hotplug_status = HOTPLUG_CRITICAL;
			return true;
		}
	} else if (temp < hotplug->critical_cpu_up_threshold &&
		   temp > hotplug->cpu_up_threshold) {
		if (hotplug->hotplug_status == HOTPLUG_CRITICAL) {
			hotplug->need_up = true;
			hotplug->control_mask =
				hotplug->critical_cpu_hotplug_mask &
				(~hotplug->cpu_hotplug_mask);
			hotplug->hotplug_status = HOTPLUG_NORMAL;
			return true;
		}
	} else if (temp < hotplug->cpu_up_threshold) {
		hotplug->need_up = true;
		if (hotplug->hotplug_status == HOTPLUG_CRITICAL)
			hotplug->control_mask =
				hotplug->critical_cpu_hotplug_mask;
		else if (hotplug->hotplug_status == HOTPLUG_NORMAL)
			hotplug->control_mask = hotplug->cpu_hotplug_mask;
		else
			hotplug->control_mask = 0;

		hotplug->hotplug_status = HOTPLUG_NONE;
		return true;
	}

	return false;
}

static int hotplug_cpu_undowned_check(struct hotplug_t *hotplug,
				int temp)
{
	if (temp > hotplug->critical_cpu_down_threshold) {
		hotplug->need_down = true;
		hotplug->hotplug_status = HOTPLUG_CRITICAL;
		hotplug->control_mask =
			hotplug->critical_cpu_hotplug_mask;
		return true;
	} else if (temp > hotplug->cpu_down_threshold) {
		hotplug->need_down = true;
		hotplug->hotplug_status = HOTPLUG_NORMAL;
		hotplug->control_mask =
			hotplug->cpu_hotplug_mask;
		return true;
	}

	return false;
}

static void thermal_hotplug_check(int *temp)
{
	unsigned long flags;
	struct hotplug_t *hotplug = &g_ipa_thermal_info.hotplug;
	bool need_wakeup = false;

#ifdef CONFIG_HOTPLUG_EMULATION
	if (hotplug->emul_temp != 0)
		*temp = hotplug->emul_temp;
#endif
	if (hotplug->initialized && !hotplug->disabled) {
		spin_lock_irqsave(&hotplug->hotplug_lock, flags); /*lint !e550*/
		hotplug->current_temp = *temp;

		if (hotplug->cpu_downed)
			need_wakeup = hotplug_cpu_downed_check(hotplug, *temp);
		else
			need_wakeup = hotplug_cpu_undowned_check(hotplug, *temp);


		spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		if (need_wakeup)
			wake_up_process(hotplug->hotplug_task);
	}
}
#endif

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
static void thermal_gpu_hotplug_check(int *temp)
{
	unsigned long flags;
	struct hotplug_t *hotplug = &g_ipa_thermal_info.hotplug;

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
	if (hotplug->gpu_emul_temp != 0)
		*temp = hotplug->gpu_emul_temp;
#endif
	if (hotplug->initialized && !hotplug->disabled) {
		spin_lock_irqsave(&hotplug->hotplug_lock, flags); /*lint !e550*/
		hotplug->gpu_current_temp = *temp;

		if (hotplug->gpu_downed) {
			if (*temp < hotplug->gpu_up_threshold) {
				hotplug->gpu_need_up = true;
				wake_up_process(hotplug->hotplug_task);
			}
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		} else {
			if (*temp > hotplug->gpu_down_threshold) {
				hotplug->gpu_need_down = true;
				wake_up_process(hotplug->hotplug_task);
			}
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		}
	}
}
#endif

#ifdef CONFIG_THERMAL_NPU_HOTPLUG
static int ipa_get_npu_temp(int *val)
{
	int id, ret;

	id = ipa_get_tsensor_id("npu");
	if (id < 0) {
		pr_err("%s:%d:tsensor npu is not exist.\n", __func__, __LINE__);
		return -ENODEV;
	}

	ret = ipa_get_sensor_value((u32)id, val);
	return ret;
}

static void thermal_npu_hotplug_check(void)
{
	unsigned long flags;
	int temp = 0;
	int ret;
	struct hotplug_t *hotplug = &g_ipa_thermal_info.hotplug;

	ret = ipa_get_npu_temp(&temp);
	if (ret != 0)
		return;

#ifdef CONFIG_NPU_HOTPLUG_EMULATION
	if (hotplug->npu_emul_temp != 0)
		temp = hotplug->npu_emul_temp;
#endif
	if (hotplug->initialized && !hotplug->disabled) {
		spin_lock_irqsave(&hotplug->hotplug_lock, flags); /*lint !e550*/
		hotplug->npu_current_temp = temp;

		if (hotplug->npu_downed) {
			if (temp < hotplug->npu_up_threshold) {
				hotplug->npu_need_up = true;
				wake_up_process(hotplug->hotplug_task);
			}
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		} else {
			if (temp > hotplug->npu_down_threshold) {
				hotplug->npu_need_down = true;
				wake_up_process(hotplug->hotplug_task);
			}
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		}
	}
}
#endif

#define MAX_ALPHA	256
static u32 thermal_zone_alpha_check(u32 alpha)
{
	if (alpha > MAX_ALPHA)
		alpha = MAX_ALPHA;
	return alpha;
}

static int get_temp_value(void *data, int *temp)
{
	int sensor_val[CAPACITY_OF_ARRAY] = {0};
	struct ipa_sensor *sensor = (struct ipa_sensor *)data;
	int val = 0;
	int val_max, id, est_temp;
	int ret = -EINVAL;
#ifdef CONFIG_THERMAL_SHELL
	struct thermal_zone_device *shell_tz = NULL;
#endif

	if (sensor->sensor_id == IPA_SENSOR_MAXID) {
		/* read all sensor */
		val_max = val;
		for (id = 0; id < (int)g_ipa_sensor_num; id++) {
			ret = ipa_get_sensor_value((u32)id, &val);
			sensor_val[id] = val;
			if(sensor_val[id] > val_max)
				val_max = sensor_val[id];
			if (ret != 0)
				return ret;
		}

		val = val_max;
		trace_IPA_get_tsens_value(g_ipa_sensor_num, sensor_val, val_max);
#ifdef CONFIG_THERMAL_SHELL
	} else if (sensor->sensor_id == IPA_SENSOR_SHELLID) {
		shell_tz = thermal_zone_get_zone_by_name(IPA_SENSOR_SHELL);
		ret = get_shell_temp(shell_tz, temp);
		trace_IPA_get_tsens_value(g_ipa_sensor_num, sensor_val, *temp);
		if (ret != 0)
			return ret;
		else
			return 0;
#endif
	} else if (sensor->sensor_id == IPA_SENSOR_VIRTUALID) {
		*temp = IPA_SENSOR_VIRTUALTEMP;
		trace_IPA_get_tsens_value(g_ipa_sensor_num, sensor_val,
					  IPA_SENSOR_VIRTUALTEMP);
		return 0;
	} else if (sensor->sensor_id < IPA_PERIPH_NUM) {
		ret = ipa_get_periph_value(sensor->sensor_id, &val);
		trace_IPA_get_tsens_value(g_ipa_sensor_num, sensor_val, val);
		if (ret != 0)
			return ret;
		val = val < 0 ? 0 : val;
	} else {
		return ret;
	}

	if (sensor->prev_temp == 0)
		sensor->prev_temp = val;
	 /*lint -e776*/
	val = thermal_zone_temp_check(val);
	sensor->prev_temp = thermal_zone_temp_check(sensor->prev_temp);
	sensor->alpha = thermal_zone_alpha_check(sensor->alpha);
	est_temp = mul_frac(sensor->alpha, val) +
		mul_frac(int_to_frac(1) - sensor->alpha, sensor->prev_temp);
	 /*lint +e776*/
	sensor->prev_temp = est_temp;
	*temp = est_temp;

	return 0;
}

static void update_debugfs(struct ipa_sensor *sensor_data)
{
#ifdef CONFIG_HISI_DEBUG_FS
	struct dentry *dentry_f = NULL;
	struct dentry *filter_d = NULL;
	char filter_name[25];
	int rc;

	rc = snprintf_s(filter_name, sizeof(filter_name), (sizeof(filter_name) - 1),
			"thermal_lpf_filter%d", sensor_data->sensor_id);
	if (rc < 0) {
		pr_err("snprintf_s error.\n");
		return;
	}
	filter_d = debugfs_create_dir(filter_name, NULL);
	if (IS_ERR_OR_NULL(filter_d)) {
		pr_warning("unable to create debugfs directory for the LPF filter\n");
		return;
	}

	dentry_f = debugfs_create_u32("alpha", S_IWUSR | S_IRUGO, filter_d,
				      (u32 *)&sensor_data->alpha);
	if (IS_ERR_OR_NULL(dentry_f)) {
		debugfs_remove(filter_d);
		pr_warn("IPA:Unable to create debugfsfile: alpha\n");
		return;
	}
#endif
}

/*lint -e785*/
static const struct thermal_zone_of_device_ops ipa_thermal_ops = {
	.get_temp = get_temp_value,
};

/*lint +e785*/

#ifdef CONFIG_THERMAL_HOTPLUG
#define DEFAULT_POLL_DELAY	200 /* unit is ms */

static int thermal_cpu_hotplug_dts_parse(struct device_node *hotplug_np,
					      struct hotplug_t *hotplug)
{
	int ret;

	ret = of_property_read_u32(hotplug_np, "hisilicon,up_threshold",
				   (u32 *)&hotplug->cpu_up_threshold);
	if (ret != 0) {
		pr_err("%s hotplug up threshold read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,down_threshold",
				   (u32 *)&hotplug->cpu_down_threshold);
	if (ret != 0) {
		pr_err("%s hotplug down threshold read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,cpu_hotplug_mask",
				   (u32 *)&hotplug->cpu_hotplug_mask);
	if (ret != 0) {
		pr_err("%s hotplug maskd read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,polling_delay",
				   (u32 *)&hotplug->polling_delay);
	if (ret != 0) {
		pr_err("%s hotplug polling_delay use  default value\n",
		       __func__);
		hotplug->polling_delay = DEFAULT_POLL_DELAY;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,polling_delay_passive",
				   (u32 *)&hotplug->polling_delay_passive);
	if (ret != 0) {
		pr_err("%s hotplug polling_delay_passive use  default value\n",
		       __func__);
		hotplug->polling_delay_passive = DEFAULT_POLL_DELAY;
	}

	return 0;
}

static void
thermal_critical_cpu_hotplug_dts_parse(struct device_node *hotplug_np,
					    struct hotplug_t *hotplug)
{
	int ret;

	ret = of_property_read_u32(hotplug_np, "hisilicon,critical_down_threshold",
				   (u32 *)&hotplug->critical_cpu_down_threshold);
	if (ret != 0) {
		pr_err("%s critical_down_threshold use down_threshold value\n",
		       __func__);
		hotplug->critical_cpu_down_threshold =
			hotplug->cpu_down_threshold;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,critical_up_threshold",
				   (u32 *)&hotplug->critical_cpu_up_threshold);
	if (ret != 0) {
		pr_err("%s critical_up_threshold use up_threshold value\n",
		       __func__);
		hotplug->critical_cpu_up_threshold =
			hotplug->cpu_up_threshold;
	}

	ret = of_property_read_u32(hotplug_np,
				   "hisilicon,critical_cpu_hotplug_mask",
				   (u32 *)&hotplug->critical_cpu_hotplug_mask);
	if (ret != 0) {
		pr_err("%s critical_cpu_hotplug_mask use "
		       "cpu_hotplug_mask value:%x\n", __func__,
			hotplug->cpu_hotplug_mask);
		hotplug->critical_cpu_hotplug_mask =
			hotplug->cpu_hotplug_mask;
	}
	pr_err("%s hotplug mask:%x, critical mask:%x\n", __func__,
		hotplug->cpu_hotplug_mask, hotplug->critical_cpu_hotplug_mask);
}

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
static int thermal_gpu_hotplug_dts_parse(struct device_node *hotplug_np,
					      struct hotplug_t *hotplug)
{
	int ret;

	ret = of_property_read_u32(hotplug_np, "hisilicon,gpu_limit_cores",
				   (u32 *)&hotplug->gpu_limit_cores);
	if (ret != 0) {
		pr_err("%s gpu_limit_cores read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,gpu_total_cores",
				   (u32 *)&hotplug->gpu_total_cores);
	if (ret != 0) {
		pr_err("%s gpu_total_cores read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,gpu_down_threshold",
				   (u32 *)&hotplug->gpu_down_threshold);
	if (ret != 0) {
		pr_err("%s gpu_down_threshold read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,gpu_up_threshold",
				   (u32 *)&hotplug->gpu_up_threshold);
	if (ret != 0) {
		pr_err("%s gpu_up_threshold read err\n", __func__);
		return ret;
	}

	return 0;
}
#endif

#ifdef CONFIG_THERMAL_NPU_HOTPLUG
static int thermal_npu_hotplug_dts_parse(struct device_node *hotplug_np,
					      struct hotplug_t *hotplug)
{
	int ret;

	ret = of_property_read_u32(hotplug_np, "hisilicon,npu_limit_cores",
				   (u32 *)&hotplug->npu_limit_cores);
	if (ret != 0) {
		pr_err("%s npu_limit_cores read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,npu_total_cores",
				   (u32 *)&hotplug->npu_total_cores);
	if (ret != 0) {
		pr_err("%s npu_total_cores read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,npu_down_threshold",
				   (u32 *)&hotplug->npu_down_threshold);
	if (ret != 0) {
		pr_err("%s gpu_down_threshold read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32(hotplug_np, "hisilicon,npu_up_threshold",
				   (u32 *)&hotplug->npu_up_threshold);
	if (ret != 0) {
		pr_err("%s npu_up_threshold read err\n", __func__);
		goto node_put;
	}

	return 0;
node_put:
	return ret;
}
#endif

static int thermal_hotplug_task_create(struct hotplug_t *hotplug)
{
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };

	hotplug->hotplug_task
		= kthread_create(thermal_hotplug_task, NULL, "thermal_hotplug");
	if (IS_ERR(hotplug->hotplug_task)) {
		pr_err("%s: thermal hotplug task create fail\n", __func__);
		hotplug->hotplug_task = NULL;
		return -EINVAL;
	}

	sched_setscheduler_nocheck(hotplug->hotplug_task, SCHED_FIFO, &param);
	get_task_struct(hotplug->hotplug_task);
	wake_up_process(hotplug->hotplug_task);

	return 0;
}

static int thermal_hotplug_init(void)
{
	struct device_node *hotplug_np = NULL;
	int ret;
	struct hotplug_t *hotplug = &g_ipa_thermal_info.hotplug;

	if (hotplug->initialized)
		return 0;

	hotplug_np = of_find_node_by_name(NULL, "cpu_temp_threshold");
	if (hotplug_np == NULL) {
		pr_err("cpu_temp_threshold node not found\n");
		return -ENODEV;
	}

	ret = thermal_cpu_hotplug_dts_parse(hotplug_np, hotplug);
	if (ret != 0) {
		pr_err("%s thermal_cpu_hotplug_dts_parse err\n", __func__);
		goto node_put;
	}
	thermal_critical_cpu_hotplug_dts_parse(hotplug_np, hotplug);

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
	ret = thermal_gpu_hotplug_dts_parse(hotplug_np, hotplug);
	if (ret != 0) {
		pr_err("%s thermal_gpu_hotplug_dts_parse err\n", __func__);
		goto node_put;
	}
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
	ret = thermal_npu_hotplug_dts_parse(hotplug_np, hotplug);
	if (ret != 0) {
		pr_err("%s thermal_npu_hotplug_dts_parse err\n", __func__);
		goto node_put;
	}
#endif
	spin_lock_init(&hotplug->hotplug_lock);
	ret = thermal_hotplug_task_create(hotplug);
	if (ret != 0) {
		pr_err("%s thermal_hotplug_task_create err\n", __func__);
		goto node_put;
	}

	hotplug->initialized = true;
	of_node_put(hotplug_np);

	return 0;

node_put:
	of_node_put(hotplug_np);

	return ret;
}
#endif

int of_parse_ipa_sensor_index_table(void)
{
	int ret, i;
	struct device_node *np = NULL;

	np = of_find_node_by_name(NULL, "ipa_sensors_info");
	if (np == NULL) {
		pr_err("ipa_sensors_info node not found\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "hisilicon,cluster_num", &g_cluster_num);
	if (ret != 0) {
		pr_err("%s cluster_num read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32(np, "hisilicon,ipa_sensor_num",
				   &g_ipa_sensor_num);
	if (ret != 0) {
		pr_err("%s ipa_sensor_num read err\n", __func__);
		goto node_put;
	}

	for (i = 0; i < (int)g_ipa_sensor_num; i++) {
		ret = of_property_read_string_index(np, "hisilicon,ipa_actor_name",
						    i, &g_ipa_actor_name[i]);
		if (ret != 0) {
			pr_err("%s g_ipa_actor_name %d read err\n", __func__, i);
			goto node_put;
		}
	}

	ret = of_property_read_u32_array(np, "hisilicon,ipa_actor_index",
					 g_ipa_actor_index, g_ipa_sensor_num);
	if (ret != 0) {
		pr_err("%s g_ipa_actor_index read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32_array(np, "hisilicon,ipa_cpufreq_table_index",
					 g_ipa_cpufreq_table_index, g_cluster_num);
	if (ret != 0) {
		pr_err("%s g_ipa_cpufreq_table_index read err\n", __func__);
		goto node_put;
	}
	for (i = 0; i < (int)g_cluster_num; i++)
		pr_err("%s, cluster%d, g_ipa_cpufreq_table_index:%d \n", __func__,
			i, g_ipa_cpufreq_table_index[i]);

	of_node_put(np);
	return 0;
node_put:
	of_node_put(np);
	return ret;
}
EXPORT_SYMBOL(of_parse_ipa_sensor_index_table);

static int of_parse_thermal_zone_caps(void)
{
	int ret, i;
	struct device_node *np = NULL;

	if (g_caps.initialized)
		return 0;

	np = of_find_node_by_name(NULL, "capacitances");
	if (np == NULL) {
		pr_err("Capacitances node not found\n");
		return -ENODEV;
	}

	ret = of_property_read_u32_array(np, "hisilicon,cluster_dyn_capacitance",
					 g_caps.cluster_dyn_capacitance,
					 g_cluster_num);
	if (ret != 0) {
		pr_err("%s actor_dyn_capacitance read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32_array(np, "hisilicon,cluster_static_capacitance",
					 g_caps.cluster_static_capacitance,
					 g_cluster_num);
	if (ret != 0) {
		pr_err("%s actor_dyn_capacitance read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32_array(np, "hisilicon,cache_capacitance",
					 g_caps.cache_capacitance,
					 g_cluster_num);
	if (ret != 0) {
		pr_err("%s actor_dyn_capacitance read err\n", __func__);
		goto node_put;
	}

	for (i = 0; i < NUM_TEMP_SCALE_CAPS; i++) {
		ret = of_property_read_string_index(np, "hisilicon,temperature_scale_capacitance",
						    i, &g_caps.temperature_scale_capacitance[i]);
		if (ret != 0) {
			pr_err("%s temperature_scale_capacitance %d "
			       "read err\n", __func__, i);
			goto node_put;
		}
	}
	g_caps.initialized = true;
	of_node_put(np);

	return 0;

node_put:
	of_node_put(np);

	return ret;
}

void ipa_get_clustermask(int *clustermask, int len)
{
	int cpu;
	int clunstercnt[MAX_THERMAL_CLUSTER_NUM] = {0};
	int clunsternum = 0;

	(void)len;
	for_each_online_cpu(cpu) { /*lint !e713 !e574*/
		int cluster_idx = topology_physical_package_id(cpu);
		if (clunstercnt[cluster_idx] == 0) {
			clunstercnt[cluster_idx]++;
			clunsternum++;
		}
	}

	if (clunsternum == 1) {
		clustermask[0] = 0;
		clustermask[1] = 0;
		clustermask[2] = 0;
	} else if (clunsternum == 2) {
		// cluster id is 0 or 1 / 0 or 2 /1 or 2
		if (clunstercnt[0] == 1 && clunstercnt[1] == 1) {
			clustermask[0] = 0;
			clustermask[1] = 1;
		} else if (clunstercnt[0] == 1 && clunstercnt[2] == 1) {
			clustermask[0] = 0;
			clustermask[2] = 1;
		} else if (clunstercnt[1] == 1 && clunstercnt[2] == 1) {
			clustermask[1] = 0;
			clustermask[2] = 1;
		}
	} else {
		clustermask[0] = 0;
		clustermask[1] = 1;
		clustermask[2] = 2;
	}
}

static int ipa_register_soc_cdev(struct ipa_thermal *thermal_data,
				 struct platform_device *pdev)
{
	int ret, cpuid, cluster, i, cpu;
	struct device_node *cdev_np = NULL;
	struct cpumask cpu_masks[CAPACITY_OF_ARRAY];
	char node[16] = {0};
	struct cpufreq_policy *policy = NULL;
	int clustermask[MAX_THERMAL_CLUSTER_NUM] = {0};

	ret = memset_s(cpu_masks, sizeof(struct cpumask) * CAPACITY_OF_ARRAY,
		       0, sizeof(struct cpumask) * CAPACITY_OF_ARRAY);
	if (ret != EOK) {
		pr_err("memset_s fail");
		return -ENOMEM;
	}

	ipa_get_clustermask(clustermask, MAX_THERMAL_CLUSTER_NUM);
	for_each_online_cpu(cpu) { /*lint !e713 !e574*/
		int cluster_id = topology_physical_package_id(cpu);

		if (clustermask[cluster_id] >= (int)g_cluster_num) {
			pr_warn("IPA:Cluster id: %d >= max id:%d\n",
				cluster_id, g_cluster_num);
			return -ENODEV;
		}
		cpumask_set_cpu((u32)cpu, &cpu_masks[clustermask[cluster_id]]);
	}

	thermal_data->cdevs =
		kcalloc((size_t)g_cluster_num,
			sizeof(struct thermal_cooling_device *), GFP_KERNEL); /*lint !e433*/
	if (thermal_data->cdevs == NULL) {
		ret = -ENOMEM;
		goto end;
	}

	for (i = 0; i < (int)g_cluster_num; i++) {
		cpuid = (int)cpumask_any(&cpu_masks[i]);
		if ((u32)cpuid >= nr_cpu_ids)
			continue;
		cluster = topology_physical_package_id(cpuid);
		ret = snprintf_s(node, sizeof(node), (sizeof(node) - 1),
				 "cluster%d", i);
		if (ret < 0) {
			dev_err(&pdev->dev, "snprintf_s error: %s\n", node);
			continue;
		}

		cdev_np = of_find_node_by_name(NULL, node);
		if (cdev_np == NULL) {
			dev_err(&pdev->dev, "Node not found: %s\n", node);
			continue;
		}

		policy = cpufreq_cpu_get(cpuid);
		thermal_data->cdevs[i] =
			of_cpufreq_power_cooling_register(cdev_np, policy,
							  get_dyn_power_coeff(cluster,
									      thermal_data),
							  cluster_get_static_power);
		if (IS_ERR(thermal_data->cdevs[i])) {
			ret = (int)PTR_ERR(thermal_data->cdevs[i]);
			if (policy != NULL)
				cpufreq_cpu_put(policy);
			dev_err(&pdev->dev,
				"IPA:Error registering cpu power actor: "
				"cluster %d ERROR_ID %d\n",
				i, ret);
			goto cdevs_unregister;
		}
		thermal_data->cdevs_num++;
		if (policy != NULL)
			cpufreq_cpu_put(policy);
		of_node_put(cdev_np);
	}

	return 0;

cdevs_unregister:
	for (i = 0; i < thermal_data->cdevs_num; i++) /*lint !e574*/
		cpufreq_cooling_unregister(thermal_data->cdevs[i]);
	thermal_data->cdevs_num = 0;
	kfree(thermal_data->cdevs);
	thermal_data->cdevs = NULL;
end:
	return ret;
}

static int ipa_register_board_cdev(struct ipa_thermal *thermal_data,
				   struct platform_device *pdev)
{
	struct device_node *board_np = NULL;
	int ret = 0;
	int i;
	struct device_node *child = NULL;

	board_np = of_find_node_by_name(NULL, "board-map");
	if (board_np == NULL) {
		dev_err(&pdev->dev, "Board map node not found\n");
		goto end;
	}

	ret = of_get_child_count(board_np);
	if (ret <= 0) {
		ret = -EINVAL;
		of_node_put(board_np);
		pr_err("IPA: board map child count err\n");
		goto end;
	}

	thermal_data->cdevs = kzalloc(sizeof(struct thermal_cooling_device *) *
				      (unsigned long)ret, /*lint !e571*/
				      GFP_KERNEL);
	if (thermal_data->cdevs == NULL) {
		ret = -ENOMEM;
		of_node_put(board_np);
		goto end;
	}

	for_each_child_of_node(board_np, child) {
		thermal_data->cdevs[thermal_data->cdevs_num] =
			board_power_cooling_register(child,
						     g_get_board_power[thermal_data->cdevs_num]);
		if (IS_ERR(thermal_data->cdevs[thermal_data->cdevs_num])) {
			of_node_put(board_np);
			ret = PTR_ERR(thermal_data->cdevs[thermal_data->cdevs_num]); /*lint !e712*/
			dev_err(&pdev->dev,
				"IPA:Error registering board power actor: "
				"ERROR_ID %d\n", ret);
			goto cdevs_unregister;
		}
		thermal_data->cdevs_num++;
	}
	of_node_put(board_np);

	return 0;

cdevs_unregister:
	for (i = 0; i < thermal_data->cdevs_num; i++)
		board_cooling_unregister(thermal_data->cdevs[i]);
	thermal_data->cdevs_num = 0;
	kfree(thermal_data->cdevs);
	thermal_data->cdevs = NULL;
end:
	return ret;
}

static inline void cooling_device_unregister(struct ipa_thermal *thermal_data)
{
	int i;

	if (thermal_data->cdevs == NULL)
		return;

	for (i = 0; i < thermal_data->cdevs_num; i++) {
		if (strstr(thermal_data->cdevs[i]->type, "thermal-board"))
			board_cooling_unregister(thermal_data->cdevs[i]);
		else
			cpufreq_cooling_unregister(thermal_data->cdevs[i]);
	}

	kfree(thermal_data->cdevs);
}

static int ipa_thermal_probe(struct platform_device *pdev)
{
	struct ipa_thermal *thermal_data = &g_ipa_thermal_info.ipa_thermal[SOC];
	int sensor, ret;
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	const char *ch = NULL;

	if (!of_device_is_available(dev_node)) {
		dev_err(&pdev->dev, "IPA thermal dev not found\n");
		return -ENODEV;
	}

	if (!cpufreq_frequency_get_table(0)) {
		dev_info(&pdev->dev, "IPA:Frequency table not initialized. "
			 "Deferring probe...\n");
		return -EPROBE_DEFER;
	}

	ret = of_parse_thermal_zone_caps();
	if (ret != 0) {
		pr_err("thermal zone caps parse error\n");
		goto end;
	}

	if (strncmp(pdev->name, "ipa-sensor@0", sizeof("ipa-sensor@0") - 1) == 0) {
		thermal_data = &g_ipa_thermal_info.ipa_thermal[SOC];
		thermal_data->caps = &g_caps;

		ret = ipa_register_soc_cdev(thermal_data, pdev);
	} else if (strncmp(pdev->name, "ipa-sensor@1",
			    sizeof("ipa-sensor@1") - 1) == 0) {
		thermal_data = &g_ipa_thermal_info.ipa_thermal[BOARD];
		thermal_data->caps = &g_caps;

		ret = ipa_register_board_cdev(thermal_data, pdev);
	}
	if (ret != 0)
		goto end;

	ret = of_property_read_string(dev_node, "type", &ch);
	if (ret != 0) {
		dev_err(dev, "%s sensor name read err\n", __func__);
		goto cdevs_unregister;
	}

	sensor = get_sensor_id_by_name(ch);
	if (sensor < 0) {
		ret = sensor;
		goto cdevs_unregister;
	}

	thermal_data->ipa_sensor.sensor_id = (u32)sensor;
	dev_info(&pdev->dev, "IPA:Probed %s sensor. Id = %d\n", ch, sensor);

	/*
	 * alpha ~= 2 / (N + 1) with N the window of a rolling mean We
	 * use 8-bit fixed point arithmetic.  For a rolling average of
	 * window 20, alpha = 2 / (20 + 1) ~= 0.09523809523809523 .
	 * In 8-bit fixed point arigthmetic, 0.09523809523809523 * 256
	 * ~= 24
	 */
	thermal_data->ipa_sensor.alpha = 24;
	thermal_data->tzd =
		thermal_zone_of_sensor_register(&pdev->dev,
						(int)thermal_data->ipa_sensor.sensor_id,
						&thermal_data->ipa_sensor,
						&ipa_thermal_ops);

	if (IS_ERR(thermal_data->tzd)) {
		dev_err(&pdev->dev, "IPA ERR:registering sensor tzd error.\n");
		ret = PTR_ERR(thermal_data->tzd); /*lint !e712*/
		goto cdevs_unregister;
	}

	update_debugfs(&thermal_data->ipa_sensor);
	thermal_zone_device_update(thermal_data->tzd, THERMAL_EVENT_UNSPECIFIED);

	platform_set_drvdata(pdev, thermal_data);
	thermal_data->init_flag = IPA_INIT_OK;

	return 0;

cdevs_unregister:
	cooling_device_unregister(thermal_data);
end:
	return ret;
}

static int ipa_thermal_remove(struct platform_device *pdev)
{
	struct ipa_thermal *thermal_data = platform_get_drvdata(pdev);

	if (thermal_data == NULL) {
		dev_warn(&pdev->dev, "%s sensor is null!\n", __func__);
		return -1;
	}

	thermal_zone_of_sensor_unregister(&pdev->dev, thermal_data->tzd);
	cooling_device_unregister(thermal_data);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

/*lint -e785 -esym(785,*)*/
static struct of_device_id ipa_thermal_of_match[] = {
	{ .compatible = "arm,ipa-thermal0" },
	{ .compatible = "arm,ipa-thermal1" },
	{},
};

/*lint -e785 +esym(785,*)*/

MODULE_DEVICE_TABLE(of, ipa_thermal_of_match);

static struct platform_driver ipa_thermal_platdrv = {
	.driver = {
		.name = "ipa-thermal",
		.owner = THIS_MODULE, /*lint !e64*/
		.of_match_table = ipa_thermal_of_match
	},
	.probe = ipa_thermal_probe,
	.remove = ipa_thermal_remove,
};

/*lint -e64 -e528 -esym(64,528,*)*/
module_platform_driver(ipa_thermal_platdrv);
/*lint -e64 -e528 +esym(64,528,*)*/

#ifdef CONFIG_THERMAL_SPM
static int powerhal_cfg_init(void)
{
	struct device_node *node = NULL;
	int ret;
	unsigned int data[CAPACITY_OF_ARRAY] = {0};

	node = of_find_compatible_node(NULL, NULL, "hisi,powerhal");
	if (node == NULL) {
		pr_err("%s cannot find powerhal dts.\n", __func__);
		return -ENODEV;
	}

	ret = of_property_count_u32_elems(node, "hisi,powerhal-spm-cfg");
	if (ret < 0) {
		pr_err("%s cannot find hisi,powerhal-spm-cfg.\n", __func__);
		goto node_put;
	}
	g_powerhal_actor_num = (unsigned int)ret;
	ret = of_property_read_u32_array(node, "hisi,powerhal-spm-cfg",
					 data, g_powerhal_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find hisi,powerhal-spm-cfg.\n", __func__);
		goto node_put;
	}
	ret = memcpy_s(g_powerhal_profiles[0], sizeof(g_powerhal_profiles[0]),
		       data, sizeof(g_powerhal_profiles[0]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}

	ret = of_property_read_u32_array(node, "hisi,powerhal-vr-cfg",
					 data, g_powerhal_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find hisi,powerhal-spm-cfg.\n", __func__);
		goto node_put;
	}
	ret = memcpy_s(g_powerhal_profiles[1], sizeof(g_powerhal_profiles[1]),
		       data, sizeof(g_powerhal_profiles[1]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}

	ret = of_property_read_u32_array(node, "hisi,powerhal-min-cfg",
					 data, g_powerhal_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find hisi,powerhal-spm-cfg.\n", __func__);
		goto node_put;
	}

	ret = memcpy_s(g_powerhal_profiles[2], sizeof(g_powerhal_profiles[2]),
		       data, sizeof(g_powerhal_profiles[2]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}
	return 0;
node_put:
	of_node_put(node);
	return ret;
}
#endif

unsigned int g_ipa_actor_num = 3;
unsigned int g_weights_profiles[2][CAPACITY_OF_ARRAY] = {{0}, {0}};

int ipa_weights_cfg_init(void)
{
	struct device_node *node = NULL;
	int ret;
	unsigned int data[2][CAPACITY_OF_ARRAY] = {{0}, {0}};

#if CONFIG_OF
	node = of_find_compatible_node(NULL, NULL, "hisi,weights");
	if (node == NULL) {
		pr_err("%s cannot find weights dts.\n", __func__);
		return -ENODEV;
	}

	ret = of_property_count_u32_elems(node, "hisi,weights-default-cfg");
	if (ret < 0) {
		pr_err("%s cannot find hisi,weights-default-cfg.\n", __func__);
		goto node_put;
	}
	g_ipa_actor_num = (unsigned int)ret;

	ret = of_property_read_u32_array(node, "hisi,weights-default-cfg",
					 data[0], g_ipa_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find hisi,weights-default-cfg.\n", __func__);
		goto node_put;
	}

	ret = memcpy_s(g_weights_profiles[0], sizeof(g_weights_profiles[0]),
		       data[0], sizeof(g_weights_profiles[0]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}
	ret = of_property_read_u32_array(node, "hisi,weights-boost-cfg",
					 data[1], g_ipa_actor_num);
	if (ret < 0) {
		pr_err("%s cannot find hisi,weights-boost-cfg.\n", __func__);
		goto node_put;
	} /*lint !e419*/

	ret = memcpy_s(g_weights_profiles[1], sizeof(g_weights_profiles[1]),
		       data[1], sizeof(g_weights_profiles[1]));
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		goto node_put;
	}
#endif
	return 0;
node_put:
	of_node_put(node);
	return ret;
}
EXPORT_SYMBOL(ipa_weights_cfg_init);

static void dynipa_get_weights_cfg(unsigned int *weight0, unsigned int *weight1)
{
	int ret;

	ret = memcpy_s(weight0, sizeof(*weight0) * g_ipa_actor_num,
		       g_weights_profiles[0], sizeof(*weight0) * g_ipa_actor_num);
	if (ret != EOK)
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
	ret = memcpy_s(weight1, sizeof(*weight1) * g_ipa_actor_num,
		       g_weights_profiles[1], sizeof(*weight1) * g_ipa_actor_num);
	if (ret != EOK)
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
}

#ifdef CONFIG_THERMAL_HOTPLUG
static void hotplug_check_work(struct work_struct *work)
{
	int sensor_val[CAPACITY_OF_ARRAY] = {0};
	int val = 0;
	int val_max, ret, id;
	u32 polling_delay = DEFAULT_POLL_DELAY;
	struct delayed_work *delayed_work = to_delayed_work(work);

	/* read all sensor */
	for (id = 0; id < (int)g_ipa_sensor_num; id++) {
		ret = ipa_get_sensor_value((u32)id, &val);
		sensor_val[id] = val;
		if (ret != 0)
			return;
	}

	val_max = sensor_val[0];
	for (id = 1; id < (int)g_cluster_num; id++) {
		if (sensor_val[id] > val_max)
			val_max = sensor_val[id];
	}

	thermal_hotplug_check(&val_max); /* max cluster temp */
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
	/* GPU */
	thermal_gpu_hotplug_check(&sensor_val[g_ipa_sensor_num - 1]);
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
	thermal_npu_hotplug_check(); /* NPU */
#endif
	if (val_max > g_ipa_thermal_info.hotplug.critical_cpu_up_threshold ||
	    sensor_val[g_ipa_sensor_num - 1] >
	    g_ipa_thermal_info.hotplug.critical_cpu_up_threshold)
		polling_delay = g_ipa_thermal_info.hotplug.polling_delay_passive;
	else
		polling_delay = g_ipa_thermal_info.hotplug.polling_delay;

	mod_delayed_work(system_freezable_power_efficient_wq, delayed_work,
			 msecs_to_jiffies(polling_delay));
}
#endif

#ifdef CONFIG_THERMAL_SPM
static int thermal_spm_node_create(void)
{
	int ret;

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_spm_mode);
	if (ret != 0) {
		pr_err("Spm mode create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_spm_freq);
	if (ret != 0) {
		pr_err("Spm freq create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_vr_mode);
	if (ret != 0) {
		pr_err("VR mode create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_vr_freq);
	if (ret != 0) {
		pr_err("VR freq create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.spm.device, &dev_attr_min_freq);
	if (ret != 0) {
		pr_err("Min freq create error\n");
		return ret;
	}

	return 0;
}
#endif

#ifdef CONFIG_HOTPLUG_EMULATION
static int thermal_hotplug_emulation_node_create(void)
{
	int ret;

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_hotplug_emul_temp);
	if (ret != 0) {
		pr_err("Hotplug emulation temp create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_cpu_down_threshold);
	if (ret != 0) {
		pr_err("Hotplug down_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_cpu_up_threshold);
	if (ret != 0) {
		pr_err("Hotplug up_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_critical_cpu_down_threshold);
	if (ret != 0) {
		pr_err("Hotplug critical_down_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_critical_cpu_up_threshold);
	if (ret != 0) {
		pr_err("Hotplug critical_up_threshold create error\n");
		return ret;
	}
	return 0;
}

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
static int thermal_hotplug_gpu_emulation_node_create(void)
{
	int ret;

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_gpu_hotplug_emul_temp);
	if (ret != 0) {
		pr_err("GPU hotplug emulation temp create error\n");
		return ret;
	}
	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_gpu_down_threshold);
	if (ret != 0) {
		pr_err("Hotplug gpu_down_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_gpu_up_threshold);
	if (ret != 0) {
		pr_err("Hotplug gpu_up_threshold create error\n");
		return ret;
	}

	return 0;
}
#endif

#ifdef CONFIG_NPU_HOTPLUG_EMULATION
static int thermal_hotplug_npu_emulation_node_create(void)
{
	int ret;

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_npu_hotplug_emul_temp);
	if (ret != 0) {
		pr_err("NPU hotplug emulation temp create error\n");
		return ret;
	}
	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_npu_down_threshold);
	if (ret != 0) {
		pr_err("Hotplug npu_down_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_npu_up_threshold);
	if (ret != 0) {
		pr_err("Hotplug gpu_up_threshold create error\n");
		return ret;
	}

	return 0;
}
#endif
#endif

static int thermal_init(void)
{
	int ret;

	dynipa_get_weights_cfg(g_ipa_normal_weights, g_ipa_gpu_boost_weights);

#if defined(CONFIG_THERMAL_SPM) || defined(CONFIG_THERMAL_HOTPLUG)
	g_ipa_thermal_info.thermal_class =
		class_create(THIS_MODULE, "hisi_thermal"); /*lint !e64*/
	if (IS_ERR(g_ipa_thermal_info.thermal_class)) {
		pr_err("thermal class create error\n");
		return (int)PTR_ERR(g_ipa_thermal_info.thermal_class);
	}
#endif

#ifdef CONFIG_THERMAL_SPM
	if (powerhal_cfg_init()) {
		pr_err("%s: powerhal_init error\n", __func__);
		ret = -ENODEV;
		goto class_destroy;
	}

	g_ipa_thermal_info.spm.device =
	    device_create(g_ipa_thermal_info.thermal_class, NULL, 0, NULL, "spm");
	if (IS_ERR(g_ipa_thermal_info.spm.device)) {
		pr_err("Spm device create error\n");
		ret = (int)PTR_ERR(g_ipa_thermal_info.spm.device);
		goto class_destroy;
	}

	ret = thermal_spm_node_create();
	if (ret != 0) {
		goto device_destroy;
	}
#endif

#ifdef CONFIG_THERMAL_HOTPLUG
	ret = thermal_hotplug_init();
	if (ret != 0) {
		pr_err("thermal hotplug init error\n");
#ifdef CONFIG_THERMAL_SPM
		goto device_destroy;
#else
		goto class_destroy;
#endif
	}

	INIT_DELAYED_WORK(&g_ipa_thermal_info.hotplug.poll_work, hotplug_check_work);
	g_ipa_thermal_info.hotplug.device =
		device_create(g_ipa_thermal_info.thermal_class, NULL, 0, NULL,
			      "hotplug");
	if (IS_ERR(g_ipa_thermal_info.hotplug.device)) {
		pr_err("Hotplug device create error\n");
		ret = (int)PTR_ERR(g_ipa_thermal_info.hotplug.device);
#ifdef CONFIG_THERMAL_SPM
		goto cancel_work;
#else
		cancel_delayed_work(&g_ipa_thermal_info.hotplug.poll_work);
		goto class_destroy;
#endif
	}

	ret = device_create_file(g_ipa_thermal_info.hotplug.device,
				 &dev_attr_hotplug_mode);
	if (ret != 0) {
		pr_err("Hotplug mode create error\n");
		goto cancel_work;
	}

#ifdef CONFIG_HOTPLUG_EMULATION
	ret = thermal_hotplug_emulation_node_create();
	if (ret != 0) {
		pr_err("thermal_hotplug_emulation_node_create error\n");
		goto cancel_work;
	}
#ifdef CONFIG_GPU_HOTPLUG_EMULATION
	ret = thermal_hotplug_gpu_emulation_node_create();
	if (ret != 0) {
		pr_err("thermal_hotplug_gpu_emulation_node_create error\n");
		goto cancel_work;
	}
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
	ret = thermal_hotplug_npu_emulation_node_create();
	if (ret != 0) {
		pr_err("thermal_hotplug_npu_emulation_node_create error\n");
		goto cancel_work;
	}
#endif
#endif
	mod_delayed_work(system_freezable_power_efficient_wq,
			 &g_ipa_thermal_info.hotplug.poll_work,
			 msecs_to_jiffies(DEFAULT_POLL_DELAY));
#endif
	return 0;

#if defined(CONFIG_THERMAL_SPM) || defined(CONFIG_THERMAL_HOTPLUG)
cancel_work:
	cancel_delayed_work(&g_ipa_thermal_info.hotplug.poll_work);
device_destroy:
	device_destroy(g_ipa_thermal_info.thermal_class, 0);
class_destroy:
	class_destroy(g_ipa_thermal_info.thermal_class);
	g_ipa_thermal_info.thermal_class = NULL;

	return ret;
#endif
}

static void thermal_exit(void)
{
#if defined(CONFIG_THERMAL_SPM) || defined(CONFIG_THERMAL_HOTPLUG)
	if (g_ipa_thermal_info.thermal_class) {
		device_destroy(g_ipa_thermal_info.thermal_class, 0);
		class_destroy(g_ipa_thermal_info.thermal_class);
	}
#endif
#ifdef CONFIG_THERMAL_HOTPLUG
	if (g_ipa_thermal_info.hotplug.hotplug_task) {
		kthread_stop(g_ipa_thermal_info.hotplug.hotplug_task);
		put_task_struct(g_ipa_thermal_info.hotplug.hotplug_task);
		g_ipa_thermal_info.hotplug.hotplug_task = NULL;
	}
	cancel_delayed_work(&g_ipa_thermal_info.hotplug.poll_work);
#endif
}

/*lint -e528 -esym(528,*)*/
late_initcall(thermal_init);
module_exit(thermal_exit);
/*lint -e528 +esym(528,*)*/

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("thermal ipa module driver");
