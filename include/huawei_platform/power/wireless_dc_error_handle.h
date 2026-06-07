/*
 * wireless_dc_error_handle.h
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

#ifndef _WIRELESS_DC_ERROR_HANDLE_H_
#define _WIRELESS_DC_ERROR_HANDLE_H_

enum wldc_eh_type {
	WLDC_EH_BEGIN = 0,
	/* for battery */
	WLDC_EH_BI_INIT = WLDC_EH_BEGIN,
	WLDC_EH_GET_VBATT,
	WLDC_EH_GET_IBATT,
	/* for ls/sc */
	WLDC_EH_LS_INIT,
	WLDC_EH_GET_LS_VBUS,
	WLDC_EH_GET_LS_IBUS,
	/* for rx */
	WLDC_EH_SET_TRX_INIT_VOUT,
	/* for security check */
	WLDC_EH_OPEN_DC_PATH,
	WLDC_EH_ILEAK,
	WLDC_EH_VDIFF,
	WLDC_EH_VOUT_ACCURACY,
	/* for charging */
	WLDC_CUT_OFF_NORMAL_PATH,
	WLDC_TURN_ON_DC_PATH,
	WLDC_EH_END,
};

void wldc_fill_eh_buf(char *buf, int size, int type, char *str);
void wldc_show_eh_buf(char *buf);
void wldc_reset_eh_buf(char *buf, int size);
void wldc_report_eh_buf(char *buf, int err_no);

#endif /* _WIRELESS_DC_ERROR_HANDLE_H_ */
