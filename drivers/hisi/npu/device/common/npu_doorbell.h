/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
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

#define DOORBELL_MAILBOX_VALUE (0x3A)
#define DOORBELL_MAILBOX_MAX_SIZE (1)

#define DOORBELL_MAX_SIZE	(128)

int devdrv_write_doorbell_val(u32 type, u32 index, u32 val);
int devdrv_dev_doorbell_init(void);
int devdrv_get_doorbell_vaddr(u32 type, u32 index, u32 **doorbell_vaddr);

#endif
