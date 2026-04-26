/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_fb.h"
#include "hisi_mipi_dsi.h"
#ifdef CONFIG_HISI_DISPLAY_DFR
#include "hisi_frame_rate_ctrl.h"
#endif
#include "hisi_fb_debug.h"
#include "hisi_hihdr.h"
/*******************************************************************************
 * handle isr
 */
static bool need_panel_mode_swtich(struct dpu_fb_data_type *dpufd, uint32_t isr_s2)
{

	if (dpufd->panel_mode_switch_isr_handler && dpufd->panel_info.panel_mode_swtich_support &&
		(dpufd->panel_info.mode_switch_to != dpufd->panel_info.current_mode)) {
		if (!(isr_s2 & BIT_LDI_UNFLOW))
			return true;
	}

	return false;
}

static bool need_dsi_bit_clk_upt(struct dpu_fb_data_type *dpufd)
{
#ifdef CONFIG_HISI_DISPLAY_DFR
	if (get_dfr_constraint(dpufd))
		return false;
#endif

	if (dpufd->mipi_dsi_bit_clk_upt_isr_handler && dpufd->panel_info.dsi_bit_clk_upt_support &&
		dpufd->panel_info.mipi.dsi_bit_clk_upt &&
		(dpufd->panel_info.mipi.dsi_bit_clk_upt != dpufd->panel_info.mipi.dsi_bit_clk))
		return true;

	return false;
}

static void dpufb_display_effect_flags_config(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->color_temperature_flag > 0)
		dpufd->color_temperature_flag--;

	if (dpufd->display_effect_flag > 0)
		dpufd->display_effect_flag--;

	if (dpufd->panel_info.xcc_set_in_isr_support && dpufd->xcc_coef_set == 1) {
		dpe_set_xcc_csc_value(dpufd);
		dpufd->xcc_coef_set = 0;
	}
}

static bool need_fps_upt(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->panel_info.fps_updt_support && dpufd->fps_upt_isr_handler &&
		(dpufd->panel_info.fps_updt != dpufd->panel_info.fps ||
		dpufd->panel_info.fps_updt_force_update) && !is_lcd_dfr_support(dpufd))
		return true;

	return false;
}

static bool need_config_frm_rate_para(struct dpu_fb_data_type *dpufd)
{
#ifdef CONFIG_HISI_DISPLAY_DFR
	return need_config_frame_rate_timing(dpufd);
#else
	void_unused(dpufd);
	return false;
#endif
}

static void config_frm_rate_para(struct dpu_fb_data_type *dpufd)
{
#ifdef CONFIG_HISI_DISPLAY_DFR
	(void)mipi_dsi_frm_rate_para_set_reg(dpufd);
#else
	void_unused(dpufd);
#endif
}

static void dss_pdp_isr_vactive0_end_handle(struct dpu_fb_data_type *dpufd,
	uint32_t isr_s2, struct dpufb_secure *secure_ctrl)
{
	dpufd->vactive0_end_flag = 1;

	dpufb_display_effect_flags_config(dpufd);

	if (dpufd->underflow_flag) {
		DPU_FB_ISR_INFO("mipi_dsi_reset_underflow_clear +\n");
		mipi_dsi_reset_underflow_clear(dpufd);

	} else {
		if (need_config_frm_rate_para(dpufd)) {
			config_frm_rate_para(dpufd);
		} else if (dpufd->pipe_clk_ctrl.pipe_clk_updt_state == PARA_UPDT_DOING) {
			;
		} else if (need_panel_mode_swtich(dpufd, isr_s2)) {
			if (((uint32_t)inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CTRL) & 0x1) == 0)
				dpufd->panel_mode_switch_isr_handler(dpufd, dpufd->panel_info.mode_switch_to);

			dpufd->panel_info.mode_switch_state = PARA_UPDT_END;
		} else if (need_dsi_bit_clk_upt(dpufd)) {
			if (!(isr_s2 & BIT_VACTIVE0_START))
				dpufd->mipi_dsi_bit_clk_upt_isr_handler(dpufd);
		} else if (need_fps_upt(dpufd)) {
			dpufd->fps_upt_isr_handler(dpufd);
		}
	}
	if (secure_ctrl->notify_secure_switch != NULL)
		secure_ctrl->notify_secure_switch(dpufd);

	if ((g_debug_ldi_underflow == 0) && (g_err_status & DSS_PDP_LDI_UNDERFLOW))
		g_err_status &= ~DSS_PDP_LDI_UNDERFLOW;
}

static void dss_pdp_isr_vactive0_start_handle(struct dpu_fb_data_type *dpufd, uint32_t isr_s2)
{
	if (dpufd->underflow_flag)
		DPU_FB_ISR_INFO("vactive end int of the underflow frame didnot come\n");

	if (dpufd->ov_vactive0_start_isr_handler != NULL)
		dpufd->ov_vactive0_start_isr_handler(dpufd);



#ifdef CONFIG_HISI_DISPLAY_DFR
	dfr_status_convert_on_isr_vstart(dpufd);
	if (need_update_frame_rate(dpufd))
		mipi_dsi_frm_rate_ctrl(dpufd, dpufd->panel_info.fps_updt);
#endif

	if ((dpufd->pipe_clk_ctrl.pipe_clk_updt_state == PARA_UPDT_NEED) ||
		(dpufd->pipe_clk_ctrl.pipe_clk_updt_state == PARA_UPDT_DOING)) {
		disable_ldi(dpufd);
		dpufd->pipe_clk_ctrl.pipe_clk_updt_state = PARA_UPDT_DOING;

		if (dpufd->pipe_clk_updt_isr_handler != NULL)
			dpufd->pipe_clk_updt_isr_handler(dpufd);
	}

	if (need_panel_mode_swtich(dpufd, isr_s2)) {
		disable_ldi(dpufd);
		dpufd->panel_info.mode_switch_state = PARA_UPDT_DOING;
	}
}

static void dss_debug_underflow_and_clear_base(struct dpu_fb_data_type *dpufd)
{
	if (g_debug_ldi_underflow_clear) {
		if (is_mipi_cmd_panel(dpufd)) {
			if (g_ldi_data_gate_en == 0) {
				if (dpufd->ldi_underflow_wq) {
					disable_ldi(dpufd);
					queue_work(dpufd->ldi_underflow_wq, &dpufd->ldi_underflow_work);
				}
			}
		} else {
			if (dpufd->ldi_underflow_wq) {
				disable_ldi(dpufd);
				queue_work(dpufd->ldi_underflow_wq, &dpufd->ldi_underflow_work);

#if defined(CONFIG_HUAWEI_DSM)
				if (dpufd->dss_underflow_debug_workqueue)
					queue_work(dpufd->dss_underflow_debug_workqueue,
						&dpufd->dss_underflow_debug_work);
#endif
			}
		}
	}

	if (g_debug_ldi_underflow) {
		if (g_debug_ovl_online_composer) {
			if (dpufd->dss_debug_wq)
				queue_work(dpufd->dss_debug_wq, &dpufd->dss_debug_work);
		}
	}
}

static void dss_te_vsync_isr_handle_base(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->vsync_isr_handler)
		dpufd->vsync_isr_handler(dpufd);

	if (dpufd->buf_sync_signal)
		dpufd->buf_sync_signal(dpufd);
}

#ifdef SUPPORT_DSI_VER_2_0
static void start_dss_clearing_work(struct dpu_fb_data_type *dpufd)
{
	if (g_debug_ldi_underflow_clear == 0)
		return;

	if (is_mipi_cmd_panel(dpufd) && g_ldi_data_gate_en)
		return;

	dpufd->underflow_flag = 1;
	disable_ldi(dpufd);
	hisi_dump_current_info(dpufd);

	if (dpufd->ldi_underflow_wq)
		queue_work(dpufd->ldi_underflow_wq, &dpufd->ldi_underflow_work);
}

static void dss_isr_underflow_handle(struct dpu_fb_data_type *dpufd, int dsi_idx)
{
	static uint32_t frame_no[2] = {0};  /* dsi[0,1] */

	if (!spin_trylock(&dpufd->underflow_lock)) {
		DPU_FB_ISR_INFO("dsi%d get underflow_lock fail\n", dsi_idx);
		return;
	}
	frame_no[dsi_idx] = dpufd->ov_req.frame_no;
	if (frame_no[0] != 0 && frame_no[0] == frame_no[1]) {
		DPU_FB_ISR_INFO("same frame, dsi%d return\n", dsi_idx);
		spin_unlock(&dpufd->underflow_lock);
		return;
	}
	spin_unlock(&dpufd->underflow_lock);

	DPU_FB_ISR_INFO("dsi%d underflow handle +\n", dsi_idx);
	start_dss_clearing_work(dpufd);

	if (g_debug_ldi_underflow && g_debug_ovl_online_composer && dpufd->dss_debug_wq)
		queue_work(dpufd->dss_debug_wq, &dpufd->dss_debug_work);

	g_err_status |= DSS_PDP_LDI_UNDERFLOW;

#if defined(CONFIG_HUAWEI_DSM)
	if ((!g_fake_lcd_flag) && (dpufd->ldi_data_gate_en == 0) && dpufd->dss_underflow_debug_workqueue)
		queue_work(dpufd->dss_underflow_debug_workqueue, &dpufd->dss_underflow_debug_work);
#endif
}

static void dss_dsi0_isr_for_te_vactive0_start(struct dpu_fb_data_type *dpufd, uint32_t isr_s2)
{
	uint32_t isr_te_vsync;

	if (is_mipi_cmd_panel(dpufd))
		isr_te_vsync = BIT_LCD_TE0_PIN;
	else
		isr_te_vsync = BIT_VSYNC;

	if (hisi_dss_is_async_play(dpufd)) {
		if (isr_s2 & isr_te_vsync) {
			dpufd->te_timestamp = ktime_get();
			dss_te_vsync_isr_handle_base(dpufd);
		}

		if (isr_s2 & BIT_VACTIVE0_START) {
			dss_pdp_isr_vactive0_start_handle(dpufd, isr_s2);
		}
	} else {
		if (isr_s2 & BIT_VACTIVE0_START) {
			dss_pdp_isr_vactive0_start_handle(dpufd, isr_s2);
		}

		if (isr_s2 & isr_te_vsync) {
			dpufd->te_timestamp = ktime_get();
			dss_te_vsync_isr_handle_base(dpufd);
		}
	}
}

irqreturn_t dss_dsi0_isr(int irq, void *ptr)
{
	struct dpu_fb_data_type *dpufd = NULL;
	uint32_t isr_s2;
	uint32_t mask;
	struct dpufb_secure *secure_ctrl = NULL;
	char __iomem *mipi_dsi_base = NULL;

	dpufd = (struct dpu_fb_data_type *)ptr;
	if (!dpufd) {
		DPU_FB_ISR_INFO("dpufd is NULL\n");
		return IRQ_NONE;
	}

	secure_ctrl = &(dpufd->secure_ctrl);

	mipi_dsi_base = get_mipi_dsi_base(dpufd);
	isr_s2 = inp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INTS);
	outp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INTS, isr_s2);
	isr_s2 &= ~((uint32_t)inp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INT_MSK));

	if (isr_s2 & BIT_VACTIVE0_END)
		dss_pdp_isr_vactive0_end_handle(dpufd, isr_s2, secure_ctrl);

	dss_dsi0_isr_for_te_vactive0_start(dpufd, isr_s2);


	if (isr_s2 & BIT_LDI_UNFLOW) {
		mask = inp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INT_MSK);
		mask |= BIT_LDI_UNFLOW;
		outp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);
		if (is_dual_mipi_panel(dpufd))
			outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);

		DPU_FB_ISR_INFO("dsi0 underflow frame_no %d, isr 0x%x, dpp_dbg 0x%x, online_fill_level 0x%x\n",
			dpufd->ov_req.frame_no, isr_s2,
			inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_DBG_CNT),
			inp32(dpufd->dss_base + DSS_DBUF0_OFFSET + DBUF_ONLINE_FILL_LEVEL));

		dss_isr_underflow_handle(dpufd, 0);
	}

	return IRQ_HANDLED;
}

irqreturn_t dss_dsi1_isr(int irq, void *ptr)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_secure *secure_ctrl = NULL;
	uint32_t isr_s2;
	uint32_t mask;
	uint32_t isr_te_vsync = BIT_VSYNC;

	dpufd = (struct dpu_fb_data_type *)ptr;
	dpu_check_and_return(!dpufd, IRQ_NONE, ISR_INFO, "dpufd is NULL!\n");

	isr_s2 = inp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS);

	outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS, isr_s2);

	isr_s2 &= ~((uint32_t)inp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK));


	DPU_FB_DEBUG("isr_s2 0x%x, mask 0x%x", isr_s2,
		inp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK));


	if ((dpufd->index == PRIMARY_PANEL_IDX) && is_dual_mipi_panel(dpufd)) {
		if (isr_s2 & BIT_LDI_UNFLOW) {
			mask = inp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK);

			mask |= BIT_LDI_UNFLOW;
			outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);

			DPU_FB_ISR_INFO("dsi1 underflow frame_no %d, isr 0x%x, dpp_dbg 0x%x, online_fill_level 0x%x\n",
				dpufd->ov_req.frame_no, isr_s2,
				inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_DBG_CNT),
				inp32(dpufd->dss_base + DSS_DBUF0_OFFSET + DBUF_ONLINE_FILL_LEVEL));
			dss_isr_underflow_handle(dpufd, 1);
		}
		return IRQ_HANDLED;
	}

	secure_ctrl = &(dpufd->secure_ctrl);
	if (is_mipi_cmd_panel(dpufd))
		isr_te_vsync = BIT_LCD_TE0_PIN;

	if (dpufd->panel_info.mipi.dsi_te_type == DSI1_TE1_TYPE)
		isr_te_vsync = BIT_LCD_TE1_PIN;

	if (isr_s2 & BIT_VACTIVE0_END)
		dss_pdp_isr_vactive0_end_handle(dpufd, isr_s2, secure_ctrl);

	if (isr_s2 & BIT_VACTIVE0_START)
		dss_pdp_isr_vactive0_start_handle(dpufd, isr_s2);

	if (isr_s2 & isr_te_vsync)
		dss_te_vsync_isr_handle_base(dpufd);

	if (isr_s2 & BIT_LDI_UNFLOW) {
		mask = inp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK);
		mask |= BIT_LDI_UNFLOW;
		outp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);

		dss_debug_underflow_and_clear_base(dpufd);
		g_err_status |= DSS_SDP_LDI_UNDERFLOW;


		DPU_FB_ISR_INFO("ldi1 underflow! frame_no = %d!\n", dpufd->ov_req.frame_no);
	}

	return IRQ_HANDLED;
}

irqreturn_t dss_sdp_isr_mipi_panel(int irq, void *ptr)
{
	struct dpu_fb_data_type *dpufd = NULL;
	uint32_t isr_s1;
	uint32_t isr_s2_smmu;

	dpufd = (struct dpu_fb_data_type *)ptr;
	if (!dpufd) {
		DPU_FB_ISR_INFO("dpufd is NULL\n");
		return IRQ_NONE;
	}

	isr_s1 = inp32(dpufd->dss_base + GLB_CPU_SDP_INTS);

	isr_s2_smmu = inp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTSTAT_NS);

	outp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTCLR_NS, isr_s2_smmu);
	outp32(dpufd->dss_base + GLB_CPU_SDP_INTS, isr_s1);

	isr_s1 &= ~((uint32_t)inp32(dpufd->dss_base + GLB_CPU_SDP_INT_MSK));

	return IRQ_HANDLED;
}

irqreturn_t dss_sdp_isr_dp(int irq, void *ptr)
{
	char __iomem *ldi_base = NULL;
	char __iomem *ldi_base1 = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	uint32_t isr_s1;
	uint32_t isr_s2;
	uint32_t isr_s3;
	uint32_t isr_s2_smmu;
	uint32_t mask;
	uint32_t temp;

	dpufd = (struct dpu_fb_data_type *)ptr;
	dpu_check_and_return(!dpufd, IRQ_NONE, ISR_INFO, "dpufd is NULL!\n");

	ldi_base = dpufd->dss_base + DSS_LDI_DP_OFFSET;
	ldi_base1 = dpufd->dss_base + DSS_LDI_DP1_OFFSET;

	isr_s1 = inp32(dpufd->dss_base + GLB_CPU_DP_INTS);
	isr_s2 = inp32(ldi_base + LDI_CPU_ITF_INTS);
	isr_s3 = inp32(ldi_base1 + LDI_CPU_ITF_INTS);
	isr_s2_smmu = inp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTSTAT_NS);

	outp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTCLR_NS, isr_s2_smmu);
	outp32(ldi_base + LDI_CPU_ITF_INTS, isr_s2);
	outp32(ldi_base1 + LDI_CPU_ITF_INTS, isr_s3);
	outp32(dpufd->dss_base + GLB_CPU_DP_INTS, isr_s1);

	isr_s1 &= ~((uint32_t)inp32(dpufd->dss_base + GLB_CPU_DP_INT_MSK));
	isr_s2 &= ~((uint32_t)inp32(ldi_base + LDI_CPU_ITF_INT_MSK));

	if (isr_s2 & BIT_VACTIVE0_END)
		dpufd->vactive0_end_flag = 1;

	if (isr_s2 & BIT_VACTIVE0_START) {
		if (dpufd->ov_vactive0_start_isr_handler)
			dpufd->ov_vactive0_start_isr_handler(dpufd);
	}

	if (isr_s2 & BIT_VSYNC)
		dss_te_vsync_isr_handle_base(dpufd);

	if (isr_s2 & BIT_LDI_UNFLOW) {
		mask = inp32(ldi_base + LDI_CPU_ITF_INT_MSK);
		mask |= BIT_LDI_UNFLOW;
		outp32(ldi_base + LDI_CPU_ITF_INT_MSK, mask);

		dss_debug_underflow_and_clear_base(dpufd);

		g_err_status |= DSS_SDP_LDI_UNDERFLOW;
		temp = inp32(dpufd->dss_base + DSS_DPP1_OFFSET + DPP_DBG_CNT);

		DPU_FB_ISR_INFO("ldi1 underflow! frame_no = %d, dpp_dbg = 0x%x\n", dpufd->ov_req.frame_no, temp);
		hisi_dump_current_info(dpufd);
	}

	return IRQ_HANDLED;
}

irqreturn_t dss_pdp_isr(int irq, void *ptr)
{
	struct dpu_fb_data_type *dpufd = NULL;
	uint32_t isr_s1;
	uint32_t isr_s2_dpp;
	uint32_t isr_s2_smmu;
	uint32_t isr_s2_wb;
	uint32_t mask;


	dpufd = (struct dpu_fb_data_type *)ptr;
	dpu_check_and_return(!dpufd, IRQ_NONE, ISR_INFO, "dpufd is NULL!\n");


	isr_s1 = inp32(dpufd->dss_base + GLB_CPU_PDP_INTS);
	isr_s2_dpp = inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + DPP_INTS);

	isr_s2_smmu = inp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTSTAT_NS);
	isr_s2_wb = inp32(dpufd->dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS);

	outp32(dpufd->dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS, isr_s2_wb);
	outp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTCLR_NS, isr_s2_smmu);
	outp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + DPP_INTS, isr_s2_dpp);

	outp32(dpufd->dss_base + GLB_CPU_PDP_INTS, isr_s1);

	isr_s1 &= ~((uint32_t)inp32(dpufd->dss_base + GLB_CPU_PDP_INT_MSK));
	isr_s2_dpp &= ~((uint32_t)inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + DPP_INT_MSK));

	if (isr_s1 & BIT_DSS_WB_ERR_INTS) {
		mask = inp32(dpufd->dss_base + GLB_CPU_PDP_INT_MSK);
		mask |= BIT_DSS_WB_ERR_INTS;
		outp32(dpufd->dss_base + GLB_CPU_PDP_INT_MSK, mask);
		DPU_FB_ISR_INFO("isr_s1 = 0x%x, isr_s2_wb = 0x%x\n", isr_s1, isr_s2_wb);
	}
#if defined(CONFIG_EFFECT_HIACE)
	if (isr_s2_dpp & BIT_HIACE_IND) {
		if (dpufd->panel_info.hiace_support && dpufd->hiace_end_wq)
			hisi_effect_hiace_trigger_wq(dpufd);
	}
#endif
	return IRQ_HANDLED;
}

#else
static void dss_te_vsync_isr_handle(struct dpu_fb_data_type *dpufd)
{
	uint32_t temp;
	struct timeval tv;
	uint32_t isr_s2 = 0;

	dpufd->te_timestamp = ktime_get();

	if (dpufd->panel_info.delayed_cmd_queue_support && dpufd->delayed_cmd_queue_wq &&
		mipi_dsi_check_delayed_cmd_queue_working()) {
		mipi_dsi_set_timestamp();
		queue_work(dpufd->delayed_cmd_queue_wq, &dpufd->delayed_cmd_queue_work);
	}

	dss_te_vsync_isr_handle_base(dpufd);

	if (g_err_status & (DSS_PDP_LDI_UNDERFLOW | DSS_PDP_SMMU_ERR | DSS_SDP_SMMU_ERR)) {
		temp = inp32(dpufd->dss_base + DSS_DPP_OFFSET + DPP_DBG_CNT);
		g_err_status &= ~DSS_PDP_LDI_UNDERFLOW;
		DPU_FB_ISR_INFO("isr_te_vsync:frame_no = %d,dpp_dbg = 0x%x!\n", dpufd->ov_req.frame_no, temp);
	}

	if (g_debug_ldi_underflow) {
		dpufb_get_timestamp(&tv);
		DPU_FB_ISR_INFO("isr_te_vsync:frame_no = %d,isr_s2 = 0x%x\n", dpufd->ov_req.frame_no, isr_s2);
	}
}

static void dss_debug_underflow_and_clear(struct dpu_fb_data_type *dpufd)
{
	if (g_debug_ldi_underflow_clear) {
		if (is_mipi_cmd_panel(dpufd)) {
			if (g_ldi_data_gate_en == 0) {
				if (dpufd->ldi_underflow_wq != NULL) {
					dpufb_pipe_clk_set_underflow_flag(dpufd, true);
					disable_ldi(dpufd);
					queue_work(dpufd->ldi_underflow_wq, &dpufd->ldi_underflow_work);
				}
			}
		} else {
			if (dpufd->ldi_underflow_wq != NULL) {
				dpufb_pipe_clk_set_underflow_flag(dpufd, true);
				disable_ldi(dpufd);
				queue_work(dpufd->ldi_underflow_wq, &dpufd->ldi_underflow_work);
			}
		}
	}

	if (g_debug_ldi_underflow) {
		if (g_debug_ovl_online_composer) {
			if (dpufd->dss_debug_wq != NULL)
				queue_work(dpufd->dss_debug_wq, &dpufd->dss_debug_work);
		}
	}
}

#if defined(CONFIG_EFFECT_HIACE)
static void dss_hiace_ind_handle(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->panel_info.hiace_support && dpufd->hiace_end_wq) {
		/* check if reading fna in isr first, if yes, no need queue work */
		hisi_display_effect_hiace_trigger_wq(dpufd, dpufd->is_idle_display);
	}
}

static void dss_display_effect_handle(struct dpu_fb_data_type *dpufd, uint32_t isr_s2, uint32_t isr_s2_dpp)
{
	if (isr_s2_dpp & BIT_HIACE_IND)
		dss_hiace_ind_handle(dpufd);
}
#endif

static void dss_pdp_te_vsync_isr_handle(struct dpu_fb_data_type *dpufd, uint32_t isr_s2)
{
	uint32_t isr_te_vsync;

	if (is_mipi_cmd_panel(dpufd))
		isr_te_vsync = BIT_LCD_TE0_PIN;
	else
		isr_te_vsync = BIT_VSYNC;

	if (isr_s2 & isr_te_vsync)
		dss_te_vsync_isr_handle(dpufd);
}

irqreturn_t dss_pdp_isr(int irq, void *ptr)
{
	struct dpu_fb_data_type *dpufd = NULL;
	uint32_t isr_s1;
	uint32_t isr_s2;
	uint32_t isr_s2_dpp;
	uint32_t isr_s2_smmu;
	uint32_t mask;
	uint32_t temp;

	dpufd = (struct dpu_fb_data_type *)ptr;
	dpu_check_and_return(!dpufd, IRQ_NONE, ISR_INFO, "dpufd is NULL!\n");

	isr_s1 = inp32(dpufd->dss_base + GLB_CPU_PDP_INTS);
	isr_s2 = inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INTS);
	isr_s2_dpp = inp32(dpufd->dss_base + DSS_DPP_OFFSET + DPP_INTS);
	isr_s2_smmu = inp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTSTAT_NS);

	outp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTCLR_NS, isr_s2_smmu);
	outp32(dpufd->dss_base + DSS_DPP_OFFSET + DPP_INTS, isr_s2_dpp);
	outp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INTS, isr_s2);
	outp32(dpufd->dss_base + GLB_CPU_PDP_INTS, isr_s1);

	isr_s1 &= ~((uint32_t)inp32(dpufd->dss_base + GLB_CPU_PDP_INT_MSK));
	isr_s2 &= ~((uint32_t)inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INT_MSK));
	isr_s2_dpp &= ~((uint32_t)inp32(dpufd->dss_base + DSS_DPP_OFFSET + DPP_INT_MSK));

	if (isr_s2 & BIT_VACTIVE0_END)
		dss_pdp_isr_vactive0_end_handle(dpufd, isr_s2, &(dpufd->secure_ctrl));

	if (isr_s2 & BIT_VACTIVE0_START)
		dss_pdp_isr_vactive0_start_handle(dpufd, isr_s2);

	dss_pdp_te_vsync_isr_handle(dpufd, isr_s2);

	if (isr_s2 & BIT_LDI_UNFLOW) {
		mask = inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INT_MSK);
		mask |= BIT_LDI_UNFLOW;
		outp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INT_MSK, mask);

		hisi_dump_current_info(dpufd);

		dss_debug_underflow_and_clear(dpufd);

		g_err_status |= DSS_PDP_LDI_UNDERFLOW;

		if (dpufd->ldi_data_gate_en == 0) {
			temp = inp32(dpufd->dss_base + DSS_DPP_OFFSET + DPP_DBG_CNT);
			DPU_FB_ISR_INFO("ldi underflow! frame_no = %d,dpp_dbg = 0x%x, vactive0_start_flag = %d!\n",
				dpufd->ov_req.frame_no, temp, dpufd->vactive0_start_flag);

#if defined(CONFIG_HUAWEI_DSM)
			if (dpufd->dss_underflow_debug_workqueue && !g_fake_lcd_flag)
				queue_work(dpufd->dss_underflow_debug_workqueue, &dpufd->dss_underflow_debug_work);
#endif
		}
	}
#if defined(CONFIG_EFFECT_HIACE)
	dss_display_effect_handle(dpufd, isr_s2, isr_s2_dpp);
#endif
	return IRQ_HANDLED;
}
#endif

static void dss_sdp_handle_isr_s2(struct dpu_fb_data_type *dpufd, char __iomem *ldi_base, uint32_t isr_s2)
{
	uint32_t mask;

	if (isr_s2 & BIT_VACTIVE0_END)
		dpufd->vactive0_end_flag = 1;

	if (isr_s2 & BIT_VACTIVE0_START) {
		if (dpufd->ov_vactive0_start_isr_handler != NULL)
			dpufd->ov_vactive0_start_isr_handler(dpufd);
	}

	if (isr_s2 & BIT_VSYNC)
		dss_te_vsync_isr_handle_base(dpufd);

	if (isr_s2 & BIT_LDI_UNFLOW) {
		mask = inp32(ldi_base + LDI_CPU_ITF_INT_MSK);
		mask |= BIT_LDI_UNFLOW;
		outp32(ldi_base + LDI_CPU_ITF_INT_MSK, mask);

		dss_debug_underflow_and_clear_base(dpufd);

		g_err_status |= DSS_SDP_LDI_UNDERFLOW;

		DPU_FB_ISR_INFO("ldi1 underflow! frame_no = %d!\n", dpufd->ov_req.frame_no);
	}
}

irqreturn_t dss_sdp_isr(int irq, void *ptr)
{
	char __iomem *ldi_base = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	uint32_t isr_s1;
	uint32_t isr_s2;
	uint32_t isr_s2_smmu;

	dpufd = (struct dpu_fb_data_type *)ptr;

	dpu_check_and_return(!dpufd, IRQ_NONE, ISR_INFO, "dpufd is NULL\n");

	ldi_base = dpufd->dss_base + DSS_LDI1_OFFSET;

	isr_s1 = inp32(dpufd->dss_base + GLB_CPU_SDP_INTS);
	isr_s2 = inp32(ldi_base + LDI_CPU_ITF_INTS);
	isr_s2_smmu = inp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTSTAT_NS);

	outp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTCLR_NS, isr_s2_smmu);
	outp32(ldi_base + LDI_CPU_ITF_INTS, isr_s2);
	outp32(dpufd->dss_base + GLB_CPU_SDP_INTS, isr_s1);

	isr_s1 &= ~((uint32_t)inp32(dpufd->dss_base + GLB_CPU_SDP_INT_MSK));
	isr_s2 &= ~((uint32_t)inp32(ldi_base + LDI_CPU_ITF_INT_MSK));

	dss_sdp_handle_isr_s2(dpufd, ldi_base, isr_s2);

	return IRQ_HANDLED;
}

static void adp_isr_wch_ints(struct dpu_fb_data_type *dpufd, uint32_t isr_s1, uint32_t isr_s3_copybit)
{
	if (isr_s1 & BIT_OFF_WCH0_INTS) {
		if (dpufd->cmdlist_info->cmdlist_wb_flag[WB_TYPE_WCH0] == 1) {
			dpufd->cmdlist_info->cmdlist_wb_done[WB_TYPE_WCH0] = 1;
			wake_up_interruptible_all(&(dpufd->cmdlist_info->cmdlist_wb_wq[WB_TYPE_WCH0]));
		}
	}

	if (isr_s1 & BIT_OFF_WCH1_INTS) {
		if (dpufd->cmdlist_info->cmdlist_wb_flag[WB_TYPE_WCH1] == 1) {
			dpufd->cmdlist_info->cmdlist_wb_done[WB_TYPE_WCH1] = 1;
			wake_up_interruptible_all(&(dpufd->cmdlist_info->cmdlist_wb_wq[WB_TYPE_WCH1]));
		}
	}

	if (isr_s1 & BIT_OFF_WCH0_WCH1_FRM_END_INT) {
		if (dpufd->cmdlist_info->cmdlist_wb_flag[WB_TYPE_WCH0_WCH1] == 1) {
			dpufd->cmdlist_info->cmdlist_wb_done[WB_TYPE_WCH0_WCH1] = 1;
			wake_up_interruptible_all(&(dpufd->cmdlist_info->cmdlist_wb_wq[WB_TYPE_WCH0_WCH1]));
		}
	}

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
	if (isr_s3_copybit & BIT_OFF_CAM_WCH2_FRMEND_INTS) {
		if (dpufd->copybit_info->copybit_flag == 1) {
			dpufd->copybit_info->copybit_done = 1;
			wake_up_interruptible_all(&(dpufd->copybit_info->copybit_wq));
		}

		if (dpufd->cmdlist_info->cmdlist_wb_flag[WB_TYPE_WCH2] == 1) {
			dpufd->cmdlist_info->cmdlist_wb_done[WB_TYPE_WCH2] = 1;
			wake_up_interruptible_all(&(dpufd->cmdlist_info->cmdlist_wb_wq[WB_TYPE_WCH2]));
		}
	}
#else
	(void)isr_s3_copybit;
#endif
}

irqreturn_t dss_adp_isr(int irq, void *ptr)
{
	struct dpu_fb_data_type *dpufd = NULL;
	uint32_t isr_s1;
	uint32_t isr_s2_smmu;
	uint32_t isr_s3_copybit = 0;

	dpufd = (struct dpu_fb_data_type *)ptr;
	dpu_check_and_return(!dpufd, IRQ_NONE, ISR_INFO, "dpufd is NULL\n");

	isr_s1 = inp32(dpufd->dss_base + GLB_CPU_OFF_INTS);
	isr_s2_smmu = inp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTSTAT_NS);

	outp32(dpufd->dss_base + DSS_SMMU_OFFSET + SMMU_INTCLR_NS, isr_s2_smmu);
	outp32(dpufd->dss_base + GLB_CPU_OFF_INTS, isr_s1);

	isr_s1 &= ~((uint32_t)inp32(dpufd->dss_base + GLB_CPU_OFF_INT_MSK));
#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
	isr_s3_copybit = inp32(dpufd->dss_base + GLB_CPU_OFF_CAM_INTS);
	outp32(dpufd->dss_base + GLB_CPU_OFF_CAM_INTS, isr_s3_copybit);
	isr_s3_copybit &= ~((uint32_t)inp32(dpufd->dss_base + GLB_CPU_OFF_CAM_INT_MSK));
#endif


	adp_isr_wch_ints(dpufd, isr_s1, isr_s3_copybit);

	return IRQ_HANDLED;
}

irqreturn_t dss_mdc_isr(int irq, void *ptr)
{
	struct dpu_fb_data_type *dpufd = NULL;
	uint32_t isr_s1;

	dpufd = (struct dpu_fb_data_type *)ptr;
	if (!dpufd) {
		DPU_FB_ISR_INFO("dpufd is NULL\n");
		return IRQ_NONE;
	}

	isr_s1 = inp32(dpufd->media_common_base + GLB_CPU_OFF_INTS);
	outp32(dpufd->media_common_base + GLB_CPU_OFF_INTS, isr_s1);

	isr_s1 &= ~((uint32_t)inp32(dpufd->media_common_base + GLB_CPU_OFF_INT_MSK));
	if (isr_s1 & (BIT_OFF_WCH1_INTS | BIT_OFF_WCH0_INTS)) {
		if (dpufd->media_common_info->mdc_flag == 1) {
			dpufd->media_common_info->mdc_done = 1;
			wake_up_interruptible_all(&(dpufd->media_common_info->mdc_wq));
		}
	}

	return IRQ_HANDLED;
}

