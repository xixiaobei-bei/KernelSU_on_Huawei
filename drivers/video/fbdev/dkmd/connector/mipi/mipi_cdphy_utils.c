/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <dpu/soc_dpu_define.h>

#include "dkmd_utils.h"
#include "mipi_cdphy_utils.h"
#include "mipi_dsi_drv.h"
#include "panel_drv.h"

void mipi_config_phy_test_code(char __iomem *mipi_dsi_base,
	uint32_t test_code_addr, uint32_t test_code_parameter)
{
	outp32(DPU_DSI_CDPHY_TEST_CTRL_1_ADDR(mipi_dsi_base), test_code_addr);
	outp32(DPU_DSI_CDPHY_TEST_CTRL_0_ADDR(mipi_dsi_base), 0x00000002);
	outp32(DPU_DSI_CDPHY_TEST_CTRL_0_ADDR(mipi_dsi_base), 0x00000000);
	outp32(DPU_DSI_CDPHY_TEST_CTRL_1_ADDR(mipi_dsi_base), test_code_parameter);
	outp32(DPU_DSI_CDPHY_TEST_CTRL_0_ADDR(mipi_dsi_base), 0x00000002);
	outp32(DPU_DSI_CDPHY_TEST_CTRL_0_ADDR(mipi_dsi_base), 0x00000000);
	pr_debug("cphy_addr[%#x]=%#x\n",test_code_addr, test_code_parameter);
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

static uint64_t get_default_lane_byte_clk(struct mipi_panel_info *mipi)
{
	int rg_pll_posdiv = 0;
	uint32_t post_div[6] = { 1, 2, 4, 8, 16, 32 };  /* clk division */
	uint64_t lane_clock;
	uint64_t vco_clk;
	uint32_t rg_pll_fbkdiv;

	if (mipi->dsi_bit_clk_default == 0) {
		pr_warn("reset dsi_bit_clk_default %u M -> %u M\n",
			mipi->dsi_bit_clk_default, mipi->dsi_bit_clk);
		mipi->dsi_bit_clk_default = mipi->dsi_bit_clk;
	}

	lane_clock = (uint64_t)(mipi->dsi_bit_clk_default);
	if (mipi->phy_mode == DPHY_MODE)
		lane_clock = lane_clock * 2;

	pr_info("default lane_clock %llu M\n", lane_clock);

	vco_clk = lane_clock * post_div[0];
	/* chip restrain, vco_clk_min and post_div index */
	while ((vco_clk <= 2000) && (rg_pll_posdiv < 5)) {
		rg_pll_posdiv++;
		vco_clk = lane_clock * post_div[rg_pll_posdiv];
	}
	vco_clk = vco_clk * 1000000;  /* MHZ to HZ */
	rg_pll_fbkdiv = vco_clk / DEFAULT_MIPI_CLK_RATE;
	lane_clock = rg_pll_fbkdiv * DEFAULT_MIPI_CLK_RATE / post_div[rg_pll_posdiv];

	pr_info("vco_clk %llu, rg_pll_fbkdiv %d, rg_pll_posdiv %d, lane_clock %llu\n",
		vco_clk, rg_pll_fbkdiv, rg_pll_posdiv, lane_clock);

	/* lanebyte clk formula which is stated in cdphy spec */
	if (mipi->phy_mode == DPHY_MODE)
		return lane_clock / 8;
	else
		return lane_clock / 7;
}

static void mipi_dsi_pll_dphy_config(struct mipi_dsi_phy_ctrl *phy_ctrl,
	uint64_t *lane_clock, int fpga_flag)
{
	uint32_t m_pll;
	uint32_t n_pll;
	uint64_t vco_div = 1;  /* default clk division */
	uint64_t vco_clk = 0;
	uint32_t post_div[6] = { 1, 2, 4, 8, 16, 32 }; /* clk division */
	int post_div_idx = 0;

	if (fpga_flag) {
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
			pr_err("80M <= lane_clock< = 2500M, not support lane_clock = %llu M\n", *lane_clock);
		}

		/* accord chip protocol, lane_clock from MHz to Hz */
		n_pll = 2;
		m_pll = (uint32_t)((*lane_clock) * vco_div * n_pll * 1000000UL / DEFAULT_MIPI_CLK_RATE);

		*lane_clock = m_pll * (DEFAULT_MIPI_CLK_RATE / n_pll) / vco_div;
		if (*lane_clock > 750000000)  /* 750MHz */
			phy_ctrl->rg_cp = 3;
		else if ((*lane_clock >= 80000000) && (*lane_clock <= 750000000)) /* 80M <= lane_clock <= 750M */
			phy_ctrl->rg_cp = 1;
		else
			pr_err("80M <= lane_clock <= 2500M, not support lane_clock = %llu M\n", *lane_clock);

		phy_ctrl->rg_pre_div = n_pll - 1;
		phy_ctrl->rg_div = m_pll;

		pr_info("lane_clock = %llu M, m_pll: %d, n_pll: %d\n", *lane_clock, m_pll, n_pll);

	} else {
		phy_ctrl->rg_pll_prediv = 0;
		vco_clk = (*lane_clock) * post_div[post_div_idx];

		/* vcc_clk_min and post_div index */
		while ((vco_clk <= VCO_CLK_MIN_VALUE) && (post_div_idx < 5)) {
			post_div_idx++;
			vco_clk = (*lane_clock) * post_div[post_div_idx];
		}

		vco_clk = vco_clk * 1000000; /* MHZ to HZ */
		phy_ctrl->rg_pll_posdiv = post_div_idx;
		phy_ctrl->rg_pll_fbkdiv = vco_clk / DEFAULT_MIPI_CLK_RATE;

		*lane_clock = phy_ctrl->rg_pll_fbkdiv * DEFAULT_MIPI_CLK_RATE / post_div[phy_ctrl->rg_pll_posdiv];

		pr_info("lane_clock = %llu M, rg_pll_prediv = %d, rg_pll_posdiv = %d, rg_pll_fbkdiv = %d\n",
			*lane_clock, phy_ctrl->rg_pll_prediv, phy_ctrl->rg_pll_posdiv,
			phy_ctrl->rg_pll_fbkdiv);
	}

	/* The following devil numbers from chip protocol */
	phy_ctrl->rg_0p8v = 0;
	phy_ctrl->rg_2p5g = 1;
	phy_ctrl->rg_320m = 0;
	phy_ctrl->rg_lpf_r = 0;
}

static uint32_t get_data_t_hs_prepare(struct mipi_panel_info *mipi,
	uint32_t accuracy, uint32_t ui)
{
	uint32_t data_t_hs_prepare;
	uint32_t prepare_val1;
	uint32_t prepare_val2;

	/*
	 * D-PHY Specification : 40ns + 4*UI <= data_t_hs_prepare <= 85ns + 6*UI
	 * clocked by TXBYTECLKHS
	 * 35 is default adjust value
	 */
	if (mipi->data_t_hs_prepare_adjust == 0)
		mipi->data_t_hs_prepare_adjust = 35;

	prepare_val1 = 400 * accuracy + 4 * ui + mipi->data_t_hs_prepare_adjust * ui;
	prepare_val2 = 850 * accuracy + 6 * ui - 8 * ui;
	data_t_hs_prepare = (prepare_val1 <= prepare_val2) ? prepare_val1 : prepare_val2;

	return data_t_hs_prepare;
}

static void mipi_dsi_clk_data_lane_dphy_config(struct mipi_panel_info *mipi,
	struct mipi_dsi_phy_ctrl *phy_ctrl, uint64_t lane_clock)
{
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

	/******************  clock/data lane parameters config  ******************/
	if (lane_clock == 0)
		return;

	accuracy = 10;  /* magnification */
	ui =  (uint32_t)(10 * 1000000000UL * accuracy / lane_clock);

	/* unit of measurement */
	unit_tx_byte_clk_hs = 8 * ui;

	/* D-PHY Specification : 60ns + 52*UI <= clk_post */
	clk_post = 600 * accuracy + 52 * ui + unit_tx_byte_clk_hs + mipi->clk_post_adjust * ui;

	/* D-PHY Specification : clk_pre >= 8*UI */
	clk_pre = 8 * ui + unit_tx_byte_clk_hs + mipi->clk_pre_adjust * ui;

	/* D-PHY Specification : clk_t_hs_exit >= 100ns */
	clk_t_hs_exit = (uint32_t)(1000 * accuracy + 100 * accuracy + mipi->clk_t_hs_exit_adjust * ui);

	/* clocked by TXBYTECLKHS */
	clk_pre_delay = 0 + mipi->clk_pre_delay_adjust * ui;

	/* D-PHY Specification : clk_t_hs_trial >= 60ns clocked by TXBYTECLKHS */
	clk_t_hs_trial = 600 * accuracy + 3 * unit_tx_byte_clk_hs + mipi->clk_t_hs_trial_adjust * ui;

	/* D-PHY Specification : 38ns <= clk_t_hs_prepare <= 95ns clocked by TXBYTECLKHS */
	clk_t_hs_prepare = 660 * accuracy;

	/* clocked by TXBYTECLKHS */
	data_post_delay = 0 + mipi->data_post_delay_adjust * ui;

	/*
	 * D-PHY Specification : data_t_hs_trial >= max( n*8*UI, 60ns + n*4*UI ),
	 * n = 1. clocked by TXBYTECLKHS
	 */
	data_t_hs_trial = ((600 * accuracy + 4 * ui) >= (8 * ui) ?
		(600 * accuracy + 4 * ui) : (8 * ui)) + 8 * ui +
		3 * unit_tx_byte_clk_hs + mipi->data_t_hs_trial_adjust * ui;

	/*
	 * D-PHY Specification : 40ns + 4*UI <= data_t_hs_prepare <= 85ns + 6*UI
	 * clocked by TXBYTECLKHS
	 */
	data_t_hs_prepare = get_data_t_hs_prepare(mipi, accuracy, ui);
	/*
	 * D-PHY chip spec : clk_t_lpx + clk_t_hs_prepare > 200ns
	 * D-PHY Specification : clk_t_lpx >= 50ns
	 * clocked by TXBYTECLKHS
	 */
	clk_t_lpx = (uint32_t)(2000 * accuracy + 10 * accuracy +
		mipi->clk_t_lpx_adjust * ui - clk_t_hs_prepare);
	/*
	 * D-PHY Specification : clk_t_hs_zero + clk_t_hs_prepare >= 300 ns
	 * clocked by TXBYTECLKHS
	 */
	clk_t_hs_zero = (uint32_t)(3000 * accuracy + 3 * unit_tx_byte_clk_hs +
		mipi->clk_t_hs_zero_adjust * ui - clk_t_hs_prepare);
	/*
	 * D-PHY chip spec : data_t_lpx + data_t_hs_prepare > 200ns
	 * D-PHY Specification : data_t_lpx >= 50ns
	 * clocked by TXBYTECLKHS
	 */
	data_t_lpx = (uint32_t)(2000 * accuracy + 10 * accuracy +
		mipi->data_t_lpx_adjust * ui - data_t_hs_prepare);
	/*
	 * D-PHY Specification : data_t_hs_zero + data_t_hs_prepare >= 145ns + 10*UI
	 * clocked by TXBYTECLKHS
	 */
	data_t_hs_zero = (uint32_t)(1450 * accuracy + 10 * ui +
		3 * unit_tx_byte_clk_hs + mipi->data_t_hs_zero_adjust * ui -
		data_t_hs_prepare);

	/* The follow code from chip code, It contains lots of fixed numbers */
	phy_ctrl->clk_pre_delay = DIV_ROUND_UP(clk_pre_delay, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_prepare = DIV_ROUND_UP(clk_t_hs_prepare, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_lpx = DIV_ROUND_UP(clk_t_lpx, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_zero = DIV_ROUND_UP(clk_t_hs_zero, unit_tx_byte_clk_hs);
	phy_ctrl->clk_t_hs_trial = DIV_ROUND_UP(clk_t_hs_trial, unit_tx_byte_clk_hs);

	phy_ctrl->data_post_delay = DIV_ROUND_UP(data_post_delay, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_prepare = DIV_ROUND_UP(data_t_hs_prepare, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_lpx = DIV_ROUND_UP(data_t_lpx, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_zero = DIV_ROUND_UP(data_t_hs_zero, unit_tx_byte_clk_hs);
	phy_ctrl->data_t_hs_trial = DIV_ROUND_UP(data_t_hs_trial, unit_tx_byte_clk_hs);

	phy_ctrl->clk_post_delay = phy_ctrl->data_t_hs_trial + DIV_ROUND_UP(clk_post, unit_tx_byte_clk_hs);
	phy_ctrl->data_pre_delay = get_data_pre_delay(mipi->lp11_flag,
		phy_ctrl, DIV_ROUND_UP(clk_pre, unit_tx_byte_clk_hs));

	phy_ctrl->clk_lane_lp2hs_time = phy_ctrl->clk_pre_delay +
		phy_ctrl->clk_t_lpx + phy_ctrl->clk_t_hs_prepare +
		phy_ctrl->clk_t_hs_zero + 5 + 7;

	phy_ctrl->clk_lane_hs2lp_time = phy_ctrl->clk_t_hs_trial +
		phy_ctrl->clk_post_delay + 8 + 4;

	phy_ctrl->data_lane_lp2hs_time =
		get_data_pre_delay_reality(mipi->lp11_flag, phy_ctrl) +
		phy_ctrl->data_t_lpx + phy_ctrl->data_t_hs_prepare +
		phy_ctrl->data_t_hs_zero + mipi->data_lane_lp2hs_time_adjust + 7;

	phy_ctrl->data_lane_hs2lp_time = phy_ctrl->data_t_hs_trial + 8 + 5;

	phy_ctrl->phy_stop_wait_time =
		get_clk_post_delay_reality(mipi->lp11_flag, phy_ctrl) +
		phy_ctrl->clk_t_hs_trial + DIV_ROUND_UP(clk_t_hs_exit, unit_tx_byte_clk_hs) -
		(phy_ctrl->data_post_delay + 4 + phy_ctrl->data_t_hs_trial) + 3;

	phy_ctrl->lane_byte_clk = lane_clock / 8;
	phy_ctrl->clk_division =
		(((phy_ctrl->lane_byte_clk / 2) % mipi->max_tx_esc_clk) > 0) ?
		(uint32_t)(phy_ctrl->lane_byte_clk / 2 / mipi->max_tx_esc_clk + 1) :
		(uint32_t)(phy_ctrl->lane_byte_clk / 2 / mipi->max_tx_esc_clk);

	phy_ctrl->lane_byte_clk_default = get_default_lane_byte_clk(mipi);
}

static void get_dsi_dphy_ctrl(struct mipi_dsi_drv_private *priv)
{
	uint32_t dsi_bit_clk;
	uint64_t lane_clock;
	struct mipi_panel_info *mipi = &priv->mipi;
	struct mipi_dsi_phy_ctrl *phy_ctrl = &priv->dsi_phy_ctrl;

	dsi_bit_clk = mipi->dsi_bit_clk_upt;
	lane_clock = (uint64_t)(2 * dsi_bit_clk);
	pr_info("Expected : lane_clock = %llu M\n", lane_clock);

	/************************  PLL parameters config  *********************/
	/* chip spec :
	 * If the output data rate is below 320 Mbps, RG_BNAD_SEL should be set to 1.
	 * At this mode a post divider of 1/4 will be applied to VCO.
	 */
	mipi_dsi_pll_dphy_config(phy_ctrl, &lane_clock, priv->fpga_flag);

	/* HSTX select VCM VREF */
	phy_ctrl->rg_vrefsel_vcm = 0x5d;
	if (mipi->rg_vrefsel_vcm_adjust != 0) {
		phy_ctrl->rg_vrefsel_vcm = mipi->rg_vrefsel_vcm_adjust;
		pr_info("rg_vrefsel_vcm=0x%x\n", phy_ctrl->rg_vrefsel_vcm);
	}

	mipi_dsi_clk_data_lane_dphy_config(mipi, phy_ctrl, lane_clock);

	pr_info("DPHY clock_lane and data_lane config :\n"
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
		phy_ctrl->phy_stop_wait_time);
}

static void mipi_dsi_get_cphy_div(struct mipi_dsi_phy_ctrl *phy_ctrl,
	uint64_t lane_clock, uint64_t *vco_div)
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
		pr_err("40M <= lane_clock <= 1500M, not support lane_clock = %llu M\n", lane_clock);
	}
}

static void mipi_dsi_pll_cphy_config(struct mipi_dsi_phy_ctrl *phy_ctrl,
	uint64_t *lane_clock, int fpga_flag)
{
	uint32_t m_pll = 0;
	uint32_t n_pll = 0;
	uint64_t vco_div = 1;  /* default clk division */
	uint64_t vco_clk = 0;
	uint32_t post_div[6] = { 1, 2, 4, 8, 16, 32 };  /* clk division */
	int post_div_idx = 0;

	if (fpga_flag) {
		mipi_dsi_get_cphy_div(phy_ctrl, *lane_clock, &vco_div);

		/* accord chip protocol, lane_clock from MHz to Hz */
		n_pll = 2;
		m_pll = (uint32_t)((*lane_clock) * vco_div * n_pll * 1000000UL / DEFAULT_MIPI_CLK_RATE);

		if (vco_div)
			*lane_clock = m_pll * (DEFAULT_MIPI_CLK_RATE / n_pll) / vco_div;

		if (*lane_clock > 750000000)  /* 750Mhz */
			phy_ctrl->rg_cp = 3;
		else if ((*lane_clock >= 40000000) && (*lane_clock <= 750000000)) /* 40M <= lane_clock <= 750M */
			phy_ctrl->rg_cp = 1;
		else
			pr_err("40M <= lane_clock <= 1500M, not support lane_clock = %llu M\n", *lane_clock);

		phy_ctrl->rg_pre_div = n_pll - 1;
		phy_ctrl->rg_div = m_pll;

		pr_info("lane_clock = %llu M, m_pll: %d, n_pll: %d\n", *lane_clock, m_pll, n_pll);

	} else {
		phy_ctrl->rg_pll_prediv = 0;
		vco_clk = (*lane_clock) * post_div[post_div_idx];

		while ((vco_clk <= VCO_CLK_MIN_VALUE) && (post_div_idx < 5)) {
			post_div_idx++;
			vco_clk = (*lane_clock) * post_div[post_div_idx];
		}

		vco_clk = vco_clk * 1000000;  /* MHz to Hz */
		phy_ctrl->rg_pll_posdiv = post_div_idx;
		phy_ctrl->rg_pll_fbkdiv = vco_clk / DEFAULT_MIPI_CLK_RATE;

		*lane_clock = phy_ctrl->rg_pll_fbkdiv * DEFAULT_MIPI_CLK_RATE /
			post_div[phy_ctrl->rg_pll_posdiv];

		pr_info("lane_clock = %llu M, rg_pll_prediv = %d, rg_pll_posdiv = %d, rg_pll_fbkdiv = %d\n",
			*lane_clock, phy_ctrl->rg_pll_prediv, phy_ctrl->rg_pll_posdiv,
			phy_ctrl->rg_pll_fbkdiv);
	}

	/* The following devil numbers from chip protocol */
	phy_ctrl->rg_0p8v = 0;
	phy_ctrl->rg_2p5g = 1;
	phy_ctrl->rg_320m = 0;
	phy_ctrl->rg_lpf_r = 0;
}

static void mipi_dsi_clk_data_lane_cphy_config(struct mipi_panel_info *mipi,
	struct mipi_dsi_phy_ctrl *phy_ctrl, uint64_t lane_clock)
{
	uint32_t accuracy;
	uint32_t ui;
	uint32_t unit_tx_word_clk_hs;

	/********************  data lane parameters config  ******************/
	if (lane_clock == 0)
		return;

	accuracy = 10;  /* magnification */
	ui = (uint32_t)(10 * 1000000000UL * accuracy / lane_clock);

	/* unit of measurement */
	unit_tx_word_clk_hs = 7 * ui;

	if (mipi->mininum_phy_timing_flag == 1) {
		/* CPHY Specification: 38ns <= t3_prepare <= 95ns */
		phy_ctrl->t_prepare = MIN_T3_PREPARE_PARAM * accuracy;

		/* CPHY Specification: 50ns <= t_lpx */
		phy_ctrl->t_lpx = MIN_T3_LPX_PARAM * accuracy + 8 * ui - unit_tx_word_clk_hs;

		/* CPHY Specification: 7*UI <= t_prebegin <= 448UI */
		phy_ctrl->t_prebegin = MIN_T3_PREBEGIN_PARAM * ui - unit_tx_word_clk_hs;

		/* CPHY Specification: 7*UI <= t_post <= 224*UI */
		phy_ctrl->t_post = MIN_T3_POST_PARAM * ui - unit_tx_word_clk_hs;
	} else {
		/* CPHY Specification: 38ns <= t3_prepare <= 95ns */
		/* 380 * accuracy - unit_tx_word_clk_hs; */
		phy_ctrl->t_prepare = T3_PREPARE_PARAM * accuracy;

		/* CPHY Specification: 50ns <= t_lpx */
		phy_ctrl->t_lpx =  T3_LPX_PARAM * accuracy + 8 * ui - unit_tx_word_clk_hs;

		/* CPHY Specification: 7*UI <= t_prebegin <= 448UI */
		phy_ctrl->t_prebegin =  T3_PREBEGIN_PARAM * ui - unit_tx_word_clk_hs;

		/* CPHY Specification: 7*UI <= t_post <= 224*UI */
		phy_ctrl->t_post = T3_POST_PARAM * ui - unit_tx_word_clk_hs;
	}

	/* The follow code from chip code, It contains lots of fixed numbers */
	phy_ctrl->t_prepare = DIV_ROUND_UP(phy_ctrl->t_prepare, unit_tx_word_clk_hs);
	phy_ctrl->t_lpx = DIV_ROUND_UP(phy_ctrl->t_lpx, unit_tx_word_clk_hs);
	phy_ctrl->t_prebegin = DIV_ROUND_UP(phy_ctrl->t_prebegin, unit_tx_word_clk_hs);
	phy_ctrl->t_post = DIV_ROUND_UP(phy_ctrl->t_post, unit_tx_word_clk_hs);

	phy_ctrl->data_lane_lp2hs_time = phy_ctrl->t_lpx + phy_ctrl->t_prepare +
		phy_ctrl->t_prebegin + 5 + 17;
	phy_ctrl->data_lane_hs2lp_time = phy_ctrl->t_post + 8 + 5;

	phy_ctrl->lane_word_clk = lane_clock / 7;
	phy_ctrl->clk_division =
		(((phy_ctrl->lane_word_clk / 2) % mipi->max_tx_esc_clk) > 0) ?
		(uint32_t)(phy_ctrl->lane_word_clk / 2 / mipi->max_tx_esc_clk + 1) :
		(uint32_t)(phy_ctrl->lane_word_clk / 2 / mipi->max_tx_esc_clk);

	phy_ctrl->phy_stop_wait_time = phy_ctrl->t_post + 8 + 5;
}

static void get_dsi_cphy_ctrl(struct mipi_dsi_drv_private *priv)
{
	uint64_t lane_clock;
	struct mipi_panel_info *mipi = &priv->mipi;
	struct mipi_dsi_phy_ctrl *phy_ctrl = &priv->dsi_phy_ctrl;

	lane_clock = mipi->dsi_bit_clk_upt;

	pr_info("Expected : lane_clock = %llu M\n", lane_clock);

	/************************  PLL parameters config  *********************/
	mipi_dsi_pll_cphy_config(phy_ctrl, &lane_clock, priv->fpga_flag);

	/* HSTX select VCM VREF */
	phy_ctrl->rg_vrefsel_vcm = 0x51;

	mipi_dsi_clk_data_lane_cphy_config(mipi, phy_ctrl, lane_clock);

	phy_ctrl->lane_byte_clk_default = get_default_lane_byte_clk(mipi);

	pr_info("CPHY clock_lane and data_lane config :\n"
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

static uint32_t get_hsize_after_spr_dsc(struct dkmd_connector_info *pinfo)
{
	// TODO: DSC calc

	return pinfo->xres;
}

static void get_mipi_dsi_timing(struct dkmd_connector_info *pinfo,
	struct mipi_panel_info *mipi)
{
	struct disp_rect rect;

	rect.x = 0;
	rect.y = 0;
	rect.w = get_hsize_after_spr_dsc(pinfo);
	rect.h = pinfo->yres;

	mipi->width = rect.w;
	mipi->vactive_line = pinfo->yres;

	pr_info("dsi_bit_clk_upt %llu M, htiming: %d, %d, %d, %d\n",
		mipi->dsi_bit_clk_upt, mipi->hsa,
		mipi->hbp, mipi->hline_time, mipi->dpi_hsize);
}

static void get_mipi_dsi_timing_config_para(struct mipi_panel_info *mipi,
	struct mipi_dsi_phy_ctrl *phy_ctrl, struct mipi_dsi_timing *timing)
{
	uint64_t lane_byte_clk;

	lane_byte_clk = (mipi->phy_mode == DPHY_MODE) ? phy_ctrl->lane_byte_clk : phy_ctrl->lane_word_clk;

	if (lane_byte_clk == phy_ctrl->lane_byte_clk_default) {
		timing->hsa = mipi->hsa;
		timing->hbp = mipi->hbp;
		timing->hline_time = mipi->hline_time;
	} else {
		if (phy_ctrl->lane_byte_clk_default == 0) {
			phy_ctrl->lane_byte_clk_default = get_default_lane_byte_clk(mipi);
			pr_err("change lane_byte_clk_default to %llu M\n",
				phy_ctrl->lane_byte_clk_default);
		}

		timing->hsa = (uint32_t)DIV_ROUND_UP(mipi->hsa * lane_byte_clk,
			phy_ctrl->lane_byte_clk_default);
		timing->hbp = (uint32_t)DIV_ROUND_UP(mipi->hbp * lane_byte_clk,
			phy_ctrl->lane_byte_clk_default);
		timing->hline_time = (uint32_t)DIV_ROUND_UP(mipi->hline_time * lane_byte_clk,
			phy_ctrl->lane_byte_clk_default);
	}

	timing->dpi_hsize = mipi->dpi_hsize;
	timing->width = mipi->width;
	timing->vsa = mipi->vsa;
	timing->vbp = mipi->vbp;
	timing->vfp = mipi->vfp;
	timing->vactive_line = mipi->vactive_line;

	pr_info("lanebyteclk: %llu M, %llu M, htiming: %d, %d, %d, %d "
		"new: %d, %d, %d, %d\n",
		lane_byte_clk, phy_ctrl->lane_byte_clk_default,
		mipi->hsa, mipi->hbp, mipi->hline_time,
		mipi->dpi_hsize, timing->hsa, timing->hbp, timing->hline_time,
		timing->dpi_hsize);
}

static void mipi_cdphy_pll_configuration(struct mipi_dsi_drv_private *priv,
	char __iomem *mipi_dsi_base, uint32_t rg_cphy_div_param)
{
	struct mipi_dsi_phy_ctrl *phy_ctrl = &priv->dsi_phy_ctrl;
	mipi_config_phy_test_code(mipi_dsi_base, 0x00010042, 0x21);

	if (priv->fpga_flag) {
		/* PLL configuration I */
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010046,
			phy_ctrl->rg_cp + (phy_ctrl->rg_lpf_r << 4));

		/* PLL configuration II */
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010048,
			phy_ctrl->rg_0p8v +
			(phy_ctrl->rg_2p5g << 1) +
			(phy_ctrl->rg_320m << 2) +
			(phy_ctrl->rg_band_sel << 3) +
			rg_cphy_div_param);

		/* PLL configuration III */
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010049,
			phy_ctrl->rg_pre_div);

		/* PLL configuration IV */
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A,
			phy_ctrl->rg_div);
	} else {
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010048,
			phy_ctrl->rg_pll_posdiv);
		mipi_config_phy_test_code(mipi_dsi_base, 0x00010049,
			(phy_ctrl->rg_pll_prediv << 4) |
			(phy_ctrl->rg_pll_fbkdiv >> 8));
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004A,
			(phy_ctrl->rg_pll_fbkdiv & 0xFF));
	}
}

void mipi_config_cphy_spec1v0_parameter(const struct mipi_dsi_phy_ctrl *phy_ctrl,
	struct mipi_panel_info *mipi, char __iomem *mipi_dsi_base, int fpga_flag)
{
	uint32_t i;
	uint32_t addr;

	for (i = 0; i <= mipi->lane_nums; i++) {
		if (fpga_flag) {
			/* Lane Transmission Property */
			addr = MIPIDSI_PHY_TST_LANE_TRANSMISSION_PROPERTY + (i << 5);
			mipi_config_phy_test_code(mipi_dsi_base, addr, 0x43);
		}

		/* Lane Timing Control - DPHY: THS-PREPARE/CPHY: T3-PREPARE */
		addr = MIPIDSI_PHY_TST_DATA_PREPARE + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, phy_reduce(phy_ctrl->t_prepare));

		/* Lane Timing Control - TLPX */
		addr = MIPIDSI_PHY_TST_DATA_TLPX + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, phy_reduce(phy_ctrl->t_lpx));
	}
}

static void mipi_config_dphy_spec1v2_parameter(const struct mipi_dsi_phy_ctrl *phy_ctrl,
	struct mipi_panel_info *mipi, char __iomem *mipi_dsi_base)
{
	uint32_t i;
	uint32_t addr;

	for (i = 0; i <= (mipi->lane_nums + 1); i++) {
		/* Lane Transmission Property */
		addr = MIPIDSI_PHY_TST_LANE_TRANSMISSION_PROPERTY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, 0x43);
	}

	/* pre_delay of clock lane request setting */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_PRE_DELAY,
		phy_reduce(phy_ctrl->clk_pre_delay));

	/* post_delay of clock lane request setting */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_POST_DELAY,
		phy_reduce(phy_ctrl->clk_post_delay));

	/* clock lane timing ctrl - t_lpx */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_TLPX,
		phy_reduce(phy_ctrl->clk_t_lpx));

	/* clock lane timing ctrl - t_hs_prepare */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_PREPARE,
		phy_reduce(phy_ctrl->clk_t_hs_prepare));

	/* clock lane timing ctrl - t_hs_zero */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_ZERO,
		phy_reduce(phy_ctrl->clk_t_hs_zero));

	/* clock lane timing ctrl - t_hs_trial */
	mipi_config_phy_test_code(mipi_dsi_base, MIPIDSI_PHY_TST_CLK_TRAIL,
		phy_reduce(phy_ctrl->clk_t_hs_trial));

	for (i = 0; i <= (mipi->lane_nums + 1); i++) {
		if (i == 2)
			i++;  /* addr: lane0:0x60; lane1:0x80; lane2:0xC0; lane3:0xE0 */

		/* data lane pre_delay */
		addr = MIPIDSI_PHY_TST_DATA_PRE_DELAY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, phy_reduce(phy_ctrl->data_pre_delay));

		/* data lane post_delay */
		addr = MIPIDSI_PHY_TST_DATA_POST_DELAY + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, phy_reduce(phy_ctrl->data_post_delay));

		/* data lane timing ctrl - t_lpx */
		addr = MIPIDSI_PHY_TST_DATA_TLPX + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, phy_reduce(phy_ctrl->data_t_lpx));

		/* data lane timing ctrl - t_hs_prepare */
		addr = MIPIDSI_PHY_TST_DATA_PREPARE + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, phy_reduce(phy_ctrl->data_t_hs_prepare));

		/* data lane timing ctrl - t_hs_zero */
		addr = MIPIDSI_PHY_TST_DATA_ZERO + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, phy_reduce(phy_ctrl->data_t_hs_zero));

		/* data lane timing ctrl - t_hs_trial */
		addr = MIPIDSI_PHY_TST_DATA_TRAIL + (i << 5);
		mipi_config_phy_test_code(mipi_dsi_base, addr, phy_reduce(phy_ctrl->data_t_hs_trial));

		pr_info("DPHY spec1v2 config:\n"
			"addr=%#x,\n"
			"clk_pre_delay=%#x,\n"
			"clk_t_hs_trial=%#x,\n"
			"data_t_hs_zero=%#x,\n"
			"data_t_lpx=%#x,\n"
			"data_t_hs_prepare=%#x,\n",
			addr,
			phy_ctrl->clk_pre_delay,
			phy_ctrl->clk_t_hs_trial,
			phy_ctrl->data_t_hs_zero,
			phy_ctrl->data_t_lpx,
			phy_ctrl->data_t_hs_prepare);
	}
}

static void mipi_cdphy_init_config(struct mipi_dsi_drv_private *priv,
	char __iomem *mipi_dsi_base)
{
	struct mipi_panel_info *mipi = &priv->mipi;
	struct mipi_dsi_phy_ctrl *phy_ctrl = &priv->dsi_phy_ctrl;

	if (mipi->phy_mode == CPHY_MODE) {
		if (mipi->mininum_phy_timing_flag == 1) {
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

		mipi_cdphy_pll_configuration(priv, mipi_dsi_base, (phy_ctrl->rg_cphy_div << 4));

		if (priv->fpga_flag) {
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004F, 0xf0);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010052, 0xa8);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010053, 0xc2);
		}

		mipi_config_phy_test_code(mipi_dsi_base, 0x00010058,
			(0x4 + mipi->lane_nums) << 4 | 0);
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001005B, 0x19);
		/* PLL update control */
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);

		/* set cphy spec parameter */
		mipi_config_cphy_spec1v0_parameter(phy_ctrl, mipi, mipi_dsi_base, priv->fpga_flag);
	} else {
		mipi_cdphy_pll_configuration(priv, mipi_dsi_base, 0);

		if (priv->fpga_flag) {
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001004F, 0xf0);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010050, 0xc0);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010051, 0x22);
			mipi_config_phy_test_code(mipi_dsi_base, 0x00010053, phy_ctrl->rg_vrefsel_vcm);
		}

		/* config dpy mipi drive voltage VCM  */
		if (mipi->rg_vrefsel_vcm_adjust != 0) {
			mipi_config_phy_test_code(mipi_dsi_base, 0x0001005B,
						mipi->rg_vrefsel_vcm_adjust);
			pr_info("rg_vrefsel_vcm = 0x%x\n", mipi->rg_vrefsel_vcm_adjust);
		}

		/* PLL update control */
		mipi_config_phy_test_code(mipi_dsi_base, 0x0001004B, 0x1);

		/* set dphy spec parameter */
		mipi_config_dphy_spec1v2_parameter(phy_ctrl, mipi, mipi_dsi_base);
	}
}

static bool mipi_phy_status_check(const char __iomem *mipi_dsi_base, uint32_t expected_value)
{
	bool is_ready = false;
	uint32_t temp = 0;
	unsigned long dw_jiffies;

	dw_jiffies = jiffies + HZ / 2;  /* HZ / 2 = 0.5s */
	do {
		temp = inp32(DPU_DSI_CDPHY_STATUS_ADDR(mipi_dsi_base));
		if ((temp & expected_value) == expected_value) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	pr_info("DPU_DSI_CDPHY_STATUS_ADDR = 0x%x.\n", temp);

	return is_ready;
}

static uint32_t mipi_get_cmp_stopstate_value(struct mipi_panel_info *mipi)
{
	uint32_t cmp_stopstate_val;

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

static void mipi_dsi_phy_config(struct mipi_dsi_drv_private *priv, char __iomem *mipi_dsi_base)
{
	bool is_ready = false;
	struct mipi_panel_info *mipi = &priv->mipi;
	struct mipi_dsi_phy_ctrl *phy_ctrl = &priv->dsi_phy_ctrl;

	/*************************Configure the PHY start*************************/
	set_reg(DPU_DSI_CDPHY_LANE_NUM_ADDR(mipi_dsi_base), mipi->lane_nums, 2, 0);
	set_reg(DPU_DSI_CLK_DIV_CTRL_ADDR(mipi_dsi_base), phy_ctrl->clk_division, 8, 0);
	set_reg(DPU_DSI_CLK_DIV_CTRL_ADDR(mipi_dsi_base), phy_ctrl->clk_division, 8, 8);

	outp32(DPU_DSI_CDPHY_RST_CTRL_ADDR(mipi_dsi_base), 0x00000000);
	outp32(DPU_DSI_CDPHY_TEST_CTRL_0_ADDR(mipi_dsi_base), 0x00000000);
	outp32(DPU_DSI_CDPHY_TEST_CTRL_0_ADDR(mipi_dsi_base), 0x00000001);
	outp32(DPU_DSI_CDPHY_TEST_CTRL_0_ADDR(mipi_dsi_base), 0x00000000);

	mipi_cdphy_init_config(priv, mipi_dsi_base);

	outp32(DPU_DSI_CDPHY_RST_CTRL_ADDR(mipi_dsi_base), 0x0000000F);

	is_ready = mipi_phy_status_check(mipi_dsi_base, 0x01);
	if (!is_ready)
		pr_info("dsi-%d, phylock is not ready!\n", priv->id);

	is_ready = mipi_phy_status_check(mipi_dsi_base, mipi_get_cmp_stopstate_value(mipi));
	if (!is_ready)
		pr_info("dsi-%d, phystopstateclklane is not ready!\n", priv->id);

	/*************************Configure the PHY end*************************/
}

static void mipi_dsi_config_dpi_interface(struct mipi_panel_info *mipi, char __iomem *mipi_dsi_base)
{
	DPU_DSI_VIDEO_POL_CTRL_UNION pol_ctrl = {
		.reg = {
			.dataen_active_low = 0, // pinfo->ldi.data_en_plr,
			.vsync_active_low = 1, // pinfo->ldi.vsync_plr,
			.hsync_active_low = 1, // pinfo->ldi.hsync_plr,
			.shutd_active_low = 0,
			.colorm_active_low = 0,
			.reserved = 0,
		},
	};

	set_reg(DPU_DSI_VIDEO_VCID_ADDR(mipi_dsi_base), mipi->vc, 2, 0);
	set_reg(DPU_DSI_VIDEO_COLOR_FORMAT_ADDR(mipi_dsi_base), mipi->color_mode, 4, 0);

	outp32(DPU_DSI_VIDEO_POL_CTRL_ADDR(mipi_dsi_base), pol_ctrl.value);
}

static void mipi_dsi_video_mode_config(struct mipi_dsi_drv_private *priv,
	struct mipi_dsi_timing *timing, char __iomem *mipi_dsi_base)
{
	struct mipi_panel_info *mipi = &priv->mipi;
	struct dkmd_connector_info *pinfo = &priv->base;

	/* video mode: low power mode */
	if (mipi->lp11_flag == MIPI_DISABLE_LP11)
		set_reg(DPU_DSI_VIDEO_MODE_CTRL_ADDR(mipi_dsi_base), 0x0f, 6, 8);
	else
		set_reg(DPU_DSI_VIDEO_MODE_CTRL_ADDR(mipi_dsi_base), 0x3f, 6, 8);

	if (is_mipi_video_panel(pinfo)) {
		set_reg(DPU_DSI_VIDEO_MODE_LP_CMD_TIMING_ADDR(mipi_dsi_base), 0x4, 8, 16);
		/* video mode: send read cmd by lp mode */
		set_reg(DPU_DSI_VIDEO_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 15);
	}

	if ((mipi->dsi_version == DSI_1_2_VERSION) &&	(is_mipi_video_panel(pinfo)) &&
		((pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE) || (pinfo->ifbc_type == IFBC_TYPE_VESA3X_DUAL))) {
		set_reg(DPU_DSI_VIDEO_PKT_LEN_ADDR(mipi_dsi_base),
			timing->width * mipi->pxl_clk_rate_div, 14, 0);

		/* video vase3x must be set BURST mode */
		if (mipi->burst_mode < DSI_BURST_SYNC_PULSES_1) {
			pr_info("mipi->burst_mode = %d, video need config "
				"BURST mode\n", mipi->burst_mode);
			mipi->burst_mode = DSI_BURST_SYNC_PULSES_1;
		}
	} else {
		set_reg(DPU_DSI_VIDEO_PKT_LEN_ADDR(mipi_dsi_base), timing->width, 14, 0);
	}

	/* burst mode */
	set_reg(DPU_DSI_VIDEO_MODE_CTRL_ADDR(mipi_dsi_base), mipi->burst_mode, 2, 0);
}

static void mipi_dsi_timing_config(struct mipi_dsi_timing *timing, char __iomem *mipi_dsi_base)
{
	if (timing->hline_time < (timing->hsa + timing->hbp + timing->dpi_hsize))
		pr_info("wrong hfp!\n");

	set_reg(DPU_DSI_VIDEO_HSA_NUM_ADDR(mipi_dsi_base), timing->hsa, 12, 0);
	set_reg(DPU_DSI_VIDEO_HBP_NUM_ADDR(mipi_dsi_base), timing->hbp, 12, 0);
	set_reg(DPU_DSI_VIDEO_HLINE_NUM_ADDR(mipi_dsi_base), timing->hline_time, 15, 0);

	set_reg(DPU_DSI_VIDEO_VSA_NUM_ADDR(mipi_dsi_base), timing->vsa, 10, 0);
	set_reg(DPU_DSI_VIDEO_VBP_NUM_ADDR(mipi_dsi_base), timing->vbp, 10, 0);

	set_reg(DPU_DSI_VIDEO_VFP_NUM_ADDR(mipi_dsi_base), timing->vfp, 10, 0);
	set_reg(DPU_DSI_VSYNC_DELAY_TIME_ADDR(mipi_dsi_base), 0, 32, 0);
	set_reg(DPU_DSI_VIDEO_VACT_NUM_ADDR(mipi_dsi_base), timing->vactive_line, 14, 0);
	set_reg(DPU_DSI_TO_TIME_CTRL_ADDR(mipi_dsi_base), 0x7FF, 16, 0);
}

static void mipi_dsi_phy_timing_config(struct mipi_dsi_phy_ctrl *phy_ctrl, char __iomem *mipi_dsi_base)
{
	/* Configure core's phy parameters */
	set_reg(DPU_DSI_CLKLANE_TRANS_TIME_ADDR(mipi_dsi_base), phy_ctrl->clk_lane_lp2hs_time, 10, 0);
	set_reg(DPU_DSI_CLKLANE_TRANS_TIME_ADDR(mipi_dsi_base), phy_ctrl->clk_lane_hs2lp_time, 10, 16);

	set_reg(DPU_DSI_CDPHY_MAX_RD_TIME_ADDR(mipi_dsi_base), 0x7FFF, 15, 0);
	set_reg(DPU_DSI_DATALANE_TRNAS_TIME_ADDR(mipi_dsi_base), phy_ctrl->data_lane_lp2hs_time, 10, 0);
	set_reg(DPU_DSI_DATALANE_TRNAS_TIME_ADDR(mipi_dsi_base), phy_ctrl->data_lane_hs2lp_time, 10, 16);
}

static void disable_mipi_ldi(struct dkmd_connector_info *pinfo, char __iomem *mipi_dsi_base)
{
	if (is_dual_mipi_panel(pinfo))
		set_reg(DPU_DSI_LDI_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 5);
	else
		set_reg(DPU_DSI_LDI_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 0);
}

static void mipi_ldi_init(struct mipi_dsi_drv_private *priv,
	struct dkmd_connector_info *pinfo,
	char __iomem *mipi_dsi_base)
{
	uint64_t lane_byte_clk;
	struct mipi_dsi_timing timing;
	struct mipi_panel_info *mipi = &priv->mipi;
	struct mipi_dsi_phy_ctrl *phy_ctrl = &priv->dsi_phy_ctrl;

	lane_byte_clk = (mipi->phy_mode == CPHY_MODE) ?	phy_ctrl->lane_word_clk : phy_ctrl->lane_byte_clk;

	memset(&timing, 0, sizeof(timing));
	get_mipi_dsi_timing_config_para(mipi, phy_ctrl, &timing);

	set_reg(DPU_DSI_LDI_DPI0_HRZ_CTRL3_ADDR(mipi_dsi_base), phy_reduce(timing.dpi_hsize), 12, 0);
	set_reg(DPU_DSI_LDI_DPI0_HRZ_CTRL2_ADDR(mipi_dsi_base), phy_reduce(timing.width), 12, 0);
	set_reg(DPU_DSI_LDI_VRT_CTRL2_ADDR(mipi_dsi_base), phy_reduce(timing.vactive_line), 12, 0);

	disable_mipi_ldi(pinfo, mipi_dsi_base);
	if (is_mipi_video_panel(pinfo)) {
		set_reg(DPU_DSI_LDI_FRM_MSK_ADDR(mipi_dsi_base), 0x0, 1, 0);
		set_reg(DPU_DSI_CMD_MOD_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 1);
	}

	if (is_mipi_cmd_panel(pinfo)) {
		if (pinfo->dsi_type == DSI0_INDEX)
			set_reg(DPU_DSI_TE_CTRL_ADDR(mipi_dsi_base), (0x1 << 17) | (0x1 << 6) | 0x1, 18, 0);

		set_reg(DPU_DSI_TE_HS_NUM_ADDR(mipi_dsi_base), 0x0, 32, 0);
		set_reg(DPU_DSI_TE_HS_WD_ADDR(mipi_dsi_base), 0x24024, 32, 0);

		if (mipi->dsi_te_type == DSI1_TE1_TYPE)
			set_reg(DPU_DSI_TE_VS_WD_ADDR(mipi_dsi_base), ((2 * lane_byte_clk / 1000000) << 12) | 0x3FC, 32, 0);
		else
			set_reg(DPU_DSI_TE_VS_WD_ADDR(mipi_dsi_base), (0x3FC << 12) | (2 * lane_byte_clk / 1000000), 32, 0);

		set_reg(DPU_DSI_SHADOW_REG_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);

		/* enable vsync delay when dirty region update */
		set_reg(DPU_DSI_VSYNC_DELAY_CTRL_ADDR(mipi_dsi_base), 0x2, 2, 0);
		set_reg(DPU_DSI_VSYNC_DELAY_TIME_ADDR(mipi_dsi_base), 0x0, 32, 0);

		set_reg(DPU_DSI_LDI_FRM_MSK_ADDR(mipi_dsi_base), 0x1, 1, 0);
	}
}

void mipi_init(struct platform_device *pdev, char __iomem *mipi_dsi_base)
{
	struct mipi_dsi_timing timing;
	struct mipi_dsi_drv_private *priv = to_mipi_dsi_private(pdev);
	struct mipi_panel_info *mipi = &priv->mipi;
	struct dkmd_connector_info *pinfo = &priv->base;
	struct mipi_dsi_phy_ctrl *phy_ctrl = &priv->dsi_phy_ctrl;

	if (mipi->max_tx_esc_clk == 0) {
		pr_warn("max_tx_esc_clk is invalid!\n");
		mipi->max_tx_esc_clk = DEFAULT_MAX_TX_ESC_CLK;
	}

	memset(phy_ctrl, 0, sizeof(struct mipi_dsi_phy_ctrl));
	if (mipi->phy_mode == CPHY_MODE)
		get_dsi_cphy_ctrl(priv);
	else
		get_dsi_dphy_ctrl(priv);

	get_mipi_dsi_timing(pinfo, mipi);

	set_reg(DPU_DSI_GLB_DSI_RESET_EN_ADDR(priv->dpu_base + DPU_MIPI_DSI_GLB_OFFSET), 1, 1, priv->id);

	memset(&timing, 0, sizeof(timing));
	get_mipi_dsi_timing_config_para(mipi, phy_ctrl, &timing);

	mipi_dsi_phy_config(priv, mipi_dsi_base);

	set_reg(DPU_DSI_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 1);
	if (is_mipi_cmd_panel(pinfo)) {
		set_reg(DPU_DSI_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);
		set_reg(DPU_DSI_EDPI_CMD_SIZE_ADDR(mipi_dsi_base), timing.width, 16, 0);

		/* cnt=2 in update-patial scene, cnt nees to be checked for different panels */
		if (mipi->hs_wr_to_time == 0)
			set_reg(DPU_DSI_HS_WR_TO_TIME_CTRL_ADDR(mipi_dsi_base), 0x1000002, 25, 0);
		else
			set_reg(DPU_DSI_HS_WR_TO_TIME_CTRL_ADDR(mipi_dsi_base),
				(0x1 << 24) | (mipi->hs_wr_to_time * phy_ctrl->lane_byte_clk / 1000000000UL), 25, 0);
	}

	/* phy_stop_wait_time */
	set_reg(DPU_DSI_CDPHY_LANE_NUM_ADDR(mipi_dsi_base), phy_ctrl->phy_stop_wait_time, 8, 8);

	/* --------------configuring the DPI packet transmission---------------- */
	/*
	 * 2. Configure the DPI Interface:
	 * This defines how the DPI interface interacts with the controller.
	 */
	mipi_dsi_config_dpi_interface(mipi, mipi_dsi_base);

	/*
	 * 3. Select the Video Transmission Mode:
	 * This defines how the processor requires the video line to be
	 * transported through the DSI link.
	 */
	mipi_dsi_video_mode_config(priv, &timing, mipi_dsi_base);

	/* for dsi read, BTA enable */
	set_reg(DPU_DSI_PERIP_CHAR_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 2);

	/*
	 * 4. Define the DPI Horizontal timing configuration:
	 *
	 * Hsa_time = HSA*(PCLK period/Clk Lane Byte Period);
	 * Hbp_time = HBP*(PCLK period/Clk Lane Byte Period);
	 * Hline_time = (HSA+HBP+HACT+HFP)*(PCLK period/Clk Lane Byte Period);
	 */
	mipi_dsi_timing_config(&timing, mipi_dsi_base);

	mipi_dsi_phy_timing_config(phy_ctrl, mipi_dsi_base);

	/* 16~19bit: pclk_en, pclk_sel, dpipclk_en, dpipclk_sel */
	set_reg(DPU_DSI_CLK_DIV_CTRL_ADDR(mipi_dsi_base), 0x5, 4, 16);

	if (mipi->phy_mode == CPHY_MODE)
		set_reg(DPU_DSI_CPHY_OR_DPHY_ADDR(mipi_dsi_base), 0x1, 1, 0);
	else
		set_reg(DPU_DSI_CPHY_OR_DPHY_ADDR(mipi_dsi_base), 0x0, 1, 0);

	mipi_ldi_init(priv, pinfo, mipi_dsi_base);

	/* Waking up Core */
	set_reg(DPU_DSI_POR_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);

	pr_info("Waking up Core!\n");
}

MODULE_LICENSE("GPL");