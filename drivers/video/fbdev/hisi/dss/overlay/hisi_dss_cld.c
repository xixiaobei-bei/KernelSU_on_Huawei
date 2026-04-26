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

#include "hisi_dss_cld.h"

void hisi_dss_cld_init(const char __iomem *cld_base, struct dss_cld *s_cld)
{
	if (cld_base == NULL) {
		DPU_FB_ERR("cld_base is NULL!\n");
		return;
	}
	if (s_cld == NULL) {
		DPU_FB_ERR("s_cld is NULL!\n");
		return;
	}

	memset(s_cld, 0, sizeof(*s_cld));
	s_cld->size_vrt = inp32(cld_base + CLD_SIZE_VRT);
	s_cld->size_hrz = inp32(cld_base + CLD_SIZE_HRZ);
	s_cld->cld_rgb = inp32(cld_base + CLD_RGB);
	s_cld->cld_clk_en = inp32(cld_base + CH_CLK_EN);
	s_cld->cld_clk_sel = inp32(cld_base + CH_CLK_SEL);
	s_cld->cld_en = inp32(cld_base + CLD_EN);
}

void hisi_dss_cld_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *cld_base, struct dss_cld *s_cld, int channel)
{
	if (dpufd == NULL) {
		DPU_FB_DEBUG("dpufd is NULL\n");
		return;
	}
	if (cld_base == NULL) {
		DPU_FB_DEBUG("cld_base is NULL\n");
		return;
	}
	if (s_cld == NULL) {
		DPU_FB_DEBUG("s_cld is NULL\n");
		return;
	}

	if (channel != DSS_RCHN_D0)
		return;

	dpufd->set_reg(dpufd, cld_base + CLD_SIZE_VRT, s_cld->size_vrt, 32, 0);
	dpufd->set_reg(dpufd, cld_base + CLD_SIZE_HRZ, s_cld->size_hrz, 32, 0);
	dpufd->set_reg(dpufd, cld_base + CLD_RGB, s_cld->cld_rgb, 32, 0);
	dpufd->set_reg(dpufd, cld_base + CLD_EN, s_cld->cld_en, 32, 0);
	dpufd->set_reg(dpufd, cld_base + CH_CLK_EN, s_cld->cld_clk_en, 1, 10);
	dpufd->set_reg(dpufd, cld_base + CH_CLK_SEL, s_cld->cld_clk_sel, 1, 10);
}

int hisi_dss_cld_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer)
{
	struct dss_cld *cld = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}
	if (layer == NULL) {
		DPU_FB_ERR("layer is NULL\n");
		return -EINVAL;
	}

	if (layer->chn_idx != DSS_RCHN_D0)
		return 0;

	cld = &(dpufd->dss_module.cld);
	dpufd->dss_module.cld_used = 1;

	if (layer->is_cld_layer == 1) {
		cld->size_vrt = layer->src_rect.h - 1;  /* CLD height is rdma height minus 1. */
		cld->size_hrz = layer->src_rect.w - 1;  /*  CLD width is rdma width minus 1. */
		cld->cld_rgb  = layer->color;  /* Can be assigned if is necessary. */
		cld->cld_en   = 1;
	} else {
		cld->size_vrt = 0;  /* CLD height is default */
		cld->size_hrz = 0;  /* CLD width is default */
		cld->cld_rgb  = 0x000000;  /* CLD RGB is default */
		cld->cld_en   = 0;
	}

	return 0;
}

