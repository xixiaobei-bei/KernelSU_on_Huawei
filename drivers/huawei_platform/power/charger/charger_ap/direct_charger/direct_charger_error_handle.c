/*
 * direct_charger_error_handle.c
 *
 * error process interface for direct charger
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
#include <huawei_platform/power/direct_charger_error_handle.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG direct_charge_eh
HWLOG_REGIST();

static const char * const direct_charge_eh_error_table[] = {
	/* for battery */
	[DC_EH_GET_VBAT] = "get battery charging voltage",
	[DC_EH_GET_IBAT] = "get battery charging current",
	/* for device */
	[DC_EH_GET_DEVICE_VBUS] = "get lvc or sc device input voltage",
	[DC_EH_GET_DEVICE_IBUS] = "get lvc or sc device input current",
	[DC_EH_GET_DEVICE_TEMP] = "get lvc or sc device temp",
	[DC_EH_DEVICE_IS_CLOSE] = "lvc or sc device is close",
	/* for adapter */
	[DC_EH_GET_VAPT] = "get adapter output voltage",
	[DC_EH_GET_IAPT] = "get adapter output current",
	[DC_EH_GET_APT_CURR_SET] = "get adapter setting current",
	[DC_EH_GET_APT_MAX_CURR] = "get adapter max current",
	[DC_EH_GET_APT_TEMP] = "get adapter temp",
	[DC_EH_SET_APT_VOLT] = "set adapter voltage",
	[DC_EH_SET_APT_CURR] = "set adapter current",
	/* for charging */
	[DC_EH_INIT_DIRECT_CHARGE] = "init direct charge",
	[DC_EH_OPEN_DIRECT_CHARGE_PATH] = "open direct charge path",
	[DC_EH_APT_VOLTAGE_ACCURACY] = "check adapter voltage accuracy",
	[DC_EH_FULL_PATH_RESISTANCE] = "check full path resistance",
	[DC_EH_USB_PORT_LEAGAGE_CURR] = "check usb port leakage current",
	[DC_EH_IBAT_ABNORMAL] = "ibat abnormal in select stage",
	[DC_EH_TLS_ABNORMAL] = "device temp abnormal in regulation",
	[DC_EH_TADP_ABNORMAL] = "adapter temp abnormal in regulation",
	/* for lvc & sc */
	[DC_EH_HAPPEN_LVC_FAULT] = "lvc fault happened",
	[DC_EH_HAPPEN_SC_FAULT] = "sc fault happened",
};

static const char *direct_charge_get_eh_string(int type)
{
	if ((type >= DC_EH_BEGIN) && (type < DC_EH_END))
		return direct_charge_eh_error_table[type];

	hwlog_err("invalid eh type(%d)\n", type);
	return NULL;
}

int direct_charge_fill_eh_buf(char *buf, int size, int type, char *str)
{
	const char *eh_string = NULL;
	int actual_size;
	int remain_size;

	if (!buf) {
		hwlog_err("buf is null\n");
		return -1;
	}

	/*
	 * get the remaining size of the buffer
	 * reserve 16 bytes to prevent buffer overflow
	 */
	actual_size = strlen(buf);
	if (actual_size + 16 >= size) {
		hwlog_err("buf is full\n");
		return -1;
	}
	remain_size = size - (actual_size + 16);

	eh_string = direct_charge_get_eh_string(type);
	if (!eh_string)
		return -1;

	if (!str) {
		hwlog_err("fill_buffer: %s\n", eh_string);
		snprintf(buf + actual_size, remain_size, "dc_error: %s\n",
			eh_string);
	} else {
		hwlog_err("fill_buffer: %s %s\n", eh_string, str);
		snprintf(buf + actual_size, remain_size, "dc_error: %s %s\n",
			eh_string, str);
	}

	return 0;
}

void direct_charge_show_eh_buf(char *buf)
{
	if (!buf) {
		hwlog_err("buf is null\n");
		return;
	}

	hwlog_info("%s\n", buf);
}

void direct_charge_clean_eh_buf(char *buf, int size)
{
	if (!buf) {
		hwlog_err("buf is null\n");
		return;
	}

	memset(buf, 0, size);

	hwlog_info("clean_eh_buf\n");
}

void direct_charge_report_eh_buf(char *buf, int err_no)
{
	if (!buf) {
		hwlog_err("buf is null\n");
		return;
	}

	power_dsm_dmd_report(POWER_DSM_BATTERY, err_no, buf);

	hwlog_info("report_eh_buf\n");
}
