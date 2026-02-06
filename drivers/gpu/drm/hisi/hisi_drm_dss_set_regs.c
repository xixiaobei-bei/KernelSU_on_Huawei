/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_drm_dpe_utils.h"
#include "hisi_drm_dss_set_regs.h"
#include "hisi_drm_dss_global.h"
#ifdef CONFIG_HISI_DSS_V600
#include <linux/hisi/hisi_drmdriver.h>
#include "hisi_drm_smmuv3.h"
#endif

static void hisi_dss_mctl_ch_set_reg(struct hisi_drm_crtc *hisi_crtc,
	struct dss_mctl_ch_base *mctl_ch_base, dss_mctl_ch_t *s_mctl_ch,
	int32_t mctl_idx)
{
	char __iomem *chn_mutex_base = NULL;
	int i = 0;

	if ((mctl_idx < DSS_MCTL0) || (mctl_idx >= DSS_MCTL_IDX_MAX)) {
		HISI_DRM_ERR("mctl_idx is out of the range!\n");
		return;
	}

	for (i = 0; i < DSS_MCTL_IDX_MAX; i++) {
		if (g_dss_module_ovl_base[i][MODULE_MCTL_BASE] == 0)
			continue;
		chn_mutex_base = mctl_ch_base->chn_mutex_base +
			g_dss_module_ovl_base[i][MODULE_MCTL_BASE];

		if (i != mctl_idx)
			hisi_set_reg(hisi_crtc, chn_mutex_base, 0, 32, 0);
	}

	chn_mutex_base = mctl_ch_base->chn_mutex_base +
		g_dss_module_ovl_base[mctl_idx][MODULE_MCTL_BASE];
	if (chn_mutex_base == NULL) {
		HISI_DRM_ERR("chn_mutex_base is NULL!\n");
		return;
	}

	hisi_set_reg(hisi_crtc, chn_mutex_base, s_mctl_ch->chn_mutex, 32, 0);
}

static void hisi_dss_rdma_afbc_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *dma_base, dss_rdma_t *s_dma)
{
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_HREG_HDR_PTR_LO,
		s_dma->afbcd_hreg_hdr_ptr_lo, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_HREG_PIC_WIDTH,
		s_dma->afbcd_hreg_pic_width, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_HREG_PIC_HEIGHT,
		s_dma->afbcd_hreg_pic_height, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_HREG_FORMAT,
		s_dma->afbcd_hreg_format, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_CTL, s_dma->afbcd_ctl, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_STR, s_dma->afbcd_str, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_LINE_CROP,
		s_dma->afbcd_line_crop, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_INPUT_HEADER_STRIDE,
		s_dma->afbcd_input_header_stride, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_PAYLOAD_STRIDE,
		s_dma->afbcd_payload_stride, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_MM_BASE_0,
		s_dma->afbcd_mm_base_0, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_AFBCD_PAYLOAD_POINTER,
		s_dma->afbcd_afbcd_payload_pointer, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_HEIGHT_BF_STR,
		s_dma->afbcd_height_bf_str, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_OS_CFG,
		s_dma->afbcd_os_cfg, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_SCRAMBLE_MODE,
		s_dma->afbcd_scramble_mode, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_CREG_FBCD_CTRL_MODE, 0x0, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + AFBCD_HEADER_POINTER_OFFSET,
		s_dma->afbcd_header_pointer_offset, 32, 0);
}

static void hisi_dss_rdma_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *dma_base, dss_rdma_t *s_dma, uint32_t chn_idx)
{
	drm_check_and_void_return(dma_base == NULL, "dma_base is NULL");

	if (chn_idx < DSS_WCHN_W0 || chn_idx == DSS_RCHN_V2) {
		hisi_set_reg(hisi_crtc, dma_base + CH_REG_DEFAULT, 0x1, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + CH_REG_DEFAULT, 0x0, 32, 0);

		hisi_set_reg(hisi_crtc, dma_base + DMA_OFT_X0, s_dma->oft_x0, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_OFT_Y0, s_dma->oft_y0, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_OFT_X1, s_dma->oft_x1, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_OFT_Y1, s_dma->oft_y1, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_MASK0, s_dma->mask0, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_MASK1, s_dma->mask1, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_STRETCH_SIZE_VRT,
			s_dma->stretch_size_vrt, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_CTRL, s_dma->ctrl, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_TILE_SCRAM, s_dma->tile_scram, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_DATA_ADDR0, s_dma->data_addr0, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_STRIDE0, s_dma->stride0, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_STRETCH_STRIDE0,
			s_dma->stretch_stride0, 32, 0);

		hisi_set_reg(hisi_crtc, dma_base + CH_CTL, s_dma->ch_ctl, 32, 0);

		if (s_dma->afbc_used)
			hisi_dss_rdma_afbc_set_reg(hisi_crtc, dma_base, s_dma);

		if (s_dma->hfbcd_used)
			HISI_DRM_ERR("not support hfbcd\n");
	}

	if ((chn_idx == DSS_RCHN_V0) || (chn_idx == DSS_RCHN_V1) ||
		(chn_idx == DSS_RCHN_V2)) {
		hisi_set_reg(hisi_crtc, dma_base + DMA_DATA_ADDR1, s_dma->data_addr1, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_STRIDE1, s_dma->stride1, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_STRETCH_STRIDE1,
			s_dma->stretch_stride1, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_DATA_ADDR2, s_dma->data_addr2, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_STRIDE2, s_dma->stride2, 32, 0);
		hisi_set_reg(hisi_crtc, dma_base + DMA_STRETCH_STRIDE2,
			s_dma->stretch_stride2, 32, 0);
	}
}

static void hisi_dss_dfc_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *dfc_base, dss_dfc_t *s_dfc)
{
	drm_check_and_void_return(dfc_base == NULL, "dfc_base is NULL");

	hisi_set_reg(hisi_crtc, dfc_base + DFC_DISP_SIZE, s_dfc->disp_size, 32, 0);
	hisi_set_reg(hisi_crtc, dfc_base + DFC_PIX_IN_NUM, s_dfc->pix_in_num, 32, 0);
	hisi_set_reg(hisi_crtc, dfc_base + DFC_DISP_FMT, s_dfc->disp_fmt, 32, 0);
	hisi_set_reg(hisi_crtc, dfc_base + DFC_CLIP_CTL_HRZ, s_dfc->clip_ctl_hrz, 32, 0);
	hisi_set_reg(hisi_crtc, dfc_base + DFC_CLIP_CTL_VRZ, s_dfc->clip_ctl_vrz, 32, 0);
	hisi_set_reg(hisi_crtc, dfc_base + DFC_CTL_CLIP_EN, s_dfc->ctl_clip_en, 32, 0);
	hisi_set_reg(hisi_crtc, dfc_base + DFC_ICG_MODULE, s_dfc->icg_module, 32, 0);
	hisi_set_reg(hisi_crtc, dfc_base + DFC_DITHER_ENABLE, s_dfc->dither_enable, 32, 0);
	hisi_set_reg(hisi_crtc, dfc_base + DFC_PADDING_CTL, s_dfc->padding_ctl, 32, 0);
	hisi_set_reg(hisi_crtc, dfc_base + DFC_BITEXT_CTL, s_dfc->bitext_ctl, 32, 0);
}

static void hisi_dss_scl_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *scl_base, dss_scl_t *s_scl)
{
	drm_check_and_void_return(scl_base == NULL, "scl_base is NULL");

	if (hisi_crtc != NULL) {
		hisi_set_reg(hisi_crtc, scl_base + SCF_EN_HSCL_STR, s_scl->en_hscl_str, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_EN_VSCL_STR, s_scl->en_vscl_str, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_H_V_ORDER, s_scl->h_v_order, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_INPUT_WIDTH_HEIGHT,
			s_scl->input_width_height, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_OUTPUT_WIDTH_HEIGHT,
			s_scl->output_width_height, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_EN_HSCL, s_scl->en_hscl, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_EN_VSCL, s_scl->en_vscl, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_ACC_HSCL, s_scl->acc_hscl, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_INC_HSCL, s_scl->inc_hscl, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_INC_VSCL, s_scl->inc_vscl, 32, 0);
		hisi_set_reg(hisi_crtc, scl_base + SCF_EN_MMP, s_scl->en_mmp, 32, 0);
	} else {
		set_reg(scl_base + SCF_EN_HSCL_STR, s_scl->en_hscl_str, 32, 0);
		set_reg(scl_base + SCF_EN_VSCL_STR, s_scl->en_vscl_str, 32, 0);
		set_reg(scl_base + SCF_H_V_ORDER, s_scl->h_v_order, 32, 0);
		set_reg(scl_base + SCF_INPUT_WIDTH_HEIGHT, s_scl->input_width_height, 32, 0);
		set_reg(scl_base + SCF_OUTPUT_WIDTH_HEIGHT,
			s_scl->output_width_height, 32, 0);
		set_reg(scl_base + SCF_EN_HSCL, s_scl->en_hscl, 32, 0);
		set_reg(scl_base + SCF_EN_VSCL, s_scl->en_vscl, 32, 0);
		set_reg(scl_base + SCF_ACC_HSCL, s_scl->acc_hscl, 32, 0);
		set_reg(scl_base + SCF_INC_HSCL, s_scl->inc_hscl, 32, 0);
		set_reg(scl_base + SCF_INC_VSCL, s_scl->inc_vscl, 32, 0);
		set_reg(scl_base + SCF_EN_MMP, s_scl->en_mmp, 32, 0);
	}
}

static void hisi_dss_arsr2p_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *arsr2p_base, struct dss_arsr2p *s_arsr2p)
{
	drm_check_and_void_return(arsr2p_base == NULL, "arsr2p_base is NULL");

	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_INPUT_WIDTH_HEIGHT,
		s_arsr2p->arsr_input_width_height, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_OUTPUT_WIDTH_HEIGHT,
		s_arsr2p->arsr_output_width_height, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_IHLEFT, s_arsr2p->ihleft, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_IHLEFT1, s_arsr2p->ihleft1, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_IHRIGHT, s_arsr2p->ihright, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_IHRIGHT1, s_arsr2p->ihright1, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_IVTOP, s_arsr2p->ivtop, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_IVBOTTOM, s_arsr2p->ivbottom, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_IVBOTTOM1, s_arsr2p->ivbottom1, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_IHINC, s_arsr2p->ihinc, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_IVINC, s_arsr2p->ivinc, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_OFFSET, s_arsr2p->offset, 32, 0);
	hisi_set_reg(hisi_crtc, arsr2p_base + ARSR2P_MODE, s_arsr2p->mode, 32, 0);
}

static void hisi_dss_chn_set_reg_default_value(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *dma_base)
{
	drm_check_and_void_return(dma_base == NULL, "dma_base is NULL");

	hisi_set_reg(hisi_crtc, dma_base + CH_REG_DEFAULT, 0x1, 32, 0);
	hisi_set_reg(hisi_crtc, dma_base + CH_REG_DEFAULT, 0x0, 32, 0);
}

#ifdef CONFIG_HISI_DSS_V600
static void hisi_dss_mif_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *mif_ch_base, dss_mif_t *s_mif)
{

#ifdef CONFIG_DRM_SMMU_V3
	/* enable pref */
	hisi_set_reg(hisi_crtc, mif_ch_base + MIF_CTRL0, 0x3, 32, 0);
#else
	/* disable pref */
	hisi_set_reg(hisi_crtc, mif_ch_base + MIF_CTRL0, 0x0, 1, 0);
#endif
	hisi_set_reg(hisi_crtc, mif_ch_base + MIF_CTRL0, 0x3, 32, 0);
	set_reg(hisi_crtc->ctx->dss_base + VBIF0_MIF_OFFSET + AIF_CMD_RELOAD, 0x1, 1, 0);
}
#endif

#ifdef CONFIG_HISI_DSS_V510
static void hisi_dss_mif_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *mif_ch_base, dss_mif_t *s_mif)
{
	drm_check_and_void_return(mif_ch_base == NULL, "mif_ch_base is NULL");

	hisi_set_reg(hisi_crtc, mif_ch_base + MIF_CTRL1,
		s_mif->mif_ctrl1, 32, 0);
	hisi_set_reg(hisi_crtc, mif_ch_base + MIF_CTRL2,
		s_mif->mif_ctrl2, 32, 0);
	hisi_set_reg(hisi_crtc, mif_ch_base + MIF_CTRL3,
		s_mif->mif_ctrl3, 32, 0);
	hisi_set_reg(hisi_crtc, mif_ch_base + MIF_CTRL4,
		s_mif->mif_ctrl4, 32, 0);
	hisi_set_reg(hisi_crtc, mif_ch_base + MIF_CTRL5,
		s_mif->mif_ctrl5, 32, 0);
}
#endif

/*lint -e568 -e685 -e679 -e613 -e838*/
static void hisi_dss_smmu_ch_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *smmu_base, struct dss_smmu *s_smmu, int chn_idx)
{
	uint32_t idx = 0;
	uint32_t i = 0;

	drm_check_and_void_return(smmu_base == NULL, "smmu_base is NULL");

	for (i = 0; i < g_dss_chn_sid_num[chn_idx]; i++) {
		idx = g_dss_smmu_smrx_idx[chn_idx] + i;
		if ((idx < 0) || (idx >= SMMU_SID_NUM)) {
			HISI_DRM_ERR("idx is invalid");
			return;
		}
		hisi_set_reg(hisi_crtc, smmu_base + SMMU_SMRx_NS + idx * 0x4,
		s_smmu->smmu_smrx_ns[idx], 32, 0);
	}
}

static void hisi_dss_mctl_sys_ch_set_reg(struct hisi_drm_crtc *hisi_crtc,
		struct dss_mctl_ch_base *mctl_ch_base, struct dss_mctl_ch *s_mctl_ch)
{
	if (mctl_ch_base->chn_ov_en_base)
		hisi_set_reg(hisi_crtc, mctl_ch_base->chn_ov_en_base,
			s_mctl_ch->chn_ov_oen, 32, 0);

	hisi_set_reg(hisi_crtc, mctl_ch_base->chn_flush_en_base,
		s_mctl_ch->chn_flush_en, 32, 0);
}

int hisi_dss_ch_module_set_regs(struct hisi_drm_crtc *hisi_crtc, uint32_t chn_idx)
{
	uint32_t i = chn_idx;
	int ret;
	uint32_t tmp;
	struct hisi_cmdlist *cmdlist_info = NULL;
	struct dss_module_reg *dss_module = NULL;

	drm_check_and_return(hisi_crtc == NULL, -EINVAL, ERR, "hisi_crtc is NULL");

	cmdlist_info = &hisi_crtc->cmdlist_info;
	dss_module = &hisi_crtc->dss_module;

	if (hisi_crtc->enable_cmdlist) {
		tmp = get_cmdlist_idx(cmdlist_info, chn_idx);
		ret = hisi_cmdlist_create_node(cmdlist_info, tmp);
		drm_check_and_return(ret != 0, ret, ERR, "hisi_cmdlist_create_node err");
	}

	if (dss_module->mctl_ch_used[i] == 1)
		hisi_dss_mctl_ch_set_reg(hisi_crtc, &(dss_module->mctl_ch_base[i]),
			&(dss_module->mctl_ch[i]), hisi_crtc->mclt_idx);

	if (dss_module->smmu_used == 1) {
#ifdef CONFIG_HISI_DSS_V600
		hisi_drm_smmu_ch_set_reg(hisi_crtc, dss_module->smmu_base,
			&(dss_module->smmu), i);
#else
		hisi_dss_smmu_ch_set_reg(hisi_crtc, dss_module->smmu_base,
			&(dss_module->smmu), i);
#endif
	}

	if (dss_module->dma_used[i] == 1)
		hisi_dss_rdma_set_reg(hisi_crtc, dss_module->dma_base[i],
			&(dss_module->rdma[i]), i);
#ifdef CONFIG_HISI_DSS_V510
	if (dss_module->aif1_ch_used[i] == 1)
		hisi_dss_aif_ch_set_reg(hisi_crtc, dss_module->aif1_ch_base[i],
			&(dss_module->aif1[i]));
#endif
	if (dss_module->mif_used[i] == 1)
		hisi_dss_mif_set_reg(hisi_crtc, dss_module->mif_ch_base[i],
			&(dss_module->mif[i]));

	if (dss_module->dfc_used[i] == 1)
		hisi_dss_dfc_set_reg(hisi_crtc, dss_module->dfc_base[i], &(dss_module->dfc[i]));

	if (dss_module->scl_used[i] == 1)
		hisi_dss_scl_set_reg(hisi_crtc, dss_module->scl_base[i], &(dss_module->scl[i]));
#ifdef CONFIG_HISI_DSS_V510
	if (dss_module->arsr2p_used[i] == 1)
		hisi_dss_arsr2p_set_reg(hisi_crtc, dss_module->arsr2p_base[i],
			&(dss_module->arsr2p[i]));
#endif
	if (dss_module->mctl_ch_used[i] == 1)
		hisi_dss_mctl_sys_ch_set_reg(hisi_crtc,
			&(dss_module->mctl_ch_base[i]), &(dss_module->mctl_ch[i]));
	return 0;
}

void hisi_dss_aif_ch_set_reg(struct hisi_drm_crtc *hisi_crtc, char __iomem *aif_ch_base,
	dss_aif_t *s_aif)
{
	drm_check_and_void_return(aif_ch_base == NULL, "aif_ch_base is NULL");
	drm_check_and_void_return(s_aif == NULL, "s_aif is NULL");

	hisi_set_reg(hisi_crtc, aif_ch_base + AIF_CH_CTL,
		s_aif->aif_ch_ctl, 32, 0);
	hisi_set_reg(hisi_crtc, aif_ch_base + AIF_CH_HS,
		s_aif->aif_ch_hs, 32, 0);
	hisi_set_reg(hisi_crtc, aif_ch_base + AIF_CH_LS,
		s_aif->aif_ch_ls, 32, 0);
}

void hisi_dss_disable_channel(struct hisi_drm_crtc *hisi_crtc, int chn_idx)
{
	int ret;
	int tmp;
	bool enable_cmdlist = false;
	struct dss_module_reg *dss_module = NULL;
	struct hisi_cmdlist *cmdlist_info = NULL;

	drm_check_and_void_return(hisi_crtc == NULL, "hisi_crtc is NULL");

	enable_cmdlist = hisi_crtc->enable_cmdlist;
	cmdlist_info = &hisi_crtc->cmdlist_info;

	if (chn_idx < 0 || chn_idx >= DSS_ONLINE_CHN_MAX) {
		HISI_DRM_ERR("invalid chn_idx, chn_idx = %d\n", chn_idx);
		return;
	}

	dss_module = &(hisi_crtc->dss_module);

	HISI_DRM_DEBUG("chn_idx=%d\n", chn_idx);

	if (enable_cmdlist) {
		tmp = get_cmdlist_idx(cmdlist_info, chn_idx);
		ret = hisi_cmdlist_create_node(cmdlist_info, tmp);
		if (ret) {
			HISI_DRM_ERR("hisi_cmdlist_create_node err, ret = %d\n", ret);
			return;
		}
	}

	hisi_dss_chn_set_reg_default_value(hisi_crtc, dss_module->dma_base[chn_idx]);

#ifdef CONFIG_HISI_DSS_V600
	hisi_drm_smmu_ch_set_reg(hisi_crtc, dss_module->smmu_base,
		&(dss_module->smmu), chn_idx);
#else
	hisi_dss_smmu_ch_set_reg(hisi_crtc, dss_module->smmu_base,
		&(dss_module->smmu), chn_idx);
#endif

	hisi_dss_mif_set_reg(hisi_crtc, dss_module->mif_ch_base[chn_idx],
		&(dss_module->mif[chn_idx]));
	hisi_dss_aif_ch_set_reg(hisi_crtc, dss_module->aif_ch_base[chn_idx],
		&(dss_module->aif[chn_idx]));

	dss_module->mctl_ch[chn_idx].chn_mutex =
		set_bits32(dss_module->mctl_ch[chn_idx].chn_mutex, 1, 1, 0);
	dss_module->mctl_ch[chn_idx].chn_flush_en =
		set_bits32(dss_module->mctl_ch[chn_idx].chn_flush_en, 1, 1, 0);
	dss_module->mctl_ch[chn_idx].chn_ov_oen =
		set_bits32(dss_module->mctl_ch[chn_idx].chn_ov_oen, 0, 32, 0);
	dss_module->mctl_ch_used[chn_idx] = 1;

	hisi_dss_mctl_sys_ch_set_reg(hisi_crtc, &(dss_module->mctl_ch_base[chn_idx]),
		&(dss_module->mctl_ch[chn_idx]));
}
