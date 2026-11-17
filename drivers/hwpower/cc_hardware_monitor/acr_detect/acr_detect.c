/*
 * acr_detect.c
 *
 * acr abnormal monitor driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "acr_detect.h"
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG acr_detect
HWLOG_REGIST();

static struct acr_det_dev *g_acr_det_dev;

#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
static void acr_det_fmd_report(struct acr_det_dev *l_dev, int acr_val)
{
	char *batt_brand = power_platform_get_battery_brand();
	int batt_volt = power_platform_get_battery_voltage();
	int batt_temp = power_platform_get_battery_temperature();
	struct message *acr_msg = NULL;

	acr_msg = kzalloc(sizeof(*acr_msg), GFP_KERNEL);
	if (!acr_msg)
		return;

	acr_msg->version = ACR_DET_DEFAULT_VERSION;
	acr_msg->data_source = DATA_FROM_KERNEL;
	acr_msg->num_events = 1;
	acr_msg->events[0].error_code = 0;
	acr_msg->events[0].item_id = ACR_DET_ITEM_ID;
	acr_msg->events[0].cycle = 0;
	memcpy(acr_msg->events[0].station, ACR_DET_NA,
		sizeof(ACR_DET_NA));
	memcpy(acr_msg->events[0].bsn, ACR_DET_NA,
		sizeof(ACR_DET_NA));
	memcpy(acr_msg->events[0].time, ACR_DET_NA,
		sizeof(ACR_DET_NA));
	memcpy(acr_msg->events[0].device_name, ACR_DET_DEVICE_NAME,
		sizeof(ACR_DET_DEVICE_NAME));
	memcpy(acr_msg->events[0].test_name, ACR_DET_TEST_NAME,
		sizeof(ACR_DET_TEST_NAME));
	snprintf(acr_msg->events[0].min_threshold, MAX_VAL_LEN,
		"%u", l_dev->rt_fmd_min);
	snprintf(acr_msg->events[0].max_threshold, MAX_VAL_LEN,
		"%u", l_dev->rt_fmd_max);

	if (acr_val > l_dev->rt_threshold)
		memcpy(acr_msg->events[0].result, ACR_DET_FMD_FAIL,
			sizeof(ACR_DET_FMD_FAIL));
	else
		memcpy(acr_msg->events[0].result, ACR_DET_FMD_PASS,
			sizeof(ACR_DET_FMD_PASS));

	snprintf(acr_msg->events[0].value, MAX_VAL_LEN, "%d", acr_val);
	memcpy(acr_msg->events[0].firmware, batt_brand, MAX_FIRMWARE_LEN);
	snprintf(acr_msg->events[0].description, MAX_DESCRIPTION_LEN,
		"batt_volt:%dmV, batt_temp:%d", batt_volt, batt_temp);

	power_dsm_report_fmd(POWER_DSM_BATTERY,
		DA_BATTERY_ACR_ERROR_NO, acr_msg);
	kfree(acr_msg);
}
#else
static inline void acr_det_fmd_report(struct acr_det_dev *l_dev, int acr_val)
{
}
#endif /* CONFIG_HUAWEI_DATA_ACQUISITION */

static void acr_det_dmd_report(int acr_val)
{
	char buf[POWER_DSM_BUF_SIZE_0128] = { 0 };

	snprintf(buf, POWER_DSM_BUF_SIZE_0128 - 1,
		"brand:%s, volt:%d, curr:%d, soc:%d, temp:%d, acr:%d\n",
		power_platform_get_battery_brand(),
		power_platform_get_battery_voltage(),
		-power_platform_get_battery_current(),
		power_platform_get_battery_capacity(),
		power_platform_get_battery_temperature(),
		acr_val);
	power_dsm_report_dmd(POWER_DSM_BATTERY,
		ERROR_BATT_ACR_OVER_THRESHOLD, buf);
}

static int acr_det_start_detect(struct acr_det_dev *l_dev)
{
	int ret;
	int acr_value = 0;

	/* first: calculate acr */
	power_platform_start_acr_calc();

	/* second: sleep 1000ms, wait for acr calculate finish */
	msleep(1000);

	/* third: get acr resistance */
	ret = power_platform_get_acr_resistance(&acr_value);
	if (ret == 0) {
		hwlog_info("acr:%d, threshold:%d\n", acr_value, l_dev->rt_threshold);
		acr_det_fmd_report(l_dev, acr_value);
		/* sleep 50ms, wait for dsm_battery client free */
		msleep(50);
		if (acr_value > l_dev->rt_threshold)
			acr_det_dmd_report(acr_value);
	}

	return ret;
}

static void acr_det_rt_work(struct work_struct *work)
{
	struct acr_det_dev *l_dev = g_acr_det_dev;
	int i;
	int ichg;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return;
	}

	l_dev->rt_run = ACR_DET_RUNNING;

	if (power_platform_set_max_input_current())
		goto detect_fail;

	for (i = 0; i < ACR_DET_RETRY_TIMES; i++) {
		ichg = -power_platform_get_battery_current();
		hwlog_info("i:%d, ichg:%d\n", i, ichg);
		if (abs(ichg) < ACR_DET_MIN_CURRENT) {
			if (acr_det_start_detect(l_dev) == 0) {
				l_dev->rt_status = ACR_DET_STATUS_SUCC;
				l_dev->rt_run = ACR_DET_NOT_RUNNING;
				hwlog_info("acr detect succ\n");
				return;
			}
			goto detect_fail;
		}
		/* sleep 100ms to stability for next check */
		msleep(100);
	}

detect_fail:
	l_dev->rt_status = ACR_DET_STATUS_FAIL;
	l_dev->rt_run = ACR_DET_NOT_RUNNING;
	hwlog_err("acr detect fail\n");
}

#ifdef CONFIG_SYSFS
static ssize_t acr_det_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t acr_det_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info acr_det_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(acr_det, 0440, ACR_DET_SYSFS_RT_SUPPORT, acr_rt_support),
	power_sysfs_attr_rw(acr_det, 0640, ACR_DET_SYSFS_RT_DETECT, acr_rt_detect),
};

#define ACR_DET_SYSFS_ATTRS_SIZE  ARRAY_SIZE(acr_det_sysfs_field_tbl)

static struct attribute *acr_det_sysfs_attrs[ACR_DET_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group acr_det_sysfs_attr_group = {
	.attrs = acr_det_sysfs_attrs,
};

static ssize_t acr_det_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct acr_det_dev *l_dev = g_acr_det_dev;

	info = power_sysfs_lookup_attr(attr->attr.name,
		acr_det_sysfs_field_tbl, ACR_DET_SYSFS_ATTRS_SIZE);
	if (!info || !l_dev)
		return -EINVAL;

	switch (info->name) {
	case ACR_DET_SYSFS_RT_SUPPORT:
		return snprintf(buf, PAGE_SIZE, "%u\n", l_dev->rt_support);
	case ACR_DET_SYSFS_RT_DETECT:
		return snprintf(buf, PAGE_SIZE, "%d\n", l_dev->rt_status);
	default:
		return 0;
	}
}

static ssize_t acr_det_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct acr_det_dev *l_dev = g_acr_det_dev;
	int val = 0;
	int ret;

	info = power_sysfs_lookup_attr(attr->attr.name,
		acr_det_sysfs_field_tbl, ACR_DET_SYSFS_ATTRS_SIZE);
	if (!info || !l_dev)
		return -EINVAL;

	if (!l_dev->rt_support) {
		hwlog_info("acr detect not support\n");
		return -EINVAL;
	}

	if (!power_cmdline_is_factory_mode()) {
		hwlog_info("acr detect not support on normal mode\n");
		return -EINVAL;
	}

	switch (info->name) {
	case ACR_DET_SYSFS_RT_DETECT:
		/* 1: only set value to 1 start acr detect */
		ret = kstrtoint(buf, POWER_BASE_DEC, &val);
		if ((ret < 0) || (val != 1)) {
			hwlog_err("unable to parse input:%s\n", buf);
			return -EINVAL;
		}

		if (l_dev->rt_run == ACR_DET_RUNNING) {
			hwlog_info("acr detect is running\n");
			return -EINVAL;
		}

		l_dev->rt_status = ACR_DET_STATUS_INIT;
		schedule_delayed_work(&l_dev->rt_work, msecs_to_jiffies(0));
		break;
	default:
		break;
	}

	return count;
}

static void acr_det_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(acr_det_sysfs_attrs,
		acr_det_sysfs_field_tbl, ACR_DET_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "acr",
		dev, &acr_det_sysfs_attr_group);
}

static void acr_det_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "acr",
		dev, &acr_det_sysfs_attr_group);
}
#else
static inline void acr_det_sysfs_create_group(struct device *dev)
{
}

static inline void acr_det_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void acr_det_parse_dts(struct device_node *np,
	struct acr_det_dev *l_dev)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"acr_rt_support", &l_dev->rt_support, ACR_DET_NOT_SUPPORT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"acr_rt_threshold", &l_dev->rt_threshold, ACR_DET_DEFAULT_THLD);
	l_dev->rt_threshold *= THOUSAND_UNIT;
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"acr_rt_fmd_min", &l_dev->rt_fmd_min, ACR_DET_DEFAULT_MIN_FMD);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"acr_rt_fmd_max", &l_dev->rt_fmd_max, ACR_DET_DEFAULT_MAX_FMD);
}

static int acr_det_probe(struct platform_device *pdev)
{
	struct acr_det_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_acr_det_dev = l_dev;
	l_dev->dev = &pdev->dev;
	np = pdev->dev.of_node;
	acr_det_parse_dts(np, l_dev);
	INIT_DELAYED_WORK(&l_dev->rt_work, acr_det_rt_work);
	acr_det_sysfs_create_group(l_dev->dev);
	platform_set_drvdata(pdev, l_dev);

	return 0;
}

static int acr_det_remove(struct platform_device *pdev)
{
	struct acr_det_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	acr_det_sysfs_remove_group(l_dev->dev);
	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_acr_det_dev = NULL;

	return 0;
}

static const struct of_device_id acr_det_match_table[] = {
	{
		.compatible = "huawei,batt_acr_detect",
		.data = NULL,
	},
	{},
};

static struct platform_driver acr_det_driver = {
	.probe = acr_det_probe,
	.remove = acr_det_remove,
	.driver = {
		.name = "huawei,batt_acr_detect",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(acr_det_match_table),
	},
};

static int __init acr_det_init(void)
{
	return platform_driver_register(&acr_det_driver);
}

static void __exit acr_det_exit(void)
{
	platform_driver_unregister(&acr_det_driver);
}

late_initcall(acr_det_init);
module_exit(acr_det_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("acr abnormal monitor driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
