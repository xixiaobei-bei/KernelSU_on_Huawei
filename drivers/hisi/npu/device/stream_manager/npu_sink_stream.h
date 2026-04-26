/*
 * npu_sink_stream.h
 *
 * about npu sink stream
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
#ifndef __NPU_SINK_STREAM_H
#define __NPU_SINK_STREAM_H

#include <linux/list.h>
#include "npu_id_allocator.h"

int npu_sink_stream_list_init(struct npu_dev_ctx *dev_ctx);

int npu_alloc_sink_stream_id(u8 dev_id, u8 is_long);

int npu_free_sink_stream_id(u8 dev_id, u32 stream_id);

struct npu_id_entity *npu_get_sink_stream_sub_addr(
	struct npu_dev_ctx *dev_ctx, u32 stream_id);

#endif
