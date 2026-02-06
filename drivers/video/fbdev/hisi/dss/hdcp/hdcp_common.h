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
#ifndef HDCP_COMMON_H
#define HDCP_COMMON_H

#include "hdcp_reg.h"

#define REPEATER_STATE 0x2693
#define MAX_LINK_ERROR_COUNT 4

#define HDCP_POLL_STOP          0
#define HDCP_POLL_CHECK_DISABLE 1
#define HDCP_POLL_CHECK_ENABLE  2

#define HDCP_CHECK_DISABLE 0
#define HDCP_CHECK_ENABLE  1
#define HDCP_CHECK_STOP    15

extern uint64_t g_bksv;
extern uint8_t g_bcaps;
extern struct platform_device *g_dp_pdev;

struct hdcp_ctrl {
	struct dp_ctrl *dptx;
	struct workqueue_struct *hdcp_notify_wq;
	struct work_struct hdcp_notify_work;
	uint32_t notification;
	int hdcp_counter;
	uint32_t auth_fail_count;
	struct hdcp_cb_fun hdcp_cb;
};

enum hdcp_cnt_type {
	HDCP_CNT_SET,
	HDCP_CNT_RESET,
	HDCP_CNT_INCREASE,
	HDCP_CNT_DECREASE
};

int hdcp_read_te_info(struct dp_ctrl *dptx);
int hdcp_get_sha1_buffer(uint8_t* pSHA1buffer, uint32_t* plength, uint8_t* pV_prime);

void hdcp_stop_polling_task(uint8_t stop);
int hdcp_check_enable(uint32_t is_check_enable);

void hdcp_notification(struct switch_dev *sdev, uint32_t state);
void hdcp_wq_notification(uint32_t notification);

void hdcp_set_counter(enum hdcp_cnt_type type);
int hdcp_get_counter(void);

void hdcp_dp_on(struct dp_ctrl *dptx, bool en);
void hdcp_register(void);
void hdcp_intr_en(struct dp_ctrl *dptx);
void hdcp_intr_dis(struct dp_ctrl *dptx);
void hdcp_handle_cp_irq(struct dp_ctrl *dptx);
void hdcp_handle_intr(struct dp_ctrl *dptx);

#endif


