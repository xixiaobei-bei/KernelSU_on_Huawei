/* Copyright (c) 2012-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_dpe_utils.h"
#include "dsc/dsc_algorithm_manager.h"
#include "dsc/utilities/hisi_debug.h"
#include <linux/sysctl.h>

static unsigned int g_comform_value;
static unsigned int g_acm_state;
static unsigned int g_gmp_state;
static unsigned int g_led_rg_csc_value[9];
static unsigned int g_is_led_rg_csc_set;
unsigned int g_led_rg_para1 = 7;
unsigned int g_led_rg_para2 = 30983;
/*lint -esym(552,g_dpp_cmdlist_delay)*/
int8_t g_dpp_cmdlist_delay;

#define GMP_CNT_COFE 4913  /* 17*17*17 */
#define XCC_CNT_COFE 12
#define COLOR_RECTIFY_DEFAULT 0x8000
#define CSC_VALUE_MIN_LEN 9

/* Function: set reg depends on platforms'capacity,
 *	some platform don't support some modules
 * Parameters:
 *	@Input:
 *		dss_base: the dss reg base addr
 *		module_offset: the module offset addr in dss
 *		reg_offset: the register offset addr in module
 *		value: the value you want to set
 *	@output:
 *		None
 *	Modified:2019.11.01 Created
 */
void dpe_set_acm_state(struct dpu_fb_data_type *dpufd)
{
	(void *)dpufd;
}

ssize_t dpe_show_acm_state(char *buf)
{
	ssize_t ret;

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	ret = snprintf(buf, PAGE_SIZE, "g_acm_state = %d\n", g_acm_state);

	return ret;
}

void acm_set_lut(char __iomem *address, uint32_t table[], uint32_t size)
{
	(void *)address;
	(void *)table;
	(void)size;
}

void acm_set_lut_hue(char __iomem *address, uint32_t table[], uint32_t size)
{
	(void *)address;
	(void *)table;
	(void)size;
}

void dpe_update_g_acm_state(unsigned int value)
{
	(void)value;
}

void dpe_store_ct_csc_value(struct dpu_fb_data_type *dpufd, unsigned int csc_value[], unsigned int len)
{
	struct dpu_panel_info *pinfo = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (len < CSC_VALUE_MIN_LEN) {
		DPU_FB_ERR("csc_value len is too short\n");
		return;
	}

	pinfo = &(dpufd->panel_info);

	if (pinfo->xcc_support == 0 || pinfo->xcc_table == NULL)
		return;

	pinfo->xcc_table[1] = csc_value[0];
	pinfo->xcc_table[2] = csc_value[1];
	pinfo->xcc_table[3] = csc_value[2];
	pinfo->xcc_table[5] = csc_value[3];
	pinfo->xcc_table[6] = csc_value[4];
	pinfo->xcc_table[7] = csc_value[5];
	pinfo->xcc_table[9] = csc_value[6];
	pinfo->xcc_table[10] = csc_value[7];
	pinfo->xcc_table[11] = csc_value[8];
}

static uint32_t get_color_rectify_rgb(uint32_t color_rectify_rgb, uint8_t color_rectify_support)
{
	uint32_t rgb = COLOR_RECTIFY_DEFAULT;

	if (color_rectify_support && color_rectify_rgb && (color_rectify_rgb <= COLOR_RECTIFY_DEFAULT))
		rgb = color_rectify_rgb;

	return rgb;
}

int dpe_set_ct_csc_value(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	char __iomem *xcc_base = NULL;
	/* color temp rectify rgb default valve */
	uint32_t color_temp_rectify_r;
	uint32_t color_temp_rectify_g;
	uint32_t color_temp_rectify_b;

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is nullptr!\n");

	pinfo = &(dpufd->panel_info);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		xcc_base = dpufd->dss_base + DSS_DPP_XCC_OFFSET;
	} else {
		DPU_FB_ERR("fb%d, not support!", dpufd->index);
		return -1;
	}

	color_temp_rectify_r = get_color_rectify_rgb(pinfo->color_temp_rectify_R, pinfo->color_temp_rectify_support);
	color_temp_rectify_g = get_color_rectify_rgb(pinfo->color_temp_rectify_G, pinfo->color_temp_rectify_support);
	color_temp_rectify_b = get_color_rectify_rgb(pinfo->color_temp_rectify_B, pinfo->color_temp_rectify_support);

	/* XCC */
	if (pinfo->xcc_support == 1) {
		/* XCC matrix */
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1] *
				g_led_rg_csc_value[0] / COLOR_RECTIFY_DEFAULT *
				color_temp_rectify_r / COLOR_RECTIFY_DEFAULT);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6] *
				g_led_rg_csc_value[4] / COLOR_RECTIFY_DEFAULT *
				color_temp_rectify_g / COLOR_RECTIFY_DEFAULT);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11] *
				g_led_rg_csc_value[8] / COLOR_RECTIFY_DEFAULT *
				discount_coefficient(g_comform_value) / CHANGE_MAX *
				color_temp_rectify_b / COLOR_RECTIFY_DEFAULT);
			dpufd->color_temperature_flag = 2;  /* display effect flag */
		}
	}

	return 0;
}

ssize_t dpe_show_ct_csc_value(struct dpu_fb_data_type *dpufd, char *buf)
{
	struct dpu_panel_info *pinfo = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	pinfo = &(dpufd->panel_info);

	if (pinfo->xcc_support == 0 || pinfo->xcc_table == NULL)
		return 0;

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		pinfo->xcc_table[1], pinfo->xcc_table[2], pinfo->xcc_table[3],
		pinfo->xcc_table[5], pinfo->xcc_table[6], pinfo->xcc_table[7],
		pinfo->xcc_table[9], pinfo->xcc_table[10], pinfo->xcc_table[11]);
}

int dpe_set_xcc_csc_value(struct dpu_fb_data_type *dpufd)
{
	(void *)dpufd;

	return 0;
}

int dpe_set_comform_ct_csc_value(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	char __iomem *xcc_base = NULL;
	uint32_t color_temp_rectify_r;
	uint32_t color_temp_rectify_g;
	uint32_t color_temp_rectify_b;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	pinfo = &(dpufd->panel_info);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		xcc_base = dpufd->dss_base + DSS_DPP_XCC_OFFSET;
	} else {
		DPU_FB_ERR("fb%d, not support!", dpufd->index);
		return -1;
	}

	color_temp_rectify_r = get_color_rectify_rgb(pinfo->color_temp_rectify_R, pinfo->color_temp_rectify_support);
	color_temp_rectify_g = get_color_rectify_rgb(pinfo->color_temp_rectify_G, pinfo->color_temp_rectify_support);
	color_temp_rectify_b = get_color_rectify_rgb(pinfo->color_temp_rectify_B, pinfo->color_temp_rectify_support);

	/* XCC */
	if (pinfo->xcc_support == 1) {
		/* XCC matrix */
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1] * g_led_rg_csc_value[0] /
				COLOR_RECTIFY_DEFAULT * color_temp_rectify_r / COLOR_RECTIFY_DEFAULT);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6] * g_led_rg_csc_value[4] /
				COLOR_RECTIFY_DEFAULT * color_temp_rectify_g / COLOR_RECTIFY_DEFAULT);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11] * g_led_rg_csc_value[8] /
				COLOR_RECTIFY_DEFAULT * discount_coefficient(g_comform_value) / CHANGE_MAX *
				color_temp_rectify_b / COLOR_RECTIFY_DEFAULT);
		}
	}

	return 0;
}

void dpe_init_led_rg_ct_csc_value(void)
{
	/* led rg csc default valve */
	g_led_rg_csc_value[0] = COLOR_RECTIFY_DEFAULT;
	g_led_rg_csc_value[1] = 0;
	g_led_rg_csc_value[2] = 0;
	g_led_rg_csc_value[3] = 0;
	g_led_rg_csc_value[4] = COLOR_RECTIFY_DEFAULT;
	g_led_rg_csc_value[5] = 0;
	g_led_rg_csc_value[6] = 0;
	g_led_rg_csc_value[7] = 0;
	g_led_rg_csc_value[8] = COLOR_RECTIFY_DEFAULT;
	g_is_led_rg_csc_set = 0;
}

void dpe_store_led_rg_ct_csc_value(unsigned int csc_value[], unsigned int len)
{
	if (len < CSC_VALUE_MIN_LEN) {
		DPU_FB_ERR("csc_value len is too short\n");
		return;
	}

	g_led_rg_csc_value[0] = csc_value[0];
	g_led_rg_csc_value[1] = csc_value[1];
	g_led_rg_csc_value[2] = csc_value[2];
	g_led_rg_csc_value[3] = csc_value[3];
	g_led_rg_csc_value[4] = csc_value[4];
	g_led_rg_csc_value[5] = csc_value[5];
	g_led_rg_csc_value[6] = csc_value[6];
	g_led_rg_csc_value[7] = csc_value[7];
	g_led_rg_csc_value[8] = csc_value[8];
	g_is_led_rg_csc_set = 1;
}

int dpe_set_led_rg_ct_csc_value(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	char __iomem *xcc_base = NULL;
	/* color_temp_rectify_rgb default valve */
	uint32_t color_temp_rectify_r;
	uint32_t color_temp_rectify_g;
	uint32_t color_temp_rectify_b;

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is nullptr!\n");

	pinfo = &(dpufd->panel_info);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		xcc_base = dpufd->dss_base + DSS_DPP_XCC_OFFSET;
	} else {
		DPU_FB_ERR("fb%d, not support!", dpufd->index);
		return -1;
	}

	color_temp_rectify_r = get_color_rectify_rgb(pinfo->color_temp_rectify_R, pinfo->color_temp_rectify_support);
	color_temp_rectify_g = get_color_rectify_rgb(pinfo->color_temp_rectify_G, pinfo->color_temp_rectify_support);
	color_temp_rectify_b = get_color_rectify_rgb(pinfo->color_temp_rectify_B, pinfo->color_temp_rectify_support);

	/* XCC */
	if (g_is_led_rg_csc_set == 1 && pinfo->xcc_support == 1) {
		DPU_FB_DEBUG("real set color temperature: g_is_led_rg_csc_set = %d, R = 0x%x, G = 0x%x, B = 0x%x .\n",
			g_is_led_rg_csc_set, g_led_rg_csc_value[0],
			g_led_rg_csc_value[4], g_led_rg_csc_value[8]);
		/* XCC matrix */
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1] * g_led_rg_csc_value[0] /
				COLOR_RECTIFY_DEFAULT * color_temp_rectify_r / COLOR_RECTIFY_DEFAULT);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6] * g_led_rg_csc_value[4] /
				COLOR_RECTIFY_DEFAULT * color_temp_rectify_g / COLOR_RECTIFY_DEFAULT);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11] * g_led_rg_csc_value[8] /
				COLOR_RECTIFY_DEFAULT * discount_coefficient(g_comform_value) / CHANGE_MAX *
				color_temp_rectify_b / COLOR_RECTIFY_DEFAULT);
		}
	}

	return 0;
}

ssize_t dpe_show_led_rg_ct_csc_value(char *buf)
{
	if (buf == NULL) {
		DPU_FB_ERR("buf, NUll pointer warning\n");
		return 0;
	}

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		g_led_rg_para1, g_led_rg_para2,
		g_led_rg_csc_value[0], g_led_rg_csc_value[1], g_led_rg_csc_value[2],
		g_led_rg_csc_value[3], g_led_rg_csc_value[4], g_led_rg_csc_value[5],
		g_led_rg_csc_value[6], g_led_rg_csc_value[7], g_led_rg_csc_value[8]);
}

ssize_t dpe_show_comform_ct_csc_value(struct dpu_fb_data_type *dpufd, char *buf)
{
	struct dpu_panel_info *pinfo = NULL;

	dpu_check_and_return(!dpufd || !buf, 0, ERR, "dpufd or buf NUll pointer warning\n");

	pinfo = &(dpufd->panel_info);
	if (pinfo->xcc_support == 0 || pinfo->xcc_table == NULL)
		return 0;

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d,g_comform_value = %d\n",
		pinfo->xcc_table[1], pinfo->xcc_table[2], pinfo->xcc_table[3],
		pinfo->xcc_table[5], pinfo->xcc_table[6], pinfo->xcc_table[7],
		pinfo->xcc_table[9], pinfo->xcc_table[10], pinfo->xcc_table[11],
		g_comform_value);
}

void dpe_update_g_comform_discount(unsigned int value)
{
	g_comform_value = value;
	DPU_FB_INFO("g_comform_value = %d\n", g_comform_value);
}

ssize_t dpe_show_cinema_value(struct dpu_fb_data_type *dpufd, char *buf)
{
	dpu_check_and_return((!dpufd || !buf), 0, ERR, "dpufd or buf is NULL\n");

	return snprintf(buf, PAGE_SIZE, "gamma type is = %d\n", dpufd->panel_info.gamma_type);
}

int dpe_set_cinema(struct dpu_fb_data_type *dpufd, unsigned int value)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd, NUll pointer warning.\n");
		return -1;
	}

	if (dpufd->panel_info.gamma_type == value) {
		DPU_FB_DEBUG("fb%d, cinema mode is already in %d!\n", dpufd->index, value);
		return 0;
	}

	dpufd->panel_info.gamma_type = value;

	return 0;
}

void dpe_update_g_gmp_state(unsigned int value)
{
	(void)value;
}

void dpe_set_gmp_state(struct dpu_fb_data_type *dpufd)
{
	(void *)dpufd;
}

ssize_t dpe_show_gmp_state(char *buf)
{
	ssize_t ret;

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	ret = snprintf(buf, PAGE_SIZE, "g_gmp_state = %d\n", g_gmp_state);

	return ret;
}

/* For some malfunctioning machines, after a deep sleep, the memory of the gama / degama
 * lut table exits shutdown slowly.
 * At this time, the software setting lut table will not take effect,. It will be random values in the hardware.
 * In order to avoid this kind of problem, once it is detected that the lut table does not take effect as expected,
 * software delay 60ms,
 * then set gama / degama lut table again. 60ms is the empirical value estimated by the software.
 */
#define LUT_CHECK_STEP 0x8
#define LUT_LOW_MASK 0xfff
#define LUT_SET_STEP 0x2
#define LUT_DELAY_TIME 60
static void dpe_check_gm_igm_lut(char __iomem *lut_base, uint32_t *lut_table, uint32_t lut_len)
{
	uint32_t i;
	char __iomem *addr = NULL;
	uint32_t value;
	uint32_t expect_value;
	bool check_succ = true;

	for (i = 0; i < lut_len - 1; i = i + LUT_CHECK_STEP) {
		addr = lut_base + i * LUT_SET_STEP;
		value = readl_relaxed(addr) & (LUT_LOW_MASK | (LUT_LOW_MASK << BIT(4)));
		expect_value = (lut_table[i] & LUT_LOW_MASK) | ((lut_table[i + 1] & LUT_LOW_MASK) << BIT(4));
		if (value != expect_value) {
			DPU_FB_INFO("[effect]check init fail! %pK 0x%x 0x%x", addr, value, expect_value);
			check_succ = false;
			break;
		}
	}

	if (!check_succ) {
		mdelay(LUT_DELAY_TIME);
		for (i = 0; i < lut_len; i = i + LUT_SET_STEP) {
			set_reg(lut_base + i * LUT_SET_STEP, lut_table[i], 12, 0);
			if (i != lut_len - 1)
				set_reg(lut_base + i * LUT_SET_STEP, lut_table[i + 1], 12, 16);
		}
		value = readl_relaxed(addr) & (LUT_LOW_MASK | (LUT_LOW_MASK << BIT(4)));
		DPU_FB_INFO("[effect]after init lut again! 0x%x 0x%x\n", value, expect_value);
	}
}

static void set_pinfo_param_degamma_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *degamma_lut_base, char __iomem *degamma_base)
{
	uint32_t i;
	uint32_t index;

	struct dpu_panel_info *pinfo = &(dpufd->panel_info);

	if (pinfo->igm_lut_table_len > 0 && pinfo->igm_lut_table_R &&
		pinfo->igm_lut_table_G && pinfo->igm_lut_table_B) {
		for (i = 0; i < pinfo->igm_lut_table_len / 2; i++) {
			index = i * 2;
			outp32(degamma_lut_base + (U_DEGAMA_R_COEF +  i * 4),
				pinfo->igm_lut_table_R[index] | pinfo->igm_lut_table_R[index + 1] << 16);
			outp32(degamma_lut_base + (U_DEGAMA_G_COEF +  i * 4),
				pinfo->igm_lut_table_G[index] | pinfo->igm_lut_table_G[index + 1] << 16);
			outp32(degamma_lut_base + (U_DEGAMA_B_COEF +  i * 4),
				pinfo->igm_lut_table_B[index] | pinfo->igm_lut_table_B[index + 1] << 16);
		}
		outp32(degamma_lut_base + U_DEGAMA_R_LAST_COEF,
			pinfo->igm_lut_table_R[pinfo->igm_lut_table_len - 1]);
		outp32(degamma_lut_base + U_DEGAMA_G_LAST_COEF,
			pinfo->igm_lut_table_G[pinfo->igm_lut_table_len - 1]);
		outp32(degamma_lut_base + U_DEGAMA_B_LAST_COEF,
			pinfo->igm_lut_table_B[pinfo->igm_lut_table_len - 1]);

		dpe_check_gm_igm_lut(degamma_lut_base + U_DEGAMA_R_COEF,
			pinfo->igm_lut_table_R, pinfo->igm_lut_table_len);
		dpe_check_gm_igm_lut(degamma_lut_base + U_DEGAMA_G_COEF,
			pinfo->igm_lut_table_G, pinfo->igm_lut_table_len);
		dpe_check_gm_igm_lut(degamma_lut_base + U_DEGAMA_B_COEF,
			pinfo->igm_lut_table_B, pinfo->igm_lut_table_len);

		DPU_FB_INFO("[effect]pinfo:R=%d,%d,%d,%d,%d,%d G=%d,%d,%d,%d,%d,%d B=%d,%d,%d,%d,%d,%d\n",
			pinfo->igm_lut_table_R[0], pinfo->igm_lut_table_R[1], pinfo->igm_lut_table_R[2],
			pinfo->igm_lut_table_R[3], pinfo->igm_lut_table_R[4], pinfo->igm_lut_table_R[5],
			pinfo->igm_lut_table_G[0], pinfo->igm_lut_table_G[1], pinfo->igm_lut_table_G[2],
			pinfo->igm_lut_table_G[3], pinfo->igm_lut_table_G[4], pinfo->igm_lut_table_G[5],
			pinfo->igm_lut_table_B[0], pinfo->igm_lut_table_B[1], pinfo->igm_lut_table_B[2],
			pinfo->igm_lut_table_B[3], pinfo->igm_lut_table_B[4], pinfo->igm_lut_table_B[5]);
	}
	set_reg(degamma_base + DEGAMA_EN, 1, 1, 0);
}

static void set_user_param_degamma_reg(char __iomem *degamma_lut_base,
	char __iomem *degamma_base, struct dpp_buf_maneger *dpp_buff_mngr, int32_t buff_sel, int disp_panel_id)
{
	uint32_t i;
	uint32_t degamma_enable;

	struct degamma_info *degamma = &(dpp_buff_mngr->buf_info_list[disp_panel_id][buff_sel].degamma);

	degamma_enable = degamma->degamma_enable;
	if (degamma_enable) {
		for (i = 0; i < LCP_IGM_LUT_LENGTH; i = i + 2) {
			set_reg(degamma_lut_base + (U_DEGAMA_R_COEF + i * 2),
				degamma->degamma_r_lut[i], 12, 0);
			if (i != LCP_IGM_LUT_LENGTH - 1)
				set_reg(degamma_lut_base + (U_DEGAMA_R_COEF + i * 2),
					degamma->degamma_r_lut[i + 1], 12, 16);

			set_reg(degamma_lut_base + (U_DEGAMA_G_COEF + i * 2),
				degamma->degamma_g_lut[i], 12, 0);
			if (i != LCP_IGM_LUT_LENGTH - 1)
				set_reg(degamma_lut_base + (U_DEGAMA_G_COEF + i * 2),
					degamma->degamma_g_lut[i + 1], 12, 16);

			set_reg(degamma_lut_base + (U_DEGAMA_B_COEF + i * 2),
				degamma->degamma_b_lut[i], 12, 0);
			if (i != LCP_IGM_LUT_LENGTH - 1)
				set_reg(degamma_lut_base + (U_DEGAMA_B_COEF + i * 2),
					degamma->degamma_b_lut[i + 1], 12, 16);
		}

		dpe_check_gm_igm_lut(degamma_lut_base + U_DEGAMA_R_COEF, degamma->degamma_r_lut, LCP_IGM_LUT_LENGTH);
		dpe_check_gm_igm_lut(degamma_lut_base + U_DEGAMA_G_COEF, degamma->degamma_g_lut, LCP_IGM_LUT_LENGTH);
		dpe_check_gm_igm_lut(degamma_lut_base + U_DEGAMA_B_COEF, degamma->degamma_b_lut, LCP_IGM_LUT_LENGTH);

		hisi_effect_print_degamma_lut_info(degamma);
	}
	set_reg(degamma_base + DEGAMA_EN, degamma->degamma_enable, 1, 0);
}

static void dpe_init_degamma(struct dpu_fb_data_type *dpufd,
	uint32_t config_ch, uint32_t buff_sel, int param_choice)
{
	struct dpu_panel_info *pinfo = &(dpufd->panel_info);
	struct dpp_buf_maneger *dpp_buff_mngr = &(dpufd->effect_info[pinfo->disp_panel_id].dpp_buf);
	char __iomem *degamma_lut_base = dpufd->dss_base + DSS_DPP_DEGAMMA_LUT_OFFSET + config_ch * 0x40000;
	char __iomem *degamma_base = dpufd->dss_base + DSS_DPP_DEGAMMA_OFFSET + config_ch * 0x40000;

	if (pinfo->gamma_support != true) {
		/* degama memory shutdown */
		outp32(degamma_base + DEGAMA_MEM_CTRL, 0x2); /* only support deep sleep */
		DPU_FB_INFO("[effect]degamma is not support!\n");
		return;
	}

	if (dpp_buff_mngr->buf_info_list[pinfo->disp_panel_id][buff_sel].degama_buf_save_status == DPP_BUF_INVALIED) {
		DPU_FB_INFO("[effect]user param is valid,choose pinfo param!\n");
		param_choice = 0;
	}

	/* choose default pinfo parameters */
	if (param_choice == 0)
		set_pinfo_param_degamma_reg(dpufd, degamma_lut_base, degamma_base);

	/* choose user parameters */
	if (param_choice == 1)
		set_user_param_degamma_reg(degamma_lut_base, degamma_base, dpp_buff_mngr, buff_sel, pinfo->disp_panel_id);
}

static uint32_t get_color_temp_rectify(struct dpu_panel_info *pinfo,
	uint32_t color_temp_rectify)
{
	if (pinfo->color_temp_rectify_support && color_temp_rectify && color_temp_rectify <= COLOR_RECTIFY_DEFAULT)
		return color_temp_rectify;

	return COLOR_RECTIFY_DEFAULT;  /* default color_temp_rectify valve */
}

static void update_default_xcc_param(char __iomem *xcc_base, struct dpu_panel_info *pinfo,
	uint32_t color_temp_rectify_r, uint32_t color_temp_rectify_g, uint32_t color_temp_rectify_b)
{
	if (pinfo->xcc_table_len == XCC_CNT_COFE && pinfo->xcc_table) {
		outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
		outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1] *
			g_led_rg_csc_value[0] / COLOR_RECTIFY_DEFAULT * color_temp_rectify_r / COLOR_RECTIFY_DEFAULT);
		outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
		outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
		outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
		outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
		outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6] *
			g_led_rg_csc_value[4] / COLOR_RECTIFY_DEFAULT * color_temp_rectify_g / COLOR_RECTIFY_DEFAULT);
		outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
		outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
		outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
		outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
		outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11] *
			g_led_rg_csc_value[8] / COLOR_RECTIFY_DEFAULT *
			discount_coefficient(g_comform_value) / CHANGE_MAX *
			color_temp_rectify_b / COLOR_RECTIFY_DEFAULT);
	}
	set_reg(xcc_base + XCC_EN, 1, 1, 0);
}

static void dpe_init_xcc(struct dpu_fb_data_type *dpufd,
	uint32_t config_ch, uint32_t buff_sel, int param_choice)
{
	uint32_t xcc_enable;
	struct xcc_info *xcc = NULL;
	uint32_t color_temp_rectify_r;
	uint32_t color_temp_rectify_g;
	uint32_t color_temp_rectify_b;
	uint32_t cnt;

	struct dpu_panel_info *pinfo = &(dpufd->panel_info);
	struct dpp_buf_maneger *dpp_buff_mngr = &(dpufd->effect_info[pinfo->disp_panel_id].dpp_buf);
	char __iomem *xcc_base = dpufd->dss_base + DSS_DPP_XCC_OFFSET + config_ch * 0x40000;

	color_temp_rectify_r = get_color_temp_rectify(pinfo, pinfo->color_temp_rectify_R);
	color_temp_rectify_g = get_color_temp_rectify(pinfo, pinfo->color_temp_rectify_G);
	color_temp_rectify_b = get_color_temp_rectify(pinfo, pinfo->color_temp_rectify_B);

	if (dpufd->effect_ctl.lcp_xcc_support != true) {
		DPU_FB_INFO("[effect]xcc is not support!\n");
		return;
	}

	if (dpp_buff_mngr->buf_info_list[pinfo->disp_panel_id][buff_sel].xcc_buf_save_status == DPP_BUF_INVALIED) {
		DPU_FB_INFO("[effect]user param is valid,choose pinfo param!\n");
		param_choice = 0;
	}

	if (param_choice == 0)
		update_default_xcc_param(xcc_base, pinfo, color_temp_rectify_r,
			color_temp_rectify_g, color_temp_rectify_b);

	if (param_choice == 1) {
		xcc = dpp_buff_mngr->buf_info_list[pinfo->disp_panel_id][buff_sel].xcc;
		xcc_enable = xcc->xcc_enable;
		if (xcc_enable) {
			for (cnt = 0; cnt < XCC_COEF_LEN; cnt++)
				set_reg(xcc_base + XCC_COEF_00 + cnt * 4, xcc->xcc_table[cnt], 17, 0);
		}
		set_reg(xcc_base + XCC_EN, xcc->xcc_enable, 1, 0);
	}
}

static void set_pinfo_param_gamma_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *gamma_lut_base, char __iomem *gama_base)
{
	uint32_t i;
	uint32_t index;

	struct dpu_panel_info *pinfo = &(dpufd->panel_info);

	if (pinfo->gamma_lut_table_len > 0 && pinfo->gamma_lut_table_R &&
		pinfo->gamma_lut_table_G && pinfo->gamma_lut_table_B) {
		for (i = 0; i < pinfo->gamma_lut_table_len / 2; i++) {
			index = i * 2;
			outp32(gamma_lut_base + (U_GAMA_R_COEF + i * 4),
				pinfo->gamma_lut_table_R[index] | pinfo->gamma_lut_table_R[index + 1] << 16);
			outp32(gamma_lut_base + (U_GAMA_G_COEF + i * 4),
				pinfo->gamma_lut_table_G[index] | pinfo->gamma_lut_table_G[index + 1] << 16);
			outp32(gamma_lut_base + (U_GAMA_B_COEF + i * 4),
				pinfo->gamma_lut_table_B[index] | pinfo->gamma_lut_table_B[index + 1] << 16);
		}
		outp32(gamma_lut_base + U_GAMA_R_LAST_COEF,
			pinfo->gamma_lut_table_R[pinfo->gamma_lut_table_len - 1]);
		outp32(gamma_lut_base + U_GAMA_G_LAST_COEF,
			pinfo->gamma_lut_table_G[pinfo->gamma_lut_table_len - 1]);
		outp32(gamma_lut_base + U_GAMA_B_LAST_COEF,
			pinfo->gamma_lut_table_B[pinfo->gamma_lut_table_len - 1]);

		dpe_check_gm_igm_lut(gamma_lut_base + U_GAMA_R_COEF, pinfo->gamma_lut_table_R,
			pinfo->gamma_lut_table_len);
		dpe_check_gm_igm_lut(gamma_lut_base + U_GAMA_G_COEF, pinfo->gamma_lut_table_G,
			pinfo->gamma_lut_table_len);
		dpe_check_gm_igm_lut(gamma_lut_base + U_GAMA_B_COEF, pinfo->gamma_lut_table_B,
			pinfo->gamma_lut_table_len);

		DPU_FB_INFO("[effect]pinfo:R=%d,%d,%d,%d,%d,%d G=%d,%d,%d,%d,%d,%d B=%d,%d,%d,%d,%d,%d\n",
			pinfo->gamma_lut_table_R[0], pinfo->gamma_lut_table_R[1],
			pinfo->gamma_lut_table_R[2], pinfo->gamma_lut_table_R[3],
			pinfo->gamma_lut_table_R[4], pinfo->gamma_lut_table_R[5],
			pinfo->gamma_lut_table_G[0], pinfo->gamma_lut_table_G[1],
			pinfo->gamma_lut_table_G[2], pinfo->gamma_lut_table_G[3],
			pinfo->gamma_lut_table_G[4], pinfo->gamma_lut_table_G[5],
			pinfo->gamma_lut_table_B[0], pinfo->gamma_lut_table_B[1],
			pinfo->gamma_lut_table_B[2], pinfo->gamma_lut_table_B[3],
			pinfo->gamma_lut_table_B[4], pinfo->gamma_lut_table_B[5]);
	}
	set_reg(gama_base + GAMA_EN, 1, 1, 0);
}

static void set_user_param_gamma_reg(struct dpu_fb_data_type *dpufd, char __iomem *gamma_lut_base,
	char __iomem *gama_base, uint32_t buff_sel)
{
	uint32_t i;
	uint32_t gama_enable;

	struct gama_info *gama = &(dpufd->effect_info[dpufd->panel_info.disp_panel_id].dpp_buf.buf_info_list[dpufd->panel_info.disp_panel_id][buff_sel].gama);

	gama_enable = gama->gama_enable;
	if (gama_enable) {
		for (i = 0; i < GAMA_LUT_LENGTH; i = i + 2) {
			set_reg(gamma_lut_base + (U_GAMA_R_COEF + i * 2), gama->gama_r_lut[i], 12, 0);
			if (i != GAMA_LUT_LENGTH - 1)
				set_reg(gamma_lut_base + (U_GAMA_R_COEF + i * 2), gama->gama_r_lut[i + 1], 12, 16);

			set_reg(gamma_lut_base + (U_GAMA_G_COEF + i * 2), gama->gama_g_lut[i], 12, 0);
			if (i != GAMA_LUT_LENGTH - 1)
				set_reg(gamma_lut_base + (U_GAMA_G_COEF + i * 2), gama->gama_g_lut[i + 1], 12, 16);

			set_reg(gamma_lut_base + (U_GAMA_B_COEF + i * 2), gama->gama_b_lut[i], 12, 0);
			if (i != GAMA_LUT_LENGTH - 1)
				set_reg(gamma_lut_base + (U_GAMA_B_COEF + i * 2), gama->gama_b_lut[i + 1], 12, 16);
		}

		dpe_check_gm_igm_lut(gamma_lut_base + U_GAMA_R_COEF, gama->gama_r_lut, GAMA_LUT_LENGTH);
		dpe_check_gm_igm_lut(gamma_lut_base + U_GAMA_G_COEF, gama->gama_g_lut, GAMA_LUT_LENGTH);
		dpe_check_gm_igm_lut(gamma_lut_base + U_GAMA_B_COEF, gama->gama_b_lut, GAMA_LUT_LENGTH);

		hisi_effect_print_gamma_lut_info(gama);
	}
	set_reg(gama_base + GAMA_EN, gama->gama_enable, 1, 0);
}

static void dpe_init_gamma(struct dpu_fb_data_type *dpufd,
	uint32_t config_ch, uint32_t buff_sel, int param_choice)
{
	struct dpu_panel_info *pinfo = &(dpufd->panel_info);
	char __iomem *gamma_lut_base = dpufd->dss_base + DSS_DPP_GAMA_LUT_OFFSET + config_ch * 0x40000;
	char __iomem *gama_base = dpufd->dss_base + DSS_DPP_GAMA_OFFSET + config_ch * 0x40000;
	struct dpp_buf_maneger *dpp_buff_mngr = &(dpufd->effect_info[pinfo->disp_panel_id].dpp_buf);

	if (pinfo->gamma_support != true) {
		/* degama memory shutdown */
		outp32(gama_base + DEGAMA_MEM_CTRL, 0x2); /* only support deep sleep */
		DPU_FB_INFO("[effect]gamma is not support!\n");
		return;
	}

	if (dpp_buff_mngr->buf_info_list[pinfo->disp_panel_id][buff_sel].gama_buf_save_status == DPP_BUF_INVALIED) {
		DPU_FB_INFO("[effect]user param is valid,choose pinfo param!\n");
		param_choice = 0;
	}

	if (param_choice == 0)
		set_pinfo_param_gamma_reg(dpufd, gamma_lut_base, gama_base);

	if (param_choice == 1)
		set_user_param_gamma_reg(dpufd, gamma_lut_base, gama_base, buff_sel);
}

static void dpe_init_post_xcc(struct dpu_fb_data_type *dpufd)
{
	char __iomem *post_xcc_base = dpufd->dss_base + DSS_DPP_POST_XCC_OFFSET;

	outp32(post_xcc_base + POST_XCC_COEF_00, g_post_xcc_table_temp[0]);
	outp32(post_xcc_base + POST_XCC_COEF_01, g_post_xcc_table_temp[1]);
	outp32(post_xcc_base + POST_XCC_COEF_02, g_post_xcc_table_temp[2]);
	outp32(post_xcc_base + POST_XCC_COEF_03, g_post_xcc_table_temp[3]);
	outp32(post_xcc_base + POST_XCC_COEF_10, g_post_xcc_table_temp[4]);
	outp32(post_xcc_base + POST_XCC_COEF_11, g_post_xcc_table_temp[5]);
	outp32(post_xcc_base + POST_XCC_COEF_12, g_post_xcc_table_temp[6]);
	outp32(post_xcc_base + POST_XCC_COEF_13, g_post_xcc_table_temp[7]);
	outp32(post_xcc_base + POST_XCC_COEF_20, g_post_xcc_table_temp[8]);
	outp32(post_xcc_base + POST_XCC_COEF_21, g_post_xcc_table_temp[9]);
	outp32(post_xcc_base + POST_XCC_COEF_22, g_post_xcc_table_temp[10]);
	outp32(post_xcc_base + POST_XCC_COEF_23, g_post_xcc_table_temp[11]);

	set_reg(post_xcc_base + POST_XCC_EN, 1, 1, 0);
}

static void update_default_gmp_param(char __iomem *gmp_lut_base, char __iomem *gmp_base)
{
	uint32_t i;

	for (i = 0; i < LCP_GMP_LUT_LENGTH; i++) {
		outp32(gmp_lut_base + i * 2 * 4, g_gmp_lut_table_low32bit_init[i]);
		outp32(gmp_lut_base + i * 2 * 4 + 4, g_gmp_lut_table_high4bit_init[i]);
	}
	set_reg(gmp_base + GMP_EN, 0, 1, 0);

	DPU_FB_INFO("[effect]pinfo:low32=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x high4=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
		g_gmp_lut_table_low32bit_init[0], g_gmp_lut_table_low32bit_init[500],
		g_gmp_lut_table_low32bit_init[1000], g_gmp_lut_table_low32bit_init[1500],
		g_gmp_lut_table_low32bit_init[2000], g_gmp_lut_table_low32bit_init[2500],
		g_gmp_lut_table_low32bit_init[3000], g_gmp_lut_table_low32bit_init[3500],
		g_gmp_lut_table_low32bit_init[4000], g_gmp_lut_table_high4bit_init[0],
		g_gmp_lut_table_high4bit_init[500], g_gmp_lut_table_high4bit_init[1000],
		g_gmp_lut_table_high4bit_init[1500], g_gmp_lut_table_high4bit_init[2000],
		g_gmp_lut_table_high4bit_init[2500], g_gmp_lut_table_high4bit_init[3000],
		g_gmp_lut_table_high4bit_init[3500], g_gmp_lut_table_high4bit_init[4000]);
}

static void update_user_gmp_param(struct dpu_fb_data_type *dpufd,
	char __iomem *gmp_lut_base, char __iomem *gmp_base, uint32_t buff_sel)
{
	uint32_t gmp_enable;
	uint32_t i;

	struct gmp_info *gmp = &(dpufd->effect_info[dpufd->panel_info.disp_panel_id].dpp_buf.buf_info_list[dpufd->panel_info.disp_panel_id][buff_sel].gmp);

	gmp_enable = (gmp->gmp_mode & GMP_ENABLE);
	if (gmp_enable) {
		for (i = 0; i < LCP_GMP_LUT_LENGTH; i++) {
			/* cppcheck-suppress */
			set_reg(gmp_lut_base + i * 2 * 4, gmp->gmp_lut_low32bit[i], 32, 0);
			/* cppcheck-suppress */
			set_reg(gmp_lut_base + i * 2 * 4 + 4, gmp->gmp_lut_high4bit[i], 4, 0);
		}

		hisi_effect_print_gmp_lut_info(gmp->gmp_lut_low32bit, gmp->gmp_lut_high4bit);
	}
	set_reg(gmp_base + GMP_EN, gmp_enable, 1, 0);
}

static void dpe_init_gmp(struct dpu_fb_data_type *dpufd,
	uint32_t config_ch, uint32_t buff_sel, int param_choice)
{
	struct dpu_panel_info *pinfo = &(dpufd->panel_info);
	struct dpp_buf_maneger *dpp_buff_mngr = &(dpufd->effect_info[pinfo->disp_panel_id].dpp_buf);
	char __iomem *gmp_lut_base = dpufd->dss_base + DSS_DPP_GMP_LUT_OFFSET + config_ch * 0x10000;
	char __iomem *gmp_base = dpufd->dss_base + DSS_DPP_GMP_OFFSET + config_ch * 0x40000;

	if (pinfo->gmp_support != true) {
		/* gmp memory shutdown */
		outp32(gmp_base + GMP_MEM_CTRL, 0x2); /* only support deep sleep */
		DPU_FB_INFO("[effect]gmp is not support!\n");
		return;
	}

	if (dpp_buff_mngr->buf_info_list[pinfo->disp_panel_id][buff_sel].gmp_buf_save_status == DPP_BUF_INVALIED) {
		DPU_FB_INFO("[effect]user param is valid,choose pinfo param!\n");
		param_choice = 0;
	}

	/* choose default pinfo parameters */
	if (param_choice == 0)
		update_default_gmp_param(gmp_lut_base, gmp_base);

	/* choose user parameters */
	if (param_choice == 1)
		update_user_gmp_param(dpufd, gmp_lut_base, gmp_base, buff_sel);
}

static int dump_start_status_info(struct dpu_fb_data_type *dpufd, uint32_t buff_sel,
	uint32_t buff_sel_roi, bool is_init)
{
	uint32_t work_channel;
	int disp_panel_id = dpufd->panel_info.disp_panel_id;

	struct dpp_buf_maneger *dpp_buff_mngr = &(dpufd->effect_info[disp_panel_id].dpp_buf);

	if (buff_sel >= DPP_BUF_MAX_COUNT) {
		DPU_FB_INFO("[effect] select buffer %d\n", buff_sel);
		return -EINVAL;
	}

	/* dump start status info  of software buffer and hardware channel */
	/* default read value is dpp0 */
	work_channel = (uint32_t)inp32(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DYN_SW_DEFAULT) & 0x1;
	DPU_FB_INFO(
		"[effect] inited=%d buf_sel=%d, buf_status:gmp=%d, gama=%d, degama=%d, "
		"xcc=%d, work_ch=%d, ch_status=%d,%d, buff_sel_roi=%d\n",
		is_init,
		buff_sel,
		dpp_buff_mngr->buf_info_list[disp_panel_id][buff_sel].gmp_buf_cfg_status,
		dpp_buff_mngr->buf_info_list[disp_panel_id][buff_sel].gama_buf_cfg_status,
		dpp_buff_mngr->buf_info_list[disp_panel_id][buff_sel].degama_buf_cfg_status,
		dpp_buff_mngr->buf_info_list[disp_panel_id][buff_sel].xcc_buf_cfg_status,
		work_channel,
		*(dpufd->effect_info[disp_panel_id].dpp_chn_status + work_channel),
		*(dpufd->effect_info[disp_panel_id].dpp_chn_status + ((~work_channel) & 0x1)),
		buff_sel_roi);

	if (buff_sel_roi < DPP_BUF_MAX_COUNT)
		DPU_FB_INFO("buf_status_roi:gmp=%d,gama=%d,degama=%d,xcc=%d\n",
		dpp_buff_mngr->buf_info_list[disp_panel_id][buff_sel_roi].gmp_buf_cfg_status,
		dpp_buff_mngr->buf_info_list[disp_panel_id][buff_sel_roi].gama_buf_cfg_status,
		dpp_buff_mngr->buf_info_list[disp_panel_id][buff_sel_roi].degama_buf_cfg_status,
		dpp_buff_mngr->buf_info_list[disp_panel_id][buff_sel_roi].xcc_buf_cfg_status);
	return 0;
}

void init_igm_gmp_xcc_gm(struct dpu_fb_data_type *dpufd)
{
	int ret;
	uint32_t buff_sel;
	uint32_t buff_sel_roi = DPP_BUF_MAX_COUNT;
	static bool is_init;
	uint32_t dpp_ch_idx;
	struct dpp_buf_maneger *dpp_buff_mngr = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return;
	}

	dpp_buff_mngr = &(dpufd->effect_info[dpufd->panel_info.disp_panel_id].dpp_buf);

	/* avoid the partial update */
	dpufd->display_effect_flag = 40;

	buff_sel = dpp_buff_mngr->latest_buf[dpufd->panel_info.disp_panel_id];

	DPU_FB_DEBUG("buff_sel=%u, latest_buf=%u\n", buff_sel, dpp_buff_mngr->latest_buf[dpufd->panel_info.disp_panel_id]);

	if (dpp_buff_mngr->buf_work_mode == DPP_BUF_WORK_MODE_BOTH_EFFECT)
		buff_sel_roi = DPP_EXTENAL_ROI_BUF;

	ret = dump_start_status_info(dpufd, buff_sel, buff_sel_roi, is_init);
	dpu_check_and_no_retval((ret != 0), ERR, "dump_start_status_info error.\n");

	if (is_init) {
		dpe_init_gmp(dpufd, 0, buff_sel, 1);
		dpe_init_xcc(dpufd, 0, buff_sel, 1);
		dpe_init_degamma(dpufd, 0, buff_sel, 1);
		dpe_init_gamma(dpufd, 0, buff_sel, 1);
		if (dpp_buff_mngr->buf_work_mode == DPP_BUF_WORK_MODE_BOTH_EFFECT) {
			dpe_init_gmp(dpufd, 1, buff_sel_roi, 1);
			dpe_init_xcc(dpufd, 1, buff_sel_roi, 1);
			dpe_init_degamma(dpufd, 1, buff_sel_roi, 1);
			dpe_init_gamma(dpufd, 1, buff_sel_roi, 1);
			dpufd->effect_info[dpufd->panel_info.disp_panel_id].dpp_buf.roi_new_updated = true;
			g_dpp_cmdlist_delay = CMDLIST_DELAY_STATUS_DISABLE;
		} else {
			dpe_init_gmp(dpufd, 1, buff_sel, 1);
			dpe_init_xcc(dpufd, 1, buff_sel, 1);
			dpe_init_degamma(dpufd, 1, buff_sel, 1);
			dpe_init_gamma(dpufd, 1, buff_sel, 1);
		}
	} else {
		for (dpp_ch_idx = 0; dpp_ch_idx < DPP_CHN_MAX_COUNT; dpp_ch_idx++) {
			dpe_init_gmp(dpufd, dpp_ch_idx, buff_sel, 0);
			dpe_init_xcc(dpufd, dpp_ch_idx, buff_sel, 0);
			dpe_init_degamma(dpufd, dpp_ch_idx, buff_sel, 0);
			dpe_init_gamma(dpufd, dpp_ch_idx, buff_sel, 0);
		}

		/* POST XCC pre matrix */
		if (dpufd->panel_info.post_xcc_support == 1)
			dpe_init_post_xcc(dpufd);

		is_init = true;
	}

}

