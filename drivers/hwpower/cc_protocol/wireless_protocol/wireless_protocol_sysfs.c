/*
 * wireless_protocol_sysfs.c
 *
 * sysfs of wireless protocol driver
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

#include "wireless_protocol_sysfs.h"
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG wireless
HWLOG_REGIST();

static struct wireless_dev *g_wireless_dev;

#ifdef CONFIG_SYSFS
static ssize_t wireless_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_sysfs_attr_info wireless_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(wireless, 0440, WIRELESS_SYSFS_TX_FWVER, tx_fwver),
	power_sysfs_attr_ro(wireless, 0440, WIRELESS_SYSFS_TX_ID, tx_id),
	power_sysfs_attr_ro(wireless, 0440, WIRELESS_SYSFS_TX_TYPE, tx_type),
	power_sysfs_attr_ro(wireless, 0440, WIRELESS_SYSFS_TX_ADP_TYPE, tx_adp_type),
	power_sysfs_attr_ro(wireless, 0440, WIRELESS_SYSFS_TX_BD_INFO, tx_bd_info),
};

#define WIRELESS_SYSFS_ATTRS_SIZE  ARRAY_SIZE(wireless_sysfs_field_tbl)

static struct attribute *wireless_sysfs_attrs[WIRELESS_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group wireless_sysfs_attr_group = {
	.attrs = wireless_sysfs_attrs,
};

static ssize_t wireless_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wireless_protocol_dev *prot_dev = wireless_get_protocol_dev();

	if (!g_wireless_dev || !prot_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		wireless_sysfs_field_tbl, WIRELESS_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case WIRELESS_SYSFS_TX_FWVER:
		return scnprintf(buf, WIRELESS_RD_BUF_SIZE, "%s\n",
			prot_dev->info.tx_fwver);
	case WIRELESS_SYSFS_TX_ID:
		return scnprintf(buf, WIRELESS_RD_BUF_SIZE, "%x\n",
			prot_dev->info.tx_id);
	case WIRELESS_SYSFS_TX_TYPE:
		return scnprintf(buf, WIRELESS_RD_BUF_SIZE, "%d\n",
			prot_dev->info.tx_type);
	case WIRELESS_SYSFS_TX_ADP_TYPE:
		return scnprintf(buf, WIRELESS_RD_BUF_SIZE, "%d\n",
			prot_dev->info.tx_adp_type);
	case WIRELESS_SYSFS_TX_BD_INFO:
		return scnprintf(buf, WIRELESS_RD_BUF_SIZE, "%s",
			prot_dev->info.tx_bd_info);
	default:
		return 0;
	}
}

static struct device *wireless_sysfs_create_group(void)
{
	power_sysfs_init_attrs(wireless_sysfs_attrs,
		wireless_sysfs_field_tbl, WIRELESS_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "wireless",
		&wireless_sysfs_attr_group);
}

static void wireless_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &wireless_sysfs_attr_group);
}
#else
static inline struct device *wireless_sysfs_create_group(void)
{
	return NULL;
}

static inline void wireless_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init wireless_init(void)
{
	struct wireless_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_wireless_dev = l_dev;
	l_dev->dev = wireless_sysfs_create_group();
	return 0;
}

static void __exit wireless_exit(void)
{
	if (!g_wireless_dev)
		return;

	wireless_sysfs_remove_group(g_wireless_dev->dev);
	kfree(g_wireless_dev);
	g_wireless_dev = NULL;
}

subsys_initcall_sync(wireless_init);
module_exit(wireless_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
