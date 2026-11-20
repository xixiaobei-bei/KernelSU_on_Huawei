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
/*lint -e529 -e574*/

#include "dp_avgen_base.h"
#include "../link/dp_edid.h"
#include "../link/dp_aux.h"
#include "hisi_defs.h"

u32 audio_freq_to_infoframe_data(u8 orig_sample_freq, u8 sample_freq)
{
	u32 audio_infoframe_data;

	if (orig_sample_freq == 12 && sample_freq == 3)
		audio_infoframe_data = 0x00000710;
	else if (orig_sample_freq == 15 && sample_freq == 0)
		audio_infoframe_data = 0x00000B10;
	else if (orig_sample_freq == 13 && sample_freq == 2)
		audio_infoframe_data = 0x00000F10;
	else if (orig_sample_freq == 7 && sample_freq == 8)
		audio_infoframe_data = 0x00001310;
	else if (orig_sample_freq == 5 && sample_freq == 10)
		audio_infoframe_data = 0x00001710;
	else if (orig_sample_freq == 3 && sample_freq == 12)
		audio_infoframe_data = 0x00001B10;
	else
		audio_infoframe_data = 0x00001F10;

	return audio_infoframe_data;
}

u32 audio_num_channels_to_infoframe_data(u8 num_channels)
{
	u32 audio_infoframe_data = 0;

	switch (num_channels) {
	case 3:
		audio_infoframe_data = 0x02000000;
		break;
	case 4:
		audio_infoframe_data = 0x03000000;
		break;
	case 5:
		audio_infoframe_data = 0x07000000;
		break;
	case 6:
		audio_infoframe_data = 0x0b000000;
		break;
	case 7:
		audio_infoframe_data = 0x0f000000;
		break;
	case 8:
		audio_infoframe_data = 0x13000000;
		break;
	default:
		break;
	}
	return audio_infoframe_data;
}

/**
 * dptx_sink_enabled_ssc() - Returns true, if sink is enabled ssc
 * @dptx: The dptx struct
 *
 */
bool dptx_sink_enabled_ssc(struct dp_ctrl *dptx)
{
	u8 byte = 0;
	int retval = 0;

	drm_check_and_return((dptx == NULL), false, ERR, "[DP] NULL Pointer\n");

	retval = dptx_read_dpcd(dptx, DP_MAX_DOWNSPREAD, &byte);
	if (retval)
		HISI_DRM_ERR("[DP] dptx_read_dpcd fail\n");

	return byte & 1;
}

int dptx_video_mode_change(struct dp_ctrl *dptx, u8 vmode)
{
	int retval = 0;
	struct video_params *vparams = NULL;
	struct dtd mdtd;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;
	vparams->mode = vmode;

	if (!convert_code_to_dtd(&mdtd, vparams->mode, vparams->refresh_rate,
			   vparams->video_format)) {
		HISI_DRM_ERR("[DP] Invalid video mode value %d\n",
						vparams->mode);
		return -EINVAL;
	}
	vparams->mdtd = mdtd;
	if (dptx->dptx_video_ts_calculate)
		retval = dptx->dptx_video_ts_calculate(dptx, vparams, mdtd.pixel_clock);

	HISI_DRM_INFO("[DP] The mode is changed as [%d]\n", vparams->mode);

	return retval;
}

bool dptx_check_low_temperature(struct dp_ctrl *dptx)
{
	u32 perictrl4;

	if (dptx == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return false;
	}

	perictrl4 = inp32(dptx->pmctrl_base + MIDIA_PERI_CTRL4);
	perictrl4 &= PMCTRL_PERI_CTRL4_TEMPERATURE_MASK;
	perictrl4 = (perictrl4 >> PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT);
	HISI_DRM_INFO("[DP] Get current temperature: %d\n", perictrl4);

	if (perictrl4 != NORMAL_TEMPRATURE)
		return true;
	else
		return false;
}

int dptx_change_video_mode_user(struct dp_ctrl *dptx)
{
	struct video_params *vparams = NULL;
	int retval;
	bool needchanged = false;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;
	if (!dptx->same_source) {
		if ((vparams->mdtd.h_active > FHD_TIMING_H_ACTIVE) || (vparams->mdtd.v_active > FHD_TIMING_V_ACTIVE)) {
			vparams->video_format = VCEA;
			vparams->mode = 16; /*Siwtch to 1080p on PC mode*/
			needchanged = TRUE;
			HISI_DRM_INFO("[DP] Video mode is changed by different source!\n");
		}
	}

	if (dptx->user_mode != 0) {
		vparams->video_format = dptx->user_mode_format;
		vparams->mode = dptx->user_mode; /*Siwtch to user setting*/
		needchanged = TRUE;
		HISI_DRM_INFO("[DP] Video mode is changed by user setting!\n");
	}

	if (needchanged) {
		retval = dptx_video_mode_change(dptx, vparams->mode);
		if (retval) {
			HISI_DRM_ERR("[DP] Change mode error!\n");
			return retval;
		}
	}

	if (dptx_check_low_temperature(dptx)) {
		if ((vparams->mdtd.h_active > FHD_TIMING_H_ACTIVE) || (vparams->mdtd.v_active > FHD_TIMING_V_ACTIVE)) {
			vparams->video_format = VCEA;
			vparams->mode = 16; /* Siwtch to 1080p on PC mode */
			HISI_DRM_INFO("[DP] Video mode is changed by low temperature!\n");

			retval = dptx_video_mode_change(dptx, vparams->mode);
			if (retval) {
				HISI_DRM_ERR("[DP] Change mode error!\n");
				return retval;
			}
		}
	}

	dp_imonitor_set_param(DP_PARAM_SOURCE_MODE, &(dptx->same_source));
	dp_imonitor_set_param(DP_PARAM_USER_MODE,   &(vparams->mode));
	dp_imonitor_set_param(DP_PARAM_USER_FORMAT, &(vparams->video_format));
	return 0;
}

/* default (bpc) -> (colordep) mapping */
static const struct default_colordep_map default_colordep_maps[] = {
	{COLOR_DEPTH_6, 18},
	{COLOR_DEPTH_8, 24},
	{COLOR_DEPTH_10, 30},
	{COLOR_DEPTH_12, 36},
	{COLOR_DEPTH_16, 48},
};

/* (bpc, encoding) -> (colordep) mapping */
static const struct colordep_map colordep_maps[] = {
	{COLOR_DEPTH_8, YCBCR420, 12},
	{COLOR_DEPTH_8, YCBCR422, 16},
	{COLOR_DEPTH_8, YONLY, 8},
	{COLOR_DEPTH_10, YCBCR420, 15},
	{COLOR_DEPTH_10, YCBCR422, 20},
	{COLOR_DEPTH_10, YONLY, 10},
	{COLOR_DEPTH_12, YCBCR420, 18},
	{COLOR_DEPTH_12, YCBCR422, 24},
	{COLOR_DEPTH_12, YONLY, 12},
	{COLOR_DEPTH_16, YCBCR420, 24},
	{COLOR_DEPTH_16, YCBCR422, 32},
	{COLOR_DEPTH_16, YONLY, 16},
};

int dptx_get_color_depth(int bpc, int encoding)
{
	int colordep;
	u32 i;

	/* set default value of color_depth */
	colordep = 18;

	/* set default value of color_depth according to different bpc */
	for (i = 0; i < ARRAY_SIZE(default_colordep_maps); i++) {
		if (bpc == default_colordep_maps[i].bpc) {
			colordep = default_colordep_maps[i].default_colordep;
			break;
		}
	}

	/* set value of color_depth according to bpc and encoding */
	for (i = 0; i < ARRAY_SIZE(colordep_maps); i++) {
		if (bpc == colordep_maps[i].bpc && encoding == colordep_maps[i].encoding) {
			colordep = colordep_maps[i].colordep;
			break;
		}
	}

	return colordep;
}

int dptx_br_to_link_clk(int rate)
{
	int link_clk;

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		link_clk = 40500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		link_clk = 67500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		link_clk = 135000;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		link_clk = 202500;
		break;
	default:
		return -EINVAL;
	}

	return link_clk;
}

int dptx_br_to_link_rate(int rate)
{
	int link_rate;

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
		return -EINVAL;
	}

	return link_rate;
}

/*
 * MISC0 bit7:5 for Pixel Encoding/Colorimetry Format Indicationa, According to Table 2-96 of DisplayPort spec 1.4
 */
static const struct video_bpc_mapping sink_bpc_maping[] = {
	{RGB, COLOR_DEPTH_6, 0},
	{RGB, COLOR_DEPTH_8, 1},
	{RGB, COLOR_DEPTH_10, 2},
	{RGB, COLOR_DEPTH_12, 3},
	{RGB, COLOR_DEPTH_16, 4},
	{YCBCR444, COLOR_DEPTH_8, 1},
	{YCBCR444, COLOR_DEPTH_10, 2},
	{YCBCR444, COLOR_DEPTH_12, 3},
	{YCBCR444, COLOR_DEPTH_16, 4},
	{YCBCR422, COLOR_DEPTH_8, 1},
	{YCBCR422, COLOR_DEPTH_10, 2},
	{YCBCR422, COLOR_DEPTH_12, 3},
	{YCBCR422, COLOR_DEPTH_16, 4},
	{YONLY, COLOR_DEPTH_8, 1},
	{YONLY, COLOR_DEPTH_10, 2},
	{YONLY, COLOR_DEPTH_12, 3},
	{YONLY, COLOR_DEPTH_16, 4},
	{RAW, COLOR_DEPTH_6, 1},
	{RAW, COLOR_DEPTH_8, 3},
	{RAW, COLOR_DEPTH_10, 4},
	{RAW, COLOR_DEPTH_12, 5},
	{RAW, COLOR_DEPTH_16, 7},
};

u8 dptx_get_sink_bpc_mapping(enum pixel_enc_type pix_enc, u8 bpc)
{
	u8 bpc_mapping = 0;
	int i;
	struct video_bpc_mapping vmap;

	for (i = 0; i < ARRAY_SIZE(sink_bpc_maping); i++) {
		vmap = sink_bpc_maping[i];

		if (pix_enc == vmap.pix_enc && bpc == vmap.bpc) {
			bpc_mapping = vmap.bpc_mapping;
			break;
		}
	}

	return bpc_mapping;
}

/*lint +e529 +e574*/
