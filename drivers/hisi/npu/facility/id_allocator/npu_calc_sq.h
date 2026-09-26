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

#include "npu_common.h"

struct npu_sq_sub_info {
	u32 index;
	struct list_head list;
	u32 ref_by_streams;
	phys_addr_t phy_addr;
	vir_addr_t virt_addr;
};

int npu_sq_list_init(u8 dev_id);

int npu_sq_list_destroy(u8 dev_id);

int npu_get_sq_phy_addr(u8 dev_id, u32 sq_id, phys_addr_t *phy_addr);

int npu_get_sq_ref_by_stream(u8 dev_id, u32 sq_id);

int npu_inc_sq_ref_by_stream(u8 dev_id, u32 sq_id);

int npu_dec_sq_ref_by_stream(u8 dev_id, u32 sq_id);

int npu_get_sq_send_count(u8 dev_id, u32 sq_id, u32 *send_count);

int npu_clear_sq_info(u8 dev_id, u32 sq_id);

int npu_alloc_sq(u8 dev_id, int pid);

int npu_free_sq(u8 dev_id, u32 sq_id);

struct npu_sq_sub_info *npu_get_sq_sub_addr(
	struct npu_dev_ctx *cur_dev_ctx, u32 sq_index);

#endif
