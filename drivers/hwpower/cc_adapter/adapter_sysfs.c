/*
 * adapter_sysfs.c
 *
 * sysfs of adapter driver
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

#include "adapter_sysfs.h"
#include <chipset_common/hwpower/adapter/adapter_test.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG adapter_sysfs
HWLOG_REGIST();

static struct adapter_sysfs_dev *g_adapter_sysfs_dev;

#ifdef CONFIG_SYSFS
static ssize_t adapter_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t adapter_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info adapter_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_SUPPORT_MODE, support_mode),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_CHIP_ID, chip_id),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_VENDOR_ID, vendor_id),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_MODULE_ID, module_id),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_SERIAL_NO, serial_no),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_HWVER, hwver),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_FWVER, fwver),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_MIN_VOLT, min_volt),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_MAX_VOLT, max_volt),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_MIN_CUR, min_cur),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_MAX_CUR, max_cur),
	power_sysfs_attr_ro(adapter, 0440, ADAPTER_SYSFS_ADP_TYPE, adapter_type),
	power_sysfs_attr_rw(adapter, 0660, ADAPTER_SYSFS_TEST_RESULT, test_result),
};

#define ADAPTER_SYSFS_ATTRS_SIZE  ARRAY_SIZE(adapter_sysfs_field_tbl)

static struct attribute *adapter_sysfs_attrs[ADAPTER_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group adapter_sysfs_attr_group = {
	.attrs = adapter_sysfs_attrs,
};

static ssize_t adapter_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct adapter_protocol_dev *l_dev = adapter_get_protocol_dev();

	if (!g_adapter_sysfs_dev || !l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		adapter_sysfs_field_tbl, ADAPTER_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case ADAPTER_SYSFS_SUPPORT_MODE:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.support_mode);
	case ADAPTER_SYSFS_CHIP_ID:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.chip_id);
	case ADAPTER_SYSFS_VENDOR_ID:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.vendor_id);
	case ADAPTER_SYSFS_MODULE_ID:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.module_id);
	case ADAPTER_SYSFS_SERIAL_NO:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.serial_no);
	case ADAPTER_SYSFS_HWVER:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.hwver);
	case ADAPTER_SYSFS_FWVER:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.fwver);
	case ADAPTER_SYSFS_MIN_VOLT:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.min_volt);
	case ADAPTER_SYSFS_MAX_VOLT:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.max_volt);
	case ADAPTER_SYSFS_MIN_CUR:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.min_cur);
	case ADAPTER_SYSFS_MAX_CUR:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.max_cur);
	case ADAPTER_SYSFS_ADP_TYPE:
		return scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.adp_type);
	case ADAPTER_SYSFS_TEST_RESULT:
		return adapter_test_get_result(buf, ADAPTER_RD_BUF_SIZE);
	default:
		return 0;
	}
}

static ssize_t adapter_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	int val = 0;
	int ret;

	if (!g_adapter_sysfs_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		adapter_sysfs_field_tbl, ADAPTER_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case ADAPTER_SYSFS_TEST_RESULT:
		ret = kstrtoint(buf, POWER_BASE_DEC, &val);
		if (ret < 0)
			return -EINVAL;
		if ((val < AT_MIN_FLAG) || (val > AT_MAX_FLAG)) {
			hwlog_err("unable to parse input:%s\n", buf);
			return -EINVAL;
		}
		if (val == AT_START_FLAG)
			adapter_test_clear_result();
		break;
	default:
		break;
	}

	return count;
}

static struct device *adapter_sysfs_create_group(void)
{
	power_sysfs_init_attrs(adapter_sysfs_attrs,
		adapter_sysfs_field_tbl, ADAPTER_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "adapter",
		&adapter_sysfs_attr_group);
}

static void adapter_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &adapter_sysfs_attr_group);
}
#else
static inline struct device *adapter_sysfs_create_group(void)
{
	return NULL;
}

static inline void adapter_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init adapter_sysfs_init(void)
{
	struct adapter_sysfs_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_adapter_sysfs_dev = l_dev;
	l_dev->dev = adapter_sysfs_create_group();
	return 0;
}

static void __exit adapter_sysfs_exit(void)
{
	if (!g_adapter_sysfs_dev)
		return;

	adapter_sysfs_remove_group(g_adapter_sysfs_dev->dev);
	kfree(g_adapter_sysfs_dev);
	g_adapter_sysfs_dev = NULL;
}

subsys_initcall_sync(adapter_sysfs_init);
module_exit(adapter_sysfs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("adapter sysfs driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
