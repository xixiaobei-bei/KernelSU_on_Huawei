/*
 * npu_thermal.c
 *
 * npu thermal calculation module
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/devfreq_cooling.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include <securec.h>
#include <linux/lpm_thermal.h>

#define NPU_DEFAULT_LOW_TEMP	0

#define COOLING_DEV_MAX	5

struct npu_trip_point {
	unsigned long temp;
	enum thermal_trip_type type;
	char cdev_name[COOLING_DEV_MAX][THERMAL_NAME_LENGTH];
};

struct npu_thsens_platform_data {
	struct npu_trip_point trip_points[THERMAL_MAX_TRIPS];
	int num_trips;
	unsigned int polling_delay;
	unsigned int polling_delay_passive;
};

struct npu_thermal_zone {
	struct thermal_zone_device *thermal_dev;
	struct mutex th_lock;
	struct delayed_work thermal_work;
	struct npu_thsens_platform_data *trip_tab;
	enum thermal_device_mode mode;
	enum thermal_trend trend;
	unsigned long cur_temp;
	unsigned int cur_index;
};

/* Local function to check if thermal zone matches cooling devices */
static int npu_thermal_match_cdev(struct thermal_cooling_device *cdev,
				  struct npu_trip_point *trip_point)
{
	int i;

	if (strlen(cdev->type) == 0)
		return -EINVAL;

	for (i = 0; i < COOLING_DEV_MAX; i++) {
		if (strncmp(cdev->type, trip_point->cdev_name[i],
			     sizeof(cdev->type) - 1) == 0)
			return 0;
	}

	return -ENODEV;
}

/* Callback to bind cooling device to thermal zone */
static int __nocfi npu_cdev_bind(struct thermal_zone_device *thermal,
				 struct thermal_cooling_device *cdev)
{
	struct npu_thermal_zone *pzone = thermal->devdata;
	struct npu_thsens_platform_data *ptrips = pzone->trip_tab;
	unsigned long max_state, upper, lower;
	int i, rc;
	int ret = 0;

	cdev->ops->get_max_state(cdev, &max_state);

	for (i = 0; i < ptrips->num_trips; i++) {
		if (npu_thermal_match_cdev(cdev, &ptrips->trip_points[i]) != 0)
			continue;

		upper = (unsigned long)i > max_state ? max_state : (unsigned long)i;
		lower = upper;

		rc = thermal_zone_bind_cooling_device(thermal, i, cdev, upper,
						       lower, THERMAL_WEIGHT_DEFAULT);
		if (rc != 0) {
			dev_err(&cdev->device, "%s bind to %d fail: %d!\n",
				cdev->type, i, rc);
			ret = rc;
		}
	}

	return ret;
}

/* Callback to unbind cooling device from thermal zone */
static int npu_cdev_unbind(struct thermal_zone_device *thermal,
			   struct thermal_cooling_device *cdev)
{
	struct npu_thermal_zone *pzone = thermal->devdata;
	struct npu_thsens_platform_data *ptrips = pzone->trip_tab;
	int i, rc;
	int ret = -EINVAL;

	for (i = 0; i < ptrips->num_trips; i++) {
		if (npu_thermal_match_cdev(cdev, &ptrips->trip_points[i]))
			continue;

		rc = thermal_zone_unbind_cooling_device(thermal, i, cdev);
		if (rc != 0) {
			dev_err(&cdev->device, "%s unbind from %d fail: %d!\n",
				cdev->type, i, rc);
			ret = rc;
		}
	}

	return ret;
}

/* Callback to get current temperature */
static int npu_sys_get_temp(struct thermal_zone_device *thermal, int *temp)
{
	struct npu_thermal_zone *pzone = thermal->devdata;
	int id, ret;

	id = ipa_get_tsensor_id("npu");
	if (id == -1)
		return -ENODEV;

	ret = ipa_get_sensor_value(id, temp);
	if (ret != 0)
		return -EINVAL;

	pzone->cur_temp = (unsigned long)(*temp);

	return 0;
}

/* Callback to get temperature changing trend */
static int npu_sys_get_trend(struct thermal_zone_device *thermal,
			     int trip, enum thermal_trend *trend)
{
	struct npu_thermal_zone *pzone = thermal->devdata;
	struct thermal_zone_device *tz = pzone->thermal_dev;
	int trip_temp = 0;
	int temp, last_temp, ret;

	if (tz == NULL)
		return -EINVAL;

	ret = tz->ops->get_trip_temp(tz, trip, &trip_temp);
	if (ret != 0)
		return ret;

	temp = READ_ONCE(tz->temperature);
	last_temp = READ_ONCE(tz->last_temperature);

	if (temp > trip_temp) {
		if (temp >= last_temp)
			*trend = THERMAL_TREND_RAISING;
		else
			*trend = THERMAL_TREND_STABLE;
	} else if (temp < trip_temp) {
		*trend = THERMAL_TREND_DROPPING;
	} else {
		*trend = THERMAL_TREND_STABLE;
	}

	return 0;
}

/* Callback to get thermal zone mode */
static int npu_sys_get_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode *mode)
{
	struct npu_thermal_zone *pzone = thermal->devdata;

	mutex_lock(&pzone->th_lock);
	*mode = pzone->mode;
	mutex_unlock(&pzone->th_lock);

	return 0;
}

/* Callback to set thermal zone mode */
static int npu_sys_set_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode mode)
{
	struct npu_thermal_zone *pzone = thermal->devdata;

	mutex_lock(&pzone->th_lock);

	pzone->mode = mode;
	if (mode == THERMAL_DEVICE_ENABLED)
		mod_delayed_work(system_freezable_power_efficient_wq,
				 &pzone->thermal_work,
				 msecs_to_jiffies(pzone->trip_tab->polling_delay));

	mutex_unlock(&pzone->th_lock);

	return 0;
}

/* Callback to get trip point type */
static int npu_sys_get_trip_type(struct thermal_zone_device *thermal,
				 int trip, enum thermal_trip_type *type)
{
	struct npu_thermal_zone *pzone = thermal->devdata;
	struct npu_thsens_platform_data *ptrips = pzone->trip_tab;

	if (trip < 0 || trip >= ptrips->num_trips)
		return -EINVAL;

	*type = ptrips->trip_points[trip].type;

	return 0;
}

/* Callback to get trip point temperature */
static int npu_sys_get_trip_temp(struct thermal_zone_device *thermal,
				 int trip, int *temp)
{
	struct npu_thermal_zone *pzone = thermal->devdata;
	struct npu_thsens_platform_data *ptrips = pzone->trip_tab;

	if (trip < 0 || trip >= ptrips->num_trips)
		return -EINVAL;

	*temp = ptrips->trip_points[trip].temp;

	return 0;
}

/* Callback to set trip point temperature */
static int npu_sys_set_trip_temp(struct thermal_zone_device *thermal,
				 int trip, int temp)
{
	struct npu_thermal_zone *pzone = thermal->devdata;
	struct npu_thsens_platform_data *ptrips = pzone->trip_tab;

	if (trip < 0 || trip >= ptrips->num_trips)
		return -EINVAL;

	dev_err(&pzone->thermal_dev->device, "old trip temp:%lu\n",
		ptrips->trip_points[trip].temp);
	ptrips->trip_points[trip].temp = (unsigned long)temp;
	dev_err(&pzone->thermal_dev->device, "new trip temp:%lu\n",
		ptrips->trip_points[trip].temp);
	return 0;
}

/* Callback to get critical trip point temperature */
static int npu_sys_get_crit_temp(struct thermal_zone_device *thermal, int *temp)
{
	struct npu_thermal_zone *pzone = thermal->devdata;
	struct npu_thsens_platform_data *ptrips = pzone->trip_tab;
	int i;

	for (i = ptrips->num_trips - 1; i > 0; i--) {
		if (ptrips->trip_points[i].type == THERMAL_TRIP_CRITICAL) {
			*temp = ptrips->trip_points[i].temp;
			return 0;
		}
	}

	return -EINVAL;
}

static struct thermal_zone_device_ops thdev_ops = {
	.bind = npu_cdev_bind,
	.unbind = npu_cdev_unbind,
	.get_temp = npu_sys_get_temp,
	.get_trend = npu_sys_get_trend,
	.get_mode = npu_sys_get_mode,
	.set_mode = npu_sys_set_mode,
	.get_trip_type = npu_sys_get_trip_type,
	.get_trip_temp = npu_sys_get_trip_temp,
	.get_crit_temp = npu_sys_get_crit_temp,
	.set_trip_temp = npu_sys_set_trip_temp,
};

static struct thermal_zone_params npu_thermal_params = {
	.governor_name = "step_wise",
	.no_hwmon = true,
};

static void npu_thermal_work(struct work_struct *work)
{
	enum thermal_device_mode cur_mode = THERMAL_DEVICE_DISABLED;
	struct npu_thermal_zone *pzone = NULL;
	int temp = 0;

	pzone = container_of(work, struct npu_thermal_zone, thermal_work.work);

	mutex_lock(&pzone->th_lock);
	cur_mode = pzone->mode;
	mutex_unlock(&pzone->th_lock);

	if (cur_mode == THERMAL_DEVICE_DISABLED)
		return;

	thermal_zone_device_update(pzone->thermal_dev, THERMAL_EVENT_UNSPECIFIED);
	dev_err(&pzone->thermal_dev->device, "thermal work finished.\n");
	pzone->thermal_dev->ops->get_temp(pzone->thermal_dev, &temp);
	if (temp > (int)pzone->trip_tab->trip_points[0].temp) {
		mod_delayed_work(system_freezable_power_efficient_wq,
				 &pzone->thermal_work,
				 msecs_to_jiffies(pzone->trip_tab->polling_delay));
	} else {
		mod_delayed_work(system_freezable_power_efficient_wq,
				 &pzone->thermal_work,
				 msecs_to_jiffies(pzone->trip_tab->polling_delay_passive));
	}
}

static int trip_point_type_parse(const char *tmp_str, u32 index, size_t str_len,
				 struct npu_thsens_platform_data *ptrips)
{
	if (strncmp(tmp_str, "active", str_len) == 0)
		ptrips->trip_points[index].type = THERMAL_TRIP_ACTIVE;
	else if (strncmp(tmp_str, "passive", str_len) == 0)
		ptrips->trip_points[index].type = THERMAL_TRIP_PASSIVE;
	else if (strncmp(tmp_str, "hot", str_len) == 0)
		ptrips->trip_points[index].type = THERMAL_TRIP_HOT;
	else if (strncmp(tmp_str, "critical", str_len) == 0)
		ptrips->trip_points[index].type = THERMAL_TRIP_CRITICAL;
	else
		return -EINVAL;

	return 0;
}

static int trips_cdev_name_parse(int i, struct device_node *np, u32 tmp_data,
				 struct npu_thsens_platform_data *ptrips)
{
	char prop_name[32] = {0};
	const char *tmp_str = NULL;
	u32 j;
	int ret;

	if (tmp_data > COOLING_DEV_MAX)
		return -EINVAL;

	for (j = 0; j < tmp_data; j++) {
		ret = snprintf_s(prop_name, sizeof(prop_name),
				 sizeof(prop_name) - 1,
				 "trip%d-cdev-name%d", i, j);
		if ((ret < 0) || of_property_read_string(np, prop_name, &tmp_str))
			return -EINVAL;

		if (strlen(tmp_str) >= THERMAL_NAME_LENGTH)
			return -EINVAL;

		ret = strncpy_s(ptrips->trip_points[i].cdev_name[j],
				sizeof(ptrips->trip_points[i].cdev_name[j]),
				tmp_str,
				sizeof(ptrips->trip_points[i].cdev_name[j]) - 1);
		if (ret != EOK)
			return -EINVAL;
	}
	return 0;
}

static struct npu_thsens_platform_data*
	npu_thermal_parse_dt(struct platform_device *pdev)
{
	struct npu_thsens_platform_data *ptrips = NULL;
	struct device_node *np = pdev->dev.of_node;
	char prop_name[32] = {0};
	const char *tmp_str = NULL;
	u32 tmp_data, i;
	int ret;

	if (np == NULL)
		return NULL;

	ptrips = devm_kzalloc(&pdev->dev, sizeof(*ptrips), GFP_KERNEL);
	if (ptrips == NULL)
		return NULL;

	if (of_property_read_u32(np, "num-trips", &tmp_data))
		goto err_parse_dt;

	if (tmp_data > THERMAL_MAX_TRIPS)
		goto err_parse_dt;

	ptrips->num_trips = tmp_data;

	if (of_property_read_u32(np, "polling-delay", &tmp_data))
		goto err_parse_dt;

	ptrips->polling_delay = tmp_data;

	if (of_property_read_u32(np, "polling-delay-passive", &tmp_data))
		goto err_parse_dt;

	ptrips->polling_delay_passive = tmp_data;

	for (i = 0; i < (u32)ptrips->num_trips; i++) {
		ret = snprintf_s(prop_name, sizeof(prop_name),
				 sizeof(prop_name) - 1, "trip%d-temp", i);
		if ((ret < 0) || of_property_read_u32(np, prop_name, &tmp_data))
			goto err_parse_dt;

		ptrips->trip_points[i].temp = tmp_data;

		ret = snprintf_s(prop_name, sizeof(prop_name),
				 sizeof(prop_name) - 1, "trip%d-type", i);
		if ((ret < 0) || of_property_read_string(np, prop_name, &tmp_str))
			goto err_parse_dt;

		if (trip_point_type_parse(tmp_str, i, strlen(tmp_str), ptrips))
			goto err_parse_dt;

		ret = snprintf_s(prop_name, sizeof(prop_name),
				 sizeof(prop_name) - 1, "trip%d-cdev-num", i);
		if ((ret < 0) || of_property_read_u32(np, prop_name, &tmp_data))
			goto err_parse_dt;

		ret = trips_cdev_name_parse(i, np, tmp_data, ptrips);
		if (ret != 0)
			goto err_parse_dt;
	}
	return ptrips;

err_parse_dt:
	dev_err(&pdev->dev, "Parsing device tree data error.\n");
	return NULL;
}

static int npu_thermal_probe(struct platform_device *pdev)
{
	struct npu_thermal_zone *pzone = NULL;
	struct npu_thsens_platform_data *ptrips = NULL;
	int trip_mask;
	int ret = 0;

	ptrips = npu_thermal_parse_dt(pdev);
	if (ptrips == NULL)
		return -EINVAL;

	pzone = devm_kzalloc(&pdev->dev, sizeof(*pzone), GFP_KERNEL);
	if (pzone == NULL)
		return -ENOMEM;

	mutex_init(&pzone->th_lock);
	mutex_lock(&pzone->th_lock);

	pzone->mode = THERMAL_DEVICE_DISABLED;
	pzone->trip_tab = ptrips;
	INIT_DELAYED_WORK(&pzone->thermal_work, npu_thermal_work);

	trip_mask = (int)(BIT((unsigned int)ptrips->num_trips) - 1);
	pzone->thermal_dev =
		thermal_zone_device_register("npu_thermal", ptrips->num_trips,
					      trip_mask, pzone, &thdev_ops,
					      &npu_thermal_params, 0, 0);

	if (IS_ERR(pzone->thermal_dev)) {
		dev_err(&pdev->dev, "Register thermal zone device failed.\n");
		ret = PTR_ERR(pzone->thermal_dev);
		goto out_unlock;
	}
	dev_info(&pdev->dev, "Thermal zone device registered.\n");

	platform_set_drvdata(pdev, pzone);
	pzone->mode = THERMAL_DEVICE_ENABLED;

out_unlock:
	mutex_unlock(&pzone->th_lock);

	return ret;
}

static int npu_thermal_remove(struct platform_device *pdev)
{
	struct npu_thermal_zone *pzone = platform_get_drvdata(pdev);

	thermal_zone_device_unregister(pzone->thermal_dev);
	cancel_delayed_work(&pzone->thermal_work);
	mutex_destroy(&pzone->th_lock);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id npu_thermal_match[] = {
	{ .compatible = "hisilicon,npu_thermal" },
	{},
};
MODULE_DEVICE_TABLE(of, npu_thermal_match);
#endif

static struct platform_driver npu_thermal_driver = {
	.driver = {
		.name = "npu_thermal",
		.of_match_table = of_match_ptr(npu_thermal_match),
	},
	.probe = npu_thermal_probe,
	.remove = npu_thermal_remove,
};

module_platform_driver(npu_thermal_driver);

MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_DESCRIPTION("npu thermal driver");
MODULE_LICENSE("GPL v2");
