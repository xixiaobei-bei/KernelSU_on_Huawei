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

#ifndef __HIDPC_DP_AVGEN_H__
#define __HIDPC_DP_AVGEN_H__

#include <linux/kernel.h>
#include "hisi_dp_drv.h"

struct dp_ctrl;

struct t1_map {
	enum color_depth colordep;
	enum pixel_enc_type encoding;
	int mst_t1_val;
};

void dptx_timing_config(struct dp_ctrl *dptx);

void dptx_audio_config(struct dp_ctrl *dptx);
void dptx_audio_core_config(struct dp_ctrl *dptx);
void dptx_en_audio_channel(struct dp_ctrl *dptx, int ch_num, int enable);
void dptx_audio_mute(struct dp_ctrl *dptx);

void dptx_audio_num_ch_change(struct dp_ctrl *dptx);
void dptx_audio_inf_type_change(struct dp_ctrl *dptx);
void dptx_audio_data_width_change(struct dp_ctrl *dptx);

void dptx_enable_default_video_stream(struct dp_ctrl *dptx, int stream);
void dptx_disable_default_video_stream(struct dp_ctrl *dptx, int stream);
int dptx_triger_media_transfer(struct dp_ctrl *dptx, bool benable);
int dptx_resolution_switch(struct hisi_dp_device *dp_dev, enum dptx_hot_plug_type etype);
int dptx_video_ts_calculate(struct dp_ctrl *dptx, struct video_params *vparams,
	int pixel_clock);


/* disable stream when hotunplug, and config dss and notify hwc */
void dptx_link_close_stream(struct dp_ctrl *dptx);

void dptx_hdr_infoframe_set_reg(struct dp_ctrl *dptx, uint8_t enable);
void dptx_audio_infoframe_sdp_send(struct dp_ctrl *dptx);

int dptx_change_video_mode_tu_fail(struct dp_ctrl *dptx);
void dptx_video_core_config(struct dp_ctrl *dptx, int stream);
void dptx_video_set_core_bpc(struct dp_ctrl *dptx, int stream);
void dptx_video_set_sink_bpc(struct dp_ctrl *dptx, int stream);
void dptx_video_set_sink_col(struct dp_ctrl *dptx, int stream);
void dptx_video_bpc_change(struct dp_ctrl *dptx, int stream);
void dptx_video_pattern_change(struct dp_ctrl *dptx, int stream);
void dptx_video_reset(struct dp_ctrl *dptx, int enable, int stream);
void dptx_vsd_ycbcr420_send(struct dp_ctrl *dptx, u8 enable);

int dptx_link_timing_config(struct dp_ctrl *dptx);
void dptx_video_timing_change(struct dp_ctrl *dptx, int stream);
void dptx_link_set_lane_after_training(struct dp_ctrl *dptx);
void dptx_video_ts_change(struct dp_ctrl *dptx, int stream);
void dptx_phy_set_pattern(struct dp_ctrl *dptx, u32 pattern);

// To enable audio during hotplug
void dptx_audio_sdp_en(struct dp_ctrl *dptx);
void dptx_audio_timestamp_sdp_en(struct dp_ctrl *dptx);
/**
 * enable/disable dsc
 */
int dptx_fec_enable(struct dp_ctrl *dptx);
void dptx_fec_disable(struct dp_ctrl *dptx);

int dptx_change_video_mode_by_drm(struct dp_ctrl *dptx,
				  struct drm_display_mode *drm_mode);
#endif /* HIDPC_DP_AVGEN_H */
