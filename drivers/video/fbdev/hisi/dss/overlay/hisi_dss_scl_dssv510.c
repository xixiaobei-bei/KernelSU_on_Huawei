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
#include "hisi_mmbuf_manager.h"

struct wb_scl_info {
	bool en_hscl;
	bool en_vscl;
	bool en_mmp;
	uint32_t h_ratio;
	uint32_t v_ratio;
	uint32_t h_v_order;
	uint32_t acc_hscl;
	uint32_t acc_vscl;
	uint32_t scf_en_vscl;
};

static void wb_scl_rect_set(dss_wb_block_info_t *pblock_info, dss_wb_layer_t *wb_layer,
	dss_rect_t *src_rect, dss_rect_t *dst_rect)
{
	dss_rect_t temp;

	if (pblock_info && (pblock_info->src_rect.w != 0) && (pblock_info->dst_rect.w != 0)) {
		*src_rect = pblock_info->src_rect;
		*dst_rect = pblock_info->dst_rect;
	} else {
		*src_rect = wb_layer->src_rect;
		*dst_rect = wb_layer->dst_rect;

		if (wb_layer->transform ==
			(HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V)) {
			temp.w = dst_rect->w;
			dst_rect->w = dst_rect->h;
			dst_rect->h = temp.w;
		}
	}
}

static int wb_scl_h_ratio_set(dss_wb_block_info_t *pblock_info, struct wb_scl_info *scl_info,
	dss_wb_layer_t *wb_layer, dss_rect_t src_rect, dss_rect_t dst_rect)
{
	if (pblock_info && (pblock_info->h_ratio != 0) && (pblock_info->h_ratio != SCF_INC_FACTOR)) {
		scl_info->h_ratio = pblock_info->h_ratio;
		scl_info->en_hscl = true;
		return 0;
	}

	if ((src_rect.w == dst_rect.w) || (DSS_HEIGHT(dst_rect.w) == 0))
		return 0;

	scl_info->en_hscl = true;

	if ((src_rect.w < SCF_MIN_INPUT) || (dst_rect.w < SCF_MIN_OUTPUT)) {
		DPU_FB_ERR("src_rect.w %d small than 16, or dst_rect.w %d small than 16\n",
			src_rect.w, dst_rect.w);
		return -EINVAL;
	}

	scl_info->h_ratio = (DSS_HEIGHT(src_rect.w) * SCF_INC_FACTOR +
		SCF_INC_FACTOR / 2 - scl_info->acc_hscl) / DSS_HEIGHT(dst_rect.w);

	if ((dst_rect.w > (src_rect.w * SCF_UPSCALE_MAX)) || (src_rect.w > (dst_rect.w * SCF_DOWNSCALE_MAX))) {
		DPU_FB_ERR("width out of range, original_src_rec %d, %d, %d, %d, "
			"src_rect %d, %d, %d, %d, dst_rect %d, %d, %d, %d\n",
			wb_layer->src_rect.x, wb_layer->src_rect.y,
			wb_layer->src_rect.w, wb_layer->src_rect.h,
			src_rect.x, src_rect.y, src_rect.w, src_rect.h,
			dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);

			return -EINVAL;
	}

	return 0;
}

static int wb_scl_v_ratio_set(dss_wb_block_info_t *pblock_info, struct wb_scl_info *scl_info,
	dss_wb_layer_t *wb_layer, dss_rect_t src_rect, dss_rect_t dst_rect)
{
	if ((src_rect.h == dst_rect.h) || (DSS_HEIGHT(dst_rect.h) == 0))
		return 0;

	scl_info->en_vscl = true;
	scl_info->scf_en_vscl = 1;

	scl_info->v_ratio = (DSS_HEIGHT(src_rect.h) * SCF_INC_FACTOR +
		SCF_INC_FACTOR / 2 - scl_info->acc_vscl) / DSS_HEIGHT(dst_rect.h);

	if ((dst_rect.h > (src_rect.h * SCF_UPSCALE_MAX)) || (src_rect.h > (dst_rect.h * SCF_DOWNSCALE_MAX))) {
		DPU_FB_ERR("height out of range, original_src_rec %d, %d, %d, %d, "
			"src_rect %d, %d, %d, %d, dst_rect %d, %d, %d, %d.\n",
			wb_layer->src_rect.x, wb_layer->src_rect.y,
			wb_layer->src_rect.w, wb_layer->src_rect.h,
			src_rect.x, src_rect.y, src_rect.w, src_rect.h,
			dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);
		return -EINVAL;
	}

	return 0;
}

static void wb_scl_param_config(struct dpu_fb_data_type *dpufd, dss_wb_layer_t *wb_layer,
	struct wb_scl_info *scl_info, dss_rect_t src_rect, dss_rect_t dst_rect)
{
	dss_scl_t *scl = NULL;
	bool has_pixel_alpha = false;

	scl = &(dpufd->dss_module.scl[wb_layer->chn_idx]);
	dpufd->dss_module.scl_used[wb_layer->chn_idx] = 1;

	has_pixel_alpha = hal_format_has_alpha(wb_layer->dst.format);

	scl->en_hscl_str = set_bits32(scl->en_hscl_str, 0x0, 1, 0);

	if (scl_info->v_ratio >= 2 * SCF_INC_FACTOR) {
		if (has_pixel_alpha)
			scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x3, 2, 0);
		else
			scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x1, 2, 0);
	} else {
		scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x0, 1, 0);
	}

	if (src_rect.h > dst_rect.h)
		scl_info->scf_en_vscl = 0x3;

	scl_info->en_mmp = 0x1;

	scl->h_v_order = set_bits32(scl->h_v_order, scl_info->h_v_order, 1, 0);
	scl->input_width_height = set_bits32(scl->input_width_height,
		DSS_HEIGHT(src_rect.h), 13, 0);
	scl->input_width_height = set_bits32(scl->input_width_height,
		DSS_WIDTH(src_rect.w), 13, 16);
	scl->output_width_height = set_bits32(scl->output_width_height,
		DSS_HEIGHT(dst_rect.h), 13, 0);
	scl->output_width_height = set_bits32(scl->output_width_height,
		DSS_WIDTH(dst_rect.w), 13, 16);
	scl->en_hscl = set_bits32(scl->en_hscl, (scl_info->en_hscl ? 0x1 : 0x0), 1, 0);
	scl->en_vscl = set_bits32(scl->en_vscl, scl_info->scf_en_vscl, 2, 0);
	scl->acc_hscl = set_bits32(scl->acc_hscl, scl_info->acc_hscl, 31, 0);
	scl->inc_hscl = set_bits32(scl->inc_hscl, scl_info->h_ratio, 24, 0);
	scl->inc_vscl = set_bits32(scl->inc_vscl, scl_info->v_ratio, 24, 0);
	scl->en_mmp = set_bits32(scl->en_mmp, scl_info->en_mmp, 1, 0);
}

int hisi_dss_wb_scl_config(struct dpu_fb_data_type *dpufd,
	dss_wb_layer_t *wb_layer)
{
	dss_rect_t src_rect;
	dss_rect_t dst_rect;
	dss_wb_block_info_t *pblock_info = NULL;
	struct wb_scl_info scl_info = {0};
	int chn_idx;

	if (!dpufd || !wb_layer)
		return -EINVAL;

	chn_idx = wb_layer->chn_idx;
	if (chn_idx != DSS_WCHN_W1)
		return 0;

	pblock_info = &(wb_layer->wb_block_info);

	wb_scl_rect_set(pblock_info, wb_layer, &src_rect, &dst_rect);

	wb_scl_h_ratio_set(pblock_info, &scl_info, wb_layer, src_rect, dst_rect);
	wb_scl_v_ratio_set(pblock_info, &scl_info, wb_layer, src_rect, dst_rect);

	if (!scl_info.en_hscl && !scl_info.en_vscl)
		return 0;

	/* scale down, do hscl first; scale up, do vscl first */
	scl_info.h_v_order = (src_rect.w > dst_rect.w) ? 0 : 1;

	if (pblock_info && (pblock_info->acc_hscl != 0))
		scl_info.acc_hscl = pblock_info->acc_hscl;

	wb_scl_param_config(dpufd, wb_layer, &scl_info, src_rect, dst_rect);

	if (g_debug_ovl_mediacommon_composer)
		DPU_FB_INFO("ch%d, wb_layer_src[%d %d], wb_layer_dst[%d %d], "
			"acc_hscl=%d, h_ratio=%d, transform=%d, w=%d, h=%d, "
			"pblock_info->src_rect[%d:%d:%d:%d], pblock_info->dst_rect[%d:%d:%d:%d]\n",
			wb_layer->chn_idx, wb_layer->src_rect.w, wb_layer->src_rect.h,
			wb_layer->dst_rect.w, wb_layer->dst_rect.h,
			wb_layer->wb_block_info.acc_hscl, pblock_info->h_ratio,
			wb_layer->transform, dst_rect.w, dst_rect.h,
			pblock_info->src_rect.x, pblock_info->src_rect.y,
			pblock_info->src_rect.w, pblock_info->src_rect.h,
			pblock_info->dst_rect.x, pblock_info->dst_rect.y,
			pblock_info->dst_rect.w, pblock_info->dst_rect.h);

	return 0;
}

