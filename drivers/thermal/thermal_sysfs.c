/*
 *  thermal.c - sysfs interface of thermal devices
 *
 *  Copyright (C) 2016 Eduardo Valentin <edubezval@gmail.com>
 *
 *  Highly based on original thermal_core.c
 *  Copyright (C) 2008 Intel Corp
 *  Copyright (C) 2008 Zhang Rui <rui.zhang@intel.com>
 *  Copyright (C) 2008 Sujith Thomas <sujith.thomas@intel.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <trace/events/thermal.h>
#include <linux/thermal.h>
#include "thermal_core.h"

#ifdef CONFIG_HISI_IPA_THERMAL
#define MAX_TEMP 145000
#define MIN_TEMP -40000
s32 thermal_zone_temp_check(s32 temperature)
{
	if (temperature > MAX_TEMP) {
		temperature = MAX_TEMP;
	} else if (temperature < MIN_TEMP) {
		temperature = MIN_TEMP;
	}
	return temperature;
}
#endif
/* sys I/F for thermal zone */

static ssize_t
type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);

	return sprintf(buf, "%s\n", tz->type);/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

static ssize_t
temp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	int temperature, ret;

	ret = thermal_zone_get_temp(tz, &temperature);

	if (ret)
		return ret;

	return sprintf(buf, "%d\n", temperature);/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

#ifdef CONFIG_HISI_IPA_THERMAL
extern void restore_actor_weights(struct thermal_zone_device *tz);
extern void update_actor_weights(struct thermal_zone_device *tz);
#endif

static ssize_t
mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	enum thermal_device_mode mode;
	int result;

	if (!tz->ops->get_mode)
		return -EPERM;

	result = tz->ops->get_mode(tz, &mode);
	if (result)
		return result;

	return sprintf(buf, "%s\n", mode == THERMAL_DEVICE_ENABLED ? "enabled" : "disabled");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

static ssize_t
mode_store(struct device *dev, struct device_attribute *attr,
	   const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	int result;

	if (!tz->ops->set_mode)
		return -EPERM;

	if (!strncmp(buf, "enabled", sizeof("enabled") - 1)) {
#ifdef CONFIG_HISI_IPA_THERMAL
		restore_actor_weights(tz);
#endif
		result = tz->ops->set_mode(tz, THERMAL_DEVICE_ENABLED);
	} else if (!strncmp(buf, "disabled", sizeof("disabled") - 1)) {
#ifdef CONFIG_HISI_IPA_THERMAL
		restore_actor_weights(tz);
#endif
		result = tz->ops->set_mode(tz, THERMAL_DEVICE_DISABLED);
	} else
		result = -EINVAL;

	if (result)
		return result;

#ifdef CONFIG_HISI_IPA_THERMAL
	if (strncmp(tz->governor->name, USER_SPACE_GOVERNOR, THERMAL_NAME_LENGTH)) {/*[false alarm]*/
		mutex_lock(&tz->lock);
		ipa_freq_limit_reset(tz);
		mutex_unlock(&tz->lock);
	}
#endif
	return count;
}

#ifdef CONFIG_HISI_IPA_THERMAL
extern ssize_t boost_show(struct device *dev, struct device_attribute *attr, char *buf);
extern ssize_t boost_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static DEVICE_ATTR(boost, 0644, boost_show, boost_store);
#endif

static ssize_t
trip_point_type_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	enum thermal_trip_type type;
	int trip, result;

	if (!tz->ops->get_trip_type)
		return -EPERM;

	if (sscanf(attr->attr.name, "trip_point_%d_type", &trip) != 1) /* unsafe_function_ignore: sscanf */
		return -EINVAL;

	result = tz->ops->get_trip_type(tz, trip, &type);
	if (result)
		return result;

	switch (type) {
	case THERMAL_TRIP_CRITICAL:
		return sprintf(buf, "critical\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
	case THERMAL_TRIP_HOT:
		return sprintf(buf, "hot\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
#if defined(CONFIG_HISI_THERMAL_TSENSOR) || defined(CONFIG_HISI_THERMAL_PERIPHERAL)
	case THERMAL_TRIP_CONFIGURABLE_HI:
		return sprintf(buf, "configurable_hi\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
	case THERMAL_TRIP_CONFIGURABLE_LOW:
		return sprintf(buf, "configurable_low\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
	case THERMAL_TRIP_CRITICAL_LOW:
		return sprintf(buf, "critical_low\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
#endif
#ifdef CONFIG_HISI_THERMAL_TRIP
	case THERMAL_TRIP_THROTTLING:
		return sprintf(buf, "throttling\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
	case THERMAL_TRIP_SHUTDOWN:
		return sprintf(buf, "shutdown\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
	case THERMAL_TRIP_BELOW_VR_MIN:
		return sprintf(buf, "below_vr_min\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
	case THERMAL_TRIP_OVER_SKIN:
		return sprintf(buf, "over_skin\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
#endif
	case THERMAL_TRIP_PASSIVE:
		return sprintf(buf, "passive\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
	case THERMAL_TRIP_ACTIVE:
		return sprintf(buf, "active\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
	default:
		return sprintf(buf, "unknown\n");/*lint !e421 */ /* unsafe_function_ignore: sprintf */
	}
}

#ifdef CONFIG_HISI_THERMAL_TSENSOR
static ssize_t
trip_point_type_activate(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	int trip = 0, result = 0;

	if (!tz->ops->activate_trip_type)
		return -EPERM;

	if (!sscanf(attr->attr.name, "trip_point_%d_type", &trip)) /* unsafe_function_ignore: sscanf */
		return -EINVAL;

	if (!strncmp(buf, "enabled", strlen("enabled"))) {
		result = tz->ops->activate_trip_type(tz, trip,
					THERMAL_TRIP_ACTIVATION_ENABLED);
		}
	else if (!strncmp(buf, "disabled", strlen("disabled"))) {
		result = tz->ops->activate_trip_type(tz, trip,
					THERMAL_TRIP_ACTIVATION_DISABLED);
		}
	else{
		result = -EINVAL;
		}
	if (result)
		return result;
	return count;
}
#endif

static ssize_t
trip_point_temp_store(struct device *dev, struct device_attribute *attr,
		      const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	int trip, ret;
	int temperature;

	if (!tz->ops->set_trip_temp)
		return -EPERM;

	if (sscanf(attr->attr.name, "trip_point_%d_temp", &trip) != 1) /* unsafe_function_ignore: sscanf */
		return -EINVAL;

	if (kstrtoint(buf, 10, &temperature))
		return -EINVAL;

	ret = tz->ops->set_trip_temp(tz, trip, temperature);
#ifdef CONFIG_HISI_IPA_THERMAL
	update_pid_value(tz);
#endif
	if (ret)
		return ret;

	thermal_zone_device_update(tz, THERMAL_EVENT_UNSPECIFIED);

	return count;
}

static ssize_t
trip_point_temp_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	int trip, ret;
	int temperature;

	if (!tz->ops->get_trip_temp)
		return -EPERM;

	if (sscanf(attr->attr.name, "trip_point_%d_temp", &trip) != 1) /* unsafe_function_ignore: sscanf */
		return -EINVAL;

	ret = tz->ops->get_trip_temp(tz, trip, &temperature);

	if (ret)
		return ret;

	return sprintf(buf, "%d\n", temperature);/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

static ssize_t
trip_point_hyst_store(struct device *dev, struct device_attribute *attr,
		      const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	int trip, ret;
	int temperature;

	if (!tz->ops->set_trip_hyst)
		return -EPERM;

	if (sscanf(attr->attr.name, "trip_point_%d_hyst", &trip) != 1) /* unsafe_function_ignore: sscanf */
		return -EINVAL;

	if (kstrtoint(buf, 10, &temperature))
		return -EINVAL;

	/*
	 * We are not doing any check on the 'temperature' value
	 * here. The driver implementing 'set_trip_hyst' has to
	 * take care of this.
	 */
	ret = tz->ops->set_trip_hyst(tz, trip, temperature);

	if (!ret)
		thermal_zone_set_trips(tz);

	return ret ? ret : count;
}

static ssize_t
trip_point_hyst_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	int trip, ret;
	int temperature;

	if (!tz->ops->get_trip_hyst)
		return -EPERM;

	if (sscanf(attr->attr.name, "trip_point_%d_hyst", &trip) != 1) /* unsafe_function_ignore: sscanf */
		return -EINVAL;

	ret = tz->ops->get_trip_hyst(tz, trip, &temperature);

	return ret ? ret : sprintf(buf, "%d\n", temperature);/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

static ssize_t
passive_store(struct device *dev, struct device_attribute *attr,
	      const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	int state;

	if (sscanf(buf, "%d\n", &state) != 1) /* unsafe_function_ignore: sscanf */
		return -EINVAL;

	/* sanity check: values below 1000 millicelcius don't make sense
	 * and can cause the system to go into a thermal heart attack
	 */
	if (state && state < 1000)
		return -EINVAL;

	if (state && !tz->forced_passive) {
		if (!tz->passive_delay)
			tz->passive_delay = 1000;
		thermal_zone_device_rebind_exception(tz, "Processor",
						     sizeof("Processor"));
	} else if (!state && tz->forced_passive) {
		tz->passive_delay = 0;
		thermal_zone_device_unbind_exception(tz, "Processor",
						     sizeof("Processor"));
	}

	tz->forced_passive = state;

	thermal_zone_device_update(tz, THERMAL_EVENT_UNSPECIFIED);

	return count;
}

static ssize_t
passive_show(struct device *dev, struct device_attribute *attr,
	     char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);

	return sprintf(buf, "%d\n", tz->forced_passive);/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

static ssize_t
policy_store(struct device *dev, struct device_attribute *attr,
	     const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	char name[THERMAL_NAME_LENGTH];
	int ret;

	snprintf(name, sizeof(name), "%s", buf); /* unsafe_function_ignore: snprintf */

	ret = thermal_zone_device_set_policy(tz, name);
	if (!ret)
		ret = count;

	return ret;
}

static ssize_t
policy_show(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);

	return sprintf(buf, "%s\n", tz->governor->name);/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

static ssize_t
available_policies_show(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	return thermal_build_list_of_policies(buf);
}

#if (IS_ENABLED(CONFIG_THERMAL_EMULATION))
static ssize_t
emul_temp_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	int ret = 0;
	int temperature;

	if (kstrtoint(buf, 10, &temperature))
		return -EINVAL;

#ifdef CONFIG_HISI_IPA_THERMAL
	temperature = thermal_zone_temp_check(temperature);
#endif
	if (!tz->ops->set_emul_temp) {
		mutex_lock(&tz->lock);
		tz->emul_temperature = temperature;
		mutex_unlock(&tz->lock);
	} else {
		ret = tz->ops->set_emul_temp(tz, temperature);
	}

	if (!ret)
		thermal_zone_device_update(tz, THERMAL_EVENT_UNSPECIFIED);

	return ret ? ret : count;
}
static DEVICE_ATTR(emul_temp, S_IWUSR, NULL, emul_temp_store);
#endif

static ssize_t
sustainable_power_show(struct device *dev, struct device_attribute *devattr,
		       char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);

	if (tz->tzp)
		return sprintf(buf, "%u\n", tz->tzp->sustainable_power);/*lint !e421*/ /* unsafe_function_ignore: sprintf */
	else
		return -EIO;
}

static ssize_t
sustainable_power_store(struct device *dev, struct device_attribute *devattr,
			const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	u32 sustainable_power;

	if (!tz->tzp)
		return -EIO;

	if (kstrtou32(buf, 10, &sustainable_power))
		return -EINVAL;

	tz->tzp->sustainable_power = sustainable_power;

#ifdef CONFIG_HISI_IPA_THERMAL
	update_pid_value(tz);
#endif

	return count;
}
#ifdef CONFIG_HISI_IPA_THERMAL
static ssize_t
boost_timeout_show(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);

	if (tz->tzp)
		return snprintf(buf, PAGE_SIZE, "%u\n", tz->tzp->boost_timeout);/* unsafe_function_ignore: snprintf */
	else
		return -EIO;
}

#define BOOST_TIMEOUT_THRES	5000
static ssize_t
boost_timeout_store(struct device *dev, struct device_attribute *devattr,
			const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	u32 boost_timeout;

	if (!tz->tzp)
		return -EIO;

	if (kstrtou32(buf, 10, &boost_timeout))
		return -EINVAL;

	if (boost_timeout > BOOST_TIMEOUT_THRES)
		return -EINVAL;

	if (boost_timeout)
		tz->tzp->boost_timeout = boost_timeout;

	trace_IPA_boost(current->pid, tz, tz->tzp->boost,
			tz->tzp->boost_timeout);
	return count;
}
static DEVICE_ATTR(boost_timeout, (S_IWUSR | S_IRUGO), boost_timeout_show,
		boost_timeout_store);
#endif

#ifdef CONFIG_HISI_IPA_THERMAL
#define  MAX_S16 0x7FFF
#endif

#define create_s32_tzp_attr(name)					\
	static ssize_t							\
	name##_show(struct device *dev, struct device_attribute *devattr, \
		char *buf)						\
	{								\
	struct thermal_zone_device *tz = to_thermal_zone(dev);		\
									\
	if (tz->tzp)							\
		return sprintf(buf, "%d\n", tz->tzp->name); /* unsafe_function_ignore: sprintf */ \
	else								\
		return -EIO;						\
	}								\
									\
	static ssize_t							\
	name##_store(struct device *dev, struct device_attribute *devattr, \
		const char *buf, size_t count)				\
	{								\
		struct thermal_zone_device *tz = to_thermal_zone(dev);	\
		s32 value;						\
									\
		if (!tz->tzp)						\
			return -EIO;					\
									\
		if (kstrtos32(buf, 10, &value))				\
			return -EINVAL;					\
									\
			if (value > MAX_S16)					\
				tz->tzp->name = MAX_S16;			\
			else if (value < -MAX_S16)				\
				tz->tzp->name = -MAX_S16;			\
			else								\
				tz->tzp->name = value;			\
									\
		return count;						\
	}								\
	static DEVICE_ATTR(name, S_IWUSR | S_IRUGO, name##_show, name##_store)

create_s32_tzp_attr(k_po);/*lint !e421 */
create_s32_tzp_attr(k_pu);/*lint !e421 */
create_s32_tzp_attr(k_i);/*lint !e421 */
create_s32_tzp_attr(k_d);/*lint !e421 */
create_s32_tzp_attr(integral_cutoff);/*lint !e421 */
create_s32_tzp_attr(slope);/*lint !e421 */
create_s32_tzp_attr(offset);/*lint !e421 */
#undef create_s32_tzp_attr

static ssize_t cur_power_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct thermal_zone_device *tz;
	struct thermal_instance *instance;
	ssize_t buf_len;
	ssize_t size = PAGE_SIZE;
	if (dev == NULL || attr == NULL || buf == NULL){
		return -EIO;
	}

	tz = to_thermal_zone(dev);
	if (tz == NULL || tz->tzp == NULL){
		return -EIO;
	}

	mutex_lock(&tz->lock);
	buf_len = scnprintf(buf, size, "%llu,%u,", tz->tzp->cur_ipa_total_power, tz->tzp->check_cnt);
	if (buf_len < 0)
		goto unlock;
	list_for_each_entry(instance, &tz->thermal_instances, tz_node) {
		struct thermal_cooling_device *cdev = instance->cdev;
		ssize_t ret;
		size = PAGE_SIZE - buf_len;
		if (size < 0 || size > (ssize_t)PAGE_SIZE)
			break;
		ret = scnprintf(buf + buf_len, size, "%llu,", cdev->cdev_cur_power);
		if (ret < 0)
			break;
		buf_len += ret;
	}
unlock:
	mutex_unlock(&tz->lock);

	if (buf_len > 0)
		buf[buf_len-1] = '\0';  ///set string end with '\0'
	return buf_len;
}

static DEVICE_ATTR(cur_power, S_IRUGO, cur_power_show, NULL);

static ssize_t
cur_enable_show(struct device *dev, struct device_attribute *devattr,
		       char *buf)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);

	if (tz->tzp)
		return scnprintf(buf, PAGE_SIZE, "%u\n", tz->tzp->cur_enable);/*lint !e421*/
	else
		return -EIO;
}

static ssize_t
cur_enable_store(struct device *dev, struct device_attribute *devattr,
			const char *buf, size_t count)
{
	struct thermal_zone_device *tz = to_thermal_zone(dev);
	u32 cur_enable;

	if (!tz->tzp)
		return -EIO;

	if (kstrtou32(buf, 10, &cur_enable))
		return -EINVAL;

	tz->tzp->cur_enable = (cur_enable > 0);

	return count;
}

static DEVICE_ATTR(cur_enable, S_IWUSR | S_IRUGO, cur_enable_show, cur_enable_store);


/*
 * These are thermal zone device attributes that will always be present.
 * All the attributes created for tzp (create_s32_tzp_attr) also are always
 * present on the sysfs interface.
 */
static DEVICE_ATTR(type, 0444, type_show, NULL);
static DEVICE_ATTR(temp, 0444, temp_show, NULL);
static DEVICE_ATTR(policy, S_IRUGO | S_IWUSR, policy_show, policy_store);
static DEVICE_ATTR(available_policies, S_IRUGO, available_policies_show, NULL);
static DEVICE_ATTR(sustainable_power, S_IWUSR | S_IRUGO, sustainable_power_show,
		   sustainable_power_store);/*lint !e421 */

/* These thermal zone device attributes are created based on conditions */
static DEVICE_ATTR(mode, 0644, mode_show, mode_store);
static DEVICE_ATTR(passive, S_IRUGO | S_IWUSR, passive_show, passive_store);

/* These attributes are unconditionally added to a thermal zone */
static struct attribute *thermal_zone_dev_attrs[] = {
	&dev_attr_type.attr,
	&dev_attr_temp.attr,
#if (IS_ENABLED(CONFIG_THERMAL_EMULATION))
	&dev_attr_emul_temp.attr,
#endif
	&dev_attr_policy.attr,
	&dev_attr_available_policies.attr,
	&dev_attr_sustainable_power.attr,
	&dev_attr_k_po.attr,
	&dev_attr_k_pu.attr,
	&dev_attr_k_i.attr,
	&dev_attr_k_d.attr,
	&dev_attr_integral_cutoff.attr,
	&dev_attr_slope.attr,
	&dev_attr_offset.attr,
#ifdef CONFIG_HISI_IPA_THERMAL
	&dev_attr_boost.attr,
	&dev_attr_boost_timeout.attr,
#endif
	&dev_attr_cur_power.attr,
	&dev_attr_cur_enable.attr,
	NULL,
};

static struct attribute_group thermal_zone_attribute_group = {
	.attrs = thermal_zone_dev_attrs,
};

/* We expose mode only if .get_mode is present */
static struct attribute *thermal_zone_mode_attrs[] = {
	&dev_attr_mode.attr,
	NULL,
};

static umode_t thermal_zone_mode_is_visible(struct kobject *kobj,
					    struct attribute *attr,
					    int attrno)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct thermal_zone_device *tz;

	tz = container_of(dev, struct thermal_zone_device, device);

	if (tz->ops->get_mode)
		return attr->mode;

	return 0;
}

static struct attribute_group thermal_zone_mode_attribute_group = {
	.attrs = thermal_zone_mode_attrs,
	.is_visible = thermal_zone_mode_is_visible,
};

/* We expose passive only if passive trips are present */
static struct attribute *thermal_zone_passive_attrs[] = {
	&dev_attr_passive.attr,
	NULL,
};

static umode_t thermal_zone_passive_is_visible(struct kobject *kobj,
					       struct attribute *attr,
					       int attrno)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct thermal_zone_device *tz;
	enum thermal_trip_type trip_type;
	int count, passive = 0;

	tz = container_of(dev, struct thermal_zone_device, device);

	for (count = 0; count < tz->trips && !passive; count++) {
		tz->ops->get_trip_type(tz, count, &trip_type);

		if (trip_type == THERMAL_TRIP_PASSIVE)
			passive = 1;
	}

	if (!passive)
		return attr->mode;

	return 0;
}

static struct attribute_group thermal_zone_passive_attribute_group = {
	.attrs = thermal_zone_passive_attrs,
	.is_visible = thermal_zone_passive_is_visible,
};

static const struct attribute_group *thermal_zone_attribute_groups[] = {
	&thermal_zone_attribute_group,
	&thermal_zone_mode_attribute_group,
	&thermal_zone_passive_attribute_group,
	/* This is not NULL terminated as we create the group dynamically */
};

/**
 * create_trip_attrs() - create attributes for trip points
 * @tz:		the thermal zone device
 * @mask:	Writeable trip point bitmap.
 *
 * helper function to instantiate sysfs entries for every trip
 * point and its properties of a struct thermal_zone_device.
 *
 * Return: 0 on success, the proper error value otherwise.
 */
static int create_trip_attrs(struct thermal_zone_device *tz, int mask)
{
	struct attribute **attrs;
	int indx;

	/* This function works only for zones with at least one trip */
	if (tz->trips <= 0)
		return -EINVAL;

	tz->trip_type_attrs = kcalloc(tz->trips, sizeof(*tz->trip_type_attrs),
				      GFP_KERNEL);
	if (!tz->trip_type_attrs)
		return -ENOMEM;

	tz->trip_temp_attrs = kcalloc(tz->trips, sizeof(*tz->trip_temp_attrs),
				      GFP_KERNEL);
	if (!tz->trip_temp_attrs) {
		kfree(tz->trip_type_attrs);
		return -ENOMEM;
	}

	if (tz->ops->get_trip_hyst) {
		tz->trip_hyst_attrs = kcalloc(tz->trips,
					      sizeof(*tz->trip_hyst_attrs),
					      GFP_KERNEL);
		if (!tz->trip_hyst_attrs) {
			kfree(tz->trip_type_attrs);
			kfree(tz->trip_temp_attrs);
			return -ENOMEM;
		}
	}

	attrs = kcalloc(tz->trips * 3 + 1, sizeof(*attrs), GFP_KERNEL);
	if (!attrs) {
		kfree(tz->trip_type_attrs);
		kfree(tz->trip_temp_attrs);
		if (tz->ops->get_trip_hyst)
			kfree(tz->trip_hyst_attrs);
		return -ENOMEM;
	}

	for (indx = 0; indx < tz->trips; indx++) {
		/* create trip type attribute */
		snprintf(tz->trip_type_attrs[indx].name, THERMAL_NAME_LENGTH,
			 "trip_point_%d_type", indx);

		sysfs_attr_init(&tz->trip_type_attrs[indx].attr.attr);
		tz->trip_type_attrs[indx].attr.attr.name =
						tz->trip_type_attrs[indx].name;
		tz->trip_type_attrs[indx].attr.attr.mode = S_IRUGO;
		tz->trip_type_attrs[indx].attr.show = trip_point_type_show;
#ifdef CONFIG_HISI_THERMAL_TSENSOR
		if (IS_ENABLED(CONFIG_THERMAL_WRITABLE_TRIPS) &&
			mask & (1 << indx)) {
			if (tz->ops->activate_trip_type) {
				tz->trip_type_attrs[indx].attr.attr.mode |= S_IWUSR;
				tz->trip_type_attrs[indx].attr.store =
								trip_point_type_activate;
			}
		}
#endif
		attrs[indx] = &tz->trip_type_attrs[indx].attr.attr;

		/* create trip temp attribute */
		snprintf(tz->trip_temp_attrs[indx].name, THERMAL_NAME_LENGTH,
			 "trip_point_%d_temp", indx);

		sysfs_attr_init(&tz->trip_temp_attrs[indx].attr.attr);
		tz->trip_temp_attrs[indx].attr.attr.name =
						tz->trip_temp_attrs[indx].name;
		tz->trip_temp_attrs[indx].attr.attr.mode = S_IRUGO;
		tz->trip_temp_attrs[indx].attr.show = trip_point_temp_show;
		if (IS_ENABLED(CONFIG_THERMAL_WRITABLE_TRIPS) &&
		    mask & (1 << indx)) {
			tz->trip_temp_attrs[indx].attr.attr.mode |= S_IWUSR;
			tz->trip_temp_attrs[indx].attr.store =
							trip_point_temp_store;
		}
		attrs[indx + tz->trips] = &tz->trip_temp_attrs[indx].attr.attr;

		/* create Optional trip hyst attribute */
		if (!tz->ops->get_trip_hyst)
			continue;
		snprintf(tz->trip_hyst_attrs[indx].name, THERMAL_NAME_LENGTH,
			 "trip_point_%d_hyst", indx);

		sysfs_attr_init(&tz->trip_hyst_attrs[indx].attr.attr);
		tz->trip_hyst_attrs[indx].attr.attr.name =
					tz->trip_hyst_attrs[indx].name;
		tz->trip_hyst_attrs[indx].attr.attr.mode = S_IRUGO;
		tz->trip_hyst_attrs[indx].attr.show = trip_point_hyst_show;
		if (tz->ops->set_trip_hyst) {
			tz->trip_hyst_attrs[indx].attr.attr.mode |= S_IWUSR;
			tz->trip_hyst_attrs[indx].attr.store =
					trip_point_hyst_store;
		}
		attrs[indx + tz->trips * 2] =
					&tz->trip_hyst_attrs[indx].attr.attr;
	}
	attrs[tz->trips * 3] = NULL;

	tz->trips_attribute_group.attrs = attrs;

	return 0;
}

/**
 * destroy_trip_attrs() - destroy attributes for trip points
 * @tz:		the thermal zone device
 *
 * helper function to free resources allocated by create_trip_attrs()
 */
static void destroy_trip_attrs(struct thermal_zone_device *tz)
{
	if (!tz)
		return;

	kfree(tz->trip_type_attrs);
	kfree(tz->trip_temp_attrs);
	if (tz->ops->get_trip_hyst)
		kfree(tz->trip_hyst_attrs);
	kfree(tz->trips_attribute_group.attrs);
}

int thermal_zone_create_device_groups(struct thermal_zone_device *tz,
				      int mask)
{
	const struct attribute_group **groups;
	int i, size, result;

	/* we need one extra for trips and the NULL to terminate the array */
	size = ARRAY_SIZE(thermal_zone_attribute_groups) + 2;
	/* This also takes care of API requirement to be NULL terminated */
	groups = kcalloc(size, sizeof(*groups), GFP_KERNEL);
	if (!groups)
		return -ENOMEM;

	for (i = 0; i < size - 2; i++)
		groups[i] = thermal_zone_attribute_groups[i];

	if (tz->trips) {
		result = create_trip_attrs(tz, mask);
		if (result) {
			kfree(groups);

			return result;
		}

		groups[size - 2] = &tz->trips_attribute_group;
	}

	tz->device.groups = groups;

	return 0;
}

void thermal_zone_destroy_device_groups(struct thermal_zone_device *tz)
{
	if (!tz)
		return;

	if (tz->trips)
		destroy_trip_attrs(tz);

	kfree(tz->device.groups);
}

/* sys I/F for cooling device */
static ssize_t
thermal_cooling_device_type_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct thermal_cooling_device *cdev = to_cooling_device(dev);

	return sprintf(buf, "%s\n", cdev->type);/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

static ssize_t
thermal_cooling_device_max_state_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct thermal_cooling_device *cdev = to_cooling_device(dev);
	unsigned long state;
	int ret;

	ret = cdev->ops->get_max_state(cdev, &state);
	if (ret)
		return ret;
	return sprintf(buf, "%ld\n", state);/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

static ssize_t
thermal_cooling_device_cur_state_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct thermal_cooling_device *cdev = to_cooling_device(dev);
	unsigned long state;
	int ret;

	ret = cdev->ops->get_cur_state(cdev, &state);
	if (ret)
		return ret;
	return sprintf(buf, "%ld\n", state);/*lint !e421 */ /* unsafe_function_ignore: sprintf */
}

static ssize_t
thermal_cooling_device_cur_state_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	struct thermal_cooling_device *cdev = to_cooling_device(dev);
	unsigned long state;
	int result;

	if (sscanf(buf, "%ld\n", &state) != 1) /* unsafe_function_ignore: sscanf */
		return -EINVAL;

	if ((long)state < 0)
		return -EINVAL;

	result = cdev->ops->set_cur_state(cdev, state);
	if (result)
		return result;
	return count;
}

static struct device_attribute dev_attr_cdev_type =
__ATTR(type, 0444, thermal_cooling_device_type_show, NULL);
static DEVICE_ATTR(max_state, 0444,
		   thermal_cooling_device_max_state_show, NULL);
static DEVICE_ATTR(cur_state, 0644,
		   thermal_cooling_device_cur_state_show,
		   thermal_cooling_device_cur_state_store);

static struct attribute *cooling_device_attrs[] = {
	&dev_attr_cdev_type.attr,
	&dev_attr_max_state.attr,
	&dev_attr_cur_state.attr,
	NULL,
};

static const struct attribute_group cooling_device_attr_group = {
	.attrs = cooling_device_attrs,
};

static const struct attribute_group *cooling_device_attr_groups[] = {
	&cooling_device_attr_group,
	NULL,
};

void thermal_cooling_device_setup_sysfs(struct thermal_cooling_device *cdev)
{
	cdev->device.groups = cooling_device_attr_groups;
}

/* these helper will be used only at the time of bindig */

