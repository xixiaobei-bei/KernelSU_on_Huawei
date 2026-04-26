/*
 * battery_cfsd.c
 *
 * driver adapter for cfsd.
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "battery_cfsd.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/coul/coul_event.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG battery_cfsd
HWLOG_REGIST();

static struct cfsd_ocv_info g_ocv_info = { 0 };
static enum CFSD_EVENT_TYPE g_cfsd_event = CFSD_EVENT_NONE;
static struct bsoh_device *g_bsoh_dev;
static struct notifier_block g_ocv_update_notify;
static struct kobject *g_sysfs_bsoh;

int cfsd_ocv_update_notifier_call(struct notifier_block *ocv_update_nb,
	unsigned long event, void *data)
{
	struct ocv_update_data *ocv_info = (struct ocv_update_data *)data;

	if (event != BATT_OCV_UPDATE)
		return NOTIFY_OK;

	if (!data)
		return NOTIFY_BAD;

	hwlog_info("cfsd received ocv changed form coul\n");
	g_ocv_info.time = ocv_info->sample_time_sec;
	g_ocv_info.ocv_uv = ocv_info->ocv_volt_uv;
	g_ocv_info.ocv_temp = ocv_info->tbatt;
	g_ocv_info.cc_value = ocv_info->cc_value;
	g_ocv_info.ocv_soc = ocv_info->pc;
	g_ocv_info.batt_chargecycles = ocv_info->batt_chargecycles;
	g_ocv_info.ocv_level =  ocv_info->ocv_level;
	g_cfsd_event = CFSD_OCV_UPDATE_EVENT;

	power_event_notify_sysfs(g_sysfs_bsoh, "cfsd", "ocv_info");
	return NOTIFY_OK;
}

#ifdef CONFIG_SYSFS
static ssize_t ocv_info_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	memcpy(buff, &g_ocv_info, sizeof(g_ocv_info));
	return sizeof(g_ocv_info);
}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t cfsd_event_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	memcpy(buff, &g_cfsd_event, sizeof(g_cfsd_event));
	return sizeof(g_cfsd_event);
}

static ssize_t cfsd_event_store(struct device *dev,
	struct device_attribute *attr, const char *buff, size_t count)
{
	power_event_notify_sysfs(g_sysfs_bsoh, "cfsd", "ocv_info");

	return sizeof(g_ocv_info);
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static DEVICE_ATTR_RO(ocv_info);
#ifdef CONFIG_HUAWEI_POWER_DEBUG
static DEVICE_ATTR(cfsd_event, 0640, cfsd_event_show, cfsd_event_store);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static struct attribute *g_cfsd_attrs[] = {
	&dev_attr_ocv_info.attr,
#ifdef CONFIG_HUAWEI_POWER_DEBUG
	&dev_attr_cfsd_event.attr,
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
	NULL,
};

static struct attribute_group g_cfsd_group = {
	.name = "cfsd",
	.attrs = g_cfsd_attrs,
};

static int cfsd_sysfs_create_group(struct bsoh_device *di)
{
	g_sysfs_bsoh = &di->dev->kobj;
	return sysfs_create_group(g_sysfs_bsoh, &g_cfsd_group);
}

static void cfsd_sysfs_remove_group(struct bsoh_device *di)
{
	sysfs_remove_group(&di->dev->kobj, &g_cfsd_group);
}
#else
static inline int cfsd_sysfs_create_group(struct bsoh_device *di)
{
	return 0;
}

static inline void cfsd_sysfs_remove_group(struct bsoh_device *di)
{
}
#endif /* CONFIG_SYSFS */

static int cfsd_sys_init(struct bsoh_device *di)
{
	int ret;

	hwlog_info("cfsd init begin\n");

	if (!di || !di->dev)
		return -EINVAL;

	ret = cfsd_sysfs_create_group(di);
	if (ret)
		return -EINVAL;

	/* register notifier for coul core */
	g_ocv_update_notify.notifier_call = cfsd_ocv_update_notifier_call;
	ret = coul_register_blocking_notifier(&g_ocv_update_notify);
	if (ret < 0)
		return -EINVAL;

	g_bsoh_dev = di;
	hwlog_info("cfsd init ok\n");
	return 0;
}

static void cfsd_sys_exit(struct bsoh_device *di)
{
	cfsd_sysfs_remove_group(di);
	g_bsoh_dev = NULL;
	g_sysfs_bsoh = NULL;
}

static const struct bsoh_sub_sys g_cfsd_sys = {
	.sys_init = cfsd_sys_init,
	.sys_exit = cfsd_sys_exit,
	.event_notify = NULL,
	.dmd_prepare = NULL,
	.type_name = "cfsd",
	.notify_node = NULL,
};

static int __init cfsd_init(void)
{
	bsoh_register_sub_sys(BSOH_SUB_SYS_CFSD, &g_cfsd_sys);
	return 0;
}

static void __exit cfsd_exit(void)
{
}

subsys_initcall(cfsd_init);
module_exit(cfsd_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery cfsd driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
