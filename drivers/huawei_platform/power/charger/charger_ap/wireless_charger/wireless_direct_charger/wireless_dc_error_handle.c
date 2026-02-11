/*
 * wireless_dc_error_handle.c
 *
 * error process interface for wireless direct charger
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
#include <linux/kobject.h>
#include <linux/slab.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_dsm.h>
#include <huawei_platform/power/wireless_dc_error_handle.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG wireless_dc_eh
HWLOG_REGIST();

static const char * const wldc_eh_error_table[WLDC_EH_END] = {
	/* for battery */
	[WLDC_EH_BI_INIT] = "batt_info init",
	[WLDC_EH_GET_VBATT] = "get vbatt",
	[WLDC_EH_GET_IBATT] = "get ibatt",
	/* for ls/sc */
	[WLDC_EH_LS_INIT] = "ls chip_init",
	[WLDC_EH_GET_LS_VBUS] = "get ls_vbus",
	[WLDC_EH_GET_LS_IBUS] = "get ls_ibus",
	/* for rx */
	[WLDC_EH_SET_TRX_INIT_VOUT] = "set trx_init_vout",
	/* for security check */
	[WLDC_EH_OPEN_DC_PATH] = "open dc_path",
	[WLDC_EH_ILEAK] = "check ileak",
	[WLDC_EH_VDIFF] = "check vdiff",
	[WLDC_EH_VOUT_ACCURACY] = "check vout accuracy",
	/* for charging */
	[WLDC_CUT_OFF_NORMAL_PATH] = "cutt off normal path",
	[WLDC_TURN_ON_DC_PATH] = "turn on dc path",
};

static const char *wldc_get_eh_string(int type)
{
	if ((type >= WLDC_EH_BEGIN) && (type < WLDC_EH_END))
		return wldc_eh_error_table[type];

	hwlog_err("%s: invalid eh type(0x%x)\n", __func__, type);
	return NULL;
}

void wldc_fill_eh_buf(char *buf, int size, int type, char *str)
{
	const char *eh_string = NULL;
	int actual_size;
	int remain_size;

	if (!buf) {
		hwlog_err("%s: buf null\n", __func__);
		return;
	}

	/*
	 * get the remaining size of the buffer
	 * reserve 16 bytes to prevent buffer overflow
	 */
	actual_size = strlen(buf);
	if (actual_size + 16 >= size) {
		hwlog_err("%s: buf full\n", __func__);
		return;
	}
	remain_size = size - (actual_size + 16);

	eh_string = wldc_get_eh_string(type);
	if (!eh_string)
		return;

	if (!str) {
		hwlog_err("fill_buffer: %s\n", eh_string);
		snprintf(buf + actual_size, remain_size, "wldc_error: %s\n",
			eh_string);
	} else {
		hwlog_err("fill_buffer: %s %s\n", eh_string, str);
		snprintf(buf + actual_size, remain_size, "wldc_error: %s %s\n",
			eh_string, str);
	}
}

void wldc_show_eh_buf(char *buf)
{
	if (!buf) {
		hwlog_err("%s: buf null\n", __func__);
		return;
	}

	hwlog_info("%s\n", buf);
}

void wldc_reset_eh_buf(char *buf, int size)
{
	if (!buf) {
		hwlog_err("%s: buf null\n", __func__);
		return;
	}

	memset(buf, 0, size);

	hwlog_info("[%s]\n", __func__);
}

void wldc_report_eh_buf(char *buf, int err_no)
{
	if (!buf) {
		hwlog_err("%s: buf null\n", __func__);
		return;
	}

	power_dsm_dmd_report(POWER_DSM_BATTERY, err_no, buf);
}
