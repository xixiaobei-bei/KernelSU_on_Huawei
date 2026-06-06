/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#ifndef __NPU_MAILBOX_H
#define __NPU_MAILBOX_H

#include <linux/irq.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include "npu_common.h"
#include "npu_mailbox_msg.h"

#define DEVDRV_MAILBOX_SRAM (0)

#define DEVDRV_MAILBOX_FREE (0)
#define DEVDRV_MAILBOX_BUSY (1)

#define DEVDRV_MAILBOX_VALID_MESSAGE   (0)
#define DEVDRV_MAILBOX_RECYCLE_MESSAGE (1)

#define DEVDRV_MAILBOX_PAYLOAD_LENGTH (64)

#define DEVDRV_MAILBOX_MESSAGE_VALID (0x5A5A)

#define DEVDRV_DOORBEEL_TYPE (0)

#define DEVDRV_MAILBOX_SYNC_MESSAGE  (1)
#define DEVDRV_MAILBOX_ASYNC_MESSAGE (2)

#define DEVDRV_MAILBOX_SYNC          (0)
#define DEVDRV_MAILBOX_ASYNC         (1)

#define DEVDRV_MAILBOX_SEMA_TIMEOUT_SECOND (5)

int devdrv_mailbox_init(u8 dev_id);
int devdrv_mailbox_message_send_for_res(u8 dev_id,
					u8 *buf, u32 len, int *result);
int devdrv_mailbox_message_send(struct devdrv_mailbox *mailbox,
				struct devdrv_mailbox_message *message,
				int *result);
int devdrv_mailbox_message_send_ext(struct devdrv_mailbox *mailbox,
				struct devdrv_mailbox_message *message,
				int *result);
void devdrv_mailbox_exit(struct devdrv_mailbox *mailbox);
void devdrv_mailbox_recycle(struct devdrv_mailbox *mailbox);
void devdrv_mailbox_destroy(int dev_id);

#endif
