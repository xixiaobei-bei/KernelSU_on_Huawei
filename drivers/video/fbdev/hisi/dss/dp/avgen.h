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
