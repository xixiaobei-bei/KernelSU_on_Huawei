/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_display_effect.h"
#include "hisi_overlay_utils.h"
#include "hisi_dpe_utils.h"
#include <linux/hisi/hw_cmdline_parse.h>

/*lint -e838, -e778, -e845, -e712, -e527, -e30, -e142, -e715, -e655, -e550*/
void hisi_dss_effect_set_reg(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return ;
	}

	if (PRIMARY_PANEL_IDX != hisifd->index) {
		HISI_FB_DEBUG("fb%d, not support!\n", hisifd->index);
		return;
	}


	return;
}

static int display_engine_blc_param_get(struct hisi_fb_data_type *hisifd, display_engine_blc_param_t *param)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	if (NULL == param) {
		HISI_FB_ERR("[effect] param is NULL\n");
		return -EINVAL;
	}

	param->enable = hisifd->de_info.blc_enable ? 1 : 0;
	param->delta = hisifd->de_info.blc_delta;
	return 0;
}

static int display_engine_blc_param_set(struct hisi_fb_data_type *hisifd, display_engine_blc_param_t *param)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	if (NULL == param) {
		HISI_FB_ERR("[effect] param is NULL\n");
		return -EINVAL;
	}

	hisifd->de_info.blc_enable = (param->enable == 1) ? true : false;
	hisifd->de_info.blc_delta = param->delta;
	return 0;
}

int display_engine_ddic_color_param_get(struct hisi_fb_data_type *hisifd, display_engine_ddic_color_param_t *param)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		HISI_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	param->ddic_color_mode = hisifd->de_info.ddic_color_mode;

	return 0;
}

int display_engine_ddic_color_param_set(struct hisi_fb_data_type *hisifd, display_engine_ddic_color_param_t *param)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	if (hisifd->pdev == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		HISI_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	hisifd->de_info.ddic_color_mode = param->ddic_color_mode;

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -1;
		goto err_out;
	}

	if (pdata->lcd_ce_mode_store) {
		char buf[8];
		int count = 0;
		hisifb_activate_vsync(hisifd);
		count = snprintf(buf, sizeof(buf), "%d", param->ddic_color_mode);
		pdata->lcd_ce_mode_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

int display_engine_ddic_cabc_param_get(struct hisi_fb_data_type *hisifd, display_engine_ddic_cabc_param_t *param)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		HISI_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}
	param->ddic_cabc_mode = hisifd->de_info.ddic_cabc_mode;
	return 0;
}

int display_engine_ddic_cabc_param_set(struct hisi_fb_data_type *hisifd, display_engine_ddic_cabc_param_t *param)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	if (hisifd->pdev == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}


	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		HISI_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	hisifd->de_info.ddic_cabc_mode = param->ddic_cabc_mode;

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -1;
		goto err_out;
	}

	if (pdata->lcd_cabc_mode_store) {
		char buf[8];
		int count = 0;
		hisifb_activate_vsync(hisifd);
		count = snprintf(buf, sizeof(buf), "%d", param->ddic_cabc_mode);
		pdata->lcd_cabc_mode_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

int display_engine_ddic_rgbw_param_get(struct hisi_fb_data_type *hisifd, display_engine_ddic_rgbw_param_t *param)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		HISI_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	param->ddic_panel_id = hisifd->de_info.ddic_panel_id;
	param->ddic_rgbw_mode = hisifd->de_info.ddic_rgbw_mode;
	param->ddic_rgbw_backlight = hisifd->de_info.ddic_rgbw_backlight;
	param->rgbw_saturation_control = hisifd->de_info.rgbw_saturation_control;
	param->frame_gain_limit = hisifd->de_info.frame_gain_limit;
	param->frame_gain_speed = hisifd->de_info.frame_gain_speed;
	param->color_distortion_allowance = hisifd->de_info.color_distortion_allowance;
	param->pixel_gain_limit = hisifd->de_info.pixel_gain_limit;
	param->pixel_gain_speed = hisifd->de_info.pixel_gain_speed;
	param->pwm_duty_gain = hisifd->de_info.pwm_duty_gain;
	HISI_FB_DEBUG("[effect] display_engine_ddic_rgbw_param_get params\n");
	return 0;
}

int display_engine_ddic_rgbw_param_set(struct hisi_fb_data_type *hisifd, display_engine_ddic_rgbw_param_t *param)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		HISI_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	hisifd->de_info.ddic_panel_id = param->ddic_panel_id;
	hisifd->de_info.ddic_rgbw_mode = param->ddic_rgbw_mode;
	hisifd->de_info.ddic_rgbw_backlight = param->ddic_rgbw_backlight;
	hisifd->de_info.rgbw_saturation_control = param->rgbw_saturation_control;
	hisifd->de_info.frame_gain_limit = param->frame_gain_limit;
	hisifd->de_info.frame_gain_speed = param->frame_gain_speed;
	hisifd->de_info.color_distortion_allowance = param->color_distortion_allowance;
	hisifd->de_info.pixel_gain_limit = param->pixel_gain_limit;
	hisifd->de_info.pixel_gain_speed = param->pixel_gain_speed;
	hisifd->de_info.pwm_duty_gain = param->pwm_duty_gain;

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -1;
		goto err_out;
	}

	if (pdata->lcd_rgbw_set_func) {
		hisifb_activate_vsync(hisifd);
		pdata->lcd_rgbw_set_func(hisifd);
		hisifb_deactivate_vsync(hisifd);

		hisifd->rgbw_bl_count++;
		HISI_FB_DEBUG("[effect] pdata support lcd_rgbw_set_func\n");
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

int display_engine_hbm_param_set(struct hisi_fb_data_type *hisifd, display_engine_hbm_param_t *param)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;

	if(NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}
	if(NULL == param) {
		HISI_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if(NULL == pdata){
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	hisifd->de_info.hbm_level = param->level;
	down(&hisifd->blank_sem);
	if(!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -1;
		goto err_out;
	}
	if(pdata->lcd_hbm_set_func) {
		hisifb_activate_vsync(hisifd);
		pdata->lcd_hbm_set_func(hisifd);
		hisifb_deactivate_vsync(hisifd);
	} else {
		HISI_FB_ERR("[effect] lcd_hbm_set_func is NULL\n");
		ret = -1;
		goto err_out;
	}

err_out:
	up(&hisifd->blank_sem);
	return ret;
}
int display_engine_panel_info_get(struct hisi_fb_data_type *hisifd, display_engine_panel_info_param_t *param) {

	struct hisi_panel_info * pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		HISI_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	pinfo = &(hisifd->panel_info);
	param->width = pinfo->xres;
	param->height = pinfo->yres;
	param->maxluminance = pinfo->hiace_param.iMaxLcdLuminance;
	param->minluminance = pinfo->hiace_param.iMinLcdLuminance;
	param->maxbacklight = pinfo->bl_max;
	param->minbacklight = pinfo->bl_min;

	return 0;
}

int display_engine_color_rectify_param_get(struct hisi_fb_data_type *hisifd, display_engine_color_rectify_param_t *param) {
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		HISI_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	if (hisifd->pdev == NULL) {
		HISI_FB_ERR("[effect] pdev is NULL Pointer\n");
		return -1;
	}

	memset(&(hisifd->de_info.lcd_color_oeminfo), 0, sizeof(struct disp_lcdbrightnesscoloroeminfo));

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -1;
		goto err_out;
	}
	
	if (pdata->lcd_color_param_get_func) {
		hisifb_activate_vsync(hisifd);
		pdata->lcd_color_param_get_func(hisifd);
		hisifb_deactivate_vsync(hisifd);
	}
	param->lcd_color_oeminfo = hisifd->de_info.lcd_color_oeminfo;
	HISI_FB_DEBUG("[effect] display_engine_color_rectify_param_get params\n");
err_out:
	up(&hisifd->blank_sem);
	return ret;
}

int hisifb_display_engine_init(struct fb_info *info, void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;
	display_engine_t de;
	int ret = 0;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (!hisifd->de_info.is_ready) {
		memset(&hisifd->de_info, 0, sizeof(display_engine_info_t));
		hisifd->de_info.is_ready = true;
	}

	de.blc_support = 1;
	de.ddic_cabc_support = pdata->lcd_cabc_mode_store == NULL ? 0 : 1;
	de.ddic_rgbw_support = pdata->lcd_rgbw_set_func == NULL ? 0 : 1;
	de.ddic_hbm_support = pdata->lcd_hbm_set_func == NULL ? 0 : 1;
	de.ddic_color_support = pdata->lcd_ce_mode_store == NULL ? 0 : 1;
	de.ddic_color_rectify_support = pdata->lcd_color_param_get_func == NULL ? 0 : 1;

	ret = (int)copy_to_user(argp, &de, sizeof(display_engine_t));
	if (ret) {
		HISI_FB_ERR("[effect] copy_to_user(display_engine_t) failed! ret=%d.\n", ret);
		goto ERR_OUT;
	}

ERR_OUT:
	return ret;
}

int hisifb_display_engine_deinit(struct fb_info *info, void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	display_engine_t de;
	int ret = 0;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	if (hisifd->de_info.is_ready) {
		ret = (int)copy_from_user(&de, argp, sizeof(display_engine_t));
		if (ret) {
			HISI_FB_ERR("[effect] copy_from_user(display_engine_t) failed! ret=%d.\n", ret);
			goto ERR_OUT;
		}
		hisifd->de_info.is_ready = false;
	}

ERR_OUT:
	return ret;
}

int hisifb_display_engine_param_get(struct fb_info *info, void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	display_engine_param_t de_param;
	int ret = 0;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	if (!hisifd->de_info.is_ready) {
		HISI_FB_ERR("[effect] display engine has not been initialized!\n");
		return -EINVAL;
	}

	ret = (int)copy_from_user(&de_param, argp, sizeof(display_engine_param_t));
	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(param) failed! ret=%d.\n", ret);
		goto ERR_OUT;
	}

	if (de_param.modules & DISPLAY_ENGINE_BLC) {
		ret = display_engine_blc_param_get(hisifd, &de_param.blc);
		if (ret) {
			HISI_FB_ERR("[effect] failed to get BLC, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_CABC) {
		ret = display_engine_ddic_cabc_param_get(hisifd, &de_param.ddic_cabc);
		if (ret) {
			HISI_FB_ERR("[effect] failed to get BLC, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_COLOR) {
		ret = display_engine_ddic_color_param_get(hisifd, &de_param.ddic_color);
		if (ret) {
			HISI_FB_ERR("[effect] failed to get BLC, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_RGBW) {
		ret = display_engine_ddic_rgbw_param_get(hisifd, &de_param.ddic_rgbw);
		if (ret) {
			HISI_FB_ERR("[effect] failed to get rgbw, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_PANEL_INFO) {
		HISI_FB_ERR("[effect ] DISPLAY_ENGINE_PANEL_INFO");
		ret = display_engine_panel_info_get(hisifd, &de_param.panel_info);
		if (ret) {
			HISI_FB_ERR("[effect] failed to get panel info, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_COLOR_RECTIFY) {
		HISI_FB_ERR("[effect ] DISPLAY_ENGINE_COLOR_RECTIFY");
		ret = display_engine_color_rectify_param_get(hisifd, &de_param.color_param);
		if (ret) {
			HISI_FB_ERR("[effect] failed to get color param, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}
	ret = (int)copy_to_user(argp, &de_param, sizeof(display_engine_param_t));
	if (ret) {
		HISI_FB_ERR("[effect] copy_to_user(param) failed! ret=%d.\n", ret);
		goto ERR_OUT;
	}

ERR_OUT:
	return ret;
}

int hisifb_display_engine_param_set(struct fb_info *info, void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	display_engine_param_t de_param;
	int ret = 0;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	if (!hisifd->de_info.is_ready) {
		HISI_FB_ERR("[effect] display engine has not been initialized!\n");
		return -EINVAL;
	}

	ret = (int)copy_from_user(&de_param, argp, sizeof(display_engine_param_t));
	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(param) failed! ret=%d.\n", ret);
		goto ERR_OUT;
	}

	if (de_param.modules & DISPLAY_ENGINE_BLC) {
		ret = display_engine_blc_param_set(hisifd, &de_param.blc);
		if (ret) {
			HISI_FB_ERR("[effect] failed to set BLC, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_CABC) {

		ret = display_engine_ddic_cabc_param_set(hisifd, &de_param.ddic_cabc);
		if (ret) {
			HISI_FB_ERR("[effect] failed to set DDIC_CABC, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_RGBW) {

		ret = display_engine_ddic_rgbw_param_set(hisifd, &de_param.ddic_rgbw);
		if (ret) {
			HISI_FB_ERR("[effect] failed to set DDIC_RGBW, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_COLOR) {

		ret = display_engine_ddic_color_param_set(hisifd, &de_param.ddic_color);
		if (ret) {
			HISI_FB_ERR("[effect] failed to set DDIC_COLOR, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}
	if (de_param.modules & DISPLAY_ENGINE_HBM) {
		ret = display_engine_hbm_param_set(hisifd, &de_param.hbm);
		if (ret) {
			HISI_FB_ERR("[effect] failed to set HBW, ret=%d\n", ret);
			goto ERR_OUT;
		}
	}

ERR_OUT:
	return ret;
}

/*lint +e838, +e778, +e845, +e712, +e527, +e30, +e142, +e715, +e655, +e550*/
