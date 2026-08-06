/* Copyright (c) 2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "dp_dsc_algorithm.h"
#include "drm_dp_helper_additions.h"
#include "../../hisi_dp_drv.h"
#include "../../hisi_defs.h"
#include "../../dsc/dsc_algorithm.h"
#include "../../dsc/dsc_algorithm_manager.h"
#include "../../dsc/utilities/hisi_debug.h"
#include "../../dsc/utilities/platform.h"
#include "dp_aux.h"
#include "hisi_panel.h"

/* (ppr_left_boundary, ppr_right_boundary) -> (slice_count) mapping */
static const struct slice_count_map slice_count_maps[] = {
	{0, 340, 1},
	{340, 680, 2},
	{680, 1360, 4},
	{1360, 3200, 8},
	{3200, 4800, 12},
	{4800, 6400, 16},
	{6400, 8000, 20},
	{8000, 9600, 24},
};

/* Line Buffer Bit Depth
 * Data Mapping
 * 0000 = 9 bits   0001 = 10 bits  0010 = 11 bits
 * 0011 = 12 bits  0100 = 13 bits  0101 = 14 bits
 * 0110 = 15 bits  0111 = 16 bits  1000 = 8 bits
 * All other values are RESERVED
 */
#define LINE_BUFFER_DEPTH_OFFSET 9

static void dptx_sink_color_depth_capabilities(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t dsc_color_depth = 0;

	retval = dptx_read_dpcd(dptx, DP_DSC_DEC_COLOR_DEPTH_CAP, &dsc_color_depth);
	if (retval)
		HISI_DRM_ERR("[DP] %s : DPCD read failed\n", __func__);

	if (dsc_color_depth & DP_DSC_8_BPC)
		HISI_DRM_DEBUG("Sink supports 8 bpc\n");

	if (dsc_color_depth & DP_DSC_10_BPC)
		HISI_DRM_DEBUG("Sink supports 10 bpc\n");

	if (dsc_color_depth & DP_DSC_12_BPC)
		HISI_DRM_DEBUG("Sink supports 12 bpc\n");
}

static void dptx_sink_color_format_capabilities(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t dsc_color_format = 0;

	retval = dptx_read_dpcd(dptx, DP_DSC_DEC_COLOR_FORMAT_CAP, &dsc_color_format);
	if (retval)
		HISI_DRM_ERR("[DP] %s : DPCD read failed\n", __func__);

	if (dsc_color_format & DP_DSC_RGB)
		HISI_DRM_DEBUG("Sink supports RGB color format\n");

	if (dsc_color_format & DP_DSC_YCBCR_444)
		HISI_DRM_DEBUG("Sink supports YCbCr 4:4:4 color format\n");

	if (dsc_color_format & DP_DSC_YCBCR_422_SIMPLE)
		HISI_DRM_DEBUG("Sink supports YCbCr 4:2:2 SIMPLE color format\n");

	if (dsc_color_format & DP_DSC_YCBCR_422_NATIVE)
		HISI_DRM_DEBUG("Sink supports YCbCr 4:2:2 NATIVE color format\n");

	if (dsc_color_format & DP_DSC_YCBCR_420_NATIVE)
		HISI_DRM_DEBUG("Sink supports YCbCr 4:2:0 NATIVE color format\n");
}

static int dptx_get_rx_line_buffer_depth(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t buf_bit_depth;
	uint8_t line_buf_depth;

	retval = dptx_read_dpcd(dptx, DP_DSC_LINE_BUF_BIT_DEPTH, &buf_bit_depth);
	if (retval)
		HISI_DRM_ERR("[DP] %s : DPCD read failed\n", __func__);

	buf_bit_depth = (buf_bit_depth & DP_DSC_LINE_BUF_BIT_DEPTH_MASK);
	HISI_DRM_DEBUG("[DP] %s PPS buf_bit_depth = %d\n", __func__, buf_bit_depth);

	if (buf_bit_depth < 8)
		line_buf_depth = buf_bit_depth + LINE_BUFFER_DEPTH_OFFSET;
	else
		line_buf_depth = 8; // RESERVED value set to 8 bits firstly

	if (dptx->dptx_line_buffer_depth_limit != NULL)
		line_buf_depth = dptx->dptx_line_buffer_depth_limit(dptx, line_buf_depth);

	return line_buf_depth;
}

int dptx_dsc_initial(struct dp_ctrl *dptx)
{
	uint8_t byte = 0;
	int retval;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	retval = dptx_read_dpcd(dptx, DP_DSC_SUPPORT, &byte);
	if (retval)
		return retval;

	if (byte & DP_DSC_DECOMPRESSION_IS_SUPPORTED) {
		HISI_DRM_INFO("[DP] DSC enable! retval=%d.\n", byte);
		dptx->dsc = true;
		dptx->fec = true;
	} else {
		dptx->dsc = false;
		dptx->fec = false;
	}

	return 0;
}

void dptx_dsc_check_rx_cap(struct dp_ctrl *dptx)
{
	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");
	dptx_sink_color_format_capabilities(dptx);
	dptx_sink_color_depth_capabilities(dptx);
}

void dptx_dsc_para_init(struct dp_ctrl *dptx)
{
	int slice_height_div;
	struct input_dsc_info input_dsc_info;
	struct dsc_algorithm_manager *p_algorithm = NULL;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	if (dptx->dptx_slice_height_limit != NULL)
		slice_height_div = dptx->dptx_slice_height_limit(dptx, dptx->vparams.mdtd.v_active);
	else
		slice_height_div = 1;

	if (slice_height_div == 0) {
		HISI_DRM_ERR("[DP] slice_height_div calc error\n");
		return;
	}

	p_algorithm = get_dsc_algorithm_manager_instance();

	if (p_algorithm == NULL) {
		HISI_DRM_ERR("[DP] get_dsc_check_instance NULL Pointer\n");
		return;
	}

	input_dsc_info.dsc_version = DSC_VERSION_V_1_2;
	input_dsc_info.format = DSC_RGB;
	input_dsc_info.pic_width = dptx->vparams.mdtd.h_active;
	input_dsc_info.pic_height = dptx->vparams.mdtd.v_active;
	input_dsc_info.slice_width = input_dsc_info.pic_width / dptx->dsc_decoders;
	input_dsc_info.slice_height = input_dsc_info.pic_height / slice_height_div;
	input_dsc_info.dsc_bpc = 8;
	input_dsc_info.block_pred_enable = 1;
	input_dsc_info.linebuf_depth = dptx_get_rx_line_buffer_depth(dptx);

	switch (dptx->dsc_ifbc_type) {
	case IFBC_TYPE_VESA2X_DUAL:
		input_dsc_info.dsc_bpp = 12;
		break;
	case IFBC_TYPE_VESA3X_DUAL:
		input_dsc_info.dsc_bpp = 8;
		break;
	case IFBC_TYPE_NONE:
	default:
		HISI_DRM_ERR("[DP] IFBC type error\n");
		return;
	}

	HISI_DRM_DEBUG("[DP] %s PPS pic_width = %d\n", __func__, input_dsc_info.pic_width);
	HISI_DRM_DEBUG("[DP] %s PPS slice_width = %d\n", __func__, input_dsc_info.slice_width);
	HISI_DRM_DEBUG("[DP] %s PPS pic_height = %d\n", __func__, input_dsc_info.pic_height);

	p_algorithm->vesa_dsc_info_calc(&input_dsc_info, &(dptx->vparams.dp_dsc_info.dsc_info), NULL);
}

