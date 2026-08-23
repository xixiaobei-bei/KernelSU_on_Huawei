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

#include "dp_aux.h"
#include "link.h"
#include "avgen.h"
#include "core.h"
#include "hisi_fb.h"
#include "intr.h"
#include "../hisi_fb_def.h"
#include "hdcp22/hdcp13.h"


extern u16 usb31phy_cr_read(u32 addr);
extern u16 usb31phy_cr_write(u32 addr, u16 value);

void dptx_combophy_set_preemphasis_swing(struct dp_ctrl *dptx,
			 uint32_t lane,
			 uint32_t sw_level,
			 uint32_t pe_level)
{
	u16 u161n02_addr = 0;
	u16 u161n03_addr = 0;
	u8 phylane = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (dptx->dptx_plug_type == DP_PLUG_TYPE_NORMAL) {
		switch(lane)
		{
			case 0:
				phylane = 2;
				break;
			case 1:
				phylane = 3;
				break;
			case 2:
				phylane = 0;
				break;
			case 3:
				phylane = 1;
				break;
			default:
				HISI_FB_ERR("[DP] Lane number error!\n");
			return;
		}
	} else {
		switch(lane)
		{
			case 0:
				phylane = 1;
				break;
			case 1:
				phylane = 0;
				break;
			case 2:
				phylane = 3;
				break;
			case 3:
				phylane = 2;
				break;
			default:
				HISI_FB_ERR("[DP] Lane number error!\n");
			return;
		}
	}

	switch(phylane)
	{
		case 0:
			u161n02_addr = 0x1002;
			u161n03_addr = 0x1003;
			break;
		case 1:
			u161n02_addr = 0x1102;
			u161n03_addr = 0x1103;
			break;
		case 2:
			u161n02_addr = 0x1202;
			u161n03_addr = 0x1203;
			break;
		case 3:
			u161n02_addr = 0x1302;
			u161n03_addr = 0x1303;
			break;
		default:
			HISI_FB_ERR("[DP] Lane number error!\n");
			return;
	}

	switch (sw_level)
	{
		case 0:
			switch(pe_level)
			{
				case 0:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[1]);//0x9a78(1);0x9BF8(4)
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[2]);
					break;
				case 1:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[3]);//0x9678(1)
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[4]);//0x2440(1)
					break;
				case 2:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);//0x240(1)
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[5]);//0x9478(1)
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[6]);//0x9478
					break;
				case 3:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);//0x240
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[7]);//0x9278(1)
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[8]);//0x2840(1)
					break;
				default:
					HISI_FB_ERR("[DP] preemphasis_ level error \n");
					break;
			}
			break;
		case 1:
			switch(pe_level)
			{
				case 0:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[9]);//0xa678(1)
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[10]);
					break;
				case 1:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);//0x240(1)
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[11]);//0xa078(1)
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[12]);//0x2640(1)
					break;
				case 2:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);//0x240(1)
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[13]);//0x9c78(1)
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[14]);//0x2a40(1)
					break;
				case 3:
					HISI_FB_WARNING("[DP] Don't support sw 1 & ps level 3 \n");
					break;
				default:
					HISI_FB_ERR("[DP] preemphasis_ level error \n");
					return;
			}
			break;
		case 2:
			switch(pe_level)
			{
				case 0:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);//0x280(1) //0x240(2)//0x2c0(3)
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[15]);
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[16]);
					break;
				case 1:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);//0x280(1)
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[17]);//0xa878(1)//0xA9F8(3) //0xA7F8(4)//0xA9F8(5)
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[18]);//0x2840(2)//0x2640(3)//0x2644(5)
					break;
				case 2:
					HISI_FB_WARNING("[DP] Don't support sw 2 & ps level 2 \n");
					break;
				case 3:
					HISI_FB_WARNING("[DP] Don't support sw 2 & ps level 3 \n");
					break;
				default:
					HISI_FB_ERR("[DP] preemphasis_ level error \n");
					return;
			}
			break;
		case 3:
			switch(pe_level)
			{
				case 0:
					usb31phy_cr_write(TX_VBOOST_ADDR, dptx->combophy_pree_swing[0]);
					usb31phy_cr_write(u161n02_addr, dptx->combophy_pree_swing[19]);//0xb078(1)
					usb31phy_cr_write(u161n03_addr, dptx->combophy_pree_swing[20]);
					break;
				case 1:
					HISI_FB_WARNING("[DP] Don't support sw 3 & ps level 1 \n");
					break;
				case 2:
					HISI_FB_WARNING("[DP] Don't support sw 3 & ps level 2 \n");
					break;
				case 3:
					HISI_FB_WARNING("[DP] Don't support sw 3 & ps level 3 \n");
					break;
				default:
					HISI_FB_ERR("[DP] preemphasis_ level error \n");
					return;
			}
			break;
		default:
			HISI_FB_ERR("[DP] vswing level error \n");
			break;
	}
}

int dptx_link_set_ssc(struct dp_ctrl *dptx, bool bswitchphy)
{
	int retval;
	uint8_t byte;
	uint32_t phyifctrl;

	/*FPGA don't need config phy*/
	if (g_fpga_flag == 1)
		return 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (bswitchphy)
	{
		dptx_phy_set_lanes_status(dptx, false);

		/* Wait for PHY busy */
		retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
		if (retval) {
			HISI_FB_ERR("[DP] Timed out waiting for PHY BUSY\n");
			return retval;
		}
	}

	retval = dptx_read_dpcd(dptx, DP_MAX_DOWNSPREAD, &byte);
	if (retval) {
		HISI_FB_ERR("[DP] Read DPCD error\n");
		return retval;
	}

	HISI_FB_INFO("[DP] SSC setting:  %x \n", byte);

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	phyifctrl |= DPTX_PHYIF_CTRL_SSC_DIS;
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);

	if (byte & DP_MAX_DOWN_SPREAD) {
			phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
			phyifctrl &= ~DPTX_PHYIF_CTRL_SSC_DIS;
			dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);

			switch (dptx->link.rate) {
				case DPTX_PHYIF_CTRL_RATE_RBR:
					usb31phy_cr_write(0x36, 0x100a);
					usb31phy_cr_write(0x37, 0x1e6);
					break;
				case DPTX_PHYIF_CTRL_RATE_HBR:
				case DPTX_PHYIF_CTRL_RATE_HBR2:
					usb31phy_cr_write(0x36, 0x101e);
					usb31phy_cr_write(0x37, 0x18a);
					break;
				case DPTX_PHYIF_CTRL_RATE_HBR3:
					usb31phy_cr_write(0x36, 0x1045);
					usb31phy_cr_write(0x37, 0x15c);
					break;
				default:
					HISI_FB_ERR("[DP] Invalid PHY rate %d\n", dptx->link.rate);
					break;
			}

		retval = dptx_read_dpcd(dptx, DP_DOWNSPREAD_CTRL, &byte);
		if (retval) {
			HISI_FB_ERR("[DP] Read DPCD error\n");
			return retval;
		}

		byte |= DP_SPREAD_AMP_0_5;
		retval = dptx_write_dpcd(dptx, DP_DOWNSPREAD_CTRL, byte);
		if (retval) {
			HISI_FB_ERR("[DP] write DPCD error\n");
			return retval;
		}
	} else {
			phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
			phyifctrl |= DPTX_PHYIF_CTRL_SSC_DIS;
			dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);

			usb31phy_cr_write(0x36, 0x25);
			usb31phy_cr_write(0x37, 0x66);

		retval = dptx_read_dpcd(dptx, DP_DOWNSPREAD_CTRL, &byte);
		if (retval) {
			HISI_FB_ERR("[DP] Read DPCD error\n");
			return retval;
		}

		byte &= ~DP_SPREAD_AMP_0_5;
		retval = dptx_write_dpcd(dptx, DP_DOWNSPREAD_CTRL, byte);
		if (retval) {
			HISI_FB_ERR("[DP] write DPCD error\n");
			return retval;
		}
	}

	if (bswitchphy)
	{
		/* Wait for PHY busy */
		retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
		if (retval) {
			HISI_FB_ERR("[DP] Timed out waiting for PHY BUSY\n");
			return retval;
		}

		dptx_phy_set_lanes_status(dptx, true);

		/* Wait for PHY busy */
		retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
		if (retval) {
			HISI_FB_ERR("[DP] Timed out waiting for PHY BUSY\n");
			return retval;
		}

		dptx_phy_enable_xmit(dptx, dptx->link.lanes, true);
	}

	return 0;
}
/*lint -save -e* */
static int dptx_link_read_status(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	return dptx_read_bytes_from_dpcd(dptx, DP_LANE0_1_STATUS,
					 dptx->link.status,
					 DP_LINK_STATUS_SIZE);
}

static int dptx_link_check_cr_done(struct dp_ctrl *dptx, bool *out_done)
{
	int retval;
	uint8_t byte;
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (out_done == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	*out_done = false;

	retval = dptx_read_dpcd(dptx, DP_TRAINING_AUX_RD_INTERVAL, &byte);
	if (retval)
		return retval;

	reg = min_t(uint32_t, (byte & 0x7f), 4);
	reg *= 4000;
	if (!reg)
		reg = 400;

	/*DPTX spec acquire:
	TRAINING_AUX_RD_INTERVAL
	00h = 400us.
	01h = 4ms all.
	02h = 8ms all.
	03h = 12ms all.
	04h = 16ms all.
	*/
	udelay(reg);

	mdelay(4);	/*Fix me: Reducing rate problem*/

	retval = dptx_link_read_status(dptx);
	if (retval)
		return retval;

	*out_done = drm_dp_clock_recovery_ok(dptx->link.status,
						dptx->link.lanes);

	HISI_FB_INFO("[DP] CR_DONE = %d\n", *out_done);

	return 0;
}

static int dptx_link_check_ch_eq_done(struct dp_ctrl *dptx,
				      bool *out_cr_done,
				      bool *out_ch_eq_done)
{
	int retval;
	bool done;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if ((out_cr_done == NULL) || (out_ch_eq_done == NULL)){
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_link_check_cr_done(dptx, &done);
	if (retval)
		return retval;

	*out_cr_done = false;
	*out_ch_eq_done = false;

	if (!done)
		return 0;

	*out_cr_done = true;
	*out_ch_eq_done = drm_dp_channel_eq_ok(dptx->link.status,
					       dptx->link.lanes);

	HISI_FB_INFO("[DP] CH_EQ_DONE = %d\n", *out_ch_eq_done);

	return 0;
}

void dptx_link_set_preemp_vswing(struct dp_ctrl *dptx)
{
	uint32_t i;
	uint8_t pe;
	uint8_t vs;
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	/*FPGA don't need config phy*/
	if (g_fpga_flag == 1)
		return;

	for (i = 0; i < dptx->link.lanes; i++) {
		dp_imonitor_set_param_vs_pe(i, &(dptx->link.vswing_level[i]), &(dptx->link.preemp_level[i]));
		pe = dptx->link.preemp_level[i];
		vs = dptx->link.vswing_level[i];
		HISI_FB_INFO("[DP] lane: (%d), vs: (%d), pe: (%d) \n",i,vs ,pe);
			dptx_combophy_set_preemphasis_swing(dptx, i, vs, pe);
	}

	/* Wait for PHY busy */
	retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
	if (retval) {
		HISI_FB_ERR("[DP] Timed out 1 waiting for PHY BUSY\n");
		return;
	}
}

int dptx_link_training_lanes_set(struct dp_ctrl *dptx)
{
	int retval;
	uint32_t i;
	uint8_t bytes[4] = { 0xff, 0xff, 0xff, 0xff };
	uint8_t byte = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	for (i = 0; i < dptx->link.lanes; i++) {
		byte |= ((dptx->link.vswing_level[i] <<
			  DP_TRAIN_VOLTAGE_SWING_SHIFT) &
			 DP_TRAIN_VOLTAGE_SWING_MASK);

		if (dptx->link.vswing_level[i] == 3)
			byte |= DP_TRAIN_MAX_SWING_REACHED;

		byte |= ((dptx->link.preemp_level[i] <<
			  DP_TRAIN_PRE_EMPHASIS_SHIFT) &
			 DP_TRAIN_PRE_EMPHASIS_MASK);

		if (dptx->link.preemp_level[i] == 3)
			byte |= DP_TRAIN_MAX_PRE_EMPHASIS_REACHED;

		bytes[i] = byte;
	}

	retval = dptx_write_bytes_to_dpcd(dptx, DP_TRAINING_LANE0_SET, bytes,
					  dptx->link.lanes);
	if (retval)
		return retval;

	return 0;
}

int dptx_link_adjust_drive_settings(struct dp_ctrl *dptx, int *out_changed)
{
	int retval;
	uint32_t lanes;
	uint32_t i;
	uint8_t byte;
	uint8_t adj[4] = { 0 };
	int changed = false;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	lanes = dptx->link.lanes;
	HISI_FB_INFO("[DP] Lane: %d",lanes);

	switch (lanes) {
	case 4:
		retval = dptx_read_dpcd(dptx, DP_ADJUST_REQUEST_LANE2_3, &byte);
		if (retval)
			return retval;

		adj[2] = byte & 0x0f;
		adj[3] = (byte & 0xf0) >> 4;
	case 2:
	case 1:
		retval = dptx_read_dpcd(dptx, DP_ADJUST_REQUEST_LANE0_1, &byte);
		if (retval)
			return retval;

		adj[0] = byte & 0x0f;
		adj[1] = (byte & 0xf0) >> 4;
		break;
	default:
		HISI_FB_ERR("[DP] Invalid number of lanes %d\n", lanes);
		return -EINVAL;
	}

	/* Save the drive settings */
	for (i = 0; i < lanes; i++) {
		uint8_t vs = adj[i] & 0x3;
		uint8_t pe = (adj[i] & 0xc) >> 2;

		if (dptx->link.vswing_level[i] != vs)
			changed = true;

		dptx->link.vswing_level[i] = vs;
		dptx->link.preemp_level[i] = pe;
	}

	dptx_link_set_preemp_vswing(dptx);

	retval = dptx_link_training_lanes_set(dptx);
	if (retval)
		return retval;

	if (out_changed)
		*out_changed = changed;

	return 0;
}

static int dptx_link_training_init(struct dp_ctrl *dptx,
				   uint8_t rate,
				   uint8_t lanes)
{
	uint8_t sink_max_rate;
	uint8_t sink_max_lanes;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] lanes=%d, rate=%d\n", lanes, rate);

	if (rate > DPTX_PHYIF_CTRL_RATE_HBR3) {
		HISI_FB_ERR("[DP] Invalid rate %d\n", rate);
		rate = DPTX_PHYIF_CTRL_RATE_RBR;
	}

	if ((!lanes) || (lanes == 3) || (lanes > 4)) {
		HISI_FB_ERR("[DP] Invalid lanes %d\n", lanes);
		lanes = 1;
	}

	/* Initialize link parameters */
	memset(dptx->link.preemp_level, 0, sizeof(uint8_t) * 4);
	memset(dptx->link.vswing_level, 0, sizeof(uint8_t) * 4);
	memset(dptx->link.status, 0, DP_LINK_STATUS_SIZE);

	sink_max_lanes = drm_dp_max_lane_count(dptx->rx_caps);

	if (lanes > sink_max_lanes)
		lanes = sink_max_lanes;

	sink_max_rate = dptx->rx_caps[DP_MAX_LINK_RATE];
	sink_max_rate = dptx_bw_to_phy_rate(sink_max_rate);

	if (rate > sink_max_rate)
		rate = sink_max_rate;

	HISI_FB_INFO("[DP] Sink Device Capability: lanes=%d, rate=%d\n", sink_max_lanes, sink_max_rate);
	dp_imonitor_set_param(DP_PARAM_MAX_RATE,   &sink_max_rate);
	dp_imonitor_set_param(DP_PARAM_MAX_LANES,  &sink_max_lanes);
	dp_imonitor_set_param(DP_PARAM_LINK_RATE,  &rate);
	dp_imonitor_set_param(DP_PARAM_LINK_LANES, &lanes);

	dptx->link.lanes = lanes;
	dptx->link.rate = rate;
	dptx->link.trained = false;

	return 0;
}

int dptx_link_training_pattern_set(struct dp_ctrl *dptx, uint8_t pattern)
{
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_write_dpcd(dptx, DP_TRAINING_PATTERN_SET, pattern);
	if (retval)
		return retval;

	return 0;
}

static int dptx_link_training_start(struct dp_ctrl *dptx)
{
	int retval;
	uint32_t cctl;
	uint8_t byte;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	dptx_phy_set_lanes_status(dptx, false);

	/* Wait for PHY busy */
	retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
	if (retval) {
		HISI_FB_ERR("[DP] Timed out 1 waiting for PHY BUSY\n");
		return retval;
	}

	/* Initialize PHY */
	dptx_phy_set_lanes(dptx, dptx->link.lanes);
	dptx_phy_set_rate(dptx, dptx->link.rate);

	/* Set SSC_DIS = 1? */
	dptx_link_set_ssc(dptx, false);

	/* Wait for PHY busy */
	retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
	if (retval) {
		HISI_FB_ERR("[DP] Timed out 2 waiting for PHY BUSY\n");
		return retval;
	}

	dptx_phy_set_lanes_status(dptx, true);

	/* Wait for PHY busy */
	retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
	if (retval) {
		HISI_FB_ERR("[DP] Timed out 3 waiting for PHY BUSY\n");
		return retval;
	}

	/* Set PHY_TX_EQ */
	dptx_link_set_preemp_vswing(dptx);

	dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);
	retval = dptx_link_training_pattern_set(dptx,
						DP_TRAINING_PATTERN_DISABLE);
	if (retval)
		return retval;

	dptx_phy_enable_xmit(dptx, dptx->link.lanes, true);

	retval = dptx_phy_rate_to_bw(dptx->link.rate);
	if (retval < 0)
		return retval;

	byte = (uint8_t)retval;
	retval = dptx_write_dpcd(dptx, DP_LINK_BW_SET, byte);
	if (retval)
		return retval;

	byte = dptx->link.lanes;
	cctl = dptx_readl(dptx, DPTX_CCTL);

	if (drm_dp_enhanced_frame_cap(dptx->rx_caps)) {
		byte |= DP_ENHANCED_FRAME_CAP;
		cctl |= DPTX_CCTL_ENH_FRAME_EN;
	} else {
		cctl &= ~DPTX_CCTL_ENH_FRAME_EN;
	}

	dptx_writel(dptx, DPTX_CCTL, cctl);

	retval = dptx_write_dpcd(dptx, DP_LANE_COUNT_SET, byte);
	if (retval)
		return retval;

	byte = DP_SPREAD_AMP_0_5;
	retval = dptx_write_dpcd(dptx, DP_DOWNSPREAD_CTRL, byte);
	if (retval)
		return retval;

	byte = 1;
	retval = dptx_write_dpcd(dptx, DP_MAIN_LINK_CHANNEL_CODING_SET, byte);
	if (retval)
		return retval;

	return 0;
}

int dptx_link_wait_cr_and_adjust(struct dp_ctrl *dptx, bool ch_eq)
{
	int i;
	int retval;
	int changed = 0;
	bool done = false;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_link_check_cr_done(dptx, &done);
	if (retval)
		return retval;

	if (done)
		return 0;

	/* Try each adjustment setting 5 times */
	for (i = 0; i < 5; i++) {
		retval = dptx_link_adjust_drive_settings(dptx, &changed);
		if (retval)
			return retval;

		/* Reset iteration count if we changed settings */
		if (changed)
			i = 0;

		retval = dptx_link_check_cr_done(dptx, &done);
		if (retval)
			return retval;

		if (done)
			return 0;

		/* TODO check for all lanes? */
		/* Failed and reached the maximum voltage swing */
		if (dptx->link.vswing_level[0] == 3)
			return -EPROTO;
	}

	return -EPROTO;
}

int dptx_link_cr(struct dp_ctrl *dptx)
{
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] link training excute cr verify\n");

	dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_1);

	retval = dptx_link_training_pattern_set(dptx,
						DP_TRAINING_PATTERN_1 | 0x20);

	if (retval)
		return retval;

	return dptx_link_wait_cr_and_adjust(dptx, false);
}

int dptx_link_ch_eq(struct dp_ctrl *dptx)
{
	int retval;
	bool cr_done;
	bool ch_eq_done;
	uint32_t pattern;
	uint32_t i;
	uint8_t dp_pattern;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] link training excute eq verify\n");

	switch (dptx->max_rate) {
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		if (drm_dp_tps4_supported(dptx->rx_caps)) {
			pattern = DPTX_PHYIF_CTRL_TPS_4;
			dp_pattern = DP_TRAINING_PATTERN_4;
			break;
		}
		/* Fall through */
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		if (drm_dp_tps3_supported(dptx->rx_caps)) {
			pattern = DPTX_PHYIF_CTRL_TPS_3;
			dp_pattern = DP_TRAINING_PATTERN_3;
			break;
		}
		/* Fall through */
	case DPTX_PHYIF_CTRL_RATE_RBR:
	case DPTX_PHYIF_CTRL_RATE_HBR:
		pattern = DPTX_PHYIF_CTRL_TPS_2;
		dp_pattern = DP_TRAINING_PATTERN_2;
		break;
	default:
		HISI_FB_ERR("[DP] Invalid rate %d\n", dptx->link.rate);
		return -EINVAL;
	}

	dptx_phy_set_pattern(dptx, pattern);

	/* TODO this needs to be different for other versions of
	 * DPRX
	 */
	retval = dptx_link_training_pattern_set(dptx,
						dp_pattern | 0x20);
	if (retval)
		return retval;

	for (i = 0; i < 6; i++) {
		retval = dptx_link_check_ch_eq_done(dptx,
						    &cr_done,
						    &ch_eq_done);

		if (retval)
			return retval;

		if (!cr_done)
			return -EPROTO;

		if (ch_eq_done)
			return 0;

		retval = dptx_link_adjust_drive_settings(dptx, NULL);

		if (retval)
			return retval;
	}

	return -EPROTO;
}

int dptx_link_reduce_rate(struct dp_ctrl *dptx)
{
	uint32_t rate = dptx->link.rate;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		return -EPROTO;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		rate = DPTX_PHYIF_CTRL_RATE_RBR;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		rate = DPTX_PHYIF_CTRL_RATE_HBR;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		rate = DPTX_PHYIF_CTRL_RATE_HBR2;
		break;
	}

	HISI_FB_INFO("[DP] Reducing rate from %d to %d\n",
					dptx->link.rate, rate);
	dptx->link.rate = rate;
	return 0;
}

int dptx_link_reduce_lanes(struct dp_ctrl *dptx)
{
	uint32_t lanes;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	switch (dptx->link.lanes) {
	case 4:
		lanes = 2;
		break;
	case 2:
		lanes = 1;
		break;
	case 1:
	default:
		return -EPROTO;
	}

	HISI_FB_INFO("[DP] Reducing lanes from %d to %d\n",
					dptx->link.lanes, lanes);
	dptx->link.lanes = lanes;
	return 0;
}

int dptx_link_training(struct dp_ctrl *dptx, uint8_t rate, uint8_t lanes)
{
	int retval;
	uint8_t byte;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	dptx_triger_media_transfer(dptx, false);

	retval = dptx_link_training_init(dptx, rate, lanes);
	if (retval)
		goto fail;

again:
	HISI_FB_INFO("[DP] Link training start!\n");
	retval = dptx_link_training_start(dptx);
	if (retval)
		goto fail;

	retval = dptx_link_cr(dptx);
	if (retval) {
		if (retval == -EPROTO) {
			// for factory test
			if (dp_factory_mode_is_enable()) {
				dp_factory_link_cr_or_ch_eq_fail(true);
				HISI_FB_ERR("[DP] not support reduce rate by CR verify in factory mode!\n");
				retval = -ECONNREFUSED;
				goto fail;
			}

			HISI_FB_INFO("[DP] Reduce rate by CR verify\n");
			if (dptx_link_reduce_rate(dptx)) {
				/* TODO If CR_DONE bits for some lanes
				 * are set, we should reduce lanes to
				 * those lanes.
				 */
				if (dptx_link_reduce_lanes(dptx)) {
					retval = -EPROTO;
					goto fail;
				}
			}

			dptx_link_training_init(dptx,
						dptx->link.rate,
						dptx->link.lanes);
			goto again;
		} else {
			goto fail;
		}
	}

	retval = dptx_link_ch_eq(dptx);
	if (retval) {
		if (retval == -EPROTO) {
			// for factory test
			if (dp_factory_mode_is_enable()) {
				dp_factory_link_cr_or_ch_eq_fail(false);
				HISI_FB_ERR("[DP] not support reduce rate by EQ verify in factory mode!\n");
				retval = -ECONNREFUSED;
				goto fail;
			}

			HISI_FB_INFO("[DP] Reduce rate by EQ verify\n");
			if (dptx_link_reduce_rate(dptx)) {
				if (dptx_link_reduce_lanes(dptx)) {
					retval = -EPROTO;
					goto fail;
				}
			}

			dptx_link_training_init(dptx,
						dptx->link.rate,
						dptx->link.lanes);
			goto again;
		} else {
			goto fail;
		}
	}

	dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);

	retval = dptx_link_training_pattern_set(dptx,
						DP_TRAINING_PATTERN_DISABLE);
	if (retval)
		goto fail;

	//dptx_enable_default_video_stream(dptx);
	dptx_phy_enable_xmit(dptx, dptx->link.lanes, true);
	dptx->link.trained = true;

	/* Branch device detection */
	retval = dptx_read_dpcd(dptx, DP_SINK_COUNT, &byte);
	if (retval)
		return retval;
	HDCP_Read_TEInfo(dptx);
	dptx_video_ts_change(dptx);
	HISI_FB_INFO("[DP] Link training succeeded rate=%d lanes=%d\n",
		 dptx->link.rate, dptx->link.lanes);

	dptx_triger_media_transfer(dptx, true);

	dptx->detect_times = 0;

	return 0;

fail:
	HISI_FB_ERR("[DP] Link training failed %d\n", retval);
	return retval;
}

int dptx_link_check_status(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t byte;
	uint8_t bytes[2];

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_read_bytes_from_dpcd(dptx, DP_SINK_COUNT,
					   bytes, 2);
	if (retval)
		return retval;

	retval = dptx_link_read_status(dptx);
	if (retval)
		return retval;

	byte = dptx->link.status[DP_LANE_ALIGN_STATUS_UPDATED -
				 DP_LANE0_1_STATUS];

	if (!(byte & DP_LINK_STATUS_UPDATED))
		return 0;

	/* Check if need to retrain link */
	if (dptx->link.trained &&
	    (!drm_dp_channel_eq_ok(dptx->link.status, dptx->link.lanes) ||
	     !drm_dp_clock_recovery_ok(dptx->link.status, dptx->link.lanes))) {
		HISI_FB_INFO("[DP] Retraining link\n");
		dp_imonitor_set_param(DP_PARAM_LINK_RETRAINING, NULL);

		retval = dptx_link_training(dptx,
					  DPTX_MAX_LINK_RATE,
					  DPTX_MAX_LINK_LANES);
		if (retval < 0) {
			dp_imonitor_set_param(DP_PARAM_LINK_TRAINING_FAILED, &retval);
		}
		return retval;
	}

	return 0;
}

/**
* dptx_link_retrain() - Retrain link
* @dptx: The dptx struct
* @rate: Link rate - Possible options: 0 - RBR, 1 - HBR, 2 - HBR2, 3 - HBR3
* @lanes: Link lanes count - Possible options 1,2 or 4
*
* Returns 0 on success otherwise negative errno.
*/
int dptx_link_retraining(struct dp_ctrl *dptx, u8 rate, u8 lanes)
{
	u32 hpdsts;
	struct video_params *vparams;
	struct dtd *mdtd;
	int i, retval;

	hpdsts = dptx_readl(dptx, DPTX_HPDSTS);
	retval = 0;

	if (!(hpdsts & g_bit_hpd_status)) {
		HISI_FB_ERR("[DP] Not connected\n");
		return  -ENODEV;
	}

	if (lanes != 1 && lanes != 2 && lanes != 4) {
		HISI_FB_ERR("[DP] Invalid number of lanes %d\n",
			lanes);
		return  -EINVAL;
	}

	if (rate > DPTX_PHYIF_CTRL_RATE_HBR3) {
		HISI_FB_ERR("[DP] Invalid number of lane rate %d\n",
			rate);
		return  -EINVAL;
	}

	for (i = 0; i < 3; i++) {
		retval = dptx_link_training(dptx, rate, lanes);
		if (retval)
			return retval;

		if ((rate == dptx->link.rate) && (lanes == dptx->link.lanes)) {
			break;
		} else {
			HISI_FB_INFO("[DP] link retrain reduce rate, try again !!!\n");
			mdelay(100);
		}
	}

	if (i == 3) {
		HISI_FB_ERR("[DP] Link retrain reduce rate!!!\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;
	retval = dptx_video_ts_calculate(dptx, dptx->link.lanes, dptx->link.rate, vparams->bpc,
				vparams->pix_enc, mdtd->pixel_clock);
	if (retval)
		return retval;

	HISI_FB_INFO("[DP] Retraining link rate=%d lanes=%d\n",
		rate, lanes);

	return retval;
}
/*lint -restore*/
