/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef __DPTX_CORE_H__
#define __DPTX_CORE_H__

#include "../hisi_dp.h"

/**
 * Core interface functions
 */
void dptx_global_intr_en(struct dp_ctrl *dp);
void dptx_global_intr_dis(struct dp_ctrl *dp);
void dptx_global_intr_clear(struct dp_ctrl *dp);
int dptx_core_init(struct dp_ctrl *dptx);
bool dptx_check_dptx_id(struct dp_ctrl *dptx);
void dptx_core_init_phy(struct dp_ctrl *dptx);
int dptx_core_deinit(struct dp_ctrl *dptx);
void dptx_aux_disreset(struct dp_ctrl *dptx, bool enable);
void dptx_soft_reset(struct dp_ctrl *dptx, uint32_t bits);
void dptx_soft_reset_all(struct dp_ctrl *dptx);
void dptx_phy_soft_reset(struct dp_ctrl *dptx);
void dptx_typec_reset_ack(struct dp_ctrl *dptx);
/**
 * PHY IF Control
 */
void dptx_phy_set_lanes_status(struct dp_ctrl *dptx, bool bopen);
void dptx_phy_set_lanes(struct dp_ctrl*dptx, uint32_t num);
int dptx_phy_get_lanes(struct dp_ctrl *dptx);
void dptx_phy_set_rate(struct dp_ctrl *dptx, uint32_t rate);
int dwc_phy_get_rate(struct dp_ctrl *dptx);
int dptx_phy_wait_busy(struct dp_ctrl *dptx, uint32_t lanes);
void dptx_phy_set_pre_emphasis(struct dp_ctrl *dptx,
	uint32_t lane, uint32_t level);
void dptx_phy_set_vswing(struct dp_ctrl *dptx,
	uint32_t lane, uint32_t level);
void dptx_phy_enable_xmit(struct dp_ctrl *dptx,
	uint32_t lane, bool enable);

int dptx_phy_rate_to_bw(uint8_t rate);
int dptx_bw_to_phy_rate(uint8_t bw);
void dptx_phy_set_pattern(struct dp_ctrl *dptx, uint32_t pattern);

#endif
