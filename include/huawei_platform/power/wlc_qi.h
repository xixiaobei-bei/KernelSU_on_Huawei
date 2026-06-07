/*
 * wlc_qi.h
 *
 * Qi cmd and var for wireless rx and tx
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

#ifndef _WLC_QI_H_
#define _WLC_QI_H_

/* command info */
#define WLC_CMD_GET_TX_FW                0x05
#define WLC_TX_FW_LEN                    4
#define WLC_CMD_SET_TX_VOUT              0x0A
#define WLC_CMD_GET_TX_CAP               0x41

enum wlc_tx_cap_info {
	WLC_TX_CAP_TYPE = 0,
	WLC_TX_CAP_VMAX,
	WLC_TX_CAP_IMAX,
	WLC_TX_CAP_ATTR,
	WLC_TX_CAP_TOTAL,
};

struct wlc_tx_cap {
	u8 type;
	int vout;
	int iout;
	u8 attr;
};

#endif /* _WLC_QI_H_ */
