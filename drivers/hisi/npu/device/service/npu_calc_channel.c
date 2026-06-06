/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

#include "npu_stream.h"
#include "npu_sink_stream.h"
#include "npu_calc_channel.h"
#include "npu_calc_sq.h"
#include "npu_calc_cq.h"
#include "npu_mailbox_msg.h"
#include "npu_mailbox.h"
#include "npu_shm.h"
#include "drv_log.h"
#include "npu_pm.h"
#include "npu_doorbell.h"

struct devdrv_ts_cq_info *devdrv_alloc_cq(u8 dev_id)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	int cq_id = -1;
	int ret = -1;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return NULL;
	}

	cq_id = devdrv_alloc_cq_id(dev_id);
	if (cq_id < 0) {
		devdrv_drv_err("alloc cq_id from dev %d failed\n", dev_id);
		return NULL;
	}
	// alloc cq mem (do it through user mmap at the stage of open device currently)
	ret = devdrv_alloc_cq_mem(dev_id, cq_id);
	if (ret != 0) {
		devdrv_drv_err("alloc cq mem from dev %d cq %d failed\n",
			       dev_id, cq_id);
		return NULL;
	}

	cq_info = devdrv_calc_cq_info(dev_id, cq_id);

	return cq_info;
}

int devdrv_send_alloc_stream_mailbox(u8 cur_dev_id, int stream_id, int cq_id)
{
	struct devdrv_stream_msg *alloc_stream_msg = NULL;
	int mbx_send_result = -1;
	u32 msg_len = 0;
	int ret = 0;
	int cq_stream_num = 0;

	cq_stream_num = devdrv_get_cq_ref_by_stream(cur_dev_id, cq_id);
	if (cq_stream_num == 0) {
		devdrv_drv_info("cq ref by steram add from zero. devid = %d, cq_id = %d\n", cur_dev_id, cq_id);
		(void)devdrv_clr_cq_info(cur_dev_id, cq_id);
	}

	// for mailbox create stream msg to ts
	(void)devdrv_inc_cq_ref_by_stream(cur_dev_id, cq_id);

	// call mailbox to info ts to create stream
	alloc_stream_msg =
	    (struct devdrv_stream_msg *)
	    kzalloc(sizeof(struct devdrv_stream_msg), GFP_KERNEL);
	if (alloc_stream_msg == NULL) {
		ret = -ENOMEM;
		devdrv_drv_err("kzalloc alloc_stream_msg failed, ret = %d.\n", ret);
		goto alloc_stream_msg_failed;
	}

	(void)devdrv_create_alloc_stream_msg(cur_dev_id, stream_id,
					     alloc_stream_msg);
	msg_len = sizeof(struct devdrv_stream_msg);

	ret = devdrv_mailbox_message_send_for_res(cur_dev_id,
						  (u8 *) alloc_stream_msg,
						  msg_len,
						  &mbx_send_result);
	if (ret != 0) {
		devdrv_drv_err("alloc stream mailbox_message_send failed"
			       " mbx_send_result = %d ret = %d\n",
			       mbx_send_result, ret);
		goto message_send_for_res_failed;
	}

	devdrv_drv_debug("alloc stream mailbox_message_send success"
		" mbx_send_result = %d ret = %d\n", mbx_send_result,
		ret);
	kfree(alloc_stream_msg);
	alloc_stream_msg = NULL;
	return 0;

message_send_for_res_failed:
	kfree(alloc_stream_msg);
	alloc_stream_msg = NULL;
alloc_stream_msg_failed:
	(void)devdrv_dec_cq_ref_by_stream(cur_dev_id,cq_id);
	return -1;
}

struct devdrv_stream_info* devdrv_alloc_sink_stream(u8 cur_dev_id)
{
	int stream_id = -1;
	struct devdrv_stream_info *stream_info = NULL;

	stream_id = devdrv_alloc_sink_stream_id(cur_dev_id);
	if (stream_id < DEVDRV_MAX_NON_SINK_STREAM_ID) {
		devdrv_drv_err("alloc sink stream_id from dev %d failed\n",
				   cur_dev_id);
		return NULL;
	}

	stream_info = devdrv_calc_stream_info(cur_dev_id, stream_id);
	if (stream_info == NULL) {
		devdrv_drv_err("sink stream info is null.\n");
		devdrv_free_sink_stream_id(cur_dev_id,stream_id);
		return NULL;
	}

	return stream_info;
}

struct devdrv_stream_info* devdrv_alloc_non_sink_stream(u8 cur_dev_id, u32 cq_id)
{
	int ret = 0;
	int sq_id = -1;
	int stream_id = -1;
	struct devdrv_stream_info *stream_info = NULL;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	int inform_ts = DEVDRV_NO_NEED_TO_INFORM;

	stream_id = devdrv_alloc_stream_id(cur_dev_id);
	if (stream_id < 0 || stream_id >= DEVDRV_MAX_NON_SINK_STREAM_ID) {
		devdrv_drv_err("alloc stream_id from dev %d failed\n",
				   cur_dev_id);
		return NULL;
	}

	stream_info = devdrv_calc_stream_info(cur_dev_id, stream_id);
	if (stream_info == NULL) {
		devdrv_drv_err("sink stream info is null.\n");
		goto calc_stream_info_failed;
	}

	//alloc sq id
	sq_id = devdrv_alloc_sq_id(cur_dev_id);
	if (sq_id < 0) {
		devdrv_drv_err("alloc sq_id from dev %d failed\n", cur_dev_id);
		goto sq_id_alloc_failed;
	}
	// alloc sq physical mem (do it through user mmap at the stage of open device currently)
	ret = devdrv_alloc_sq_mem(cur_dev_id, sq_id);
	if (ret != 0) {
		devdrv_drv_err("alloc sq mem from dev %d failed\n", cur_dev_id);
		goto sq_mem_alloc_failed;
	}
	// bind stream with sq_id
	if (devdrv_bind_stream_with_sq(cur_dev_id, stream_id, sq_id)) {
		devdrv_drv_err
		    ("bind stream = %d with sq_id = %d from dev %d failed\n",
		     stream_id, sq_id, cur_dev_id);
		goto bind_stream_with_sq_failed;
	}
	// increment sq ref by current stream
	(void)devdrv_inc_sq_ref_by_stream(cur_dev_id, sq_id);

	// bind stream with cq_id
	if (devdrv_bind_stream_with_cq(cur_dev_id, stream_id, cq_id)) {
		devdrv_drv_err
		    ("bind stream = %d with cq_id = %d from dev %d failed\n",
		     stream_id, cq_id, cur_dev_id);
		goto bind_stream_with_cq_failed;
	}

	cur_dev_ctx = get_dev_ctx_by_id(cur_dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n", cur_dev_id);
		return NULL;
	}

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	if (cur_dev_ctx->power_stage != DEVDRV_PM_UP) {
		devdrv_drv_info("free stream no need to inform ts.\n");
		inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	}
	else{
		devdrv_drv_info("free stream inform ts.\n");
		inform_ts = DEVDRV_HAVE_TO_INFORM;
	}

	if (inform_ts == DEVDRV_HAVE_TO_INFORM) {
		ret = devdrv_send_alloc_stream_mailbox(cur_dev_id, stream_id, cq_id);
		if (ret != 0) {
			mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
			devdrv_drv_err("send alloc stream %d mailbox failed.\n", stream_id);
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
	devdrv_free_stream_id(cur_dev_id,stream_id);
	return NULL;
}

struct devdrv_stream_info* devdrv_alloc_stream(u32 cq_id, u32 strategy)
{
	const u8 cur_dev_id = 0;
	struct devdrv_stream_info *stream_info = NULL;

	if (cq_id >= DEVDRV_MAX_CQ_NUM) {
		devdrv_drv_err("illegal npu cq id %d\n", cq_id);
		return NULL;
	}

	if (strategy == STREAM_STRATEGY_SINK) {
		stream_info = devdrv_alloc_sink_stream(cur_dev_id);
		if (stream_info != NULL) {
			stream_info->strategy = strategy;
		}
		return stream_info;
	}

	stream_info = devdrv_alloc_non_sink_stream(cur_dev_id, cq_id);
	if (stream_info != NULL) {
		stream_info->strategy = strategy;
	}

	return stream_info;
}

int devdrv_free_stream(u8 dev_id, u32 stream_id, u32 *sq_send_count)
{
	struct devdrv_stream_info *stream_info = NULL;
	struct devdrv_stream_msg *free_stream_msg = NULL;
	struct devdrv_platform_info *plat_info = NULL;
	struct devdrv_dev_ctx* cur_dev_ctx = NULL;
	int inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	const u8 cur_dev_id = 0;	// get from platform
	int mbx_send_result = -1;
	u32 msg_len = 0;
	u32 sq_id;
	u32 cq_id;
	int ret = -1;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (stream_id >= DEVDRV_MAX_STREAM_ID) {
		devdrv_drv_err("illegal npu dev id\n");
		return -1;
	}

	if (sq_send_count == NULL) {
		devdrv_drv_err("sq_send_count ptr is null\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n",dev_id);
		return -EINVAL;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("get plat_ops failed.\n");
		return -EFAULT;
	}

	stream_info = devdrv_calc_stream_info(dev_id, stream_id);

	if (stream_info->strategy == STREAM_STRATEGY_SINK)
	{
		devdrv_drv_debug("free sink stream success.\n");
		return devdrv_free_sink_stream_id(dev_id, stream_id);
	}

	sq_id = stream_info->sq_index;
	cq_id = stream_info->cq_index;

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	if (cur_dev_ctx->power_stage != DEVDRV_PM_UP) {
		devdrv_drv_info("free stream no need to inform ts.\n");
		inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	}
	else{
		devdrv_drv_info("free stream inform ts.\n");
		inform_ts = DEVDRV_HAVE_TO_INFORM;
	}

	// call mailbox to info ts to free stream
	if (inform_ts == DEVDRV_HAVE_TO_INFORM) {
		free_stream_msg = (struct devdrv_stream_msg *)kzalloc
		    (sizeof(struct devdrv_stream_msg), GFP_KERNEL);
		if (free_stream_msg == NULL) {
			mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
			devdrv_drv_err("kzalloc free_stream_msg failed,"
				       " will cause resource leak\n");
			return -1;
		}

		(void)devdrv_create_free_stream_msg(cur_dev_id, stream_id,
						    free_stream_msg);
		msg_len = sizeof(struct devdrv_stream_msg);

		ret = devdrv_mailbox_message_send_for_res(cur_dev_id,
							  (u8 *)free_stream_msg,
							  msg_len,
							  &mbx_send_result);
		kfree(free_stream_msg);
		free_stream_msg = NULL;
		if (ret != 0) {
			mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
			devdrv_drv_err("free stream mailbox_message_send failed"
				       " will cause resource leak"
				       " mbx_send_result = %d ret = %d\n",
				       mbx_send_result, ret);
			return -1;
		}

		devdrv_drv_debug("free stream mailbox_message_send success"
				 " mbx_send_result = %d ret = %d\n",
				 mbx_send_result, ret);

	}

	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	// dec ref of cq used by cur stream
	devdrv_dec_cq_ref_by_stream(dev_id, cq_id);

	// dec ref of sq used by cur stream
	devdrv_dec_sq_ref_by_stream(dev_id, sq_id);

	*sq_send_count = 0;	// to make sure upper layer get right data when no free sq
	if (devdrv_is_sq_ref_by_no_stream(dev_id, sq_id)) {
		devdrv_get_sq_send_count(dev_id, sq_id, sq_send_count);
		devdrv_drv_debug
		    ("prepare free dev %d sq %d total sq_send_count = %d\n",
		     dev_id, sq_id, *sq_send_count);
		devdrv_free_sq_id(dev_id, sq_id);
		devdrv_free_sq_mem(dev_id, sq_id);
	}
	// add stream_info to dev_ctx ->stream_available_list
	return devdrv_free_stream_id(dev_id, stream_id);
}
