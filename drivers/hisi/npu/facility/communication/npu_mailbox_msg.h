/*
 * npu_mailbox_msg.h
 *
 * about npu mailbox msg
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
#ifndef __NPU_MAILBOX_MSG_H
#define __NPU_MAILBOX_MSG_H
#include <linux/types.h>

#include "npu_common.h"

enum mailbox_cmd_type {
	DEVDRV_MAILBOX_CREATE_CQSQ_CALC = 0x01,
	DEVDRV_MAILBOX_RELEASE_CQSQ_CALC = 0x02,
	DEVDRV_MAILBOX_CREATE_CQSQ_LOG = 0x03,
	DEVDRV_MAILBOX_RELEASE_CQSQ_LOG = 0x04,
	DEVDRV_MAILBOX_CREATE_CQSQ_DEBUG = 0x05,
	DEVDRV_MAILBOX_RELEASE_CQSQ_DEBUG = 0x06,
	DEVDRV_MAILBOX_CREATE_CQSQ_PROFILE = 0x07,
	DEVDRV_MAILBOX_RELEASE_CQSQ_PROFILE = 0x08,
	DEVDRV_MAILBOX_CREATE_CQSQ_BEAT = 0x09,
	DEVDRV_MAILBOX_RELEASE_CQSQ_BEAT = 0x0A,
	DEVDRV_MAILBOX_RECYCLE_EVENT_ID = 0x0B,
	DEVDRV_MAILBOX_RECYCLE_STREAM_ID = 0x0C,
	DEVDRV_MAILBOX_SEND_CONTAINER_TFLOP = 0x0D,
	DEVDRV_MAILBOX_CONFIG_P2P_INFO = 0x0E,
	DEVDRV_MAILBOX_SEND_RDMA_INFO = 0x0F,
	DEVDRV_MAILBOX_RESET_NOTIFY_ID = 0x10,
	DEVDRV_MAILBOX_RECYCLE_PID_ID = 0x12,
	DEVDRV_MAILBOX_INVALID_INDEX = 0xFF
};

#define DEVDRV_RECYCLE_MAX_EVENT_NUM  25

struct devdrv_stream_msg {
	u16 valid;       /* validity judgement, 0x5a5a is valid */
	u16 cmd_type;       /* command type */
	u32 result;       /* TS's process result succ or fail: no error: 0, error: not 0 */
	u64 sq_addr;       /* invalid addr: 0x0 */
	u64 cq0_addr;
	u64 cq1_addr;
	u64 ttbr;       /* ttbr */
	u64 tcr;       /* tcr */
	u16 sq_index;       /* invalid index: 0xFFFF */
	u16 cq0_index;       /* sq's return */
	u16 cq1_index;       /* ts's return */
	u16 cq2_index;       /* ai cpu's return */
	u16 ssid;       /* ssid */
	u16 stream_id;       /* binding stream id for sq and cq (for calculation task) */
	u8 plat_type;       /* inform TS, msg is sent from host or device, device: 0 host: 1 */
	u8 cq_slot_size;    /* calculation cq's slot size, default: 12 bytes */
	u16 group_id;
};

struct devdrv_mailbox_message {
	u8 is_sent;
	u8 *message_payload;
	int message_length;
	int message_type;
	int process_result;
	int cmd_type; /* related to task */
	int sync_type; /* sync or nonsync */
	int message_index;  /* when there is several message within one task */
	int message_pid; /* who am i(process who send this message) */
	int abandon;
	struct semaphore wait;
	struct work_struct send_work;
	struct list_head send_list_node;
	struct devdrv_mailbox *mailbox;
};

struct devdrv_mailbox_message_header {
	u16 valid; /* validity judgement, 0x5a5a is valid */
	u16 cmd_type; /* identify command or operation */
	u32 result; /* TS's process result succ or fail */
};

struct devdrv_recycle_event_msg {
	struct devdrv_mailbox_message_header header;
	u16 count;
	u16 event[DEVDRV_RECYCLE_MAX_EVENT_NUM];
	u8 plat_type;
	u8 reserved[3];
};

int devdrv_create_alloc_stream_msg(u8 dev_id, u32 stream_id, struct devdrv_stream_msg *stream_msg);

int devdrv_create_free_stream_msg(u8 dev_id, u32 stream_id, struct devdrv_stream_msg *stream_msg);

int devdrv_create_recycle_event_msg(const struct devdrv_event_info *event_info,
                                    struct devdrv_recycle_event_msg *recycle_event);

#endif
