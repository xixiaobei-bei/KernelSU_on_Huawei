/*
 * npu_heart_beat.h
 *
 * about npu heart beat
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
#ifndef __NPU_HEART_BEAT_H
#define __NPU_HEART_BEAT_H

#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/uio_driver.h>
#include <linux/notifier.h>
#include <linux/radix-tree.h>

#include <linux/hisi/rdr_pub.h>

#include "npu_common.h"

#define DEVDRV_HEART_BEAT_SQ_CMD       0xAABBCCDD
#define DEVDRV_HEART_BEAT_CYCLE        30     /* bbox 30 second */
#define DEVDRV_HEART_BEAT_TIMEOUT      (DEVDRV_HEART_BEAT_CYCLE * 2)     /* second */
#define DEVDRV_HEART_BEAT_THRESHOLD    3
#define DEVDRV_HEART_BEAT_MAX_QUEUE    20

struct devdrv_pm {
	int (*suspend)(u32 devid, u32 status);
	int (*resume)(u32 devid);
	struct list_head list;
};

struct devdrv_manager_msg_head {
	u32 dev_id;
	u32 msg_id;
	u16 valid;  /* validity judgement, 0x5A5A is valide */
	u16 result; /* process result from rp, zero for succ, non zero for fail */
};

#define DEVDRV_MANAGER_INFO_LEN  128
#define DEVDRV_MANAGER_INFO_PAYLOAD_LEN  (DEVDRV_MANAGER_INFO_LEN - sizeof(struct devdrv_manager_msg_head))

int devdrv_heart_beat_init(struct devdrv_dev_ctx *dev_ctx);

void devdrv_heart_beat_exit(struct devdrv_dev_ctx *dev_ctx);

int devdrv_heart_beat_resource_init(struct devdrv_dev_ctx *dev_ctx);

void devdrv_heart_beat_resource_destroy(struct devdrv_dev_ctx *dev_ctx);

extern struct dsm_client *davinci_dsm_client;

#endif /* __NPU_HEART_BEAT_H */
