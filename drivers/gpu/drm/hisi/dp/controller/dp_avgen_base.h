/* Copyright (c) 2019-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __DP_AVGEN_BASE_H__
#define __DP_AVGEN_BASE_H__

#include "hisi_dp_drv.h"

#define AUDIO_INFOFREAME_HEADER 0x441B8400
#define HDR_INFOFRAME_HEADER 0x4C1D8700

#define DPTX_AUDIO_REFER_TO_STREAM_HEADER		0x0    /* Configured to 0, sink just check stream head */

#define DPTX_AUDIO_CHANNEL_CNT_2CH				0x1
#define DPTX_AUDIO_CHANNEL_CNT_3CH				0x2
#define DPTX_AUDIO_CHANNEL_CNT_4CH				0x3
#define DPTX_AUDIO_CHANNEL_CNT_5CH				0x4
#define DPTX_AUDIO_CHANNEL_CNT_6CH				0x5
#define DPTX_AUDIO_CHANNEL_CNT_7CH				0x6
#define DPTX_AUDIO_CHANNEL_CNT_8CH				0x7
#define DPTX_AUDIO_CHANNEL_CNT_SHIFT			0
#define DPTX_AUDIO_CHANNEL_CNT_MASK			GENMASK(2, 0)

#define DPTX_AUDIO_CODING_TYPE_LPCM			0x1
#define DPTX_AUDIO_CODING_TYPE_AC3				0x2
#define DPTX_AUDIO_CODING_TYPE_MPEG1			0x3
#define DPTX_AUDIO_CODING_TYPE_MP3				0x4
#define DPTX_AUDIO_CODING_TYPE_MPEG2			0x5
#define DPTX_AUDIO_CODING_TYPE_AAC				0x6
#define DPTX_AUDIO_CODING_TYPE_DTS				0x7
#define DPTX_AUDIO_CODING_TYPE_ATRAC			0x8
#define DPTX_AUDIO_CODING_TYPE_ONE_BIT_AUDIO	0x9
#define DPTX_AUDIO_CODING_TYPE_E_AC3			0xA
#define DPTX_AUDIO_CODING_TYPE_DTS_HD			0xB
#define DPTX_AUDIO_CODING_TYPE_MAT				0xC
#define DPTX_AUDIO_CODING_TYPE_DST				0xD
#define DPTX_AUDIO_CODING_TYPE_WMA_PRO		0xE
#define DPTX_AUDIO_CODING_TYPE_REFER_TO_CXT	0xF
#define DPTX_AUDIO_CODING_TYPE_SHIFT			4
#define DPTX_AUDIO_CODING_TYPE_MASK			GENMASK(7, 4)

#define DPTX_AUDIO_SAMPLE_SIZE_16BIT			0x1
#define DPTX_AUDIO_SAMPLE_SIZE_20BIT			0x2
#define DPTX_AUDIO_SAMPLE_SIZE_24BIT			0x3
#define DPTX_AUDIO_SAMPLE_SIZE_SHIFT			8
#define DPTX_AUDIO_SAMPLE_SIZE_MASK			GENMASK(9, 8)

#define DPTX_AUDIO_SAMPLE_FREQ_32K				0x1
#define DPTX_AUDIO_SAMPLE_FREQ_44_1K			0x2
#define DPTX_AUDIO_SAMPLE_FREQ_48K				0x3
#define DPTX_AUDIO_SAMPLE_FREQ_88_2K			0x4
#define DPTX_AUDIO_SAMPLE_FREQ_96K				0x5
#define DPTX_AUDIO_SAMPLE_FREQ_176_4K			0x6
#define DPTX_AUDIO_SAMPLE_FREQ_192K			0x7
#define DPTX_AUDIO_SAMPLE_FREQ_SHIFT			10
#define DPTX_AUDIO_SAMPLE_FREQ_MASK			GENMASK(12, 10)

/* Data Bytes 4 and 5 apply only to multi-channel (i.e., more than two channels) Uncompressed Audio */
/* Data Byte 4 contains information that describes how various speaker locations are allocated to
 * transmission channels.
 */
#define DPTX_AUDIO_SPEAKER_MAPPING_2CH			0x0 /* - - - - - FR FL */
#define DPTX_AUDIO_SPEAKER_MAPPING_3CH			0x2 /* - - - - FC - FR FL */
#define DPTX_AUDIO_SPEAKER_MAPPING_4CH			0x3 /* - - - - FC LFE FR FL */
#define DPTX_AUDIO_SPEAKER_MAPPING_5CH			0x7 /* - - - RC FC LFE FR FL */
#define DPTX_AUDIO_SPEAKER_MAPPING_6CH			0xB /* - - RR RL FC LFE FR FL */
#define DPTX_AUDIO_SPEAKER_MAPPING_7CH			0xF /* - RC RR RL FC LFE FR FL */
#define DPTX_AUDIO_SPEAKER_MAPPING_8CH			0x13 /* RRC RLC RR RL FC LFE FR FL */
#define DPTX_AUDIO_SPEAKER_MAPPING_SHIFT		24
#define DPTX_AUDIO_SPEAKER_MAPPING_MASK		GENMASK(31, 24)

struct video_bpc_mapping {
	enum pixel_enc_type pix_enc;
	u8 bpc;
	u8 bpc_mapping;
};

struct default_colordep_map {
	enum color_depth bpc;
	int default_colordep;
};

struct colordep_map {
	enum color_depth bpc;
	int encoding;
	int colordep;
};

u8 dptx_get_sink_bpc_mapping(enum pixel_enc_type pix_enc, u8 bpc);
int dptx_get_color_depth(int bpc, int encoding);
int dptx_br_to_link_clk(int rate);
int dptx_br_to_link_rate(int rate);

u32 audio_freq_to_infoframe_data(u8 orig_sample_freq, u8 sample_freq);
u32 audio_num_channels_to_infoframe_data(u8 num_channels);
bool dptx_check_low_temperature(struct dp_ctrl *dptx);
int dptx_change_video_mode_user(struct dp_ctrl *dptx);
int dptx_video_mode_change(struct dp_ctrl *dptx, u8 vmode);

bool dptx_sink_enabled_ssc(struct dp_ctrl *dptx);

#endif // DP_AVGEN_BASE_H

