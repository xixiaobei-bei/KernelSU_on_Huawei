/*
 * npu_mailbox.h
 *
 * about npu mailbox
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#define NPU_MAILBOX_SRAM 0

#define NPU_MAILBOX_FREE 0
#define NPU_MAILBOX_BUSY 1

#define NPU_MAILBOX_VALID_MESSAGE   0
#define NPU_MAILBOX_RECYCLE_MESSAGE 1

#define NPU_MAILBOX_PAYLOAD_LENGTH 64

#define NPU_MAILBOX_MESSAGE_VALID 0x5A5A

#define NPU_DOORBEEL_TYPE 0

#define NPU_MAILBOX_SYNC_MESSAGE  1
#define NPU_MAILBOX_ASYNC_MESSAGE 2

#define NPU_MAILBOX_SYNC          0
#define NPU_MAILBOX_ASYNC         1

#ifdef CONFIG_NPU_FPGA_PLAT
#define NPU_MAILBOX_SEND_TIMEOUT_SECOND 5000
#else
#define NPU_MAILBOX_SEND_TIMEOUT_SECOND 200
#endif

int npu_mailbox_init(u8 dev_id);

int npu_mailbox_message_send_for_res(u8 dev_id, const u8 *buf, u32 len,
	int *result);

int npu_mailbox_message_send_ext(struct npu_mailbox *mailbox,
	struct npu_mailbox_message *message, int *result);

void npu_mailbox_exit(struct npu_mailbox *mailbox);

void npu_mailbox_recycle(struct npu_mailbox *mailbox);

void npu_mailbox_destroy(int dev_id);

#endif
