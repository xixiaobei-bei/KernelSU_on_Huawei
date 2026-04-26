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
#include "hisi_block_algorithm.h"
#include "hisi_overlay_cmdlist_utils.h"
#include "hisi_mmbuf_manager.h"

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V320) || \
	defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V501)
static void hisi_tuning_offline_clk_rate(struct dpu_fb_data_type *dpufd)
{
	uint64_t offline_clk_rate;
	uint64_t online_clk_rate;
	uint64_t external_clk_rate;
	uint64_t target_clk_rate;

	if (IS_ERR(dpufd->dss_pri_clk)) {
		DPU_FB_ERR("fb%u: dss_pri_clk is null.\n", dpufd->index);
		return;
	}

	if (dpufd->need_tuning_clk) {
		offline_clk_rate = dpufd->dss_vote_cmd.dss_pri_clk_rate;
		online_clk_rate = dpufd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_pri_clk_rate;

		if (dpufd_list[EXTERNAL_PANEL_IDX]) {
			external_clk_rate = dpufd_list[EXTERNAL_PANEL_IDX]->dss_vote_cmd.dss_pri_clk_rate;
			target_clk_rate = (external_clk_rate > online_clk_rate) ? external_clk_rate : online_clk_rate;
		} else {
			target_clk_rate = online_clk_rate;
		}

		if (offline_clk_rate >= target_clk_rate) {
			(void)clk_set_rate(dpufd->dss_pri_clk, offline_clk_rate);
		}

		DPU_FB_DEBUG("fb%u set online(%llu) -> offline(%llu)\n",
			dpufd->index, online_clk_rate, offline_clk_rate);
	}
}

static void hisi_recovery_online_clk_rate(struct dpu_fb_data_type *dpufd)
{
	uint64_t online_clk_rate;
	uint64_t external_clk_rate;
	uint64_t target_clk_rate;

	if (IS_ERR(dpufd->dss_pri_clk)) {
		DPU_FB_ERR("fb%u: dss_pri_clk is null\n", dpufd->index);
		return;
	}

	if (dpufd->need_tuning_clk) {
		online_clk_rate = dpufd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_pri_clk_rate;

		if (dpufd_list[EXTERNAL_PANEL_IDX]) {
			external_clk_rate = dpufd_list[EXTERNAL_PANEL_IDX]->dss_vote_cmd.dss_pri_clk_rate;
			target_clk_rate = external_clk_rate > online_clk_rate ? external_clk_rate : online_clk_rate;
		} else {
			target_clk_rate = online_clk_rate;
		}

		(void)clk_set_rate(dpufd->dss_pri_clk, target_clk_rate);
		dpufd->dss_vote_cmd.dss_pri_clk_rate = target_clk_rate;

		dpufd->need_tuning_clk = false;

		DPU_FB_DEBUG("fb%u recovery online[%llu]\n", dpufd->index, online_clk_rate);
	}
}
#endif

static uint32_t hisi_get_timeout_interval(void)
{
	uint32_t timeout_interval;

	if (g_fpga_flag == 0)
		timeout_interval = DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC;
	else
		timeout_interval = DSS_COMPOSER_TIMEOUT_THRESHOLD_FPGA;

	return timeout_interval;
}

static int32_t hisi_get_wb_type(dss_overlay_t *pov_req)
{
	int32_t wb_type = -1;
	dss_wb_layer_t *wb_layer = NULL;

	if (!pov_req) {
		DPU_FB_ERR("pov_req is NULL\n");
		return -EINVAL;
	}
	wb_layer = &(pov_req->wb_layer_infos[0]);

	if (pov_req->wb_layer_nums == MAX_DSS_DST_NUM) {
		wb_type = WB_TYPE_WCH0_WCH1;
	} else {
		if (wb_layer->chn_idx == DSS_WCHN_W0)
			wb_type = WB_TYPE_WCH0;
		else if (wb_layer->chn_idx == DSS_WCHN_W1)
			wb_type = WB_TYPE_WCH1;
		else if (wb_layer->chn_idx == DSS_WCHN_W2)
			wb_type = WB_TYPE_WCH2;
		else
			DPU_FB_ERR("wb_layer->chn_idx[%d] no support\n", wb_layer->chn_idx);
	}

	return wb_type;
}

static int hisi_wait_interrupt_event(struct dpu_fb_data_type *dpufd, int32_t wb_type)
{
	uint32_t timeout_interval = hisi_get_timeout_interval();
	int times = 0;
	int ret;

	while (times < 50) {
		/*lint -e666*/
		ret = wait_event_interruptible_timeout(dpufd->cmdlist_info->cmdlist_wb_wq[wb_type], /*lint !e578*/
			(dpufd->cmdlist_info->cmdlist_wb_done[wb_type] == 1), /* wb type status is true */
			msecs_to_jiffies(timeout_interval));
		/*lint +e666*/

		if (ret != -ERESTARTSYS)
			break;

		times++;
		mdelay(10); /* 10ms */
	}

	return ret;
}

static int hisi_check_two_wch_wb_finish(
	struct dpu_fb_data_type *dpufd,
	uint32_t last_cmdlist_idxs,
	int last_mctl_idx,
	uint32_t cmdlist_idxs,
	int mctl_idx)
{
	int ret;
	int ret1;
	int ret_cmdlist_state;
	int i;
	struct timeval tv0;
	struct timeval tv1;

	dpufb_get_timestamp(&tv0);

	ret = hisi_wait_interrupt_event(dpufd, WB_TYPE_WCH0);

	ret1 = hisi_wait_interrupt_event(dpufd, WB_TYPE_WCH1);

	dpufb_get_timestamp(&tv1);

	ret_cmdlist_state = hisi_cmdlist_check_cmdlist_state(dpufd, (cmdlist_idxs | last_cmdlist_idxs));
	if ((ret <= 0) || (ret1 <= 0) || (ret_cmdlist_state < 0)) {
		DPU_FB_ERR("compose timeout, GLB_CPU_OFF_INTS=0x%x, ret=%d, ret1=%d,"
			" ret_rch_state=%d, diff=%d usecs,wb_done_wch0=%d, wb_done_wch1=%d\n",
			inp32(dpufd->dss_base + GLB_CPU_OFF_INTS), ret, ret1, ret_cmdlist_state,
			dpufb_timestamp_diff(&tv0, &tv1), dpufd->cmdlist_info->cmdlist_wb_done[WB_TYPE_WCH0],
			dpufd->cmdlist_info->cmdlist_wb_done[WB_TYPE_WCH1]);
		ret = -ETIMEDOUT;
	} else {
		/* remove last mctl ch & ov */
		hisi_remove_mctl_mutex(dpufd, last_mctl_idx, last_cmdlist_idxs);
		/* remove mctl ch & ov */
		hisi_remove_mctl_mutex(dpufd, mctl_idx, cmdlist_idxs);
		ret = 0;
	}

	for (i = 0; i < WB_TYPE_WCH0_WCH1; i++) {
		dpufd->cmdlist_info->cmdlist_wb_flag[i] = 0;
		dpufd->cmdlist_info->cmdlist_wb_done[i] = 0;
	}

	return ret;
}

static int hisi_check_wch_wb_finish(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	uint32_t wb_compose_type,
	uint32_t cmdlist_idxs,
	int mctl_idx)
{
	int ret;
	int32_t wb_type;
	int ret_cmdlist_state;
	struct timeval tv0;
	struct timeval tv1;

	wb_type = hisi_get_wb_type(pov_req);
	if (wb_type < 0) {
		DPU_FB_ERR("unexpect wb_type = %d", wb_type);
		return -EINVAL;
	}

	dpufb_get_timestamp(&tv0);

	ret = hisi_wait_interrupt_event(dpufd, wb_type);

	dpufb_get_timestamp(&tv1);
	dpufd->cmdlist_info->cmdlist_wb_flag[wb_type] = 0;
	dpufd->cmdlist_info->cmdlist_wb_done[wb_type] = 0;

	ret_cmdlist_state = hisi_cmdlist_check_cmdlist_state(dpufd, cmdlist_idxs);
	if ((ret <= 0) || (ret_cmdlist_state < 0)) {
		DPU_FB_ERR("compose timeout, wb_compose_type=%d, GLB_CPU_OFF_INTS=0x%x,"
			"ret=%d, ret_rch_state=%d, diff =%d usecs!\n",
			wb_compose_type, inp32(dpufd->dss_base + GLB_CPU_OFF_INTS), ret, ret_cmdlist_state,
			dpufb_timestamp_diff(&tv0, &tv1));

		hisi_cmdlist_dump_all_node(dpufd, pov_req, cmdlist_idxs);
		ret = -ETIMEDOUT;
	} else {
		/* remove mctl ch & ov */
		hisi_remove_mctl_mutex(dpufd, mctl_idx, cmdlist_idxs);
		ret = 0;
	}

	return ret;
}

static uint32_t hisi_get_block_info_layers_channels(
	dss_overlay_block_t *block_infos,
	uint32_t block_nums)
{
	dss_overlay_block_t *block_info = NULL;
	dss_layer_t *layer_info = NULL;
	uint32_t chn_idx = 0;
	uint32_t i;
	uint32_t j;

	if (!block_infos)
		return 0;

	if (block_nums > HISI_DSS_OV_BLOCK_NUMS)
		return 0;

	for (i = 0; i < block_nums; i++) {
		block_info = &block_infos[i];
		for (j = 0; j < block_info->layer_nums; j++) {
			layer_info = &block_info->layer_infos[j];
			if ((layer_info->chn_idx >= DSS_CHN_MAX_DEFINE) ||
				(layer_info->chn_idx < DSS_RCHN_D2))
				continue;

			if (layer_info->img.shared_fd < 0 ||
				(layer_info->img.phy_addr == 0 && layer_info->img.vir_addr == 0))
				continue;

			if (!(chn_idx & (1 << (uint32_t)layer_info->chn_idx)))
				chn_idx |= 1 << (uint32_t)layer_info->chn_idx;
		}
	}

	return chn_idx;
}

bool hisi_offline_chn_include_online_chn(
	dss_overlay_block_t *offline_block_infos,
	uint32_t offline_block_nums)
{
	uint32_t offline_chn_idx;
	uint32_t online_pre_chn_idx;
	uint32_t online_pre_pre_chn_idx;
	struct dpu_fb_data_type *dpufd_primary = dpufd_list[PRIMARY_PANEL_IDX];

	if (is_mipi_cmd_panel(dpufd_primary))
		return false;

	offline_chn_idx = hisi_get_block_info_layers_channels(offline_block_infos, offline_block_nums);
	online_pre_chn_idx = hisi_get_block_info_layers_channels(
		(dss_overlay_block_t *)(uintptr_t)(dpufd_primary->ov_req_prev.ov_block_infos_ptr),
		dpufd_primary->ov_req_prev.ov_block_nums);
	if (offline_chn_idx & online_pre_chn_idx) {
		DPU_FB_ERR("offline_chn_idx(0x%x) be included at online_chn_idx(0x%x)\n",
			offline_chn_idx, online_pre_chn_idx);
		return true;
	}

	online_pre_pre_chn_idx = hisi_get_block_info_layers_channels(
		(dss_overlay_block_t *)(uintptr_t)(dpufd_primary->ov_req_prev_prev.ov_block_infos_ptr),
		dpufd_primary->ov_req_prev_prev.ov_block_nums);

	if (offline_chn_idx & online_pre_pre_chn_idx)
		DPU_FB_INFO("offline_chn_idx(0x%x) be included at online_pre_pre_chn_idx(0x%x)\n",
			offline_chn_idx, online_pre_pre_chn_idx);

	return false;
}

static void hisi_dss_offline_composer_on(struct dpu_fb_data_type *dpufd)
{
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd_primary = NULL;
	int prev_refcount;

	fbi = dpufd->fbi;
	if (!fbi) {
		DPU_FB_ERR("fbi is NULL\n");
		return;
	}

	DPU_FB_DEBUG("fb%u, +.\n", dpufd->index);

	dpufd_primary = dpufd_list[PRIMARY_PANEL_IDX];
	dpufb_activate_vsync(dpufd_primary);

	down(&dpufd->offline_composer_sr_sem);
	mutex_lock(&(dpufd_primary->vsync_ctrl.vsync_lock));
	dpufd_primary->vsync_ctrl.vsync_ctrl_offline_enabled = 1;
	mutex_unlock(&(dpufd_primary->vsync_ctrl.vsync_lock));

	prev_refcount = (dpufd->offline_composer_sr_refcount)++;
	if (!prev_refcount) {
		if (fbi->fbops->fb_blank)
			fbi->fbops->fb_blank(FB_BLANK_UNBLANK, fbi);
	}

	up(&dpufd->offline_composer_sr_sem);

	DPU_FB_DEBUG("fb%u, -.\n", dpufd->index);
}

static void hisi_dss_offline_composer_off(struct dpu_fb_data_type *dpufd)
{
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd_primary = NULL;
	int new_refcount;

	fbi = dpufd->fbi;
	if (!fbi) {
		DPU_FB_ERR("fbi is NULL\n");
		return;
	}

	DPU_FB_DEBUG("fb%u, +.\n", dpufd->index);

	dpufd_primary = dpufd_list[PRIMARY_PANEL_IDX];

	down(&dpufd->offline_composer_sr_sem);
	new_refcount = --(dpufd->offline_composer_sr_refcount);
	if (new_refcount < 0)
		DPU_FB_ERR("dss new_refcount err\n");

	if (!new_refcount) {
		if (fbi->fbops->fb_blank)
			fbi->fbops->fb_blank(FB_BLANK_POWERDOWN, fbi);
	}

	mutex_lock(&(dpufd_primary->vsync_ctrl.vsync_lock));
	dpufd_primary->vsync_ctrl.vsync_ctrl_offline_enabled = 0;
	mutex_unlock(&(dpufd_primary->vsync_ctrl.vsync_lock));

	up(&dpufd->offline_composer_sr_sem);

	dpufb_deactivate_vsync(dpufd_primary);

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);
}

static int hisi_add_clear_module_reg_node(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	int cmdlist_idxs,
	bool enable_cmdlist,
	int *use_comm_mmbuf)
{
	int ret;

	if ((cmdlist_idxs < 0) || (cmdlist_idxs >= HISI_DSS_CMDLIST_IDXS_MAX)) {
		DPU_FB_ERR("cmdlist_idxs is invalid\n");
		return -EINVAL;
	}

	ret = hisi_dss_module_init(dpufd);
	if (ret != 0) {
		DPU_FB_ERR("hisi_dss_module_init failed! ret=%d\n", ret);
		return ret;
	}

	hisi_dss_prev_module_set_regs(dpufd, pov_req, cmdlist_idxs, enable_cmdlist, use_comm_mmbuf);

	return 0;
}

static void hisi_offline_play_free_data(dss_overlay_t *pov_req)
{
	dss_overlay_block_t *pov_h_block_infos = NULL;

	if (pov_req) {
		pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
		if (pov_h_block_infos) {
			kfree(pov_h_block_infos);
			pov_h_block_infos = NULL;
		}
		kfree(pov_req);
	}
	pov_req = NULL;
}

static dss_overlay_t *hisi_offline_play_alloc_data(void)
{
	dss_overlay_t *pov_req = NULL;

	pov_req = (dss_overlay_t *)kmalloc(sizeof(dss_overlay_t), GFP_ATOMIC);
	if (!pov_req) {
		HISI_FB_INFO("pov_req gfp_atomic alloc failed!\n");
		pov_req = (dss_overlay_t *)kmalloc(sizeof(dss_overlay_t), GFP_KERNEL);
		if (!pov_req) {
			HISI_FB_ERR("pov_req gfp_kernel alloc failed!\n");
			return NULL;
		}
	}
	if (pov_req)
		dpufb_dss_overlay_info_init(pov_req);

	return pov_req;
}

static void hisi_offline_clear(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	int enable_cmdlist,
	uint32_t cmdlist_idxs,
	bool reset)
{
	hisi_drm_layer_offline_clear(dpufd, pov_req);
	dpufb_buf_sync_signal(dpufd);

	if (enable_cmdlist) {
		if (g_debug_ovl_offline_composer_hold || g_debug_ovl_block_composer || g_debug_ovl_cmdlist)
			hisi_cmdlist_dump_all_node(dpufd, pov_req, cmdlist_idxs);

		if (reset) {
			if (g_debug_ovl_offline_composer_hold)
				mdelay(HISI_DSS_COMPOSER_HOLD_TIME);

			hisi_cmdlist_config_reset(dpufd, pov_req, cmdlist_idxs);
		}
		hisi_cmdlist_del_node(dpufd, pov_req, cmdlist_idxs);
		hisi_dss_mmbuf_info_clear(dpufd, 0);
	}
	hisi_offline_play_free_data(pov_req);
}

static int hisi_get_ov_data_from_user(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, const void __user *argp)
{
	int ret;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	uint32_t ov_block_size;

	ret = copy_from_user(pov_req, argp, sizeof(dss_overlay_t));
	if (ret) {
		DPU_FB_ERR("fb%u, copy_from_user failed!\n", dpufd->index);
		return -EINVAL;
	}

	pov_req->release_fence = -1;
	if ((pov_req->ov_block_nums <= 0) || (pov_req->ov_block_nums > HISI_DSS_OV_BLOCK_NUMS)) {
		DPU_FB_ERR("fb%u, ov_block_nums(%d) is out of range!\n", dpufd->index, pov_req->ov_block_nums);
		return -EINVAL;
	}

	if ((dss_overlay_block_t *)(uintptr_t)pov_req->ov_block_infos_ptr == NULL) {
		DPU_FB_ERR("pov_req->ov_block_infos_ptr is NULL\n");
		return -EINVAL;
	}

	ov_block_size = pov_req->ov_block_nums * sizeof(dss_overlay_block_t);
	pov_h_block_infos = (dss_overlay_block_t *)kmalloc(ov_block_size, GFP_ATOMIC);
	if (!pov_h_block_infos) {
		HISI_FB_INFO("fb%u, pov_h_block_infos gfp_atomic alloc failed!\n", dpufd->index);
		pov_h_block_infos = (dss_overlay_block_t *)kmalloc(ov_block_size, GFP_KERNEL);
		if (!pov_h_block_infos) {
			HISI_FB_ERR("pov_h_block_infos gfp_kernel alloc failed!\n");
			pov_req->ov_block_infos_ptr = 0;
			return -EINVAL;
		}
	}

	memset(pov_h_block_infos, 0, ov_block_size);

	ret = copy_from_user(pov_h_block_infos, (dss_overlay_block_t *)(uintptr_t)pov_req->ov_block_infos_ptr,
		ov_block_size);
	if (ret) {
		DPU_FB_ERR("fb%u, dss_overlay_block_t copy_from_user failed!\n", dpufd->index);
		kfree(pov_h_block_infos);
		pov_h_block_infos = NULL;
		pov_req->ov_block_infos_ptr = 0;
		return -EINVAL;
	}

	ret = hisi_dss_check_userdata(dpufd, pov_req, pov_h_block_infos);
	if (ret != 0) {
		DPU_FB_ERR("fb%u, hisi_dss_check_userdata failed!\n", dpufd->index);
		kfree(pov_h_block_infos);
		pov_h_block_infos = NULL;
		pov_req->ov_block_infos_ptr = 0;
		return -EINVAL;
	}

	pov_req->ov_block_infos_ptr = (uint64_t)(uintptr_t)pov_h_block_infos;

	return ret;
}

static bool hisi_check_csc_config_needed(dss_overlay_t *pov_req_h_v)
{
	dss_overlay_block_t *pov_h_v_block = (dss_overlay_block_t *)(uintptr_t)(pov_req_h_v->ov_block_infos_ptr);
	bool is_yuv_single_layer = (pov_h_v_block->layer_nums == 1) &&
		(is_yuv(pov_h_v_block->layer_infos[0].img.format));

	bool is_yuv_single_wb_layer = (pov_req_h_v->wb_layer_nums == 1) &&
		(is_yuv(pov_req_h_v->wb_layer_infos[0].dst.format));

	bool is_rect_equal = (pov_h_v_block->layer_infos[0].dst_rect.w == pov_req_h_v->wb_layer_infos[0].dst_rect.w) &&
		(pov_h_v_block->layer_infos[0].dst_rect.h == pov_req_h_v->wb_layer_infos[0].dst_rect.h);

	/* check whether csc config needed or not */
	if (!is_yuv_single_layer)
		return true;

	if (!is_yuv_single_wb_layer)
		return true;

	if (!is_rect_equal)
		return true;

	return false;
}

static int hisi_vactive0_start_wait(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	uint32_t use_comm_mmbuf)
{
	struct dpu_fb_data_type *online_dpufd = NULL;
	struct timeval tv0;
	struct timeval tv1;
	int ret = 1;
	int times = 0;
	uint32_t timeout_interval;
	uint32_t prev_vactive0_start;

	timeout_interval = hisi_get_timeout_interval();
	if (use_comm_mmbuf & (1 << DSS_OVL0))
		online_dpufd = dpufd_list[PRIMARY_PANEL_IDX];

	if (use_comm_mmbuf & (1 << DSS_OVL1))
		online_dpufd = dpufd_list[EXTERNAL_PANEL_IDX];

	if (online_dpufd && online_dpufd->panel_power_on) {
		dpufb_get_timestamp(&tv0);
		if (is_mipi_cmd_panel(online_dpufd) && (online_dpufd->vactive0_start_flag == 0)) {
REDO_CMD:
			/*lint -e666*/
			ret = wait_event_interruptible_timeout(online_dpufd->vactive0_start_wq,  /*lint !e578*/
				online_dpufd->vactive0_start_flag, msecs_to_jiffies(timeout_interval));
			/*lint +e666*/

		} else if (!is_mipi_cmd_panel(online_dpufd)) {
			prev_vactive0_start = online_dpufd->vactive0_start_flag;
REDO_VIDEO:
			/*lint -e666*/
			ret = wait_event_interruptible_timeout(online_dpufd->vactive0_start_wq,  /*lint !e578*/
				(prev_vactive0_start != online_dpufd->vactive0_start_flag),
				msecs_to_jiffies(timeout_interval));
			/*lint +e666*/
		}

		if (ret == -ERESTARTSYS) {
			if (times < 50) {  /* wait 500ms */
				times++;
				mdelay(10);  /* delay 10ms */
				if (is_mipi_cmd_panel(online_dpufd))
					goto REDO_CMD;

				if (is_mipi_video_panel(online_dpufd))
					goto REDO_VIDEO;
			}
		}

		if (ret <= 0) {
			dpufb_get_timestamp(&tv1);
			DPU_FB_ERR("fb%u, wait_for vactive0_start_flag timeout!ret=%d, "
				"vactive0_start_flag=%d, TIMESTAMP_DIFF is %u us!\n",
				online_dpufd->index, ret, online_dpufd->vactive0_start_flag,
				dpufb_timestamp_diff(&tv0, &tv1));

			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int hisi_get_mctl_idx(dss_overlay_t *pov_req)
{
	int mctl_idx;

	if (hisi_get_wb_type(pov_req) == WB_TYPE_WCH2)
		mctl_idx = DSS_MCTL5;
	else
		mctl_idx = pov_req->ovl_idx;

	return mctl_idx;
}

static bool hisi_get_parallel_compose_flag(
	struct dpu_fb_data_type *dpufd,
	uint32_t wb_compose_type)
{
	bool parallel_compose_flag = false;

	if ((dpufd->wb_info.to_be_continued == DSS_LAYER_PARALLEL_COMPOSE) &&
		(wb_compose_type == DSS_WB_COMPOSE_PRIMARY))
		parallel_compose_flag = true;

	return parallel_compose_flag;
}

static int hisi_ov_layers_compose(struct dpu_fb_data_type *dpufd, dss_overlay_block_t *pov_h_v_block,
	struct hisi_ov_compose_rect *ov_compose_rect, struct hisi_ov_compose_flag *ov_compose_flag)
{
	uint32_t i;
	int ret;
	dss_layer_t *layer = NULL;

	for (i = 0; i < pov_h_v_block->layer_nums; i++) {
		layer = &(pov_h_v_block->layer_infos[i]);
		memset(ov_compose_rect->clip_rect, 0, sizeof(dss_rect_ltrb_t));
		memset(ov_compose_rect->aligned_rect, 0, sizeof(dss_rect_t));
		ov_compose_flag->rdma_stretch_enable = false;

		ret = hisi_ov_compose_handler(dpufd, pov_h_v_block, layer, ov_compose_rect, ov_compose_flag);
		dpu_check_and_return(ret != 0, ret, ERR, "fb%u, compose handler failed! ret=%d\n", dpufd->index, ret);
	}

	return 0;
}

static int hisi_wb_ov_layers_compose(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_rect_t *wb_ov_block_rect, bool last_block, struct hisi_ov_compose_flag ov_compose_flag)
{
	uint32_t i;
	int ret;
	dss_wb_layer_t *wb_layer = NULL;
	dss_overlay_t *pov_req_h_v = &(dpufd->ov_req);
	uint32_t type;

	for (i = 0; i < pov_req_h_v->wb_layer_nums; i++) {
		wb_layer = &(pov_req_h_v->wb_layer_infos[i]);

		ret = hisi_wb_compose_handler(dpufd, wb_layer, wb_ov_block_rect, last_block, ov_compose_flag);
		if (ret != 0) {
			DPU_FB_ERR("fb%u, hisi dss write back config failed! ret=%d\n", dpufd->index, ret);
			return ret;
		}

		ret = hisi_get_wb_type(pov_req);
		if (ret < 0) {
			DPU_FB_ERR("hisi_get_wb_type error");
			return ret;
		}
		type = ret;
		ret = hisi_wb_ch_module_set_regs(dpufd, type, wb_layer, ov_compose_flag.enable_cmdlist);
		if (ret != 0)
			return ret;
	}

	return 0;
}

static int hisi_offline_play_h_block_handle(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	dss_rect_t *wb_ov_block_rect,
	int h_block_index,
	bool last_block)
{
	int ret;
	dss_overlay_block_t *pov_h_v_block = NULL;
	dss_overlay_t *pov_req_h_v = NULL;
	dss_rect_ltrb_t clip_rect;
	dss_rect_t aligned_rect;
	struct hisi_ov_compose_rect ov_compose_rect = { &pov_req->wb_layer_infos[0].dst_rect, wb_ov_block_rect,
		&clip_rect, &aligned_rect };
	struct hisi_ov_compose_flag ov_compose_flag = { false, false, true, false };
	struct ov_module_set_regs_flag ov_module_flag = { false, false, 0, 0 };

	pov_req_h_v = &(dpufd->ov_req);
	pov_h_v_block = (dss_overlay_block_t *)(uintptr_t)(pov_req_h_v->ov_block_infos_ptr);

	ov_compose_flag.csc_needed = hisi_check_csc_config_needed(pov_req_h_v);
	hisi_dss_aif_handler(dpufd, pov_req_h_v, pov_h_v_block);
	ov_compose_flag.enable_cmdlist = g_enable_ovl_cmdlist_offline;

	ret = hisi_dss_ovl_base_config(dpufd, pov_req_h_v, pov_h_v_block, wb_ov_block_rect, h_block_index);
	dpu_check_and_return(ret != 0, -EINVAL, ERR, "offline ovl base config faild!\n");

	ret = hisi_ov_layers_compose(dpufd, pov_h_v_block, &ov_compose_rect, &ov_compose_flag);
	if (ret != 0)
		return ret;

	if (h_block_index == 0) {
		ret = hisi_wb_ov_layers_compose(dpufd, pov_req, wb_ov_block_rect, last_block, ov_compose_flag);
		if (ret != 0)
			return ret;
	}

	ret = hisi_dss_mctl_ov_config(dpufd, pov_req, pov_req_h_v->ovl_idx, ov_compose_flag.has_base, false);
	dpu_check_and_return(ret != 0, -EINVAL, ERR, "fb%u, mctl ov config faild!\n", dpufd->index);

	ov_module_flag.enable_cmdlist = ov_compose_flag.enable_cmdlist;
	ret = hisi_dss_ov_module_set_regs(dpufd, pov_req, pov_req_h_v->ovl_idx, ov_module_flag);
	dpu_check_and_return(ret != 0, -EINVAL, ERR, "fb%u, ov module set faild!\n", dpufd->index);

	return 0;
}

static int hisi_ov_block_config(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	dss_overlay_block_t *pov_h_block_infos)
{
	int ret = 0;
	int k;
	uint32_t m;
	int block_num = 0;
	dss_wb_layer_t *wb_layer4block = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	dss_overlay_t *pov_req_h_v = NULL;
	dss_rect_t wb_ov_block_rect;
	bool last_block = false;

	wb_layer4block = &(pov_req->wb_layer_infos[0]);
	pov_req_h_v = &(dpufd->ov_req);
	pov_req_h_v->ov_block_infos_ptr = (uint64_t)(uintptr_t)(dpufd->ov_block_infos);

	(void)hisi_dss_module_init(dpufd);
	for (m = 0; m < pov_req->ov_block_nums; m++) {
		pov_h_block = &(pov_h_block_infos[m]);
		ret = get_ov_block_rect(pov_req, pov_h_block, &block_num, dpufd->ov_block_rects, false);
		if ((ret != 0) || (block_num == 0) || (block_num >= HISI_DSS_CMDLIST_BLOCK_MAX)) {
			DPU_FB_ERR("get_ov_block_rect failed! ret=%d, block_num[%d]\n", ret, block_num);
			ret = -1;
			return ret;
		}

		for (k = 0; k < block_num; k++) {
			ret = get_block_layers(pov_req, pov_h_block, *dpufd->ov_block_rects[k],
				pov_req_h_v, block_num);
			dpu_check_and_return(ret != 0, -EINVAL, ERR, "offline get_block_layers faild!\n");

			ret = get_wb_ov_block_rect(*dpufd->ov_block_rects[k], wb_layer4block->src_rect,
					&wb_ov_block_rect, pov_req_h_v, pov_req->wb_compose_type);
			if (ret == 0) {
				DPU_FB_ERR("no cross! ov_block_rects[%d]{%d %d %d %d}, wb src_rect{%d %d %d %d}\n", k,
					dpufd->ov_block_rects[k]->x, dpufd->ov_block_rects[k]->y,
					dpufd->ov_block_rects[k]->w, dpufd->ov_block_rects[k]->h,
					wb_layer4block->src_rect.x, wb_layer4block->src_rect.y,
					wb_layer4block->src_rect.w, wb_layer4block->src_rect.h);
				continue;
			}

			last_block = (k == (block_num - 1)) ? true : false;
			ret = hisi_offline_play_h_block_handle(dpufd, pov_req, &wb_ov_block_rect, m, last_block);
			if (ret != 0) {
				DPU_FB_ERR("fb%u play_h_block_handle failed! ret=%d\n", dpufd->index, ret);
				return -1;
			}

			if (k < (block_num - 1))
				(void)hisi_dss_module_init(dpufd);
		}
	}
	return ret;
}

static int hisi_check_panel_status_and_composer_type(dss_overlay_t *pov_req)
{
	int32_t wb_type;
	struct dpu_fb_data_type *primary_dpufd = NULL;

	primary_dpufd = dpufd_list[PRIMARY_PANEL_IDX];
	if (!primary_dpufd) {
		DPU_FB_INFO("primary_dpufd is NULL Pointer\n");
		return 0;
	}

	if (pov_req->wb_layer_infos[0].dst.display_id == EXTERNAL_PANEL_IDX)
		return 0;

	if (!primary_dpufd->panel_power_on &&
		(pov_req->wb_compose_type == DSS_WB_COMPOSE_PRIMARY)) {
		DPU_FB_INFO("primary panel is power off, quit offline compose!\n");
		return -EINVAL;
	}

	wb_type = hisi_get_wb_type(pov_req);
	if ((wb_type < WB_TYPE_WCH0) || (wb_type >= WB_TYPE_MAX)) {
		DPU_FB_ERR("hisi_get_wb_type failed!\n");
		return -EINVAL;
	}

	return 0;
}

static int hisi_offline_play_data_prepare(
	struct dpu_fb_data_type *dpufd,
	const void __user *argp,
	dss_overlay_t **pov_req_out)
{
	int ret;
	dss_overlay_t *pov_req = NULL;

	pov_req = hisi_offline_play_alloc_data();
	if (!pov_req) {
		HISI_FB_ERR("fb%u, dss_overlay_t alloc failed!\n", dpufd->index);
		return -1;
	}

	ret = hisi_get_ov_data_from_user(dpufd, pov_req, argp);
	if (ret != 0) {
		DPU_FB_ERR("fb%u, hisi_get_ov_data_from_user failed! ret=%d\n", dpufd->index, ret);
		hisi_offline_play_free_data(pov_req);
		return -1;
	}

	ret = hisi_check_panel_status_and_composer_type(pov_req);
	if (ret != 0) {
		hisi_offline_play_free_data(pov_req);
		/* primary panel is power off, quit offline compose */
		return 1;
	}
	*pov_req_out = pov_req;

	return 0;
}

static void hisi_offline_handle_device_power(
	struct dpu_fb_data_type *dpufd,
	uint32_t wb_compose_type,
	bool poweron)
{
#ifdef SUPPORT_COPYBIT_VOTE_CTRL
	if (poweron) {
		hisi_dss_offline_composer_on(dpufd);
		if (wb_compose_type == DSS_WB_COMPOSE_COPYBIT)
			(void)dpufb_offline_vote_ctrl(dpufd, true);
	} else {
		if (wb_compose_type == DSS_WB_COMPOSE_COPYBIT)
			(void)dpufb_offline_vote_ctrl(dpufd, false);
		hisi_dss_offline_composer_off(dpufd);
	}
#else
	if (poweron) {
		hisi_dss_offline_composer_on(dpufd);
		hisi_tuning_offline_clk_rate(dpufd);
	} else {
		hisi_recovery_online_clk_rate(dpufd);
		hisi_dss_offline_composer_off(dpufd);
	}
#endif
}

static int hisi_offline_play_device_prepare(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	int *enable_cmdlist,
	uint32_t *cmdlist_idxs)
{
	int ret;
	struct list_head lock_list;
	dss_overlay_block_t *pov_h_block_infos = NULL;

	INIT_LIST_HEAD(&lock_list);
	hisi_offline_handle_device_power(dpufd, pov_req->wb_compose_type, true);
	dpufb_offline_layerbuf_lock(dpufd, pov_req, &lock_list);
	dpufb_layerbuf_flush(dpufd, &lock_list);

	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
	ret = hisi_offline_chn_include_online_chn(pov_h_block_infos, pov_req->ov_block_nums);
	if (ret != 0) {
		DPU_FB_ERR("fb%u, hisi_offline_ch_include_online_ch failed!, wb_compose_type=%d\n",
			dpufd->index, pov_req->wb_compose_type);
		dpufb_buf_sync_signal(dpufd);
		hisi_offline_handle_device_power(dpufd, pov_req->wb_compose_type, false);
		return -1;
	}

	dpufd->mmbuf_info = hisi_dss_mmbuf_info_get(dpufd, 0);
	if (!dpufd->mmbuf_info) {
		DPU_FB_ERR("get mmbuf_info is NULL\n");
		dpufb_buf_sync_signal(dpufd);
		hisi_offline_handle_device_power(dpufd, pov_req->wb_compose_type, false);
		return -1;
	}

	*enable_cmdlist = g_enable_ovl_cmdlist_offline;
	if (*enable_cmdlist) {
		dpufd->set_reg = hisi_cmdlist_set_reg;

		hisi_cmdlist_data_get_offline(dpufd, pov_req);
		ret = hisi_cmdlist_get_cmdlist_idxs(pov_req, NULL, cmdlist_idxs);
		if (ret != 0) {
			DPU_FB_ERR("fb%u get_cmdlist_idxs failed! ret = %d\n", ret, dpufd->index);
			hisi_dss_mmbuf_info_clear(dpufd, 0);
			dpufb_buf_sync_signal(dpufd);
			hisi_offline_handle_device_power(dpufd, pov_req->wb_compose_type, false);
			return -1;
		}
	} else {
		dpufd->set_reg = dpufb_set_reg;
	}

	return 0;
}

static int hisi_offline_play_prepare(
	struct dpu_fb_data_type *dpufd,
	const void __user *argp,
	dss_overlay_t **pov_req_out,
	int *enable_cmdlist,
	uint32_t *cmdlist_idxs)
{
	int ret;

	ret = hisi_offline_play_data_prepare(dpufd, argp, pov_req_out);
	if (ret != 0) {
		DPU_FB_INFO("fb%u, data prepare failed!\n", dpufd->index);
		return ret;
	}

	ret = hisi_offline_play_device_prepare(dpufd, *pov_req_out, enable_cmdlist, cmdlist_idxs);
	if (ret != 0) {
		DPU_FB_INFO("fb%u, device prepare failed!\n", dpufd->index);
		hisi_offline_play_free_data(*pov_req_out);
		return ret;
	}

	return 0;
}

static int hisi_offline_play_config_locked(
	struct dpu_fb_data_type *dpufd,
	const void __user *argp,
	dss_overlay_t **ov_req,
	uint32_t *use_comm_mmbuf,
	struct dss_cmdlist_idxs *cmdlist_idxs_tmp)
{
	int ret = 0;
	uint32_t wb_compose_type;
	dss_overlay_t *pov_req = NULL;
	dss_overlay_block_t *pov_h_block_infos = NULL;

	down(&(dpufd->cmdlist_info->cmdlist_wb_common_sem));
	ret = hisi_offline_play_prepare(dpufd, argp, ov_req,
		cmdlist_idxs_tmp->enable_cmdlist, cmdlist_idxs_tmp->cmdlist_idxs);

	if (ret != 0) {
		DPU_FB_INFO("fb%u, hisi offline play prepare failed!\n", dpufd->index);
		up(&(dpufd->cmdlist_info->cmdlist_wb_common_sem));
		return ret;
	}
	pov_req = *ov_req;
	wb_compose_type = pov_req->wb_compose_type;
	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);

	hisi_drm_layer_offline_config(dpufd, pov_req);
	ret = hisi_ov_block_config(dpufd, pov_req, pov_h_block_infos);
	if (ret != 0) {
		DPU_FB_ERR("fb%u, hisi_ov_block_config failed!\n", dpufd->index);
		goto err_return;
	}

	ret = hisi_add_clear_module_reg_node(dpufd, pov_req,
		*(cmdlist_idxs_tmp->cmdlist_idxs), *(cmdlist_idxs_tmp->enable_cmdlist), use_comm_mmbuf);
	if (ret != 0) {
		DPU_FB_ERR("fb%u hisi_add_clear_module_reg_node failed! ret=%d\n", dpufd->index, ret);
		goto err_return;
	}

	if (*(cmdlist_idxs_tmp->enable_cmdlist))
		hisi_cmdlist_flush_cache(dpufd, *(cmdlist_idxs_tmp->cmdlist_idxs));

	dpufb_buf_sync_handle(dpufd, pov_req);

	up(&(dpufd->cmdlist_info->cmdlist_wb_common_sem));

	return 0;

err_return:
	hisi_offline_clear(dpufd, pov_req,
		*(cmdlist_idxs_tmp->enable_cmdlist), *(cmdlist_idxs_tmp->cmdlist_idxs), false);

	hisi_offline_handle_device_power(dpufd, wb_compose_type, false);

	up(&(dpufd->cmdlist_info->cmdlist_wb_common_sem));

	return ret;
}

static void set_dpufd_wb_info(struct dpu_fb_data_type *dpufd, uint32_t cmdlist_idxs, int mctl_idx,
	uint32_t wb_compose_type)
{
	dpufd->wb_info.to_be_continued = DSS_LAYER_PARALLEL_COMPOSE;
	dpufd->wb_info.cmdlist_idxs = cmdlist_idxs;
	dpufd->wb_info.mctl_idx = mctl_idx;
	dpufd->wb_info.wb_compose_type = wb_compose_type;
}

static void hisi_cmdlist_handle(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	uint32_t cmdlist_idxs, int mctl_idx, uint32_t wb_compose_type)
{
	bool parallel_compose_flag = hisi_get_parallel_compose_flag(dpufd, wb_compose_type);

	if (parallel_compose_flag) {
		hisi_cmdlist_exec(dpufd, cmdlist_idxs | dpufd->wb_info.cmdlist_idxs);
		hisi_cmdlist_config_start(dpufd, dpufd->wb_info.mctl_idx,
			dpufd->wb_info.cmdlist_idxs, dpufd->wb_info.wb_compose_type);
		hisi_cmdlist_config_start(dpufd, mctl_idx, cmdlist_idxs, wb_compose_type);
	} else {
		if (pov_req->to_be_continued == DSS_LAYER_SERIAL_COMPOSE) {
			hisi_cmdlist_exec(dpufd, cmdlist_idxs);
			hisi_cmdlist_config_start(dpufd, mctl_idx, cmdlist_idxs, wb_compose_type);
		}
	}
}

static int hisi_offline_play_work_locked(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	int enable_cmdlist, uint32_t use_comm_mmbuf, uint32_t cmdlist_idxs)
{
	int ret;
	bool reset = false;
	int wb_type = hisi_get_wb_type(pov_req);
	int mctl_idx = hisi_get_mctl_idx(pov_req);
	uint32_t wb_compose_type = pov_req->wb_compose_type;
	bool parallel_compose_flag = hisi_get_parallel_compose_flag(dpufd, wb_compose_type);

	dpu_check_and_return(((wb_type < 0) || (wb_type >= WB_TYPE_MAX)), -1, ERR,
		"wb_type = %d is out of the array range", wb_type);
	down(&(dpufd->cmdlist_info->cmdlist_wb_sem[wb_type]));
	dpufd->cmdlist_info->cmdlist_wb_flag[wb_type] = 1;
	dpufd->cmdlist_info->cmdlist_wb_done[wb_type] = 0;

	if (enable_cmdlist) {
		if (use_comm_mmbuf)
			hisi_vactive0_start_wait(dpufd, pov_req, use_comm_mmbuf);

		hisi_cmdlist_handle(dpufd, pov_req, cmdlist_idxs, mctl_idx, wb_compose_type);
	}

	if (pov_req->to_be_continued == DSS_LAYER_PARALLEL_COMPOSE) {
		hisi_offline_play_free_data(pov_req);
		set_dpufd_wb_info(dpufd, cmdlist_idxs, mctl_idx, wb_compose_type);
		up(&(dpufd->cmdlist_info->cmdlist_wb_sem[wb_type]));
		hisi_offline_handle_device_power(dpufd, wb_compose_type, false);
		return 0;
	}

	if (parallel_compose_flag)
		ret = hisi_check_two_wch_wb_finish(dpufd, dpufd->wb_info.cmdlist_idxs,
			dpufd->wb_info.mctl_idx, cmdlist_idxs, mctl_idx);
	else
		ret = hisi_check_wch_wb_finish(dpufd, pov_req, wb_compose_type, cmdlist_idxs, mctl_idx);

	if (ret != 0)
		reset = true;

	if (parallel_compose_flag) {
		hisi_offline_clear(dpufd, pov_req, enable_cmdlist,
			(dpufd->wb_info.cmdlist_idxs | cmdlist_idxs), reset);
		memset(&dpufd->wb_info, 0, sizeof(struct dss_wb_info));
	} else {
		hisi_offline_clear(dpufd, pov_req, enable_cmdlist, cmdlist_idxs, reset);
	}

	hisi_offline_handle_device_power(dpufd, wb_compose_type, false);

	up(&(dpufd->cmdlist_info->cmdlist_wb_sem[wb_type]));

	return ret;
}

int hisi_ov_offline_play(struct dpu_fb_data_type *dpufd, const void __user *argp)
{
	int ret;
	int enable_cmdlist = 0;
	dss_overlay_t *pov_req = NULL;
	struct timeval tv0;
	struct timeval tv1;
	uint32_t timediff;
	uint32_t use_comm_mmbuf = 0;
	uint32_t cmdlist_idxs = 0;
	struct dss_cmdlist_idxs cmdlist_idxs_tmp = { NULL, &cmdlist_idxs, &enable_cmdlist };

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	dpu_check_and_return(!argp, -EINVAL, ERR, "argp is nullptr!\n");

	dpufb_get_timestamp(&tv0);

	ret = hisi_offline_play_config_locked(dpufd, argp, &pov_req, &use_comm_mmbuf, &cmdlist_idxs_tmp);
	if (ret != 0) {
		DPU_FB_INFO("config offline play failed!\n");
		return ret;
	}

	dpufb_get_timestamp(&tv1);
	timediff = dpufb_timestamp_diff(&tv0, &tv1);
	DPU_FB_DEBUG("fb%u config offline play took %u us!\n", dpufd->index, timediff);

	ret = hisi_offline_play_work_locked(dpufd, pov_req, enable_cmdlist, use_comm_mmbuf, cmdlist_idxs);
	if (ret != 0) {
		DPU_FB_ERR("offline play work failed!\n");
		return ret;
	}
	dpufb_get_timestamp(&tv1);
	timediff = dpufb_timestamp_diff(&tv0, &tv1);
	DPU_FB_DEBUG("fb%u offline play work took %u us!\n", dpufd->index, timediff);

	return 0;
}

