/*
 * Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef HISI_DRM_DPE_UTILS_H
#define HISI_DRM_DPE_UTILS_H

#include "hisi_dpe.h"
#include "hisi_drm_drv.h"

#define DTS_COMP_DSS_NAME "hisilicon,kunpeng902-dpe"

#ifndef AFBC_FORMAT_MOD_BLOCK_SIZE_16X16
#define AFBC_FORMAT_MOD_BLOCK_SIZE_16X16 (1ULL)
#endif

#ifndef AFBC_FORMAT_MOD_YTR
#define AFBC_FORMAT_MOD_YTR (1ULL << 4)
#endif

#ifndef AFBC_FORMAT_MOD_SPLIT
#define AFBC_FORMAT_MOD_SPLIT (1ULL << 5)
#endif

#ifndef AFBC_FORMAT_MOD_SPARSE
#define AFBC_FORMAT_MOD_SPARSE (1ULL << 6)
#endif

#ifndef DRM_FORMAT_MOD_VENDOR_ARM
#define DRM_FORMAT_MOD_VENDOR_ARM 0x08
#endif

#ifndef DRM_FORMAT_MOD_ARM_AFBC
#define DRM_FORMAT_MOD_ARM_AFBC(__afbc_mode) fourcc_mod_code(ARM, (__afbc_mode))
#endif

#define AFBC_FORMAT_MODIFIER DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_16X16 | \
	AFBC_FORMAT_MOD_YTR | \
	AFBC_FORMAT_MOD_SPLIT | \
	AFBC_FORMAT_MOD_SPARSE)

enum hisi_drm_blending {
	HISI_BLENDING_NONE = 0,
	HISI_BLENDING_PREMULT = 1,
	HISI_BLENDING_COVERAGE = 2,
	HISI_BLENDING_MAX = 3,
};

enum dss_afbc_scramble_mode {
	DSS_AFBC_SCRAMBLE_NONE = 0,
	DSS_AFBC_SCRAMBLE_MODE1,
	DSS_AFBC_SCRAMBLE_MODE2,
	DSS_AFBC_SCRAMBLE_MODE3,
	DSS_AFBC_SCRAMBLE_MODE_MAX,
};

struct hisi_afbc_aligned_info {
	uint32_t width;
	uint32_t height;
	uint32_t aligned_width;
	uint32_t aligned_height;
	uint32_t header_size;
};


// adjust position
void set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs);
uint32_t set_bits32(uint32_t old_val, uint32_t val, uint8_t bw, uint8_t bs);
void hisi_set_reg(struct hisi_drm_crtc *hisi_crtc, char __iomem *addr,
	uint32_t val, uint8_t bw, uint8_t bs);
bool is_mipi_cmd_mode(struct hisi_drm_crtc *hisi_crtc);
bool is_mipi_video_mode(struct hisi_drm_crtc *hisi_crtc);
bool is_dp(struct hisi_drm_crtc *hisi_crtc);
bool is_mipi_mode(struct hisi_drm_crtc *hisi_crtc);
bool is_dual_mipi_mode(struct hisi_drm_crtc *hisi_crtc);

void hisi_drm_get_timestamp(struct timeval *tv);
uint32_t hisi_drm_timestamp_diff(struct timeval *lasttime,
	struct timeval *curtime);

void init_dbuf(struct hisi_drm_crtc *hisi_crtc);
void init_dpp(struct hisi_drm_crtc *hisi_crtc);
void init_ldi(struct hisi_drm_crtc *hisi_crtc);

int dpe_deinit(struct hisi_drm_crtc *hisi_crtc);
void dpe_check_itf_status(struct hisi_drm_crtc *hisi_crtc);
int dpe_inner_clk_disable(struct hisi_drm_crtc *hisi_crtc);
void dss_inner_clk_common_disable(struct dss_hw_ctx *ctx);
int dpe_pdp_regulator_enable(struct hisi_drm_crtc *hisi_crtc);
int dpe_pdp_regulator_disable(struct hisi_drm_crtc *hisi_crtc);
int dpe_inner_clk_enable(struct hisi_drm_crtc *hisi_crtc);
int dpe_common_clk_enable(struct dss_hw_ctx *ctx);
int dpe_common_clk_disable(struct dss_hw_ctx *ctx);
void dss_inner_clk_sdp_enable(struct hisi_drm_crtc *hisi_crtc);
void dss_inner_clk_sdp_disable(struct hisi_drm_crtc *hisi_crtc);
void dss_inner_clk_common_enable(struct hisi_drm_crtc *hisi_crtc);
void dpe_interrupt_clear(struct hisi_drm_crtc *hisi_crtc);
void dpe_interrupt_unmask(struct hisi_drm_crtc *hisi_crtc);
void dpe_interrupt_mask(struct hisi_drm_crtc *hisi_crtc);
int dpe_regulator_enable(struct dss_hw_ctx *ctx, int regulator);
int dpe_regulator_disable(struct dss_hw_ctx *ctx, int regulator);
int get_dss_clock_value(uint32_t voltage_level,
	struct dss_vote_cmd *dss_vote_cmd);

int dp_pxl_ppll7_init(struct hisi_drm_crtc *hisi_crtc, uint64_t pixel_clock);

int dpe_init(struct hisi_drm_crtc *hisi_crtc);
void deinit_dbuf(struct hisi_drm_crtc *hisi_crtc);
int hisi_overlay_on(struct hisi_drm_crtc *hisi_crtc);

int hisi_dss_mctl_mutex_lock(struct hisi_drm_crtc *hisi_crtc);
int hisi_dss_mctl_mutex_unlock(struct hisi_drm_crtc *hisi_crtc);

int hisi_dss_off(struct hisi_drm_crtc *hisi_crtc);

int hisi_dss_module_default(struct hisi_drm_crtc *hisi_crtc);
int hisi_dss_module_init(struct hisi_drm_crtc *hisi_crtc);

u32 dss_get_format(uint32_t pixel_format, uint64_t modifier);
int hisi_set_dss_external_vote_pre(struct hisi_drm_crtc *hisi_crtc,
	uint64_t pixel_clock);

int dpe_set_clk_rate(struct hisi_drm_crtc *hisi_crtc);
int hisi_set_mmbuf_clk_rate(struct dss_hw_ctx *ctx);
int dpe_regulator_clk_irq_setup(struct device *dev,
	struct hisi_drm_crtc *hisi_crtc);
int dpe_dts_parse(struct platform_device *pdev, struct dss_hw_ctx *ctx);

void init_acm(struct dss_hw_ctx *ctx);
void init_dither(struct dss_hw_ctx *ctx);
void init_igm_gmp_xcc_gm(struct dss_hw_ctx *ctx);
void init_ifbc(struct dss_hw_ctx *ctx);
void hisi_display_post_process_chn_init(struct hisi_drm_crtc *hisi_crtc);

int hisi_ov_base_config(struct hisi_drm_crtc *hisi_crtc);

void hisi_dss_update_layer(struct drm_plane *plane);

int hisi_vactive0_start_config(struct hisi_drm_crtc *hisi_crtc);
void ldi_data_gate(struct hisi_drm_crtc *hisi_crtc, bool enable);

irqreturn_t dss_irq_handler(int irq, void *ptr);

void enable_dp_ldi(char __iomem *dss_base);
void disable_dp_ldi(char __iomem *dss_base);
void crtc_disable_ldi(struct hisi_drm_crtc *hisi_crtc);
void crtc_single_frame_update(struct hisi_drm_crtc *hisi_crtc);
void hisi_dump_current_info(struct hisi_drm_crtc *hisi_crtc);

void no_memory_lp_ctrl(struct hisi_drm_crtc *hisi_crtc);
void lp_second_level_clk_gate_ctrl(struct hisi_drm_crtc *hisi_crtc);
void lp_first_level_clk_gate_ctrl(struct hisi_drm_crtc *hisi_crtc);

#endif
