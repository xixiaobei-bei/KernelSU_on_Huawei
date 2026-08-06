/*
 * lcd_kit_utils.h
 *
 * lcdkit utils function head file for lcd driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_UTILS_H_
#define __LCD_KIT_UTILS_H_
#include <linux/kernel.h>
#include <linux/ctype.h>
#include "lcd_kit_common.h"
#include "lcd_kit_panel.h"
#include "lcd_kit_sysfs.h"
#include "lcd_kit_adapt.h"

/* macro */
/* default panel */
#define LCD_KIT_DEFAULT_PANEL  "auo_otm1901a_5p2_1080p_video_default"

/* lcd fps scence */
#define LCD_KIT_FPS_SCENCE_IDLE        BIT(0)
#define LCD_KIT_FPS_SCENCE_VIDEO       BIT(1)
#define LCD_KIT_FPS_SCENCE_GAME        BIT(2)
#define LCD_KIT_FPS_SCENCE_WEB         BIT(3)
#define LCD_KIT_FPS_SCENCE_EBOOK       BIT(4)
#define LCD_KIT_FPS_SCENCE_FORCE_30FPS          BIT(5)
#define LCD_KIT_FPS_SCENCE_FUNC_DEFAULT_ENABLE  BIT(6)
#define LCD_KIT_FPS_SCENCE_FUNC_DEFAULT_DISABLE BIT(7)
/* lcd fps value */
#define LCD_KIT_FPS_30 30
#define LCD_KIT_FPS_55 55
#define LCD_KIT_FPS_60 60
#define LCD_KIT_FPS_90 90
#define LCD_KIT_FPS_120 120
#define MAX_BUF        60
#define LCD_REG_LENGTH_MAX 200
#define LCD_DDIC_INFO_LEN      64
/* 2d barcode */
#define BARCODE_LENGTH 46

/* project id */
#define PROJECTID_LEN 10
#define SN_CODE_LENGTH_MAX 54
#define MTK_MODE_STATE_OK 0
#define MTK_MODE_STATE_FAIL 1

/* ddic low voltage detect */
#define DETECT_NUM     4
#define DETECT_LOOPS   6
#define ERR_THRESHOLD  4
#define DET_START      1
#define VAL_NUM        2
#define VAL_0          0
#define VAL_1          1
#define DET1_INDEX     0
#define DET2_INDEX     1
#define DET3_INDEX     2
#define DET4_INDEX     3
#define DMD_DET_ERR_LEN      300
#define ENABLE	        1
#define DISABLE	        0
#define INVALID_INDEX  0xFF

/* pcd errflag detect */
#define PCD_ERRFLAG_SUCCESS       0
#define PCD_FAIL                  1
#define ERRFLAG_FAIL              2
#define LCD_KIT_PCD_SIZE          3
#define LCD_KIT_ERRFLAG_SIZE      8
#define DMD_ERR_INFO_LEN         50
#define LCD_KIT_PCD_DETECT_OPEN   1
#define LCD_KIT_PCD_DETECT_CLOSE  0

struct ldi_panel_info {
	u32 h_back_porch;
	u32 h_front_porch;
	u32 h_pulse_width;
	/*
	 * note: vbp > 8 if used overlay compose,
	 * also lcd vbp > 8 in lcd power on sequence
	 */
	u32 v_back_porch;
	u32 v_front_porch;
	u32 v_pulse_width;
	u8 hsync_plr;
	u8 vsync_plr;
	u8 pixelclk_plr;
	u8 data_en_plr;
	/* for cabc */
	u8 dpi0_overlap_size;
	u8 dpi1_overlap_size;
	u32 v_front_porch_forlp;
};

struct mipi_hopping_info {
	u32 h_back_porch;
	u32 h_front_porch;
	u32 h_pulse_width;
	u32 v_back_porch;
	u32 v_front_porch;
	u32 v_pulse_width;
	u32 vfp_lp_dyn;
	u32 data_rate;
	u32 switch_en;
};

struct display_engine_ddic_rgbw_param {
	int ddic_panel_id;
	int ddic_rgbw_mode;
	int ddic_rgbw_backlight;
	int pixel_gain_limit;
};

struct display_engine_panel_info_param {
	int width;
	int height;
	int maxluminance;
	int minluminance;
	int maxbacklight;
	int minbacklight;
};

struct display_engine {
	u8 ddic_cabc_support;
	u8 ddic_rgbw_support;
};

/* lcd fps set from app */
enum {
	LCD_FPS_APP_SET_60 = 3,
	LCD_FPS_APP_SET_90,
	LCD_FPS_APP_SET_120,
};

/* pcd errflag detect */
enum {
	PCD_COMPARE_MODE_EQUAL = 0,
	PCD_COMPARE_MODE_BIGGER = 1,
	PCD_COMPARE_MODE_MASK = 2,
};

/* lcd fps scence */
enum {
	LCD_FPS_SCENCE_60 = 0,
	LCD_FPS_SCENCE_H60 = 1,
	LCD_FPS_SCENCE_90 = 2,
	LCD_FPS_SCENCE_120 = 3,
	LCD_FPS_SCENCE_MAX = 4,
};

/* lcd fps index */
enum {
	LCD_FPS_60_INDEX = 0,
	LCD_FPS_90_INDEX = 1,
	LCD_FPS_120_INDEX = 2,
};

/* fps dsi mipi parameter index */
enum {
	FPS_HFP_INDEX = 0,
	FPS_HBP_INDEX = 1,
	FPS_HS_INDEX = 2,
	FPS_VFP_INDEX = 3,
	FPS_VBP_INDEX = 4,
	FPS_VS_INDEX = 5,
	FPS_VRE_INDEX = 6,
	FPS_RATE_INDEX = 7,
	FPS_LOWER_INDEX = 8,
	FPS_DA_HS_EXIT = 9,
	FPS_DSI_TIMMING_PARA_NUM = 10,
};

#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
struct panel_round_corner {
	u32 round_corner_en;
	u32 full_content;
	u32 w;
	u32 h;
	u32 w_bot;
	u32 h_bot;
	u32 tp_size;
	u32 bt_size;
	void *lt_addr;
	void *rt_addr;
	void *lb_addr;
	void *rb_addr;
};
#endif

struct mipi_panel_info {
	u8 dsi_version;
	u8 vc;
	u8 lane_nums;
	u8 lane_nums_select_support;
	u8 color_mode;
	u32 dsi_bit_clk; /* clock lane(p/n) */
	u32 burst_mode;
	u32 max_tx_esc_clk;
	u8 non_continue_en;
	u32 dsi_bit_clk_val1;
	u32 dsi_bit_clk_val2;
	u32 dsi_bit_clk_val3;
	u32 dsi_bit_clk_val4;
	u32 dsi_bit_clk_val5;
	u32 dsi_bit_clk_upt;
	u32 hs_wr_to_time;
	/* dphy config parameter adjust */
	u32 mipi_ulpm_after_vci;
	u32 clk_post_adjust;
	u32 clk_pre_adjust;
	u32 clk_pre_delay_adjust;
	u32 clk_t_hs_exit_adjust;
	u32 clk_t_hs_trial_adjust;
	u32 clk_t_hs_prepare_adjust;
	int clk_t_lpx_adjust;
	u32 clk_t_hs_zero_adjust;
	u32 data_post_delay_adjust;
	int data_t_lpx_adjust;
	u32 data_t_hs_prepare_adjust;
	u32 data_t_hs_zero_adjust;
	u32 data_t_hs_trial_adjust;
	u32 rg_vrefsel_vcm_adjust;
	/* only for 3660 use */
	u32 rg_vrefsel_vcm_clk_adjust;
	u32 rg_vrefsel_vcm_data_adjust;
	u32 phy_mode;  /* 0: DPHY, 1:CPHY */
	u32 lp11_flag; /* 0: nomal_lp11, 1:short_lp11, 2:disable_lp11 */
	u32 phy_m_n_count_update; /* 0:old ,1:new can get 988.8M */
	u32 eotp_disable_flag; /* 0: eotp enable, 1:eotp disable */
};

struct mtk_panel_info {
	u32 panel_state;
	u32 panel_lcm_type;
	u32 panel_dsi_mode;
	u32 panel_dsi_switch_mode;
	u32 panel_trans_seq;
	u32 panel_data_padding;
	u32 panel_packtet_size;
	u32 panel_ps;
	u32 panel_density;
	u32 output_mode; /* MTK_PANEL_OUTPUT_MODE */
	u32 lcm_cmd_if; /* lcm_cmd_if is used to distinguish for Panel Cmd single or dual Port */
	u32 type;
	u32 xres;
	u32 yres;
	u32 width; /* mm */
	u32 height;
	u32 bpp;
	u32 fps;
	u32 fps_updt;
	u32 orientation;
	u32 bgr_fmt;
	u32 bl_set_type;
	u32 bl_min;
	u32 bl_max;
	u32 bl_def;
	u32 bl_v200;
	u32 bl_otm;
	u32 bl_default;
	u32 bl_current;
	u32 blpwm_precision_type;
	u32 blpwm_preci_no_convert;
	u32 blpwm_out_div_value;
	u32 blpwm_input_ena;
	u32 blpwm_input_disable;
	u32 blpwm_in_num;
	u32 blpwm_input_precision;
	u32 bl_ic_ctrl_mode;
	unsigned int esd_recovery_bl_support;
	u32 gpio_offset;
	u64 pxl_clk_rate;
	u64 pxl_clk_rate_adjust;
	u32 pxl_clk_rate_div;
	u32 data_rate;
	u32 pxl_fbk_div;
	u32 vsync_ctrl_type;
	u32 fake_external;
	u8  reserved[3];

	u32 ifbc_type;
	u32 ifbc_cmp_dat_rev0;
	u32 ifbc_cmp_dat_rev1;
	u32 ifbc_auto_sel;
	u32 ifbc_orise_ctl;
	u32 ifbc_orise_ctr;
	u32 ssc_disable;
	u32 vrefresh;
	u32 ssc_range;
	u8 sbl_support;
	u8 sre_support;
	u8 color_temperature_support;
	u8 color_temp_rectify_support;
	u32 color_temp_rectify_R;
	u32 color_temp_rectify_G;
	u32 color_temp_rectify_B;
	u8 comform_mode_support;
	u8 cinema_mode_support;
	u8 frc_enable;
	u8 esd_enable;
	u8 esd_skip_mipi_check;
	u8 esd_recover_step;
	u8 esd_expect_value_type;
	u8 dirty_region_updt_support;
	u8 snd_cmd_before_frame_support;
	u8 dsi_bit_clk_upt_support;
	u8 mipiclk_updt_support_new;
	u8 fps_updt_support;
	u8 fps_updt_panel_only;
	u8 fps_updt_force_update;
	u8 fps_scence;

	u8 panel_effect_support;

	u8 gmp_support;
	u8 colormode_support;
	u8 gamma_support;
	u8 gamma_type; /* normal, cinema */
	u8 xcc_support;
	u8 acm_support;
	u8 acm_ce_support;
	u8 rgbw_support;
	u8 hbm_support;

	u8 hiace_support;
	u8 dither_support;
	u8 arsr1p_sharpness_support;
	struct ldi_panel_info ldi;
	struct mipi_panel_info mipi;
	struct mipi_hopping_info mipi_hopping;
	int maxluminance;
	int minluminance;
	/* sn code */
	uint32_t sn_code_length;
	unsigned char sn_code[SN_CODE_LENGTH_MAX];
#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	struct panel_round_corner round_corner;
#endif
};
/* enum */
enum {
	RGBW_SET1_MODE = 1,
	RGBW_SET2_MODE = 2,
	RGBW_SET3_MODE = 3,
	RGBW_SET4_MODE = 4,
};

enum {
	LCD_OFFLINE = 0,
	LCD_ONLINE = 1,
};

struct lcd_kit_gamma {
	u32 support;
	u32 addr;
	u32 length;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_color_coordinate {
	u32 support;
	/* color consistency support */
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_panel_id {
	u32 modulesn;
	u32 equipid;
	u32 modulemanufactdate;
	u32 vendorid;
};

struct lcd_kit_2d_barcode {
	u32 support;
	int number_offset;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_oem_info {
	u32 support;
	/* 2d barcode */
	struct lcd_kit_2d_barcode barcode_2d;
	/* color coordinate */
	struct lcd_kit_color_coordinate col_coordinate;
};

struct lcd_kit_brightness_color_oeminfo {
	u32 support;
	struct lcd_kit_oem_info oem_info;
};

struct lcd_kit_project_id {
	u32 support;
	char id[LCD_DDIC_INFO_LEN];
	char *default_project_id;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_pcd_errflag {
	u32 pcd_support;
	u32 errflag_support;
	u32 pcd_value_compare_mode;
	u32 pcd_errflag_check_support;
	u32 gpio_pcd;
	u32 gpio_errflag;
	u32 exp_pcd_mask;
	u32 pcd_det_num;
	struct lcd_kit_dsi_panel_cmds start_pcd_check_cmds;
	struct lcd_kit_dsi_panel_cmds switch_page_cmds;
	struct lcd_kit_dsi_panel_cmds read_pcd_cmds;
	struct lcd_kit_array_data pcd_value;
	struct lcd_kit_dsi_panel_cmds read_errflag_cmds;
};

struct lcd_kit_fps {
	u32 support;
	u32 fps_switch_support;
	unsigned int default_fps;
	unsigned int current_fps;
	unsigned int hop_support;
	struct lcd_kit_dsi_panel_cmds dfr_enable_cmds;
	struct lcd_kit_dsi_panel_cmds dfr_disable_cmds;
	struct lcd_kit_dsi_panel_cmds fps_to_30_cmds;
	struct lcd_kit_dsi_panel_cmds fps_to_60_cmds;
	struct lcd_kit_array_data low_frame_porch;
	struct lcd_kit_array_data normal_frame_porch;
	struct lcd_kit_array_data panel_support_fps_list;
	struct lcd_kit_dsi_panel_cmds fps_to_cmds[LCD_FPS_SCENCE_MAX];
	struct lcd_kit_array_data fps_dsi_timming[LCD_FPS_SCENCE_MAX];
	struct lcd_kit_array_data hop_info[LCD_FPS_SCENCE_MAX];
};

struct lcd_kit_rgbw {
	u32 support;
	u32 rgbw_bl_max;
	struct lcd_kit_dsi_panel_cmds mode1_cmds;
	struct lcd_kit_dsi_panel_cmds mode2_cmds;
	struct lcd_kit_dsi_panel_cmds mode3_cmds;
	struct lcd_kit_dsi_panel_cmds mode4_cmds;
	struct lcd_kit_dsi_panel_cmds backlight_cmds;
	struct lcd_kit_dsi_panel_cmds saturation_ctrl_cmds;
	struct lcd_kit_dsi_panel_cmds frame_gain_limit_cmds;
	struct lcd_kit_dsi_panel_cmds frame_gain_speed_cmds;
	struct lcd_kit_dsi_panel_cmds color_distor_allowance_cmds;
	struct lcd_kit_dsi_panel_cmds pixel_gain_limit_cmds;
	struct lcd_kit_dsi_panel_cmds pixel_gain_speed_cmds;
	struct lcd_kit_dsi_panel_cmds pwm_gain_cmds;
};

struct lcd_kit_alpm {
	u32 support;
	u32 state;
	u32 doze_delay;
	u32 need_reset;
	struct lcd_kit_dsi_panel_cmds exit_cmds;
	struct lcd_kit_dsi_panel_cmds off_cmds;
	struct lcd_kit_dsi_panel_cmds low_light_cmds;
	struct lcd_kit_dsi_panel_cmds middle_light_cmds;
	struct lcd_kit_dsi_panel_cmds high_light_cmds;
	struct lcd_kit_dsi_panel_cmds double_clock_cmds;
	struct lcd_kit_dsi_panel_cmds single_clock_cmds;
};

struct lcd_kit_snd_disp {
	u32 support;
	struct lcd_kit_dsi_panel_cmds on_cmds;
	struct lcd_kit_dsi_panel_cmds off_cmds;
};

struct lcd_kit_quickly_sleep_out {
	u32 support;
	u32 interval;
	u32 panel_on_tag;
	struct timeval panel_on_record_tv;
};

enum bl_control_mode {
	MTK_AP_MODE = 0,
	I2C_ONLY_MODE = 1,
	PWM_ONLY_MODE,
	MTK_PWM_HIGH_I2C_MODE,
	MUTI_THEN_RAMP_MODE,
	RAMP_THEN_MUTI_MODE,
	MTK_AAL_I2C_MODE,
	MTK_MIPI_MODE,
	MTK_MIPI_BL_IC_PWM_MODE,
	PWM_I2C_MODE,
};

enum bias_control_mode {
	MT_AP_MODE = 0,
	PMIC_ONLY_MODE = 1,
	GPIO_ONLY_MODE,
	GPIO_THEN_I2C_MODE,
};

struct display_engine_ddic_hbm_param {
	int type;      // 0:fp   1:MMI   2:light
	int level;
	bool dimming;  // 0:no dimming  1:dimming
};

struct display_engine_share_memory {
	uint64_t addr_virt;
	uint64_t addr_phy;
};

struct display_engine_share_memory_param {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t interval;
	uint32_t enable;
} ;

struct display_engine_share_memory_status {
	uint32_t frame_index;
	uint32_t buffer_index;
};

enum HBM_CFG_TYPE {
	HBM_FOR_FP = 0,
	HBM_FOR_MMI = 1,
	HBM_FOR_LIGHT = 2
};

struct hbm_type_cfg {
	int source;
	void *dsi;
	void *cb;
	void *handle;
};

/* function declare */
int lcd_kit_read_project_id(void);
int lcd_kit_utils_init(struct device_node *np, struct mtk_panel_info *pinfo);
bool lcd_kit_support(void);
void lcd_kit_disp_on_record_time(void);
int lcd_kit_get_bl_max_nit_from_dts(void);
void lcd_kit_disp_on_check_delay(void);
void lcd_kit_set_bl_cmd(uint32_t level);
int lcd_kit_alpm_setting(uint32_t mode);
int lcd_kit_mipi_set_backlight(struct hbm_type_cfg hbm_source, uint32_t level);
#ifdef LCD_KIT_DEBUG_ENABLE
int dpd_init(struct device_node *np);
int lcd_kit_dbg_init(void);
#endif
struct mtk_panel_info *lcd_kit_get_mkt_panel_info(void);
void lcd_esd_enable(int enable);
void lcd_kit_recovery_display(void);
void lcd_kit_ddic_lv_detect_dmd_report(
	u8 reg_val[DETECT_LOOPS][DETECT_NUM][VAL_NUM]);
int lcd_kit_check_pcd_errflag_check_fac(void);
#ifdef LCD_FACTORY_MODE
int lcd_kit_gpio_pcd_errflag_check(void);
#endif
int lcd_kit_start_pcd_check(void *hld);
int lcd_kit_check_pcd_errflag_check(void *hld);
#endif
