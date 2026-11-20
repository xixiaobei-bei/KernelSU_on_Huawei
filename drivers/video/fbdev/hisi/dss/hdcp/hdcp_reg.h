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

#ifndef HDCP_REG_H
#define HDCP_REG_H

#include "../hisi_fb_def.h"
#include "hisi_fb.h"
#include "hisi_dp.h"

#define HDCP_TAG "[HDCP]"
#define HDCP13_TAG "[HDCP13]"
#define HDCP22_TAG "[HDCP22]"

/* hdcp return code  */
#define RET_HDCP_SUCCESS        0
#define RET_HDCP_ERROR          (-1)
#define RET_HDCP13_AUTH_SUCCESS 1
#define RET_HDCP13_AUTH_FAIL    2
#define RET_HDCP13_AUTH_TIMEOUT 3
#define RET_HDCP_ENABLE         4
#define RET_HDCP_DISABLE        5
#define RET_HDCP_CP_IRQ         6



struct hdcp_cb_fun {
	int (*intr_en)(struct dp_ctrl *dptx);
	int (*intr_dis)(struct dp_ctrl *dptx);
	int (*handle_cp_irq)(struct dp_ctrl *dptx);
	int (*handle_intr)(struct dp_ctrl *dptx, uint32_t *auth_fail_count);
	void (*init)(void);
	void (*deinit)(void);
	int (*check_status)(struct dp_ctrl *dptx);
};

void hdcp_cb_fun_register(struct hdcp_cb_fun *hdcp_cb);


#endif

