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

void hisi_dss_mif_init(char __iomem *mif_ch_base,
	dss_mif_t *s_mif, int chn_idx)
{
	uint32_t rw_type;

	if (mif_ch_base == NULL) {
		DPU_FB_ERR("mif_ch_base is NULL\n");
		return;
	}
	if (s_mif == NULL) {
		DPU_FB_ERR("s_mif is NULL\n");
		return;
	}

	memset(s_mif, 0, sizeof(dss_mif_t));

	s_mif->mif_ctrl1 = 0x00000020;
	s_mif->mif_ctrl2 = 0x0;
	s_mif->mif_ctrl3 = 0x0;
	s_mif->mif_ctrl4 = 0x0;
	s_mif->mif_ctrl5 = 0x0;

	rw_type = (chn_idx < DSS_WCHN_W0 || chn_idx == DSS_RCHN_V2) ? 0x0 : 0x1;

	s_mif->mif_ctrl1 = set_bits32(s_mif->mif_ctrl1, 0x0, 1, 5);
	s_mif->mif_ctrl1 = set_bits32(s_mif->mif_ctrl1, rw_type, 1, 17);
}

void hisi_dss_mif_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *mif_ch_base, dss_mif_t *s_mif, int chn_idx)
{
	if (dpufd == NULL) {
		DPU_FB_DEBUG("dpufd is NULL!\n");
		return;
	}

	if (mif_ch_base == NULL) {
		DPU_FB_DEBUG("mif_ch_base is NULL!\n");
		return;
	}

	if (s_mif == NULL) {
		DPU_FB_DEBUG("s_mif is NULL!\n");
		return;
	}

	dpufd->set_reg(dpufd, mif_ch_base + MIF_CTRL1, s_mif->mif_ctrl1, 32, 0);
	dpufd->set_reg(dpufd, mif_ch_base + MIF_CTRL2, s_mif->mif_ctrl2, 32, 0);
	dpufd->set_reg(dpufd, mif_ch_base + MIF_CTRL3, s_mif->mif_ctrl3, 32, 0);
	dpufd->set_reg(dpufd, mif_ch_base + MIF_CTRL4, s_mif->mif_ctrl4, 32, 0);
	dpufd->set_reg(dpufd, mif_ch_base + MIF_CTRL5, s_mif->mif_ctrl5, 32, 0);
}

void hisi_dss_mif_on(struct dpu_fb_data_type *dpufd)
{
	char __iomem *mif_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	mif_base = dpufd->dss_base + DSS_MIF_OFFSET;

	set_reg(mif_base + MIF_ENABLE, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH0_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH1_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH2_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH3_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH4_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH5_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH6_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH7_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH8_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH9_OFFSET + MIF_CTRL0, 0x1, 1, 0);

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V510)
	set_reg(dpufd->dss_base + MIF_CH10_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dpufd->dss_base + MIF_CH11_OFFSET + MIF_CTRL0, 0x1, 1, 0);
#endif

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V345) || \
	defined(CONFIG_HISI_FB_V346)
	set_reg(mif_base + AIF_CMD_RELOAD, 0x1, 1, 0);
#endif

}

static void set_mif_ctrl_accord_to_sel_value(dss_mif_t *mif, dss_img_t *img)
{
	uint32_t *semi_plane2 = NULL;

	semi_plane2 = &mif->mif_ctrl4;

	if (img->stride > 0)
		mif->mif_ctrl5 = set_bits32(mif->mif_ctrl5,
			((img->stride / MIF_STRIDE_UNIT) +
			(((img->stride % MIF_STRIDE_UNIT) > 0) ? 1 : 0)), 20, 0);

	if (is_yuv_semiplanar(img->format)) {
		if (img->stride_plane1 > 0)
			*semi_plane2 = set_bits32(*semi_plane2,
				((img->stride_plane1 / MIF_STRIDE_UNIT) +
				(((img->stride_plane1 % MIF_STRIDE_UNIT) > 0) ? 1 : 0)), 20, 0);
	} else if (is_yuv_plane(img->format)) {
		if (img->stride_plane1 > 0)
			mif->mif_ctrl4 = set_bits32(mif->mif_ctrl4,
				((img->stride_plane1 / MIF_STRIDE_UNIT) +
				(((img->stride_plane1 % MIF_STRIDE_UNIT) > 0) ? 1 : 0)), 20, 0);

		if (img->stride_plane2 > 0)
			mif->mif_ctrl3 = set_bits32(mif->mif_ctrl3,
				((img->stride_plane2 / MIF_STRIDE_UNIT) +
				(((img->stride_plane2 % MIF_STRIDE_UNIT) > 0) ? 1 : 0)), 20, 0);
	} else {
		;
	}
}

static void set_mif_ctrl(struct dpu_fb_data_type *dpufd, dss_mif_t *mif,
		dss_img_t *img, uint32_t invalid_sel)
{
	uint32_t *semi_plane1 = NULL;

	semi_plane1 = &mif->mif_ctrl4;

	mif->mif_ctrl1 = set_bits32(mif->mif_ctrl1, 0x0, 1, 5);
	mif->mif_ctrl1 = set_bits32(mif->mif_ctrl1, invalid_sel, 2, 10);
	mif->mif_ctrl1 = set_bits32(mif->mif_ctrl1, ((invalid_sel == 0) ? 0x1 : 0x0), 1, 19);

	if (invalid_sel == 0) {
		mif->mif_ctrl2 = set_bits32(mif->mif_ctrl2, 0x0, 20, 0);
		mif->mif_ctrl3 = set_bits32(mif->mif_ctrl3, 0x0, 20, 0);
		mif->mif_ctrl4 = set_bits32(mif->mif_ctrl4, 0x0, 20, 0);
		mif->mif_ctrl5 = set_bits32(mif->mif_ctrl5, 0x0, 20, 0);
	} else if ((invalid_sel == 1) || (invalid_sel == 2)) {
		set_mif_ctrl_accord_to_sel_value(mif, img);
	} else if (invalid_sel == 3) {
		if (img->stride > 0)
			mif->mif_ctrl5 = set_bits32(mif->mif_ctrl5, DSS_MIF_CTRL2_INVAL_SEL3_STRIDE_MASK, 4, 16);
		if (is_yuv_semiplanar(img->format)) {
			if (img->stride_plane1 > 0)
				*semi_plane1 = set_bits32(*semi_plane1, 0xE, 4, 16);

		} else if (is_yuv_plane(img->format)) {
			if (img->stride_plane1 > 0)
				mif->mif_ctrl3 = set_bits32(mif->mif_ctrl3, 0xE, 4, 16);

			if (img->stride_plane2 > 0)
				mif->mif_ctrl4 = set_bits32(mif->mif_ctrl4, 0xE, 4, 16);
		} else {
			;  /* do nothing */
		}

		/* Tile
		 * YUV_SP: RDMA0 128KB aligned, RDMA1 64KB aligned
		 * YUV_P: RDMA0 128KB aligned, RDMA1 64KB aligned, RDMA2 64KB aligned
		 * ROT: 256KB aligned.
		 */
	} else {
		DPU_FB_ERR("fb%d, invalid_sel:%d not support!\n", dpufd->index, invalid_sel);
	}
}

int hisi_dss_mif_config(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer, dss_wb_layer_t *wb_layer, bool rdma_stretch_enable)
{
	dss_mif_t *mif = NULL;
	int chn_idx = 0;
	dss_img_t *img = NULL;
	uint32_t transform = 0;
	uint32_t invalid_sel = 0;
	uint32_t need_cap = 0;
	int v_scaling_factor = 0;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}
	if ((layer == NULL) && (wb_layer == NULL)) {
		DPU_FB_ERR("layer and wb_layer is NULL\n");
		return -EINVAL;
	}

	if (wb_layer != NULL) {
		img = &(wb_layer->dst);
		chn_idx = wb_layer->chn_idx;
		transform = wb_layer->transform;
		need_cap = wb_layer->need_cap;
		v_scaling_factor = 1;
	} else {
		img = &(layer->img);
		chn_idx = layer->chn_idx;
		transform = layer->transform;
		need_cap = layer->need_cap;
		v_scaling_factor = layer->src_rect.h / layer->dst_rect.h +
			((layer->src_rect.h % layer->dst_rect.h) > 0 ? 1 : 0);
	}

	mif = &(dpufd->dss_module.mif[chn_idx]);
	dpufd->dss_module.mif_used[chn_idx] = 1;


	if (img->mmu_enable == 0) {
		mif->mif_ctrl1 = set_bits32(mif->mif_ctrl1, 0x1, 1, 5);
	} else {
		if (need_cap & (CAP_AFBCD | CAP_AFBCE | CAP_HFBCD | CAP_HFBCE | CAP_HEBCD | CAP_HEBCE))
			invalid_sel = 0;
		else
			invalid_sel = hisi_dss_mif_get_invalid_sel(img, transform, v_scaling_factor,
				((need_cap & CAP_TILE) ? 1 : 0), rdma_stretch_enable);

		set_mif_ctrl(dpufd, mif, img, invalid_sel);
	}

	return 0;
}
