/*
 * npu_ipc.h
 *
 * about npu ipc
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
#ifndef __DRV_IPC_H__
#define __DRV_IPC_H__

#include <linux/errno.h>
#include <linux/hisi/hisi_rproc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define IPCDRV_RPROC_MSG_LENGTH 8
#define IPCDRV_MSG_LENGTH 28
#define IPCDRV_MSG_MAX_LENGTH           128

struct ipcdrv_msg_header {
	u32 msg_type : 1;
	u32 cmd_type : 7;
	u32 sync_type : 1;
	u32 reserved : 1;
	u32 msg_length : 14;
	u32 msg_index : 8;
};

struct ipcdrv_message {
	struct ipcdrv_msg_header ipc_msg_header;
	u8 ipcdrv_payload[IPCDRV_MSG_LENGTH];
};

struct ipcdrv_msg {
	struct ipcdrv_message ipc_message;
	u8 channel_type;
	void (*eventCallbackfunc)(int result, void *args);
};

enum ipc_msg_type {
	MSGTYPE_DRIVER_SEND = 0,
	MSGTYPE_DRIVER_RECEIVE = 1
};

enum ipc_msg_sync_type {
	IPCDRV_MSG_SYNC = 0,
	IPCDRV_MSG_ASYNC = 1
};

struct npu_time_sync_message {
	struct timespec64 system_time;
	struct timespec64 wall_time;
	uint64_t ccpu_cycle;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __DRV_IPC_H__ */
