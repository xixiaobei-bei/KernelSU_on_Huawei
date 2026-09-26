/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
/*lint -e86 -e438 -e529 -e546 -e550 -e559 -e563 -e571 -e574 -e578 -e648 -e666 -e712 -e747 -e580 -e616*/
#include <securec.h>

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/of_graph.h>
#include <linux/component.h>
#include <video/of_display_timing.h>
#include <video/of_videomode.h>
#include <video/videomode.h>

#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_of.h>
#include <drm/drm_panel.h>
#include <drm/drm_atomic_helper.h>

#include <linux/hisi/usb/tca.h>

#include "dp_audio.h"
#include "hisi_panel.h"
#include "hisi_dp_drv.h"
#include "hisi_drm_drv.h"
#include "hisi_defs.h"
#include "dp/link/dp_irq.h"
#include "dp/link/dp_aux.h"
#include "dp/link/dp_edid.h"
#include "dp/link/dp_link_training.h"
#include "dp/controller/hidpc/hidpc_dp_aux.h"
#include "dp/controller/hidpc/hidpc_dp_avgen.h"
#include "dp/controller/hidpc/hidpc_dp_core.h"
#include "dp/controller/hidpc/hidpc_dp_irq.h"
#include "dp/controller/dp_core_interface.h"
#include "dp/link/dp_link_layer_interface.h"

#include "hisi_drm_dpe_utils.h"
#include "bridge/rtd2172.h"
#include "bridge/edp_bridge.h"

#include <huawei_platform/dp_source/dp_source_switch.h>
#include <huawei_platform/dp_source/dp_debug.h>

#define connector_to_dp(c) \
		container_of(c, struct hisi_dp_device, connector)

#define encoder_to_dp(c) \
		container_of(c, struct hisi_dp_device, encoder)

#define DTS_COMP_SWING_VALUE "hisilicon,drm_hisi_dp"
#define DTS_DP_AUX_SWITCH "huawei,dp_aux_switch"
#define PERI_VOLTAGE_LEVEL0_060V 0 /* 0.60v */
#define PERI_VOLTAGE_LEVEL1_065V 1 /* 0.65v */
#define PERI_VOLTAGE_LEVEL2_070V 2 /* 0.70v */
#define PERI_VOLTAGE_LEVEL3_080V 3 /* 0.80v */

enum HPD_IRQ_TYPE_E {
	IRQ_HPD_IN = 0,
	IRQ_HPD_OUT,
};

bool is_valid_irq = false;
static bool btrigger_timeout;
struct platform_device *g_dp_pdev;

struct dp_div_info {
	uint64_t refdiv;
	uint64_t fbdiv;
	uint64_t frac;
	uint64_t postdiv1;
	uint64_t postdiv2;
	uint64_t vco_freq_output;
};

int dptx_phy_rate_to_bw(uint8_t rate)
{
	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		return DP_LINK_BW_1_62;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		return DP_LINK_BW_2_7;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		return DP_LINK_BW_5_4;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		return DP_LINK_BW_8_1;
	default:
		HISI_DRM_ERR("[DP] Invalid rate 0x%x\n", rate);
		return -EINVAL;
	}
}

int dptx_bw_to_phy_rate(uint8_t bw)
{
	switch (bw) {
	case DP_LINK_BW_1_62:
		return DPTX_PHYIF_CTRL_RATE_RBR;
	case DP_LINK_BW_2_7:
		return DPTX_PHYIF_CTRL_RATE_HBR;
	case DP_LINK_BW_5_4:
		return DPTX_PHYIF_CTRL_RATE_HBR2;
	case DP_LINK_BW_8_1:
		return DPTX_PHYIF_CTRL_RATE_HBR3;
	default:
		HISI_DRM_ERR("[DP] Invalid bw 0x%x\n", bw);
		return -EINVAL;
	}
}

/*
 * Audio/Video Parameters Reset
 */
void dptx_audio_params_reset(struct audio_params *params)
{
	int ret;

	if (params == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return;
	}

	ret = memset_s(params, sizeof(struct audio_params), 0x0, sizeof(struct audio_params));
	drm_check_and_void_return(ret != EOK, "memset failed!");
	params->iec_channel_numcl0 = 8;
	params->iec_channel_numcr0 = 4;
	params->use_lut = 1;
	params->iec_samp_freq = 3;
	params->iec_word_length = 11;
	params->iec_orig_samp_freq = 12;
	params->data_width = 16;
	params->num_channels = 2;
	params->inf_type = 1;
	params->ats_ver = 17;
	params->mute = 0;
}

void dptx_video_params_reset(struct video_params *params)
{
	int ret;

	if (params == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return;
	}

	ret = memset_s(params, sizeof(struct video_params), 0x0, sizeof(struct video_params));
	drm_check_and_void_return(ret != EOK, "memset failed!");

	/* 6 bpc should be default - use 8 bpc for MST calculation */
	params->bpc = COLOR_DEPTH_8;
	params->dp_dsc_info.dsc_info.dsc_bpp = 8; // DPTX_BITS_PER_PIXEL
	params->pix_enc = RGB;
	params->mode = 1;
	params->colorimetry = ITU601;
	params->dynamic_range = CEA;
	params->aver_bytes_per_tu = 30;
	params->aver_bytes_per_tu_frac = 0;
	params->init_threshold = 15;
	params->pattern_mode = RAMP;
	params->refresh_rate = 60000;
	params->video_format = VCEA;
}

/*
 * DTD reset
 */
void dwc_dptx_dtd_reset(struct dtd *mdtd)
{
	if (mdtd == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return;
	}

	mdtd->pixel_repetition_input = 0;
	mdtd->pixel_clock  = 0;
	mdtd->h_active = 0;
	mdtd->h_blanking = 0;
	mdtd->h_sync_offset = 0;
	mdtd->h_sync_pulse_width = 0;
	mdtd->h_image_size = 0;
	mdtd->v_active = 0;
	mdtd->v_blanking = 0;
	mdtd->v_sync_offset = 0;
	mdtd->v_sync_pulse_width = 0;
	mdtd->v_image_size = 0;
	mdtd->interlaced = 0;
	mdtd->v_sync_polarity = 0;
	mdtd->h_sync_polarity = 0;
}

void dptx_notify(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_DRM_ERR("[DP] dptx is NULL!\n");
		return;
	}

	wake_up_interruptible(&dptx->waitq);
}

void dptx_notify_shutdown(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_DRM_ERR("[DP] dptx is NULL!\n");
		return;
	}

	atomic_set(&dptx->shutdown, 1);
	wake_up_interruptible(&dptx->waitq);
}


static int dp_ceil(uint64_t a, uint64_t b)
{
	if (b == 0)
		return -1;


	if (a % b != 0)
		return a / b + 1;
	else
		return a / b;
}

static int dp_ppll7_calc_postdiv(struct dp_div_info *div_info, uint64_t pixel_clock_cur)
{
	int i;
	int ceil_temp;
	uint64_t ppll7_freq_divider;

	int freq_divider_list[22] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
			12, 14, 15, 16, 20, 21, 24,
			25, 30, 36, 42, 49};

	int postdiv1_list[22] = {1, 2, 3, 4, 5, 6, 7, 4, 3, 5,
		   4, 7, 5, 4, 5, 7, 6, 5, 6, 6,
			7, 7};

	int postdiv2_list[22] = {1, 1, 1, 1, 1, 1, 1, 2, 3, 2,
		3, 2, 3, 4, 4, 3, 4, 5, 5, 6,
			6, 7};

	ceil_temp = dp_ceil(KIRIN_VCO_MIN_FREQ_OUPUT, pixel_clock_cur);

	if (ceil_temp < 0)
		return -EINVAL;

	ppll7_freq_divider = (uint64_t)ceil_temp;

	for (i = 0; i < 22; i++) {
		if (ppll7_freq_divider <= freq_divider_list[i]) {
			ppll7_freq_divider = freq_divider_list[i];
			div_info->postdiv1 = postdiv1_list[i];
			div_info->postdiv2 = postdiv2_list[i];
			HISI_DRM_DEBUG("[DP] postdiv1=0x%llx, POSTDIV2=0x%llx\n",
				div_info->postdiv1, div_info->postdiv2);
			break;
		}
	}

	div_info->vco_freq_output = ppll7_freq_divider * pixel_clock_cur;

	return 0;

}

static int dp_ppll7_calc_div(struct dp_div_info *div_info, uint64_t pixel_clock_cur)
{
	int ret;
	int ceil_temp;
	uint64_t vco_freq_output;
	uint64_t refdiv, fbdiv;
	uint64_t sys_clock_fref = KIRIN_SYS_FREQ;
	uint64_t frac_range = 0x1000000; /* 2^24 */

	ret = dp_ppll7_calc_postdiv(div_info, pixel_clock_cur);
	if (ret)
		return ret;

	vco_freq_output = div_info->vco_freq_output;
	if (vco_freq_output == 0)
		return -EINVAL;

	HISI_DRM_DEBUG("[DP] vco_freq_output=0x%llx\n", vco_freq_output);

	ceil_temp = dp_ceil(400000, vco_freq_output);

	if (ceil_temp < 0)
		return -EINVAL;

	refdiv = ((vco_freq_output * ceil_temp) >= 494000) ? 1 : 2;
	HISI_DRM_DEBUG("[DP] refdiv=0x%llx\n", refdiv);

	fbdiv = (vco_freq_output * ceil_temp) * refdiv / sys_clock_fref;
	HISI_DRM_DEBUG("[DP] fbdiv=0x%llx\n", fbdiv);

	div_info->frac = (uint64_t)(ceil_temp * vco_freq_output -
		sys_clock_fref / refdiv * fbdiv) * refdiv * frac_range;

	div_info->frac = (uint64_t)div_info->frac / sys_clock_fref;
	HISI_DRM_DEBUG("[DP] frac=0x%llx\n", div_info->frac);

	div_info->refdiv = refdiv;
	div_info->fbdiv = fbdiv;
	return 0;
}

static void dp_ppll7_ctrl_config(struct dss_hw_ctx *ctx, struct dp_div_info div_info)
{
	uint32_t ppll7ctrl0;
	uint32_t ppll7ctrl1;
	uint32_t ppll7ctrl0_val;
	uint32_t ppll7ctrl1_val;

	ppll7ctrl0 = inp32(ctx->pmctrl_base + MIDIA_PPLL7_CTRL0);
	ppll7ctrl0 &= ~MIDIA_PPLL7_FREQ_DEVIDER_MASK;

	ppll7ctrl0_val = 0x0;
	ppll7ctrl0_val |= (uint32_t)(div_info.postdiv2 << 23 | div_info.postdiv1 << 20 |
		div_info.fbdiv << 8 | div_info.refdiv << 2);
	ppll7ctrl0_val &= MIDIA_PPLL7_FREQ_DEVIDER_MASK;
	ppll7ctrl0 |= ppll7ctrl0_val;

	outp32(ctx->pmctrl_base + MIDIA_PPLL7_CTRL0, ppll7ctrl0);

	ppll7ctrl1 = inp32(ctx->pmctrl_base + MIDIA_PPLL7_CTRL1);
	ppll7ctrl1 &= ~MIDIA_PPLL7_FRAC_MODE_MASK;

	ppll7ctrl1_val = 0x0;
	ppll7ctrl1_val |= (uint32_t)(1 << 25 | 0 << 24 | div_info.frac);
	ppll7ctrl1_val &= MIDIA_PPLL7_FRAC_MODE_MASK;
	ppll7ctrl1 |= ppll7ctrl1_val;

	outp32(ctx->pmctrl_base + MIDIA_PPLL7_CTRL1, ppll7ctrl1);
}

int dp_pxl_ppll7_init(struct hisi_drm_crtc *hisi_crtc, uint64_t pixel_clock)
{
	struct dss_hw_ctx *ctx = NULL;
	struct dp_div_info div_info = {0};
	int ret;
	uint8_t j = 1;
	uint64_t pixel_clock_cur;
	uint64_t vco_min_freq_output = KIRIN_VCO_MIN_FREQ_OUPUT;

	if (pixel_clock == 0)
		return -EINVAL;

	if (hisi_crtc == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	ctx = hisi_crtc->ctx;

	pixel_clock_cur = pixel_clock / 2000;

	if (pixel_clock_cur * 49 < vco_min_freq_output) { /* Fractional PLL can not output the so small clock */
		j = 2;
		pixel_clock_cur *= j; /* multiple frequency */
	}

	ret = dp_ppll7_calc_div(&div_info, pixel_clock_cur);
	if (ret)
		return -EINVAL;

	dp_ppll7_ctrl_config(ctx, div_info);

	/* comfirm ldi1 switch ppll7 */
	pixel_clock_cur = pixel_clock_cur * 1000;

	ret = hisi_set_dss_external_vote_pre(hisi_crtc, pixel_clock_cur / j);
	if (ret < 0)
		HISI_DRM_ERR("[DP] fb%d dss pxl1 clk set rate(%llu) set vote failed, error=%d!\n",
			hisi_crtc->crtc_id, pixel_clock_cur, ret);

	ret = clk_set_rate(ctx->dpe_clk[DPE_PXL1_CLK],
		DEFAULT_MIDIA_PPLL7_CLOCK_FREQ / j);

	if (ret < 0)
		HISI_DRM_ERR("[DP] dss pxl1 clk set rate(%llu) failed, error=%d!\n",
			pixel_clock_cur, ret);

	HISI_DRM_DEBUG("crtc %d clk set rate(%llu)\n", hisi_crtc->crtc_id,
		(uint64_t)clk_get_rate(hisi_crtc->ctx->dpe_clk[DPE_PXL1_CLK]));

	return ret;
}

static int auxclk_enable(struct hisi_dp_device *dp_dev)
{
	int ret;
	struct clk *clk_tmp;

	clk_tmp = dp_dev->dss_auxclk_dpctrl_clk;
	if (clk_tmp == NULL) {
		HISI_DRM_ERR("[DP] dss auxclk dpctrl clk is NULL");
		return -EINVAL;
	}

	ret = clk_prepare(clk_tmp);
	if (ret) {
		HISI_DRM_ERR("[DP] auxclk prepare failed, error=%d\n",
			ret);
		return -EINVAL;
	}

	ret = clk_enable(clk_tmp);
	if (ret) {
		HISI_DRM_ERR("[DP] auxclk enable failed, error=%d!\n",
			ret);
		return -EINVAL;
	}

	return 0;
}


static int pclk_enable(struct hisi_dp_device *dp_dev)
{
	int ret;
	struct clk *clk_tmp;

	clk_tmp = dp_dev->dss_pclk_dpctrl_clk;

	if (clk_tmp == NULL) {
		HISI_DRM_ERR("[DP] dss pclk dpctrl clk is NULL");
		return -EINVAL;
	}

	ret = clk_prepare(clk_tmp);
	if (ret) {
		HISI_DRM_ERR("[DP] pclk prepare failed, error=%d!\n",
			ret);
		return -EINVAL;
	}

	ret = clk_enable(clk_tmp);
	if (ret) {
		HISI_DRM_ERR("[DP] pclk enable failed, error=%d!\n",
			ret);
		return -EINVAL;
	}

	return 0;
}

static int aclk_enable(struct hisi_dp_device *dp_dev)
{
	int ret;
	struct clk *clk_tmp;

	clk_tmp = dp_dev->dss_aclk_dpctrl_clk;
	if (clk_tmp == NULL) {
		HISI_DRM_ERR("[DP] dss aclk dpctrl clk is NULL");
		return -EINVAL;
	}

	ret = clk_prepare(clk_tmp);
	if (ret) {
		HISI_DRM_ERR("[DP] dss aclk clk_prepare failed, error=%d!\n",
			ret);
		return -EINVAL;
	}

	ret = clk_enable(clk_tmp);
	if (ret) {
		HISI_DRM_ERR("[DP] dss aclk clk_enable failed, error=%d!\n",
			ret);
		return -EINVAL;
	}

	return 0;
}

static int dp_clk_enable(struct hisi_dp_device *dp_dev)
{
	int ret;

	HISI_DRM_INFO("[DP] dp clk enable +.\n");

	ret = auxclk_enable(dp_dev);
	if (ret)
		return -EINVAL;

	ret = pclk_enable(dp_dev);
	if (ret)
		return -EINVAL;

	ret = aclk_enable(dp_dev);
	if (ret)
		return -EINVAL;

	HISI_DRM_INFO("[DP] dp clk enable -.\n");

	return 0;
}

static void auxclk_disable(struct hisi_dp_device *dp_dev)
{
	struct clk *clk_tmp;

	clk_tmp = dp_dev->dss_auxclk_dpctrl_clk;

	if (clk_tmp == NULL) {
		HISI_DRM_ERR("[DP] dss auxclk dpctrl clk is NULL");
	} else {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}
}

static void pclk_disable(struct hisi_dp_device *dp_dev)
{
	struct clk *clk_tmp;

	clk_tmp = dp_dev->dss_pclk_dpctrl_clk;

	if (clk_tmp == NULL) {
		HISI_DRM_ERR("[DP] dss pclk dpctrl clk is NULL");
	} else {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}
}

static void aclk_disable(struct hisi_dp_device *dp_dev)
{
	struct clk *clk_tmp;

	clk_tmp = dp_dev->dss_aclk_dpctrl_clk;

	if (clk_tmp == NULL) {
		HISI_DRM_ERR("[DP] dss aclk dpctrl clk is NULL");
	} else {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}
}


static int dp_clk_disable(struct hisi_dp_device *dp_dev)
{
	auxclk_disable(dp_dev);

	pclk_disable(dp_dev);

	aclk_disable(dp_dev);

	return 0;
}

static int auxclk_setup(struct device *dev, struct hisi_dp_device *dp_dev)
{
	int ret;

	dp_dev->dss_auxclk_dpctrl_clk = devm_clk_get(dev, dp_dev->dss_auxclk_dpctrl_name);
	if (IS_ERR(dp_dev->dss_auxclk_dpctrl_clk)) {
		ret = PTR_ERR(dp_dev->dss_auxclk_dpctrl_clk);
		HISI_DRM_ERR("[DP] %s get fail ret = %d.\n",
			dp_dev->dss_auxclk_dpctrl_name, ret);
		return ret;
	}

	ret = clk_set_rate(dp_dev->dss_auxclk_dpctrl_clk, DEFAULT_AUXCLK_DPCTRL_RATE);
	if (ret < 0) {
		HISI_DRM_ERR("[DP] dss_auxclk_dpctrl_clk clk_set_rate(%lu) failed, error=%d!\n",
			DEFAULT_AUXCLK_DPCTRL_RATE, ret);
		return -EINVAL;
	}

	HISI_DRM_INFO("[DP] dss_auxclk_dpctrl_clk:[%lu]->[%lu].\n",
		DEFAULT_AUXCLK_DPCTRL_RATE, clk_get_rate(dp_dev->dss_auxclk_dpctrl_clk));

	return 0;
}

static int aclk_setup(struct device *dev, struct hisi_dp_device *dp_dev)
{
	int ret;
	uint32_t default_aclk_dpctrl_rate;

	default_aclk_dpctrl_rate = DEFAULT_ACLK_DPCTRL_RATE;
	dp_dev->dss_aclk_dpctrl_clk = devm_clk_get(dev, dp_dev->dss_aclk_dpctrl_name);
	if (IS_ERR(dp_dev->dss_aclk_dpctrl_clk)) {
		ret = PTR_ERR(dp_dev->dss_aclk_dpctrl_clk);
		HISI_DRM_ERR("[DP] dss_aclk_dpctrl_clk get fail ret = %d.\n", ret);
		return ret;
	}
	ret = clk_set_rate(dp_dev->dss_aclk_dpctrl_clk, default_aclk_dpctrl_rate);
	if (ret < 0) {
		HISI_DRM_ERR("[DP]dss_aclk_dpctrl_clk clk_set_rate(%u) failed, error=%d!\n",
			default_aclk_dpctrl_rate, ret);
		return -EINVAL;
	}

	HISI_DRM_INFO("[DP] dss_aclk_dpctrl_clk:[%u]->[%lu].\n",
		default_aclk_dpctrl_rate, clk_get_rate(dp_dev->dss_aclk_dpctrl_clk));

	return 0;
}

static int pclk_setup(struct device *dev, struct hisi_dp_device *dp_dev)
{
	int ret;

	dp_dev->dss_pclk_dpctrl_clk = devm_clk_get(dev, dp_dev->dss_pclk_dpctrl_name);

	if (IS_ERR(dp_dev->dss_pclk_dpctrl_clk)) {
		ret = PTR_ERR(dp_dev->dss_pclk_dpctrl_clk);
		HISI_DRM_ERR("[DP] dss_pclk_dpctrl_clk get fail ret = %d.\n", ret);
		return ret;
	}
	return 0;
}


static int dp_clock_setup(struct device *dev, struct hisi_dp_device *dp_dev)
{
	int ret;

	ret = auxclk_setup(dev, dp_dev);
	if (ret)
		return ret;

	ret = aclk_setup(dev, dp_dev);
	if (ret)
		return ret;

	ret = pclk_setup(dev, dp_dev);
	if (ret)
		return ret;

	return 0;
}


static int dp_dis_reset(struct hisi_dp_device *dp_dev, bool benable)
{
	struct dp_ctrl *dptx = &dp_dev->dp;

	HISI_DRM_INFO("+, %d", benable);

	if (benable)
		outp32(dptx->mmc0_crg + 0x24, 0x8); /* dp dis reset */
	else
		outp32(dptx->mmc0_crg + 0x20, 0x8); /* dp reset */
	HISI_DRM_INFO("-");

	return 0;
}

static int dp_on(struct hisi_dp_device *dp_dev)
{
	int ret;
	struct dp_ctrl *dptx = NULL;

	HISI_DRM_INFO("[DP] dp on, +.\n");

	dptx = &(dp_dev->dp);

	mutex_lock(&dptx->dptx_mutex);
	if (dptx->dptx_enable) {
		HISI_DRM_INFO("[DP] dptx has already on.\n");
		mutex_unlock(&dptx->dptx_mutex);
		return 0;
	}

	ret = dp_dis_reset(dp_dev, true);
	if (ret) {
		HISI_DRM_ERR("[DP] DPTX dis reset failed !!!\n");
		ret = -ENODEV;
		goto err_out;
	}

	ret = dp_clk_enable(dp_dev);
	if (ret) {
		HISI_DRM_ERR("[DP] DPTX dp clock enable failed !!!\n");
		ret = -ENODEV;
		goto err_out;
	}

	if (dptx->dptx_core_on) {
		ret = dptx->dptx_core_on(dptx);
		if (ret) {
			HISI_DRM_ERR("[DP] DPTX core init failed!\n");
			ret = -ENODEV;
			goto err_out;
		}
	}

	/* FIXME: clear intr */
	dptx->dptx_enable = true;
	dptx->detect_times = 0;
	dptx->current_link_rate = dptx->max_rate;
	dptx->current_link_lanes = dptx->max_lanes;
	btrigger_timeout = false;

err_out:
	mutex_unlock(&dptx->dptx_mutex);

	HISI_DRM_INFO("[DP] dp on, -.\n");

	return ret;
}

void hisi_wait_dp_off(void)
{
	int ret;
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;

	HISI_DRM_INFO("+");

	if (g_dp_pdev == NULL) {
		HISI_DRM_ERR("[DP] g_dp_pdev is NULL!\n");
		return;
	}

	dp_dev = platform_get_drvdata(g_dp_pdev);
	if (dp_dev == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx = &(dp_dev->dp);
	if (dptx == NULL) {
		HISI_DRM_ERR("[DP] dptx is NULL!\n");
		return;
	}

	if (dptx->output_mode != PRODUCT_TYPE_LAPTOP) {
		HISI_DRM_INFO("[DP] desktop do not wait\n");
		return;
	}
	ret = wait_event_interruptible_timeout(dptx->suspend_wait, !dptx->video_transfer_enable,
		msecs_to_jiffies(3000));
	if (ret <= 0) {
		HISI_DRM_ERR("Timeout waiting video_transfer_enable, %d", ret);
		return;
	}

	HISI_DRM_INFO("-");
}

/* tell crtc we hot-unpluged */
static void dp_help_notify_unhotplug(struct dp_ctrl *dptx, struct drm_connector *connector)
{
	enum drm_connector_status old_status;
	enum drm_connector_status new_status;

	old_status = connector->status;
	new_status = dptx->video_transfer_enable ?
		connector_status_connected : connector_status_disconnected;

	HISI_DRM_INFO("[DP] dptx has already connector old_status %d, cur_status %d",
		old_status, new_status);

	if (old_status != new_status)
		drm_kms_helper_hotplug_event(dptx->dp_dev->drm_dev);
}

static int dp_off(struct hisi_dp_device *dp_dev)
{
	int ret;
	struct dp_ctrl *dptx = NULL;
	struct drm_connector *connector = NULL;
	bool need_poll_event = false;

	dptx = &(dp_dev->dp);
	connector = &(dp_dev->connector);

	mutex_lock(&dptx->dptx_mutex);

	if (!dptx->dptx_enable) {
		HISI_DRM_INFO("[DP] dptx has already off.\n");
		mutex_unlock(&dptx->dptx_mutex);

		return 0;
	}

	/* FIXME: */
	if (dptx->video_transfer_enable) {
		if (dptx->handle_hotunplug)
			dptx->handle_hotunplug(dp_dev);
		need_poll_event = true;
		mdelay(10);
	}

	/* FIXME: clear intr */
	if (dptx->dptx_core_off)
		dptx->dptx_core_off(dptx);
	dp_clk_disable(dp_dev);

	ret = dp_dis_reset(dp_dev, false);
	if (ret) {
		HISI_DRM_ERR("[DP] DPTX dis reset failed !!!\n");
		ret = -ENODEV;
	}

	dptx->detect_times = 0;
	dptx->dptx_vr = false;
	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;
	dptx->dptx_plug_type = DP_PLUG_TYPE_NORMAL;
	btrigger_timeout = false;

	mutex_unlock(&dptx->dptx_mutex);
	if (need_poll_event)
		dp_help_notify_unhotplug(dptx, connector);

	wake_up_interruptible(&dptx->suspend_wait);
	HISI_DRM_INFO("[DP] dp off -.\n");
	return ret;
}

void dp_send_cable_notification(struct dp_ctrl *dptx, int val)
{
	int state = 0;
	struct dtd *mdtd = NULL;
	struct video_params *vparams = NULL;

	UNUSED(state);
	UNUSED(mdtd);
	UNUSED(vparams);

	if (dptx == NULL) {
		HISI_DRM_ERR("[DP] dptx is NULL!\n");
		return;
	}

	if (dptx->edid_info.audio.basic_audio == 0x1) {
		if (val == HOT_PLUG_OUT || val == HOT_PLUG_OUT_VR) {
			audio_dp_jack_report(false);
		} else if (val == HOT_PLUG_IN || val == HOT_PLUG_IN_VR) {
			audio_dp_jack_report(true);
		}
	} else {
		DRM_WARN("[DP] basic_audio(%ud) no support!\n", dptx->edid_info.audio.basic_audio);
	}
}

static int hisi_dptx_gpio_hpd_trigger(struct hisi_dp_device *dp_dev, enum HPD_IRQ_TYPE_E irq_type)
{
	int ret;
	struct dp_ctrl *dptx = NULL;

	if (dp_dev == NULL) {
		HISI_DRM_ERR("[DP] g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	HISI_DRM_INFO("[DP] +\n");

	dptx = &(dp_dev->dp);
	ret = 0;

	mutex_lock(&dptx->dptx_mutex);

	if (!dptx->dptx_enable) {
		HISI_DRM_ERR("[DP] dptx has already off\n");
		ret = -EINVAL;
		goto fail;
	}

	if (dptx->video_transfer_enable) {
		HISI_DRM_INFO("[DP] video_transfer is already enable\n");
		mutex_unlock(&dptx->dptx_mutex);
		return 0;
	}

#if CONFIG_DP_ENABLE
	dptx->same_source = get_current_dp_source_mode();
#endif
	dptx->user_mode = 0;
	dptx->dptx_plug_type = DP_PLUG_TYPE_NORMAL;
	dptx->user_mode_format = VCEA;
	HISI_DRM_INFO("[DP] DP HPD Type:%d, Gate:%d\n", irq_type, dptx->dptx_gate);

	/* DP HPD event must be delayed when system is booting */
	/* no need to wait as we enable dss in atomic enable after hotplug for now */
	if (!dptx->dptx_gate) {
		mutex_unlock(&dptx->dptx_mutex);
		wait_event_interruptible_timeout(dptx->dptxq, dptx->dptx_gate, msecs_to_jiffies(20000));
		mutex_lock(&dptx->dptx_mutex);
	}

	switch (irq_type) {
	case IRQ_HPD_OUT:
		if (dptx->dptx_hpd_handler)
			dptx_hpd_handler(dptx, false, DPTX_MAX_LINK_LANES);
		break;
	case IRQ_HPD_IN:
		if (dptx->dptx_hpd_handler)
			dptx_hpd_handler(dptx, true, DPTX_MAX_LINK_LANES);
		break;
	default:
		HISI_DRM_ERR("[DP] not supported tca_irq_type=%d.\n", irq_type);
		ret = -EINVAL;
		goto fail;
	}

fail:
	mutex_unlock(&dptx->dptx_mutex);

	HISI_DRM_INFO("[DP] -.\n");

	return ret;
}

static int hisi_dptx_gpio_triger(struct hisi_dp_device *dp_dev, bool enable)
{
	struct dp_ctrl *dptx = NULL;
	int ret;

	if (dp_dev == NULL) {
		HISI_DRM_ERR("[DP] g_dp_pdev is NULL!\n");
		/* return 0 to ensure USB flow when DP is absent */
		return 0;
	}

	HISI_DRM_INFO("[DP] +. DP Device %s\n", enable ? "ON" : "OFF");

	dptx = &(dp_dev->dp);

	if (!dptx->hpd_enable) {
		HISI_DRM_ERR("[DP] hot plugin detection not enable\n");
		return -EINVAL;
	}

	if (enable) {
		ret = dp_on(dp_dev);
		if (ret)
			HISI_DRM_ERR("[DP] dp_on failed!\n");
	} else {
		ret = dp_off(dp_dev);
		if (ret)
			HISI_DRM_ERR("[DP] dp_off failed!\n");
	}

	HISI_DRM_INFO("[DP] -.\n");
	return ret;
}

static irqreturn_t hpd_gpio_irq_thread(int irq, void *ptr)
{
	struct hisi_dp_device *dp_dev = ptr;
	int key_event;
	int ret = 0;

	key_event = gpio_get_value_cansleep(dp_dev->dp_hpd_gpio);
	HISI_DRM_INFO("[DP] key_event %d", key_event);

	mdelay(5);
	if (!is_valid_irq) {
		HISI_DRM_INFO("[DP] ignore invalid irq\n");
		return 0;
	}

	is_valid_irq = false;

	if (key_event == 0) {
		mdelay(100); // ignore short irq
		key_event = gpio_get_value_cansleep(dp_dev->dp_hpd_gpio);
		if (key_event == 1) {
			HISI_DRM_INFO("[DP] ignore the irq  key_event=%d", key_event);
			return 0;
		}
		ret = hisi_dptx_gpio_triger(dp_dev, false);
		if (ret) {
			HISI_DRM_ERR("OFF failed, %d", ret);
			return ret;
		}
	} else {
		ret = hisi_dptx_gpio_triger(dp_dev, true);
		if (ret) {
			HISI_DRM_ERR("ON failed, %d", ret);
			return ret;
		}
		return hisi_dptx_gpio_hpd_trigger(dp_dev, IRQ_HPD_IN);
	}
	return 0;
}

static irqreturn_t hpd_gpio_irq(int irq, void *ptr)
{
	HISI_DRM_INFO("[DP] hpd gpio irq\n");
	static bool first_irq = true;
	if (first_irq) {
		is_valid_irq = false;
		first_irq = false;
	}
	is_valid_irq = !is_valid_irq;
	return IRQ_WAKE_THREAD;
}

static void try_trigger_irq_thread(struct hisi_dp_device *dp_dev)
{
	int key_event;

	key_event = gpio_get_value_cansleep(dp_dev->dp_hpd_gpio);
	HISI_DRM_INFO("[DP] try trigger irq thread key_event %d", key_event);

	if (key_event != 0) {
		is_valid_irq = true;
		irq_wake_thread(dp_dev->dp.hpd_irq, dp_dev);
	}
}

static void enable_hpd_irq(struct dp_ctrl *dptx)
{

	HISI_DRM_INFO("enable irq %d, flag %d",
		dptx->hpd_irq, dptx->hpd_enable);
	mutex_lock(&dptx->dptx_mutex);
	if (!dptx->hpd_enable) {
		enable_irq(dptx->hpd_irq);
		dptx->hpd_enable = true;
	}
	mutex_unlock(&dptx->dptx_mutex);
}

static void disable_hpd_irq(struct dp_ctrl *dptx)
{
	HISI_DRM_INFO("disable irq %d, flag %d",
		dptx->hpd_irq, dptx->hpd_enable);
	mutex_lock(&dptx->dptx_mutex);
	if (dptx->hpd_enable) {
		disable_irq_nosync(dptx->hpd_irq);
		dptx->hpd_enable = false;
	}
	mutex_unlock(&dptx->dptx_mutex);
}

int hisi_dptx_triger(bool benable)
{
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;

	if (g_dp_pdev == NULL) {
		HISI_DRM_ERR("[DP] g_dp_pdev is NULL!\n");
		/* return 0 to ensure USB flow when DP is absent */
		return 0;
	}
	dp_dev = platform_get_drvdata(g_dp_pdev);
	if (dp_dev == NULL) {
		HISI_DRM_ERR("[DP] dp_dev is NULL!\n");
		return -EINVAL;
	}
	dptx = &(dp_dev->dp);
	HISI_DRM_INFO("+. triger by usb. enable: %s",
		benable ? "true" : "false");

	if (benable) {
		// wait for usb phy ready, then try trigger hpd irq
		enable_hpd_irq(dptx);
		try_trigger_irq_thread(dp_dev);
	} else {
		disable_hpd_irq(dptx);
		dp_off(dp_dev);
	}
	HISI_DRM_INFO("-");
	return 0;
}
EXPORT_SYMBOL(hisi_dptx_triger);

int hisi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode, TYPEC_PLUG_ORIEN_E typec_orien)
{
	HISI_DRM_INFO("hisi dptx hpd trigger by usb");
	return 0;
}
EXPORT_SYMBOL(hisi_dptx_hpd_trigger);

int hisi_dptx_notify_switch(void)
{
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;
	int ret = 0;
	bool lanes_status_change = false;

	if (g_dp_pdev == NULL) {
		HISI_DRM_ERR("[DP] g_dp_pdev is NULL!\n");
		/* return 0 to ensure USB flow when DP is absent.*/
		return 0;
	}

	dp_dev = platform_get_drvdata(g_dp_pdev);
	if (dp_dev == NULL) {
		HISI_DRM_ERR("[DP] dp_dev is NULL!\n");
		return -EINVAL;
	}


	dptx = &(dp_dev->dp);
	HISI_DRM_INFO("[DP] Status: %d.\n", dp_dev->dp.dptx_enable);
	mutex_lock(&dptx->dptx_mutex);

	if (dptx->dptx_enable) {
		lanes_status_change = true;
		dptx_disable_default_video_stream(dptx, 0);
		dptx_typec_reset_ack(dptx);
		dptx_phy_set_lanes_power(dptx, false);
		HISI_DRM_INFO("[DP] typec reset has ack\n");
	}
	mutex_unlock(&dptx->dptx_mutex);

	if (!lanes_status_change) {
		ret = dp_on(dp_dev);
		if (ret) {
			HISI_DRM_ERR("[DP] dp_on failed!\n");
			return -EINVAL;
		}
	}

	return 0;
}
EXPORT_SYMBOL(hisi_dptx_notify_switch);

bool hisi_dptx_ready(void)
{
	return (g_dp_pdev != NULL);
}
EXPORT_SYMBOL_GPL(hisi_dptx_ready);

int dp_wakeup(void)
{
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;

	if (g_dp_pdev == NULL) {
		HISI_DRM_ERR("[DP] g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	dp_dev = platform_get_drvdata(g_dp_pdev);
	if (dp_dev == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	dptx = &(dp_dev->dp);

	if (dptx == NULL) {
		HISI_DRM_ERR("[DP] dptx is NULL!\n");
		return -EINVAL;
	}

	HISI_DRM_INFO("dp_wakeup");

	dptx->dptx_gate = true;
	if (dptx->dptx_enable)
		wake_up_interruptible(&dptx->dptxq);

	return 0;
}

static int hisi_dp_connector_get_modes(struct drm_connector *connector)
{
	struct hisi_dp_device *dp_dev = connector_to_dp(connector);
	struct dp_ctrl *dptx = &dp_dev->dp;
	int ret;

	HISI_DRM_INFO("[DP] hisi dp connector get modes");

	mutex_lock(&dptx->dptx_mutex);
	ret = drm_add_edid_modes(connector,(struct edid*)dptx->edid);
	if (ret) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
		drm_mode_connector_update_edid_property(connector,
							(struct edid*)dptx->edid);
		drm_edid_to_eld(connector, (struct edid*)dptx->edid);
#else
		drm_connector_update_edid_property(connector,
						(struct edid*)dptx->edid);
#endif
	}
	mutex_unlock(&dptx->dptx_mutex);
	HISI_DRM_INFO("[DP] hisi dp connector get_modes number %d", ret);
	return ret;
}

static struct drm_encoder *hisi_dp_connector_best_encoder(struct drm_connector *connector)
{
	struct hisi_dp_device *dp_dev = connector_to_dp(connector);

	return &dp_dev->encoder;
}

static int hisi_dp_connector_mode_valid(struct drm_connector *connector,
		struct drm_display_mode *mode)
{
	struct hisi_dp_device *dp_dev = connector_to_dp(connector);
	struct dp_ctrl *dptx = &dp_dev->dp;
	struct dtd mdtd;
	int retval;

	/* If DP is disconnected, every mode is invalid */
	if (!dptx->video_transfer_enable)
		return MODE_BAD;

	if (mode->flags & DRM_MODE_FLAG_INTERLACE) {
		HISI_DRM_INFO("[DP] not support INTERLACE");
		return MODE_BAD;
	}

	drmmode_to_dtd(&mdtd, mode);

	if (mdtd.v_blanking > VBLANKING_MAX) {
		HISI_DRM_INFO("[DP] v_blanking more than %d", VBLANKING_MAX);
		return MODE_BAD;
	}

	if (mdtd.h_sync_pulse_width < 6) {
		HISI_DRM_INFO("[DP] In Double Pixel mode, h_pulse_width should be more than 6.\n");
		return MODE_BAD;
	}

	retval = dptx_video_ts_calculate(dptx, &dptx->vparams, mdtd.pixel_clock);
	if (retval) {
		HISI_DRM_INFO("mode valid fail clock=%d\n", mode->clock);
		return MODE_CLOCK_HIGH;
	}

	return MODE_OK;
}

static enum drm_connector_status hisi_dp_connector_detect(
		struct drm_connector *connector, bool force)
{
	struct hisi_dp_device *dp = connector_to_dp(connector);
	struct dp_ctrl *dptx = &dp->dp;
	enum drm_connector_status status = connector_status_disconnected;
	HISI_DRM_INFO("[DP] hisi dp connector detect + ");
	mutex_lock(&dptx->dptx_mutex);
	status = gpio_get_value_cansleep(dp->dp_hpd_gpio) ? connector_status_connected : connector_status_disconnected;
	mutex_unlock(&dptx->dptx_mutex);
	HISI_DRM_INFO("[DP] hisi dp connector detect -:%d", status);

	return status;
}

static void hisi_dp_connector_destroy(struct drm_connector *connector)
{
	drm_connector_unregister(connector);
	drm_connector_cleanup(connector);
}

static struct drm_connector_helper_funcs hisi_dp_connector_helper_funcs = {
	.get_modes = hisi_dp_connector_get_modes,
	.best_encoder = hisi_dp_connector_best_encoder,
	.mode_valid = hisi_dp_connector_mode_valid,
};

static const struct drm_connector_funcs hisi_dp_atomic_connector_funcs = {
	.detect = hisi_dp_connector_detect,
	.destroy = hisi_dp_connector_destroy,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

static void hisi_dp_encoder_mode_set(struct drm_encoder *encoder,
		struct drm_display_mode *mode,
		struct drm_display_mode *adj_mode)
{
	struct hisi_dp_device *dp_dev = encoder_to_dp(encoder);
	struct dp_ctrl *dptx = NULL;

	if (dp_dev == NULL) {
		HISI_DRM_ERR("dp_dev is NULL");
		return;
	}

	dptx = &dp_dev->dp;

	HISI_DRM_INFO("[DP] adj_mode:clock %dkHZ," \
		"hdisplay=%d,vdisplay=%d," \
		"hsync_start=%d,hsync_end=%d," \
		"htotal=%d,vsync_start=%d," \
		"vsync_end=%d,vtotal=%d," \
		"flags=%d",
		adj_mode->clock,
		adj_mode->hdisplay,
		adj_mode->vdisplay,
		adj_mode->hsync_start,
		adj_mode->hsync_end,
		adj_mode->htotal,
		adj_mode->vsync_start,
		adj_mode->vsync_end,
		adj_mode->vtotal,
		adj_mode->flags);

	mutex_lock(&dptx->dptx_mutex);
	drm_mode_copy(&dp_dev->cur_mode, adj_mode);
	mutex_unlock(&dptx->dptx_mutex);

	HISI_DRM_DEBUG("[DP] dp_encoder_mode_set, dsi->cur_mode.clock=%d\n", dp_dev->cur_mode.clock);
}

static int hisi_dp_encoder_atomic_check(struct drm_encoder *encoder,
		struct drm_crtc_state *crtc_state,
		struct drm_connector_state *conn_state)
{
	/* do nothing */
	return 0;
}

static int dp_encoder_config(struct hisi_dp_device *dp_dev, struct dp_ctrl *dptx)
{
	int ret;

	ret = dptx_link_check_status(dptx);
	if (ret) {
		HISI_DRM_ERR("Failed link train %d\n", ret);
		return ret;
	}

	ret = dptx_change_video_mode_by_drm(dptx, &dp_dev->cur_mode);
	if (ret)
		HISI_DRM_ERR("[DP] Change mode by user setting error!\n");

	ret = dptx_resolution_switch(dp_dev, HOT_PLUG_IN);
	if (ret)
		HISI_DRM_ERR("[DP] HOT_PLUG_IN DSS init fail !!!\n");

	dptx_timing_config(dptx);

	dptx->current_link_rate = dptx->link.rate;
	dptx->current_link_lanes = dptx->link.lanes;
	return 0;
}

static void hisi_dp_encoder_enable(struct drm_encoder *encoder)
{
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;
	int ret;
	int key_event;

	if (encoder == NULL) {
		HISI_DRM_ERR("[DP] NULL pointer -\n");
		return;
	}

	dp_dev = encoder_to_dp(encoder);
	if (dp_dev == NULL) {
		HISI_DRM_ERR("[DP] dp_dev NULL pointer -\n");
		return;
	}

	dptx = &dp_dev->dp;
	HISI_DRM_INFO("[DP] +");

	key_event = gpio_get_value_cansleep(dp_dev->dp_hpd_gpio);
	if (key_event == 0) {
		HISI_DRM_INFO("key_event = 0, do not enable dp encoder\n");
		HISI_DRM_INFO("[DP] -");
		return;
	} else {
		HISI_DRM_INFO("wake up irq thread");

		ret = hisi_dptx_gpio_triger(dp_dev, true);
		if (ret)
			HISI_DRM_ERR("ON failed, %d", ret);
		hisi_dptx_gpio_hpd_trigger(dp_dev, IRQ_HPD_IN);
	}

	ret = wait_event_interruptible_timeout(dptx->transfer_wait, dptx->video_transfer_enable,
		msecs_to_jiffies(1000));
	if (ret <= 0) {
		HISI_DRM_ERR("Timeout waiting video_transfer_enable=%d ret = %d",
			dptx->video_transfer_enable, ret);
		return;
	}

	mutex_lock(&dptx->dptx_mutex);
	if (!dptx->video_transfer_enable) { /* may dp_off in gpio */
		mutex_unlock(&dptx->dptx_mutex);
		HISI_DRM_ERR("video_transfer_enable is false!");
		return;
	}

	ret = dp_encoder_config(dp_dev, dptx);
	if (ret)
		goto lock_out;

	if (atomic_read(&dptx->sink_request)) {
		atomic_set(&dptx->sink_request, 0);
		HISI_DRM_INFO("[DP] handle_sink_request");
		ret = handle_sink_request(dptx);
		if (ret)
			HISI_DRM_ERR("[DP] Unable to handle sink request %d\n", ret);
	}

lock_out:
	mutex_unlock(&dptx->dptx_mutex);

	HISI_DRM_INFO("[DP] -");
}

static void dp_check_itf_status(char __iomem *dss_base)
{
	uint32_t val = 0;
	int delay_count = 0;
	bool is_timeout = true;
	char __iomem *mctl_sys_base = NULL;

	mctl_sys_base =  dss_base + DSS_MCTRL_SYS_OFFSET;
	while (1) {
		val= inp32(mctl_sys_base + MCTL_MOD17_STATUS + 0x4);
		if (((val & 0x10) == 0x10) || delay_count > 100) {
			is_timeout = (delay_count > 100) ? true : false;
			break;
		}

		mdelay(1);
		++delay_count;
	}

	if (is_timeout)
		HISI_DRM_DEBUG("mctl_itf not in idle status,ints=0x%x !\n", val);
}

static void hisi_dp_encoder_disable(struct drm_encoder *encoder)
{
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;
	struct drm_device *drm = NULL;
	struct hisi_drm_private *priv = NULL;
	int key_event;

	HISI_DRM_INFO("[DP] +\n");

	if (encoder == NULL) {
		HISI_DRM_ERR("[DP] NULL pointer -\n");
		return;
	}

	dp_dev = encoder_to_dp(encoder);
	if (dp_dev == NULL) {
		HISI_DRM_ERR("[DP] dp_dev NULL pointer -\n");
		return;
	}

	drm = dp_dev->drm_dev;
	if (drm == NULL) {
		HISI_DRM_ERR("[DP] drm_dev NULL pointer -\n");
		return;
	}

	priv =  drm->dev_private;
	if (priv == NULL) {
		HISI_DRM_ERR("[DP] dev_private NULL pointer -\n");
		return;
	}
	key_event = gpio_get_value_cansleep(dp_dev->dp_hpd_gpio);
	dptx = &dp_dev->dp;
	if (!key_event) {
		disable_dp_ldi(dptx->dss_base);
		dp_check_itf_status(dptx->dss_base);
		dp_off(dp_dev);
	} else {
		HISI_DRM_INFO("hpd on,do not off dp\n");
	}

	HISI_DRM_INFO("[DP] -\n");
}

static const struct drm_encoder_helper_funcs hisi_dp_encoder_helper_funcs = {
	.mode_set = hisi_dp_encoder_mode_set,
	.enable = hisi_dp_encoder_enable,
	.disable = hisi_dp_encoder_disable,
	.atomic_check = hisi_dp_encoder_atomic_check,
};

static const struct drm_encoder_funcs hisi_dp_encoder_funcs = {
	.destroy = drm_encoder_cleanup,
};

static int init_hpd_gpio(struct hisi_dp_device *dp_dev)
{
	int ret;
	int irq_num;

	ret = devm_gpio_request(dp_dev->dev, dp_dev->dp_hpd_gpio, "gpio_hpd");
	if (ret) {
		HISI_DRM_ERR("[DP] Fail request gpio:%d\n", dp_dev->dp_hpd_gpio);
		return ret;
	}

	ret = gpio_direction_input(dp_dev->dp_hpd_gpio);
	if (ret < 0) {
		HISI_DRM_ERR("[DP] Failed to set gpio_down direction!\n");
		return ret;
	}

	irq_num = gpio_to_irq(dp_dev->dp_hpd_gpio);
	if (irq_num < 0) {
		HISI_DRM_ERR("[DP] Failed to get dp_hpd_gpio irq!\n");
		ret = -EINVAL;
		return ret;
	}

	ret = devm_request_threaded_irq(dp_dev->dev,
		irq_num, hpd_gpio_irq, hpd_gpio_irq_thread,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		"hpd_gpio", (void *)dp_dev);
	if (ret) {
		HISI_DRM_ERR("[DP] Failed to request press interupt handler!\n");
		return ret;
	}
	dp_dev->dp.hpd_irq = irq_num;
	HISI_DRM_INFO("[DP] hpd_irq %d!\n", irq_num);

	return 0;
}

static const struct drm_display_mode default_mode = {
	DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 40000, 800, 840,
	968, 1056, 0, 600, 601, 605, 628, 0,
	DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)
};

static int dp_i2c_check_aux_reply(struct dp_ctrl *dptx, u8 *reply)
{
	u32 auxsts;
	int count = 0;

	while (1) {
		auxsts = dptx_readl(dptx, DPTX_AUX_STS);

		if (!(auxsts & DPTX_AUX_STS_REPLY_RECEIVED))
			break;

		count++;
		if (count > 5000) {
			HISI_DRM_INFO("[DP] count > 5000\n");
			return -ETIMEDOUT;
		}
		udelay(1);
	};

	auxsts = dptx_readl(dptx, DPTX_AUX_STS);

	*reply = (auxsts & DPTX_AUX_STS_STATUS_MASK) >>
		DPTX_AUX_STS_STATUS_SHIFT;

	return 0;
}

static struct hisi_dp_device *aux_to_dp_dev(struct drm_dp_aux *aux)
{
	return container_of(aux, struct hisi_dp_device, aux);
}

static ssize_t hisi_dp_aux_transfer(struct drm_dp_aux *aux,
		struct drm_dp_aux_msg *msg)
{
	u8 bytes[1];
	u32 len;
	int ret;
	bool rw = false;
	bool addr_only = false;
	bool i2c = true;
	bool mot = false;
	u8 *buf = NULL;
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;

	if (!aux) {
		HISI_DRM_ERR("[DP] aux is null\n");
		return 0;
	}

	if (!msg) {
		HISI_DRM_ERR("[DP] msg is null\n");
		return 0;
	}

	HISI_DRM_INFO("[DP] +\n");

	dp_dev = aux_to_dp_dev(aux);
	dptx = &dp_dev->dp;
	buf = (u8 *)(msg->buffer);

	if (!dptx->video_transfer_enable) {
		HISI_DRM_INFO("[DP] DP is disabled!\n");
		return 0;
	}
	mutex_lock(&dptx->dptx_mutex);
	if (!dptx->dptx_enable) {
		HISI_DRM_ERR("[DP] dp has not connect\n");
		mutex_unlock(&dptx->dptx_mutex);
		return 0;
	}

	len = msg->size;
	if (msg->size == 0 && !msg->buffer) {
		addr_only = true;
		len = 1;
		buf = bytes;
	}
	mot = (msg->request & DP_AUX_I2C_MOT) ? true : false;

	switch (msg->request & ~DP_AUX_I2C_MOT) {
	case DP_AUX_NATIVE_WRITE:
		i2c = false;
	case DP_AUX_I2C_WRITE:
	case DP_AUX_I2C_WRITE_STATUS_UPDATE:
		rw = false;
		break;
	case DP_AUX_NATIVE_READ:
		i2c = false;
	case DP_AUX_I2C_READ:
		rw = true;
		break;
	default:
		HISI_DRM_ERR("[DP] invalid msg request\n");
		ret = -EINVAL;
		goto err_out;
	}

	HISI_DRM_DEBUG("[DP]addr 0x%x, rw %d, i2c %d, mot %d, addr_only %d, len %d\n",
	  msg->address, rw ,i2c, mot, addr_only, len);

	ret = dptx->aux_rw(dptx, rw, i2c, mot, addr_only,
		msg->address, buf, len);
	if (ret)
		goto err_out;

	ret = dp_i2c_check_aux_reply(dptx, &(msg->reply));
	if (ret)
		goto err_out;

	mutex_unlock(&dptx->dptx_mutex);
	HISI_DRM_INFO("[DP] -\n");
	return msg->size;
err_out:
	mutex_unlock(&dptx->dptx_mutex);
	HISI_DRM_INFO("[DP] err_out -\n");
	return ret;

}

static void dp_i2c_over_aux_init(struct device *dev, struct hisi_dp_device *dp_dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	HISI_DRM_INFO("[DP] +\n");
	dp_dev->aux.name = "dp-aux";
	dp_dev->aux.dev = &pdev->dev;
	dp_dev->aux.transfer = hisi_dp_aux_transfer;
	drm_dp_aux_register(&dp_dev->aux);
	HISI_DRM_INFO("[DP] -\n");
}
static int hisi_dp_bind(struct device *dev, struct device *master, void *data)
{
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;
	struct drm_encoder *encoder = NULL;
	struct drm_connector *connector = NULL;
	struct drm_device *drm_dev = NULL;
	int ret;
	UNUSED(master);
	if (dev == NULL || data == NULL) {
		HISI_DRM_INFO("[DP] NULL Pointer");
		return -EINVAL;
	}

	dp_dev = dev_get_drvdata(dev);

	if (dp_dev == NULL) {
		HISI_DRM_INFO("[DP] dp_dev NULL Pointer");
		return -EINVAL;
	}

	dptx = &dp_dev->dp;
	drm_dev = data;

	HISI_DRM_INFO("[DP] hisi dp bind +");

	dp_dev->drm_dev = drm_dev;
	encoder = &dp_dev->encoder;

	encoder->possible_crtcs = 1 << 1;

	HISI_DRM_INFO("possible_crtcs = 0x%x\n", encoder->possible_crtcs);

	ret = drm_encoder_init(drm_dev, encoder, &hisi_dp_encoder_funcs,
				   DRM_MODE_ENCODER_TMDS, NULL);
	if (ret) {
		HISI_DRM_ERR("failed to initialize encoder with drm\n");
		return ret;
	}

	drm_encoder_helper_add(encoder, &hisi_dp_encoder_helper_funcs);

	connector = &dp_dev->connector;
	connector->polled = DRM_CONNECTOR_POLL_HPD;
	connector->dpms = DRM_MODE_DPMS_OFF;

	if (dptx->output_mode != PRODUCT_TYPE_LAPTOP) {
		ret = drm_connector_init(drm_dev, connector,
					 &hisi_dp_atomic_connector_funcs,
					 DRM_MODE_CONNECTOR_HDMIA);
		dp_i2c_over_aux_init(dev, dp_dev);
	} else {
		ret = drm_connector_init(drm_dev, connector,
					 &hisi_dp_atomic_connector_funcs,
					 DRM_MODE_CONNECTOR_DisplayPort);
	}

	if (ret) {
		HISI_DRM_ERR("failed to initialize connector with drm\n");
		goto err_free_encoder;
	}

	drm_connector_helper_add(connector, &hisi_dp_connector_helper_funcs);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	ret = drm_mode_connector_attach_encoder(connector, encoder);
#else
	ret = drm_connector_attach_encoder(connector, encoder);
#endif
	if (ret) {
		HISI_DRM_ERR("failed to attach connector and encoder\n");
		goto err_free_connector;
	}

	drm_mode_copy(&dp_dev->cur_mode, &default_mode);

	ret = dp_clock_setup(dev, dp_dev);
	if (ret) {
		HISI_DRM_ERR("[DP] DP clock setup failed!\n");
		return ret;
	}

#ifndef CONFIG_HISI_DSS_V600
	// use for hisi_dptx_ready
	g_dp_pdev = to_platform_device(dev);
#endif


	HISI_DRM_INFO("[DP] hisi dp bind -");

	return 0;

err_free_connector:
	drm_connector_cleanup(connector);
err_free_encoder:
	drm_encoder_cleanup(encoder);
	return ret;

}

static void hisi_dp_unbind(struct device *dev, struct device *master, void *data)
{
	struct hisi_dp_device *dp_dev = dev_get_drvdata(dev);
	struct drm_encoder *encoder = &dp_dev->encoder;
	struct drm_connector *connector = &dp_dev->connector;

	HISI_DRM_INFO("[DP] hisi dp unbind +");

	disable_irq(dp_dev->dp.hpd_irq);
	dp_off(dp_dev);

	encoder->funcs->destroy(encoder);
	connector->funcs->destroy(connector);

	HISI_DRM_INFO("[DP] hisi dp unbind -");
}


static const struct component_ops hisi_dp_ops = {
	.bind	= hisi_dp_bind,
	.unbind	= hisi_dp_unbind,
};

static int get_dss_reg_base(struct dp_ctrl *dptx, struct device_node *np)
{
	int ret = 0;

	dptx->dss_base = of_iomap(np, 0);
	if (!dptx->dss_base) {
		HISI_DRM_ERR("[DP] failed to get hisifd dss base resource.\n");
		return -ENXIO;
	}
	HISI_DRM_INFO("[DP] dss_base 0x%lx", dptx->dss_base);

	dptx->peri_crg_base = of_iomap(np, 1);
	if (!dptx->peri_crg_base) {
		HISI_DRM_ERR("[DP] failed to get peri_crg_base resource.\n");
		return -ENXIO;
	}
	HISI_DRM_INFO("[DP] peri crg base 0x%lx", dptx->peri_crg_base);

	dptx->pmctrl_base = of_iomap(np, 6);
	if (!dptx->pmctrl_base) {
		HISI_DRM_ERR("[DP] failed to get pmctrl_base resource.\n");
		return -ENXIO;
	}
	HISI_DRM_INFO("[DP] pmctrl_base 0x%lx", dptx->pmctrl_base);

	dptx->dp_base = of_iomap(np, 9);
	if (dptx->dp_base == NULL) {
		HISI_DRM_ERR("[DP] failed to get dp_base resource\n");
		return -ENXIO;
	}
	HISI_DRM_INFO("[DP] dp_base 0x%lx", dptx->dp_base);

	dptx->mmc0_crg = of_iomap(np, 11);
	if (!dptx->mmc0_crg) {
		HISI_DRM_ERR("[DP] failed to get mmc0_crg_base resource.\n");
		return -ENXIO;
	}
	HISI_DRM_INFO("[DP] mmc0_crg 0x%lx", dptx->mmc0_crg);

	dptx->dp_phy_base = of_iomap(np, 12);
	if (!dptx->dp_phy_base) {
		HISI_DRM_ERR("[DP] failed to get dp_phy_base resource.\n");
		return -ENXIO;
	}
	HISI_DRM_INFO("[DP] dp_phy_base 0x%lx", dptx->dp_phy_base);

	ret = of_property_read_u32(np, "product_type", &dptx->output_mode);
	if (ret < 0)
		HISI_DRM_WARN("get output_mode dts config failed!");

	HISI_DRM_INFO("get output_mode = %d !", dptx->output_mode);

	return ret;
}

static int get_dss_resource(struct dp_ctrl *dptx, struct device_node *np, struct hisi_dp_device *dp_dev)
{
	int ret;

	dptx->irq = irq_of_parse_and_map(np, 5);
	if (!dptx->irq) {
		HISI_DRM_ERR("[DP] failed to get irq_dptx resource.\n");
		return -ENXIO;
	}
	HISI_DRM_INFO("[DP] irq 0x%x", dptx->irq);

	ret = of_property_read_string_index(np, "clock-names", 0,
					&dp_dev->dss_auxclk_dpctrl_name);
	if (ret != 0) {
		HISI_DRM_ERR("failed to get auxclk_dpctrl resource! ret=%d.\n", ret);
		return ret;
	}
	HISI_DRM_INFO("auxclk_dpctrl: %s", dp_dev->dss_auxclk_dpctrl_name);

	ret = of_property_read_string_index(np, "clock-names", 1,
					&dp_dev->dss_pclk_dpctrl_name);
	if (ret != 0) {
		HISI_DRM_ERR("failed to get pclk_dpctrl resource! ret=%d.\n", ret);
		return ret;
	}
	HISI_DRM_INFO("pclk_dpctrl: %s", dp_dev->dss_pclk_dpctrl_name);

	ret = of_property_read_string_index(np, "clock-names", 2,
					&dp_dev->dss_aclk_dpctrl_name);
	if (ret != 0) {
		HISI_DRM_ERR("failed to get aclk_dpctrl resource! ret=%d.\n", ret);
		return ret;
	}
	HISI_DRM_INFO("aclk_dpctrl: %s", dp_dev->dss_aclk_dpctrl_name);

	return ret;
}

static int dp_dts_parse(struct hisi_dp_device *dp_dev)
{
	struct dp_ctrl *dptx = &dp_dev->dp;
	struct device_node *np = NULL;
	int ret;
	int i;

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SWING_VALUE);
	if (np == NULL) {
		HISI_DRM_ERR("[DP] NOT FOUND device node %s!\n", DTS_COMP_SWING_VALUE);
		return -ENOMEM;
	}
	for (i = 0; i < DPTX_COMBOPHY_PARAM_NUM; i++) {
		ret = of_property_read_u32_index(np, "preemphasis_swing", i, &(dptx->combophy_pree_swing[i]));
		if (ret) {
			HISI_DRM_ERR("[DP] preemphasis_swing[%d] is got fail\n", i);
			return ret;
		}
	}
	ret = of_property_read_u32(np, "dp_hpd_gpio", &dp_dev->dp_hpd_gpio);
	if (ret < 0) {
		HISI_DRM_ERR("get dp_hpd_gpio dts config failed\n");
		return ret;
	}

	/* get dss reg base */
	ret = get_dss_reg_base(dptx, np);

	/* get dss resource */
	ret = get_dss_resource(dptx, np, dp_dev);

	np = of_find_compatible_node(NULL, NULL, DTS_DP_AUX_SWITCH);
	if (np == NULL) {
		dptx->edid_try_count = MAX_AUX_RETRY_COUNT;
		dptx->edid_try_delay = AUX_RETRY_DELAY_TIME;
	} else {
		ret = of_property_read_u32(np, "edid_try_count", &dptx->edid_try_count);
		if (ret < 0)
			dptx->edid_try_count = MAX_AUX_RETRY_COUNT;

		ret = of_property_read_u32(np, "edid_try_delay", &dptx->edid_try_delay);
		if (ret < 0)
			dptx->edid_try_delay = AUX_RETRY_DELAY_TIME;
	}
	HISI_DRM_INFO("[DP] edid try count=%d, delay=%d ms.\n", dptx->edid_try_count, dptx->edid_try_delay);
	return 0;
}

static int dp_device_params_init(struct dp_ctrl *dptx)
{
	mutex_init(&dptx->dptx_mutex);
	init_waitqueue_head(&dptx->waitq);
	init_waitqueue_head(&dptx->transfer_wait);
	init_waitqueue_head(&dptx->suspend_wait);
	atomic_set(&(dptx->sink_request), 0);
	atomic_set(&(dptx->shutdown), 0);
	atomic_set(&(dptx->c_connect), 0);

	dptx->max_rate = DPTX_PHYIF_CTRL_RATE_HBR2;
	dptx->dsc_decoders = DSC_DEFAULT_DECODER;

	dptx->dummy_dtds_present = false;
	dptx->selected_est_timing = NONE;
	dptx->dptx_vr = false;
	dptx->dptx_gate = false;
	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;
	dptx->dptx_detect_inited = false;
	dptx->user_mode = 0;
	dptx->detect_times = 0;
	dptx->dptx_plug_type = DP_PLUG_TYPE_NORMAL;
	dptx->user_mode_format = VCEA;
	dptx->hpd_enable = false;
	dptx->max_edid_timing_hactive = 0;

	dptx->bstatus = 0;

	dptx->dsc_ifbc_type = IFBC_TYPE_VESA3X_DUAL;
#if CONFIG_DP_ENABLE
	dptx->same_source = get_current_dp_source_mode();
	dp_debug_init_combophy_pree_swing(dptx->combophy_pree_swing, DPTX_COMBOPHY_PARAM_NUM);
#endif
	return 0;
}

static int dp_device_buf_alloc(struct dp_ctrl *dptx)
{
	int ret;

	dptx->edid_second = kzalloc(DPTX_DEFAULT_EDID_BUFLEN, GFP_KERNEL);
	if (dptx->edid_second == NULL) {
		HISI_DRM_ERR("[DP] dptx base is NULL!\n");
		return -ENOMEM;
	}

	ret = memset_s(dptx->edid_second, DPTX_DEFAULT_EDID_BUFLEN, 0, DPTX_DEFAULT_EDID_BUFLEN);
	if (ret != EOK) {
		kfree(dptx->edid_second);
		dptx->edid_second = NULL;
		HISI_DRM_ERR("memset failed");
		return -ENOMEM;
	}

	dptx->edid = kzalloc(DPTX_DEFAULT_EDID_BUFLEN, GFP_KERNEL);
	if (dptx->edid == NULL) {
		HISI_DRM_ERR("[DP] dptx base is NULL!\n");
		return -ENOMEM;
	}
	ret = memset_s(dptx->edid, DPTX_DEFAULT_EDID_BUFLEN, 0, DPTX_DEFAULT_EDID_BUFLEN);
	if (ret != EOK) {
		kfree(dptx->edid_second);
		kfree(dptx->edid);
		dptx->edid_second = NULL;
		dptx->edid = NULL;
		HISI_DRM_ERR("memset failed");
		return -ENOMEM;
	}

	return 0;
}

static int dp_irq_setup(struct dp_ctrl *dptx, struct hisi_dp_device *dp_dev)
{
	int ret;

	ret = devm_request_threaded_irq(dptx->dev,
		dptx->irq, dptx->dptx_irq, dptx->dptx_threaded_irq,
		IRQF_SHARED | IRQ_LEVEL, "dwc_dptx", (void *)dp_dev);
	if (ret) {
		HISI_DRM_ERR("[DP] Request for irq %d failed!\n", dptx->irq);
		return ret;
	}

	disable_irq(dptx->irq);

	ret = init_hpd_gpio(dp_dev);
	if (ret) {
		HISI_DRM_ERR("[DP] Request for hpd_irq %d failed!\n", dptx->hpd_irq);
		return ret;
	}
	disable_irq(dptx->hpd_irq);
	dptx->hpd_enable = false;

	return 0;
}

static int dp_device_init(struct platform_device *pdev, struct hisi_dp_device *dp_dev)
{
	struct dp_ctrl *dptx = &dp_dev->dp;
	int ret;

	HISI_DRM_INFO("[DP] dp device init +\n");

	dp_dev->dev = &pdev->dev;
	dptx->dev = &pdev->dev;
	dptx->dp_dev = dp_dev;

	ret = dp_dts_parse(dp_dev);
	if (ret) {
		HISI_DRM_ERR("[DP] dp_dts_parse failed!\n");
		return ret;
	}

	ret = dp_device_params_init(dptx);
	if (ret)
		return ret;

	dptx_link_layer_init(dptx);
	if (dptx->dptx_default_params_from_core)
		dptx->dptx_default_params_from_core(dptx);

	ret = dp_device_buf_alloc(dptx);
	if (ret)
		goto err_edid_alloc;

	ret = dp_irq_setup(dptx, dp_dev);
	if (ret)
		goto err_sdev_hdmi_register;

	HISI_DRM_INFO("[DP] dp device init -.\n");

	return 0;

err_sdev_hdmi_register:

err_edid_alloc:
	if (dptx->edid != NULL) {
		kfree(dptx->edid);
		dptx->edid = NULL;
	}

	if (dptx->edid_second != NULL) {
		kfree(dptx->edid_second);
		dptx->edid_second = NULL;
	}

	return ret;
}

static int hisi_dp_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;

	if (pdev == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	dp_dev = platform_get_drvdata(pdev);
	if (dp_dev == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_DRM_DEBUG("[DP] dp_remove, +.\n");

	dptx = &(dp_dev->dp);

	free_irq(dptx->hpd_irq, dp_dev);

	if (dptx->edid != NULL) {
		kfree(dptx->edid);
		dptx->edid = NULL;
	}

	if (dptx->edid_second != NULL) {
		kfree(dptx->edid_second);
		dptx->edid_second = NULL;
	}

	dptx->dptx_vr = false;
	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;
	if (dptx->dptx_core_remove)
		dptx->dptx_core_remove(dptx);

	dptx_notify_shutdown(dptx);
	mdelay(15);

	component_del(&pdev->dev, &hisi_dp_ops);

	HISI_DRM_INFO("[DP] dp_remove, -.\n");

	return ret;
}

static int hisi_dp_probe(struct platform_device *pdev)
{
	struct hisi_dp_device *dp_dev = NULL;
	int ret;

	if (pdev == NULL) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	dp_dev = devm_kzalloc(&pdev->dev, sizeof(struct hisi_dp_device),
			  GFP_KERNEL);
	if (!dp_dev) {
		HISI_DRM_ERR("[DP] devm_kzalloc fail\n");
		return -ENOMEM;
	}

	ret = dp_device_init(pdev, dp_dev);
	if (ret) {
		HISI_DRM_ERR("[DP] dp_device_init failed, error=%d!\n", ret);
		return -EINVAL;
	}

	/*
	 * We just use the drvdata until driver run into component
	 * add function, and then we would set drvdata to null
	 */
	platform_set_drvdata(pdev, dp_dev);

	ret = component_add(&pdev->dev, &hisi_dp_ops);
	if (ret) {
		HISI_DRM_ERR("[DP] component_add failed, error=%d!\n", ret);
		return ret;
	}

	HISI_DRM_INFO("hisi dp probe -\n");

	return ret;
}

static const struct of_device_id hisi_dp_match_table[] = {
	{
		.compatible = DTS_COMP_SWING_VALUE,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_dp_match_table);

struct platform_driver hisi_dp_driver = {
	.probe = hisi_dp_probe,
	.remove = hisi_dp_remove,
	.suspend = NULL, // sr will trigger by usb
	.resume = NULL,
	.shutdown = NULL,

	.driver = {
		.name = "hisi-drm-dp",
		.owner = THIS_MODULE,
		.of_match_table = hisi_dp_match_table,
	},

};

/*lint +e86 +e438 +e529 +e546 +e550 +e559 +e578 +e563 +e571 +e574 +e648 +e666 +e712 +e747 +e580 +e616*/
