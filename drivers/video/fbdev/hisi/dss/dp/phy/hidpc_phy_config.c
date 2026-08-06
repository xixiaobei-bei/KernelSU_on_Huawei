/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Copyright (c) 2020 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#include "dp_phy_config_interface.h"
#include "hisi_dp.h"
#include "hisi_fb.h"
#include "hisi_fb_def.h"
#include <linux/hisi/usb/hisi_usb.h>


#define DPTX_WRAP_REG_DPC_AUX_CTRL  0x12298
#define DPTX_DPC_AUX_CTRL_BLOCK		BIT(0)
#define DPTX_TYPEC_AUX_VOD_TUNE_SHIFT	12
#define DPTX_TYPEC_AUX_VOD_TUNE_MASK	GENMASK(13, 12)

void dptx_combophy_set_lanes_power(bool bopen)
{
#if CONFIG_DP_SETTING_COMBOPHY && defined(CONFIG_HISI_FB_970)
	if (bopen) {
		usb31phy_cr_write(0x4, 0x211);
		usb31phy_cr_write(0x4, 0);
	}
#endif
}

/* (plug_type, lane_num) -> (phylane) mapping */
static const struct phylane_mapping phylane_mappings[] = {
	{DP_PLUG_TYPE_NORMAL, 0, 2},
	{DP_PLUG_TYPE_NORMAL, 1, 3},
	{DP_PLUG_TYPE_NORMAL, 2, 0},
	{DP_PLUG_TYPE_NORMAL, 3, 1},
	{DP_PLUG_TYPE_FLIPPED, 0, 1},
	{DP_PLUG_TYPE_FLIPPED, 1, 0},
	{DP_PLUG_TYPE_FLIPPED, 2, 3},
	{DP_PLUG_TYPE_FLIPPED, 3, 2},
};

static int dptx_mapping_phy_lane(uint32_t lane, int plug_type,
	uint16_t *u161n02_addr, uint16_t *u161n03_addr)
{
	uint32_t i;
	uint8_t phylane = 0;

	for (i = 0; i < ARRAY_SIZE(phylane_mappings); i++) {
		if (plug_type == phylane_mappings[i].plug_type && lane == phylane_mappings[i].lane) {
			phylane = phylane_mappings[i].phylane;
			break;
		}
	}

	if (i == ARRAY_SIZE(phylane_mappings)) {
		DPU_FB_ERR("[DP] lane number error!\n");
		return -EINVAL;
	}

	switch (phylane) {
	case 0:
		*u161n02_addr = 0x1002;
		*u161n03_addr = 0x1003;
		break;
	case 1:
		*u161n02_addr = 0x1102;
		*u161n03_addr = 0x1103;
		break;
	case 2:
		*u161n02_addr = 0x1202;
		*u161n03_addr = 0x1203;
		break;
	case 3:
		*u161n02_addr = 0x1302;
		*u161n03_addr = 0x1303;
		break;
	default:
		DPU_FB_ERR("[DP] phylane number error!\n");
		return -EINVAL;
	}

	return 0;
}

static int dptx_config_swing_and_preemphasis(struct dp_ctrl *dptx, uint32_t sw_level, uint32_t pe_level,
	uint16_t u161n02_addr, uint16_t u161n03_addr)
{
	uint32_t i;
	struct usb31phy_mapping usb31phy_mappings[] = {
		{0, 0, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[1], dptx->combophy_pree_swing[2]},
		{0, 1, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[3], dptx->combophy_pree_swing[4]},
		{0, 2, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[5], dptx->combophy_pree_swing[6]},
		{0, 3, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[7], dptx->combophy_pree_swing[8]},
		{1, 0, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[9], dptx->combophy_pree_swing[10]},
		{1, 1, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[11], dptx->combophy_pree_swing[12]},
		{1, 2, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[13], dptx->combophy_pree_swing[14]},
		{1, 3, false, -1, -1, -1},
		{2, 0, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[15], dptx->combophy_pree_swing[16]},
		{2, 1, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[17], dptx->combophy_pree_swing[18]},
		{2, 2, false, -1, -1, -1},
		{2, 3, false, -1, -1, -1},
		{3, 0, true, dptx->combophy_pree_swing[0], dptx->combophy_pree_swing[19], dptx->combophy_pree_swing[20]},
		{3, 1, false, -1, -1, -1},
		{3, 2, false, -1, -1, -1},
		{3, 3, false, -1, -1, -1},
	};

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	for (i = 0; i < ARRAY_SIZE(usb31phy_mappings); i++) {
		if (sw_level == usb31phy_mappings[i].sw_level && pe_level == usb31phy_mappings[i].pe_level) {
			if (usb31phy_mappings[i].need_config) {
				usb31phy_cr_write(TX_VBOOST_ADDR, usb31phy_mappings[i].tx_vboot_addr_val);
				usb31phy_cr_write(u161n02_addr, usb31phy_mappings[i].u161n02_addr_val);
				usb31phy_cr_write(u161n03_addr, usb31phy_mappings[i].u161n03_addr_val);
			}
			break;
		}
	}

	if (i == ARRAY_SIZE(usb31phy_mappings)) {
		DPU_FB_ERR("[DP] vswing_level or preemphasis_ level error\n");
		return -EINVAL;
	}

	return 0;
}

void dptx_combophy_set_preemphasis_swing(struct dp_ctrl *dptx,
	uint32_t lane, uint32_t sw_level, uint32_t pe_level)
{
	uint16_t u161n02_addr = 0;
	uint16_t u161n03_addr = 0;
	int ret;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL!\n");

	ret = dptx_mapping_phy_lane(lane, dptx->dptx_plug_type, &u161n02_addr, &u161n03_addr);
	if (ret)
		return;

	ret = dptx_config_swing_and_preemphasis(dptx, sw_level, pe_level, u161n02_addr, u161n03_addr);
	if (ret)
		return;
}

void dptx_combophy_set_ssc_addtions(uint8_t link_rate, bool before)
{
}

void dptx_combophy_set_ssc_dis(struct dp_ctrl *dptx, bool ssc_disable)
{
	uint8_t link_rate;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL!\n");

	link_rate = dptx->link.rate;
#if CONFIG_DP_SETTING_COMBOPHY && defined(CONFIG_HISI_FB_970)
	if (ssc_disable) {
		switch (link_rate) {
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
			DPU_FB_ERR("[DP] Invalid PHY rate %d\n", link_rate);
			break;
		}
	} else {
		usb31phy_cr_write(0x36, 0x25);
		usb31phy_cr_write(0x37, 0x66);
	}
#endif
}


void dptx_aux_disreset(struct dp_ctrl *dptx, bool enable)
{
	uint32_t reg;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dpufd = dptx->dpufd;
	dpu_check_and_no_retval((dpufd == NULL), ERR, "[DP] dpufd is NULL!\n");

	reg = (uint32_t)dptx_readl(dptx, DPTX_WRAP_REG_DPC_AUX_CTRL);/*lint !e838*/

	if (enable)
		reg |= DPTX_DPC_AUX_CTRL_BLOCK;//lint !e737 !e713
	else
		reg &= ~DPTX_DPC_AUX_CTRL_BLOCK;//lint !e737 !e713

	reg &= ~DPTX_TYPEC_AUX_VOD_TUNE_MASK;
	reg |= (3 << DPTX_TYPEC_AUX_VOD_TUNE_SHIFT);

	dptx_writel(dptx, DPTX_WRAP_REG_DPC_AUX_CTRL, reg);

	mdelay(1);//lint !e778 !e747 !e774
}

void dptx_phy_layer_init(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL!\n");

	dptx->dptx_combophy_set_preemphasis_swing = dptx_combophy_set_preemphasis_swing;
	dptx->dptx_change_physetting_hbr2 = NULL;
	dptx->dptx_combophy_set_ssc_addtions = NULL;
	dptx->dptx_combophy_set_lanes_power = dptx_combophy_set_lanes_power;
	dptx->dptx_combophy_set_ssc_dis = dptx_combophy_set_ssc_dis;
	dptx->dptx_aux_disreset = dptx_aux_disreset;
	dptx->dptx_combophy_set_rate = NULL;
}

