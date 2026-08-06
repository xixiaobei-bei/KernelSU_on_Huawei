/* Copyright (c) 2019-2021, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winteger-overflow"

#include "overlay/hisi_overlay_utils.h"
#include "hisi_displayengine_utils.h"
#include "hisi_fb.h"

bool de_fp_open_active = false;
static bool is_mask_layer_on;
static const uint32_t te_interval_60_fps_us = 16667;
static const uint64_t te_interval_90_fps_us = 11111;
static const uint64_t te_interval_120_fps_us = 8334;
static const uint32_t fp_hbm_enter_flag = 1;
static const uint32_t fp_hbm_exit_flag = 2;
static const uint8_t fp_mmi_test_flag = 1;

enum force_alpha_enable_state {
	NO_FORCING = -1,
	FORCE_DISABLE = 0,
	FORCE_ENABLE = 1
};

static bool hisi_mask_layer_backlight_config_check_parameter_valid(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req_prev, dss_overlay_t *pov_req)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is null!\n");
		return false;
	}

	if (pov_req == NULL) {
		DPU_FB_DEBUG("pov_req is null!\n");
		return false;
	}

	if (pov_req_prev == NULL) {
		DPU_FB_DEBUG("pov_req_prev is null!\n");
		return false;
	}

	if (PRIMARY_PANEL_IDX != dpufd->index) {
		DPU_FB_DEBUG("fb%d, not support!\n", dpufd->index);
		return false;
	}

	return true;
}

static enum mask_layer_change_status hisi_masklayer_change_status(
	struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_prev,
	dss_overlay_t *pov)
{
	if (pov->mask_layer_exist != pov_prev->mask_layer_exist) {
		DPU_FB_INFO("mask_layer_exist pov : %d, pov_prev : %d, hbm_status : %d",
			pov->mask_layer_exist, pov_prev->mask_layer_exist, dpufd->hbm_is_opened);
	}
	if (pov->mask_layer_exist == MASK_LAYER_COMMON_STATE
		&& !(pov_prev->mask_layer_exist)) {
		return MASK_LAYER_COMMON_ADDED;
	} else if (dpufd->bl_level > 0 && dpufd->hbm_need_to_open
		&& dpufd->backlight.bl_updated > 0
		&& pov->mask_layer_exist == MASK_LAYER_SCREENON_STATE
		&& !(pov_prev->mask_layer_exist)) {
		dpufd->hbm_need_to_open = false;
		return MASK_LAYER_SCREENON_ADDED;
	} else if ((pov_prev->mask_layer_exist == CIRCLE_LAYER_STATE
		|| pov_prev->mask_layer_exist == MASK_LAYER_SCREENON_STATE
		|| pov_prev->mask_layer_exist == MASK_LAYER_COMMON_STATE)
		&& !(pov->mask_layer_exist)) {
		return MASK_LAYER_REMOVED;
	} else if (pov->mask_layer_exist == CIRCLE_LAYER_STATE
		&& pov_prev->mask_layer_exist == MASK_LAYER_SCREENON_STATE) {
		return CIRCLE_LAYER_ADDED;
	} else if (pov->mask_layer_exist == MASK_LAYER_SCREENON_STATE
		&& pov_prev->mask_layer_exist == CIRCLE_LAYER_STATE) {
		return CIRCLE_LAYER_REMOVED;
	}

	return MASK_LAYER_NO_CHANGE;
}

static uint64_t get_real_te_interval(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd)
		return te_interval_60_fps_us;

	if (dpufd->panel_info.fps == FPS_120HZ)
		return te_interval_120_fps_us;
	else if (dpufd->panel_info.fps == FPS_90HZ)
		return te_interval_90_fps_us;
	else
		return te_interval_60_fps_us;
}

static uint64_t correct_time_based_on_fps(uint32_t real_te_interval,
	uint64_t time_60_fps)
{
	return time_60_fps * real_te_interval / te_interval_60_fps_us;
}


static uint64_t get_backlight_sync_delay_time_us(
	struct dpu_fb_panel_data *pdata,
	struct dpu_fb_data_type *dpufd, bool is_dc)
{
	uint64_t left_thres_us;
	uint64_t right_thres_us;
	uint64_t te_interval;
	uint64_t diff_time;
	uint64_t current_time; //lint !e578
	uint64_t delayus = 0;
	uint32_t real_te_interval;

	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return 0;
	}
	real_te_interval = get_real_te_interval(dpufd);
	left_thres_us = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->left_time_to_te_us);
	right_thres_us = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->right_time_to_te_us);
	if (is_dc && pdata->panel_info->dcdelay_support > 0)
		right_thres_us = correct_time_based_on_fps(real_te_interval,
			pdata->panel_info->dc_time_to_te_us);

	te_interval = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->te_interval_us);
	if (te_interval == 0) {
		DPU_FB_WARNING("te_interval is 0, used default time!");
		te_interval = te_interval_60_fps_us;
	}
	if (left_thres_us == 0 && right_thres_us == 0) {
		DPU_FB_WARNING("left_thres_us && right_thres_us are 0, delay_time use 0!");
		return 0;
	}
	diff_time = ktime_to_us(ktime_get()) - ktime_to_us(dpufd->te_timestamp);

	// when diff time is more than 10 times TE, FP maybe blink.
	if (diff_time > real_te_interval * 10) {
		usleep_range(real_te_interval, real_te_interval);
		diff_time = ktime_to_us(ktime_get()) - ktime_to_us(dpufd->te_timestamp);
		DPU_FB_INFO("delay one frame to wait te, te = %d, diff_time =%ld",
			real_te_interval, diff_time);
	}
	current_time = diff_time - diff_time / te_interval * te_interval;
	if (current_time < left_thres_us)
		delayus = left_thres_us - current_time;
	else if (current_time > right_thres_us)
		delayus = te_interval - current_time + left_thres_us;
	DPU_FB_INFO("backlight_sync current_time = %lu, diff_time = %lu us",
		current_time, diff_time);
	DPU_FB_INFO("backlight_sync delay = %lu us, left_threshold = %lu us",
		delayus, left_thres_us);
	DPU_FB_INFO("backlight_sync right_th = %lu us, te_interval = %lu",
		right_thres_us, te_interval);

	return delayus;
}

static int display_engine_ddic_irc_set_inner(struct dpu_fb_data_type *dpufd, bool enable)
{
	return 0;
}

static void display_engine_local_hbm_get_dbv_threshold(struct dpu_fb_data_type *dpufd, uint32_t *threshold)
{
	static bool is_first_getting = true;
	uint32_t i = 0;

	if (!is_first_getting)
		return;
	for (i = 0; i < sizeof(dpufd->de_info.alpha_map) / sizeof(uint16_t); i++) {
		if (dpufd->de_info.alpha_map[i] == ALPHA_DEFAULT) {
			*threshold = i;
			is_first_getting = false;
			break;
		}
	}
}

u32 display_engine_local_hbm_get_support(void)
{
	return 0;
}

static int display_engine_local_hbm_set_dbv(struct dpu_fb_data_type *dpufd, uint32_t dbv)
{
	return 0;
}

static int display_engine_local_hbm_set_circle(struct dpu_fb_data_type *dpufd, bool is_on)
{
	return 0;
}

static u32 display_engine_get_mipi_level(void)
{
	return 0;
}

static void display_engine_local_hbm_alpha_log_print(
	struct dpu_fb_data_type *dpufd, uint32_t dbv_threshold)
{
	// 0 indicates the minimum dbv, and 4095 indicates the maximum dbv.
	if (dbv_threshold == 0)
		DPU_FB_INFO("alpha map alpha_map[%d]:%d, alpha_map[%d]:%d, alpha_map[%d]:%d\n",
			dbv_threshold, dpufd->de_info.alpha_map[dbv_threshold],
			dbv_threshold + 1, dpufd->de_info.alpha_map[dbv_threshold + 1],
			dbv_threshold + 2, dpufd->de_info.alpha_map[dbv_threshold + 2]);
	else if (dbv_threshold == 4095)
		DPU_FB_INFO("alpha map alpha_map[0]:%d, alpha_map[1]:%d, alpha_map[2]:%d, "
			"alpha_map[%d]:%d, alpha_map[%d]:%d, alpha_map[%d]:%d\n",
			dpufd->de_info.alpha_map[0], dpufd->de_info.alpha_map[1],
			dpufd->de_info.alpha_map[2], dbv_threshold - 2,
			dpufd->de_info.alpha_map[dbv_threshold - 2], dbv_threshold - 1,
			dpufd->de_info.alpha_map[dbv_threshold - 1], dbv_threshold,
			dpufd->de_info.alpha_map[dbv_threshold]);
	else
		DPU_FB_INFO("alpha map alpha_map[0]:%d, alpha_map[1]:%d, alpha_map[2]:%d, "
			"alpha_map[%d]:%d, alpha_map[%d]:%d, alpha_map[%d]:%d\n",
			dpufd->de_info.alpha_map[0], dpufd->de_info.alpha_map[1],
			dpufd->de_info.alpha_map[2], dbv_threshold - 1,
			dpufd->de_info.alpha_map[dbv_threshold - 1], dbv_threshold,
			dpufd->de_info.alpha_map[dbv_threshold], dbv_threshold + 1,
			dpufd->de_info.alpha_map[dbv_threshold + 1]);
}

static void display_engine_local_hbm_set_circle_alpha_dbv(struct dpu_fb_data_type *dpufd)
{
	static uint32_t dbv_threshold = (uint32_t)(-1);
	uint32_t dbv;
	uint16_t alpha;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	is_mask_layer_on = true;
	display_engine_local_hbm_get_dbv_threshold(dpufd, &dbv_threshold);

	/* 4095 indicates the maximum dbv */
	if (dbv_threshold > 4095) {
		DPU_FB_ERR("invalid dbv_threshold\n");
		return;
	}
	dbv = display_engine_get_mipi_level();
	DPU_FB_INFO("dbv:%d, dbv_threshold:%d\n", dbv, dbv_threshold);
	display_engine_local_hbm_alpha_log_print(dpufd, dbv_threshold);
	if (dbv >= dbv_threshold) {
		alpha = ALPHA_DEFAULT;
	} else {
		if (dbv >= sizeof(dpufd->de_info.alpha_map) / sizeof(uint16_t)) {
			DPU_FB_ERR("The dbv value cannot be mapped to alpha\n");
			alpha = ALPHA_DEFAULT;
		} else {
			DPU_FB_INFO("alpha:%d\n", dpufd->de_info.alpha_map[dbv]);
			alpha = dpufd->de_info.alpha_map[dbv];
		}
		dbv = dbv_threshold;
	}
	display_engine_local_hbm_set_dbv(dpufd, dbv);
	display_engine_alpha_set_inner(dpufd, alpha, FORCE_ENABLE);
	display_engine_local_hbm_set_circle(dpufd, true);
}

static int display_engine_local_hbm_color_set_by_grayscale(struct dpu_fb_data_type *dpufd, int grayscale)
{
	return 0;
}

static int display_engine_local_hbm_mmie_set_circle_alpha_dbv_color(struct dpu_fb_data_type *dpufd, int grayscale)
{
	int ret;

	/* 4095 indicates the maximum dbv */
	int dbv = 4095;

	ret = display_engine_local_hbm_color_set_by_grayscale(dpufd, grayscale);
	if (ret != 0) {
		DPU_FB_ERR("display_engine_local_hbm_color_set_by_grayscale error\n");
		return ret;
	}
	display_engine_local_hbm_set_dbv(dpufd, dbv);
	display_engine_alpha_set_inner(dpufd, ALPHA_DEFAULT, FORCE_ENABLE);
	display_engine_local_hbm_set_circle(dpufd, true);
	return ret;
}

static void handle_mask_layer_on(struct dpu_fb_data_type *dpufd,
	const int mask_delay_time_before_fp, struct dpu_fb_panel_data *pdata,
	const int status)
{
	uint64_t fp_delay_time_us = 0;

	DPU_FB_INFO("max change_status=%d, delay_time_before_fp =%d,"
		" StartTime = %ld us, LastTe= %ld us, diff =%ld\n",
		status, mask_delay_time_before_fp, ktime_to_us(ktime_get()),
		ktime_to_us(dpufd->te_timestamp),
		ktime_to_us(ktime_get()) - ktime_to_us(dpufd->te_timestamp));
	fp_delay_time_us = get_backlight_sync_delay_time_us(pdata, dpufd, false);
	if (fp_delay_time_us > 0)
		usleep_range(fp_delay_time_us, fp_delay_time_us);

	DPU_FB_INFO("ENTER HBM DelayEndTime = %ld us, LastTe= %ld us,"
		" delaytime = %ld us",
		ktime_to_us(ktime_get()), ktime_to_us(dpufd->te_timestamp),
		fp_delay_time_us);

	if (status == MASK_LAYER_SCREENON_ADDED) {
		pdata->lcd_set_hbm_for_screenon(dpufd->pdev,
			fp_hbm_enter_flag);
	} else {
		pdata->lcd_set_backlight_by_type_func(dpufd->pdev,
			fp_hbm_enter_flag);
		/* clear ddic alpha if needed */
		display_engine_alpha_set(dpufd, ALPHA_DEFAULT);
	}

	usleep_range(mask_delay_time_before_fp, mask_delay_time_before_fp);
	if (dpufd->de_param.ddic_irc.irc_enable != 0) {
		DPU_FB_INFO("lcd_kit_set_panel_irc(dpufd, false)+");
		display_engine_ddic_irc_set_inner(dpufd, false);
		DPU_FB_INFO("lcd_kit_set_panel_irc(dpufd, false)-");
	}
	dpufd->hbm_is_opened = true;

#if defined(CONFIG_HISI_FB_AOD) && !defined(CONFIG_HISI_FB_970)
	// To avoid XCC influencing the lightness of mask layer,
	// the config of XCC(include xcc enable state)
	// should be stored and cleaned temporarily.
	// XCC config will be ignored during mask layer exists,
	// by setting mask_layer_xcc_flag to 1.
	dpufd->mask_layer_xcc_flag = 1;
	clear_xcc_table(dpufd);
#endif
}

static void display_engine_local_hbm_exit_circle_alpha_dbv(struct dpu_fb_data_type *dpufd)
{
	int backlight_out = 0;

	backlight_out = dpufd->de_info.amoled_param.dc_lowac_fixed_dbv_thres;
	display_engine_effect_dbv_map(dpufd, &backlight_out);
	display_engine_local_hbm_set_dbv(dpufd, backlight_out);
	display_engine_alpha_set_inner(dpufd, dpufd->de_info.ddic_alpha, FORCE_DISABLE);
	display_engine_local_hbm_set_circle(dpufd, false);
	is_mask_layer_on = false;
}

static void handle_mask_layer_off(struct dpu_fb_data_type *dpufd,
	const int mask_delay_time_after_fp, struct dpu_fb_panel_data *pdata,
	const int status)
{
	uint64_t fp_delay_time_us = 0;

	DPU_FB_INFO("min change_status=%d, delay_time_after_fp =%d\n",
		status, mask_delay_time_after_fp);
	fp_delay_time_us = get_backlight_sync_delay_time_us(pdata, dpufd, false);
	if (fp_delay_time_us > 0)
		usleep_range(fp_delay_time_us, fp_delay_time_us);

	DPU_FB_INFO("EXIT HBM DelayEndTime = %ld us, LastTe= %ld us,"
		" delaytime = %ld us",
		ktime_to_us(ktime_get()), ktime_to_us(dpufd->te_timestamp),
		fp_delay_time_us);
	pdata->lcd_set_backlight_by_type_func(dpufd->pdev, fp_hbm_exit_flag);
	usleep_range(mask_delay_time_after_fp, mask_delay_time_after_fp);
	if (dpufd->de_param.ddic_irc.irc_enable != 0) {
		DPU_FB_INFO("lcd_kit_set_panel_irc(dpufd, true)+");
		display_engine_ddic_irc_set_inner(dpufd, true);
		DPU_FB_INFO("lcd_kit_set_panel_irc(dpufd, true)-");
	}
	dpufd->hbm_is_opened = false;

#if defined(CONFIG_HISI_FB_AOD) && !defined(CONFIG_HISI_FB_970)
	// restore XCC config(include XCC enable state) while mask layer disappear.
	restore_xcc_table(dpufd);
	dpufd->mask_layer_xcc_flag = 0;
#endif
}

void display_engine_effect_dbv_map(
	struct dpu_fb_data_type *dpufd, int *backlight_out)
{
	if (backlight_out == NULL) {
		DPU_FB_ERR("backlight_out is NULL!\n");
		return;
	}
	if (*backlight_out >= MAP_LUMLUT_LENGTH) {
		if (dpufd->panel_info.dbv_max)
			*backlight_out = dpufd->panel_info.dbv_max;
		else
			*backlight_out = dpufd->panel_info.bl_max;
		DPU_FB_INFO("backlight_out is outline!\n");
		return;
	}

	if (*backlight_out <= 0) {
		DPU_FB_INFO("backlight_out is set 0!\n");
		*backlight_out = 0;
		return;
	}

	if (dpufd->de_info.dbv_map[*backlight_out] == 0)
		DPU_FB_INFO("dbv map is 0\n");
	else
		*backlight_out = dpufd->de_info.dbv_map[*backlight_out];

	if (dpufd->panel_info.dbv_max > 0 &&
		(uint32_t)(*backlight_out) > dpufd->panel_info.dbv_max)
		*backlight_out = dpufd->panel_info.dbv_max;
}

void dpufb_mask_layer_backlight_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req_prev,
	dss_overlay_t *pov_req, bool *masklayer_maxbacklight_flag)
{
	struct dpu_fb_panel_data *pdata = NULL;
	int mask_delay_time_before_fp;
	int mask_delay_time_after_fp;
	bool parameter_valid = false;
	int status; // masklayer change status
	uint64_t real_te_interval;

	parameter_valid = hisi_mask_layer_backlight_config_check_parameter_valid(dpufd, pov_req_prev,pov_req);
	if (parameter_valid == false) {
		return;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (NULL == pdata) {
		DPU_FB_ERR("pdata is NULL");
		return;
	}

	if (pdata->lcd_set_backlight_by_type_func == NULL)
		return;
	real_te_interval = get_real_te_interval(dpufd);
	mask_delay_time_before_fp = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->mask_delay.delay_time_before_fp);
	mask_delay_time_after_fp = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->mask_delay.delay_time_after_fp);
	dpufd->masklayer_flag = pov_req->mask_layer_exist;

	status = hisi_masklayer_change_status(dpufd, pov_req_prev, pov_req);
	if (status == MASK_LAYER_NO_CHANGE)
		return;

	if (!dpufd->hbm_is_opened && (status == MASK_LAYER_COMMON_ADDED
		|| status == MASK_LAYER_SCREENON_ADDED)) {
		handle_mask_layer_on(dpufd, mask_delay_time_before_fp, pdata,
			status);
		if (status == MASK_LAYER_COMMON_ADDED)
			*masklayer_maxbacklight_flag = true;
	} else if (dpufd->hbm_is_opened && status == MASK_LAYER_REMOVED) {
		handle_mask_layer_off(dpufd, mask_delay_time_after_fp, pdata,
			status);
		*masklayer_maxbacklight_flag = false;
		de_ctrl_ic_dim(dpufd);
	} else if (status == CIRCLE_LAYER_ADDED) {
		DPU_FB_INFO("circle visible masklayer_change_status = %d\n", status);
		*masklayer_maxbacklight_flag = true;
	} else if (status == CIRCLE_LAYER_REMOVED) {
		DPU_FB_INFO("circle invisible masklayer_change_status = %d\n", status);
		*masklayer_maxbacklight_flag = false;
	}
}

static void dpufb_dc_skip_even_frame(struct dpu_fb_data_type *dpufd)
{
	(void)dpufd;
}

static void dpufb_dc_te_vsync_delay(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_panel_data *pdata = NULL;
	long te_delay_time;
	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return;
	}
	te_delay_time = get_backlight_sync_delay_time_us(pdata, dpufd, true);
	if (te_delay_time > 0)
		usleep_range(te_delay_time, te_delay_time);
	dpufb_dc_skip_even_frame(dpufd);
}

static void dpufb_dc_backlight_param_update(struct dpu_fb_data_type *dpufd) {
	struct dpu_panel_info *pinfo = NULL;

	dpufd->de_info.amoled_param.dc_brightness_dimming_enable_real =
		dpufd->de_info.amoled_param.
		dc_brightness_dimming_enable;
	dpufd->de_info.amoled_param.amoled_diming_enable =
		dpufd->de_info.amoled_param.
		amoled_enable_from_hal;
	dpufd->panel_info.dc_switch_xcc_updated = false;
	pinfo = get_panel_info(dpufd, dpufd->panel_info.disp_panel_id);
	if (pinfo == NULL) {
		DPU_FB_ERR("pinfo is NULL\n");
		return;
	}
	dpufd->de_info.blc_enable = pinfo->blc_enable_delayed;
	dpufd->de_info.blc_delta = pinfo->delta_bl_delayed;
	dpufd->de_info.ddic_alpha = pinfo->ddic_alpha_delayed;
}

int display_engine_alpha_set_inner(struct dpu_fb_data_type *dpufd, uint32_t alpha, int force_alpha_enable)
{
	int ret = 0;

	DPU_FB_DEBUG("alpha:%d, force_alpha_enable:%d\n", alpha, force_alpha_enable);
	return ret;
}

int display_engine_alpha_set(struct dpu_fb_data_type *dpufd, uint32_t alpha)
{
	int ret = 0;

	return ret;
}

struct dpu_panel_info *get_panel_info(struct dpu_fb_data_type *dpufd, int panel_id)
{
	struct dpu_panel_info *pinfo = NULL;

	if (dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE)
		get_lcd_panel_info(dpufd, &pinfo, panel_id);
	else
		pinfo = &dpufd->panel_info;

	return pinfo;
}

u32 display_engine_alpha_get_support(void)
{
	return 0;
}

u32 display_engine_force_delta_bl_update_get_support(void)
{
	return 0;
}

void dpufb_dc_backlight_config(struct dpu_fb_data_type *dpufd)
{
	bool need_sync = (dpufd->de_info.amoled_param.
		dc_brightness_dimming_enable_real !=
		dpufd->de_info.amoled_param.dc_brightness_dimming_enable) ||
		(dpufd->de_info.amoled_param.amoled_diming_enable !=
		dpufd->de_info.amoled_param.amoled_enable_from_hal);
	uint64_t bl_delay_time;
	uint64_t real_te_interval;
	if (need_sync && dpufd->dirty_region_updt_enable == 0 &&
		dpufd->panel_info.dc_switch_xcc_updated) {
		if (dpufd->bl_level > 0 && dpufd->backlight.bl_updated) {
			dpufb_dc_te_vsync_delay(dpufd);
			dpufb_dc_backlight_param_update(dpufd);
			down(&dpufd->brightness_esd_sem);
			dpufb_set_backlight(dpufd, dpufd->bl_level, true);
			up(&dpufd->brightness_esd_sem);
			if (dpufd->de_info.amoled_param.
				dc_backlight_delay_us == 0)
				return;
			real_te_interval = get_real_te_interval(dpufd);
			bl_delay_time = correct_time_based_on_fps(
				real_te_interval,
				dpufd->de_info.amoled_param.
				dc_backlight_delay_us);
			usleep_range(bl_delay_time, bl_delay_time);
		} else {
			dpufb_dc_backlight_param_update(dpufd);
		}
	}
}

// Display Add: active te single when using fingeprint , fix fingeprint blink question
void de_open_update_te()
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpufd = dpufd_list[PRIMARY_PANEL_IDX];

	if (dpufd == NULL) {
		DPU_FB_ERR("NULL pointer\n");
		return;
	}

	if (!de_fp_open_active) {
		down(&dpufd->blank_sem);
		if (!dpufd->panel_power_on) {
			DPU_FB_INFO("[effect] panel power off!\n");
			up(&dpufd->blank_sem);
			return;
		}
		dpufb_activate_vsync(dpufd);
		up(&dpufd->blank_sem);
		DPU_FB_INFO("de activate vsync: 1\n");
		de_fp_open_active = true;

		if (dpufd->de_updata_te_wq != NULL)
			queue_work(dpufd->de_updata_te_wq,
				&dpufd->de_updata_te_work);
	}
}

void de_close_update_te(struct work_struct *work)
{
	const int active_time = 17000;
	struct dpu_fb_data_type *dpufd = NULL;
	if (work == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return;
	}

	dpufd = container_of(work, struct dpu_fb_data_type, de_updata_te_work);
	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return;
	}
	if (de_fp_open_active) {
		usleep_range(active_time, active_time);
		dpufb_deactivate_vsync(dpufd);
		DPU_FB_INFO("de deactivate vsync: 0\n");
		de_fp_open_active = false;
	}
}

void de_ctrl_ic_dim(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_panel_data *pdata = NULL;

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return;
	}

	DPU_FB_INFO("ic dimmming is enable : %d",
		pdata->panel_info->ic_dim_ctrl_enable);
	if (pdata->panel_info->ic_dim_ctrl_enable == 1) {
		if (dpufd->de_updata_te_wq != NULL)
			queue_work(dpufd->de_updata_te_wq,
				&dpufd->de_open_ic_dim_work); // open ic dim
	}
}

void de_open_ic_dim(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	uint64_t delay_time;

	if (work == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return;
	}
	dpufd = container_of(work, struct dpu_fb_data_type,
		de_open_ic_dim_work);
	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return;
	}
	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return;
	}
	// there is not effect performance, so delay one more te time is safe
	delay_time = pdata->panel_info->mask_delay.delay_time_after_fp +
		pdata->panel_info->te_interval_us;
	usleep_range(delay_time, delay_time);
	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_INFO("fb%d, panel power off!\n", dpufd->index);
		up(&dpufd->blank_sem);
		return;
	}
	if (pdata->lcd_set_ic_dim_on != NULL) {
		dpufb_activate_vsync(dpufd);
		pdata->lcd_set_ic_dim_on(dpufd);
		dpufb_deactivate_vsync(dpufd);
		DPU_FB_INFO("ic dimming is opend!\n");
	} else {
		DPU_FB_ERR("lcd_set_ic_dim_on is NULL\n");
	}
	up(&dpufd->blank_sem);
}

void de_set_hbm_func(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	display_engine_hbm_param_t *param = NULL;
	int last_hbm_level = 0;
	static int last_mmi_level = 0;

	if (work == NULL) {
		DPU_FB_ERR("work is NULL\n");
		return;
	}
	dpufd = container_of(work, struct dpu_fb_data_type,
		de_set_hbm_func_work);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL Pointer\n");
		return;
	}
	param = &dpufd->de_param.hbm;
	if (param->mmi_level != last_mmi_level) {
		DPU_FB_INFO("mmi level %d->%d\n",
			last_mmi_level, param->mmi_level);
		last_mmi_level = param->mmi_level;
		if (pdata->lcd_set_hbm_for_mmi_func != NULL) {
			/* clear ddic alpha if needed */
			display_engine_alpha_set(dpufd, ALPHA_DEFAULT);
			pdata->lcd_set_hbm_for_mmi_func(dpufd->pdev,
				param->mmi_level);
		} else {
			DPU_FB_ERR("lcd_set_hbm_for_mmi_func is NULL\n");
		}
	} else {
		DPU_FB_INFO("hbm level %d->%d\n",
			dpufd->de_info.last_hbm_level, dpufd->de_info.hbm_level);
		last_hbm_level = dpufd->de_info.hbm_level;
		dpufd->de_info.hbm_dimming =
			(param->dimming == 1) ? true : false;
		if (pdata->lcd_hbm_set_func != NULL) {
			pdata->lcd_hbm_set_func(dpufd);
			dpufd->de_info.last_hbm_level = last_hbm_level;
		} else {
			DPU_FB_ERR("lcd_hbm_set_func is NULL\n");
		}
	}
}

static void init_hbm_workqueue(struct dpu_fb_data_type *dpufd)
{
	const char *wq_name = "fb0_display_engine_set_hbm";
	struct dpu_fb_panel_data *pdata = NULL;

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return;
	}

	if (pdata->panel_info->ic_dim_ctrl_enable ||
		pdata->panel_info->fps_lock_command_support ||
		(pdata->panel_info->te_interval_us > 0)) {
		dpufd->de_updata_te_wq =
			create_singlethread_workqueue(wq_name);
		if (dpufd->de_updata_te_wq == NULL) {
			DPU_FB_ERR("[effect] create de workqueue failed!\n");
			return;
		}
		INIT_WORK(&dpufd->de_updata_te_work, de_close_update_te);
		INIT_WORK(&dpufd->de_open_ic_dim_work, de_open_ic_dim);
		INIT_WORK(&dpufd->de_set_hbm_func_work, de_set_hbm_func);
	}
}

static void deinit_hbm_workqueue(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->de_updata_te_wq != NULL) {
		destroy_workqueue(dpufd->de_updata_te_wq);
		dpufd->de_updata_te_wq = NULL;
	}
}

static void set_hbm_by_workquen(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_panel_data *pdata = NULL;

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return;
	}
	if (dpufd->de_updata_te_wq != NULL)
		queue_work(dpufd->de_updata_te_wq,
			&dpufd->de_set_hbm_func_work);
}

void display_engine_set_mmi_fp_bl(struct dpu_fb_data_type *dpufd,
	uint32_t mmi_level)
{
	struct dpu_fb_panel_data *pdata = NULL;

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return;
	}
	DPU_FB_INFO("mmi_level = %d \n",mmi_level);
	dpufb_activate_vsync(dpufd);
	/* clear ddic alpha if needed */
	display_engine_alpha_set(dpufd, ALPHA_DEFAULT);
	pdata->lcd_set_hbm_for_mmi_func(dpufd->pdev, mmi_level);
	dpufb_deactivate_vsync(dpufd);
	if (mmi_level == 0)
		de_ctrl_ic_dim(dpufd);
}

int display_engine_hbm_param_set(struct dpu_fb_data_type *dpufd,
	display_engine_hbm_param_t *param)
{
	struct dpu_fb_panel_data *pdata = NULL;
	static int last_mmi_level = 0;

	if ((dpufd == NULL) || (param == NULL)) {
		DPU_FB_ERR("[effect] dpufd param is NULL Pointer\n");
		return -1;
	}
	dpufd->de_info.hbm_level = param->level;
	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}
	if (pdata->panel_info->fps_lock_command_support) {
		set_hbm_by_workquen(dpufd);
		return 0;
	}
	down(&dpufd->blank_sem);
	// set hbm for mmi finger test
	if (param->mmi_level != last_mmi_level) {
		DPU_FB_INFO("mmi level %d->%d\n",
			last_mmi_level, param->mmi_level);
		last_mmi_level = param->mmi_level;
		if (!dpufd->panel_power_on ||
			(pdata->lcd_set_hbm_for_mmi_func == NULL)) {
			up(&dpufd->blank_sem);
			return -1;
		} else
			display_engine_set_mmi_fp_bl(dpufd, param->mmi_level);
	} else {
		DPU_FB_INFO("hbm level %d->%d\n",
			dpufd->de_info.last_hbm_level, dpufd->de_info.hbm_level);
		dpufd->de_info.hbm_dimming =
			(param->dimming == 1) ? true : false;
		if (!dpufd->panel_power_on ||
			(pdata->lcd_hbm_set_func == NULL)) {
			up(&dpufd->blank_sem);
			return -1;
		} else {
			dpufb_activate_vsync(dpufd);
			pdata->lcd_hbm_set_func(dpufd);
			dpufb_deactivate_vsync(dpufd);
			dpufd->de_info.last_hbm_level = dpufd->de_info.hbm_level;
		}
	}
	up(&dpufd->blank_sem);
	return 0;
}

int display_engine_ddic_irc_set(struct dpu_fb_data_type *dpufd,
	display_engine_ddic_irc_param_t *param)
{
	int ret = 0;
	DPU_FB_INFO("[effect] enter\n");
	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}

	if (dpufd->pdev == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		DPU_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	down(&dpufd->blank_sem); // could take some time waiting this lock

	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -1;
		goto err_out;
	}

	dpufb_activate_vsync(dpufd);
	ret = display_engine_ddic_irc_set_inner(dpufd, (bool)param->irc_enable);
	dpufb_deactivate_vsync(dpufd);

err_out:
	up(&dpufd->blank_sem);
	return ret;
}

int display_engine_local_hbm_set(int grayscale)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	DPU_FB_INFO("grayscale = %d\n", grayscale);

	/* The maximum value of grayscale is 255 and the minimum value is 0 */
	if (grayscale < 0 || grayscale > 255) {
		DPU_FB_ERR("Invalid grayscale\n");
		return -1;
	}
	dpufd = dpufd_list[PRIMARY_PANEL_IDX];
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -1;
	}
	if (display_engine_local_hbm_get_support() == 0) {
		DPU_FB_ERR("local_hbm_support is 0\n");
		return -1;
	}

	down(&dpufd->blank_sem); /* could take some time waiting this lock */

	DPU_FB_INFO("power off protection sem down\n");
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		up(&dpufd->blank_sem);
		return -1;
	}

	dpufb_activate_vsync(dpufd);
	if (grayscale > 0) {
		ret = display_engine_local_hbm_color_set_by_grayscale(dpufd, grayscale);
		if (ret != 0) {
			DPU_FB_ERR("display_engine_local_hbm_color_set_by_grayscale error\n");
			dpufb_deactivate_vsync(dpufd);
			up(&dpufd->blank_sem);
			return ret;
		}
		display_engine_local_hbm_set_circle_alpha_dbv(dpufd);
		dpufb_masklayer_backlight_flag_config(dpufd, true);
	} else {
		display_engine_local_hbm_exit_circle_alpha_dbv(dpufd);
	}
	dpufb_deactivate_vsync(dpufd);
	up(&dpufd->blank_sem);

	DPU_FB_INFO("power off protection sem up\n");
	return 0;
}

int display_engine_local_hbm_mmie_set(struct dpu_fb_data_type *dpufd,
	struct display_engine_ddic_local_hbm_param *param)
{
	int ret = 0;

	if (!dpufd) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}

	if (!(dpufd->pdev)) {
		DPU_FB_ERR("[effect] dpufd->pdev is NULL Pointer\n");
		return -1;
	}

	if (!param) {
		DPU_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	DPU_FB_INFO("param->circle_grayscale = %d\n", param->circle_grayscale);

	/* The maximum value of grayscale is 255 and the minimum value is 0 */
	if (param->circle_grayscale < 0 || param->circle_grayscale > 255) {
		DPU_FB_ERR("Invalid param->circle_grayscale\n");
		return -1;
	}
	down(&(dpufd->blank_sem)); // could take some time waiting this lock

	if (!(dpufd->panel_power_on)) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -1;
		goto err_out;
	}

	dpufb_activate_vsync(dpufd);
	if (param->circle_grayscale > 0)
		display_engine_local_hbm_mmie_set_circle_alpha_dbv_color(dpufd,
			param->circle_grayscale);
	else
		display_engine_local_hbm_exit_circle_alpha_dbv(dpufd);
	dpufb_deactivate_vsync(dpufd);

err_out:
	up(&(dpufd->blank_sem));
	return ret;
}

int dpufb_display_engine_register(struct dpu_fb_data_type *dpufd) {
	int ret = 0;
	const char *wq_name = "fb0_display_engine";

	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return -EINVAL;
	}

	memset(&dpufd->de_info, 0, sizeof(dpufd->de_info));
	mutex_init(&dpufd->de_info.param_lock);

	dpufd->display_engine_wq = create_singlethread_workqueue(wq_name);
	if (dpufd->display_engine_wq == NULL) {
		DPU_FB_ERR("[effect] create display engine workqueue failed!\n");
		ret = -1;
		goto ERR_OUT;
	}
	INIT_WORK(&dpufd->display_engine_work, dpufb_display_engine_workqueue_handler);
	init_hbm_workqueue(dpufd);

ERR_OUT:
	return ret;
}

int dpufb_display_engine_unregister(struct dpu_fb_data_type *dpufd)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return -EINVAL;
	}

	if (dpufd->display_engine_wq != NULL) {
		destroy_workqueue(dpufd->display_engine_wq);
		dpufd->display_engine_wq = NULL;
	}
	mutex_lock(&dpufd->de_info.param_lock);
	mutex_unlock(&dpufd->de_info.param_lock);
	mutex_destroy(&dpufd->de_info.param_lock);
	deinit_hbm_workqueue(dpufd);

	return 0;  //lint !e454
}

// Creat masklayer backlight notify workqueue for UDfingerprint.
static void creat_masklayer_backlight_notify_wq(struct dpu_fb_data_type *dpufd)
{
	const char *wq_name = "masklayer_backlight_notify";

	dpufd->masklayer_backlight_notify_wq = create_singlethread_workqueue(wq_name);
	if (dpufd->masklayer_backlight_notify_wq == NULL) {
		DPU_FB_ERR("create masklayer backlight notify workqueue failed!\n");
		return;
	}
	INIT_WORK(&dpufd->masklayer_backlight_notify_work, dpufb_masklayer_backlight_notify_handler);
}

// Init masklayer backlight notify workqueue for UDfingerprint.
void hisi_init_masklayer_backlight_notify_wq(struct dpu_fb_data_type *dpufd)
{
#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V345) || \
	defined(CONFIG_HISI_FB_V360)

	// CONFIG_CMD_AOD_ENABLE
	if (is_mipi_cmd_panel(dpufd)) {
		creat_masklayer_backlight_notify_wq(dpufd);
		return;
	}
#endif

#if defined(CONFIG_HISI_FB_V320) || defined(CONFIG_HISI_FB_V330) || \
	defined(CONFIG_HISI_FB_V350)

	// CONFIG_VIDEO_AOD_ENABLE
	creat_masklayer_backlight_notify_wq(dpufd);
#endif
}
