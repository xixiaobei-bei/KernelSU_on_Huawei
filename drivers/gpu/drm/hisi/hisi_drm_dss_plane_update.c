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
/*lint -e502 -e530 -e574 -e732*/
#include <securec.h>

#include "hisi_drm_drv.h"
#include "hisi_drm_dss_rdma_configure.h"
#include "hisi_drm_dss_set_regs.h"
#include "hisi_drm_dss_global.h"
#ifdef CONFIG_HISI_DSS_V600
#include "hisi_drm_smmuv3.h"
#endif

struct dss_arsr_config {
	u32 size_hrz;
	u32 dst_size_hrz;
	u32 size_vrt;
	u32 dst_size_vrt;

	bool en_hscl;
	bool en_vscl;

	/* arsr mode */
	bool nointplen;   // bit8
	bool prescaleren; // bit7
	bool nearest_en;  // bit6
	bool diintpl_en;  // bit5
	bool textureanalyhsisen_en; // bit4
	bool arsr2p_bypass; // bit0
	bool hscldown_flag;

	int ih_inc;
	int iv_inc;
	int ih_left;   // input left acc
	int ih_right;  // input end position
	int iv_top;    // input top position
	int iv_bottom; // input bottom position
	int uv_offset;
	int src_width;

	int outph_left;   // output left acc
	int outph_right;  // output end position
	int outpv_bottom; // output bottom position
};

struct dss_scl_config {
	bool en_hscl;
	bool en_vscl;
	uint32_t h_ratio;
	uint32_t v_ratio;
	uint32_t scf_en_vscl;
	uint32_t size_hrz;
	uint32_t size_vrt;
	uint32_t dst_size_hrz;
	uint32_t dst_size_vrt;
	uint32_t h_v_order;
};

static dss_ovl_alpha_t g_ovl_alpha[DSS_BLEND_MAX] = {
	{ 0, 0, 0, 0, 0,  0,  0, 0, 0, 0,  0 },	// DSS_BLEND_CLEAR
	{ 0, 0, 0, 0, 0,  0,  1, 0, 0, 0,  0 },	// DSS_BLEND_SRC
	{ 1, 0, 0, 0, 0,  0,  0, 0, 0, 0,  0 },	// DSS_BLEND_DST
	{ 3, 0, 0, 0, 1,  0,  1, 0, 0, 0,  0 },	// DSS_BLEND_SRC_OVER_DST
	{ 1, 0, 0, 0, 0,  0,  3, 0, 0, 1,  0 },	// DSS_BLEND_DST_OVER_SRC
	{ 0, 0, 0, 0, 0,  0,  3, 0, 0, 0,  0 },	// DSS_BLEND_SRC_IN_DST
	{ 3, 0, 0, 0, 0,  0,  0, 0, 0, 0,  0 },	// DSS_BLEND_DST_IN_SRC
	{ 0, 0, 0, 0, 0,  0,  3, 0, 0, 1,  0 },	// DSS_BLEND_SRC_OUT_DST
	{ 3, 0, 0, 0, 1,  0,  0, 0, 0, 0,  0 },	// DSS_BLEND_DST_OUT_SRC
	{ 3, 0, 0, 0, 1,  0,  3, 0, 0, 0,  0 },	// DSS_BLEND_SRC_ATOP_DST
	{ 3, 0, 0, 0, 0,  0,  3, 0, 0, 1,  0 },	// DSS_BLEND_DST_ATOP_SRC
	{ 3, 0, 0, 0, 1,  0,  3, 0, 0, 1,  0 },	// DSS_BLEND_SRC_XOR_DST
	{ 1, 0, 0, 0, 0,  0,  1, 0, 0, 0,  0 },	// DSS_BLEND_SRC_ADD_DST
	{ 3, 0, 0, 0, 1,  0,  3, 0, 0, 0,  1 },	// DSS_BLEND_FIX_OVER
	{ 3, 0, 0, 0, 1,  0,  3, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER0
	{ 3, 0, 0, 0, 1,  1,  3, 1, 0, 0,  1 },	// DSS_BLEND_FIX_PER1
	{ 2, 2, 0, 0, 0,  0,  1, 0, 0, 0,  0 },	// DSS_BLEND_FIX_PER2
	{ 1, 0, 0, 0, 0,  0,  2, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER3
	{ 0, 0, 0, 0, 0,  0,  3, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER4
	{ 3, 2, 0, 0, 0,  0,  0, 0, 0, 0,  0 },	// DSS_BLEND_FIX_PER5
	{ 0, 0, 0, 0, 0,  0,  2, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER6
	{ 2, 2, 0, 0, 0,  0,  0, 0, 0, 0,  0 },	// DSS_BLEND_FIX_PER7
	{ 2, 2, 0, 0, 0,  0,  3, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER8
	{ 3, 2, 0, 0, 0,  0,  2, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER9
	{ 2, 2, 0, 0, 0,  0,  2, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER10
	{ 3, 2, 0, 0, 0,  0,  3, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER11
	{ 2, 1, 0, 0, 0,  0,  3, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER12 DSS_BLEND_SRC_OVER_DST
	{ 2, 1, 0, 0, 0,  0,  3, 1, 0, 0,  1 },	// DSS_BLEND_FIX_PER13 DSS_BLEND_FIX_OVER
	{ 0, 0, 0, 0, 0,  1,  3, 0, 1, 0,  0 },	// DSS_BLEND_FIX_PER14 BASE_FF
	{ 2, 1, 0, 0, 0,  1,  3, 2, 0, 0,  0 },	// DSS_BLEND_FIX_PER15 DSS_BLEND_SRC_OVER_DST
	{ 2, 1, 0, 0, 0,  1,  3, 1, 0, 0,  1 },	// DSS_BLEND_FIX_PER16 DSS_BLEND_FIX_OVER
	{ 0, 0, 0, 0, 0,  1,  1, 0, 0, 0,  0 },	// DSS_BLEND_FIX_PER17 DSS_BLEND_SRC
};

static bool hal_format_has_alpha(uint32_t format)
{
	switch (format) {
	case HISI_DSS_FORMAT_RGBA4444:
	case HISI_DSS_FORMAT_RGBA5551:
	case HISI_DSS_FORMAT_RGBA8888:

	case HISI_DSS_FORMAT_BGRA4444:
	case HISI_DSS_FORMAT_BGRA5551:
	case HISI_DSS_FORMAT_BGRA8888:

	case HISI_DSS_FORMAT_RGBA1010102:
	case HISI_DSS_FORMAT_BGRA1010102:
		return true;

	default:
		return false;
	}
}

static bool is_pixel_10bit_2_dfc(int format)
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

static bool is_need_rect_clip(struct dss_rect_ltrb clip_rect)
{
	return ((clip_rect.left > 0) || (clip_rect.top > 0) ||
		(clip_rect.right > 0) || (clip_rect.bottom > 0));
}

static int hisi_pixel_format_hal2dfc(int format)
{
	static const struct dss_format dfc_formats_array[] = {
		{ DFC_PIXEL_FORMAT_RGB_565, HISI_DSS_FORMAT_RGB565 },
		{ DFC_PIXEL_FORMAT_XBGR_4444, HISI_DSS_FORMAT_RGBX4444 },
		{ DFC_PIXEL_FORMAT_ABGR_4444, HISI_DSS_FORMAT_RGBA4444 },
		{ DFC_PIXEL_FORMAT_XBGR_5551, HISI_DSS_FORMAT_RGBX5551 },
		{ DFC_PIXEL_FORMAT_ABGR_5551, HISI_DSS_FORMAT_RGBA5551 },
		{ DFC_PIXEL_FORMAT_XBGR_8888, HISI_DSS_FORMAT_RGBX8888 },
		{ DFC_PIXEL_FORMAT_ABGR_8888, HISI_DSS_FORMAT_RGBA8888 },
		{ DFC_PIXEL_FORMAT_ARGB_8888, HISI_DSS_FORMAT_BGRA8888 },
		{ DFC_PIXEL_FORMAT_BGR_565, HISI_DSS_FORMAT_BGR565 },
		{ DFC_PIXEL_FORMAT_XRGB_4444, HISI_DSS_FORMAT_BGRX4444 },
		{ DFC_PIXEL_FORMAT_ARGB_4444, HISI_DSS_FORMAT_BGRA4444 },
		{ DFC_PIXEL_FORMAT_XRGB_5551, HISI_DSS_FORMAT_BGRX5551 },
		{ DFC_PIXEL_FORMAT_ARGB_5551, HISI_DSS_FORMAT_BGRA5551 },
		{ DFC_PIXEL_FORMAT_XRGB_8888, HISI_DSS_FORMAT_BGRX8888 },
		{ DFC_PIXEL_FORMAT_YUYV422, HISI_DSS_FORMAT_YUV422_I },
		{ DFC_PIXEL_FORMAT_YUYV422, HISI_DSS_FORMAT_YUYV422_PKG },
		{ DFC_PIXEL_FORMAT_YVYU422, HISI_DSS_FORMAT_YVYU422_PKG },
		{ DFC_PIXEL_FORMAT_UYVY422, HISI_DSS_FORMAT_UYVY422_PKG },
		{ DFC_PIXEL_FORMAT_VYUY422, HISI_DSS_FORMAT_VYUY422_PKG },
		{ DFC_PIXEL_FORMAT_YUYV422, HISI_DSS_FORMAT_YCBCR422_SP },
		{ DFC_PIXEL_FORMAT_YVYU422, HISI_DSS_FORMAT_YCRCB422_SP },
		{ DFC_PIXEL_FORMAT_YUYV422, HISI_DSS_FORMAT_YCBCR420_SP },
		{ DFC_PIXEL_FORMAT_YVYU422, HISI_DSS_FORMAT_YCRCB420_SP },
		{ DFC_PIXEL_FORMAT_YUYV422, HISI_DSS_FORMAT_YCBCR422_P },
		{ DFC_PIXEL_FORMAT_YUYV422, HISI_DSS_FORMAT_YCBCR420_P },
		{ DFC_PIXEL_FORMAT_YVYU422, HISI_DSS_FORMAT_YCRCB422_P },
		{ DFC_PIXEL_FORMAT_YVYU422, HISI_DSS_FORMAT_YCRCB420_P }
	};

	int i;

	for (i = 0; i < ARRAY_SIZE(dfc_formats_array); i++) {
		if (dfc_formats_array[i].dss_format == format)
			return dfc_formats_array[i].pixel_format;
	}

	return -1;
}

#ifdef CONFIG_HISI_DSS_V510
static int hisi_dss_smmu_ch_config(struct dss_module_reg *dss_module, int chn_idx, bool mmu_enable)
{
	struct dss_smmu *smmu = NULL;
	uint32_t idx = 0;
	uint32_t i = 0;

	smmu = &dss_module->smmu;
	dss_module->smmu_used = 1;

	smmu->smmu_smrx_ns_used[chn_idx] = 1;

	for (i = 0; i < g_dss_chn_sid_num[chn_idx]; i++) {
		idx = g_dss_smmu_smrx_idx[chn_idx] + i;
		if (idx >= SMMU_SID_NUM) {
			HISI_DRM_ERR("idx is invalid");
			return -EINVAL;
		}

		if (mmu_enable == 0) {
			smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x1, 1, 0);
		} else {
			/* stream config */
			smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x0, 1, 0);  // smr_bypass
			smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x3, 2, 2);  // smr_ptw_qos
			smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x1, 1, 4);  // smr_invld_en
		}
	}
	return 0;
}
#endif

static void hisi_dss_set_mif_ctrl(uint32_t invalid_sel, struct drm_framebuffer *fb,
	uint32_t format, dss_mif_t *mif)
{
	uint32_t stride = fb->pitches[0];
	uint32_t stride1 = fb->pitches[1];
	uint32_t stride2 = fb->pitches[2];

	mif->mif_ctrl1 = set_bits32(mif->mif_ctrl1, 0x0, 1, 5);
	mif->mif_ctrl1 = set_bits32(mif->mif_ctrl1, invalid_sel, 2, 10);
	mif->mif_ctrl1 = set_bits32(mif->mif_ctrl1, ((invalid_sel == 0) ? 0x1 : 0x0), 1, 19);

	if (invalid_sel == 0) {
		mif->mif_ctrl2 = set_bits32(mif->mif_ctrl2, 0x0, 20, 0);
		mif->mif_ctrl3 = set_bits32(mif->mif_ctrl3, 0x0, 20, 0);
		mif->mif_ctrl4 = set_bits32(mif->mif_ctrl4, 0x0, 20, 0);
		mif->mif_ctrl5 = set_bits32(mif->mif_ctrl5, 0x0, 20, 0);
	} else if ((invalid_sel == 1) || (invalid_sel == 2)) {
		if (stride > 0)
			mif->mif_ctrl5 = set_bits32(mif->mif_ctrl5, ((stride / MIF_STRIDE_UNIT) +
				(((stride % MIF_STRIDE_UNIT) > 0) ? 1 : 0)), 20, 0);

		if (check_yuv_semi_planar(format)) {
			if (stride1 > 0)
				mif->mif_ctrl4 = set_bits32(mif->mif_ctrl4, ((stride1 / MIF_STRIDE_UNIT)
					+ (((stride1 % MIF_STRIDE_UNIT) > 0) ? 1 : 0)), 20, 0);
		} else if (check_yuv_planar(format)) {
			if (stride1 > 0)
				mif->mif_ctrl4 = set_bits32(mif->mif_ctrl4,
					((stride1 / MIF_STRIDE_UNIT) +
					(((stride1 % MIF_STRIDE_UNIT) > 0) ? 1 : 0)), 20, 0);

			if (stride2 > 0)
				mif->mif_ctrl3 = set_bits32(mif->mif_ctrl3,
					((stride2 / MIF_STRIDE_UNIT) +
					(((stride2 % MIF_STRIDE_UNIT) > 0) ? 1 : 0)), 20, 0);
		}
	} else if (invalid_sel == 3) {
		if (stride > 0)
			mif->mif_ctrl5 = set_bits32(mif->mif_ctrl5,
				DSS_MIF_CTRL2_INVAL_SEL3_STRIDE_MASK, 4, 16);
		if (check_yuv_semi_planar(format)) {
			if (stride1 > 0)
				mif->mif_ctrl4 = set_bits32(mif->mif_ctrl4, 0xE, 4, 16);
		} else if (check_yuv_planar(format)) {
			if (stride1 > 0)
				mif->mif_ctrl3 = set_bits32(mif->mif_ctrl3, 0xE, 4, 16);
			if (stride2 > 0)
				mif->mif_ctrl4 = set_bits32(mif->mif_ctrl4, 0xE, 4, 16);
		}
	}

}

static uint32_t hisi_dss_mif_get_invalid_sel(uint32_t transform, int v_scaling_factor,
	uint8_t is_tile, bool rdma_stretch_enable)
{
	uint32_t invalid_sel_val = 0;
	uint32_t tlb_tag_org = 0;

	if ((transform == (DRM_MODE_ROTATE_90 | DRM_MODE_REFLECT_X))
		|| (transform == (DRM_MODE_ROTATE_90 | DRM_MODE_REFLECT_X)))
		transform = DRM_MODE_ROTATE_90;

	HISI_DRM_DEBUG("transform=%d\n", transform);

	tlb_tag_org =  (transform & 0x7) |
		((is_tile ? 1 : 0) << 3) | ((rdma_stretch_enable ? 1 : 0) << 4);

	switch (tlb_tag_org) {
	case MMU_TLB_TAG_ORG_0x0:
	case MMU_TLB_TAG_ORG_0x1:
	case MMU_TLB_TAG_ORG_0x10:
	case MMU_TLB_TAG_ORG_0x11:
		invalid_sel_val = 1;
		break;
	case MMU_TLB_TAG_ORG_0x2:
	case MMU_TLB_TAG_ORG_0x3:
	case MMU_TLB_TAG_ORG_0x12:
	case MMU_TLB_TAG_ORG_0x13:
		invalid_sel_val = 2;
		break;
	case MMU_TLB_TAG_ORG_0x8:
	case MMU_TLB_TAG_ORG_0x9:
	case MMU_TLB_TAG_ORG_0xA:
	case MMU_TLB_TAG_ORG_0xB:
	case MMU_TLB_TAG_ORG_0x18:
	case MMU_TLB_TAG_ORG_0x19:
	case MMU_TLB_TAG_ORG_0x1A:
	case MMU_TLB_TAG_ORG_0x1B:
		invalid_sel_val = 3;
		break;
	default:
		invalid_sel_val = 0;
		HISI_DRM_INFO("tlb_tag_org=(0x%x)!\n", tlb_tag_org);
		break;
	}

	return invalid_sel_val;
}

static int hisi_dss_mif_config(struct dss_module_reg *dss_module,
	struct drm_plane_state *state, int chn_idx, bool mmu_enable,
	bool rdma_stretch_enable)
{
	dss_mif_t *mif = NULL;
	uint32_t transform = 0;
	uint32_t invalid_sel = 0;
	uint32_t need_cap = CAP_BASE;
	int v_scaling_factor;
	uint32_t format;
	struct hisi_drm_plane_state *hisi_state = NULL;

	drm_check_and_return(state->fb == NULL, -EINVAL, ERR, "fb is NULL");
	drm_check_and_return(state->fb->format == NULL, -EINVAL, ERR, "format is NULL");

	hisi_state = to_hisi_plane_state(state);
	need_cap = hisi_state->need_cap;

	if (state->fb->modifier == AFBC_FORMAT_MODIFIER)
		need_cap |= CAP_AFBCD;

	transform = state->rotation;
	format = dss_get_format(state->fb->format->format, state->fb->modifier);
	drm_check_and_return(format == HISI_DSS_FORMAT_UNSUPPORT,
		-EINVAL, ERR, "format not support");

	v_scaling_factor = (state->src_h >> 16) / state->crtc_h +
		(((state->src_h >> 16) % state->crtc_h) > 0 ? 1 : 0);

	mif = &(dss_module->mif[chn_idx]);
	dss_module->mif_used[chn_idx] = 1;

	if (!mmu_enable) {
		mif->mif_ctrl1 = set_bits32(mif->mif_ctrl1, 0x1, 1, 5);
		return 0;
	}

	if (need_cap & CAP_AFBCD) {
		invalid_sel = 0;
	} else {
		invalid_sel = hisi_dss_mif_get_invalid_sel(transform,
			v_scaling_factor, ((need_cap & CAP_TILE) ? 1 : 0),
			rdma_stretch_enable);
	}

	HISI_DRM_DEBUG("invalid_sel=%d\n", invalid_sel);

	hisi_dss_set_mif_ctrl(invalid_sel, state->fb, format, mif);

	return 0;
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
		ret = 2;
		break;

	case DFC_PIXEL_FORMAT_XRGB_8888:
	case DFC_PIXEL_FORMAT_ARGB_8888:
	case DFC_PIXEL_FORMAT_XBGR_8888:
	case DFC_PIXEL_FORMAT_ABGR_8888:
	case DFC_PIXEL_FORMAT_BGRA_1010102:
		ret = 4;
		break;

	case DFC_PIXEL_FORMAT_YUV444:
	case DFC_PIXEL_FORMAT_YVU444:
		ret = 3;
		break;

	case DFC_PIXEL_FORMAT_YUYV422:
	case DFC_PIXEL_FORMAT_YVYU422:
	case DFC_PIXEL_FORMAT_VYUY422:
	case DFC_PIXEL_FORMAT_UYVY422:
	case DFC_PIXEL_FORMAT_YUYV_10:
		ret = 2;
		break;
	case DFC_PIXEL_FORMAT_UYVA_1010102:
		ret = 4;
		break;
	default:
		HISI_DRM_ERR("not support format %d !\n", dfc_format);
		ret = -1;
		break;
	}

	return ret;
}

static void hisi_dfc_set_clip_ctl(dss_dfc_t *dfc, int dfc_bpp, struct dss_rect_ltrb clip_rect,
	struct dss_rect *aligned_rect, uint32_t format)
{
	u32 size_hrz = 0;
	u32 size_vrt = 0;
	int dfc_hrz_clip = 0;
	int dfc_aligned = 0;
	bool need_clip = false;

	dfc_aligned = (dfc_bpp <= 2) ? 4 : 2;
	need_clip = is_need_rect_clip(clip_rect);

	size_hrz = DSS_WIDTH(aligned_rect->w);
	size_vrt = DSS_HEIGHT(aligned_rect->h);

	if (((size_hrz + 1) % dfc_aligned) != 0) {
		size_hrz -= 1;
		HISI_DRM_ERR("format=%d, rdma_out_rect %d,%d,%d,%d\n", format,
			aligned_rect->x, aligned_rect->y, aligned_rect->w, aligned_rect->h);
	}

	dfc_hrz_clip = (size_hrz + 1) % dfc_aligned;
	if (dfc_hrz_clip) {
		clip_rect.right += dfc_hrz_clip;
		size_hrz += dfc_hrz_clip;
		need_clip = true;
	}

	dfc->disp_size = set_bits32(dfc->disp_size, (size_vrt | (size_hrz << 16)), 29, 0);

	if (need_clip) {
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

	if (need_clip) {
		aligned_rect->w -= (clip_rect.left + clip_rect.right);
		aligned_rect->h -= (clip_rect.top + clip_rect.bottom);
	}

	return;
}

static int hisi_dss_rdfc_config(struct dss_module_reg *dss_module, uint32_t format,
	int chn_idx, struct dss_rect *aligned_rect, struct dss_rect_ltrb clip_rect)
{
	int dfc_fmt = 0;
	int dfc_bpp = 0;
	u32 dfc_pix_in_num = 0;

	dss_dfc_t *dfc = &(dss_module->dfc[chn_idx]);

	dss_module->dfc_used[chn_idx] = 1;
	dfc_fmt = hisi_pixel_format_hal2dfc(format);
	if (dfc_fmt < 0) {
		HISI_DRM_ERR("layer format %d not support !\n", format);
		return -EINVAL;
	}

	dfc_bpp = hisi_dfc_get_bpp(dfc_fmt);
	dfc_pix_in_num = (dfc_bpp <= 2) ? 0x1 : 0x0;

	// need_clip
	hisi_dfc_set_clip_ctl(dfc, dfc_bpp, clip_rect, aligned_rect, format);

	dfc->pix_in_num = set_bits32(dfc->pix_in_num, dfc_pix_in_num, 1, 0);
	dfc->disp_fmt = set_bits32(dfc->disp_fmt, dfc_fmt << 1, 8, 0);
	dfc->icg_module = set_bits32(dfc->icg_module, 0x1, 1, 0);
	dfc->dither_enable = set_bits32(dfc->dither_enable, 0x0, 1, 0);
	dfc->padding_ctl = set_bits32(dfc->padding_ctl, 0x0, 17, 0);

	if (is_pixel_10bit_2_dfc(dfc_fmt) == false)
		dfc->bitext_ctl = set_bits32(dfc->bitext_ctl, 0x3, 32, 0);

	HISI_DRM_DEBUG("rdfc output, aligned_rect %d,%d,%d,%d",
		aligned_rect->x, aligned_rect->y, aligned_rect->w, aligned_rect->h);

	return 0;
}

static int hisi_dss_get_hrz_rate(struct dss_scl_config *config)
{
	uint32_t acc_hscl = 0;

	if (config->size_hrz != config->dst_size_hrz) {
		config->en_hscl = true;

		if ((config->size_hrz + 1 < SCF_MIN_INPUT) || (config->dst_size_hrz + 1 < SCF_MIN_OUTPUT)) {
			HISI_DRM_ERR("size_hrz %d small than 16, or dst_size_hrz %d small than 16",
				config->size_hrz + 1, config->dst_size_hrz + 1);
			return -EINVAL;
		}

		config->h_ratio = (config->size_hrz * SCF_INC_FACTOR + SCF_INC_FACTOR / 2 - acc_hscl) /
			config->dst_size_hrz;

		if (((config->dst_size_hrz + 1) > ((config->size_hrz + 1) * SCF_UPSCALE_MAX))
			|| ((config->size_hrz + 1) > ((config->dst_size_hrz + 1) * SCF_DOWNSCALE_MAX))) {
			HISI_DRM_ERR("dst_size_hrz = %d size_hrz=%d", config->dst_size_hrz, config->size_hrz);
			return -EINVAL;
		}
	}

	return 0;

}

static int hisi_dss_get_vrt_rate(struct dss_scl_config *config)
{
	uint32_t acc_vscl = 0;

	if (config->size_vrt != config->dst_size_vrt) {
		config->en_vscl = true;
		config->scf_en_vscl = 1;

		config->v_ratio = (config->size_vrt * SCF_INC_FACTOR + SCF_INC_FACTOR / 2 - acc_vscl) / config->dst_size_vrt;

		if (((config->dst_size_vrt + 1) > ((config->size_vrt + 1) * SCF_UPSCALE_MAX))
			|| ((config->size_vrt + 1) > ((config->dst_size_vrt + 1) * SCF_DOWNSCALE_MAX))) {
			HISI_DRM_ERR("dst_size_vrt = %d size_vrt=%d", config->dst_size_vrt, config->size_vrt);
			return -EINVAL;
		}
	}

	return 0;
}

static int hisi_dss_scl_prepare_config(const struct dss_rect_ltrb *src_rect, const struct dss_rect_ltrb *dst_rect,
	const int chn_idx, struct dss_scl_config *config)
{
	int ret = 0;
	config->size_hrz = src_rect->right - src_rect->left;
	config->size_vrt = src_rect->bottom - src_rect->top;
	config->dst_size_hrz = dst_rect->right - dst_rect->left;
	config->dst_size_vrt = dst_rect->bottom - dst_rect->top;

	if (chn_idx == DSS_RCHN_V0) {
		config->dst_size_vrt = config->size_vrt; // set dst height to src height, disable vertical scaling in v0scf
		config->dst_size_hrz = (config->size_hrz > config->dst_size_hrz ? config->dst_size_hrz : config->size_hrz);
	}

	ret = hisi_dss_get_hrz_rate(config);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_get_hrz_rate failed");

	ret = hisi_dss_get_vrt_rate(config);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_get_vrt_rate failed");

	/* scale down, do hscl first; scale up, do vscl first*/
	config->h_v_order = (config->size_hrz > config->dst_size_hrz) ? 0 : 1;

	if (config->size_vrt > config->dst_size_vrt)
		config->scf_en_vscl = 0x3;

	return 0;
}

static int hisi_dss_scl_config(struct dss_module_reg *dss_module,
	const struct dss_rect_ltrb *src_rect, const struct dss_rect_ltrb *dst_rect,
	const int chn_idx, int format)
{
	int ret = 0;
	dss_scl_t *scl = NULL;
	struct dss_scl_config config;

	ret = memset_s(&config, sizeof(config), 0x00, sizeof(config));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset failed");
	ret = hisi_dss_scl_prepare_config(src_rect, dst_rect, chn_idx, &config);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_scl_prepare_config failed");

	if (!config.en_hscl && !config.en_vscl)
		return 0;

	scl = &(dss_module->scl[chn_idx]);
	dss_module->scl_used[chn_idx] = 1;

	if (config.v_ratio >= 2 * SCF_INC_FACTOR) {
		if (hal_format_has_alpha(format))
			scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x3, 2, 0);
		else
			scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x1, 2, 0);
	} else {
		scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x0, 1, 0);
	}

	scl->en_hscl_str = set_bits32(scl->en_hscl_str, 0x0, 1, 0);
	scl->h_v_order = set_bits32(scl->h_v_order, config.h_v_order, 1, 0);
	scl->input_width_height = set_bits32(scl->input_width_height, config.size_vrt, 13, 0);
	scl->input_width_height = set_bits32(scl->input_width_height, config.size_hrz, 13, 16);
	scl->output_width_height = set_bits32(scl->output_width_height, config.dst_size_vrt, 13, 0);
	scl->output_width_height = set_bits32(scl->output_width_height, config.dst_size_hrz, 13, 16);
	scl->en_hscl = set_bits32(scl->en_hscl, (config.en_hscl ? 0x1 : 0x0), 1, 0);
	scl->en_vscl = set_bits32(scl->en_vscl, config.scf_en_vscl, 2, 0);
	scl->acc_hscl = set_bits32(scl->acc_hscl, 0x0, 31, 0);
	scl->inc_hscl = set_bits32(scl->inc_hscl, config.h_ratio, 24, 0);
	scl->inc_vscl = set_bits32(scl->inc_vscl, config.v_ratio, 24, 0);
	scl->en_mmp = set_bits32(scl->en_mmp, 0x1, 1, 0);
	scl->fmt = format;

	return 0;
}

static void hisi_dss_prepare_arsr_config(struct dss_arsr_config *arsr_config)
{
	int extraw = 0;

	/* horizental scaler compute */
	if (arsr_config->size_hrz > arsr_config->dst_size_hrz) {
		arsr_config->src_width = arsr_config->dst_size_hrz;
		arsr_config->hscldown_flag = true;
	} else {
		arsr_config->src_width = arsr_config->size_hrz;
	}

	if (arsr_config->src_width != arsr_config->dst_size_hrz)
		arsr_config->en_hscl = true;

	arsr_config->ih_inc = ((arsr_config->src_width + 1) * ARSR2P_INC_FACTOR) / (arsr_config->dst_size_hrz + 1);

	extraw = (8 * ARSR2P_INC_FACTOR) / arsr_config->ih_inc;
	extraw = (extraw % 2) ? (extraw + 1) : (extraw);
	arsr_config->ih_left = arsr_config->outph_left - extraw * arsr_config->ih_inc;
	if (arsr_config->ih_left < 0)
		arsr_config->ih_left = 0;

	arsr_config->outph_right = arsr_config->dst_size_hrz * arsr_config->ih_inc;

	extraw = (2 * ARSR2P_INC_FACTOR) / arsr_config->ih_inc;
	extraw = (extraw % 2) ? (extraw + 1) : (extraw);
	arsr_config->ih_right = arsr_config->outph_right + extraw * arsr_config->ih_inc;

	extraw = arsr_config->dst_size_hrz * arsr_config->ih_inc;

	if (arsr_config->ih_right > extraw)
		arsr_config->ih_right = extraw;

	if (arsr_config->size_vrt != arsr_config->dst_size_vrt)
		arsr_config->en_vscl = true;

	if (arsr_config->size_vrt > arsr_config->dst_size_vrt) {
		arsr_config->iv_inc = (arsr_config->size_vrt * ARSR2P_INC_FACTOR +
			ARSR2P_INC_FACTOR / 2 - arsr_config->iv_top) / arsr_config->dst_size_vrt;
	} else {
		arsr_config->iv_inc = (arsr_config->size_vrt * ARSR2P_INC_FACTOR +
			ARSR2P_INC_FACTOR - arsr_config->iv_top) / arsr_config->dst_size_vrt;
	}
	arsr_config->iv_bottom = arsr_config->dst_size_vrt * arsr_config->iv_inc + arsr_config->iv_top;
	arsr_config->outpv_bottom = arsr_config->iv_bottom;

	do {
		if (arsr_config->hscldown_flag) {
			arsr_config->prescaleren = true;
			break;
		}

		if (!arsr_config->en_hscl && (arsr_config->iv_inc >= 2 * ARSR2P_INC_FACTOR)) {
			arsr_config->nearest_en = true;
			break;
		}

		if ((!arsr_config->en_hscl) && (!arsr_config->en_vscl))
			break;

		arsr_config->diintpl_en = true;
		arsr_config->textureanalyhsisen_en = true;
	} while (0);

}

static int hisi_dss_arsr_pre_config(struct dss_module_reg *dss_module,
	const struct dss_rect_ltrb *rect, const struct dss_rect_ltrb *dst_rect, int chn_idx)
{
	struct dss_arsr2p *arsr_pre = NULL;
	struct dss_arsr_config arsr_config;
	int ret;

	ret = memset_s(&arsr_config, sizeof(arsr_config), 0x00, sizeof(arsr_config));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset for arsr_config failed!");

	if (chn_idx != DSS_RCHN_V0) {
		HISI_DRM_DEBUG("ARSR PRE PASSS, chn_idx = %d", chn_idx);
		return 0;
	}

	arsr_config.size_hrz = rect->right - rect->left;
	arsr_config.size_vrt = rect->bottom - rect->top;
	arsr_config.dst_size_hrz = dst_rect->right - dst_rect->left;
	arsr_config.dst_size_vrt = dst_rect->bottom - dst_rect->top;
	hisi_dss_prepare_arsr_config(&arsr_config);

	HISI_DRM_DEBUG("ARSR PRE src: %d,%d,%d,%d--%d,%d, dst: %d,%d,%d,%d--%d,%d," \
		"ih_left=%d, ih_right=%d, iv_top=%d, iv_bottom=%d, ih_inc=%d, iv_inc=%d, uv_offset=%d," \
		"arsr2p_bypass=%d", rect->left, rect->right, rect->top, rect->bottom, arsr_config.size_hrz,
		arsr_config.size_vrt, dst_rect->left, dst_rect->right, dst_rect->top, dst_rect->bottom,
		arsr_config.dst_size_hrz, arsr_config.dst_size_vrt, arsr_config.ih_left, arsr_config.ih_right,
		arsr_config.iv_top, arsr_config.iv_bottom, arsr_config.ih_inc, arsr_config.iv_inc, arsr_config.uv_offset,
		arsr_config.arsr2p_bypass);

	arsr_pre = &(dss_module->arsr2p[chn_idx]);

	dss_module->arsr2p_used[chn_idx] = 1;

	dss_module->arsr2p_effect_used[chn_idx] = 0;

	arsr_pre->arsr_input_width_height = set_bits32(arsr_pre->arsr_input_width_height, arsr_config.size_vrt, 13, 0);
	arsr_pre->arsr_input_width_height = set_bits32(arsr_pre->arsr_input_width_height, arsr_config.src_width, 13, 16);
	arsr_pre->arsr_output_width_height = set_bits32(arsr_pre->arsr_output_width_height, arsr_config.dst_size_vrt, 13, 0);
	arsr_pre->arsr_output_width_height = set_bits32(arsr_pre->arsr_output_width_height, arsr_config.dst_size_hrz, 13, 16);
	arsr_pre->ihleft = set_bits32(arsr_pre->ihleft, arsr_config.ih_left, 29, 0);
	arsr_pre->ihright = set_bits32(arsr_pre->ihright, arsr_config.ih_right, 29, 0);
	arsr_pre->ivtop = set_bits32(arsr_pre->ivtop, arsr_config.iv_top, 29, 0);
	arsr_pre->ivbottom = set_bits32(arsr_pre->ivbottom, arsr_config.iv_bottom, 29, 0);
	arsr_pre->ihinc = set_bits32(arsr_pre->ihinc, arsr_config.ih_inc, 22, 0);
	arsr_pre->ivinc = set_bits32(arsr_pre->ivinc, arsr_config.iv_inc, 22, 0);
	arsr_pre->offset = set_bits32(arsr_pre->offset, arsr_config.uv_offset, 22, 0);
	arsr_pre->mode = set_bits32(arsr_pre->mode, arsr_config.arsr2p_bypass, 1, 0);
	arsr_pre->mode = set_bits32(arsr_pre->mode, arsr_pre->arsr2p_effect.sharp_enable, 1, 1);
	arsr_pre->mode = set_bits32(arsr_pre->mode, arsr_pre->arsr2p_effect.shoot_enable, 1, 2);
	arsr_pre->mode = set_bits32(arsr_pre->mode, arsr_pre->arsr2p_effect.skin_enable, 1, 3);
	arsr_pre->mode = set_bits32(arsr_pre->mode, arsr_config.textureanalyhsisen_en, 1, 4);
	arsr_pre->mode = set_bits32(arsr_pre->mode, arsr_config.diintpl_en, 1, 5);
	arsr_pre->mode = set_bits32(arsr_pre->mode, arsr_config.nearest_en, 1, 6);
	arsr_pre->mode = set_bits32(arsr_pre->mode, arsr_config.prescaleren, 1, 7);
	arsr_pre->mode = set_bits32(arsr_pre->mode, arsr_config.nointplen, 1, 8);

	arsr_pre->ihleft1 = set_bits32(arsr_pre->ihleft1, arsr_config.outph_left, 29, 0);
	arsr_pre->ihright1 = set_bits32(arsr_pre->ihright1, arsr_config.outph_right, 29, 0);
	arsr_pre->ivbottom1 = set_bits32(arsr_pre->ivbottom1, arsr_config.outpv_bottom, 29, 0);

	return 0;
}

static uint32_t get_ovl_blending_mode(
	struct hisi_drm_plane_state *hisi_plane_state, uint32_t format)
{
	uint32_t blend_mode = 0;
	bool has_per_pixel_alpha = false;

	has_per_pixel_alpha = hal_format_has_alpha(format);
	blend_mode = hisi_plane_state->blending;

	if (hisi_plane_state->blending == HISI_BLENDING_PREMULT) {
		if (has_per_pixel_alpha) {
			blend_mode = (hisi_plane_state->alpha < 0xFF) ?
				DSS_BLEND_FIX_PER12 : DSS_BLEND_SRC_OVER_DST;
		} else {
			blend_mode = (hisi_plane_state->alpha < 0xFF) ?
				DSS_BLEND_FIX_PER8 : DSS_BLEND_SRC;
		}
	} else if (hisi_plane_state->blending == HISI_BLENDING_COVERAGE) {
		if (has_per_pixel_alpha) {
			blend_mode = (hisi_plane_state->alpha < 0xFF) ?
				DSS_BLEND_FIX_PER13 : DSS_BLEND_FIX_OVER;
		} else {
			blend_mode = (hisi_plane_state->alpha < 0xFF) ?
				DSS_BLEND_FIX_PER8 : DSS_BLEND_SRC;
		}
	} else {
		if (has_per_pixel_alpha)
			blend_mode = DSS_BLEND_SRC;
		else
			blend_mode = DSS_BLEND_FIX_PER17;
	}

	return blend_mode;
}

static void hisi_dss_set_layer_alpha_config(dss_ovl_layer_t *layer, int blend_mode, uint32_t glb_alpha)
{
	layer->layer_alpha = set_bits32(layer->layer_alpha,
		((g_ovl_alpha[blend_mode].fix_mode << 8) |
		(g_ovl_alpha[blend_mode].dst_pmode << 9) |
		(g_ovl_alpha[blend_mode].alpha_offdst << 10) |
		(g_ovl_alpha[blend_mode].dst_gmode << 12) |
		(g_ovl_alpha[blend_mode].dst_amode << 14) |
		(g_ovl_alpha[blend_mode].alpha_smode << 24) |
		(g_ovl_alpha[blend_mode].src_pmode << 25) |
		(g_ovl_alpha[blend_mode].src_lmode << 26) |
		(g_ovl_alpha[blend_mode].alpha_offdst << 27) |
		(g_ovl_alpha[blend_mode].src_gmode << 28) |
		(g_ovl_alpha[blend_mode].src_amode << 30)), 32, 0);

	layer->layer_alpha_a = set_bits32(layer->layer_alpha_a, glb_alpha | (glb_alpha << 16), 32, 0);
}

static void hisi_dss_set_layer_pos_config(dss_ovl_layer_t *ovl_layer,
	dss_ovl_layer_pos_t *ovl_layer_pos, const struct dss_rect_ltrb *dst_rect)
{
	ovl_layer->layer_pos = set_bits32(ovl_layer->layer_pos, dst_rect->left, 15, 0);
	ovl_layer->layer_pos = set_bits32(ovl_layer->layer_pos, dst_rect->top, 15, 16);
	ovl_layer->layer_size = set_bits32(ovl_layer->layer_size, dst_rect->right, 15, 0);
	ovl_layer->layer_size = set_bits32(ovl_layer->layer_size, dst_rect->bottom, 15, 16);

	ovl_layer_pos->layer_pspos = set_bits32(ovl_layer_pos->layer_pspos, dst_rect->left, 15, 0);
	ovl_layer_pos->layer_pspos = set_bits32(ovl_layer_pos->layer_pspos, dst_rect->top, 15, 16);
	ovl_layer_pos->layer_pepos = set_bits32(ovl_layer_pos->layer_pepos, dst_rect->right, 15, 0);
	ovl_layer_pos->layer_pepos = set_bits32(ovl_layer_pos->layer_pepos, dst_rect->bottom, 15, 16);
}

static int hisi_dss_ovl_layer_config(struct dss_module_reg *dss_module, int ovl_idx,
	const struct dss_rect_ltrb *dst_rect, int layer_idx, int format,
	struct hisi_drm_plane_state *hisi_plane_state)
{
	dss_ovl_t *ovl = NULL;

	uint32_t color = hisi_plane_state->color;
	uint32_t glb_alpha = hisi_plane_state->alpha;
	uint32_t color_rgb;
	uint32_t color_alpha;
	int blend_mode;

	ovl = &(dss_module->ov[ovl_idx]);
	dss_module->ov_used[ovl_idx] = 1;

	if (hisi_plane_state->need_cap & CAP_BASE) {
		color_rgb = hisi_plane_state->color;
		color_alpha = (hisi_plane_state->color >> 24) * OVL_PATTERN_RATIO;
		color_rgb = (((hisi_plane_state->color >> 16) & 0xff) << 20) |
					(((hisi_plane_state->color >> 8) & 0xff) << 10) |
					(hisi_plane_state->color & 0xff);
		ovl->ovl_bg_color = set_bits32(ovl->ovl_bg_color, color_rgb, 32, 0);
		ovl->ovl_bg_color_alpha = set_bits32(ovl->ovl_bg_color_alpha, color_alpha, 32, 0);
		ovl->ovl_gcfg = set_bits32(ovl->ovl_gcfg, 0x1, 1, 16);
		return 0;
	}

	if ((glb_alpha) > 0xFF) {
		HISI_DRM_ERR("layer's glb_alpha(0x%x) is out of range!", glb_alpha);
		glb_alpha = 0xFF;
	}

	glb_alpha = glb_alpha * OVL_PATTERN_RATIO;
	color_alpha = (color >> 24) * OVL_PATTERN_RATIO;
	color_rgb = (((color >> 16) & 0xff) << 20) | (((color >> 8) & 0xff) << 10) | (color & 0xff);

	blend_mode = get_ovl_blending_mode(hisi_plane_state, format);
	if ((blend_mode < 0) || (blend_mode >= DSS_BLEND_MAX)) {
		HISI_DRM_ERR("blend_mode = %d is out of range!\n", blend_mode);
		return -EINVAL;
	}

	HISI_DRM_DEBUG("layer=%d, rgb=%d, glb_alpha=%d, color_alpha=%d, blend=%d",
			layer_idx, color_rgb, glb_alpha, color_alpha, blend_mode);

	ovl->ovl_layer_used[layer_idx] = 1;

	hisi_dss_set_layer_alpha_config(&(ovl->ovl_layer[layer_idx]), blend_mode, glb_alpha);

	if (hisi_plane_state->need_cap & (CAP_DIM | CAP_PURE_COLOR))
		ovl->ovl_layer[layer_idx].layer_pattern_alpha =
			set_bits32(ovl->ovl_layer[layer_idx].layer_pattern_alpha, color_alpha, 32, 0);

	ovl->ovl_layer[layer_idx].layer_pattern = set_bits32(ovl->ovl_layer[layer_idx].layer_pattern, color_rgb, 32, 0);
	ovl->ovl_layer[layer_idx].layer_cfg = set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 0x1, 1, 0);

	if (hisi_plane_state->need_cap & (CAP_DIM | CAP_PURE_COLOR))
		ovl->ovl_layer[layer_idx].layer_cfg = set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 0x1, 1, 8);
	else
		ovl->ovl_layer[layer_idx].layer_cfg = set_bits32(ovl->ovl_layer[layer_idx].layer_cfg, 0x0, 1, 8);

	hisi_dss_set_layer_pos_config(&(ovl->ovl_layer[layer_idx]), &(ovl->ovl_layer_pos[layer_idx]), dst_rect);

	return 0;
}

static int hisi_dss_mctl_ch_config(struct dss_module_reg *dss_module, int chn_idx,
	int ovl_idx, int layer_idx, uint32_t dst_y,
	struct hisi_drm_plane_state *hisi_plane_state)
{
	dss_mctl_sys_t *mctl_sys = NULL;
	dss_mctl_ch_t *mctl_ch = NULL;
	int ch_ov_sel_pattern = 8;
	int chn_ov_sel_max_num = 7;

	if (hisi_plane_state->need_cap & CAP_BASE)
		return 0;

	ch_ov_sel_pattern = 0xE;

	mctl_sys = &(dss_module->mctl_sys);
	dss_module->mctl_sys_used = 1;

	if (hisi_plane_state->need_cap & (CAP_DIM | CAP_PURE_COLOR)) {
		if (layer_idx < chn_ov_sel_max_num) {
			mctl_sys->chn_ov_sel[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel[ovl_idx],
				ch_ov_sel_pattern, 4, (layer_idx + 1) * 4);
			mctl_sys->chn_ov_sel_used[ovl_idx] = 1;
		} else {
			mctl_sys->chn_ov_sel1[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel1[ovl_idx],
				ch_ov_sel_pattern, 4, 0);
			mctl_sys->chn_ov_sel_used[ovl_idx] = 1;
		}
	} else {
		mctl_ch = &(dss_module->mctl_ch[chn_idx]);
		dss_module->mctl_ch_used[chn_idx] = 1;

		mctl_ch->chn_mutex = set_bits32(mctl_ch->chn_mutex, 0x1, 1, 0);
		mctl_ch->chn_flush_en = set_bits32(mctl_ch->chn_flush_en, 0x1, 1, 0);
		mctl_ch->chn_ov_oen =
			set_bits32(mctl_ch->chn_ov_oen, 0x100 << (uint32_t)ovl_idx, 32, 0);
		mctl_ch->chn_starty = set_bits32(mctl_ch->chn_starty, dst_y | (0x8 << 16), 32, 0);

		if (chn_idx == DSS_RCHN_V2)
			chn_idx = 0x8;

		if (layer_idx < chn_ov_sel_max_num) {
			mctl_sys->chn_ov_sel[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel[ovl_idx],
				chn_idx, 4, (layer_idx + 1) * 4);
			mctl_sys->chn_ov_sel_used[ovl_idx] = 1;
		} else {
			mctl_sys->chn_ov_sel1[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel1[ovl_idx],
				chn_idx, 4, 0);
			mctl_sys->chn_ov_sel_used[ovl_idx] = 1;
		}
	}

	return 0;
}


static void hisi_get_rect_from_plane(struct drm_plane_state *state,
	struct dss_rect_ltrb *src_rect, struct dss_rect_ltrb *dst_rect)
{
	src_rect->left = state->src_x >> 16;
	src_rect->right = src_rect->left + (state->src_w >> 16) - 1;
	src_rect->top = state->src_y >> 16;
	src_rect->bottom = src_rect->top + (state->src_h >> 16) - 1;

	dst_rect->left = state->crtc_x;
	dst_rect->right = state->crtc_x + state->crtc_w - 1;
	dst_rect->top = state->crtc_y;
	dst_rect->bottom = state->crtc_y + state->crtc_h - 1;
}

static int hisi_ov_compose_handler(struct drm_plane *plane)
{
	int ret;
	struct drm_plane_state *state = NULL;
	struct hisi_drm_plane *hisi_plane = NULL;
	struct hisi_drm_plane_state *hisi_plane_state = NULL;
	struct drm_framebuffer *fb = NULL;
	struct hisi_drm_crtc *hisi_crtc = NULL;
	struct dss_module_reg *dss_module = NULL;
	struct dss_rect_ltrb clip_rect;
	struct dss_rect aligned_rect;
	struct dss_rect_ltrb dst_rect;
	struct dss_rect_ltrb src_rect;
	bool rdma_stretch_enable = false;
	uint32_t hal_fmt;
	int chn_idx;
	int ovl_idx;

#ifdef CONFIG_DRM_DSS_IOMMU
	bool mmu_enable = true;
#else
	bool mmu_enable = false;
#endif

	state = plane->state;
	drm_check_and_return(state == NULL, -EINVAL, ERR, "state is NULL");
	drm_check_and_return(state->crtc == NULL, -EINVAL, ERR, "crtc is NULL");

	fb = state->fb;
	drm_check_and_return(fb == NULL, -EINVAL, ERR, "fb is NULL");
	drm_check_and_return(fb->format == NULL, -EINVAL, ERR, "format is NULL");

	hisi_plane = to_hisi_drm_plane(plane);
	hisi_plane_state = to_hisi_plane_state(state);
	hisi_crtc = to_hisi_drm_crtc(state->crtc);
	dss_module = &hisi_crtc->dss_module;

	chn_idx = hisi_plane->chn;
	ovl_idx = hisi_crtc->ovl_idx;

	hal_fmt = dss_get_format(fb->format->format, fb->modifier);
	drm_check_and_return(hal_fmt == HISI_DSS_FORMAT_UNSUPPORT,
		-EINVAL, ERR, "format not support");

	hisi_get_rect_from_plane(state, &src_rect, &dst_rect);

	if (hisi_plane_state->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR)) {
		ret = hisi_dss_ovl_layer_config(dss_module, ovl_idx, &dst_rect, state->zpos, hal_fmt, hisi_plane_state);
		drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_ovl_config failed");

		ret = hisi_dss_mctl_ch_config(dss_module, chn_idx, ovl_idx, state->zpos, dst_rect.top, hisi_plane_state);
		drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_mctl_ch_config failed");

		return ret;
	}
#ifdef CONFIG_HISI_DSS_V510
	ret = hisi_dss_smmu_ch_config(dss_module, chn_idx, mmu_enable);
#else
	ret = hisi_drm_smmu_ch_config(dss_module, chn_idx, mmu_enable, hisi_crtc->frame_no);
#endif
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_smmu_ch_config failed");

	ret = hisi_dss_rdma_config(dss_module, hisi_plane_state, chn_idx, ovl_idx, &clip_rect,
		&aligned_rect, &rdma_stretch_enable);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "dss rdma config failed");

	ret = hisi_dss_mif_config(dss_module, state, chn_idx, mmu_enable, rdma_stretch_enable);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_mif_config failed");

	ret = hisi_dss_rdfc_config(dss_module, hal_fmt, chn_idx, &aligned_rect, clip_rect);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_mif_config failed");

	ret = hisi_dss_scl_config(dss_module, &src_rect, &dst_rect, chn_idx, hal_fmt);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_scl_config failed");

	ret = hisi_dss_arsr_pre_config(dss_module, &src_rect, &dst_rect, chn_idx);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_arsr_pre_config failed");

	ret = hisi_dss_ovl_layer_config(dss_module, ovl_idx, &dst_rect, state->zpos, hal_fmt, hisi_plane_state);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_ovl_layer_config failed");

	ret = hisi_dss_mctl_ch_config(dss_module, chn_idx, ovl_idx, state->zpos, dst_rect.top, hisi_plane_state);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_mctl_ch_config failed");

	ret = hisi_dss_ch_module_set_regs(hisi_crtc, chn_idx);
	drm_check_and_return(ret != 0, -EINVAL, ERR, "hisi_dss_ch_module_set_regs failed");

	return 0;
}

void hisi_dss_update_layer(struct drm_plane *plane)
{
	struct hisi_drm_plane *hisi_plane = NULL;
	struct hisi_drm_crtc *hisi_crtc = NULL;
	struct drm_plane_state *state = NULL;
	dss_aif_bw_t *aif_bw = NULL;
	int chn_idx;
	int ret;

	drm_check_and_void_return(plane == NULL, "plane is NULL");
	drm_check_and_void_return(plane->state == NULL, "state is NULL");
	drm_check_and_void_return(plane->state->crtc == NULL, "crtc is NULL");

	hisi_plane = to_hisi_drm_plane(plane);
	hisi_crtc = to_hisi_drm_crtc(plane->state->crtc);
	chn_idx = hisi_plane->chn;
	aif_bw = &(hisi_crtc->dss_module.aif_bw[chn_idx]);
	state = plane->state;
	hisi_crtc->layer_mask |= (1 << chn_idx);
	hisi_crtc->pre_layer_mask &= ~(1 << chn_idx);
	hisi_crtc->has_layer = true;

	// FIX bw
	aif_bw->bw = (state->src_w >> 16) * (state->src_h >> 16);
	aif_bw->chn_idx = chn_idx;
	aif_bw->is_used = 1;
	aif_bw->axi_sel = AXI_CHN0;

	HISI_DRM_DEBUG("layer chn=%d zpos=%d, rotation=%d, src: %d,%d,%d,%d, crtc: %d,%d,%d,%d, hisi_crtc->pre_layer_mask %d\n",
		chn_idx, state->zpos, state->rotation, state->src_x >> 16, state->src_y >> 16, state->src_w >> 16,
		state->src_h >> 16, state->crtc_x, state->crtc_y, state->crtc_w, state->crtc_h, hisi_crtc->pre_layer_mask);

	down(&hisi_crtc->sem);
	ret = hisi_ov_compose_handler(plane);
	if (ret != 0)
		HISI_DRM_ERR("hisi_ov_compose_handler failed");
	up(&hisi_crtc->sem);

}

/*lint +e502 +e530 +e574 +e732*/
