/* Copyright (c) 2021-2021, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_overlay_utils.h"

static void hisi_dump_dma_info(struct dpu_fb_data_type *dpufd)
{
	int i;

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (g_dss_module_base[i][MODULE_DMA] == 0)
			continue;
		DPU_FB_INFO("0x%x, 0x%x, %d, 0x%x!\n",
			inp32(dpufd->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG0),
			inp32(dpufd->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG1),
			i, inp32(dpufd->dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MOD0_STATUS + i * 0x4));
	}
}

static void hisi_dump_vote_info(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_data_type *fb0 = dpufd_list[PRIMARY_PANEL_IDX];
	struct dpu_fb_data_type *fb1 = dpufd_list[EXTERNAL_PANEL_IDX];
	struct dpu_fb_data_type *fb2 = dpufd_list[AUXILIARY_PANEL_IDX];
	struct dpu_fb_data_type *fb3 = dpufd_list[MEDIACOMMON_PANEL_IDX];

	DPU_FB_INFO("fb%d, [%d, %d, %d, %d], [%llu, %llu, %llu, %llu]\n",
		dpufd->index,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_voltage_level : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_voltage_level : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_voltage_level : 0,
		(fb3 != NULL) ? fb3->dss_vote_cmd.dss_voltage_level : 0,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb3 != NULL) ? fb3->dss_vote_cmd.dss_pri_clk_rate : 0);
}

static void hisi_dump_mediacrg_info(struct dpu_fb_data_type *dpufd)
{
	int i;

	for (i = 0; i < 10; i++)
		DPU_FB_INFO("clk%d: 0x%x\n", i + 1,
			inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + i * 4));
}

static void hisi_dump_dpp_info(struct dpu_fb_data_type *dpufd)
{
	DPU_FB_INFO("0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x,0x%x, 0x%x\n",
		inp32(dpufd->dss_base + DSS_DPP_OFFSET + DPP_IMG_SIZE_BEF_SR),
		inp32(dpufd->dss_base + DSS_DPP_OFFSET + DPP_IMG_SIZE_AFT_SR),
		inp32(dpufd->dss_base + DSS_DPP_GAMA_OFFSET + GAMA_EN),
		inp32(dpufd->dss_base + DSS_DPP_DEGAMMA_OFFSET + DEGAMA_EN),
		inp32(dpufd->dss_base + DSS_DPP_GMP_OFFSET + GMP_EN),
		inp32(dpufd->dss_base + DSS_DPP_XCC_OFFSET + LCP_XCC_BYPASS_EN),
		inp32(dpufd->dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_ACE),
		inp32(dpufd->dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_NR),
		inp32(dpufd->dss_base + DSS_HI_ACE_OFFSET + DPE_IMAGE_INFO),
		inp32(dpufd->dss_base + DSS_DPP_DITHER_OFFSET + DITHER_CTL0));
}

static void hisi_dump_arsr_post_info(struct dpu_fb_data_type *dpufd)
{
	dpu_check_and_no_retval(!dpufd->dss_module.post_scf_base, ERR, "post_scf_base is NULL!\n");
#if defined(CONFIG_HISI_FB_970)
	DPU_FB_INFO("0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x, 0x%x\n",
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_MODE),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHLEFT),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHLEFT1),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHRIGHT),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHRIGHT1),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IVTOP),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IVBOTTOM),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHINC),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IVINC));
#else
	DPU_FB_INFO("0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x\n",
		inp32(dpufd->dss_module.post_scf_base + SCF_EN_HSCL_STR),
		inp32(dpufd->dss_module.post_scf_base + SCF_EN_VSCL_STR),
		inp32(dpufd->dss_module.post_scf_base + SCF_INPUT_WIDTH_HEIGHT),
		inp32(dpufd->dss_module.post_scf_base + SCF_OUTPUT_WIDTH_HEIGHT),
		inp32(dpufd->dss_module.post_scf_base + SCF_EN_HSCL),
		inp32(dpufd->dss_module.post_scf_base + SCF_EN_VSCL),
		inp32(dpufd->dss_module.post_scf_base + SCF_INC_HSCL),
		inp32(dpufd->dss_module.post_scf_base + SCF_INC_VSCL),
		inp32(dpufd->dss_module.post_scf_base + SCF_CLK_SEL),
		inp32(dpufd->dss_module.post_scf_base + SCF_CLK_EN));
#endif
}

void hisi_dump_current_info(struct dpu_fb_data_type *dpufd)
{
	char __iomem *mipi_dsi_base = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL!\n");
	dpu_check_and_no_retval(!dpufd->dss_base, ERR, "dpufd->dss_base is NULL!\n");
	dpu_check_and_no_retval(!dpufd->pmctrl_base, ERR, "dpufd->pmctrl_base is NULL!\n");

	if ((dpufd->index != PRIMARY_PANEL_IDX) && (dpufd->index != EXTERNAL_PANEL_IDX))
		return;

	hisi_dump_dma_info(dpufd);
	mipi_dsi_base = get_mipi_dsi_base(dpufd);
	dpu_check_and_no_retval(!mipi_dsi_base, ERR, "dpufd->mipi_dsi_base is NULL!\n");

	DPU_FB_INFO("aif: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		inp32(dpufd->dss_base + AIF0_MONITOR_OS_R0), inp32(dpufd->dss_base + AIF0_MONITOR_OS_R1),
		inp32(dpufd->dss_base + AIF0_MONITOR_OS_R2), inp32(dpufd->dss_base + AIF0_MONITOR_OS_R3),
		inp32(dpufd->dss_base + AIF0_MONITOR_OS_R4));

	hisi_dump_vote_info(dpufd);
	hisi_dump_arsr_post_info(dpufd);
	hisi_dump_dpp_info(dpufd);
	DPU_FB_INFO("ov:0x%x,dbuf:0x%x,0x%x\n",
		inp32(dpufd->dss_base + DSS_OVL0_OFFSET + OV_SIZE),
		inp32(dpufd->dss_base + DSS_DBUF0_OFFSET + DBUF_FRM_SIZE),
		inp32(dpufd->dss_base + DSS_DBUF0_OFFSET + DBUF_FRM_HSIZE));

#if defined(CONFIG_HISI_FB_970)
	DPU_FB_INFO("dsc:0x%x\n", inp32(dpufd->dss_base + DSS_DSC_OFFSET + DSC_PIC_SIZE));
#endif

	DPU_FB_INFO("ldi-dsi:0x%x,0x%x vstate:0x%x\n",
		inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_DPI0_HRZ_CTRL2),
		inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_VRT_CTRL2),
		inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_VSTATE));

	hisi_dump_mediacrg_info(dpufd);

	DPU_FB_INFO("PMCTRL:0x%x, 0x%x\n",
		inp32(dpufd->pmctrl_base + PMCTRL_PERI_CTRL4), inp32(dpufd->pmctrl_base + PMCTRL_PERI_CTRL5));
}
