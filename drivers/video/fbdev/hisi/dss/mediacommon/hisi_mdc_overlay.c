/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#include "hisi_mdc_dev.h"

static int hisi_mdc_clear_module_reg_node(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	int cmdlist_idxs,
	bool enable_cmdlist)
{
	dpu_check_and_return((cmdlist_idxs < 0), -EINVAL, ERR, "cmdlist_idxs=%d!\n", cmdlist_idxs);
	dpu_check_and_return((cmdlist_idxs >= HISI_DSS_CMDLIST_IDXS_MAX),
		-EINVAL, ERR, "cmdlist_idxs=%d!\n", cmdlist_idxs);

	(void)hisi_dss_module_init(dpufd);
	(void)hisi_dss_prev_module_set_regs(dpufd, pov_req,
			cmdlist_idxs, enable_cmdlist, NULL);

	return 0;
}

static int hisi_mdc_check_user_layer_buffer(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	dss_overlay_block_t *pov_h_block_infos)
{
	dss_layer_t *layer = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	uint32_t i;
	uint32_t j;
	uint32_t layer_num;

	if ((pov_req->ov_block_nums <= 0) ||
		(pov_req->ov_block_nums > HISI_DSS_OV_BLOCK_NUMS)) {
		DPU_FB_ERR("fb%d, invalid ov_block_nums=%d!",
			dpufd->index, pov_req->ov_block_nums);
		return -EINVAL;
	}

	if ((pov_h_block_infos->layer_nums <= 0) ||
		(pov_h_block_infos->layer_nums > OVL_LAYER_NUM_MAX)) {
		DPU_FB_ERR("fb%d, invalid layer_nums=%d!",
			dpufd->index, pov_h_block_infos->layer_nums);
		return -EINVAL;
	}


	for (i = 0; i < pov_req->ov_block_nums; i++) {
		pov_h_block = &(pov_h_block_infos[i]);
		layer_num = pov_h_block->layer_nums;
		for (j = 0; j < layer_num; j++) {
			layer = &(pov_h_block->layer_infos[j]);
			if (!hisi_dss_check_layer_addr_validate(layer)) {
				DPU_FB_ERR("fb%d, invalid layer_idx=%d!",
					dpufd->index, layer->layer_idx);
				return -EINVAL;
			}

		}
	}

	return 0;
}

static void hisi_media_common_clear(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	uint32_t cmdlist_idxs,
	bool reset,
	bool debug)
{
	dss_overlay_block_t *pov_h_block_infos = NULL;

	dpufd->buf_sync_ctrl.refresh = 1;
	dpufb_buf_sync_signal(dpufd);

	dpu_check_and_no_retval(!pov_req, ERR, "pov_req is nullptr!\n");

	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)pov_req->ov_block_infos_ptr;
	if (!pov_h_block_infos) {
		kfree(pov_req);
		pov_req = NULL;
		return;
	}

	if (g_debug_ovl_cmdlist || debug) {
		dump_dss_overlay(dpufd, pov_req);
		hisi_cmdlist_dump_all_node(dpufd, pov_req, cmdlist_idxs);
	}

	if (reset)
		hisi_mdc_cmdlist_config_reset(dpufd, pov_req, cmdlist_idxs);

	hisi_cmdlist_del_node(dpufd, pov_req, cmdlist_idxs);

	if (pov_h_block_infos != NULL) {
		kfree(pov_h_block_infos);
		pov_h_block_infos = NULL;
	}
	kfree(pov_req);
	pov_req = NULL;
	dpufd->pov_req = NULL;
}

static int hisi_mdc_get_data_from_user(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	const void __user *argp)
{
	int ret;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	uint32_t ov_block_size;

	ret = copy_from_user(pov_req, argp, sizeof(*pov_req));
	dpu_check_and_return(ret, -EINVAL, ERR, "copy_from_user failed!\n");

	dpu_check_and_return((pov_req->ov_block_nums <= 0) ||
		(pov_req->ov_block_nums > HISI_DSS_OV_BLOCK_NUMS),
		-EINVAL, ERR, "ov_block_nums=%u!\n", pov_req->ov_block_nums);

	dpu_check_and_return(
		(pov_req->wb_compose_type != DSS_WB_COMPOSE_MEDIACOMMON) &&
		(pov_req->wb_compose_type != DSS_WB_COMPOSE_COPYBIT),
		-EINVAL, ERR, "wb_compose_type=%u!\n", pov_req->wb_compose_type);

	dpu_check_and_return(
		(pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON) &&
		(pov_req->ovl_idx != DSS_OVL2), -EINVAL, ERR, "inval ov idx!\n");

	pov_req->release_fence = -1;
	ov_block_size = pov_req->ov_block_nums * sizeof(dss_overlay_block_t);
	pov_h_block_infos = (dss_overlay_block_t *)kmalloc(ov_block_size, GFP_ATOMIC);
	dpu_check_and_return(!pov_h_block_infos, -EINVAL, ERR, "null ptr!\n");
	memset(pov_h_block_infos, 0, ov_block_size);

	if ((dss_overlay_block_t *)(uintptr_t)pov_req->ov_block_infos_ptr == NULL)
		goto err_out;

	ret = copy_from_user(pov_h_block_infos,
		(dss_overlay_block_t *)(uintptr_t)pov_req->ov_block_infos_ptr,
		ov_block_size);
	if (ret != 0)
		goto err_out;

	ret = hisi_dss_check_userdata(dpufd, pov_req, pov_h_block_infos);
	if (ret != 0)
		goto err_out;

	if (g_enable_ovl_async_composer) {
		ret = hisi_mdc_check_user_layer_buffer(dpufd, pov_req, pov_h_block_infos);
		if (ret != 0)
			goto err_out;
	}

	pov_req->ov_block_infos_ptr = (uint64_t)(uintptr_t)pov_h_block_infos;

	return ret;

err_out:
	kfree(pov_h_block_infos);
	pov_h_block_infos = NULL;
	return -EINVAL;
}

static bool hisi_mdc_check_csc_config_needed(
	dss_overlay_t *pov_req_h_v)
{
	dss_overlay_block_t *pov_h_v_block = NULL;

	pov_h_v_block = (dss_overlay_block_t *)(uintptr_t)(pov_req_h_v->ov_block_infos_ptr);
	/* check whether csc config needed or not */
	if ((pov_h_v_block->layer_nums == 1) && (is_yuv(pov_h_v_block->layer_infos[0].img.format))) {
		if ((pov_req_h_v->wb_layer_nums == 1) && is_yuv(pov_req_h_v->wb_layer_infos[0].dst.format))
			return false;
	}

	return true;
}

static void hisi_mdc_remove_mctl_mutex(
	struct dpu_fb_data_type *dpufd,
	uint32_t cmdlist_idxs)
{
	int i;
	char __iomem *mctl_mutex_base = NULL;
	char __iomem *cmdlist_base = NULL;
	uint32_t cmdlist_idxs_temp;

	if (dpufd->ov_req.wb_compose_type == DSS_WB_COMPOSE_COPYBIT)
		mctl_mutex_base = dpufd->media_common_base + DSS_MCTRL_CTL0_OFFSET;
	else
		mctl_mutex_base = dpufd->media_common_base + DSS_MCTRL_CTL1_OFFSET;

	set_reg(SOC_MDC_CTL_MUTEX_RCH0_ADDR(mctl_mutex_base), 0, 32, 0);
	set_reg(SOC_MDC_CTL_MUTEX_RCH1_ADDR(mctl_mutex_base), 0, 32, 0);
	set_reg(SOC_MDC_CTL_MUTEX_RCH2_ADDR(mctl_mutex_base), 0, 32, 0);
	set_reg(SOC_MDC_CTL_MUTEX_RCH6_ADDR(mctl_mutex_base), 0, 32, 0);

	set_reg(SOC_MDC_CTL_MUTEX_WCH0_ADDR(mctl_mutex_base), 0, 32, 0);
	set_reg(SOC_MDC_CTL_MUTEX_WCH1_ADDR(mctl_mutex_base), 0, 32, 0);

	cmdlist_base = dpufd->media_common_base + DSS_CMDLIST_OFFSET;
	cmdlist_idxs_temp = cmdlist_idxs;
	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1)
			set_reg(SOC_MDC_CMDLIST_CH_CTRL_ADDR(cmdlist_base, i), 0x6, 3, 2);

		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}
}

static int hisi_mdc_data_init_and_check(
	struct dpu_fb_data_type *dpufd,
	const void __user *argp)
{
	int ret;
	dss_overlay_t *pov_req = NULL;
	struct list_head lock_list;

	dpu_check_and_return(!dpufd, -1, ERR, "dpufd is nullptr!\n");
	dpu_check_and_return(!argp, -1, ERR, "argp is nullptr!\n");
	INIT_LIST_HEAD(&lock_list);

	dpufd->pov_req = NULL;
	pov_req = (dss_overlay_t *)kmalloc(sizeof(*pov_req), GFP_ATOMIC);
	dpu_check_and_return(!pov_req, -1, ERR, "pov_req is nullptr!\n");
	dpufb_dss_overlay_info_init(pov_req);

	ret = hisi_mdc_get_data_from_user(dpufd, pov_req, argp);
	if (ret != 0) {
		DPU_FB_ERR("hisi_mdc_get_data_from_user failed\n");
		kfree(pov_req);
		pov_req = NULL;
		return ret;
	}

	dpufb_offline_layerbuf_lock(dpufd, pov_req, &lock_list);
	dpufb_layerbuf_flush(dpufd, &lock_list);

	/* save pre overlay construction */
	memcpy(&dpufd->ov_req, pov_req, sizeof(*pov_req));
	dpufd->pov_req = pov_req;

	return ret;
}

static int hisi_mdc_get_block_info(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	dss_overlay_block_t *pov_h_block,
	int *out_block_num,
	dss_rect_t *out_block_rect)
{
	int ret;
	bool has_wb_scl = false;
	dss_wb_layer_t *wb_layer4block = NULL;

	wb_layer4block = &(pov_req->wb_layer_infos[0]);
	if (wb_layer4block->need_cap & CAP_SCL)
		has_wb_scl = true;

	ret = get_ov_block_rect(pov_req, pov_h_block, out_block_num, dpufd->ov_block_rects, has_wb_scl);
	dpu_check_and_return(ret != 0, -EINVAL, ERR, "get ov block faild!\n");
	dpu_check_and_return(*out_block_num <= 0, -EINVAL, ERR, "block=%d", *out_block_num);
	dpu_check_and_return(*out_block_num >= HISI_DSS_CMDLIST_BLOCK_MAX,
		-EINVAL, ERR, "overflow, block=%d", *out_block_num);

	ret = get_wb_layer_block_rect(wb_layer4block, has_wb_scl, out_block_rect);
	dpu_check_and_return(ret != 0, -EINVAL, ERR, "get wb block faild!\n");

	return 0;
}

static int hisi_mdc_layer_ov_composer(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	dss_layer_t *layer,
	dss_rect_t *wb_ov_block_rect,
	bool *has_base)
{
	int ret;
	dss_rect_ltrb_t clip_rect = {0, 0, 0, 0};
	dss_rect_t aligned_rect = {0, 0, 0, 0};
	dss_wb_layer_t *wb_layer4block = NULL;
	dss_overlay_block_t *pov_h_v_block = NULL;
	dss_rect_t wb_dst_rect = {0, 0, 0, 0};
	dss_overlay_t *pov_req_h_v = &(dpufd->ov_req);
	struct hisi_ov_compose_rect ov_compose_rect = { &wb_dst_rect, wb_ov_block_rect, &clip_rect, &aligned_rect };
	struct hisi_ov_compose_flag ov_compose_flag = { false, *has_base, false, true };

	pov_h_v_block = (dss_overlay_block_t *)(uintptr_t)(pov_req_h_v->ov_block_infos_ptr);
	wb_layer4block = &(pov_req->wb_layer_infos[0]);
	ov_compose_flag.csc_needed = hisi_mdc_check_csc_config_needed(pov_req_h_v);

	ov_compose_rect.wb_dst_rect = &wb_layer4block->dst_rect;

	ret = hisi_ov_compose_handler(dpufd, pov_h_v_block, layer, &ov_compose_rect, &ov_compose_flag);
	if (ret != 0)
		DPU_FB_ERR("hisi_ov_compose_handler failed!");

	return ret;
}

static int hisi_mdc_offline_oneblock_config(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	dss_rect_t *wb_ov_block_rect,
	int h_block_index,
	bool last_block)
{
	int ret = 0;
	uint32_t i;
	bool has_base = false;
	dss_overlay_block_t *pov_h_v_block = NULL;
	dss_layer_t *layer = NULL;
	dss_wb_layer_t *wb_layer = NULL;
	dss_overlay_t *pov_req_h_v = &(dpufd->ov_req);
	struct ov_module_set_regs_flag ov_module_flag = { true, false, 0, 0 };
	struct hisi_ov_compose_flag ov_flag = { false, false, true, true };

	pov_h_v_block = (dss_overlay_block_t *)(uintptr_t)(pov_req_h_v->ov_block_infos_ptr);
	hisi_dss_aif_handler(dpufd, pov_req_h_v, pov_h_v_block);

	if (pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON) {
		ret = hisi_dss_ovl_base_config(dpufd, pov_req_h_v, pov_h_v_block, wb_ov_block_rect, h_block_index);
		dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc base_config faild!\n");
	}

	for (i = 0; i < pov_h_v_block->layer_nums; i++) {
		layer = &(pov_h_v_block->layer_infos[i]);
		ret = hisi_mdc_layer_ov_composer(dpufd, pov_req, layer, wb_ov_block_rect, &has_base);
		dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc ov composer faild!\n");
	}

	if (h_block_index == 0) {
		ov_flag.csc_needed = hisi_mdc_check_csc_config_needed(pov_req_h_v);
		for (i = 0; i < pov_req_h_v->wb_layer_nums; i++) {
			wb_layer = &(pov_req_h_v->wb_layer_infos[i]);
			ret = hisi_wb_compose_handler(dpufd, wb_layer, wb_ov_block_rect, last_block, ov_flag);
			dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc wb composer faild!\n");

			ret = hisi_wb_ch_module_set_regs(dpufd, 0, wb_layer, ov_flag.enable_cmdlist);
			if (ret != 0)
				return -EINVAL;
		}
	}

	if (pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON) {
		ret = hisi_dss_mctl_ov_config(dpufd, pov_req, pov_req_h_v->ovl_idx, has_base, false);
		dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc mctl faild!\n");

		ret = hisi_dss_ov_module_set_regs(dpufd, pov_req, pov_req_h_v->ovl_idx, ov_module_flag);
		dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc ov_module faild!\n");
	}
	return ret;
}

static int hisi_mdc_offline_block_handle(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	uint32_t h_block_index)
{
	int ret;
	int block_num = 0;
	int k = 0;
	dss_overlay_t *pov_req_h_v = NULL;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	bool last_block = false;
	dss_rect_t wb_ov_block_rect = {0, 0, 0, 0};
	dss_rect_t wb_layer_block_rect = {0, 0, 0, 0};

	pov_req_h_v = &(dpufd->ov_req);
	pov_req_h_v->ov_block_infos_ptr = (uintptr_t)(&(dpufd->ov_block_infos)); /*lint !e545*/
	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
	pov_h_block = &(pov_h_block_infos[h_block_index]);
	ret = hisi_mdc_get_block_info(dpufd, pov_req, pov_h_block, &block_num, &wb_layer_block_rect);
	dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc get_block faild!\n");

	hisi_dss_module_init(dpufd);
	for (; k < block_num; k++) {
		ret = get_block_layers(pov_req, pov_h_block, *(dpufd->ov_block_rects[k]), pov_req_h_v, block_num);
		dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc get_block_layers faild!\n");
		ret = rect_across_rect(*dpufd->ov_block_rects[k],
			wb_layer_block_rect, &wb_ov_block_rect);
		if (ret == 0) {
			DPU_FB_ERR("no cross! ov_block_rects[%d]{%d %d %d %d}, wb src_rect{%d %d %d %d}\n",
				k,
				dpufd->ov_block_rects[k]->x, dpufd->ov_block_rects[k]->y,
				dpufd->ov_block_rects[k]->w, dpufd->ov_block_rects[k]->h,
				wb_layer_block_rect.x, wb_layer_block_rect.y,
				wb_layer_block_rect.w, wb_layer_block_rect.h);
			continue;
		}

		last_block = (k == (block_num - 1)) ? true : false;
		ret = hisi_mdc_offline_oneblock_config(dpufd, pov_req, &wb_ov_block_rect, h_block_index, last_block);
		dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc offline_oneblock faild!\n");
		if (!last_block)
			hisi_dss_module_init(dpufd);
	}

	return ret;
}

static void hisi_mdc_cmdlist_start(
	struct dpu_fb_data_type *dpufd,
	char __iomem *mctl_base,
	uint32_t mctl_idx,
	uint32_t cmdlist_idxs)
{
	char __iomem *cmdlist_base = NULL;
	struct dss_cmdlist_node *cmdlist_node = NULL;
	uint32_t list_addr = 0;
	uint32_t i;
	uint32_t cmdlist_idxs_temp;
	uint32_t temp = 0;
	uint32_t status_temp;
	uint32_t ints_temp;

	cmdlist_base = dpufd->media_common_base + DSS_CMDLIST_OFFSET;
	cmdlist_idxs_temp = cmdlist_idxs;
	dsb(sy);

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			status_temp = inp32(SOC_MDC_CMDLIST_CH_STATUS_ADDR(cmdlist_base, i));
			ints_temp = inp32(SOC_MDC_CMDLIST_CH_INTS_ADDR(cmdlist_base, i));

			cmdlist_node = list_first_entry(&(dpufd->media_common_cmdlist_data->cmdlist_head_temp[i]),
				dss_cmdlist_node_t, list_node);
			list_addr = cmdlist_node->header_phys;
			if (g_debug_ovl_cmdlist)
				DPU_FB_INFO("list_addr:0x%x, i=%u, ints_temp=0x%x\n", list_addr, i, ints_temp);

			temp |= (1 << i);
			outp32(SOC_MDC_CMDLIST_ADDR_MASK_EN_ADDR(cmdlist_base), BIT(i));
			set_reg(SOC_MDC_CMDLIST_CH_CTRL_ADDR(cmdlist_base, i), mctl_idx, 3, 2);
			set_reg(SOC_MDC_CMDLIST_CH_CTRL_ADDR(cmdlist_base, i), 0x0, 1, 6);
			set_reg(SOC_MDC_CMDLIST_CH_STAAD_ADDR(cmdlist_base, i), list_addr, 32, 0);
			set_reg(SOC_MDC_CMDLIST_CH_CTRL_ADDR(cmdlist_base, i), 0x1, 1, 0);

			if (((status_temp & 0xF) == 0x0) || ((ints_temp & 0x2) == 0x2))
				set_reg(SOC_MDC_CMDLIST_SWRST_ADDR(cmdlist_base), 0x1, 1, i);
			else
				DPU_FB_INFO("i=%u, status_temp=0x%x, ints_temp=0x%x\n", i, status_temp, ints_temp);
		}
		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}

	outp32(SOC_MDC_CMDLIST_ADDR_MASK_DIS_ADDR(cmdlist_base), temp);

	set_reg(SOC_MDC_CTL_ST_SEL_ADDR(mctl_base), 0x1, 1, 0);
	set_reg(SOC_MDC_CTL_SW_ST_ADDR(mctl_base), 0x1, 1, 0);
}

static int hisi_mdc_cmdlist_config_start(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	uint32_t *cmdlist_idxs)
{
	int ret = 0;
	uint32_t h_block_index = 0;

	dpufd->set_reg = hisi_cmdlist_set_reg;
	dpufd->cmdlist_data = dpufd->media_common_cmdlist_data;
	dpu_check_and_return(!dpufd->cmdlist_data, -EINVAL, ERR, "nullptr!\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "pov_req is nullptr!\n");

	if (pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON)
		ret = hisi_cmdlist_get_cmdlist_idxs(pov_req, NULL, cmdlist_idxs);
	dpu_check_and_return(ret != 0, -EINVAL, ERR, "get_cmdlist failed!\n");

	for (; h_block_index < pov_req->ov_block_nums; h_block_index++) {
		ret = hisi_mdc_offline_block_handle(dpufd, pov_req, h_block_index);
		dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc_offline_block failed!\n");
	}
	ret = hisi_mdc_clear_module_reg_node(dpufd, pov_req, *cmdlist_idxs, true);
	dpu_check_and_return(ret != 0, -EINVAL, ERR, "mdc_clear_module failed!\n");

	hisi_cmdlist_flush_cache(dpufd, *cmdlist_idxs);

	/* wait for buffer ready */
	dpufb_buf_sync_wait_async(&(dpufd->buf_sync_ctrl));

	dpu_check_and_return((!dpufd->media_common_info), -EINVAL, ERR, "media_common_info is nullptr!\n");

	dpufd->media_common_info->mdc_flag = 1;
	dpufd->media_common_info->mdc_done = 0;

	hisi_cmdlist_exec(dpufd, *cmdlist_idxs);

	if (pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON)
		hisi_mdc_cmdlist_start(dpufd,
			dpufd->media_common_base + DSS_MCTRL_CTL1_OFFSET,
			DSS_MCTL1, *cmdlist_idxs);
	else
		hisi_mdc_cmdlist_start(dpufd,
			dpufd->media_common_base + DSS_MCTRL_CTL0_OFFSET,
			DSS_MCTL0, *cmdlist_idxs);

	return ret;
}

static void hisi_mdc_wait_and_sync_compose(
	struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req,
	uint32_t cmdlist_idxs)
{
	int ret;
	int times = 0;
	int ret_cmdlist_state;
	struct timeval tv0;
	struct timeval tv1;
	uint32_t timeout_interval;

	dpu_check_and_no_retval((!dpufd->media_common_info), ERR, "media_common_info is nullptr!\n");

	dpufb_get_timestamp(&tv0);
	timeout_interval = (g_fpga_flag == 0) ?
		DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC : DSS_COMPOSER_TIMEOUT_THRESHOLD_FPGA;
	do {
		ret = wait_event_interruptible_timeout(dpufd->media_common_info->mdc_wq, /*lint !e578*/
			(dpufd->media_common_info->mdc_done == 1),
			msecs_to_jiffies(timeout_interval)); /*lint !e666*/
		if (ret != -ERESTARTSYS)
			break;
		mdelay(10);
	} while (++times < 50);  /* wait 500ms */

	dpufb_get_timestamp(&tv1);

	dpufd->media_common_info->mdc_flag = 0;
	ret_cmdlist_state = hisi_cmdlist_check_cmdlist_state(dpufd, cmdlist_idxs);
	if ((ret <= 0) || (ret_cmdlist_state < 0)) {
		DPU_FB_ERR("compose timeout, GLB_CPU_OFF_INTS=0x%x, ret=%d, ret_rch_state=%d, diff=%u usecs!\n",
			inp32(dpufd->media_common_base + GLB_CPU_OFF_INTS),
			ret, ret_cmdlist_state, dpufb_timestamp_diff(&tv0, &tv1));
		hisi_media_common_clear(dpufd, pov_req, cmdlist_idxs, true, true);
	} else {
		/* remove mctl ch */
		hisi_mdc_remove_mctl_mutex(dpufd, cmdlist_idxs);
		hisi_media_common_clear(dpufd, pov_req, cmdlist_idxs, false, false);
	}
}

void hisi_mdc_preoverlay_async_play(struct kthread_work *work)
{
	int ret;
	uint32_t timediff;
	struct timeval tv0;
	struct timeval tv1;
	dss_overlay_t *pov_req = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	uint32_t cmdlist_idxs = HISI_DSS_MEDIACOMMON_CMDLIST_IDXS;

	dpufd = container_of(work, struct dpu_fb_data_type, preoverlay_work);
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");

	dpufb_get_timestamp(&tv0);
	pov_req = dpufd->pov_req;

	ret = hisi_mdc_cmdlist_config_start(dpufd, pov_req, &cmdlist_idxs);
	if (ret != 0) {
		DPU_FB_INFO("mediacommon cmdlist=0x%x config fail\n", cmdlist_idxs);
		hisi_media_common_clear(dpufd, pov_req, cmdlist_idxs, true, true);
		mutex_unlock(&(dpufd->work_lock));
		return;
	}
	hisi_mdc_wait_and_sync_compose(dpufd, pov_req, cmdlist_idxs);
	mutex_unlock(&(dpufd->work_lock));

	dpufb_get_timestamp(&tv1);
	timediff = dpufb_timestamp_diff(&tv0, &tv1);
	DPU_FB_INFO("mdc pre overlay took %u us!\n", timediff);

	hisi_mdc_power_off(dpufd);
}

int hisi_mdc_preoverlay_sync_play(struct dpu_fb_data_type *dpufd)
{
	int ret;
	uint32_t timediff;
	struct timeval tv0, tv1;
	dss_overlay_t *pov_req = NULL;
	uint32_t cmdlist_idxs = HISI_DSS_MEDIACOMMON_CMDLIST_IDXS;

	dpufb_get_timestamp(&tv0);
	pov_req = dpufd->pov_req;

	ret = hisi_mdc_cmdlist_config_start(dpufd, pov_req, &cmdlist_idxs);
	if (ret != 0) {
		DPU_FB_INFO("mediacommon cmdlist=0x%x config fail\n", cmdlist_idxs);
		hisi_media_common_clear(dpufd, pov_req, cmdlist_idxs, true, true);
		return ret;
	}
	hisi_mdc_wait_and_sync_compose(dpufd, pov_req, cmdlist_idxs);

	dpufb_get_timestamp(&tv1);
	timediff = dpufb_timestamp_diff(&tv0, &tv1);
	DPU_FB_INFO("mdc pre overlay took %u us!\n", timediff);

	return ret;
}

int hisi_ov_media_common_play(
	struct dpu_fb_data_type *dpufd,
	const void __user *argp)
{
	int ret;
	uint32_t timediff;
	struct timeval tv0;
	struct timeval tv1;

	dpu_check_and_return((!dpufd || !argp), -EINVAL, ERR, "nullptr!\n");
	dpufb_get_timestamp(&tv0);
	mutex_lock(&(dpufd->work_lock));

	ret = hisi_mdc_data_init_and_check(dpufd, argp);
	if (ret != 0) {
		DPU_FB_INFO("mediacommon alloc or check pov_req fail\n");
		mutex_unlock(&(dpufd->work_lock));
		return -EINVAL;
	}
	dpufb_offline_create_fence(dpufd);

	if (!IS_ERR_OR_NULL(dpufd->preoverlay_thread) && g_enable_ovl_async_composer) {
		DPU_FB_DEBUG("mdc pre overlay exec async play!\n");
		kthread_queue_work(&(dpufd->preoverlay_worker), &(dpufd->preoverlay_work));
	} else {
		dpufb_buf_sync_close_fence(&dpufd->ov_req.release_fence, &dpufd->ov_req.retire_fence);
		ret = hisi_mdc_preoverlay_sync_play(dpufd);
		mutex_unlock(&(dpufd->work_lock));
	}

	if (copy_to_user((struct dss_overlay_t __user *)argp,
		&(dpufd->ov_req), sizeof(dss_overlay_t))) {
		DPU_FB_ERR("fb%d, copy_to_user failed\n", dpufd->index);
		dpufb_buf_sync_close_fence(&dpufd->ov_req.release_fence, &dpufd->ov_req.retire_fence);
		ret = -EFAULT;
	}
	dpufd->ov_req.release_fence = -1;

	dpufb_get_timestamp(&tv1);
	timediff = dpufb_timestamp_diff(&tv0, &tv1);
	DPU_FB_INFO("mdc pre overlay config took %u us!\n", timediff);

	return ret;
}

