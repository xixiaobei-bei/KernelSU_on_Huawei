/*
 * npu_hwts_sq.h
 *
 * about npu hwts sq
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
#ifndef _NPU_HWTS_SQ_H
#define _NPU_HWTS_SQ_H

#include "devdrv_user_common.h"

int devdrv_hwts_sq_init(u8 dev_id);

int devdrv_hwts_sq_destroy(u8 dev_id);

int devdrv_alloc_hwts_sq(u8 dev_id);

int devdrv_free_hwts_sq(u8 dev_id, u32 hwts_sq_id);

#endif
