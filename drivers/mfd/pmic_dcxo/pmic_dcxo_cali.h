/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_dcxo_cali.h
 *
 * driver for pmic dcxo calibration
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __PMIC_DCXO_CALI_H
#define __PMIC_DCXO_CALI_H

#include <linux/types.h>

enum IPC_MSG_TYPE {
	IPC_MSG_CFG_DCXO_PARA = 1,
	IPC_MSG_BUTT
};

struct ipcmsg_header_t {
	u32 version   : 4;
	u32 func      : 8;
	u32 split     : 3;
	u32 length    : 9;
	u32 timestamp : 8;
	u16 msg_type;
	u16 rsv;
};

struct cfg_dcxo_para_req {
	struct ipcmsg_header_t msg_head;
	u16 dcxo_c1;
	u16 dcxo_c2;
};

struct cfg_dcxo_para_resp {
	struct ipcmsg_header_t msg_head;
	u32 result;
};

struct dcxo_msg_proc {
	enum IPC_MSG_TYPE msg_type;
	int (*msg_proc)(void *msg);
};

#endif
