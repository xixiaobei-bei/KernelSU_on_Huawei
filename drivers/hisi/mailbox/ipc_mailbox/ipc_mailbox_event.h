/*
 *
 * IPC mailbox driver event manager.
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
 */
#ifndef IPC_MAILBOX_EVENT_H
#define IPC_MAILBOX_EVENT_H

#include "ipc_mailbox.h"

#define MAX_EVENT_MSG_LEN 512

/* mailbox event type definition */
enum ipc_mbox_event {
	EVENT_IPC_SYNC_SEND_BEGIN = 0,
	EVENT_IPC_SYNC_SEND_END,
	EVENT_IPC_SYNC_SEND_TIMEOUT,
	EVENT_IPC_SYNC_RECV_ACK,

	EVENT_IPC_SYNC_TASKLET_JAM,
	EVENT_IPC_SYNC_ISR_JAM,
	EVENT_IPC_SYNC_ACK_LOST,

	EVENT_IPC_SYNC_PROC_RECORD,

	EVENT_IPC_BEFORE_IRQ_TO_MDEV,
	EVENT_IPC_AFTER_IRQ_TO_MDEV,

	EVENT_IPC_RECV_BH,
	EVENT_IPC_SEND_BH,

	EVENT_MBOX_COMPLETE_STATUS_CHANGE,

	EVENT_IPC_ASYNC_TASK_IN_QUEUE,
	EVENT_IPC_ASYNC_TASK_SEND,
	EVENT_IPC_ASYNC_IN_QUEUE_FAIL,

	EVENT_IPC_MBX_STATUS_CHANGE,
	EVENT_IPC_DEACTIVATE_BEGIN,

	MAX_IPC_EVENT_NUM
};


enum ipc_mailbox_event_err_code {
	EVENT_SUCCESS,

	ER_ALLOC_NOTIFIER_FAILED,
	ER_INVALID_EVENT_NUM,
	ER_NOTIFIER_REGIST_FAILED,
	ER_INVALID_EVENT_MSG,
	ER_NULL_EVENT_HANDLE,
	ER_UNKOWN_EVENT_ERR
};

int ipc_regist_mailbox_event(unsigned int event,
	void (*event_handle)(struct hisi_mbox_device *mbox,
		unsigned int event, const char *fmt_msg, va_list args));

int ipc_unregist_mailbox_event(unsigned int event,
	void (*event_handle)(struct hisi_mbox_device *mbox,
		unsigned int event, const char *fmt_msg, va_list args));

void ipc_notify_mailbox_event(unsigned int event,
	struct hisi_mbox_device *mbox, const char *fmt_msg, ...);

void ipc_clear_mailbox_event_notifier(unsigned int event);

char *ipc_get_event_name(unsigned int event);

#endif
