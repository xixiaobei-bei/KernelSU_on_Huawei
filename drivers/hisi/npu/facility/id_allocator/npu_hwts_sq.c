/*
 * npu_hwts_sq.c
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
#include "npu_hwts_sq.h"

#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/vmalloc.h>

#include "devdrv_user_common.h"
#include "npu_common.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_pool.h"


#define DEVDRV_HWTS_SQ_BLOCK_NUM  1  // do not consider the case where the pool size is not the same

static int devdrv_hwts_sq_pool_alloc(u32 len, vir_addr_t *virt_addr, phys_addr_t *phy_addr, unsigned long *iova)
{
	*iova = 0;
	*phy_addr = (unsigned long long)(g_hwts_sq_desc->base);
	*virt_addr = (unsigned long long)(uintptr_t) ioremap_wc(*phy_addr, DEVDRV_MAX_HWTS_SQ_NUM *
		DEVDRV_MAX_HWTS_SQ_DEPTH * DEVDRV_HWTS_SQ_SLOT_SIZE);
	if (*virt_addr == 0) {
		NPU_DRV_ERR("devdrv_hwts_sq_alloc=>ioremap_noncache error! virt_addr = 0, phy_addr=%llx\n", *phy_addr);
		return -1;
	}
	return 0;
}
static void devdrv_hwts_sq_pool_free(vir_addr_t virt_addr, phys_addr_t phy_addr, unsigned long iova)
{
	iounmap((void *)(uintptr_t) (virt_addr));
	return;
}

int devdrv_hwts_sq_init(u8 dev_id)
{
	struct devdrv_hwts_sq_info *hwts_sq_info = NULL;
	int ret;
	int i;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id\n");
		return -1;
	}

	ret = devdrv_pool_regist(dev_id, DEVDRV_MEM_POOL_TYPE_HWTS_SQ, DEVDRV_MAX_HWTS_SQ_NUM,
		DEVDRV_MAX_HWTS_SQ_DEPTH * DEVDRV_HWTS_SQ_SLOT_SIZE, DEVDRV_HWTS_SQ_BLOCK_NUM,
		devdrv_hwts_sq_pool_alloc, devdrv_hwts_sq_pool_free);
	if (ret != 0) {
		NPU_DRV_ERR("devdrv_hwts_sq_init=>devdrv_pool_regist error! ret = %d", ret);
		return -1;
	}

	for (i = 0; i < DEVDRV_MAX_HWTS_SQ_NUM; i++) {
		hwts_sq_info = devdrv_calc_hwts_sq_info(dev_id, i);
		hwts_sq_info->head = 0;
		hwts_sq_info->tail = 0;
		hwts_sq_info->credit = DEVDRV_MAX_HWTS_SQ_DEPTH - 1;
		hwts_sq_info->index = (u32)i;
		hwts_sq_info->uio_addr = 0;
		hwts_sq_info->uio_num = DEVDRV_INVALID_FD_OR_NUM;
		hwts_sq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
		hwts_sq_info->uio_size = DEVDRV_MAX_HWTS_SQ_DEPTH * DEVDRV_HWTS_SQ_SLOT_SIZE;
		hwts_sq_info->stream_num = 0;
		hwts_sq_info->send_count = 0;
	}
	return 0;
}

int devdrv_hwts_sq_destroy(u8 dev_id)
{
	return devdrv_pool_unregist(dev_id, DEVDRV_MEM_POOL_TYPE_HWTS_SQ);
}

int devdrv_alloc_hwts_sq(u8 dev_id)
{
	struct devdrv_entity_info *entity_info = NULL;
	struct devdrv_hwts_sq_info *hwts_sq_info = NULL;

	entity_info = devdrv_alloc_entity(dev_id, DEVDRV_MEM_POOL_TYPE_HWTS_SQ);
	if (entity_info == NULL) {
		NPU_DRV_ERR("devdrv_alloc_hwts_sq=>devdrv_alloc_entry fail\n");
		return -1;
	}

	hwts_sq_info = devdrv_calc_hwts_sq_info(dev_id, entity_info->index);
	if (hwts_sq_info == NULL) {
		NPU_DRV_ERR("devdrv_alloc_hwts_sq=>devdrv_calc_hwts_sq_info fail, index=%d\n", entity_info->index);
		devdrv_free_entity(dev_id, DEVDRV_MEM_POOL_TYPE_HWTS_SQ, entity_info->index);
		return -1;
	}

	hwts_sq_info->hwts_sq_sub = (u64)(uintptr_t)entity_info;
	hwts_sq_info->phy_addr = entity_info->phy_addr;
	NPU_DRV_INFO("devdrv_alloc_hwts_sq=>index=%d, virtaddr=%llx, phyaddr=%llx, iova=%llx\n", entity_info->index,
		entity_info->vir_addr, entity_info->phy_addr, entity_info->iova);
	return entity_info->index;
}

int devdrv_free_hwts_sq(u8 dev_id, u32 hwts_sq_id)
{
	struct devdrv_hwts_sq_info *hwts_sq_info = NULL;
	int ret;

	ret = devdrv_free_entity(dev_id, DEVDRV_MEM_POOL_TYPE_HWTS_SQ, hwts_sq_id);
	if (ret != 0) {
		NPU_DRV_ERR("devdrv_free_hwts_sq=>devdrv_free_entity error!hwts_sq_id=%d, ret = %d", hwts_sq_id, ret);
		return ret;
	}

	hwts_sq_info = devdrv_calc_hwts_sq_info(dev_id, hwts_sq_id);
	if (hwts_sq_info == NULL) {
		NPU_DRV_DEBUG("sq_sub is null\n");
		return -1;
	}
	hwts_sq_info->uio_addr = 0;
	hwts_sq_info->uio_num = DEVDRV_INVALID_FD_OR_NUM;
	hwts_sq_info->uio_map = DEVDRV_SQ_CQ_MAP;
	hwts_sq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
	hwts_sq_info->phy_addr = 0;
	return 0;
}

