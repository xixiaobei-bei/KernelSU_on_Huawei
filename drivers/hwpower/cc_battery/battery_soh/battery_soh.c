/*
 * battery_soh.c
 *
 * driver adapter for bsoh.
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

#include <chipset_common/hwpower/battery/battery_soh.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG battery_soh
HWLOG_REGIST();

#define RSV_END_SPACE 2

static struct bsoh_device *g_bsoh_device;

/*
 * type_name is the identity of soh's subsystem.
 * 1 Report DMD through bsoh_dmd_report needs type_name.
 * 2 To check whether subsystem is enable, native processes can read
 * bsoh_subsys and check if return value contains subsystem's type_name.
 * 3 To enable subsystem, type_name property of dts should be set to 1.
 * for example: basp = <1>
 */
static const struct bsoh_sub_sys *g_bsoh_sub_sys[BSOH_SUB_SYS_END];

static const char * const g_bsoh_event_table[BSOH_EVT_END] = {
	[BSOH_EVT_OCV_UPDATE] = "EVT_OCV_UPDATE",
	[BSOH_EVT_SOH_DCR_UPDATE] = "EVT_SOH_DCR_UPDATE",
	[BSOH_EVT_SOH_ACR_UPDATE] = "EVT_SOH_ACR_UPDATE",
	[BSOH_EVT_EIS_FREQ_UPDATE] = "EVT_EIS_FREQ_UPDATE",
	[BSOH_EVT_BATT_INFO_UPDATE] = "EVT_BATT_INFO_UPDATE",
};

void bsoh_register_sub_sys(enum bsoh_sub_sys_type type,
	const struct bsoh_sub_sys *sub_sys)
{
	if (!sub_sys)
		return;

	if ((type < BSOH_SUB_SYS_BEGIN) || (type >= BSOH_SUB_SYS_END))
		return;

	g_bsoh_sub_sys[type] = sub_sys;
}

static const struct bsoh_sub_sys *bsoh_get_sub_sys(const char *type_name)
{
	int i;
	unsigned int enable;

	for (i = 0; i < BSOH_SUB_SYS_END; i++) {
		if (!g_bsoh_sub_sys[i])
			continue;

		enable = g_bsoh_device->sub_sys_enable & BIT(i);
		if (!enable || !g_bsoh_sub_sys[i]->type_name ||
			!sysfs_streq(type_name, g_bsoh_sub_sys[i]->type_name))
			continue;

		return g_bsoh_sub_sys[i];
	}

	return NULL;
}

void bsoh_uevent_rcv(unsigned int event, const char *data)
{
	int ret;
	int len;
	char *bsoh_event = NULL;
	struct power_event_notify_data n_data;

	if (event >= BSOH_EVT_END) {
		hwlog_err("invalid bsoh event\n");
		return;
	}

	bsoh_event = kzalloc(BSOH_EVENT_NOTIFY_SIZE, GFP_KERNEL);
	if (!bsoh_event)
		return;

	ret = snprintf(bsoh_event, BSOH_EVENT_NOTIFY_SIZE, "BSOH_EVT=%s",
		g_bsoh_event_table[event]);
	if (ret <= 0) {
		kfree(bsoh_event);
		return;
	}
	len = ret;

	if (data &&
		(len < (int)BSOH_EVENT_NOTIFY_SIZE) &&
		((int)(strlen(data) + RSV_END_SPACE) < ((int)BSOH_EVENT_NOTIFY_SIZE - len))) {
		ret = snprintf(bsoh_event + len, BSOH_EVENT_NOTIFY_SIZE - len, "@%s", data);
		if (ret <= 0) {
			kfree(bsoh_event);
			return;
		}
		len += ret;
	}

	bsoh_event[len] = '\n';
	bsoh_event[len + 1] = '\0';
	n_data.event_len = len + RSV_END_SPACE;
	n_data.event = bsoh_event;

	power_event_report_uevent(&n_data);
	kfree(bsoh_event);
}

void bsoh_event_rcv(unsigned int event)
{
	int i;
	unsigned int enable;

	if (!g_bsoh_device || !g_bsoh_device->dev)
		return;

	for (i = 0; i < BSOH_SUB_SYS_END; i++) {
		if (!g_bsoh_sub_sys[i])
			continue;

		enable = g_bsoh_device->sub_sys_enable & BIT(i);
		if (enable && g_bsoh_sub_sys[i]->event_notify)
			g_bsoh_sub_sys[i]->event_notify(g_bsoh_device, event);
	}
}

static int bsoh_dmd_report(struct bsoh_dmd_entry *entry)
{
	char *buff = NULL;

	buff = kzalloc(BSOH_MAX_DMD_BUF_SIZE, GFP_KERNEL);
	if (!buff)
		return -1;

	entry->prepare(buff, BSOH_MAX_DMD_BUF_SIZE);
	buff[BSOH_MAX_DMD_BUF_SIZE - 1] = '\0';
	if (power_dsm_report_dmd(POWER_DSM_BATTERY, entry->dmd_no, buff)) {
		kfree(buff);
		return -1;
	}

	kfree(buff);
	return 0;
}

static void bsoh_dmd_work(struct work_struct *work)
{
	struct bsoh_dmd_entry *pos = NULL;
	struct bsoh_dmd_entry *tmp = NULL;
	struct bsoh_device *di = g_bsoh_device;

	if (!di)
		return;

	mutex_lock(&di->dmd_list_lock);
	list_for_each_entry_safe(pos, tmp, &di->dmd_list_head, node) {
		pos->retry_times++;
		if (bsoh_dmd_report(pos) &&
			(pos->retry_times < BSOH_MAX_DMD_REPORT_TIMES)) {
			mutex_unlock(&di->dmd_list_lock);
			/* 3s */
			schedule_delayed_work(&di->bsoh_dmd_wrok, 3 * HZ);
			return;
		}

		list_del_init(&pos->node);
		kfree(pos);
	}
	mutex_unlock(&di->dmd_list_lock);
}

void bsoh_dmd_append(const char *type_name, unsigned int dmd_no)
{
	struct bsoh_dmd_entry *temp = NULL;
	const struct bsoh_sub_sys *sys = NULL;
	struct bsoh_device *di = g_bsoh_device;

	if (!di || !type_name || (dmd_no == 0))
		return;

	sys = bsoh_get_sub_sys(type_name);
	if (!sys || !sys->dmd_prepare) {
		hwlog_err("invalid type name %s\n", type_name);
		return;
	}

	temp = kzalloc(sizeof(*temp), GFP_KERNEL);
	if (!temp)
		return;

	temp->dmd_no = dmd_no;
	temp->prepare = sys->dmd_prepare;
	temp->retry_times = 0;
	INIT_LIST_HEAD(&temp->node);

	mutex_lock(&di->dmd_list_lock);
	list_add_tail(&temp->node, &di->dmd_list_head);
	mutex_unlock(&di->dmd_list_lock);

	cancel_delayed_work(&di->bsoh_dmd_wrok);
	/* 3s */
	schedule_delayed_work(&di->bsoh_dmd_wrok, 3 * HZ);
}

#ifdef CONFIG_SYSFS
static ssize_t bsoh_battery_removed_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	int flag = power_platform_is_battery_removed();

	memmove(buff, &flag, sizeof(flag));
	return sizeof(flag);
}

static ssize_t bsoh_subsys_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	int i;
	int enable;
	int len = 0;

	if (!g_bsoh_device)
		return -1;

	for (i = 0; i < BSOH_SUB_SYS_END; i++) {
		if (!g_bsoh_sub_sys[i])
			continue;

		enable = g_bsoh_device->sub_sys_enable & BIT(i);
		if (!enable || !g_bsoh_sub_sys[i]->type_name)
			continue;

		if (len > 0)
			len += snprintf(buff + len, PAGE_SIZE - len - 1, ",%s",
				g_bsoh_sub_sys[i]->type_name);
		else
			len += snprintf(buff + len, PAGE_SIZE - len - 1, "%s",
				g_bsoh_sub_sys[i]->type_name);

		if ((len + 1) >= (int)PAGE_SIZE)
			break;
	}

	return len;
}

static ssize_t bsoh_dmd_report_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int dmd_no;
	char type_name[BSOH_MAX_RD_BUF_SIZE] = { 0 };

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (BSOH_MAX_RD_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 2: the fields of "type dmd_no" */
	if (sscanf(buf, "%s %u", type_name, &dmd_no) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	bsoh_dmd_append(type_name, dmd_no);
	hwlog_info("to report %s dmd and no is %d\n", type_name, dmd_no);
	return count;
}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t bsoh_sysfs_notify_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	const struct bsoh_sub_sys *sys = NULL;

	sys = bsoh_get_sub_sys(buf);
	if (!sys || !sys->notify_node) {
		hwlog_err("invalid type name %s\n", buf);
		return -1;
	}

	power_event_notify_sysfs(&dev->kobj, sys->type_name, sys->notify_node);
	return count;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static const DEVICE_ATTR_RO(bsoh_battery_removed);
static const DEVICE_ATTR_RO(bsoh_subsys);
static const DEVICE_ATTR_WO(bsoh_dmd_report);
#ifdef CONFIG_HUAWEI_POWER_DEBUG
static const DEVICE_ATTR_WO(bsoh_sysfs_notify);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
static const struct attribute *g_bsoh_attrs[] = {
	&dev_attr_bsoh_battery_removed.attr,
	&dev_attr_bsoh_subsys.attr,
	&dev_attr_bsoh_dmd_report.attr,
#ifdef CONFIG_HUAWEI_POWER_DEBUG
	&dev_attr_bsoh_sysfs_notify.attr,
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
	NULL,
};

static int bsoh_sysfs_create_files(struct bsoh_device *di)
{
	return sysfs_create_files(&di->dev->kobj, g_bsoh_attrs);
}

static inline void bsoh_sysfs_remove_files(struct bsoh_device *di)
{
	sysfs_remove_files(&di->dev->kobj, g_bsoh_attrs);
}
#else
static inline int bsoh_sysfs_create_files(struct bsoh_device *di)
{
	return 0;
}

static inline void bsoh_sysfs_remove_files(struct bsoh_device *di)
{
}
#endif /* CONFIG_SYSFS */

static void bsoh_sub_sys_init(struct bsoh_device *di)
{
	int i;
	unsigned int enable;

	for (i = 0; i < BSOH_SUB_SYS_END; i++) {
		if (!g_bsoh_sub_sys[i])
			continue;

		enable = di->sub_sys_enable & BIT(i);
		if (enable && g_bsoh_sub_sys[i]->sys_init &&
			g_bsoh_sub_sys[i]->sys_init(di))
			hwlog_err("create sub system %d failed\n", i);
	}
}

static void bsoh_sub_sys_exit(struct bsoh_device *di)
{
	int i;
	unsigned int enable;

	for (i = 0; i < BSOH_SUB_SYS_END; i++) {
		if (!g_bsoh_sub_sys[i])
			continue;

		enable = g_bsoh_device->sub_sys_enable & BIT(i);
		if (enable && g_bsoh_sub_sys[i]->sys_exit)
			g_bsoh_sub_sys[i]->sys_exit(di);
	}
}

static int bsoh_dts_parse(struct device_node *np, struct bsoh_device *di)
{
	int i;
	unsigned int subsys_enable;

	for (i = 0; i < BSOH_SUB_SYS_END; i++) {
		if (!g_bsoh_sub_sys[i] || !g_bsoh_sub_sys[i]->type_name)
			continue;

		subsys_enable = 0;
		(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			g_bsoh_sub_sys[i]->type_name, &subsys_enable, 0);
		di->sub_sys_enable |= (!!subsys_enable) << i;
	}

	return 0;
}

static int bsoh_probe(struct platform_device *pdev)
{
	struct bsoh_device *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	if (bsoh_dts_parse(pdev->dev.of_node, di))
		goto free_mem;

	di->dev = &pdev->dev;
	INIT_DELAYED_WORK(&di->bsoh_dmd_wrok, bsoh_dmd_work);
	INIT_LIST_HEAD(&di->dmd_list_head);
	mutex_init(&di->dmd_list_lock);
	if (bsoh_sysfs_create_files(di)) {
		hwlog_err("create device node error\n");
		goto destroy_mutex;
	}

	bsoh_sub_sys_init(di);
	g_bsoh_device = di;
	return 0;

destroy_mutex:
	mutex_destroy(&di->dmd_list_lock);
free_mem:
	devm_kfree(&pdev->dev, di);
	return -1;
}

static void bsoh_free_dmd_resource(struct bsoh_device *di)
{
	struct bsoh_dmd_entry *pos = NULL;
	struct bsoh_dmd_entry *tmp = NULL;

	mutex_lock(&di->dmd_list_lock);
	list_for_each_entry_safe(pos, tmp, &di->dmd_list_head, node) {
		list_del_init(&pos->node);
		kfree(pos);
	}
	mutex_unlock(&di->dmd_list_lock);
	mutex_destroy(&g_bsoh_device->dmd_list_lock);
}

static int bsoh_remove(struct platform_device *pdev)
{
	if (!g_bsoh_device)
		return 0;

	bsoh_sub_sys_exit(g_bsoh_device);
	bsoh_sysfs_remove_files(g_bsoh_device);
	bsoh_free_dmd_resource(g_bsoh_device);
	kfree(g_bsoh_device);
	g_bsoh_device = NULL;

	return 0;
}

static const struct of_device_id bsoh_match_table[] = {
	{
		.compatible = "huawei,battery-soh",
		.data = NULL,
	},
	{},
};

static struct platform_driver bsoh_driver = {
	.probe = bsoh_probe,
	.remove = bsoh_remove,
	.driver = {
		.name = "battery-soh",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bsoh_match_table),
	},
};

static int __init bsoh_init(void)
{
	return platform_driver_register(&bsoh_driver);
}

static void __exit bsoh_exit(void)
{
	platform_driver_unregister(&bsoh_driver);
}

fs_initcall(bsoh_init);
module_exit(bsoh_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei battery soh driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
