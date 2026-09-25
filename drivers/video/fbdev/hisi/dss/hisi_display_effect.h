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
#ifndef HISI_DISPLAY_EFFECT_H
#define HISI_DISPLAY_EFFECT_H

#include "hisi_fb_def.h"
#include "hisi_dss.h"

#if defined(CONFIG_HISI_FB_970)
#include "hisi_display_effect_kirin970.h"
#endif
#if defined(CONFIG_HISI_FB_V501)
#include "hisi_display_effect_dssv501.h"
#endif
#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V350)
#include "hisi_display_effect_dssv510.h"
#endif
#if defined(CONFIG_HISI_FB_V320)
#include "hisi_display_effect_dssv320.h"
#endif

#if defined(CONFIG_HISI_FB_V330)
#include "hisi_display_effect_dssv330.h"
#endif

#define CE_VALUE_BITWIDTH (8)
#define CE_VALUE_RANK     (1 << CE_VALUE_BITWIDTH)
#define METADATA_SIZE     (1024)

/* HIACE_INT_INTERVAL_MAX is threshold to control frame skipping,15000us */
#define HIACE_INT_INTERVAL_MAX (15000)

#if defined(CONFIG_HISI_FB_3660) || defined (CONFIG_HISI_FB_V320)
#define DISPLAY_EFFECT_USE_FRM_END_INT
#define XBLOCKNUM               (6)
#define YBLOCKNUM               (6)
#define HIACE_LHIST_RANK        (16)
#define HIACE_GAMMA_RANK        (11)
#define HIACE_GHIST_RANK        (32)
#define HIACE_FNA_RANK          (1)
#define CE_SIZE_HIST            (HIACE_GHIST_RANK + YBLOCKNUM * XBLOCKNUM * HIACE_LHIST_RANK)
#define CE_SIZE_LUT             (YBLOCKNUM * XBLOCKNUM * HIACE_GAMMA_RANK)
#elif defined (CONFIG_HISI_FB_970) || defined (CONFIG_HISI_FB_V501) || \
	defined (CONFIG_HISI_FB_V330)
#define XBLOCKNUM               (6)
#define YBLOCKNUM               (6)
#define HIACE_LHIST_RANK        (16)
#define HIACE_GAMMA_RANK        (8)
#define HIACE_GHIST_RANK        (32)
#define HIACE_FNA_RANK          (1)
#define CE_SIZE_HIST            (HIACE_GHIST_RANK * 2 + YBLOCKNUM * XBLOCKNUM * HIACE_LHIST_RANK + YBLOCKNUM * XBLOCKNUM * HIACE_FNA_RANK + 1)
#define CE_SIZE_LUT             (YBLOCKNUM * XBLOCKNUM * HIACE_GAMMA_RANK)
#define DETAIL_WEIGHT_SIZE      (9)
#define LOG_LUM_EOTF_LUT_SIZE   (32)
#define LUMA_GAMA_LUT_SIZE      (21)
#elif defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V350)
#define XBLOCKNUM               (6)
#define YBLOCKNUM               (6)
#define HIACE_LHIST_RANK        (16)
#define HIACE_GAMMA_RANK        (8)
#define HIACE_GHIST_RANK        (32)
#define HIACE_FNA_RANK          (1)
#define HIACE_SKIN_COUNT_RANK   (36)
#define CE_SIZE_HIST            (HIACE_GHIST_RANK * 2 + YBLOCKNUM * XBLOCKNUM * HIACE_LHIST_RANK + YBLOCKNUM * XBLOCKNUM * HIACE_FNA_RANK + HIACE_SKIN_COUNT_RANK + 1)
#define CE_SIZE_LUT             (YBLOCKNUM * XBLOCKNUM * HIACE_GAMMA_RANK)
#define DETAIL_WEIGHT_SIZE      (9)
#define LOG_LUM_EOTF_LUT_SIZE   (32)
#define LUMA_GAMA_LUT_SIZE      (21)
#else
#define CE_SIZE_HIST            CE_VALUE_RANK
#define CE_SIZE_LUT             CE_VALUE_RANK
#endif

// Debug parameter
#define ENABLE_EFFECT_HIACE           ((int)BIT(0))
#define ENABLE_EFFECT_BL              ((int)BIT(1))
#define DEBUG_EFFECT_ENTRY            ((int)BIT(0)) // Trace entrance
#define DEBUG_EFFECT_DELAY            ((int)BIT(1)) // Count average delay of each step
#define DEBUG_EFFECT_FRAME            ((int)BIT(2)) // Trace every frame
#define DEBUG_EFFECT_METADATA         ((int)BIT(3)) // Print metadata
#define DISPLAYENGINE_BL_DEBUG_FRAMES 5

// RGBW
#define LG_NT36772A_RGBW_ID          8
#define LG_NT36772A_RGBW_ID_HMA          12
#define BOE_HX83112E_RGBW_ID_HMA          13
extern const int JDI_TD4336_RT8555_RGBW_ID;
extern const int SHARP_TD4336_RT8555_RGBW_ID;
extern const int LG_NT36772A_RT8555_RGBW_ID;
/*
*1542 = gamma_r + gamma_g + gamma_b = (257 + 257 + 257) * sizeof(u16);
*1542 = degamma_r + degamma_g + degamma_b = (257 + 257 +257) * sizeof(u16);
*/
#define GM_IGM_LEN (1542 + 1542)

/*******************************************************************************
**ACM REG DIMMING
*/
#define ACM_HUE_SIZE 256
#define ACM_SATA_SIZE 256
#define ACM_SATR_SIZE 64
#define DIMMING_COUNT 20
#define SATR_REG_OFFSET 256
#define SATR_REG_RANGE (SATR_REG_OFFSET / 2)
#define HUE_REG_OFFSET 1024
#define HUE_REG_RANGE (HUE_REG_OFFSET / 6)
#define MAP_LUMLUT_LENGTH 10001

struct hisi_fb_data_type;

extern int8_t g_dpp_cmdlist_delay;

typedef enum {
	CMDLIST_DELAY_STATUS_DISABLE = 0,
	CMDLIST_DELAY_STATUS_ENABLE,
	CMDLIST_DELAY_STATUS_COUNT,
} cmdlist_delay_status;

typedef enum {
	CE_SERVICE_IDLE = 0,
	CE_SERVICE_HIST_REQ,
	CE_SERVICE_LUT_REQ,
	CE_SERVICE_STATUS_COUNT,
} ce_service_status;

typedef enum {
	CE_MODE_DISABLE = 0,
	CE_MODE_VIDEO,
	CE_MODE_IMAGE,
	CE_MODE_SINGLE,
	CE_MODE_COUNT,
} ce_mode;

typedef enum {
	SCENE_MODE_GALLERY = 0,
	SCENE_MODE_DEFAULT,
	SCENE_MODE_VIDEO,
} scene_mode;

typedef struct dss_display_effect_al {
	int ctrl_al_value; // ambient light
	struct mutex ctrl_lock;
} dss_display_effect_al_t;

typedef struct dss_display_effect_ce {
	int ctrl_ce_mode; //0: disable, 1: weak mode 2: stronger mode
	struct mutex ctrl_lock;
} dss_display_effect_ce_t;

typedef struct dss_display_effect_bl {
	int ctrl_bl_delta; // back light output by BLC algorithm
	struct mutex ctrl_lock;
} dss_display_effect_bl_t;

typedef struct dss_display_effect_bl_enable {
	int ctrl_bl_enable; // 0 -- disable back light algorithm; 1 -- enable algorithm
	struct mutex ctrl_lock;
} dss_display_effect_bl_enable_t;

typedef struct dss_display_effect_sre {
	int ctrl_sre_enable; // 0 -- disable sre algorithm; 1 -- enable sre algorithm
	int ctrl_sre_al; // ambient light
	int ctrl_sre_al_threshold; // ambient light threshold of enable sre
	struct mutex ctrl_lock;
} dss_display_effect_sre_t;

typedef struct dss_display_effect_metadata {
	char metadata[METADATA_SIZE];
	int count;
	struct mutex ctrl_lock;
} dss_display_effect_metadata_t;

typedef struct dss_display_effect_xcc {
	uint32_t xcc_enable;
	int xcc_table[12];
} dss_display_effect_xcc_t;

typedef struct display_engine_info {
	bool is_ready;
	bool blc_used;
	bool blc_enable;
	int blc_delta;
	int actual_bl_level;
	int ddic_cabc_mode;
	int ddic_color_mode;
	//ddic rgbw params
	int ddic_panel_id;
	int ddic_rgbw_mode;
	int ddic_rgbw_backlight;
	int rgbw_saturation_control;
	int frame_gain_limit;
	int frame_gain_speed;
	int color_distortion_allowance;
	int pixel_gain_limit;
	int pixel_gain_speed;
	int pwm_duty_gain;
	int rgbw_total_glim;
	//hbm params
	bool hbm_dimming;
	int hbm_level;
	int last_hbm_level;
	// backlight map from level(0~10000) to dbv level
	uint16_t dbv_map[MAP_LUMLUT_LENGTH];
	struct disp_lcdbrightnesscoloroeminfo lcd_color_oeminfo;
	struct mutex param_lock;
	struct display_engine_amoled_param amoled_param;
} display_engine_info_t;

typedef struct dss_ce_info {
	// Running control
	bool new_hist_rpt;
	bool first_lut_set;
	bool hiace_enable;
	bool enable;
	int gradual_frames;
	bool to_stop_hdr;
	bool to_stop_sre;
	struct mutex hist_lock;
	struct mutex lut_lock;

	// Register value
	int algorithm_result;
	uint32_t lut_sel;
	char __iomem *lut_base;
	uint32_t histogram[CE_SIZE_HIST];
#ifdef CONFIG_HISI_FB_V320
	uint32_t lut_table[CE_SIZE_LUT];
#else
	compat_pointer(lut_table);
#endif

	// Algorithm parameter
	int width; // Image size
	int height; // Image size
	int hist_mode; // 0 -- V of HSV, 1 -- Y of YUV
	int mode; // 0 -- video, 1 -- normal image
	ce_algorithm_parameter_t *p_ce_alg_param;
	int thminv;
} dss_ce_info_t;

typedef struct ce_service {
	bool is_ready;
	bool new_hist;
	bool new_lut;
	bool stop;
	bool hist_stop;
	bool lut_stop;
	bool use_last_value;
	bool blc_used;
	ce_service_status status;
	wait_queue_head_t wq_hist;
	wait_queue_head_t wq_lut;
	dss_ce_info_t *ce_info;
} ce_service_t;

typedef struct _dynamic_gamma {
	int use_dynamic_gm;
	int use_dynamic_gm_init;
} dss_gm_t;

typedef struct dss_module_update {
	bool acm_effect_updated;
	bool arsr1p_effect_updated;
	bool arsr2p_effect_updated;
	bool gmp_effect_updated;
	bool igm_effect_updated;
	bool xcc_effect_updated;
	bool gamma_effect_updated;
	bool hiace_effect_updated;
	bool post_xcc_effect_updated;
	bool dbm_dpp_effect_updated;
} dss_module_update_t;

/*******************************************************************************
**ACM REG DIMMING
*/
typedef struct acm_reg {
	uint32_t acm_lut_hue_table[ACM_HUE_SIZE];
	uint32_t acm_lut_sata_table[ACM_SATA_SIZE];
	uint32_t acm_lut_satr0_table[ACM_SATR_SIZE];
	uint32_t acm_lut_satr1_table[ACM_SATR_SIZE];
	uint32_t acm_lut_satr2_table[ACM_SATR_SIZE];
	uint32_t acm_lut_satr3_table[ACM_SATR_SIZE];
	uint32_t acm_lut_satr4_table[ACM_SATR_SIZE];
	uint32_t acm_lut_satr5_table[ACM_SATR_SIZE];
	uint32_t acm_lut_satr6_table[ACM_SATR_SIZE];
	uint32_t acm_lut_satr7_table[ACM_SATR_SIZE];
} acm_reg_t;

typedef struct acm_reg_table {
	uint32_t *acm_lut_hue_table;
	uint32_t *acm_lut_sata_table;
	uint32_t *acm_lut_satr0_table;
	uint32_t *acm_lut_satr1_table;
	uint32_t *acm_lut_satr2_table;
	uint32_t *acm_lut_satr3_table;
	uint32_t *acm_lut_satr4_table;
	uint32_t *acm_lut_satr5_table;
	uint32_t *acm_lut_satr6_table;
	uint32_t *acm_lut_satr7_table;
} acm_reg_table_t;

/*******************************************************************************
** FUNCTIONS PROTOTYPES
*/
extern uint32_t g_enable_effect;
extern uint32_t g_debug_effect;
extern int g_factory_gamma_enable;


void de_ctrl_ic_dim(struct hisi_fb_data_type *hisifd);
void de_open_ic_dim(struct work_struct *work);
void de_set_hbm_func(struct work_struct *work);

void hisi_effect_init(struct hisi_fb_data_type *hisifd);
void hisi_effect_deinit(struct hisi_fb_data_type *hisifd);

int hisifb_ce_service_blank(int blank_mode, struct fb_info *info);
int hisifb_ce_service_get_support(struct fb_info *info, void __user *argp);
int hisifb_ce_service_get_limit(struct fb_info *info, void __user *argp);
int hisifb_ce_service_get_param(struct fb_info *info, void __user *argp);
int hisifb_ce_service_set_param(struct fb_info *info, const void __user *argp);
int hisifb_ce_service_enable_hiace(struct fb_info *info, const void __user *argp);
int hisifb_ce_service_get_hist(struct fb_info *info, void __user *argp);
int hisifb_ce_service_set_lut(struct fb_info *info, const void __user *argp);
int hisifb_ce_service_set_HDR10_lut(struct fb_info *info, void __user *argp);
int hisifb_get_hiace_enable(struct fb_info *info, void __user *argp);
int hisifb_get_hiace_roi(struct fb_info *info, void __user *argp);
int hisifb_get_reg_val(struct fb_info *info, void __user *argp);
int hisifb_display_engine_register(struct hisi_fb_data_type *hisifd);
int hisifb_display_engine_unregister(struct hisi_fb_data_type *hisifd);
int hisifb_display_engine_blank(int blank_mode, struct fb_info *info);
int hisifb_display_engine_init(struct fb_info *info, void __user *argp);
int hisifb_display_engine_deinit(struct fb_info *info, void __user *argp);
int hisifb_display_engine_param_get(struct fb_info *info, void __user *argp);
int hisifb_display_engine_param_set(struct fb_info *info, void __user *argp);
void hisifb_display_engine_workqueue_handler(struct work_struct *work);
ssize_t hisifb_display_effect_al_ctrl_show(struct fb_info *info, char *buf);
ssize_t hisifb_display_effect_al_ctrl_store(struct fb_info *info, const char *buf, size_t count);
ssize_t hisifb_display_effect_ce_ctrl_show(struct fb_info *info, char *buf);
ssize_t hisifb_display_effect_ce_ctrl_store(struct fb_info *info, const char *buf, size_t count);
ssize_t hisifb_display_effect_bl_ctrl_show(struct fb_info *info, char *buf);
ssize_t hisifb_display_effect_bl_ctrl_store(struct fb_info *info, const char *buf, size_t count);
ssize_t hisifb_display_effect_bl_enable_ctrl_show(struct fb_info *info, char *buf);
ssize_t hisifb_display_effect_bl_enable_ctrl_store(struct fb_info *info, const char *buf, size_t count);
ssize_t hisifb_display_effect_sre_ctrl_show(struct fb_info *info, char *buf);
ssize_t hisifb_display_effect_sre_ctrl_store(struct fb_info *info, const char *buf, size_t count);
ssize_t hisifb_display_effect_metadata_ctrl_show(struct fb_info *info, char *buf);
ssize_t hisifb_display_effect_metadata_ctrl_store(struct fb_info *info, const char *buf, size_t count);
void hisifb_display_effect_func_switch(struct hisi_fb_data_type *hisifd, const char *command);

bool hisifb_display_effect_is_need_ace(struct hisi_fb_data_type *hisifd);
bool hisifb_hiace_roi_is_disable(struct hisi_fb_data_type *hisifd);
bool hisifb_display_effect_is_need_blc(struct hisi_fb_data_type *hisifd);
bool hisifb_display_effect_check_bl_value(int curr, int last);
bool hisifb_display_effect_check_bl_delta(int curr, int last);
bool hisifb_display_effect_fine_tune_backlight(struct hisi_fb_data_type *hisifd, int backlight_in, int *backlight_out);

void init_acm_ce(struct hisi_fb_data_type *hisifd);
void hisi_dss_dpp_ace_set_reg(struct hisi_fb_data_type *hisifd);
void hisi_dpp_ace_end_handle_func(struct work_struct *work);

void init_hiace(struct hisi_fb_data_type *hisifd);
void init_noisereduction(struct hisi_fb_data_type *hisifd);
void hisi_dss_dpp_hiace_set_reg(struct hisi_fb_data_type *hisifd);
void hisi_dpp_hiace_end_handle_func(struct work_struct *work);
void hisifb_update_dynamic_gamma(struct hisi_fb_data_type *hisifd, const char *buffer, size_t len);
int hisifb_use_dynamic_gamma(struct hisi_fb_data_type *hisifd, char __iomem *dpp_base);
int hisifb_use_dynamic_degamma(struct hisi_fb_data_type *hisifd, char __iomem *dpp_base);
void hisifb_update_gm_from_reserved_mem(uint32_t *gm_r, uint32_t *gm_g, uint32_t *gm_b, uint32_t *igm_r, uint32_t *igm_g, uint32_t *igm_b);
int hisifb_display_effect_blc_cabc_update(struct hisi_fb_data_type *hisifd);

void hisi_display_effect_init(struct hisi_fb_data_type *hisifd);
int display_engine_hbm_param_set(struct hisi_fb_data_type *hisifd,
	display_engine_hbm_param_t *param);

int hisi_effect_arsr2p_info_get(struct hisi_fb_data_type *hisifd, struct arsr2p_info *arsr2p);
int hisi_effect_arsr1p_info_get(struct hisi_fb_data_type *hisifd, struct arsr1p_info *arsr1p);
int hisi_effect_acm_info_get(struct hisi_fb_data_type *hisifd, struct acm_info *acm);
int hisi_effect_lcp_info_get(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp);
int hisi_effect_gamma_info_get(struct hisi_fb_data_type *hisifd, struct gamma_info *gamma);
int hisi_effect_hiace_info_get(struct hisi_fb_data_type *hisifd, struct hiace_info *hiace);
int hisifb_ce_service_get_hiace_param(struct fb_info *info, void __user *argp);

int hisi_effect_arsr2p_info_set(struct hisi_fb_data_type *hisifd, struct arsr2p_info *arsr2p);
int hisi_effect_arsr1p_info_set(struct hisi_fb_data_type *hisifd, struct arsr1p_info *arsr1p);
int hisi_effect_acm_info_set(struct hisi_fb_data_type *hisifd, struct acm_info *acm);
int hisi_effect_lcp_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp);
int hisi_effect_gmp_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp_src);
int hisi_effect_igm_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp_src);
int hisi_effect_xcc_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp_src);
int hisi_effect_xcc_info_set_kernel(struct hisi_fb_data_type *hisifd, struct dss_display_effect_xcc *lcp_src);

int hisi_effect_gamma_info_set(struct hisi_fb_data_type *hisifd, struct gamma_info *gamma);

void hisi_effect_color_dimming_acm_reg_set(struct hisi_fb_data_type *hisifd, acm_reg_t *cur_acm_reg);
void hisi_effect_color_dimming_acm_reg_init(struct hisi_fb_data_type *hisifd);

void hisi_effect_acm_set_reg(struct hisi_fb_data_type *hisifd);
void hisi_effect_lcp_set_reg(struct hisi_fb_data_type *hisifd);
void hisi_effect_gamma_set_reg(struct hisi_fb_data_type *hisifd);
void hisi_hiace_skip_frame_report(struct hisi_fb_data_type *hisifd);

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V350)
int hisi_effect_dpp_info_set(struct hisi_fb_data_type *hisifd,
	struct dss_effect_info *effect_info);
int hisi_effect_post_xcc_info_set(struct hisi_fb_data_type *hisifd,
	struct lcp_info *lcp_src);
int hisi_effect_dpproi_info_set(struct hisi_fb_data_type *hisifd,
	struct roi_rect *dpproi_src, uint32_t roi_region_count);
void hisi_effect_post_xcc_set_reg(struct hisi_fb_data_type *hisifd);
int hisi_effect_dpp_set_buf_work_mode(struct hisi_fb_data_type *hisifd,
	const struct dss_effect_info *effect_info, uint32_t *modules_roi,
	uint32_t *modules_none_roi);
int hisi_effect_dpp_buf_info_set(struct hisi_fb_data_type *hisifd,
	struct dss_effect_info *effect_info, bool roi_flag);
#endif
void hisi_dss_effect_set_reg(struct hisi_fb_data_type *hisifd);

int hisi_effect_arsr2p_config(struct arsr2p_info *arsr2p_effect_dst, int ih_inc, int iv_inc);
int hisi_effect_arsr1p_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req);
int hisi_effect_hiace_config(struct hisi_fb_data_type *hisifd);

void hisi_dss_roi_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req);
int hisifb_hiace_roi_info_init(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req);
void update_hiace_roi_by_dirty_region(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req);
void hisifb_hiace_roi_reg_set(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req);
void update_gamma_xcc_roi_by_dirty_region(struct hisi_fb_data_type *hisifd,
	uint32_t *top_left, uint32_t *bot_right);
void hisifb_gamma_xcc_reg_set(struct hisi_fb_data_type *hisifd,
	uint32_t top_left, uint32_t bot_right);
void hiace_size_config(struct hisi_fb_data_type *hisifd, uint32_t width, uint32_t height);
uint32_t get_fixed_point_offset(uint32_t half_block_size);

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600)
void hisifb_effect_gmp_lut_workqueue_handler(struct work_struct *work);
#endif

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600)
void deinit_effect(struct hisi_fb_data_type *hisifd);
#endif

#endif  //HISI_DISPLAY_EFFECT_H
