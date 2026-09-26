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
#ifndef _HDCP13_H
#define _HDCP13_H

#include <linux/kernel.h>
#include "hisi_dp.h"


#define REPEATER_STATE 0x2693
#define MAX_LINK_ERROR_COUNT    4

#define HDCP_POLL_STOP             0
#define HDCP_POLL_CHECK_DISABLE    1
#define HDCP_POLL_CHECK_ENABLE     2

#define HDCP_CHECK_DISABLE 0
#define HDCP_CHECK_ENABLE  1
#define HDCP_CHECK_STOP    15

extern uint64_t g_BKSV;
extern uint8_t g_bcaps;
extern struct platform_device *g_dp_pdev;

struct hdcp_ctrl {
    struct dp_ctrl *dptx;
    struct workqueue_struct *hdcp_notify_wq;
    struct work_struct hdcp_notify_work;
    uint32_t notification;
};

void HDCP13_enable(struct dp_ctrl *dptx, bool en);

void HDCP_DP_on(struct dp_ctrl *dptx, bool en);

void HDCP_Stop_Polling_task(uint8_t stop);

int HDCP_Read_TEInfo(struct dp_ctrl *dptx);

int HDCP_CheckEnable(uint32_t IsCheckEnable);

int HDCP_GetSHA1Buffer(uint8_t* pSHA1buffer, uint32_t* plength, uint8_t* pV_prime);

void HDCP_SendNotification(uint32_t notification);

#endif


