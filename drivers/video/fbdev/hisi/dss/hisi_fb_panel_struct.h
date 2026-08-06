/* Copyright (c) 2013-2021, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#ifndef HISI_FB_PANEL_STRUCT_H
#define HISI_FB_PANEL_STRUCT_H

#include "hisi_fb_panel_enum.h"

/* resource desc */
struct resource_desc {
	uint32_t flag;
	char *name;
	uint32_t *value;
};

/* vcc desc */
struct vcc_desc {
	int dtype;
	char *id;
	struct regulator **regulator;
	int min_uV;
	int max_uV;
	int waittype;
	int wait;
};

/* pinctrl data */
struct pinctrl_data {
	struct pinctrl *p;
	struct pinctrl_state *pinctrl_def;
	struct pinctrl_state *pinctrl_idle;
};

struct pinctrl_cmd_desc {
	int dtype;
	struct pinctrl_data *pctrl_data;
	int mode;
};

/* gpio desc */
struct gpio_desc {
	int dtype;
	int waittype;
	int wait;
	char *label;
	uint32_t *gpio;
	int value;
};

struct spi_cmd_desc {
	int reg_len;
	char *reg;
	int val_len;
	char *val;
	int waittype;
	int wait;
};

struct mipi_dsi_timing {
	uint32_t hsa;
	uint32_t hbp;
	uint32_t dpi_hsize;
	uint32_t width;
	uint32_t hline_time;

	uint32_t vsa;
	uint32_t vbp;
	uint32_t vactive_line;
	uint32_t vfp;
};

typedef struct mipi_ifbc_division {
	uint32_t xres_div;
	uint32_t yres_div;
	uint32_t comp_mode;
	uint32_t pxl0_div2_gt_en;
	uint32_t pxl0_div4_gt_en;
	uint32_t pxl0_divxcfg;
	uint32_t pxl0_dsi_gt_en;
} mipi_ifbc_division_t;

/* for sensorhub_aod, keep 32-bit aligned */
struct ldi_panel_info {
	/*
	 * For video panel support DP 4K on 501
	 * pipe_clk_rate_pre_set: pipe clk for DP 4k
	 * hporch_pre_set[3]: Proch for DP 4K
	 * div_pre_set:div setting for DP 4K
	 */
	uint64_t pipe_clk_rate_pre_set;
	uint32_t hporch_pre_set[PIPE_CLK_HPORCH_SET_ID];
	uint32_t div_pre_set;

	uint32_t h_back_porch;
	uint32_t h_front_porch;
	uint32_t h_pulse_width;

	/*
	 * note: vbp > 8 if used overlay compose,
	 * also lcd vbp > 8 in lcd power on sequence
	 */
	uint32_t v_back_porch;
	uint32_t v_front_porch;
	uint32_t v_pulse_width;

	uint32_t hbp_store_vid;
	uint32_t hfp_store_vid;
	uint32_t hpw_store_vid;
	uint32_t vbp_store_vid;
	uint32_t vfp_store_vid;
	uint32_t vpw_store_vid;
	uint64_t pxl_clk_store_vid;

	uint32_t hbp_store_cmd;
	uint32_t hfp_store_cmd;
	uint32_t hpw_store_cmd;
	uint32_t vbp_store_cmd;
	uint32_t vfp_store_cmd;
	uint32_t vpw_store_cmd;
	uint64_t pxl_clk_store_cmd;

	uint8_t hsync_plr;
	uint8_t vsync_plr;
	uint8_t pixelclk_plr;
	uint8_t data_en_plr;

	/* for cabc */
	uint8_t dpi0_overlap_size;
	uint8_t dpi1_overlap_size;
};

/* DSI PHY configuration */
struct mipi_dsi_phy_ctrl {
	uint64_t lane_byte_clk;
	uint64_t lane_word_clk;
	uint64_t lane_byte_clk_default;
	uint32_t clk_division;

	uint32_t clk_lane_lp2hs_time;
	uint32_t clk_lane_hs2lp_time;
	uint32_t data_lane_lp2hs_time;
	uint32_t data_lane_hs2lp_time;
	uint32_t clk2data_delay;
	uint32_t data2clk_delay;

	uint32_t clk_pre_delay;
	uint32_t clk_post_delay;
	uint32_t clk_t_lpx;
	uint32_t clk_t_hs_prepare;
	uint32_t clk_t_hs_zero;
	uint32_t clk_t_hs_trial;
	uint32_t clk_t_wakeup;
	uint32_t data_pre_delay;
	uint32_t data_post_delay;
	uint32_t data_t_lpx;
	uint32_t data_t_hs_prepare;
	uint32_t data_t_hs_zero;
	uint32_t data_t_hs_trial;
	uint32_t data_t_ta_go;
	uint32_t data_t_ta_get;
	uint32_t data_t_wakeup;

	uint32_t phy_stop_wait_time;

	uint32_t rg_lptx_sri;
	uint32_t rg_vrefsel_lptx;
	uint32_t rg_vrefsel_vcm;
	uint32_t rg_hstx_ckg_sel;
	uint32_t rg_pll_fbd_div5f;
	uint32_t rg_pll_fbd_div1f;
	uint32_t rg_pll_fbd_2p;
	uint32_t rg_pll_enbwt;
	uint32_t rg_pll_fbd_p;
	uint32_t rg_pll_fbd_s;
	uint32_t rg_pll_pre_div1p;
	uint32_t rg_pll_pre_p;
	uint32_t rg_pll_vco_750m;
	uint32_t rg_pll_lpf_rs;
	uint32_t rg_pll_lpf_cs;
	uint32_t rg_pll_enswc;
	uint32_t rg_pll_chp;

	/* only for 3660 use */
	uint32_t pll_register_override;
	uint32_t pll_power_down;
	uint32_t rg_band_sel;
	uint32_t rg_phase_gen_en;
	uint32_t reload_sel;
	uint32_t rg_pll_cp_p;
	uint32_t rg_pll_refsel;
	uint32_t rg_pll_cp;
	uint32_t load_command;

	/* for CDPHY */
	uint32_t rg_cphy_div;
	uint32_t rg_div;
	uint32_t rg_pre_div;
	uint32_t rg_320m;
	uint32_t rg_2p5g;
	uint32_t rg_0p8v;
	uint32_t rg_lpf_r;
	uint32_t rg_cp;
	uint32_t rg_pll_fbkdiv;
	uint32_t rg_pll_prediv;
	uint32_t rg_pll_posdiv;
	uint32_t t_prepare;
	uint32_t t_lpx;
	uint32_t t_prebegin;
	uint32_t t_post;
};

/* cphy config parameter adjust */
struct mipi_cphy_adjust {
	uint32_t need_adjust_cphy_para;
	int cphy_data_t_prepare_adjust;
	int cphy_data_t_lpx_adjust;
	int cphy_data_t_prebegin_adjust;
	int cphy_data_t_post_adjust;
};

struct mipi_panel_info {
	uint8_t dsi_version;
	uint8_t vc;
	uint8_t lane_nums;
	uint8_t lane_nums_select_support;
	uint8_t color_mode;
	uint32_t dsi_bit_clk; /* clock lane(p/n) */
	uint32_t dsi_bit_clk_default;
	uint32_t burst_mode;
	uint32_t max_tx_esc_clk;
	uint8_t non_continue_en;
	uint8_t txoff_rxulps_en;
	int frame_rate;
	int take_effect_delayed_frm_cnt;

	uint32_t hsa;
	uint32_t hbp;
	uint32_t dpi_hsize;
	uint32_t width;
	uint32_t hline_time;

	uint32_t vsa;
	uint32_t vbp;
	uint32_t vactive_line;
	uint32_t vfp;
	uint32_t ignore_hporch;
	uint32_t porch_ratio;
	uint8_t dsi_timing_support;

	uint32_t dsi_bit_clk_val1;
	uint32_t dsi_bit_clk_val2;
	uint32_t dsi_bit_clk_val3;
	uint32_t dsi_bit_clk_val4;
	uint32_t dsi_bit_clk_val5;
	uint32_t dsi_bit_clk_upt;

	uint32_t hs_wr_to_time;

	/* dphy config parameter adjust */
	uint32_t clk_post_adjust;
	uint32_t clk_pre_adjust;
	uint32_t clk_pre_delay_adjust;
	int clk_t_hs_exit_adjust;
	int clk_t_hs_trial_adjust;
	uint32_t clk_t_hs_prepare_adjust;
	int clk_t_lpx_adjust;
	uint32_t clk_t_hs_zero_adjust;
	uint32_t data_post_delay_adjust;
	int data_t_lpx_adjust;
	uint32_t data_t_hs_prepare_adjust;
	uint32_t data_t_hs_zero_adjust;
	int data_t_hs_trial_adjust;
	uint32_t rg_vrefsel_vcm_adjust;
	uint32_t support_de_emphasis;
	uint32_t de_emphasis_reg[DE_EMPHASIS_REG_NUM];
	uint32_t de_emphasis_value[DE_EMPHASIS_REG_NUM];
	uint32_t rg_vrefsel_lptx_adjust;
	uint32_t rg_lptx_sri_adjust;
	int data_lane_lp2hs_time_adjust;

	/* only for 3660 use */
	uint32_t rg_vrefsel_vcm_clk_adjust;
	uint32_t rg_vrefsel_vcm_data_adjust;

	uint32_t phy_mode;  /* 0: DPHY, 1:CPHY */
	uint32_t lp11_flag; /* 0: nomal_lp11, 1:short_lp11, 2:disable_lp11 */
	uint32_t phy_m_n_count_update;  /* 0:old ,1:new can get 988.8M */
	uint32_t eotp_disable_flag; /* 0: eotp enable, 1:eotp disable */

	uint8_t mininum_phy_timing_flag; /* 1:support entering lp11 with minimum clock */

	uint32_t dynamic_dsc_en; /* used for dfr */
	uint32_t dsi_te_type; /* 0: dsi0&te0, 1: dsi1&te0, 2: dsi1&te1 */
	struct mipi_cphy_adjust mipi_cphy_adjust;
};

struct sbl_panel_info {
	uint32_t strength_limit;
	uint32_t calibration_a;
	uint32_t calibration_b;
	uint32_t calibration_c;
	uint32_t calibration_d;
	uint32_t t_filter_control;
	uint32_t backlight_min;
	uint32_t backlight_max;
	uint32_t backlight_scale;
	uint32_t ambient_light_min;
	uint32_t filter_a;
	uint32_t filter_b;
	uint32_t logo_left;
	uint32_t logo_top;
	uint32_t variance_intensity_space;
	uint32_t slope_max;
	uint32_t slope_min;
};

struct dsc_info_mipi {
	uint32_t out_dsc_en;
	uint32_t pic_width;
	uint32_t pic_height;
	uint32_t dual_dsc_en;
	uint32_t dsc_insert_byte_num;

	uint32_t chunk_size;
	uint32_t final_offset;
	uint32_t nfl_bpg_offset;
	uint32_t slice_bpg_offset;
	uint32_t scale_increment_interval;
	uint32_t initial_scale_value;
	uint32_t scale_decrement_interval;
	uint32_t adjustment_bits;
	uint32_t adj_bits_per_grp;
	uint32_t bits_per_grp;
	uint32_t slices_per_line;
	uint32_t pic_line_grp_num;
	uint32_t hrd_delay;
};

/* the same as DDIC */
/* for sensorhub_aod, keep 32-bit aligned */
struct dsc_panel_info {
	/* DSC_CTRL */
	uint32_t dsc_version;
	uint32_t native_422;
	uint32_t bits_per_pixel;
	uint32_t block_pred_enable;
	uint32_t linebuf_depth;
	uint32_t bits_per_component;

	/* DSC_SLICE_SIZE */
	uint32_t slice_width;
	uint32_t slice_height;

	/* DSC_INITIAL_DELAY */
	uint32_t initial_xmit_delay;

	/* DSC_RC_PARAM1 */
	uint32_t first_line_bpg_offset;

	uint32_t mux_word_size;

	/* C_PARAM3 */
	/* uint32_t final_offset; */
	uint32_t initial_offset;

	/* FLATNESS_QP_TH */
	uint32_t flatness_max_qp;
	uint32_t flatness_min_qp;

	/* RC_PARAM4 */
	uint32_t rc_edge_factor;
	uint32_t rc_model_size;

	/* DSC_RC_PARAM5 */
	uint32_t rc_tgt_offset_lo;
	uint32_t rc_tgt_offset_hi;
	uint32_t rc_quant_incr_limit1;
	uint32_t rc_quant_incr_limit0;

	/* DSC_RC_BUF_THRESH0 */
	uint32_t rc_buf_thresh0;
	uint32_t rc_buf_thresh1;
	uint32_t rc_buf_thresh2;
	uint32_t rc_buf_thresh3;

	/* DSC_RC_BUF_THRESH1 */
	uint32_t rc_buf_thresh4;
	uint32_t rc_buf_thresh5;
	uint32_t rc_buf_thresh6;
	uint32_t rc_buf_thresh7;

	/* DSC_RC_BUF_THRESH2 */
	uint32_t rc_buf_thresh8;
	uint32_t rc_buf_thresh9;
	uint32_t rc_buf_thresh10;
	uint32_t rc_buf_thresh11;

	/* DSC_RC_BUF_THRESH3 */
	uint32_t rc_buf_thresh12;
	uint32_t rc_buf_thresh13;

	/* DSC_RC_RANGE_PARAM0 */
	uint32_t range_min_qp0;
	uint32_t range_max_qp0;
	uint32_t range_bpg_offset0;
	uint32_t range_min_qp1;
	uint32_t range_max_qp1;
	uint32_t range_bpg_offset1;

	/* DSC_RC_RANGE_PARAM1 */
	uint32_t range_min_qp2;
	uint32_t range_max_qp2;
	uint32_t range_bpg_offset2;
	uint32_t range_min_qp3;
	uint32_t range_max_qp3;
	uint32_t range_bpg_offset3;

	/* DSC_RC_RANGE_PARAM2 */
	uint32_t range_min_qp4;
	uint32_t range_max_qp4;
	uint32_t range_bpg_offset4;
	uint32_t range_min_qp5;
	uint32_t range_max_qp5;
	uint32_t range_bpg_offset5;

	/* DSC_RC_RANGE_PARAM3 */
	uint32_t range_min_qp6;
	uint32_t range_max_qp6;
	uint32_t range_bpg_offset6;
	uint32_t range_min_qp7;
	uint32_t range_max_qp7;
	uint32_t range_bpg_offset7;

	/* DSC_RC_RANGE_PARAM4 */
	uint32_t range_min_qp8;
	uint32_t range_max_qp8;
	uint32_t range_bpg_offset8;
	uint32_t range_min_qp9;
	uint32_t range_max_qp9;
	uint32_t range_bpg_offset9;

	/* DSC_RC_RANGE_PARAM5 */
	uint32_t range_min_qp10;
	uint32_t range_max_qp10;
	uint32_t range_bpg_offset10;
	uint32_t range_min_qp11;
	uint32_t range_max_qp11;
	uint32_t range_bpg_offset11;

	/* DSC_RC_RANGE_PARAM6 */
	uint32_t range_min_qp12;
	uint32_t range_max_qp12;
	uint32_t range_bpg_offset12;
	uint32_t range_min_qp13;
	uint32_t range_max_qp13;
	uint32_t range_bpg_offset13;

	/* DSC_RC_RANGE_PARAM7 */
	uint32_t range_min_qp14;
	uint32_t range_max_qp14;
	uint32_t range_bpg_offset14;
};

struct frame_rate_ctrl {
	uint8_t registered;
	uint32_t status;
	uint32_t current_hline_time;
	uint32_t current_vfp;
	uint32_t notify_type;

	int target_frame_rate;
	int current_frame_rate;
	uint32_t porch_ratio;
	uint64_t target_dsi_bit_clk;
	uint64_t current_dsi_bit_clk;
	struct mipi_dsi_timing timing;
	struct mipi_dsi_phy_ctrl phy_ctrl;
	uint32_t current_lane_byte_clk;
	uint32_t current_dsc_en;
	uint32_t target_dsc_en;
	uint32_t dbuf_size;
	uint32_t dbuf_hsize;
	uint32_t dmipi_hsize;
	bool ignore_hporch;
};

struct panel_dsc_info {
	enum pixel_format format;
	uint16_t dsc_version;
	uint16_t native_422;
	uint32_t idata_422;
	uint32_t convert_rgb;
	uint32_t adjustment_bits;
	uint32_t adj_bits_per_grp;
	uint32_t bits_per_grp;
	uint32_t slices_per_line;
	uint32_t pic_line_grp_num;
	uint32_t dsc_insert_byte_num;
	uint32_t dual_dsc_en;
	uint32_t dsc_en;
	struct dsc_info dsc_info;
};

#endif /* HISI_FB_PANEL_STRUCT_H */
