/*
 * wireless_test_hw.c
 *
 * wireless test hardware
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_test.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/wireless_charge/wireless_test_hw.h>

#define HWLOG_TAG wireless_tst_hardware
HWLOG_REGIST();

static struct wireless_hw_dev_info *g_wireless_hw_di;
static struct wireless_hw_test_ops g_hw_chk_ops;

int wireless_hw_test_ops_register(const struct wireless_hw_test_ops *ops)
{
	if (!ops)
		return -EPERM;

	if (ops->chk_pwr_good_gpio) {
		hwlog_info("register chk_pwr_good_gpio ops\n");
		g_hw_chk_ops.chk_pwr_good_gpio = ops->chk_pwr_good_gpio;
	}

	if (ops->chk_alignment_circuit) {
		hwlog_info("register chk_alignment_circuit ops\n");
		g_hw_chk_ops.chk_alignment_circuit = ops->chk_alignment_circuit;
	}

	if (ops->chk_reverse_cp) {
		hwlog_info("register chk_reverse_cp ops\n");
		g_hw_chk_ops.chk_reverse_cp = ops->chk_reverse_cp;
	}

	if (ops->chk_reverse_cp_prepare) {
		hwlog_info("register chk_cp_prepare ops\n");
		g_hw_chk_ops.chk_reverse_cp_prepare = ops->chk_reverse_cp_prepare;
	}

	return 0;
}

static void wireless_hw_test_pwr_good_gpio(void)
{
	struct wireless_hw_dev_info *di = g_wireless_hw_di;

	if (!di || !di->ops || !di->ops->chk_pwr_good_gpio)
		return;

	if (di->hw_test_module & WIRELESS_HW_TEST_PWR_GOOD_GPIO &&
		!di->ops->chk_pwr_good_gpio())
		di->hw_test_result |= WIRELESS_HW_TEST_PWR_GOOD_GPIO;
}

static void wireless_hw_test_alignment_circuit(void)
{
	struct wireless_hw_dev_info *di = g_wireless_hw_di;

	if (!di || !di->ops || !di->ops->chk_alignment_circuit)
		return;

	if (di->hw_test_module & WIRELESS_HW_TEST_ALIGN_CIRCUIT &&
		!di->ops->chk_alignment_circuit())
		di->hw_test_result |= WIRELESS_HW_TEST_ALIGN_CIRCUIT;
}

static void wireless_hw_test_reverse_cp(void)
{
	struct wireless_hw_dev_info *di = g_wireless_hw_di;

	if (!di || !di->ops || !di->ops->chk_reverse_cp)
		return;

	if ((di->hw_test_module & WIRELESS_HW_TEST_REVERSE_CP) &&
		!di->ops->chk_reverse_cp())
		di->hw_test_result |= WIRELESS_HW_TEST_REVERSE_CP;
}

int wireless_hw_test_reverse_cp_prepare(int type)
{
	struct wireless_hw_dev_info *di = g_wireless_hw_di;

	if (!di || !di->ops || !di->ops->chk_reverse_cp_prepare)
		return -EINVAL;

	return di->ops->chk_reverse_cp_prepare(type);
}

static void wireless_hw_test_chk_work(struct work_struct *work)
{
	struct wireless_hw_dev_info *di = g_wireless_hw_di;

	if (!di)
		return;

	di->test_busy = true;
	hwlog_info("test begin, hw_test_module=0x%x\n", di->hw_test_module);

	wireless_hw_test_pwr_good_gpio();
	wireless_hw_test_alignment_circuit();
	wireless_hw_test_reverse_cp();

	di->test_busy = false;
	hwlog_info("test end, hw_test_result=0x%x\n", di->hw_test_result);
}

static void wireless_hw_test_start(u16 expected_module)
{
	struct wireless_hw_dev_info *di = g_wireless_hw_di;

	if (!di)
		return;

	if (di->test_busy) {
		hwlog_err("test busy\n");
		return;
	}

	hwlog_info("[test start] expected_hw_module=0x%x\n", expected_module);
	di->hw_test_module = expected_module & di->hw_support_module;
	di->hw_test_result = expected_module & (~di->hw_test_module);
	schedule_work(&di->chk_hw_work);
}

#ifdef CONFIG_SYSFS
static ssize_t wireless_hw_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t wireless_hw_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info wireless_hw_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(wireless_hw, 0640, WIRELESS_HW_TEST, hw_tst),
	power_sysfs_attr_rw(wireless_hw, 0640, WIRELESS_COIL_TEST, coil_tst),
};

#define WIRELESS_HW_SYSFS_ATTRS_SIZE  ARRAY_SIZE(wireless_hw_sysfs_field_tbl)

static struct attribute *wireless_hw_sysfs_attrs[WIRELESS_HW_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group wireless_hw_sysfs_attr_group = {
	.name = "wireless_hw",
	.attrs = wireless_hw_sysfs_attrs,
};

static ssize_t wireless_hw_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wireless_hw_dev_info *di = g_wireless_hw_di;

	info = power_sysfs_lookup_attr(attr->attr.name,
		wireless_hw_sysfs_field_tbl, WIRELESS_HW_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WIRELESS_HW_TEST:
		if (di->test_busy)
			return snprintf(buf, PAGE_SIZE, "test running\n");
		return snprintf(buf, PAGE_SIZE, "%u\n", di->hw_test_result);
	case WIRELESS_COIL_TEST:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			wirless_coil_test_result());
	default:
		break;
	}

	return 0;
}

static ssize_t wireless_hw_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		wireless_hw_sysfs_field_tbl, WIRELESS_HW_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case WIRELESS_HW_TEST:
		/* 255: maximum of hw_test_module */
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > 255)) {
			hwlog_err("sysfs_store: input invalid\n");
			return -EINVAL;
		}
		wireless_hw_test_start((u16)val);
		break;
	case WIRELESS_COIL_TEST:
		wireless_coil_test_start();
		break;
	default:
		break;
	}
	return count;
}

static int wireless_hw_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(wireless_hw_sysfs_attrs,
		wireless_hw_sysfs_field_tbl, WIRELESS_HW_SYSFS_ATTRS_SIZE);
	return sysfs_create_group(&dev->kobj, &wireless_hw_sysfs_attr_group);
}

static void wireless_hw_sysfs_remove_group(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &wireless_hw_sysfs_attr_group);
}
#else
static int wireless_hw_sysfs_create_group(struct device *dev)
{
	return 0;
}

static void wireless_hw_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int wireless_hw_test_parse_dts(struct wireless_hw_dev_info *di)
{
	u32 support_module = 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), di->np,
		"wl_hw_module", &support_module, 0);
	di->hw_support_module = (u16)support_module;

	return 0;
}

static void wireless_hw_test_init(struct device *dev)
{
	int ret;
	struct wireless_hw_dev_info *di = NULL;

	if (!dev || !dev->of_node) {
		hwlog_err("init: para error\n");
		return;
	}

	di = devm_kzalloc(dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	g_wireless_hw_di = di;
	di->ops = &g_hw_chk_ops;
	di->np = dev->of_node;
	INIT_WORK(&di->chk_hw_work, wireless_hw_test_chk_work);

	ret = wireless_hw_test_parse_dts(di);
	if (ret) {
		hwlog_err("init: parse dts failed\n");
		goto free_mem;
	}

	ret = wireless_hw_sysfs_create_group(dev);
	if (ret) {
		hwlog_err("init: create group failed\n");
		goto free_mem;
	}

	hwlog_info("init succ\n");
	return;

free_mem:
	devm_kfree(dev, di);
	g_wireless_hw_di = NULL;
}

static void wireless_hw_test_exit(struct device *dev)
{
	if (g_wireless_hw_di && dev) {
		wireless_hw_sysfs_remove_group(dev);
		devm_kfree(dev, g_wireless_hw_di);
		g_wireless_hw_di = NULL;
	}
}

static struct power_test_ops g_wireless_hw_test_ops = {
	.name = "wl_hw_test",
	.init = wireless_hw_test_init,
	.exit = wireless_hw_test_exit,
};

static int __init wireless_hw_test_module_init(void)
{
	return power_test_ops_register(&g_wireless_hw_test_ops);
}

module_init(wireless_hw_test_module_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless hardware test module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
