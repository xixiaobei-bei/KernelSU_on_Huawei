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

enum hebce_rect_align_type {
	ROT90_YUV = 0,
	ROT90_NO_YUV,
	ROT0_YUV,
	ROT0_NO_YUV,
	ALIGN_TYPE_MAX
};

struct hebce_rect_border {
	int32_t width_min;
	int32_t width_max;
	int32_t width_alion;
	int32_t heigth_mix;
	int32_t heigth_max;
	int32_t heigth_align;
};

struct dss_hfbcd_para {
	bool mmu_enable;
	bool is_yuv_semi_planar;
	bool is_yuv_planar;
	int rdma_format;
	int rdma_transform;
	uint32_t stretch_size_vrt;
	uint32_t stretched_line_num;
	uint32_t mm_base0_y8;
	uint32_t mm_base1_c8;
	uint32_t mm_base2_y2;
	uint32_t mm_base3_c2;
	uint32_t hfbcd_block_type;

	uint32_t hfbcd_block_width_align;
	uint32_t hfbcd_block_height_align;
	uint32_t hfbcd_crop_num_max;

	int rdma_oft_x0;
	int rdma_oft_x1;
	uint32_t hfbcd_header_addr0;
	uint32_t hfbcd_header_stride0;
	uint32_t hfbcd_header_addr1;
	uint32_t hfbcd_header_offset;
	uint32_t hfbcd_header_stride1;
	uint32_t hfbcd_top_crop_num;
	uint32_t hfbcd_bottom_crop_num;
	uint32_t hfbcd_payload_addr0;
	uint32_t hfbcd_payload_stride0;
	uint32_t hfbcd_payload0_align;
	uint32_t hfbcd_payload_addr1;
	uint32_t hfbcd_payload_stride1;
	uint32_t hfbcd_payload1_align;
	uint32_t hfbcd_hreg_pic_width;
	uint32_t hfbcd_hreg_pic_height;
};

struct dss_hebcd_para {
	bool mmu_enable;
	bool is_yuv_semi_planar;
	int rdma_format;
	int rdma_transform;
	uint32_t stretch_size_vrt;
	uint32_t stretched_line_num;
	uint32_t mm_base0_y8;
	uint32_t mm_base1_c8;
	uint32_t hebcd_block_type;
	uint32_t color_transform;

	uint32_t hebcd_block_width_align;
	uint32_t hebcd_block_height_align;
	uint32_t hebcd_crop_num_max;

	int rdma_oft_x0;
	int rdma_oft_x1;
	uint32_t hebcd_header_addr0;
	uint32_t hebcd_header_stride0;
	uint32_t hebcd_header_addr1;
	uint32_t hebcd_header_stride1;
	uint32_t hebcd_header_offset;
	uint32_t hebcd_top_crop_num;
	uint32_t hebcd_bottom_crop_num;
	uint32_t hebcd_payload_addr0;
	uint32_t hebcd_payload_stride0;
	uint32_t hebcd_payload0_align;
	uint32_t hebcd_payload_addr1;
	uint32_t hebcd_payload_stride1;
	uint32_t hebcd_payload1_align;
	uint32_t hebcd_hreg_pic_width;
	uint32_t hebcd_hreg_pic_height;
	uint32_t hebcd_height_bf_str;
};


static int hisi_dss_hfbcd_mmbuf_calc(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	bool is_pixel_10bit, int chn_idx, uint32_t mmbuf_line_num)
{
	struct dss_mmbuf_info *mm_info = dpufd->mmbuf_info;
	int32_t rect_left;
	int32_t rect_right;

	if (is_pixel_10bit) {
		if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
			rect_left = ALIGN_DOWN((uint32_t)layer->src_rect.y, MMBUF_ADDR_ALIGN);
			rect_right = ALIGN_UP(ALIGN_UP((uint32_t)(layer->src_rect.y - rect_left + layer->src_rect.h),
				MMBUF_ADDR_ALIGN) / 4, MMBUF_ADDR_ALIGN);
		} else {
			rect_left = ALIGN_DOWN((uint32_t)layer->src_rect.x, MMBUF_ADDR_ALIGN);
			rect_right = ALIGN_UP(ALIGN_UP((uint32_t)(layer->src_rect.x - rect_left + layer->src_rect.w),
				MMBUF_ADDR_ALIGN) / 4, MMBUF_ADDR_ALIGN);
		}

		mm_info->mm_size2_y2[chn_idx] = rect_right * mmbuf_line_num;
		mm_info->mm_size3_c2[chn_idx] = mm_info->mm_size2_y2[chn_idx] / 2;
	}

	mm_info->mm_size[chn_idx] = mm_info->mm_size0_y8[chn_idx] + mm_info->mm_size1_c8[chn_idx] +
		mm_info->mm_size2_y2[chn_idx] + mm_info->mm_size3_c2[chn_idx];

	mm_info->mm_base[chn_idx] = hisi_dss_mmbuf_alloc(g_mmbuf_gen_pool, mm_info->mm_size[chn_idx]);
	mm_info->mm_base0_y8[chn_idx] = mm_info->mm_base[chn_idx];
	mm_info->mm_base1_c8[chn_idx] = mm_info->mm_base0_y8[chn_idx] + mm_info->mm_size0_y8[chn_idx];
	mm_info->mm_base2_y2[chn_idx] = mm_info->mm_base1_c8[chn_idx] + mm_info->mm_size1_c8[chn_idx];
	mm_info->mm_base3_c2[chn_idx] = mm_info->mm_base2_y2[chn_idx] + mm_info->mm_size2_y2[chn_idx];

	if ((mm_info->mm_base0_y8[chn_idx] < MMBUF_BASE) || (mm_info->mm_base1_c8[chn_idx] < MMBUF_BASE)) {
		DPU_FB_ERR("fb%d, chn%d failed to alloc mmbuf, mm_base0_y8=0x%x, mm_base1_c8=0x%x.\n",
			dpufd->index, chn_idx, mm_info->mm_base0_y8[chn_idx], mm_info->mm_base1_c8[chn_idx]);
			return -EINVAL;
	}

	if (is_pixel_10bit) {
		if ((mm_info->mm_base2_y2[chn_idx] < MMBUF_BASE) || (mm_info->mm_base3_c2[chn_idx] < MMBUF_BASE)) {
			DPU_FB_ERR("fb%d, chn%d failed to alloc mmbuf, mm_base2_y2=0x%x, mm_base3_c2=0x%x.\n",
				dpufd->index, chn_idx, mm_info->mm_base2_y2[chn_idx], mm_info->mm_base3_c2[chn_idx]);
			return -EINVAL;
		}
	}

	return 0;
}

static int hisi_dss_hfbcd_mmbuf_config(struct dpu_fb_data_type *dpufd, int ovl_idx,
	dss_layer_t *layer, uint32_t hfbcd_block_type, bool is_pixel_10bit)
{
	int chn_idx;
	dss_rect_t src_rect;
	bool mm_alloc_needed = false;

	dss_rect_ltrb_t hfbcd_rect;
	uint32_t mmbuf_line_num = 0;

	chn_idx = layer->chn_idx;
	src_rect = layer->src_rect;

	if (ovl_idx <= DSS_OVL1) {
		mm_alloc_needed = true;
	} else {
		if (dpufd->mmbuf_info->mm_used[chn_idx] == 1)
			mm_alloc_needed = false;
		else
			mm_alloc_needed = true;
	}

	if (mm_alloc_needed) {
		hfbcd_rect.left = ALIGN_DOWN((uint32_t)src_rect.x, MMBUF_ADDR_ALIGN);
		hfbcd_rect.right = ALIGN_UP((uint32_t)(src_rect.x - hfbcd_rect.left + src_rect.w), MMBUF_ADDR_ALIGN);

		if (hfbcd_block_type == 0) {
			if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
				mmbuf_line_num = MMBUF_BLOCK0_ROT_LINE_NUM;
				hfbcd_rect.left = ALIGN_DOWN((uint32_t)src_rect.y, MMBUF_ADDR_ALIGN);
				hfbcd_rect.right = ALIGN_UP((uint32_t)(src_rect.y - hfbcd_rect.left + src_rect.h),
					MMBUF_ADDR_ALIGN);
			} else {
				mmbuf_line_num = MMBUF_BLOCK0_LINE_NUM;
			}
		} else if (hfbcd_block_type == 1) {
			mmbuf_line_num = MMBUF_BLOCK1_LINE_NUM;
		} else {
			DPU_FB_ERR("hfbcd_block_type=%d no support!\n", layer->img.hfbcd_block_type);
			return -EINVAL;
		}

		dpufd->mmbuf_info->mm_size0_y8[chn_idx] = hfbcd_rect.right * mmbuf_line_num;
		dpufd->mmbuf_info->mm_size1_c8[chn_idx] = dpufd->mmbuf_info->mm_size0_y8[chn_idx] / 2;

		if (hisi_dss_hfbcd_mmbuf_calc(dpufd, layer, is_pixel_10bit, chn_idx, mmbuf_line_num) != 0)
			return -EINVAL;
	}

	dpufd->mmbuf_info->mm_used[chn_idx] = 1;

	return 0;
}

static int hisi_dss_hfbcd_config_check_mm_base0(
	struct dpu_fb_data_type *dpufd, uint32_t mm_base0_y8,
	uint32_t mm_base1_c8, int chn_idx, int32_t layer_idx)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL, return!\n");
		return -EINVAL;
	}

	if ((mm_base0_y8 & (MMBUF_ADDR_ALIGN - 1)) ||
		(dpufd->mmbuf_info->mm_size0_y8[chn_idx] & (MMBUF_ADDR_ALIGN - 1)) ||
		(mm_base1_c8 & (MMBUF_ADDR_ALIGN - 1)) ||
		(dpufd->mmbuf_info->mm_size1_c8[chn_idx] & (MMBUF_ADDR_ALIGN - 1))) {
		DPU_FB_ERR("layer%d mm_base0_y8 0x%x or mm_size0_y8 0x%x or "
			"mm_base1_c8 0x%x or mm_size1_c8 0x%x is not %d bytes aligned!\n",
			layer_idx, mm_base0_y8, dpufd->mmbuf_info->mm_size0_y8[chn_idx],
			mm_base1_c8, dpufd->mmbuf_info->mm_size1_c8[chn_idx],
			MMBUF_ADDR_ALIGN);
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_hfbcd_config_check_mm_base2(
	struct dpu_fb_data_type *dpufd, uint32_t mm_base2_y2,
	uint32_t mm_base3_c2, int chn_idx, int32_t layer_idx)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL, return!\n");
		return -EINVAL;
	}

	if ((mm_base2_y2 & (MMBUF_ADDR_ALIGN - 1)) ||
		(dpufd->mmbuf_info->mm_size2_y2[chn_idx] & (MMBUF_ADDR_ALIGN - 1)) ||
		(mm_base3_c2 & (MMBUF_ADDR_ALIGN - 1)) ||
		(dpufd->mmbuf_info->mm_size3_c2[chn_idx] & (MMBUF_ADDR_ALIGN - 1))) {
		DPU_FB_ERR("layer%d mm_base2_y2 0x%x or mm_size2_y2 0x%x or "
			"mm_base3_c2 0x%x or mm_size3_c2 0x%x is not %d bytes aligned!\n",
			layer_idx, mm_base2_y2, dpufd->mmbuf_info->mm_size2_y2[chn_idx],
			mm_base3_c2, dpufd->mmbuf_info->mm_size3_c2[chn_idx],
			MMBUF_ADDR_ALIGN);
		return -EINVAL;
	}

	return 0;
}

int hisi_dss_hfbcd_calc_base_para(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	struct dss_hfbcd_para *hfbcd_para)
{
	int chn_idx = layer->chn_idx;

	hfbcd_para->mmu_enable = (layer->img.mmu_enable == 1) ? true : false;
	hfbcd_para->is_yuv_semi_planar = is_yuv_semiplanar(layer->img.format);
	hfbcd_para->is_yuv_planar = is_yuv_plane(layer->img.format);

	hfbcd_para->rdma_format = hisi_pixel_format_hal2dma(layer->img.format);
	if (hfbcd_para->rdma_format < 0) {
		DPU_FB_ERR("layer format %d not support!\n", layer->img.format);
		return -EINVAL;
	}

	hfbcd_para->rdma_transform = hisi_transform_hal2dma(layer->transform, chn_idx);
	if (hfbcd_para->rdma_transform < 0) {
		DPU_FB_ERR("layer transform %d not support!\n", layer->transform);
		return -EINVAL;
	}

	return 0;
}

int hisi_dss_hfbcd_using_base_para(struct dpu_fb_data_type *dpufd, int ovl_idx, dss_layer_t *layer,
	struct dss_hfbcd_para *hfbcd_para)
{
	bool is_pixel_10bit = false;
	int ret;
	int chn_idx;

	chn_idx = layer->chn_idx;
	is_pixel_10bit = is_pixel_10bit2dma(hfbcd_para->rdma_format);
	if ((layer->img.hfbc_mmbuf_base0_y8 > 0) &&
		(layer->img.hfbc_mmbuf_base1_c8 > 0)) {
		hfbcd_para->mm_base0_y8 = layer->img.hfbc_mmbuf_base0_y8;
		hfbcd_para->mm_base1_c8 = layer->img.hfbc_mmbuf_base1_c8;
		hfbcd_para->mm_base2_y2 = layer->img.hfbc_mmbuf_base2_y2;
		hfbcd_para->mm_base3_c2 = layer->img.hfbc_mmbuf_base3_c2;
	} else {
		ret = hisi_dss_hfbcd_mmbuf_config(dpufd, ovl_idx, layer, layer->img.hfbcd_block_type, is_pixel_10bit);
		if (ret < 0) {
			DPU_FB_ERR("hisi_dss_hfbcd_mmbuf_config fail!\n");
			return -EINVAL;
		}

		hfbcd_para->mm_base0_y8 = dpufd->mmbuf_info->mm_base0_y8[chn_idx];
		hfbcd_para->mm_base1_c8 = dpufd->mmbuf_info->mm_base1_c8[chn_idx];
		if (is_pixel_10bit) {
			hfbcd_para->mm_base2_y2 = dpufd->mmbuf_info->mm_base2_y2[chn_idx];
			hfbcd_para->mm_base3_c2 = dpufd->mmbuf_info->mm_base3_c2[chn_idx];
		}
	}

	hfbcd_para->mm_base0_y8 -= MMBUF_BASE;
	hfbcd_para->mm_base1_c8 -= MMBUF_BASE;

	ret = hisi_dss_hfbcd_config_check_mm_base0(dpufd, hfbcd_para->mm_base0_y8, hfbcd_para->mm_base1_c8,
		chn_idx, layer->layer_idx);
	if (ret == -EINVAL)
		return ret;

	if (is_pixel_10bit) {
		hfbcd_para->mm_base2_y2 -= MMBUF_BASE;
		hfbcd_para->mm_base3_c2 -= MMBUF_BASE;

		ret = hisi_dss_hfbcd_config_check_mm_base2(dpufd, hfbcd_para->mm_base2_y2, hfbcd_para->mm_base3_c2,
			chn_idx, layer->layer_idx);
		if (ret == -EINVAL)
			return ret;
	}

	return 0;
}

int hisi_dss_hfbcd_get_align_info(struct dss_hfbcd_para *hfbcd_para, dss_layer_t *layer)
{
	if (layer->img.hfbcd_block_type == 0) {
		hfbcd_para->hfbcd_block_width_align = HFBC_BLOCK0_WIDTH_ALIGN;
		hfbcd_para->hfbcd_block_height_align = HFBC_BLOCK0_HEIGHT_ALIGN;
	} else if (layer->img.hfbcd_block_type == 1) {
		hfbcd_para->hfbcd_block_width_align = HFBC_BLOCK1_WIDTH_ALIGN;
		hfbcd_para->hfbcd_block_height_align = HFBC_BLOCK1_HEIGHT_ALIGN;
	} else {
		DPU_FB_ERR("hfbcd_block_type=%d no support!\n", layer->img.hfbcd_block_type);
		return -EINVAL;
	}

	return 0;
}

int hisi_dss_hfbcd_get_hreg_pic_info(struct dss_hfbcd_para *hfbcd_para,
	const dss_layer_t *layer, dss_rect_ltrb_t *aligned_rect)
{
	dss_rect_t src_rect = layer->src_rect;
	uint32_t src_bottom = (uint32_t)src_rect.y + src_rect.h;
	uint32_t src_right = (uint32_t)src_rect.x + src_rect.w;
	/* aligned rect */
	aligned_rect->left = ALIGN_DOWN((uint32_t)src_rect.x, hfbcd_para->hfbcd_block_width_align);
	aligned_rect->right = ALIGN_UP(src_right, hfbcd_para->hfbcd_block_width_align) - 1;
	aligned_rect->top = ALIGN_DOWN((uint32_t)src_rect.y, hfbcd_para->hfbcd_block_height_align);
	aligned_rect->bottom = ALIGN_UP(src_bottom, hfbcd_para->hfbcd_block_height_align) - 1;

	hfbcd_para->hfbcd_hreg_pic_width = aligned_rect->right - aligned_rect->left;
	hfbcd_para->hfbcd_hreg_pic_height = aligned_rect->bottom - aligned_rect->top;

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		if ((hfbcd_para->hfbcd_hreg_pic_width > HFBC_PIC_WIDTH_ROT_MAX) ||
			(hfbcd_para->hfbcd_hreg_pic_height > HFBC_PIC_HEIGHT_ROT_MAX)) {
			DPU_FB_ERR("layer%d hreg_pic_width %d, hreg_pic_height %d is larger than 4096*2160!\n",
				layer->layer_idx, hfbcd_para->hfbcd_hreg_pic_width, hfbcd_para->hfbcd_hreg_pic_height);
			return -EINVAL;
		}
	}

	return 0;
}

int hisi_dss_hfbcd_get_payload_info(struct dss_hfbcd_para *hfbcd_para,
	const dss_layer_t *layer, dss_rect_ltrb_t *aligned_rect, bool is_pixel_10bit)
{
	/* hfbcd payload */
	if (is_pixel_10bit) {
		hfbcd_para->hfbcd_payload0_align = HFBC_PAYLOAD_ALIGN_10BIT;
		hfbcd_para->hfbcd_payload1_align = HFBC_PAYLOAD_ALIGN_10BIT;
	} else {
		hfbcd_para->hfbcd_payload0_align = HFBC_PAYLOAD0_ALIGN_8BIT;
		hfbcd_para->hfbcd_payload1_align = HFBC_PAYLOAD1_ALIGN_8BIT;
	}

	hfbcd_para->hfbcd_payload_stride0 = layer->img.hfbc_payload_stride0;
	hfbcd_para->hfbcd_payload_stride1 = layer->img.hfbc_payload_stride1;

	hfbcd_para->hfbcd_payload_addr0 = layer->img.hfbc_payload_addr0 +
		(aligned_rect->top / hfbcd_para->hfbcd_block_height_align) * hfbcd_para->hfbcd_payload_stride0 +
		(aligned_rect->left / hfbcd_para->hfbcd_block_width_align) * hfbcd_para->hfbcd_payload0_align;

	hfbcd_para->hfbcd_payload_addr1 = layer->img.hfbc_payload_addr1 +
		(aligned_rect->top / hfbcd_para->hfbcd_block_height_align) * hfbcd_para->hfbcd_payload_stride1 +
		(aligned_rect->left / hfbcd_para->hfbcd_block_width_align) * hfbcd_para->hfbcd_payload1_align;

	if (is_pixel_10bit) {
		if ((hfbcd_para->hfbcd_payload_addr0 != hfbcd_para->hfbcd_payload_addr1) ||
			(hfbcd_para->hfbcd_payload_stride0 != hfbcd_para->hfbcd_payload_stride1)) {
			DPU_FB_ERR("layer%d 10bit hfbcd_payload_addr0 0x%x is not equal "
				"to hfbcd_payload_addr1 0x%x or hfbcd_payload_stride0 0x%x is "
				"not equal to hfbcd_payload_stride1 0x%x!\n",
				layer->layer_idx, hfbcd_para->hfbcd_payload_addr0, hfbcd_para->hfbcd_payload_addr1,
				hfbcd_para->hfbcd_payload_stride0, hfbcd_para->hfbcd_payload_stride1);
			return -EINVAL;
		}
	}

	dpu_check_and_return((hfbcd_para->hfbcd_payload_addr0 & (hfbcd_para->hfbcd_payload0_align - 1)),
		-EINVAL, ERR, "hfbcd_payload_addr0 and hfbcd_payload0_align - 1 not match!\n");
	dpu_check_and_return((hfbcd_para->hfbcd_payload_stride0 & (hfbcd_para->hfbcd_payload0_align - 1)),
		-EINVAL, ERR, "hfbcd_payload_stride0 and hfbcd_payload0_align not match!\n");
	dpu_check_and_return((hfbcd_para->hfbcd_payload_addr1 & (hfbcd_para->hfbcd_payload1_align - 1)),
		-EINVAL, ERR, "hfbcd_payload_addr1 and hfbcd_payload1_align - 1 not match!\n");
	dpu_check_and_return((hfbcd_para->hfbcd_payload_stride1 & (hfbcd_para->hfbcd_payload1_align - 1)),
		-EINVAL, ERR, "hfbcd_payload_stride1 and hfbcd_payload1_align - 1 not match!\n");

	return 0;
}

static int hisi_dss_hfbcd_get_encoded_para(struct dss_hfbcd_para *hfbcd_para,
	const dss_layer_t *layer, dss_rect_ltrb_t *aligned_rect)
{
	int bpp;
	int aligned_pixel;
	bool is_pixel_10bit = false;

	is_pixel_10bit = is_pixel_10bit2dma(hfbcd_para->rdma_format);
	bpp = (hfbcd_para->is_yuv_semi_planar || hfbcd_para->is_yuv_planar) ? 1 : layer->img.bpp;
	if (is_pixel_10bit)
		bpp = layer->img.bpp;

	aligned_pixel = DMA_ALIGN_BYTES / bpp;

	if (hisi_dss_hfbcd_get_hreg_pic_info(hfbcd_para, layer, aligned_rect) != 0)
		return -EINVAL;

	dpu_check_and_return((layer->img.width & (hfbcd_para->hfbcd_block_width_align - 1)), -EINVAL, ERR,
		"layer->img.width and hfbcd_block_width_align - 1 not match!\n");
	dpu_check_and_return((layer->img.height & (hfbcd_para->hfbcd_block_height_align - 1)), -EINVAL, ERR,
		"layer->img.height and hfbcd_block_height_align - 1 not match!\n");

	if (hfbcd_para->hfbcd_block_type == 0) {
		if (layer->transform & HISI_FB_TRANSFORM_ROT_90)
			hfbcd_para->hfbcd_crop_num_max = HFBCD_BLOCK0_ROT_CROP_MAX;
		else
			hfbcd_para->hfbcd_crop_num_max = HFBCD_BLOCK0_CROP_MAX;
	} else {
		hfbcd_para->hfbcd_crop_num_max = HFBCD_BLOCK1_CROP_MAX;
	}

	hfbcd_para->rdma_oft_x0 = aligned_rect->left / aligned_pixel;
	hfbcd_para->rdma_oft_x1 = aligned_rect->right / aligned_pixel;

	hfbcd_para->hfbcd_header_stride0 = layer->img.hfbc_header_stride0;
	hfbcd_para->hfbcd_header_offset =
		(aligned_rect->top / hfbcd_para->hfbcd_block_height_align) * hfbcd_para->hfbcd_header_stride0 +
		(aligned_rect->left / hfbcd_para->hfbcd_block_width_align) * HFBC_HEADER_STRIDE_BLOCK;

	hfbcd_para->hfbcd_header_addr0 = layer->img.hfbc_header_addr0 + hfbcd_para->hfbcd_header_offset;
	hfbcd_para->hfbcd_header_addr1 = layer->img.hfbc_header_addr1 + hfbcd_para->hfbcd_header_offset;
	hfbcd_para->hfbcd_header_stride1 = hfbcd_para->hfbcd_header_stride0;
	dpu_check_and_return((hfbcd_para->hfbcd_header_addr0 & (HFBC_HEADER_ADDR_ALIGN - 1)), -EINVAL, ERR,
		"hfbcd_para->hfbcd_header_addr0 and HFBC_HEADER_ADDR_ALIGN - 1 not match!\n");
	dpu_check_and_return((hfbcd_para->hfbcd_header_stride0 & (HFBC_HEADER_STRIDE_ALIGN - 1)), -EINVAL, ERR,
		"hfbcd_para->hfbcd_header_stride0 and HFBC_HEADER_STRIDE_ALIGN - 1 not match!\n");
	dpu_check_and_return((hfbcd_para->hfbcd_header_addr1 & (HFBC_HEADER_ADDR_ALIGN - 1)), -EINVAL, ERR,
		"hfbcd_para->hfbcd_header_addr1 and HFBC_HEADER_ADDR_ALIGN - 1 not match!\n");

	if (hisi_dss_hfbcd_get_payload_info(hfbcd_para, layer, aligned_rect, is_pixel_10bit) != 0)
		return -EINVAL;

	return 0;
}

static int hisi_dss_hfbcd_get_out_rect(struct dss_hfbcd_para *hfbcd_para, dss_layer_t *layer,
	dss_rect_ltrb_t *aligned_rect, dss_rect_ltrb_t *clip_rect, dss_rect_t *out_aligned_rect)
{
	dss_rect_t src_rect;
	int chn_idx;
	uint32_t src_bottom;
	uint32_t src_right;

	chn_idx = layer->chn_idx;
	src_rect = layer->src_rect;
	src_bottom = (uint32_t)src_rect.y + src_rect.h;
	src_right = (uint32_t)src_rect.x + src_rect.w;

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		aligned_rect->left = ALIGN_DOWN((uint32_t)src_rect.y, hfbcd_para->hfbcd_block_height_align);
		aligned_rect->right = ALIGN_UP(src_bottom, hfbcd_para->hfbcd_block_height_align) - 1;
		aligned_rect->top = ALIGN_DOWN((uint32_t)src_rect.x, hfbcd_para->hfbcd_block_width_align);
		aligned_rect->bottom = ALIGN_UP(src_right, hfbcd_para->hfbcd_block_width_align) - 1;
	}

	/* out_aligned_rect */
	out_aligned_rect->x = 0;
	out_aligned_rect->y = 0;
	out_aligned_rect->w = aligned_rect->right - aligned_rect->left + 1;
	out_aligned_rect->h = aligned_rect->bottom - aligned_rect->top + 1;

	/* rdfc clip_rect */
	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		clip_rect->left = src_rect.y - aligned_rect->left;
		clip_rect->right = aligned_rect->right - DSS_WIDTH(src_bottom);
		clip_rect->top = src_rect.x - aligned_rect->top;
		clip_rect->bottom = aligned_rect->bottom - DSS_HEIGHT(src_right);
	} else {
		clip_rect->left = src_rect.x - aligned_rect->left;
		clip_rect->right = aligned_rect->right - DSS_WIDTH(src_right);
		clip_rect->top = src_rect.y - aligned_rect->top;
		clip_rect->bottom = aligned_rect->bottom - DSS_HEIGHT(src_bottom);
	}
	if (hisi_adjust_clip_rect(layer, clip_rect) < 0) {
		DPU_FB_ERR("clip rect invalid => layer_idx=%d, chn_idx=%d, clip_rect %d, %d, %d, %d.\n",
			layer->layer_idx, chn_idx, clip_rect->left, clip_rect->right,
			clip_rect->top, clip_rect->bottom);
		return -EINVAL;
	}

	/* hfbcd crop */
	hfbcd_para->hfbcd_top_crop_num = (clip_rect->top > hfbcd_para->hfbcd_crop_num_max) ?  /*lint !e574*/
		hfbcd_para->hfbcd_crop_num_max : clip_rect->top;
	hfbcd_para->hfbcd_bottom_crop_num = (clip_rect->bottom > hfbcd_para->hfbcd_crop_num_max) ?  /*lint !e574*/
		hfbcd_para->hfbcd_crop_num_max : clip_rect->bottom;

	clip_rect->top -= hfbcd_para->hfbcd_top_crop_num;
	clip_rect->bottom -= hfbcd_para->hfbcd_bottom_crop_num;

	/* adjust out_aligned_rect */
	out_aligned_rect->h -= (hfbcd_para->hfbcd_top_crop_num + hfbcd_para->hfbcd_bottom_crop_num);
	hfbcd_para->stretch_size_vrt = DSS_HEIGHT(out_aligned_rect->h);

	return 0;
}

static void hisi_dss_hfbcd_generate_reg_value(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	const struct dss_hfbcd_para *hfbcd_para, dss_rect_ltrb_t aligned_rect, bool *rdma_stretch_enable)
{
	dss_rdma_t *dma = NULL;
	int chn_idx;

	chn_idx = layer->chn_idx;

	dma = &(dpufd->dss_module.rdma[chn_idx]);
	dpufd->dss_module.dma_used[chn_idx] = 1;

	dma->hfbcd_used = 1;
	dma->oft_x0 = set_bits32(dma->oft_x0, hfbcd_para->rdma_oft_x0, 16, 0);
	dma->oft_x1 = set_bits32(dma->oft_x1, hfbcd_para->rdma_oft_x1, 16, 0);
	dma->stretch_size_vrt = set_bits32(dma->stretch_size_vrt,
		(hfbcd_para->stretch_size_vrt | (hfbcd_para->stretched_line_num << 13)), 19, 0);
	dma->ctrl = set_bits32(dma->ctrl, hfbcd_para->rdma_format, 5, 3);
	dma->ctrl = set_bits32(dma->ctrl, (hfbcd_para->mmu_enable ? 0x1 : 0x0), 1, 8);
	dma->ctrl = set_bits32(dma->ctrl, hfbcd_para->rdma_transform, 3, 9);
	dma->ctrl = set_bits32(dma->ctrl, (*rdma_stretch_enable ? 1 : 0), 1, 12);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 0);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 2);

	dma->hfbcd_hreg_hdr_ptr_l0 = set_bits32(dma->hfbcd_hreg_hdr_ptr_l0, hfbcd_para->hfbcd_header_addr0, 32, 0);
	dma->hfbcd_hreg_pic_width = set_bits32(dma->hfbcd_hreg_pic_width, hfbcd_para->hfbcd_hreg_pic_width, 16, 0);
	dma->hfbcd_hreg_pic_height = set_bits32(dma->hfbcd_hreg_pic_height, hfbcd_para->hfbcd_hreg_pic_height, 16, 0);
	dma->hfbcd_line_crop = set_bits32(dma->hfbcd_line_crop,
		((hfbcd_para->hfbcd_top_crop_num << 8) | hfbcd_para->hfbcd_bottom_crop_num), 16, 0);
	dma->hfbcd_input_header_stride0 = set_bits32(dma->hfbcd_input_header_stride0,
		hfbcd_para->hfbcd_header_stride0, 14, 0);
	dma->hfbcd_hreg_hdr_ptr_l1 = set_bits32(dma->hfbcd_hreg_hdr_ptr_l1, hfbcd_para->hfbcd_header_addr1, 32, 0);
	dma->hfbcd_header_stride1 = set_bits32(dma->hfbcd_header_stride1, hfbcd_para->hfbcd_header_stride1, 14, 0);
	dma->hfbcd_mm_base0_y8 = set_bits32(dma->hfbcd_mm_base0_y8, hfbcd_para->mm_base0_y8, 32, 0);
	dma->hfbcd_mm_base1_c8 = set_bits32(dma->hfbcd_mm_base1_c8, hfbcd_para->mm_base1_c8, 32, 0);
	dma->hfbcd_mm_base2_y2 = set_bits32(dma->hfbcd_mm_base2_y2, hfbcd_para->mm_base2_y2, 32, 0);
	dma->hfbcd_mm_base3_c2 = set_bits32(dma->hfbcd_mm_base3_c2, hfbcd_para->mm_base3_c2, 32, 0);
	dma->hfbcd_payload_pointer = set_bits32(dma->hfbcd_payload_pointer, hfbcd_para->hfbcd_payload_addr0, 32, 0);
	dma->hfbcd_payload_stride0 = set_bits32(dma->hfbcd_payload_stride0, hfbcd_para->hfbcd_payload_stride0, 20, 0);
	dma->hfbcd_hreg_pld_ptr_l1 = set_bits32(dma->hfbcd_hreg_pld_ptr_l1, hfbcd_para->hfbcd_payload_addr1, 32, 0);
	dma->hfbcd_payload_stride1 = set_bits32(dma->hfbcd_payload_stride1, hfbcd_para->hfbcd_payload_stride1, 20, 0);
	dma->hfbcd_creg_fbcd_ctrl_mode = set_bits32(dma->hfbcd_creg_fbcd_ctrl_mode, 1, 2, 0);
	/* hfbcd_scramble_mode */
	dma->hfbcd_scramble_mode = set_bits32(dma->hfbcd_scramble_mode, layer->img.hfbc_scramble_mode, 4, 2);
	dma->hfbcd_block_type = set_bits32(dma->hfbcd_block_type, hfbcd_para->hfbcd_block_type, 2, 0);

	if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer)
		DPU_FB_INFO("fb%d, mm_base0_y8=0x%x, mm_base2_y2=0x%x, "
			"mm_base1_c8=0x%x, mm_base3_c2=0x%x, mm_base0_y8_size=%d, "
			"mm_base2_y2_size=%d,mm_base1_c8_size=%d,mm_base3_c2_size=%d,"
			"aligned_rect %d,%d,%d,%d, hfbcd_block_type=%d!\n",
			dpufd->index, hfbcd_para->mm_base0_y8, hfbcd_para->mm_base2_y2, hfbcd_para->mm_base1_c8,
			hfbcd_para->mm_base3_c2, layer->img.hfbc_mmbuf_size0_y8,
			layer->img.hfbc_mmbuf_size2_y2, layer->img.hfbc_mmbuf_size1_c8, layer->img.hfbc_mmbuf_size3_c2,
			aligned_rect.left, aligned_rect.top, aligned_rect.right,
			aligned_rect.bottom, hfbcd_para->hfbcd_block_type);
}

int hisi_dss_hfbcd_config(struct dpu_fb_data_type *dpufd,
	int ovl_idx, dss_layer_t *layer, struct hisi_ov_compose_rect *ov_compose_rect,
	struct hisi_ov_compose_flag *ov_compose_flag)
{
	struct dss_hfbcd_para hfbcd_para = {0};
	dss_rect_ltrb_t aligned_rect = { 0, 0, 0, 0 };
	int ret;

	if (!dpufd || !layer)
		return -EINVAL;

	if (!ov_compose_rect || !ov_compose_flag)
		return -EINVAL;

	if (!ov_compose_rect->aligned_rect || !ov_compose_rect->clip_rect)
		return -EINVAL;

	hfbcd_para.stretched_line_num = is_need_rdma_stretch_bit(dpufd, layer);
	ov_compose_flag->rdma_stretch_enable = (hfbcd_para.stretched_line_num > 0) ? true : false;

	ret = hisi_dss_hfbcd_calc_base_para(dpufd, layer, &hfbcd_para);
	if (ret)
		return -EINVAL;

	ret = hisi_dss_hfbcd_using_base_para(dpufd, ovl_idx, layer, &hfbcd_para);
	if (ret)
		return -EINVAL;

	ret = hisi_dss_hfbcd_get_align_info(&hfbcd_para, layer);
	if (ret)
		return -EINVAL;

	ret = hisi_dss_hfbcd_get_encoded_para(&hfbcd_para, layer, &aligned_rect);
	if (ret)
		return -EINVAL;

	ret = hisi_dss_hfbcd_get_out_rect(&hfbcd_para, layer, &aligned_rect, ov_compose_rect->clip_rect,
		ov_compose_rect->aligned_rect);
	if (ret)
		return -EINVAL;

	hisi_dss_hfbcd_generate_reg_value(dpufd, layer, &hfbcd_para, aligned_rect,
		&ov_compose_flag->rdma_stretch_enable);

	return 0;
}

void hisi_dss_hfbcd_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *dma_base, dss_rdma_t *s_dma)
{
	if (!dpufd || !dma_base || !s_dma) {
		DPU_FB_DEBUG("dpufd or dma_base, s_dma is NULL!\n");
		return;
	}

	dpufd->set_reg(dpufd, dma_base + AFBCD_HREG_HDR_PTR_LO,
		s_dma->hfbcd_hreg_hdr_ptr_l0, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_HREG_PIC_WIDTH,
		s_dma->hfbcd_hreg_pic_width, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_HREG_PIC_HEIGHT,
		s_dma->hfbcd_hreg_pic_height, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_LINE_CROP,
		s_dma->hfbcd_line_crop, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_INPUT_HEADER_STRIDE,
		s_dma->hfbcd_input_header_stride0, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_PAYLOAD_STRIDE,
		s_dma->hfbcd_payload_stride0, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_AFBCD_PAYLOAD_POINTER,
		s_dma->hfbcd_payload_pointer, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_SCRAMBLE_MODE,
		s_dma->hfbcd_scramble_mode, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_CREG_FBCD_CTRL_MODE,
		s_dma->hfbcd_creg_fbcd_ctrl_mode, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_HREG_HDR_PTR_L1,
		s_dma->hfbcd_hreg_hdr_ptr_l1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_HREG_PLD_PTR_L1,
		s_dma->hfbcd_hreg_pld_ptr_l1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_HEADER_SRTIDE_1,
		s_dma->hfbcd_header_stride1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_PAYLOAD_SRTIDE_1,
		s_dma->hfbcd_payload_stride1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_BLOCK_TYPE,
		s_dma->hfbcd_block_type, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_MM_BASE_0,
		s_dma->hfbcd_mm_base0_y8, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_MM_BASE_1,
		s_dma->hfbcd_mm_base1_c8, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_MM_BASE_2,
		s_dma->hfbcd_mm_base2_y2, 32, 0);
	dpufd->set_reg(dpufd, dma_base + AFBCD_MM_BASE_3,
		s_dma->hfbcd_mm_base3_c2, 32, 0);
}


