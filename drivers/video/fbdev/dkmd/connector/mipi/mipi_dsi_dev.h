/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __MIPI_DSI_DEV_H__
#define __MIPI_DSI_DEV_H__

#include "dkmd_connector.h"

/* mipi dsi panel */
enum {
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

enum {
	EN_DSI_TX_NORMAL_MODE = 0x0,
	EN_DSI_TX_LOW_PRIORITY_DELAY_MODE = 0x1,
	EN_DSI_TX_HIGH_PRIORITY_DELAY_MODE = 0x2,
	EN_DSI_TX_AUTO_MODE = 0xF,
};

enum MIPI_LP11_MODE {
	MIPI_NORMAL_LP11 = 0,
	MIPI_SHORT_LP11 = 1,
	MIPI_DISABLE_LP11 = 2,
};

enum V320_DPHY_VER {
	DPHY_VER_12 = 0,
	DPHY_VER_14,
	DPHY_VER_MAX
};

enum DSI_TE_TYPE {
	DSI0_TE0_TYPE = 0,  /* include video lcd */
	DSI1_TE0_TYPE = 1,
	DSI1_TE1_TYPE = 2,
};

enum {
	CONNECTOR_DSI0,
	CONNECTOR_DSI1,
	CONNECTOR_DSI2,
	CONNECTOR_MAX_NUM,
};

#define DSI_VIDEO_DST_FORMAT_RGB565 0
#define DSI_VIDEO_DST_FORMAT_RGB666 1
#define DSI_VIDEO_DST_FORMAT_RGB666_LOOSE 2
#define DSI_VIDEO_DST_FORMAT_RGB888 3

#define DSI_CMD_DST_FORMAT_RGB565 0
#define DSI_CMD_DST_FORMAT_RGB666 1
#define DSI_CMD_DST_FORMAT_RGB888 2

#define GEN_VID_LP_CMD BIT(24) /* vid lowpwr cmd write */
/* dcs read/write */
#define DTYPE_DCS_WRITE 0x05 /* short write, 0 parameter */
#define DTYPE_DCS_WRITE1 0x15 /* short write, 1 parameter */
#define DTYPE_DCS_WRITE2 0x07 /* short write, 2 parameter */
#define DTYPE_DCS_READ 0x06 /* read */
#define DTYPE_DCS_LWRITE 0x39 /* long write */
#define DTYPE_DSC_LWRITE 0x0A /* dsc dsi1.2 vase3x long write */

/* generic read/write */
#define DTYPE_GEN_WRITE 0x03 /* short write, 0 parameter */
#define DTYPE_GEN_WRITE1 0x13 /* short write, 1 parameter */
#define DTYPE_GEN_WRITE2 0x23 /* short write, 2 parameter */
#define DTYPE_GEN_LWRITE 0x29 /* long write */
#define DTYPE_GEN_READ 0x04 /* long read, 0 parameter */
#define DTYPE_GEN_READ1 0x14 /* long read, 1 parameter */
#define DTYPE_GEN_READ2 0x24 /* long read, 2 parameter */

#define DTYPE_TEAR_ON 0x35 /* set tear on */
#define DTYPE_MAX_PKTSIZE 0x37 /* set max packet size */
#define DTYPE_NULL_PKT 0x09 /* null packet, no data */
#define DTYPE_BLANK_PKT 0x19 /* blankiing packet, no data */

#define DTYPE_CM_ON 0x02 /* color mode off */
#define DTYPE_CM_OFF 0x12 /* color mode on */
#define DTYPE_PERIPHERAL_OFF 0x22
#define DTYPE_PERIPHERAL_ON 0x32

#define dsi_hdr_dtype(dtype) ((dtype) & 0x03f)
#define dsi_hdr_vc(vc) (((vc) & 0x03) << 6)
#define dsi_hdr_data1(data) (((data) & 0x0ff) << 8)
#define dsi_hdr_data2(data) (((data) & 0x0ff) << 16)
#define dsi_hdr_wc(wc) (((wc) & 0x0ffff) << 8)

#define dsi_pld_data1(data) ((data) & 0x0ff)
#define dsi_pld_data2(data) (((data) & 0x0ff) << 8)
#define dsi_pld_data3(data) (((data) & 0x0ff) << 16)
#define dsi_pld_data4(data) (((data) & 0x0ff) << 24)

struct mipi_dsi_timing {
	uint32_t hsa;
	uint32_t hbp;
	uint32_t dpi_hsize;
	uint32_t width;
	uint32_t hline_time;

	uint32_t vsa;
	uint32_t vbp;
	uint32_t vactive_line;
	uint32_t vfp;
};

struct mipi_panel_info {
	uint8_t dsi_version;
	uint8_t vc;
	uint8_t lane_nums;
	uint8_t lane_nums_select_support;
	uint8_t color_mode;
	uint32_t dsi_bit_clk; /* clock lane(p/n) */
	uint32_t dsi_bit_clk_default;
	uint32_t burst_mode;
	uint32_t max_tx_esc_clk;
	uint8_t non_continue_en;
	uint8_t txoff_rxulps_en;
	int frame_rate;
	int take_effect_delayed_frm_cnt;

	uint32_t hsa;
	uint32_t hbp;
	uint32_t dpi_hsize;
	uint32_t width;
	uint32_t hline_time;

	uint32_t vsa;
	uint32_t vbp;
	uint32_t vactive_line;
	uint32_t vfp;

	uint32_t dsi_bit_clk_upt_support;
	uint32_t dsi_bit_clk_val1;
	uint32_t dsi_bit_clk_val2;
	uint32_t dsi_bit_clk_val3;
	uint32_t dsi_bit_clk_val4;
	uint32_t dsi_bit_clk_val5;
	uint32_t dsi_bit_clk_upt;

	uint32_t hs_wr_to_time;

	/* dphy config parameter adjust */
	uint32_t clk_post_adjust;
	uint32_t clk_pre_adjust;
	uint32_t clk_pre_delay_adjust;
	int clk_t_hs_exit_adjust;
	int clk_t_hs_trial_adjust;
	uint32_t clk_t_hs_prepare_adjust;
	int clk_t_lpx_adjust;
	uint32_t clk_t_hs_zero_adjust;
	uint32_t data_post_delay_adjust;
	int data_t_lpx_adjust;
	uint32_t data_t_hs_prepare_adjust;
	uint32_t data_t_hs_zero_adjust;
	int data_t_hs_trial_adjust;
	uint32_t rg_vrefsel_vcm_adjust;
	uint32_t support_de_emphasis;
	uint32_t rg_vrefsel_lptx_adjust;
	uint32_t rg_lptx_sri_adjust;
	int data_lane_lp2hs_time_adjust;

	uint32_t phy_mode;  /* 0: DPHY, 1:CPHY */
	uint32_t lp11_flag; /* 0: nomal_lp11, 1:short_lp11, 2:disable_lp11 */
	uint32_t phy_m_n_count_update;  /* 0:old ,1:new can get 988.8M */
	uint32_t eotp_disable_flag; /* 0: eotp enable, 1:eotp disable */

	uint8_t mininum_phy_timing_flag; /* 1:support entering lp11 with minimum clock */

	uint32_t dynamic_dsc_en; /* used for dfr */
	uint32_t dsi_te_type; /* 0: dsi0&te0, 1: dsi1&te0, 2: dsi1&te1 */

	uint64_t pxl_clk_rate;
	uint32_t pxl_clk_rate_div;
};

struct dsi_dev_private_data {
	struct dkmd_connector_data base;

	/* add other function define */
	struct panel_drv_private *panel_priv;
};

struct dsi_ops_handle_data {
	char *ops_cmd;
	int (*handle_func)(struct platform_device *pdev, void *desc);
};

static inline struct dsi_dev_private_data *to_dsi_dev_private_data(struct platform_device *pdev)
{
	struct dkmd_connector_data *pdata = dev_get_platdata(&pdev->dev);

	return container_of(pdata, struct dsi_dev_private_data, base);
}

struct composer_dev_private_data {
	struct dkmd_connector_data base;
	struct platform_device *pdev;
};

int dsi_device_register(struct platform_device *pdev);
int dsi_device_unregister(struct platform_device *pdev);

#endif