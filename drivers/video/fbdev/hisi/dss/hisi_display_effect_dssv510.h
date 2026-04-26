/*
 * Copyright (c) 2012-2018, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef EFFECT_DSSV510_H
#define EFFECT_DSSV510_H

#include <linux/types.h>

// for 32/64 bit compatiable
#define compat_pointer(x) \
	union { \
		uint32_t *x; \
		uint64_t x##_sizer; \
	}

#define BYTES_PER_TABLE_ELEMENT 4

#define XCC_COEF_LEN         12
#define XCC_COEF_INDEX_01    1
#define XCC_COEF_INDEX_12    6
#define XCC_COEF_INDEX_23    11
#define XCC_DEFAULT          0x8000
#define GMP_BLOCK_SIZE       137
#define GMP_CNT_NUM          18

#define LCP_GMP_LUT_LENGTH  ((uint32_t)17 * 17 * 17)
#define LCP_GMP_LUT_CHECK_BLACK 0
#define LCP_GMP_LUT_CHECK_BLUE 16
#define LCP_GMP_LUT_CHECK_GREEN 272
#define LCP_GMP_LUT_CHECK_RED 4624
#define LCP_GMP_LUT_CHECK_WHITE 4912

#define LCP_IGM_LUT_CHECK_LEN 190
#define LCP_IGM_LUT_CHECK_START 9
#define LCP_IGM_LUT_CHECK_STEP 30

#define LCP_XCC_LUT_LENGTH  ((uint32_t)12)
#define LCP_IGM_LUT_LENGTH  ((uint32_t)257)

#define GAMA_LUT_LENGTH  ((uint32_t)257)
#define GAMA_LUT_CHECK_0 2
#define GAMA_LUT_CHECK_1 4
#define GAMA_LUT_CHECK_2 8
#define GAMA_LUT_CHECK_3 16
#define GAMA_LUT_CHECK_4 32
#define GAMA_LUT_CHECK_5 64
#define GAMA_LUT_CHECK_6 128
#define GAMA_LUT_CHECK_7 255

#define POST_XCC_LUT_LENGTH ((uint32_t)12)

#define HIACE_DETAIL_WEIGHT_TABLE_LEN 33
#define HIACE_LOGLUM_EOTF_TABLE_LEN 63
#define HIACE_LUMA_GAMA_TABLE_LEN 63

#define TAG_ARSR_1P_ENABLE  100
#define TAG_LCP_GMP_ENABLE  300
#define TAG_LCP_XCC0_ENABLE  302
#define TAG_LCP_XCC1_ENABLE  303
#define TAG_LCP_IGM_ENABLE  304
#define TAG_GAMMA_ENABLE    400
#define TAG_HIACE_LHIST_SFT 503

// dpp save buffer status
#define DPP_BUF_INVALIED        4
#define DPP_BUF_COPIED_ONCE     3
#define DPP_BUF_FROM_ANOTHER    2
#define DPP_BUF_NEW_UPDATED     1
#define DPP_BUF_INIT_UPDATE     0x8000

// dpp config buffer status
#define DPP_BUF_READY_FOR_BOTH   3 // 11
#define DPP_BUF_READY_FOR_CH0    1 // 01
#define DPP_BUF_READY_FOR_CH1    2 // 10
#define DPP_BUF_READY_FOR_NONE   0 // 00

// dpp buffer manger status
#define DPP_BUF_ONLINE_CONFIG    1
#define DPP_BUF_WORKQ_CONFIG     2

#define DPP_BUF_ROI_REGION_COUNT 2
#define DPP_CHN_MAX_COUNT        2

#define DPP_CHN_GMP_NEED_CONFIG     (1 << 0)
#define DPP_CHN_GAMA_NEED_CONFIG    (1 << 1)
#define DPP_CHN_XCC_NEED_CONFIG     (1 << 2)
#define DPP_CHN_DEGAMMA_NEED_CONFIG (1 << 3)

#define DPP_CHN_UPDATE_READY 3
#define DPP_CHN_CONFIG_READY 2
#define DPP_CHN_CONFIG_DOING 1
#define DPP_CHN_NEED_CONFIG  0

#define ARSR1P_ROG_FHD_FLAG BIT(0)
#define ARSR1P_ROG_HD_FLAG BIT(1)
#define ARSR1P_INFO_SIZE 3
#define ARSR2P_MAX_NUM 3

#define GMP_ENABLE BIT(0)
#define GMP_PANEL_OFF_SCENE BIT(0)
#define GMP_LAST_PARAM_SCENE BIT(1)

#define HIGH16(val) (val << 16)
#define LOW16(val)  (val)

// acm algorithm info define, mem_ctrl decide by enable or not
struct acm_info {
	uint32_t acm_en;
	uint32_t param_mode;
	uint32_t sata_offset;
	uint32_t acm_hue_rlh01;
	uint32_t acm_hue_rlh23;
	uint32_t acm_hue_rlh45;
	uint32_t acm_hue_rlh67;
	uint32_t acm_hue_param01;
	uint32_t acm_hue_param23;
	uint32_t acm_hue_param45;
	uint32_t acm_hue_param67;
	uint32_t acm_hue_smooth0;
	uint32_t acm_hue_smooth1;
	uint32_t acm_hue_smooth2;
	uint32_t acm_hue_smooth3;
	uint32_t acm_hue_smooth4;
	uint32_t acm_hue_smooth5;
	uint32_t acm_hue_smooth6;
	uint32_t acm_hue_smooth7;
	uint32_t acm_color_choose;
	uint32_t acm_l_cont_en;
	uint32_t acm_lc_param01;
	uint32_t acm_lc_param23;
	uint32_t acm_lc_param45;
	uint32_t acm_lc_param67;
	uint32_t acm_l_adj_ctrl;
	uint32_t acm_capture_ctrl;
	uint32_t acm_capture_in;
	uint32_t acm_capture_out;
	uint32_t acm_ink_ctrl;
	uint32_t acm_ink_out;

	compat_pointer(hue_table);
	compat_pointer(sata_table);
	compat_pointer(satr0_table);
	compat_pointer(satr1_table);
	compat_pointer(satr2_table);
	compat_pointer(satr3_table);
	compat_pointer(satr4_table);
	compat_pointer(satr5_table);
	compat_pointer(satr6_table);
	compat_pointer(satr7_table);

};

struct arsr1p_info {
	uint32_t enable;
	uint32_t para_mode;	// For ARSR POST
	// enable switch
	uint32_t sharp_enable;
	uint32_t skin_enable;
	uint32_t shoot_enable;

	// registers
	uint32_t skin_thres_y;
	uint32_t skin_thres_u;
	uint32_t skin_thres_v;
	uint32_t skin_cfg0;
	uint32_t skin_cfg1;
	uint32_t skin_cfg2;
	uint32_t shoot_cfg1;
	uint32_t shoot_cfg2;
	uint32_t shoot_cfg3;
	uint32_t sharp_cfg3;
	uint32_t sharp_cfg4;
	uint32_t sharp_cfg5;
	uint32_t sharp_cfg6;
	uint32_t sharp_cfg7;
	uint32_t sharp_cfg8;
	uint32_t sharp_level;
	uint32_t sharp_gain_low;
	uint32_t sharp_gain_mid;
	uint32_t sharp_gain_high;
	uint32_t sharp_gainctrl_sloph_mf;
	uint32_t sharp_gainctrl_slopl_mf;
	uint32_t sharp_gainctrl_sloph_hf;
	uint32_t sharp_gainctrl_slopl_hf;
	uint32_t sharp_mf_lmt;
	uint32_t sharp_gain_mf;
	uint32_t sharp_mf_b;
	uint32_t sharp_hf_lmt;
	uint32_t sharp_gain_hf;
	uint32_t sharp_hf_b;
	uint32_t sharp_lf_ctrl;
	uint32_t sharp_lf_var;
	uint32_t sharp_lf_ctrl_slop;
	uint32_t sharp_hf_select;
	uint32_t sharp_cfg2_h;
	uint32_t sharp_cfg2_l;
	uint32_t texture_analysis;
	uint32_t intplshootctrl;

	uint32_t update;
};

struct arsr2p_info {
	uint32_t enable;
	// enable switch
	uint32_t sharp_enable;
	uint32_t skin_enable;
	uint32_t shoot_enable;

	// registers
	uint32_t skin_thres_y;
	uint32_t skin_thres_u;
	uint32_t skin_thres_v;
	uint32_t skin_cfg0;
	uint32_t skin_cfg1;
	uint32_t skin_cfg2;
	uint32_t shoot_cfg1;
	uint32_t shoot_cfg2;
	uint32_t shoot_cfg3;
	uint32_t sharp_cfg3;
	uint32_t sharp_cfg4;
	uint32_t sharp_cfg6;
	uint32_t sharp_cfg7;
	uint32_t sharp_cfg8;
	uint32_t sharp_level;
	uint32_t sharp_gain_low;
	uint32_t sharp_gain_mid;
	uint32_t sharp_gain_high;
	uint32_t sharp_gainctrl_sloph_mf;
	uint32_t sharp_gainctrl_slopl_mf;
	uint32_t sharp_gainctrl_sloph_hf;
	uint32_t sharp_gainctrl_slopl_hf;
	uint32_t sharp_mf_lmt;
	uint32_t sharp_gain_mf;
	uint32_t sharp_mf_b;
	uint32_t sharp_hf_lmt;
	uint32_t sharp_gain_hf;
	uint32_t sharp_hf_b;
	uint32_t sharp_lf_ctrl;
	uint32_t sharp_lf_var;
	uint32_t sharp_lf_ctrl_slop;
	uint32_t sharp_hf_select;
	uint32_t sharp_cfg2_h;
	uint32_t sharp_cfg2_l;
	uint32_t texture_analysis;
	uint32_t intplshootctrl;

	uint32_t update;
};

enum ROI_EFFECT_AREA {
	ROI_EFFECT_AREA_OUT_RECT = 0,
	ROI_EFFECT_AREA_IN_RECT  = 1,
	ROI_EFFECT_AREA_BOTH     = 2,
};

enum GAMMA_PARA_MODE {
	GAMMA_PARA_MODE_NORMAL = 0,
	GAMMA_PARA_MODE_CINEMA = 1,
	GAMMA_PARA_MODE_USER   = 2,
};

enum DPP_ROI_PARAM_TYPE {
	DPP_ROI_PARAM_VALID   = 0,
	DPP_ROI_PARAM_INVALID = 1,
};

enum DPP_EFFECT_UPDATE_STATE {
	DPP_EFFECT_UPDATE_SUCCESS = 0,
	DPP_EFFECT_UPDATE_NONE = 1,
};

enum DPP_BUF_IDX {
	DPP_GENERAL_BUF_0   = 0,
	DPP_GENERAL_BUF_1   = 1,
	DPP_EXTENAL_ROI_BUF = 2,
	DPP_BUF_MAX_COUNT,
};

enum DPP_BUF_SET_RESULT {
	DPP_BUF_SET_SUCCESS = 0,
	DPP_BUF_SET_NONE    = 1,
};

/* two roi rect config for dpp roi mode
 * top_left > bottom_right,thisi roi rect configuration does not take effect
 * only one region takes effect, another area must be set to
 * top_left_x = 0
 * top_left_y = 0
 * bottom_right_x = 0xF
 * bottom_right_y = 0xF
 */
struct roi_rect {
	uint32_t enable;
	uint32_t top_left_x;
	uint32_t top_left_y;
	uint32_t bottom_right_x;
	uint32_t bottom_right_y;
};

struct effect_roi {
    /* 1: take effect in roi, 0: take effect out roi */
	uint32_t gmp_area;
	uint32_t xcc0_area;
	uint32_t xcc1_area;
	uint32_t igm_area;
};

/*xcc0 and xcc1 can work simultaneously in one frame*/
struct lcp_info {
	uint32_t gmp_mode;
	uint32_t xcc_enable;
	uint32_t xcc0_enable;
	uint32_t xcc1_enable;
	uint32_t igm_enable;
	uint32_t roi_region_num;
	uint32_t post_xcc_enable;

	struct effect_roi effect_area;

	compat_pointer(gmp_table_low32); // gmp lut length is 17*17*17
	compat_pointer(gmp_table_high4);
	compat_pointer(igm_r_table); // igm lut length is 257, the same with gamma
	compat_pointer(igm_g_table);
	compat_pointer(igm_b_table);
	compat_pointer(xcc_table);
	compat_pointer(xcc0_table);
	compat_pointer(xcc1_table);
	compat_pointer(dpp_roi_region);
	compat_pointer(post_xcc_table);
};

/* gama_info is for kernel; gamma_info is for hal sdk*/
struct gamma_info {
	uint32_t enable;
	uint32_t roi_enable;
	uint32_t gama_effect_roi;
	uint32_t roi_region_num;
	uint32_t para_mode;

	compat_pointer(gamma_r_table); // gamma lut length 257(even+odd)
	compat_pointer(gamma_g_table);
	compat_pointer(gamma_b_table);
	compat_pointer(gamma_roi_r_table);
	compat_pointer(gamma_roi_g_table);
	compat_pointer(gamma_roi_b_table);
};

// hiace v3
struct hiace_v3_register {
	// highlight
	uint32_t highlight;
	uint32_t hl_hue_gain0;
	uint32_t hl_hue_gain1;
	uint32_t hl_hue_gain2;
	uint32_t hl_hue_gain3;
	uint32_t hl_hue_gain4;
	uint32_t hl_hue_gain5;
	uint32_t hl_sat;
	uint32_t hl_yout_maxth;
	uint32_t hl_yout;
	uint32_t hl_diff;

	// skin
	uint32_t skin_count;

	// sat
	uint32_t lre_sat;

	// local refresh
	uint32_t local_refresh_h;
	uint32_t local_refresh_v;
	uint32_t global_hist_en;
	uint32_t global_hist_start;
	uint32_t global_hist_size;

	// separator screen
	uint32_t screen_mode;
	uint32_t separator;
	uint32_t weight_min_max;

	// local_hist, fna protect
	uint32_t fna_en;
	uint32_t dbg_hiace;
};

struct hiace_info {
	int disp_panel_id;
	uint32_t image_info;
	uint32_t half_block_info;
	uint32_t xyweight;
	uint32_t lhist_sft;
	uint32_t roi_start_point;
	uint32_t roi_width_high;
	uint32_t roi_mode_ctrl;
	uint32_t roi_hist_stat_mode;
	uint32_t hue;
	uint32_t saturation;
	uint32_t value;
	uint32_t skin_gain;
	uint32_t up_low_th;
	uint32_t rgb_blend_weight;
	uint32_t fna_statistic;
	uint32_t up_cnt;
	uint32_t low_cnt;
	uint32_t sum_saturation;
	uint32_t lhist_en;
	uint32_t gamma_w;
	uint32_t gamma_r;
	uint32_t fna_addr;
	uint32_t fna_data;
	uint32_t update_fna;
	uint32_t fna_valid;
	uint32_t db_pipe_cfg;
	uint32_t db_pipe_ext_width;
	uint32_t db_pipe_full_img_width;
	uint32_t bypass_nr;
	uint32_t s3_some_brightness01;
	uint32_t s3_some_brightness23;
	uint32_t s3_some_brightness4;
	uint32_t s3_min_max_sigma;
	uint32_t s3_green_sigma03;
	uint32_t s3_green_sigma45;
	uint32_t s3_red_sigma03;
	uint32_t s3_red_sigma45;
	uint32_t s3_blue_sigma03;
	uint32_t s3_blue_sigma45;
	uint32_t s3_white_sigma03;
	uint32_t s3_white_sigma45;
	uint32_t s3_filter_level;
	uint32_t s3_similarity_coeff;
	uint32_t s3_v_filter_weight_adj;
	uint32_t s3_hue;
	uint32_t s3_saturation;
	uint32_t s3_value;
	uint32_t s3_skin_gain;
	uint32_t param_update;
	uint32_t enable;
	uint32_t enable_update; // 1 valid; 0 invalid
	uint32_t lut_update; // 1 valid; 0 invalid
	uint32_t thminv;
	unsigned int *lut;

	uint32_t hdr10_en;
	uint32_t loglum_max;
	uint32_t hist_modev;
	uint32_t end_point;
	uint32_t bypass_nr_gain;

	struct hiace_v3_register hiace_v3_regs;

	/*hiace HDR LUT*/
	uint32_t table_update;

	compat_pointer(detail_weight_table);
	compat_pointer(loglum_eotf_table);
	compat_pointer(luma_gamma_table);

};


struct dss_effect {
	bool dss_ready;
	bool arsr2p_sharp_support;
	bool arsr1p_sharp_support;
	bool acm_support;
	bool ace_support;
	bool hiace_support;
	bool post_hihdr_support;
	bool lcp_igm_support;
	bool lcp_gmp_support;
	bool lcp_xcc_support;
	bool gamma_support;
	bool dither_support;
	bool post_xcc_support;
};

struct dss_reg {
	uint32_t tag;
	uint32_t value;
};

/* ioctl update info */
struct dss_effect_info {
	int disp_panel_id; /* 0: inner panel; 1: outer panel */
	uint32_t modules;
	uint32_t arsr1p_rog_initialized;

	struct acm_info acm;
	struct arsr1p_info arsr1p[3]; /* 0: normal arsr1p; 1: FHD ROG arsr1p; 2: HD ROG arsr1p */
	struct arsr2p_info arsr2p[3]; /* 0: 1:1 arsr2p; 1: scale-up arsr2p; 2: scale-down arsr2p */
	struct lcp_info lcp;
	struct gamma_info gamma;
	struct hiace_info hiace;
	struct roi_rect dpp_roi[DPP_BUF_ROI_REGION_COUNT]; // only surpport two roi region
};

/* begin:effect buffer maneger */
struct gmp_info {
	uint32_t gmp_mode;
	uint32_t gmp_lut_low32bit[LCP_GMP_LUT_LENGTH];
	uint32_t gmp_lut_high4bit[LCP_GMP_LUT_LENGTH];
};

/* gama_info is for kernel; gamma_info is for hal sdk*/
struct gama_info {
	uint32_t gama_enable;
	uint32_t para_mode;
	uint32_t gama_r_lut[GAMA_LUT_LENGTH];
	uint32_t gama_g_lut[GAMA_LUT_LENGTH];
	uint32_t gama_b_lut[GAMA_LUT_LENGTH];
};

struct degamma_info {
	uint32_t degamma_enable;
	uint32_t degamma_r_lut[LCP_IGM_LUT_LENGTH];
	uint32_t degamma_g_lut[LCP_IGM_LUT_LENGTH];
	uint32_t degamma_b_lut[LCP_IGM_LUT_LENGTH];
};

struct xcc_info {
	uint32_t xcc_enable;
	uint32_t xcc_table[LCP_XCC_LUT_LENGTH];
};

/* content and properties of each buffer in dpp_buf_maneger */
struct dpp_buf_info {
/* status: defult value is 0
 * DPP_BUF_INVALIED:Invalide param
 * DPP_BUF_FROM_ANOTHER: copy from anther channel no need to update
 * DPP_BUF_NEW_UPDATED: new paramter, ready to config
 */
	uint32_t gmp_buf_save_status    : 3;
	uint32_t gama_buf_save_status   : 3;
	uint32_t degama_buf_save_status : 3;
	uint32_t xcc_buf_save_status    : 3;
	// status: defult value is 0
	// DPP_BUF_ONLINE_CONFIG: config in online display
	// DPP_BUF_WORKQ_CONFIG: config in workqueue (only use in gmp)
	uint32_t dpp_config_mode        : 2;
	// display effect is in or out of the ROI area,1:effect in roi,0:effect out roi
	uint32_t gmp_effect_roi         : 2;
	uint32_t gama_effect_roi        : 2;
	uint32_t degama_effect_roi      : 2;
	uint32_t xcc0_effect_roi        : 2;
	uint32_t xcc1_effect_roi        : 2;
	/* status: this module buffer can set reg to channel x (bit0 ch0, bit1 ch1)
	 * DPP_BUF_READY_FOR_BOTH ch0 and ch1 both can set this buffer
	 * DPP_BUF_READY_FOR_CH0 ch0 can set this buffer
	 * DPP_BUF_READY_FOR_CH1 ch1 can set this buffer
	 */
	uint32_t gmp_buf_cfg_status     : 2;
	uint32_t gama_buf_cfg_status    : 2;
	uint32_t degama_buf_cfg_status  : 2;
	uint32_t xcc_buf_cfg_status     : 2;

	struct gmp_info gmp;
	struct gama_info gama;
	struct degamma_info degamma;
	struct xcc_info xcc[2];
};

/* save DPP parameters from display effect hal
 * responsible for the rotation and condition maintenance of two buffers
 * maintaining the orderly configuration of parameters
 */

struct dpp_buf_maneger {
	/* the latest update of buffer is used to determine which block to
	 * rewrite when the buffer state is the same. defule value is 0
	*/
	/* 0:rotation; 1:both_effect; enum DPP_BUF_WORK_MODE */
	uint32_t buf_work_mode              : 1;
	uint32_t online_mode_available      : 1;
	uint32_t gmp_call_wq_scene          : 2;
	uint32_t reserved                   : 29;
	uint16_t latest_buf[DISP_PANEL_NUM];
	bool roi_new_updated;

	struct roi_rect buf_roi_region[DPP_BUF_ROI_REGION_COUNT]; // only surpport two roi region
	struct dpp_buf_info buf_info_list[DISP_PANEL_NUM][DPP_BUF_MAX_COUNT];
};

struct post_xcc_info {
	uint32_t enable;
	uint32_t post_xcc_table[POST_XCC_LUT_LENGTH];
};

/* dpufb save info */
struct dpufb_effect_info {
	uint32_t modules;
	uint32_t dpp_cmdlist_type;
	uint32_t arsr1p_rog_initialized;
	/* The initial value of Channel's software configuration state should be configed ready(0) */
	uint32_t dpp_chn_status[DPP_CHN_MAX_COUNT];

	struct acm_info acm;
	struct arsr1p_info arsr1p[3]; /* 0: normal arsr1p; 1: ROG arsr1p */
	struct arsr2p_info arsr2p[3]; /* 0: 1:1 arsr2p; 1: scale-up arsr2p; 2: scale-down arsr2p */
	struct hiace_info hiace;
	struct dpp_buf_maneger dpp_buf;
	struct post_xcc_info post_xcc;
};
/* end:effect buffer maneger */
#endif
