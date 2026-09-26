/*
 * npu_hwts_sq.c
 *
 * about npu hwts sq
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
#include "npu_hwts_sq.h"

#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/vmalloc.h>

#include "npu_user_common.h"
#include "npu_common.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_iova.h"
#include "npu_pool.h"

// iova way
static int npu_hwts_sq_pool_alloc(u32 len, vir_addr_t *virt_addr,
	unsigned long *iova)
{
	int ret;

	if (virt_addr == NULL || iova == NULL) {
		npu_drv_err("virt_add or iova is null\n");
		return -1;
	}

	ret = npu_iova_alloc(virt_addr, len);
	if (ret != 0) {
		npu_drv_err("npu_iova_alloc fail, ret=%d\n", ret);
		return ret;
	}
	ret = npu_iova_map(*virt_addr, iova);
	if (ret != 0) {
		npu_drv_err("npu_iova_map fail, ret=%d\n", ret);
		npu_iova_free(*virt_addr);
	}
	return ret;
}

static void npu_hwts_sq_pool_free(vir_addr_t virt_addr, unsigned long iova)
{
	int ret = 0;

	ret = npu_iova_unmap(virt_addr, iova);
	if (ret != 0) {
		npu_drv_err("npu_iova_unmap fail, ret=%d\n", ret);
		return;
	}

	npu_iova_free(virt_addr);
}

int npu_hwts_sq_init(u8 dev_id)
{
	struct npu_hwts_sq_info *hwts_sq_info = NULL;
	int ret;
	int i;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	ret = npu_pool_regist(dev_id, NPU_MEM_POOL_TYPE_HWTS_SQ,
		NPU_MAX_LONG_HWTS_SQ_NUM,
		NPU_MAX_HWTS_SQ_NUM - NPU_MAX_LONG_HWTS_SQ_NUM,
		NPU_MAX_HWTS_SQ_DEPTH * NPU_HWTS_SQ_SLOT_SIZE,
		NPU_MAX_HWTS_SQ_POOL_NUM, npu_hwts_sq_pool_alloc,
		npu_hwts_sq_pool_free);
	if (ret != 0) {
		npu_drv_err("npu_pool_regist error! ret = %d", ret);
		return -1;
	}

	ret = npu_pool_regist(dev_id, NPU_MEM_POOL_TYPE_LONG_HWTS_SQ,
		0, NPU_MAX_LONG_HWTS_SQ_NUM,
		NPU_MAX_LONG_HWTS_SQ_DEPTH * NPU_HWTS_SQ_SLOT_SIZE,
		NPU_MAX_LONG_HWTS_SQ_POOL_NUM, npu_hwts_sq_pool_alloc,
		npu_hwts_sq_pool_free);
	if (ret != 0) {
		npu_pool_unregist(dev_id, NPU_MEM_POOL_TYPE_HWTS_SQ);
		npu_drv_err("npu_pool_regist error! ret = %d", ret);
		return -1;
	}

	for (i = 0; i < NPU_MAX_HWTS_SQ_NUM; i++) {
		hwts_sq_info = npu_calc_hwts_sq_info(dev_id, i);
		hwts_sq_info->head = 0;
		hwts_sq_info->tail = 0;
		hwts_sq_info->credit = i < NPU_MAX_LONG_HWTS_SQ_NUM ?
			(NPU_MAX_LONG_HWTS_SQ_DEPTH - 1) :
			(NPU_MAX_HWTS_SQ_DEPTH - 1);
		hwts_sq_info->index = (u32)i;
		hwts_sq_info->vir_addr = 0;
		hwts_sq_info->pid = NPU_INVALID_FD_OR_NUM;
		hwts_sq_info->length = hwts_sq_info->credit + 1;
		hwts_sq_info->stream_num = 0;
		hwts_sq_info->send_count = 0;
	}
	return 0;
}

int npu_hwts_sq_destroy(u8 dev_id)
{
	int ret;

	ret = npu_pool_unregist(dev_id, NPU_MEM_POOL_TYPE_HWTS_SQ);
	ret += npu_pool_unregist(dev_id, NPU_MEM_POOL_TYPE_LONG_HWTS_SQ);
	return ret;
}

int npu_alloc_hwts_sq(u8 dev_id, u8 is_long)
{
	struct npu_entity_info *entity_info = NULL;
	struct npu_hwts_sq_info *hwts_sq_info = NULL;

	entity_info = npu_alloc_entity(dev_id, is_long ?
		NPU_MEM_POOL_TYPE_LONG_HWTS_SQ : NPU_MEM_POOL_TYPE_HWTS_SQ);
	if (entity_info == NULL) {
		npu_drv_err("npu_alloc_entry fail\n");
		return -1;
	}

	hwts_sq_info = npu_calc_hwts_sq_info(dev_id, entity_info->index);
	if (hwts_sq_info == NULL) {
		npu_drv_err("npu_calc_hwts_sq_info fail, index = %u\n",
			entity_info->index);
		npu_free_entity(dev_id, is_long ? NPU_MEM_POOL_TYPE_LONG_HWTS_SQ :
			NPU_MEM_POOL_TYPE_HWTS_SQ, entity_info->index);
		return -1;
	}

	hwts_sq_info->hwts_sq_sub = (u64)(uintptr_t)entity_info;
	hwts_sq_info->iova_addr = entity_info->iova;
	npu_drv_debug("index = %u, virtaddr = %llx, iova = %llx\n",
		entity_info->index, entity_info->vir_addr, entity_info->iova);
	return entity_info->index;
}

int npu_free_hwts_sq(u8 dev_id, u32 hwts_sq_id)
{
	struct npu_hwts_sq_info *hwts_sq_info = NULL;
	int ret;

	ret = npu_free_entity(dev_id, hwts_sq_id >= NPU_MAX_LONG_HWTS_SQ_NUM ?
		NPU_MEM_POOL_TYPE_HWTS_SQ : NPU_MEM_POOL_TYPE_LONG_HWTS_SQ,
		hwts_sq_id);
	if (ret != 0) {
		npu_drv_err("npu_free_entity error!hwts_sq_id = %u, ret = %d",
		hwts_sq_id, ret);
		return ret;
	}

	hwts_sq_info = npu_calc_hwts_sq_info(dev_id, hwts_sq_id);
	if (hwts_sq_info == NULL) {
		npu_drv_debug("sq_sub is null\n");
		return -1;
	}
	hwts_sq_info->vir_addr = 0;
	hwts_sq_info->pid = NPU_INVALID_FD_OR_NUM;
	hwts_sq_info->iova_addr = 0;
	return 0;
}
