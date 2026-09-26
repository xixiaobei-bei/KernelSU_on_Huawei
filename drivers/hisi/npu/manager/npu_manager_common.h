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

#include "npu_user_common.h"

#define NPU_CALLBACK_REG_NUM  10

struct npu_reg_callbak_info {
	// power up callback
	int (*op_callback[NPU_CALLBACK_REG_NUM])(unsigned int dev_id);
	struct mutex op_mutex;
	// power down callback
	int (*rs_callback[NPU_CALLBACK_REG_NUM])(unsigned int dev_id);
	struct mutex rs_mutex;
};

void npu_register_callbak_info_init(void);

int npu_open_npu_callback_reg(int (*callback)(unsigned int dev_id),
	unsigned int *callback_idx);

int npu_open_npu_callback_dereg(unsigned int callback_idx);

void npu_open_npu_callback_proc(unsigned int dev_id);

int npu_release_npu_callback_reg(int (*callback)(unsigned int dev_id),
	unsigned int *callback_idx);

int npu_release_npu_callback_dereg(unsigned int callback_idx);

void npu_release_npu_callback_proc(unsigned int dev_id);

#define NPU_MANAGER_MSG_VALID  0x5A5A
#define NPU_MANAGER_MSG_INVALID_RESULT    0x1A

enum {
	NPU_MANAGER_CHAN_H2D_SEND_DEVID,
	NPU_MANAGER_CHAN_D2H_DEVICE_READY,
	NPU_MANAGER_CHAN_D2H_DEVICE_READY_AND_TS_WORK,
	NPU_MANAGER_CHAN_D2H_DOWN,
	NPU_MANAGER_CHAN_D2H_SUSNPEND,
	NPU_MANAGER_CHAN_D2H_RESUME,
	NPU_MANAGER_CHAN_D2H_FAIL_TO_SUSPEND,
	NPU_MANAGER_CHAN_D2H_CORE_INFO,
	NPU_MANAGER_CHAN_H2D_HEART_BEAT,
	NPU_MANAGER_CHAN_D2H_GET_PCIE_ID_INFO,
	NPU_MANAGER_CHAN_H2D_SYNC_GET_STREAM,
	NPU_MANAGER_CHAN_D2H_SYNC_GET_STREAM,
	NPU_MANAGER_CHAN_H2D_SYNC_GET_EVENT,
	NPU_MANAGER_CHAN_D2H_SYNC_GET_EVENT,
	NPU_MANAGER_CHAN_H2D_SYNC_GET_SQ,
	NPU_MANAGER_CHAN_D2H_SYNC_GET_SQ,
	NPU_MANAGER_CHAN_H2D_SYNC_GET_CQ,
	NPU_MANAGER_CHAN_D2H_SYNC_GET_CQ,
	NPU_MANAGER_CHAN_H2D_SYNC_GET_MODEL,
	NPU_MANAGER_CHAN_D2H_SYNC_GET_MODEL,
	NPU_MANAGER_CHAN_H2D_SYNC_GET_NOTIFY,
	NPU_MANAGER_CHAN_D2H_SYNC_GET_NOTIFY,
	NPU_MANAGER_CHAN_H2D_SYNC_GET_DEVINFO,
	NPU_MANAGER_CHAN_H2D_CONTAINER,
	NPU_MANAGER_CHAN_H2D_GET_TASK_STATUS,
	NPU_MANAGER_CHAN_H2D_LOAD_KERNEL,
	NPU_MANAGER_CHAN_D2H_LOAD_KERNEL_RESULT,
	NPU_MANAGER_CHAN_MAX_ID
};

static inline u64 npu_read_cntpct(void)
{
	u64 cntpct = 0;

	asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));
	return cntpct;
}

void npu_register_callbak_info_init(void);

int npu_open_npu_callback_reg(int (*callback)(unsigned int dev_id),
	unsigned int *callback_idx);

int npu_open_npu_callback_dereg(unsigned int callback_idx);

void npu_open_npu_callback_proc(unsigned int dev_id);

int npu_release_npu_callback_reg(int (*callback)(unsigned int dev_id),
	unsigned int *callback_idx);

int npu_release_npu_callback_dereg(unsigned int callback_idx);

void npu_release_npu_callback_proc(unsigned int dev_id);

#endif /* __NPU_MANAGER_COMMON_H */
