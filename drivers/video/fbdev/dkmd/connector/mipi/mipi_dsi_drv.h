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
#ifndef __MIPI_DSI_DRV_H__
#define __MIPI_DSI_DRV_H__

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>

#include "dkmd_connector.h"
#include "mipi_cdphy_utils.h"
#include "mipi_dsi_dev.h"

#define DEV_NAME_DSI "mipi_dsi"
#define DTS_COMP_DSI0 "hisilicon,mipi_dsi0"
#define DTS_COMP_DSI1 "hisilicon,mipi_dsi1"

#undef pr_fmt
#define pr_fmt(fmt)  "mipi_dsi: " fmt

struct mipi_dsi_match_data {
	int version_id;
	int (*of_device_setup) (struct platform_device *pdev);
	void (*of_device_release) (struct platform_device *pdev);
};

struct mipi_dsi_drv_private {
	struct dkmd_connector_info base;
	struct mipi_dsi_phy_ctrl dsi_phy_ctrl;
	struct mipi_panel_info mipi;
	struct mipi_panel_info mipi_updt;

	char __iomem *dpu_base;
	char __iomem *dsi_base;
	char __iomem *peri_crg_base;

	struct regulator *dsssubsys_regulator;
	struct regulator *vivobus_regulator;
	struct regulator *media1_subsys_regulator;

	struct clk *aclk_gate_dss;
	struct clk *pclk_gate_dss;
	struct clk *clk_gate_edc;
	struct clk *clk_gate_txdphy_ref;
	struct clk *clk_gate_txdphy_cfg;
	struct clk *pclk_gate_dsi;
	uint32_t irq;

	int id; /* dsi idx: dsi0-0x0 dsi1-0x1 */
	int fpga_flag;
	int device_initialized;

	struct platform_device *pdev;
	/* for dual mipi panel */
	struct platform_device *next_dsi_dev;
};

extern struct platform_device *g_dsi_dev[CONNECTOR_MAX_NUM];
static inline struct mipi_dsi_drv_private *to_mipi_dsi_private(struct platform_device *pdev)
{
	struct dkmd_connector_info *pinfo = NULL;

	if (pdev) {
		pinfo = platform_get_drvdata(pdev);
		if (pinfo) {
			return container_of(pinfo, struct mipi_dsi_drv_private, base);
		}
	}
	return NULL;
}

static inline struct mipi_dsi_drv_private *get_dsi_private_data_by_idx(int idx)
{
	if (idx > CONNECTOR_MAX_NUM) {
		return NULL;
	} else {
		return to_mipi_dsi_private(g_dsi_dev[idx]);
	}
}

#endif
