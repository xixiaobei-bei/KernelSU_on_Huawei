/*
 * power_cmdline.c
 *
 * cmdline parse interface for power module
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
#include <linux/kernel.h>
#include <huawei_platform/log/hw_log.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG power_cmdline
HWLOG_REGIST();

static bool g_init_flag;
static bool g_factory_mode_flag;
static bool g_powerdown_charging_flag;

bool power_cmdline_is_factory_mode(void)
{
	if (g_init_flag)
		return g_factory_mode_flag;

	if (strstr(saved_command_line, "androidboot.swtype=factory")) {
		hwlog_info("is factory mode\n");
		g_factory_mode_flag = true;
	} else {
		g_factory_mode_flag = false;
	}

	return g_factory_mode_flag;
}
EXPORT_SYMBOL_GPL(power_cmdline_is_factory_mode);

bool power_cmdline_is_powerdown_charging_mode(void)
{
	if (g_init_flag)
		return g_powerdown_charging_flag;

	if (strstr(saved_command_line, "androidboot.mode=charger")) {
		hwlog_info("is powerdown charging mode\n");
		g_powerdown_charging_flag = true;
	} else {
		g_powerdown_charging_flag = false;
	}

	return g_powerdown_charging_flag;
}
EXPORT_SYMBOL_GPL(power_cmdline_is_powerdown_charging_mode);

static int __init power_cmdline_init(void)
{
	hwlog_info("probe begin\n");

	(void)power_cmdline_is_factory_mode();
	(void)power_cmdline_is_powerdown_charging_mode();
	g_init_flag = true;

	hwlog_info("probe end\n");
	return 0;
}

static void __exit power_cmdline_exit(void)
{
	hwlog_info("remove end\n");
}

subsys_initcall(power_cmdline_init);
module_exit(power_cmdline_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cmdline for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
