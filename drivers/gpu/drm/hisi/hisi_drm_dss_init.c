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
/*lint -e529*/
#include <securec.h>

#include "hisi_drm_dss_global.h"
#include "hisi_defs.h"

#define HIGH16(val) (val << 16)
#define LOW16(val) (val)

static void hisi_dss_mctl_init(dss_mctl_t *s_mctl)
{
	int ret;

	ret = memset_s(s_mctl, sizeof(dss_mctl_t), 0, sizeof(dss_mctl_t));
	drm_check_and_void_return(ret != EOK, "memset failed!");
}

static void hisi_dss_ovl_init(const char __iomem *ovl_base, dss_ovl_t *s_ovl)
{
	int i;
	int ret;

	ret = memset_s(s_ovl, sizeof(dss_ovl_t), 0, sizeof(dss_ovl_t));
	drm_check_and_void_return(ret != EOK, "memset failed!");

	s_ovl->ovl_size = inp32(ovl_base + OV_SIZE);
	s_ovl->ovl_bg_color = inp32(ovl_base + OV_BG_COLOR_RGB);
	s_ovl->ovl_bg_color_alpha = inp32(ovl_base + OV_BG_COLOR_A);
	s_ovl->ovl_dst_startpos = inp32(ovl_base + OV_DST_STARTPOS);
	s_ovl->ovl_dst_endpos = inp32(ovl_base + OV_DST_ENDPOS);
	s_ovl->ovl_gcfg = inp32(ovl_base + OV_GCFG);

	for (i = 0; i < OV_8LAYER_NUM; i++) {
		s_ovl->ovl_layer[i].layer_pos =
			inp32(ovl_base + OV_LAYER0_POS + i * 0x60);
		s_ovl->ovl_layer[i].layer_size =
			inp32(ovl_base + OV_LAYER0_SIZE + i * 0x60);
		s_ovl->ovl_layer[i].layer_pattern =
			inp32(ovl_base + OV_LAYER0_PATTERN_RGB + i * 0x60);
		s_ovl->ovl_layer[i].layer_pattern_alpha =
			inp32(ovl_base + OV_LAYER0_PATTERN_A + i * 0x60);
		s_ovl->ovl_layer[i].layer_alpha =
			inp32(ovl_base + OV_LAYER0_ALPHA_MODE + i * 0x60);
		s_ovl->ovl_layer[i].layer_alpha_a =
			inp32(ovl_base + OV_LAYER0_ALPHA_A + i * 0x60);
		s_ovl->ovl_layer[i].layer_cfg =
			inp32(ovl_base + OV_LAYER0_CFG + i * 0x60);
		s_ovl->ovl_layer_pos[i].layer_pspos =
			inp32(ovl_base + OV_LAYER0_PSPOS + i * 0x60);
		s_ovl->ovl_layer_pos[i].layer_pepos =
			inp32(ovl_base + OV_LAYER0_PEPOS + i * 0x60);
	}

	s_ovl->ovl_block_size = inp32(ovl_base + OV8_BLOCK_SIZE);
}

static void hisi_dss_aif_init(const char __iomem *aif_ch_base,
	dss_aif_t *s_aif)
{
	int ret;

	ret = memset_s(s_aif, sizeof(dss_aif_t), 0, sizeof(dss_aif_t));
	drm_check_and_void_return(ret != EOK, "memset failed!");

	s_aif->aif_ch_ctl = inp32(aif_ch_base + AIF_CH_CTL);
	s_aif->aif_ch_hs = inp32(aif_ch_base + AIF_CH_HS);
	s_aif->aif_ch_ls = inp32(aif_ch_base + AIF_CH_LS);
}

static void hisi_dss_mif_init(dss_mif_t *s_mif, int chn_idx)
{
	uint32_t rw_type;
	int ret;

	ret = memset_s(s_mif, sizeof(dss_mif_t), 0, sizeof(dss_mif_t));
	drm_check_and_void_return(ret != EOK, "memset failed!");

	s_mif->mif_ctrl1 = 0x00000020;
	s_mif->mif_ctrl2 = 0x0;
	s_mif->mif_ctrl3 = 0x0;
	s_mif->mif_ctrl4 = 0x0;
	s_mif->mif_ctrl5 = 0x0;

	rw_type = (chn_idx < DSS_WCHN_W0 || chn_idx == DSS_RCHN_V2) ? 0x0 : 0x1;

	s_mif->mif_ctrl1 = set_bits32(s_mif->mif_ctrl1, 0x0, 1, 5);
	s_mif->mif_ctrl1 = set_bits32(s_mif->mif_ctrl1, rw_type, 1, 17);
}

static void hisi_dss_mctl_ch_starty_init(const char __iomem *mctl_ch_starty_base, dss_mctl_ch_t *s_mctl_ch)
{
	int ret;

	ret = memset_s(s_mctl_ch, sizeof(dss_mctl_ch_t), 0, sizeof(dss_mctl_ch_t));
	drm_check_and_void_return(ret != EOK, "memset failed!");

	s_mctl_ch->chn_starty = inp32(mctl_ch_starty_base);
}

static void hisi_dss_mctl_ch_mod_dbg_init(const char __iomem *mctl_ch_dbg_base, dss_mctl_ch_t *s_mctl_ch)
{
	s_mctl_ch->chn_mod_dbg = inp32(mctl_ch_dbg_base);
}

static void hisi_dss_rdma_init(const char __iomem *dma_base, dss_rdma_t *s_dma)
{
	int ret;

	ret = memset_s(s_dma, sizeof(dss_rdma_t), 0, sizeof(dss_rdma_t));
	drm_check_and_void_return(ret != EOK, "memset failed!");

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


static void hisi_dss_dfc_init(const char __iomem *dfc_base, dss_dfc_t *s_dfc)
{
	int ret;

	ret = memset_s(s_dfc, sizeof(dss_dfc_t), 0, sizeof(dss_dfc_t));
	drm_check_and_void_return(ret != EOK, "memset failed!");

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

static void hisi_dss_scl_init(const char __iomem *scl_base, dss_scl_t *s_scl)
{
	int ret;

	ret = memset_s(s_scl, sizeof(dss_scl_t), 0, sizeof(dss_scl_t));
	drm_check_and_void_return(ret != EOK, "memset failed!");

	s_scl->en_hscl_str = inp32(scl_base + SCF_EN_HSCL_STR);
	s_scl->en_vscl_str = inp32(scl_base + SCF_EN_VSCL_STR);
	s_scl->h_v_order = inp32(scl_base + SCF_H_V_ORDER);
	s_scl->input_width_height = inp32(scl_base + SCF_INPUT_WIDTH_HEIGHT);
	s_scl->output_width_height = inp32(scl_base + SCF_OUTPUT_WIDTH_HEIGHT);
	s_scl->en_hscl = inp32(scl_base + SCF_EN_HSCL);
	s_scl->en_vscl = inp32(scl_base + SCF_EN_VSCL);
	s_scl->acc_hscl = inp32(scl_base + SCF_ACC_HSCL);
	s_scl->inc_hscl = inp32(scl_base + SCF_INC_HSCL);
	s_scl->inc_vscl = inp32(scl_base + SCF_INC_VSCL);
	s_scl->en_mmp = inp32(scl_base + SCF_EN_MMP);
}

static void arsr_pre_para_init(struct arsr2p_info *arsr_para)
{
	drm_check_and_void_return(arsr_para == NULL, "hisi_crtc is NULL");

	arsr_para->skin_thres_y = (600 << 20) | (332 << 10) | 300; /* 0x2585312C */
	arsr_para->skin_thres_u = (452 << 20) | (40 << 10) | 20;   /* 0x1C40A014 */
	arsr_para->skin_thres_v = (580 << 20) | (48 << 10) | 24;   /* 0x2440C018 */
	arsr_para->skin_cfg0 = (12 << 13) | 512;                   /* 0x00018200 */
	arsr_para->skin_cfg1 = 819;                                /* 0x00000333 */
	arsr_para->skin_cfg2 = 682;                                /* 0x000002AA */
	arsr_para->shoot_cfg1 = (20 << 16) | 341;                  /* 0x00140155 */
	arsr_para->shoot_cfg2 = (-80 & 0x7ff) | (16 << 16);        /* 0x001007B0 */
	arsr_para->shoot_cfg3 = 20;                                /* 0x00000014 */
	arsr_para->sharp_cfg3 = HIGH16(0xA0) | LOW16(0x60);        /* 0x00A00060 */
	arsr_para->sharp_cfg4 = HIGH16(0x60) | LOW16(0x20);        /* 0x00600020 */
	arsr_para->sharp_cfg6 = HIGH16(0x4) | LOW16(0x8);          /* 0x00040008 */
	arsr_para->sharp_cfg7 = (6 << 8) | 10;                     /* 0x0000060A */
	arsr_para->sharp_cfg8 = HIGH16(0xA0) | LOW16(0x10);        /* 0x00A00010 */

	arsr_para->sharp_level = 0x0020002;
	arsr_para->sharp_gain_low = 0x3C0078;
	arsr_para->sharp_gain_mid = 0x6400C8;
	arsr_para->sharp_gain_high = 0x5000A0;
	arsr_para->sharp_gainctrl_sloph_mf = 0x280;
	arsr_para->sharp_gainctrl_slopl_mf = 0x1400;
	arsr_para->sharp_gainctrl_sloph_hf = 0x140;
	arsr_para->sharp_gainctrl_slopl_hf = 0xA00;
	arsr_para->sharp_mf_lmt = 0x40;
	arsr_para->sharp_gain_mf = 0x12C012C;
	arsr_para->sharp_mf_b = 0;
	arsr_para->sharp_hf_lmt = 0x80;
	arsr_para->sharp_gain_hf = 0x104012C;
	arsr_para->sharp_hf_b = 0x1400;
	arsr_para->sharp_lf_ctrl = 0x100010;
	arsr_para->sharp_lf_var = 0x1800080;
	arsr_para->sharp_lf_ctrl_slop = 0;
	arsr_para->sharp_hf_select = 0;
	arsr_para->sharp_cfg2_h = 0x10000C0;
	arsr_para->sharp_cfg2_l = 0x200010;
	arsr_para->texture_analysis = 0x500040;
	arsr_para->intplshootctrl = 0x8;
}

static void hisi_dss_arsr_pre_init(const char __iomem *arsr2p_base, struct dss_arsr2p *s_arsr2p)
{
	int ret;

	ret = memset_s(s_arsr2p, sizeof(struct dss_arsr2p), 0, sizeof(struct dss_arsr2p));
	drm_check_and_void_return(ret != EOK, "memset for arsr2p failed!");

	s_arsr2p->arsr_input_width_height = inp32(arsr2p_base + ARSR2P_INPUT_WIDTH_HEIGHT);
	s_arsr2p->arsr_output_width_height = inp32(arsr2p_base + ARSR2P_OUTPUT_WIDTH_HEIGHT);
	s_arsr2p->ihleft = inp32(arsr2p_base + ARSR2P_IHLEFT);
	s_arsr2p->ihleft1 = inp32(arsr2p_base + ARSR2P_IHLEFT1);
	s_arsr2p->ihright = inp32(arsr2p_base + ARSR2P_IHRIGHT);
	s_arsr2p->ihright1 = inp32(arsr2p_base + ARSR2P_IHRIGHT1);
	s_arsr2p->ivtop = inp32(arsr2p_base + ARSR2P_IVTOP);
	s_arsr2p->ivbottom = inp32(arsr2p_base + ARSR2P_IVBOTTOM);
	s_arsr2p->ivbottom1 = inp32(arsr2p_base + ARSR2P_IVBOTTOM1);
	s_arsr2p->ihinc = inp32(arsr2p_base + ARSR2P_IHINC);
	s_arsr2p->ivinc = inp32(arsr2p_base + ARSR2P_IVINC);
	s_arsr2p->offset = inp32(arsr2p_base + ARSR2P_OFFSET);
	s_arsr2p->mode = inp32(arsr2p_base + ARSR2P_MODE);
	s_arsr2p->mode = ((s_arsr2p->mode) & 0x1FF) | 0x800; /* dbg_en=4 for memory low power */

	arsr_pre_para_init(&s_arsr2p->arsr2p_effect);
	arsr_pre_para_init(&s_arsr2p->arsr2p_effect_scale_up);
	arsr_pre_para_init(&s_arsr2p->arsr2p_effect_scale_down);
}


static void hisi_dss_mctl_sys_init(const char __iomem *mctl_sys_base, dss_mctl_sys_t *s_mctl_sys)
{
	int i = 0;
	int ret;

	ret = memset_s(s_mctl_sys, sizeof(dss_mctl_sys_t), 0, sizeof(dss_mctl_sys_t));
	drm_check_and_void_return(ret != EOK, "memset for arsr2p failed!");

	for (i = 0; i < DSS_OVL_IDX_MAX; i++) {
		s_mctl_sys->chn_ov_sel[i] = inp32(mctl_sys_base + MCTL_RCH_OV0_SEL + i * 0x4);
		if (i <= DSS_OVL1)
			s_mctl_sys->chn_ov_sel1[i] = inp32(mctl_sys_base + MCTL_RCH_OV0_SEL1 + i * 0x4);
	}

	for (i = 0; i < DSS_WCH_MAX; i++)
		s_mctl_sys->wchn_ov_sel[i] = inp32(mctl_sys_base + MCTL_WCH_OV2_SEL + i * 0x4);
}

static void hisi_dss_smmu_init(struct dss_smmu *s_smmu)
{
	int ret;

	ret = memset_s(s_smmu, sizeof(struct dss_smmu), 0, sizeof(struct dss_smmu));
	drm_check_and_void_return(ret != EOK, "memset for arsr2p failed!");
}

static void hisi_dss_chnl_model_init(uint32_t module_base, char __iomem *dss_base,
	struct dss_module_reg *dss_module, uint32_t chnl)
{
	module_base = g_dss_module_base[chnl][MODULE_AIF0_CHN];
	if (module_base != 0) {
		dss_module->aif_ch_base[chnl] = dss_base + module_base;
		hisi_dss_aif_init(dss_module->aif_ch_base[chnl], &(dss_module->aif[chnl]));
	}

	module_base = g_dss_module_base[chnl][MODULE_AIF1_CHN];
	if (module_base != 0) {
		dss_module->aif1_ch_base[chnl] = dss_base + module_base;
		hisi_dss_aif_init(dss_module->aif1_ch_base[chnl], &(dss_module->aif1[chnl]));
	}

	module_base = g_dss_module_base[chnl][MODULE_MIF_CHN];
	if (module_base != 0) {
		dss_module->mif_ch_base[chnl] = dss_base + module_base;
		hisi_dss_mif_init(&(dss_module->mif[chnl]), chnl);
	}

	module_base = g_dss_module_base[chnl][MODULE_MCTL_CHN_STARTY];
	if (module_base != 0) {
		dss_module->mctl_ch_base[chnl].chn_starty_base = dss_base + module_base;
		hisi_dss_mctl_ch_starty_init(dss_module->mctl_ch_base[chnl].chn_starty_base,
			&(dss_module->mctl_ch[chnl]));
	}

	module_base = g_dss_module_base[chnl][MODULE_MCTL_CHN_MOD_DBG];
	if (module_base != 0) {
		dss_module->mctl_ch_base[chnl].chn_mod_dbg_base = dss_base + module_base;
		hisi_dss_mctl_ch_mod_dbg_init(dss_module->mctl_ch_base[chnl].chn_mod_dbg_base,
			&(dss_module->mctl_ch[chnl]));
	}

	module_base = g_dss_module_base[chnl][MODULE_DMA];
	if (module_base != 0) {
		dss_module->dma_base[chnl] = dss_base + module_base;
		if (chnl < DSS_WCHN_W0 || chnl == DSS_RCHN_V2)
			hisi_dss_rdma_init(dss_module->dma_base[chnl], &(dss_module->rdma[chnl]));
	}

	module_base = g_dss_module_base[chnl][MODULE_DFC];
	if (module_base != 0) {
		dss_module->dfc_base[chnl] = dss_base + module_base;
		hisi_dss_dfc_init(dss_module->dfc_base[chnl], &(dss_module->dfc[chnl]));
	}

	module_base = g_dss_module_base[chnl][MODULE_SCL];
	if (module_base != 0) {
		dss_module->scl_base[chnl] = dss_base + module_base;
		hisi_dss_scl_init(dss_module->scl_base[chnl], &(dss_module->scl[chnl]));
	}

	module_base = g_dss_module_base[chnl][MODULE_ARSR2P];
	if (module_base != 0) {
		dss_module->arsr2p_base[chnl] = dss_base + module_base;
		hisi_dss_arsr_pre_init(dss_module->arsr2p_base[chnl], &(dss_module->arsr2p[chnl]));
	}
}

int hisi_dss_module_default(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_module_reg *dss_module = NULL;
	uint32_t module_base = 0;
	char __iomem *dss_base = NULL;
	int i = 0;
	int ret;

	drm_check_and_return(hisi_crtc == NULL, -EINVAL, ERR, "hisi_crtc is NULL");
	drm_check_and_return(hisi_crtc->ctx == NULL, -EINVAL, ERR,
		"ctx in hisi_crtc is NULL");

	dss_base = hisi_crtc->ctx->dss_base;
	drm_check_and_return(dss_base == NULL, -EINVAL, ERR,
		"dss_base is NULL");

	dss_module = &(hisi_crtc->dss_module_default);
	ret = memset_s(dss_module, sizeof(struct dss_module_reg), 0, sizeof(struct dss_module_reg));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset failed!");

	for (i = 0; i < DSS_MCTL_IDX_MAX; i++) {
		module_base = g_dss_module_ovl_base[i][MODULE_MCTL_BASE];
		if (module_base != 0) {
			dss_module->mctl_base[i] = dss_base + module_base;
			hisi_dss_mctl_init(&(dss_module->mctl[i]));
		}
	}

	for (i = 0; i < DSS_OVL_IDX_MAX; i++) {
		module_base = g_dss_module_ovl_base[i][MODULE_OVL_BASE];
		if (module_base != 0) {
			dss_module->ov_base[i] = dss_base + module_base;
			hisi_dss_ovl_init(dss_module->ov_base[i], &(dss_module->ov[i]));
		}
	}

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (i == DSS_WCHN_W2)
			continue;

		module_base = g_dss_module_base[i][MODULE_MCTL_CHN_MUTEX];
		if (module_base != 0)
			dss_module->mctl_ch_base[i].chn_mutex_base = dss_base + module_base;

		module_base = g_dss_module_base[i][MODULE_MCTL_CHN_FLUSH_EN];
		if (module_base != 0)
			dss_module->mctl_ch_base[i].chn_flush_en_base = dss_base + module_base;

		module_base = g_dss_module_base[i][MODULE_MCTL_CHN_OV_OEN];
		if (module_base != 0)
			dss_module->mctl_ch_base[i].chn_ov_en_base = dss_base + module_base;

		hisi_dss_chnl_model_init(module_base, dss_base, dss_module, i);
	}

	dss_module->mctl_sys_base = dss_base + DSS_MCTRL_SYS_OFFSET;
	hisi_dss_mctl_sys_init(dss_module->mctl_sys_base, &(dss_module->mctl_sys));

	dss_module->smmu_base = dss_base + DSS_SMMU_OFFSET;
	hisi_dss_smmu_init(&(dss_module->smmu));

	return 0;
}

/*lint +e529*/
