/*
 * power_test.c
 *
 * power test module
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
#include <linux/platform_device.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_test.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_test
HWLOG_REGIST();

static LIST_HEAD(power_test_list);
static unsigned int g_power_test_total;

int power_test_ops_register(struct power_test_ops *ops)
{
	if (!ops || !ops->name) {
		hwlog_err("para error\n");
		return -EINVAL;
	}

	if (g_power_test_total >= POWER_TEST_MAX_OPS) {
		hwlog_err("too much ops register\n");
		return -ENOSPC;
	}

	list_add_tail(&ops->ptst_node, &power_test_list);
	g_power_test_total++;

	hwlog_info("%s registered, t_ops=%d\n", ops->name, g_power_test_total);
	return 0;
}

static void power_test_list_release(void)
{
	struct power_test_ops *temp = NULL;
	struct power_test_ops *next = NULL;

	list_for_each_entry_safe(temp, next, &power_test_list, ptst_node) {
		list_del(&temp->ptst_node);
		temp = NULL;
	}
}

static void power_test_parse_dts(struct power_test_dev *di)
{
	unsigned int count = 0;
	struct power_test_ops *ops = NULL;

	list_for_each_entry(ops, &power_test_list, ptst_node) {
		if (count++ >= g_power_test_total)
			break;

		if (!ops || !ops->name)
			continue;

		(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), di->np,
			ops->name, &ops->enable, 0);
	}
}

static void power_test_init_item(struct power_test_dev *di)
{
	unsigned int count = 0;
	struct power_test_ops *ops = NULL;

	list_for_each_entry(ops, &power_test_list, ptst_node) {
		if (count++ >= g_power_test_total)
			break;

		if (!ops || !ops->enable || !ops->init)
			continue;

		ops->init(di->dev);
	}
}

static void power_test_exit_item(struct power_test_dev *di)
{
	unsigned int count = 0;
	struct power_test_ops *ops = NULL;

	list_for_each_entry(ops, &power_test_list, ptst_node) {
		if (count++ >= g_power_test_total)
			break;

		if (!ops || !ops->enable || !ops->exit)
			continue;

		ops->exit(di->dev);
	}
}

static int power_test_probe(struct platform_device *pdev)
{
	struct power_test_dev *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	di->np = pdev->dev.of_node;
	platform_set_drvdata(pdev, di);
	power_test_parse_dts(di);
	power_test_init_item(di);
	return 0;
}

static int power_test_remove(struct platform_device *pdev)
{
	struct power_test_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_test_exit_item(di);
	power_test_list_release();
	platform_set_drvdata(pdev, NULL);
	devm_kfree(di->dev, di);
	return 0;
}

static const struct of_device_id power_test_match_table[] = {
	{
		.compatible = "huawei,power_test",
		.data = NULL,
	},
	{},
};

static struct platform_driver power_test_driver = {
	.probe = power_test_probe,
	.remove = power_test_remove,
	.driver = {
		.name = "huawei,power_test",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(power_test_match_table),
	},
};

static int __init power_test_init(void)
{
	return platform_driver_register(&power_test_driver);
}

static void __exit power_test_exit(void)
{
	platform_driver_unregister(&power_test_driver);
}

late_initcall_sync(power_test_init);
module_exit(power_test_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power test module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
