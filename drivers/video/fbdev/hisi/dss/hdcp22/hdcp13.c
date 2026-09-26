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


#include "../hisi_fb_def.h"
#include "../dp/dp_aux.h"
#include "hisi_fb.h"
#include "../dp/intr.h"
#include "hdcp13.h"

#define SHA1_MAX_LENGTH 650    //635+2+8

uint64_t g_BKSV=0;
uint8_t g_bcaps = 0;


//static uint8_t g_dp_on = 0;
static uint8_t g_Binfo[2];
//static uint8_t g_sha1_buffer[SHA1_MAX_LENGTH];
//static uint8_t V_prime[20];
static struct task_struct *hdcp_polling_task = NULL;
static uint8_t hdcp_polling_flag = 0;
DEFINE_SEMAPHORE(g_hdcp_poll_sem);
static struct hdcp_ctrl hdcp_control;


static int GetKSVListFromDPCD(struct dp_ctrl *dptx, uint8_t* sha1_buffer, uint32_t* plength)
{
	uint32_t dev_count,  i;
	uint8_t temp[16];
	uint32_t ptr=0;
	uint8_t* pKSVList;
	uint32_t len = 10;
	int retval;

	if ((dptx == NULL) || (sha1_buffer == NULL) || (plength == NULL)) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	pKSVList = sha1_buffer;

	retval = dptx_read_bytes_from_dpcd(dptx, 0x6802A, g_Binfo, 2);
	if (retval) {
		HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd Binfo, retval=%d.\n", retval);
		return retval;
	}
	HISI_FB_DEBUG("dptx_read Binfo:%x,%x!\n", g_Binfo[0],g_Binfo[1]);
	dev_count = g_Binfo[0];

	while (dev_count>0)
	{
		if (dev_count >=3)
		{
			retval = dptx_read_bytes_from_dpcd(dptx, 0x6802C, temp, 15);
			if (retval) {
				HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd KSVlist, retval=%d.\n", retval);
				return retval;
			}
			for(i=0; i<15; i++)
			{
				pKSVList[ptr++] = temp[i];
			}
			dev_count -= 3;
		}
		else
		{
			retval = dptx_read_bytes_from_dpcd(dptx, 0x6802C, temp, dev_count*5);
			if (retval) {
				HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd KSVlist, retval=%d.\n", retval);
				return retval;
			}
			for(i=0; i<dev_count*5; i++)
			{
				pKSVList[ptr++] = temp[i];
			}
			dev_count = 0;
		}
	}
	len += ptr;
	HISI_FB_DEBUG("Read %d KSV:\n", ptr);
	pKSVList[ptr++] = g_Binfo[0];
	pKSVList[ptr++] = g_Binfo[1];

	/*for(i=0; i<8; i++)
	{
		pKSVList[ptr++]  = (M0>>(i*8)) & 0xFF;
	}*/

	//test print
	//HISI_FB_ERR("sha1_buffer:\n");
	//for (i=0; i<len; i++)
	//	  HISI_FB_ERR("%d:%x\n", i,pKSVList[i]);

	*plength = len;
	return 0;
}


static int Read_V_prime_fromDPCD(struct dp_ctrl *dptx, uint8_t* pV_prime)
{
	int retval;

	if ((dptx == NULL) || (pV_prime == NULL)) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	retval = dptx_read_bytes_from_dpcd(dptx, 0x68014, &pV_prime[0], 16);
	if (retval) {
		HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd V', retval=%d.\n", retval);
		return retval;
	}
	retval = dptx_read_bytes_from_dpcd(dptx, 0x68024, &pV_prime[16], 4);
	if (retval) {
		HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd V', retval=%d.\n", retval);
		return retval;
	}

	return 0;
}


/////////////////////////////////////////
//extern API
/////////////////////////////////////////
int HDCP_Read_TEInfo(struct dp_ctrl *dptx)
{
	uint8_t temp[16];
	uint32_t i=0;
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	retval = dptx_read_bytes_from_dpcd(dptx, 0x68000, temp, 5);
	if (retval) {
		HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd bKSV, retval=%d.\n", retval);
		return retval;
	}
	g_BKSV = 0;
	for(i=0; i<5; i++)
	{
		g_BKSV += temp[4-i];
		//HISI_FB_DEBUG(" BKSV: %d: 0x%x\n", 4-i, temp[4-i]);
		if(i<4)
			g_BKSV <<= 8;
	}
	//HISI_FB_DEBUG("dptx_read BKSV: 0x%lx\n", g_BKSV);

	retval = dptx_read_dpcd(dptx, 0x68028, &g_bcaps);
	if (retval) {
		HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd bcaps, retval=%d.\n", retval);
		return retval;
	}
	HISI_FB_DEBUG("dptx_read bcaps:%x!\n", g_bcaps);

	//dptx_read_dpcd(dptx, 0x68029, &bstatus);

	retval = dptx_read_bytes_from_dpcd(dptx, 0x6802A, g_Binfo, 2);
	if (retval) {
		HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd binfo, retval=%d.\n", retval);
		return retval;
	}
	HISI_FB_DEBUG("dptx_read Binfo:%x,%x!\n", g_Binfo[0],g_Binfo[1]);

	//dptx_writel(dptx, 0x3608, 0x1);	  //use manual An temply, need add random number

	return 0;
}

int HDCP_GetSHA1Buffer(uint8_t* pSHA1buffer, uint32_t* plength, uint8_t* pV_prime)
{
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;

	if(pSHA1buffer == NULL || plength == NULL || pV_prime == NULL){
		HISI_FB_ERR("pSHA1buffer or plength or pV_prime is NULL!\n");
		return -1;
	}

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("g_dp_pdev is NULL!\n");
		return -1;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
			HISI_FB_ERR("hisifd is NULL!\n");
			return -1;
	}
	dptx = &(hisifd->dp);

	if (GetKSVListFromDPCD(dptx, pSHA1buffer, plength))
	{
		HISI_FB_ERR("GetKSVListFromDPCD failed!\n");
		return -1;
	}

	//Read V'
	if (Read_V_prime_fromDPCD(dptx, pV_prime))
	{
		HISI_FB_ERR("Read_V_prime_fromDPCD failed!\n");
		return -1;
	}
	/*for (i=0; i<20; i++)
	{
		HISI_FB_DEBUG("%d:0x%x\n", i, V_prime[i]);
	}*/

	return 0;
}

static void HDCP_Notification(struct switch_dev *sdev, uint32_t state)
{
	char *envp[3];
	char name_buf[120];
	char state_buf[120];
	int length;

	if (sdev == NULL) {
		HISI_FB_ERR("[HDCP] sdev is NULL!\n");
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

	HISI_FB_NOTICE("[HDCP] Notification: %s, %s.\n", name_buf, state_buf);
	return;
}

static int hdcp_polling_thread(void *p)
{
	//uint32_t i=0;
	uint32_t temp_value;
	struct hdcp_params *hparams;
	struct dp_ctrl *dptx;

	dptx = (struct dp_ctrl *)p;
	if(!dptx){
		HISI_FB_ERR("dptx is null!\n");
		return -1;
	}

	hparams = &dptx->hparams;

	while(!kthread_should_stop()) {
		msleep(100);
		down(&g_hdcp_poll_sem);
		if (hdcp_polling_flag == HDCP_POLL_CHECK_DISABLE) {
			if (dptx->hisifd->secure_ctrl.hdcp_reg_get) {
				temp_value = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_CFG);
			} else {
				hdcp_polling_flag = HDCP_POLL_STOP;
				up(&g_hdcp_poll_sem);
				continue;
			}
			HISI_FB_DEBUG("check hdcp disable: DPTX_HDCP_CFG is:0x%x!\n", temp_value);
			if(!(temp_value & DPTX_CFG_EN_HDCP)) { //hdcp disable
				hdcp_polling_flag = HDCP_POLL_STOP;
				HDCP_Notification(&dptx->sdev, Hot_Plug_HDCP_DISABLE);
			}
		} else if (hdcp_polling_flag == HDCP_POLL_CHECK_ENABLE) {
			if (dptx->hisifd->secure_ctrl.hdcp_reg_get) {
				temp_value = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_CFG);
			} else {
				hdcp_polling_flag = HDCP_POLL_STOP;
				up(&g_hdcp_poll_sem);
				continue;
			}
			HISI_FB_DEBUG("check hdcp enable: DPTX_HDCP_CFG is:0x%x!\n", temp_value);
			if(temp_value & DPTX_CFG_EN_HDCP) { //hdcp enable
				hdcp_polling_flag = HDCP_POLL_STOP;
				HDCP_Notification(&dptx->sdev, Hot_Plug_HDCP_ENABLE);
			}
		}
		up(&g_hdcp_poll_sem);
	}
	return 0;
}

void HDCP_Stop_Polling_task(uint8_t stop)
{
	down(&g_hdcp_poll_sem);
	hdcp_polling_flag = HDCP_POLL_STOP;
	up(&g_hdcp_poll_sem);
}

/*
0-check hdcp disable
1-check hdcp enable
*/
int HDCP_CheckEnable(uint32_t IsCheckEnable)
{
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("g_dp_pdev is NULL!\n");
		return -1;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -1;
	}
	dptx = &(hisifd->dp);

	HISI_FB_ERR("HDCP_CheckEnable enter!\n");
	if (!hdcp_polling_task) {
		hdcp_polling_task = kthread_create(hdcp_polling_thread, dptx, "hdcp_polling_task");
		if(IS_ERR(hdcp_polling_task)) {
			HISI_FB_ERR("Unable to start kernel hdcp_polling_task./n");
			hdcp_polling_task = NULL;
			return -2;//-EINVAL;
		}
	}
	down(&g_hdcp_poll_sem);
	hdcp_polling_flag = (IsCheckEnable == 0) ? (HDCP_POLL_CHECK_DISABLE):(HDCP_POLL_CHECK_ENABLE);
	up(&g_hdcp_poll_sem);

	wake_up_process(hdcp_polling_task);
	return 0;
}

static void hdcp_notify_wq_handler(struct work_struct *work)
{
	struct hdcp_ctrl *hdcp = NULL;

	hdcp = container_of(work, struct hdcp_ctrl, hdcp_notify_work);

	if (hdcp == NULL) {
		HISI_FB_ERR("[HDCP] hdcp is null!\n");
		return;
	}
	if (hdcp->dptx == NULL) {
		HISI_FB_ERR("[HDCP] hdcp->dptx is null!\n");
		return;
	}

	HDCP_Notification(&hdcp->dptx->sdev, hdcp->notification);
}

static void HDCP_Init(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[HDCP]dptx is null!\n");
		return;
	}

	hdcp_control.dptx = dptx;
	hdcp_control.notification = 0;
	hdcp_control.hdcp_notify_wq = create_singlethread_workqueue("hdcp_notify");
	if (!hdcp_control.hdcp_notify_wq) {
		HISI_FB_ERR("[HDCP]create hdcp_wq failed!\n");
		return;
	}
	INIT_WORK(&hdcp_control.hdcp_notify_work, hdcp_notify_wq_handler);
	HISI_FB_NOTICE("[HDCP]hdcp kernel is initialized\n");
}

static void HDCP_DeInit(void)
{
	hdcp_control.dptx = NULL;
	hdcp_control.notification = 0;
	if (hdcp_control.hdcp_notify_wq) {
		destroy_workqueue(hdcp_control.hdcp_notify_wq);
		hdcp_control.hdcp_notify_wq = NULL;
	}
	HISI_FB_NOTICE("[HDCP]hdcp kernel is de-initialized\n");
}

void HDCP_DP_on(struct dp_ctrl *dptx, bool en)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[HDCP]dptx is null!\n");
		return;
	}

	if (hdcp_polling_task) {
		kthread_stop(hdcp_polling_task);
		hdcp_polling_task = NULL;
		hdcp_polling_flag = HDCP_POLL_STOP;
	}

	if (en) {
		HDCP_Init(dptx);
	} else {
		HDCP_DeInit();
	}
}

void HDCP_SendNotification(uint32_t notification)
{
	hdcp_control.notification = notification;
	if (hdcp_control.hdcp_notify_wq) {
		queue_work(hdcp_control.hdcp_notify_wq, &(hdcp_control.hdcp_notify_work));
	}
}

