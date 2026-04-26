/*
 * npu_dfx_sq.c
 *
 * about npu dfx sq
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
#include <linux/irq.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <securec.h>

#include "npu_common.h"
#include "npu_pm_framework.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_mailbox.h"
#include "npu_user_common.h"
#include "npu_platform.h"
#include "npu_dfx_cq.h"
#include "npu_dfx.h"
#include "npu_cache.h"

int npu_get_dfx_sq_memory(const struct npu_dfx_sq_info *sq_info,
	u32 size, phys_addr_t *phy_addr, u8 **sq_addr, unsigned int *buf_size)
{
	struct npu_platform_info *plat_info = NULL;
	unsigned int buf_size_tmp;

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return -EINVAL;
	}

	buf_size_tmp = NPU_MAX_DFX_SQ_DEPTH * NPU_DFX_MAX_SQ_SLOT_LEN;
	if (size > buf_size_tmp) {
		npu_drv_err("sq size=0x%x > 0x%x error\n", size, buf_size_tmp);
		return -ENOMEM;
	}
	// phy_addr:sq_calc_size + cq_calc_size + off_size
	*phy_addr = (unsigned long long)(g_sq_desc->base +
		NPU_DFX_SQ_OFFSET + (u64) sq_info->index * buf_size_tmp);

	*sq_addr = ioremap_nocache(*phy_addr, buf_size_tmp);
	if (*sq_addr == NULL) {
		npu_drv_err("ioremap_nocache failed\n");
		return -ENOMEM;
	}
	*buf_size = buf_size_tmp;

	npu_drv_debug("cur sq %d phy_addr = %pK virt_addr = %pK base = %pK\n",
		sq_info->index, (void *)(uintptr_t)(*phy_addr),
		(void *)(uintptr_t)(*sq_addr),
		(void *)(uintptr_t) (long)g_sq_desc->base);

	return 0;
}

int npu_dfx_sq_para_check(const struct npu_dfx_create_sq_para *sq_para)
{
	if (sq_para->sq_index >= NPU_MAX_DFX_SQ_NUM || sq_para->addr == NULL) {
		npu_drv_err("invalid input argument\n");
		return -EINVAL;
	}

	if (sq_para->slot_len <= 0 ||
		sq_para->slot_len > NPU_DFX_MAX_SQ_SLOT_LEN) {
		npu_drv_err("invalid input argument\n");
		return -EINVAL;
	}

	return 0;
}

int npu_create_dfx_sq(struct npu_dev_ctx *cur_dev_ctx,
	struct npu_dfx_create_sq_para *sq_para)
{
	struct npu_dfx_sq_info *sq_info = NULL;
	struct npu_dfx_cqsq *cqsq = NULL;
	u32 len;
	unsigned int buf_size = 0;
	unsigned int i;
	u32 sq_index;
	int ret;
	phys_addr_t phy_addr = 0;
	u8 *sq_addr = NULL;

	cond_return_error(cur_dev_ctx == NULL || sq_para == NULL, -EINVAL,
		"invalid input argument\n");
	cond_return_error(npu_dfx_sq_para_check(sq_para), -EINVAL,
		"invalid input argument\n");

	cqsq = npu_get_dfx_cqsq_info(cur_dev_ctx);
	cond_return_error(cqsq == NULL, -ENOMEM, "cqsq is null\n");

	cond_return_error(cqsq->sq_num == 0, -ENOMEM,
		"no available sq num=%d\n", cqsq->sq_num);

	sq_index = sq_para->sq_index;
	len = NPU_MAX_DFX_SQ_DEPTH * sq_para->slot_len;
	sq_info = cqsq->sq_info;
	ret = npu_get_dfx_sq_memory(&sq_info[sq_index], len, &phy_addr,
		&sq_addr, &buf_size);
	cond_return_error(ret, -ENOMEM, "type =%d get memory failure\n", sq_index);

	mutex_lock(&cqsq->dfx_mutex);
	sq_info[sq_index].phy_addr = phy_addr;
	sq_info[sq_index].addr = sq_addr;
	for (i = 0; i < buf_size; i += 4)
		writel(0, &sq_info[sq_index].addr[i]);

	sq_info[sq_index].function = sq_para->function;
	sq_info[sq_index].depth = NPU_MAX_DFX_SQ_DEPTH;
	sq_info[sq_index].slot_len = sq_para->slot_len;
	cqsq->sq_num--;
	*sq_para->addr = (unsigned long)sq_info[sq_index].phy_addr;
	mutex_unlock(&cqsq->dfx_mutex);

	npu_drv_debug("dev[%d] dfx sq is created, sq id: %d, sq addr: 0x%lx\n",
		(u32) cur_dev_ctx->devid, sq_info[sq_index].index,
		(unsigned long)sq_info[sq_index].phy_addr);   // for test

	return 0;
}
EXPORT_SYMBOL(npu_create_dfx_sq);

void npu_destroy_dfx_sq(struct npu_dev_ctx *cur_dev_ctx, u32 sq_index)
{
	struct npu_dfx_sq_info *sq_info = NULL;
	struct npu_dfx_cqsq *cqsq = NULL;

	if (cur_dev_ctx == NULL || sq_index >= NPU_MAX_DFX_SQ_NUM) {
		npu_drv_err("invalid input argument\n");
		return;
	}

	cqsq = npu_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		npu_drv_err("cqsq is null\n");
		return;
	}

	sq_info = cqsq->sq_info;
	if (sq_info == NULL) {
		npu_drv_err("sq_info is null\n");
		return;
	}
	if (sq_info[sq_index].addr != NULL) {
		mutex_lock(&cqsq->dfx_mutex);
#ifdef PROFILING_USE_RESERVED_MEMORY
		iounmap(sq_info[sq_index].addr);
#else
		kfree(sq_info[sq_index].addr);
#endif
		sq_info[sq_index].slot_len = 0;
		sq_info[sq_index].addr = NULL;
		sq_info[sq_index].head = 0;
		sq_info[sq_index].tail = 0;
		sq_info[sq_index].credit = NPU_MAX_DFX_SQ_DEPTH;
		sq_info[sq_index].function = NPU_MAX_CQSQ_FUNC;
		sq_info[sq_index].phy_addr = 0;
		cqsq->sq_num++;
		mutex_unlock(&cqsq->dfx_mutex);
	}
}
EXPORT_SYMBOL(npu_destroy_dfx_sq);

int npu_dfx_send_sq(u32 devid, u32 sq_index, const u8 *buffer, u32 buf_len)
{
	struct npu_dfx_sq_info *sq_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_dfx_cqsq *cqsq = NULL;
	int credit;
	u8 *addr = NULL;
	u32 tail;
	int ret;

	if (devid >= NPU_MAX_DEVICE_NUM || sq_index >= NPU_MAX_DFX_SQ_NUM ||
		buffer == NULL || buf_len <= 0 ||
		buf_len > NPU_DFX_MAX_SQ_SLOT_LEN) {
		npu_drv_err("invalid input argument\n");
		return -EINVAL;
	}

	cur_dev_ctx = get_dev_ctx_by_id(devid);
	cond_return_error(cur_dev_ctx == NULL, -ENODEV,
		"cur_dev_ctx %d is null\n", devid);

	// ts_work_status 0: power down
	cond_return_error(cur_dev_ctx->ts_work_status == 0, NPU_TS_DOWN,
		"device is not working\n");

	cqsq = npu_get_dfx_cqsq_info(cur_dev_ctx);
	cond_return_error(cqsq == NULL, -ENOMEM, "cqsq is null\n");

	sq_info = cqsq->sq_info;
	cond_return_error(sq_info[sq_index].addr == NULL, -ENOMEM,
		"invalid sq, sq_index = %u\n", sq_index);

	tail = sq_info[sq_index].tail;
	cond_return_error(tail >= NPU_MAX_DFX_SQ_DEPTH, -EINVAL,
		"no available sq tail:%d\n", tail);
	credit = (sq_info->tail >= sq_info->head) ?
		(NPU_MAX_DFX_SQ_DEPTH - (sq_info->tail - sq_info->head + 1)) :
		(sq_info->head - sq_info->tail - 1);
	cond_return_error(credit <= 0, -ENOMEM, "no available sq slot\n");

	addr = sq_info[sq_index].addr + (unsigned long)tail *
		sq_info[sq_index].slot_len;

	ret = memcpy_s(addr, buf_len, buffer, buf_len);
	cond_return_error(ret != 0, -ENOMEM, "memcpy_s failed, ret=%d\n", ret);

	if (tail >= NPU_MAX_DFX_SQ_DEPTH - 1)
		tail = 0;
	else
		tail++;

	sq_info[sq_index].tail = tail;
	*sq_info[sq_index].doorbell = (u32)tail;

	npu_drv_debug("a new dfx sq cmd is sent, sq_index : %d, tail : %d,phy_addr: "
		"0x%llx, doorbell_addr: 0x%llx\n", sq_index, tail,
		sq_info[sq_index].phy_addr,
		(phys_addr_t) (uintptr_t) sq_info[sq_index].doorbell);
	return 0;
}
EXPORT_SYMBOL(npu_dfx_send_sq);
