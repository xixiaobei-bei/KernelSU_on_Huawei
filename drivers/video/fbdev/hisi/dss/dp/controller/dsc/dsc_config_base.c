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

#include "dsc_config_base.h"
#include "../../../hisi_fb.h"
#include "../../../hisi_fb_def.h"
#include "../dp_core_interface.h"

#include "../hidpc/hidpc_reg.h"

/* 5 pps begin */
static void dptx_program_dsc_version(struct dp_ctrl *dptx)
{
	uint32_t pps;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0));
	pps |= dptx->vparams.dp_dsc_info.dsc_info.dsc_version_minor;
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.dsc_version_major << DPTX_DSC_VER_MAJ_SHIFT);
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0), pps);
}

static void dptx_program_dsc_buf_bit_depth(struct dp_ctrl *dptx)
{
	uint32_t pps;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0));
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.linebuf_depth << DPTX_DSC_BUF_DEPTH_SHIFT);
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0), pps);
}

static void dptx_program_dsc_block_prediction(struct dp_ctrl *dptx)
{
	uint32_t pps;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1));
	pps |=  dptx->vparams.dp_dsc_info.dsc_info.block_pred_enable << 5;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1), pps);
}

static void dptx_program_dsc_bits_perpixel(struct dp_ctrl *dptx)
{
	enum pixel_enc_type;
	uint8_t bpp_high, bpp_low;
	uint32_t pps;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");
	/* Get high and low parts of bpp (10 bits) */

	bpp_high = (dptx->vparams.dp_dsc_info.dsc_info.dsc_bpp & DPTX_DSC_BPP_HIGH_MASK) >> 8;
	bpp_low = (dptx->vparams.dp_dsc_info.dsc_info.dsc_bpp*16) & DPTX_DSC_BPP_LOW_MASK;

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1));
	pps |= bpp_high;
	pps |= bpp_low << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1), pps);

	DPU_FB_INFO("[DP] %s PPS  bpp = %d\n", __func__, dptx->vparams.dp_dsc_info.dsc_info.dsc_bpp);
}

void dptx_program_dsc_bpc_and_depth(struct dp_ctrl *dptx)
{
	uint32_t pps;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0));
	switch (dptx->vparams.dp_dsc_info.dsc_info.dsc_bpc) {
	case COLOR_DEPTH_8:
		pps |= dptx->vparams.dp_dsc_info.dsc_info.dsc_bpc << DPTX_DSC_BPC_SHIFT;
		break;
	case COLOR_DEPTH_10:
		pps |= dptx->vparams.dp_dsc_info.dsc_info.dsc_bpc << DPTX_DSC_BPC_SHIFT;
		break;
	case COLOR_DEPTH_12:
		pps |= dptx->vparams.dp_dsc_info.dsc_info.dsc_bpc << DPTX_DSC_BPC_SHIFT;
		break;
	case COLOR_DEPTH_16:
		pps |= dptx->vparams.dp_dsc_info.dsc_info.dsc_bpc << DPTX_DSC_BPC_SHIFT;
		break;
	default:
		DPU_FB_INFO("[DP] Unsupported Color depth by DSC spec\n");
		break;
	}

	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0), pps);

	switch (dptx->vparams.dp_dsc_info.dsc_info.format) {
	case DSC_RGB:
		pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1));
		pps |= 1 << 4;
		dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1), pps);
		dptx->vparams.dp_dsc_info.dsc_info.native_420 = 0;
		break;
	case DSC_YUV:
		pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 23));
		pps |= 1 << 1;
		dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 23), pps);
		dptx->vparams.dp_dsc_info.dsc_info.native_420 = 1;
		break;
	default:
		break;
	}
}

void dptx_program_dsc_slice_width(struct dp_ctrl *dptx)
{
	uint32_t pps = 0;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	pps |= dptx->vparams.dp_dsc_info.dsc_info.slice_width >> 8;
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.slice_width & GENMASK(7, 0)) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 3), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 3));
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.chunk_size >> 8) << 16;
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.chunk_size & GENMASK(7, 0)) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 3), pps);
}

void dptx_program_dsc_pic_width_height(struct dp_ctrl *dptx)
{
	uint32_t pps;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 2));
	pps = 0;
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.pic_width >> 8);
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.pic_width & GENMASK(7, 0)) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 2), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1));
	pps &= ~GENMASK(31, 16);
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.pic_height >> 8) << 16;
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.pic_height & GENMASK(7, 0)) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1), pps);
}

void dptx_program_dsc_slice_height(struct dp_ctrl *dptx)
{
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	reg = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 2));
	reg |= (dptx->vparams.dp_dsc_info.dsc_info.slice_height >> 8) << 16;
	reg |= (dptx->vparams.dp_dsc_info.dsc_info.slice_height & GENMASK(7, 0)) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 2), reg);

	reg = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 6));
	reg |= dptx->vparams.dp_dsc_info.dsc_info.first_line_bpg_offset << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 6), reg);

	reg = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 22));
	reg |= dptx->vparams.dp_dsc_info.dsc_info.second_line_bpg_offset << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 22), reg);
}

void dptx_program_dsc_min_rate_bufsize(struct dp_ctrl *dptx)
{
	uint32_t pps;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	/* Program rc_model_size pps */
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 9));
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.rc_model_size >> 8) << 16;
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.rc_model_size & GENMASK(7, 0)) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 9), pps);

	/* Program initial_offset pps */
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 8));
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.initial_offset & GENMASK(15, 8)) >> 8);
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.initial_offset & GENMASK(7, 0)) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 8), pps);

	/* Program initial_delay pps */
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 4));
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.initial_xmit_delay & GENMASK(9, 8)) >> 8);
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.initial_xmit_delay & GENMASK(7, 0)) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 4), pps);
}

void dptx_program_dsc_hrdelay(struct dp_ctrl *dptx)
{
	uint32_t pps;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 4));
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.initial_dec_delay >> 8) << 16;
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.initial_dec_delay & GENMASK(7, 0)) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 4), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 5));
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.initial_scale_value) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 5), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 6));
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.scale_decrement_interval & GENMASK(11, 9)) >> 9;
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.scale_decrement_interval & GENMASK(7, 0)) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 6), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 7));
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.nfl_bpg_offset & GENMASK(15, 8)) >> 8);
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.nfl_bpg_offset & GENMASK(7, 0))) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 7), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 8));
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.final_offset & GENMASK(15, 8)) >> 8) << 16;
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.final_offset & GENMASK(7, 0))) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 8), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 7));
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.slice_bpg_offset & GENMASK(15, 8)) >> 8) << 16;
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.slice_bpg_offset & GENMASK(7, 0))) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 7), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 5));
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.scale_increment_interval >> 8) << 16;
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.scale_increment_interval & GENMASK(7, 0))) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 5), pps);
	DPU_FB_DEBUG("[DP] addr= %d\n", DPTX_DSC_PPS(DPTX_SST_MODE, 5));
}

void dptx_program_flatness_qp(struct dp_ctrl *dptx)
{
	uint32_t pps;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 9));
	pps |= dptx->vparams.dp_dsc_info.dsc_info.flatness_min_qp;
	pps |= dptx->vparams.dp_dsc_info.dsc_info.flatness_max_qp << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 9), pps);
}

uint8_t dptx_get_twos_compl_rep(int number)
{
	uint8_t res;

	if (number >= 0)
		return (uint8_t)number;
	number = -number;
	/* Calculate 2's complement representation */
	res = ((uint8_t)number ^ 0x1f) + 1;
	/* Set the sign bit before returning */
	return (res | (1 << 5));
}

void dptx_program_rc_range_parameters(struct dp_ctrl *dptx)
{
	uint32_t pps;
	int i;
	int pps_index;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, (14)));
	pps |= ((dptx->vparams.dp_dsc_info.dsc_info.rc_range[0].max_qp & GENMASK(4, 2)) >> 2) << 16;
	pps |= dptx->vparams.dp_dsc_info.dsc_info.rc_range[0].min_qp << 19;
	pps |= dptx_get_twos_compl_rep(dptx->vparams.dp_dsc_info.dsc_info.rc_range[0].offset) << 24;
	pps |= (dptx->vparams.dp_dsc_info.dsc_info.rc_range[0].max_qp & GENMASK(1, 0)) << 30;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 14), pps);

	/* Read once 4 byte and program in there 2  rc_range_parameters */
	for (i = 1, pps_index = 60; i < 15; i += 2, pps_index += 4) {
		pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, (pps_index / 4)));
		pps |= ((dptx->vparams.dp_dsc_info.dsc_info.rc_range[i].max_qp & GENMASK(4, 2)) >> 2);
		pps |= (dptx->vparams.dp_dsc_info.dsc_info.rc_range[i].min_qp) << 3;
		pps |= dptx_get_twos_compl_rep(dptx->vparams.dp_dsc_info.dsc_info.rc_range[i].offset) << 8;
		pps |= (dptx->vparams.dp_dsc_info.dsc_info.rc_range[i].max_qp & GENMASK(1, 0)) << 14;

		/*lint -e661*/
		pps |= ((dptx->vparams.dp_dsc_info.dsc_info.rc_range[i + 1].max_qp & GENMASK(4, 2)) >> 2) << 16;
		pps |= (dptx->vparams.dp_dsc_info.dsc_info.rc_range[i + 1].min_qp) << 19;
		pps |= dptx_get_twos_compl_rep(dptx->vparams.dp_dsc_info.dsc_info.rc_range[i + 1].offset) << 24;
		pps |= (dptx->vparams.dp_dsc_info.dsc_info.rc_range[i + 1].max_qp & GENMASK(1, 0)) << 30;
		/*lint +e661*/

		dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, pps_index / 4), pps);
	}
}

void dptx_program_rc_parameter_set(struct dp_ctrl *dptx)
{
	uint32_t pps;
	int i;
	int pps_index;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	/* rc_edge_factor */
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 10));
	pps |= dptx->vparams.dp_dsc_info.dsc_info.rc_edge_factor;

	/* rc_quant_incr_limit0 */
	pps |= dptx->vparams.dp_dsc_info.dsc_info.rc_quant_incr_limit0 << 8;

	/* rc_quant_incr_limit1 */
	pps |= dptx->vparams.dp_dsc_info.dsc_info.rc_quant_incr_limit1 << 16;

	/* rc_tgt_offset_hi */
	pps |= dptx->vparams.dp_dsc_info.dsc_info.rc_tgt_offset_hi << 28;

	/* rc_tgt_offset_lo */
	pps |= dptx->vparams.dp_dsc_info.dsc_info.rc_tgt_offset_lo << 24;

	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 10), pps);

	/* PPS44 - PPS57 rc_buf_threshold values */
	for (i = 0, pps_index = 44; i < 14; ++i, ++pps_index) {
		pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, (pps_index / 4)));
		pps |= (dptx->vparams.dp_dsc_info.dsc_info.rc_buf_thresh[i]) << ((pps_index % 4) * 8);
		dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, pps_index / 4), pps);
	}

	/* RC_RANGE_PARAMETERS from DSC 1.2 spec */
	dptx_program_rc_range_parameters(dptx);
}

static void dptx_clear_pps(struct dp_ctrl *dptx)
{
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	for (reg = DPTX_DSC_PPS(DPTX_SST_MODE, DPTX_DSC_PPS_NUM_MIN);
		reg < DPTX_DSC_PPS(DPTX_SST_MODE, DPTX_DSC_PPS_NUM_MAX); reg += DPTX_DSC_PPS_NUM_STEP)
		dptx_writel(dptx, reg, DPTX_CLEAR_DSC_REGISTER_TO_ZERO);
}

void dptx_program_pps_sdps(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_clear_pps(dptx);
	dptx_program_dsc_version(dptx);
	dptx_program_dsc_buf_bit_depth(dptx);
	dptx_program_dsc_block_prediction(dptx);
	dptx_program_dsc_bits_perpixel(dptx);
	dptx_program_dsc_bpc_and_depth(dptx);
	dptx_program_dsc_slice_width(dptx);
	dptx_program_dsc_pic_width_height(dptx);
	dptx_program_dsc_slice_height(dptx);
	dptx_program_dsc_min_rate_bufsize(dptx);
	dptx_program_dsc_hrdelay(dptx);
	dptx_program_flatness_qp(dptx);
	dptx_program_rc_parameter_set(dptx);
}

void dptx_dsc_notice_rx(struct dp_ctrl *dptx)
{
	DPU_FB_DEBUG("[DP] +\n");
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	if (dptx_write_dpcd(dptx, DP_DSC_ENABLE, 0x1))
		DPU_FB_ERR("[DP] Fail to write DPCD\n");
}
