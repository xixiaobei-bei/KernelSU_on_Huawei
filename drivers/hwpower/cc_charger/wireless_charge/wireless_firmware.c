/*
 * wireless_firmware.c
 *
 * wireless firmware driver
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>

#define HWLOG_TAG wireless_firmware
HWLOG_REGIST();

static LIST_HEAD(wireless_fw_list);
static int g_total_fw;
static struct wireless_fw_dev *g_wireless_fw_di;

int wireless_fw_ops_register(struct wireless_fw_ops *ops)
{
	if (!ops || !ops->ic_name) {
		hwlog_err("ops_register: para error\n");
		return -EINVAL;
	}

	if (g_total_fw >= WLTRX_IC_MAX) {
		hwlog_err("ops_register: too much ic registered\n");
		return -ENOSPC;
	}

	list_add_tail(&ops->fw_node, &wireless_fw_list);
	++g_total_fw;

	hwlog_info("[%s] registered, t_ops=%d\n", ops->ic_name, g_total_fw);
	return 0;
}

static int wireless_get_fw_status(unsigned int *status)
{
	int ret;
	struct wireless_fw_ops *ops = NULL;

	list_for_each_entry(ops, &wireless_fw_list, fw_node) {
		if (!ops || !ops->get_fw_status)
			return -EPERM;
		ret = ops->get_fw_status(status, ops->dev_data);
		if (ret)
			return ret;
	}

	return 0;
}

static int wireless_program_fw(unsigned int prog_type)
{
	int ret;
	struct wireless_fw_ops *ops = NULL;

	list_for_each_entry(ops, &wireless_fw_list, fw_node) {
		if (!ops || !ops->program_fw)
			return -EPERM;
		ret = ops->program_fw(prog_type, ops->dev_data);
		if (ret)
			return ret;
	}

	return 0;
}

static void wireless_fw_program_work(struct work_struct *work)
{
	int ret;
	static bool first_in = true;
	unsigned int fw_status = WIRELESS_FW_NON_PROGRAMED;
	struct wireless_fw_dev *di =
		container_of(work, struct wireless_fw_dev, program_work);

	if (!di)
		return;

	hwlog_info("program_fw begin\n");

	ret = wireless_get_fw_status(&fw_status);
	if (!ret && (fw_status == WIRELESS_FW_PROGRAMED)) {
		di->program_fw_flag = false;
		return;
	}

	if (first_in) {
		first_in = false;
		(void)wireless_program_fw(WIRELESS_PROGRAM_FW);
	} else {
		(void)wireless_program_fw(WIRELESS_RECOVER_FW);
	}

	di->program_fw_flag = false;
	hwlog_info("program_fw end\n");
}

#ifdef CONFIG_SYSFS
static int wireless_check_fw(void)
{
	int ret;
	struct wireless_fw_ops *ops = NULL;

	list_for_each_entry(ops, &wireless_fw_list, fw_node) {
		if (!ops || !ops->check_fw)
			return -EPERM;
		ret = ops->check_fw(ops->dev_data);
		if (ret)
			return ret;
	}

	return 0;
}

static void wireless_fw_sysfs_program_fw(struct wireless_fw_dev *di)
{
	if (!power_cmdline_is_factory_mode())
		return;

	if (g_total_fw != di->ic_nums) {
		hwlog_err("ic_nums=%d expected=%d\n", g_total_fw, di->ic_nums);
		return;
	}

	if (!di->program_fw_flag) {
		di->program_fw_flag = true;
		schedule_work(&di->program_work);
	}
}

static int wireless_fw_sysfs_check_fw(struct wireless_fw_dev *di, char *buf, int len)
{
	if (!power_cmdline_is_factory_mode())
		return 0;

	if (g_total_fw != di->ic_nums) {
		hwlog_err("ic_nums=%d expected=%d\n", g_total_fw, di->ic_nums);
		return -EINVAL;
	}

	if (di->program_fw_flag)
		return -EBUSY;

	return snprintf(buf, len, "%s\n", wireless_check_fw() ?
		"0: otp is bad" : "1: otp is good");
}

static int wireless_fw_sysfs_get_fw_status(struct wireless_fw_dev *di, char *buf, int len)
{
	int ret;
	unsigned int status = WIRELESS_FW_NON_PROGRAMED;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (g_total_fw != di->ic_nums) {
		hwlog_err("ic_nums=%d expected=%d\n", g_total_fw, di->ic_nums);
		return -EINVAL;
	}

	if (di->program_fw_flag)
		return -EBUSY;

	ret = wireless_get_fw_status(&status);
	if (ret)
		return ret;

	return snprintf(buf, len, "%u\n", status);
}

static ssize_t wireless_fw_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t wireless_fw_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info wireless_fw_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(wireless_fw, 0644, WLFW_SYSFS_PROGRAM_FW, program_fw),
	power_sysfs_attr_ro(wireless_fw, 0444, WLFW_SYSFS_CHK_FW, check_fw),
};

#define WIRELESS_FW_SYSFS_ATTRS_SIZE  ARRAY_SIZE(wireless_fw_sysfs_field_tbl)

static struct attribute *wireless_fw_sysfs_attrs[WIRELESS_FW_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group wireless_fw_sysfs_attr_group = {
	.attrs = wireless_fw_sysfs_attrs,
};

static ssize_t wireless_fw_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct wireless_fw_dev *di = g_wireless_fw_di;
	struct power_sysfs_attr_info *info = power_sysfs_lookup_attr(
		attr->attr.name, wireless_fw_sysfs_field_tbl,
		WIRELESS_FW_SYSFS_ATTRS_SIZE);

	if (!di || !info)
		return -EINVAL;

	switch (info->name) {
	case WLFW_SYSFS_PROGRAM_FW:
		return wireless_fw_sysfs_get_fw_status(di, buf, PAGE_SIZE);
	case WLFW_SYSFS_CHK_FW:
		return wireless_fw_sysfs_check_fw(di, buf, PAGE_SIZE);
	default:
		return 0;
	}
}

static ssize_t wireless_fw_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	struct wireless_fw_dev *di = g_wireless_fw_di;
	struct power_sysfs_attr_info *info = power_sysfs_lookup_attr(
		attr->attr.name, wireless_fw_sysfs_field_tbl,
		WIRELESS_FW_SYSFS_ATTRS_SIZE);

	if (!di || !info)
		return -EINVAL;

	switch (info->name) {
	case WLFW_SYSFS_PROGRAM_FW:
		/* 1: program otp */
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val != 1))
			return -EINVAL;

		wireless_fw_sysfs_program_fw(di);
		break;
	default:
		break;
	}
	return count;
}

static struct device *wireless_fw_sysfs_create_group(void)
{
	power_sysfs_init_attrs(wireless_fw_sysfs_attrs,
		wireless_fw_sysfs_field_tbl, WIRELESS_FW_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "wireless_fw",
		&wireless_fw_sysfs_attr_group);
}

static void wireless_fw_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &wireless_fw_sysfs_attr_group);
}
#else
static inline struct device *wireless_fw_sysfs_create_group(void)
{
	return NULL;
}

static inline void wireless_fw_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void wireless_fw_parse_dts(struct wireless_fw_dev *di)
{
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,wireless_fw", "ic_nums", (u32 *)&di->ic_nums,
		WIRELESS_FW_DFT_IC_NUMS);
	if (di->ic_nums <= 0)
		di->ic_nums = WIRELESS_FW_DFT_IC_NUMS;
}

static int __init wireless_fw_init(void)
{
	struct wireless_fw_dev *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	wireless_fw_parse_dts(di);
	g_wireless_fw_di = di;
	INIT_WORK(&di->program_work, wireless_fw_program_work);
	di->dev = wireless_fw_sysfs_create_group();
	return 0;
}

static void __exit wireless_fw_exit(void)
{
	if (!g_wireless_fw_di)
		return;

	wireless_fw_sysfs_remove_group(g_wireless_fw_di->dev);
	kfree(g_wireless_fw_di);
	g_wireless_fw_di = NULL;
}

subsys_initcall_sync(wireless_fw_init);
module_exit(wireless_fw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless firmware driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
