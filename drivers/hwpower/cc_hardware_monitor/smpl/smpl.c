/*
 * smpl.c
 *
 * smpl(sudden momentary power loss) error monitor driver
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

#include "smpl.h"
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>

#define HWLOG_TAG smpl
HWLOG_REGIST();

static struct smpl_dev *g_smpl_dev;

static bool smpl_check_reboot_reason(void)
{
	if (strstr(saved_command_line, "normal_reset_type=SMPL")) {
		hwlog_info("smpl happened: normal_reset_type=smpl\n");
		return true;
	} else if (strstr(saved_command_line, "reboot_reason=power_loss")) {
		hwlog_info("smpl happened: reboot_reason=power_loss\n");
		return true;
	} else if (strstr(saved_command_line, "reboot_reason=2sec_reboot")) {
		hwlog_info("smpl happened: reboot_reason=2sec_reboot\n");
		return true;
	}

	return false;
}

static void smpl_error_monitor_work(struct work_struct *work)
{
	char buf[POWER_DSM_BUF_SIZE_0128] = { 0 };

	hwlog_info("monitor_work begin\n");

	if (!smpl_check_reboot_reason())
		return;

	snprintf(buf, POWER_DSM_BUF_SIZE_0128 - 1,
		"smpl happened : brand=%s t_bat=%d, volt=%d, soc=%d\n",
		power_platform_get_battery_brand(),
		power_platform_get_battery_temperature(),
		power_platform_get_battery_voltage(),
		power_platform_get_battery_capacity());
	power_dsm_report_dmd(POWER_DSM_SMPL, ERROR_NO_SMPL, buf);
	hwlog_info("smpl happened: %s\n", buf);
}

static int __init smpl_init(void)
{
	struct smpl_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_smpl_dev = l_dev;

	INIT_DELAYED_WORK(&l_dev->monitor_work, smpl_error_monitor_work);
	schedule_delayed_work(&l_dev->monitor_work,
		msecs_to_jiffies(DELAY_TIME_FOR_MONITOR_WORK));
	return 0;
}

static void __exit smpl_exit(void)
{
	if (!g_smpl_dev)
		return;

	cancel_delayed_work(&g_smpl_dev->monitor_work);
	kfree(g_smpl_dev);
	g_smpl_dev = NULL;
}

module_init(smpl_init);
module_exit(smpl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("smpl error monitor module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
