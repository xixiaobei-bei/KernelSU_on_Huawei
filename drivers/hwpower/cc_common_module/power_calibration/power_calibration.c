/*
 * power_calibration.c
 *
 * calibration for power module
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

#include <chipset_common/hwpower/common_module/power_calibration.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_cali
HWLOG_REGIST();

static LIST_HEAD(g_power_cali_list);
static unsigned int g_power_cali_total;
static struct power_cali_dev *g_power_cali_dev;

static const char * const g_power_cali_user_table[] = {
	[POWER_CALI_USER_ATCMD] = "atcmd",
	[POWER_CALI_USER_HIDL] = "hidl",
	[POWER_CALI_USER_SHELL] = "shell",
};

static int power_cali_get_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_power_cali_user_table); i++) {
		if (!strcmp(str, g_power_cali_user_table[i]))
			return i;
	}

	hwlog_err("invalid user_str=%s\n", str);
	return -1;
}

static struct power_cali_dev *power_cali_get_dev(void)
{
	if (!g_power_cali_dev) {
		hwlog_err("g_power_cali_dev is null\n");
		return NULL;
	}

	return g_power_cali_dev;
}

int power_cali_ops_register(struct power_cali_ops *ops)
{
	if (!ops || !ops->name) {
		hwlog_err("ops or name is null\n");
		return -EINVAL;
	}

	if (g_power_cali_total >= POWER_CALI_MAX_OPS) {
		hwlog_err("too much ops register\n");
		return -EINVAL;
	}

	list_add_tail(&ops->node, &g_power_cali_list);
	g_power_cali_total++;

	hwlog_info("total_ops=%d name=%s ops register ok\n",
		g_power_cali_total, ops->name);
	return 0;
}

static void power_cali_release_list(void)
{
	struct power_cali_ops *temp = NULL;
	struct power_cali_ops *next = NULL;

	list_for_each_entry_safe(temp, next, &g_power_cali_list, node) {
		list_del(&temp->node);
		temp = NULL;
	}
}

static struct power_cali_ops *power_cali_find_ops(const char *name)
{
	unsigned int count = 0;
	struct power_cali_ops *ops = NULL;

	list_for_each_entry(ops, &g_power_cali_list, node) {
		if (count++ >= g_power_cali_total)
			break;

		if (!ops || strcmp(name, ops->name))
			continue;

		return ops;
	}

	return NULL;
}

static int power_cali_get_data(const char *name,
	unsigned int offset, int *data)
{
	struct power_cali_ops *ops = power_cali_find_ops(name);
	int ret;

	if (!ops || !ops->get_data || !data)
		return -EINVAL;

	ret = ops->get_data(offset, data, ops->dev_data);
	hwlog_info("get_data %s,%u,%d ret=%d\n", ops->name, offset, *data, ret);
	return ret;
}

static int power_cali_set_data(const char *name,
	unsigned int offset, int data)
{
	struct power_cali_ops *ops = power_cali_find_ops(name);
	int ret;

	if (!ops || !ops->set_data)
		return -EINVAL;

	ret = ops->set_data(offset, data, ops->dev_data);
	hwlog_info("set_data %s,%u,%d ret=%d\n", ops->name, offset, data, ret);
	return ret;
}

static int power_cali_get_mode(const char *name, int *data)
{
	struct power_cali_ops *ops = power_cali_find_ops(name);
	int ret;

	if (!ops || !ops->get_mode || !data)
		return -EINVAL;

	ret = ops->get_mode(data, ops->dev_data);
	hwlog_info("get_mode %s,%d ret=%d\n", ops->name, *data, ret);
	return ret;
}

static int power_cali_set_mode(const char *name, int data)
{
	struct power_cali_ops *ops = power_cali_find_ops(name);
	int ret;

	if (!ops || !ops->set_mode)
		return -EINVAL;

	ret = ops->set_mode(data, ops->dev_data);
	hwlog_info("set_mode %s,%d ret=%d\n", ops->name, data, ret);
	return ret;
}

static int power_cali_save_data(const char *name)
{
	struct power_cali_ops *ops = power_cali_find_ops(name);
	int ret;

	if (!ops || !ops->save_data)
		return -EINVAL;

	ret = ops->save_data(ops->dev_data);
	hwlog_info("save_data %s, ret=%d\n", ops->name, ret);
	return ret;
}

static int power_cali_show_data(const char *name, char *buf, unsigned int size)
{
	struct power_cali_ops *ops = power_cali_find_ops(name);
	int ret;

	if (!ops || !ops->show_data)
		return -EINVAL;

	ret = ops->show_data(buf, size, ops->dev_data);
	hwlog_info("show_data %s, ret=%d\n", ops->name, ret);
	return ret;
}

int power_cali_common_get_data(const char *name,
	unsigned int offset, int *data)
{
	if (!name || !data)
		return -EINVAL;

	return power_cali_get_data(name, offset, data);
}

#ifdef CONFIG_SYSFS
static ssize_t power_cali_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t power_cali_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info power_cali_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(power_cali, 0660, POWER_CALI_SYSFS_DC_CUR_A, dc_cur_a),
	power_sysfs_attr_rw(power_cali, 0660, POWER_CALI_SYSFS_DC_CUR_B, dc_cur_b),
	power_sysfs_attr_rw(power_cali, 0660, POWER_CALI_SYSFS_AUX_SC_CUR_A, aux_sc_cur_a),
	power_sysfs_attr_rw(power_cali, 0660, POWER_CALI_SYSFS_AUX_SC_CUR_B, aux_sc_cur_b),
	power_sysfs_attr_wo(power_cali, 0220, POWER_CALI_SYSFS_DC_SAVE, dc_save),
	power_sysfs_attr_ro(power_cali, 0440, POWER_CALI_SYSFS_DC_SHOW, dc_show),
	power_sysfs_attr_rw(power_cali, 0660, POWER_CALI_SYSFS_COUL_CUR_A, fg_cur_a),
	power_sysfs_attr_rw(power_cali, 0660, POWER_CALI_SYSFS_COUL_CUR_B, fg_cur_b),
	power_sysfs_attr_rw(power_cali, 0660, POWER_CALI_SYSFS_COUL_VOL_A, fg_vol_a),
	power_sysfs_attr_rw(power_cali, 0660, POWER_CALI_SYSFS_COUL_VOL_B, fg_vol_b),
	power_sysfs_attr_ro(power_cali, 0440, POWER_CALI_SYSFS_COUL_CUR, fg_cur),
	power_sysfs_attr_ro(power_cali, 0440, POWER_CALI_SYSFS_COUL_VOL, fg_vol),
	power_sysfs_attr_rw(power_cali, 0660, POWER_CALI_SYSFS_COUL_MODE, fg_mode),
	power_sysfs_attr_wo(power_cali, 0220, POWER_CALI_SYSFS_COUL_SAVE, fg_save),
};

#define POWER_CALI_SYSFS_ATTRS_SIZE  ARRAY_SIZE(power_cali_sysfs_field_tbl)

static struct attribute *power_cali_sysfs_attrs[POWER_CALI_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group power_cali_sysfs_attr_group = {
	.attrs = power_cali_sysfs_attrs,
};

static ssize_t power_cali_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_cali_dev *l_dev = power_cali_get_dev();
	int value = -1;
	char temp_buf[POWER_CALI_WR_BUF_SIZE] = { 0 };
	int offset;

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_cali_sysfs_field_tbl, POWER_CALI_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case POWER_CALI_SYSFS_DC_CUR_A:
		offset = POWER_CALI_SYSFS_DC_CUR_A - POWER_CALI_SYSFS_DC_CUR_A;
		power_cali_get_data("dc", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_DC_CUR_B:
		offset = POWER_CALI_SYSFS_DC_CUR_B - POWER_CALI_SYSFS_DC_CUR_A;
		power_cali_get_data("dc", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_AUX_SC_CUR_A:
		offset = POWER_CALI_SYSFS_AUX_SC_CUR_A - POWER_CALI_SYSFS_DC_CUR_A;
		power_cali_get_data("dc", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_AUX_SC_CUR_B:
		offset = POWER_CALI_SYSFS_AUX_SC_CUR_B - POWER_CALI_SYSFS_DC_CUR_A;
		power_cali_get_data("dc", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_DC_SHOW:
		if (power_cali_show_data("dc", temp_buf, POWER_CALI_WR_BUF_SIZE - 1))
			return -EINVAL;
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%s\n", temp_buf);
	case POWER_CALI_SYSFS_COUL_CUR_A:
		offset = POWER_CALI_SYSFS_COUL_CUR_A - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_get_data("coul", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_COUL_CUR_B:
		offset = POWER_CALI_SYSFS_COUL_CUR_B - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_get_data("coul", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_COUL_VOL_A:
		offset = POWER_CALI_SYSFS_COUL_VOL_A - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_get_data("coul", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_COUL_VOL_B:
		offset = POWER_CALI_SYSFS_COUL_VOL_B - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_get_data("coul", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_COUL_CUR:
		offset = POWER_CALI_SYSFS_COUL_CUR - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_get_data("coul", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_COUL_VOL:
		offset = POWER_CALI_SYSFS_COUL_VOL - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_get_data("coul", offset, &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	case POWER_CALI_SYSFS_COUL_MODE:
		power_cali_get_mode("coul", &value);
		return scnprintf(buf, POWER_CALI_WR_BUF_SIZE, "%d\n", value);
	default:
		return 0;
	}
}

static ssize_t power_cali_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_cali_dev *l_dev = power_cali_get_dev();
	char user[POWER_CALI_WR_BUF_SIZE] = { 0 };
	int value;
	int offset;

	if (!power_cmdline_is_factory_mode()) {
		hwlog_info("only factory version can set\n");
		return -EINVAL;
	}

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_cali_sysfs_field_tbl, POWER_CALI_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	/* 2: the fields of "user value" */
	if (sscanf(buf, "%s %d", user, &value) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (power_cali_get_user(user) < 0)
		return -EINVAL;

	hwlog_info("set: user=%s, value=%d\n", user, value);

	switch (info->name) {
	case POWER_CALI_SYSFS_DC_CUR_A:
		offset = POWER_CALI_SYSFS_DC_CUR_A - POWER_CALI_SYSFS_DC_CUR_A;
		power_cali_set_data("dc", offset, value);
		break;
	case POWER_CALI_SYSFS_DC_CUR_B:
		offset = POWER_CALI_SYSFS_DC_CUR_B - POWER_CALI_SYSFS_DC_CUR_A;
		power_cali_set_data("dc", offset, value);
		break;
	case POWER_CALI_SYSFS_AUX_SC_CUR_A:
		offset = POWER_CALI_SYSFS_AUX_SC_CUR_A - POWER_CALI_SYSFS_DC_CUR_A;
		power_cali_set_data("dc", offset, value);
		break;
	case POWER_CALI_SYSFS_AUX_SC_CUR_B:
		offset = POWER_CALI_SYSFS_AUX_SC_CUR_B - POWER_CALI_SYSFS_DC_CUR_A;
		power_cali_set_data("dc", offset, value);
		break;
	case POWER_CALI_SYSFS_DC_SAVE:
		if (power_cali_save_data("dc") < 0)
			return -EINVAL;
		break;
	case POWER_CALI_SYSFS_COUL_CUR_A:
		offset = POWER_CALI_SYSFS_COUL_CUR_A - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_set_data("coul", offset, value);
		break;
	case POWER_CALI_SYSFS_COUL_CUR_B:
		offset = POWER_CALI_SYSFS_COUL_CUR_B - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_set_data("coul", offset, value);
		break;
	case POWER_CALI_SYSFS_COUL_VOL_A:
		offset = POWER_CALI_SYSFS_COUL_VOL_A - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_set_data("coul", offset, value);
		break;
	case POWER_CALI_SYSFS_COUL_VOL_B:
		offset = POWER_CALI_SYSFS_COUL_VOL_B - POWER_CALI_SYSFS_COUL_CUR_A;
		power_cali_set_data("coul", offset, value);
		break;
	case POWER_CALI_SYSFS_COUL_MODE:
		power_cali_set_mode("coul", value);
		break;
	case POWER_CALI_SYSFS_COUL_SAVE:
		if (power_cali_save_data("coul") < 0)
			return -EINVAL;
		break;
	default:
		break;
	}

	return count;
}

static struct device *power_cali_sysfs_create_group(void)
{
	power_sysfs_init_attrs(power_cali_sysfs_attrs,
		power_cali_sysfs_field_tbl, POWER_CALI_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "power_cali",
		&power_cali_sysfs_attr_group);
}

static void power_cali_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_cali_sysfs_attr_group);
}
#else
static inline struct device *power_cali_sysfs_create_group(void)
{
	return NULL;
}

static inline void power_cali_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init power_cali_init(void)
{
	struct power_cali_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_power_cali_dev = l_dev;
	l_dev->dev = power_cali_sysfs_create_group();
	return 0;
}

static void __exit power_cali_exit(void)
{
	struct power_cali_dev *l_dev = g_power_cali_dev;

	if (!l_dev)
		return;

	power_cali_release_list();
	power_cali_sysfs_remove_group(l_dev->dev);
	kfree(l_dev);
	g_power_cali_dev = NULL;
}

subsys_initcall_sync(power_cali_init);
module_exit(power_cali_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery calibration module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
