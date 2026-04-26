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

#include "overlay/hisi_overlay_utils.h"
#include "hisi_dpe_utils.h"
#include "hisi_mmbuf_manager.h"
#include "hisi_mipi_dsi.h"
#include "product/rgb_stats/hisi_fb_rgb_stats.h"

#if defined(CONFIG_HISI_FB_AOD)
extern bool hisi_aod_get_aod_status(void);
#endif

static int hisi_check_cmdlist_enabled(
	struct dpu_fb_data_type *dpufd)
{
	int enable_cmdlist;

	enable_cmdlist = g_enable_ovl_cmdlist_online;
	if ((dpufd->index == EXTERNAL_PANEL_IDX) && dpufd->panel_info.fake_external)
		enable_cmdlist = 0;

	return enable_cmdlist;
}

static int hisi_check_cmdlist_prepare(
	struct dpu_fb_data_type *dpufd,
	uint32_t *out_cmdlist_pre_idxs,
	uint32_t *out_cmdlist_idxs,
	int enable_cmdlist)
{
	int ret;
	uint32_t cmdlist_pre_idxs = 0;
	uint32_t cmdlist_idxs = 0;
	dss_overlay_t *pov_req = NULL;
	dss_overlay_t *pov_req_prev = NULL;

	pov_req = &(dpufd->ov_req);
	pov_req_prev = &(dpufd->ov_req_prev);
	(void)hisi_dss_handle_cur_ovl_req(dpufd, pov_req);

	(void)hisi_dss_module_init(dpufd);

	hisi_mmbuf_info_get_online(dpufd);
	dpufd->use_mmbuf_cnt = 0;
	dpufd->use_axi_cnt = 0;

	if (enable_cmdlist) {
		dpufd->set_reg = hisi_cmdlist_set_reg;

		hisi_cmdlist_data_get_online(dpufd);

		ret = hisi_cmdlist_get_cmdlist_idxs(pov_req_prev, &cmdlist_pre_idxs, NULL);
		if (ret != 0) {
			DPU_FB_ERR("fb%d, get_cmdlist_idxs pov_req_prev failed! ret=%d\n", dpufd->index, ret);
			return -1;
		}
		ret = hisi_cmdlist_get_cmdlist_idxs(pov_req, &cmdlist_pre_idxs, &cmdlist_idxs);
		if (ret != 0) {
			DPU_FB_ERR("fb%d, get_cmdlist_idxs pov_req failed! ret=%d\n", dpufd->index, ret);
			return -1;
		}
		hisi_cmdlist_add_nop_node(dpufd, cmdlist_pre_idxs, 0, 0);
		hisi_cmdlist_add_nop_node(dpufd, cmdlist_idxs, 0, 0);
	} else {
		dpufd->set_reg = dpufb_set_reg;
		hisi_dss_mctl_mutex_lock(dpufd, pov_req->ovl_idx);
		cmdlist_pre_idxs = ~0;
	}

	hisi_dss_prev_module_set_regs(dpufd, pov_req_prev, cmdlist_pre_idxs, enable_cmdlist, NULL);
	*out_cmdlist_pre_idxs = cmdlist_pre_idxs;
	*out_cmdlist_idxs = cmdlist_idxs;

	return 0;
}

static void hisi_ov_pan_display_init(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	struct fb_info *fbi,
	int hal_format,
	uint32_t offset)
{
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	dss_layer_t *layer = NULL;

	pov_req->ov_block_infos_ptr = (uint64_t)(uintptr_t)(dpufd->ov_block_infos);
	pov_req->ov_block_nums = 1;
	pov_req->ovl_idx = DSS_OVL0;
	pov_req->dirty_rect.x = 0;
	pov_req->dirty_rect.y = 0;
	pov_req->dirty_rect.w = fbi->var.xres + dpufd->panel_info.dummy_pixel_num;
	pov_req->dirty_rect.h = fbi->var.yres;

	pov_req->res_updt_rect.x = 0;
	pov_req->res_updt_rect.y = 0;
	pov_req->res_updt_rect.w = fbi->var.xres + dpufd->panel_info.dummy_pixel_num;
	pov_req->res_updt_rect.h = fbi->var.yres;
	pov_req->release_fence = -1;

	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
	pov_h_block = &(pov_h_block_infos[0]);
	pov_h_block->layer_nums = 1;

	layer = &(pov_h_block->layer_infos[0]);
	layer->img.format = hal_format;
	layer->img.width = fbi->var.xres;
	layer->img.height = fbi->var.yres;
	layer->img.bpp = fbi->var.bits_per_pixel >> 3;
	layer->img.stride = fbi->fix.line_length;
	layer->img.buf_size = layer->img.stride * layer->img.height;
	layer->img.phy_addr = fbi->fix.smem_start + offset;
	layer->img.vir_addr = fbi->fix.smem_start + offset;
	layer->img.mmu_enable = 1;
	layer->img.shared_fd = -1;
	layer->src_rect.x = 0;
	layer->src_rect.y = 0;
	layer->src_rect.w = fbi->var.xres;
	layer->src_rect.h = fbi->var.yres;
	layer->dst_rect.x = 0;
	layer->dst_rect.y = 0;
	layer->dst_rect.w = fbi->var.xres;
	layer->dst_rect.h = fbi->var.yres;
	layer->transform = HISI_FB_TRANSFORM_NOP;
	layer->blending = HISI_FB_BLENDING_NONE;
	layer->glb_alpha = 0xFF;
	layer->color = 0x0;
	layer->layer_idx = 0x0;
	layer->chn_idx = DSS_RCHN_D2;
	layer->need_cap = 0;
	layer->acquire_fence = -1;
}

static int hisi_ov_pan_display_prepare(
	struct dpu_fb_data_type *dpufd,
	struct fb_info *fbi)
{
	int ret;
	dss_overlay_t *pov_req = NULL;
	uint32_t offset;
	int hal_format;

	dpufb_activate_vsync(dpufd);
	offset = fbi->var.xoffset * (fbi->var.bits_per_pixel >> 3) + fbi->var.yoffset * fbi->fix.line_length;
	if (!fbi->fix.smem_start) {
		DPU_FB_ERR("fb%d, smem_start is null!\n", dpufd->index);
		return -EINVAL;
	}
	dpufd->fb_pan_display = true;

	if (fbi->fix.smem_len <= 0) {
		DPU_FB_ERR("fb%d, smem_len = %d is out of range!\n", dpufd->index, fbi->fix.smem_len);
		return -EINVAL;
	}

	hal_format = hisi_get_hal_format(fbi);
	if (hal_format < 0) {
		DPU_FB_ERR("fb%d, not support this fb_info's format!\n", dpufd->index);
		return -EINVAL;
	}

	pov_req = &(dpufd->ov_req);
	ret = hisi_vactive0_start_config(dpufd, pov_req);
	if (ret != 0) {
		DPU_FB_ERR("fb%d, hisi_vactive0_start_config failed! ret=%d\n", dpufd->index, ret);
		return -EINVAL;
	}
	memset(pov_req, 0, sizeof(*pov_req));

	hisi_ov_pan_display_init(dpufd, pov_req, fbi, hal_format, offset);

	return 0;
}

static int hisi_dss_check_vsync_and_mipi_resource(struct dpu_fb_data_type *dpufd,
		dss_overlay_t *pov_req, bool *vsync_time_checked)
{
	int ret = 0;

	if (is_mipi_video_panel(dpufd) || is_dp_panel(dpufd)) {
		*vsync_time_checked = true;
		ret = dpufb_wait_for_mipi_resource_available(dpufd, pov_req);
		if (ret < 0)
			return ret;
		ret = hisi_dss_check_vsync_timediff(dpufd, pov_req);
		if (ret < 0)
			return ret;
	}

	return ret;
}

static bool hisi_online_play_need_continue(
	struct dpu_fb_data_type *dpufd,
	void __user *argp)
{
#if defined(CONFIG_HISI_FB_AOD)
	if ((dpufd->index == EXTERNAL_PANEL_IDX) && hisi_aod_get_aod_status()) {
		DPU_FB_INFO("In aod mode!\n");
		return false;
	}
#endif


	if (!dpufd->panel_power_on) {
		DPU_FB_INFO("fb%d, panel is power off!\n", dpufd->index);
		dpufd->backlight.bl_updated = 0;
		return false;
	}

	if (g_debug_ldi_underflow) {
		if (g_err_status & (DSS_PDP_LDI_UNDERFLOW | DSS_SDP_LDI_UNDERFLOW)) {
			mdelay(HISI_DSS_COMPOSER_HOLD_TIME);
			return false;
		}
	}

	if (g_debug_ovl_online_composer_return)
		return false;

	if (hisi_online_play_bypass_check(dpufd)) {
		/* resync fence with hwcomposer */
		dpufb_buf_sync_suspend(dpufd);
		DPU_FB_INFO("fb%d online play is bypassed!\n", dpufd->index);
		return false;
	}

	return true;
}

static int hisi_online_play_prepare(
	struct dpu_fb_data_type *dpufd,
	const void __user *argp,
	int dss_free_buffer_refcount)
{
	int ret;
	int need_wait_1vsync = 0;
	dss_overlay_t *pov_req = NULL;
	uint32_t timediff;
	struct timeval tv0;
	struct timeval tv1;

	pov_req = &(dpufd->ov_req);
	dpufb_get_timestamp(&tv0);
	dpufb_snd_cmd_before_frame(dpufd);

	ret = hisi_dss_get_ov_data_from_user(dpufd, pov_req, argp);
	if (ret != 0) {
		DPU_FB_ERR("fb%d, hisi_dss_get_ov_data_from_user failed! ret=%d\n", dpufd->index, ret);
		return ret;
	}

	if ((is_mipi_video_panel(dpufd)) && (dpufd->online_play_count == 1)) {
		need_wait_1vsync = 1;
		DPU_FB_INFO("video panel wait a vsync when first frame displayed!\n");
	}


	if (is_mipi_cmd_panel(dpufd) || (dss_free_buffer_refcount < 2) || need_wait_1vsync) {
		ret = hisi_vactive0_start_config(dpufd, pov_req);
		if (ret != 0) {
			DPU_FB_ERR("fb%d, hisi_vactive0_start_config failed! ret=%d\n", dpufd->index, ret);
			dpufd->vactive0_start_flag = 1;
			return ret;
		}
	}
	dpufb_get_timestamp(&tv1);
	timediff = dpufb_timestamp_diff(&tv0, &tv1);
	if (timediff >= g_debug_ovl_online_composer_time_threshold)
		DPU_FB_INFO("online_vactive_wait_timediff is %u us!\n", timediff);

	if (g_debug_ovl_online_composer)
		dump_dss_overlay(dpufd, pov_req);

	return ret;
}

static void hisi_online_play_cmdlist_flush(
	struct dpu_fb_data_type *dpufd,
	uint32_t cmdlist_pre_idxs,
	uint32_t *cmdlist_idxs,
	int enable_cmdlist)
{
	if (enable_cmdlist) {
		g_online_cmdlist_idxs |= *cmdlist_idxs;

		/* add taskend for share channel */
		hisi_cmdlist_add_nop_node(dpufd, *cmdlist_idxs, 0, 0);

		/* remove ch cmdlist */
		hisi_cmdlist_config_stop(dpufd, cmdlist_pre_idxs);

		*cmdlist_idxs |= cmdlist_pre_idxs;
		hisi_cmdlist_flush_cache(dpufd, *cmdlist_idxs);
	}
}

static int hisi_online_play_cmdlist_start(
	struct dpu_fb_data_type *dpufd,
	uint32_t cmdlist_idxs,
	int enable_cmdlist)
{
	dss_overlay_t *pov_req = &(dpufd->ov_req);

	if (enable_cmdlist) {
		if (g_fastboot_enable_flag)
			set_reg(dpufd->dss_base + DSS_MCTRL_CTL0_OFFSET + MCTL_CTL_TOP, 0x1, 32, 0);

		if (hisi_cmdlist_config_start(dpufd, pov_req->ovl_idx, cmdlist_idxs, 0)) {
			dpufb_buf_sync_close_fence(&pov_req->release_fence, &pov_req->retire_fence);
			return -EFAULT;
		}
	} else {
		hisi_dss_mctl_mutex_unlock(dpufd, pov_req->ovl_idx);
	}

	return 0;
}

static int hisi_online_play_h_block_handle(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_block_t *pov_h_block,
	int ov_block_idx,
	bool *vsync_time_checked,
	int enable_cmdlist)
{
	int ret;
	uint32_t i;
	dss_layer_t *layer = NULL;
	dss_rect_ltrb_t clip_rect = {0};
	dss_rect_t aligned_rect = {0};
	dss_overlay_t *pov_req = NULL;
	struct hisi_ov_compose_rect ov_compose_rect = { NULL, NULL, &clip_rect, &aligned_rect };
	struct hisi_ov_compose_flag ov_compose_flag = { false, false, true, enable_cmdlist };

	pov_req = &(dpufd->ov_req);
	hisi_dss_aif_handler(dpufd, pov_req, pov_h_block);

	ret = hisi_dss_ovl_base_config(dpufd, pov_req, pov_h_block, NULL, ov_block_idx);
	if (ret != 0) {
		DPU_FB_ERR("fb%d, hisi_dss_ovl_init failed! ret=%d\n", dpufd->index, ret);
		return ret;
	}

	if (ov_block_idx == 0) {
		ret = hisi_dss_check_vsync_and_mipi_resource(dpufd, pov_req, vsync_time_checked);
		if (ret < 0) {
			DPU_FB_ERR("fb%d, check vsync failed! ret=%d\n", dpufd->index, ret);
			return ret;
		}
	}

	/* Go through all layers */
	for (i = 0; i < pov_h_block->layer_nums; i++) {
		layer = &(pov_h_block->layer_infos[i]);
		memset(ov_compose_rect.clip_rect, 0, sizeof(dss_rect_ltrb_t));
		memset(ov_compose_rect.aligned_rect, 0, sizeof(dss_rect_t));
		ov_compose_flag.rdma_stretch_enable = false;

		ret = hisi_ov_compose_handler(dpufd, pov_h_block, layer, &ov_compose_rect, &ov_compose_flag);
		if (ret != 0) {
			DPU_FB_ERR("fb%d, hisi_ov_compose_handler failed! ret=%d\n", dpufd->index, ret);
			return ret;
		}
	}

	ret = hisi_dss_mctl_ov_config(dpufd, pov_req, pov_req->ovl_idx, ov_compose_flag.has_base, (ov_block_idx == 0));
	if (ret != 0) {
		DPU_FB_ERR("fb%d, hisi_dss_mctl_config failed! ret=%d\n", dpufd->index, ret);
		return ret;
	}

	return ret;
}

static int hisi_online_play_blocks_handle(
	struct dpu_fb_data_type *dpufd,
	bool *vsync_time_checked,
	int enable_cmdlist)
{
	int ret = 0;
	uint32_t m;
	dss_overlay_t *pov_req = NULL;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	struct ov_module_set_regs_flag ov_module_flag = { enable_cmdlist, false, 0, 0 };

	pov_req = &(dpufd->ov_req);
	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
	for (m = 0; m < pov_req->ov_block_nums; m++) {
		pov_h_block = &(pov_h_block_infos[m]);
		(void)hisi_dss_module_init(dpufd);

		ret = hisi_online_play_h_block_handle(dpufd, pov_h_block, m, vsync_time_checked, enable_cmdlist);
		if (ret != 0) {
			DPU_FB_ERR("fb%d, play_h_block_handle failed! ret=%d\n", dpufd->index, ret);
			return ret;
		}

		if (m == 0) {
			if (dpufd->panel_info.dirty_region_updt_support)
				(void)hisi_dss_dirty_region_dbuf_config(dpufd, pov_req);


			ret = hisi_dss_post_scf_config(dpufd, pov_req);
			if (ret != 0) {
				DPU_FB_ERR("fb%d, post_scf_config failed! ret=%d\n", dpufd->index, ret);
				return ret;
			}

			ret = hisi_effect_arsr1p_config(dpufd, pov_req);
			if (ret != 0) {
				DPU_FB_ERR("fb%d, hisi_effect_arsr1p_config failed! ret=%d\n", dpufd->index, ret);
				return ret;
			}
#if defined(CONFIG_EFFECT_HIACE)
			hisi_effect_hiace_config(dpufd);
#endif
		}
		ov_module_flag.is_first_ov_block = (m == 0) ? true : false;
		ret = hisi_dss_ov_module_set_regs(dpufd, pov_req, pov_req->ovl_idx, ov_module_flag);
		if (ret != 0) {
			DPU_FB_ERR("fb%d, hisi_dss_module_config failed! ret=%d\n", dpufd->index, ret);
			return ret;
		}
	}

	return ret;
}

static int hisi_online_play_config_start(
	struct dpu_fb_data_type *dpufd,
	struct dss_cmdlist_idxs *idxs_tmp,
	bool vsync_time_checked,
	bool *masklayer_maxbacklight_flag)
{
	int ret;
	unsigned long flags = 0;
	dss_overlay_t *pov_req = NULL;
	dss_overlay_t *pov_req_prev = NULL;

	pov_req = &(dpufd->ov_req);
	pov_req_prev = &(dpufd->ov_req_prev);
	hisi_sec_mctl_set_regs(dpufd);

	hisi_online_play_cmdlist_flush(dpufd,
		*(idxs_tmp->cmdlist_pre_idxs), idxs_tmp->cmdlist_idxs, *(idxs_tmp->enable_cmdlist));
	if (!g_fake_lcd_flag)
		hisi_dss_unflow_handler(dpufd, pov_req, true);

	if (vsync_time_checked == false) {
		ret = hisi_dss_check_vsync_timediff(dpufd, pov_req);
		if (ret < 0)
			return ret;
	}
	dpufb_buf_sync_create_fence(dpufd, &pov_req->release_fence, &pov_req->retire_fence);

	spin_lock_irqsave(&dpufd->buf_sync_ctrl.refresh_lock, flags);
	dpufd->buf_sync_ctrl.refresh++;
	spin_unlock_irqrestore(&dpufd->buf_sync_ctrl.refresh_lock, flags);

	if (is_mipi_cmd_panel(dpufd))
		(void)dpufb_wait_for_mipi_resource_available(dpufd, pov_req);

	dpufb_mask_layer_backlight_config(dpufd, pov_req_prev, pov_req, masklayer_maxbacklight_flag);

	ret = hisi_online_play_cmdlist_start(dpufd, *(idxs_tmp->cmdlist_idxs), *(idxs_tmp->enable_cmdlist));
	if (ret != 0)
		return ret;

	if (dpufd->panel_info.dirty_region_updt_support)
		hisi_dss_dirty_region_updt_config(dpufd, pov_req);

	/* cpu config drm layer */
	hisi_drm_layer_online_config(dpufd, pov_req_prev, pov_req);

	dpufb_dc_backlight_config(dpufd);


	hisi_vactive0_end_isr_handler(dpufd);
	single_frame_update(dpufd);
	dpufb_frame_updated(dpufd);

	return 0;
}

/* wakeup DP when android system has been startup */
static void hisi_dp_wake_up(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_data_type *dpufd1 = dpufd_list[EXTERNAL_PANEL_IDX];
	dpu_check_and_no_retval(!dpufd1, ERR, "dpufd1 is NULL!\n");

	if (!(dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) ||
		((dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) &&
		(dpufd->panel_power_status != EN_INNER_OUTER_PANEL_ON))) {
		dpufd1->dp.dptx_gate = true;
		if (dpufd1->dp_wakeup)
			dpufd1->dp_wakeup(dpufd1);
	}
}

static void hisi_online_play_post_handle(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	int dss_free_buffer_refcount)
{
	/* pass to hwcomposer handle, driver doesn't use it no longer */
	pov_req->release_fence = -1;
	pov_req->retire_fence = -1;
	dpufd->enter_idle = false;
	dpufd->emi_protect_check_count = 0;

	if ((dpufd->index == PRIMARY_PANEL_IDX) && (dss_free_buffer_refcount > 1)) {
		if (!dpufd->fb_mem_free_flag) {
			dpufb_free_fb_buffer(dpufd);
			dpufd->fb_mem_free_flag = true;
		}

		if (g_logo_buffer_base && g_logo_buffer_size) {
			dpufb_free_logo_buffer(dpufd);
			DPU_FB_INFO("dss_free_buffer_refcount=%d!\n", dss_free_buffer_refcount);
		}

		hisi_dp_wake_up(dpufd);
	}
	dpufd->frame_count++;

	memcpy(&dpufd->ov_req_prev_prev, &dpufd->ov_req_prev, sizeof(dss_overlay_t));
	memcpy(dpufd->ov_block_infos_prev_prev, dpufd->ov_block_infos_prev,
		dpufd->ov_req_prev.ov_block_nums * sizeof(dss_overlay_block_t));
	dpufd->ov_req_prev_prev.ov_block_infos_ptr = (uint64_t)(uintptr_t)(dpufd->ov_block_infos_prev_prev);

	memcpy(&dpufd->ov_req_prev, pov_req, sizeof(dss_overlay_t));
	memcpy(dpufd->ov_block_infos_prev, dpufd->ov_block_infos,
		pov_req->ov_block_nums * sizeof(dss_overlay_block_t));
	dpufd->ov_req_prev.ov_block_infos_ptr = (uint64_t)(uintptr_t)(dpufd->ov_block_infos_prev);
	dpufd->vsync_ctrl.vsync_timestamp_prev = dpufd->vsync_ctrl.vsync_timestamp;
}

static int hisi_ov_pan_display_config_handle(
	struct dpu_fb_data_type *dpufd)
{
	int ret;
	bool flag = false;
	int enable_cmdlist;
	uint32_t cmdlist_pre_idxs = 0;
	uint32_t cmdlist_idxs = 0;
	struct dss_cmdlist_idxs idxs_tmp = { &cmdlist_pre_idxs, &cmdlist_idxs, &enable_cmdlist };

	enable_cmdlist = hisi_check_cmdlist_enabled(dpufd);
	ret = hisi_check_cmdlist_prepare(dpufd, &cmdlist_pre_idxs, &cmdlist_idxs, enable_cmdlist);
	if (ret) {
		DPU_FB_ERR("pan display prepare failed!\n");
		return -1;
	}

	ret = hisi_online_play_blocks_handle(dpufd, &flag, enable_cmdlist);
	if (ret) {
		DPU_FB_ERR("pan display block config failed!\n");
		return -1;
	}

	ret = hisi_online_play_config_start(dpufd, &idxs_tmp, false, &flag);
	if (ret) {
		DPU_FB_ERR("pan display start config failed!\n");
		return -1;
	}

	/* fence for pan display is useless, need close */
	dpufb_buf_sync_close_fence(&dpufd->ov_req.release_fence, &dpufd->ov_req.retire_fence);

	return 0;
}

int hisi_overlay_pan_display(
	struct dpu_fb_data_type *dpufd)
{
	int ret;
	struct fb_info *fbi = NULL;
	dss_overlay_t *pov_req = NULL;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	fbi = dpufd->fbi;
	dpu_check_and_return(!fbi, -EINVAL, ERR, "fbi is nullptr!\n");
	dpu_check_and_return(!dpufd->panel_power_on, 0, INFO, "fb%d, panel is power off!\n", dpufd->index);

	ret = hisi_ov_pan_display_prepare(dpufd, fbi);
	if (ret != 0) {
		DPU_FB_INFO("fb%d, pan display prepare failed!\n", dpufd->index);
		goto err_return;
	}

	ret = hisi_ov_pan_display_config_handle(dpufd);
	if (ret != 0) {
		DPU_FB_INFO("fb%d, pan display config handle failed!\n", dpufd->index);
		goto err_return;
	}

	dpufd->frame_count++;
	memcpy(&dpufd->ov_req_prev_prev, &dpufd->ov_req_prev, sizeof(dss_overlay_t));
	memcpy(dpufd->ov_block_infos_prev_prev, dpufd->ov_block_infos_prev,
		dpufd->ov_req_prev.ov_block_nums * sizeof(dss_overlay_block_t));
	dpufd->ov_req_prev_prev.ov_block_infos_ptr = (uint64_t)(uintptr_t)(dpufd->ov_block_infos_prev_prev);

	pov_req = &(dpufd->ov_req);
	memcpy(&dpufd->ov_req_prev, pov_req, sizeof(dss_overlay_t));
	memcpy(dpufd->ov_block_infos_prev, dpufd->ov_block_infos,
		pov_req->ov_block_nums * sizeof(dss_overlay_block_t));
	dpufd->ov_req_prev.ov_block_infos_ptr = (uint64_t)(uintptr_t)(dpufd->ov_block_infos_prev);

	return 0;

err_return:
	if (is_mipi_cmd_panel(dpufd))
		dpufd->vactive0_start_flag = 1;
	else
		single_frame_update(dpufd);

	dpufb_deactivate_vsync(dpufd);

	return ret;
}

static int hisi_online_play_config_locked(
	struct dpu_fb_data_type *dpufd)
{
	int ret;
	int enable_cmdlist;
	uint32_t cmdlist_idxs = 0;
	uint32_t cmdlist_pre_idxs = 0;
	bool vsync_time_checked = false;
	bool masklayer_maxbkl_flag = false;
	struct list_head lock_list;
	dss_overlay_t *pov_req = NULL;
	struct dss_cmdlist_idxs idxs_tmp = { &cmdlist_pre_idxs, &cmdlist_idxs, &enable_cmdlist };

	pov_req = &(dpufd->ov_req);
	enable_cmdlist = hisi_check_cmdlist_enabled(dpufd);


	INIT_LIST_HEAD(&lock_list);
	dpufb_layerbuf_lock(dpufd, pov_req, &lock_list);

	ret = hisi_check_cmdlist_prepare(dpufd, &cmdlist_pre_idxs, &cmdlist_idxs, enable_cmdlist);
	if (ret) {
		DPU_FB_ERR("online play prepare failed!\n");
		goto err_return;
	}

	ret = hisi_online_play_blocks_handle(dpufd, &vsync_time_checked, enable_cmdlist);
	if (ret) {
		DPU_FB_ERR("online play block config failed!\n");
		goto err_return;
	}

	ret = hisi_online_play_config_start(dpufd, &idxs_tmp, vsync_time_checked, &masklayer_maxbkl_flag);
	if (ret) {
		DPU_FB_ERR("online play start config failed!\n");
		goto err_return;
	}

	dpufb_layerbuf_flush(dpufd, &lock_list);

	if (g_debug_ovl_cmdlist && enable_cmdlist)
		hisi_cmdlist_dump_all_node(dpufd, NULL, cmdlist_idxs);

	dpufb_masklayer_backlight_flag_config(dpufd, masklayer_maxbkl_flag);

	return ret;

err_return:
	if (is_mipi_cmd_panel(dpufd))
		dpufd->vactive0_start_flag = 1;

	dpufb_layerbuf_lock_exception(dpufd, &lock_list);
	return ret;
}

static void dpufb_check_timediff(struct timeval *tv_begin, struct timeval *tv_end)
{
	uint32_t timediff;

	dpufb_get_timestamp(tv_end);
	timediff = dpufb_timestamp_diff(tv_begin, tv_end);
	if ((timediff >= g_debug_ovl_online_composer_time_threshold) && (g_fpga_flag == 0))
		DPU_FB_INFO("online play config took %u us!\n", timediff);
}

int hisi_ov_online_play(
	struct dpu_fb_data_type *dpufd,
	void __user *argp)
{
	static int dss_free_buffer_refcount;
	int ret;
	uint64_t ov_block_infos_ptr;
	dss_overlay_t *pov_req = NULL;
	struct timeval tv[3] = { {0}, {0}, {0}};  /* timestamp diff */

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	dpu_check_and_return(!argp, -EINVAL, ERR, "argp is nullptr!\n");

	if (!hisi_online_play_need_continue(dpufd, argp)) {
		DPU_FB_INFO("fb%d skip online play!\n", dpufd->index);
		return 0;
	}

	pov_req = &(dpufd->ov_req);
	dpufb_get_timestamp(&tv[0]);

	dpufb_activate_vsync(dpufd);
	if (hisi_online_play_prepare(dpufd, argp, dss_free_buffer_refcount)) {
		dpufb_deactivate_vsync(dpufd);
		return 0;
	}

	down(&dpufd->blank_sem0);
	ret = hisi_online_play_config_locked(dpufd);
	if (ret == 0) {
		dpufb_get_timestamp(&tv[1]);
		pov_req->online_wait_timediff = dpufb_timestamp_diff(&tv[0], &tv[1]);

		ov_block_infos_ptr = pov_req->ov_block_infos_ptr;
		pov_req->ov_block_infos_ptr = (uint64_t)0;
		if (copy_to_user((struct dss_overlay_t __user *)argp, pov_req, sizeof(dss_overlay_t))) {
			dpufb_buf_sync_close_fence(&pov_req->release_fence, &pov_req->retire_fence);
			DPU_FB_ERR("fb%d, copy_to_user failed\n", dpufd->index);
			ret = -EFAULT;
		} else {
			/* restore the original value from the variable ov_block_infos_ptr */
			pov_req->ov_block_infos_ptr = ov_block_infos_ptr;
			hisi_online_play_post_handle(dpufd, pov_req, dss_free_buffer_refcount);
			dss_free_buffer_refcount++;
		}
	}

	up(&dpufd->blank_sem0);
	dpufb_deactivate_vsync(dpufd);

	if (ret != 0) {
		if (g_underflow_count <= DSS_UNDERFLOW_COUNT) {
			g_underflow_count++;
			ret = ERR_RETURN_DUMP;
		}
	}

	dpufb_check_timediff(&tv[0], &tv[2]);

	return ret;
}

int hisi_online_play_bypass(struct dpu_fb_data_type *dpufd, const void __user *argp)
{
	int ret;
	int bypass;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	dpu_check_and_return(!argp, -EINVAL, ERR, "argp is nullptr!\n");

	/* only bypass primary display */
	if (dpufd->index != PRIMARY_PANEL_IDX)
		return -EINVAL;

	/* only active with cmd panel */
	if (!is_mipi_cmd_panel(dpufd))
		return -EINVAL;

	DPU_FB_INFO("+\n");

	ret = (int)copy_from_user(&bypass, argp, sizeof(bypass));
	if (ret) {
		DPU_FB_ERR("arg is invalid\n");
		return -EINVAL;
	}

	if (!dpufd->panel_power_on) {
		DPU_FB_INFO("fb%d, panel is power off!\n", dpufd->index);
		if (bypass) {
			DPU_FB_INFO("cannot bypss when power off!\n");
			return -EINVAL;
		}
	}
	(void)hisi_online_play_bypass_set(dpufd, bypass);

	DPU_FB_INFO("-\n");

	return 0;
}

bool hisi_online_play_bypass_set(struct dpu_fb_data_type *dpufd, int bypass)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is nullptr!\n");
		return false;
	}

	/* only bypass primary display */
	if (dpufd->index != PRIMARY_PANEL_IDX)
		return false;

	/* only active with cmd panel */
	if (!is_mipi_cmd_panel(dpufd))
		return false;

	DPU_FB_INFO("bypass = %d\n", bypass);

	if (bypass != 0) {
		if (dpufd->bypass_info.bypass_count > ONLINE_PLAY_BYPASS_MAX_COUNT) {
			DPU_FB_WARNING("already reach the max bypass count, disable bypass first\n");
			return false;
		}
		dpufd->bypass_info.bypass = true;
	} else {
		/* reset state, be available to start a new bypass cycle */
		dpufd->bypass_info.bypass = false;
		dpufd->bypass_info.bypass_count = 0;
	}

	return true;
}

bool hisi_online_play_bypass_check(struct dpu_fb_data_type *dpufd)
{
	bool ret = true;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is nullptr!\n");
		return false;
	}

	/* only bypass primary display */
	if (dpufd->index != PRIMARY_PANEL_IDX)
		return false;

	/* only active with cmd panel */
	if (!is_mipi_cmd_panel(dpufd))
		return false;

	if (g_debug_online_play_bypass) {
		if (g_debug_online_play_bypass < 0)
			g_debug_online_play_bypass = 0;
		if (dpufd->panel_power_on)
			hisi_online_play_bypass_set(dpufd, g_debug_online_play_bypass);
		g_debug_online_play_bypass = 0;
	}

	if (!dpufd->bypass_info.bypass)
		return false;

	dpufd->bypass_info.bypass_count++;
	if (dpufd->bypass_info.bypass_count > ONLINE_PLAY_BYPASS_MAX_COUNT) {
		/* not reset the bypass_count, wait the call of stopping bypass */
		dpufd->bypass_info.bypass = false;
		ret = false;
		DPU_FB_WARNING("reach the max bypass count, restore online play\n");
	}

	return ret;
}

