#ifndef HISI_DRM_DSI_H
#define HISI_DRM_DSI_H

#include <linux/clk.h>
#include <linux/component.h>
#include <linux/of_graph.h>
#include <linux/iopoll.h>
#include <video/mipi_display.h>
#include <linux/gpio/consumer.h>
#include <linux/of_address.h>

#include <drm/drm_of.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_encoder_slave.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_panel.h>

#include "hisi_panel.h"

#define DTS_COMP_DSI_NAME "hisilicon,kunpeng902-dsi"

enum dsi_work_mode {
	DSI_VIDEO_MODE,
	DSI_CMD_MODE,
};

enum {
	DSI_1_1_VERSION = 0,
	DSI_1_2_VERSION,
};

enum {
	DSI_1_LANES = 0,
	DSI_2_LANES,
	DSI_3_LANES,
	DSI_4_LANES,
};

enum {
	DSI_LANE_NUMS_DEFAULT = 0,
	DSI_1_LANES_SUPPORT = BIT(0),
	DSI_2_LANES_SUPPORT = BIT(1),
	DSI_3_LANES_SUPPORT = BIT(2),
	DSI_4_LANES_SUPPORT = BIT(3),
};

enum {
	DSI_16BITS_1 = 0,
	DSI_16BITS_2,
	DSI_16BITS_3,
	DSI_18BITS_1,
	DSI_18BITS_2,
	DSI_24BITS_1,
	DSI_24BITS_2,
	DSI_24BITS_3,
	DSI_DSC24_COMPRESSED_DATA = 0xF,
};

enum {
	DSI_NON_BURST_SYNC_PULSES = 0,
	DSI_NON_BURST_SYNC_EVENTS,
	DSI_BURST_SYNC_PULSES_1,
	DSI_BURST_SYNC_PULSES_2,
};
#define DSI_VIDEO_DST_FORMAT_RGB565 0
#define DSI_VIDEO_DST_FORMAT_RGB666 1
#define DSI_VIDEO_DST_FORMAT_RGB666_LOOSE 2
#define DSI_VIDEO_DST_FORMAT_RGB888 3

#define DSI_CMD_DST_FORMAT_RGB565 0
#define DSI_CMD_DST_FORMAT_RGB666 1
#define DSI_CMD_DST_FORMAT_RGB888 2

#define GEN_VID_LP_CMD BIT(24) /* vid lowpwr cmd write*/
/* dcs read/write */
#define DTYPE_DCS_WRITE  0x05 /* short write, 0 parameter */
#define DTYPE_DCS_WRITE1 0x15 /* short write, 1 parameter */
#define DTYPE_DCS_READ   0x06 /* read */
#define DTYPE_DCS_LWRITE 0x39 /* long write */
#define DTYPE_DSC_LWRITE 0x0A /* dsc dsi1.2 vase3x long write */

/* generic read/write */
#define DTYPE_GEN_WRITE  0x03 /* short write, 0 parameter */
#define DTYPE_GEN_WRITE1 0x13 /* short write, 1 parameter */
#define DTYPE_GEN_WRITE2 0x23 /* short write, 2 parameter */
#define DTYPE_GEN_LWRITE 0x29 /* long write */
#define DTYPE_GEN_READ   0x04 /* long read, 0 parameter */
#define DTYPE_GEN_READ1  0x14 /* long read, 1 parameter */
#define DTYPE_GEN_READ2  0x24 /* long read, 2 parameter */

#define DTYPE_TEAR_ON     0x35 /* set tear on */
#define DTYPE_MAX_PKTSIZE 0x37 /* set max packet size */
#define DTYPE_NULL_PKT    0x09 /* null packet, no data */
#define DTYPE_BLANK_PKT   0x19 /* blankiing packet, no data */

#define DTYPE_CM_ON  0x02 /* color mode off */
#define DTYPE_CM_OFF 0x12 /* color mode on */
#define DTYPE_PERIPHERAL_OFF 0x22
#define DTYPE_PERIPHERAL_ON  0x32

#define DSI_HDR_DTYPE(dtype) ((dtype) & 0x03f)
#define DSI_HDR_VC(vc)       (((vc) & 0x03) << 6)
#define DSI_HDR_DATA1(data)  (((data) & 0x0ff) << 8)
#define DSI_HDR_DATA2(data)  (((data) & 0x0ff) << 16)
#define DSI_HDR_WC(wc)       (((wc) & 0x0ffff) << 8)

#define DSI_PLD_DATA1(data) ((data) & 0x0ff)
#define DSI_PLD_DATA2(data) (((data) & 0x0ff) << 8)
#define DSI_PLD_DATA3(data) (((data) & 0x0ff) << 16)
#define DSI_PLD_DATA4(data) (((data) & 0x0ff) << 24)


#define ROUND(x, y) ((x) / (y) + \
				((x) % (y) * 10 / (y) >= 5 ? 1 : 0))
#define ROUND1(x, y) ((x) / (y) + ((x) % (y)  ? 1 : 0))
#define PHY_REF_CLK_RATE 19200000
#define PHY_REF_CLK_PERIOD_PS (1000000000 / (PHY_REF_CLK_RATE / 1000))

#define encoder_to_dsi(encoder) \
	container_of(encoder, struct hisi_dsi, encoder)
#define host_to_dsi(host) \
	container_of(host, struct hisi_dsi, host)
#define connector_to_dsi(connector) \
	container_of(connector, struct hisi_dsi, connector)
#define DSS_REDUCE(x) ((x) > 0 ? ((x) - 1) : (x))

enum dsi_output_client {
	OUT_HDMI = 0,
	OUT_PANEL,
	OUT_MAX
};

struct dsi_hw_ctx {
	void __iomem *dss_base;
	void __iomem *mipi_dsi0_base;
	void __iomem *mipi_dsi1_base;
	char __iomem *peri_crg_base;
	char __iomem *pctrl_base;
	char __iomem *pmctrl_base;

	struct clk *dss_dphy0_ref_clk;
	struct clk *dss_dphy1_ref_clk;
	struct clk *dss_dphy0_cfg_clk;
	struct clk *dss_dphy1_cfg_clk;
	struct clk *dss_pclk_dsi0_clk;
	struct clk *dss_pclk_dsi1_clk;

	uint32_t dsi0_irq;
};

struct hisi_dsi_client {
	u32 lanes;
	u32 phy_clock; /* in kHz */
	enum mipi_dsi_pixel_format format;
	unsigned long mode_flags;
};

struct hisi_dsi {
	struct drm_encoder encoder;
	struct drm_bridge *bridge;
	struct device_node *panel_node;
	struct device_node *bridge_node;
	struct drm_panel *panel;
	struct mipi_dsi_host host;
	struct drm_connector connector; /* connector for panel */
	struct drm_display_mode cur_mode;
	struct dsi_hw_ctx *ctx;
	struct mipi_dsi_phy_ctrl phy;
	struct mipi_panel_info mipi;
	struct ldi_panel_info ldi;
	struct hisi_panel_info *panel_info;
	struct hisi_dsi_client client;
	struct hisi_drm_crtc *hisi_crtc;
	bool enable;
	bool is_pre_enabled;
};

struct dsi_data {
	struct hisi_dsi dsi;
	struct dsi_hw_ctx ctx;
};

struct dsi_phy_range {
	u32 min_range_khz;
	u32 max_range_khz;
	u32 pll_vco_750m;
	u32 hstx_ckg_sel;
};

struct dsi_cmd_desc {
	uint32_t dtype;
	uint32_t vc;
	uint32_t wait;
	uint32_t waittype;
	uint32_t dlen;
	char *payload;
};

struct mipi_dsi_read_compare_data {
	uint32_t *read_value;
	uint32_t *expected_value;
	uint32_t *read_mask;
	char **reg_name;
	int log_on;
	struct dsi_cmd_desc *cmds;
	int cnt;
};

struct dsi_phy_seq_info {
	uint32_t min_range;
	uint32_t max_range;
	uint32_t rg_pll_vco_750m;
	uint32_t rg_hstx_ckg_sel;
};

int mipi_dsi_on_sub1(struct hisi_dsi *dsi, char __iomem *mipi_dsi_base);
int mipi_dsi_on_sub2(struct hisi_dsi *dsi, char __iomem *mipi_dsi_base);
irqreturn_t dss_dsi0_isr(int irq, void *ptr);
void get_dsi_dphy_ctrl(struct hisi_dsi *dsi,
	struct mipi_dsi_phy_ctrl *phy_ctrl);
void get_dsi_cphy_ctrl(struct hisi_dsi *dsi,
	struct mipi_dsi_phy_ctrl *phy_ctrl);

int mipi_dsi_clk_disable(struct hisi_dsi *dsi);
int mipi_dsi_off_sub(char __iomem *mipi_dsi_base);
int mipi_dsi_clk_enable(struct hisi_dsi *dsi);
int mipi_ifbc_get_rect(struct hisi_dsi *dsi, struct dss_rect *rect);
int mipi_dsi_cmds_tx(struct mipi_dsi_device *dsi,
	struct dsi_cmd_desc *cmds, int cnt);

bool is_dual_mipi(struct hisi_dsi *dsi);

#endif
