/*
 * Copyright (c) 2016 Linaro Limited.
 * Copyright (c) 2014-2020 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef HISI_DRM_DRV_H
#define HISI_DRM_DRV_H

#include <drm/drmP.h>
#include <linux/iommu.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
#include <linux/ion.h>
#endif
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#include <linux/timer.h>

#include "drm_crtc.h"
#include "drm_fb_helper.h"

#include "hisi_dpe.h"
#include "hisi_overlay_cmdlist_utils.h"
#include "hisi_overlay_utils.h"

#define MAX_PIXEL_WIDE 4096
#define MAX_PIXEL_HEIGHT 2160
#define MMBUF_BPP 4

/* MmbufSize = 4096(width) * 4(bpp) * 8(mmbuf_line_num) */
#define MMBUFFER_SIZE_PER_CHANNEL (MAX_PIXEL_WIDE * MMBUF_BPP * MMBUF_LINE_NUM)


enum crtc_idx {
	PRIMARY_IDX = 0,
	EXTERNAL_IDX,
	CRTC_MAX_IDX,
};

struct dss_hw_ctx {
	int index;

	char __iomem *dss_base;
	char __iomem *peri_crg_base;
	char __iomem *sctrl_base;
	char __iomem *pctrl_base;
	char __iomem *noc_dss_base;
	char __iomem *mmbuf_crg_base;
	char __iomem *mmbuf_asc0_base;
	char __iomem *pmctrl_base;

	char __iomem *media_crg_base;
	char __iomem *media_common_base;
	uint32_t dss_base_phy;

	struct regmap *noc_regmap;
	struct reset_control *reset;

	struct regulator_bulk_data dpe_regulator[DPE_REGULATER_NUM];

	char *dpe_clk_name[DPE_CLK_NUM];
	struct clk *dpe_clk[DPE_CLK_NUM];

	uint32_t dp_irq;
	uint32_t mmbuf_asc0_irq;

	bool is_pre_enabled;
	int dss_sr_refcnt;

	uint32_t dss_regulator_refcount;
	uint32_t dss_inner_clk_refcount;

	struct platform_device *pdev;
};

struct hisi_drm_crtc {
	struct drm_crtc base;
	struct dss_hw_ctx *ctx;
	int crtc_id;
	enum dss_ovl_idx ovl_idx;
	int mclt_idx;
	uint32_t layer_mask;
	uint32_t pre_layer_mask;
	bool has_layer;
	struct dss_vote_cmd dss_vote_cmd;

	struct dss_module_reg dss_module;
	struct dss_module_reg dss_module_default;

	bool enable_cmdlist;
	struct hisi_cmdlist cmdlist_info;

	wait_queue_head_t vactive0_start_wq;
	uint32_t vactive0_start_flag;
	struct hisi_vsync vsync_ctrl;
	uint32_t frame_no;
	bool power_on;

	bool underflow_flag;
	struct workqueue_struct *ldi_underflow_wq;
	struct work_struct ldi_underflow_work;
	uint32_t encoder_type;
	struct semaphore sem;

	uint32_t frame_update_flag;
};

struct hisi_drm_plane_state {
	struct drm_plane_state base;
	uint32_t transform;
	int32_t blending;
	uint32_t alpha;
	uint32_t color;
	uint32_t need_cap;
};

struct hisi_drm_plane {
	struct drm_plane base;
	u8 chn;
	char *name;
	uint32_t caps;
	uint32_t mmbuffer_addr;

	struct {
		struct drm_property *alpha;
		struct drm_property *blending;
		struct drm_property *color;
		struct drm_property *transform;
		struct drm_property *need_cap;
		struct drm_property *caps;
	} props;
};

struct dss_data {
	struct hisi_drm_crtc hisi_crtc[CRTC_MAX_IDX];
	struct hisi_drm_plane planes[DSS_ONLINE_CHN_MAX];
	struct dss_hw_ctx ctx;
};

struct dss_format {
	u32 pixel_format;
	enum hisi_dss_format dss_format;
};

struct hisi_drm_private {
	struct drm_fb_helper *fb_helper;

	struct drm_crtc *crtc[CRTC_MAX_IDX];
	struct device *iommu_dev;

	struct drm_atomic_state *suspend_state;
	bool suspend;
	bool need_poll_change;
	struct mutex suspend_mutex;
};

struct hisi_fbdev {
	struct drm_fb_helper fb_helper;
	struct drm_framebuffer *fb;

	struct sg_table *sgt;
	void *screen_base;
	unsigned long smem_start;
	unsigned long screen_size;
	int shared_fd;
};

#define to_hisi_drm_crtc(crtc) \
	container_of(crtc, struct hisi_drm_crtc, base)

#define to_hisi_drm_plane(plane) \
	container_of(plane, struct hisi_drm_plane, base)

#define to_hisi_plane_state(state) \
	container_of(state, struct hisi_drm_plane_state, base)

#define to_hisi_fbdev(x) \
	container_of(x, struct hisi_fbdev, fb_helper)

struct drm_framebuffer *hisi_framebuffer_init(
	struct drm_device *dev,
	struct drm_mode_fb_cmd2 *mode_cmd);

struct drm_fb_helper *hisi_drm_fbdev_init(
	struct drm_device *dev);

void hisi_drm_fbdev_fini(
	struct drm_fb_helper *helper);

int hisi_drm_plane_init(
	struct drm_device *dev,
	struct hisi_drm_plane *hisi_plane,
	uint32_t possible_crtcs,
	enum dss_chn_idx chn_idx);

void hisi_drm_plane_deinit(
	struct hisi_drm_plane *hisi_plane);

irqreturn_t dss_dsi0_isr(int irq, void *ptr);

extern struct platform_driver hisi_dpe_driver;
extern struct platform_driver hisi_mipi_dsi_driver;
extern struct platform_driver hisi_dp_driver;

static inline struct device *to_dma_dev(struct drm_device *dev)
{
	struct hisi_drm_private *priv = dev->dev_private;

	return priv->iommu_dev;
}
#endif /* HISI_DRM_DRV_H */
