/*
 * acr_detect.c
 *
 * acr abnormal monitor driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <linux/power/hisi/soh/hisi_soh_interface.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>

#include "acr_detect.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG acr_detect
HWLOG_REGIST();

static int acr_det_rt_run;

static int acr_det_start_detect(struct acr_det_device_info *di)
{
	/* first: cal acr */
	soh_acr_low_precision_cal_start();

	/* second: sleep 1000ms, wait for acr cal finish */
	msleep(1000);

	memset(&di->acr_data, 0, sizeof(di->acr_data));

	/* third: get acr resistance */
	return soh_get_acr_resistance(&di->acr_data, ACR_L_PRECISION);
}

#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
static void acr_det_fmd_data_init(struct acr_det_device_info *di)
{
	di->acr_msg.version = ACR_RT_DEFAULT_VERSION;
	di->acr_msg.data_source = DATA_FROM_KERNEL;
	di->acr_msg.num_events = 1;

	di->acr_msg.events[0].error_code = 0;
	di->acr_msg.events[0].item_id = ACR_RT_ITEM_ID;
	di->acr_msg.events[0].cycle = 0;
	memcpy(di->acr_msg.events[0].station, ACR_RT_NA,
		sizeof(ACR_RT_NA));
	memcpy(di->acr_msg.events[0].bsn, ACR_RT_NA,
		sizeof(ACR_RT_NA));
	memcpy(di->acr_msg.events[0].time, ACR_RT_NA,
		sizeof(ACR_RT_NA));
	memcpy(di->acr_msg.events[0].device_name, ACR_RT_DEVICE_NAME,
		sizeof(ACR_RT_DEVICE_NAME));
	memcpy(di->acr_msg.events[0].test_name, ACR_RT_TEST_NAME,
		sizeof(ACR_RT_TEST_NAME));
	snprintf(di->acr_msg.events[0].min_threshold, MAX_VAL_LEN,
		"%d", di->rt_fmd_min);
	snprintf(di->acr_msg.events[0].max_threshold, MAX_VAL_LEN,
		"%d", di->rt_fmd_max);
}

static int acr_det_fmd_report(struct acr_det_device_info *di, int acr_rt)
{
	char *batt_brand = hisi_battery_brand();
	int batt_volt = hisi_battery_voltage();
	int batt_temp = hisi_battery_temperature();

	acr_det_fmd_data_init(di);

	if (acr_rt > di->rt_threshold)
		memcpy(di->acr_msg.events[0].result, ACR_RT_FMD_FAIL,
			sizeof(ACR_RT_FMD_FAIL));
	else
		memcpy(di->acr_msg.events[0].result, ACR_RT_FMD_PASS,
			sizeof(ACR_RT_FMD_PASS));

	snprintf(di->acr_msg.events[0].value, MAX_VAL_LEN, "%d", acr_rt);
	memcpy(di->acr_msg.events[0].firmware, batt_brand, MAX_FIRMWARE_LEN);
	snprintf(di->acr_msg.events[0].description, MAX_DESCRIPTION_LEN,
		"batt_volt:%dmV, batt_temp:%d", batt_volt, batt_temp);

	return power_dsm_bigdata_report(POWER_DSM_BATTERY,
		DA_BATTERY_ACR_ERROR_NO, &di->acr_msg);
}

#else

static inline int acr_det_fmd_report(struct acr_det_device_info *di, int acr_rt)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_DATA_ACQUISITION */

static void acr_det_dmd_report(int acr_rt)
{
	char buf[POWER_DSM_BUF_SIZE_0128] = { 0 };
	char *batt_brand = hisi_battery_brand();
	int batt_volt = hisi_battery_voltage();
	int batt_curr = -hisi_battery_current();
	int batt_soc = hisi_battery_capacity();
	int batt_temp = hisi_battery_temperature();

	snprintf(buf, POWER_DSM_BUF_SIZE_0128 - 1,
		"brand:%s, volt:%d, curr:%d, soc:%d, temp:%d, acr:%d\n",
		batt_brand, batt_volt, batt_curr, batt_soc, batt_temp, acr_rt);

	power_dsm_dmd_report(POWER_DSM_BATTERY,
		ERROR_BATT_ACR_OVER_THRESHOLD, buf);
}

static void acr_det_rt_work(struct work_struct *work)
{
	struct delayed_work *d_work = NULL;
	struct acr_det_device_info *di = NULL;
	int i;
	int ichg_coul;
	int acr_rt;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	d_work = to_delayed_work(work);
	di = container_of(d_work, struct acr_det_device_info, rt_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	hwlog_info("acr detect work begin\n");

	acr_det_rt_run = ACR_TRUE;

	if (charge_set_input_current_max() == 0) {
		for (i = 0; i < ACR_RT_RETRY_TIMES; i++) {
			ichg_coul = -hisi_battery_current();
			if (abs(ichg_coul) < ACR_RT_CURRENT_MIN) {
				hwlog_info("charging disabled, i:%d, ichg_coul:%d\n",
					i, ichg_coul);
				goto acr_detect;
			}

			msleep(100); /* sleep 100ms for next check */
		}
	}

	di->rt_status = ACR_DET_RT_STATUS_FAIL;
	acr_det_rt_run = ACR_FALSE;

	hwlog_err("acr detect fail1\n");
	return;

acr_detect:
	if (acr_det_start_detect(di) == 0) {
		acr_rt = di->acr_data.batt_acr;
		hwlog_info("acr:%d, threshold:%d\n",
			acr_rt, di->rt_threshold);

		if (acr_det_fmd_report(di, acr_rt) < 0)
			hwlog_err("acr detect fmd data acquisition fail\n");

		msleep(50); /* sleep 50ms, wait for dsm_battery client free */

		if (acr_rt > di->rt_threshold)
			acr_det_dmd_report(acr_rt);

		di->rt_status = ACR_DET_RT_STATUS_SUCC;
		acr_det_rt_run = ACR_FALSE;
		hwlog_info("acr detect succ\n");
		return;
	}

	di->rt_status = ACR_DET_RT_STATUS_FAIL;
	acr_det_rt_run = ACR_FALSE;

	hwlog_err("acr detect fail2\n");
}

#ifdef CONFIG_SYSFS
#define ACR_DET_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, acr_det_sysfs_show, store), \
	.name = ACR_DET_SYSFS_##n, \
}

#define ACR_DET_SYSFS_FIELD_RW(_name, n) \
	ACR_DET_SYSFS_FIELD(_name, n, 0644, acr_det_sysfs_store)

#define ACR_DET_SYSFS_FIELD_RO(_name, n) \
	ACR_DET_SYSFS_FIELD(_name, n, 0444, NULL)

static ssize_t acr_det_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t acr_det_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct acr_det_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static struct acr_det_sysfs_field_info acr_det_sysfs_field_tbl[] = {
	ACR_DET_SYSFS_FIELD_RO(acr_rt_support, RT_SUPPORT),
	ACR_DET_SYSFS_FIELD_RW(acr_rt_detect, RT_DETECT),
};

#define ACR_DET_SYSFS_ATTRS_SIZE  (ARRAY_SIZE(acr_det_sysfs_field_tbl) + 1)

static struct attribute *acr_det_sysfs_attrs[ACR_DET_SYSFS_ATTRS_SIZE];

static const struct attribute_group acr_det_sysfs_attr_group = {
	.attrs = acr_det_sysfs_attrs,
};

static void acr_det_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(acr_det_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		acr_det_sysfs_attrs[i] = &acr_det_sysfs_field_tbl[i].attr.attr;

	acr_det_sysfs_attrs[limit] = NULL;
}

static struct acr_det_sysfs_field_info *acr_det_sysfs_field_lookup(
	const char *name)
{
	int i;
	int limit = ARRAY_SIZE(acr_det_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, acr_det_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &acr_det_sysfs_field_tbl[i];
}

static ssize_t acr_det_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct acr_det_sysfs_field_info *info = NULL;
	struct acr_det_device_info *di = dev_get_drvdata(dev);
	int len;

	info = acr_det_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case ACR_DET_SYSFS_RT_SUPPORT:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->rt_support);
		break;

	case ACR_DET_SYSFS_RT_DETECT:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->rt_status);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		len = 0;
		break;
	}

	return len;
}

static ssize_t acr_det_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct acr_det_sysfs_field_info *info = NULL;
	struct acr_det_device_info *di = dev_get_drvdata(dev);
	int val = 0;
	int ret;

	info = acr_det_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case ACR_DET_SYSFS_RT_DETECT:
		/* 10: decimal base, 1: only set value to 1 start acr detect */
		ret = kstrtoint(buf, 10, &val);
		if ((ret < 0) || (val != 1)) {
			hwlog_err("unable to parse input:%s\n", buf);
			return -EINVAL;
		}

		if (!di->rt_support) {
			hwlog_info("acr detect not support\n");
			return -EINVAL;
		}

		if (!strstr(saved_command_line, "androidboot.swtype=factory")) {
			hwlog_info("acr detect not support on normal mode\n");
			return -EINVAL;
		}

		if (acr_det_rt_run == ACR_TRUE) {
			hwlog_info("acr detect is running\n");
			return -EINVAL;
		}

		di->rt_status = ACR_DET_RT_STATUS_INIT;
		schedule_delayed_work(&di->rt_work, msecs_to_jiffies(0));
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return count;
}

static int acr_det_sysfs_create_group(struct acr_det_device_info *di)
{
	acr_det_sysfs_init_attrs();

	return sysfs_create_group(&di->dev->kobj, &acr_det_sysfs_attr_group);
}

static inline void acr_det_sysfs_remove_group(struct acr_det_device_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &acr_det_sysfs_attr_group);
}

#else

static inline int acr_det_sysfs_create_group(struct acr_det_device_info *di)
{
	return 0;
}

static inline void acr_det_sysfs_remove_group(struct acr_det_device_info *di)
{
}
#endif /* CONFIG_SYSFS */

static void acr_det_parse_dts(struct device_node *np,
	struct acr_det_device_info *di)
{
	int ret;

	ret = of_property_read_u32(np, "acr_rt_support",
		&(di->rt_support));
	if (ret) {
		hwlog_err("acr_rt_support dts read failed\n");
		di->rt_support = ACR_RT_NOT_SUPPORT;
	}
	hwlog_info("acr_rt_support=%d\n", di->rt_support);

	ret = of_property_read_u32(np, "acr_rt_threshold",
		&(di->rt_threshold));
	if (ret) {
		hwlog_err("acr_rt_threshold dts read failed\n");
		di->rt_threshold = ACR_RT_THRESHOLD_DEFAULT;
	}
	hwlog_info("acr_rt_threshold=%d\n", di->rt_threshold);

	di->rt_threshold *= 1000; /* 1000: mA to uA unit */

	ret = of_property_read_u32(np, "acr_rt_fmd_min",
		&(di->rt_fmd_min));
	if (ret) {
		hwlog_err("acr_rt_fmd_min dts read failed\n");
		di->rt_fmd_min = ACR_RT_FMD_MIN_DEFAULT;
	}
	hwlog_info("acr_rt_fmd_min=%d\n", di->rt_fmd_min);

	ret = of_property_read_u32(np, "acr_rt_fmd_max",
		&(di->rt_fmd_max));
	if (ret) {
		hwlog_err("acr_rt_fmd_max dts read failed\n");
		di->rt_fmd_max = ACR_RT_FMD_MAX_DEFAULT;
	}
	hwlog_info("acr_rt_fmd_max=%d\n", di->rt_fmd_max);
}

static int acr_det_probe(struct platform_device *pdev)
{
	int ret;
	struct acr_det_device_info *di = NULL;
	struct device_node *np = NULL;
	struct class *power_class = NULL;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;

	acr_det_parse_dts(np, di);

	INIT_DELAYED_WORK(&di->rt_work, acr_det_rt_work);

	ret = acr_det_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto fail_free_mem;
	}

	power_class = hw_power_get_class();
	if (power_class) {
		if (!charge_dev)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charger");
		if (IS_ERR(charge_dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(charge_dev);
			goto fail_create_device;
		}

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"acr");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto fail_create_link;
		}
	}

	platform_set_drvdata(pdev, di);

	hwlog_info("probe end\n");
	return 0;

fail_create_link:
fail_create_device:
	acr_det_sysfs_remove_group(di);
fail_free_mem:
	kfree(di);
	di = NULL;

	return ret;
}

static int acr_det_remove(struct platform_device *pdev)
{
	struct acr_det_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	acr_det_sysfs_remove_group(di);
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;

	hwlog_info("remove end\n");
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
