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
/*lint -e529 -e559 -e578*/

#include "hisi_drm_drv.h"
#include "hisi_drm_dss_global.h"

static void dump_ldi_regs(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *mipi_dsi0_base;
	int i;

	ctx = hisi_crtc->ctx;
	mipi_dsi0_base = ctx->dss_base + DSS_MIPI_DSI0_OFFSET;

	HISI_DRM_ERR("DUMP ldi regs:\n");
	for (i = 0x1B0; i < 0x24c; i += 0x10)
		HISI_DRM_ERR("regs: 0x%lx: 0x%lx, 0x%lx, 0x%lx, 0x%lx\n",
			mipi_dsi0_base + i, inp32(mipi_dsi0_base + i),
			inp32(mipi_dsi0_base + i + 4),
			inp32(mipi_dsi0_base + i + 8),
			inp32(mipi_dsi0_base + i + 0xC));

	if (is_dp(hisi_crtc)) {
		HISI_DRM_ERR("itf_int_mask:0x%x,itf_ints:0x%x,vstate:0x%x,dpi0_hstate:0x%x\n",
			inp32(ctx->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK),
			inp32(ctx->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INTS),
			inp32(ctx->dss_base + DSS_LDI_DP_OFFSET + LDI_VSTATE),
			inp32(ctx->dss_base + DSS_LDI_DP_OFFSET + LDI_DPI0_HSTATE));
	} else {
		char __iomem *mipi_dsi0_base = ctx->dss_base + DSS_MIPI_DSI0_OFFSET;

		HISI_DRM_ERR("ldi-dsi:0x%x,0x%x,0x%x,0x%x\n",
			inp32(mipi_dsi0_base + MIPI_LDI_DPI0_HRZ_CTRL2),
			inp32(mipi_dsi0_base + MIPI_LDI_VRT_CTRL2),
			inp32(mipi_dsi0_base + MIPIDSI_EDPI_CMD_SIZE_OFFSET),
			inp32(mipi_dsi0_base + MIPIDSI_VID_VACTIVE_LINES_OFFSET));
	}

	HISI_DRM_ERR("DUMP ldi regs end\n");
}

static void dump_mipi_regs(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *mipi_dsi0_base;
	int i;

	ctx = hisi_crtc->ctx;
	mipi_dsi0_base = ctx->dss_base + DSS_MIPI_DSI0_OFFSET;

	HISI_DRM_ERR("DUMP mipi0 regs:\n");
	for (i = 0; i < 0x100; i += 0x10)
		HISI_DRM_ERR("regs: 0x%lx: 0x%lx, 0x%lx, 0x%lx, 0x%lx\n",
			mipi_dsi0_base + i, inp32(mipi_dsi0_base + i),
			inp32(mipi_dsi0_base + i + 4),
			inp32(mipi_dsi0_base + i + 8),
			inp32(mipi_dsi0_base + i + 0xC));

	HISI_DRM_ERR("MIPIDSI_DPHYTX_STOPSNT_OFFSET=0x%lx\n",
		inp32(mipi_dsi0_base + MIPIDSI_DPHYTX_STOPSNT_OFFSET));
	HISI_DRM_ERR("MIPIDSI_DPHYTX_CTRL_OFFSET=0x%lx\n",
		inp32(mipi_dsi0_base + MIPIDSI_DPHYTX_CTRL_OFFSET));
	HISI_DRM_ERR("MIPIDSI_DPHYTX_TRSTOP_FLAG_OFFSET=0x%lx\n",
		inp32(mipi_dsi0_base + MIPIDSI_DPHYTX_TRSTOP_FLAG_OFFSET));

	HISI_DRM_ERR("DUMP mipi regs end\n");
}

static void dump_mctl_reg(struct hisi_drm_crtc *hisi_crtc)
{
	uint32_t chns[2] = {PRIMARY_CH, EXTERNAL_CH};
	struct dss_hw_ctx *ctx = hisi_crtc->ctx;
	char __iomem *dss_base = ctx->dss_base;
	char __iomem *mctl_sys_base = dss_base + MCTL_SYS_OFFSET;
	char __iomem *mctl_mutex_base = dss_base + MCTL_MUTEX_OFFSET;

	int i;
	for (i = 0; i < 2; i++) {
		uint32_t chn = chns[i];
		uint32_t ov_base = g_dss_module_ovl_base[i][MODULE_OVL_BASE];

		HISI_DRM_ERR("dump chn%d regs:", chn);
		HISI_DRM_ERR("CHN_OV_OEN:0x%x, CHN_FLASH_OEN:0x%x, OV_FLUSH_EN:0x%x, "\
			"RCH_OV_SEL:0x%x, MCTL_CTL_MUTEX_CHN:0x%x, LAYER_DBG_INFO:0x%x",
			inp32(dss_base + g_dss_module_base[chn][MODULE_MCTL_CHN_OV_OEN]),
			inp32(dss_base + g_dss_module_base[chn][MODULE_MCTL_CHN_FLUSH_EN]),
			inp32(mctl_sys_base + MCTL_OV0_FLUSH_EN + (i * 0x4)),
			inp32(mctl_sys_base + MCTL_RCH_OV0_SEL + (i * 0x4)),
			inp32(mctl_mutex_base + MCTL_CTL_MUTEX_RCH0 + (chn * 0x4)),
			inp32(dss_base + ov_base + OVL_LAYER0_DBG_INFO));

		HISI_DRM_ERR("mctl_mutex, base: 0x%lx", mctl_mutex_base + (i * 0x100));
		HISI_DRM_ERR("MCTL_CTL_EN: 0x%x, MCTL_CTL_MUTEX: 0x%x, "\
			"MCTL_CTL_MUTEX_STATUS: 0x%x, MCTL_CTL_MUTEX_ITF: 0x%x, "\
			"MCTL_CTL_MUTEX_DBUF:0x%x, MCTL_CTL_TOP:0x%x, "\
			"MCTL_CTL_STATUS:0x%x, MCTL_CTL_INTEN:0x%x, "\
			"MCTL_CTL_SW_ST:0x%x, MCTL_CTL_ST_SEL:0x%x, MCTL_CTL_DBG:0x%x",
			inp32(mctl_mutex_base + MCTL_CTL_EN + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_MUTEX + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_MUTEX_STATUS + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_MUTEX_ITF + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_MUTEX_DBUF + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_TOP + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_STATUS + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_INTEN + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_SW_ST + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_ST_SEL + (i * 0x100)),
			inp32(mctl_mutex_base + MCTL_CTL_DBG + (i * 0x100)));
	}

	for (i = MCTL_MOD0_STATUS; i <= MCTL_MOD20_STATUS; i += 4)
		HISI_DRM_ERR("MOD%d_STATUS: 0x%lx\n",
			(i - MCTL_MOD0_STATUS) / 4, inp32(mctl_sys_base + i));
}

static void arsr2p_reg_dump(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *arsr2p_base;
	int i = DSS_RCHN_V0;

	ctx = hisi_crtc->ctx;
	arsr2p_base = hisi_crtc->dss_module.arsr2p_base[i];
	drm_check_and_void_return(arsr2p_base == NULL, "arsr2p_base is null");

	HISI_DRM_ERR("ARSR2P: INPUT_W_H: 0x%lx, OUTPUT_W_H: 0x%lx, IHLEFT: 0x%lx, "\
		"IHLEFT1: 0x%lx, IHRIGHT: 0x%lx, IHRIGHT1: 0x%lx, IVTOP: 0x%lx, "\
		"IVBOTTOM: 0x%lx, IVBOTTOM1: 0x%lx, IHINC: 0x%lx, OFFSET: 0x%lx, "\
		"MODE:0x%lx", inp32(arsr2p_base + ARSR2P_INPUT_WIDTH_HEIGHT),
		inp32(arsr2p_base + ARSR2P_OUTPUT_WIDTH_HEIGHT),
		inp32(arsr2p_base + ARSR2P_IHLEFT),
		inp32(arsr2p_base + ARSR2P_IHLEFT1),
		inp32(arsr2p_base + ARSR2P_IHRIGHT),
		inp32(arsr2p_base + ARSR2P_IHRIGHT1),
		inp32(arsr2p_base + ARSR2P_IVTOP),
		inp32(arsr2p_base + ARSR2P_IVBOTTOM),
		inp32(arsr2p_base + ARSR2P_IVBOTTOM1),
		inp32(arsr2p_base + ARSR2P_IHINC),
		inp32(arsr2p_base + ARSR2P_IVINC),
		inp32(arsr2p_base + ARSR2P_OFFSET),
		inp32(arsr2p_base + ARSR2P_MODE));
}

static void dump_dpp_reg(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = hisi_crtc->ctx;

	HISI_DRM_ERR("dpp: 0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, "\
		"x%x, 0x%x,0x%x,0x%x, 0x%x\n",
		inp32(ctx->dss_base + DSS_DISP_GLB_OFFSET + DYN_SW_DEFAULT),
		inp32(ctx->dss_base + DSS_DPP_CH0_GAMA_OFFSET + GAMA_EN),
		inp32(ctx->dss_base + DSS_DPP_CH1_GAMA_OFFSET + GAMA_EN),
		inp32(ctx->dss_base + DSS_DPP_CH0_DEGAMMA_OFFSET + DEGAMA_EN),
		inp32(ctx->dss_base + DSS_DPP_CH1_DEGAMMA_OFFSET + DEGAMA_EN),
		inp32(ctx->dss_base + DSS_DPP_CH0_GMP_OFFSET + GMP_EN),
		inp32(ctx->dss_base + DSS_DPP_CH1_GMP_OFFSET + GMP_EN),
		inp32(ctx->dss_base + DSS_DPP_CH0_XCC_OFFSET + XCC_EN),
		inp32(ctx->dss_base + DSS_DPP_CH1_XCC_OFFSET + XCC_EN),
		inp32(ctx->dss_base + DSS_DPP_POST_XCC_OFFSET + POST_XCC_EN),
		inp32(ctx->dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_ACE),
		inp32(ctx->dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_NR),
		inp32(ctx->dss_base + DSS_HI_ACE_OFFSET + DPE_IMAGE_INFO),
		inp32(ctx->dss_base + DSS_DPP_DITHER_OFFSET + DITHER_CTL0));
}

void hisi_dump_current_info(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	uint32_t dma_base;
	int i;

	drm_check_and_void_return(hisi_crtc == NULL, "hisi_crtc is null");
	ctx = hisi_crtc->ctx;
	drm_check_and_void_return(ctx == NULL, "ctx is null");
	drm_check_and_void_return(ctx->dss_base == NULL, "ctx->dss_base is null");
	drm_check_and_void_return(ctx->pmctrl_base == NULL, "ctx->pmctrl_base is null");

	HISI_DRM_ERR("CRTC-%d, frame: %d", hisi_crtc->crtc_id, hisi_crtc->frame_no);

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (g_dss_module_base[i][MODULE_DMA] == 0)
			continue;
		dma_base = g_dss_module_base[i][MODULE_DMA];

		HISI_DRM_ERR("chn%d DMA_BUF_DBG0=0x%x, DMA_BUF_DBG1=0x%x\n", i,
			inp32(ctx->dss_base + dma_base + DMA_BUF_DBG0),
			inp32(ctx->dss_base + dma_base + DMA_BUF_DBG1));
	}

	HISI_DRM_ERR("DSS_DISP_CH0_OFFSET: 0x%x, 0x%x, DSS_DISP_CH1_OFFSET: 0x%x, 0x%x\n",
		inp32(ctx->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR),
		inp32(ctx->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR),
		inp32(ctx->dss_base + DSS_DISP_CH1_OFFSET + IMG_SIZE_AFT_SR),
		inp32(ctx->dss_base + DSS_DISP_CH1_OFFSET + IMG_SIZE_AFT_SR));

	HISI_DRM_ERR("ov:0x%x, 0x%lx, dbuf:0x%x,0x%x,disp_ch:0x%x,0x%x,0x%x\n",
		inp32(ctx->dss_base +  DSS_OVL0_OFFSET + OV_SIZE),
		inp32(ctx->dss_base +  DSS_OVL1_OFFSET + OV_SIZE),
		inp32(ctx->dss_base +  DSS_DBUF0_OFFSET + DBUF_FRM_SIZE),
		inp32(ctx->dss_base +  DSS_DBUF0_OFFSET + DBUF_FRM_HSIZE),
		inp32(ctx->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR),
		inp32(ctx->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR),
		inp32(ctx->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_IFBCSW));


	HISI_DRM_ERR("PMCTRL PERI_CTRL4, CTRL5 = 0x%x, 0x%x\n",
		inp32(ctx->pmctrl_base + PMCTRL_PERI_CTRL4),
		inp32(ctx->pmctrl_base + PMCTRL_PERI_CTRL5));

	dump_dpp_reg(hisi_crtc);
	dump_mctl_reg(hisi_crtc);
	dump_ldi_regs(hisi_crtc);
	dump_mipi_regs(hisi_crtc);
	arsr2p_reg_dump(hisi_crtc);
}

/*lint +e529 +e559 +e578*/
