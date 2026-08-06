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

#include "hisi_overlay_utils.h"
#include "../hisi_display_effect.h"
#include "../hisi_dpe_utils.h"
#include "../hisi_ovl_online_wb.h"
#include "../hisi_mmbuf_manager.h"
#include "../hisi_spr_dsc.h"
#include "../hisi_frame_rate_ctrl.h"

#define ARSR2P_CHN_IDX DSS_RCHN_V0

static dss_ovl_alpha_t g_ovl_alpha[DSS_BLEND_MAX] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  /* DSS_BLEND_CLEAR */
	{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },  /* DSS_BLEND_SRC */
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  /* DSS_BLEND_DST */
	{ 3, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 },  /* DSS_BLEND_SRC_OVER_DST */
	{ 1, 0, 0, 0, 0, 0, 3, 0, 0, 1, 0 },  /* DSS_BLEND_DST_OVER_SRC */
	{ 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0 },  /* DSS_BLEND_SRC_IN_DST */
	{ 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  /* DSS_BLEND_DST_IN_SRC */
	{ 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 0 },  /* DSS_BLEND_SRC_OUT_DST */
	{ 3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },  /* DSS_BLEND_DST_OUT_SRC */
	{ 3, 0, 0, 0, 1, 0, 3, 0, 0, 0, 0 },  /* DSS_BLEND_SRC_ATOP_DST */
	{ 3, 0, 0, 0, 0, 0, 3, 0, 0, 1, 0 },  /* DSS_BLEND_DST_ATOP_SRC */
	{ 3, 0, 0, 0, 1, 0, 3, 0, 0, 1, 0 },  /* DSS_BLEND_SRC_XOR_DST */
	{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },  /* DSS_BLEND_SRC_ADD_DST */
	{ 3, 0, 0, 0, 1, 0, 3, 0, 0, 0, 1 },  /* DSS_BLEND_FIX_OVER */
	{ 3, 0, 0, 0, 1, 0, 3, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER0 */
	{ 3, 0, 0, 0, 1, 1, 3, 1, 0, 0, 1 },  /* DSS_BLEND_FIX_PER1 */
	{ 2, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0 },  /* DSS_BLEND_FIX_PER2 */
	{ 1, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER3 */
	{ 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER4 */
	{ 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  /* DSS_BLEND_FIX_PER5 */
	{ 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER6 */
	{ 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  /* DSS_BLEND_FIX_PER7 */
	{ 2, 2, 0, 0, 0, 0, 3, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER8 */
	{ 3, 2, 0, 0, 0, 0, 2, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER9 */
	{ 2, 2, 0, 0, 0, 0, 2, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER10 */
	{ 3, 2, 0, 0, 0, 0, 3, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER11 */
	{ 2, 1, 0, 0, 0, 0, 3, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER12 DSS_BLEND_SRC_OVER_DST */
	{ 2, 1, 0, 0, 0, 0, 3, 1, 0, 0, 1 },  /* DSS_BLEND_FIX_PER13 DSS_BLEND_FIX_OVER */
	{ 0, 0, 0, 0, 0, 1, 3, 0, 1, 0, 0 },  /* DSS_BLEND_FIX_PER14 BASE_FF */
	{ 2, 1, 0, 0, 0, 1, 3, 2, 0, 0, 0 },  /* DSS_BLEND_FIX_PER15 DSS_BLEND_SRC_OVER_DST */
	{ 2, 1, 0, 0, 0, 1, 3, 1, 0, 0, 1 },  /* DSS_BLEND_FIX_PER16 DSS_BLEND_FIX_OVER */
	{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },  /* DSS_BLEND_FIX_PER17 DSS_BLEND_SRC */
};

static uint32_t get_ovl_blending_mode(dss_layer_t *layer)
{
	uint32_t blend_mode;
	bool has_per_pixel_alpha = false;

	has_per_pixel_alpha = hal_format_has_alpha(layer->img.format);

	if (layer->blending == HISI_FB_BLENDING_PREMULT) {
		if (has_per_pixel_alpha)
			blend_mode = (layer->glb_alpha < 0xFF) ? DSS_BLEND_FIX_PER12 : DSS_BLEND_SRC_OVER_DST;
		else
			blend_mode = (layer->glb_alpha < 0xFF) ? DSS_BLEND_FIX_PER8 : DSS_BLEND_SRC;
	} else if (layer->blending == HISI_FB_BLENDING_COVERAGE) {
		if (has_per_pixel_alpha)
			blend_mode = (layer->glb_alpha < 0xFF) ? DSS_BLEND_FIX_PER13 : DSS_BLEND_FIX_OVER;
		else
			blend_mode = (layer->glb_alpha < 0xFF) ? DSS_BLEND_FIX_PER8 : DSS_BLEND_SRC;
	} else {
		if (has_per_pixel_alpha)
			blend_mode = DSS_BLEND_SRC;
		else
			blend_mode = DSS_BLEND_FIX_PER17;
	}

	if (layer->is_cld_layer == 1)
		blend_mode = DSS_BLEND_FIX_OVER;

	if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer)
		DPU_FB_INFO("layer_idx[%d], blending=%d, fomat=%d, has_per_pixel_alpha=%d, blend_mode=%d.\n",
			layer->layer_idx, layer->blending, layer->img.format, has_per_pixel_alpha, blend_mode);

	return blend_mode;
}

static bool is_dirty_rect_empty(dss_overlay_t *pov_req)
{
	return ((!pov_req) || (pov_req->dirty_rect.x == 0 && pov_req->dirty_rect.y == 0 &&
		pov_req->dirty_rect.w == 0 && pov_req->dirty_rect.h == 0));
}

static bool is_rog_scale_empty(dss_overlay_t *pov_req, struct dpu_panel_info *pinfo)
{
	return (!pinfo->cascadeic_support ||
		(pov_req->rog_width == pinfo->xres && pov_req->rog_height == pinfo->yres) ||
		(pov_req->rog_width == 0 && pov_req->rog_height == 0));
}

int get_img_size_info(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_rect_t *wb_ov_block_rect, struct img_size *img_info)
{
	struct dpu_panel_info *pinfo = NULL;

	dpu_check_and_return((!dpufd || !img_info), -EINVAL, ERR, "dpufd or img_info is NULL\n");

	pinfo = &(dpufd->panel_info);

	if (wb_ov_block_rect) {
		img_info->img_width = wb_ov_block_rect->w;
		img_info->img_height = wb_ov_block_rect->h;
		return 0;
	}

	if (is_dirty_rect_empty(pov_req)) {
		img_info->img_width = get_panel_xres(dpufd);
		img_info->img_height = get_panel_yres(dpufd);
	} else {
		if (is_rog_scale_empty(pov_req, pinfo)) {  /* no rog scale */
			img_info->img_width = pov_req->dirty_rect.w;
			img_info->img_height = pov_req->dirty_rect.h;
		} else {
			img_info->img_width = get_panel_xres(dpufd);
			img_info->img_height = get_panel_yres(dpufd);
		}
	}

	if (pinfo->cascadeic_support && pov_req)
		DPU_FB_DEBUG("dirty_rect: %d,%d / rog size: %d,%d / get panel res: %d,%d\n",
			pov_req->dirty_rect.w, pov_req->dirty_rect.h, pov_req->rog_width, pov_req->rog_height,
			get_panel_xres(dpufd), get_panel_yres(dpufd));

	return 0;
}

static void set_ovl_single_layer_struct(dss_ovl_t *ovl, dss_layer_t *layer,
	struct img_size img_info, int layer_idx, int pov_h_block_idx)
{
	if ((layer_idx >= pov_h_block_idx) && layer_idx < (sizeof(ovl->ovl_layer) / sizeof(ovl->ovl_layer[0]))) {
		ovl->ovl_layer[layer_idx].layer_pos = set_bits32(ovl->ovl_layer[layer_idx].layer_pos, 0, 15, 0);
		ovl->ovl_layer[layer_idx].layer_pos = set_bits32(ovl->ovl_layer[layer_idx].layer_pos,
			img_info.img_height, 15, 16);

		ovl->ovl_layer[layer_idx].layer_size = set_bits32(ovl->ovl_layer[layer_idx].layer_size,
			img_info.img_width, 15, 0);
		ovl->ovl_layer[layer_idx].layer_size = set_bits32(ovl->ovl_layer[layer_idx].layer_size,
			img_info.img_height + 1, 15, 16);
		ovl->ovl_layer[layer_idx].layer_cfg = set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 0x1, 1, 0);

		if (layer->need_cap & (CAP_DIM | CAP_PURE_COLOR)) {
			ovl->ovl_layer[layer_idx].layer_pattern =
				set_bits32(ovl->ovl_layer[layer_idx].layer_pattern, layer->color, 32, 0);
			ovl->ovl_layer[layer_idx].layer_cfg =
				set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 1, 1, 0);
			ovl->ovl_layer[layer_idx].layer_cfg =
				set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 1, 1, 8);
		} else {
			ovl->ovl_layer[layer_idx].layer_pattern =
				set_bits32(ovl->ovl_layer[layer_idx].layer_pattern, 0, 32, 0);
			ovl->ovl_layer[layer_idx].layer_cfg =
				set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 1, 1, 0);
			ovl->ovl_layer[layer_idx].layer_cfg =
				set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 0, 1, 8);
		}

		ovl->ovl_layer_used[layer_idx] = 1;
	}

}

static void set_ovl_layer_struct(dss_overlay_t *pov_req, dss_ovl_t *ovl, struct img_size img_info, int ov_h_block_idx)
{
	uint32_t i;
	uint32_t m;
	int layer_idx;
	bool has_base = false;
	int pov_h_block_idx = 0;
	dss_layer_t *layer = NULL;
	dss_overlay_block_t *pov_h_block_tmp = NULL;
	dss_overlay_block_t *pov_h_block_infos_tmp = NULL;

	pov_h_block_infos_tmp = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
	for (m = ov_h_block_idx; m < (pov_req->ov_block_nums); m++) {
		pov_h_block_tmp = &(pov_h_block_infos_tmp[m]);
		has_base = false;

		for (i = 0; i < (pov_h_block_tmp->layer_nums); i++) {
			layer = &(pov_h_block_tmp->layer_infos[i]);

			if (layer->need_cap & CAP_BASE) {
				DPU_FB_INFO("layer%d is base, i=%d!\n", layer->layer_idx, i);
				has_base = true;
				continue;
			}

			layer_idx = i;
			if (has_base)
				layer_idx = i - 1;

			set_ovl_single_layer_struct(ovl, layer, img_info, layer_idx, pov_h_block_idx);

			pov_h_block_idx = layer_idx + 1;
		}
	}
}

static int calculate_block_size(dss_overlay_t *pov_req, dss_overlay_block_t *pov_h_block,
	dss_rect_t *wb_ov_block_rect)
{
	int temp;
	int block_size;

	if (wb_ov_block_rect) {
		if ((pov_req->wb_layer_infos[0].transform & HISI_FB_TRANSFORM_ROT_90) ||
			(pov_req->wb_layer_infos[1].transform & HISI_FB_TRANSFORM_ROT_90)) {
			block_size = DSS_HEIGHT(wb_ov_block_rect->h);
		} else {
			temp = pov_h_block->ov_block_rect.y + DSS_HEIGHT(pov_h_block->ov_block_rect.h) -
				wb_ov_block_rect->y;
			if (temp >= wb_ov_block_rect->h)
				block_size = DSS_HEIGHT(wb_ov_block_rect->h);
			else
				block_size = temp;
		}
	} else {
		block_size = pov_h_block->ov_block_rect.y + DSS_HEIGHT(pov_h_block->ov_block_rect.h);
	}

	return block_size;
}

void set_ovl_struct(struct dpu_fb_data_type *dpufd, dss_ovl_t *ovl, struct img_size img_info, int block_size)
{
	uint32_t ovl_bg_color;

	dpu_check_and_no_retval((!dpufd || !ovl), ERR, "dpufd or ovl is NULL\n");

	ovl->ovl_size = set_bits32(ovl->ovl_size, DSS_WIDTH(img_info.img_width), 15, 0);
	ovl->ovl_size = set_bits32(ovl->ovl_size, DSS_HEIGHT(img_info.img_height), 15, 16);

#ifdef SUPPORT_FULL_CHN_10BIT_COLOR

	ovl_bg_color = 0x00000000;  /* full channel 10bit display black color  */
#else
	ovl_bg_color = 0xFF000000;  /* full channel 8bit display black color  */
#endif

	if ((dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) &&
		dpufd->de_info.is_maintaining) {
		ovl_bg_color = 0xFFFFFFFF;
		DPU_FB_INFO("dual panel support display effect");
	}

	ovl->ovl_bg_color = set_bits32(ovl->ovl_bg_color, ovl_bg_color, 32, 0);

#ifdef SUPPORT_FULL_CHN_10BIT_COLOR
	ovl->ovl_bg_color_alpha = set_bits32(ovl->ovl_bg_color_alpha, 0x3FF, 32, 0);
	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		ovl->ovl_bg_color_alpha = set_bits32(ovl->ovl_bg_color_alpha, 0x0, 32, 0);
#endif

	ovl->ovl_dst_startpos = set_bits32(ovl->ovl_dst_startpos, 0x0, 32, 0);
	ovl->ovl_dst_endpos = set_bits32(ovl->ovl_dst_endpos, DSS_WIDTH(img_info.img_width), 15, 0);
	ovl->ovl_dst_endpos = set_bits32(ovl->ovl_dst_endpos, DSS_HEIGHT(img_info.img_height), 15, 16);
	ovl->ovl_gcfg = set_bits32(ovl->ovl_gcfg, 0x1, 1, 0);
	ovl->ovl_gcfg = set_bits32(ovl->ovl_gcfg, 0x1, 1, 16);
	ovl->ovl_block_size = set_bits32(ovl->ovl_block_size, block_size, 15, 16);
}

int hisi_dss_ovl_base_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	  dss_overlay_block_t *pov_h_block, dss_rect_t *wb_ov_block_rect, int ov_h_block_idx)
{
	int ret;
	dss_ovl_t *ovl = NULL;
	struct img_size img_info = {0};
	int block_size = 0x7FFF;  /* default max reg valve */

	dpu_check_and_return((!dpufd || !pov_req), -EINVAL, ERR, "dpufd or pov_req is nullptr!\n");
	if ((pov_req->ovl_idx < DSS_OVL0) || (pov_req->ovl_idx >= DSS_OVL_IDX_MAX)) {
		DPU_FB_ERR("ovl_idx is invalid\n");
		return -EINVAL;
	}

	if (pov_req->wb_layer_infos[0].chn_idx == DSS_WCHN_W2)  /* chicago copybit no ovl */
		return 0;

	ovl = &(dpufd->dss_module.ov[pov_req->ovl_idx]);
	dpufd->dss_module.ov_used[pov_req->ovl_idx] = 1;

	ret = get_img_size_info(dpufd, pov_req, wb_ov_block_rect, &img_info);
	dpu_check_and_return((ret != 0), -ret, ERR, "get_img_size_info error!\n");

	if ((pov_h_block != NULL) && (pov_req != NULL) && (pov_req->ov_block_nums != 0)) {
		if (pov_req->ov_block_nums > 1)
			set_ovl_layer_struct(pov_req, ovl, img_info, ov_h_block_idx);

		block_size = calculate_block_size(pov_req, pov_h_block, wb_ov_block_rect);
	}

	if (wb_ov_block_rect == NULL &&
		is_arsr_post_need_padding(pov_req, &(dpufd->panel_info), ARSR1P_INC_FACTOR)) {
		img_info.img_width += ARSR_POST_COLUMN_PADDING_NUM;
		DPU_FB_DEBUG("img_width = %d, padding %d column\n", img_info.img_width, ARSR_POST_COLUMN_PADDING_NUM);
	}

	set_ovl_struct(dpufd, ovl, img_info, block_size);

	return ret;
}

static void adjust_dst_rect_x(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req, dss_rect_t *pdst_rect)
{
	uint32_t dummy_x_left = dpufd->panel_info.dummy_pixel_x_left;

	if ((dummy_x_left != 0) && (pov_req->rog_width > 0)) {
		dummy_x_left = dummy_x_left * pov_req->rog_width /
			(dpufd->panel_info.xres - dpufd->panel_info.dummy_pixel_num);

		pdst_rect->x = pdst_rect->x + dummy_x_left;
	}
}

static void config_ovl_struct(dss_layer_t *layer, dss_ovl_t *ovl)
{
	uint32_t color_rgb;
#ifdef SUPPORT_FULL_CHN_10BIT_COLOR
	uint32_t color_alpha;
#endif

	color_rgb = layer->color;
#ifdef SUPPORT_FULL_CHN_10BIT_COLOR
	color_alpha = (layer->color >> 24) * OVL_PATTERN_RATIO;
	color_rgb = (((layer->color >> 16) & 0xff) << 20) |
		(((layer->color >> 8) & 0xff) << 10) | (layer->color & 0xff);
#endif
	ovl->ovl_bg_color = set_bits32(ovl->ovl_bg_color, color_rgb, 32, 0);
#ifdef SUPPORT_FULL_CHN_10BIT_COLOR
	ovl->ovl_bg_color_alpha = set_bits32(ovl->ovl_bg_color_alpha, color_alpha, 32, 0);
#endif
	ovl->ovl_gcfg = set_bits32(ovl->ovl_gcfg, 0x1, 1, 16);
}

static int get_ovl_blend_mode(dss_overlay_t *pov_req, dss_layer_t *layer, int *blend_mode)
{
	*blend_mode = get_ovl_blending_mode(layer);
	if ((*blend_mode < 0) || (*blend_mode >= DSS_BLEND_MAX)) {
		DPU_FB_ERR("blend_mode = %d is out of range!\n", *blend_mode);
		return -EINVAL;
	}
	if (pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON)
		*blend_mode = DSS_BLEND_FIX_OVER;

	return 0;
}

static void config_dst_rect_struct(dss_layer_t *layer, dss_rect_t *dst_rect)
{
	if ((layer->chn_idx == ARSR2P_CHN_IDX) && layer->block_info.arsr2p_left_clip) {
		dst_rect->x = layer->dst_rect.x + layer->block_info.arsr2p_left_clip;
		dst_rect->y = layer->dst_rect.y;
		dst_rect->w = layer->dst_rect.w - layer->block_info.arsr2p_left_clip;
		dst_rect->h = layer->dst_rect.h;
	} else {
		*dst_rect = layer->dst_rect;
	}
}

static void config_ovl_layer_pos_and_size(dss_overlay_t *pov_req, dss_rect_t *wb_ov_block_rect,
		dss_ovl_t *ovl, int layer_idx, dss_rect_t dst_rect)
{
	dss_rect_t wb_ov_rect;

	wb_ov_rect.x = pov_req->wb_ov_rect.x + wb_ov_block_rect->x;
	wb_ov_rect.y = pov_req->wb_ov_rect.y;

	ovl->ovl_layer[layer_idx].layer_pos = set_bits32(ovl->ovl_layer[layer_idx].layer_pos,
		(dst_rect.x - wb_ov_rect.x), 15, 0);
	ovl->ovl_layer[layer_idx].layer_pos = set_bits32(ovl->ovl_layer[layer_idx].layer_pos,
		(dst_rect.y - wb_ov_rect.y), 15, 16);

	ovl->ovl_layer[layer_idx].layer_size = set_bits32(ovl->ovl_layer[layer_idx].layer_size,
		(dst_rect.x - wb_ov_rect.x + DSS_WIDTH(dst_rect.w)), 15, 0);
	ovl->ovl_layer[layer_idx].layer_size = set_bits32(ovl->ovl_layer[layer_idx].layer_size,
		(dst_rect.y - wb_ov_rect.y + DSS_HEIGHT(dst_rect.h)), 15, 16);
}

static void config_ovl_layer_def_pos_and_size(dss_overlay_t *pov_req, dss_ovl_t *ovl,
	int layer_idx, dss_rect_t dst_rect)
{
	ovl->ovl_layer[layer_idx].layer_pos = set_bits32(ovl->ovl_layer[layer_idx].layer_pos,
		dst_rect.x, 15, 0);
	ovl->ovl_layer[layer_idx].layer_pos = set_bits32(ovl->ovl_layer[layer_idx].layer_pos,
		dst_rect.y, 15, 16);

	ovl->ovl_layer[layer_idx].layer_size = set_bits32(ovl->ovl_layer[layer_idx].layer_size,
		DSS_WIDTH(dst_rect.x + dst_rect.w), 15, 0);
	ovl->ovl_layer[layer_idx].layer_size = set_bits32(ovl->ovl_layer[layer_idx].layer_size,
		DSS_HEIGHT(dst_rect.y + dst_rect.h), 15, 16);
}

static void config_ovl_layer_alpha(dss_layer_t *layer, dss_ovl_t *ovl, int layer_idx, int blend_mode)
{
#ifdef SUPPORT_FULL_CHN_10BIT_COLOR
	uint32_t glb_alpha;
	uint32_t color_alpha;

	glb_alpha = layer->glb_alpha * OVL_PATTERN_RATIO;
	color_alpha = (layer->color >> 24) * OVL_PATTERN_RATIO;

	ovl->ovl_layer[layer_idx].layer_alpha = set_bits32(ovl->ovl_layer[layer_idx].layer_alpha,
		((g_ovl_alpha[blend_mode].fix_mode << 8) |
		(g_ovl_alpha[blend_mode].dst_pmode << 9) | (g_ovl_alpha[blend_mode].alpha_offdst << 10) |
		(g_ovl_alpha[blend_mode].dst_gmode << 12) | (g_ovl_alpha[blend_mode].dst_amode << 14) |
		(g_ovl_alpha[blend_mode].alpha_smode << 24) |
		(g_ovl_alpha[blend_mode].src_pmode << 25) | (g_ovl_alpha[blend_mode].src_lmode << 26) |
		(g_ovl_alpha[blend_mode].alpha_offdst << 27) | (g_ovl_alpha[blend_mode].src_gmode << 28) |
		(g_ovl_alpha[blend_mode].src_amode << 30)), 32, 0);
	ovl->ovl_layer[layer_idx].layer_alpha_a = set_bits32(ovl->ovl_layer[layer_idx].layer_alpha_a,
		glb_alpha | (glb_alpha << 16), 32, 0);
	if (layer->need_cap & (CAP_DIM | CAP_PURE_COLOR))
		ovl->ovl_layer[layer_idx].layer_pattern_alpha =
			set_bits32(ovl->ovl_layer[layer_idx].layer_pattern_alpha, color_alpha, 32, 0);

#else
	ovl->ovl_layer[layer_idx].layer_alpha = set_bits32(ovl->ovl_layer[layer_idx].layer_alpha,
		((layer->glb_alpha << 0) |
		(g_ovl_alpha[blend_mode].fix_mode << 8) |
		(g_ovl_alpha[blend_mode].dst_pmode << 9) | (g_ovl_alpha[blend_mode].alpha_offdst << 10) |
		(g_ovl_alpha[blend_mode].dst_gmode << 12) | (g_ovl_alpha[blend_mode].dst_amode << 14) |
		(layer->glb_alpha << 16) |
		(g_ovl_alpha[blend_mode].alpha_smode << 24) |
		(g_ovl_alpha[blend_mode].src_pmode << 25) | (g_ovl_alpha[blend_mode].src_lmode << 26) |
		(g_ovl_alpha[blend_mode].alpha_offdst << 27) | (g_ovl_alpha[blend_mode].src_gmode << 28) |
		(g_ovl_alpha[blend_mode].src_amode << 30)), 32, 0);
#endif
}

static void config_ovl_layer_pattern_and_cfg(dss_layer_t *layer, dss_ovl_t *ovl, int layer_idx)
{
	uint32_t color_rgb;

#ifdef SUPPORT_FULL_CHN_10BIT_COLOR
	color_rgb = (((layer->color >> 16) & 0xff) << 20) |
		(((layer->color >> 8) & 0xff) << 10) | (layer->color & 0xff);
#else
	color_rgb = layer->color;
#endif

	if (layer->need_cap & (CAP_DIM | CAP_PURE_COLOR)) {
		ovl->ovl_layer[layer_idx].layer_pattern =
			set_bits32(ovl->ovl_layer[layer_idx].layer_pattern, color_rgb, 32, 0);
		ovl->ovl_layer[layer_idx].layer_cfg =
			set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 0x1, 1, 0);
		ovl->ovl_layer[layer_idx].layer_cfg =
			set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 0x1, 1, 8);
	} else {
		ovl->ovl_layer[layer_idx].layer_pattern =
			set_bits32(ovl->ovl_layer[layer_idx].layer_pattern, 0x0, 32, 0);
		ovl->ovl_layer[layer_idx].layer_cfg =
			set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 0x1, 1, 0);
		ovl->ovl_layer[layer_idx].layer_cfg =
			set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 0x0, 1, 8);
	}
}

static void config_ovl_layer_pspos_and_pepos(dss_ovl_t *ovl, int layer_idx, dss_rect_t dst_rect)
{
	ovl->ovl_layer_pos[layer_idx].layer_pspos =
		set_bits32(ovl->ovl_layer_pos[layer_idx].layer_pspos, dst_rect.x, 15, 0);
	ovl->ovl_layer_pos[layer_idx].layer_pspos =
		set_bits32(ovl->ovl_layer_pos[layer_idx].layer_pspos, dst_rect.y, 15, 16);
	ovl->ovl_layer_pos[layer_idx].layer_pepos =
		set_bits32(ovl->ovl_layer_pos[layer_idx].layer_pepos,
		DSS_WIDTH(dst_rect.x + dst_rect.w), 15, 0);
	ovl->ovl_layer_pos[layer_idx].layer_pepos =
		set_bits32(ovl->ovl_layer_pos[layer_idx].layer_pepos,
		DSS_HEIGHT(dst_rect.y + dst_rect.h), 15, 16);
}

int hisi_dss_ovl_layer_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_layer_t *layer, dss_rect_t *wb_ov_block_rect, bool has_base)
{
	dss_ovl_t *ovl = NULL;
	int ovl_idx;
	int layer_idx;
	int blend_mode = 0;
	dss_rect_t dst_rect;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	dpu_check_and_return(!layer, -EINVAL, ERR, "layer is nullptr!\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "pov_req is nullptr!\n");

	ovl_idx = dpufd->ov_req.ovl_idx;
	layer_idx = layer->layer_idx;
	dpu_check_and_return(((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)),
		-EINVAL, ERR, "ovl_idx = %d is out of range!\n", ovl_idx);

	if ((dpufd->index == MEDIACOMMON_PANEL_IDX) &&
		(pov_req->wb_compose_type == DSS_WB_COMPOSE_COPYBIT))
		return 0;

	ovl = &(dpufd->dss_module.ov[ovl_idx]);
	dpufd->dss_module.ov_used[ovl_idx] = 1;

	if (layer->need_cap & CAP_BASE) {
		config_ovl_struct(layer, ovl);
		return 0;
	}

	if ((layer->glb_alpha) > 0xFF) {
		DPU_FB_ERR("layer's glb_alpha(0x%x) is out of range!\n", layer->glb_alpha);
		layer->glb_alpha = 0xFF;
	}

	if (get_ovl_blend_mode(pov_req, layer, &blend_mode) < 0) {
		DPU_FB_ERR("get_ovl_blend_mode failed\n");
		return -EINVAL;
	}

	if (has_base) {
		layer_idx -= 1;
		dpu_check_and_return((layer_idx < 0), -EINVAL, ERR, "layer_idx[%d] is out of range!\n", layer_idx);
	}

	ovl->ovl_layer_used[layer_idx] = 1;

	config_dst_rect_struct(layer, &dst_rect);

	if (wb_ov_block_rect) {
		config_ovl_layer_pos_and_size(pov_req, wb_ov_block_rect, ovl, layer_idx, dst_rect);
	} else {
		adjust_dst_rect_x(dpufd, pov_req, &dst_rect);

		if (is_arsr_post_need_padding(pov_req, &(dpufd->panel_info), ARSR1P_INC_FACTOR))
			dst_rect.x += ARSR_POST_COLUMN_PADDING_NUM;

		config_ovl_layer_def_pos_and_size(pov_req, ovl, layer_idx, dst_rect);
	}

	config_ovl_layer_alpha(layer, ovl, layer_idx, blend_mode);

	config_ovl_layer_pattern_and_cfg(layer, ovl, layer_idx);

	config_ovl_layer_pspos_and_pepos(ovl, layer_idx, dst_rect);

	return 0;
}
