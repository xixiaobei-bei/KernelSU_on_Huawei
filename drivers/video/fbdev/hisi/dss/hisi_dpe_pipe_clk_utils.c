/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
*/

#include "hisi_dpe_pipe_clk_utils.h"

static int hisifb_change_pipe_clk_rate(struct hisi_fb_data_type *hisifd, uint64_t pipe_clk_rate)
{
	int ret = -1;

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);
	if (hisifd->dss_pxl0_clk) {
		ret = clk_set_rate(hisifd->dss_pxl0_clk, pipe_clk_rate); //ppll0 div
		if (ret < 0) {
			HISI_FB_ERR("set pipe_clk_rate[%llu] fail, reset to [%llu], ret[%d].\n",
				hisifd->pipe_clk_ctrl.pipe_clk_rate, hisifd->panel_info.pxl_clk_rate, ret);

			ret = clk_set_rate(hisifd->dss_pxl0_clk, hisifd->panel_info.pxl_clk_rate);
			if ((ret < 0) && (g_fpga_flag == 0)) {
				HISI_FB_ERR("fb%d dss_pxl0_clk clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, hisifd->panel_info.pxl_clk_rate, ret);
				return -EINVAL;
			}
		}

		HISI_FB_INFO("dss_pxl0_clk:[%llu]->[%llu].\n",
			pipe_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl0_clk));
	}

	return ret;
}

static int hisifb_pipe_clk_updt_config(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct hisifb_pipe_clk *pipe_clk_ctrl = NULL;
	char __iomem *ldi_base = NULL;
	uint32_t pxl0_divxcfg = 0;
	uint32_t mipi_idx;
	int ret = 0;

	if (NULL == hisifd) {
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);
	pipe_clk_ctrl = &(hisifd->pipe_clk_ctrl);
	if (NULL == pinfo) {
		return -EINVAL;
	}

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		return ret;
	}

	ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;

	ret = hisifb_change_pipe_clk_rate(hisifd, pipe_clk_ctrl->pipe_clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d, set clk_ldi0 to [%llu] failed, error=%d!\n",
			hisifd->index, pipe_clk_ctrl->pipe_clk_rate, ret);
		return ret;
	}

	mipi_idx = is_dual_mipi_panel(hisifd) ? 1 : 0;
	pxl0_divxcfg = g_mipi_ifbc_division[mipi_idx][pinfo->ifbc_type].pxl0_divxcfg;
	pxl0_divxcfg = (pxl0_divxcfg + 1) * pipe_clk_ctrl->pipe_clk_rate_div;
	set_reg(ldi_base + LDI_PXL0_DIVXCFG, (pxl0_divxcfg - 1), 3, 0);

	if (is_mipi_video_panel(hisifd)) {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0,
				pipe_clk_ctrl->pipe_clk_updt_hporch[1] | ((pipe_clk_ctrl->pipe_clk_updt_hporch[0] + DSS_WIDTH(pipe_clk_ctrl->pipe_clk_updt_hporch[2])) << 16));
	} else {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0, pipe_clk_ctrl->pipe_clk_updt_hporch[1] | (pipe_clk_ctrl->pipe_clk_updt_hporch[0] << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, DSS_WIDTH(pipe_clk_ctrl->pipe_clk_updt_hporch[2]));
	}

	if (is_dual_mipi_panel(hisifd)) {
		if (is_mipi_video_panel(hisifd)) {
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL0, (pipe_clk_ctrl->pipe_clk_updt_hporch[0] + DSS_WIDTH(pipe_clk_ctrl->pipe_clk_updt_hporch[2])) << 16);
		} else {
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL0, pipe_clk_ctrl->pipe_clk_updt_hporch[0] << 16);
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL1, DSS_WIDTH(pipe_clk_ctrl->pipe_clk_updt_hporch[2]));
		}
	}

	HISI_FB_INFO("set pipe_clk_updt: rate[%llu], div[%d], pxl0_divxcfg[%d]; updt hporch: hbp[%d], hfp[%d], hpw[%d].\n",
		pipe_clk_ctrl->pipe_clk_rate,
		pipe_clk_ctrl->pipe_clk_rate_div,
		pxl0_divxcfg,
		pipe_clk_ctrl->pipe_clk_updt_hporch[0],
		pipe_clk_ctrl->pipe_clk_updt_hporch[1],
		pipe_clk_ctrl->pipe_clk_updt_hporch[2]);

	return ret;
}

int hisifb_pipe_clk_updt_handler(struct hisi_fb_data_type *primary_hisifd, bool primary_panel_pwr_on)
{
	struct hisi_panel_info *pinfo = NULL;
	struct hisifb_pipe_clk *pipe_clk_ctrl = NULL;
	int ret = 0;

	if (NULL == primary_hisifd) {
		HISI_FB_ERR("primary_hisifd is null.\n");
		return -EINVAL;
	}

	if (primary_hisifd->index != PRIMARY_PANEL_IDX) {
		HISI_FB_INFO("fb%d not support.\n", primary_hisifd->index);
		return ret;
	}

	pinfo = &(primary_hisifd->panel_info);
	if (NULL == pinfo) {
		return -EINVAL;
	}
	pipe_clk_ctrl = &(primary_hisifd->pipe_clk_ctrl);

	HISI_FB_INFO("+, primary_panel_pwr_on[%d].\n", primary_panel_pwr_on);

	if (primary_panel_pwr_on) {
		if (hisifd_list[EXTERNAL_PANEL_IDX]) {
			down(&hisifd_list[EXTERNAL_PANEL_IDX]->blank_sem0);
			if ((hisifd_list[EXTERNAL_PANEL_IDX]->panel_power_on)
				&& (pipe_clk_ctrl->pipe_clk_rate > pinfo->pxl_clk_rate)) {
				ret = hisifb_pipe_clk_updt_config(primary_hisifd);
			}
			up(&hisifd_list[EXTERNAL_PANEL_IDX]->blank_sem0);
		}
	} else {
		if (pipe_clk_ctrl->pipe_clk_rate < pinfo->pxl_clk_rate) {
			HISI_FB_INFO("pipe_clk_rate[%llu] is less than pxl0_clk_rate[%llu].\n",
				pipe_clk_ctrl->pipe_clk_rate, pinfo->pxl_clk_rate);
			return ret;
		}
		/*only for pipe_clk_updt in vactive_end*/
		ret = hisifb_pipe_clk_updt_config(primary_hisifd);
	}

	HISI_FB_DEBUG("-.\n");
	return ret;
}

int hisifb_get_ldi_hporch_updt_para(struct hisi_fb_data_type *hisifd, bool fps_updt_use)
{
	struct hisi_panel_info *pinfo = NULL;
	struct hisifb_pipe_clk *pipe_clk_ctrl;
	uint64_t pxl_clk_rate_new;
	uint32_t pxl_clk_ratio = 1;
	uint32_t fps = 0;
	uint32_t T_frame = 0;
	uint32_t T_hline = 0;
	uint32_t T_hbphpw = 0;
	uint32_t h_size;
	uint32_t v_size;
	uint32_t hbp = 0;
	uint32_t hfp = 0;
	uint32_t hpw = 0;

	uint32_t hbp_new = 0;
	uint32_t hfp_new = 0;
	uint32_t hpw_new = 0;
	uint32_t precision = 100;//with 2 decimal places
	uint32_t millisecond = 1000;

	pinfo = &(hisifd->panel_info);
	if (NULL == pinfo) {
		HISI_FB_ERR("pinfo is null.\n");
		return -1;
	}

	pipe_clk_ctrl = &(hisifd->pipe_clk_ctrl);
	if (NULL == pipe_clk_ctrl) {
		HISI_FB_ERR("pipe_clk_ctrl is null.\n");
		return -1;
	}
	HISI_FB_DEBUG("+ ,fps_updt_use[%d].\n", fps_updt_use);

	if (fps_updt_use) {
		hbp = pinfo->ldi_updt.h_back_porch;
		hfp = pinfo->ldi_updt.h_front_porch;
		hpw = pinfo->ldi_updt.h_pulse_width;
	} else {
		hbp = pinfo->ldi.h_back_porch;
		hfp = pinfo->ldi.h_front_porch;
		hpw = pinfo->ldi.h_pulse_width;
	}

	pxl_clk_rate_new = pipe_clk_ctrl->pipe_clk_rate / pipe_clk_ctrl->pipe_clk_rate_div;

	if (pxl_clk_rate_new > pinfo->pxl_clk_rate) {
		if (pinfo->pxl_clk_rate_div > 1) {
			hbp *= pinfo->pxl_clk_rate_div;
			hfp *= pinfo->pxl_clk_rate_div;
			hpw *= pinfo->pxl_clk_rate_div;
		}

		/*hporch paras calc*/
		pxl_clk_ratio = precision * (pxl_clk_rate_new /100) / (pinfo->pxl_clk_rate / 100);//with 2 decimal places
		hbp_new = hbp * pxl_clk_ratio;//with 2 decimal places
		hpw_new = hpw * pxl_clk_ratio;//with 2 decimal places

		h_size = pinfo->xres + hbp + hfp + hpw;
		v_size = pinfo->yres + pinfo->ldi.v_back_porch + pinfo->ldi.v_front_porch + pinfo->ldi.v_pulse_width;
		fps = pinfo->pxl_clk_rate / h_size * precision / v_size;//us with 2 decimal places
		T_frame = millisecond * millisecond * precision / fps;//us
		T_hline = precision * T_frame / v_size;//us with 2 decimal places
		T_hbphpw = T_hline * (hbp + hpw) / h_size;//us with 2 decimal places
		hfp_new = (hbp_new + hpw_new) * T_hline / T_hbphpw - (pinfo->xres * precision + hbp_new + hpw_new);//with 2 decimal places
		hbp_new = hbp_new / precision + (hbp_new % precision > 0 ? 1 : 0);
		hfp_new = hfp_new / precision + (hfp_new % precision > 0 ? 1 : 0);
		hpw_new = hpw_new / precision + (hpw_new % precision > 0 ? 1 : 0);

		if (pinfo->pxl_clk_rate_div > 1) {
			hbp_new /= pinfo->pxl_clk_rate_div;
			hfp_new /= pinfo->pxl_clk_rate_div;
			hpw_new /= pinfo->pxl_clk_rate_div;
		}
	} else {
		hbp_new = hbp;
		hfp_new = hfp;
		hpw_new = hpw;
	}

	if (fps_updt_use) {
		pipe_clk_ctrl->fps_updt_hporch[0] = hbp_new;
		pipe_clk_ctrl->fps_updt_hporch[1] = hfp_new;
		pipe_clk_ctrl->fps_updt_hporch[2] = hpw_new;
	} else {
		pipe_clk_ctrl->pipe_clk_updt_hporch[0] = hbp_new;
		pipe_clk_ctrl->pipe_clk_updt_hporch[1] = hfp_new;
		pipe_clk_ctrl->pipe_clk_updt_hporch[2] = hpw_new;
	}
	HISI_FB_DEBUG("hbp[%d], hfp[%d], hpw[%d], pxl_clk_ratio[%d],\n"
		"fps[%d], T_frame[%d], T_hline[%d], T_hbphpw[%d]\n"
		"hbp_new[%d], hfp_new[%d], hpw_new[%d].\n",
		hbp, hfp, hpw, pxl_clk_ratio, fps, T_frame, T_hline, T_hbphpw, hbp_new, hfp_new, hpw_new);

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
		for (div = 1; div < 20; div++) {
			if ((ppll_clk_rate / div) < dp_pxl_clk_rate) {
				break;
			}
		}
		div = div - 1;
		if (div > 0) {
			pipe_clk_rate_ppll = (ppll_clk_rate % div) ? (ppll_clk_rate / div + 1) : (ppll_clk_rate / div);
		} else {
			HISI_FB_ERR("div = %d.\n", div);
		}
	}

	HISI_FB_DEBUG("ppll_clk_rate[%llu], div[%d], pipe_clk_rate_ppll[%llu].\n", ppll_clk_rate, div, pipe_clk_rate_ppll);
	return pipe_clk_rate_ppll;
}

static int get_para_for_pipe_clk_updt(struct hisi_fb_data_type *hisifd, uint64_t dp_pxl_clk_rate)
{
	struct hisi_panel_info *pinfo = NULL;
	uint64_t pipe_clk_rate;
	uint64_t pipe_clk_rate_ppll[3];
	uint32_t pipe_clk_rate_div = 1;
	int count;
	int ret = 0;
	uint64_t pxl_clk_rate_max_080v = 645000000UL;

	pinfo = &(hisifd->panel_info);

	pipe_clk_rate_ppll[0] = calc_pipe_clk_rate_by_ppll(CRGPERI_PLL0_CLK_RATE, dp_pxl_clk_rate);
	pipe_clk_rate_ppll[1] = calc_pipe_clk_rate_by_ppll(CRGPERI_PLL2_CLK_RATE, dp_pxl_clk_rate);
	pipe_clk_rate_ppll[2] = calc_pipe_clk_rate_by_ppll(CRGPERI_PLL3_CLK_RATE, dp_pxl_clk_rate);
	//pipe_clk_rate_ppll[3] = calc_pipe_clk_rate_by_ppll(CRGPERI_PLL7_CLK_RATE, dp_pxl_clk_rate);

	pipe_clk_rate = CRGPERI_PLL0_CLK_RATE;
	for (count = 0; count < 3; count++) {
		if ((pipe_clk_rate_ppll[count] > 0) && (pipe_clk_rate > pipe_clk_rate_ppll[count])) {
			pipe_clk_rate = pipe_clk_rate_ppll[count];
		}
	}

	for (pipe_clk_rate_div = 1; pipe_clk_rate_div < 20; pipe_clk_rate_div++) {
		if ((pipe_clk_rate/pipe_clk_rate_div) < pinfo->pxl_clk_rate) {
			break;
		}
	}
	pipe_clk_rate_div --;

	if (pipe_clk_rate_div < 1) {
		ret = -2;
	} else if (((pipe_clk_rate/pipe_clk_rate_div) < pinfo->pxl_clk_rate) || (pipe_clk_rate > pxl_clk_rate_max_080v)) {
		ret = -3;
	} else {
		hisifd->pipe_clk_ctrl.pipe_clk_rate = pipe_clk_rate;
		hisifd->pipe_clk_ctrl.pipe_clk_rate_div = pipe_clk_rate_div;
		ret = hisifb_get_ldi_hporch_updt_para(hisifd, false);
	}

	HISI_FB_DEBUG("ret[%d],pipe_clk_rate[%llu]->[%llu], pipe_clk_rate_div[%d]->[%d].\n",
		ret, pipe_clk_rate, hisifd->pipe_clk_ctrl.pipe_clk_rate, pipe_clk_rate_div, hisifd->pipe_clk_ctrl.pipe_clk_rate_div);

	return ret;
}

static int wait_pipe_clk_para_updt_end(void)
{
	struct hisi_fb_data_type *primary_hisifd= NULL;
	struct hisi_panel_info *primary_pinfo = NULL;
	struct hisifb_pipe_clk *pipe_clk_ctrl = NULL;
	uint32_t wait_time = 0;
	int ret = 0;

	primary_hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (NULL == primary_hisifd) {
		HISI_FB_ERR("hisifd_primary is NULL .\n");
		return -1;
	}
	primary_pinfo = &(primary_hisifd->panel_info);
	if (NULL == primary_pinfo) {
		HISI_FB_ERR("pinfo is NULL .\n");
		return -1;
	}
	pipe_clk_ctrl = &(primary_hisifd->pipe_clk_ctrl);

	while (pipe_clk_ctrl->pipe_clk_updt_state != PARA_UPDT_END) {
		if (!primary_hisifd->panel_power_on) {
			HISI_FB_INFO("primary_panel_power off, break.\n");
			break;
		}
		if ((wait_time++) > 200) {
			HISI_FB_ERR("wait PARA_UPDT_END time > 1 s.\n");
			ret = -1;
			break;
		}
		msleep(5);
	}

	if ((!primary_hisifd->panel_power_on) && (pipe_clk_ctrl->pipe_clk_updt_times == 0)) {
		HISI_FB_INFO("primary_panel_power_on[%d], only change pipeclkrate\n", primary_hisifd->panel_power_on);
		ret = hisifb_change_pipe_clk_rate(primary_hisifd, pipe_clk_ctrl->pipe_clk_rate);
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
		if (ret < 0) {
			HISI_FB_ERR("set pipe_clk rate fail. ret=%d.\n", ret);
			return ret;
		}
	} else if (pipe_clk_ctrl->pipe_clk_updt_times >= 5) {
		ret = -1;
	}

	return ret;
}

static void hisifb_primary_panel_refresh_notification(struct hisi_fb_data_type *hisifd)
{
	char *envp[2];
	char buf[64];

	snprintf(buf, sizeof(buf), "Refresh=1");
	envp[0] = buf;
	envp[1] = NULL;
	kobject_uevent_env(&(hisifd->fbi->dev->kobj), KOBJ_CHANGE, envp);

	HISI_FB_DEBUG("pipe clk updt refresh frame.\n");
	return;
}

int hisifb_pipe_clk_input_para_check(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_data_type *primary_hisifd= NULL;
	struct hisi_panel_info *primary_pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null \n");
		return -1;
	}

	primary_hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (NULL == primary_hisifd) {
		HISI_FB_ERR("hisifd_primary is NULL .\n");
		return -1;
	}
	primary_pinfo = &(primary_hisifd->panel_info);
	if (NULL == primary_pinfo) {
		HISI_FB_ERR("pinfo is NULL .\n");
		return -1;
	}

	return 0;
}

static int hisifb_pipe_clk_pre_process(struct hisi_fb_data_type *primary_hisifd)
{
	struct hisi_panel_info *primary_pinfo;
	struct hisifb_pipe_clk *pipe_clk_ctrl;
	int ret = 0;

	primary_pinfo = &(primary_hisifd->panel_info);
	pipe_clk_ctrl = &(primary_hisifd->pipe_clk_ctrl);

	if (primary_hisifd->panel_power_on) {
		if (pipe_clk_ctrl->pipe_clk_rate > primary_pinfo->pxl_clk_rate) {
			pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_NEED;
			hisifb_primary_panel_refresh_notification(primary_hisifd);
		}
	} else {
		if (pipe_clk_ctrl->pipe_clk_rate > primary_pinfo->pxl_clk_rate) {
			ret = hisifb_change_pipe_clk_rate(primary_hisifd, pipe_clk_ctrl->pipe_clk_rate);
			if (ret < 0) {
				HISI_FB_ERR("set pipe_clk rate fail. ret=%d.\n", ret);
				return ret;
			}
		}
	}

	return ret;
}

/*FIXME:*/
static void hisifb_pipe_clk_updt_disable_dirty_region(bool disable)
{
	struct hisifb_pipe_clk *pipe_clk_ctrl;
	if (NULL == hisifd_list[PRIMARY_PANEL_IDX]) {
		return;
	}
	pipe_clk_ctrl = &(hisifd_list[PRIMARY_PANEL_IDX]->pipe_clk_ctrl);
	if (NULL == pipe_clk_ctrl) {
		return;
	}

	if (disable) {
		pipe_clk_ctrl->dirty_region_updt_disable = 1;
	} else {
		pipe_clk_ctrl->dirty_region_updt_disable = 0;
	}

	return;
}

int hisifb_wait_pipe_clk_updt(struct hisi_fb_data_type *hisifd, bool dp_on)
{
	struct hisi_fb_data_type *primary_hisifd= NULL;
	struct hisi_panel_info *primary_pinfo = NULL;
	struct hisifb_pipe_clk *pipe_clk_ctrl = NULL;
	int ret = 0;
	uint64_t dp_pxl_clk_rate = 0;

	ret = hisifb_pipe_clk_input_para_check(hisifd);
	if (ret < 0) {
		HISI_FB_ERR("input para check fail.\n");
		return ret;
	}
	if (!is_dp_panel(hisifd)) {
		HISI_FB_DEBUG("fb%d, is not dp panel \n", hisifd->index);
		return 0;
	}

	dp_pxl_clk_rate = hisifd->panel_info.pxl_clk_rate;
	primary_hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	primary_pinfo = &(primary_hisifd->panel_info);
	if (dp_pxl_clk_rate <= primary_pinfo->pxl_clk_rate) {
		HISI_FB_INFO("pxl_clk_rate[%llu] is enough for dp_pxl_clk_rate[%llu]. \n",
			primary_pinfo->pxl_clk_rate, dp_pxl_clk_rate);
		return 0;
	}

	pipe_clk_ctrl = &(primary_hisifd->pipe_clk_ctrl);
	pipe_clk_ctrl->pipe_clk_updt_times = 0;//clean upda_times when dp in/out.
	pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;

	HISI_FB_INFO("+, dp_on[%d], dp_pxl_clk_rate = %llu \n",dp_on, dp_pxl_clk_rate);
	HISI_FB_DEBUG("primary_pinfo: \n"
		"pxl_clk_rate[%llu], pxl_clk_rate_div[%d], xres[%d], yres[%d],\n"
		"hbp[%d], hfp[%d], hpw[%d], vbp[%d], vfp[%d], vpw[%d].\n",
		primary_pinfo->pxl_clk_rate, primary_pinfo->pxl_clk_rate_div,
		primary_pinfo->xres, primary_pinfo->yres,
		primary_pinfo->ldi.h_back_porch, primary_pinfo->ldi.h_front_porch, primary_pinfo->ldi.h_pulse_width,
		primary_pinfo->ldi.v_back_porch, primary_pinfo->ldi.v_front_porch, primary_pinfo->ldi.v_pulse_width);

	if (dp_on) {
		ret = get_para_for_pipe_clk_updt(primary_hisifd, dp_pxl_clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("pipe_clk para calc error. ret=%d.\n", ret);
			return ret;
		}

		if (pipe_clk_ctrl->pipe_clk_rate > primary_pinfo->pxl_clk_rate) {
			hisifb_pipe_clk_updt_disable_dirty_region(true);
		}

		ret = hisifb_pipe_clk_pre_process(primary_hisifd);
		if (ret < 0) {
			HISI_FB_ERR("pipe_clk_pre_process fail, ret=%d .\n", ret);
			hisifb_pipe_clk_updt_disable_dirty_region(false);
			return ret;
		}

		if (pipe_clk_ctrl->pipe_clk_updt_state != PARA_UPDT_END) {
			ret = wait_pipe_clk_para_updt_end();
			if (ret < 0) {
				HISI_FB_ERR("dp_on, pipe_clk_updt failed .\n");
				pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
				hisifb_pipe_clk_updt_disable_dirty_region(false);
				return ret;
			}
		}
	} else {
		/*dp_off*/
		if (pipe_clk_ctrl->pipe_clk_rate > primary_pinfo->pxl_clk_rate) {
			pipe_clk_ctrl->pipe_clk_rate = primary_pinfo->pxl_clk_rate;
			pipe_clk_ctrl->pipe_clk_rate_div = 1;
			pipe_clk_ctrl->pipe_clk_updt_hporch[0] = primary_pinfo->ldi.h_back_porch;
			pipe_clk_ctrl->pipe_clk_updt_hporch[1] = primary_pinfo->ldi.h_front_porch;
			pipe_clk_ctrl->pipe_clk_updt_hporch[2] = primary_pinfo->ldi.h_pulse_width;
			if (primary_hisifd->panel_power_on) {
				pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_NEED;
				hisifb_primary_panel_refresh_notification(primary_hisifd);
			}
			hisifb_pipe_clk_updt_disable_dirty_region(false);
		}
	}

	HISI_FB_DEBUG(" -, pipe_clk_updt_state[%d],\n"
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

static void hisi_pipe_clk_updt_work_handler(struct work_struct *work)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisifb_pipe_clk *pipe_clk_ctrl = NULL;

	pipe_clk_ctrl = container_of(work, struct hisifb_pipe_clk, pipe_clk_handle_work);
	hisifd = pipe_clk_ctrl->hisifd;
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return;
	}
	if (hisifd->index != PRIMARY_PANEL_IDX) {
		HISI_FB_ERR("fb%d pipe_clk_updt not support.\n", hisifd->index);
		return;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	down(&hisifd->blank_sem0);

	if (PARA_UPDT_END == pipe_clk_ctrl->pipe_clk_updt_state) {
		HISI_FB_INFO("fb%d, no need to updt pipe_clk, state[%d] .\n", hisifd->index, pipe_clk_ctrl->pipe_clk_updt_state);
		up(&hisifd->blank_sem0);
		return;
	}

	if (hisifd->pipe_clk_ctrl.underflow_int) {
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_NEED;
		up(&hisifd->blank_sem0);
		HISI_FB_INFO("fb%d, underflow hasn't been cleaded up .\n", hisifd->index);
		return;
	}

	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel_power_off .\n", hisifd->index);
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
		up(&hisifd->blank_sem0);
		return;
	}

	hisifb_activate_vsync(hisifd);
	disable_ldi(hisifd);
	if (hisifb_pipe_clk_updt_handler(hisifd, false) < 0) {
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_NEED;
		pipe_clk_ctrl->pipe_clk_updt_times ++;
		if (pipe_clk_ctrl->pipe_clk_updt_times >= 5) {
			HISI_FB_ERR("pipe_clk_updt fail, total times > 5.\n");
			pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
		}
	} else {
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
	}

	enable_ldi(hisifd);
	hisifb_deactivate_vsync(hisifd);

	up(&hisifd->blank_sem0);

	HISI_FB_DEBUG("fb%d, - \n", hisifd->index);
}

void hisifb_pipe_clk_updt_isr_handler(struct hisi_fb_data_type *hisifd)
{
	struct hisifb_pipe_clk *pipe_clk_ctrl;
	pipe_clk_ctrl = &(hisifd->pipe_clk_ctrl);

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return;
	}
	if (NULL == pipe_clk_ctrl) {
		HISI_FB_ERR("pipe_clk_ctrl is NULL\n");
		return;
	}

	if (!pipe_clk_ctrl->inited) {
		HISI_FB_ERR("pipe_clk_ctrl has not been inited. \n");
		pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
		return;
	}
	if ((inp32(hisifd->dss_base + DSS_LDI0_OFFSET + LDI_CTRL) & 0x1) != 0) {
		HISI_FB_INFO("disable ldi fail, continue trying to update pipe_clk.\n");
	}
	queue_work(pipe_clk_ctrl->pipe_clk_handle_wq, &(pipe_clk_ctrl->pipe_clk_handle_work));
	return;
}

void hisifb_pipe_clk_updt_work_init(struct hisi_fb_data_type *hisifd)
{
	struct hisifb_pipe_clk *pipe_clk_ctrl;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return;
	}
	if (hisifd->index != PRIMARY_PANEL_IDX) {
		return;
	}
	pipe_clk_ctrl = &(hisifd->pipe_clk_ctrl);
	if (NULL == pipe_clk_ctrl) {
		HISI_FB_ERR("pipe_clk_ctrl is NULL\n");
		return;
	}
	if (pipe_clk_ctrl->inited) {
		HISI_FB_INFO("pipe_clk_ctrl has been inited\n");
		return;
	}
	pipe_clk_ctrl->hisifd = hisifd;
	pipe_clk_ctrl->pipe_clk_rate = 0;
	pipe_clk_ctrl->underflow_int = 0;
	pipe_clk_ctrl->pipe_clk_updt_state = PARA_UPDT_END;
	pipe_clk_ctrl->dirty_region_updt_disable = 0;

	pipe_clk_ctrl->pipe_clk_handle_wq = create_singlethread_workqueue("pipe_clk_updt_work");
	if (!pipe_clk_ctrl->pipe_clk_handle_wq) {
		HISI_FB_ERR("fb%d, create pipeclk_handle workqueue failed!\n", hisifd->index);
		return;
	}
	INIT_WORK(&(pipe_clk_ctrl->pipe_clk_handle_work), hisi_pipe_clk_updt_work_handler);
	pipe_clk_ctrl->inited = 1;

}

