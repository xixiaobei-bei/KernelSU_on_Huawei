/* include/linux/hisi_dss.h
 *
 * Copyright (c) 2013-2021, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef _HISI_DSS_H_
#define _HISI_DSS_H_

#include "hisi_dss_enum.h"
#include "hisi_dss_display_engine.h"

#define GTM_PQ2SLF_SIZE 8
#define GTM_LUT_PQ2SLF_SIZE 64
#define GTM_LUT_LUMA_SIZE 256
#define GTM_LUT_CHROMA_SIZE 256
#define GTM_LUT_CHROMA0_SIZE 256
#define GTM_LUT_CHROMA1_SIZE 256
#define GTM_LUT_CHROMA2_SIZE 256
#define GTM_LUT_LUMALUT0_SIZE 64
#define GTM_LUT_CHROMALUT0_SIZE 16

#define ITM_LUT_LUMA_SIZE 256
#define ITM_LUT_CHROMA_SIZE 256
#define ITM_LUT_CHROMA0_SIZE 256
#define ITM_LUT_CHROMA1_SIZE 256
#define ITM_LUT_CHROMA2_SIZE 256
#define ITM_LUT_LUMALUT0_SIZE 64
#define ITM_LUT_CHROMALUT0_SIZE 16

/*********************** ALSC Feature *************************/
struct alsc_reg_info {
	uint32_t alsc_en;
	uint32_t alsc_addr;
	uint32_t alsc_size;
	uint32_t pic_size;
};

struct alsc_enable_info {
	uint32_t alsc_en;
	uint32_t alsc_addr;
	uint32_t alsc_size;
	uint32_t bl_update;
};

/* this head file to save the structs that both ade and dss will use
 * note: if the left_align is 8,right_align is 8,and w_min is larger than 802,then w_min should be set to 808,
 * make sure that it is 8 align,if w_min is set to 802,there will be an error.left_align,right_align,top_align
 * bottom_align,w_align,h_align,w_min and h_min's valid value should be larger than 0,top_start and bottom_start
 * maybe equal to 0. if it's not surpport partial update, these value should set to invalid value(-1).
 *
 */
typedef struct lcd_dirty_region_info {
	int left_align;
	int right_align;
	int top_align;
	int bottom_align;

	int w_align;
	int h_align;
	int w_min;
	int h_min;

	int top_start;
	int bottom_start;
	int reserved;

	struct alsc_reg_info alsc;
} lcd_dirty_region_info_t;

typedef struct dss_rect {
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
} dss_rect_t;

typedef struct dss_rect_ltrb {
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;
} dss_rect_ltrb_t;

typedef struct dss_mmbuf {
	uint32_t addr;
	uint32_t size;
	int owner;
} dss_mmbuf_t;

typedef struct iova_info {
	uint64_t iova;
	uint64_t size;
	int share_fd;
	int calling_pid;
} iova_info_t;

typedef struct dss_img {
	uint32_t format;
	uint32_t width;
	uint32_t height;
	uint32_t bpp; /* bytes per pixel */
	uint32_t buf_size;
	uint32_t stride;
	uint32_t stride_plane1;
	uint32_t stride_plane2;
	uint64_t phy_addr;
	uint64_t vir_addr;
	uint32_t offset_plane1;
	uint32_t offset_plane2;

	uint64_t afbc_header_addr;
	uint64_t afbc_header_offset;
	uint64_t afbc_payload_addr;
	uint64_t afbc_payload_offset;
	uint32_t afbc_header_stride;
	uint32_t afbc_payload_stride;
	uint32_t afbc_scramble_mode;
	uint32_t hfbcd_block_type;
	uint64_t hfbc_header_addr0;
	uint64_t hfbc_header_offset0;
	uint64_t hfbc_payload_addr0;
	uint64_t hfbc_payload_offset0;
	uint32_t hfbc_header_stride0;
	uint32_t hfbc_payload_stride0;
	uint64_t hfbc_header_addr1;
	uint64_t hfbc_header_offset1;
	uint64_t hfbc_payload_addr1;
	uint64_t hfbc_payload_offset1;
	uint32_t hfbc_header_stride1;
	uint32_t hfbc_payload_stride1;
	uint32_t hfbc_scramble_mode;
	uint32_t hfbc_mmbuf_base0_y8;
	uint32_t hfbc_mmbuf_size0_y8;
	uint32_t hfbc_mmbuf_base1_c8;
	uint32_t hfbc_mmbuf_size1_c8;
	uint32_t hfbc_mmbuf_base2_y2;
	uint32_t hfbc_mmbuf_size2_y2;
	uint32_t hfbc_mmbuf_base3_c2;
	uint32_t hfbc_mmbuf_size3_c2;

	uint32_t hebcd_block_type;
	uint64_t hebc_header_addr0;
	uint64_t hebc_header_offset0;
	uint64_t hebc_payload_addr0;
	uint64_t hebc_payload_offset0;
	uint32_t hebc_header_stride0;
	uint32_t hebc_payload_stride0;
	uint64_t hebc_header_addr1;
	uint64_t hebc_header_offset1;
	uint64_t hebc_payload_addr1;
	uint64_t hebc_payload_offset1;
	uint32_t hebc_header_stride1;
	uint32_t hebc_payload_stride1;
	uint32_t hebc_scramble_mode;
	uint32_t hebc_mmbuf_base0_y8;
	uint32_t hebc_mmbuf_size0_y8;
	uint32_t hebc_mmbuf_base1_c8;
	uint32_t hebc_mmbuf_size1_c8;

	uint32_t mmbuf_base;
	uint32_t mmbuf_size;

	uint32_t mmu_enable;
	uint32_t csc_mode;
	uint32_t secure_mode;
	int32_t shared_fd;
	uint32_t display_id;
} dss_img_t;

typedef struct dss_block_info {
	int32_t first_tile;
	int32_t last_tile;
	uint32_t acc_hscl;
	uint32_t h_ratio;
	uint32_t v_ratio;
	uint32_t h_ratio_arsr2p;
	uint32_t arsr2p_left_clip;
	uint32_t both_vscfh_arsr2p_used;
	dss_rect_t arsr2p_in_rect;
	uint32_t arsr2p_src_x; /* new added */
	uint32_t arsr2p_src_y; /* new added */
	uint32_t arsr2p_dst_x; /* new added */
	uint32_t arsr2p_dst_y; /* new added */
	uint32_t arsr2p_dst_w; /* new added */
	int32_t h_v_order;
} dss_block_info_t;

typedef struct dss_layer {
	dss_img_t img;
	dss_rect_t src_rect;
	dss_rect_t src_rect_mask;
	dss_rect_t dst_rect;
	uint32_t transform;
	int32_t blending;
	uint32_t glb_alpha;
	uint32_t color; /* background color or dim color */
	int32_t layer_idx;
	int32_t chn_idx;
	uint32_t need_cap;
	int32_t acquire_fence;

	dss_block_info_t block_info;

	uint32_t is_cld_layer;
	uint32_t cld_width;
	uint32_t cld_height;
	uint32_t cld_data_offset;
	int32_t axi_idx;
	int32_t mmaxi_idx;
} dss_layer_t;

typedef struct dss_wb_block_info {
	uint32_t acc_hscl;
	uint32_t h_ratio;
	int32_t h_v_order;
	dss_rect_t src_rect;
	dss_rect_t dst_rect;
} dss_wb_block_info_t;

typedef struct dss_wb_layer {
	dss_img_t dst;
	dss_rect_t src_rect;
	dss_rect_t dst_rect;
	uint32_t transform;
	int32_t chn_idx;
	uint32_t need_cap;

	int32_t acquire_fence;
	int32_t release_fence;
	int32_t retire_fence;

	dss_wb_block_info_t wb_block_info;
} dss_wb_layer_t;

typedef struct dss_crc_info {
	uint32_t crc_ov_result;
	uint32_t crc_ldi_result;
	uint32_t crc_sum_result;
	uint32_t crc_ov_frm;
	uint32_t crc_ldi_frm;
	uint32_t crc_sum_frm;

	uint32_t err_status;
	uint32_t reserved0;
} dss_crc_info_t;

typedef struct mdc_ch_info {
	uint32_t hold_flag;
	uint32_t is_drm;
	uint32_t rch_need_cap;
	uint32_t wch_need_cap;
	int32_t rch_idx;
	int32_t wch_idx;
	int32_t ovl_idx;
	uint32_t wb_composer_type;

	uint32_t mmbuf_addr;
	uint32_t mmbuf_size;
	uint32_t need_request_again;
} mdc_ch_info_t;
/*********************** MDC END *****************************/

typedef struct dss_overlay_block {
	dss_layer_t layer_infos[MAX_DSS_SRC_NUM];
	dss_rect_t ov_block_rect;
	uint32_t layer_nums;
	uint32_t reserved0;
} dss_overlay_block_t;

/********************** HIHDR START **************************/
typedef struct gtm_basic_info {
	int isinslf;
	int input_min_e;
	int output_min_e;

	int ratiolutinclip;
	int ratiolutoutclip;
	int lumaoutbit0;
	int lumaoutbit1;

	int chromaratiolutstep0;
	int chromaratiolutstep1;

	int lumaratiolutstep0;
	int lumaratiolutstep1;

	int chromalutthres;
	int lumalutthres;

	int y_transform[3];

	int chromaoutbit1;
	int chromaoutbit2;

	int chromaRatioclip;

	uint32_t pq2slf_step[GTM_PQ2SLF_SIZE];
	uint32_t pq2slf_pos[GTM_PQ2SLF_SIZE];
	uint32_t pq2slf_num[GTM_PQ2SLF_SIZE];

	uint32_t lut_pq2slf[GTM_LUT_PQ2SLF_SIZE];
} gtm_basic_info_t;

typedef struct gtm_dynamic_info {
	int lut_luma[GTM_LUT_LUMA_SIZE];
	int lut_chroma[GTM_LUT_CHROMA_SIZE];
	int lut_chroma0[GTM_LUT_CHROMA0_SIZE];
	int lut_chroma1[GTM_LUT_CHROMA1_SIZE];
	int lut_chroma2[GTM_LUT_CHROMA2_SIZE];
	uint32_t lut_lumalut0[GTM_LUT_LUMALUT0_SIZE];
	int lut_chromalut0[GTM_LUT_CHROMALUT0_SIZE];
} gtm_dynamic_info_t;

typedef struct itm_basic_info {
	int input_min_e;
	int output_min_e;

	int ratiolutinclip;
	int ratiolutoutclip;
	int lumaoutbit0;
	int lumaoutbit1;

	int chromaratiolutstep0;
	int chromaratiolutstep1;

	int lumaratiolutstep0;
	int lumaratiolutstep1;

	int chromalutthres;
	int lumalutthres;

	int chromaoutbit1;
	int chromaoutbit2;

	int chromaRatioclip;
} itm_basic_info_t;

typedef struct itm_dynamic_info {
	uint32_t lut_luma[ITM_LUT_LUMA_SIZE];  // GTM_LUT_LUMA
	uint32_t lut_chroma[ITM_LUT_CHROMA_SIZE];
	uint32_t lut_chroma0[ITM_LUT_CHROMA0_SIZE];
	uint32_t lut_chroma1[ITM_LUT_CHROMA1_SIZE];
	uint32_t lut_chroma2[ITM_LUT_CHROMA2_SIZE];
	uint32_t lut_lumalut0[ITM_LUT_LUMALUT0_SIZE];
	uint32_t lut_chromalut0[ITM_LUT_CHROMALUT0_SIZE];
} itm_dynamic_info_t;

typedef struct hihdr_itm_info {
	uint32_t csc_flag;
	uint32_t flag;
	itm_basic_info_t basic_info;
	itm_dynamic_info_t dynamic_info;
} hihdr_itm_info_t;

typedef struct hihdr_gtm_info {
	uint32_t flag;
	gtm_basic_info_t basic_info;
	gtm_dynamic_info_t dynamic_info;
} hihdr_gtm_info_t;

typedef struct dss_hihdr_info {
	uint32_t hihdr_status;
	hihdr_itm_info_t itm_info;
	hihdr_gtm_info_t gtm_info;
} dss_hihdr_info_t;
/********************** HIHDR END **************************/


typedef struct dss_overlay {
	dss_wb_layer_t wb_layer_infos[MAX_DSS_DST_NUM];
	dss_rect_t wb_ov_rect;
	uint32_t wb_layer_nums;
	uint32_t wb_compose_type;

	/* hihdr */
	uint64_t ov_hihdr_infos_ptr;

	/* struct dss_overlay_block */
	uint64_t ov_block_infos_ptr;
	uint32_t ov_block_nums;
	int32_t ovl_idx;
	uint32_t wb_enable;
	uint32_t frame_no;

	/* dirty region */
	dss_rect_t dirty_rect;
	int spr_overlap_type;
	uint32_t frameFps;

	/* resolution change */
	struct dss_rect res_updt_rect;

	/* crc */
	dss_crc_info_t crc_info;
	int32_t crc_enable_status;
	uint32_t sec_enable_status;

	uint32_t to_be_continued;
	int32_t release_fence;
	int32_t retire_fence;
	uint8_t video_idle_status;
	uint8_t mask_layer_exist;
	uint8_t reserved_0;
	uint8_t reserved_1;

	uint32_t online_wait_timediff;

	bool hiace_roi_support;
	bool hiace_roi_enable;
	dss_rect_t hiace_roi_rect;

	/* rog scale size, for scale ratio calculating */
	uint32_t rog_width;
	uint32_t rog_height;

	uint32_t is_evs_request;
} dss_overlay_t;

struct dss_fence {
	int32_t release_fence;
	int32_t retire_fence;
};

typedef struct dss_vote_cmd {
	uint64_t dss_pri_clk_rate;
	uint64_t dss_axi_clk_rate;
	uint64_t dss_pclk_dss_rate;
	uint64_t dss_pclk_pctrl_rate;
	uint64_t dss_mmbuf_rate;
	uint32_t dss_voltage_level;
	uint32_t reserved;
} dss_vote_cmd_t;

typedef struct ce_algorithm_parameter {
	int iDiffMaxTH;
	int iDiffMinTH;
	int iAlphaMinTH;
	int iFlatDiffTH;
	int iBinDiffMaxTH;

	int iDarkPixelMinTH;
	int iDarkPixelMaxTH;
	int iDarkAvePixelMinTH;
	int iDarkAvePixelMaxTH;
	int iWhitePixelTH;
	int fweight;
	int fDarkRatio;
	int fWhiteRatio;

	int iDarkPixelTH;
	int fDarkSlopeMinTH;
	int fDarkSlopeMaxTH;
	int fDarkRatioMinTH;
	int fDarkRatioMaxTH;

	int iBrightPixelTH;
	int fBrightSlopeMinTH;
	int fBrightSlopeMaxTH;
	int fBrightRatioMinTH;
	int fBrightRatioMaxTH;

	int iZeroPos0MaxTH;
	int iZeroPos1MaxTH;

	int iDarkFMaxTH;
	int iDarkFMinTH;
	int iPos0MaxTH;
	int iPos0MinTH;

	int fKeepRatio;
} ce_algorithm_parameter_t;

typedef struct ce_parameter {
	int width;
	int height;
	int hist_mode;
	int mode;
	int result;
	uint32_t reserved0;
	uint32_t *histogram;
	uint8_t *lut_table;
	void *service;
	ce_algorithm_parameter_t ce_alg_param;
} ce_parameter_t;

/* HIACE struct */
typedef struct hiace_alg_parameter {
	/* paramters to avoid interference of black/white edge */
	int iGlobalHistBlackPos;
	int iGlobalHistWhitePos;
	int iGlobalHistBlackWeight;
	int iGlobalHistWhiteWeight;
	int iGlobalHistZeroCutRatio;
	int iGlobalHistSlopeCutRatio;

	/* Photo metadata */
	char Classifieresult[META_DATA_SIZE];
	int iResultLen;

	/* function enable/disable switch */
	int iDoLCE;
	int iDoSRE;
	int iDoAPLC;

	/* minimum ambient light to enable SRE */
	int iLaSensorSREOnTH;

	int iWidth;
	int iHeight;
	int bitWidth;
	int iMode; /* Image(1) or Video(0) mode */
	int iLevel; /* Video(0), gallery(1) ... */
	int ilhist_sft;

	int iMaxLcdLuminance;
	int iMinLcdLuminance;
	int iMaxBackLight;
	int iMinBackLight;
	int iAmbientLight;
	int iBackLight;
	long long lTimestamp; /* Timestamp of frame in millisecond */

	/* path of xml file */
	char chCfgName[512];
} hiace_alg_parameter_t;

typedef struct hiace_interface_set {
	int disp_panel_id;
	int thminv;
	compat_uint_pointer(lut);
} hiace_interface_set_t;

struct hiace_enable_set {
	int disp_panel_id;
	uint32_t enable;
};

typedef struct hiace_HDR10_lut_set {
	unsigned int *detail_weight;
	unsigned int *LogLumEOTFLUT;
	unsigned int *LumEOTFGammaLUT;
} hiace_HDR10_lut_set_t;

struct disp_panelid {
	uint32_t modulesn;
	uint32_t equipid;
	uint32_t modulemanufactdate;
	uint32_t vendorid;
};

struct disp_coloruniformparams {
	uint32_t c_lmt[3];
	uint32_t mxcc_matrix[3][3];
	uint32_t white_decay_luminace;
};

struct disp_colormeasuredata {
	uint32_t chroma_coordinates[4][2];
	uint32_t white_luminance;
};

struct disp_lcdbrightnesscoloroeminfo {
	uint32_t id_flag;
	uint32_t tc_flag;
	struct disp_panelid  panel_id;
	struct disp_coloruniformparams color_params;
	struct disp_colormeasuredata color_mdata;
};

struct hdr_metadata {
	uint32_t electro_optical_transfer_function;
	uint32_t static_metadata_descriptor_id;
	uint32_t red_primary_x;
	uint32_t red_primary_y;
	uint32_t green_primary_x;
	uint32_t green_primary_y;
	uint32_t blue_primary_x;
	uint32_t blue_primary_y;
	uint32_t white_point_x;
	uint32_t white_point_y;
	uint32_t max_display_mastering_luminance;
	uint32_t min_display_mastering_luminance;
	uint32_t max_content_light_level;
	uint32_t max_frame_average_light_level;
};

struct dss_csc_info {
	int chn_idx;
	uint32_t format;
	uint32_t csc_mode;
	bool need_yuv2p3;
};

typedef struct display_engine_color_rectify_param {
	struct disp_lcdbrightnesscoloroeminfo lcd_color_oeminfo;
} display_engine_color_rectify_param_t;

typedef struct display_engine_param {
	uint32_t modules;
	display_engine_blc_param_t blc;
	display_engine_ddic_cabc_param_t ddic_cabc;
	display_engine_ddic_color_param_t ddic_color;
	display_engine_ddic_rgbw_param_t ddic_rgbw;
	display_engine_panel_info_param_t panel_info;
	display_engine_common_panel_info_param_t common_panel_info;
	display_engine_hbm_param_t hbm;
	display_engine_color_rectify_param_t color_param;
	display_engine_amoled_param_t amoled_param;
	display_engine_flicker_detector_config_t flicker_detector_config;
	display_engine_share_memory_t share_mem;
	display_engine_manufacture_brightness_t manufacture_brightness;
	display_engine_foldable_info_t foldable_info;
	display_engine_sync_ud_fingerprint_backlight_t ud_fingerprint_backlight;
	display_engine_demura_t demura;
	display_engine_irdrop_t irdrop;
	display_engine_amoled_param_sync_t amoled_param_sync;
	display_engine_ddic_irc_param_t ddic_irc;
	struct display_engine_ddic_local_hbm_param ddic_local_hbm;
	bool is_maintaining;
} display_engine_param_t;

typedef struct _panel_region_notify {
	enum_en_notify_mode notify_mode;
	enum_en_display_region panel_display_region;
} panel_region_notify_t;

struct dss_hiace_single_mode_ctrl_info {
	uint32_t info_type;      /* global hist, local hist, or fna */
	uint32_t blocking_mode;  /* 0:asynchronous, 1:synchronize; */
	uint32_t isr_handle;     /* if get by isr routine directly */
};

struct platform_product_info {
	uint32_t max_hwc_mmbuf_size;
	uint32_t max_mdc_mmbuf_size;
	uint32_t fold_display_support;
	uint32_t dfr_support_value;
	uint32_t dummy_pixel_num;
	uint32_t ifbc_type;
	uint32_t need_two_panel_display;
	uint32_t virtual_fb_xres;
	uint32_t virtual_fb_yres;
	uint32_t p3_support;
	uint32_t hdr_flw_support;
	uint32_t post_hihdr_support;
	uint32_t spr_enable;
	uint32_t panel_id[DISPLAY_PANEL_ID_MAX];
	uint32_t xres[DISPLAY_PANEL_ID_MAX];
	uint32_t yres[DISPLAY_PANEL_ID_MAX];
	uint32_t width[DISPLAY_PANEL_ID_MAX]; /* mm */
	uint32_t height[DISPLAY_PANEL_ID_MAX];
	uint32_t dfr_method;
	uint32_t support_tiny_porch_ratio;
	uint32_t support_ddr_bw_adjust;
	uint32_t actual_porch_ratio;
};

struct hiace_roi_info {
	uint32_t roi_top;
	uint32_t roi_left;
	uint32_t roi_bot;
	uint32_t roi_right;
	uint32_t roi_enable;
};
#endif /*_HISI_DSS_H_*/
