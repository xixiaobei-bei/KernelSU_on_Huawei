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

typedef struct dss_sharpness_bit {
	uint32_t sharp_en;
	uint32_t sharp_mode;

	uint32_t flt0_c0;
	uint32_t flt0_c1;
	uint32_t flt0_c2;

	uint32_t flt1_c0;
	uint32_t flt1_c1;
	uint32_t flt1_c2;

	uint32_t flt2_c0;
	uint32_t flt2_c1;
	uint32_t flt2_c2;

	uint32_t ungain;
	uint32_t ovgain;

	uint32_t lineamt1;
	uint32_t linedeten;
	uint32_t linethd2;
	uint32_t linethd1;

	uint32_t sharpthd1;
	uint32_t sharpthd1mul;
	uint32_t sharpamt1;

	uint32_t edgethd1;
	uint32_t edgethd1mul;
	uint32_t edgeamt1;
} sharp2d_t;

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

struct dpu_panel_info {
	uint32_t type;
	uint32_t product_type;
	uint32_t xres;
	uint32_t yres;
	uint32_t fb_xres;
	uint32_t fb_yres;
	uint32_t width; /* mm */
	uint32_t height;
	uint32_t bpp;
	uint32_t fps;
	uint32_t fps_updt;
	uint32_t orientation;
	uint32_t bgr_fmt;
	uint32_t bl_set_type;
	uint32_t bl_min;
	uint32_t bl_max;
	uint32_t dbv_max;
	uint32_t bl_def;
	uint32_t bl_v200;
	uint32_t bl_otm;
	uint32_t bl_default;
	uint32_t blpwm_precision_type;
	uint32_t blpwm_preci_no_convert;
	uint32_t blpwm_out_div_value;
	uint32_t blpwm_input_ena;
	uint32_t blpwm_input_disable;
	uint32_t blpwm_in_num;
	uint32_t blpwm_input_precision;
	uint32_t need_set_dsi1_te_ctrl;
	uint32_t bl_ic_ctrl_mode;
	uint64_t pxl_clk_rate;
	uint64_t pxl_clk_rate_adjust;
	uint32_t pxl_clk_rate_div;
	uint32_t vsync_ctrl_type;
	uint32_t fake_external;
	uint8_t  reserved[3];
	char *panel_name;
	char lcd_panel_version[LCD_PANEL_VERSION_SIZE];
	uint32_t board_version;
	uint8_t dbv_curve_mapped_support;
	uint8_t is_dbv_need_mapped;
	uint8_t dbv_map_index;
	uint32_t dbv_map_points_num;
	short *dbv_map_curve_pointer;

	uint32_t ifbc_type;
	uint32_t ifbc_cmp_dat_rev0;
	uint32_t ifbc_cmp_dat_rev1;
	uint32_t ifbc_auto_sel;
	uint32_t ifbc_orise_ctl;
	uint32_t ifbc_orise_ctr;
	uint32_t dynamic_dsc_en; /* dynamic variable for dfr */
	uint32_t dynamic_dsc_support; /* static variable for dfr */
	uint32_t dynamic_dsc_ifbc_type; /* static variable for dfr */

	uint8_t lcd_init_step;
	uint8_t lcd_uninit_step;
	uint8_t lcd_uninit_step_support;
	uint8_t lcd_refresh_direction_ctrl;
	uint8_t lcd_adjust_support;

	uint8_t sbl_support;
	uint8_t sre_support;
	uint8_t color_temperature_support;
	uint8_t color_temp_rectify_support;
	uint32_t color_temp_rectify_R;
	uint32_t color_temp_rectify_G;
	uint32_t color_temp_rectify_B;
	uint8_t comform_mode_support;
	uint8_t cinema_mode_support;
	uint8_t frc_enable;
	uint32_t esd_check_time_period;
	uint8_t esd_enable;
	uint8_t esd_skip_mipi_check;
	uint8_t esd_recover_step;
	uint8_t esd_expect_value_type;
	uint8_t esd_timing_ctrl;
	uint32_t esd_recovery_max_count;
	uint32_t esd_check_max_count;
	uint8_t dirty_region_updt_support;
	uint8_t snd_cmd_before_frame_support;
	uint8_t dsi_bit_clk_upt_support;
	uint8_t mipiclk_updt_support_new;
	uint8_t fps_updt_support;
	uint8_t fps_updt_panel_only;
	uint8_t fps_updt_force_update;
	uint8_t fps_scence;
	uint32_t dfr_support_value;
	uint32_t dfr_method;
	uint32_t support_ddr_bw_adjust;

	/* constraint between dfr and other features
	 * BIT(0) - not support mipi clk update in hight frame rate
	 * BIT(1-31) - reserved
	 */
	uint32_t dfr_constraint;
	uint32_t send_dfr_cmd_in_vactive;
	uint32_t support_tiny_porch_ratio; /* support porch ratio less than 10% */
	uint8_t panel_effect_support;
	struct timeval hiace_int_timestamp;

	uint8_t prefix_ce_support; /* rch ce */
	uint8_t prefix_sharpness1D_support; /* rch sharpness 1D */
	uint8_t prefix_sharpness2D_support; /* rch sharpness 2D */
	sharp2d_t *sharp2d_table;
	uint8_t scaling_ratio_threshold;

	uint8_t gmp_support;
	uint8_t colormode_support;
	uint8_t gamma_support;
	uint8_t gamma_type; /* normal, cinema */
	uint8_t xcc_support;
	uint8_t acm_support;
	uint8_t acm_ce_support;
	uint8_t rgbw_support;
	uint8_t hbm_support;
	uint8_t local_hbm_support;

	uint8_t hiace_support;
	uint8_t dither_support;
	uint8_t arsr1p_sharpness_support;

	uint8_t post_scf_support;
	uint8_t default_gmp_off;
	uint8_t smart_color_mode_support;

	uint8_t p3_support;
	uint8_t hdr_flw_support;
	uint8_t post_hihdr_support;

	uint8_t noisereduction_support;

	uint8_t gamma_pre_support;
	uint32_t *gamma_pre_lut_table_R;
	uint32_t *gamma_pre_lut_table_G;
	uint32_t *gamma_pre_lut_table_B;
	uint32_t gamma_pre_lut_table_len;
	uint8_t xcc_pre_support;
	uint32_t *xcc_pre_table;
	uint32_t xcc_pre_table_len;
	uint8_t post_xcc_support;
	uint32_t *post_xcc_table;
	uint32_t post_xcc_table_len;

	int xcc_set_in_isr_support; /* set xcc reg in isr func */

	/* acm, acm lut */
	uint32_t acm_valid_num;
	uint32_t r0_hh;
	uint32_t r0_lh;
	uint32_t r1_hh;
	uint32_t r1_lh;
	uint32_t r2_hh;
	uint32_t r2_lh;
	uint32_t r3_hh;
	uint32_t r3_lh;
	uint32_t r4_hh;
	uint32_t r4_lh;
	uint32_t r5_hh;
	uint32_t r5_lh;
	uint32_t r6_hh;
	uint32_t r6_lh;

	uint32_t hue_param01;
	uint32_t hue_param23;
	uint32_t hue_param45;
	uint32_t hue_param67;
	uint32_t hue_smooth0;
	uint32_t hue_smooth1;
	uint32_t hue_smooth2;
	uint32_t hue_smooth3;
	uint32_t hue_smooth4;
	uint32_t hue_smooth5;
	uint32_t hue_smooth6;
	uint32_t hue_smooth7;
	uint32_t color_choose;
	uint32_t l_cont_en;
	uint32_t lc_param01;
	uint32_t lc_param23;
	uint32_t lc_param45;
	uint32_t lc_param67;
	uint32_t l_adj_ctrl;
	uint32_t capture_ctrl;
	uint32_t capture_in;
	uint32_t capture_out;
	uint32_t ink_ctrl;
	uint32_t ink_out;
	uint32_t cinema_acm_valid_num;
	uint32_t cinema_r0_hh;
	uint32_t cinema_r0_lh;
	uint32_t cinema_r1_hh;
	uint32_t cinema_r1_lh;
	uint32_t cinema_r2_hh;
	uint32_t cinema_r2_lh;
	uint32_t cinema_r3_hh;
	uint32_t cinema_r3_lh;
	uint32_t cinema_r4_hh;
	uint32_t cinema_r4_lh;
	uint32_t cinema_r5_hh;
	uint32_t cinema_r5_lh;
	uint32_t cinema_r6_hh;
	uint32_t cinema_r6_lh;

	uint32_t video_acm_valid_num;
	uint32_t video_r0_hh;
	uint32_t video_r0_lh;
	uint32_t video_r1_hh;
	uint32_t video_r1_lh;
	uint32_t video_r2_hh;
	uint32_t video_r2_lh;
	uint32_t video_r3_hh;
	uint32_t video_r3_lh;
	uint32_t video_r4_hh;
	uint32_t video_r4_lh;
	uint32_t video_r5_hh;
	uint32_t video_r5_lh;
	uint32_t video_r6_hh;
	uint32_t video_r6_lh;

	uint32_t *acm_lut_hue_table;
	uint32_t acm_lut_hue_table_len;
	uint32_t *acm_lut_value_table;
	uint32_t acm_lut_value_table_len;
	uint32_t *acm_lut_sata_table;
	uint32_t acm_lut_sata_table_len;
	uint32_t *acm_lut_satr_table;
	uint32_t acm_lut_satr_table_len;

	uint32_t *cinema_acm_lut_hue_table;
	uint32_t cinema_acm_lut_hue_table_len;
	uint32_t *cinema_acm_lut_value_table;
	uint32_t cinema_acm_lut_value_table_len;
	uint32_t *cinema_acm_lut_sata_table;
	uint32_t cinema_acm_lut_sata_table_len;
	uint32_t *cinema_acm_lut_satr_table;
	uint32_t cinema_acm_lut_satr_table_len;

	uint32_t *video_acm_lut_hue_table;
	uint32_t *video_acm_lut_value_table;
	uint32_t *video_acm_lut_sata_table;
	uint32_t *video_acm_lut_satr_table;

	/* acm */
	uint32_t *acm_lut_satr0_table;
	uint32_t acm_lut_satr0_table_len;
	uint32_t *acm_lut_satr1_table;
	uint32_t acm_lut_satr1_table_len;
	uint32_t *acm_lut_satr2_table;
	uint32_t acm_lut_satr2_table_len;
	uint32_t *acm_lut_satr3_table;
	uint32_t acm_lut_satr3_table_len;
	uint32_t *acm_lut_satr4_table;
	uint32_t acm_lut_satr4_table_len;
	uint32_t *acm_lut_satr5_table;
	uint32_t acm_lut_satr5_table_len;
	uint32_t *acm_lut_satr6_table;
	uint32_t acm_lut_satr6_table_len;
	uint32_t *acm_lut_satr7_table;
	uint32_t acm_lut_satr7_table_len;

	uint32_t *cinema_acm_lut_satr0_table;
	uint32_t *cinema_acm_lut_satr1_table;
	uint32_t *cinema_acm_lut_satr2_table;
	uint32_t *cinema_acm_lut_satr3_table;
	uint32_t *cinema_acm_lut_satr4_table;
	uint32_t *cinema_acm_lut_satr5_table;
	uint32_t *cinema_acm_lut_satr6_table;
	uint32_t *cinema_acm_lut_satr7_table;

	uint32_t *video_acm_lut_satr0_table;
	uint32_t *video_acm_lut_satr1_table;
	uint32_t *video_acm_lut_satr2_table;
	uint32_t *video_acm_lut_satr3_table;
	uint32_t *video_acm_lut_satr4_table;
	uint32_t *video_acm_lut_satr5_table;
	uint32_t *video_acm_lut_satr6_table;
	uint32_t *video_acm_lut_satr7_table;

	/* acm */
	uint32_t *acm_lut_satr_face_table;
	uint32_t *acm_lut_lta_table;
	uint32_t *acm_lut_ltr0_table;
	uint32_t *acm_lut_ltr1_table;
	uint32_t *acm_lut_ltr2_table;
	uint32_t *acm_lut_ltr3_table;
	uint32_t *acm_lut_ltr4_table;
	uint32_t *acm_lut_ltr5_table;
	uint32_t *acm_lut_ltr6_table;
	uint32_t *acm_lut_ltr7_table;
	uint32_t *acm_lut_lh0_table;
	uint32_t *acm_lut_lh1_table;
	uint32_t *acm_lut_lh2_table;
	uint32_t *acm_lut_lh3_table;
	uint32_t *acm_lut_lh4_table;
	uint32_t *acm_lut_lh5_table;
	uint32_t *acm_lut_lh6_table;
	uint32_t *acm_lut_lh7_table;
	uint32_t *acm_lut_ch0_table;
	uint32_t *acm_lut_ch1_table;
	uint32_t *acm_lut_ch2_table;
	uint32_t *acm_lut_ch3_table;
	uint32_t *acm_lut_ch4_table;
	uint32_t *acm_lut_ch5_table;
	uint32_t *acm_lut_ch6_table;
	uint32_t *acm_lut_ch7_table;
	uint32_t acm_lut_satr_face_table_len;
	uint32_t acm_lut_lta_table_len;
	uint32_t acm_lut_ltr0_table_len;
	uint32_t acm_lut_ltr1_table_len;
	uint32_t acm_lut_ltr2_table_len;
	uint32_t acm_lut_ltr3_table_len;
	uint32_t acm_lut_ltr4_table_len;
	uint32_t acm_lut_ltr5_table_len;
	uint32_t acm_lut_ltr6_table_len;
	uint32_t acm_lut_ltr7_table_len;
	uint32_t acm_lut_lh0_table_len;
	uint32_t acm_lut_lh1_table_len;
	uint32_t acm_lut_lh2_table_len;
	uint32_t acm_lut_lh3_table_len;
	uint32_t acm_lut_lh4_table_len;
	uint32_t acm_lut_lh5_table_len;
	uint32_t acm_lut_lh6_table_len;
	uint32_t acm_lut_lh7_table_len;
	uint32_t acm_lut_ch0_table_len;
	uint32_t acm_lut_ch1_table_len;
	uint32_t acm_lut_ch2_table_len;
	uint32_t acm_lut_ch3_table_len;
	uint32_t acm_lut_ch4_table_len;
	uint32_t acm_lut_ch5_table_len;
	uint32_t acm_lut_ch6_table_len;
	uint32_t acm_lut_ch7_table_len;

	/* gamma, igm, gmp, xcc */
	uint32_t *gamma_lut_table_R;
	uint32_t *gamma_lut_table_G;
	uint32_t *gamma_lut_table_B;
	uint32_t gamma_lut_table_len;
	uint32_t *cinema_gamma_lut_table_R;
	uint32_t *cinema_gamma_lut_table_G;
	uint32_t *cinema_gamma_lut_table_B;
	uint32_t cinema_gamma_lut_table_len;
	uint32_t *igm_lut_table_R;
	uint32_t *igm_lut_table_G;
	uint32_t *igm_lut_table_B;
	uint32_t igm_lut_table_len;
	uint32_t *gmp_lut_table_low32bit;
	uint32_t *gmp_lut_table_high4bit;
	uint32_t gmp_lut_table_len;
	uint32_t *xcc_table;
	uint32_t xcc_table_len;

	/* arsr1p lut */
	uint32_t *pgainlsc0;
	uint32_t *pgainlsc1;
	uint32_t pgainlsc_len;
	uint32_t *hcoeff0y;
	uint32_t *hcoeff1y;
	uint32_t *hcoeff2y;
	uint32_t *hcoeff3y;
	uint32_t *hcoeff4y;
	uint32_t *hcoeff5y;
	uint32_t hcoeffy_len;
	uint32_t *vcoeff0y;
	uint32_t *vcoeff1y;
	uint32_t *vcoeff2y;
	uint32_t *vcoeff3y;
	uint32_t *vcoeff4y;
	uint32_t *vcoeff5y;
	uint32_t vcoeffy_len;
	uint32_t *hcoeff0uv;
	uint32_t *hcoeff1uv;
	uint32_t *hcoeff2uv;
	uint32_t *hcoeff3uv;
	uint32_t hcoeffuv_len;
	uint32_t *vcoeff0uv;
	uint32_t *vcoeff1uv;
	uint32_t *vcoeff2uv;
	uint32_t *vcoeff3uv;
	uint32_t vcoeffuv_len;

	uint8_t non_check_ldi_porch;
	uint8_t hisync_mode;
	uint8_t vsync_delay_time;
	uint8_t video_idle_mode;

	/* dpi_set */
	uint8_t dpi01_exchange_flag;

	uint8_t panel_mode_swtich_support;
	uint8_t current_mode;
	uint8_t mode_switch_to;
	uint8_t mode_switch_state;
	uint8_t ic_dim_ctrl_enable;
	uint32_t fps_lock_command_support;
	uint64_t left_time_to_te_us;
	uint64_t right_time_to_te_us;
	uint64_t te_interval_us;
	uint32_t bl_delay_frame;
	int need_skip_delta;
	/* elle need delay 3frames between bl and hbm code */
	uint32_t hbm_entry_delay;
	/* elle the time stamp for bl Code */
	ktime_t hbm_blcode_ts;

	uint32_t elvss_dim_val;

	struct mask_delay_time mask_delay;
	struct spi_device *spi_dev;
	struct ldi_panel_info ldi;
	struct ldi_panel_info ldi_updt;
	struct ldi_panel_info ldi_lfps;
	struct mipi_panel_info mipi;
	struct mipi_panel_info mipi_updt;
	struct sbl_panel_info smart_bl;
	struct dsc_panel_info vesa_dsc;
	struct panel_dsc_info panel_dsc_info;
	struct lcd_dirty_region_info dirty_region_info;

	struct mipi_dsi_phy_ctrl dsi_phy_ctrl;
	uint32_t dummy_pixel_num;
	uint32_t dummy_pixel_x_left;
	uint8_t spr_dsc_mode;
	struct spr_dsc_panel_para spr;

	/* Contrast Alogrithm */
	struct hiace_alg_parameter hiace_param;
	struct ce_algorithm_parameter ce_alg_param;
	struct frame_rate_ctrl frm_rate_ctrl;
	uint32_t dfr_delay_time;  /* us */

	uint32_t need_two_panel_display;
	int skip_power_on_off;
	int disp_panel_id;  /* 0: inner panel; 1: outer panel */

	/* for cascade ic, set the correct display area for saving power */
	uint8_t cascadeic_support;
	uint8_t current_display_region;

	/* aging offset support */
	uint32_t single_deg_support;

	/* min hbm dbv level */
	uint32_t min_hbm_dbv;

	/* for mipi dsi tx interface, support delayed cmd queue which will send after next frame start(vsync) */
	uint8_t delayed_cmd_queue_support;

	/* idle timeout trigger frame refresh if emi protect enable */
	uint8_t emi_protect_enable;

	/* support delay set backlight threshold */
	uint8_t delay_set_bl_thr_support;

	/* delay set backlight threshold */
	uint8_t delay_set_bl_thr;

	/* sn code */
	uint32_t sn_code_length;
	unsigned char sn_code[SN_CODE_LENGTH_MAX];

	uint8_t mipi_no_round;
	/* aod */
	uint8_t ramless_aod;
	/* set 1, means update core clk to L2 */
	uint32_t update_core_clk_support;
	/* aod conflict with esd */
	uint8_t aod_esd_flag;

	uint8_t split_support;

	/* set 50, mean 50% */
	uint8_t split_logical1_ratio;

	/* poweric detect status */
	uint32_t poweric_num_length;
	uint32_t poweric_status[POWERIC_NUM_MAX];
	int delta_bl_delayed;
	bool blc_enable_delayed;
	bool dc_switch_xcc_updated;
	uint32_t ddic_alpha_delayed;
	int last_alpha;
	uint32_t need_adjust_dsi_vcm;
};

struct dpu_fb_data_type;

typedef struct demura_set_info {
	uint8_t *data0;
	uint8_t len0;
	uint8_t *data1;
	uint8_t len1;
} demura_set_info_t;

struct dpu_fb_panel_data {
	struct dpu_panel_info *panel_info;

	/* function entry chain */
	int (*set_fastboot)(struct platform_device *pdev);
	int (*on)(struct platform_device *pdev);
	int (*off)(struct platform_device *pdev);
	int (*lp_ctrl)(struct platform_device *pdev, bool lp_enter);
	int (*remove)(struct platform_device *pdev);
	int (*set_backlight)(struct platform_device *pdev, uint32_t bl_level);
	int (*lcd_set_backlight_by_type_func)(struct platform_device *pdev, int backlight_type);
	int (*lcd_set_hbm_for_screenon)(struct platform_device *pdev, int bl_type);
	int (*lcd_set_hbm_for_mmi_func)(struct platform_device *pdev, int level);
	int (*set_blc_brightness)(struct platform_device *pdev, uint32_t bl_level);
	int (*vsync_ctrl)(struct platform_device *pdev, int enable);
	int (*lcd_fps_scence_handle)(struct platform_device *pdev, uint32_t scence);
	int (*lcd_fps_updt_handle)(struct platform_device *pdev);
	void (*snd_cmd_before_frame)(struct platform_device *pdev);
	int (*esd_handle)(struct platform_device *pdev);
	int (*set_display_region)(struct platform_device *pdev, struct dss_rect *dirty);
	int (*set_pixclk_rate)(struct platform_device *pdev);
	int (*set_display_resolution)(struct platform_device *pdev);
	int (*get_lcd_id)(struct platform_device *pdev);
	int (*panel_bypass_powerdown_ulps_support)(struct platform_device *pdev);
	int (*set_tcon_mode)(struct platform_device *pdev, uint8_t mode);
	void (*get_spr_para_list)(uint32_t *gma, uint32_t *degma);
	int (*panel_switch)(struct platform_device *pdev, uint32_t fold_status);
	struct dpu_panel_info *(*get_panel_info)(struct platform_device *pdev, uint32_t panel_id);

	ssize_t (*snd_mipi_clk_cmd_store)(struct platform_device *pdev, uint32_t clk_val);
	ssize_t (*lcd_model_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_cabc_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_cabc_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_rgbw_set_func)(struct dpu_fb_data_type *dpufd);
	ssize_t (*lcd_hbm_set_func)(struct dpu_fb_data_type *dpufd);
	ssize_t (*lcd_set_ic_dim_on)(struct dpu_fb_data_type *dpufd);
	ssize_t (*lcd_color_param_get_func)(struct dpu_fb_data_type *dpufd);
	ssize_t (*lcd_ce_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_ce_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_check_reg)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_mipi_detect)(struct platform_device *pdev, char *buf);
	ssize_t (*mipi_dsi_bit_clk_upt_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*mipi_dsi_bit_clk_upt_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_hkadc_debug_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_hkadc_debug_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_gram_check_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_gram_check_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_dynamic_sram_checksum_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_dynamic_sram_checksum_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_color_temperature_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_color_temperature_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_ic_color_enhancement_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_ic_color_enhancement_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*led_rg_lcd_color_temperature_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*led_rg_lcd_color_temperature_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_comform_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_comform_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_cinema_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_cinema_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_support_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_support_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_voltage_enable_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_bist_check)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_sleep_ctrl_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_sleep_ctrl_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_test_config_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_test_config_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_reg_read_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_reg_read_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_support_checkmode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_lp2hs_mipi_check_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_lp2hs_mipi_check_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_inversion_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_inversion_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_scan_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_scan_show)(struct platform_device *pdev, char *buf);
	ssize_t (*amoled_pcd_errflag_check)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_hbm_ctrl_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_hbm_ctrl_show)(struct platform_device *pdev, char *buf);
	ssize_t (*sharpness2d_table_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*sharpness2d_table_show)(struct platform_device *pdev, char *buf);
	ssize_t (*panel_info_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_acm_state_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_acm_state_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_acl_ctrl_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_acl_ctrl_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_gmp_state_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_gmp_state_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_amoled_vr_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_amoled_vr_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*amoled_alpm_setting_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_xcc_store)(struct platform_device *pdev, const char *buf, size_t count);
	int (*lcd_get_demura)(struct platform_device *pdev, uint8_t dsi,
		 uint8_t *out, int out_len,  uint8_t type, uint8_t len);
	int (*lcd_set_demura)(struct platform_device *pdev, uint8_t type, const demura_set_info_t *info);
	struct platform_device *next;
};

#if defined(CONFIG_HUAWEI_DSM)
struct lcd_reg_read_t {
	u8 reg_addr; /* register address */
	u32 expected_value; /* the expected value should returned when lcd is in good condtion */
	u32 read_mask; /* set read mask if there are bits should ignored */
	char *reg_name; /* register name */
	bool for_debug; /* for debug */
	u8 recovery; /* if need recovery */
};
#endif

struct panel_aging_time_info {
	s64 start_time[EN_AGING_PANEL_NUM];
	s64 duration_time[EN_AGING_PANEL_NUM];
	uint32_t time_state[EN_AGING_PANEL_NUM];
	spinlock_t time_lock;
	bool hiace_enable;

	uint32_t fold_count;
	spinlock_t count_lock;
	uint8_t record_region;
};

struct display_engine_duration {
	uint32_t *dbv_acc;
	uint32_t *original_dbv_acc;
	uint32_t *screen_on_duration;
	uint32_t *hbm_acc;
	uint32_t *hbm_duration;
	uint32_t size_dbv_acc;
	uint32_t size_original_dbv_acc;
	uint32_t size_screen_on_duration;
	uint32_t size_hbm_acc;
	uint32_t size_hbm_duration;
};

#endif /* HISI_FB_PANEL_STRUCT_H */
