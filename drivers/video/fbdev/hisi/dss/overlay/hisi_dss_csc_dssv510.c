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

/*
 * DSS CSC
 */
#define CSC_ROW 3
#define CSC_COL 5

struct dss_csc_coe {
	int (*csc_coe_yuv2rgb)[CSC_COL];
	int (*csc_coe_rgb2yuv)[CSC_COL];
};

/*
 * [ p00 p01 p02 idc2 odc2 ]
 * [ p10 p11 p12 idc1 odc1 ]
 * [ p20 p21 p22 idc0 odc0 ]
 */

/* regular rgb<->yuv */
static int g_csc_coe_yuv2rgb601_wide[CSC_ROW][CSC_COL] = {
	{ 0x4000, 0x00000, 0x059ba, 0x000, 0x000 },
	{ 0x4000, 0x1e9fa, 0x1d24c, 0x600, 0x000 },
	{ 0x4000, 0x07168, 0x00000, 0x600, 0x000 },
};

static int g_csc_coe_rgb2yuv601_wide[CSC_ROW][CSC_COL] = {
	{ 0x01323, 0x02591, 0x0074c, 0x000, 0x000 },
	{ 0x1f533, 0x1eacd, 0x02000, 0x000, 0x200 },
	{ 0x02000, 0x1e534, 0x1facc, 0x000, 0x200 },
};

static int g_csc_coe_yuv2rgb601_narrow[CSC_ROW][CSC_COL] = {
	{ 0x4a85, 0x00000, 0x06625, 0x7c0, 0x000 },
	{ 0x4a85, 0x1e6ed, 0x1cbf8, 0x600, 0x000 },
	{ 0x4a85, 0x0811a, 0x00000, 0x600, 0x000 },
};

static int g_csc_coe_rgb2yuv601_narrow[CSC_ROW][CSC_COL] = {
	{ 0x0106f, 0x02044, 0x00644, 0x000, 0x040 },
	{ 0x1f684, 0x1ed60, 0x01c1c, 0x000, 0x200 },
	{ 0x01c1c, 0x1e876, 0x1fb6e, 0x000, 0x200 },
};

static int g_csc_coe_yuv2rgb709_wide[CSC_ROW][CSC_COL] = {
	{ 0x4000, 0x00000, 0x064ca, 0x000, 0x000 },
	{ 0x4000, 0x1f403, 0x1e20a, 0x600, 0x000 },
	{ 0x4000, 0x076c2, 0x00000, 0x600, 0x000 },
};

static int g_csc_coe_rgb2yuv709_wide[CSC_ROW][CSC_COL] = {
	{ 0x00d9b, 0x02dc6, 0x0049f, 0x000, 0x000 },
	{ 0x1f8ab, 0x1e755, 0x02000, 0x000, 0x200 },
	{ 0x02000, 0x1e2ef, 0x1fd11, 0x000, 0x200 },
};

static int g_csc_coe_yuv2rgb709_narrow[CSC_ROW][CSC_COL] = {
	{ 0x4a85, 0x00000, 0x072bc, 0x7c0, 0x000 },
	{ 0x4a85, 0x1f25a, 0x1dde5, 0x600, 0x000 },
	{ 0x4a85, 0x08732, 0x00000, 0x600, 0x000 },
};

static int g_csc_coe_rgb2yuv709_narrow[CSC_ROW][CSC_COL] = {
	{ 0x00baf, 0x02750, 0x003f8, 0x000, 0x040 },
	{ 0x1f98f, 0x1ea55, 0x01c1c, 0x000, 0x200 },
	{ 0x01c1c, 0x1e678, 0x1fd6c, 0x000, 0x200 },
};

static int g_csc_coe_rgb2yuv2020[CSC_ROW][CSC_COL] = {
	{ 0x04000, 0x00000, 0x00000, 0x00000, 0x00000 },
	{ 0x00000, 0x04000, 0x00000, 0x00600, 0x00000 },
	{ 0x00000, 0x00000, 0x04000, 0x00600, 0x00000 },
};

static int g_csc_coe_yuv2rgb2020[CSC_ROW][CSC_COL] = {
	{ 0x04A85, 0x00000, 0x06B6F, 0x007C0, 0x00000 },
	{ 0x04A85, 0x1F402, 0x1D65F, 0x00600, 0x00000 },
	{ 0x04A85, 0x08912, 0x00000, 0x00600, 0x00000 },
};

/* RGB: srgb to P3 */
static int g_csc_coe_srgb2p3[CSC_ROW][CSC_COL] = {
	{ 0x034A3, 0x00B5D, 0x00000, 0x00000, 0x00000 },
	{ 0x00220, 0x03DE0, 0x00000, 0x00000, 0x00000 },
	{ 0x00118, 0x004A2, 0x03A46, 0x00000, 0x00000 },
};

/* YUV:srgb <-> RGB:P3 */
static int g_csc_coe_yuv2p3601_narrow[CSC_ROW][CSC_COL] = {
	{ 0x4a85, 0x1fb8c, 0x04ac6, 0x7c0, 0x000 },
	{ 0x4a85, 0x1e7c2, 0x1d116, 0x600, 0x000 },
	{ 0x4a85, 0x073bc, 0x1fdfa, 0x600, 0x000 },
};

static int g_csc_coe_yuv2p3601_wide[CSC_ROW][CSC_COL] = {
	{ 0x4000, 0x1fc17, 0x041af, 0x000, 0x000 },
	{ 0x4000, 0x1e6cc, 0x1d6cb, 0x600, 0x000 },
	{ 0x4000, 0x065aa, 0x1fe39, 0x600, 0x000 },
};

static int g_csc_coe_yuv2p3709_narrow[CSC_ROW][CSC_COL] = {
	{ 0x4a85, 0x1fd94, 0x0584f, 0x7c0, 0x000 },
	{ 0x4a85, 0x1f2ce, 0x1e2d6, 0x600, 0x000 },
	{ 0x4a85, 0x07a1c, 0x1ff7e, 0x600, 0x000 },
};

static int g_csc_coe_yuv2p3709_wide[CSC_ROW][CSC_COL] = {
	{ 0x4000, 0x1fddf, 0x04d93, 0x000, 0x000 },
	{ 0x4000, 0x1f469, 0x1E661, 0x600, 0x000 },
	{ 0x4000, 0x06b43, 0x1ff8d, 0x600, 0x000 },
};

void hisi_dss_csc_init(const char __iomem *csc_base, dss_csc_t *s_csc)
{
	if (!csc_base) {
		DPU_FB_ERR("csc_base is NULL\n");
		return;
	}
	if (!s_csc) {
		DPU_FB_ERR("s_csc is NULL\n");
		return;
	}

	memset(s_csc, 0, sizeof(dss_csc_t));

	s_csc->idc0 = inp32(csc_base + CSC_IDC0);
	s_csc->idc2 = inp32(csc_base + CSC_IDC2);
	s_csc->odc0 = inp32(csc_base + CSC_ODC0);
	s_csc->odc2 = inp32(csc_base + CSC_ODC2);

	s_csc->p00 = inp32(csc_base + CSC_P00);
	s_csc->p01 = inp32(csc_base + CSC_P01);
	s_csc->p02 = inp32(csc_base + CSC_P02);
	s_csc->p10 = inp32(csc_base + CSC_P10);
	s_csc->p11 = inp32(csc_base + CSC_P11);
	s_csc->p12 = inp32(csc_base + CSC_P12);
	s_csc->p20 = inp32(csc_base + CSC_P20);
	s_csc->p21 = inp32(csc_base + CSC_P21);
	s_csc->p22 = inp32(csc_base + CSC_P22);
	s_csc->icg_module = inp32(csc_base + CSC_ICG_MODULE);
}

void hisi_dss_csc_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *csc_base, dss_csc_t *s_csc)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is null\n");
		return;
	}

	if (!csc_base) {
		DPU_FB_ERR("csc_base is null\n");
		return;
	}

	if (!s_csc) {
		DPU_FB_ERR("s_csc is null\n");
		return;
	}

	dpufd->set_reg(dpufd, csc_base + CSC_IDC0, s_csc->idc0, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_IDC2, s_csc->idc2, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_ODC0, s_csc->odc0, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_ODC2, s_csc->odc2, 32, 0);

	dpufd->set_reg(dpufd, csc_base + CSC_P00, s_csc->p00, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_P01, s_csc->p01, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_P02, s_csc->p02, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_P10, s_csc->p10, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_P11, s_csc->p11, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_P12, s_csc->p12, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_P20, s_csc->p20, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_P21, s_csc->p21, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_P22, s_csc->p22, 32, 0);
	dpufd->set_reg(dpufd, csc_base + CSC_ICG_MODULE, s_csc->icg_module, 32, 0);
}

static bool is_pcsc_needed(dss_layer_t *layer)
{
	if (layer->chn_idx != DSS_RCHN_V0)
		return false;

	if (layer->need_cap & CAP_2D_SHARPNESS)
		return true;

	/* horizental shrink is not supported by arsr2p */
	if ((layer->dst_rect.h != layer->src_rect.h) || (layer->dst_rect.w > layer->src_rect.w))
		return true;

	return false;
}

static void hisi_dss_csc_coef_array_config(dss_csc_t *csc, int **p, int j)
{
	csc->icg_module = set_bits32(csc->icg_module, 0x1, 1, 0);
	/* The follow code get one by one array value to calculate csc coef value
	 * It contains lots of fixed numbers
	 */
	csc->idc0 = set_bits32(csc->idc0, (uint32_t)(*((int *)p + 2 * j + 3)) |
		((uint32_t)(*((int *)p + 1 * j + 3)) << 16), 27, 0);
	csc->idc2 = set_bits32(csc->idc2, *((int *)p + 0 * j + 3), 11, 0);

	csc->odc0 = set_bits32(csc->odc0, (uint32_t)(*((int *)p + 2 * j + 4)) |
		((uint32_t)(*((int *)p + 1 * j + 4)) << 16), 27, 0);
	csc->odc2 = set_bits32(csc->odc2, *((int *)p + 0 * j + 4), 11, 0);

	csc->p00 = set_bits32(csc->p00, *((int *)p + 0 * j + 0), 17, 0);
	csc->p01 = set_bits32(csc->p01, *((int *)p + 0 * j + 1), 17, 0);
	csc->p02 = set_bits32(csc->p02, *((int *)p + 0 * j + 2), 17, 0);

	csc->p10 = set_bits32(csc->p10, *((int *)p + 1 * j + 0), 17, 0);
	csc->p11 = set_bits32(csc->p11, *((int *)p + 1 * j + 1), 17, 0);
	csc->p12 = set_bits32(csc->p12, *((int *)p + 1 * j + 2), 17, 0);

	csc->p20 = set_bits32(csc->p20, *((int *)p + 2 * j + 0), 17, 0);
	csc->p21 = set_bits32(csc->p21, *((int *)p + 2 * j + 1), 17, 0);
	csc->p22 = set_bits32(csc->p22, *((int *)p + 2 * j + 2), 17, 0);
}

bool csc_need_p3_process(uint32_t csc_mode)
{
	return csc_mode & P3_PROCESS_NEEDED_LAYER;
}

static void csc_get_regular_coef(uint32_t csc_mode, struct dss_csc_coe *csc_coe)
{
	switch (csc_mode) {
	case DSS_CSC_601_WIDE:
		csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2rgb601_wide;
		csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv601_wide;
		break;
	case DSS_CSC_601_NARROW:
		csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2rgb601_narrow;
		csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv601_narrow;
		break;
	case DSS_CSC_709_WIDE:
		csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2rgb709_wide;
		csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv709_wide;
		break;
	case DSS_CSC_709_NARROW:
		csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2rgb709_narrow;
		csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv709_narrow;
		break;
	case DSS_CSC_2020:
		csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2rgb2020;
		csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv2020;
		break;
	case DSS_CSC_DISPLAY_P3:
		/* use 601 wide parameters to fit camera ISP rgb->yuv parameters */
		csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2rgb601_wide;
		csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv601_wide;
		break;
	default:
		csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2rgb601_narrow;
		csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv601_narrow;
		break;
	}
}

static void csc_get_p3_coef(uint32_t csc_mode, struct dss_csc_info *csc_info, struct dss_csc_coe *csc_coe)
{
	if (!csc_info->need_yuv2p3) {
		csc_coe->csc_coe_yuv2rgb = g_csc_coe_srgb2p3;
	} else {
		switch (csc_mode) {
		case DSS_CSC_601_WIDE:
			csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2p3601_wide;
			csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv601_wide;
			break;
		case DSS_CSC_601_NARROW:
			csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2p3601_narrow;
			csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv601_narrow;
			break;
		case DSS_CSC_709_WIDE:
			csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2p3709_wide;
			csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv709_wide;
			break;
		case DSS_CSC_709_NARROW:
			csc_coe->csc_coe_yuv2rgb = g_csc_coe_yuv2p3709_narrow;
			csc_coe->csc_coe_rgb2yuv = g_csc_coe_rgb2yuv709_narrow;
			break;
		default:
			break;
		}
	}
}

static void hisi_dss_csc_get_coef(struct dpu_fb_data_type *dpufd, struct dss_csc_info *csc_info,
	dss_layer_t *layer, dss_wb_layer_t *wb_layer)
{
	dss_csc_t *csc = NULL;
	struct dss_csc_coe csc_coe;

	if (!csc_need_p3_process(csc_info->csc_mode)) {
		/* no need to process srgb->p3 */
		csc_get_regular_coef(csc_info->csc_mode, &csc_coe);
	} else {
		csc_info->csc_mode = csc_info->csc_mode & ~P3_PROCESS_NEEDED_LAYER;
		csc_get_p3_coef(csc_info->csc_mode, csc_info, &csc_coe);
	}

	/* config rch csc and pcsc */
	if (layer) {
		if (dpufd->dss_module.csc_used[csc_info->chn_idx]) {
			csc = &(dpufd->dss_module.csc[csc_info->chn_idx]);
			hisi_dss_csc_coef_array_config(csc, (int **)(csc_coe.csc_coe_yuv2rgb), CSC_COL);
		}
		if (dpufd->dss_module.pcsc_used[csc_info->chn_idx]) {
			csc = &(dpufd->dss_module.pcsc[csc_info->chn_idx]);
			hisi_dss_csc_coef_array_config(csc, (int **)(csc_coe.csc_coe_rgb2yuv), CSC_COL);
		}
	}

	/* config wch csc */
	if (wb_layer && dpufd->dss_module.csc_used[csc_info->chn_idx]) {
		csc = &(dpufd->dss_module.csc[csc_info->chn_idx]);
		hisi_dss_csc_coef_array_config(csc, (int **)(csc_coe.csc_coe_rgb2yuv), CSC_COL);
	}
}

int hisi_dss_csc_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer, dss_wb_layer_t *wb_layer)
{
	struct dss_csc_info csc_info;

	csc_info.chn_idx = 0;
	csc_info.format = 0;
	csc_info.csc_mode = 0;
	csc_info.need_yuv2p3 = false;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	if (wb_layer) {
		csc_info.chn_idx = wb_layer->chn_idx;
		csc_info.format = wb_layer->dst.format;
		csc_info.csc_mode = wb_layer->dst.csc_mode;
	} else if (layer) {
		csc_info.chn_idx = layer->chn_idx;
		csc_info.format = layer->img.format;
		csc_info.csc_mode = layer->img.csc_mode;
	}

	if (csc_info.chn_idx != DSS_RCHN_V0) {
		if (!is_yuv(csc_info.format)) {
			if (!csc_need_p3_process(csc_info.csc_mode))
				return 0;
			dpufd->dss_module.csc_used[csc_info.chn_idx] = 1;
		} else {
			csc_info.need_yuv2p3 = true;
			dpufd->dss_module.csc_used[csc_info.chn_idx] = 1;
		}
	} else if ((csc_info.chn_idx == DSS_RCHN_V0) && !(is_yuv(csc_info.format))) {
		if (layer) {
			if (!is_pcsc_needed(layer)) {
				if (!csc_need_p3_process(csc_info.csc_mode))
					return 0;
				dpufd->dss_module.csc_used[csc_info.chn_idx] = 1;
			} else {
				csc_info.need_yuv2p3 = true;
				dpufd->dss_module.csc_used[DSS_RCHN_V0] = 1;
				dpufd->dss_module.pcsc_used[DSS_RCHN_V0] = 1;
			}
		}
	} else {
		/* v0, yuv format */
		csc_info.need_yuv2p3 = true;
		dpufd->dss_module.csc_used[csc_info.chn_idx] = 1;
	}

	hisi_dss_csc_get_coef(dpufd, &csc_info, layer, wb_layer);

	return 0;
}
