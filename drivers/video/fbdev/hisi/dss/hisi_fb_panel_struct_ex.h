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
#ifndef HISI_FB_PANEL_STRUCT_EX_H
#define HISI_FB_PANEL_STRUCT_EX_H

#include "hisi_fb_panel_enum.h"
#include "hisi_fb_panel_struct.h"

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
	uint32_t display_on_before_backlight;
	uint32_t before_bl_on_mdelay;

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
	uint32_t esd_first_check_delay;
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
	uint32_t bypass_dvfs;

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
	uint8_t dcdelay_support;
	uint64_t dc_time_to_te_us;
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

	/* power off delay before LP00 */
	uint32_t delay_bf_lp00;

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

#endif /* HISI_FB_PANEL_STRUCT_EX_H */
