/*
 * wireless_rx_pmode.h
 *
 * power mode for wireless charging
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_RX_PMODE_H_
#define _WIRELESS_RX_PMODE_H_

#define WLRX_PMODE_CFG_ROW           8
#define WLRX_PMODE_CFG_COL           13

enum wlrx_pmode_judge_type {
	WLRX_PMODE_QUICK_JUDGE = 0, /* quick icon display */
	WLRX_PMODE_NORMAL_JUDGE, /* recorecting icon display */
	WLRX_PMODE_FINAL_JUDGE, /* judging power mode */
	WLDC_PMODE_FINAL_JUDGE, /* wireless direct charging */
};

struct wlrx_pmode {
	const char *name;
	int vtx_min;
	int itx_min;
	int vtx; /* ctrl_para */
	int vrx; /* ctrl_para */
	int irx; /* ctrl_para */
	int vrect_lth;
	int tbatt;
	int cable; /* cable detect type */
	int auth; /* authenticate type */
	int icon;
	int timeout;
	int expect_mode;
};

struct wlrx_vmode {
	u32 id;
	int vtx;
};

#endif /* _WIRELESS_RX_PMODE_H_ */
