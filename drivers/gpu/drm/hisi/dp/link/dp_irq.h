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

#ifndef __DP_IRQ_H__
#define __DP_IRQ_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include "hisi_dp_drv.h"

#define MAX_EXT_BLOCKS 3
#define DP_DMD_REPORT_SIZE 900
#define VBLANKING_MAX 255

int handle_hotplug(struct hisi_dp_device *dp_dev);
int handle_hotunplug(struct hisi_dp_device *dp_dev);
void dptx_link_params_reset(struct dp_ctrl *dptx);
void drmmode_to_dtd(struct dtd *mdtd, struct drm_display_mode *newmode);

#define link_timming_descriptor(htotal, vtotal, hstart, vstart, hwidth, vwidth, \
	hsyncwidth, vsyncwidth, refreshrate, vmode, vformat) \
	.h_total = htotal, .v_total = vtotal, \
	.h_start = hstart, .v_start = vstart, \
	.h_width = hwidth, .v_width = vwidth, \
	.h_sync_width = hsyncwidth, .v_sync_width = vsyncwidth, \
	.refresh_rate = refreshrate,\
	.v_mode = vmode, .video_format = vformat \

struct dp_test_link_timming {
	u32 h_total;
	u32 v_total;
	u32 h_start;
	u32 v_start;
	u32 v_width;
	u32 h_width;
	u32 h_sync_width;
	u32 v_sync_width;
	u32 refresh_rate;
	u8 v_mode;
	enum video_format_type video_format;
};

extern const struct dp_test_link_timming timming_table[];

/**
 *
 */
int handle_sink_request(struct dp_ctrl *dptx);
#endif /* DP_IRQ_H */
