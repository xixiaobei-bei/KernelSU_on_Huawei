/*
 * Copyright (c) 2018-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
/*lint -e86 -e529 -e570 -e571 -e578*/
#include <securec.h>

#include <linux/bitops.h>
#include <linux/clk.h>
#include <video/display_timing.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/iommu.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/component.h>
#include <linux/suspend.h>
#include <linux/semaphore.h>

#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_fb_cma_helper.h>

#include "hisi_drm_drv.h"
#include "hisi_drm_dpe_utils.h"
#include "hisi_dpe.h"
#include "hisi_mipi_dsi.h"

#include "hisi_dss_iommu.h"
#include "hisi_dp_drv.h"
#include "hisi_drm_debug.h"
#include "hisi_mmbuf_manager.h"
#ifdef CONFIG_HISI_DSS_V600
#include "bridge/edp_bridge.h"
#include "hisi_drm_smmuv3.h"
#endif

#define WQ_NAME_LEN 128

static const struct dss_format g_dss_formats[] = {
	/* 16bpp RGB: */
	{ DRM_FORMAT_RGB565, HISI_DSS_FORMAT_RGB565 },
	{ DRM_FORMAT_BGR565, HISI_DSS_FORMAT_BGR565 },
	/* 32bpp [A]RGB: */
	{ DRM_FORMAT_XRGB8888, HISI_DSS_FORMAT_BGRX8888 },
	{ DRM_FORMAT_XBGR8888, HISI_DSS_FORMAT_RGBX8888 },
	{ DRM_FORMAT_RGBX8888, HISI_DSS_FORMAT_RGBX8888 },
	{ DRM_FORMAT_RGBA8888, HISI_DSS_FORMAT_RGBA8888 },
	{ DRM_FORMAT_BGRA8888, HISI_DSS_FORMAT_BGRA8888 },
	{ DRM_FORMAT_YVU420, HISI_DSS_FORMAT_YCRCB420_P },
	{ DRM_FORMAT_NV16, HISI_DSS_FORMAT_YCBCR422_SP },
	{ DRM_FORMAT_NV21, HISI_DSS_FORMAT_YCRCB420_SP },
	{ DRM_FORMAT_YUV422, HISI_DSS_FORMAT_YUV422_I },
	{ DRM_FORMAT_NV12, HISI_DSS_FORMAT_YCBCR422_SP },
	{ DRM_FORMAT_YUV420, HISI_DSS_FORMAT_YCBCR422_P },
	{ DRM_FORMAT_RGBA1010102, HISI_DSS_FORMAT_RGBA1010102 },
};

static const struct dss_format g_afbc_formats[] = {
	/* Debug with GPU (HISI_DSS_FORMAT_RGBX8888) */
	{ DRM_FORMAT_XRGB8888, HISI_DSS_FORMAT_RGBA8888 },
	{ DRM_FORMAT_RGBA8888, HISI_DSS_FORMAT_RGBA8888 },
};

struct isr_param {
	u32 isr_s1;
	u32 isr_s2;
	u32 isr_s3;
	u32 isr_s2_smmu;
};

static void unflow_mipi_ldi_int_handler(char __iomem *mipi_ldi_base, bool unmask)
{
	u32 tmp = 0;

	tmp = inp32(mipi_ldi_base + LDI_CPU_ITF_INT_MSK);
	if (unmask)
		tmp &= ~BIT_LDI_UNFLOW;
	else
		tmp |= BIT_LDI_UNFLOW;

	outp32(mipi_ldi_base + LDI_CPU_ITF_INT_MSK, tmp);
}

static void hisi_dss_unflow_handler(struct hisi_drm_crtc *hisi_crtc, bool unmask)
{
	struct dss_hw_ctx *ctx = NULL;
	uint32_t tmp = 0;

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}
	if (is_dp(hisi_crtc)) {
		tmp = inp32(ctx->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK);
		if (unmask)
			tmp &= ~BIT_LDI_UNFLOW;
		else
			tmp |= BIT_LDI_UNFLOW;
		outp32(ctx->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK, tmp);

		tmp = inp32(ctx->dss_base + DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INT_MSK);
		if (unmask)
			tmp &= ~BIT_LDI_UNFLOW;
		else
			tmp |= BIT_LDI_UNFLOW;
		outp32(ctx->dss_base + DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INT_MSK, tmp);
	} else {
		unflow_mipi_ldi_int_handler(ctx->dss_base + DSS_MIPI_DSI0_OFFSET, unmask);
	}
}

static void dp_underflow_clear_handler(struct dss_hw_ctx *ctx, struct hisi_drm_crtc *hisi_crtc)
{
	u32 mask;
	u32 dpp_dbg;
	char __iomem *ldi_base = NULL;

	dpp_dbg = inp32(ctx->dss_base + DSS_DPP_OFFSET + DPP_DBG_CNT);
	HISI_DRM_ERR("dp underflow dpp_dbg = 0x%x\n", dpp_dbg);

	ldi_base = ctx->dss_base + DSS_LDI_DP_OFFSET;
	hisi_dump_current_info(hisi_crtc);

	mask = inp32(ldi_base + LDI_CPU_ITF_INT_MSK);
	mask |= BIT_LDI_UNFLOW;
	outp32(ldi_base + LDI_CPU_ITF_INT_MSK, mask);
	if (hisi_crtc->ldi_underflow_wq) {
		crtc_disable_ldi(hisi_crtc);
		queue_work(hisi_crtc->ldi_underflow_wq, &hisi_crtc->ldi_underflow_work);
	}
}

static int dpe_irq_enable(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = hisi_crtc->ctx;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return -EINVAL;
	}

	if (hisi_crtc->crtc_id == EXTERNAL_IDX && ctx->dp_irq)
		enable_irq(ctx->dp_irq);

	return 0;
}

static int dpe_irq_disable(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = hisi_crtc->ctx;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return -EINVAL;
	}

	if (hisi_crtc->crtc_id == EXTERNAL_IDX && ctx->dp_irq)
		disable_irq(ctx->dp_irq);

	return 0;
}

irqreturn_t dss_sdp_isr_dp(int irq, void *ptr)
{
	char __iomem *ldi_base = NULL;
	char __iomem *ldi_base1 = NULL;
	struct isr_param isr_info;
	struct dss_data *dss = NULL;
	struct dss_hw_ctx *ctx = NULL;
	struct hisi_drm_crtc *hisi_crtc = NULL;

	dss = (struct dss_data *)ptr;
	if (!dss) {
		HISI_DRM_ERR("dss data is NULL");
		return IRQ_NONE;
	}

	ctx = &dss->ctx;
	if (!ctx) {
		HISI_DRM_ERR("dss ctx is NULL");
		return IRQ_NONE;
	}

	hisi_crtc = &dss->hisi_crtc[EXTERNAL_IDX];

	ldi_base = ctx->dss_base + DSS_LDI_DP_OFFSET;
	ldi_base1 = ctx->dss_base + DSS_LDI_DP1_OFFSET;

	isr_info.isr_s1 = inp32(ctx->dss_base + GLB_CPU_DP_INTS);
	isr_info.isr_s2 = inp32(ldi_base + LDI_CPU_ITF_INTS);
	isr_info.isr_s3 = inp32(ldi_base1 + LDI_CPU_ITF_INTS);
	isr_info.isr_s2_smmu = inp32(ctx->dss_base + DSS_SMMU_OFFSET + SMMU_INTSTAT_NS);

	outp32(ctx->dss_base + DSS_SMMU_OFFSET + SMMU_INTCLR_NS, isr_info.isr_s2_smmu);
	outp32(ldi_base + LDI_CPU_ITF_INTS, isr_info.isr_s2);
	outp32(ldi_base1 + LDI_CPU_ITF_INTS, isr_info.isr_s3);
	outp32(ctx->dss_base + GLB_CPU_DP_INTS, isr_info.isr_s1);

	isr_info.isr_s1 &= ~((u32)inp32(ctx->dss_base + GLB_CPU_DP_INT_MSK));
	isr_info.isr_s2 &= ~((u32)inp32(ldi_base + LDI_CPU_ITF_INT_MSK));

	if (isr_info.isr_s2 & BIT_VACTIVE0_START) {
		hisi_crtc->vactive0_start_flag++;
		wake_up_interruptible_all(&hisi_crtc->vactive0_start_wq);
	}

	if (isr_info.isr_s2 & BIT_VSYNC) {
		hisi_crtc->vsync_ctrl.vsync_timestamp = ktime_get();
		drm_crtc_handle_vblank(&hisi_crtc->base);
	}

	if (isr_info.isr_s2 & BIT_LDI_UNFLOW)
		dp_underflow_clear_handler(ctx, hisi_crtc);

	return IRQ_HANDLED;
}

/* convert from fourcc format to dss format */
u32 dss_get_format(uint32_t pixel_format, uint64_t modifier)
{
	int formats_size = 0;
	struct dss_format *formats = NULL;
	int i;

	if (modifier == AFBC_FORMAT_MODIFIER) {
		formats = (struct dss_format *)g_afbc_formats;
		formats_size = ARRAY_SIZE(g_afbc_formats);
	} else {
		formats = (struct dss_format *)g_dss_formats;
		formats_size = ARRAY_SIZE(g_dss_formats);
	}

	for (i = 0; i < formats_size; i++) {
		if (formats[i].pixel_format == pixel_format)
			return formats[i].dss_format;
	}

	/* not found */
	HISI_DRM_ERR("Not found pixel format!!fourcc_format= %d\n",
		     pixel_format);
	return HISI_DSS_FORMAT_UNSUPPORT;
}

static void hisi_crtc_handle_event(struct drm_crtc *crtc)
{
	struct drm_pending_vblank_event *event = crtc->state->event;

	if (event) {
		crtc->state->event = NULL;
		spin_lock_irq(&crtc->dev->event_lock);
		if (drm_crtc_vblank_get(crtc) == 0)
			drm_crtc_arm_vblank_event(crtc, event);
		else
			drm_crtc_send_vblank_event(crtc, event);

		spin_unlock_irq(&crtc->dev->event_lock);
	} else {
		HISI_DRM_INFO("event is NULL");
	}
}

static int dss_enable_vblank(struct drm_device *dev, unsigned int pipe)
{
	return 0;
}

static void dss_disable_vblank(struct drm_device *dev, unsigned int pipe)
{
}

static void dpe_set_fastboot(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	dss_inner_clk_common_enable(hisi_crtc);

	dpe_init(hisi_crtc);
	dpe_interrupt_mask(hisi_crtc);
	dpe_interrupt_clear(hisi_crtc);
	dpe_irq_enable(hisi_crtc);
	dpe_interrupt_unmask(hisi_crtc);
}

void dpe_on(struct hisi_drm_crtc *hisi_crtc)
{
	int ret;
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	ret = dpe_regulator_enable(ctx, DPE_REGULATOR_MEDIA);
	if (ret) {
		HISI_DRM_ERR("DPE_REGULATOR_MEDIA, error=%d", ret);
		return;
	}

	ret = dpe_set_clk_rate(hisi_crtc); /* edc pxl */
	if (ret) {
		HISI_DRM_ERR("dpe_set_clk_rate, error=%d", ret);
		return;
	}

	ret = dpe_common_clk_enable(ctx);
	if (ret) {
		HISI_DRM_ERR("dpe_common_clk_enable, error=%d", ret);
		return;
	}

	ret = dpe_inner_clk_enable(hisi_crtc);
	if (ret) {
		HISI_DRM_ERR("dpe_inner_clk_enable, error=%d", ret);
		return;
	}

	ret = dpe_pdp_regulator_enable(hisi_crtc);
	if (ret) {
		HISI_DRM_ERR("dpe_regulator_enable, error=%d", ret);
		return;
	}

	if (hisi_set_mmbuf_clk_rate(ctx) < 0) {
		HISI_DRM_ERR("reset_mmbuf_clk_rate failed !\n");
		return;
	}

	dss_inner_clk_common_enable(hisi_crtc);
	if (hisi_crtc->crtc_id == EXTERNAL_IDX)
		dss_inner_clk_sdp_enable(hisi_crtc);

	dpe_init(hisi_crtc);

	dpe_interrupt_mask(hisi_crtc);
	dpe_interrupt_clear(hisi_crtc);
	dpe_irq_enable(hisi_crtc);
	dpe_interrupt_unmask(hisi_crtc);
}

static void hisi_drm_crtc_atomic_enable(struct drm_crtc *crtc,
					struct drm_crtc_state *old_state)
{
	struct hisi_drm_crtc *hisi_crtc = to_hisi_drm_crtc(crtc);
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return;
	}

	HISI_DRM_INFO("crtc-%d +", hisi_crtc->crtc_id);

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	if (hisi_crtc->power_on) {
		HISI_DRM_ERR("crtc-%d, already on!\n",
			     hisi_crtc->crtc_id);
		return;
	}

	if (ctx->is_pre_enabled)
		dpe_set_fastboot(hisi_crtc);
	else
		dpe_on(hisi_crtc);

	hisi_overlay_on(hisi_crtc);
	hisi_crtc->power_on = true;
	ctx->is_pre_enabled = NEED_PRE_ENABLE_NEXT;
	hisi_crtc->frame_no = 0;

	drm_crtc_vblank_on(crtc);

	HISI_DRM_INFO("crtc-%d -", hisi_crtc->crtc_id);
}

void dpe_off(struct hisi_drm_crtc *hisi_crtc)
{
	dpe_interrupt_mask(hisi_crtc);
	dpe_irq_disable(hisi_crtc);
	dpe_deinit(hisi_crtc);
	dpe_check_itf_status(hisi_crtc);

	if (hisi_crtc->crtc_id == EXTERNAL_IDX)
		dss_inner_clk_sdp_disable(hisi_crtc);
	dss_inner_clk_common_disable(hisi_crtc->ctx);

#ifdef CONFIG_DRM_SMMU_V3
	/* smmu tbu must be disconnect before dss power off */
	hisi_drm_smmuv3_off(hisi_crtc);
#endif

	dpe_pdp_regulator_disable(hisi_crtc);
	dpe_inner_clk_disable(hisi_crtc);
	dpe_common_clk_disable(hisi_crtc->ctx);
	dpe_regulator_disable(hisi_crtc->ctx, DPE_REGULATOR_MEDIA);
}

static void hisi_drm_crtc_atomic_disable(struct drm_crtc *crtc,
					 struct drm_crtc_state *old_crtc_state)
{
	struct hisi_drm_crtc *hisi_crtc = NULL;

	if (!crtc) {
		HISI_DRM_ERR("crtc is NULL");
		return;
	}
	UNUSED(old_crtc_state);

	hisi_crtc = to_hisi_drm_crtc(crtc);

	HISI_DRM_INFO("crtc-%d +", hisi_crtc->crtc_id);

	if (!hisi_crtc->power_on) {
		HISI_DRM_INFO("CRTC-%d, already off", hisi_crtc->crtc_id);
		return;
	}

	drm_crtc_vblank_off(crtc);
	dpe_off(hisi_crtc);
	hisi_dss_off(hisi_crtc);
	hisi_crtc_handle_event(crtc);

	hisi_crtc->power_on = false;

	HISI_DRM_INFO("crtc-%d -", hisi_crtc->crtc_id);
}

struct time_param {
	u64 vsync_timediff;
	u64 timestamp;
	u64 prepare_time;
	u64 vsync_time;
	u64 vsync_time_prev;
};

int hisi_check_vsync_timediff(struct hisi_drm_crtc *hisi_crtc)
{
	ktime_t prepare_timestamp;
	struct time_param time_param;
	struct hisi_vsync *vsync_ctrl = NULL;
	struct drm_display_mode *mode = NULL;
	int ret;

	if (!is_mipi_video_mode(hisi_crtc) && !is_dp(hisi_crtc))
		return 0;

	vsync_ctrl = &hisi_crtc->vsync_ctrl;
	mode = &hisi_crtc->base.state->adjusted_mode;
	time_param.timestamp = 2000000;

	time_param.vsync_timediff = (uint64_t)mode->htotal * mode->vtotal *
		1000000UL / mode->clock;

	prepare_timestamp = ktime_get();
	time_param.prepare_time = ktime_to_ns(prepare_timestamp);
	time_param.vsync_time = ktime_to_ns(vsync_ctrl->vsync_timestamp);
	time_param.vsync_time_prev = ktime_to_ns(vsync_ctrl->vsync_timestamp_prev);
	if (time_param.prepare_time > time_param.vsync_time &&
	    ((time_param.prepare_time - time_param.vsync_time) < (time_param.vsync_timediff - time_param.timestamp)) &&
	    time_param.vsync_time_prev != time_param.vsync_time) {
		HISI_DRM_DEBUG_FRAME("CRTC-%d, timestamp_diff=%llu.\n",
				     hisi_crtc->crtc_id,
				     time_param.prepare_time - time_param.vsync_time);
	} else {
		HISI_DRM_DEBUG_FRAME("CRTC-%d, wait vactive0\n",
				     hisi_crtc->crtc_id);
		ret = hisi_vactive0_start_config(hisi_crtc);
		if (ret) {
			HISI_DRM_INFO("CRTC-%d, wait failed! ret=%d\n",
				      hisi_crtc->crtc_id, ret);
			return ret;
		}
	}

	return 0;
}

static void hisi_drm_crtc_atomic_begin(struct drm_crtc *crtc,
				       struct drm_crtc_state *old_state)
{
	struct hisi_drm_crtc *hisi_crtc = to_hisi_drm_crtc(crtc);
	int ret;
	bool enable_cmdlist = hisi_crtc->enable_cmdlist;

	HISI_DRM_DEBUG("CRTC-%d +\n", hisi_crtc->crtc_id);
	HISI_DRM_DEBUG_FRAME("CRTC-%d, frame no:%d\n",
			     hisi_crtc->crtc_id, hisi_crtc->frame_no);

	down(&hisi_crtc->sem);
	if (is_mipi_cmd_mode(hisi_crtc))
		hisi_vactive0_start_config(hisi_crtc);

	ret = hisi_dss_module_init(hisi_crtc);
	if (ret != 0) {
		HISI_DRM_ERR("hisi_dss_module_init failed! ret = %d\n", ret);
		goto err_return;
	}

	hisi_crtc->layer_mask = 0;
	hisi_crtc->has_layer = false;

	if (enable_cmdlist)
		hisi_cmdlist_data_get_online(&hisi_crtc->cmdlist_info);
	else
		hisi_dss_mctl_mutex_lock(hisi_crtc);

err_return:
	up(&hisi_crtc->sem);
	HISI_DRM_DEBUG("CRTC-%d -\n", hisi_crtc->crtc_id);
}

static void hisi_cmdlist_config(struct hisi_drm_crtc *hisi_crtc, struct hisi_cmdlist **cmdlist_info, int ovl_idx)
{
	u32 cmdlist_idxs;
	u32 cmdlist_pre_idxs;

	*cmdlist_info = &hisi_crtc->cmdlist_info;
	cmdlist_idxs = (*cmdlist_info)->cmdlist_idxs_mask;
	cmdlist_pre_idxs = (*cmdlist_info)->cmdlist_prev_idxs_mask;
	/* add taskend for share channel */
	hisi_cmdlist_add_nop_node(*cmdlist_info, cmdlist_idxs, 0, 0);

	/* remove ch cmdlist */
	cmdlist_pre_idxs &= (~cmdlist_idxs);
	hisi_cmdlist_config_stop(hisi_crtc, cmdlist_pre_idxs, ovl_idx);

	cmdlist_idxs |= cmdlist_pre_idxs;
	hisi_cmdlist_flush_cache(*cmdlist_info, cmdlist_idxs);

	if (hisi_cmdlist_config_start(hisi_crtc,
					  hisi_crtc->mclt_idx,
					  cmdlist_idxs, 0)) {
		HISI_DRM_ERR("hisi_cmdlist_config_start error\n");
		return;
	}

	if (g_debug_ovl_cmdlist)
		hisi_cmdlist_dump_all_node(*cmdlist_info, cmdlist_idxs);

}

static void hisi_drm_crtc_atomic_flush(struct drm_crtc *crtc,
				       struct drm_crtc_state *old_state)
{
	struct hisi_drm_crtc *hisi_crtc = NULL;
	bool enable_cmdlist = false;
	struct hisi_cmdlist *cmdlist_info = NULL;
	int ovl_idx;
	int ret;
	UNUSED(old_state);

	if (!crtc) {
		HISI_DRM_ERR("crtc is NULL");
		return;
	}

	hisi_crtc = to_hisi_drm_crtc(crtc);
	enable_cmdlist = hisi_crtc->enable_cmdlist;

	HISI_DRM_DEBUG("CRTC-%d +\n", hisi_crtc->crtc_id);

	down(&hisi_crtc->sem);
	ovl_idx =  hisi_crtc->ovl_idx;
	if (!hisi_crtc->has_layer && hisi_crtc->vactive0_start_flag > 1) {
		HISI_DRM_INFO("CRTC-%d, no layer, skip!", hisi_crtc->crtc_id);
		if (!enable_cmdlist)
			hisi_dss_mctl_mutex_unlock(hisi_crtc);
		hisi_crtc_handle_event(crtc);
		up(&hisi_crtc->sem);
		return;
	}

	ret = hisi_ov_base_config(hisi_crtc);

	if (ret != 0) {
		HISI_DRM_ERR("hisi_ov_base_config failed\n");
		up(&hisi_crtc->sem);
		return;
	}

	hisi_dss_unflow_handler(hisi_crtc, true);

	ret = hisi_check_vsync_timediff(hisi_crtc);
	if (ret) {
		HISI_DRM_ERR("hisi_check_vsync_timediff failed, ret=%d\n", ret);
		up(&hisi_crtc->sem);
		return;
	}

	if (enable_cmdlist)
		hisi_cmdlist_config(hisi_crtc, &cmdlist_info, ovl_idx);
	else
		hisi_dss_mctl_mutex_unlock(hisi_crtc);

	HISI_DRM_DEBUG("INPT 0x028 = 0x%x\n", inp32(hisi_crtc->ctx->pctrl_base + 0x028));
#ifdef CONFIG_DRM_SMMU_V3
	udelay(1000);
	hisi_dss_mmu_tlb_flush(hisi_crtc, hisi_crtc->frame_no);
#endif
	crtc_single_frame_update(hisi_crtc);
	hisi_crtc_handle_event(crtc);

	HISI_DRM_DEBUG_FRAME("CRTC-%d, frame no:%d\n",
			     hisi_crtc->crtc_id, hisi_crtc->frame_no);

	hisi_crtc->vsync_ctrl.vsync_timestamp_prev =
		hisi_crtc->vsync_ctrl.vsync_timestamp;
	hisi_crtc->pre_layer_mask = hisi_crtc->layer_mask;

	hisi_crtc->frame_no++;

	up(&hisi_crtc->sem);
	HISI_DRM_DEBUG("CRTC-%d -\n", hisi_crtc->crtc_id);

}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static int hisi_drm_crtc_atomic_check(struct drm_crtc *crtc,
	struct drm_crtc_state *new_state)
{
	bool has_primary = false;
	u32 primary_plane_mask;

	if (!crtc || !new_state)
		return -EINVAL;

	primary_plane_mask = 1 << drm_plane_index(crtc->primary);
	has_primary = new_state->plane_mask & primary_plane_mask;

	if (has_primary != new_state->enable)
		return -EINVAL;

	return 0;
}
#endif

static const struct drm_crtc_helper_funcs hisi_drm_crtc_helper_funcs = {
	.atomic_enable	= hisi_drm_crtc_atomic_enable,
	.atomic_disable	= hisi_drm_crtc_atomic_disable,
	.atomic_begin	= hisi_drm_crtc_atomic_begin,
	.atomic_flush	= hisi_drm_crtc_atomic_flush,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	.atomic_check	= hisi_drm_crtc_atomic_check,
#endif
};

static const struct drm_crtc_funcs hisi_drm_crtc_funcs = {
	.destroy	= drm_crtc_cleanup,
	.set_config	= drm_atomic_helper_set_config,
	.page_flip	= drm_atomic_helper_page_flip,
	.reset		= drm_atomic_helper_crtc_reset,
	.atomic_duplicate_state	= drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_crtc_destroy_state,
};

static int plane_init_for_drm(struct hisi_drm_plane **plane, struct dss_data *dss,
	struct drm_device *drm_dev)
{
	int ret;

	*plane = &dss->planes[PRIMARY_CH];
	ret = hisi_drm_plane_init(drm_dev, *plane, 1, PRIMARY_CH);
	if (ret) {
		HISI_DRM_ERR("hisi_drm_plane_init ret=%d\n", ret);
		return -1;
	}

	*plane = &dss->planes[EXTERNAL_CH];
	ret = hisi_drm_plane_init(drm_dev, *plane, 2, EXTERNAL_CH);
	if (ret) {
		HISI_DRM_ERR("hisi_drm_plane_init ret=%d\n", ret);
		return -2;
	}
	return 0;
}

static int hisi_drm_crtc_init(struct device *dev,
			      struct drm_device *drm_dev,
				struct drm_crtc *crtc, struct drm_plane *plane)
{
	struct hisi_drm_private *priv = drm_dev->dev_private;
	struct device_node *port = NULL;
	int ret;

	/* set crtc port so that
	 * drm_of_find_possible_crtcs call works
	 */
	port = of_get_child_by_name(dev->of_node, "port");
	if (!port) {
		HISI_DRM_ERR("no port node found in %s\n",
			     dev->of_node->full_name);
		return -EINVAL;
	}
	of_node_put(port);
	crtc->port = port;

	ret = drm_crtc_init_with_planes(drm_dev, crtc, plane, NULL,
					&hisi_drm_crtc_funcs, NULL);
	if (ret) {
		HISI_DRM_ERR("failed to init crtc.\n");
		return ret;
	}

	drm_crtc_helper_add(crtc, &hisi_drm_crtc_helper_funcs);
	priv->crtc[drm_crtc_index(crtc)] = crtc;

	return 0;
}

static enum dss_ovl_idx crtc_id_to_ov(enum crtc_idx crtc_id)
{
	switch (crtc_id) {
	case PRIMARY_IDX:
		return DSS_OVL0;
	case EXTERNAL_IDX:
		return DSS_OVL1;
	default:
		HISI_DRM_ERR("crtc_id %d more than max OVL number %d",
			     crtc_id, DSS_OVL_IDX_MAX);
		return -1;
	}
}

static enum dss_mctl_idx crtc_id_to_ov_mclt(enum crtc_idx crtc_id)
{
	switch (crtc_id) {
	case PRIMARY_IDX:
		return DSS_MCTL0;
	case EXTERNAL_IDX:
		return DSS_MCTL1;
	default:
		HISI_DRM_ERR("crtc_id %d more than max MCTL number %d ",
			     crtc_id, DSS_MCTL_IDX_MAX);
		return -1;
	}
}

static void hisi_ldi_underflow_handle_func(struct work_struct *work)
{
	struct hisi_drm_crtc *hisi_crtc = NULL;

	uint32_t cmdlist_idxs_prev = 0;

	hisi_crtc = container_of(work, struct hisi_drm_crtc, ldi_underflow_work);
	if (hisi_crtc == NULL) {
		HISI_DRM_ERR("hisifd is NULL point!\n");
		return;
	}

	HISI_DRM_INFO("crtc%d, +.\n", hisi_crtc->crtc_id);

	down(&hisi_crtc->sem);
	if (!hisi_crtc->power_on) {
		HISI_DRM_ERR("crtc%d, panel is power off!", hisi_crtc->crtc_id);
		up(&hisi_crtc->sem);
		return;
	}

	cmdlist_idxs_prev |= (1 << PRIMARY_CH);
	cmdlist_idxs_prev |= (1 << (uint32_t)(DSS_CMDLIST_OV0 + hisi_crtc->ovl_idx));

	hisi_cmdlist_config_reset(hisi_crtc, cmdlist_idxs_prev, hisi_crtc->ovl_idx);
	up(&hisi_crtc->sem);

	HISI_DRM_INFO("crtc%d, -.\n", hisi_crtc->crtc_id);
	return;

}

static int hisi_crtc_init(struct device *dev,
			  struct drm_device *drm_dev,
				struct hisi_drm_crtc *hisi_crtc,
				int crtc_id)
{
	int ret;
	struct dss_data *dss = dev_get_drvdata(dev);
	struct drm_plane *plane = NULL;
	char wq_name[WQ_NAME_LEN] = {0};

	hisi_crtc->underflow_flag = false;
	hisi_crtc->frame_update_flag = 0;
	hisi_crtc->enable_cmdlist = false;
	hisi_crtc->ovl_idx = crtc_id_to_ov(crtc_id);
	hisi_crtc->mclt_idx = crtc_id_to_ov_mclt(crtc_id);
	hisi_crtc->crtc_id = crtc_id;
	hisi_crtc->ctx = &dss->ctx;
	hisi_crtc->power_on = false;
	hisi_crtc->ldi_underflow_wq = NULL;
	hisi_crtc->vsync_ctrl.vsync_timestamp = ktime_get();

	init_waitqueue_head(&hisi_crtc->vactive0_start_wq);
	hisi_crtc->vactive0_start_flag = 0;

	if (crtc_id == PRIMARY_IDX) {
		plane = &dss->planes[PRIMARY_CH].base;
		hisi_crtc->encoder_type = PANEL_MIPI_VIDEO;

	} else if (crtc_id == EXTERNAL_IDX) {
		plane = &dss->planes[EXTERNAL_CH].base;
		hisi_crtc->encoder_type = PANEL_DP;
	}

	ret = snprintf_s(wq_name, WQ_NAME_LEN, WQ_NAME_LEN - 1, "crtc%d_ldi_underflow", crtc_id);
	hisi_crtc->ldi_underflow_wq = create_singlethread_workqueue(wq_name);
	if (hisi_crtc->ldi_underflow_wq == NULL) {
		HISI_DRM_ERR("crtc%d, create ldi underflow workqueue failed!\n", crtc_id);
		return -EINVAL;
	}
	INIT_WORK(&hisi_crtc->ldi_underflow_work, hisi_ldi_underflow_handle_func);

	ret = hisi_drm_crtc_init(dev, drm_dev, &hisi_crtc->base, plane);
	if (ret) {
		HISI_DRM_ERR("hisi_drm_crtc_init FAIL. ret=%d\n", ret);
		return ret;
	}

	return 0;
}

static int crtc_init(struct hisi_drm_crtc **hisi_crtc_primary, struct hisi_drm_crtc **hisi_crtc_external,
	struct dss_data *dss, struct device *dev, struct drm_device *drm_dev)
{
	int ret;

	*hisi_crtc_primary = &dss->hisi_crtc[PRIMARY_IDX];
	*hisi_crtc_external = &dss->hisi_crtc[EXTERNAL_IDX];
	ret = hisi_crtc_init(dev, drm_dev, *hisi_crtc_primary, PRIMARY_IDX);
	if (ret) {
		HISI_DRM_ERR("init crtc_primary fail. ret=%d\n", ret);
		return ret;
	}

	ret = hisi_crtc_init(dev, drm_dev, *hisi_crtc_external, EXTERNAL_IDX);
	if (ret) {
		HISI_DRM_ERR("init crtc_external fail. ret=%d\n", ret);
		return ret;
	}
	return 0;
}

static int hisi_dpe_bind(struct device *dev,
			 struct device *master, void *data)
{
	struct drm_device *drm_dev = data;
	struct hisi_drm_private *drm_priv = drm_dev->dev_private;
	struct dss_data *dss = dev_get_drvdata(dev);
	struct hisi_drm_crtc *hisi_crtc_primary = NULL;
	struct hisi_drm_crtc *hisi_crtc_external = NULL;
	struct hisi_drm_plane *plane = NULL;
	int ret;

	HISI_DRM_INFO("+");

	if (!dev) {
		HISI_DRM_ERR("dev is nullptr!\n");
		goto err_out;
	}
	if (!data) {
		HISI_DRM_ERR("data is nullptr!\n");
		goto err_out;
	}
	if (!dss) {
		HISI_DRM_ERR("dss is nullptr!\n");
		goto err_out;
	}
	ret = hisi_dss_mmbuf_init();
	if (ret) {
		HISI_DRM_ERR("hisi_dss_mmbuf_init ret=%d\n", ret);
		goto err_out;
	}

	ret = plane_init_for_drm(&plane, dss, drm_dev);
	if (ret == -1)
		goto err_deinit_mmbuf_out;
	if (ret == -2)
		goto err_deinit_primary_out;

	ret = crtc_init(&hisi_crtc_primary, &hisi_crtc_external, dss, dev, drm_dev);
	if (ret)
		goto err_deinit_external_out;
#ifdef CONFIG_DRM_DSS_IOMMU
	ret = hisi_dss_iommu_enable(dev);
	if (ret) {
		HISI_DRM_ERR("hisi_dss_iommu_enable failed, ret=%d\n", ret);
		goto err_deinit_external_out;
	}

	drm_priv->iommu_dev = dev;
#endif
	dpe_regulator_clk_irq_setup(dev, hisi_crtc_primary);
	hisi_dss_module_default(hisi_crtc_primary);
	hisi_dss_module_default(hisi_crtc_external);

	drm_dev->driver->get_vblank_counter = NULL;
	drm_dev->driver->enable_vblank = dss_enable_vblank;
	drm_dev->driver->disable_vblank = dss_disable_vblank;

	/* disable vt switch during suspend and resume */
	pm_set_vt_switch(0);

	HISI_DRM_INFO("-");

	return 0;

err_deinit_external_out:
	hisi_drm_plane_deinit(&dss->planes[EXTERNAL_CH]);
err_deinit_primary_out:
	hisi_drm_plane_deinit(&dss->planes[PRIMARY_CH]);
err_deinit_mmbuf_out:
	hisi_dss_mmbuf_deinit();
err_out:
	return ret;
}

static void hisi_dpe_unbind(struct device *dev, struct device *master,
			    void *data)
{
	struct dss_data *dss = NULL;
	struct drm_crtc *crtc = NULL;
	int i;

	UNUSED(master);
	UNUSED(data);

	if (!dev) {
		HISI_DRM_ERR("dev is nullptr!\n");
		return;
	}

	dss = dev_get_drvdata(dev);

	for (i = 0; i < CRTC_MAX_IDX; i++) {
		crtc = &dss->hisi_crtc[i].base;
		drm_crtc_cleanup(crtc);
	}

	hisi_drm_plane_deinit(&dss->planes[PRIMARY_CH]);

	hisi_drm_plane_deinit(&dss->planes[EXTERNAL_CH]);

	hisi_dss_mmbuf_deinit();
}

static const struct component_ops dpe_component_ops = {
	.bind = hisi_dpe_bind,
	.unbind = hisi_dpe_unbind,
};

#ifdef CONFIG_HISI_DSS_V600
struct device *g_smmu_dev = NULL;
#endif

static int hisi_dpe_probe(struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct dss_data *dss = NULL;
	struct dss_hw_ctx *ctx = NULL;
	int ret;

	if (!pdev) {
		HISI_DRM_ERR("pdev is nullptr!\n");
		return -EINVAL;
	}

	dev = &pdev->dev;
#ifdef CONFIG_HISI_DSS_V600
	g_smmu_dev = &pdev->dev;
#endif
	dss = devm_kzalloc(dev, sizeof(*dss), GFP_KERNEL);
	if (!dss) {
		HISI_DRM_ERR("failed to alloc dss_data\n");
		return -ENOMEM;
	}

	ctx = &dss->ctx;
	ctx->pdev = pdev;

	ret = dpe_dts_parse(pdev, ctx);
	if (ret) {
		devm_kfree(dev, dss);
		HISI_DRM_ERR("dss_dts_parse error, ret=%d\n", ret);
		return ret;
	}

	ctx->dss_inner_clk_refcount = 0;
	ctx->dss_regulator_refcount = 0;
	ctx->is_pre_enabled = PRE_ENABLED_IN_UEFI;

	ctx->dpe_regulator[DPE_REGULATOR_PDP].supply = REGULATOR_PDP_NAME;
	ctx->dpe_regulator[DPE_REGULATOR_MMBUF].supply = REGULATOR_MMBUF;
	ctx->dpe_regulator[DPE_REGULATOR_MEDIA].supply = REGULATOR_MEDIA_NAME;
#ifdef CONFIG_HISI_DSS_V600
	ctx->dpe_regulator[DPE_REGULATOR_SMMU].supply = REGULATOR_SMMU_TCU_NAME;
#endif
	ret = devm_regulator_bulk_get(&pdev->dev, DPE_REGULATER_NUM, ctx->dpe_regulator);
	if (ret) {
		devm_kfree(dev, dss);
		dev_err(dev, "failed to get regulator! ret=%d.\n", ret);
		return -ENXIO;
	}

	ret = request_irq(ctx->dp_irq, dss_sdp_isr_dp, 0, "irq_sdp", dss);
	if (ret) {
		devm_kfree(dev, dss);
		HISI_DRM_ERR("fail to request sdp irq, ret=%d!", ret);
		return ret;
	}
	disable_irq(ctx->dp_irq);

	platform_set_drvdata(pdev, dss);

	ret = component_add(&pdev->dev, &dpe_component_ops);

	return ret;
}

static int hisi_dpe_remove(struct platform_device *pdev)
{
	if (!pdev) {
		HISI_DRM_ERR("pdev is nullptr!\n");
		return -EINVAL;
	}

	component_del(&pdev->dev, &dpe_component_ops);

	return 0;
}

static const struct of_device_id hisi_dpe_match_table[] = {
	{
		.compatible = DTS_COMP_DSS_NAME,
		.data = NULL,
	},
	{},
};

struct platform_driver hisi_dpe_driver = {
	.probe = hisi_dpe_probe,
	.remove = hisi_dpe_remove,
	.driver = {
		.name = "hisi-dpe",
		.owner = THIS_MODULE,
		.of_match_table = hisi_dpe_match_table,
	},
};

/*lint +e86 +e529 +e570 +e571 +e578*/
