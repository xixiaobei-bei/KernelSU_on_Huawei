/*
 * direct_charger_error_handle.h
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

#ifndef _DIRECT_CHARGER_ERROR_HANDLE_H_
#define _DIRECT_CHARGER_ERROR_HANDLE_H_

enum direct_charge_eh_type {
	DC_EH_BEGIN = 0,

	/* for battery */
	DC_EH_GET_VBAT = DC_EH_BEGIN,
	DC_EH_GET_IBAT,
	/* for device */
	DC_EH_GET_DEVICE_VBUS,
	DC_EH_GET_DEVICE_IBUS,
	DC_EH_GET_DEVICE_TEMP,
	DC_EH_DEVICE_IS_CLOSE,
	/* for adapter */
	DC_EH_GET_VAPT,
	DC_EH_GET_IAPT,
	DC_EH_GET_APT_CURR_SET,
	DC_EH_GET_APT_MAX_CURR,
	DC_EH_GET_APT_TEMP,
	DC_EH_SET_APT_VOLT,
	DC_EH_SET_APT_CURR,
	/* for charging */
	DC_EH_INIT_DIRECT_CHARGE,
	DC_EH_OPEN_DIRECT_CHARGE_PATH,
	DC_EH_APT_VOLTAGE_ACCURACY,
	DC_EH_FULL_PATH_RESISTANCE,
	DC_EH_USB_PORT_LEAGAGE_CURR,
	DC_EH_IBAT_ABNORMAL,
	DC_EH_TLS_ABNORMAL,
	DC_EH_TADP_ABNORMAL,
	/* for lvc & sc */
	DC_EH_HAPPEN_LVC_FAULT,
	DC_EH_HAPPEN_SC_FAULT,

	DC_EH_END,
};

#ifdef CONFIG_DIRECT_CHARGER
int direct_charge_fill_eh_buf(char *buf, int size, int type, char *str);
void direct_charge_show_eh_buf(char *buf);
void direct_charge_clean_eh_buf(char *buf, int size);
void direct_charge_report_eh_buf(char *buf, int err_no);

#else

static inline int direct_charge_fill_eh_buf(char *buf, int size,
	int type, char *str)
{
	return -1;
}

static inline void direct_charge_show_eh_buf(char *buf)
{
}

static inline void direct_charge_clean_eh_buf(char *buf, int size)
{
}

static inline void direct_charge_report_eh_buf(char *buf, int err_no)
{
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGER_ERROR_HANDLE_H_ */
