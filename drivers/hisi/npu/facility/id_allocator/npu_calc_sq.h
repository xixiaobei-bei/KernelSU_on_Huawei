/*
 * npu_calc_sq.h
 *
 * about npu calc sq
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
#ifndef _NPU_CALC_SQ_H
#define _NPU_CALC_SQ_H

#include <linux/list.h>
#include "npu_shm.h"

struct devdrv_sq_sub_info {
	u32 index;
	struct list_head list;
	u32 ref_by_streams;
	phys_addr_t phy_addr;
};

int devdrv_sq_list_init(u8 dev_id);

int devdrv_alloc_sq_id(u8 dev_id);

int devdrv_free_sq_id(u8 dev_id, u32 sq_id);

int devdrv_alloc_sq_mem(u8 dev_id, u32 sq_id);

int devdrv_get_sq_phy_addr(u8 dev_id, u32 sq_id, phys_addr_t *phy_addr);

int devdrv_free_sq_mem(u8 dev_id, u32 sq_id);

int devdrv_is_sq_ref_by_no_stream(u8 dev_id, u32 sq_id);

int devdrv_inc_sq_ref_by_stream(u8 dev_id, u32 sq_id);

int devdrv_dec_sq_ref_by_stream(u8 dev_id, u32 sq_id);

int devdrv_get_sq_send_count(u8 dev_id, u32 sq_id, u32 *send_count);

int devdrv_sq_list_destroy(u8 dev_id);

int devdrv_clr_sq_info(u8 dev_id, u32 sq_id);

#endif
