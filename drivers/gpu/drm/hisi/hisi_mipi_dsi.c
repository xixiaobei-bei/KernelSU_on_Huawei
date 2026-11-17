/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
/*lint -e414 -e529 -e548 -e559 -e578 -e644 -e647 -e712 -e715 -e737 -e744 -e776 -e838 -e850*/
#include <securec.h>

#include "hisi_drm_dsi.h"
#include "hisi_dsi_reg.h"
#include "hisi_dpe.h"
#include "hisi_drm_dpe_utils.h"
#include "hisi_drm_drv.h"
#include "hisi_panel.h"
#include "hisi_mipi_dsi.h"

#include "hisi_drm_debug.h"

#ifdef CONFIG_HISI_DSS_V600
#define VCO_CLK_MIN_VALUE 2750
#else
#define VCO_CLK_MIN_VALUE 2000
#endif

int mipi_ifbc_get_rect(struct hisi_dsi *dsi,
	struct dss_rect *rect)
{
	uint32_t ifbc_type;
	uint32_t mipi_idx;
	uint32_t xres_div;
	uint32_t yres_div;
	struct hisi_panel_info *panel_info = NULL;

	if (!dsi) {
		HISI_DRM_ERR("dsi is NULL");
		return -EINVAL;
	}

	panel_info = dsi->panel_info;
	if (panel_info == NULL) {
		HISI_DRM_ERR("panel_info is NULL");
		return -EINVAL;
	}
	if (rect == NULL) {
		HISI_DRM_ERR("rect is NULL");
		return -EINVAL;
	}

	ifbc_type = panel_info->ifbc_type;
	if ((ifbc_type < IFBC_TYPE_NONE) || (ifbc_type >= IFBC_TYPE_MAX)) {
		HISI_DRM_ERR("ifbc_type is invalid");
		return -EINVAL;
	}

	mipi_idx = is_dual_mipi(dsi) ? 1 : 0;

	xres_div = g_mipi_ifbc_division[mipi_idx][ifbc_type].xres_div;
	yres_div = g_mipi_ifbc_division[mipi_idx][ifbc_type].yres_div;

	if ((rect->w % xres_div) > 0)
		HISI_DRM_ERR("xres %d is not division_h %d pixel aligned!\n",
			rect->w, xres_div);

	if ((rect->h % yres_div) > 0)
		HISI_DRM_ERR("yres %d is not division_v %d pixel aligned!\n",
			rect->h, yres_div);

	if ((mipi_idx == 0) && (ifbc_type == IFBC_TYPE_RSP3X)) {
		rect->w *= 2;
		rect->h /= 2;
	}

	rect->w /= xres_div;
	rect->h /= yres_div;

	return 0;
}

static uint32_t get_data_t_hs_prepare(struct mipi_panel_info *mipi,
					uint32_t accuracy, uint32_t ui)
{
	uint32_t data_t_hs_prepare;

	if (mipi->data_t_hs_prepare_adjust == 0)
		mipi->data_t_hs_prepare_adjust = 35;

	data_t_hs_prepare = ((400  * accuracy + 4 * ui + mipi->data_t_hs_prepare_adjust * ui) <=
		(850 * accuracy + 6 * ui - 8 * ui)) ?
		(400  * accuracy + 4 * ui + mipi->data_t_hs_prepare_adjust * ui) : (850 * accuracy + 6 * ui - 8 * ui);

	return data_t_hs_prepare;
}

static uint32_t get_data_pre_delay(uint32_t lp11_flag,
				struct mipi_dsi_phy_ctrl *phy_ctrl,
				uint32_t clk_pre)
{
	uint32_t data_pre_delay = 0;

	if (lp11_flag != MIPI_SHORT_LP11)
		data_pre_delay = phy_ctrl->clk_pre_delay + 2 + phy_ctrl->clk_t_lpx +
			phy_ctrl->clk_t_hs_prepare + phy_ctrl->clk_t_hs_zero + 8 + clk_pre;
	return data_pre_delay;
}

static uint32_t get_data_pre_delay_reality(uint32_t lp11_flag,
					struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	uint32_t data_pre_delay_reality = 0;

	if (lp11_flag != MIPI_SHORT_LP11)
		data_pre_delay_reality = phy_ctrl->data_pre_delay + 5;

	return data_pre_delay_reality;
}

static uint32_t get_clk_post_delay_reality(uint32_t lp11_flag,
					struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	uint32_t clk_post_delay_reality = 0;

	if (lp11_flag != MIPI_SHORT_LP11)
		clk_post_delay_reality = phy_ctrl->clk_post_delay + 4;

	return clk_post_delay_reality;
}

struct dsi_dphy_ctrl_info {
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
};

static void pll_param_config(struct mipi_dsi_phy_ctrl *phy_ctrl, uint64_t *lane_clock)
{
	/*chip spec :
	 *If the output data rate is below 320 Mbps,
	 *RG_BNAD_SEL should be set to 1.
	 *At this mode a post divider of 1/4 will be applied to VCO.
	 */
	uint32_t post_div[6] = {1, 2, 4, 8, 16, 32};
	uint64_t vco_clk;
	int post_div_idx = 0;

	phy_ctrl->rg_pll_prediv = 0;
	vco_clk = *lane_clock * post_div[post_div_idx];

	while ((vco_clk <= VCO_CLK_MIN_VALUE) && (post_div_idx < 5)) {
		post_div_idx++;
		vco_clk = *lane_clock * post_div[post_div_idx];
	}

	vco_clk = vco_clk * 1000000;
	phy_ctrl->rg_pll_posdiv = post_div_idx;
	phy_ctrl->rg_pll_fbkdiv = vco_clk / DEFAULT_MIPI_CLK_RATE;
	*lane_clock = phy_ctrl->rg_pll_fbkdiv * DEFAULT_MIPI_CLK_RATE /
		post_div[phy_ctrl->rg_pll_posdiv];
	HISI_DRM_DEBUG("rg_pll_prediv=%d, rg_pll_posdiv=%d, rg_pll_fbkdiv=%d\n",
		phy_ctrl->rg_pll_prediv, phy_ctrl->rg_pll_posdiv,
		phy_ctrl->rg_pll_fbkdiv);

	phy_ctrl->rg_0p8v = 0;
	phy_ctrl->rg_2p5g = 1;
	phy_ctrl->rg_320m = 0;
	phy_ctrl->rg_lpf_r = 0;

	phy_ctrl->rg_vrefsel_vcm = 0x5d;
}

static void lane_param_config(struct dsi_dphy_ctrl_info *dphy_ctrl_info,
	uint32_t *unit_tx_byte_clk_hs, struct mipi_panel_info *mipi, uint64_t lane_clock)
{
	uint32_t accuracy;
	uint32_t ui;

	accuracy = 10;
	if (!lane_clock) {
		HISI_DRM_DEBUG("invalid lane clock!");
		return;
	}
	ui =  (uint32_t)(10 * 1000000000UL * accuracy / lane_clock);
	/* unit of measurement */
	*unit_tx_byte_clk_hs = 8 * ui;

	/* D-PHY Specification : 60ns + 52*UI <= clk_post */
	dphy_ctrl_info->clk_post = 600 * accuracy + 52 * ui + (*unit_tx_byte_clk_hs) +
			mipi->clk_post_adjust * ui;

	/* D-PHY Specification : clk_pre >= 8*UI */
	dphy_ctrl_info->clk_pre = 8 * ui + (*unit_tx_byte_clk_hs) + mipi->clk_pre_adjust * ui;

	/* D-PHY Specification : clk_t_hs_exit >= 100ns */
	dphy_ctrl_info->clk_t_hs_exit = 1000 * accuracy + 100 * accuracy +
			mipi->clk_t_hs_exit_adjust * ui;

	/* clocked by TXBYTECLKHS */
	dphy_ctrl_info->clk_pre_delay = 0 + mipi->clk_pre_delay_adjust * ui;

	/* D-PHY Specification : clk_t_hs_trial >= 60ns
	 * clocked by TXBYTECLKHS
	 */
	dphy_ctrl_info->clk_t_hs_trial = 600 * accuracy + 3 * (*unit_tx_byte_clk_hs) +
			mipi->clk_t_hs_trial_adjust * ui;

	/* D-PHY Specification : 38ns <= clk_t_hs_prepare <= 95ns
	 * clocked by TXBYTECLKHS
	 */
	dphy_ctrl_info->clk_t_hs_prepare = 660 * accuracy;

	/* clocked by TXBYTECLKHS */
	dphy_ctrl_info->data_post_delay = 0 + mipi->data_post_delay_adjust * ui;

	/* D-PHY Specification : data_t_hs_trial >= max( n*8*UI, 60ns + n*4*UI ), n = 1
	 * clocked by TXBYTECLKHS
	 */
	dphy_ctrl_info->data_t_hs_trial = ((600 * accuracy + 4 * ui) >= (8 * ui) ?
		(600 * accuracy + 4 * ui) : (8 * ui)) + 8 * ui +
		3 * (*unit_tx_byte_clk_hs) + mipi->data_t_hs_trial_adjust * ui;

	/* D-PHY Specification : 40ns + 4*UI <= data_t_hs_prepare <= 85ns + 6*UI
	 * clocked by TXBYTECLKHS
	 */
	dphy_ctrl_info->data_t_hs_prepare = get_data_t_hs_prepare(mipi, accuracy, ui);
	/* D-PHY chip spec : clk_t_lpx + clk_t_hs_prepare > 200ns
	 * D-PHY Specification : clk_t_lpx >= 50ns
	 * clocked by TXBYTECLKHS
	 */
	dphy_ctrl_info->clk_t_lpx = (uint32_t)(2000 * accuracy + 10 * accuracy +
			mipi->clk_t_lpx_adjust * ui - dphy_ctrl_info->clk_t_hs_prepare);

	/* D-PHY Specification : clk_t_hs_zero + clk_t_hs_prepare >= 300 ns
	 * clocked by TXBYTECLKHS
	 */
	dphy_ctrl_info->clk_t_hs_zero = (uint32_t)(3000 * accuracy + 3 * (*unit_tx_byte_clk_hs) +
			mipi->clk_t_hs_zero_adjust * ui - dphy_ctrl_info->clk_t_hs_prepare);

	/* D-PHY chip spec : data_t_lpx + data_t_hs_prepare > 200ns
	 * D-PHY Specification : data_t_lpx >= 50ns
	 * clocked by TXBYTECLKHS
	 */
	dphy_ctrl_info->data_t_lpx = (uint32_t)(2000 * accuracy + 10 * accuracy +
			mipi->data_t_lpx_adjust * ui - dphy_ctrl_info->data_t_hs_prepare);

	/* D-PHY Specification : data_t_hs_zero + data_t_hs_prepare >= 145ns + 10*UI
	 * clocked by TXBYTECLKHS
	 */
	dphy_ctrl_info->data_t_hs_zero = (uint32_t)(1450 * accuracy + 10 * ui +
		3 * (*unit_tx_byte_clk_hs) + mipi->data_t_hs_zero_adjust * ui - dphy_ctrl_info->data_t_hs_prepare);

}

static void dphy_ctrl_config(struct mipi_dsi_phy_ctrl *phy_ctrl,
	struct dsi_dphy_ctrl_info *dphy_ctrl_info, uint64_t lane_clock,
	uint32_t unit_tx_byte_clk_hs, struct mipi_panel_info *mipi)
{
	if (!unit_tx_byte_clk_hs) {
		HISI_DRM_DEBUG("invalid clock!");
		return;
	}
	phy_ctrl->clk_pre_delay = ROUND1(dphy_ctrl_info->clk_pre_delay, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_prepare = ROUND1(dphy_ctrl_info->clk_t_hs_prepare, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_lpx = ROUND1(dphy_ctrl_info->clk_t_lpx, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_zero = ROUND1(dphy_ctrl_info->clk_t_hs_zero, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_trial = ROUND1(dphy_ctrl_info->clk_t_hs_trial, unit_tx_byte_clk_hs);

	phy_ctrl->data_post_delay = ROUND1(dphy_ctrl_info->data_post_delay, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_prepare = ROUND1(dphy_ctrl_info->data_t_hs_prepare, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_lpx = ROUND1(dphy_ctrl_info->data_t_lpx, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_zero = ROUND1(dphy_ctrl_info->data_t_hs_zero, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_trial = ROUND1(dphy_ctrl_info->data_t_hs_trial, unit_tx_byte_clk_hs);

	phy_ctrl->clk_post_delay = phy_ctrl->data_t_hs_trial +
					ROUND1(dphy_ctrl_info->clk_post, unit_tx_byte_clk_hs);
	phy_ctrl->data_pre_delay = get_data_pre_delay(mipi->lp11_flag,
				phy_ctrl, ROUND1(dphy_ctrl_info->clk_pre, unit_tx_byte_clk_hs));

	phy_ctrl->clk_lane_lp2hs_time = phy_ctrl->clk_pre_delay +
					phy_ctrl->clk_t_lpx +
					phy_ctrl->clk_t_hs_prepare +
					phy_ctrl->clk_t_hs_zero + 5 + 7;
	phy_ctrl->clk_lane_hs2lp_time = phy_ctrl->clk_t_hs_trial +
					phy_ctrl->clk_post_delay + 8 + 4;
	phy_ctrl->data_lane_lp2hs_time = get_data_pre_delay_reality(mipi->lp11_flag, phy_ctrl) +
					phy_ctrl->data_t_lpx +
					phy_ctrl->data_t_hs_prepare +
					phy_ctrl->data_t_hs_zero + 7;
	phy_ctrl->data_lane_hs2lp_time = phy_ctrl->data_t_hs_trial + 8 + 5;

	phy_ctrl->phy_stop_wait_time = get_clk_post_delay_reality(mipi->lp11_flag, phy_ctrl) +
					phy_ctrl->clk_t_hs_trial +
					ROUND1(dphy_ctrl_info->clk_t_hs_exit, unit_tx_byte_clk_hs) -
					(phy_ctrl->data_post_delay + 4 +
					phy_ctrl->data_t_hs_trial) + 3;

	phy_ctrl->lane_byte_clk = lane_clock / 8;
	phy_ctrl->clk_division = (((phy_ctrl->lane_byte_clk / 2) % mipi->max_tx_esc_clk) > 0) ?
		(uint32_t)(phy_ctrl->lane_byte_clk / 2 / mipi->max_tx_esc_clk + 1) :
		(uint32_t)(phy_ctrl->lane_byte_clk / 2 / mipi->max_tx_esc_clk);

}

struct dphy_param {
	uint32_t m_pll;
	uint32_t n_pll;
	uint64_t lane_clock;
	uint32_t unit_tx_byte_clk_hs;
	uint32_t dsi_bit_clk;
};

void get_dsi_dphy_ctrl(struct hisi_dsi *dsi,
	struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	struct mipi_panel_info *mipi = NULL;
	struct drm_display_mode *mode = NULL;
	struct dsi_dphy_ctrl_info dphy_ctrl_info;
	struct dphy_param dphy_param;

	dphy_param.m_pll = 0;
	dphy_param.n_pll = 0;
	WARN_ON(!phy_ctrl);
	WARN_ON(!dsi);

	mode = &dsi->cur_mode;
	mipi = &dsi->panel_info->mipi;

	dphy_param.dsi_bit_clk = mipi->dsi_bit_clk_upt;
	dphy_param.lane_clock = (uint64_t)(2 * dphy_param.dsi_bit_clk);

	HISI_DRM_DEBUG("Expected: lane_clock = %llu M\n", dphy_param.lane_clock);

	/************************  PLL parameters config  *********************/
	pll_param_config(phy_ctrl, &(dphy_param.lane_clock));

	/********************  clock/data lane parameters config  ******************/
	lane_param_config(&dphy_ctrl_info, &(dphy_param.unit_tx_byte_clk_hs), mipi, dphy_param.lane_clock);

	dphy_ctrl_config(phy_ctrl, &dphy_ctrl_info, dphy_param.lane_clock, dphy_param.unit_tx_byte_clk_hs, mipi);

	HISI_DRM_DEBUG("DPHY clock_lane and data_lane config :\n"
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
		"phy_stop_wait_time=%u\n"
		"lane_byte_clk=%llu\n"
		"clk_division=%u\n",
		dphy_param.lane_clock, dphy_param.n_pll, dphy_param.m_pll,
		phy_ctrl->rg_cp,
		phy_ctrl->rg_band_sel,
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
		phy_ctrl->phy_stop_wait_time,
		phy_ctrl->lane_byte_clk,
		phy_ctrl->clk_division);
}

static void cphy_pll_param_config(struct mipi_dsi_phy_ctrl *phy_ctrl, uint64_t *lane_clock)
{
	uint64_t vco_clk;
	uint32_t post_div[6] = {1, 2, 4, 8, 16, 32};
	int post_div_idx = 0;

	phy_ctrl->rg_pll_prediv = 0;
	vco_clk = (*lane_clock) * post_div[post_div_idx];

	while ((vco_clk <= VCO_CLK_MIN_VALUE) && (post_div_idx < 5)) {
		post_div_idx++;
		vco_clk = (*lane_clock) * post_div[post_div_idx];
	}
	vco_clk = vco_clk * 1000000;
	phy_ctrl->rg_pll_posdiv = post_div_idx;
	phy_ctrl->rg_pll_fbkdiv = vco_clk / DEFAULT_MIPI_CLK_RATE;
	*lane_clock = phy_ctrl->rg_pll_fbkdiv * DEFAULT_MIPI_CLK_RATE /
		post_div[phy_ctrl->rg_pll_posdiv];
	HISI_DRM_INFO("rg_pll_prediv=%d, rg_pll_posdiv=%d, rg_pll_fbkdiv=%d\n",
		phy_ctrl->rg_pll_prediv, phy_ctrl->rg_pll_posdiv,
		phy_ctrl->rg_pll_fbkdiv);

	phy_ctrl->rg_0p8v = 0;
	phy_ctrl->rg_2p5g = 1;
	phy_ctrl->rg_320m = 0;
	phy_ctrl->rg_lpf_r = 0;

	phy_ctrl->rg_vrefsel_vcm = 0x51;

}

static void cphy_lane_param_config(struct mipi_dsi_phy_ctrl *phy_ctrl,
	uint64_t lane_clock, struct mipi_panel_info *mipi)
{
	uint32_t ui;
	uint32_t accuracy;
	uint32_t unit_tx_word_clk_hs;

	accuracy = 10;
	if (!lane_clock) {
		HISI_DRM_DEBUG("invalid lane clock!");
		return;
	}
	ui = (uint32_t)(10 * 1000000000UL * accuracy / lane_clock);
	/* unit of measurement */
	unit_tx_word_clk_hs = 7 * ui;

	/* CPHY Specification: 38ns <= t3_prepare <= 95ns */
	phy_ctrl->t_prepare = 650 * accuracy;

	/* CPHY Specification: 50ns <= t_lpx */
	phy_ctrl->t_lpx = 600 * accuracy + 8 * ui - unit_tx_word_clk_hs;

	/* CPHY Specification: 7*UI <= t_prebegin <= 448UI */
	phy_ctrl->t_prebegin = 350 * ui - unit_tx_word_clk_hs;

	/* CPHY Specification: 7*UI <= t_post <= 224*UI */
	phy_ctrl->t_post = 224 * ui - unit_tx_word_clk_hs;

	phy_ctrl->t_prepare = ROUND1(phy_ctrl->t_prepare, unit_tx_word_clk_hs);
	phy_ctrl->t_lpx = ROUND1(phy_ctrl->t_lpx, unit_tx_word_clk_hs);
	phy_ctrl->t_prebegin = ROUND1(phy_ctrl->t_prebegin, unit_tx_word_clk_hs);
	phy_ctrl->t_post = ROUND1(phy_ctrl->t_post, unit_tx_word_clk_hs);

	phy_ctrl->data_lane_lp2hs_time = phy_ctrl->t_lpx + phy_ctrl->t_prepare +
					phy_ctrl->t_prebegin + 5 + 17;
	phy_ctrl->data_lane_hs2lp_time = phy_ctrl->t_post + 8 + 5;

	phy_ctrl->lane_word_clk = lane_clock / 7;
	phy_ctrl->clk_division = (((phy_ctrl->lane_word_clk / 2) % mipi->max_tx_esc_clk) > 0) ?
		(uint32_t)(phy_ctrl->lane_word_clk / 2 / mipi->max_tx_esc_clk + 1) :
		(uint32_t)(phy_ctrl->lane_word_clk / 2 / mipi->max_tx_esc_clk);

	phy_ctrl->phy_stop_wait_time = phy_ctrl->t_post + 8 + 5;

}

void get_dsi_cphy_ctrl(struct hisi_dsi *dsi, struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	uint32_t m_pll = 0;
	uint32_t n_pll = 0;
	uint64_t lane_clock;

	struct mipi_panel_info *mipi = NULL;
	struct drm_display_mode *mode = NULL;

	if (dsi == NULL || phy_ctrl == NULL) {
		HISI_DRM_ERR("hisi_dsi or phy_ctrl is null.\n");
		return;
	}

	mode = &dsi->cur_mode;
	mipi = &dsi->panel_info->mipi;

	lane_clock = mipi->dsi_bit_clk_upt;
	HISI_DRM_INFO("Expected : lane_clock = %llu M\n", lane_clock);

	/************************  PLL parameters config  *********************/
	cphy_pll_param_config(phy_ctrl, &lane_clock);

	/********************  data lane parameters config  ******************/
	cphy_lane_param_config(phy_ctrl, lane_clock, mipi);

	HISI_DRM_INFO("CPHY clock_lane and data_lane config :\n"
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

uint32_t mipi_pixel_clk(struct hisi_dsi *dsi)
{
	struct hisi_panel_info *pinfo = NULL;

	if (!dsi) {
		HISI_DRM_ERR("dsi is NULL\n");
		return 0;
	}

	pinfo = dsi->panel_info;
	if (!pinfo) {
		HISI_DRM_ERR("panel_info is NULL\n");
		return 0;
	}

	if (pinfo->pxl_clk_rate_div == 0)
		return (uint32_t)pinfo->pxl_clk_rate;

	if ((pinfo->ifbc_type == IFBC_TYPE_NONE) &&
		!is_dual_mipi(dsi))
		pinfo->pxl_clk_rate_div = 1;

	return (uint32_t)pinfo->pxl_clk_rate / pinfo->pxl_clk_rate_div;
}

void mipi_config_phy_test_code(char __iomem *mipi_dsi_base,
	uint32_t test_code_addr, uint32_t test_code_parameter)
{
	if (!mipi_dsi_base) {
		HISI_DRM_ERR("mipi_dsi_base is NULL\n");
		return;
	}

	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL1_OFFSET, test_code_addr);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000002);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL1_OFFSET, test_code_parameter);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000002);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);
}

void mipi_config_cphy_spec1v0_parameter(char __iomem *mipi_dsi_base,
						struct hisi_dsi *dsi)
{
	uint32_t i;
	uint32_t addr;
	struct mipi_panel_info *mipi = NULL;
	struct mipi_dsi_phy_ctrl *dsi_phy_ctrl = NULL;

	if (!dsi) {
		HISI_DRM_ERR("dsi is NULL\n");
		return;
	}

	if (!dsi->panel_info) {
		HISI_DRM_ERR("panel_info is NULL\n");
		return;
	}

	mipi = &dsi->panel_info->mipi;
	dsi_phy_ctrl = &dsi->panel_info->dsi_phy_ctrl;
	for (i = 0; i <= mipi->lane_nums; i++) {
		/* Lane Timing Control - DPHY: THS-PREPARE/CPHY: T3-PREPARE */
		addr = MIPIDSI_PHY_TST_DATA_PREPARE + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(dsi_phy_ctrl->t_prepare));

		/* Lane Timing Control - TLPX */
		addr = MIPIDSI_PHY_TST_DATA_TLPX + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, DSS_REDUCE(dsi_phy_ctrl->t_lpx));
	}
}

static void clk_lane_request_set(char __iomem *mipi_dsi_base, struct mipi_dsi_phy_ctrl *dsi_phy_ctrl)
{
	/* pre_delay of clock lane request setting */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_PRE_DELAY,
					DSS_REDUCE(dsi_phy_ctrl->clk_pre_delay));

	/* post_delay of clock lane request setting */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_POST_DELAY,
					DSS_REDUCE(dsi_phy_ctrl->clk_post_delay));

	/* clock lane timing ctrl - t_lpx */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_TLPX,
					DSS_REDUCE(dsi_phy_ctrl->clk_t_lpx));

	/* clock lane timing ctrl - t_hs_prepare */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_PREPARE,
					DSS_REDUCE(dsi_phy_ctrl->clk_t_hs_prepare));

	/* clock lane timing ctrl - t_hs_zero */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_ZERO,
					DSS_REDUCE(dsi_phy_ctrl->clk_t_hs_zero));

	/* clock lane timing ctrl - t_hs_trial */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_TRAIL,
					DSS_REDUCE(dsi_phy_ctrl->clk_t_hs_trial));

}

void mipi_config_dphy_spec1v2_parameter(char __iomem *mipi_dsi_base, struct hisi_dsi *dsi)
{
	uint32_t i;
	uint32_t addr;
	struct mipi_panel_info *mipi = NULL;
	struct mipi_dsi_phy_ctrl *dsi_phy_ctrl = NULL;

	if (!dsi) {
		HISI_DRM_ERR("dsi is NULL\n");
		return;
	}

	if (!dsi->panel_info) {
		HISI_DRM_ERR("panel_info is NULL\n");
		return;
	}

	mipi = &dsi->panel_info->mipi;
	dsi_phy_ctrl = &dsi->panel_info->dsi_phy_ctrl;
	for (i = 0; i <= (mipi->lane_nums + 1); i++) {
		/* Lane Transmission Property */
		addr = MIPIDSI_PHY_TST_LANE_TRANSMISSION_PROPERTY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, 0x43);
	}

	clk_lane_request_set(mipi_dsi_base, dsi_phy_ctrl);

	for (i = 0; i <= (mipi->lane_nums + 1); i++) {
		if (i == 2)
			i++;  /* addr: lane0:0x60; lane1:0x80; lane2:0xC0; lane3:0xE0 */

		/* data lane pre_delay */
		addr = MIPIDSI_PHY_TST_DATA_PRE_DELAY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr,
					DSS_REDUCE(dsi_phy_ctrl->data_pre_delay));

		/* data lane post_delay */
		addr = MIPIDSI_PHY_TST_DATA_POST_DELAY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr,
					DSS_REDUCE(dsi_phy_ctrl->data_post_delay));

		/* data lane timing ctrl - t_lpx */
		addr = MIPIDSI_PHY_TST_DATA_TLPX + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr,
					DSS_REDUCE(dsi_phy_ctrl->data_t_lpx));

		/* data lane timing ctrl - t_hs_prepare */
		addr = MIPIDSI_PHY_TST_DATA_PREPARE + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr,
					DSS_REDUCE(dsi_phy_ctrl->data_t_hs_prepare));

		/* data lane timing ctrl - t_hs_zero */
		addr = MIPIDSI_PHY_TST_DATA_ZERO + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr,
					DSS_REDUCE(dsi_phy_ctrl->data_t_hs_zero));

		/* data lane timing ctrl - t_hs_trial */
		addr = MIPIDSI_PHY_TST_DATA_TRAIL + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr,
					DSS_REDUCE(dsi_phy_ctrl->data_t_hs_trial));

		HISI_DRM_DEBUG("DPHY spec1v2 config :\n"
			"addr=0x%x\n"
			"clk_pre_delay=%u\n"
			"clk_t_hs_trial=%u\n"
			"data_t_hs_zero=%u\n"
			"data_t_lpx=%u\n"
			"data_t_hs_prepare=%u\n",
			addr,
			dsi_phy_ctrl->clk_pre_delay,
			dsi_phy_ctrl->clk_t_hs_trial,
			dsi_phy_ctrl->data_t_hs_zero,
			dsi_phy_ctrl->data_t_lpx,
			dsi_phy_ctrl->data_t_hs_prepare);
	}
}

static uint32_t mipi_get_cmp_stopstate_value(struct mipi_panel_info *mipi)
{
	uint32_t cmp_stopstate_val = 0;

	if (mipi->lane_nums >= DSI_4_LANES)
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9) | BIT(11));
	else if (mipi->lane_nums >= DSI_3_LANES)
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9));
	else if (mipi->lane_nums >= DSI_2_LANES)
		cmp_stopstate_val = (BIT(4) | BIT(7));
	else
		cmp_stopstate_val = (BIT(4));

	return cmp_stopstate_val;
}

static bool mipi_phy_status_check(char __iomem *mipi_dsi_base, uint32_t expected_value)
{
	bool is_ready = false;
	unsigned long dw_jiffies;
	uint32_t tmp;

	dw_jiffies = jiffies + HZ / 2;
	do {
		tmp = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((tmp & expected_value) == expected_value) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	HISI_DRM_DEBUG("MIPIDSI_PHY_STATUS_OFFSET=0x%x.\n", tmp);
	return is_ready;
}

void init_dual_mipi_ctrl(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = NULL;
	struct dss_rect rect = { 0, 0, 0, 0 };
	uint32_t dmipi_hsize;

	if (!dsi) {
		HISI_DRM_ERR("dsi is NULL!\n");
		return;
	}

	ctx = dsi->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	if (!ctx->dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	rect.x = 0;
	rect.y = 0;
	rect.w = dsi->cur_mode.hdisplay;
	rect.h = dsi->cur_mode.vdisplay;
	dmipi_hsize = (((uint32_t)rect.w * 2 - 1) << 16) | ((uint32_t)rect.w - 1);

	mipi_ifbc_get_rect(dsi, &rect);

	set_reg(ctx->dss_base + DSS_DISP_GLB_OFFSET + DUAL_MIPI_HSIZE,
		dmipi_hsize, 32, 0);
	set_reg(ctx->dss_base + DSS_DISP_GLB_OFFSET + DUAL_MIPI_SWAP, 0x0, 1, 0);
}

static void phy_start_config(char __iomem *mipi_dsi_base, struct mipi_panel_info *mipi,
	struct hisi_panel_info *pinfo, struct hisi_dsi *dsi)
{
	bool is_ready = false;

	set_reg(mipi_dsi_base + MIPIDSI_PHY_IF_CFG_OFFSET, mipi->lane_nums, 2, 0);
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_division, 8, 0);
	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_division, 8, 8);

	outp32(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x00000000);

	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000001);
	outp32(mipi_dsi_base + MIPIDSI_PHY_TST_CTRL0_OFFSET, 0x00000000);

	mipi_cdphy_init_config(mipi_dsi_base, dsi);

	outp32(mipi_dsi_base + MIPIDSI_PHY_RSTZ_OFFSET, 0x0000000F);

	is_ready = mipi_phy_status_check(mipi_dsi_base, 0x01);
	if (is_ready == 0)
		HISI_DRM_INFO("phylock is not ready!\n");

	is_ready = mipi_phy_status_check(mipi_dsi_base, mipi_get_cmp_stopstate_value(mipi));
	if (is_ready == 0)
		HISI_DRM_INFO("phystopstateclklane is not ready!\n");
}

static void phy_end_config(char __iomem *mipi_dsi_base,
	struct hisi_panel_info *pinfo, struct dss_rect rect)
{
	set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 1);
	if (is_mipi_cmd_panel(pinfo)) {
		/* config to command mode */
		set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 0);
		/* ALLOWED_CMD_SIZE */
		set_reg(mipi_dsi_base + MIPIDSI_EDPI_CMD_SIZE_OFFSET, rect.w, 16, 0);

		/* cnt=2 in update-patial scene, cnt nees to be checked for different panels */
		if (pinfo->mipi.hs_wr_to_time == 0) {
			set_reg(mipi_dsi_base + MIPIDSI_HS_WR_TO_CNT_OFFSET, 0x1000002, 25, 0);
		} else {
			set_reg(mipi_dsi_base + MIPIDSI_HS_WR_TO_CNT_OFFSET,
				(0x1 << 24) | (pinfo->mipi.hs_wr_to_time *
					pinfo->dsi_phy_ctrl.lane_byte_clk / 1000000000UL), 25, 0);
		}

	}
	/* phy_stop_wait_time */
	set_reg(mipi_dsi_base + MIPIDSI_PHY_IF_CFG_OFFSET, pinfo->dsi_phy_ctrl.phy_stop_wait_time, 8, 8);

}

static void dpi_interface_config(char __iomem *mipi_dsi_base, struct hisi_panel_info *pinfo)
{
	set_reg(mipi_dsi_base + MIPIDSI_DPI_VCID_OFFSET, pinfo->mipi.vc, 2, 0);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_COLOR_CODING_OFFSET, pinfo->mipi.color_mode, 4, 0);

	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, pinfo->ldi.data_en_plr, 1, 0);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, pinfo->ldi.vsync_plr, 1, 1);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, pinfo->ldi.hsync_plr, 1, 2);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, 0x0, 1, 3);
	set_reg(mipi_dsi_base + MIPIDSI_DPI_CFG_POL_OFFSET, 0x0, 1, 4);
}

static void video_transmis_mode_config(char __iomem *mipi_dsi_base, struct hisi_panel_info *pinfo, struct dss_rect rect)
{
	/* video mode: low power mode */
	if (pinfo->mipi.lp11_flag == MIPI_DISABLE_LP11)
		set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x0f, 6, 8);
	else
		set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x3f, 6, 8);

	if (is_mipi_video_panel(pinfo)) {
		set_reg(mipi_dsi_base + MIPIDSI_DPI_LP_CMD_TIM_OFFSET, 0x4, 8, 16);
		/* video mode: send read cmd by lp mode */
		set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, 0x1, 1, 15);
	}
	if ((pinfo->mipi.dsi_version == DSI_1_2_VERSION)
		&& (is_mipi_video_panel(pinfo))
		&& ((pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE)
			|| (pinfo->ifbc_type == IFBC_TYPE_VESA3X_DUAL))) {
		set_reg(mipi_dsi_base + MIPIDSI_VID_PKT_SIZE_OFFSET, rect.w * pinfo->pxl_clk_rate_div, 14, 0);
		/* video vase3x must be set BURST mode */
		if (pinfo->mipi.burst_mode < DSI_BURST_SYNC_PULSES_1)
			pinfo->mipi.burst_mode = DSI_BURST_SYNC_PULSES_1;
	} else {
		set_reg(mipi_dsi_base + MIPIDSI_VID_PKT_SIZE_OFFSET, rect.w, 14, 0);
	}

	/* burst mode */
	set_reg(mipi_dsi_base + MIPIDSI_VID_MODE_CFG_OFFSET, pinfo->mipi.burst_mode, 2, 0);
	/* for dsi read, BTA enable */
	set_reg(mipi_dsi_base + MIPIDSI_PCKHDL_CFG_OFFSET, 0x1, 1, 2);
}

static void dpi_horizontal_timing_config(char __iomem *mipi_dsi_base,
	struct hisi_panel_info *pinfo, struct dss_rect rect, struct hisi_dsi *dsi, u32 *hline_time)
{
	u32 hsa_time = 0;
	u32 hbp_time = 0;
	u64 pixel_clk;

	if (pinfo->mipi.dsi_timing_support) {
		set_reg(mipi_dsi_base + MIPIDSI_VID_HSA_TIME_OFFSET, pinfo->mipi.hsa, 12, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_HBP_TIME_OFFSET, pinfo->mipi.hbp, 12, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_HLINE_TIME_OFFSET, pinfo->mipi.hline_time, 15, 0);
		if (pinfo->mipi.hline_time - (pinfo->mipi.hsa + pinfo->mipi.hbp + pinfo->mipi.dpi_hsize) < 0)
			HISI_DRM_INFO("wrong hfp\n");
		set_reg(mipi_dsi_base + MIPIDSI_VID_VSA_LINES_OFFSET, pinfo->mipi.vsa, 10, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_VBP_LINES_OFFSET, pinfo->mipi.vbp, 10, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_VFP_LINES_OFFSET, pinfo->mipi.vfp, 10, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_VACTIVE_LINES_OFFSET, rect.h, 14, 0);
	} else {
		pixel_clk = mipi_pixel_clk(dsi);
		if (!pixel_clk) {
			HISI_DRM_DEBUG("invalid pixel clk!");
			return;
		}

		if (pinfo->mipi.phy_mode == DPHY_MODE) {
			hsa_time = ROUND1(pinfo->ldi.h_pulse_width * pinfo->dsi_phy_ctrl.lane_byte_clk, pixel_clk);
			hbp_time = ROUND1(pinfo->ldi.h_back_porch * pinfo->dsi_phy_ctrl.lane_byte_clk, pixel_clk);
			*hline_time = ROUND1((pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch +
				rect.w + pinfo->ldi.h_front_porch) * pinfo->dsi_phy_ctrl.lane_byte_clk, pixel_clk);
		} else {
			hsa_time = ROUND1(pinfo->ldi.h_pulse_width * pinfo->dsi_phy_ctrl.lane_word_clk, pixel_clk);
			hbp_time = ROUND1(pinfo->ldi.h_back_porch * pinfo->dsi_phy_ctrl.lane_word_clk, pixel_clk);
			*hline_time = ROUND1((pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch +
				rect.w + pinfo->ldi.h_front_porch) * pinfo->dsi_phy_ctrl.lane_word_clk, pixel_clk);
		}
		HISI_DRM_DEBUG("hsa_time=%d, hbp_time=%d, hline_time=%d\n",
			hsa_time, hbp_time, *hline_time);
		set_reg(mipi_dsi_base + MIPIDSI_VID_HSA_TIME_OFFSET, hsa_time, 12, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_HBP_TIME_OFFSET, hbp_time, 12, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_HLINE_TIME_OFFSET, *hline_time, 15, 0);

		/* Define the Vertical line configuration */
		set_reg(mipi_dsi_base + MIPIDSI_VID_VSA_LINES_OFFSET, pinfo->ldi.v_pulse_width, 10, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_VBP_LINES_OFFSET, pinfo->ldi.v_back_porch, 10, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_VFP_LINES_OFFSET, pinfo->ldi.v_front_porch, 10, 0);
		set_reg(mipi_dsi_base + MIPIDSI_VID_VACTIVE_LINES_OFFSET, rect.h, 14, 0);
	}
	set_reg(mipi_dsi_base + MIPIDSI_TO_CNT_CFG_OFFSET, 0x7FF, 16, 0);

}

static void core_phy_param_config(char __iomem *mipi_dsi_base,
	struct hisi_panel_info *pinfo, struct hisi_dsi *dsi, u32 hline_time)
{
	/* auto ulps */
	uint32_t auto_ulps_enter_delay;
	uint32_t twakeup_cnt;
	uint32_t twakeup_clk_div;

	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_lane_lp2hs_time, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_LPCLK_CFG_OFFSET, pinfo->dsi_phy_ctrl.clk_lane_hs2lp_time, 10, 16);

	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_RD_CFG_OFFSET, 0x7FFF, 15, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET, pinfo->dsi_phy_ctrl.data_lane_lp2hs_time, 10, 0);
	set_reg(mipi_dsi_base + MIPIDSI_PHY_TMR_CFG_OFFSET, pinfo->dsi_phy_ctrl.data_lane_hs2lp_time, 10, 16);

	set_reg(mipi_dsi_base + MIPIDSI_CLKMGR_CFG_OFFSET, 0x5, 4, 16);

	if (is_mipi_cmd_panel(pinfo)) {
		auto_ulps_enter_delay = hline_time * 3 / 2;
		twakeup_clk_div = 8;
		/* twakeup_cnt*twakeup_clk_div*t_lanebyteclk>1ms */
		if (pinfo->mipi.phy_mode == CPHY_MODE)
			twakeup_cnt = pinfo->dsi_phy_ctrl.lane_word_clk / 1000 * 3 / 2 / twakeup_clk_div;
		else
			twakeup_cnt = pinfo->dsi_phy_ctrl.lane_byte_clk / 1000 * 3 / 2 / twakeup_clk_div;

		set_reg(mipi_dsi_base + AUTO_ULPS_ENTER_DELAY, auto_ulps_enter_delay, 32, 0);
		set_reg(mipi_dsi_base + AUTO_ULPS_WAKEUP_TIME, twakeup_clk_div, 16, 0);
		set_reg(mipi_dsi_base + AUTO_ULPS_WAKEUP_TIME, twakeup_cnt, 16, 16);
	}

	if (pinfo->mipi.phy_mode == CPHY_MODE)
		set_reg(mipi_dsi_base + PHY_MODE, 0x1, 1, 0);
	else
		set_reg(mipi_dsi_base + PHY_MODE, 0x0, 1, 0);

	mipi_ldi_init(dsi, mipi_dsi_base);

}

static void mipi_init(struct hisi_dsi *dsi, char __iomem *mipi_dsi_base)
{
	struct mipi_panel_info *mipi = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dss_rect rect = {0, 0, 0, 0};
	u32 hline_time = 0;
	int ret;

	WARN_ON(!dsi);
	WARN_ON(!mipi_dsi_base);

	pinfo = dsi->panel_info;
	mipi = &pinfo->mipi;

	if (mipi->max_tx_esc_clk == 0) {
		HISI_DRM_INFO("max_tx_esc_clk is invalid!");
		mipi->max_tx_esc_clk = DEFAULT_MAX_TX_ESC_CLK;
	}

	ret = memset_s(&(pinfo->dsi_phy_ctrl), sizeof(struct mipi_dsi_phy_ctrl),
		0, sizeof(struct mipi_dsi_phy_ctrl));
	if (ret != EOK) {
		HISI_DRM_ERR("memset for dsi_phy_ctrl failed!");
		return;
	}

	if (mipi->phy_mode == CPHY_MODE) {
		HISI_DRM_INFO("mipi init, phy_mode = CPHY\n");
		get_dsi_cphy_ctrl(dsi, &(pinfo->dsi_phy_ctrl));
	} else {
		get_dsi_dphy_ctrl(dsi, &(pinfo->dsi_phy_ctrl));
	}

	rect.x = 0;
	rect.y = 0;
	rect.w = dsi->cur_mode.hdisplay;
	rect.h = dsi->cur_mode.vdisplay;

	mipi_ifbc_get_rect(dsi, &rect);

	/***************Configure the PHY start**************/
	phy_start_config(mipi_dsi_base, mipi, pinfo, dsi);

	/*************************Configure the PHY end*************************/
	phy_end_config(mipi_dsi_base, pinfo, rect);

	/*--------------configuring the DPI packet transmission----------------*/
	/*
	 * 2. Configure the DPI Interface:
	 * This defines how the DPI interface interacts with the controller.
	 */
	dpi_interface_config(mipi_dsi_base, pinfo);

	/*
	 * 3. Select the Video Transmission Mode:
	 * This defines how the processor requires the video line to be
	 * transported through the DSI link.
	 */
	video_transmis_mode_config(mipi_dsi_base, pinfo, rect);

	/*
	 ** 4. Define the DPI Horizontal timing configuration:
	 **
	 ** Hsa_time = HSA*(PCLK period/Clk Lane Byte Period);
	 ** Hbp_time = HBP*(PCLK period/Clk Lane Byte Period);
	 ** Hline_time = (HSA+HBP+HACT+HFP)*(PCLK period/Clk Lane Byte Period);
	 */
	dpi_horizontal_timing_config(mipi_dsi_base, pinfo, rect, dsi, &hline_time);

	/* Configure core's phy parameters */
	core_phy_param_config(mipi_dsi_base, pinfo, dsi, hline_time);
	/* Waking up Core*/
	set_reg(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x1, 1, 0);
}

static int clk_prepare_and_enable(struct clk *clk_tmp)
{
	int ret = 0;

	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_DRM_ERR("clk_prepare failed, error=%d!\n", ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_DRM_ERR("clk_enable failed, error=%d!\n", ret);
			return -EINVAL;
		}
	}
	return ret;
}

int mipi_dsi_clk_enable(struct hisi_dsi *dsi)
{
	int ret;
	struct dsi_hw_ctx *ctx = NULL;

	if (!dsi) {
		HISI_DRM_ERR("dsi is NULL");
		return -EINVAL;
	}

	ctx = dsi->ctx;
	if (ctx == NULL) {
		HISI_DRM_ERR("dsi_hw_ctx is NULL");
		return -EINVAL;
	}

	ret = clk_prepare_and_enable(ctx->dss_dphy0_ref_clk);
	if (ret)
		return ret;

	ret = clk_prepare_and_enable(ctx->dss_dphy0_cfg_clk);
	if (ret)
		return ret;


	if (is_dual_mipi(dsi)) {
		ret = clk_prepare_and_enable(ctx->dss_dphy1_ref_clk);
		if (ret)
			return ret;

		ret = clk_prepare_and_enable(ctx->dss_dphy1_cfg_clk);
		if (ret)
			return ret;

	}
	return 0;
}

int mipi_dsi_clk_disable(struct hisi_dsi *dsi)
{
	struct clk *clk_tmp = NULL;
	struct dsi_hw_ctx *ctx = NULL;

	if (!dsi) {
		HISI_DRM_ERR("dsi is NULL");
		return -EINVAL;
	}

	ctx = dsi->ctx;
	if (ctx == NULL) {
		HISI_DRM_ERR("mipi dsi ctx is NULL");
		return -EINVAL;
	}

	clk_tmp = ctx->dss_dphy0_ref_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	clk_tmp = ctx->dss_dphy0_cfg_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	if (is_dual_mipi(dsi)) {
		clk_tmp = ctx->dss_dphy1_ref_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}

		clk_tmp = ctx->dss_dphy1_cfg_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}
	}
	return 0;
}

int mipi_dsi_on_sub1(struct hisi_dsi *dsi, char __iomem *mipi_dsi_base)
{
	WARN_ON(!mipi_dsi_base);

	/* mipi init */
	mipi_init(dsi, mipi_dsi_base);
#ifdef CONFIG_HISI_DSS_V510
	outp32(mipi_dsi_base + DSI_MEM_CTRL, 0x02600008);
#endif
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

static void pctrl_dphytx_stopcnt_config(struct hisi_dsi *dsi)
{
	uint64_t pctrl_dphytx_stopcnt = 0;
	uint32_t stopcnt_div;
	uint64_t lane_byte_clk;
	struct hisi_panel_info *pinfo = NULL;

	pinfo = dsi->panel_info;
	stopcnt_div = is_dual_mipi(dsi) ? 2 : 1;
	/* init: wait DPHY 4 data lane stopstate */
	if (is_mipi_video_panel(pinfo)) {
		if (pinfo->pxl_clk_rate) {
			pctrl_dphytx_stopcnt = (uint64_t)(pinfo->ldi.h_back_porch +
				pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width + pinfo->xres / stopcnt_div + 5) *
#ifdef CONFIG_HISI_DSS_V510
				DEFAULT_PCLK_PCTRL_RATE / (pinfo->pxl_clk_rate / stopcnt_div);
#else
				DEFAULT_PCLK_DSS_RATE / (pinfo->pxl_clk_rate / stopcnt_div);
#endif
		} else {
			HISI_DRM_DEBUG("pxl clk rate error!");
			return;
		}
	} else {
		if (pinfo->pxl_clk_rate) {
			pctrl_dphytx_stopcnt = (uint64_t)(pinfo->ldi.h_back_porch +
				pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width + 5) *
#ifdef CONFIG_HISI_DSS_V510
				DEFAULT_PCLK_PCTRL_RATE / (pinfo->pxl_clk_rate / stopcnt_div);
#else
				DEFAULT_PCLK_DSS_RATE / (pinfo->pxl_clk_rate / stopcnt_div);
#endif
		} else {
			HISI_DRM_DEBUG("pxl clk rate error!");
			return;
		}
	}

	HISI_DRM_DEBUG("pctrl_dphytx_stopcnt = %d\n", pctrl_dphytx_stopcnt);
	if (pinfo->mipi.dsi_timing_support) {
		lane_byte_clk = (pinfo->mipi.phy_mode == DPHY_MODE) ?
			pinfo->dsi_phy_ctrl.lane_byte_clk :
			pinfo->dsi_phy_ctrl.lane_word_clk;
		if (lane_byte_clk) {
#ifdef CONFIG_HISI_DSS_V510
			pctrl_dphytx_stopcnt = ROUND1(((uint64_t)pinfo->mipi.hline_time *
				DEFAULT_PCLK_PCTRL_RATE), lane_byte_clk);
#else
			pctrl_dphytx_stopcnt = ROUND1(((uint64_t)pinfo->mipi.hline_time *
				DEFAULT_PCLK_DSS_RATE), lane_byte_clk);
#endif
			HISI_DRM_INFO("pctrl_dphytx_stopcnt = %d\n", pctrl_dphytx_stopcnt);
		} else {
			HISI_DRM_DEBUG("invalid lane byte clk!");
			return;
		}
	}

	outp32(dsi->ctx->mipi_dsi0_base + MIPIDSI_DPHYTX_STOPSNT_OFFSET, (uint32_t)pctrl_dphytx_stopcnt);
	if (is_dual_mipi(dsi))
		outp32(dsi->ctx->mipi_dsi1_base + MIPIDSI_DPHYTX_STOPSNT_OFFSET, (uint32_t)pctrl_dphytx_stopcnt);
}

static void check_mipi_panel(struct hisi_panel_info *pinfo, char __iomem *mipi_dsi_base)
{
	if (is_mipi_video_panel(pinfo)) {
		/* switch to video mode */
		set_reg(mipi_dsi_base + MIPIDSI_MODE_CFG_OFFSET, 0x0, 1, 0);
	}

	if (is_mipi_cmd_panel(pinfo)) {
		/* cmd mode: high speed mode */
		set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 7, 8);
		set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 4, 16);
		set_reg(mipi_dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 1, 24);
	}
}

static void eotp_tx_enable(struct hisi_panel_info *pinfo, char __iomem *mipi_dsi_base)
{
	if (pinfo->mipi.phy_mode == DPHY_MODE) {
		/* Some vendors don't need eotp check */
		if (pinfo->mipi.eotp_disable_flag == 1)
			set_reg(mipi_dsi_base + MIPIDSI_PCKHDL_CFG_OFFSET, 0x0, 1, 0);
		else
			set_reg(mipi_dsi_base + MIPIDSI_PCKHDL_CFG_OFFSET, 0x1, 1, 0);
	}
}

static bool is_ifbc_vesa_panel(struct hisi_panel_info *pinfo)
{
	if ((pinfo->ifbc_type == IFBC_TYPE_VESA2X_SINGLE) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA2X_DUAL) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA3X_DUAL) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA3_75X_DUAL))
		return true;

	return false;
}

int mipi_dsi_on_sub2(struct hisi_dsi *dsi, char __iomem *mipi_dsi_base)
{
	struct hisi_panel_info *pinfo = NULL;

	if (dsi == NULL || mipi_dsi_base == NULL) {
		HISI_DRM_ERR("hisifd or mipi_dsi_base is null.\n");
		return 0;
	}

	pinfo = dsi->panel_info;
	if (!pinfo) {
		HISI_DRM_ERR("panel_info is null.\n");
		return 0;
	}

	check_mipi_panel(pinfo, mipi_dsi_base);

	eotp_tx_enable(pinfo, mipi_dsi_base);

	/* enable generate High Speed clock, non continue */
	if (pinfo->mipi.non_continue_en)
		set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x3, 2, 0);
	else
		set_reg(mipi_dsi_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x1, 2, 0);

	if ((pinfo->mipi.dsi_version == DSI_1_2_VERSION)
		&& is_ifbc_vesa_panel(pinfo)) {
		set_reg(mipi_dsi_base + MIPIDSI_DSC_PARAMETER_OFFSET, 0x01, 32, 0);
	}

	pctrl_dphytx_stopcnt_config(dsi);
	return 0;
}

int mipi_dsi_off_sub(char __iomem *mipi_dsi_base)
{
	if (mipi_dsi_base == NULL) {
		HISI_DRM_ERR("mipi_dsi_base is NULL");
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

static bool check_pctrl_trstop_flag(struct hisi_dsi *dsi, int time_count)
{
	bool is_ready = false;
	int count;
	uint32_t tmp = 0;
	uint32_t tmp1 = 0;
	char __iomem *addr, *addr1;
	struct dsi_hw_ctx *ctx = dsi->ctx;

	addr = ctx->mipi_dsi0_base + MIPIDSI_DPHYTX_TRSTOP_FLAG_OFFSET;
	addr1 = ctx->mipi_dsi1_base + MIPIDSI_DPHYTX_TRSTOP_FLAG_OFFSET;

	if (is_dual_mipi(dsi)) {
		for (count = 0; count < time_count; count++) {
			tmp = inp32(addr);
			tmp1 = inp32(addr1);
			if ((tmp & tmp1 & 0x1) == 0x1) {
				is_ready = true;
				break;
			}
			udelay(2);
		}
	} else {
		for (count = 0; count < time_count; count++) {
			tmp = inp32(addr);
			if ((tmp & 0x1) == 0x1) {
				is_ready = true;
				break;
			}
			udelay(2);
		}
	}

	return is_ready;
}

static uint32_t get_stopstate_msk_value(uint8_t lane_nums)
{
	uint32_t stopstate_msk = 0;

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

struct underflow_clear_timeval {
	struct timeval tv0;
	struct timeval tv1;
};

static int mipi_dsi_reset_underflow_clear(struct hisi_dsi *dsi)
{
	uint32_t vfp_time;
	char __iomem *mipi_dsi_base = NULL;
	uint64_t lane_byte_clk = 0;
	uint32_t stopstate_msk = 0;
	bool is_ready = false;
	struct underflow_clear_timeval tv;
	uint32_t timediff = 0;

	HISI_DRM_INFO("+");

	if (!dsi->ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return -EINVAL;
	}

	mipi_dsi_base = dsi->ctx->mipi_dsi0_base;
	if (!mipi_dsi_base) {
		HISI_DRM_ERR("mipi_dsi_base is NULL!\n");
		return -EINVAL;
	}

	hisi_drm_get_timestamp(&(tv.tv0));

	stopstate_msk = get_stopstate_msk_value(dsi->panel_info->mipi.lane_nums);

	lane_byte_clk = (dsi->panel_info->mipi.phy_mode == DPHY_MODE) ?
		dsi->panel_info->dsi_phy_ctrl.lane_byte_clk : dsi->panel_info->dsi_phy_ctrl.lane_word_clk;
	lane_byte_clk /= 1000000; /* unit conversion: Hz to MHz */

	if (lane_byte_clk == 0) {
		vfp_time = 200; /* exception, set 200us */
		DRM_WARN("lane_byte_clk == 0\n");
	} else {
		vfp_time = ((uint32_t)inp32(mipi_dsi_base + MIPIDSI_VID_VFP_LINES_OFFSET) & 0x3ff) + 1;
		vfp_time *= ((uint32_t)inp32(mipi_dsi_base + MIPIDSI_VID_HLINE_TIME_OFFSET) & 0x7fff);
		vfp_time /= lane_byte_clk;
	}

	set_reg(mipi_dsi_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 1, 1, 0);
	set_reg(mipi_dsi_base + MIPIDSI_DPHYTX_CTRL_OFFSET, stopstate_msk, 5, 3);
	while ((!is_ready) && (timediff < vfp_time)) {
		is_ready = check_pctrl_trstop_flag(dsi, 10);
		hisi_drm_get_timestamp(&(tv.tv1));
		timediff = hisi_drm_timestamp_diff(&(tv.tv0), &(tv.tv1));
	}
	set_reg(mipi_dsi_base + MIPIDSI_DPHYTX_CTRL_OFFSET, 0, 1, 0);

	if (!is_ready)
		HISI_DRM_ERR("check_pctrl_trstop_flag fail.\n");

	set_reg(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x0, 1, 0);
	udelay(5); /* timing constraint */
	set_reg(mipi_dsi_base + MIPIDSI_PWR_UP_OFFSET, 0x1, 1, 0);
	if (is_dual_mipi(dsi)) {
		set_reg(dsi->ctx->mipi_dsi1_base + MIPIDSI_PWR_UP_OFFSET, 0x0, 1, 0);
		udelay(5);
		set_reg(dsi->ctx->mipi_dsi1_base + MIPIDSI_PWR_UP_OFFSET, 0x1, 1, 0);
	}

	ldi_dsi_data_gate(dsi, true);
	enable_dsi_ldi(dsi);

	HISI_DRM_INFO("-");
	return 0;
}

static void dsi_vactive0_end_handler(struct hisi_dsi *dsi)
{
	struct hisi_drm_crtc *hisi_crtc = NULL;

	HISI_DRM_INFO_DSI_IRQ("BIT_VACTIVE0_END");
	hisi_crtc = dsi->hisi_crtc;

	if (hisi_crtc->underflow_flag) {
		hisi_crtc->underflow_flag = 0;
		mipi_dsi_reset_underflow_clear(dsi);
	}
}

static void dsi_vactive0_start_handler(struct hisi_dsi *dsi)
{
	struct hisi_drm_crtc *hisi_crtc = NULL;

	HISI_DRM_INFO_DSI_IRQ("BIT_VACTIVE0_START");
	hisi_crtc = dsi->hisi_crtc;

	if (is_mipi_cmd_panel(dsi->panel_info))
		hisi_crtc->vactive0_start_flag = 1;
	else
		hisi_crtc->vactive0_start_flag++;
	wake_up_interruptible_all(&hisi_crtc->vactive0_start_wq);

	if (is_mipi_cmd_panel(dsi->panel_info))
		disable_dsi_ldi(dsi);
}

static void dsi_underflow_clear_handler(struct hisi_dsi *dsi)
{
	uint32_t dpp_dbg;
	uint32_t mask;
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	struct hisi_drm_crtc *hisi_crtc = NULL;

	hisi_crtc = dsi->hisi_crtc;
	ctx = dsi->hisi_crtc->ctx;
	dpp_dbg = inp32(ctx->dss_base + DSS_DPP_OFFSET + DPP_DBG_CNT);
	HISI_DRM_ERR("ldi underflow dpp_dbg = 0x%x vactive0_start_flag=%d!\n",
				dpp_dbg, hisi_crtc->vactive0_start_flag);

	mipi_dsi0_base = dsi->ctx->mipi_dsi0_base;
	mask = inp32(mipi_dsi0_base + MIPI_LDI_CPU_ITF_INT_MSK);
	mask |= BIT_LDI_UNFLOW;
	outp32(mipi_dsi0_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);
	if (is_dual_mipi(dsi))
		outp32(dsi->ctx->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);

	hisi_crtc->underflow_flag = true;
	crtc_disable_ldi(hisi_crtc);
	hisi_dump_current_info(hisi_crtc);

	if (hisi_crtc->ldi_underflow_wq) {
		queue_work(hisi_crtc->ldi_underflow_wq, &hisi_crtc->ldi_underflow_work);
	} else {
		HISI_DRM_ERR("ldi_underflow_wq is NULL");
	}
}


irqreturn_t dss_dsi0_isr(int irq, void *ptr)
{
	struct hisi_dsi *dsi = NULL;
	struct hisi_drm_crtc *hisi_crtc = NULL;
	uint32_t isr_s2 = 0;
	uint32_t isr_te_vsync = 0;
	char __iomem *mipi_dsi0_base = NULL;

	if (ptr == NULL) {
		HISI_DRM_ERR("ptr is NULL\n");
		return IRQ_HANDLED;
	}

	dsi = ptr;
	hisi_crtc = dsi->hisi_crtc;
	if (hisi_crtc == NULL) {
		HISI_DRM_ERR("hisi_crtc is NULL\n");
		return IRQ_HANDLED;
	}

	if (!dsi->ctx) {
		HISI_DRM_ERR("ctx is NULL\n");
		return IRQ_HANDLED;
	}

	mipi_dsi0_base = dsi->ctx->mipi_dsi0_base;
	if (!mipi_dsi0_base) {
		HISI_DRM_ERR("mipi_dsi0_base is NULL\n");
		return IRQ_HANDLED;
	}

	isr_s2 = inp32(mipi_dsi0_base + MIPI_LDI_CPU_ITF_INTS);
	outp32(mipi_dsi0_base + MIPI_LDI_CPU_ITF_INTS, isr_s2);
	if (is_dual_mipi(dsi))
		outp32(dsi->ctx->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS, isr_s2);

	isr_s2 &= ~(inp32(mipi_dsi0_base + MIPI_LDI_CPU_ITF_INT_MSK));

	if (is_mipi_cmd_panel(dsi->panel_info))
		isr_te_vsync = BIT_LCD_TE0_PIN;
	else
		isr_te_vsync = BIT_VSYNC;

	if (isr_s2 & BIT_VACTIVE0_END)
		dsi_vactive0_end_handler(dsi);

	if (isr_s2 & BIT_VACTIVE0_START)
		dsi_vactive0_start_handler(dsi);

	if (isr_s2 & isr_te_vsync) {
		HISI_DRM_INFO_DSI_IRQ("isr_te_vsync");
		hisi_crtc->vsync_ctrl.vsync_timestamp = ktime_get();
		drm_crtc_handle_vblank(&hisi_crtc->base);
	}

	if (isr_s2 & BIT_LDI_UNFLOW)
		dsi_underflow_clear_handler(dsi);

	return IRQ_HANDLED;
}

/*lint +e414 +e529 +e548 +e559 +e578 +e644 +e647 +e712 +e715 +e737 +e744 +e776 +e838 +e850*/
