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

#ifndef __DPTX_AVGEN_H__
#define __DPTX_AVGEN_H__

#include "../hisi_dp.h"

#define AUDIO_INFOFREAME_HEADER 0x441B8400

void dptx_audio_params_reset(struct audio_params *aparams);
void dptx_audio_config(struct dp_ctrl *dptx);
void dptx_audio_core_config(struct dp_ctrl *dptx);
void dptx_audio_gen_config(struct dp_ctrl *dptx);
void dptx_en_audio_channel(struct dp_ctrl *dptx, int ch_num, int enable);
void dptx_audio_mute(struct dp_ctrl *dptx);

void dptx_audio_inf_type_change(struct dp_ctrl *dptx);
void dptx_audio_num_ch_change(struct dp_ctrl *dptx);
void dptx_audio_data_width_change(struct dp_ctrl *dptx);

bool dptx_dtd_fill(struct dtd *mdtd, uint8_t code, uint32_t refresh_rate,
		  uint8_t video_format);
void dwc_dptx_dtd_reset(struct dtd *mdtd);
int dptx_dtd_parse(struct dp_ctrl *dptx, struct dtd *mdtd, uint8_t data[18]);
void dptx_video_params_reset(struct video_params *params);
void dptx_video_timing_change(struct dp_ctrl *dptx, int stream);
int dptx_change_video_mode_tu_fail(struct dp_ctrl *dptx);
int dptx_change_video_mode_user(struct dp_ctrl *dptx);
int dptx_video_mode_change(struct dp_ctrl *dptx, uint8_t vmode, int stream);
int dptx_video_config(struct dp_ctrl *dptx, int stream);
void dptx_video_core_config(struct dp_ctrl *dptx, int stream);
void dptx_video_gen_config(struct dp_ctrl *dptx, int stream);
void dptx_video_set_core_bpc(struct dp_ctrl *dptx, int stream);
void dptx_video_set_gen_bpc(struct dp_ctrl *dptx, int stream);
void dptx_video_set_sink_bpc(struct dp_ctrl *dptx, int stream);
void dptx_video_set_sink_col(struct dp_ctrl *dptx, int stream);
void dptx_video_set_gen_bits_per_comp(struct dp_ctrl *dptx);
void dptx_video_pattern_change(struct dp_ctrl *dptx, int stream);
void dptx_video_bpc_change(struct dp_ctrl *dptx, int stream);
void dptx_video_ycc_mapping_change(struct dp_ctrl *dptx, int stream);
void dptx_video_ts_change(struct dp_ctrl *dptx, int stream);
int dptx_video_ts_calculate(struct dp_ctrl *dptx, int lane_num, int rate,
			    int bpc, int encoding, int pixel_clock);
void dptx_video_pattern_set(struct dp_ctrl *dptx, uint8_t pattern, int stream);
void dptx_enable_default_video_stream(struct dp_ctrl *dptx, int stream);
void dptx_disable_default_video_stream(struct dp_ctrl *dptx, int stream);
void dptx_audio_infoframe_sdp_send(struct dp_ctrl *dptx);
void dptx_video_reset(struct dp_ctrl *dptx, int enable, int stream);
//void dptx_audio_clock_freq_change(struct dp_ctrl *dptx, uint8_t freq_id);
void dptx_audio_samp_freq_config(struct dp_ctrl *dptx);
void dptx_audio_infoframe_sdp_send(struct dp_ctrl *dptx);
bool dptx_check_low_temperature(struct dp_ctrl *dptx);
#ifdef CONFIG_HISI_FB_V510
void dptx_vsd_ycbcr420_send(struct dp_ctrl *dptx, u8 enable);
#endif
//#define DPTX_COMBO_PHY
// To enable audio during hotplug
void dptx_audio_sdp_en(struct dp_ctrl *dptx);
void dptx_audio_timestamp_sdp_en(struct dp_ctrl *dptx);

#endif
