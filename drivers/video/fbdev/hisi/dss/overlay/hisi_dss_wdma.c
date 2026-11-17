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

struct wdma_config_param {
	int wdma_format;
	int wdma_transform;
	uint32_t oft_x0;
	uint32_t oft_x1;
	uint32_t oft_y0;
	uint32_t oft_y1;
	uint32_t data_num;
	uint32_t wdma_addr;
	uint32_t wdma_stride;
	uint32_t wdma_buf_width;
	uint32_t wdma_buf_height;
	dss_rect_t in_rect;
	int aligned_pixel;
	uint32_t l2t_interleave_n;
	bool mmu_enable;
	dss_rect_t *ov_block;
};

struct wdma_config_afbcd_param {
	uint32_t afbce_hreg_pic_blks;
	uint32_t afbc_header_addr;
	uint32_t afbc_header_stride;
	uint32_t afbc_payload_addr;
	uint32_t afbc_payload_stride;
	uint32_t afbc_header_pointer_offset;
	int32_t afbc_header_start_pos;
	dss_rect_ltrb_t afbc_header_rect;
	dss_rect_ltrb_t afbc_payload_rect;
};

static uint32_t hisi_calculate_display_addr_wb(bool mmu_enable, dss_wb_layer_t *wb_layer,
	dss_rect_t aligned_rect, dss_rect_t *ov_block_rect, int add_type)
{
	uint32_t addr;
	uint32_t dst_addr = 0;
	uint32_t stride = 0;
	uint32_t offset;
	int left;
	int top;
	int bpp;

	if (wb_layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		left = wb_layer->dst_rect.x;
		top = ov_block_rect->x - wb_layer->dst_rect.x + wb_layer->dst_rect.y;
	} else {
		left = aligned_rect.x;
		top = aligned_rect.y;
	}

	if (add_type == DSS_ADDR_PLANE0) {
		stride = wb_layer->dst.stride;
		dst_addr = mmu_enable ? wb_layer->dst.vir_addr : wb_layer->dst.phy_addr;
	} else if (add_type == DSS_ADDR_PLANE1) {
		stride = wb_layer->dst.stride_plane1;
		offset = wb_layer->dst.offset_plane1;
		dst_addr = mmu_enable ? (wb_layer->dst.vir_addr + offset) : (wb_layer->dst.phy_addr + offset);
		top /= 2;  /* half valve of top */
	} else {
		DPU_FB_ERR("NOT SUPPORT this add_type[%d].\n", add_type);
	}

	bpp = wb_layer->dst.bpp;
	addr = dst_addr + left * bpp + top * stride;

	if (g_debug_ovl_offline_composer)
		DPU_FB_INFO("addr=0x%x,dst_addr=0x%x,left=%d,top=%d,stride=%d,bpp=%d\n",
			addr, dst_addr, left, top, wb_layer->dst.stride, bpp);

	return addr;
}

void hisi_dss_wdma_init(const char __iomem *wdma_base, dss_wdma_t *s_wdma)
{
	if (!wdma_base) {
		DPU_FB_ERR("wdma_base is NULL\n");
		return;
	}
	if (!s_wdma) {
		DPU_FB_ERR("s_wdma is NULL\n");
		return;
	}

	memset(s_wdma, 0, sizeof(dss_wdma_t));

	s_wdma->oft_x0 = inp32(wdma_base + DMA_OFT_X0);
	s_wdma->oft_y0 = inp32(wdma_base + DMA_OFT_Y0);
	s_wdma->oft_x1 = inp32(wdma_base + DMA_OFT_X1);
	s_wdma->oft_y1 = inp32(wdma_base + DMA_OFT_Y1);
	s_wdma->mask0 = inp32(wdma_base + DMA_MASK0);
	s_wdma->mask1 = inp32(wdma_base + DMA_MASK1);
	s_wdma->stretch_size_vrt = inp32(wdma_base + DMA_STRETCH_SIZE_VRT);
	s_wdma->ctrl = inp32(wdma_base + DMA_CTRL);
	s_wdma->tile_scram = inp32(wdma_base + DMA_TILE_SCRAM);
	s_wdma->sw_mask_en = inp32(wdma_base + WDMA_DMA_SW_MASK_EN);
	s_wdma->start_mask0 = inp32(wdma_base + WDMA_DMA_START_MASK0);
	s_wdma->end_mask0 = inp32(wdma_base + WDMA_DMA_END_MASK1);
	s_wdma->start_mask1 = inp32(wdma_base + WDMA_DMA_START_MASK1);
	s_wdma->end_mask1 = inp32(wdma_base + WDMA_DMA_END_MASK1);
	s_wdma->data_addr = inp32(wdma_base + DMA_DATA_ADDR0);
	s_wdma->stride0 = inp32(wdma_base + DMA_STRIDE0);
	s_wdma->data1_addr = inp32(wdma_base + DMA_DATA_ADDR1);
	s_wdma->stride1 = inp32(wdma_base + DMA_STRIDE1);
	s_wdma->stretch_stride = inp32(wdma_base + DMA_STRETCH_STRIDE0);
	s_wdma->data_num = inp32(wdma_base + DMA_DATA_NUM0);

	s_wdma->ch_rd_shadow = inp32(wdma_base + CH_RD_SHADOW);
	s_wdma->ch_ctl = inp32(wdma_base + CH_CTL);
	s_wdma->ch_secu_en = inp32(wdma_base + CH_SECU_EN);
	s_wdma->ch_sw_end_req = inp32(wdma_base + CH_SW_END_REQ);

	s_wdma->afbce_hreg_pic_blks = inp32(wdma_base + AFBCE_HREG_PIC_BLKS);
	s_wdma->afbce_hreg_format = inp32(wdma_base + AFBCE_HREG_FORMAT);
	s_wdma->afbce_hreg_hdr_ptr_l0 = inp32(wdma_base + AFBCE_HREG_HDR_PTR_L0);
	s_wdma->afbce_hreg_pld_ptr_l0 = inp32(wdma_base + AFBCE_HREG_PLD_PTR_L0);
	s_wdma->afbce_picture_size = inp32(wdma_base + AFBCE_PICTURE_SIZE);
	s_wdma->afbce_ctl = inp32(wdma_base + AFBCE_CTL);
	s_wdma->afbce_header_srtide = inp32(wdma_base + AFBCE_HEADER_SRTIDE);
	s_wdma->afbce_payload_stride = inp32(wdma_base + AFBCE_PAYLOAD_STRIDE);
	s_wdma->afbce_enc_os_cfg = inp32(wdma_base + AFBCE_ENC_OS_CFG);
	s_wdma->afbce_mem_ctrl = inp32(wdma_base + AFBCE_MEM_CTRL);
}

static void set_wdma_afbc_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *wdma_base, dss_wdma_t *s_wdma)
{
	dpufd->set_reg(dpufd, wdma_base + AFBCE_HREG_PIC_BLKS, s_wdma->afbce_hreg_pic_blks, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_HREG_FORMAT, s_wdma->afbce_hreg_format, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_HREG_HDR_PTR_L0, s_wdma->afbce_hreg_hdr_ptr_l0, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_HREG_PLD_PTR_L0, s_wdma->afbce_hreg_pld_ptr_l0, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_PICTURE_SIZE, s_wdma->afbce_picture_size, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_HEADER_SRTIDE, s_wdma->afbce_header_srtide, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_PAYLOAD_STRIDE, s_wdma->afbce_payload_stride, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_ENC_OS_CFG, s_wdma->afbce_enc_os_cfg, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_THRESHOLD, s_wdma->afbce_threshold, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_SCRAMBLE_MODE, s_wdma->afbce_scramble_mode, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + AFBCE_HEADER_POINTER_OFFSET,
		s_wdma->afbce_header_pointer_offset, 32, 0);
#ifdef SUPPORT_DSS_AFBC_ENHANCE
	dpufd->set_reg(dpufd, wdma_base + AFBCE_CREG_FBCE_CTRL_MODE, 0x0, 32, 0);
#endif
}

void hisi_dss_wdma_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *wdma_base, dss_wdma_t *s_wdma)
{
	dpu_check_and_no_retval((dpufd == NULL), ERR, "dpufd is NULL!\n");
	dpu_check_and_no_retval((wdma_base == NULL), ERR, "wdma_base is NULL!\n");
	dpu_check_and_no_retval((s_wdma == NULL), ERR, "s_wdma is NULL!\n");

	dpufd->set_reg(dpufd, wdma_base + CH_REG_DEFAULT, 0x1, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + CH_REG_DEFAULT, 0x0, 32, 0);

	dpufd->set_reg(dpufd, wdma_base + DMA_OFT_X0, s_wdma->oft_x0, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_OFT_Y0, s_wdma->oft_y0, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_OFT_X1, s_wdma->oft_x1, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_OFT_Y1, s_wdma->oft_y1, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_CTRL, s_wdma->ctrl, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_TILE_SCRAM, s_wdma->tile_scram, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + WDMA_DMA_SW_MASK_EN, s_wdma->sw_mask_en, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + WDMA_DMA_START_MASK0, s_wdma->start_mask0, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + WDMA_DMA_END_MASK0, s_wdma->end_mask0, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_DATA_ADDR0, s_wdma->data_addr, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_STRIDE0, s_wdma->stride0, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_DATA_ADDR1, s_wdma->data1_addr, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_STRIDE1, s_wdma->stride1, 32, 0);

	dpufd->set_reg(dpufd, wdma_base + CH_CTL, s_wdma->ch_ctl, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + ROT_SIZE, s_wdma->rot_size, 32, 0);

	dpufd->set_reg(dpufd, wdma_base + DMA_BUF_SIZE, s_wdma->dma_buf_size, 32, 0);

	if (s_wdma->afbc_used == 1)
		set_wdma_afbc_reg(dpufd, wdma_base, s_wdma);

#ifdef SUPPORT_DSS_HFBCE
	if (s_wdma->hfbce_used == 1)
		hisi_dss_hfbce_set_reg(dpufd, wdma_base, s_wdma);
#endif

}

static int hisi_dss_wdma_afbc_check_header(dss_wb_layer_t *layer, dss_rect_t in_rect)
{
	if (layer == NULL) {
		DPU_FB_ERR("layer is NULL\n");
		return -EINVAL;
	}

	if ((layer->dst.width & (AFBC_HEADER_ADDR_ALIGN - 1)) ||
		(layer->dst.height & (AFBC_BLOCK_ALIGN - 1))) {
		DPU_FB_ERR("%s[%d] is not %d bytes aligned, or %s[%d] is not %d bytes aligned!\n",
			"wb_layer img width", layer->dst.width, AFBC_HEADER_ADDR_ALIGN,
			"img heigh", layer->dst.height, AFBC_BLOCK_ALIGN);
		return -EINVAL;
	}

	if ((in_rect.w < AFBC_PIC_WIDTH_MIN) || (in_rect.w > AFBCE_IN_WIDTH_MAX) ||
		(in_rect.h < AFBC_PIC_HEIGHT_MIN) || (in_rect.h > AFBC_PIC_HEIGHT_MAX) ||
		((uint32_t)(in_rect.w) & (AFBC_BLOCK_ALIGN - 1)) || ((uint32_t)(in_rect.h) & (AFBC_BLOCK_ALIGN - 1))) {
		DPU_FB_ERR("afbce in_rect[%d,%d, %d,%d] is out of range!\n",
			in_rect.x, in_rect.y, in_rect.w, in_rect.h);
		return -EINVAL;
	}

	return  0;
}

static int hisi_dss_wdma_afbc_check_payload(dss_wb_layer_t *layer, dss_rect_ltrb_t afbc_payload_rect,
	dss_rect_t in_rect, uint32_t *afbc_payload_addr, uint32_t *afbc_payload_stride)
{
	uint32_t stride_align = 0;
	uint32_t addr_align = 0;

	if (layer == NULL) {
		DPU_FB_ERR("layer is NULL\n");
		return -EINVAL;
	}
	if (afbc_payload_addr == NULL || afbc_payload_stride == NULL) {
		DPU_FB_ERR("stride and addr is NULL\n");
		return -EINVAL;
	}

	if (layer->dst.bpp == 4) {  /* Bit Per Pixel */
		stride_align = AFBC_PAYLOAD_STRIDE_ALIGN_32;
		addr_align = AFBC_PAYLOAD_ADDR_ALIGN_32;
	} else if (layer->dst.bpp == 2) {  /* Bit Per Pixel */
		stride_align = AFBC_PAYLOAD_STRIDE_ALIGN_16;
		addr_align = AFBC_PAYLOAD_ADDR_ALIGN_16;
	} else {
		DPU_FB_ERR("bpp[%d] not supported!\n", layer->dst.bpp);
		return -EINVAL;
	}

	*afbc_payload_stride = layer->dst.afbc_payload_stride;
	if (layer->dst.afbc_scramble_mode != DSS_AFBC_SCRAMBLE_MODE2)
		*afbc_payload_stride = (layer->dst.width / AFBC_BLOCK_ALIGN) * stride_align;

	*afbc_payload_addr = layer->dst.afbc_payload_addr +
		((uint32_t)afbc_payload_rect.top / AFBC_BLOCK_ALIGN) * (*afbc_payload_stride) +
		((uint32_t)afbc_payload_rect.left / AFBC_BLOCK_ALIGN) * stride_align;

	if ((*afbc_payload_addr & (addr_align - 1)) ||
		(*afbc_payload_stride & (stride_align - 1))) {
		DPU_FB_ERR("%s[0x%x] is not %d bytes aligned, or %s[0x%x] is not %d bytes aligned!\n",
			"afbc_payload_addr", *afbc_payload_addr, addr_align,
			"afbc_payload_stride", *afbc_payload_stride, stride_align);
			return -EINVAL;
	}

	if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer)
		DPU_FB_INFO("aligned_rect[%d,%d,%d,%d], afbc_rect[%d,%d,%d,%d]!\n",
			in_rect.x, in_rect.y,
			DSS_WIDTH(in_rect.x + in_rect.w), DSS_WIDTH(in_rect.y + in_rect.h),
			afbc_payload_rect.left, afbc_payload_rect.top,
			afbc_payload_rect.right, afbc_payload_rect.bottom);

	return 0;
}

static int config_afbc_header_and_payload_rect(dss_wb_layer_t *layer,
	struct wdma_config_afbcd_param *afbcd_param, struct wdma_config_param *param)
{
	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		afbcd_param->afbc_header_rect.left = ALIGN_DOWN((uint32_t)layer->dst_rect.x, AFBC_HEADER_ADDR_ALIGN);
		afbcd_param->afbc_header_rect.top = ALIGN_DOWN((uint32_t)(layer->dst_rect.y +
			(param->ov_block->x - layer->dst_rect.x)),
			AFBC_BLOCK_ALIGN);

		afbcd_param->afbc_payload_rect.left = ALIGN_DOWN((uint32_t)layer->dst_rect.x, AFBC_BLOCK_ALIGN);
		afbcd_param->afbc_payload_rect.top = afbcd_param->afbc_header_rect.top;

		afbcd_param->afbc_header_start_pos = (layer->dst_rect.x - afbcd_param->afbc_header_rect.left) /
			AFBC_BLOCK_ALIGN;
	} else {
		afbcd_param->afbc_header_rect.left = ALIGN_DOWN((uint32_t)param->in_rect.x, AFBC_HEADER_ADDR_ALIGN);
		afbcd_param->afbc_header_rect.top = ALIGN_DOWN((uint32_t)param->in_rect.y, AFBC_BLOCK_ALIGN);

		afbcd_param->afbc_payload_rect.left = ALIGN_DOWN((uint32_t)param->in_rect.x, AFBC_BLOCK_ALIGN);
		afbcd_param->afbc_payload_rect.top = afbcd_param->afbc_header_rect.top;

		afbcd_param->afbc_header_start_pos = (param->in_rect.x - afbcd_param->afbc_header_rect.left) /
			AFBC_BLOCK_ALIGN;
	}

	if (afbcd_param->afbc_header_start_pos < 0) {
		DPU_FB_ERR("afbc_header_start_pos[%d] is invalid!\n", afbcd_param->afbc_header_start_pos);
		return -EINVAL;
	}

#ifdef SUPPORT_DSS_AFBC_ENHANCE
	if (afbcd_param->afbc_header_start_pos != 0) {
		DPU_FB_ERR("transform[%d], dst_rect.x[%d], afbc_header_rect.left[%d],"
				"in_rect.x[%d], afbc_header_start_pos[%d]is err.\n",
			layer->transform, layer->dst_rect.x, afbcd_param->afbc_header_rect.left,
			param->in_rect.x, afbcd_param->afbc_header_start_pos);
			return -EINVAL;
		return -EINVAL;
	}
#endif

	return 0;
}

static int config_wdma_afbc_header(dss_wb_layer_t *layer, struct wdma_config_afbcd_param *afbcd_param)
{
	afbcd_param->afbc_header_stride = (layer->dst.width / AFBC_BLOCK_ALIGN) * AFBC_HEADER_STRIDE_BLOCK;
	afbcd_param->afbc_header_pointer_offset = ((uint32_t)afbcd_param->afbc_header_rect.top /
		AFBC_BLOCK_ALIGN) * afbcd_param->afbc_header_stride +
		((uint32_t)afbcd_param->afbc_header_rect.left / AFBC_BLOCK_ALIGN) * AFBC_HEADER_STRIDE_BLOCK;
	afbcd_param->afbc_header_addr = layer->dst.afbc_header_addr + afbcd_param->afbc_header_pointer_offset;

	if ((afbcd_param->afbc_header_addr & (AFBC_HEADER_ADDR_ALIGN - 1)) ||
		(afbcd_param->afbc_header_stride & (AFBC_HEADER_STRIDE_ALIGN - 1))) {
		DPU_FB_ERR("wb_layer afbc_header_addr:0x%x is not %d bytes aligned, "
			"or afbc_header_stride:0x%x is not %d bytes aligned!\n",
			afbcd_param->afbc_header_addr, AFBC_HEADER_ADDR_ALIGN,
			afbcd_param->afbc_header_stride, AFBC_HEADER_STRIDE_ALIGN);
		return -EINVAL;
	}

	return 0;
}

static void set_wdma_afbce(dss_wb_layer_t *layer, dss_wdma_t *wdma, bool last_block,
	struct wdma_config_param *param, struct wdma_config_afbcd_param afbcd_param)
{
	wdma->ctrl = set_bits32(wdma->ctrl, param->wdma_format, 5, 3);
	wdma->ctrl = set_bits32(wdma->ctrl, (param->mmu_enable ? 0x1 : 0x0), 1, 8);
	wdma->ctrl = set_bits32(wdma->ctrl, param->wdma_transform, 3, 9);
	if (last_block)
		wdma->ch_ctl = set_bits32(wdma->ch_ctl, 0x1d, 5, 0);
	else
		wdma->ch_ctl = set_bits32(wdma->ch_ctl, 0xd, 5, 0);

	wdma->rot_size = set_bits32(wdma->rot_size,
		(DSS_WIDTH((uint32_t)param->in_rect.w) | (DSS_HEIGHT((uint32_t)param->in_rect.h) << 16)), 32, 0);

	wdma->afbce_hreg_pic_blks = set_bits32(wdma->afbce_hreg_pic_blks, afbcd_param.afbce_hreg_pic_blks, 24, 0);
	/* color transform */
	wdma->afbce_hreg_format = set_bits32(wdma->afbce_hreg_format,
		(is_yuv_package(layer->dst.format) ? 0x0 : 0x1), 1, 21);
	wdma->afbce_hreg_hdr_ptr_l0 = set_bits32(wdma->afbce_hreg_hdr_ptr_l0, afbcd_param.afbc_header_addr, 32, 0);
	wdma->afbce_hreg_pld_ptr_l0 = set_bits32(wdma->afbce_hreg_pld_ptr_l0, afbcd_param.afbc_payload_addr, 32, 0);
	wdma->afbce_picture_size = set_bits32(wdma->afbce_picture_size,
		((DSS_WIDTH((uint32_t)param->in_rect.w) << 16) | DSS_HEIGHT((uint32_t)param->in_rect.h)), 32, 0);
	wdma->afbce_header_srtide = set_bits32(wdma->afbce_header_srtide,
		(((uint32_t)afbcd_param.afbc_header_start_pos << 14) | afbcd_param.afbc_header_stride), 16, 0);
	wdma->afbce_payload_stride = set_bits32(wdma->afbce_payload_stride, afbcd_param.afbc_payload_stride, 20, 0);
	wdma->afbce_enc_os_cfg = set_bits32(wdma->afbce_enc_os_cfg, DSS_AFBCE_ENC_OS_CFG_DEFAULT_VAL, 3, 0);
	wdma->afbce_mem_ctrl = set_bits32(wdma->afbce_mem_ctrl, 0x0, 12, 0);
	wdma->afbce_threshold = set_bits32(wdma->afbce_threshold, 0x2, 32, 0);
	wdma->afbce_header_pointer_offset = set_bits32(wdma->afbce_header_pointer_offset,
		afbcd_param.afbc_header_pointer_offset, 32, 0);
	/* afbcd_scramble_mode */
	wdma->afbce_scramble_mode = set_bits32(wdma->afbce_scramble_mode,
		layer->dst.afbc_scramble_mode, 2, 0);
}

static int hisi_dss_wdma_afbce_config(struct dpu_fb_data_type *dpufd, dss_wb_layer_t *layer,
	bool last_block, struct wdma_config_param *param)
{
	dss_wdma_t *wdma = NULL;
	struct wdma_config_afbcd_param afbcd_param = {0};
	int ret;

	wdma = &(dpufd->dss_module.wdma[layer->chn_idx]);

	wdma->afbc_used = 1;

	ret = hisi_dss_wdma_afbc_check_header(layer, param->in_rect);
	if (ret != 0) {
		DPU_FB_ERR("hisi_dss_afbc header error! ret = %d\n", ret);
		return -EINVAL;
	}

	ret = config_afbc_header_and_payload_rect(layer, &afbcd_param, param);
	if (ret < 0) {
		DPU_FB_ERR("config_afbc_header_and_payload error! ret = %d\n", ret);
		return -EINVAL;
	}

	afbcd_param.afbce_hreg_pic_blks = (param->in_rect.w / AFBC_BLOCK_ALIGN) *
		(param->in_rect.h / AFBC_BLOCK_ALIGN) - 1;

	/* afbc header */
	ret = config_wdma_afbc_header(layer, &afbcd_param);
	if (ret < 0) {
		DPU_FB_ERR("config_afbc_header error! ret = %d\n", ret);
		return -EINVAL;
	}

	/* afbc payload */
	ret = hisi_dss_wdma_afbc_check_payload(layer, afbcd_param.afbc_payload_rect, param->in_rect,
		&afbcd_param.afbc_payload_addr, &afbcd_param.afbc_payload_stride);
	if (ret != 0) {
		DPU_FB_ERR("hisi_dss_afbc payload error! ret = %d\n", ret);
		return -EINVAL;
	}

	set_wdma_afbce(layer, wdma, last_block, param, afbcd_param);

	return 0;
}

static int prepare_wdma_common_param(dss_wb_layer_t *layer, dss_rect_t aligned_rect,
	struct wdma_config_param *param)
{
	param->wdma_format = hisi_pixel_format_hal2dma(layer->dst.format);
	dpu_check_and_return((param->wdma_format < 0), -EINVAL, ERR, "hisi_pixel_format_hal2dma failed!\n");

	param->in_rect = aligned_rect;
	param->aligned_pixel = DMA_ALIGN_BYTES / layer->dst.bpp;

	param->wdma_transform = hisi_transform_hal2dma(layer->transform, layer->chn_idx);
	dpu_check_and_return((param->wdma_transform < 0), -EINVAL, ERR, "hisi_transform_hal2dma failed!\n");

	param->mmu_enable = (layer->dst.mmu_enable == 1) ? true : false;
	param->wdma_addr = param->mmu_enable ? layer->dst.vir_addr : layer->dst.phy_addr;

	return 0;
}

static int config_wdma_l2t_interleave(dss_wb_layer_t *layer, struct wdma_config_param *param)
{
	if (layer->need_cap & CAP_TILE) {
		param->l2t_interleave_n = (uint32_t)hisi_get_rdma_tile_interleave(layer->dst.stride);
		dpu_check_and_return((param->l2t_interleave_n < MIN_INTERLEAVE), -EINVAL, ERR,
			"tile stride should be 256*2^n, error stride:%d!\n", layer->dst.stride);

		if (param->wdma_addr & (TILE_DMA_ADDR_ALIGN - 1)) {
			DPU_FB_ERR("tile wdma_addr:0x%x is not %d bytes aligned.\n",
				param->wdma_addr, TILE_DMA_ADDR_ALIGN);
			return -EINVAL;
		}
	}

	return 0;
}

static void config_wdma_oft(dss_wb_layer_t *layer, struct wdma_config_param *param)
{
	int temp;

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		temp = param->in_rect.w;
		param->in_rect.w = param->in_rect.h;
		param->in_rect.h = temp;

		param->oft_x0 = 0;
		param->oft_x1 = DSS_WIDTH(param->in_rect.w) / param->aligned_pixel;
		param->oft_y0 = 0;
		param->oft_y1 = DSS_HEIGHT(param->ov_block->w);
	} else {
		param->oft_x0 = param->in_rect.x / param->aligned_pixel;
		param->oft_x1 = DSS_WIDTH(param->in_rect.x + param->in_rect.w) / param->aligned_pixel;
		param->oft_y0 = param->in_rect.y;
		param->oft_y1 = DSS_HEIGHT(param->in_rect.y + param->in_rect.h);
	}
}

static void set_wdma(dss_wb_layer_t *layer, dss_rect_t aligned_rect,
	struct wdma_config_param *param, bool last_block, dss_wdma_t *wdma)
{
	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		wdma->rot_size = set_bits32(wdma->rot_size,
			((uint32_t)DSS_WIDTH(param->ov_block->w) |
			((uint32_t)DSS_HEIGHT(aligned_rect.h) << 16)), 32, 0);

		param->wdma_buf_width = DSS_HEIGHT(param->ov_block->h);
		param->wdma_buf_height = DSS_WIDTH(param->ov_block->w);

	} else {
		param->wdma_buf_width = DSS_WIDTH(param->ov_block->w);
		param->wdma_buf_height = DSS_HEIGHT(param->ov_block->h);

	}
	wdma->oft_x0 = set_bits32(wdma->oft_x0, param->oft_x0, 12, 0);
	wdma->oft_y0 = set_bits32(wdma->oft_y0, param->oft_y0, 16, 0);
	wdma->oft_x1 = set_bits32(wdma->oft_x1, param->oft_x1, 12, 0);
	wdma->oft_y1 = set_bits32(wdma->oft_y1, param->oft_y1, 16, 0);

	wdma->ctrl = set_bits32(wdma->ctrl, param->wdma_format, 5, 3);
	wdma->ctrl = set_bits32(wdma->ctrl, param->wdma_transform, 3, 9);
	wdma->ctrl = set_bits32(wdma->ctrl, (param->mmu_enable ? 0x1 : 0x0), 1, 8);
	wdma->data_num = set_bits32(wdma->data_num, param->data_num, 30, 0);

	wdma->ctrl = set_bits32(wdma->ctrl, ((layer->need_cap & CAP_TILE) ? 0x1 : 0x0), 1, 1);
	wdma->tile_scram = set_bits32(wdma->tile_scram, ((layer->need_cap & CAP_TILE) ? 0x1 : 0x0), 1, 0);

	wdma->data_addr = set_bits32(wdma->data_addr, param->wdma_addr, 32, 0);
	wdma->stride0 = set_bits32(wdma->stride0, param->wdma_stride | (param->l2t_interleave_n << 16), 20, 0);

	if (is_yuv_sp_420(layer->dst.format)) {
		param->wdma_addr = hisi_calculate_display_addr_wb(param->mmu_enable,
			layer, param->in_rect, param->ov_block, DSS_ADDR_PLANE1);
		param->wdma_stride = layer->dst.stride_plane1 / DMA_ALIGN_BYTES;
		wdma->data1_addr = set_bits32(wdma->data1_addr, param->wdma_addr, 32, 0);
		wdma->stride1 = set_bits32(wdma->stride1, param->wdma_stride, 13, 0);
	}

	if (last_block)
		wdma->ch_ctl = set_bits32(wdma->ch_ctl, 1, 1, 4);
	else
		wdma->ch_ctl = set_bits32(wdma->ch_ctl, 0, 1, 4);

	wdma->ch_ctl = set_bits32(wdma->ch_ctl, 1, 1, 3);
	wdma->ch_ctl = set_bits32(wdma->ch_ctl, 1, 1, 0);

	wdma->dma_buf_size = set_bits32(wdma->dma_buf_size,
		param->wdma_buf_width | (param->wdma_buf_height << 16), 32, 0);
}

static void print_wdma_info(dss_rect_t aligned_rect, struct wdma_config_param *param)
{
	if (g_debug_ovl_offline_composer)
		DPU_FB_INFO("dma_oft:x0_y0_x1_y1,%d:%d:%d:%d,rot_size:w_h,%d:%d,wdma_buf_size:w_h,%d:%d\n",
		param->oft_x0, param->oft_y0, param->oft_x1, param->oft_y1,
		param->ov_block->w, aligned_rect.h, param->wdma_buf_width, param->wdma_buf_height);
}

int hisi_dss_wdma_config(struct dpu_fb_data_type *dpufd, dss_wb_layer_t *layer,
	dss_rect_t aligned_rect, dss_rect_t *ov_block_rect, bool last_block)
{
	struct wdma_config_param param = {0};
	dss_wdma_t *wdma = NULL;
	int ret;

	dpu_check_and_return((dpufd == NULL || layer == NULL), -EINVAL, ERR, "NULL ptr.\n");
	dpu_check_and_return((ov_block_rect == NULL), -EINVAL, ERR, "NULL ov_block_rect!\n");
	dpu_check_and_return((layer->dst.bpp == 0), -EINVAL, ERR, "zero bpp.\n");

	param.ov_block = ov_block_rect;

	wdma = &(dpufd->dss_module.wdma[layer->chn_idx]);
	dpufd->dss_module.dma_used[layer->chn_idx] = 1;

	ret = prepare_wdma_common_param(layer, aligned_rect, &param);
	dpu_check_and_return((ret < 0), -EINVAL, ERR, "prepare_wdma_common_param failed!\n");

	if (layer->need_cap & CAP_HFBCE) {
#ifdef SUPPORT_DSS_HFBCE
		ret = hisi_dss_hfbce_config(dpufd, layer, aligned_rect, param.ov_block, last_block);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "hisi_dss_hfbce_config failed! ret = %d\n", ret);
#endif
		return 0;
	}

	if (layer->need_cap & CAP_HEBCE) {
		return 0;
	}

	if (layer->need_cap & CAP_AFBCE) {
		ret = hisi_dss_wdma_afbce_config(dpufd, layer, last_block, &param);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "hisi_dss_wdma_afbcd_config failed! ret = %d\n", ret);

		return 0;
	}

	ret = config_wdma_l2t_interleave(layer, &param);
	dpu_check_and_return((ret < 0), -EINVAL, ERR, "NULL config_wdma_l2t_interleave failed!\n");

	config_wdma_oft(layer, &param);

	param.wdma_addr = hisi_calculate_display_addr_wb(param.mmu_enable, layer, param.in_rect,
		param.ov_block, DSS_ADDR_PLANE0);
	param.wdma_stride = layer->dst.stride / DMA_ALIGN_BYTES;

	param.data_num = (param.oft_x1 - param.oft_x0 + 1) * (param.oft_y1 - param.oft_y0 + 1);

	set_wdma(layer, aligned_rect, &param, last_block, wdma);

	print_wdma_info(aligned_rect, &param);

	return 0;
}
