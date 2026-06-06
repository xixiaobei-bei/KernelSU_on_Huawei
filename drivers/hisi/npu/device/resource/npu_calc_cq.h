/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#ifndef _NPU_CALC_CQ_H
#define _NPU_CALC_CQ_H
#include <linux/list.h>

struct devdrv_cq_sub_info {
	u32 index;
	struct list_head list;
	void *proc_ctx;		// struct devdrv_proc_ctx
	spinlock_t spinlock;	/*
				 * use for avoid the problem:
				 * tasklet(devdrv_find_cq_index) may access cq's uio mem,
				 * there is a delay time, between set cq's uio invalid and accessing cq's uio mem by tasklet.
				 */
	phys_addr_t virt_addr;
	phys_addr_t phy_addr;
};

int devdrv_cq_list_init(u8 dev_id);

int devdrv_inc_cq_ref_by_stream(u8 dev_id, u32 cq_id);

int devdrv_dec_cq_ref_by_stream(u8 dev_id, u32 cq_id);

int devdrv_get_cq_ref_by_stream(u8 dev_id, u32 cq_id);

int devdrv_alloc_cq_id(u8 dev_id);

int devdrv_free_cq_id(u8 dev_id, u32 cq_id);

int devdrv_alloc_cq_mem(u8 dev_id, u32 cq_id);

int devdrv_get_cq_phy_addr(u8 dev_id, u32 cq_id, phys_addr_t *phy_addr);

int devdrv_free_cq_mem(u8 dev_id, u32 cq_id);

int devdrv_cq_list_destroy(u8 dev_id);

int devdrv_clr_cq_info(u8 dev_id, u32 cq_id);

#endif
