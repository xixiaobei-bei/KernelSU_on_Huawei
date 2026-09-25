/*
 * npu_calc_channel.c
 *
 * about npu calc channel
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
#include "npu_pm.h"
#include "npu_doorbell.h"
#include "npu_hwts.h"

struct devdrv_ts_cq_info *devdrv_alloc_cq(u8 dev_id)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	int cq_id;
	int ret;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id %d\n", dev_id);
		return NULL;
	}

	cq_id = devdrv_alloc_cq_id(dev_id);
	if (cq_id < 0) {
		NPU_DRV_ERR("alloc cq_id from dev %d failed\n", dev_id);
		return NULL;
	}
	// alloc cq mem (do it through user mmap at the stage of open device currently)
	ret = devdrv_alloc_cq_mem(dev_id, cq_id);
	if (ret != 0) {
		NPU_DRV_ERR("alloc cq mem from dev %d cq %d failed\n", dev_id, cq_id);
		return NULL;
	}

	cq_info = devdrv_calc_cq_info(dev_id, cq_id);

	return cq_info;
}

int devdrv_send_alloc_stream_mailbox(u8 cur_dev_id, int stream_id, int cq_id)
{
	struct devdrv_stream_msg *alloc_stream_msg = NULL;
	int mbx_send_result = -1;
	u32 msg_len;
	int ret;
	int cq_stream_num;

	cq_stream_num = devdrv_get_cq_ref_by_stream(cur_dev_id, cq_id);
	if (cq_stream_num == 0) {
		NPU_DRV_INFO("cq ref by steram add from zero. devid = %d, cq_id = %d\n", cur_dev_id, cq_id);
		(void)devdrv_clr_cq_info(cur_dev_id, cq_id);
	}

	// for mailbox create stream msg to ts
	(void)devdrv_inc_cq_ref_by_stream(cur_dev_id, cq_id);

	// call mailbox to info ts to create stream
	alloc_stream_msg = (struct devdrv_stream_msg *)kzalloc(sizeof(struct devdrv_stream_msg), GFP_KERNEL);
	if (alloc_stream_msg == NULL) {
		ret = -ENOMEM;
		NPU_DRV_ERR("kzalloc alloc_stream_msg failed, ret = %d\n", ret);
		goto alloc_stream_msg_failed;
	}

	(void)devdrv_create_alloc_stream_msg(cur_dev_id, stream_id, alloc_stream_msg);
	msg_len = sizeof(struct devdrv_stream_msg);
	ret = devdrv_mailbox_message_send_for_res(cur_dev_id, (u8 *) alloc_stream_msg, msg_len, &mbx_send_result);
	if (ret != 0) {
		NPU_DRV_ERR("alloc stream mailbox_message_send failed"
			" mbx_send_result = %d ret = %d\n", mbx_send_result, ret);
		goto message_send_for_res_failed;
	}

	NPU_DRV_DEBUG("alloc stream mailbox_message_send success"
		" mbx_send_result = %d ret = %d\n", mbx_send_result, ret);
	kfree(alloc_stream_msg);
	alloc_stream_msg = NULL;
	return 0;

message_send_for_res_failed:
	kfree(alloc_stream_msg);
	alloc_stream_msg = NULL;
alloc_stream_msg_failed:
	(void)devdrv_dec_cq_ref_by_stream(cur_dev_id, cq_id);
	return -1;
}

struct devdrv_stream_info* devdrv_alloc_sink_stream(u8 dev_id, u32 cq_id)
{
	u32 hwts_cq_id = cq_id;
	int hwts_sq_id;
	int stream_id;
	struct devdrv_stream_info *stream_info = NULL;
	int ret = 0;
	struct devdrv_platform_info* plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info\n");
		return NULL;
	}

	stream_id = devdrv_alloc_sink_stream_id(dev_id);
	if (stream_id < DEVDRV_MAX_NON_SINK_STREAM_ID) {
		NPU_DRV_ERR("alloc sink stream_id from dev %d failed\n", dev_id);
		return NULL;
	}

	stream_info = devdrv_calc_stream_info(dev_id, stream_id);
	if (stream_info == NULL) {
		NPU_DRV_ERR("sink stream info is null\n");
		goto calc_stream_info_failed;
	}

	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1) {
		hwts_sq_id = devdrv_alloc_hwts_sq(dev_id);
		if (hwts_sq_id < 0) {
			NPU_DRV_ERR("alloc hwts_sq from dev %d failed\n", dev_id);
			goto alloc_hwts_sq_faild;
		}

		ret = devdrv_bind_stream_with_hwts_sq(dev_id, stream_id, hwts_sq_id);
		if (ret != 0) {
			NPU_DRV_ERR("bind hwts_sq:%d to stream:%d failed\n", hwts_sq_id, stream_id);
			goto bind_stream_with_hwts_sq_failed;
		}

		ret = devdrv_bind_stream_with_hwts_cq(dev_id, stream_id, hwts_cq_id);
		if (ret != 0) {
			NPU_DRV_ERR("alloc hwts_sq_mem from dev %d failed\n", dev_id);
			goto bind_stream_with_hwts_cq_failed;
		}

		NPU_DRV_DEBUG("alloc sink-stream success stream_id:%d, hwts_sq_id = %d, hwts_cq_id = %d\n",
			stream_info->id, stream_info->sq_index, stream_info->cq_index);
	}
	return stream_info;

// --- hwts feature switch ---
bind_stream_with_hwts_cq_failed:
bind_stream_with_hwts_sq_failed:
	devdrv_free_hwts_sq(dev_id, hwts_sq_id);

alloc_hwts_sq_faild:
calc_stream_info_failed:
	devdrv_free_sink_stream_id(dev_id, stream_id);

	return NULL;
}

struct devdrv_stream_info* devdrv_alloc_non_sink_stream(u8 cur_dev_id, u32 cq_id)
{
	int ret;
	struct devdrv_stream_info *stream_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	int inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	int sq_id;
	int stream_id = devdrv_alloc_stream_id(cur_dev_id);

	COND_RETURN_ERROR(stream_id < 0 || stream_id >= DEVDRV_MAX_NON_SINK_STREAM_ID, NULL,
		"alloc stream_id from dev %d failed\n", cur_dev_id);

	stream_info = devdrv_calc_stream_info(cur_dev_id, stream_id);
	COND_GOTO_ERROR(stream_info == NULL, calc_stream_info_failed, ret, 0, "sink stream info is null\n");

	// alloc sq id
	sq_id = devdrv_alloc_sq_id(cur_dev_id);
	COND_GOTO_ERROR(sq_id < 0, sq_id_alloc_failed, ret, 0, "alloc sq_id from dev %d failed\n", cur_dev_id);

	// alloc sq physical mem (do it through user mmap at the stage of open device currently)
	ret = devdrv_alloc_sq_mem(cur_dev_id, sq_id);
	COND_GOTO_ERROR(ret != 0, sq_mem_alloc_failed, ret, ret, "alloc sq mem from dev %d failed\n", cur_dev_id);

	// bind stream with sq_id
	COND_GOTO_ERROR(devdrv_bind_stream_with_sq(cur_dev_id, stream_id, sq_id), bind_stream_with_sq_failed,
		ret, ret, "bind stream = %d with sq_id = %d from dev %d failed\n", stream_id, sq_id, cur_dev_id);

	// increment sq ref by current stream
	(void)devdrv_inc_sq_ref_by_stream(cur_dev_id, sq_id);

	// bind stream with cq_id
	COND_GOTO_ERROR(devdrv_bind_stream_with_cq(cur_dev_id, stream_id, cq_id), bind_stream_with_cq_failed,
		ret, ret, "bind stream = %d with cq_id = %d from dev %d failed\n", stream_id, cq_id, cur_dev_id);

	cur_dev_ctx = get_dev_ctx_by_id(cur_dev_id);

	COND_RETURN_ERROR(cur_dev_ctx == NULL, NULL, "cur_dev_ctx %d is null\n", cur_dev_id);

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	if (cur_dev_ctx->power_stage != DEVDRV_PM_UP) {
		NPU_DRV_INFO("alloc stream no need to inform ts\n");
		inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	} else if (cur_dev_ctx->secure_state == NPU_SEC) {
		NPU_DRV_WARN("alloc stream no need to inform ts in NPU_SEC state\n");
		inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	} else {
		NPU_DRV_INFO("alloc stream inform ts\n");
		inform_ts = DEVDRV_HAVE_TO_INFORM;
	}

	if (inform_ts == DEVDRV_HAVE_TO_INFORM) {
		ret = devdrv_send_alloc_stream_mailbox(cur_dev_id, stream_id, cq_id);
		if (ret != 0) {
			mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
			NPU_DRV_ERR("send alloc stream %d mailbox failed\n", stream_id);
			goto send_alloc_stream_mailbox;
		}
	}

	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	return stream_info;

send_alloc_stream_mailbox:
bind_stream_with_cq_failed:
	devdrv_dec_sq_ref_by_stream(cur_dev_id, sq_id);
bind_stream_with_sq_failed:
	devdrv_free_sq_id(cur_dev_id, sq_id);
sq_mem_alloc_failed:
sq_id_alloc_failed:
calc_stream_info_failed:
	devdrv_free_stream_id(cur_dev_id, stream_id);
	return NULL;
}

struct devdrv_stream_info* devdrv_alloc_stream(u32 cq_id, u32 strategy)
{
	const u8 cur_dev_id = 0;
	struct devdrv_stream_info *stream_info = NULL;

	if (cq_id >= DEVDRV_MAX_CQ_NUM) {
		NPU_DRV_ERR("illegal npu cq id %d\n", cq_id);
		return NULL;
	}

	if (strategy == STREAM_STRATEGY_SINK) {
		stream_info = devdrv_alloc_sink_stream(cur_dev_id, cq_id);
		if (stream_info != NULL)
			stream_info->strategy = strategy;

		return stream_info;
	}

	stream_info = devdrv_alloc_non_sink_stream(cur_dev_id, cq_id);
	if (stream_info != NULL)
		stream_info->strategy = strategy;

	return stream_info;
}

int devdrv_free_sink_stream(u8 dev_id, int stream_id)
{
	struct devdrv_stream_info *stream_info = NULL;
	int ret;
	struct devdrv_platform_info* plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info\n");
		return -1;
	}

	ret = devdrv_free_sink_stream_id(dev_id, stream_id);
	if (ret != 0) {
		NPU_DRV_ERR("free sink stream id:%d, dev:%d failed\n", stream_id, dev_id);
		return -1;
	}

	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1) {
		stream_info = devdrv_calc_stream_info(dev_id, stream_id);
		if (stream_info == NULL) {
			NPU_DRV_ERR("calc stream info failed, stream_id:%d, dev:%d\n", stream_id, dev_id);
			return -1;
		}
		(void)devdrv_free_hwts_sq(dev_id, stream_info->sq_index);
	}
	return 0;
}

int devdrv_free_non_sink_stream(u8 dev_id, int stream_id, u32 *sq_send_count)
{
	struct devdrv_stream_msg *free_stream_msg = NULL;
	int inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	int mbx_send_result = -1;
	u32 msg_len;
	int ret;
	struct devdrv_stream_info *stream_info = NULL;
	u32 sq_id;
	u32 cq_id ;
	struct devdrv_dev_ctx *cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	COND_RETURN_ERROR(cur_dev_ctx == NULL, -EINVAL, "cur_dev_ctx %d is null\n", dev_id);

	stream_info = devdrv_calc_stream_info(dev_id, stream_id);
	if (stream_info == NULL) {
		NPU_DRV_ERR("stream info is null\n");
		return -1;
	}

	sq_id = stream_info->sq_index;
	cq_id = stream_info->cq_index;

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	if (cur_dev_ctx->power_stage != DEVDRV_PM_UP || (cur_dev_ctx->will_powerdown == true)) {
		NPU_DRV_INFO("free stream no need to inform ts\n");
		inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	} else if (cur_dev_ctx->secure_state == NPU_SEC) {
		NPU_DRV_WARN("free stream no need to inform ts in NPU_SEC state\n");
		inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	} else {
		NPU_DRV_INFO("free stream inform ts\n");
		inform_ts = DEVDRV_HAVE_TO_INFORM;
	}

	// call mailbox to info ts to free stream
	if (inform_ts == DEVDRV_HAVE_TO_INFORM) {
		free_stream_msg = (struct devdrv_stream_msg *)kzalloc(sizeof(struct devdrv_stream_msg), GFP_KERNEL);
		COND_GOTO_ERROR(free_stream_msg == NULL, fail, ret, -1,
			"kzalloc free_stream_msg failed, will cause resource leak\n");

		(void)devdrv_create_free_stream_msg(dev_id, stream_id, free_stream_msg);
		msg_len = sizeof(struct devdrv_stream_msg);

		ret = devdrv_mailbox_message_send_for_res(dev_id, (u8 *)free_stream_msg,
			msg_len, &mbx_send_result);
		kfree(free_stream_msg);
		free_stream_msg = NULL;
		COND_GOTO_ERROR(ret != 0, fail, ret, -1, "free stream mailbox_message_send failed will cause "
			"resource leak mbx_send_result = %d ret = %d\n", mbx_send_result, ret);

		NPU_DRV_DEBUG("free stream mailbox_message_send success"
			" mbx_send_result = %d ret = %d\n", mbx_send_result, ret);
	}
	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	// dec ref of cq used by cur stream
	devdrv_dec_cq_ref_by_stream(dev_id, cq_id);
	// dec ref of sq used by cur stream
	devdrv_dec_sq_ref_by_stream(dev_id, sq_id);

	*sq_send_count = 0;  // to make sure upper layer get right data when no free sq
	if (devdrv_is_sq_ref_by_no_stream(dev_id, sq_id)) {
		devdrv_get_sq_send_count(dev_id, sq_id, sq_send_count);
		NPU_DRV_DEBUG("prepare free dev %d sq %d total sq_send_count=%d\n", dev_id, sq_id, *sq_send_count);
		devdrv_free_sq_id(dev_id, sq_id);
		devdrv_free_sq_mem(dev_id, sq_id);
	}
	// add stream_info to dev_ctx ->stream_available_list
	return devdrv_free_stream_id(dev_id, stream_id);
fail:
	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
	return ret;
}

int devdrv_free_stream(u8 dev_id, u32 stream_id, u32 *sq_send_count)
{
	struct devdrv_stream_info *stream_info = NULL;
	struct devdrv_platform_info *plat_info = NULL;
	struct devdrv_dev_ctx* cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id\n");
		return -1;
	}

	if (stream_id >= DEVDRV_MAX_STREAM_ID) {
		NPU_DRV_ERR("illegal npu dev id\n");
		return -1;
	}

	if (sq_send_count == NULL) {
		NPU_DRV_ERR("sq_send_count ptr is null\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -EINVAL;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("get plat_ops failed\n");
		return -EFAULT;
	}

	stream_info = devdrv_calc_stream_info(dev_id, stream_id);
	if (stream_info->strategy == STREAM_STRATEGY_SINK) {
		NPU_DRV_DEBUG("free sink stream\n");
		return devdrv_free_sink_stream(dev_id, stream_id);
	} else {
		return devdrv_free_non_sink_stream(dev_id, stream_id, sq_send_count);
	}

	return 0;
}
