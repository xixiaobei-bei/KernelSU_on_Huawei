/*
 * shell_temp.c
 *
 * shell temp calculation
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

#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <asm/page.h>
#include "peripheral_tm.h"
#include <linux/lpm_thermal.h>
#include <linux/power/hisi/coul/coul_drv.h>

#define CREATE_TRACE_POINTS

#include <trace/events/shell_temp.h>
#include <securec.h>
#define MUTIPLY_FACTOR		100000
#define DEFAULT_SHELL_TEMP	25000
#define ERROR_SHELL_TEMP	125000
#define SENSOR_PARA_COUNT	3
#define BATTERY_NAME		"Battery"
#define INVALID_TEMP_OUT_OF_RANGE	1
#define DEFAULT_TSENS_MAX_TEMP		100000
#define DEFAULT_TSENS_MIN_TEMP		0
#define DEFAULT_TSENS_STEP_RANGE	4000
#define DEFAULT_NTC_MAX_TEMP		80000
#define DEFAULT_NTC_MIN_TEMP		(-20000)
#define DEFAULT_NTC_STEP_RANGE		2000
#define DEFAULT_SHELL_TEMP_STEP_RANGE	400
#define DEFAULT_SHELL_TEMP_STEP		200
#define HAVE_INVALID_SENSOR_TEMP	1
#define CHECK_TSENS			0x80
#define CHECK_NTC			0x7f

enum {
	TYPE_TSENS = 0x80,
	TYPE_PERIPHERAL = 0x01,
	TYPE_BATTERY = 0x02,
	TYPE_TERMINAL = 0x04,
	TYPE_UNKNOWN = 0x0,
};

struct temp_tracing_t {
	int temp;
	int coef;
	int temp_invalid_flag;
};

struct shell_sensor_t {
	const char *sensor_name;
	u32 sensor_type;
	struct temp_tracing_t temp_tracing[0];
};

struct temperature_node_t {
	struct device *device;
	int ambient;
};

struct hw_thermal_class {
	struct class *thermal_class;
	struct temperature_node_t temperature_node;
};

struct hw_thermal_class hw_thermal_info;

struct shell_t {
	int sensor_count;
	int sample_count;
	int tsensor_temp_step;
	int tsensor_max_temp;
	int tsensor_min_temp;
	int ntc_temp_step;
	int ntc_max_temp;
	int ntc_min_temp;
	int shell_temp_step_range;
	int shell_temp_step;
	u32 interval;
	int bias;
	int temp;
	int old_temp;
	int index;
	int valid_flag;
	int is_framework;
#ifdef CONFIG_SHELL_TEMP_DEBUG
	int channel;
	int debug_temp;
#endif
	struct thermal_zone_device *tz_dev;
	struct delayed_work work;
	struct shell_sensor_t shell_sensor[0];
};

static ssize_t
shell_show_temp(struct device *dev, struct device_attribute *devattr,
		char *buf)
{
	struct shell_t *shell = NULL;

	if (dev == NULL || devattr == NULL || buf == NULL)
		return 0;

	if (dev->driver_data == NULL)
		return 0;

	shell = dev->driver_data;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			  shell->temp);
}

#define IS_FRAMEWORK 1
#define MIN_TEMPERATURE (-40000)
#define MAX_TEMPERATURE 145000
#define DECIMAL 10
static ssize_t
shell_store_temp(struct device *dev, struct device_attribute *devattr,
			const char *buf, size_t count)
{
	int temp;
	struct platform_device *pdev = NULL;
	struct shell_t *shell = NULL;

	if (dev == NULL || devattr == NULL || buf == NULL)
		return 0;

	if (kstrtoint(buf, DECIMAL, &temp) != 0) {
		pr_err("%s Invalid input para\n", __func__);
		return -EINVAL;
	}

	if (temp < MIN_TEMPERATURE || temp > MAX_TEMPERATURE)
		return -EINVAL;

	pdev = container_of(dev, struct platform_device, dev);
	shell = platform_get_drvdata(pdev);
	if (shell->is_framework != IS_FRAMEWORK)
		return -EINVAL;

	shell->temp = temp;
	return (ssize_t)count;
}
static DEVICE_ATTR(temp, S_IWUSR | S_IRUGO,
		shell_show_temp, shell_store_temp);

#ifdef CONFIG_SHELL_TEMP_DEBUG
static ssize_t
shell_store_debug_temp(struct device *dev, struct device_attribute *devattr,
			    const char *buf, size_t count)
{
	int channel, temp;
	struct platform_device *pdev = NULL;
	struct shell_t *shell = NULL;

	if (dev == NULL || devattr == NULL || buf == NULL)
		return 0;

	if (sscanf_s(buf, "%d %d\n", &channel, &temp) != 2) {
		pr_err("%s Invalid input para\n", __func__);
		return -EINVAL;
	}

	pdev = container_of(dev, struct platform_device, dev);
	shell = platform_get_drvdata(pdev);

	shell->channel = channel;
	shell->debug_temp = temp;

	return (ssize_t)count;
}
static DEVICE_ATTR(debug_temp, S_IWUSR, NULL, shell_store_debug_temp);
#endif

static struct attribute *shell_attributes[] = {
	&dev_attr_temp.attr,
#ifdef CONFIG_SHELL_TEMP_DEBUG
	&dev_attr_debug_temp.attr,
#endif
	NULL
};

static struct attribute_group shell_attribute_group = {
	.attrs = shell_attributes,
};

static BLOCKING_NOTIFIER_HEAD(ambient_chain_head);
int register_ambient_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&ambient_chain_head, nb);
}
EXPORT_SYMBOL_GPL(register_ambient_notifier);

int unregister_ambient_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&ambient_chain_head, nb);
}
EXPORT_SYMBOL_GPL(unregister_ambient_notifier);

int ambient_notifier_call_chain(int val)
{
	return blocking_notifier_call_chain(&ambient_chain_head, 0, &val);
}

static ssize_t show_ambient(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			  (int)hw_thermal_info.temperature_node.ambient);
}

static ssize_t store_ambient(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	int temp = 0;
	int prev_temp;

	if (dev == NULL || attr == NULL || buf == NULL)
		return 0;

	if (kstrtoint(buf, 10, &temp)) /*lint !e64*/
		return -EINVAL;

	prev_temp = hw_thermal_info.temperature_node.ambient;
	if (temp != prev_temp) {
		hw_thermal_info.temperature_node.ambient = temp;
		ambient_notifier_call_chain(temp);
	}

	return (ssize_t)count;
}

static DEVICE_ATTR(ambient, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP,
		   show_ambient, store_ambient);

int get_shell_temp(struct thermal_zone_device *thermal, int *temp)
{
	struct shell_t *shell = thermal->devdata;

	if (shell == NULL || temp == NULL)
		return -EINVAL;

	*temp = shell->temp;

	return 0;
}

/*lint -e785*/
struct thermal_zone_device_ops shell_thermal_zone_ops = {
	.get_temp = get_shell_temp,
};

/*lint +e785*/

static int calc_shell_temp(struct shell_t *shell)
{
	int i, j, k;
	struct shell_sensor_t *shell_sensor = NULL;
	long sum = 0;

	for (i = 0; i < shell->sensor_count; i++) {
		shell_sensor = (struct shell_sensor_t *)(uintptr_t)
			((u64)(uintptr_t)(shell->shell_sensor) +
			 (u64)((long)i) * (sizeof(struct shell_sensor_t) +
			 sizeof(struct temp_tracing_t) *
			 (u64)((long)shell->sample_count)));
		for (j = 0; j < shell->sample_count; j++) {
			k = (shell->index - j) <  0 ?
			     ((shell->index - j) + shell->sample_count) :
			     shell->index - j;
			sum += (long)shell_sensor->temp_tracing[j].coef *
				(long)shell_sensor->temp_tracing[k].temp;
			trace_calc_shell_temp(shell->tz_dev, i, j,
					      shell_sensor->temp_tracing[j].coef,
					      shell_sensor->temp_tracing[k].temp,
					      sum);
		}
	}

	sum += ((long)shell->bias * 1000L);

	return (int)(sum / MUTIPLY_FACTOR);
}

static int hkadc_handle_temp_data(struct shell_t *shell,
				  struct shell_sensor_t *shell_sensor,
				  int temp)
{
	int ret = 0;
	int old_index, old_temp, diff;

	old_index = (shell->index - 1) <  0 ?
		     ((shell->index - 1) + shell->sample_count) :
		     (shell->index - 1);
	old_temp = shell_sensor->temp_tracing[old_index].temp;
	diff = (temp - old_temp) < 0 ? (old_temp - temp) : (temp - old_temp);
	diff = (u32)diff / shell->interval * 1000;

	shell_sensor->temp_tracing[shell->index].temp_invalid_flag = 0;
	if (shell->valid_flag || shell->index) {
		if (shell_sensor->sensor_type & CHECK_TSENS &&
		    diff > shell->tsensor_temp_step) {
			pr_err("%s %s sensor_name = %s tsensor_temp_step_out_of_range\n",
			       __func__, shell->tz_dev->type,
			       shell_sensor->sensor_name);
			shell_sensor->temp_tracing[shell->index].temp = old_temp;
		} else if (shell_sensor->sensor_type & CHECK_NTC &&
			   diff > shell->ntc_temp_step) {
			pr_err("%s %s sensor_name = %s ntc_temp_step_out_of_range\n",
			       __func__, shell->tz_dev->type,
			       shell_sensor->sensor_name);
			shell_sensor->temp_tracing[shell->index].temp = old_temp;
		}
	}

	if (shell_sensor->sensor_type & CHECK_TSENS) {
		if (temp > shell->tsensor_max_temp ||
		    temp < shell->tsensor_min_temp) {
			pr_err("%s %s sensor_name = %s sensor_temp = %d "
			       "tsensor_temp_out_of_range\n", __func__,
			       shell->tz_dev->type,
			       shell_sensor->sensor_name, temp);
			shell_sensor->temp_tracing[shell->index].temp_invalid_flag =
				INVALID_TEMP_OUT_OF_RANGE;
			ret = HAVE_INVALID_SENSOR_TEMP;
		}
	} else if (shell_sensor->sensor_type & CHECK_NTC) {
		if (temp > shell->ntc_max_temp ||
		    temp < shell->ntc_min_temp) {
			pr_err("%s %s sensor_name = %s sensor_temp = %d "
			       "ntc_temp_out_of_range\n", __func__,
			       shell->tz_dev->type,
			       shell_sensor->sensor_name, temp);
			shell_sensor->temp_tracing[shell->index].temp_invalid_flag =
				INVALID_TEMP_OUT_OF_RANGE;
			ret = HAVE_INVALID_SENSOR_TEMP;
		}
	}

	return ret;
}

static int calc_sensor_temp_avg(struct shell_t *shell, int *tsensor_avg,
				int *ntc_avg)
{
	int i;
	int tsensor_good_count = 0;
	int ntc_good_count = 0;
	int sum_tsensor_temp = 0;
	int sum_ntc_temp = 0;
	bool have_tsensor = false;
	bool have_ntc = false;
	struct shell_sensor_t *shell_sensor = NULL;

	for (i = 0; i < shell->sensor_count; i++) {
		shell_sensor = (struct shell_sensor_t *)(uintptr_t)
			((u64)(uintptr_t)(shell->shell_sensor) +
			 (u64)((long)i) * (sizeof(struct shell_sensor_t) +
			 sizeof(struct temp_tracing_t) *
			 (u64)((long)shell->sample_count)));
		if (shell_sensor->sensor_type & CHECK_TSENS) {
			have_tsensor = true;
			if (!shell_sensor->temp_tracing[shell->index].temp_invalid_flag) {
				tsensor_good_count++;
				sum_tsensor_temp +=
					shell_sensor->temp_tracing[shell->index].temp;
			}
		}
		if (shell_sensor->sensor_type & CHECK_NTC) {
			have_ntc = true;
			if (!shell_sensor->temp_tracing[shell->index].temp_invalid_flag) {
				ntc_good_count++;
				sum_ntc_temp +=
					shell_sensor->temp_tracing[shell->index].temp;
			}
		}
	}

	if ((have_tsensor && tsensor_good_count == 0) ||
	    (have_ntc && ntc_good_count == 0)) {
		pr_err("%s, %s, all tsensors or ntc sensors damaged!\n",
		       __func__, shell->tz_dev->type);
		return 1;
	}

	if (tsensor_good_count != 0)
		*tsensor_avg = sum_tsensor_temp / tsensor_good_count;
	if (ntc_good_count != 0)
		*ntc_avg = sum_ntc_temp / ntc_good_count;

	return 0;
}

static int handle_invalid_temp(struct shell_t *shell)
{
	int i;
	int invalid_temp = ERROR_SHELL_TEMP;
	int tsensor_avg = DEFAULT_SHELL_TEMP;
	int ntc_avg = DEFAULT_SHELL_TEMP;
	struct shell_sensor_t *shell_sensor = NULL;

	if (calc_sensor_temp_avg(shell, &tsensor_avg, &ntc_avg))
		return 1;

	for (i = 0; i < shell->sensor_count; i++) {
		shell_sensor = (struct shell_sensor_t *)(uintptr_t)
			((u64)(uintptr_t)(shell->shell_sensor) +
			 (u64)((long)i) * (sizeof(struct shell_sensor_t) +
			 sizeof(struct temp_tracing_t) *
			 (u64)((long)shell->sample_count)));
		if (shell_sensor->sensor_type & CHECK_TSENS &&
		    shell_sensor->temp_tracing[shell->index].temp_invalid_flag) {
			invalid_temp = shell_sensor->temp_tracing[shell->index].temp;
			pr_err("%s, %s, handle tsensor %d invalid data %d, "
			       "result is %d\n", __func__, shell->tz_dev->type,
			       (i + 1), invalid_temp, tsensor_avg);
			shell_sensor->temp_tracing[shell->index].temp = tsensor_avg;
		}
		if (shell_sensor->sensor_type & CHECK_NTC &&
		    shell_sensor->temp_tracing[shell->index].temp_invalid_flag) {
			invalid_temp = shell_sensor->temp_tracing[shell->index].temp;
			pr_err("%s, %s, handle ntc sensor %d invalid "
			       "data %d, result is %d\n", __func__,
			       shell->tz_dev->type, (i + 1),
			       invalid_temp, ntc_avg);
			shell_sensor->temp_tracing[shell->index].temp = ntc_avg;
		}
		trace_handle_invalid_temp(shell->tz_dev, i, invalid_temp,
					  shell_sensor->temp_tracing[shell->index].temp);
	}

	return 0;
}

static int calc_shell_temp_first(struct shell_t *shell)
{
	int i, temp;
	int sum_board_sensor_temp = 0;
	int count_board_sensor = 0;
	struct shell_sensor_t *shell_sensor = NULL;

	temp = coul_drv_battery_temperature() * 1000;
	if (temp <= shell->ntc_max_temp && temp >= shell->ntc_min_temp)
		return temp;
	pr_err("%s, %s, battery temperature %d out of range!!!\n",
	       __func__, shell->tz_dev->type, temp / 1000);

	for (i = 0; i < shell->sensor_count; i++) {
		shell_sensor = (struct shell_sensor_t *)(uintptr_t)
			((u64)(uintptr_t)(shell->shell_sensor) +
			 (u64)((long)i) * (sizeof(struct shell_sensor_t) +
			 sizeof(struct temp_tracing_t) *
			 (u64)((long)shell->sample_count)));
		if (shell_sensor->sensor_type == TYPE_PERIPHERAL) {
			count_board_sensor++;
			sum_board_sensor_temp +=
				shell_sensor->temp_tracing[shell->index].temp;
		}
	}
	if (count_board_sensor != 0)
		return sum_board_sensor_temp / count_board_sensor;
	pr_err("%s, %s, read temperature of board sensor err!\n", __func__,
	       shell->tz_dev->type);
	return DEFAULT_SHELL_TEMP;
}

static int
set_shell_temp(struct shell_t *shell, int have_invalid_temp, int index)
{
	int ret = 0;

	if (have_invalid_temp)
		ret = handle_invalid_temp(shell);

	if (ret != 0)
		shell->valid_flag = 0;

	if (!shell->valid_flag && index >= shell->sample_count - 1)
		shell->valid_flag = 1;

	if (shell->valid_flag && ret == 0) {
		shell->temp = calc_shell_temp(shell);
		if (shell->old_temp == 0) {
			shell->old_temp = shell->temp;
		} else {
			if ((shell->temp - shell->old_temp) >=
			    (shell->shell_temp_step_range *
			    (int)shell->interval / 1000))
				shell->temp = shell->old_temp +
			   		shell->shell_temp_step *
					(int)shell->interval / 1000;
			else if ((shell->temp - shell->old_temp) <=
				 (-shell->shell_temp_step_range *
				 (int)shell->interval / 1000))
				shell->temp = shell->old_temp -
					shell->shell_temp_step *
					(int)shell->interval / 1000;
			shell->old_temp = shell->temp;
		}
	} else if (!shell->valid_flag && ret == 0) {
		shell->temp = calc_shell_temp_first(shell);
		shell->old_temp = 0;
	} else {
		shell->temp = ERROR_SHELL_TEMP;
		shell->old_temp = 0;
	}

	trace_shell_temp(shell->tz_dev, shell->temp);

	return ret;
}

static void hkadc_sample_temp(struct work_struct *work)
{
	int i, index, ret;
	int have_invalid_temp = 0;
	struct shell_t *shell = NULL;
	struct shell_sensor_t *shell_sensor = NULL;
	struct thermal_zone_device *tz = NULL;
	int temp = 0;

	shell =
		container_of((struct delayed_work *)work, struct shell_t, work); /*lint !e826*/
	index = shell->index;
	mod_delayed_work(system_freezable_power_efficient_wq,
			 (struct delayed_work *)work,
			 round_jiffies(msecs_to_jiffies(shell->interval)));

	for (i = 0; i < shell->sensor_count ; i++) {
		shell_sensor = (struct shell_sensor_t *)(uintptr_t)
			((u64)(uintptr_t)(shell->shell_sensor) +
			 (u64)((long)i) * (sizeof(struct shell_sensor_t) +
			 sizeof(struct temp_tracing_t) *
			 (u64)((long)shell->sample_count)));
		tz = thermal_zone_get_zone_by_name(shell_sensor->sensor_name);
		ret = thermal_zone_get_temp(tz, &temp);
		if (shell_sensor->sensor_type == TYPE_TSENS)
			temp = temp * 1000;
		if (ret != 0)
			temp = DEFAULT_SHELL_TEMP;

#ifdef CONFIG_SHELL_TEMP_DEBUG
		if (i == shell->channel - 1 && shell->debug_temp) {
			pr_err("%s, %s, sensor_name = %s, temp = %d\n", __func__,
			       shell->tz_dev->type,
			       shell_sensor->sensor_name,
			       shell->debug_temp);
			temp = shell->debug_temp;
		}
#endif

		shell_sensor->temp_tracing[shell->index].temp = temp;
		have_invalid_temp = hkadc_handle_temp_data(shell, shell_sensor,
							   (int)temp);
	}

	if (!set_shell_temp(shell, have_invalid_temp, index))
		index++;
	else
		index = 0;
	shell->index = index >= shell->sample_count ? 0 : index;
}

static int fill_sensor_coef(struct shell_t *shell, struct device_node *np)
{
	int i = 0;
	int j, ret;
	int coef = 0;
	const char *ptr_coef = NULL;
	const char *ptr_type = NULL;
	struct device_node *child = NULL;
	struct shell_sensor_t *shell_sensor = NULL;
	struct thermal_zone_device *tz = NULL;

	for_each_child_of_node(np, child) {
		shell_sensor = (struct shell_sensor_t *)(uintptr_t)
			((u64)(uintptr_t)shell +
			sizeof(struct shell_t) +
			(u64)((long)i) * (sizeof(struct shell_sensor_t) +
			sizeof(struct temp_tracing_t) *
			(u64)((long)shell->sample_count)));

		ret = of_property_read_string(child, "type", &ptr_type);
		if (ret != 0) {
			pr_err("%s, %s, type read err\n", __func__,
			       shell->tz_dev->type);
			return ret;
		}
		/* tsens sensor */
		if ((ret = ipa_get_tsensor_id(ptr_type)) >= 0) {
			shell_sensor->sensor_type = TYPE_TSENS;
		/* peripherl sensor */
		} else if ((ret = ipa_get_periph_id(ptr_type)) >= 0) {
			shell_sensor->sensor_type = TYPE_PERIPHERAL;
		/* Battery */
		} else if (strncmp(ptr_type, BATTERY_NAME,
				   strlen(BATTERY_NAME)) == 0) {
			pr_info("%s, %s, type is Battery.\n", __func__,
				shell->tz_dev->type);
			shell_sensor->sensor_type = TYPE_BATTERY;
		/* terminal sensor */
		} else if (!IS_ERR(tz = thermal_zone_get_zone_by_name(ptr_type))) {
			pr_info("%s, %s, terminal sensor\n",
				__func__, shell->tz_dev->type);
			shell_sensor->sensor_type = TYPE_TERMINAL;
		} else { /* others */
			pr_err("%s, %s, %s, sensor id get err\n", __func__,
			       shell->tz_dev->type, ptr_type);
			shell_sensor->sensor_type = TYPE_UNKNOWN;
		}
		shell_sensor->sensor_name = ptr_type;

		for (j = 0; j < shell->sample_count; j++) {
			ret =  of_property_read_string_index(child, "coef", j,
							     &ptr_coef);
			if (ret != 0) {
				pr_err("%s coef %d read err\n", __func__, j);
				return ret;
			}

			ret = kstrtoint(ptr_coef, 10, &coef);
			if (ret != 0) {
				pr_err("%s kstortoint is failed\n", __func__);
				return ret;
			}
			shell_sensor->temp_tracing[j].coef = coef;
		}
		i++;
	}
	return ret;
}

static int read_temp_range(struct device_node *dev_node,
			   struct shell_t *shell,
			   int *tsensor_range, int tsens_len,
			   int *ntc_range, int ntc_len)
{
	int ret, i;
	const char *ptr_tsensor = NULL;
	const char *ptr_ntc = NULL;

	if (tsens_len > SENSOR_PARA_COUNT || ntc_len > SENSOR_PARA_COUNT)
		return -EINVAL;

	ret = of_property_read_s32(dev_node, "shell_temp_step_range",
				   &shell->shell_temp_step_range);
	if (ret != 0) {
		pr_err("%s shell_temp_step_range read err\n", __func__);
		shell->shell_temp_step_range = DEFAULT_SHELL_TEMP_STEP_RANGE;
	}

	ret = of_property_read_s32(dev_node, "shell_temp_step",
				   &shell->shell_temp_step);
	if (ret != 0) {
		pr_err("%s shell_temp_step read err\n", __func__);
		shell->shell_temp_step = DEFAULT_SHELL_TEMP_STEP;
	}

	for (i = 0; i < SENSOR_PARA_COUNT; i++) {
		ret = of_property_read_string_index(dev_node, "tsensor_para", i,
						    &ptr_tsensor);
		if (ret != 0) {
			pr_err("%s tsensor_para %d read err\n", __func__, i);
			return -EINVAL;
		}

		ret = kstrtoint(ptr_tsensor, 10, &tsensor_range[i]);
		if (ret != 0) {
			pr_err("%s kstrtoint is failed with tsensor_para %d\n",
			       __func__, i);
			return -EINVAL;
		}

		ret = of_property_read_string_index(dev_node, "ntc_para", i, &ptr_ntc);
		if (ret != 0) {
			pr_err("%s ntc_para %d read err\n", __func__, i);
			return -EINVAL;
		}

		ret = kstrtoint(ptr_ntc, 10, &ntc_range[i]);
		if (ret != 0) {
			pr_err("%s kstrtoint is failed with ntc_para %d",
			       __func__, i);
			return -EINVAL;
		}
	}

	return ret;
}

static int get_shell_dts_para(struct device_node *dev_node, int sample_count,
			      struct shell_t *shell, int sensor_count,
			      struct device_node *np)
{
	int tsensor_para[SENSOR_PARA_COUNT] = {0};
	int ntc_para[SENSOR_PARA_COUNT] = {0};
	int ret;

	if (read_temp_range(dev_node, shell, tsensor_para,
			    SENSOR_PARA_COUNT, ntc_para, SENSOR_PARA_COUNT)) {
		shell->tsensor_temp_step = DEFAULT_TSENS_STEP_RANGE;
		shell->tsensor_max_temp = DEFAULT_TSENS_MAX_TEMP;
		shell->tsensor_min_temp = DEFAULT_TSENS_MIN_TEMP;
		shell->ntc_temp_step = DEFAULT_NTC_STEP_RANGE;
		shell->ntc_max_temp = DEFAULT_NTC_MAX_TEMP;
		shell->ntc_min_temp = DEFAULT_NTC_MIN_TEMP;
	} else {
		shell->tsensor_temp_step = tsensor_para[0];
		shell->tsensor_max_temp = tsensor_para[1];
		shell->tsensor_min_temp = tsensor_para[2];
		shell->ntc_temp_step = ntc_para[0];
		shell->ntc_max_temp = ntc_para[1];
		shell->ntc_min_temp = ntc_para[2];
	}

	shell->index = 0;
	shell->valid_flag = 0;
	shell->old_temp = 0;
	shell->sensor_count = sensor_count;
	shell->sample_count = sample_count;
	shell->is_framework = 0;
#ifdef CONFIG_SHELL_TEMP_DEBUG
	shell->channel = 0;
	shell->debug_temp = 0;
#endif

	ret = of_property_read_u32(dev_node, "interval", &shell->interval);
	if (ret != 0) {
		pr_err("%s interval read err\n", __func__);
		return -EINVAL;
	}

	ret = of_property_read_s32(dev_node, "bias", &shell->bias);
	if (ret != 0) {
		pr_err("%s bias read err\n", __func__);
		return -EINVAL;
	}

	ret = fill_sensor_coef(shell, np);
	if (ret != 0)
		return -EINVAL;

	return 0;
}

static int create_file_node(struct platform_device *pdev, struct attribute_group *attr)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	int ret;
	ret = sysfs_create_link(&hw_thermal_info.temperature_node.device->kobj, &pdev->dev.kobj, dev_node->name);
	if (ret != 0) {
		pr_err("%s: create hw_thermal device file error: %d\n", dev_node->name, ret);
		return -EINVAL;
	}
	ret = sysfs_create_group(&pdev->dev.kobj, attr);
	if (ret != 0) {
		pr_err("%s: create shell file error: %d\n", dev_node->name, ret);
		sysfs_remove_link(&hw_thermal_info.temperature_node.device->kobj, dev_node->name);
		return -EINVAL;
	}
	return 0;
}

static int create_framework_file_node(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	int is_framework, ret;
	struct shell_t *shell = NULL;

	ret = of_property_read_s32(dev_node, "is_framework", &is_framework);
	if (ret != 0) {
		pr_err("%s is_framework read err\n", __func__);
		is_framework = 0;
	}
	shell = platform_get_drvdata(pdev);
	shell->is_framework = is_framework;
	if (is_framework == IS_FRAMEWORK) {
		ret = create_file_node(pdev, &shell_attribute_group);
		if (ret == 0)
			pr_err("shell framework node create ok!\n");
		return ret;
	}
	return -EINVAL;
}

static int shell_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	int ret, sensor_count;
	int sample_count = 0;
	struct device_node *np = NULL;
	struct shell_t *shell = NULL;

	if (!of_device_is_available(dev_node)) {
		dev_err(dev, "shell dev not found\n");
		return -ENODEV;
	}

	ret = of_property_read_s32(dev_node, "count", &sample_count);
	if (ret != 0) {
		pr_err("%s count read err\n", __func__);
		goto exit;
	}

	np = of_find_node_by_name(dev_node, "sensors");
	if (np == NULL) {
		pr_err("sensors node not found\n");
		ret = -ENODEV;
		goto exit;
	}

	sensor_count = of_get_child_count(np);
	if (sensor_count <= 0) {
		ret = -EINVAL;
		pr_err("%s sensor count read err\n", __func__);
		goto node_put;
	}

	shell = kzalloc(sizeof(struct shell_t) +
			     (u64)((long)sensor_count) *
			     (sizeof(struct shell_sensor_t) +
			     sizeof(struct temp_tracing_t) *
			     (u64)((long)sample_count)), GFP_KERNEL);
	if (shell == NULL) {
		ret = -ENOMEM;
		pr_err("no enough memory\n");
		goto node_put;
	}

	ret = get_shell_dts_para(dev_node, sample_count, shell,
				 sensor_count, np);
	if (ret != 0)
		goto free_mem;

	shell->tz_dev
		= thermal_zone_device_register(dev_node->name, 0, 0, shell,
					       &shell_thermal_zone_ops, NULL, 0, 0);
	if (IS_ERR(shell->tz_dev)) {
		dev_err(dev, "register thermal zone for shell failed.\n");
		ret = -ENODEV;
		goto free_mem;
	}

	shell->temp = coul_drv_battery_temperature() * 1000;
	pr_info("%s: temp %d\n", dev_node->name, shell->temp);

	platform_set_drvdata(pdev, shell);
	ret = create_framework_file_node(pdev);
	if (ret == 0) {
		of_node_put(np);
		return 0;
	}

	INIT_DELAYED_WORK(&shell->work, hkadc_sample_temp); /*lint !e747*/
	mod_delayed_work(system_freezable_power_efficient_wq,
			 &shell->work,
			 round_jiffies(msecs_to_jiffies(shell->interval)));
	ret = create_file_node(pdev, &shell_attribute_group);
	if (ret != 0)
		goto cancel_work;
	of_node_put(np);

	return 0; /*lint !e429*/

cancel_work:
	cancel_delayed_work(&shell->work);
	thermal_zone_device_unregister(shell->tz_dev);
free_mem:
	kfree(shell);
node_put:
	of_node_put(np);
exit:
	return ret;
}

static int shell_remove(struct platform_device *pdev)
{
	struct shell_t *shell = platform_get_drvdata(pdev);

	if (shell != NULL) {
		platform_set_drvdata(pdev, NULL);
		thermal_zone_device_unregister(shell->tz_dev);
		kfree(shell);
	}

	return 0;
}

/*lint -e785*/
static struct of_device_id shell_of_match[] = {
	{ .compatible = "hisi,shell-temp" },
	{},
};

/*lint +e785*/
MODULE_DEVICE_TABLE(of, shell_of_match);

int shell_temp_pm_resume(struct platform_device *pdev)
{
	struct shell_t *shell = NULL;

	pr_info("%s+\n", __func__);
	shell = platform_get_drvdata(pdev);

	if (shell != NULL) {
		shell->temp = coul_drv_battery_temperature() * 1000;
		pr_info("%s: temp %d\n", shell->tz_dev->type, shell->temp);
		shell->index = 0;
		shell->valid_flag = 0;
		shell->old_temp = 0;
#ifdef CONFIG_SHELL_TEMP_DEBUG
		shell->channel = 0;
		shell->debug_temp = 0;
#endif
	}
	pr_info("%s-\n", __func__);

	return 0;
}

/*lint -e64 -e785 -esym(64,785,*)*/
static struct platform_driver shell_platdrv = {
	.driver = {
		.name = "hisi-shell-temp",
		.owner = THIS_MODULE,
		.of_match_table = shell_of_match,
	},
	.probe = shell_probe,
	.remove = shell_remove,
	.resume = shell_temp_pm_resume,
};

/*lint -e64 -e785 +esym(64,785,*)*/

static int __init shell_init(void)
{
	int ret;

	/* create thermal class */
	hw_thermal_info.thermal_class = class_create(THIS_MODULE, "hw_thermal"); /*lint !e64*/
	if (IS_ERR(hw_thermal_info.thermal_class)) {
		pr_err("thermal class create error\n");
		return PTR_ERR(hw_thermal_info.thermal_class);
	}
	/* create device "temp" */
	hw_thermal_info.temperature_node.device =
		device_create(hw_thermal_info.thermal_class, NULL, 0, NULL, "temp");
	if (IS_ERR(hw_thermal_info.temperature_node.device)) {
		pr_err("hw_thermal:temperature_node device create error\n");
		class_destroy(hw_thermal_info.thermal_class);
		hw_thermal_info.thermal_class = NULL;
		return PTR_ERR(hw_thermal_info.temperature_node.device);
	}
	/* create an ambient node for thermal-daemon. */
	ret = device_create_file(hw_thermal_info.temperature_node.device,
				 &dev_attr_ambient);
	if (ret != 0) {
		pr_err("hw_thermal:ambient node create error\n");
		device_destroy(hw_thermal_info.thermal_class, 0);
		class_destroy(hw_thermal_info.thermal_class);
		hw_thermal_info.thermal_class = NULL;
		return ret;
	}

	return platform_driver_register(&shell_platdrv); /*lint !e64*/
}

static void __exit shell_exit(void)
{
	if (hw_thermal_info.thermal_class != NULL) {
		device_destroy(hw_thermal_info.thermal_class, 0);
		class_destroy(hw_thermal_info.thermal_class);
	}
	platform_driver_unregister(&shell_platdrv);
}

/*lint -e528 -esym(528,*)*/
module_init(shell_init);
module_exit(shell_exit);
/*lint -e528 +esym(528,*)*/

/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("thermal shell temp module driver");
/*lint -e753 +esym(753,*)*/
