/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
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
