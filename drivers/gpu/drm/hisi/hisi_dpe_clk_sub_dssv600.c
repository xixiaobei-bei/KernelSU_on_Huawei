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

static void config_rch_memory(char __iomem *dss_base)
{
	outp32(dss_base + DSS_RCH_VG0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);  /* RCH_V0, SCF MEM */
	outp32(dss_base + DSS_RCH_VG0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);  /* RCH_V0, SCF MEM */
	outp32(dss_base + DSS_RCH_VG0_ARSR_OFFSET + ARSR2P_LB_MEM_CTRL, 0x00000008);  /* RCH_V0, ARSR_PRE MEM */
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + VPP_MEM_CTRL, 0x00000008);  /* RCH_V0, VPP MEM */
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* RCH_V0, DMA_BUF MEM */
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);  /* RCH_V0, AFBCD MEM */
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);  /* RCH_V0, HFBCD MEM */
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);  /* RCH_V0, HFBCD MEM */
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + HEBCD_MEM_CTRL, 0x00000088);  /* RCH_V0, HEBCD MEM */

	outp32(dss_base + DSS_RCH_VG1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);  /* RCH_V1, SCF MEM */
	outp32(dss_base + DSS_RCH_VG1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);  /* RCH_V1, SCF MEM */
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* RCH_V1, DMA_BUF MEM */
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);  /* RCH_V1, AFBCD MEM */
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);  /* RCH_V1, HFBCD MEM */
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);  /* RCH_V1, HFBCD MEM */
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + HEBCD_MEM_CTRL, 0x00000088);  /* RCH_V1, HEBCD MEM */

	outp32(dss_base + DSS_RCH_VG2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* RCH_V2, DMA_BUF MEM */

	outp32(dss_base + DSS_RCH_G0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);  /* RCH_G0, SCF MEM */
	outp32(dss_base + DSS_RCH_G0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);  /* CH_G0, SCF MEM */
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* RCH_G0, DMA_BUF MEM */
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);  /* RCH_G0, AFBCD MEM */
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + HEBCD_MEM_CTRL, 0x00000088);  /* RCH_G0, HEBCD MEM */

	outp32(dss_base + DSS_RCH_G1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);  /* RCH_G1, SCF MEM */
	outp32(dss_base + DSS_RCH_G1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);  /* RCH_G1, SCF MEM */
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* RCH_G1, DMA_BUF MEM */
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);  /* RCH_G1, AFBCD MEM */
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + HEBCD_MEM_CTRL, 0x00000088);  /* RCH_G1, HEBCD MEM */

	outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* RCH_D2, DMA_BUF MEM */
	outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* RCH_D3, DMA_BUF MEM */
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* RCH_D0, DMA_BUF MEM */
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);  /* RCH_D0, AFBCD MEM */
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + HEBCD_MEM_CTRL, 0x00000088);  /* RCH_D0, HEBCD MEM */
	outp32(dss_base + DSS_RCH_D1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* RCH_D1, DMA_BUF MEM */
}

void no_memory_lp_ctrl(struct hisi_drm_crtc *hisi_crtc)
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

	outp32(dss_base + GLB_DSS_PM_CTRL, 0x0401A00F);

	outp32(dss_base + GLB_MODULE_CLK_SEL, 0xFFFFFFFF);  /* APB CLK */

	outp32(dss_base + DSS_DSC_OFFSET + DSC_MEM_CTRL, 0x00000088);  /*  VESA MEM */

	outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_MEM_CTRL, 0x00000008);  /* CMD MEM */
	outp32(dss_base + VBIF0_MIF_OFFSET + MIF_MEM_CTRL, 0x00000008);  /* MIF MEM */

	config_rch_memory(dss_base);

	outp32(dss_base + DSS_WCH0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* WCH0, DMA_BUF MEM */
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + AFBCE_MEM_CTRL, 0x00000888);  /* WCH0, AFBCE MEM */
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + HEBCE_MEM_CTRL, 0x00000008);  /* WCH0, HEBCE MEM */
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);  /* WCH0, ROT MEM */

	outp32(dss_base + DSS_WCH1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);  /* WCH1, DMA_BUF MEM */
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + AFBCE_MEM_CTRL, 0x00000888);  /* WCH1, AFBCE MEM */
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + HEBCE_MEM_CTRL, 0x00000008);  /* WCH1, HEBCE MEM */
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);  /* WCH1, ROT MEM */
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + WCH_SCF_COEF_MEM_CTRL, 0x00000088);  /* WCH1, SCF MEM */
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + WCH_SCF_LB_MEM_CTRL, 0x00000008);  /* WCH1, SCF MEM */

	outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_MEM_CTRL, 0x00000008);  /* DBUF0 MEM */
	outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_MEM_CTRL, 0x00000008);  /* DBUF1 MEM */

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_ARSR2P_LB_MEM_CTRL, 0x00000008);  /* CH0 ARSR LB_MEM */
	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_ARSR2P_COEF_MEM_CTRL, 0x00000088);  /* CH0 ARSR COEF_MEM */
	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_DITHER_MEM_CTRL, 0x00000008);  /* CH0 DITHER MEM */
	outp32(dss_base + ALSC_OFFSET + ALSC_DEGAMMA_MEM_CTRL, 0x00000008);  /* CH0 ALSC DEGAMMA MEM */

	outp32(dss_base + DSS_DPP_GAMA_OFFSET + GAMA_MEM_CTRL, 0x00000008);  /* DPP0 GAMMA MEM */
	outp32(dss_base + DSS_DPP_DEGAMMA_OFFSET + DEGAMA_MEM_CTRL, 0x00000008);  /* DPP0 DEGAMMA MEM */
	outp32(dss_base + DSS_DPP_GMP_OFFSET + GMP_MEM_CTRL, 0x00000008);  /* DPP0 GMP MEM */

	outp32(dss_base + DSS_DSC_OFFSET + DSC_MEM_CTRL, 0x00000088);  /* CH0 DSC MEM */

	outp32(dss_base + DSS_DPP1_GAMA_OFFSET + GAMA_MEM_CTRL, 0x00000008);  /* DPP1 GAMMA MEM */
	outp32(dss_base + DSS_DPP1_DEGAMMA_OFFSET + DEGAMA_MEM_CTRL, 0x00000008);  /* DPP1 DEGAMMA MEM */
	outp32(dss_base + DSS_DPP1_GMP_OFFSET + GMP_MEM_CTRL, 0x00000008);  /* DPP1 GMP MEM */
	outp32(dss_base + DSS_DISP_CH2_OFFSET + DISP_CH_DITHER_MEM_CTRL, 0x00000008);  /* CH2 DITHER MEM */
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
	outp32(dss_base + GLB_DSS_PM_CTRL, 0x0401A00F);

	outp32(dss_base + GLB_MODULE_CLK_SEL, 0x00000000); /* core clk, pclk */
	outp32(dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL, 0x00000000); /* disp core clk */

	outp32(dss_base + DSS_VBIF0_AIF + AIF_MODULE_CLK_SEL, 0x00000000); /* axi clk */
	outp32(dss_base + DSS_VBIF1_AIF + AIF_MODULE_CLK_SEL, 0x00000000); /* mmbuf clk */

}

static void config_mctl_clk_gate(char __iomem *dss_base)
{
	outp32(dss_base + DSS_MCTRL_CTL0_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);  /* mctl mutex0 */
	outp32(dss_base + DSS_MCTRL_CTL1_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);  /* mctl mutex1 */
	outp32(dss_base + DSS_MCTRL_CTL2_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);  /* mctl mutex2 */
	outp32(dss_base + DSS_MCTRL_CTL3_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);  /* mctl mutex3 */
	outp32(dss_base + DSS_MCTRL_CTL4_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);  /* mctl mutex4 */
	outp32(dss_base + DSS_MCTRL_CTL5_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);  /* mctl mutex5 */
	outp32(dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MCTL_CLK_SEL, 0x00000000);  /*  mctl1_sys */
	outp32(dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MOD_CLK_SEL, 0x00000000);  /* mctl1_sy*/

}

static void config_rch_clk_gate(char __iomem *dss_base)
{
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* rch_v0 */
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);  /* rch_v0 */
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* rch_v1 */
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);  /* rch_v1 */
	outp32(dss_base + DSS_RCH_VG2_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* rch_v2 */
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* rch_g0 */
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);  /* rch_g0 */
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* rch_g1 */
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);  /* rch_g1 */
	outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* rch_d2 */
	outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* rch_d3 */
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* rch_d0 */
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);  /*  rch_d0 */
	outp32(dss_base + DSS_RCH_D1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* rch_d1 */

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
	outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_CLK_SEL, 0x00000000);  /* cmd */
	outp32(dss_base + AIF0_CH0_OFFSET + AIF_CLK_SEL0, 0xFFFFFFFF);  /* aif0 */
	outp32(dss_base + AIF0_CH0_OFFSET + AIF_CLK_SEL1, 0xFFFFFFFF);  /* aif0 */

	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL0, 0x00000000);  /* aif1 */
	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL1, 0x00000000);  /* aif1 */

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_CLK_SEL, 0x00000000);  /* ch0 */

	outp32(dss_base + DSS_HI_ACE_OFFSET + DPE_RAMCLK_FUNC, 0x00000000);  /* hiace */
	outp32(dss_base + DSS_DPP_OFFSET + DPP_CLK_SEL, 0x00000000);  /* dpp0 */

	outp32(dss_base + DSS_DSC_OFFSET + DSC_CLK_SEL, 0x00000000);  /* vesa(dsc12a) */
	outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_CLK_SEL, 0x00000000);  /* dbuf0 */

	outp32(dss_base + DSS_DISP_CH1_OFFSET + DISP_CH_CLK_SEL, 0x00000000);  /* ch1 */
	outp32(dss_base + DSS_DPP1_OFFSET + DPP_CLK_SEL, 0x00000000);  /* dpp1 */

	outp32(dss_base + DSS_DISP_CH2_OFFSET + DISP_CH_CLK_SEL, 0x00000000);  /* ch2 */
	outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_CLK_SEL, 0x00000000);  /* dbuf1 */

	outp32(dss_base + DSS_WB_OFFSET + WB_CLK_SEL, 0x00000000);  /* wb */

	outp32(dss_base + DSS_MIF_OFFSET + MIF_CLK_CTL,  0x00000003);  /* mif */
	config_mctl_clk_gate(dss_base);
	config_rch_clk_gate(dss_base);
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* wch0 */
	outp32(dss_base + DSS_WCH0_FBCE_CREG_CTRL_GATE,  0x0000000C);  /* wch0 */
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);  /* wch1 */
	outp32(dss_base + DSS_WCH1_FBCE_CREG_CTRL_GATE,  0x0000000C);  /* wch1 */

	outp32(dss_base + DSS_OVL0_OFFSET + OV8_CLK_SEL, 0x00000000);  /* ov0 */
	outp32(dss_base + DSS_OVL2_OFFSET + OV8_CLK_SEL, 0x00000000);  /* ov2 */
	outp32(dss_base + DSS_OVL1_OFFSET + OV8_CLK_SEL, 0x00000000);  /* ov1 */
	outp32(dss_base + DSS_OVL3_OFFSET + OV2_CLK_SEL, 0x00000000);  /* ov3 */

	outp32(dss_base + DSS_PIPE_SW_DSI0_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);  /* pipe_sw0 */
	outp32(dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);  /* pipe_sw1 */
	outp32(dss_base + DSS_PIPE_SW_DP_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);  /* pipe_sw2 */
	outp32(dss_base + DSS_PIPE_SW3_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);  /* pipe_sw3 */
	outp32(dss_base + DSS_PIPE_SW_WB_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);  /* pipe_sw4 */

}
#endif

