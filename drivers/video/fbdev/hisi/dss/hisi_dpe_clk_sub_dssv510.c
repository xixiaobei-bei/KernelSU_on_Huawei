/* Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

/* Function: set reg depends on platforms'capacity,
 *	some platform don't support some modules
 * Parameters:
 *	@Input:
 *		dss_base: the dss reg base addr
 *		module_offset: the module offset addr in dss
 *		reg_offset: the register offset addr in module
 *		value: the value you want to set
 *	@output:
 *		None
 *	Modified:2019.11.01 Created
 */
static void dss_set_reg_inheritor(char __iomem *dss_base,
	uint32_t module_offset, uint32_t reg_offset, uint32_t value)
{
	if (module_offset == 0)
		return;
	outp32(dss_base + module_offset + reg_offset, value);
}

#ifdef CONFIG_DSS_LP_USED
void lp_first_level_clk_gate_ctrl(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dss_base = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");
	dss_base = dpufd->dss_base;
	outp32(dss_base + GLB_MODULE_CLK_SEL, 0x01800001);
	outp32(dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL,
		DSS_MODULE_CORE_CLK_SEL_DEFAULT_VAL);

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
	inheritor_dss_wr_reg(dss_base + DSS_RCH_VG0_DMA_OFFSET + CH_CLK_EN, 0xFFFF7FFF);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_RCH_VG2_DMA_OFFSET, CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	dss_set_reg_inheritor(dss_base, DSS_RCH_G1_DMA_OFFSET, CH_CLK_SEL, 0x00000000);
	dss_set_reg_inheritor(dss_base, DSS_RCH_G1_DMA_OFFSET, FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE, 0x0000000C);
	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_RCH_D1_DMA_OFFSET, CH_CLK_SEL, 0x00000000);
}

void lp_second_level_clk_gate_ctrl(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dss_base = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");
	dss_base = dpufd->dss_base;
	outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_VBIF0_AIF + AIF_CLK_SEL0, 0x00000000);
	outp32(dss_base + DSS_VBIF0_AIF + AIF_CLK_SEL1, 0x00000000);
	outp32(dss_base + DSS_SMMU_OFFSET + SMMU_LP_CTRL, 0x00000001);

	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL0, 0x00000000);
	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL1, 0x00000000);

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_CLK_SEL, 0x00000000);
	inheritor_dss_wr_reg(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_CLK_EN, 0xFFFF7FFF);
	outp32(dss_base + DSS_HI_ACE_OFFSET + DPE_RAMCLK_FUNC, 0x00000000);
	outp32(dss_base + DSS_DPP_OFFSET + DPP_CLK_SEL, 0x00000000);

	dss_set_reg_inheritor(dss_base + DSS_DISP_CH0_OFFSET, DISP_CH_SPR_CLK_SEL,
		0, 0x00000000); // spr(spr+dsc12a)

	/* notice: es, dsc no lp */
	outp32(dss_base + DSS_DSC_OFFSET + DSC_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_DISP_CH1_OFFSET + DISP_CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_DPP1_OFFSET + DPP_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_WB_OFFSET + WB_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_MIF_OFFSET + MIF_CLK_CTL,  0x00000001);
	config_mctl_clk_gate(dss_base);
	config_rch_clk_gate(dss_base);
	dss_set_reg_inheritor(dss_base, DSS_WCH0_DMA_OFFSET, CH_CLK_SEL, 0x00000000);
	dss_set_reg_inheritor(dss_base, DSS_WCH0_DMA_OFFSET, FBCE_CREG_CTRL_GATE, 0x0000000C);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + FBCE_CREG_CTRL_GATE, 0x0000000C);

	outp32(dss_base + DSS_OVL0_OFFSET + OV8_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_OVL2_OFFSET + OV8_CLK_SEL, 0x00000000);
	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_OVL3_OFFSET, OV2_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_PIPE_SW_DSI0_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_PIPE_SW_WB_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
}
#endif

/*lint -e838*/
static void config_rch_memory(char __iomem *dss_base)
{
	if (dss_base == NULL) {
		DPU_FB_ERR("dss_base is NULL");
		return;
	}

	outp32(dss_base + DSS_RCH_VG0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_VG0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_ARSR_OFFSET + ARSR2P_LB_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + VPP_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);
	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_RCH_VG1_SCL_OFFSET, SCF_COEF_MEM_CTRL, 0x00000088);

	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_RCH_VG1_SCL_OFFSET, SCF_LB_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);
	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_RCH_VG2_DMA_OFFSET, DMA_BUF_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_RCH_G0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_G0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_RCH_G1_SCL_OFFSET, SCF_COEF_MEM_CTRL, 0x00000088);
	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_RCH_G1_SCL_OFFSET, SCF_LB_MEM_CTRL, 0x0000008);

	dss_set_reg_inheritor(dss_base, DSS_RCH_G1_DMA_OFFSET, DMA_BUF_MEM_CTRL, 0x00000008);
	dss_set_reg_inheritor(dss_base, DSS_RCH_G1_DMA_OFFSET, AFBCD_MEM_CTRL, 0x00008888);
	outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_RCH_D1_DMA_OFFSET, DMA_BUF_MEM_CTRL, 0x00000008);
}

void no_memory_lp_ctrl(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dss_base = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");
	dss_base = dpufd->dss_base;

	/* dssv350: dsi apb clk must be enabled after dsi reset, so now disable dsi apb clk */
	inheritor_dss_wr_reg(dss_base + GLB_MODULE_CLK_SEL, 0xFE7FFFFF);
	inheritor_dss_wr_reg(dss_base + GLB_MODULE_CLK_EN, 0xFE7FFFFF);
	outp32(dss_base + GLB_DSS_PM_CTRL, 0x0401A00F);

	outp32(dss_base + DSS_DSC_OFFSET + DSC_MEM_CTRL, 0x00000088);

	outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_MEM_CTRL, 0x00000008);

	config_rch_memory(dss_base);

	dss_set_reg_inheritor(dss_base, DSS_WCH0_DMA_OFFSET, DMA_BUF_MEM_CTRL, 0x00000008);
	dss_set_reg_inheritor(dss_base, DSS_WCH0_DMA_OFFSET, AFBCE_MEM_CTRL, 0x00000888);
	dss_set_reg_inheritor(dss_base, DSS_WCH0_DMA_OFFSET, ROT_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	/* afbce hfbce dssv350: 0x00000888 */
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + AFBCE_MEM_CTRL, 0x88888888);
	/* hfbce mem dssv350: researve */
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + AFBCE_MEM_CTRL_1, 0x00000088);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + WCH_SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + WCH_SCF_LB_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_ARSR2P_LB_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_ARSR2P_COEF_MEM_CTRL, 0x00000088);
	/* dssv350 not support this module */
	dss_set_reg_inheritor(dss_base, DSS_HDR_OFFSET, HDR_MEM_CTRL, 0x08888888);

	dss_set_reg_inheritor(dss_base + DSS_DISP_CH0_OFFSET,
			DISP_CH_SPR_MEM_CTRL, 0, 0x00000088); // spr(spr+dsc12a)
	dss_set_reg_inheritor(dss_base + DSS_DISP_CH0_OFFSET,
			DISP_CH_ALSC_DEGAMA_MEM_CTRL, 0, 0x00000008); // alsc degamma_mem

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_DITHER_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_DPP_GAMA_OFFSET + GAMA_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DPP_DEGAMMA_OFFSET + DEGAMA_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DPP_GMP_OFFSET + GMP_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DSC_OFFSET + DSC_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_DPP1_GAMA_OFFSET + GAMA_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DPP1_DEGAMMA_OFFSET + DEGAMA_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DPP1_GMP_OFFSET + GMP_MEM_CTRL, 0x00000008);
}