/*
 * npu_mailbox.h
 *
 * about npu mailbox
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
#ifndef __NPU_MAILBOX_H
#define __NPU_MAILBOX_H

#include <linux/irq.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include "npu_common.h"
#include "npu_mailbox_msg.h"

#define DEVDRV_MAILBOX_SRAM 0

#define DEVDRV_MAILBOX_FREE 0
#define DEVDRV_MAILBOX_BUSY 1

#define DEVDRV_MAILBOX_VALID_MESSAGE   0
#define DEVDRV_MAILBOX_RECYCLE_MESSAGE 1

#define DEVDRV_MAILBOX_PAYLOAD_LENGTH 64

#define DEVDRV_MAILBOX_MESSAGE_VALID 0x5A5A

#define DEVDRV_DOORBEEL_TYPE 0

#define DEVDRV_MAILBOX_SYNC_MESSAGE  1
#define DEVDRV_MAILBOX_ASYNC_MESSAGE 2

#define DEVDRV_MAILBOX_SYNC          0
#define DEVDRV_MAILBOX_ASYNC         1

#define DEVDRV_MAILBOX_SEMA_TIMEOUT_SECOND 2

int devdrv_mailbox_init(u8 dev_id);

int devdrv_mailbox_message_send_for_res(u8 dev_id, const u8 *buf, u32 len, int *result);

int devdrv_mailbox_message_send(struct devdrv_mailbox *mailbox, struct devdrv_mailbox_message *message, int *result);

int devdrv_mailbox_message_send_ext(struct devdrv_mailbox *mailbox,
                                    struct devdrv_mailbox_message *message, int *result);

void devdrv_mailbox_exit(struct devdrv_mailbox *mailbox);

void devdrv_mailbox_recycle(struct devdrv_mailbox *mailbox);

void devdrv_mailbox_destroy(int dev_id);

#endif
