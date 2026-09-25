/*
 * npu_notify.h
 *
 * about npu notify
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
#ifndef __NPU_NOTIFY_H
#define __NPU_NOTIFY_H

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include "npu_mailbox.h"

#define DEVDRV_NOTIFY_INFORM_TS 0
#define DEVDRV_NOTIFY_NOT_INFORM_TS 1
#define DEVDRV_MAILBOX_RESET_NOTIFY_ID 0x0010

struct devdrv_notify_info {
	int id;
	u32 devid;
	struct list_head list;
	spinlock_t spinlock;
};

struct devdrv_ts_notify_msg {
	struct devdrv_mailbox_message_header header;
	int notify_id;
	u16 resv[25];
	u8 reserved[3];
};

int devdrv_notify_list_init(u8 dev_ctx_id);

int devdrv_alloc_notify_id(int dev_ctx_id, unsigned long arg);

int devdrv_free_notify_id(int dev_ctx_id, unsigned long arg);

int devdrv_notify_list_destroy(int dev_ctx_id);

int devdrv_notify_software_register(int dev_ctx_id);

#endif
