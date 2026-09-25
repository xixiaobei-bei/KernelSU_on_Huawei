/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef HISI_SPR_DSC1_2_H
#define HISI_SPR_DSC1_2_H

#include <linux/types.h>
#include "hisi_dss.h"

#define SPR_COLOR_COEF_LEN (3)
#define SPR_CSC_COEF_LEN   (4)
#define SPR_RC_BUF_THRESH_LEN (4)
#define SPR_RC_PARA_LEN (15)

struct spr_dsc_panel_para {
	uint8_t spr_rgbg2uyvy_8biten;
	uint8_t spr_hpartial_mode;
	uint8_t spr_partial_mode;
	uint8_t spr_rgbg2uyvy_en;
	uint8_t spr_horzborderdect;
	uint8_t spr_linebuf_1dmode;
	uint8_t spr_bordertb_dummymode;
	uint8_t spr_borderlr_dummymode;
	uint8_t spr_pattern_mode;
	uint8_t spr_pattern_en;
	uint8_t spr_subpxl_layout;
	uint8_t ck_gt_spr_en;
	uint8_t spr_en;

	uint32_t spr_coeffsel_even;
	uint32_t spr_coeffsel_odd;
	uint32_t pix_panel_arrange_sel;

	uint32_t spr_r_v0h0_coef[SPR_COLOR_COEF_LEN];
	uint32_t spr_r_v0h1_coef[SPR_COLOR_COEF_LEN];
	uint32_t spr_r_v1h0_coef[SPR_COLOR_COEF_LEN];
	uint32_t spr_r_v1h1_coef[SPR_COLOR_COEF_LEN];

	uint32_t spr_g_v0h0_coef[SPR_COLOR_COEF_LEN];
	uint32_t spr_g_v0h1_coef[SPR_COLOR_COEF_LEN];
	uint32_t spr_g_v1h0_coef[SPR_COLOR_COEF_LEN];
	uint32_t spr_g_v1h1_coef[SPR_COLOR_COEF_LEN];

	uint32_t spr_b_v0h0_coef[SPR_COLOR_COEF_LEN];
	uint32_t spr_b_v0h1_coef[SPR_COLOR_COEF_LEN];
	uint32_t spr_b_v1h0_coef[SPR_COLOR_COEF_LEN];
	uint32_t spr_b_v1h1_coef[SPR_COLOR_COEF_LEN];

	uint32_t spr_borderlr_detect_r;
	uint32_t spr_bordertb_detect_r;
	uint32_t spr_borderlr_detect_g;
	uint32_t spr_bordertb_detect_g;
	uint32_t spr_borderlr_detect_b;
	uint32_t spr_bordertb_detect_b;
	uint64_t spr_pixgain;
	uint32_t spr_borderl_position;
	uint32_t spr_borderr_position;
	uint32_t spr_bordert_position;
	uint32_t spr_borderb_position;
	uint32_t spr_r_diff;
	uint64_t spr_r_weight;
	uint32_t spr_g_diff;
	uint64_t spr_g_weight;
	uint32_t spr_b_diff;
	uint64_t spr_b_weight;
	uint32_t spr_rgbg2uyvy_coeff[SPR_CSC_COEF_LEN];

	uint32_t input_reso;
	uint8_t dsc_enable;
	uint8_t slice0_ck_gt_en;
	uint8_t slice1_ck_gt_en;
	uint32_t rcb_bits;

	uint8_t dsc_alg_ctrl;
	uint8_t bits_per_component;
	uint8_t dsc_sample;

	uint32_t bpp_chk;
	uint32_t pic_reso;
	uint32_t slc_reso;
	uint32_t initial_xmit_delay;
	uint32_t initial_dec_delay;
	uint32_t initial_scale_value;
	uint32_t scale_interval;
	uint32_t first_bpg;
	uint32_t second_bpg;
	uint32_t second_adj;
	uint32_t init_finl_ofs;
	uint32_t slc_bpg;
	uint32_t flat_range;
	uint32_t rc_mode_edge;
	uint32_t rc_qua_tgt;
	uint32_t rc_buf_thresh[SPR_RC_BUF_THRESH_LEN];
	uint32_t rc_para[SPR_RC_PARA_LEN];
	uint32_t spr_lut_table_len;
	uint32_t *spr_lut_table;
};


struct spr_dsc_platform_para {
	uint32_t iw;
	uint32_t ih;
	uint32_t spr_pxl_layout;
	uint32_t spr_hpartial_mode;
	uint32_t bpc;
	uint32_t bpp;
	uint32_t h_slice_num;
	uint32_t v_slice_num;

	uint8_t dual_slice;
	uint8_t partial_update_mode;

	uint8_t spr_en;
	uint8_t dsc_en;
	uint8_t spr_rgbg2uyvy_8biten;
	uint8_t spr_pattern_en;
};

void spr_dsc_para_init(struct hisi_fb_data_type *hisifd);
#if defined(CONFIG_HISI_FB_V350)
#define get_hsize_after_spr_dsc(hisifd, out_width)	(0)
#else
uint32_t get_hsize_after_spr_dsc(struct hisi_fb_data_type *hisifd, uint32_t rect_width);
#endif
void spr_dsc_init(struct hisi_fb_data_type *hisifd, bool fastboot_enable);
void spr_dsc_partial_updt_config(struct hisi_fb_data_type *hisifd,
	struct dss_rect dirty, struct dss_rect dirty_overlap);

#endif
