/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 */
/*lint -e559*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "hisi_fb.h"
#include "hisi_overlay_utils.h"
#include "hisi_display_effect.h"
#include "hisi_dpe_utils.h"
#include "hisi_frame_rate_ctrl.h"
#include "hisi_mipi_dsi.h"
#include "product/rgb_stats/hisi_fb_rgb_stats.h"

#if defined (CONFIG_HISI_PERIDVFS)
#include "peri_volt_poll.h"
#endif

static bool check_panel_status_without_VR(struct hisi_fb_data_type *hisifd)
{
	if (hisifd_list[EXTERNAL_PANEL_IDX] && !hisifd_list[EXTERNAL_PANEL_IDX]->dp.dptx_vr) {
		if (hisifd->index != AUXILIARY_PANEL_IDX && !hisifd->panel_power_on) {
			HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
			return false;
		}
	}
	return true;
}

void set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp = 0;

	tmp = inp32(addr);
	tmp &= ~(mask << bs);

	outp32(addr, tmp | ((val & mask) << bs));

	if (g_debug_set_reg_val) {
		HISI_FB_INFO("writel: [%pK] = 0x%x\n", addr, tmp | ((val & mask) << bs));
	}
}

uint32_t set_bits32(uint32_t old_val, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp = 0;

	tmp = old_val;
	tmp &= ~(mask << bs);

	return (tmp | ((val & mask) << bs));
}

void hisifb_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	set_reg(addr, val, bw, bs);
}

bool is_fastboot_display_enable(void)
{
	return ((g_fastboot_enable_flag == 1) ? true : false);
}

bool is_dss_idle_enable(void)
{
	return ((g_enable_dss_idle == 1) ? true : false);
}

uint32_t get_panel_xres(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return 0;
	}

	return ((hisifd->resolution_rect.w > 0) ? hisifd->resolution_rect.w : hisifd->panel_info.xres);
}

uint32_t get_panel_yres(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return 0;
	}

	return ((hisifd->resolution_rect.h > 0) ? hisifd->resolution_rect.h : hisifd->panel_info.yres);
}

uint32_t hisifb_line_length(int index, uint32_t xres, int bpp)
{
	return ALIGN_UP(xres * (uint32_t)bpp, DMA_STRIDE_ALIGN);
}

void hisifb_get_timestamp(struct timeval *tv)
{
	struct timespec ts;

	ktime_get_ts(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;

	//struct timeval timestamp;
	//do_gettimeofday(&timestamp);
	//timestamp = ktime_to_timeval(ktime_get());
}

uint32_t hisifb_timestamp_diff(struct timeval *lasttime, struct timeval *curtime)
{
	uint32_t ret;
	ret = (curtime->tv_usec >= lasttime->tv_usec) ?
		curtime->tv_usec - lasttime->tv_usec:
		1000000 - (lasttime->tv_usec - curtime->tv_usec);

	return ret;

	//return (curtime->tv_sec - lasttime->tv_sec) * 1000 +
	//	(curtime->tv_usec - lasttime->tv_usec) /1000;
}

void hisifb_save_file(char *filename, const char *buf, uint32_t buf_len)
{
	ssize_t write_len = 0;
	struct file *fd = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	if (NULL == filename) {
		HISI_FB_ERR("filename is NULL");
		return;
	}
	if (NULL == buf) {
		HISI_FB_ERR("buf is NULL");
		return;
	}

	fd = filp_open(filename, O_CREAT|O_RDWR, 0644);
	if (IS_ERR(fd)) {
		HISI_FB_ERR("filp_open returned:filename %s, error %ld\n",
			filename, PTR_ERR(fd));
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS); //lint !e501

	write_len = vfs_write(fd, (char __user*)buf, buf_len, &pos);

	pos = 0;
	set_fs(old_fs);
	filp_close(fd, NULL);
}

extern uint32_t g_fastboot_already_set;
int hisifb_ctrl_fastboot(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}


	if (pdata->set_fastboot && !g_fastboot_already_set) {
		ret = pdata->set_fastboot(hisifd->pdev);
	}

	hisifb_vsync_resume(hisifd);

	hisi_overlay_on(hisifd, true);

	if (hisifd->panel_info.esd_enable) {
		hrtimer_restart(&hisifd->esd_ctrl.esd_hrtimer);
	}

	return ret;
}

int hisifb_get_other_fb_votelevel(struct hisi_fb_data_type *hisifd, uint32_t *max_vote_level)
{
	struct hisi_fb_data_type *targetfd1 = NULL;
	struct hisi_fb_data_type *targetfd2 = NULL;
	uint32_t target_dss_voltage_level;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		targetfd1 = hisifd_list[EXTERNAL_PANEL_IDX];
		targetfd2 = hisifd_list[AUXILIARY_PANEL_IDX];
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		targetfd1 = hisifd_list[PRIMARY_PANEL_IDX];
		targetfd2 = hisifd_list[AUXILIARY_PANEL_IDX];
	} else {
		targetfd1 = hisifd_list[PRIMARY_PANEL_IDX];
		targetfd2 = hisifd_list[EXTERNAL_PANEL_IDX];
	}

	if ((targetfd1 == NULL) && (targetfd2 == NULL)) {
		target_dss_voltage_level = PERI_VOLTAGE_LEVEL0;
	} else if (targetfd1 == NULL) {
		target_dss_voltage_level = targetfd2->dss_vote_cmd.dss_voltage_level;
	} else if (targetfd2 == NULL) {
		target_dss_voltage_level = targetfd1->dss_vote_cmd.dss_voltage_level;
	} else {
		target_dss_voltage_level = ((targetfd1->dss_vote_cmd.dss_voltage_level > targetfd2->dss_vote_cmd.dss_voltage_level) ?
			targetfd1->dss_vote_cmd.dss_voltage_level : targetfd2->dss_vote_cmd.dss_voltage_level);
	}

	*max_vote_level = target_dss_voltage_level;

	return 0;
}

int hisifb_ctrl_on(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);

	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}


	if (pdata->on != NULL) {
		ret = pdata->on(hisifd->pdev);
		if (ret < 0) {
			HISI_FB_ERR("regulator/clk on fail.\n");
			return ret;
		}
	}

	hisifb_vsync_resume(hisifd);


	hisi_overlay_on(hisifd, false);

	if (hisifd->panel_info.esd_enable) {
		hrtimer_start(&hisifd->esd_ctrl.esd_hrtimer, ktime_set(ESD_CHECK_TIME_PERIOD / 1000,
			(ESD_CHECK_TIME_PERIOD % 1000) * 1000000), HRTIMER_MODE_REL);
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		hisifb_rgb_reg_enable(hisifd);
		hisifb_rgb_start_wq();
	}

	return ret;
}

int hisifb_ctrl_off(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	if (hisifd->panel_info.esd_enable) {
		hrtimer_cancel(&hisifd->esd_ctrl.esd_hrtimer);
	}

	hisifb_vsync_suspend(hisifd);

	hisi_overlay_off(hisifd);


	if (pdata->off != NULL) {
		ret = pdata->off(hisifd->pdev);
	}

	// FIXME:
	if ((hisifd->index == PRIMARY_PANEL_IDX) ||
		(hisifd->index == EXTERNAL_PANEL_IDX)) {
		down(&hisifd->buf_sync_ctrl.layerbuf_sem);
		//ov base display ok, disable irq, now layerbuf unlock
		hisifb_layerbuf_unlock(hisifd, &(hisifd->buf_sync_ctrl.layerbuf_list));
		up(&hisifd->buf_sync_ctrl.layerbuf_sem);
	}

	if (hisifd->index == PRIMARY_PANEL_IDX)
		hisifb_rgb_cancel_wq();

	return ret;
}

int hisifb_ctrl_lp(struct hisi_fb_data_type *hisifd, bool lp_enter)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	if (lp_enter) {
		hisi_overlay_off_lp(hisifd);

		if (pdata->lp_ctrl != NULL) {
			ret = pdata->lp_ctrl(hisifd->pdev, lp_enter);
		}
	} else {
		if (pdata->lp_ctrl != NULL) {
			ret = pdata->lp_ctrl(hisifd->pdev, lp_enter);
		}

		hisi_overlay_on_lp(hisifd);
	}

	return ret;
}

#define TIME_RANGE_TO_NEXT_VSYNC 3000000
#define DELAY_TIME_AFTER_TE 1000
#define DELAY_TIME_RANGE 500
static void hisi_esd_timing_ctrl(struct hisi_fb_data_type *hisifd)
{
	uint64_t vsync_period;
	ktime_t curr_time;
	uint64_t time_diff;
	uint64_t delay_time = 0;

	if (!hisifd->panel_info.esd_timing_ctrl)
		return;

	if (hisifd->panel_info.fps == 0) {
		HISI_FB_ERR("error fps %d", hisifd->panel_info.fps);
		return;
	}

	vsync_period = 1000000000UL / hisifd->panel_info.fps; // convert to ns from s
	curr_time = ktime_get();

	if (ktime_to_ns(curr_time) > ktime_to_ns(hisifd->vsync_ctrl.vsync_timestamp)) {
		time_diff = ktime_to_ns(curr_time) - ktime_to_ns(hisifd->vsync_ctrl.vsync_timestamp);
		if ((vsync_period > time_diff) &&
			(vsync_period - time_diff < TIME_RANGE_TO_NEXT_VSYNC)) {
			delay_time = (vsync_period - time_diff) / 1000 +
				DELAY_TIME_AFTER_TE; // convert to us from ns

			usleep_range(delay_time, delay_time + DELAY_TIME_RANGE);
			HISI_FB_INFO("vsync %llu ns, timediff %llu ns, delay %llu us",
				vsync_period, time_diff, delay_time);
		}
	}
}

int hisifb_ctrl_esd(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;


	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	down(&hisifd->power_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->esd_handle != NULL) {
		hisifb_vsync_disable_enter_idle(hisifd, true);
		hisifb_activate_vsync(hisifd);

		hisi_esd_timing_ctrl(hisifd);
		ret = pdata->esd_handle(hisifd->pdev);
		hisifb_vsync_disable_enter_idle(hisifd, false);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->power_sem);

	return ret;
}

int hisifb_fps_upt_isr_handler(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_fps_updt_handle != NULL) {
		ret = pdata->lcd_fps_updt_handle(hisifd->pdev);
	}

err_out:
	return ret;
}

/*lint -e644 -e540*/
int dss_get_peri_volt(int *curr_volt)
{
#if defined (CONFIG_HISI_PERIDVFS)
	struct peri_volt_poll *pvp = NULL;

	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	if (!pvp) {
		HISI_FB_ERR("pvp get failed!\n");
		return -EINVAL;
	}
	*curr_volt = peri_get_volt(pvp);
#endif

	return 0;
}

int dss_set_peri_volt(int volt_to_set, int *curr_volt)
{
	int ret = 0;

#if defined (CONFIG_HISI_PERIDVFS)
	struct peri_volt_poll *pvp = NULL;
	bool is_lowtemp = false;

	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	if (!pvp) {
		HISI_FB_ERR("get pvp failed!\n");
		return -EINVAL;
	}

	if (peri_get_temperature(pvp)) {
		is_lowtemp = true;
	}
#ifdef LOW_TEMP_VOLTAGE_LIMIT
	if (!is_vote_needed_for_low_temp(is_lowtemp, volt_to_set)) {
		HISI_FB_DEBUG("is_lowtemp, return\n");
		return -EINVAL;
	}
#endif
	ret = peri_set_volt(pvp, volt_to_set);
	if (ret) {
		HISI_FB_ERR("set votage_value=%d failed, ret=%d", volt_to_set, ret);
		return -EINVAL;
	}
	HISI_FB_DEBUG("set votage_value=%d\n", volt_to_set);

	*curr_volt = peri_get_volt(pvp);
#endif

	return ret;
}

int hisifb_ctrl_dss_voltage_get(struct fb_info *info, void __user *argp)
{
	int voltage_value = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	dss_vote_cmd_t dss_vote_cmd;

	if (NULL == info) {
		HISI_FB_ERR("dss voltage get info NULL Pointer!\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("dss voltage get hisifd NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("dss voltage get argp NULL Pointer!\n");
		return -EINVAL;
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX) {
		HISI_FB_ERR("fb%d, dss voltage get not supported!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (hisifd->core_clk_upt_support == 0) {
			HISI_FB_DEBUG("no support core_clk_upt\n");
			return 0;
		}
	}
	memset(&dss_vote_cmd, 0, sizeof(dss_vote_cmd_t));

	dss_get_peri_volt(&voltage_value);

	dss_vote_cmd.dss_voltage_level = dpe_get_voltage_level(voltage_value);

	if (copy_to_user(argp, &dss_vote_cmd, sizeof(dss_vote_cmd_t))) {
		HISI_FB_ERR("copy to user fail\n");
		return -EFAULT;
	}
	HISI_FB_DEBUG("fb%d, current_peri_voltage_level = %d\n", hisifd->index, dss_vote_cmd.dss_voltage_level);

	return 0;
}

int hisifb_set_dss_vote_voltage(struct hisi_fb_data_type *hisifd, uint32_t dss_voltage_level, int *curr_volt)
{
	int ret;
	int current_peri_voltage = 0;
	int volt_to_set;
	uint32_t volt_level_to_set;
	uint32_t current_dss_voltage_level;

	struct hisi_fb_data_type *fb0 = hisifd_list[PRIMARY_PANEL_IDX];
	struct hisi_fb_data_type *fb1 = hisifd_list[EXTERNAL_PANEL_IDX];
	struct hisi_fb_data_type *fb2 = hisifd_list[AUXILIARY_PANEL_IDX];
	struct hisi_fb_data_type *fb3 = hisifd_list[MEDIACOMMON_PANEL_IDX];

	if ((hisifd == NULL) || (curr_volt == NULL)) {
		HISI_FB_ERR("null ptr");
		return -EINVAL;
	}

	ret = hisifb_get_other_fb_votelevel(hisifd, &current_dss_voltage_level);
	if (ret) {
		HISI_FB_ERR("set max votage_value=%d failed\n", ret);
		return -EINVAL;
	}

	volt_level_to_set = (dss_voltage_level > current_dss_voltage_level) ?
		dss_voltage_level : current_dss_voltage_level;

	volt_to_set = dpe_get_voltage_value(volt_level_to_set);
	if (volt_to_set < 0) {
		HISI_FB_ERR("get votage_value failed\n");
		return -EINVAL;
	}

	if (dss_set_peri_volt(volt_to_set, curr_volt)) {
		HISI_FB_ERR("dss_set_peri_volt %d failed\n", volt_to_set);
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d->level %d, [%d, %d, %d, %d], set %d, current %d\n", hisifd->index, dss_voltage_level,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_voltage_level : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_voltage_level : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_voltage_level : 0,
		(fb3 != NULL) ? fb3->dss_vote_cmd.dss_voltage_level : 0,
		volt_to_set, *curr_volt);

	hisifd->dss_vote_cmd.dss_voltage_level = dss_voltage_level;

	return ret;
}

int hisifb_ctrl_dss_voltage_set(struct fb_info *info, void __user *argp)
{
	int ret = 0;
	dss_vote_cmd_t dss_vote_cmd;
	int current_peri_voltage = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (info == NULL) {
		HISI_FB_ERR("dss voltage set info NULL Pointer!\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (hisifd == NULL) {
		HISI_FB_ERR("dss voltage set hisifd NULL Pointer!\n");
		return -EINVAL;
	}

	if (argp == NULL) {
		HISI_FB_ERR("dss voltage set argp NULL Pointer!\n");
		return -EINVAL;
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX) {
		HISI_FB_ERR("fb%d, dss voltage get not supported!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		HISI_FB_ERR("fb%d, dss voltage get not supported!\n", hisifd->index);
		return -EINVAL;
	}

	if ((hisifd->index == PRIMARY_PANEL_IDX) && (hisifd->core_clk_upt_support == 0)) {
		HISI_FB_DEBUG("no support core_clk_upt\n");
		return ret;
	}

	if (hisifd_list[EXTERNAL_PANEL_IDX] && !hisifd_list[EXTERNAL_PANEL_IDX]->dp.dptx_vr) {
		if ((hisifd->index == AUXILIARY_PANEL_IDX) && (!hisifd_list[PRIMARY_PANEL_IDX]->panel_power_on)) {
			HISI_FB_INFO("fb%d, primary_panel is power off!\n", hisifd->index);
			return -EINVAL;
		}
	}

	down(&g_hisifb_dss_clk_vote_sem);

	ret = copy_from_user(&dss_vote_cmd, argp, sizeof(dss_vote_cmd_t));//lint !e509
	if (ret) {
		HISI_FB_ERR("copy_from_user failed!ret=%d!\n", ret);
		goto volt_vote_out;
	}

	if (dss_vote_cmd.dss_voltage_level == hisifd->dss_vote_cmd.dss_voltage_level) {
		HISI_FB_DEBUG("fb%d same voltage level %d\n", hisifd->index, dss_vote_cmd.dss_voltage_level);
		goto volt_vote_out;
	}

	ret = hisifb_set_dss_vote_voltage(hisifd, dss_vote_cmd.dss_voltage_level, &current_peri_voltage);
	if (ret < 0) {
		goto volt_vote_out;
	}

	dss_vote_cmd.dss_voltage_level = dpe_get_voltage_level(current_peri_voltage);

	if (copy_to_user(argp, &dss_vote_cmd, sizeof(dss_vote_cmd_t))) {
		HISI_FB_ERR("copy to user fail\n");
		ret = -EFAULT;
		goto volt_vote_out;
	}

volt_vote_out:
	up(&g_hisifb_dss_clk_vote_sem);
	return ret;
}
/*lint +e644 +e540*/

int hisifb_ctrl_dss_vote_cmd_set(struct fb_info *info, const void __user *argp)
{
	int ret = 0;
	bool status = true;
	struct hisi_fb_data_type *hisifd = NULL;
	dss_vote_cmd_t vote_cmd;

	if (NULL == info) {
		HISI_FB_ERR("dss clk rate set info NULL Pointer!\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("dss clk rate set hisifd NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("dss clk rate set argp NULL Pointer!\n");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (hisifd->core_clk_upt_support == 0) {
			HISI_FB_DEBUG("no support core_clk_upt\n");
			return ret;
		}
	}

	ret = copy_from_user(&vote_cmd, argp, sizeof(dss_vote_cmd_t));
	if (ret) {
		HISI_FB_ERR("copy_from_user failed!ret=%d.", ret);
		return ret;
	}

	down(&hisifd->blank_sem);

	down(&g_hisifb_dss_clk_vote_sem);

	status = check_panel_status_without_VR(hisifd);
	if (!status) {
		ret = -EPERM;
		goto err_out;
	}

	ret = set_dss_vote_cmd(hisifd, vote_cmd);

err_out:
	up(&g_hisifb_dss_clk_vote_sem);

	up(&hisifd->blank_sem);

	return ret;
}

#pragma GCC diagnostic pop
