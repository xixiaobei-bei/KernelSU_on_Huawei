/*
 * direct_charge_error_handle.h
 *
 * error process interface for direct charge
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

#ifndef _DIRECT_CHARGE_ERROR_HANDLE_H_
#define _DIRECT_CHARGE_ERROR_HANDLE_H_

#include <linux/of.h>

enum dc_eh_type {
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
	DC_EH_GET_APT_PWR_DROP_CURR,
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
	DC_EH_CURR_RATIO,
	DC_EH_VBAT_DELTA,
	DC_EH_TBAT_DELTA,
	DC_EH_TLS_ABNORMAL,
	DC_EH_TADP_ABNORMAL,
	/* for lvc & sc */
	DC_EH_HAPPEN_LVC_FAULT,
	DC_EH_HAPPEN_SC_FAULT,
	/* for battery with wireless */
	WLDC_EH_BI_INIT,
	WLDC_EH_GET_VBATT,
	WLDC_EH_GET_IBATT,
	/* for ls&sc with wireless */
	WLDC_EH_LS_INIT,
	WLDC_EH_GET_LS_VBUS,
	WLDC_EH_GET_LS_IBUS,
	/* for rx with wireless */
	WLDC_EH_SET_TRX_INIT_VOUT,
	/* for security check with wireless */
	WLDC_EH_OPEN_DC_PATH,
	WLDC_EH_ILEAK,
	WLDC_EH_VDIFF,
	WLDC_EH_VOUT_ACCURACY,
	/* for charging with wireless */
	WLDC_EH_CUT_OFF_NORMAL_PATH,
	WLDC_EH_TURN_ON_DC_PATH,
	DC_EH_END,
};

struct nty_data {
	unsigned short addr;
	u8 event1;
	u8 event2;
	u8 event3;
};

#ifdef CONFIG_DIRECT_CHARGER
void dc_fill_eh_buf(char *buf, int size, int type, const char *str);
void dc_show_eh_buf(const char *buf);
void dc_clean_eh_buf(char *buf, int size);
void dc_report_eh_buf(const char *buf, int err_no);
#else
static inline void dc_fill_eh_buf(char *buf, int size, int type, const char *str)
{
}

static inline void dc_show_eh_buf(const char *buf)
{
}

static inline void dc_clean_eh_buf(char *buf, int size)
{
}

static inline void dc_report_eh_buf(const char *buf, int err_no)
{
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_ERROR_HANDLE_H_ */
