/*
 * buck_boost.c
 *
 * buck_boost driver
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

#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG buck_boost
HWLOG_REGIST();

static struct buck_boost_dev *g_buck_boost_di;

int buck_boost_ops_register(struct buck_boost_ops *ops)
{
	if (!g_buck_boost_di || !ops) {
		hwlog_err("buck boost ops register fail\n");
		return -EPERM;
	}

	g_buck_boost_di->ops = ops;
	hwlog_err("buck boost ops register ok\n");
	return 0;
}

static struct buck_boost_ops *buck_boost_get_ops(void)
{
	if (!g_buck_boost_di || !g_buck_boost_di->ops) {
		hwlog_err("g_buck_boost_di or ops is null\n");
		return NULL;
	}

	return g_buck_boost_di->ops;
}

int buck_boost_set_pwm_enable(unsigned int enable)
{
	struct buck_boost_ops *l_ops = buck_boost_get_ops();

	if (!l_ops)
		return -EPERM;

	if (!l_ops->set_pwm_enable) {
		hwlog_err("set_pwm_enable is null\n");
		return -EPERM;
	}

	return l_ops->set_pwm_enable(enable, l_ops->dev_data);
}

int buck_boost_set_vout(unsigned int vol)
{
	struct buck_boost_ops *l_ops = buck_boost_get_ops();

	if (!l_ops)
		return -EPERM;

	if (!l_ops->set_vout) {
		hwlog_err("set_vout is null\n");
		return -EPERM;
	}

	return l_ops->set_vout(vol, l_ops->dev_data);
}

bool buck_boost_pwr_good(void)
{
	struct buck_boost_ops *l_ops = buck_boost_get_ops();

	if (!l_ops)
		return false;

	if (!l_ops->pwr_good) {
		hwlog_err("pwr_good is null\n");
		return false;
	}

	return l_ops->pwr_good(l_ops->dev_data);
}

bool buck_boost_set_enable(unsigned int enable)
{
	struct buck_boost_ops *l_ops = buck_boost_get_ops();

	if (!l_ops)
		return false;

	if (!l_ops->set_enable) {
		hwlog_err("set_enable is null\n");
		return false;
	}

	return l_ops->set_enable(enable, l_ops->dev_data);
}

static int buck_boost_probe(struct platform_device *pdev)
{
	struct buck_boost_dev *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_buck_boost_di = di;
	di->dev = &pdev->dev;
	platform_set_drvdata(pdev, di);
	return 0;
}

static int buck_boost_remove(struct platform_device *pdev)
{
	struct buck_boost_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	g_buck_boost_di = NULL;
	return 0;
}

static const struct of_device_id buck_boost_match_table[] = {
	{
		.compatible = "huawei, buck_boost",
		.data = NULL,
	},
	{},
};

static struct platform_driver buck_boost_driver = {
	.probe = buck_boost_probe,
	.remove = buck_boost_remove,
	.driver = {
		.name = "huawei, buck_boost",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(buck_boost_match_table),
	},
};

static int __init buck_boost_init(void)
{
	return platform_driver_register(&buck_boost_driver);
}

static void __exit buck_boost_exit(void)
{
	platform_driver_unregister(&buck_boost_driver);
}

fs_initcall_sync(buck_boost_init);
module_exit(buck_boost_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("buck_boost module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
