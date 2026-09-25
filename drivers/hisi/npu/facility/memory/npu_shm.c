/*
 * npu_shm.c
 *
 * about npu shm
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
#include "npu_shm.h"

#include <linux/idr.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/swap.h>
#include <asm/uaccess.h>
#include <linux/gfp.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/tlbflush.h>

#include "devdrv_user_common.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "hisi_svm.h"

struct devdrv_mem_desc *g_sq_desc = NULL;
struct devdrv_mem_info g_shm_desc[NPU_DEV_NUM][DEVDRV_MAX_MEM];
static struct devdrv_continuous_mem g_continuous_mem[NPU_DEV_NUM];

static int devdrv_continuous_mem_init(u8 dev_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	void *cpu_addr = NULL;
	struct devdrv_platform_info *plat_info = NULL;
	dma_addr_t dma_handle = 0;
	u64 size = MEM_INIT_SIZE;

	spin_lock_init(&g_continuous_mem[dev_id].cm_spinlock);
	spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
	g_continuous_mem[dev_id].total_size = 0;
	spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("npu dev %d `s dev_ctx is null\n", dev_id);
		return -1;
	}
	dev_ctx->cm = &g_continuous_mem[dev_id];

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info failed\n");
	} else {
		cpu_addr = dma_alloc_coherent(DEVDRV_PLAT_GET_PDEV(plat_info), size, &dma_handle, GFP_KERNEL);
		if (cpu_addr == NULL) {
			NPU_DRV_ERR("fail to malloc cma mem size 0x%llx\n", size);
		} else {
			NPU_DRV_WARN("succ to malloc cma mem size 0x%llx\n", size);
			spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
			g_continuous_mem[dev_id].mem_info[SHARE_NUM_OCCUPIED].dma_handle = dma_handle;
			g_continuous_mem[dev_id].mem_info[SHARE_NUM_OCCUPIED].cpu_addr = cpu_addr;
			g_continuous_mem[dev_id].mem_info[SHARE_NUM_OCCUPIED].size = size;
			g_continuous_mem[dev_id].mem_info[SHARE_NUM_OCCUPIED].valid = 0;
			spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);
		}
	}

	return 0;
}

struct devdrv_ts_sq_info *devdrv_calc_sq_info(u8 devid, u32 index)
{
	struct devdrv_ts_sq_info *sq = NULL;
	u64 addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;

	NPU_DRV_DEBUG("sq_info base vaddr = 0x%llx\n", addr);

	sq = (struct devdrv_ts_sq_info *)(uintptr_t) (addr +
		(long)(unsigned)sizeof(struct devdrv_ts_sq_info) * (index));

	NPU_DRV_DEBUG("sq->index = 0x%x, sq->head = 0x%x, "
		"sq->tail = 0x%x, sq->credit = 0x%x, "
		"sq->uio_size = 0x%x, sq->uio_addr = 0x%llx, " "\n",
		sq->index, sq->head, sq->tail, sq->credit,
		sq->uio_size, sq->uio_addr);
	return sq;
}

struct devdrv_ts_cq_info *devdrv_calc_cq_info(u8 devid, u32 index)
{
	struct devdrv_ts_cq_info *cq = NULL;
	u64 addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
	NPU_DRV_DEBUG("cq_info base vaddr = 0x%llx\n", addr);

	cq = (struct devdrv_ts_cq_info *)(uintptr_t) (addr + DEVDRV_SQ_INFO_OCCUPY_SIZE +
		(long)(unsigned)sizeof(struct devdrv_ts_cq_info) * (index));

	NPU_DRV_DEBUG("cq->index = 0x%x, cq->head = 0x%x, "
		"cq->tail = 0x%x, cq->phase = 0x%x, "
		"cq->int_flag = 0x%x, cq->slot_size = 0x%x, "
		"cq->uio_addr = 0x%llx, cq->cq_sub = 0x%llx, \n",
		cq->index, cq->head, cq->tail, cq->phase,
		cq->int_flag, cq->slot_size, cq->uio_addr, cq->cq_sub);

	return cq;
}

struct devdrv_stream_info *devdrv_calc_stream_info(u8 devid, u32 index)
{
	struct devdrv_stream_info *stream_info = NULL;

	u64 addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
	stream_info = (struct devdrv_stream_info *)(uintptr_t) (addr +
		DEVDRV_SQ_INFO_OCCUPY_SIZE + DEVDRV_CQ_INFO_OCCUPY_SIZE +
		(long)(unsigned)sizeof(struct devdrv_stream_info) * (index));
	return stream_info;
}

u32 *devdrv_get_ts_work_status(u8 devid)
{
	u64 addr;
	u32 *ts_status = NULL;

	addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
	ts_status = (u32 *)(uintptr_t) (addr + DEVDRV_INFO_OCCUPY_SIZE);
	NPU_DRV_INFO("ts_status offset:0x%lx", DEVDRV_INFO_OCCUPY_SIZE);
	return ts_status;
}

int devdrv_shm_init(u8 dev_id)
{
	gfp_t gfp_flags = GFP_KERNEL | __GFP_COMP | __GFP_ZERO;
	char *tmp = NULL;
	struct devdrv_mem_desc *persistent_task_buf_desc = NULL;
	u32 *ts_status = NULL;
	struct devdrv_platform_info *plat_info = NULL;
	u32 order = 0;
	u32 doorbell_size;
	phys_addr_t doorbell_base ;

	NPU_DRV_DEBUG("dev_id = %u\n", dev_id);
	COND_RETURN_ERROR(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id = %d\n", dev_id);

	plat_info = devdrv_plat_get_info();
	COND_RETURN_ERROR(plat_info == NULL, -1, "devdrv_plat_get_info failed\n");

	order = DEVDRV_MAX_INFO_ORDER;
	doorbell_base = DEVDRV_PLAT_GET_REG_DESC(plat_info, DEVDRV_REG_TS_DOORBELL).base;
	doorbell_size = DEVDRV_PLAT_GET_REG_DESC(plat_info, DEVDRV_REG_TS_DOORBELL).len;
	persistent_task_buf_desc = DEVDRV_PLAT_GET_PERSISTENT_TASK_BUF(plat_info);
	g_sq_desc = DEVDRV_PLAT_GET_SQCQ_BUF(plat_info);

	tmp = (char *)(uintptr_t) __get_free_pages(gfp_flags, order);
	COND_RETURN_ERROR(tmp == NULL, -ENOMEM, "alloc share mem descriptor memory failed !\n");

	g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr = g_sq_desc->base + CHIP_BASEADDR_PA_OFFSET * dev_id;
	g_shm_desc[dev_id][DEVDRV_SQ_MEM].size = g_sq_desc->len;
	g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr = virt_to_phys(tmp);
	g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr = (vir_addr_t) (uintptr_t) tmp;
	g_shm_desc[dev_id][DEVDRV_INFO_MEM].size = (long)(unsigned)(1 << order) * PAGE_SIZE;
	g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].phy_addr = doorbell_base;
	g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].size = doorbell_size;
	g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].phy_addr = persistent_task_buf_desc->base;
	g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].size = persistent_task_buf_desc->len;

	ts_status = devdrv_get_ts_work_status(dev_id);
	*ts_status = DEVDRV_TS_DOWN;

	gfp_flags = GFP_KERNEL | __GFP_ZERO;
	tmp = (char *)(uintptr_t) __get_free_pages(gfp_flags, 0);
	COND_RETURN_ERROR(tmp == NULL, -ENOMEM, "alloc share mem pad memory failed\n");

	g_shm_desc[dev_id][DEVDRV_PAD_MEM].phy_addr = virt_to_phys(tmp);
	g_shm_desc[dev_id][DEVDRV_PAD_MEM].virt_addr = (vir_addr_t)(uintptr_t)tmp;
	g_shm_desc[dev_id][DEVDRV_PAD_MEM].size = PAGE_SIZE;

	NPU_DRV_DEBUG("sq mem: phy_addr = 0x%llx, size = %lu\n",
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr,
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].size);

	NPU_DRV_DEBUG("info mem: virt_addr = 0x%llx, order = %u, size = %lu\n",
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr,
		order, g_shm_desc[dev_id][DEVDRV_INFO_MEM].size);

	NPU_DRV_DEBUG("doorbell mem: phy_addr = 0x%llx, size = %lu\n",
		g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].phy_addr,
		g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].size);

	NPU_DRV_DEBUG("persistent task buf mem: phy_addr = 0x%llx, size = %lu\n",
		g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].phy_addr,
		g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].size);

	// init continuous memory resource
	return devdrv_continuous_mem_init(dev_id);
}

void devdrv_shm_destroy(u8 dev_id)
{
	unsigned long addr;
	u32 order;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id %d\n", dev_id);
		return;
	}

	order = DEVDRV_MAX_INFO_ORDER;
	addr = (unsigned long)g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr;
	free_pages(addr, order);
	addr = (unsigned long)g_shm_desc[dev_id][DEVDRV_PAD_MEM].virt_addr;
	free_pages(addr, 0);
}

struct devdrv_mem_desc *g_hwts_sq_desc = NULL;
struct devdrv_mem_desc *g_hwts_cq_desc = NULL;

struct devdrv_hwts_sq_info *devdrv_calc_hwts_sq_info(u8 devid, u32 index)
{
	struct devdrv_hwts_sq_info *hwts_sq_info = NULL;
	u64 addr;

	if (index >= DEVDRV_MAX_HWTS_SQ_INFO_NUM) {
		NPU_DRV_ERR("illegal npu dev id = %d, index=%d\n", devid, index);
		return NULL;
	}

	addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
	hwts_sq_info = (struct devdrv_hwts_sq_info *)(uintptr_t) (addr + DEVDRV_HWTS_SQ_INFO_OFFSET +
		(long)(unsigned)sizeof(struct devdrv_hwts_sq_info) * (index));

	return hwts_sq_info;
}
struct devdrv_hwts_cq_info *devdrv_calc_hwts_cq_info(u8 devid, u32 index)
{
	struct devdrv_hwts_cq_info *hwts_cq_info = NULL;
	u64 addr;

	if (index >= DEVDRV_MAX_HWTS_CQ_INFO_NUM) {
		NPU_DRV_ERR("illegal npu dev id = %d, index=%d\n", devid, index);
		return NULL;
	}

	addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
	hwts_cq_info = (struct devdrv_hwts_cq_info *)(uintptr_t) (addr + DEVDRV_HWTS_CQ_INFO_OFFSET +
		(long)(unsigned)sizeof(struct devdrv_hwts_cq_info) * (index));

	return hwts_cq_info;
}

struct devdrv_model_desc_info *devdrv_calc_model_desc_info(u8 devid, u32 index)
{
	struct devdrv_model_desc_info *model_desc_info = NULL;
	u64 addr;

	if (index >= DEVDRV_MAX_MODEL_ID) {
		NPU_DRV_ERR("illegal npu dev id = %d, model_id=%d\n", devid, index);
		return NULL;
	}

	addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
	model_desc_info = (struct devdrv_model_desc_info *)(uintptr_t) (addr + DEVDRV_MODEL_INFO_OFFSET +
		(long)(unsigned)sizeof(struct devdrv_model_desc_info) * (index));

	return model_desc_info;
}

int devdrv_shm_v200_init(u8 dev_id)
{
	struct devdrv_platform_info *plat_info = NULL;
	gfp_t gfp_flags;
	char *tmp = NULL;
	phys_addr_t doorbell_base;
	u32 doorbell_size;
	struct devdrv_mem_desc *persistent_task_buf_desc = NULL;
	u32 *ts_status = NULL;

	NPU_DRV_DEBUG("dev_id = %u\n", dev_id);
	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id = %d\n", dev_id);
		return -1;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info failed\n");
		return -1;
	}

	doorbell_base = DEVDRV_PLAT_GET_REG_DESC(plat_info, DEVDRV_REG_TS_DOORBELL).base;
	doorbell_size = DEVDRV_PLAT_GET_REG_DESC(plat_info, DEVDRV_REG_TS_DOORBELL).len;
	persistent_task_buf_desc = DEVDRV_PLAT_GET_PERSISTENT_TASK_BUF(plat_info);
	g_sq_desc = DEVDRV_PLAT_GET_SQCQ_BUF(plat_info);
	g_hwts_sq_desc = DEVDRV_PLAT_GET_HWTS_SQ_BUF(plat_info);
	g_hwts_cq_desc = DEVDRV_PLAT_GET_HWTS_CQ_BUF(plat_info);

	g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr = g_sq_desc->base + CHIP_BASEADDR_PA_OFFSET * dev_id;
	g_shm_desc[dev_id][DEVDRV_SQ_MEM].size = g_sq_desc->len;

	NPU_DRV_DEBUG("sqcq mem: phy_addr = 0x%llx, size = %lu, "
		"sq phy_addr = 0x%llx, cq phy_addr = 0x%llx, "
		"dfx sq phy_addr = 0x%llx, dfx cq phy_addr = 0x%llx \n",
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr,
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].size,
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr,
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr + DEVDRV_SQ_OCCUPY_SIZE,
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr + DEVDRV_SQ_OCCUPY_SIZE + DEVDRV_CQ_OCCUPY_SIZE,
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr + DEVDRV_SQ_OCCUPY_SIZE +
			DEVDRV_CQ_OCCUPY_SIZE + DEVDRV_DFX_SQ_OCCUPY_SIZE);

	g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr = DEVDRV_PLAT_GET_INFO_BUF(plat_info)->base;
	g_shm_desc[dev_id][DEVDRV_INFO_MEM].size = DEVDRV_PLAT_GET_INFO_BUF(plat_info)->len;
	g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr = (unsigned long long)(uintptr_t)ioremap_nocache(
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr, g_shm_desc[dev_id][DEVDRV_INFO_MEM].size);
	if ((uintptr_t)g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr == 0) {
		NPU_DRV_ERR("ioremap_nocache failed!");
		return -1;
	}

	NPU_DRV_DEBUG("info mem: phy_addr = 0x%llx, size = %lu, "
		"sqinfo phy_addr = 0x%llx, cqinfo phy_addr = 0x%llx, "
		"streaminfo phy_addr = 0x%llx, hwtssqinfo phy_addr = 0x%llx, "
		"hwtscqinfo phy_addr = 0x%llx, modelinfo phy_addr = 0x%llx \n",
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr,
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].size,
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr,
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr + DEVDRV_SQ_INFO_OCCUPY_SIZE,
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr + DEVDRV_SQ_INFO_OCCUPY_SIZE + DEVDRV_CQ_INFO_OCCUPY_SIZE,
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr + DEVDRV_SQ_INFO_OCCUPY_SIZE +
			DEVDRV_CQ_INFO_OCCUPY_SIZE + DEVDRV_STREAM_INFO_OCCUPY_SIZE,
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr + DEVDRV_SQ_INFO_OCCUPY_SIZE +
			DEVDRV_CQ_INFO_OCCUPY_SIZE + DEVDRV_STREAM_INFO_OCCUPY_SIZE + DEVDRV_HWTS_SQ_INFO_OCCUPY_SIZE,
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr + DEVDRV_SQ_INFO_OCCUPY_SIZE +
			DEVDRV_CQ_INFO_OCCUPY_SIZE + DEVDRV_STREAM_INFO_OCCUPY_SIZE +
			DEVDRV_HWTS_SQ_INFO_OCCUPY_SIZE + DEVDRV_HWTS_CQ_INFO_OCCUPY_SIZE);

	g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].phy_addr = doorbell_base;
	g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].size = doorbell_size;

	ts_status = devdrv_get_ts_work_status(dev_id);
	*ts_status = DEVDRV_TS_DOWN;

	gfp_flags = GFP_KERNEL | __GFP_ZERO;
	tmp = (char *)(uintptr_t) __get_free_pages(gfp_flags, 0);
	if (tmp == NULL) {
		NPU_DRV_ERR("alloc share mem pad memory failed.n");
		return -ENOMEM;
	}
	g_shm_desc[dev_id][DEVDRV_PAD_MEM].phy_addr = virt_to_phys(tmp);
	g_shm_desc[dev_id][DEVDRV_PAD_MEM].virt_addr = (vir_addr_t)(uintptr_t) tmp;
	g_shm_desc[dev_id][DEVDRV_PAD_MEM].size = PAGE_SIZE;

	NPU_DRV_DEBUG("sq mem: phy_addr = 0x%llx, size = %lu\n",
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr,
		g_shm_desc[dev_id][DEVDRV_SQ_MEM].size);

	NPU_DRV_DEBUG("info mem: virt_addr = 0x%llx, size = %lu\n",
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr,
		g_shm_desc[dev_id][DEVDRV_INFO_MEM].size);

	NPU_DRV_DEBUG("doorbell mem: phy_addr = 0x%llx, size = %lu\n",
		g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].phy_addr,
		g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].size);

	NPU_DRV_DEBUG("pad buf mem: phy_addr = 0x%llx, size = %lu\n",
		g_shm_desc[dev_id][DEVDRV_PAD_MEM].phy_addr,
		g_shm_desc[dev_id][DEVDRV_PAD_MEM].size);

	// init continuous memory resource
	return devdrv_continuous_mem_init(dev_id);
}

void devdrv_shm_v200_destroy(u8 dev_id)
{
	unsigned long addr;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id %d\n", dev_id);
		return;
	}

	addr = (unsigned long)g_shm_desc[dev_id][DEVDRV_PAD_MEM].virt_addr;
	free_pages(addr, 0);
}

int devdrv_save_cm_info(struct devdrv_dev_ctx *dev_ctx, struct devdrv_cm_info *info, u32 *share_num)
{
	int i;
	u8 dev_id;

	if (dev_ctx == NULL) {
		NPU_DRV_ERR("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		NPU_DRV_ERR("invalid cm value\n");
		return -EINVAL;
	}

	if (info == NULL) {
		NPU_DRV_ERR("cm info is null\n");
		return -EINVAL;
	}

	if (share_num == NULL) {
		NPU_DRV_ERR("share_num addr is null\n");
		return -EINVAL;
	}

	dev_id = dev_ctx->devid;
	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id = %d\n", dev_id);
		return -EINVAL;
	}

	for (i = 0; i < MAX_MEM_INFO_NUM; i++) {
		if (dev_ctx->cm->mem_info[i].valid != MEM_INFO_VALID) {
			spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
			dev_ctx->cm->mem_info[i].dma_handle = info->dma_handle;
			dev_ctx->cm->mem_info[i].cpu_addr = info->cpu_addr;
			dev_ctx->cm->mem_info[i].size = info->size;
			dev_ctx->cm->mem_info[i].valid = MEM_INFO_VALID;
			*share_num = (u32)i;
			spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);
			return 0;
		} else if (info->dma_handle == dev_ctx->cm->mem_info[i].dma_handle) {
			NPU_DRV_ERR("repeat insert npu cm l2 ctrl addr\n");
			return -EINVAL;
		}
	}

	return -EINVAL;
}

int devdrv_save_cm_info_occupied(struct devdrv_dev_ctx *dev_ctx, u32 *share_num)
{
	u8 dev_id;

	if (dev_ctx == NULL) {
		NPU_DRV_ERR("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		NPU_DRV_ERR("invalid cm value\n");
		return -EINVAL;
	}

	if (share_num == NULL) {
		NPU_DRV_ERR("share_num addr is null\n");
		return -EINVAL;
	}

	dev_id = dev_ctx->devid;
	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id = %d\n", dev_id);
		return -EINVAL;
	}

	spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
	dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].valid = MEM_INFO_VALID;
	*share_num = SHARE_NUM_OCCUPIED;
	spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);
	return 0;
}

int devdrv_delete_cm_info(struct devdrv_dev_ctx *dev_ctx, u32 share_num)
{
	u8 dev_id;

	if (dev_ctx == NULL) {
		NPU_DRV_ERR("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		NPU_DRV_ERR("invalid cm value\n");
		return -EINVAL;
	}

	dev_id = dev_ctx->devid;
	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id = %d\n", dev_id);
		return -EINVAL;
	}

	if (share_num >= MAX_MEM_INFO_NUM) {
		NPU_DRV_ERR("invalid share_num %d\n", share_num);
		return -EINVAL;
	}

	if (dev_ctx->cm->mem_info[share_num].valid != MEM_INFO_VALID) {
		NPU_DRV_ERR("invalid info ,no need to delete %d\n", dev_ctx->cm->mem_info[share_num].valid);
		return -EINVAL;
	}

	spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
	dev_ctx->cm->mem_info[share_num].dma_handle = 0;
	dev_ctx->cm->mem_info[share_num].cpu_addr = 0;
	dev_ctx->cm->mem_info[share_num].size = 0;
	dev_ctx->cm->mem_info[share_num].valid = 0;
	dev_ctx->cm->mem_info[share_num].user_va = 0;
	spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);

	return 0;
}

int devdrv_delete_cm_info_occupied(struct devdrv_dev_ctx *dev_ctx)
{
	u8 dev_id;

	if (dev_ctx == NULL) {
		NPU_DRV_ERR("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		NPU_DRV_ERR("invalid cm value\n");
		return -EINVAL;
	}

	dev_id = dev_ctx->devid;
	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id = %d\n", dev_id);
		return -EINVAL;
	}

	if (dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].valid != MEM_INFO_VALID) {
		NPU_DRV_ERR("invalid info ,no need to delete %d\n", dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].valid);
		return -EINVAL;
	}

	spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
	dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].valid = 0;
	spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);

	return 0;
}

int devdrv_inc_cm_total_size(struct devdrv_dev_ctx *dev_ctx, u64 size)
{
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		NPU_DRV_ERR("invalid cm value\n");
		return -EINVAL;
	}

	spin_lock(&dev_ctx->cm->cm_spinlock);
	dev_ctx->cm->total_size += size;
	spin_unlock(&dev_ctx->cm->cm_spinlock);

	NPU_DRV_DEBUG("total ocuppied cm size = %llx after request now\n", dev_ctx->cm->total_size);

	return 0;
}

static int devdrv_get_cm_info(u8 dev_id, u32 share_num, struct devdrv_cm_info *info)
{
	spin_lock(&g_continuous_mem[dev_id].cm_spinlock);

	info->dma_handle = g_continuous_mem[dev_id].mem_info[share_num].dma_handle;
	info->cpu_addr = g_continuous_mem[dev_id].mem_info[share_num].cpu_addr;
	info->size = g_continuous_mem[dev_id].mem_info[share_num].size;
	spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);

	return 0;
}

int devdrv_dec_cm_total_size(struct devdrv_dev_ctx *dev_ctx, u64 size)
{
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		NPU_DRV_ERR("invalid cm value\n");
		return -EINVAL;
	}

	spin_lock(&dev_ctx->cm->cm_spinlock);
	dev_ctx->cm->total_size -= size;
	spin_unlock(&dev_ctx->cm->cm_spinlock);

	NPU_DRV_DEBUG("total ocuppied cm size = 0x%llx after free now\n", dev_ctx->cm->total_size);

	return 0;
}

int devdrv_is_cm_available(const struct devdrv_dev_ctx *dev_ctx, u64 req_size)
{
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		NPU_DRV_ERR("invalid cm value\n");
		return -EINVAL;
	}

	if (dev_ctx->cm->total_size >= MEM_TOTAL_SIZE || req_size > MEM_TOTAL_SIZE || req_size == 0) {
		NPU_DRV_ERR("total size = 0x%llx, req_size = 0x%llx\n", dev_ctx->cm->total_size, req_size);
		return -EINVAL;
	}

	NPU_DRV_DEBUG("current cm occupied total size = 0x%llx, "
		"request_size = 0x%llx\n dev mem total size = 0x%x \n",
		dev_ctx->cm->total_size, req_size, MEM_TOTAL_SIZE);

	if (req_size > MEM_TOTAL_SIZE) {
		NPU_DRV_ERR("req_size = 0x%llx is oversize\n", req_size);
		return -EINVAL;
	}

	if (dev_ctx->cm->total_size > (MEM_TOTAL_SIZE - req_size)) {
		NPU_DRV_ERR("left cm size is no enough now\n");
		return -EINVAL;
	}

	return 0;
}

int devdrv_is_cm_valid(const struct devdrv_dev_ctx *dev_ctx, u32 share_num)
{
	u64 total_size;
	u64 size;

	if (dev_ctx == NULL) {
		NPU_DRV_ERR("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		NPU_DRV_ERR("invalid cm value\n");
		return -EINVAL;
	}

	if (share_num >= MAX_MEM_INFO_NUM) {
		NPU_DRV_ERR("invalid share_num %d\n", share_num);
		return -EINVAL;
	}

	if (dev_ctx->cm->mem_info[share_num].valid != MEM_INFO_VALID) {
		NPU_DRV_ERR("invalid cm info valid_val =  %d share_num = %d\n",
			dev_ctx->cm->mem_info[share_num].valid, share_num);
		return -EINVAL;
	}

	size = dev_ctx->cm->mem_info[share_num].size;
	total_size = dev_ctx->cm->total_size;
	if (size > total_size) {
		NPU_DRV_ERR("share_num = %d invalid free_size 0x%llx "
			"total size 0x%llx\n", share_num, size, total_size);
		return -EINVAL;
	}

	return 0;
}

int devdrv_devmem_swapin(struct vm_area_struct *vma, unsigned long devmem_base,
                         unsigned long size, unsigned long align_size)
{
	int err = 0;
	unsigned long vma_start_aligned;
	COND_RETURN_ERROR (vma == NULL, -EFAULT, "vma is NULL\n");

	vma_start_aligned = ALIGN_UP(vma->vm_start, align_size);
	NPU_DRV_DEBUG(
		"zap_vma_ptes.devmem_base=0x%lx, vma_start_aligned=0x%lx, size=0x%lx, align_size=0x%lx, err=%d\n",
		devmem_base, vma_start_aligned, size, align_size, err);
	NPU_DRV_DEBUG("vm_start = 0x%lx vm_end = 0x%lx vm_flags=0x%x vm_page_prot=0x%x",
		vma->vm_start, vma->vm_end, vma->vm_flags, vma->vm_page_prot);

	err = zap_vma_ptes(vma, vma_start_aligned, size);
	if (err) {
		NPU_DRV_ERR(
			"zap_vma_ptes failed.devmem_base=0x%lx, vma_start_aligned=0x%lx, size=0x%lx,align_size=0x%lx, err=%d\n",
			devmem_base, vma_start_aligned, size, align_size, err);
		NPU_DRV_ERR("vm_start = 0x%lx vm_end = 0x%lx vm_flags=0x%x vm_page_prot=0x%x",
			vma->vm_start, vma->vm_end, vma->vm_flags, vma->vm_page_prot);
		return -EFAULT;
	}
	COND_RETURN_ERROR(size <= 0, -ENOMEM," size = %lu\n", size);

	if (remap_pfn_range(vma, vma_start_aligned, __phys_to_pfn(devmem_base), size, vma->vm_page_prot)) {
		NPU_DRV_ERR("fail to map body mem.vma_start_aligned=0x%lx, align_size=0x%lx\n",
			vma_start_aligned, align_size);
		return -ENXIO;
	}
	flush_tlb_all();
	return 0;
}

int devdrv_devmem_swapout(struct vm_area_struct *vma, unsigned long pad_base,
                          unsigned long size, unsigned long align_size)
{
	int err = 0;
	unsigned long vma_start_aligned;
	unsigned long pad_start;
	unsigned long vma_tmp;

	vma_start_aligned = ALIGN_UP(vma->vm_start, align_size);

	NPU_DRV_DEBUG("vma_start_aligned=0x%lx, size=0x%lx, align_size=0x%lx, err=%d\n",
		vma_start_aligned, size, align_size, err);
	NPU_DRV_DEBUG("vm_start = 0x%lx vm_end = 0x%lx vm_flags=0x%x vm_page_prot=0x%x",
		vma->vm_start, vma->vm_end, vma->vm_flags, vma->vm_page_prot);

	err = zap_vma_ptes(vma, vma_start_aligned, size);
	if (err) {
		NPU_DRV_ERR("zap_vma_ptes failed.vma_start_aligned=0x%lx, size=0x%lx, align_size=0x%lx, err=%d\n",
			vma_start_aligned, size, align_size, err);
		NPU_DRV_ERR("vm_start = 0x%lx vm_end = 0x%lx vm_flags=0x%x vm_page_prot=0x%x",
			vma->vm_start, vma->vm_end, vma->vm_flags, vma->vm_page_prot);
		return -EFAULT;
	}

	vma_tmp = vma_start_aligned + size;
	COND_RETURN_ERROR(size <= 0, -ENOMEM," size = %lu\n", size);
	for (pad_start = vma_start_aligned; pad_start < vma_tmp; pad_start += PAGE_SIZE) {
		if (remap_pfn_range(vma, pad_start, __phys_to_pfn(pad_base), PAGE_SIZE, vma->vm_page_prot)) {
			NPU_DRV_ERR("fail to map pad mem\n");
			return -ENXIO;
		}
	}
	flush_tlb_all();
	return 0;
}

bool devdrv_vma_valid(const struct vm_area_struct *vma)
{
	struct mm_struct *vm_mm = NULL;
	struct vm_area_struct *vma_node = NULL;
	int i;
	int map_count;

	if (vma == NULL) {
		NPU_DRV_ERR("vma is null\n");
		return false;
	}

	vm_mm = current->mm;
	if (vm_mm == NULL) {
		NPU_DRV_ERR("vm_mm is null\n");
		return false;
	}

	vma_node = vm_mm->mmap; /* list of VMAs, first node */
	map_count = vm_mm->map_count;

	for (i = 0; i < map_count && vma_node != NULL; i++) {
		if (vma_node == vma) {
			NPU_DRV_INFO("VALID.%d vma=%p vm_mm=%p vm_next=%p vm_start=0x%llx, vm_end =0x%llx, map_count=%d\n",
				i, vma, vma->vm_mm, vma->vm_next, vma->vm_start, vma->vm_end, map_count);
			return true;
		}
		vma_node = vma_node->vm_next;
	}
	NPU_DRV_INFO("UNVALID.%d vma=%p vm_mm=%p vm_next=%p vm_start=0x%llx, vm_end =0x%llx, map_count=%d\n",
		i, vma, vma->vm_mm, vma->vm_next, vma->vm_start, vma->vm_end, map_count);

	return false;
}

int l2_mem_swapin(struct vm_area_struct *vma)
{
	struct devdrv_mem_desc *l2_desc = NULL;
	unsigned long l2_len;
	int err;

	l2_desc = devdrv_plat_get_reg_desc(DEVDRV_REG_L2BUF_BASE);
	if (l2_desc == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_reg_desc failed\n");
		return -EFAULT;
	}

	l2_len = l2_desc->len + 1; // becasue of dts will minus 1

	err = devdrv_devmem_swapin(vma, l2_desc->base, l2_len, l2_len);
	if (err) {
		NPU_DRV_ERR("devdrv_devmem_swapin failed.l2_desc->base = 0x%lx, l2_len=0x%lx\n",
			l2_desc->base, l2_len);
		return -1;
	}
	NPU_DRV_WARN("l2_mem_swapin exit");
	return 0;
}

int l2_mem_swapout(struct vm_area_struct *vma, u8 dev_id)
{
	struct devdrv_mem_desc *l2_desc = NULL;
	unsigned long l2_len;
	unsigned long pad_base;
	int err;
	NPU_DRV_DEBUG(" enter");

	l2_desc = devdrv_plat_get_reg_desc(DEVDRV_REG_L2BUF_BASE);
	if (l2_desc == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_reg_desc failed\n");
		return -EFAULT;
	}
	pad_base = g_shm_desc[dev_id][DEVDRV_PAD_MEM].phy_addr;
	if (pad_base == 0) {
		NPU_DRV_ERR("invalid pad_base\n");
		return -EFAULT;
	}

	l2_len = l2_desc->len + 1; // becasue of dts will minus 1

	err = devdrv_devmem_swapout(vma, pad_base, l2_len, l2_len);
	if (err) {
		NPU_DRV_ERR("devdrv_devmem_swapout failed.pad_base= 0x%lx, l2_len=0x%lx\n", pad_base, l2_len);
		return -1;
	}
	NPU_DRV_WARN("l2_mem_swapout exit");
	return 0;
}

int devdrv_map_l2_buff(const struct file *filp, struct vm_area_struct *vma, u8 dev_id)
{
	unsigned long pad_base, pad_start;

	NPU_DRV_DEBUG("map l2 buff enter");

	if ((vma == NULL) || (filp == NULL) || (dev_id >= NPU_DEV_NUM)) {
		NPU_DRV_ERR("invalid para\n");
		return -EFAULT;
	}

	NPU_DRV_INFO("vma=%p vm_mm=%p vm_next=%p vm_start=0x%llx, vm_end =0x%llx\n",
		vma, vma->vm_mm, vma->vm_next, vma->vm_start, vma->vm_end);

	/* we do not want to have this area swapped out, lock it */
	vma->vm_flags |= VM_LOCKED;

	pad_base = g_shm_desc[dev_id][DEVDRV_PAD_MEM].phy_addr;
	if (pad_base == 0) {
		NPU_DRV_ERR("invalid mem base\n");
		return -EFAULT;
	}

	/**
	 * map head with the pad page
	 */
	for (pad_start = vma->vm_start; pad_start < vma->vm_end; pad_start += PAGE_SIZE) {
		if (remap_pfn_range(vma, pad_start, __phys_to_pfn(pad_base), PAGE_SIZE, vma->vm_page_prot)) {
			NPU_DRV_ERR("fail to map pad mem\n");
			return -ENXIO;
		}
	}

	NPU_DRV_DEBUG("map l2 buff success");
	return 0;
}

// for l2ctrl using currently
int devdrv_map_cm(const struct devdrv_proc_ctx *proc_ctx, struct vm_area_struct *vma,
		  u32 share_num, u8 dev_id)
{
	struct devdrv_cm_info info = { 0 };
	unsigned long pfn;
	unsigned long size;
	int ret;
	struct mm_struct *mm = NULL;
	COND_RETURN_ERROR(dev_id >= NPU_DEV_NUM, -EFAULT, "illegal npu dev id %d\n", dev_id);

	COND_RETURN_ERROR(share_num >= MAX_MEM_INFO_NUM, -EFAULT, "illegal share_num %d\n", share_num);

	COND_RETURN_ERROR(proc_ctx == NULL, -EFAULT, "proc_ctx is NULL\n");

	COND_RETURN_ERROR(proc_ctx->map_ops_flag != true, -EFAULT, "map_ops_flag is false. cannot map\n");
	size = vma->vm_end - vma->vm_start;

	ret = devdrv_get_cm_info(dev_id, share_num, &info);
	COND_RETURN_ERROR(ret != 0, -EFAULT, "get continue mem info fail ret %d\n", ret);

	pfn = info.dma_handle >> PAGE_SHIFT;
	/* we do not want to have this area swapped out, lock it */
	vma->vm_flags |= VM_LOCKED;

	COND_RETURN_ERROR((size != info.size) || (size > MAP_MAX_SIZE), -ENOMEM, "invalid cm size = %lu\n", size);

	NPU_DRV_INFO("vma_cm=%p vm_next=%p vm_start=0x%llx, vm_end =0x%llx\n",
		vma, vma->vm_next, vma->vm_start, vma->vm_end);

	vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot, 0, PTE_DIRTY);
	ret = remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot);
	COND_RETURN_ERROR(ret != 0, -ENXIO, "remap_pfn_range cm failed ret = %d\n", ret);

	mm = current->mm;
	COND_RETURN_ERROR(mm == NULL, -ENXIO);
	COND_RETURN_ERROR(hisi_svm_flush_cache(mm, vma->vm_start, size), -ENXIO);

	return ret;
}

int devdrv_cm_resource_recycle(struct devdrv_dev_ctx *dev_ctx)
{
	struct devdrv_platform_info *plat_info = NULL;
	int i = 0;
	u64 size = 0;
	if (dev_ctx == NULL) {
		NPU_DRV_ERR("invalid dev_ctx value\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		NPU_DRV_ERR("invalid cm value\n");
		return -EINVAL;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info failed\n");
		return -EINVAL;
	}

	for (i = 0; i < MAX_MEM_INFO_NUM; i++) {
		if (dev_ctx->cm->mem_info[i].valid != MEM_INFO_VALID) {
			continue;
		}
		size = dev_ctx->cm->mem_info[i].size;
		if ((dev_ctx->cm->total_size) < size) {
			NPU_DRV_WARN("invalid size 0x%llx total size 0x%llx\n", size, dev_ctx->cm->total_size);
			return -EINVAL;
		}
		NPU_DRV_WARN("recycle cm mem %d size 0x%llx \n", i, size);
		dma_free_coherent(DEVDRV_PLAT_GET_PDEV(plat_info), size, dev_ctx->cm->mem_info[i].cpu_addr,
			dev_ctx->cm->mem_info[i].dma_handle);

		(void)devdrv_delete_cm_info(dev_ctx, i);
		spin_lock(&dev_ctx->cm->cm_spinlock);
		dev_ctx->cm->total_size -= size;
		spin_unlock(&dev_ctx->cm->cm_spinlock);
		NPU_DRV_WARN("recycle cm mem %d size 0x%llx " "cm->total_size(occupied) = 0x%llx \n",
			i, size, dev_ctx->cm->total_size);
	}

	return 0;
}

// map dev_id npu's info a€sq and cq to user space
int devdrv_info_sq_cq_mmap(u8 dev_id, const struct file *filep, struct vm_area_struct *vma)
{
	int err;
	size_t size;
	unsigned long start_addr = 0;
	phys_addr_t phy_addr;
	struct devdrv_platform_info* plat_info = devdrv_plat_get_info();

	COND_RETURN_ERROR(plat_info == NULL, -EINVAL, "devdrv_plat_get_info\n");
	COND_RETURN_ERROR(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id %d\n", dev_id);
	COND_RETURN_ERROR(vma == NULL, -1, "vma is null\n");

	NPU_DRV_DEBUG("vma->vm_end = %pK  vma->vm_start = %pK \n",
		(void *)(uintptr_t) vma->vm_end, (void *)(uintptr_t) vma->vm_start);

	COND_RETURN_ERROR(vma->vm_end < vma->vm_start, -EINVAL, "vma->vm_end < vma->vm_start, invalid vma para\n");

	vma->vm_flags |= VM_LOCKED;
	vma->vm_flags |= VM_DONTEXPAND;
	vma->vm_flags |= VM_PFNMAP;
	vma->vm_flags |= VM_WRITE;

	/*
	   |___SQ(32MB)___|____INFO(32MB)_____|__DOORBELL(32MB)___|___CQ(32MB)___|(32M vitural address space respectively)
	 */
	start_addr = vma->vm_start;
	phy_addr = g_sq_desc->base;
	NPU_DRV_DEBUG("npu dev %d sq mem: user_virt_addr=0x%lx, phys_addr=0x%llx\n", dev_id, start_addr, phy_addr);

	/* remap sq cq pfn range for user space */
	err = remap_pfn_range(vma, start_addr, phy_addr >> PAGE_SHIFT,
		DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE * DEVDRV_MAX_SQ_NUM, vma->vm_page_prot);
	COND_RETURN_ERROR(err != 0, -EINVAL, "npu dev_id %d sq memory mmap failed\n", dev_id);

	/* remap info pfn range for user space */
	phy_addr = g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr;
	size = g_shm_desc[dev_id][DEVDRV_INFO_MEM].size;
	start_addr += DEVDRV_VM_BLOCK_OFFSET;  // gap to reduce memory override probability
	COND_RETURN_ERROR(size <= 0, -ENOMEM, "npu dev %d illegal info mem size = %lu\n", dev_id, size);

	NPU_DRV_DEBUG("npu dev %d info mem:user_virt_addr = 0x%lx, "
		"phys_addr = 0x%llx, size = %lu\n", dev_id, start_addr, phy_addr, size);
	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1)
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	err = remap_pfn_range(vma, start_addr, phy_addr >> PAGE_SHIFT, size, vma->vm_page_prot);
	if (err) {
		NPU_DRV_ERR("npu dev_id %d info memory mmap failed\n", dev_id);
		return -EINVAL;
	}
	/* remap doorbell pfn range for user space temporarily,will delete later */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	start_addr += DEVDRV_VM_BLOCK_OFFSET;
	phy_addr = g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].phy_addr;
	size = g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].size;
	COND_RETURN_ERROR(size <= 0, -ENOMEM, "npu dev %d illegal doorbell cfg size = %lu\n", dev_id, size);
	NPU_DRV_DEBUG("npu dev %d doorbell mem:user_virt_addr = 0x%lx, "
		"phys_addr = 0x%llx, size = %lu\n", dev_id, start_addr, phy_addr, size);

	err = remap_pfn_range(vma, start_addr, phy_addr >> PAGE_SHIFT, size, vma->vm_page_prot);
	COND_RETURN_ERROR(err != 0, -EINVAL, "npu dev_id %d doobell register mmap failed\n", dev_id);

	// cq non cache solution
	start_addr += DEVDRV_VM_BLOCK_OFFSET;
	phy_addr = (unsigned long long)(g_sq_desc->base +
		DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE * DEVDRV_MAX_SQ_NUM);
	NPU_DRV_DEBUG("npu dev %d cq mem:user_virt_addr=0x%lx, phys_addr=0x%llx\n", dev_id, start_addr, phy_addr);

	err = remap_pfn_range(vma, start_addr, phy_addr >> PAGE_SHIFT,
		DEVDRV_MAX_CQ_DEPTH * DEVDRV_CQ_SLOT_SIZE, vma->vm_page_prot);
	COND_RETURN_ERROR(err != 0, -EINVAL, "npu dev_id %d cq mem mmap failed\n", dev_id);

	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 0) {
		// persistent task buff
		start_addr += DEVDRV_VM_BLOCK_OFFSET;
		phy_addr =  g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].phy_addr;
		size =  g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].size;
		COND_RETURN_ERROR(size <= 0, -ENOMEM, "npu dev %d illegal info mem size = %lu\n", dev_id, size);

		NPU_DRV_DEBUG("npu dev %d persistent task buff mem:user_virt_addr = 0x%lx, "
			"phys_addr = 0x%llx, size = %lu\n", dev_id, start_addr, phy_addr, size);
		err = remap_pfn_range(vma, start_addr, phy_addr >> PAGE_SHIFT, size, pgprot_writecombine(PAGE_SHARED));
		if (err) {
			NPU_DRV_ERR("npu dev_id %d persistent task buff mem mmap failed\n", dev_id);
			return -EINVAL;
		}
	}

	return 0;
}
