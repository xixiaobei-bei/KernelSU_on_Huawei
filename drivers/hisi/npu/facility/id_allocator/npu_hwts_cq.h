/*
 * npu_hwts_cq.h
 *
 * about npu hwts cq
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
#ifndef _NPU_HWTS_CQ_H
#define _NPU_HWTS_CQ_H

#include "npu_user_common.h"

int npu_hwts_cq_init(u8 dev_id);

int npu_hwts_cq_destroy(u8 dev_id);

int npu_alloc_hwts_cq(u8 dev_id);

int npu_free_hwts_cq(u8 dev_id, u32 hwts_cq_id);

#endif
