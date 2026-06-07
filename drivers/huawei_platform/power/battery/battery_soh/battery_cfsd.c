/*
 * battery_cfsd.c
 *
 * Driver adapter for cfsd.
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include <securec.h>
#include <linux/platform_device.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <linux/power/hisi/coul/hisi_coul_event.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/battery_soh.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
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

	if (event != HISI_OCV_UPDATE)
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

	if (g_sysfs_bsoh)
		sysfs_notify(g_sysfs_bsoh, "cfsd", "ocv_info");

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
	if (g_sysfs_bsoh)
		sysfs_notify(g_sysfs_bsoh, "cfsd", "ocv_info");

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

int cfsd_init(struct bsoh_device *di)
{
	int ret;

	hwlog_info("cfsd init begin\n");
	if (!di || !di->dev)
		return -1;

	ret = cfsd_sysfs_create_group(di);
	if (ret)
		return -1;
	/* register notifier for coul core */
	g_ocv_update_notify.notifier_call = cfsd_ocv_update_notifier_call;
	ret = hisi_coul_register_blocking_notifier(&g_ocv_update_notify);
	if (ret < 0) {
		hwlog_err("ocv_update_register_notifier failed\n");
		return -1;
	}
	g_bsoh_dev = di;
	hwlog_info("cfsd init ok\n");
	return 0;
}

void cfsd_exit(struct bsoh_device *di)
{
	cfsd_sysfs_remove_group(di);
	g_bsoh_dev = NULL;
	g_sysfs_bsoh = NULL;
}

