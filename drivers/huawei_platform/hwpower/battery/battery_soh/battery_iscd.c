/*
 * battery_iscd.c
 *
 * driver adapter for iscd.
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

#include "battery_iscd.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/power/hisi/bci_battery.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/coul/coul_event.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG battery_iscd
HWLOG_REGIST();

static struct iscd_ocv_data g_ocv_update_data = { 0 };
static int g_current_event;
static unsigned int g_iscd_status;
static unsigned int g_iscd_trigger_type;
static struct bsoh_device *g_bsoh_dev;
static char *g_dmd_content;
struct notifier_block g_ocv_update_notify;
static struct delayed_work g_ocv_uevent_work;
static struct ocv_update_data g_ocv_data;

static int dump_ocv_to_buf(struct ocv_update_data *ocv, char *buf, int buf_size)
{
	int ret;

	ret = snprintf(buf, buf_size,
		"tm:%ld,volt_uv:%d,soc_uah:%d,cc:%ld,temp:%d,pc:%d,lv:%d,cyc:%d",
		ocv->sample_time_sec, ocv->ocv_volt_uv, ocv->ocv_soc_uah, ocv->cc_value,
		ocv->tbatt, ocv->pc, ocv->ocv_level, ocv->batt_chargecycles);
	if (ret <= 0)
		hwlog_err("dump ocv info failed\n");

	hwlog_info("ocv_info: %s\n", buf);
	return ret;
}

static void iscd_soh_uevent_work(struct work_struct *work)
{
	int ret;
	char *event_buf = NULL;

	event_buf = kzalloc(BSOH_EVENT_NOTIFY_SIZE, GFP_KERNEL);
	if (!event_buf)
		return;

	hwlog_info("soh_evt_to_send\n");
	ret = dump_ocv_to_buf(&g_ocv_data, event_buf, BSOH_EVENT_NOTIFY_SIZE);
	if (ret > 0)
		bsoh_uevent_rcv(BSOH_EVT_OCV_UPDATE, event_buf);

	kfree(event_buf);
}

static void iscd_event_notify(struct bsoh_device *di, unsigned int event)
{
	if (!di || !di->dev)
		return;

	switch (event) {
	case VCHRG_START_AC_CHARGING_EVENT:
	case VCHRG_START_USB_CHARGING_EVENT:
		g_current_event = ISCD_START_CHARGE_EVENT;
		power_event_notify_sysfs(&di->dev->kobj, "iscd", "iscd_process_event");
		break;
	case VCHRG_STOP_CHARGING_EVENT:
		g_current_event = ISCD_STOP_CHARGE_EVENT;
		power_event_notify_sysfs(&di->dev->kobj, "iscd", "iscd_process_event");
		break;
	default:
		break;
	};
}

static void iscd_dmd_content_prepare(char *buff, unsigned int size)
{
	char *bat_brand = coul_drv_battery_brand();

	if (!bat_brand || !g_dmd_content)
		return;
	snprintf(buff, size - 1,
		"batt_brand:%s, dmd_content:%s\n", bat_brand, g_dmd_content);
}

int iscd_ocv_update_notifier_call(struct notifier_block *ocv_update_nb,
	unsigned long event, void *data)
{
	struct ocv_update_data *ocv_data = (struct ocv_update_data *)data;

	if (event != BATT_OCV_UPDATE)
		return NOTIFY_OK;

	if (!data)
		return NOTIFY_BAD;

	g_ocv_update_data.sample_time_sec = ocv_data->sample_time_sec;
	g_ocv_update_data.ocv_volt_uv = ocv_data->ocv_volt_uv;
	g_ocv_update_data.ocv_soc_uah = ocv_data->ocv_soc_uah;
	g_ocv_update_data.cc_value = ocv_data->cc_value;
	g_ocv_update_data.tbatt = ocv_data->tbatt;
	g_ocv_update_data.pc = ocv_data->pc;
	g_ocv_update_data.ocv_level = ocv_data->ocv_level;

	g_current_event = ISCD_OCV_UPDATE_EVENT;
	if (!g_iscd_status)
		power_event_notify_sysfs(&g_bsoh_dev->dev->kobj,
			"iscd", "iscd_process_event");

	g_ocv_data = *ocv_data;
	schedule_delayed_work(&g_ocv_uevent_work, 0);
	return NOTIFY_OK;
}

int iscd_send_uevent_notify(void)
{
	char *envp_ext[] = { "BATTERY_EVENT=FATAL_ISC", NULL };
	int ret;

	if (!g_bsoh_dev) {
		hwlog_err("driver for isc probe uncorrect\n");
		return -1;
	}

	ret = kobject_uevent_env(&g_bsoh_dev->dev->kobj, KOBJ_CHANGE, envp_ext);
	if (ret) {
		hwlog_err("iscd uevent notify failed\n");
		return -1;
	}
	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t iscd_data_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	memcpy(buff, &g_ocv_update_data, sizeof(g_ocv_update_data));
	return sizeof(g_ocv_update_data);
}

static ssize_t iscd_imonitor_data_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	char *bat_brand = NULL;
	struct iscd_imonitor_data data;

	memset(&data, 0, sizeof(data));
	data.fcc = coul_drv_battery_fcc_design();
	data.bat_cyc = coul_drv_battery_cycle_count();
	data.q_max = coul_drv_battery_get_qmax();
	bat_brand = coul_drv_battery_brand();
	if (bat_brand)
		memcpy(data.bat_man, bat_brand,
			strnlen(bat_brand, ISCD_BASIC_INFO_MAX_LEN - 1));

	memcpy(buff, &data, sizeof(data));
	return sizeof(data);
}

static ssize_t iscd_process_event_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	int value = -1;

	switch (g_current_event) {
	case ISCD_OCV_UPDATE_EVENT:
	case ISCD_START_CHARGE_EVENT:
	case ISCD_STOP_CHARGE_EVENT:
		memcpy(buff, &g_current_event, sizeof(g_current_event));
		return sizeof(g_current_event);
	default:
		memcpy(buff, &value, sizeof(value));
		return sizeof(value);
	}
}

static ssize_t iscd_battery_current_avg_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	int current_avg_ma;

	current_avg_ma = coul_drv_battery_current_avg();
	memcpy(buff, &current_avg_ma, sizeof(current_avg_ma));
	return sizeof(current_avg_ma);
}

static ssize_t iscd_uevent_notify_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int prompt;
	int ret;

	if (kstrtoint(buf, 0, &prompt))
		return -1;

	if (prompt) {
		ret = iscd_send_uevent_notify();
		if (ret)
			return -1;
	}
	return count;
}

static ssize_t iscd_dmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct iscd_dmd_data dmd;

	if (count != sizeof(dmd))
		return -1;

	memcpy(&dmd, buf, sizeof(dmd));
	memset(g_dmd_content, 0, ISCD_MAX_FATAL_ISC_DMD_NUM);
	memcpy(g_dmd_content, dmd.buff, ISCD_MAX_FATAL_ISC_DMD_NUM - 1);
	bsoh_dmd_append("iscd", dmd.err_no);
	return count;
}

static ssize_t iscd_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%u\n", g_iscd_status);
}

static ssize_t iscd_status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int status;

	if (kstrtouint(buf, 0, &status))
		return -1;

	g_iscd_status = status;
	return count;
}

static ssize_t iscd_limit_support_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%u\n", g_iscd_trigger_type);
}

static ssize_t iscd_limit_support_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int trigger_type;

	if (kstrtouint(buf, 0, &trigger_type))
		return -1;

	g_iscd_trigger_type = trigger_type;
	return count;
}

static DEVICE_ATTR_RO(iscd_data);
static DEVICE_ATTR_RO(iscd_process_event);
static DEVICE_ATTR_RO(iscd_imonitor_data);
static DEVICE_ATTR_RO(iscd_battery_current_avg);
static DEVICE_ATTR_WO(iscd_uevent_notify);
static DEVICE_ATTR_WO(iscd_dmd);
static DEVICE_ATTR_RW(iscd_status);
static DEVICE_ATTR_RW(iscd_limit_support);
static struct attribute *g_iscd_attrs[] = {
	&dev_attr_iscd_data.attr,
	&dev_attr_iscd_process_event.attr,
	&dev_attr_iscd_imonitor_data.attr,
	&dev_attr_iscd_battery_current_avg.attr,
	&dev_attr_iscd_uevent_notify.attr,
	&dev_attr_iscd_dmd.attr,
	&dev_attr_iscd_status.attr,
	&dev_attr_iscd_limit_support.attr,
	NULL,
};

static struct attribute_group g_iscd_group = {
	.name = "iscd",
	.attrs = g_iscd_attrs,
};

static int iscd_sysfs_create_group(struct bsoh_device *di)
{
	return sysfs_create_group(&di->dev->kobj, &g_iscd_group);
}

static void iscd_sysfs_remove_group(struct bsoh_device *di)
{
	sysfs_remove_group(&di->dev->kobj, &g_iscd_group);
}
#else
static inline int iscd_sysfs_create_group(struct bsoh_device *di)
{
	return 0;
}

static inline void iscd_sysfs_remove_group(struct bsoh_device *di)
{
}
#endif /* CONFIG_SYSFS */

static int iscd_sys_init(struct bsoh_device *di)
{
	int ret;

	hwlog_info("iscd init begin\n");

	if (!di || !di->dev)
		return -EINVAL;

	ret = iscd_sysfs_create_group(di);
	if (ret)
		return -EINVAL;

	/* register notifier for coul core */
	g_ocv_update_notify.notifier_call = iscd_ocv_update_notifier_call;
	ret = coul_register_blocking_notifier(&g_ocv_update_notify);
	if (ret < 0)
		return -EINVAL;

	INIT_DELAYED_WORK(&g_ocv_uevent_work, iscd_soh_uevent_work);

	kfree(g_dmd_content);
	g_dmd_content = kzalloc(ISCD_MAX_FATAL_ISC_DMD_NUM, GFP_KERNEL);
	if (!g_dmd_content)
		return -EINVAL;

	g_bsoh_dev = di;
	g_current_event = 0;
	hwlog_info("iscd init ok\n");
	return 0;
}

static void iscd_sys_exit(struct bsoh_device *di)
{
	iscd_sysfs_remove_group(di);
	kfree(g_dmd_content);
	g_bsoh_dev = NULL;
	g_dmd_content = NULL;
}

static const struct bsoh_sub_sys g_iscd_sys = {
	.sys_init = iscd_sys_init,
	.sys_exit = iscd_sys_exit,
	.event_notify = iscd_event_notify,
	.dmd_prepare = iscd_dmd_content_prepare,
	.type_name = "iscd",
	.notify_node = "iscd_process_event",
};

static int __init iscd_init(void)
{
	bsoh_register_sub_sys(BSOH_SUB_SYS_ISCD, &g_iscd_sys);
	return 0;
}

static void __exit iscd_exit(void)
{
}

subsys_initcall(iscd_init);
module_exit(iscd_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery iscd driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
