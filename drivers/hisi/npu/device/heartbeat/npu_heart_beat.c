/*
 * npu_heart_beat.c
 *
 * about npu heart beat
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
#include "npu_heart_beat.h"

#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/types.h>
#include <dsm/dsm_pub.h>
#include <securec.h>

#include "npu_manager.h"
#include "npu_proc_ctx.h"
#include "npu_manager_ioctl_services.h"
#include "npu_ioctl_services.h"
#include "npu_user_common.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_dfx_cq.h"
#include "npu_dfx_log.h"
#include "npu_manager_common.h"
#include "npu_mailbox.h"
#include "bbox/npu_dfx_black_box.h"

static u8 dev_ctx_id;

static void npu_heart_beat_start(struct npu_dev_ctx *dev_ctx)
{
	dev_ctx->heart_beat.stop = 0;
}

static void npu_heart_beat_stop(struct npu_dev_ctx *dev_ctx)
{
	dev_ctx->heart_beat.stop = 1;
}

/*
 * heart beat between driver and TS
 * alloc a functional sq and a functional cq
 * sq: send a cmd per second
 * cq: TS's report, TS have to send report back within one second
 */
static int npu_heart_beat_judge(struct npu_dev_ctx *dev_ctx)
{
	struct npu_heart_beat_node *beat_node = NULL;
	struct npu_heart_beat_node *pprev_node = NULL;
	struct npu_heart_beat_node *prev_node = NULL;
	struct list_head *pprev = NULL;
	struct list_head *prev = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	unsigned long flags;

	spin_lock_irqsave(&dev_ctx->heart_beat.spinlock, flags);
	if (list_empty_careful(&dev_ctx->heart_beat.queue) == 0) {
		list_for_each_safe(pos, n, &dev_ctx->heart_beat.queue) {
			beat_node = list_entry(pos, struct npu_heart_beat_node, list);
			if ((pprev != NULL) && (prev != NULL)) {
				pprev_node = list_entry(pprev,
					struct npu_heart_beat_node, list);
				prev_node = list_entry(prev,
					struct npu_heart_beat_node, list);
				if ((pprev_node->sq->number + 1 == prev_node->sq->number) &&
					(prev_node->sq->number + 1 == beat_node->sq->number)) {
					/* heart beat timeout not return exception */
				} else if (prev_node->sq->number + 1 == beat_node->sq->number) {
					list_del(pprev);
					kfree(pprev_node->sq);
					kfree(pprev_node);
					pprev = prev;
					prev = pos;
				} else {
					list_del(pprev);
					kfree(pprev_node->sq);
					kfree(pprev_node);
					list_del(prev);
					kfree(prev_node->sq);
					kfree(prev_node);
					pprev = NULL;
					prev = pos;
				}
			} else {
				pprev = prev;
				prev = pos;
			}
		}
	}
	spin_unlock_irqrestore(&dev_ctx->heart_beat.spinlock, flags);
	return 0;
}

static void npu_heart_beat_event_proc(struct npu_dev_ctx *dev_ctx)
{
	struct timespec64 now;
	int ret;
	unsigned long flags;
	struct timespec wall;
	struct npu_heart_beat *hb = NULL;
	struct npu_heart_beat_sq *sq = NULL;
	struct npu_heart_beat_node *beat_node = NULL;

	npu_drv_debug("enter\n");
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");
	cond_return_void(dev_ctx->heart_beat.working == 0);
	cond_goto_error(dev_ctx->heart_beat.stop, out, ret, 0);

	hb = &dev_ctx->heart_beat;
	/* judge whether TS is in exception */
	ret = npu_heart_beat_judge(dev_ctx);
	cond_return_void(ret, "npu_heart_beat_judge return false\n");

	/* send heart beat to TS */
	sq = kzalloc(sizeof(struct npu_heart_beat_sq), GFP_ATOMIC);
	cond_goto_error(sq == NULL, out, sq, sq,
		"kmalloc in time event fail once, "
		"give up sending heart beat this time\n");

	beat_node = kzalloc(sizeof(struct npu_heart_beat_node), GFP_ATOMIC);
	if (beat_node == NULL) {
		kfree(sq);
		npu_drv_err("kmalloc in time event fail once, give up sending heart beat this time\n");
		goto out;
	}

	wall = current_kernel_time();
	getrawmonotonic64(&now);
	sq->number = hb->cmd_inc_counter;
	sq->devid = dev_ctx->devid;
	sq->cmd = NPU_HEART_BEAT_SQ_CMD;
	sq->stamp = now;
	sq->wall = timespec_to_timespec64(wall);
	sq->cntpct = npu_read_cntpct();

	ret = npu_dfx_send_sq(dev_ctx->devid, dev_ctx->heart_beat.sq,
		(u8 *)sq, sizeof(struct npu_heart_beat_sq));
	if (ret) {
		npu_drv_err("functional_send_sq in timeevent failed\n");
		kfree(sq);
		kfree(beat_node);
		goto out;
	}

	npu_drv_debug("send one heart beat to ts, number: %d\n", sq->number);
	spin_lock_irqsave(&dev_ctx->heart_beat.spinlock, flags);
	beat_node->sq = sq;

	list_add_tail(&beat_node->list, &hb->queue);
	hb->cmd_inc_counter++;
	spin_unlock_irqrestore(&dev_ctx->heart_beat.spinlock, flags);

out:
	dev_ctx->heart_beat.timer.expires = jiffies + NPU_HEART_BEAT_CYCLE * HZ;
	add_timer(&dev_ctx->heart_beat.timer);
}

static void npu_heart_beat_event(unsigned long data)
{
	npu_heart_beat_event_proc((struct npu_dev_ctx *)(uintptr_t)data);
}

static void npu_driver_hardware_exception(struct npu_dev_ctx *dev_ctx)
{
	struct npu_proc_ctx *proc_ctx = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (dev_ctx == NULL)
		return;

	if (list_empty_careful(&dev_ctx->proc_ctx_list) == 0) {
		list_for_each_safe(pos, n, &dev_ctx->proc_ctx_list) {
			proc_ctx = list_entry(pos, struct npu_proc_ctx, dev_ctx_list);
			proc_ctx->last_ts_status = NPU_TS_DOWN;
			proc_ctx->cq_tail_updated = CQ_HEAD_UPDATE_FLAG;
			wake_up(&proc_ctx->report_wait);
		}
	}
}

static void npu_inform_device_manager(struct npu_dev_ctx *dev_ctx)
{
	struct npu_manager_info *d_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct npu_pm *pm = NULL;
	unsigned long flags;

	if (dev_ctx == NULL) {
		npu_drv_err("dev_ctx is null\n");
		return;
	}

	d_info = npu_get_manager_info();

	/* inform all modules related to ts driver that ts can not work */
	spin_lock_irqsave(&d_info->pm_list_lock, flags);
	if (!list_empty_careful(&d_info->pm_list_header)) {
		list_for_each_safe(pos, n, &d_info->pm_list_header) {
			pm = list_entry(pos, struct npu_pm, list);
			if (pm->suspend != NULL)
				(void)pm->suspend(dev_ctx->devid, NPU_TS_DOWN);
		}
	}
	spin_unlock_irqrestore(&d_info->pm_list_lock, flags);

	npu_driver_hardware_exception(dev_ctx);
	npu_mailbox_recycle(&dev_ctx->mailbox);
}

static void npu_ts_exception_task(unsigned long data)
{
	u32 *ts_status = NULL;
	struct npu_dev_ctx *dev_ctx = (struct npu_dev_ctx *)(uintptr_t)data;

	if (dev_ctx == NULL) {
		npu_drv_err("dev_ctx is null\n");
		return;
	}

	ts_status = npu_get_ts_work_status(dev_ctx->devid);
	if (ts_status != NULL)
		npu_drv_err("begin to inform ts[%d] status: %d\n",
			dev_ctx->devid, *ts_status);
	npu_inform_device_manager(dev_ctx);
}

static void npu_heart_beat_ts_down(struct npu_dev_ctx *dev_ctx)
{
	struct npu_heart_beat_node *beat_node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct timespec os_time;
	excep_time timestamp;
	unsigned long flags;

	npu_drv_err(
		"TS heart beat exception is detected, process ts down exception\n");
	dev_ctx->heart_beat.broken = 1;

	os_time = current_kernel_time();
	timestamp.tv_sec = (u64)os_time.tv_sec;
	timestamp.tv_usec = os_time.tv_nsec / 1000;

	if (npu_rdr_exception_is_count_exceeding(
		RDR_EXC_TYPE_TS_RUNNING_EXCEPTION) == 0) {
		npu_drv_err("call rdr_system_error: time: %llu.%llu, arg: 0\n",
			timestamp.tv_sec, timestamp.tv_usec);
		/* bbox : receive TS exception */
		rdr_system_error((u32)RDR_EXC_TYPE_TS_RUNNING_EXCEPTION, 0, 0);
	}

	npu_ts_exception_task((unsigned long)(uintptr_t)dev_ctx);

	del_timer_sync(&dev_ctx->heart_beat.timer);
	npu_destroy_dfx_sq(dev_ctx, dev_ctx->heart_beat.sq);
	npu_destroy_dfx_cq(dev_ctx, dev_ctx->heart_beat.cq);
	dev_ctx->heart_beat.sq = NPU_MAX_FUNCTIONAL_SQ_NUM;
	dev_ctx->heart_beat.cq = NPU_MAX_FUNCTIONAL_CQ_NUM;
	dev_ctx->heart_beat.cmd_inc_counter = 0;

	spin_lock_irqsave(&dev_ctx->heart_beat.spinlock, flags);
	if (list_empty_careful(&dev_ctx->heart_beat.queue) == 0) {
		list_for_each_safe(pos, n, &dev_ctx->heart_beat.queue) {
			beat_node = list_entry(pos, struct npu_heart_beat_node, list);
			list_del(&beat_node->list);
			kfree(beat_node->sq);
			kfree(beat_node);
		}
	}
	spin_unlock_irqrestore(&dev_ctx->heart_beat.spinlock, flags);
}

static void npu_heart_beat_ai_down(struct npu_dev_ctx *dev_ctx,
	void *data)
{
	unsigned long flags;
	u32 i;
	struct npu_heart_beat_cq *cq = (struct npu_heart_beat_cq *)data;
	u32 cpu_bitmap = 0;
	u32 cpu_count = 0;
	u32 core_bitmap = 0;
	u32 core_count = 0;

	if (cq->u.exception_info.aicpu_heart_beat_exception) {
		for (i = 0; i < dev_ctx->ai_cpu_core_num; i++) {
			if (cq->u.exception_info.aicpu_heart_beat_exception &
				(0x01U << i)) {
				cpu_bitmap |= (0x01U << i);
				if (!(dev_ctx->inuse.ai_cpu_error_bitmap & (0x01U << i))) {
					npu_drv_err(
						"receive TS message ai cpu: %u heart beat exception\n",
						i);
					if (npu_rdr_exception_is_count_exceeding(
						RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION) == 0)
						rdr_system_error(
							(u32)RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION,
							0, 0);
				}
			} else {
				cpu_count++;
			}
		}
	}
	if (cq->u.exception_info.aicore_bitmap) {
		for (i = 0; i < dev_ctx->ai_core_num; i++) {
			if (cq->u.exception_info.aicore_bitmap & (0x01U << i)) {
				core_bitmap |= (0x01U << i);
				if (!(dev_ctx->inuse.ai_core_error_bitmap & (0x01U << i))) {
					npu_drv_err("receive TS message ai core: %u exception\n",
						i);
					if (npu_rdr_exception_is_count_exceeding(
						EXC_TYPE_TS_AICORE_EXCEPTION) == 0)
						rdr_system_error((u32)EXC_TYPE_TS_AICORE_EXCEPTION,
							0, 0);
				}
			} else {
				core_count++;
			}
		}
	}

	if (cq->u.exception_info.syspcie_sysdma_status & 0xFFFF) {
		npu_drv_err("ts sysdma is broken\n");
		dev_ctx->ai_subsys_ip_broken_map |=
			(0x01U << NPU_AI_SUBSYS_SDMA_WORKING_STATUS_OFFSET);
	}
	if ((cq->u.exception_info.syspcie_sysdma_status >> 16) & 0xFFFF) {
		npu_drv_err("ts syspcie is broken\n");
		dev_ctx->ai_subsys_ip_broken_map |=
			(0x01U << NPU_AI_SUBSYS_SPCIE_WORKING_STATUS_OFFSET);
	}

	spin_lock_irqsave(&dev_ctx->ts_spinlock, flags);
	dev_ctx->inuse.ai_cpu_num = cpu_count;
	dev_ctx->inuse.ai_cpu_error_bitmap = cpu_bitmap;
	dev_ctx->inuse.ai_core_num = core_count;
	dev_ctx->inuse.ai_core_error_bitmap = core_bitmap;
	spin_unlock_irqrestore(&dev_ctx->ts_spinlock, flags);
}

static void npu_heart_beat_broken(struct npu_dev_ctx *dev_ctx)
{
	struct npu_heart_beat *hb = &(dev_ctx->heart_beat);
	/* jugde which exception is */
	if (hb->exception_info == NULL)
		npu_heart_beat_ts_down(dev_ctx);
	else
		npu_heart_beat_ai_down(dev_ctx, hb->exception_info);
}

static void npu_heart_beat_del_node(struct npu_dev_ctx *dev_ctx,
	struct npu_heart_beat_cq *cq)
{
	struct npu_heart_beat_node *beat_node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	unsigned long flags;

	spin_lock_irqsave(&dev_ctx->heart_beat.spinlock, flags);
	if (list_empty_careful(&dev_ctx->heart_beat.queue) == 0) {
		list_for_each_safe(pos, n, &dev_ctx->heart_beat.queue) {
			beat_node = list_entry(pos, struct npu_heart_beat_node, list);
			if (beat_node->sq->number == cq->number) {
				list_del(pos);
				kfree(beat_node->sq);
				kfree(beat_node);
				break;
			}
		}
	}
	spin_unlock_irqrestore(&dev_ctx->heart_beat.spinlock, flags);
}

int npu_heart_beat_is_exception(const u8 *cq_slot)
{
	struct npu_heart_beat_cq *cq = NULL;

	cond_return_error(cq_slot == NULL, -1, "slot is null\n");
	cq = (struct npu_heart_beat_cq *)cq_slot;
	if ((u32)cq->exception_code >= EXC_TYPE_HWTS_ERROR_START &&
		(u32)cq->exception_code <= EXC_TYPE_HWTS_ERROR_END)
		return 1;
	return 0;
}

int npu_heart_beat_exception_process(const u8 *cq_slot, u32 size)
{
	struct npu_dev_ctx *dev_ctx = NULL;
	struct npu_heart_beat_cq *cq = NULL;

	cond_return_error(cq_slot == NULL, -1, "slot is null\n");
	cond_return_error(size < sizeof(struct npu_heart_beat_cq), -1,
		"size is invalid\n");

	cq = (struct npu_heart_beat_cq *)cq_slot;
	cond_return_error(!(cq->exception_code >= EXC_TYPE_HWTS_ERROR_START &&
		cq->exception_code <= EXC_TYPE_HWTS_ERROR_END), -1,
		"exception code is error, code=%d\n", cq->exception_code);

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	cond_return_error(dev_ctx == NULL, -1, "dev_ctx is null\n");

	if (npu_rdr_exception_is_count_exceeding(cq->exception_code) == 0)
		rdr_system_error((unsigned int)cq->exception_code, 0, 0);
	if (dev_ctx->heart_beat.hwts_exception_callback != NULL)
		(dev_ctx->heart_beat.hwts_exception_callback)(dev_ctx,
			&(cq->u.hwts_exception_info));

	return 0;
}

static void npu_heart_beat_callback(const u8 *cq_slot, const u8 *sq_slot)
{
	struct npu_dev_ctx *dev_ctx = NULL;
	struct npu_heart_beat_cq *cq = NULL;
	excep_time timestamp;

	npu_drv_debug("enter\n");
	cond_return_void(cq_slot == NULL || sq_slot == NULL, "slot is null\n");

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");

	cq = (struct npu_heart_beat_cq *)cq_slot;

	if (cq->report_type != 0) {
		timestamp.tv_sec = (u64)cq->exception_time.tv_sec;
		timestamp.tv_usec = cq->exception_time.tv_nsec / 1000;

		npu_drv_err("receive ts exception msg, call mntn_system_error: 0x%x, time: %llu.%llu, arg: 0\n",
			cq->exception_code, timestamp.tv_sec, timestamp.tv_usec);

		/* bbox : receive TS exception */
		if ((unsigned int)cq->exception_code >= DMD_EXC_TYPE_EXCEPTION_START &&
			(unsigned int)cq->exception_code <= DMD_EXC_TYPE_EXCEPTION_END) {
			if (!dsm_client_ocuppy(npu_dsm_client)) {
				dsm_client_record(npu_dsm_client, "npu power up failed\n");
				dsm_client_notify(npu_dsm_client,
					DSM_AI_KERN_WTD_TIMEOUT_ERR_NO);
				npu_drv_err("[I/DSM] %s dmd report\n",
					npu_dsm_client->client_name);
			}
		}
		if (npu_rdr_exception_is_count_exceeding(cq->exception_code) == 0)
			rdr_system_error((unsigned int)cq->exception_code, 0, 0);
	}

	npu_heart_beat_del_node(dev_ctx, cq);

	if (cq->u.exception_info.ts_status ||
		cq->u.exception_info.syspcie_sysdma_status ||
		cq->u.exception_info.aicpu_heart_beat_exception ||
		cq->u.exception_info.aicore_bitmap) {
		dev_ctx->heart_beat.exception_info = (u8 *)cq_slot;
		npu_heart_beat_broken(dev_ctx);
	}
}

static void npu_enable_ts_heart_beat(struct npu_dev_ctx *dev_ctx)
{
	dev_ctx->config.ts_func.ts_heart_beat_en = 1;
}

static void npu_disenable_ts_heart_beat(struct npu_dev_ctx *dev_ctx)
{
	dev_ctx->config.ts_func.ts_heart_beat_en = 0;
}


static int npu_heart_beat_para_init(struct npu_dev_ctx *dev_ctx)
{
	dev_ctx->heart_beat.sq = DFX_HEART_BEAT_SQ;
	dev_ctx->heart_beat.cq = DFX_HEART_BEAT_REPORT_CQ;
	dev_ctx->heart_beat.exception_info = NULL;
	dev_ctx->heart_beat.stop = 0;
	dev_ctx->heart_beat.broken = 0;
	dev_ctx->heart_beat.working = 1;

	dev_ctx_id = dev_ctx->devid;
	return 0;
}

static int npu_heart_beat_create_dfx_sq(struct npu_dev_ctx *dev_ctx,
	u64 *sq_addr)
{
	int ret;
	u64 sq_0_addr = 0;
	struct npu_dfx_create_sq_para sq_para;

	cond_return_error(sq_addr == NULL, -ENOMEM, "output para sq_addr is NULL");
	ret = memset_s(&sq_para, sizeof(struct npu_dfx_create_sq_para), 0,
		sizeof(struct npu_dfx_create_sq_para));
	if (ret != 0)
		npu_drv_err("memset_s failed. ret=%d\n", ret);

	sq_para.slot_len = LOG_SQ_SLOT_LEN;
	sq_para.sq_index = DFX_HEART_BEAT_SQ;
	sq_para.addr = (unsigned long long *)&sq_0_addr;
	sq_para.function = NPU_CQSQ_HEART_BEAT;
	ret = npu_create_dfx_sq(dev_ctx, &sq_para);
	cond_return_error(ret != 0, -ENOMEM, "create_functional_sq failed\n");
	*sq_addr = sq_0_addr;
	return 0;
}

static int npu_heart_beat_create_dfx_cq(struct npu_dev_ctx *dev_ctx,
	u64 *cq_addr)
{
	int ret;
	u64 cq_0_addr = 0;
	struct npu_dfx_create_cq_para cq_para;

	cond_return_error(cq_addr == NULL, -ENOMEM, "output para cq_addr is NULL");

	ret = memset_s(&cq_para, sizeof(struct npu_dfx_create_cq_para), 0,
		sizeof(struct npu_dfx_create_cq_para));
	if (ret != 0)
		npu_drv_err("memset_s failed. ret=%d\n", ret);

	cq_para.cq_type = DFX_DETAILED_CQ;
	cq_para.cq_index = DFX_HEART_BEAT_REPORT_CQ;
	cq_para.function = NPU_CQSQ_HEART_BEAT;
	cq_para.slot_len = LOG_SQ_SLOT_LEN;
	cq_para.callback = npu_heart_beat_callback;
	cq_para.addr = (unsigned long long *)&cq_0_addr;

	ret = npu_create_dfx_cq(dev_ctx, &cq_para);
	if (ret) {
		npu_drv_err("create_functional_cq failed\n");
		return -ENOMEM;
	}
	*cq_addr = cq_0_addr;
	return 0;
}

static int npu_heart_beat_mailbox_init(const struct npu_dev_ctx *dev_ctx,
	u64 sq_addr, u64 cq_addr)
{
	int ret;
	int result = 0;
	struct npu_mailbox_cqsq cqsq;

	cqsq.cmd_type = NPU_MAILBOX_CREATE_CQSQ_BEAT;
	cqsq.valid = NPU_MAILBOX_MESSAGE_VALID;
	cqsq.result = 0;
	cqsq.sq_index = DFX_HEART_BEAT_SQ;
	cqsq.cq0_index = DFX_HEART_BEAT_REPORT_CQ;
	cqsq.sq_addr = sq_addr;
	cqsq.cq0_addr = cq_addr;
	cqsq.plat_type = dev_ctx->plat_type;

	ret = npu_mailbox_message_send_for_res(dev_ctx->devid, (u8 *)&cqsq,
		sizeof(struct npu_mailbox_cqsq), &result);
	if (ret != 0) {
		npu_drv_err("npu_mailbox_message_send_for_res failed ret = %d\n",
			ret);
		return -ENOMEM;
	}

	return 0;
}

int npu_heart_beat_init(struct npu_dev_ctx *dev_ctx)
{
	int ret;
	u64 sq_addr = 0;
	u64 cq_addr = 0;

	npu_drv_info("enter\n");
	cond_return_error(dev_ctx == NULL, -1, "dev_ctx is null\n");

	npu_enable_ts_heart_beat(dev_ctx);

	if (dev_ctx->config.ts_func.ts_heart_beat_en == 0) {
		dev_ctx->heart_beat.stop = 1;
		dev_ctx->heart_beat.sq = NPU_MAX_FUNCTIONAL_SQ_NUM;
		dev_ctx->heart_beat.cq = NPU_MAX_FUNCTIONAL_SQ_NUM;
		npu_drv_err("nve config: close heart beat between TS and device manager\n");
		return -1;
	}

	/* para init */
	npu_heart_beat_para_init(dev_ctx);

	ret = npu_heart_beat_create_dfx_sq(dev_ctx, &sq_addr);
	if (ret)
		return ret;

	ret = npu_heart_beat_create_dfx_cq(dev_ctx, &cq_addr);
	if (ret) {
		npu_destroy_dfx_sq(dev_ctx, DFX_HEART_BEAT_SQ);
		return ret;
	}

	ret = npu_heart_beat_mailbox_init(dev_ctx, sq_addr, cq_addr);
	if (ret) {
		npu_destroy_dfx_sq(dev_ctx, DFX_HEART_BEAT_SQ);
		npu_destroy_dfx_cq(dev_ctx, DFX_HEART_BEAT_REPORT_CQ);
		return ret;
	}

	npu_heart_beat_start(dev_ctx);

	dev_ctx->heart_beat.timer.expires = jiffies + NPU_HEART_BEAT_CYCLE * HZ;
	add_timer(&dev_ctx->heart_beat.timer);

	return 0;
}

static void npu_list_queue_free(struct npu_dev_ctx *dev_ctx)
{
	struct npu_heart_beat_node *beat_node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (dev_ctx->heart_beat.queue.next == NULL) {
		npu_drv_warn("heart_beat.queue.next is NULL\n");
		return;
	}

	list_for_each_safe(pos, n, &dev_ctx->heart_beat.queue) {
		beat_node = list_entry(pos, struct npu_heart_beat_node, list);
		if (beat_node == NULL) {
			npu_drv_err("beat_node is null\n");
			return;
		}
		list_del(&beat_node->list);
		if (beat_node->sq != NULL) {
			kfree(beat_node->sq);
			beat_node->sq = NULL;
		}
		kfree(beat_node);
		beat_node = NULL;
		if (n == NULL) {
			npu_drv_err("pos->next is null\n");
			break;
		}
	}
}

void npu_heart_beat_exit(struct npu_dev_ctx *dev_ctx)
{
	struct npu_mailbox_cqsq cqsq;
	unsigned long flags;
	int result;
	int ret;

	npu_drv_info("enter\n");
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");

	npu_disenable_ts_heart_beat(dev_ctx);
	npu_heart_beat_stop(dev_ctx);
	dev_ctx->heart_beat.working = 0;
	del_timer_sync(&dev_ctx->heart_beat.timer);

	if ((dev_ctx->heart_beat.sq < NPU_MAX_FUNCTIONAL_SQ_NUM) &&
		(dev_ctx->heart_beat.cq < NPU_MAX_FUNCTIONAL_CQ_NUM)) {
		cqsq.cmd_type = NPU_MAILBOX_RELEASE_CQSQ_BEAT;
		cqsq.valid = NPU_MAILBOX_MESSAGE_VALID;
		cqsq.result = 0;
		cqsq.sq_index = dev_ctx->heart_beat.sq;
		cqsq.cq0_index = dev_ctx->heart_beat.cq;
		cqsq.sq_addr = 0;
		cqsq.cq0_addr = 0;
		cqsq.plat_type = dev_ctx->plat_type;

		ret = npu_mailbox_message_send_for_res(dev_ctx->devid,
			(u8 *)&cqsq, sizeof(struct npu_mailbox_cqsq), &result);
		if (ret != 0)
			npu_drv_err("npu_mailbox_message_send_for_res failed ret = %d\n",
				ret);

		/* free cd and sq */
		npu_destroy_dfx_sq(dev_ctx, dev_ctx->heart_beat.sq);
		npu_destroy_dfx_cq(dev_ctx, dev_ctx->heart_beat.cq);

		dev_ctx->heart_beat.sq = NPU_MAX_FUNCTIONAL_SQ_NUM;
		dev_ctx->heart_beat.cq = NPU_MAX_FUNCTIONAL_CQ_NUM;
	}

	spin_lock_irqsave(&dev_ctx->heart_beat.spinlock, flags);
	if (list_empty_careful(&dev_ctx->heart_beat.queue) == 0)
		npu_list_queue_free(dev_ctx);

	spin_unlock_irqrestore(&dev_ctx->heart_beat.spinlock, flags);
}

/* global resource init when register */
int npu_heart_beat_resource_init(struct npu_dev_ctx *dev_ctx)
{
	if (dev_ctx == NULL) {
		npu_drv_err("dev_ctx is null\n");
		return -1;
	}
	npu_drv_debug("enter\n");

	/* assign invalid value */
	dev_ctx->heart_beat.sq = NPU_MAX_FUNCTIONAL_SQ_NUM;
	dev_ctx->heart_beat.cq = NPU_MAX_FUNCTIONAL_CQ_NUM;
	dev_ctx->heart_beat.cmd_inc_counter = 0;

	/* init */
	INIT_LIST_HEAD(&dev_ctx->heart_beat.queue);
	spin_lock_init(&dev_ctx->heart_beat.spinlock);

	dev_ctx->heart_beat.hb_wq = create_workqueue("devdrv-heartbeat-work");
	if (dev_ctx->heart_beat.hb_wq == NULL) {
		npu_drv_err("create_workqueue error\n");
		return -ENOMEM;
	}

	setup_timer(&dev_ctx->heart_beat.timer, npu_heart_beat_event,
		(unsigned long)(uintptr_t)dev_ctx);

	return 0;
}

void npu_heart_beat_resource_destroy(struct npu_dev_ctx *dev_ctx)
{
	if (dev_ctx == NULL) {
		npu_drv_err("dev_ctx is null\n");
		return;
	}

	if (dev_ctx->heart_beat.hb_wq != NULL)
		destroy_workqueue(dev_ctx->heart_beat.hb_wq);
}

