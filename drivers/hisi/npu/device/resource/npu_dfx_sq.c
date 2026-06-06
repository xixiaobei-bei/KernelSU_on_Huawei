/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#include <linux/irq.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include "npu_common.h"
#include "npu_pm.h"
#include "npu_shm.h"
#include "drv_log.h"
#include "npu_mailbox.h"
#include "devdrv_user_common.h"
#include "npu_platform.h"
#include "npu_dfx_cq.h"
#include "npu_dfx.h"
#include "npu_cache.h"

int devdrv_get_dfx_sq_memory(struct devdrv_dfx_sq_info *sq_info,
			     u32 size, phys_addr_t *phy_addr,
				 u8 **sq_addr, unsigned int *buf_size)
{
	struct devdrv_platform_info *plat_info = NULL;
	unsigned int buf_size_tmp = 0;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info.\n");
		return -EINVAL;
	}

	buf_size_tmp = DEVDRV_MAX_DFX_SQ_DEPTH * DEVDRV_DFX_MAX_SQ_SLOT_LEN;
	if (size > buf_size_tmp) {
		devdrv_drv_err("sq size=0x%x > 0x%x error\n", size,
			       buf_size_tmp);
		return -ENOMEM;
	}
	// phy_addr:sq_calc_size + cq_calc_size + off_size
	*phy_addr = (unsigned long long)(g_sq_desc->base +
					 DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE * DEVDRV_MAX_SQ_NUM +
					 (DEVDRV_MAX_CQ_NUM * DEVDRV_MAX_CQ_DEPTH * DEVDRV_CQ_SLOT_SIZE) +
					 (u64) sq_info->index * buf_size_tmp);	//lint !e647
	*sq_addr = ioremap_nocache(*phy_addr, buf_size_tmp);
	if (*sq_addr == NULL) {
		devdrv_drv_err("ioremap_nocache failed.\n");
		return -ENOMEM;
	}
	*buf_size = buf_size_tmp;

	devdrv_drv_debug("cur sq %d phy_addr = %pK "
			 "cq_virt_addr = %pK base = %pK\n",
			 sq_info->index, (void *)(uintptr_t) *phy_addr,
			 (void *)(uintptr_t) (long)*sq_addr,
			 (void *)(uintptr_t) (long)g_sq_desc->base);

	return 0;
}

int devdrv_dfx_sq_para_check(struct devdrv_dfx_create_sq_para *sq_para)
{
	if (sq_para->sq_index >= DEVDRV_MAX_DFX_SQ_NUM || sq_para->addr == NULL) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	if (sq_para->slot_len <= 0
	    || sq_para->slot_len > DEVDRV_DFX_MAX_SQ_SLOT_LEN) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	return 0;
}

int devdrv_create_dfx_sq(struct devdrv_dev_ctx *cur_dev_ctx,
			 struct devdrv_dfx_create_sq_para *sq_para)
{
	struct devdrv_dfx_sq_info *sq_info = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;
	u32 len;
	unsigned int buf_size = 0;
	unsigned int i;
	u32 sq_index = 0;
	int ret = 0;
	phys_addr_t phy_addr = 0;
	u8 *sq_addr = NULL;

	if (cur_dev_ctx == NULL || sq_para == NULL) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	if (devdrv_dfx_sq_para_check(sq_para)) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		devdrv_drv_err("cqsq is null.\n");
		return -ENOMEM;
	}

	if (cqsq->sq_num == 0) {
		devdrv_drv_err("no available sq num=%d.\n", cqsq->sq_num);
		return -ENOMEM;
	}

	sq_index = sq_para->sq_index;
	len = DEVDRV_MAX_DFX_SQ_DEPTH * sq_para->slot_len;
	sq_info = cqsq->sq_info;
	ret = devdrv_get_dfx_sq_memory(&sq_info[sq_index], len, &phy_addr,
					&sq_addr, &buf_size);
	if (ret) {
		devdrv_drv_err("type =%d get memory failure.\n", sq_index);
		return -ENOMEM;
	}

	mutex_lock(&cqsq->dfx_mutex);
	sq_info[sq_index].phy_addr = phy_addr;
	sq_info[sq_index].addr = sq_addr;
	for (i = 0; i < buf_size; i += 4) {
		writel(0, &sq_info[sq_index].addr[i]);
	}
	sq_info[sq_index].function = sq_para->function;
	sq_info[sq_index].depth = DEVDRV_MAX_DFX_SQ_DEPTH;
	sq_info[sq_index].slot_len = sq_para->slot_len;
	cqsq->sq_num--;
	*sq_para->addr = (unsigned long)sq_info[sq_index].phy_addr;
	mutex_unlock(&cqsq->dfx_mutex);

	devdrv_drv_debug("dev[%d] dfx sq is created, sq id: %d, sq addr: 0x%lx.\n",
			 (u32) cur_dev_ctx->devid, sq_info[sq_index].index,
			 (unsigned long)sq_info[sq_index].phy_addr);	// for test

	return 0;
}

EXPORT_SYMBOL(devdrv_create_dfx_sq);

void devdrv_destroy_dfx_sq(struct devdrv_dev_ctx *cur_dev_ctx, u32 sq_index)
{
	struct devdrv_dfx_sq_info *sq_info = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;

	if (cur_dev_ctx == NULL || sq_index >= DEVDRV_MAX_DFX_SQ_NUM) {
		devdrv_drv_err("invalid input argument.\n");
		return;
	}

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		devdrv_drv_err("cqsq is null.\n");
		return;
	}

	sq_info = cqsq->sq_info;
	if(sq_info == NULL) {
		devdrv_drv_err("sq_info is null.\n");
		return ;
	}
	if (sq_info[sq_index].addr != NULL) {
		iounmap(sq_info[sq_index].addr);
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
	u32 len;

	if (devid >= DEVDRV_MAX_DAVINCI_NUM || sq_index >= DEVDRV_MAX_DFX_SQ_NUM
	    || buffer == NULL || buf_len <= 0
	    || buf_len > DEVDRV_DFX_MAX_SQ_SLOT_LEN) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", devid);
		return -ENODEV;
	}
	// ts_work_status 0: power down
	if (cur_dev_ctx->ts_work_status == 0) {
		devdrv_drv_err("device is not working.\n");
		return DEVDRV_TS_DOWN;
	}

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		devdrv_drv_err("cqsq is null.\n");
		return -ENOMEM;
	}

	sq_info = cqsq->sq_info;
	if (sq_info[sq_index].addr == NULL) {
		devdrv_drv_err("invalid sq, sq_index = %u.\n", sq_index);
		return -ENOMEM;
	}

	tail = sq_info[sq_index].tail;
	credit = (sq_info->tail >= sq_info->head) ?
	    (DEVDRV_MAX_DFX_SQ_DEPTH - (sq_info->tail - sq_info->head + 1)) :
	    (sq_info->head - sq_info->tail - 1);
	if (credit <= 0) {
		devdrv_drv_err("no available sq slot.\n");
		return -ENOMEM;
	}

	addr =
	    sq_info[sq_index].addr +
	    (unsigned long)tail * sq_info[sq_index].slot_len;

	memcpy(addr, buffer, buf_len);

	len = sq_info[sq_index].slot_len * DEVDRV_MAX_DFX_SQ_DEPTH;

	if (tail >= DEVDRV_MAX_DFX_SQ_DEPTH - 1)
		tail = 0;
	else
		tail++;

	sq_info[sq_index].tail = tail;
	*sq_info[sq_index].doorbell = (u32) tail;

	devdrv_drv_debug
	    ("a new dfx sq cmd is sent, sq_index : %d, tail : %d,phy_addr: "
	     "0x%llx, doorbell_addr: 0x%llx.\n", sq_index, tail,
	     sq_info[sq_index].phy_addr,
	     (phys_addr_t) (uintptr_t) sq_info[sq_index].doorbell);

	return 0;
}

EXPORT_SYMBOL(devdrv_dfx_send_sq);
