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

struct hisi_dss_addr_config {
	uint32_t src_addr;
	uint32_t stride;
	int bpp;
	int left;
	int right;
	int top;
	int bottom;
};

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

struct rdma_config_param {
	bool mmu_enable;
	bool src_rect_mask_enable;
	uint32_t rdma_addr;
	uint32_t rdma_stride;
	int rdma_format;
	int rdma_transform;
	int rdma_data_num;
	uint32_t stretch_size_vrt;
	uint32_t stretched_line_num;
	uint32_t stretched_stride;
	int aligned_pixel;
	struct dss_rdma_oft_pos rdma_oft_pos;
	struct dss_rdma_mask_pos rdma_mask_pos;
	uint32_t l2t_interleave_n;
	dss_rect_ltrb_t aligned_rect;
	dss_rect_ltrb_t aligned_src_rect;
	dss_rect_ltrb_t aligned_mask_rect;
	dss_rect_t src_rect;
	uint32_t src_bottom;
	uint32_t src_right;
	bool is_pixel_10bit;
	uint32_t mm_base_0;
	uint32_t mm_base_1;
};

struct rdma_config_afbcd_param {
	uint32_t half_block_mode;
	uint32_t stretch_acc;
	uint32_t stretch_inc;
	uint32_t height_bf_str;
	uint32_t top_crop_num;
	uint32_t bottom_crop_num;
	uint32_t header_addr;
	uint32_t header_stride;
	uint32_t payload_addr;
	uint32_t payload_stride;
	uint32_t header_start_pos;
	uint32_t header_pointer_offset;
};

void hisi_dss_rdma_init(const char __iomem *dma_base, dss_rdma_t *s_dma)
{
	dpu_check_and_no_retval(!dma_base, ERR, "dma_base is NULL\n");
	dpu_check_and_no_retval(!s_dma, ERR, "s_dma is NULL\n");

	memset(s_dma, 0, sizeof(*s_dma));

	s_dma->oft_x0 = inp32(dma_base + DMA_OFT_X0);
	s_dma->oft_y0 = inp32(dma_base + DMA_OFT_Y0);
	s_dma->oft_x1 = inp32(dma_base + DMA_OFT_X1);
	s_dma->oft_y1 = inp32(dma_base + DMA_OFT_Y1);
	s_dma->mask0 = inp32(dma_base + DMA_MASK0);
	s_dma->mask1 = inp32(dma_base + DMA_MASK1);
	s_dma->stretch_size_vrt = inp32(dma_base + DMA_STRETCH_SIZE_VRT);
	s_dma->ctrl = inp32(dma_base + DMA_CTRL);
	s_dma->tile_scram = inp32(dma_base + DMA_TILE_SCRAM);

	s_dma->ch_rd_shadow = inp32(dma_base + CH_RD_SHADOW);
	s_dma->ch_ctl = inp32(dma_base + CH_CTL);

	s_dma->data_addr0 = inp32(dma_base + DMA_DATA_ADDR0);
	s_dma->stride0 = inp32(dma_base + DMA_STRIDE0);
	s_dma->stretch_stride0 = inp32(dma_base + DMA_STRETCH_STRIDE0);
	s_dma->data_num0 = inp32(dma_base + DMA_DATA_NUM0);

	s_dma->vpp_ctrl = inp32(dma_base + VPP_CTRL);
	s_dma->vpp_mem_ctrl = inp32(dma_base + VPP_MEM_CTRL);

	s_dma->dma_buf_ctrl = inp32(dma_base + DMA_BUF_CTRL);

	s_dma->afbcd_hreg_hdr_ptr_lo = inp32(dma_base + AFBCD_HREG_HDR_PTR_LO);
	s_dma->afbcd_hreg_pic_width = inp32(dma_base + AFBCD_HREG_PIC_WIDTH);
	s_dma->afbcd_hreg_pic_height = inp32(dma_base + AFBCD_HREG_PIC_HEIGHT);
	s_dma->afbcd_hreg_format = inp32(dma_base + AFBCD_HREG_FORMAT);
	s_dma->afbcd_ctl = inp32(dma_base + AFBCD_CTL);
	s_dma->afbcd_str = inp32(dma_base + AFBCD_STR);
	s_dma->afbcd_line_crop = inp32(dma_base + AFBCD_LINE_CROP);
	s_dma->afbcd_input_header_stride = inp32(dma_base + AFBCD_INPUT_HEADER_STRIDE);
	s_dma->afbcd_payload_stride = inp32(dma_base + AFBCD_PAYLOAD_STRIDE);
	s_dma->afbcd_mm_base_0 = inp32(dma_base + AFBCD_MM_BASE_0);
	s_dma->afbcd_afbcd_payload_pointer = inp32(dma_base + AFBCD_AFBCD_PAYLOAD_POINTER);
	s_dma->afbcd_height_bf_str = inp32(dma_base + AFBCD_HEIGHT_BF_STR);
	s_dma->afbcd_os_cfg = inp32(dma_base + AFBCD_OS_CFG);
	s_dma->afbcd_mem_ctrl = inp32(dma_base + AFBCD_MEM_CTRL);
}

void hisi_dss_rdma_u_init(const char __iomem *dma_base, dss_rdma_t *s_dma)
{
	dpu_check_and_no_retval(!dma_base, ERR, "dma_base is NULL\n");
	dpu_check_and_no_retval(!s_dma, ERR, "s_dma is NULL\n");

	s_dma->data_addr1 = inp32(dma_base + DMA_DATA_ADDR1);
	s_dma->stride1 = inp32(dma_base + DMA_STRIDE1);
	s_dma->stretch_stride1 = inp32(dma_base + DMA_STRETCH_STRIDE1);
	s_dma->data_num1 = inp32(dma_base + DMA_DATA_NUM1);
}

void hisi_dss_rdma_v_init(const char __iomem *dma_base, dss_rdma_t *s_dma)
{
	dpu_check_and_no_retval(!dma_base, ERR, "dma_base is NULL\n");
	dpu_check_and_no_retval(!s_dma, ERR, "s_dma is NULL\n");

	s_dma->data_addr2 = inp32(dma_base + DMA_DATA_ADDR2);
	s_dma->stride2 = inp32(dma_base + DMA_STRIDE2);
	s_dma->stretch_stride2 = inp32(dma_base + DMA_STRETCH_STRIDE2);
	s_dma->data_num2 = inp32(dma_base + DMA_DATA_NUM2);
}

void hisi_dss_chn_set_reg_default_value(struct dpu_fb_data_type *dpufd,
	char __iomem *dma_base)
{
	dpu_check_and_no_retval(!dpufd, DEBUG, "dpufd is NULL\n");
	dpu_check_and_no_retval(!dma_base, DEBUG, "dma_base is NULL\n");

	dpufd->set_reg(dpufd, dma_base + CH_REG_DEFAULT, 0x1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + CH_REG_DEFAULT, 0x0, 32, 0);
}

static void set_rdma_afbcd_reg(struct dpu_fb_data_type *dpufd,
		char __iomem *dma_base, dss_rdma_t *s_dma)
{
	if (s_dma->afbc_used) {
		dpufd->set_reg(dpufd, dma_base + AFBCD_HREG_HDR_PTR_LO, s_dma->afbcd_hreg_hdr_ptr_lo, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_HREG_PIC_WIDTH, s_dma->afbcd_hreg_pic_width, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_HREG_PIC_HEIGHT, s_dma->afbcd_hreg_pic_height, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_HREG_FORMAT, s_dma->afbcd_hreg_format, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_CTL, s_dma->afbcd_ctl, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_STR, s_dma->afbcd_str, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_LINE_CROP, s_dma->afbcd_line_crop, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_INPUT_HEADER_STRIDE, s_dma->afbcd_input_header_stride, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_PAYLOAD_STRIDE, s_dma->afbcd_payload_stride, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_MM_BASE_0, s_dma->afbcd_mm_base_0, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_AFBCD_PAYLOAD_POINTER,
			s_dma->afbcd_afbcd_payload_pointer, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_HEIGHT_BF_STR, s_dma->afbcd_height_bf_str, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_OS_CFG, s_dma->afbcd_os_cfg, 32, 0);
		dpufd->set_reg(dpufd, dma_base + AFBCD_SCRAMBLE_MODE, s_dma->afbcd_scramble_mode, 32, 0);
#ifdef SUPPORT_DSS_AFBC_ENHANCE
		dpufd->set_reg(dpufd, dma_base + AFBCD_CREG_FBCD_CTRL_MODE, 0x0, 32, 0);
#endif
		dpufd->set_reg(dpufd, dma_base + AFBCD_HEADER_POINTER_OFFSET,
			s_dma->afbcd_header_pointer_offset, 32, 0);
	}
}

void hisi_dss_rdma_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *dma_base, dss_rdma_t *s_dma)
{
	dpu_check_and_no_retval((dpufd == NULL), ERR, "dpufd is NULL!\n");
	dpu_check_and_no_retval((dma_base == NULL), ERR, "dma_base is NULL!\n");
	dpu_check_and_no_retval((s_dma == NULL), ERR, "s_dma is NULL!\n");


	dpufd->set_reg(dpufd, dma_base + CH_REG_DEFAULT, 0x1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + CH_REG_DEFAULT, 0x0, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_OFT_X0, s_dma->oft_x0, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_OFT_Y0, s_dma->oft_y0, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_OFT_X1, s_dma->oft_x1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_OFT_Y1, s_dma->oft_y1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_MASK0, s_dma->mask0, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_MASK1, s_dma->mask1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_STRETCH_SIZE_VRT, s_dma->stretch_size_vrt, 32, 0);


	dpufd->set_reg(dpufd, dma_base + DMA_CTRL, s_dma->ctrl, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_TILE_SCRAM, s_dma->tile_scram, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_DATA_ADDR0, s_dma->data_addr0, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_STRIDE0, s_dma->stride0, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_STRETCH_STRIDE0, s_dma->stretch_stride0, 32, 0);

	dpufd->set_reg(dpufd, dma_base + CH_RD_SHADOW, s_dma->ch_rd_shadow, 32, 0);
	dpufd->set_reg(dpufd, dma_base + CH_CTL, s_dma->ch_ctl, 32, 0);

	if (s_dma->vpp_used)
		dpufd->set_reg(dpufd, dma_base + VPP_CTRL, s_dma->vpp_ctrl, 32, 0);

	dpufd->set_reg(dpufd, dma_base + DMA_BUF_CTRL, s_dma->dma_buf_ctrl, 32, 0);

	set_rdma_afbcd_reg(dpufd, dma_base, s_dma);

#ifdef SUPPORT_DSS_HFBCD
	if (s_dma->hfbcd_used)
		hisi_dss_hfbcd_set_reg(dpufd, dma_base, s_dma);
#endif

}

void hisi_dss_rdma_u_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *dma_base, dss_rdma_t *s_dma)
{
	dpu_check_and_no_retval(!dpufd, DEBUG, "dpufd is NULL\n");
	dpu_check_and_no_retval(!dma_base, DEBUG, "dma_base is NULL\n");
	dpu_check_and_no_retval(!s_dma, DEBUG, "s_dma is NULL\n");

	dpufd->set_reg(dpufd, dma_base + DMA_DATA_ADDR1, s_dma->data_addr1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_STRIDE1, s_dma->stride1, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_STRETCH_STRIDE1, s_dma->stretch_stride1, 32, 0);
}

void hisi_dss_rdma_v_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *dma_base, dss_rdma_t *s_dma)
{
	dpu_check_and_no_retval(!dpufd, DEBUG, "dpufd is NULL\n");
	dpu_check_and_no_retval(!dma_base, DEBUG, "dma_base is NULL\n");
	dpu_check_and_no_retval(!s_dma, DEBUG, "s_dma is NULL\n");

	dpufd->set_reg(dpufd, dma_base + DMA_DATA_ADDR2, s_dma->data_addr2, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_STRIDE2, s_dma->stride2, 32, 0);
	dpufd->set_reg(dpufd, dma_base + DMA_STRETCH_STRIDE2, s_dma->stretch_stride2, 32, 0);
}

int hisi_get_rdma_tile_interleave(uint32_t stride)
{
	int i;
	uint32_t interleave[MAX_TILE_SURPORT_NUM] = { 256, 512, 1024, 2048, 4096, 8192 };

	for (i = 0; i < MAX_TILE_SURPORT_NUM; i++) {
		if (interleave[i] == stride)
			return MIN_INTERLEAVE + i;
	}

	return 0;
}

static int hisi_dss_rdma_aligned_mask_rect(dss_layer_t *layer, dss_rect_ltrb_t *aligned_mask_rect,
	bool src_rect_mask_enable, int aligned_pixel)
{
	if (src_rect_mask_enable) {
		if (is_yuv_p_420(layer->img.format) || is_yuv_p_422(layer->img.format)) {
			aligned_mask_rect->left =
				ALIGN_UP((uint32_t)layer->src_rect_mask.x, 2 * (uint32_t)aligned_pixel);
			aligned_mask_rect->right =
				ALIGN_DOWN((uint32_t)(layer->src_rect_mask.x + layer->src_rect_mask.w),
				2 * (uint32_t)aligned_pixel) - 1;
		} else {
			aligned_mask_rect->left = ALIGN_UP((uint32_t)layer->src_rect_mask.x, (uint32_t)aligned_pixel);
			aligned_mask_rect->right =
				ALIGN_DOWN((uint32_t)(layer->src_rect_mask.x + layer->src_rect_mask.w),
				(uint32_t)aligned_pixel) - 1;
		}

		if (is_yuv_sp_420(layer->img.format) || is_yuv_p_420(layer->img.format)) {
			aligned_mask_rect->top = ALIGN_UP((uint32_t)layer->src_rect_mask.y, 2);  /* even alignment */
			aligned_mask_rect->bottom =
				/* odd alignment */
				ALIGN_DOWN((uint32_t)(layer->src_rect_mask.y + layer->src_rect_mask.h), 2) - 1;
		} else {
			aligned_mask_rect->top = layer->src_rect_mask.y;
			aligned_mask_rect->bottom = DSS_HEIGHT(layer->src_rect_mask.y + layer->src_rect_mask.h);
		}
	}

	return 0;
}

static int hisi_dss_rdma_aligned_rect(dss_layer_t *layer, dss_rect_ltrb_t *aligned_rect,
	dss_rect_t src_rect, int aligned_pixel)
{
	uint32_t src_bottom = (uint32_t)src_rect.y + src_rect.h;
	uint32_t src_right = (uint32_t)src_rect.x + src_rect.w;

	if (is_yuv_p_420(layer->img.format) || is_yuv_p_422(layer->img.format)) {
		/* even alignment */
		aligned_rect->left = ALIGN_DOWN((uint32_t)src_rect.x, 2 * (uint32_t)aligned_pixel);
		aligned_rect->right = ALIGN_UP(src_right, 2 * (uint32_t)aligned_pixel) - 1;  /* odd alignment */
	} else {
		aligned_rect->left = ALIGN_DOWN((uint32_t)src_rect.x, (uint32_t)aligned_pixel);
		aligned_rect->right = ALIGN_UP(src_right, (uint32_t)aligned_pixel) - 1;  /* odd alignment */
	}

	if (is_yuv_sp_420(layer->img.format) || is_yuv_p_420(layer->img.format)) {
		aligned_rect->top = ALIGN_DOWN((uint32_t)src_rect.y, 2);  /* even alignment */
		aligned_rect->bottom = ALIGN_UP(src_bottom, 2) - 1;  /* odd alignment */
	} else {
		aligned_rect->top = src_rect.y;
		aligned_rect->bottom = DSS_HEIGHT(src_bottom);
	}

	return 0;
}

static int hisi_dss_rdma_afbc_layer_aligned(dss_layer_t *layer, uint32_t mm_base_0, uint32_t mm_base_1)
{
	if ((layer->img.width & (AFBC_HEADER_ADDR_ALIGN - 1)) ||
		(layer->img.height & (AFBC_BLOCK_ALIGN - 1))) {
		DPU_FB_ERR("layer%d img width[%d] is not %d bytes aligned, or "
			"img heigh[%d] is not %d bytes aligned!\n",
			layer->layer_idx, layer->img.width, AFBC_HEADER_ADDR_ALIGN,
			layer->img.height, AFBC_BLOCK_ALIGN);
		return -EINVAL;
	}

	if ((mm_base_0 & (MMBUF_ADDR_ALIGN - 1)) ||
		(mm_base_1 & (MMBUF_ADDR_ALIGN - 1)) ||
		(layer->img.mmbuf_size & (MMBUF_ADDR_ALIGN - 1))) {
		DPU_FB_ERR("layer%d mm_base_0(0x%x) is not %d bytes aligned, or "
			"mm_base_1[0x%x] is not %d bytes aligned, or mmbuf_size[0x%x] is "
			"not %d bytes aligned!\n",
			layer->layer_idx, mm_base_0, MMBUF_ADDR_ALIGN,
			mm_base_1, MMBUF_ADDR_ALIGN, layer->img.mmbuf_size, MMBUF_ADDR_ALIGN);
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_rdma_stretch(dss_layer_t *layer, dss_rect_t *out_aligned_rect,
	const bool *rdma_stretch_enable, struct rdma_config_afbcd_param *afbcd_param)
{
	int rdma_transform;

	if (*rdma_stretch_enable) {
		afbcd_param->stretch_inc = 0;
		afbcd_param->stretch_acc = 0;

		/* adjust out_aligned_rect */
		out_aligned_rect->h /= 2;

		if (layer->transform & HISI_FB_TRANSFORM_FLIP_V)
			afbcd_param->half_block_mode = AFBC_HALF_BLOCK_LOWER_ONLY;
		else
			afbcd_param->half_block_mode = AFBC_HALF_BLOCK_UPPER_ONLY;
	} else {
		rdma_transform = hisi_transform_hal2dma(layer->transform, layer->chn_idx);
		dpu_check_and_return((rdma_transform < 0), -EINVAL, ERR,
			"layer transform=%d not support!\n", layer->transform);

		if ((uint32_t)rdma_transform & DSS_TRANSFORM_ROT)
			afbcd_param->half_block_mode = ((uint32_t)rdma_transform & DSS_TRANSFORM_FLIP_H) ?
				AFBC_HALF_BLOCK_LOWER_UPPER_ALL : AFBC_HALF_BLOCK_UPPER_LOWER_ALL;
		else
			afbcd_param->half_block_mode = ((uint32_t)rdma_transform & DSS_TRANSFORM_FLIP_V) ?
				AFBC_HALF_BLOCK_LOWER_UPPER_ALL : AFBC_HALF_BLOCK_UPPER_LOWER_ALL;
	}

	return 0;
}

static int hisi_dss_rdma_offset_pos_set(dss_layer_t *layer, struct dss_rdma_oft_pos *rdma_oft_pos,
	uint32_t *stretched_line_num)
{
	/* according to register require /2 -1 */
	if (is_yuv_p_420(layer->img.format) || is_yuv_p_422(layer->img.format)) {
		rdma_oft_pos->rdma_oft_x0 /= 2;
		rdma_oft_pos->rdma_oft_x1 = (rdma_oft_pos->rdma_oft_x1 + 1) / 2 - 1;
	}

	if (is_yuv_sp_420(layer->img.format) || is_yuv_p_420(layer->img.format)) {
		rdma_oft_pos->rdma_oft_y0 /= 2;
		rdma_oft_pos->rdma_oft_y1 = (rdma_oft_pos->rdma_oft_y1 + 1) / 2 - 1;

		*stretched_line_num /= 2;
	}

	return 0;
}

static bool is_src_rect_masked(dss_layer_t *layer, int aligned_pixel)
{
	return ((layer->src_rect_mask.w != 0) &&
		(layer->src_rect_mask.h != 0) &&
		(ALIGN_DOWN((uint32_t)(layer->src_rect_mask.x + layer->src_rect_mask.w),
		(uint32_t)aligned_pixel) > 1));
}

static int hisi_dss_rdma_set_dpp_and_dma(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	int *aligned_pixel, bool *src_rect_mask_enable, dss_rdma_t *dma)
{
	int chn_idx;
	int rdma_format;
	int bpp;
	bool is_pixel_10bit = false;
	bool is_yuv_semi_planar = false;
	bool is_yuv_planar = false;

	chn_idx = layer->chn_idx;
	rdma_format = hisi_pixel_format_hal2dma(layer->img.format);
	if (rdma_format < 0) {
		DPU_FB_ERR("layer format[%d] not support!\n", layer->img.format);
		return -EINVAL;
	}
	is_pixel_10bit = is_pixel_10bit2dma(rdma_format);
	is_yuv_semi_planar = is_yuv_semiplanar(layer->img.format);
	is_yuv_planar = is_yuv_plane(layer->img.format);

	bpp = (is_yuv_semi_planar || is_yuv_planar) ? 1 : layer->img.bpp;
	if (is_pixel_10bit)
		bpp = layer->img.bpp;

	if (bpp == 0) {
		DPU_FB_ERR("bpp is 0!\n");
		return -EINVAL;
	}
	*aligned_pixel = DMA_ALIGN_BYTES / bpp;

	*src_rect_mask_enable = is_src_rect_masked(layer, *aligned_pixel);

	dpufd->dss_module.dma_used[chn_idx] = 1;

	if (layer->need_cap & CAP_YUV_DEINTERLACE) {
		dma->vpp_used = 1;

		if (layer->transform & HISI_FB_TRANSFORM_ROT_90)
			dma->vpp_ctrl = set_bits32(dma->vpp_ctrl, 0x2, 2, 0);
		else
			dma->vpp_ctrl = set_bits32(dma->vpp_ctrl, 0x3, 2, 0);
	}

	return 0;
}

static int hisi_dss_rdma_addr_aligned(dss_layer_t *layer, uint32_t *l2t_interleave_n,
	uint32_t *rdma_addr, bool mmu_enable)
{
	void_unused(mmu_enable);
	*rdma_addr = layer->img.vir_addr;
	if (*rdma_addr & (DMA_ADDR_ALIGN - 1)) {
		DPU_FB_ERR("layer%d rdma_addr[0x%x] is not %d bytes aligned.\n",
			layer->layer_idx, *rdma_addr, DMA_ADDR_ALIGN);
		return -EINVAL;
	}

	if (layer->img.stride & (DMA_STRIDE_ALIGN - 1)) {
		DPU_FB_ERR("layer%d stride[0x%x] is not %d bytes aligned.\n",
			layer->layer_idx, layer->img.stride, DMA_STRIDE_ALIGN);
		return -EINVAL;
	}

	if (layer->need_cap & CAP_TILE) {
		*l2t_interleave_n = hisi_get_rdma_tile_interleave(layer->img.stride);
		if (*l2t_interleave_n < MIN_INTERLEAVE) {
			DPU_FB_ERR("tile stride should be 256*2^n, error stride:%d!\n", layer->img.stride);
			return -EINVAL;
		}

		if (*rdma_addr & (TILE_DMA_ADDR_ALIGN - 1)) {
			DPU_FB_ERR("layer%d tile rdma_addr[0x%x] is not %d bytes aligned.\n",
				layer->layer_idx, *rdma_addr, TILE_DMA_ADDR_ALIGN);
			return -EINVAL;
		}
	}

	return 0;
}

static int his_dss_rdma_afbcd_crop(dss_rect_ltrb_t *clip_rect, uint32_t *afbcd_top_crop_num,
	uint32_t *afbcd_bottom_crop_num)
{
	*afbcd_top_crop_num = (clip_rect->top > AFBCD_TOP_CROP_MAX) ?
		AFBCD_TOP_CROP_MAX : clip_rect->top;
	*afbcd_bottom_crop_num = (clip_rect->bottom > AFBCD_BOTTOM_CROP_MAX) ?
		AFBCD_BOTTOM_CROP_MAX : clip_rect->bottom;

	clip_rect->top -= *afbcd_top_crop_num;
	if (clip_rect->top < 0) {
		DPU_FB_ERR("clip_rect->top is invalid\n");
		return -EINVAL;
	}
	clip_rect->bottom -= *afbcd_bottom_crop_num;
	if (clip_rect->bottom < 0) {
		DPU_FB_ERR("clip_rect->bottom is invalid\n");
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_rdma_oft_pos_check(struct dss_rdma_oft_pos rdma_oft_pos)
{
	if ((rdma_oft_pos.rdma_oft_x1 - rdma_oft_pos.rdma_oft_x0) < 0 ||
		(rdma_oft_pos.rdma_oft_x1 - rdma_oft_pos.rdma_oft_x0 + 1) > DMA_IN_WIDTH_MAX) {
		DPU_FB_ERR("out of range, rdma_oft_x0 = %d, rdma_oft_x1 = %d!\n",
			rdma_oft_pos.rdma_oft_x0, rdma_oft_pos.rdma_oft_x1);
		return -EINVAL;
	}

	if ((rdma_oft_pos.rdma_oft_y1 - rdma_oft_pos.rdma_oft_y0) < 0 ||
		(rdma_oft_pos.rdma_oft_y1 - rdma_oft_pos.rdma_oft_y0 + 1) > DMA_IN_HEIGHT_MAX) {
		DPU_FB_ERR("out of range, rdma_oft_y0 = %d, rdma_oft_y1 = %d\n",
			rdma_oft_pos.rdma_oft_y0, rdma_oft_pos.rdma_oft_y1);
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_rdma_mask_pos_set(struct dss_rdma_oft_pos rdma_oft_pos,
	struct dss_rdma_mask_pos *rdma_mask_pos, dss_rect_ltrb_t aligned_mask_rect,
	bool *src_rect_mask_enable, int aligned_pixel)
{
	if (*src_rect_mask_enable) {
		rdma_mask_pos->rdma_mask_y0 = aligned_mask_rect.top;
		rdma_mask_pos->rdma_mask_y1 = aligned_mask_rect.bottom;
		rdma_mask_pos->rdma_mask_x0 = aligned_mask_rect.left / aligned_pixel;
		rdma_mask_pos->rdma_mask_x1 = aligned_mask_rect.right / aligned_pixel;

		/* according to register require setting */
		if ((rdma_mask_pos->rdma_mask_x1 - rdma_mask_pos->rdma_mask_x0) > 2)
			rdma_mask_pos->rdma_mask_x0 += 2;

		if ((rdma_mask_pos->rdma_mask_x0 <= rdma_oft_pos.rdma_oft_x0) ||
			(rdma_mask_pos->rdma_mask_x1 >= rdma_oft_pos.rdma_oft_x1) ||
			(rdma_mask_pos->rdma_mask_y0 <= rdma_oft_pos.rdma_oft_y0) ||
			(rdma_mask_pos->rdma_mask_y1 >= rdma_oft_pos.rdma_oft_y1)) {
			*src_rect_mask_enable = false;
			rdma_mask_pos->rdma_mask_x0 = 0;
			rdma_mask_pos->rdma_mask_y0 = 0;
			rdma_mask_pos->rdma_mask_x1 = 0;
			rdma_mask_pos->rdma_mask_y1 = 0;
		}
	}

	return 0;
}

static int hisi_dss_rdma_payload_set(dss_layer_t *layer, uint32_t *afbc_payload_stride,
	uint32_t *afbc_payload_addr, dss_rect_ltrb_t aligned_rect)
{
	uint32_t stride_align = 0;
	uint32_t addr_align = 0;

	if (layer->img.bpp == 4) {  /* Bit Per Pixel */
		stride_align = AFBC_PAYLOAD_STRIDE_ALIGN_32;
		addr_align = AFBC_PAYLOAD_ADDR_ALIGN_32;
	} else if (layer->img.bpp == 2) {  /* Bit Per Pixel */
		stride_align = AFBC_PAYLOAD_STRIDE_ALIGN_16;
		addr_align = AFBC_PAYLOAD_ADDR_ALIGN_16;
	} else {
		DPU_FB_ERR("bpp[%d] not supported!\n", layer->img.bpp);
		return -EINVAL;
	}

	*afbc_payload_stride = layer->img.afbc_payload_stride;
	if (layer->img.afbc_scramble_mode != DSS_AFBC_SCRAMBLE_MODE2)
		*afbc_payload_stride = (layer->img.width / AFBC_BLOCK_ALIGN) * stride_align;

	*afbc_payload_addr = layer->img.afbc_payload_addr +
		(aligned_rect.top / AFBC_BLOCK_ALIGN) * (*afbc_payload_stride) +
		(aligned_rect.left / AFBC_BLOCK_ALIGN) * stride_align;

	if ((*afbc_payload_addr & (addr_align - 1)) ||
		(*afbc_payload_stride & (stride_align - 1))) {
		DPU_FB_ERR("layer%d afbc_payload_addr[0x%x] is not %d bytes aligned, or "
			"afbc_payload_stride[0x%x] is not %d bytes aligned!\n",
			layer->layer_idx, *afbc_payload_addr, addr_align,
			*afbc_payload_stride, stride_align);
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_rdma_head_check(dss_layer_t *layer, dss_rect_ltrb_t aligned_rect,
	dss_rect_ltrb_t afbc_rect, struct rdma_config_afbcd_param *afbcd_param)
{
	if ((afbcd_param->header_addr & (AFBC_HEADER_ADDR_ALIGN - 1)) ||
		(afbcd_param->header_stride & (AFBC_HEADER_STRIDE_ALIGN - 1))) {
		DPU_FB_ERR("layer%d afbc_header_addr[0x%x] is not %d bytes aligned, or "
			"afbc_header_stride[0x%x] is not %d bytes aligned!\n",
			layer->layer_idx, afbcd_param->header_addr, AFBC_HEADER_ADDR_ALIGN,
			afbcd_param->header_stride, AFBC_HEADER_STRIDE_ALIGN);
		return -EINVAL;
	}

	if ((aligned_rect.left - afbc_rect.left) < 0) {
		DPU_FB_ERR("aligned_rect.left[%d] small than  afbc_rect.left[%d] is err.\n",
			aligned_rect.left, afbc_rect.left);
		return -EINVAL;
	};

	afbcd_param->header_start_pos = (aligned_rect.left - afbc_rect.left) / AFBC_BLOCK_ALIGN;
#ifdef SUPPORT_DSS_AFBC_ENHANCE
	if (afbcd_param->header_start_pos != 0) {
		DPU_FB_ERR("aligned_rect.left[%d] no equal to afbc_rect.left[%d] is err.\n",
			aligned_rect.left, afbc_rect.left);
		return -EINVAL;
	}
#endif
	return 0;
}

static uint32_t get_display_addr(dss_layer_t *layer, struct hisi_dss_addr_config add_cfg)
{
	uint32_t disp_addr = 0;

	switch (layer->transform) {
	case HISI_FB_TRANSFORM_NOP:
		disp_addr = add_cfg.src_addr + add_cfg.top * add_cfg.stride + add_cfg.left * add_cfg.bpp;
		break;
	case HISI_FB_TRANSFORM_FLIP_H:
		disp_addr = add_cfg.src_addr + add_cfg.top * add_cfg.stride + add_cfg.right * add_cfg.bpp;
		break;
	case HISI_FB_TRANSFORM_FLIP_V:
		disp_addr = add_cfg.src_addr + add_cfg.bottom * add_cfg.stride + add_cfg.left * add_cfg.bpp;
		break;
	case HISI_FB_TRANSFORM_ROT_180:
		disp_addr = add_cfg.src_addr + add_cfg.bottom * add_cfg.stride + add_cfg.right * add_cfg.bpp;
		break;
	default:
		DPU_FB_ERR("not supported this transform:%d!\n", layer->transform);
		break;
	}

	return disp_addr;
}

static void get_addr_config_param(struct hisi_dss_addr_config *add_config, bool mmu_enable,
	dss_layer_t *layer, uint32_t offset, bool is_pixel_10bit)
{
	void_unused(mmu_enable);
	add_config->src_addr = layer->img.vir_addr + offset;
	add_config->bpp = 1;  /* Bit Per Pixel */
	if (is_pixel_10bit)
		add_config->bpp = layer->img.bpp;

	if (is_yuv_p_420(layer->img.format) || is_yuv_p_422(layer->img.format)) {
		add_config->left /= 2;  /* left half valve */
		add_config->right /= 2;  /* right half valve */
	}

	if (is_yuv_sp_420(layer->img.format) || is_yuv_p_420(layer->img.format)) {
		add_config->top /= 2;  /* top half valve */
		add_config->bottom /= 2;  /* bottom half valve */
	}
}

static uint32_t hisi_calculate_display_addr(bool mmu_enable, dss_layer_t *layer,
	dss_rect_ltrb_t *aligned_rect, int add_type, bool is_pixel_10bit)
{
	uint32_t offset;
	struct hisi_dss_addr_config add_config = {0};

	add_config.left = aligned_rect->left;
	add_config.right = aligned_rect->right;
	add_config.top = aligned_rect->top;
	add_config.bottom = aligned_rect->bottom;

	if (add_type == DSS_ADDR_PLANE0) {
		add_config.stride = layer->img.stride;
		add_config.src_addr = layer->img.vir_addr;
		add_config.bpp = layer->img.bpp;
		if (layer->is_cld_layer == 1)
			add_config.src_addr += layer->cld_data_offset;
	} else if (add_type == DSS_ADDR_PLANE1) {
		add_config.stride = layer->img.stride_plane1;
		offset = layer->img.offset_plane1;
		get_addr_config_param(&add_config, mmu_enable, layer, offset, is_pixel_10bit);
	} else if (add_type == DSS_ADDR_PLANE2) {
		add_config.stride = layer->img.stride_plane2;
		offset = layer->img.offset_plane2;
		get_addr_config_param(&add_config, mmu_enable, layer, offset, is_pixel_10bit);
	} else {
		DPU_FB_ERR("NOT SUPPORT this add_type:%d.\n", add_type);
	}

	return get_display_addr(layer, add_config);
}

static void config_afbc_aligned_rect(dss_layer_t *layer,
	struct rdma_config_param *param, struct rdma_config_afbcd_param *afbcd_param)
{
	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		param->aligned_rect.left = ALIGN_DOWN((uint32_t)param->src_rect.y, AFBC_BLOCK_ALIGN);
		param->aligned_rect.right = ALIGN_UP(param->src_bottom, AFBC_BLOCK_ALIGN) - 1;  /* odd alignment */
		param->aligned_rect.top = ALIGN_DOWN((uint32_t)param->src_rect.x, AFBC_BLOCK_ALIGN);
		param->aligned_rect.bottom = ALIGN_UP(param->src_right, AFBC_BLOCK_ALIGN) - 1;  /* odd alignment */

		afbcd_param->height_bf_str = param->aligned_rect.right - param->aligned_rect.left + 1;
	} else {
		param->aligned_rect.left = ALIGN_DOWN((uint32_t)param->src_rect.x, AFBC_BLOCK_ALIGN);
		param->aligned_rect.right = ALIGN_UP(param->src_right, AFBC_BLOCK_ALIGN) - 1;  /* odd alignment */
		param->aligned_rect.top = ALIGN_DOWN((uint32_t)param->src_rect.y, AFBC_BLOCK_ALIGN);
		param->aligned_rect.bottom = ALIGN_UP(param->src_bottom, AFBC_BLOCK_ALIGN) - 1;  /* odd alignment */

		afbcd_param->height_bf_str = param->aligned_rect.bottom - param->aligned_rect.top + 1;
	}
}

static void config_afbc_original_aligned_rect(struct rdma_config_param *param)
{
	param->aligned_src_rect.left = ALIGN_DOWN((uint32_t)param->src_rect.x, AFBC_BLOCK_ALIGN);
	param->aligned_src_rect.right = ALIGN_UP(param->src_right, AFBC_BLOCK_ALIGN) - 1;  /* odd alignment */
	param->aligned_src_rect.top = ALIGN_DOWN((uint32_t)param->src_rect.y, AFBC_BLOCK_ALIGN);
	param->aligned_src_rect.bottom = ALIGN_UP(param->src_bottom, AFBC_BLOCK_ALIGN) - 1;  /* odd alignment */
}

static void config_afbc_out_aligned_rect(dss_rect_t *out_aligned_rect, struct rdma_config_param *param)
{
	out_aligned_rect->x = 0;
	out_aligned_rect->y = 0;
	out_aligned_rect->w = param->aligned_rect.right - param->aligned_rect.left + 1;
	out_aligned_rect->h = param->aligned_rect.bottom - param->aligned_rect.top + 1;
}

static int config_afbc_clip_rect(dss_rect_ltrb_t *clip_rect, dss_layer_t *layer,
	struct rdma_config_param *param)
{
	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		clip_rect->left = param->src_rect.y - param->aligned_rect.left;
		clip_rect->right = param->aligned_rect.right - DSS_WIDTH(param->src_bottom);
		clip_rect->top = param->src_rect.x - param->aligned_rect.top;
		clip_rect->bottom = param->aligned_rect.bottom - DSS_HEIGHT(param->src_right);
	} else {
		clip_rect->left = param->src_rect.x - param->aligned_rect.left;
		clip_rect->right = param->aligned_rect.right - DSS_WIDTH(param->src_right);
		clip_rect->top = param->src_rect.y - param->aligned_rect.top;
		clip_rect->bottom = param->aligned_rect.bottom - DSS_HEIGHT(param->src_bottom);
	}

	if (hisi_adjust_clip_rect(layer, clip_rect) < 0) {
		DPU_FB_ERR("clip rect invalid => layer_idx=%d, chn_idx=%d, clip_rect[%d, %d, %d, %d].\n",
			layer->layer_idx, layer->chn_idx, clip_rect->left, clip_rect->right,
			clip_rect->top, clip_rect->bottom);
		return -EINVAL;
	}

	return 0;
}

static void config_afbc_rect(dss_rect_ltrb_t *afbc_rect, struct rdma_config_param *param)
{
	afbc_rect->left = ALIGN_DOWN((uint32_t)param->src_rect.x, AFBC_HEADER_ADDR_ALIGN);
	afbc_rect->right = ALIGN_UP(param->src_right, AFBC_HEADER_ADDR_ALIGN) - 1;  /* odd alignment */
	afbc_rect->top = ALIGN_DOWN((uint32_t)param->src_rect.y, AFBC_BLOCK_ALIGN);
	afbc_rect->bottom = ALIGN_UP(param->src_bottom, AFBC_BLOCK_ALIGN) - 1;  /* odd alignment */
}

static int config_afbc_header(dss_layer_t *layer, struct rdma_config_param *param,
	dss_rect_ltrb_t afbc_rect, struct rdma_config_afbcd_param *afbcd_param)
{
	int ret;

	afbcd_param->header_stride = (layer->img.width / AFBC_BLOCK_ALIGN) * AFBC_HEADER_STRIDE_BLOCK;
	afbcd_param->header_pointer_offset = (afbc_rect.top / AFBC_BLOCK_ALIGN) * afbcd_param->header_stride +
		(afbc_rect.left / AFBC_BLOCK_ALIGN) * AFBC_HEADER_STRIDE_BLOCK;
	afbcd_param->header_addr = layer->img.afbc_header_addr + afbcd_param->header_pointer_offset;
	ret = hisi_dss_rdma_head_check(layer, param->aligned_src_rect, afbc_rect, afbcd_param);
	if (ret != 0)
		return -EINVAL;

	return 0;
}

static void set_afbcd_dma(dss_rdma_t *dma, dss_layer_t *layer, struct rdma_config_param *param,
	struct rdma_config_afbcd_param afbcd_param, bool *rdma_stretch_enable)
{
	dma->oft_x0 = set_bits32(dma->oft_x0, param->rdma_oft_pos.rdma_oft_x0, 16, 0);
	dma->oft_x1 = set_bits32(dma->oft_x1, param->rdma_oft_pos.rdma_oft_x1, 16, 0);
	dma->stretch_size_vrt = set_bits32(dma->stretch_size_vrt,
		(param->stretch_size_vrt | (param->stretched_line_num << 13)), 19, 0);
	dma->ctrl = set_bits32(dma->ctrl, param->rdma_format, 5, 3);
	dma->ctrl = set_bits32(dma->ctrl, (param->mmu_enable ? 0x1 : 0x0), 1, 8);
	dma->ctrl = set_bits32(dma->ctrl, param->rdma_transform, 3, 9);
	dma->ctrl = set_bits32(dma->ctrl, (*rdma_stretch_enable ? 1 : 0), 1, 12);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 0);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 2);

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		dma->afbcd_hreg_pic_width = set_bits32(dma->afbcd_hreg_pic_width,
			(param->aligned_rect.bottom - param->aligned_rect.top), 16, 0);
		dma->afbcd_hreg_pic_height = set_bits32(dma->afbcd_hreg_pic_height,
			(param->aligned_rect.right - param->aligned_rect.left), 16, 0);
	} else {
		dma->afbcd_hreg_pic_width = set_bits32(dma->afbcd_hreg_pic_width,
			(param->aligned_rect.right - param->aligned_rect.left), 16, 0);
		dma->afbcd_hreg_pic_height = set_bits32(dma->afbcd_hreg_pic_height,
			(param->aligned_rect.bottom - param->aligned_rect.top), 16, 0);
	}

	/* block split mode */
	dma->afbcd_hreg_format = set_bits32(dma->afbcd_hreg_format, 0x1, 1, 0);
	/* color transform */
	dma->afbcd_hreg_format = set_bits32(dma->afbcd_hreg_format,
		(is_yuv_package(layer->img.format) ? 0x0 : 0x1), 1, 21);
	dma->afbcd_ctl = set_bits32(dma->afbcd_ctl, afbcd_param.half_block_mode, 2, 6);
	dma->afbcd_str = set_bits32(dma->afbcd_str, ((afbcd_param.stretch_acc << 8) | afbcd_param.stretch_inc), 12, 0);

#ifdef SUPPORT_DSS_AFBC_ENHANCE
	dma->afbcd_line_crop = set_bits32(dma->afbcd_line_crop,
		((afbcd_param.top_crop_num << 8) | afbcd_param.bottom_crop_num), 16, 0);
#else
	dma->afbcd_line_crop = set_bits32(dma->afbcd_line_crop,
		((afbcd_param.top_crop_num << 4) | afbcd_param.bottom_crop_num), 8, 0);
#endif

	dma->afbcd_hreg_hdr_ptr_lo = set_bits32(dma->afbcd_hreg_hdr_ptr_lo, afbcd_param.header_addr, 32, 0);
	dma->afbcd_input_header_stride = set_bits32(dma->afbcd_input_header_stride,
		(afbcd_param.header_start_pos << 14) | afbcd_param.header_stride, 16, 0);
	dma->afbcd_payload_stride = set_bits32(dma->afbcd_payload_stride, afbcd_param.payload_stride, 20, 0);
	dma->afbcd_mm_base_0 = set_bits32(dma->afbcd_mm_base_0, param->mm_base_0, 32, 0);
	dma->afbcd_afbcd_payload_pointer = set_bits32(dma->afbcd_afbcd_payload_pointer,
		afbcd_param.payload_addr, 32, 0);
	dma->afbcd_height_bf_str = set_bits32(dma->afbcd_height_bf_str, DSS_HEIGHT(afbcd_param.height_bf_str), 16, 0);
	dma->afbcd_header_pointer_offset = set_bits32(dma->afbcd_header_pointer_offset,
		afbcd_param.header_pointer_offset, 32, 0);
	/* afbcd_scramble_mode */
	dma->afbcd_scramble_mode = set_bits32(dma->afbcd_scramble_mode, layer->img.afbc_scramble_mode, 2, 0);
}

static void print_afbcd_info(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	dss_rect_ltrb_t afbc_rect, struct rdma_config_param *param)
{
	if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer)
		DPU_FB_INFO("fb%d, mm_base_0=0x%x, mm_base_1=0x%x, mmbuf_size=%d, "
			"aligned_rect[%d,%d,%d,%d], afbc_rect[%d,%d,%d,%d]!\n",
			dpufd->index, param->mm_base_0, param->mm_base_1, layer->img.mmbuf_size,
			param->aligned_rect.left, param->aligned_rect.top,
			param->aligned_rect.right, param->aligned_rect.bottom,
			afbc_rect.left, afbc_rect.top, afbc_rect.right, afbc_rect.bottom);
}

static int hisi_dss_afbcd_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	struct hisi_ov_compose_rect *ov_compose_rect,
	struct hisi_ov_compose_flag *ov_compose_flag,
	struct rdma_config_param *param)
{
	int ret;
	dss_rdma_t *dma = &(dpufd->dss_module.rdma[layer->chn_idx]);
	dss_rect_ltrb_t afbc_rect;
	struct rdma_config_afbcd_param afbcd_param = {0};

	ret = hisi_dss_rdma_set_mmbuf_base_and_size(dpufd, layer, &afbc_rect, &param->mm_base_0, &param->mm_base_1);
	if (ret != 0)
		return -EINVAL;

	if (hisi_dss_rdma_afbc_layer_aligned(layer, param->mm_base_0, param->mm_base_1) != 0)
		return -EINVAL;

	dma->afbc_used = 1;

	config_afbc_aligned_rect(layer, param, &afbcd_param);
	config_afbc_original_aligned_rect(param);
	config_afbc_out_aligned_rect(ov_compose_rect->aligned_rect, param);

	/* stretch */
	ret = hisi_dss_rdma_stretch(layer, ov_compose_rect->aligned_rect,
		&ov_compose_flag->rdma_stretch_enable, &afbcd_param);
	if (ret != 0)
		return -EINVAL;

	if (layer->img.afbc_header_addr & (AFBC_SUPER_GRAPH_HEADER_ADDR_ALIGN - 1)) {
		DPU_FB_ERR("layer%d super graph afbc_header_addr[0x%x] is not %d bytes aligned!\n",
			layer->layer_idx, afbcd_param.header_addr, AFBC_SUPER_GRAPH_HEADER_ADDR_ALIGN);
		return -EINVAL;
	}

	/* rdfc clip_rect */
	if (config_afbc_clip_rect(ov_compose_rect->clip_rect, layer, param) != 0)
		return -EINVAL;

	/* afbcd crop */
	if (his_dss_rdma_afbcd_crop(ov_compose_rect->clip_rect, &afbcd_param.top_crop_num,
		&afbcd_param.bottom_crop_num) != 0)
		return -EINVAL;

	/* adjust out_aligned_rect */
	ov_compose_rect->aligned_rect->h -= (afbcd_param.top_crop_num + afbcd_param.bottom_crop_num);
	if (param->aligned_pixel == 0) {
		DPU_FB_ERR("The value of aligned_pixel is 0\n");
		return -EINVAL;
	}
	param->rdma_oft_pos.rdma_oft_x0 = param->aligned_src_rect.left / param->aligned_pixel;
	param->rdma_oft_pos.rdma_oft_x1 = param->aligned_src_rect.right / param->aligned_pixel;
	param->stretch_size_vrt = DSS_HEIGHT(ov_compose_rect->aligned_rect->h);
	param->stretched_line_num = 0;

	config_afbc_rect(&afbc_rect, param);

	/* header */
	if (config_afbc_header(layer, param, afbc_rect, &afbcd_param) != 0)
		return -EINVAL;

	/* payload */
	ret = hisi_dss_rdma_payload_set(layer,
		&(afbcd_param.payload_stride), &(afbcd_param.payload_addr), param->aligned_src_rect);
	if (ret != 0)
		return -EINVAL;

	print_afbcd_info(dpufd, layer, afbc_rect, param);

	set_afbcd_dma(dma, layer, param, afbcd_param, &ov_compose_flag->rdma_stretch_enable);

	return 0;
}

static void config_out_aligned_rect(dss_rect_t *out_aligned_rect, struct rdma_config_param *param)
{
	out_aligned_rect->x = 0;
	out_aligned_rect->y = 0;
	out_aligned_rect->w = param->aligned_rect.right - param->aligned_rect.left + 1;
	out_aligned_rect->h = param->aligned_rect.bottom - param->aligned_rect.top + 1;
	if (param->stretched_line_num > 0) {
		param->stretch_size_vrt = (out_aligned_rect->h / param->stretched_line_num) +
			((out_aligned_rect->h % param->stretched_line_num) ? 1 : 0) - 1;

		out_aligned_rect->h = param->stretch_size_vrt + 1;
	} else {
		param->stretch_size_vrt = 0x0;
	}
}

static int config_clip_rect(dss_rect_ltrb_t *clip_rect, dss_layer_t *layer, struct rdma_config_param *param)
{
	clip_rect->left = param->src_rect.x - param->aligned_rect.left;
	clip_rect->right = param->aligned_rect.right - DSS_WIDTH(param->src_right);
	clip_rect->top = param->src_rect.y - param->aligned_rect.top;
	clip_rect->bottom = param->aligned_rect.bottom - DSS_HEIGHT(param->src_bottom);

	if (hisi_adjust_clip_rect(layer, clip_rect) < 0) {
		DPU_FB_ERR("clip rect invalid => layer_idx=%d, chn_idx=%d, clip_rect[%d, %d, %d, %d].\n",
			layer->layer_idx, layer->chn_idx, clip_rect->left, clip_rect->right,
			clip_rect->top, clip_rect->bottom);
		return -EINVAL;
	}

	return 0;
}

static void set_dma(dss_rdma_t *dma, dss_layer_t *layer, struct rdma_config_param *param)
{
	dma->oft_x0 = set_bits32(dma->oft_x0, param->rdma_oft_pos.rdma_oft_x0, 16, 0);
	dma->oft_y0 = set_bits32(dma->oft_y0, param->rdma_oft_pos.rdma_oft_y0, 16, 0);
	dma->oft_x1 = set_bits32(dma->oft_x1, param->rdma_oft_pos.rdma_oft_x1, 16, 0);
	dma->oft_y1 = set_bits32(dma->oft_y1, param->rdma_oft_pos.rdma_oft_y1, 16, 0);
	dma->mask0 = set_bits32(dma->mask0,
		((uint32_t)param->rdma_mask_pos.rdma_mask_y0 |
		((uint32_t)param->rdma_mask_pos.rdma_mask_x0 << 16)), 32, 0);
	dma->mask1 = set_bits32(dma->mask1,
		((uint32_t)param->rdma_mask_pos.rdma_mask_y1 |
		((uint32_t)param->rdma_mask_pos.rdma_mask_x1 << 16)), 32, 0);
	dma->stretch_size_vrt = set_bits32(dma->stretch_size_vrt,
		(param->stretch_size_vrt | (param->stretched_line_num << 13)), 19, 0);
	dma->ctrl = set_bits32(dma->ctrl, ((layer->need_cap & CAP_TILE) ? 0x1 : 0x0), 1, 1);
	dma->ctrl = set_bits32(dma->ctrl, param->rdma_format, 5, 3);
	dma->ctrl = set_bits32(dma->ctrl, (param->mmu_enable ? 0x1 : 0x0), 1, 8);
	dma->ctrl = set_bits32(dma->ctrl, param->rdma_transform, 3, 9);
	dma->ctrl = set_bits32(dma->ctrl, ((param->stretched_line_num > 0) ? 0x1 : 0x0), 1, 12);
	dma->ctrl = set_bits32(dma->ctrl, (param->src_rect_mask_enable ? 0x1 : 0x0), 1, 17);
	dma->tile_scram = set_bits32(dma->tile_scram, ((layer->need_cap & CAP_TILE) ? 0x1 : 0x0), 1, 0);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 0);

	dma->data_addr0 = set_bits32(dma->data_addr0, param->rdma_addr, 32, 0);
	dma->stride0 = set_bits32(dma->stride0,
		((param->rdma_stride / DMA_ALIGN_BYTES) | (param->l2t_interleave_n << 16)), 20, 0);
	dma->stretch_stride0 = set_bits32(dma->stretch_stride0, param->stretched_stride, 19, 0);
	dma->data_num0 = set_bits32(dma->data_num0, param->rdma_data_num, 30, 0);
}

static int rdma_no_compress_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	dss_rect_ltrb_t *clip_rect, dss_rect_t *out_aligned_rect, struct rdma_config_param *param)
{
	int ret;
	dss_rdma_t *dma = &(dpufd->dss_module.rdma[layer->chn_idx]);

	ret = hisi_dss_rdma_addr_aligned(layer, &(param->l2t_interleave_n), &(param->rdma_addr), param->mmu_enable);
	if (ret != 0) {
		DPU_FB_ERR("rdma_addr is not aligned\n");
		return -EINVAL;
	}

	/* aligned_rect */
	ret = hisi_dss_rdma_aligned_rect(layer, &(param->aligned_rect), param->src_rect, param->aligned_pixel);
	if (ret != 0)
		return -EINVAL;

	/* aligned_mask_rect */
	ret = hisi_dss_rdma_aligned_mask_rect(layer,
		&(param->aligned_mask_rect), param->src_rect_mask_enable, param->aligned_pixel);
	if (ret != 0)
		return -EINVAL;

	/* out_rect */
	config_out_aligned_rect(out_aligned_rect, param);

	/* clip_rect */
	if (config_clip_rect(clip_rect, layer, param) != 0)
		return -EINVAL;

	if (param->aligned_pixel == 0) {
		DPU_FB_ERR("The value of aligned_pixel is 0\n");
		return -EINVAL;
	}

	param->rdma_oft_pos.rdma_oft_y0 = param->aligned_rect.top;
	param->rdma_oft_pos.rdma_oft_y1 = param->aligned_rect.bottom;
	param->rdma_oft_pos.rdma_oft_x0 = param->aligned_rect.left / param->aligned_pixel;
	param->rdma_oft_pos.rdma_oft_x1 = param->aligned_rect.right / param->aligned_pixel;

	if (hisi_dss_rdma_oft_pos_check(param->rdma_oft_pos) != 0)
		return -EINVAL;

	param->rdma_addr = hisi_calculate_display_addr(param->mmu_enable, layer,
		&(param->aligned_rect), DSS_ADDR_PLANE0, param->is_pixel_10bit);
	param->rdma_stride = layer->img.stride;
	param->rdma_data_num = (param->rdma_oft_pos.rdma_oft_x1 - param->rdma_oft_pos.rdma_oft_x0 + 1) *
		(param->rdma_oft_pos.rdma_oft_y1 - param->rdma_oft_pos.rdma_oft_y0 + 1);
	ret = hisi_dss_rdma_mask_pos_set(param->rdma_oft_pos, &(param->rdma_mask_pos),
		param->aligned_mask_rect, &(param->src_rect_mask_enable), param->aligned_pixel);
	if (ret != 0)
		return -EINVAL;

	if (param->stretched_line_num > 0) {
		param->stretched_stride = param->stretched_line_num * param->rdma_stride / DMA_ALIGN_BYTES;
		param->rdma_data_num =
			(param->stretch_size_vrt + 1) *
			(param->rdma_oft_pos.rdma_oft_x1 - param->rdma_oft_pos.rdma_oft_x0 + 1);
	} else {
		param->stretch_size_vrt = param->rdma_oft_pos.rdma_oft_y1 - param->rdma_oft_pos.rdma_oft_y0;
		param->stretched_line_num = 0x0;
		param->stretched_stride = 0x0;
	}

	set_dma(dma, layer, param);

	return 0;
}

static int rdma_yuv_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	struct rdma_config_param *param, bool *rdma_stretch_enable)
{
	int ret;
	bool is_yuv_semi_planar = false;
	bool is_yuv_planar = false;
	dss_rdma_t *dma = &(dpufd->dss_module.rdma[layer->chn_idx]);

	is_yuv_semi_planar = is_yuv_semiplanar(layer->img.format);
	is_yuv_planar = is_yuv_plane(layer->img.format);
	if (is_yuv_semi_planar || is_yuv_planar) {
		ret = hisi_dss_rdma_offset_pos_set(layer, &param->rdma_oft_pos, &param->stretched_line_num);
		if (ret != 0)
			return -EINVAL;

		param->rdma_addr = hisi_calculate_display_addr(param->mmu_enable, layer,
			&(param->aligned_rect), DSS_ADDR_PLANE1, param->is_pixel_10bit);
		param->rdma_stride = layer->img.stride_plane1;
		param->rdma_data_num = (param->rdma_oft_pos.rdma_oft_x1 - param->rdma_oft_pos.rdma_oft_x0 + 1) *
			(param->rdma_oft_pos.rdma_oft_y1 - param->rdma_oft_pos.rdma_oft_y0 + 1) * 2;

		if (*rdma_stretch_enable) {
			param->stretched_stride = param->stretched_line_num * param->rdma_stride / DMA_ALIGN_BYTES;
			param->rdma_data_num = (param->stretch_size_vrt + 1) *
				(param->rdma_oft_pos.rdma_oft_x1 - param->rdma_oft_pos.rdma_oft_x0 + 1) * 2;
		} else {
			param->stretched_stride = 0;
		}

		dma->data_addr1 = set_bits32(dma->data_addr1, param->rdma_addr, 32, 0);
		dma->stride1 = set_bits32(dma->stride1,
			((param->rdma_stride / DMA_ALIGN_BYTES) | (param->l2t_interleave_n << 16)), 20, 0);
		dma->stretch_stride1 = set_bits32(dma->stretch_stride1, param->stretched_stride, 19, 0);
		dma->data_num1 = set_bits32(dma->data_num1, param->rdma_data_num, 30, 0);

		if (is_yuv_planar) {
			param->rdma_addr = hisi_calculate_display_addr(param->mmu_enable, layer,
				&(param->aligned_rect), DSS_ADDR_PLANE2, param->is_pixel_10bit);
			param->rdma_stride = layer->img.stride_plane2;

			dma->data_addr2 = set_bits32(dma->data_addr2, param->rdma_addr, 32, 0);
			dma->stride2 = set_bits32(dma->stride2,
				((param->rdma_stride / DMA_ALIGN_BYTES) | (param->l2t_interleave_n << 16)), 20, 0);
			dma->stretch_stride2 = set_bits32(dma->stretch_stride1, param->stretched_stride, 19, 0);
			dma->data_num2 = set_bits32(dma->data_num1, param->rdma_data_num, 30, 0);
		}
	}

	return 0;
}


static int rdma_config_param_prepare(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	struct rdma_config_param *param, struct hisi_ov_compose_flag *ov_compose_flag)
{
	param->src_rect = layer->src_rect;
	param->src_bottom = (uint32_t)param->src_rect.y + param->src_rect.h;
	param->src_right = (uint32_t)param->src_rect.x + param->src_rect.w;

	param->stretched_line_num = is_need_rdma_stretch_bit(dpufd, layer);
	ov_compose_flag->rdma_stretch_enable = (param->stretched_line_num > 0) ? true : false;

	param->mmu_enable = (layer->img.mmu_enable == 1) ? true : false;

	param->rdma_format = hisi_pixel_format_hal2dma(layer->img.format);
	dpu_check_and_return((param->rdma_format < 0), -EINVAL, ERR, "rdmaformat:%d not support\n",
		layer->img.format);

	param->rdma_transform = hisi_transform_hal2dma(layer->transform, layer->chn_idx);
	dpu_check_and_return((param->rdma_transform < 0),
		-EINVAL, ERR, "layer transform:%d not support!\n", layer->transform);

	param->is_pixel_10bit = is_pixel_10bit2dma(param->rdma_format);

	return 0;
}

int hisi_dss_rdma_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	struct hisi_ov_compose_rect *ov_compose_rect,
	struct hisi_ov_compose_flag *ov_compose_flag)
{
	dss_rdma_t *dma = NULL;
	int ret;
	struct rdma_config_param param = {0};
	int ovl_idx;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return(!layer, -EINVAL, ERR, "layer is NULL\n");
	dpu_check_and_return(!ov_compose_rect, -EINVAL, ERR, "ov_compose_rect is NULL\n");
	dpu_check_and_return((!ov_compose_rect->aligned_rect || !ov_compose_rect->clip_rect),
		-EINVAL, ERR, "aligned_rect or clip_rect is NULL\n");
	dpu_check_and_return(!ov_compose_flag, -EINVAL, ERR, "ov_compose_flag is NULL\n");

	ovl_idx = dpufd->ov_req.ovl_idx;
	dpu_check_and_return(((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)), -EINVAL, ERR, "ovl_idx NULL\n");

	ret = rdma_config_param_prepare(dpufd, layer, &param, ov_compose_flag);
	if (ret < 0)
		return -EINVAL;

	dma = &(dpufd->dss_module.rdma[layer->chn_idx]);
	ret = hisi_dss_rdma_set_dpp_and_dma(dpufd, layer, &param.aligned_pixel, &param.src_rect_mask_enable, dma);
	if (ret != 0)
		return -EINVAL;

	if (layer->need_cap & CAP_HFBCD) {
#ifdef SUPPORT_DSS_HFBCD
		ret = hisi_dss_hfbcd_config(dpufd, ovl_idx, layer, ov_compose_rect, ov_compose_flag);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "hisi_dss_hfbcd_config failed! ret = %d\n", ret);
#endif
		return 0;
	}

	if (layer->need_cap & CAP_HEBCD) {
		return 0;
	}

	if (layer->need_cap & CAP_AFBCD) {
		ret = hisi_dss_afbcd_config(dpufd, layer, ov_compose_rect, ov_compose_flag, &param);
		return ret;
	}

	if (rdma_no_compress_config(dpufd, layer, ov_compose_rect->clip_rect,
		ov_compose_rect->aligned_rect, &param) != 0)
		return -EINVAL;

	ret = rdma_yuv_config(dpufd, layer, &param, &ov_compose_flag->rdma_stretch_enable);
	dpu_check_and_return((ret < 0), -EINVAL, ERR, "rdma_yuv_config failed!\n");


	return ret;
}

uint32_t get_rdma_stretch_line_num(dss_layer_t *layer)
{
	int v_stretch_ratio_threshold;
	uint32_t v_stretch_line_num;

	dpu_check_and_return(!layer, 0, ERR, "layer is null\n");
	if ((layer->need_cap & CAP_AFBCD) || (layer->need_cap & CAP_HFBCD) || (layer->need_cap & CAP_HEBCD)) {
		v_stretch_line_num = 0;
	} else {
		if (is_yuv_sp_420(layer->img.format) || is_yuv_p_420(layer->img.format)) {
			v_stretch_ratio_threshold = ((layer->src_rect.h + layer->dst_rect.h - 1) / layer->dst_rect.h);
			v_stretch_line_num = ((layer->src_rect.h / layer->dst_rect.h) / 2) * 2;
		} else {
			v_stretch_ratio_threshold = ((layer->src_rect.h + layer->dst_rect.h - 1) / layer->dst_rect.h);
			v_stretch_line_num = (layer->src_rect.h / layer->dst_rect.h);
		}

		if (v_stretch_ratio_threshold <= g_rdma_stretch_threshold)
			v_stretch_line_num = 0;
	}

	return v_stretch_line_num;
}

