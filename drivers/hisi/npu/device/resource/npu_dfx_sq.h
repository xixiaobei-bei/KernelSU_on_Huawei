/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#ifndef __NPU_DFX_SQ_H
#define __NPU_DFX_SQ_H
#include "npu_common.h"

#define DEVDRV_MAX_DFX_SQ_DEPTH             (256)

enum dfx_cqsq_func {
	DEVDRV_CQSQ_HEART_BEAT = 0x0,
	DEVDRV_MAX_CQSQ_FUNC,
};

struct devdrv_dfx_create_sq_para {
	u32 slot_len;
	u32 sq_index;
	u64 *addr;
	enum dfx_cqsq_func function;
};

int devdrv_create_dfx_sq(struct devdrv_dev_ctx *cur_dev_ctx,
			 struct devdrv_dfx_create_sq_para *sq_para);
int devdrv_dfx_send_sq(u32 devid, u32 sq_index, const u8 *buffer, u32 buf_len);

#endif
