/*
 * power_cmdline.c
 *
 * cmdline parse interface for power module
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_cmdline
HWLOG_REGIST();

static bool g_init_flag;
static bool g_factory_mode_flag;
static bool g_recovery_mode_flag;
static bool g_erecovery_mode_flag;
static bool g_powerdown_charging_flag;

#ifdef CONFIG_HUAWEI_POWER_DEBUG
bool power_cmdline_is_root_mode(void)
{
	return true;
}
#else
bool power_cmdline_is_root_mode(void)
{
	return false;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

bool power_cmdline_is_factory_mode(void)
{
	if (g_init_flag)
		return g_factory_mode_flag;

	if (strstr(saved_command_line, "androidboot.swtype=factory"))
		g_factory_mode_flag = true;
	else
		g_factory_mode_flag = false;

	return g_factory_mode_flag;
}

bool power_cmdline_is_recovery_mode(void)
{
	if (g_init_flag)
		return g_recovery_mode_flag;

	if (strstr(saved_command_line, "enter_recovery=1"))
		g_recovery_mode_flag = true;
	else
		g_recovery_mode_flag = false;

	return g_recovery_mode_flag;
}

bool power_cmdline_is_erecovery_mode(void)
{
	if (g_init_flag)
		return g_erecovery_mode_flag;

	if (strstr(saved_command_line, "enter_erecovery=1"))
		g_erecovery_mode_flag = true;
	else
		g_erecovery_mode_flag = false;

	return g_erecovery_mode_flag;
}

bool power_cmdline_is_powerdown_charging_mode(void)
{
	if (g_init_flag)
		return g_powerdown_charging_flag;

	if (strstr(saved_command_line, "androidboot.mode=charger"))
		g_powerdown_charging_flag = true;
	else
		g_powerdown_charging_flag = false;

	return g_powerdown_charging_flag;
}

static int __init power_cmdline_init(void)
{
	(void)power_cmdline_is_factory_mode();
	(void)power_cmdline_is_recovery_mode();
	(void)power_cmdline_is_erecovery_mode();
	(void)power_cmdline_is_powerdown_charging_mode();
	g_init_flag = true;

	return 0;
}

static void __exit power_cmdline_exit(void)
{
}

subsys_initcall(power_cmdline_init);
module_exit(power_cmdline_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cmdline for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
