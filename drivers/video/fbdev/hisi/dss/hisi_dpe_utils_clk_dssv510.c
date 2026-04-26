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

DEFINE_SEMAPHORE(hisi_fb_dss_inner_clk_sem);
static int g_dss_inner_clk_refcount;

#if defined(CONFIG_HISI_FB_AOD)
extern bool hisi_aod_get_aod_status(void);
#endif

void dss_inner_clk_common_enable(struct dpu_fb_data_type *dpufd,
	bool fastboot_enable)
{
	int prev_refcount;
	bool is_in_aod = false;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL point\n");
		return;
	}

	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		return;

	down(&hisi_fb_dss_inner_clk_sem);

	prev_refcount = g_dss_inner_clk_refcount++;
	if (!prev_refcount && !fastboot_enable) {
		if (g_fpga_flag == 1) {
			no_memory_lp_ctrl(dpufd);
		} else {
#if defined(CONFIG_HISI_FB_AOD)
			is_in_aod = hisi_aod_get_aod_status();
#endif
			if (!is_in_aod) {
#ifdef CONFIG_DSS_LP_USED
				lp_first_level_clk_gate_ctrl(dpufd);
				lp_second_level_clk_gate_ctrl(dpufd);
#else
				no_memory_lp_ctrl(dpufd);
#endif
			}
		}
	}

	DPU_FB_DEBUG("fb%d, g_dss_inner_clk_refcount=%d\n",
		dpufd->index, g_dss_inner_clk_refcount);

	up(&hisi_fb_dss_inner_clk_sem);
}

void dss_inner_clk_common_disable(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL point\n");
		return;
	}

	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		return;

	down(&hisi_fb_dss_inner_clk_sem);
	if (--g_dss_inner_clk_refcount < 0)
		DPU_FB_ERR("dss new_refcount err");

	DPU_FB_DEBUG("fb%d, g_dss_inner_clk_refcount=%d\n", dpufd->index, g_dss_inner_clk_refcount);
	up(&hisi_fb_dss_inner_clk_sem);
}

void dss_inner_clk_pdp_enable(struct dpu_fb_data_type *dpufd, bool fastboot_enable)
{

	(void *)dpufd;
	(void)fastboot_enable;
}

void dss_inner_clk_pdp_disable(struct dpu_fb_data_type *dpufd)
{
	(void *)dpufd;
}

void dss_inner_clk_sdp_enable(struct dpu_fb_data_type *dpufd)
{
	char __iomem *dss_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}
	dss_base = dpufd->dss_base;

#ifdef CONFIG_DSS_LP_USED
	if (is_dp_panel(dpufd)) {
		outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_MODULE_CLK_SEL, 0xE);
		outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_MODULE_CLK_SEL, 0xE);
		outp32(dss_base + DSS_PIPE_SW_DP_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
		outp32(dss_base + DSS_PIPE_SW3_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
	}
	outp32(dss_base + DSS_DISP_CH2_OFFSET + DISP_CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_OVL1_OFFSET + OV8_CLK_SEL, 0x00000000);

#else
	if (is_dp_panel(dpufd)) {
		outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_MEM_CTRL, 0x00000008);
		outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_MEM_CTRL, 0x00000008);
	}
	outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DISP_CH2_OFFSET + DISP_CH_DITHER_MEM_CTRL, 0x00000008);
#endif
}

void dss_inner_clk_sdp_disable(struct dpu_fb_data_type *dpufd)
{
	int ret;
	uint64_t dss_mmbuf_rate;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL point!");
		return;
	}

	dpufd->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;

	if (dpufd_list[PRIMARY_PANEL_IDX]) {
		dss_mmbuf_rate = dpufd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_mmbuf_rate;
		ret = clk_set_rate(dpufd->dss_mmbuf_clk, dss_mmbuf_rate);
		if (ret < 0) {
			DPU_FB_ERR("fb%d dss_mmbuf clk_set_rate %llu failed, error=%d!\n",
				dpufd->index, dss_mmbuf_rate, ret);
			return;
		}
	}
}


