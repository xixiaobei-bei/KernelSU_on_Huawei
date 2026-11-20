/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
/*lint -e559*/

#include "hisi_mipi_dsi.h"
#include "hisi_drm_dsi.h"
#include "hisi_drm_dpe_utils.h"

void mipi_cdphy_init_config(char __iomem *mipi_dsi_base, struct hisi_dsi *dsi)
{
	struct mipi_panel_info *mipi = NULL;
	struct mipi_dsi_phy_ctrl *dsi_phy_ctrl = NULL;
	if (!dsi) {
		HISI_DRM_DEBUG("mipi_cdphy_init_config: dsi is null!!!");
		return;
	}
	if (!dsi->panel_info) {
		HISI_DRM_DEBUG("mipi_cdphy_init_config: dsi->panel_info is null!!!");
		return;
	}
	mipi = &dsi->panel_info->mipi;
	dsi_phy_ctrl = &dsi->panel_info->dsi_phy_ctrl;
	if (mipi->phy_mode == CPHY_MODE) {
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010001, 0x3e); /* T3-PREBEGIN */
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010002, 0x1e); /* T3-POST */
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010042, 0x21);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010049,
			(dsi_phy_ctrl->rg_pll_posdiv << 4) | dsi_phy_ctrl->rg_pll_prediv);
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A,
			dsi_phy_ctrl->rg_pll_fbkdiv);
		/* PLL update control */
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);
		/* set cphy spec parameter */
		mipi_config_cphy_spec1v0_parameter(mipi_dsi_base, dsi);
	} else {
#ifdef CONFIG_HISI_DSS_V510
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010042, 0x21);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010049,
			(dsi_phy_ctrl->rg_pll_posdiv << 4) | dsi_phy_ctrl->rg_pll_prediv);
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A,
			dsi_phy_ctrl->rg_pll_fbkdiv);
#else
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010048,
			dsi_phy_ctrl->rg_pll_posdiv);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010049,
			(dsi_phy_ctrl->rg_pll_prediv << 4) |
			(dsi_phy_ctrl->rg_pll_fbkdiv >> 8));
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A,
			(dsi_phy_ctrl->rg_pll_fbkdiv & 0xFF));
#endif
		/* PLL update control */
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);
		/* set dphy spec parameter */
		mipi_config_dphy_spec1v2_parameter(mipi_dsi_base, dsi);
	}
}

void mipi_ldi_init(struct hisi_dsi *dsi, char __iomem *mipi_dsi_base)
{
	struct dss_rect rect = {0, 0, 0, 0};
	uint32_t hsize_real;
	uint32_t pixel_clk;
	uint64_t lane_byte_clk;
	struct hisi_panel_info *pinfo = NULL;
	if (!dsi) {
		HISI_DRM_DEBUG("mipi_ldi_init: dsi is null!!!");
		return;
	}
	pinfo = dsi->panel_info;
	if (!pinfo) {
		HISI_DRM_DEBUG("mipi_ldi_init: pinfo is null!!!");
		return;
	}
	if (pinfo->mipi.phy_mode == CPHY_MODE)
		lane_byte_clk = pinfo->dsi_phy_ctrl.lane_word_clk;
	else
		lane_byte_clk = pinfo->dsi_phy_ctrl.lane_byte_clk;

	HISI_DRM_DEBUG("lane_byte_clk=%d, dsi->phy.lane_byte_clk=%d\n",
		lane_byte_clk, dsi->phy.lane_byte_clk);

	rect.x = 0;
	rect.y = 0;
	rect.w = dsi->cur_mode.hdisplay;
	rect.h = dsi->cur_mode.vdisplay;
	mipi_ifbc_get_rect(dsi, &rect);

	if (pinfo->mipi.dsi_timing_support) {
		hsize_real = pinfo->mipi.dpi_hsize;
	} else {
		pixel_clk = mipi_pixel_clk(dsi);
		if (pixel_clk == 0)
			return;
		hsize_real = ROUND1(rect.w * lane_byte_clk, pixel_clk);
	}

	set_reg(mipi_dsi_base + MIPI_LDI_DPI0_HRZ_CTRL3, DSS_REDUCE(hsize_real), 12, 0);
	set_reg(mipi_dsi_base + MIPI_LDI_DPI0_HRZ_CTRL2, DSS_REDUCE(rect.w), 12, 0);
	set_reg(mipi_dsi_base + MIPI_LDI_VRT_CTRL2, DSS_REDUCE(rect.h), 12, 0);

	disable_dsi_ldi(dsi);
	if (is_mipi_video_panel(dsi->panel_info)) {
		set_reg(mipi_dsi_base + MIPI_LDI_FRM_MSK, 0x0, 1, 0);
		set_reg(mipi_dsi_base + DSI_CMD_MOD_CTRL, 0x1, 1, 1);
	}
	if (is_dual_mipi(dsi))
		set_reg(mipi_dsi_base + MIPI_LDI_CTRL,
			((mipi_dsi_base == dsi->ctx->mipi_dsi0_base) ? 0 : 1), 1, 13);

	if (is_mipi_cmd_panel(pinfo)) {
		if (is_dual_mipi(dsi)) {
			if (mipi_dsi_base == dsi->ctx->mipi_dsi0_base)
				set_reg(mipi_dsi_base + MIPI_DSI_TE_CTRL, (0x1<<17) | (0x1<<6) | 0x1, 18, 0);
		} else {
			set_reg(mipi_dsi_base + MIPI_DSI_TE_CTRL, (0x1<<17) | (0x1<<6) | 0x1, 18, 0);
		}
		set_reg(mipi_dsi_base + MIPI_DSI_TE_HS_NUM, 0x0, 32, 0);
		set_reg(mipi_dsi_base + MIPI_DSI_TE_HS_WD, 0x24024, 32, 0);
		set_reg(mipi_dsi_base + MIPI_DSI_TE_VS_WD, (0x3FC << 12) | (2  * lane_byte_clk / 1000000), 32, 0);
		set_reg(mipi_dsi_base + VID_SHADOW_CTRL, 0x1, 1, 0);
	}
}

void enable_dsi_ldi(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = NULL;
	if (!dsi) {
		HISI_DRM_DEBUG("enable : dsi is null!!!");
		return;
	}
	ctx = dsi->ctx;
	if (!ctx) {
		HISI_DRM_DEBUG("enable : ctx is null!!!");
		return;
	}
	if (is_dual_mipi(dsi)) {
		set_reg(ctx->mipi_dsi0_base + MIPI_LDI_CTRL, 0x1, 1, 5);
		return;
	}

	set_reg(ctx->mipi_dsi0_base + MIPI_LDI_CTRL, 0x1, 1, 0);
}

void disable_dsi_ldi(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = NULL;
	if (!dsi) {
		HISI_DRM_DEBUG("disable : dsi is null!!!");
		return;
	}
	ctx = dsi->ctx;
	if (!ctx) {
		HISI_DRM_DEBUG("disable : ctx is null!!!");
		return;
	}
	/* ldi disable */
	if (is_dual_mipi(dsi)) {
		set_reg(ctx->mipi_dsi0_base + MIPI_LDI_CTRL, 0x0, 1, 5);
		return;
	}
	set_reg(ctx->mipi_dsi0_base + MIPI_LDI_CTRL, 0x0, 1, 0);
}
#ifdef CONFIG_HISI_DSS_V510
void single_frame_update(struct hisi_dsi *dsi)
{
	if (!dsi) {
		HISI_DRM_DEBUG("single_frame_update : dsi is null!!!");
		return;
	}
	enable_dsi_ldi(dsi);
}
#endif
static void mipi_ldi_data_gate_ctrl(char __iomem *ldi_base, bool enable)
{
	set_reg(ldi_base + MIPI_LDI_FRM_VALID_DBG, 0x1, 1, 29);
}

void ldi_dsi_data_gate(struct hisi_dsi *dsi, bool enable)
{
	struct dsi_hw_ctx *ctx = NULL;
	if (!dsi) {
		HISI_DRM_DEBUG("ldi_dsi_data_gate : dsi is null!!!");
		return;
	}
	ctx = dsi->ctx;
	if (!ctx) {
		HISI_DRM_DEBUG("ldi_dsi_data_gate : ctx is null!!!");
		return;
	}
	if (is_mipi_video_panel(dsi->panel_info))
		return;

	mipi_ldi_data_gate_ctrl(ctx->mipi_dsi0_base, enable);
	if (is_dual_mipi(dsi))
		mipi_ldi_data_gate_ctrl(ctx->mipi_dsi1_base, enable);
}

void ldi_data_gate(struct hisi_drm_crtc *hisi_crtc, bool enable)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *ldi_base = NULL;
	if (!hisi_crtc) {
		HISI_DRM_DEBUG("ldi_data_gate : hisi_crtc is null!!!");
		return;
	}
	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_DEBUG("ldi_data_gate : ctx is null!!!");
		return;
	}
	if (is_dp(hisi_crtc)) {
		ldi_base = ctx->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 2);
		ldi_base = ctx->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 2);
	} else {
		mipi_ldi_data_gate_ctrl(ctx->dss_base + DSS_MIPI_DSI0_OFFSET, enable);
	}
}

/*lint +e559*/
