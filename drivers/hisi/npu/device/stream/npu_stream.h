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

struct devdrv_stream_sub_info {
	u32 id;
	struct list_head list;
	void *proc_ctx;  // struct devdrv_proc_ctx
};

int devdrv_stream_list_init(u8 dev_id);

int devdrv_alloc_stream_id(u8 dev_id);

int devdrv_free_stream_id(u8 dev_id, u32 stream_id);

int devdrv_bind_stream_with_sq(u8 dev_id, u32 stream_id, u32 sq_id);

int devdrv_bind_stream_with_cq(u8 dev_id, u32 stream_id, u32 cq_id);

int devdrv_stream_list_destroy(u8 dev_id);

#endif
