/*
 * npu_manager_common.c
 *
 * about npu manager common
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
#ifndef __NPU_MANAGER_COMMON_H
#define __NPU_MANAGER_COMMON_H

#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/uio_driver.h>
#include <linux/notifier.h>
#include <linux/radix-tree.h>

#include "devdrv_user_common.h"

#define DEVDRV_CALLBACK_REG_NUM  10

struct devdrv_reg_callbak_info {
	int (*op_callback[DEVDRV_CALLBACK_REG_NUM])(unsigned int dev_id); // power up callback
	struct mutex op_mutex;
	int (*rs_callback[DEVDRV_CALLBACK_REG_NUM])(unsigned int dev_id); // power down callback
	struct mutex rs_mutex;
};

void devdrv_register_callbak_info_init(void);

int devdrv_open_npu_callback_reg(int (*callback)(unsigned int dev_id), unsigned int *callback_idx);

int devdrv_open_npu_callback_dereg(unsigned int callback_idx);

void devdrv_open_npu_callback_proc(unsigned int dev_id);

int devdrv_release_npu_callback_reg(int (*callback)(unsigned int dev_id), unsigned int *callback_idx);

int devdrv_release_npu_callback_dereg(unsigned int callback_idx);

void devdrv_release_npu_callback_proc(unsigned int dev_id);

#define DEVDRV_MANAGER_MSG_VALID  0x5A5A
#define DEVDRV_MANAGER_MSG_INVALID_RESULT    0x1A

enum {
	DEVDRV_MANAGER_CHAN_H2D_SEND_DEVID,
	DEVDRV_MANAGER_CHAN_D2H_DEVICE_READY,
	DEVDRV_MANAGER_CHAN_D2H_DEVICE_READY_AND_TS_WORK,
	DEVDRV_MANAGER_CHAN_D2H_DOWN,
	DEVDRV_MANAGER_CHAN_D2H_SUSNPEND,
	DEVDRV_MANAGER_CHAN_D2H_RESUME,
	DEVDRV_MANAGER_CHAN_D2H_FAIL_TO_SUSPEND,
	DEVDRV_MANAGER_CHAN_D2H_CORE_INFO,
	DEVDRV_MANAGER_CHAN_H2D_HEART_BEAT,
	DEVDRV_MANAGER_CHAN_D2H_GET_PCIE_ID_INFO,
	DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_STREAM,
	DEVDRV_MANAGER_CHAN_D2H_SYNC_GET_STREAM,
	DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_EVENT,
	DEVDRV_MANAGER_CHAN_D2H_SYNC_GET_EVENT,
	DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_SQ,
	DEVDRV_MANAGER_CHAN_D2H_SYNC_GET_SQ,
	DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_CQ,
	DEVDRV_MANAGER_CHAN_D2H_SYNC_GET_CQ,
	DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_MODEL,
	DEVDRV_MANAGER_CHAN_D2H_SYNC_GET_MODEL,
	DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_NOTIFY,
	DEVDRV_MANAGER_CHAN_D2H_SYNC_GET_NOTIFY,
	DEVDRV_MANAGER_CHAN_H2D_SYNC_GET_DEVINFO,
	DEVDRV_MANAGER_CHAN_H2D_CONTAINER,
	DEVDRV_MANAGER_CHAN_H2D_GET_TASK_STATUS,
	DEVDRV_MANAGER_CHAN_H2D_LOAD_KERNEL,
	DEVDRV_MANAGER_CHAN_D2H_LOAD_KERNEL_RESULT,
	DEVDRV_MANAGER_CHAN_MAX_ID
};

static inline u64 devdrv_read_cntpct(void)
{
	u64 cntpct = 0;
	asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));
	return cntpct;
}

void devdrv_register_callbak_info_init(void);

int devdrv_open_npu_callback_reg(int (*callback)(unsigned int dev_id), unsigned int *callback_idx);

int devdrv_open_npu_callback_dereg(unsigned int callback_idx);

void devdrv_open_npu_callback_proc(unsigned int dev_id);

int devdrv_release_npu_callback_reg(int (*callback)(unsigned int dev_id), unsigned int *callback_idx);

int devdrv_release_npu_callback_dereg(unsigned int callback_idx);

void devdrv_release_npu_callback_proc(unsigned int dev_id);

#endif /* __NPU_MANAGER_COMMON_H */
