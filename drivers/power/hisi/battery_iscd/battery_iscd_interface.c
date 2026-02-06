/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: dts and sysfs operation for iscd.
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

#include "battery_iscd_interface.h"

void set_fatal_isc_action(struct iscd_info *iscd)
{
	if (iscd == NULL) {
		iscd_core_err("iscd is Null found in %s\n", __func__);
		return;
	}

	if (iscd->fatal_isc_trigger_type)
		power_event_bnc_cond_register(POWER_BNT_CHG,
			&iscd->isc_listen_to_charge_event_nb);
	else
		power_event_bnc_unregister(POWER_BNT_CHG,
			&iscd->isc_listen_to_charge_event_nb);

	if (iscd->fatal_isc_action & BIT(NORAML_CHARGING_ACTION))
		blocking_notifier_chain_cond_register(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_chg_limit_soc_nb);
	else
		blocking_notifier_chain_unregister(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_chg_limit_soc_nb);

	if (iscd->fatal_isc_action & BIT(UPLOAD_UEVENT_ACTION))
		blocking_notifier_chain_cond_register(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_uevent_notify_nb);
	else
		blocking_notifier_chain_unregister(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_uevent_notify_nb);

	if (iscd->fatal_isc_action & BIT(DIRECT_CHARGING_ACTION))
		blocking_notifier_chain_cond_register(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_direct_chg_limit_soc_nb);
	else
		blocking_notifier_chain_unregister(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_direct_chg_limit_soc_nb);

	if (iscd->fatal_isc_action & BIT(UPDATE_OCV_ACTION))
		blocking_notifier_chain_cond_register(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_ocv_update_nb);
	else
		blocking_notifier_chain_unregister(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_ocv_update_nb);

	if (iscd->fatal_isc_action & BIT(UPLOAD_DMD_ACTION))
		blocking_notifier_chain_cond_register(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_dsm_report_nb);
	else
		blocking_notifier_chain_unregister(&iscd->isc_limit_func_head,
			&iscd->fatal_isc_dsm_report_nb);
}


void fatal_isc_protection(struct iscd_info *iscd, unsigned long event)
{
	int capacity = get_bci_soc();

	if (iscd == NULL) {
		iscd_core_err("iscd is Null found in %s\n", __func__);
		return;
	}

	blocking_notifier_call_chain(&iscd->isc_limit_func_head,
		event, &capacity);
	if (iscd->need_monitor)
		schedule_delayed_work(&iscd->isc_limit_work,
			msecs_to_jiffies(ISC_LIMIT_CHECKING_INTERVAL));
}

static ssize_t isc_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"),
			sizeof("Error") - 1, "%s", "Error");
	}

	iscd->fatal_isc_action = iscd->fatal_isc_action_dts;
	set_fatal_isc_action(iscd);
	spin_lock(&iscd->boot_complete);
	iscd->app_ready = 1;

	if (!iscd->isc_splash2_ready) {
		spin_unlock(&iscd->boot_complete);
		schedule_delayed_work(&iscd->isc_splash2_work, 0);
	} else if (iscd->uevent_wait_for_send) {
		iscd->uevent_wait_for_send = 0;
		spin_unlock(&iscd->boot_complete);
		fatal_isc_protection(iscd, ISC_LIMIT_BOOT_STAGE);
	} else {
		spin_unlock(&iscd->boot_complete);
	}

	if (iscd->fatal_isc_action == FATAL_ISC_ACTION_DMD_ONLY)
		return snprintf_s(buf, sizeof(iscd->isc_status),
			sizeof(iscd->isc_status) - 1, "%d", 0);

	return snprintf_s(buf, sizeof(iscd->isc_status),
		sizeof(iscd->isc_status) - 1, "%d", iscd->isc_status);
}

static ssize_t isc_shutdown_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	spin_lock(&iscd->boot_complete);
	if (!iscd->isc_splash2_ready) {
		spin_unlock(&iscd->boot_complete);
		schedule_delayed_work(&iscd->isc_splash2_work, 0);
	} else if (iscd->uevent_wait_for_send && iscd->has_reported) {
		iscd->uevent_wait_for_send = 0;
		spin_unlock(&iscd->boot_complete);
		iscd->fatal_isc_action = iscd->fatal_isc_action_dts;
		set_fatal_isc_action(iscd);
		fatal_isc_protection(iscd, ISC_LIMIT_BOOT_STAGE);
	} else {
		spin_unlock(&iscd->boot_complete);
	}

	if (iscd->has_reported &&
		(iscd->fatal_isc_action != FATAL_ISC_ACTION_DMD_ONLY))
		return snprintf_s(buf, sizeof(iscd->isc_status),
			sizeof(iscd->isc_status) - 1, "%d", iscd->isc_status);
	else
		return snprintf_s(buf, sizeof(iscd->isc_status),
			sizeof(iscd->isc_status) - 1, "%d", 0);
}

static ssize_t isc_limit_support_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	return snprintf_s(buf, sizeof(iscd->fatal_isc_trigger_type),
		sizeof(iscd->fatal_isc_trigger_type) - 1, "%d",
		iscd->fatal_isc_trigger_type);
}

static DEVICE_ATTR_RO(isc);
static DEVICE_ATTR_RO(isc_shutdown_status);
static DEVICE_ATTR_RO(isc_limit_support);

static struct attribute *isc_func_attrs[] = {
	&dev_attr_isc.attr,
	&dev_attr_isc_shutdown_status.attr,
	&dev_attr_isc_limit_support.attr,
	NULL,
};

#ifdef ISC_TEST
static ssize_t isc_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"%d\n", iscd->isc_status);
}

static ssize_t isc_status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);
	long val = 0;

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return -EINVAL;
	}

	if (kstrtol(buf, DEC, &val) < 0)
		return -EINVAL;
	iscd->isc_status = val ? 1 : 0;

	return count;
}

static ssize_t fatal_isc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);
	int i, ret;
	int val = 0;

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"status:%02x trigger:%02x valid num:%02x dmd:%02x reported:%02x version:%08x\n",
		iscd->fatal_isc_hist.isc_status,
		iscd->fatal_isc_hist.trigger_type,
		iscd->fatal_isc_hist.valid_num,
		iscd->fatal_isc_hist.dmd_report,
		iscd->fatal_isc_config.has_reported,
		iscd->fatal_isc_hist.magic_num);
	if (ret != -1)
		val += ret;
	ret = snprintf_s(buf + val, PAGE_SIZE, PAGE_SIZE - 1, "%11s%11s%11s%11s%11s%6s%6s\n",
		"ISC(uA)", "FCC", "RM", "QMAX", "CYCLES", "YEAR", "YDAY");
	if (ret != -1)
		val += ret;
	for (i = 0; i < MAX_FATAL_ISC_NUM; i++) {
		ret = snprintf_s(buf + val, PAGE_SIZE, PAGE_SIZE - 1, "%11d%11d%11d%11d%11d%6d%6d\n",
			iscd->fatal_isc_hist.isc[i],
			iscd->fatal_isc_hist.fcc[i],
			iscd->fatal_isc_hist.rm[i],
			iscd->fatal_isc_hist.qmax[i],
			iscd->fatal_isc_hist.charge_cycles[i],
			iscd->fatal_isc_hist.year[i],
			iscd->fatal_isc_hist.yday[i]);
		if (ret != -1)
			val += ret;
		if (val > (int)(PAGE_SIZE >> 1))
			break;
	}
	return val;
}

static ssize_t fatal_isc_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);
	long val = 0;

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return -EINVAL;
	}

	if ((kstrtol(buf, DEC, &val) < 0) || (val < 0))
		return -EINVAL;
	iscd->isc = val;
	iscd_core_info("one fatal isc set to %ld\n", val);
	update_isc_hist(iscd, smallest_in_oneday);
	if (iscd->isc_status) {
		iscd->enable = DISABLED;
		fatal_isc_protection(iscd, ISC_LIMIT_BOOT_STAGE);
	}
	return count;
}

static ssize_t isc_prompt_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);
	int val;

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	val = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", iscd->isc_prompt);
	iscd->isc_prompt = 0;
	return val;
}

static ssize_t isc_prompt_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return -EINVAL;
	}

	__fatal_isc_uevent(&iscd->fatal_isc_uevent_work);
	return count;
}

static ssize_t isc_dmd_only_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		!(iscd->fatal_isc_action & (~BIT(UPLOAD_DMD_ACTION))));
}

static ssize_t isc_dmd_only_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);
	long val = 0;

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return -EINVAL;
	}

	if (kstrtol(buf, DEC, &val) < 0)
		return -EINVAL;
	val = !!val;
	iscd->fatal_isc_action = 0;
	if (iscd->fatal_isc_trigger_type != INVALID_ISC_JUDGEMENT) {
		if (val != FATAL_ISC_DMD_ONLY) {
			iscd->fatal_isc_action |= BIT(NORAML_CHARGING_ACTION);
			iscd->fatal_isc_action |= BIT(DIRECT_CHARGING_ACTION);
			iscd->fatal_isc_action |= BIT(UPLOAD_UEVENT_ACTION);
		}
		iscd->fatal_isc_action |= BIT(UPLOAD_DMD_ACTION);
	}
	set_fatal_isc_action(iscd);
	return count;
}

static ssize_t isc_trigger_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		iscd->fatal_isc_trigger_type);
}

static ssize_t isc_charge_limit_soc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		iscd->fatal_isc_soc_limit[UPLIMIT]);
}

static ssize_t isc_charge_limit_soc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);
	long val = 0;

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return -EINVAL;
	}

	if ((kstrtol(buf, DEC, &val) < 0) || (val < FATAL_ISC_SOC_LIMIT_LOWER) ||
		(val > FATAL_ISC_SOC_LIMIT_UPPER))
		return -EINVAL;
	iscd->fatal_isc_soc_limit[UPLIMIT] =
		(val <= iscd->fatal_isc_soc_limit[RECHARGE]) ?
			(iscd->fatal_isc_soc_limit[RECHARGE] + 1) : val;
	return count;
}

static ssize_t isc_recharge_soc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		iscd->fatal_isc_soc_limit[RECHARGE]);
}

static ssize_t isc_recharge_soc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);
	long val = 0;

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return -EINVAL;
	}

	if ((kstrtol(buf, DEC, &val) < 0) || (val < FATAL_ISC_SOC_LIMIT_LOWER) ||
		(val > FATAL_ISC_SOC_LIMIT_UPPER))
		return -EINVAL;
	iscd->fatal_isc_soc_limit[RECHARGE] =
		(val >= iscd->fatal_isc_soc_limit[UPLIMIT]) ?
			(iscd->fatal_isc_soc_limit[UPLIMIT] - 1) : val;
	return count;
}

static ssize_t isc_valid_cycles_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		iscd->isc_valid_cycles);
}

static ssize_t isc_valid_cycles_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);
	long val = 0;

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return -EINVAL;
	}

	if (kstrtol(buf, DEC, &val) < 0)
		return -EINVAL;
	iscd->isc_valid_cycles = val;
	return count;
}

static ssize_t isc_monitor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		iscd->need_monitor);
}

static ssize_t isc_valid_delay_cycles_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return snprintf_s(buf, sizeof("Error"), sizeof("Error") - 1,
			"%s", "Error");
	}

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		iscd->isc_valid_delay_cycles);
}

static ssize_t isc_valid_delay_cycles_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct iscd_info *iscd = dev_get_drvdata(dev);
	long val = 0;

	if (iscd == NULL) {
		iscd_core_err("dev is Null found in %s\n", __func__);
		return -EINVAL;
	}

	if (kstrtol(buf, DEC, &val) < 0)
		return -EINVAL;

	iscd->isc_valid_delay_cycles = val;
	return count;
}

static DEVICE_ATTR_RW(isc_status);
static DEVICE_ATTR_RW(fatal_isc);
static DEVICE_ATTR_RW(isc_prompt);
static DEVICE_ATTR_RW(isc_dmd_only);
static DEVICE_ATTR_RO(isc_trigger_type);
static DEVICE_ATTR_RW(isc_charge_limit_soc);
static DEVICE_ATTR_RW(isc_recharge_soc);
static DEVICE_ATTR_RW(isc_valid_cycles);
static DEVICE_ATTR_RO(isc_monitor);
static DEVICE_ATTR_RW(isc_valid_delay_cycles);

static struct attribute *isc_test_attrs[] = {
	&dev_attr_isc_status.attr,
	&dev_attr_fatal_isc.attr,
	&dev_attr_isc_prompt.attr,
	&dev_attr_isc_dmd_only.attr,
	&dev_attr_isc_trigger_type.attr,
	&dev_attr_isc_charge_limit_soc.attr,
	&dev_attr_isc_recharge_soc.attr,
	&dev_attr_isc_valid_cycles.attr,
	&dev_attr_isc_monitor.attr,
	&dev_attr_isc_valid_delay_cycles.attr,
	NULL,
};

static const struct attribute_group isc_test_group = {
	.name = "isc_test",
	.attrs = isc_test_attrs,
};
#endif

static const struct attribute_group isc_func_group = {
	.attrs = isc_func_attrs,
};

static const struct attribute_group *isc_groups[] = {
	&isc_func_group,
#ifdef ISC_TEST
	&isc_test_group,
#endif
	NULL,
};

void iscd_create_sysfs(struct iscd_info *iscd)
{
	int ret;
	struct class *hw_power = NULL;
	struct device *battery = NULL;

	if (iscd == NULL) {
		iscd_core_err("iscd in %s is NULL\n", __func__);
		return;
	}

	hw_power = power_sysfs_get_class("hw_power");
	if (hw_power == NULL)
		iscd_core_err("Can't get hw_power class in %s\n", __func__);
	else
		battery = device_create(hw_power, NULL, 0, iscd, "battery");

	if (IS_ERR_OR_NULL(battery)) {
		iscd_core_err("Can't create device battery in %s\n", __func__);
	} else {
		ret = sysfs_create_groups(&battery->kobj, isc_groups);
		if (ret)
			iscd_core_err("creat isc attribute groups under battery failed in %s\n",
				__func__);
	}
}

static void get_iscd_dts_info_part_one(struct device_node *np,
	struct iscd_info *iscd)
{
	int ret;

	ret = of_property_read_s32(np, "iscd_enable", &iscd->enable);
	if (ret) {
		iscd_core_err("get iscd_enable fail, use default one\n");
		iscd->enable = DISABLED;
	}
	iscd_core_info("ISCD iscd_enable = %d\n", iscd->enable);

	ret = of_property_read_s32(np, "iscd_ocv_min", &iscd->ocv_min);
	if (ret) {
		iscd_core_err("get iscd_ocv_min fail, use default one\n");
		iscd->ocv_min = ISCD_DEFAULT_OCV_MIN;
	}
	iscd_core_info("ISCD ocv_min = %d\n", iscd->ocv_min);

	ret = of_property_read_s32(np, "iscd_batt_temp_min", &iscd->tbatt_min);
	if (ret) {
		iscd_core_err("get iscd_batt_temp_min fail, use default one\n");
		iscd->tbatt_min = ISCD_DEFAULT_TBATT_MIN;
	}
	iscd_core_info("ISCD tbatt_min = %d\n", iscd->tbatt_min);

	ret = of_property_read_s32(np, "iscd_batt_temp_max", &iscd->tbatt_max);
	if (ret) {
		iscd_core_err("get iscd_batt_temp_max fail, use default one\n");
		iscd->tbatt_max = ISCD_DEFAULT_TBATT_MAX;
	}
	iscd_core_info("ISCD tbatt_max = %d\n", iscd->tbatt_max);

	ret = of_property_read_s32(np, "iscd_batt_temp_diff_max",
		&iscd->tbatt_diff_max);
	if (ret) {
		iscd_core_err("get iscd_batt_temp_diff_max fail, use default one\n");
		iscd->tbatt_diff_max = ISCD_DEFAULT_TBATT_DIFF;
	}
	iscd_core_info("ISCD tbatt_diff_max = %d\n", iscd->tbatt_diff_max);

	ret = of_property_read_s32(np, "iscd_sample_time_interval",
		&iscd->sample_time_interval);
	if (ret) {
		iscd_core_err("get iscd_sample_time_interval fail, use default one\n");
		iscd->sample_time_interval = ISCD_DEFAULT_SAMPLE_INTERVAL;
	}
	iscd_core_info("ISCD sample_time_interval = %d\n",
			iscd->sample_time_interval);
}

static void get_iscd_dts_info_part_two(struct device_node *np,
	struct iscd_info *iscd)
{
	int ret;

	ret = of_property_read_s32(np, "iscd_sample_time_delay",
		&iscd->sample_time_delay);
	if (ret) {
		iscd_core_err("get iscd_sample_time_delay fail, use default one\n");
		iscd->sample_time_delay = ISCD_DEFAULT_SAMPLE_DELAY;
	}
	iscd_core_info("ISCD sample_time_delay = %d\n", iscd->sample_time_delay);

	ret = of_property_read_s32(np, "iscd_calc_time_interval_min",
		&iscd->calc_time_interval_min);
	if (ret) {
		iscd_core_err("get iscd_calc_time_interval_min fail, use default one\n");
		iscd->calc_time_interval_min = ISCD_DEFAULT_CALC_INTERVAL_MIN;
	}
	iscd_core_info("ISCD calc_time_interval_min = %d\n",
			iscd->calc_time_interval_min);

	ret = of_property_read_s32(np, "iscd_level_warning_threhold",
		&iscd->isc_warning_threhold);
	if (ret) {
		iscd_core_err("get iscd_level_warning_threhold fail, use default one\n");
		iscd->isc_warning_threhold = ISCD_WARNING_LEVEL_THREHOLD;
	}
	iscd_core_info("ISCD isc_warning_threhold = %d\n",
		iscd->isc_warning_threhold);

	ret = of_property_read_s32(np, "iscd_level_error_threhold",
		&iscd->isc_error_threhold);
	if (ret) {
		iscd_core_err("get iscd_level_error_threhold fail, use default one\n");
		iscd->isc_error_threhold = ISCD_ERROR_LEVEL_THREHOLD;
	}
	iscd_core_info("ISCD isc_error_threhold = %d\n",
		iscd->isc_error_threhold);

	ret = of_property_read_s32(np, "iscd_level_critical_threhold",
		&iscd->isc_critical_threhold);
	if (ret) {
		iscd_core_err("get iscd_level_critical_threhold fail, use default one\n");
		iscd->isc_critical_threhold = ISCD_CRITICAL_LEVEL_THREHOLD;
	}
	iscd_core_info("ISCD isc_critical_threhold = %d\n",
		iscd->isc_critical_threhold);
}

static void get_iscd_dts_info_part_three(struct device_node *np,
	struct iscd_info *iscd)
{
	int ret;

	ret = of_property_read_s32(np, "iscd_chrg_delay_cycles",
		&iscd->isc_chrg_delay_cycles);
	if (ret) {
		iscd_core_err("get iscd_chrg_delay_cycles fail, use default one\n");
		iscd->isc_chrg_delay_cycles = ISCD_CHRG_DELAY_CYCLES;
	}
	iscd_core_info("ISCD isc_chrg_delay_cycles = %d\n",
		iscd->isc_chrg_delay_cycles);

	ret = of_property_read_s32(np, "iscd_delay_cycles_enable",
		&iscd->isc_delay_cycles_enable);
	if (ret) {
		iscd_core_err("get iscd_delay_cycles_enable fail, use default one\n");
		iscd->isc_delay_cycles_enable = ISCD_DELAY_CYCLES_ENABLE;
	}
	iscd_core_info("ISCD iscd_delay_cycles_enable = %d\n",
		iscd->isc_delay_cycles_enable);

	ret = of_property_read_s32(np, "iscd_file_magic_num",
		&iscd->iscd_file_magic_num);
	if (ret) {
		iscd_core_err("get iscd_file_magic_num fail, use default one\n");
		iscd->iscd_file_magic_num = FATAL_ISC_MAGIC_NUM;
	}
	iscd_core_info("ISCD iscd_file_magic_num = %d\n",
		iscd->iscd_file_magic_num);
}

static void update_iscd_level_config(struct iscd_info *iscd,
	struct device_node *np, int array_len)
{
	int ret, i;
	u32 config_tmp[ISCD_LEVEL_CONFIG_CNT * ISCD_MAX_LEVEL] = {0};

	ret = of_property_read_u32_array(np, "iscd_level_info",
		config_tmp, (unsigned long)(long)array_len);
	if (ret) {
		iscd->total_level  = 0;
		iscd_core_err("ISCD dts:get iscd_level_info fail\n");
	} else {
		iscd->total_level  = array_len / ISCD_LEVEL_CONFIG_CNT;
		for (i = 0; i < iscd->total_level; i++) {
			/* (int) for pclint and can never be out of bounds */
			iscd->level_config[i].isc_min =
				(int)config_tmp[(int)(ISCD_ISC_MIN +
					ISCD_LEVEL_CONFIG_CNT * i)];
			iscd->level_config[i].isc_max =
				(int)config_tmp[(int)(ISCD_ISC_MAX +
					ISCD_LEVEL_CONFIG_CNT * i)];
			iscd->level_config[i].dsm_err_no =
				(int)config_tmp[(int)(ISCD_DSM_ERR_NO +
					ISCD_LEVEL_CONFIG_CNT * i)];
			iscd->level_config[i].dsm_report_cnt =
				(int)config_tmp[(int)(ISCD_DSM_REPORT_CNT +
					ISCD_LEVEL_CONFIG_CNT * i)];
			iscd->level_config[i].dsm_report_time =
				config_tmp[(int)(ISCD_DSM_REPORT_TIME +
					ISCD_LEVEL_CONFIG_CNT * i)];
			iscd->level_config[i].protection_type =
				(int)config_tmp[(int)(ISCD_PROTECTION_TYPE +
					ISCD_LEVEL_CONFIG_CNT * i)];
			iscd_core_info("ISCD level[%d], isc_min: %-6d isc_max: %-7d dsm_err_no: %-9d dsm_report_cnt: %d dsm_report_time:%ld dsm_protection_type:%d\n",
				i, iscd->level_config[i].isc_min,
				iscd->level_config[i].isc_max,
				iscd->level_config[i].dsm_err_no,
				iscd->level_config[i].dsm_report_cnt,
				iscd->level_config[i].dsm_report_time,
				iscd->level_config[i].protection_type);
		}
	}
}

static void coul_core_get_iscd_dsm_config(struct device_node *np,
	struct iscd_info *iscd)
{
	int array_len;

	/* iscd dsm config para */
	array_len = of_property_count_u32_elems(np, "iscd_level_info");
	if ((array_len <= 0) || (array_len % ISCD_LEVEL_CONFIG_CNT != 0)) {
		iscd->total_level = 0;
		iscd_core_err("ISCD iscd_level_info is invaild, please check iscd_level_info number\n");
	} else if (array_len > (int)ISCD_MAX_LEVEL * ISCD_LEVEL_CONFIG_CNT) {
		iscd->total_level  = 0;
		iscd_core_err("ISCD iscd_level_info is too long, use only front %d paras\n",
			array_len);
	} else {
		update_iscd_level_config(iscd, np, array_len);
	}
}

void coul_core_get_iscd_info(struct device_node *np, struct iscd_info *iscd)
{
	if ((np == NULL) || (iscd == NULL)) {
		iscd_core_err("iscd in %s is NULL\n", __func__);
		return;
	}

	get_iscd_dts_info_part_one(np, iscd);
	get_iscd_dts_info_part_two(np, iscd);
	get_iscd_dts_info_part_three(np, iscd);
	coul_core_get_iscd_dsm_config(np, iscd);
	iscd->isc_valid_cycles = ISCD_CHARGE_CYCLE_MIN;
}

