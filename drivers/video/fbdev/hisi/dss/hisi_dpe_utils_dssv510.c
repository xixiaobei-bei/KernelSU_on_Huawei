/* Copyright (c) 2012-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_dpe_utils.h"
#include "dsc/dsc_algorithm_manager.h"
#include "dsc/utilities/hisi_debug.h"
#include "hisi_fb_hisync.h"

/* dpp csc config */
#define CSC_ROW 3
#define CSC_COL 5

/* dbuf config */
#define DBUF_THRESHOLD_IN_FOR_SINGLE_DSC 50
#define DBUF_THRESHOLD_OUT_FOR_SINGLE_DSC 60

/*
 * Rec.601 for Computer
 * [ p00 p01 p02 cscidc2 cscodc2 ]
 * [ p10 p11 p12 cscidc1 cscodc1 ]
 * [ p20 p21 p22 cscidc0 cscodc0 ]
 */
static int g_csc10b_yuv2rgb709_wide[CSC_ROW][CSC_COL] = {
	{ 0x4000, 0x00000, 0x064ca, 0x000, 0x000 },
	{ 0x4000, 0x1f403, 0x1e20a, 0x600, 0x000 },
	{ 0x4000, 0x076c2, 0x00000, 0x600, 0x000 },
};

static int g_csc10b_rgb2yuv709_wide[CSC_ROW][CSC_COL] = {
	{ 0x00d9b, 0x02dc6, 0x0049f, 0x000, 0x000 },
	{ 0x1f8ab, 0x1e755, 0x02000, 0x000, 0x200 },
	{ 0x02000, 0x1e2ef, 0x1fd11, 0x000, 0x200 },
};

struct dsc_algorithm {
	uint32_t pic_width;
	uint32_t pic_height;
	uint32_t chunk_size;
	uint32_t groups_per_line;
	uint32_t rbs_min;
	uint32_t hrd_delay;
	uint32_t target_bpp_x16;
	uint32_t num_extra_mux_bits;
	uint32_t slice_bits;
	uint32_t final_offset;
	uint32_t final_scale;
	uint32_t nfl_bpg_offset;
	uint32_t groups_total;
	uint32_t slice_bpg_offset;
	uint32_t scale_increment_interval;
	uint32_t initial_scale_value;
	uint32_t scale_decrement_interval;
	uint32_t adjustment_bits;
	uint32_t adj_bits_per_grp;
	uint32_t bits_per_grp;
	uint32_t slices_per_line;
	uint32_t pic_line_grp_num;
};

#define OFFSET_FRACTIONAL_BITS 11
#define GMP_CNT_COFE 4913  /* 17*17*17 */

extern uint32_t g_dyn_sw_default;

void init_dpp(struct dpu_fb_data_type *dpufd)
{
	char __iomem *disp_ch_base = NULL;
	struct dpu_panel_info *pinfo = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL point!");
		return;
	}
	pinfo = &(dpufd->panel_info);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		disp_ch_base = dpufd->dss_base + DSS_DISP_CH0_OFFSET;
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		disp_ch_base = dpufd->dss_base + DSS_DISP_CH2_OFFSET;
	} else {
		DPU_FB_ERR("fb%d, not support!\n", dpufd->index);
		return;
	}
	DPU_FB_DEBUG("fb%d +\n", dpufd->index);

	outp32(disp_ch_base + IMG_SIZE_BEF_SR,
		(DSS_HEIGHT(pinfo->yres) << 16) | DSS_WIDTH(pinfo->xres));
	outp32(disp_ch_base + IMG_SIZE_AFT_SR,
		(DSS_HEIGHT(pinfo->yres) << 16) | DSS_WIDTH(pinfo->xres));
	outp32(disp_ch_base + IMG_SIZE_AFT_IFBCSW,
		(DSS_HEIGHT(pinfo->yres) << 16) | DSS_WIDTH(pinfo->xres));
}

static void hisi_dss_dsc_set_reg_default_value(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dsc_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	dsc_base = dpufd->dss_base + DSS_DSC_OFFSET;

	set_reg(dsc_base + DSC_REG_DEFAULT, 0x1, 1, 0); /* 0x1 --> reset dsc reg */
	set_reg(dsc_base + DSC_REG_DEFAULT, 0x0, 1, 0); /* 0x0 --> disreset dsc reg */
}

static void config_dsc_enable(struct dpu_fb_data_type *dpufd,
	struct dsc_info_mipi *out_dsc_info, uint32_t ifbc_type)
{
	uint32_t dsc_en = 0;
	uint32_t dual_dsc_en = 0;
	uint32_t reset_ich_per_line = 0;
	struct dpu_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);

	if ((ifbc_type == IFBC_TYPE_VESA2X_SINGLE) ||
		(ifbc_type == IFBC_TYPE_VESA3X_SINGLE) ||
		(ifbc_type == IFBC_TYPE_VESA3_75X_SINGLE) ||
		(ifbc_type == IFBC_TYPE_VESA4X_SINGLE_SPR) ||
		(ifbc_type == IFBC_TYPE_VESA2X_SINGLE_SPR)) {
		dsc_en = 0x1;  /* default valve */
		dual_dsc_en = 0;
		reset_ich_per_line = 0;
	} else {
		dsc_en = 0x1;  /* default valve */
		dual_dsc_en = 1;
		reset_ich_per_line = 1;
	}

	if (pinfo->dynamic_dsc_support)
		dsc_en = set_bits32(dsc_en, pinfo->dynamic_dsc_en, 1, 0);


	out_dsc_info->out_dsc_en = (reset_ich_per_line << 3) | (dual_dsc_en << 1) | dsc_en;
	out_dsc_info->dual_dsc_en = dual_dsc_en;

	DPU_FB_DEBUG("dynamic_dsc_support=%d, ifbc_type:%d, pinfo_dynamic_dsc_en:%d, dsc_en=%d\n",
		pinfo->dynamic_dsc_support, pinfo->ifbc_type, pinfo->dynamic_dsc_en, dsc_en);
}

/*
 * scale_increment_interval:At the end of the slice, it is also desirable to shrink the effective RC model range
 * to maximize tracking ability. It is recommended to use a value equal to
 * (final_offset / (nfl_bpg_offset + slice_bpg_offset)) / (8 * (finalScaleValue - 1.125)),
 * where finalScaleValue is equal to rc_model_size / (rc_model_size - final_offset).
 * If finalScaleValue is less than or equal to 9, a value of 0 should be used to disable
 * the scale increment at the end of the slice. If the calculation for scale_increment_interval
 * results in a value that exceeds 65535, a smaller slice height should be used so that the
 * programmed scale_increment_interval fits within a 16-bit field.
 * those magice numbers were assigned by the DSC protocol.
 */
static void calc_dsc_scale_increment_interval(struct dsc_algorithm *dsc_algo, struct dsc_panel_info *dsc)
{
	dsc_algo->final_offset = dsc->rc_model_size -
		((dsc->initial_xmit_delay * dsc_algo->target_bpp_x16 + 8) >> 4) +
		dsc_algo->num_extra_mux_bits; /* 4336(0x10f0) */

	if (dsc->rc_model_size == dsc_algo->final_offset) {
		HISI_FB_ERR("divsion by zero error");
		return;
	}

	dsc_algo->final_scale = 8 * dsc->rc_model_size / (dsc->rc_model_size - dsc_algo->final_offset);

	/* 793(0x319) */
	dsc_algo->nfl_bpg_offset = round1(dsc->first_line_bpg_offset << OFFSET_FRACTIONAL_BITS, dsc->slice_height);
	dsc_algo->groups_total = dsc_algo->groups_per_line * (dsc->slice_height + 1);
	dsc_algo->slice_bpg_offset = round1((1 << OFFSET_FRACTIONAL_BITS) *
		(dsc->rc_model_size - dsc->initial_offset + dsc_algo->num_extra_mux_bits),
		dsc_algo->groups_total); /* 611(0x263) */

	if (dsc_algo->final_scale > 9)
		/* 903(0x387) */
		dsc_algo->scale_increment_interval = (1 << OFFSET_FRACTIONAL_BITS) * dsc_algo->final_offset /
			((dsc_algo->final_scale - 9) * (dsc_algo->nfl_bpg_offset + dsc_algo->slice_bpg_offset));
	else
		dsc_algo->scale_increment_interval = 0;
}

/* The follow code is used for DSC algorithm.
 * It contains lots of fixed numbers.
 */
static void calc_dsc_algo_valve(struct dsc_info_mipi *out_dsc_info, struct dsc_algorithm *dsc_algo,
	struct dpu_panel_info *pinfo)
{
	struct dsc_panel_info *dsc = &(pinfo->vesa_dsc);

	dsc_algo->slices_per_line = 0;

	dsc_algo->num_extra_mux_bits = 3 * (dsc->mux_word_size + (4 * dsc->bits_per_component + 4) - 2);

	dsc_algo->chunk_size = round1((dsc->slice_width + 1) * dsc->bits_per_pixel, 8);

	dsc_algo->pic_line_grp_num = ((dsc->slice_width + 3) / 3) * (dsc_algo->slices_per_line + 1) - 1;

	dsc_algo->groups_per_line = (dsc->slice_width + 3) / 3;

	dsc_algo->rbs_min = dsc->rc_model_size - dsc->initial_offset + dsc->initial_xmit_delay * dsc->bits_per_pixel +
		dsc_algo->groups_per_line * dsc->first_line_bpg_offset;
	dsc_algo->hrd_delay = round1(dsc_algo->rbs_min, dsc->bits_per_pixel);

	dsc_algo->target_bpp_x16 = dsc->bits_per_pixel * 16;
	dsc_algo->slice_bits = 8 * dsc_algo->chunk_size * (dsc->slice_height + 1);

	while ((dsc_algo->num_extra_mux_bits > 0) &&
		((dsc_algo->slice_bits - dsc_algo->num_extra_mux_bits) % dsc->mux_word_size))
		dsc_algo->num_extra_mux_bits--;

	calc_dsc_scale_increment_interval(dsc_algo, dsc);

	/* Those magice numbers were assigned by the DSC protocol */
	if (dsc->rc_model_size == dsc->initial_offset) {
		HISI_FB_ERR("divsion by zero error");
		return;
	}

	dsc_algo->initial_scale_value = 8 * dsc->rc_model_size / (dsc->rc_model_size - dsc->initial_offset);
	if (dsc_algo->groups_per_line < dsc_algo->initial_scale_value - 8)
		dsc_algo->initial_scale_value = dsc_algo->groups_per_line + 8;

	/*
	 * scale_decrement_interval: Specifies the number of group times between
	 * decrementing the rcXformScale factor at the beginning of a slice.
	 * those magice numbers were assigned by the DSC protocol
	 */
	if (dsc_algo->initial_scale_value > 8)
		dsc_algo->scale_decrement_interval = dsc_algo->groups_per_line / (dsc_algo->initial_scale_value - 8);
	else
		dsc_algo->scale_decrement_interval = 4095;

	dsc_algo->adjustment_bits = (8 - (dsc->bits_per_pixel * (dsc->slice_width + 1)) % 8) % 8;
	dsc_algo->adj_bits_per_grp = dsc->bits_per_pixel * 3 - 3;
	dsc_algo->bits_per_grp = dsc->bits_per_pixel * 3;
}

static void config_dsc_para(struct dpu_fb_data_type *dpufd, struct dsc_info_mipi *out_dsc_info)
{
	uint32_t ifbc_type;
	struct dsc_algorithm dsc_algo = {0};
	struct dpu_panel_info *pinfo = NULL;

	dpu_check_and_no_retval((dpufd == NULL), ERR, "dpufd is NULL!\n");
	dpu_check_and_no_retval((out_dsc_info == NULL), ERR, "out_dsc_info is NULL!\n");

	pinfo = &(dpufd->panel_info);

	if (pinfo->dynamic_dsc_support)
		ifbc_type = pinfo->dynamic_dsc_ifbc_type;
	else
		ifbc_type = pinfo->ifbc_type;

	if (ifbc_type == IFBC_TYPE_NONE) {
		DPU_FB_DEBUG("ifbc not support\n");
		return;
	}

	config_dsc_enable(dpufd, out_dsc_info, ifbc_type);

	dsc_algo.pic_width = DSS_WIDTH(pinfo->xres);
	dsc_algo.pic_height = DSS_HEIGHT(pinfo->yres);

	calc_dsc_algo_valve(out_dsc_info, &dsc_algo, pinfo);

	out_dsc_info->pic_width = dsc_algo.pic_width;
	out_dsc_info->pic_height = dsc_algo.pic_height;
	out_dsc_info->chunk_size = dsc_algo.chunk_size;
	out_dsc_info->final_offset = dsc_algo.final_offset;
	out_dsc_info->nfl_bpg_offset = dsc_algo.nfl_bpg_offset;
	out_dsc_info->slice_bpg_offset = dsc_algo.slice_bpg_offset;
	out_dsc_info->scale_increment_interval = dsc_algo.scale_increment_interval;
	out_dsc_info->initial_scale_value = dsc_algo.initial_scale_value;
	out_dsc_info->scale_decrement_interval = dsc_algo.scale_decrement_interval;
	out_dsc_info->adjustment_bits = dsc_algo.adjustment_bits;
	out_dsc_info->adj_bits_per_grp = dsc_algo.adj_bits_per_grp;
	out_dsc_info->pic_line_grp_num = dsc_algo.pic_line_grp_num;
	out_dsc_info->bits_per_grp = dsc_algo.bits_per_grp;
	out_dsc_info->hrd_delay = dsc_algo.hrd_delay;
}

static void set_dsc_ctrl_reg(char __iomem *dsc_base,
		struct dsc_panel_info *dsc, struct dsc_info_mipi *input_dsc_info)
{
	uint32_t dsc_insert_byte_num = 0;

	if (input_dsc_info->chunk_size % 3)
		dsc_insert_byte_num = (3 - input_dsc_info->chunk_size % 3) & 0x3;

	if (dsc_insert_byte_num != 0) {
		DPU_FB_INFO("dsc_insert_byte_num = %d\n", dsc_insert_byte_num);
		set_reg(dsc_base + DSC_CTRL, dsc->bits_per_component |
			(dsc->linebuf_depth << 4) | (dsc_insert_byte_num << 8) |
			(dsc->block_pred_enable << 10) |
			(0x1 << 11) | (dsc->bits_per_pixel << 16), 26, 0);
	} else {
		/* bits_per_component, convert_rgb, bits_per_pixel */
		set_reg(dsc_base + DSC_CTRL, dsc->bits_per_component |
			(dsc->linebuf_depth << 4) | (dsc->block_pred_enable << 10) |
			(0x1 << 11) | (dsc->bits_per_pixel << 16), 26, 0);
	}
}

static void set_dsc_reg_ctrl(char __iomem *dsc_base,
	struct dsc_panel_info *dsc, struct dsc_info_mipi *input_dsc_info)
{
	/* dsc_en */
	set_reg(dsc_base + DSC_EN, input_dsc_info->out_dsc_en, 4, 0);

	set_dsc_ctrl_reg(dsc_base, dsc, input_dsc_info);

	/* pic_width, pic_height */
	set_reg(dsc_base + DSC_PIC_SIZE,
		(input_dsc_info->pic_width << 16) | input_dsc_info->pic_height, 32, 0);

	/* slice_width, slice_height */
	set_reg(dsc_base + DSC_SLICE_SIZE,
		(dsc->slice_width << 16) | dsc->slice_height, 32, 0);

	/* chunk_size */
	set_reg(dsc_base + DSC_CHUNK_SIZE, input_dsc_info->chunk_size, 16, 0);

	/* initial_xmit_delay, initial_dec_delay = hrd_delay -initial_xmit_delay */
	set_reg(dsc_base + DSC_INITIAL_DELAY, dsc->initial_xmit_delay |
		((input_dsc_info->hrd_delay - dsc->initial_xmit_delay) << 16), 32, 0);

	/* initial_scale_value, scale_increment_interval */
	set_reg(dsc_base + DSC_RC_PARAM0,
		input_dsc_info->initial_scale_value | (input_dsc_info->scale_increment_interval << 16), 32, 0);

	/* scale_decrement_interval, first_line_bpg_offset */
	set_reg(dsc_base + DSC_RC_PARAM1,
		(dsc->first_line_bpg_offset << 16) | input_dsc_info->scale_decrement_interval, 21, 0);

	/* nfl_bpg_offset, slice_bpg_offset */
	set_reg(dsc_base + DSC_RC_PARAM2,
		input_dsc_info->nfl_bpg_offset | (input_dsc_info->slice_bpg_offset << 16), 32, 0);

	/* DSC_RC_PARAM3 */
	set_reg(dsc_base + DSC_RC_PARAM3,
		((input_dsc_info->final_offset << 16) | dsc->initial_offset), 32, 0);

	/* DSC_FLATNESS_QP_TH */
	if (dsc->dsc_version == DSC_VERSION_1_2)
		set_reg(dsc_base + DSC_FLATNESS_QP_TH, ((dsc->flatness_max_qp << 16) |
			(((uint8_t)dsc->flatness_min_qp) << 0)), 24, 0);
	else
		set_reg(dsc_base + DSC_FLATNESS_QP_TH, ((0x0 << 24)|(dsc->flatness_max_qp << 16) |
			(0x0 << 8)|((uint8_t)dsc->flatness_min_qp)), 32, 0);

	/* DSC_RC_PARAM4 */
	set_reg(dsc_base + DSC_RC_PARAM4,
		((dsc->rc_edge_factor << 20) | (dsc->rc_model_size << 0)), 24, 0);
	/* DSC_RC_PARAM5 */
	set_reg(dsc_base + DSC_RC_PARAM5,
		((dsc->rc_tgt_offset_lo << 20) | (dsc->rc_tgt_offset_hi << 16) |
		(dsc->rc_quant_incr_limit1 << 8) | (dsc->rc_quant_incr_limit0 << 0)), 24, 0);

	if (dsc->dsc_version != DSC_VERSION_1_2) {
		/* adjustment_bits */
		set_reg(dsc_base + DSC_ADJUSTMENT_BITS, input_dsc_info->adjustment_bits, 4, 0);

		/* bits_per_grp, adj_bits_per_grp */
		set_reg(dsc_base + DSC_BITS_PER_GRP, input_dsc_info->bits_per_grp |
			(input_dsc_info->adj_bits_per_grp << 8), 14, 0);
	}

	/* slices_per_line, pic_line_grp_num */
	set_reg(dsc_base + DSC_MULTI_SLICE_CTL, input_dsc_info->slices_per_line |
		(input_dsc_info->pic_line_grp_num << 16), 32, 0);
}

static void set_dsc_reg_rctable(char __iomem *dsc_base,
	struct dsc_panel_info *dsc, struct dsc_info_mipi *input_dsc_info)
{
	if ((dsc_base == NULL) || (dsc == NULL) || (input_dsc_info == NULL)) {
		DPU_FB_ERR("dsc_base is NULL");
		return;
	}

	/* DSC_RC_BUF_THRESH */
	set_reg(dsc_base + DSC_RC_BUF_THRESH0,
		((dsc->rc_buf_thresh0 << 24) | (dsc->rc_buf_thresh1 << 16) |
		(dsc->rc_buf_thresh2 << 8) | (dsc->rc_buf_thresh3 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH1,
		((dsc->rc_buf_thresh4 << 24) | (dsc->rc_buf_thresh5 << 16) |
		(dsc->rc_buf_thresh6 << 8) | (dsc->rc_buf_thresh7 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH2,
		((dsc->rc_buf_thresh8 << 24) | (dsc->rc_buf_thresh9 << 16) |
		(dsc->rc_buf_thresh10 << 8) | (dsc->rc_buf_thresh11 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH3,
		((dsc->rc_buf_thresh12 << 24) | (dsc->rc_buf_thresh13 << 16)), 32, 0);

	/* DSC_RC_RANGE_PARAM */
	set_reg(dsc_base + DSC_RC_RANGE_PARAM0,
		((dsc->range_min_qp0 << 27) | (dsc->range_max_qp0 << 22) |
		(dsc->range_bpg_offset0 << 16) | (dsc->range_min_qp1 << 11) |
		(dsc->range_max_qp1 << 6) | (dsc->range_bpg_offset1 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM1,
		((dsc->range_min_qp2 << 27) | (dsc->range_max_qp2 << 22) |
		(dsc->range_bpg_offset2 << 16) | (dsc->range_min_qp3 << 11) |
		(dsc->range_max_qp3 << 6) | (dsc->range_bpg_offset3 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM2,
		((dsc->range_min_qp4 << 27) | (dsc->range_max_qp4 << 22) |
		(dsc->range_bpg_offset4 << 16) | (dsc->range_min_qp5 << 11) |
		(dsc->range_max_qp5 << 6) | (dsc->range_bpg_offset5 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM3,
		((dsc->range_min_qp6 << 27) | (dsc->range_max_qp6 << 22) |
		(dsc->range_bpg_offset6 << 16) | (dsc->range_min_qp7 << 11) |
		(dsc->range_max_qp7 << 6) | (dsc->range_bpg_offset7 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM4,
		((dsc->range_min_qp8 << 27) | (dsc->range_max_qp8 << 22) |
		(dsc->range_bpg_offset8 << 16) | (dsc->range_min_qp9 << 11) |
		(dsc->range_max_qp9 << 6) | (dsc->range_bpg_offset9 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM5,
		((dsc->range_min_qp10 << 27) | (dsc->range_max_qp10 << 22) |
		(dsc->range_bpg_offset10 << 16) | (dsc->range_min_qp11 << 11) |
		(dsc->range_max_qp11 << 6) | (dsc->range_bpg_offset11 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM6,
		((dsc->range_min_qp12 << 27) | (dsc->range_max_qp12 << 22) |
		(dsc->range_bpg_offset12 << 16) | (dsc->range_min_qp13 << 11) |
		(dsc->range_max_qp13 << 6) | (dsc->range_bpg_offset13 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM7,
		((dsc->range_min_qp14 << 27) | (dsc->range_max_qp14 << 22) |
		(dsc->range_bpg_offset14 << 16)), 32, 0);
}

static void set_dsc_reg_basic(char __iomem *dsc_base)
{
	if (dsc_base == NULL) {
		DPU_FB_ERR("dsc_base is NULL");
		return;
	}

	set_reg(dsc_base + DSC_CLK_SEL, 0x7, 32, 0); /* no lp */
	set_reg(dsc_base + DSC_CLK_EN, 0x7, 32, 0);
	set_reg(dsc_base + DSC_MEM_CTRL, 0x0, 32, 0);
	set_reg(dsc_base + DSC_ST_DATAIN, 0x0, 28, 0);
	set_reg(dsc_base + DSC_ST_DATAOUT, 0x0, 16, 0);
	set_reg(dsc_base + DSC0_ST_SLC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC1_ST_SLC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC0_ST_PIC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC1_ST_PIC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC0_ST_FIFO, 0x0, 14, 0);
	set_reg(dsc_base + DSC1_ST_FIFO, 0x0, 14, 0);
	set_reg(dsc_base + DSC0_ST_LINEBUF, 0x0, 24, 0);
	set_reg(dsc_base + DSC1_ST_LINEBUF, 0x0, 24, 0);
	set_reg(dsc_base + DSC_ST_ITFC, 0x0, 10, 0);
}

static void set_dsc_reg(struct dpu_fb_data_type *dpufd, struct dsc_info_mipi *input_dsc_info)
{
	char __iomem *dsc_base = NULL;
	struct dpu_panel_info *pinfo = NULL;
	struct dsc_panel_info *dsc = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	if (input_dsc_info == NULL) {
		DPU_FB_ERR("out_dsc_info is NULL");
		return;
	}

	pinfo = &(dpufd->panel_info);
	dsc = &(pinfo->vesa_dsc);

	dsc_base = dpufd->dss_base + DSS_DSC_OFFSET;

	set_reg(dsc_base + DSC_REG_DEFAULT, 0x1, 1, 0);

	set_dsc_reg_ctrl(dsc_base, dsc, input_dsc_info);
	set_dsc_reg_rctable(dsc_base, dsc, input_dsc_info);
	set_dsc_reg_basic(dsc_base);

	set_reg(dsc_base + DSC_REG_DEFAULT, 0x0, 1, 0);
}

static void init_dsc(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dsc_info_mipi dsc_info = {0};

	pinfo = &(dpufd->panel_info);


	config_dsc_para(dpufd, &dsc_info);
	set_dsc_reg(dpufd, &dsc_info);
}

static void dss_dsc_version_cfg(char __iomem *dsc_base, struct panel_dsc_info *dsc)
{
	DPU_FB_DEBUG("[DSC] +\n");
	dpu_check_and_no_retval((dsc == NULL || dsc_base == NULL), ERR, "NULL POINTER");

	set_reg(dsc_base + DSC_REG_DEFAULT, 0x1, 1, 0);
	if (dsc->dsc_info.dsc_version_minor == 2)
		set_reg(dsc_base + DSC_VERSION, 0x2, 4, 0);
	else
		set_reg(dsc_base + DSC_VERSION, 0x1, 4, 0);

	set_reg(dsc_base + DSC_EN, dsc->dsc_en, 4, 0);
}

static void dss_dsc_ctrl_cfg(char __iomem *dsc_base, struct panel_dsc_info *dsc)
{
	dpu_check_and_no_retval((dsc == NULL || dsc_base == NULL), ERR, "NULL POINTER");

	if (dsc->dsc_insert_byte_num != 0) {
		DPU_FB_INFO("dsc_insert_byte_num = %d\n", dsc->dsc_insert_byte_num);
		set_reg(dsc_base + DSC_CTRL,
			 dsc->dsc_info.dsc_bpc |
			(dsc->dsc_info.linebuf_depth << 4) |
			(dsc->dsc_insert_byte_num << 8) |
			(dsc->dsc_info.block_pred_enable << 26) |
			(dsc->dsc_info.convert_rgb << 11) |
			(dsc->dsc_info.dsc_bpp << 16) |
			(dsc->dsc_info.native_422 << 13) |
			(dsc->idata_422 << 27), 32, 0);
	} else {
		set_reg(dsc_base + DSC_CTRL,
			 dsc->dsc_info.dsc_bpc |
			(dsc->dsc_info.linebuf_depth << 4) |
			(dsc->dsc_info.block_pred_enable << 26) |
			(dsc->dsc_info.convert_rgb << 11) |
			(dsc->dsc_info.dsc_bpp << 16) |
			(dsc->dsc_info.native_422 << 13) |
			(dsc->idata_422 << 27), 32, 0);
	}
	if (dsc->dsc_info.dsc_version_minor == 2)
		set_reg(dsc_base + 0xC4, 0x2, 10, 0);
}

static void dss_dsc_basic_cfg(char __iomem *dsc_base, struct panel_dsc_info *dsc)
{
	dpu_check_and_no_retval((dsc == NULL || dsc_base == NULL), ERR, "NULL POINTER");

	set_reg(dsc_base + DSC_PIC_SIZE,
		((dsc->dsc_info.pic_width - 1) << 16) | (dsc->dsc_info.pic_height - 1), 32, 0);
	set_reg(dsc_base + DSC_SLICE_SIZE,
		((dsc->dsc_info.slice_width - 1) << 16) | (dsc->dsc_info.slice_height - 1), 32, 0);
	set_reg(dsc_base + DSC_CHUNK_SIZE,
		dsc->dsc_info.chunk_size, 16, 0);
	set_reg(dsc_base + DSC_INITIAL_DELAY, dsc->dsc_info.initial_xmit_delay |
		((dsc->dsc_info.initial_dec_delay) << 16), 32, 0);
	set_reg(dsc_base + DSC_RC_PARAM0,
		dsc->dsc_info.initial_scale_value | (dsc->dsc_info.scale_increment_interval << 16), 32, 0);
	set_reg(dsc_base + DSC_RC_PARAM1,
		(dsc->dsc_info.first_line_bpg_offset << 16) | dsc->dsc_info.scale_decrement_interval, 21, 0);
	set_reg(dsc_base + DSC_RC_PARAM2,
		dsc->dsc_info.nfl_bpg_offset | (dsc->dsc_info.slice_bpg_offset << 16), 32, 0);
	set_reg(dsc_base + DSC_RC_PARAM3,
		((dsc->dsc_info.final_offset << 16) | dsc->dsc_info.initial_offset), 32, 0);
	if (dsc->dsc_info.dsc_version_minor == 2)
		set_reg(dsc_base + DSC_FLATNESS_QP_TH, ((dsc->dsc_info.flatness_max_qp << 16) |
			(((uint8_t)dsc->dsc_info.flatness_min_qp) << 0)), 24, 0);
	else
		set_reg(dsc_base + DSC_FLATNESS_QP_TH, ((0x0 << 24) | (dsc->dsc_info.flatness_max_qp << 16) |
			(0x0 << 8) | ((uint8_t)dsc->dsc_info.flatness_min_qp)), 32, 0);
	set_reg(dsc_base + DSC_RC_PARAM4,
		((dsc->dsc_info.rc_edge_factor << 20) | (dsc->dsc_info.rc_model_size << 0)), 24, 0);
	set_reg(dsc_base + DSC_RC_PARAM5,
		((dsc->dsc_info.rc_tgt_offset_lo << 20) | (dsc->dsc_info.rc_tgt_offset_hi << 16) |
		(dsc->dsc_info.rc_quant_incr_limit1 << 8) | (dsc->dsc_info.rc_quant_incr_limit0 << 0)), 24, 0);
}

static void dss_dsc_rc_thresh_cfg(char __iomem *dsc_base, struct panel_dsc_info *dsc)
{
	dpu_check_and_no_retval((dsc == NULL || dsc_base == NULL), ERR, "NULL POINTER");
	set_reg(dsc_base + DSC_RC_BUF_THRESH0,
		((dsc->dsc_info.rc_buf_thresh[0] << 24) | (dsc->dsc_info.rc_buf_thresh[1] << 16) |
		(dsc->dsc_info.rc_buf_thresh[2] << 8) | (dsc->dsc_info.rc_buf_thresh[3] << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH1,
		((dsc->dsc_info.rc_buf_thresh[4] << 24) | (dsc->dsc_info.rc_buf_thresh[5] << 16) |
		(dsc->dsc_info.rc_buf_thresh[6] << 8) | (dsc->dsc_info.rc_buf_thresh[7] << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH2,
		((dsc->dsc_info.rc_buf_thresh[8] << 24) | (dsc->dsc_info.rc_buf_thresh[9] << 16) |
		(dsc->dsc_info.rc_buf_thresh[10] << 8) | (dsc->dsc_info.rc_buf_thresh[11] << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH3,
		((dsc->dsc_info.rc_buf_thresh[12] << 24) | (dsc->dsc_info.rc_buf_thresh[13] << 16)), 32, 0);
}

static void dss_dsc_rc_range_cfg(char __iomem *dsc_base, struct panel_dsc_info *dsc)
{
	dpu_check_and_no_retval((dsc == NULL || dsc_base == NULL), ERR, "NULL POINTER");
	set_reg(dsc_base + DSC_RC_RANGE_PARAM0,
		((dsc->dsc_info.rc_range[0].min_qp << 27) | (dsc->dsc_info.rc_range[0].max_qp << 22) |
		(dsc->dsc_info.rc_range[0].offset << 16) | (dsc->dsc_info.rc_range[1].min_qp << 11) |
		(dsc->dsc_info.rc_range[1].max_qp << 6) | (dsc->dsc_info.rc_range[1].offset << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM1,
		((dsc->dsc_info.rc_range[2].min_qp << 27) | (dsc->dsc_info.rc_range[2].max_qp << 22) |
		(dsc->dsc_info.rc_range[2].offset << 16) | (dsc->dsc_info.rc_range[3].min_qp << 11) |
		(dsc->dsc_info.rc_range[3].max_qp << 6) | (dsc->dsc_info.rc_range[3].offset << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM2,
		((dsc->dsc_info.rc_range[4].min_qp << 27) | (dsc->dsc_info.rc_range[4].max_qp << 22) |
		(dsc->dsc_info.rc_range[4].offset << 16) | (dsc->dsc_info.rc_range[5].min_qp << 11) |
		(dsc->dsc_info.rc_range[5].max_qp << 6) | (dsc->dsc_info.rc_range[5].offset << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM3,
		((dsc->dsc_info.rc_range[6].min_qp << 27) | (dsc->dsc_info.rc_range[6].max_qp << 22) |
		(dsc->dsc_info.rc_range[6].offset << 16) | (dsc->dsc_info.rc_range[7].min_qp << 11) |
		(dsc->dsc_info.rc_range[7].max_qp << 6) | (dsc->dsc_info.rc_range[7].offset << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM4,
		((dsc->dsc_info.rc_range[8].min_qp << 27) | (dsc->dsc_info.rc_range[8].max_qp << 22) |
		(dsc->dsc_info.rc_range[8].offset << 16) | (dsc->dsc_info.rc_range[9].min_qp << 11) |
		(dsc->dsc_info.rc_range[9].max_qp << 6) | (dsc->dsc_info.rc_range[9].offset << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM5,
		((dsc->dsc_info.rc_range[10].min_qp << 27) | (dsc->dsc_info.rc_range[10].max_qp << 22) |
		(dsc->dsc_info.rc_range[10].offset << 16) | (dsc->dsc_info.rc_range[11].min_qp << 11) |
		(dsc->dsc_info.rc_range[11].max_qp << 6) | (dsc->dsc_info.rc_range[11].offset << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM6,
		((dsc->dsc_info.rc_range[12].min_qp << 27) | (dsc->dsc_info.rc_range[12].max_qp << 22) |
		(dsc->dsc_info.rc_range[12].offset << 16) | (dsc->dsc_info.rc_range[13].min_qp << 11) |
		(dsc->dsc_info.rc_range[13].max_qp << 6) | (dsc->dsc_info.rc_range[13].offset << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM7,
		((dsc->dsc_info.rc_range[14].min_qp << 27) | (dsc->dsc_info.rc_range[14].max_qp << 22) |
		(dsc->dsc_info.rc_range[14].offset << 16)), 32, 0);
}

static void dss_dsc_bits_cfg(char __iomem *dsc_base, struct panel_dsc_info *dsc)
{
	dpu_check_and_no_retval((dsc == NULL || dsc_base == NULL), ERR, "NULL POINTER");
	if (dsc->dsc_info.dsc_version_minor != 2) {
		set_reg(dsc_base + DSC_ADJUSTMENT_BITS, dsc->adjustment_bits, 4, 0);
		set_reg(dsc_base + DSC_BITS_PER_GRP, dsc->bits_per_grp | (dsc->adj_bits_per_grp << 8), 14, 0);
	}
}

static void dss_dsc_multi_slice_cfg(char __iomem *dsc_base, struct panel_dsc_info *dsc)
{
	dpu_check_and_no_retval((dsc == NULL || dsc_base == NULL), ERR, "NULL POINTER");

	set_reg(dsc_base + DSC_MULTI_SLICE_CTL, dsc->slices_per_line | (dsc->pic_line_grp_num << 16), 32, 0);
}

static void dss_dsc_out_mode_cfg(char __iomem *dsc_base, struct panel_dsc_info *dsc)
{
	dpu_check_and_no_retval((dsc == NULL || dsc_base == NULL), ERR, "NULL POINTER");

	if (((dsc->dsc_info.chunk_size + dsc->dsc_insert_byte_num) % 6) == 0) {
		set_reg(dsc_base + DSC_OUT_CTRL, 0x0, 1, 0);
		DPU_FB_DEBUG("dsc->dsc_insert_byte_num 6\n");
	} else if (((dsc->dsc_info.chunk_size + dsc->dsc_insert_byte_num) % 4) == 0) {
		set_reg(dsc_base + DSC_OUT_CTRL, 0x1, 1, 0);
		DPU_FB_DEBUG("dsc->dsc_insert_byte_num 4\n");
	} else {
		DPU_FB_ERR("chunk_size should be mode by 3 or 2, but chunk_size = %u\n", dsc->dsc_info.chunk_size);
		return;
	}
}

static void dss_dsc_normal_cfg(char __iomem *dsc_base, struct panel_dsc_info *dsc)
{
	dpu_check_and_no_retval((dsc == NULL || dsc_base == NULL), ERR, "NULL POINTER");
	set_reg(dsc_base + DSC_CLK_SEL, 0x1F, 32, 0);
	set_reg(dsc_base + DSC_CLK_EN, 0x7, 32, 0);
	set_reg(dsc_base + DSC_MEM_CTRL, 0x0, 32, 0);
	set_reg(dsc_base + DSC_ST_DATAIN, 0x0, 28, 0);
	set_reg(dsc_base + DSC_ST_DATAOUT, 0x0, 16, 0);
	set_reg(dsc_base + DSC0_ST_SLC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC1_ST_SLC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC0_ST_PIC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC1_ST_PIC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC0_ST_FIFO, 0x0, 14, 0);
	set_reg(dsc_base + DSC1_ST_FIFO, 0x0, 14, 0);
	set_reg(dsc_base + DSC0_ST_LINEBUF, 0x0, 24, 0);
	set_reg(dsc_base + DSC1_ST_LINEBUF, 0x0, 24, 0);
	set_reg(dsc_base + DSC_ST_ITFC, 0x0, 10, 0);
	set_reg(dsc_base + DSC_REG_DEFAULT, 0x0, 1, 0);
}

void dss_dsc_cfg(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dsc_base = NULL;
	struct dpu_panel_info *pinfo = NULL;
	struct panel_dsc_info *panel_dsc_info = NULL;

	dpu_check_and_no_retval((dpufd == NULL), ERR, "NULL POINTER");
	DPU_FB_DEBUG("+\n");
	pinfo = &(dpufd->panel_info);
	panel_dsc_info = &(pinfo->panel_dsc_info);
	if (dpufd->index == PRIMARY_PANEL_IDX) {
		dsc_base = dpufd->dss_base + DSS_DSC_OFFSET;
		DPU_FB_DEBUG("[dsc] offset_0\n");
	} else {
		DPU_FB_DEBUG("[dsc] offset_1\n");
	}

	dss_dsc_version_cfg(dsc_base, panel_dsc_info);
	dss_dsc_ctrl_cfg(dsc_base, panel_dsc_info);
	dss_dsc_basic_cfg(dsc_base, panel_dsc_info);
	dss_dsc_rc_thresh_cfg(dsc_base, panel_dsc_info);
	dss_dsc_rc_range_cfg(dsc_base, panel_dsc_info);
	dss_dsc_bits_cfg(dsc_base, panel_dsc_info);
	dss_dsc_multi_slice_cfg(dsc_base, panel_dsc_info);
	dss_dsc_out_mode_cfg(dsc_base, panel_dsc_info);
	dss_dsc_normal_cfg(dsc_base, panel_dsc_info);
}

void dss_dsc_calculation(struct dpu_panel_info *pinfo)
{
	struct panel_dsc_info *panel_dsc_info = NULL;

	DPU_FB_DEBUG("[DSC] +\n");
	dpu_check_and_no_retval((pinfo == NULL), ERR, "NULL POINTER");
	panel_dsc_info = &(pinfo->panel_dsc_info);
	panel_dsc_info->adjustment_bits =
		(8 - (panel_dsc_info->dsc_info.dsc_bpp * (panel_dsc_info->dsc_info.slice_width + 1 - 1)) % 8) % 8;
	DPU_FB_DEBUG("adjustment_bits = %d\n", panel_dsc_info->adjustment_bits);
	panel_dsc_info->adj_bits_per_grp = panel_dsc_info->dsc_info.dsc_bpp * 3 - 3;
	DPU_FB_DEBUG("adj_bits_per_grp = %d\n", panel_dsc_info->adj_bits_per_grp);
	panel_dsc_info->bits_per_grp = panel_dsc_info->dsc_info.dsc_bpp * 3;
	DPU_FB_DEBUG("bits_per_grp = %d\n", panel_dsc_info->bits_per_grp);

	panel_dsc_info->slices_per_line = 0;
	if (panel_dsc_info->dsc_info.native_422 == 0x1)
		panel_dsc_info->pic_line_grp_num =
			(((panel_dsc_info->dsc_info.slice_width + 1) >> 1) + 2) / 3 *
			(panel_dsc_info->slices_per_line + 1) - 1;
	else
		panel_dsc_info->pic_line_grp_num =
			((panel_dsc_info->dsc_info.slice_width + 3) / 3) * (panel_dsc_info->slices_per_line + 1) - 1;
	DPU_FB_DEBUG("pic_line_grp_num = %d\n", panel_dsc_info->pic_line_grp_num);
	panel_dsc_info->dsc_insert_byte_num = 0;
	if (panel_dsc_info->dsc_info.chunk_size % 6)
		panel_dsc_info->dsc_insert_byte_num = (6 - panel_dsc_info->dsc_info.chunk_size % 6);
	DPU_FB_DEBUG("dsc_insert_byte_num = %d\n", panel_dsc_info->dsc_insert_byte_num);
}

void dss_dsc_calc_dsc_ctrl(struct dpu_panel_info *pinfo)
{
	struct panel_dsc_info *panel_dsc_info = NULL;
	uint32_t dsc_en = 0;
	uint32_t dual_dsc_en = 0;
	uint32_t reset_ich_per_line = 0;

	DPU_FB_DEBUG("[DSC] +\n");
	dpu_check_and_no_retval((pinfo == NULL), ERR, "NULL POINTER");
	panel_dsc_info = &(pinfo->panel_dsc_info);
	if ((pinfo->ifbc_type == IFBC_TYPE_VESA2X_SINGLE) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA3_75X_SINGLE) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA4X_SINGLE_SPR) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA2X_SINGLE_SPR)) {
		dsc_en = 0x1;
		dual_dsc_en = 0;
		reset_ich_per_line = 0;
	} else {
		dsc_en = 0x1;
		dual_dsc_en = 1;
		reset_ich_per_line = 1;
	}

	panel_dsc_info->dsc_en = (reset_ich_per_line << 3) | (dual_dsc_en << 1) | dsc_en;

	panel_dsc_info->dual_dsc_en = dual_dsc_en;

	if ((panel_dsc_info->dsc_info.native_422) == 0x1) {
		panel_dsc_info->idata_422 = 0x1;
		panel_dsc_info->dsc_info.convert_rgb = 0x0;
	} else {
		DPU_FB_DEBUG("[DSC] native_422 0\n");
		panel_dsc_info->idata_422 = 0x0;
		panel_dsc_info->dsc_info.convert_rgb = 0x1;
	}
}

static void dss_dsc_do_calculation(struct dpu_panel_info *pinfo)
{
	DPU_FB_DEBUG("+\n");
	dss_dsc_calc_dsc_ctrl(pinfo);
	dss_dsc_calculation(pinfo);
}

void init_ifbc(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;

	DPU_FB_DEBUG("+\n");
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is null.\n");
		return;
	}
	pinfo = &(dpufd->panel_info);
	if (pinfo->ifbc_type >= IFBC_TYPE_MAX) {
		DPU_FB_ERR("ifbc_type is larger than IFBC_TYPE_MAX.\n");
		return;
	}

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_IFBC))
		return;

	if (dpufd->index != PRIMARY_PANEL_IDX) {
		DPU_FB_INFO("fb%d not support ifbc!", dpufd->index);
		return;
	}

	hisi_dss_dsc_set_reg_default_value(dpufd);

	if (is_ifbc_vesa_panel(dpufd)) {
		init_dsc(dpufd);
	}
}

static void set_post_scf_skin_reg(char __iomem *scf_base)
{
	outp32(scf_base + ARSR_POST_SKIN_THRES_Y, 0x2585312C);
	outp32(scf_base + ARSR_POST_SKIN_THRES_U, 0x1C40A014);
	outp32(scf_base + ARSR_POST_SKIN_THRES_V, 0x2440C018);
	outp32(scf_base + ARSR_POST_SKIN_CFG0, 0x00018200);
	outp32(scf_base + ARSR_POST_SKIN_CFG1, 0x00000333);
	outp32(scf_base + ARSR_POST_SKIN_CFG2, 0x000002AA);
}

static void set_post_scf_shoot_reg(char __iomem *scf_base)
{
	outp32(scf_base + ARSR_POST_SHOOT_CFG1, 0x00140155);
	outp32(scf_base + ARSR_POST_SHOOT_CFG2, 0x001007B0);
	outp32(scf_base + ARSR_POST_SHOOT_CFG3, 0x00000014);
}

static void set_post_scf_sharp_reg(char __iomem *scf_base)
{
	outp32(scf_base + ARSR_POST_SHARP_CFG3, 0x00A00060);
	outp32(scf_base + ARSR_POST_SHARP_CFG4, 0x00600020);
	outp32(scf_base + ARSR_POST_SHARP_CFG5, 0);
	outp32(scf_base + ARSR_POST_SHARP_CFG6, 0x00040008);
	outp32(scf_base + ARSR_POST_SHARP_CFG7, 0x0000060A);
	outp32(scf_base + ARSR_POST_SHARP_CFG8, 0x00A00010);
	outp32(scf_base + ARSR_POST_SHARP_LEVEL, 0x20002);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_LOW, 0x3C0078);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_MID, 0x6400C8);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_HIGH, 0x5000A0);
	outp32(scf_base + ARSR_POST_SHARP_GAINCTRLSLOPH_MF, 0x280);
	outp32(scf_base + ARSR_POST_SHARP_GAINCTRLSLOPL_MF, 0x1400);
	outp32(scf_base + ARSR_POST_SHARP_GAINCTRLSLOPH_HF, 0x140);
	outp32(scf_base + ARSR_POST_SHARP_GAINCTRLSLOPL_HF, 0xA00);
	outp32(scf_base + ARSR_POST_SHARP_MF_LMT, 0x40);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_MF, 0x12C012C);
	outp32(scf_base + ARSR_POST_SHARP_MF_B, 0);
	outp32(scf_base + ARSR_POST_SHARP_HF_LMT, 0x80);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_HF, 0x104012C);
	outp32(scf_base + ARSR_POST_SHARP_HF_B, 0x1400);
	outp32(scf_base + ARSR_POST_SHARP_LF_CTRL, 0x100010);
	outp32(scf_base + ARSR_POST_SHARP_LF_VAR, 0x1800080);
	outp32(scf_base + ARSR_POST_SHARP_LF_CTRL_SLOP, 0);
	outp32(scf_base + ARSR_POST_SHARP_HF_SELECT, 0);
	outp32(scf_base + ARSR_POST_SHARP_CFG2_H, 0x10000C0);
	outp32(scf_base + ARSR_POST_SHARP_CFG2_L, 0x200010);
}

void init_post_scf(struct dpu_fb_data_type *dpufd)
{
	char __iomem *scf_lut_base;
	char __iomem *scf_base;
	int ihright;
	int ihright1;
	int ivbottom;
	struct dpu_panel_info *pinfo = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	pinfo = &(dpufd->panel_info);

	scf_lut_base = dpufd->dss_base + DSS_POST_SCF_LUT_OFFSET;

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_POST_SCF))
		return;

	/* use arsr_init_update to set arsr1p update status in dual-panel display effect */
	dpufd->effect_info[pinfo->disp_panel_id].arsr1p[0].update = dpufd->effect_init_update[pinfo->disp_panel_id].arsr1p_update_normal;
	dpufd->effect_info[pinfo->disp_panel_id].arsr1p[1].update = dpufd->effect_init_update[pinfo->disp_panel_id].arsr1p_update_fhd;
	// cppcheck-suppress *
	dpufd->effect_info[pinfo->disp_panel_id].arsr1p[2].update = dpufd->effect_init_update[pinfo->disp_panel_id].arsr1p_update_hd;
	dpufd->effect_info[pinfo->disp_panel_id].arsr2p[0].update = dpufd->effect_init_update[pinfo->disp_panel_id].arsr2p_update_normal;
	dpufd->effect_info[pinfo->disp_panel_id].arsr2p[1].update = dpufd->effect_init_update[pinfo->disp_panel_id].arsr2p_update_scale_up;
	// cppcheck-suppress *
	dpufd->effect_info[pinfo->disp_panel_id].arsr2p[2].update = dpufd->effect_init_update[pinfo->disp_panel_id].arsr2p_update_scale_down;

	pinfo->post_scf_support = 1;
	hisi_dss_arsr_post_coef_on(dpufd);

	scf_base = dpufd->dss_base + DSS_POST_SCF_OFFSET;

	set_post_scf_skin_reg(scf_base);
	set_post_scf_shoot_reg(scf_base);
	set_post_scf_sharp_reg(scf_base);

	outp32(scf_base + ARSR_POST_TEXTURE_ANALYSIS, 0x500040);
	outp32(scf_base + ARSR_POST_INTPLSHOOTCTRL, 0x8);

	ihright1 = ((int)pinfo->xres - 1) * ARSR1P_INC_FACTOR;
	ihright = ihright1 + 2 * ARSR1P_INC_FACTOR;
	if (ihright >= ((int)pinfo->xres) * ARSR1P_INC_FACTOR)
		ihright = ((int)pinfo->xres) * ARSR1P_INC_FACTOR - 1;

	ivbottom = ((int)pinfo->yres - 1) * ARSR1P_INC_FACTOR;
	if (ivbottom >= ((int)pinfo->yres) * ARSR1P_INC_FACTOR)
		ivbottom = ((int)pinfo->yres) * ARSR1P_INC_FACTOR - 1;

	outp32(scf_base + ARSR_POST_IHLEFT, 0x0);
	outp32(scf_base + ARSR_POST_IHRIGHT, ihright);
	outp32(scf_base + ARSR_POST_IHLEFT1, 0x0);
	outp32(scf_base + ARSR_POST_IHRIGHT1, ihright1);
	outp32(scf_base + ARSR_POST_IVTOP, 0x0);
	outp32(scf_base + ARSR_POST_IVBOTTOM, ivbottom);
	outp32(scf_base + ARSR_POST_IVBOTTOM1, ivbottom);
	outp32(scf_base + ARSR_POST_IHINC, ARSR1P_INC_FACTOR);
	outp32(scf_base + ARSR_POST_IVINC, ARSR1P_INC_FACTOR);

	outp32(scf_base + ARSR_POST_MODE, 0x1);
}

static int init_dbuf_calculate_rect_width(struct dpu_fb_data_type *dpufd)
{
	uint32_t ifbc_type;
	uint32_t mipi_idx;
	uint32_t rect_width;
	uint32_t xres_div;
	struct dpu_panel_info *pinfo = NULL;
	struct panel_dsc_info *panel_dsc_info = NULL;

	pinfo = &(dpufd->panel_info);
	panel_dsc_info = &(pinfo->panel_dsc_info);

	if (is_dp_panel(dpufd)) {
		rect_width = pinfo->xres;
	} else {
		mipi_idx = 0;
		ifbc_type = pinfo->ifbc_type;
		xres_div = g_mipi_ifbc_division[mipi_idx][ifbc_type].xres_div;

		if ((pinfo->mode_switch_to == MODE_10BIT_VIDEO_3X) && (pinfo->ifbc_type == IFBC_TYPE_VESA3X_DUAL)) {
			/* data size of 10bit vesa3x is larger than 10bit vesa3.75x
			 * xres_div is enum XRES_DIV, not 0
			 */
			rect_width = pinfo->xres * 30 / 24 / xres_div;
		} else {
			if ((pinfo->xres % xres_div) > 0)
				rect_width = ((panel_dsc_info->dsc_info.chunk_size + panel_dsc_info->dsc_insert_byte_num)
				* (panel_dsc_info->dual_dsc_en + 1)) / xres_div;
			else
				rect_width = pinfo->xres / xres_div;
		}
	}

	return rect_width;
}

static void set_dbuf_reg(struct dpu_fb_data_type *dpufd, char __iomem *dbuf_base,
	struct thd thd_datas, struct ram_info ram_data, uint32_t rect_width)
{
	struct dpu_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);
	outp32(dbuf_base + DBUF_FRM_SIZE, rect_width * (pinfo->yres));
	outp32(dbuf_base + DBUF_FRM_HSIZE, DSS_WIDTH(rect_width));
	outp32(dbuf_base + DBUF_SRAM_VALID_NUM, ram_data.sram_valid_num); /* burst:0x3  non_burst:0x1 */

	outp32(dbuf_base + DBUF_THD_RQOS, (thd_datas.rqos_out << 16) | thd_datas.rqos_in); /* 0x63c058b */
	outp32(dbuf_base + DBUF_THD_CG, (thd_datas.cg_out << 16) |
		thd_datas.cg_in); /* burst:0x1a541bb7  non_burst:0xd290ddb */
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_BEF,
		(thd_datas.flux_req_befdfs_out << 16) | thd_datas.flux_req_befdfs_in); /* 0x58b04d9 */
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_AFT,
		(thd_datas.flux_req_aftdfs_out << 16) | thd_datas.flux_req_aftdfs_in);  /* 0x58b04d9 */
	outp32(dbuf_base + DBUF_THD_DFS_OK, thd_datas.dfs_ok); /* 0x4d9 */
	outp32(dbuf_base + DBUF_FLUX_REQ_CTRL, (0 << 1) | thd_datas.flux_req_sw_en); /* 0x0 */
	outp32(dbuf_base + DBUF_DFS_LP_CTRL, 0x1);
	outp32(dbuf_base + DBUF_DFS_RAM_MANAGE, ram_data.dfs_ram); /* 0x0 */

	outp32(dbuf_base + DBUF_THD_RQOS_IDLE, thd_datas.rqos_idle); /* 0x0 */

	if (dpufd->index == PRIMARY_PANEL_IDX)
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK, 0x0, 1, 0);
	else if (dpufd->index == EXTERNAL_PANEL_IDX)
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK, 0x0, 1, 1);
}

#if defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V345) || defined(CONFIG_HISI_FB_V346)
static void init_dbuf_calculate_thd(struct thd *thd_datas, struct ram_info ram_data)
{
	int depth;

	depth = (ram_data.sram_valid_num + 1) * DBUF_DEPTH;

	/* dbuf threshold data is X% dbuf depth */
	thd_datas->cg_in = depth - 1;
	thd_datas->cg_out = thd_datas->cg_in * 95 / 100;

	thd_datas->rqos_in = DBUF_DEPTH * 80 / 100;
	thd_datas->rqos_out = DBUF_DEPTH * 90 / 100;
	thd_datas->flux_req_befdfs_in = DBUF_DEPTH * 70 / 100;
	thd_datas->flux_req_befdfs_out = DBUF_DEPTH * 80 / 100;
	thd_datas->flux_req_aftdfs_in = thd_datas->flux_req_befdfs_in;
	thd_datas->flux_req_aftdfs_out = thd_datas->flux_req_befdfs_out;
	thd_datas->dfs_ok = thd_datas->flux_req_befdfs_in;
	thd_datas->flux_req_sw_en = 0;
	thd_datas->wqos_in = 0;
	thd_datas->wqos_out = 0;
	thd_datas->wr_wait = 0;
	thd_datas->cg_hold = 0;
	thd_datas->rqos_idle = 0;

	DPU_FB_INFO("depth=%d,\n"
		"ram_data.sram_valid_num=0x%x\n"
		"thd_datas->cg_in=0x%x\n"
		"thd_datas->cg_out=0x%x\n"
		"thd_datas->rqos_in=0x%x\n"
		"thd_datas->rqos_out=0x%x\n"
		"thd_datas->flux_req_befdfs_in=0x%x\n"
		"thd_datas->flux_req_befdfs_out=0x%x\n"
		"thd_datas->flux_req_aftdfs_in=0x%x\n"
		"thd_datas->flux_req_aftdfs_out=0x%x\n"
		"thd_datas->dfs_ok=0x%x\n",
		DBUF_DEPTH,
		ram_data.sram_valid_num,
		thd_datas->cg_in,
		thd_datas->cg_out,
		thd_datas->rqos_in,
		thd_datas->rqos_out,
		thd_datas->flux_req_befdfs_in,
		thd_datas->flux_req_befdfs_out,
		thd_datas->flux_req_aftdfs_in,
		thd_datas->flux_req_aftdfs_out,
		thd_datas->dfs_ok);
}



void init_dbuf(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dbuf_base = NULL;
	struct dpu_panel_info *pinfo = NULL;
	struct thd thd_datas;
	struct ram_info ram_data;
	uint32_t rect_width;

	dpu_check_and_no_retval((!dpufd), ERR, "dpufd is NULL.\n");
	pinfo = &(dpufd->panel_info);

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_DBUF)) {
		DPU_FB_ERR("HISI_DSS_SUPPORT_DPP_MODULE_BIT not support\n");
		return;
	}

	dpu_check_and_no_retval((dpufd->index != PRIMARY_PANEL_IDX),  ERR, "dpufd index not support\n");

	dbuf_base = dpufd->dss_base + DSS_DBUF0_OFFSET;

	rect_width = init_dbuf_calculate_rect_width(dpufd);

	if (pinfo->pxl_clk_rate_div <= 0)
		pinfo->pxl_clk_rate_div = 1;

	ram_data.sram_valid_num = 1;
	ram_data.dfs_ram = 0;

	init_dbuf_calculate_thd(&thd_datas, ram_data);

	set_dbuf_reg(dpufd, dbuf_base, thd_datas, ram_data, rect_width);
}

#else
static void init_dbuf_calculate_thd(struct thd *thd_datas, struct ram_info ram_data)
{
	int depth;

	depth = (ram_data.sram_valid_num + 1) * DBUF_DEPTH;

	/* dbuf threshold data is X% dbuf depth */
	thd_datas->cg_in = depth - 1;
	thd_datas->cg_out = thd_datas->cg_in * 95 / 100;

	thd_datas->rqos_in = depth * 80 / 100;
	thd_datas->rqos_out = depth * 90 / 100;
	thd_datas->rqos_idle = depth;
	thd_datas->flux_req_befdfs_in = depth * 70 / 100;
	thd_datas->flux_req_befdfs_out = depth * 80 / 100;
	thd_datas->flux_req_aftdfs_in = thd_datas->flux_req_befdfs_in;
	thd_datas->flux_req_aftdfs_out = thd_datas->flux_req_befdfs_out;
	thd_datas->dfs_ok = thd_datas->flux_req_befdfs_in;
	thd_datas->flux_req_sw_en = 1;
	thd_datas->wqos_in = 0;
	thd_datas->wqos_out = 0;
	thd_datas->wr_wait = 0;
	thd_datas->cg_hold = 0;

	DPU_FB_INFO("ram_data.sram_valid_num=%d,\n"
		"thd_datas->rqos_in=0x%x\n"
		"thd_datas->rqos_out=0x%x\n"
		"thd_datas->cg_in=0x%x\n"
		"thd_datas->cg_out=0x%x\n"
		"thd_datas->flux_req_befdfs_in=0x%x\n"
		"thd_datas->flux_req_befdfs_out=0x%x\n"
		"thd_datas->flux_req_aftdfs_in=0x%x\n"
		"thd_datas->flux_req_aftdfs_out=0x%x\n"
		"thd_datas->dfs_ok=0x%x\n",
		ram_data.sram_valid_num,
		thd_datas->rqos_in,
		thd_datas->rqos_out,
		thd_datas->cg_in,
		thd_datas->cg_out,
		thd_datas->flux_req_befdfs_in,
		thd_datas->flux_req_befdfs_out,
		thd_datas->flux_req_aftdfs_in,
		thd_datas->flux_req_aftdfs_out,
		thd_datas->dfs_ok);
}

void init_dbuf(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dbuf_base = NULL;
	struct dpu_panel_info *pinfo = NULL;
	struct thd thd_datas;
	struct ram_info ram_data;
	uint32_t rect_width;

	dpu_check_and_no_retval((!dpufd), ERR, "dpufd is NULL.\n");
	pinfo = &(dpufd->panel_info);

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_DBUF)) {
		DPU_FB_ERR("HISI_DSS_SUPPORT_DPP_MODULE_BIT not support\n");
		return;
	}

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		dbuf_base = dpufd->dss_base + DSS_DBUF0_OFFSET;
		ram_data.sram_valid_num = 0;
		ram_data.dfs_ram = 0xFFF0;
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		dbuf_base = dpufd->dss_base + DSS_DBUF1_OFFSET;
		ram_data.sram_valid_num = 2;
		ram_data.dfs_ram = 0x111F;
	} else {
		DPU_FB_INFO("fb%d, not support!", dpufd->index);
		return;
	}

	rect_width = init_dbuf_calculate_rect_width(dpufd);

	if (pinfo->pxl_clk_rate_div <= 0)
		pinfo->pxl_clk_rate_div = 1;

	init_dbuf_calculate_thd(&thd_datas, ram_data);

	set_dbuf_reg(dpufd, dbuf_base, thd_datas, ram_data, rect_width);
}
#endif

void deinit_dbuf(struct dpu_fb_data_type *dpufd)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	if (dpufd->index == PRIMARY_PANEL_IDX)
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK, 0x1, 1, 0);
	else if (dpufd->index == EXTERNAL_PANEL_IDX)
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK, 0x1, 1, 1);
}

static void init_dual_mipi_ctrl(struct dpu_fb_data_type *dpufd)
{
	dss_rect_t rect = { 0, 0, 0, 0 };
	uint32_t dmipi_hsize;

	rect.x = 0;
	rect.y = 0;
	rect.w = dpufd->panel_info.xres;
	rect.h = dpufd->panel_info.yres;

	mipi_ifbc_get_rect(dpufd, &rect);
	dmipi_hsize = (((uint32_t)rect.w * 2 - 1) << 16) | ((uint32_t)rect.w - 1);

	set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DUAL_MIPI_HSIZE, dmipi_hsize, 32, 0);
	set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DUAL_MIPI_SWAP, 0x0, 1, 0);
}

static char __iomem *get_pipe_sw_dsi_base(struct dpu_fb_data_type *dpufd)
{
	char __iomem *pipe_sw_dsi_base = NULL;

	pipe_sw_dsi_base = dpufd->dss_base + DSS_PIPE_SW_DSI0_OFFSET;
	if (is_dsi1_pipe_switch_connector(dpufd))
		pipe_sw_dsi_base = dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET;
	return pipe_sw_dsi_base;
}

static void init_pipe_sw(struct dpu_fb_data_type *dpufd)
{
	char __iomem *pipe_sw_dsi_base = NULL;

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		pipe_sw_dsi_base = get_pipe_sw_dsi_base(dpufd);

		set_reg(pipe_sw_dsi_base + PIPE_SW_SIG_CTRL, 0x1, 8, 0);
		set_reg(pipe_sw_dsi_base + SW_POS_CTRL_SIG_EN, 0x1, 1, 0);
		set_reg(pipe_sw_dsi_base + PIPE_SW_DAT_CTRL, 0x1, 8, 0);
		set_reg(pipe_sw_dsi_base + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);

		if (is_dual_mipi_panel(dpufd)) {
			set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_SIG_CTRL, 0x1, 8, 0);
			set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI0_OFFSET + SW_POS_CTRL_SIG_EN, 0x0, 1, 0);
			set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_SIG_EN, 0x1, 1, 0);
			set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_DAT_CTRL, 0x1, 8, 0);
			set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);
			init_dual_mipi_ctrl(dpufd);
		}
	} else if ((dpufd->index == EXTERNAL_PANEL_IDX) && is_dp_panel(dpufd)) {
		set_reg(dpufd->dss_base + DSS_PIPE_SW_DP_OFFSET + PIPE_SW_SIG_CTRL, 0x2, 8, 0);
		set_reg(dpufd->dss_base + DSS_PIPE_SW_DP_OFFSET + SW_POS_CTRL_SIG_EN, 0x1, 1, 0);
		set_reg(dpufd->dss_base + DSS_PIPE_SW_DP_OFFSET + PIPE_SW_DAT_CTRL, 0x2, 8, 0);
		set_reg(dpufd->dss_base + DSS_PIPE_SW_DP_OFFSET + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);
	} else if ((dpufd->index == EXTERNAL_PANEL_IDX) && is_mipi_panel(dpufd)) {
		set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_SIG_CTRL, 0x2, 8, 0);
		set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_SIG_EN, 0x1, 1, 0);
		set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_DAT_CTRL, 0x2, 8, 0);
		set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);
	}
}

static void init_dpp_ifbc_sw(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	pinfo = &(dpufd->panel_info);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DPPSW_SIG_CTRL, 0x20101, 24, 0);
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DPPSW_DAT_CTRL, 0x20101, 24, 0);
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + IFBCSW_SIG_CTRL, 0x403, 16, 0);
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + IFBCSW_DAT_CTRL, 0x403, 16, 0);
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DYN_SW_DEFAULT, 0x0, 1, 0);
		g_dyn_sw_default = 0x0;
	}
}

static void ldi_init_rect(struct dpu_fb_data_type *dpufd, dss_rect_t *rect)
{
	rect->x = 0;
	rect->y = 0;
	rect->w = dpufd->panel_info.xres;
	rect->h = dpufd->panel_info.yres;
}

static void set_ldi_dpio_reg(struct dpu_fb_data_type *dpufd, dss_rect_t rect, char __iomem *ldi_base)
{
	struct dpu_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);

	if (is_mipi_video_panel(dpufd)) {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0,
			pinfo->ldi.h_front_porch | ((pinfo->ldi.h_back_porch +
				DSS_WIDTH(pinfo->ldi.h_pulse_width)) << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, 0);
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(rect.w));
	} else {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0,
			pinfo->ldi.h_front_porch | (pinfo->ldi.h_back_porch << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, DSS_WIDTH(pinfo->ldi.h_pulse_width));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(rect.w));
	}
}

static void set_ldi_dsi_reg(struct dpu_fb_data_type *dpufd, char __iomem *ldi_base)
{
	uint32_t te_source;
	struct dpu_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);

	if (is_mipi_cmd_panel(dpufd)) {
		set_reg(ldi_base + LDI_DSI_CMD_MOD_CTRL, 0x1, 2, 0);

		te_source = 0; /* select te_pin */

		/* dsi_te_hard_en */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x1, 1, 0);
		/* dsi_te0_pin_p , dsi_te1_pin_p */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 2, 1);
		/* dsi_te_hard_sel */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, te_source, 1, 3);
		/* select TE0 PIN */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x01, 2, 6);
		/* dsi_te_mask_en */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 1, 8);
		/* dsi_te_mask_dis */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 4, 9);
		/* dsi_te_mask_und */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 4, 13);
		/* dsi_te_pin_en */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x1, 1, 17);

		set_reg(ldi_base + LDI_DSI_TE_HS_NUM, 0x0, 32, 0);
		set_reg(ldi_base + LDI_DSI_TE_HS_WD, 0x24024, 32, 0);

		/* dsi_te0_vs_wd = lcd_te_width / T_pxl_clk, experience lcd_te_width = 2us */
		if (pinfo->pxl_clk_rate_div == 0) {
			DPU_FB_ERR("pxl_clk_rate_div is NULL, not support !\n");
			pinfo->pxl_clk_rate_div = 1;
		}
		set_reg(ldi_base + LDI_DSI_TE_VS_WD, (0x3FC << 12) | (2 * pinfo->pxl_clk_rate /
			pinfo->pxl_clk_rate_div / 1000000), 32, 0);
	} else {
		/* dsi_te_hard_en */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 1, 0);
		set_reg(ldi_base + LDI_DSI_CMD_MOD_CTRL, 0x2, 2, 0);
	}
}

static void set_ldi_colorbar_used_reg(struct dpu_fb_data_type *dpufd, char __iomem *ldi_base)
{
	/* normal */
	set_reg(ldi_base + LDI_WORK_MODE, 0x1, 1, 0);
}

void init_ldi(struct dpu_fb_data_type *dpufd, bool fastboot_enable)
{
	int i = 0;
	dss_rect_t rect;
	char __iomem *ldi_base = NULL;
	struct dpu_panel_info *pinfo = NULL;

	dpu_check_and_no_retval((!dpufd), ERR, "dpufd is NULL.\n");

	pinfo = &(dpufd->panel_info);

	init_pipe_sw(dpufd);
	init_dpp_ifbc_sw(dpufd);

	dpu_check_and_no_retval((!is_dp_panel(dpufd)), INFO, "fb%d is not dp panel\n", dpufd->index);

	for (i = 0; i < dpufd->dp.streams; i++) {
		ldi_base = (i == 0) ? (dpufd->dss_base + DSS_LDI_DP_OFFSET) : (dpufd->dss_base + DSS_LDI_DP1_OFFSET);
		DPU_FB_INFO("fb%d, fastboot_enable = %d!", dpufd->index, fastboot_enable);
		ldi_init_rect(dpufd, &rect);
		mipi_ifbc_get_rect(dpufd, &rect);
		set_ldi_dpio_reg(dpufd, rect, ldi_base);
		outp32(ldi_base + LDI_VRT_CTRL0, pinfo->ldi.v_front_porch | (pinfo->ldi.v_back_porch << 16));
		outp32(ldi_base + LDI_VRT_CTRL1, DSS_HEIGHT(pinfo->ldi.v_pulse_width));
		outp32(ldi_base + LDI_VRT_CTRL2, DSS_HEIGHT(rect.h));

		outp32(ldi_base + LDI_PLR_CTRL, pinfo->ldi.vsync_plr | (pinfo->ldi.hsync_plr << 1) |
			(pinfo->ldi.pixelclk_plr << 2) | (pinfo->ldi.data_en_plr << 3));

		/* bpp */
		set_reg(ldi_base + LDI_CTRL, pinfo->bpp, 2, 3);
		/* bgr */
		set_reg(ldi_base + LDI_CTRL, pinfo->bgr_fmt, 1, 13);
		/* for ddr pmqos */
		outp32(ldi_base + LDI_VINACT_MSK_LEN, pinfo->ldi.v_front_porch);
		/* cmd event sel */
		outp32(ldi_base + LDI_CMD_EVENT_SEL, 0x1);
		/* double pixel mode 1772 */
		outp32(ldi_base + LDI_MP_MODE, 0x1);
		/* for 1Hz LCD and mipi command LCD */
		set_ldi_dsi_reg(dpufd, ldi_base);
		set_ldi_colorbar_used_reg(dpufd, ldi_base);

		if (is_mipi_cmd_panel(dpufd))
			set_reg(ldi_base + LDI_FRM_MSK, (dpufd->frame_update_flag == 1) ? 0x0 : 0x1, 1, 0);

		if (dpufd->index == EXTERNAL_PANEL_IDX && (is_mipi_panel(dpufd)))
			set_reg(ldi_base + LDI_DP_DSI_SEL, 0x1, 1, 0);

		/* mst mode */
		if (dpufd->dp.mst)
			set_reg(ldi_base + LDI_MST_CONTROL, 0x1, 1, 0);

		if (i == 1)
			set_reg(ldi_base + LDI_WORK_MODE, 0x0, 1, 0);
	}

	if (is_hisync_mode(dpufd))
		dpufb_hisync_disp_sync_enable(dpufd);

	/* ldi disable */
	if (!fastboot_enable)
		disable_ldi(dpufd);
}

static void deinit_pipe_sw(struct dpu_fb_data_type *dpufd)
{
	char __iomem *pipe_sw_dsi_base = NULL;
	dpu_check_and_no_retval((!dpufd), ERR, "dpufd is NULL.\n");

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		pipe_sw_dsi_base = get_pipe_sw_dsi_base(dpufd);

		set_reg(pipe_sw_dsi_base + PIPE_SW_SIG_CTRL, 0x0, 32, 0);
		set_reg(pipe_sw_dsi_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
		set_reg(pipe_sw_dsi_base + PIPE_SW_DAT_CTRL, 0x0, 32, 0);
		set_reg(pipe_sw_dsi_base + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);

		if (is_dual_mipi_panel(dpufd)) {
			set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_SIG_CTRL, 0x0, 32, 0);
			set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
			set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_DAT_CTRL, 0x0, 32, 0);
			set_reg(dpufd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);
			set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DUAL_MIPI_HSIZE, 0, 32, 0);
			set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DUAL_MIPI_SWAP, 0x0, 32, 0);
		}
	}
}

void deinit_ldi(struct dpu_fb_data_type *dpufd)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	if (is_dp_panel(dpufd))
		disable_ldi(dpufd);

	deinit_pipe_sw(dpufd);
}

void enable_ldi(struct dpu_fb_data_type *dpufd)
{
	char __iomem *ldi_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	if (is_dp_panel(dpufd)) {
		ldi_base = dpufd->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);

		ldi_base = dpufd->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
	} else {
		if ((dpufd->index == PRIMARY_PANEL_IDX) && is_dual_mipi_panel(dpufd)) {
			set_reg(dpufd->mipi_dsi0_base + MIPI_LDI_CTRL, 0x1, 1, 5);
			return;
		}

		if (dpufd->index == PRIMARY_PANEL_IDX)
			ldi_base = get_mipi_dsi_base(dpufd);
		else if (dpufd->index == EXTERNAL_PANEL_IDX)
			ldi_base = dpufd->mipi_dsi1_base;
		else
			return;

		set_reg(ldi_base + MIPI_LDI_CTRL, 0x1, 1, 0);
	}
}

void disable_ldi(struct dpu_fb_data_type *dpufd)
{
	char __iomem *ldi_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	if (is_dp_panel(dpufd)) {
		ldi_base = dpufd->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);

		ldi_base = dpufd->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);
	} else {
		if ((dpufd->index == PRIMARY_PANEL_IDX) && is_dual_mipi_panel(dpufd)) {
			set_reg(dpufd->mipi_dsi0_base + MIPI_LDI_CTRL, 0x0, 1, 5);
			return;
		}

		if (dpufd->index == PRIMARY_PANEL_IDX)
			ldi_base = get_mipi_dsi_base(dpufd);
		else if (dpufd->index == EXTERNAL_PANEL_IDX)
			ldi_base = dpufd->mipi_dsi1_base;
		else
			return;

		set_reg(ldi_base + MIPI_LDI_CTRL, 0x0, 1, 0);
	}
}

void ldi_frame_update(struct dpu_fb_data_type *dpufd, bool update)
{
	char __iomem *mipi_dsi_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		mipi_dsi_base = get_mipi_dsi_base(dpufd);
		if (is_mipi_cmd_panel(dpufd)) {
			set_reg(mipi_dsi_base + MIPI_LDI_FRM_MSK, (update ? 0x0 : 0x1), 1, 0);
			if (is_dual_mipi_panel(dpufd))
				set_reg(dpufd->mipi_dsi1_base + MIPI_LDI_FRM_MSK,
					(update ? 0x0 : 0x1), 1, 0);
			if (update)
				enable_ldi(dpufd);
		}
	} else {
		DPU_FB_ERR("fb%d, not support!", dpufd->index);
	}
}

void single_frame_update(struct dpu_fb_data_type *dpufd)
{
	char __iomem *ldi_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	if (is_dp_panel(dpufd)) {
		ldi_base = dpufd->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_FRM_MSK_UP, 0x1, 1, 0);

		ldi_base = dpufd->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_FRM_MSK_UP, 0x1, 1, 0);
	} else if (is_mipi_cmd_panel(dpufd)) {
		if (need_config_dsi0(dpufd))
			set_reg(dpufd->mipi_dsi0_base + MIPI_LDI_FRM_MSK_UP, 0x1, 1, 0);
		if (need_config_dsi1(dpufd))
			set_reg(dpufd->mipi_dsi1_base + MIPI_LDI_FRM_MSK_UP, 0x1, 1, 0);
	}

	enable_ldi(dpufd);
}

void dpe_interrupt_clear(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dss_base = NULL;
	char __iomem *mipi_dsi_base = NULL;
	uint32_t clear;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	dss_base = dpufd->dss_base;

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_PDP_INTS, clear);
		mipi_dsi_base = get_mipi_dsi_base(dpufd);
		outp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INTS, clear);
		if (is_dual_mipi_panel(dpufd))
			outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS, clear);
		outp32(dss_base + DSS_DPP_OFFSET + DPP_INTS, clear);

		outp32(dss_base + DSS_DBG_OFFSET + DBG_MCTL_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH0_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH1_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH2_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH3_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_DSS_GLB_INTS, clear);
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_SDP_INTS, clear);
		if (is_dp_panel(dpufd)) {
			outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INTS, clear);
			outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INTS, clear);
		} else {
			outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS, clear);
		}
	} else if (dpufd->index == AUXILIARY_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_OFF_INTS, clear);
	}
}

static void primary_panel_dpe_interrupt_unmask(struct dpu_fb_data_type *dpufd,
		struct dpu_panel_info *pinfo, char __iomem *dss_base)
{
	char __iomem *mipi_dsi_base = NULL;
	uint32_t pri_unmask = 0;
	uint32_t lcd_te_pin = BIT_LCD_TE0_PIN;
	uint32_t halfway_frm_irq = 0;

	pri_unmask = ~0;
	pri_unmask &= ~(BIT_DPP_INTS | BIT_ITF0_INTS);
	outp32(dss_base + GLB_CPU_PDP_INT_MSK, pri_unmask);

	if (dpufd->panel_info.mipi.dsi_te_type == DSI1_TE1_TYPE)
		lcd_te_pin = BIT_LCD_TE1_PIN;


	pri_unmask = ~0;
	if (is_mipi_cmd_panel(dpufd))
		pri_unmask &= ~(lcd_te_pin | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END | halfway_frm_irq);
	else
		pri_unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);

	mipi_dsi_base = get_mipi_dsi_base(dpufd);
	outp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INT_MSK, pri_unmask);
	if (is_dual_mipi_panel(dpufd)) {
		pri_unmask = ~0;
		outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, pri_unmask);
	}

	pri_unmask = ~0;
	if ((pinfo->acm_ce_support == 1) && !!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_ACE))
		pri_unmask &= ~(BIT_CE_END_IND);

	if (pinfo->hiace_support == 1)
		pri_unmask &= ~(BIT_HIACE_IND);

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DPP_INT_MSK, pri_unmask);
}

void dpe_interrupt_unmask(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dss_base = 0;
	uint32_t unmask = 0;
	struct dpu_panel_info *pinfo = NULL;

	dpu_check_and_no_retval((dpufd == NULL), ERR, "dpufd is NULL!\n");

	pinfo = &(dpufd->panel_info);
	dss_base = dpufd->dss_base;

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		primary_panel_dpe_interrupt_unmask(dpufd, pinfo, dss_base);
	} else if ((dpufd->index == EXTERNAL_PANEL_IDX) &&
		is_mipi_panel(dpufd) && !is_dual_mipi_panel(dpufd)) {
		unmask = ~0;
		unmask &= ~(BIT_SDP_ITF1_INTS | BIT_SDP_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_SDP_INT_MSK, unmask);

		unmask = ~0;
		if (is_mipi_cmd_panel(dpufd))
			unmask &= ~(BIT_LCD_TE0_PIN | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		else
			unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);

		outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, unmask);
	} else if ((dpufd->index == EXTERNAL_PANEL_IDX) && is_dp_panel(dpufd)) {
		unmask = ~0;
		unmask &= ~(BIT_DP_ITF2_INTS | BIT_DP_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_DP_INT_MSK, unmask);

		unmask = ~0;
		unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		outp32(dss_base +  DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK, unmask);

		outp32(dss_base +  DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INT_MSK, unmask);
	} else if (dpufd->index == AUXILIARY_PANEL_IDX) {
		unmask = ~0;
		unmask &= ~(BIT_OFF_WCH0_INTS | BIT_OFF_WCH1_INTS |
			BIT_OFF_WCH0_WCH1_FRM_END_INT | BIT_OFF_MMU_IRPT_NS);

		outp32(dss_base + GLB_CPU_OFF_INT_MSK, unmask);
		unmask = ~0;
		unmask &= ~(BIT_OFF_CAM_WCH2_FRMEND_INTS);
		outp32(dss_base + GLB_CPU_OFF_CAM_INT_MSK, unmask);
	}
}

void dpe_interrupt_mask(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dss_base = 0;
	char __iomem *mipi_dsi_base = NULL;
	uint32_t mask = 0;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	dss_base = dpufd->dss_base;

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_PDP_INT_MSK, mask);

		mipi_dsi_base = get_mipi_dsi_base(dpufd);
		outp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);
		if (is_dual_mipi_panel(dpufd))
			outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);

		outp32(dss_base + DSS_DISP_CH0_OFFSET + DPP_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_DSS_GLB_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_MCTL_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH0_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH1_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH2_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH3_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INT_MSK, mask);
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_SDP_INT_MSK, mask);
		if (is_dp_panel(dpufd)) {
			outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK, mask);
			outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INT_MSK, mask);
		} else {
			outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);
		}
	} else if (dpufd->index == AUXILIARY_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_OFF_INT_MSK, mask);
		outp32(dss_base + GLB_CPU_OFF_CAM_INT_MSK, mask);
	}

}

static void mipi_ldi_data_gate_ctrl(char __iomem *ldi_base, bool enable)
{
	if (g_ldi_data_gate_en == 1)
		set_reg(ldi_base + MIPI_LDI_FRM_VALID_DBG, (enable ? 0x0 : 0x1), 1, 29);
	else
		set_reg(ldi_base + MIPI_LDI_FRM_VALID_DBG, 0x1, 1, 29);
}

static void ldi_set_gate(char __iomem *ldi_base, uint32_t gate_en)
{
	if (g_ldi_data_gate_en == 1)
		set_reg(ldi_base + LDI_CTRL, gate_en, 1, 2);
	else
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 2);
}

void ldi_data_gate(struct dpu_fb_data_type *dpufd, bool enble)
{
	char __iomem *ldi_base = NULL;
	char __iomem *mipi_dsi_base = NULL;
	uint32_t gate_en = (enble ? 1 : 0);

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL!\n");

	if (!is_mipi_cmd_panel(dpufd)) {
		dpufd->ldi_data_gate_en = gate_en;
		return;
	}

	if (is_dp_panel(dpufd)) {
		ldi_base = dpufd->dss_base + DSS_LDI_DP_OFFSET;
		ldi_set_gate(ldi_base, gate_en);

		ldi_base = dpufd->dss_base + DSS_LDI_DP1_OFFSET;
		ldi_set_gate(ldi_base, gate_en);
	} else {
		if (dpufd->index == PRIMARY_PANEL_IDX) {
			mipi_dsi_base = get_mipi_dsi_base(dpufd);
			mipi_ldi_data_gate_ctrl(mipi_dsi_base, enble);
			if (is_dual_mipi_panel(dpufd))
				set_reg(dpufd->mipi_dsi1_base + MIPI_LDI_FRM_VALID_DBG, 0, 1, 29);
		} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
			mipi_ldi_data_gate_ctrl(dpufd->mipi_dsi1_base, enble);
		} else {
			DPU_FB_ERR("fb%d, not support!", dpufd->index);
			return;
		}
	}

	if (g_ldi_data_gate_en == 1)
		dpufd->ldi_data_gate_en = gate_en;
	else
		dpufd->ldi_data_gate_en = 0;
}

static void init_csc10b(struct dpu_fb_data_type *dpufd,
	char __iomem *dpp_csc10b_base)
{
	int (*csc_coe)[CSC_COL] = {NULL};

	if (dpp_csc10b_base == (dpufd->dss_base + DSS_DPP_CSC_RGB2YUV10B_OFFSET)) {
		csc_coe = g_csc10b_rgb2yuv709_wide;
		outp32(dpp_csc10b_base + CSC10B_MPREC, 0x2);
	} else if (dpp_csc10b_base == (dpufd->dss_base + DSS_DPP_CSC_YUV2RGB10B_OFFSET)) {
		csc_coe = g_csc10b_yuv2rgb709_wide;
		outp32(dpp_csc10b_base + CSC10B_MPREC, 0x0);
	} else {
		return;
	}

	outp32(dpp_csc10b_base + CSC10B_IDC0, csc_coe[2][3]);
	outp32(dpp_csc10b_base + CSC10B_IDC1, csc_coe[1][3]);
	outp32(dpp_csc10b_base + CSC10B_IDC2, csc_coe[0][3]);
	outp32(dpp_csc10b_base + CSC10B_ODC0, csc_coe[2][4]);
	outp32(dpp_csc10b_base + CSC10B_ODC1, csc_coe[1][4]);
	outp32(dpp_csc10b_base + CSC10B_ODC2, csc_coe[0][4]);
	outp32(dpp_csc10b_base + CSC10B_P00, csc_coe[0][0]);
	outp32(dpp_csc10b_base + CSC10B_P01, csc_coe[0][1]);
	outp32(dpp_csc10b_base + CSC10B_P02, csc_coe[0][2]);
	outp32(dpp_csc10b_base + CSC10B_P10, csc_coe[1][0]);
	outp32(dpp_csc10b_base + CSC10B_P11, csc_coe[1][1]);
	outp32(dpp_csc10b_base + CSC10B_P12, csc_coe[1][2]);
	outp32(dpp_csc10b_base + CSC10B_P20, csc_coe[2][0]);
	outp32(dpp_csc10b_base + CSC10B_P21, csc_coe[2][1]);
	outp32(dpp_csc10b_base + CSC10B_P22, csc_coe[2][2]);

	outp32(dpp_csc10b_base + CSC10B_MODULE_EN, 0x1);
}

void init_dpp_csc(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return;
	}

	pinfo = &(dpufd->panel_info);

	if (pinfo->acm_support || pinfo->arsr1p_sharpness_support || pinfo->post_scf_support) {
		/* init csc10b rgb2yuv */
		init_csc10b(dpufd, dpufd->dss_base + DSS_DPP_CSC_RGB2YUV10B_OFFSET);
		/* init csc10b yuv2rgb */
		init_csc10b(dpufd, dpufd->dss_base + DSS_DPP_CSC_YUV2RGB10B_OFFSET);
	}
}

void init_acm(struct dpu_fb_data_type *dpufd)
{
	(void *)dpufd;
}

static void init_dither_hifreq(char __iomem *dither_base)
{
	set_reg(dither_base + DITHER_HIFREQ_REG_INI_CFG_EN, 0x00000001, 1, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_0, 0x6495FC13, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_1, 0x27E5DB75, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_2, 0x69036280, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_3, 0x7478D47C, 31, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_0, 0x36F5325D, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_1, 0x90757906, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_2, 0xBBA85F01, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_3, 0x74B34461, 31, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_0, 0x76435C64, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_1, 0x4989003F, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_2, 0xA2EA34C6, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_3, 0x4CAD42CB, 31, 0);
	set_reg(dither_base + DITHER_HIFREQ_POWER_CTRL, 0x00000009, 4, 0);
	set_reg(dither_base + DITHER_HIFREQ_FILT_0, 0x00000421, 15, 0);
	set_reg(dither_base + DITHER_HIFREQ_FILT_1, 0x00000701, 15, 0);
	set_reg(dither_base + DITHER_HIFREQ_FILT_2, 0x00000421, 15, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_R0, 0x6D92B7DB, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_R1, 0x00002448, 16, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_G0, 0x6D92B7DB, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_G1, 0x00002448, 16, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_B0, 0x6D92B7DB, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_B1, 0x00002448, 16, 0);
}

void init_dither(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	char __iomem *dither_base = NULL;

	if (dpufd == NULL)	{
		DPU_FB_ERR("dpufd is NULL!\n");
		return;
	}

	pinfo = &(dpufd->panel_info);

	if (pinfo->dither_support != 1)
		return;

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		dither_base = dpufd->dss_base + DSS_DPP_DITHER_OFFSET;
	} else {
		DPU_FB_ERR("fb%d, not support!", dpufd->index);
		return;
	}

	set_reg(dither_base + DITHER_CTL1, 0x00000005, 6, 0);
	set_reg(dither_base + DITHER_CTL0, 0x0000000B, 5, 0);
	set_reg(dither_base + DITHER_TRI_THD12_0, 0x00080080, 24, 0);
	set_reg(dither_base + DITHER_TRI_THD12_1, 0x00000080, 12, 0);
	set_reg(dither_base + DITHER_TRI_THD10, 0x02008020, 30, 0);
	set_reg(dither_base + DITHER_TRI_THD12_UNI_0, 0x00100100, 24, 0);
	set_reg(dither_base + DITHER_TRI_THD12_UNI_1, 0x00000000, 12, 0);
	set_reg(dither_base + DITHER_TRI_THD10_UNI, 0x00010040, 30, 0);
	set_reg(dither_base + DITHER_BAYER_CTL, 0x00000000, 28, 0);
	set_reg(dither_base + DITHER_BAYER_ALPHA_THD, 0x00000000, 30, 0);
	set_reg(dither_base + DITHER_MATRIX_PART1, 0x5D7F91B3, 32, 0);
	set_reg(dither_base + DITHER_MATRIX_PART0, 0x6E4CA280, 32, 0);

	init_dither_hifreq(dither_base);

	set_reg(dither_base + DITHER_ERRDIFF_CTL, 0x00000000, 3, 0);
	set_reg(dither_base + DITHER_ERRDIFF_WEIGHT, 0x01232134, 28, 0);

	set_reg(dither_base + DITHER_FRC_CTL, 0x00000001, 4, 0);
	set_reg(dither_base + DITHER_FRC_01_PART1, 0xFFFF0000, 32, 0);
	set_reg(dither_base + DITHER_FRC_01_PART0, 0x00000000, 32, 0);
	set_reg(dither_base + DITHER_FRC_10_PART1, 0xFFFFFFFF, 32, 0);
	set_reg(dither_base + DITHER_FRC_10_PART0, 0x00000000, 32, 0);
	set_reg(dither_base + DITHER_FRC_11_PART1, 0xFFFFFFFF, 32, 0);
	set_reg(dither_base + DITHER_FRC_11_PART0, 0xFFFF0000, 32, 0);
}

