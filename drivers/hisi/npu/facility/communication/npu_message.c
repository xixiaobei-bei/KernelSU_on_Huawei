/*
 * npu_message.c
 *
 * about npu sq/cq msg
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

#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/hisi/rdr_pub.h>
#include <dsm/dsm_pub.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <securec.h>
#include <linux/timer.h>

#include "npu_task_message.h"
#include "npu_common.h"
#include "npu_proc_ctx.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_mailbox.h"
#include "npu_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "npu_pm_framework.h"
#include "npu_heart_beat.h"
#include "npu_adapter.h"
#include "npu_comm_sqe_fmt.h"
#include "npu_comm_task_verify.h"
#include "npu_calc_sq.h"
#include "npu_pool.h"
#include "npu_iova.h"
#include "npu_doorbell.h"
#include "npu_platform.h"
#include "npu_svm.h"
#include "bbox/npu_dfx_black_box.h"

enum npu_refcnt_updata {
	OPS_ADD = 0,
	OPS_SUB = 1,
	OPS_BUTT
};

const u32 dummy = 0xFFFFFFFF;

static struct npu_report *__npu_get_report(phys_addr_t base_addr,
	u32 slot_size, u32 slot_id)
{
	return (struct npu_report *)(uintptr_t)(base_addr + slot_size * slot_id);
}

static inline u32 npu_report_get_phase(struct npu_report *report)
{
	cond_return_error(report == NULL, dummy, "failed\n");
	return report->phase;
}

static inline u32 npu_report_get_sq_index(struct npu_report *report)
{
	cond_return_error(report == NULL, dummy, "failed\n");
	return report->sq_id;
}

static inline u32 npu_report_get_sq_head(struct npu_report *report)
{
	cond_return_error(report == NULL, dummy, "failed\n");
	return report->sq_head;
}

void npu_task_cnt_update(struct npu_dev_ctx *dev_ctx, u32 cnt, u32 flag)
{
	int ref_cnt;

	if (flag == OPS_ADD) {
		atomic_add(cnt, &dev_ctx->pm.task_ref_cnt);
		npu_drv_debug("add , task ref cnt = %d, cnt = %d\n",
			atomic_read(&dev_ctx->pm.task_ref_cnt), cnt);
	} else {
		ref_cnt = atomic_sub_return(cnt, &dev_ctx->pm.task_ref_cnt);
		npu_drv_debug("sub , task ref cnt = %d, cnt = %d\n",
			atomic_read(&dev_ctx->pm.task_ref_cnt), cnt);
		if (ref_cnt == 0) {
			npu_pm_add_idle_timer(dev_ctx);
			npu_drv_debug("idle timer add, ref_cnt = %d\n", cnt);
		}
	}
}

int npu_release_report(struct npu_proc_ctx *proc_ctx,
	struct npu_ts_cq_info *cq_info, u32 count)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_cq_sub_info *cq_sub_info = NULL;
	struct npu_report *p_report = NULL;
	u32 head;
	u32 i;

	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", proc_ctx->devid);
		return -1;
	}
	cond_return_error(cq_info == NULL, -1, "cq_info is null\n");
	cq_sub_info = npu_get_cq_sub_addr(cur_dev_ctx, cq_info->index);
	cond_return_error(cq_sub_info == NULL, -1,
		"invalid para cq info index %u\n", cq_info->index);

	/* remove task */
	for (i = 0; i < count; i++) {
		p_report = __npu_get_report(cq_sub_info->virt_addr,
			cq_info->slot_size,
			(cq_info->head + i) % NPU_MAX_CQ_DEPTH);
		npu_task_set_remove(
			cur_dev_ctx, p_report->stream_id, p_report->task_id);
	}
	npu_task_cnt_update(cur_dev_ctx, count, OPS_SUB);

	/* update head */
	spin_lock(&cur_dev_ctx->spinlock);
	cq_info->receive_count += count;
	head = (cq_info->head + count) % NPU_MAX_CQ_DEPTH;
	cq_info->head = head;
	spin_unlock(&cur_dev_ctx->spinlock);

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);
	if (cur_dev_ctx->power_stage == NPU_PM_UP)
		(void)npu_write_doorbell_val(
			DOORBELL_RES_CAL_CQ, cq_info->index, head);
	else
		npu_drv_err("npu is powered off, power_stage[%u]\n",
			cur_dev_ctx->power_stage);

	npu_drv_debug("receive report, cq_id= %u, cq_tail= %u, cq_head= %u, "
		"receive_count= %lld\n", cq_info->index, cq_info->tail,
		cq_info->head, cq_info->receive_count);
	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
	return 0;
}

int npu_proc_get_calc_sq_info(struct npu_proc_ctx *proc_ctx, u32 sq_index,
	npu_ts_sq_info_t **sq_info, struct npu_sq_sub_info **sq_sub_info)
{
	struct npu_dev_ctx *cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);

	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %u is null\n", proc_ctx->devid);
		return -1;
	}

	/* outer function should make sure paras are not null */
	*sq_info = npu_calc_sq_info(proc_ctx->devid, sq_index);
	if (*sq_info == NULL) {
		npu_drv_err("sq_index= %u, sq_info is null\n", sq_index);
		return -1;
	}

	*sq_sub_info = npu_get_sq_sub_addr(cur_dev_ctx, (*sq_info)->index);
	if (*sq_sub_info == NULL) {
		npu_drv_err("sq_index= %u sq_sub_info is null\n", sq_index);
		return -1;
	}

	return 0;
}

/* according to DevdrvUpdateCqTailAndSqHead() */
void npu_update_cq_tail_sq_head(
	struct npu_proc_ctx *proc_ctx, struct npu_cq_sub_info *cq_sub_info,
	struct npu_ts_cq_info *cq_info)
{
	npu_ts_sq_info_t *sq_info = NULL;
	struct npu_report *report = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	u8 dev_id = 0;
	u32 next_tail;
	u32 sq_head;

	npu_drv_debug("enter\n");

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_void(cur_dev_ctx == NULL, "cur dev ctx is null\n");
	report = __npu_get_report(cq_sub_info->virt_addr, cq_info->slot_size,
		cq_info->tail);

	npu_drv_debug("cq_tail=%u, cq_base_addr=%pK, valid_phase=%u\n",
		cq_info->tail, cq_sub_info->virt_addr,
		cq_info->phase);
	/* update cq_info->tail */
	while (npu_report_get_phase(report) == cq_info->phase) {
		npu_drv_debug("cq_tail=%u\n", cq_info->tail);
		next_tail = (cq_info->tail + 1) % NPU_MAX_CQ_DEPTH;

		/* cq_info->tail must not cover register->cq_head
		 * use one slot to keep tail not cover the head, which may have report
		 */
		if (next_tail == cq_info->head)
			break;

		sq_info = npu_calc_sq_info(proc_ctx->devid,
			npu_report_get_sq_index(report));
		cond_return_void(sq_info == NULL,
			"npu_calc_sq_info error, sq_index = %d\n",
			npu_report_get_sq_index(report));
		/* handle to which one (sq slot) */
		sq_head = npu_report_get_sq_head(report);
		if (sq_head >= NPU_MAX_SQ_DEPTH) {
			npu_drv_err("wrong sq head from cq, sqHead = %u\n", sq_head);
			break;
		}
		spin_lock(&cur_dev_ctx->spinlock);
		sq_info->head = sq_head;
		npu_drv_debug("update sqinfo[%d]: head:%d, tail:%d, credit:%d\n",
			sq_info->index, sq_info->head, sq_info->tail, sq_info->credit);

		next_tail = cq_info->tail + 1;
		if (next_tail > (NPU_MAX_CQ_DEPTH - 1)) {
			cq_info->phase = (cq_info->phase == 0) ? 1 : 0;
			cq_info->tail = 0;
		} else {
			cq_info->tail++;
		}
		report = __npu_get_report(cq_sub_info->virt_addr,
			cq_info->slot_size, cq_info->tail);
		spin_unlock(&cur_dev_ctx->spinlock);
	}
	npu_drv_debug("cq_tail=%u\n", cq_info->tail);
}

int npu_get_report(struct npu_proc_ctx *proc_ctx,
	struct npu_ts_cq_info *cq_info, struct npu_cq_sub_info *cq_sub_info,
	struct npu_receive_response_info *report_info)
{
	int ret;
	u32 count;
	struct npu_report *report = NULL;

	if ((proc_ctx == NULL) || (cq_info == NULL) || (cq_sub_info == NULL)) {
		npu_drv_err("invalid pointer\n");
		return -1;
	}

	report = __npu_get_report(cq_sub_info->virt_addr, cq_info->slot_size,
		cq_info->head);

	if (cq_info->tail > cq_info->head) {
		count = cq_info->tail - cq_info->head;
		if (count > cq_info->count_report)
			npu_drv_err("devid: %d, too much report, cq id: %d, "
				"cq head: %d, cq tail: %d, count: %d, count_report: %d\n",
				proc_ctx->devid, cq_info->index, cq_info->head,
				cq_info->tail, count, cq_info->count_report);
		/* use atomic sub */
	} else {
		count = NPU_MAX_CQ_DEPTH - cq_info->head;
		if (count > cq_info->count_report)
			npu_drv_err("devid: %d, too much report, cq id: %d, "
				"cq head: %d, cq tail: %d, count: %d, count_report: %d\n",
				proc_ctx->devid, cq_info->index, cq_info->head,
				cq_info->tail, count, cq_info->count_report);
		/* use atomic sub */
	}

	count = (report_info->response_count < count ?
		report_info->response_count : count);
	__sync_sub_and_fetch(&cq_info->count_report, count);
	ret = copy_to_user_safe(
		(void *)(uintptr_t)report_info->response_addr, report,
		count * sizeof(struct npu_report));
	if (ret != 0) {
		npu_drv_err("copy report to user fail ret %d\n", ret);
		return ret;
	}

	report_info->response_count = count;
	return 0;
}

int npu_proc_send_request_occupy(npu_ts_sq_info_t *sq_info)
{
	u32 cmd_count = 1; /* need to be sent */
	u32 available_count;
	u32 sq_head = sq_info->head;
	u32 sq_tail = sq_info->tail;
	/* if credit is less than cmdCount, update credit */
	if (sq_info->credit < cmd_count)
		sq_info->credit = (sq_tail >= sq_head) ?
			(NPU_MAX_SQ_DEPTH - (sq_tail - sq_head + 1)) :
			(sq_head - sq_tail - 1);

	if (sq_info->credit == 0) {
		npu_drv_warn(
			"no available sq slot, sq id= %u, sq head= %u, sq tail= %u\n",
			sq_info->index, sq_head, sq_tail);
		return -1;
	}

	if ((sq_tail >= sq_head) && (NPU_MAX_SQ_DEPTH - sq_tail <= cmd_count)) {
		if (sq_head == 0)
			available_count = NPU_MAX_SQ_DEPTH - sq_tail - 1;
		else
			available_count = NPU_MAX_SQ_DEPTH - sq_tail;

		if (available_count < cmd_count) {
			npu_drv_err("occupy fail available_count %d cmd_count %d\n",
				available_count, cmd_count);
			return -1;
		} else {
			return 0;
		}
	}

	if (sq_info->credit < cmd_count) {
		npu_drv_err("sq_info credit is not enough\n");
		return -1;
	}
	return 0;
}

int npu_proc_send_request_send(struct npu_dev_ctx *cur_dev_ctx,
	npu_ts_sq_info_t *sq_info, u32 sq_index, u32 cq_index, u32 cmd_count)
{
	struct npu_ts_cq_info *cq_info = NULL;
	u32 report_count = 1;
	u32 new_sq_tail;

	npu_drv_debug("enter\n");
	cq_info = npu_calc_cq_info(cur_dev_ctx->devid, cq_index);
	cond_return_error(cq_info == NULL, -1, "get cq info failed\n");

	/* if credit(available cmd slots) is less than cmdCount,
	 * there must be an error
	 */
	cond_return_error(sq_info->credit < cmd_count, -EINVAL,
		"sqid= %u, credit= %u, cmd_count= %d is too much\n",
		sq_info->index, sq_info->credit, cmd_count);

	spin_lock(&cur_dev_ctx->spinlock);
	new_sq_tail = (sq_info->tail + cmd_count) % NPU_MAX_SQ_DEPTH;

	sq_info->tail = new_sq_tail;
	sq_info->credit -= cmd_count;
	sq_info->send_count += cmd_count;
	/* use atomic add */
	__sync_add_and_fetch(&cq_info->count_report, report_count);
	spin_unlock(&cur_dev_ctx->spinlock);

	npu_drv_debug("update sqinfo, id= %u, head= %u, tail= %u, credit= %u\n",
		sq_info->index, sq_info->head,
		sq_info->tail, sq_info->credit);

	if (new_sq_tail == sq_info->head)
		npu_drv_err("too much cmd, count= %u, credit= %u, "
			"sq_id= %u, head= %u, tail= %u\n",
			cmd_count, sq_info->credit, sq_info->index,
			sq_info->head, sq_info->tail);

	/* update doorbell */
	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);
	if (cur_dev_ctx->power_stage == NPU_PM_UP)
		(void)npu_write_doorbell_val(
			DOORBELL_RES_CAL_SQ, sq_index, sq_info->tail);
	else
		npu_drv_err("npu is powered off, power_stage[%u]\n",
			cur_dev_ctx->power_stage);
	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	return 0;
}

static void npu_proc_process_exception_response(struct npu_dev_ctx *dev_ctx,
	struct npu_receive_response_info *report_info)
{
	int ret = 0;
	struct npu_report report = {0};
	struct npu_report *tmp = NULL;
	struct npu_report_payload *payload = NULL;
	struct npu_task_info *task = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	u32 i;
	u32 count = 0;

	/* 1. fake exception cqe by taskset */
	tmp = (struct npu_report *)(uintptr_t)report_info->response_addr;
	mutex_lock(&dev_ctx->pm.task_set_lock);
	for (i = 0; i < NPU_MAX_NON_SINK_STREAM_ID; i++) {
		list_for_each_safe(pos, n, &dev_ctx->pm.task_set[i]) {
			if (unlikely(count == report_info->response_count)) {
				npu_drv_warn("response buffer is full, response count = %u\n",
					report_info->response_count);
				break;
			}
			task = list_entry(pos, struct npu_task_info, node);
			report.task_id = task->task_id;
			report.stream_id = task->stream_id;
			payload = (struct npu_report_payload *)(&report.pay_load);
			payload->err_code = TS_EXCEPTION;
			if (report.task_id == dev_ctx->pm.npu_exception_task_id) {
				payload->persist_stream_id =
					dev_ctx->pm.npu_exception_persist_stream_id;
				payload->persist_task_id =
					dev_ctx->pm.npu_exception_persist_task_id;
			}
			ret = copy_to_user_safe((void *)(uintptr_t)tmp,
				&report, sizeof(struct npu_report));
			if (ret != 0) {
				npu_drv_err("copy report to user fail ret %d\n", ret);
				break;
			}
			list_del(&task->node);
			list_add(&task->node, &dev_ctx->pm.task_available_list);
			tmp++;
			count++;
		}
		if (unlikely(ret != 0 || count == report_info->response_count))
			break;
	}
	mutex_unlock(&dev_ctx->pm.task_set_lock);

	if (unlikely(count <= 0)) {
		npu_drv_err("no report, ret = %d, task_ref_cnt = %d",
			ret, atomic_read(&dev_ctx->pm.task_ref_cnt));
		return;
	}

	npu_task_cnt_update(dev_ctx, count, OPS_SUB);
	/* reset npu_exception_status */
	if (ret == 0 &&
		(count < report_info->response_count ||
		atomic_read(&dev_ctx->pm.task_ref_cnt) == 0)) {
		dev_ctx->pm.npu_exception_status = NPU_STATUS_NORMAL;
	} else {
		npu_drv_warn("has fake report, cnt = %d, buffer size = %d, ret = %d\n",
			count, report_info->response_count, ret);
		npu_drv_warn("has report not fake, cnt = %d\n",
			atomic_read(&dev_ctx->pm.task_ref_cnt));
	}
	report_info->response_count = count;
	report_info->wait_result = 1;
}

static int npu_proc_process_valid_response(struct npu_dev_ctx *dev_ctx,
	struct npu_proc_ctx *proc_ctx, struct npu_cq_sub_info *cq_sub_info,
	struct npu_ts_cq_info *cq_info,
	struct npu_receive_response_info *report_info)
{
	int ret = 0;

	npu_update_cq_tail_sq_head(proc_ctx, cq_sub_info, cq_info);
	/* 1. do not have valid cqe */
	if (cq_info->head == cq_info->tail)
		return -1;

	/* 2. have valid cqe */
	ret = npu_get_report(proc_ctx, cq_info, cq_sub_info, report_info);
	cond_return_error(ret != 0, ret, "fail to get report ret= %d\n", ret);

	ret = npu_release_report(proc_ctx, cq_info, report_info->response_count);
	cond_return_error(ret != 0, ret, "fail to release report ret= %d\n", ret);

	report_info->wait_result = 1;
	return 0;
}

int npu_proc_process_receive_response(struct npu_dev_ctx *dev_ctx,
	struct npu_proc_ctx *proc_ctx, struct npu_cq_sub_info *cq_sub_info,
	struct npu_ts_cq_info *cq_info,
	struct npu_receive_response_info *report_info)
{
	int ret = 0;
	down_read(&dev_ctx->pm.exception_lock);

	if (dev_ctx->pm.npu_exception_status == NPU_STATUS_EXCEPTION)
		/* 1. exception */
		npu_proc_process_exception_response(dev_ctx, report_info);
	else
		/* 2. get cqe report */
		ret = npu_proc_process_valid_response(
			dev_ctx, proc_ctx, cq_sub_info, cq_info, report_info);

	up_read(&dev_ctx->pm.exception_lock);
	return ret;
}

int npu_proc_receive_response_wait(struct npu_proc_ctx *proc_ctx,
	struct npu_cq_sub_info *cq_sub_info, struct npu_ts_cq_info *cq_info,
	struct npu_receive_response_info *report_info)
{
	long ret;
	unsigned long tmp;
	int wait_time = report_info->timeout;
	int wait_event_once = 1000; /* 1s */
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	cond_return_error(cq_info == NULL, -1, "cq_info is null\n");
	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", proc_ctx->devid);
		return -1;
	}

	do {
		/* 1. process report:1)exception; 2)valid cqe */
		if (npu_proc_process_receive_response(
			cur_dev_ctx, proc_ctx, cq_sub_info, cq_info, report_info) == 0)
			return 0;

		/* 2. wait */
		proc_ctx->cq_tail_updated = CQ_HEAD_INITIAL_FLAG;
		if (report_info->timeout == -1) {
			wait_event(
				proc_ctx->report_wait,
				proc_ctx->cq_tail_updated == CQ_HEAD_UPDATED_FLAG);
		} else {
			tmp = (wait_time >= wait_event_once ?
				msecs_to_jiffies(wait_event_once) :
				msecs_to_jiffies(wait_time));
			ret = wait_event_timeout(proc_ctx->report_wait, /*lint !e578*/
				proc_ctx->cq_tail_updated == CQ_HEAD_UPDATED_FLAG, tmp);
			wait_time -= wait_event_once;
			if (ret == 0 && wait_time <= 0) {
				/* timeout */
				npu_exception_timeout(cur_dev_ctx, proc_ctx);
				npu_proc_process_receive_response(
					cur_dev_ctx, proc_ctx, cq_sub_info, cq_info, report_info);
				break;
			}
		}
	} while (1);

	return 0;
}

extern int hisi_svm_get_ttbr(u64 *ttbr, u64 *tcr, pid_t process_id);
static npu_complete_comm_sqe(u8 devid, npu_rt_task_t *comm_task)
{
	uint64_t ttbr;
	uint16_t ssid;
	uint64_t tcr;
	int ret;

	cond_return_error(comm_task == NULL, -1, "comm_task is null\n");
	if (comm_task->type == NPU_TASK_MODEL_EXECUTE) {
		// v200:new procedure
		ret = npu_get_ssid_bypid(devid, current->tgid,
			comm_task->u.model_execute_task.execute_pid,
			&ssid, &ttbr, &tcr);
		cond_return_error(ret != 0, -1,
			"npu_get_ssid_bypid fail, ret= %d\n", ret);
#ifdef NPU_ARCH_V100
		ret = hisi_svm_get_ttbr(&ttbr, &tcr, comm_task->u.model_execute_task.execute_pid);
		cond_return_error(ret != 0, -1,
			"fail to get process ttbr info, ret= %d\n", ret);
#endif
		comm_task->u.model_execute_task.asid = ttbr >> 48;
		comm_task->u.model_execute_task.asid_baddr = ttbr &
			(0x0000FFFFFFFFFFFFu);
		comm_task->u.model_execute_task.smmu_svm_ssid = ssid;
		comm_task->u.model_execute_task.tcr = tcr;
	}
	return 0;
}

int npu_send_request(struct npu_proc_ctx *proc_ctx,
	npu_rt_task_t *comm_task)
{
	npu_stream_info_t *stream_info = NULL;
	npu_ts_sq_info_t *sq_info = NULL;
	struct npu_sq_sub_info *sq_sub_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	u32 cmd_count = 1; /* send one by one time */
	int ret = 0;

	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx %d is null\n",
		proc_ctx->devid);

	down_read(&cur_dev_ctx->pm.exception_lock);

	cond_goto_error(
		cur_dev_ctx->pm.npu_exception_status == NPU_STATUS_EXCEPTION,
		NPU_EXCEPTION, ret, -1, "npu is in an abnormal state\n");
	npu_task_cnt_update(cur_dev_ctx, cmd_count, OPS_ADD);
	ret = npu_pm_enter_workmode(proc_ctx, cur_dev_ctx, NPU_NONSEC);
	cond_goto_error(ret != 0, PRE_SEND_FAIL, ret, ret,
		"first task : power up fail, ret= %d\n", ret);

	/* verify ts_sqe */
	ret = npu_verify_ts_sqe(comm_task);
	cond_goto_error(ret != 0, PRE_SEND_FAIL, ret, ret,
		"npu_verify_ts_sqe fail, ret= %d\n", ret);
	/* complete ts_sqe */
	ret = npu_complete_comm_sqe(proc_ctx->devid, comm_task);
	cond_goto_error(ret != 0, PRE_SEND_FAIL, ret, ret,
		"npu_complete_comm_sqe fail, ret= %d\n", ret);
	/* save ts_sqe */
	stream_info = npu_calc_stream_info(proc_ctx->devid,
		comm_task->stream_id);
	cond_goto_error(stream_info == NULL, PRE_SEND_FAIL, ret, -1,
		"npu_calc_stream_info fail, ret= %d\n", -1);
	ret = npu_proc_get_calc_sq_info(proc_ctx, stream_info->sq_index,
		&sq_info, &sq_sub_info);
	cond_goto_error(((ret != 0 || sq_info == NULL || sq_sub_info == NULL)),
		PRE_SEND_FAIL, ret, ret,
		"npu_proc_send_request_occypy failed, "
		"sq_info or sq_sub_info is NULL\n");

	npu_drv_debug("sq_sub_info, index= %u, "
		"ref_by_streams= %u, phy_addr= 0x%llx, virt_addr= 0x%llx\n",
		sq_sub_info->index, sq_sub_info->ref_by_streams,
		sq_sub_info->phy_addr, sq_sub_info->virt_addr);

	ret = npu_proc_send_request_occupy(sq_info);
	cond_goto_error(ret != 0, PRE_SEND_FAIL, ret, ret,
		"npu_proc_send_request_occypy failed, sq_id= %u, sq_head= %u sq_tail= %u\n",
		sq_info->index, sq_info->head, sq_info->tail);

	ret = npu_format_ts_sqe((void *)(uintptr_t)sq_sub_info->virt_addr,
		comm_task, sq_info->tail);
	cond_goto_error(ret != 0, PRE_SEND_FAIL, ret, ret, "format sqe failed, "
		"ret= %d, virt_addr= 0x%llx, phy_addr= 0x%llx, tail= %u\n",
		ret, sq_sub_info->virt_addr, sq_sub_info->phy_addr, sq_info->tail);

	ret = npu_task_set_insert(
		cur_dev_ctx, stream_info->id, comm_task->task_id);
	cond_goto_error(ret != 0, PRE_SEND_FAIL, ret, ret,
		"insert task set failed, ret= %d, stream id = %u, task id = %u\n",
		ret, stream_info->id, comm_task->task_id);

	ret = npu_proc_send_request_send(cur_dev_ctx, sq_info,
		stream_info->sq_index, stream_info->cq_index, cmd_count);
	cond_goto_error(ret != 0, SEND_FAIL, ret, ret,
		"npu_proc_send_request_occypy failed, sq_id= %u, sq_head= %u sq_tail= %u\n",
		sq_info->index, sq_info->head, sq_info->tail);

	npu_drv_debug("communication stream= %d, sq_indx= %u, sq_tail= %u, send_count= %lld\n",
		stream_info->id,
		stream_info->sq_index, sq_info->tail, sq_info->send_count);

	up_read(&cur_dev_ctx->pm.exception_lock);
	return 0;

SEND_FAIL:
	npu_task_set_remove(cur_dev_ctx, stream_info->id, comm_task->task_id);
PRE_SEND_FAIL:
	npu_task_cnt_update(cur_dev_ctx, cmd_count, OPS_SUB);
NPU_EXCEPTION:
	up_read(&cur_dev_ctx->pm.exception_lock);
	return ret;
}

int npu_receive_response(struct npu_proc_ctx *proc_ctx,
	struct npu_receive_response_info *report_info)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_cq_sub_info *cq_sub_info = NULL;
	struct npu_ts_cq_info *cq_info = NULL;
	int ret = 0;
	u32 cq_id = 0xFF;

	cond_return_error(proc_ctx == NULL, -1, "invalid proc ctx\n");

	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(cur_dev_ctx == NULL, -1, "invalid dev ctx\n");
	ret = npu_proc_get_cq_id(proc_ctx, &cq_id);
	cond_return_error((ret != 0), -1, "get cq fail\n");

	cq_info = npu_calc_cq_info(proc_ctx->devid, cq_id);
	cond_return_error(cq_info == NULL, -1, "get cq info failed\n");

	cq_sub_info = npu_get_cq_sub_addr(cur_dev_ctx, cq_info->index);
	cond_return_error(cq_sub_info == NULL, -1,
		"invalid para cq info index %u\n", cq_info->index);

	npu_set_report_timeout(cur_dev_ctx, &(report_info->timeout));
	ret = npu_proc_receive_response_wait(proc_ctx, cq_sub_info, cq_info,
		report_info);
	if (report_info->wait_result <= 0) {
		npu_drv_info("drv receive response wait timeout, need powerdown!!!\n");
		return ret;
	}
	cond_return_error((ret != 0), ret,
		"receive response wait ret %d result %d\n", ret,
		report_info->wait_result);

	return 0;
}

