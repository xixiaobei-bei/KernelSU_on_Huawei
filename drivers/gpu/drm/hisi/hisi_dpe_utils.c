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

#include "hisi_drm_drv.h"

#include "hisi_overlay_utils.h"
#include "hisi_drm_dpe_utils.h"
#include "hisi_dpe.h"
#include "hisi_panel.h"
#include "hisi_mipi_dsi.h"

int get_dss_clock_value(u32 voltage_level,
			struct dss_vote_cmd *dss_vote_cmd)
{
	if (!dss_vote_cmd) {
		HISI_DRM_ERR("dss_vote_cmd is NULL point!\n");
		return -EINVAL;
	}

	switch (voltage_level) {
	case PERI_VOLTAGE_LEVEL3:
		dss_vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L4;
		dss_vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L4;
		break;
	case PERI_VOLTAGE_LEVEL2:
		dss_vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L3;
		dss_vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L3;
		break;
	case PERI_VOLTAGE_LEVEL1:
		dss_vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L2;
		dss_vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L2;
		break;
	case PERI_VOLTAGE_LEVEL0:
		dss_vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L1;
		dss_vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L1;
		break;
	default:
		HISI_DRM_ERR("wrong voltage value %d\n", voltage_level);
		return -EINVAL;
	}

	return 0;
}

void dss_inner_clk_common_enable(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	if (!ctx->dss_inner_clk_refcount && !ctx->is_pre_enabled) {
#ifdef CONFIG_DSS_LP_USED
		lp_first_level_clk_gate_ctrl(hisi_crtc);
		lp_second_level_clk_gate_ctrl(hisi_crtc);
#else
		no_memory_lp_ctrl(hisi_crtc);
#endif
	}
	ctx->dss_inner_clk_refcount++;
	HISI_DRM_DEBUG("inner clk ref=%d", ctx->dss_inner_clk_refcount);
}

void dss_inner_clk_common_disable(struct dss_hw_ctx *ctx)
{
	ctx->dss_inner_clk_refcount--;
}

void init_dpp(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	struct drm_display_mode *mode = NULL;
	char __iomem *dpp_base = NULL;
	u32 image_size;

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	mode = &hisi_crtc->base.state->adjusted_mode;

	if (hisi_crtc->crtc_id == PRIMARY_IDX) {
		dpp_base = ctx->dss_base + DSS_DISP_CH0_OFFSET;
	} else if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		dpp_base = ctx->dss_base + DSS_DISP_CH2_OFFSET;
	} else {
		HISI_DRM_ERR("CRTC-%d, not support!\n", hisi_crtc->crtc_id);
		return;
	}

	image_size = (DSS_HEIGHT(mode->vdisplay) << 16) |
		DSS_WIDTH(mode->hdisplay);
	outp32(dpp_base + IMG_SIZE_BEF_SR, image_size);
	outp32(dpp_base + IMG_SIZE_AFT_SR, image_size);
	outp32(dpp_base + IMG_SIZE_AFT_IFBCSW, image_size);
#ifdef CONFIG_HISI_FB_DPP_COLORBAR_USED
	outp32(dpp_base + DPP_CLRBAR_CTRL,
	       (DSS_HEIGHT(mode->hdisplay / 4 / 3) << 24) | (0 << 1) | 0x1);
	set_reg(dpp_base + DPP_CLRBAR_1ST_CLR, 0x3FF00000, 30, 0); /* Red */
	set_reg(dpp_base + DPP_CLRBAR_2ND_CLR, 0x000FFC00, 30, 0); /* Green */
	set_reg(dpp_base + DPP_CLRBAR_3RD_CLR, 0x000003FF, 30, 0); /* Blue */
#endif
}

void init_ifbc(struct dss_hw_ctx *ctx)
{
	int ifbc_type = IFBC_TYPE_VESA3X_DUAL;

	if (ifbc_type >= IFBC_TYPE_MAX) {
		HISI_DRM_ERR("ifbc_type is larger than IFBC_TYPE_MAX.\n");
		return;
	}

	if (ifbc_type == IFBC_TYPE_NONE)
		return;
}

struct frame_info {
	u32 hfp;
	u32 hbp;
	u32 hsw;
	u32 vfp;
	u32 vbp;
	u32 vsw;
};

struct thd_info {
	u32 thd_rqos_in;
	u32 thd_rqos_out;
	u32 thd_wqos_in;
	u32 thd_wqos_out;
	u32 thd_cg_in;
	u32 thd_cg_out;
	u32 thd_wr_wait;
	u32 thd_cg_hold;
	u32 thd_flux_req_befdfs_in;
	u32 thd_flux_req_befdfs_out;
	u32 thd_flux_req_aftdfs_in;
	u32 thd_flux_req_aftdfs_out;
	u32 thd_dfs_ok;
	u32 dfs_ok_mask;
	u32 thd_flux_req_sw_en;
	u32 thd_rqos_idle;
};

static void thd_info_init(struct thd_info *dbuf_thd_info)
{
	dbuf_thd_info->thd_rqos_in = 0;
	dbuf_thd_info->thd_rqos_out = 0;
	dbuf_thd_info->thd_wqos_in = 0;
	dbuf_thd_info->thd_wqos_out = 0;
	dbuf_thd_info->thd_cg_in = 0;
	dbuf_thd_info->thd_cg_out = 0;
	dbuf_thd_info->thd_wr_wait = 0;
	dbuf_thd_info->thd_cg_hold = 0;
	dbuf_thd_info->thd_flux_req_befdfs_in = 0;
	dbuf_thd_info->thd_flux_req_befdfs_out = 0;
	dbuf_thd_info->thd_flux_req_aftdfs_in = 0;
	dbuf_thd_info->thd_flux_req_aftdfs_out = 0;
	dbuf_thd_info->thd_dfs_ok = 0;
	dbuf_thd_info->dfs_ok_mask = 0;
	dbuf_thd_info->thd_flux_req_sw_en = 1;
}

static void frame_info_init(struct frame_info *dbuf_frame_info, struct drm_display_mode *mode)
{
	dbuf_frame_info->hfp = mode->hsync_start - mode->hdisplay;
	dbuf_frame_info->hbp = mode->htotal - mode->hsync_end;
	dbuf_frame_info->hsw = mode->hsync_end - mode->hsync_start;
	dbuf_frame_info->vfp = mode->vsync_start - mode->vdisplay;
	dbuf_frame_info->vbp = mode->vtotal - mode->vsync_end;
	dbuf_frame_info->vsw = mode->vsync_end - mode->vsync_start;
}

void calc_thd_info(struct thd_info *dbuf_thd_info, int depth)
{
	dbuf_thd_info->thd_cg_in = depth - 1;
	dbuf_thd_info->thd_cg_out = dbuf_thd_info->thd_cg_in * 95 / 100;
	dbuf_thd_info->thd_rqos_in = depth * 80 / 100;
	dbuf_thd_info->thd_rqos_out = depth * 90 / 100;
	dbuf_thd_info->thd_rqos_idle = depth;
	dbuf_thd_info->thd_flux_req_befdfs_in = depth * 70 / 100;
	dbuf_thd_info->thd_flux_req_befdfs_out = depth * 80 / 100;
	dbuf_thd_info->thd_flux_req_aftdfs_in = dbuf_thd_info->thd_flux_req_befdfs_in;
	dbuf_thd_info->thd_flux_req_aftdfs_out = dbuf_thd_info->thd_flux_req_befdfs_out;
	dbuf_thd_info->thd_dfs_ok = dbuf_thd_info->thd_flux_req_befdfs_in;
	dbuf_thd_info->dfs_ok_mask = 0;
}

static void set_reg_for_dbuf(struct drm_display_mode *mode, char __iomem *dbuf_base,
	int sram_valid_num, struct thd_info *dbuf_thd_info, int dfs_ram)
{
	outp32(dbuf_base + DBUF_FRM_SIZE, mode->hdisplay * mode->vdisplay);
	outp32(dbuf_base + DBUF_FRM_HSIZE, DSS_WIDTH(mode->hdisplay));
	outp32(dbuf_base + DBUF_SRAM_VALID_NUM, sram_valid_num);

	outp32(dbuf_base + DBUF_THD_RQOS, (dbuf_thd_info->thd_rqos_out << 16) | dbuf_thd_info->thd_rqos_in);
	outp32(dbuf_base + DBUF_THD_WQOS, (dbuf_thd_info->thd_wqos_out << 16) | dbuf_thd_info->thd_wqos_in);
	outp32(dbuf_base + DBUF_THD_CG, (dbuf_thd_info->thd_cg_out << 16) | dbuf_thd_info->thd_cg_in);
	outp32(dbuf_base + DBUF_THD_OTHER, (dbuf_thd_info->thd_cg_hold << 16) | dbuf_thd_info->thd_wr_wait);
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_BEF,
	       (dbuf_thd_info->thd_flux_req_befdfs_out << 16) | dbuf_thd_info->thd_flux_req_befdfs_in);
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_AFT,
	       (dbuf_thd_info->thd_flux_req_aftdfs_out << 16) | dbuf_thd_info->thd_flux_req_aftdfs_in);
	outp32(dbuf_base + DBUF_THD_DFS_OK, dbuf_thd_info->thd_dfs_ok);
	outp32(dbuf_base + DBUF_FLUX_REQ_CTRL,
	       (dbuf_thd_info->dfs_ok_mask << 1) | dbuf_thd_info->thd_flux_req_sw_en);

	outp32(dbuf_base + DBUF_DFS_LP_CTRL, 0x1);
	outp32(dbuf_base + DBUF_DFS_RAM_MANAGE, dfs_ram);

	outp32(dbuf_base + DBUF_THD_RQOS_IDLE, dbuf_thd_info->thd_rqos_idle);

}

struct dbuf_param {
	int dfs_time;
	int depth;
	int dfs_ram;
	int sram_valid_num;
};

void init_dbuf(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	struct drm_display_mode *mode = NULL;
	struct drm_display_mode *adj_mode = NULL;
	char __iomem *dbuf_base = NULL;
	struct frame_info dbuf_frame_info;
	struct thd_info dbuf_thd_info;
	struct dbuf_param dbuf_param;

	dbuf_param.sram_valid_num = 0;

	thd_info_init(&dbuf_thd_info);

	dbuf_param.dfs_time = 0;
	dbuf_param.dfs_ram = 0;

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	mode = &hisi_crtc->base.state->adjusted_mode;
	adj_mode = &hisi_crtc->base.state->adjusted_mode;

	frame_info_init(&dbuf_frame_info, mode);

	if (hisi_crtc->crtc_id == PRIMARY_IDX) {
		dbuf_base = ctx->dss_base + DSS_DBUF0_OFFSET;
		dbuf_param.sram_valid_num = 0;
		dbuf_param.dfs_ram = 0xFFF0;
	} else if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		dbuf_base = ctx->dss_base + DSS_DBUF1_OFFSET;
		dbuf_param.sram_valid_num = 2;
		dbuf_param.dfs_ram = 0x111F;
	} else {
		HISI_DRM_INFO("crtc-%d, not support!", hisi_crtc->crtc_id);
		return;
	}

	dbuf_param.depth = (dbuf_param.sram_valid_num + 1) * DBUF_DEPTH;

	HISI_DRM_DEBUG("dfs_time=%d,\n"
		"adj_mode->clock=%d\n"
		"hsw=%d\n"
		"hbp=%d\n"
		"hfp=%d\n"
		"mode->hdisplay=%d\n"
		"mode->vdisplay=%d\n",
		dbuf_param.dfs_time,
		adj_mode->clock,
		dbuf_frame_info.hsw,
		dbuf_frame_info.hbp,
		dbuf_frame_info.hfp,
		mode->hdisplay,
		mode->vdisplay);

	calc_thd_info(&dbuf_thd_info, dbuf_param.depth);

	HISI_DRM_DEBUG("hdisplay=%d\n"
		"vdisplay=%d\n"
		"sram_valid_num=%d,\n"
		"thd_rqos_in=0x%x\n"
		"thd_rqos_out=0x%x\n"
		"thd_cg_in=0x%x\n"
		"thd_cg_out=0x%x\n"
		"thd_flux_req_befdfs_in=0x%x\n"
		"thd_flux_req_befdfs_out=0x%x\n"
		"thd_flux_req_aftdfs_in=0x%x\n"
		"thd_flux_req_aftdfs_out=0x%x\n"
		"thd_dfs_ok=0x%x\n",
		mode->hdisplay,
		mode->vdisplay,
		dbuf_param.sram_valid_num,
		dbuf_thd_info.thd_rqos_in,
		dbuf_thd_info.thd_rqos_out,
		dbuf_thd_info.thd_cg_in,
		dbuf_thd_info.thd_cg_out,
		dbuf_thd_info.thd_flux_req_befdfs_in,
		dbuf_thd_info.thd_flux_req_befdfs_out,
		dbuf_thd_info.thd_flux_req_aftdfs_in,
		dbuf_thd_info.thd_flux_req_aftdfs_out,
		dbuf_thd_info.thd_dfs_ok);

	set_reg_for_dbuf(mode, dbuf_base, dbuf_param.sram_valid_num, &dbuf_thd_info, dbuf_param.dfs_ram);

	if (hisi_crtc->crtc_id == PRIMARY_IDX)
		set_reg(ctx->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK,
			0x0, 1, 0);
	else if (hisi_crtc->crtc_id == EXTERNAL_IDX)
		set_reg(ctx->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK,
			0x0, 1, 1);
}

void deinit_dbuf(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (hisi_crtc->crtc_id == PRIMARY_IDX)
		set_reg(ctx->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK,
			0x1, 1, 0);
	else if (hisi_crtc->crtc_id == EXTERNAL_IDX)
		set_reg(ctx->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK,
			0x1, 1, 1);
}

static void set_pipe_sw_reg(char __iomem *pipe_sw_offset, int crtc_id)
{
	if (crtc_id == PRIMARY_IDX) {
		set_reg(pipe_sw_offset + PIPE_SW_SIG_CTRL, 0x1, 8, 0);
		set_reg(pipe_sw_offset + SW_POS_CTRL_SIG_EN, 0x1, 1, 0);
		set_reg(pipe_sw_offset + PIPE_SW_DAT_CTRL, 0x1, 8, 0);
		set_reg(pipe_sw_offset + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);
	} else if (crtc_id == EXTERNAL_IDX) {
		set_reg(pipe_sw_offset + PIPE_SW_SIG_CTRL, 0x2, 8, 0);
		set_reg(pipe_sw_offset + SW_POS_CTRL_SIG_EN, 0x1, 1, 0);
		set_reg(pipe_sw_offset + PIPE_SW_DAT_CTRL, 0x2, 8, 0);
		set_reg(pipe_sw_offset + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);
	}
}

static void init_pipe_sw(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *pipe_sw_offset = NULL;
	char __iomem *dss_base = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	dss_base = ctx->dss_base;
	if (hisi_crtc->crtc_id == PRIMARY_IDX) {
		pipe_sw_offset = dss_base + DSS_PIPE_SW_DSI0_OFFSET;
		set_pipe_sw_reg(pipe_sw_offset, hisi_crtc->crtc_id);
		if (is_dual_mipi_mode(hisi_crtc)) {
			pipe_sw_offset = dss_base + DSS_PIPE_SW_DSI1_OFFSET;
			set_pipe_sw_reg(pipe_sw_offset, hisi_crtc->crtc_id);
		}
	} else if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		if (is_dp(hisi_crtc))
			pipe_sw_offset = dss_base + DSS_PIPE_SW_DP_OFFSET;
		else
			pipe_sw_offset = dss_base + DSS_PIPE_SW_DSI1_OFFSET;
		set_pipe_sw_reg(pipe_sw_offset, hisi_crtc->crtc_id);
	}
}

static void init_dpp_ifbc_sw(struct dss_hw_ctx *ctx)
{
	char __iomem *disp_glb_offset = NULL;

	disp_glb_offset = ctx->dss_base + DSS_DISP_GLB_OFFSET;
	set_reg(disp_glb_offset + DPPSW_SIG_CTRL, 0x20101, 24, 0);
	set_reg(disp_glb_offset + DPPSW_DAT_CTRL, 0x20101, 24, 0);
	set_reg(disp_glb_offset + IFBCSW_SIG_CTRL, 0x403, 16, 0);
	set_reg(disp_glb_offset + IFBCSW_DAT_CTRL, 0x403, 16, 0);
	set_reg(disp_glb_offset + DYN_SW_DEFAULT, 0x0, 1, 0);
}

void hisi_display_post_process_chn_init(
	struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = hisi_crtc->ctx;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	init_pipe_sw(hisi_crtc);
	init_dpp_ifbc_sw(ctx);
}

void init_ldi(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	hisi_display_post_process_chn_init(hisi_crtc);
#ifdef CONFIG_HISI_DSS_V510
	crtc_disable_ldi(hisi_crtc);
#endif
}

void init_acm(struct dss_hw_ctx *ctx)
{
	char __iomem *acm_base = NULL;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}
	acm_base = ctx->dss_base + DSS_DPP_ACM_OFFSET;

	outp32(acm_base + ACM_MEM_CTRL_ES, 0x4);
}

void init_igm_gmp_xcc_gm(struct dss_hw_ctx *ctx)
{
	char __iomem *xcc_base = NULL;
	char __iomem *gmp_base = NULL;
	char __iomem *degamma_base = NULL;
	char __iomem *gamma_base = NULL;
	char __iomem *disp_glb_base = NULL;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	degamma_base = ctx->dss_base + DSS_DPP_DEGAMMA_OFFSET;
	gmp_base = ctx->dss_base + DSS_DPP_GMP_OFFSET;
	gamma_base = ctx->dss_base + DSS_DPP_GAMA_OFFSET;
	disp_glb_base = ctx->dss_base + DSS_DISP_GLB_OFFSET;
	xcc_base = ctx->dss_base + DSS_DPP_XCC_OFFSET;

	outp32(degamma_base + DEGAMA_MEM_CTRL, 0x2);
	outp32(degamma_base + 0x40000 + DEGAMA_MEM_CTRL, 0x2);
	outp32(gmp_base + GMP_MEM_CTRL, 0x2);
	outp32(gmp_base + 0x40000 + GMP_MEM_CTRL, 0x2);
	outp32(gamma_base + GAMA_MEM_CTRL, 0x2);
	outp32(gamma_base + + 0x40000 + GAMA_MEM_CTRL, 0x2);

	set_reg(gmp_base + GMP_EN, 0, 1, 0);
	set_reg(degamma_base + DEGAMA_EN, 0, 1, 0);
	set_reg(gamma_base + GAMA_EN, 0, 1, 0);
	set_reg(xcc_base + XCC_EN, 0, 1, 0);
	set_reg(gmp_base + 0x40000 + GMP_EN, 0, 1, 0);
	set_reg(degamma_base + 0x40000 + DEGAMA_EN, 0, 1, 0);
	set_reg(gamma_base + 0x40000 + GAMA_EN, 0, 1, 0);
	set_reg(xcc_base + 0x40000 + XCC_EN, 0, 1, 0);
}

void init_dither(struct dss_hw_ctx *ctx)
{
	(void)ctx;
}

