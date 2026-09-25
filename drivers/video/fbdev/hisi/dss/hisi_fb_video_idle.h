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
#ifndef HISI_FB_VIDEO_IDLE_H
#define HISI_FB_VIDEO_IDLE_H
#ifdef CONFIG_HISI_L3CACHE_SHARE
#include <linux/hisi/hisi_l3share.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
#include <linux/ion-iommu.h>
#endif
#include "hisi_fb.h"
#include <linux/hisi/hisi_lb.h>

#define CACHE_WB_SIZE (1080*2244*4)
extern uint32_t g_dss_chn_sid_num[DSS_CHN_MAX_DEFINE];
extern uint32_t g_dss_smmu_smrx_idx[DSS_CHN_MAX_DEFINE];

/*
 * V510 and V600
 * request_size=1 map to 1M L3
 * request_size=2 map to 2M L3
 * request_size=3 map to 3M L3
 * request_size=4 map to 4M L3
 *
 * V350
 * request_size=1 map to 0.5M L3
 * request_size=2 map to 1M L3
 * request_size=3 map to 1.5M L3
 * request_size=4 map to 2M L3
*/
// V510
#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
#ifdef CONFIG_HISI_LB
#define DISPLAY_LB_POLICY_ID	ION_LB_IDISPLAY // 4
#define LB_AVAILABLE_SIZE	(0x380000) // 3.5M
#else
#define DISPLAY_LB_POLICY_ID	(0x0) // ION_LB_BYPASS
#define LB_AVAILABLE_SIZE	(0x0)
#endif
#ifdef CONFIG_HISI_L3CACHE_SHARE
#ifdef CONFIG_HISI_FB_V600
#define L3CACHE_AVAILABLE_SIZE	(0x300000) // 3M
#define L3CACHE_REQUEST_SIZE	3
#else
#define L3CACHE_AVAILABLE_SIZE	(0x0)
#define L3CACHE_REQUEST_SIZE	0
#endif
#else
#define L3CACHE_AVAILABLE_SIZE	(0x0)
#define L3CACHE_REQUEST_SIZE	0
#endif
// V350
#elif defined(CONFIG_HISI_FB_V350)
#define DISPLAY_LB_POLICY_ID	(0x0)
#define LB_AVAILABLE_SIZE	(0x0)
#ifdef CONFIG_HISI_L3CACHE_SHARE
#define L3CACHE_AVAILABLE_SIZE	(0x200000) // 2M
#define L3CACHE_REQUEST_SIZE	4
#else
#define L3CACHE_AVAILABLE_SIZE	(0x0)
#define L3CACHE_REQUEST_SIZE	0
#endif
// others
#else
#define DISPLAY_LB_POLICY_ID	(0x0)
#define LB_AVAILABLE_SIZE	(0)
#ifdef CONFIG_HISI_L3CACHE_SHARE
#define L3CACHE_AVAILABLE_SIZE	(0x300000)
#define L3CACHE_REQUEST_SIZE	3
#else
#define L3CACHE_AVAILABLE_SIZE	(0x0)
#define L3CACHE_REQUEST_SIZE	0
#endif
#endif

struct idle_rb_closed_reg {
	bool need_recovery;
	uint32_t dsc_en;
	uint32_t dpp_clip_en;
	uint32_t dither_ctl0;
	uint32_t gama_en;
	uint32_t xcc_en;
	uint32_t degama_en;
	uint32_t gmp_en;
	uint32_t arsr_post_bypass;
	uint32_t ifbc_en;
	uint32_t hiace_bypass;
	uint32_t nr_bypass;
};

struct hisifb_video_idle_ctrl {
	int idle_ctrl_created;

	bool mmu_enable;
	bool afbc_enable;
	bool compress_enable;
	bool video_idle_wb_status;
	bool video_idle_rb_status;
	bool gpu_compose_idle_frame;

	uint32_t wb_irq;
	uint32_t rch_idx;
	uint32_t wch_idx;
	uint32_t ovl_idx;
	uint32_t wdma_format;

	uint32_t wb_hsize;
	uint32_t wb_vsize;

	uint32_t wb_pad_num;
	uint32_t wb_pack_hsize;

	uint32_t wdfc_pad_hsize;
	uint32_t wdfc_pad_num;

	uint32_t l3cache_size;
	uint32_t l3_request_size; // range:1~4
	uint32_t lb_size;
	uint32_t policy_id;

	struct dma_buf *buf;
	uint32_t buf_size;

	struct idle_rb_closed_reg rb_closed_reg;
	struct ion_handle *wb_handle;

	uint32_t wb_buffer_size;
	char __iomem *wb_buffer_base;
	phys_addr_t wb_phys_addr;
	uint64_t wb_vir_addr;

	bool buffer_alloced;
	bool idle_frame_display;
#ifdef CONFIG_HISI_L3CACHE_SHARE
	struct l3_cache_request_params request_params;
	struct l3_cache_release_params release_params;
#endif
	struct workqueue_struct *idle_wb_err_wq;
	struct work_struct idle_wb_err_work;
	struct work_struct wb_err_work;

	struct mutex video_idle_ctrl_lock;
	struct work_struct video_idle_ctrl_work;

	struct hisi_fb_data_type *hisifd;
};

int hisifb_video_panel_idle_display_ctrl(struct hisi_fb_data_type *hisifd, uint32_t video_idle_status);
void hisifb_hisync_disp_sync_enable(struct hisi_fb_data_type *hisifd);
int hisifb_hisync_disp_sync_config(struct hisi_fb_data_type *hisifd);
void hisifb_video_idle_buffer_free(struct hisi_fb_data_type *hisifd);
int hisifb_video_idle_release_cache(struct hisi_fb_data_type *hisifd);

#endif /* HISI_FB_VIDEO_IDLE_H */

