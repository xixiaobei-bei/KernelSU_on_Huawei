/*
 * bat_heating.c
 *
 * battery heating of low temperature control driver
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

#include "bat_heating.h"
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_nv.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_ui_ne.h>
#include <chipset_common/hwpower/common_module/power_temp.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG bat_heating
HWLOG_REGIST();

static struct bat_heating_dev *g_bat_heating_dev;

static const char * const g_bat_heating_op_user_table[BAT_HEATING_OP_USER_END] = {
	[BAT_HEATING_OP_USER_SHELL] = "shell",
	[BAT_HEATING_OP_USER_BMS_HEATING] = "bms_heating",
};

static ssize_t bat_heating_dbg_show(void *dev_data, char *buf, size_t size)
{
	struct bat_heating_dev *l_dev = dev_data;

	if (!buf || !l_dev) {
		hwlog_err("buf or l_dev is null\n");
		return scnprintf(buf, size, "buf or l_dev is null\n");
	}

	return scnprintf(buf, size,
		"low_temp_min_thld=%d\n"
		"low_temp_max_thld=%d\n"
		"low_temp_hysteresis=%d\n"
		"low_temp_min_ibat=%d\n",
		l_dev->low_temp_min_thld,
		l_dev->low_temp_max_thld,
		l_dev->low_temp_hysteresis,
		l_dev->low_temp_min_ibat);
}

static ssize_t bat_heating_dbg_store(void *dev_data, const char *buf,
	size_t size)
{
	struct bat_heating_dev *l_dev = dev_data;
	int low_temp_min_thld = 0;
	int low_temp_max_thld = 0;
	int low_temp_hysteresis = 0;
	int low_temp_min_ibat = 0;

	if (!buf || !l_dev) {
		hwlog_err("buf or l_dev is null\n");
		return -EINVAL;
	}

	/* 4: four parameters */
	if (sscanf(buf, "%d %d %d %d",
		&low_temp_min_thld,
		&low_temp_max_thld,
		&low_temp_hysteresis,
		&low_temp_min_ibat) != 4) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	l_dev->low_temp_min_thld = low_temp_min_thld;
	l_dev->low_temp_max_thld = low_temp_max_thld;
	l_dev->low_temp_hysteresis = low_temp_hysteresis;
	l_dev->low_temp_min_ibat = low_temp_min_ibat;

	hwlog_info("low_temp_min_thld=%d\n", l_dev->low_temp_min_thld);
	hwlog_info("low_temp_max_thld=%d\n", l_dev->low_temp_max_thld);
	hwlog_info("low_temp_hysteresis=%d\n", l_dev->low_temp_hysteresis);
	hwlog_info("low_temp_min_ibat=%d\n", l_dev->low_temp_min_ibat);

	return size;
}

static int bat_heating_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_bat_heating_op_user_table); i++) {
		if (!strcmp(str, g_bat_heating_op_user_table[i]))
			return i;
	}

	hwlog_err("invalid user_str=%s\n", str);
	return -1;
}

static struct bat_heating_dev *bat_heating_get_dev(void)
{
	if (!g_bat_heating_dev) {
		hwlog_err("g_bat_heating_dev is null\n");
		return NULL;
	}

	return g_bat_heating_dev;
}

static void bat_heating_send_uevent(struct bat_heating_dev *l_dev, int type)
{
	struct power_event_notify_data n_data;
	int value;

	switch (type) {
	case BAT_HEATING_START_HEAT_UP:
		if (l_dev->heat_up_flag)
			return;
		l_dev->heat_up_flag = true;
		n_data.event = "HEATING_START=";
		n_data.event_len = 14; /* length of HEATING_START= */
		power_event_report_uevent(&n_data);
		break;
	case BAT_HEATING_STOP_HEAT_UP:
		if (!l_dev->heat_up_flag)
			return;
		n_data.event = "HEATING_STOP=";
		n_data.event_len = 13; /* length of HEATING_STOP= */
		power_event_report_uevent(&n_data);
		break;
	case BAT_HEATING_POPUP_UI_MSG:
		if (l_dev->ui_msg_flag)
			return;
		l_dev->ui_msg_flag = true;
		value = 1; /* 1: popup */
		power_ui_event_notify(POWER_UI_NE_HEATING_STATUS, &value);
		break;
	case BAT_HEATING_REMOVE_UI_MSG:
		if (!l_dev->ui_msg_flag)
			return;
		value = 0; /* 0: remove */
		power_ui_event_notify(POWER_UI_NE_HEATING_STATUS, &value);
		break;
	default:
		break;
	}
}

static void bat_heating_set_default_data(struct bat_heating_dev *l_dev)
{
	bat_heating_send_uevent(l_dev, BAT_HEATING_REMOVE_UI_MSG);
	bat_heating_send_uevent(l_dev, BAT_HEATING_STOP_HEAT_UP);

	l_dev->hysteresis = 0;
	l_dev->trigger = 0;
	l_dev->bat_temp = 0;
	l_dev->usb_temp = 0;
	l_dev->usb_state = BAT_HEATING_USB_DISCONNECT;
	l_dev->screen_state = BAT_HEATING_SCREEN_ON;
	l_dev->heat_up_flag = false;
	l_dev->ui_msg_flag = false;
	l_dev->overload_count = 0;
}

static bool bat_heating_check_charger_type(struct bat_heating_dev *l_dev)
{
	unsigned int type = power_platform_get_charger_type();

	if ((type == CHARGER_TYPE_STANDARD) ||
		(type == CHARGER_TYPE_FCP) ||
		(type == CHARGER_TYPE_SCP) ||
		(type == CHARGER_TYPE_PD))
		return false;

	hwlog_info("check: charger_type=%d is invalid\n", type);
	return true;
}

static bool bat_heating_check_screen_state(struct bat_heating_dev *l_dev)
{
	if (l_dev->screen_state != BAT_HEATING_SCREEN_ON)
		return false;

	hwlog_info("check: screen on\n");
	return true;
}

static bool bat_heating_check_usb_state(struct bat_heating_dev *l_dev)
{
	if (l_dev->usb_state != BAT_HEATING_USB_DISCONNECT)
		return false;

	hwlog_info("check: usb is disconnect\n");
	return true;
}

static bool bat_heating_check_soc(struct bat_heating_dev *l_dev)
{
	int soc = power_platform_get_battery_capacity();

	if (soc < BAT_HEATING_FULL_SOC)
		return false;

	hwlog_info("check: soc=%d is invalid\n", soc);
	return true;
}

static bool bat_heating_check_current(struct bat_heating_dev *l_dev)
{
	int cur = -power_platform_get_battery_current();

	if (cur >= l_dev->low_temp_min_ibat) {
		l_dev->overload_count = 0;
		return false;
	}

	hwlog_info("check: count=%d current=%d is overload\n",
		l_dev->overload_count, cur);
	if (l_dev->overload_count++ >= BAT_HEATING_OVERLOAD_THLD)
		return true;
	return false;
}

static bool bat_heating_check_temp(struct bat_heating_dev *l_dev)
{
	l_dev->usb_temp = power_temp_get_average_value(POWER_TEMP_USB_PORT);
	l_dev->usb_temp /= POWER_TEMP_UNIT;
	l_dev->bat_temp = power_platform_get_battery_temperature();

	if (abs(l_dev->bat_temp - l_dev->usb_temp) >= BAT_HEATING_USB_TEMP_THLD) {
		hwlog_info("check: usb_temp=%d bat_temp=%d\n",
			l_dev->usb_temp, l_dev->bat_temp);
		return true;
	}

	if ((l_dev->bat_temp < l_dev->low_temp_min_thld) ||
		(l_dev->bat_temp > l_dev->low_temp_max_thld + l_dev->hysteresis)) {
		hwlog_info("check: bat_temp=%d min_thld=%d max_thld=%d\n",
			l_dev->bat_temp,
			l_dev->low_temp_min_thld,
			l_dev->low_temp_max_thld + l_dev->hysteresis);
		return true;
	}

	return false;
}

static int bat_heating_check(struct bat_heating_dev *l_dev)
{
	if (bat_heating_check_charger_type(l_dev))
		goto end_check;

	if (bat_heating_check_usb_state(l_dev))
		goto end_check;

	if (bat_heating_check_soc(l_dev))
		goto end_check;

	if (bat_heating_check_current(l_dev))
		goto end_check;

	if (bat_heating_check_temp(l_dev))
		goto end_check;

	return 0;

end_check:
	bat_heating_set_default_data(l_dev);
	return -EINVAL;
}

static void bat_heating_set_charging_current(struct bat_heating_dev *l_dev,
	int cur)
{
	/* limit buck input current */
	if (cur == -1) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, l_dev->buck_iin_limit);
		return;
	}

	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
		POWER_IF_SYSFS_VBUS_IIN_LIMIT, 0);
	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
		POWER_IF_SYSFS_BATT_ICHG_LIMIT, 0);

	/* recovery charging current */
	if (cur == 0) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_LVC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, 0);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_SC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, 0);
		return;
	}

	if (l_dev->bat_temp >= BAT_HEATING_DC_TEMP_THLD) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_LVC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, cur);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_SC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, cur);
	} else {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_BATT_ICHG_LIMIT, cur);
	}
}

static void bat_heating_select_charging_current(struct bat_heating_dev *l_dev)
{
	int i;
	int ichg = 0;

	for (i = 0; i < BAT_HEATING_TEMP_LEVEL; ++i) {
		if ((l_dev->bat_temp >= l_dev->temp_para[i].temp_min) &&
			(l_dev->bat_temp < l_dev->temp_para[i].temp_max)) {
			ichg = l_dev->temp_para[i].temp_ichg;
			break;
		}
	}

	bat_heating_set_charging_current(l_dev, ichg);
}

static void bat_heating_monitor_work(struct work_struct *work)
{
	struct bat_heating_dev *l_dev = bat_heating_get_dev();

	if (!l_dev)
		return;

	if (bat_heating_check(l_dev))
		return;

	if (bat_heating_check_screen_state(l_dev)) {
		/* popup ui message */
		bat_heating_send_uevent(l_dev, BAT_HEATING_POPUP_UI_MSG);
		/* stop heat up */
		bat_heating_send_uevent(l_dev, BAT_HEATING_STOP_HEAT_UP);
		l_dev->heat_up_flag = false;
		goto schedule_work;
	}

	/* start heat up */
	bat_heating_send_uevent(l_dev, BAT_HEATING_START_HEAT_UP);

	l_dev->hysteresis = l_dev->low_temp_hysteresis;
	if (l_dev->trigger)
		bat_heating_select_charging_current(l_dev);

schedule_work:
	schedule_delayed_work(&l_dev->monitor_work,
		msecs_to_jiffies(DELAY_TIME_FOR_MONITOR_WORK));
}

static void bat_heating_read_count_work(struct work_struct *work)
{
	struct bat_heating_dev *l_dev = bat_heating_get_dev();

	if (!l_dev)
		return;

	(void)power_nv_read(POWER_NV_BATHEAT,
		&l_dev->count, sizeof(l_dev->count));
	hwlog_info("read nv count=%d\n", l_dev->count);
}

static void bat_heating_write_count_work(struct work_struct *work)
{
	struct bat_heating_dev *l_dev = bat_heating_get_dev();

	if (!l_dev)
		return;

	(void)power_nv_write(POWER_NV_BATHEAT,
		&l_dev->count, sizeof(l_dev->count));
	hwlog_info("write nv count=%d\n", l_dev->count);
}

static void bat_heating_report_dmd_work(struct work_struct *work)
{
	struct bat_heating_dev *l_dev = bat_heating_get_dev();
	char buf[POWER_DSM_BUF_SIZE_0128] = { 0 };

	if (l_dev->dmd_count++ >= BAT_HEATING_DMD_REPORT_COUNTS) {
		hwlog_info("dmd report over %d time\n", l_dev->dmd_count);
		return;
	}

	snprintf(buf, POWER_DSM_BUF_SIZE_0128 - 1,
		"count=%d chg_type=%d brand=%s volt=%d soc=%d t_bat=%d t_usb=%d\n",
		l_dev->count,
		power_platform_get_charger_type(),
		power_platform_get_battery_brand(),
		power_platform_get_battery_voltage(),
		power_platform_get_battery_capacity(),
		l_dev->bat_temp,
		l_dev->usb_temp);
	power_dsm_report_dmd(POWER_DSM_BATTERY, DSM_BATTERY_HEATING, buf);
}

static void bat_heating_report_dmd(struct bat_heating_dev *l_dev)
{
	if (l_dev->trigger)
		schedule_delayed_work(&l_dev->rpt_dmd_work,
			msecs_to_jiffies(DELAY_TIME_FOR_DMD_WORK));
}

static void bat_heating_start(struct bat_heating_dev *l_dev)
{
	l_dev->usb_state = BAT_HEATING_USB_CONNECT;
	cancel_delayed_work(&l_dev->monitor_work);
	cancel_delayed_work(&l_dev->rpt_dmd_work);
	schedule_delayed_work(&l_dev->monitor_work,
		msecs_to_jiffies(DELAY_TIME_FOR_INIT_MONITOR_WORK));
}

static void bat_heating_stop(struct bat_heating_dev *l_dev)
{
	if (l_dev->trigger)
		bat_heating_set_charging_current(l_dev, 0);
	bat_heating_set_default_data(l_dev);
	cancel_delayed_work(&l_dev->monitor_work);
	cancel_delayed_work(&l_dev->rpt_dmd_work);
}

static int bat_heating_fb_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct bat_heating_dev *l_dev = bat_heating_get_dev();
	struct fb_event *blank_event = (struct fb_event *)data;
	int *blank = NULL;

	if (!l_dev || !blank_event || !blank_event->data) {
		hwlog_err("l_dev or blank_event is null\n");
		return NOTIFY_DONE;
	}

	blank = blank_event->data;

	if ((event == FB_EVENT_BLANK) && (*blank == FB_BLANK_UNBLANK)) {
		l_dev->screen_state = BAT_HEATING_SCREEN_ON;
		hwlog_info("screen on\n");
	} else if ((event == FB_EVENT_BLANK) && (*blank == FB_BLANK_POWERDOWN)) {
		l_dev->screen_state = BAT_HEATING_SCREEN_OFF;
		hwlog_info("screen off\n");
	}

	return NOTIFY_DONE;
}

static int bat_heating_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct bat_heating_dev *l_dev = bat_heating_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_STOP_CHARGING:
		bat_heating_stop(l_dev);
		break;
	case POWER_NE_START_CHARGING:
		bat_heating_start(l_dev);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

#ifdef CONFIG_SYSFS
static ssize_t bat_heating_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t bat_heating_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info bat_heating_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(bat_heating, 0640, BAT_HEATING_SYSFS_COUNT, count),
	power_sysfs_attr_rw(bat_heating, 0640, BAT_HEATING_SYSFS_TRIGGER, trigger),
};

#define BAT_HEATING_SYSFS_ATTRS_SIZE  ARRAY_SIZE(bat_heating_sysfs_field_tbl)

static struct attribute *bat_heating_sysfs_attrs[BAT_HEATING_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group bat_heating_sysfs_attr_group = {
	.attrs = bat_heating_sysfs_attrs,
};

static ssize_t bat_heating_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct bat_heating_dev *l_dev = bat_heating_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		bat_heating_sysfs_field_tbl, BAT_HEATING_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case BAT_HEATING_SYSFS_COUNT:
		return scnprintf(buf, PAGE_SIZE, "%d\n", l_dev->count);
	case BAT_HEATING_SYSFS_TRIGGER:
		return scnprintf(buf, PAGE_SIZE, "%d\n", l_dev->trigger);
	default:
		return 0;
	}
}

static ssize_t bat_heating_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct bat_heating_dev *l_dev = bat_heating_get_dev();
	char user[BAT_HEATING_RW_BUF_SIZE] = { 0 };
	int value;

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		bat_heating_sysfs_field_tbl, BAT_HEATING_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (BAT_HEATING_RW_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 2: the fields of "user value" */
	if (sscanf(buf, "%s %d", user, &value) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (bat_heating_get_op_user(user) < 0)
		return -EINVAL;

	hwlog_info("set: name=%d, user=%s, value=%d\n",
		info->name, user, value);

	switch (info->name) {
	case BAT_HEATING_SYSFS_COUNT:
		l_dev->count = value;
		schedule_delayed_work(&l_dev->wr_count_work, msecs_to_jiffies(0));
		break;
	case BAT_HEATING_SYSFS_TRIGGER:
		if (l_dev->trigger) {
			hwlog_info("ignore the same trigger\n");
			break;
		}
		l_dev->trigger = value;
		bat_heating_report_dmd(l_dev);
		break;
	default:
		break;
	}

	return count;
}

static struct device *bat_heating_sysfs_create_group(void)
{
	power_sysfs_init_attrs(bat_heating_sysfs_attrs,
		bat_heating_sysfs_field_tbl, BAT_HEATING_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "bat_heating",
		&bat_heating_sysfs_attr_group);
}

static void bat_heating_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &bat_heating_sysfs_attr_group);
}
#else
static inline struct device *bat_heating_sysfs_create_group(void)
{
	return NULL;
}

static inline void bat_heating_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void bat_heating_parse_temp_para(struct device_node *np,
	struct bat_heating_dev *l_dev)
{
	int row, col, len;
	int idata[BAT_HEATING_TEMP_LEVEL * BAT_HEATING_TEMP_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"temp_para", idata, BAT_HEATING_TEMP_LEVEL, BAT_HEATING_TEMP_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / BAT_HEATING_TEMP_TOTAL; row++) {
		col = row * BAT_HEATING_TEMP_TOTAL + BAT_HEATING_TEMP_MIN;
		l_dev->temp_para[row].temp_min = idata[col];

		col = row * BAT_HEATING_TEMP_TOTAL + BAT_HEATING_TEMP_MAX;
		l_dev->temp_para[row].temp_max = idata[col];

		col = row * BAT_HEATING_TEMP_TOTAL + BAT_HEATING_TEMP_ICHG;
		l_dev->temp_para[row].temp_ichg = idata[col];
	}
}

static void bat_heating_parse_dts(struct device_node *np,
	struct bat_heating_dev *l_dev)
{
	(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
		"low_temp_min_thld", &l_dev->low_temp_min_thld,
		DEFAULT_LOW_TEMP_MIN_THLD);
	(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
		"low_temp_max_thld", &l_dev->low_temp_max_thld,
		DEFAULT_LOW_TEMP_MAX_THLD);
	(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
		"low_temp_hysteresis", &l_dev->low_temp_hysteresis,
		DEFAULT_LOW_TEMP_HYSTERESIS);
	(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
		"low_temp_min_ibat", &l_dev->low_temp_min_ibat,
		DEFAULT_LOW_TEMP_MIN_IBAT);
	(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
		"buck_iin_limit", &l_dev->buck_iin_limit,
		DEFAULT_BUCK_IIN_LIMIT);

	bat_heating_parse_temp_para(np, l_dev);
}

static void bat_heating_dbg_register(struct bat_heating_dev *l_dev)
{
	power_dbg_ops_register("bat_heating", (void *)l_dev,
		(power_dbg_show)bat_heating_dbg_show,
		(power_dbg_store)bat_heating_dbg_store);
}

static int bat_heating_probe(struct platform_device *pdev)
{
	int ret;
	struct bat_heating_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (power_cmdline_is_powerdown_charging_mode() ||
		power_cmdline_is_factory_mode()) {
		hwlog_info("not support battery heating\n");
		return -ENODEV;
	}

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_bat_heating_dev = l_dev;
	np = pdev->dev.of_node;
	bat_heating_parse_dts(np, l_dev);

	INIT_DELAYED_WORK(&l_dev->monitor_work, bat_heating_monitor_work);
	INIT_DELAYED_WORK(&l_dev->rd_count_work, bat_heating_read_count_work);
	INIT_DELAYED_WORK(&l_dev->wr_count_work, bat_heating_write_count_work);
	INIT_DELAYED_WORK(&l_dev->rpt_dmd_work, bat_heating_report_dmd_work);

	l_dev->nb.notifier_call = bat_heating_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &l_dev->nb);
	if (ret)
		goto fail_free_mem;

	l_dev->fb_nb.notifier_call = bat_heating_fb_notifier_call;
	ret = fb_register_client(&l_dev->fb_nb);
	if (ret)
		goto fail_reg_fb;

	l_dev->dev = bat_heating_sysfs_create_group();
	bat_heating_dbg_register(l_dev);
	platform_set_drvdata(pdev, l_dev);
	schedule_delayed_work(&l_dev->rd_count_work, msecs_to_jiffies(DELAY_TIME_FOR_COUNT_WORK));
	return 0;

fail_reg_fb:
	power_event_bnc_unregister(POWER_BNT_CHARGING, &l_dev->nb);
fail_free_mem:
	kfree(l_dev);
	g_bat_heating_dev = NULL;
	return ret;
}

static int bat_heating_remove(struct platform_device *pdev)
{
	struct bat_heating_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	cancel_delayed_work(&l_dev->monitor_work);
	cancel_delayed_work(&l_dev->rd_count_work);
	cancel_delayed_work(&l_dev->wr_count_work);
	cancel_delayed_work(&l_dev->rpt_dmd_work);
	fb_unregister_client(&l_dev->fb_nb);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &l_dev->nb);
	bat_heating_sysfs_remove_group(l_dev->dev);
	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_bat_heating_dev = NULL;

	return 0;
}

static const struct of_device_id bat_heating_match_table[] = {
	{
		.compatible = "huawei,bat_heating",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_heating_driver = {
	.probe = bat_heating_probe,
	.remove = bat_heating_remove,
	.driver = {
		.name = "huawei,bat_heating",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_heating_match_table),
	},
};

static int __init bat_heating_init(void)
{
	return platform_driver_register(&bat_heating_driver);
}

static void __exit bat_heating_exit(void)
{
	platform_driver_unregister(&bat_heating_driver);
}

fs_initcall_sync(bat_heating_init);
module_exit(bat_heating_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery heating control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
