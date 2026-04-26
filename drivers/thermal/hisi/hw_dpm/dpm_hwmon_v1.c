/*
 * dpm_hwmon_v1.c
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

#include "dpm_hwmon_v1.h"
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/time.h>

#define DPMONITOR_SIGNAL_MODE_ADDR(base)		((base) + 0xE4)
#define DPMONITOR_CFG_TIME_ADDR(base)		((base) + 0x004)
#define DPMONITOR_CTRL_EN_ADDR(base)		((base) + 0x000)
#define DPMONITOR_SENSOR_EN_ADDR(base)		((base) + 0x008)
#define DPMONITOR_SOFT_SAMPLE_PULSE_ADDR(base)	((base) + 0x0E8)
#define DPMONITOR_SENSOR_DIS_ADDR(base)		((base) + 0x00C)
#define DPMONITOR_LOW_LOAD0_ADDR(base)		((base) + 0x014)
#define UINT64_MAX		0xffffffffffffffff

static DEFINE_MUTEX(g_dpm_hwmon_ops_lock);

LIST_HEAD(g_dpm_hwmon_ops_list);
DEFINE_MUTEX(g_dpm_hwmon_ops_list_lock);

/* common module start */
static void free_dpm_ops_mem(struct dpm_hwmon_ops *dpm_ops)
{
	if (dpm_ops->dpm_counter_table != NULL) {
		kfree(dpm_ops->dpm_counter_table);
		dpm_ops->dpm_counter_table = NULL;
	}
	if (dpm_ops->last_dpm_counter != NULL) {
		kfree(dpm_ops->last_dpm_counter);
		dpm_ops->last_dpm_counter = NULL;
	}
	if (dpm_ops->dpm_fitting_table != NULL) {
		kfree(dpm_ops->dpm_fitting_table);
		dpm_ops->dpm_fitting_table = NULL;
	}
}

int dpm_hwmon_register(struct dpm_hwmon_ops *dpm_ops)
{
	struct dpm_hwmon_ops *pos = NULL;
	int ret = -1;
	const int qword_size = sizeof(unsigned long long);

	if (dpm_ops == NULL || dpm_ops->dpm_module_id < 0 ||
	    dpm_ops->dpm_module_id >= DPM_MODULE_NUM) {
		pr_err("%s LINE %d dpm_ops is invalid\n", __func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&g_dpm_hwmon_ops_list_lock);
	list_for_each_entry(pos, &g_dpm_hwmon_ops_list, ops_list) {
		if (dpm_ops->dpm_module_id == pos->dpm_module_id) {
			pr_err("%s LINE %d dpm module %d has registered\n",
			       __func__, __LINE__, dpm_ops->dpm_module_id);
			mutex_unlock(&g_dpm_hwmon_ops_list_lock);
			return ret;
		}
	}
	list_add(&dpm_ops->ops_list, &g_dpm_hwmon_ops_list);
	mutex_unlock(&g_dpm_hwmon_ops_list_lock);

	if ((dpm_ops->dpm_cnt_len > 0 && dpm_ops->dpm_cnt_len < DPM_BUFFER_SIZE) &&
	    (dpm_ops->dpm_fit_len > 0 && dpm_ops->dpm_fit_len < DPM_BUFFER_SIZE)) {
		dpm_ops->dpm_counter_table = kzalloc(qword_size * dpm_ops->dpm_cnt_len,
						     GFP_KERNEL);
		dpm_ops->last_dpm_counter = kzalloc(qword_size * dpm_ops->dpm_cnt_len,
						    GFP_KERNEL);
		dpm_ops->dpm_fitting_table = kzalloc(sizeof(int) * dpm_ops->dpm_fit_len,
						     GFP_KERNEL);
		if (dpm_ops->dpm_counter_table == NULL ||
		    dpm_ops->last_dpm_counter == NULL ||
		    dpm_ops->dpm_fitting_table == NULL) {
			ret = -ENOMEM;
			goto err_handler;
		}

	} else {
		return -EINVAL;
	}

	if (dpm_ops->hi_dpm_fitting_coff != NULL) {
		ret = dpm_ops->hi_dpm_fitting_coff();
		if (ret < 0)
			goto err_handler;
	}
	pr_err("dpm hwmon %d register!\n", dpm_ops->dpm_module_id);
	return 0;

err_handler:
	ret = dpm_hwmon_unregister(dpm_ops);
	if (ret < 0)
		pr_err("%s LINE %d register fail", __func__, __LINE__);
	return ret;
}

int dpm_hwmon_unregister(struct dpm_hwmon_ops *dpm_ops)
{
	struct dpm_hwmon_ops *pos = NULL;
	struct dpm_hwmon_ops *tmp = NULL;

	if (dpm_ops == NULL) {
		pr_err("%s LINE %d dpm_ops is NULL\n", __func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&g_dpm_hwmon_ops_list_lock);
	list_for_each_entry_safe(pos, tmp, &g_dpm_hwmon_ops_list, ops_list) {
		if (dpm_ops->dpm_module_id == pos->dpm_module_id) {
			free_dpm_ops_mem(pos);
			list_del_init(&pos->ops_list);
			break;
		}
	}
	mutex_unlock(&g_dpm_hwmon_ops_list_lock);
	return 0;
}
/* common module end */

static unsigned long long get_timer_value(void)
{
	unsigned long long total_us;
	struct timespec ts = (struct timespec){0, 0};

	getnstimeofday(&ts);
	total_us = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

	return total_us;
}

void dpm_monitor_enable(void __iomem *module_dpm_addr,
			unsigned int dpmonitor_signal_mode)
{
	/* step 5: module signal mode */
	writel(dpmonitor_signal_mode, DPMONITOR_SIGNAL_MODE_ADDR(module_dpm_addr));
	/* step 6: module cfg time */
	writel(0xffffffff, DPMONITOR_CFG_TIME_ADDR(module_dpm_addr));
	/* step 7: low power enable */
	writel(0x00080008, DPMONITOR_CTRL_EN_ADDR(module_dpm_addr));
	/* step 8: enable controller */
	writel(0x00010001, DPMONITOR_CTRL_EN_ADDR(module_dpm_addr));
	/* step 9: enable tsensor */
	writel(0x0000FFFF, DPMONITOR_SENSOR_EN_ADDR(module_dpm_addr));
}

void dpm_monitor_disable(void __iomem *module_dpm_addr)
{
	/* step 1: disable tsensor */
	writel(0xFFFFFFFF, DPMONITOR_SENSOR_DIS_ADDR(module_dpm_addr));
	/* step 2: disable controller */
	writel(0x00010000, DPMONITOR_CTRL_EN_ADDR(module_dpm_addr));
}

static long long get_first_level_power(struct dpm_hwmon_ops *pos)
{
	unsigned int  i;
	unsigned long long counter, counter_diff;
	long long dpm_power = 0;

	for (i = 0; (i < pos->dpm_cnt_len) && (i < pos->dpm_fit_len); i++) {
		counter = pos->dpm_counter_table[i];
		if (counter < pos->last_dpm_counter[i])
			counter_diff = UINT64_MAX - pos->last_dpm_counter[i] +
				       counter;
		else
			counter_diff = counter - pos->last_dpm_counter[i];

		dpm_power += (counter_diff * (long long)(pos->dpm_fitting_table[i]));
		pos->last_dpm_counter[i] = counter;
	}
	return dpm_power;
}

static long long get_compensate_power(struct dpm_hwmon_ops *pos)
{
	unsigned long long time;
	long long voltage, compensate;

	time = get_timer_value();
	voltage = (long long)(pos->hi_dpm_get_voltage());
	if (voltage > 0)
		compensate = (long long)(pos->compensate_coff) * voltage *
					 voltage * ((time - pos->last_time) / 1000);
	else
		compensate = 0;
	pos->last_time = time;
	return compensate;
}

unsigned long long get_dpm_power(struct dpm_hwmon_ops *pos)
{
	long long dpm_power = 0;

	if (pos != NULL) {
		mutex_lock(&g_dpm_hwmon_ops_lock);
		dpm_power = get_first_level_power(pos) +
			    get_compensate_power(pos);
		mutex_unlock(&g_dpm_hwmon_ops_lock);
	}
	return (unsigned long long)dpm_power;
}

void dpm_enable_module(struct dpm_hwmon_ops *pos, bool dpm_switch)
{
	/* no module_enabled in v1 */
}

bool get_dpm_enabled(struct dpm_hwmon_ops *pos)
{
	/* V1 default enabled */
	return true;
}

void dpm_sample(struct dpm_hwmon_ops *pos)
{
	if (pos == NULL)
		return;
	if (pos->hi_dpm_update_counter() < 0)
		pr_err("DPM %d sample fail\n", pos->dpm_module_id);
}
