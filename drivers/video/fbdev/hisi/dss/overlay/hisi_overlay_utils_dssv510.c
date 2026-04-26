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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winteger-overflow"

#include "hisi_overlay_utils.h"
#include "../hisi_mmbuf_manager.h"

struct timediff_info {
	struct timeval tv0;
	struct timeval tv1;
};

static uint32_t g_vactive_timeout_count;

uint32_t g_fpga_flag;
void *g_smmu_rwerraddr_virt;

#if defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V345) || defined(CONFIG_HISI_FB_V346)
void hisi_dss_qos_on(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	outp32(dpufd->noc_dss_base + 0xc, 0x1);
	outp32(dpufd->noc_dss_base + 0x10, 0x1000);
	outp32(dpufd->noc_dss_base + 0x14, 0x20);
	outp32(dpufd->noc_dss_base + 0x18, 0x1);

	outp32(dpufd->noc_dss_base + 0x10c, 0x1);
	outp32(dpufd->noc_dss_base + 0x110, 0x1000);
	outp32(dpufd->noc_dss_base + 0x114, 0x20);
	outp32(dpufd->noc_dss_base + 0x118, 0x1);

	outp32(dpufd->noc_dss_base + 0x18c, 0x1);
	outp32(dpufd->noc_dss_base + 0x190, 0x1000);
	outp32(dpufd->noc_dss_base + 0x194, 0x20);
	outp32(dpufd->noc_dss_base + 0x198, 0x1);
}
#else
void hisi_dss_qos_on(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd)
		return;
}
#endif

static int hisi_dss_check_wblayer_buff(
	struct dpu_fb_data_type *dpufd, dss_wb_layer_t *wb_layer)
{
	if (hisi_dss_check_addr_validate(&wb_layer->dst))
		return 0;

	if (wb_layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR))
		return 0;

	return -EINVAL;
}

static int hisi_dss_check_wblayer_rect(
	struct dpu_fb_data_type *dpufd, dss_wb_layer_t *wb_layer)
{
	if (wb_layer->src_rect.x < 0 || wb_layer->src_rect.y < 0 ||
		wb_layer->src_rect.w <= 0 || wb_layer->src_rect.h <= 0) {
		DPU_FB_ERR("fb%d, src_rect %d, %d, %d, %d is out of range!\n",
			dpufd->index, wb_layer->src_rect.x, wb_layer->src_rect.y,
			wb_layer->src_rect.w, wb_layer->src_rect.h);
		return -EINVAL;
	}

	if (wb_layer->dst_rect.x < 0 || wb_layer->dst_rect.y < 0 ||
		wb_layer->dst_rect.w <= 0 || wb_layer->dst_rect.h <= 0 ||
		wb_layer->dst_rect.w > wb_layer->dst.width ||  /*lint !e574*/
		wb_layer->dst_rect.h > wb_layer->dst.height) {  /*lint !e574*/
		DPU_FB_ERR("fb%d, dst_rect %d, %d, %d, %d, dst %d, %d is out of range!\n",
			dpufd->index, wb_layer->dst_rect.x,
			wb_layer->dst_rect.y, wb_layer->dst_rect.w,
			wb_layer->dst_rect.h, wb_layer->dst.width, wb_layer->dst.height);
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_check_userdata_base(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, dss_overlay_block_t *pov_h_block_infos)
{
	if ((pov_req->ov_block_nums <= 0) ||
		(pov_req->ov_block_nums > HISI_DSS_OV_BLOCK_NUMS)) {
		DPU_FB_ERR("fb%d, invalid ov_block_nums=%d!\n",
			dpufd->index, pov_req->ov_block_nums);
		return -EINVAL;
	}

	if ((pov_h_block_infos->layer_nums <= 0) ||
		(pov_h_block_infos->layer_nums > OVL_LAYER_NUM_MAX)) {
		DPU_FB_ERR("fb%d, invalid layer_nums=%d!\n",
			dpufd->index, pov_h_block_infos->layer_nums);
		return -EINVAL;
	}

	if ((pov_req->ovl_idx < 0) || (pov_req->ovl_idx >= DSS_OVL_IDX_MAX)) {
		DPU_FB_ERR("fb%d, invalid ovl_idx=%d!\n", dpufd->index, pov_req->ovl_idx);
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_check_userdata_dst(dss_wb_layer_t *wb_layer,
	uint32_t index)
{
	if (wb_layer->need_cap & CAP_AFBCE) {
		if ((wb_layer->dst.afbc_header_stride == 0) ||
			(wb_layer->dst.afbc_payload_stride == 0)) {
			DPU_FB_ERR("fb%d, afbc_header_stride=%d, afbc_payload_stride=%d is invalid!\n",
				index, wb_layer->dst.afbc_header_stride, wb_layer->dst.afbc_payload_stride);
			return -EINVAL;
		}
	}

	if (wb_layer->need_cap & CAP_HFBCE) {
		if ((wb_layer->dst.hfbc_header_stride0 == 0) ||
			(wb_layer->dst.hfbc_payload_stride0 == 0) ||
			(wb_layer->dst.hfbc_header_stride1 == 0) ||
			(wb_layer->dst.hfbc_payload_stride1 == 0) ||
			(wb_layer->chn_idx != DSS_WCHN_W1)) {
			DPU_FB_ERR("fb%d, hfbc_header_stride0=%d, hfbc_payload_stride0=%d,"
				"hfbc_header_stride1=%d, hfbc_payload_stride1=%d is "
				"invalid or wchn_idx=%d no support hfbce!\n",
				index, wb_layer->dst.hfbc_header_stride0,
				wb_layer->dst.hfbc_payload_stride0,
				wb_layer->dst.hfbc_header_stride1,
				wb_layer->dst.hfbc_payload_stride1,
				wb_layer->chn_idx);
			return -EINVAL;
		}
	}

	return 0;
}

static int hisi_dss_check_dirty_rect(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req)
{
	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (dpufd->panel_info.dirty_region_updt_support) {
			if (pov_req->dirty_rect.x < 0 || pov_req->dirty_rect.y < 0 ||
				pov_req->dirty_rect.w < 0 || pov_req->dirty_rect.h < 0) {
				DPU_FB_ERR("dirty_rect %d, %d, %d, %d is out of range!\n",
					pov_req->dirty_rect.x, pov_req->dirty_rect.y,
					pov_req->dirty_rect.w, pov_req->dirty_rect.h);
				return -EINVAL;
			}
		}
	}

	return 0;
}

static int hisi_dss_check_ov_info(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req)
{
	if ((pov_req->wb_layer_nums <= 0) ||
		(pov_req->wb_layer_nums > MAX_DSS_DST_NUM)) {
		DPU_FB_ERR("fb%d, invalid wb_layer_nums=%d!\n",
			dpufd->index, pov_req->wb_layer_nums);
		return -EINVAL;
	}

	if (pov_req->wb_ov_rect.x < 0 || pov_req->wb_ov_rect.y < 0) {
		DPU_FB_ERR("wb_ov_rect %d, %d is out of range!\n",
			pov_req->wb_ov_rect.x, pov_req->wb_ov_rect.y);
		return -EINVAL;
	}

	if (pov_req->wb_compose_type >= DSS_WB_COMPOSE_TYPE_MAX) {
		DPU_FB_ERR("wb_compose_type=%u is invalid!\n", pov_req->wb_compose_type);
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_check_layer_afbc_info(struct dpu_fb_data_type *dpufd,
	dss_wb_layer_t *wb_layer)
{
	if (!(wb_layer->need_cap & (CAP_HFBCE | CAP_HEBCE))) {
		if (is_yuv_semiplanar(wb_layer->dst.format) ||
			is_yuv_plane(wb_layer->dst.format)) {
			if ((wb_layer->dst.stride_plane1 == 0) || (wb_layer->dst.offset_plane1 == 0)) {
				DPU_FB_ERR("fb%d, stride_plane1=%d, offset_plane1=%d is invalid!\n",
					dpufd->index, wb_layer->dst.stride_plane1,
					wb_layer->dst.offset_plane1);
				return -EINVAL;
			}
		}

		if (is_yuv_plane(wb_layer->dst.format)) {
			if ((wb_layer->dst.stride_plane2 == 0) || (wb_layer->dst.offset_plane2 == 0)) {
				DPU_FB_ERR("fb%d, stride_plane2=%d, offset_plane2=%d is invalid!\n",
					dpufd->index, wb_layer->dst.stride_plane2,
					wb_layer->dst.offset_plane2);
				return -EINVAL;
			}
		}
	}

	if (wb_layer->dst.afbc_scramble_mode >= DSS_AFBC_SCRAMBLE_MODE_MAX) {
		DPU_FB_ERR("fb%d, afbc_scramble_mode=%d is invalid!\n",
			dpufd->index, wb_layer->dst.afbc_scramble_mode);
		return -EINVAL;
	}

	return 0;
}

static bool is_wb_layer_size_info_empty(dss_wb_layer_t *wb_layer)
{
	return ((wb_layer->dst.bpp == 0) || (wb_layer->dst.width == 0) ||
		(wb_layer->dst.height == 0) || ((wb_layer->dst.stride == 0) &&
		(!(wb_layer->need_cap & (CAP_HFBCE | CAP_HEBCE)))));
}

static int hisi_dss_check_wb_layer_info(struct dpu_fb_data_type *dpufd,
	dss_wb_layer_t *wb_layer)
{
	if (wb_layer->chn_idx != DSS_WCHN_W2) {
		if (wb_layer->chn_idx < DSS_WCHN_W0 || wb_layer->chn_idx > DSS_WCHN_W1) {
			DPU_FB_ERR("fb%d, wchn_idx=%d is invalid!\n", dpufd->index, wb_layer->chn_idx);
			return -EINVAL;
		}
	}

	if (wb_layer->dst.format >= HISI_FB_PIXEL_FORMAT_MAX) {
		DPU_FB_ERR("fb%d, format=%d is invalid!\n", dpufd->index, wb_layer->dst.format);
		return -EINVAL;
	}

	if (is_wb_layer_size_info_empty(wb_layer)) {
		DPU_FB_ERR("fb%d, bpp=%d, width=%d, height=%d, stride=%d is invalid!\n",
			dpufd->index, wb_layer->dst.bpp, wb_layer->dst.width,
			wb_layer->dst.height, wb_layer->dst.stride);
		return -EINVAL;
	}

	if (hisi_dss_check_wblayer_buff(dpufd, wb_layer)) {
		DPU_FB_ERR("fb%d, failed to check_wblayer_buff!\n", dpufd->index);
		return -EINVAL;
	}

	if (hisi_dss_check_layer_afbc_info(dpufd, wb_layer) < 0)
		return -EINVAL;

	if (hisi_dss_check_userdata_dst(wb_layer, dpufd->index) < 0)
		return -EINVAL;

	if (wb_layer->dst.csc_mode >= DSS_CSC_MOD_MAX) {
		DPU_FB_ERR("fb%d, csc_mode=%d is invalid!\n", dpufd->index, wb_layer->dst.csc_mode);
		return -EINVAL;
	}

	if (hisi_dss_check_wblayer_rect(dpufd, wb_layer)) {
		DPU_FB_ERR("fb%d, failed to check_wblayer_rect!\n", dpufd->index);
		return -EINVAL;
	}

	return 0;
}

int hisi_dss_check_userdata(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, dss_overlay_block_t *pov_h_block_infos)
{
	uint32_t i;
	dss_wb_layer_t *wb_layer = NULL;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "invalid dpufd!\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "invalid pov_req!\n");
	dpu_check_and_return(!pov_h_block_infos, -EINVAL, ERR, "invalid pov_h_block_infos!\n");

	if (hisi_dss_check_userdata_base(dpufd, pov_req, pov_h_block_infos) < 0)
		return -EINVAL;
	if (hisi_dss_check_dirty_rect(dpufd, pov_req) < 0)
		return -EINVAL;

	if (dpufd->index == AUXILIARY_PANEL_IDX) {
		if (pov_req->wb_enable != 1) {
			DPU_FB_ERR("pov_req->wb_enable=%u is invalid!\n", pov_req->wb_enable);
			return -EINVAL;
		}

		if (hisi_dss_check_ov_info(dpufd, pov_req) < 0)
			return -EINVAL;

		for (i = 0; i < pov_req->wb_layer_nums; i++) {
			wb_layer = &(pov_req->wb_layer_infos[i]);
			if (hisi_dss_check_wb_layer_info(dpufd, wb_layer) < 0)
				return -EINVAL;
		}
	} else if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		if (pov_req->wb_layer_nums > MAX_DSS_DST_NUM) {
			DPU_FB_ERR("fb%d, invalid wb_layer_nums=%d!\n",
				dpufd->index, pov_req->wb_layer_nums);
			return -EINVAL;
		}
		for (i = 0; i < pov_req->wb_layer_nums; i++) {
			wb_layer = &(pov_req->wb_layer_infos[i]);
			if (hisi_dss_check_wblayer_buff(dpufd, wb_layer)) {
				DPU_FB_ERR("fb%d, failed to check_wblayer_buff!\n", dpufd->index);
				return -EINVAL;
			}
		}
	}

	return 0;
}

static int hisi_dss_check_layer_buff(
	struct dpu_fb_data_type *dpufd, dss_layer_t *layer)
{

	if (hisi_dss_check_layer_addr_validate(layer))
		return 0;

	if (layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR))
		return 0;

	return -EINVAL;
}

static int hisi_dss_check_layer_rect(dss_layer_t *layer)
{
	if (layer->src_rect.x < 0 || layer->src_rect.y < 0 ||
		layer->src_rect.w <= 0 || layer->src_rect.h <= 0) {
		DPU_FB_ERR("src_rect %d, %d, %d, %d is out of range!\n",
			layer->src_rect.x, layer->src_rect.y,
			layer->src_rect.w, layer->src_rect.h);
		return -EINVAL;
	}

	if (layer->src_rect_mask.x < 0 || layer->src_rect_mask.y < 0 ||
		layer->src_rect_mask.w < 0 || layer->src_rect_mask.h < 0) {
		DPU_FB_ERR("src_rect_mask %d, %d, %d, %d is out of range!\n",
			layer->src_rect_mask.x, layer->src_rect_mask.y,
			layer->src_rect_mask.w, layer->src_rect_mask.h);
		return -EINVAL;
	}

	if (layer->dst_rect.x < 0 || layer->dst_rect.y < 0 ||
		layer->dst_rect.w <= 0 || layer->dst_rect.h <= 0) {
		DPU_FB_ERR("dst_rect %d, %d, %d, %d is out of range!\n",
			layer->dst_rect.x, layer->dst_rect.y,
			layer->dst_rect.w, layer->dst_rect.h);
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_check_layer_par_need_cap(
	dss_layer_t *layer, uint32_t index)
{
	if (layer->need_cap & CAP_AFBCD) {
		if ((layer->img.afbc_header_stride == 0) ||
			(layer->img.afbc_payload_stride == 0) ||
			(layer->img.mmbuf_size == 0)) {
			DPU_FB_ERR("fb%d, afbc_header_stride=%d, afbc_payload_stride=%d, mmbuf_size=%d is invalid!",
				index, layer->img.afbc_header_stride,
				layer->img.afbc_payload_stride, layer->img.mmbuf_size);
			return -EINVAL;
		}
	}

	if (layer->need_cap & CAP_HFBCD) {
		if ((layer->img.hfbc_header_stride0 == 0) ||
			(layer->img.hfbc_payload_stride0 == 0) ||
			(layer->img.hfbc_header_stride1 == 0) ||
			(layer->img.hfbc_payload_stride1 == 0) ||
			((layer->chn_idx != DSS_RCHN_V0) &&
			(layer->chn_idx != DSS_RCHN_V1))) {
			DPU_FB_ERR("fb%d, hfbc_header_stride0=%d, hfbc_payload_stride0=%d,"
				"hfbc_header_stride1=%d, hfbc_payload_stride1=%d is invalid or "
				"chn_idx=%d no support hfbcd!\n",
				index, layer->img.hfbc_header_stride0,
				layer->img.hfbc_payload_stride0,
				layer->img.hfbc_header_stride1,
				layer->img.hfbc_payload_stride1, layer->chn_idx);
			return -EINVAL;
		}
	}

	return 0;
}

static int hisi_dss_check_layer_ch_valid(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer)
{
	if (dpufd->index == AUXILIARY_PANEL_IDX) {
		if (layer->chn_idx != DSS_RCHN_V2) {
			if (layer->chn_idx < 0 || layer->chn_idx >= DSS_WCHN_W0) {
				DPU_FB_ERR("fb%d, rchn_idx=%d is invalid!\n",
					dpufd->index, layer->chn_idx);
				return -EINVAL;
			}
		}

		if (layer->chn_idx == DSS_RCHN_D2) {
			DPU_FB_ERR("fb%d, chn_idx[%d] does not used by offline play!\n",
				dpufd->index, layer->chn_idx);
			return -EINVAL;
		}
	} else {
		if (layer->chn_idx < 0 || layer->chn_idx >= DSS_CHN_MAX_DEFINE) {
			DPU_FB_ERR("fb%d, rchn_idx=%d is invalid!\n",
				dpufd->index, layer->chn_idx);
			return -EINVAL;
		}
	}

	return 0;
}

static bool is_layer_basic_size_info_empty(dss_layer_t *layer)
{
	return ((layer->img.bpp == 0) || (layer->img.width == 0) ||
		(layer->img.height == 0) || ((layer->img.stride == 0) &&
		(!(layer->need_cap & (CAP_HFBCD | CAP_HEBCD)))));
}

static int hisi_dss_check_layer_basic_param_valid(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer)
{
	dpu_check_and_return((layer->blending < 0 || layer->blending >= HISI_FB_BLENDING_MAX), -EINVAL, ERR,
		"fb%d, blending=%d is invalid!\n", dpufd->index, layer->blending);
	dpu_check_and_return((layer->img.format >= HISI_FB_PIXEL_FORMAT_MAX), -EINVAL, ERR,
		"fb%d, format=%d is invalid!\n", dpufd->index, layer->img.format);

	if (is_layer_basic_size_info_empty(layer)) {
		DPU_FB_ERR("fb%d, bpp=%d, width=%d, height=%d, stride=%d is invalid!\n",
			dpufd->index, layer->img.bpp, layer->img.width, layer->img.height, layer->img.stride);
		return -EINVAL;
	}

	if (hisi_dss_check_layer_buff(dpufd, layer)) {
		DPU_FB_ERR("fb%d, failed to check_layer_buff!\n", dpufd->index);
		return -EINVAL;
	}

	if (!(layer->need_cap & (CAP_HFBCD | CAP_HEBCD))) {
		if (is_yuv_semiplanar(layer->img.format) || is_yuv_plane(layer->img.format)) {
			if ((layer->img.stride_plane1 == 0) || (layer->img.offset_plane1 == 0)) {
				DPU_FB_ERR("fb%d, stride_plane1=%d, offset_plane1=%d is invalid!\n",
					dpufd->index, layer->img.stride_plane1, layer->img.offset_plane1);
				return -EINVAL;
			}
		}

		if (is_yuv_plane(layer->img.format)) {
			if ((layer->img.stride_plane2 == 0) || (layer->img.offset_plane2 == 0)) {
				DPU_FB_ERR("fb%d, stride_plane2=%d, offset_plane2=%d is invalid!\n",
					dpufd->index, layer->img.stride_plane2, layer->img.offset_plane2);
				return -EINVAL;
			}
		}
	}

	return 0;
}

static int hisi_dss_check_layer_rot_cap(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer)
{
#ifdef SUPPORT_AFBCD_ROT
	u32 cap_online_rot = CAP_HFBCD | CAP_HEBCD | CAP_AFBCD;
#else
	u32 cap_online_rot = CAP_HFBCD;
#endif

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		if (((layer->chn_idx == DSS_RCHN_V0) ||
			(layer->chn_idx == DSS_RCHN_V1)) &&
			(layer->need_cap & cap_online_rot)) {
			DPU_FB_DEBUG("fb%d, ch%d,need_cap=%d, need rot!\n!",
				dpufd->index, layer->chn_idx, layer->need_cap);
		} else {
			DPU_FB_ERR("fb%d, ch%d is not support need_cap=%d, transform=%d!\n",
				dpufd->index, layer->chn_idx, layer->need_cap, layer->transform);
			return -EINVAL;
		}
	}

	return 0;
}

int hisi_dss_check_layer_par(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer)
{
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");
	dpu_check_and_return(!layer, -EINVAL, ERR, "layer is NULL, return!\n");

	if (layer->layer_idx < 0 || layer->layer_idx >= OVL_LAYER_NUM_MAX) {
		DPU_FB_ERR("fb%d, layer_idx=%d is invalid!\n", dpufd->index, layer->layer_idx);
		return -EINVAL;
	}

	if (layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR))
		return 0;

	if (hisi_dss_check_layer_ch_valid(dpufd, layer) < 0)
		return -EINVAL;

	if (hisi_dss_check_layer_basic_param_valid(dpufd, layer) < 0)
		return -EINVAL;

	if (hisi_dss_check_layer_par_need_cap(layer, dpufd->index) < 0)
		return -EINVAL;

	if (hisi_dss_check_layer_rot_cap(dpufd, layer) < 0)
		return -EINVAL;

	if (layer->img.csc_mode >= DSS_CSC_MOD_MAX && !(csc_need_p3_process(layer->img.csc_mode))) {
		DPU_FB_ERR("fb%d, csc_mode=%d is invalid!\n", dpufd->index, layer->img.csc_mode);
		return -EINVAL;
	}

	if (layer->img.afbc_scramble_mode >= DSS_AFBC_SCRAMBLE_MODE_MAX) {
		DPU_FB_ERR("fb%d, afbc_scramble_mode=%d is invalid!\n",
			dpufd->index, layer->img.afbc_scramble_mode);
		return -EINVAL;
	}

	if ((layer->layer_idx != 0) && (layer->need_cap & CAP_BASE)) {
		DPU_FB_ERR("fb%d, layer%d is not base!\n",
			dpufd->index, layer->layer_idx);
		return -EINVAL;
	}

	if (hisi_dss_check_layer_rect(layer)) {
		DPU_FB_ERR("fb%d, failed to check_layer_rect!\n", dpufd->index);
		return -EINVAL;
	}

	return 0;
}

/*******************************************************************************
 * DSS disreset
 */
void dpufb_dss_disreset(struct dpu_fb_data_type *dpufd)
{
	(void)dpufd;
}

static int hisi_dss_dpp_gamma_para_set_reg(struct dpu_fb_data_type *dpufd, uint8_t last_gamma_type)
{
	uint32_t *gamma_lut_r = NULL;
	uint32_t *gamma_lut_g = NULL;
	uint32_t *gamma_lut_b = NULL;
	struct dpu_panel_info *pinfo = &(dpufd->panel_info);
	char __iomem *gamma_base = dpufd->dss_base + DSS_DPP_GAMA_OFFSET;
	char __iomem *gamma_lut_base = dpufd->dss_base + DSS_DPP_GAMA_LUT_OFFSET;
	uint32_t i;
	uint32_t index;
	uint32_t gama_lut_sel;

	if (last_gamma_type == 1) {
		/* set gamma cinema parameter */
		if (pinfo->cinema_gamma_lut_table_len > 0 && pinfo->cinema_gamma_lut_table_R &&
			pinfo->cinema_gamma_lut_table_G && pinfo->cinema_gamma_lut_table_B) {
			gamma_lut_r = pinfo->cinema_gamma_lut_table_R;
			gamma_lut_g = pinfo->cinema_gamma_lut_table_G;
			gamma_lut_b = pinfo->cinema_gamma_lut_table_B;
		} else {
			DPU_FB_ERR("can't get gamma cinema paramter from pinfo.\n");
			return -1;
		}
	} else {
		if (pinfo->gamma_lut_table_len > 0 && pinfo->gamma_lut_table_R &&
			pinfo->gamma_lut_table_G && pinfo->gamma_lut_table_B) {
			gamma_lut_r = pinfo->gamma_lut_table_R;
			gamma_lut_g = pinfo->gamma_lut_table_G;
			gamma_lut_b = pinfo->gamma_lut_table_B;
		} else {
			DPU_FB_ERR("can't get gamma normal parameter from pinfo.\n");
			return -1;
		}
	}

	/* config regsiter use default or cinema parameter */
	for (index = 0; index < pinfo->gamma_lut_table_len / 2; index++) {
		i = index << 1;
		outp32(gamma_lut_base + (U_GAMA_R_COEF + index * 4), (gamma_lut_r[i] | (gamma_lut_r[i + 1] << 16)));
		outp32(gamma_lut_base + (U_GAMA_G_COEF + index * 4), (gamma_lut_g[i] | (gamma_lut_g[i + 1] << 16)));
		outp32(gamma_lut_base + (U_GAMA_B_COEF + index * 4), (gamma_lut_b[i] | (gamma_lut_b[i + 1] << 16)));
		/* GAMA  PRE LUT */
		outp32(gamma_lut_base + (U_GAMA_PRE_R_COEF + index * 4), (gamma_lut_r[i] | (gamma_lut_r[i + 1] << 16)));
		outp32(gamma_lut_base + (U_GAMA_PRE_G_COEF + index * 4), (gamma_lut_g[i] | (gamma_lut_g[i + 1] << 16)));
		outp32(gamma_lut_base + (U_GAMA_PRE_B_COEF + index * 4), (gamma_lut_b[i] | (gamma_lut_b[i + 1] << 16)));
	}
	outp32(gamma_lut_base + U_GAMA_R_LAST_COEF, gamma_lut_r[pinfo->gamma_lut_table_len - 1]);
	outp32(gamma_lut_base + U_GAMA_G_LAST_COEF, gamma_lut_g[pinfo->gamma_lut_table_len - 1]);
	outp32(gamma_lut_base + U_GAMA_B_LAST_COEF, gamma_lut_b[pinfo->gamma_lut_table_len - 1]);
	/* GAMA PRE LUT */
	outp32(gamma_lut_base + U_GAMA_PRE_R_LAST_COEF, gamma_lut_r[pinfo->gamma_lut_table_len - 1]);
	outp32(gamma_lut_base + U_GAMA_PRE_G_LAST_COEF, gamma_lut_g[pinfo->gamma_lut_table_len - 1]);
	outp32(gamma_lut_base + U_GAMA_PRE_B_LAST_COEF, gamma_lut_b[pinfo->gamma_lut_table_len - 1]);

	gama_lut_sel = (uint32_t)inp32(gamma_base + GAMA_LUT_SEL);
	set_reg(gamma_base + GAMA_LUT_SEL, (~(gama_lut_sel & 0x1)) & 0x1, 1, 0);

	return 0;
}

void hisi_dss_dpp_acm_gm_set_reg(struct dpu_fb_data_type *dpufd)
{
	char __iomem *gmp_base = NULL;
	char __iomem *xcc_base = NULL;
	char __iomem *gamma_base = NULL;
	static uint8_t last_gamma_type;
	static uint32_t gamma_config_flag;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");


	if (dpufd->panel_info.gamma_support == 0)
		return;

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_GAMA)) {
		DPU_FB_DEBUG("gamma is not suppportted in this platform\n");
		return;
	}

	dpu_check_and_no_retval((dpufd->index != PRIMARY_PANEL_IDX), ERR, "fb%d, not support!\n", dpufd->index);

	gmp_base = dpufd->dss_base + DSS_DPP_GMP_OFFSET;
	xcc_base = dpufd->dss_base + DSS_DPP_XCC_OFFSET;
	gamma_base = dpufd->dss_base + DSS_DPP_GAMA_OFFSET;


	if (gamma_config_flag == 0) {
		if (last_gamma_type != dpufd->panel_info.gamma_type) {
			set_reg(gamma_base + GAMA_EN, 0x0, 1, 0);
			set_reg(gmp_base + GMP_EN, 0x0, 1, 0);
			set_reg(xcc_base + XCC_EN, 0x1, 1, 0);

			gamma_config_flag = 1;
			last_gamma_type = dpufd->panel_info.gamma_type;
		}
		return;
	}

	if (gamma_config_flag == 1 && hisi_dss_dpp_gamma_para_set_reg(dpufd, last_gamma_type) != 0)
		return;

	set_reg(gamma_base + GAMA_EN, 0x1, 1, 0);
	set_reg(gmp_base + GMP_EN, 0x1, 1, 0);
	set_reg(xcc_base + XCC_EN, 0x1, 1, 0);

	gamma_config_flag = 0;
}

static void print_dsi_error_info(struct dpu_fb_data_type *dpufd, uint32_t *read_value, int num_read_value)
{
	char __iomem *mipi_dsi_base = get_mipi_dsi_base(dpufd);

	if (num_read_value < 4)  /* Mipi read back 4 register valve */
		DPU_FB_INFO("number read value overflow\n");

	DPU_FB_ERR("fb%d, "
		"Number of the Errors on DSI : 0x05 = 0x%x\n"
		"Display Power Mode : 0x0A = 0x%x\n"
		"Display Signal Mode : 0x0E = 0x%x\n"
		"Display Self-Diagnostic Result : 0x0F = 0x%x\n"
		"LDI vstate : 0x%x, LDI dpi0_hstate : 0x%x\n",
		dpufd->index,
		read_value[0], read_value[1], read_value[2], read_value[3],
		inp32(mipi_dsi_base + MIPI_LDI_VSTATE),
		inp32(mipi_dsi_base + MIPI_LDI_DPI0_HSTATE));
}

static void dpufb_te_state_handle(struct dpu_fb_data_type *dpufd,
	uint32_t ldi_vstate, uint32_t phy_status, bool panel_check, uint32_t timeout)
{
	/* waitting te0 */
	if ((ldi_vstate  == LDI_VSTATE_V_WAIT_TE0) || (!panel_check && (phy_status & BIT(1)))) {
		g_vactive_timeout_count++;
		if ((g_vactive_timeout_count >= timeout) && dpufd->panel_info.esd_enable) {
			dpufd->esd_recover_state = ESD_RECOVER_STATE_START;
			if (dpufd->esd_ctrl.esd_check_wq)
				queue_work(dpufd->esd_ctrl.esd_check_wq, &(dpufd->esd_ctrl.esd_check_work));
		}
	}
}

static void hisi_dump_dma_info(struct dpu_fb_data_type *dpufd)
{
	int i;

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (g_dss_module_base[i][MODULE_DMA] == 0)
			continue;
		DPU_FB_INFO("chn%d: [DMA_BUF_DBG0]=0x%x [DMA_BUF_DBG1]=0x%x MOD%d_STATUS=0x%x!\n", i,
			inp32(dpufd->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG0),
			inp32(dpufd->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG1),
			i, inp32(dpufd->dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MOD0_STATUS + i * 0x4));

	}
}

static void print_ldi_int_state(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req_dump)
{
	uint32_t isr_s1;
	uint32_t isr_s2;
	uint32_t isr_s2_mask;
	uint32_t ldi_ctrl;
	uint32_t ldi_frm_msk;
	char __iomem *mipi_dsi_base = NULL;

	if (!pov_req_dump)
		return;

	mipi_dsi_base = get_mipi_dsi_base(dpufd);

	if (pov_req_dump->ovl_idx == DSS_OVL0) {
		isr_s1 = inp32(dpufd->dss_base + GLB_CPU_PDP_INTS);
		isr_s2_mask = inp32(mipi_dsi_base + 0x024C);
		isr_s2 = inp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INTS);
		ldi_ctrl = inp32(mipi_dsi_base + MIPI_LDI_CTRL);
		ldi_frm_msk = inp32(mipi_dsi_base + MIPI_LDI_FRM_MSK);

		DPU_FB_ERR("fb%d, isr_s1=0x%x, isr_s2_mask=0x%x, isr_s2=0x%x, "
			"LDI_CTRL[0x%x], LDI_FRM_MSK[0x%x]\n",
			dpufd->index, isr_s1, isr_s2_mask, isr_s2, ldi_ctrl, ldi_frm_msk);
	} else if (pov_req_dump->ovl_idx == DSS_OVL1) {
		isr_s1 = inp32(dpufd->dss_base + GLB_CPU_SDP_INTS);
		if (is_dp_panel(dpufd)) {
			isr_s2_mask = inp32(dpufd->dss_base +
				DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK);
			isr_s2 = inp32(dpufd->dss_base +
				DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INTS);
			ldi_ctrl = inp32(dpufd->dss_base +
				DSS_LDI_DP_OFFSET + LDI_CTRL);
			ldi_frm_msk = inp32(dpufd->dss_base +
				DSS_LDI_DP_OFFSET + LDI_FRM_MSK);
		} else {
			isr_s2_mask = inp32(dpufd->mipi_dsi1_base + 0x024C);
			isr_s2 = inp32(dpufd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS);
			ldi_ctrl = inp32(dpufd->mipi_dsi1_base + MIPI_LDI_CTRL);
			ldi_frm_msk = inp32(dpufd->mipi_dsi1_base + MIPI_LDI_FRM_MSK);
		}
		DPU_FB_ERR("fb%d, isr_s1=0x%x, isr_s2_mask=0x%x, isr_s2=0x%x, "
			"LDI_CTRL[0x%x], LDI_FRM_MSK[0x%x]\n",
			dpufd->index, isr_s1, isr_s2_mask, isr_s2, ldi_ctrl, ldi_frm_msk);
	}
}

static void hisi_dump_vote_info(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_data_type *fb0 = dpufd_list[PRIMARY_PANEL_IDX];
	struct dpu_fb_data_type *fb1 = dpufd_list[EXTERNAL_PANEL_IDX];
	struct dpu_fb_data_type *fb2 = dpufd_list[AUXILIARY_PANEL_IDX];
	struct dpu_fb_data_type *fb3 = dpufd_list[MEDIACOMMON_PANEL_IDX];

	DPU_FB_INFO("fb%d, voltage[%d, %d, %d, %d], edc[%llu, %llu, %llu, %llu]\n",
		dpufd->index,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_voltage_level : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_voltage_level : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_voltage_level : 0,
		(fb3 != NULL) ? fb3->dss_vote_cmd.dss_voltage_level : 0,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb3 != NULL) ? fb3->dss_vote_cmd.dss_pri_clk_rate : 0);
}

static void hisi_dump_arsr_post_info(struct dpu_fb_data_type *dpufd)
{
	DPU_FB_INFO("arsr_post: 0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x, 0x%x,0x%x,0x%x, 0x%x\n",
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_MODE),
		inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR),
		inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHLEFT),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHLEFT1),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHRIGHT),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHRIGHT1),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IVTOP),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IVBOTTOM),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IVBOTTOM1),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IHINC),
		inp32(dpufd->dss_module.post_scf_base + ARSR_POST_IVINC));
}

static void hisi_dump_dpp_info(struct dpu_fb_data_type *dpufd)
{
	DPU_FB_INFO("dpp: 0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x, 0x%x,0x%x,0x%x, 0x%x\n",
		inp32(dpufd->dss_base + DSS_DISP_GLB_OFFSET + DYN_SW_DEFAULT),
		inp32(dpufd->dss_base + DSS_DPP_CH0_GAMA_OFFSET + GAMA_EN),
		inp32(dpufd->dss_base + DSS_DPP_CH1_GAMA_OFFSET + GAMA_EN),
		inp32(dpufd->dss_base + DSS_DPP_CH0_DEGAMMA_OFFSET + DEGAMA_EN),
		inp32(dpufd->dss_base + DSS_DPP_CH1_DEGAMMA_OFFSET + DEGAMA_EN),
		inp32(dpufd->dss_base + DSS_DPP_CH0_GMP_OFFSET + GMP_EN),
		inp32(dpufd->dss_base + DSS_DPP_CH1_GMP_OFFSET + GMP_EN),
		inp32(dpufd->dss_base + DSS_DPP_CH0_XCC_OFFSET + XCC_EN),
		inp32(dpufd->dss_base + DSS_DPP_CH1_XCC_OFFSET + XCC_EN),
		inp32(dpufd->dss_base + DSS_DPP_POST_XCC_OFFSET + POST_XCC_EN),
		inp32(dpufd->dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_ACE),
		inp32(dpufd->dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_NR),
		inp32(dpufd->dss_base + DSS_HI_ACE_OFFSET + DPE_IMAGE_INFO),
		inp32(dpufd->dss_base + DSS_DPP_DITHER_OFFSET + DITHER_CTL0));
}

static void hisi_dump_mediacrg_info(struct dpu_fb_data_type *dpufd)
{
	DPU_FB_INFO("Mediacrg CLKDIV: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 0 * 4),  /* 64 */
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 1 * 4),  /* 68 */
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 2 * 4),  /* 6c */
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 3 * 4),  /* 70 */
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 4 * 4),  /* 74 */
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 5 * 4),  /* 78 */
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 6 * 4),  /* 7c */
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 7 * 4),  /* 80 */
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 8 * 4),  /* 84 */
		inp32(dpufd->media_crg_base + MEDIA_CLKDIV1 + 9 * 4));  /* 88 */
}

void hisi_dump_current_info(struct dpu_fb_data_type *dpufd)
{
	char __iomem *mipi_dsi_base = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL!\n");
	dpu_check_and_no_retval(!dpufd->dss_base, ERR, "dpufd->dss_base is NULL!\n");
	dpu_check_and_no_retval(!dpufd->pctrl_base, ERR, "dpufd->pctrl_base is NULL!\n");
	dpu_check_and_no_retval(!dpufd->pmctrl_base, ERR, "dpufd->pmctrl_base is NULL!\n");

	if ((dpufd->index != PRIMARY_PANEL_IDX) && (dpufd->index != EXTERNAL_PANEL_IDX))
		return;

	hisi_dump_dma_info(dpufd);
	mipi_dsi_base = get_mipi_dsi_base(dpufd);
	dpu_check_and_no_retval(!mipi_dsi_base, ERR, "dpufd->mipi_dsi_base is NULL!\n");

	DPU_FB_INFO("AIF0 status: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		inp32(dpufd->dss_base + AIF0_MONITOR_OS_R0), inp32(dpufd->dss_base + AIF0_MONITOR_OS_R1),
		inp32(dpufd->dss_base + AIF0_MONITOR_OS_R2), inp32(dpufd->dss_base + AIF0_MONITOR_OS_R3),
		inp32(dpufd->dss_base + AIF0_MONITOR_OS_R4));

	DPU_FB_INFO("PERI_STAT: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		inp32(dpufd->pctrl_base + PERI_STAT0), inp32(dpufd->pctrl_base + PERI_STAT53),
		inp32(dpufd->pctrl_base + PERI_STAT55), inp32(dpufd->pctrl_base + PERI_STAT63),
		inp32(dpufd->pmctrl_base + NOC_POWER_IDLEREQ), inp32(dpufd->pmctrl_base + NOC_POWER_IDLE),
		inp32(dpufd->pmctrl_base + PERI_INT1_MASK), inp32(dpufd->pmctrl_base + PERI_INT1_STAT));

	hisi_dump_vote_info(dpufd);
	hisi_dump_arsr_post_info(dpufd);
	hisi_dump_dpp_info(dpufd);

	DPU_FB_INFO("ov:0x%x,dbuf:0x%x,0x%x,disp_ch:0x%x,0x%x,0x%x,dsc:0x%x\n",
		inp32(dpufd->dss_base +  DSS_OVL0_OFFSET + OV_SIZE),
		inp32(dpufd->dss_base +  DSS_DBUF0_OFFSET + DBUF_FRM_SIZE),
		inp32(dpufd->dss_base +  DSS_DBUF0_OFFSET + DBUF_FRM_HSIZE),
		inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR),
		inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR),
		inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_IFBCSW),
		inp32(dpufd->dss_base + DSS_DSC_OFFSET + DSC_PIC_SIZE));

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V346)
	DPU_FB_INFO("spr ctrl:0x%x,reso:0x%x\n",
		inp32(dpufd->dss_base + SPR_OFFSET + SPR_CTRL),
		inp32(dpufd->dss_base + SPR_OFFSET + SPR_RESO));
#endif

	if (is_dp_panel(dpufd)) {
		DPU_FB_INFO("itf_int_mask:0x%x,itf_ints:0x%x,vstate:0x%x,dpi0_hstate:0x%x\n",
			inp32(dpufd->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK),
			inp32(dpufd->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INTS),
			inp32(dpufd->dss_base + DSS_LDI_DP_OFFSET + LDI_VSTATE),
			inp32(dpufd->dss_base + DSS_LDI_DP_OFFSET + LDI_DPI0_HSTATE));
	} else {
		DPU_FB_INFO("ldi-dsi:0x%x,0x%x,0x%x,0x%x,0x%x\n",
			inp32(mipi_dsi_base + MIPI_LDI_DPI0_HRZ_CTRL2),
			inp32(mipi_dsi_base + MIPI_LDI_VRT_CTRL2),
			inp32(mipi_dsi_base + MIPIDSI_EDPI_CMD_SIZE_OFFSET),
			inp32(mipi_dsi_base + MIPIDSI_VID_VACTIVE_LINES_OFFSET),
			inp32(mipi_dsi_base + MIPI_VSYNC_DELAY_CTRL));
	}

	hisi_dump_mediacrg_info(dpufd);

	DPU_FB_INFO("PMCTRL PERI_CTRL4, CTRL5 = 0x%x, 0x%x\n",
		inp32(dpufd->pmctrl_base + PMCTRL_PERI_CTRL4), inp32(dpufd->pmctrl_base + PMCTRL_PERI_CTRL5));

	DPU_FB_INFO("panel_info.fps=%d, current_frame_rate=%d\n",
		dpufd->panel_info.fps, dpufd->panel_info.frm_rate_ctrl.current_frame_rate);
}

static int hisi_vactive0_get_cmdlist_idxs(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	struct timediff_info *timediff, int ret)
{
	int ret1;
	uint32_t cmdlist_idxs;
	uint32_t cmdlist_idxs_prev = 0;
	uint32_t cmdlist_idxs_prev_prev = 0;
	dss_overlay_t *pov_req_dump = NULL;
	dss_overlay_t *pov_req_prev = NULL;
	dss_overlay_t *pov_req_prev_prev = NULL;

	pov_req_dump = &(dpufd->ov_req_prev_prev);
	pov_req_prev = &(dpufd->ov_req_prev);
	pov_req_prev_prev = &(dpufd->ov_req_prev_prev);

	ret1 = hisi_cmdlist_get_cmdlist_idxs(pov_req_prev, &cmdlist_idxs_prev, NULL);
	if (ret1 != 0)
		DPU_FB_INFO("fb%d, hisi_cmdlist_get_cmdlist_idxs pov_req_prev failed! ret = %d\n",
			dpufd->index, ret1);

	ret1 = hisi_cmdlist_get_cmdlist_idxs(pov_req_prev_prev, &cmdlist_idxs_prev_prev, NULL);
	if (ret1 != 0)
		DPU_FB_INFO("fb%d, hisi_cmdlist_get_cmdlist_idxs "
			"pov_req_prev_prev failed! ret = %d\n", dpufd->index, ret1);

	cmdlist_idxs = cmdlist_idxs_prev | cmdlist_idxs_prev_prev;
	/* cppcheck-suppress */
	DPU_FB_ERR("fb%d, 1st timeout!ret=%d, "
		"flag=%d, pre_pre_frm_no=%u, frm_no=%u, timediff= %u us, "
		"cmdlist_idxs_prev=0x%x, cmdlist_idxs_prev_prev=0x%x, "
		"cmdlist_idxs=0x%x\n",
		dpufd->index, ret, dpufd->vactive0_start_flag,
		pov_req_dump->frame_no, pov_req->frame_no,
		dpufb_timestamp_diff(&(timediff->tv0), &(timediff->tv1)),
		cmdlist_idxs_prev, cmdlist_idxs_prev_prev, cmdlist_idxs);

	return cmdlist_idxs;
}

static int hisi_vactive0_check_and_reset(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	struct timediff_info *timediff, int ret, bool *panel_check)
{
	uint32_t cmdlist_idxs;
	uint32_t read_value[4] = {0};  /* 4 is number of Mipi read back registers */
	uint32_t phy_status;
	uint32_t ldi_vstate;
	char __iomem *mipi_dsi_base = NULL;
	dss_overlay_t *pov_req_dump = NULL;
	dss_overlay_t *pov_req_prev = NULL;
	dss_overlay_t *pov_req_prev_prev = NULL;

	pov_req_dump = &(dpufd->ov_req_prev_prev);
	pov_req_prev = &(dpufd->ov_req_prev);
	pov_req_prev_prev = &(dpufd->ov_req_prev_prev);

	mipi_dsi_base = (dpufd->index == EXTERNAL_PANEL_IDX) ? dpufd->mipi_dsi1_base : get_mipi_dsi_base(dpufd);

	cmdlist_idxs = hisi_vactive0_get_cmdlist_idxs(dpufd, pov_req, timediff, ret);

	ldi_vstate = inp32(mipi_dsi_base + MIPI_LDI_VSTATE);
	phy_status = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
	DPU_FB_ERR("itf_ints=0x%x, dpp_dbg=0x%x, online_fill_lvl=0x%x, "
		"ldi_vstate=0x%x, phy_status=0x%x\n",
		inp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INTS),
		inp32(dpufd->dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_DBG_CNT),
		inp32(dpufd->dss_base + DSS_DBUF0_OFFSET + DBUF_ONLINE_FILL_LEVEL), ldi_vstate, phy_status);

	hisi_dump_current_info(dpufd);
	if (g_debug_ldi_underflow || g_debug_ovl_online_composer_hold)
		hisi_dss_underflow_dump_cmdlist(dpufd, pov_req_prev, pov_req_prev_prev);

	if (g_debug_ovl_online_composer_hold)
		mdelay(HISI_DSS_COMPOSER_HOLD_TIME);

	if (g_debug_ldi_underflow_clear && g_ldi_data_gate_en) {
		hisi_cmdlist_config_reset(dpufd, pov_req_dump, cmdlist_idxs);
		ldi_data_gate(dpufd, false);
		dpufd->underflow_flag = 1;

		*panel_check = mipi_panel_check_reg(dpufd, read_value, ARRAY_SIZE(read_value));
		print_dsi_error_info(dpufd, read_value, ARRAY_SIZE(read_value));

		memset(dpufd->ov_block_infos_prev, 0, HISI_DSS_OV_BLOCK_NUMS * sizeof(dss_overlay_block_t));
		dpufb_dss_overlay_info_init(&dpufd->ov_req_prev);
		dpufb_te_state_handle(dpufd, ldi_vstate, phy_status, *panel_check, 1);
		return -1;
	}

	ldi_data_gate(dpufd, false);
	*panel_check = mipi_panel_check_reg(dpufd, read_value, ARRAY_SIZE(read_value));
	print_dsi_error_info(dpufd, read_value, ARRAY_SIZE(read_value));

	return 0;
}

static int hisi_vactive0_start_event_check(struct dpu_fb_data_type *dpufd, int timeout)
{
	int ret;
	int times = 0;

	while (1) {
		/*lint -e666*/
		ret = wait_event_interruptible_timeout(dpufd->vactive0_start_wq,  /*lint !e578*/
			dpufd->vactive0_start_flag, msecs_to_jiffies(timeout));
		/*lint +e666*/
		if ((ret == -ERESTARTSYS) && (times++ < 50))  /* wait 500ms */
			mdelay(10);  /* 10ms */
		else
			break;
	}

	return ret;
}

static int hisi_vactive0_start_config_br1(
	struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	int ret = 0;
	bool panel_check = true;
	struct timediff_info time_diff = { {0}, {0} };
	uint32_t timeout_interval;
	char __iomem *mipi_dsi_base = NULL;
	uint32_t phy_status;
	uint32_t ldi_vstate;

	timeout_interval = (g_fpga_flag == 0) ?
		DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC : DSS_COMPOSER_TIMEOUT_THRESHOLD_FPGA;
	mipi_dsi_base = (dpufd->index == EXTERNAL_PANEL_IDX) ? dpufd->mipi_dsi1_base : get_mipi_dsi_base(dpufd);

	if ((dpufd->vactive0_start_flag == 1) && (dpufd->secure_ctrl.secure_event == DSS_SEC_ENABLE) &&
		((&(dpufd->ov_req_prev))->sec_enable_status == DSS_SEC_ENABLE)) {
		dpufd->vactive0_start_flag = 0;
		single_frame_update(dpufd);
	}

	if (dpufd->vactive0_start_flag == 0) {
		dpufb_get_timestamp(&(time_diff.tv0));

		ret = hisi_vactive0_start_event_check(dpufd, timeout_interval);
		if (ret <= 0) {
			dpufb_get_timestamp(&(time_diff.tv1));

			ret = hisi_vactive0_check_and_reset(dpufd, pov_req, &time_diff, ret, &panel_check);
			if (ret)
				return 0;

			ret = hisi_vactive0_start_event_check(dpufd, timeout_interval);
			if (ret <= 0) {
				DPU_FB_ERR("fb%d, 2end timeout!ret=%d, vactive0_start_flag=%d, frame_no=%u\n",
					dpufd->index, ret, dpufd->vactive0_start_flag,
					(&(dpufd->ov_req_prev_prev))->frame_no);

				ldi_data_gate(dpufd, false);
				ret = -ETIMEDOUT;
				print_ldi_int_state(dpufd, &(dpufd->ov_req_prev_prev));

				ldi_vstate = inp32(mipi_dsi_base + MIPI_LDI_VSTATE);
				phy_status = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
				dpufb_te_state_handle(dpufd, ldi_vstate, phy_status, panel_check, 1);
			} else {
				ldi_data_gate(dpufd, true);
				ret = -1;
			}
		} else {
			ldi_data_gate(dpufd, true);
			ret = 0;
		}
	}

	dpufd->vactive0_start_flag = 0;
	dpufd->vactive0_end_flag = 0;
	if (ret >= 0)
		g_vactive_timeout_count = 0;

	return ret;
}

static int hisi_vactive0_start_config_video(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	int times = 0;
	struct timeval tv0;
	struct timeval tv1;
	uint32_t cmdlist_idxs = 0;
	dss_overlay_t *pov_req_dump = NULL;
	uint32_t timeout_interval;
	uint32_t prev_vactive0_start;
	int ret;

	pov_req_dump = &(dpufd->ov_req_prev);

	if (g_fpga_flag == 0)
		timeout_interval = DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC;
	else
		timeout_interval = DSS_COMPOSER_TIMEOUT_THRESHOLD_FPGA;

	dpufb_get_timestamp(&tv0);
	ldi_data_gate(dpufd, false);
	prev_vactive0_start = dpufd->vactive0_start_flag;

	while (1) {
		/*lint -e666*/
		ret = wait_event_interruptible_timeout(dpufd->vactive0_start_wq,  /*lint !e578*/
			(prev_vactive0_start != dpufd->vactive0_start_flag),
			msecs_to_jiffies(timeout_interval));
		/*lint +e666*/
		if ((ret == -ERESTARTSYS) && (times++ < 50))  /* 500ms */
			mdelay(10);  /* 10ms */
		else
			break;
	}

	if (ret <= 0) {
		dpufb_get_timestamp(&tv1);

		ret = hisi_cmdlist_get_cmdlist_idxs(pov_req_dump, &cmdlist_idxs, NULL);
		if (ret != 0)
			DPU_FB_INFO("fb%d, hisi_cmdlist_get_cmdlist_idxs pov_req_prev failed! ret = %d\n",
				dpufd->index, ret);

		DPU_FB_ERR("fb%d, 1wait_for vactive0_start_flag timeout!ret=%d, "
			"vactive0_start_flag=%d, pre frame_no=%u, TIMESTAMP_DIFF is %u us,"
			"cmdlist_idxs=0x%x!\n",
			dpufd->index, ret, dpufd->vactive0_start_flag,
			pov_req_dump->frame_no,
			dpufb_timestamp_diff(&tv0, &tv1), cmdlist_idxs);

		hisi_dump_current_info(dpufd);
		if (g_debug_ldi_underflow || g_debug_ovl_online_composer_hold)
			hisi_dss_underflow_dump_cmdlist(dpufd, pov_req_dump, NULL);

		if (g_debug_ovl_online_composer_hold)
			mdelay(HISI_DSS_COMPOSER_HOLD_TIME);
		ret = 0;
	} else {
		ret = 0;
	}

	return ret;
}

int hisi_vactive0_start_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	int ret;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "pov_req is NULL!\n");

	if (is_mipi_cmd_panel(dpufd) && (dpufd->frame_update_flag == 0))
		ret = hisi_vactive0_start_config_br1(dpufd, pov_req);
	else  /* video mode */
		ret = hisi_vactive0_start_config_video(dpufd, pov_req);

	return ret;
}

#pragma GCC diagnostic pop

