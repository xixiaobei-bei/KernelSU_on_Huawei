/*
 * about_npu_recycle.c
 *
 * about about npu recycle
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
#include "npu_recycle.h"

#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>
#include <securec.h>

#include "npu_event.h"
#include "npu_hwts_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "npu_mailbox_msg.h"
#include "npu_proc_ctx.h"
#include "npu_user_common.h"
#include "npu_common.h"
#include "npu_calc_cq.h"
#include "npu_calc_sq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_pm_framework.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "npu_hwts.h"

int npu_recycle_event_id(struct npu_proc_ctx *proc_ctx)
{
	struct npu_id_entity *event_info = NULL;
	struct npu_platform_info *plat_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct list_head *pos = NULL, *n = NULL;

	if (list_empty_careful(&proc_ctx->event_list)) {
		npu_drv_debug("proc context event list is empty\n");
		return 0;
	}

	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(cur_dev_ctx == NULL, -EINVAL,
		"cur_dev_ctx %d is null\n", proc_ctx->devid);

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -EFAULT,
		"get plat_ops failed\n");

	mutex_lock(&proc_ctx->event_mutex);

	list_for_each_safe(pos, n, &proc_ctx->event_list) {
		event_info = list_entry(pos, struct npu_id_entity, list);
		if (event_info != NULL)
			(void)npu_proc_free_event(proc_ctx, event_info->id,
			EVENT_STRATEGY_TS);
	}
	mutex_unlock(&proc_ctx->event_mutex);

	npu_drv_debug("recycle %d event resource success\n",
		proc_ctx->event_num);
	return 0;
}

int npu_recycle_hwts_event_id(struct npu_proc_ctx *proc_ctx)
{
	struct npu_id_entity *event_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (list_empty_careful(&proc_ctx->hwts_event_list)) {
		npu_drv_debug("proc context hwts event list is empty\n");
		return 0;
	}

	mutex_lock(&proc_ctx->event_mutex);
	/* no need inform ts */
	list_for_each_safe(pos, n, &proc_ctx->hwts_event_list) {
		event_info = list_entry(pos, struct npu_id_entity, list);
		if (event_info != NULL)
			(void)npu_proc_free_event(proc_ctx, event_info->id,
				EVENT_STRATEGY_HWTS);
	}
	mutex_unlock(&proc_ctx->event_mutex);

	npu_drv_debug("recycle %u hwts event resource success\n",
		proc_ctx->hwts_event_num);
	return 0;
}

void npu_recycle_model_id(struct npu_proc_ctx *proc_ctx)
{
	struct npu_id_entity *model_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (list_empty_careful(&proc_ctx->model_list)) {
		npu_drv_debug("proc context model list is empty\n");
		return;
	}

	mutex_lock(&proc_ctx->model_mutex);
	list_for_each_safe(pos, n, &proc_ctx->model_list) {
		model_info = list_entry(pos, struct npu_id_entity, list);
		if (model_info != NULL)
			(void)npu_proc_free_model(proc_ctx, model_info->id);
	}
	mutex_unlock(&proc_ctx->model_mutex);
}

void npu_recycle_task_id(struct npu_proc_ctx *proc_ctx)
{
	struct npu_id_entity *task_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (list_empty_careful(&proc_ctx->task_list)) {
		npu_drv_debug("proc context task list is empty\n");
		return;
	}

	mutex_lock(&proc_ctx->task_mutex);
	list_for_each_safe(pos, n, &proc_ctx->task_list) {
		task_info = list_entry(pos, struct npu_id_entity, list);
		if (task_info != NULL)
			(void)npu_proc_free_task(proc_ctx, task_info->id);
	}
	mutex_unlock(&proc_ctx->task_mutex);
}

bool npu_is_proc_resource_leaks(const struct npu_proc_ctx *proc_ctx)
{
	bool result = false;

	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is null\n");
		return false;
	}

	if (!list_empty_careful(&proc_ctx->message_list_header) ||
		atomic_read(&proc_ctx->mailbox_message_count) ||
		!list_empty_careful(&proc_ctx->sink_stream_list) ||
		!list_empty_careful(&proc_ctx->stream_list) ||
		!list_empty_careful(&proc_ctx->event_list) ||
		!list_empty_careful(&proc_ctx->hwts_event_list) ||
		!list_empty_careful(&proc_ctx->model_list) ||
		!list_empty_careful(&proc_ctx->task_list))
		result = true;

	return result;
}

void npu_resource_leak_print(const struct npu_proc_ctx *proc_ctx)
{
	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is null\n");
		return;
	}

	npu_drv_warn("some npu resource are not released. Process Name: %s "
		"PID: %d, TGID: %d\n", current->comm, current->pid, current->tgid);

	if (!list_empty_careful(&proc_ctx->message_list_header))
		npu_drv_warn("message_list_header is not empty\n");

	if (atomic_read(&proc_ctx->mailbox_message_count))
		npu_drv_warn("leak mailbox_message_count is %d\n",
			proc_ctx->mailbox_message_count.counter);

	if (!list_empty_careful(&proc_ctx->sink_stream_list))
		npu_drv_warn(
			"some sink stream id are not released !! stream num = %d\n",
			proc_ctx->sink_stream_num);

	if (!list_empty_careful(&proc_ctx->stream_list))
		npu_drv_warn(
			"some non sink stream id are not released !! stream num = %d\n",
			proc_ctx->stream_num);

	if (!list_empty_careful(&proc_ctx->event_list))
		npu_drv_warn("some event id are not released !! event num = %d\n",
			proc_ctx->event_num);

	if (!list_empty_careful(&proc_ctx->hwts_event_list))
		npu_drv_warn(
			"some hwts event id are not released !! hwts event num = %u\n",
			proc_ctx->hwts_event_num);

	if (!list_empty_careful(&proc_ctx->model_list))
		npu_drv_warn("some model id are not released !! model num = %d\n",
			proc_ctx->model_num);

	if (!list_empty_careful(&proc_ctx->task_list))
		npu_drv_warn("some task id are not released !! task num = %d\n",
			proc_ctx->task_num);
}

// it makes sense only runtime ,driver and ts  work together and driver do not
// totally free cq(don not clear cq_head and cq_tail to zero value)
static void npu_update_cq_info_phase(struct npu_proc_ctx *proc_ctx)
{
	struct npu_cq_sub_info *cq_sub_info = NULL;
	struct npu_ts_cq_info *cq_info = NULL;
	u32 report_phase;

	if (list_empty_careful(&proc_ctx->cq_list)) {
		npu_drv_err("cur proc_ctx cq_list null\n");
		return;
	}

	cq_sub_info = list_first_entry(&proc_ctx->cq_list,
		struct npu_cq_sub_info, list);
	cond_return_void(cq_sub_info == NULL, "cq_sub_info is null");
	cq_info = npu_calc_cq_info(proc_ctx->devid, cq_sub_info->index);
	cond_return_void(cq_info == NULL, "cq_info is null");
	report_phase = npu_proc_get_cq_head_report_phase(proc_ctx);
	if (report_phase == CQ_INVALID_PHASE)
		return;

	if (cq_info->head == cq_info->tail) {
		// overturn the cq phase when the end of a round
		if (cq_info->head == NPU_MAX_CQ_DEPTH - 1) {
			cq_info->phase = (u32)((report_phase == 1) ? 0 : 1);
			npu_drv_warn("cq %d phase overturned,cq head = %d report phase"
				" = %d, cq tail = %d, phase = %d\n", cq_info->index,
				cq_info->head, report_phase, cq_info->tail, cq_info->phase);
		}
	} else if (cq_info->head < cq_info->tail) {
		cq_info->phase = report_phase;
		npu_drv_warn("cq %d phase no overturned,cq head = %d ,"
			"cq tail = %d report phase = %d, phase = %d\n",
			cq_info->index, cq_info->head,
			cq_info->tail, report_phase, cq_info->phase);
	} else {
		cq_info->phase = (u32)((report_phase == 1) ? 0 : 1);
		npu_drv_warn("cq %d phase overturned,cq head = %d report phase"
			" = %d, tail = %d, phase = %d\n",
			cq_info->index, cq_info->head, report_phase,
			cq_info->tail, cq_info->phase);
	}
}

static int npu_recycle_sink_stream_list(struct npu_proc_ctx *proc_ctx)
{
	int ret;
	int error = 0;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct npu_id_entity *stream_sub = NULL;

	list_for_each_safe(pos, n, &proc_ctx->sink_stream_list) {
		stream_sub = list_entry(pos, struct npu_id_entity, list);

		mutex_lock(&proc_ctx->stream_mutex);
		// no need to inform ts
		ret = npu_proc_free_stream(proc_ctx, stream_sub->id);
		mutex_unlock(&proc_ctx->stream_mutex);

		if (ret != 0)
			error++;
	}

	return error;
}

static int npu_recycle_stream_list(struct npu_proc_ctx *proc_ctx)
{
	int ret;
	int error = 0;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct npu_id_entity *stream_sub = NULL;

	list_for_each_safe(pos, n, &proc_ctx->stream_list) {
		stream_sub = list_entry(pos, struct npu_id_entity, list);

		mutex_lock(&proc_ctx->stream_mutex);
		ret = npu_proc_free_stream(proc_ctx, stream_sub->id);
		mutex_unlock(&proc_ctx->stream_mutex);

		if (ret != 0)
			error++;
	}

	return error;
}

static int npu_recycle_stream(struct npu_proc_ctx *proc_ctx)
{
	int error = 0;

	if (list_empty_careful(&proc_ctx->sink_stream_list) &&
		list_empty_careful(&proc_ctx->stream_list)) {
		npu_drv_debug("no stream leaks, no need to recycle\n");
		return 0;
	}

	npu_update_cq_info_phase(proc_ctx);

	if (!list_empty_careful(&proc_ctx->sink_stream_list))
		error += npu_recycle_sink_stream_list(proc_ctx);

	if (!list_empty_careful(&proc_ctx->stream_list))
		error += npu_recycle_stream_list(proc_ctx);

	if (error != 0) {
		error = -error;
		npu_drv_err(
			"recycle %d sink stream %d non sink stream resource error happened ,"
			" error times = %d\n",
			proc_ctx->sink_stream_num, proc_ctx->stream_num, error);
		return -1;
	}

	npu_drv_debug("recycle %d stream resource success\n",
		proc_ctx->stream_num);
	return 0;
}

static void npu_recycle_cq(struct npu_proc_ctx *proc_ctx)
{
	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is null\n");
		return;
	}

	npu_unbind_proc_ctx_with_cq_int_ctx(proc_ctx);
	(void)npu_remove_proc_ctx(&proc_ctx->dev_ctx_list, proc_ctx->devid);
	(void)npu_proc_free_cq(proc_ctx);
}

void npu_recycle_npu_resources(struct npu_proc_ctx *proc_ctx)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	int ret;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return;
	}

	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is null\n");
		return;
	}

	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx is null,no leak resource get recycled\n");
		return;
	}

	/* recycle stream */
	ret = npu_recycle_stream(proc_ctx);
	if (ret != 0) {
		npu_drv_err("npu_recycle_stream failed\n");
		goto recycle_error;
	}

	/* recycle event */
	ret = npu_recycle_event_id(proc_ctx);
	if (ret != 0) {
		npu_drv_err("npu_recycle_event failed\n");
		goto recycle_error;
	}

	/* recycle hwts event */
	ret = npu_recycle_hwts_event_id(proc_ctx);
	if (ret != 0) {
		npu_drv_err("npu_recycle_hwts_event failed\n");
		goto recycle_error;
	}

	/* recycle model */
	npu_recycle_model_id(proc_ctx);
	npu_recycle_task_id(proc_ctx);

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS])
		npu_recycle_proc_ctx_sub(proc_ctx);
	/* recycle cq */
	npu_recycle_cq(proc_ctx);
	npu_drv_warn("recycle all sources success\n");

recycle_error:
	// unbind
	npu_unbind_proc_ctx_with_cq_int_ctx(proc_ctx);
	(void)npu_remove_proc_ctx(&proc_ctx->dev_ctx_list, proc_ctx->devid);
}
