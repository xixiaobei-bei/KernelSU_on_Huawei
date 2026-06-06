/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/vmalloc.h>

#include "devdrv_user_common.h"
#include "npu_calc_sq.h"
#include "npu_common.h"
#include "npu_shm.h"
#include "drv_log.h"

int devdrv_sq_list_init(u8 dev_id)
{
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct devdrv_sq_sub_info *sq_sub_info = NULL;
	struct devdrv_ts_sq_info *sq_info = NULL;
	unsigned long size;
	u32 num_sq = DEVDRV_MAX_SQ_NUM;	// need get from platform
	u32 i;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	INIT_LIST_HEAD(&cur_dev_ctx->sq_available_list);
	if (!list_empty_careful(&cur_dev_ctx->sq_available_list)) {
		devdrv_drv_err("sq_available_list is not empty.\n");
		return -1;
	}

	cur_dev_ctx->sq_num = 0;
	size = (long)(unsigned)sizeof(struct devdrv_sq_sub_info) * num_sq;
	sq_sub_info = vmalloc(size);
	if (sq_sub_info == NULL) {
		devdrv_drv_err("no mem to alloc sq sub info list.\n");
		return -ENOMEM;
	}

	cur_dev_ctx->sq_sub_addr = (void *)sq_sub_info;

	for (i = 0; i < num_sq; i++) {
		sq_info = devdrv_calc_sq_info(dev_id, i);
		sq_info->head = 0;
		sq_info->tail = 0;
		sq_info->credit = DEVDRV_MAX_SQ_DEPTH - 1;
		sq_info->index = i;
		sq_info->uio_addr = NULL;
		sq_info->uio_num = DEVDRV_INVALID_FD_OR_NUM;
		sq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
		sq_info->uio_size = DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE;
		sq_info->stream_num = 0;
		sq_info->send_count = 0;

		sq_info->sq_sub = (void *)(sq_sub_info + i);
		sq_sub_info[i].index = sq_info->index;
		sq_sub_info[i].ref_by_streams = 0;

		list_add_tail(&sq_sub_info[i].list,
			      &cur_dev_ctx->sq_available_list);
		cur_dev_ctx->sq_num++;
	}
	devdrv_drv_debug("cur dev %d own %d calc sq \n", dev_id,
			 cur_dev_ctx->sq_num);

	return 0;
}

int devdrv_alloc_sq_id(u8 dev_id)
{
	struct devdrv_sq_sub_info *sq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	if (list_empty_careful(&cur_dev_ctx->sq_available_list)) {
		spin_unlock(&cur_dev_ctx->spinlock);
		devdrv_drv_err("cur dev %d available sq list empty,"
			       "left sq_num = %d !!!\n", dev_id,
			       cur_dev_ctx->sq_num);
		return -1;
	}
	sq_sub = list_first_entry(&cur_dev_ctx->sq_available_list,
				  struct devdrv_sq_sub_info, list);
	list_del(&sq_sub->list);
	cur_dev_ctx->sq_num--;
	spin_unlock(&cur_dev_ctx->spinlock);
	devdrv_drv_debug("cur dev %d left %d sq\n", dev_id,
			 cur_dev_ctx->sq_num);

	return sq_sub->index;
}

int devdrv_get_sq_send_count(u8 dev_id, u32 sq_id, u32 *send_count)
{
	struct devdrv_ts_sq_info *sq_info = NULL;
	// struct devdrv_sq_sub_info *sq_sub;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (sq_id >= DEVDRV_MAX_SQ_NUM) {
		devdrv_drv_err("illegal npu sq id\n");
		return -1;
	}

	if (send_count == NULL) {
		devdrv_drv_err("send_count is null ptr\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	sq_info = devdrv_calc_sq_info(dev_id, sq_id);
	*send_count = sq_info->send_count;

	return 0;
}

int devdrv_is_sq_ref_by_no_stream(u8 dev_id, u32 sq_id)
{
	struct devdrv_ts_sq_info *sq_info = NULL;
	struct devdrv_sq_sub_info *sq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (sq_id >= DEVDRV_MAX_SQ_NUM) {
		devdrv_drv_err("illegal npu sq id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	sq_info = devdrv_calc_sq_info(dev_id, sq_id);
	sq_sub = (struct devdrv_sq_sub_info *)sq_info->sq_sub;

	if (sq_sub == NULL) {
		devdrv_drv_debug("sq_sub is null.\n");
		return -1;
	}

	if (sq_sub->ref_by_streams != 0) {
		devdrv_drv_debug("can't release cur_dev_ctx %d "
		     "sq calc channel %d for ref_by_streams = %d\n",
			 dev_id, sq_id, sq_sub->ref_by_streams);
		return 0;
	}

	return -1;
}

// sq_sub->ref_by_streams-- excute by service layer
int devdrv_free_sq_id(u8 dev_id, u32 sq_id)
{
	struct devdrv_ts_sq_info *sq_info = NULL;
	struct devdrv_sq_sub_info *sq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (sq_id >= DEVDRV_MAX_SQ_NUM) {
		devdrv_drv_err("illegal npu sq id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	sq_info = devdrv_calc_sq_info(dev_id, sq_id);
	sq_sub = (struct devdrv_sq_sub_info *)sq_info->sq_sub;

	if (sq_sub == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		devdrv_drv_debug("sq_sub is null.\n");
		return -1;
	}

	if (sq_sub->ref_by_streams != 0) {
		devdrv_drv_debug("can't release cur_dev_ctx %d "
		     "sq calc channel %d for ref_by_streams = %d\n",
			 dev_id, sq_id, sq_sub->ref_by_streams);
		spin_unlock(&cur_dev_ctx->spinlock);
		return -1;
	}
	list_add(&sq_sub->list, &cur_dev_ctx->sq_available_list);
	// no stream use it
	sq_sub->ref_by_streams = 0;
	cur_dev_ctx->sq_num++;
	sq_info->head = 0;
	sq_info->tail = 0;
	sq_info->credit = DEVDRV_MAX_SQ_DEPTH - 1;
	sq_info->stream_num = 0;
	sq_info->send_count = 0;
	spin_unlock(&cur_dev_ctx->spinlock);
	devdrv_drv_debug("cur dev %d own %d sq\n", dev_id, cur_dev_ctx->sq_num);

	return 0;
}

int devdrv_alloc_sq_mem(u8 dev_id, u32 sq_id)
{
	struct devdrv_ts_sq_info *sq_info = NULL;
	struct devdrv_sq_sub_info *sq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	phys_addr_t phy_addr = 0;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (sq_id >= DEVDRV_MAX_SQ_NUM) {
		devdrv_drv_err("illegal npu sq id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	phy_addr = (unsigned long long)(g_sq_desc->base +
			 (sq_id * DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE));	//lint !e647

	spin_lock(&cur_dev_ctx->spinlock);
	sq_info = devdrv_calc_sq_info(dev_id, sq_id);
	sq_sub = (struct devdrv_sq_sub_info *)sq_info->sq_sub;
	if (sq_sub == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		devdrv_drv_debug("sq_sub is null.\n");
		return -1;
	}
	sq_sub->phy_addr = phy_addr;
	spin_unlock(&cur_dev_ctx->spinlock);

	devdrv_drv_debug("dev %d cur sq %d phy_addr = %pK \n",
			 dev_id, sq_id, (void *)(uintptr_t) phy_addr);

	return 0;
}

// get sq_id sq`s sq_addr from dev_id(must called after alloc_sq_mem)
int devdrv_get_sq_phy_addr(u8 dev_id, u32 sq_id, phys_addr_t *phy_addr)
{
	struct devdrv_ts_sq_info *sq_info = NULL;
	struct devdrv_sq_sub_info *sq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (sq_id >= DEVDRV_MAX_SQ_NUM) {
		devdrv_drv_err("illegal npu sq id\n");
		return -1;
	}

	if (phy_addr == NULL) {
		devdrv_drv_err("phy_addr is null\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	sq_info = devdrv_calc_sq_info(dev_id, sq_id);
	sq_sub = (struct devdrv_sq_sub_info *)sq_info->sq_sub;
	if (sq_sub == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		devdrv_drv_debug("sq_sub is null.\n");
		return -1;
	}
	*phy_addr = sq_sub->phy_addr;
	spin_unlock(&cur_dev_ctx->spinlock);

	devdrv_drv_debug("dev %d cur sq %d phy_addr = %pK \n",
			 dev_id, sq_id, (void *)(*phy_addr));

	return 0;
}

int devdrv_clr_sq_info(u8 dev_id, u32 sq_id)
{
	struct devdrv_ts_sq_info *sq_info;
	struct devdrv_dev_ctx *cur_dev_ctx;

	if (dev_id >= NPU_DEV_NUM)
	{
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	sq_info = devdrv_calc_sq_info(dev_id, sq_id);
	sq_info->head = 0;
	sq_info->tail = 0;
	sq_info->credit = DEVDRV_MAX_SQ_DEPTH - 1;
	spin_unlock(&cur_dev_ctx->spinlock);

	return 0;
}

int devdrv_free_sq_mem(u8 dev_id, u32 sq_id)
{
	struct devdrv_sq_sub_info *sq_sub = NULL;
	struct devdrv_ts_sq_info *sq_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (sq_id >= DEVDRV_MAX_SQ_NUM) {
		devdrv_drv_err("illegal npu sq id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	sq_info = devdrv_calc_sq_info(dev_id, sq_id);
	sq_info->uio_addr = NULL;
	sq_info->uio_num = DEVDRV_INVALID_FD_OR_NUM;
	sq_info->uio_map = DEVDRV_SQ_CQ_MAP;
	sq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
	sq_sub = (struct devdrv_sq_sub_info *)sq_info->sq_sub;
	if (sq_sub == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		devdrv_drv_debug("sq_sub is null.\n");
		return -1;
	}
	sq_sub->phy_addr = 0;
	spin_unlock(&cur_dev_ctx->spinlock);

	return 0;
}

// called by alloc stream in service layer
int devdrv_inc_sq_ref_by_stream(u8 dev_id, u32 sq_id)
{
	struct devdrv_ts_sq_info *sq_info = NULL;
	struct devdrv_sq_sub_info *sq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	sq_info = devdrv_calc_sq_info(dev_id, sq_id);
	sq_sub = (struct devdrv_sq_sub_info *)sq_info->sq_sub;
	if (sq_sub == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		devdrv_drv_debug("sq_sub is null.\n");
		return -1;
	}
	sq_info->stream_num++;	// should do it here or user driver
	sq_sub->ref_by_streams++;
	spin_unlock(&cur_dev_ctx->spinlock);

	return 0;
}

// called by free stream in service layer
int devdrv_dec_sq_ref_by_stream(u8 dev_id, u32 sq_id)
{
	struct devdrv_ts_sq_info *sq_info = NULL;
	struct devdrv_sq_sub_info *sq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	sq_info = devdrv_calc_sq_info(dev_id, sq_id);
	sq_sub = (struct devdrv_sq_sub_info *)sq_info->sq_sub;
	if (sq_sub == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		devdrv_drv_debug("sq_sub is null.\n");
		return -1;
	}
	sq_info->stream_num--;	// should do it here or user driver
	sq_sub->ref_by_streams--;
	spin_unlock(&cur_dev_ctx->spinlock);

	return 0;
}

int devdrv_sq_list_destroy(u8 dev_id)
{
	struct devdrv_sq_sub_info *sq_sub_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	if (!list_empty_careful(&cur_dev_ctx->sq_available_list)) {
		list_for_each_safe(pos, n, &cur_dev_ctx->sq_available_list) {
			cur_dev_ctx->sq_num--;
			sq_sub_info =
			    list_entry(pos, struct devdrv_sq_sub_info, list);
			list_del(pos);
		}
	}

	vfree(cur_dev_ctx->sq_sub_addr);
	cur_dev_ctx->sq_sub_addr = NULL;

	return 0;
}
