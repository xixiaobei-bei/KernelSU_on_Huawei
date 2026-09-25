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

int devdrv_sink_stream_list_init(u8 dev_id);

int devdrv_alloc_sink_stream_id(u8 dev_id);

int devdrv_free_sink_stream_id(u8 dev_id, u32 stream_id);

int devdrv_sink_stream_list_destroy(u8 dev_id);

#endif
