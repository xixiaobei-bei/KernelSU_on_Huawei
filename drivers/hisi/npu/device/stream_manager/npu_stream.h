/*
 * npu_stream.h
 *
 * about npu stream
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
#ifndef __NPU_STREAM_H
#define __NPU_STREAM_H

#include <linux/list.h>
#include "npu_id_allocator.h"

struct npu_stream_sub_extra_info {
	wait_queue_head_t sync_report_wait;
};

int npu_stream_list_init(struct npu_dev_ctx *dev_ctx);

int npu_alloc_stream_id(u8 dev_id);

int npu_free_stream_id(u8 dev_id, u32 stream_id);

int npu_bind_stream_with_sq(u8 dev_id, u32 stream_id, u32 sq_id);

int npu_bind_stream_with_cq(u8 dev_id, u32 stream_id, u32 cq_id);

struct npu_id_entity *npu_get_non_sink_stream_sub_addr(struct npu_dev_ctx *dev_ctx, u32 stream_id);

#endif
