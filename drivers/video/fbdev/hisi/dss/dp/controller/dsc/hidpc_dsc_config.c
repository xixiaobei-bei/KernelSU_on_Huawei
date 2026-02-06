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
 * Copyright (c) 2020 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#include "dsc_config_base.h"
#include "../../../hisi_fb.h"
#include "../../../hisi_fb_def.h"
#include "../dp_core_interface.h"
#include "../hidpc/hidpc_dp_avgen.h"
#include "../hidpc/hidpc_dp_core.h"
#include "../hidpc/hidpc_reg.h"

#define DSC_12BPC 12
#define DSS_DSC_MAX_LINE_BUF_DEP 15

int dptx_slice_height_limit(struct dp_ctrl *dptx, uint32_t pic_height)
{
	int divide_val;
	uint32_t reg;
	uint16_t dsc_max_num_lines;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");
	divide_val = 1;
	reg = dptx_readl(dptx, DPTX_CONFIG_REG2);
	dsc_max_num_lines = (reg & DSC_MAX_NUM_LINES_MASK) >> DSC_MAX_NUM_LINES_SHIFT;

	DPU_FB_DEBUG("[DP] %s dsc_max_num_lines = %d\n", __func__, dsc_max_num_lines);

	if (pic_height < dsc_max_num_lines) {
		divide_val = 1;
	} else {
		if (divide_val == 0) {
			DPU_FB_ERR("[DP] divide_val cannot be zero");
			return -EINVAL;
		}
		while (!((pic_height / divide_val) < dsc_max_num_lines))
			divide_val++; /* divide to 2 */
	}
	return divide_val;
}

int dptx_line_buffer_depth_limit(uint8_t line_buf_depth)
{
	if (line_buf_depth > DSS_DSC_MAX_LINE_BUF_DEP)
		line_buf_depth = DSS_DSC_MAX_LINE_BUF_DEP;

	return line_buf_depth;
}

void dptx_dsc_enable(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(0));
	reg |= DPTX_VSAMPLE_CTRL_ENABLE_DSC;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(0), reg);
}

static void dptx_mark_encoders(struct dp_ctrl *dptx)
{
	uint32_t dsc_ctl;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx->vparams.dp_dsc_info.encoders = 2;
	dsc_ctl = dptx_readl(dptx, DPTX_DSC_CTL);
	dsc_ctl |= (dptx->vparams.dp_dsc_info.encoders / 2) << 22;
	dptx_writel(dptx, DPTX_DSC_CTL, dsc_ctl);
}

static void dptx_divide_pixel_clock(struct dp_ctrl *dptx)
{
	uint32_t reg;
	uint8_t pixels_per_pixelclk = 0;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	/* Determine sampled pixel count based on pixel clock */
	switch (dptx->multipixel) {
	case DPTX_MP_SINGLE_PIXEL:
		pixels_per_pixelclk = 1;
		break;
	case DPTX_MP_DUAL_PIXEL:
		pixels_per_pixelclk = 2;
		break;
	case DPTX_MP_QUAD_PIXEL:
		pixels_per_pixelclk = 4;
		break;
	default:
		break;
	}

	/* Program DSC_CTL.STREAMn_ENC_CLK_DIVIDED bit */
	if (dptx->vparams.dp_dsc_info.encoders > pixels_per_pixelclk) {
		/* Divide pixel clock for DSC encoder */
		reg = dptx_readl(dptx, DPTX_DSC_CTL);
		reg |= 1 << DPTX_DSC_STREAM0_ENC_CLK_DIV_SHIFT;
		dptx_writel(dptx, DPTX_DSC_CTL, reg);
	}
}

static void dptx_pixel_mode_based_on_encoder_count(struct dp_ctrl *dptx)
{
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	/* Change pixel mode based on encoders count */
	switch (dptx->vparams.dp_dsc_info.encoders) {
	case 8:
		dptx->multipixel = DPTX_MP_QUAD_PIXEL;
		break;
	case 4:
		dptx->multipixel = DPTX_MP_DUAL_PIXEL;
		break;
	default:
		break;
	}

	/* Single, dual, or quad pixel */
	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(0));
	reg &= ~DPTX_VSAMPLE_CTRL_MULTI_PIXEL_MASK;
	reg |= dptx->multipixel << DPTX_VSAMPLE_CTRL_MULTI_PIXEL_SHIFT;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(0), reg);

	/* Divide pixel clock, if needed */
	dptx_divide_pixel_clock(dptx);
}

static void dptx_calc_wait_cnt(struct dp_ctrl *dptx)
{
	uint8_t encoders;
	uint16_t bpp;
	uint64_t fixp;
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	bpp = dptx->vparams.dp_dsc_info.dsc_info.dsc_bpp;
	encoders = dptx->vparams.dp_dsc_info.encoders;

	/* Get the integer part */
	fixp = drm_fixp_from_fraction(128, (bpp * encoders));
	dptx->vparams.dp_dsc_info.wait_cnt_int = drm_fixp2int(fixp);

	/* Get the fractional part */
	fixp &= DRM_FIXED_DECIMAL_MASK;
	fixp *= 64;
	dptx->vparams.dp_dsc_info.wait_cnt_frac = drm_fixp2int(fixp);

	DPU_FB_INFO("[DP] wait_cnt_int = %u, wait_cnt_frac = %u\n",
		dptx->vparams.dp_dsc_info.wait_cnt_int, dptx->vparams.dp_dsc_info.wait_cnt_frac);

	reg = dptx_readl(dptx, DPTX_VIDEO_DSCCFG);
	reg &= ~DPTX_DSC_LSTEER_INT_SHIFT_MASK;
	reg &= ~DPTX_DSC_LSTEER_FRAC_SHIFT_MASK;
	reg |= dptx->vparams.dp_dsc_info.wait_cnt_int << DPTX_DSC_LSTEER_INT_SHIFT;
	reg |= dptx->vparams.dp_dsc_info.wait_cnt_frac << DPTX_DSC_LSTEER_FRAC_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_DSCCFG, reg);
}

static void dptx_calc_dsc_lsteer_xmit_delay(struct dp_ctrl *dptx)
{
	int encoder_delay;
	uint32_t mux_word_size;
	uint32_t muxer_initial_delay;
	uint16_t h_active;
	uint16_t h_blanking;
	uint8_t horizontal_slices;
	uint8_t vertical_slices;
	uint8_t bpc;
	uint8_t multipixel;
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	bpc = dptx->vparams.dp_dsc_info.dsc_info.dsc_bpc;
	mux_word_size = (bpc < DSC_12BPC) ? 48 : 64;
	muxer_initial_delay = (mux_word_size + (4 * bpc + 4) - 3 + 32) * 3;
	horizontal_slices = dptx->vparams.dp_dsc_info.encoders;
	DPU_FB_DEBUG("[DP] horizontal_slices  = %d\n", horizontal_slices);
	multipixel = dptx->multipixel;
	DPU_FB_DEBUG("[DP] multipixel  = %d\n", multipixel);
	h_active = dptx->vparams.mdtd.h_active;
	DPU_FB_DEBUG("[DP] h_active  = %d\n", h_active);
	h_blanking = dptx->vparams.mdtd.h_blanking;
	DPU_FB_DEBUG("[DP] h_blanking  = %d\n", h_blanking);
	if (dptx->vparams.dp_dsc_info.dsc_info.slice_height == 0) {
		DPU_FB_ERR("[DP] slice_height cannot be zero");
		return;
	}
	vertical_slices = dptx->vparams.mdtd.v_active / dptx->vparams.dp_dsc_info.dsc_info.slice_height;
	DPU_FB_DEBUG("[DP] vertical_slices  = %d\n", vertical_slices);
	encoder_delay = (((PIXEL_HOLD_DELAY + PIXEL_FLATNESSBUF_DELAY +
		PIXEL_GROUP_DELAY + muxer_initial_delay +
		MUXER_INITIAL_BUFFERING_DELAY) * horizontal_slices)) /
		(1 << multipixel);
	/* calc lsteer_xmit_delay ybr */
	dptx->vparams.dp_dsc_info.lsteer_xmit_delay = encoder_delay + (((h_blanking + h_active) * vertical_slices) +
		h_blanking + h_active +
		(dptx->vparams.dp_dsc_info.dsc_info.initial_xmit_delay * horizontal_slices)) / (1 << multipixel);

	DPU_FB_DEBUG("[DP] DSC XMIT delay = %d\n", dptx->vparams.dp_dsc_info.lsteer_xmit_delay);
	DPU_FB_DEBUG("[DP] muxer inital delay  = %d\n", muxer_initial_delay);
	DPU_FB_DEBUG("[DP] DSC encoder delay = %d\n", encoder_delay);
	DPU_FB_DEBUG("[DP] DSC XMIT delay = %d\n", dptx->vparams.dp_dsc_info.lsteer_xmit_delay);

	reg = dptx_readl(dptx, DPTX_VIDEO_DSCCFG);
	reg &= ~DPTX_DSC_LSTEER_XMIT_DELAY_MASK;
	reg |= dptx->vparams.dp_dsc_info.lsteer_xmit_delay << DPTX_DSC_LSTEER_XMIT_DELAY_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_DSCCFG, reg);
}

static void dptx_check_encoders(struct dp_ctrl *dptx)
{
	uint8_t available_encoders;
	uint8_t dsc_hwcfg;
	uint8_t encoders;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");
	encoders = dptx->dsc_decoders;

	dsc_hwcfg = dptx_readl(dptx, DPTX_DSC_HWCFG);
	available_encoders = dsc_hwcfg & DPTX_DSC_NUM_ENC_MSK;

	DPU_FB_DEBUG("[DP] Calculated encoders count = %d\n", encoders);
	DPU_FB_DEBUG("[DP] Available encoders count = %d\n", available_encoders);

	if (encoders > available_encoders)
		DPU_FB_INFO("[DP] Encoder count is greather than available encoders\n");
}

void dptx_dsc_cfg(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_check_encoders(dptx);
	dptx_mark_encoders(dptx);
	dptx_dsc_notice_rx(dptx);
	dptx_soft_reset(dptx, DPTX_SRST_VIDEO_RESET_N(0));
	dptx_pixel_mode_based_on_encoder_count(dptx);

	/* program pps table */
	dptx_program_pps_sdps(dptx);

	/* calc dsc timing generate delay*/
	dptx_calc_wait_cnt(dptx);
	dptx_calc_dsc_lsteer_xmit_delay(dptx);
}
