/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#ifndef __NPU_CALC_CHANNEL_H
#define __NPU_CALC_CHANNEL_H

struct devdrv_ts_cq_info *devdrv_alloc_cq(u8 dev_id);

int devdrv_send_alloc_stream_mailbox(u8 cur_dev_id, int stream_id, int cq_id);

struct devdrv_stream_info* devdrv_alloc_stream(u32 cq_id, u32 strategy);

int devdrv_free_stream(u8 dev_id, u32 stream_id, u32 *sq_send_count);
#endif /* __NPU_CALC_CHANNEL_H */
