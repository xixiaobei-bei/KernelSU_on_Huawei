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

static void hisi_dss_post_scf_scale_init(const char __iomem *dss_base,
	const char __iomem *post_scf_base, dss_arsr1p_t *s_post_scf)
{
	s_post_scf->dpp_img_size_bef_sr = (uint32_t)inp32(dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR);
	s_post_scf->dpp_img_size_aft_sr = (uint32_t)inp32(dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR);

	s_post_scf->ihleft = (uint32_t)inp32(post_scf_base + ARSR_POST_IHLEFT);
	s_post_scf->ihleft1 = (uint32_t)inp32(post_scf_base + ARSR_POST_IHLEFT1);
	s_post_scf->ihright = (uint32_t)inp32(post_scf_base + ARSR_POST_IHRIGHT);
	s_post_scf->ihright1 = (uint32_t)inp32(post_scf_base + ARSR_POST_IHRIGHT1);
	s_post_scf->ivtop = (uint32_t)inp32(post_scf_base + ARSR_POST_IVTOP);
	s_post_scf->ivbottom = (uint32_t)inp32(post_scf_base + ARSR_POST_IVBOTTOM);
	s_post_scf->ivbottom1 = (uint32_t)inp32(post_scf_base + ARSR_POST_IVBOTTOM1);
	s_post_scf->ihinc = (uint32_t)inp32(post_scf_base + ARSR_POST_IHINC);
	s_post_scf->ivinc = (uint32_t)inp32(post_scf_base + ARSR_POST_IVINC);
	s_post_scf->uv_offset = (uint32_t)inp32(post_scf_base + ARSR_POST_UV_OFFSET);
	s_post_scf->mode = (uint32_t)inp32(post_scf_base + ARSR_POST_MODE);
}

static void hisi_dss_post_scf_effect_init(dss_arsr1p_t *s_post_scf)
{
	/* The follow code from chip protocol, It contains lots of fixed numbers */
	s_post_scf->skin_thres_y = (600 << 20) | (332 << 10) | 300;  /* 0x2585312C */
	s_post_scf->skin_thres_u = (452 << 20) | (40 << 10) | 20;  /* 0x1C40A014 */
	s_post_scf->skin_thres_v = (580 << 20) | (48 << 10) | 24;  /* 0x2440C018 */
	s_post_scf->skin_cfg0 = (12 << 13) | 512;  /* 0x00018200 */
	s_post_scf->skin_cfg1 = 819;  /* 0x00000333 */
	s_post_scf->skin_cfg2 = 682;  /* 0x000002AA */
	s_post_scf->shoot_cfg1 = (20 << 16) | 341;  /* 0x00140155 */
	s_post_scf->shoot_cfg2 = (-80 & 0x7ff) | (16 << 16);  /* 0x001007B0 */
	s_post_scf->shoot_cfg3 = 20;  /* 0x00000014 */
	s_post_scf->sharp_cfg3 = (0xA0 << 16) | 0x60;  /* 0x00A00060 */
	s_post_scf->sharp_cfg4 = (0x60 << 16) | 0x20;  /* 0x00600020 */
	s_post_scf->sharp_cfg5 = 0;
	s_post_scf->sharp_cfg6 = (0x4 << 16) | 0x8;  /* 0x00040008 */
	s_post_scf->sharp_cfg7 = (6 << 8) | 10;  /* 0x0000060A */
	s_post_scf->sharp_cfg8 = (0xA0 << 16) | 0x10;  /* 0x00A00010 */

	s_post_scf->sharp_level = 0x0020002;
	s_post_scf->sharp_gain_low = 0x3C0078;
	s_post_scf->sharp_gain_mid = 0x6400C8;
	s_post_scf->sharp_gain_high = 0x5000A0;
	s_post_scf->sharp_gainctrl_sloph_mf = 0x280;
	s_post_scf->sharp_gainctrl_slopl_mf = 0x1400;
	s_post_scf->sharp_gainctrl_sloph_hf = 0x140;
	s_post_scf->sharp_gainctrl_slopl_hf = 0xA00;
	s_post_scf->sharp_mf_lmt = 0x40;
	s_post_scf->sharp_gain_mf = 0x12C012C;
	s_post_scf->sharp_mf_b = 0;
	s_post_scf->sharp_hf_lmt = 0x80;
	s_post_scf->sharp_gain_hf = 0x104012C;
	s_post_scf->sharp_hf_b = 0x1400;
	s_post_scf->sharp_lf_ctrl = 0x100010;
	s_post_scf->sharp_lf_var = 0x1800080;
	s_post_scf->sharp_lf_ctrl_slop = 0;
	s_post_scf->sharp_hf_select = 0;
	s_post_scf->sharp_cfg2_h = 0x10000C0;
	s_post_scf->sharp_cfg2_l = 0x200010;
	s_post_scf->texture_analysis = 0x500040;
	s_post_scf->intplshootctrl = 0x8;
}

void hisi_dss_post_scf_init(const char __iomem *dss_base,
	const char __iomem *post_scf_base, dss_arsr1p_t *s_post_scf)
{
	dpu_check_and_no_retval(!post_scf_base, ERR, "post_scf_base is NULL!\n");
	dpu_check_and_no_retval(!s_post_scf, ERR, "s_post_scf is NULL!\n");
	dpu_check_and_no_retval(!dss_base, ERR, "dss_base is NULL!\n");

	memset(s_post_scf, 0, sizeof(dss_arsr1p_t));

	hisi_dss_post_scf_scale_init(dss_base, post_scf_base, s_post_scf);
	hisi_dss_post_scf_effect_init(s_post_scf);
}

static int post_scf_rect_set(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo, dss_overlay_t *pov_req,
	dss_rect_t *src_rect, dss_rect_t *dst_rect)
{
	if (pov_req) {
		if ((pov_req->res_updt_rect.w <= 0) || (pov_req->res_updt_rect.h <= 0)) {
			DPU_FB_DEBUG("fb%d, res_updt_rect[%d,%d, %d,%d] is invalid!\n",
				dpufd->index,
				pov_req->res_updt_rect.x, pov_req->res_updt_rect.y,
				pov_req->res_updt_rect.w, pov_req->res_updt_rect.h);
			return  0;
		}

		if ((pov_req->res_updt_rect.w == dpufd->ov_req_prev.res_updt_rect.w) &&
			(pov_req->res_updt_rect.h == dpufd->ov_req_prev.res_updt_rect.h)) {
			return 0;
		}

		*src_rect = pov_req->res_updt_rect;

		DPU_FB_DEBUG("fb%d, post scf res_updt_rect[%d, %d]->lcd_rect[%d, %d]\n",
			dpufd->index,
			pov_req->res_updt_rect.w, pov_req->res_updt_rect.h,
			pinfo->xres, pinfo->yres);
	} else {
		src_rect->x = 0;
		src_rect->y = 0;
		src_rect->w = pinfo->xres;
		src_rect->h = pinfo->yres;
	}

	if (pinfo->cascadeic_support && pov_req &&
		(pov_req->rog_width > 0 && pov_req->rog_height > 0)) {
		/* for foldable display ROG partial update */
		dst_rect->x = pov_req->res_updt_rect.x;
		dst_rect->y = pov_req->res_updt_rect.y;
		dst_rect->w = pov_req->res_updt_rect.w;
		dst_rect->h = pov_req->res_updt_rect.h;
	} else {
		dst_rect->x = 0;
		dst_rect->y = 0;
		dst_rect->w = pinfo->xres;
		dst_rect->h = pinfo->yres;
	}

	return 1;
}

static void post_scf_img_size_config(dss_arsr1p_t *post_scf, dss_rect_t src_rect, dss_rect_t dst_rect)
{
	post_scf->dpp_img_size_bef_sr = set_bits32(post_scf->dpp_img_size_bef_sr,
		(DSS_HEIGHT((uint32_t)src_rect.h) << 16) |
		DSS_WIDTH((uint32_t)src_rect.w), 32, 0);
	post_scf->dpp_img_size_aft_sr = set_bits32(post_scf->dpp_img_size_aft_sr,
		(DSS_HEIGHT((uint32_t)dst_rect.h) << 16) |
		DSS_WIDTH((uint32_t)dst_rect.w), 32, 0);
}

static int post_scf_mode_config(struct dpu_panel_info *pinfo, dss_arsr1p_t *post_scf, int32_t ihinc, int32_t ivinc)
{
	if ((ihinc == ARSR1P_INC_FACTOR) && (ivinc == ARSR1P_INC_FACTOR) && (pinfo->arsr1p_sharpness_support != 1)) {
		/* ARSR1P bypass */
		post_scf->mode = 0x1;
		return 0;
	}

	/* threshold value(8192(0x2000)~65536(0x10000)) */
	if ((ihinc < 0x2000) || (ihinc > ARSR1P_INC_FACTOR) || (ivinc < 0x2000) || (ivinc > ARSR1P_INC_FACTOR)) {
		DPU_FB_ERR("invalid ihinc 0x%x, ivinc 0x%x!\n", ihinc, ivinc);
		/* ARSR1P bypass */
		post_scf->mode = 0x1;
		return -1;
	}

	if ((ihinc > ARSR1P_INC_FACTOR) ||  (ivinc > ARSR1P_INC_FACTOR)) {
		/* scaler down, not supported */
		DPU_FB_ERR("scaling down is not supported by ARSR1P, ihinc = 0x%x, ivinc = 0x%x\n", ihinc, ivinc);
		/* ARSR1P bypass */
		post_scf->mode = 0x1;
		return -1;
	}

	/* enable arsr1p */
	post_scf->mode = 0x0;

	if (pinfo->arsr1p_sharpness_support)
		/* enable sharp  skinctrl, shootdetect */
		post_scf->mode |= 0xe;

	/* enable direction */
	post_scf->mode |= 0x10;

	if ((ihinc < ARSR1P_INC_FACTOR) ||  (ivinc < ARSR1P_INC_FACTOR))
		/* enable diintplen */
		post_scf->mode |= 0x20;
	else
		/* only sharp, enable nointplen */
		post_scf->mode |= 0x100;

	return 1;
}

static int post_scf_cordinate_config(dss_arsr1p_t *post_scf, dss_rect_t src_rect, dss_rect_t dst_rect,
	int32_t ihinc, int32_t ivinc)
{
	int32_t ihleft;
	int32_t ihright;
	int32_t ihleft1;
	int32_t ihright1;
	int32_t ivtop;
	int32_t ivbottom;
	int32_t extraw;
	int32_t extraw_left;
	int32_t extraw_right;

	dpu_check_and_return((ihinc == 0), -1, ERR, "ihinc is zero\n");

	extraw = (8 * ARSR1P_INC_FACTOR) / ihinc;  /*lint !e414*/
	extraw_left = (extraw % 2) ? (extraw + 1) : (extraw);
	extraw = (2 * ARSR1P_INC_FACTOR) / ihinc;  /*lint !e414*/
	extraw_right = (extraw % 2) ? (extraw + 1) : (extraw);

	/* ihleft1 = (startX_o * ihinc) - (ov_startX0 << 16) */
	ihleft1 = dst_rect.x * ihinc - src_rect.x * ARSR1P_INC_FACTOR;
	if (ihleft1 < 0)
		ihleft1 = 0;
	/* ihleft = ihleft1 - even(8 * 65536 / ihinc) * ihinc; */
	ihleft = ihleft1 - extraw_left * ihinc;
	if (ihleft < 0)
		ihleft = 0;
	/* ihright1 = ihleft1 + (oww-1) * ihinc */
	ihright1 = ihleft1 + (dst_rect.w - 1) * ihinc;
	/* ihright = ihright1 + even(2 * 65536/ihinc) * ihinc */
	ihright = ihright1 + extraw_right * ihinc;
	/* ihright >= img_width * ihinc */
	if (ihright >= src_rect.w * ARSR1P_INC_FACTOR)
		ihright = src_rect.w * ARSR1P_INC_FACTOR - 1;
	/* ivtop = (startY_o * ivinc) - (ov_startY0<<16) */
	ivtop = dst_rect.y * ivinc - src_rect.y * ARSR1P_INC_FACTOR;
	if (ivtop < 0)
		ivtop = 0;
	/* ivbottom = ivtop + (ohh - 1) * ivinc */
	ivbottom = ivtop + (dst_rect.h - 1) * ivinc;

	/* ivbottom >= img_height * ivinc */
	if (ivbottom >= src_rect.h * ARSR1P_INC_FACTOR)
		ivbottom = src_rect.h * ARSR1P_INC_FACTOR - 1;
	/* (ihleft1 - ihleft) % (ihinc) == 0; */
	if ((ihleft1 - ihleft) % (ihinc)) {  /*lint !e414*/
		DPU_FB_ERR("ihleft1 %d-ihleft %d  ihinc %d != 0, invalid!\n", ihleft1, ihleft, ihinc);
		post_scf->mode = 0x1;
		return -1;
	}

	/* (ihright1 - ihleft1) % ihinc == 0; */
	if ((ihright1 - ihleft1) % ihinc) {  /*lint !e414*/
		DPU_FB_ERR("ihright1 %d-ihleft1 %d  ihinc %d != 0, invalid!\n", ihright1, ihleft1, ihinc);
		post_scf->mode = 0x1;
		return -1;
	}

	post_scf->ihleft = set_bits32(post_scf->ihleft, ihleft, 32, 0);
	post_scf->ihright = set_bits32(post_scf->ihright, ihright, 32, 0);
	post_scf->ihleft1 = set_bits32(post_scf->ihleft1, ihleft1, 32, 0);
	post_scf->ihright1 = set_bits32(post_scf->ihright1, ihright1, 32, 0);
	post_scf->ivtop = set_bits32(post_scf->ivtop, ivtop, 32, 0);
	post_scf->ivbottom = set_bits32(post_scf->ivbottom, ivbottom, 32, 0);
	post_scf->ivbottom1 = set_bits32(post_scf->ivbottom1, ivbottom, 32, 0);

	return 1;
}

int hisi_dss_post_scf_config(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req)
{
	struct dpu_panel_info *pinfo = NULL;
	dss_rect_t src_rect = {0};
	dss_rect_t dst_rect = {0};
	dss_arsr1p_t *post_scf = NULL;
	int ret;
	int32_t ihinc;
	int32_t ivinc;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}
	pinfo = &(dpufd->panel_info);

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_POST_SCF))
		return 0;

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return 0;

	ret = post_scf_rect_set(dpufd, pinfo, pov_req, &src_rect, &dst_rect);
	if (ret <= 0)
		return ret;

	post_scf = &(dpufd->dss_module.post_scf);
	dpufd->dss_module.post_scf_used = 1;

	post_scf_img_size_config(post_scf, src_rect, dst_rect);
	post_scf->dpp_used = 1;

	if ((src_rect.w < ARSR_POST_SRC_MIN_WIDTH) || (src_rect.h < ARSR_POST_SRC_MIN_HEIGHT) ||
		(src_rect.w > ARSR_POST_SRC_MAX_WIDTH) || (src_rect.h > ARSR_POST_SRC_MAX_HEIGHT) ||
		(dst_rect.w > ARSR_POST_DST_MAX_WIDTH) || (dst_rect.h > ARSR_POST_DST_MAX_HEIGHT)) {
		DPU_FB_ERR("fb%d, src_rect %d,%d,%d,%d should be larger than %d*%d,less than %d*%d!\n"
			"dst_rect %d,%d,%d,%d should be less than %d*%d!\n",
			dpufd->index, src_rect.x, src_rect.y, src_rect.w, src_rect.h,
			ARSR_POST_SRC_MIN_WIDTH, ARSR_POST_SRC_MIN_HEIGHT,
			ARSR_POST_SRC_MAX_WIDTH, ARSR_POST_SRC_MAX_HEIGHT,
			dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h,
			ARSR_POST_DST_MAX_WIDTH, ARSR_POST_DST_MAX_HEIGHT);
		/* ARSR_POST bypass */
		post_scf->mode = 0x1;
		return 0;
	}

	ihinc = ARSR1P_INC_FACTOR * src_rect.w / dst_rect.w;  /*lint !e414*/
	ivinc = ARSR1P_INC_FACTOR * src_rect.h / dst_rect.h;  /*lint !e414*/

	ret = post_scf_mode_config(pinfo, post_scf, ihinc, ivinc);
	if (ret <= 0)
		return ret;

	ret = post_scf_cordinate_config(post_scf, src_rect, dst_rect, ihinc, ivinc);
	if (ret <= 0)
		return ret;

	post_scf->ihinc = set_bits32(post_scf->ihinc, ihinc, 32, 0);
	post_scf->ivinc = set_bits32(post_scf->ivinc, ivinc, 32, 0);

	return 0;
}

static void post_scf_basic_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *post_scf_base, dss_arsr1p_t *s_post_scf)
{
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_IHLEFT,
		s_post_scf->ihleft, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_IHRIGHT,
		s_post_scf->ihright, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_IHLEFT1,
		s_post_scf->ihleft1, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_IHRIGHT1,
		s_post_scf->ihright1, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_IVTOP,
		s_post_scf->ivtop, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_IVBOTTOM,
		s_post_scf->ivbottom, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_IVBOTTOM1,
		s_post_scf->ivbottom1, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_UV_OFFSET,
		s_post_scf->uv_offset, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_IHINC,
		s_post_scf->ihinc, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_IVINC,
		s_post_scf->ivinc, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_MODE,
		s_post_scf->mode, 32, 0);
}

static void post_scf_func_param_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *post_scf_base, dss_arsr1p_t *s_post_scf)
{
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SKIN_THRES_Y,
		s_post_scf->skin_thres_y, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SKIN_THRES_U,
		s_post_scf->skin_thres_u, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SKIN_THRES_V,
		s_post_scf->skin_thres_v, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SKIN_CFG0,
		s_post_scf->skin_cfg0, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SKIN_CFG1,
		s_post_scf->skin_cfg1, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SKIN_CFG2,
		s_post_scf->skin_cfg2, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHOOT_CFG1,
		s_post_scf->shoot_cfg1, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHOOT_CFG2,
		s_post_scf->shoot_cfg2, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHOOT_CFG3,
		s_post_scf->shoot_cfg3, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_CFG3,
		s_post_scf->sharp_cfg3, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_CFG4,
		s_post_scf->sharp_cfg4, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_CFG5,
		s_post_scf->sharp_cfg5, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_CFG6,
		s_post_scf->sharp_cfg6, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_CFG7,
		s_post_scf->sharp_cfg7, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_CFG8,
		s_post_scf->sharp_cfg8, 32, 0);
}

static void post_scf_sharp_control_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *post_scf_base, dss_arsr1p_t *s_post_scf)
{
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_LEVEL,
		s_post_scf->sharp_level, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_GAIN_LOW,
		s_post_scf->sharp_gain_low, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_GAIN_MID,
		s_post_scf->sharp_gain_mid, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_GAIN_HIGH,
		s_post_scf->sharp_gain_high, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_GAINCTRLSLOPH_MF,
		s_post_scf->sharp_gainctrl_sloph_mf, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_GAINCTRLSLOPL_MF,
		s_post_scf->sharp_gainctrl_slopl_mf, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_GAINCTRLSLOPH_HF,
		s_post_scf->sharp_gainctrl_sloph_hf, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_GAINCTRLSLOPL_HF,
		s_post_scf->sharp_gainctrl_slopl_hf, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_MF_LMT,
		s_post_scf->sharp_mf_lmt, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_GAIN_MF,
		s_post_scf->sharp_gain_mf, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_MF_B,
		s_post_scf->sharp_mf_b, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_HF_LMT,
		s_post_scf->sharp_hf_lmt, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_GAIN_HF,
		s_post_scf->sharp_gain_hf, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_HF_B,
		s_post_scf->sharp_hf_b, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_LF_CTRL,
		s_post_scf->sharp_lf_ctrl, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_LF_VAR,
		s_post_scf->sharp_lf_var, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_LF_CTRL_SLOP,
		s_post_scf->sharp_lf_ctrl_slop, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_HF_SELECT,
		s_post_scf->sharp_hf_select, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_CFG2_H,
		s_post_scf->sharp_cfg2_h, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_SHARP_CFG2_L,
		s_post_scf->sharp_cfg2_l, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_TEXTURE_ANALYSIS,
		s_post_scf->texture_analysis, 32, 0);
	dpufd->set_reg(dpufd, post_scf_base + ARSR_POST_INTPLSHOOTCTRL,
		s_post_scf->intplshootctrl, 32, 0);
}

void hisi_dss_post_scf_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *post_scf_base, dss_arsr1p_t *s_post_scf)
{
	uint32_t img_size_bef_sr;

	if (!dpufd || !post_scf_base || !s_post_scf) {
		DPU_FB_ERR("NULL ptr.\n");
		return;
	}

	img_size_bef_sr = s_post_scf->dpp_img_size_bef_sr;
	if (is_arsr_post_need_padding(NULL, &(dpufd->panel_info), s_post_scf->ihinc)) {
		img_size_bef_sr = s_post_scf->dpp_img_size_bef_sr + ARSR_POST_COLUMN_PADDING_NUM;
		DPU_FB_DEBUG("img_size_bef_sr 0x%x, padding to 0x%x",
			s_post_scf->dpp_img_size_bef_sr, img_size_bef_sr);
	}

	if (s_post_scf->dpp_used == 1) {
		dpufd->set_reg(dpufd,
			dpufd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR,
			img_size_bef_sr, 32, 0);
		dpufd->set_reg(dpufd,
			dpufd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR,
			s_post_scf->dpp_img_size_aft_sr, 32, 0);
	}

	post_scf_basic_set_reg(dpufd, post_scf_base, s_post_scf);

	post_scf_func_param_set_reg(dpufd, post_scf_base, s_post_scf);

	post_scf_sharp_control_set_reg(dpufd, post_scf_base, s_post_scf);
}

