/*
 * npu_doorbell.h
 *
 * about npu doorbell
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
#ifndef __NPU_DOORBELL_H
#define __NPU_DOORBELL_H

#include <linux/types.h>

enum res_type {
	DOORBELL_RES_CAL_SQ,
	DOORBELL_RES_CAL_CQ,
	DOORBELL_RES_DFX_SQ,
	DOORBELL_RES_DFX_CQ,
	DOORBELL_RES_MAILBOX,
	DOORBELL_RES_RESERVED
};

#define DOORBELL_MAILBOX_VALUE 0x3A
#define DOORBELL_MAILBOX_MAX_SIZE 1

#define DOORBELL_MAX_SIZE  128

int npu_write_doorbell_val(u32 type, u32 index, u32 val);
int npu_dev_doorbell_init(void);
int npu_get_doorbell_vaddr(u32 type, u32 index, u32 **doorbell_vaddr);

#endif
