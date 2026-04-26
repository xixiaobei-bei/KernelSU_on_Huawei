/* Copyright (c) 2018-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_dpe_pipe_clk_utils.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static int dpufb_change_pipe_clk_rate(struct dpu_fb_data_type *dpufd, uint64_t pipe_clk_rate)
{
	int ret = -1;

	if (dpufd->dss_pxl0_clk != NULL) {
		ret = clk_set_rate(dpufd->dss_pxl0_clk, pipe_clk_rate); /* ppll0 */
		if (ret < 0) {
			DPU_FB_ERR("set pipe_clk_rate[%llu] fail, reset to [%llu], ret[%d].\n",
				dpufd->pipe_clk_ctrl.pipe_clk_rate, dpufd->panel_info.pxl_clk_rate, ret);

			ret = clk_set_rate(dpufd->dss_pxl0_clk, dpufd->panel_info.pxl_clk_rate);
			if ((ret < 0) && (g_fpga_flag == 0)) {
				DPU_FB_ERR("fb%d dss_pxl0_clk clk_set_rate(%llu) failed, error=%d!\n",
				dpufd->index, dpufd->panel_info.pxl_clk_rate, ret);
				return -EINVAL;
			}
		}

		DPU_FB_INFO("dss_pxl0_clk:set[%llu], get[%llu].\n",
			pipe_clk_rate, (uint64_t)clk_get_rate(dpufd->dss_pxl0_clk));
	}

	return ret;
}

static int dpufb_pipe_clk_updt_config(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;
	uint32_t *hporch = NULL;
	char __iomem *ldi_base = NULL;
	uint32_t pxl0_divxcfg;
	uint32_t mipi_idx;
	int ret = 0;

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is nullptr!\n");

	pinfo = &(dpufd->panel_info);
	pipe_clk_ctrl = &(dpufd->pipe_clk_ctrl);
	dpu_check_and_return((pinfo == NULL), -EINVAL, ERR, "pinfo is nullptr!\n");

	hporch = pipe_clk_ctrl->pipe_clk_updt_hporch;

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return ret;

	ldi_base = dpufd->dss_base + DSS_LDI0_OFFSET;

	ret = dpufb_change_pipe_clk_rate(dpufd, pipe_clk_ctrl->pipe_clk_rate);
	dpu_check_and_return((ret < 0), ret, ERR, "fb%d, set clk_ldi0 to [%llu] failed, error=%d!\n",
		dpufd->index, pipe_clk_ctrl->pipe_clk_rate, ret);

	mipi_idx = is_dual_mipi_panel(dpufd) ? 1 : 0;
	pxl0_divxcfg = g_mipi_ifbc_division[mipi_idx][pinfo->ifbc_type].pxl0_divxcfg;
	pxl0_divxcfg = (pxl0_divxcfg + 1) * pipe_clk_ctrl->pipe_clk_rate_div;

	set_reg(ldi_base + LDI_PXL0_DIVXCFG, (pxl0_divxcfg - 1), 3, 0);
	if (pxl0_divxcfg > 1)
		set_reg(ldi_base + LDI_PXL0_DSI_GT_EN, 3, 2, 0);
	else
		set_reg(ldi_base + LDI_PXL0_DSI_GT_EN, 1, 2, 0);

	DPU_FB_INFO("step1. set pxl0_divxcfg[%d]\n", (pxl0_divxcfg - 1));

	if (is_mipi_video_panel(dpufd)) {
		/* pipe_clk_updt_hporch valve */
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0, hporch[1] | ((hporch[0] + DSS_WIDTH(hporch[2])) << 16));
	} else {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0, hporch[1] | (hporch[0] << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, DSS_WIDTH(hporch[2]));
	}

	if (is_dual_mipi_panel(dpufd)) {
		if (is_mipi_video_panel(dpufd)) {
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL0, (hporch[0] + DSS_WIDTH(hporch[2])) << 16);
		} else {
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL0, hporch[0] << 16);
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL1, DSS_WIDTH(hporch[2]));
		}
	}

	DPU_FB_INFO("step2. updt hporch: hbp[%d], hfp[%d], hpw[%d].\n", hporch[0], hporch[1], hporch[2]);

	return ret;
}

int dpufb_pipe_clk_updt_handler(struct dpu_fb_data_type *primary_dpufd, bool primary_panel_pwr_on)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;
	int ret = 0;

	if (primary_dpufd == NULL) {
		DPU_FB_ERR("primary_dpufd is null.\n");
		return -EINVAL;
	}

	if (primary_dpufd->index != PRIMARY_PANEL_IDX) {
		DPU_FB_INFO("fb%d not support.\n", primary_dpufd->index);
		return ret;
	}

	pinfo = &(primary_dpufd->panel_info);

	pipe_clk_ctrl = &(primary_dpufd->pipe_clk_ctrl);

	DPU_FB_DEBUG("+, primary_panel_pwr_on[%d].\n", primary_panel_pwr_on);

	if (primary_panel_pwr_on) {
		if (dpufd_list[EXTERNAL_PANEL_IDX] && dpufd_list[EXTERNAL_PANEL_IDX]->panel_power_on &&
			(pipe_clk_ctrl->pipe_clk_rate > pinfo->pxl_clk_rate))
			ret = dpufb_pipe_clk_updt_config(primary_dpufd);
	} else {
		/* only for pipe_clk_updt in vactive_end */
		if (pipe_clk_ctrl->pipe_clk_rate >= pinfo->pxl_clk_rate)
			ret = dpufb_pipe_clk_updt_config(primary_dpufd);
	}

	DPU_FB_DEBUG("-.\n");
	return ret;
}

static void calc_hporch_paras(struct dpu_panel_info *pinfo, struct dpufb_pipe_clk *pipe_clk_ctrl,
		struct hporch_value hp_value, struct hporch_new_value *hp_new_val)
{
	uint64_t pxl_clk_rate_new;
	uint32_t pxl_clk_ratio = 1;
	uint32_t precision = 100;  /* with 2 decimal places */
	uint32_t millisecond = 1000; /* us */
	uint32_t h_size;
	uint32_t v_size;
	uint32_t fps = 0;
	uint32_t t_frame = 0;
	uint32_t t_hline = 0;
	uint32_t t_hbphpw = 0;

	/* pipe_clk_rate_div calc from get_para_for_pipe_clk_updt(), not 0 */
	pxl_clk_rate_new = pipe_clk_ctrl->pipe_clk_rate / pipe_clk_ctrl->pipe_clk_rate_div;

	if (pxl_clk_rate_new > pinfo->pxl_clk_rate) {
		if (pinfo->pxl_clk_rate_div > 1) {
			hp_value.hbp *= pinfo->pxl_clk_rate_div;
			hp_value.hfp *= pinfo->pxl_clk_rate_div;
			hp_value.hpw *= pinfo->pxl_clk_rate_div;
		}

		/* hporch paras calc */
		/* with 2 decimal places */
		pxl_clk_ratio = precision * (pxl_clk_rate_new / 100) / (pinfo->pxl_clk_rate / 100);
		hp_new_val->hbp_new = hp_value.hbp * pxl_clk_ratio;  /* with 2 decimal places */
		hp_new_val->hpw_new = hp_value.hpw * pxl_clk_ratio;  /* with 2 decimal places */

		/* h_size, v_size from pinfo, lcd init will set pinfo value, not 0 */
		h_size = pinfo->xres + hp_value.hbp + hp_value.hfp + hp_value.hpw;
		v_size = pinfo->yres + pinfo->ldi.v_back_porch + pinfo->ldi.v_front_porch + pinfo->ldi.v_pulse_width;
		fps = pinfo->pxl_clk_rate / h_size * precision / v_size;  /* us with 2 decimal places */
		t_frame = millisecond * millisecond * precision / fps; /* us */
		t_hline = precision * t_frame / v_size;  /* us with 2 decimal places */
		t_hbphpw = t_hline * (hp_value.hbp + hp_value.hpw) / h_size;  /* us with 2 decimal places */
		/* with 2 decimal places */
		hp_new_val->hfp_new = (hp_new_val->hbp_new + hp_new_val->hpw_new) * t_hline / t_hbphpw -
			(pinfo->xres * precision + hp_new_val->hbp_new + hp_new_val->hpw_new);
		hp_new_val->hbp_new = hp_new_val->hbp_new / precision + (hp_new_val->hbp_new % precision > 0 ? 1 : 0);
		hp_new_val->hfp_new = hp_new_val->hfp_new / precision + (hp_new_val->hfp_new % precision > 0 ? 1 : 0);
		hp_new_val->hpw_new = hp_new_val->hpw_new / precision + (hp_new_val->hpw_new % precision > 0 ? 1 : 0);

		if (pinfo->pxl_clk_rate_div > 1) {
			hp_new_val->hbp_new /= pinfo->pxl_clk_rate_div;
			hp_new_val->hfp_new /= pinfo->pxl_clk_rate_div;
			hp_new_val->hpw_new /= pinfo->pxl_clk_rate_div;
		}
	} else {
		hp_new_val->hbp_new = hp_value.hbp;
		hp_new_val->hfp_new = hp_value.hfp;
		hp_new_val->hpw_new = hp_value.hpw;
	}

	DPU_FB_DEBUG("hbp[%d], hfp[%d], hpw[%d], pxl_clk_ratio[%d],\n"
		"fps[%d], t_frame[%d], t_hline[%d], t_hbphpw[%d]\n"
		"hbp_new[%d], hfp_new[%d], hpw_new[%d].\n",
		hp_value.hbp, hp_value.hfp, hp_value.hpw, pxl_clk_ratio, fps,
		t_frame, t_hline, t_hbphpw, hp_new_val->hbp_new, hp_new_val->hfp_new, hp_new_val->hpw_new);
}

int dpufb_get_ldi_hporch_updt_para(struct dpu_fb_data_type *dpufd, bool fps_updt_use)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;
	struct hporch_value hp_value;
	struct hporch_new_value hp_new_val;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	pinfo = &(dpufd->panel_info);

	pipe_clk_ctrl = &(dpufd->pipe_clk_ctrl);

	DPU_FB_DEBUG("+ ,fps_updt_use[%d].\n", fps_updt_use);

	if (fps_updt_use) {
		hp_value.hbp = pinfo->ldi_updt.h_back_porch;
		hp_value.hfp = pinfo->ldi_updt.h_front_porch;
		hp_value.hpw = pinfo->ldi_updt.h_pulse_width;
	} else {
		hp_value.hbp = pinfo->ldi.h_back_porch;
		hp_value.hfp = pinfo->ldi.h_front_porch;
		hp_value.hpw = pinfo->ldi.h_pulse_width;
	}

	calc_hporch_paras(pinfo, pipe_clk_ctrl, hp_value, &hp_new_val);

	if (fps_updt_use) {
		pipe_clk_ctrl->fps_updt_hporch[0] = hp_new_val.hbp_new;
		pipe_clk_ctrl->fps_updt_hporch[1] = hp_new_val.hfp_new;
		pipe_clk_ctrl->fps_updt_hporch[2] = hp_new_val.hpw_new;
	} else {
		pipe_clk_ctrl->pipe_clk_updt_hporch[0] = hp_new_val.hbp_new;
		pipe_clk_ctrl->pipe_clk_updt_hporch[1] = hp_new_val.hfp_new;
		pipe_clk_ctrl->pipe_clk_updt_hporch[2] = hp_new_val.hpw_new;
	}

	return 0;
}

static uint64_t calc_pipe_clk_rate_by_ppll(uint64_t ppll_clk_rate, uint64_t dp_pxl_clk_rate)
{
	uint64_t pipe_clk_rate_ppll = 0;
	uint32_t div = 0;

	if (ppll_clk_rate < dp_pxl_clk_rate) {
		pipe_clk_rate_ppll = 0;
	} else if (ppll_clk_rate == dp_pxl_clk_rate) {
		pipe_clk_rate_ppll = ppll_clk_rate;
	} else {
		for (div = 1; div < 20; div++) {  /* clk division from 1 to 19 */
			if ((ppll_clk_rate / div) < dp_pxl_clk_rate)
				break;
		}
		div = div - 1;
		if (div > 0)
			pipe_clk_rate_ppll = (ppll_clk_rate % div) ? (ppll_clk_rate / div + 1) : (ppll_clk_rate / div);
		else
			DPU_FB_ERR("div = %u.\n", div);
	}

	DPU_FB_DEBUG("ppll_clk_rate[%llu], div[%u], pipe_clk_rate_ppll[%llu].\n",
		ppll_clk_rate, div, pipe_clk_rate_ppll);
	return pipe_clk_rate_ppll;
}

static int get_pxl_clk_div_by_ppll(uint64_t pxl_clk_rate, uint64_t ppll_clk_rate, int *out_div)
{
	uint32_t div = 1;
	int ret = 0;

	if (ppll_clk_rate != CRGPERI_PLL0_CLK_RATE
		&& ppll_clk_rate != CRGPERI_PLL2_CLK_RATE
		&& ppll_clk_rate != CRGPERI_PLL3_CLK_RATE) {
		DPU_FB_ERR("wrong ppll_clk_rate[%llu], which must be configed in panel init.\n", ppll_clk_rate);
		return -1;
	}

	if (ppll_clk_rate < pxl_clk_rate) {
		ret = -1;
	} else if (ppll_clk_rate == pxl_clk_rate) {
		div = 1;
	} else {
		for (div = 1; div < 20; div++) {  /* clk division from 1 to 19 */
			if ((ppll_clk_rate / div) <= pxl_clk_rate)
				break;
		}
	}

	if (ret)
		DPU_FB_ERR("ret=%d, pxl_clk_rate[%llu], div[%d].\n", ret, pxl_clk_rate, div);

	*out_div = div;

	return ret;
}

static int get_preset_para_for_pipe_clk_updt(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;

	pinfo = &(dpufd->panel_info);
	pipe_clk_ctrl = &(dpufd->pipe_clk_ctrl);

	pipe_clk_ctrl->pipe_clk_rate = pinfo->ldi.pipe_clk_rate_pre_set * DEFAULT_PIPE_CLK_RATE;
	pipe_clk_ctrl->pipe_clk_rate_div = pinfo->ldi.div_pre_set;
	if (pinfo->ldi.hporch_pre_set[0] != 0) {
		pipe_clk_ctrl->pipe_clk_updt_hporch[0] = pinfo->ldi.hporch_pre_set[0];
		pipe_clk_ctrl->pipe_clk_updt_hporch[1] = pinfo->ldi.hporch_pre_set[1];
		pipe_clk_ctrl->pipe_clk_updt_hporch[2] = pinfo->ldi.hporch_pre_set[2];
	} else {
		pipe_clk_ctrl->pipe_clk_updt_hporch[0] = pinfo->ldi.h_back_porch;
		pipe_clk_ctrl->pipe_clk_updt_hporch[1] = pinfo->ldi.h_front_porch;
		pipe_clk_ctrl->pipe_clk_updt_hporch[2] = pinfo->ldi.h_pulse_width;
	}

	DPU_FB_INFO("self-adaption not support, use pre-set value\n");

	return 0;
}

static int get_para_for_pipe_clk_updt(struct dpu_fb_data_type *dpufd, uint64_t dp_pxl_clk_rate)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;
	uint64_t pipe_clk_rate;
	uint64_t pipe_clk_rate_ppll[3];  /* PLL0/PLL1/PLL2 clk rate */
	uint32_t pipe_clk_rate_div = 1;
	int count;
	int ret;
	uint64_t pxl_clk_rate_max_080v = 645000000UL;  /* 645MHz */

	pinfo = &(dpufd->panel_info);
	pipe_clk_ctrl = &(dpufd->pipe_clk_ctrl);

	if (pinfo->ldi.pipe_clk_rate_pre_set)
		return get_preset_para_for_pipe_clk_updt(dpufd);

	pipe_clk_rate_ppll[0] = calc_pipe_clk_rate_by_ppll(CRGPERI_PLL0_CLK_RATE, dp_pxl_clk_rate);
	pipe_clk_rate_ppll[1] = calc_pipe_clk_rate_by_ppll(CRGPERI_PLL2_CLK_RATE, dp_pxl_clk_rate);
	pipe_clk_rate_ppll[2] = calc_pipe_clk_rate_by_ppll(CRGPERI_PLL3_CLK_RATE, dp_pxl_clk_rate);

	pipe_clk_rate = CRGPERI_PLL0_CLK_RATE;
	for (count = 0; count < 3; count++) {  /* PLL0/PLL1/PLL2 clk rate */
		if ((pipe_clk_rate_ppll[count] > 0) && (pipe_clk_rate > pipe_clk_rate_ppll[count]))
			pipe_clk_rate = pipe_clk_rate_ppll[count];
	}

	/* clk division from 1 to 19 */
	for (pipe_clk_rate_div = 1; pipe_clk_rate_div < 20; pipe_clk_rate_div++) {
		if ((pipe_clk_rate / pipe_clk_rate_div) < pinfo->pxl_clk_rate)
			break;
	}
	pipe_clk_rate_div--;

	if (pipe_clk_rate_div < 1) {
		ret = -2;
	} else if (((pipe_clk_rate / pipe_clk_rate_div) < pinfo->pxl_clk_rate) ||
		(pipe_clk_rate > pxl_clk_rate_max_080v)) {
		ret = -3;
	} else {
		dpufd->pipe_clk_ctrl.pipe_clk_rate = pipe_clk_rate;
		dpufd->pipe_clk_ctrl.pipe_clk_rate_div = pipe_clk_rate_div;
		ret = dpufb_get_ldi_hporch_updt_para(dpufd, false);
	}

	DPU_FB_DEBUG("ret[%d],pipe_clk_rate[%llu]->[%llu], pipe_clk_rate_div[%d]->[%d].\n",
		ret, pipe_clk_rate, dpufd->pipe_clk_ctrl.pipe_clk_rate, pipe_clk_rate_div,
		dpufd->pipe_clk_ctrl.pipe_clk_rate_div);

	return ret;
}

static int wait_pipe_clk_para_updt_end(void)
{
	struct dpu_fb_data_type *primary_dpufd = NULL;
	struct dpu_panel_info *primary_pinfo = NULL;
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;
	uint32_t wait_time = 0;
	int ret = 0;

	primary_dpufd = dpufd_list[PRIMARY_PANEL_IDX];
	if (primary_dpufd == NULL) {
		DPU_FB_ERR("dpufd_primary is NULL .\n");
		return -1;
	}
	primary_pinfo = &(primary_dpufd->panel_info);

	pipe_clk_ctrl = &(primary_dpufd->pipe_clk_ctrl);

	while (pipe_clk_ctrl->pipe_clk_updt_state != PARA_UPDT_END) {
		if (!primary_dpufd->panel_power_on) {
			DPU_FB_INFO("primary_panel_power off, break.\n");
			break;
		}
		if ((wait_time++) > 200) {  /* wait time 1s */
			DPU_FB_ERR("wait PARA_UPDT_END time > 1 s.\n");
			ret = -1;
			break;
		}
		msleep(5);  /* sleep 5 ms */
	}

	if ((!primary_dpufd->panel_power_on) && (pipe_clk_ctrl->pipe_clk_updt_times == 0)) {
		DPU_FB_INFO("primary_panel_power_on[%d], only change pipeclkrate\n", primary_dpufd->panel_power_on);
		ret = dpufb_change_pipe_clk_rate(primary_dpufd, pipe_clk_ctrl->pipe_clk_rate);
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
		if (ret < 0) {
			DPU_FB_ERR("set pipe_clk rate fail. ret=%d.\n", ret);
			return ret;
		}
	} else if (pipe_clk_ctrl->pipe_clk_updt_times >= 5) {
		ret = -1;
	}

	return ret;
}

int dpufb_pipe_clk_input_para_check(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_data_type *primary_dpufd = NULL;
	struct dpu_panel_info *primary_pinfo = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is null\n");
		return -1;
	}

	primary_dpufd = dpufd_list[PRIMARY_PANEL_IDX];
	if (primary_dpufd == NULL) {
		DPU_FB_ERR("dpufd_primary is NULL .\n");
		return -1;
	}
	primary_pinfo = &(primary_dpufd->panel_info);

	return 0;
}

static int dpufb_pipe_clk_pre_process(struct dpu_fb_data_type *primary_dpufd)
{
	struct dpu_panel_info *primary_pinfo = NULL;
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;
	int ret = 0;

	primary_pinfo = &(primary_dpufd->panel_info);
	pipe_clk_ctrl = &(primary_dpufd->pipe_clk_ctrl);

	if (primary_dpufd->panel_power_on) {
		if (pipe_clk_ctrl->pipe_clk_rate > primary_pinfo->pxl_clk_rate) {
			pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_NEED;
			hisi_fb_frame_refresh(primary_dpufd, "pipeclk");
		}
	} else {
		if (pipe_clk_ctrl->pipe_clk_rate > primary_pinfo->pxl_clk_rate) {
			ret = dpufb_change_pipe_clk_rate(primary_dpufd, pipe_clk_ctrl->pipe_clk_rate);
			if (ret < 0) {
				DPU_FB_ERR("set pipe_clk rate fail. ret=%d.\n", ret);
				return ret;
			}
		}
	}

	return ret;
}

static void dpufb_pipe_clk_updt_disable_dirty_region(bool disable)
{
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;

	if (dpufd_list[PRIMARY_PANEL_IDX] == NULL)
		return;

	pipe_clk_ctrl = &(dpufd_list[PRIMARY_PANEL_IDX]->pipe_clk_ctrl);

	if (disable)
		pipe_clk_ctrl->dirty_region_updt_disable = 1;
	else
		pipe_clk_ctrl->dirty_region_updt_disable = 0;
}

static int pipe_clk_updt_dpe_on(struct dpu_fb_data_type *primary_dpufd, struct dpu_panel_info *primary_pinfo,
	struct dpufb_pipe_clk *pipe_clk_ctrl, uint64_t dp_pxl_clk_rate)
{
	int ret;

	ret = get_para_for_pipe_clk_updt(primary_dpufd, dp_pxl_clk_rate);
	if (ret < 0) {
		DPU_FB_ERR("pipe_clk para calc error. ret=%d.\n", ret);
		return ret;
	}

	if (pipe_clk_ctrl->pipe_clk_rate > primary_pinfo->pxl_clk_rate)
		dpufb_pipe_clk_updt_disable_dirty_region(true);

	ret = dpufb_pipe_clk_pre_process(primary_dpufd);
	if (ret < 0) {
		DPU_FB_ERR("pipe_clk_pre_process fail, ret=%d .\n", ret);
		dpufb_pipe_clk_updt_disable_dirty_region(false);
		return ret;
	}

	if (pipe_clk_ctrl->pipe_clk_updt_state != PARA_UPDT_END) {
		ret = wait_pipe_clk_para_updt_end();
		if (ret < 0) {
			DPU_FB_ERR("dp_on, pipe_clk_updt failed .\n");
			pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
			dpufb_pipe_clk_updt_disable_dirty_region(false);
			return ret;
		}
	}

	return ret;
}

static void pipe_clk_updt_dpe_off(struct dpu_fb_data_type *primary_dpufd, struct dpu_panel_info *primary_pinfo,
		struct dpufb_pipe_clk *pipe_clk_ctrl)
{
	if (pipe_clk_ctrl->pipe_clk_rate > primary_pinfo->pxl_clk_rate) {
		pipe_clk_ctrl->pipe_clk_rate = primary_pinfo->pxl_clk_rate;
		pipe_clk_ctrl->pipe_clk_rate_div = 1;
		pipe_clk_ctrl->pipe_clk_updt_hporch[0] = primary_pinfo->ldi.h_back_porch;
		pipe_clk_ctrl->pipe_clk_updt_hporch[1] = primary_pinfo->ldi.h_front_porch;
		pipe_clk_ctrl->pipe_clk_updt_hporch[2] = primary_pinfo->ldi.h_pulse_width;
		if (primary_dpufd->panel_power_on) {
			pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_NEED;
			hisi_fb_frame_refresh(primary_dpufd, "pipeclk");
		}

		dpufb_pipe_clk_updt_disable_dirty_region(false);
	}
}

int dpufb_wait_pipe_clk_updt(struct dpu_fb_data_type *dpufd, bool dp_on)
{
	struct dpu_fb_data_type *primary_dpufd = NULL;
	struct dpu_panel_info *primary_pinfo = NULL;
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;
	int ret;
	uint64_t dp_pxl_clk_rate;

	ret = dpufb_pipe_clk_input_para_check(dpufd);
	dpu_check_and_return((ret < 0), ret, ERR, "input para check fail.\n");

	dpu_check_and_return((!is_dp_panel(dpufd)), 0, DEBUG, "fb%d, is not dp panel\n", dpufd->index);

	dp_pxl_clk_rate = dpufd->panel_info.pxl_clk_rate;
	primary_dpufd = dpufd_list[PRIMARY_PANEL_IDX];
	primary_pinfo = &(primary_dpufd->panel_info);

	DPU_FB_INFO("+, dp_on[%d], dp_pxl_clk_rate = %llu\n", dp_on, dp_pxl_clk_rate);

	if (dp_pxl_clk_rate <= primary_pinfo->pxl_clk_rate) {
		DPU_FB_INFO("pxl_clk_rate[%llu] is enough, return.\n", primary_pinfo->pxl_clk_rate);
		return 0;
	}

	pipe_clk_ctrl = &(primary_dpufd->pipe_clk_ctrl);
	pipe_clk_ctrl->pipe_clk_updt_times = 0;  /* clean upda_times when dp in/out. */
	pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;

	DPU_FB_DEBUG("primary_pinfo:\n"
		"pxl_clk_rate[%llu], pxl_clk_rate_div[%d], xres[%d], yres[%d],\n"
		"hbp[%d], hfp[%d], hpw[%d], vbp[%d], vfp[%d], vpw[%d].\n",
		primary_pinfo->pxl_clk_rate, primary_pinfo->pxl_clk_rate_div,
		primary_pinfo->xres, primary_pinfo->yres,
		primary_pinfo->ldi.h_back_porch, primary_pinfo->ldi.h_front_porch, primary_pinfo->ldi.h_pulse_width,
		primary_pinfo->ldi.v_back_porch, primary_pinfo->ldi.v_front_porch, primary_pinfo->ldi.v_pulse_width);

	if (dp_on) {
		ret = pipe_clk_updt_dpe_on(primary_dpufd, primary_pinfo, pipe_clk_ctrl, dp_pxl_clk_rate);
		if (ret < 0) {
			DPU_FB_ERR("pipe_clk_updt_dpe_on fail. ret=%d.\n", ret);
			return ret;
		}

	} else {
		/* dp_off */
		pipe_clk_updt_dpe_off(primary_dpufd, primary_pinfo, pipe_clk_ctrl);
	}

	DPU_FB_DEBUG(" -, pipe_clk_updt_state[%d],\n"
		"pipe_clk_rate[%llu], pipe_clk_rate_div[%d],\n"
		"pipe_clk_updt:hbp[%d], hfp[%d], hpw[%d].\n",
		pipe_clk_ctrl->pipe_clk_updt_state,
		pipe_clk_ctrl->pipe_clk_rate,
		pipe_clk_ctrl->pipe_clk_rate_div,
		pipe_clk_ctrl->pipe_clk_updt_hporch[0],
		pipe_clk_ctrl->pipe_clk_updt_hporch[1],
		pipe_clk_ctrl->pipe_clk_updt_hporch[2]);

	return 0;
}

static void wait_ldi_vstate_idle_time(struct dpu_fb_data_type *dpufd)
{
	int delay_count = 0;

	while ((((uint32_t)inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_VSTATE)) & 0x7FF) != 0x1) {
		if (++delay_count > 16) { /* wait times 16ms */
			DPU_FB_ERR("wait ldi vstate idle timeout.\n");
			break;
		}
		usleep_range(1000, 1000); /* delay 1ms */
	}
	DPU_FB_INFO("wait ldi vstate idle %d ms.\n", delay_count);
}

static void hisi_pipe_clk_updt_work_handler(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;

	pipe_clk_ctrl = container_of(work, struct dpufb_pipe_clk, pipe_clk_handle_work);
	dpufd = pipe_clk_ctrl->dpufd;
	dpu_check_and_no_retval((dpufd == NULL), ERR, "dpufd is NULL!\n");
	dpu_check_and_no_retval((dpufd->index != PRIMARY_PANEL_IDX), ERR,
		"fb%d pipe_clk_updt not support.\n", dpufd->index);

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	down(&dpufd->blank_sem0);

	if (pipe_clk_ctrl->pipe_clk_updt_state == PARA_UPDT_END) {
		DPU_FB_INFO("fb%d, no need to updt pipe_clk, state[%d] .\n",
			dpufd->index, pipe_clk_ctrl->pipe_clk_updt_state);
		up(&dpufd->blank_sem0);
		return;
	}

	if (dpufd->pipe_clk_ctrl.underflow_int) {
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_NEED;
		up(&dpufd->blank_sem0);
		DPU_FB_INFO("fb%d, underflow hasn't been cleaded up .\n", dpufd->index);
		return;
	}

	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel_power_off .\n", dpufd->index);
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
		up(&dpufd->blank_sem0);
		return;
	}

	dpufb_activate_vsync(dpufd);
	disable_ldi(dpufd);
	wait_ldi_vstate_idle_time(dpufd);

	if (dpufb_pipe_clk_updt_handler(dpufd, false) < 0) {
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_NEED;
		pipe_clk_ctrl->pipe_clk_updt_times++;
		if (pipe_clk_ctrl->pipe_clk_updt_times >= 5) { /* 5 is try times */
			DPU_FB_ERR("pipe_clk_updt fail, total times > 5.\n");
			pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
		}
	} else {
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
	}

	enable_ldi(dpufd);
	dpufb_deactivate_vsync(dpufd);

	up(&dpufd->blank_sem0);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);
}

void dpufb_pipe_clk_updt_isr_handler(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	pipe_clk_ctrl = &(dpufd->pipe_clk_ctrl);
	if (pipe_clk_ctrl == NULL) {
		DPU_FB_ERR("pipe_clk_ctrl is NULL\n");
		return;
	}

	if (!pipe_clk_ctrl->inited) {
		DPU_FB_ERR("pipe_clk_ctrl has not been inited.\n");
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
		return;
	}

	queue_work(pipe_clk_ctrl->pipe_clk_handle_wq, &(pipe_clk_ctrl->pipe_clk_handle_work));
}

void dpufb_pipe_clk_updt_work_init(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_pipe_clk *pipe_clk_ctrl = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	if (dpufd->index != PRIMARY_PANEL_IDX)
		return;

	pipe_clk_ctrl = &(dpufd->pipe_clk_ctrl);

	if (pipe_clk_ctrl->inited) {
		DPU_FB_INFO("pipe_clk_ctrl has been inited\n");
		return;
	}
	pipe_clk_ctrl->dpufd = dpufd;
	pipe_clk_ctrl->pipe_clk_rate = 0;
	pipe_clk_ctrl->underflow_int = 0;
	pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
	pipe_clk_ctrl->dirty_region_updt_disable = 0;

	pipe_clk_ctrl->pipe_clk_handle_wq = create_singlethread_workqueue("pipe_clk_updt_work");
	if (pipe_clk_ctrl->pipe_clk_handle_wq == NULL) {
		DPU_FB_ERR("fb%d, create pipeclk_handle workqueue failed!\n", dpufd->index);
		return;
	}

	INIT_WORK(&(pipe_clk_ctrl->pipe_clk_handle_work), hisi_pipe_clk_updt_work_handler);
	pipe_clk_ctrl->inited = 1;
}
#pragma GCC diagnostic pop

