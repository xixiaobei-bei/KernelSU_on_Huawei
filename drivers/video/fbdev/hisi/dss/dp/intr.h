/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#ifndef __DPTX_INTR_H__
#define __DPTX_INTR_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>

#define MAX_AUX_RETRY_COUNT 6
#define MAX_EXT_BLOCKS 3
#define AUX_RETRY_DELAY_TIME 100
#define DP_DMD_REPORT_SIZE 900

extern uint32_t g_bit_hpd_status;

enum dptx_hot_plug_type{
	Hot_Plug_OUT = 0,
	Hot_Plug_IN,
	Hot_Plug_TEST,
	Hot_Plug_IN_VR,
	Hot_Plug_OUT_VR,
	Hot_Plug_HDCP13_SUCCESS,
	Hot_Plug_HDCP13_TIMEOUT,
	Hot_Plug_HDCP13_FAIL,
	Hot_Plug_HDCP13_POSTAUTH,
	Hot_Plug_HDCP_ENABLE,
	Hot_Plug_HDCP_DISABLE,
	Hot_Plug_TEST_OUT,
	Hot_Plug_MAINPANEL_UP,
	Hot_Plug_MAINPANEL_DOWN,
};

irqreturn_t dptx_irq(int irq, void *dev);
irqreturn_t dptx_threaded_irq(int irq, void *dev);

int handle_hotunplug(struct hisi_fb_data_type *hisifd);
int handle_hotplug(struct hisi_fb_data_type *hisifd);
void dptx_hpd_handler(struct dp_ctrl *dptx, bool plugin, uint8_t dp_lanes);
void dptx_hpd_irq_handler(struct dp_ctrl *dptx);
int dptx_triger_media_transfer(struct dp_ctrl *dptx, bool benable);
#endif
