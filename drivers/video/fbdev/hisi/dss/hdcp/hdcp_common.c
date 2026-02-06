/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#include "hdcp_common.h"
#include "../dp/link/dp_aux.h"

#define SHA1_MAX_LENGTH 650 /* 635+2+8 */

#define HDCP_STOP_WAIT_TIME 100 /* 1000ms */
#define HDCP_STOP_CHECK_INTERVAL 10

uint64_t g_bksv; /*lint !e552*/
uint8_t g_bcaps;
static uint8_t g_Binfo[2];

static struct task_struct *g_hdcp_polling_task = NULL;
static uint8_t g_hdcp_polling_flag = 0;
DEFINE_SEMAPHORE(g_hdcp_poll_sem);
static struct hdcp_ctrl g_hdcp_control;


static int hdcp_get_ksv_list(struct dp_ctrl *dptx, uint8_t* sha1_buffer, uint32_t* plength)
{
	uint32_t dev_count, i;
	uint8_t temp[16] = { 0 };
	uint32_t ptr = 0;
	uint8_t* ksv_list = NULL;
	uint32_t len = 10;
	int retval;

	if ((dptx == NULL) || (sha1_buffer == NULL) || (plength == NULL)) {
		DPU_FB_ERR("NULL Pointer\n");
		return -1;
	}

	ksv_list = sha1_buffer;
	retval = dptx_read_bytes_from_dpcd(dptx, 0x6802A, g_Binfo, 2);
	if (retval) {
		DPU_FB_ERR("failed to dptx_read_bytes_from_dpcd Binfo, retval=%d.\n", retval);
		return retval;
	}
	DPU_FB_DEBUG("dptx_read Binfo:%x,%x!\n", g_Binfo[0], g_Binfo[1]);
	dev_count = g_Binfo[0];

	while (dev_count > 0) {
		if (dev_count >= 3) {
			retval = dptx_read_bytes_from_dpcd(dptx, 0x6802C, temp, 15);
			if (retval) {
				DPU_FB_ERR("failed to dptx_read_bytes_from_dpcd KSVlist, retval=%d.\n", retval);
				return retval;
			}
			for(i = 0; i < 15; i++)
				ksv_list[ptr++] = temp[i];

			dev_count -= 3;
		}
		else {
			retval = dptx_read_bytes_from_dpcd(dptx, 0x6802C, temp, dev_count * 5);
			if (retval) {
				DPU_FB_ERR("failed to dptx_read_bytes_from_dpcd KSVlist, retval=%d.\n", retval);
				return retval;
			}
			for(i = 0; i < dev_count * 5; i++)
				ksv_list[ptr++] = temp[i];

			dev_count = 0;
		}
	}
	len += ptr;
	DPU_FB_DEBUG("Read %d KSV:\n", ptr);
	ksv_list[ptr++] = g_Binfo[0];
	ksv_list[ptr++] = g_Binfo[1];

	*plength = len;
	return 0;
}


static int hdcp_read_v_prime(struct dp_ctrl *dptx, uint8_t* v_prime)
{
	int retval;

	if ((dptx == NULL) || (v_prime == NULL)) {
		DPU_FB_ERR("NULL Pointer\n");
		return -1;
	}

	retval = dptx_read_bytes_from_dpcd(dptx, 0x68014, &v_prime[0], 16);
	if (retval) {
		DPU_FB_ERR("failed to dptx_read_bytes_from_dpcd V', retval=%d.\n", retval);
		return retval;
	}
	retval = dptx_read_bytes_from_dpcd(dptx, 0x68024, &v_prime[16], 4);
	if (retval) {
		DPU_FB_ERR("failed to dptx_read_bytes_from_dpcd V', retval=%d.\n", retval);
		return retval;
	}

	return 0;
}

int hdcp_read_te_info(struct dp_ctrl *dptx)
{
	uint8_t temp[16] = { 0 };
	uint32_t i;
	int retval;

	if (dptx == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return -1;
	}

	retval = dptx_read_bytes_from_dpcd(dptx, 0x68000, temp, 5);
	if (retval) {
		DPU_FB_ERR("failed to dptx_read_bytes_from_dpcd bKSV, retval=%d.\n", retval);
		return retval;
	}
	g_bksv = 0;
	for(i = 0; i < 5; i++)
	{
		g_bksv += temp[4-i];
		if(i<4)
			g_bksv <<= 8;
	}

	retval = dptx_read_bytes_from_dpcd(dptx, 0x6802A, g_Binfo, 2);
	if (retval) {
		DPU_FB_ERR("failed to dptx_read_bytes_from_dpcd binfo, retval=%d.\n", retval);
		return retval;
	}
	DPU_FB_DEBUG("dptx_read Binfo:%x,%x!\n", g_Binfo[0],g_Binfo[1]);
	return 0;
}

int hdcp_get_sha1_buffer(uint8_t* sha1_buffer, uint32_t* plength, uint8_t* v_prime)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dp_ctrl *dptx = NULL;

	if(sha1_buffer == NULL || plength == NULL || v_prime == NULL){
		DPU_FB_ERR("pSHA1buffer or plength or pV_prime is NULL!\n");
		return -1;
	}

	if (g_dp_pdev == NULL) {
		DPU_FB_ERR("g_dp_pdev is NULL!\n");
		return -1;
	}

	dpufd = platform_get_drvdata(g_dp_pdev);
	if (dpufd == NULL) {
			DPU_FB_ERR("dpufd is NULL!\n");
			return -1;
	}
	dptx = &(dpufd->dp);

	if (hdcp_get_ksv_list(dptx, sha1_buffer, plength))
	{
		DPU_FB_ERR("GetKSVListFromDPCD failed!\n");
		return -1;
	}

	/* Read V' */
	if (hdcp_read_v_prime(dptx, v_prime))
	{
		DPU_FB_ERR("Read_V_prime_fromDPCD failed!\n");
		return -1;
	}

	return 0;
}

void hdcp_notification(struct switch_dev *sdev, uint32_t state)
{
	char *envp[3];
	char name_buf[120];
	char state_buf[120];
	int length;

	if (sdev == NULL) {
		DPU_FB_ERR("[HDCP] sdev is NULL!\n");
		return;
	}

	length = snprintf(name_buf, sizeof(name_buf), "SWITCH_NAME=%s\n", sdev->name);
	if (length > 0) {
		if (name_buf[length-1] == '\n') {
			name_buf[length-1] = 0;
		}
	}

	length = snprintf(state_buf, sizeof(state_buf), "SWITCH_STATE=%d\n", state);
	if (length > 0) {
		if (state_buf[length-1] == '\n') {
			state_buf[length-1] = 0;
		}
	}

	envp[0] = name_buf;
	envp[1] = state_buf;
	envp[2] = NULL;
	kobject_uevent_env(&sdev->dev->kobj, KOBJ_CHANGE, envp);

	DPU_FB_NOTICE("[HDCP] Notification: %s, %s.\n", name_buf, state_buf);
	return;
}

static int hdcp_polling_thread(void *p)
{
	struct dp_ctrl *dptx = (struct dp_ctrl *)p;
	int ret;

	while(!kthread_should_stop()) {
		msleep(100);
		if (!g_hdcp_control.hdcp_cb.check_status)
			continue;

		down(&g_hdcp_poll_sem);
		if (g_hdcp_polling_flag == HDCP_POLL_CHECK_DISABLE) {
			ret = g_hdcp_control.hdcp_cb.check_status(dptx);
			if (ret == RET_HDCP_ERROR) {
				g_hdcp_polling_flag = HDCP_POLL_STOP;
				up(&g_hdcp_poll_sem);
				continue;
			}
			DPU_FB_DEBUG("%s check hdcp disable", HDCP_TAG);
			if (ret == RET_HDCP_DISABLE) {
				g_hdcp_polling_flag = HDCP_POLL_STOP;
				hdcp_notification(&dptx->sdev, HOT_PLUG_HDCP_DISABLE);
			}
		} else if (g_hdcp_polling_flag == HDCP_POLL_CHECK_ENABLE) {
			ret = g_hdcp_control.hdcp_cb.check_status(dptx);
			if (ret == RET_HDCP_ERROR) {
				g_hdcp_polling_flag = HDCP_POLL_STOP;
				up(&g_hdcp_poll_sem);
				continue;
			}
			DPU_FB_DEBUG("%s check hdcp enable", HDCP_TAG);
			if (ret == RET_HDCP_ENABLE) {
				g_hdcp_polling_flag = HDCP_POLL_STOP;
				hdcp_notification(&dptx->sdev, HOT_PLUG_HDCP_ENABLE);
			}
		}
		up(&g_hdcp_poll_sem);
	}
	return 0;
}


void hdcp_stop_polling_task(uint8_t stop)
{
	down(&g_hdcp_poll_sem);
	g_hdcp_polling_flag = HDCP_POLL_STOP;
	up(&g_hdcp_poll_sem);
}

/*
 * 0-check hdcp disable
 * 1-check hdcp enable
 */
int hdcp_check_enable(uint32_t is_check_enable)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dp_ctrl *dptx = NULL;

	if (g_dp_pdev == NULL) {
		DPU_FB_ERR("g_dp_pdev is NULL!\n");
		return -1;
	}

	dpufd = platform_get_drvdata(g_dp_pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return -1;
	}
	dptx = &(dpufd->dp);

	DPU_FB_ERR("HDCP_CheckEnable enter!\n");
	if (g_hdcp_polling_task == NULL) {
		g_hdcp_polling_task = kthread_create(hdcp_polling_thread, dptx, "hdcp_polling_task");
		if(IS_ERR(g_hdcp_polling_task)) {
			DPU_FB_ERR("Unable to start kernel hdcp_polling_task./n");
			g_hdcp_polling_task = NULL;
			return -1;
		}
	}
	down(&g_hdcp_poll_sem);
	g_hdcp_polling_flag = (is_check_enable == 0) ? (HDCP_POLL_CHECK_DISABLE):(HDCP_POLL_CHECK_ENABLE);
	up(&g_hdcp_poll_sem);

	wake_up_process(g_hdcp_polling_task);
	return 0;
}

static void hdcp_notify_wq_handler(struct work_struct *work)
{
	struct hdcp_ctrl *hdcp = NULL;

	hdcp = container_of(work, struct hdcp_ctrl, hdcp_notify_work);

	if (hdcp == NULL) {
		DPU_FB_ERR("[HDCP] hdcp is null!\n");
		return;
	}
	if (hdcp->dptx == NULL) {
		DPU_FB_ERR("[HDCP] hdcp->dptx is null!\n");
		return;
	}

	hdcp_notification(&hdcp->dptx->sdev, hdcp->notification);
}

static void hdcp_init(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		DPU_FB_ERR("[HDCP]dptx is null!\n");
		return;
	}

	g_hdcp_control.dptx = dptx;
	g_hdcp_control.notification = 0;
	g_hdcp_control.hdcp_notify_wq = create_singlethread_workqueue("hdcp_notify");
	if (g_hdcp_control.hdcp_notify_wq == NULL) {
		DPU_FB_ERR("[HDCP]create hdcp_wq failed!\n");
		return;
	}
	INIT_WORK(&g_hdcp_control.hdcp_notify_work, hdcp_notify_wq_handler);

	g_hdcp_control.auth_fail_count = 0;
	DPU_FB_NOTICE("[HDCP]hdcp kernel is initialized\n");
}

static void hdcp_deinit(struct dp_ctrl *dptx)
{
	int i = 0;

	g_hdcp_control.dptx = NULL;
	g_hdcp_control.notification = 0;
	if (g_hdcp_control.hdcp_notify_wq != NULL) {
		destroy_workqueue(g_hdcp_control.hdcp_notify_wq);
		g_hdcp_control.hdcp_notify_wq = NULL;
	}

	/*
	 * wait hdcp read/write registers in TA and BL31 finish
	 * time delay is 1s, once is 10ms
	 */
	hdcp_notification(&dptx->sdev, HOT_PLUG_HDCP_OUT);
	if (g_hdcp_control.hdcp_counter > 0) {
		DPU_FB_INFO("[HDCP]The hdcp is processing, need to wait\n");
		while (i < HDCP_STOP_WAIT_TIME) {
			msleep(HDCP_STOP_CHECK_INTERVAL);
			if (g_hdcp_control.hdcp_counter <= 0)
				break;
			i++;
		}
	}
	DPU_FB_INFO("[HDCP]hdcp kernel is de-init, wait count = %d\n", i);
}

void hdcp_dp_on(struct dp_ctrl *dptx, bool en)
{
	if (dptx == NULL) {
		DPU_FB_ERR("[HDCP]dptx is null!\n");
		return;
	}

	if (g_hdcp_polling_task != NULL) {
		kthread_stop(g_hdcp_polling_task);
		g_hdcp_polling_task = NULL;
		g_hdcp_polling_flag = HDCP_POLL_STOP;
	}

	if (en) {
		if (g_hdcp_control.hdcp_cb.init)
			g_hdcp_control.hdcp_cb.init();

		hdcp_init(dptx);
	} else {
		if (g_hdcp_control.hdcp_cb.deinit)
			g_hdcp_control.hdcp_cb.deinit();

		hdcp_deinit(dptx);
	}
}

void hdcp_wq_notification(uint32_t notification)
{
	g_hdcp_control.notification = notification;
	if (g_hdcp_control.hdcp_notify_wq != NULL)
		queue_work(g_hdcp_control.hdcp_notify_wq, &(g_hdcp_control.hdcp_notify_work));
}

void hdcp_set_counter(enum hdcp_cnt_type type)
{
	switch (type) {
	case HDCP_CNT_SET:
		g_hdcp_control.hdcp_counter = 1;
		break;
	case HDCP_CNT_RESET:
		g_hdcp_control.hdcp_counter = 0;
		break;
	case HDCP_CNT_INCREASE:
		g_hdcp_control.hdcp_counter++;
		break;
	default: /* counter decrease */
		g_hdcp_control.hdcp_counter--;
		break;
	}
	DPU_FB_DEBUG("%s hdcp cnt = %d, type = %u", HDCP_TAG, g_hdcp_control.hdcp_counter, type);
}

int hdcp_get_counter(void)
{
	return g_hdcp_control.hdcp_counter;
}

void hdcp_register(void)
{
	hdcp_cb_fun_register(&g_hdcp_control.hdcp_cb);
}

void hdcp_intr_en(struct dp_ctrl *dptx)
{
	if (g_hdcp_control.hdcp_cb.intr_en)
		g_hdcp_control.hdcp_cb.intr_en(dptx);
}
void hdcp_intr_dis(struct dp_ctrl *dptx)
{
	if (g_hdcp_control.hdcp_cb.intr_dis)
		g_hdcp_control.hdcp_cb.intr_dis(dptx);
}

void hdcp_handle_cp_irq(struct dp_ctrl *dptx)
{
	int ret;

	if (!g_hdcp_control.hdcp_cb.handle_cp_irq)
		return;

	ret = g_hdcp_control.hdcp_cb.handle_cp_irq(dptx);
	if (ret == RET_HDCP_CP_IRQ)
		hdcp_wq_notification(HOT_PLUG_HDCP_CP_IRQ);
}

void hdcp_handle_intr(struct dp_ctrl *dptx)
{
	int ret;

	if (!g_hdcp_control.hdcp_cb.handle_intr)
		return;

	ret = g_hdcp_control.hdcp_cb.handle_intr(dptx, &g_hdcp_control.auth_fail_count);
	switch(ret) {
	case RET_HDCP13_AUTH_SUCCESS:
		hdcp_wq_notification((uint32_t)HOT_PLUG_HDCP13_SUCCESS);
		break;
	case RET_HDCP13_AUTH_FAIL:
		hdcp_wq_notification((uint32_t)HOT_PLUG_HDCP13_FAIL);
		break;
	case RET_HDCP13_AUTH_TIMEOUT:
		hdcp_wq_notification((uint32_t)HOT_PLUG_HDCP13_TIMEOUT);
		break;
	default:
		break;
	}
}


