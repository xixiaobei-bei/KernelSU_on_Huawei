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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"

#include "hisi_fb.h"

#if defined(CONFIG_HISI_FB_AOD) && !defined(CONFIG_HISI_FB_970) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_FINGERPRINT)
#include <huawei_platform/fingerprint_interface/fingerprint_interface.h>
#endif

/* /sys/class/graphics/fb0/vsync_event */
#define VSYNC_CTRL_EXPIRE_COUNT 4
#define MASKLAYER_BACKLIGHT_WAIT_VSYNC_COUNT 2

void dpufb_masklayer_backlight_flag_config(struct dpu_fb_data_type *dpufd,
	bool masklayer_backlight_flag)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (masklayer_backlight_flag == true)
			dpufd->masklayer_maxbacklight_flag = true;
	}
}

static void dpufb_masklayer_backlight_notify(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (dpufd->masklayer_backlight_notify_wq)
			queue_work(dpufd->masklayer_backlight_notify_wq, &dpufd->masklayer_backlight_notify_work);
	}
}

static bool is_primary_panel_at_diff_soc(struct dpu_fb_data_type *dpufd)
{
	return ((dpufd->index == PRIMARY_PANEL_IDX) &&
			(g_dss_version_tag != FB_ACCEL_KIRIN970));
}

void dpufb_masklayer_backlight_notify_handler(struct work_struct *work)
{
	static uint32_t vsync_count;
	struct dpu_fb_data_type *dpufd = NULL;

	if (!work) {
		DPU_FB_ERR("work is NULL.\n");
		return;
	}

	dpufd = container_of(work, struct dpu_fb_data_type, masklayer_backlight_notify_work);
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL.\n");
		return;
	}

	if (is_primary_panel_at_diff_soc(dpufd)) {
		if (g_debug_online_vsync)
			DPU_FB_DEBUG("flag=%d, vsync_count=%d.\n", dpufd->masklayer_maxbacklight_flag, vsync_count);

		if (dpufd->masklayer_maxbacklight_flag == true) {
			vsync_count += 1;
			if (vsync_count == MASKLAYER_BACKLIGHT_WAIT_VSYNC_COUNT) {
				DPU_FB_INFO("mask layer max backlight done notify.\n");
				vsync_count = 0;
				dpufd->masklayer_maxbacklight_flag = false;
#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V320) || defined(CONFIG_HISI_FB_V350) || defined(CONFIG_FINGERPRINT)
				ud_fp_on_hbm_completed();
#endif
			}
		} else {
			vsync_count = 0;
		}
	}
}

void dpufb_frame_updated(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (dpufd->vsync_ctrl.vsync_report_fnc)
		atomic_inc(&(dpufd->vsync_ctrl.buffer_updated));
}

void dpufb_vsync_isr_handler(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_vsync *vsync_ctrl = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	int buffer_updated = 0;
	ktime_t pre_vsync_timestamp;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	vsync_ctrl = &(dpufd->vsync_ctrl);
	pdata = dev_get_platdata(&dpufd->pdev->dev);
	dpu_check_and_no_retval(!pdata, ERR, "pdata is NULL\n");

	pre_vsync_timestamp = vsync_ctrl->vsync_timestamp;
	vsync_ctrl->vsync_timestamp = ktime_get();

	dpufb_masklayer_backlight_notify(dpufd);

	if (dpufd->vsync_ctrl.vsync_enabled)
		wake_up_interruptible_all(&(vsync_ctrl->vsync_wait));

	if (dpufd->panel_info.vsync_ctrl_type != VSYNC_CTRL_NONE) {
		spin_lock(&vsync_ctrl->spin_lock);
		if (vsync_ctrl->vsync_ctrl_expire_count) {
			vsync_ctrl->vsync_ctrl_expire_count--;

			if (vsync_ctrl->vsync_ctrl_expire_count == 0)
				schedule_work(&vsync_ctrl->vsync_ctrl_work);
		}
		spin_unlock(&vsync_ctrl->spin_lock);
	}

	if (vsync_ctrl->vsync_report_fnc) {
		if (dpufd->vsync_ctrl.vsync_enabled)
			buffer_updated = atomic_dec_return(&(vsync_ctrl->buffer_updated));
		else
			buffer_updated = 1;

		if (buffer_updated < 0)
			atomic_cmpxchg(&(vsync_ctrl->buffer_updated), buffer_updated, 1);  /*lint !e571*/
		else if (is_mipi_video_panel(dpufd))
			vsync_ctrl->vsync_report_fnc(0);
		else
			vsync_ctrl->vsync_report_fnc(buffer_updated);
	}

	if (g_debug_online_vsync)
		DPU_FB_INFO("fb%d, VSYNC=%llu, time_diff=%llu.\n", dpufd->index,
			ktime_to_ns(dpufd->vsync_ctrl.vsync_timestamp),
			(ktime_to_ns(dpufd->vsync_ctrl.vsync_timestamp) - ktime_to_ns(pre_vsync_timestamp)));
}

static int vsync_timestamp_changed(struct dpu_fb_data_type *dpufd,
	ktime_t prev_timestamp)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}
	return !(prev_timestamp == dpufd->vsync_ctrl.vsync_timestamp);
}

static ssize_t vsync_show_event(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	int vsync_flag;
	bool report_flag = false;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_secure *secure_ctrl = NULL;
	ktime_t prev_timestamp;

	dpu_check_and_return((!dev), -1, ERR, "dev is NULL.\n");

	fbi = dev_get_drvdata(dev);
	dpu_check_and_return((!fbi), -1, ERR, "fbi is NULL.\n");

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	dpu_check_and_return((!dpufd), -1, ERR, "dpufd is NULL.\n");

	dpu_check_and_return((!buf), -1, ERR, "buf is NULL.\n");

	secure_ctrl = &(dpufd->secure_ctrl);
	prev_timestamp = dpufd->vsync_ctrl.vsync_timestamp;

	/*lint -e666*/
	ret = wait_event_interruptible(dpufd->vsync_ctrl.vsync_wait, /*lint !e578*/
		(vsync_timestamp_changed(dpufd, prev_timestamp) && dpufd->vsync_ctrl.vsync_enabled) ||
		(!!secure_ctrl->tui_need_switch));
	/*lint +e666*/

	vsync_flag = (vsync_timestamp_changed(dpufd, prev_timestamp) && dpufd->vsync_ctrl.vsync_enabled);

	report_flag = !!secure_ctrl->tui_need_switch;

	if (vsync_flag && report_flag) {
		DPU_FB_INFO("report (secure_event = %d) to hwc with vsync at (frame_no = %d).\n",
			secure_ctrl->secure_event, dpufd->ov_req.frame_no);

		ret = snprintf(buf, PAGE_SIZE, "VSYNC=%llu, SecureEvent=%d\n",
			ktime_to_ns(dpufd->vsync_ctrl.vsync_timestamp), secure_ctrl->secure_event);
		buf[strlen(buf) + 1] = '\0';
		if (secure_ctrl->secure_event == DSS_SEC_DISABLE)
			secure_ctrl->tui_need_switch = 0;

	} else if (vsync_flag) {
		ret = snprintf(buf, PAGE_SIZE, "VSYNC=%llu, xxxxxxEvent=x\n",
			ktime_to_ns(dpufd->vsync_ctrl.vsync_timestamp));
		buf[strlen(buf) + 1] = '\0';
		secure_ctrl->tui_need_skip_report = 0;

	} else if (report_flag && !secure_ctrl->tui_need_skip_report) {
		DPU_FB_INFO("report (secure_event = %d) to hwc at (frame_no = %d).\n",
			secure_ctrl->secure_event, dpufd->ov_req.frame_no);

		ret = snprintf(buf, PAGE_SIZE, "xxxxx=%llu, SecureEvent=%d\n",
			ktime_to_ns(dpufd->vsync_ctrl.vsync_timestamp), secure_ctrl->secure_event);
		buf[strlen(buf) + 1] = '\0';
		secure_ctrl->tui_need_skip_report = 1;
		if (secure_ctrl->secure_event == DSS_SEC_DISABLE)
			secure_ctrl->tui_need_switch = 0;

	} else {
		return -1;
	}

	return ret;
}

static ssize_t vsync_timestamp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (!dev) {
		DPU_FB_ERR("NULL Pointer.\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (!fbi) {
		DPU_FB_ERR("NULL Pointer.\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (!dpufd) {
		DPU_FB_ERR("NULL Pointer.\n");
		return -1;
	}

	if (!buf) {
		DPU_FB_ERR("NULL Pointer.\n");
		return -1;
	}

	ret = snprintf(buf, PAGE_SIZE, "%llu\n", ktime_to_ns(dpufd->vsync_ctrl.vsync_timestamp));
	buf[strlen(buf) + 1] = '\0';

	return ret;
}

static DEVICE_ATTR(vsync_event, 0444, vsync_show_event, NULL);
static DEVICE_ATTR(vsync_timestamp, 0444, vsync_timestamp_show, NULL);

static void dpufb_handle_vsync_ctrl(struct dpu_fb_data_type *dpufd,
	struct dpufb_vsync *vsync_ctrl, struct dpu_fb_panel_data *pdata)
{
	unsigned long flags = 0;

	if (vsync_ctrl->vsync_ctrl_disabled_set && !vsync_ctrl->vsync_disable_enter_idle &&
		(vsync_ctrl->vsync_ctrl_expire_count == 0) && vsync_ctrl->vsync_ctrl_enabled &&
		!vsync_ctrl->vsync_enabled && !vsync_ctrl->vsync_ctrl_offline_enabled) {

		DPU_FB_DEBUG("fb%d, dss clk off!\n", dpufd->index);

		spin_lock_irqsave(&(vsync_ctrl->spin_lock), flags);
		if (pdata->vsync_ctrl)
			pdata->vsync_ctrl(dpufd->pdev, 0);
		else
			DPU_FB_ERR("fb%d, vsync_ctrl not supported!\n", dpufd->index);
		vsync_ctrl->vsync_ctrl_enabled = 0;
		vsync_ctrl->vsync_ctrl_disabled_set = 0;
		spin_unlock_irqrestore(&(vsync_ctrl->spin_lock), flags);

		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_MIPI_ULPS)
			mipi_dsi_ulps_cfg(dpufd, 0);

		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_VCC_OFF) {
			if (dpufd->lp_fnc)
				dpufd->lp_fnc(dpufd, true);
		}

		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF) {

			dpe_inner_clk_disable(dpufd);
			dpe_common_clk_disable(dpufd);
			mipi_dsi_clk_disable(dpufd);
		}

		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_VCC_OFF)
			dpe_regulator_disable(dpufd);

		dpufd->enter_idle = true;
	}
}

static void dpufb_vsync_ctrl_workqueue_handler(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_vsync *vsync_ctrl = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	vsync_ctrl = container_of(work, typeof(*vsync_ctrl), vsync_ctrl_work);
	dpu_check_and_no_retval((!vsync_ctrl), ERR, "vsync_ctrl is NULL.\n");

	dpufd = vsync_ctrl->dpufd;
	dpu_check_and_no_retval((!dpufd), ERR, "dpufd is NULL.\n");

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	dpu_check_and_no_retval((!pdata), ERR, "pdata is NULL.\n");

	down(&(dpufd->blank_sem));
	down(&dpufd->blank_sem_effect_hiace);
	down(&dpufd->blank_sem_effect_gmp);

	if (!dpufd->panel_power_on) {
		DPU_FB_INFO("fb%d, panel is power off!\n", dpufd->index);
		goto exit;
	}

	mutex_lock(&(vsync_ctrl->vsync_lock));
	dpufb_handle_vsync_ctrl(dpufd, vsync_ctrl, pdata);
	mutex_unlock(&(vsync_ctrl->vsync_lock));

	if (vsync_ctrl->vsync_report_fnc != NULL) {
		if (is_mipi_video_panel(dpufd))
			vsync_ctrl->vsync_report_fnc(0);
		else
			vsync_ctrl->vsync_report_fnc(1);
	}

exit:
	up(&dpufd->blank_sem_effect_gmp);
	up(&dpufd->blank_sem_effect_hiace);
	up(&(dpufd->blank_sem));
}

void dpufb_vsync_register(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_vsync *vsync_ctrl = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return;
	}
	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		dev_err(&pdev->dev, "dpufd is NULL\n");
		return;
	}
	vsync_ctrl = &(dpufd->vsync_ctrl);
	if (!vsync_ctrl) {
		dev_err(&pdev->dev, "vsync_ctrl is NULL\n");
		return;
	}

	if (vsync_ctrl->vsync_created)
		return;
	mutex_init(&(vsync_ctrl->vsync_lock));
	mutex_lock(&(vsync_ctrl->vsync_lock));
	vsync_ctrl->dpufd = dpufd;
	vsync_ctrl->vsync_infinite = 0;
	vsync_ctrl->vsync_enabled = 0;
	vsync_ctrl->vsync_ctrl_offline_enabled = 0;
	mutex_unlock(&(vsync_ctrl->vsync_lock));
	vsync_ctrl->vsync_timestamp = ktime_get();
	vsync_ctrl->vactive_timestamp = ktime_get();
	init_waitqueue_head(&(vsync_ctrl->vsync_wait));
	spin_lock_init(&(vsync_ctrl->spin_lock));
	INIT_WORK(&vsync_ctrl->vsync_ctrl_work, dpufb_vsync_ctrl_workqueue_handler);

	atomic_set(&(vsync_ctrl->buffer_updated), 1);
#ifdef CONFIG_REPORT_VSYNC
	vsync_ctrl->vsync_report_fnc = mali_kbase_pm_report_vsync;
#else
	vsync_ctrl->vsync_report_fnc = NULL;
#endif

	if (dpufd->sysfs_attrs_append_fnc) {
		dpufd->sysfs_attrs_append_fnc(dpufd, &dev_attr_vsync_event.attr);
		dpufd->sysfs_attrs_append_fnc(dpufd, &dev_attr_vsync_timestamp.attr);
	}

	vsync_ctrl->vsync_created = 1;
}

void dpufb_vsync_unregister(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_vsync *vsync_ctrl = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return;
	}
	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		dev_err(&pdev->dev, "dpufd is NULL\n");
		return;
	}
	vsync_ctrl = &(dpufd->vsync_ctrl);
	if (!vsync_ctrl) {
		dev_err(&pdev->dev, "vsync_ctrl is NULL\n");
		return;
	}

	if (!vsync_ctrl->vsync_created)
		return;

	vsync_ctrl->vsync_created = 0;
}

void dpufb_set_vsync_activate_state(struct dpu_fb_data_type *dpufd, bool infinite)
{
	struct dpufb_vsync *vsync_ctrl = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	vsync_ctrl = &(dpufd->vsync_ctrl);
	if (!vsync_ctrl) {
		DPU_FB_ERR("vsync_ctrl is NULL\n");
		return;
	}

	if (dpufd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE)
		return;

	mutex_lock(&(vsync_ctrl->vsync_lock));

	if (infinite)
		vsync_ctrl->vsync_infinite_count += 1;
	else
		vsync_ctrl->vsync_infinite_count -= 1;

	if (vsync_ctrl->vsync_infinite_count >= 1)
		vsync_ctrl->vsync_infinite = 1;

	if (vsync_ctrl->vsync_infinite_count == 0)
		vsync_ctrl->vsync_infinite = 0;

	mutex_unlock(&(vsync_ctrl->vsync_lock));
}

static bool dpufb_vsync_ctrl_enabled(struct dpu_fb_data_type *dpufd, struct dpufb_vsync *vsync_ctrl)
{
	if (vsync_ctrl->vsync_ctrl_enabled == 0) {
		DPU_FB_DEBUG("fb%d, dss clk on!\n", dpufd->index);

		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_VCC_OFF)
			dpe_regulator_enable(dpufd);

		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF) {
			mipi_dsi_clk_enable(dpufd);
			(void)dpe_common_clk_enable(dpufd);
			(void)dpe_inner_clk_enable(dpufd);
		}

		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_VCC_OFF) {
			if (dpufd->lp_fnc != NULL)
				dpufd->lp_fnc(dpufd, false);
		}

		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_MIPI_ULPS)
			mipi_dsi_ulps_cfg(dpufd, 1);

		vsync_ctrl->vsync_ctrl_enabled = 1;
		return true;
	} else if (vsync_ctrl->vsync_ctrl_isr_enabled) {
		vsync_ctrl->vsync_ctrl_isr_enabled = 0;
		return true;
	} else {
		return false;
	}
}

void dpufb_activate_vsync(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpufb_vsync *vsync_ctrl = NULL;
	unsigned long flags = 0;
	bool clk_enabled = false;

	dpu_check_and_no_retval((!dpufd), ERR, "dpufd is NULL.\n");

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	dpu_check_and_no_retval((!pdata), ERR, "pdata is NULL.\n");

	vsync_ctrl = &(dpufd->vsync_ctrl);
	dpu_check_and_no_retval((!vsync_ctrl), ERR, "vsync_ctrl is NULL.\n");

	if (dpufd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE)
		return;

	mutex_lock(&(vsync_ctrl->vsync_lock));
	clk_enabled = dpufb_vsync_ctrl_enabled(dpufd, vsync_ctrl);

	spin_lock_irqsave(&(vsync_ctrl->spin_lock), flags);
	vsync_ctrl->vsync_ctrl_disabled_set = 0;
	vsync_ctrl->vsync_ctrl_expire_count = 0;

	if (clk_enabled) {
		if (pdata->vsync_ctrl)
			pdata->vsync_ctrl(dpufd->pdev, 1);
		else
			DPU_FB_ERR("fb%d, vsync_ctrl not supported!\n", dpufd->index);
	}
	spin_unlock_irqrestore(&(vsync_ctrl->spin_lock), flags);

	mutex_unlock(&(vsync_ctrl->vsync_lock));
}

void dpufb_deactivate_vsync(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpufb_vsync *vsync_ctrl = NULL;
	unsigned long flags = 0;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (!pdata) {
		DPU_FB_ERR("pdata is NULL\n");
		return;
	}
	vsync_ctrl = &(dpufd->vsync_ctrl);
	if (!vsync_ctrl) {
		DPU_FB_ERR("vsync_ctrl is NULL\n");
		return;
	}

	if (dpufd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE)
		return;

	if (dpufd->secure_ctrl.secure_event == DSS_SEC_ENABLE)
		return;

	mutex_lock(&(vsync_ctrl->vsync_lock));

	spin_lock_irqsave(&(vsync_ctrl->spin_lock), flags);
	if (vsync_ctrl->vsync_infinite == 0)
		vsync_ctrl->vsync_ctrl_disabled_set = 1;

	if (vsync_ctrl->vsync_ctrl_enabled)
		vsync_ctrl->vsync_ctrl_expire_count = VSYNC_CTRL_EXPIRE_COUNT;
	spin_unlock_irqrestore(&(vsync_ctrl->spin_lock), flags);

	mutex_unlock(&(vsync_ctrl->vsync_lock));
}

int dpufb_vsync_ctrl(struct fb_info *info, const void __user *argp)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpufb_vsync *vsync_ctrl = NULL;
	int enable = 0;

	dpu_check_and_return((!info), -EINVAL, ERR, "vsync ctrl info NULL Pointer!\n");

	dpufd = (struct dpu_fb_data_type *)info->par;
	dpu_check_and_return((!dpufd), -EINVAL, ERR, "vsync ctrl dpufd NULL Pointer!\n");

	dpu_check_and_return(((dpufd->index != PRIMARY_PANEL_IDX) && (dpufd->index != EXTERNAL_PANEL_IDX)),
		-EINVAL, ERR, "fb%d, vsync ctrl not supported!\n", dpufd->index);

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	dpu_check_and_return((!pdata), -EINVAL, ERR, "pdata NULL Pointer!\n");

	vsync_ctrl = &(dpufd->vsync_ctrl);
	dpu_check_and_return((!vsync_ctrl), -EINVAL, ERR, "vsync_ctrl NULL Pointer!\n");

	dpu_check_and_return((!argp), -EINVAL, ERR, "argp NULL Pointer!\n");

	ret = copy_from_user(&enable, argp, sizeof(enable));
	dpu_check_and_return(ret, ret, ERR, "vsync ctrl ioctl failed!\n");

	enable = (enable) ? 1 : 0;

	mutex_lock(&(vsync_ctrl->vsync_lock));

	if (vsync_ctrl->vsync_enabled == enable) {
		mutex_unlock(&(vsync_ctrl->vsync_lock));
		return 0;
	}

	if (g_debug_online_vsync)
		DPU_FB_INFO("fb%d, enable=%d!\n", dpufd->index, enable);

	vsync_ctrl->vsync_enabled = enable;

	mutex_unlock(&(vsync_ctrl->vsync_lock));

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_INFO("fb%d, panel is power off!", dpufd->index);
		up(&dpufd->blank_sem);
		return 0;
	}

	if (enable)
		dpufb_activate_vsync(dpufd);
	else
		dpufb_deactivate_vsync(dpufd);

	up(&dpufd->blank_sem);

	return 0;
}

void dpufb_vsync_disable_enter_idle(struct dpu_fb_data_type *dpufd, bool disable)
{
	struct dpufb_vsync *vsync_ctrl = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	vsync_ctrl = &(dpufd->vsync_ctrl);
	if (!vsync_ctrl) {
		DPU_FB_ERR("vsync_ctrl is NULL\n");
		return;
	}

	if (dpufd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE)
		return;

	mutex_lock(&(vsync_ctrl->vsync_lock));
	if (disable)
		vsync_ctrl->vsync_disable_enter_idle = 1;
	else
		vsync_ctrl->vsync_disable_enter_idle = 0;
	mutex_unlock(&(vsync_ctrl->vsync_lock));
}

int dpufb_vsync_resume(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_vsync *vsync_ctrl = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}
	vsync_ctrl = &(dpufd->vsync_ctrl);
	if (!vsync_ctrl) {
		DPU_FB_ERR("vsync_ctrl is NULL\n");
		return -EINVAL;
	}

	mutex_lock(&(vsync_ctrl->vsync_lock));
	vsync_ctrl->vsync_ctrl_expire_count = 0;
	vsync_ctrl->vsync_ctrl_disabled_set = 0;
	vsync_ctrl->vsync_ctrl_enabled = 1;
	vsync_ctrl->vsync_ctrl_isr_enabled = 1;
	mutex_unlock(&(vsync_ctrl->vsync_lock));

	atomic_set(&(vsync_ctrl->buffer_updated), 1);

	return 0;
}

int dpufb_vsync_suspend(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	dpufd->vsync_ctrl.vsync_enabled = 0;

	return 0;
}
#pragma GCC diagnostic pop

