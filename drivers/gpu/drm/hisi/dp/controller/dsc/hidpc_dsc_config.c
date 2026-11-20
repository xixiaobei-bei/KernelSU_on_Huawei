/*lint -e529*/
#include "dsc_config_base.h"
#include "hisi_dp_drv.h"
#include "hisi_defs.h"
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

	drm_check_and_return((dptx == NULL), false, ERR, "[DP] NULL Pointer\n");
	divide_val = 1;
	reg = dptx_readl(dptx, DPTX_CONFIG_REG2);
	dsc_max_num_lines = (reg & DSC_MAX_NUM_LINES_MASK) >> DSC_MAX_NUM_LINES_SHIFT;

	HISI_DRM_DEBUG("[DP] %s dsc_max_num_lines = %d\n", __func__, dsc_max_num_lines);

	if (pic_height < dsc_max_num_lines) {
		divide_val = 1;
	} else {
		while (!((pic_height / divide_val) < dsc_max_num_lines))
			divide_val++; // divide to 2
	}
	return divide_val;
}

int dptx_line_buffer_depth_limit(struct dp_ctrl *dptx, uint8_t line_buf_depth)
{
	drm_check_and_return((dptx == NULL), false, ERR, "[DP] NULL Pointer\n");

	if (line_buf_depth > DSS_DSC_MAX_LINE_BUF_DEP)
		line_buf_depth = DSS_DSC_MAX_LINE_BUF_DEP;

	return line_buf_depth;
}

void dptx_dsc_enable(struct dp_ctrl *dptx)
{
	uint32_t reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(0));
	reg |= DPTX_VSAMPLE_CTRL_ENABLE_DSC;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(0), reg);
}

static void dptx_mark_encoders(struct dp_ctrl *dptx)
{
	uint32_t dsc_ctl;

	dptx->vparams.dp_dsc_info.encoders = 2;
	dsc_ctl = dptx_readl(dptx, DPTX_DSC_CTL);
	dsc_ctl |= (dptx->vparams.dp_dsc_info.encoders / 2) << 22;
	dptx_writel(dptx, DPTX_DSC_CTL, dsc_ctl);
}

static void dptx_divide_pixel_clock(struct dp_ctrl *dptx)
{
	uint32_t reg;
	uint8_t pixels_per_pixelclk = 0;

	// Determine sampled pixel count based on pixel clock
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

	// Program DSC_CTL.STREAMn_ENC_CLK_DIVIDED bit
	if (dptx->vparams.dp_dsc_info.encoders > pixels_per_pixelclk) {
		// Divide pixel clock for DSC encoder
		reg = dptx_readl(dptx, DPTX_DSC_CTL);
		reg |= 1 << DPTX_DSC_STREAM0_ENC_CLK_DIV_SHIFT;
		dptx_writel(dptx, DPTX_DSC_CTL, reg);
	}
}

static void dptx_pixel_mode_based_on_encoder_count(struct dp_ctrl *dptx)
{
	uint32_t reg;
	uint8_t stream = 0;

	// Change pixel mode based on encoders count
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
	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	reg &= ~DPTX_VSAMPLE_CTRL_MULTI_PIXEL_MASK;
	reg |= dptx->multipixel << DPTX_VSAMPLE_CTRL_MULTI_PIXEL_SHIFT;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), reg);

	/* Divide pixel clock, if needed */
	dptx_divide_pixel_clock(dptx);
}

static void dptx_calc_wait_cnt(struct dp_ctrl *dptx)
{
	uint8_t encoders;
	uint16_t bpp;
	s64 fixp;
	uint32_t reg;

	bpp = dptx->vparams.dp_dsc_info.dsc_info.dsc_bpp;
	encoders = dptx->vparams.dp_dsc_info.encoders;

	// Get the integer part
	fixp = drm_fixp_from_fraction(128, (bpp * encoders));
	dptx->vparams.dp_dsc_info.wait_cnt_int = drm_fixp2int(fixp);

	// Get the fractional part
	fixp &= DRM_FIXED_DECIMAL_MASK;
	fixp *= 64;
	dptx->vparams.dp_dsc_info.wait_cnt_frac = drm_fixp2int(fixp);

	HISI_DRM_INFO("[DP] wait_cnt_int = %u, wait_cnt_frac = %u\n",
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

	uint8_t horizontal_slices;
	uint8_t vertical_slices;
	uint8_t bpc;
	uint32_t reg;

	bpc = dptx->vparams.dp_dsc_info.dsc_info.dsc_bpc;
	mux_word_size = (bpc < DSC_12BPC) ? 48 : 64;
	muxer_initial_delay = (mux_word_size + (4 * bpc + 4) - 3 + 32) * 3;
	horizontal_slices = dptx->vparams.dp_dsc_info.encoders;

	HISI_DRM_DEBUG("[DP] horizontal_slices  = %d\n", horizontal_slices);
	HISI_DRM_DEBUG("[DP] multipixel  = %d\n", dptx->multipixel);
	HISI_DRM_DEBUG("[DP] h_active  = %d\n", dptx->vparams.mdtd.h_active);
	HISI_DRM_DEBUG("[DP] h_blanking  = %d\n", dptx->vparams.mdtd.h_blanking);

	if(dptx->vparams.dp_dsc_info.dsc_info.slice_height == 0) {
		HISI_DRM_ERR("[DP] slice_height is 0\n");
		return;
	}

	if(dptx->multipixel == 0) {
		HISI_DRM_ERR("[DP] multipixel is 0\n");
		return;
	}

	vertical_slices = dptx->vparams.mdtd.v_active / dptx->vparams.dp_dsc_info.dsc_info.slice_height;
	HISI_DRM_DEBUG("[DP] vertical_slices  = %d\n", vertical_slices);
	encoder_delay = (((PIXEL_HOLD_DELAY + PIXEL_FLATNESSBUF_DELAY +
		PIXEL_GROUP_DELAY + muxer_initial_delay +
		MUXER_INITIAL_BUFFERING_DELAY) * horizontal_slices)) /
		(1 << dptx->multipixel);

	// calc lsteer_xmit_delay ybr
	dptx->vparams.dp_dsc_info.lsteer_xmit_delay = encoder_delay +
		(((dptx->vparams.mdtd.h_blanking + dptx->vparams.mdtd.h_active) * vertical_slices) +
		dptx->vparams.mdtd.h_blanking + dptx->vparams.mdtd.h_active +
		(dptx->vparams.dp_dsc_info.dsc_info.initial_xmit_delay * horizontal_slices)) / (1 << dptx->multipixel);

	HISI_DRM_DEBUG("[DP] DSC XMIT delay = %d\n", dptx->vparams.dp_dsc_info.lsteer_xmit_delay);
	HISI_DRM_DEBUG("[DP] muxer inital delay  = %d\n", muxer_initial_delay);
	HISI_DRM_DEBUG("[DP] DSC encoder delay = %d\n", encoder_delay);
	HISI_DRM_DEBUG("[DP] DSC XMIT delay = %d\n", dptx->vparams.dp_dsc_info.lsteer_xmit_delay);

	reg = dptx_readl(dptx, DPTX_VIDEO_DSCCFG);
	reg &= ~DPTX_DSC_LSTEER_XMIT_DELAY_MASK;
	reg |= dptx->vparams.dp_dsc_info.lsteer_xmit_delay << DPTX_DSC_LSTEER_XMIT_DELAY_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_DSCCFG, reg);

}

static void dptx_check_encoders(struct dp_ctrl *dptx)
{
	uint8_t available_encoders;
	uint8_t dsc_hwcfg;
	uint8_t encoders = dptx->dsc_decoders;
	uint32_t dsc_ctl;

	dsc_hwcfg = dptx_readl(dptx, DPTX_DSC_HWCFG);
	dsc_ctl = dptx_readl(dptx, DPTX_DSC_CTL);
	available_encoders = dsc_hwcfg & DPTX_DSC_NUM_ENC_MSK;

	HISI_DRM_DEBUG("[DP] Calculated encoders count = %d\n", encoders);
	HISI_DRM_DEBUG("[DP] Available encoders count = %d\n", available_encoders);

	if (encoders > available_encoders)
		HISI_DRM_INFO("[DP] Encoder count is greather than available encoders\n");
}

void dptx_dsc_cfg(struct dp_ctrl *dptx)
{
	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	dptx_check_encoders(dptx);
	dptx_mark_encoders(dptx);
	dptx_dsc_notice_rx(dptx);
	dptx_soft_reset(dptx, DPTX_SRST_VIDEO_RESET_N(0));
	dptx_pixel_mode_based_on_encoder_count(dptx);

	/* program pps table */
	dptx_program_pps_sdps(dptx);

	/* calc dsc timing generate delay */
	dptx_calc_wait_cnt(dptx);
	dptx_calc_dsc_lsteer_xmit_delay(dptx);

}

/*lint +e529*/
