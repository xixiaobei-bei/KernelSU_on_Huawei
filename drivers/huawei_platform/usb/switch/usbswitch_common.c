/*
 * usbswitch_common.c
 *
 * common interface for usbswitch driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/errno.h>

#include <huawei_platform/log/hw_log.h>
#include "usbswitch_common.h"

#define HWLOG_TAG usbswitch_common
HWLOG_REGIST();

static struct usbswitch_common_ops *g_switch_ops;

int usbswitch_common_ops_register(struct usbswitch_common_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_switch_ops = ops;
		hwlog_info("usbswitch common ops register ok\n");
	} else {
		hwlog_info("usbswitch common ops register fail\n");
		ret = -1;
	}

	return ret;
}

int usbswitch_common_manual_sw(int input_select)
{
	if (!g_switch_ops || !g_switch_ops->manual_switch) {
		hwlog_err("g_switch_ops or manual_switch is null\n");
		return -1;
	}

	return g_switch_ops->manual_switch(input_select);
}

int usbswitch_common_manual_detach(void)
{
	if (!g_switch_ops || !g_switch_ops->manual_detach) {
		hwlog_err("g_switch_ops or manual_detach is null\n");
		return -1;
	}

	return g_switch_ops->manual_detach();
}

int usbswitch_common_dcd_timeout_enable(bool enable_flag)
{
	if (!g_switch_ops || !g_switch_ops->dcd_timeout_enable) {
		hwlog_err("g_switch_ops or dcd_timeout_enable is null\n");
		return -1;
	}

	return g_switch_ops->dcd_timeout_enable(enable_flag);
}

int usbswitch_common_dcd_timeout_status(void)
{
	if (!g_switch_ops || !g_switch_ops->dcd_timeout_status) {
		hwlog_err("g_switch_ops or dcd_timeout_status is null\n");
		return -1;
	}

	return g_switch_ops->dcd_timeout_status();
}
