/*
 * power_ui.c
 *
 * ui display for power module
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

#include "power_ui.h"
#include <chipset_common/hwpower/common_module/power_ui_ne.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_ui
HWLOG_REGIST();

static struct power_ui_dev *g_power_ui_dev;

static const char * const g_power_ui_ne_info[] = {
	[POWER_UI_NE_CABLE_TYPE] = "UI_CABLE_TYPE=",
	[POWER_UI_NE_MAX_POWER] = "UI_MAX_POWER=",
	[POWER_UI_NE_WL_OFF_POS] = "UI_WL_OFF_POS=",
	[POWER_UI_NE_WL_FAN_STATUS] = "UI_WL_FAN_STATUS=",
	[POWER_UI_NE_WL_COVER_STATUS] = "UI_WL_COVER_STATUS=",
	[POWER_UI_NE_WL_ACC_STATUS] = "UI_ACC_DET_STATUS=",
	[POWER_UI_NE_WATER_STATUS] = "UI_WATER_STATUS=",
	[POWER_UI_NE_HEATING_STATUS] = "UI_HEATING_STATUS=",
	[POWER_UI_NE_ICON_TYPE] = "UI_ICON_TYPE=",
	[POWER_UI_NE_DEFAULT] = "DEFAULT=",
};

static const char * const g_power_ui_op_user_table[POWER_UI_OP_USER_END] = {
	[POWER_UI_OP_USER_SHELL] = "shell",
	[POWER_UI_OP_USER_SYSTEM_UI] = "system_ui",
};

static int power_ui_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_power_ui_op_user_table); i++) {
		if (!strcmp(str, g_power_ui_op_user_table[i]))
			return i;
	}

	hwlog_err("invalid user_str=%s\n", str);
	return -1;
}

static struct power_ui_dev *power_ui_get_dev(void)
{
	if (!g_power_ui_dev) {
		hwlog_err("g_power_ui_dev is null\n");
		return NULL;
	}

	return g_power_ui_dev;
}

void power_ui_event_notify(unsigned long event, const void *data)
{
	struct power_ui_dev *l_dev = power_ui_get_dev();
	char e_buf[POWER_EVENT_NOTIFY_SIZE] = { 0 };
	struct power_event_notify_data n_data;

	if (!l_dev)
		return;

	hwlog_info("receive event %lu\n", event);

	switch (event) {
	case POWER_UI_NE_CABLE_TYPE:
		l_dev->info.cable_type = *(int *)data;
		break;
	case POWER_UI_NE_MAX_POWER:
		l_dev->info.max_power = *(int *)data;
		break;
	case POWER_UI_NE_WL_OFF_POS:
		l_dev->info.wl_off_pos = *(int *)data;
		break;
	case POWER_UI_NE_WL_FAN_STATUS:
		l_dev->info.wl_fan_status = *(int *)data;
		break;
	case POWER_UI_NE_WL_COVER_STATUS:
		l_dev->info.wl_cover_status = *(int *)data;
		break;
	case POWER_UI_NE_WL_ACC_STATUS:
		l_dev->info.wl_acc_status = *(int *)data;
		break;
	case POWER_UI_NE_WATER_STATUS:
		l_dev->info.water_status = *(int *)data;
		break;
	case POWER_UI_NE_HEATING_STATUS:
		l_dev->info.heating_status = *(int *)data;
		break;
	case POWER_UI_NE_ICON_TYPE:
		l_dev->info.icon_type = *(int *)data;
		break;
	case POWER_UI_NE_DEFAULT:
		memset(&l_dev->info, 0, sizeof(l_dev->info));
		return;
	default:
		return;
	}

	n_data.event_len = snprintf(e_buf, POWER_EVENT_NOTIFY_SIZE - 1, "%s%d",
		g_power_ui_ne_info[event], *(int *)data);
	n_data.event = e_buf;
	power_event_report_uevent(&n_data);
}

#ifdef CONFIG_SYSFS
static ssize_t power_ui_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t power_ui_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info power_ui_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(power_ui, 0440, POWER_UI_SYSFS_CABLE_TYPE, cable_type),
	power_sysfs_attr_ro(power_ui, 0440, POWER_UI_SYSFS_MAX_POWER, max_power),
	power_sysfs_attr_ro(power_ui, 0440, POWER_UI_SYSFS_WL_OFF_POS, wl_off_pos),
	power_sysfs_attr_rw(power_ui, 0660, POWER_UI_SYSFS_WL_FAN_STATUS, wl_fan_status),
	power_sysfs_attr_rw(power_ui, 0660, POWER_UI_SYSFS_WL_COVER_STATUS, wl_cover_status),
	power_sysfs_attr_rw(power_ui, 0660, POWER_UI_SYSFS_WL_ACC_STATUS, wl_acc_status),
	power_sysfs_attr_rw(power_ui, 0660, POWER_UI_SYSFS_WATER_STATUS, water_status),
	power_sysfs_attr_rw(power_ui, 0660, POWER_UI_SYSFS_HEATING_STATUS, heating_status),
	power_sysfs_attr_ro(power_ui, 0440, POWER_UI_SYSFS_ICON_TYPE, icon_type),
};

#define POWER_UI_SYSFS_ATTRS_SIZE  ARRAY_SIZE(power_ui_sysfs_field_tbl)

static struct attribute *power_ui_sysfs_attrs[POWER_UI_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group power_ui_sysfs_attr_group = {
	.attrs = power_ui_sysfs_attrs,
};

static ssize_t power_ui_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_ui_dev *l_dev = power_ui_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_ui_sysfs_field_tbl, POWER_UI_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case POWER_UI_SYSFS_CABLE_TYPE:
		return scnprintf(buf, POWER_UI_RD_BUF_SIZE, "%d\n",
			l_dev->info.cable_type);
	case POWER_UI_SYSFS_MAX_POWER:
		return scnprintf(buf, POWER_UI_RD_BUF_SIZE, "%d\n",
			l_dev->info.max_power);
	case POWER_UI_SYSFS_WL_OFF_POS:
		return scnprintf(buf, POWER_UI_RD_BUF_SIZE, "%d\n",
			l_dev->info.wl_off_pos);
	case POWER_UI_SYSFS_WL_FAN_STATUS:
		return scnprintf(buf, POWER_UI_RD_BUF_SIZE, "%d\n",
			l_dev->info.wl_fan_status);
	case POWER_UI_SYSFS_WL_COVER_STATUS:
		return scnprintf(buf, POWER_UI_RD_BUF_SIZE, "%d\n",
			l_dev->info.wl_cover_status);
	case POWER_UI_SYSFS_WL_ACC_STATUS:
		return scnprintf(buf, POWER_UI_RD_BUF_SIZE, "%d\n",
			l_dev->info.wl_acc_status);
	case POWER_UI_SYSFS_WATER_STATUS:
		return scnprintf(buf, POWER_UI_RD_BUF_SIZE, "%d\n",
			l_dev->info.water_status);
	case POWER_UI_SYSFS_HEATING_STATUS:
		return scnprintf(buf, POWER_UI_RD_BUF_SIZE, "%d\n",
			l_dev->info.heating_status);
	case POWER_UI_SYSFS_ICON_TYPE:
		return scnprintf(buf, POWER_UI_RD_BUF_SIZE, "%d\n",
			l_dev->info.icon_type);
	default:
		return 0;
	}
}

static ssize_t power_ui_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_ui_dev *l_dev = power_ui_get_dev();
	char user[POWER_UI_WR_BUF_SIZE] = { 0 };
	int value;

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_ui_sysfs_field_tbl, POWER_UI_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (POWER_UI_WR_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 2: the fields of "user value" */
	if (sscanf(buf, "%s %d", user, &value) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (power_ui_get_op_user(user) < 0)
		return -EINVAL;

	hwlog_info("set: user=%s, value=%d\n", user, value);

	switch (info->name) {
	case POWER_UI_SYSFS_WL_FAN_STATUS:
		power_ui_event_notify(POWER_UI_NE_WL_FAN_STATUS, &value);
		break;
	case POWER_UI_SYSFS_WL_COVER_STATUS:
		power_ui_event_notify(POWER_UI_NE_WL_COVER_STATUS, &value);
		break;
	case POWER_UI_SYSFS_WL_ACC_STATUS:
		power_ui_event_notify(POWER_UI_NE_WL_ACC_STATUS, &value);
		break;
	case POWER_UI_SYSFS_WATER_STATUS:
		power_ui_event_notify(POWER_UI_NE_WATER_STATUS, &value);
		break;
	case POWER_UI_SYSFS_HEATING_STATUS:
		power_ui_event_notify(POWER_UI_NE_HEATING_STATUS, &value);
		break;
	default:
		break;
	}

	return count;
}

static struct device *power_ui_sysfs_create_group(void)
{
	power_sysfs_init_attrs(power_ui_sysfs_attrs,
		power_ui_sysfs_field_tbl, POWER_UI_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "power_ui",
		&power_ui_sysfs_attr_group);
}

static void power_ui_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_ui_sysfs_attr_group);
}
#else
static inline struct device *power_ui_sysfs_create_group(void)
{
	return NULL;
}

static inline void power_ui_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init power_ui_init(void)
{
	struct power_ui_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_power_ui_dev = l_dev;
	l_dev->dev = power_ui_sysfs_create_group();
	return 0;
}

static void __exit power_ui_exit(void)
{
	struct power_ui_dev *l_dev = g_power_ui_dev;

	if (!l_dev)
		return;

	power_ui_sysfs_remove_group(l_dev->dev);
	kfree(l_dev);
	g_power_ui_dev = NULL;
}

subsys_initcall_sync(power_ui_init);
module_exit(power_ui_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power ui module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
