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
#include "avgen.h"
#include "../hisi_fb.h"
#include "../hisi_fb_def.h"

#define FHD_TIMING_H_ACTIVE 1920
#define FHD_TIMING_V_ACTIVE 1080

/*lint -save -e* */
static inline uint8_t dptx_bit_field(const uint16_t data, uint8_t shift, uint8_t width)
{
	return ((data >> shift) & ((((uint16_t)1) << width) - 1));
}

uint16_t dptx_concat_bits(uint8_t bhi, uint8_t ohi, uint8_t nhi, uint8_t blo, uint8_t olo, uint8_t nlo)
{
	return (dptx_bit_field(bhi, ohi, nhi) << nlo) |
		dptx_bit_field(blo, olo, nlo);
}

uint16_t dptx_byte_to_word(const uint8_t hi, const uint8_t lo)
{
	return dptx_concat_bits(hi, 0, 8, lo, 0, 8);
}

uint32_t dptx_byte_to_dword(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)
{
	uint32_t retval = 0;

	retval = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
	return retval;
}

int dptx_dtd_parse(struct dp_ctrl *dptx, struct dtd *mdtd, uint8_t data[18])
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	mdtd->pixel_repetition_input = 0;

	mdtd->pixel_clock = dptx_byte_to_word(data[1], data[0]);
	if (mdtd->pixel_clock < 0x01)
		return -EINVAL;

	mdtd->h_active = dptx_concat_bits(data[4], 4, 4, data[2], 0, 8);
	mdtd->h_blanking = dptx_concat_bits(data[4], 0, 4, data[3], 0, 8);
	mdtd->h_sync_offset = dptx_concat_bits(data[11], 6, 2, data[8], 0, 8);
	mdtd->h_sync_pulse_width = dptx_concat_bits(data[11], 4, 2, data[9],
							0, 8);
	mdtd->h_image_size = dptx_concat_bits(data[14], 4, 4, data[12], 0, 8);

	mdtd->v_active = dptx_concat_bits(data[7], 4, 4, data[5], 0, 8);
	mdtd->v_blanking = dptx_concat_bits(data[7], 0, 4, data[6], 0, 8);
	mdtd->v_sync_offset = dptx_concat_bits(data[11], 2, 2, data[10], 4, 4);
	mdtd->v_sync_pulse_width = dptx_concat_bits(data[11], 0, 2,
							data[10], 0, 4);
	mdtd->v_image_size = dptx_concat_bits(data[14], 0, 4, data[13], 0, 8);
	if (dptx_bit_field(data[17], 4, 1) != 1)
		return -EINVAL;
	if (dptx_bit_field(data[17], 3, 1) != 1)
		return -EINVAL;

	mdtd->interlaced = dptx_bit_field(data[17], 7, 1) == 1;
	mdtd->v_sync_polarity = dptx_bit_field(data[17], 2, 1) == 0;
	mdtd->h_sync_polarity = dptx_bit_field(data[17], 1, 1) == 0;
	if (mdtd->interlaced == 1)
		mdtd->v_active /= 2;
	HISI_FB_INFO("[DP] DTD pixel_clock: %llu interlaced: %d\n",
		 mdtd->pixel_clock, mdtd->interlaced);
	HISI_FB_INFO("[DP] h_active: %d h_blanking: %d h_sync_offset: %d\n",
		 mdtd->h_active, mdtd->h_blanking, mdtd->h_sync_offset);
	HISI_FB_INFO("[DP] h_sync_pulse_width: %d h_image_size: %d h_sync_polarity: %d\n",
		 mdtd->h_sync_pulse_width, mdtd->h_image_size,
		 mdtd->h_sync_polarity);
	HISI_FB_INFO("[DP] v_active: %d v_blanking: %d v_sync_offset: %d\n",
		 mdtd->v_active, mdtd->v_blanking, mdtd->v_sync_offset);
	HISI_FB_INFO("[DP] v_sync_pulse_width: %d v_image_size: %d v_sync_polarity: %d\n",
		 mdtd->v_sync_pulse_width, mdtd->v_image_size,
		 mdtd->v_sync_polarity);
	mdtd->pixel_clock *= 10;
	return 0;
}

void dptx_audio_sdp_en(struct dp_ctrl *dptx)
{
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_STREAM_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
	reg |= DPTX_EN_AUDIO_STREAM_SDP;
	dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL, reg);
}

void dptx_audio_timestamp_sdp_en(struct dp_ctrl *dptx)
{
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_TIMESTAMP_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}

static uint8_t dptx_audio_get_sample_freq_cfg(struct audio_params *aparams)
{
	uint8_t iec_orig_samp_freq = 0;
	uint8_t iec_samp_freq = 0;
	uint8_t sample_freq_cfg = 0;

	iec_orig_samp_freq = aparams->iec_orig_samp_freq;
	iec_samp_freq = aparams->iec_samp_freq;

	if (iec_orig_samp_freq == IEC_ORIG_SAMP_FREQ_32K && iec_samp_freq == IEC_SAMP_FREQ_32K) {
		sample_freq_cfg = DPTX_AUDIO_SAMPLE_FREQ_32K;
	} else if (iec_orig_samp_freq == IEC_ORIG_SAMP_FREQ_48K && iec_samp_freq == IEC_SAMP_FREQ_48K) {
		sample_freq_cfg = DPTX_AUDIO_SAMPLE_FREQ_48K;
	} else if (iec_orig_samp_freq == IEC_ORIG_SAMP_FREQ_96K && iec_samp_freq == IEC_SAMP_FREQ_96K) {
		sample_freq_cfg = DPTX_AUDIO_SAMPLE_FREQ_96K;
	} else if (iec_orig_samp_freq == IEC_ORIG_SAMP_FREQ_192K && iec_samp_freq == IEC_SAMP_FREQ_192K) {
		sample_freq_cfg = DPTX_AUDIO_SAMPLE_FREQ_192K;
	} else {
		sample_freq_cfg = DPTX_AUDIO_REFER_TO_STREAM_HEADER;
	}

	return sample_freq_cfg;
}

static uint8_t dptx_audio_get_data_width_cfg(struct audio_params *aparams)
{
	uint8_t data_width_cfg = 0;

	if (aparams->data_width == 16) {
		data_width_cfg = DPTX_AUDIO_SAMPLE_SIZE_16BIT;
	} else if (aparams->data_width == 24) {
		data_width_cfg = DPTX_AUDIO_SAMPLE_SIZE_24BIT;
	} else {
		data_width_cfg = DPTX_AUDIO_REFER_TO_STREAM_HEADER;
	}

	return data_width_cfg;
}

static uint8_t dptx_audio_get_num_channels_cfg(struct audio_params *aparams)
{
	uint8_t num_channels_cfg = 0;

	if (aparams->num_channels == 2) {
		num_channels_cfg = DPTX_AUDIO_CHANNEL_CNT_2CH;
	} else if (aparams->num_channels == 8) {
		num_channels_cfg = DPTX_AUDIO_CHANNEL_CNT_8CH;
	} else {
		num_channels_cfg = DPTX_AUDIO_REFER_TO_STREAM_HEADER;
	}

	return num_channels_cfg;
}

static uint8_t dptx_audio_get_speaker_map_cfg(struct audio_params *aparams)
{
	uint8_t speaker_map_cfg = 0;

	if (aparams->num_channels == 2) {
		speaker_map_cfg = DPTX_AUDIO_SPEAKER_MAPPING_2CH;
	} else {
		speaker_map_cfg = DPTX_AUDIO_SPEAKER_MAPPING_8CH;
	}

	return speaker_map_cfg;
}
void dptx_audio_infoframe_sdp_send(struct dp_ctrl *dptx)
{
	uint32_t reg = 0;
	uint8_t sample_freq_cfg = 0;
	uint8_t data_width_cfg = 0;
	uint8_t num_channels_cfg = 0;
	uint8_t speaker_map_cfg = 0;
	uint32_t audio_infoframe_header = AUDIO_INFOFREAME_HEADER;
	uint32_t audio_infoframe_data[3] = {0x0, 0x0, 0x0};

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	sample_freq_cfg = dptx_audio_get_sample_freq_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= sample_freq_cfg << DPTX_AUDIO_SAMPLE_FREQ_SHIFT;

	data_width_cfg = dptx_audio_get_data_width_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= data_width_cfg << DPTX_AUDIO_SAMPLE_SIZE_SHIFT;

	num_channels_cfg = dptx_audio_get_num_channels_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= num_channels_cfg << DPTX_AUDIO_CHANNEL_CNT_SHIFT;

	speaker_map_cfg = dptx_audio_get_speaker_map_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= speaker_map_cfg << DPTX_AUDIO_SPEAKER_MAPPING_SHIFT;

	dptx->sdp_list[0].payload[0] = audio_infoframe_header;
	dptx_writel(dptx, DPTX_SDP_BANK, audio_infoframe_header);
	/* Synosys FAE luheng:
		set reg offset 0x604 to all zero. When infoframe is zero, sink just check stream head.
		Otherwire sink would checkout if inforame equal stream head info */
	dptx_writel(dptx, DPTX_SDP_BANK + 4, audio_infoframe_data[0]);
	dptx_writel(dptx, DPTX_SDP_BANK + 8, audio_infoframe_data[1]);
	dptx_writel(dptx, DPTX_SDP_BANK + 12, audio_infoframe_data[2]);

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_INFOFRAME_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}

void dptx_disable_sdp(struct dp_ctrl *dptx, uint32_t *payload)
{
	int i;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (payload == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	for (i = 0; i < DPTX_SDP_NUM; i++)
		if (!memcmp(dptx->sdp_list[i].payload, payload, 9))
			memset(dptx->sdp_list[i].payload, 0, 9);
}

void dptx_enable_sdp(struct dp_ctrl *dptx, struct sdp_full_data *data)
{
	int i;
	uint32_t reg;
	int reg_num;
	uint32_t header;
	int sdp_offset;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (data == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg_num = 0;
	header = cpu_to_be32(data->payload[0]);
	for (i = 0; i < DPTX_SDP_NUM; i++)
		if (dptx->sdp_list[i].payload[0] == 0) {
			dptx->sdp_list[i].payload[0] = header;
			sdp_offset = i * DPTX_SDP_SIZE;
			reg_num = 0;
			while (reg_num < DPTX_SDP_LEN) {
				dptx_writel(dptx, DPTX_SDP_BANK + sdp_offset
					    + reg_num * 4,
					    cpu_to_be32(
							data->payload[reg_num])
					    );
				reg_num++;
			}
			switch (data->blanking) {
			case 0:
				reg = dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
				break;
			case 1:
				reg = dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL,
					    reg);
				break;
			case 2:
				reg = dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
				reg = dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL,
					    reg);
				break;
			}
			break;
		}
}

void dptx_fill_sdp(struct dp_ctrl *dptx, struct sdp_full_data *data)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (data == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (data->en == 1)
		dptx_enable_sdp(dptx, data);
	else
		dptx_disable_sdp(dptx, data->payload);
}

void dptx_en_audio_channel(struct dp_ctrl *dptx, int ch_num, int enable)
{
	uint32_t reg;
	uint32_t data_en = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_DATA_EN_IN_MASK;

	if (enable) {
		switch (ch_num) {
			case 1:
				data_en = DPTX_EN_AUDIO_CH_1;
				break;
			case 2:
				data_en = DPTX_EN_AUDIO_CH_2;
				break;
			case 3:
				data_en = DPTX_EN_AUDIO_CH_3;
				break;
			case 4:
				data_en = DPTX_EN_AUDIO_CH_4;
				break;
			case 5:
				data_en = DPTX_EN_AUDIO_CH_5;
				break;
			case 6:
				data_en = DPTX_EN_AUDIO_CH_6;
				break;
			case 7:
				data_en = DPTX_EN_AUDIO_CH_7;
				break;
			case 8:
				data_en = DPTX_EN_AUDIO_CH_8;
				break;
			default:
				break;
		}
		reg |= data_en << DPTX_AUD_CONFIG1_DATA_EN_IN_SHIFT;
	} else {
		switch (ch_num) {
			case 1:
				data_en = DPTX_EN_AUDIO_CH_1;
				break;
			case 2:
				data_en = DPTX_EN_AUDIO_CH_2;
				break;
			case 3:
				data_en = DPTX_EN_AUDIO_CH_3;
				break;
			case 4:
				data_en = DPTX_EN_AUDIO_CH_4;
				break;
			case 5:
				data_en = DPTX_EN_AUDIO_CH_5;
				break;
			case 6:
				data_en = DPTX_EN_AUDIO_CH_6;
				break;
			case 7:
				data_en = DPTX_EN_AUDIO_CH_7;
				break;
			case 8:
				data_en = DPTX_EN_AUDIO_CH_8;
				break;
			default:
				break;
		}
		reg &= ~(data_en << DPTX_AUD_CONFIG1_DATA_EN_IN_SHIFT);
	}
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_video_reset(struct dp_ctrl *dptx, int enable, int stream)
{
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_SRST_CTRL);
	if (enable)
		reg |= DPTX_SRST_VIDEO_RESET_N(stream);
	else
		reg &= ~DPTX_SRST_VIDEO_RESET_N(stream);
	dptx_writel(dptx, DPTX_SRST_CTRL, reg);
}

void dptx_audio_mute(struct dp_ctrl *dptx)
{
	uint32_t reg;
	struct audio_params *aparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;
	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);

	if (aparams->mute == 1)
		reg |= DPTX_AUDIO_MUTE;
	else
		reg &= ~DPTX_AUDIO_MUTE;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_config(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_audio_core_config(dptx);
	dptx_audio_sdp_en(dptx);
	dptx_audio_timestamp_sdp_en(dptx);

	dptx_audio_infoframe_sdp_send(dptx);
}

void dptx_audio_core_config(struct dp_ctrl *dptx)
{
	struct audio_params *aparams;
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;

	dptx_audio_inf_type_change(dptx);
	dptx_audio_num_ch_change(dptx);
	dptx_audio_data_width_change(dptx);

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_ATS_VER_MASK;
	reg |= aparams->ats_ver << (unsigned int)DPTX_AUD_CONFIG1_ATS_VER_SHFIT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);

	dptx_en_audio_channel(dptx, aparams->num_channels, 1);
}


void dptx_audio_inf_type_change(struct dp_ctrl *dptx)
{
	struct audio_params *aparams;
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_INF_TYPE_MASK;
	reg |= aparams->inf_type << (unsigned int)DPTX_AUD_CONFIG1_INF_TYPE_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_num_ch_change(struct dp_ctrl *dptx)
{
	uint32_t reg = 0;
	uint32_t num_ch_map = 0;
	struct audio_params *aparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_NCH_MASK;

	if (aparams->num_channels > 0 && aparams->num_channels <= 8) {
		num_ch_map = aparams->num_channels - 1;
	} else {
		num_ch_map = DPTX_AUD_CONFIG1_NCH_DEFAULT_VALUE;
	}

	reg |= num_ch_map << (unsigned int)DPTX_AUD_CONFIG1_NCH_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_data_width_change(struct dp_ctrl *dptx)
{
	uint32_t reg;
	struct audio_params *aparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_DATA_WIDTH_MASK;
	reg |= aparams->data_width << (unsigned int)DPTX_AUD_CONFIG1_DATA_WIDTH_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

bool dptx_check_low_temperature(struct dp_ctrl *dptx)
{
	uint32_t perictrl4;
	struct hisi_fb_data_type *hisifd;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return FALSE;
	}

	hisifd = dptx->hisifd;

	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return FALSE;
	}

	perictrl4 = inp32(hisifd->pmctrl_base + MIDIA_PERI_CTRL4);
	perictrl4 &= PMCTRL_PERI_CTRL4_TEMPERATURE_MASK;
	perictrl4 = (perictrl4 >> PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT);
	HISI_FB_INFO("[DP] Get current temperature: %d \n", perictrl4);

	if (perictrl4 != NORMAL_TEMPRATURE)
		return TRUE;
	else
		return FALSE;
}
/*
 * Video Generation
 */
void dptx_video_timing_change(struct dp_ctrl *dptx, int stream)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_disable_default_video_stream(dptx, stream);
	dptx_video_core_config(dptx, stream);
	dptx_video_ts_change(dptx, stream);
	dptx_enable_default_video_stream(dptx, stream);
}

int dptx_change_video_mode_tu_fail(struct dp_ctrl *dptx)
{
	struct dtd *mdtd;
	mdtd = &dptx->vparams.mdtd;

	if ((mdtd->pixel_clock > 500000) && (mdtd->h_active >= 3840)) {	/*4k 60HZ*/
		if (((dptx->link.lanes == 2) && (dptx->link.rate == DPTX_PHYIF_CTRL_RATE_HBR2)) ||
			((dptx->link.lanes == 4) && (dptx->link.rate == DPTX_PHYIF_CTRL_RATE_HBR)))
			return 95;	/*4k 30HZ*/
	}
	if(mdtd->h_active > 1920)
		return 16;/*1920*1080*/
	if(mdtd->h_active > 1280)
		return 4;/*1280*720*/
	if(mdtd->h_active > 640)
		return 1;/*640*480*/
	return 1;
}

int dptx_change_video_mode_user(struct dp_ctrl *dptx)
{
	struct video_params *vparams;
	int retval;
	bool needchanged;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	needchanged = FALSE;
	if (!dptx->same_source) {
		if((vparams->mdtd.h_active > FHD_TIMING_H_ACTIVE) || (vparams->mdtd.v_active > FHD_TIMING_V_ACTIVE)) {
			vparams->video_format = VCEA;
			vparams->mode = 16; /*Siwtch to 1080p on PC mode*/
			needchanged = TRUE;
			HISI_FB_INFO("[DP] Video mode is changed by different source!\n");
		}
	}

	if(dptx->user_mode != 0) {
		vparams->video_format = dptx->user_mode_format;
		vparams->mode = dptx->user_mode; /*Siwtch to user setting*/
		needchanged = TRUE;
		HISI_FB_INFO("[DP] Video mode is changed by user setting!\n");
	}

	if (needchanged) {
		retval = dptx_video_mode_change(dptx, vparams->mode, 0);
		if (retval) {
			HISI_FB_ERR("[DP] Change mode error!\n");
			return retval;
		}
	}

	if (dptx_check_low_temperature(dptx)) {
		if((vparams->mdtd.h_active > FHD_TIMING_H_ACTIVE) || (vparams->mdtd.v_active > FHD_TIMING_V_ACTIVE)) {
			vparams->video_format = VCEA;
			vparams->mode = 16; /*Siwtch to 1080p on PC mode*/
			HISI_FB_INFO("[DP] Video mode is changed by low temperature!\n");

			retval = dptx_video_mode_change(dptx, vparams->mode, 0);
			if (retval) {
				HISI_FB_ERR("[DP] Change mode error!\n");
				return retval;
			}
		}
	}

	dp_imonitor_set_param(DP_PARAM_SOURCE_MODE, &(dptx->same_source));
	dp_imonitor_set_param(DP_PARAM_USER_MODE,   &(vparams->mode));
	dp_imonitor_set_param(DP_PARAM_USER_FORMAT, &(vparams->video_format));
	return 0;
}

int dptx_video_mode_change(struct dp_ctrl *dptx, uint8_t vmode, int stream)
{
	int retval;
	struct video_params *vparams;
	struct dtd mdtd;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}
	vparams = &dptx->vparams;
	vparams->mode = vmode;

	if (!dptx_dtd_fill(&mdtd, vparams->mode, vparams->refresh_rate,
			   vparams->video_format)) {
		HISI_FB_ERR("[DP] Invalid video mode value %d\n",
						vparams->mode);
		return -EINVAL;
	}
	vparams->mdtd = mdtd;
	retval = dptx_video_ts_calculate(dptx, dptx->link.lanes,
					 dptx->link.rate, vparams->bpc,
					 vparams->pix_enc, mdtd.pixel_clock);

	HISI_FB_INFO("[DP] The mode is changed as [%d]\n", vparams->mode);

	return retval;
}

int dptx_video_config(struct dp_ctrl *dptx, int stream)
{
	struct video_params *vparams;
	struct dtd *mdtd;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	if (!dptx_dtd_fill(mdtd, vparams->mode,
			   vparams->refresh_rate, vparams->video_format))
		return -EINVAL;
	dptx_video_core_config(dptx, stream);
	return 0;
}
void dptx_video_core_config(struct dp_ctrl *dptx, int stream)
{
	struct video_params *vparams;
	struct dtd *mdtd;
	uint32_t reg;
	uint8_t vmode;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	vmode = vparams->mode;

	dptx_video_set_core_bpc(dptx, stream);
	/* Configure DPTX_VSAMPLE_POLARITY_CTRL register */
	reg = 0;

	if (mdtd->h_sync_polarity == 1)
		reg |= DPTX_POL_CTRL_H_SYNC_POL_EN;
	if (mdtd->v_sync_polarity == 1)
		reg |= DPTX_POL_CTRL_V_SYNC_POL_EN;

	dptx_writel(dptx, DPTX_VSAMPLE_POLARITY_CTRL_N(stream), reg);

	reg = 0;

	/* Configure video_config1 register */
	if (vparams->video_format == VCEA) {
		if (vmode == 5 || vmode == 6 || vmode == 7 ||
		    vmode == 10 || vmode == 11 || vmode == 20 ||
		    vmode == 21 || vmode == 22 || vmode == 39 ||
		    vmode == 25 || vmode == 26 || vmode == 40 ||
		    vmode == 44 || vmode == 45 || vmode == 46 ||
		    vmode == 50 || vmode == 51 || vmode == 54 ||
		    vmode == 55 || vmode == 58 || vmode  == 59)
			reg |= DPTX_VIDEO_CONFIG1_IN_OSC_EN;
	}

	if (mdtd->interlaced == 1)
		reg |= DPTX_VIDEO_CONFIG1_O_IP_EN;

	reg |= mdtd->h_active << DPTX_VIDEO_H_ACTIVE_SHIFT;
	reg |= mdtd->h_blanking << DPTX_VIDEO_H_BLANK_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG1_N(stream), reg);

	/* Configure video_config2 register */
	reg = 0;
	reg |= mdtd->v_active << DPTX_VIDEO_V_ACTIVE_SHIFT;
	reg |= mdtd->v_blanking << DPTX_VIDEO_V_BLANK_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG2_N(stream), reg);

	/* Configure video_config3 register */
	reg = 0;
	reg |= mdtd->h_sync_offset << DPTX_VIDEO_H_FRONT_PORCH;
	reg |= mdtd->h_sync_pulse_width << DPTX_VIDEO_H_SYNC_WIDTH;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG3_N(stream), reg);

	/* Configure video_config4 register */
	reg = 0;
	reg |= mdtd->v_sync_offset << DPTX_VIDEO_V_FRONT_PORCH;
	reg |= mdtd->v_sync_pulse_width << DPTX_VIDEO_V_SYNC_WIDTH;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG4_N(stream), reg);

	/* Configure video_config5 register */
	dptx_video_ts_change(dptx, stream);

	/* Configure video_msa1 register */
	reg = 0;
	reg |= (mdtd->h_blanking - mdtd->h_sync_offset)
		<< DPTX_VIDEO_MSA1_H_START_SHIFT;
	reg |= (mdtd->v_blanking - mdtd->v_sync_offset)
		<< DPTX_VIDEO_MSA1_V_START_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_MSA1_N(stream), reg);

	dptx_video_set_sink_bpc(dptx, stream);
}


int dptx_video_ts_calculate(struct dp_ctrl *dptx, int lane_num, int rate,
			    int bpc, int encoding, int pixel_clock)
{
	struct video_params *vparams;
	struct dtd *mdtd;
	int link_rate;
	int retval = 0;
	int ts;
	int tu;
	int tu_frac;
	int color_dep;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		link_rate = 162;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		link_rate = 270;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		link_rate = 540;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		link_rate = 810;
		break;
	default:
		link_rate = 162;
		break;
	}

	switch (bpc) {
	case COLOR_DEPTH_6:
		color_dep = 18;
		break;
	case COLOR_DEPTH_8:
		if (encoding == YCBCR420)
			color_dep  = 12;
		else if (encoding == YCBCR422)
			color_dep = 16;
		else if (encoding == YONLY)
			color_dep = 8;
		else
			color_dep = 24;
		break;
	case COLOR_DEPTH_10:
		if (encoding == YCBCR420)
			color_dep = 15;
		else if (encoding == YCBCR422)
			color_dep = 20;
		else if (encoding  == YONLY)
			color_dep = 10;
		else
			color_dep = 30;
		break;

	case COLOR_DEPTH_12:
		if (encoding == YCBCR420)
			color_dep = 18;
		else if (encoding == YCBCR422)
			color_dep = 24;
		else if (encoding == YONLY)
			color_dep = 12;
		else
			color_dep = 36;
		break;

	case COLOR_DEPTH_16:
		if (encoding == YCBCR420)
			color_dep = 24;
		else if (encoding == YCBCR422)
			color_dep = 32;
		else if (encoding == YONLY)
			color_dep = 16;
		else
			color_dep = 48;
		break;
	default:
		color_dep = 18;
		break;
	}

	if (lane_num * link_rate == 0) {
		HISI_FB_ERR("[DP] lane_num = %d, link_rate = %d", lane_num, link_rate);
		return -EINVAL;
	}

	ts = (8 * color_dep * pixel_clock) / (lane_num * link_rate);
	tu  = ts / 1000;
	dp_imonitor_set_param(DP_PARAM_TU, &tu);

	if (tu >= 65) {
		HISI_FB_ERR("[DP] tu(%d) > 65", tu);
		return -EINVAL;
	}

	tu_frac = ts / 100 - tu * 10;

	if (tu < 6)
		vparams->init_threshold = 32;
	else if ((encoding == RGB || encoding == YCBCR444) &&
		 mdtd->h_blanking <= 80)
		vparams->init_threshold = 12;
	else
		vparams->init_threshold = 15;

	HISI_FB_INFO("[DP] tu = %d\n", tu);

	vparams->aver_bytes_per_tu = (uint8_t)tu;

	vparams->aver_bytes_per_tu_frac = (uint8_t)tu_frac;

	return retval;
}

void dptx_video_ts_change(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg;
	struct video_params *vparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_VIDEO_CONFIG5_N(stream));
	reg = reg & (~DPTX_VIDEO_CONFIG5_TU_MASK);
	reg = reg | (vparams->aver_bytes_per_tu <<
			DPTX_VIDEO_CONFIG5_TU_SHIFT);
	reg = reg & (~DPTX_VIDEO_CONFIG5_TU_FRAC_MASK_SST);
	reg = reg | (vparams->aver_bytes_per_tu_frac <<
		       DPTX_VIDEO_CONFIG5_TU_FRAC_SHIFT_SST);

	reg = reg & (~DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_MASK);
	reg = reg | (vparams->init_threshold <<
			DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_SHIFT);

	dptx_writel(dptx, DPTX_VIDEO_CONFIG5_N(stream), reg);

	return;
}


void dptx_video_bpc_change(struct dp_ctrl *dptx, int stream)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_video_set_core_bpc(dptx, stream);
	dptx_video_set_sink_bpc(dptx, stream);
}

void dptx_video_set_core_bpc(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg;
	uint8_t bpc_mapping = 0, bpc = 0;
	enum pixel_enc_type pix_enc;
	struct video_params *vparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	bpc = vparams->bpc;
	pix_enc = vparams->pix_enc;

	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	reg &= ~DPTX_VSAMPLE_CTRL_VMAP_BPC_MASK;

	switch (pix_enc) {
	case RGB:
		if (bpc == COLOR_DEPTH_6)
			bpc_mapping = 0;
		else if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case YCBCR444:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 5;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 6;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 7;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 8;
		break;
	case YCBCR422:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 9;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 10;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 11;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 12;
		break;
	case YCBCR420:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 13;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 14;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 15;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 16;
		break;
	case YONLY:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 17;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 18;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 19;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 20;
		break;
	case RAW:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 23;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 24;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 25;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 27;
		break;
	}

	reg |= (bpc_mapping << DPTX_VSAMPLE_CTRL_VMAP_BPC_SHIFT);
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), reg);
}
void dptx_video_set_sink_col(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg_msa2;
	uint8_t col_mapping;
	uint8_t colorimetry;
	uint8_t dynamic_range;
	struct video_params *vparams;
	enum pixel_enc_type pix_enc;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	pix_enc = vparams->pix_enc;
	colorimetry = vparams->colorimetry;
	dynamic_range = vparams->dynamic_range;

	reg_msa2 = dptx_readl(dptx, DPTX_VIDEO_MSA2_N(stream));
	reg_msa2 &= ~DPTX_VIDEO_VMSA2_COL_MASK;

	col_mapping = 0;

	/* According to Table 2-94 of DisplayPort spec 1.3 */
	switch (pix_enc) {
	case RGB:
		if (dynamic_range == CEA)
			col_mapping = 4;
		else if (dynamic_range == VESA)
			col_mapping = 0;
		break;
	case YCBCR422:
		if (colorimetry == ITU601)
			col_mapping = 5;
		else if (colorimetry == ITU709)
			col_mapping = 13;
		break;
	case YCBCR444:
		if (colorimetry == ITU601)
			col_mapping = 6;
		else if (colorimetry == ITU709)
			col_mapping = 14;
		break;
	case RAW:
		col_mapping = 1;
		break;
	case YCBCR420:
	case YONLY:
		break;
	}

	reg_msa2 |= (col_mapping << DPTX_VIDEO_VMSA2_COL_SHIFT);
	dptx_writel(dptx, DPTX_VIDEO_MSA2_N(stream), reg_msa2);
}

void dptx_video_set_sink_bpc(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg_msa2, reg_msa3;
	uint8_t bpc_mapping = 0, bpc = 0;
	struct video_params *vparams;
	enum pixel_enc_type pix_enc;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	pix_enc = vparams->pix_enc;
	bpc = vparams->bpc;

	reg_msa2 = dptx_readl(dptx, DPTX_VIDEO_MSA2_N(stream));
	reg_msa3 = dptx_readl(dptx, DPTX_VIDEO_MSA3_N(stream));

	reg_msa2 &= ~DPTX_VIDEO_VMSA2_BPC_MASK;
	reg_msa3 &= ~DPTX_VIDEO_VMSA3_PIX_ENC_MASK;

	switch (pix_enc) {
	case RGB:
		if (bpc == COLOR_DEPTH_6)
			bpc_mapping = 0;
		else if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case YCBCR444:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case YCBCR422:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case YCBCR420:
		reg_msa3 |= 1 << DPTX_VIDEO_VMSA3_PIX_ENC_YCBCR420_SHIFT;
		break;
	case YONLY:
		/* According to Table 2-94 of DisplayPort spec 1.3 */
		reg_msa3 |= 1 << DPTX_VIDEO_VMSA3_PIX_ENC_SHIFT;

		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case RAW:
		 /* According to Table 2-94 of DisplayPort spec 1.3 */
		reg_msa3 |= (1 << DPTX_VIDEO_VMSA3_PIX_ENC_SHIFT);

		if (bpc == COLOR_DEPTH_6)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 3;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 4;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 5;
		else if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 7;
		break;
	default:
		break;
	}
	reg_msa2 |= (bpc_mapping << DPTX_VIDEO_VMSA2_BPC_SHIFT);

	dptx_writel(dptx, DPTX_VIDEO_MSA2_N(stream), reg_msa2);
	dptx_writel(dptx, DPTX_VIDEO_MSA3_N(stream), reg_msa3);
	dptx_video_set_sink_col(dptx, stream);
}


void dptx_disable_default_video_stream(struct dp_ctrl *dptx, int stream)
{
	uint32_t vsamplectrl;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vsamplectrl = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	vsamplectrl &= ~DPTX_VSAMPLE_CTRL_STREAM_EN;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), vsamplectrl);

	if ((dptx->dptx_vr) && (dptx->dptx_detect_inited)) {
		HISI_FB_INFO("[DP] Cancel dptx detect err count when disable video stream.\n");
		hrtimer_cancel(&dptx->dptx_hrtimer);
	}
}

void dptx_enable_default_video_stream(struct dp_ctrl *dptx, int stream)
{
	uint32_t vsamplectrl;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vsamplectrl = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	vsamplectrl |= DPTX_VSAMPLE_CTRL_STREAM_EN;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), vsamplectrl);

	if ((dptx->dptx_vr) && (dptx->dptx_detect_inited)) {
		HISI_FB_INFO("[DP] restart dptx detect err count when enable video stream.\n");
		hrtimer_restart(&dptx->dptx_hrtimer);
	}
}

/*
 * Audio/Video Parameters
 */

void dptx_audio_params_reset(struct audio_params *params)
{
	if (params == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	memset(params, 0x0, sizeof(struct audio_params));
	params->iec_channel_numcl0 = 8;
	params->iec_channel_numcr0 = 4;
	params->use_lut = 1;
	params->iec_samp_freq = 3;
	params->iec_word_length = 11;
	params->iec_orig_samp_freq = 12;
	params->data_width = 16;
	params->num_channels = 2;
	params->inf_type = 1;
	params->ats_ver = 17;
	params->mute = 0;
}

void dptx_video_params_reset(struct video_params *params)
{
	if (params == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	memset(params, 0x0, sizeof(struct video_params));
	/* TODO 6 bpc should be default - use 8 bpc for MST calculation */
	params->bpc = COLOR_DEPTH_8;
	params->pix_enc = RGB;
	params->mode = 1;
	params->colorimetry = ITU601;
	params->dynamic_range = CEA;
	params->aver_bytes_per_tu = 30;
	params->aver_bytes_per_tu_frac = 0;
	params->init_threshold = 15;
	params->pattern_mode = RAMP;
	params->refresh_rate = 60000;
	params->video_format = VCEA;
}

/*
 * DTD
 */

void dwc_dptx_dtd_reset(struct dtd *mdtd)
{
	if (mdtd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	mdtd->pixel_repetition_input = 0;
	mdtd->pixel_clock  = 0;
	mdtd->h_active = 0;
	mdtd->h_blanking = 0;
	mdtd->h_sync_offset = 0;
	mdtd->h_sync_pulse_width = 0;
	mdtd->h_image_size = 0;
	mdtd->v_active = 0;
	mdtd->v_blanking = 0;
	mdtd->v_sync_offset = 0;
	mdtd->v_sync_pulse_width = 0;
	mdtd->v_image_size = 0;
	mdtd->interlaced = 0;
	mdtd->v_sync_polarity = 0;
	mdtd->h_sync_polarity = 0;
}

bool dptx_dtd_fill(struct dtd *mdtd, uint8_t code, uint32_t refresh_rate,
		  uint8_t video_format)
{
	if (mdtd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return false;
	}

	dwc_dptx_dtd_reset(mdtd);

	mdtd->h_image_size = 16;
	mdtd->v_image_size = 9;

	if (video_format == VCEA) {
		switch (code) {
		case 1: /* 640x480p @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 640;
			mdtd->v_active = 480;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 10;
			mdtd->h_sync_pulse_width = 96;
			mdtd->v_sync_pulse_width = 2;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 25175;
			break;
		case 2: /* 720x480p @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 3: /* 720x480p @ 59.94/60Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 480;
			mdtd->h_blanking = 138;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 62;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 27000;
			break;
		case 69:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 4: /* 1280x720p @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 370;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 110;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 5: /* 1920x1080i @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 74250;
			break;
		case 6: /* 720(1440)x480i @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 7: /* 720(1440)x480i @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 240;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 38;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 27000;
			break;
		case 8: /* 720(1440)x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 9: /* 720(1440)x240p @59.826/60.054/59.886/60.115Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 240;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = (refresh_rate == 59940) ? 22 : 23;
			mdtd->h_sync_offset = 38;
			mdtd->v_sync_offset = (refresh_rate == 59940) ? 4 : 5;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 27000;
			/*  else 60.115/59.886 Hz */
			break;
		case 10: /* 2880x480i @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 11: /* 2880x480i @ 59.94/60Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 240;
			mdtd->h_blanking = 552;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 76;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 248;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 54000;
			break;
		case 12: /* 2880x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 13: /* 2880x240p @ 59.826/60.054/59.886/60.115Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 240;
			mdtd->h_blanking = 552;
			mdtd->v_blanking = (refresh_rate == 60054) ? 22 : 23;
			mdtd->h_sync_offset = 76;
			mdtd->v_sync_offset = (refresh_rate == 60054) ? 4 : 5;
			mdtd->h_sync_pulse_width = 248;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 14: /* 1440x480p @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 15: /* 1440x480p @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 480;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 32;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 76:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 16: /* 1920x1080p @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 148500;
			break;
		case 17: /* 720x576p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 18: /* 720x576p @ 50Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 576;
			mdtd->h_blanking = 144;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 12;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 64;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 27000;
			break;
		case 68:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 19: /* 1280x720p @ 50Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 700;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 440;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 20: /* 1920x1080i @ 50Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 74250;
			break;
		case 21: /* 720(1440)x576i @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 22: /* 720(1440)x576i @ 50Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 288;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 126;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 27000;
			break;
		case 23: /* 720(1440)x288p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 24: /* 720(1440)x288p @ 50Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 288;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = (refresh_rate == 50080) ? 24
				: ((refresh_rate == 49920) ? 25 : 26);
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = (refresh_rate == 50080) ? 2
				: ((refresh_rate == 49920) ? 3 : 4);
			mdtd->h_sync_pulse_width = 126;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 27000;
			break;
		case 25: /* 2880x576i @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 26: /* 2880x576i @ 50Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 288;
			mdtd->h_blanking = 576;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 252;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 54000;
			break;
		case 27: /* 2880x288p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 28: /* 2880x288p @ 50Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 288;
			mdtd->h_blanking = 576;
			mdtd->v_blanking = (refresh_rate == 50080) ? 24
				: ((refresh_rate == 49920) ? 25 : 26);
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = (refresh_rate == 50080) ? 2
				: ((refresh_rate == 49920) ? 3 : 4);
			mdtd->h_sync_pulse_width = 252;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 29: /* 1440x576p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 30: /* 1440x576p @ 50Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 576;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 75:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 31: /* 1920x1080p @ 50Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 148500;
			break;
		case 72:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 32: /* 1920x1080p @ 23.976/24Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 830;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 638;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 73:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 33: /* 1920x1080p @ 25Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 74:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 34: /* 1920x1080p @ 29.97/30Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 35: /* 2880x480p @ 60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 36: /* 2880x480p @ 60Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 480;
			mdtd->h_blanking = 552;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 248;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 108000;
			break;
		case 37: /* 2880x576p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 38: /* 2880x576p @ 50Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 576;
			mdtd->h_blanking = 576;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 256;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 108000;
			break;
		case 39: /* 1920x1080i (1250 total) @ 50Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 384;
			mdtd->v_blanking = 85;
			mdtd->h_sync_offset = 32;
			mdtd->v_sync_offset = 23;
			mdtd->h_sync_pulse_width = 168;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 72000;
			break;
		case 40: /* 1920x1080i @ 100Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 148500;
			break;
		case 70:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 41: /* 1280x720p @ 100Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 700;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 440;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 148500;
			break;
		case 42: /* 720x576p @ 100Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 43: /* 720x576p @ 100Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 576;
			mdtd->h_blanking = 144;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 12;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 64;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 44: /* 720(1440)x576i @ 100Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 45: /* 720(1440)x576i @ 100Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 288;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 126;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 54000;
			break;
		case 46: /* 1920x1080i @ 119.88/120Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 148500;
			break;
		case 71:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 47: /* 1280x720p @ 119.88/120Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 370;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 110;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 148500;
			break;
		case 48: /* 720x480p @ 119.88/120Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 49: /* 720x480p @ 119.88/120Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 480;
			mdtd->h_blanking = 138;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 62;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 50: /* 720(1440)x480i @ 119.88/120Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 51: /* 720(1440)x480i @ 119.88/120Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 240;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 38;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 54000;
			break;
		case 52: /* 720X576p @ 200Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 53: /* 720X576p @ 200Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 576;
			mdtd->h_blanking = 144;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 12;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 64;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 108000;
			break;
		case 54: /* 720(1440)x576i @ 200Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 55: /* 720(1440)x576i @ 200Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 288;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 126;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 108000;
			break;
		case 56: /* 720x480p @ 239.76/240Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 57: /* 720x480p @ 239.76/240Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 480;
			mdtd->h_blanking = 138;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 62;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 108000;
			break;
		case 58: /* 720(1440)x480i @ 239.76/240Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 59: /* 720(1440)x480i @ 239.76/240Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 240;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 38;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 108000;
			break;
		case 65:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 60: /* 1280x720p @ 23.97/24Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 2020;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 1760;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 66:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 61: /* 1280x720p @ 25Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 2680;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 2420;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 67:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 62: /* 1280x720p @ 29.97/30Hz  16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 2020;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 1760;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 78:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 63: /* 1920x1080p @ 119.88/120Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 77:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 64: /* 1920x1080p @ 100Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 79:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 1620;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 1360;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 80:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 1488;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 1228;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 81:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 960;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 700;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 82:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 520;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 260;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 82500;
			break;
		case 83:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 520;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 260;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 99000;
			break;
		case 84:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 320;
			mdtd->v_blanking = 105;
			mdtd->h_sync_offset = 60;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 165000;
			break;
		case 85:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 320;
			mdtd->v_blanking = 105;
			mdtd->h_sync_offset = 60;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 198000;
			break;
		case 86:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 1190;
			mdtd->v_blanking = 20;
			mdtd->h_sync_offset = 998;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 99000;
			break;
		case 87:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 640;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 448;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 90000;
			break;
		case 88:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 960;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 768;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 118800;
			break;
		case 89:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 740;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 548;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 185625;
			break;
		case 90:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 440;
			mdtd->v_blanking = 20;
			mdtd->h_sync_offset = 248;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 198000;
			break;
		case 91:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 410;
			mdtd->v_blanking = 170;
			mdtd->h_sync_offset = 218;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 371250;
			break;
		case 92:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 740;
			mdtd->v_blanking = 170;
			mdtd->h_sync_offset = 548;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 495000;
			break;
		case 99:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1184;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 968;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 100:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 304;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 101:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1184;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 968;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 102:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 304;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 103:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 93:		/* 4k x 2k, 30Hz */
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1660;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 1276;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 104:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 94:
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1440;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 1056;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 105:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 95:
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 176;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 106:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 96:
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1440;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 1056;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 107:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 97:
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 176;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 98:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1404;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 1020;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		default:
			return false;
		}
	} else if (video_format == CVT) {
		switch (code) {
		case 1:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 640;
			mdtd->v_active = 480;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 20;
			mdtd->h_sync_offset = 8;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 8;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 23750;
			break;
		case 2:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 800;
			mdtd->v_active = 600;
			mdtd->h_blanking = 224;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 31;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 81;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 38250;
			break;
		case 3:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1024;
			mdtd->v_active = 768;
			mdtd->h_blanking = 304;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 104;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 63500;
			break;
		case 4:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 960;
			mdtd->h_blanking = 416;
			mdtd->v_blanking = 36;
			mdtd->h_sync_offset = 80;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 101250;
			break;
		case 5:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1400;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 464;
			mdtd->v_blanking = 39;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 144;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 121750;
			break;
		case 6:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1600;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 112;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 68;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 161000;
			break;
		case 12:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 1024;
			mdtd->h_blanking = 432;
			mdtd->v_blanking = 39;
			mdtd->h_sync_offset = 80;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 136;
			mdtd->v_sync_pulse_width = 7;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 109000;
			break;
		case 13:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 768;
			mdtd->h_blanking = 384;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 7;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 79500;
			break;
		case 16:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 384;
			mdtd->v_blanking = 28;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 74500;
			break;
		case 17:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1360;
			mdtd->v_active = 768;
			mdtd->h_blanking = 416;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 72;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 136;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 84750;
			break;
		case 20:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 656;
			mdtd->v_blanking = 40;
			mdtd->h_sync_offset = 128;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 200;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 173000;
			break;
		case 22:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 2560;
			mdtd->v_active = 1440;
			mdtd->h_blanking = 928;
			mdtd->v_blanking = 53;
			mdtd->h_sync_offset = 192;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 272;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 312250;
			break;
		case 28:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 800;
			mdtd->h_blanking = 400;
			mdtd->v_blanking = 31;
			mdtd->h_sync_offset = 72;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 83500;
			break;
		case 34:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 672;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 136;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 200;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 193250;
			break;
		case 38:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 3840;
			mdtd->v_active = 2400;
			mdtd->h_blanking = 80;
			mdtd->v_blanking = 69;
			mdtd->h_sync_offset = 320;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 424;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 580128;
			break;
		case 40:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1600;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 35;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 124076;
			break;
		case 41:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 2048;
			mdtd->v_active = 1536;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 44;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 208000;
			break;
		default:
			return false;
		}
	} else if (video_format == DMT) {
		switch (code) {
		case 1: // HISilicon timing
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 3600;
			mdtd->v_active = 1800;
			mdtd->h_blanking = 120;
			mdtd->v_blanking = 128;
			mdtd->h_sync_offset = 20;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 20;
			mdtd->v_sync_pulse_width = 2;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 645500;
			break;
		case 2:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 62;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 533000;
			break;
		case 4:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 640;
			mdtd->v_active = 480;
			mdtd->h_blanking = 144;
			mdtd->v_blanking = 29;
			mdtd->h_sync_offset = 8;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 96;
			mdtd->v_sync_pulse_width = 2;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 25175;
			break;
		case 11:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 800;
			mdtd->v_active = 600;
			mdtd->h_blanking = 256;
			mdtd->v_blanking = 25;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 80;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 49500;
			break;
		case 13:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 800;
			mdtd->v_active = 600;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 36;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 73250;
			break;
		case 14: /* 848x480p@60Hz */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 848;
			mdtd->v_active = 480;
			mdtd->h_blanking = 240;
			mdtd->v_blanking = 37;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 6;
			mdtd->h_sync_pulse_width = 112;
			mdtd->v_sync_pulse_width = 8;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI)  */;
			mdtd->pixel_clock = 33750;
			break;
		case 22:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 768;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 7;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 68250;
			break;
		case 35:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 1024;
			mdtd->h_blanking = 408;
			mdtd->v_blanking = 42;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 112;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 108000;
			break;
		case 39:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1360;
			mdtd->v_active = 768;
			mdtd->h_blanking = 432;
			mdtd->v_blanking = 27;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 112;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 85500;
			break;
		case 40:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1360;
			mdtd->v_active = 768;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 148250;
			break;
		case 81:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1366;
			mdtd->v_active = 768;
			mdtd->h_blanking = 426;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 70;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 142;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 85500;
			break;
		case 86:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1366;
			mdtd->v_active = 768;
			mdtd->h_blanking = 134;
			mdtd->v_blanking = 32;
			mdtd->h_sync_offset = 14;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 56;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 72000;
			break;
		case 87:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 80;
			mdtd->v_blanking = 62;
			mdtd->h_sync_offset = 8;
			mdtd->v_sync_offset = 48;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 8;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 556744;
			break;
		case 88:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 80;
			mdtd->v_blanking = 62;
			mdtd->h_sync_offset = 8;
			mdtd->v_sync_offset = 48;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 8;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 556188;
			break;
		case 41:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1400;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 101000;
			break;
		case 42:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1400;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 464;
			mdtd->v_blanking = 39;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 144;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 121750;
			break;
		case 46:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1440;
			mdtd->v_active = 900;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 26;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 88750;
			break;
		case 47:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1440;
			mdtd->v_active = 900;
			mdtd->h_blanking = 464;
			mdtd->v_blanking = 34;
			mdtd->h_sync_offset = 80;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 152;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 106500;
			break;
		case 51:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1600;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 50;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 192;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 162000;
			break;
		case 57:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1680;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 119000;
			break;
		case 58:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1680;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 39;
			mdtd->h_sync_offset = 104;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 176;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 146250;
			break;
		case 68:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 35;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 154000;
			break;
		case 69:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 672;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 136;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 200;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 193250;
			break;
		case 76:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 2560;
			mdtd->v_active = 1600;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 46;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 193250;
			break;
		case 77:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 2560;
			mdtd->v_active = 1600;
			mdtd->h_blanking = 940;
			mdtd->v_blanking = 58;
			mdtd->h_sync_offset = 192;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 280;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 193250;
			break;
		case 82:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock  = 148500;
			break;
		case 83:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1600;
			mdtd->v_active = 900;
			mdtd->h_blanking = 200;
			mdtd->v_blanking = 100;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 80;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 108000;
			break;
		case 9:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 800;
			mdtd->v_active = 600;
			mdtd->h_blanking = 256;
			mdtd->v_blanking = 28;
			mdtd->h_sync_offset = 40;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 40000;
			break;
		case 16:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1024;
			mdtd->v_active = 768;
			mdtd->h_blanking = 320;
			mdtd->v_blanking = 38;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 136;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 65000;
			break;
		case 23:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 768;
			mdtd->h_blanking = 384;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 7;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 79500;
			break;
		case 62:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active =  1792;
			mdtd->v_active = 1344;
			mdtd->h_blanking = 656;
			mdtd->v_blanking =  50;
			mdtd->h_sync_offset = 128;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 200;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 204750;
			break;
		case 32:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 960;
			mdtd->h_blanking = 520;
			mdtd->v_blanking = 40;
			mdtd->h_sync_offset = 96;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 112;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;  /* (progressive_nI) */
			mdtd->pixel_clock = 108000;
			break;
		case 73:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1440;
			mdtd->h_blanking = 680;
			mdtd->v_blanking = 60;
			mdtd->h_sync_offset = 128;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 208;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;  /* (progressive_nI) */
			mdtd->pixel_clock = 234000;
			break;
		case 27:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 800;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 23;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;  /* (progressive_nI) */
			mdtd->pixel_clock = 71000;
			break;
		default:
			return false;
		}
	} else {
		HISI_FB_ERR("[DP] Video Format is ERROR\n");
		return false;
	}

	return true;
}
