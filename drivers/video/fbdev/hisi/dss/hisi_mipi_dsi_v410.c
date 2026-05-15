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

#include "hisi_mipi_dsi.h"

#define ROUND1(x,y)	((x) / (y) + ((x) % (y) > 0 ? 1 : 0))
#define DSS_REDUCE(x)	((x) > 0 ? ((x) - 1) : (x))

int mipi_dsi_ulps_cfg(struct hisi_fb_data_type *hisifd, int enable);

/*lint -e834 */
static uint32_t get_data_t_hs_prepare(struct hisi_fb_data_type *hisifd, uint32_t accuracy, uint32_t ui)
{
	struct hisi_panel_info *pinfo;
	uint32_t data_t_hs_prepare = 0;

	if ( NULL == hisifd) {
		HISI_FB_ERR("hisifd is null.\n");
		return 0;
	}
	pinfo = &(hisifd->panel_info);

	if ( NULL == pinfo) {
		HISI_FB_ERR("pinfo is null.\n");
		return 0;
	}

	if (pinfo->mipi.data_t_hs_prepare_adjust == 0)
		pinfo->mipi.data_t_hs_prepare_adjust = 35;

	data_t_hs_prepare = ((400  * accuracy + 4 * ui + pinfo->mipi.data_t_hs_prepare_adjust * ui) <= (850 * accuracy + 6 * ui - 8 * ui)) ?
		(400  * accuracy + 4 * ui + pinfo->mipi.data_t_hs_prepare_adjust * ui) : (850 * accuracy + 6 * ui - 8 * ui);

	return data_t_hs_prepare;
}

static uint32_t get_data_pre_delay(uint32_t lp11_flag, struct mipi_dsi_phy_ctrl *phy_ctrl, uint32_t clk_pre)
{
	uint32_t data_pre_delay = 0;
	//if use 1080 X 2160 resolution panel,need reduce the lp11 time,and disable noncontinue mode
	if (MIPI_SHORT_LP11 != lp11_flag) {
		data_pre_delay = phy_ctrl->clk_pre_delay + 2 + phy_ctrl->clk_t_lpx +
			phy_ctrl->clk_t_hs_prepare + phy_ctrl->clk_t_hs_zero + 8 + clk_pre ;
	}
	return data_pre_delay;
}

static uint32_t get_data_pre_delay_reality(uint32_t lp11_flag, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	uint32_t data_pre_delay_reality = 0;
	//if use 1080 X 2160 resolution panel,need reduce the lp11 time,and disable noncontinue mode
	if (MIPI_SHORT_LP11 != lp11_flag) {
		data_pre_delay_reality = phy_ctrl->data_pre_delay + 5;
	}
	return data_pre_delay_reality;
}

static uint32_t get_clk_post_delay_reality(uint32_t lp11_flag, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	uint32_t clk_post_delay_reality = 0;
	//if use 1080 X 2160 resolution panel,need reduce the lp11 time,and disable noncontinue mode
	if (MIPI_SHORT_LP11 != lp11_flag) {
		clk_post_delay_reality = phy_ctrl->clk_post_delay + 4;
	}
	return clk_post_delay_reality;
}
/*lint -e647 */
static void get_dsi_dphy_ctrl(struct hisi_fb_data_type *hisifd, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	struct hisi_panel_info *pinfo;
	uint32_t dsi_bit_clk;

	uint32_t ui;
	uint32_t m_pll;
	uint32_t n_pll;
	uint64_t lane_clock;
	uint64_t vco_div = 1;

	uint32_t accuracy;
	uint32_t unit_tx_byte_clk_hs;
	uint32_t clk_post;
	uint32_t clk_pre;
	uint32_t clk_t_hs_exit;
	uint32_t clk_pre_delay;
	uint32_t clk_t_hs_prepare;
	uint32_t clk_t_lpx;
	uint32_t clk_t_hs_zero;
	uint32_t clk_t_hs_trial;
	uint32_t data_post_delay;
	uint32_t data_t_hs_prepare;
	uint32_t data_t_hs_zero;
	uint32_t data_t_hs_trial;
	uint32_t data_t_lpx;

	if (NULL == phy_ctrl || NULL == hisifd) {
		HISI_FB_ERR("hisifd or phyctrl is null.\n");
		return;
	}
	pinfo = &(hisifd->panel_info);

	dsi_bit_clk = pinfo->mipi.dsi_bit_clk_upt;
	lane_clock = (uint64_t)(2 * dsi_bit_clk);
	HISI_FB_INFO("Expected : lane_clock = %llu M\n", lane_clock);

	/************************  PLL parameters config  *********************/
	//chip spec :
	//If the output data rate is below 320 Mbps, RG_BNAD_SEL should be set to 1.
	//At this mode a post divider of 1/4 will be applied to VCO.
	if ((320 <= lane_clock) && (lane_clock <= 2500)) {
		phy_ctrl->rg_band_sel = 0;
		vco_div = 1;
	} else if ((80 <= lane_clock) && (lane_clock < 320)) {
		phy_ctrl->rg_band_sel = 1;
		vco_div = 4;
	} else {
		HISI_FB_ERR("80M <= lane_clock< = 2500M, not support lane_clock = %llu M\n", lane_clock);
	}

	n_pll = 2;
	m_pll = (uint32_t)(lane_clock * vco_div * n_pll * 1000000UL / DEFAULT_MIPI_CLK_RATE);

	lane_clock = m_pll * (DEFAULT_MIPI_CLK_RATE / n_pll) / vco_div;
	if (lane_clock > 750000000) {
		phy_ctrl->rg_cp = 3;
	} else if ((80000000 <= lane_clock) && (lane_clock <= 750000000)) {
		phy_ctrl->rg_cp = 1;
	} else {
		HISI_FB_ERR("80M <= lane_clock< = 2500M, not support lane_clock = %llu M.\n", lane_clock);
	}

	//chip spec :
	phy_ctrl->rg_pre_div = n_pll - 1;
	phy_ctrl->rg_div = m_pll;
	phy_ctrl->rg_0p8v = 0;
	phy_ctrl->rg_2p5g = 1;
	phy_ctrl->rg_320m = 0;
	phy_ctrl->rg_lpf_r = 0;

	//TO DO HSTX select VCM VREF
	phy_ctrl->rg_vrefsel_vcm = 0x5d;

	/********************  clock/data lane parameters config  ******************/
	accuracy = 10;
	ui =  (uint32_t)(10 * 1000000000UL * accuracy / lane_clock);
	//unit of measurement
	unit_tx_byte_clk_hs = 8 * ui;

	// D-PHY Specification : 60ns + 52*UI <= clk_post
	clk_post = 600 * accuracy + 52 * ui + unit_tx_byte_clk_hs + pinfo->mipi.clk_post_adjust * ui;

	// D-PHY Specification : clk_pre >= 8*UI
	clk_pre = 8 * ui + unit_tx_byte_clk_hs + pinfo->mipi.clk_pre_adjust * ui;

	// D-PHY Specification : clk_t_hs_exit >= 100ns
	clk_t_hs_exit = 1000 * accuracy + 100 * accuracy + pinfo->mipi.clk_t_hs_exit_adjust * ui;

	// clocked by TXBYTECLKHS
	clk_pre_delay = 0 + pinfo->mipi.clk_pre_delay_adjust * ui;

	// D-PHY Specification : clk_t_hs_trial >= 60ns
	// clocked by TXBYTECLKHS
	clk_t_hs_trial = 600 * accuracy + 3 * unit_tx_byte_clk_hs + pinfo->mipi.clk_t_hs_trial_adjust * ui;

	// D-PHY Specification : 38ns <= clk_t_hs_prepare <= 95ns
	// clocked by TXBYTECLKHS
	clk_t_hs_prepare = 660 * accuracy;

	// clocked by TXBYTECLKHS
	data_post_delay = 0 + pinfo->mipi.data_post_delay_adjust * ui;

	// D-PHY Specification : data_t_hs_trial >= max( n*8*UI, 60ns + n*4*UI ), n = 1
	// clocked by TXBYTECLKHS
	data_t_hs_trial = ((600 * accuracy + 4 * ui) >= (8 * ui) ? (600 * accuracy + 4 * ui) : (8 * ui)) + 8 * ui +
		3 * unit_tx_byte_clk_hs + pinfo->mipi.data_t_hs_trial_adjust * ui;

	// D-PHY Specification : 40ns + 4*UI <= data_t_hs_prepare <= 85ns + 6*UI
	// clocked by TXBYTECLKHS
	data_t_hs_prepare = get_data_t_hs_prepare(hisifd, accuracy, ui);
	// D-PHY chip spec : clk_t_lpx + clk_t_hs_prepare > 200ns
	// D-PHY Specification : clk_t_lpx >= 50ns
	// clocked by TXBYTECLKHS
	clk_t_lpx = (uint32_t)(2000 * accuracy + 10 * accuracy + pinfo->mipi.clk_t_lpx_adjust * ui - clk_t_hs_prepare);

	// D-PHY Specification : clk_t_hs_zero + clk_t_hs_prepare >= 300 ns
	// clocked by TXBYTECLKHS
	clk_t_hs_zero = (uint32_t)(3000 * accuracy + 3 * unit_tx_byte_clk_hs + pinfo->mipi.clk_t_hs_zero_adjust * ui - clk_t_hs_prepare);

	// D-PHY chip spec : data_t_lpx + data_t_hs_prepare > 200ns
	// D-PHY Specification : data_t_lpx >= 50ns
	// clocked by TXBYTECLKHS
	data_t_lpx = (uint32_t)(2000 * accuracy + 10 * accuracy + pinfo->mipi.data_t_lpx_adjust * ui - data_t_hs_prepare);

	// D-PHY Specification : data_t_hs_zero + data_t_hs_prepare >= 145ns + 10*UI
	// clocked by TXBYTECLKHS
	data_t_hs_zero = (uint32_t)(1450 * accuracy + 10 * ui +
		3 * unit_tx_byte_clk_hs + pinfo->mipi.data_t_hs_zero_adjust * ui - data_t_hs_prepare);

	phy_ctrl->clk_pre_delay = ROUND1(clk_pre_delay, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_prepare = ROUND1(clk_t_hs_prepare, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_lpx = ROUND1(clk_t_lpx, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_zero = ROUND1(clk_t_hs_zero, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_trial = ROUND1(clk_t_hs_trial, unit_tx_byte_clk_hs);

	phy_ctrl->data_post_delay = ROUND1(data_post_delay, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_prepare = ROUND1(data_t_hs_prepare, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_lpx = ROUND1(data_t_lpx, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_zero = ROUND1(data_t_hs_zero, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_trial = ROUND1(data_t_hs_trial, unit_tx_byte_clk_hs);

	phy_ctrl->clk_post_delay = phy_ctrl->data_t_hs_trial + ROUND1(clk_post, unit_tx_byte_clk_hs);
	phy_ctrl->data_pre_delay = get_data_pre_delay(pinfo->mipi.lp11_flag, phy_ctrl, ROUND1(clk_pre, unit_tx_byte_clk_hs));

	phy_ctrl->clk_lane_lp2hs_time = phy_ctrl->clk_pre_delay + phy_ctrl->clk_t_lpx + phy_ctrl->clk_t_hs_prepare +
		phy_ctrl->clk_t_hs_zero + 5 + 7;
	phy_ctrl->clk_lane_hs2lp_time = phy_ctrl->clk_t_hs_trial + phy_ctrl->clk_post_delay + 8 + 4;
	phy_ctrl->data_lane_lp2hs_time = get_data_pre_delay_reality(pinfo->mipi.lp11_flag, phy_ctrl) + phy_ctrl->data_t_lpx + phy_ctrl->data_t_hs_prepare +
		phy_ctrl->data_t_hs_zero + 7;
	phy_ctrl->data_lane_hs2lp_time = phy_ctrl->data_t_hs_trial + 8 + 5;

	phy_ctrl->phy_stop_wait_time = get_clk_post_delay_reality(pinfo->mipi.lp11_flag, phy_ctrl) + phy_ctrl->clk_t_hs_trial +
		ROUND1(clk_t_hs_exit, unit_tx_byte_clk_hs) - (phy_ctrl->data_post_delay + 4 + phy_ctrl->data_t_hs_trial) + 3;

	phy_ctrl->lane_byte_clk = lane_clock / 8;
	phy_ctrl->clk_division = (((phy_ctrl->lane_byte_clk / 2) % pinfo->mipi.max_tx_esc_clk) > 0) ?
		(uint32_t)(phy_ctrl->lane_byte_clk / 2 / pinfo->mipi.max_tx_esc_clk + 1) :
		(uint32_t)(phy_ctrl->lane_byte_clk / 2 / pinfo->mipi.max_tx_esc_clk);

	HISI_FB_INFO("DPHY clock_lane and data_lane config : \n"
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
/*lint +e647 */
static void get_dsi_cphy_ctrl(struct hisi_fb_data_type *hisifd, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	struct hisi_panel_info *pinfo;

	uint32_t ui;
	uint32_t m_pll;
	uint32_t n_pll;
	uint64_t lane_clock;
	uint64_t vco_div = 1;

	uint32_t accuracy;
	uint32_t unit_tx_word_clk_hs;

	if (NULL == hisifd || NULL == phy_ctrl) {
		HISI_FB_ERR("hisifd or phy_ctrl is null.\n");
		return;
	}
	pinfo = &(hisifd->panel_info);

	lane_clock = pinfo->mipi.dsi_bit_clk_upt;
	HISI_FB_INFO("Expected : lane_clock = %llu M\n", lane_clock);

	/************************  PLL parameters config  *********************/
	//chip spec :
	//C PHY Data rate range is from 1500 Mbps to 40 Mbps
	if ((320 <= lane_clock) && (lane_clock <= 1500)) {
		phy_ctrl->rg_cphy_div = 0;
		vco_div = 1;
	} else if ((160 <= lane_clock) && (lane_clock < 320)) {
		phy_ctrl->rg_cphy_div = 1;
		vco_div = 2;
	} else if ((80 <= lane_clock) && (lane_clock < 160)) {
		phy_ctrl->rg_cphy_div = 2;
		vco_div = 4;
	} else if ((40 <= lane_clock) && (lane_clock < 80)) {
		phy_ctrl->rg_cphy_div = 3;
		vco_div = 8;
	} else {
		HISI_FB_ERR("40M <= lane_clock< = 1500M, not support lane_clock = %llu M\n", lane_clock);
	}

	n_pll = 2;
	m_pll = (uint32_t)(lane_clock * vco_div * n_pll * 1000000UL / DEFAULT_MIPI_CLK_RATE);

	lane_clock = m_pll * (DEFAULT_MIPI_CLK_RATE / n_pll) / vco_div;
	if (lane_clock > 750000000) {
		phy_ctrl->rg_cp = 3;
	} else if ((40000000 <= lane_clock) && (lane_clock <= 750000000)) {
		phy_ctrl->rg_cp = 1;
	} else {
		HISI_FB_ERR("40M <= lane_clock< = 1500M, not support lane_clock = %llu M.\n", lane_clock);
	}

	//chip spec :
	phy_ctrl->rg_pre_div = n_pll - 1;
	phy_ctrl->rg_div = m_pll;
	phy_ctrl->rg_0p8v = 0;
	phy_ctrl->rg_2p5g = 1;
	phy_ctrl->rg_320m = 0;
	phy_ctrl->rg_lpf_r = 0;

	//TO DO HSTX select VCM VREF
	phy_ctrl->rg_vrefsel_vcm = 0x51;

	/********************  data lane parameters config  ******************/
	accuracy = 10;
	ui = (uint32_t)(10 * 1000000000UL * accuracy / lane_clock);
	//unit of measurement
	unit_tx_word_clk_hs = 7 * ui;

	//CPHY Specification: 38ns <= t3_prepare <= 95ns
	phy_ctrl->t_prepare = 650 * accuracy;//380 * accuracy - unit_tx_word_clk_hs;

	//CPHY Specification: 50ns <= t_lpx
	phy_ctrl->t_lpx = 600 * accuracy + 8 * ui - unit_tx_word_clk_hs;

	//CPHY Specification: 7*UI <= t_prebegin <= 448UI
	phy_ctrl->t_prebegin = 350 * ui - unit_tx_word_clk_hs;

	//CPHY Specification: 7*UI <= t_post <= 224*UI
	phy_ctrl->t_post = 224 * ui - unit_tx_word_clk_hs;//224 * ui;

	phy_ctrl->t_prepare = ROUND1(phy_ctrl->t_prepare, unit_tx_word_clk_hs);
	phy_ctrl->t_lpx = ROUND1(phy_ctrl->t_lpx, unit_tx_word_clk_hs);
	phy_ctrl->t_prebegin = ROUND1(phy_ctrl->t_prebegin, unit_tx_word_clk_hs);
	phy_ctrl->t_post = ROUND1(phy_ctrl->t_post, unit_tx_word_clk_hs);

	phy_ctrl->data_lane_lp2hs_time = phy_ctrl->t_lpx + phy_ctrl->t_prepare + phy_ctrl->t_prebegin + 5 + 17;
	phy_ctrl->data_lane_hs2lp_time = phy_ctrl->t_post + 8 + 5;

	phy_ctrl->lane_word_clk = lane_clock / 7;
	phy_ctrl->clk_division = (((phy_ctrl->lane_word_clk / 2) % pinfo->mipi.max_tx_esc_clk) > 0) ?
		(uint32_t)(phy_ctrl->lane_word_clk / 2 / pinfo->mipi.max_tx_esc_clk + 1) :
		(uint32_t)(phy_ctrl->lane_word_clk / 2 / pinfo->mipi.max_tx_esc_clk);

	phy_ctrl->phy_stop_wait_time = phy_ctrl->t_post + 8 + 5;

	HISI_FB_INFO("CPHY clock_lane and data_lane config : \n"
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
/* lint +e834 */
static uint32_t mipi_pixel_clk(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	pinfo = &(hisifd->panel_info);//lint !e838

	if ((pinfo->pxl_clk_rate_div == 0) || (g_fpga_flag == 1)) {
		return (uint32_t)pinfo->pxl_clk_rate;
	}

	if ((pinfo->ifbc_type == IFBC_TYPE_NONE) &&
		!is_dual_mipi_panel(hisifd)) {
		pinfo->pxl_clk_rate_div = 1;
	}

	return (uint32_t)pinfo->pxl_clk_rate / pinfo->pxl_clk_rate_div;
}
/*lint -e715*/
static void mipi_config_phy_test_code(char __iomem *mipi_dsi_base, uint32_t test_code_addr, uint32_t test_code_parameter)
{
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL1_OFFSET, test_code_addr);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000002);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL1_OFFSET, test_code_parameter);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000002);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);
}
/*lint +e715*/
static void mipi_config_cphy_spec1v0_parameter(char __iomem *mipi_dsi_base, struct hisi_panel_info *pinfo)
{
	uint32_t i;
	uint32_t addr = 0;

	for (i = 0; i <= pinfo->mipi.lane_nums; i++) {
		//Lane Transmission Property
		addr = MIPIDSI_PHY_TST_LANE_TRANSMISSION_PROPERTY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, 0x43);

		//Lane Timing Control - DPHY: THS-PREPARE/CPHY: T3-PREPARE
		addr = MIPIDSI_PHY_TST_DATA_PREPARE + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.t_prepare));

		//Lane Timing Control - TLPX
		addr = MIPIDSI_PHY_TST_DATA_TLPX + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.t_lpx));

		//Lane Timing Control - DPHY: THS-ZERO/CPHY: T3-PREBEGIN
		addr = MIPIDSI_PHY_TST_DATA_ZERO + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.t_prebegin));

		//Lane Timing Control - DPHY: THS-TRAIL/CPHY: T3-POST
		addr = MIPIDSI_PHY_TST_DATA_TRAIL + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.t_post));
	}
}

static void mipi_config_dphy_spec1v2_parameter(char __iomem *mipi_dsi_base, struct hisi_panel_info *pinfo)
{
	uint32_t i;
	uint32_t addr = 0;

	for (i = 0; i <= (pinfo->mipi.lane_nums + 1); i++) {
		//Lane Transmission Property
		addr = MIPIDSI_PHY_TST_LANE_TRANSMISSION_PROPERTY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, 0x43);
	}

	//pre_delay of clock lane request setting
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_PRE_DELAY, DSS_REDUCE(pinfo->dsi_phy_ctrl.clk_pre_delay));

	//post_delay of clock lane request setting
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_POST_DELAY, DSS_REDUCE(pinfo->dsi_phy_ctrl.clk_post_delay));

	//clock lane timing ctrl - t_lpx
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_TLPX, DSS_REDUCE(pinfo->dsi_phy_ctrl.clk_t_lpx));

	//clock lane timing ctrl - t_hs_prepare
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_PREPARE, DSS_REDUCE(pinfo->dsi_phy_ctrl.clk_t_hs_prepare));

	//clock lane timing ctrl - t_hs_zero
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_ZERO, DSS_REDUCE(pinfo->dsi_phy_ctrl.clk_t_hs_zero));

	//clock lane timing ctrl - t_hs_trial
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_TRAIL, DSS_REDUCE(pinfo->dsi_phy_ctrl.clk_t_hs_trial));

	for (i = 0; i <= (pinfo->mipi.lane_nums + 1); i++) {//lint !e850
		if (i == 2) {
			i++;  //addr: lane0:0x60; lane1:0x80; lane2:0xC0; lane3:0xE0
		}

		//data lane pre_delay
		addr = MIPIDSI_PHY_TST_DATA_PRE_DELAY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.data_pre_delay));

		//data lane post_delay
		addr = MIPIDSI_PHY_TST_DATA_POST_DELAY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.data_post_delay));

		//data lane timing ctrl - t_lpx
		addr = MIPIDSI_PHY_TST_DATA_TLPX + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.data_t_lpx));

		//data lane timing ctrl - t_hs_prepare
		addr = MIPIDSI_PHY_TST_DATA_PREPARE + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.data_t_hs_prepare));

		//data lane timing ctrl - t_hs_zero
		addr = MIPIDSI_PHY_TST_DATA_ZERO + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.data_t_hs_zero));

		//data lane timing ctrl - t_hs_trial
		addr = MIPIDSI_PHY_TST_DATA_TRAIL + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(pinfo->dsi_phy_ctrl.data_t_hs_trial));

		HISI_FB_INFO("DPHY spec1v2 config : \n"
			"addr=0x%x\n"
			"clk_pre_delay=%u\n"
			"clk_t_hs_trial=%u\n"
			"data_t_hs_zero=%u\n"
			"data_t_lpx=%u\n"
			"data_t_hs_prepare=%u\n",
			addr,
			pinfo->dsi_phy_ctrl.clk_pre_delay,
			pinfo->dsi_phy_ctrl.clk_t_hs_trial,
			pinfo->dsi_phy_ctrl.data_t_hs_zero,
			pinfo->dsi_phy_ctrl.data_t_lpx,
			pinfo->dsi_phy_ctrl.data_t_hs_prepare);
	}
}

void mipi_init(struct hisi_fb_data_type *hisifd, char __iomem *mipi_dsi_base)
{
	uint32_t hline_time = 0;
	uint32_t hsa_time = 0;
	uint32_t hbp_time = 0;
	uint64_t pixel_clk = 0;
	unsigned long dw_jiffies = 0;
	uint32_t tmp = 0;
	bool is_ready = false;
	struct hisi_panel_info *pinfo = NULL;
	dss_rect_t rect;
	uint32_t cmp_stopstate_val = 0;

	//auto ulps
	//uint32_t pll_off_ulps;
	uint32_t auto_ulps_enter_delay;
	uint32_t twakeup_cnt;
	uint32_t twakeup_clk_div;

	pinfo = &(hisifd->panel_info);//lint !e838

	if (pinfo->mipi.max_tx_esc_clk == 0) {
		HISI_FB_ERR("fb%d, max_tx_esc_clk is invalid!", hisifd->index);
		pinfo->mipi.max_tx_esc_clk = DEFAULT_MAX_TX_ESC_CLK;
	}

	memset(&(pinfo->dsi_phy_ctrl), 0, sizeof(struct mipi_dsi_phy_ctrl));

	if (pinfo->mipi.phy_mode == CPHY_MODE) {
		get_dsi_cphy_ctrl(hisifd, &(pinfo->dsi_phy_ctrl));
	} else {
		get_dsi_dphy_ctrl(hisifd, &(pinfo->dsi_phy_ctrl));
	}

	rect.x = 0;
	rect.y = 0;
	rect.w = pinfo->xres;//lint !e713
	rect.h = pinfo->yres;//lint !e713

	mipi_ifbc_get_rect(hisifd, &rect);

	/*************************Configure the PHY start*************************/

	set_reg(mipi_dsi_base + MIPIDSI_PHY_IF_CFG_OFFSET, pinfo->mipi.lane_nums, 2, 0);
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_division, 8, 0);
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_division, 8, 8);

	outp32(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x00000000);

	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000001);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);

	if (pinfo->mipi.phy_mode == CPHY_MODE) {
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010001, 0x3f);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010042, 0x21);

		//PLL configuration I
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010046, pinfo->dsi_phy_ctrl.rg_cp + (pinfo->dsi_phy_ctrl.rg_lpf_r << 4));

		//PLL configuration II
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010048, pinfo->dsi_phy_ctrl.rg_0p8v + (pinfo->dsi_phy_ctrl.rg_2p5g << 1) +
			(pinfo->dsi_phy_ctrl.rg_320m << 2) + (pinfo->dsi_phy_ctrl.rg_band_sel << 3) + (pinfo->dsi_phy_ctrl.rg_cphy_div << 4));

		//PLL configuration III
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010049, pinfo->dsi_phy_ctrl.rg_pre_div);

		//PLL configuration IV
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A, pinfo->dsi_phy_ctrl.rg_div);

		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004F, 0xf0);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010052, 0xa8);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010053, 0xc2);

		if ((g_fpga_flag==0) && (g_dss_version_tag & (FB_ACCEL_KIRIN970 | FB_ACCEL_DSSV501))) {
			/*enable BTA*/
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010054, 0x07);
		}
		//PLL update control
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);

		//set cphy spec parameter
		mipi_config_cphy_spec1v0_parameter(mipi_dsi_base, pinfo);
	} else {
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010042, 0x21);
		//PLL configuration I
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010046, pinfo->dsi_phy_ctrl.rg_cp + (pinfo->dsi_phy_ctrl.rg_lpf_r << 4));

		//PLL configuration II
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010048, pinfo->dsi_phy_ctrl.rg_0p8v + (pinfo->dsi_phy_ctrl.rg_2p5g << 1) +
			(pinfo->dsi_phy_ctrl.rg_320m << 2) + (pinfo->dsi_phy_ctrl.rg_band_sel << 3));

		//PLL configuration III
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010049, pinfo->dsi_phy_ctrl.rg_pre_div);

		//PLL configuration IV
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A, pinfo->dsi_phy_ctrl.rg_div);

		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004F, 0xf0);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010050, 0xc0);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010051, 0x22);

		mipi_config_phy_test_code(mipi_dsi_base, 0x00010053, pinfo->dsi_phy_ctrl.rg_vrefsel_vcm);

		if ((g_fpga_flag==0) && (g_dss_version_tag & (FB_ACCEL_KIRIN970 | FB_ACCEL_DSSV501))) {
			/*enable BTA*/
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010054, 0x03);
		}
		//PLL update control
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);
		//set dphy spec parameter
		mipi_config_dphy_spec1v2_parameter(mipi_dsi_base, pinfo);
	}

	outp32(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x0000000F);

	is_ready = false;
	dw_jiffies = jiffies + HZ / 2;
	do {
		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((tmp & 0x00000001) == 0x00000001) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	if (!is_ready) {
		HISI_FB_INFO("fb%d, phylock is not ready!MIPIDSI_PHY_STATUS_OFFSET=0x%x.\n",
			hisifd->index, tmp);
	}

	if (pinfo->mipi.lane_nums >= DSI_4_LANES) {
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9) | BIT(11));
	} else if (pinfo->mipi.lane_nums >= DSI_3_LANES) {
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9));
	} else if (pinfo->mipi.lane_nums >= DSI_2_LANES) {
		cmp_stopstate_val = (BIT(4) | BIT(7));
	} else {
		cmp_stopstate_val = (BIT(4));
	}

	is_ready = false;
	dw_jiffies = jiffies + HZ / 2;
	do {
		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((tmp & cmp_stopstate_val) == cmp_stopstate_val) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	if (!is_ready) {
		HISI_FB_INFO("fb%d, phystopstateclklane is not ready! MIPIDSI_PHY_STATUS_OFFSET=0x%x.\n",
			hisifd->index, tmp);
	}

	/*************************Configure the PHY end*************************/
	if (is_mipi_cmd_panel(hisifd)) {
		// config to command mode
		set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 0);
		// ALLOWED_CMD_SIZE
		set_reg(mipi_dsi_base + MIPIDSI_EDPI_CMD_SIZE_OFFSET, rect.w, 16, 0);

		// cnt=2 in update-patial scene, cnt nees to be checked for different panels
		if (pinfo->mipi.hs_wr_to_time == 0) {
			set_reg(mipi_dsi_base + MIPIDSI_HS_WR_TO_CNT_OFFSET, 0x1000002, 25, 0);
		} else {
			set_reg(mipi_dsi_base + MIPIDSI_HS_WR_TO_CNT_OFFSET,
				(0x1 << 24) | (pinfo->mipi.hs_wr_to_time * pinfo->dsi_phy_ctrl.lane_byte_clk / 1000000000UL), 25, 0);
		}

		// FIXME: test tearing effect, if use gpio, no need
		//set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 0);
	}

	// phy_stop_wait_time
	set_reg(mipi_dsi_base + MIPIDSI_PHY_IF_CFG_OFFSET, pinfo->dsi_phy_ctrl.phy_stop_wait_time, 8, 8);

	//--------------configuring the DPI packet transmission----------------
	/*
	** 2. Configure the DPI Interface:
	** This defines how the DPI interface interacts with the controller.
	*/
	set_reg(mipi_dsi_base + MIPIDSI_DPI_VCID_OFFSET, pinfo->mipi.vc, 2, 0);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_COLOR_CODING_OFFSET, pinfo->mipi.color_mode, 4, 0);

	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, pinfo->ldi.data_en_plr, 1, 0);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, pinfo->ldi.vsync_plr, 1, 1);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, pinfo->ldi.hsync_plr, 1, 2);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, 0x0, 1, 3);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, 0x0, 1, 4);


	/*
	** 3. Select the Video Transmission Mode:
	** This defines how the processor requires the video line to be
	** transported through the DSI link.
	*/
	// video mode: low power mode
	if (MIPI_DISABLE_LP11 == pinfo->mipi.lp11_flag) {
		set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x0f, 6, 8);
		HISI_FB_INFO("set_reg MIPIDSI_VID_MODE_CFG_OFFSET 0x0f \n");
	} else {
		set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x3f, 6, 8);
		HISI_FB_INFO("set_reg MIPIDSI_VID_MODE_CFG_OFFSET 0x3f \n");
	}
	/* set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x0, 1, 14); */
	if (is_mipi_video_panel(hisifd)) {
		// TODO: fix blank display bug when set backlight
		set_reg(mipi_dsi_base + MIPIDSI_DPI_LP_CMD_TIM_OFFSET, 0x4, 8, 16);
		// video mode: send read cmd by lp mode
		set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x1, 1, 15);
	}

	if ((pinfo->mipi.dsi_version == DSI_1_2_VERSION)
		&& (is_mipi_video_panel(hisifd))
		&& ((pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE)
			|| (pinfo->ifbc_type == IFBC_TYPE_VESA3X_DUAL))) {

		set_reg(mipi_dsi_base + MIPIDSI_VID_PKT_SIZE_OFFSET, rect.w * pinfo->pxl_clk_rate_div, 14, 0);
		// video vase3x must be set BURST mode
		if (pinfo->mipi.burst_mode < DSI_BURST_SYNC_PULSES_1) {
			HISI_FB_INFO("pinfo->mipi.burst_mode = %d. video need config BURST mode\n", pinfo->mipi.burst_mode);
			pinfo->mipi.burst_mode = DSI_BURST_SYNC_PULSES_1;
		}
	} else {
		set_reg(mipi_dsi_base + MIPIDSI_VID_PKT_SIZE_OFFSET, rect.w, 14, 0);
	}

	// burst mode
	set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, pinfo->mipi.burst_mode, 2, 0);
	// for dsi read, BTA enable
	set_reg(mipi_dsi_base + MIPIDSI_PCKHDL_CFG_OFFSET, 0x1, 1, 2);

	/*
	** 4. Define the DPI Horizontal timing configuration:
	**
	** Hsa_time = HSA*(PCLK period/Clk Lane Byte Period);
	** Hbp_time = HBP*(PCLK period/Clk Lane Byte Period);
	** Hline_time = (HSA+HBP+HACT+HFP)*(PCLK period/Clk Lane Byte Period);
	*/
	pixel_clk = mipi_pixel_clk(hisifd);
	HISI_FB_INFO("pixel_clk = %llu\n", pixel_clk);

/*lint -e737 -e776 -e712*/
	if (pinfo->mipi.phy_mode == DPHY_MODE) {
		hsa_time = pinfo->ldi.h_pulse_width * pinfo->dsi_phy_ctrl.lane_byte_clk / pixel_clk;
		hbp_time = pinfo->ldi.h_back_porch * pinfo->dsi_phy_ctrl.lane_byte_clk / pixel_clk;
		hline_time = (pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch +
			rect.w + pinfo->ldi.h_front_porch) * pinfo->dsi_phy_ctrl.lane_byte_clk / pixel_clk;
	} else {
		hsa_time = pinfo->ldi.h_pulse_width * pinfo->dsi_phy_ctrl.lane_word_clk / pixel_clk;
		hbp_time = pinfo->ldi.h_back_porch * pinfo->dsi_phy_ctrl.lane_word_clk / pixel_clk;
		hline_time = (pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch +
			rect.w + pinfo->ldi.h_front_porch) * pinfo->dsi_phy_ctrl.lane_word_clk / pixel_clk;
	}
	HISI_FB_INFO("hsa_time = %d, hbp_time = %d, hline_time = %d \n", hsa_time, hbp_time, hline_time);
/*lint +e737 +e776 +e712*/

	set_reg(mipi_dsi_base + MIPIDSI_VID_HSA_TIME_OFFSET, hsa_time, 12, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_HBP_TIME_OFFSET, hbp_time, 12, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_HLINE_TIME_OFFSET, hline_time, 15, 0);

	// Define the Vertical line configuration
	set_reg(mipi_dsi_base + MIPIDSI_VID_VSA_LINES_OFFSET, pinfo->ldi.v_pulse_width, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_VBP_LINES_OFFSET, pinfo->ldi.v_back_porch, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_VFP_LINES_OFFSET, pinfo->ldi.v_front_porch, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_VACTIVE_LINES_OFFSET, rect.h, 14, 0);
	set_reg(mipi_dsi_base + MIPIDSI_TO_CNT_CFG_OFFSET, 0x7FF, 16, 0);

	// Configure core's phy parameters
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_lane_lp2hs_time, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_lane_hs2lp_time, 10, 16);

	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_RD_CFG_OFFSET, 0x7FFF, 15, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET, pinfo->dsi_phy_ctrl.data_lane_lp2hs_time, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET, pinfo->dsi_phy_ctrl.data_lane_hs2lp_time, 10, 16);

/*lint -e712*/
	//16~19bit:pclk_en, pclk_sel, dpipclk_en, dpipclk_sel
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET, 0x5, 4, 16);
	if (is_mipi_cmd_panel(hisifd)) {
		//pll_off_ulps = pinfo->mipi.non_continue_en ? 1 : 0;
		//(t_dpipclk*(ldi_hsw+ldi_hbp+ldi_hact+ldi_hfp))*1.5
		auto_ulps_enter_delay = hline_time * 3 / 2;
		twakeup_clk_div = 8;
		//twakeup_cnt*twakeup_clk_div*t_lanebyteclk>1ms
		if (pinfo->mipi.phy_mode == CPHY_MODE) {
			twakeup_cnt = pinfo->dsi_phy_ctrl.lane_word_clk / 1000 * 3 / 2 / twakeup_clk_div;
		} else {
			twakeup_cnt = pinfo->dsi_phy_ctrl.lane_byte_clk / 1000 * 3 / 2 / twakeup_clk_div;
		}

		//set_reg(mipi_dsi_base + AUTO_ULPS_MODE, pll_off_ulps, 1, 16);
		set_reg(mipi_dsi_base + AUTO_ULPS_ENTER_DELAY, auto_ulps_enter_delay, 32, 0);
		set_reg(mipi_dsi_base + AUTO_ULPS_WAKEUP_TIME, twakeup_clk_div, 16, 0);
		set_reg(mipi_dsi_base + AUTO_ULPS_WAKEUP_TIME, twakeup_cnt, 16, 16);
	}

	if (pinfo->mipi.phy_mode == CPHY_MODE) {
		//1:cphy
		set_reg(mipi_dsi_base + PHY_MODE, 0x1, 1, 0);
	} else {
		//0:dphy
		set_reg(mipi_dsi_base + PHY_MODE, 0x0, 1, 0);
	}
/*lint +e712*/
	// Waking up Core
	set_reg(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x1, 1, 0);
}

int mipi_dsi_clk_enable(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	struct clk *clk_tmp = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		clk_tmp = hisifd->dss_dphy0_ref_clk;
		if (clk_tmp) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_dphy0_ref_clk clk_prepare failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_dphy0_ref_clk clk_enable failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}
		}

		clk_tmp = hisifd->dss_dphy0_cfg_clk;
		if (clk_tmp) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_dphy0_cfg_clk clk_prepare failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_dphy0_cfg_clk clk_enable failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}
		}

		clk_tmp = hisifd->dss_pclk_dsi0_clk;
		if (clk_tmp) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_pclk_dsi0_clk clk_prepare failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_pclk_dsi0_clk clk_enable failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}
		}
	}


	if (is_dual_mipi_panel(hisifd) || (hisifd->index == EXTERNAL_PANEL_IDX)) {
		clk_tmp = hisifd->dss_dphy1_ref_clk;
		if (clk_tmp) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_dphy1_ref_clk clk_prepare failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_dphy1_ref_clk clk_enable failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}
		}

		clk_tmp = hisifd->dss_dphy1_cfg_clk;
		if (clk_tmp) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_dphy1_cfg_clk clk_prepare failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_dphy1_cfg_clk clk_enable failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}
		}

		clk_tmp = hisifd->dss_pclk_dsi1_clk;
		if (clk_tmp) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_pclk_dsi1_clk clk_prepare failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_pclk_dsi1_clk clk_enable failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}
		}
	}

	return 0;
}

int mipi_dsi_clk_disable(struct hisi_fb_data_type *hisifd)
{
	struct clk *clk_tmp = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		clk_tmp = hisifd->dss_dphy0_ref_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}

		clk_tmp = hisifd->dss_dphy0_cfg_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}

		clk_tmp = hisifd->dss_pclk_dsi0_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}
	}


	if (is_dual_mipi_panel(hisifd) || (hisifd->index == EXTERNAL_PANEL_IDX)) {
		clk_tmp = hisifd->dss_dphy1_ref_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}

		clk_tmp = hisifd->dss_dphy1_cfg_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}

		clk_tmp = hisifd->dss_pclk_dsi1_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}
	}

	return 0;
}


/*******************************************************************************
**
*/
/*lint -e732*/
static int mipi_dsi_pll_status_check_ec(struct hisi_fb_data_type *hisifd,
	char __iomem *mipi_dsi_base)
{
	uint32_t tmp;
	uint32_t cmp_ulpsactivenot_val = 0;
	uint32_t cmp_stopstate_val = 0;
	uint32_t try_times;
	struct timeval tv0;
	struct timeval tv1;
	uint32_t redo_count = 0;

	if (NULL == hisifd || NULL == mipi_dsi_base) {
		HISI_FB_ERR("hisifd or mipi_dsi_base is null.\n");
		return 0;
	}

	HISI_FB_DEBUG("fb%d, +!\n", hisifd->index);

	hisifb_get_timestamp(&tv0);

	////////////////////////////////////////////////////////////////////////////
	//
	// enter ulps
	//
	if (hisifd->panel_info.mipi.lane_nums >= DSI_4_LANES) {
		cmp_ulpsactivenot_val = (BIT(5) | BIT(8) | BIT(10) | BIT(12));
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9) | BIT(11));
	} else if (hisifd->panel_info.mipi.lane_nums >= DSI_3_LANES) {
		cmp_ulpsactivenot_val = (BIT(5) | BIT(8) | BIT(10));
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9));
	} else if (hisifd->panel_info.mipi.lane_nums >= DSI_2_LANES) {
		cmp_ulpsactivenot_val = (BIT(5) | BIT(8));
		cmp_stopstate_val = (BIT(4) | BIT(7));
	} else {
		cmp_ulpsactivenot_val = (BIT(5));
		cmp_stopstate_val = (BIT(4));
	}

	mipi_config_phy_test_code(mipi_dsi_base, 0x00010050, 0x00000092);
	mipi_config_phy_test_code(mipi_dsi_base, 0x00010050, 0x00000093);

REDO:
	if (redo_count > 100) {
		HISI_FB_ERR("mipi phy pll retry fail, phy is not ready.\n");
		return 0;
	}
	redo_count ++;

	// check DPHY data and clock lane stopstate
	try_times = 0;
	tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	while ((tmp & cmp_stopstate_val) != cmp_stopstate_val) {
		udelay(10); //lint !e774  !e747  !e778
		if (++try_times > 100) {
			HISI_FB_INFO("fb%d, check1, check phy data clk lane stop state fail, PHY_STATUS=0x%x!\n",
				hisifd->index, tmp);

			set_reg(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x0, 32, 0);
			set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x0, 1, 0);
			udelay(5); //lint !e774  !e747  !e778
			set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x1, 1, 0);
			goto REDO;
		}

		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET); //lint !e732
	}

	// disable DPHY clock lane's Hight Speed Clock
	//set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);

	// request that data lane enter ULPS
	set_reg(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x4, 4, 0);

	//check DPHY data lane ulpsactivenot_status
	try_times = 0;
	tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET); //lint !e732
	while ((tmp & cmp_ulpsactivenot_val) != 0) {
		if (++try_times > 4) {
			HISI_FB_INFO("fb%d, check2, request data lane enter ulps fail, PHY_STATUS=0x%x!\n",
				hisifd->index, tmp);

			set_reg(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x0, 32, 0);
			set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x0, 1, 0);
			udelay(5); //lint !e774  !e747  !e778
			set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x1, 1, 0);
			goto REDO;
		}

		udelay(5); //lint !e774  !e747  !e778
		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET); //lint !e732
	}

	// enable DPHY PLL, force_pll = 1
	outp32(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0xF);

	////////////////////////////////////////////////////////////////////////////
	//
	// exit ulps
	//
	// request that data lane  exit ULPS
	outp32(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0xC);
	try_times= 0;
	tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET); //lint !e732
	while ((tmp & cmp_ulpsactivenot_val) != cmp_ulpsactivenot_val) {
		udelay(10); //lint !e774  !e747  !e778
		if (++try_times > 3) {
			HISI_FB_INFO("fb%d, check3, request data clock lane exit ulps fail, PHY_STATUS=0x%x!\n",
				hisifd->index, tmp);

			set_reg(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x0, 32, 0);
			set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x0, 1, 0);
			udelay(5); //lint !e774  !e747  !e778
			set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x1, 1, 0);
			goto REDO;
		}

		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET); //lint !e732
	}

	// mipi spec
	mdelay(1);

	// clear PHY_ULPS_CTRL
	outp32(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x0);

	try_times= 0;
	tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET); //lint !e732
	while ((tmp & cmp_stopstate_val) != cmp_stopstate_val) {
		udelay(10); //lint !e774  !e747  !e778
		if (++try_times > 3) {
			HISI_FB_INFO("fb%d, check4, wait data clock lane stop state fail, PHY_STATUS=0x%x!\n",
				hisifd->index, tmp);

			set_reg(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x0, 32, 0);
			set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x0, 1, 0);
			udelay(5); //lint !e774  !e747  !e778
			set_reg(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x1, 1, 0);
			goto REDO;
		}

		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET); //lint !e732
	}

	mipi_config_phy_test_code(mipi_dsi_base, 0x00010050, 0x00000092);
	mipi_config_phy_test_code(mipi_dsi_base, 0x00010050, 0x00000090);

	// enable DPHY clock lane's Hight Speed Clock
	//set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, 1, 0);

	hisifb_get_timestamp(&tv1);

	HISI_FB_INFO("fb%d, wait data clock lane stop state SUCC, redo_count=%d, cost time %u us!\n",
		hisifd->index, redo_count, hisifb_timestamp_diff(&tv0, &tv1));

	HISI_FB_DEBUG("fb%d, -!\n", hisifd->index);

	return 0;
}
/*lint +e732*/
/*lint -e776 -e715 -e712 -e737 -e776 -e838*/
static int mipi_dsi_on_sub1(struct hisi_fb_data_type *hisifd, char __iomem *mipi_dsi_base)
{
	if (NULL == hisifd || NULL == mipi_dsi_base) {
		HISI_FB_ERR("hisifd or mipi_dsi_base is null.\n");
		return 0;
	}

	/* mipi init */
	mipi_init(hisifd, mipi_dsi_base);

	/* dsi memory init */
	outp32(mipi_dsi_base + DSI_MEM_CTRL, 0x02600008);

	/* switch to cmd mode */
	set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 0);
	/* cmd mode: low power mode */
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x7f, 7, 8);
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0xf, 4, 16);
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 24);
	/* disable generate High Speed clock */
	/* delete? */
	set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);

	return 0;
}

static void pctrl_dphytx_stopcnt_config(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	uint64_t pctrl_dphytx_stopcnt = 0;
	uint32_t stopcnt_div = 1;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd or mipi_dsi_base is null.\n");
		return;
	}
	pinfo = &(hisifd->panel_info);

	stopcnt_div = is_dual_mipi_panel(hisifd) ? 2 : 1;
	// init: wait DPHY 4 data lane stopstate
	if (is_mipi_video_panel(hisifd)) {
		pctrl_dphytx_stopcnt = (uint64_t)(pinfo->ldi.h_back_porch +
			pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width + pinfo->xres / stopcnt_div + 5) *
			hisifd->dss_vote_cmd.dss_pclk_pctrl_rate / (pinfo->pxl_clk_rate / stopcnt_div);
	} else {
		pctrl_dphytx_stopcnt = (uint64_t)(pinfo->ldi.h_back_porch +
			pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width + 5) *
			hisifd->dss_vote_cmd.dss_pclk_pctrl_rate / (pinfo->pxl_clk_rate / stopcnt_div);
	}
	//FIXME:
	outp32(hisifd->pctrl_base + PERI_CTRL29, (uint32_t)pctrl_dphytx_stopcnt);
	if (is_dual_mipi_panel(hisifd)) {
		outp32(hisifd->pctrl_base + PERI_CTRL32, (uint32_t)pctrl_dphytx_stopcnt);
	}

	return;
}

static int mipi_dsi_on_sub2(struct hisi_fb_data_type *hisifd, char __iomem *mipi_dsi_base)
{
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd || NULL == mipi_dsi_base) {
		HISI_FB_ERR("hisifd or mipi_dsi_base is null.\n");
		return 0;
	}

	pinfo = &(hisifd->panel_info);

	if (is_mipi_video_panel(hisifd)) {
		/* switch to video mode */
		set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x0, 1, 0);
	}

	if (is_mipi_cmd_panel(hisifd)) {
		/* cmd mode: high speed mode */
		set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 7, 8);
		set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 4, 16);
		set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 1, 24);
	}

	/* enable EOTP TX */
	if (pinfo->mipi.phy_mode == DPHY_MODE) {
		/* Some vendors don't need eotp check.*/
		if(pinfo->mipi.eotp_disable_flag == 1){
		    set_reg(mipi_dsi_base + MIPIDSI_PCKHDL_CFG_OFFSET, 0x0, 1, 0);
		} else {
		    set_reg(mipi_dsi_base + MIPIDSI_PCKHDL_CFG_OFFSET, 0x1, 1, 0);
		}
	}

	/* enable generate High Speed clock, non continue */
	if (pinfo->mipi.non_continue_en) {
		set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x3, 2, 0);
	} else {
		set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, 2, 0);
	}

	if ((pinfo->mipi.dsi_version == DSI_1_2_VERSION)
		&& is_ifbc_vesa_panel(hisifd)) {
		set_reg(mipi_dsi_base + MIPIDSI_DSC_PARAMETER_OFFSET, 0x01, 32, 0);
	}

	pctrl_dphytx_stopcnt_config(hisifd);
	return 0;
}

static int mipi_dsi_off_sub(struct hisi_fb_data_type *hisifd, char __iomem *mipi_dsi_base)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	if(NULL == mipi_dsi_base){
		HISI_FB_ERR("mipi_dsi_base is NULL");
		return -EINVAL;
	}

	/* switch to cmd mode */
	set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 0);
	/* cmd mode: low power mode */
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x7f, 7, 8);
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0xf, 4, 16);
	set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 24);

	/* disable generate High Speed clock */
	set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);
	udelay(10);

	/* shutdown d_phy */
	set_reg(mipi_dsi_base +  MIPIDSI_PHY_RSTZ_OFFSET, 0x0, 3, 0);

	return 0;
}

void mipi_dsi_reset(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	set_reg(hisifd->mipi_dsi0_base + MIPIDSI_PWR_UP_OFFSET, 0x0, 1, 0);
	msleep(2);
	set_reg(hisifd->mipi_dsi0_base + MIPIDSI_PWR_UP_OFFSET, 0x1, 1, 0);
}

/*******************************************************************************
** MIPI DPHY GPIO for FPGA
*/
#define GPIO_MIPI_DPHY_PG_SEL_A_NAME	"pg_sel_a"
#define GPIO_MIPI_DPHY_PG_SEL_B_NAME	"pg_sel_b"
#define GPIO_MIPI_DPHY_TX_RX_A_NAME	"tx_rx_a"
#define GPIO_MIPI_DPHY_TX_RX_B_NAME	"tx_rx_b"

static uint32_t gpio_pg_sel_a = GPIO_PG_SEL_A;
static uint32_t gpio_tx_rx_a = GPIO_TX_RX_A;
static uint32_t gpio_pg_sel_b = GPIO_PG_SEL_B;
static uint32_t gpio_tx_rx_b = GPIO_TX_RX_B;

static struct gpio_desc mipi_dphy_gpio_request_cmds[] = {
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0},
};

static struct gpio_desc mipi_dphy_gpio_free_cmds[] = {
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0},

};

static struct gpio_desc mipi_dphy_gpio_normal_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 1},

	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 1},
};

static struct gpio_desc mipi_dphy_gpio_lowpower_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0},

	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0},
};

static int mipi_dsi_dphy_fastboot_fpga(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	if (g_fpga_flag == 1) {
		/* mpi dphy gpio request */
		gpio_cmds_tx(mipi_dphy_gpio_request_cmds,
			ARRAY_SIZE(mipi_dphy_gpio_request_cmds));
	}

	return 0;
}

static int mipi_dsi_dphy_on_fpga(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX)
		return 0;

	if (g_fpga_flag == 1) {
		/* mipi dphy gpio request */
		gpio_cmds_tx(mipi_dphy_gpio_request_cmds,
			ARRAY_SIZE(mipi_dphy_gpio_request_cmds));

		/* mipi dphy gpio normal */
		gpio_cmds_tx(mipi_dphy_gpio_normal_cmds, \
			ARRAY_SIZE(mipi_dphy_gpio_normal_cmds));
	}

	return 0;
}

static int mipi_dsi_dphy_off_fpga(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX)
		return 0;

	if (g_fpga_flag == 1) {
		/* mipi dphy gpio lowpower */
		gpio_cmds_tx(mipi_dphy_gpio_lowpower_cmds, \
			ARRAY_SIZE(mipi_dphy_gpio_lowpower_cmds));
		/* mipi dphy gpio free */
		gpio_cmds_tx(mipi_dphy_gpio_free_cmds, \
			ARRAY_SIZE(mipi_dphy_gpio_free_cmds));
	}

	return 0;
}


/*******************************************************************************
**
*/
int mipi_dsi_set_fastboot(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	mipi_dsi_dphy_fastboot_fpga(hisifd);

	mipi_dsi_clk_enable(hisifd);

	pinfo = &(hisifd->panel_info);//lint !e838
	if (pinfo && (pinfo->mipiclk_updt_support_new)) {
		memset(&(pinfo->dsi_phy_ctrl), 0, sizeof(struct mipi_dsi_phy_ctrl));
		if (pinfo->mipi.phy_mode == CPHY_MODE) {
			get_dsi_cphy_ctrl(hisifd, &(pinfo->dsi_phy_ctrl));
		} else {
			get_dsi_dphy_ctrl(hisifd, &(pinfo->dsi_phy_ctrl));
		}
	}

	ret = panel_next_set_fastboot(pdev);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

int mipi_dsi_on(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	mipi_dsi_dphy_on_fpga(hisifd);

	/* set LCD init step before LCD on*/
	hisifd->panel_info.lcd_init_step = LCD_INIT_POWER_ON;
	panel_next_on(pdev);

	//dis-reset
	//ip_reset_dis_dsi0, ip_reset_dis_dsi1
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (is_dual_mipi_panel(hisifd))
			outp32(hisifd->peri_crg_base + PERRSTDIS3, 0x30000000);
		else
			outp32(hisifd->peri_crg_base + PERRSTDIS3, 0x10000000);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		outp32(hisifd->peri_crg_base + PERRSTDIS3, 0x20000000);
	} else {
		HISI_FB_ERR("fb%d, not supported!\n", hisifd->index);
	}

	mipi_dsi_clk_enable(hisifd);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		mipi_dsi_on_sub1(hisifd, hisifd->mipi_dsi0_base);
		if (is_dual_mipi_panel(hisifd))
			mipi_dsi_on_sub1(hisifd, hisifd->mipi_dsi1_base);

	//Here need to exit ulps when panel off bypass ddic power down.
	if(panel_next_bypass_powerdown_ulps_support(pdev)){
		mipi_dsi_ulps_cfg(hisifd, 1);
	}

		mipi_dsi_pll_status_check_ec(hisifd, hisifd->mipi_dsi0_base);
		if (is_dual_mipi_panel(hisifd)) {
			mipi_dsi_pll_status_check_ec(hisifd, hisifd->mipi_dsi1_base);
		}
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		mipi_dsi_on_sub1(hisifd, hisifd->mipi_dsi1_base);
		mipi_dsi_pll_status_check_ec(hisifd, hisifd->mipi_dsi1_base);
	} else {
		HISI_FB_ERR("fb%d, not supported!\n", hisifd->index);
	}
	panel_next_on(pdev);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		mipi_dsi_on_sub2(hisifd, hisifd->mipi_dsi0_base);
		if (is_dual_mipi_panel(hisifd))
			mipi_dsi_on_sub2(hisifd, hisifd->mipi_dsi1_base);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		mipi_dsi_on_sub2(hisifd, hisifd->mipi_dsi1_base);
	} else {
		HISI_FB_ERR("fb%d, not supported!\n", hisifd->index);
	}

	/* mipi hs video/command mode */
	panel_next_on(pdev);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;
}

int mipi_dsi_off(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	/* set LCD uninit step before LCD off*/
	hisifd->panel_info.lcd_uninit_step = LCD_UNINIT_MIPI_HS_SEND_SEQUENCE;
	ret = panel_next_off(pdev);

	if (hisifd->panel_info.lcd_uninit_step_support) {
		/* TODO: add MIPI LP mode here if necessary */
		/* MIPI LP mode end */
		ret = panel_next_off(pdev);
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {

		//Here need to enter ulps when panel off bypass ddic power down.
		if(panel_next_bypass_powerdown_ulps_support(pdev)){
			mipi_dsi_ulps_cfg(hisifd, 0);
		}
		mipi_dsi_off_sub(hisifd, hisifd->mipi_dsi0_base);
		if (is_dual_mipi_panel(hisifd))
			mipi_dsi_off_sub(hisifd, hisifd->mipi_dsi1_base);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		mipi_dsi_off_sub(hisifd, hisifd->mipi_dsi1_base);
	} else {
		HISI_FB_ERR("fb%d, not supported!\n", hisifd->index);
	}

	mipi_dsi_clk_disable(hisifd);

	mipi_dsi_dphy_off_fpga(hisifd);

	// reset DSI
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (is_dual_mipi_panel(hisifd))
			outp32(hisifd->peri_crg_base + PERRSTEN3, 0x30000000);
		else
			outp32(hisifd->peri_crg_base + PERRSTEN3, 0x10000000);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		outp32(hisifd->peri_crg_base + PERRSTEN3, 0x20000000);
	} else {
		HISI_FB_ERR("fb%d, not supported!\n", hisifd->index);
	}

	if (hisifd->panel_info.lcd_uninit_step_support) {
		ret = panel_next_off(pdev);
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}


static int mipi_dsi_ulps_enter(struct hisi_fb_data_type *hisifd,
	char __iomem *mipi_dsi_base)
{
	uint32_t tmp;
	uint32_t cmp_ulpsactivenot_val = 0;
	uint32_t cmp_stopstate_val = 0;
	uint32_t try_times;

	if (NULL == hisifd || NULL == mipi_dsi_base) {
		HISI_FB_ERR("hisifd or mipi_dsi_base is NULL.\n");
		return 0;
	}

	HISI_FB_DEBUG("fb%d, +!\n", hisifd->index);

	if (hisifd->panel_info.mipi.lane_nums >= DSI_4_LANES) {
		cmp_ulpsactivenot_val = (BIT(5) | BIT(8) | BIT(10) | BIT(12));
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9) | BIT(11));
	} else if (hisifd->panel_info.mipi.lane_nums >= DSI_3_LANES) {
		cmp_ulpsactivenot_val = (BIT(5) | BIT(8) | BIT(10));
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9));
	} else if (hisifd->panel_info.mipi.lane_nums >= DSI_2_LANES) {
		cmp_ulpsactivenot_val = (BIT(5) | BIT(8));
		cmp_stopstate_val = (BIT(4) | BIT(7));
	} else {
		cmp_ulpsactivenot_val = (BIT(5));
		cmp_stopstate_val = (BIT(4));
	}

	tmp = inp32(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET) & BIT(1);
	if (tmp && (hisifd->panel_info.mipi.phy_mode == DPHY_MODE)) {
		cmp_stopstate_val |= (BIT(2));
	}

	// check DPHY data and clock lane stopstate
	try_times = 0;
	tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	while ((tmp & cmp_stopstate_val) != cmp_stopstate_val) {
		udelay(10);
		if (++try_times > 100) {
			HISI_FB_ERR("fb%d, check phy data and clk lane stop state failed! PHY_STATUS=0x%x.\n",
				hisifd->index, tmp);
			return 0;
		}

		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	}

	// disable DPHY clock lane's Hight Speed Clock
	set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);

	// request that data lane enter ULPS
	set_reg(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x4, 4, 0);

	//check DPHY data lane ulpsactivenot_status
	try_times = 0;
	tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	while ((tmp & cmp_ulpsactivenot_val) != 0) {
		udelay(10);
		if (++try_times > 100) {
			HISI_FB_ERR("fb%d, request phy data lane enter ulps failed! PHY_STATUS=0x%x.\n",
				hisifd->index, tmp);
			break;
		}

		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	}

	// request that clock lane enter ULPS
	if (hisifd->panel_info.mipi.phy_mode == DPHY_MODE) {
		set_reg(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x5, 4, 0);

		// check DPHY clock lane ulpsactivenot_status
		try_times = 0;
		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		while ((tmp & BIT(3)) != 0) {
			udelay(10);
			if (++try_times > 100) {
				HISI_FB_ERR("fb%d, request phy clk lane enter ulps failed! PHY_STATUS=0x%x.\n",
					hisifd->index, tmp);
				break;
			}

			tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		}
	}

	//bit13 lock sel enable (dual_mipi_panel bit29 set 1) ,colse clock gate
	set_reg(hisifd->pctrl_base + PERI_CTRL33, 0x1, 1, 13);
	if (is_dual_mipi_panel(hisifd)) {
		set_reg(hisifd->pctrl_base + PERI_CTRL30, 0x1, 1, 29);
	}

	if (is_dual_mipi_panel(hisifd)) {
		set_reg(hisifd->peri_crg_base + PERDIS3, 0xf, 4, 28);
	} else {
		set_reg(hisifd->peri_crg_base + PERDIS3, 0x3, 4, 28);
	}

	HISI_FB_DEBUG("fb%d, -!\n", hisifd->index);

	return 0;
}

static int mipi_dsi_ulps_exit(struct hisi_fb_data_type *hisifd,
	char __iomem *mipi_dsi_base)
{
	uint32_t tmp = 0;
	uint32_t cmp_ulpsactivenot_val = 0;
	uint32_t cmp_stopstate_val = 0;
	uint32_t try_times = 0;
	uint32_t need_pll_retry = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	if(NULL == mipi_dsi_base){
		HISI_FB_ERR("mipi_dsi_base is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +!\n", hisifd->index);

	if (hisifd->panel_info.mipi.lane_nums >= DSI_4_LANES) {
		cmp_ulpsactivenot_val = (BIT(3) | BIT(5) | BIT(8) | BIT(10) | BIT(12));
		cmp_stopstate_val = (BIT(2) | BIT(4) | BIT(7) | BIT(9) | BIT(11));
	} else if (hisifd->panel_info.mipi.lane_nums >= DSI_3_LANES) {
		cmp_ulpsactivenot_val = (BIT(3) | BIT(5) | BIT(8) | BIT(10));
		cmp_stopstate_val = (BIT(2) | BIT(4) | BIT(7) | BIT(9));
	} else if (hisifd->panel_info.mipi.lane_nums >= DSI_2_LANES) {
		cmp_ulpsactivenot_val = (BIT(3) | BIT(5) | BIT(8));
		cmp_stopstate_val = (BIT(2) | BIT(4) | BIT(7));
	} else {
		cmp_ulpsactivenot_val = (BIT(3) | BIT(5));
		cmp_stopstate_val = (BIT(2) | BIT(4));
	}
	if (hisifd->panel_info.mipi.phy_mode == CPHY_MODE) {
		cmp_ulpsactivenot_val &= (~ BIT(3));
		cmp_stopstate_val &= (~ BIT(2));
	}

	if (is_dual_mipi_panel(hisifd)) {
		set_reg(hisifd->peri_crg_base + PEREN3, 0xf, 4, 28);
	} else {
		set_reg(hisifd->peri_crg_base + PEREN3, 0x3, 4, 28);
	}

	udelay(100);//wait pll clk
	//bit13 lock sel enable (dual_mipi_panel bit29 set 1) ,colse clock gate
	set_reg(hisifd->pctrl_base + PERI_CTRL33, 0x0, 1, 13);
	if (is_dual_mipi_panel(hisifd)) {
		set_reg(hisifd->pctrl_base + PERI_CTRL30, 0x0, 1, 29);
	}

	// enable DPHY PLL, force_pll = 1
	//outp32(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0xF); ???

	// request that data lane and clock lane exit ULPS
	outp32(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0xF);
	try_times= 0;
	tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	while ((tmp & cmp_ulpsactivenot_val) != cmp_ulpsactivenot_val) {
		udelay(10);
		if (++try_times > 100) {
			HISI_FB_ERR("fb%d, request data clock lane exit ulps fail!PHY_STATUS=0x%x.\n",
				hisifd->index, tmp);
			need_pll_retry = BIT(0);
			break;
		}

		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	}

	// mipi spec
	mdelay(1);

	// clear PHY_ULPS_CTRL
	outp32(mipi_dsi_base + MIPIDSI_PHY_ULPS_CTRL_OFFSET, 0x0);

	//check DPHY data lane cmp_stopstate_val
	try_times = 0;
	tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	while ((tmp & cmp_stopstate_val) != cmp_stopstate_val) {
		udelay(10);
		if (++try_times > 100) {
			HISI_FB_ERR("fb%d, check phy data clk lane stop state failed! PHY_STATUS=0x%x.\n",
				hisifd->index, tmp);
			need_pll_retry |= BIT(1);
			break;
		}

		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	}

	if (need_pll_retry > 0) {
		HISI_FB_ERR("need_pll_retry = 0x%x .\n", need_pll_retry);
		mipi_dsi_pll_status_check_ec(hisifd, mipi_dsi_base);
	}

	// enable DPHY clock lane's Hight Speed Clock
	set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, 1, 0);

	//reset dsi
	outp32(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x0);
	udelay(5);
	// Power_up dsi
	outp32(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x1);

	HISI_FB_DEBUG("fb%d, -!\n", hisifd->index);
	return 0;
}
//lint -restore

int mipi_dsi_ulps_cfg(struct hisi_fb_data_type *hisifd, int enable)
{
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (enable) {
		mipi_dsi_ulps_exit(hisifd, hisifd->mipi_dsi0_base);
		if (is_dual_mipi_panel(hisifd))
			mipi_dsi_ulps_exit(hisifd, hisifd->mipi_dsi1_base);
	} else {
		mipi_dsi_ulps_enter(hisifd, hisifd->mipi_dsi0_base);
		if (is_dual_mipi_panel(hisifd))
			mipi_dsi_ulps_enter(hisifd, hisifd->mipi_dsi1_base);
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static void mipi_dsi_set_cdphy_bit_clk_upt_cmd(struct hisi_fb_data_type *hisifd,
	char __iomem *mipi_dsi_base, struct mipi_dsi_phy_ctrl* phy_ctrl)
{
	struct hisi_panel_info *pinfo;
	unsigned long dw_jiffies;
	bool is_ready ;
	uint32_t tmp = 0;

	if (NULL == hisifd || NULL == phy_ctrl) {
		HISI_FB_ERR("hisifd or phy_ctrl is null.\n");
		return;
	}
	HISI_FB_DEBUG("fb%d +.\n", hisifd->index);
	pinfo = &(hisifd->panel_info);

	//config parameter M, N, Q of PLL
	if (pinfo->mipi.phy_mode == CPHY_MODE) {
		if (phy_ctrl->rg_pre_div > pinfo->dsi_phy_ctrl.rg_pre_div) {
			//PLL configuration III N
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010049, phy_ctrl->rg_pre_div);
			//PLL configuration IV M
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A, phy_ctrl->rg_div);
		} else {
			//PLL configuration IV M
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A, phy_ctrl->rg_div);
			//PLL configuration III N
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010049, phy_ctrl->rg_pre_div);
		}
		//PLL configuration II Q
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010048, phy_ctrl->rg_0p8v + (phy_ctrl->rg_2p5g << 1) +
			(phy_ctrl->rg_320m << 2) + (phy_ctrl->rg_band_sel << 3) + (phy_ctrl->rg_cphy_div << 4));

	} else {
		if (phy_ctrl->rg_pre_div > pinfo->dsi_phy_ctrl.rg_pre_div) {
			//PLL configuration III N
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010049, phy_ctrl->rg_pre_div);
			//PLL configuration IV M
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A, phy_ctrl->rg_div);
		} else {
			//PLL configuration IV M
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A, phy_ctrl->rg_div);
			//PLL configuration III N
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010049, phy_ctrl->rg_pre_div);
		}
		//PLL configuration II Q
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010048, phy_ctrl->rg_0p8v + (phy_ctrl->rg_2p5g << 1) +
			(phy_ctrl->rg_320m << 2) + (phy_ctrl->rg_band_sel << 3));
	}
	//PLL update control
	mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);

	// clk lane HS2LP/LP2HS
	outp32(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET,
		(phy_ctrl->clk_lane_lp2hs_time + (phy_ctrl->clk_lane_hs2lp_time << 16)));
	// data lane HS2LP/ LP2HS
	outp32(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET,
		(phy_ctrl->data_lane_lp2hs_time + (phy_ctrl->data_lane_hs2lp_time << 16)));

	// escape clock dividor
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET,
		(phy_ctrl->clk_division + (phy_ctrl->clk_division << 8)), 16, 0);
	/*lint -e550 -e732*/
	is_ready = false;
	dw_jiffies = jiffies + HZ / 2;//500ms
	do {
		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((tmp & 0x00000001) == 0x00000001) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));
	if (!is_ready) {
		HISI_FB_INFO("fb%d, phylock is not ready!MIPIDSI_PHY_STATUS_OFFSET=0x%x.\n",
			hisifd->index, tmp);
	}
	/*lint +e550 +e732*/

	HISI_FB_DEBUG("fb%d -.\n", hisifd->index);
}

static void mipi_dsi_set_cdphy_bit_clk_upt_video(struct hisi_fb_data_type *hisifd,
	char __iomem *mipi_dsi_base, struct mipi_dsi_phy_ctrl* phy_ctrl)
{

	struct hisi_panel_info *pinfo;
	uint32_t hline_time = 0;
	uint32_t hsa_time = 0;
	uint32_t hbp_time = 0;
	uint32_t pixel_clk;
	dss_rect_t rect;
	unsigned long dw_jiffies;
	uint32_t tmp;
	bool is_ready;

	if (NULL == hisifd || NULL == phy_ctrl) {
		HISI_FB_ERR("hisifd or phy_ctrl is null.\n");
		return;
	}
	HISI_FB_DEBUG("fb%d +.\n", hisifd->index);

	pinfo = &(hisifd->panel_info);
	pinfo->dsi_phy_ctrl = *phy_ctrl;

	rect.x = 0;
	rect.y = 0;
	rect.w = pinfo->xres;//lint !e713
	rect.h = pinfo->yres;//lint !e713

	mipi_ifbc_get_rect(hisifd, &rect);

	if (pinfo->mipi.phy_mode == CPHY_MODE) {
		//PLL configuration I
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010046, phy_ctrl->rg_cp + (phy_ctrl->rg_lpf_r << 4));

		//PLL configuration II
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010048, phy_ctrl->rg_0p8v + (phy_ctrl->rg_2p5g << 1) +
			(phy_ctrl->rg_320m << 2) + (phy_ctrl->rg_band_sel << 3) + (phy_ctrl->rg_cphy_div << 4));

		//PLL configuration III
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010049, phy_ctrl->rg_pre_div);

		//PLL configuration IV
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A, phy_ctrl->rg_div);

		//PLL update control
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);

		//set cphy spec parameter
		mipi_config_cphy_spec1v0_parameter(mipi_dsi_base, pinfo);
	} else {
		//PLL configuration I
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010046, phy_ctrl->rg_cp + (phy_ctrl->rg_lpf_r << 4));

		//PLL configuration II
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010048, phy_ctrl->rg_0p8v + (phy_ctrl->rg_2p5g << 1) +
			(phy_ctrl->rg_320m << 2) + (phy_ctrl->rg_band_sel << 3));

		//PLL configuration III
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010049, phy_ctrl->rg_pre_div);

		//PLL configuration IV
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A, phy_ctrl->rg_div);

		//PLL update control
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);

		//set dphy spec parameter
		mipi_config_dphy_spec1v2_parameter(mipi_dsi_base, pinfo);
	}
	/*lint -e550 -e732*/
	is_ready = false;
	dw_jiffies = jiffies + HZ / 2;
	do {
		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((tmp & 0x00000001) == 0x00000001) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	if (!is_ready) {
		HISI_FB_ERR("fb%d, phylock is not ready!MIPIDSI_PHY_STATUS_OFFSET=0x%x.\n",
			hisifd->index, tmp);
	}
	/*lint +e550 +e732*/
	// phy_stop_wait_time
	set_reg(mipi_dsi_base + MIPIDSI_PHY_IF_CFG_OFFSET, phy_ctrl->phy_stop_wait_time, 8, 8);

	/* 4. Define the DPI Horizontal timing configuration:	*/

	pixel_clk = mipi_pixel_clk(hisifd);
	/*lint -e737 -e776 -e712*/
	if (pinfo->mipi.phy_mode == DPHY_MODE) {
		hsa_time = pinfo->ldi.h_pulse_width * phy_ctrl->lane_byte_clk / pixel_clk;
		hbp_time = pinfo->ldi.h_back_porch * phy_ctrl->lane_byte_clk / pixel_clk;
		hline_time = (pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch +
			rect.w + pinfo->ldi.h_front_porch) * phy_ctrl->lane_byte_clk / pixel_clk;
	} else {
		hsa_time = pinfo->ldi.h_pulse_width * phy_ctrl->lane_word_clk / pixel_clk;
		hbp_time = pinfo->ldi.h_back_porch * phy_ctrl->lane_word_clk / pixel_clk;
		hline_time = (pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch +
			rect.w + pinfo->ldi.h_front_porch) * phy_ctrl->lane_word_clk / pixel_clk;
	}
	/*lint +e737 +e776 +e712*/
	set_reg(mipi_dsi_base + MIPIDSI_VID_HSA_TIME_OFFSET, hsa_time, 12, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_HBP_TIME_OFFSET, hbp_time, 12, 0);
	set_reg(mipi_dsi_base + MIPIDSI_VID_HLINE_TIME_OFFSET, hline_time, 15, 0);

	// Configure core's phy parameters
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET, phy_ctrl->clk_lane_lp2hs_time, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET, phy_ctrl->clk_lane_hs2lp_time, 10, 16);

	outp32(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET,
		(phy_ctrl->data_lane_lp2hs_time + (phy_ctrl->data_lane_hs2lp_time << 16)));

	HISI_FB_DEBUG("fb%d -.\n", hisifd->index);
}

static bool check_pctrl_trstop_flag(struct hisi_fb_data_type *hisifd, int time_count)
{
	bool is_ready = false;
	int count;
	uint32_t tmp = 0;

	if (is_dual_mipi_panel(hisifd)) {
		for(count = 0; count < time_count; count++) {
			tmp = inp32(hisifd->pctrl_base + PERI_STAT0);
			if ((tmp & 0xC0000000) == 0xC0000000) {
				is_ready = true;
				break;
			}
			udelay(2);
		}
	} else {
		for(count = 0; count < time_count; count++) {
			tmp = inp32(hisifd->pctrl_base + PERI_STAT0);
			if ((tmp & 0x80000000) == 0x80000000) {
				is_ready = true;
				break;
			}
			udelay(2);
		}
	}

	return is_ready;
}

int mipi_dsi_bit_clk_upt_isr_handler(struct hisi_fb_data_type *hisifd)
{
	struct mipi_dsi_phy_ctrl phy_ctrl = {0};
	struct hisi_panel_info *pinfo;
	uint32_t dsi_bit_clk_upt;
	uint32_t stopstate_msk = 0;
	bool is_ready = false;
	uint8_t esd_enable;

	struct timeval tv0;
	struct timeval tv1;
	uint32_t timediff = 0;
	uint32_t vfp_time = 0;
	uint64_t lane_byte_clk = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null!\n");
		return 0;
	}
	pinfo = &(hisifd->panel_info);
	dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk_upt;

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		HISI_FB_ERR("fb%d, not support!\n", hisifd->index);
		return 0;
	}

	if (dsi_bit_clk_upt == pinfo->mipi.dsi_bit_clk) {
		return 0;
	}

	HISI_FB_DEBUG("fb%d +.\n", hisifd->index);

	if (pinfo->mipiclk_updt_support_new) {
		hisifb_get_timestamp(&tv0);
		lane_byte_clk = hisifd->panel_info.dsi_phy_ctrl.lane_byte_clk;
		if (hisifd->panel_info.mipi.phy_mode == CPHY_MODE) {
			lane_byte_clk = hisifd->panel_info.dsi_phy_ctrl.lane_word_clk;
		}
		vfp_time = inp32(hisifd->mipi_dsi0_base + MIPIDSI_VID_HLINE_TIME_OFFSET) & 0x7fff;
		if (lane_byte_clk != 0) {
			vfp_time = vfp_time * (hisifd->panel_info.ldi.v_front_porch + 10) / ((uint32_t)(lane_byte_clk/1000000));
		} else {
			HISI_FB_ERR("vfp_time == 0.\n");
			vfp_time = 80;//80us
		}
	}

	esd_enable = pinfo->esd_enable;
	if (is_mipi_video_panel(hisifd)) {
		pinfo->esd_enable = 0;
		disable_ldi(hisifd);
	}

	/* get new phy_ctrl value according to dsi_bit_clk_next */
	if (hisifd->panel_info.mipi.lane_nums == DSI_4_LANES) {
		stopstate_msk = BIT(0);
	} else if (hisifd->panel_info.mipi.lane_nums == DSI_3_LANES) {
		stopstate_msk = BIT(0) | BIT(4);
	} else if (hisifd->panel_info.mipi.lane_nums == DSI_2_LANES) {
		stopstate_msk = BIT(0) | BIT(3) | BIT(4);
	} else {
		stopstate_msk = BIT(0) | BIT(2) | BIT(3) | BIT(4);
	}
	if (pinfo->mipi.phy_mode == CPHY_MODE) {
		get_dsi_cphy_ctrl(hisifd, &phy_ctrl);
	} else {
		get_dsi_dphy_ctrl(hisifd, &phy_ctrl);
	}
	//1. wait stopstate cnt:dphy_stopstate_cnt_en=1 (pctrl_dphy_ctrl[0])
	//PERI_CTRL33[0:15]-PHY0, PERI_CTRL30[16:31]-PHY1.
	set_reg(hisifd->pctrl_base + PERI_CTRL33, 1, 1, 0);
	set_reg(hisifd->pctrl_base + PERI_CTRL33, stopstate_msk, 5, 3);

	if (pinfo->mipiclk_updt_support_new) {
		while ((!is_ready) && (timediff < vfp_time)) {
			is_ready = check_pctrl_trstop_flag(hisifd, 10);
			hisifb_get_timestamp(&tv1);
			timediff = hisifb_timestamp_diff(&tv0, &tv1);
		}
		HISI_FB_INFO("timediff=%d us, vfp_time=%d us.\n", timediff, vfp_time);
	} else {
		is_ready = check_pctrl_trstop_flag(hisifd, 40);
	}

	set_reg(hisifd->pctrl_base + PERI_CTRL33, 0, 1, 0);
	if (!is_ready) {
		if (is_mipi_video_panel(hisifd)) {
			pinfo->esd_enable = esd_enable;
			enable_ldi(hisifd);
		}
		HISI_FB_DEBUG("PERI_STAT0 is not ready.\n");
		return 0;
	}

	if (is_mipi_cmd_panel(hisifd)) {
		mipi_dsi_set_cdphy_bit_clk_upt_cmd(hisifd, hisifd->mipi_dsi0_base, &phy_ctrl);
		if (is_dual_mipi_panel(hisifd)) {
			mipi_dsi_set_cdphy_bit_clk_upt_cmd(hisifd, hisifd->mipi_dsi1_base, &phy_ctrl);
		}
	} else {
		if (pinfo->mipiclk_updt_support_new) {
			set_reg(hisifd->mipi_dsi0_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);
			mipi_dsi_set_cdphy_bit_clk_upt_video(hisifd, hisifd->mipi_dsi0_base, &phy_ctrl);
			set_reg(hisifd->mipi_dsi0_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, 1, 0);
			if (is_dual_mipi_panel(hisifd)) {
				set_reg(hisifd->mipi_dsi1_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);
				mipi_dsi_set_cdphy_bit_clk_upt_video(hisifd, hisifd->mipi_dsi1_base, &phy_ctrl);
				set_reg(hisifd->mipi_dsi1_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, 1, 0);
			}
		} else {
			mipi_dsi_set_cdphy_bit_clk_upt_video(hisifd, hisifd->mipi_dsi0_base, &phy_ctrl);
			if (is_dual_mipi_panel(hisifd)) {
				mipi_dsi_set_cdphy_bit_clk_upt_video(hisifd, hisifd->mipi_dsi1_base, &phy_ctrl);
			}
		}

		pinfo->esd_enable = esd_enable;
		enable_ldi(hisifd);
	}

	HISI_FB_INFO("Mipi clk successfully changed from (%d)M switch to (%d)M!\n", pinfo->mipi.dsi_bit_clk, dsi_bit_clk_upt);

	pinfo->dsi_phy_ctrl = phy_ctrl;
	pinfo->mipi.dsi_bit_clk = dsi_bit_clk_upt;

	HISI_FB_DEBUG("fb%d -.\n", hisifd->index);

	return 0;
}

