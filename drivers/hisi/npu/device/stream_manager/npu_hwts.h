/*
 * npu_hwts.h
 *
 * about npu hwts
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
#ifndef __NPU_HWTS_H
#define __NPU_HWTS_H

#include "npu_hwts_sq.h"
#include "npu_hwts_cq.h"

// used for bindin dev_ctx with hwts cq/sq and othersome
struct npu_dev_ctx_sub {
	u8 devid;
	u8 resv[7];
	// npu open mutex
	struct mutex hwts_cq_mutex_t;
};

struct npu_proc_ctx_sub {
	pid_t pid;
	u8 devid;
	u8 resv[3];
	u32 hwts_cq_num;
	struct list_head hwts_cq_list;
};

struct npu_proc_ctx;

int npu_dev_ctx_sub_init(u8 dev_id);

void npu_dev_ctx_sub_destroy(u8 dev_id);

int npu_proc_ctx_sub_init(struct npu_proc_ctx *proc_ctx);

void npu_proc_ctx_sub_destroy(struct npu_proc_ctx *proc_ctx);

int npu_proc_alloc_proc_ctx_sub(struct npu_proc_ctx *proc_ctx);

int npu_proc_free_proc_ctx_sub(struct npu_proc_ctx *proc_ctx);

void npu_recycle_proc_ctx_sub(struct npu_proc_ctx *proc_ctx);

int npu_proc_get_hwts_cq_id(struct npu_proc_ctx *proc_ctx,
	u32 *hwts_cq_id);

struct npu_hwts_cq_info *npu_proc_alloc_hwts_cq(
	struct npu_proc_ctx *proc_ctx);

int npu_proc_free_hwts_cq(struct npu_proc_ctx *proc_ctx);

int npu_bind_stream_with_hwts_sq(u8 dev_id, u32 stream_id, u32 hwts_sq_id);

int npu_bind_stream_with_hwts_cq(u8 dev_id, u32 stream_id, u32 hwts_cq_id);

#endif
