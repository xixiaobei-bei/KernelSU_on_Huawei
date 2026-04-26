/*
 * npu_dfx_sq.h
 *
 * about npu dfx sq
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
#ifndef __NPU_DFX_SQ_H
#define __NPU_DFX_SQ_H

#include "npu_common.h"

enum dfx_cqsq_func_type {
	NPU_CQSQ_HEART_BEAT = 0x0,
	NPU_MAX_CQSQ_FUNC
};

struct npu_dfx_create_sq_para {
	u32 slot_len;
	u32 sq_index;
	u64 *addr;
	enum dfx_cqsq_func_type function;
};

int npu_create_dfx_sq(struct npu_dev_ctx *cur_dev_ctx,
	struct npu_dfx_create_sq_para *sq_para);

int npu_dfx_send_sq(u32 devid, u32 sq_index, const u8 *buffer, u32 buf_len);

#endif
