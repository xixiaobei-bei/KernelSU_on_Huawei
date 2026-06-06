/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#ifndef __NPU_EVENT_H
#define __NPU_EVENT_H

#include <linux/types.h>

int devdrv_event_list_init(u8 dev_id);

struct devdrv_event_info *devdrv_alloc_event(u8 dev_id);

int devdrv_free_event_id(u8 dev_id, u32 event_id);

int devdrv_event_list_destroy(u8 dev_id);

int devdrv_event_software_ops_register(u8 dev_id);

#endif
