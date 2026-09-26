/*
 * Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
/*lint -e559 -e574 -e676*/
#include <securec.h>
#include <drm/drm_gem_framebuffer_helper.h>

#include "hisi_drm_drv.h"
#include "hisi_drm_gem.h"
#include "hisi_overlay_utils.h"
#include "hisi_drm_dss_global.h"

#define DRM_ALIGN(value, base) (((value) + ((base) - 1)) & ~((base) - 1))
#define AFBC_PIXELS_PER_BLOCK  (16)
#define AFBC_RECT_ALIGN_PIXELS (64)
#define AFBC_HEADER_BUFFER_BYTES_PER_BLOCKENTRY (16)
#define AFBC_BODY_BUFFER_BYTE_ALIGNMENT         (PAGE_SIZE)

static const int g_rdma_stretch_threshold = RDMA_STRETCH_THRESHOLD;

struct dss_rdma_oft_pos {
	int rdma_oft_x0;
	int rdma_oft_y0;
	int rdma_oft_x1;
	int rdma_oft_y1;
};

struct dss_rdma_mask_pos {
	int rdma_mask_x0;
	int rdma_mask_y0;
	int rdma_mask_x1;
	int rdma_mask_y1;
};

struct dss_afbc_config {
	uint32_t afbc_header_addr;
	uint32_t afbc_header_stride;
	uint32_t afbc_payload_addr;
	uint32_t afbc_payload_stride;
	uint32_t afbc_header_start_pos;
	uint32_t afbc_header_offset;
	uint32_t afbc_mm_base_0;
	uint32_t afbcd_height_bf_str;
	uint32_t afbcd_half_block_mode;
	uint32_t afbcd_top_crop_num;
	uint32_t afbcd_bottom_crop_num;
};

struct dss_rdma_config {
	int rdma_format;
	int rdma_transform;
	int rdma_data_num;
	uint32_t rdma_addr;
	uint32_t rdma_stride;
	uint32_t stretch_size_vrt;
	uint32_t stretched_line_num;
	uint32_t stretched_stride;
	uint32_t l2t_interleave_n;
	bool mmu_enable;
};

static bool is_yuv_sp_420(uint32_t format)
{
	switch (format) {
	case HISI_DSS_FORMAT_YCBCR420_SP:
	case HISI_DSS_FORMAT_YCRCB420_SP:
	case HISI_DSS_FORMAT_YCBCR420_SP_10BIT:
	case HISI_DSS_FORMAT_YCRCB420_SP_10BIT:
		return true;

	default:
		return false;
	}
}

static bool is_yuv_p_422(uint32_t format)
{
	switch (format) {
	case HISI_DSS_FORMAT_YCBCR422_P:
	case HISI_DSS_FORMAT_YCRCB422_P:
	case HISI_DSS_FORMAT_YCBCR422_P_10BIT:
		return true;

	default:
		return false;
	}
}

static bool is_yuv_p_420(uint32_t format)
{
	switch (format) {
	case HISI_DSS_FORMAT_YCBCR420_P:
	case HISI_DSS_FORMAT_YCRCB420_P:
	case HISI_DSS_FORMAT_YCBCR420_P_10BIT:
		return true;

	default:
		return false;
	}
}

static bool is_pixel10bit_2_dma(int format)
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

static int hisi_pixel_format_hal2dma(int format)
{
	static const struct dss_format dma_formats_array[] = {
		{DMA_PIXEL_FORMAT_RGB_565, HISI_DSS_FORMAT_RGB565},
		{DMA_PIXEL_FORMAT_RGB_565, HISI_DSS_FORMAT_BGR565},
		{DMA_PIXEL_FORMAT_XRGB_4444, HISI_DSS_FORMAT_RGBX4444},
		{DMA_PIXEL_FORMAT_XRGB_4444, HISI_DSS_FORMAT_BGRX4444},
		{DMA_PIXEL_FORMAT_ARGB_4444, HISI_DSS_FORMAT_RGBA4444},
		{DMA_PIXEL_FORMAT_ARGB_4444, HISI_DSS_FORMAT_BGRA4444},
		{DMA_PIXEL_FORMAT_XRGB_5551, HISI_DSS_FORMAT_RGBX5551},
		{DMA_PIXEL_FORMAT_XRGB_5551, HISI_DSS_FORMAT_BGRX5551},
		{DMA_PIXEL_FORMAT_ARGB_5551, HISI_DSS_FORMAT_RGBA5551},
		{DMA_PIXEL_FORMAT_ARGB_5551, HISI_DSS_FORMAT_BGRA5551},
		{DMA_PIXEL_FORMAT_XRGB_8888, HISI_DSS_FORMAT_RGBX8888},
		{DMA_PIXEL_FORMAT_XRGB_8888, HISI_DSS_FORMAT_BGRX8888},
		{DMA_PIXEL_FORMAT_ARGB_8888, HISI_DSS_FORMAT_RGBA8888},
		{DMA_PIXEL_FORMAT_ARGB_8888, HISI_DSS_FORMAT_BGRA8888},
		{DMA_PIXEL_FORMAT_YUYV_422_Pkg, HISI_DSS_FORMAT_YUV422_I},
		{DMA_PIXEL_FORMAT_YUYV_422_Pkg, HISI_DSS_FORMAT_YUYV422_PKG},
		{DMA_PIXEL_FORMAT_YUYV_422_Pkg, HISI_DSS_FORMAT_YVYU422_PKG},
		{DMA_PIXEL_FORMAT_YUYV_422_Pkg, HISI_DSS_FORMAT_UYVY422_PKG},
		{DMA_PIXEL_FORMAT_YUYV_422_Pkg, HISI_DSS_FORMAT_VYUY422_PKG},
		{DMA_PIXEL_FORMAT_YUV_422_P_HP, HISI_DSS_FORMAT_YCBCR422_P},
		{DMA_PIXEL_FORMAT_YUV_422_P_HP, HISI_DSS_FORMAT_YCRCB422_P},
		{DMA_PIXEL_FORMAT_YUV_420_P_HP, HISI_DSS_FORMAT_YCBCR420_P},
		{DMA_PIXEL_FORMAT_YUV_420_P_HP, HISI_DSS_FORMAT_YCRCB420_P},
		{DMA_PIXEL_FORMAT_YUV_422_SP_HP, HISI_DSS_FORMAT_YCBCR422_SP},
		{DMA_PIXEL_FORMAT_YUV_422_SP_HP, HISI_DSS_FORMAT_YCRCB422_SP},
		{DMA_PIXEL_FORMAT_YUV_420_SP_HP, HISI_DSS_FORMAT_YCBCR420_SP},
		{DMA_PIXEL_FORMAT_YUV_420_SP_HP, HISI_DSS_FORMAT_YCRCB420_SP},
		{DMA_PIXEL_FORMAT_RGBA_1010102, HISI_DSS_FORMAT_RGBA1010102},
		{DMA_PIXEL_FORMAT_RGBA_1010102, HISI_DSS_FORMAT_BGRA1010102},
		{DMA_PIXEL_FORMAT_Y410_10BIT, HISI_DSS_FORMAT_Y410_10BIT},
		{DMA_PIXEL_FORMAT_YUV422_10BIT, HISI_DSS_FORMAT_YUV422_10BIT},
		{DMA_PIXEL_FORMAT_YUV420_SP_10BIT, HISI_DSS_FORMAT_YCRCB420_SP_10BIT},
		{DMA_PIXEL_FORMAT_YUV420_SP_10BIT, HISI_DSS_FORMAT_YCBCR420_SP_10BIT},
		{DMA_PIXEL_FORMAT_YUV422_SP_10BIT, HISI_DSS_FORMAT_YCBCR422_SP_10BIT},
		{DMA_PIXEL_FORMAT_YUV420_P_10BIT, HISI_DSS_FORMAT_YCBCR420_P_10BIT},
		{DMA_PIXEL_FORMAT_YUV422_P_10BIT, HISI_DSS_FORMAT_YCBCR422_P_10BIT},
	};

	int i = 0;
	for (i = 0; i < ARRAY_SIZE(dma_formats_array); i++) {
		if (dma_formats_array[i].dss_format == format) {
			return dma_formats_array[i].pixel_format;
		}
	}

	return -1;
}

static int hisi_dss_rdma_ctrl_set(dss_rdma_t *dma, struct drm_framebuffer *fb,
	struct dss_rect_ltrb *aligned_rect, struct dss_rect *out_aligned_rect)
{
	uint32_t bpp = 0;
	uint32_t format = 0;
	int rdma_format = 0;
	int aligned_pixel = 0;
	bool mmu_enable = false;
	uint32_t stretch_size_vrt = 0;
	struct dss_rdma_oft_pos rdma_oft_pos = {0, 0, 0, 0};

	drm_check_and_return(fb->format == NULL, -EINVAL, ERR, "format is NULL");

	format = dss_get_format(fb->format->format, fb->modifier);
	drm_check_and_return(format == HISI_DSS_FORMAT_UNSUPPORT,
		-EINVAL, ERR, "format not support");

	rdma_format = hisi_pixel_format_hal2dma(format);

	if (rdma_format < 0) {
		HISI_DRM_ERR("layer format %d not support !\n", format);
		return -EINVAL;
	}

	bpp = fb->format->cpp[0];
	aligned_pixel = DMA_ALIGN_BYTES / (bpp);

	rdma_oft_pos.rdma_oft_x0 = aligned_rect->left / aligned_pixel;
	rdma_oft_pos.rdma_oft_x1 = aligned_rect->right / aligned_pixel;
	stretch_size_vrt = DSS_HEIGHT(out_aligned_rect->h);

#ifdef CONFIG_DRM_DSS_IOMMU
	mmu_enable = true;
#else
	mmu_enable = false;
#endif

	dma->oft_x0 = set_bits32(dma->oft_x0, rdma_oft_pos.rdma_oft_x0, 16, 0);
	dma->oft_x1 = set_bits32(dma->oft_x1, rdma_oft_pos.rdma_oft_x1, 16, 0);
	dma->stretch_size_vrt = set_bits32(dma->stretch_size_vrt, stretch_size_vrt, 19, 0);
	dma->ctrl = set_bits32(dma->ctrl, rdma_format, 5, 3);
	dma->ctrl = set_bits32(dma->ctrl, (mmu_enable ? 0x1 : 0x0), 1, 8);
	dma->ctrl = set_bits32(dma->ctrl, DSS_TRANSFORM_NOP, 3, 9);
	dma->ctrl = set_bits32(dma->ctrl, 0, 1, 12);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 0);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 2);

	return 0;
}

static int hisi_dss_rdma_payload_set(struct drm_framebuffer *fb, uint32_t aligned_width,
	uint32_t payload_addr, uint32_t *afbc_payload_stride, uint32_t *afbc_payload_addr,
	struct dss_rect_ltrb *aligned_rect)
{
	uint32_t stride_align = 0;
	uint32_t addr_align = 0;
	uint32_t bpp = 0;

	drm_check_and_return(fb->format ==  NULL, -EINVAL, ERR, "format is NULL");

	bpp = fb->format->cpp[0];

	if (bpp == 4) {
		stride_align = AFBC_PAYLOAD_STRIDE_ALIGN_32;
		addr_align = AFBC_PAYLOAD_ADDR_ALIGN_32;
	} else if (bpp == 2) {
		stride_align = AFBC_PAYLOAD_STRIDE_ALIGN_16;
		addr_align = AFBC_PAYLOAD_ADDR_ALIGN_16;
	} else {
		HISI_DRM_ERR("bpp %d not supported!\n", bpp);
		return -EINVAL;
	}

	*afbc_payload_stride = (aligned_width / AFBC_BLOCK_ALIGN) * stride_align;

	*afbc_payload_addr = payload_addr +
		(aligned_rect->top / AFBC_BLOCK_ALIGN) * (*afbc_payload_stride) +
		(aligned_rect->left / AFBC_BLOCK_ALIGN) * stride_align;

	if ((*afbc_payload_addr & (addr_align - 1)) ||
		(*afbc_payload_stride & (stride_align - 1))) {
		HISI_DRM_ERR("afbc_payload_addr or afbc_payload_stride is not aligned");
		return -EINVAL;
	}
	return 0;
}

static int hisi_adjust_clip_rect(uint32_t transform, struct dss_rect_ltrb *clip_rect)
{
	int ret = 0;
	uint32_t temp = 0;

	if ((clip_rect->left < 0 || clip_rect->left > DFC_MAX_CLIP_NUM) ||
		(clip_rect->right < 0 || clip_rect->right > DFC_MAX_CLIP_NUM) ||
		(clip_rect->top < 0 || clip_rect->top > DFC_MAX_CLIP_NUM) ||
		(clip_rect->bottom < 0 || clip_rect->bottom > DFC_MAX_CLIP_NUM)) {
		HISI_DRM_ERR("clip rect invalid => clip_rect %d, %d, %d, %d\n",
				clip_rect->left, clip_rect->right, clip_rect->top, clip_rect->bottom);
		return -EINVAL;
	}

	switch (transform) {
	case DRM_MODE_ROTATE_0:
		// do nothing
		break;
	case DRM_MODE_REFLECT_X:
	case DRM_MODE_ROTATE_90:
		{
			temp = clip_rect->left;
			clip_rect->left = clip_rect->right;
			clip_rect->right = temp;
		}
		break;
	case DRM_MODE_REFLECT_Y:
	case DRM_MODE_ROTATE_270:
		{
			temp = clip_rect->top;
			clip_rect->top = clip_rect->bottom;
			clip_rect->bottom = temp;
		}
		break;
	case DRM_MODE_ROTATE_180:
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
		HISI_DRM_ERR("not supported this transform %d!", transform);
		break;
	}

	return ret;
}

static int drm_transform_hal2dma(int transform, int chn_idx)
{
	int ret = 0;

	if (chn_idx < DSS_WCHN_W0 || chn_idx == DSS_RCHN_V2) {
		switch (transform) {
		case DRM_MODE_ROTATE_0:
			ret = DSS_TRANSFORM_NOP;
			break;
		case DRM_MODE_REFLECT_X:
			ret = DSS_TRANSFORM_FLIP_H;
			break;
		case DRM_MODE_REFLECT_Y:
			ret = DSS_TRANSFORM_FLIP_V;
			break;
		case DRM_MODE_ROTATE_180:
			ret = DSS_TRANSFORM_FLIP_V | DSS_TRANSFORM_FLIP_H;
			break;
		case DRM_MODE_ROTATE_90:
			ret = DSS_TRANSFORM_ROT | DSS_TRANSFORM_FLIP_H;
			break;
		case DRM_MODE_ROTATE_270:
			ret = DSS_TRANSFORM_ROT | DSS_TRANSFORM_FLIP_V;
			break;
		default:
			ret = -1;
			HISI_DRM_ERR("Transform %d is not supported, chn_idx=%d",
				transform, chn_idx);
			break;
		}
	} else {
		if (transform == DRM_MODE_ROTATE_0) {
			ret = DSS_TRANSFORM_NOP;
		} else if (transform == (DRM_MODE_ROTATE_90 | DRM_MODE_REFLECT_Y)) {
			ret = DSS_TRANSFORM_ROT;
		} else {
			ret = -1;
			HISI_DRM_ERR("Transform %d is not supported, chn_idx=%d",
				transform, chn_idx);
		}
	}

	return ret;
}

static int need_rdma_stretch_blt(struct hisi_drm_plane_state *hisi_state, uint32_t *v_stretch_ratio)
{
	uint32_t format;
	uint32_t v_stretch_ratio_threshold = 0;
	struct dss_rect src_rect;
	struct dss_rect dst_rect;
	struct drm_plane_state *state = NULL;

	state = &(hisi_state->base);

	dst_rect.x = state->crtc_x;
	dst_rect.y = state->crtc_y;
	dst_rect.w = state->crtc_w;
	dst_rect.h = state->crtc_h;
	src_rect.x = state->src_x >> 16;
	src_rect.y = state->src_y >> 16;
	src_rect.w = state->src_w >> 16;
	src_rect.h = state->src_h >> 16;

	drm_check_and_return(state->fb == NULL, -EINVAL, ERR, "fb is NULL");
	drm_check_and_return(state->fb->format == NULL, -EINVAL, ERR, "format is NULL");

	format = dss_get_format(state->fb->format->format, state->fb->modifier);
	drm_check_and_return(format == HISI_DSS_FORMAT_UNSUPPORT,
		-EINVAL, ERR, "format not support");

	*v_stretch_ratio = 0;
	if ((hisi_state->need_cap & CAP_AFBCD) || (hisi_state->need_cap & CAP_HFBCD)) {
		*v_stretch_ratio = 0;
	} else {
		if (is_yuv_sp_420(format) || is_yuv_p_420(format)) {
			v_stretch_ratio_threshold = ((src_rect.h + dst_rect.h - 1) / dst_rect.h);
			*v_stretch_ratio = ((src_rect.h / dst_rect.h) / 2) * 2;
		} else {
			v_stretch_ratio_threshold = ((src_rect.h + dst_rect.h - 1) / dst_rect.h);
			*v_stretch_ratio = (src_rect.h / dst_rect.h);
		}

		if (v_stretch_ratio_threshold <= g_rdma_stretch_threshold)
			*v_stretch_ratio = 0;
	}

	return 0;
}

static int hisi_dss_rdma_stretch(struct drm_plane_state *state, int chn_idx,
	uint32_t *afbcd_half_block_mode)
{
	int rdma_transform = drm_transform_hal2dma(state->rotation, chn_idx);

	if (rdma_transform < 0) {
		HISI_DRM_ERR("state rotation=%d not support!\n", state->rotation);
		return -EINVAL;
	}
	if ((uint32_t)rdma_transform & DSS_TRANSFORM_ROT) {
		*afbcd_half_block_mode = ((uint32_t)rdma_transform & DSS_TRANSFORM_FLIP_H) ?
			AFBC_HALF_BLOCK_LOWER_UPPER_ALL : AFBC_HALF_BLOCK_UPPER_LOWER_ALL;
	} else {
		*afbcd_half_block_mode = ((uint32_t)rdma_transform & DSS_TRANSFORM_FLIP_V) ?
			AFBC_HALF_BLOCK_LOWER_UPPER_ALL : AFBC_HALF_BLOCK_UPPER_LOWER_ALL;
	}

	return 0;
}

static void his_dss_rdma_afbcd_crop(struct dss_rect_ltrb *clip_rect,
	uint32_t *afbcd_top_crop_num, uint32_t *afbcd_bottom_crop_num)
{
	*afbcd_top_crop_num = (clip_rect->top > AFBCD_TOP_CROP_MAX) ?
		AFBCD_TOP_CROP_MAX : clip_rect->top;
	*afbcd_bottom_crop_num = (clip_rect->bottom > AFBCD_BOTTOM_CROP_MAX) ?
		AFBCD_BOTTOM_CROP_MAX : clip_rect->bottom;

	clip_rect->top -= *afbcd_top_crop_num;

	clip_rect->bottom -= *afbcd_bottom_crop_num;

	return;
}

static unsigned long __map_iova(struct hisi_drm_gem *hisi_gem)
{
	struct drm_device *drm_dev = hisi_gem->base.dev;
	unsigned long buf_size;

	hisi_gem->iova = hisi_iommu_map_sg(to_dma_dev(drm_dev),
					   hisi_gem->sgt->sgl, 0, &buf_size);
	if (!hisi_gem->iova) {
		HISI_DRM_ERR("failed to iommu map sg!\n");
		return 0;
	}

	return hisi_gem->iova;
}

static uint32_t get_rdma_addr(struct drm_plane_state *state)
{
	uint32_t stride;
	uint32_t rdma_addr = 0;
	struct hisi_drm_gem *hisi_gem = NULL;

#ifdef CONFIG_DRM_DSS_IOMMU
	struct drm_gem_object *obj = NULL;
#else
	struct drm_gem_cma_object *obj = NULL;
#endif

	drm_check_and_return(state->fb == NULL, rdma_addr, ERR, "state fb is NULL");

	stride = state->fb->pitches[0];

#ifdef CONFIG_DRM_DSS_IOMMU
	obj = drm_gem_fb_get_obj(state->fb, 0);
	drm_check_and_return(obj == NULL, rdma_addr, ERR, "get obj Fail");

	hisi_gem = to_hisi_gem(obj);

	if (hisi_gem->iova) {
		rdma_addr = hisi_gem->iova;
	} else {
		HISI_DRM_WARN("failed to get iova from gem, plz check\n");
		HISI_DRM_WARN("we will map iova to it\n");
		rdma_addr = __map_iova(hisi_gem);
	}

#else
	obj = drm_fb_cma_get_gem_obj(state->fb, 0);
	rdma_addr = (uint32_t)obj->paddr + state->src_y * state->fb->pitches[0];
#endif

	return rdma_addr;
}

static int hisi_get_rdma_tile_interleave(uint32_t stride)
{
	int i = 0;
	uint32_t interleave[MAX_TILE_SURPORT_NUM] = {256, 512, 1024, 2048, 4096, 8192};

	for (i = 0; i < MAX_TILE_SURPORT_NUM; i++) {
		if (interleave[i] == stride)
			return MIN_INTERLEAVE + i;
	}

	return 0;
}

static int hisi_dss_rdma_afbc_aligned(struct drm_plane_state *state, int crtc_id,
	uint32_t *aligned_width, uint32_t *afbc_header_addr, uint32_t *afbc_payload_addr)
{
	uint32_t w_blocks = 0;
	uint32_t h_blocks = 0;
	uint32_t header_stride = 0;
	struct hisi_afbc_aligned_info *aligned_info = NULL;
	struct drm_framebuffer *fb = NULL;
	uint32_t src_addr;

	static struct hisi_afbc_aligned_info afbc_aligned_info[CRTC_MAX_IDX] = {{0}};

	if (crtc_id >= CRTC_MAX_IDX) {
		HISI_DRM_ERR("crtc_id=%d not supported\n", crtc_id);
		return -EINVAL;
	}
	fb = state->fb;
	aligned_info = &afbc_aligned_info[crtc_id];

	if (fb->width != aligned_info->width || fb->height != aligned_info->height) {
		aligned_info->width = fb->width;
		aligned_info->height = fb->height;

		aligned_info->aligned_width = DRM_ALIGN(aligned_info->width,
			AFBC_RECT_ALIGN_PIXELS);
		aligned_info->aligned_height = DRM_ALIGN(aligned_info->height,
			AFBC_PIXELS_PER_BLOCK);

		w_blocks = aligned_info->aligned_width / AFBC_PIXELS_PER_BLOCK;
		h_blocks = aligned_info->aligned_height / AFBC_PIXELS_PER_BLOCK;
		header_stride = DRM_ALIGN(w_blocks *
			AFBC_HEADER_BUFFER_BYTES_PER_BLOCKENTRY, AFBC_RECT_ALIGN_PIXELS);

		aligned_info->header_size = DRM_ALIGN(header_stride *
			h_blocks, AFBC_BODY_BUFFER_BYTE_ALIGNMENT);
	}

	src_addr = get_rdma_addr(state);

	*aligned_width = aligned_info->aligned_width;
	*afbc_header_addr = src_addr;
	*afbc_payload_addr = src_addr + aligned_info->header_size;

	return 0;
}

static int hisi_calculate_display_addr(struct drm_plane_state *state, struct dss_rect_ltrb aligned_rect,
	uint32_t *rdma_addr, uint32_t *l2t_interleave_n)
{
	uint32_t src_addr = 0;
	uint32_t stride = 0;
	int bpp = 0;
	struct hisi_drm_plane_state *hisi_state = NULL;

	stride = state->fb->pitches[0];
	bpp = state->fb->format->cpp[0];
	src_addr = get_rdma_addr(state);

	if (src_addr == 0) {
		HISI_DRM_ERR("rdma_addr(0x%x) is zero\n", *rdma_addr);
		return -EINVAL;
	}

	if (src_addr & (DMA_ADDR_ALIGN - 1)) {
		HISI_DRM_ERR("rdma_addr(0x%x) is not %d bytes aligned.\n",
			src_addr, DMA_ADDR_ALIGN);
		return -EINVAL;
	}

	if (stride & (DMA_STRIDE_ALIGN - 1)) {
		HISI_DRM_ERR("stride(0x%x) is not %d bytes aligned.\n",
			stride, DMA_STRIDE_ALIGN);
		return -EINVAL;
	}

	hisi_state= to_hisi_plane_state(state);
	if (hisi_state->need_cap & CAP_TILE) {
		*l2t_interleave_n = hisi_get_rdma_tile_interleave(stride);
		if (*l2t_interleave_n < MIN_INTERLEAVE) {
			HISI_DRM_ERR("tile stride should be 256*2^n, error stride:%d!\n", stride);
			return -EINVAL;
		}

		if (src_addr & (TILE_DMA_ADDR_ALIGN - 1)) {
			HISI_DRM_ERR("tile rdma_addr(0x%x) is not %d bytes aligned.\n",
				src_addr, TILE_DMA_ADDR_ALIGN);
			return -EINVAL;
		}
	}

	switch (state->rotation) {
	case DRM_MODE_ROTATE_0:
		*rdma_addr = src_addr + aligned_rect.top * stride + aligned_rect.left * bpp;
		break;
	case DRM_MODE_REFLECT_X:
		*rdma_addr = src_addr + aligned_rect.top * stride + aligned_rect.right * bpp;
		break;
	case DRM_MODE_REFLECT_Y:
		*rdma_addr = src_addr + aligned_rect.bottom * stride + aligned_rect.left * bpp;
		break;
	case DRM_MODE_ROTATE_180:
		*rdma_addr = src_addr + aligned_rect.bottom * stride + aligned_rect.right * bpp;
		break;
	default:
		HISI_DRM_ERR("not supported this transform %d!", state->rotation);
		break;
	}

	return 0;
}

static int hisi_dss_rdma_set_oft_pos(struct dss_rect_ltrb aligned_rect, int aligned_pixel,
	struct dss_rdma_oft_pos *rdma_oft_pos)
{
	rdma_oft_pos->rdma_oft_y0 = aligned_rect.top;
	rdma_oft_pos->rdma_oft_y1 = aligned_rect.bottom;
	if (aligned_pixel == 0) {
		HISI_DRM_ERR("aligned_pixel invalid!");
		return -EINVAL;
	}
	rdma_oft_pos->rdma_oft_x0 = aligned_rect.left / aligned_pixel;
	rdma_oft_pos->rdma_oft_x1 = aligned_rect.right / aligned_pixel;

	if ((rdma_oft_pos->rdma_oft_x1 - rdma_oft_pos->rdma_oft_x0) < 0 ||
		(rdma_oft_pos->rdma_oft_x1 - rdma_oft_pos->rdma_oft_x0 + 1) > DMA_IN_WIDTH_MAX) {
		HISI_DRM_ERR("out of range, rdma_oft_x0 = %d, rdma_oft_x1 = %d!\n",
			rdma_oft_pos->rdma_oft_x0, rdma_oft_pos->rdma_oft_x1);
		return -EINVAL;
	}

	if ((rdma_oft_pos->rdma_oft_y1 - rdma_oft_pos->rdma_oft_y0) < 0 ||
		(rdma_oft_pos->rdma_oft_y1 - rdma_oft_pos->rdma_oft_y0 + 1) > DMA_IN_HEIGHT_MAX) {
		HISI_DRM_ERR("out of range, rdma_oft_y0 = %d, rdma_oft_y1 = %d\n",
			rdma_oft_pos->rdma_oft_y0, rdma_oft_pos->rdma_oft_y1);
		return -EINVAL;
	}
	return 0;
}

static int hisi_dss_rdma_aligned_rect(struct drm_plane_state *state, int *aligned_pixel,
	struct dss_rect_ltrb *aligned_rect, struct dss_rect *out_aligned_rect, struct dss_rect_ltrb *clip_rect)
{
	int ret = 0;
	int bpp = 0;
	uint32_t format = 0;
	int rdma_format = 0;
	struct drm_framebuffer *fb = NULL;
	struct dss_rect src_rect;

	src_rect.x = state->src_x >> 16;
	src_rect.y = state->src_y >> 16;
	src_rect.w = state->src_w >> 16;
	src_rect.h = state->src_h >> 16;

	fb = state->fb;
	drm_check_and_return(fb == NULL, -EINVAL, ERR, "fb is null");
	drm_check_and_return(fb->format == NULL, -EINVAL, ERR, "format is null");

	format = dss_get_format(fb->format->format, fb->modifier);
	drm_check_and_return(format == HISI_DSS_FORMAT_UNSUPPORT,
		-EINVAL, ERR, "format not support");

	bpp = fb->format->cpp[0];
	drm_check_and_return(bpp == 0, -EINVAL, ERR, "bpp is 0");

	rdma_format = hisi_pixel_format_hal2dma(format);
	drm_check_and_return(rdma_format < 0, -EINVAL, ERR, "layer format not support");

	if (!is_pixel10bit_2_dma(rdma_format) && (check_yuv_semi_planar(format) || check_yuv_planar(format)))
		bpp = 1;

	*aligned_pixel = DMA_ALIGN_BYTES / (bpp);

	if (is_yuv_p_420(format) || is_yuv_p_422(format)) {
		aligned_rect->left = ALIGN_DOWN((uint32_t)src_rect.x,
			2 * (*aligned_pixel));
		aligned_rect->right = ALIGN_UP((uint32_t)(src_rect.x + src_rect.w),
			2 * (*aligned_pixel)) - 1;
	} else {
		aligned_rect->left = ALIGN_DOWN((uint32_t)src_rect.x, (*aligned_pixel));
		aligned_rect->right = ALIGN_UP((uint32_t)(src_rect.x + src_rect.w),
			(*aligned_pixel)) - 1;
	}

	if (is_yuv_sp_420(format) || is_yuv_p_420(format)) {
		aligned_rect->top = ALIGN_DOWN((uint32_t)src_rect.y, 2);
		aligned_rect->bottom = ALIGN_UP((uint32_t)(src_rect.y + src_rect.h), 2) - 1;
	} else {
		aligned_rect->top = src_rect.y;
		aligned_rect->bottom = DSS_HEIGHT(src_rect.y + src_rect.h);
	}

	// out_rect
	out_aligned_rect->x = 0;
	out_aligned_rect->y = 0;
	out_aligned_rect->w = aligned_rect->right - aligned_rect->left + 1;
	out_aligned_rect->h = aligned_rect->bottom - aligned_rect->top + 1;

	// clip_rect
	clip_rect->left = src_rect.x - aligned_rect->left;
	clip_rect->right = aligned_rect->right - DSS_WIDTH(src_rect.x + src_rect.w);
	clip_rect->top = src_rect.y - aligned_rect->top;
	clip_rect->bottom = aligned_rect->bottom - DSS_HEIGHT(src_rect.y + src_rect.h);

	ret = hisi_adjust_clip_rect(state->rotation, clip_rect);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_adjust_clip_rect failed");

	return 0;
}

static void hisi_dss_rdma_set_dpp_and_dma(struct dss_module_reg *dss_module,
	struct drm_plane_state *state, int chn_idx)
{
	dss_rdma_t *dma = NULL;
	struct hisi_drm_plane_state *hisi_state = NULL;

	dma = &(dss_module->rdma[chn_idx]);

	hisi_state = to_hisi_plane_state(state);
	if (hisi_state->need_cap & CAP_YUV_DEINTERLACE) {
		dma->vpp_used = 1;

		if (state->rotation & DRM_MODE_ROTATE_90)
			dma->vpp_ctrl = set_bits32(dma->vpp_ctrl, 0x2, 2, 0);
		else
			dma->vpp_ctrl = set_bits32(dma->vpp_ctrl, 0x3, 2, 0);
	}
}

static int hisi_prepare_afbc_rect(struct drm_plane_state *state, struct dss_rect_ltrb *aligned_rect,
	struct dss_rect *out_aligned_rect, struct dss_rect_ltrb *clip_rect)
{
	int ret = 0;
	uint32_t src_bottom = 0;
	uint32_t src_right = 0;
	struct dss_rect src_rect = {0, 0, 0, 0};

	src_rect.x = state->src_x >> 16;
	src_rect.y = state->src_y >> 16;
	src_rect.w = state->src_w >> 16;
	src_rect.h = state->src_h >> 16;

	src_bottom = (uint32_t)src_rect.y + src_rect.h;
	src_right = (uint32_t)src_rect.x + src_rect.w;

	aligned_rect->left = ALIGN_DOWN((uint32_t)src_rect.x, AFBC_BLOCK_ALIGN);
	aligned_rect->right = ALIGN_UP(src_right, AFBC_BLOCK_ALIGN) - 1;
	aligned_rect->top = ALIGN_DOWN((uint32_t)src_rect.y, AFBC_BLOCK_ALIGN);
	aligned_rect->bottom = ALIGN_UP(src_bottom, AFBC_BLOCK_ALIGN) - 1;

	// out_aligned_rect
	out_aligned_rect->x = 0;
	out_aligned_rect->y = 0;
	out_aligned_rect->w = aligned_rect->right - aligned_rect->left + 1;
	out_aligned_rect->h = aligned_rect->bottom - aligned_rect->top + 1;

	// rdfc clip_rect
	clip_rect->left = src_rect.x - aligned_rect->left;
	clip_rect->right = aligned_rect->right - DSS_WIDTH(src_right);
	clip_rect->top = src_rect.y - aligned_rect->top;
	clip_rect->bottom = aligned_rect->bottom - DSS_HEIGHT(src_bottom);

	ret = hisi_adjust_clip_rect(state->rotation, clip_rect);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_adjust_clip_rect failed");

	return 0;
}

static int hisi_prepare_afbc_buffer_config(struct drm_plane_state *state, struct dss_rect_ltrb *aligned_rect,
	struct dss_afbc_config *config)
{
	int ret = 0;
	uint32_t aligned_width = 0;
	uint32_t aligned_header_addr = 0;
	uint32_t aligned_payload_addr = 0;
	struct drm_framebuffer *fb = NULL;
	struct hisi_drm_plane *hisi_plane = NULL;
	struct hisi_drm_crtc *hisi_crtc = NULL;

	hisi_plane = to_hisi_drm_plane(state->plane);
	drm_check_and_return(hisi_plane == NULL, -EINVAL, ERR, "hisi_plane is null");

	hisi_crtc = to_hisi_drm_crtc(state->crtc);
	drm_check_and_return(hisi_crtc == NULL, -EINVAL, ERR, "hisi_crtc is null");

	fb = state->fb;
	drm_check_and_return(fb == NULL, -EINVAL, ERR, "fb is null");

	ret = hisi_dss_rdma_afbc_aligned(state, hisi_crtc->crtc_id, &aligned_width, &aligned_header_addr, &aligned_payload_addr);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_rdma_afbc_aligned failed");

	HISI_DRM_DEBUG("AFBC aligned_width=%d", aligned_width);

	// header
	config->afbc_header_stride = (aligned_width / AFBC_BLOCK_ALIGN) *
		AFBC_HEADER_STRIDE_BLOCK;
	config->afbc_header_offset = (aligned_rect->top / AFBC_BLOCK_ALIGN) *
		config->afbc_header_stride + (aligned_rect->left / AFBC_BLOCK_ALIGN) *
		AFBC_HEADER_STRIDE_BLOCK;
	config->afbc_header_addr = aligned_header_addr + config->afbc_header_offset;
	if ((config->afbc_header_addr & (AFBC_HEADER_ADDR_ALIGN - 1)) ||
		(config->afbc_header_stride & (AFBC_HEADER_STRIDE_ALIGN - 1))) {
		HISI_DRM_ERR("afbc_header_addr or afbc_header_stride is not bytes aligned");
		return -EINVAL;
	}

	// payload
	ret = hisi_dss_rdma_payload_set(fb, aligned_width, aligned_payload_addr,
			&(config->afbc_payload_stride), &(config->afbc_payload_addr), aligned_rect);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_rdma_payload_set failed");

	config->afbc_mm_base_0 = hisi_plane->mmbuffer_addr - MMBUF_BASE;

	return 0;
}

static int hisi_dss_afbc_rdma_config(struct dss_module_reg *dss_module,
	struct hisi_drm_plane_state *hisi_state, int chn_idx, struct dss_rect_ltrb *clip_rect,
	struct dss_rect *out_aligned_rect)
{
	int ret = 0;
	struct drm_framebuffer *fb = NULL;
	struct dss_rect_ltrb aligned_rect;
	struct dss_afbc_config config;
	struct drm_plane_state *state = NULL;
	dss_rdma_t *dma = NULL;

	ret = memset_s(&aligned_rect, sizeof(aligned_rect), 0x00, sizeof(aligned_rect));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset failed!");
	ret = memset_s(&config, sizeof(config), 0x00, sizeof(config));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset failed!");

	state = &(hisi_state->base);

	fb = state->fb;
	drm_check_and_return(fb == NULL, -EINVAL, ERR, "fb is null");

	// stretch
	ret = hisi_dss_rdma_stretch(state, chn_idx, &config.afbcd_half_block_mode);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_rdma_stretch failed");

	// rect
	ret = hisi_prepare_afbc_rect(state, &aligned_rect, out_aligned_rect, clip_rect);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_prepare_afbc_rect failed");

	// afbcd crop
	his_dss_rdma_afbcd_crop(clip_rect, &config.afbcd_top_crop_num, &config.afbcd_bottom_crop_num);

	// calculate buffer header/payload configure
	ret = hisi_prepare_afbc_buffer_config(state, &aligned_rect, &config);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_prepare_afbc_block_config failed");

	// adjust out_aligned_rect
	out_aligned_rect->h -= (config.afbcd_top_crop_num + config.afbcd_bottom_crop_num);

	config.afbcd_height_bf_str = aligned_rect.bottom - aligned_rect.top + 1;

	dss_module->dma_used[chn_idx] = 1;
	dma = &(dss_module->rdma[chn_idx]);

	dma->afbc_used = 1;
	dma->afbcd_hreg_pic_width = set_bits32(dma->afbcd_hreg_pic_width,
		(aligned_rect.right - aligned_rect.left), 16, 0);
	dma->afbcd_hreg_pic_height = set_bits32(dma->afbcd_hreg_pic_height,
		(aligned_rect.bottom - aligned_rect.top), 16, 0);

	// block split mode
	dma->afbcd_hreg_format = set_bits32(dma->afbcd_hreg_format, 0x1, 1, 0);
	// color transform
	dma->afbcd_hreg_format = set_bits32(dma->afbcd_hreg_format, 0x1, 1, 21);
	dma->afbcd_ctl = set_bits32(dma->afbcd_ctl, config.afbcd_half_block_mode, 2, 6);
	dma->afbcd_str = set_bits32(dma->afbcd_str, 0, 12, 0);
	dma->afbcd_line_crop = set_bits32(dma->afbcd_line_crop,
		(config.afbcd_top_crop_num << 8 | config.afbcd_bottom_crop_num), 16, 0);
	dma->afbcd_hreg_hdr_ptr_lo = set_bits32(dma->afbcd_hreg_hdr_ptr_lo, config.afbc_header_addr, 32, 0);
	dma->afbcd_input_header_stride = set_bits32(dma->afbcd_input_header_stride,
		(config.afbc_header_start_pos << 14) | config.afbc_header_stride, 16, 0);
	dma->afbcd_payload_stride = set_bits32(dma->afbcd_payload_stride, config.afbc_payload_stride, 20, 0);
	dma->afbcd_mm_base_0 = set_bits32(dma->afbcd_mm_base_0, config.afbc_mm_base_0, 32, 0);
	dma->afbcd_afbcd_payload_pointer = set_bits32(dma->afbcd_afbcd_payload_pointer,
		config.afbc_payload_addr, 32, 0);
	dma->afbcd_height_bf_str = set_bits32(dma->afbcd_height_bf_str, DSS_HEIGHT(config.afbcd_height_bf_str), 16, 0);
	dma->afbcd_header_pointer_offset = set_bits32(dma->afbcd_header_pointer_offset,
		config.afbc_header_offset, 32, 0);

	// afbcd_scramble_mode: ddr not support scramble_mode in linux
	dma->afbcd_scramble_mode = set_bits32(dma->afbcd_scramble_mode, DSS_AFBC_SCRAMBLE_NONE, 2, 0);

	ret = hisi_dss_rdma_ctrl_set(dma, fb, &aligned_rect, out_aligned_rect);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_rdma_ctrl_set failed");

	return 0;
}

static int hisi_dss_rdma_set_stretch_config(struct hisi_drm_plane_state *hisi_state,
	struct dss_rdma_oft_pos rdma_oft_pos, struct dss_rect *out_aligned_rect,
	struct dss_rdma_config *config, bool *rdma_stretch_enable)
{
	int ret;
	struct drm_plane_state *state = NULL;

	state = &(hisi_state->base);
	config->rdma_stride = state->fb->pitches[0];
	config->rdma_data_num = (rdma_oft_pos.rdma_oft_x1 - rdma_oft_pos.rdma_oft_x0 + 1) *
		(rdma_oft_pos.rdma_oft_y1 - rdma_oft_pos.rdma_oft_y0 + 1);

	ret = need_rdma_stretch_blt(hisi_state, &config->stretched_line_num);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "need_rdma_stretch_blt fail");
	if (config->stretched_line_num > 0) {
		config->stretch_size_vrt = (out_aligned_rect->h / config->stretched_line_num) +
			((out_aligned_rect->h % config->stretched_line_num) ? 1 : 0) - 1;
		out_aligned_rect->h = config->stretch_size_vrt + 1;
		config->stretched_stride = config->stretched_line_num * config->rdma_stride / DMA_ALIGN_BYTES;
		config->rdma_data_num = (config->stretch_size_vrt + 1) *
			(rdma_oft_pos.rdma_oft_x1 - rdma_oft_pos.rdma_oft_x0 + 1);
		*rdma_stretch_enable = true;
	} else {
		config->stretch_size_vrt = rdma_oft_pos.rdma_oft_y1 - rdma_oft_pos.rdma_oft_y0;
		config->stretched_line_num = 0x0;
		config->stretched_stride = 0x0;
		*rdma_stretch_enable = false;
	}

#ifdef CONFIG_DRM_DSS_IOMMU
	config->mmu_enable = true;
#else
	config->mmu_enable = false;
#endif

	return ret;
}

static int hisi_dss_rdma_set_format_and_transform(struct drm_plane_state *state, int chn_idx,
	struct dss_rdma_config *config)
{
	uint32_t format;
	struct drm_framebuffer *fb = NULL;

	fb = state->fb;
	drm_check_and_return(fb == NULL, -EINVAL, ERR, "fb is null");
	drm_check_and_return(fb->format == NULL, -EINVAL, ERR, "format is null");

	format = dss_get_format(fb->format->format, fb->modifier);
	drm_check_and_return(format == HISI_DSS_FORMAT_UNSUPPORT,
		-EINVAL, ERR, "format not support");

	config->rdma_format = hisi_pixel_format_hal2dma(format);
	drm_check_and_return(config->rdma_format < 0, -EINVAL, ERR, "layer format not support");

	config->rdma_transform = drm_transform_hal2dma(state->rotation, chn_idx);
	drm_check_and_return(config->rdma_transform < 0, -EINVAL, ERR, "layer transform not support");

	return 0;
}

static int hisi_dss_normal_rdma_config(struct dss_module_reg *dss_module,
	struct hisi_drm_plane_state *hisi_state, int chn_idx, int ovl_idx,
	struct dss_rect_ltrb *clip_rect, struct dss_rect *out_aligned_rect,
	bool *rdma_stretch_enable)
{
	int ret;
	int aligned_pixel = 0;
	struct dss_rect_ltrb aligned_rect;
	dss_rdma_t *dma = NULL;
	struct drm_plane_state *state = NULL;
	struct dss_rdma_config config;
	struct dss_rdma_oft_pos rdma_oft_pos;

	ret = memset_s(&config, sizeof(config), 0x00, sizeof(config));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset fail for config");
	ret = memset_s(&aligned_rect, sizeof(aligned_rect), 0x00, sizeof(aligned_rect));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset fail for aligned rect");
	ret = memset_s(&rdma_oft_pos, sizeof(rdma_oft_pos), 0x00, sizeof(rdma_oft_pos));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset fail for rdma oft pos");

	drm_check_and_return((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX),
		-EINVAL, ERR, "ovl_idx is invalid");

	state = &(hisi_state->base);

	// hisi_dss_rdma_set_dpp_and_dma
	hisi_dss_rdma_set_dpp_and_dma(dss_module, state, chn_idx);

	ret = hisi_dss_rdma_set_format_and_transform(state, chn_idx, &config);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_rdma_set_format_and_transform failed");

	// aligned_rect
	ret = hisi_dss_rdma_aligned_rect(state, &aligned_pixel, &aligned_rect, out_aligned_rect, clip_rect);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_rdma_aligned_rect failed");

	ret = hisi_dss_rdma_set_oft_pos(aligned_rect, aligned_pixel, &rdma_oft_pos);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_rdma_oft_pos failed");

	ret = hisi_calculate_display_addr(state, aligned_rect, &config.rdma_addr, &config.l2t_interleave_n);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_calculate_display_addr failed");

	ret = hisi_dss_rdma_set_stretch_config(hisi_state, rdma_oft_pos, out_aligned_rect, &config, rdma_stretch_enable);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_rdma_set_stretch_config failed");

	dss_module->dma_used[chn_idx] = 1;
	dma = &(dss_module->rdma[chn_idx]);
	dma->oft_x0 = set_bits32(dma->oft_x0, rdma_oft_pos.rdma_oft_x0, 16, 0);
	dma->oft_y0 = set_bits32(dma->oft_y0, rdma_oft_pos.rdma_oft_y0, 16, 0);
	dma->oft_x1 = set_bits32(dma->oft_x1, rdma_oft_pos.rdma_oft_x1, 16, 0);
	dma->oft_y1 = set_bits32(dma->oft_y1, rdma_oft_pos.rdma_oft_y1, 16, 0);
	dma->mask0 = set_bits32(dma->mask0, 0, 32, 0);
	dma->mask1 = set_bits32(dma->mask1, 0, 32, 0);
	dma->stretch_size_vrt = set_bits32(dma->stretch_size_vrt, (config.stretch_size_vrt | (config.stretched_line_num << 13)), 19, 0);
	dma->ctrl = set_bits32(dma->ctrl, ((hisi_state->need_cap & CAP_TILE) ? 0x1 : 0x0), 1, 1);
	dma->ctrl = set_bits32(dma->ctrl, config.rdma_format, 5, 3);
	dma->ctrl = set_bits32(dma->ctrl, (config.mmu_enable ? 0x1 : 0x0), 1, 8);
	dma->ctrl = set_bits32(dma->ctrl, config.rdma_transform, 3, 9);
	dma->ctrl = set_bits32(dma->ctrl, ((config.stretched_line_num > 0) ? 0x1 : 0x0), 1, 12);
	dma->ctrl = set_bits32(dma->ctrl, 0x0, 1, 17);
	dma->tile_scram = set_bits32(dma->tile_scram, ((hisi_state->need_cap & CAP_TILE) ? 0x1 : 0x0), 1, 0);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 0);

	dma->data_addr0 = set_bits32(dma->data_addr0, config.rdma_addr, 32, 0);

	dma->stride0 = set_bits32(dma->stride0, ((config.rdma_stride / DMA_ALIGN_BYTES) | (config.l2t_interleave_n << 16)), 20, 0);
	dma->stretch_stride0 = set_bits32(dma->stretch_stride0, config.stretched_stride, 19, 0);
	dma->data_num0 = set_bits32(dma->data_num0, config.rdma_data_num, 30, 0);

	HISI_DRM_DEBUG("rdma_transform=%d, rdma_addr=0x%lx", config.rdma_transform, config.rdma_addr);

	return 0;
}

int hisi_dss_rdma_config(struct dss_module_reg *dss_module,
	struct hisi_drm_plane_state *hisi_state, int chn_idx, int ovl_idx,
	struct dss_rect_ltrb *clip_rect, struct dss_rect *aligned_rect,
	bool *rdma_stretch_enable)
{
	int ret = 0;
	struct drm_plane_state *state = NULL;
	struct drm_framebuffer *fb = NULL;

	drm_check_and_return(dss_module == NULL, -EINVAL, ERR, "dss_module is null");

	drm_check_and_return(hisi_state == NULL, -EINVAL, ERR, "hisi_state is null");

	state = &(hisi_state->base);
	fb = state->fb;
	drm_check_and_return(fb == NULL, -EINVAL, ERR, "fb is null");

	if (fb->modifier == AFBC_FORMAT_MODIFIER) {
		ret = hisi_dss_afbc_rdma_config(dss_module, hisi_state, chn_idx, clip_rect,
			aligned_rect);
	} else {
		ret = hisi_dss_normal_rdma_config(dss_module, hisi_state, chn_idx, ovl_idx,
			clip_rect, aligned_rect, rdma_stretch_enable);
	}
	drm_check_and_return(ret != 0, -EINVAL, ERR, "dss rdma config failed");

	HISI_DRM_DEBUG("rdma output, clip_rect %d,%d,%d,%d, aligned_rect %d,%d,%d,%d",
		clip_rect->left, clip_rect->right, clip_rect->top, clip_rect->bottom, aligned_rect->x,
		aligned_rect->y, aligned_rect->w, aligned_rect->h);

	return 0;
}

/*lint +e559 +e574 +e676*/
