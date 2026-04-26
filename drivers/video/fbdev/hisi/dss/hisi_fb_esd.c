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

#if defined(CONFIG_LCD_KIT_DRIVER) && defined(CONFIG_HUAWEI_DSM)
#include "lcd_kit_common.h"
extern struct lcd_kit_esd_error_info g_esd_error_info;
#endif

#define HISI_ESD_RECOVER_MAX_COUNT 10
#define HISI_ESD_CHECK_MAX_COUNT 3
#define HISI_ESD_POWER_OFF_TIME 100

extern unsigned int g_esd_recover_disable;

#if defined ESD_RESTORE_DSS_VOLTAGE_CLK
static void dpufb_esd_get_vote_cmd(struct dpu_fb_data_type *dpufd, dss_vote_cmd_t *vote_cmd)
{
	down(&g_dpufb_dss_clk_vote_sem);
	vote_cmd->dss_voltage_level = dpufd->dss_vote_cmd.dss_voltage_level;
	vote_cmd->dss_pri_clk_rate = dpufd->dss_vote_cmd.dss_pri_clk_rate;
	vote_cmd->dss_mmbuf_rate = dpufd->dss_vote_cmd.dss_mmbuf_rate;
	up(&g_dpufb_dss_clk_vote_sem);
	DPU_FB_DEBUG("fb%d, dss_voltage_level=%d, dss_pri_clk_rate=%llu\n",
		dpufd->index, vote_cmd->dss_voltage_level, vote_cmd->dss_pri_clk_rate);
}
#endif

static void dpufb_esd_recover(struct dpu_fb_data_type *dpufd)
{
	int ret;
	uint32_t bl_level_cur;
	int vsync_enabled;
#ifdef ESD_RESTORE_DSS_VOLTAGE_CLK
	dss_vote_cmd_t vote_cmd = {0};
#endif

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");
	dpu_check_and_no_retval(g_esd_recover_disable, ERR, "g_esd_recover_disable is disable\n");

#if defined(CONFIG_HISI_FB_AOD)
	down(&dpufd->blank_aod_sem);
#endif
	down(&dpufd->brightness_esd_sem);
	if (dpufd->panel_power_on == false) {
		up(&dpufd->brightness_esd_sem);
#if defined(CONFIG_HISI_FB_AOD)
		up(&dpufd->blank_aod_sem);
#endif
		DPU_FB_INFO("panel is off and exit esd recover");
		return;
	}
#if defined(CONFIG_HISI_FB_AOD)
	if (hisi_fb_get_aod_lock_status() == false) {
		DPU_FB_INFO("ap had released aod lock, stop esd recover!\n");
		up(&dpufd->brightness_esd_sem);
		up(&dpufd->blank_aod_sem);
		return;
	}
#endif

	bl_level_cur = dpufd->bl_level;
	dpufb_set_backlight(dpufd, 0, false);
	up(&dpufd->brightness_esd_sem);

#ifdef ESD_RESTORE_DSS_VOLTAGE_CLK
	dpufb_esd_get_vote_cmd(dpufd, &vote_cmd);
#endif

	vsync_enabled = dpufd->vsync_ctrl.vsync_enabled;
	/* lcd panel off */
	ret = hisi_fb_blank_sub(FB_BLANK_POWERDOWN, dpufd->fbi);
	msleep(HISI_ESD_POWER_OFF_TIME);
	if (ret != 0)
		DPU_FB_ERR("fb%d, blank_mode[%d] failed!\n", dpufd->index, FB_BLANK_POWERDOWN);

	/* lcd panel on */
	if (hisi_fb_blank_sub(FB_BLANK_UNBLANK, dpufd->fbi) != 0)
		DPU_FB_ERR("fb%d, blank_mode[%d] failed!\n", dpufd->index, FB_BLANK_UNBLANK);

	dpufd->vsync_ctrl.vsync_enabled = vsync_enabled;

	if (g_debug_online_vsync)
		DPU_FB_INFO("fb%d, vsync_enabled=%d!\n", dpufd->index, dpufd->vsync_ctrl.vsync_enabled);

#ifdef ESD_RESTORE_DSS_VOLTAGE_CLK
	(void)dpufb_restore_dss_voltage_clk_vote(dpufd, vote_cmd);
#endif

	hisi_fb_frame_refresh(dpufd, "esd");
#if defined(CONFIG_HISI_FB_AOD)
	up(&dpufd->blank_aod_sem);
#endif

	/* backlight on */
	msleep(100);  /* sleep 100ms */
	down(&dpufd->brightness_esd_sem);
	dpufb_set_backlight(dpufd, bl_level_cur ? bl_level_cur : dpufd->bl_level, false);
	up(&dpufd->brightness_esd_sem);
}

static void dsm_client_record_esd_err(uint32_t err_no)
{
#if defined(CONFIG_LCD_KIT_DRIVER) && defined(CONFIG_HUAWEI_DSM)
	int i = 0;

	if (lcd_dclient && !dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_record(lcd_dclient, "lcd esd register status error:");
		for (i = 0; i < g_esd_error_info.esd_error_reg_num; i++)
			dsm_client_record(lcd_dclient, "read_reg_val[%d]=0x%x, expect_reg_val[%d]=0x%x",
				g_esd_error_info.esd_reg_index[i],
				g_esd_error_info.esd_error_reg_val[i],
				g_esd_error_info.esd_reg_index[i],
				g_esd_error_info.esd_expect_reg_val[i]);

		dsm_client_record(lcd_dclient, "\n");
		dsm_client_notify(lcd_dclient, err_no);
	}
#endif
}

static void dpufb_esd_check_and_recover(struct dpu_fb_data_type *dpufd, uint32_t *recover_count)
{
	uint32_t esd_check_count = 0;
	int ret = 0;

	while (*recover_count < dpufd->panel_info.esd_recovery_max_count) {
#ifdef CONFIG_LCD_KIT_DRIVER
		if (esd_check_count < dpufd->panel_info.esd_check_max_count) {
#else
		if (esd_check_count < HISI_ESD_CHECK_MAX_COUNT) {
#endif
			if ((dpufd->secure_ctrl.secure_status == DSS_SEC_RUNNING) ||
				((dpufd->secure_ctrl.secure_status == DSS_SEC_IDLE) &&
				(dpufd->secure_ctrl.secure_event == DSS_SEC_ENABLE)))
				break;

			ret = dpufb_ctrl_esd(dpufd);
			if (ret || (dpufd->esd_recover_state == ESD_RECOVER_STATE_START)) {
				esd_check_count++;
				dpufd->esd_happened = 1;
				DPU_FB_INFO("esd check abnormal, esd_check_count:%d!\n", esd_check_count);
			} else {
				dpufd->esd_happened = 0;
				break;
			}
		}

#ifdef CONFIG_LCD_KIT_DRIVER
		if ((esd_check_count >= dpufd->panel_info.esd_check_max_count) ||
			(dpufd->esd_recover_state == ESD_RECOVER_STATE_START)) {
#else
		if ((esd_check_count >= HISI_ESD_CHECK_MAX_COUNT) ||
			(dpufd->esd_recover_state == ESD_RECOVER_STATE_START)) {
#endif
			DPU_FB_ERR("esd recover panel, recover_count:%d!\n", *recover_count);
#ifdef CONFIG_HUAWEI_DSM
			dsm_client_record_esd_err(DSM_LCD_ESD_STATUS_ERROR_NO);
#endif
			down(&dpufd->esd_panel_change_sem);
			dpufb_esd_recover(dpufd);
			up(&dpufd->esd_panel_change_sem);
			dpufd->esd_recover_state = ESD_RECOVER_STATE_COMPLETE;
			esd_check_count = 0;
			(*recover_count)++;
		}
	}
}

static void dpufb_esd_check_wq_handler(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_esd *esd_ctrl = NULL;
	uint32_t recover_count = 0;

	esd_ctrl = container_of(work, struct dpufb_esd, esd_check_work);
	dpu_check_and_no_retval(!esd_ctrl, ERR, "esd_ctrl is NULL\n");

	dpufd = esd_ctrl->dpufd;
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	if (dpufd->panel_info.emi_protect_enable && dpufd->enter_idle) {
		dpufd->emi_protect_check_count++;
		if (dpufd->emi_protect_check_count >= HISI_EMI_PROTECT_CHECK_MAX_COUNT)
			hisi_fb_frame_refresh(dpufd, "emi");
	}

	if (!dpufd->panel_info.esd_enable || g_esd_recover_disable) {
		if (g_esd_recover_disable)
			DPU_FB_INFO("esd_enable=%d, g_esd_recover_disable=%d\n",
				dpufd->panel_info.esd_enable, g_esd_recover_disable);

		return;
	}

	dpufb_esd_check_and_recover(dpufd, &recover_count);


	/* recover count equate 5, we disable esd check function */
	if (recover_count >= dpufd->panel_info.esd_recovery_max_count) {
#ifdef CONFIG_HUAWEI_DSM
		dsm_client_record_esd_err(DSM_LCD_POWER_ABNOMAL_ERROR_NO);
#endif
		hrtimer_cancel(&esd_ctrl->esd_hrtimer);
		dpufd->panel_info.esd_enable = 0;
		DPU_FB_ERR("esd recover %d count, disable esd function\n", dpufd->panel_info.esd_recovery_max_count);
	}
}

static enum hrtimer_restart dpufb_esd_hrtimer_fnc(struct hrtimer *timer)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_esd *esd_ctrl = NULL;

	esd_ctrl = container_of(timer, struct dpufb_esd, esd_hrtimer);
	if (!esd_ctrl) {
		DPU_FB_ERR("esd_ctrl is NULL\n");
		return HRTIMER_NORESTART;
	}
	dpufd = esd_ctrl->dpufd;
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return HRTIMER_NORESTART;
	}

	if (dpufd->panel_info.esd_enable) {
		if (esd_ctrl->esd_check_wq != NULL)
			queue_work(esd_ctrl->esd_check_wq, &(esd_ctrl->esd_check_work));
	}
	if (dpufd->panel_info.esd_check_time_period) {
		hrtimer_start(&esd_ctrl->esd_hrtimer, ktime_set(dpufd->panel_info.esd_check_time_period / 1000,
			(dpufd->panel_info.esd_check_time_period % 1000) * 1000000), HRTIMER_MODE_REL);
	} else {
		hrtimer_start(&esd_ctrl->esd_hrtimer, ktime_set(ESD_CHECK_TIME_PERIOD / 1000,
			(ESD_CHECK_TIME_PERIOD % 1000) * 1000000), HRTIMER_MODE_REL);
	}

	return HRTIMER_NORESTART;
}


void dpufb_esd_register(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_esd *esd_ctrl = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return;
	}

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		dev_err(&pdev->dev, "dpufd is NULL\n");
		return;
	}

	esd_ctrl = &(dpufd->esd_ctrl);
	if (!esd_ctrl) {
		dev_err(&pdev->dev, "esd_ctrl is NULL\n");
		return;
	}

	if (esd_ctrl->esd_inited)
		return;

	if (dpufd->panel_info.esd_enable) {
		dpufd->esd_happened = 0;
		dpufd->esd_recover_state = ESD_RECOVER_STATE_NONE;
		esd_ctrl->dpufd = dpufd;

		esd_ctrl->esd_check_wq = create_singlethread_workqueue("esd_check");
		if (esd_ctrl->esd_check_wq == NULL)
			dev_err(&pdev->dev, "create esd_check_wq failed\n");

		INIT_WORK(&esd_ctrl->esd_check_work, dpufb_esd_check_wq_handler);

		/* hrtimer for ESD timing */
		hrtimer_init(&esd_ctrl->esd_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		esd_ctrl->esd_hrtimer.function = dpufb_esd_hrtimer_fnc;
		if (dpufd->panel_info.esd_check_time_period) {
			hrtimer_start(&esd_ctrl->esd_hrtimer, ktime_set(dpufd->panel_info.esd_check_time_period / 1000,
				(dpufd->panel_info.esd_check_time_period % 1000) * 1000000), HRTIMER_MODE_REL);
		} else {
			hrtimer_start(&esd_ctrl->esd_hrtimer, ktime_set(ESD_CHECK_TIME_PERIOD / 1000,
				(ESD_CHECK_TIME_PERIOD % 1000) * 1000000), HRTIMER_MODE_REL);
		}

		esd_ctrl->esd_inited = 1;
	}
}

void dpufb_esd_unregister(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_esd *esd_ctrl = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return;
	}

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		dev_err(&pdev->dev, "dpufd is NULL\n");
		return;
	}

	esd_ctrl = &(dpufd->esd_ctrl);
	if (!esd_ctrl) {
		dev_err(&pdev->dev, "esd_ctrl is NULL\n");
		return;
	}

	if (!esd_ctrl->esd_inited)
		return;

	if (dpufd->panel_info.esd_enable)
		hrtimer_cancel(&esd_ctrl->esd_hrtimer);

	esd_ctrl->esd_inited = 0;
}

