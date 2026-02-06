/*
 * its.c
 *
 * its module for component
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#include <linux/debugfs.h>
#include <linux/of.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/topology.h>
#include <soc_its_para.h>
#include <securec.h>

#ifdef CONFIG_ITS_DEBUG
#include <linux/ktime.h>
#include <linux/thermal.h>
#endif
#include <linux/lpm_thermal.h>
#ifdef CONFIG_DPM_HWMON
#include <linux/hisi/dpm_hwmon_user.h>
#endif

#define ITS_SVC_REG_RD	0xc5009901UL
#define DEFAULT_FREQ_IDX	0xFF
#define MODE_LEN	16
#define ITS_ERR	(-1)

#ifdef CONFIG_ITS_IPA
#define DPM_SWITCH_MASK	0x80000000
#define DPM_GPU_ENABLE_MASK	0x2
#endif
static DEFINE_MUTEX(g_its_lock);

struct its_power {
	bool initialized;
	int enabled;
	int polling_timeout;
	unsigned long long power[CORE_NUMBER]; /* total power */
	unsigned long long leakage[CORE_NUMBER]; /* leakage power per cycle */
	unsigned long long dynamic[CORE_NUMBER]; /* dynamic power per cycle */
	unsigned long long gpu_leakage;
	unsigned long long gpu_dynamic;
#ifdef CONFIG_ITS_IPA
	unsigned long long gpu_energy; /* last energy data */
	struct timespec last_ts; /* record interval */
#endif
	struct delayed_work work;
	struct class *its_class;
	struct device *its_device;
};

enum its_ops {
	GET_DYNAMIC_POWER_BY_CORE = 0,
	GET_DYNAMIC_POWER_BY_CLUSTER,
	SET_FREQ_CHANGE,
	GET_STATIC_POWER_BY_CORE,
	GET_TOTAL_POWER_BY_CORE,
	GET_GPU_LEAKAGE,
	FUNCTION_MAX
};

static struct its_power *g_its_power_data;
#ifdef CONFIG_ITS_DEBUG
static int g_its_debug;
#endif

noinline unsigned long atfd_its_smc(u64 _function_id, u64 _arg0, u64 _arg1,
				    u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile (
			__asmeq("%0", "x0")
			__asmeq("%1", "x1")
			__asmeq("%2", "x2")
			__asmeq("%3", "x3")
			"smc #0\n"
			: "+r" (function_id)
			: "r" (arg0), "r" (arg1), "r" (arg2));
	return (unsigned long)function_id;
}

noinline unsigned long atfd_its_ret3_smc(u64 _function_id, u64 *_arg0,
					 u64 *_arg1, u64 *_arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = *_arg0;
	register u64 arg1 asm("x2") = *_arg1;
	register u64 arg2 asm("x3") = *_arg2;
	asm volatile (
			__asmeq("%0", "x0")
			__asmeq("%1", "x1")
			__asmeq("%2", "x2")
			__asmeq("%3", "x3")
			"smc #0\n"
			: "+r" (function_id)
			: "r" (arg0), "r" (arg1), "r" (arg2));
	*_arg0 = arg0;
	*_arg1 = arg1;
	*_arg2 = arg2;
	return (unsigned long)function_id;
}

u64 freq_to_index(u64 core_idx, u64 freq)
{
	struct cpufreq_frequency_table *freq_table = NULL;
	struct cpufreq_frequency_table *pos = NULL;
	u64 freq_idx = DEFAULT_FREQ_IDX;
	u64 idx = 0;

	freq_table = cpufreq_frequency_get_table(core_idx);
	if (freq_table == NULL || freq == 0)
		return freq_idx;

	cpufreq_for_each_valid_entry(pos, freq_table) {
		if (freq == pos->frequency) {
			freq_idx = idx;
			break;
		}
		idx++;
	}

	if (freq_idx == DEFAULT_FREQ_IDX)
		return freq_idx;

	return freq_idx;
}

u64 get_cluster_core_idx(u64 cluster_idx)
{
	u64 core_idx = 0;
	unsigned int i;

	for (i = 0; i < CORE_NUMBER; i++) {
		if (cluster_idx == g_its_core_para[i].cluster_idx) {
			core_idx = (u64)i;
			break;
		}
	}

	return core_idx;
}

static unsigned long long hisi_sec_its_set_window_by_core(u64 core_idx, u64 old_freq,
						   u64 freq)
{
	u64 freq_idx, old_freq_idx;

	if (!g_its_power_data->initialized)
		return 0;

	old_freq_idx = freq_to_index(core_idx, old_freq);
	freq_idx = freq_to_index(core_idx, freq);
	if (freq_idx == DEFAULT_FREQ_IDX || old_freq_idx == DEFAULT_FREQ_IDX)
		return 0;

	return atfd_its_smc(ITS_SVC_REG_RD, SET_FREQ_CHANGE,
			    (old_freq_idx << ITS_OLD_FREQ_IDX_START) |
			    (freq_idx << ITS_NEW_FREQ_IDX_START) | core_idx,
			    ((old_freq / 1000) << ITS_OLD_FREQ_START) |
			    (freq / 1000));
}

static int hisi_sec_its_get_total_power(u64 core_idx, u64 freq, u64 *dynamic_power,
				 u64 *static_power)
{
	int ret;
	u64 x1, x2, x3, freq_idx;

	if (!g_its_power_data->initialized) {
		pr_err("%s: initialized not ready\n", __func__);
		return ITS_ERR;
	}

	freq_idx = freq_to_index(core_idx, freq);

	x1 = GET_TOTAL_POWER_BY_CORE;
	x2 = (freq_idx << ITS_NEW_FREQ_IDX_START) | core_idx;
	x3 = freq / 1000;
	ret = atfd_its_ret3_smc(ITS_SVC_REG_RD, &x1, &x2, &x3);
	if (ret != 0)
		return ret;

	*dynamic_power = x1;
	*static_power = x2;

	return 0;
}

#ifdef CONFIG_ITS_IPA
static int hisi_sec_its_get_gpu_leakage(u64 *static_power)
{
	int ret;
	u64 x1, x2, x3;

	if (!g_its_power_data->initialized) {
		pr_err("%s: initialized not ready\n", __func__);
		return ITS_ERR;
	}

	x1 = GET_GPU_LEAKAGE;
	x2 = 0;
	x3 = 0;
	ret = atfd_its_ret3_smc(ITS_SVC_REG_RD, &x1, &x2, &x3);
	if (ret != 0)
		return ret;

	*static_power = x1;

	return 0;
}
#endif

static int its_cpufreq_notifier(struct notifier_block *nb,
				unsigned long event, void *data)
{
	struct cpufreq_freqs *freq = data;

	if (event != CPUFREQ_PRECHANGE)
		return NOTIFY_DONE;

	if (freq->new != freq->old)
		hisi_sec_its_set_window_by_core(freq->cpu, freq->old, freq->new);

	return NOTIFY_OK;
}

/* Notifier for cpufreq transition change */
static struct notifier_block its_cpufreq_notifier_block = {
	.notifier_call = its_cpufreq_notifier,
};

/* when dubai call IOCTL, set power result to zero */
int reset_power_result(void)
{
	int ret = 0;

	if (g_its_power_data == NULL || !g_its_power_data->initialized)
		return -EINVAL;

	ret = memset_s(g_its_power_data->power,
		       CORE_NUMBER * sizeof(unsigned long long),
		       0, CORE_NUMBER * sizeof(unsigned long long));
	if (ret != EOK)
		pr_err("%s memset failed\n", __func__);

	return ret;
}
EXPORT_SYMBOL(reset_power_result);

/* when dubai call IOCTL, get power result */
int get_its_power_result(its_cpu_power_t *result)
{
	int ret = 0;
	int i;

	if (g_its_power_data == NULL || !g_its_power_data->initialized ||
	    result == NULL)
		return -EFAULT;

	for (i = 0; i < CORE_NUMBER; i++)
		result->power[i] = g_its_power_data->power[i];

	return ret;
}
EXPORT_SYMBOL(get_its_power_result);

int get_its_core_dynamic_power(int core, unsigned long long *power)
{
	if (power == NULL || g_its_power_data == NULL ||
	    !g_its_power_data->initialized)
		return -EFAULT;

	*power = 0;
	if (core >= 0 && core < CORE_NUMBER)
		*power = g_its_power_data->dynamic[core];

	return 0;
}
EXPORT_SYMBOL(get_its_core_dynamic_power);

int get_its_core_leakage_power(int core, unsigned long long *power)
{
	if (power == NULL || g_its_power_data == NULL ||
	    !g_its_power_data->initialized)
		return -EFAULT;

	*power = 0;
	if (core >= 0 && core < CORE_NUMBER)
		*power = g_its_power_data->leakage[core];

	return 0;
}
EXPORT_SYMBOL(get_its_core_leakage_power);

bool check_its_enabled(void)
{
	if (g_its_power_data == NULL)
		return false;

	if (g_its_power_data->enabled != 0 && g_its_power_data->initialized)
		return true;

	return false;
}
EXPORT_SYMBOL(check_its_enabled);

int get_gpu_dynamic_power(unsigned long long *power)
{
	if (power == NULL || g_its_power_data == NULL)
		return -EFAULT;

	*power = g_its_power_data->gpu_dynamic;

	return 0;
}
EXPORT_SYMBOL(get_gpu_dynamic_power);

int get_gpu_leakage_power(unsigned long long *power)
{
	if (power == NULL || g_its_power_data == NULL)
		return -EFAULT;

	*power = g_its_power_data->gpu_leakage;

	return 0;
}
EXPORT_SYMBOL(get_gpu_leakage_power);

#ifdef CONFIG_ITS_DEBUG
static void print_its_power_info(void)
{
	struct timespec ts = (struct timespec){0, 0};
	struct thermal_zone_device *temptz = NULL;
	int temp0 = 0;
	int temp1 = 0;
	int temp2 = 0;

	temptz = thermal_zone_get_zone_by_name("cluster0");
	(void)thermal_zone_get_temp(temptz, &temp0);
	temptz = thermal_zone_get_zone_by_name("cluster1");
	(void)thermal_zone_get_temp(temptz, &temp1);
	temptz = thermal_zone_get_zone_by_name("cluster2");
	(void)thermal_zone_get_temp(temptz, &temp2);

	getnstimeofday(&ts);

	pr_err("Time:%ld us UTC, temp:%d,%d,%d, power:%llu, %llu, %llu, %llu,"
		" %llu, %llu, %llu, %llu\n",
		ts.tv_sec * 1000000 + (ts.tv_nsec / 1000),
		temp0, temp1, temp2,
		g_its_power_data->power[0], g_its_power_data->power[1],
		g_its_power_data->power[2], g_its_power_data->power[3],
		g_its_power_data->power[4], g_its_power_data->power[5],
		g_its_power_data->power[6], g_its_power_data->power[7]);
	pr_err("detail power: %llu + %llu, %llu + %llu, %llu + %llu, %llu + %llu,"
		" %llu + %llu, %llu + %llu, %llu + %llu, %llu + %llu\n",
		g_its_power_data->dynamic[0], g_its_power_data->leakage[0],
		g_its_power_data->dynamic[1], g_its_power_data->leakage[1],
		g_its_power_data->dynamic[2], g_its_power_data->leakage[2],
		g_its_power_data->dynamic[3], g_its_power_data->leakage[3],
		g_its_power_data->dynamic[4], g_its_power_data->leakage[4],
		g_its_power_data->dynamic[5], g_its_power_data->leakage[5],
		g_its_power_data->dynamic[6], g_its_power_data->leakage[6],
		g_its_power_data->dynamic[7], g_its_power_data->leakage[7]);
#ifdef CONFIG_ITS_IPA
	pr_err("gpu power: %llu + %llu\n", g_its_power_data->gpu_dynamic,
		g_its_power_data->gpu_leakage);
#endif
}
#endif

#ifdef CONFIG_ITS_IPA
static void update_gpu_dynamic(void)
{
	unsigned long long gpu_energy;
	long time_interval;
	struct timespec ts = (struct timespec){0, 0};

	getnstimeofday(&ts);
	/* let GPU update energy data first */
	update_dpm_power(DPM_GPU_ID);
	/* get energy result */
	gpu_energy = get_dpm_chdmod_power(DPM_GPU_ID);
	/* interval ms */
	time_interval = ((ts.tv_sec - g_its_power_data->last_ts.tv_sec) * 1000) +
			((ts.tv_nsec - g_its_power_data->last_ts.tv_nsec) / 1000000);

	/* calculate energy diff to power */
	if (time_interval <= 0 ||
	    gpu_energy <= g_its_power_data->gpu_energy)
		g_its_power_data->gpu_dynamic = 0;
	else
		g_its_power_data->gpu_dynamic =
			(gpu_energy - g_its_power_data->gpu_energy) /
			(time_interval * 1000);
	/* save new energy data */
	g_its_power_data->gpu_energy = gpu_energy;
	g_its_power_data->last_ts.tv_sec = ts.tv_sec;
	g_its_power_data->last_ts.tv_nsec = ts.tv_nsec;
}

static void update_gpu_power(void)
{
	int ret;
	u64 gpu_leakage = 0;

	update_gpu_dynamic();

	ret = hisi_sec_its_get_gpu_leakage(&gpu_leakage);
	if (ret != 0)
		g_its_power_data->gpu_leakage = 0;
	else
		g_its_power_data->gpu_leakage = gpu_leakage;
}
#endif

static void set_its_mode(int enable)
{
	if (!g_its_power_data->initialized)
		return;

	mutex_lock(&g_its_lock);
	pr_info("%s set mode %d\n", __func__, enable);
	g_its_power_data->enabled = enable;
	if (enable != 0) {
		queue_delayed_work(system_freezable_power_efficient_wq,
				   &g_its_power_data->work,
				   msecs_to_jiffies(g_its_power_data->polling_timeout));
#ifdef CONFIG_ITS_IPA
		update_gpu_dynamic();
#endif
	} else {
		cancel_delayed_work(&g_its_power_data->work);
	}
	mutex_unlock(&g_its_lock);
}

static void its_dubai_getpower(struct work_struct *work)
{
	int idx, ret;
	unsigned long freq;
	u64 static_power = 0;
	u64 dynamic_power = 0;

	if (g_its_power_data->enabled == 0)
		return;

	for (idx = 0; idx < (int)nr_cpu_ids; idx++) {
		freq = cpufreq_quick_get(idx);
		/* total power = dynamic_power + static_power */
		if (freq > 0)
			ret = hisi_sec_its_get_total_power(idx, freq,
							   &dynamic_power,
							   &static_power);
		else
			ret = ITS_ERR;

		if (ret != 0) {
			g_its_power_data->leakage[idx] = 0;
			g_its_power_data->dynamic[idx] = 0;
		} else {
			g_its_power_data->leakage[idx] = static_power;
			g_its_power_data->dynamic[idx] = dynamic_power;
			if (ULONG_MAX - (dynamic_power + static_power) >
			    g_its_power_data->power[idx])
				g_its_power_data->power[idx] +=
					(dynamic_power + static_power);
			else
				g_its_power_data->power[idx] =
					(dynamic_power + static_power);
		}
	}
#ifdef CONFIG_ITS_IPA
	update_gpu_power();
#endif

#ifdef CONFIG_ITS_DEBUG
	if (g_its_debug != 0)
		print_its_power_info();
#endif

	if (g_its_power_data->polling_timeout != 0)
		queue_delayed_work(system_freezable_power_efficient_wq,
				   &g_its_power_data->work,
				   msecs_to_jiffies(g_its_power_data->polling_timeout));
	else
		cancel_delayed_work(&g_its_power_data->work);
}

static ssize_t
its_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	return snprintf_s(buf, MODE_LEN, MODE_LEN - 1, "%d\n",
			  g_its_power_data->enabled);
}

static ssize_t
its_mode_store(struct device *dev, struct device_attribute *attr,
	       const char *buf, size_t count)
{
	int value = 0;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (kstrtoint(buf, 10, &value) != 0)
		return -EINVAL;

	set_its_mode(value);

	return (ssize_t)count;
}

static DEVICE_ATTR(its_mode, 0644, its_mode_show, its_mode_store);

#ifdef CONFIG_ITS_DEBUG
static ssize_t
its_debug_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	return snprintf_s(buf, MODE_LEN,  MODE_LEN - 1, "%d\n", g_its_debug);
}

static ssize_t
its_debug_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int value;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (kstrtoint(buf, 10, &value) != 0)
		return -EINVAL;

	g_its_debug = value;

	return (ssize_t)count;
}

static DEVICE_ATTR(its_debug, 0644, its_debug_show, its_debug_store);

static ssize_t
its_polling_timeout_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	return snprintf_s(buf, MODE_LEN, MODE_LEN - 1, "%d\n",
			  g_its_power_data->polling_timeout);
}

static ssize_t
its_polling_timeout_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int value = 0;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (kstrtoint(buf, 10, &value) != 0)
		return -EINVAL;

	mutex_lock(&g_its_lock);
	g_its_power_data->polling_timeout = value;

	if (g_its_power_data->polling_timeout != 0)
		queue_delayed_work(system_freezable_power_efficient_wq,
				   &g_its_power_data->work,
				   msecs_to_jiffies(g_its_power_data->polling_timeout));
	else
		cancel_delayed_work(&g_its_power_data->work);

	mutex_unlock(&g_its_lock);

	return (ssize_t)count;
}
static DEVICE_ATTR(polling_timeout, 0660, its_polling_timeout_show,
		   its_polling_timeout_store);
#endif

static int hisi_its_init(void)
{
	int ret;
	struct device_node *np = NULL;

	g_its_power_data = kzalloc(sizeof(struct its_power), GFP_KERNEL);
	if (g_its_power_data == NULL)
		return -ENOMEM;

	np = of_find_node_by_name(NULL, "its_config");
	if (np == NULL) {
		pr_err("its_config node not found\n");
		ret = -ENODEV;
		goto out_find_node;
	}

	ret = of_property_read_u32(np, "hisilicon,its_polling_timeout",
				   (u32 *)&g_its_power_data->polling_timeout);
	if (ret != 0) {
		pr_err("%s its_polling_timeout read err\n", __func__);
		goto node_put;
	}

	ret = cpufreq_register_notifier(&its_cpufreq_notifier_block,
					CPUFREQ_TRANSITION_NOTIFIER);
	if (ret != 0) {
		pr_err("%s: register cpufreq notifier fail\n", __func__);
		goto register_err;
	}

	g_its_power_data->its_class = class_create(THIS_MODULE, "its");
	if (IS_ERR(g_its_power_data->its_class)) {
		pr_err("hisi its class create error\n");
		goto register_err;
	}

	g_its_power_data->its_device =
		device_create(g_its_power_data->its_class, NULL, 0, NULL, "its");
	if (IS_ERR(g_its_power_data->its_device)) {
		pr_err("its device create error\n");
		ret = (int)PTR_ERR(g_its_power_data->its_device);
		goto class_destroy;
	}

	ret = device_create_file(g_its_power_data->its_device, &dev_attr_its_mode);
	if (ret != 0) {
		pr_err("its mode create error\n");
		goto device_destroy;
	}

#ifdef CONFIG_ITS_DEBUG
	ret = device_create_file(g_its_power_data->its_device,
				 &dev_attr_polling_timeout);
	if (ret != 0) {
		pr_err("polling timeout create error\n");
		goto device_remove_file;
	}
	ret = device_create_file(g_its_power_data->its_device, &dev_attr_its_debug);
	if (ret != 0) {
		pr_err("its_debug create error\n");
		goto device_remove_file1;
	}
#endif

	g_its_power_data->initialized = true;
	g_its_power_data->enabled = 1;
	/* workqueue to calc power for dubai */
	if (g_its_power_data->polling_timeout != 0) {
		INIT_DEFERRABLE_WORK(&g_its_power_data->work, its_dubai_getpower);
		queue_delayed_work(system_freezable_power_efficient_wq,
				   &g_its_power_data->work,
				   msecs_to_jiffies(g_its_power_data->polling_timeout));
	}
	of_node_put(np);
	return 0;

#ifdef CONFIG_ITS_DEBUG
device_remove_file1:
	device_remove_file(g_its_power_data->its_device, &dev_attr_polling_timeout);
device_remove_file:
	device_remove_file(g_its_power_data->its_device, &dev_attr_its_mode);
#endif
device_destroy:
	device_destroy(g_its_power_data->its_class, 0);
class_destroy:
	class_destroy(g_its_power_data->its_class);
	g_its_power_data->its_class = NULL;
register_err:
	cancel_delayed_work(&g_its_power_data->work);
node_put:
	of_node_put(np);
out_find_node:
	kfree(g_its_power_data);
	g_its_power_data = NULL;
	return ret;
}

static void hisi_its_exit(void)
{
	if (g_its_power_data != NULL) {
#ifdef CONFIG_ITS_DEBUG
		device_remove_file(g_its_power_data->its_device, &dev_attr_its_debug);
		device_remove_file(g_its_power_data->its_device, &dev_attr_polling_timeout);
#endif
		device_remove_file(g_its_power_data->its_device, &dev_attr_its_mode);
		device_destroy(g_its_power_data->its_class, 0);
		class_destroy(g_its_power_data->its_class);
		g_its_power_data->its_class = NULL;
		cancel_delayed_work(&g_its_power_data->work);
		kfree(g_its_power_data);
		g_its_power_data = NULL;
	}
}

module_init(hisi_its_init);
module_exit(hisi_its_exit);
