/*
 * npu_dfx_sq.c
 *
 * about npu dfx sq
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
#include <linux/irq.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include "npu_common.h"
#include "npu_pm.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_mailbox.h"
#include "devdrv_user_common.h"
#include "npu_platform.h"
#include "npu_dfx_cq.h"
#include "npu_dfx.h"
#include "npu_cache.h"

int devdrv_get_dfx_sq_memory(const struct devdrv_dfx_sq_info *sq_info, u32 size, phys_addr_t *phy_addr,
                             u8 **sq_addr, unsigned int *buf_size)
{
	struct devdrv_platform_info *plat_info = NULL;
	unsigned int buf_size_tmp;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info\n");
		return -EINVAL;
	}

	buf_size_tmp = DEVDRV_MAX_DFX_SQ_DEPTH * DEVDRV_DFX_MAX_SQ_SLOT_LEN;
	if (size > buf_size_tmp) {
		NPU_DRV_ERR("sq size=0x%x > 0x%x error\n", size, buf_size_tmp);
		return -ENOMEM;
	}
	// phy_addr:sq_calc_size + cq_calc_size + off_size
	*phy_addr = (unsigned long long)(g_sq_desc->base +
		DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE * DEVDRV_MAX_SQ_NUM +
		(DEVDRV_MAX_CQ_NUM * DEVDRV_MAX_CQ_DEPTH * DEVDRV_CQ_SLOT_SIZE) +
		(u64) sq_info->index * buf_size_tmp);
	*sq_addr = ioremap_nocache(*phy_addr, buf_size_tmp);
	if (*sq_addr == NULL) {
		NPU_DRV_ERR("ioremap_nocache failed\n");
		return -ENOMEM;
	}
	*buf_size = buf_size_tmp;

	NPU_DRV_DEBUG("cur sq %d phy_addr = %pK cq_virt_addr = %pK base = %pK\n",
		sq_info->index, (void *)(uintptr_t)(*phy_addr), (void *)(uintptr_t)(*sq_addr),
		(void *)(uintptr_t) (long)g_sq_desc->base);

	return 0;
}

int devdrv_dfx_sq_para_check(const struct devdrv_dfx_create_sq_para *sq_para)
{
	if (sq_para->sq_index >= DEVDRV_MAX_DFX_SQ_NUM || sq_para->addr == NULL) {
		NPU_DRV_ERR("invalid input argument\n");
		return -EINVAL;
	}

	if (sq_para->slot_len <= 0 || sq_para->slot_len > DEVDRV_DFX_MAX_SQ_SLOT_LEN) {
		NPU_DRV_ERR("invalid input argument\n");
		return -EINVAL;
	}

	return 0;
}

int devdrv_create_dfx_sq(struct devdrv_dev_ctx *cur_dev_ctx, struct devdrv_dfx_create_sq_para *sq_para)
{
	struct devdrv_dfx_sq_info *sq_info = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;
	u32 len;
	unsigned int buf_size = 0;
	unsigned int i;
	u32 sq_index;
	int ret;
	phys_addr_t phy_addr = 0;
	u8 *sq_addr = NULL;
	COND_RETURN_ERROR(cur_dev_ctx == NULL || sq_para == NULL, -EINVAL, "invalid input argument\n");

	COND_RETURN_ERROR(devdrv_dfx_sq_para_check(sq_para), -EINVAL, "invalid input argument\n");

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	COND_RETURN_ERROR(cqsq == NULL, -ENOMEM, "cqsq is null\n");

	COND_RETURN_ERROR(cqsq->sq_num == 0, -ENOMEM, "no available sq num=%d\n", cqsq->sq_num);

	sq_index = sq_para->sq_index;
	len = DEVDRV_MAX_DFX_SQ_DEPTH * sq_para->slot_len;
	sq_info = cqsq->sq_info;
	ret = devdrv_get_dfx_sq_memory(&sq_info[sq_index], len, &phy_addr, &sq_addr, &buf_size);
	COND_RETURN_ERROR(ret, -ENOMEM, "type =%d get memory failure\n", sq_index);

	mutex_lock(&cqsq->dfx_mutex);
	sq_info[sq_index].phy_addr = phy_addr;
	sq_info[sq_index].addr = sq_addr;
	for (i = 0; i < buf_size; i += 4)
		writel(0, &sq_info[sq_index].addr[i]);

	sq_info[sq_index].function = sq_para->function;
	sq_info[sq_index].depth = DEVDRV_MAX_DFX_SQ_DEPTH;
	sq_info[sq_index].slot_len = sq_para->slot_len;
	cqsq->sq_num--;
	*sq_para->addr = (unsigned long)sq_info[sq_index].phy_addr;
	mutex_unlock(&cqsq->dfx_mutex);

	NPU_DRV_DEBUG("dev[%d] dfx sq is created, sq id: %d, sq addr: 0x%lx\n",
		(u32) cur_dev_ctx->devid, sq_info[sq_index].index,
		(unsigned long)sq_info[sq_index].phy_addr);   // for test

	return 0;
}
EXPORT_SYMBOL(devdrv_create_dfx_sq);

void devdrv_destroy_dfx_sq(struct devdrv_dev_ctx *cur_dev_ctx, u32 sq_index)
{
	struct devdrv_dfx_sq_info *sq_info = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;

	if (cur_dev_ctx == NULL || sq_index >= DEVDRV_MAX_DFX_SQ_NUM) {
		NPU_DRV_ERR("invalid input argument\n");
		return;
	}

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		NPU_DRV_ERR("cqsq is null\n");
		return;
	}

	sq_info = cqsq->sq_info;
	if (sq_info == NULL) {
		NPU_DRV_ERR("sq_info is null\n");
		return ;
	}
	if (sq_info[sq_index].addr != NULL) {
#ifdef PROFILING_USE_RESERVED_MEMORY
		iounmap(sq_info[sq_index].addr);
#else
		kfree(sq_info[sq_index].addr);
#endif
		mutex_lock(&cqsq->dfx_mutex);
		sq_info[sq_index].slot_len = 0;
		sq_info[sq_index].addr = NULL;
		sq_info[sq_index].head = 0;
		sq_info[sq_index].tail = 0;
		sq_info[sq_index].credit = DEVDRV_MAX_DFX_SQ_DEPTH;
		sq_info[sq_index].function = DEVDRV_MAX_CQSQ_FUNC;
		sq_info[sq_index].phy_addr = 0;
		cqsq->sq_num++;
		mutex_unlock(&cqsq->dfx_mutex);
	}
}
EXPORT_SYMBOL(devdrv_destroy_dfx_sq);

int devdrv_dfx_send_sq(u32 devid, u32 sq_index, const u8 *buffer, u32 buf_len)
{
	struct devdrv_dfx_sq_info *sq_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;
	int credit;
	u8 *addr = NULL;
	u32 tail;

	if (devid >= DEVDRV_MAX_DAVINCI_NUM || sq_index >= DEVDRV_MAX_DFX_SQ_NUM ||
		buffer == NULL || buf_len <= 0 || buf_len > DEVDRV_DFX_MAX_SQ_SLOT_LEN) {
		NPU_DRV_ERR("invalid input argument\n");
		return -EINVAL;
	}

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	COND_RETURN_ERROR(cur_dev_ctx == NULL, -ENODEV, "cur_dev_ctx %d is null\n", devid);

	// ts_work_status 0: power down
	COND_RETURN_ERROR(cur_dev_ctx->ts_work_status == 0, DEVDRV_TS_DOWN, "device is not working\n");

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	COND_RETURN_ERROR(cqsq == NULL, -ENOMEM, "cqsq is null\n");

	sq_info = cqsq->sq_info;
	COND_RETURN_ERROR(sq_info[sq_index].addr == NULL, -ENOMEM, "invalid sq, sq_index = %u\n", sq_index);

	tail = sq_info[sq_index].tail;
	credit = (sq_info->tail >= sq_info->head) ?
		(DEVDRV_MAX_DFX_SQ_DEPTH - (sq_info->tail - sq_info->head + 1)) :
		(sq_info->head - sq_info->tail - 1);
	COND_RETURN_ERROR(credit <= 0, -ENOMEM, "no available sq slot\n");

	addr = sq_info[sq_index].addr + (unsigned long)tail * sq_info[sq_index].slot_len;

	memcpy(addr, buffer, buf_len);

	if (tail >= DEVDRV_MAX_DFX_SQ_DEPTH - 1) {
		tail = 0;
	} else {
		tail++;
	}
	sq_info[sq_index].tail = tail;
	*sq_info[sq_index].doorbell = (u32) tail;

	NPU_DRV_DEBUG("a new dfx sq cmd is sent, sq_index : %d, tail : %d,phy_addr: "
		"0x%llx, doorbell_addr: 0x%llx\n", sq_index, tail, sq_info[sq_index].phy_addr,
		(phys_addr_t) (uintptr_t) sq_info[sq_index].doorbell);

	return 0;
}
EXPORT_SYMBOL(devdrv_dfx_send_sq);
