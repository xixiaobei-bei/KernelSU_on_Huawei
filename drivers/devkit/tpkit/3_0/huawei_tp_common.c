/*
 * Huawei Touchscreen Driver
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/notifier.h>
#include <huawei_ts_kit.h>
#include <huawei_platform/log/hw_log.h>

#define TP_COLOR_BUF_SIZE 20
#define WHITE 0xE1
#define BLACK 0xD2
#define CMDLINE_PANEL_NAME "boe_nt51017te_9p6_800p_video"

char __attribute__((weak)) tp_color_buf[TP_COLOR_BUF_SIZE];
unsigned int panel_name_flag;

int __init early_parse_tp_color_cmdline(char *arg)
{
	unsigned int len;

	memset(tp_color_buf, 0, sizeof(tp_color_buf));
	if (arg) {
		len = strlen(arg);
		if (len >= sizeof(tp_color_buf))
			len = sizeof(tp_color_buf) - 1;
		memcpy(tp_color_buf, arg, len);
	} else {
		TS_LOG_INFO("%s : arg is NULL\n", __func__);
	}

	return 0;
}

early_param("TP_COLOR", early_parse_tp_color_cmdline);

/*
 * parse panel_name cmdline which is passed from lk
 * Format : new agassi lcd:panel_name_flag = true;
 * Format : old agassi lcd:panel_name_flag = false;
 */
static int __init early_parse_panel_name_cmdline(char *p)
{
	if (p) {
		if (strstr(p, CMDLINE_PANEL_NAME) != NULL)
			panel_name_flag = true;
		else
			panel_name_flag = false;
		TS_LOG_INFO("panel_name_flag :%u\n", panel_name_flag);
	}
	return 0;
}

early_param("mdss_mdp.panel", early_parse_panel_name_cmdline);

