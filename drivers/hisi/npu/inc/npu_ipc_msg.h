/*
 * npu_ipc_msg.h
 *
 * about npu ipc msg
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_IPC_MSG_H
#define __NPU_IPC_MSG_H

struct ipcdrv_msg_payload {
	u8 result;
	u8 reserved[IPCDRV_MSG_LENGTH - 1];
};

enum ts_msg_cmd_type {
	IPCDRV_TS_SUSPEND = 0,
	IPCDRV_TS_SUSPEND_READY = 1,
	IPCDRV_TS_CONFIG_AICPU_GICR = 2,
	IPCDRV_TS_INFORM_CORE_INFO = 3,
	IPCDRV_TS_INQUIRY_AI_RATE = 4,
	IPCDRV_TS_INFORM_TS_IDLE = 5,
	IPCDRV_TS_INFORM_TS_LIMIT_AICORE = 6,
	IPCDRV_TS_INFORM_TS_CANCEL_LIMIT = 7,
	IPCDRV_TS_UPPER_DDR_FREQ = 8,
	IPCDRV_TS_LOWER_DDR_FREQ = 9,
	IPCDRV_TS_HEARTBEAT_TOAICPU = 10,
	IPCDRV_TS_TIME_SYNC = 11,
	IPCDRV_TS_MAX,
};

#endif
