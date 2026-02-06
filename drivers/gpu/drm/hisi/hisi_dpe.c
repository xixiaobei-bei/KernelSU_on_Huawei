/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
/*lint -e529 -e530 -e559 -e578 -e666*/
#include <securec.h>

#include "hisi_dpe.h"
#include "hisi_drm_drv.h"

#include "hisi_drm_dpe_utils.h"
#include "hisi_mipi_dsi.h"
#include "hisi_dp_drv.h"

#if defined(CONFIG_HISI_PERIDVFS)
#include "peri_volt_poll.h"
#endif

#define PERI_VOLTAGE_LEVEL0_060V (0) /* 0.60v */
#define PERI_VOLTAGE_LEVEL1_065V (1) /* 0.65v */
#define PERI_VOLTAGE_LEVEL2_070V (2) /* 0.70v */
#define PERI_VOLTAGE_LEVEL3_080V (3) /* 0.80v */

static int get_max_votelevel(struct hisi_drm_crtc *hisi_crtc,
		      u32 *max_vote_level)
{
	u32 target_voltage;
	struct hisi_drm_crtc *hisi_drm_crtc = NULL;
	struct drm_device *dev = NULL;
	struct drm_crtc *crtc = NULL;
	u32 voltage_level;

	target_voltage = PERI_VOLTAGE_LEVEL0;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return -EINVAL;
	}

	dev = hisi_crtc->base.dev;
	drm_for_each_crtc(crtc, dev) {
		if (!crtc->state || !crtc->state->active)
			continue;

		hisi_drm_crtc = to_hisi_drm_crtc(crtc);
		voltage_level = hisi_drm_crtc->dss_vote_cmd.dss_voltage_level;
		if (voltage_level > target_voltage) {
			target_voltage = voltage_level;
			HISI_DRM_DEBUG("get target_voltage=%d, crtc-%d",
				       target_voltage, hisi_drm_crtc->crtc_id);
		}
	}

	*max_vote_level = target_voltage;

	return 0;
}

static int get_base_for_ctx(struct device_node *np, struct dss_hw_ctx *ctx)
{
	ctx->dss_base = of_iomap(np, 0);
	if (!ctx->dss_base) {
		HISI_DRM_ERR("failed to get dss_base");
		return -ENXIO;
	}

	ctx->peri_crg_base = of_iomap(np, 1);
	if (!ctx->peri_crg_base) {
		HISI_DRM_ERR("failed to get peri_crg_base");
		return -ENXIO;
	}

	ctx->sctrl_base = of_iomap(np, 2);
	if (!ctx->sctrl_base) {
		HISI_DRM_ERR("failed to get sctrl_base");
		return -ENXIO;
	}

	ctx->pctrl_base = of_iomap(np, 3);
	if (!ctx->pctrl_base) {
		HISI_DRM_ERR("failed to get pctrl_base");
		return -ENXIO;
	}

	ctx->noc_dss_base = of_iomap(np, 4);
	if (!ctx->noc_dss_base) {
		HISI_DRM_ERR("failed to get noc_dss_base");
		return -ENXIO;
	}

	ctx->mmbuf_crg_base = of_iomap(np, 5);
	if (!ctx->mmbuf_crg_base) {
		HISI_DRM_ERR("failed to get mmbuf_crg_base");
		return -ENXIO;
	}

	ctx->pmctrl_base = of_iomap(np, 6);
	if (!ctx->pmctrl_base) {
		HISI_DRM_ERR("failed to get pmctrl_base");
		return -ENXIO;
	}

	ctx->media_crg_base = of_iomap(np, 7);
	if (!ctx->media_crg_base) {
		HISI_DRM_ERR("failed to get media_crg_base");
		return -ENXIO;
	}

	ctx->media_common_base = of_iomap(np, 10);
	if (!ctx->media_common_base) {
		HISI_DRM_ERR("failed to get media_common_base");
		return -ENXIO;
	}
	return 0;
}

int dpe_dts_parse(struct platform_device *pdev, struct dss_hw_ctx *ctx)
{
	struct device_node *np = NULL;
	int ret;
	int i;

	UNUSED(pdev);
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return -EINVAL;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_DSS_NAME);
	if (!np) {
		HISI_DRM_ERR("NOT FOUND device node %s!", DTS_COMP_DSS_NAME);
		return -ENXIO;
	}

	ctx->dp_irq = irq_of_parse_and_map(np, 7);
	if (!ctx->dp_irq) {
		HISI_DRM_ERR("failed to get irq_dp");
		return -ENXIO;
	}

	ret = get_base_for_ctx(np, ctx);
	if (ret)
		return ret;

	ret = of_property_read_u32(np, "dss_base_phy", &ctx->dss_base_phy);
	if (ret) {
		HISI_DRM_ERR("failed to get dss_base_phy, ret=%d\n", ret);
		return -ENXIO;
	}

	for (i = 0; i < DPE_CLK_NUM; i++) {
		ret = of_property_read_string_index(np, "clock-names", i,
						    (const char **)&ctx->dpe_clk_name[i]);
		if (ret != 0) {
			HISI_DRM_ERR("failed to get axi_clk, ret=%d.\n", ret);
			return -ENXIO;
		}
	}
	return 0;
}

extern struct platform_device *g_dp_pdev;

struct dp_ldi_info {
	int hfp;
	int hbp;
	int hpw;
	int vfp;
	int vbp;
	int vpw;
	int dpi_hbp;
	int dpi_hsw;
	int vsync_plr;
	int hsync_plr;
	int pxl_clk_rate_div;
	int pxl_clk_rate;
};

static void set_dp_ldi_param(struct dp_ldi_info *dp_ldi_param, struct drm_display_mode *dmode)
{
	dp_ldi_param->hfp = dmode->hsync_start - dmode->hdisplay;
	dp_ldi_param->hpw = dmode->hsync_end - dmode->hsync_start;
	dp_ldi_param->hbp = dmode->htotal - dmode->hsync_end;

	dp_ldi_param->vfp = dmode->vsync_start - dmode->vdisplay;
	dp_ldi_param->vpw = dmode->vsync_end - dmode->vsync_start;
	dp_ldi_param->vbp = dmode->vtotal - dmode->vsync_end;

	dp_ldi_param->vsync_plr = (dmode->flags & DRM_MODE_FLAG_PVSYNC) ? 0 : 1;
	dp_ldi_param->hsync_plr = (dmode->flags & DRM_MODE_FLAG_PHSYNC) ? 0 : 1;
	dp_ldi_param->pxl_clk_rate_div = 1;
	dp_ldi_param->pxl_clk_rate = dmode->clock / 2;

}
static void check_mipi_video_mode(struct dp_ldi_info *dp_ldi_param, struct hisi_drm_crtc *hisi_crtc)
{
	if (is_mipi_video_mode(hisi_crtc)) {
		dp_ldi_param->dpi_hbp = dp_ldi_param->hbp + DSS_WIDTH(dp_ldi_param->hpw);
		dp_ldi_param->dpi_hsw = 0;
	} else {
		dp_ldi_param->dpi_hbp = dp_ldi_param->hbp;
		dp_ldi_param->dpi_hsw = DSS_WIDTH(dp_ldi_param->hpw);
	}
}

static void set_reg_for_mipi_cmd_mode(char __iomem *ldi_base,
	struct dp_ldi_info **dp_ldi_param, struct hisi_drm_crtc *hisi_crtc)
{
	u32 te_source;

	/* for 1Hz LCD and mipi command LCD */
	if (is_mipi_cmd_mode(hisi_crtc)) {
		set_reg(ldi_base + LDI_DSI_CMD_MOD_CTRL, 0x1, 2, 0);

		/* DSI_TE_CTRL
		 * te_source = 0, select te_pin
		 * te_source = 1, select te_triger
		 */
		te_source = 0;

		/* dsi_te_hard_en */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x1, 1, 0);
		/* dsi_te0_pin_p , dsi_te1_pin_p */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 2, 1);
		/* dsi_te_hard_sel */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, te_source, 1, 3);
		/* select TE0 PIN */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x01, 2, 6);
		/* dsi_te_mask_en */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 1, 8);
		/* dsi_te_mask_dis */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 4, 9);
		/* dsi_te_mask_und */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 4, 13);
		/* dsi_te_pin_en */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x1, 1, 17);

		set_reg(ldi_base + LDI_DSI_TE_HS_NUM, 0x0, 32, 0);
		set_reg(ldi_base + LDI_DSI_TE_HS_WD, 0x24024, 32, 0);

		if ((*dp_ldi_param)->pxl_clk_rate_div == 0) {
			HISI_DRM_ERR("pxl_clk_rate_div is NULL\n");
			(*dp_ldi_param)->pxl_clk_rate_div = 1;
		}
		set_reg(ldi_base + LDI_DSI_TE_VS_WD, (0x3FC << 12) |
			(2 * (*dp_ldi_param)->pxl_clk_rate / (*dp_ldi_param)->pxl_clk_rate_div / 1000000),
			32, 0);
	} else {
		/* dsi_te_hard_en */
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 1, 0);
		set_reg(ldi_base + LDI_DSI_CMD_MOD_CTRL, 0x2, 2, 0);
	}

}

static void set_reg_for_dp_ldi(struct dp_ctrl *dp, char __iomem **ldi_base,
	struct dss_hw_ctx *ctx, struct dp_ldi_info *dp_ldi_param, struct hisi_drm_crtc *hisi_crtc,
		struct drm_display_mode *dmode)
{
	int i;

	for (i = 0; i < dp->streams; i++) {
		if (i == 0)
			*ldi_base = ctx->dss_base + DSS_LDI_DP_OFFSET;
		else
			*ldi_base = ctx->dss_base + DSS_LDI_DP1_OFFSET;

		outp32(*ldi_base + LDI_DPI0_HRZ_CTRL0, dp_ldi_param->hfp | (dp_ldi_param->dpi_hbp << 16));
		outp32(*ldi_base + LDI_DPI0_HRZ_CTRL1, dp_ldi_param->dpi_hsw);
		outp32(*ldi_base + LDI_DPI0_HRZ_CTRL2,
			   DSS_WIDTH(dmode->hdisplay));

		outp32(*ldi_base + LDI_VRT_CTRL0, dp_ldi_param->vfp | (dp_ldi_param->vbp << 16));
		outp32(*ldi_base + LDI_VRT_CTRL1, DSS_HEIGHT(dp_ldi_param->vpw));
		outp32(*ldi_base + LDI_VRT_CTRL2, DSS_HEIGHT(dmode->vdisplay));

		outp32(*ldi_base + LDI_PLR_CTRL, dp_ldi_param->vsync_plr | (dp_ldi_param->hsync_plr << 1));

		/* for ddr pmqos */
		outp32(*ldi_base + LDI_VINACT_MSK_LEN, dp_ldi_param->vfp);

		/* cmd event sel */
		outp32(*ldi_base + LDI_CMD_EVENT_SEL, 0x1);

		/* double pixel mode 1772 */
		outp32(*ldi_base + LDI_MP_MODE, 0x1);

		set_reg_for_mipi_cmd_mode(*ldi_base, &dp_ldi_param, hisi_crtc);

#ifdef CONFIG_HISI_FB_LDI_COLORBAR_USED
		/* colorbar width */
		set_reg(*ldi_base + LDI_CTRL, DSS_WIDTH(0x3c), 7, 6);
		/* colorbar ort */
		set_reg(*ldi_base + LDI_WORK_MODE, 0x0, 1, 1);
		/* colorbar enable */
		set_reg(*ldi_base + LDI_WORK_MODE, 0x0, 1, 0);
#else
		/* normal */
		set_reg(*ldi_base + LDI_WORK_MODE, 0x1, 1, 0);
#endif

		if (is_mipi_cmd_mode(hisi_crtc))
			set_reg(*ldi_base + LDI_FRM_MSK, 0x1, 1, 0);

		if (hisi_crtc->crtc_id == EXTERNAL_IDX &&
			is_mipi_mode(hisi_crtc))
			set_reg(*ldi_base + LDI_DP_DSI_SEL, 0x1, 1, 0);

		/* mst mode */
		if (dp->mst)
			set_reg(*ldi_base + LDI_MST_CONTROL, 0x1, 1, 0);

		if (i == 1)
			set_reg(*ldi_base + LDI_WORK_MODE, 0x0, 1, 0);
	}

}

static void init_dp_ldi(struct hisi_drm_crtc *hisi_crtc, bool fastboot_enable)
{
	char __iomem *ldi_base = NULL;
	struct drm_display_mode *dmode = NULL;
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dp = NULL;
	struct dss_hw_ctx *ctx = NULL;
	struct dp_ldi_info dp_ldi_param;

	if (!g_dp_pdev) {
		HISI_DRM_ERR("[DP] g_dp_pdev is NULL!");
		return;
	}

	dp_dev = platform_get_drvdata(g_dp_pdev);
	if (!dp_dev) {
		HISI_DRM_ERR("[DP] hisifd is NULL!");
		return;
	}

	HISI_DRM_INFO("[DP] +");

	dp = &dp_dev->dp;

	dmode = &dp_dev->cur_mode;
	ctx = hisi_crtc->ctx;

	set_dp_ldi_param(&dp_ldi_param, dmode);

	check_mipi_video_mode(&dp_ldi_param, hisi_crtc);

	hisi_display_post_process_chn_init(hisi_crtc);

	set_reg_for_dp_ldi(dp, &ldi_base, ctx, &dp_ldi_param, hisi_crtc, dmode);

	/* ldi disable */
	if (!fastboot_enable)
		disable_dp_ldi(ctx->dss_base);

	HISI_DRM_INFO("[DP] -!\n");
}

static void dpe_reset_dp_pipesw(struct hisi_drm_crtc *hisi_crtc)
{
	u32 value;
	char __iomem *pipe_sw_offset = NULL;
	struct dss_hw_ctx *ctx = NULL;

	ctx = hisi_crtc->ctx;
	pipe_sw_offset = ctx->dss_base + DSS_PIPE_SW_DP_OFFSET;
	value = inp32(pipe_sw_offset + PIPE_SW_SIG_CTRL); //lint !e529
	if (value) {
		set_reg(ctx->dss_base + DSS_LDI_DP_OFFSET + LDI_CTRL, 0x0, 1, 0);
		dpe_check_itf_status(hisi_crtc);

		set_reg(pipe_sw_offset + PIPE_SW_SIG_CTRL, 0x0, 8, 0);
		set_reg(pipe_sw_offset + SW_POS_CTRL_SIG_EN, 0x0, 1, 0);
		set_reg(pipe_sw_offset + PIPE_SW_DAT_CTRL, 0x0, 8, 0);
		set_reg(pipe_sw_offset + SW_POS_CTRL_DAT_EN, 0x0, 1, 0);
	}
}

int dpe_init(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_drm_crtc is NULL");
		return -EINVAL;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("dss_hw_ctx is NULL");
		return -EINVAL;
	}

	HISI_DRM_DEBUG("crtc_id: %d", hisi_crtc->crtc_id);
	/*
	  Because of dp hpd have not come yet,
	  disable dp ldi and adjust pipsw to make sure primary display is normal
	*/
	if (ctx->is_pre_enabled)
		dpe_reset_dp_pipesw(hisi_crtc);

	if (hisi_crtc->crtc_id == PRIMARY_IDX) {
		init_dbuf(hisi_crtc);
		init_dpp(hisi_crtc);
		init_acm(ctx);
		init_dither(ctx);
		init_igm_gmp_xcc_gm(ctx);
		init_ifbc(ctx);
		init_ldi(hisi_crtc);
	} else if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		init_dpp(hisi_crtc);
		init_dbuf(hisi_crtc);
		init_dp_ldi(hisi_crtc, ctx->is_pre_enabled);
	}
	sema_init(&hisi_crtc->sem, 1);
	return 0;
}

static void deinit_ldi(struct hisi_drm_crtc *hisi_crtc)
{
	crtc_disable_ldi(hisi_crtc);
}

int dpe_deinit(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return -EINVAL;
	}
	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL\n");
		return -EINVAL;
	}

	deinit_ldi(hisi_crtc);
	deinit_dbuf(hisi_crtc);

	return 0;
}

void dpe_check_itf_status(struct hisi_drm_crtc *hisi_crtc)
{
	int tmp;
	int delay_count = 0;
	bool is_timeout = true;
	int itf_idx;
	char __iomem *mctl_sys_base = NULL;
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL\n");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("NULL ptr.\n");
		return;
	}

	itf_idx = hisi_crtc->crtc_id;
	mctl_sys_base =  ctx->dss_base + DSS_MCTRL_SYS_OFFSET;

	while (1) {
		tmp = inp32(mctl_sys_base + MCTL_MOD17_STATUS + itf_idx * 0x4); //lint !e529
		if (((tmp & 0x10) == 0x10) || delay_count > 100) {
			is_timeout = (delay_count > 100) ? true : false;
			delay_count = 0;
			break;
		}
		mdelay(1);
		++delay_count;
	}

	if (is_timeout)
		HISI_DRM_INFO("mctl_itf0 not in idle status,ints=0x%x\n", tmp);
}

int dpe_regulator_enable(struct dss_hw_ctx *ctx, int regulator)
{
	int ret;

	if (!ctx) {
		HISI_DRM_ERR("NULL ptr\n");
		return -EINVAL;
	}

	ret = regulator_enable(ctx->dpe_regulator[regulator].consumer);
	if (ret)
		HISI_DRM_ERR("regulator_enable failed, error=%d!\n", ret);

	return ret;
}

int dpe_regulator_disable(struct dss_hw_ctx *ctx, int regulator)
{
	int ret;

	if (!ctx) {
		HISI_DRM_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	ret = regulator_disable(ctx->dpe_regulator[regulator].consumer);
	if (ret)
		HISI_DRM_ERR("regulator_disable failed, error=%d!\n", ret);

	return ret;
}

static void hisi_recovery_external_clk_rate(struct hisi_drm_crtc *hisi_crtc)
{
	int ret;
	u32 current_dss_voltage_level;
	struct dss_vote_cmd target_dss_vote_cmd;
	struct dss_vote_cmd *vote_cmd = NULL;

	vote_cmd = &hisi_crtc->dss_vote_cmd;
	if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L1;
		vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L1;
		vote_cmd->dss_voltage_level = PERI_VOLTAGE_LEVEL0;

		ret = get_max_votelevel(hisi_crtc, &current_dss_voltage_level);
		if (ret) {
			HISI_DRM_ERR("set max votage_value=%d failed,", ret);
			return;
		}

		ret = get_dss_clock_value(current_dss_voltage_level,
					  &target_dss_vote_cmd);
		if (ret) {
			HISI_DRM_ERR("get max dss clock=%d failed,", ret);
			return;
		}

		ret = clk_set_rate(hisi_crtc->ctx->dpe_clk[DPE_PRI_CLK],
				   target_dss_vote_cmd.dss_pri_clk_rate);
		if (ret < 0)
			HISI_DRM_ERR("set pri_clk_rate(%llu) fail, err=%d\n",
				     target_dss_vote_cmd.dss_pri_clk_rate, ret);

		ret = clk_set_rate(hisi_crtc->ctx->dpe_clk[DPE_MMBUF_CLK],
				   target_dss_vote_cmd.dss_mmbuf_rate);
		if (ret < 0)
			HISI_DRM_ERR("set mmbuf_rate(%llu) fail, err=%d\n",
				     target_dss_vote_cmd.dss_mmbuf_rate, ret);
	}
}

static int dpe_set_pixel_clk_rate_on_pll0(struct hisi_drm_crtc *hisi_crtc)
{
	int ret = 0;
	struct dss_hw_ctx *ctx = NULL;
	u64 clk_rate;
#if defined(CONFIG_HISI_PERIDVFS)
	struct peri_volt_poll *pvp = NULL;
#endif

	ctx = hisi_crtc->ctx;
	if (is_dp(hisi_crtc)) {
		hisi_recovery_external_clk_rate(hisi_crtc);

		clk_rate = DSS_PXL1_CLK_RATE_OFF;
		ret = clk_set_rate(ctx->dpe_clk[DPE_PXL1_CLK], clk_rate);
		if (ret < 0)
			HISI_DRM_ERR("crtc-%d, set pxl1 clk fail, err=%d\n",
				     hisi_crtc->crtc_id, ret);
		HISI_DRM_INFO("dss_pxl1_clk:[%llu]->[%llu]\n", clk_rate,
			      clk_get_rate(ctx->dpe_clk[DPE_PXL1_CLK]));

#if defined(CONFIG_HISI_PERIDVFS)
		pvp = peri_volt_poll_get(DEV_LDI1_VOLTAGE_ID, NULL);
		if (!pvp) {
			HISI_DRM_ERR("get pvp failed!\n");
			return -EINVAL;
		}

		ret = peri_set_volt(pvp, PERI_VOLTAGE_LEVEL0_060V);
		if (ret) {
			HISI_DRM_ERR("set voltage_value=0 failed!\n");
			return -EINVAL;
		}
#endif
	}

	return ret;
}

static int set_primary_core_clk(struct hisi_drm_crtc *hisi_crtc,
				struct dss_vote_cmd vote_cmd)
{
	int ret;
	bool set_rate_succ = true;
	u32 current_volt;
	struct dss_vote_cmd target_vote_cmd;
	struct dss_hw_ctx *ctx = NULL;

	ret = memset_s(&target_vote_cmd, sizeof(target_vote_cmd), 0, sizeof(target_vote_cmd));
	if (ret != EOK) {
		HISI_DRM_ERR("memset for target_vote_cmd failed!");
		return -EINVAL;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is null");
		return -EINVAL;
	}

	ret = get_max_votelevel(hisi_crtc, &current_volt);
	if (ret) {
		HISI_DRM_ERR("set max votage_value=%d failed,", ret);
		return -EINVAL;
	}

	ret = get_dss_clock_value(current_volt, &target_vote_cmd);
	if (ret) {
		HISI_DRM_ERR("get max dss clock=%d failed,", ret);
		return -EINVAL;
	}

	if (vote_cmd.dss_pri_clk_rate >= target_vote_cmd.dss_pri_clk_rate) {
		ret = clk_set_rate(ctx->dpe_clk[DPE_PRI_CLK],
				   vote_cmd.dss_pri_clk_rate);
		if (ret < 0) {
			set_rate_succ = false;
			HISI_DRM_ERR("set pri_clk_rate(%llu) fail, err=%d\n",
				     vote_cmd.dss_pri_clk_rate, ret);
		}

		ret = clk_set_rate(ctx->dpe_clk[DPE_MMBUF_CLK],
				   vote_cmd.dss_mmbuf_rate);
		if (ret < 0) {
			set_rate_succ = false;
			HISI_DRM_ERR("set mmbuf_rate(%llu) fail, err=%d\n",
				     vote_cmd.dss_mmbuf_rate, ret);
		}

		HISI_DRM_INFO("CRTC-%d get dss_pri_clk = %llu",
			      hisi_crtc->crtc_id,
			(uint64_t)clk_get_rate(ctx->dpe_clk[DPE_PRI_CLK]));
		HISI_DRM_INFO("CRTC-%d get dss_mmbuf_rate = %llu",
			      hisi_crtc->crtc_id,
			(uint64_t)clk_get_rate(ctx->dpe_clk[DPE_MMBUF_CLK]));
		if (!set_rate_succ)
			return -1;
	}

	hisi_crtc->dss_vote_cmd.dss_pri_clk_rate = vote_cmd.dss_pri_clk_rate;
	hisi_crtc->dss_vote_cmd.dss_axi_clk_rate = vote_cmd.dss_axi_clk_rate;
	hisi_crtc->dss_vote_cmd.dss_mmbuf_rate = vote_cmd.dss_mmbuf_rate;
	HISI_DRM_INFO("set coreClk=%llu, other fb->coreClk=%llu\n",
		      vote_cmd.dss_pri_clk_rate,
		      target_vote_cmd.dss_pri_clk_rate);

	return ret;
}

static int set_dss_vote_cmd(struct hisi_drm_crtc *hisi_crtc,
		     struct dss_vote_cmd vote_cmd)
{
	int ret = 0;
	u64 pri_clk_rate;

	pri_clk_rate = vote_cmd.dss_pri_clk_rate;
	if (pri_clk_rate != DSS_CORE_CLK_RATE_L1 &&
	    pri_clk_rate != DSS_CORE_CLK_RATE_L2 &&
	    pri_clk_rate != DSS_CORE_CLK_RATE_L3 &&
	    pri_clk_rate != DSS_CORE_CLK_RATE_L4) {
		HISI_DRM_ERR("crtc-%d no support set pri_clk_rate(%llu)\n",
			     hisi_crtc->crtc_id, pri_clk_rate);
		return -1;
	}

	if (pri_clk_rate == hisi_crtc->dss_vote_cmd.dss_pri_clk_rate)
		return ret;

	if (hisi_crtc->crtc_id == PRIMARY_IDX ||
	    hisi_crtc->crtc_id == EXTERNAL_IDX)
		ret = set_primary_core_clk(hisi_crtc, vote_cmd);

	HISI_DRM_DEBUG("set dss_pri_clk_rate = %llu\n",
		       pri_clk_rate);
	return ret;
}

static void set_dss_voltage_value(u64 pixel_clock_ori, struct dss_vote_cmd *dss_vote_cmd,
	unsigned int *voltage_value, struct dss_vote_cmd *vote_cmd)
{
	if (pixel_clock_ori > DSS_PXL1_CLK_RATE_L3) {
		dss_vote_cmd->dss_voltage_level = PERI_VOLTAGE_LEVEL3;
		*voltage_value = PERI_VOLTAGE_LEVEL3_080V;
		vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L4;
		vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L4;
	} else if (pixel_clock_ori > DSS_PXL1_CLK_RATE_L2) {
		dss_vote_cmd->dss_voltage_level = PERI_VOLTAGE_LEVEL2;
		*voltage_value = PERI_VOLTAGE_LEVEL2_070V;
		vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L3;
		vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L3;
	} else if (pixel_clock_ori > DSS_PXL1_CLK_RATE_L1) {
		dss_vote_cmd->dss_voltage_level = PERI_VOLTAGE_LEVEL1;
		*voltage_value = PERI_VOLTAGE_LEVEL1_065V;
		vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L2;
		vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L2;
	} else {
		dss_vote_cmd->dss_voltage_level = PERI_VOLTAGE_LEVEL0;
		*voltage_value = PERI_VOLTAGE_LEVEL0_060V;
		vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L1;
		vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L1;
	}
}

static int set_dss_core_clk(unsigned int voltage_value, struct hisi_drm_crtc *hisi_crtc,
	uint64_t pixel_clock, struct dss_vote_cmd vote_cmd, struct dss_vote_cmd *dss_vote_cmd)
{
	int ret;

	/* pixel clock/2 need more than DSS core clk. In double pixel mode,
	 * pixel clock is half of previous.
	 * So it only need pixel clock more than DSS core clk.
	 */
	if (voltage_value > PERI_VOLTAGE_LEVEL0_060V &&
	    pixel_clock > DSS_CORE_CLK_RATE_L1) {
		ret = set_dss_vote_cmd(hisi_crtc, vote_cmd);
		if (ret < 0) {
			HISI_DRM_ERR("[DP] DSS core clk set failed!!\n");
			return ret;
		}
	} else {
		dss_vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L1;
		dss_vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L1;
	}
	return 0;
}

int hisi_set_dss_external_vote_pre(struct hisi_drm_crtc *hisi_crtc,
				   uint64_t pixel_clock)
{
	int ret = 0;
	unsigned int voltage_value;
	int voltage_timeout = 15;
	u64 pixel_clock_ori;
	struct dss_vote_cmd vote_cmd;
	struct dss_vote_cmd *dss_vote_cmd = NULL;
#if defined(CONFIG_HISI_PERIDVFS)
	struct peri_volt_poll *pvp = NULL;
#endif

	if (!hisi_crtc) {
		HISI_DRM_ERR("NULL Pointer!\n");
		return -1;
	}

	if (hisi_crtc->crtc_id != EXTERNAL_IDX)
		return 0;

	pixel_clock_ori = pixel_clock * 2; /* double pixel */
	ret = memset_s(&vote_cmd, sizeof(vote_cmd), 0, sizeof(vote_cmd));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset failed!");

	dss_vote_cmd = &hisi_crtc->dss_vote_cmd;
	set_dss_voltage_value(pixel_clock_ori, dss_vote_cmd, &voltage_value, &vote_cmd);

#if defined(CONFIG_HISI_PERIDVFS)
	pvp = peri_volt_poll_get(DEV_LDI1_VOLTAGE_ID, NULL);
	if (!pvp) {
		HISI_DRM_ERR("[DP] get pvp failed!\n");
		return -EINVAL;
	}

	HISI_DRM_DEBUG("[DP] cur volt :%d\n", peri_get_volt(pvp));
	ret = peri_set_volt(pvp, voltage_value);
	if (ret) {
		HISI_DRM_ERR("[DP] set voltage_value failed!\n");
		return -EINVAL;
	}

	while (voltage_timeout) {
		mdelay(10);
		if (voltage_value <= peri_get_volt(pvp))
			break;
		voltage_timeout--;
	}

	if (voltage_value > peri_get_volt(pvp)) {
		HISI_DRM_ERR("[DP] set volt time out! request %d, cur %d\n", voltage_value, peri_get_volt(pvp));
		return -EINVAL;
	}
	HISI_DRM_INFO("[DP] voltage request: %d, cur: %d\n", voltage_value, peri_get_volt(pvp));
#endif

	set_dss_core_clk(voltage_value, hisi_crtc, pixel_clock, vote_cmd, dss_vote_cmd);
	if (ret)
		return ret;

	return 0;
}

static int dpe_clk_enable(struct dss_hw_ctx *ctx, int clk)
{
	int ret;
	struct clk *clk_tmp = NULL;

	clk_tmp = ctx->dpe_clk[clk];
	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_DRM_ERR("clk_prepare failed, error=%d, clk=%d!\n",
				     ret, clk);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_DRM_ERR("clk_enable failed, error=%d, clk=%d!\n",
				     ret, clk);
			return -EINVAL;
		}
	}

	return 0;
}

static void dpe_clk_disable(struct dss_hw_ctx *ctx, int clk)
{
	struct clk *clk_tmp = NULL;

	clk_tmp = ctx->dpe_clk[clk];
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}
}

int dpe_common_clk_enable(struct dss_hw_ctx *ctx)
{
	int ret;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL point!\n");
		return -EINVAL;
	}

	ret = dpe_clk_enable(ctx, DPE_MMBUF_CLK);
	if (ret) {
		HISI_DRM_ERR("DPE_MMBUF_CLK enable failed, ret=%d\n", ret);
		return -EINVAL;
	}

	ret = dpe_clk_enable(ctx, DPE_AXI_CLK);
	if (ret) {
		HISI_DRM_ERR("DPE_AXI_CLK enable failed, ret=%d\n", ret);
		return -EINVAL;
	}

	ret = dpe_clk_enable(ctx, DPE_PCLK_CLK);
	if (ret) {
		HISI_DRM_ERR("DPE_PCLK_CLK enable failed, ret=%d\n", ret);
		return -EINVAL;
	}

	return 0;
}

int dpe_inner_clk_enable(struct hisi_drm_crtc *hisi_crtc)
{
	int ret;
	struct dss_hw_ctx *ctx = hisi_crtc->ctx;

	if (!ctx) {
		HISI_DRM_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	ret = dpe_clk_enable(ctx, DPE_PRI_CLK);
	if (ret) {
		HISI_DRM_ERR("DPE_MMBUF_CLK enable failed, ret=%d\n", ret);
		return -EINVAL;
	}

	if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		if (ctx->dpe_clk[DPE_PXL1_CLK]) {
			ret = dpe_clk_enable(ctx, DPE_PXL1_CLK);
			if (ret) {
				HISI_DRM_ERR("crtc-%d, pxl1 enable err=%d\n",
					     hisi_crtc->crtc_id, ret);
				return -EINVAL;
			}
		} else {
			HISI_DRM_ERR("plx1 clk is null");
		}
	}

	return 0;
}

int dpe_common_clk_disable(struct dss_hw_ctx *ctx)
{
	if (!ctx) {
		HISI_DRM_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	dpe_clk_disable(ctx, DPE_PCLK_CLK);
	dpe_clk_disable(ctx, DPE_AXI_CLK);
	dpe_clk_disable(ctx, DPE_MMBUF_CLK);

	return 0;
}

int dpe_inner_clk_disable(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL point!\n");
		return -EINVAL;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is null!\n");
		return -EINVAL;
	}

	if (is_dp(hisi_crtc))
		dpe_clk_disable(ctx, DPE_PXL1_CLK);

	dpe_clk_disable(ctx, DPE_PRI_CLK);

	return 0;
}

static int dpe_clk_get(struct device *dev,
		       struct dss_hw_ctx *ctx)
{
	int ret = 0;
	int i;

	for (i = 0; i < DPE_CLK_NUM; i++) {
		ctx->dpe_clk[i] = devm_clk_get(dev, ctx->dpe_clk_name[i]);
		if (IS_ERR(ctx->dpe_clk)) {
			ret = PTR_ERR(ctx->dpe_clk);
			HISI_DRM_ERR("dpe_clk:%d error, ret = %d", i, ret);
			return ret;
		}
	}

	return ret;
}

irqreturn_t dss_mmbuf_asc0_isr(int irq, void *ptr)
{
	return IRQ_HANDLED;
}

int dpe_pdp_regulator_enable(struct hisi_drm_crtc *hisi_crtc)
{
	int ret;
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is null\n");
		return -EINVAL;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is null\n");
		return -EINVAL;
	}

	ret = regulator_enable(ctx->dpe_regulator[DPE_REGULATOR_PDP].consumer);
	if (ret != 0) {
		HISI_DRM_ERR("dpe regulator enable failed, error=%d\n", ret);
		return ret;
	}

	ctx->dss_regulator_refcount++;
	HISI_DRM_DEBUG("dss regulator ref = %d\n", ctx->dss_regulator_refcount);
	return ret;
}

static int dpe_set_common_clk_rate_on_pll0(struct hisi_drm_crtc *hisi_crtc)
{
	int ret;
	u64 clk_rate;
	struct dss_hw_ctx *ctx = NULL;
#if defined(CONFIG_HISI_PERIDVFS)
	struct peri_volt_poll *pvp = NULL;
#endif

	HISI_DRM_INFO("+");

	ctx = hisi_crtc->ctx;
	if(!ctx) {
		HISI_DRM_ERR("ctx is NULL\n");
		return -EINVAL;
	}

	clk_rate = DSS_MMBUF_CLK_RATE_OFF;
	ret = clk_set_rate(ctx->dpe_clk[DPE_MMBUF_CLK], clk_rate);
	if (ret < 0) {
		HISI_DRM_ERR("set dss mmbuf clk to (%llu) fail, err=%d!\n",
			     clk_rate, ret);
		return -EINVAL;
	}
	HISI_DRM_INFO("set dss mmbuf clk:[%llu]->[%llu].\n", clk_rate,
		      (uint64_t)clk_get_rate(ctx->dpe_clk[DPE_MMBUF_CLK]));

	clk_rate = DSS_CORE_CLK_RATE_OFF;
	ret = clk_set_rate(ctx->dpe_clk[DPE_PRI_CLK], clk_rate);
	if (ret < 0) {
		HISI_DRM_ERR("set dss pri clk to (%llu) fail, err=%d!\n",
			     clk_rate, ret);
		return -EINVAL;
	}
	HISI_DRM_INFO("set pri clk:[%llu]->[%llu].\n", clk_rate,
		      (uint64_t)clk_get_rate(ctx->dpe_clk[DPE_PRI_CLK]));

#if defined(CONFIG_HISI_PERIDVFS)
	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	if (!pvp) {
		HISI_DRM_ERR("get pvp failed!\n");
		return -EINVAL;
	}

	ret = peri_set_volt(pvp, PERI_VOLTAGE_LEVEL0_060V);
	if (ret) {
		HISI_DRM_ERR("set voltage_value=0 failed!\n");
		return -EINVAL;
	}
#endif

	return ret;
}

int dpe_pdp_regulator_disable(struct hisi_drm_crtc *hisi_crtc)
{
	int ret;
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("NULL ptr\n");
		return -EINVAL;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is null\n");
		return -EINVAL;
	}

	ctx->dss_regulator_refcount--;
	HISI_DRM_INFO("dss regulator ref = %d\n", ctx->dss_regulator_refcount);

	dpe_set_pixel_clk_rate_on_pll0(hisi_crtc);
	if (ctx->dss_regulator_refcount == 0)
		dpe_set_common_clk_rate_on_pll0(hisi_crtc);

	if (hisi_crtc->crtc_id == PRIMARY_IDX)
		hisi_crtc->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;

	ret = regulator_disable(ctx->dpe_regulator[DPE_REGULATOR_PDP].consumer);
	if (ret != 0) {
		HISI_DRM_ERR("dpe regulator_disable failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

int dpe_regulator_clk_irq_setup(struct device *dev,
				struct hisi_drm_crtc *hisi_crtc)
{
	int ret;
	struct dss_hw_ctx *ctx = NULL;

	if (!dev) {
		HISI_DRM_ERR("pdev is NULL");
		return -EINVAL;
	}

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return -EINVAL;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return -EINVAL;
	}

	if (dpe_clk_get(dev, ctx)) {
		HISI_DRM_ERR("dpe_clk_get fail\n");
		return -EINVAL;
	}

	if (ctx->mmbuf_asc0_irq) {
		ret = request_irq(ctx->mmbuf_asc0_irq, dss_mmbuf_asc0_isr, 0,
				  "irq_mmbuf_asc0", (void *)ctx);
		if (ret != 0) {
			HISI_DRM_ERR("request irq fail, irq_no=%d error=%d!\n",
				     ctx->mmbuf_asc0_irq, ret);
			return ret;
		}
		disable_irq(ctx->mmbuf_asc0_irq);
	}

	dpe_regulator_enable(ctx, DPE_REGULATOR_MEDIA);

	ret = dpe_set_clk_rate(hisi_crtc);
	if (ret) {
		HISI_DRM_ERR("dpe_set_clk_rate, error=%d!\n", ret);
		return -EINVAL;
	}

	ret = dpe_common_clk_enable(ctx);
	if (ret) {
		HISI_DRM_ERR("dpe_common_clk_enable, error=%d!\n", ret);
		return -EINVAL;
	}

	ret = dpe_inner_clk_enable(hisi_crtc);
	if (ret) {
		HISI_DRM_ERR("dpe_inner_clk_enable, error=%d!\n", ret);
		return -EINVAL;
	}

	if (hisi_set_mmbuf_clk_rate(ctx) < 0) {
		HISI_DRM_ERR("reset_mmbuf_clk_rate failed !\n");
		return -EINVAL;
	}

	dpe_pdp_regulator_enable(hisi_crtc);

	return 0;
}

static int
hisi_vactive0_start_config_br1(struct hisi_drm_crtc *hisi_crtc)
{
	int times;
	int ret = 0;

	if (hisi_crtc->vactive0_start_flag == 0) {
		times = 0;
		while (1) {
			ret = wait_event_interruptible_timeout(
				hisi_crtc->vactive0_start_wq,
				hisi_crtc->vactive0_start_flag,
				msecs_to_jiffies(300));

			if ((ret == -ERESTARTSYS) && (times++ < 50))
				mdelay(10);
			else
				break;
		}

		if (ret <= 0) {
			HISI_DRM_ERR("crtc-%d: wait for vactive0 failed\n",
				     hisi_crtc->crtc_id);
			ret = -ETIMEDOUT;
		} else {
			ldi_data_gate(hisi_crtc, true);
			ret = 0;
		}
	}

	hisi_crtc->vactive0_start_flag = 0;
	return ret;
}

int hisi_vactive0_start_config(struct hisi_drm_crtc *hisi_crtc)
{
	u32 prev_flag;
	int ret;
	int times = 0;
	u32 timeout_interval = DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return -EINVAL;
	}

	if (is_mipi_cmd_mode(hisi_crtc)) {
		ret = hisi_vactive0_start_config_br1(hisi_crtc);
		return ret;
	}

	prev_flag = hisi_crtc->vactive0_start_flag;
	while (1) {
		ret = wait_event_interruptible_timeout(
			hisi_crtc->vactive0_start_wq,
			((prev_flag != hisi_crtc->vactive0_start_flag) ||
			(hisi_crtc->vactive0_start_flag == 1)),
			msecs_to_jiffies(timeout_interval));

		if ((ret == -ERESTARTSYS) && (times++ < 50))
			mdelay(10);
		else
			break;
	}

	if (ret <= 0)
		HISI_DRM_ERR("crtc-%d, vactive0 timeout! ret=%d, flag=%d\n",
			     hisi_crtc->crtc_id, ret,
			hisi_crtc->vactive0_start_flag);
	else
		ret = 0;

	return ret;
}

static void get_vote_cmd_clk_rate(struct dss_vote_cmd *vote_cmd, struct drm_display_mode *mode, int frame_rate)
{
	u64 dss_clk_rate;

	dss_clk_rate = vote_cmd->dss_pri_clk_rate;
	if (dss_clk_rate == 0) {
		if ((mode->hdisplay * mode->vdisplay) >= (RES_4K_PHONE)) {
			vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L4;
		} else if ((mode->hdisplay * mode->vdisplay) >= (RES_1440P)) {
			if (frame_rate >= 110)
				vote_cmd->dss_pri_clk_rate =
					DSS_CORE_CLK_RATE_L4;
			else
				vote_cmd->dss_pri_clk_rate =
					DSS_CORE_CLK_RATE_L1;
		} else if ((mode->hdisplay * mode->vdisplay) >= (RES_1080P)) {
			vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L1;
		} else {
			vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L1;
		}
		vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
		vote_cmd->dss_mmbuf_rate = DSS_MMBUF_CLK_RATE_L1;
	}

}

static struct dss_vote_cmd *get_dss_vote_cmd(struct hisi_drm_crtc *hisi_crtc)
{
	int frame_rate;
	struct dss_vote_cmd *vote_cmd = NULL;
	struct drm_display_mode *mode = NULL;

	vote_cmd = &hisi_crtc->dss_vote_cmd;
	if (!hisi_crtc->base.state) {
		vote_cmd->dss_pri_clk_rate = DSS_CORE_CLK_RATE_L1;
		vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
		HISI_DRM_INFO("crtc state is null\n");
		return vote_cmd;
	}

	mode = &hisi_crtc->base.state->adjusted_mode;
	frame_rate = mode->vrefresh;
	HISI_DRM_DEBUG("frame_rate = %d, xres=%d, yres=%d",
		       frame_rate, mode->hdisplay, mode->vdisplay);

	get_vote_cmd_clk_rate(vote_cmd, mode, frame_rate);

	return vote_cmd;
}

static int dpe_set_pxl_clk_rate(struct hisi_drm_crtc *hisi_crtc)
{
	struct drm_display_mode *mode = NULL;
	int ret = 0;

	if (!is_dp(hisi_crtc))
		return ret;

	if (!hisi_crtc->base.state) {
		HISI_DRM_ERR("state is NULL");
		return -EINVAL;
	}

	mode = &hisi_crtc->base.state->mode;
	ret = dp_pxl_ppll7_init(hisi_crtc, mode->clock * 1000);
	if (ret < 0) {
		HISI_DRM_ERR("crtc-%d clk_set_rate(%llu) fail, err=%d\n",
			     hisi_crtc->crtc_id, mode->clock * 1000, ret);
			return -EINVAL;
	}

	HISI_DRM_INFO("crtc-%d dss_pxl1_clk:[%llu].\n",
		      hisi_crtc->crtc_id,
		(uint64_t)clk_get_rate(hisi_crtc->ctx->dpe_clk[DPE_PXL1_CLK]));

	return ret;
}

static u32 pri_clk_to_volt(u32 pri_clk_rate)
{
	u32 volt;

	switch (pri_clk_rate) {
	case DSS_CORE_CLK_RATE_L4:
		volt = PERI_VOLTAGE_LEVEL3;
		break;
	case DSS_CORE_CLK_RATE_L3:
		volt = PERI_VOLTAGE_LEVEL2;
		break;
	case DSS_CORE_CLK_RATE_L2:
		volt = PERI_VOLTAGE_LEVEL1;
		break;
	case DSS_CORE_CLK_RATE_L1:
		volt = PERI_VOLTAGE_LEVEL0;
		break;
	default:
		HISI_DRM_ERR("wrong pri_clk_rate %d\n", pri_clk_rate);
		volt = PERI_VOLTAGE_LEVEL3;
		break;
	}
	return volt;
}

static int get_pri_clk_rate(struct hisi_drm_crtc *hisi_crtc, struct dss_hw_ctx *ctx,
	struct dss_vote_cmd *pdss_vote_cmd, u64 *dss_pri_clk_rate)
{
	u32 current_voltage;
	struct dss_vote_cmd target_dss_vote_cmd;
	int ret;

	*dss_pri_clk_rate = pdss_vote_cmd->dss_pri_clk_rate;
	ret = get_max_votelevel(hisi_crtc, &current_voltage);
	if (ret) {
		HISI_DRM_ERR("set max votage_value=%d failed,", ret);
		return -EINVAL;
	}
	HISI_DRM_INFO("get_max_votelevel: %d\n", current_voltage);

	ret = get_dss_clock_value(current_voltage, &target_dss_vote_cmd);
	if (ret) {
		HISI_DRM_ERR("get max dss clock=%d failed,", ret);
		return -EINVAL;
	}

	if (*dss_pri_clk_rate < target_dss_vote_cmd.dss_pri_clk_rate)
		*dss_pri_clk_rate = target_dss_vote_cmd.dss_pri_clk_rate;

	ret = clk_set_rate(ctx->dpe_clk[DPE_PRI_CLK], *dss_pri_clk_rate);
	if (ret < 0) {
		HISI_DRM_ERR("set pri clk (%llu) failed, error=%d!\n",
			     *dss_pri_clk_rate, ret);
		return -EINVAL;
	}

	HISI_DRM_INFO("CRTC-%d pri clk:[%llu]->[%llu]\n",
		      hisi_crtc->crtc_id,
		*dss_pri_clk_rate,
		(uint64_t)clk_get_rate(ctx->dpe_clk[DPE_PRI_CLK]));
	return 0;
}

int dpe_set_clk_rate(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	u64 dss_pri_clk_rate;

	int ret;
#if defined(CONFIG_HISI_PERIDVFS)
	struct peri_volt_poll *pvp = NULL;
	u32 set_volt;
#endif
	if (!hisi_crtc) {
		HISI_DRM_INFO("hisi_crtc is null\n");
		return -EINVAL;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_INFO("ctx is null\n");
		return -EINVAL;
	}

	ret = dpe_set_pxl_clk_rate(hisi_crtc);
	if (ret < 0) {
		HISI_DRM_ERR("crtc-%d set pxl clk rate failed, error=%d!\n",
			     hisi_crtc->crtc_id, ret);
		return -EINVAL;
	}

	pdss_vote_cmd = get_dss_vote_cmd(hisi_crtc);
	if (!pdss_vote_cmd) {
		HISI_DRM_ERR("get_dss_vote_cmd error!\n");
		return -EINVAL;
	}

	ret = get_pri_clk_rate(hisi_crtc, ctx, pdss_vote_cmd, &dss_pri_clk_rate);
	if (ret)
		return ret;

#if defined(CONFIG_HISI_PERIDVFS)
	set_volt = pri_clk_to_volt(dss_pri_clk_rate);
	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	if (!pvp) {
		HISI_DRM_ERR("get pvp failed!\n");
		return -EINVAL;
	}

	ret = peri_set_volt(pvp, set_volt);
	if (ret) {
		HISI_DRM_ERR("set volt to %d failed, ret=%d,", set_volt, ret);
		return -EINVAL;
	}
	HISI_DRM_INFO("crtc-%d, set volt: %d, get volt:%d!\n",
		      hisi_crtc->crtc_id, set_volt, peri_get_volt(pvp));
#endif

	return ret;
}

int hisi_set_mmbuf_clk_rate(struct dss_hw_ctx *ctx)
{
	int ret;
	u64 dss_mmbuf_rate;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return -EINVAL;
	}

	dss_mmbuf_rate = 208000000;
	ret = clk_set_rate(ctx->dpe_clk[DPE_MMBUF_CLK], dss_mmbuf_rate);
	if (ret < 0) {
		HISI_DRM_ERR("dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n",
			     dss_mmbuf_rate, ret);
		return -EINVAL;
	}

	HISI_DRM_DEBUG("mmbuf clk set[%llu],get[%llu].\n", dss_mmbuf_rate,
		       (uint64_t)clk_get_rate(ctx->dpe_clk[DPE_MMBUF_CLK]));

	return 0;
}

static void dp_interrupt_clear(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *dss_base = NULL;
	u32 clear;

	ctx = hisi_crtc->ctx;
	dss_base = ctx->dss_base;
	clear = ~0;
	outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INTS, clear);
	outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INTS, clear);
}

static void dp_interrupt_unmask(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *dss_base = NULL;
	u32 unmask;

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	dss_base = ctx->dss_base;

	unmask = ~0;
	unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START |
		BIT_VACTIVE0_END | BIT_FRM_END);
	outp32(dss_base +  DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK, unmask);
	outp32(dss_base +  DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INT_MSK, unmask);
}

static void dp_interrupt_mask(struct hisi_drm_crtc *hisi_crtc)
{
	char __iomem *dss_base = NULL;
	u32 mask;
	struct dss_hw_ctx *ctx = hisi_crtc->ctx;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	dss_base = ctx->dss_base;
	mask = ~0;
	outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK, mask);
	outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INT_MSK, mask);
}

void dpe_interrupt_clear(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *dss_base = NULL;
	u32 clear;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	dss_base = ctx->dss_base;
	if (!dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	if (hisi_crtc->crtc_id == PRIMARY_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_PDP_INTS, clear);

		outp32(dss_base + DSS_DPP_OFFSET + DPP_INTS, clear);

		outp32(dss_base + DSS_DBG_OFFSET + DBG_MCTL_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH0_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH1_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH2_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH3_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_DSS_GLB_INTS, clear);
	} else if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_SDP_INTS, clear);
		if (is_dp(hisi_crtc))
			dp_interrupt_clear(hisi_crtc);
	}
}

void dpe_interrupt_unmask(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	char __iomem *dss_base = NULL;
	char __iomem *dpp_base = NULL;
	u32 unmask;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	dss_base = ctx->dss_base;
	if (!dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	if (hisi_crtc->crtc_id == PRIMARY_IDX) {
		unmask = ~0;
		unmask &= ~(BIT_DPP_INTS | BIT_ITF0_INTS);
		dpp_base = dss_base + DSS_DISP_CH0_OFFSET;
		outp32(dss_base + GLB_CPU_PDP_INT_MSK, unmask);

		unmask = ~0;
		outp32(dpp_base + DPP_INT_MSK, unmask);
	} else if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		unmask = ~0;
		unmask &= ~(BIT_DP_ITF2_INTS | BIT_DP_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_DP_INT_MSK, unmask);
		if (is_dp(hisi_crtc))
			dp_interrupt_unmask(hisi_crtc);
	}
}

void dpe_interrupt_mask(struct hisi_drm_crtc *hisi_crtc)
{
	char __iomem *dss_base = NULL;
	char __iomem *dpp_base = NULL;
	u32 mask;
	struct dss_hw_ctx *ctx = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	dss_base = ctx->dss_base;
	if (!dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	dpp_base = dss_base + DSS_DISP_CH0_OFFSET;

	if (hisi_crtc->crtc_id == PRIMARY_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_PDP_INT_MSK, mask);
		outp32(dpp_base + DPP_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_DSS_GLB_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_MCTL_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH0_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH1_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH2_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH3_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INT_MSK, mask);
	} else if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_SDP_INT_MSK, mask);
		if (is_dp(hisi_crtc))
			dp_interrupt_mask(hisi_crtc);
	}
}

void dss_inner_clk_sdp_enable(struct hisi_drm_crtc *hisi_crtc)
{
	char __iomem *dss_base = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisifd is NULL");
		return;
	}

	if (!hisi_crtc->ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	dss_base = hisi_crtc->ctx->dss_base;
	if (!dss_base) {
		HISI_DRM_ERR("dss_base is NULL");
		return;
	}

#ifdef CONFIG_DSS_LP_USED
	if (is_dp(hisi_crtc)) {
		outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_MODULE_CLK_SEL, 0xE);
		outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_MODULE_CLK_SEL, 0xE);
		outp32(dss_base + DSS_PIPE_SW_DP_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
		outp32(dss_base + DSS_PIPE_SW3_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
	}
	outp32(dss_base + DSS_DISP_CH2_OFFSET + DISP_CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_OVL1_OFFSET + OV8_CLK_SEL, 0x00000000);

#else
	if (is_dp(hisi_crtc)) {
		outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_MEM_CTRL, 0x00000008);
		outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_MEM_CTRL, 0x00000008);
	}
	outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DISP_CH2_OFFSET + DISP_CH_DITHER_MEM_CTRL, 0x00000008);
#endif
}

void dss_inner_clk_sdp_disable(struct hisi_drm_crtc *hisi_crtc)
{
	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL point!");
		return;
	}
}

static void crtc_enable_ldi(struct hisi_drm_crtc *hisi_crtc)
{
	char __iomem *ldi_base = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return;
	}

	if (!hisi_crtc->ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	if (!hisi_crtc->ctx->dss_base) {
		HISI_DRM_ERR("dss_base is NULL");
		return;
	}

	if (is_dp(hisi_crtc)) {
		ldi_base = hisi_crtc->ctx->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
		ldi_base = hisi_crtc->ctx->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
	} else {
		ldi_base = hisi_crtc->ctx->dss_base + DSS_MIPI_DSI0_OFFSET;
		set_reg(ldi_base + MIPI_LDI_CTRL, 0x1, 1, 0);
	}
}

void crtc_single_frame_update(struct hisi_drm_crtc *hisi_crtc)
{
	char __iomem *ldi_base = NULL;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return;
	}

	if (!hisi_crtc->ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	if (!hisi_crtc->ctx->dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	if (is_dp(hisi_crtc)) {
		ldi_base = hisi_crtc->ctx->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_FRM_MSK_UP, 0x1, 1, 0);

		ldi_base = hisi_crtc->ctx->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_FRM_MSK_UP, 0x1, 1, 0);
	}

	crtc_enable_ldi(hisi_crtc);
}

void enable_dp_ldi(char __iomem *dss_base)
{
	char __iomem *ldi_base = NULL;

	if (!dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	ldi_base = dss_base + DSS_LDI_DP_OFFSET;
	set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
	ldi_base = dss_base + DSS_LDI_DP1_OFFSET;
	set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
}

void disable_dp_ldi(char __iomem *dss_base)
{
	char __iomem *ldi_base = NULL;

	if (!dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	ldi_base = dss_base + DSS_LDI_DP_OFFSET;
	set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);

	ldi_base = dss_base + DSS_LDI_DP1_OFFSET;
	set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);
}

void crtc_disable_ldi(struct hisi_drm_crtc *hisi_crtc)
{
	char __iomem *ldi_base = NULL;

#ifdef CONFIG_HISI_DSS_V600
	return;
#endif
	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return;
	}

	if (!hisi_crtc->ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return;
	}

	if (!hisi_crtc->ctx->dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	if (is_dp(hisi_crtc)) {
		ldi_base = hisi_crtc->ctx->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);
		ldi_base = hisi_crtc->ctx->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);
	} else {
		ldi_base = hisi_crtc->ctx->dss_base + DSS_MIPI_DSI0_OFFSET;
		set_reg(ldi_base + MIPI_LDI_CTRL, 0x0, 1, 0);
	}
}

/*lint +e529 +e530 +e559 +e578 +e666*/
