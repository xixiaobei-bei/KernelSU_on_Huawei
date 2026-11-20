/*
 * vbus_monitor.c
 *
 * vbus monitor driver
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

#include "vbus_monitor.h"
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_vote.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG vbus_monitor
HWLOG_REGIST();

static struct vbus_dev *g_vbus_dev;

static struct vbus_dev *vbus_get_dev(void)
{
	if (!g_vbus_dev) {
		hwlog_err("g_vbus_dev is null\n");
		return NULL;
	}

	return g_vbus_dev;
}

static void vbus_send_absent_uevent(struct vbus_dev *l_dev)
{
	struct power_event_notify_data n_data;

	n_data.event = "VBUS_ABSENT=";
	n_data.event_len = 12; /* length of VBUS_ABSENT= */
	power_event_report_uevent(&n_data);
}

static void vbus_send_connect_uevent(struct vbus_dev *l_dev)
{
	struct power_event_notify_data n_data;

	/* ignore repeat event */
	if (l_dev->connect_state == VBUS_STATE_CONNECT)
		return;
	l_dev->connect_state = VBUS_STATE_CONNECT;

	n_data.event = "VBUS_CONNECT=";
	n_data.event_len = 13; /* length of VBUS_CONNECT= */
	power_event_report_uevent(&n_data);
}

static void vbus_send_disconnect_uevent(struct vbus_dev *l_dev)
{
	struct power_event_notify_data n_data;

	/* ignore repeat event */
	if (l_dev->connect_state == VBUS_STATE_DISCONNECT)
		return;
	l_dev->connect_state = VBUS_STATE_DISCONNECT;

	n_data.event = "VBUS_DISCONNECT=";
	n_data.event_len = 16; /* length of VBUS_DISCONNECT= */
	power_event_report_uevent(&n_data);

	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL, POWER_IF_SYSFS_READY, 0);
}

static void vbus_send_wireless_tx_open_uevent(void)
{
	struct power_event_notify_data n_data;

	n_data.event = "TX_OPEN=";
	n_data.event_len = 8; /* length of TX_OPEN= */
	power_event_report_uevent(&n_data);
}

static void vbus_send_wireless_tx_close_uevent(void)
{
	struct power_event_notify_data n_data;

	n_data.event = "TX_CLOSE=";
	n_data.event_len = 9; /* length of TX_CLOSE= */
	power_event_report_uevent(&n_data);
}

static void vbus_vote_send_connect_uevent(void)
{
	struct power_event_notify_data n_data;

	n_data.event = "VBUS_VOTE_CONNECT=";
	n_data.event_len = 18; /* length of VBUS_VOTE_CONNECT= */
	power_event_report_uevent(&n_data);
}

static void vbus_vote_send_disconnect_uevent(void)
{
	struct power_event_notify_data n_data;

	n_data.event = "VBUS_VOTE_DISCONNECT=";
	n_data.event_len = 21; /* length of VBUS_VOTE_DISCONNECT= */
	power_event_report_uevent(&n_data);
}

static int vbus_vote_callback(struct power_vote_object *obj,
	void *data, int result, const char *client_str)
{
	struct vbus_dev *l_dev = (struct vbus_dev *)data;

	if (!l_dev || !client_str) {
		hwlog_err("l_dev or client_str is null\n");
		return -EINVAL;
	}

	hwlog_info("result=%d client_str=%s\n", result, client_str);

	if (result)
		vbus_vote_send_connect_uevent();
	else
		vbus_vote_send_disconnect_uevent();

	return 0;
}

static int vbus_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct vbus_dev *l_dev = vbus_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_USB_DISCONNECT:
		vbus_send_disconnect_uevent(l_dev);
		power_vote_set(VBUS_MONITOR_VOTE_OBJECT, "usb", false, false);
		break;
	case POWER_NE_WIRELESS_DISCONNECT:
		vbus_send_disconnect_uevent(l_dev);
		power_vote_set(VBUS_MONITOR_VOTE_OBJECT, "wireless", false, false);
		break;
	case POWER_NE_USB_CONNECT:
		vbus_send_connect_uevent(l_dev);
		power_vote_set(VBUS_MONITOR_VOTE_OBJECT, "usb", true, true);
		break;
	case POWER_NE_WIRELESS_CONNECT:
		vbus_send_connect_uevent(l_dev);
		power_vote_set(VBUS_MONITOR_VOTE_OBJECT, "wireless", true, true);
		break;
	case POWER_NE_WIRELESS_TX_START:
		vbus_send_wireless_tx_open_uevent();
		power_vote_set(VBUS_MONITOR_VOTE_OBJECT, "wireless_tx", true, true);
		break;
	case POWER_NE_WIRELESS_TX_STOP:
		vbus_send_wireless_tx_close_uevent();
		power_vote_set(VBUS_MONITOR_VOTE_OBJECT, "wireless_tx", false, false);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static void vbus_absent_monitor(struct vbus_dev *l_dev)
{
	if (power_platform_in_dc_charging_stage()) {
		l_dev->absent_cnt = 0;
		return;
	}

	if (power_platform_get_vbus_status() == 0) {
		l_dev->absent_state = VBUS_STATE_ABSENT;
		l_dev->absent_cnt++;
	} else {
		l_dev->absent_state = VBUS_STATE_PRESENT;
		l_dev->absent_cnt = 0;
	}

	hwlog_info("state=%d, cnt=%d\n", l_dev->absent_state, l_dev->absent_cnt);
	if (l_dev->absent_cnt < VBUS_ABSENT_MAX_CNTS)
		return;

	vbus_send_absent_uevent(l_dev);
}

static void vbus_absent_monitor_work(struct work_struct *work)
{
	struct vbus_dev *l_dev = vbus_get_dev();

	if (!l_dev || !l_dev->absent_monitor_enabled)
		return;

	if (!power_cmdline_is_powerdown_charging_mode())
		return;

	vbus_absent_monitor(l_dev);
	schedule_delayed_work(&l_dev->absent_monitor_work,
		msecs_to_jiffies(VBUS_ABSENT_CHECK_TIME));
}

#ifdef CONFIG_SYSFS
static ssize_t vbus_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_sysfs_attr_info vbus_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(vbus, 0440, VBUS_SYSFS_ABSENT_STATE, absent_state),
	power_sysfs_attr_ro(vbus, 0440, VBUS_SYSFS_CONNECT_STATE, connect_state),
};

#define VBUS_SYSFS_ATTRS_SIZE  ARRAY_SIZE(vbus_sysfs_field_tbl)

static struct attribute *vbus_sysfs_attrs[VBUS_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group vbus_sysfs_attr_group = {
	.attrs = vbus_sysfs_attrs,
};

static ssize_t vbus_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct vbus_dev *l_dev = vbus_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		vbus_sysfs_field_tbl, VBUS_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case VBUS_SYSFS_ABSENT_STATE:
		return scnprintf(buf, PAGE_SIZE, "%d\n", l_dev->absent_state);
	case VBUS_SYSFS_CONNECT_STATE:
		return scnprintf(buf, PAGE_SIZE, "%d\n", l_dev->connect_state);
	default:
		return 0;
	}
}

static struct device *vbus_sysfs_create_group(void)
{
	power_sysfs_init_attrs(vbus_sysfs_attrs,
		vbus_sysfs_field_tbl, VBUS_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "vbus",
		&vbus_sysfs_attr_group);
}

static void vbus_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &vbus_sysfs_attr_group);
}
#else
static inline struct device *vbus_sysfs_create_group(void)
{
	return NULL;
}

static inline void vbus_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void vbus_parse_dts(struct vbus_dev *l_dev)
{
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,vbus_monitor", "absent_monitor_enabled",
		&l_dev->absent_monitor_enabled, 0);
}

static int __init vbus_init(void)
{
	int ret;
	struct vbus_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_vbus_dev = l_dev;
	vbus_parse_dts(l_dev);
	l_dev->nb.notifier_call = vbus_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CONNECT, &l_dev->nb);
	if (ret)
		goto fail_free_mem;

	l_dev->dev = vbus_sysfs_create_group();
	l_dev->absent_state = VBUS_STATE_PRESENT;
	l_dev->connect_state = VBUS_DEFAULT_CONNECT_STATE;
	power_vote_create_object(VBUS_MONITOR_VOTE_OBJECT, POWER_VOTE_SET_ANY,
		vbus_vote_callback, l_dev);

	INIT_DELAYED_WORK(&l_dev->absent_monitor_work, vbus_absent_monitor_work);
	schedule_delayed_work(&l_dev->absent_monitor_work,
		msecs_to_jiffies(VBUS_ABSENT_INIT_TIME));
	return 0;

fail_free_mem:
	kfree(l_dev);
	g_vbus_dev = NULL;
	return ret;
}

static void __exit vbus_exit(void)
{
	struct vbus_dev *l_dev = g_vbus_dev;

	if (!l_dev)
		return;

	power_vote_destroy_object(VBUS_MONITOR_VOTE_OBJECT);
	cancel_delayed_work(&l_dev->absent_monitor_work);
	power_event_bnc_unregister(POWER_BNT_CONNECT, &l_dev->nb);
	vbus_sysfs_remove_group(l_dev->dev);
	kfree(l_dev);
	g_vbus_dev = NULL;
}

fs_initcall_sync(vbus_init);
module_exit(vbus_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("vbus monitor module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
