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

struct rect_pad {
	uint32_t left;
	uint32_t right;
	uint32_t top;
	uint32_t bottom;
};

struct hisi_fb_to_dfc_pixel_fmt {
	uint32_t fb_pixel_format;
	uint32_t dfc_pixel_format;
};

struct dfc_config {
	int dfc_fmt;
	int dfc_bpp;
	int dfc_pix_in_num;
	int dfc_aligned;
	uint32_t size_hrz;
	uint32_t size_vrt;
	int dfc_hrz_clip;
	bool need_clip;
};

struct wdfc_internal_data {
	uint32_t size_hrz;
	uint32_t size_vrt;
	int dfc_pix_in_num;
	uint32_t dfc_w;
	int aligned_pixel;
	uint32_t dfc_aligned;
};

void hisi_dss_dfc_init(const char __iomem *dfc_base, dss_dfc_t *s_dfc)
{
	if (dfc_base == NULL) {
		DPU_FB_ERR("dfc_base is NULL\n");
		return;
	}
	if (s_dfc == NULL) {
		DPU_FB_ERR("s_dfc is NULL\n");
		return;
	}

	memset(s_dfc, 0, sizeof(dss_dfc_t));

	s_dfc->disp_size = inp32(dfc_base + DFC_DISP_SIZE);
	s_dfc->pix_in_num = inp32(dfc_base + DFC_PIX_IN_NUM);
	s_dfc->disp_fmt = inp32(dfc_base + DFC_DISP_FMT);
	s_dfc->clip_ctl_hrz = inp32(dfc_base + DFC_CLIP_CTL_HRZ);
	s_dfc->clip_ctl_vrz = inp32(dfc_base + DFC_CLIP_CTL_VRZ);
	s_dfc->ctl_clip_en = inp32(dfc_base + DFC_CTL_CLIP_EN);
	s_dfc->icg_module = inp32(dfc_base + DFC_ICG_MODULE);
	s_dfc->dither_enable = inp32(dfc_base + DFC_DITHER_ENABLE);
	s_dfc->padding_ctl = inp32(dfc_base + DFC_PADDING_CTL);
}

void hisi_dss_dfc_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *dfc_base, dss_dfc_t *s_dfc)
{
	if (dpufd == NULL) {
		DPU_FB_DEBUG("dpufd is NULL!\n");
		return;
	}

	if (dfc_base == NULL) {
		DPU_FB_DEBUG("dfc_base is NULL!\n");
		return;
	}

	if (s_dfc == NULL) {
		DPU_FB_DEBUG("s_dfc is NULL!\n");
		return;
	}

	dpufd->set_reg(dpufd, dfc_base + DFC_DISP_SIZE, s_dfc->disp_size, 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_PIX_IN_NUM, s_dfc->pix_in_num, 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_DISP_FMT, s_dfc->disp_fmt, 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_CLIP_CTL_HRZ, s_dfc->clip_ctl_hrz, 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_CLIP_CTL_VRZ, s_dfc->clip_ctl_vrz, 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_CTL_CLIP_EN, s_dfc->ctl_clip_en, 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_ICG_MODULE, s_dfc->icg_module, 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_DITHER_ENABLE, s_dfc->dither_enable, 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_PADDING_CTL, s_dfc->padding_ctl, 32, 0);

	if (dpufd->index != EXTERNAL_PANEL_IDX)
		dpufd->set_reg(dpufd, dfc_base + DFC_BITEXT_CTL, s_dfc->bitext_ctl, 32, 0);
}

static struct hisi_fb_to_dfc_pixel_fmt hisi_dfc_fmt[] = {
	{ HISI_FB_PIXEL_FORMAT_RGB_565, DFC_PIXEL_FORMAT_RGB_565 },
	{ HISI_FB_PIXEL_FORMAT_RGBX_4444, DFC_PIXEL_FORMAT_XBGR_4444 },
	{ HISI_FB_PIXEL_FORMAT_RGBA_4444, DFC_PIXEL_FORMAT_ABGR_4444 },
	{ HISI_FB_PIXEL_FORMAT_RGBX_5551, DFC_PIXEL_FORMAT_XBGR_5551 },
	{ HISI_FB_PIXEL_FORMAT_RGBA_5551, DFC_PIXEL_FORMAT_ABGR_5551 },
	{ HISI_FB_PIXEL_FORMAT_RGBX_8888, DFC_PIXEL_FORMAT_XBGR_8888 },
	{ HISI_FB_PIXEL_FORMAT_RGBA_8888, DFC_PIXEL_FORMAT_ABGR_8888 },
	{ HISI_FB_PIXEL_FORMAT_RGBA_1010102, DFC_PIXEL_FORMAT_BGRA_1010102 },
	{ HISI_FB_PIXEL_FORMAT_BGR_565, DFC_PIXEL_FORMAT_BGR_565 },
	{ HISI_FB_PIXEL_FORMAT_BGRX_4444, DFC_PIXEL_FORMAT_XRGB_4444 },
	{ HISI_FB_PIXEL_FORMAT_BGRA_4444, DFC_PIXEL_FORMAT_ARGB_4444 },
	{ HISI_FB_PIXEL_FORMAT_BGRX_5551, DFC_PIXEL_FORMAT_XRGB_5551 },
	{ HISI_FB_PIXEL_FORMAT_BGRA_5551, DFC_PIXEL_FORMAT_ARGB_5551 },
	{ HISI_FB_PIXEL_FORMAT_BGRX_8888, DFC_PIXEL_FORMAT_XRGB_8888 },
	{ HISI_FB_PIXEL_FORMAT_BGRA_8888, DFC_PIXEL_FORMAT_ARGB_8888 },
	{ HISI_FB_PIXEL_FORMAT_BGRA_1010102, DFC_PIXEL_FORMAT_BGRA_1010102 },
	{ HISI_FB_PIXEL_FORMAT_YUV_422_I, DFC_PIXEL_FORMAT_YUYV422 },
	{ HISI_FB_PIXEL_FORMAT_YUYV_422_PKG, DFC_PIXEL_FORMAT_YUYV422 },
	{ HISI_FB_PIXEL_FORMAT_YVYU_422_PKG, DFC_PIXEL_FORMAT_YVYU422 },
	{ HISI_FB_PIXEL_FORMAT_UYVY_422_PKG, DFC_PIXEL_FORMAT_UYVY422 },
	{ HISI_FB_PIXEL_FORMAT_VYUY_422_PKG, DFC_PIXEL_FORMAT_VYUY422 },
	{ HISI_FB_PIXEL_FORMAT_YCBCR_422_SP, DFC_PIXEL_FORMAT_YUYV422 },
	{ HISI_FB_PIXEL_FORMAT_YCRCB_422_SP, DFC_PIXEL_FORMAT_YVYU422 },
	{ HISI_FB_PIXEL_FORMAT_YCBCR_420_SP, DFC_PIXEL_FORMAT_YUYV422 },
	{ HISI_FB_PIXEL_FORMAT_YCRCB_420_SP, DFC_PIXEL_FORMAT_YVYU422 },
	{ HISI_FB_PIXEL_FORMAT_YCBCR_422_P, DFC_PIXEL_FORMAT_YUYV422 },
	{ HISI_FB_PIXEL_FORMAT_YCBCR_420_P, DFC_PIXEL_FORMAT_YUYV422 },
	{ HISI_FB_PIXEL_FORMAT_YCRCB_422_P, DFC_PIXEL_FORMAT_YVYU422 },
	{ HISI_FB_PIXEL_FORMAT_YCRCB_420_P, DFC_PIXEL_FORMAT_YVYU422 },
	{ HISI_FB_PIXEL_FORMAT_Y410_10BIT, DFC_PIXEL_FORMAT_UYVA_1010102 },
	{ HISI_FB_PIXEL_FORMAT_YCRCB420_SP_10BIT, DFC_PIXEL_FORMAT_YUYV_10 },
	{ HISI_FB_PIXEL_FORMAT_YUV422_10BIT, DFC_PIXEL_FORMAT_YUYV_10 },
	{ HISI_FB_PIXEL_FORMAT_YCBCR420_SP_10BIT, DFC_PIXEL_FORMAT_YUYV_10 },
	{ HISI_FB_PIXEL_FORMAT_YCBCR422_SP_10BIT, DFC_PIXEL_FORMAT_YUYV_10 },
	{ HISI_FB_PIXEL_FORMAT_YCBCR420_P_10BIT, DFC_PIXEL_FORMAT_YUYV_10 },
	{ HISI_FB_PIXEL_FORMAT_YCBCR422_P_10BIT, DFC_PIXEL_FORMAT_YUYV_10 },
};

static int hisi_pixel_format_hal2dfc(int format)
{
	int i;
	int size = sizeof(hisi_dfc_fmt) / sizeof(hisi_dfc_fmt[0]);

	for (i = 0; i < size; i++) {
		if (hisi_dfc_fmt[i].fb_pixel_format == format)
			break;
	}

	if (i >= size) {
		DPU_FB_ERR("not support format-%d!\n", format);
		return -1;
	}

	return hisi_dfc_fmt[i].dfc_pixel_format;
}

static int hisi_rb_swap(int format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_BGR_565:
	case HISI_FB_PIXEL_FORMAT_BGRX_4444:
	case HISI_FB_PIXEL_FORMAT_BGRA_4444:
	case HISI_FB_PIXEL_FORMAT_BGRX_5551:
	case HISI_FB_PIXEL_FORMAT_BGRA_5551:
	case HISI_FB_PIXEL_FORMAT_BGRX_8888:
	case HISI_FB_PIXEL_FORMAT_BGRA_8888:
	case HISI_FB_PIXEL_FORMAT_BGRA_1010102:
		return 1;
	default:
		return 0;
	}
}

static int hisi_uv_swap(int format)
{
	switch (format) {
	case HISI_FB_PIXEL_FORMAT_YCRCB_422_SP:
	case HISI_FB_PIXEL_FORMAT_YCRCB_420_SP:
	case HISI_FB_PIXEL_FORMAT_YCRCB_422_P:
	case HISI_FB_PIXEL_FORMAT_YCRCB_420_P:
	case HISI_FB_PIXEL_FORMAT_YCRCB420_SP_10BIT:
	case HISI_FB_PIXEL_FORMAT_YCRCB422_SP_10BIT:
		return 1;

	default:
		return 0;
	}
}

static int hisi_dfc_get_bpp(int dfc_format)
{
	int ret = 0;

	switch (dfc_format) {
	case DFC_PIXEL_FORMAT_RGB_565:
	case DFC_PIXEL_FORMAT_XRGB_4444:
	case DFC_PIXEL_FORMAT_ARGB_4444:
	case DFC_PIXEL_FORMAT_XRGB_5551:
	case DFC_PIXEL_FORMAT_ARGB_5551:

	case DFC_PIXEL_FORMAT_BGR_565:
	case DFC_PIXEL_FORMAT_XBGR_4444:
	case DFC_PIXEL_FORMAT_ABGR_4444:
	case DFC_PIXEL_FORMAT_XBGR_5551:
	case DFC_PIXEL_FORMAT_ABGR_5551:
		ret = 2;  /* dfc_bpp format */
		break;

	case DFC_PIXEL_FORMAT_XRGB_8888:
	case DFC_PIXEL_FORMAT_ARGB_8888:
	case DFC_PIXEL_FORMAT_XBGR_8888:
	case DFC_PIXEL_FORMAT_ABGR_8888:
	case DFC_PIXEL_FORMAT_BGRA_1010102:
		ret = 4;  /* dfc_bpp format */
		break;

	case DFC_PIXEL_FORMAT_YUV444:
	case DFC_PIXEL_FORMAT_YVU444:
		ret = 3;  /* dfc_bpp format */
		break;

	case DFC_PIXEL_FORMAT_YUYV422:
	case DFC_PIXEL_FORMAT_YVYU422:
	case DFC_PIXEL_FORMAT_VYUY422:
	case DFC_PIXEL_FORMAT_UYVY422:
	case DFC_PIXEL_FORMAT_YUYV_10:
		ret = 2;  /* dfc_bpp format */
		break;
	case DFC_PIXEL_FORMAT_UYVA_1010102:
		ret = 4;  /* dfc_bpp format */
		break;
	default:
		DPU_FB_ERR("not support format[%d]!\n", dfc_format);
		ret = -1;
		break;
	}

	return ret;
}

static bool is_need_rect_clip(dss_rect_ltrb_t clip_rect)
{
	return ((clip_rect.left > 0) || (clip_rect.top > 0) ||
		(clip_rect.right > 0) || (clip_rect.bottom > 0));
}

static void hisi_dss_config_dfc_value(dss_layer_t *layer, dss_dfc_t *dfc, struct dfc_config rdfc_config,
		dss_rect_ltrb_t clip_rect, bool need_bitext)
{
	/* disp-size valve */
	dfc->disp_size = set_bits32(dfc->disp_size, (rdfc_config.size_vrt | (rdfc_config.size_hrz << 16)), 29, 0);
	dfc->pix_in_num = set_bits32(dfc->pix_in_num, rdfc_config.dfc_pix_in_num, 1, 0); /* pix_num valve */
	dfc->disp_fmt = set_bits32(dfc->disp_fmt,
		(((uint32_t)rdfc_config.dfc_fmt << 1) | ((uint32_t)hisi_uv_swap(layer->img.format) << 6) |
		((uint32_t)hisi_rb_swap(layer->img.format) << 7)), 8, 0);

	if (rdfc_config.need_clip) {
		dfc->clip_ctl_hrz = set_bits32(dfc->clip_ctl_hrz,
			((uint32_t)clip_rect.right | ((uint32_t)clip_rect.left << 16)), 32, 0);
		dfc->clip_ctl_vrz = set_bits32(dfc->clip_ctl_vrz,
			((uint32_t)clip_rect.bottom | ((uint32_t)clip_rect.top << 16)), 32, 0);
		dfc->ctl_clip_en = set_bits32(dfc->ctl_clip_en, 0x1, 1, 0);
	} else {
		dfc->clip_ctl_hrz = set_bits32(dfc->clip_ctl_hrz, 0x0, 32, 0);
		dfc->clip_ctl_vrz = set_bits32(dfc->clip_ctl_vrz, 0x0, 32, 0);
		dfc->ctl_clip_en = set_bits32(dfc->ctl_clip_en, 0x1, 1, 0);
	}
	dfc->icg_module = set_bits32(dfc->icg_module, 0x1, 1, 0);
	dfc->dither_enable = set_bits32(dfc->dither_enable, 0x0, 1, 0);
	dfc->padding_ctl = set_bits32(dfc->padding_ctl, 0x0, 17, 0);

	if (need_bitext)
		dfc->bitext_ctl = set_bits32(dfc->bitext_ctl, 0x3, 32, 0);
}

int hisi_dss_rdfc_config(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer,
	dss_rect_t *aligned_rect,
	dss_rect_ltrb_t clip_rect,
	struct hisi_ov_compose_flag *ov_compose_flag)
{
	dss_dfc_t *dfc = NULL;
	int chn_idx;
	bool need_bitext = false;
	struct dfc_config rdfc_config;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return(!layer, -EINVAL, ERR, "layer is NULL\n");
	dpu_check_and_return(!aligned_rect, -EINVAL, ERR, "aligned_rect is NULL\n");
	dpu_check_and_return(!ov_compose_flag, -EINVAL, ERR, "ov_compose_flag is NULL\n");

	chn_idx = layer->chn_idx;

	dfc = &(dpufd->dss_module.dfc[chn_idx]);
	dpufd->dss_module.dfc_used[chn_idx] = 1;

	rdfc_config.dfc_fmt = hisi_pixel_format_hal2dfc(layer->img.format);
	if (rdfc_config.dfc_fmt < 0) {
		DPU_FB_ERR("layer format-%d not support!\n", layer->img.format);
		return -EINVAL;
	}
	need_bitext = !is_pixel_10bit2dfc(rdfc_config.dfc_fmt);

	rdfc_config.dfc_bpp = hisi_dfc_get_bpp(rdfc_config.dfc_fmt);
	if (rdfc_config.dfc_bpp <= 0) {
		DPU_FB_ERR("dfc_bpp-%d not support !\n", rdfc_config.dfc_bpp);
		return -EINVAL;
	}

	rdfc_config.dfc_pix_in_num = (rdfc_config.dfc_bpp <= 2) ? 0x1 : 0x0;  /* dfc pix nums */
	rdfc_config.dfc_aligned = (rdfc_config.dfc_bpp <= 2) ? 4 : 2;  /* dfc aligned valve */

	rdfc_config.need_clip = is_need_rect_clip(clip_rect);

	rdfc_config.size_hrz = DSS_WIDTH(aligned_rect->w);
	rdfc_config.size_vrt = DSS_HEIGHT(aligned_rect->h);

	if (((rdfc_config.size_hrz + 1) % rdfc_config.dfc_aligned) != 0) {
		rdfc_config.size_hrz -= 1;
		DPU_FB_ERR("SIZE_HRT=%d mismatch!bpp=%d\n", rdfc_config.size_hrz, layer->img.bpp);

		DPU_FB_ERR("layer_idx%d, format=%d, transform=%d, original_src_rect:%d,%d,%d,%d, "
			"rdma_out_rect:%d,%d,%d,%d, dst_rect:%d,%d,%d,%d!\n",
			layer->layer_idx, layer->img.format, layer->transform,
			layer->src_rect.x, layer->src_rect.y, layer->src_rect.w, layer->src_rect.h,
			aligned_rect->x, aligned_rect->y, aligned_rect->w, aligned_rect->h,
			layer->dst_rect.x, layer->dst_rect.y, layer->dst_rect.w, layer->dst_rect.h);
	}

	rdfc_config.dfc_hrz_clip = (rdfc_config.size_hrz + 1) % rdfc_config.dfc_aligned;
	if (rdfc_config.dfc_hrz_clip) {
		clip_rect.right += rdfc_config.dfc_hrz_clip;
		rdfc_config.size_hrz += rdfc_config.dfc_hrz_clip;
		rdfc_config.need_clip = true;
	}

	if (!ov_compose_flag->csc_needed)
		need_bitext = false;

	hisi_dss_config_dfc_value(layer, dfc, rdfc_config, clip_rect, need_bitext);

	/* update */
	if (rdfc_config.need_clip) {
		aligned_rect->w -= (clip_rect.left + clip_rect.right);
		aligned_rect->h -= (clip_rect.top + clip_rect.bottom);
	}

	return 0;
}

static bool is_need_dither(int fmt)
{
	return (fmt == DFC_PIXEL_FORMAT_RGB_565) || (fmt == DFC_PIXEL_FORMAT_BGR_565);
}

static void compress_set_rect_align(dss_rect_t *aligned_rect, dss_rect_t in_rect, uint32_t dfc_w,
	uint32_t xw_align, uint32_t yh_aligh)
{
	aligned_rect->x = ALIGN_DOWN((uint32_t)in_rect.x, xw_align);
	aligned_rect->w = ALIGN_UP((uint32_t)(in_rect.x - aligned_rect->x + in_rect.w + dfc_w), xw_align);
	aligned_rect->y = ALIGN_DOWN((uint32_t)in_rect.y, yh_aligh);
	aligned_rect->h = ALIGN_UP((uint32_t)(in_rect.y - aligned_rect->y + in_rect.h), yh_aligh);
}

static void compress_set_rect_pad(
	dss_rect_t *aligned_rect, dss_rect_t in_rect, uint32_t dfc_w, struct rect_pad *pad)
{
	pad->left = in_rect.x - aligned_rect->x;
	pad->right = aligned_rect->w - (in_rect.x - aligned_rect->x + in_rect.w + dfc_w);
	pad->top = in_rect.y - aligned_rect->y;
	pad->bottom = aligned_rect->h - (in_rect.y - aligned_rect->y + in_rect.h);
}

static void hfbce_set_rect_align(dss_rect_t *aligned_rect, dss_rect_t in_rect, uint32_t dfc_w,
	dss_wb_layer_t *layer, struct rect_pad *pad)
{
#ifdef SUPPORT_DSS_HFBCE
	bool is_rot90 = layer->transform & HISI_FB_TRANSFORM_ROT_90;
	uint32_t xw_align = is_rot90 ? HFBC_BLOCK1_HEIGHT_ALIGN : HFBC_BLOCK0_WIDTH_ALIGN;
	uint32_t yh_align = is_rot90 ? HFBC_BLOCK1_WIDTH_ALIGN : HFBC_BLOCK0_HEIGHT_ALIGN;

	compress_set_rect_align(aligned_rect, in_rect, dfc_w, xw_align, yh_align);
#endif
	compress_set_rect_pad(aligned_rect, in_rect, dfc_w, pad);
}

static void hebce_set_rect_align(dss_rect_t *aligned_rect, dss_rect_t in_rect, uint32_t dfc_w,
	dss_wb_layer_t *layer, struct rect_pad *pad)
{
	compress_set_rect_pad(aligned_rect, in_rect, dfc_w, pad);
}

static void no_compress_rot90_set_rect_align(dss_rect_t *aligned_rect, dss_rect_t in_rect,
	dss_wb_layer_t *layer, struct wdfc_internal_data data, struct rect_pad *pad)
{
	uint32_t aligned_line;
	uint32_t addr;
	uint32_t dst_addr;
	uint32_t bpp;
	bool mmu_enable;

	aligned_line = (layer->dst.bpp <= 2) ? 32 : 16; /* sp:32bytes ; others:64bytes */
	mmu_enable = (layer->dst.mmu_enable == 1) ? true : false;
	dst_addr = mmu_enable ? layer->dst.vir_addr : layer->dst.phy_addr;
	bpp = layer->dst.bpp;
	addr = dst_addr + layer->dst_rect.x * bpp +
		(in_rect.x - layer->dst_rect.x + layer->dst_rect.y) * layer->dst.stride;

	if (is_yuv_sp_420(layer->dst.format))
		pad->top = (addr & 0x1F) / bpp;  /* bit[0~4] */
	else
		pad->top = (addr & 0x3F) / bpp;  /* bit[0~5] */

	aligned_rect->x = in_rect.x;
	aligned_rect->y = in_rect.y;
	aligned_rect->w = ALIGN_UP(data.size_hrz + 1, data.dfc_aligned);
	aligned_rect->h = ALIGN_UP((uint32_t)in_rect.h + pad->top, aligned_line);

	pad->left = 0;
	pad->right = aligned_rect->w - data.size_hrz - 1;
	pad->bottom = aligned_rect->h - in_rect.h - pad->top;
}

static void no_compress_set_rect_align(dss_rect_t *aligned_rect, dss_rect_t in_rect, struct wdfc_internal_data data,
	dss_wb_layer_t *layer, struct rect_pad *pad)
{
	aligned_rect->x = ALIGN_DOWN((uint32_t)in_rect.x, (uint32_t)data.aligned_pixel);
	aligned_rect->w = ALIGN_UP((uint32_t)(in_rect.x - aligned_rect->x + in_rect.w + data.dfc_w),
		(uint32_t)data.aligned_pixel);
	aligned_rect->y = in_rect.y;

	if (is_yuv_sp_420(layer->dst.format))
		aligned_rect->h = ALIGN_UP((uint32_t)(in_rect.h), 2);  /* even alignment */
	else
		aligned_rect->h = in_rect.h;

	pad->left = in_rect.x - aligned_rect->x;
	pad->right = aligned_rect->w - (pad->left + in_rect.w + data.dfc_w);
	pad->top = 0;
	pad->bottom = aligned_rect->h - in_rect.h;
}

static void wdfc_set_rect_align(dss_rect_t *aligned_rect, dss_rect_t in_rect,
	struct wdfc_internal_data data, dss_wb_layer_t *layer, struct rect_pad *pad)
{
	if (layer->need_cap & CAP_AFBCE) {
		compress_set_rect_align(aligned_rect, in_rect, data.dfc_w,
			(uint32_t)data.aligned_pixel, (uint32_t)data.aligned_pixel);
		compress_set_rect_pad(aligned_rect, in_rect, data.dfc_w, pad);
	} else if (layer->need_cap & CAP_HFBCE) {
		hfbce_set_rect_align(aligned_rect, in_rect, data.dfc_w, layer, pad);
	} else if (layer->need_cap & CAP_HEBCE) {
		hebce_set_rect_align(aligned_rect, in_rect, data.dfc_w, layer, pad);
	} else if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		no_compress_rot90_set_rect_align(aligned_rect, in_rect, layer, data, pad);
	} else {
		no_compress_set_rect_align(aligned_rect, in_rect, data, layer, pad);
	}
}

static int wdfc_set_internal_rect(struct dpu_fb_data_type *dpufd, dss_wb_layer_t *layer,
	dss_rect_t *ov_block_rect, dss_rect_t *in_rect)
{
	if (ov_block_rect != NULL) {
		memcpy(in_rect, ov_block_rect, sizeof(dss_rect_t));
	} else {
		*in_rect = layer->src_rect;
	}

	return 0;
}

static int wdfc_set_internel_data(
	dss_wb_layer_t *layer, dss_rect_t in_rect, struct wdfc_internal_data *data)
{
	data->aligned_pixel = (layer->need_cap & CAP_AFBCE) ? AFBC_BLOCK_ALIGN : (DMA_ALIGN_BYTES / layer->dst.bpp);

	data->size_hrz = DSS_WIDTH(in_rect.w);
	data->size_vrt = DSS_HEIGHT(in_rect.h);

	if ((data->size_hrz + 1) % 2 == 1) {  /* if size_hrz is even, +1 to odd */
		data->size_hrz += 1;
		data->dfc_w = 1;
	}

	data->dfc_aligned = 0x2;  /* dfc alignment */
	if (layer->dst.bpp <= 2) {  /* Bit Per Pixel */
		data->dfc_pix_in_num = 0x1;  /* pix num */
		data->dfc_aligned = 0x4;  /* dfc alignment */
	}

	return 0;
}

static void wdfc_config_module_struct(
	dss_dfc_t *dfc, struct wdfc_internal_data data, bool need_dither, struct rect_pad pad, int dfc_fmt)
{
	dfc->disp_size = set_bits32(dfc->disp_size, (data.size_vrt | (data.size_hrz << 16)), 32, 0);
	dfc->pix_in_num = set_bits32(dfc->pix_in_num, data.dfc_pix_in_num, 1, 0);
	dfc->disp_fmt = set_bits32(dfc->disp_fmt, dfc_fmt, 5, 1);
	dfc->clip_ctl_hrz = set_bits32(dfc->clip_ctl_hrz, 0x0, 12, 0);
	dfc->clip_ctl_vrz = set_bits32(dfc->clip_ctl_vrz, 0x0, 12, 0);
	dfc->ctl_clip_en = set_bits32(dfc->ctl_clip_en, 0x0, 1, 0);
	dfc->icg_module = set_bits32(dfc->icg_module, 0x1, 1, 0);
	if (need_dither) {
		dfc->dither_enable = set_bits32(dfc->dither_enable, 0x1, 1, 0);
		dfc->bitext_ctl = set_bits32(dfc->bitext_ctl, 0x3, 32, 0);
	} else {
		dfc->dither_enable = set_bits32(dfc->dither_enable, 0x0, 1, 0);
		dfc->bitext_ctl = set_bits32(dfc->bitext_ctl, 0x0, 32, 0);
	}

	if (pad.left || pad.right || pad.top || pad.bottom) {
		dfc->padding_ctl = set_bits32(dfc->padding_ctl, (pad.left |
			(pad.right << 8) | (pad.top << 16) | (pad.bottom << 24) | (0x1 << 31)), 32, 0);
	} else {
		dfc->padding_ctl = set_bits32(dfc->padding_ctl, 0x0, 32, 0);
	}
}

int hisi_dss_wdfc_config(struct dpu_fb_data_type *dpufd, dss_wb_layer_t *layer,
	dss_rect_t *aligned_rect, dss_rect_t *ov_block_rect)
{
	dss_dfc_t *dfc = NULL;
	dss_rect_t in_rect = {0};
	bool need_dither = false;
	int dfc_fmt;
	struct wdfc_internal_data data = {0};
	struct rect_pad pad = {0};

	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return((!layer), -EINVAL, ERR, "layer is NULL\n");
	dpu_check_and_return((!aligned_rect), -EINVAL, ERR, "aligned_rect is NULL\n");

	dfc = &(dpufd->dss_module.dfc[layer->chn_idx]);
	dpufd->dss_module.dfc_used[layer->chn_idx] = 1;

	dfc_fmt = hisi_pixel_format_hal2dfc(layer->dst.format);
	dpu_check_and_return((dfc_fmt < 0), -EINVAL, ERR, "layer format [%d] not support!\n", layer->dst.format);

	need_dither = is_need_dither(dfc_fmt);
	wdfc_set_internal_rect(dpufd, layer, ov_block_rect, &in_rect);
	wdfc_set_internel_data(layer, in_rect, &data);

	wdfc_set_rect_align(aligned_rect, in_rect, data, layer, &pad);

	wdfc_config_module_struct(dfc, data, need_dither, pad, dfc_fmt);

	if (g_debug_ovl_offline_composer)
		DPU_FB_INFO("in_rect[x_y_w_h][%d:%d:%d:%d],align_rect[x_y_w_h][%d:%d:%d:%d], "
			"pad[l_r_t_b][%d:%d:%d:%d],bpp=%d\n",
			in_rect.x, in_rect.y, in_rect.w, in_rect.h,
			aligned_rect->x, aligned_rect->y, aligned_rect->w, aligned_rect->h,
			pad.left, pad.right, pad.top, pad.bottom, layer->dst.bpp);

	return 0;
}
