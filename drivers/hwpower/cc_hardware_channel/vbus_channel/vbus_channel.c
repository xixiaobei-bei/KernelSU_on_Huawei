/*
 * vbus_channel.c
 *
 * vbus channel driver
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

#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG vbus_ch
HWLOG_REGIST();

static struct vbus_ch_dev *g_vbus_ch_dev;

static const char * const g_vbus_ch_op_user_table[VBUS_CH_USER_END] = {
	[VBUS_CH_USER_WIRED_OTG] = "wired_otg",
	[VBUS_CH_USER_WR_TX] = "wireless_tx",
	[VBUS_CH_USER_DC] = "direct_charger",
	[VBUS_CH_USER_PD] = "pd",
	[VBUS_CH_USER_AUDIO] = "audio",
	[VBUS_CH_USER_POGOPIN] = "pogopin",
};

static const char * const g_vbus_ch_op_type_table[VBUS_CH_TYPE_END] = {
	[VBUS_CH_TYPE_CHARGER] = "charger",
	[VBUS_CH_TYPE_BOOST_GPIO] = "boost_gpio",
	[VBUS_CH_TYPE_POGOPIN_BOOST] = "pogopin_boost",
};

static int vbus_ch_check_op_user(unsigned int index)
{
	if ((index >= VBUS_CH_USER_BEGIN) && (index < VBUS_CH_USER_END))
		return 0;

	hwlog_err("invalid user=%u\n", index);
	return -EINVAL;
}

static int vbus_ch_check_op_type(unsigned int index)
{
	if ((index >= VBUS_CH_TYPE_BEGIN) && (index < VBUS_CH_TYPE_END))
		return 0;

	hwlog_err("invalid type=%u\n", index);
	return -EINVAL;
}

static int vbus_ch_get_op_type(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_vbus_ch_op_type_table); i++) {
		if (!strncmp(str, g_vbus_ch_op_type_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid type_str=%s\n", str);
	return -EINVAL;
}

static const char *vbus_ch_get_op_user_name(unsigned int index)
{
	if ((index >= VBUS_CH_USER_BEGIN) && (index < VBUS_CH_USER_END))
		return g_vbus_ch_op_user_table[index];

	return "invalid user";
}

static const char *vbus_ch_get_op_type_name(unsigned int index)
{
	if ((index >= VBUS_CH_TYPE_BEGIN) && (index < VBUS_CH_TYPE_END))
		return g_vbus_ch_op_type_table[index];

	return "invalid type";
}

static struct vbus_ch_dev *vbus_ch_get_dev(void)
{
	if (!g_vbus_ch_dev) {
		hwlog_err("g_vbus_ch_dev is null\n");
		return NULL;
	}

	return g_vbus_ch_dev;
}

static struct vbus_ch_ops *vbus_ch_get_ops(unsigned int user,
	unsigned int type)
{
	if (vbus_ch_check_op_user(user))
		return NULL;

	if (vbus_ch_check_op_type(type))
		return NULL;

	if (!g_vbus_ch_dev) {
		hwlog_err("g_vbus_ch_dev is null\n");
		return NULL;
	}

	if (!g_vbus_ch_dev->p_ops[type]) {
		hwlog_err("type=%u p_ops is null\n", type);
		return NULL;
	}

	return g_vbus_ch_dev->p_ops[type];
}

int vbus_ch_ops_register(struct vbus_ch_ops *ops)
{
	int type;

	if (!g_vbus_ch_dev || !ops || !ops->type_name) {
		hwlog_err("g_vbus_ch_dev or ops or type_name is null\n");
		return -EINVAL;
	}

	type = vbus_ch_get_op_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -EINVAL;
	}

	g_vbus_ch_dev->p_ops[type] = ops;
	g_vbus_ch_dev->total_ops++;
	g_vbus_ch_dev->support_type |= (1 << type);

	hwlog_info("total_ops=%u type=%d:%s ops register ok\n",
		g_vbus_ch_dev->total_ops, type, ops->type_name);

	return 0;
}

int vbus_ch_open(unsigned int user, unsigned int type, int flag)
{
	struct vbus_ch_ops *l_ops = vbus_ch_get_ops(user, type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->open) {
		hwlog_err("open is null\n");
		return -EINVAL;
	}

	hwlog_info("user=%s type=%s open\n",
		vbus_ch_get_op_user_name(user),
		vbus_ch_get_op_type_name(type));

	return l_ops->open(user, flag);
}

int vbus_ch_close(unsigned int user, unsigned int type, int flag, int force)
{
	struct vbus_ch_ops *l_ops = vbus_ch_get_ops(user, type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->close) {
		hwlog_err("close is null\n");
		return -EINVAL;
	}

	hwlog_info("user=%s type=%s close\n",
		vbus_ch_get_op_user_name(user),
		vbus_ch_get_op_type_name(type));

	return l_ops->close(user, flag, force);
}

/*lint -e580*/
int vbus_ch_get_state(unsigned int user, unsigned int type, int *state)
{
	struct vbus_ch_ops *l_ops = vbus_ch_get_ops(user, type);

	if (!l_ops || !state)
		return -EINVAL;

	if (!l_ops->get_state) {
		hwlog_err("get_state is null\n");
		return -EINVAL;
	}

	hwlog_info("user=%s type=%s get_state\n",
		vbus_ch_get_op_user_name(user),
		vbus_ch_get_op_type_name(type));

	return l_ops->get_state(user, state);
}
EXPORT_SYMBOL_GPL(vbus_ch_get_state);
/*lint +e580*/

int vbus_ch_get_mode(unsigned int user, unsigned int type, int *mode)
{
	struct vbus_ch_ops *l_ops = vbus_ch_get_ops(user, type);

	if (!l_ops || !mode)
		return -EINVAL;

	if (!l_ops->get_mode) {
		hwlog_err("get_mode is null\n");
		return -EINVAL;
	}

	return l_ops->get_mode(user, mode);
}

int vbus_ch_set_switch_mode(unsigned int user, unsigned int type, int mode)
{
	struct vbus_ch_ops *l_ops = vbus_ch_get_ops(user, type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->set_switch_mode) {
		hwlog_err("set_switch_mode is null\n");
		return -EINVAL;
	}

	hwlog_info("user=%s type=%s set_switch_mode\n",
		vbus_ch_get_op_user_name(user),
		vbus_ch_get_op_type_name(type));

	return l_ops->set_switch_mode(user, mode);
}

int vbus_ch_set_voltage(unsigned int user, unsigned int type, int volt)
{
	struct vbus_ch_ops *l_ops = vbus_ch_get_ops(user, type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->set_voltage) {
		hwlog_err("set_voltage is null\n");
		return -EINVAL;
	}

	hwlog_info("user=%s type=%s set_voltage\n",
		vbus_ch_get_op_user_name(user),
		vbus_ch_get_op_type_name(type));

	return l_ops->set_voltage(user, volt);
}

int vbus_ch_get_voltage(unsigned int user, unsigned int type, int *volt)
{
	struct vbus_ch_ops *l_ops = vbus_ch_get_ops(user, type);

	if (!l_ops || !volt)
		return -EINVAL;

	if (!l_ops->get_voltage) {
		hwlog_err("get_voltage is null\n");
		return -EINVAL;
	}

	hwlog_info("user=%s type=%s get_voltage\n",
		vbus_ch_get_op_user_name(user),
		vbus_ch_get_op_type_name(type));

	return l_ops->get_voltage(user, volt);
}

#ifdef CONFIG_SYSFS
static ssize_t vbus_ch_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_sysfs_attr_info vbus_ch_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(vbus_ch, 0440, VBUS_CH_SYSFS_SUPPORT_TYPE, support_type),
};

#define VBUS_CH_SYSFS_ATTRS_SIZE  ARRAY_SIZE(vbus_ch_sysfs_field_tbl)

static struct attribute *vbus_ch_sysfs_attrs[VBUS_CH_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group vbus_ch_sysfs_attr_group = {
	.attrs = vbus_ch_sysfs_attrs,
};

static ssize_t vbus_ch_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct vbus_ch_dev *l_dev = vbus_ch_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		vbus_ch_sysfs_field_tbl, VBUS_CH_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case VBUS_CH_SYSFS_SUPPORT_TYPE:
		return scnprintf(buf, VBUS_CH_RW_BUF_SIZE, "%x\n",
			l_dev->support_type);
	default:
		return 0;
	}
}

static struct device *vbus_ch_sysfs_create_group(void)
{
	power_sysfs_init_attrs(vbus_ch_sysfs_attrs,
		vbus_ch_sysfs_field_tbl, VBUS_CH_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "vbus_channel",
		&vbus_ch_sysfs_attr_group);
}

static void vbus_ch_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &vbus_ch_sysfs_attr_group);
}
#else
static inline struct device *vbus_ch_sysfs_create_group(void)
{
	return NULL;
}

static inline void vbus_ch_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init vbus_ch_init(void)
{
	struct vbus_ch_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_vbus_ch_dev = l_dev;
	l_dev->dev = vbus_ch_sysfs_create_group();
	return 0;
}

static void __exit vbus_ch_exit(void)
{
	struct vbus_ch_dev *l_dev = g_vbus_ch_dev;

	if (!l_dev)
		return;

	vbus_ch_sysfs_remove_group(l_dev->dev);
	kfree(l_dev);
	g_vbus_ch_dev = NULL;
}

subsys_initcall_sync(vbus_ch_init);
module_exit(vbus_ch_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("vbus channel module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
