/*
 * npu_event.h
 *
 * about npu event
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
#ifndef __NPU_EVENT_H
#define __NPU_EVENT_H

#include <linux/types.h>

int devdrv_event_list_init(u8 dev_id);

struct devdrv_event_info *devdrv_alloc_event(u8 dev_id);

int devdrv_free_event_id(u8 dev_id, u32 event_id);

int devdrv_event_list_destroy(u8 dev_id);

int devdrv_event_software_ops_register(u8 dev_id);

#endif
