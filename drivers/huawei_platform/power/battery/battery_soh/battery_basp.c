/*
 * battery_basp.c
 *
 * Driver adapter for basp.
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

#include "battery_basp.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <huawei_platform/power/power_dsm.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG battery_basp
HWLOG_REGIST();

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static struct basp_data g_dbg_basp_data = {0};
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
static unsigned int g_basp_level;

void basp_event_notify(struct bsoh_device *di, unsigned int event)
{
	if (!di || !di->dev)
		return;

	switch (event) {
	case VCHRG_CHARGE_DONE_EVENT:
	case VCHRG_STOP_CHARGING_EVENT:
		sysfs_notify(&di->dev->kobj, "basp", "basp_data");
		hwlog_info("recv charger event %d\n", event);
		break;
	default:
		break;
	};
}

void basp_dmd_content_prepare(char *buff, unsigned int size)
{
	int i;
	int len = 0;
	unsigned int charge_cycles = hisi_battery_cycle_count();
	int record_fcc_num;
	unsigned int record_fcc[MAX_RECORDS_CNT] = {0};
	char *bat_brand = hisi_battery_brand();

	record_fcc_num = hisi_battery_get_recrod_fcc(MAX_RECORDS_CNT,
		record_fcc);
	for (i = 0; i < record_fcc_num; i++)
		len += snprintf(buff + len, size - len - 1,
			"real_fcc_record[%d]:%d, ", i, record_fcc[i]);

	if (!bat_brand)
		return;
	snprintf(buff + len, size - len - 1,
		"batt_brand:%s, batt_chargecycles:%d, basp_level:%d\n",
		bat_brand, charge_cycles, g_basp_level);
}

#ifdef CONFIG_SYSFS
static ssize_t basp_data_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct basp_data data = {0};

	memset(&data, 0, sizeof(data));
	data.ratio_fcc = hisi_battery_fcc_design();
	data.charge_cycles = hisi_battery_cycle_count() *
		HISI_CHARGER_CYCLE_SCALE;
	data.record_fcc_num = hisi_battery_get_recrod_fcc(
		MAX_RECORDS_CNT, data.record_fcc);

	memmove(buff, &data, sizeof(data));
	return sizeof(data);
}

static ssize_t basp_imonitor_data_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	char *bat_brand = NULL;
	struct basp_imonitor_data data;

	memset(&data, 0, sizeof(data));
	data.bat_cap = hisi_battery_fcc_design();
	data.bat_cyc = hisi_battery_cycle_count();
	data.q_max = hisi_battery_get_qmax();

	bat_brand = hisi_battery_brand();
	if (bat_brand)
		memcpy(data.bat_man, bat_brand,
			strnlen(bat_brand, BASP_BASIC_INFO_MAX_LEN - 1));

	memcpy(buff, &data, sizeof(data));
	return sizeof(data);
}

static ssize_t basp_policy_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct basp_policy policy;

	if (count != sizeof(policy))
		return -1;
	memmove(&policy, buf, sizeof(policy));

	if (g_basp_level == policy.level)
		return count;

	g_basp_level = policy.level;
	hisi_battery_update_basp_policy(policy.level,
		policy.nondc_volt_dec);

	bsoh_dmd_append("basp", policy.dmd_no);
	return count;
}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t basp_charge_cycles_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int cycles;

	if (kstrtouint(buf, 0, &cycles))
		return -1;

	g_dbg_basp_data.charge_cycles = cycles;
	return count;
}

static ssize_t basp_record_fcc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int i;
	size_t len;
	char *sub = NULL;
	char *cur = NULL;
	char str[MAX_TEMP_BUF_LEN] = {0};

	len = min_t(size_t, sizeof(str) - 1, count);
	memcpy(str, buf, len);
	cur = &str[0];

	g_dbg_basp_data.record_fcc_num = 0;
	for (i = 0; i < MAX_RECORDS_CNT; i++) {
		sub = strsep(&cur, " ");
		if (!sub || kstrtouint(sub, 0, &g_dbg_basp_data.record_fcc[i]))
			return -1;
		g_dbg_basp_data.record_fcc_num++;

		if (!cur || (*cur == '\0'))
			break;
	}

	return count;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static DEVICE_ATTR_RO(basp_data);
static DEVICE_ATTR_RO(basp_imonitor_data);
static DEVICE_ATTR_WO(basp_policy);
#ifdef CONFIG_HUAWEI_POWER_DEBUG
static DEVICE_ATTR_WO(basp_charge_cycles);
static DEVICE_ATTR_WO(basp_record_fcc);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
static struct attribute *g_basp_attrs[] = {
	&dev_attr_basp_data.attr,
	&dev_attr_basp_imonitor_data.attr,
	&dev_attr_basp_policy.attr,
#ifdef CONFIG_HUAWEI_POWER_DEBUG
	&dev_attr_basp_charge_cycles.attr,
	&dev_attr_basp_record_fcc.attr,
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
	NULL,
};

static struct attribute_group g_basp_group = {
	.name = "basp",
	.attrs = g_basp_attrs,
};

int basp_sysfs_create_group(struct bsoh_device *di)
{
	return sysfs_create_group(&di->dev->kobj, &g_basp_group);
}

void basp_sysfs_remove_group(struct bsoh_device *di)
{
	sysfs_remove_group(&di->dev->kobj, &g_basp_group);
}
#else
int basp_sysfs_create_group(struct bsoh_device *di)
{
	return 0;
}

void basp_sysfs_remove_group(struct bsoh_device *di)
{
}
#endif /* CONFIG_SYSFS */
