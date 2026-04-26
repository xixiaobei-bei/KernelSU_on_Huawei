/*
 * npu_calc_channel.c
 *
 * about npu calc channel
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
#include "npu_calc_channel.h"

#include <linux/errno.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

#include "npu_stream.h"
#include "npu_sink_stream.h"
#include "npu_calc_sq.h"
#include "npu_calc_cq.h"
#include "npu_mailbox_msg.h"
#include "npu_mailbox.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_pm_framework.h"
#include "npu_doorbell.h"
#include "npu_hwts.h"

int npu_send_alloc_stream_mailbox(u8 cur_dev_id, int stream_id, int cq_id)
{
	struct npu_stream_msg *alloc_stream_msg = NULL;
	int mbx_send_result = -1;
	u32 msg_len;
	int ret;

	(void)npu_inc_cq_ref_by_communication_stream(cur_dev_id, cq_id);
	// call mailbox to info ts to create stream
	alloc_stream_msg = (struct npu_stream_msg *)kzalloc(
		sizeof(struct npu_stream_msg), GFP_KERNEL);
	if (alloc_stream_msg == NULL) {
		ret = -ENOMEM;
		npu_drv_err("kzalloc alloc_stream_msg failed, ret = %d\n", ret);
		goto alloc_stream_msg_failed;
	}

	(void)npu_create_alloc_stream_msg(cur_dev_id, stream_id,
		alloc_stream_msg);
	msg_len = sizeof(struct npu_stream_msg);
	ret = npu_mailbox_message_send_for_res(cur_dev_id,
		(u8 *)alloc_stream_msg, msg_len, &mbx_send_result);
	if (ret != 0) {
		npu_drv_err("alloc stream mailbox_message_send failed"
			" mbx_send_result = %d ret = %d\n", mbx_send_result, ret);
		goto message_send_for_res_failed;
	}

	npu_drv_debug("alloc stream mailbox_message_send success"
		" mbx_send_result = %d ret = %d\n", mbx_send_result, ret);
	kfree(alloc_stream_msg);
	alloc_stream_msg = NULL;
	return 0;

message_send_for_res_failed:
	kfree(alloc_stream_msg);
	alloc_stream_msg = NULL;
alloc_stream_msg_failed:
	(void)npu_dec_cq_ref_by_communication_stream(cur_dev_id, cq_id);
	return -1;
}

struct npu_stream_info *npu_alloc_sink_stream(u8 dev_id, u8 is_long,
	u32 sq_id, u32 cq_id)
{
	u32 hwts_cq_id = cq_id;
	u32 hwts_sq_id = sq_id;
	int stream_id;
	struct npu_stream_info *stream_info = NULL;
	int ret = 0;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return NULL;
	}

	stream_id = npu_alloc_sink_stream_id(dev_id, is_long);
	if (stream_id < NPU_MAX_NON_SINK_STREAM_ID) {
		npu_drv_err("alloc sink stream_id from dev %d failed\n", dev_id);
		return NULL;
	}

	stream_info = npu_calc_stream_info(dev_id, stream_id);
	if (stream_info == NULL) {
		npu_drv_err("sink stream info is null\n");
		goto calc_stream_info_failed;
	}

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {

		ret = npu_bind_stream_with_hwts_sq(dev_id, stream_id, hwts_sq_id);
		if (ret != 0) {
			npu_drv_err("bind hwts_sq:%d to stream:%d failed\n",
				hwts_sq_id, stream_id);
			goto bind_stream_with_hwts_sq_failed;
		}

		ret = npu_bind_stream_with_hwts_cq(dev_id, stream_id, hwts_cq_id);
		if (ret != 0) {
			npu_drv_err("alloc hwts_sq_mem from dev %d failed\n", dev_id);
			goto bind_stream_with_hwts_cq_failed;
		}

		npu_drv_debug("alloc sink-stream success stream_id:%d, hwts_sq_id = %d, hwts_cq_id = %d\n",
			stream_info->id, stream_info->sq_index, stream_info->cq_index);
	}
	return stream_info;

// --- hwts feature switch ---
bind_stream_with_hwts_cq_failed:
bind_stream_with_hwts_sq_failed:
calc_stream_info_failed:
	npu_free_sink_stream_id(dev_id, stream_id);

	return NULL;
}

struct npu_stream_info *npu_alloc_non_sink_stream(u8 cur_dev_id,
	u32 sq_id, u32 cq_id)
{
	int ret = 0;
	struct npu_stream_info *stream_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	int inform_ts = NPU_NO_NEED_TO_INFORM;
	int stream_id = npu_alloc_stream_id(cur_dev_id);

	cond_return_error(stream_id < 0 ||
		stream_id >= NPU_MAX_NON_SINK_STREAM_ID, NULL,
		"alloc stream_id from dev %d failed\n", cur_dev_id);

	stream_info = npu_calc_stream_info(cur_dev_id, stream_id);
	cond_goto_error(stream_info == NULL, calc_stream_info_failed, ret, 0,
		"sink stream info is null\n");

	// bind stream with sq_id
	cond_goto_error(npu_bind_stream_with_sq(cur_dev_id, stream_id, sq_id),
		bind_stream_with_sq_failed,
		ret, ret, "bind stream = %d with sq_id = %d from dev %d failed\n",
		stream_id, sq_id, cur_dev_id);

	// increase sq ref by current stream
	(void)npu_inc_sq_ref_by_stream(cur_dev_id, sq_id);

	// bind stream with cq_id
	cond_goto_error(npu_bind_stream_with_cq(cur_dev_id, stream_id, cq_id),
		bind_stream_with_cq_failed,
		ret, ret, "bind stream = %d with cq_id = %d from dev %d failed\n",
		stream_id, cq_id, cur_dev_id);
	(void)npu_inc_cq_ref_by_stream(cur_dev_id, cq_id);

	cur_dev_ctx = get_dev_ctx_by_id(cur_dev_id);

	cond_return_error(cur_dev_ctx == NULL, NULL, "cur_dev_ctx %d is null\n",
		cur_dev_id);

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	if (cur_dev_ctx->power_stage != NPU_PM_UP) {
		npu_drv_info("alloc stream no need to inform ts, stream:%d sq:%u\n",
			stream_id, sq_id);
		inform_ts = NPU_NO_NEED_TO_INFORM;
	} else if (bitmap_get(cur_dev_ctx->pm.work_mode, NPU_SEC)) {
		npu_drv_warn("no need to inform ts in work_mode:%u stream:%d sq:%u\n",
			cur_dev_ctx->pm.work_mode, stream_id, sq_id);
		inform_ts = NPU_NO_NEED_TO_INFORM;
	} else {
		// mailbox notification is needed in case of stream with shared sq,
		// and ts_fw would init stream info but sqcq info.
		npu_drv_info("alloc stream inform ts, stream:%d sq:%u\n", stream_id,
			sq_id);
		inform_ts = NPU_HAVE_TO_INFORM;
	}

	if (inform_ts == NPU_HAVE_TO_INFORM) {
		ret = npu_send_alloc_stream_mailbox(cur_dev_id, stream_id, cq_id);
		if (ret != 0) {
			mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
			npu_drv_err("send alloc stream:%d sq:%u mailbox failed\n",
				stream_id, sq_id);
			goto send_alloc_stream_mailbox;
		}
	}

	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	npu_drv_debug("alloc non-sink stream:%d with sq:%u\n", stream_id, sq_id);
	return stream_info;

send_alloc_stream_mailbox:
bind_stream_with_cq_failed:
	npu_dec_sq_ref_by_stream(cur_dev_id, sq_id);
bind_stream_with_sq_failed:
calc_stream_info_failed:
	npu_free_stream_id(cur_dev_id, stream_id);
	return NULL;
}

struct npu_stream_info *npu_alloc_stream(u32 strategy, u32 sq_id,
	u32 cq_id)
{
	const u8 cur_dev_id = 0;
	struct npu_stream_info *stream_info = NULL;

	if (strategy & STREAM_STRATEGY_SINK) {
		stream_info = npu_alloc_sink_stream(cur_dev_id,
			strategy & STREAM_STRATEGY_LONG, sq_id, cq_id);
		if (stream_info != NULL)
			stream_info->strategy = strategy;

		return stream_info;
	}

	stream_info = npu_alloc_non_sink_stream(cur_dev_id, sq_id, cq_id);
	if (stream_info != NULL)
		stream_info->strategy = strategy;

	return stream_info;
}

int npu_free_sink_stream(u8 dev_id, int stream_id)
{
	struct npu_stream_info *stream_info = NULL;
	int ret;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info\n");
		return -1;
	}

	ret = npu_free_sink_stream_id(dev_id, stream_id);
	if (ret != 0) {
		npu_drv_err("free sink stream id:%d, dev:%d failed\n",
			stream_id, dev_id);
		return -1;
	}

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
		stream_info = npu_calc_stream_info(dev_id, stream_id);
		if (stream_info == NULL) {
			npu_drv_err("calc stream info failed, stream_id:%d, dev:%d\n",
				stream_id, dev_id);
			return -1;
		}
	}
	return 0;
}

int npu_free_non_sink_stream(u8 dev_id, int stream_id, u32 *sq_send_count)
{
	struct npu_stream_msg *free_stream_msg = NULL;
	int mbx_send_result = -1;
	int ret;
	struct npu_stream_info *stream_info = NULL;
	u32 sq_id;
	u32 cq_id;
	struct npu_dev_ctx *cur_dev_ctx = get_dev_ctx_by_id(dev_id);

	cond_return_error(cur_dev_ctx == NULL, -EINVAL,
		"cur_dev_ctx %d is null\n", dev_id);

	stream_info = npu_calc_stream_info(dev_id, stream_id);
	cond_return_error(stream_info == NULL, -1, "stream info is null\n");

	sq_id = stream_info->sq_index;
	cq_id = stream_info->cq_index;

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	if (cur_dev_ctx->power_stage != NPU_PM_UP ||
		(cur_dev_ctx->will_powerdown == true)) {
		npu_drv_info("free stream no need to inform ts\n");
	} else if (bitmap_get(cur_dev_ctx->pm.work_mode, NPU_SEC)) {
		npu_drv_warn("no need to inform ts in work_mode:%u, stream:%d sq:%u\n",
			cur_dev_ctx->pm.work_mode, stream_id, sq_id);
	} else if (npu_get_sq_ref_by_stream(dev_id, sq_id) > 1) {
		// free stream msg should not be sent to ts in case of stream
		// with shared sq, as ts_fw would init sqcq_info.
		npu_drv_info("free stream with sahred sq, no need to inform ts, stream:%d sq:%u\n",
			stream_id, sq_id);
	} else { // call mailbox to info ts to free stream
		npu_drv_info("free stream inform ts, stream:%d sq:%u\n",
			stream_id, sq_id);
		(void)npu_dec_cq_ref_by_communication_stream(dev_id, cq_id);
		free_stream_msg = (struct npu_stream_msg *)kzalloc(
			sizeof(struct npu_stream_msg), GFP_KERNEL);
		cond_goto_error(free_stream_msg == NULL, fail, ret, -1,
			"kzalloc free_stream_msg failed, will cause resource leak\n");

		(void)npu_create_free_stream_msg(dev_id, stream_id, free_stream_msg);

		ret = npu_mailbox_message_send_for_res(dev_id, (u8 *)free_stream_msg,
			sizeof(struct npu_stream_msg), &mbx_send_result);
		kfree(free_stream_msg);
		free_stream_msg = NULL;
		cond_goto_error(ret != 0, fail, ret, -1,
			"free stream mailbox_message_send failed will cause "
			"resource leak result:%d ret:%d stream_id:%d sq_id:%u\n",
			mbx_send_result, ret, stream_id, sq_id);

		npu_drv_debug("free stream mailbox_message_send success"
			" mbx_send_result = %d ret = %d\n", mbx_send_result, ret);
	}
	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	// dec ref of cq used by cur stream
	npu_dec_cq_ref_by_stream(dev_id, cq_id);
	// dec ref of sq used by cur stream
	npu_dec_sq_ref_by_stream(dev_id, sq_id);

	// to make sure upper layer get right data when no free sq
	*sq_send_count = 0;
	if (npu_get_sq_ref_by_stream(dev_id, sq_id) == 0) {
		npu_get_sq_send_count(dev_id, sq_id, sq_send_count);
		npu_drv_debug("prepare free dev %d sq %d total sq_send_count=%d\n",
			dev_id, sq_id, *sq_send_count);
	}
	// add stream_info to dev_ctx ->stream_available_list
	return npu_free_stream_id(dev_id, stream_id);
fail:
	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
	return ret;
}

int npu_free_stream(u8 dev_id, u32 stream_id, u32 *sq_send_count)
{
	struct npu_stream_info *stream_info = NULL;
	struct npu_platform_info *plat_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (stream_id >= NPU_MAX_STREAM_ID) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (sq_send_count == NULL) {
		npu_drv_err("sq_send_count ptr is null\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -EINVAL;
	}

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_ops failed\n");
		return -EFAULT;
	}

	stream_info = npu_calc_stream_info(dev_id, stream_id);
	if (stream_info == NULL) {
		npu_drv_err("get stream_info failed\n");
		return -EFAULT;
	} else if (stream_info->strategy & STREAM_STRATEGY_SINK) {
		npu_drv_debug("free sink stream\n");
		return npu_free_sink_stream(dev_id, stream_id);
	} else {
		return npu_free_non_sink_stream(dev_id, stream_id, sq_send_count);
	}

	return 0;
}
