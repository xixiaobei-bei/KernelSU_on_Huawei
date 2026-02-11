/*
 * power_event.c
 *
 * event for power module
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

#include <huawei_platform/power/power_event.h>
#include <huawei_platform/power/power_sysfs.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger_uevent.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/power/hisi/hisi_bci_battery.h>

#define HWLOG_TAG power_event
HWLOG_REGIST();

struct power_event_dev *g_power_event_dev;
static BLOCKING_NOTIFIER_HEAD(g_power_event_nh);

static struct power_event_dev *power_event_get_dev(void)
{
	if (!g_power_event_dev) {
		hwlog_err("g_power_event_dev is null\n");
		return NULL;
	}

	return g_power_event_dev;
}

static void power_event_notify_sysfs(struct power_event_dev *l_dev,
	const char *attr)
{
	if (!l_dev || !l_dev->sysfs_ne)
		return;

	sysfs_notify(l_dev->sysfs_ne, NULL, attr);
}

static int power_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct power_event_dev *l_dev = power_event_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	hwlog_info("receive event %d\n", event);

	switch (event) {
	case POWER_EVENT_NE_USB_DISCONNECT:
	case POWER_EVENT_NE_WIRELESS_DISCONNECT:
		/* ignore repeat event */
		if (l_dev->connect_state == POWER_EVENT_DISCONNECT)
			break;
		l_dev->connect_state = POWER_EVENT_DISCONNECT;
		power_event_notify_sysfs(l_dev, "connect_state");
		break;
	case POWER_EVENT_NE_USB_CONNECT:
	case POWER_EVENT_NE_WIRELESS_CONNECT:
		/* ignore repeat event */
		if (l_dev->connect_state == POWER_EVENT_CONNECT)
			break;
		l_dev->connect_state = POWER_EVENT_CONNECT;
		power_event_notify_sysfs(l_dev, "connect_state");
		break;
	default:
		hwlog_info("unknown notifier event\n");
		break;
	}

	return NOTIFY_OK;
}

static int power_event_notifier_chain_register(struct notifier_block *nb)
{
	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return blocking_notifier_chain_register(&g_power_event_nh, nb);
}

static int power_event_notifier_chain_unregister(struct notifier_block *nb)
{
	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return blocking_notifier_chain_unregister(&g_power_event_nh, nb);
}

void power_event_notify(unsigned long event, void *data)
{
	blocking_notifier_call_chain(&g_power_event_nh, event, data);
}

#ifdef CONFIG_SYSFS
static ssize_t power_event_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_sysfs_attr_info power_event_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(power_event, 0440, POWER_EVENT_SYSFS_CONNECT_STATE, connect_state),
};

#define POWER_EVENT_SYSFS_ATTRS_SIZE  ARRAY_SIZE(power_event_sysfs_field_tbl)

static struct attribute *power_event_sysfs_attrs[POWER_EVENT_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group power_event_sysfs_attr_group = {
	.attrs = power_event_sysfs_attrs,
};

static void power_event_sysfs_init_attrs(void)
{
	int s;

	for (s = 0; s < POWER_EVENT_SYSFS_ATTRS_SIZE; s++)
		power_event_sysfs_attrs[s] = &power_event_sysfs_field_tbl[s].attr.attr;

	power_event_sysfs_attrs[s] = NULL;
}

static struct power_sysfs_attr_info *power_event_sysfs_field_lookup(
	const char *name)
{
	int s;

	for (s = 0; s < POWER_EVENT_SYSFS_ATTRS_SIZE; s++) {
		if (!strncmp(name,
			power_event_sysfs_field_tbl[s].attr.attr.name,
			strlen(name)))
			break;
	}

	if (s >= POWER_EVENT_SYSFS_ATTRS_SIZE)
		return NULL;

	return &power_event_sysfs_field_tbl[s];
}

static ssize_t power_event_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_event_dev *l_dev = power_event_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = power_event_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case POWER_EVENT_SYSFS_CONNECT_STATE:
		return scnprintf(buf, PAGE_SIZE, "%d\n", l_dev->connect_state);
	default:
		hwlog_err("invalid sysfs_name\n");
		return 0;
	}
}
#endif /* CONFIG_SYSFS */

static int __init power_event_init(void)
{
	int ret;
	struct power_event_dev *l_dev = NULL;
#ifdef CONFIG_SYSFS
	struct class *power_class = NULL;
#endif /* CONFIG_SYSFS */

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_power_event_dev = l_dev;
	l_dev->nb.notifier_call = power_event_notifier_call;
	ret = power_event_notifier_chain_register(&l_dev->nb);
	if (ret)
		goto fail_free_mem;

#ifdef CONFIG_SYSFS
	power_event_sysfs_init_attrs();
	power_class = hw_power_get_class();
	if (power_class) {
		l_dev->dev = device_create(power_class, NULL, 0, NULL,
			"power_event");
		if (IS_ERR(l_dev->dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(l_dev->dev);
			goto fail_create_device;
		}

		ret = sysfs_create_group(&l_dev->dev->kobj,
			&power_event_sysfs_attr_group);
		if (ret) {
			hwlog_err("sysfs group create failed\n");
			goto fail_create_sysfs;
		}
	}
#endif /* CONFIG_SYSFS */

	if (l_dev->dev)
		l_dev->sysfs_ne = &l_dev->dev->kobj;
	l_dev->connect_state = POWER_EVENT_INVAID;

	return 0;

#ifdef CONFIG_SYSFS
fail_create_sysfs:
fail_create_device:
	power_event_notifier_chain_unregister(&l_dev->nb);
#endif /* CONFIG_SYSFS */
fail_free_mem:
	kfree(l_dev);
	g_power_event_dev = NULL;

	return ret;
}

static void __exit power_event_exit(void)
{
	struct power_event_dev *l_dev = g_power_event_dev;

	if (!l_dev)
		return;
	power_event_notifier_chain_unregister(&l_dev->nb);
#ifdef CONFIG_SYSFS
	sysfs_remove_group(&l_dev->dev->kobj, &power_event_sysfs_attr_group);
	kobject_del(&l_dev->dev->kobj);
	kobject_put(&l_dev->dev->kobj);
#endif /* CONFIG_SYSFS */
	kfree(l_dev);
	g_power_event_dev = NULL;
}

fs_initcall_sync(power_event_init);
module_exit(power_event_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power event module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
