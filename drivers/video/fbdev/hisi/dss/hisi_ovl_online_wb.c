/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
#include "hisi_fb.h"

static void hisifb_ovl_online_wb_init(struct hisi_fb_data_type *hisifd)
{
	int bpp = 0;
	dss_rect_t rect;
	struct hisifb_writeback *wb_ctrl = NULL;

	if (NULL == hisifd) {
		return;
	}
	HISI_FB_INFO("fb%d enter.\n", hisifd->index);

	wb_ctrl = &(hisifd->wb_ctrl);

	rect.x = 0;
	rect.y = 0;
	rect.w = hisifd->panel_info.xres;
	rect.h = hisifd->panel_info.yres;
	if (wb_ctrl->wdma_format == DMA_PIXEL_FORMAT_RGB_565) {
		bpp = 2;
	} else if (wb_ctrl->wdma_format == DMA_PIXEL_FORMAT_ARGB_8888) {
		bpp = 4;
	} else {
		bpp = 4;
	}
	wb_ctrl->wb_buffer_size = ALIGN_UP(rect.w * bpp, DMA_STRIDE_ALIGN) * rect.h;

	mipi_ifbc_get_rect(hisifd, &rect);
	wb_ctrl->wb_hsize = ALIGN_UP(rect.w, 8);
	wb_ctrl->wb_pad_num = ALIGN_UP(rect.w, 8) - rect.w;

	if (wb_ctrl->compress_enable) {
		wb_ctrl->wb_pack_hsize = ALIGN_UP(rect.w, 8) * 3 / 4;
	} else {
		wb_ctrl->wb_pack_hsize = ALIGN_UP(rect.w, 8);
	}

	wb_ctrl->wdfc_pad_hsize = ALIGN_UP(wb_ctrl->wb_pack_hsize, 4);
	wb_ctrl->wdfc_pad_num = ALIGN_UP(wb_ctrl->wb_pack_hsize, 4) - wb_ctrl->wb_pack_hsize;
	wb_ctrl->wb_vsize = rect.h;

	HISI_FB_INFO("fb%d exit: mmu_enable(%d), afbc_enable(%d), compress_enable(%d)\n"
		"\t ovl_idx(%d), wch_idx(%d), wdma_format(%d), wb_hsize(%d), wb_vsize(%d)\n"
		"\t wb_pad_num(%d), wb_pack_hsize(%d), wdfc_pad_hsize(%d), wdfc_pad_num(%d),"
		"wb_buffer_size(%d)\n",
		hisifd->index, wb_ctrl->mmu_enable, wb_ctrl->afbc_enable, wb_ctrl->compress_enable,
		wb_ctrl->ovl_idx, wb_ctrl->wch_idx, wb_ctrl->wdma_format, wb_ctrl->wb_hsize, wb_ctrl->wb_vsize,
		wb_ctrl->wb_pad_num, wb_ctrl->wb_pack_hsize, wb_ctrl->wdfc_pad_hsize, wb_ctrl->wdfc_pad_num,
		wb_ctrl->wb_buffer_size);
}

/*lint -e429*/
static int hisifb_ovl_online_wb_alloc_buffer(struct hisi_fb_data_type *hisifd)
{
	struct ion_client *client = NULL;
	struct ion_handle *handle = NULL; //lint !e429
	struct hisifb_writeback *wb_ctrl = NULL;
	size_t buf_len = 0;
	unsigned long buf_addr = 0;

	if (NULL == hisifd || !(hisifd->pdev)) {
		return -EINVAL;
	}

	wb_ctrl = &(hisifd->wb_ctrl);
	if (wb_ctrl->buffer_alloced) {
		HISI_FB_INFO("fb%d buffer already alloced.\n", hisifd->index);
		return 0;
	}
	buf_len = roundup(wb_ctrl->wb_buffer_size, PAGE_SIZE);

	client = hisifd->ion_client;
	if (IS_ERR_OR_NULL(client)) {
		HISI_FB_ERR("fb%d failed to create ion client!\n", hisifd->index);
		return -EINVAL;
	}

	//handle = ion_alloc(client, buf_len, PAGE_SIZE, ION_HEAP(ION_SYSTEM_HEAP_ID), 0); //lint !e429
	handle = ion_alloc(client, buf_len, PAGE_SIZE, ION_HEAP(ION_GRALLOC_HEAP_ID), 0); //lint !e429
	if (IS_ERR_OR_NULL(handle)) { //lint !e429
		HISI_FB_ERR("fb%d failed to ion_alloc!\n", hisifd->index);
		return -EINVAL;
	}

	wb_ctrl->wb_buffer_base = ion_map_kernel(client, handle);
	if (!wb_ctrl->wb_buffer_base) {
		HISI_FB_ERR("fb%d failed to ion_map_kernel!\n", hisifd->index);
		ion_free(client, handle); //lint !e668
		return -EINVAL;
	}

	if (hisifb_ion_phys(client, handle, &(hisifd->pdev->dev), &buf_addr, &buf_len)) {
		HISI_FB_ERR("fb%d failed to get ion phys!\n", hisifd->index);
		ion_unmap_kernel(client, handle);
		ion_free(client, handle); //lint !e668
		return -EINVAL;
	}

	wb_ctrl->wb_handle = handle;
	wb_ctrl->wb_phys_addr = buf_addr;
	wb_ctrl->buffer_alloced = true;
	wb_ctrl->wb_buffer_size = buf_len;

	HISI_FB_INFO("fb%d wb_handle=%p, wb_phys_addr=0x%x!\n",
		hisifd->index, wb_ctrl->wb_handle, (uint32_t)wb_ctrl->wb_phys_addr);

	return 0;
}
/*lint +e429*/

static int hisifb_ovl_online_wb_free_buffer(struct hisi_fb_data_type *hisifd)
{
	struct hisifb_writeback *wb_ctrl = NULL;

	if (NULL == hisifd) {
		return -EINVAL;
	}
	HISI_FB_INFO("fb%d enter.\n", hisifd->index);

	wb_ctrl = &(hisifd->wb_ctrl);
	if (!wb_ctrl->buffer_alloced) {
		HISI_FB_ERR("fb%d buffer already free.\n", hisifd->index);
		return 0;
	}

	if (hisifd->ion_client != NULL && wb_ctrl->wb_handle != NULL) {
		//ion_unmap_iommu(hisifd->ion_client, wb_ctrl->wb_handle);
		ion_unmap_kernel(hisifd->ion_client, wb_ctrl->wb_handle);
		ion_free(hisifd->ion_client, wb_ctrl->wb_handle);

		wb_ctrl->wb_handle = NULL;
		wb_ctrl->wb_buffer_base = 0;
		wb_ctrl->wb_phys_addr = 0;
		wb_ctrl->buffer_alloced = false;
	}

	HISI_FB_INFO("fb%d exit.\n", hisifd->index);

	return 0;
}

static void hisifb_ovl_online_wb_save_buffer(struct hisi_fb_data_type *hisifd)
{
	struct hisifb_writeback *wb_ctrl = NULL;
	char filename[256] = {0};
	ssize_t write_len = 0;
	struct file *fd = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	wb_ctrl = &(hisifd->wb_ctrl);
	HISI_FB_INFO("fb%d enter.\n", hisifd->index);

	snprintf(filename, 256, "/data/dssdump/dss_frame_%d.bin", hisifd->ov_req.frame_no);
	fd = filp_open(filename, O_CREAT|O_RDWR, 0644);
	if (IS_ERR(fd)) {
		HISI_FB_WARNING("filp_open returned: filename %s, error %ld\n",
			filename, PTR_ERR(fd));
		return;
	}

	old_fs = get_fs(); //lint !e501
	set_fs(KERNEL_DS); //lint !e501

	write_len = vfs_write(fd, wb_ctrl->wb_buffer_base, wb_ctrl->wb_buffer_size, &pos);

	pos = 0;
	set_fs(old_fs);
	filp_close(fd, NULL);

	HISI_FB_INFO("fb%d exit.\n", hisifd->index);
}

static void hisifb_ovl_online_wb_interrupt_clear(struct hisi_fb_data_type *hisifd)
{
	uint32_t isr_s1 = 0;
	char __iomem *dss_base = 0;

	dss_base = hisifd->dss_base;

	isr_s1 = BIT_DSS_WB_ERR_INTS;//0x12224
	outp32(hisifd->dss_base + GLB_CPU_PDP_INTS, isr_s1);

	isr_s1 = BIT_OFF_WCH0_INTS; // BIT_OFF_WCH0_INTS
	outp32(dss_base + GLB_CPU_OFF_INTS, isr_s1);//0x12234

	isr_s1 = BIT_WB_ONLINE_ERR_INT_MSK;
	outp32(dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS, isr_s1);//0x7d400+0x24
}

static void hisifb_ovl_online_wb_pipe_set_reg(struct hisi_fb_data_type *hisifd)
{
	char __iomem *wb_base = NULL;
	char __iomem *pipe_sw_base = NULL;
	struct hisifb_writeback *wb_ctrl = NULL;
	wb_ctrl = &(hisifd->wb_ctrl);

	/*step1. pipe sw config*/
	pipe_sw_base = hisifd->dss_base + DSS_PIPE_SW_WB_OFFSET;
	hisifd->set_reg(hisifd, pipe_sw_base + PIPE_SW_SIG_CTRL, 0x1, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_base + PIPE_SW_DAT_CTRL, 0x1, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_base + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);

	hisifd->set_reg(hisifd, pipe_sw_base + NXT_SW_NO_PR, 0x1, 32, 0);

	/*step2. wb pack config*/
	wb_base = hisifd->dss_base + DSS_WB_OFFSET;
	if (wb_ctrl->compress_enable) {
		hisifd->set_reg(hisifd, wb_base + WB_CTRL, 0x3, 32, 0);
	} else {
		hisifd->set_reg(hisifd, wb_base + WB_CTRL, 0x2, 32, 0);
	}
	hisifd->set_reg(hisifd, wb_base + WB_WORK_MODE, 0x1, 32, 0);

	hisifd->set_reg(hisifd, wb_base + WB_IMG_HSIZE, DSS_WIDTH(wb_ctrl->wb_hsize), 32, 0);
	hisifd->set_reg(hisifd, wb_base + WB_IMG_VSIZE, DSS_HEIGHT(wb_ctrl->wb_vsize), 32, 0);
	hisifd->set_reg(hisifd, wb_base + WB_PAD_NUM, wb_ctrl->wb_pad_num, 32, 0);
}

static void hisifb_ovl_online_wb_mmu_config(struct hisi_fb_data_type *hisifd)
{
	uint32_t i, idx;
	uint32_t chn_idx = 0;
	dss_smmu_t *s_smmu = NULL;
	struct hisifb_writeback *wb_ctrl = NULL;
	wb_ctrl = &(hisifd->wb_ctrl);
	chn_idx = wb_ctrl->wch_idx;

	s_smmu = &(hisifd->dss_module.smmu);
	hisifd->dss_module.smmu_used = 1;

	s_smmu->smmu_smrx_ns_used[chn_idx] = 1;
	for (i = 0; i < g_dss_chn_sid_num[chn_idx]; i++) {
		idx = g_dss_smmu_smrx_idx[chn_idx] + i;
		if (idx >= SMMU_SID_NUM) {
			HISI_FB_WARNING("idx is invalid.\n");
			return ;
		}

		if (wb_ctrl->mmu_enable) {
			s_smmu->smmu_smrx_ns[idx] = set_bits32(s_smmu->smmu_smrx_ns[idx], 0x0, 1, 0);  //smr_bypass
			s_smmu->smmu_smrx_ns[idx] = set_bits32(s_smmu->smmu_smrx_ns[idx], 0x1, 1, 4);  //smr_invld_en
		} else {
			s_smmu->smmu_smrx_ns[idx] = set_bits32(s_smmu->smmu_smrx_ns[idx], 0x1, 1, 0);
		}
	}
	return;
}

static void hisifb_ovl_online_wb_set_reg(struct hisi_fb_data_type *hisifd)
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
	struct hisifb_writeback *wb_ctrl = NULL;
	wb_ctrl = &(hisifd->wb_ctrl);
	chn_idx = wb_ctrl->wch_idx;
	ovl_idx = wb_ctrl->ovl_idx;

	/* step1. mctl config*/
	mctl_mutex_base = hisifd->dss_base + g_dss_module_ovl_base[ovl_idx][MODULE_MCTL_BASE];
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_MUTEX_WB, 0x1, 32, 0);
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_MUTEX_WCH0, 0x1, 32, 0); // MCTL_CTL_MUTEX_WCH0

	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_END_SEL, 0xffffbeff, 32, 0);

	mctl_ov_ien = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_OV_OEN];
	hisifd->set_reg(hisifd, mctl_ov_ien, 0x3, 32, 0);

	/* step2. wch0 config*/
	s_wdma = &(hisifd->dss_module.wdma[chn_idx]);
	wdma_base = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_DMA];
	hisifd->set_reg(hisifd, wdma_base + CH_REG_DEFAULT, 0x1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + CH_REG_DEFAULT, 0x0, 32, 0);

	hisifd->set_reg(hisifd, wdma_base + DMA_OFT_X0, 0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_OFT_Y0, 0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_OFT_X1, DSS_WIDTH(wb_ctrl->wdfc_pad_hsize) / 4, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_OFT_Y1, DSS_HEIGHT(wb_ctrl->wb_vsize), 32, 0);

	hisifd->set_reg(hisifd, wdma_base + DMA_CTRL, 0x28, 32, 0); // FIXME: DMA_PIXEL_FORMAT_ARGB_8888?
	hisifd->set_reg(hisifd, wdma_base + DMA_DATA_ADDR0, wb_ctrl->wb_phys_addr, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + DMA_STRIDE0, wb_ctrl->wdfc_pad_hsize / 4, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + CH_CTL, 0x1, 32, 0);

	hisifd->set_reg(hisifd, wdma_base + DMA_BUF_SIZE,
		(DSS_HEIGHT(wb_ctrl->wb_vsize) << 16) | DSS_WIDTH(wb_ctrl->wdfc_pad_hsize), 32, 0);
	hisifd->set_reg(hisifd, wdma_base + ROT_SIZE,
		(DSS_HEIGHT(wb_ctrl->wb_vsize) << 16) | DSS_WIDTH(wb_ctrl->wdfc_pad_hsize), 32, 0);

	/* step3. wch0-dfc config*/
	s_dfc = &(hisifd->dss_module.dfc[chn_idx]);
	dfc_base = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_DFC];
	hisifd->set_reg(hisifd, dfc_base + DFC_PADDING_CTL, BIT(31) | (wb_ctrl->wdfc_pad_num << 8), 32, 0); //lint !e647
	hisifd->set_reg(hisifd, dfc_base + DFC_DISP_SIZE, (DSS_WIDTH(wb_ctrl->wb_pack_hsize) << 16) | DSS_HEIGHT(wb_ctrl->wb_vsize), 32, 0);
	hisifd->set_reg(hisifd, dfc_base + DFC_DISP_FMT, 0xC, 32, 0); //FIXME DFC_PIXEL_FORMAT_ARGB_8888?
	hisifd->set_reg(hisifd, dfc_base + DFC_ICG_MODULE, 0x1, 32, 0);

	hisifd->set_reg(hisifd, hisifd->dss_base +
		g_dss_module_base[chn_idx][MODULE_AIF0_CHN], 0xF000, 32, 0); //AXI_CHN0

	hisifd->set_reg(hisifd, hisifd->dss_base +
		g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL1, BIT(17) | BIT(5), 32, 0);

	/* step4. smmu config*/
	s_smmu = &(hisifd->dss_module.smmu);
	hisi_dss_smmu_ch_set_reg(hisifd, hisifd->dss_module.smmu_base, s_smmu, chn_idx); // 0x1d

	/* step5. mctl sys config*/
	mctl_chn_flush_en = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_FLUSH_EN];
	hisifd->set_reg(hisifd, mctl_chn_flush_en, 0x1, 32, 0);
}

static void hisifb_ovl_online_wb_clear(struct hisi_fb_data_type *hisifd)
{
	uint32_t ovl_idx = 0;
	uint32_t chn_idx = 0;
	char __iomem *wb_base = NULL;
	char __iomem *pipe_sw_wb_base = NULL;
	char __iomem *mctl_ov_ien = NULL;
	char __iomem *mctl_chn_flush_en = NULL;
	char __iomem *mctl_mutex_base = NULL;
	struct hisifb_writeback *wb_ctrl = NULL;
	wb_ctrl = &(hisifd->wb_ctrl);
	chn_idx = wb_ctrl->wch_idx;
	ovl_idx = wb_ctrl->ovl_idx;

	if (!wb_ctrl->wb_clear_config) {
		return;
	}
	HISI_FB_INFO("fb%d clear wb config enter.\n", hisifd->index);

	/*step1. pipe sw config*/
	pipe_sw_wb_base = hisifd->dss_base + DSS_PIPE_SW_WB_OFFSET;
	hisifd->set_reg(hisifd, pipe_sw_wb_base + PIPE_SW_SIG_CTRL, 0x0, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_wb_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_wb_base + PIPE_SW_DAT_CTRL, 0x0, 32, 0);
	hisifd->set_reg(hisifd, pipe_sw_wb_base + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);

	/*step2. wb pack config*/
	wb_base = hisifd->dss_base + DSS_WB_OFFSET;
	hisifd->set_reg(hisifd, wb_base + WB_REG_DEFAULT, 0x1, 32, 0);
	hisifd->set_reg(hisifd, wb_base + WB_REG_DEFAULT, 0x0, 32, 0);

	/* step3. mctl config*/
	mctl_mutex_base = hisifd->dss_base + g_dss_module_ovl_base[ovl_idx][MODULE_MCTL_BASE];
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_MUTEX_WB, 0x1, 32, 0);
	hisifd->set_reg(hisifd, mctl_mutex_base + MCTL_CTL_MUTEX_WCH0, 0x1, 32, 0);

	mctl_ov_ien = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_OV_OEN];
	hisifd->set_reg(hisifd, mctl_ov_ien, 0x0, 32, 0);

	/* step5. mctl sys config*/
	mctl_chn_flush_en = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_FLUSH_EN];
	hisifd->set_reg(hisifd, mctl_chn_flush_en, 0x1, 32, 0);

	wb_ctrl->wb_clear_config = false;
	HISI_FB_INFO("fb%d clear wb config exit.\n", hisifd->index);
}

void hisifb_ovl_online_wb_config(struct hisi_fb_data_type *hisifd)
{
	struct hisifb_writeback *wb_ctrl = NULL;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		return;
	}
	hisifb_ovl_online_wb_clear(hisifd);

	if ((g_debug_ovl_online_wb_count <= 0) || (g_debug_ovl_online_wb_count > 10)) {
		return;
	}

	wb_ctrl = &(hisifd->wb_ctrl);
	mutex_lock(&(wb_ctrl->wb_ctrl_lock));

	hisifb_ovl_online_wb_init(hisifd);
	hisifb_ovl_online_wb_alloc_buffer(hisifd);

	hisifb_ovl_online_wb_interrupt_clear(hisifd);

	hisifb_ovl_online_wb_pipe_set_reg(hisifd);

	hisifb_ovl_online_wb_mmu_config(hisifd);

	hisifb_ovl_online_wb_set_reg(hisifd);

	schedule_work(&(wb_ctrl->wb_ctrl_work));

	mutex_unlock(&(wb_ctrl->wb_ctrl_lock));

	g_debug_ovl_online_wb_count--;
}

#define ONLINE_WB_TIMEOUT_COUNT (10)
static void hisifb_ovl_online_wb_workqueue_handler(struct work_struct *work)
{
	int count = 0;
	uint32_t isr_s1 = 0;
	uint32_t ovl_idx = 0;
	uint32_t chn_idx = 0;
	char __iomem *wb_base = NULL;
	char __iomem *pipe_sw_wb_base = NULL;
	char __iomem *mctl_ov_ien = NULL;
	char __iomem *mctl_chn_flush_en = NULL;
	char __iomem *mctl_mutex_base = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisifb_writeback *wb_ctrl = NULL;

	wb_ctrl = container_of(work, typeof(*wb_ctrl), wb_ctrl_work);
	if (NULL == wb_ctrl) {
		HISI_FB_ERR("wb_ctrl is NULL.\n");
		return;
	}

	hisifd = wb_ctrl->hisifd;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL.\n");
		return;
	}

	HISI_FB_INFO("fb%d enter.\n", hisifd->index);

	down(&hisifd->blank_sem);

	hisifb_activate_vsync(hisifd);

	isr_s1 = inp32(hisifd->dss_base + GLB_CPU_OFF_INTS);
	while (((isr_s1 & BIT_OFF_WCH0_INTS) != BIT_OFF_WCH0_INTS) && (count < ONLINE_WB_TIMEOUT_COUNT)) {
		msleep(10);
		isr_s1 = inp32(hisifd->dss_base + GLB_CPU_OFF_INTS);
		HISI_FB_INFO("GLB_CPU_OFF_INTS: 0x%x. count = %d\n", isr_s1, count);
		count++;
	}
	if (count == ONLINE_WB_TIMEOUT_COUNT) {
		HISI_FB_INFO("GLB_CPU_OFF_INTS: 0x%x. timeout\n", isr_s1);
		up(&hisifd->blank_sem);
		return;
	}
	outp32(hisifd->dss_base + GLB_CPU_OFF_INTS, BIT_OFF_WCH0_INTS);

	/*step1. pipe sw config*/
	pipe_sw_wb_base = hisifd->dss_base + DSS_PIPE_SW_WB_OFFSET;
	set_reg(pipe_sw_wb_base + PIPE_SW_SIG_CTRL, 0x0, 32, 0);
	set_reg(pipe_sw_wb_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
	set_reg(pipe_sw_wb_base + PIPE_SW_DAT_CTRL, 0x0, 32, 0);
	set_reg(pipe_sw_wb_base + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);

	/*step2. wb pack config*/
	wb_base = hisifd->dss_base + DSS_WB_OFFSET;
	set_reg(wb_base + WB_REG_DEFAULT, 0x1, 32, 0);
	set_reg(wb_base + WB_REG_DEFAULT, 0x0, 32, 0);

	/* step3. mctl config*/
	ovl_idx = wb_ctrl->ovl_idx;
	mctl_mutex_base = hisifd->dss_base + g_dss_module_ovl_base[ovl_idx][MODULE_MCTL_BASE];
	set_reg(mctl_mutex_base + MCTL_CTL_MUTEX_WB, 0x1, 32, 0);
	set_reg(mctl_mutex_base + MCTL_CTL_MUTEX_WCH0, 0x1, 32, 0);

	chn_idx = wb_ctrl->wch_idx;
	mctl_ov_ien = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_OV_OEN];
	set_reg(mctl_ov_ien, 0x0, 32, 0);

	/* step5. mctl sys config*/
	mctl_chn_flush_en = hisifd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_FLUSH_EN];
	set_reg(mctl_chn_flush_en, 0x1, 32, 0);

	single_frame_update(hisifd);

	mdelay(16);

	__flush_dcache_area(wb_ctrl->wb_buffer_base, wb_ctrl->wb_buffer_size);

	hisifb_ovl_online_wb_save_buffer(hisifd);

	hisifb_ovl_online_wb_free_buffer(hisifd);

	hisifb_deactivate_vsync(hisifd);

	wb_ctrl->wb_clear_config = true;

	up(&hisifd->blank_sem);

	HISI_FB_INFO("fb%d exit.\n", hisifd->index);

	return;
}

void hisifb_ovl_online_wb_register(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisifb_writeback *wb_ctrl = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL\n");
		return;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return;
	}

	if (PRIMARY_PANEL_IDX != hisifd->index) {
		HISI_FB_INFO("fb%d not support.\n", hisifd->index);
		return;
	}

	wb_ctrl = &(hisifd->wb_ctrl);
	if (wb_ctrl->online_wb_created) {
		return;
	}
	HISI_FB_INFO("fb%d enter.\n", hisifd->index);

	wb_ctrl->hisifd = hisifd;
	wb_ctrl->afbc_enable = false;
	wb_ctrl->mmu_enable = false;
	wb_ctrl->compress_enable = false;
	wb_ctrl->wb_clear_config = false;

	wb_ctrl->buffer_alloced = false;
	wb_ctrl->ovl_idx = DSS_OVL0;
	wb_ctrl->wch_idx = DSS_WCHN_W0; // only wch0 support
	wb_ctrl->wdma_format = DMA_PIXEL_FORMAT_ARGB_8888; // BGRX ok

	mutex_init(&(wb_ctrl->wb_ctrl_lock));

	INIT_WORK(&wb_ctrl->wb_ctrl_work, hisifb_ovl_online_wb_workqueue_handler);
	hisifb_ovl_online_wb_init(hisifd);

	wb_ctrl->wb_init = hisifb_ovl_online_wb_init;
	wb_ctrl->wb_alloc_buffer = hisifb_ovl_online_wb_alloc_buffer;
	wb_ctrl->wb_free_buffer = hisifb_ovl_online_wb_free_buffer;
	wb_ctrl->wb_save_buffer = hisifb_ovl_online_wb_save_buffer;

	wb_ctrl->online_wb_created = 1;

	HISI_FB_INFO("fb%d exit.\n", hisifd->index);

	return;
}

void hisifb_ovl_online_wb_unregister(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisifb_writeback *wb_ctrl = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL\n");
		return;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return;
	}
	wb_ctrl = &(hisifd->wb_ctrl);

	if (!wb_ctrl->online_wb_created) {
		return;
	}
	hisifb_ovl_online_wb_free_buffer(hisifd);

	wb_ctrl->online_wb_created = 0;
	return;
}

