/*
 * wireless_dc_cp.h
 *
 * common interface, variables, definition etc for wireless dc cp
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_DC_CP_H_
#define _WIRELESS_DC_CP_H_

#define WLDC_CP_ERR_CNT                   1
#define WLDC_CP_AUX_CHECK_CNT             1
#define WLDC_CP_VOUT_ERR_TH               1000

enum wldc_cp_ibus_info {
	WIRELESS_DC_CP_IBUS_HTH = 0,
	WIRELESS_DC_CP_HTIME,
	WIRELESS_DC_CP_IBUS_LTH,
	WIRELESS_DC_CP_LTIME,
	WIRELESS_DC_CP_TOTAL,
};

struct wldc_cp_ibus_para {
	int hth;
	int h_time;
	int hth_cnt;
	int lth;
	int l_time;
	int lth_cnt;
};

struct wldc_cp_data {
	int support_multi_cp;
	int cur_type;
	int single_cp_iout_th;
	int aux_check_cnt;
	u32 multi_cp_err_cnt;
	struct wldc_cp_ibus_para ibus_para;
};

#endif /* _WIRELESS_DC_CP_H_ */
