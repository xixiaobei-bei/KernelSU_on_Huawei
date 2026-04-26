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

#include "hisi_overlay_utils.h"
#include "hisi_mmbuf_manager.h"


/*******************************************************************************
 * DSS ARSR2P
 */
#define ARSR2P_PHASE_NUM 9
#define ARSR2P_TAP4 4
#define ARSR2P_TAP6 6
#define ARSR2P_MIN_INPUT 16
#define ARSR2P_MAX_WIDTH 2560
#define ARSR2P_MAX_HEIGHT 8192
#define ARSR2P_DST_MAX_WIDTH 8192
#define ARSR2P_SCALE_MAX 60

#define ARSR2P_SCL_UP_OFFSET 0x48
#define ARSR2P_COEF_H0_OFFSET 0x100
#define ARSR2P_COEF_H1_OFFSET 0x200

#define ARSR1P_COEF_OFFSET 0x24

#define LSC_ROW 2
#define LSC_COL 27
/* arsr1p lsc gain */
static const uint32_t g_arsr1p_lsc_gain_table[LSC_ROW][LSC_COL] = {
	{1024, 1085, 1158, 1232, 1305, 1382, 1454, 1522, 1586, 1646, 1701, 1755,
	1809, 1864, 1926, 1989, 2058, 2131, 2207, 2291, 2376, 2468, 2576, 2687,
	2801, 2936, 3038},   /* pgainlsc0 */
	{1052, 1122, 1192, 1268, 1345, 1418, 1488, 1554, 1616, 1674, 1728, 1783,
	1838, 1895, 1957, 2023, 2089, 2165, 2245, 2331, 2424, 2523, 2629, 2744,
	2866, 3006, 3038}   /* pgainlsc1 */
};

/* c0, c1, c2, c3 */
static const int g_coef_auv_scl_up_tap4[ARSR2P_PHASE_NUM][ARSR2P_TAP4] = {
	{ -3, 254, 6, -1},
	{ -9, 255, 13, -3},
	{ -18, 254, 27, -7},
	{ -23, 245, 44, -10},
	{ -27, 233, 64, -14},
	{ -29, 218, 85, -18},
	{ -29, 198, 108, -21},
	{ -29, 177, 132, -24},
	{ -27, 155, 155, -27}
};

/* c0, c1, c2, c3 */
static const int g_coef_auv_scl_down_tap4[ARSR2P_PHASE_NUM][ARSR2P_TAP4] = {
	{ 31, 194, 31, 0},
	{ 23, 206, 44, -17},
	{ 14, 203, 57, -18},
	{ 6, 198, 70, -18},
	{ 0, 190, 85, -19},
	{ -5, 180, 99, -18},
	{ -10, 170, 114, -18},
	{ -13, 157, 129, -17},
	{ -15, 143, 143, -15}
};

/* c0, c1, c2, c3, c4, c5 */
static const int g_coef_y_scl_up_tap6[ARSR2P_PHASE_NUM][ARSR2P_TAP6] = {
	{ 0, -3, 254, 6, -1, 0},
	{ 4, -12, 252, 15, -5, 2},
	{ 7, -22, 245, 31, -9, 4},
	{ 10, -29, 234, 49, -14, 6},
	{ 12, -34, 221, 68, -19, 8},
	{ 13, -37, 206, 88, -24, 10},
	{ 14, -38, 189, 108, -29, 12},
	{ 14, -38, 170, 130, -33, 13},
	{ 14, -36, 150, 150, -36, 14}
};

static const int g_coef_y_scl_down_tap6[ARSR2P_PHASE_NUM][ARSR2P_TAP6] = {
	{ -22, 43, 214, 43, -22, 0},
	{ -18, 29, 205, 53, -23, 10},
	{ -16, 18, 203, 67, -25, 9},
	{ -13, 9, 198, 80, -26, 8},
	{ -10, 0, 191, 95, -27, 7},
	{ -7, -7, 182, 109, -27, 6},
	{ -5, -14, 174, 124, -27, 4},
	{ -2, -18, 162, 137, -25, 2},
	{ 0, -22, 150, 150, -22, 0}
};

struct dss_arsr2p_pos_size {
	int ih_inc;
	int iv_inc;
	int ih_left;  /* input left acc */
	int ih_right;  /* input end position */
	int iv_top;  /* input top position */
	int iv_bottom;  /* input bottom position */
	int uv_offset;
	int src_width;
	int src_height;
	int dst_whole_width;
	int outph_left;   /* output left acc */
	int outph_right;  /* output end position */
	int outpv_bottom;  /* output bottom position */
};

struct dss_arsr_mode {
	bool nointplen;  /* bit8 */
	bool prescaleren;  /* bit7 */
	bool nearest_en;  /* bit6 */
	bool diintpl_en;  /* bit5 */
	bool textureanalyhsisen_en;  /* bit4 */
	bool arsr2p_bypass;  /* bit0 */
};

struct dss_arsr2p_scl_flag {
	bool en_hscl;
	bool en_vscl;
	bool hscldown_flag;
	bool rdma_stretch_enable;
};

struct arsr_scl_para {
	const int **scl_down;
	const int **scl_up;
	int row;
	int col;
};

/*******************************************************************************
 * DSS ARSR
 */

static int hisi_dss_arsr_post_write_coefs(
	struct dpu_fb_data_type *dpufd, char __iomem *addr,
	const int **p, int row, int col)
{
	int coef_value = 0;
	int coef_num;
	int i;
	int j;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}
	if (!addr) {
		DPU_FB_ERR("addr is NULL\n");
		return -EINVAL;
	}

	if ((row != ARSR2P_PHASE_NUM) || ((col != ARSR2P_TAP4) && (col != ARSR2P_TAP6))) {
		DPU_FB_ERR("arsr1p filter coefficients is err, arsr1p_phase_num = %d, arsr1p_tap_num = %d\n",
			row, col);
		return -EINVAL;
	}

	/* The follow code get one by one array value to calculate coef_value
	 * It contains lots of fixed numbers
	 */
	coef_num = (col == ARSR2P_TAP4 ? 2 : 3);
	for (i = 0; i < row; i++) {
		for (j = 0; j < 2; j++) {
			if (coef_num == 2)
				coef_value = (*((int *)p + i * col + j * coef_num) & 0x1FF) |
					((*((int *)p + i * col + j * coef_num + 1) & 0x1FF) << 9);
			else
				coef_value = (*((int *)p + i * col + j * coef_num) & 0x1FF) |
					((*((int *)p + i * col + j * coef_num + 1) & 0x1FF) << 9) |
					((*((int *)p + i * col + j * coef_num + 2) & 0x1FF) << 18);
			set_reg(addr + 0x8 * i + j * 0x4, coef_value, 32, 0);
		}
	}

	return 0;
}

static void hisi_dss_arsr_post_write_config_coefs(
	struct dpu_fb_data_type *dpufd, char __iomem *addr, struct arsr_scl_para *scl_tap)
{
	int ret;

	ret = hisi_dss_arsr_post_write_coefs(dpufd, addr, scl_tap->scl_down, scl_tap->row, scl_tap->col);
	if (ret < 0) {
		DPU_FB_ERR("Error to write arsr post COEF_SCL_DOWN coefficients.\n");
		return;
	}

	ret = hisi_dss_arsr_post_write_coefs(dpufd, addr + ARSR2P_SCL_UP_OFFSET,
		scl_tap->scl_up, scl_tap->row, scl_tap->col);
	if (ret < 0) {
		DPU_FB_ERR("Error to write arsr post COEF_SCL_UP coefficients.\n");
		return;
	}
}

void hisi_dss_arsr_post_coef_on(struct dpu_fb_data_type *dpufd)
{
	char __iomem *coefy_v = NULL;
	char __iomem *coefa_v = NULL;
	char __iomem *coefuv_v = NULL;
	struct arsr_scl_para scl_tap4 = { (const int **)g_coef_auv_scl_down_tap4, (const int **)g_coef_auv_scl_up_tap4,
		ARSR2P_PHASE_NUM, ARSR2P_TAP4 };
	struct arsr_scl_para scl_tap6 = { (const int **)g_coef_y_scl_down_tap6, (const int **)g_coef_y_scl_up_tap6,
		ARSR2P_PHASE_NUM, ARSR2P_TAP6 };
	uint32_t arsr_post_lut_base;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	arsr_post_lut_base = ARSR_POST_LUT_OFFSET;
	coefy_v = dpufd->dss_base + arsr_post_lut_base + ARSR2P_LUT_COEFY_V_OFFSET;
	coefa_v = dpufd->dss_base + arsr_post_lut_base + ARSR2P_LUT_COEFA_V_OFFSET;
	coefuv_v = dpufd->dss_base + arsr_post_lut_base + ARSR2P_LUT_COEFUV_V_OFFSET;

	/* COEFY_V COEFY_H */
	hisi_dss_arsr_post_write_config_coefs(dpufd, coefy_v, &scl_tap6);
	hisi_dss_arsr_post_write_config_coefs(dpufd, coefy_v + ARSR2P_COEF_H0_OFFSET, &scl_tap6);
	hisi_dss_arsr_post_write_config_coefs(dpufd, coefy_v + ARSR2P_COEF_H1_OFFSET, &scl_tap6);

	/* COEFA_V COEFA_H */
	hisi_dss_arsr_post_write_config_coefs(dpufd, coefa_v, &scl_tap4);
	hisi_dss_arsr_post_write_config_coefs(dpufd, coefa_v + ARSR2P_COEF_H0_OFFSET, &scl_tap4);
	hisi_dss_arsr_post_write_config_coefs(dpufd, coefa_v + ARSR2P_COEF_H1_OFFSET, &scl_tap4);

	/* COEFUV_V COEFUV_H */
	hisi_dss_arsr_post_write_config_coefs(dpufd, coefuv_v, &scl_tap4);
	hisi_dss_arsr_post_write_config_coefs(dpufd, coefuv_v + ARSR2P_COEF_H0_OFFSET, &scl_tap4);
	hisi_dss_arsr_post_write_config_coefs(dpufd, coefuv_v + ARSR2P_COEF_H1_OFFSET, &scl_tap4);
}

static int hisi_dss_arsr2p_write_coefs(struct dpu_fb_data_type *dpufd,
	bool enable_cmdlist, char __iomem *addr, const int **p, struct arsr_scl_para *scl_tap)
{
	int coef_value = 0;
	int coef_num = 0;
	int i;
	int j;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}
	if (!addr) {
		DPU_FB_ERR("addr is NULL\n");
		return -EINVAL;
	}

	if ((scl_tap->row != ARSR2P_PHASE_NUM) || ((scl_tap->col != ARSR2P_TAP4) && (scl_tap->col != ARSR2P_TAP6))) {
		DPU_FB_ERR("arsr2p filter coefficients is err, arsr2p_phase_num = %d, arsr2p_tap_num = %d\n",
			scl_tap->row, scl_tap->col);
		return -EINVAL;
	}

	/* The follow code get one by one array value to calculate coef_value
	 * It contains lots of fixed numbers
	 */
	coef_num = (scl_tap->col == ARSR2P_TAP4 ? 2 : 3);

	for (i = 0; i < scl_tap->row; i++) {
		for (j = 0; j < 2; j++) {
			if (coef_num == 2)
				coef_value = (*((int *)p + i * scl_tap->col + j * coef_num) & 0x1FF) |
					((*((int *)p + i * scl_tap->col + j * coef_num + 1)  & 0x1FF) << 9);
			else
				coef_value = (*((int *)p + i * scl_tap->col + j * coef_num) & 0x1FF) |
					((*((int *)p + i * scl_tap->col + j * coef_num + 1)  & 0x1FF) << 9) |
					((*((int *)p + i * scl_tap->col + j * coef_num + 2)  & 0x1FF) << 18);

			if (enable_cmdlist)
				dpufd->set_reg(dpufd, addr + 0x8 * i + j * 0x4, coef_value, 32, 0);
			else
				set_reg(addr + 0x8 * i + j * 0x4, coef_value, 32, 0);
		}
	}

	return 0;
}

static void hisi_dss_arsr2p_write_config_coefs(struct dpu_fb_data_type *dpufd,
	bool enable_cmdlist, char __iomem *addr, struct arsr_scl_para *scl_tap)
{
	int ret;

	ret = hisi_dss_arsr2p_write_coefs(dpufd, enable_cmdlist, addr, scl_tap->scl_down, scl_tap);
	if (ret < 0) {
		DPU_FB_ERR("Error to write COEF_SCL_DOWN coefficients.\n");
		return;
	}

	ret = hisi_dss_arsr2p_write_coefs(dpufd, enable_cmdlist, addr + ARSR2P_SCL_UP_OFFSET,
		scl_tap->scl_up, scl_tap);
	if (ret < 0) {
		DPU_FB_ERR("Error to write COEF_SCL_UP coefficients.\n");
		return;
	}

}

static void arsr_pre_para_init(struct arsr2p_info *arsr_para)
{
	if (!arsr_para)
		return;

	/* The follow code from chip protocol, It contains lots of fixed numbers */
	arsr_para->skin_thres_y = (600 << 20) | (332 << 10) | 300;  /* 0x2585312C */
	arsr_para->skin_thres_u = (452 << 20) | (40 << 10) | 20;  /* 0x1C40A014 */
	arsr_para->skin_thres_v = (580 << 20) | (48 << 10) | 24;  /* 0x2440C018 */
	arsr_para->skin_cfg0 = (12 << 13) | 512;  /* 0x00018200 */
	arsr_para->skin_cfg1 = 819;  /* 0x00000333 */
	arsr_para->skin_cfg2 = 682;  /* 0x000002AA */
	arsr_para->shoot_cfg1 = (20 << 16) | 341;  /* 0x00140155 */
	arsr_para->shoot_cfg2 = (-80 & 0x7ff) | (16 << 16);  /* 0x001007B0 */
	arsr_para->shoot_cfg3 = 20;  /* 0x00000014 */
	arsr_para->sharp_cfg3 = HIGH16(0xA0) | LOW16(0x60);  /* 0x00A00060 */
	arsr_para->sharp_cfg4 = HIGH16(0x60) | LOW16(0x20);  /* 0x00600020 */
	arsr_para->sharp_cfg6 = HIGH16(0x4) | LOW16(0x8);  /* 0x00040008 */
	arsr_para->sharp_cfg7 = (6 << 8) | 10;  /* 0x0000060A */
	arsr_para->sharp_cfg8 = HIGH16(0xA0) | LOW16(0x10);  /* 0x00A00010 */

	arsr_para->sharp_level = 0x0020002;
	arsr_para->sharp_gain_low = 0x3C0078;
	arsr_para->sharp_gain_mid = 0x6400C8;
	arsr_para->sharp_gain_high = 0x5000A0;
	arsr_para->sharp_gainctrl_sloph_mf = 0x280;
	arsr_para->sharp_gainctrl_slopl_mf = 0x1400;
	arsr_para->sharp_gainctrl_sloph_hf = 0x140;
	arsr_para->sharp_gainctrl_slopl_hf = 0xA00;
	arsr_para->sharp_mf_lmt = 0x40;
	arsr_para->sharp_gain_mf = 0x12C012C;
	arsr_para->sharp_mf_b = 0;
	arsr_para->sharp_hf_lmt = 0x80;
	arsr_para->sharp_gain_hf = 0x104012C;
	arsr_para->sharp_hf_b = 0x1400;
	arsr_para->sharp_lf_ctrl = 0x100010;
	arsr_para->sharp_lf_var = 0x1800080;
	arsr_para->sharp_lf_ctrl_slop = 0;
	arsr_para->sharp_hf_select = 0;
	arsr_para->sharp_cfg2_h = 0x10000C0;
	arsr_para->sharp_cfg2_l = 0x200010;
	arsr_para->texture_analysis = 0x500040;
	arsr_para->intplshootctrl = 0x8;
}

void hisi_dss_arsr2p_init(const char __iomem *arsr2p_base, struct dss_arsr2p *s_arsr2p)
{
	if (!arsr2p_base) {
		DPU_FB_ERR("arsr2p_base is NULL\n");
		return;
	}

	if (!s_arsr2p) {
		DPU_FB_ERR("s_arsr2p is NULL\n");
		return;
	}

	memset(s_arsr2p, 0, sizeof(struct dss_arsr2p));

	s_arsr2p->arsr_input_width_height = inp32(arsr2p_base + ARSR2P_INPUT_WIDTH_HEIGHT);
	s_arsr2p->arsr_output_width_height = inp32(arsr2p_base + ARSR2P_OUTPUT_WIDTH_HEIGHT);
	s_arsr2p->ihleft = inp32(arsr2p_base + ARSR2P_IHLEFT);
	s_arsr2p->ihleft1 = inp32(arsr2p_base + ARSR2P_IHLEFT1);
	s_arsr2p->ihright = inp32(arsr2p_base + ARSR2P_IHRIGHT);
	s_arsr2p->ihright1 = inp32(arsr2p_base + ARSR2P_IHRIGHT1);
	s_arsr2p->ivtop = inp32(arsr2p_base + ARSR2P_IVTOP);
	s_arsr2p->ivbottom = inp32(arsr2p_base + ARSR2P_IVBOTTOM);
	s_arsr2p->ivbottom1 = inp32(arsr2p_base + ARSR2P_IVBOTTOM1);
	s_arsr2p->ihinc = inp32(arsr2p_base + ARSR2P_IHINC);
	s_arsr2p->ivinc = inp32(arsr2p_base + ARSR2P_IVINC);
	s_arsr2p->offset = inp32(arsr2p_base + ARSR2P_OFFSET);
	s_arsr2p->mode = inp32(arsr2p_base + ARSR2P_MODE);
	s_arsr2p->mode = ((s_arsr2p->mode) & 0x1FF) | 0x800;  /* dbg_en=4 for memory low power */

	arsr_pre_para_init(&s_arsr2p->arsr2p_effect);
	arsr_pre_para_init(&s_arsr2p->arsr2p_effect_scale_up);
	arsr_pre_para_init(&s_arsr2p->arsr2p_effect_scale_down);

}

void arsr2p_basic_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *arsr2p_base, struct dss_arsr2p *s_arsr2p)
{
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_INPUT_WIDTH_HEIGHT,
		s_arsr2p->arsr_input_width_height, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_OUTPUT_WIDTH_HEIGHT,
		s_arsr2p->arsr_output_width_height, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_IHLEFT, s_arsr2p->ihleft, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_IHLEFT1, s_arsr2p->ihleft1, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_IHRIGHT, s_arsr2p->ihright, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_IHRIGHT1, s_arsr2p->ihright1, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_IVTOP, s_arsr2p->ivtop, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_IVBOTTOM, s_arsr2p->ivbottom, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_IVBOTTOM1, s_arsr2p->ivbottom1, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_IHINC, s_arsr2p->ihinc, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_IVINC, s_arsr2p->ivinc, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_OFFSET, s_arsr2p->offset, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_MODE, s_arsr2p->mode, 32, 0);
	effect_debug_log(DEBUG_EFFECT_FRAME, "s_arsr2p->mode=0x%x\n", s_arsr2p->mode);
}

void arsr2p_func_param_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *arsr2p_base, struct dss_arsr2p *s_arsr2p)
{
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SKIN_THRES_Y,
		s_arsr2p->arsr2p_effect.skin_thres_y, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SKIN_THRES_U,
		s_arsr2p->arsr2p_effect.skin_thres_u, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SKIN_THRES_V,
		s_arsr2p->arsr2p_effect.skin_thres_v, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SKIN_CFG0,
		s_arsr2p->arsr2p_effect.skin_cfg0, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SKIN_CFG1,
		s_arsr2p->arsr2p_effect.skin_cfg1, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SKIN_CFG2,
		s_arsr2p->arsr2p_effect.skin_cfg2, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHOOT_CFG1,
		s_arsr2p->arsr2p_effect.shoot_cfg1, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHOOT_CFG2,
		s_arsr2p->arsr2p_effect.shoot_cfg2, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHOOT_CFG3,
		s_arsr2p->arsr2p_effect.shoot_cfg3, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_CFG3,
		s_arsr2p->arsr2p_effect.sharp_cfg3, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_CFG4,
		s_arsr2p->arsr2p_effect.sharp_cfg4, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_CFG6,
		s_arsr2p->arsr2p_effect.sharp_cfg6, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_CFG7,
		s_arsr2p->arsr2p_effect.sharp_cfg7, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_CFG8,
		s_arsr2p->arsr2p_effect.sharp_cfg8, 32, 0);
}

void arsr2p_sharp_control_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *arsr2p_base, struct dss_arsr2p *s_arsr2p)
{
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_LEVEL,
		s_arsr2p->arsr2p_effect.sharp_level, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_GAIN_LOW,
		s_arsr2p->arsr2p_effect.sharp_gain_low, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_GAIN_MID,
		s_arsr2p->arsr2p_effect.sharp_gain_mid, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_GAIN_HIGH,
		s_arsr2p->arsr2p_effect.sharp_gain_high, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_GAINCTRLSLOPH_MF,
		s_arsr2p->arsr2p_effect.sharp_gainctrl_sloph_mf, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_GAINCTRLSLOPL_MF,
		s_arsr2p->arsr2p_effect.sharp_gainctrl_slopl_mf, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_GAINCTRLSLOPH_HF,
		s_arsr2p->arsr2p_effect.sharp_gainctrl_sloph_hf, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_GAINCTRLSLOPL_HF,
		s_arsr2p->arsr2p_effect.sharp_gainctrl_slopl_hf, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_MF_LMT,
		s_arsr2p->arsr2p_effect.sharp_mf_lmt, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_GAIN_MF,
		s_arsr2p->arsr2p_effect.sharp_gain_mf, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_MF_B,
		s_arsr2p->arsr2p_effect.sharp_mf_b, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_HF_LMT,
		s_arsr2p->arsr2p_effect.sharp_hf_lmt, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_GAIN_HF,
		s_arsr2p->arsr2p_effect.sharp_gain_hf, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_HF_B,
		s_arsr2p->arsr2p_effect.sharp_hf_b, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_LF_CTRL,
		s_arsr2p->arsr2p_effect.sharp_lf_ctrl, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_LF_VAR,
		s_arsr2p->arsr2p_effect.sharp_lf_var, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_LF_CTRL_SLOP,
		s_arsr2p->arsr2p_effect.sharp_lf_ctrl_slop, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_HF_SELECT,
		s_arsr2p->arsr2p_effect.sharp_hf_select, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_CFG2_H,
		s_arsr2p->arsr2p_effect.sharp_cfg2_h, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_SHARP_CFG2_L,
		s_arsr2p->arsr2p_effect.sharp_cfg2_l, 32, 0);
}

void hisi_dss_arsr2p_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *arsr2p_base, struct dss_arsr2p *s_arsr2p)
{
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL!\n");
	dpu_check_and_no_retval(!arsr2p_base, ERR, "arsr2p_base is NULL!\n");
	dpu_check_and_no_retval(!s_arsr2p, ERR, "s_arsr2p is NULL!\n");

	arsr2p_basic_set_reg(dpufd, arsr2p_base, s_arsr2p);
	arsr2p_func_param_set_reg(dpufd, arsr2p_base, s_arsr2p);
	arsr2p_sharp_control_set_reg(dpufd, arsr2p_base, s_arsr2p);

	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_TEXTURW_ANALYSTS,
		s_arsr2p->arsr2p_effect.texture_analysis, 32, 0);
	dpufd->set_reg(dpufd, arsr2p_base + ARSR2P_INTPLSHOOTCTRL,
		s_arsr2p->arsr2p_effect.intplshootctrl, 32, 0);
}

void hisi_dss_arsr2p_coef_on(struct dpu_fb_data_type *dpufd,
	bool enable_cmdlist)
{
	uint32_t module_base;
	char __iomem *arsr2p_base = NULL;
	char __iomem *coefy_v = NULL;
	char __iomem *coefa_v = NULL;
	char __iomem *coefuv_v = NULL;
	struct arsr_scl_para scl_tap4 = { (const int **)g_coef_auv_scl_down_tap4, (const int **)g_coef_auv_scl_up_tap4,
		ARSR2P_PHASE_NUM, ARSR2P_TAP4 };
	struct arsr_scl_para scl_tap6 = { (const int **)g_coef_y_scl_down_tap6, (const int **)g_coef_y_scl_up_tap6,
		ARSR2P_PHASE_NUM, ARSR2P_TAP6 };

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	module_base = g_dss_module_base[DSS_RCHN_V0][MODULE_ARSR2P_LUT];
	coefy_v = dpufd->dss_base + module_base + ARSR2P_LUT_COEFY_V_OFFSET;
	coefa_v = dpufd->dss_base + module_base + ARSR2P_LUT_COEFA_V_OFFSET;
	coefuv_v = dpufd->dss_base + module_base + ARSR2P_LUT_COEFUV_V_OFFSET;
	arsr2p_base = dpufd->dss_base + g_dss_module_base[DSS_RCHN_V0][MODULE_ARSR2P];

	/* COEFY_V COEFY_H */
	hisi_dss_arsr2p_write_config_coefs(dpufd, enable_cmdlist, coefy_v, &scl_tap6);
	hisi_dss_arsr2p_write_config_coefs(dpufd, enable_cmdlist, coefy_v + ARSR2P_COEF_H0_OFFSET, &scl_tap6);
	hisi_dss_arsr2p_write_config_coefs(dpufd, enable_cmdlist, coefy_v + ARSR2P_COEF_H1_OFFSET, &scl_tap6);

	/* COEFA_V COEFA_H */
	hisi_dss_arsr2p_write_config_coefs(dpufd, enable_cmdlist, coefa_v, &scl_tap4);
	hisi_dss_arsr2p_write_config_coefs(dpufd, enable_cmdlist, coefa_v + ARSR2P_COEF_H0_OFFSET, &scl_tap4);
	hisi_dss_arsr2p_write_config_coefs(dpufd, enable_cmdlist, coefa_v + ARSR2P_COEF_H1_OFFSET, &scl_tap4);

	/* COEFUV_V COEFUV_H */
	hisi_dss_arsr2p_write_config_coefs(dpufd, enable_cmdlist, coefuv_v, &scl_tap4);
	hisi_dss_arsr2p_write_config_coefs(dpufd, enable_cmdlist, coefuv_v + ARSR2P_COEF_H0_OFFSET, &scl_tap4);
	hisi_dss_arsr2p_write_config_coefs(dpufd, enable_cmdlist, coefuv_v + ARSR2P_COEF_H1_OFFSET, &scl_tap4);
}

static int hisi_dss_arsr2p_config_check_width(
	dss_rect_t *dest_rect, int source_width, struct dss_arsr2p_scl_flag scl_flag,
	int source_height, uint32_t index)
{
	if (!dest_rect) {
		DPU_FB_ERR("dest_rect is NULL\n");
		return -EINVAL;
	}

	/* check arsr2p input and output width */
	if ((source_width < ARSR2P_MIN_INPUT) || (dest_rect->w < ARSR2P_MIN_INPUT) ||
		(source_width > ARSR2P_MAX_WIDTH) || (dest_rect->w > ARSR2P_DST_MAX_WIDTH)) {
		if ((!scl_flag.en_hscl) && (!scl_flag.en_vscl)) {
			DPU_FB_DEBUG("fb%d, src_w %d is not between [%d, %d]\n"
				"or dst_w is %d not between [%d, %d], but arsr2p bypass!\n",
				index, source_width, ARSR2P_MIN_INPUT, ARSR2P_MAX_WIDTH,
				dest_rect->w, ARSR2P_MIN_INPUT, ARSR2P_DST_MAX_WIDTH);
			return 0;
		}

		DPU_FB_ERR("fb%d, src_w %d is not between [%d, %d], or dst_w is %d not between [%d, %d]!\n",
			index, source_width, ARSR2P_MIN_INPUT, ARSR2P_MAX_WIDTH,
			dest_rect->w, ARSR2P_MIN_INPUT, ARSR2P_DST_MAX_WIDTH);
		return -EINVAL;
	}

	return 1;
}

static int hisi_dss_arsr2p_config_check_heigh(
	dss_rect_t *dest_rect, dss_rect_t *source_rect,
	dss_layer_t *layer, int source_width)
{
	if (!dest_rect) {
		DPU_FB_ERR("dest_rect is NULL\n");
		return -EINVAL;
	}

	if (!source_rect) {
		DPU_FB_ERR("source_rect is NULL\n");
		return -EINVAL;
	}

	if (!layer) {
		DPU_FB_ERR("layer is NULL\n");
		return -EINVAL;
	}
	if ((dest_rect->w > (source_width * ARSR2P_SCALE_MAX)) ||
		(source_width > (dest_rect->w * ARSR2P_SCALE_MAX))) {
		DPU_FB_ERR("width out of range, original_src_rec %d, %d, %d, %d, "
			"src_rect %d, %d, %d, %d, dst_rect %d, %d, %d, %d\n",
			layer->src_rect.x, layer->src_rect.y, source_width, layer->src_rect.h,
			source_rect->x, source_rect->y, source_width, source_rect->h,
			dest_rect->x, dest_rect->y, dest_rect->w, dest_rect->h);
		return -EINVAL;
	}

	/* check arsr2p input and output height */
	if ((source_rect->h > ARSR2P_MAX_HEIGHT) || (dest_rect->h > ARSR2P_MAX_HEIGHT)) {
		DPU_FB_ERR("src_rect.h %d or dst_rect.h %d is smaller than 16 or larger than 8192!\n",
			source_rect->h, dest_rect->h);
		return -EINVAL;
	}

	if ((dest_rect->h > (source_rect->h * ARSR2P_SCALE_MAX)) ||
		(source_rect->h > (dest_rect->h * ARSR2P_SCALE_MAX))) {
		DPU_FB_ERR("height out of range, original_src_rec %d, %d, %d, %d, "
			"src_rect%d, %d, %d, %d, dst_rect %d, %d, %d, %d.\n",
			layer->src_rect.x, layer->src_rect.y, layer->src_rect.w, layer->src_rect.h,
			source_rect->x, source_rect->y, source_rect->w, source_rect->h,
			dest_rect->x, dest_rect->y, dest_rect->w, dest_rect->h);
		return -EINVAL;
	}

	return 0;
}

static void hisi_dss_arsr2p_src_rect_align(dss_rect_t *src_rect, dss_layer_t *layer,
	dss_rect_t *aligned_rect, dss_block_info_t *pblock_info)
{
	src_rect->h = aligned_rect->h;

	if (g_debug_ovl_offline_composer) {
		DPU_FB_INFO("aligned_rect = %d, %d, %d, %d\n",
			aligned_rect->x, aligned_rect->y,
			aligned_rect->w, aligned_rect->h);
		DPU_FB_INFO("layer->src_rect = %d, %d, %d, %d\n",
			layer->src_rect.x, layer->src_rect.y,
			layer->src_rect.w, layer->src_rect.h);
		DPU_FB_INFO("layer->dst rect = %d, %d, %d, %d\n",
			layer->dst_rect.x, layer->dst_rect.y,
			layer->dst_rect.w, layer->dst_rect.h);
		DPU_FB_INFO("arsr2p_in_rect rect = %d, %d, %d, %d\n",
			pblock_info->arsr2p_in_rect.x, pblock_info->arsr2p_in_rect.y,
			pblock_info->arsr2p_in_rect.w, pblock_info->arsr2p_in_rect.h);
	}
}

static int hisi_dss_arsr2p_check_vhscl(struct dpu_fb_data_type *dpufd, struct dss_arsr_mode *arsr_mode,
	struct dss_arsr2p_scl_flag scl_flag)
{
	if ((!scl_flag.en_hscl) && (!scl_flag.en_vscl)) {
		if (dpufd->ov_req.wb_compose_type == DSS_WB_COMPOSE_COPYBIT)
			return -1;

		if (!scl_flag.hscldown_flag) {
			/* if only sharpness is needed, disable image interplo, enable textureanalyhsis */
			arsr_mode->nointplen = true;
			arsr_mode->textureanalyhsisen_en = true;
		}
	}

	return 0;
}

static void hisi_dss_arsr2p_cordinate_set(struct dss_arsr2p_pos_size *arsr2p_pos_size,
	dss_rect_t *src_rect, dss_rect_t *dst_rect, int ih_inc)
{
	int extraw;
	int extraw_left;
	int extraw_right = 0;

	dpu_check_and_no_retval((ih_inc == 0), ERR, "ih_inc is zero\n");

	/* ihleft1 = starto*ihinc - (strati <<16) */
	arsr2p_pos_size->outph_left = dst_rect->x * ih_inc - (src_rect->x * ARSR2P_INC_FACTOR);
	if (arsr2p_pos_size->outph_left < 0)
		arsr2p_pos_size->outph_left = 0;

	/* ihleft = ihleft1 - even(8*65536/ihinc) * ihinc */
	extraw = (8 * ARSR2P_INC_FACTOR) / ih_inc;
	extraw_left = (extraw % 2) ? (extraw + 1) : (extraw);
	arsr2p_pos_size->ih_left = arsr2p_pos_size->outph_left - extraw_left * ih_inc;
	if (arsr2p_pos_size->ih_left < 0)
		arsr2p_pos_size->ih_left = 0;

	/* ihright1 = endo * ihinc - (strati <<16); */
	arsr2p_pos_size->outph_right = (dst_rect->x + dst_rect->w - 1) * ih_inc -
		(src_rect->x * ARSR2P_INC_FACTOR);

	if (arsr2p_pos_size->dst_whole_width == dst_rect->w) {
		/* ihright = ihright1 + even(2*65536/ihinc) * ihinc */
		extraw = (2 * ARSR2P_INC_FACTOR) / ih_inc;
		extraw_right = (extraw % 2) ? (extraw + 1) : (extraw);
		arsr2p_pos_size->ih_right = arsr2p_pos_size->outph_right + extraw_right * ih_inc;

		extraw = (arsr2p_pos_size->dst_whole_width - 1) * ih_inc -
			(src_rect->x * ARSR2P_INC_FACTOR);  /* ihright is checked in every tile */

		if (arsr2p_pos_size->ih_right > extraw)
			arsr2p_pos_size->ih_right = extraw;
	} else {
		/* (endi-starti+1) << 16 - 1 */
		arsr2p_pos_size->ih_right = arsr2p_pos_size->src_width * ARSR2P_INC_FACTOR - 1;
	}
}

static void hisi_dss_arsr2p_hscl_compute(struct dss_arsr2p_pos_size *arsr2p_pos_size, dss_rect_t *src_rect,
	dss_rect_t *dst_rect, dss_block_info_t *pblock_info, struct dss_arsr2p_scl_flag *scl_flag)
{
	/* horizental scaler compute, offline subblock */
	if (pblock_info != NULL && pblock_info->h_ratio_arsr2p) {
		arsr2p_pos_size->ih_inc = pblock_info->h_ratio_arsr2p;
		arsr2p_pos_size->src_width = src_rect->w;
		arsr2p_pos_size->src_height = src_rect->h;
		arsr2p_pos_size->dst_whole_width = pblock_info->arsr2p_dst_w;
		src_rect->x = src_rect->x - pblock_info->arsr2p_src_x;
		src_rect->y = src_rect->y - pblock_info->arsr2p_src_y;
		dst_rect->x = dst_rect->x - pblock_info->arsr2p_dst_x;
		dst_rect->y = dst_rect->y - pblock_info->arsr2p_dst_y;

		if (pblock_info->both_vscfh_arsr2p_used)
			scl_flag->hscldown_flag = true;  /* horizental scaling down */

		if (scl_flag->rdma_stretch_enable)
			scl_flag->en_hscl = true;

		if (arsr2p_pos_size->ih_inc && arsr2p_pos_size->ih_inc != ARSR2P_INC_FACTOR)
			scl_flag->en_hscl = true;
	} else {
		/* horizental scaling down is not supported by arsr2p, set src_rect.w = dst_rect.w */
		if (src_rect->w > dst_rect->w) {
			arsr2p_pos_size->src_width = dst_rect->w;
			scl_flag->hscldown_flag = true;  /* horizental scaling down */
		} else {
			arsr2p_pos_size->src_width = src_rect->w;
		}
		arsr2p_pos_size->dst_whole_width = dst_rect->w;

		src_rect->x = 0;  /* set src rect to zero, in case */
		src_rect->y = 0;
		dst_rect->x = 0;  /* set dst rect to zero, in case */
		dst_rect->y = 0;

		if (arsr2p_pos_size->src_width != dst_rect->w)
			scl_flag->en_hscl = true;

		/* hisi_dss_check_layer_rect() will check dst_rect->w, not 0 */
		arsr2p_pos_size->ih_inc = (DSS_WIDTH(arsr2p_pos_size->src_width) * ARSR2P_INC_FACTOR +
			ARSR2P_INC_FACTOR - arsr2p_pos_size->ih_left) / dst_rect->w;
	}

	hisi_dss_arsr2p_cordinate_set(arsr2p_pos_size, src_rect, dst_rect, arsr2p_pos_size->ih_inc);
}

static bool hisi_dss_arsr2p_vscl_compute(struct dss_arsr2p_pos_size *arsr2p_pos_size,
	dss_rect_t *src_rect, dss_rect_t *dst_rect)
{
	bool en_vscl = false;

	/* vertical scaler compute */
	if (src_rect->h != dst_rect->h)
		en_vscl = true;

	if ((src_rect->h > dst_rect->h) && (DSS_HEIGHT(dst_rect->h) != 0)) {
		/* ivinc=(arsr_input_height*65536+65536/2-ivtop)/(arsr_output_height) */
		arsr2p_pos_size->iv_inc = (DSS_HEIGHT(src_rect->h) * ARSR2P_INC_FACTOR +
			ARSR2P_INC_FACTOR / 2 - arsr2p_pos_size->iv_top) / DSS_HEIGHT(dst_rect->h);
	} else {
		/* ivinc=(arsr_input_height*65536+65536-ivtop)/(arsr_output_height+1)
		 * hisi_dss_check_layer_rect() will check dst_rect->h, not 0
		 */
		if (dst_rect->h != 0)
			arsr2p_pos_size->iv_inc = (DSS_HEIGHT(src_rect->h) * ARSR2P_INC_FACTOR +
				ARSR2P_INC_FACTOR - arsr2p_pos_size->iv_top) / dst_rect->h;
	}

	/* ivbottom = arsr_output_height*ivinc + ivtop */
	arsr2p_pos_size->iv_bottom =
		DSS_HEIGHT(dst_rect->h) * arsr2p_pos_size->iv_inc + arsr2p_pos_size->iv_top;
	arsr2p_pos_size->outpv_bottom = arsr2p_pos_size->iv_bottom;

	return en_vscl;
}

static void hisi_dss_arsr2p_mode_config(struct dss_arsr2p_pos_size *arsr2p_pos_size,
	struct dss_arsr_mode *arsr_mode, dss_block_info_t *pblock_info, struct dss_arsr2p_scl_flag scl_flag
)
{
	arsr_mode->arsr2p_bypass = false;
	if (scl_flag.hscldown_flag) {  /* horizental scale down */
		arsr_mode->prescaleren = true;
		return;
	}

	if (!scl_flag.en_hscl && (arsr2p_pos_size->iv_inc >= 2 * ARSR2P_INC_FACTOR) && !pblock_info->h_ratio_arsr2p) {
		/* only vertical scale down, enable nearest scaling down, disable sharp in non-block scene */
		arsr_mode->nearest_en = true;
		return;
	}

	if ((!scl_flag.en_hscl) && (!scl_flag.en_vscl))
		return;

	arsr_mode->diintpl_en = true;
	/* imageintpl_dis = true; */
	arsr_mode->textureanalyhsisen_en = true;
}

static void hisi_dss_arsr2p_set_param(struct dss_arsr2p *arsr2p, struct dss_arsr2p_pos_size *arsr2p_pos_size,
	struct dss_arsr_mode *arsr_mode, dss_rect_t *src_rect, dss_rect_t *dst_rect)
{
	arsr2p->arsr_input_width_height = set_bits32(
		arsr2p->arsr_input_width_height, DSS_HEIGHT(src_rect->h), 13, 0);
	arsr2p->arsr_input_width_height = set_bits32(
		arsr2p->arsr_input_width_height, DSS_WIDTH(arsr2p_pos_size->src_width), 13, 16);
	arsr2p->arsr_output_width_height = set_bits32(
		arsr2p->arsr_output_width_height, DSS_HEIGHT(dst_rect->h), 13, 0);
	arsr2p->arsr_output_width_height = set_bits32(
		arsr2p->arsr_output_width_height, DSS_WIDTH(dst_rect->w), 13, 16);
	arsr2p->ihleft = set_bits32(arsr2p->ihleft, arsr2p_pos_size->ih_left, 29, 0);
	arsr2p->ihright = set_bits32(arsr2p->ihright, arsr2p_pos_size->ih_right, 29, 0);
	arsr2p->ivtop = set_bits32(arsr2p->ivtop, arsr2p_pos_size->iv_top, 29, 0);
	arsr2p->ivbottom = set_bits32(arsr2p->ivbottom, arsr2p_pos_size->iv_bottom, 29, 0);
	arsr2p->ihinc = set_bits32(arsr2p->ihinc, arsr2p_pos_size->ih_inc, 22, 0);
	arsr2p->ivinc = set_bits32(arsr2p->ivinc, arsr2p_pos_size->iv_inc, 22, 0);
	arsr2p->offset = set_bits32(arsr2p->offset, arsr2p_pos_size->uv_offset, 22, 0);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr_mode->arsr2p_bypass, 1, 0);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr2p->arsr2p_effect.sharp_enable, 1, 1);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr2p->arsr2p_effect.shoot_enable, 1, 2);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr2p->arsr2p_effect.skin_enable, 1, 3);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr_mode->textureanalyhsisen_en, 1, 4);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr_mode->diintpl_en, 1, 5);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr_mode->nearest_en, 1, 6);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr_mode->prescaleren, 1, 7);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr_mode->nointplen, 1, 8);

	arsr2p->ihleft1 = set_bits32(arsr2p->ihleft1, arsr2p_pos_size->outph_left, 29, 0);
	arsr2p->ihright1 = set_bits32(arsr2p->ihright1, arsr2p_pos_size->outph_right, 29, 0);
	arsr2p->ivbottom1 = set_bits32(arsr2p->ivbottom1, arsr2p_pos_size->outpv_bottom, 29, 0);
}

int hisi_dss_arsr2p_config(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer, dss_rect_t *aligned_rect, bool rdma_stretch_enable)
{
	struct dss_arsr2p *arsr2p = NULL;
	dss_block_info_t *pblock_info = NULL;
	struct dss_arsr2p_pos_size arsr2p_pos_size = {0};
	struct dss_arsr_mode arsr_mode = {0};

	struct dss_arsr2p_scl_flag scl_flag = { false, false, false, rdma_stretch_enable };
	dss_rect_t src_rect;
	dss_rect_t dst_rect;
	int ret;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	dpu_check_and_return(!layer, -EINVAL, ERR, "layer is NULL\n");

	if (layer->chn_idx != DSS_RCHN_V0)
		return 0;

	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		return 0;

	if ((dpufd->index == PRIMARY_PANEL_IDX) && (layer->need_cap & CAP_ROT) && aligned_rect)
		src_rect = *aligned_rect;
	else
		src_rect = layer->src_rect;

	dst_rect = layer->dst_rect;
	pblock_info = &(layer->block_info);

	if (pblock_info && pblock_info->h_ratio_arsr2p)
		/* src_rect = arsr2p_in_rect when both arsr2p and vscfh are extended */
		src_rect = pblock_info->arsr2p_in_rect;

	/* if vertical ratio of scaling down is larger than or equal to 2,
	 * set src_rect height to aligned rect height
	 */
	if (aligned_rect)
		hisi_dss_arsr2p_src_rect_align(&src_rect, layer, aligned_rect, pblock_info);

	hisi_dss_arsr2p_hscl_compute(&arsr2p_pos_size, &src_rect, &dst_rect, pblock_info, &scl_flag);

	scl_flag.en_vscl = hisi_dss_arsr2p_vscl_compute(&arsr2p_pos_size, &src_rect, &dst_rect);

	ret = hisi_dss_arsr2p_check_vhscl(dpufd, &arsr_mode, scl_flag);
	if (ret)
		return 0;

	arsr2p = &(dpufd->dss_module.arsr2p[layer->chn_idx]);
	dpufd->dss_module.arsr2p_used[layer->chn_idx] = 1;

	ret = hisi_dss_arsr2p_config_check_width(&dst_rect, arsr2p_pos_size.src_width, scl_flag,
		arsr2p_pos_size.src_height, dpufd->index);
	if (ret <= 0)
		return ret;

	ret = hisi_dss_arsr2p_config_check_heigh(&dst_rect, &src_rect, layer, arsr2p_pos_size.src_width);
	if (ret < 0)
		return ret;

	hisi_dss_arsr2p_mode_config(&arsr2p_pos_size, &arsr_mode, pblock_info, scl_flag);

	/* config the effect parameters as long as arsr2p is used */
	hisi_effect_arsr2p_config(&(arsr2p->arsr2p_effect), arsr2p_pos_size.ih_inc, arsr2p_pos_size.iv_inc);
	dpufd->dss_module.arsr2p_effect_used[layer->chn_idx] = 1;

	hisi_dss_arsr2p_set_param(arsr2p, &arsr2p_pos_size, &arsr_mode, &src_rect, &dst_rect);

	return 0;
}

