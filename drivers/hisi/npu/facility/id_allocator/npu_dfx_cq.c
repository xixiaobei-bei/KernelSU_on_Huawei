/*
 * npu_dfx_cq.c
 *
 * about npu dfx cq
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
#include "npu_dfx_cq.h"

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <securec.h>

#include "npu_dfx.h"
#include "npu_manager.h"
#include "npu_doorbell.h"
#include "npu_common.h"
#include "npu_pm_framework.h"
#include "npu_shm.h"
#include "npu_cache.h"
#include "npu_log.h"
#include "npu_mailbox.h"
#include "npu_platform.h"
#include "npu_user_common.h"
#include "npu_adapter.h"
#include "npu_heart_beat.h"

static void npu_heart_beat_work(struct work_struct *work)
{
	struct npu_dfx_cq_report *report = NULL;
	struct npu_dfx_sq_info *sq_info = NULL;
	struct npu_dfx_cq_info *cq_info = NULL;
	struct npu_dfx_cqsq *cqsq = NULL;
	u32 sq_slot_index;
	u32 sq_index;
	u8 *sq_slot = NULL;
	u8 *cq_slot = NULL;
	u8 *cq_data = NULL;
	int exception_flag = 0;
	u8 cq_data_tmp[NPU_DFX_MAX_CQ_SLOT_LEN];

	cond_return_void(work == NULL, "work is null\n");

	cq_info = container_of(work, struct npu_dfx_cq_info, work);
	cqsq = (struct npu_dfx_cqsq *)cq_info->dfx_cqsq_ptr;
	cond_return_void(cqsq == NULL, "dfx_cqsq_ptr is null\n");
	mutex_lock(&cq_info->lock);
	mutex_lock(&cqsq->dfx_mutex);

	cond_goto_error(cq_info->addr == NULL, fail, sq_index, 0);
	cq_slot = cq_info->addr + (unsigned long)cq_info->tail * cq_info->slot_len;
	while (cq_slot[0] == cq_info->phase) {
		report = (struct npu_dfx_cq_report *)cq_slot;
		sq_index = report->sq_index;
		sq_info = &cqsq->sq_info[sq_index];
		cond_goto_error(sq_info->addr == NULL, fail, sq_index, 0);
		sq_slot_index = (u32)((report->sq_head == 0) ?
			(NPU_MAX_DFX_SQ_DEPTH - 1) : (report->sq_head - 1));
		sq_slot = sq_info->addr + (unsigned long)sq_slot_index *
			sq_info->slot_len;
		cq_data = cq_slot + NPU_DFX_DETAILED_CQ_OFFSET;

		npu_drv_debug("call cq callback, cq id: %d\n", cq_info->index);

		if (npu_heart_beat_is_exception(cq_data) == 1) {
			npu_drv_warn("it is a exception heart!\n");
			(void)memcpy_s(cq_data_tmp, NPU_DFX_MAX_CQ_SLOT_LEN, cq_data,
				cq_info->slot_len - NPU_DFX_DETAILED_CQ_OFFSET);
			exception_flag = 1;
			break;
		}

		if (cq_info->callback != NULL)
			cq_info->callback(cq_data, sq_slot);

		sq_info->head = report->sq_head;
		if (cq_info->tail >= NPU_MAX_DFX_CQ_DEPTH - 1) {
			cq_info->phase = (u32)((cq_info->phase == NPU_DFX_PHASE_ONE) ?
				NPU_DFX_PHASE_ZERO : NPU_DFX_PHASE_ONE);
			cq_info->tail = 0;
		} else {
			cq_info->tail++;
		}
		cq_slot = cq_info->addr + (unsigned long)cq_info->tail *
			cq_info->slot_len;
	}

	cq_info->head = cq_info->tail;
	*cq_info->doorbell = (u32) cq_info->tail;

fail:
	mutex_unlock(&cqsq->dfx_mutex);
	mutex_unlock(&cq_info->lock);
	if (exception_flag == 1)
		npu_heart_beat_exception_process(cq_data_tmp, NPU_DFX_MAX_CQ_SLOT_LEN);
}

static void npu_get_dfx_cq_report(struct npu_dfx_int_context *int_context,
	struct npu_dfx_cq_info *cq_info)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (cq_info->function != NPU_CQSQ_HEART_BEAT) {
		queue_work(int_context->wq, &cq_info->work);
		return;
	}
	cur_dev_ctx = get_dev_ctx_by_id(cq_info->devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", cq_info->devid);
		return;
	}

	if (cur_dev_ctx->heart_beat.hb_wq != NULL) {
		queue_work(cur_dev_ctx->heart_beat.hb_wq, &cq_info->work);
		return;
	}
}

static int npu_get_dfx_cq_memory(struct npu_dfx_cq_info *cq_info,
	u32 size)
{
	struct npu_platform_info *plat_info = NULL;
	unsigned int cnt;
	unsigned int buf_size;

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return -EINVAL;
	}

	buf_size = NPU_MAX_DFX_CQ_DEPTH * NPU_DFX_MAX_CQ_SLOT_LEN;
	if (size > buf_size) {
		npu_drv_err("cq size=0x%x > 0x%x error\n", size, buf_size);
		return -ENOMEM;
	}
	// phy_addr:sq_calc_size + cq_calc_size + sq_dfx_size + off_size
	cq_info->phy_addr = (unsigned long long)(g_sq_desc->base +
		NPU_DFX_SQ_OFFSET +
		(NPU_MAX_DFX_SQ_NUM * NPU_MAX_DFX_SQ_DEPTH *
		NPU_DFX_MAX_SQ_SLOT_LEN) + (u64) cq_info->index * buf_size);
	cq_info->addr = ioremap_nocache(cq_info->phy_addr, buf_size);
	if (cq_info->addr == NULL) {
		npu_drv_err("ioremap_nocache failed\n");
		return -ENOMEM;
	}
	for (cnt = 0; cnt < buf_size; cnt += 4)
		writel(0, &cq_info->addr[cnt]);

	npu_drv_debug("cur sq %d phy_addr = %pK base = %pK\n", cq_info->index,
		(void *)(uintptr_t)cq_info->phy_addr,
		(void *)(uintptr_t)(long)g_sq_desc->base);

	return 0;
}

int npu_dfx_cq_para_check(const struct npu_dfx_create_cq_para *cq_para)
{
	if (cq_para->cq_type > DFX_DETAILED_CQ ||
		cq_para->cq_index >= NPU_MAX_DFX_CQ_NUM ||
		cq_para->addr == NULL ||
		cq_para->slot_len <= 0 ||
		cq_para->slot_len > NPU_DFX_MAX_CQ_SLOT_LEN) {
		npu_drv_err("invalid input argument\n");
		return -EINVAL;
	}

	return 0;
}

int npu_create_dfx_cq(struct npu_dev_ctx *cur_dev_ctx,
	struct npu_dfx_create_cq_para *cq_para)
{
	unsigned long flags;
	int ret;
	int len;
	u32 cq_index;
	struct npu_dfx_cqsq *cqsq = NULL;
	struct npu_dfx_cq_info *cq_info = NULL;
	struct npu_dfx_int_context *int_context = NULL;

	cond_return_error(cur_dev_ctx == NULL || cq_para == NULL, -EINVAL,
		"invalid input argument\n");
	cond_return_error(npu_dfx_cq_para_check(cq_para), -EINVAL,
		"invalid input argument\n");

	cqsq = npu_get_dfx_cqsq_info(cur_dev_ctx);
	cond_return_error(cqsq == NULL, -ENOMEM, "cqsq is null\n");

	cond_return_error(cqsq->cq_num == 0, -ENOMEM, "no available cq num=%d\n",
		cqsq->cq_num);

	cq_info = cqsq->cq_info;
	cq_index = cq_para->cq_index;

	mutex_lock(&cqsq->dfx_mutex);
	len = NPU_MAX_DFX_CQ_DEPTH * cq_para->slot_len;
#ifdef PROFILING_USE_RESERVED_MEMORY
	cond_goto_error(npu_get_dfx_cq_memory(&cq_info[cq_index], len), fail,
		ret, -ENOMEM, "npu_get_dfx_cq_memory failed\n");
#else
	cq_info[cq_index].addr = kzalloc(len, GFP_KERNEL);
#endif
	cond_goto_error(cq_info[cq_index].addr == NULL, fail, ret, -ENOMEM,
		"kmalloc failed\n");
	cq_info[cq_index].depth = NPU_MAX_DFX_CQ_DEPTH;
	cq_info[cq_index].slot_len = cq_para->slot_len;
	cq_info[cq_index].type = cq_para->cq_type;
	cq_info[cq_index].phase = NPU_DFX_PHASE_ONE;
	cq_info[cq_index].callback = cq_para->callback;
	cq_info[cq_index].function = cq_para->function;
	cqsq->cq_num--;
#ifdef PROFILING_USE_RESERVED_MEMORY
	*cq_para->addr = cq_info[cq_index].phy_addr;
#else
	*cq_para->addr = (unsigned long)virt_to_phys(
		(void *)cq_info[cq_index].addr);
	cond_goto_error(*cq_para->addr == NULL, fail, ret, -ENOMEM,
		"virt_to_phys failed\n");
#endif
	mutex_unlock(&cqsq->dfx_mutex);
	int_context = &cqsq->int_context;
	spin_lock_irqsave(&int_context->spinlock, flags);
	list_add(&cq_info[cq_index].int_list_node, &int_context->int_list_header);
	spin_unlock_irqrestore(&int_context->spinlock, flags);

	npu_drv_debug("dev[%d] dfx cq is created, cq id: %d, cq addr: %llx %llx\n",
		cur_dev_ctx->devid, cq_index, *cq_para->addr,
		(u64)(uintptr_t)cq_info[cq_index].addr);
	return 0;
fail:
#ifdef PROFILING_USE_RESERVED_MEMORY
	if (cq_info[cq_index].addr != NULL)
		iounmap((void *)cq_info[cq_index].addr);
#else
	if (cq_info[cq_index].addr != NULL)
		kfree(cq_info[cq_index].addr);
#endif
	cq_info[cq_index].addr = NULL;
	mutex_unlock(&cqsq->dfx_mutex);
	return ret;
}
EXPORT_SYMBOL(npu_create_dfx_cq);

irqreturn_t npu_dfx_cq_irq_handler(int irq, void *data)
{
	struct npu_dfx_int_context *int_context = NULL;
	struct npu_dfx_cq_info *cq_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	unsigned long flags;
	u8 *addr = NULL;
	u32 get = 0;
	u32 len;

	int_context = (struct npu_dfx_int_context *)data;
	cur_dev_ctx = int_context->cur_dev_ctx;
	if (cur_dev_ctx->power_stage != NPU_PM_UP)
		return IRQ_NONE;

	spin_lock_irqsave(&int_context->spinlock, flags);
	npu_plat_handle_irq_tophalf(NPU_IRQ_DFX_CQ_UPDATE);
	list_for_each_safe(pos, n, &int_context->int_list_header) {
		cq_info = list_entry(pos, struct npu_dfx_cq_info, int_list_node);
		if (cq_info->addr == NULL)
			continue;

		addr = cq_info->addr + (unsigned long)cq_info->tail * cq_info->slot_len;
		len = cq_info->slot_len * NPU_MAX_DFX_CQ_DEPTH;

		if (addr[0] == cq_info->phase) {
			npu_get_dfx_cq_report(int_context, cq_info);
			npu_drv_debug("receive irq find one report, cq id: %d, addr: %pK\n",
				cq_info->index, (void *)addr);
			get++;
		}
	}
	spin_unlock_irqrestore(&int_context->spinlock, flags);

	if (get == 0)
		npu_drv_err("receive irq but no report found\n");

	return IRQ_HANDLED;
}
EXPORT_SYMBOL(npu_dfx_cq_irq_handler);

void npu_destroy_dfx_cq(struct npu_dev_ctx *cur_dev_ctx, u32 cq_index)
{
	struct npu_dfx_int_context *int_context = NULL;
	struct npu_dfx_cq_info *cq_info = NULL;
	struct npu_dfx_cqsq *cqsq = NULL;
	unsigned long flags;

	if (cur_dev_ctx == NULL || cq_index >= NPU_MAX_DFX_CQ_NUM) {
		npu_drv_err("invalid input argument\n");
		return;
	}

	cqsq = npu_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		npu_drv_err("cqsq is null\n");
		return;
	}

	cq_info = cqsq->cq_info;
	mutex_lock(&cq_info[cq_index].lock);
	if (cq_info[cq_index].addr != NULL) {
		int_context = &cqsq->int_context;
		spin_lock_irqsave(&int_context->spinlock, flags);
		list_del(&cq_info[cq_index].int_list_node);
		spin_unlock_irqrestore(&int_context->spinlock, flags);

#ifdef PROFILING_USE_RESERVED_MEMORY
		iounmap(cq_info[cq_index].addr);
#else
		kfree(cq_info[cq_index].addr);
#endif
		mutex_lock(&cqsq->dfx_mutex);
		cq_info[cq_index].head = 0;
		cq_info[cq_index].tail = 0;
		cq_info[cq_index].addr = NULL;
		cq_info[cq_index].depth = NPU_MAX_DFX_CQ_DEPTH;
		cq_info[cq_index].slot_len = 0;
		cq_info[cq_index].type = 0;
		cq_info[cq_index].phase = NPU_DFX_PHASE_ONE;
		cq_info[cq_index].callback = NULL;
		cq_info[cq_index].function = NPU_MAX_CQSQ_FUNC;
		cqsq->cq_num++;
		mutex_unlock(&cqsq->dfx_mutex);
	}
	mutex_unlock(&cq_info[cq_index].lock);
}
EXPORT_SYMBOL(npu_destroy_dfx_cq);

static void npu_dfx_work(struct work_struct *work)
{
	struct npu_dfx_cq_report *report = NULL;
	struct npu_dfx_sq_info *sq_info = NULL;
	u32 sq_slot_index;
	u32 sq_index;
	u8 *cq_slot = NULL;
	u8 *sq_slot = NULL;
	u8 *cq_data = NULL;

	struct npu_dfx_cq_info *cq_info = container_of(work,
		struct npu_dfx_cq_info, work);
	struct npu_dfx_cqsq *cqsq =
		(struct npu_dfx_cqsq *)cq_info->dfx_cqsq_ptr;
	cond_return_void(cqsq == NULL, "dfx_cqsq_ptr is null\n");

	mutex_lock(&cq_info->lock);
	cond_goto_error(cq_info->addr == NULL, fail, sq_index, 0);

	cq_slot = cq_info->addr + (unsigned long)cq_info->tail * cq_info->slot_len;

	while (cq_slot[0] == cq_info->phase) {
		if (cq_info->type == DFX_DETAILED_CQ) {
			report = (struct npu_dfx_cq_report *)cq_slot;
			sq_index = report->sq_index;
			sq_info = &cqsq->sq_info[sq_index];
			sq_slot_index = (u32)((report->sq_head == 0) ?
				(NPU_MAX_DFX_SQ_DEPTH - 1) : (report->sq_head - 1));
			sq_slot = sq_info->addr + (unsigned long)sq_slot_index *
				sq_info->slot_len;
			cq_data = cq_slot + NPU_DFX_DETAILED_CQ_OFFSET;

			npu_drv_debug("call cq callback1, cq id: %d\n", cq_info->index);

			if (cq_info->callback != NULL)
				cq_info->callback(cq_data, sq_slot);

			sq_info->head = report->sq_head;
		} else {
			cq_data = cq_slot + NPU_DFX_BRIEF_CQ_OFFSET;
			npu_drv_debug("call cq callback0, cq id: %d\n", cq_info->index);

			if (cq_info->callback != NULL)
				cq_info->callback(cq_data, NULL);
		}

		if (cq_info->tail >= NPU_MAX_DFX_CQ_DEPTH - 1) {
			cq_info->phase = (u32)((cq_info->phase == NPU_DFX_PHASE_ONE) ?
				NPU_DFX_PHASE_ZERO : NPU_DFX_PHASE_ONE);
			cq_info->tail = 0;
		} else {
			cq_info->tail++;
		}
		cq_slot = cq_info->addr + (unsigned long)cq_info->tail *
			cq_info->slot_len;
	}

	cq_info->head = cq_info->tail;
	*cq_info->doorbell = (u32)cq_info->tail;
fail:
	mutex_unlock(&cq_info->lock);
}

struct npu_dfx_cqsq *npu_get_dfx_cqsq_info(
	struct npu_dev_ctx *cur_dev_ctx)
{
	if (cur_dev_ctx->dfx_cqsq_addr == NULL) {
		npu_drv_err("dfx_cqsq_addr is null\n");
		return NULL;
	}

	return (struct npu_dfx_cqsq *)cur_dev_ctx->dfx_cqsq_addr;
}

static int npu_dfx_sq_init(struct npu_dfx_sq_info *sq_info,
	const struct npu_dev_ctx *cur_dev_ctx)
{
	u32 i;
	int ret;
	u32 *doorbell_vaddr = NULL;

	for (i = 0; i < NPU_MAX_DFX_SQ_NUM; i++) {
		sq_info[i].devid = cur_dev_ctx->devid;
		sq_info[i].index = i;
		sq_info[i].depth = NPU_MAX_DFX_SQ_DEPTH;
		sq_info[i].slot_len = 0;
		sq_info[i].addr = NULL;
		sq_info[i].head = 0;
		sq_info[i].tail = 0;
		sq_info[i].credit = NPU_MAX_DFX_SQ_DEPTH;
		sq_info[i].phy_addr = 0;
		ret = npu_get_doorbell_vaddr(DOORBELL_RES_DFX_SQ, i,
			&doorbell_vaddr);
		if (ret) {
			npu_drv_err("npu_get_doorbell_vaddr sq failed %u\n", i);
			break;
		}

		sq_info[i].doorbell = doorbell_vaddr;
		sq_info[i].function = NPU_MAX_CQSQ_FUNC;
	}
	return ret;
}

static int npu_dfx_cq_init(struct npu_dfx_cq_info *cq_info,
	const struct npu_dev_ctx *cur_dev_ctx, struct npu_dfx_cqsq *cqsq)
{
	u32 i;
	int ret;
	u32 *doorbell_vaddr = NULL;

	for (i = 0; i < NPU_MAX_DFX_CQ_NUM; i++) {
		cq_info[i].devid = (u32)cur_dev_ctx->devid;
		cq_info[i].index = i;
		cq_info[i].depth = NPU_MAX_DFX_CQ_DEPTH;
		cq_info[i].slot_len = 0;
		cq_info[i].type = 0;
		cq_info[i].dfx_cqsq_ptr = (void *)cqsq;

		if (i != DFX_HEART_BEAT_REPORT_CQ)
			INIT_WORK(&cq_info[i].work, npu_dfx_work);
		else
			INIT_WORK(&cq_info[i].work, npu_heart_beat_work);

		mutex_init(&cq_info[i].lock);

		cq_info[i].addr = NULL;
		cq_info[i].head = 0;
		cq_info[i].tail = 0;
		cq_info[i].phase = NPU_DFX_PHASE_ONE;
		ret = npu_get_doorbell_vaddr(DOORBELL_RES_DFX_CQ, i,
			&doorbell_vaddr);
		if (ret) {
			npu_drv_err("npu_get_doorbell_vaddr cq failed %u\n", i);
			break;
		}
		cq_info[i].doorbell = doorbell_vaddr;
		cq_info[i].callback = NULL;
		cq_info[i].function = NPU_MAX_CQSQ_FUNC;
	}
	return ret;
}

int npu_dfx_cqsq_init(struct npu_dev_ctx *cur_dev_ctx)
{
	struct npu_dfx_sq_info *sq_info = NULL;
	struct npu_dfx_cq_info *cq_info = NULL;
	struct npu_dfx_cqsq *cqsq = NULL;
	struct npu_platform_info *plat_info = NULL;
	int ret;

	cond_return_error(cur_dev_ctx == NULL, -EINVAL,
		"invalid input argument\n");

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -EINVAL, "npu_plat_get_info\n");

	cur_dev_ctx->dfx_cqsq_addr = kzalloc(sizeof(struct npu_dfx_cqsq),
		GFP_KERNEL);
	cond_return_error(cur_dev_ctx->dfx_cqsq_addr == NULL, -ENOMEM,
		"no mem to alloc dfx cqsq\n");

	cqsq = npu_get_dfx_cqsq_info(cur_dev_ctx);
	cond_goto_error(cqsq == NULL, dfx_cqsq_addr_free, ret, -ENOMEM,
		"cqsq is null\n");

	INIT_LIST_HEAD(&cqsq->int_context.int_list_header);
	spin_lock_init(&cqsq->int_context.spinlock);
	cqsq->int_context.wq = create_workqueue("devdrv-cqsq-work");
	cond_goto_error(cqsq->int_context.wq == NULL, dfx_cqsq_addr_free, ret,
		-ENOMEM, "create_workqueue error\n");

	sq_info = kzalloc(sizeof(struct npu_dfx_sq_info) * NPU_MAX_DFX_SQ_NUM,
		GFP_KERNEL);
	cond_goto_error(sq_info == NULL, workqueue_free, ret, -ENOMEM,
		"kmalloc failed\n");

	cq_info = kzalloc(sizeof(struct npu_dfx_cq_info) * NPU_MAX_DFX_CQ_NUM,
		GFP_KERNEL);
	cond_goto_error(cq_info == NULL, sq_info_free, ret, -ENOMEM,
		"kmalloc failed\n");

	ret = npu_dfx_sq_init(sq_info, cur_dev_ctx);
	cond_goto_debug(ret, cq_info_free, ret, -ENOMEM);

	ret = npu_dfx_cq_init(cq_info, cur_dev_ctx, cqsq);
	cond_goto_debug(ret, cq_info_free, ret, -ENOMEM);

	cqsq->sq_info = sq_info;
	cqsq->sq_num = NPU_MAX_DFX_SQ_NUM;
	cqsq->cq_info = cq_info;
	cqsq->cq_num = NPU_MAX_DFX_CQ_NUM;
	cqsq->int_context.irq_num = plat_info->dts_info.irq_dfx_cq;
	cqsq->int_context.cur_dev_ctx = cur_dev_ctx;
	ret = request_irq(cqsq->int_context.irq_num, npu_dfx_cq_irq_handler,
		IRQF_TRIGGER_NONE, "devdrv-functional_cq", &cqsq->int_context);
	cond_goto_error(ret, request_irq_free, ret, -ENOMEM,
		"request_irq failed\n");

	mutex_init(&cqsq->dfx_mutex);

	return 0;

request_irq_free:
	cqsq->sq_info = NULL;
	cqsq->sq_num = 0;
	cqsq->cq_info = NULL;
	cqsq->cq_num = 0;
cq_info_free:
	kfree(cq_info);
	cq_info = NULL;
sq_info_free:
	kfree(sq_info);
	sq_info = NULL;
workqueue_free:
	destroy_workqueue(cqsq->int_context.wq);
dfx_cqsq_addr_free:
	kfree(cur_dev_ctx->dfx_cqsq_addr);
	cur_dev_ctx->dfx_cqsq_addr = NULL;

	return ret;
}

void npu_destroy_dfx_cqsq(struct npu_dev_ctx *cur_dev_ctx)
{
	struct npu_dfx_cqsq *cqsq = NULL;

	if (cur_dev_ctx == NULL) {
		npu_drv_err("device not exist");
		return;
	}

	cqsq = npu_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		npu_drv_err("cqsq is null\n");
		return;
	}
	free_irq(cqsq->int_context.irq_num, &cqsq->int_context);

	destroy_workqueue(cqsq->int_context.wq);

	if (cqsq->sq_info != NULL)
		kfree(cqsq->sq_info);

	if (cqsq->cq_info != NULL)
		kfree(cqsq->cq_info);

	if (cur_dev_ctx->dfx_cqsq_addr != NULL)
		kfree(cur_dev_ctx->dfx_cqsq_addr);

	mutex_lock(&cqsq->dfx_mutex);
	cqsq->sq_info = NULL;
	cqsq->sq_num = 0;
	cqsq->cq_info = NULL;
	cqsq->cq_num = 0;
	mutex_unlock(&cqsq->dfx_mutex);
	cur_dev_ctx->dfx_cqsq_addr = NULL;
}
