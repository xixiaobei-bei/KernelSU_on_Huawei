/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Implement of swing driver
 * Author: lidenghui
 * Create: 2019-03-26
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#include <media/huawei/camera.h>
#include <media/v4l2-event.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <securec.h>
#include "inputhub_api.h"
#include "common.h"
#include "shmem.h"
#include "swing_cam_dev.h"
#include "hwcam_intf.h"

static struct swingcam_priv g_swing_priv = {0};
static struct miniisp_t g_miniIsp = {
	.name = "miniisp",
};

enum timestamp_state_t {
	TIMESTAMP_UNINITIAL = 0,
	TIMESTAMP_INITIAL,
};

static enum timestamp_state_t s_timestamp_state;
static struct timeval s_timeval;
static struct timespec64 s_time_spec;

static void swing_cam_init_timestamp(void)
{
	s_timestamp_state = TIMESTAMP_INITIAL;
	get_monotonic_boottime64(&s_time_spec);
	do_gettimeofday(&s_timeval);
}

static void swing_cam_destroy_timestamp(void)
{
	s_timestamp_state = TIMESTAMP_UNINITIAL;
	(void)memset_s(&s_timeval, sizeof(s_timeval), 0x00, sizeof(s_timeval));
	(void)memset_s(&s_time_spec, sizeof(s_time_spec), 0x00, sizeof(s_time_spec));
}

static void swing_cam_set_timestamp(u32 *timestamp_high, u32 *timestamp_low)
{
#define MICROSECOND_PER_SECOND 1000000
#define NANOSECOND_PER_MICROSECOND 1000
	u64 fw_micro_second;
	u64 fw_sys_counter;
	u64 micro_second;

	if (s_timestamp_state == TIMESTAMP_UNINITIAL) {
		swing_cam_log_err("%s wouldn't enter this branch",
			__FUNCTION__);
		swing_cam_init_timestamp();
	}

	if (!timestamp_high || !timestamp_low) {
		swing_cam_log_err("%s timestamp_high or timestamp_low is null",
			__FUNCTION__);
		return;
	}

	if (*timestamp_high == 0 && *timestamp_low == 0)
		return;
	/* 32:offset */
	fw_sys_counter = ((u64)(*timestamp_high) << 32) | (u64)(*timestamp_low);
	micro_second = (fw_sys_counter / NANOSECOND_PER_MICROSECOND) -
		s_time_spec.tv_sec * MICROSECOND_PER_SECOND -
		s_time_spec.tv_nsec / NANOSECOND_PER_MICROSECOND;

	fw_micro_second =
		(micro_second / MICROSECOND_PER_SECOND + s_timeval.tv_sec) *
		MICROSECOND_PER_SECOND +
		((micro_second % MICROSECOND_PER_SECOND) + s_timeval.tv_usec);

	/* 32:offset */
	*timestamp_high = (u32)((fw_micro_second >> 32) & 0xFFFFFFFF);
	*timestamp_low = (u32)(fw_micro_second & 0xFFFFFFFF);
}

static int swing_cam_put_data_to_read_kfifo(unsigned char *buf,
	unsigned int len)
{
	int ret;
	unsigned int length = sizeof(struct swingcam_data);
	struct swingcam_data read_data = {0};

	mutex_lock(&g_swing_priv.swing_mutex);
	if (g_swing_priv.ref_cnt == 0) {
		swing_cam_log_info("%s: ref cnt is 0", __FUNCTION__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -1;
	}

	if (kfifo_avail(&g_swing_priv.read_kfifo) < length) {
		swing_cam_log_err("%s read_kfifo is full, drop upload data",
			__FUNCTION__);
		goto ret_unlock;
	}

	read_data.recv_len = len;
	read_data.p_recv = kzalloc(read_data.recv_len, GFP_ATOMIC);
	if (!read_data.p_recv) {
		swing_cam_log_err("Failed to alloc memory to save resp...");
		goto ret_unlock;
	}

	ret = memcpy_s(read_data.p_recv, read_data.recv_len,
		buf, read_data.recv_len);
	if (ret != 0) {
		swing_cam_log_err("%s memcpy_s failed...", __FUNCTION__);
		goto ret_unlock;
	}

	length = kfifo_in(&g_swing_priv.read_kfifo, (unsigned char *)&read_data,
		sizeof(struct swingcam_data));
	if (length == 0) {
		swing_cam_log_err("%s: kfifo_in failed", __FUNCTION__);
		goto ret_unlock;
	}
	mutex_unlock(&g_swing_priv.swing_mutex);

	if (!g_miniIsp.subdev.devnode) {
		swing_cam_log_err("%s vdec is null", __FUNCTION__);
		goto ret_err;
	}
	return 0;

ret_unlock:
	mutex_unlock(&g_swing_priv.swing_mutex);

ret_err:
	if (read_data.p_recv)
		kfree(read_data.p_recv);

	return -EFAULT;
}

static int swing_cam_enqueue(unsigned char *buf,
	unsigned int len, enum miniisp_event_kind cur_kind)
{
	int ret;
	struct v4l2_event ev;
	struct video_device *vdev = NULL;
	struct miniisp_event *req = NULL;

	ret = swing_cam_put_data_to_read_kfifo(buf, len);
	if (ret != 0)
		return ret;

	req = (struct miniisp_event *)ev.u.data;
	(void)memset_s(&ev, sizeof(struct v4l2_event), 0,
		sizeof(struct v4l2_event));

	vdev = g_miniIsp.subdev.devnode;
	ev.type = MINIISP_V4L2_EVENT_TYPE;
	ev.id = MINIISP_HIGH_PRIO_EVENT;
	req->kind = cur_kind;

	v4l2_event_queue(vdev, &ev);

	return 0;
}

static int swing_cam_execute_subcmd(struct pkt_subcmd_resp *p_resp,
	struct miniisp_msg_t *p_msg_ack)
{
	int ret = 0;
	switch (p_resp->subcmd) {
	case SUB_CMD_SWING_CAM_CONFIG:
		swing_cam_log_info("Config Ack Received");
		break;
	case SUB_CMD_SWING_CAM_CAPTURE:
		swing_cam_log_info("Capture Ack Received");
		swing_cam_set_timestamp(
			&(p_msg_ack->u.ack_miniisp_request.timestamp_high),
			&(p_msg_ack->u.ack_miniisp_request.timestamp_low));
		break;
	case SUB_CMD_SWING_CAM_MATCH_ID:
		swing_cam_log_info("Received, match id, status:%d",
			p_msg_ack->u.ack_query_swing_cam.status);
		break;
	case SUB_CMD_SWING_CAM_ACQUIRE:
		swing_cam_log_info("Received, csi_index:%u init_ret:%d",
			p_msg_ack->u.ack_acquire_swing_cam.csi_index,
			p_msg_ack->u.ack_acquire_swing_cam.init_ret);
		break;
	case SUB_CMD_SWING_CAM_EXTEND:
		swing_cam_log_info("%s Received, extend_cmd:%d", __FUNCTION__,
			p_msg_ack->u.ack_extend_swing_set.extend_cmd);
		break;
	case SUB_CMD_SWING_GET_OTP:
		swing_cam_log_info("%s Received, get_otp status:%d",
			__FUNCTION__, p_msg_ack->u.ack_get_ao_otp.status);
		break;
	default:
		swing_cam_log_err("unhandled cmd: tag[%d], sub_cmd[%d]",
			p_resp->hd.tag, p_resp->subcmd);
		ret = -EFAULT;
		break;
	}
	return ret;
}

static int swing_cam_get_resp(const struct pkt_header *head)
{
	struct pkt_subcmd_resp *p_resp = NULL;
	int ret;
	struct miniisp_msg_t *p_msg_ack = NULL;

	p_resp = (struct pkt_subcmd_resp *)(head);

	if (!p_resp)
		return -EFAULT;

	if (p_resp->hd.tag != TAG_SWING_CAM)
		return -EFAULT;

	p_msg_ack = (struct miniisp_msg_t *)(p_resp + 1);

	swing_cam_log_info("%s cmd[%d], subcmd[%d] length[%d]", __FUNCTION__,
		p_resp->hd.cmd, p_resp->subcmd, p_resp->hd.length);

	swing_cam_log_info("%s tag[%d], errno[%d], tranid[%d]", __FUNCTION__,
		p_resp->hd.tag, p_resp->hd.errno, p_resp->hd.tranid);

	swing_cam_log_info("%s size[%d], name[%d] id[%d]", __FUNCTION__,
		p_msg_ack->message_size, p_msg_ack->api_name,
		p_msg_ack->message_id);

	if (p_resp->hd.cmd == CMD_CMN_CLOSE_RESP) {
		swing_cam_log_info("Close Ack Received");
		ret = swing_cam_enqueue((unsigned char *)p_msg_ack,
			sizeof(struct miniisp_msg_t), MINIISP_MSG_CB);
		return ret;
	}

	ret = swing_cam_execute_subcmd(p_resp, p_msg_ack);
	if (ret != 0)
		return ret;

	ret = swing_cam_enqueue((unsigned char *)p_msg_ack,
		sizeof(struct miniisp_msg_t), MINIISP_MSG_CB);

	return ret;
}

static void swing_cam_clear_fifo()
{
	int i = 0;
	u32 length = sizeof(struct swingcam_data);
	unsigned int ret;
	struct swingcam_data read_data = {0};

	while (i < SWING_READ_CACHE_COUNT) {
		if (kfifo_len(&g_swing_priv.read_kfifo) < length) {
			swing_cam_log_err("%s: kfifo len is less than swing data i=%d",
				__FUNCTION__, i);
			break;
		}

		ret = kfifo_out(&g_swing_priv.read_kfifo,
				(unsigned char *)&read_data, length);
		if (ret == 0) {
			swing_cam_log_err("%s: kfifo out failed i=%d",
				__FUNCTION__, i);
			break;
		}

		if (read_data.p_recv) {
			kfree(read_data.p_recv);
			read_data.p_recv = NULL;
			read_data.recv_len = 0;
		}
		i++;
	}
	kfifo_free(&g_swing_priv.read_kfifo);
}

static int swing_cam_dev_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	swing_cam_log_info("enter %s...", __FUNCTION__);

	(void)sd;
	(void)fh;
	mutex_lock(&g_swing_priv.swing_mutex);

	if (g_swing_priv.ref_cnt == 0) {
		if (kfifo_alloc(&g_swing_priv.read_kfifo,
			sizeof(struct swingcam_data) * SWING_READ_CACHE_COUNT,
			GFP_KERNEL)) {
			swing_cam_log_err("%s kfifo alloc failed",
				__FUNCTION__);
			mutex_unlock(&g_swing_priv.swing_mutex);
			return -ENOMEM;
		}
	}
	g_swing_priv.ref_cnt++;
	mutex_unlock(&g_swing_priv.swing_mutex);

	return 0;
}

static int swing_cam_dev_release(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	int rc = 0;
	swing_cam_log_info("enter %s...", __FUNCTION__);

	(void)sd;
	(void)fh;
	mutex_lock(&g_swing_priv.swing_mutex);
	if (g_swing_priv.ref_cnt == 0) {
		swing_cam_log_err("%s: ref cnt is 0", __FUNCTION__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -EFAULT;
	}

	g_swing_priv.ref_cnt--;

	if (g_swing_priv.ref_cnt == 0) {
		if (send_cmd_from_kernel(TAG_SWING_CAM, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0) != 0) {
			swing_cam_log_err("[%s]send cmd error", __FUNCTION__);
			rc = -EFAULT;
		} else {
			swing_cam_log_info("enter %s...", __FUNCTION__);
		}
		swing_cam_clear_fifo();
		swing_cam_log_info("finish");
	}

	mutex_unlock(&g_swing_priv.swing_mutex);

	return rc;
}

static int miniisp_reset_notify(void)
{
	int ret;
	struct miniisp_msg_t msg_ack = {0};

	msg_ack.u.event_sent.event_id = EVENT_MINI_RECOVER_CODE;
	msg_ack.api_name = MINI_EVENT_SENT;
	ret = swing_cam_enqueue((unsigned char *)(&msg_ack),
		sizeof(struct miniisp_msg_t), MINIISP_MSG_CB);
	swing_cam_log_info("enter [%s]", __FUNCTION__);
	return ret;
}

static int miniisp_reset_notifier(struct notifier_block *nb,
	unsigned long action, void *data)
{
	int ret = 0;

	switch (action) {
	case IOM3_RECOVERY_IDLE:
		ret = miniisp_reset_notify();
		break;
	default:
		break;
	}

	return ret;
}

static struct notifier_block g_miniisp_reboot_notify = {
	.notifier_call = miniisp_reset_notifier,
	.priority = -1,
};

static int __init swing_cam_dev_init(struct platform_device *pdev)
{
	int ret;

	swing_cam_log_info("%s enter", __FUNCTION__);

	if (is_sensorhub_disabled() != 0) {
		swing_cam_log_err("sensorhub disabled...");
		return -EFAULT;
	}

	mutex_init(&g_swing_priv.swing_mutex);

	ret = register_mcu_event_notifier(TAG_SWING_CAM,
		CMD_CMN_CONFIG_RESP, swing_cam_get_resp);
	if (ret != 0) {
		swing_cam_log_err("reg notifier failed. [%d]", ret);
		return -EFAULT;
	}
	ret = register_mcu_event_notifier(TAG_SWING_CAM,
		CMD_CMN_CLOSE_RESP, swing_cam_get_resp);
	if (ret != 0) {
		swing_cam_log_err("reg notifier failed. [%d]", ret);
		return -EFAULT;
	}
	ret = register_iom3_recovery_notifier(&g_miniisp_reboot_notify);
	if (ret < 0) {
		swing_cam_log_err("register_iom3_recovery_notifier fail");
		return ret;
	}
	g_swing_priv.ref_cnt = 0;
	g_swing_priv.self = &(pdev->dev);

	swing_cam_log_info("%s exit, success", __FUNCTION__);
	return 0;
}

static void __exit miniisp_dev_exit(void)
{
	unregister_mcu_event_notifier(TAG_SWING_CAM,
		CMD_CMN_CLOSE_RESP, swing_cam_get_resp);
	unregister_mcu_event_notifier(TAG_SWING_CAM,
		CMD_CMN_CONFIG_RESP, swing_cam_get_resp);
}

static int miniisp_recv_rpmsg(void *arg)
{
	struct miniisp_msg_t *resp_msg = (struct miniisp_msg_t *)arg;

	struct swingcam_data read_data = {0};
	u32 length = sizeof(struct swingcam_data);
	unsigned int ret;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __FUNCTION__);
		return -1;
	}

	mutex_lock(&g_swing_priv.swing_mutex);
	if (g_swing_priv.ref_cnt == 0) {
		swing_cam_log_info("%s: ref cnt is 0", __FUNCTION__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -1;
	}

	if (kfifo_len(&g_swing_priv.read_kfifo) < length) {
		swing_cam_log_err("%s: read data failed", __FUNCTION__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -1;
	}

	ret = kfifo_out(&g_swing_priv.read_kfifo,
		(unsigned char *)&read_data, length);
	mutex_unlock(&g_swing_priv.swing_mutex);
	if (ret == 0) {
		swing_cam_log_err("%s: kfifo out failed", __FUNCTION__);
		return -1;
	}

	if (read_data.recv_len == sizeof(struct miniisp_msg_t))
		(void)memcpy_s(resp_msg, sizeof(struct miniisp_msg_t),
			read_data.p_recv, sizeof(struct miniisp_msg_t));
	else
		swing_cam_log_err("[%s] copy fail len[0x%x]",
			__FUNCTION__, read_data.recv_len);

	if (read_data.p_recv) {
		kfree(read_data.p_recv);
		read_data.p_recv = NULL;
		read_data.recv_len = 0;
	}

	return (int)ret;
}

static int swing_cam_acquire_check(struct msg_req_acquire_swing_cam *acq)
{
	if (acq->sensor_spec.sensor_mode >= SENSOR_WORK_MODE_MAX)
		return -EFAULT;

	if (acq->sensor_spec.test_pattern >= SENSOR_TEST_PATTERN_MAX)
		return -EFAULT;

	return 0;
}

static int swing_v4l2_ioctl_cam_acquire(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_acquire_swing_cam *acp_req = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __FUNCTION__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_ACQUIRE_SWING_CAM) {
		swing_cam_log_err("[%s] api_name err", __FUNCTION__);
		return -EFAULT;
	}
	swing_cam_log_info("%s api_name:0x%x id:0x%x size:0x%x",
		__FUNCTION__, mini_req->api_name,
		mini_req->message_id, mini_req->message_size);

	acp_req = &(mini_req->u.req_acquire_swing_cam);

	(acp_req->sensor_name)[sizeof(acp_req->sensor_name) - 1] = '\0';
	(acp_req->i2c_bus_type)[sizeof(acp_req->i2c_bus_type) - 1] = '\0';
	swing_cam_log_info("i2c_index:%d, i2c_bus_type:%s",
		acp_req->i2c_index, acp_req->i2c_bus_type);

	swing_cam_log_info("sensor_mode:%d, test_patt:%d is_master:%d",
		acp_req->sensor_spec.sensor_mode,
		acp_req->sensor_spec.test_pattern,
		acp_req->sensor_spec.phy_info.is_master_sensor);

	swing_cam_log_info("phy_id:%d,phy_mode:%d, phy_work_mode:%d",
		acp_req->sensor_spec.phy_info.phy_id,
		acp_req->sensor_spec.phy_info.phy_mode,
		acp_req->sensor_spec.phy_info.phy_work_mode);

	if (swing_cam_acquire_check(acp_req)) {
		swing_cam_log_err("[%s] acquire check fail", __FUNCTION__);
		return -EFAULT;
	}
	if (send_cmd_from_kernel(TAG_SWING_CAM, CMD_CMN_CONFIG_REQ,
		SUB_CMD_SWING_CAM_ACQUIRE,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s]send cmd error", __FUNCTION__);
		return -EFAULT;
	}

	return 0;
}

static int swing_cam_config_check(struct msg_req_config_swing_cam *cfg)
{
	if (cfg->extension > FULL_SIZE)
		return -EFAULT;

	if (cfg->flow_mode > SWING_CONTINUOUS)
		return -EFAULT;

	if (cfg->process_pattern >= SENSOR_PIXEL_MAX)
		return -EFAULT;

	if (cfg->stream_cfg.format > SWING_PIXEL_FMT_YUV422I)
		return -EFAULT;

	return 0;
}

static int swing_v4l2_ioctl_cam_config(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_config_swing_cam *cfgReq = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __FUNCTION__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_USECASE_CONFIG_SWING_CAM) {
		swing_cam_log_err("[%s] api_name err", __FUNCTION__);
		return -EFAULT;
	}
	swing_cam_log_info("api_name:0x%x message_id:0x%x message_size:0x%x",
		mini_req->api_name, mini_req->message_id, mini_req->message_size);

	cfgReq = &(mini_req->u.req_usecase_cfg_swing_cam);
	swing_cam_log_info("extension:%d, flow_mode:%d is_secure:%d %d",
		cfgReq->extension, cfgReq->flow_mode,
		cfgReq->is_secure[STREAM_MINIISP_OUT_PREVIEW],
		cfgReq->is_secure[STREAM_MINIISP_OUT_META]);

	swing_cam_log_info("%s pattern:%d w:%d, h:%d, stride:%d format:%d",
		__FUNCTION__, cfgReq->process_pattern,
		cfgReq->stream_cfg.width, cfgReq->stream_cfg.height,
		cfgReq->stream_cfg.stride, cfgReq->stream_cfg.format);

	if (swing_cam_config_check(cfgReq)) {
		swing_cam_log_err("[%s] cfg check fail", __FUNCTION__);
		return -EFAULT;
	}
	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_CAM_CONFIG,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s]send cmd error", __FUNCTION__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_cam_match_id(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_query_swing_cam *matchReq = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __FUNCTION__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_QUERY_SWING_CAM) {
		swing_cam_log_err("[%s] api_name err", __FUNCTION__);
		return -EFAULT;
	}
	swing_cam_log_info("%s api_name:0x%x id:0x%x size:0x%x", __FUNCTION__,
		mini_req->api_name, mini_req->message_id, mini_req->message_size);

	matchReq = &(mini_req->u.req_query_swing_cam);

	(matchReq->product_name)[sizeof(matchReq->product_name) - 1] = '\0';
	(matchReq->name)[sizeof(matchReq->name) - 1] = '\0';
	swing_cam_log_info("pos:%d, i2c:%d, csi:%d",
		matchReq->mount_position, matchReq->i2c_index,
		matchReq->csi_index);

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_CAM_MATCH_ID,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s]send cmd error", __FUNCTION__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_cam_capture(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct miniisp_req_request *capReq = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __FUNCTION__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_MINIISP_REQUEST) {
		swing_cam_log_err("[%s] api_name err", __FUNCTION__);
		return -EFAULT;
	}
	swing_cam_log_info("%s api_name:0x%x id:0x%x size:0x%x",
		__FUNCTION__, mini_req->api_name,
		mini_req->message_id, mini_req->message_size);

	capReq = &(mini_req->u.req_miniisp_request);
	swing_cam_log_info("map_fd:%d, frame:%d",
		capReq->map_fd, capReq->frame_num);

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_CAM_CAPTURE,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s]send cmd error", __FUNCTION__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_cam_extend(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_extend_swing_set *wxt_req = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __FUNCTION__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_MINIISP_EXTEND_SET) {
		swing_cam_log_err("[%s] api_name err", __FUNCTION__);
		return -EFAULT;
	}
	swing_cam_log_info("%s api_name:0x%x id:0x%x size:0x%x",
		__FUNCTION__, mini_req->api_name,
		mini_req->message_id, mini_req->message_size);

	wxt_req = &(mini_req->u.req_subcmd_swing_cam);
	swing_cam_log_info("%s extend_cmd=%d", __FUNCTION__,
		wxt_req->extend_cmd);

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_CAM_EXTEND,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s]send cmd error", __FUNCTION__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_get_otp(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_get_ao_otp_t *otpReq = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __FUNCTION__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_GET_SWING_CAM_OTP) {
		swing_cam_log_err("[%s] api_name err", __FUNCTION__);
		return -EFAULT;
	}
	swing_cam_log_info("%s api_name:0x%x id:0x%x size:0x%x",
		__FUNCTION__, mini_req->api_name,
		mini_req->message_id, mini_req->message_size);

	otpReq = &(mini_req->u.req_get_ao_otp);
	(otpReq->sensor_name)[sizeof(otpReq->sensor_name) - 1] = '\0';

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_GET_OTP,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s]send cmd error", __FUNCTION__);
		return -EFAULT;
	}

	return 0;
}

static long miniisp_vo_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	long ret = 0;

	swing_cam_log_info("%s cmd is (0x%x), arg(%pK)",
		__FUNCTION__, cmd, arg);

	if ((arg) && ((sizeof(struct miniisp_msg_t) > MAX_SWING_PAYLOAD)))
		return -EFAULT;

	switch (cmd) {
	case SWING_IOCTL_CAM_OPEN:
		send_cmd_from_kernel(TAG_SWING_CAM,
			CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0);
		swing_cam_init_timestamp();
		break;

	case SWING_IOCTL_CAM_CLOSE:
		swing_cam_destroy_timestamp();
		send_cmd_from_kernel(TAG_SWING_CAM,
			CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
		break;

	case SWING_IOCTL_CAM_CONFIG:
		ret = swing_v4l2_ioctl_cam_config(arg);
		break;

	case SWING_IOCTL_CAM_CAPTURE:
		ret = swing_v4l2_ioctl_cam_capture(arg);
		break;

	case SWING_IOCTL_CAM_MATCH_ID:
		ret = swing_v4l2_ioctl_cam_match_id(arg);
		break;

	case SWING_IOCTL_CAM_RECV_RPMSG:
		ret = miniisp_recv_rpmsg(arg);
		break;

	case SWING_IOCTL_CAM_ACQUIRE:
		ret = swing_v4l2_ioctl_cam_acquire(arg);
		break;

	case SWING_IOCTL_CAM_SET_EXTEND:
		ret = swing_v4l2_ioctl_cam_extend(arg);
		break;

	case SWING_IOCTL_CAM_GET_OTP:
		ret = swing_v4l2_ioctl_get_otp(arg);
		break;

	default:
		swing_cam_log_err("%s unknown cmd : 0x%x", __FUNCTION__, cmd);
		return -ENOTTY;
	}

	return ret;
}

static int miniisp_subdev_subscribe_event(struct v4l2_subdev *sd,
	struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
	swing_cam_log_info("enter %s", __FUNCTION__);
	return v4l2_event_subscribe(fh, sub, 128, NULL); /* 128:length */
}

static int miniisp_subdev_unsubscribe_event(struct v4l2_subdev *sd,
	struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
	swing_cam_log_info("enter %s", __FUNCTION__);
	return v4l2_event_unsubscribe(fh, sub);
}

static struct v4l2_subdev_core_ops s_subdev_core_ops_miniisp = {
	.ioctl = miniisp_vo_subdev_ioctl,
	.subscribe_event = miniisp_subdev_subscribe_event,
	.unsubscribe_event = miniisp_subdev_unsubscribe_event,
};

static struct v4l2_subdev_internal_ops s_subdev_internal_ops_miniisp = {
	.open = swing_cam_dev_open,
	.close = swing_cam_dev_release,
};

static struct v4l2_subdev_ops s_subdev_ops_miniisp = {
	.core = &s_subdev_core_ops_miniisp,
};

static int miniisp_probe(struct platform_device *pdev)
{
	int rc;
	struct v4l2_subdev *subdev = NULL;

	subdev = &g_miniIsp.subdev;

	if (!pdev) {
		swing_cam_log_err("%s: pdev is NULL", __FUNCTION__);
		return -EFAULT;
	}

	swing_cam_log_info("%s...", __FUNCTION__);

	g_miniIsp.pdev = pdev;
	mutex_init(&g_miniIsp.lock);
	v4l2_subdev_init(subdev, &s_subdev_ops_miniisp);
	subdev->internal_ops = &s_subdev_internal_ops_miniisp;
	snprintf_s(subdev->name, sizeof(subdev->name),
		sizeof(subdev->name), "hwcam-cfg-miniisp");
	subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS;

	(void)init_subdev_media_entity(subdev, CAM_SUBDEV_SWING);
	(void)cam_cfgdev_register_subdev(subdev, CAM_SUBDEV_SWING);
	subdev->devnode->lock = &g_miniIsp.lock;
	platform_set_drvdata(pdev, subdev);

	rc = swing_cam_dev_init(pdev);
	swing_cam_log_info("%s exit...", __FUNCTION__);

	return rc;
}

static int __exit miniisp_remove(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev = NULL;

	swing_cam_log_info("%s...", __FUNCTION__);

	(void)pdev;
	if (g_miniIsp.pdev == NULL)
		return -EFAULT;
	subdev = platform_get_drvdata(g_miniIsp.pdev);
	media_entity_cleanup(&subdev->entity);
	cam_cfgdev_unregister_subdev(subdev);
	mutex_destroy(&g_miniIsp.lock);

	miniisp_dev_exit();

	return 0;
}

static const struct of_device_id g_swing_cam_match_table[] = {
	{ .compatible = SWING_CAM_DRV_NAME, },
	{},
};

static struct platform_driver g_swing_cam_platdev = {
	.driver = {
		.name = "huawei,swing_cam_dev",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_swing_cam_match_table),
	},
	.probe  = miniisp_probe,
	.remove = miniisp_remove,
};

static int __init swing_cam_main_init(void)
{
	swing_cam_log_info("%s enter", __FUNCTION__);
	return platform_driver_register(&g_swing_cam_platdev);
}

static void __exit swing_cam_main_exit(void)
{
	swing_cam_log_info("%s enter", __FUNCTION__);
	platform_driver_unregister(&g_swing_cam_platdev);
}

late_initcall_sync(swing_cam_main_init);
module_exit(swing_cam_main_exit);
MODULE_DESCRIPTION("swingcamdev");
MODULE_LICENSE("GPL v2");
