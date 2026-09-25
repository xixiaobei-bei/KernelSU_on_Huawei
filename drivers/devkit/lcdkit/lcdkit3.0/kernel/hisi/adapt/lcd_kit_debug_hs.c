/*
 * lcd_kit_debug_hs.c
 *
 * lcdkit debug function for lcd driver of hisi platform
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

#include "hisi_fb.h"
#include "lcd_kit_common.h"
#include "lcd_kit_dbg.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_power.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_dpd.h"
#include <base.h>

static bool dbg_panel_power_on(void);

static int dbg_fps_updt_support(int val)
{
	disp_info->fps.support = val;
	LCD_KIT_INFO("disp_info->fps.support = %d\n", disp_info->fps.support);
	return LCD_KIT_OK;
}

static int dbg_quickly_sleep_out_support(int val)
{
	disp_info->quickly_sleep_out.support = val;
	LCD_KIT_INFO("disp_info->quickly_sleep_out.support = %d\n",
		disp_info->quickly_sleep_out.support);
	return LCD_KIT_OK;
}

static int init_panel_info(struct hisi_fb_data_type *hisifd,
	struct hisi_panel_info **pinfo)
{
	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	*pinfo = &hisifd->panel_info;
	if (*pinfo == NULL) {
		LCD_KIT_ERR("*pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int dbg_blpwm_input_support(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->blpwm_input_disable = val;
	LCD_KIT_INFO("blpwm_input_disable = %d\n", pinfo->blpwm_input_disable);
	return LCD_KIT_OK;
}

static int dbg_dsi_upt_support(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->dsi_bit_clk_upt_support = val;
	LCD_KIT_INFO("dsi_upt_support = %d\n", pinfo->dsi_bit_clk_upt_support);
	return LCD_KIT_OK;
}

static int dbg_rgbw_support(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->rgbw_support = val;
	disp_info->rgbw.support = val;
	LCD_KIT_INFO("pinfo->rgbw_support = %d\n", pinfo->rgbw_support);
	return LCD_KIT_OK;
}

static int dbg_gamma_support(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->gamma_support = val;
	LCD_KIT_INFO("pinfo->gamma_support = %d\n", pinfo->gamma_support);
	return LCD_KIT_OK;
}

static int dbg_gmp_support(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->gmp_support = val;
	LCD_KIT_INFO("pinfo->gmp_support = %d\n", pinfo->gmp_support);
	return LCD_KIT_OK;
}

static int dbg_hiace_support(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->hiace_support = val;
	LCD_KIT_INFO("pinfo->hiace_support = %d\n", pinfo->hiace_support);
	return LCD_KIT_OK;
}

static int dbg_xcc_support(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->xcc_support = val;
	LCD_KIT_INFO("pinfo->xcc_support = %d\n", pinfo->xcc_support);
	return LCD_KIT_OK;
}

static int dbg_arsr1psharpness_support(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->arsr1p_sharpness_support = val;
	LCD_KIT_INFO("arsr1p_support = %d\n", pinfo->arsr1p_sharpness_support);
	return LCD_KIT_OK;
}

static int dbg_prefixsharptwo_d_support(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->prefix_sharpness2D_support = val;
	LCD_KIT_INFO("prefix_support=%d\n", pinfo->prefix_sharpness2D_support);
	return LCD_KIT_OK;
}

static void hisifb_frame_refresh(struct hisi_fb_data_type *hisifd)
{
#define ENVP_LEN 2
#define BUF_LEN 64
	char *envp[ENVP_LEN] = {0};
	char buf[BUF_LEN];

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return;
	}

	snprintf(buf, sizeof(buf), "Refresh=1");
	envp[0] = buf;
	envp[1] = NULL;
	kobject_uevent_env(&(hisifd->fbi->dev->kobj), KOBJ_CHANGE, envp);
}

static int dbg_video_idle_mode_support(int val)
{
#define BACKLIGHT_DELAY 100
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	struct hisi_panel_info *pinfo = NULL;
	bool panel_power_on = dbg_panel_power_on();
	uint32_t bl_level_cur;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	/* lcd panel off */
	if (panel_power_on) {
		bl_level_cur = hisifd->bl_level;
		hisifb_set_backlight(hisifd, 0, false);
		ret = hisi_fb_blank_sub(FB_BLANK_POWERDOWN, hisifd->fbi);
		if (ret != 0) {
			LCD_KIT_ERR("fb%d, blank_mode(%d) failed!\n",
				hisifd->index, FB_BLANK_POWERDOWN);
			return LCD_KIT_FAIL;
		}
	}

	pinfo->video_idle_mode = val;

	/* lcd panel on */
	if (panel_power_on) {
		ret = hisi_fb_blank_sub(FB_BLANK_UNBLANK, hisifd->fbi);
		if (ret != 0) {
			HISI_FB_ERR("fb%d, blank_mode(%d) failed!\n",
				hisifd->index, FB_BLANK_UNBLANK);
			return LCD_KIT_FAIL;
		}
		hisifb_frame_refresh(hisifd);
		msleep(BACKLIGHT_DELAY);
		bl_level_cur = bl_level_cur ? bl_level_cur : hisifd->bl_level;
		hisifb_set_backlight(hisifd, bl_level_cur, false);
	}
	LCD_KIT_INFO("pinfo->video_idle_mode = %d\n", pinfo->video_idle_mode);
	return LCD_KIT_OK;
}

static int dbg_cmd_type(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->type = val;
	LCD_KIT_INFO("pinfo->type = %d\n", pinfo->type);
	return LCD_KIT_OK;
}

static int dbg_pxl_clk(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->pxl_clk_rate = val * 1000000UL;
	LCD_KIT_INFO("pinfo->pxl_clk_rate = %llu\n", pinfo->pxl_clk_rate);
	return LCD_KIT_OK;
}

static int dbg_pxl_clk_div(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->pxl_clk_rate_div = val;
	LCD_KIT_INFO("pinfo->pxl_clk_rate_div = %d\n", pinfo->pxl_clk_rate_div);
	return LCD_KIT_OK;
}

static int dbg_vsync_ctrl_type(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->vsync_ctrl_type = val;
	LCD_KIT_INFO("pinfo->vsync_ctrl_type = %d\n", pinfo->vsync_ctrl_type);
	return LCD_KIT_OK;
}

static int dbg_hback_porch(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->ldi.h_back_porch = val;
	LCD_KIT_INFO("pinfo->ldi.h_back_porch = %d\n", pinfo->ldi.h_back_porch);
	return LCD_KIT_OK;
}

static int dbg_hfront_porch(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->ldi.h_front_porch = val;
	LCD_KIT_INFO("ldi.h_front_porch = %d\n", pinfo->ldi.h_front_porch);
	return LCD_KIT_OK;
}

static int dbg_hpulse_width(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->ldi.h_pulse_width = val;
	LCD_KIT_INFO("ldi.h_pulse_width = %d\n", pinfo->ldi.h_pulse_width);
	return LCD_KIT_OK;
}

static int dbg_vback_porch(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->ldi.v_back_porch = val;
	LCD_KIT_INFO("pinfo->ldi.v_back_porch = %d\n", pinfo->ldi.v_back_porch);
	return LCD_KIT_OK;
}

static int dbg_vfront_porch(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->ldi.v_front_porch = val;
	LCD_KIT_INFO("pinfo->ldi.v_front_porch=%d\n", pinfo->ldi.v_front_porch);
	return LCD_KIT_OK;
}

static int dbg_vpulse_width(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->ldi.v_pulse_width = val;
	LCD_KIT_INFO("pinfo->ldi.v_pulse_width=%d\n", pinfo->ldi.v_pulse_width);
	return LCD_KIT_OK;
}

static int dbg_mipi_burst_mode(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.burst_mode = val;
	LCD_KIT_INFO("pinfo->mipi.burst_mode = %d\n", pinfo->mipi.burst_mode);
	return LCD_KIT_OK;
}

static int dbg_mipi_max_tx_esc_clk(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.max_tx_esc_clk = val;
	LCD_KIT_INFO("mipi.max_tx_esc_clk = %d\n", pinfo->mipi.max_tx_esc_clk);
	return LCD_KIT_OK;
}

static int dbg_mipi_dsi_bit_clk(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.dsi_bit_clk_upt = val;
	LCD_KIT_INFO("pinfo->mipi.dsi_bit_clk = %d\n", pinfo->mipi.dsi_bit_clk);
	return LCD_KIT_OK;
}

static int dbg_mipi_dsi_bit_clk_a(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.dsi_bit_clk_val1 = val;
	LCD_KIT_INFO("dsi_bit_clk_val1 = %d\n", pinfo->mipi.dsi_bit_clk_val1);
	return LCD_KIT_OK;
}

static int dbg_mipi_dsi_bit_clk_b(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.dsi_bit_clk_val2 = val;
	LCD_KIT_INFO("dsi_bit_clk_val2 = %d\n", pinfo->mipi.dsi_bit_clk_val2);
	return LCD_KIT_OK;
}

static int dbg_mipi_dsi_bit_clk_c(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.dsi_bit_clk_val3 = val;
	LCD_KIT_INFO("dsi_bit_clk_val3 = %d\n", pinfo->mipi.dsi_bit_clk_val3);
	return LCD_KIT_OK;
}

static int dbg_mipi_dsi_bit_clk_d(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.dsi_bit_clk_val4 = val;
	LCD_KIT_INFO("dsi_bit_clk_val4 = %d\n", pinfo->mipi.dsi_bit_clk_val4);
	return LCD_KIT_OK;
}

static int dbg_mipi_dsi_bit_clk_e(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.dsi_bit_clk_val5 = val;
	LCD_KIT_INFO("dsi_bit_clk_val5 = %d\n", pinfo->mipi.dsi_bit_clk_val5);
	return LCD_KIT_OK;
}

static int dbg_mipi_noncontinue_enable(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.non_continue_en = val;
	LCD_KIT_INFO("non_continue_en = %d\n", pinfo->mipi.non_continue_en);
	return LCD_KIT_OK;
}

static int dbg_mipi_rg_vcm_adjust(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.rg_vrefsel_vcm_adjust = val;
	LCD_KIT_INFO("rg_vcm_adjust = %d\n", pinfo->mipi.rg_vrefsel_vcm_adjust);
	return LCD_KIT_OK;
}

static int dbg_mipi_clk_post_adjust(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.clk_post_adjust = val;
	LCD_KIT_INFO("pinfo->mipi.clk_post_adjust = %d\n", pinfo->mipi.clk_post_adjust);
	return LCD_KIT_OK;
}

static int dbg_mipi_clk_pre_adjust(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.clk_pre_adjust = val;
	LCD_KIT_INFO("clk_pre_adjust = %d\n", pinfo->mipi.clk_pre_adjust);
	return LCD_KIT_OK;
}

static int dbg_mipi_clk_ths_prepare_adjust(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.clk_t_hs_prepare_adjust = val;
	LCD_KIT_INFO("clk_ths_pre=%d\n", pinfo->mipi.clk_t_hs_prepare_adjust);
	return LCD_KIT_OK;
}

static int dbg_mipi_clk_tlpx_adjust(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.clk_t_lpx_adjust = val;
	LCD_KIT_INFO("clk_tlpx_adjust = %d\n", pinfo->mipi.clk_t_lpx_adjust);
	return LCD_KIT_OK;
}

static int dbg_mipi_clk_ths_trail_adjust(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.clk_t_hs_trial_adjust = val;
	LCD_KIT_INFO("clk_t_hs_trial=%d\n", pinfo->mipi.clk_t_hs_trial_adjust);
	return LCD_KIT_OK;
}

static int dbg_mipi_clk_ths_exit_adjust(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.clk_t_hs_exit_adjust = val;
	LCD_KIT_INFO("clk_t_hs_exit = %d\n", pinfo->mipi.clk_t_hs_exit_adjust);
	return LCD_KIT_OK;
}

static int dbg_mipi_clk_ths_zero_adjust(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.clk_t_hs_zero_adjust = val;
	LCD_KIT_INFO("clk_t_hs_zero = %d\n", pinfo->mipi.clk_t_hs_zero_adjust);
	return LCD_KIT_OK;
}

static int dbg_mipi_lp11_flag(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.lp11_flag = val;
	LCD_KIT_INFO("pinfo->mipi.lp11_flag = %d\n", pinfo->mipi.lp11_flag);
	return LCD_KIT_OK;
}

static int dbg_mipi_phy_update(int val)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->mipi.phy_m_n_count_update = val;
	LCD_KIT_INFO("phy_update = %d\n", pinfo->mipi.phy_m_n_count_update);
	return LCD_KIT_OK;
}

static int dbg_rgbw_bl_max(int val)
{
	disp_info->rgbw.rgbw_bl_max = val;
	LCD_KIT_INFO("rgbw_bl_max = %d\n", disp_info->rgbw.rgbw_bl_max);
	return LCD_KIT_OK;
}

static int dbg_rgbw_set_mode1(char *buf, int len)
{
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(&disp_info->rgbw.mode1_cmds, buf, len);
	return LCD_KIT_OK;
}

static int dbg_rgbw_set_mode2(char *buf, int len)
{
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(&disp_info->rgbw.mode2_cmds, buf, len);
	return LCD_KIT_OK;
}

static int dbg_rgbw_set_mode3(char *buf, int len)
{
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(&disp_info->rgbw.mode3_cmds, buf, len);
	return LCD_KIT_OK;
}

static int dbg_rgbw_set_mode4(char *buf, int len)
{
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(&disp_info->rgbw.mode4_cmds, buf, len);
	return LCD_KIT_OK;
}

static int dbg_rgbw_backlight_cmd(char *buf, int len)
{
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(&disp_info->rgbw.backlight_cmds, buf, len);
	return LCD_KIT_OK;
}

static int dbg_rgbw_pixel_gainlimit_cmd(char *buf, int len)
{
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(&disp_info->rgbw.pixel_gain_limit_cmds, buf, len);
	return LCD_KIT_OK;
}

static int dbg_barcode_2d_cmd(char *buf, int len)
{
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(&disp_info->oeminfo.barcode_2d.cmds, buf, len);
	return LCD_KIT_OK;
}

static int dbg_brightness_color_cmd(char *buf, int len)
{
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(&disp_info->oeminfo.brightness_color_uniform.brightness_color_cmds,
		buf, len);
	return LCD_KIT_OK;
}

static int dbg_set_voltage(void)
{
	int ret;

	ret = lcd_kit_dbg_set_voltage();
	return ret;
}

static int dbg_dsi_cmds_rx(uint8_t *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds)
{
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!out || !cmds) {
		LCD_KIT_ERR("out is null or cmds is null\n");
		return LCD_KIT_FAIL;
	}
	return lcd_kit_dsi_cmds_rx(hisifd, out, cmds);
}

static bool dbg_panel_power_on(void)
{
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	bool panel_power_on = false;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return false;
	}
	down(&hisifd->blank_sem);
	panel_power_on = hisifd->panel_power_on;
	up(&hisifd->blank_sem);
	return panel_power_on;
}

static int dbg_esd_check_func(void)
{
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	ret = init_panel_info(hisifd, &pinfo);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("init_panel_info fail\n");
		return ret;
	}
	pinfo->esd_enable = 1;
	if (hisifd->esd_register)
		hisifd->esd_register(hisifd->pdev);
	return LCD_KIT_OK;
}

struct lcd_kit_dbg_ops hisi_dbg_ops = {
	.fps_updt_support = dbg_fps_updt_support,
	.quickly_sleep_out_support = dbg_quickly_sleep_out_support,
	.blpwm_input_support = dbg_blpwm_input_support,
	.dsi_upt_support = dbg_dsi_upt_support,
	.rgbw_support = dbg_rgbw_support,
	.gamma_support = dbg_gamma_support,
	.gmp_support = dbg_gmp_support,
	.hiace_support = dbg_hiace_support,
	.xcc_support = dbg_xcc_support,
	.arsr1psharpness_support = dbg_arsr1psharpness_support,
	.prefixsharptwo_d_support = dbg_prefixsharptwo_d_support,
	.video_idle_mode_support = dbg_video_idle_mode_support,
	.cmd_type = dbg_cmd_type,
	.pxl_clk = dbg_pxl_clk,
	.pxl_clk_div = dbg_pxl_clk_div,
	.vsync_ctrl_type = dbg_vsync_ctrl_type,
	.hback_porch = dbg_hback_porch,
	.hfront_porch = dbg_hfront_porch,
	.hpulse_width = dbg_hpulse_width,
	.vback_porch = dbg_vback_porch,
	.vfront_porch = dbg_vfront_porch,
	.vpulse_width = dbg_vpulse_width,
	.mipi_burst_mode = dbg_mipi_burst_mode,
	.mipi_max_tx_esc_clk = dbg_mipi_max_tx_esc_clk,
	.mipi_dsi_bit_clk = dbg_mipi_dsi_bit_clk,
	.mipi_dsi_bit_clk_a = dbg_mipi_dsi_bit_clk_a,
	.mipi_dsi_bit_clk_b = dbg_mipi_dsi_bit_clk_b,
	.mipi_dsi_bit_clk_c = dbg_mipi_dsi_bit_clk_c,
	.mipi_dsi_bit_clk_d = dbg_mipi_dsi_bit_clk_d,
	.mipi_dsi_bit_clk_e = dbg_mipi_dsi_bit_clk_e,
	.mipi_noncontinue_enable = dbg_mipi_noncontinue_enable,
	.mipi_rg_vcm_adjust = dbg_mipi_rg_vcm_adjust,
	.mipi_clk_post_adjust = dbg_mipi_clk_post_adjust,
	.mipi_clk_pre_adjust = dbg_mipi_clk_pre_adjust,
	.mipi_clk_ths_prepare_adjust = dbg_mipi_clk_ths_prepare_adjust,
	.mipi_clk_tlpx_adjust = dbg_mipi_clk_tlpx_adjust,
	.mipi_clk_ths_trail_adjust = dbg_mipi_clk_ths_trail_adjust,
	.mipi_clk_ths_exit_adjust = dbg_mipi_clk_ths_exit_adjust,
	.mipi_clk_ths_zero_adjust = dbg_mipi_clk_ths_zero_adjust,
	.mipi_lp11_flag = dbg_mipi_lp11_flag,
	.mipi_phy_update = dbg_mipi_phy_update,
	.rgbw_bl_max = dbg_rgbw_bl_max,
	.rgbw_set_mode1 = dbg_rgbw_set_mode1,
	.rgbw_set_mode2 = dbg_rgbw_set_mode2,
	.rgbw_set_mode3 = dbg_rgbw_set_mode3,
	.rgbw_set_mode4 = dbg_rgbw_set_mode4,
	.rgbw_backlight_cmd = dbg_rgbw_backlight_cmd,
	.rgbw_pixel_gainlimit_cmd = dbg_rgbw_pixel_gainlimit_cmd,
	.barcode_2d_cmd = dbg_barcode_2d_cmd,
	.brightness_color_cmd = dbg_brightness_color_cmd,
	.set_voltage = dbg_set_voltage,
	.dbg_mipi_rx = dbg_dsi_cmds_rx,
	.panel_power_on = dbg_panel_power_on,
	.esd_check_func = dbg_esd_check_func,
};

int lcd_kit_dbg_init(void)
{
	int ret;

	ret = lcd_kit_debug_register(&hisi_dbg_ops);
	return ret;
}

/*
 * lcd dpd
 */
static int file_sys_is_ready(void)
{
	int fd = -1;
	int ret;
	struct device_node *np = NULL;
	char *lcd_name = NULL;
	char xml_name[XML_FILE_MAX];

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n",
			DTS_COMP_LCD_KIT_PANEL_TYPE);
		return LCD_KIT_FAIL;
	}
	lcd_name = (char *)of_get_property(np, "lcd_panel_type", NULL);
	if (!lcd_name) {
		LCD_KIT_ERR("can not get lcd kit compatible\n");
		return LCD_KIT_FAIL;
	}
	memset(xml_name, 0, sizeof(xml_name));
	ret = snprintf(xml_name, XML_FILE_MAX, "/odm/lcd/%s.xml", lcd_name);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf error: %s\n", xml_name);
		return LCD_KIT_FAIL;
	}
	fd = sys_access(xml_name, 0);
	if (fd) {
		LCD_KIT_INFO("can not access file!\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static void dpd_power_init(const struct device_node *np)
{
	/* vci */
	if (power_hdl->lcd_vci.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-vci",
			&power_hdl->lcd_vci);
	/* iovcc */
	if (power_hdl->lcd_iovcc.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-iovcc",
			&power_hdl->lcd_iovcc);
	/* vsp */
	if (power_hdl->lcd_vsp.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-vsp",
			&power_hdl->lcd_vsp);
	/* vsn */
	if (power_hdl->lcd_vsn.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-vsn",
			&power_hdl->lcd_vsn);
	/* backlight */
	if (power_hdl->lcd_backlight.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-backlight",
			&power_hdl->lcd_backlight);
	/* vdd */
	if (power_hdl->lcd_vdd.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-vdd",
			&power_hdl->lcd_vdd);
}

struct delayed_work detect_fs_work;
static int g_dpd_enter;
static void detect_fs_wq_handler(struct work_struct *work)
{
	if (file_sys_is_ready()) {
		LCD_KIT_INFO("sysfs is not ready!\n");
		device_unblock_probing();
		/* delay 500ms schedule work */
		schedule_delayed_work(&detect_fs_work, msecs_to_jiffies(500));
		return;
	}
	LCD_KIT_INFO("sysfs is ready!\n");
	g_dpd_enter = 1; // lcd dpd is ok, entry lcd dpd mode
	device_unblock_probing();
}

/* previous attribute name in xml file */
unsigned char item_name[LCD_KIT_ITEM_NAME_MAX];
/* attribute name char count */
static int item_cnt;
/* attribute data char count */
static int data_cnt;
/* parse xml file status varial */
static int par_status = PARSE_HEAD;
static int receive_data(int status, const char *ch, char *out, int len)
{
	switch (status) {
	case PARSE_HEAD:
		if (is_valid_char(*ch)) {
			if (item_cnt >= LCD_KIT_ITEM_NAME_MAX) {
				LCD_KIT_ERR("item is too long\n");
				return LCD_KIT_FAIL;
			}
			item_name[item_cnt++] = *ch;
		}
		break;
	case RECEIVE_DATA:
		if (data_cnt >= len) {
			LCD_KIT_ERR("data is too long\n");
			return LCD_KIT_FAIL;
		}
		out[data_cnt++] = *ch;
		break;
	case NOT_MATCH:
	case PARSE_FINAL:
	case INVALID_DATA:
		break;
	default:
		break;
	}
	return LCD_KIT_OK;
}

static int parse_char(const char *item, const char *ch,
	char *out, int len, int *find)
{
	int item_len;
	int tmp_len;

	switch (*ch) {
	case '<':
		if (par_status == RECEIVE_DATA) {
			*find = 1;
			return LCD_KIT_OK;
		}
		par_status = PARSE_HEAD;
		data_cnt = 0;
		item_cnt = 0;
		memset(item_name, 0, LCD_KIT_ITEM_NAME_MAX);
		break;
	case '>':
		if (par_status == PARSE_HEAD) {
			tmp_len = strlen(item_name);
			item_len = strlen(item);
			if ((tmp_len != item_len) ||
				strncmp(item_name, item, tmp_len)) {
				par_status = NOT_MATCH;
				return LCD_KIT_OK;
			}
			item_cnt = 0;
			par_status = RECEIVE_DATA;
		}
		break;
	case '/':
		par_status = PARSE_FINAL;
		break;
	case '!':
		if (par_status == PARSE_HEAD)
			par_status = INVALID_DATA;
		break;
	default:
		receive_data(par_status, ch, out, len);
		break;
	}
	return LCD_KIT_OK;
}

static int parse_open(const char *path)
{
	int ret;
	int fd = -1;

	set_fs(get_ds()); /* use kernel limit */
	fd = sys_open((const char __force *)path, O_RDONLY, 0);
	if (fd < 0) {
		LCD_KIT_ERR("%s file doesn't exsit\n", path);
		return LCD_KIT_FAIL;
	}
	ret = sys_lseek(fd, (off_t)0, 0);
	if (ret < 0) {
		LCD_KIT_ERR("sys_lseek error!\n");
		sys_close(fd);
		return LCD_KIT_FAIL;
	}
	return fd;
}

static int parse_xml(const char *path, const char *item, char *out, int len)
{
	char ch = '\0';
	int ret;
	int cur_seek = 0;
	int find = 0;
	int fd = -1;
	mm_segment_t fs;

	if (!out) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	fs = get_fs(); /* save previous value */
	fd = parse_open(path);
	if (fd < 0) {
		LCD_KIT_ERR("open file:%s fail!\n", path);
		set_fs(fs);
		return LCD_KIT_FAIL;
	}
	par_status = PARSE_HEAD;
	while (1) {
		/* read 1 char from file */
		if ((unsigned int)sys_read(fd, &ch, 1) != 1) {
			LCD_KIT_INFO("it's end of file\n");
			break;
		}
		cur_seek++;
		ret = sys_lseek(fd, (off_t)cur_seek, 0);
		if (ret < 0) {
			LCD_KIT_ERR("sys_lseek error!\n");
			break;
		}
		ret = parse_char(item, &ch, out, len, &find);
		if (ret < 0) {
			LCD_KIT_ERR("parse ch error!\n");
			break;
		}
		if (find) {
			LCD_KIT_INFO("find item:%s = %s\n", item, out);
			break;
		}
		continue;
	}
	if (find)
		ret = LCD_KIT_OK;
	else
		ret = LCD_KIT_FAIL;
	set_fs(fs);
	sys_close(fd);
	return ret;
}

static int stat_value_count(const char *in)
{
	char ch;
	int cnt = 1;
	int len;
	int i = 0;

	if (!in) {
		LCD_KIT_ERR("in or out is null\n");
		return LCD_KIT_FAIL;
	}
	len = strlen(in);
	while (len--) {
		ch = in[i++];
		if (ch != ',' && ch != '\n')
			continue;
		cnt++; // calculate data number in string:in, data separate use ', '
	}
	return cnt;
}

static char *str_convert(const char *item)
{
	int i;
	int len;
	int tmp_len;
	const int array_size = ARRAY_SIZE(xml_string_name_map);

	tmp_len = strlen(item);
	for (i = 0; i < array_size; i++) {
		len = strlen(xml_string_name_map[i].dtsi_string_name);
		if (tmp_len != len)
			continue;
		if (!strncmp(item, xml_string_name_map[i].dtsi_string_name, len))
			return xml_string_name_map[i].xml_string_name;
	}
	LCD_KIT_INFO("not find item:%s\n", item);
	return NULL;
}

static int parse_u32(const char *path, const char *item,
	unsigned int *out, unsigned int def)
{
	int ret;
	char *data = NULL;
	unsigned int value = 0;
	/*
	 * request (0xffffffff->4294967295) max 10 byte to save
	 * string and 1 byte to save '\0'
	 */
	const int u32_str_size = 11;

	data = kzalloc(u32_str_size, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	*out = def;
	ret = parse_xml(path, item, data, u32_str_size);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	/* parse 1 digit */
	lcd_kit_parse_u32_digit(data, &value, 1);
	*out = value;
	kfree(data);
	return LCD_KIT_OK;
}

static int parse_u8(const char *path, const char *item,
	unsigned char *out, unsigned char def)
{
	int ret;
	char *data = NULL;
	unsigned int value = 0;

	data = kzalloc(sizeof(unsigned int), 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	*out = def;
	ret = parse_xml(path, item, data, sizeof(unsigned int));
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	/* parse 1 digit */
	lcd_kit_parse_u32_digit(data, &value, 1);
	*out = (unsigned char)(value & 0xff);
	kfree(data);
	return LCD_KIT_OK;
}

static int parse_dcs(const char *path, const char *item,
	const char *item_state, struct lcd_kit_dsi_panel_cmds *pcmds)
{
	int ret;
	int len;
	char *data = NULL;
	char *temp_buff = NULL; // temp_buff can not free, it will be used

	data = kzalloc(LCD_KIT_DCS_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_DCS_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	len = stat_value_count(data);
	if (len <= 0) {
		LCD_KIT_ERR("invalid len\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	temp_buff = kzalloc(sizeof(int) * (len + 1), 0);
	if (!temp_buff) {
		LCD_KIT_ERR("kzalloc fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(data, temp_buff, len);
	if (len <= 0) {
		LCD_KIT_ERR("lcd_kit_parse_u8_digit error\n");
		kfree(temp_buff);
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(pcmds, temp_buff, len);
	ret = parse_u32(path, item_state, &(pcmds->link_state),
		LCD_KIT_DSI_HS_MODE);
	if (ret < 0)
		LCD_KIT_INFO("parse link_state fail\n");
	kfree(data);
	return LCD_KIT_OK;
}

static int parse_arrays(const char *path, const char *item,
	struct lcd_kit_arrays_data *out, int size)
{
	char ch = '"';
	char *data = NULL;
	unsigned int *temp_buff = NULL; // temp_buff can not free, it will be used
	int ret;
	int len;

	data = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(data, ch);
	len = stat_value_count(data);
	if (len <= 0) {
		kfree(data);
		LCD_KIT_ERR("invalid len\n");
		return LCD_KIT_FAIL;
	}
	temp_buff = kzalloc(sizeof(int) * (len + 1), 0);
	if (!temp_buff) {
		LCD_KIT_ERR("temp_buff kzalloc fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	if (size <= 0) {
		LCD_KIT_ERR("size is error\n");
		return LCD_KIT_FAIL;
	}
	len = len / size;
	if (len <= 0) {
		LCD_KIT_ERR("len is error\n");
		return LCD_KIT_FAIL;
	}
	out->arry_data = kzalloc(sizeof(struct lcd_kit_array_data) * len, 0);
	if (!out->arry_data) {
		LCD_KIT_ERR("out->arry_data kzalloc fail\n");
		kfree(temp_buff);
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_array(data, temp_buff, out, size);
	kfree(data);
	return LCD_KIT_OK;
}

static int parse_array(const char *path, const char *item,
	struct lcd_kit_array_data *out)
{
	char ch = '"';
	char *data = NULL;
	unsigned int *temp_buff = NULL; // temp_buff can not free, it will be used
	int ret;
	int len;
	int cnt;

	data = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(data, ch);
	len = stat_value_count(data);
	if (len <= 0) {
		kfree(data);
		LCD_KIT_ERR("invalid len\n");
		return NULL;
	}
	temp_buff = kzalloc(sizeof(int) * (len + 1), 0);
	if (!temp_buff) {
		LCD_KIT_ERR("temp_buff kzalloc fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	cnt = lcd_kit_parse_u32_digit(data, temp_buff, len);
	out->buf = temp_buff;
	out->cnt = cnt;
	kfree(data);
	return LCD_KIT_OK;
}

static char *parse_string(const char *path, const char *item)
{
	char ch = '"';
	char *data = NULL;
	char *string = NULL;
	int ret;
	int len;

	data = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return NULL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return NULL;
	}
	lcd_kit_str_to_del_ch(data, ch);
	len = strlen(data);
	if (len <= 0) {
		kfree(data);
		LCD_KIT_ERR("invalid len\n");
		return NULL;
	}
	string = kzalloc((len + 1), 0);
	if (!string) {
		LCD_KIT_ERR("string kzalloc fail\n");
		kfree(data);
		return NULL;
	}
	strncpy(string, data, len);
	kfree(data);
	return string;
}

static int parse_string_array(const char *path, const char *item,
	const char **out)
{
	char ch = '"';
	char *data = NULL;
	char *string = NULL;
	char *str1 = NULL;
	char *str2 = NULL;
	int ret;
	int len;

	data = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(data, ch);
	len = strlen(data);
	if (len <= 0) {
		kfree(data);
		LCD_KIT_ERR("invalid len\n");
		return LCD_KIT_FAIL;
	}
	string = kzalloc((len + 1), 0);
	if (!string) {
		LCD_KIT_ERR("temp_buff kzalloc fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	strncpy(string, data, len);
	str1 = string;
	do {
		str2 = strstr(str1, ",");
		if (str2 == NULL) {
			*out++ = str1;
			break;
		}
		*str2 = 0;
		*out++ = str1;
		str2++;
		str1 = str2;
	} while (str2 != NULL);
	kfree(data);
	return LCD_KIT_OK;
}

static void parse_dirty_region(const char *path, const char *item,
	int *out)
{
	int value = 0;
	int ret;

	ret = parse_u32(path, item, &value, 0xffff);
	if (ret)
		LCD_KIT_INFO("parse item:%s fail\n", item);
	if (value == 0xffff)
		value = -1;
	*out = value;
}

static int g_dpd_mode;
static int __init early_parse_debug_cmdline(char *arg)
{
	char debug[CMDLINE_MAX + 1];
	int ret;

	if (!arg) {
		LCD_KIT_ERR("arg is NULL\n");
		return 0;
	}
	memset(debug, 0, (CMDLINE_MAX + 1));
	memcpy(debug, arg, CMDLINE_MAX);
	debug[CMDLINE_MAX] = '\0';
	ret = kstrtouint(debug, 10, &g_dpd_mode);
	if (ret)
		return ret;
	LCD_KIT_INFO("g_dpd_mode = %d\n", g_dpd_mode);
	return 0;
}
early_param("lcd_dpd", early_parse_debug_cmdline);

char g_xml_name[XML_NAME_MAX];
static char *dpd_get_xml_name(const struct device_node *np)
{
	char *comp = NULL;
	int ret;

	comp = (char *)of_get_property(np, "compatible", NULL);
	if (!comp) {
		LCD_KIT_ERR("comp is null\n");
		return NULL;
	}
	memset(g_xml_name, 0, sizeof(g_xml_name));
	ret = snprintf(g_xml_name, XML_NAME_MAX, "/odm/lcd/%s.xml", comp);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf fail\n");
		return NULL;
	}
	return g_xml_name;
}

int lcd_parse_u32(const struct device_node *np, const char *item,
	unsigned int *out, unsigned int def)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("pointer is null:%s, %s, %s\n",
			np, item, out);
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_u32(path, item_tmp, out, def);
}

int lcd_parse_u8(const struct device_node *np, const char *item,
	unsigned char *out, unsigned char def)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("pointer is null:%s, %s, %s, %s\n",
			np, item, out);
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_u8(path, item_tmp, out, def);
}

int lcd_parse_dcs(const struct device_node *np, const char *item,
	const char *item_state, struct lcd_kit_dsi_panel_cmds *pcmds)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !item_state || !pcmds) {
		LCD_KIT_ERR("pointer is null:%s, %s, %s, %s\n",
			np, item, item_state, pcmds);
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_dcs(path, item_tmp, item_state, pcmds);
}

int lcd_parse_arrays(const struct device_node *np, const char *item,
	struct lcd_kit_arrays_data *out, int size)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("pointer is null:%s, %s, %s\n", np, item, out);
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_arrays(path, item_tmp, out, size);
}

int lcd_parse_array(const struct device_node *np, const char *item,
	struct lcd_kit_array_data *out)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("pointer is null:%s, %s, %s\n", np, item, out);
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_array(path, item_tmp, out);
}

char *lcd_parse_string(const struct device_node *np, const char *item)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item) {
		LCD_KIT_ERR("pointer is null:%s, %s\n", np, item);
		return NULL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return NULL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return NULL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_string(path, item_tmp);
}


int lcd_parse_string_array(const struct device_node *np, const char *item,
	const char **out)
{
	char *path = NULL;
	char *item_tmp = NULL;
	int ret;

	if (!np || !item || !out) {
		LCD_KIT_ERR("pointer is null:%s, %s, %s\n", np, item, out);
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	ret = parse_string_array(path, item_tmp, out);
	if (ret) {
		LCD_KIT_ERR("parse string array fail\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

void lcd_parse_dirty_region(const struct device_node *np, const char *item,
	int *out)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("pointer is null:%s, %s, %s\n", np, item, out);
		return;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	parse_dirty_region(path, item_tmp, out);
}

int dpd_init(struct platform_device *pdev)
{
	static bool is_first = true;
	struct device_node *np = NULL;

	if (!pdev) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}
	if (!g_dpd_mode)
		return LCD_KIT_OK;
	np = pdev->dev.of_node;
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node\n");
		return LCD_KIT_FAIL;
	}
	if (is_first) {
		is_first = false;
		dpd_power_init(np);
		lcd_kit_power_init(pdev);
		/* register lcd ops */
		lcd_kit_ops_register(&g_lcd_ops);
		INIT_DELAYED_WORK(&detect_fs_work, detect_fs_wq_handler);
		/* delay 500ms schedule work */
		schedule_delayed_work(&detect_fs_work, msecs_to_jiffies(500));
	}
	if (file_sys_is_ready()) {
		LCD_KIT_INFO("sysfs is not ready!\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

int is_dpd_mode(void)
{
	if ((g_dpd_mode == 1) && (g_dpd_enter == 1))
		return 1; // dpd mode
	return 0; // normal mode
}
