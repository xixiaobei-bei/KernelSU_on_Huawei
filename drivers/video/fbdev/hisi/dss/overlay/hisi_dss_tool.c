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

struct hisi_fb_to_dma_pixel_fmt {
	uint32_t fb_pixel_format;
	uint32_t dma_pixel_format;
};

static int hisi_dss_lcd_refresh_right_top(dss_layer_t *layer)
{
	int ret = 0;

	if (layer == NULL) {
		DPU_FB_ERR("layer is NULL\n");
		return -EINVAL;
	}
	switch (layer->transform) {
	case HISI_FB_TRANSFORM_NOP:
		layer->transform = HISI_FB_TRANSFORM_FLIP_H;
		break;
	case HISI_FB_TRANSFORM_FLIP_H:
		layer->transform = HISI_FB_TRANSFORM_NOP;
		break;
	case HISI_FB_TRANSFORM_FLIP_V:
		layer->transform = HISI_FB_TRANSFORM_ROT_180;
		break;
	case HISI_FB_TRANSFORM_ROT_90:
		layer->transform = (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_H);
		break;
	case HISI_FB_TRANSFORM_ROT_180:
		layer->transform = HISI_FB_TRANSFORM_FLIP_V;
		break;
	case HISI_FB_TRANSFORM_ROT_270:
		layer->transform = (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V);
		break;

	case (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_H):
		layer->transform = HISI_FB_TRANSFORM_ROT_90;
		break;
	case (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V):
		layer->transform = HISI_FB_TRANSFORM_ROT_270;
		break;

	default:
		DPU_FB_ERR("not support this transform[%d].\n", layer->transform);
		ret = -1;
		break;
	}

	return ret;
}

static int hisi_dss_lcd_refresh_left_bottom(dss_layer_t *layer)
{
	int ret = 0;

	if (layer == NULL) {
		DPU_FB_ERR("layer is NULL\n");
		return -EINVAL;
	}
	switch (layer->transform) {
	case HISI_FB_TRANSFORM_NOP:
		layer->transform = HISI_FB_TRANSFORM_FLIP_V;
		break;
	case HISI_FB_TRANSFORM_FLIP_H:
		layer->transform = HISI_FB_TRANSFORM_ROT_180;
		break;
	case HISI_FB_TRANSFORM_FLIP_V:
		layer->transform = HISI_FB_TRANSFORM_NOP;
		break;
	case HISI_FB_TRANSFORM_ROT_90:
		layer->transform = (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V);
		break;
	case HISI_FB_TRANSFORM_ROT_180:
		layer->transform = HISI_FB_TRANSFORM_FLIP_H;
		break;
	case HISI_FB_TRANSFORM_ROT_270:
		layer->transform = (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_H);
		break;

	case (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_H):
		layer->transform = HISI_FB_TRANSFORM_ROT_270;
		break;
	case (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V):
		layer->transform = HISI_FB_TRANSFORM_ROT_90;
		break;

	default:
		DPU_FB_ERR("not support this transform[%d].\n", layer->transform);
		ret = -1;
		break;
	}

	return ret;
}

static int hisi_dss_lcd_refresh_right_bottom(dss_layer_t *layer)
{
	int ret = 0;

	if (layer == NULL) {
		DPU_FB_ERR("layer is NULL\n");
		return -EINVAL;
	}
	switch (layer->transform) {
	case HISI_FB_TRANSFORM_NOP:
		layer->transform = HISI_FB_TRANSFORM_ROT_180;
		break;
	case HISI_FB_TRANSFORM_FLIP_H:
		layer->transform = HISI_FB_TRANSFORM_FLIP_V;
		break;
	case HISI_FB_TRANSFORM_FLIP_V:
		layer->transform = HISI_FB_TRANSFORM_FLIP_H;
		break;
	case HISI_FB_TRANSFORM_ROT_90:
		layer->transform = HISI_FB_TRANSFORM_ROT_270;
		break;
	case HISI_FB_TRANSFORM_ROT_180:
		layer->transform = HISI_FB_TRANSFORM_NOP;
		break;
	case HISI_FB_TRANSFORM_ROT_270:
		layer->transform = HISI_FB_TRANSFORM_ROT_90;
		break;

	case (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_H):
		layer->transform = (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V);
		break;
	case (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V):
		layer->transform = (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_H);
		break;

	default:
		DPU_FB_ERR("not support this transform[%d].\n", layer->transform);
		ret = -1;
		break;
	}

	return ret;
}

static void set_layer_dst_rect_x(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_layer_t *layer, struct dpu_panel_info *pinfo)
{
	if ((pinfo->dirty_region_updt_support == 1) &&
		(pov_req->dirty_rect.w > 0) && (pov_req->dirty_rect.h > 0))
		layer->dst_rect.x = (pov_req->dirty_rect.w - (layer->dst_rect.x + layer->dst_rect.w));
	else
		layer->dst_rect.x = (get_panel_xres(dpufd) - (layer->dst_rect.x + layer->dst_rect.w));
}

static void set_layer_dst_rect_y(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_layer_t *layer, struct dpu_panel_info *pinfo)
{
	if ((pinfo->dirty_region_updt_support == 1) &&
		(pov_req->dirty_rect.w > 0) && (pov_req->dirty_rect.h > 0))
		layer->dst_rect.y = (pov_req->dirty_rect.h - (layer->dst_rect.y + layer->dst_rect.h));
	else
		layer->dst_rect.y = (get_panel_yres(dpufd) - (layer->dst_rect.y + layer->dst_rect.h));
}

static int hisi_dss_lcd_refresh_direction_layer(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, dss_layer_t *layer)
{
	struct dpu_panel_info *pinfo = NULL;
	int ret = 0;

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return((pov_req == NULL), -EINVAL, ERR, "pov_req is NULL\n");
	dpu_check_and_return((layer == NULL), -EINVAL, ERR, "layer is NULL\n");

	pinfo = &(dpufd->panel_info);

	if ((pov_req->ovl_idx != DSS_OVL0) && (pov_req->ovl_idx != DSS_OVL1))
		return ret;

	if (pinfo->lcd_refresh_direction_ctrl == LCD_REFRESH_LEFT_TOP) {
		;  /* do nothing */
	} else if (pinfo->lcd_refresh_direction_ctrl == LCD_REFRESH_RIGHT_TOP) {
		ret = hisi_dss_lcd_refresh_right_top(layer);
		if (ret == 0)
			set_layer_dst_rect_x(dpufd, pov_req, layer, pinfo);
	} else if (pinfo->lcd_refresh_direction_ctrl == LCD_REFRESH_LEFT_BOTTOM) {
		ret = hisi_dss_lcd_refresh_left_bottom(layer);
		if (ret == 0)
			set_layer_dst_rect_y(dpufd, pov_req, layer, pinfo);
	} else if (pinfo->lcd_refresh_direction_ctrl == LCD_REFRESH_RIGHT_BOTTOM) {
		ret = hisi_dss_lcd_refresh_right_bottom(layer);
		if (ret == 0) {
			set_layer_dst_rect_x(dpufd, pov_req, layer, pinfo);
			set_layer_dst_rect_y(dpufd, pov_req, layer, pinfo);
		}
	} else {
		DPU_FB_ERR("fb%d, not support this lcd_refresh_direction_ctrl:%d!\n",
			dpufd->index, pinfo->lcd_refresh_direction_ctrl);
		ret = -1;
	}

	return ret;
}

static int hisi_dss_lcd_refresh_direction_dirty_region(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req)
{
	struct dpu_panel_info *pinfo = NULL;
	int ret = 0;

	pinfo = &(dpufd->panel_info);

	if ((pov_req->ovl_idx != DSS_OVL0) && (pov_req->ovl_idx != DSS_OVL1))
		return 0;

	if (pinfo->lcd_refresh_direction_ctrl == LCD_REFRESH_LEFT_TOP) {
		; /* do nothing */
	} else if (pinfo->lcd_refresh_direction_ctrl == LCD_REFRESH_RIGHT_TOP) {
		if (pinfo->dirty_region_updt_support == 1)
			pov_req->dirty_rect.x =
				(get_panel_xres(dpufd) - (pov_req->dirty_rect.x + pov_req->dirty_rect.w));
	} else if (pinfo->lcd_refresh_direction_ctrl == LCD_REFRESH_LEFT_BOTTOM) {
		if (pinfo->dirty_region_updt_support == 1)
			pov_req->dirty_rect.y =
				(get_panel_yres(dpufd) - (pov_req->dirty_rect.y + pov_req->dirty_rect.h));
	} else if (pinfo->lcd_refresh_direction_ctrl == LCD_REFRESH_RIGHT_BOTTOM) {
		if (pinfo->dirty_region_updt_support == 1) {
			pov_req->dirty_rect.x =
				(get_panel_xres(dpufd) - (pov_req->dirty_rect.x + pov_req->dirty_rect.w));
			pov_req->dirty_rect.y =
				(get_panel_yres(dpufd) - (pov_req->dirty_rect.y + pov_req->dirty_rect.h));
		}
	} else {
		DPU_FB_ERR("fb%d, not support this lcd_refresh_direction_ctrl[%d]!\n",
			dpufd->index, pinfo->lcd_refresh_direction_ctrl);
		ret = -1;
	}

	return ret;
}

int hisi_dss_handle_cur_ovl_req(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req)
{
	struct dpu_panel_info *pinfo = NULL;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	dss_layer_t *layer = NULL;
	uint32_t i;
	uint32_t m;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "pov_req is NULL\n");

	pinfo = &(dpufd->panel_info);

	dpufd->resolution_rect = pov_req->res_updt_rect;

	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
	for (m = 0; m < pov_req->ov_block_nums; m++) {
		pov_h_block = &(pov_h_block_infos[m]);

		for (i = 0; i < pov_h_block->layer_nums; i++) {
			layer = &(pov_h_block->layer_infos[i]);

			hisi_dss_lcd_refresh_direction_layer(dpufd, pov_req, layer);
		}
	}

	hisi_dss_lcd_refresh_direction_dirty_region(dpufd, pov_req);

	return 0;
}

static int get_hal_format_16_bpp(const int *hal_format_table, int table_len, uint32_t offset)
{
	int i;

	for (i = 0; i < table_len; i += 2) {
		if (offset == hal_format_table[i])
			return hal_format_table[i + 1];
	}

	return HISI_FB_PIXEL_FORMAT_MAX;
}

int hisi_get_hal_format(struct fb_info *info)
{
	struct fb_var_screeninfo *var = NULL;
	int hal_format = 0;
	/* hal_fromat_blue[0, 2, 4] is offset of Blue bitfield
	 * hal_fromat_blue[1, 3, 5] is hal_format
	 */
	int hal_fromat_blue[] = {
		8, HISI_FB_PIXEL_FORMAT_RGBX_4444,
		10, HISI_FB_PIXEL_FORMAT_RGBX_5551,
		11, HISI_FB_PIXEL_FORMAT_BGR_565
	};
	/* hal_fromat_red[0, 2, 4] is offset of Red bitfield
	 * hal_fromat_red[1, 3, 5] is hal_format
	 */
	int hal_fromat_red[] = {
		8, HISI_FB_PIXEL_FORMAT_BGRX_4444,
		10, HISI_FB_PIXEL_FORMAT_BGRX_5551,
		11, HISI_FB_PIXEL_FORMAT_RGB_565
	};

	dpu_check_and_return((info == NULL), -EINVAL, ERR, "info is NULL\n");

	var = &info->var;

	switch (var->bits_per_pixel) {
	case 16:  /* 16bits per pixel */
		if (var->transp.offset == 12) {  /* offset of transp */
			hal_fromat_blue[1] =  HISI_FB_PIXEL_FORMAT_RGBA_4444;
			hal_fromat_blue[3] = HISI_FB_PIXEL_FORMAT_RGBA_5551;
			hal_fromat_red[1] = HISI_FB_PIXEL_FORMAT_BGRA_4444;
			hal_fromat_red[3] = HISI_FB_PIXEL_FORMAT_BGRA_5551;
		}

		if (var->blue.offset == 0)
			hal_format = get_hal_format_16_bpp(hal_fromat_red, ARRAY_SIZE(hal_fromat_red),
				var->red.offset);
		else
			hal_format = get_hal_format_16_bpp(hal_fromat_blue, ARRAY_SIZE(hal_fromat_blue),
				var->blue.offset);

		if (hal_format == HISI_FB_PIXEL_FORMAT_MAX)
			goto err_return;
		break;
	case 32:  /* 32bits per pixel */
		if (var->blue.offset == 0)
			hal_format = (var->transp.length == 8) ?  /* length of transp */
				HISI_FB_PIXEL_FORMAT_BGRA_8888 : HISI_FB_PIXEL_FORMAT_BGRX_8888;
		else
			hal_format = (var->transp.length == 8) ?  /* length of transp */
				HISI_FB_PIXEL_FORMAT_RGBA_8888 : HISI_FB_PIXEL_FORMAT_RGBX_8888;
		break;

	default:
		goto err_return;
	}

	return hal_format;

err_return:
	DPU_FB_ERR("not support this bits_per_pixel:%d!\n", var->bits_per_pixel);
	return -1;
}

bool hal_format_has_alpha(uint32_t format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_RGBA_4444:
	case HISI_FB_PIXEL_FORMAT_RGBA_5551:
	case HISI_FB_PIXEL_FORMAT_RGBA_8888:

	case HISI_FB_PIXEL_FORMAT_BGRA_4444:
	case HISI_FB_PIXEL_FORMAT_BGRA_5551:
	case HISI_FB_PIXEL_FORMAT_BGRA_8888:

	case HISI_FB_PIXEL_FORMAT_RGBA_1010102:
	case HISI_FB_PIXEL_FORMAT_BGRA_1010102:
		return true;

	default:
		return false;
	}
}

bool is_pixel_10bit2dma(int format)
{
	switch (format) {
	case DMA_PIXEL_FORMAT_RGBA_1010102:
	case DMA_PIXEL_FORMAT_Y410_10BIT:
	case DMA_PIXEL_FORMAT_YUV422_10BIT:
	case DMA_PIXEL_FORMAT_YUV420_SP_10BIT:
	case DMA_PIXEL_FORMAT_YUV422_SP_10BIT:
	case DMA_PIXEL_FORMAT_YUV420_P_10BIT:
	case DMA_PIXEL_FORMAT_YUV422_P_10BIT:
		return true;

	default:
		return false;
	}
}

bool is_pixel_10bit2dfc(int format)
{
	switch (format) {
	case DFC_PIXEL_FORMAT_BGRA_1010102:
	case DFC_PIXEL_FORMAT_YUVA_1010102:
	case DFC_PIXEL_FORMAT_UYVA_1010102:
	case DFC_PIXEL_FORMAT_VUYA_1010102:
	case DFC_PIXEL_FORMAT_YUYV_10:
	case DFC_PIXEL_FORMAT_UYVY_10:
		return true;

	default:
		return false;
	}
}

bool is_yuv_package(uint32_t format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_YUV_422_I:
	case HISI_FB_PIXEL_FORMAT_YUYV_422_PKG:
	case HISI_FB_PIXEL_FORMAT_YVYU_422_PKG:
	case HISI_FB_PIXEL_FORMAT_UYVY_422_PKG:
	case HISI_FB_PIXEL_FORMAT_VYUY_422_PKG:
	case HISI_FB_PIXEL_FORMAT_YUV422_10BIT:
		return true;

	default:
		return false;
	}
}

bool is_yuv_semiplanar(uint32_t format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_YCBCR_422_SP:
	case HISI_FB_PIXEL_FORMAT_YCRCB_422_SP:
	case HISI_FB_PIXEL_FORMAT_YCBCR_420_SP:
	case HISI_FB_PIXEL_FORMAT_YCRCB_420_SP:
	case HISI_FB_PIXEL_FORMAT_YCRCB420_SP_10BIT:
	case HISI_FB_PIXEL_FORMAT_YCBCR420_SP_10BIT:
	case HISI_FB_PIXEL_FORMAT_YCBCR422_SP_10BIT:
		return true;

	default:
		return false;
	}
}

bool is_yuv_plane(uint32_t format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_YCBCR_422_P:
	case HISI_FB_PIXEL_FORMAT_YCRCB_422_P:
	case HISI_FB_PIXEL_FORMAT_YCBCR_420_P:
	case HISI_FB_PIXEL_FORMAT_YCRCB_420_P:
	case HISI_FB_PIXEL_FORMAT_YCBCR420_P_10BIT:
	case HISI_FB_PIXEL_FORMAT_YCBCR422_P_10BIT:
		return true;

	default:
		return false;
	}
}

bool is_yuv(uint32_t format)
{
	return (is_yuv_package(format) || is_yuv_semiplanar(format) || is_yuv_plane(format));
}

bool is_yuv_sp_420(uint32_t format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_YCBCR_420_SP:
	case HISI_FB_PIXEL_FORMAT_YCRCB_420_SP:
	case HISI_FB_PIXEL_FORMAT_YCBCR420_SP_10BIT:
	case HISI_FB_PIXEL_FORMAT_YCRCB420_SP_10BIT:
		return true;

	default:
		return false;
	}
}

bool is_yuv_sp_422(uint32_t format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_YCBCR_422_SP:
	case HISI_FB_PIXEL_FORMAT_YCRCB_422_SP:
		return true;

	default:
		return false;
	}
}

bool is_yuv_p_420(uint32_t format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_YCBCR_420_P:
	case HISI_FB_PIXEL_FORMAT_YCRCB_420_P:
	case HISI_FB_PIXEL_FORMAT_YCBCR420_P_10BIT:
		return true;

	default:
		return false;
	}
}

bool is_yuv_p_422(uint32_t format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_YCBCR_422_P:
	case HISI_FB_PIXEL_FORMAT_YCRCB_422_P:
	case HISI_FB_PIXEL_FORMAT_YCBCR422_P_10BIT:
		return true;

	default:
		return false;
	}
}

bool is_rgbx(uint32_t format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_RGBX_4444:
	case HISI_FB_PIXEL_FORMAT_BGRX_4444:
	case HISI_FB_PIXEL_FORMAT_RGBX_5551:
	case HISI_FB_PIXEL_FORMAT_BGRX_5551:
	case HISI_FB_PIXEL_FORMAT_RGBX_8888:
	case HISI_FB_PIXEL_FORMAT_BGRX_8888:
		return true;

	default:
		return false;
	}
}

uint32_t is_need_rdma_stretch_bit(struct dpu_fb_data_type *dpufd, dss_layer_t *layer)
{
	int v_stretch_ratio_threshold = 0;
	uint32_t v_stretch_ratio = 0;

	if (layer == NULL) {
		DPU_FB_ERR("layer is NULL\n");
		return 0;
	}

	if ((layer->need_cap & CAP_AFBCD) || (layer->need_cap & CAP_HFBCD) || (layer->need_cap & CAP_HEBCD)) {
		v_stretch_ratio = 0;
	} else {
		if (is_yuv_sp_420(layer->img.format) || is_yuv_p_420(layer->img.format)) {
			v_stretch_ratio_threshold = ((layer->src_rect.h + layer->dst_rect.h - 1) / layer->dst_rect.h);
			v_stretch_ratio = ((layer->src_rect.h / layer->dst_rect.h) / 2) * 2;
		} else {
			v_stretch_ratio_threshold = ((layer->src_rect.h + layer->dst_rect.h - 1) / layer->dst_rect.h);
			v_stretch_ratio = (layer->src_rect.h / layer->dst_rect.h);
		}

		if (v_stretch_ratio_threshold <= g_rdma_stretch_threshold)
			v_stretch_ratio = 0;
	}

	return v_stretch_ratio;
}

bool is_arsr_post_need_padding(dss_overlay_t *pov_req, struct dpu_panel_info *pinfo, uint32_t ihinc)
{
	uint32_t ret = false;

	if (pinfo != NULL && pinfo->dummy_pixel_num > 0) {
		DPU_FB_DEBUG("dummy_pixel_num %d\n", pinfo->dummy_pixel_num);
		return false;
	}

	if (ihinc < ARSR1P_INC_FACTOR) {
		ret = true;
		DPU_FB_DEBUG("scale up\n");
	}

	if (pov_req != NULL && pinfo != NULL &&
		(pov_req->res_updt_rect.w > 0) && ((uint32_t)pov_req->res_updt_rect.w < pinfo->xres) &&
		(pov_req->res_updt_rect.h > 0) && ((uint32_t)pov_req->res_updt_rect.h < pinfo->yres)) {
		ret = true;
		DPU_FB_DEBUG("rog enable\n");
	}

#if !defined(CONFIG_HISI_FB_V320) && !defined(CONFIG_HISI_FB_V330) && \
	!defined(CONFIG_HISI_FB_V501) && !defined(CONFIG_HISI_FB_970)
	if (ret && pinfo != NULL &&
		pinfo->dirty_region_updt_support &&
		pinfo->dirty_region_info.w_min != pinfo->xres) {
		DPU_FB_ERR("dirty_region width limit %d != xres %d\n",
			pinfo->dirty_region_info.w_min, pinfo->xres);
		ret = false;
	}

	return ret;
#endif

	return false;
}

int hisi_adjust_clip_rect(dss_layer_t *layer, dss_rect_ltrb_t *clip_rect)
{
	int ret = 0;
	uint32_t temp;

	dpu_check_and_return((layer == NULL), -EINVAL, ERR, "layer is NULL\n");
	dpu_check_and_return((clip_rect == NULL), -EINVAL, ERR, "clip_rect is NULL\n");

	if ((clip_rect->left < 0 || clip_rect->left > DFC_MAX_CLIP_NUM) ||
		(clip_rect->right < 0 || clip_rect->right > DFC_MAX_CLIP_NUM) ||
		(clip_rect->top < 0 || clip_rect->top > DFC_MAX_CLIP_NUM) ||
		(clip_rect->bottom < 0 || clip_rect->bottom > DFC_MAX_CLIP_NUM))
		return -EINVAL;

	switch (layer->transform) {
	case HISI_FB_TRANSFORM_NOP:
		/* do nothing */
		break;
	case HISI_FB_TRANSFORM_FLIP_H:
	case HISI_FB_TRANSFORM_ROT_90:
		{
			temp = clip_rect->left;
			clip_rect->left = clip_rect->right;
			clip_rect->right = temp;
		}
		break;
	case HISI_FB_TRANSFORM_FLIP_V:
	case HISI_FB_TRANSFORM_ROT_270:
		{
			temp = clip_rect->top;
			clip_rect->top = clip_rect->bottom;
			clip_rect->bottom = temp;
		}
		break;
	case HISI_FB_TRANSFORM_ROT_180:
		{
			temp = clip_rect->left;
			clip_rect->left =  clip_rect->right;
			clip_rect->right = temp;

			temp = clip_rect->top;
			clip_rect->top =  clip_rect->bottom;
			clip_rect->bottom = temp;
		}
		break;
	default:
		DPU_FB_ERR("not supported this transform:%d!\n", layer->transform);
		break;
	}

	return ret;
}

static struct hisi_fb_to_dma_pixel_fmt hisi_dma_fmt[] = {
	{ HISI_FB_PIXEL_FORMAT_RGB_565, DMA_PIXEL_FORMAT_RGB_565 },
	{ HISI_FB_PIXEL_FORMAT_BGR_565, DMA_PIXEL_FORMAT_RGB_565 },
	{ HISI_FB_PIXEL_FORMAT_RGBX_4444, DMA_PIXEL_FORMAT_XRGB_4444 },
	{ HISI_FB_PIXEL_FORMAT_BGRX_4444, DMA_PIXEL_FORMAT_XRGB_4444 },
	{ HISI_FB_PIXEL_FORMAT_RGBA_4444, DMA_PIXEL_FORMAT_ARGB_4444 },
	{ HISI_FB_PIXEL_FORMAT_BGRA_4444, DMA_PIXEL_FORMAT_ARGB_4444 },
	{ HISI_FB_PIXEL_FORMAT_RGBX_5551, DMA_PIXEL_FORMAT_XRGB_5551 },
	{ HISI_FB_PIXEL_FORMAT_BGRX_5551, DMA_PIXEL_FORMAT_XRGB_5551 },
	{ HISI_FB_PIXEL_FORMAT_RGBA_5551, DMA_PIXEL_FORMAT_ARGB_5551 },
	{ HISI_FB_PIXEL_FORMAT_BGRA_5551, DMA_PIXEL_FORMAT_ARGB_5551 },
	{ HISI_FB_PIXEL_FORMAT_RGBX_8888, DMA_PIXEL_FORMAT_XRGB_8888 },
	{ HISI_FB_PIXEL_FORMAT_BGRX_8888, DMA_PIXEL_FORMAT_XRGB_8888 },
	{ HISI_FB_PIXEL_FORMAT_RGBA_8888, DMA_PIXEL_FORMAT_ARGB_8888 },
	{ HISI_FB_PIXEL_FORMAT_BGRA_8888, DMA_PIXEL_FORMAT_ARGB_8888 },
	{ HISI_FB_PIXEL_FORMAT_YUV_422_I, DMA_PIXEL_FORMAT_YUYV_422_Pkg },
	{ HISI_FB_PIXEL_FORMAT_YUYV_422_PKG, DMA_PIXEL_FORMAT_YUYV_422_Pkg },
	{ HISI_FB_PIXEL_FORMAT_YVYU_422_PKG, DMA_PIXEL_FORMAT_YUYV_422_Pkg },
	{ HISI_FB_PIXEL_FORMAT_UYVY_422_PKG, DMA_PIXEL_FORMAT_YUYV_422_Pkg },
	{ HISI_FB_PIXEL_FORMAT_VYUY_422_PKG, DMA_PIXEL_FORMAT_YUYV_422_Pkg },
	{ HISI_FB_PIXEL_FORMAT_YCBCR_422_P, DMA_PIXEL_FORMAT_YUV_422_P_HP },
	{ HISI_FB_PIXEL_FORMAT_YCRCB_422_P, DMA_PIXEL_FORMAT_YUV_422_P_HP },
	{ HISI_FB_PIXEL_FORMAT_YCBCR_420_P, DMA_PIXEL_FORMAT_YUV_420_P_HP },
	{ HISI_FB_PIXEL_FORMAT_YCRCB_420_P, DMA_PIXEL_FORMAT_YUV_420_P_HP },
	{ HISI_FB_PIXEL_FORMAT_YCBCR_422_SP, DMA_PIXEL_FORMAT_YUV_422_SP_HP },
	{ HISI_FB_PIXEL_FORMAT_YCRCB_422_SP, DMA_PIXEL_FORMAT_YUV_422_SP_HP },
	{ HISI_FB_PIXEL_FORMAT_YCBCR_420_SP, DMA_PIXEL_FORMAT_YUV_420_SP_HP },
	{ HISI_FB_PIXEL_FORMAT_YCRCB_420_SP, DMA_PIXEL_FORMAT_YUV_420_SP_HP },
	{ HISI_FB_PIXEL_FORMAT_RGBA_1010102, DMA_PIXEL_FORMAT_RGBA_1010102 },
	{ HISI_FB_PIXEL_FORMAT_BGRA_1010102, DMA_PIXEL_FORMAT_RGBA_1010102 },
	{ HISI_FB_PIXEL_FORMAT_Y410_10BIT, DMA_PIXEL_FORMAT_Y410_10BIT },
	{ HISI_FB_PIXEL_FORMAT_YUV422_10BIT, DMA_PIXEL_FORMAT_YUV422_10BIT },
	{ HISI_FB_PIXEL_FORMAT_YCRCB420_SP_10BIT, DMA_PIXEL_FORMAT_YUV420_SP_10BIT },
	{ HISI_FB_PIXEL_FORMAT_YCBCR420_SP_10BIT, DMA_PIXEL_FORMAT_YUV420_SP_10BIT },
	{ HISI_FB_PIXEL_FORMAT_YCBCR422_SP_10BIT, DMA_PIXEL_FORMAT_YUV422_SP_10BIT },
	{ HISI_FB_PIXEL_FORMAT_YCBCR420_P_10BIT, DMA_PIXEL_FORMAT_YUV420_P_10BIT },
	{ HISI_FB_PIXEL_FORMAT_YCBCR422_P_10BIT, DMA_PIXEL_FORMAT_YUV422_P_10BIT },
};

int hisi_pixel_format_hal2dma(int format)
{
	int i;
	int size = sizeof(hisi_dma_fmt) / sizeof(hisi_dma_fmt[0]);

	for (i = 0; i < size; i++) {
		if (hisi_dma_fmt[i].fb_pixel_format == format)
			break;
	}

	if (i >= size) {
		DPU_FB_ERR("not support format-%d!\n", format);
		return -1;
	}

	return hisi_dma_fmt[i].dma_pixel_format;
}
/*lint -e655*/
int hisi_transform_hal2dma(int transform, int chn_idx)
{
	int ret;

	if (chn_idx < DSS_WCHN_W0 || chn_idx == DSS_RCHN_V2) {
		switch (transform) {
		case HISI_FB_TRANSFORM_NOP:
			ret = DSS_TRANSFORM_NOP;
			break;
		case HISI_FB_TRANSFORM_FLIP_H:
			ret = DSS_TRANSFORM_FLIP_H;
			break;
		case HISI_FB_TRANSFORM_FLIP_V:
			ret = DSS_TRANSFORM_FLIP_V;
			break;
		case HISI_FB_TRANSFORM_ROT_180:
			ret = DSS_TRANSFORM_FLIP_V | DSS_TRANSFORM_FLIP_H;
			break;
		case HISI_FB_TRANSFORM_ROT_90:
			ret = DSS_TRANSFORM_ROT | DSS_TRANSFORM_FLIP_H;
			break;
		case HISI_FB_TRANSFORM_ROT_270:
			ret = DSS_TRANSFORM_ROT | DSS_TRANSFORM_FLIP_V;
			break;
		default:
			ret = -1;
			DPU_FB_ERR("Transform %d is not supported\n", transform);
			break;
		}
	} else {
		if (transform == HISI_FB_TRANSFORM_NOP) {
			ret = DSS_TRANSFORM_NOP;
		} else if (transform == (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V)) {
			ret = DSS_TRANSFORM_ROT;
		} else {
			ret = -1;
			DPU_FB_ERR("Transform %d is not supported\n", transform);
		}
	}

	return ret;
}

