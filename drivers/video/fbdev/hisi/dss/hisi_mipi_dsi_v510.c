/*
 * Copyright (c) 2012-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_mipi_dsi.h"
#ifdef CONFIG_HISI_DISPLAY_DFR
#include "hisi_frame_rate_ctrl.h"
#endif

#define VFP_TIME_MASK 0x7fff
#define VFP_TIME_OFFSET 10
#define VFP_DEF_TIME 80
#define MILLION_CONVERT 1000000
#define PCTRL_TRY_TIME 10
#define DSI_CLK_BW 1
#define DSI_CLK_BS 0
#define VCO_CLK_MIN_VALUE 2000

struct mipi_dsi_clk_data {
	uint32_t accuracy;
	uint32_t ui;
	uint32_t unit_tx_byte_clk_hs;
	uint32_t clk_post;
	uint32_t clk_pre;
	uint32_t clk_t_hs_exit;
	uint32_t clk_pre_delay;
	uint32_t clk_t_hs_prepare;
	uint32_t clk_t_hs_trial;
	uint32_t data_post_delay;
	uint32_t data_t_hs_trial;
	uint32_t data_t_hs_prepare;
	uint32_t clk_t_lpx;
	uint32_t clk_t_hs_zero;
	uint32_t data_t_hs_zero;
	uint32_t data_t_lpx;
};

int mipi_dsi_ulps_cfg(struct dpu_fb_data_type *dpufd, int enable);

static uint32_t get_data_t_hs_prepare(struct dpu_fb_data_type *dpufd, uint32_t accuracy, uint32_t ui)
{
	struct dpu_panel_info *pinfo = NULL;
	uint32_t data_t_hs_prepare;
	uint32_t prepare_val1;
	uint32_t prepare_val2;

	pinfo = &(dpufd->panel_info);
	dpu_check_and_return(!pinfo, 0, ERR, "pinfo is NULL\n");

	/*
	 * D-PHY Specification : 40ns + 4*UI <= data_t_hs_prepare <= 85ns + 6*UI
	 * clocked by TXBYTECLKHS
	 * 35 is default adjust value
	 */
	if (pinfo->mipi.data_t_hs_prepare_adjust == 0)
		pinfo->mipi.data_t_hs_prepare_adjust = 35;

	prepare_val1 = 400 * accuracy + 4 * ui + pinfo->mipi.data_t_hs_prepare_adjust * ui;
	prepare_val2 = 850 * accuracy + 6 * ui - 8 * ui;
	data_t_hs_prepare = (prepare_val1 <= prepare_val2) ? prepare_val1 : prepare_val2;

	return data_t_hs_prepare;
}

static uint32_t get_data_pre_delay(uint32_t lp11_flag, struct mipi_dsi_phy_ctrl *phy_ctrl, uint32_t clk_pre)
{
	uint32_t data_pre_delay = 0;
	/* if use 1080 X 2160 resolution panel,need reduce the lp11 time, and disable noncontinue mode */
	if (lp11_flag != MIPI_SHORT_LP11)
		data_pre_delay = phy_ctrl->clk_pre_delay + 2 + phy_ctrl->clk_t_lpx +
			phy_ctrl->clk_t_hs_prepare + phy_ctrl->clk_t_hs_zero + 8 + clk_pre;

	return data_pre_delay;
}

static uint32_t get_data_pre_delay_reality(uint32_t lp11_flag, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	uint32_t data_pre_delay_reality = 0;
	/* if use 1080 X 2160 resolution panel,need reduce the lp11 time, and disable noncontinue mode */
	if (lp11_flag != MIPI_SHORT_LP11)
		data_pre_delay_reality = phy_ctrl->data_pre_delay + 5;

	return data_pre_delay_reality;
}

static uint32_t get_clk_post_delay_reality(uint32_t lp11_flag, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	uint32_t clk_post_delay_reality = 0;
	/* if use 1080 X 2160 resolution panel,need reduce the lp11 time, and disable noncontinue mode */
	if (lp11_flag != MIPI_SHORT_LP11)
		clk_post_delay_reality = phy_ctrl->clk_post_delay + 4;

	return clk_post_delay_reality;
}

static uint64_t get_default_lane_byte_clk(struct dpu_fb_data_type *dpufd)
{
	int rg_pll_posdiv = 0;
	uint32_t post_div[6] = { 1, 2, 4, 8, 16, 32 };  /* clk division */
	uint64_t lane_clock;
	uint64_t vco_clk;
	uint32_t rg_pll_fbkdiv;

	struct dpu_panel_info *pinfo = &(dpufd->panel_info);

	if (pinfo->mipi.dsi_bit_clk_default == 0) {
		DPU_FB_WARNING("reset dsi_bit_clk_default %u M -> %u M\n",
			pinfo->mipi.dsi_bit_clk_default, pinfo->mipi.dsi_bit_clk);
		pinfo->mipi.dsi_bit_clk_default = pinfo->mipi.dsi_bit_clk;
	}

	lane_clock = (uint64_t)(pinfo->mipi.dsi_bit_clk_default);
	if (pinfo->mipi.phy_mode == DPHY_MODE)
		lane_clock = lane_clock * 2;

	DPU_FB_DEBUG("default lane_clock %llu M\n", lane_clock);

	vco_clk = lane_clock * post_div[0];
	/* chip restrain, vco_clk_min and post_div index */
	while ((vco_clk <= 2000) && (rg_pll_posdiv < 5)) {
		rg_pll_posdiv++;
		vco_clk = lane_clock * post_div[rg_pll_posdiv];
	}
	vco_clk = vco_clk * 1000000;  /* MHZ to HZ */
	rg_pll_fbkdiv = vco_clk / DEFAULT_MIPI_CLK_RATE;
	lane_clock = rg_pll_fbkdiv * DEFAULT_MIPI_CLK_RATE / post_div[rg_pll_posdiv];

	DPU_FB_DEBUG("vco_clk %llu, rg_pll_fbkdiv %d, rg_pll_posdiv %d, lane_clock %llu\n",
		vco_clk, rg_pll_fbkdiv, rg_pll_posdiv, lane_clock);

	/* lanebyte clk formula which is stated in cdphy spec */
	if (pinfo->mipi.phy_mode == DPHY_MODE)
		return lane_clock / 8;
	else
		return lane_clock / 7;
}

static void mipi_dsi_pll_dphy_config(struct mipi_dsi_phy_ctrl *phy_ctrl, uint64_t *lane_clock,
	uint32_t *m_pll, uint32_t *n_pll)
{
	uint64_t vco_div = 1;  /* default clk division */
	uint64_t vco_clk;
	uint32_t post_div[6] = { 1, 2, 4, 8, 16, 32 }; /* clk division */
	int post_div_idx = 0;

	if (g_fpga_flag) {
		/* D PHY Data rate range is from 2500 Mbps to 80 Mbps
		 * The following devil numbers from chip protocol
		 * It contains lots of fixed numbers
		 */
		if ((*lane_clock >= 320) && (*lane_clock <= 2500)) {
			phy_ctrl->rg_band_sel = 0;
			vco_div = 1;  /* clk division */
		} else if ((*lane_clock >= 80) && (*lane_clock < 320)) {
			phy_ctrl->rg_band_sel = 1;
			vco_div = 4; /* clk division */
		} else {
			DPU_FB_ERR("80M <= lane_clock< = 2500M, not support lane_clock = %llu M\n", *lane_clock);
		}

		/* accord chip protocol, lane_clock from MHz to Hz */
		*n_pll = 2;
		*m_pll = (uint32_t)((*lane_clock) * vco_div * (*n_pll) * 1000000UL / DEFAULT_MIPI_CLK_RATE);

		*lane_clock = (*m_pll) * (DEFAULT_MIPI_CLK_RATE / (*n_pll)) / vco_div;
		if (*lane_clock > 750000000)  /* 750MHz */
			phy_ctrl->rg_cp = 3;
		/* 80M <= lane_clock <= 750M */
		else if ((*lane_clock >= 80000000) && (*lane_clock <= 750000000))
			phy_ctrl->rg_cp = 1;
		else
			DPU_FB_ERR("80M <= lane_clock< = 2500M, not support lane_clock = %llu M\n", *lane_clock);

		phy_ctrl->rg_pre_div = *n_pll - 1;
		phy_ctrl->rg_div = *m_pll;
	} else {
		phy_ctrl->rg_pll_prediv = 0;
		vco_clk = (*lane_clock) * post_div[0];
		/* vcc_clk_min and post_div index */
		while ((vco_clk <= VCO_CLK_MIN_VALUE) && (post_div_idx < 5)) {
			post_div_idx++;
			vco_clk = (*lane_clock) * post_div[post_div_idx];
		}
		vco_clk = vco_clk * 1000000; /* MHZ to HZ */
		phy_ctrl->rg_pll_posdiv = post_div_idx;
		phy_ctrl->rg_pll_fbkdiv = vco_clk / DEFAULT_MIPI_CLK_RATE;

		*lane_clock = phy_ctrl->rg_pll_fbkdiv * DEFAULT_MIPI_CLK_RATE / post_div[phy_ctrl->rg_pll_posdiv];
		DPU_FB_INFO("rg_pll_prediv=%d, rg_pll_posdiv=%d, rg_pll_fbkdiv=%d\n",
			phy_ctrl->rg_pll_prediv, phy_ctrl->rg_pll_posdiv, phy_ctrl->rg_pll_fbkdiv);
	}

	/* The following devil numbers from chip protocol */
	phy_ctrl->rg_0p8v = 0;
	phy_ctrl->rg_2p5g = 1;
	phy_ctrl->rg_320m = 0;
	phy_ctrl->rg_lpf_r = 0;
}

static void set_phy_ctrl_data(struct dpu_panel_info *pinfo, struct mipi_dsi_phy_ctrl *phy_ctrl,
	struct mipi_dsi_clk_data clk_data, struct dpu_fb_data_type *dpufd, uint64_t lane_clock)
{
	/* The follow code from chip code, It contains lots of fixed numbers */
	phy_ctrl->clk_pre_delay = round1(clk_data.clk_pre_delay, clk_data.unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_prepare = round1(clk_data.clk_t_hs_prepare, clk_data.unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_lpx = round1(clk_data.clk_t_lpx, clk_data.unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_zero = round1(clk_data.clk_t_hs_zero, clk_data.unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_trial = round1(clk_data.clk_t_hs_trial, clk_data.unit_tx_byte_clk_hs);

	phy_ctrl->data_post_delay = round1(clk_data.data_post_delay, clk_data.unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_prepare = round1(clk_data.data_t_hs_prepare, clk_data.unit_tx_byte_clk_hs);
	phy_ctrl->data_t_lpx = round1(clk_data.data_t_lpx, clk_data.unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_zero = round1(clk_data.data_t_hs_zero, clk_data.unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_trial = round1(clk_data.data_t_hs_trial, clk_data.unit_tx_byte_clk_hs);

	phy_ctrl->clk_post_delay = phy_ctrl->data_t_hs_trial + round1(clk_data.clk_post, clk_data.unit_tx_byte_clk_hs);
	phy_ctrl->data_pre_delay = get_data_pre_delay(pinfo->mipi.lp11_flag,
		phy_ctrl, round1(clk_data.clk_pre, clk_data.unit_tx_byte_clk_hs));

	phy_ctrl->clk_lane_lp2hs_time = phy_ctrl->clk_pre_delay +
		phy_ctrl->clk_t_lpx + phy_ctrl->clk_t_hs_prepare + phy_ctrl->clk_t_hs_zero + 5 + 7;
	phy_ctrl->clk_lane_hs2lp_time = phy_ctrl->clk_t_hs_trial + phy_ctrl->clk_post_delay + 8 + 4;
	phy_ctrl->data_lane_lp2hs_time =
		get_data_pre_delay_reality(pinfo->mipi.lp11_flag, phy_ctrl) +
		phy_ctrl->data_t_lpx + phy_ctrl->data_t_hs_prepare +
		phy_ctrl->data_t_hs_zero + pinfo->mipi.data_lane_lp2hs_time_adjust + 7;

	phy_ctrl->data_lane_hs2lp_time = phy_ctrl->data_t_hs_trial + 8 + 5;

	phy_ctrl->phy_stop_wait_time =
		get_clk_post_delay_reality(pinfo->mipi.lp11_flag, phy_ctrl) +
		phy_ctrl->clk_t_hs_trial + round1(clk_data.clk_t_hs_exit, clk_data.unit_tx_byte_clk_hs) -
		(phy_ctrl->data_post_delay + 4 + phy_ctrl->data_t_hs_trial) + 3;

	phy_ctrl->lane_byte_clk = lane_clock / 8;
	phy_ctrl->clk_division =
		(((phy_ctrl->lane_byte_clk / 2) % pinfo->mipi.max_tx_esc_clk) > 0) ?
		(uint32_t)(phy_ctrl->lane_byte_clk / 2 / pinfo->mipi.max_tx_esc_clk + 1) :
		(uint32_t)(phy_ctrl->lane_byte_clk / 2 / pinfo->mipi.max_tx_esc_clk);

	phy_ctrl->lane_byte_clk_default = get_default_lane_byte_clk(dpufd);
}

static void mipi_dsi_clk_data_lane_dphy_config(struct dpu_panel_info *pinfo,
	struct mipi_dsi_phy_ctrl *phy_ctrl, struct dpu_fb_data_type *dpufd, uint64_t lane_clock)
{
	struct mipi_dsi_clk_data clk_data = {0};

	/******************  clock/data lane parameters config  ******************/
	dpu_check_and_no_retval((lane_clock == 0), ERR, "lane_clock is zero\n");

	clk_data.accuracy = 10;  /* magnification */
	clk_data.ui =  (uint32_t)(10 * 1000000000UL * clk_data.accuracy / lane_clock);
	/* unit of measurement */
	clk_data.unit_tx_byte_clk_hs = 8 * clk_data.ui;
	dpu_check_and_no_retval((clk_data.unit_tx_byte_clk_hs == 0), ERR, "unit_tx_byte_clk_hs is zero\n");
	/* D-PHY Specification : 60ns + 52*UI <= clk_post */
	clk_data.clk_post = 600 * clk_data.accuracy + 52 * clk_data.ui + clk_data.unit_tx_byte_clk_hs +
		pinfo->mipi.clk_post_adjust * clk_data.ui;

	/* D-PHY Specification : clk_pre >= 8*UI */
	clk_data.clk_pre = 8 * clk_data.ui + clk_data.unit_tx_byte_clk_hs + pinfo->mipi.clk_pre_adjust * clk_data.ui;

	/* D-PHY Specification : clk_t_hs_exit >= 100ns */
	clk_data.clk_t_hs_exit = (uint32_t)(1000 * clk_data.accuracy + 100 * clk_data.accuracy +
		pinfo->mipi.clk_t_hs_exit_adjust * clk_data.ui);

	/* clocked by TXBYTECLKHS */
	clk_data.clk_pre_delay = 0 + pinfo->mipi.clk_pre_delay_adjust * clk_data.ui;

	/* D-PHY Specification : clk_t_hs_trial >= 60ns clocked by TXBYTECLKHS */
	clk_data.clk_t_hs_trial = 600 * clk_data.accuracy + 3 * clk_data.unit_tx_byte_clk_hs +
		pinfo->mipi.clk_t_hs_trial_adjust * clk_data.ui;

	/* D-PHY Specification : 38ns <= clk_t_hs_prepare <= 95ns clocked by TXBYTECLKHS */
	clk_data.clk_t_hs_prepare = 660 * clk_data.accuracy;

	/* clocked by TXBYTECLKHS */
	clk_data.data_post_delay = 0 + pinfo->mipi.data_post_delay_adjust * clk_data.ui;

	/*
	 * D-PHY Specification : data_t_hs_trial >= max( n*8*UI, 60ns + n*4*UI ),
	 * n = 1. clocked by TXBYTECLKHS
	 */
	clk_data.data_t_hs_trial = ((600 * clk_data.accuracy + 4 * clk_data.ui) >= (8 * clk_data.ui) ?
		(600 * clk_data.accuracy + 4 * clk_data.ui) : (8 * clk_data.ui)) + 8 * clk_data.ui +
		3 * clk_data.unit_tx_byte_clk_hs + pinfo->mipi.data_t_hs_trial_adjust * clk_data.ui;

	/*
	 * D-PHY Specification : 40ns + 4*UI <= data_t_hs_prepare <= 85ns + 6*UI
	 * clocked by TXBYTECLKHS
	 */
	clk_data.data_t_hs_prepare = get_data_t_hs_prepare(dpufd, clk_data.accuracy, clk_data.ui);
	/*
	 * D-PHY chip spec : clk_t_lpx + clk_t_hs_prepare > 200ns
	 * D-PHY Specification : clk_t_lpx >= 50ns
	 * clocked by TXBYTECLKHS
	 */
	clk_data.clk_t_lpx = (uint32_t)(2000 * clk_data.accuracy + 10 * clk_data.accuracy +
		pinfo->mipi.clk_t_lpx_adjust * clk_data.ui - clk_data.clk_t_hs_prepare);
	/*
	 * D-PHY Specification : clk_t_hs_zero + clk_t_hs_prepare >= 300 ns
	 * clocked by TXBYTECLKHS
	 */
	clk_data.clk_t_hs_zero = (uint32_t)(3000 * clk_data.accuracy + 3 * clk_data.unit_tx_byte_clk_hs +
		pinfo->mipi.clk_t_hs_zero_adjust * clk_data.ui - clk_data.clk_t_hs_prepare);
	/*
	 * D-PHY chip spec : data_t_lpx + data_t_hs_prepare > 200ns
	 * D-PHY Specification : data_t_lpx >= 50ns
	 * clocked by TXBYTECLKHS
	 */
	clk_data.data_t_lpx = (uint32_t)(2000 * clk_data.accuracy + 10 * clk_data.accuracy +
		pinfo->mipi.data_t_lpx_adjust * clk_data.ui - clk_data.data_t_hs_prepare);
	/*
	 * D-PHY Specification : data_t_hs_zero + data_t_hs_prepare >= 145ns + 10*UI
	 * clocked by TXBYTECLKHS
	 */
	clk_data.data_t_hs_zero = (uint32_t)(1450 * clk_data.accuracy + 10 * clk_data.ui +
		3 * clk_data.unit_tx_byte_clk_hs + pinfo->mipi.data_t_hs_zero_adjust * clk_data.ui -
		clk_data.data_t_hs_prepare);

	set_phy_ctrl_data(pinfo, phy_ctrl, clk_data, dpufd, lane_clock);
}

void get_dsi_dphy_ctrl(struct dpu_fb_data_type *dpufd, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	struct dpu_panel_info *pinfo = NULL;

#if defined(CONFIG_HISI_DISPLAY_DFR)
	struct mipi_panel_info *mipi = NULL;
#endif
	uint32_t m_pll = 0;
	uint32_t n_pll = 0;
	uint32_t dsi_bit_clk;
	uint64_t lane_clock;

	pinfo = &(dpufd->panel_info);

#if defined(CONFIG_HISI_DISPLAY_DFR)
	mipi = get_mipi_ctrl(dpufd);
	dsi_bit_clk = mipi->dsi_bit_clk_upt;
#else
	dsi_bit_clk = pinfo->mipi.dsi_bit_clk_upt;
#endif

	lane_clock = (uint64_t)(2 * dsi_bit_clk);
	DPU_FB_INFO("Expected : lane_clock = %llu M\n", lane_clock);

	/************************  PLL parameters config  *********************/
	/* chip spec :
	 * If the output data rate is below 320 Mbps, RG_BNAD_SEL should be set to 1.
	 * At this mode a post divider of 1/4 will be applied to VCO.
	 */
	mipi_dsi_pll_dphy_config(phy_ctrl, &lane_clock, &m_pll, &n_pll);

	/* HSTX select VCM VREF */
	phy_ctrl->rg_vrefsel_vcm = 0;
	if (pinfo->mipi.rg_vrefsel_vcm_adjust != 0) {
		phy_ctrl->rg_vrefsel_vcm = pinfo->mipi.rg_vrefsel_vcm_adjust;
		DPU_FB_INFO("rg_vrefsel_vcm=0x%x\n", phy_ctrl->rg_vrefsel_vcm);
	}

	if (pinfo->mipi.rg_lptx_sri_adjust != 0)
		phy_ctrl->rg_lptx_sri = pinfo->mipi.rg_lptx_sri_adjust;

	if (pinfo->mipi.rg_vrefsel_lptx_adjust != 0)
		phy_ctrl->rg_vrefsel_lptx = pinfo->mipi.rg_vrefsel_lptx_adjust;

	mipi_dsi_clk_data_lane_dphy_config(pinfo, phy_ctrl, dpufd, lane_clock);

	DPU_FB_INFO("DPHY clock_lane and data_lane config :\n"
		"lane_clock = %llu, n_pll=%u, m_pll=%u\n"
		"rg_cp=%u\n"
		"rg_band_sel=%u\n"
		"rg_vrefsel_vcm=%u\n"
		"clk_pre_delay=%u\n"
		"clk_post_delay=%u\n"
		"clk_t_hs_prepare=%u\n"
		"clk_t_lpx=%u\n"
		"clk_t_hs_zero=%u\n"
		"clk_t_hs_trial=%u\n"
		"data_pre_delay=%u\n"
		"data_post_delay=%u\n"
		"data_t_hs_prepare=%u\n"
		"data_t_lpx=%u\n"
		"data_t_hs_zero=%u\n"
		"data_t_hs_trial=%u\n"
		"clk_lane_lp2hs_time=%u\n"
		"clk_lane_hs2lp_time=%u\n"
		"data_lane_lp2hs_time=%u\n"
		"data_lane_hs2lp_time=%u\n"
		"phy_stop_wait_time=%u\n",
		lane_clock, n_pll, m_pll,
		pinfo->dsi_phy_ctrl.rg_cp,
		pinfo->dsi_phy_ctrl.rg_band_sel,
		phy_ctrl->rg_vrefsel_vcm,
		phy_ctrl->clk_pre_delay,
		phy_ctrl->clk_post_delay,
		phy_ctrl->clk_t_hs_prepare,
		phy_ctrl->clk_t_lpx,
		phy_ctrl->clk_t_hs_zero,
		phy_ctrl->clk_t_hs_trial,
		phy_ctrl->data_pre_delay,
		phy_ctrl->data_post_delay,
		phy_ctrl->data_t_hs_prepare,
		phy_ctrl->data_t_lpx,
		phy_ctrl->data_t_hs_zero,
		phy_ctrl->data_t_hs_trial,
		phy_ctrl->clk_lane_lp2hs_time,
		phy_ctrl->clk_lane_hs2lp_time,
		phy_ctrl->data_lane_lp2hs_time,
		phy_ctrl->data_lane_hs2lp_time,
		phy_ctrl->phy_stop_wait_time);
}

static void mipi_dsi_get_cphy_div(struct mipi_dsi_phy_ctrl *phy_ctrl, uint64_t lane_clock, uint64_t *vco_div)
{
	/* C PHY Data rate range is from 1500 Mbps to 40 Mbps
	 * The following devil numbers from chip protocol
	 * It contains lots of fixed numbers
	 */
	if ((lane_clock >= 320) && (lane_clock <= 1500)) {
		phy_ctrl->rg_cphy_div = 0;
		*vco_div = 1;  /* clk division */
	} else if ((lane_clock >= 160) && (lane_clock < 320)) {
		phy_ctrl->rg_cphy_div = 1;
		*vco_div = 2;  /* clk division */
	} else if ((lane_clock >= 80) && (lane_clock < 160)) {
		phy_ctrl->rg_cphy_div = 2;
		*vco_div = 4;  /* clk division */
	} else if ((lane_clock >= 40) && (lane_clock < 80)) {
		phy_ctrl->rg_cphy_div = 3;
		*vco_div = 8;  /* clk division */
	} else {
		DPU_FB_ERR("40M <= lane_clock< = 1500M, not support lane_clock = %llu M\n", lane_clock);
	}
}

static void mipi_dsi_pll_cphy_config(struct mipi_dsi_phy_ctrl *phy_ctrl, uint64_t *lane_clock,
	uint32_t *m_pll, uint32_t *n_pll)
{
	uint64_t vco_div = 1;  /* default clk division */
	uint64_t vco_clk;
	uint32_t post_div[6] = { 1, 2, 4, 8, 16, 32 };  /* clk division */
	int post_div_idx = 0;

	if (g_fpga_flag) {
		mipi_dsi_get_cphy_div(phy_ctrl, *lane_clock, &vco_div);

		/* accord chip protocol, lane_clock from MHz to Hz */
		*n_pll = 2;
		*m_pll = (uint32_t)((*lane_clock) * vco_div * (*n_pll) * 1000000UL / DEFAULT_MIPI_CLK_RATE);
		if (vco_div)
			*lane_clock = (*m_pll) * (DEFAULT_MIPI_CLK_RATE / (*n_pll)) / vco_div;
		if (*lane_clock > 750000000)  /* 750Mhz */
			phy_ctrl->rg_cp = 3;
		/* 40M <= lane_clock <= 750M */
		else if ((*lane_clock >= 40000000) && (*lane_clock <= 750000000))
			phy_ctrl->rg_cp = 1;
		else
			DPU_FB_ERR("40M <= lane_clock< = 1500M, not support lane_clock = %llu M\n", *lane_clock);

		phy_ctrl->rg_pre_div = *n_pll - 1;
		phy_ctrl->rg_div = *m_pll;
	} else {
		phy_ctrl->rg_pll_prediv = 0;
		vco_clk = (*lane_clock) * post_div[0];
		while ((vco_clk <= VCO_CLK_MIN_VALUE) && (post_div_idx < 5)) {
			post_div_idx++;
			vco_clk = (*lane_clock) * post_div[post_div_idx];
		}
		vco_clk = vco_clk * 1000000;  /* MHz to Hz */
		phy_ctrl->rg_pll_posdiv = post_div_idx;
		phy_ctrl->rg_pll_fbkdiv = vco_clk / DEFAULT_MIPI_CLK_RATE;
		*lane_clock = phy_ctrl->rg_pll_fbkdiv * DEFAULT_MIPI_CLK_RATE /
			post_div[phy_ctrl->rg_pll_posdiv];
		DPU_FB_INFO("rg_pll_prediv=%d, rg_pll_posdiv=%d, rg_pll_fbkdiv=%d\n",
			phy_ctrl->rg_pll_prediv, phy_ctrl->rg_pll_posdiv,
			phy_ctrl->rg_pll_fbkdiv);
	}

	/* The following devil numbers from chip protocol */
	phy_ctrl->rg_0p8v = 0;
	phy_ctrl->rg_2p5g = 1;
	phy_ctrl->rg_320m = 0;
	phy_ctrl->rg_lpf_r = 0;
}

static void mipi_dsi_clk_data_lane_cphy_config(struct dpu_panel_info *pinfo,
	struct mipi_dsi_phy_ctrl *phy_ctrl, struct dpu_fb_data_type *dpufd, uint64_t lane_clock)
{
	uint32_t accuracy;
	uint32_t ui;
	uint32_t unit_tx_word_clk_hs;

	/********************  data lane parameters config  ******************/
	dpu_check_and_no_retval((lane_clock == 0), ERR, "lane_clock is zero\n");
	accuracy = 10;  /* magnification */
	ui = (uint32_t)(10 * 1000000000UL * accuracy / lane_clock);
	/* unit of measurement */
	unit_tx_word_clk_hs = 7 * ui;
	dpu_check_and_no_retval((unit_tx_word_clk_hs == 0), ERR, "unit_tx_word_clk_hs is zero\n");

	if (pinfo->mipi.mininum_phy_timing_flag == 1) {
		/* CPHY Specification: 38ns <= t3_prepare <= 95ns */
		phy_ctrl->t_prepare = MIN_T3_PREPARE_PARAM * accuracy +
				pinfo->mipi.mipi_cphy_adjust.cphy_data_t_prepare_adjust;

		/* CPHY Specification: 50ns <= t_lpx */
		phy_ctrl->t_lpx = MIN_T3_LPX_PARAM * accuracy + 8 * ui - unit_tx_word_clk_hs +
				pinfo->mipi.mipi_cphy_adjust.cphy_data_t_lpx_adjust;

		/* CPHY Specification: 7*UI <= t_prebegin <= 448UI */
		phy_ctrl->t_prebegin = MIN_T3_PREBEGIN_PARAM * ui - unit_tx_word_clk_hs +
				pinfo->mipi.mipi_cphy_adjust.cphy_data_t_prebegin_adjust;

		/* CPHY Specification: 7*UI <= t_post <= 224*UI */
		phy_ctrl->t_post = MIN_T3_POST_PARAM * ui - unit_tx_word_clk_hs +
				pinfo->mipi.mipi_cphy_adjust.cphy_data_t_post_adjust;
	} else {
		/* CPHY Specification: 38ns <= t3_prepare <= 95ns */
		/* 380 * accuracy - unit_tx_word_clk_hs; */
		phy_ctrl->t_prepare = T3_PREPARE_PARAM * accuracy +
				pinfo->mipi.mipi_cphy_adjust.cphy_data_t_prepare_adjust;

		/* CPHY Specification: 50ns <= t_lpx */
		phy_ctrl->t_lpx =  T3_LPX_PARAM * accuracy + 8 * ui - unit_tx_word_clk_hs +
				pinfo->mipi.mipi_cphy_adjust.cphy_data_t_lpx_adjust;

		/* CPHY Specification: 7*UI <= t_prebegin <= 448UI */
		phy_ctrl->t_prebegin =  T3_PREBEGIN_PARAM * ui - unit_tx_word_clk_hs +
				pinfo->mipi.mipi_cphy_adjust.cphy_data_t_prebegin_adjust;

		/* CPHY Specification: 7*UI <= t_post <= 224*UI */
		phy_ctrl->t_post = T3_POST_PARAM * ui - unit_tx_word_clk_hs +
				pinfo->mipi.mipi_cphy_adjust.cphy_data_t_post_adjust;
	}

	/* The follow code from chip code, It contains lots of fixed numbers */
	phy_ctrl->t_prepare = round1(phy_ctrl->t_prepare, unit_tx_word_clk_hs);
	phy_ctrl->t_lpx = round1(phy_ctrl->t_lpx, unit_tx_word_clk_hs);
	phy_ctrl->t_prebegin = round1(phy_ctrl->t_prebegin, unit_tx_word_clk_hs);
	phy_ctrl->t_post = round1(phy_ctrl->t_post, unit_tx_word_clk_hs);

	phy_ctrl->data_lane_lp2hs_time = phy_ctrl->t_lpx + phy_ctrl->t_prepare +
		phy_ctrl->t_prebegin + 5 + 17;
	phy_ctrl->data_lane_hs2lp_time = phy_ctrl->t_post + 8 + 5;

	phy_ctrl->lane_word_clk = lane_clock / 7;
	phy_ctrl->clk_division =
		(((phy_ctrl->lane_word_clk / 2) % pinfo->mipi.max_tx_esc_clk) > 0) ?
		(uint32_t)(phy_ctrl->lane_word_clk / 2 / pinfo->mipi.max_tx_esc_clk + 1) :
		(uint32_t)(phy_ctrl->lane_word_clk / 2 / pinfo->mipi.max_tx_esc_clk);

	phy_ctrl->phy_stop_wait_time = phy_ctrl->t_post + 8 + 5;
	phy_ctrl->lane_byte_clk_default = get_default_lane_byte_clk(dpufd);
}

void get_dsi_cphy_ctrl(struct dpu_fb_data_type *dpufd, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	struct dpu_panel_info *pinfo = NULL;
#if defined(CONFIG_HISI_DISPLAY_DFR)
	struct mipi_panel_info *mipi = NULL;
#endif
	uint32_t m_pll = 0;
	uint32_t n_pll = 0;
	uint64_t lane_clock;

	pinfo = &(dpufd->panel_info);

#if defined(CONFIG_HISI_DISPLAY_DFR)
	mipi = get_mipi_ctrl(dpufd);
	lane_clock = mipi->dsi_bit_clk_upt;
#else
	lane_clock = pinfo->mipi.dsi_bit_clk_upt;
#endif

	DPU_FB_INFO("Expected : lane_clock = %llu M\n", lane_clock);

	/************************  PLL parameters config  *********************/
	mipi_dsi_pll_cphy_config(phy_ctrl, &lane_clock, &m_pll, &n_pll);

	/* HSTX select VCM VREF */
	phy_ctrl->rg_vrefsel_vcm = 0x51;

	mipi_dsi_clk_data_lane_cphy_config(pinfo, phy_ctrl, dpufd, lane_clock);

	DPU_FB_INFO("CPHY clock_lane and data_lane config :\n"
		"lane_clock=%llu, n_pll=%u, m_pll=%u\n"
		"rg_cphy_div=%u\n"
		"rg_cp=%u\n"
		"rg_vrefsel_vcm=%u\n"
		"t_prepare=%u\n"
		"t_lpx=%u\n"
		"t_prebegin=%u\n"
		"t_post=%u\n"
		"lane_word_clk=%llu\n"
		"data_lane_lp2hs_time=%u\n"
		"data_lane_hs2lp_time=%u\n"
		"clk_division=%u\n"
		"phy_stop_wait_time=%u\n",
		lane_clock, n_pll, m_pll,
		phy_ctrl->rg_cphy_div,
		phy_ctrl->rg_cp,
		phy_ctrl->rg_vrefsel_vcm,
		phy_ctrl->t_prepare,
		phy_ctrl->t_lpx,
		phy_ctrl->t_prebegin,
		phy_ctrl->t_post,
		phy_ctrl->lane_word_clk,
		phy_ctrl->data_lane_lp2hs_time,
		phy_ctrl->data_lane_hs2lp_time,
		phy_ctrl->clk_division,
		phy_ctrl->phy_stop_wait_time);
}

static uint32_t mipi_pixel_clk(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);
	if ((pinfo->pxl_clk_rate_div == 0) || (g_fpga_flag == 1))
		return (uint32_t)pinfo->pxl_clk_rate;

	if ((pinfo->ifbc_type == IFBC_TYPE_NONE) && !is_dual_mipi_panel(dpufd))
		pinfo->pxl_clk_rate_div = 1;

	return (uint32_t)pinfo->pxl_clk_rate / pinfo->pxl_clk_rate_div;
}

void mipi_config_phy_test_code(char __iomem *mipi_dsi_base,
	uint32_t test_code_addr, uint32_t test_code_parameter)
{
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL1_OFFSET, test_code_addr);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000002);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL1_OFFSET, test_code_parameter);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000002);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);
}

void mipi_config_cphy_spec1v0_parameter(char __iomem *mipi_dsi_base,
	struct dpu_panel_info *pinfo, const struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	uint32_t i;
	uint32_t addr;

	for (i = 0; i <= pinfo->mipi.lane_nums; i++) {
		if (g_fpga_flag) {
			/* Lane Transmission Property */
			addr = MIPIDSI_PHY_TST_LANE_TRANSMISSION_PROPERTY + (i << 5);
			mipi_config_phy_test_code(mipi_dsi_base, addr, 0x43);
		}

		/* Lane Timing Control - DPHY: THS-PREPARE/CPHY: T3-PREPARE */
		addr = MIPIDSI_PHY_TST_DATA_PREPARE + (i << 5);

		mipi_config_phy_test_code(mipi_dsi_base, addr, dss_reduce(phy_ctrl->t_prepare));

		/* Lane Timing Control - TLPX */
		addr = MIPIDSI_PHY_TST_DATA_TLPX + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, dss_reduce(phy_ctrl->t_lpx));
	}

	if (pinfo->mipi.mipi_cphy_adjust.need_adjust_cphy_para)
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010002, dss_reduce(phy_ctrl->t_post));
}

void mipi_config_dphy_spec1v2_parameter(char __iomem *mipi_dsi_base,
	struct dpu_panel_info *pinfo, const struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	uint32_t i;
	uint32_t addr;

	for (i = 0; i <= (pinfo->mipi.lane_nums + 1); i++) {
		/* Lane Transmission Property */
		addr = MIPIDSI_PHY_TST_LANE_TRANSMISSION_PROPERTY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, 0x43);
	}

	/* pre_delay of clock lane request setting */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_PRE_DELAY,
		dss_reduce(phy_ctrl->clk_pre_delay));

	/* post_delay of clock lane request setting */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_POST_DELAY,
		dss_reduce(phy_ctrl->clk_post_delay));

	/* clock lane timing ctrl - t_lpx */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_TLPX,
		dss_reduce(phy_ctrl->clk_t_lpx));

	/* clock lane timing ctrl - t_hs_prepare */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_PREPARE,
		dss_reduce(phy_ctrl->clk_t_hs_prepare));

	/* clock lane timing ctrl - t_hs_zero */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_ZERO,
		dss_reduce(phy_ctrl->clk_t_hs_zero));

	/* clock lane timing ctrl - t_hs_trial */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_TRAIL,
		dss_reduce(phy_ctrl->clk_t_hs_trial));

	for (i = 0; i <= (pinfo->mipi.lane_nums + 1); i++) {
		if (i == 2)
			i++;  /* addr: lane0:0x60; lane1:0x80; lane2:0xC0; lane3:0xE0 */

		/* data lane pre_delay */
		addr = MIPIDSI_PHY_TST_DATA_PRE_DELAY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, dss_reduce(phy_ctrl->data_pre_delay));

		/* data lane post_delay */
		addr = MIPIDSI_PHY_TST_DATA_POST_DELAY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, dss_reduce(phy_ctrl->data_post_delay));

		/* data lane timing ctrl - t_lpx */
		addr = MIPIDSI_PHY_TST_DATA_TLPX + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, dss_reduce(phy_ctrl->data_t_lpx));

		/* data lane timing ctrl - t_hs_prepare */
		addr = MIPIDSI_PHY_TST_DATA_PREPARE + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, dss_reduce(phy_ctrl->data_t_hs_prepare));

		/* data lane timing ctrl - t_hs_zero */
		addr = MIPIDSI_PHY_TST_DATA_ZERO + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, dss_reduce(phy_ctrl->data_t_hs_zero));

		/* data lane timing ctrl - t_hs_trial */
		addr = MIPIDSI_PHY_TST_DATA_TRAIL + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, dss_reduce(phy_ctrl->data_t_hs_trial));


		DPU_FB_INFO("DPHY spec1v2 config :\n"
			"addr=0x%x\n"
			"clk_pre_delay=%u\n"
			"clk_t_hs_trial=%u\n"
			"data_t_hs_zero=%u\n"
			"data_t_lpx=%u\n"
			"data_t_hs_prepare=%u\n",
			addr,
			phy_ctrl->clk_pre_delay,
			phy_ctrl->clk_t_hs_trial,
			phy_ctrl->data_t_hs_zero,
			phy_ctrl->data_t_lpx,
			phy_ctrl->data_t_hs_prepare);
	}
}

static void mipi_cdphy_PLL_configuration(char __iomem *mipi_dsi_base, struct dpu_panel_info *pinfo,
	uint32_t rg_cphy_div_param)
{
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010042, 0x21);
		if (g_fpga_flag) {
			/* PLL configuration I */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010046,
				pinfo->dsi_phy_ctrl.rg_cp + (pinfo->dsi_phy_ctrl.rg_lpf_r << 4));

			/* PLL configuration II */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010048,
				pinfo->dsi_phy_ctrl.rg_0p8v +
				(pinfo->dsi_phy_ctrl.rg_2p5g << 1) +
				(pinfo->dsi_phy_ctrl.rg_320m << 2) +
				(pinfo->dsi_phy_ctrl.rg_band_sel << 3) +
				rg_cphy_div_param);

			/* PLL configuration III */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010049,
				pinfo->dsi_phy_ctrl.rg_pre_div);

			/* PLL configuration IV */
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A,
				pinfo->dsi_phy_ctrl.rg_div);
		} else {
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010049,
				(pinfo->dsi_phy_ctrl.rg_pll_posdiv << 4) |
				pinfo->dsi_phy_ctrl.rg_pll_prediv);
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A,
				pinfo->dsi_phy_ctrl.rg_pll_fbkdiv);
		}
}

static void mipi_cdphy_init_config(char __iomem *mipi_dsi_base, struct dpu_panel_info *pinfo)
{
	int i;

	if (pinfo->mipi.phy_mode == CPHY_MODE) {
		if (pinfo->mipi.mininum_phy_timing_flag == 1) {
			/* T3-PREBEGIN */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010001, MIN_T3_PREBEGIN_PHY_TIMING);
			/* T3-POST */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010002, MIN_T3_POST_PHY_TIMING);
		} else {
			/* T3-PREBEGIN */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010001, T3_PREBEGIN_PHY_TIMING);
			/* T3-POST */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010002, T3_POST_PHY_TIMING);
		}

		mipi_cdphy_PLL_configuration(mipi_dsi_base, pinfo, (pinfo->dsi_phy_ctrl.rg_cphy_div << 4));

		if (g_fpga_flag) {
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004F, 0xf0);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010052, 0xa8);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010053, 0xc2);
		}

		mipi_config_phy_test_code(mipi_dsi_base, 0x00010058,
			(0x4 + pinfo->mipi.lane_nums) << 4 | 0);
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001005B, 0x19);
		/* PLL update control */
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);

		/* set cphy spec parameter */
		mipi_config_cphy_spec1v0_parameter(mipi_dsi_base, pinfo, &pinfo->dsi_phy_ctrl);
	} else {
		mipi_cdphy_PLL_configuration(mipi_dsi_base, pinfo, 0);

		if (g_fpga_flag) {
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004F, 0xf0);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010050, 0xc0);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010051, 0x22);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010053,
				pinfo->dsi_phy_ctrl.rg_vrefsel_vcm);
		}
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001005B,
			pinfo->dsi_phy_ctrl.rg_vrefsel_vcm);
		if (pinfo->mipi.support_de_emphasis) {
			for (i = 0; i < DE_EMPHASIS_REG_NUM; i++) {
				mipi_config_phy_test_code(mipi_dsi_base, pinfo->mipi.de_emphasis_reg[i],
					pinfo->mipi.de_emphasis_value[i]);
			}
		}
		/* PLL update control */
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);

		/* set dphy spec parameter */
		mipi_config_dphy_spec1v2_parameter(mipi_dsi_base, pinfo, &pinfo->dsi_phy_ctrl);
	}
	if (pinfo->need_adjust_dsi_vcm) {
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010050, pinfo->dsi_phy_ctrl.rg_vrefsel_vcm);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010051, pinfo->dsi_phy_ctrl.rg_vrefsel_lptx);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010052, pinfo->dsi_phy_ctrl.rg_lptx_sri);
	}
}

static uint32_t mipi_get_cmp_stopstate_value(struct dpu_panel_info *pinfo)
{
	uint32_t cmp_stopstate_val;

	if (pinfo->mipi.lane_nums >= DSI_4_LANES)
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9) | BIT(11));
	else if (pinfo->mipi.lane_nums >= DSI_3_LANES)
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9));
	else if (pinfo->mipi.lane_nums >= DSI_2_LANES)
		cmp_stopstate_val = (BIT(4) | BIT(7));
	else
		cmp_stopstate_val = (BIT(4));

	return cmp_stopstate_val;
}

static bool mipi_phy_status_check(const char __iomem *mipi_dsi_base, uint32_t expected_value)
{
	bool is_ready = false;
	uint32_t temp;
	unsigned long dw_jiffies;

	dw_jiffies = jiffies + HZ / 2;  /* HZ / 2 = 0.5s */
	do {
		temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((temp & expected_value) == expected_value) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	DPU_FB_DEBUG("MIPIDSI_PHY_STATUS_OFFSET=0x%x.\n", temp);

	return is_ready;
}

static void get_mipi_dsi_timing(struct dpu_fb_data_type *dpufd)
{
	dss_rect_t rect;
	uint64_t pixel_clk;
	uint64_t lane_byte_clk;
	struct dpu_panel_info *pinfo;

	pinfo = &(dpufd->panel_info);

	rect.x = 0;
	rect.y = 0;
	rect.w = pinfo->xres;
	rect.h = pinfo->yres;
	mipi_ifbc_get_rect(dpufd, &rect);

	pinfo->mipi.width = rect.w;
	pinfo->mipi.vactive_line = pinfo->yres;
#ifdef CONFIG_HISI_DISPLAY_DFR
	pinfo->mipi.porch_ratio = get_porch_ratio(dpufd);
#endif
	if (pinfo->mipi.dsi_timing_support)
		return;

	pixel_clk = mipi_pixel_clk(dpufd);
	if (pixel_clk == 0)
		return;

	lane_byte_clk = (pinfo->mipi.phy_mode == DPHY_MODE) ?
		pinfo->dsi_phy_ctrl.lane_byte_clk : pinfo->dsi_phy_ctrl.lane_word_clk;

	pinfo->mipi.hsa = round1(pinfo->ldi.h_pulse_width * lane_byte_clk, pixel_clk);
	pinfo->mipi.hbp = round1(pinfo->ldi.h_back_porch * lane_byte_clk, pixel_clk);
	pinfo->mipi.hline_time = round1((pinfo->ldi.h_pulse_width +
		pinfo->ldi.h_back_porch + pinfo->mipi.width +
		pinfo->ldi.h_front_porch) * lane_byte_clk, pixel_clk);
	pinfo->mipi.dpi_hsize = round1(pinfo->mipi.width * lane_byte_clk, pixel_clk);

	pinfo->mipi.vsa = pinfo->ldi.v_pulse_width;
	pinfo->mipi.vbp = pinfo->ldi.v_back_porch;
	pinfo->mipi.vfp = pinfo->ldi.v_front_porch;

	DPU_FB_DEBUG("lane_byte_clk_default %llu M, htiming: %d, %d, %d, %d\n",
		pinfo->dsi_phy_ctrl.lane_byte_clk_default, pinfo->mipi.hsa,
		pinfo->mipi.hbp, pinfo->mipi.hline_time, pinfo->mipi.dpi_hsize);
}

uint32_t get_mipi_timing_hline_time(struct dpu_fb_data_type *dpufd, uint32_t object_hline_time)
{
	uint32_t tmp_hline_time;
	uint64_t lane_byte_clk;

	struct dpu_panel_info *pinfo = &(dpufd->panel_info);

	lane_byte_clk = (pinfo->mipi.phy_mode == DPHY_MODE) ?
		pinfo->dsi_phy_ctrl.lane_byte_clk : pinfo->dsi_phy_ctrl.lane_word_clk;
	if (lane_byte_clk == pinfo->dsi_phy_ctrl.lane_byte_clk_default) {
		tmp_hline_time = object_hline_time;  /* pinfo->mipi.hline_time; */
	} else {
		if (pinfo->dsi_phy_ctrl.lane_byte_clk_default == 0) {
			pinfo->dsi_phy_ctrl.lane_byte_clk_default = get_default_lane_byte_clk(dpufd);
			DPU_FB_ERR("change lane_byte_clk_default to %llu M\n",
				pinfo->dsi_phy_ctrl.lane_byte_clk_default);
		}

		tmp_hline_time = (uint32_t)round1(object_hline_time * lane_byte_clk,
			pinfo->dsi_phy_ctrl.lane_byte_clk_default);
	}

	DPU_FB_INFO("hline_time = %d\n", tmp_hline_time);

	return tmp_hline_time;
}

static void get_mipi_dsi_timing_config_para(
	struct dpu_fb_data_type *dpufd, struct mipi_dsi_phy_ctrl *phy_ctrl,
	struct mipi_dsi_timing *timing)
{
	uint32_t tmp_hline_time;
	uint64_t lane_byte_clk;
	struct dpu_panel_info *pinfo;

	pinfo = &(dpufd->panel_info);
	dpu_check_and_no_retval((!timing || !pinfo), ERR, "timing or pinfo is NULL\n");

	if (is_lcd_dfr_support(dpufd)) {
		if (dpufd->panel_info.fps == FPS_60HZ)
			tmp_hline_time = pinfo->mipi.hline_time;
		else
			tmp_hline_time = dpufd->panel_info.mipi_updt.hline_time;
	} else {
		tmp_hline_time = pinfo->mipi.hline_time;
	}

	lane_byte_clk = (pinfo->mipi.phy_mode == DPHY_MODE) ? phy_ctrl->lane_byte_clk : phy_ctrl->lane_word_clk;

	if (lane_byte_clk == pinfo->dsi_phy_ctrl.lane_byte_clk_default) {
		timing->hsa = pinfo->mipi.hsa;
		timing->hbp = pinfo->mipi.hbp;
		timing->hline_time = tmp_hline_time;
	} else {
		if (pinfo->dsi_phy_ctrl.lane_byte_clk_default == 0) {
			pinfo->dsi_phy_ctrl.lane_byte_clk_default = get_default_lane_byte_clk(dpufd);
			DPU_FB_ERR("change lane_byte_clk_default to %llu M\n",
				pinfo->dsi_phy_ctrl.lane_byte_clk_default);
		}

		timing->hsa = (uint32_t)round1(pinfo->mipi.hsa * lane_byte_clk,
			pinfo->dsi_phy_ctrl.lane_byte_clk_default);
		timing->hbp = (uint32_t)round1(pinfo->mipi.hbp * lane_byte_clk,
			pinfo->dsi_phy_ctrl.lane_byte_clk_default);
		timing->hline_time = (uint32_t)round1(tmp_hline_time * lane_byte_clk,
			pinfo->dsi_phy_ctrl.lane_byte_clk_default);
	}

	timing->dpi_hsize = pinfo->mipi.dpi_hsize;
	timing->width = pinfo->mipi.width;
	timing->vsa = pinfo->mipi.vsa;
	timing->vbp = pinfo->mipi.vbp;
	timing->vfp = pinfo->mipi.vfp;
	timing->vactive_line = pinfo->mipi.vactive_line;

	if (is_lcd_dfr_support(dpufd))
		pinfo->frm_rate_ctrl.current_hline_time = timing->hline_time;

	DPU_FB_DEBUG("lanebyteclk: %llu M, %llu M, htiming: %d, %d, %d, %d "
		"new: %d, %d, %d, %d\n",
		lane_byte_clk, pinfo->dsi_phy_ctrl.lane_byte_clk_default,
		pinfo->mipi.hsa, pinfo->mipi.hbp, tmp_hline_time,
		pinfo->mipi.dpi_hsize, timing->hsa, timing->hbp, timing->hline_time,
		timing->dpi_hsize);
}

static void cmd_panel_mipi_ldi_init(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base,
	uint64_t lane_byte_clk)
{
	if (is_dual_mipi_panel(dpufd)) {
		if ((dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) ||
			(mipi_dsi_base == dpufd->mipi_dsi0_base) ||
			dpufd->panel_info.need_set_dsi1_te_ctrl)
			set_reg(mipi_dsi_base + MIPI_DSI_TE_CTRL, (0x1 << 17) | (0x1 << 6) | 0x1, 18, 0);
	} else {
		if (need_config_dsi0(dpufd)) {
				set_reg(mipi_dsi_base + MIPI_DSI_TE_CTRL, (0x1 << 17) | (0x1 << 6) | 0x1, 18, 0);
		}

		if (dpufd->panel_info.mipi.dsi_te_type == DSI1_TE1_TYPE) {
			if (dpufd->index == PRIMARY_PANEL_IDX)
				set_reg(mipi_dsi_base + MIPI_DSI_TE_CTRL, (0x1 << 17) | (0x2 << 6) | 0x1, 18, 0);
		}
	}
	set_reg(mipi_dsi_base + MIPI_DSI_TE_HS_NUM, 0x0, 32, 0);
	set_reg(mipi_dsi_base + MIPI_DSI_TE_HS_WD, 0x24024, 32, 0);

	if (dpufd->panel_info.mipi.dsi_te_type == DSI1_TE1_TYPE)
		set_reg(mipi_dsi_base + MIPI_DSI_TE_VS_WD, ((2 * lane_byte_clk / MILLION_CONVERT) << 12) |
			0x3FC, 32, 0);
	else
		set_reg(mipi_dsi_base + MIPI_DSI_TE_VS_WD,
			(0x3FC << 12) | (2 * lane_byte_clk / MILLION_CONVERT), 32, 0);

	set_reg(mipi_dsi_base + VID_SHADOW_CTRL, 0x1, 1, 0);

	/* enable vsync delay when dirty region update */
	set_reg(mipi_dsi_base + MIPI_VSYNC_DELAY_CTRL, 0x2, 2, 0);
	set_reg(mipi_dsi_base + MIPI_VSYNC_DELAY_TIME, VSYNC_DELAY_TIME_DEFAULT, 32, 0);

	/* enable single frame update */
	set_reg(mipi_dsi_base + MIPI_LDI_FRM_MSK, (dpufd->frame_update_flag == 1) ? 0x0 : 0x1, 1, 0);
}

static void mipi_ldi_init(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base)
{
	uint64_t lane_byte_clk;
	struct dpu_panel_info *pinfo = NULL;
	struct mipi_dsi_timing timing;

	pinfo = &(dpufd->panel_info);

	lane_byte_clk = (pinfo->mipi.phy_mode == CPHY_MODE) ?
		pinfo->dsi_phy_ctrl.lane_word_clk : pinfo->dsi_phy_ctrl.lane_byte_clk;

	memset(&timing, 0, sizeof(timing));
	get_mipi_dsi_timing_config_para(dpufd, &(pinfo->dsi_phy_ctrl), &timing);

	set_reg(mipi_dsi_base + MIPI_LDI_DPI0_HRZ_CTRL3, dss_reduce(timing.dpi_hsize), 12, 0);
	set_reg(mipi_dsi_base + MIPI_LDI_DPI0_HRZ_CTRL2, dss_reduce(timing.width), 12, 0);
	set_reg(mipi_dsi_base + MIPI_LDI_VRT_CTRL2, dss_reduce(timing.vactive_line), 12, 0);

	disable_ldi(dpufd);
	if (is_mipi_video_panel(dpufd)) {
		set_reg(mipi_dsi_base + MIPI_LDI_FRM_MSK, 0x0, 1, 0);
		set_reg(mipi_dsi_base + DSI_CMD_MOD_CTRL, 0x1, 1, 1);
	}

	if (is_dual_mipi_panel(dpufd))
		set_reg(mipi_dsi_base + MIPI_LDI_CTRL,
			((mipi_dsi_base == dpufd->mipi_dsi0_base) ? 0 : 1), 1, 13);

	if (is_mipi_cmd_panel(dpufd)) {
		cmd_panel_mipi_ldi_init(dpufd, mipi_dsi_base, lane_byte_clk);
	} else if (is_mipi_video_panel(dpufd) && is_lcd_dfr_support(dpufd)) {
		/* enable vsync delay */
		set_reg(mipi_dsi_base + MIPI_VSYNC_DELAY_CTRL, 0x2, 2, 0);
		/*
		 * If vsync delay is enable, the vsync_delay_count can not be set to 0.
		 * It is recommended to set to a small value to avoid the abnormality of vsync_delay function
		 */
		set_reg(mipi_dsi_base + MIPI_VSYNC_DELAY_TIME, 0x5, 32, 0);
	}
}

static void mipi_dsi_phy_config(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base,
	struct dpu_panel_info *pinfo)
{
	bool is_ready = false;

	/*************************Configure the PHY start*************************/

	set_reg(mipi_dsi_base + MIPIDSI_PHY_IF_CFG_OFFSET, pinfo->mipi.lane_nums, 2, 0);
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_division, 8, 0);
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_division, 8, 8);

	outp32(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x00000000);

	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000001);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);

	mipi_cdphy_init_config(mipi_dsi_base, pinfo);


	outp32(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x0000000F);

	is_ready = mipi_phy_status_check(mipi_dsi_base, 0x01);
	if (!is_ready)
		DPU_FB_INFO("fb%d, phylock is not ready!\n", dpufd->index);

	is_ready = mipi_phy_status_check(mipi_dsi_base,
		mipi_get_cmp_stopstate_value(pinfo));
	if (!is_ready)
		DPU_FB_INFO("fb%d, phystopstateclklane is not ready!\n", dpufd->index);

	/*************************Configure the PHY end*************************/
}

static void mipi_dsi_config_dpi_interface(struct dpu_panel_info *pinfo, char __iomem *mipi_dsi_base)
{
	set_reg(mipi_dsi_base + MIPIDSI_DPI_VCID_OFFSET, pinfo->mipi.vc, 2, 0);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_COLOR_CODING_OFFSET, pinfo->mipi.color_mode, 4, 0);

	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, pinfo->ldi.data_en_plr, 1, 0);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, pinfo->ldi.vsync_plr, 1, 1);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, pinfo->ldi.hsync_plr, 1, 2);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, 0x0, 1, 3);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, 0x0, 1, 4);
}

static void mipi_dsi_video_mode_config(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base,
	struct dpu_panel_info *pinfo, struct mipi_dsi_timing *timing)
{
	/* video mode: low power mode */
	if (pinfo->mipi.lp11_flag == MIPI_DISABLE_LP11)
		set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x0f, 6, 8);
	else
		set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x3f, 6, 8);

	if (is_mipi_video_panel(dpufd)) {
		set_reg(mipi_dsi_base + MIPIDSI_DPI_LP_CMD_TIM_OFFSET, 0x4, 8, 16);
		/* video mode: send read cmd by lp mode */
		set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x1, 1, 15);
	}

	if ((pinfo->mipi.dsi_version == DSI_1_2_VERSION) && (is_mipi_video_panel(dpufd)) &&
		((pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE) || (pinfo->ifbc_type == IFBC_TYPE_VESA3X_DUAL))) {
		set_reg(mipi_dsi_base + MIPIDSI_VID_PKT_SIZE_OFFSET, timing->width * pinfo->pxl_clk_rate_div, 14, 0);
		/* video vase3x must be set BURST mode */
		if (pinfo->mipi.burst_mode < DSI_BURST_SYNC_PULSES_1) {
			DPU_FB_INFO("pinfo->mipi.burst_mode = %d, video need config BURST mode\n",
				pinfo->mipi.burst_mode);
			pinfo->mipi.burst_mode = DSI_BURST_SYNC_PULSES_1;
		}
	} else {
		set_reg(mipi_dsi_base + MIPIDSI_VID_PKT_SIZE_OFFSET, timing->width, 14, 0);
	}

	/* burst mode */
	set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, pinfo->mipi.burst_mode, 2, 0);
}

static void mipi_dsi_horizontal_timing_config(struct mipi_dsi_timing *timing, char __iomem *mipi_dsi_base)
{
	if (timing->hline_time < (timing->hsa + timing->hbp + timing->dpi_hsize))
		DPU_FB_ERR("wrong hfp\n");

	set_reg(mipi_dsi_base + MIPIDSI_VID_HSA_TIME_OFFSET, timing->hsa, 12, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_HBP_TIME_OFFSET, timing->hbp, 12, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_HLINE_TIME_OFFSET, timing->hline_time, 15, 0);

	set_reg(mipi_dsi_base + MIPIDSI_VID_VSA_LINES_OFFSET, timing->vsa, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_VBP_LINES_OFFSET, timing->vbp, 10, 0);
}

static void mipi_dsi_vfp_vsync_config(struct mipi_dsi_timing *timing,  char __iomem *mipi_dsi_base)
{
	uint32_t ldi_vrt_ctrl0;
	uint32_t vsync_delay_cnt;

	if (timing->vfp > V_FRONT_PORCH_MAX) {
		ldi_vrt_ctrl0 = V_FRONT_PORCH_MAX;
		vsync_delay_cnt = (timing->vfp - V_FRONT_PORCH_MAX) * timing->hline_time;
		DPU_FB_WARNING("vfp %d > 1023", timing->vfp);
	} else {
		ldi_vrt_ctrl0 = timing->vfp;
		vsync_delay_cnt = VSYNC_DELAY_TIME_DEFAULT;
	}
	set_reg(mipi_dsi_base + MIPIDSI_VID_VFP_LINES_OFFSET, ldi_vrt_ctrl0, 10, 0);
	set_reg(mipi_dsi_base + MIPI_VSYNC_DELAY_TIME, vsync_delay_cnt, 32, 0);

	set_reg(mipi_dsi_base + MIPIDSI_VID_VACTIVE_LINES_OFFSET, timing->vactive_line, 14, 0);

	set_reg(mipi_dsi_base + MIPIDSI_TO_CNT_CFG_OFFSET, 0x7FF, 16, 0);
}

static void mipi_dsi_phy_timing_config(struct dpu_panel_info *pinfo, char __iomem *mipi_dsi_base)
{
	/* Configure core's phy parameters */
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_lane_lp2hs_time, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_lane_hs2lp_time, 10, 16);

	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_RD_CFG_OFFSET, 0x7FFF, 15, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET, pinfo->dsi_phy_ctrl.data_lane_lp2hs_time, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET, pinfo->dsi_phy_ctrl.data_lane_hs2lp_time, 10, 16);
}

static void mipi_dsi_auto_ulps_config(struct mipi_dsi_timing *timing, struct dpu_panel_info *pinfo,
	char __iomem *mipi_dsi_base)
{
	uint32_t twakeup_cnt;
	uint32_t auto_ulps_enter_delay;
	uint32_t twakeup_clk_div;

	auto_ulps_enter_delay = timing->hline_time * 3 / 2;  /* chip protocol */
	twakeup_clk_div = 8;  /* frequency division is 8 */
	/* twakeup_cnt*twakeup_clk_div*t_lanebyteclk>1ms */
	if (pinfo->mipi.phy_mode == CPHY_MODE)
		twakeup_cnt = pinfo->dsi_phy_ctrl.lane_word_clk;
	else
		twakeup_cnt = pinfo->dsi_phy_ctrl.lane_byte_clk;

	/* chip protocol */
	twakeup_cnt = twakeup_cnt / 1000 * 3 / 2 / twakeup_clk_div;

	set_reg(mipi_dsi_base + AUTO_ULPS_ENTER_DELAY, auto_ulps_enter_delay, 32, 0);
	set_reg(mipi_dsi_base + AUTO_ULPS_WAKEUP_TIME, twakeup_clk_div, 16, 0);
	set_reg(mipi_dsi_base + AUTO_ULPS_WAKEUP_TIME, twakeup_cnt, 16, 16);
}

void mipi_init(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base)
{
	struct dpu_panel_info *pinfo;
	struct mipi_dsi_timing timing;

	pinfo = &(dpufd->panel_info);

	if (pinfo->mipi.max_tx_esc_clk == 0) {
		DPU_FB_ERR("fb%d, max_tx_esc_clk is invalid!\n", dpufd->index);
		pinfo->mipi.max_tx_esc_clk = DEFAULT_MAX_TX_ESC_CLK;
	}

	memset(&(pinfo->dsi_phy_ctrl), 0, sizeof(struct mipi_dsi_phy_ctrl));

	if (pinfo->mipi.phy_mode == CPHY_MODE)
		get_dsi_cphy_ctrl(dpufd, &(pinfo->dsi_phy_ctrl));
	else
		get_dsi_dphy_ctrl(dpufd, &(pinfo->dsi_phy_ctrl));

	get_mipi_dsi_timing(dpufd);
	memset(&timing, 0, sizeof(timing));
	get_mipi_dsi_timing_config_para(dpufd, &(pinfo->dsi_phy_ctrl), &timing);

	mipi_dsi_phy_config(dpufd, mipi_dsi_base, pinfo);

	set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 1);
	if (is_mipi_cmd_panel(dpufd)) {
		set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 0);
		if (pinfo->mipi.color_mode == DSI_DSC24_COMPRESSED_DATA)
			set_reg(mipi_dsi_base + MIPIDSI_EDPI_CMD_SIZE_OFFSET,
				pinfo->panel_dsc_info.dsc_info.chunk_size, 16, 0);
		else
			set_reg(mipi_dsi_base + MIPIDSI_EDPI_CMD_SIZE_OFFSET,
				timing.width, 16, 0);
		/* cnt=2 in update-patial scene, cnt nees to be checked for different panels */
		if (pinfo->mipi.hs_wr_to_time == 0)
			set_reg(mipi_dsi_base + MIPIDSI_HS_WR_TO_CNT_OFFSET, 0x1000002, 25, 0);
		else
			set_reg(mipi_dsi_base + MIPIDSI_HS_WR_TO_CNT_OFFSET,
				(0x1 << 24) | (pinfo->mipi.hs_wr_to_time *
				pinfo->dsi_phy_ctrl.lane_byte_clk / 1000000000UL), 25, 0);
	}

	/* phy_stop_wait_time */
	set_reg(mipi_dsi_base + MIPIDSI_PHY_IF_CFG_OFFSET, pinfo->dsi_phy_ctrl.phy_stop_wait_time, 8, 8);

	/* --------------configuring the DPI packet transmission---------------- */
	/*
	 * 2. Configure the DPI Interface:
	 * This defines how the DPI interface interacts with the controller.
	 */
	mipi_dsi_config_dpi_interface(pinfo, mipi_dsi_base);

	/*
	 * 3. Select the Video Transmission Mode:
	 * This defines how the processor requires the video line to be
	 * transported through the DSI link.
	 */
	mipi_dsi_video_mode_config(dpufd, mipi_dsi_base, pinfo, &timing);

	/* for dsi read, BTA enable */
	set_reg(mipi_dsi_base + MIPIDSI_PCKHDL_CFG_OFFSET, 0x1, 1, 2);

	/*
	 * 4. Define the DPI Horizontal timing configuration:
	 *
	 * Hsa_time = HSA*(PCLK period/Clk Lane Byte Period);
	 * Hbp_time = HBP*(PCLK period/Clk Lane Byte Period);
	 * Hline_time = (HSA+HBP+HACT+HFP)*(PCLK period/Clk Lane Byte Period);
	 */
	mipi_dsi_horizontal_timing_config(&timing, mipi_dsi_base);

	mipi_dsi_vfp_vsync_config(&timing, mipi_dsi_base);

	mipi_dsi_phy_timing_config(pinfo, mipi_dsi_base);

	/* 16~19bit:pclk_en, pclk_sel, dpipclk_en, dpipclk_sel */
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET, 0x5, 4, 16);

	if (is_mipi_cmd_panel(dpufd))
		mipi_dsi_auto_ulps_config(&timing, pinfo, mipi_dsi_base);

	if (pinfo->mipi.phy_mode == CPHY_MODE)
		set_reg(mipi_dsi_base + PHY_MODE, 0x1, 1, 0);
	else
		set_reg(mipi_dsi_base + PHY_MODE, 0x0, 1, 0);

	mipi_ldi_init(dpufd, mipi_dsi_base);

	/* Waking up Core */
	set_reg(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x1, 1, 0);
}

int mipi_dsi_clk_enable(struct dpu_fb_data_type *dpufd)
{
	int ret;
	struct clk *clk_tmp = NULL;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	if (need_config_dsi0(dpufd)) {
		clk_tmp = dpufd->dss_dphy0_ref_clk;
		if (clk_tmp) {
			ret = clk_prepare_enable(clk_tmp);
			dpu_check_and_return(ret, -EINVAL, ERR,
				"fb%d dss_dphy0_ref_clk clk_prepare_enable failed, error=%d!\n", dpufd->index, ret);
		}

		clk_tmp = dpufd->dss_dphy0_cfg_clk;
		if (clk_tmp) {
			ret = clk_prepare_enable(clk_tmp);
			dpu_check_and_return(ret, -EINVAL, ERR,
				"fb%d dss_dphy0_cfg_clk clk_prepare_enable failed, error=%d!\n", dpufd->index, ret);
		}
	}


	if (need_config_dsi1(dpufd)) {
		clk_tmp = dpufd->dss_dphy1_ref_clk;
		if (clk_tmp) {
			ret = clk_prepare_enable(clk_tmp);
			dpu_check_and_return(ret, -EINVAL, ERR,
				"fb%d dss_dphy1_ref_clk clk_prepare_enable failed, error=%d!\n", dpufd->index, ret);
		}

		clk_tmp = dpufd->dss_dphy1_cfg_clk;
		if (clk_tmp) {
			ret = clk_prepare_enable(clk_tmp);
			dpu_check_and_return(ret, -EINVAL, ERR,
				"fb%d dss_dphy1_cfg_clk clk_prepare_enable failed, error=%d!\n", dpufd->index, ret);
		}
	}

	return 0;
}

int mipi_dsi_clk_disable(struct dpu_fb_data_type *dpufd)
{
	struct clk *clk_tmp = NULL;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	if (need_config_dsi0(dpufd)) {
		clk_tmp = dpufd->dss_dphy0_ref_clk;
		if (clk_tmp)
			clk_disable_unprepare(clk_tmp);

		clk_tmp = dpufd->dss_dphy0_cfg_clk;
		if (clk_tmp)
			clk_disable_unprepare(clk_tmp);
	}


	if (need_config_dsi1(dpufd)) {
		clk_tmp = dpufd->dss_dphy1_ref_clk;
		if (clk_tmp)
			clk_disable_unprepare(clk_tmp);

		clk_tmp = dpufd->dss_dphy1_cfg_clk;
		if (clk_tmp)
			clk_disable_unprepare(clk_tmp);
	}

	return 0;
}

static int mipi_dsi_on_sub1(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base)
{
	/* mipi init */
	mipi_init(dpufd, mipi_dsi_base);

	/* switch to cmd mode */
	set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 0);
	/* cmd mode: low power mode */
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x7f, 7, 8);
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0xf, 4, 16);
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 24);
	/* disable generate High Speed clock */
	set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);

	return 0;
}

static void pctrl_dphytx_stopcnt_config(struct dpu_fb_data_type *dpufd)
{
	uint64_t lane_byte_clk;
	uint64_t pctrl_dphytx_stopcnt;
	char __iomem *mipi_dsi_base = NULL;
	struct dpu_panel_info *pinfo = NULL;
	uint32_t stopcnt_div;
	struct mipi_dsi_timing timing;

	pinfo = &(dpufd->panel_info);

	memset(&timing, 0, sizeof(timing));
	get_mipi_dsi_timing_config_para(dpufd, &(pinfo->dsi_phy_ctrl), &timing);

	stopcnt_div = is_dual_mipi_panel(dpufd) ? 2 : 1;
	/* init: wait DPHY 4 data lane stopstate */
	dpu_check_and_no_retval((pinfo->pxl_clk_rate == 0), ERR, "pxl_clk_rate is zero\n");
	if (is_mipi_video_panel(dpufd)) {
		pctrl_dphytx_stopcnt = (uint64_t)(pinfo->ldi.h_back_porch +
			pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width +
			pinfo->xres / stopcnt_div + 5) *
			dpufd->dss_vote_cmd.dss_pclk_dss_rate /
			(pinfo->pxl_clk_rate / stopcnt_div);
	} else {
		pctrl_dphytx_stopcnt = (uint64_t)(pinfo->ldi.h_back_porch +
			pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width + 5) *
			dpufd->dss_vote_cmd.dss_pclk_dss_rate /
			(pinfo->pxl_clk_rate / stopcnt_div);
	}

	if (pinfo->mipi.dsi_timing_support) {
		lane_byte_clk = (pinfo->mipi.phy_mode == DPHY_MODE) ?
			pinfo->dsi_phy_ctrl.lane_byte_clk : pinfo->dsi_phy_ctrl.lane_word_clk;
		dpu_check_and_no_retval((lane_byte_clk == 0), ERR, "lane_byte_clk is zero\n");
		pctrl_dphytx_stopcnt = round1(((uint64_t)timing.hline_time *
			dpufd->dss_vote_cmd.dss_pclk_dss_rate), lane_byte_clk);
		DPU_FB_INFO("pctrl_dphytx_stopcnt = %llu, pclk = %lu\n",
			pctrl_dphytx_stopcnt, dpufd->dss_vote_cmd.dss_pclk_dss_rate);
	}

	mipi_dsi_base = get_mipi_dsi_base(dpufd);
	outp32(mipi_dsi_base + MIPIDSI_DPHYTX_STOPSNT_OFFSET, (uint32_t)pctrl_dphytx_stopcnt);
	if (is_dual_mipi_panel(dpufd))
		outp32(dpufd->mipi_dsi1_base + MIPIDSI_DPHYTX_STOPSNT_OFFSET, (uint32_t)pctrl_dphytx_stopcnt);
}

static int mipi_dsi_on_sub2(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base)
{
	struct dpu_panel_info *pinfo = NULL;

	dpu_check_and_return((!dpufd || !mipi_dsi_base), 0, ERR, "dpufd or mipi_dsi_base is NULL\n");

	pinfo = &(dpufd->panel_info);

	if (is_mipi_video_panel(dpufd))
		/* switch to video mode */
		set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x0, 1, 0);

	if (is_mipi_cmd_panel(dpufd)) {
		/* cmd mode: high speed mode */
		set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 7, 8);
		set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 4, 16);
		set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 1, 24);
	}

	/* enable EOTP TX */
	if (pinfo->mipi.phy_mode == DPHY_MODE) {
		/* Some vendors don't need eotp check */
		if (pinfo->mipi.eotp_disable_flag == 1)
			set_reg(mipi_dsi_base + MIPIDSI_PCKHDL_CFG_OFFSET, 0x0, 1, 0);
		else
			set_reg(mipi_dsi_base + MIPIDSI_PCKHDL_CFG_OFFSET, 0x1, 1, 0);
	}

	/* enable generate High Speed clock, non continue */
	if (pinfo->mipi.non_continue_en)
		set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x3, 2, 0);
	else
		set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, 2, 0);

	if ((pinfo->mipi.dsi_version == DSI_1_2_VERSION)
		&& is_ifbc_vesa_panel(dpufd))
		set_reg(mipi_dsi_base + MIPIDSI_DSC_PARAMETER_OFFSET, 0x01, 32, 0);

	pctrl_dphytx_stopcnt_config(dpufd);

	return 0;
}

static void mipi_check_itf_status(struct dpu_fb_data_type *dpufd)
{
	int delay_count = 0;
	bool is_timeout = true;
	char __iomem *mctl_sys_base = NULL;
	int max_delay_count = 100;  /* 100ms */
	uint32_t temp;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return;

	mctl_sys_base =  dpufd->dss_base + DSS_MCTRL_SYS_OFFSET;

	while (1) {
		temp = inp32(mctl_sys_base + MCTL_MOD17_STATUS);
		if (((temp & 0x10) == 0x10) || (delay_count > max_delay_count)) {
			is_timeout = (delay_count > max_delay_count) ? true : false;
			break;
		}
		mdelay(1);  /* 1ms */
		++delay_count;
	}

	if (is_timeout)
		DPU_FB_INFO("mctl_itf%d not in idle status, ints=0x%x\n", dpufd->index, temp);
}

static int mipi_dsi_off_sub(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base)
{
	/* switch to cmd mode */
	set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 0);
	/* cmd mode: low power mode */
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x7f, 7, 8);
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0xf, 4, 16);
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 24);

	/* disable generate High Speed clock */
	set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);
	udelay(10);  /* 10us */

	/* shutdown d_phy */
	set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x0, 3, 0);

	return 0;
}

void mipi_dsi_reset(struct dpu_fb_data_type *dpufd)
{
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	set_reg(dpufd->mipi_dsi0_base + MIPIDSI_PWR_UP_OFFSET, 0x0, 1, 0);
	msleep(2); /* 2ms */
	set_reg(dpufd->mipi_dsi0_base + MIPIDSI_PWR_UP_OFFSET, 0x1, 1, 0);
}

/*******************************************************************************
 * MIPI DPHY GPIO for FPGA
 */
#define GPIO_MIPI_DPHY_PG_SEL_A_NAME "pg_sel_a"
#define GPIO_MIPI_DPHY_PG_SEL_B_NAME "pg_sel_b"
#define GPIO_MIPI_DPHY_TX_RX_A_NAME "tx_rx_a"
#define GPIO_MIPI_DPHY_TX_RX_B_NAME "tx_rx_b"

static uint32_t gpio_pg_sel_a = GPIO_PG_SEL_A;
static uint32_t gpio_tx_rx_a = GPIO_TX_RX_A;
static uint32_t gpio_pg_sel_b = GPIO_PG_SEL_B;
static uint32_t gpio_tx_rx_b = GPIO_TX_RX_B;

static struct gpio_desc mipi_dphy_gpio_request_cmds[] = {
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0 },
};

static struct gpio_desc mipi_dphy_gpio_free_cmds[] = {
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0 },

};

static struct gpio_desc mipi_dphy_gpio_normal_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 1 },

	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 1 },
};

static struct gpio_desc mipi_dphy_gpio_lowpower_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0 },

	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0 },
};

static int mipi_dsi_dphy_fastboot_fpga(struct dpu_fb_data_type *dpufd)
{
	if (g_fpga_flag == 1)
		/* mpi dphy gpio request */
		gpio_cmds_tx(mipi_dphy_gpio_request_cmds, ARRAY_SIZE(mipi_dphy_gpio_request_cmds));

	return 0;
}

static int mipi_dsi_dphy_on_fpga(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->index == EXTERNAL_PANEL_IDX)
		return 0;

	if (g_fpga_flag == 1) {
		/* mipi dphy gpio request */
		gpio_cmds_tx(mipi_dphy_gpio_request_cmds, ARRAY_SIZE(mipi_dphy_gpio_request_cmds));

		/* mipi dphy gpio normal */
		gpio_cmds_tx(mipi_dphy_gpio_normal_cmds, ARRAY_SIZE(mipi_dphy_gpio_normal_cmds));
	}

	return 0;
}

static int mipi_dsi_dphy_off_fpga(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->index == EXTERNAL_PANEL_IDX)
		return 0;

	if (g_fpga_flag == 1) {
		/* mipi dphy gpio lowpower */
		gpio_cmds_tx(mipi_dphy_gpio_lowpower_cmds, ARRAY_SIZE(mipi_dphy_gpio_lowpower_cmds));
		/* mipi dphy gpio free */
		gpio_cmds_tx(mipi_dphy_gpio_free_cmds, ARRAY_SIZE(mipi_dphy_gpio_free_cmds));
	}

	return 0;
}

int mipi_dsi_set_fastboot(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	int ret;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	mipi_dsi_dphy_fastboot_fpga(dpufd);

	mipi_dsi_clk_enable(dpufd);

	/* bugfix for access dsi reg noc before apb clock enable */
	if (dpufd->index == PRIMARY_PANEL_IDX) {
		/* disp core dsi0 clk */
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL, 0x1, 1, 16);
		set_reg(dpufd->dss_base + GLB_MODULE_CLK_SEL, 0x1, 1, 23);
		set_reg(dpufd->dss_base + GLB_MODULE_CLK_EN, 0x1, 1, 23);
		if (is_dual_mipi_panel(dpufd)) {
			/* disp core dsi1 clk */
			set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL, 0x1, 1, 17);
			set_reg(dpufd->dss_base + GLB_MODULE_CLK_SEL, 0x1, 1, 24);
			set_reg(dpufd->dss_base + GLB_MODULE_CLK_EN, 0x1, 1, 24);
		}
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		/* disp core dsi1 clk */
		set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL, 0x1, 1, 17);
		set_reg(dpufd->dss_base + GLB_MODULE_CLK_SEL, 0x1, 1, 24);
		set_reg(dpufd->dss_base + GLB_MODULE_CLK_EN, 0x1, 1, 24);
	}

	pinfo = &(dpufd->panel_info);
	memset(&(pinfo->dsi_phy_ctrl), 0, sizeof(struct mipi_dsi_phy_ctrl));
	if (pinfo->mipi.phy_mode == CPHY_MODE)
		get_dsi_cphy_ctrl(dpufd, &(pinfo->dsi_phy_ctrl));
	else
		get_dsi_dphy_ctrl(dpufd, &(pinfo->dsi_phy_ctrl));

	get_mipi_dsi_timing(dpufd);

	ret = panel_next_set_fastboot(pdev);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

static void mipi_dsi0_clk_enable(struct dpu_fb_data_type *dpufd)
{
	set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL, 0x1, 1, 16);
	set_reg(dpufd->dss_base + GLB_MODULE_CLK_SEL, 0x1, 1, 23);
	set_reg(dpufd->dss_base + GLB_MODULE_CLK_EN, 0x1, 1, 23);
	udelay(10);  /* 10us */
}

static void mipi_dsi1_clk_enable(struct dpu_fb_data_type *dpufd)
{
	set_reg(dpufd->dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL, 0x1, 1, 17);
	set_reg(dpufd->dss_base + GLB_MODULE_CLK_SEL, 0x1, 1, 24);
	set_reg(dpufd->dss_base + GLB_MODULE_CLK_EN, 0x1, 1, 24);
	udelay(10);  /* 10us */
}

static void mipi_dsi_module_clk_sel(struct dpu_fb_data_type *dpufd, struct platform_device *pdev)
{
	if (need_config_dsi0(dpufd)) {
		/* disp core dsi0 clk */
		mipi_dsi0_clk_enable(dpufd);
		mipi_dsi_on_sub1(dpufd, dpufd->mipi_dsi0_base);
		if (is_dual_mipi_panel(dpufd)) {
			/* disp core dsi1 clk */
			mipi_dsi1_clk_enable(dpufd);
			mipi_dsi_on_sub1(dpufd, dpufd->mipi_dsi1_base);
		}

		/* Here need to exit ulps when panel off bypass ddic power down */
		if (panel_next_bypass_powerdown_ulps_support(pdev) ||
			(dpufd->panel_info.skip_power_on_off == SKIP_POWER_ON_OFF))
			mipi_dsi_ulps_cfg(dpufd, 1);  /* 0--enable 1--not enabled */
	}

	if ((dpufd->index == EXTERNAL_PANEL_IDX) || (is_dsi1_pipe_switch_connector(dpufd))) {
		if (is_dsi1_pipe_switch_connector(dpufd))
			mipi_dsi0_clk_enable(dpufd);
		/* disp core dsi1 clk */
		mipi_dsi1_clk_enable(dpufd);
		mipi_dsi_on_sub1(dpufd, dpufd->mipi_dsi1_base);

		/* Here need to exit ulps when panel off bypass ddic power down */
		if (panel_next_bypass_powerdown_ulps_support(pdev) ||
			(dpufd->panel_info.skip_power_on_off == SKIP_POWER_ON_OFF))
			mipi_dsi_ulps_cfg(dpufd, 1);  /* 0--enable 1--not enabled */
	}
}

int mipi_dsi_on(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_INFO("fb%d, +\n", dpufd->index);

	mipi_dsi_dphy_on_fpga(dpufd);

	/* set LCD init step before LCD on */
	dpufd->panel_info.lcd_init_step = LCD_INIT_POWER_ON;
	panel_next_on(pdev);

	/* dis-reset ip_reset_dis_dsi0, ip_reset_dis_dsi1 */
	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (need_config_dsi1(dpufd))
			outp32(dpufd->peri_crg_base + PERRSTDIS3, 0x30000000);
		else
			outp32(dpufd->peri_crg_base + PERRSTDIS3, 0x10000000);
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		outp32(dpufd->peri_crg_base + PERRSTDIS3, 0x20000000);
	} else {
		DPU_FB_ERR("fb%d, not supported!\n", dpufd->index);
	}
	mipi_dsi_clk_enable(dpufd);

	mipi_dsi_module_clk_sel(dpufd, pdev);

	panel_next_on(pdev);

	if (need_config_dsi0(dpufd))
		mipi_dsi_on_sub2(dpufd, dpufd->mipi_dsi0_base);
	if (need_config_dsi1(dpufd))
		mipi_dsi_on_sub2(dpufd, dpufd->mipi_dsi1_base);

	/* mipi hs video/command mode */
	panel_next_on(pdev);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return 0;
}

static void mipi_tx_off_rx_ulps_config(
	struct dpu_fb_data_type *dpufd, struct platform_device *pdev)
{
	int delay_count = 0;
	int delay_count_max = 16;
	char __iomem *mipi_dsi_base = get_mipi_dsi_base(dpufd);

	if (!panel_next_bypass_powerdown_ulps_support(pdev) &&
		(dpufd->panel_info.skip_power_on_off != SKIP_POWER_ON_OFF))
		return;

	disable_ldi(dpufd);
	/*
	 * Read register status, maximum waiting time is 16ms
	 * 0x7FF--The lower 11 bits of the register 0x1--Register value
	 */
	while ((((uint32_t)inp32(mipi_dsi_base + MIPI_LDI_VSTATE)) & 0x7FF) != 0x1) {
		if (++delay_count > delay_count_max) {
			DPU_FB_ERR("wait ldi vstate idle timeout\n");
			break;
		}
		msleep(1);  /* 1ms */
	}
	/* 0--enable 1--not enabled */
	mipi_dsi_ulps_cfg(dpufd, 0);
}

int mipi_dsi_off(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	int ret;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) {
		if (is_dual_mipi_panel(dpufd)) {
			set_reg(dpufd->mipi_dsi0_base + MIPI_LDI_FRM_VALID_DBG, 1, 1, 29);
			set_reg(dpufd->mipi_dsi1_base + MIPI_LDI_FRM_VALID_DBG, 1, 1, 29);
		}
	}

	/* set LCD uninit step before LCD off */
	dpufd->panel_info.lcd_uninit_step = LCD_UNINIT_MIPI_HS_SEND_SEQUENCE;
	ret = panel_next_off(pdev);

	if (dpufd->panel_info.lcd_uninit_step_support)
		/* add MIPI LP mode here if necessary MIPI LP mode end */
		ret = panel_next_off(pdev);

	disable_ldi(dpufd);

	if (need_config_dsi0(dpufd)) {
		/* Here need to enter ulps when panel off bypass ddic power down */
		mipi_tx_off_rx_ulps_config(dpufd, pdev);
		mipi_check_itf_status(dpufd);
		mipi_dsi_off_sub(dpufd, dpufd->mipi_dsi0_base);
	}
	if (need_config_dsi1(dpufd)) {
		if (is_dsi1_pipe_switch_connector(dpufd))
			/* Here need to enter ulps when panel off bypass ddic power down */
			mipi_tx_off_rx_ulps_config(dpufd, pdev);
		mipi_dsi_off_sub(dpufd, dpufd->mipi_dsi1_base);
	}

	mipi_dsi_clk_disable(dpufd);

	mipi_dsi_dphy_off_fpga(dpufd);

	/* reset DSI */
	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (need_config_dsi1(dpufd))
			outp32(dpufd->peri_crg_base + PERRSTEN3, 0x30000000);
		else
			outp32(dpufd->peri_crg_base + PERRSTEN3, 0x10000000);
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		outp32(dpufd->peri_crg_base + PERRSTEN3, 0x20000000);
	} else {
		DPU_FB_ERR("fb%d, not supported!\n", dpufd->index);
	}

	if (dpufd->panel_info.lcd_uninit_step_support)
		ret = panel_next_off(pdev);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

static void mipi_dsi_get_ulps_stopstate(struct dpu_fb_data_type *dpufd, uint32_t *cmp_ulpsactivenot_val,
	uint32_t *cmp_stopstate_val, bool enter_ulps)
{
	/* ulps enter */
	if (enter_ulps) {
		if (dpufd->panel_info.mipi.lane_nums >= DSI_4_LANES) {
			*cmp_ulpsactivenot_val = (BIT(5) | BIT(8) | BIT(10) | BIT(12));
			*cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9) | BIT(11));
		} else if (dpufd->panel_info.mipi.lane_nums >= DSI_3_LANES) {
			*cmp_ulpsactivenot_val = (BIT(5) | BIT(8) | BIT(10));
			*cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9));
		} else if (dpufd->panel_info.mipi.lane_nums >= DSI_2_LANES) {
			*cmp_ulpsactivenot_val = (BIT(5) | BIT(8));
			*cmp_stopstate_val = (BIT(4) | BIT(7));
		} else {
			*cmp_ulpsactivenot_val = (BIT(5));
			*cmp_stopstate_val = (BIT(4));
		}
	} else { /* ulps exit */
		if (dpufd->panel_info.mipi.lane_nums >= DSI_4_LANES) {
			*cmp_ulpsactivenot_val = (BIT(3) | BIT(5) | BIT(8) | BIT(10) | BIT(12));
			*cmp_stopstate_val = (BIT(2) | BIT(4) | BIT(7) | BIT(9) | BIT(11));
		} else if (dpufd->panel_info.mipi.lane_nums >= DSI_3_LANES) {
			*cmp_ulpsactivenot_val = (BIT(3) | BIT(5) | BIT(8) | BIT(10));
			*cmp_stopstate_val = (BIT(2) | BIT(4) | BIT(7) | BIT(9));
		} else if (dpufd->panel_info.mipi.lane_nums >= DSI_2_LANES) {
			*cmp_ulpsactivenot_val = (BIT(3) | BIT(5) | BIT(8));
			*cmp_stopstate_val = (BIT(2) | BIT(4) | BIT(7));
		} else {
			*cmp_ulpsactivenot_val = (BIT(3) | BIT(5));
			*cmp_stopstate_val = (BIT(2) | BIT(4));
		}
		if (dpufd->panel_info.mipi.phy_mode == CPHY_MODE) {
			*cmp_ulpsactivenot_val &= ~(BIT(3));
			*cmp_stopstate_val &= ~(BIT(2));
		}
	}
}

static int mipi_dsi_check_ulps_stopstate(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base,
	uint32_t cmp_stopstate_val, bool enter_ulps)
{
	uint32_t try_times = 0;
	uint32_t temp;

	if (enter_ulps) {
		/* check DPHY data and clock lane stopstate */
		temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		while ((temp & cmp_stopstate_val) != cmp_stopstate_val) {
			udelay(10);  /* 10us */
			if (++try_times > 100) {  /* try 1ms */
				DPU_FB_ERR("fb%d, check phy data and clk lane stop state failed! "
					"PHY_STATUS=0x%x\n", dpufd->index, temp);
				return -1;
			}
			if ((temp & cmp_stopstate_val) == (cmp_stopstate_val & ~(BIT(2)))) {
				DPU_FB_INFO("fb%d, datalanes are in stop state, pull down "
					"phy_txrequestclkhs\n", dpufd->index);
				set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 1);
				set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);
			}
			temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		}
	} else {
		/* check DPHY data lane cmp_stopstate_val */
		try_times = 0;
		temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		while ((temp & cmp_stopstate_val) != cmp_stopstate_val) {
			udelay(10); /* 10us */
			if (++try_times > 100) { /* try 1ms */
				DPU_FB_ERR("fb%d, check phy data clk lane stop state failed! "
					"PHY_STATUS=0x%x\n", dpufd->index, temp);
				break;
			}

			temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		}
	}

	return 0;
}

static void mipi_dsi_data_clock_lane_enter_ulps(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base,
	uint32_t cmp_ulpsactivenot_val)
{
	uint32_t try_times;
	uint32_t temp;

	/* request that data lane enter ULPS */
	set_reg(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x4, 4, 0);

	/* check DPHY data lane ulpsactivenot_status */
	try_times = 0;
	temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	while ((temp & cmp_ulpsactivenot_val) != 0) {
		udelay(10);  /* 10us */
		if (++try_times > 100) {  /* try 1ms */
			DPU_FB_ERR("fb%d, request phy data lane enter ulps failed! "
				"PHY_STATUS=0x%x.\n", dpufd->index, temp);
			break;
		}

		temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	}

	/* request that clock lane enter ULPS */
	if (dpufd->panel_info.mipi.phy_mode == DPHY_MODE) {
		set_reg(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x5, 4, 0);

		/* check DPHY clock lane ulpsactivenot_status */
		try_times = 0;
		temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		while ((temp & BIT(3)) != 0) {
			udelay(10); /* 10us */
			if (++try_times > 100) { /* try 1ms */
				DPU_FB_ERR("fb%d, request phy clk lane enter ulps failed! "
					"PHY_STATUS=0x%x\n", dpufd->index, temp);
				break;
			}

			temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		}
	}
}

static int mipi_dsi_ulps_enter(struct dpu_fb_data_type *dpufd,
	char __iomem *mipi_dsi_base, uint32_t mipi_idx)
{
	uint32_t cmp_ulpsactivenot_val = 0;
	uint32_t cmp_stopstate_val = 0;
	bool is_ready = false;
	uint32_t temp;

	DPU_FB_DEBUG("fb%d, mipi_idx=%d, %s +!\n", dpufd->index, mipi_idx, __func__);

	mipi_dsi_get_ulps_stopstate(dpufd, &cmp_ulpsactivenot_val, &cmp_stopstate_val, true);

	temp = (uint32_t)inp32(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET) & BIT(1);
	if (temp && (dpufd->panel_info.mipi.phy_mode == DPHY_MODE))
		cmp_stopstate_val |= (BIT(2));

	if (mipi_dsi_check_ulps_stopstate(dpufd, mipi_dsi_base, cmp_stopstate_val, true))
		return 0;

	/* disable DPHY clock lane's Hight Speed Clock */
	set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);

	/* force_pll = 0 */
	set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x0, 1, 3);

	mipi_dsi_data_clock_lane_enter_ulps(dpufd, mipi_dsi_base, cmp_ulpsactivenot_val);

	/* check phy lock == 0? */
	is_ready = mipi_phy_status_check(mipi_dsi_base, 0x0);
	if (!is_ready)
		DPU_FB_DEBUG("fb%d, phylock == 1!\n", dpufd->index);

	/* bit13 lock sel enable (dual_mipi_panel mipi_dsi1_base+bit13 set 1), colse clock gate */
	set_reg(mipi_dsi_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 0x1, 1, 13);
	set_reg(dpufd->peri_crg_base + PERDIS3, 0x3, 4, 28);
	if (mipi_idx == DSI1_INDEX) {
		set_reg(dpufd->mipi_dsi1_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 0x1, 1, 13);
		set_reg(dpufd->peri_crg_base + PERDIS3, 0xf, 4, 28);
	}

	DPU_FB_DEBUG("fb%d, mipi_idx=%d, %s -!\n", dpufd->index, mipi_idx, __func__);

	return 0;
}

static void mipi_dsi_data_clock_lane_exit_ulps(struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base,
	uint32_t cmp_ulpsactivenot_val)
{
	uint32_t try_times;
	uint32_t temp;

	/* request that data lane and clock lane exit ULPS */
	outp32(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0xF);
	try_times = 0;
	temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	while ((temp & cmp_ulpsactivenot_val) != cmp_ulpsactivenot_val) {
		udelay(10);  /* delay 10us */
		if (++try_times > 100) {  /* try 1ms */
			DPU_FB_ERR("fb%d, request data clock lane exit ulps fail! "
				"PHY_STATUS=0x%x\n", dpufd->index, temp);
			break;
		}

		temp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	}
}

static int mipi_dsi_ulps_exit(struct dpu_fb_data_type *dpufd,
	char __iomem *mipi_dsi_base, uint32_t mipi_idx)
{
	uint32_t cmp_ulpsactivenot_val = 0;
	uint32_t cmp_stopstate_val = 0;
	bool is_ready = false;

	DPU_FB_DEBUG("fb%d, mipi_idx=%d, %s +!\n", dpufd->index, mipi_idx, __func__);

	mipi_dsi_get_ulps_stopstate(dpufd, &cmp_ulpsactivenot_val, &cmp_stopstate_val, false);

	if (is_dual_mipi_panel(dpufd) || (mipi_idx == DSI1_INDEX))
		set_reg(dpufd->peri_crg_base + PEREN3, 0xf, 4, 28);
	else
		set_reg(dpufd->peri_crg_base + PEREN3, 0x3, 4, 28);

	udelay(10); /* 10us */
	/* force pll = 1 */
	set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x1, 1, 3);

	udelay(100); /* wait pll clk */

	/* check phy lock == 1? */
	is_ready = mipi_phy_status_check(mipi_dsi_base, 0x1);
	if (!is_ready)
		DPU_FB_DEBUG("fb%d, phylock == 0, phylock is not ready!\n", dpufd->index);

	/* bit13 lock sel enable (dual_mipi_panel mipi_dsi1_base+bit13 set 1), colse clock gate */
	set_reg(mipi_dsi_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 0x0, 1, 13);
	if (is_dual_mipi_panel(dpufd))
		set_reg(dpufd->mipi_dsi1_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 0x0, 1, 13);

	mipi_dsi_data_clock_lane_exit_ulps(dpufd, mipi_dsi_base, cmp_ulpsactivenot_val);

	/* mipi spec */
	mdelay(1);

	/* clear PHY_ULPS_CTRL */
	outp32(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x0);

	mipi_dsi_check_ulps_stopstate(dpufd, mipi_dsi_base, cmp_stopstate_val, false);

	/* enable DPHY clock lane's Hight Speed Clock */
	set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, 1, 0);
	if (dpufd->panel_info.mipi.non_continue_en)
		set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, 1, 1);

	/* reset dsi */
	outp32(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x0);
	udelay(5);
	/* Power_up dsi */
	outp32(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x1);

	DPU_FB_DEBUG("fb%d, mipi_idx=%d, %s -!\n", dpufd->index, mipi_idx, __func__);
	return 0;
}

int mipi_dsi_ulps_cfg(struct dpu_fb_data_type *dpufd, int enable)
{
	char __iomem *mipi_dsi_base = NULL;
	uint32_t mipi_idx;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	mipi_dsi_base = get_mipi_dsi_base(dpufd);
	mipi_idx = get_mipi_dsi_index(dpufd);

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	if (enable) {
		mipi_dsi_ulps_exit(dpufd, mipi_dsi_base, mipi_idx);
		if (is_dual_mipi_panel(dpufd))
			mipi_dsi_ulps_exit(dpufd, dpufd->mipi_dsi1_base, DSI1_INDEX);
	} else {
		mipi_dsi_ulps_enter(dpufd, mipi_dsi_base, mipi_idx);
		if (is_dual_mipi_panel(dpufd))
			mipi_dsi_ulps_enter(dpufd, dpufd->mipi_dsi1_base, DSI1_INDEX);
	}

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return 0;
}

static void mipi_pll_cfg_for_clk_upt(
	char __iomem *mipi_dsi_base, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	/* PLL configuration III */
	mipi_config_phy_test_code(mipi_dsi_base, PLL_POS_DIV_ADDR,
		(phy_ctrl->rg_pll_posdiv << SHIFT_4BIT) | phy_ctrl->rg_pll_prediv);
	/* PLL configuration IV */
	mipi_config_phy_test_code(mipi_dsi_base, PLL_FBK_DIV_ADDR,
		(phy_ctrl->rg_pll_fbkdiv & PLL_FBK_DIV_MAX_VALUE));
}

static void mipi_pll_cfg_for_clk_upt_cmd(
	struct dpu_panel_info *pinfo, char __iomem *mipi_dsi_base,
	struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	if (g_fpga_flag) {
		if (pinfo->mipi.phy_mode == CPHY_MODE) {
			/* PLL configuration I */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010046,
				phy_ctrl->rg_cp + (phy_ctrl->rg_lpf_r << 4));
			/* PLL configuration II */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010048,
				phy_ctrl->rg_0p8v + (phy_ctrl->rg_2p5g << 1) +
				(pinfo->dsi_phy_ctrl.rg_320m << 2) +
				(phy_ctrl->rg_band_sel << 3) + (phy_ctrl->rg_cphy_div << 4));
			/* PLL configuration III */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010049,
				phy_ctrl->rg_pre_div);
			/* PLL configuration IV */
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A,
				phy_ctrl->rg_div);
		} else {
			/* PLL configuration I */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010046,
				phy_ctrl->rg_cp + (phy_ctrl->rg_lpf_r << 4));
			/* PLL configuration II */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010048,
				phy_ctrl->rg_0p8v + (phy_ctrl->rg_2p5g << 1) +
				(phy_ctrl->rg_320m << 2) + (phy_ctrl->rg_band_sel << 3));
			/* PLL configuration III */
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010049,
				phy_ctrl->rg_pre_div);
			/* PLL configuration IV */
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A,
				phy_ctrl->rg_div);
		}
	} else {
		/* PLL configuration */
		mipi_pll_cfg_for_clk_upt(mipi_dsi_base, phy_ctrl);
	}
}

static void mipi_dsi_set_cdphy_bit_clk_upt_cmd(
	struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base,
	struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	struct dpu_panel_info *pinfo = NULL;
	bool is_ready = false;

	DPU_FB_INFO("fb%d +\n", dpufd->index);
	pinfo = &(dpufd->panel_info);

	/* PLL configuration */
	mipi_pll_cfg_for_clk_upt_cmd(pinfo, mipi_dsi_base, phy_ctrl);

	/* PLL update control */
	mipi_config_phy_test_code(mipi_dsi_base, PLL_UPT_CTRL_ADDR, 0x1);

	/* clk lane HS2LP/LP2HS */
	outp32(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET,
		(phy_ctrl->clk_lane_lp2hs_time + (phy_ctrl->clk_lane_hs2lp_time << SHIFT_16BIT)));
	/* data lane HS2LP/ LP2HS */
	outp32(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET,
		(phy_ctrl->data_lane_lp2hs_time + (phy_ctrl->data_lane_hs2lp_time << SHIFT_16BIT)));

	/* escape clock dividor */
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET,
		(phy_ctrl->clk_division + (phy_ctrl->clk_division << 8)), 16, 0);

	is_ready = mipi_phy_status_check(mipi_dsi_base, PHY_LOCK_STANDARD_STATUS);
	if (!is_ready)
		DPU_FB_INFO("fb%d, phylock is not ready!\n", dpufd->index);

	DPU_FB_DEBUG("fb%d -\n", dpufd->index);
}

static void mipi_dsi_set_cdphy_bit_clk_upt_video(
	struct dpu_fb_data_type *dpufd, char __iomem *mipi_dsi_base,
	struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	struct dpu_panel_info *pinfo = NULL;
	dss_rect_t rect;
	bool is_ready = false;
	struct mipi_dsi_timing timing;

	DPU_FB_DEBUG("fb%d +\n", dpufd->index);

	pinfo = &(dpufd->panel_info);
	pinfo->dsi_phy_ctrl = *phy_ctrl;

	rect.x = 0;
	rect.y = 0;
	rect.w = pinfo->xres;
	rect.h = pinfo->yres;

	mipi_ifbc_get_rect(dpufd, &rect);

	/* PLL configuration */
	mipi_pll_cfg_for_clk_upt(mipi_dsi_base, phy_ctrl);

	/* PLL update control */
	mipi_config_phy_test_code(mipi_dsi_base, PLL_UPT_CTRL_ADDR, 0x1);

	if (pinfo->mipi.phy_mode == CPHY_MODE)
		mipi_config_cphy_spec1v0_parameter(mipi_dsi_base, pinfo,
			&pinfo->dsi_phy_ctrl);
	else
		mipi_config_dphy_spec1v2_parameter(mipi_dsi_base, pinfo,
			&pinfo->dsi_phy_ctrl);

	is_ready = mipi_phy_status_check(mipi_dsi_base, PHY_LOCK_STANDARD_STATUS);
	if (!is_ready)
		DPU_FB_INFO("fb%d, phylock is not ready!\n", dpufd->index);

	/* phy_stop_wait_time */
	set_reg(mipi_dsi_base + MIPIDSI_PHY_IF_CFG_OFFSET,
		phy_ctrl->phy_stop_wait_time, 8, 8);

	/*
	 * 4. Define the DPI Horizontal timing configuration:
	 *
	 * Hsa_time = HSA*(PCLK period/Clk Lane Byte Period);
	 * Hbp_time = HBP*(PCLK period/Clk Lane Byte Period);
	 * Hline_time = (HSA+HBP+HACT+HFP)*(PCLK period/Clk Lane Byte Period);
	 */
	memset(&timing, 0, sizeof(timing));
	get_mipi_dsi_timing_config_para(dpufd, phy_ctrl, &timing);

	if (timing.hline_time < (timing.hsa + timing.hbp + timing.dpi_hsize))
		DPU_FB_ERR("wrong hfp\n");

	set_reg(mipi_dsi_base + MIPIDSI_VID_HSA_TIME_OFFSET, timing.hsa, 12, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_HBP_TIME_OFFSET, timing.hbp, 12, 0);
	set_reg(mipi_dsi_base + MIPI_LDI_DPI0_HRZ_CTRL3,
		dss_reduce(timing.dpi_hsize), 12, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_HLINE_TIME_OFFSET,
		timing.hline_time, 15, 0);

	/* Configure core's phy parameters */
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET,
		phy_ctrl->clk_lane_lp2hs_time, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET,
		phy_ctrl->clk_lane_hs2lp_time, 10, 16);

	outp32(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET,
		(phy_ctrl->data_lane_lp2hs_time +
		(phy_ctrl->data_lane_hs2lp_time << 16)));

	DPU_FB_DEBUG("fb%d -\n", dpufd->index);
}

static bool check_pctrl_trstop_flag(
	struct dpu_fb_data_type *dpufd, int time_count)
{
	bool is_ready = false;
	int count;
	uint32_t temp;
	uint32_t tmp1;

	if (is_dual_mipi_panel(dpufd)) {
		for (count = 0; count < time_count; count++) {
			temp = inp32(dpufd->mipi_dsi0_base + MIPIDSI_DPHYTX_TRSTOP_FLAG_OFFSET);
			tmp1 = inp32(dpufd->mipi_dsi1_base + MIPIDSI_DPHYTX_TRSTOP_FLAG_OFFSET);
			if ((temp & tmp1 & 0x1) == 0x1) {
				is_ready = true;
				break;
			}
			udelay(2); /* 2us delay each time  */
		}
	} else {
		for (count = 0; count < time_count; count++) {
			temp = inp32(dpufd->mipi_dsi0_base + MIPIDSI_DPHYTX_TRSTOP_FLAG_OFFSET);
			if ((temp & 0x1) == 0x1) {
				is_ready = true;
				break;
			}
			udelay(2); /* 2us delay each time  */
		}
	}

	return is_ready;
}

static uint32_t get_stopstate_msk_value(uint8_t lane_nums)
{
	uint32_t stopstate_msk;

	if (lane_nums == DSI_4_LANES)
		stopstate_msk = BIT(0);
	else if (lane_nums == DSI_3_LANES)
		stopstate_msk = BIT(0) | BIT(4);
	else if (lane_nums == DSI_2_LANES)
		stopstate_msk = BIT(0) | BIT(3) | BIT(4);
	else
		stopstate_msk = BIT(0) | BIT(2) | BIT(3) | BIT(4);

	return stopstate_msk;
}

bool mipi_dsi_wait_vfp_end(struct dpu_fb_data_type *dpufd,
	uint64_t lane_byte_clk, struct timeval *tv0)
{
	uint32_t vfp_time;
	uint32_t timediff = 0;
	bool is_ready = false;
	uint32_t hline_time;
	uint32_t vfp_line;
	uint32_t stopstate_msk;
	struct timeval tv1;

	hline_time = (uint32_t)inp32(dpufd->mipi_dsi0_base + MIPIDSI_VID_HLINE_TIME_OFFSET) & VFP_TIME_MASK;

	vfp_line = (uint32_t)inp32(dpufd->mipi_dsi0_base + MIPIDSI_VID_VFP_LINES_OFFSET) & 0x3FF;

	if (lane_byte_clk >= MILLION_CONVERT) {
		vfp_time = (vfp_line + VFP_TIME_OFFSET) * hline_time / ((uint32_t)(lane_byte_clk / MILLION_CONVERT));
	} else {
		DPU_FB_ERR("vfp_time == 0\n");
		vfp_time = VFP_DEF_TIME;
	}

	DPU_FB_DEBUG("hline_time = %u, vfp_line = %u, lane_byte_clk = %llu\n",
		hline_time, vfp_line, lane_byte_clk);

	stopstate_msk = get_stopstate_msk_value(dpufd->panel_info.mipi.lane_nums);

	set_reg(dpufd->mipi_dsi0_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 1, 1, 0);
	set_reg(dpufd->mipi_dsi0_base + MIPIDSI_DPHYTX_CTRL_OFFSET, stopstate_msk, 5, 3);
	if (is_dual_mipi_panel(dpufd)) {
		set_reg(dpufd->mipi_dsi1_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 1, 1, 0);
		set_reg(dpufd->mipi_dsi1_base + MIPIDSI_DPHYTX_CTRL_OFFSET, stopstate_msk, 5, 3);
	}
	DPU_FB_DEBUG("is_ready == %d, timediff = %d , vfp_time = %d\n", is_ready, timediff, vfp_time);
	while ((!is_ready) && (timediff < vfp_time)) {
		is_ready = check_pctrl_trstop_flag(dpufd, PCTRL_TRY_TIME);
		DPU_FB_INFO("is_ready == %d\n", is_ready);
		dpufb_get_timestamp(&tv1);
		timediff = dpufb_timestamp_diff(tv0, &tv1);
	}
	DPU_FB_INFO("timediff = %d us, vfp_time = %d us\n", timediff, vfp_time);
	set_reg(dpufd->mipi_dsi0_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 0, 1, 0);
	if (is_dual_mipi_panel(dpufd))
		set_reg(dpufd->mipi_dsi1_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 0, 1, 0);

	return is_ready;
}

static bool mipi_dsi_check_ldi_vstate(struct dpu_fb_data_type *dpufd, uint64_t lane_byte_clk)
{
	bool is_ready = false;
	uint32_t count = 0;
	uint32_t vfp_time;
	uint32_t ldi_vstate;
	uint32_t hline_time;
	uint32_t vfp_line;

	hline_time = (uint32_t)inp32(dpufd->mipi_dsi0_base +
		MIPIDSI_VID_HLINE_TIME_OFFSET) & VFP_TIME_MASK;

	vfp_line = (uint32_t)inp32(dpufd->mipi_dsi0_base +
		MIPIDSI_VID_VFP_LINES_OFFSET) & 0x3FF; /* mask bit0:9 */

	if (lane_byte_clk >= MILLION_CONVERT) {
		vfp_time = (vfp_line + VFP_TIME_OFFSET) *
		hline_time / ((uint32_t)(lane_byte_clk / MILLION_CONVERT)); /*lint -e414*/
	} else {
		DPU_FB_ERR("vfp_time == 0\n");
		vfp_time = VFP_DEF_TIME;
	}

	DPU_FB_DEBUG("hline_time = %d, vfp_line = %d, vfp_time = %d\n", hline_time, vfp_line, vfp_time);

	/* read ldi vstate reg value and mask bit0:15 */
	ldi_vstate = inp32(dpufd->mipi_dsi0_base + MIPI_LDI_VSTATE) & 0xFFFF;
	while ((ldi_vstate == LDI_VSTATE_VFP) && (count < vfp_time)) {
		udelay(1);  /* 1us delay each time  */
		ldi_vstate = inp32(dpufd->mipi_dsi0_base + MIPI_LDI_VSTATE) & 0xFFFF;
		count++;
	}
	if ((ldi_vstate == LDI_VSTATE_IDLE) || (ldi_vstate == LDI_VSTATE_V_WAIT_GPU))
		is_ready = true;

	if (!is_ready)
		DPU_FB_INFO("ldi_vstate = 0x%x\n", ldi_vstate);

	return is_ready;
}

int mipi_dsi_reset_underflow_clear(struct dpu_fb_data_type *dpufd)
{
	uint32_t vfp_time = 0;
	bool is_ready = false;
	uint32_t timediff = 0;
	char __iomem *mipi_dsi_base;
	uint64_t lane_byte_clk;
	struct timeval tv0;

	if (is_dp_panel(dpufd))
		goto reset_exit;

	if (dpufd->index == PRIMARY_PANEL_IDX)
		mipi_dsi_base = get_mipi_dsi_base(dpufd);
	else if (dpufd->index == EXTERNAL_PANEL_IDX)
		mipi_dsi_base = dpufd->mipi_dsi1_base;
	else
		goto reset_exit;

	dpufb_get_timestamp(&tv0);

	lane_byte_clk = (dpufd->panel_info.mipi.phy_mode == DPHY_MODE) ?
		dpufd->panel_info.dsi_phy_ctrl.lane_byte_clk :
		dpufd->panel_info.dsi_phy_ctrl.lane_word_clk;

	is_ready = mipi_dsi_wait_vfp_end(dpufd, lane_byte_clk, &tv0);
	if (!is_ready)
		DPU_FB_ERR("check_pctrl_trstop_flag fail, vstate = 0x%x\n",
			inp32(mipi_dsi_base + MIPI_LDI_VSTATE));

	DPU_FB_INFO("timediff=%d us, vfp_time=%d us\n", timediff, vfp_time);

	set_reg(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x0, 1, 0);
	if (is_dual_mipi_panel(dpufd))
		set_reg(dpufd->mipi_dsi1_base + MIPIDSI_PWR_UP_OFFSET, 0x0, 1, 0);
	udelay(5); /* timing constraint */
	set_reg(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x1, 1, 0);
	if (is_dual_mipi_panel(dpufd))
		set_reg(dpufd->mipi_dsi1_base + MIPIDSI_PWR_UP_OFFSET, 0x1, 1, 0);

	ldi_data_gate(dpufd, true);
	enable_ldi(dpufd);

reset_exit:
	dpufd->underflow_flag = 0;
	return 0;
}

static void mipi_set_cdphy_bit_clk(
	struct dpu_fb_data_type *dpufd, struct mipi_dsi_phy_ctrl *phy_ctrl,
	struct dpu_panel_info *pinfo, uint8_t esd_enable)
{
	if (is_mipi_cmd_panel(dpufd)) {
		mipi_dsi_set_cdphy_bit_clk_upt_cmd(dpufd, dpufd->mipi_dsi0_base, phy_ctrl);
		if (is_dual_mipi_panel(dpufd))
			mipi_dsi_set_cdphy_bit_clk_upt_cmd(dpufd, dpufd->mipi_dsi1_base, phy_ctrl);
	} else {
		set_reg(dpufd->mipi_dsi0_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, DSI_CLK_BW, DSI_CLK_BS);
		mipi_dsi_set_cdphy_bit_clk_upt_video(dpufd, dpufd->mipi_dsi0_base, phy_ctrl);
		set_reg(dpufd->mipi_dsi0_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, DSI_CLK_BW, DSI_CLK_BS);
		if (is_dual_mipi_panel(dpufd)) {
			set_reg(dpufd->mipi_dsi1_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, DSI_CLK_BW, DSI_CLK_BS);
			mipi_dsi_set_cdphy_bit_clk_upt_video(dpufd, dpufd->mipi_dsi1_base, phy_ctrl);
			set_reg(dpufd->mipi_dsi1_base + MIPIDSI_LPCLK_CTRL_OFFSET,
				0x1, DSI_CLK_BW, DSI_CLK_BS);
			DPU_FB_DEBUG("end, phy1 status: 0x%x, vstate: 0x%x,\n",
				inp32(dpufd->mipi_dsi1_base + MIPIDSI_PHY_STATUS_OFFSET),
				inp32(dpufd->mipi_dsi1_base + MIPI_LDI_VSTATE));
		}
		pinfo->esd_enable = esd_enable;
		enable_ldi(dpufd);
	}
	DPU_FB_DEBUG("end, phy status: 0x%x, vstate: 0x%x,\n",
		inp32(dpufd->mipi_dsi0_base + MIPIDSI_PHY_STATUS_OFFSET),
		inp32(dpufd->mipi_dsi0_base + MIPI_LDI_VSTATE));
}

int mipi_dsi_bit_clk_upt_isr_handler(
	struct dpu_fb_data_type *dpufd)
{
	struct mipi_dsi_phy_ctrl phy_ctrl = {0};
	struct dpu_panel_info *pinfo = NULL;
	bool is_ready = false;
	uint8_t esd_enable;
	uint64_t lane_byte_clk;
	uint32_t dsi_bit_clk_upt;

	dpu_check_and_return(!dpufd, 0, ERR, "dpufd is NULL!\n");

	pinfo = &(dpufd->panel_info);
	dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk_upt;

	dpu_check_and_return((dpufd->index != PRIMARY_PANEL_IDX), 0, ERR, "fb%d, not support!\n", dpufd->index);

	if (!spin_trylock(&dpufd->mipi_resource_lock)) {
		DPU_FB_INFO("dsi_bit_clk %u will update in next frame\n", dsi_bit_clk_upt);
		return 0;
	}

	dpufd->mipi_dsi_bit_clk_update = 1;
	esd_enable = pinfo->esd_enable;
	if (is_mipi_video_panel(dpufd)) {
		pinfo->esd_enable = 0;
		disable_ldi(dpufd);
	}

	spin_unlock(&dpufd->mipi_resource_lock);

	if (dsi_bit_clk_upt == pinfo->mipi.dsi_bit_clk) {
		dpufd->mipi_dsi_bit_clk_update = 0;
		return 0;
	}

	if (pinfo->mipi.phy_mode == CPHY_MODE)
		get_dsi_cphy_ctrl(dpufd, &phy_ctrl);
	else
		get_dsi_dphy_ctrl(dpufd, &phy_ctrl);

	lane_byte_clk = dpufd->panel_info.dsi_phy_ctrl.lane_byte_clk;
	if (dpufd->panel_info.mipi.phy_mode == CPHY_MODE)
		lane_byte_clk = dpufd->panel_info.dsi_phy_ctrl.lane_word_clk;

	is_ready = mipi_dsi_check_ldi_vstate(dpufd, lane_byte_clk);
	if (!is_ready) {
		if (is_mipi_video_panel(dpufd)) {
			pinfo->esd_enable = esd_enable;
			enable_ldi(dpufd);
		}
		dpufd->mipi_dsi_bit_clk_update = 0;
		DPU_FB_INFO("PERI_STAT0 or ldi vstate is not ready\n");
		return 0;
	}

	mipi_set_cdphy_bit_clk(dpufd, &phy_ctrl, pinfo, esd_enable);

	DPU_FB_INFO("Mipi clk success changed from %d M switch to %d M\n", pinfo->mipi.dsi_bit_clk, dsi_bit_clk_upt);

	pinfo->dsi_phy_ctrl = phy_ctrl;
	pinfo->mipi.dsi_bit_clk = dsi_bit_clk_upt;
	dpufd->mipi_dsi_bit_clk_update = 0;

	return 0;
}

