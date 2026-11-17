/*
 * dual_loadswitch.c
 *
 * dual loadswitch driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>

#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <dual_loadswitch.h>

#define HWLOG_TAG dual_loadswitch
HWLOG_REGIST();

static struct dual_loadswitch_info *dli;
static struct dc_ic_ops *g_main_ops;
static struct dc_ic_ops *g_aux_ops;

int loadswitch_main_ops_register(struct dc_ic_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_main_ops = ops;
		hwlog_info("dual loadswitch main ops register ok\n");
	} else {
		hwlog_info("dual loadswitch main ops has registered\n");
		ret = -1;
	}

	return ret;
}

int loadswitch_aux_ops_register(struct dc_ic_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_aux_ops = ops;
		hwlog_info("dual loadswitch aux ops register ok\n");
	} else {
		hwlog_info("dual loadswitch aux ops has registered\n");
		ret = -1;
	}

	return ret;
}

static int dual_loadswitch_charge_status(void *dev_data)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->get_ic_status)
		ret += g_main_ops->get_ic_status(dev_data);
	else
		ret += -1;

	if (g_aux_ops && g_aux_ops->get_ic_status)
		ret += g_aux_ops->get_ic_status(dev_data);
	else
		ret += -1;

	return ret;
}

static int dual_loadswitch_charge_init(void *dev_data)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->ic_init)
		ret += g_main_ops->ic_init(dev_data);

	if (g_aux_ops && g_aux_ops->ic_init)
		ret += g_aux_ops->ic_init(dev_data);

	return ret;
}

static int dual_loadswitch_charge_exit(void *dev_data)
{
	int ret = 0;

	if (!dli) {
		hwlog_err("dli is null\n");
		return -1;
	}

	if (dli->exit_order_inversion) {
		if (g_aux_ops && g_aux_ops->ic_exit)
			ret |= g_aux_ops->ic_exit(dev_data);

		if (g_main_ops && g_main_ops->ic_exit)
			ret |= g_main_ops->ic_exit(dev_data);
	} else {
		if (g_main_ops && g_main_ops->ic_exit)
			ret |= g_main_ops->ic_exit(dev_data);

		if (g_aux_ops && g_aux_ops->ic_exit)
			ret |= g_aux_ops->ic_exit(dev_data);
	}

	return ret;
}

static int dual_loadswitch_enable(int enable, void *dev_data)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->ic_enable)
		ret |= g_main_ops->ic_enable(enable, dev_data);
	else
		hwlog_err("g_main_ops is null\n");

	if (g_aux_ops && g_aux_ops->ic_enable)
		ret |= g_aux_ops->ic_enable(enable, dev_data);
	else
		hwlog_err("g_aux_ops is null\n");

	return ret;
}

static int dual_loadswitch_discharge(int enable, void *dev_data)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->ic_discharge)
		ret |= g_main_ops->ic_discharge(enable, dev_data);

	if (g_aux_ops && g_aux_ops->ic_discharge)
		ret |= g_aux_ops->ic_discharge(enable, dev_data);

	return ret;
}

static int dual_loadswitch_is_ls_close(void *dev_data)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->is_ic_close)
		ret |= g_main_ops->is_ic_close(dev_data);

	if (g_aux_ops && g_aux_ops->is_ic_close)
		ret |= g_aux_ops->is_ic_close(dev_data);

	return ret;
}

static int dual_loadswitch_get_ls_id(void *dev_data)
{
	int ret = -1;

	if (g_main_ops && g_main_ops->get_ic_id)
		ret = g_main_ops->get_ic_id(dev_data);

	return ret;
}

static int dual_loadswitch_config_watchdog_ms(int time, void *dev_data)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->config_ic_watchdog)
		ret |= g_main_ops->config_ic_watchdog(time, dev_data);

	if (g_aux_ops && g_aux_ops->config_ic_watchdog)
		ret |= g_aux_ops->config_ic_watchdog(time, dev_data);

	return ret;
}

static struct dc_ic_ops dual_loadswitch_ops = {
	.dev_name = "dual_loadswitch",
	.ic_init = dual_loadswitch_charge_init,
	.ic_exit = dual_loadswitch_charge_exit,
	.ic_enable = dual_loadswitch_enable,
	.ic_discharge = dual_loadswitch_discharge,
	.is_ic_close = dual_loadswitch_is_ls_close,
	.get_ic_id = dual_loadswitch_get_ls_id,
	.get_ic_status = dual_loadswitch_charge_status,
	.config_ic_watchdog = dual_loadswitch_config_watchdog_ms,
};

static int dual_loadswitch_probe(struct platform_device *pdev)
{
	struct dual_loadswitch_info *info = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	dli = info;
	info->pdev = pdev;
	info->dev = &pdev->dev;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), info->dev->of_node,
		"exit_order_inversion", &info->exit_order_inversion, 0);

	ret = dc_ic_ops_register(LVC_MODE, CHARGE_IC_TYPE_MAIN, &dual_loadswitch_ops);
	if (ret) {
		hwlog_err("dual loadswitch ops register failed\n");
		goto dual_loadswitch_fail_0;
	}

	if (!g_main_ops ||
		!g_main_ops->ic_init ||
		!g_main_ops->ic_enable ||
		!g_main_ops->ic_exit) {
		hwlog_err("main loadswitch ops is null\n");
		ret = -EINVAL;
		goto dual_loadswitch_fail_1;
	}

	if (!g_aux_ops ||
		!g_aux_ops->ic_init ||
		!g_aux_ops->ic_enable ||
		!g_aux_ops->ic_exit) {
		hwlog_err("aux loadswitch ops is null\n");
		ret = -EINVAL;
		goto dual_loadswitch_fail_2;
	}

	platform_set_drvdata(pdev, info);

	return ret;

dual_loadswitch_fail_2:
	g_aux_ops = NULL;
dual_loadswitch_fail_1:
	g_main_ops = NULL;
dual_loadswitch_fail_0:
	devm_kfree(&pdev->dev, info);
	dli = NULL;

	return ret;
}

static int dual_loadswitch_remove(struct platform_device *pdev)
{
	struct dual_loadswitch_info *info = platform_get_drvdata(pdev);

	if (!info)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	dli = NULL;

	return 0;
}

static const struct of_device_id dual_loadswitch_match_table[] = {
	{
		.compatible = "dual_loadswitch",
		.data = NULL,
	},
	{},
};

static struct platform_driver dual_loadswitch_driver = {
	.probe = dual_loadswitch_probe,
	.remove = dual_loadswitch_remove,
	.driver = {
		.name = "dual_loadswitch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dual_loadswitch_match_table),
	},
};

static int __init dual_loadswitch_init(void)
{
	return platform_driver_register(&dual_loadswitch_driver);
}

static void __exit dual_loadswitch_exit(void)
{
	platform_driver_unregister(&dual_loadswitch_driver);
}

device_initcall_sync(dual_loadswitch_init);
module_exit(dual_loadswitch_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("dual loadswitch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
