/* Copyright (c) 2018-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
/*lint -e529 -e559 -e578 -e647 -e671 -e838*/

#include "hisi_drm_dsi.h"

#include <linux/clk.h>
#include <linux/component.h>
#include <linux/of_graph.h>
#include <linux/iopoll.h>
#include <video/mipi_display.h>
#include <linux/gpio/consumer.h>
#include <linux/of_address.h>

#include <drm/drmP.h>
#include <drm/drm_of.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_encoder_slave.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_panel.h>

#include <securec.h>

#include "hisi_dsi_reg.h"
#include "hisi_dpe.h"
#include "hisi_drm_dpe_utils.h"
#include "hisi_mipi_dsi.h"
#include "panel/hisi_drm_panel.h"

enum {
	BRIDGE_OUT = 1,
	PANEL_OUT = 2,
};

static const struct dsi_phy_range dphy_range_info[] = {
	{   46875,    62500,   1,    7 },
	{   62500,    93750,   0,    7 },
	{   93750,   125000,   1,    6 },
	{  125000,   187500,   0,    6 },
	{  187500,   250000,   1,    5 },
	{  250000,   375000,   0,    5 },
	{  375000,   500000,   1,    4 },
	{  500000,   750000,   0,    4 },
	{  750000,  1000000,   1,    0 },
	{ 1000000,  1500000,   0,    0 }
};

#define IS_DUAL_MIPI (0)

bool is_dual_mipi(struct hisi_dsi *dsi)
{
	UNUSED(dsi);
	return IS_DUAL_MIPI;
}

int mipi_dsi_cmds_tx(struct mipi_dsi_device *dsi,
	struct dsi_cmd_desc *cmds, int cnt)
{
	struct dsi_cmd_desc *cm = NULL;
	int i;

	if (cmds == NULL) {
		HISI_DRM_ERR("cmds is NULL");
		return -EINVAL;
	}
	if (dsi == NULL) {
		HISI_DRM_ERR("dsi is NULL");
		return -EINVAL;
	}

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		mipi_dsi_dcs_write(dsi, cm->payload[0],
			cm->payload + 1, cm->dlen - 1);

		if (cm->wait) {
			if (cm->waittype == WAIT_TYPE_US) {
				udelay(cm->wait);
			} else if (cm->waittype == WAIT_TYPE_MS) {
				if (cm->wait <= 10)
					mdelay(cm->wait);
				else
					msleep(cm->wait);
			} else {
				msleep(cm->wait * 1000);
			}
		}
		cm++;
	}

	return cnt;
}

static void mipi_check_itf_status(struct hisi_dsi *dsi)
{
	uint32_t tmp;
	int delay_count = 0;
	bool is_timeout = true;
	char __iomem *mctl_sys_base = NULL;
	struct dsi_hw_ctx *ctx = NULL;

	if (dsi == NULL) {
		HISI_DRM_ERR("dsi is NULL");
		return;
	}

	ctx = dsi->ctx;
	if (ctx == NULL) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	mctl_sys_base =  ctx->dss_base + DSS_MCTRL_SYS_OFFSET;
	while (1) {
		tmp = inp32(mctl_sys_base + MCTL_MOD17_STATUS);
		if (((tmp & 0x10) == 0x10) || delay_count > 100) {
			is_timeout = (delay_count > 100) ? true : false;
			break;
		}
		mdelay(1);
		++delay_count;
	}

	if (is_timeout)
		HISI_DRM_INFO("mctl_itf not in idle status,ints=0x%x !\n", tmp);
}

static void dsi_interrupt_mask(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = NULL;
	u32 mask;

	ctx = dsi->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	mask = ~0;
	outp32(ctx->mipi_dsi0_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);
	if (is_dual_mipi(dsi))
		outp32(ctx->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);
}

static void dsi_interrupt_unmask(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = NULL;
	u32 unmask;

	ctx = dsi->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	unmask = ~0;
	if (is_mipi_cmd_panel(dsi->panel_info))
		unmask &= ~(BIT_LCD_TE0_PIN | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
	else
		unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);

	outp32(ctx->mipi_dsi0_base + MIPI_LDI_CPU_ITF_INT_MSK, unmask);
	if (is_dual_mipi(dsi))
		outp32(ctx->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, unmask);
}

static void dsi_interrupt_clear(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = NULL;
	u32 clear;

	ctx = dsi->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	clear = ~0;
	outp32(ctx->mipi_dsi0_base + MIPI_LDI_CPU_ITF_INTS, clear);
	if (is_dual_mipi(dsi))
		outp32(ctx->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS, clear);
}

static void dsi_irq_enable(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = dsi->ctx;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	if (ctx->dsi0_irq) {
		HISI_DRM_INFO("ctx->dsi0_irq=%d\n", ctx->dsi0_irq);
		enable_irq(ctx->dsi0_irq);
	}
}

static void dsi_irq_disable(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = dsi->ctx;

	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}

	if (ctx->dsi0_irq) {
		HISI_DRM_INFO("ctx->dsi0_irq=%d\n", ctx->dsi0_irq);
		disable_irq(ctx->dsi0_irq);
	}
}

static void dsi_encoder_disable(struct drm_encoder *encoder)
{
	struct hisi_dsi *dsi = NULL;
	struct dsi_hw_ctx *ctx = NULL;
	void __iomem *mipi_dsi0_base = NULL;

	HISI_DRM_INFO("+");
	if (!encoder) {
		HISI_DRM_ERR("encoder is NULL");
		return;
	}

	dsi = encoder_to_dsi(encoder);
	ctx = dsi->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL");
		return;
	}
	mipi_dsi0_base = ctx->mipi_dsi0_base;

	if (!dsi->enable)
		return;

	dsi_interrupt_mask(dsi);
	dsi_irq_disable(dsi);

	/* turn off panel's backlight */
	if (dsi->panel && drm_panel_disable(dsi->panel))
		HISI_DRM_ERR("failed to disable panel\n");

	/* turn off panel */
	if (dsi->panel && drm_panel_unprepare(dsi->panel))
		HISI_DRM_ERR("failed to unprepare panel\n");

	disable_dsi_ldi(dsi);
	mipi_check_itf_status(dsi);

	mipi_dsi_off_sub(mipi_dsi0_base);
	if (is_dual_mipi(dsi))
		mipi_dsi_off_sub(dsi->ctx->mipi_dsi1_base);

	mipi_dsi_clk_disable(dsi);

	if (is_dual_mipi(dsi))
		outp32(ctx->peri_crg_base + PERRSTEN3, 0x30000000);
	else
		outp32(ctx->peri_crg_base + PERRSTEN3, 0x10000000);

	/* turn off panel */
	if (dsi->panel && drm_panel_unprepare(dsi->panel))
		HISI_DRM_ERR("failed to unprepare panel\n");

	dsi->enable = false;

	HISI_DRM_INFO("-");
}

static void mipi_dsi_set_fastboot(struct hisi_dsi *dsi)
{
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	HISI_DRM_INFO("+");
	if (is_dual_mipi(dsi))
		init_dual_mipi_ctrl(dsi);

	if (dsi->panel && drm_panel_prepare(dsi->panel))
		HISI_DRM_ERR("failed to prepare panel\n");

	mipi_dsi_clk_enable(dsi);

	pinfo = dsi->panel_info;
	ret = memset_s(&(pinfo->dsi_phy_ctrl), sizeof(struct mipi_dsi_phy_ctrl), 0, sizeof(struct mipi_dsi_phy_ctrl));
	if (ret != EOK) {
		HISI_DRM_ERR("memset for dsi_phy_ctrl failed!");
		return;
	}
	if (pinfo->mipi.phy_mode == CPHY_MODE)
		get_dsi_cphy_ctrl(dsi, &(pinfo->dsi_phy_ctrl));
	else
		get_dsi_dphy_ctrl(dsi, &(pinfo->dsi_phy_ctrl));

	if (dsi->panel && drm_panel_enable(dsi->panel))
		HISI_DRM_ERR("failed to enable panel\n");
}
#ifdef CONFIG_HISI_DSS_V600
static void mipi_dsi0_clk_enable(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = dsi->ctx;

	set_reg(ctx->dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL, 0x1, 1, 16);
	set_reg(ctx->dss_base + GLB_MODULE_CLK_SEL, 0x1, 1, 23);
	set_reg(ctx->dss_base + GLB_MODULE_CLK_EN, 0x1, 1, 23);
	udelay(10);  /* 10us */
}
#endif

static void mipi_dsi_on(struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = dsi->ctx;

	HISI_DRM_INFO("+");

	if (is_dual_mipi(dsi))
		init_dual_mipi_ctrl(dsi);

	if (dsi->panel && drm_panel_prepare(dsi->panel))
		HISI_DRM_ERR("failed to prepare panel\n");

	if (is_dual_mipi(dsi)) {
		outp32(ctx->peri_crg_base + PERRSTDIS3, 0x30000000);
	} else {
		outp32(ctx->peri_crg_base + PERRSTDIS3, 0x10000000);
		outp32(ctx->peri_crg_base + PERRSTDIS3, 0x20000000);
	}

	mipi_dsi_clk_enable(dsi);
#ifdef CONFIG_HISI_DSS_V600
	mipi_dsi0_clk_enable(dsi);
#endif

	mipi_dsi_on_sub1(dsi, ctx->mipi_dsi0_base);
	if (is_dual_mipi(dsi))
		mipi_dsi_on_sub1(dsi, ctx->mipi_dsi1_base);

	if (dsi->panel && drm_panel_enable(dsi->panel))
		HISI_DRM_ERR("failed to enable panel");

	mipi_dsi_on_sub2(dsi, ctx->mipi_dsi0_base);
	if (is_dual_mipi(dsi))
		mipi_dsi_on_sub2(dsi, ctx->mipi_dsi1_base);

	if (dsi->panel && drm_panel_enable(dsi->panel))
		HISI_DRM_ERR("failed to enable panel");
}

static void dsi_encoder_enable(struct drm_encoder *encoder)
{
	struct hisi_dsi *dsi = NULL;

	HISI_DRM_INFO("+");
	if (!encoder) {
		HISI_DRM_ERR("encoder is NULL");
		return;
	}

	dsi = encoder_to_dsi(encoder);
	if (dsi->enable)
		return;

	if (dsi->is_pre_enabled)
		mipi_dsi_set_fastboot(dsi);
	else
		mipi_dsi_on(dsi);

	dsi_interrupt_mask(dsi);
	dsi_interrupt_clear(dsi);
	dsi_irq_enable(dsi);
	dsi_interrupt_unmask(dsi);

	ldi_dsi_data_gate(dsi, dsi->is_pre_enabled);

	dsi->is_pre_enabled = NEED_PRE_ENABLE_NEXT;
	dsi->enable = true;
	HISI_DRM_INFO("-");
}

static void dsi_encoder_mode_set(struct drm_encoder *encoder,
				 struct drm_display_mode *mode,
				 struct drm_display_mode *adj_mode)
{
	struct hisi_dsi *dsi = NULL;

	UNUSED(mode);
	if (!encoder) {
		HISI_DRM_ERR("encoder is NULL");
		return;
	}

	if (!adj_mode) {
		HISI_DRM_ERR("adj_mode is NULL");
		return;
	}

	dsi = encoder_to_dsi(encoder);

	drm_mode_copy(&dsi->cur_mode, adj_mode);

	HISI_DRM_INFO("dsi->cur_mode.clock=%d\n", dsi->cur_mode.clock);
}

#define MODE_SIZE(m) ((m)->hdisplay * (m)->vdisplay)

static int dsi_encoder_atomic_check(struct drm_encoder *encoder,
				    struct drm_crtc_state *crtc_state,
				    struct drm_connector_state *conn_state)
{
	struct drm_display_mode *cur_mode = NULL;

	UNUSED(encoder);

	if (!crtc_state) {
		HISI_DRM_ERR("crtc_state is NULL");
		return -EINVAL;
	}

	if (!conn_state) {
		HISI_DRM_ERR("conn_state is NULL");
		return -EINVAL;
	}

	list_for_each_entry(cur_mode, &conn_state->connector->modes, head) {
		if (cur_mode->type == DRM_MODE_TYPE_PREFERRED)
			break;
	}

	if (cur_mode == NULL) {
		HISI_DRM_INFO("cur_mode is NULL");
		return 0;
	}

	HISI_DRM_INFO("cur_mode->type%d\n", cur_mode->type);
	if ((cur_mode->type == DRM_MODE_TYPE_PREFERRED) &&
		(MODE_SIZE(cur_mode) > MODE_SIZE(&crtc_state->adjusted_mode))) {
		crtc_state->adjusted_mode = *cur_mode;
		HISI_DRM_INFO("adjusted_mode: " DRM_MODE_FMT "\n",
			DRM_MODE_ARG(&crtc_state->adjusted_mode));
	}

	return 0;
}

static const struct drm_encoder_helper_funcs dsi_encoder_helper_funcs = {
	.atomic_check = dsi_encoder_atomic_check,
	.mode_set = dsi_encoder_mode_set,
	.enable = dsi_encoder_enable,
	.disable = dsi_encoder_disable
};

static const struct drm_encoder_funcs dsi_encoder_funcs = {
	.destroy = drm_encoder_cleanup,
};

static int hisi_drm_dsi_encoder_init(struct device *dev,
				struct drm_device *drm_dev,
				struct drm_encoder *encoder)
{
	int ret;
	uint32_t crtc_mask;

	HISI_DRM_INFO("+");

	crtc_mask = drm_of_find_possible_crtcs(drm_dev, dev->of_node);
	if (!crtc_mask) {
		HISI_DRM_ERR("failed to find crtc mask");
		return -EINVAL;
	}

	encoder->possible_crtcs = crtc_mask;
	ret = drm_encoder_init(drm_dev, encoder, &dsi_encoder_funcs,
				DRM_MODE_ENCODER_DSI, NULL);
	if (ret) {
		HISI_DRM_ERR("failed to init dsi encoder");
		return ret;
	}
	encoder->possible_clones = 0; // not support clone with dp

	drm_encoder_helper_add(encoder, &dsi_encoder_helper_funcs);

	return 0;
}

static int dsi_host_attach(struct mipi_dsi_host *host,
			struct mipi_dsi_device *mdsi)
{
	struct hisi_dsi *dsi = NULL;
	struct hisi_drm_panel *panel = NULL;
	struct hisi_panel_info *pinfo = NULL;

	HISI_DRM_INFO("+");
	if (!host) {
		HISI_DRM_ERR("host is null\n");
		return -EINVAL;
	}
	if (!mdsi) {
		HISI_DRM_ERR("mdsi is null\n");
		return -EINVAL;
	}

	dsi = host_to_dsi(host);
	panel = mipi_dsi_get_drvdata(mdsi);
	if (!panel) {
		HISI_DRM_ERR("panel is null\n");
		return -EINVAL;
	}
	pinfo = panel->panel_info;

	if (pinfo == NULL) {
		HISI_DRM_ERR("pinfo is null\n");
		return -EINVAL;
	}

	if (mdsi->lanes < 1 || mdsi->lanes > 4) {
		HISI_DRM_ERR("dsi device params invalid\n");
		return -EINVAL;
	}

	dsi->panel_info = pinfo;
	dsi->mipi = pinfo->mipi;
	dsi->ldi = pinfo->ldi;

	dsi->client.lanes = mdsi->lanes;
	dsi->client.format = mdsi->format;
	dsi->client.mode_flags = mdsi->mode_flags;
	dsi->client.phy_clock = 0;

	return 0;
}

static int dsi_host_detach(struct mipi_dsi_host *host,
			struct mipi_dsi_device *mdsi)
{
	/* do nothing */
	return 0;
}

static int dsi_gen_pkt_hdr_write(void __iomem *base, u32 val)
{
	u32 status;
	int ret;

	ret = readx_poll_timeout(readl, base + CMD_PKT_STATUS, status,
				!(status & GEN_CMD_FULL), 1000,
				CMD_PKT_STATUS_TIMEOUT_US);
	if (ret < 0) {
		HISI_DRM_ERR("failed to get available command FIFO\n");
		return ret;
	}

	writel(val, base + GEN_HDR);

	ret = readx_poll_timeout(readl, base + CMD_PKT_STATUS, status,
				status & (GEN_CMD_EMPTY | GEN_PLD_W_EMPTY),
				1000, CMD_PKT_STATUS_TIMEOUT_US);
	if (ret < 0) {
		HISI_DRM_ERR("failed to write command FIFO\n");
		return ret;
	}

	return 0;
}

static int dsi_dcs_short_write(void __iomem *base,
				const struct mipi_dsi_msg *msg)
{
	const u16 *tx_buf = msg->tx_buf;
	u32 val = GEN_HDATA(*tx_buf) | GEN_HTYPE(msg->type);

	if (msg->tx_len > 2) {
		HISI_DRM_ERR("too long tx buf length %zu for short write\n",
			msg->tx_len);
		return -EINVAL;
	}

	return dsi_gen_pkt_hdr_write(base, val);
}

static int dsi_dcs_long_write(void __iomem *base,
				const struct mipi_dsi_msg *msg)
{
	const u32 *tx_buf = msg->tx_buf;
	int len = msg->tx_len;
	int pld_data_bytes = sizeof(*tx_buf);
	int ret;
	u32 val = GEN_HDATA(msg->tx_len) | GEN_HTYPE(msg->type);
	u32 remainder = 0;
	u32 status;

	if (msg->tx_len < 3) {
		HISI_DRM_ERR("wrong tx buf length %zu for long write\n",
			  msg->tx_len);
		return -EINVAL;
	}

	while (DIV_ROUND_UP(len, pld_data_bytes)) {
		if (len < pld_data_bytes) {
			ret = memcpy_s(&remainder, len, tx_buf, len);
			if (ret != EOK) {
				HISI_DRM_ERR("remainder copy failed!\n");
				return -EINVAL;
			}
			writel(remainder, base + GEN_PLD_DATA);
			len = 0;
		} else {
			writel(*tx_buf, base + GEN_PLD_DATA);
			tx_buf++;
			len -= pld_data_bytes;
		}

		ret = readx_poll_timeout(readl, base + CMD_PKT_STATUS,
					status, !(status & GEN_PLD_W_FULL),
					1000, CMD_PKT_STATUS_TIMEOUT_US);
		if (ret < 0) {
			HISI_DRM_ERR("failed to get available write payload FIFO\n");
			return ret;
		}
	}

	return dsi_gen_pkt_hdr_write(base, val);
}

static ssize_t dsi_host_transfer(struct mipi_dsi_host *host,
				    const struct mipi_dsi_msg *msg)
{
	struct hisi_dsi *dsi = host_to_dsi(host);
	struct dsi_hw_ctx *ctx = dsi->ctx;
	void __iomem *base = ctx->mipi_dsi0_base;
	int ret;

	switch (msg->type) {
	case MIPI_DSI_DCS_SHORT_WRITE:
	case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
	case MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE:
		ret = dsi_dcs_short_write(base, msg);
		break;
	case MIPI_DSI_DCS_LONG_WRITE:
		ret = dsi_dcs_long_write(base, msg);
		break;
	default:
		HISI_DRM_ERR("unsupported message type\n");
		ret = -EINVAL;
	}

	return ret;
}

static const struct mipi_dsi_host_ops dsi_host_ops = {
	.attach = dsi_host_attach,
	.detach = dsi_host_detach,
	.transfer = dsi_host_transfer,
};

static int dsi_host_init(struct device *dev, struct hisi_dsi *dsi)
{
	struct mipi_dsi_host *host = &dsi->host;
	int ret;

	host->dev = dev;
	host->ops = &dsi_host_ops;

	ret = mipi_dsi_host_register(host);

	if (ret) {
		HISI_DRM_ERR("failed to register dsi host\n");
		return ret;
	}

	return 0;
}

static int dsi_bridge_init(struct drm_device *dev, struct hisi_dsi *dsi)
{
	struct drm_encoder *encoder = &dsi->encoder;
	struct drm_bridge *bridge = dsi->bridge;
	int ret;

	/* associate the bridge to dsi encoder */
	bridge->encoder = encoder;

	ret = drm_bridge_attach(encoder, bridge, NULL);
	if (ret) {
		HISI_DRM_ERR("failed to attach external bridge, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

static int dsi_connector_get_modes(struct drm_connector *connector)
{

	struct hisi_dsi *dsi = NULL;

	HISI_DRM_INFO("+");

	if (!connector) {
		HISI_DRM_ERR("connector is null\n");
		return -EINVAL;
	}

	dsi = connector_to_dsi(connector);

	return drm_panel_get_modes(dsi->panel);
}

static enum drm_mode_status
dsi_connector_mode_valid(struct drm_connector *connector,
			struct drm_display_mode *mode)
{
	enum drm_mode_status mode_status = MODE_OK;

	return mode_status;
}

static struct drm_encoder *
dsi_connector_best_encoder(struct drm_connector *connector)
{
	struct hisi_dsi *dsi = NULL;

	dsi = connector_to_dsi(connector);
	return &dsi->encoder;
}

static struct drm_connector_helper_funcs dsi_connector_helper_funcs = {
	.get_modes = dsi_connector_get_modes,
	.mode_valid = dsi_connector_mode_valid,
	.best_encoder = dsi_connector_best_encoder,
};

static enum drm_connector_status
dsi_connector_detect(struct drm_connector *connector, bool force)
{
	struct hisi_dsi *dsi = NULL;
	enum drm_connector_status status;

	dsi = connector_to_dsi(connector);
	status = dsi->panel ? connector_status_connected :
		connector_status_disconnected;

	HISI_DRM_INFO("status=%d\n", status);

	return status;
}

static void dsi_connector_destroy(struct drm_connector *connector)
{
	drm_connector_unregister(connector);
	drm_connector_cleanup(connector);
}

static const struct drm_connector_funcs dsi_atomic_connector_funcs = {
	.dpms = drm_helper_connector_dpms,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.detect = dsi_connector_detect,
	.destroy = dsi_connector_destroy,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

static int dsi_connector_init(struct drm_device *dev, struct hisi_dsi *dsi)
{
	struct drm_encoder *encoder = &dsi->encoder;
	struct drm_connector *connector = &dsi->connector;
	int ret;

	connector->polled = DRM_CONNECTOR_POLL_HPD;
	drm_connector_helper_add(connector,
				 &dsi_connector_helper_funcs);

	ret = drm_connector_init(dev, &dsi->connector,
				 &dsi_atomic_connector_funcs,
				 DRM_MODE_CONNECTOR_DSI);
	if (ret)
		return ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	ret = drm_mode_connector_attach_encoder(connector, encoder);
#else
	ret = drm_connector_attach_encoder(connector, encoder);
#endif
	if (ret)
		return ret;

	if (dsi->panel) {
		ret = drm_panel_attach(dsi->panel, connector);
		if (ret)
			return ret;
	}

	return 0;
}
static int dsi_bind(struct device *dev, struct device *master, void *data)
{
	struct dsi_data *ddata = NULL;
	struct hisi_dsi *dsi = NULL;
	struct drm_device *drm_dev = NULL;
	struct hisi_drm_private *priv = NULL;
	int ret;

	UNUSED(master);

	HISI_DRM_INFO("+");

	if (!dev) {
		HISI_DRM_ERR("dev is NULL");
		return -EINVAL;
	}

	if (data == NULL) {
		HISI_DRM_ERR("data is NULL");
		return -EINVAL;
	}
	ddata = dev_get_drvdata(dev);
	dsi = &ddata->dsi;
	drm_dev = data;

	priv = drm_dev->dev_private;
	if (priv == NULL || priv->crtc[PRIMARY_IDX] == NULL) {
		HISI_DRM_ERR("priv or crtc is null");
		return -EINVAL;
	}
	dsi->hisi_crtc = to_hisi_drm_crtc(priv->crtc[PRIMARY_IDX]);

	ret = hisi_drm_dsi_encoder_init(dev, drm_dev, &dsi->encoder);
	if (ret) {
		HISI_DRM_ERR("hisi_drm_dsi_encoder_init init fail, ret=%d", ret);
		return ret;
	}

	if (dsi->panel) {
		ret = dsi_connector_init(drm_dev, dsi);
		if (ret)
			return ret;
	} else if (dsi->bridge) {
		ret = dsi_bridge_init(drm_dev, dsi);
		if (ret)
			return ret;
	}

	HISI_DRM_INFO("-");
	return 0;
}

static void dsi_unbind(struct device *dev, struct device *master, void *data)
{
	/* do nothing */
}

static const struct component_ops hisi_dsi_ops = {
	.bind = dsi_bind,
	.unbind = dsi_unbind,
};

static int get_base_for_dsi_hw_ctx(struct dsi_hw_ctx *ctx,
	struct device_node *np)
{
	ctx->dss_base = of_iomap(np, 0);

	if (!(ctx->dss_base)) {
		HISI_DRM_ERR("failed to get mipi_dsi0_base resource");
		return -ENXIO;
	}

	ctx->mipi_dsi0_base = ctx->dss_base + DSS_MIPI_DSI0_OFFSET;
	ctx->mipi_dsi1_base = ctx->dss_base + DSS_MIPI_DSI1_OFFSET;

	ctx->peri_crg_base = of_iomap(np, 1);
	if (!(ctx->peri_crg_base)) {
		HISI_DRM_ERR("failed to get peri_crg_base resource");
		return -ENXIO;
	}

	ctx->pctrl_base = of_iomap(np, 3);
	if (!(ctx->pctrl_base)) {
		HISI_DRM_ERR("failed to get pctrl_base resource");
		return -ENXIO;
	}

	ctx->pmctrl_base = of_iomap(np, 6);
	if (!(ctx->pmctrl_base)) {
		HISI_DRM_ERR("failed to get pmctrl_base resource");
		return -ENXIO;
	}
	return 0;
}

static int set_dphy0_ref_clk(struct dsi_hw_ctx *ctx,
	struct platform_device *pdev)
{
	int ret;

	ctx->dss_dphy0_ref_clk = devm_clk_get(&pdev->dev, "clk_txdphy0_ref");
	if (IS_ERR(ctx->dss_dphy0_ref_clk)) {
		HISI_DRM_ERR("failed to get dss_dphy0_ref_clk clock");
		return PTR_ERR(ctx->dss_dphy0_ref_clk);
	}

	ret = clk_set_rate(ctx->dss_dphy0_ref_clk, DEFAULT_MIPI_CLK_RATE);
	if (ret < 0) {
		HISI_DRM_ERR("dss_dphy0_ref_clk clk_set_rate(%lu) failed, error=%d!\n",
			DEFAULT_MIPI_CLK_RATE, ret);
		return -EINVAL;
	}

	HISI_DRM_INFO("dss_dphy0_ref_clk:[%lu]->[%lu]",
		DEFAULT_MIPI_CLK_RATE, clk_get_rate(ctx->dss_dphy0_ref_clk));
	return 0;
}

static int set_dphy0_cfg_clk(struct dsi_hw_ctx *ctx,
	struct platform_device *pdev)
{
	int ret;

	ctx->dss_dphy0_cfg_clk = devm_clk_get(&pdev->dev, "clk_txdphy0_cfg");
	if (IS_ERR(ctx->dss_dphy0_cfg_clk)) {
		HISI_DRM_ERR("failed to get dss_dphy0_cfg_clk clock");
		return PTR_ERR(ctx->dss_dphy0_cfg_clk);
	}

	ret = clk_set_rate(ctx->dss_dphy0_cfg_clk, DEFAULT_MIPI_CLK_RATE);
	if (ret < 0) {
		HISI_DRM_ERR("dss_dphy0_cfg_clk clk_set_rate(%lu) failed, error=%d",
			DEFAULT_MIPI_CLK_RATE, ret);
		return -EINVAL;
	}

	HISI_DRM_INFO("dss_dphy0_cfg_clk:[%lu]->[%lu]",
		DEFAULT_MIPI_CLK_RATE, clk_get_rate(ctx->dss_dphy0_cfg_clk));
	return 0;
}

static int set_dphy1_ref_clk(struct dsi_hw_ctx *ctx,
	struct platform_device *pdev)
{
	int ret;

	ctx->dss_dphy1_ref_clk = devm_clk_get(&pdev->dev, "clk_txdphy1_ref");
	if (IS_ERR(ctx->dss_dphy1_ref_clk)) {
		HISI_DRM_ERR("failed to get clk_txdphy1_ref clock");
		return PTR_ERR(ctx->dss_dphy1_ref_clk);
	}

	ret = clk_set_rate(ctx->dss_dphy1_ref_clk, DEFAULT_MIPI_CLK_RATE);
	if (ret < 0) {
		HISI_DRM_ERR("clk_txdphy1_ref clk_set_rate(%lu) failed, error=%d",
			DEFAULT_MIPI_CLK_RATE, ret);
		return -EINVAL;
	}
	HISI_DRM_INFO("clk_txdphy1_ref:[%lu]->[%lu]",
		DEFAULT_MIPI_CLK_RATE, clk_get_rate(ctx->dss_dphy1_ref_clk));
	return 0;
}

static int set_dphy1_cfg_clk(struct dsi_hw_ctx *ctx,
	struct platform_device *pdev)
{
	int ret;

	ctx->dss_dphy1_cfg_clk = devm_clk_get(&pdev->dev, "clk_txdphy1_cfg");
	if (IS_ERR(ctx->dss_dphy1_cfg_clk)) {
		HISI_DRM_ERR("failed to get clk_txdphy1_cfg clock");
		return PTR_ERR(ctx->dss_dphy1_cfg_clk);
	}
	ret = clk_set_rate(ctx->dss_dphy1_cfg_clk, DEFAULT_MIPI_CLK_RATE);
	if (ret < 0) {
		HISI_DRM_ERR("clk_txdphy1_cfg clk_set_rate(%lu) failed, error=%d",
			DEFAULT_MIPI_CLK_RATE, ret);
		return -EINVAL;
	}
	HISI_DRM_INFO("clk_txdphy1_cfg:[%lu]->[%lu]",
		DEFAULT_MIPI_CLK_RATE, clk_get_rate(ctx->dss_dphy1_cfg_clk));
	return 0;
}

static int dsi_parse_dt(struct platform_device *pdev, struct hisi_dsi *dsi)
{
	struct dsi_hw_ctx *ctx = dsi->ctx;
	int ret;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_DSI_NAME);
	if (!np) {
		HISI_DRM_ERR("NOT FOUND device node %s!",
				    DTS_COMP_DSI_NAME);
		return -ENXIO;
	}

	ret = get_base_for_dsi_hw_ctx(ctx, np);
	if (ret)
		return ret;

	ctx->dsi0_irq = irq_of_parse_and_map(np, 3);
	if (!ctx->dsi0_irq) {
		HISI_DRM_ERR("failed to get dsi0_irq resource");
		return -ENXIO;
	}

	ret = set_dphy0_ref_clk(ctx, pdev);
	if (ret)
		return ret;

	ret = set_dphy0_cfg_clk(ctx, pdev);
	if (ret)
		return ret;

	if (IS_DUAL_MIPI) {
		ret = set_dphy1_ref_clk(ctx, pdev);
		if (ret)
			return ret;

		ret = set_dphy1_cfg_clk(ctx, pdev);
		if (ret)
			return ret;
	}
	dsi->is_pre_enabled = PRE_ENABLED_IN_UEFI; /* Incase uefi has different resolution */

	return 0;
}

static int hisi_find_bridge(struct device_node *np, struct hisi_dsi *dsi)
{
	int endpoint;

	endpoint = 0;
	do {
		dsi->bridge_node = of_graph_get_remote_node(np,
			BRIDGE_OUT, endpoint);
		if (dsi->bridge_node) {
			HISI_DRM_INFO("find dsi->bridge_node, node=%s\n",
				      dsi->bridge_node->name);
			dsi->bridge = of_drm_find_bridge(dsi->bridge_node);
			if (dsi->bridge)
				break;
			endpoint++;
		}
	} while (dsi->bridge_node);

	if (endpoint && !dsi->bridge) {
		HISI_DRM_INFO("bridge is not ready!");
		return -EPROBE_DEFER;
	}

	if (!dsi->bridge)
		HISI_DRM_INFO("can't find dsi->bridge");

	return 0;
}

static int hisi_dsi_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct device *dev = NULL;
	struct dsi_data *data = NULL;
	struct hisi_dsi *dsi = NULL;
	struct dsi_hw_ctx *ctx = NULL;
	int ret;

	HISI_DRM_INFO("+");

	if (!pdev) {
		HISI_DRM_ERR("pdev is null\n");
		return -EINVAL;
	}

	np = pdev->dev.of_node;
	dev = &pdev->dev;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		HISI_DRM_ERR("failed to allocate dsi data");
		return -ENOMEM;
	}
	dsi = &data->dsi;
	ctx = &data->ctx;
	dsi->ctx = ctx;

	ret = hisi_find_bridge(np, dsi);
	if (ret) {
		devm_kfree(dev, data);
		HISI_DRM_ERR("find no bridge");
		return ret;
	}

	dsi_host_init(dev, dsi);
	dsi->panel_node = of_graph_get_remote_node(np, PANEL_OUT, 0);
	if (dsi->panel_node) {
		HISI_DRM_INFO("find dsi->panel_node, dsi->panel_node=0x%lx\n",
			dsi->panel_node);
		dsi->panel = of_drm_find_panel(dsi->panel_node);
	} else {
		HISI_DRM_INFO("can't find dsi->panel_node");
	}

	ret = dsi_parse_dt(pdev, dsi);
	if (ret) {
		devm_kfree(dev, data);
		HISI_DRM_ERR("dsi_parse_dt failed, ret=%d", ret);
		return ret;
	}

	ret = request_irq(ctx->dsi0_irq, dss_dsi0_isr, 0, "dsi0_irq", dsi);
	if (ret) {
		devm_kfree(dev, data);
		HISI_DRM_ERR("fail to request dsi0_irq, ret=%d", ret);
		return ret;
	}
	disable_irq(ctx->dsi0_irq);

	platform_set_drvdata(pdev, data);

	ret = component_add(dev, &hisi_dsi_ops);
	if (ret) {
		HISI_DRM_ERR("component_add failed, ret=%d", ret);
		return ret;
	}

	HISI_DRM_INFO("-");

	return 0;
}

static int hisi_dsi_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &hisi_dsi_ops);

	return 0;
}

static const struct of_device_id hisi_mipi_dsi_match_table[] = {
	{
		.compatible = DTS_COMP_DSI_NAME,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_mipi_dsi_match_table);

struct platform_driver hisi_mipi_dsi_driver = {
	.probe = hisi_dsi_probe,
	.remove = hisi_dsi_remove,
	.driver = {
		.name = "hisi-mipi-dsi",
		.owner = THIS_MODULE,
		.of_match_table = hisi_mipi_dsi_match_table,
	},
};

/*lint +e529 +e559 +e578 +e647 +e671 +e838*/
