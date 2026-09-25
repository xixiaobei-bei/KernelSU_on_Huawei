/* Copyright (c) 2019-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_overlay_utils.h"
#include "hisi_frame_rate_ctrl.h"
#ifdef CONFIG_LCD_KIT_DRIVER
#include "lcd_kit_adapt.h"
#endif

bool de_fp_open_active = false;
static const uint32_t te_interval_60_fps_us = 16667;
static const uint64_t te_interval_90_fps_us = 11111;
static const uint32_t fp_hbm_enter_flag = 1;
static const uint32_t fp_hbm_exit_flag = 2;
static const uint8_t fp_mmi_test_flag = 1;

static bool hisi_mask_layer_backlight_config_check_parameter_valid(struct hisi_fb_data_type *hisifd,
	dss_overlay_t *pov_req_prev, dss_overlay_t *pov_req)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null!\n");
		return false;
	}

	if (pov_req == NULL) {
		HISI_FB_DEBUG("pov_req is null!\n");
		return false;
	}

	if (pov_req_prev == NULL) {
		HISI_FB_DEBUG("pov_req_prev is null!\n");
		return false;
	}

	if (PRIMARY_PANEL_IDX != hisifd->index) {
		HISI_FB_DEBUG("fb%d, not support!\n", hisifd->index);
		return false;
	}

	return true;
}

static enum mask_layer_change_status hisi_masklayer_change_status(
	struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_prev,
	dss_overlay_t *pov)
{
	if (pov->mask_layer_exist == MASK_LAYER_COMMON_STATE
		&& !(pov_prev->mask_layer_exist)) {
		return MASK_LAYER_COMMON_ADDED;
	} else if (hisifd->bl_level > 0 && hisifd->hbm_need_to_open
		&& hisifd->backlight.bl_updated > 0
		&& pov->mask_layer_exist == MASK_LAYER_SCREENON_STATE
		&& !(pov_prev->mask_layer_exist)) {
		hisifd->hbm_need_to_open = false;
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

static uint64_t get_real_te_interval(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL, use default te interval");
		return te_interval_60_fps_us;
	}
	if (hisifd->panel_info.fps == FPS_90HZ) {
		return te_interval_90_fps_us;
	} else {
		return te_interval_60_fps_us;
	}
}

static uint64_t correct_time_based_on_fps(uint32_t real_te_interval,
	uint64_t time_60_fps)
{
	return time_60_fps * real_te_interval / te_interval_60_fps_us;
}


static uint64_t get_backlight_sync_delay_time_us(
	struct hisi_fb_panel_data *pdata, ktime_t te_timestamp,
	struct hisi_fb_data_type *hisifd)
{
	uint64_t left_thres_us;
	uint64_t right_thres_us;
	uint64_t te_interval;
	uint64_t diff_time;
	uint64_t current_time; //lint !e578
	uint64_t delayus = 0;
	uint32_t real_te_interval;

	if (pdata == NULL) {
		HISI_FB_ERR("pdata is NULL");
		return 0;
	}
	real_te_interval = get_real_te_interval(hisifd);

	left_thres_us = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->left_time_to_te_us);
	right_thres_us = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->right_time_to_te_us);
	te_interval = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->te_interval_us);
	if (te_interval == 0) {
		HISI_FB_ERR("te_interval is 0, used default time!");
		te_interval = te_interval_60_fps_us;
	}
	diff_time = ktime_to_us(ktime_get()) - ktime_to_us(te_timestamp);
	current_time = diff_time - diff_time / te_interval * te_interval;

	if (current_time < left_thres_us)
		delayus = left_thres_us - current_time;
	else if (current_time > right_thres_us)
		delayus = te_interval - current_time + left_thres_us;
	HISI_FB_INFO("backlight_sync current_time = %lu, diff_time = %lu us",
		current_time, diff_time);
	HISI_FB_INFO("backlight_sync delay = %lu us, left_threshold = %lu us",
		delayus, left_thres_us);
	HISI_FB_INFO("backlight_sync right_th = %lu us, te_interval = %lu",
		right_thres_us, te_interval);

	return delayus;
}

static void handle_mask_layer_on(struct hisi_fb_data_type *hisifd,
	const int mask_delay_time_before_fp, struct hisi_fb_panel_data *pdata,
	const int status)
{
	uint64_t fp_delay_time_us = 0;

	HISI_FB_INFO("max change_status=%d, delay_time_before_fp =%d,"
		" StartTime = %ld us, LastTe= %ld us, diff =%ld\n",
		status, mask_delay_time_before_fp, ktime_to_us(ktime_get()),
		ktime_to_us(hisifd->te_timestamp),
		ktime_to_us(ktime_get()) - ktime_to_us(hisifd->te_timestamp));
	fp_delay_time_us = get_backlight_sync_delay_time_us(pdata,
		hisifd->te_timestamp, hisifd);
	if (fp_delay_time_us > 0)
		usleep_range(fp_delay_time_us, fp_delay_time_us);

	HISI_FB_INFO("ENTER HBM DelayEndTime = %ld us, LastTe= %ld us,"
		" delaytime = %ld us",
		ktime_to_us(ktime_get()), ktime_to_us(hisifd->te_timestamp),
		fp_delay_time_us);

	if (status == MASK_LAYER_SCREENON_ADDED)
		pdata->lcd_set_hbm_for_screenon(hisifd->pdev,
			fp_hbm_enter_flag);
	else
		pdata->lcd_set_backlight_by_type_func(hisifd->pdev,
			fp_hbm_enter_flag);
	usleep_range(mask_delay_time_before_fp, mask_delay_time_before_fp);
	hisifd->hbm_is_opened = true;

#if defined(CONFIG_SH_AOD_ENABLE) && !defined(CONFIG_HISI_FB_970)
	// To avoid XCC influencing the lightness of mask layer,
	// the config of XCC(include xcc enable state)
	// should be stored and cleaned temporarily.
	// XCC config will be ignored during mask layer exists,
	// by setting mask_layer_xcc_flag to 1.
	hisifd->mask_layer_xcc_flag = 1;
	clear_xcc_table(hisifd);
#endif
}

static void handle_mask_layer_off(struct hisi_fb_data_type *hisifd,
	const int mask_delay_time_after_fp, struct hisi_fb_panel_data *pdata,
	const int status)
{
	uint64_t fp_delay_time_us = 0;

	HISI_FB_INFO("min change_status=%d, delay_time_after_fp =%d\n",
		status, mask_delay_time_after_fp);
	fp_delay_time_us = get_backlight_sync_delay_time_us(pdata,
		hisifd->te_timestamp, hisifd);
	if (fp_delay_time_us > 0)
		usleep_range(fp_delay_time_us, fp_delay_time_us);

	HISI_FB_INFO("EXIT HBM DelayEndTime = %ld us, LastTe= %ld us,"
		" delaytime = %ld us",
		ktime_to_us(ktime_get()), ktime_to_us(hisifd->te_timestamp),
		fp_delay_time_us);
	pdata->lcd_set_backlight_by_type_func(hisifd->pdev, fp_hbm_exit_flag);
	usleep_range(mask_delay_time_after_fp, mask_delay_time_after_fp);
	hisifd->hbm_is_opened = false;

#if defined(CONFIG_SH_AOD_ENABLE) && !defined(CONFIG_HISI_FB_970)
	// restore XCC config(include XCC enable state) while mask layer disappear.
	restore_xcc_table(hisifd);
	hisifd->mask_layer_xcc_flag = 0;
#endif
}

void hisifb_mask_layer_backlight_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req_prev,
	dss_overlay_t *pov_req, bool *masklayer_maxbacklight_flag)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int mask_delay_time_before_fp;
	int mask_delay_time_after_fp;
	bool parameter_valid = false;
	int status; // masklayer change status
	uint64_t real_te_interval;

	parameter_valid = hisi_mask_layer_backlight_config_check_parameter_valid(hisifd, pov_req_prev,pov_req);
	if (parameter_valid == false) {
		return;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return;
	}

	if (pdata->lcd_set_backlight_by_type_func == NULL)
		return;
	real_te_interval = get_real_te_interval(hisifd);
	mask_delay_time_before_fp = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->mask_delay.delay_time_before_fp);
	mask_delay_time_after_fp = correct_time_based_on_fps(real_te_interval,
		pdata->panel_info->mask_delay.delay_time_after_fp);
	hisifd->masklayer_flag = pov_req->mask_layer_exist;

	status = hisi_masklayer_change_status(hisifd, pov_req_prev, pov_req);
	if (status == MASK_LAYER_NO_CHANGE)
		return;

	if (!hisifd->hbm_is_opened && (status == MASK_LAYER_COMMON_ADDED
		|| status == MASK_LAYER_SCREENON_ADDED)) {
		handle_mask_layer_on(hisifd, mask_delay_time_before_fp, pdata,
			status);
		if (status == MASK_LAYER_COMMON_ADDED)
			*masklayer_maxbacklight_flag = true;
	} else if (hisifd->hbm_is_opened && status == MASK_LAYER_REMOVED) {
		handle_mask_layer_off(hisifd, mask_delay_time_after_fp, pdata,
			status);
		*masklayer_maxbacklight_flag = false;
		de_ctrl_ic_dim(hisifd);
	} else if (status == CIRCLE_LAYER_ADDED) {
		HISI_FB_INFO("circle visible masklayer_change_status = %d\n", status);
		*masklayer_maxbacklight_flag = true;
	} else if (status == CIRCLE_LAYER_REMOVED) {
		HISI_FB_INFO("circle invisible masklayer_change_status = %d\n", status);
		*masklayer_maxbacklight_flag = false;
	}
}

#ifdef CONFIG_LCD_KIT_DRIVER
static void hisifb_dc_skip_even_frame(struct hisi_fb_data_type *hisifd)
{
	uint64_t real_te_interval;
	if (!lcd_kit_is_current_frame_ok_to_set_backlight(hisifd)) {
		real_te_interval = get_real_te_interval(hisifd);
		HISI_FB_INFO("need to skip one frame, real_te_interval = %ld",
			real_te_interval);
		usleep_range(real_te_interval, real_te_interval);
	}
}
#endif

static void hisifb_dc_te_vsync_delay(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	long te_delay_time;
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("pdata is NULL");
		return;
	}
	te_delay_time = get_backlight_sync_delay_time_us(pdata,
		hisifd->te_timestamp, hisifd);
	if (te_delay_time > 0)
		usleep_range(te_delay_time, te_delay_time);
#ifdef CONFIG_LCD_KIT_DRIVER
	hisifb_dc_skip_even_frame(hisifd);
#endif
}

static void hisifb_dc_backlight_param_update(struct hisi_fb_data_type *hisifd) {
	hisifd->de_info.amoled_param.dc_brightness_dimming_enable_real =
		hisifd->de_info.amoled_param.
		dc_brightness_dimming_enable;
	hisifd->de_info.amoled_param.amoled_diming_enable =
		hisifd->de_info.amoled_param.
		amoled_enable_from_hal;
	hisifd->de_info.blc_enable = blc_enable_delayed;
	hisifd->de_info.blc_delta = delta_bl_delayed;
}

void hisifb_dc_backlight_config(struct hisi_fb_data_type *hisifd)
{
	bool need_sync = (hisifd->de_info.amoled_param.
		dc_brightness_dimming_enable_real !=
		hisifd->de_info.amoled_param.dc_brightness_dimming_enable) ||
		(hisifd->de_info.amoled_param.amoled_diming_enable !=
		hisifd->de_info.amoled_param.amoled_enable_from_hal);
	uint64_t bl_delay_time;
	uint64_t real_te_interval;
	if (need_sync && hisifd->dirty_region_updt_enable == 0 &&
		dc_switch_xcc_updated) {
		if (hisifd->bl_level > 0 && hisifd->backlight.bl_updated) {
			hisifb_dc_te_vsync_delay(hisifd);
			hisifb_dc_backlight_param_update(hisifd);
			down(&hisifd->brightness_esd_sem);
			hisifb_set_backlight(hisifd, hisifd->bl_level, true);
			up(&hisifd->brightness_esd_sem);
			if (hisifd->de_info.amoled_param.
				dc_backlight_delay_us == 0)
				return;
			real_te_interval = get_real_te_interval(hisifd);
			bl_delay_time = correct_time_based_on_fps(
				real_te_interval,
				hisifd->de_info.amoled_param.
				dc_backlight_delay_us);
			usleep_range(bl_delay_time, bl_delay_time);
		} else {
			hisifb_dc_backlight_param_update(hisifd);
		}
	}
}

// Display Add: active te single when using fingeprint , fix fingeprint blink question
void de_open_update_te()
{
	const char *wq_name = "fb0_display_engine_active_vsync";
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = hisifd_list[PRIMARY_PANEL_IDX];

	if (hisifd == NULL) {
		HISI_FB_ERR("NULL pointer\n");
		return;
	}

	if (hisifd->de_updata_te_wq == NULL) {
		hisifd->de_updata_te_wq =
			create_singlethread_workqueue(wq_name);
		if (hisifd->de_updata_te_wq == NULL) {
			HISI_FB_ERR("[effect] create display engine updata te workqueue failed!\n");
			return;
		}
		INIT_WORK(&hisifd->de_updata_te_work, de_close_update_te);
		INIT_WORK(&hisifd->de_open_ic_dim_work, de_open_ic_dim);
		INIT_WORK(&hisifd->de_set_hbm_func_work, de_set_hbm_func);
	}

	if (!de_fp_open_active) {
		down(&hisifd->blank_sem);
		if (!hisifd->panel_power_on) {
			HISI_FB_INFO("[effect] panel power off!\n");
			up(&hisifd->blank_sem);
			return;
		}
		hisifb_activate_vsync(hisifd);
		up(&hisifd->blank_sem);
		HISI_FB_INFO("de activate vsync: 1\n");
		de_fp_open_active = true;

		if (hisifd->de_updata_te_wq != NULL)
			queue_work(hisifd->de_updata_te_wq,
				&hisifd->de_updata_te_work);
	}
}

void de_close_update_te(struct work_struct *work)
{
	const int active_time = 17000;
	struct hisi_fb_data_type *hisifd = NULL;
	if (work == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	hisifd = container_of(work, struct hisi_fb_data_type, de_updata_te_work);
	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}
	if (de_fp_open_active) {
		usleep_range(active_time, active_time);
		hisifb_deactivate_vsync(hisifd);
		HISI_FB_INFO("de deactivate vsync: 0\n");
		de_fp_open_active = false;
	}
}

void de_ctrl_ic_dim(struct hisi_fb_data_type *hisifd)
{
	const char *wq_name = "fb0_display_engine_open_ic_dimming";
	struct hisi_fb_panel_data *pdata = NULL;

	if (hisifd->de_updata_te_wq == NULL) {
		hisifd->de_updata_te_wq =
			create_singlethread_workqueue(wq_name);
		if (hisifd->de_updata_te_wq == NULL) {
			HISI_FB_ERR("[effect] create display engine updata te workqueue failed!\n");
			return;
		}
		INIT_WORK(&hisifd->de_updata_te_work, de_close_update_te);
		INIT_WORK(&hisifd->de_open_ic_dim_work, de_open_ic_dim);
		INIT_WORK(&hisifd->de_set_hbm_func_work, de_set_hbm_func);
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("pdata is NULL");
		return;
	}

	HISI_FB_INFO("ic dimmming is enable : %d",
		pdata->panel_info->ic_dim_ctrl_enable);
	if (pdata->panel_info->ic_dim_ctrl_enable == 1) {
		if (hisifd->de_updata_te_wq != NULL)
			queue_work(hisifd->de_updata_te_wq,
				&hisifd->de_open_ic_dim_work); // open ic dim
	}
}

void de_open_ic_dim(struct work_struct *work)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;
	uint64_t delay_time;

	if (work == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}
	hisifd = container_of(work, struct hisi_fb_data_type,
		de_open_ic_dim_work);
	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return;
	}
	// there is not effect performance, so delay one more te time is safe
	delay_time = pdata->panel_info->mask_delay.delay_time_after_fp +
		pdata->panel_info->te_interval_us;
	usleep_range(delay_time, delay_time);
	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_INFO("fb%d, panel power off!\n", hisifd->index);
		up(&hisifd->blank_sem);
		return;
	}
	if (pdata->lcd_set_ic_dim_on != NULL) {
		hisifb_activate_vsync(hisifd);
		pdata->lcd_set_ic_dim_on(hisifd);
		hisifb_deactivate_vsync(hisifd);
		HISI_FB_INFO("ic dimming is opend!\n");
	} else {
		HISI_FB_ERR("lcd_set_ic_dim_on is NULL\n");
	}
	up(&hisifd->blank_sem);
}

void de_set_hbm_func(struct work_struct *work)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;
	display_engine_hbm_param_t *param = NULL;
	int last_hbm_level = 0;

	if (work == NULL) {
		HISI_FB_ERR("work is NULL\n");
		return;
	}
	hisifd = container_of(work, struct hisi_fb_data_type,
		de_set_hbm_func_work);
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL\n");
		return;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("pdata is NULL Pointer\n");
		return;
	}
	param = &hisifd->de_param.hbm;
	if (param->mmi_flag == fp_mmi_test_flag) {
		if (pdata->lcd_set_hbm_for_mmi_func != NULL)
			pdata->lcd_set_hbm_for_mmi_func(hisifd->pdev,
				param->level);
		else
			HISI_FB_ERR("lcd_set_hbm_for_mmi_func is NULL\n");
	} else {
		last_hbm_level = hisifd->de_info.hbm_level;
		hisifd->de_info.hbm_dimming =
			(param->dimming == 1) ? true : false;
		if (pdata->lcd_hbm_set_func != NULL) {
			pdata->lcd_hbm_set_func(hisifd);
			hisifd->de_info.last_hbm_level = last_hbm_level;
		} else {
			HISI_FB_ERR("lcd_hbm_set_func is NULL\n");
		}
	}
}

static void set_hbm_by_workquen(struct hisi_fb_data_type *hisifd)
{
	const char *wq_name = "fb0_display_engine_set_hbm";
	struct hisi_fb_panel_data *pdata = NULL;

	if (hisifd->de_updata_te_wq == NULL) {
		hisifd->de_updata_te_wq =
			create_singlethread_workqueue(wq_name);
		if (hisifd->de_updata_te_wq == NULL) {
			HISI_FB_ERR("[effect] create de workqueue failed!\n");
			return;
		}
		INIT_WORK(&hisifd->de_updata_te_work, de_close_update_te);
		INIT_WORK(&hisifd->de_open_ic_dim_work, de_open_ic_dim);
		INIT_WORK(&hisifd->de_set_hbm_func_work, de_set_hbm_func);
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("pdata is NULL");
		return;
	}
	if (hisifd->de_updata_te_wq != NULL)
		queue_work(hisifd->de_updata_te_wq,
			&hisifd->de_set_hbm_func_work);
}

void display_engine_set_mmi_fp_bl(struct hisi_fb_data_type *hisifd,
	uint32_t mmi_level)
{
	struct hisi_fb_panel_data *pdata = NULL;

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return;
	}
	hisifb_activate_vsync(hisifd);
	pdata->lcd_set_hbm_for_mmi_func(hisifd->pdev, mmi_level);
	hisifb_deactivate_vsync(hisifd);
	if (mmi_level == 0)
		de_ctrl_ic_dim(hisifd);
}

int display_engine_hbm_param_set(struct hisi_fb_data_type *hisifd,
	display_engine_hbm_param_t *param)
{
	int ret = 0;
	struct hisi_fb_panel_data *pdata = NULL;

	if ((hisifd == NULL) || (param == NULL)) {
		HISI_FB_ERR("[effect] hisifd param is NULL Pointer\n");
		return -1;
	}
	hisifd->de_info.hbm_level = param->level;
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL Pointer\n");
		return -1;
	}
	if (pdata->panel_info->fps_lock_command_support) {
		set_hbm_by_workquen(hisifd);
		return ret;
	}
	down(&hisifd->blank_sem);
	// set hbm for mmi finger test
	if (param->mmi_flag == fp_mmi_test_flag) {
		if (!hisifd->panel_power_on ||
			(pdata->lcd_set_hbm_for_mmi_func == NULL)) {
			HISI_FB_DEBUG("fb%d, power off!\n", hisifd->index);
			ret = -1;
			goto err_out;
		} else {
			display_engine_set_mmi_fp_bl(hisifd, param->level);
		}
	} else {
		hisifd->de_info.hbm_dimming =
			(param->dimming == 1) ? true : false;
		if (!hisifd->panel_power_on ||
			(pdata->lcd_hbm_set_func == NULL)) {
			HISI_FB_DEBUG("fb%d, power off!\n", hisifd->index);
			ret = -1;
			goto err_out;
		} else {
			hisifb_activate_vsync(hisifd);
			pdata->lcd_hbm_set_func(hisifd);
			hisifb_deactivate_vsync(hisifd);
		}
	}
err_out:
	hisifd->de_info.last_hbm_level = hisifd->de_info.hbm_level;
	up(&hisifd->blank_sem);
	return ret;
}
