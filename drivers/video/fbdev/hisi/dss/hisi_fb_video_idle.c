/* Copyright (c) 2017-2018, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
/*lint -e679 -e429*/

#include "hisi_fb.h"
#include "hisi_fb_video_idle.h"

#define ONLINE_WB_TIMEOUT_COUNT (16600)
#define DSS_CLEAR_TIMEOUT  (1000)
static void hisifb_video_idle_wb_err_clear(struct hisi_fb_data_type *hisifd)
{
	char __iomem *pipe_sw_wb_base = NULL;
	char __iomem *mctl_sys_base = NULL;
	uint32_t wch0_status = 0;
	uint32_t wback_status = 0;
	uint32_t delay_count = 0;

	/*pipe sw config*/
	pipe_sw_wb_base = hisifd->dss_base + DSS_PIPE_SW_WB_OFFSET;
	set_reg(pipe_sw_wb_base + PIPE_SW_SIG_CTRL, 0x0, 32, 0);
	set_reg(pipe_sw_wb_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
	set_reg(pipe_sw_wb_base + PIPE_SW_DAT_CTRL, 0x0, 32, 0);
	set_reg(pipe_sw_wb_base + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);

	mctl_sys_base = hisifd->dss_base + DSS_MCTRL_SYS_OFFSET;
	set_reg(mctl_sys_base + MCTL_MOD_CLK_SEL, 0x1, 1, 14);

	set_reg(mctl_sys_base + MCTL_MOD14_DBG, 0x3, 2, 22); // wback_debug[23:22] =2'b11
	set_reg(mctl_sys_base + MCTL_MOD8_DBG, 0x3, 2, 22);  // wch0_debug[23:22] =2'b11

	while (delay_count++ < DSS_CLEAR_TIMEOUT) {
		wch0_status = inp32(mctl_sys_base + MCTL_MOD8_STATUS);
		wback_status = inp32(mctl_sys_base + MCTL_MOD14_STATUS);
		if ((wch0_status & BIT(5)) && (wback_status & BIT(5))) {
			set_reg(mctl_sys_base + MCTL_MOD8_DBG, 0x0, 2, 22);  // wch0_debug[23:22] =2'b11
			set_reg(mctl_sys_base + MCTL_MOD14_DBG, 0x0, 2, 22); // wback_debug[23:22] =2'b11
			HISI_FB_INFO("wb err clear succ! +++++++\n");
			break;
		}
		udelay(1);
	}

	if (delay_count == DSS_CLEAR_TIMEOUT) {
		HISI_FB_ERR("wb err clear timeout!\n");
	}

	{
		char *envp[2];
		char buf[64];
		snprintf(buf, sizeof(buf), "Refresh=1");
		envp[0] = buf;
		envp[1] = NULL;
		kobject_uevent_env(&(hisifd->fbi->dev->kobj), KOBJ_CHANGE, envp);
	}

	return;
}

static void hisifb_video_idle_init(struct hisi_fb_data_type *hisifd)
{
	int bpp = 0;
	dss_rect_t rect;
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;

	video_idle_ctrl = &(hisifd->video_idle_ctrl);
	video_idle_ctrl->hisifd = hisifd;
	video_idle_ctrl->afbc_enable = false;
	video_idle_ctrl->mmu_enable = false;
	video_idle_ctrl->compress_enable = true;

	video_idle_ctrl->buffer_alloced = false;
	video_idle_ctrl->video_idle_wb_status = false;
	video_idle_ctrl->video_idle_rb_status = false;
	video_idle_ctrl->rb_closed_reg.need_recovery = false;

	video_idle_ctrl->rch_idx = DSS_RCHN_D2;
	video_idle_ctrl->wch_idx = DSS_WCHN_W0;
	video_idle_ctrl->wdma_format = DMA_PIXEL_FORMAT_ARGB_8888;
	video_idle_ctrl->ovl_idx = DSS_OVL0;
	video_idle_ctrl->l3cache_size = 0x300000;

	rect.x = 0;
	rect.y = 0;
	rect.w = hisifd->panel_info.xres;
	rect.h = hisifd->panel_info.yres;
	if (video_idle_ctrl->wdma_format == DMA_PIXEL_FORMAT_RGB_565) {
		bpp = 2;
	} else if (video_idle_ctrl->wdma_format == DMA_PIXEL_FORMAT_ARGB_8888) {
		bpp = 4;
	} else {
		bpp = 4;
	}
	video_idle_ctrl->wb_buffer_size = ALIGN_UP(rect.w * bpp, DMA_STRIDE_ALIGN) * rect.h;

	mipi_ifbc_get_rect(hisifd, &rect);
	video_idle_ctrl->wb_hsize = ALIGN_UP(rect.w, 8);
	video_idle_ctrl->wb_pad_num = ALIGN_UP(rect.w, 8) - rect.w;

	if (video_idle_ctrl->compress_enable) {
		video_idle_ctrl->wb_pack_hsize = ALIGN_UP(rect.w, 8) * 3 / 4;
	} else {
		video_idle_ctrl->wb_pack_hsize = ALIGN_UP(rect.w, 8);
	}

	video_idle_ctrl->wdfc_pad_hsize = ALIGN_UP(video_idle_ctrl->wb_pack_hsize, 4);
	video_idle_ctrl->wdfc_pad_num = video_idle_ctrl->wdfc_pad_hsize - video_idle_ctrl->wb_pack_hsize;
	video_idle_ctrl->wb_vsize = rect.h;

	return;
}

static void hisifb_video_idle_interrupt_config(struct hisi_fb_data_type *hisifd)
{
	uint32_t isr_s1 = 0;
	char __iomem *dss_base = 0;

	dss_base = hisifd->dss_base;
	isr_s1 = BIT_DSS_WB_ERR_INTS;
	outp32(dss_base + GLB_CPU_PDP_INTS, isr_s1);

	isr_s1 = BIT_OFF_WCH0_INTS;
	outp32(dss_base + GLB_CPU_OFF_INTS, isr_s1);

	isr_s1 = BIT_WB_ONLINE_ERR_INTS;
	outp32(dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS, isr_s1);

}

static void hisifb_video_idle_buffer_alloc(struct hisi_fb_data_type *hisifd, uint32_t req_size)
{
	struct ion_client *client = NULL;
	struct ion_handle *handle = NULL; //lint !e429
	size_t buf_len = 0;
	unsigned long buf_addr = 0;
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;
	video_idle_ctrl = &(hisifd->video_idle_ctrl);

	if (!(hisifd->pdev)) {
		HISI_FB_ERR("fb%d pdev is NULL!\n", hisifd->index);
		return;
	}

	if (!video_idle_ctrl->buffer_alloced) {
		buf_len = roundup(video_idle_ctrl->wb_buffer_size, PAGE_SIZE);

		client = hisifd->ion_client;
		if (IS_ERR_OR_NULL(client)) {
			HISI_FB_ERR("fb%d failed to create ion client!\n", hisifd->index);
			return;
		}

		handle = ion_alloc(client, buf_len, PAGE_SIZE, ION_HEAP(ION_GRALLOC_HEAP_ID), 0);
		if (IS_ERR_OR_NULL(handle)) {
			HISI_FB_ERR("fb%d failed to ion_alloc!\n", hisifd->index);
			return;
		}

		video_idle_ctrl->wb_buffer_base = ion_map_kernel(client, handle);
		if (!video_idle_ctrl->wb_buffer_base) {
			HISI_FB_ERR("fb%d failed to ion_map_kernel!\n", hisifd->index);
			ion_free(client, handle); //lint !e668
			return;
		}
		if (hisifb_ion_phys(client, handle, &(hisifd->pdev->dev), &(buf_addr), &buf_len)) {
			HISI_FB_ERR("fb%d failed to get ion phys!\n", hisifd->index);
			ion_unmap_kernel(hisifd->ion_client, video_idle_ctrl->wb_handle);
			ion_free(client, handle); //lint !e668
			return;
		}

		//video_idle_ctrl->wb_vir_addr = hisi_iommu_map_l3(buf_addr, buf_len);
		video_idle_ctrl->wb_phys_addr = buf_addr;
		video_idle_ctrl->wb_buffer_size = buf_len;
		video_idle_ctrl->wb_handle = handle;

		video_idle_ctrl->request_params.id = DSS_IDLE;
		video_idle_ctrl->request_params.request_size = req_size / 0x100000; // 3M
		video_idle_ctrl->request_params.type = PRIVATE;
		if (l3_cache_request(&(video_idle_ctrl->request_params))) {
			HISI_FB_ERR("request l3 cache failed.\n");
			//ion_unmap_iommu(hisifd->ion_client, wb_ctrl->wb_handle);
			ion_unmap_kernel(hisifd->ion_client, video_idle_ctrl->wb_handle);
			ion_free(hisifd->ion_client, video_idle_ctrl->wb_handle);

			video_idle_ctrl->wb_handle = NULL;
			video_idle_ctrl->wb_buffer_base = 0;
			video_idle_ctrl->wb_phys_addr = 0;
			return;
		}
		video_idle_ctrl->buffer_alloced = true;

		if (video_idle_ctrl->wb_vir_addr) {
			video_idle_ctrl->mmu_enable = true;
			HISI_FB_INFO("fb%d alloc buffer and L3cache, smmu enable = %d! \n",
				hisifd->index, video_idle_ctrl->mmu_enable);
		}
	}

	return;
}

void hisifb_video_idle_buffer_free(struct hisi_fb_data_type *hisifd)
{
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL \n");
		return;
	}

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		return;
	}

	if (!is_video_idle_ctrl_mode(hisifd)) {
		return;
	}

	video_idle_ctrl = &(hisifd->video_idle_ctrl);
	if (!video_idle_ctrl->buffer_alloced) {
		HISI_FB_DEBUG("fb%d buffer already free.\n", hisifd->index);
		return;
	}

	if ((hisifd->ion_client != NULL) && (video_idle_ctrl->wb_handle != NULL)) {
		//ion_unmap_iommu(hisifd->ion_client, wb_ctrl->wb_handle);
		ion_unmap_kernel(hisifd->ion_client, video_idle_ctrl->wb_handle);
		ion_free(hisifd->ion_client, video_idle_ctrl->wb_handle);

		video_idle_ctrl->wb_handle = NULL;
		video_idle_ctrl->wb_buffer_base = 0;
		video_idle_ctrl->wb_phys_addr = 0;
		video_idle_ctrl->wb_vir_addr = 0;
	}

	video_idle_ctrl->release_params.id = DSS_IDLE;
	l3_cache_release(&(video_idle_ctrl->release_params)); //release l3cache
	HISI_FB_INFO("fb%d free buffer and relase L3cache! \n", hisifd->index);

	video_idle_ctrl->buffer_alloced = false;
}

static void hisifb_video_idle_wb_pipe_set_reg(struct hisi_fb_data_type *hisifd)
{
	char __iomem *wb_base = NULL;
	char __iomem *pipe_sw_base = NULL;
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;
	video_idle_ctrl = &(hisifd->video_idle_ctrl);

	/*step1. pipe sw config*/
	pipe_sw_base = hisifd->dss_base + DSS_PIPE_SW_WB_OFFSET;
	hisifd->set_reg(hisifd, pipe_sw_base + PIPE_SW_SIG_CTRL, 0x1, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_base + PIPE_SW_DAT_CTRL, 0x1, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_base + SW_POS_CTRL_DAT_EN, 0x1, 32, 0);

	hisifd->set_reg(hisifd, pipe_sw_base + NXT_SW_NO_PR, 0x1, 32, 0);

	/*step2. wb pack config*/
	wb_base = hisifd->dss_base + DSS_WB_OFFSET;
	if (video_idle_ctrl->compress_enable) {
		hisifd->set_reg(hisifd, wb_base + WB_CTRL, 0x3, 32, 0);
	} else {
		hisifd->set_reg(hisifd, wb_base + WB_CTRL, 0x2, 32, 0);
	}
	hisifd->set_reg(hisifd, wb_base + WB_WORK_MODE, 0x1, 32, 0);

	hisifd->set_reg(hisifd, wb_base + WB_IMG_HSIZE, DSS_WIDTH(video_idle_ctrl->wb_hsize), 32, 0);
	hisifd->set_reg(hisifd, wb_base + WB_IMG_VSIZE, DSS_HEIGHT(video_idle_ctrl->wb_vsize), 32, 0);
	hisifd->set_reg(hisifd, wb_base + WB_PAD_NUM, video_idle_ctrl->wb_pad_num, 32, 0);
}

static void hisifb_video_idle_wch_config(struct hisi_fb_data_type *hisifd)
{
	uint32_t i, idx;
	uint32_t chn_idx = 0;
	dss_dfc_t *s_dfc = NULL;
	dss_wdma_t *s_wdma = NULL;
	dss_smmu_t *s_smmu = NULL;
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;
	video_idle_ctrl = &(hisifd->video_idle_ctrl);
	chn_idx= video_idle_ctrl->wch_idx;

	s_smmu = &(hisifd->dss_module.smmu);
	s_dfc = &(hisifd->dss_module.dfc[chn_idx]);
	s_wdma = &(hisifd->dss_module.wdma[chn_idx]);

	s_dfc->padding_ctl = set_bits32(s_dfc->padding_ctl, BIT(31) | (video_idle_ctrl->wdfc_pad_num << 8), 32, 0); //lint !e647 !e648
	s_dfc->disp_size = set_bits32(s_dfc->disp_size,
		(DSS_WIDTH(video_idle_ctrl->wb_pack_hsize) << 16) | DSS_HEIGHT(video_idle_ctrl->wb_vsize), 32, 0);

	s_wdma->oft_x0 = set_bits32(s_wdma->oft_x0, 0, 12, 0);
	s_wdma->oft_y0 = set_bits32(s_wdma->oft_y0, 0, 16, 0);
	s_wdma->oft_x1 = set_bits32(s_wdma->oft_x1, DSS_WIDTH(video_idle_ctrl->wdfc_pad_hsize) / 4, 12, 0);
	s_wdma->oft_y1 = set_bits32(s_wdma->oft_y1, DSS_HEIGHT(video_idle_ctrl->wb_vsize), 16, 0);

	s_wdma->ctrl = set_bits32(s_wdma->ctrl, (video_idle_ctrl->mmu_enable ? 0x1 : 0x0), 1, 8);
	s_wdma->ctrl = set_bits32(s_wdma->ctrl, video_idle_ctrl->wdma_format, 5, 3);

	if (video_idle_ctrl->mmu_enable) {
		s_wdma->data_addr = set_bits32(s_wdma->data_addr, video_idle_ctrl->wb_vir_addr, 32, 0);
	} else {
		s_wdma->data_addr = set_bits32(s_wdma->data_addr, video_idle_ctrl->wb_phys_addr, 32, 0);
	}

	s_wdma->stride0 = set_bits32(s_wdma->stride0, video_idle_ctrl->wdfc_pad_hsize / 4, 13, 0);
	s_wdma->ch_ctl = set_bits32(s_wdma->ch_ctl, 0x1, 32, 0);

	s_smmu->smmu_smrx_ns_used[chn_idx] = 1;
	for (i = 0; i < g_dss_chn_sid_num[chn_idx]; i++) {
		idx = g_dss_smmu_smrx_idx[chn_idx] + i;
		if (idx >= SMMU_SID_NUM) {
			HISI_FB_ERR("idx is invalid");
			return ;
		}

		if (video_idle_ctrl->mmu_enable) {
			s_smmu->smmu_smrx_ns[idx] = set_bits32(s_smmu->smmu_smrx_ns[idx], 0x0, 1, 0);  //smr_bypass
			s_smmu->smmu_smrx_ns[idx] = set_bits32(s_smmu->smmu_smrx_ns[idx], 0x1, 1, 4);  //smr_invld_en
		} else {
			s_smmu->smmu_smrx_ns[idx] = set_bits32(s_smmu->smmu_smrx_ns[idx], 0x1, 1, 0);
		}
	}
	return;
}

static void hisifb_video_idle_wch_set_reg(struct hisi_fb_data_type *hisifd)
{
	uint32_t ovl_idx = 0;
	uint32_t chn_idx = 0;
	char __iomem *mctl_ov_ien = NULL;
	char __iomem *mctl_chn_flush_en = NULL;
	char __iomem *mctl_mutex_base = NULL;
	char __iomem *wdma_base = NULL;
	char __iomem *dfc_base = NULL;
	dss_wdma_t *s_wdma = NULL;
	dss_dfc_t *s_dfc = NULL;
	dss_smmu_t *s_smmu = NULL;
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;
	video_idle_ctrl = &(hisifd->video_idle_ctrl);
	chn_idx = video_idle_ctrl->wch_idx;
	ovl_idx = video_idle_ctrl->ovl_idx;

	/* step1. mctl config*/
	mctl_mutex_base = hisifd->dss_base + g_dss_module_ovl_base[ovl_idx][MODULE_MCTL_BASE];
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_MUTEX_WB, 0x1, 32, 0);
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_MUTEX_WCH0, 0x1, 32, 0);

	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_END_SEL, 0xffffbeff, 32, 0);

	mctl_ov_ien = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_OV_OEN];
	hisifd->set_reg(hisifd, mctl_ov_ien, 0x3, 32, 0);// mctl_sys 0x1A0

	/* step2. wch0 config*/
	s_wdma = &(hisifd->dss_module.wdma[chn_idx]);
	wdma_base = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_DMA];
	hisifd->set_reg(hisifd, wdma_base + CH_REG_DEFAULT, 0x1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + CH_REG_DEFAULT, 0x0, 32, 0);

	hisifd->set_reg(hisifd, wdma_base + DMA_OFT_X0, s_wdma->oft_x0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_OFT_Y0, s_wdma->oft_y0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_OFT_X1, s_wdma->oft_x1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_OFT_Y1, s_wdma->oft_y1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_CTRL, s_wdma->ctrl, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_DATA_ADDR0, s_wdma->data_addr, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_STRIDE0, s_wdma->stride0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + CH_CTL, s_wdma->ch_ctl, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_BUF_SIZE,
		(s_wdma->oft_y1 << 16) | DSS_WIDTH(video_idle_ctrl->wdfc_pad_hsize), 32, 0);
	hisifd->set_reg(hisifd, wdma_base + ROT_SIZE,
		(s_wdma->oft_y1 << 16) | DSS_WIDTH(video_idle_ctrl->wdfc_pad_hsize), 32, 0);

	/* step3. wch0-dfc config*/
	s_dfc = &(hisifd->dss_module.dfc[chn_idx]);
	dfc_base = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_DFC];
	hisifd->set_reg(hisifd, dfc_base + DFC_PIX_IN_NUM, 0x0, 32, 0);
	hisifd->set_reg(hisifd, dfc_base + DFC_PADDING_CTL, s_dfc->padding_ctl, 32, 0);
	hisifd->set_reg(hisifd, dfc_base + DFC_DISP_SIZE, s_dfc->disp_size, 32, 0);
	hisifd->set_reg(hisifd, dfc_base + DFC_DISP_FMT, (DFC_PIXEL_FORMAT_ARGB_8888 << 1), 32, 0);
	hisifd->set_reg(hisifd, dfc_base + DFC_ICG_MODULE, 0x1, 32, 0); //enable clock

	hisifd->set_reg(hisifd, hisifd->dss_base +
		g_dss_module_base[chn_idx][MODULE_AIF0_CHN], AXI_CHN0, 1, 0);

	if (video_idle_ctrl->mmu_enable) {
		hisifd->set_reg(hisifd, hisifd->dss_base +
			g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL1, BIT(17), 32, 0);
	} else {
		hisifd->set_reg(hisifd, hisifd->dss_base +
			g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL1, BIT(17) | BIT(5), 32, 0);
	}

	/* step4. smmu config*/
	s_smmu = &(hisifd->dss_module.smmu);
	hisi_dss_smmu_ch_set_reg(hisifd, hisifd->dss_module.smmu_base, s_smmu, chn_idx); // 0x1d

	/* step5. mctl sys config*/
	mctl_chn_flush_en = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_FLUSH_EN];
	hisifd->set_reg(hisifd, mctl_chn_flush_en, 0x1, 32, 0);
}

static void hisifb_video_idle_config_dpp_regs(struct hisi_fb_data_type *hisifd)
{
	uint32_t dsc_en = 0;
	uint32_t dpp_clip_en = 0;
	uint32_t dither_ctl0 = 0;
	uint32_t gama_en = 0;
	uint32_t xcc_en = 0;
	uint32_t degama_en = 0;
	uint32_t gmp_en = 0;
	uint32_t arsr_post_bypass = 0;
	uint32_t ifbc_en = 0;
	uint32_t hiace_bypass = 0;
	uint32_t nr_bypass = 0;
	uint32_t sbl_en = 0;
	char __iomem *dss_base = NULL;
	struct hisifb_video_idle_ctrl *vic = NULL;
	vic = &(hisifd->video_idle_ctrl);

	dss_base = hisifd->dss_base;
	dsc_en = inp32(dss_base + DSS_DSC_OFFSET + DSC_EN); //0x7dc08
	dpp_clip_en = inp32(dss_base + DSS_DPP_OFFSET + DPP_CLIP_EN); //0x70190
	dither_ctl0 = inp32(dss_base + DSS_DPP_DITHER_OFFSET + DITHER_CTL0); //0x70204
	gama_en = inp32(dss_base + DSS_DPP_GAMA_OFFSET + GAMA_EN); //0x70600
	xcc_en = inp32(dss_base + DSS_DPP_XCC_OFFSET + XCC_EN); //0x70934
	degama_en = inp32(dss_base + DSS_DPP_DEGAMMA_OFFSET + DEGAMA_EN); //0x70950
	gmp_en = inp32(dss_base + DSS_DPP_GMP_OFFSET + GMP_EN); //0x709A0
	arsr_post_bypass = inp32(dss_base + DSS_DPP_ARSR_POST_OFFSET + ARSR_POST_MODE); //0x70A24
	ifbc_en  = inp32(dss_base + DSS_IFBC_OFFSET + IFBC_EN);  //0x7d818
	hiace_bypass = inp32(dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_ACE); //0x6f008
	nr_bypass = inp32(dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_NR); //0x6f400
	sbl_en = inp32(dss_base + DSS_DPP_OFFSET + DPP_SBL); //0x70014

	vic->rb_closed_reg.dsc_en = dsc_en;
	vic->rb_closed_reg.dpp_clip_en = dpp_clip_en;
	vic->rb_closed_reg.dither_ctl0 = dither_ctl0;
	vic->rb_closed_reg.gama_en = gama_en;
	vic->rb_closed_reg.xcc_en = xcc_en;
	vic->rb_closed_reg.degama_en = degama_en;
	vic->rb_closed_reg.gmp_en = gmp_en;
	vic->rb_closed_reg.arsr_post_bypass = arsr_post_bypass;
	vic->rb_closed_reg.ifbc_en = ifbc_en;
	vic->rb_closed_reg.hiace_bypass = hiace_bypass;
	vic->rb_closed_reg.nr_bypass = nr_bypass;
	vic->rb_closed_reg.sbl_en = sbl_en;

	HISI_FB_DEBUG("fb%d, dsc_en[0x7dc08]=0x%x, dpp_clip_en[0x70190]=0x%x, dither_ctl0[0x70204]=0x%x\n"
		"gama_en[0x70600]=0x%x, xcc_en[0x70934]=0x%x, degama_en[0x70950]=0x%x, gmp_en[0x709A0]=0x%x\n"
		"arsr_post_mode[0x70A24]=0x%x, ifbc_en[0x7d818]=0x%x, hiace_bypass[0x6f008]=0x%x, "
		"hiace_bypass[0x6f400]=0x%x, sbl_en[0x70014]=0x%x\n",
		hisifd->index, dsc_en, dpp_clip_en, dither_ctl0, gama_en, xcc_en, degama_en, gmp_en, arsr_post_bypass,
		ifbc_en, hiace_bypass, nr_bypass, sbl_en);

	/*NOTICE: IFBC & DPP, these configed in initial*/
	dsc_en = set_bits32(dsc_en, 0x0, 1, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_DSC_OFFSET + DSC_EN, dsc_en, 32, 0);

	dpp_clip_en = set_bits32(dpp_clip_en, 0x0, 1, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_DPP_OFFSET + DPP_CLIP_EN, dpp_clip_en, 32, 0);

	dither_ctl0 = set_bits32(dither_ctl0, 0x0, 1, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_DPP_DITHER_OFFSET + DITHER_CTL0, dither_ctl0, 32, 0);

	gama_en = set_bits32(gama_en, 0x0, 2, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_DPP_GAMA_OFFSET + GAMA_EN, gama_en, 32, 0);

	xcc_en = set_bits32(xcc_en, 0x0, 2, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_DPP_XCC_OFFSET + XCC_EN, xcc_en, 32, 0);

	degama_en = set_bits32(degama_en, 0x0, 1, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_DPP_DEGAMMA_OFFSET + DEGAMA_EN, degama_en, 32, 0);

	gmp_en = set_bits32(gmp_en, 0x0, 1, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_DPP_GMP_OFFSET + GMP_EN, gmp_en, 32, 0);

	arsr_post_bypass = set_bits32(arsr_post_bypass, 0x1, 1, 0); //bypass
	hisifd->set_reg(hisifd, dss_base + DSS_DPP_ARSR_POST_OFFSET + ARSR_POST_MODE, arsr_post_bypass, 32, 0);

	ifbc_en = set_bits32(ifbc_en, 0x0, 1, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_IFBC_OFFSET + IFBC_EN, ifbc_en, 32, 0);

	hiace_bypass = set_bits32(hiace_bypass, 0x1, 1, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_ACE, hiace_bypass, 32, 0);

	nr_bypass = set_bits32(nr_bypass, 0x1, 1, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_NR, nr_bypass, 32, 0);

	sbl_en = set_bits32(sbl_en, 0x0, 1, 0); //bypass
	hisifd->set_reg(hisifd, dss_base + DSS_DPP_OFFSET + DPP_SBL, sbl_en, 32, 0);

	vic->rb_closed_reg.need_recovery = true;
}

static void hisifb_video_idle_wb_clear(struct hisi_fb_data_type *hisifd)
{
	char __iomem *pipe_sw_wb_base = NULL;
	char __iomem *mctl_ov_ien = NULL;
	char __iomem *mctl_mutex_base = NULL;
	struct hisifb_video_idle_ctrl *vic = NULL;
	vic = &(hisifd->video_idle_ctrl);
	pipe_sw_wb_base = hisifd->dss_base + DSS_PIPE_SW_WB_OFFSET;
	hisifd->set_reg(hisifd, pipe_sw_wb_base + PIPE_SW_SIG_CTRL, 0x0, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_wb_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_wb_base + PIPE_SW_DAT_CTRL, 0x0, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_wb_base + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_wb_base + NXT_SW_NO_PR, 0x0, 32, 0);

	mctl_mutex_base = hisifd->dss_base + g_dss_module_ovl_base[vic->ovl_idx][MODULE_MCTL_BASE];
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_MUTEX_WB, 0x0, 32, 0);
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_MUTEX_WCH0, 0x0, 32, 0);
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_END_SEL, 0xffffbfff, 32, 0);

	mctl_ov_ien = hisifd->dss_base + g_dss_module_base[vic->wch_idx][MODULE_MCTL_CHN_OV_OEN];
	hisifd->set_reg(hisifd, mctl_ov_ien, 0x0, 32, 0);
}

static void hisifb_video_idle_rch_set_reg(struct hisi_fb_data_type *hisifd)
{
	uint32_t i = 0, smmu_scr_value = 0;
	int ret = 0, idx = 0;
	uint32_t ovl_idx = 0;
	uint32_t chn_idx = 0;
	char __iomem *mctl_mutex_base = NULL;
	char __iomem *mctl_sys_base = NULL;
	char __iomem *rdma_base = NULL;
	char __iomem *rdfc_base = NULL;
	char __iomem *smmu_base = NULL;
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;
	video_idle_ctrl = &(hisifd->video_idle_ctrl);
	ovl_idx = video_idle_ctrl->ovl_idx;
	chn_idx = video_idle_ctrl->rch_idx;

	hisifd->cmdlist_idx = chn_idx;
	ret = hisi_cmdlist_add_new_node(hisifd, 0x1 << chn_idx, 0, 0, 0, 0, 0);
	if (ret) {
		HISI_FB_ERR("fb%d, hisi_cmdlist_add_new_node err: %d \n", hisifd->index, ret);
		return;
	}

	/* step1. aif ar cache */
	hisifd->set_reg(hisifd, hisifd->dss_base +
		g_dss_module_base[chn_idx][MODULE_AIF0_CHN], 0xf000, 32, 0);

	if (video_idle_ctrl->mmu_enable) {
		hisifd->set_reg(hisifd, hisifd->dss_base + DSS_VBIF0_AIF + ARCACH0_ADDR_THRH,
			video_idle_ctrl->wb_vir_addr + video_idle_ctrl->l3cache_size, 32, 0);

		hisifd->set_reg(hisifd, hisifd->dss_base + DSS_VBIF0_AIF + ARCACH0_ADDR_THRL, video_idle_ctrl->wb_vir_addr, 32, 0);
	} else {
		hisifd->set_reg(hisifd, hisifd->dss_base + DSS_VBIF0_AIF + ARCACH0_ADDR_THRH,
			video_idle_ctrl->wb_phys_addr + video_idle_ctrl->l3cache_size, 32, 0);

		hisifd->set_reg(hisifd, hisifd->dss_base + DSS_VBIF0_AIF + ARCACH0_ADDR_THRL, video_idle_ctrl->wb_phys_addr, 32, 0);
	}
	hisifd->set_reg(hisifd, hisifd->dss_base + DSS_VBIF0_AIF + ARCACH0_VALUE0, 0x0, 32, 0);
	hisifd->set_reg(hisifd, hisifd->dss_base + DSS_VBIF0_AIF + ARCACH0_VALUE1, 0xF, 32, 0);

	/* step2. mmu/mif config */
	if (video_idle_ctrl->mmu_enable) {
		hisifd->set_reg(hisifd, hisifd->dss_base +
			g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL1, BIT(19), 32, 0);
		hisifd->set_reg(hisifd, hisifd->dss_base +
			g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL2, 0, 32, 0);
		hisifd->set_reg(hisifd, hisifd->dss_base +
			g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL3, 0, 32, 0);
		hisifd->set_reg(hisifd, hisifd->dss_base +
			g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL4, 0, 32, 0);
		hisifd->set_reg(hisifd, hisifd->dss_base +
			g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL5, 0, 32, 0);

	} else {
		hisifd->set_reg(hisifd, hisifd->dss_base +
			g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL1, 1, 1, 5);
	}

	smmu_base = hisifd->dss_base + DSS_SMMU_OFFSET;
	for (i = 0; i < g_dss_chn_sid_num[chn_idx]; i++) {
		idx = g_dss_smmu_smrx_idx[chn_idx] + i;
		if (idx >= SMMU_SID_NUM) {
			HISI_FB_ERR("idx is invalid");
			return;
		}
		smmu_scr_value = inp32(smmu_base + SMMU_SMRx_NS + idx * 0x4);
		if (video_idle_ctrl->mmu_enable) {
			smmu_scr_value = set_bits32(smmu_scr_value, 0x1, 1, 9);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x1, 2, 7);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x1, 2, 5);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x0, 1, 1);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x1, 1, 4);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x3, 2, 2);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x0, 1, 0);
		} else {
			smmu_scr_value = set_bits32(smmu_scr_value, 0x1, 1, 9);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x0, 2, 7);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x0, 2, 5);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x0, 1, 1);
			smmu_scr_value = set_bits32(smmu_scr_value, 0x1, 1, 0);  //bypass
		}
		hisifd->set_reg(hisifd, smmu_base + SMMU_SMRx_NS + idx * 0x4, smmu_scr_value, 32, 0);
	}

	/* step3. rch dma config */
	rdma_base = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_DMA];
	hisifd->set_reg(hisifd, rdma_base + CH_REG_DEFAULT, 0x1, 32, 0);
	hisifd->set_reg(hisifd, rdma_base + CH_REG_DEFAULT, 0x0, 32, 0);

	hisifd->set_reg(hisifd, rdma_base + DMA_OFT_X0, 0, 32, 0);
	hisifd->set_reg(hisifd, rdma_base + DMA_OFT_Y0, 0, 32, 0);
	hisifd->set_reg(hisifd, rdma_base + DMA_OFT_X1, DSS_WIDTH(video_idle_ctrl->wdfc_pad_hsize) / 4, 32, 0);
	hisifd->set_reg(hisifd, rdma_base + DMA_OFT_Y1, DSS_HEIGHT(video_idle_ctrl->wb_vsize), 32, 0);
	hisifd->set_reg(hisifd, rdma_base + DMA_STRETCH_SIZE_VRT, DSS_HEIGHT(video_idle_ctrl->wb_vsize), 32, 0);
	hisifd->set_reg(hisifd, rdma_base + DMA_CTRL, ((video_idle_ctrl->mmu_enable ? 0x1 : 0x0) << 8) | (video_idle_ctrl->wdma_format << 3), 32, 0);
	if (video_idle_ctrl->mmu_enable) {
		hisifd->set_reg(hisifd, rdma_base + DMA_DATA_ADDR0, video_idle_ctrl->wb_vir_addr, 32, 0);
	} else {
		hisifd->set_reg(hisifd, rdma_base + DMA_DATA_ADDR0, video_idle_ctrl->wb_phys_addr, 32, 0);
	}
	hisifd->set_reg(hisifd, rdma_base + DMA_STRIDE0, video_idle_ctrl->wdfc_pad_hsize / 4, 32, 0);
	hisifd->set_reg(hisifd, rdma_base + CH_CTL, (0xF << 12) | 0x1, 32, 0);
	hisifd->set_reg(hisifd, rdma_base + DMA_BUF_SIZE,
		(DSS_HEIGHT(video_idle_ctrl->wb_vsize) << 16) | DSS_WIDTH(video_idle_ctrl->wdfc_pad_hsize), 32, 0);

	/* step4. rch dfc config */
	rdfc_base = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_DFC];
	hisifd->set_reg(hisifd, rdfc_base + DFC_DISP_SIZE,
		(DSS_WIDTH(video_idle_ctrl->wdfc_pad_hsize) << 16) | DSS_HEIGHT(video_idle_ctrl->wb_vsize), 32, 0);
	hisifd->set_reg(hisifd, rdfc_base + DFC_CLIP_CTL_HRZ, video_idle_ctrl->wdfc_pad_num, 32, 0);
	hisifd->set_reg(hisifd, rdfc_base + DFC_PIX_IN_NUM, 0, 32, 0);
	hisifd->set_reg(hisifd, rdfc_base + DFC_DISP_FMT, DFC_PIXEL_FORMAT_ARGB_8888 << 1, 32, 0);
	hisifd->set_reg(hisifd, rdfc_base + DFC_CTL_CLIP_EN, 0x1, 32, 0);
	hisifd->set_reg(hisifd, rdfc_base + DFC_ICG_MODULE, 0x1, 32, 0);

	/* step5. mctl mutex config */
	mctl_mutex_base = hisifd->dss_base + g_dss_module_ovl_base[ovl_idx][MODULE_MCTL_BASE];
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_MUTEX_RCH0 + chn_idx * 0x4, 0x1, 32, 0);

	/* step6. mctl sys config */
	mctl_sys_base = hisifd->dss_base + DSS_MCTRL_SYS_OFFSET;
	hisifd->set_reg(hisifd, mctl_sys_base + MCTL_RCH0_OV_OEN + chn_idx * 0x4, 0x100, 32, 0);
	hisifd->set_reg(hisifd, mctl_sys_base + MCTL_RCH0_FLUSH_EN + chn_idx * 0x4, 0x1, 32, 0);
}

static void hisifb_video_idle_ovl_set_reg(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	uint32_t ovl_idx = 0;
	uint32_t chn_idx = 0;
	char __iomem *ovl_base = NULL;
	char __iomem *mctl_sys_base = NULL;
	char __iomem *dbuf_base = NULL;
	char __iomem *dss_base = NULL;
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;
	video_idle_ctrl = &(hisifd->video_idle_ctrl);
	ovl_idx = video_idle_ctrl->ovl_idx;
	chn_idx = video_idle_ctrl->rch_idx;
	dss_base = hisifd->dss_base;

	hisifd->cmdlist_idx = DSS_CMDLIST_OV0;
	ret = hisi_cmdlist_add_new_node(hisifd, 0x1 << DSS_CMDLIST_OV0, 0, 0, 0, 0, 0);
	if (ret) {
		HISI_FB_ERR("fb%d, hisi_cmdlist_add_new_node err:%d \n", hisifd->index, ret);
		return;
	}
	/* step1. pipe sw disconnect */
	hisifb_video_idle_wb_clear(hisifd);

	/* step2. ov config */
	ovl_base = dss_base + g_dss_module_ovl_base[ovl_idx][MODULE_OVL_BASE];
	hisifd->set_reg(hisifd, ovl_base + OV8_REG_DEFAULT, 0x1, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV8_REG_DEFAULT, 0x0, 32, 0);

	hisifd->set_reg(hisifd, ovl_base + OV_SIZE,
		(DSS_HEIGHT(video_idle_ctrl->wb_vsize) << 16) | DSS_WIDTH(video_idle_ctrl->wb_pack_hsize), 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_BG_COLOR_RGB, 0x0, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_BG_COLOR_A, 0x0, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_DST_STARTPOS, 0x0, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_DST_ENDPOS,
		(DSS_HEIGHT(video_idle_ctrl->wb_vsize) << 16) | DSS_WIDTH(video_idle_ctrl->wb_pack_hsize), 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_GCFG, 0x10001, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_POS, 0x0, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_SIZE,
		(DSS_HEIGHT(video_idle_ctrl->wb_vsize) << 16) | DSS_WIDTH(video_idle_ctrl->wb_pack_hsize), 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_ALPHA_MODE, 0x4000, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_ALPHA_A, 0x3ff03ff, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_CFG, 0x1, 32, 0);

	/* step3. dbuf unpack & post clip */
	dbuf_base = dss_base + DSS_DBUF0_OFFSET;
	hisifd->set_reg(hisifd, dbuf_base + DBUF_POST_CLIP_DISP_SIZE,
		DSS_WIDTH(video_idle_ctrl->wb_hsize) << 16 | DSS_HEIGHT(video_idle_ctrl->wb_vsize), 32, 0);
	hisifd->set_reg(hisifd, dbuf_base + DBUF_POST_CLIP_CTL_HRZ, video_idle_ctrl->wb_pad_num, 32, 0);
	if (video_idle_ctrl->compress_enable) {
		hisifd->set_reg(hisifd, dbuf_base + DBUF_UNPACK_IMG_FMT, 0x1, 32, 0);
	} else {
		hisifd->set_reg(hisifd, dbuf_base + DBUF_UNPACK_IMG_FMT, 0x0, 32, 0);
	}
	hisifd->set_reg(hisifd, dbuf_base + DBUF_WB_UNPACK_EN, 0x1, 32, 0);
	hisifd->set_reg(hisifd, dbuf_base + DBUF_WB_UNPACK_SIZE,
		DSS_WIDTH(video_idle_ctrl->wb_pack_hsize) << 16 | DSS_HEIGHT(video_idle_ctrl->wb_vsize), 32, 0);

	/* step4. dsc bypass & dpp bypass */
	hisifb_video_idle_config_dpp_regs(hisifd);

	/* step5. mctl sys flush en */
	mctl_sys_base = dss_base + DSS_MCTRL_SYS_OFFSET;
	hisifd->set_reg(hisifd, mctl_sys_base + MCTL_RCH_OV0_SEL, 0xFFFFFF0E, 32, 0); // DSS_RCHN_D2
	hisifd->set_reg(hisifd, mctl_sys_base + MCTL_OV0_FLUSH_EN, 0xD, 32, 0);
}

static void hisifb_video_idle_cmdlist_init(struct hisi_fb_data_type *hisifd,
	uint32_t *cmdlist_pre_idxs, uint32_t *cmdlist_idxs)
{
	uint32_t cmdlist_idxs_temp = 0;
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;
	video_idle_ctrl = &(hisifd->video_idle_ctrl);

	cmdlist_idxs_temp |= (1 << (video_idle_ctrl->ovl_idx + DSS_CMDLIST_OV0));
	cmdlist_idxs_temp |= (1 << video_idle_ctrl->rch_idx);
	*cmdlist_idxs = cmdlist_idxs_temp;
	*cmdlist_pre_idxs &= (~ (*cmdlist_idxs));
}


static int hisifb_video_idle_rb_config(struct hisi_fb_data_type *hisifd)
{
	int enable_cmdlist = 0;
	int ret = 0, count = 0, time = 16000;
	uint32_t isr_wch = 0, isr_wb = 0;
	uint32_t cmdlist_pre_idxs = 0;
	uint32_t cmdlist_idxs = 0;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	dss_overlay_t *pov_req_prev = NULL;
	struct hisifb_video_idle_ctrl *vic = NULL;
	vic = &(hisifd->video_idle_ctrl);

	/* step1. wait wb start */
	ret = hisi_vactive0_start_config(hisifd, &(hisifd->ov_req_prev));
	if (ret != 0) {
		HISI_FB_ERR("fb%d, hisi_vactive0_start_config failed! ret = %d\n", hisifd->index, ret);
		hisifb_video_idle_wb_err_clear(hisifd);
		return -1;
	}
	disable_ldi(hisifd);

	enable_cmdlist = g_enable_ovl_cmdlist_online;
	pov_req_prev = &(hisifd->ov_req_prev);
	if (enable_cmdlist) {
		hisifd->set_reg = hisi_cmdlist_set_reg;

		hisi_cmdlist_data_get_online(hisifd);

		hisi_cmdlist_get_cmdlist_idxs(pov_req_prev, &cmdlist_pre_idxs, NULL);

		hisifb_video_idle_cmdlist_init(hisifd, &cmdlist_pre_idxs, &cmdlist_idxs);

		hisi_cmdlist_add_nop_node(hisifd, cmdlist_pre_idxs, 0, 0);
		hisi_cmdlist_add_nop_node(hisifd, cmdlist_idxs, 0, 0);
	} else {
		hisifd->set_reg = hisifb_set_reg;
		hisi_dss_mctl_mutex_lock(hisifd, vic->ovl_idx);
		cmdlist_pre_idxs = ~0;
	}
	hisi_dss_prev_module_set_regs(hisifd, pov_req_prev, cmdlist_pre_idxs, enable_cmdlist, NULL);
	(void)hisi_dss_module_init(hisifd);

	hisifb_video_idle_rch_set_reg(hisifd);
	hisifb_video_idle_ovl_set_reg(hisifd);

	if (enable_cmdlist) {
		g_online_cmdlist_idxs |= cmdlist_idxs;

		//add taskend for share channel
		hisi_cmdlist_add_nop_node(hisifd, cmdlist_idxs, 0, 0);

		//remove ch cmdlist
		hisi_cmdlist_config_stop(hisifd, cmdlist_pre_idxs);

		cmdlist_idxs |= cmdlist_pre_idxs;
		hisi_cmdlist_flush_cache(hisifd, cmdlist_idxs);
	}

	isr_wch = inp32(hisifd->dss_base + GLB_CPU_OFF_INTS);
	isr_wb = inp32(hisifd->dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS);
	while (((isr_wch & BIT_OFF_WCH0_INTS) != BIT_OFF_WCH0_INTS)
		&& ((isr_wb & BIT_WB_ONLINE_ERR_INTS) != BIT_WB_ONLINE_ERR_INTS)
		&& (count < ONLINE_WB_TIMEOUT_COUNT)) {
		time = time / 2;
		if (time >= 1000) {
			udelay(time);
		} else {
			udelay(1);
			time = 1;
		}
		isr_wch = inp32(hisifd->dss_base + GLB_CPU_OFF_INTS);
		isr_wb = inp32(hisifd->dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS);
		count += time;
	}
	outp32(hisifd->dss_base + GLB_CPU_OFF_INTS, BIT_OFF_WCH0_INTS);
	outp32(hisifd->dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS, BIT_WB_ONLINE_ERR_INTS);

	if ((isr_wb & BIT_WB_ONLINE_ERR_INTS) == BIT_WB_ONLINE_ERR_INTS) {
		enable_ldi(hisifd);
		hisifb_video_idle_wb_err_clear(hisifd);
		return -1;
	}

	if (count == ONLINE_WB_TIMEOUT_COUNT) {
		enable_ldi(hisifd);
		hisifb_video_idle_wb_err_clear(hisifd);
		return -1;
	}
	__flush_dcache_area(vic->wb_buffer_base, vic->wb_buffer_size);
	HISI_FB_INFO("enter video idle success! time: %d us ++++++.\n", count);

	if (enable_cmdlist) {
		hisi_cmdlist_config_start(hisifd, vic->ovl_idx, cmdlist_idxs, 0);
	} else {
		hisi_dss_mctl_mutex_unlock(hisifd, vic->ovl_idx);
	}
	enable_ldi(hisifd);

	//hisifb_video_idle_wb_save_buffer(hisifd);

	if (g_debug_ovl_cmdlist)
		hisi_cmdlist_dump_all_node(hisifd, NULL, cmdlist_idxs);

	hisifd->frame_count++;
	pov_req_prev->ov_block_nums = 1;
	pov_req_prev->ovl_idx = DSS_OVL0;
	pov_h_block_infos = (dss_overlay_block_t *)pov_req_prev->ov_block_infos_ptr;
	pov_h_block = &(pov_h_block_infos[0]);
	pov_h_block->layer_nums = 1;
	pov_h_block->layer_infos[0].chn_idx = vic->rch_idx;
	pov_h_block->layer_infos[0].layer_idx = 0;

	return ret;
}

static void hisifb_video_idle_workqueue_handler(struct work_struct *work)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisifb_video_idle_ctrl *vic = NULL;
	vic = container_of(work, typeof(*vic), video_idle_ctrl_work);
	if (NULL == vic) {
		HISI_FB_ERR("video_idle_ctrl is NULL \n");
		return;
	}

	hisifd = vic->hisifd;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL \n");
		return;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->ov_req.video_idle_status) {
		HISI_FB_INFO("-----> not in idle.\n");
		up(&hisifd->blank_sem);
		return;
	}

	if (!vic->video_idle_wb_status) {
		HISI_FB_INFO("-----> not online wb.\n");
		up(&hisifd->blank_sem);
		return;
	}

	if (!hisifd->panel_power_on) {
		HISI_FB_INFO("fb%d, panel is power off!", hisifd->index);
		up(&hisifd->blank_sem);
		return ;
	}

	if (!vic->video_idle_rb_status) {
		if (!hisifb_video_idle_rb_config(hisifd)) {
			vic->video_idle_wb_status = false;
			vic->video_idle_rb_status = true;
		}
	}
	up(&hisifd->blank_sem);
}

static void hisifb_video_idle_rb_clear(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dbuf_base = NULL;
	char __iomem *dss_base = NULL;
	struct hisifb_video_idle_ctrl *vic = NULL;
	vic = &(hisifd->video_idle_ctrl);

	dss_base = hisifd->dss_base;
	hisifd->set_reg(hisifd, dss_base + DSS_VBIF0_AIF + ARCACH0_ADDR_THRH, 0x0, 32, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_VBIF0_AIF + ARCACH0_ADDR_THRL, 0xFFFFFFFF, 32, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_VBIF0_AIF + ARCACH0_VALUE0, 0x0, 32, 0);
	hisifd->set_reg(hisifd, dss_base + DSS_VBIF0_AIF + ARCACH0_VALUE1, 0x0, 32, 0);

	dbuf_base = dss_base + DSS_DBUF0_OFFSET;
	hisifd->set_reg(hisifd, dbuf_base + DBUF_UNPACK_IMG_FMT, 0x0, 32, 0);
	hisifd->set_reg(hisifd, dbuf_base + DBUF_WB_UNPACK_EN, 0x0, 32, 0);
	hisifd->set_reg(hisifd, dbuf_base + DBUF_POST_CLIP_DISP_SIZE,0x0, 32, 0);
	hisifd->set_reg(hisifd, dbuf_base + DBUF_POST_CLIP_CTL_HRZ, 0x0, 32, 0);
	hisifd->set_reg(hisifd, dbuf_base + DBUF_WB_UNPACK_SIZE, 0x0, 32, 0);

	/*recovery IFBC & DPP configeration*/
	if (vic->rb_closed_reg.need_recovery) {
		hisifd->set_reg(hisifd, dss_base + DSS_DSC_OFFSET + DSC_EN,
			vic->rb_closed_reg.dsc_en, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_DPP_OFFSET + DPP_CLIP_EN,
			vic->rb_closed_reg.dpp_clip_en, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_DPP_DITHER_OFFSET + DITHER_CTL0,
			vic->rb_closed_reg.dither_ctl0, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_DPP_GAMA_OFFSET + GAMA_EN,
			vic->rb_closed_reg.gama_en, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_DPP_XCC_OFFSET + XCC_EN,
			vic->rb_closed_reg.xcc_en, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_DPP_DEGAMMA_OFFSET + DEGAMA_EN,
			vic->rb_closed_reg.degama_en, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_DPP_GMP_OFFSET + GMP_EN,
			vic->rb_closed_reg.gmp_en, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_DPP_ARSR_POST_OFFSET + ARSR_POST_MODE,
			vic->rb_closed_reg.arsr_post_bypass, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_IFBC_OFFSET + IFBC_EN,
			vic->rb_closed_reg.ifbc_en, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_ACE,
			vic->rb_closed_reg.hiace_bypass, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_NR,
			vic->rb_closed_reg.nr_bypass, 32, 0);

		hisifd->set_reg(hisifd, dss_base + DSS_DPP_OFFSET + DPP_SBL,
			vic->rb_closed_reg.sbl_en, 32, 0);

		vic->rb_closed_reg.need_recovery = false;
	}
}


irqreturn_t hisifb_video_idle_dss_wb_isr(int irq, void *ptr)
{
	uint32_t isr_s1 = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisifb_video_idle_ctrl *video_idle_ctrl = NULL;

	hisifd = (struct hisi_fb_data_type *)ptr;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return IRQ_NONE;
	}
	video_idle_ctrl = &(hisifd->video_idle_ctrl);
	isr_s1 = inp32(hisifd->dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS);
	outp32(hisifd->dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS, isr_s1);
	if ((isr_s1 & BIT_WB_ONLINE_ERR_INTS) == BIT_WB_ONLINE_ERR_INTS) {
		schedule_work(&(video_idle_ctrl->wb_err_work));
	}

	return IRQ_HANDLED;
}

void hisifb_video_idle_ctrl_register(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisifb_video_idle_ctrl *vic = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL\n");
		return;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		dev_err(&pdev->dev, "hisifd is NULL\n");
		return;
	}

	vic = &(hisifd->video_idle_ctrl);
	if (vic->idle_ctrl_created) {
		return;
	}

	if (!is_video_idle_ctrl_mode(hisifd)) {
		return;
	}

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		return;
	}

	hisifb_video_idle_init(hisifd);

	INIT_WORK(&vic->video_idle_ctrl_work, hisifb_video_idle_workqueue_handler);
	mutex_init(&(vic->video_idle_ctrl_lock));


	vic->idle_ctrl_created = 1;
	return;
}

void hisifb_video_idle_ctrl_unregister(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisifb_video_idle_ctrl *vic = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL\n");
		return;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		dev_err(&pdev->dev, "hisifd is NULL\n");
		return;
	}

	if (!is_video_idle_ctrl_mode(hisifd)) {
		return;
	}

	vic = &(hisifd->video_idle_ctrl);
	if (!vic->idle_ctrl_created) {
		return;
	}

	vic->idle_ctrl_created = 0;
	return;
}

int hisifb_video_idle_check_enable(struct hisi_fb_data_type *hisifd, uint32_t video_idle_status)
{
	int i = 0, m = 0;
	static int dss_free_cache_refcount = 0;
	struct hisifb_video_idle_ctrl *vic = NULL;
	dss_overlay_t *pov_req = NULL;
	dss_layer_t *layer = NULL;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL \n");
		return -EINVAL;
	}

	if (!g_enable_video_idle_l3cache) {
		return 0;
	}

	if (!is_video_idle_ctrl_mode(hisifd)) {
		return 0;
	}

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		return 0;
	}

	vic = &(hisifd->video_idle_ctrl);
	if (hisifd->ov_req.sec_enable_status == DSS_SEC_ENABLE) {
		goto exit;
	}

	pov_req = &(hisifd->ov_req);
	pov_h_block_infos = (dss_overlay_block_t *)(pov_req->ov_block_infos_ptr);
	for (m = 0; m < (int)pov_req->ov_block_nums; m++) {
		pov_h_block = &(pov_h_block_infos[m]);
		for (i = 0; i < (int)pov_h_block->layer_nums; i++) {
			layer = &(pov_h_block->layer_infos[i]);
			if (layer->img.secure_mode == 1) {
				goto exit;
			}
		}
	}

	if (video_idle_status && !vic->video_idle_wb_status) {
		// 0. interrupt config
		hisifb_video_idle_interrupt_config(hisifd);

		// 1. alloc buffer and l3cache
		hisifb_video_idle_buffer_alloc(hisifd, vic->l3cache_size);
		if (!vic->buffer_alloced) {
			HISI_FB_INFO("request l3cache buffer failed, exit video idle.\n");
			return 0;
		}
		// 2. wb config
		hisifb_video_idle_wb_pipe_set_reg(hisifd);

		// 3. wch config
		hisifb_video_idle_wch_config(hisifd);
		hisifb_video_idle_wch_set_reg(hisifd);

		// 4. wait for next vsync
		vic->video_idle_wb_status = true;
		dss_free_cache_refcount = 0;

		schedule_work(&(vic->video_idle_ctrl_work));

		return 0;
	} else {
		dss_free_cache_refcount++;
	}

exit:
	if (vic->video_idle_wb_status) {
		HISI_FB_INFO("exit video idle! wb clear ------.\n");
		hisifb_video_idle_wb_clear(hisifd);
	}

	if (vic->video_idle_rb_status) {
		HISI_FB_INFO("exit video idle! rb clear ------.\n");
		hisifb_video_idle_rb_clear(hisifd);
	}

	/* delay two frame */
	if (dss_free_cache_refcount > 3) {
		hisifb_video_idle_buffer_free(hisifd);
	}
	vic->video_idle_wb_status = false;
	vic->video_idle_rb_status = false;

	return 0;
}

void hisifb_hisync_disp_sync_enable(struct hisi_fb_data_type *hisifd)
{
	char __iomem *cmdlist_base = NULL;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null ptr\n");
		return;
	}

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		return;
	}
	HISI_FB_INFO("enable hisync feature!\n");

	cmdlist_base = hisifd->dss_base + DSS_CMDLIST_OFFSET;
	set_reg(cmdlist_base + CMD_SYNC_MODE, 0x1, 1, 0);
}

int hisifb_hisync_disp_sync_config(struct hisi_fb_data_type *hisifd)
{
	uint32_t try_times = 0;
	uint32_t cycle = 0;
	char __iomem *ldi_base = NULL;
	char __iomem *cmdlist_base = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null ptr\n");
		return -1;
	}

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		return -1;
	}

	cmdlist_base = hisifd->dss_base + DSS_CMDLIST_OFFSET;
	while (inp32(cmdlist_base + CMD_CFG_FLAG) & BIT(0)) {
		udelay(1);
		if (++try_times > 10000) {
			HISI_FB_ERR("Read cmd_cfg_flag timeout!\n");
			return -1;
		}
	}
	HISI_FB_INFO("wait hisync disp sync succss!\n");

	set_reg(cmdlist_base + CMD_CFG_FLAG, 0x1, 1, 0);

	ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;
	if (hisifd->panel_info.vsync_delay_time > 0) {
		cycle = hisifd->panel_info.vsync_delay_time * hisifd->panel_info.pxl_clk_rate / 1000;
		set_reg(ldi_base + VSYNC_DELAY_TIME, cycle - 1, 32, 0);
		set_reg(ldi_base + VSYNC_DELAY_EN, 0x1, 32, 0);
		set_reg(ldi_base + VSYNC_DELAY_DIS, 0x0, 32, 0);
	} else {
		set_reg(ldi_base + VSYNC_DELAY_EN, 0x0, 32, 0);
	}

	return 0;
}
/*lint +e679 +e429*/
#pragma GCC diagnostic pop
