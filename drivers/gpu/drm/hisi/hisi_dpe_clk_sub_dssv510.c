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
#include "hisi_drm_dpe_utils.h"

#ifndef CONFIG_DSS_LP_USED
static void set_rch_reg(char __iomem *base)
{
	outp32(base + DSS_RCH_VG0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(base + DSS_RCH_VG0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_VG0_ARSR_OFFSET + ARSR2P_LB_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_VG0_DMA_OFFSET + VPP_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_VG0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_VG0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);
	outp32(base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);

	outp32(base + DSS_RCH_VG1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(base + DSS_RCH_VG1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_VG1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_VG1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);
	outp32(base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);
	outp32(base + DSS_RCH_VG2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_G0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(base + DSS_RCH_G0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);
	outp32(base + DSS_RCH_G0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_G0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(base + DSS_RCH_G1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(base + DSS_RCH_G1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);
	outp32(base + DSS_RCH_G1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_G1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(base + DSS_RCH_D2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_D3_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_D0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_RCH_D0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(base + DSS_RCH_D1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
}

static void set_wch_dma_reg(char __iomem *base)
{
	outp32(base + DSS_WCH0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_WCH0_DMA_OFFSET + AFBCE_MEM_CTRL, 0x00000888);
	outp32(base + DSS_WCH0_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);
	outp32(base + DSS_WCH1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(base + DSS_WCH1_DMA_OFFSET + AFBCE_MEM_CTRL, 0x88888888);
	outp32(base + DSS_WCH1_DMA_OFFSET + AFBCE_MEM_CTRL_1, 0x00000088);
	outp32(base + DSS_WCH1_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);
	outp32(base + DSS_WCH1_DMA_OFFSET + WCH_SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(base + DSS_WCH1_DMA_OFFSET + WCH_SCF_LB_MEM_CTRL, 0x00000008);

}

void no_memory_lp_ctrl(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *base = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}
	base = ctx->dss_base;

	outp32(base + GLB_DSS_PM_CTRL, 0x0401A00F);

	outp32(base + DSS_DSC_OFFSET + DSC_MEM_CTRL, 0x00000088);

	outp32(base + DSS_CMDLIST_OFFSET + CMD_MEM_CTRL, 0x00000008);

	set_rch_reg(base);

	set_wch_dma_reg(base);

	outp32(base + DSS_DBUF0_OFFSET + DBUF_MEM_CTRL, 0x00000008);

	outp32(base + DSS_DISP_CH0_OFFSET +
		DISP_CH_ARSR2P_LB_MEM_CTRL, 0x00000008);
	outp32(base + DSS_DISP_CH0_OFFSET +
		DISP_CH_ARSR2P_COEF_MEM_CTRL, 0x00000088);

	outp32(base + DSS_HDR_OFFSET + HDR_MEM_CTRL, 0x08888888);

	outp32(base + DSS_DISP_CH0_OFFSET +
		DISP_CH_DITHER_MEM_CTRL, 0x00000008);

	outp32(base + DSS_DPP_GAMA_OFFSET + GAMA_MEM_CTRL, 0x00000008);
	outp32(base + DSS_DPP_DEGAMMA_OFFSET + DEGAMA_MEM_CTRL, 0x00000008);
	outp32(base + DSS_DPP_GMP_OFFSET + GMP_MEM_CTRL, 0x00000008);

	outp32(base + DSS_DSC_OFFSET + DSC_MEM_CTRL, 0x00000088);
	outp32(base + DSS_DSC_OFFSET + DSC_CLK_SEL, 0x00000007);

	outp32(base + DSS_DPP1_GAMA_OFFSET + GAMA_MEM_CTRL, 0x00000008);
	outp32(base + DSS_DPP1_DEGAMMA_OFFSET + DEGAMA_MEM_CTRL, 0x00000008);
	outp32(base + DSS_DPP1_GMP_OFFSET + GMP_MEM_CTRL, 0x00000008);
}
#else
void lp_first_level_clk_gate_ctrl(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *dss_base = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	dss_base = ctx->dss_base;
	outp32(dss_base + GLB_MODULE_CLK_SEL, 0x01800000);
	outp32(dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL, 0x00030000);

	outp32(dss_base + DSS_VBIF0_AIF + AIF_MODULE_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_VBIF1_AIF + AIF_MODULE_CLK_SEL, 0x00000000);
}

static void config_mctl_clk_gate(char __iomem *dss_base)
{
	outp32(dss_base + DSS_MCTRL_CTL0_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL1_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL2_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL3_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL4_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL5_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MCTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MOD_CLK_SEL, 0x00000000);
}

static void config_rch_clk_gate(char __iomem *dss_base)
{
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	outp32(dss_base + DSS_RCH_VG2_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	outp32(dss_base + DSS_RCH_D1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
}

void lp_second_level_clk_gate_ctrl(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *dss_base = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	dss_base = ctx->dss_base;
	outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_VBIF0_AIF + AIF_CLK_SEL0, 0x00000000);
	outp32(dss_base + DSS_VBIF0_AIF + AIF_CLK_SEL1, 0x00000000);
	outp32(dss_base + DSS_SMMU_OFFSET + SMMU_LP_CTRL, 0x00000001);

	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL0, 0x00000000);
	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL1, 0x00000000);

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_HI_ACE_OFFSET + DPE_RAMCLK_FUNC, 0x00000000);
	outp32(dss_base + DSS_DPP_OFFSET + DPP_CLK_SEL, 0x00000000);

	/* notice: es, dsc no lp */
	outp32(dss_base + DSS_DSC_OFFSET + DSC_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_DISP_CH1_OFFSET + DISP_CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_DPP1_OFFSET + DPP_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_WB_OFFSET + WB_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_MIF_OFFSET + MIF_CLK_CTL,  0x00000001);
	config_mctl_clk_gate(dss_base);
	config_rch_clk_gate(dss_base);
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_WCH0_FBCE_CREG_CTRL_GATE,  0x0000000C);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_WCH1_FBCE_CREG_CTRL_GATE,  0x0000000C);

	outp32(dss_base + DSS_OVL0_OFFSET + OV8_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_OVL2_OFFSET + OV8_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_OVL3_OFFSET + OV2_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_PIPE_SW_DSI0_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_PIPE_SW_WB_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
}
#endif


