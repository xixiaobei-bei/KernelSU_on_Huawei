/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2019-04-29
 */
#ifndef __NPU_SINK_STREAM_H
#define __NPU_SINK_STREAM_H

#include <linux/list.h>

int devdrv_sink_stream_list_init(u8 dev_id);

int devdrv_alloc_sink_stream_id(u8 dev_id);

int devdrv_free_sink_stream_id(u8 dev_id, u32 stream_id);

int devdrv_sink_stream_list_destroy(u8 dev_id);

#endif
