/*
 * npu_calc_cq.c
 *
 * about npu calc cq
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
#include "npu_calc_cq.h"

#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

#include "devdrv_user_common.h"
#include "npu_common.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_doorbell.h"
#include "npu_proc_ctx.h"

int devdrv_cq_list_init(u8 dev_id)
{
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct devdrv_cq_sub_info *cq_sub_info = NULL;
	struct devdrv_ts_cq_info *cq_info = NULL;
	unsigned long size;
	u32 i;

	COND_RETURN_ERROR(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id\n");

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	COND_RETURN_ERROR(cur_dev_ctx == NULL, -1, "cur_dev_ctx %d is null\n", dev_id);

	INIT_LIST_HEAD(&cur_dev_ctx->cq_available_list);
	COND_RETURN_ERROR(!list_empty_careful(&cur_dev_ctx->cq_available_list), -1, "cq_available_list is not empty");

	cur_dev_ctx->cq_num = 0;
	size = (long)(unsigned)sizeof(struct devdrv_cq_sub_info) * DEVDRV_MAX_CQ_NUM;
	cq_sub_info = vmalloc(size);
	COND_RETURN_ERROR(cq_sub_info == NULL, -ENOMEM, "no mem to alloc cq sub info list\n");

	cur_dev_ctx->cq_sub_addr = (void *)cq_sub_info;

	for (i = 0; i < DEVDRV_MAX_CQ_NUM; i++) {
		cq_info = devdrv_calc_cq_info(dev_id, i);
		cq_info->head = 0;
		cq_info->tail = 0;
		cq_info->index = i;
		cq_info->count_report = 0;
		cq_info->uio_addr = NULL;
		cq_info->uio_num = DEVDRV_INVALID_FD_OR_NUM;
		cq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
		cq_info->uio_map = DEVDRV_SQ_CQ_MAP;
		cq_info->uio_size = DEVDRV_MAX_CQ_DEPTH * DEVDRV_CQ_SLOT_SIZE;
		cq_info->slot_size = DEVDRV_CQ_SLOT_SIZE;
		cq_info->stream_num = 0;
		cq_info->receive_count = 0;
		cq_info->phase = 1;
		cq_info->cq_sub = (void *)(cq_sub_info + i);
		cq_sub_info[i].proc_ctx = NULL;
		cq_sub_info[i].index = cq_info->index;
		cq_sub_info[i].virt_addr = (phys_addr_t) NULL;
		cq_sub_info[i].phy_addr = (phys_addr_t) NULL;
		spin_lock_init(&cq_sub_info[i].spinlock);
		list_add_tail(&cq_sub_info[i].list, &cur_dev_ctx->cq_available_list);
		cur_dev_ctx->cq_num++;
	}
	NPU_DRV_DEBUG("cur dev %d own %d calc cq \n", dev_id, cur_dev_ctx->cq_num);

	return 0;
}

int devdrv_inc_cq_ref_by_stream(u8 dev_id, u32 cq_id)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id %d \n", dev_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = devdrv_calc_cq_info(dev_id, cq_id);
	cq_info->stream_num++; // should do it here or user driver
	spin_unlock(&cur_dev_ctx->spinlock);

	return 0;
}

int devdrv_dec_cq_ref_by_stream(u8 dev_id, u32 cq_id)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("invalid npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}
	NPU_DRV_INFO("cq_id %d \n", cq_id);

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = devdrv_calc_cq_info(dev_id, cq_id);
	if (cq_info->stream_num == 0) {
		spin_unlock(&cur_dev_ctx->spinlock);
		NPU_DRV_WARN("cq_info stream num is zero \n");
		return -1;
	}
	cq_info->stream_num--;  // should do it here or user driver
	spin_unlock(&cur_dev_ctx->spinlock);

	return 0;
}

int devdrv_clr_cq_info(u8 dev_id, u32 cq_id)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_cq_sub_info *cq_sub_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct devdrv_report *report = NULL;
	u32 index = 0;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	(void)devdrv_write_doorbell_val(DOORBELL_RES_CAL_CQ, cq_id, 0);

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = devdrv_calc_cq_info(dev_id, cq_id);
	cq_sub_info = (struct devdrv_cq_sub_info *)cq_info->cq_sub;
	for (index = 0; index < DEVDRV_MAX_CQ_DEPTH; index++) {
		report = (struct devdrv_report *)(uintptr_t)(cq_sub_info->virt_addr +
		(long)(unsigned)(cq_info->slot_size * index));
		if (devdrv_get_phase_from_report(report) == 1) {
			NPU_DRV_DEBUG("clear cq %d report %d", cq_id, index);
			devdrv_clr_phase_in_report(report);
		}
	}

	cq_info->count_report = 0;
	cq_info->stream_num = 0;
	spin_unlock(&cur_dev_ctx->spinlock);

	NPU_DRV_WARN("devdrv_clr_cq_info end. head = 0, tail = 0, phase = 1\n");

	return 0;
}

int devdrv_get_cq_ref_by_stream(u8 dev_id, u32 cq_id)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	u32 cq_stream_num;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = devdrv_calc_cq_info(dev_id, cq_id);
	cq_stream_num = cq_info->stream_num;
	spin_unlock(&cur_dev_ctx->spinlock);

	return cq_stream_num;
}


int devdrv_alloc_cq_id(u8 dev_id)
{
	struct devdrv_cq_sub_info *cq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	if (list_empty_careful(&cur_dev_ctx->cq_available_list)) {
		spin_unlock(&cur_dev_ctx->spinlock);
		NPU_DRV_ERR("cur dev %d available cq list empty, left cq_num = %d !!!\n", dev_id,
			cur_dev_ctx->cq_num);
		return -1;
	}
	cq_sub = list_first_entry(&cur_dev_ctx->cq_available_list, struct devdrv_cq_sub_info, list);
	list_del(&cq_sub->list);
	if (cur_dev_ctx->cq_num == 0) {
		spin_unlock(&cur_dev_ctx->spinlock);
		NPU_DRV_ERR("cur dev %d cq_num is zero\n", dev_id);
		return -1;
	}
	cur_dev_ctx->cq_num--;
	spin_unlock(&cur_dev_ctx->spinlock);
	NPU_DRV_DEBUG("cur dev %d alloc:%d, left %d cq\n", dev_id, cq_sub->index, cur_dev_ctx->cq_num);

	return cq_sub->index;
}

int devdrv_free_cq_id(u8 dev_id, u32 cq_id)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_cq_sub_info *cq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (cq_id >= DEVDRV_MAX_CQ_NUM) {
		NPU_DRV_ERR("illegal npu cq id %d\n", cq_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = devdrv_calc_cq_info(dev_id, cq_id);
	cq_sub = (struct devdrv_cq_sub_info *)cq_info->cq_sub;
	list_add(&cq_sub->list, &cur_dev_ctx->cq_available_list);
	// no stream use it
	cur_dev_ctx->cq_num++;
	cq_sub->proc_ctx = NULL;
	cq_info->head = 0;
	cq_info->tail = 0;
	cq_info->count_report = 0;
	cq_info->stream_num = 0;
	cq_info->receive_count = 0;
	cq_info->slot_size = DEVDRV_CQ_SLOT_SIZE;
	spin_unlock(&cur_dev_ctx->spinlock);
	NPU_DRV_WARN("cur dev %d own %d cq\n", dev_id, cur_dev_ctx->cq_num);

	return 0;
}

static void devdrv_clear_mem_data(void *addr, u32 size)
{
	u32 i;
	u32 *tmp_addr = (u32 *) addr;
	for (i = 0; i < size / sizeof(u32); i++) {
		*tmp_addr = 0;
		tmp_addr++;
	}
}

// make sure the cq_mem data all been zero when alloced success,or bug happens
// because TS will write from cq head 0,but user driver will not when we reuse the
// dirty cq mem
int devdrv_alloc_cq_mem(u8 dev_id, u32 cq_id)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_cq_sub_info *cq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	phys_addr_t phy_addr;
	vir_addr_t cq_virt_addr;
	u64 cq_size;

	COND_RETURN_ERROR(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id %d\n", dev_id);

	COND_RETURN_ERROR(cq_id >= DEVDRV_MAX_CQ_NUM, -1, "illegal npu cq id %d\n", cq_id);

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	COND_RETURN_ERROR(cur_dev_ctx == NULL, -1, "cur_dev_ctx %d is null\n", dev_id);

	cq_info = devdrv_calc_cq_info(dev_id, cq_id);
	cq_size = DEVDRV_MAX_CQ_DEPTH * cq_info->slot_size;
	NPU_DRV_DEBUG("cq_info->slot_size = %u\n", cq_info->slot_size);

	phy_addr = (unsigned long long)(g_sq_desc->base +
		DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE * DEVDRV_MAX_SQ_NUM +
		(cq_id * DEVDRV_MAX_CQ_DEPTH * DEVDRV_CQ_SLOT_SIZE));

	cq_virt_addr = (unsigned long long)(uintptr_t) ioremap_wc(phy_addr, cq_size);
	if (cq_virt_addr == 0) {
		NPU_DRV_ERR("cur_dev_ctx %d calc cq ioremap_wc failed \n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_sub = (struct devdrv_cq_sub_info *)cq_info->cq_sub;
	cq_sub->virt_addr = cq_virt_addr;
	cq_sub->phy_addr = phy_addr;
	spin_unlock(&cur_dev_ctx->spinlock);

	// make cq mem clean
	devdrv_clear_mem_data((void *)(uintptr_t) cq_virt_addr, cq_size);
	NPU_DRV_DEBUG("dev %d cur cq %d phy_addr = %pK " "cq_virt_addr = %pK cq_size = 0x%llx\n",
		dev_id, cq_id, (void *)(uintptr_t) phy_addr, (void *)(uintptr_t) cq_virt_addr, cq_size);

	return 0;
}

int devdrv_free_cq_mem(u8 dev_id, u32 cq_id)
{
	struct devdrv_cq_sub_info *cq_sub = NULL;
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (cq_id >= DEVDRV_MAX_CQ_NUM) {
		NPU_DRV_ERR("illegal npu cq id %d\n", cq_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	cq_info = devdrv_calc_cq_info(dev_id, cq_id);

	cq_sub = (struct devdrv_cq_sub_info *)cq_info->cq_sub;
	iounmap((void *)(uintptr_t) (cq_sub->virt_addr));

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info->uio_addr = NULL;
	cq_info->uio_num = DEVDRV_INVALID_FD_OR_NUM;
	cq_info->uio_map = DEVDRV_SQ_CQ_MAP;
	cq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
	cq_sub->virt_addr = 0;
	cq_sub->phy_addr = 0;
	spin_unlock(&cur_dev_ctx->spinlock);
	NPU_DRV_DEBUG("free dev %d cur cq %d memory success\n", dev_id, cq_id);

	return 0;
}

// get cq_id cq`s cq_addr from dev_id(must called after alloc_cq_mem)
int devdrv_get_cq_phy_addr(u8 dev_id, u32 cq_id, phys_addr_t *phy_addr)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_cq_sub_info *cq_sub = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (cq_id >= DEVDRV_MAX_CQ_NUM) {
		NPU_DRV_ERR("illegal npu cq id %d\n", cq_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	if (phy_addr == NULL) {
		NPU_DRV_ERR("phy_addr is null\n");
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = devdrv_calc_cq_info(dev_id, cq_id);
	cq_sub = (struct devdrv_cq_sub_info *)cq_info->cq_sub;
	*phy_addr = cq_sub->phy_addr;
	spin_unlock(&cur_dev_ctx->spinlock);

	NPU_DRV_DEBUG("dev %d cur cq %d phy_addr = %pK \n", dev_id, cq_id, (void *)(uintptr_t)(*phy_addr));

	return 0;
}

int devdrv_cq_list_destroy(u8 dev_id)
{
	struct devdrv_cq_sub_info *cq_sub_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	if (!list_empty_careful(&cur_dev_ctx->cq_available_list)) {
		list_for_each_safe(pos, n, &cur_dev_ctx->cq_available_list) {
			cur_dev_ctx->cq_num--;
			cq_sub_info = list_entry(pos, struct devdrv_cq_sub_info, list);
			list_del(pos);
		}
	}

	vfree(cur_dev_ctx->cq_sub_addr);
	cur_dev_ctx->cq_sub_addr = NULL;
	return 0;
}
