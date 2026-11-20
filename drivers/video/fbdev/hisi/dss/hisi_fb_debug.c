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
#include "hisi_fb_panel_debug.h"
#if defined(CONFIG_LCDKIT_DRIVER)
#include "lcdkit_fb_util.h"
#include "lcdkit_panel.h"
#endif

/*
 * for debug, S_IRUGO
 * /sys/module/dpufb/parameters
 */
int64_t g_dpufb_mdfx_id = -1;

unsigned int g_dpu_fb_msg_level = 7;

uint32_t g_dsi_pipe_switch_connector;

int g_dss_perf_debug;

uint32_t g_dss_dfr_debug;

int g_debug_mmu_error;

int g_debug_underflow_error = 1;

int g_debug_ldi_underflow;

int g_debug_ldi_underflow_clear = 1;

int g_debug_panel_mode_switch;

int g_debug_set_reg_val;

int g_debug_online_vsync;

int g_debug_online_vactive;

int g_debug_ovl_online_composer;

int g_debug_ovl_online_composer_hold;

int g_debug_ovl_online_composer_return;

uint32_t g_debug_ovl_online_composer_timediff;

uint32_t g_debug_ovl_online_composer_time_threshold = 60000;  /* us */

int g_debug_ovl_offline_composer;

int g_debug_ovl_block_composer;

int g_debug_ovl_offline_composer_hold;

uint32_t g_debug_ovl_offline_composer_timediff;

int g_debug_ovl_offline_composer_time_threshold = 12000;  /* us */

int g_debug_ovl_offline_block_num = -1;

int g_enable_ovl_async_composer = 1;

int g_debug_ovl_mediacommon_composer;

int g_debug_ovl_cmdlist;

int g_dump_cmdlist_content;

int g_enable_ovl_cmdlist_online = 1;

int g_enable_video_idle_l3cache = 1;

int g_debug_ovl_online_wb_count;

int g_smmu_global_bypass;

int g_enable_ovl_cmdlist_offline = 1;

int g_rdma_stretch_threshold = RDMA_STRETCH_THRESHOLD;

int g_enable_dirty_region_updt = 1;

int g_enable_alsc = 1;

int g_debug_dirty_region_updt;

int g_enable_mmbuf_debug;

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V350)
int g_ldi_data_gate_en = 1;
#else
int g_ldi_data_gate_en;
#endif

int g_debug_ovl_credit_step;

int g_debug_layerbuf_sync;

int g_debug_fence_timeline;

int g_enable_dss_idle = 1;

unsigned int g_dss_smmu_outstanding = DSS_SMMU_OUTSTANDING_VAL + 1;

int g_debug_dump_mmbuf;

int g_debug_dump_iova;

uint32_t g_underflow_stop_perf_stat;

uint32_t g_dss_min_bandwidth_inbusbusy = 200; /* 200M */

uint32_t g_mmbuf_addr_test;

uint32_t g_dump_sensorhub_aod_hwlock;

int g_dss_effect_sharpness1D_en = 1;

int g_dss_effect_sharpness2D_en;

int g_dss_effect_acm_ce_en = 1;

int g_debug_online_play_bypass;


/* wait asynchronous vactive0 start threshold 300ms */
uint32_t g_debug_wait_asy_vactive0_thr = 300;

uint32_t g_debug_enable_asynchronous_play;

int g_debug_dpp_cmdlist_dump;

int g_debug_dpp_cmdlist_type = 1;

int g_debug_dpp_cmdlist_debug = 0;

int g_debug_effect_hihdr = 0;

int g_debug_mipi_phy_config = 0;

#if defined(CONFIG_HUAWEI_DSM)

static struct dsm_dev dsm_lcd = {
	.name = "dsm_lcd",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

struct dsm_client *lcd_dclient;

static void dsm_client_notify_underflow(struct dpu_fb_data_type *dpufd, unsigned long curr_ddr)
{
	u32 dpp_dbg_value = 0;

	if (lcd_dclient) {
		if (!dsm_client_ocuppy(lcd_dclient)) {
			if (dpufd->index == PRIMARY_PANEL_IDX) {
				dpufb_activate_vsync(dpufd);
				dpp_dbg_value = inp32(dpufd->dss_base + DSS_DPP_OFFSET + DPP_DBG_CNT);
				dpufb_deactivate_vsync(dpufd);
				dsm_client_record(lcd_dclient,
					"ldi underflow, curr_ddr = %u, frame_no = %d, dpp_dbg = 0x%x!\n",
					curr_ddr, dpufd->ov_req.frame_no, dpp_dbg_value);
			} else {
				dsm_client_record(lcd_dclient, "ldi underflow!\n");
			}
			dsm_client_notify(lcd_dclient, DSM_LCD_LDI_UNDERFLOW_NO);
		}
	}
}

void dss_underflow_debug_func(struct work_struct *work)
{
	struct clk *ddr_clk = NULL;
	unsigned long curr_ddr = 0;
	struct dpu_fb_data_type *dpufd = NULL;
	static u32 underflow_index = 0;
	static ktime_t underflow_timestamp[UNDERFLOW_EXPIRE_COUNT];
	s64 underflow_msecs = 0;
	int i;

	dpu_check_and_no_retval(!work, ERR, "work is NULL\n");

	if (underflow_index < UNDERFLOW_EXPIRE_COUNT) {
		underflow_timestamp[underflow_index] = ktime_get();
		underflow_index++;
	}
	if (underflow_index == UNDERFLOW_EXPIRE_COUNT) {
		underflow_timestamp[UNDERFLOW_EXPIRE_COUNT - 1] = ktime_get();
		underflow_msecs = ktime_to_ms(underflow_timestamp[UNDERFLOW_EXPIRE_COUNT - 1]) -
			ktime_to_ms(underflow_timestamp[0]);
		for (i = 0; i < UNDERFLOW_EXPIRE_COUNT - 1; i++)
			underflow_timestamp[i] = underflow_timestamp[i + 1];
	}

	ddr_clk = clk_get(NULL, "clk_ddrc_freq");
	if (ddr_clk) {
		curr_ddr = clk_get_rate(ddr_clk);
		clk_put(ddr_clk);
	} else {
		DPU_FB_ERR("Get ddr clk failed");
	}

	dpufd = container_of(work, struct dpu_fb_data_type, dss_underflow_debug_work);
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	if (g_underflow_stop_perf_stat)
		dump_dss_overlay(dpufd, &dpufd->ov_req);

	DPU_FB_INFO("Current ddr is %lu\n", curr_ddr);

	if ((underflow_msecs <= UNDERFLOW_INTERVAL) && (underflow_msecs > 0)) {
		DPU_FB_INFO("abnormal, underflow times:%d, interval:%llu, expire interval:%d\n",
			UNDERFLOW_EXPIRE_COUNT, underflow_msecs, UNDERFLOW_INTERVAL);
	} else {
		DPU_FB_INFO("normal, underflow times:%d, interval:%llu, expire interval:%d\n",
			UNDERFLOW_EXPIRE_COUNT, underflow_msecs, UNDERFLOW_INTERVAL);
		return;
	}

	dsm_client_notify_underflow(dpufd, curr_ddr);

	/* report underflow event to mdfx */
	dpufb_debug_create_mdfx_client();
	mdfx_report_default_event(g_dpufb_mdfx_id, DEF_EVENT_UNDER_FLOW);
}
#endif

void hisi_dss_underflow_dump_cmdlist(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req_prev, dss_overlay_t *pov_req_prev_prev)
{
	uint32_t cmdlist_idxs_prev = 0;
	uint32_t cmdlist_idxs_prev_prev = 0;

	if ((g_debug_underflow_error) && (g_underflow_count < DSS_UNDERFLOW_COUNT)) {
		if (pov_req_prev_prev != NULL) {
			(void)hisi_cmdlist_get_cmdlist_idxs(pov_req_prev_prev, &cmdlist_idxs_prev_prev, NULL);
			dump_dss_overlay(dpufd, pov_req_prev_prev);
			hisi_cmdlist_dump_all_node(dpufd, NULL, cmdlist_idxs_prev_prev);
		}

		if (pov_req_prev != NULL) {
			(void)hisi_cmdlist_get_cmdlist_idxs(pov_req_prev, &cmdlist_idxs_prev, NULL);
			dump_dss_overlay(dpufd, pov_req_prev);
			hisi_cmdlist_dump_all_node(dpufd, NULL, cmdlist_idxs_prev);
		}
	}
}

void dpufb_debug_register(struct platform_device *pdev)
{
#if defined(CONFIG_HUAWEI_DSM)
#if defined(CONFIG_LCDKIT_DRIVER)
	struct lcdkit_panel_data *panel;
#endif
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_no_retval(!pdev, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		dev_err(&pdev->dev, "dpufd is NULL");
		return;
	}

	/* dsm lcd */
	if (!lcd_dclient) {
#if defined(CONFIG_LCDKIT_DRIVER)
		if (get_lcdkit_support() && dpufd->index == PRIMARY_PANEL_IDX) {
			panel = lcdkit_get_panel_info();
			if (panel && panel->panel_infos.panel_model)
				dsm_lcd.module_name = panel->panel_infos.panel_model;
			else if (panel && panel->panel_infos.panel_name)
				dsm_lcd.module_name = panel->panel_infos.panel_name;
		}
#endif

		lcd_dclient = dsm_register_client(&dsm_lcd);
	}

	/* dss underflow debug */
	dpufd->dss_underflow_debug_workqueue = create_singlethread_workqueue("dss_underflow_debug");
	if (!dpufd->dss_underflow_debug_workqueue)
		dev_err(&pdev->dev, "fb%d, create dss underflow debug workqueue failed!\n", dpufd->index);
	else
		INIT_WORK(&dpufd->dss_underflow_debug_work, dss_underflow_debug_func);
#endif

	/* create mdx visitor for dss */
	dpufb_debug_create_mdfx_client();

	/* init panel debug */

}

void dpufb_debug_unregister(struct platform_device *pdev)
{
#if defined(CONFIG_HUAWEI_DSM)
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_no_retval(!pdev, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		dev_err(&pdev->dev, "dpufd is NULL");
		return;
	}
#endif

	dpufb_debug_destroy_mdfx_client();
}
