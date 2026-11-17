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

#include "hisi_fb.h"

static void dpufb_ovl_online_wb_init(struct dpu_fb_data_type *dpufd)
{
	int bpp;
	dss_rect_t rect;
	struct dpufb_writeback *wb_ctrl = NULL;

	if (!dpufd)
		return;

	DPU_FB_INFO("fb%d enter.\n", dpufd->index);

	wb_ctrl = &(dpufd->wb_ctrl);

	rect.x = 0;
	rect.y = 0;
	rect.w = dpufd->panel_info.xres;
	rect.h = dpufd->panel_info.yres;
	if (wb_ctrl->wdma_format == DMA_PIXEL_FORMAT_RGB_565)
		bpp = 2;  /* BIT PER PIEXL, 2 - LCD_RGB565 */
	else if (wb_ctrl->wdma_format == DMA_PIXEL_FORMAT_ARGB_8888)
		bpp = 4;  /* BIT PER PIEXL, 4 - LCD_RGBA8888 */
	else
		bpp = 4;

	wb_ctrl->wb_buffer_size = ALIGN_UP(rect.w * bpp, DMA_STRIDE_ALIGN) * rect.h;

	mipi_ifbc_get_rect(dpufd, &rect);
	wb_ctrl->wb_hsize = ALIGN_UP(rect.w, 8);
	wb_ctrl->wb_pad_num = ALIGN_UP(rect.w, 8) - rect.w;

	if (wb_ctrl->compress_enable)
		wb_ctrl->wb_pack_hsize = ALIGN_UP(rect.w, 8) * 3 / 4;  /* 4 bytes align */
	else
		wb_ctrl->wb_pack_hsize = ALIGN_UP(rect.w, 8);

	wb_ctrl->wdfc_pad_hsize = ALIGN_UP(wb_ctrl->wb_pack_hsize, 4);
	wb_ctrl->wdfc_pad_num = ALIGN_UP(wb_ctrl->wb_pack_hsize, 4) - wb_ctrl->wb_pack_hsize;
	wb_ctrl->wb_vsize = rect.h;

	DPU_FB_INFO("fb%d exit: mmu_enable[%d], afbc_enable[%d], compress_enable[%d]\n"
		"\t ovl_idx[%d], wch_idx[%d], wdma_format[%d], wb_hsize[%d], wb_vsize[%d]\n"
		"\t wb_pad_num[%d], wb_pack_hsize[%d], wdfc_pad_hsize[%d], wdfc_pad_num[%d],"
		"wb_buffer_size[%d]\n",
		dpufd->index, wb_ctrl->mmu_enable, wb_ctrl->afbc_enable, wb_ctrl->compress_enable,
		wb_ctrl->ovl_idx, wb_ctrl->wch_idx, wb_ctrl->wdma_format, wb_ctrl->wb_hsize, wb_ctrl->wb_vsize,
		wb_ctrl->wb_pad_num, wb_ctrl->wb_pack_hsize, wb_ctrl->wdfc_pad_hsize, wb_ctrl->wdfc_pad_num,
		wb_ctrl->wb_buffer_size);
}

static int dpufb_ovl_online_wb_alloc_buffer(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_writeback *wb_ctrl = NULL;
	size_t buf_len;
	unsigned long buf_addr = 0;

	if (!dpufd || !(dpufd->pdev))
		return -EINVAL;

	wb_ctrl = &(dpufd->wb_ctrl);
	if (wb_ctrl->buffer_alloced) {
		DPU_FB_INFO("fb%d buffer already alloced.\n", dpufd->index);
		return 0;
	}
	buf_len = roundup(wb_ctrl->wb_buffer_size, PAGE_SIZE);
	if (hisi_dss_alloc_cma_buffer(buf_len, (dma_addr_t *)&buf_addr,
		(void **)&wb_ctrl->wb_buffer_base)) {
		DPU_FB_ERR("fb%d failed to alloc cma buffert!\n", dpufd->index);
		return -EINVAL;
	}

	wb_ctrl->wb_phys_addr = buf_addr;
	wb_ctrl->wb_buffer_size = buf_len;
	wb_ctrl->buffer_alloced = true;

	return 0;
}

static int dpufb_ovl_online_wb_free_buffer(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_writeback *wb_ctrl = NULL;

	DPU_FB_INFO("fb%d enter.\n", dpufd->index);

	wb_ctrl = &(dpufd->wb_ctrl);
	if (!wb_ctrl->buffer_alloced) {
		DPU_FB_ERR("fb%d buffer already free.\n", dpufd->index);
		return 0;
	}

	if (wb_ctrl->wb_buffer_base != 0) {
		hisi_dss_free_cma_buffer(wb_ctrl->wb_buffer_size, wb_ctrl->wb_phys_addr, wb_ctrl->wb_buffer_base);

		wb_ctrl->wb_buffer_base = 0;
		wb_ctrl->wb_phys_addr = 0;
		wb_ctrl->buffer_alloced = false;
	}

	DPU_FB_INFO("fb%d exit.\n", dpufd->index);

	return 0;
}

static void dpufb_ovl_online_wb_save_buffer(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_writeback *wb_ctrl = NULL;
	char filename[256] = {0};  /* ovl online wb save buffer */
	struct file *fd = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	wb_ctrl = &(dpufd->wb_ctrl);
	DPU_FB_INFO("fb%d enter.\n", dpufd->index);

	snprintf(filename, 256, "/data/dssdump/dss_frame_%d.bin", dpufd->ov_req.frame_no);
	fd = filp_open(filename, O_CREAT | O_RDWR, 0644);
	if (IS_ERR(fd)) {
		DPU_FB_WARNING("filp_open returned: filename %s, error %ld\n", filename, PTR_ERR(fd));
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	(void)vfs_write(fd, wb_ctrl->wb_buffer_base, wb_ctrl->wb_buffer_size, &pos);

	pos = 0;
	set_fs(old_fs);
	filp_close(fd, NULL);

	DPU_FB_INFO("fb%d exit.\n", dpufd->index);
}

static void dpufb_ovl_online_wb_interrupt_clear(struct dpu_fb_data_type *dpufd)
{
	uint32_t isr_s1;
	char __iomem *dss_base = NULL;

	dss_base = dpufd->dss_base;

	isr_s1 = BIT_DSS_WB_ERR_INTS;  /* 0x12224 */
	outp32(dpufd->dss_base + GLB_CPU_PDP_INTS, isr_s1);

	isr_s1 = BIT_OFF_WCH0_INTS;  /* BIT_OFF_WCH0_INTS */
	outp32(dss_base + GLB_CPU_OFF_INTS, isr_s1);  /* 0x12234 */

	isr_s1 = BIT_WB_ONLINE_ERR_INT_MSK;
	outp32(dss_base + DSS_WB_OFFSET + WB_ONLINE_ERR_INTS, isr_s1);  /* 0x7d400+0x24 */
}

static void dpufb_ovl_online_wb_pipe_set_reg(struct dpu_fb_data_type *dpufd)
{
	char __iomem *wb_base = NULL;
	char __iomem *pipe_sw_base = NULL;
	struct dpufb_writeback *wb_ctrl = NULL;

	wb_ctrl = &(dpufd->wb_ctrl);

	/* step1. pipe sw config */
	pipe_sw_base = dpufd->dss_base + DSS_PIPE_SW_WB_OFFSET;
	dpufd->set_reg(dpufd, pipe_sw_base + PIPE_SW_SIG_CTRL, 0x1, 32, 0);
	dpufd->set_reg(dpufd, pipe_sw_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
	dpufd->set_reg(dpufd, pipe_sw_base + PIPE_SW_DAT_CTRL, 0x1, 32, 0);
	dpufd->set_reg(dpufd, pipe_sw_base + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);

	dpufd->set_reg(dpufd, pipe_sw_base + NXT_SW_NO_PR, 0x1, 32, 0);

	/* step2. wb pack config */
	wb_base = dpufd->dss_base + DSS_WB_OFFSET;
	if (wb_ctrl->compress_enable)
		dpufd->set_reg(dpufd, wb_base + WB_CTRL, 0x3, 32, 0);
	else
		dpufd->set_reg(dpufd, wb_base + WB_CTRL, 0x2, 32, 0);

	dpufd->set_reg(dpufd, wb_base + WB_WORK_MODE, 0x1, 32, 0);

	dpufd->set_reg(dpufd, wb_base + WB_IMG_HSIZE, DSS_WIDTH(wb_ctrl->wb_hsize), 32, 0);
	dpufd->set_reg(dpufd, wb_base + WB_IMG_VSIZE, DSS_HEIGHT(wb_ctrl->wb_vsize), 32, 0);
	dpufd->set_reg(dpufd, wb_base + WB_PAD_NUM, wb_ctrl->wb_pad_num, 32, 0);
}

static void dpufb_ovl_online_wb_mmu_config(struct dpu_fb_data_type *dpufd)
{
	uint32_t i;
	uint32_t idx;
	uint32_t chn_idx;
	dss_smmu_t *s_smmu = NULL;
	struct dpufb_writeback *wb_ctrl = NULL;

	wb_ctrl = &(dpufd->wb_ctrl);
	chn_idx = wb_ctrl->wch_idx;

	s_smmu = &(dpufd->dss_module.smmu);
	dpufd->dss_module.smmu_used = 1;

	s_smmu->smmu_smrx_ns_used[chn_idx] = 1;
	for (i = 0; i < g_dss_chn_sid_num[chn_idx]; i++) {
		idx = g_dss_smmu_smrx_idx[chn_idx] + i;
		if (idx >= SMMU_SID_NUM) {
			DPU_FB_WARNING("idx is invalid.\n");
			return;
		}

		if (wb_ctrl->mmu_enable) {
			/* smr_bypass */
			s_smmu->smmu_smrx_ns[idx] = set_bits32(s_smmu->smmu_smrx_ns[idx], 0x0, 1, 0);
			/* smr_invld_en */
			s_smmu->smmu_smrx_ns[idx] = set_bits32(s_smmu->smmu_smrx_ns[idx], 0x1, 1, 4);
		} else {
			s_smmu->smmu_smrx_ns[idx] = set_bits32(s_smmu->smmu_smrx_ns[idx], 0x1, 1, 0);
		}
	}
}

static void dpufb_ovl_online_wb_set_reg(struct dpu_fb_data_type *dpufd)
{
	uint32_t ovl_idx;
	uint32_t chn_idx;
	char __iomem *mctl_ov_ien = NULL;
	char __iomem *mctl_chn_flush_en = NULL;
	char __iomem *mctl_mutex_base = NULL;
	char __iomem *wdma_base = NULL;
	char __iomem *dfc_base = NULL;
	dss_smmu_t *s_smmu = NULL;
	struct dpufb_writeback *wb_ctrl = NULL;

	wb_ctrl = &(dpufd->wb_ctrl);
	chn_idx = wb_ctrl->wch_idx;
	ovl_idx = wb_ctrl->ovl_idx;

	/* step1. mctl config */
	mctl_mutex_base = dpufd->dss_base + g_dss_module_ovl_base[ovl_idx][MODULE_MCTL_BASE];
	dpufd->set_reg(dpufd, mctl_mutex_base + MCTL_CTL_MUTEX_WB, 0x1, 32, 0);
	/* MCTL_CTL_MUTEX_WCH0 */
	dpufd->set_reg(dpufd, mctl_mutex_base + MCTL_CTL_MUTEX_WCH0, 0x1, 32, 0);

	dpufd->set_reg(dpufd, mctl_mutex_base + MCTL_CTL_END_SEL, 0xffffbeff, 32, 0);

	mctl_ov_ien = dpufd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_OV_OEN];
	dpufd->set_reg(dpufd, mctl_ov_ien, 0x3, 32, 0);

	/* step2. wch0 config */
	wdma_base = dpufd->dss_base + g_dss_module_base[chn_idx][MODULE_DMA];
	dpufd->set_reg(dpufd, wdma_base + CH_REG_DEFAULT, 0x1, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + CH_REG_DEFAULT, 0x0, 32, 0);

	dpufd->set_reg(dpufd, wdma_base + DMA_OFT_X0, 0, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_OFT_Y0, 0, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_OFT_X1, DSS_WIDTH(wb_ctrl->wdfc_pad_hsize) / 4, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_OFT_Y1, DSS_HEIGHT(wb_ctrl->wb_vsize), 32, 0);

	/* FIXME: DMA_PIXEL_FORMAT_ARGB_8888? */
	dpufd->set_reg(dpufd, wdma_base + DMA_CTRL, 0x28, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_DATA_ADDR0, wb_ctrl->wb_phys_addr, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + DMA_STRIDE0, wb_ctrl->wdfc_pad_hsize / 4, 32, 0);
	dpufd->set_reg(dpufd, wdma_base + CH_CTL, 0x1, 32, 0);

	dpufd->set_reg(dpufd, wdma_base + DMA_BUF_SIZE,
		(DSS_HEIGHT(wb_ctrl->wb_vsize) << 16) | DSS_WIDTH(wb_ctrl->wdfc_pad_hsize), 32, 0);
	dpufd->set_reg(dpufd, wdma_base + ROT_SIZE,
		(DSS_HEIGHT(wb_ctrl->wb_vsize) << 16) | DSS_WIDTH(wb_ctrl->wdfc_pad_hsize), 32, 0);

	/* step3. wch0-dfc config */
	dfc_base = dpufd->dss_base + g_dss_module_base[chn_idx][MODULE_DFC];
	dpufd->set_reg(dpufd, dfc_base + DFC_PADDING_CTL, BIT(31) | (wb_ctrl->wdfc_pad_num << 8), 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_DISP_SIZE,
		(DSS_WIDTH(wb_ctrl->wb_pack_hsize) << 16) | DSS_HEIGHT(wb_ctrl->wb_vsize), 32, 0);
	/* FIXME DFC_PIXEL_FORMAT_ARGB_8888? */
	dpufd->set_reg(dpufd, dfc_base + DFC_DISP_FMT, 0xC, 32, 0);
	dpufd->set_reg(dpufd, dfc_base + DFC_ICG_MODULE, 0x1, 32, 0);

	dpufd->set_reg(dpufd, dpufd->dss_base +
		g_dss_module_base[chn_idx][MODULE_AIF0_CHN], 0xF000, 32, 0);   /* AXI_CHN0 */

	dpufd->set_reg(dpufd, dpufd->dss_base +
		g_dss_module_base[chn_idx][MODULE_MIF_CHN] + MIF_CTRL1, BIT(17) | BIT(5), 32, 0);

	/* step4. smmu config */
	s_smmu = &(dpufd->dss_module.smmu);
	hisi_dss_smmu_ch_set_reg(dpufd, dpufd->dss_module.smmu_base, s_smmu, chn_idx);  /* 0x1d */

	/* step5. mctl sys config */
	mctl_chn_flush_en = dpufd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_FLUSH_EN];
	dpufd->set_reg(dpufd, mctl_chn_flush_en, 0x1, 32, 0);
}

static void dpufb_ovl_online_wb_clear(struct dpu_fb_data_type *dpufd)
{
	uint32_t ovl_idx;
	uint32_t chn_idx;
	char __iomem *wb_base = NULL;
	char __iomem *pipe_sw_wb_base = NULL;
	char __iomem *mctl_ov_ien = NULL;
	char __iomem *mctl_chn_flush_en = NULL;
	char __iomem *mctl_mutex_base = NULL;
	struct dpufb_writeback *wb_ctrl = NULL;

	wb_ctrl = &(dpufd->wb_ctrl);
	chn_idx = wb_ctrl->wch_idx;
	ovl_idx = wb_ctrl->ovl_idx;

	if (!wb_ctrl->wb_clear_config)
		return;

	DPU_FB_INFO("fb%d clear wb config enter.\n", dpufd->index);

	/* step1. pipe sw config */
	pipe_sw_wb_base = dpufd->dss_base + DSS_PIPE_SW_WB_OFFSET;
	dpufd->set_reg(dpufd, pipe_sw_wb_base + PIPE_SW_SIG_CTRL, 0x0, 32, 0);
	dpufd->set_reg(dpufd, pipe_sw_wb_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
	dpufd->set_reg(dpufd, pipe_sw_wb_base + PIPE_SW_DAT_CTRL, 0x0, 32, 0);
	dpufd->set_reg(dpufd, pipe_sw_wb_base + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);

	/* step2. wb pack config */
	wb_base = dpufd->dss_base + DSS_WB_OFFSET;
	dpufd->set_reg(dpufd, wb_base + WB_REG_DEFAULT, 0x1, 32, 0);
	dpufd->set_reg(dpufd, wb_base + WB_REG_DEFAULT, 0x0, 32, 0);

	/* step3. mctl config */
	mctl_mutex_base = dpufd->dss_base + g_dss_module_ovl_base[ovl_idx][MODULE_MCTL_BASE];
	dpufd->set_reg(dpufd, mctl_mutex_base + MCTL_CTL_MUTEX_WB, 0x1, 32, 0);
	dpufd->set_reg(dpufd, mctl_mutex_base + MCTL_CTL_MUTEX_WCH0, 0x1, 32, 0);

	mctl_ov_ien = dpufd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_OV_OEN];
	dpufd->set_reg(dpufd, mctl_ov_ien, 0x0, 32, 0);

	/* step5. mctl sys config */
	mctl_chn_flush_en = dpufd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_FLUSH_EN];
	dpufd->set_reg(dpufd, mctl_chn_flush_en, 0x1, 32, 0);

	wb_ctrl->wb_clear_config = false;
	DPU_FB_INFO("fb%d clear wb config exit.\n", dpufd->index);
}

void dpufb_ovl_online_wb_config(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_writeback *wb_ctrl = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return;

	dpufb_ovl_online_wb_clear(dpufd);

	if ((g_debug_ovl_online_wb_count <= 0) || (g_debug_ovl_online_wb_count > 10))
		return;

	wb_ctrl = &(dpufd->wb_ctrl);
	mutex_lock(&(wb_ctrl->wb_ctrl_lock));

	dpufb_ovl_online_wb_init(dpufd);

	(void)dpufb_ovl_online_wb_alloc_buffer(dpufd);

	dpufb_ovl_online_wb_interrupt_clear(dpufd);

	dpufb_ovl_online_wb_pipe_set_reg(dpufd);

	dpufb_ovl_online_wb_mmu_config(dpufd);

	dpufb_ovl_online_wb_set_reg(dpufd);

	schedule_work(&(wb_ctrl->wb_ctrl_work));

	mutex_unlock(&(wb_ctrl->wb_ctrl_lock));

	g_debug_ovl_online_wb_count--;
}

#define ONLINE_WB_TIMEOUT_COUNT 10
static void dpufb_ovl_online_wb_workqueue_set_reg(struct dpu_fb_data_type *dpufd,
	struct dpufb_writeback *wb_ctrl)
{
	uint32_t ovl_idx;
	uint32_t chn_idx;
	char __iomem *wb_base = NULL;
	char __iomem *pipe_sw_wb_base = NULL;
	char __iomem *mctl_ov_ien = NULL;
	char __iomem *mctl_chn_flush_en = NULL;
	char __iomem *mctl_mutex_base = NULL;

	/* step1. pipe sw config */
	pipe_sw_wb_base = dpufd->dss_base + DSS_PIPE_SW_WB_OFFSET;
	set_reg(pipe_sw_wb_base + PIPE_SW_SIG_CTRL, 0x0, 32, 0);
	set_reg(pipe_sw_wb_base + SW_POS_CTRL_SIG_EN, 0x0, 32, 0);
	set_reg(pipe_sw_wb_base + PIPE_SW_DAT_CTRL, 0x0, 32, 0);
	set_reg(pipe_sw_wb_base + SW_POS_CTRL_DAT_EN, 0x0, 32, 0);

	/* step2. wb pack config */
	wb_base = dpufd->dss_base + DSS_WB_OFFSET;
	set_reg(wb_base + WB_REG_DEFAULT, 0x1, 32, 0);
	set_reg(wb_base + WB_REG_DEFAULT, 0x0, 32, 0);

	/* step3. mctl config */
	ovl_idx = wb_ctrl->ovl_idx;
	mctl_mutex_base = dpufd->dss_base + g_dss_module_ovl_base[ovl_idx][MODULE_MCTL_BASE];
	set_reg(mctl_mutex_base + MCTL_CTL_MUTEX_WB, 0x1, 32, 0);
	set_reg(mctl_mutex_base + MCTL_CTL_MUTEX_WCH0, 0x1, 32, 0);

	chn_idx = wb_ctrl->wch_idx;
	mctl_ov_ien = dpufd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_OV_OEN];
	set_reg(mctl_ov_ien, 0x0, 32, 0);

	/* step5. mctl sys config */
	mctl_chn_flush_en = dpufd->dss_base + g_dss_module_base[chn_idx][MODULE_MCTL_CHN_FLUSH_EN];
	set_reg(mctl_chn_flush_en, 0x1, 32, 0);
}

static void dpufb_ovl_online_wb_workqueue_handler(struct work_struct *work)
{
	int count = 0;
	uint32_t isr_s1;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_writeback *wb_ctrl = NULL;

	wb_ctrl = container_of(work, typeof(*wb_ctrl), wb_ctrl_work);
	dpu_check_and_no_retval(!wb_ctrl, ERR, "wb_ctrl is NULL!\n");
	dpufd = wb_ctrl->dpufd;
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL!\n");

	DPU_FB_INFO("fb%d enter.\n", dpufd->index);

	down(&dpufd->blank_sem);

	dpufb_activate_vsync(dpufd);

	isr_s1 = inp32(dpufd->dss_base + GLB_CPU_OFF_INTS);
	while (((isr_s1 & BIT_OFF_WCH0_INTS) != BIT_OFF_WCH0_INTS) && (count < ONLINE_WB_TIMEOUT_COUNT)) {
		msleep(10);  /* 10ms */
		isr_s1 = inp32(dpufd->dss_base + GLB_CPU_OFF_INTS);
		DPU_FB_INFO("GLB_CPU_OFF_INTS: 0x%x. count = %d\n", isr_s1, count);
		count++;
	}
	if (count == ONLINE_WB_TIMEOUT_COUNT) {
		DPU_FB_INFO("GLB_CPU_OFF_INTS: 0x%x. timeout\n", isr_s1);
		up(&dpufd->blank_sem);
		return;
	}
	outp32(dpufd->dss_base + GLB_CPU_OFF_INTS, BIT_OFF_WCH0_INTS);

	dpufb_ovl_online_wb_workqueue_set_reg(dpufd, wb_ctrl);

	single_frame_update(dpufd);

	mdelay(16);   /* delay 16ms */

	__flush_dcache_area(wb_ctrl->wb_buffer_base, wb_ctrl->wb_buffer_size);

	dpufb_ovl_online_wb_save_buffer(dpufd);

	dpufb_ovl_online_wb_free_buffer(dpufd);

	dpufb_deactivate_vsync(dpufd);

	wb_ctrl->wb_clear_config = true;

	up(&dpufd->blank_sem);

	DPU_FB_INFO("fb%d exit.\n", dpufd->index);
}

void dpufb_ovl_online_wb_register(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_writeback *wb_ctrl = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return;
	}

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (dpufd->index != PRIMARY_PANEL_IDX) {
		DPU_FB_INFO("fb%d not support.\n", dpufd->index);
		return;
	}

	wb_ctrl = &(dpufd->wb_ctrl);
	if (wb_ctrl->online_wb_created)
		return;

	DPU_FB_INFO("fb%d enter.\n", dpufd->index);

	wb_ctrl->dpufd = dpufd;
	wb_ctrl->afbc_enable = false;
	wb_ctrl->mmu_enable = false;
	wb_ctrl->compress_enable = false;
	wb_ctrl->wb_clear_config = false;
	wb_ctrl->wb_buffer_base = 0;

	wb_ctrl->buffer_alloced = false;
	wb_ctrl->ovl_idx = DSS_OVL0;
	wb_ctrl->wch_idx = DSS_WCHN_W0;  /* only wch0 support */
	wb_ctrl->wdma_format = DMA_PIXEL_FORMAT_ARGB_8888;  /* BGRX ok */

	mutex_init(&(wb_ctrl->wb_ctrl_lock));

	INIT_WORK(&wb_ctrl->wb_ctrl_work, dpufb_ovl_online_wb_workqueue_handler);
	dpufb_ovl_online_wb_init(dpufd);

	wb_ctrl->wb_init = dpufb_ovl_online_wb_init;
	wb_ctrl->wb_alloc_buffer = dpufb_ovl_online_wb_alloc_buffer;
	wb_ctrl->wb_free_buffer = dpufb_ovl_online_wb_free_buffer;
	wb_ctrl->wb_save_buffer = dpufb_ovl_online_wb_save_buffer;

	wb_ctrl->online_wb_created = 1;

	DPU_FB_INFO("fb%d exit.\n", dpufd->index);
}

void dpufb_ovl_online_wb_unregister(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_writeback *wb_ctrl = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return;
	}

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	wb_ctrl = &(dpufd->wb_ctrl);

	if (!wb_ctrl->online_wb_created)
		return;

	dpufb_ovl_online_wb_free_buffer(dpufd);

	wb_ctrl->online_wb_created = 0;
}

