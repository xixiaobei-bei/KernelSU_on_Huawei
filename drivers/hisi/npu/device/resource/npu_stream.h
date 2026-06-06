/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#ifndef __NPU_STREAM_H
#define __NPU_STREAM_H

#include <linux/list.h>

struct devdrv_stream_sub_info {
	u32 id;
	struct list_head list;
	void *proc_ctx;		// struct devdrv_proc_ctx
};

int devdrv_stream_list_init(u8 dev_id);

int devdrv_alloc_stream_id(u8 dev_id);

int devdrv_free_stream_id(u8 dev_id, u32 stream_id);

int devdrv_bind_stream_with_sq(u8 dev_id, u32 stream_id, u32 sq_id);

int devdrv_bind_stream_with_cq(u8 dev_id, u32 stream_id, u32 cq_id);

int devdrv_stream_list_destroy(u8 dev_id);

#endif
