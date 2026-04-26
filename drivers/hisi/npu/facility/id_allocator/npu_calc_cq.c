/*
 * npu_calc_cq.c
 *
 * about npu calc cq
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

#include "npu_common.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_doorbell.h"
#include "npu_proc_ctx.h"
#include "npu_calc_cq.h"

int npu_alloc_sync_cq_mem(u8 dev_id);
int npu_cq_list_init(u8 dev_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_cq_sub_info *cq_sub_info = NULL;
	struct npu_ts_cq_info *cq_info = NULL;
	unsigned long size;
	u32 i;
	int ret;

	cond_return_error(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id\n");

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx %d is null\n",
		dev_id);

	INIT_LIST_HEAD(&cur_dev_ctx->cq_available_list);
	cond_return_error(!list_empty_careful(&cur_dev_ctx->cq_available_list),
		-1, "cq_available_list is not empty");

	cur_dev_ctx->cq_num = 0;
	size = (unsigned long)((sizeof(struct npu_cq_sub_info) *
		NPU_MAX_CQ_NUM) +
		(sizeof(struct npu_sync_cq_info) * NPU_DEV_NUM));
	cq_sub_info = vmalloc(size);
	cond_return_error(cq_sub_info == NULL, -ENOMEM,
		"no mem to alloc cq sub info list\n");

	cur_dev_ctx->cq_sub_addr = (void *)cq_sub_info;

	for (i = 0; i < NPU_MAX_CQ_NUM; i++) {
		cq_info = npu_calc_cq_info(dev_id, i);
		cond_return_error(cq_info == NULL, -ENOMEM,
			"no mem to alloc cq info\n");
		cq_info->head = 0;
		cq_info->tail = 0;
		cq_info->index = i;
		cq_info->count_report = 0;
		cq_info->vir_addr = NULL;
		cq_info->pid = NPU_INVALID_FD_OR_NUM;
		cq_info->slot_size = NPU_CQ_SLOT_SIZE;
		cq_info->stream_num = 0;
		cq_info->receive_count = 0;
		cq_info->phase = NPU_PHASE_TOGGLE_STATE_1;
		cq_sub_info[i].proc_ctx = NULL;
		cq_sub_info[i].index = cq_info->index;
		cq_sub_info[i].virt_addr = (phys_addr_t) NULL;
		cq_sub_info[i].phy_addr = (phys_addr_t) NULL;
		spin_lock_init(&cq_sub_info[i].spinlock);
		list_add_tail(&cq_sub_info[i].list, &cur_dev_ctx->cq_available_list);
		cur_dev_ctx->cq_num++;
	}
	npu_drv_debug("cur dev %d own %d calc cq\n", dev_id, cur_dev_ctx->cq_num);

	cur_dev_ctx->sync_cq = (void *)(&cq_sub_info[NPU_MAX_CQ_NUM]);
	ret = npu_alloc_sync_cq_mem(dev_id);

	return ret;
}

struct npu_cq_sub_info *npu_get_cq_sub_addr(
	struct npu_dev_ctx *cur_dev_ctx, u32 cq_index)
{
	struct npu_cq_sub_info *cq_sub_info = NULL;

	if ((cq_index >= NPU_MAX_CQ_NUM) || (cur_dev_ctx->cq_sub_addr == NULL)) {
		return NULL;
	}

	cq_sub_info = (struct npu_cq_sub_info *)cur_dev_ctx->cq_sub_addr;
	cq_sub_info = cq_sub_info + cq_index;
	if (cq_sub_info->index != cq_index) {
		return NULL;
	}
	return cq_sub_info;
}

int npu_inc_cq_ref_by_stream(u8 dev_id, u32 cq_id)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}
	cq_info->stream_num++; // should do it here or user driver

	if (cq_info->stream_num != 1) {
		spin_unlock(&cur_dev_ctx->spinlock);
		return 0;
	}
	spin_unlock(&cur_dev_ctx->spinlock);
	npu_clear_cq(dev_id, cq_id);

	return 0;
}

int npu_inc_cq_ref_by_communication_stream(u8 dev_id, u32 cq_id)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}
	cq_info->communication_stream_num++; // should do it here or user driver
	spin_unlock(&cur_dev_ctx->spinlock);

	return 0;
}

int npu_dec_cq_ref_by_stream(u8 dev_id, u32 cq_id)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("invalid npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}
	npu_drv_info("cq_id %d\n", cq_id);

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}
	if (cq_info->stream_num == 0) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_warn("cq_info stream num is zero\n");
		return -1;
	}
	cq_info->stream_num--;  // should do it here or user driver

	if (cq_info->stream_num != 0) {
		spin_unlock(&cur_dev_ctx->spinlock);
		return 0;
	}
	spin_unlock(&cur_dev_ctx->spinlock);
	npu_clear_cq(dev_id, cq_id);

	return 0;
}

int npu_dec_cq_ref_by_communication_stream(u8 dev_id, u32 cq_id)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("invalid npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}
	npu_drv_info("cq_id %d\n", cq_id);

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}
	if (cq_info->communication_stream_num == 0) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_warn("cq_info communication stream num is zero\n");
		return -1;
	}
	// should do it here or user driver
	cq_info->communication_stream_num--;
	spin_unlock(&cur_dev_ctx->spinlock);

	return 0;
}

int npu_get_cq_ref_by_stream(u8 dev_id, u32 cq_id)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	u32 cq_stream_num;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}
	cq_stream_num = cq_info->stream_num;
	spin_unlock(&cur_dev_ctx->spinlock);

	return cq_stream_num;
}

int npu_get_cq_ref_by_communication_stream(u8 dev_id, u32 cq_id)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	u32 cq_stream_num;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}
	cq_stream_num = cq_info->communication_stream_num;
	spin_unlock(&cur_dev_ctx->spinlock);

	return cq_stream_num;
}

int npu_alloc_cq_id(u8 dev_id)
{
	struct npu_cq_sub_info *cq_sub = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	if (list_empty_careful(&cur_dev_ctx->cq_available_list)) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err(
			"cur dev %d available cq list empty, left cq_num = %d !!!\n",
			dev_id, cur_dev_ctx->cq_num);
		return -1;
	}
	cq_sub = list_first_entry(&cur_dev_ctx->cq_available_list,
		struct npu_cq_sub_info, list);
	list_del(&cq_sub->list);
	cur_dev_ctx->cq_num--;
	spin_unlock(&cur_dev_ctx->spinlock);
	npu_drv_debug("cur dev %d alloc:%d, left %d cq\n", dev_id, cq_sub->index,
		cur_dev_ctx->cq_num);

	return cq_sub->index;
}

int npu_free_cq_id(u8 dev_id, u32 cq_id)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_cq_sub_info *cq_sub = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (cq_id >= NPU_MAX_CQ_NUM) {
		npu_drv_err("illegal npu cq id %d\n", cq_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}
	cq_sub = npu_get_cq_sub_addr(cur_dev_ctx, cq_info->index);
	if (cq_sub == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_debug("cq_sub is null\n");
		return -1;
	}
	list_add(&cq_sub->list, &cur_dev_ctx->cq_available_list);
	// no stream use it
	cur_dev_ctx->cq_num++;
	cq_sub->proc_ctx = NULL;
	cq_info->head = 0;
	cq_info->tail = 0;
	cq_info->count_report = 0;
	cq_info->stream_num = 0;
	cq_info->receive_count = 0;
	cq_info->slot_size = NPU_CQ_SLOT_SIZE;
	spin_unlock(&cur_dev_ctx->spinlock);
	npu_drv_warn("cur dev %d own %d cq\n", dev_id, cur_dev_ctx->cq_num);

	return 0;
}

static void npu_clear_mem_data(void *addr, u32 size)
{
	u32 i;
	u32 *tmp_addr = (u32 *)addr;

	for (i = 0; i < size / sizeof(u32); i++) {
		*tmp_addr = 0;
		tmp_addr++;
	}
}

// make sure the cq_mem data all been zero when alloced success,
// or bug happens because TS will write from cq head 0,
// but user driver will not when we reuse the dirty cq mem
int npu_alloc_cq_mem(u8 dev_id, u32 cq_id, int pid)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_cq_sub_info *cq_sub = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	phys_addr_t phy_addr;
	vir_addr_t cq_virt_addr;
	u64 cq_size;

	cond_return_error(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id %d\n",
		dev_id);

	cond_return_error(cq_id >= NPU_MAX_CQ_NUM, -1, "illegal npu cq id %d\n",
		cq_id);

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx %d is null\n",
		dev_id);

	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}
	cq_size = NPU_MAX_CQ_DEPTH * cq_info->slot_size;
	npu_drv_debug("cq_info->slot_size = %u\n", cq_info->slot_size);

	phy_addr = (unsigned long long)(g_sq_desc->base +
		NPU_MAX_SQ_DEPTH * NPU_SQ_SLOT_SIZE * NPU_MAX_SQ_NUM +
		(cq_id * NPU_MAX_CQ_DEPTH * NPU_CQ_SLOT_SIZE));

	cq_virt_addr = (unsigned long long)(uintptr_t)ioremap_wc(phy_addr, cq_size);
	if (cq_virt_addr == 0) {
		npu_drv_err("cur_dev_ctx %d calc cq ioremap_wc failed\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_sub = npu_get_cq_sub_addr(cur_dev_ctx, cq_info->index);
	if (cq_sub == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		iounmap((void *)(uintptr_t)cq_virt_addr);
		npu_drv_debug("cq_sub is null\n");
		return -1;
	}
	cq_sub->virt_addr = cq_virt_addr;
	cq_sub->phy_addr = phy_addr;
	cq_info->vir_addr = NULL;
	cq_info->pid = pid;
	spin_unlock(&cur_dev_ctx->spinlock);

	npu_drv_debug("dev %d cur cq %d phy_addr = %pK "
		"cq_virt_addr = %pK cq_size = 0x%llx\n",
		dev_id, cq_id, (void *)(uintptr_t) phy_addr,
		(void *)(uintptr_t)cq_virt_addr, cq_size);

	return 0;
}

int npu_alloc_sync_cq_mem(u8 dev_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_sync_cq_info *sync_cq_info = NULL;
	phys_addr_t phy_addr;
	vir_addr_t cq_virt_addr = 0;
	u64 cq_size;

	cond_return_error(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id %d\n",
		dev_id);

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx %d is null\n",
		dev_id);

	cq_size = NPU_MAX_CQ_DEPTH * NPU_CQ_SLOT_SIZE;

	phy_addr = (unsigned long long)(g_sq_desc->base +
		NPU_MAX_SQ_DEPTH * NPU_SQ_SLOT_SIZE * NPU_MAX_SQ_NUM +
		(NPU_MAX_CQ_NUM * NPU_MAX_CQ_DEPTH * NPU_CQ_SLOT_SIZE) +
		(dev_id * cq_size));

	cq_virt_addr = (vir_addr_t)(uintptr_t)ioremap_wc(phy_addr, cq_size);
	if (cq_virt_addr == 0) {
		npu_drv_err("cur_dev_ctx %d calc sync_cq ioremap_wc failed\n", dev_id);
		return -1;
	}

	sync_cq_info = (struct npu_sync_cq_info *)cur_dev_ctx->sync_cq;

	sync_cq_info->sync_cq_paddr = phy_addr;
	sync_cq_info->sync_cq_vaddr = cq_virt_addr;
	sync_cq_info->slot_size = NPU_CQ_SLOT_SIZE;
	bitmap_zero(sync_cq_info->sync_stream_bitmap,
		NPU_MAX_NON_SINK_STREAM_ID);
	spin_lock_init(&sync_cq_info->spinlock);

	// make cq mem clean
	npu_clear_mem_data((void *)(uintptr_t) cq_virt_addr, cq_size);
	npu_drv_debug("dev %d cur sync_cq phy_addr = %pK "
		"cq_virt_addr = %pK cq_size = 0x%llx\n",
		dev_id, (void *)(uintptr_t)phy_addr,
		(void *)(uintptr_t)cq_virt_addr, cq_size);

	return 0;
}

int npu_free_cq_mem(u8 dev_id, u32 cq_id)
{
	struct npu_cq_sub_info *cq_sub = NULL;
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (cq_id >= NPU_MAX_CQ_NUM) {
		npu_drv_err("illegal npu cq id %d\n", cq_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}

	cq_sub = npu_get_cq_sub_addr(cur_dev_ctx, cq_info->index);
	if (cq_sub == NULL) {
		npu_drv_debug("cq_sub is null\n");
		return -1;
	}
	iounmap((void *)(uintptr_t) (cq_sub->virt_addr));

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info->vir_addr = NULL;
	cq_info->pid = NPU_INVALID_FD_OR_NUM;
	cq_sub->virt_addr = 0;
	cq_sub->phy_addr = 0;
	spin_unlock(&cur_dev_ctx->spinlock);
	npu_drv_debug("free dev %d cur cq %d memory success\n", dev_id, cq_id);

	return 0;
}

int npu_free_sync_cq_mem(u8 dev_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	iounmap((void *)(uintptr_t) (cur_dev_ctx->sync_cq));

	spin_lock(&cur_dev_ctx->spinlock);
	cur_dev_ctx->sync_cq = NULL;
	spin_unlock(&cur_dev_ctx->spinlock);
	npu_drv_debug("free dev %d cur sync_cq memory success\n", dev_id);

	return 0;
}

// get cq_id cq`s cq_addr from dev_id(must called after alloc_cq_mem)
int npu_get_cq_phy_addr(u8 dev_id, u32 cq_id, phys_addr_t *phy_addr)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_cq_sub_info *cq_sub = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (cq_id >= NPU_MAX_CQ_NUM) {
		npu_drv_err("illegal npu cq id %d\n", cq_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	if (phy_addr == NULL) {
		npu_drv_err("phy_addr is null\n");
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}

	cq_sub = npu_get_cq_sub_addr(cur_dev_ctx, cq_info->index);
	if (cq_sub == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_sub %u is null\n", cq_info->index);
		return -1;
	}
	*phy_addr = cq_sub->phy_addr;
	spin_unlock(&cur_dev_ctx->spinlock);

	npu_drv_debug("dev %d cur cq %d phy_addr = %pK\n", dev_id, cq_id,
		(void *)(uintptr_t)(*phy_addr));

	return 0;
}

// clear cq
// called at 2 timings
//     1. alloc
//     2. power up
//     3. power down
//     4. alloc the first stream binded with the cq
//     5. free the last stream binded with the cq
int npu_clear_cq(u8 dev_id, u32 cq_id)
{
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_cq_sub_info *cq_sub_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_report *report = NULL;
	u32 index = 0;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return -1;
	}

	cq_sub_info = npu_get_cq_sub_addr(cur_dev_ctx, cq_info->index);
	if (cq_sub_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		return -1;
	}

	for (index = 0; index < NPU_MAX_CQ_DEPTH; index++) {
		report = (struct npu_report *)(uintptr_t)(cq_sub_info->virt_addr +
		(unsigned long)(cq_info->slot_size * index));
		if (npu_get_phase_from_report(report) == 1) {
			npu_drv_debug("clear cq %d report %d", cq_id, index);
			npu_set_phase_to_report(report, 0);
		}
	}
	cq_info->count_report = 0;
	cq_info->phase = NPU_PHASE_TOGGLE_STATE_1;
	cq_info->head = 0;
	cq_info->tail = 0;
	cq_info->communication_stream_num = 0;

	spin_unlock(&cur_dev_ctx->spinlock);

	npu_drv_warn("clr_cq_info cq:%u, head=0, tail=0, phase=1\n", cq_id);

	return 0;
}

struct npu_ts_cq_info *npu_alloc_cq(u8 dev_id, int pid)
{
	struct npu_ts_cq_info *cq_info = NULL;
	int cq_id;
	int ret;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return NULL;
	}

	cq_id = npu_alloc_cq_id(dev_id);
	if (cq_id < 0) {
		npu_drv_err("alloc cq_id from dev %d failed\n", dev_id);
		return NULL;
	}
	// alloc cq mem
	// (do it through user mmap at the stage of open device currently)
	ret = npu_alloc_cq_mem(dev_id, cq_id, pid);
	if (ret != 0) {
		npu_free_cq_id(dev_id, cq_id);
		npu_drv_err("alloc cq mem from dev %d cq %d failed\n", dev_id, cq_id);
		return NULL;
	}

	ret = npu_clear_cq(dev_id, cq_id);
	if (ret != 0) {
		npu_free_cq_id(dev_id, cq_id);
		npu_free_cq_mem(dev_id, cq_id);
		npu_drv_err("clear cq from dev %d cq %d failed\n", dev_id, cq_id);
		return NULL;
	}

	cq_info = npu_calc_cq_info(dev_id, cq_id);
	if (cq_info == NULL) {
		npu_free_cq_id(dev_id, cq_id);
		npu_free_cq_mem(dev_id, cq_id);
		npu_drv_err("cq_info id: %u is null\n", cq_id);
		return NULL;
	}
	return cq_info;
}

int npu_free_cq(u8 dev_id, u32 cq_id)
{
	int ret;

	ret = npu_free_cq_id(dev_id, cq_id);
	cond_return_error(ret != 0, -1, "free cq id failed, ret:%d\n", ret);

	ret = npu_clear_cq(dev_id, cq_id);
	cond_return_error(ret != 0, -1, "clear cq failed, ret:%d\n", ret);

	ret = npu_free_cq_mem(dev_id, cq_id);
	cond_return_error(ret != 0, -1, "free cq mem failed, ret:%d\n", ret);

	return 0;
}

int npu_cq_list_destroy(u8 dev_id)
{
	struct npu_cq_sub_info *cq_sub_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	if (!list_empty_careful(&cur_dev_ctx->cq_available_list)) {
		list_for_each_safe(pos, n, &cur_dev_ctx->cq_available_list) {
			cur_dev_ctx->cq_num--;
			cq_sub_info = list_entry(pos, struct npu_cq_sub_info, list);
			list_del(pos);
		}
	}

	(void)npu_free_sync_cq_mem(dev_id);

	vfree(cur_dev_ctx->cq_sub_addr);
	cur_dev_ctx->cq_sub_addr = NULL;
	return 0;
}
