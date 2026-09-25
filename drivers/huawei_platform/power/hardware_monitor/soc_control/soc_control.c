/*
 * soc_control.c
 *
 * battery capacity(soc: state of charge) control driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <huawei_platform/power/power_sysfs.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#include "soc_control.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG soc_control
HWLOG_REGIST();

static struct soc_ctrl_dev *g_soc_ctrl_dev;

static const char * const g_soc_ctrl_op_user_table[SOC_CTRL_OP_USER_END] = {
	[SOC_CTRL_OP_USER_DEFAULT] = "default",
	[SOC_CTRL_OP_USER_RC] = "rc",
	[SOC_CTRL_OP_USER_HIDL] = "hidl",
	[SOC_CTRL_OP_USER_BMS_SOC] = "bms_soc",
	[SOC_CTRL_OP_USER_SHELL] = "shell",
	[SOC_CTRL_OP_USER_CUST] = "cust",
	[SOC_CTRL_OP_USER_BSOH] = "bsoh",
};

static int soc_ctrl_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_soc_ctrl_op_user_table); i++) {
		if (!strcmp(str, g_soc_ctrl_op_user_table[i]))
			return i;
	}

	hwlog_err("invalid user_str=%s\n", str);
	return -1;
}

static const char *soc_ctrl_get_op_user_name(unsigned int index)
{
	if ((index >= SOC_CTRL_OP_USER_BEGIN) && (index < SOC_CTRL_OP_USER_END))
		return g_soc_ctrl_op_user_table[index];

	return "invalid user";
}

static struct soc_ctrl_dev *soc_ctrl_get_dev(void)
{
	if (!g_soc_ctrl_dev) {
		hwlog_err("g_soc_ctrl_dev is null\n");
		return NULL;
	}

	return g_soc_ctrl_dev;
}

/* class a of soc control strategy will fluctuate between 55% and 75% */
static void soc_ctrl_startup_control_class_a(struct soc_ctrl_dev *l_dev)
{
	int cur_soc = hisi_battery_capacity();

	hwlog_info("startup_a=%d cur_soc=%d, min_soc=%d, max_soc=%d\n",
		l_dev->work_mode, cur_soc, l_dev->min_soc, l_dev->max_soc);

	/*
	 * for example, soc between 75% and 100%,
	 * we need disable charging and set input current to 100ma
	 */
	if ((cur_soc > l_dev->max_soc) &&
		(l_dev->work_mode != WORK_IN_DISABLE_CHG_MODE)) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL,
			POWER_IF_SYSFS_ENABLE_CHARGER, SOC_CTRL_CHG_DISABLE);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, SOC_CTRL_IIN_LIMIT);

		l_dev->work_mode = WORK_IN_DISABLE_CHG_MODE;
	}

	/*
	 * for example, soc between 0% and 55%,
	 * we need enable charging and recovery input current
	 */
	if ((cur_soc < l_dev->min_soc) &&
		(l_dev->work_mode != WORK_IN_ENABLE_CHG_MODE)) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL,
			POWER_IF_SYSFS_ENABLE_CHARGER, SOC_CTRL_CHG_ENABLE);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, SOC_CTRL_IIN_UNLIMIT);

		l_dev->work_mode = WORK_IN_ENABLE_CHG_MODE;
	}
}

/* class b of soc control strategy will keep between 55% and 75% */
static void soc_ctrl_startup_control_class_b(struct soc_ctrl_dev *l_dev)
{
	int cur_soc = hisi_battery_capacity();

	hwlog_info("startup_b=%d cur_soc=%d, min_soc=%d, max_soc=%d\n",
		l_dev->work_mode, cur_soc, l_dev->min_soc, l_dev->max_soc);

	/*
	 * for example, soc between 75% and 100%,
	 * we need disable charging and set input current to 100ma
	 */
	if ((cur_soc > l_dev->max_soc) &&
		(l_dev->work_mode != WORK_IN_DISABLE_CHG_MODE)) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL,
			POWER_IF_SYSFS_ENABLE_CHARGER, SOC_CTRL_CHG_DISABLE);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, SOC_CTRL_IIN_LIMIT);

		l_dev->work_mode = WORK_IN_DISABLE_CHG_MODE;
	}

	/*
	 * for example, soc keep at between 55% and 75%,
	 * we need disable charging and recovery input current
	 */
	if ((cur_soc == l_dev->max_soc) &&
		(l_dev->work_mode == WORK_IN_DISABLE_CHG_MODE)) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL,
			POWER_IF_SYSFS_ENABLE_CHARGER, SOC_CTRL_CHG_DISABLE);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, SOC_CTRL_IIN_UNLIMIT);

		l_dev->work_mode = WORK_IN_BALANCE_MODE;
	}

	/*
	 * for example, soc between 0% and 55%,
	 * we need enable charging and recovery input current
	 */
	if ((cur_soc < l_dev->min_soc) &&
		(l_dev->work_mode != WORK_IN_ENABLE_CHG_MODE)) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL,
			POWER_IF_SYSFS_ENABLE_CHARGER, SOC_CTRL_CHG_ENABLE);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, SOC_CTRL_IIN_UNLIMIT);

		l_dev->work_mode = WORK_IN_ENABLE_CHG_MODE;
	}
}

static void soc_ctrl_startup_control(struct soc_ctrl_dev *l_dev)
{
	switch (l_dev->strategy) {
	case STRATEGY_TYPE_CLASS_A:
		soc_ctrl_startup_control_class_a(l_dev);
		break;
	case STRATEGY_TYPE_CLASS_B:
		soc_ctrl_startup_control_class_b(l_dev);
		break;
	default:
		break;
	}
}

static void soc_ctrl_recovery_control(struct soc_ctrl_dev *l_dev)
{
	if (l_dev->work_mode == WORK_IN_DEFAULT_MODE)
		return;

	hwlog_info("recovery=%d cur_soc=%d, min_soc=%d, max_soc=%d\n",
		l_dev->work_mode,
		hisi_battery_capacity(), l_dev->min_soc, l_dev->max_soc);

	l_dev->work_mode = WORK_IN_DEFAULT_MODE;

	/* enable charging and recovery input current */
	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL,
		POWER_IF_SYSFS_ENABLE_CHARGER, SOC_CTRL_CHG_ENABLE);
	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
		POWER_IF_SYSFS_VBUS_IIN_LIMIT, SOC_CTRL_IIN_UNLIMIT);
}

static void soc_ctrl_event_work(struct work_struct *work)
{
	struct soc_ctrl_dev *l_dev = NULL;

	l_dev = soc_ctrl_get_dev();
	if (!l_dev)
		return;

	soc_ctrl_startup_control(l_dev);

	schedule_delayed_work(&l_dev->work,
		msecs_to_jiffies(SOC_CTRL_LOOP_TIME));
}

static void soc_ctrl_event_control(int event)
{
	struct soc_ctrl_dev *l_dev = soc_ctrl_get_dev();

	if (!l_dev)
		return;

	/*
	 * enable flag is set 0
	 * case1: bypass if not start soc control
	 * case2: stop soc control if soc control has started
	 */
	if (l_dev->enable == 0) {
		hwlog_info("disable: event=%d\n", event);

		cancel_delayed_work(&l_dev->work);
		soc_ctrl_recovery_control(l_dev);

		return;
	}

	/*
	 * enable flag is set 1
	 * case1: start soc control with 30s delay when usb insert
	 * case2: stop soc control when usb remove
	 */
	if (event == SOC_CTRL_START_EVENT) {
		hwlog_info("enable: start soc control\n");

		cancel_delayed_work(&l_dev->work);
		schedule_delayed_work(&l_dev->work,
			msecs_to_jiffies(SOC_CTRL_START_TIME));
	} else {
		hwlog_info("enable: stop soc control\n");

		l_dev->event = SOC_CTRL_DEFAULT_EVENT;
		cancel_delayed_work(&l_dev->work);
		soc_ctrl_recovery_control(l_dev);
	}
}

static int soc_ctrl_event_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct soc_ctrl_dev *l_dev = soc_ctrl_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	/*
	 * after receiving the message of non-stop charging,
	 * we set the event to start, otherwise set the event to stop
	 */
	if (event != CHARGER_STOP_CHARGING_EVENT) {
		/* ignore repeat event */
		if (l_dev->event == SOC_CTRL_START_EVENT)
			return NOTIFY_OK;

		l_dev->event = SOC_CTRL_START_EVENT;
	} else {
		/* ignore repeat event */
		if (l_dev->event == SOC_CTRL_STOP_EVENT)
			return NOTIFY_OK;

		l_dev->event = SOC_CTRL_STOP_EVENT;
	}

	/* process soc control */
	soc_ctrl_event_control(l_dev->event);

	return NOTIFY_OK;
}

#ifdef CONFIG_SYSFS
static ssize_t soc_ctrl_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t soc_ctrl_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info soc_ctrl_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(soc_ctrl, 0640, SOC_CTRL_SYSFS_CONTROL, control),
	power_sysfs_attr_rw(soc_ctrl, 0640, SOC_CTRL_SYSFS_STRATEGY, strategy),
};

#define SOC_CTRL_SYSFS_ATTRS_SIZE  ARRAY_SIZE(soc_ctrl_sysfs_field_tbl)

static struct attribute *soc_ctrl_sysfs_attrs[SOC_CTRL_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group soc_ctrl_sysfs_attr_group = {
	.attrs = soc_ctrl_sysfs_attrs,
};

static void soc_ctrl_sysfs_init_attrs(void)
{
	int s;

	for (s = 0; s < SOC_CTRL_SYSFS_ATTRS_SIZE; s++)
		soc_ctrl_sysfs_attrs[s] = &soc_ctrl_sysfs_field_tbl[s].attr.attr;

	soc_ctrl_sysfs_attrs[s] = NULL;
}

static struct power_sysfs_attr_info *soc_ctrl_sysfs_field_lookup(
	const char *name)
{
	int s;

	for (s = 0; s < SOC_CTRL_SYSFS_ATTRS_SIZE; s++) {
		if (!strncmp(name, soc_ctrl_sysfs_field_tbl[s].attr.attr.name,
			strlen(name)))
			break;
	}

	if (s >= SOC_CTRL_SYSFS_ATTRS_SIZE)
		return NULL;

	return &soc_ctrl_sysfs_field_tbl[s];
}

static ssize_t soc_ctrl_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct soc_ctrl_dev *l_dev = soc_ctrl_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = soc_ctrl_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case SOC_CTRL_SYSFS_CONTROL:
		return scnprintf(buf, PAGE_SIZE,
			"user=%s, enable=%d, min_soc=%d, max_soc=%d\n",
			soc_ctrl_get_op_user_name(l_dev->user),
			l_dev->enable,
			l_dev->min_soc, l_dev->max_soc);
	case SOC_CTRL_SYSFS_STRATEGY:
		return scnprintf(buf, PAGE_SIZE, "%d\n", l_dev->strategy);
	default:
		return 0;
	}
}

static int soc_ctrl_sysfs_store_control(struct soc_ctrl_dev *l_dev,
	const char *buf)
{
	char user_name[SOC_CTRL_RW_BUF_SIZE] = { 0 };
	int user;
	int enable;
	unsigned int min_soc;
	unsigned int max_soc;

	/* 4: the fields of "user enable min_soc max_soc" */
	if (sscanf(buf, "%s %d %d %d",
		user_name, &enable, &min_soc, &max_soc) != 4) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	user = soc_ctrl_get_op_user(user_name);
	if (user < 0)
		return -EINVAL;

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((enable < 0) || (enable > 1)) {
		hwlog_err("invalid enable=%d\n", enable);
		return -EINVAL;
	}

	/* soc must be (0, 100) and (max_soc - min_soc) >= 5 */
	if ((min_soc < 0 || min_soc > 100) ||
		(max_soc < 0 || max_soc > 100) ||
		(min_soc + 5 > max_soc)) {
		hwlog_err("invalid min_soc=%d or max_soc=%d\n",
			min_soc, max_soc);
		return -EINVAL;
	}

	hwlog_info("set: user=%s, enable=%d, min_soc=%d, max_soc=%d\n",
		soc_ctrl_get_op_user_name(user), enable, min_soc, max_soc);

	l_dev->user = user;
	l_dev->min_soc = min_soc;
	l_dev->max_soc = max_soc;

	/* ignore the same control event */
	if (l_dev->enable == enable) {
		hwlog_info("ignore the same control event\n");
		return 0;
	}
	l_dev->enable = enable;

	/* process soc control */
	soc_ctrl_event_control(l_dev->event);
	return 0;
}

static int soc_ctrl_sysfs_store_strategy(struct soc_ctrl_dev *l_dev,
	const char *buf)
{
	char user_name[SOC_CTRL_RW_BUF_SIZE] = { 0 };
	int user;
	int val;

	/* 2: the fields of "user type" */
	if (sscanf(buf, "%s %d", user_name, &val) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	user = soc_ctrl_get_op_user(user_name);
	if (user < 0)
		return -EINVAL;

	if ((val < STRATEGY_TYPE_BEGIN) || (val > STRATEGY_TYPE_END)) {
		hwlog_err("invalid strategy=%d\n", val);
		return -EINVAL;
	}

	hwlog_info("set: user=%s, strategy=%d\n",
		soc_ctrl_get_op_user_name(user), val);

	/* ignore the same strategy event */
	if (l_dev->strategy == val) {
		hwlog_info("ignore the same strategy event\n");
		return 0;
	}
	l_dev->strategy = val;

	return 0;
}

static ssize_t soc_ctrl_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct soc_ctrl_dev *l_dev = soc_ctrl_get_dev();
	int ret;

	if (!l_dev)
		return -EINVAL;

	info = soc_ctrl_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (SOC_CTRL_RW_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	switch (info->name) {
	case SOC_CTRL_SYSFS_CONTROL:
		ret = soc_ctrl_sysfs_store_control(l_dev, buf);
		if (ret)
			return -EINVAL;
		break;
	case SOC_CTRL_SYSFS_STRATEGY:
		ret = soc_ctrl_sysfs_store_strategy(l_dev, buf);
		if (ret)
			return -EINVAL;
		break;
	default:
		break;
	}

	return count;
}

static struct device *soc_ctrl_sysfs_create_group(void)
{
	soc_ctrl_sysfs_init_attrs();
	return power_sysfs_create_group("hw_power", "soc_control",
		&soc_ctrl_sysfs_attr_group);
}

static void soc_ctrl_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &soc_ctrl_sysfs_attr_group);
}
#else
static inline struct device *soc_ctrl_sysfs_create_group(void)
{
	return NULL;
}

static inline void soc_ctrl_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init soc_ctrl_init(void)
{
	int ret;
	struct soc_ctrl_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_soc_ctrl_dev = l_dev;
	INIT_DELAYED_WORK(&l_dev->work, soc_ctrl_event_work);
	l_dev->nb.notifier_call = soc_ctrl_event_call;
	ret = blocking_notifier_chain_register(&charger_event_notify_head,
		&l_dev->nb);
	if (ret) {
		hwlog_err("register charger_event notifier failed\n");
		goto fail_free_mem;
	}

	l_dev->dev = soc_ctrl_sysfs_create_group();

	return 0;

fail_free_mem:
	kfree(l_dev);
	g_soc_ctrl_dev = NULL;

	return ret;
}

static void __exit soc_ctrl_exit(void)
{
	struct soc_ctrl_dev *l_dev = g_soc_ctrl_dev;

	if (!l_dev)
		return;

	cancel_delayed_work(&l_dev->work);
	blocking_notifier_chain_unregister(&charger_event_notify_head,
		&l_dev->nb);
	soc_ctrl_sysfs_remove_group(l_dev->dev);
	kfree(l_dev);
	g_soc_ctrl_dev = NULL;
}

fs_initcall_sync(soc_ctrl_init);
module_exit(soc_ctrl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery capacity control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
