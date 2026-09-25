/*
 * npu_calc_channel.h
 *
 * about npu calc channel
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
#ifndef __NPU_CALC_CHANNEL_H
#define __NPU_CALC_CHANNEL_H

#include <linux/list.h>

struct devdrv_ts_cq_info *devdrv_alloc_cq(u8 dev_id);

int devdrv_send_alloc_stream_mailbox(u8 cur_dev_id, int stream_id, int cq_id);

struct devdrv_stream_info* devdrv_alloc_stream(u32 cq_id, u32 strategy);

int devdrv_free_stream(u8 dev_id, u32 stream_id, u32 *sq_send_count);

#endif /* __NPU_CALC_CHANNEL_H */
