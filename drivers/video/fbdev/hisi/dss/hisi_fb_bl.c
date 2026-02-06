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

#include "hisi_fb.h"
#include <linux/leds.h>

#ifdef CONFIG_HW_ZEROHUNG
#include <chipset_common/hwzrhung/hung_wp_screen.h>
#endif

static int lcd_backlight_registered;
static unsigned int is_recovery_mode;
static int is_no_fastboot_bl_enable;

#define MSEC_PER_FRAME 17
void dpufb_set_backlight(struct dpu_fb_data_type *dpufd, uint32_t bkl_lvl, bool enforce)
{
	struct dpu_fb_panel_data *pdata = NULL;
	uint32_t temp = bkl_lvl;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");
	dpu_check_and_no_retval(!dpufd->pdev, ERR, "dpufd->pdev is NULL\n");

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	dpu_check_and_no_retval(!pdata, ERR, "pdata is NULL\n");

	if (!dpufd->panel_power_on || !dpufd->backlight.bl_updated) {
		dpufd->bl_level = bkl_lvl;
		DPU_FB_INFO("set_bl return: panel_power_on = %d, backlight.bl_updated = %d! bkl_lvl = %d\n",
			dpufd->panel_power_on, dpufd->backlight.bl_updated, bkl_lvl);
		return;
	}

	if (pdata->set_backlight) {
		if (dpufd->backlight.bl_level_old == temp && !enforce) {
			dpufd->bl_level = bkl_lvl;
			DPU_FB_INFO("set_bl return: bl_level_old = %d, current_bl_level = %d, enforce = %d!\n",
				dpufd->backlight.bl_level_old, temp, enforce);
			return;
		}

#ifdef CONFIG_HW_ZEROHUNG
		hung_wp_screen_setbl(temp);
#endif
		if (dpufd->backlight.bl_level_old == 0)
			DPU_FB_INFO("backlight level = %d\n", bkl_lvl);

		dpufd->bl_level = bkl_lvl;

		if (dpufd->panel_info.bl_set_type & BL_SET_BY_MIPI) {
			dpufb_set_vsync_activate_state(dpufd, true);
			dpufb_activate_vsync(dpufd);
		}

		pdata->set_backlight(dpufd->pdev, bkl_lvl);

		if (dpufd->panel_info.bl_set_type & BL_SET_BY_MIPI) {
			dpufb_set_vsync_activate_state(dpufd, false);
			dpufb_deactivate_vsync(dpufd);
			if (bkl_lvl > 0)
				dpufd->secure_ctrl.have_set_backlight = true;
			else
				dpufd->secure_ctrl.have_set_backlight = false;
		}
		dpufd->backlight.bl_timestamp = ktime_get();
		dpufd->backlight.bl_level_old = temp;
	}
}

#ifdef CONFIG_HISI_FB_BACKLIGHT_DELAY
static void dpufb_bl_workqueue_handler(struct work_struct *work)
#else
static void dpufb_bl_workqueue_handler(struct dpu_fb_data_type *dpufd)
#endif
{
	struct dpu_fb_panel_data *pdata = NULL;
#ifdef CONFIG_HISI_FB_BACKLIGHT_DELAY
	struct dpufb_backlight *pbacklight = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	pbacklight = container_of(to_delayed_work(work), struct dpufb_backlight, bl_worker);  /*lint !e666*/
	dpu_check_and_no_retval(!pbacklight, ERR, "pbacklight is NULL\n");

	dpufd = container_of(pbacklight, struct dpu_fb_data_type, backlight);
#endif

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (!pdata) {
		DPU_FB_ERR("pdata is NULL\n");
		return;
	}

	if (dpufd->online_play_count < ONLINE_PLAY_LOG_PRINTF)
		DPU_FB_INFO("[online_play_count = %d] begin bl_updated = %d, frame_updated = %d.\n",
			dpufd->online_play_count, dpufd->backlight.bl_updated, dpufd->backlight.frame_updated);

	if (!dpufd->backlight.bl_updated) {
		down(&dpufd->blank_sem);

		if (dpufd->backlight.frame_updated == 0) {
			up(&dpufd->blank_sem);
			return;
		}

		dpufd->backlight.frame_updated = 0;
		down(&dpufd->brightness_esd_sem);

		dpufd->backlight.bl_updated = 1;
		if (dpufd->online_play_count < ONLINE_PLAY_LOG_PRINTF)
			DPU_FB_INFO("[online_play_count = %d] set bl_updated = 1.\n", dpufd->online_play_count);

		if (is_recovery_mode) {
			/*
			 * fix the bug:recovery interface app can not set backlight successfully
			 * if backlight not be set at recovery,driver will set default value
			 * otherwise driver not set anymore
			 */
			if (dpufd->bl_level == 0)
				dpufd->bl_level = dpufd->panel_info.bl_default;
		} else {
			if (!is_fastboot_display_enable() && !is_no_fastboot_bl_enable) {
				is_no_fastboot_bl_enable = 1;
				dpufd->bl_level = dpufd->panel_info.bl_default;
			}
		}

		dpufb_set_backlight(dpufd, dpufd->bl_level, false);
		up(&dpufd->brightness_esd_sem);
		up(&dpufd->blank_sem);
	}
	if (dpufd->online_play_count < ONLINE_PLAY_LOG_PRINTF)
		DPU_FB_INFO("[online_play_count = %d] end bl_updated = %d, frame_updated = %d.\n",
			dpufd->online_play_count, dpufd->backlight.bl_updated, dpufd->backlight.frame_updated);
}

void dpufb_backlight_update(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_panel_data *pdata = NULL;
	unsigned long backlight_duration = 2 * MSEC_PER_FRAME; /* default delay two frame */

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is null!\n");
	dpu_check_and_no_retval(!dpufd->pdev, ERR, "dpufd->pdev is null!\n");

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	dpu_check_and_no_retval(!pdata, ERR, "pdata is null!\n");

	if (dpufd->online_play_count < ONLINE_PLAY_LOG_PRINTF)
		DPU_FB_INFO("[online_play_count = %d] panel_power_on = %d, bl_updated = %d.\n",
			dpufd->online_play_count, dpufd->panel_power_on, dpufd->backlight.bl_updated);

	if (!dpufd->backlight.bl_updated && dpufd->panel_power_on) {
		dpufd->backlight.frame_updated = 1;
#ifdef CONFIG_HISI_FB_BACKLIGHT_DELAY
		if (dpufd->panel_info.delay_set_bl_thr_support)
			backlight_duration = dpufd->panel_info.delay_set_bl_thr * MSEC_PER_FRAME;

		schedule_delayed_work(&dpufd->backlight.bl_worker, msecs_to_jiffies(backlight_duration));
#else
		dpufb_bl_workqueue_handler(dpufd);
#endif

		DPU_FB_INFO("backlight_duration = %lu\n", backlight_duration);
	}
}

void dpufb_backlight_cancel(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_panel_data *pdata = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is null!\n");
	dpu_check_and_no_retval(!dpufd->pdev, ERR, "dpufd->pdev is null!\n");

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	dpu_check_and_no_retval(!pdata, ERR, "pdata is null!\n");

#ifdef CONFIG_HISI_FB_BACKLIGHT_DELAY
	cancel_delayed_work(&dpufd->backlight.bl_worker);
#endif
	dpufd->backlight.bl_updated = 0;
	dpufd->backlight.bl_level_old = 0;
	dpufd->backlight.frame_updated = 0;
	dpufd->online_play_count = 0;
	if (dpufd->online_play_count < ONLINE_PLAY_LOG_PRINTF)
		DPU_FB_INFO("[online_play_count = %d] set bl_updated = 0.\n", dpufd->online_play_count);

	if (pdata->set_backlight) {
		dpufd->bl_level = 0;
#ifdef CONFIG_HW_ZEROHUNG
		hung_wp_screen_setbl(dpufd->bl_level);
#endif
		if (dpufd->panel_info.bl_set_type & BL_SET_BY_MIPI)
			dpufb_activate_vsync(dpufd);
		pdata->set_backlight(dpufd->pdev, dpufd->bl_level);
		if (dpufd->panel_info.bl_set_type & BL_SET_BY_MIPI)
			dpufb_deactivate_vsync(dpufd);
	}
}

static void hisi_fb_set_bl_brightness(struct led_classdev *led_cdev,
	enum led_brightness value)
{
	struct dpu_fb_data_type *dpufd = NULL;
	int bl_lvl;

	if (!led_cdev) {
		DPU_FB_ERR("NULL pointer!\n");
		return;
	}

	dpufd = dev_get_drvdata(led_cdev->dev->parent);
	if (!dpufd) {
		DPU_FB_ERR("NULL pointer!\n");
		return;
	}

	if (value > dpufd->panel_info.bl_max)
		value = dpufd->panel_info.bl_max;

	/* This maps android backlight level 0 to 255 into */
	/* driver backlight level 0 to bl_max with rounding */
	bl_lvl = (2 * value * dpufd->panel_info.bl_max + dpufd->panel_info.bl_max) / (2 * dpufd->panel_info.bl_max);
	if (!bl_lvl && value)
		bl_lvl = 1;
	down(&dpufd->brightness_esd_sem);
	dpufb_set_backlight(dpufd, bl_lvl, false);
	up(&dpufd->brightness_esd_sem);
}

static struct led_classdev backlight_led = {
	.name = DEV_NAME_LCD_BKL,
	.brightness_set = hisi_fb_set_bl_brightness,
};

void dpufb_backlight_register(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return;
	}

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		dev_err(&pdev->dev, "dpufd is NULL");
		return;
	}

	dpufd->backlight.bl_updated = 0;
	dpufd->backlight.frame_updated = 0;
	dpufd->backlight.bl_level_old = 0;
	sema_init(&dpufd->backlight.bl_sem, 1);
#ifdef CONFIG_HISI_FB_BACKLIGHT_DELAY
	INIT_DELAYED_WORK(&dpufd->backlight.bl_worker, dpufb_bl_workqueue_handler);
#endif

	if (lcd_backlight_registered)
		return;

	is_recovery_mode = get_boot_into_recovery_flag();

	backlight_led.brightness = dpufd->panel_info.bl_default;
	backlight_led.max_brightness = dpufd->panel_info.bl_max;
	/* android supports only one lcd-backlight/lcd for now */
#ifdef CONFIG_LEDS_CLASS
	if (led_classdev_register(&pdev->dev, &backlight_led)) {
		dev_err(&pdev->dev, "led_classdev_register failed!\n");
		return;
	}
#endif

	lcd_backlight_registered = 1;
}

void dpufb_backlight_unregister(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return;
	}
	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		dev_err(&pdev->dev, "dpufd is NULL\n");
		return;
	}

	if (lcd_backlight_registered) {
		lcd_backlight_registered = 0;
#ifdef CONFIG_LEDS_CLASS
		led_classdev_unregister(&backlight_led);
#endif
	}
}

struct bl_flicker_detector_data {
	unsigned long timestamp_jiffies;
	int upper_bl_value;
	int upper_bl_weber;
	int device_bl_value;
	int algo_delta_bl_value;
};

struct bl_flicker_detector_info {
	display_engine_flicker_detector_config_t config;
	struct bl_flicker_detector_data *ring_buffer;
	int ring_buffer_length;
	int ring_buffer_index;
	int time_window_length_jiffies;

	bool is_first_flag;
	int upper_bl_value_cur;
	int upper_bl_value_last;
	int device_bl_value_last;
	int algo_delta_bl_value_cur;
};

static struct bl_flicker_detector_info g_flicker_detector;

static inline int ring_buffer_index_increase(int index, int size)
{
	return (++index > (size - 1) ? 0 : index);
}

static inline int ring_buffer_index_decrease(int index, int size)
{
	return (--index < 0 ? (size - 1) : index);
}

static void bl_flicker_detector_ring_buffer_add(struct bl_flicker_detector_data data)
{
	g_flicker_detector.ring_buffer_index =
		ring_buffer_index_increase(g_flicker_detector.ring_buffer_index, g_flicker_detector.ring_buffer_length);
	g_flicker_detector.ring_buffer[g_flicker_detector.ring_buffer_index] = data;
}

static void bl_flicker_detector_ring_buffer_get_weber_range(unsigned long jiffies_cur,
	int *weber_min, int *weber_max)
{
	int min_get = 0;
	int max_get = 0;
	int index = g_flicker_detector.ring_buffer_index;
	unsigned long jiffies_threshold = jiffies_cur - g_flicker_detector.time_window_length_jiffies;
	unsigned long jiffies_get;
	int weber;
	int len;

	if (!weber_min || !weber_max)
		return;

	for (len = 0; len < g_flicker_detector.ring_buffer_length; len++) {
		jiffies_get = g_flicker_detector.ring_buffer[index].timestamp_jiffies;
		if (!jiffies_get || time_before(jiffies_get, jiffies_threshold))
			break;

		weber = g_flicker_detector.ring_buffer[index].upper_bl_weber;
		min_get = min(min_get, weber);
		max_get = max(max_get, weber);
		index = ring_buffer_index_decrease(index, g_flicker_detector.ring_buffer_length);
	}

	*weber_min = min_get;
	*weber_max = max_get;
}

static void bl_flicker_detector_ring_buffer_dump(unsigned long jiffies_cur)
{
	int dump_len = 0;
	int msecs_start;
	int index = g_flicker_detector.ring_buffer_index;
	unsigned long jiffies_threshold = jiffies_cur - g_flicker_detector.time_window_length_jiffies;
	unsigned long jiffies_get;
	int len;
	int dump_index;

	for (len = 0; len < g_flicker_detector.ring_buffer_length; len++) {
		jiffies_get = g_flicker_detector.ring_buffer[index].timestamp_jiffies;
		if (!jiffies_get || time_before(jiffies_get, jiffies_threshold))
			break;

		index = ring_buffer_index_decrease(index, g_flicker_detector.ring_buffer_length);
	}
	if (len == 0) {
		DPU_FB_WARNING("timestamp_jiffies load error\n");
		return;
	}

	dump_index = ring_buffer_index_increase(index, g_flicker_detector.ring_buffer_length);
	msecs_start = jiffies_to_msecs(g_flicker_detector.ring_buffer[dump_index].timestamp_jiffies);
	while (++dump_len <= len) {
		DPU_FB_WARNING("^%d^%d^%d^%d\n",
			jiffies_to_msecs(g_flicker_detector.ring_buffer[dump_index].timestamp_jiffies) - msecs_start,
			g_flicker_detector.ring_buffer[dump_index].upper_bl_value,
			g_flicker_detector.ring_buffer[dump_index].device_bl_value,
			g_flicker_detector.ring_buffer[dump_index].algo_delta_bl_value);
		dump_index = ring_buffer_index_increase(dump_index, g_flicker_detector.ring_buffer_length);
	}
}

#define FLICKER_DETECTOR_WEBER_RESOLUTION 1000
#define FLICKER_DETECTOR_WEBER_MAX 1000000
#define FLICKER_DETECTOR_FIRST_BL_VALUE 10000

static inline int bl_flicker_detector_calc_weber(int cur_level, int last_level)
{
	if (last_level == 0)
		return FLICKER_DETECTOR_WEBER_MAX;

	return FLICKER_DETECTOR_WEBER_RESOLUTION * (cur_level - last_level) / last_level;
}

static void bl_flicker_detector_save_first_data(unsigned long jiffies_cur)
{
	struct bl_flicker_detector_data data;

	data.timestamp_jiffies = jiffies_cur;
	data.upper_bl_value = FLICKER_DETECTOR_FIRST_BL_VALUE;
	data.upper_bl_weber = -FLICKER_DETECTOR_WEBER_RESOLUTION;
	data.device_bl_value = FLICKER_DETECTOR_FIRST_BL_VALUE;
	data.algo_delta_bl_value = 0;
	bl_flicker_detector_ring_buffer_add(data);
}

static void bl_flicker_detector_save_data(unsigned long jiffies_cur, int upper_bl_value,
	int device_bl_value, int algo_delta_bl_value)
{
	struct bl_flicker_detector_data data;

	data.timestamp_jiffies = jiffies_cur;
	data.upper_bl_value = upper_bl_value;
	data.upper_bl_weber = bl_flicker_detector_calc_weber(upper_bl_value, g_flicker_detector.upper_bl_value_last);
	data.device_bl_value = device_bl_value;
	data.algo_delta_bl_value = algo_delta_bl_value;
	bl_flicker_detector_ring_buffer_add(data);
}

static void bl_flicker_detector_calc_weber_threshold(unsigned long jiffies_cur,
	int *weber_threshold_min, int *weber_threshold_max)
{
	int weber_min = 0;
	int weber_max = 0;

	if (!weber_threshold_min || !weber_threshold_max)
		return;

	bl_flicker_detector_ring_buffer_get_weber_range(jiffies_cur, &weber_min, &weber_max);
	weber_min = min(weber_min, 0);
	weber_max = max(weber_max, 0);
	weber_min -= g_flicker_detector.config.weber_threshold;
	weber_max += g_flicker_detector.config.weber_threshold;
	*weber_threshold_min = weber_min;
	*weber_threshold_max = weber_max;
}

static bool bl_flicker_detector_skip_detect(int upper_level, int device_level)
{
	int delta_threshold = 1;
	int delta;

	if (upper_level < g_flicker_detector.config.low_level_upper_bl_value_threshold)
		delta_threshold = g_flicker_detector.config.low_level_device_bl_delta_threshold;

	delta = device_level - g_flicker_detector.device_bl_value_last;
	return abs(delta) <= delta_threshold;
}

#define FLICKER_DETECTOR_VALID_TIME_WINDOW_MS_MIN 500
#define FLICKER_DETECTOR_VALID_TIME_WINDOW_MS_MAX 2000
#define FLICKER_DETECTOR_VALID_WEBER_THRESHOLD_MIN 0
#define FLICKER_DETECTOR_VALID_WEBER_THRESHOLD_MAX 500
#define FLICKER_DETECTOR_BUFFER_LENGTH_PER_SECOND 100

void bl_flicker_detector_init(display_engine_flicker_detector_config_t config)
{
	struct bl_flicker_detector_data *buffer = NULL;
	int buffer_length;
	unsigned long buffer_size;

	if (g_flicker_detector.config.detect_enable) {
		DPU_FB_ERR("already inited!\n");
		return;
	}

	if (!config.detect_enable) {
		DPU_FB_ERR("detect_enable=false, needn't init!\n");
		return;
	}

	if (config.time_window_length_ms < FLICKER_DETECTOR_VALID_TIME_WINDOW_MS_MIN ||
		config.time_window_length_ms > FLICKER_DETECTOR_VALID_TIME_WINDOW_MS_MAX) {
		DPU_FB_ERR("time_window_length_ms=%d out of range\n", config.time_window_length_ms);
		return;
	}

	if (config.weber_threshold <= FLICKER_DETECTOR_VALID_WEBER_THRESHOLD_MIN ||
		config.weber_threshold >= FLICKER_DETECTOR_VALID_WEBER_THRESHOLD_MAX) {
		DPU_FB_ERR("weber_threshold=%d out of range\n", config.weber_threshold);
		return;
	}

	/* MS_PER_SEC */
	buffer_length = config.time_window_length_ms * FLICKER_DETECTOR_BUFFER_LENGTH_PER_SECOND / 1000;
	buffer_size = sizeof(struct bl_flicker_detector_data) * buffer_length;
	buffer = vmalloc(buffer_size);
	if (!buffer) {
		DPU_FB_ERR("vmalloc size %ld failed!\n", buffer_size);
		return;
	}

	g_flicker_detector.config.dump_enable = config.dump_enable;
	g_flicker_detector.config.time_window_length_ms = config.time_window_length_ms;
	g_flicker_detector.config.weber_threshold = config.weber_threshold;
	g_flicker_detector.config.low_level_upper_bl_value_threshold = config.low_level_upper_bl_value_threshold;
	g_flicker_detector.config.low_level_device_bl_delta_threshold = config.low_level_device_bl_delta_threshold;

	g_flicker_detector.ring_buffer = buffer;
	g_flicker_detector.ring_buffer_length = buffer_length;
	g_flicker_detector.time_window_length_jiffies =
		(int)msecs_to_jiffies(g_flicker_detector.config.time_window_length_ms);
	g_flicker_detector.is_first_flag = true;

	DPU_FB_INFO("window_length=%d, jiffies=%d, weber=%d, low_level_upper_bl=%d, low_level_device_delta=%d\n",
		g_flicker_detector.config.time_window_length_ms,
		g_flicker_detector.time_window_length_jiffies,
		g_flicker_detector.config.weber_threshold,
		g_flicker_detector.config.low_level_upper_bl_value_threshold,
		g_flicker_detector.config.low_level_device_bl_delta_threshold);

	g_flicker_detector.config.detect_enable = true;
}

void bl_flicker_detector_collect_upper_bl(int level)
{
	g_flicker_detector.upper_bl_value_cur = level;
}

void bl_flicker_detector_collect_algo_delta_bl(int level)
{
	g_flicker_detector.algo_delta_bl_value_cur = level;
}

void bl_flicker_detector_collect_device_bl(int level)
{
	int upper_bl_value_cur = g_flicker_detector.upper_bl_value_cur;
	int algo_delta_bl_value_cur = g_flicker_detector.algo_delta_bl_value_cur;
	int device_bl_value_cur = level;
	int weber_threshold_min = 0;
	int weber_threshold_max = 0;
	int device_bl_weber;
	unsigned long jiffies_cur;

	if (!g_flicker_detector.config.detect_enable)
		return;

	jiffies_cur = jiffies;

	if (g_flicker_detector.is_first_flag) {
		bl_flicker_detector_save_first_data(jiffies_cur);
		g_flicker_detector.is_first_flag = false;
	}

	bl_flicker_detector_save_data(jiffies_cur, upper_bl_value_cur, device_bl_value_cur, algo_delta_bl_value_cur);

	if (bl_flicker_detector_skip_detect(upper_bl_value_cur, device_bl_value_cur)) {
		DPU_FB_DEBUG("upper=%d, device=%d, algo_delta=%d skip detect\n",
			upper_bl_value_cur, device_bl_value_cur, algo_delta_bl_value_cur);
		g_flicker_detector.upper_bl_value_last = g_flicker_detector.upper_bl_value_cur;
		g_flicker_detector.device_bl_value_last = device_bl_value_cur;
		return;
	}

	DPU_FB_DEBUG("upper=%d, device=%d, algo_delta=%d\n",
		upper_bl_value_cur, device_bl_value_cur, algo_delta_bl_value_cur);

	device_bl_weber = bl_flicker_detector_calc_weber(device_bl_value_cur, g_flicker_detector.device_bl_value_last);
	bl_flicker_detector_calc_weber_threshold(jiffies_cur, &weber_threshold_min, &weber_threshold_max);
	if (device_bl_weber < weber_threshold_min || device_bl_weber > weber_threshold_max) {
		DPU_FB_WARNING("flicker warning: upper %d->%d, device %d->%d, %d out of [%d, %d]\n",
			g_flicker_detector.upper_bl_value_last, upper_bl_value_cur,
			g_flicker_detector.device_bl_value_last, device_bl_value_cur,
			device_bl_weber, weber_threshold_min, weber_threshold_max);
#if defined(CONFIG_HUAWEI_DSM)
		if (!dsm_client_ocuppy(lcd_dclient))
			dsm_client_record(lcd_dclient,
				"flicker warning: upper %d->%d, device %d->%d, %d out of [%d, %d]\n",
				g_flicker_detector.upper_bl_value_last, upper_bl_value_cur,
				g_flicker_detector.device_bl_value_last, device_bl_value_cur,
				device_bl_weber, weber_threshold_min, weber_threshold_max);
		dsm_client_notify(lcd_dclient, DSM_LCD_BACKLIGHT_FLICKER_ERROR_NO);
#endif
		if (g_flicker_detector.config.dump_enable)
			bl_flicker_detector_ring_buffer_dump(jiffies_cur);
	}

	g_flicker_detector.upper_bl_value_last = g_flicker_detector.upper_bl_value_cur;
	g_flicker_detector.device_bl_value_last = device_bl_value_cur;
}

