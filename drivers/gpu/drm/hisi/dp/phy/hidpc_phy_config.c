/*
 * Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#include "hisi_dp_drv.h"
#include "hisi_defs.h"
#include "dp_phy_config_interface.h"
#include "../link/dp_aux.h"
#include "../controller/hidpc/hidpc_dp_core.h"

#include <linux/hisi/usb/hisi_usb.h>

/*DP & USB control REG*/
#define DPTX_DP_AUX_CTRL 0x05C

#define DPTX_DPC_AUX_CTRL_BLOCK       BIT(10)
#define DPTX_TYPEC_AUX_VOD_TUNE_SHIFT 4
#define DPTX_TYPEC_AUX_VOD_TUNE_MASK  GENMASK(5, 4)

#define U161U02_BASE_ADDR 0x1002
#define U161U03_BASE_ADDR 0x1003
#define U161U02_OFFSET    0x100

static void dptx_combophy_set_preemphasis_swing(
	struct dp_ctrl *dptx, u32 lane, u32 sw_level, u32 pe_level)
{
	u16 u161n02_addr;
	u16 u161n03_addr;
	u8 phylane;

	/* PHY lane array, lane : 0, 1 ,2 ,3 */
	u32 normal_phy_lane[4] = {2, 3, 0, 1};
	u32 flipped_phy_lane[4] = {1, 0, 3, 2};

	if (lane > 3) {
		HISI_DRM_ERR("[DP] Lane number error!\n");
		return;
	}

	if (dptx->dptx_plug_type == DP_PLUG_TYPE_NORMAL)
		phylane = normal_phy_lane[lane];
	else
		phylane = flipped_phy_lane[lane];

	u161n02_addr = U161U02_BASE_ADDR + (phylane * U161U02_OFFSET);
	u161n03_addr = U161U03_BASE_ADDR + (phylane * U161U02_OFFSET);

	/* Inferred formula to calculate combophy_pree_swing array index:
	 * index = sw_level * (9 - sw_level) + 2 * pe_level + 1
	 */
	if (sw_level <= 3) {
		if (sw_level + pe_level <= 3) {
			usb31phy_cr_write(TX_VBOOST_ADDR,
				dptx->combophy_pree_swing[0]);
			usb31phy_cr_write(u161n02_addr,
				dptx->combophy_pree_swing[sw_level * (9 - sw_level) + 2 * pe_level + 1]);
			usb31phy_cr_write(u161n03_addr,
				dptx->combophy_pree_swing[sw_level * (9 - sw_level) + 2 * pe_level + 2]);
		} else if (pe_level <= 3) {
			HISI_DRM_WARN("[DP] not support sw %u & ps level %u", sw_level, pe_level);
		} else {
			HISI_DRM_ERR("[DP] preemphasis_ level error\n");
		}
	} else {
		HISI_DRM_ERR("[DP] vswing level error\n");
	}
}

static int dptx_change_physetting_hbr2(struct dp_ctrl *dptx)
{
	switch (dptx->link.rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
	case DPTX_PHYIF_CTRL_RATE_HBR:
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		usb31phy_cr_write(0x12, 0x1887);
		usb31phy_cr_write(0x13, 0x212);
		usb31phy_cr_write(0x19, 0x4000);
		usb31phy_cr_write(0x1a, 0x0);
		usb31phy_cr_write(0x1b, 0x1);
		usb31phy_cr_write(0x1c, 0x210);
		break;
	default:
		HISI_DRM_ERR("[DP] Invalid PHY rate %d\n", dptx->link.rate);
		break;
	}

	return 0;
}

static void dptx_aux_disreset(struct dp_ctrl *dptx, bool enable)
{
	u32 reg;

	/* Enable AUX Block */
	reg = (u32)inp32(dptx->dp_phy_base + DPTX_DP_AUX_CTRL); //lint !e838

	if (enable)
		reg |= DPTX_DPC_AUX_CTRL_BLOCK;  //lint !e737 !e713
	else
		reg &= ~DPTX_DPC_AUX_CTRL_BLOCK; //lint !e737 !e713

	reg &= ~DPTX_TYPEC_AUX_VOD_TUNE_MASK;
	reg |= (3 << DPTX_TYPEC_AUX_VOD_TUNE_SHIFT);

	outp32(dptx->dp_phy_base + DPTX_DP_AUX_CTRL, reg);

	mdelay(1); //lint !e778 !e747 !e774
}

void dptx_phy_layer_init(struct dp_ctrl *dptx)
{
	drm_check_and_void_return((dptx == NULL), "[DP] dptx is NULL!\n");

	dptx->dptx_combophy_set_preemphasis_swing = dptx_combophy_set_preemphasis_swing;
	dptx->dptx_change_physetting_hbr2 = dptx_change_physetting_hbr2;
	dptx->dptx_aux_disreset = dptx_aux_disreset;
}

