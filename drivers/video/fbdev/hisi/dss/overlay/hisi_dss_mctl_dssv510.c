/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_overlay_utils.h"
#include "hisi_mmbuf_manager.h"

void hisi_dss_mctl_sys_init(const char __iomem *mctl_sys_base,
	dss_mctl_sys_t *s_mctl_sys)
{
	int i;

	if (!mctl_sys_base || !s_mctl_sys) {
		DPU_FB_ERR("NULL ptr\n");
		return;
	}

	memset(s_mctl_sys, 0, sizeof(dss_mctl_sys_t));

	for (i = 0; i < DSS_OVL_IDX_MAX; i++) {
		s_mctl_sys->chn_ov_sel[i] =
			inp32(mctl_sys_base + MCTL_RCH_OV0_SEL + i * 0x4);
		if (i != DSS_OVL3)
			s_mctl_sys->chn_ov_sel1[i] =
				inp32(mctl_sys_base + MCTL_RCH_OV0_SEL1 + i * 0x4);
	}

	for (i = 0; i < DSS_WCH_MAX; i++)
		s_mctl_sys->wchn_ov_sel[i] = inp32(mctl_sys_base + MCTL_WCH_OV2_SEL + i * 0x4);
}

void hisi_dss_mctl_ov_set_ctl_dbg_reg(
	struct dpu_fb_data_type *dpufd, char __iomem *mctl_base, bool enable_cmdlist)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is null\n");
		return;
	}

	if (!mctl_base) {
		DPU_FB_ERR("mctl_base is null!\n");
		return;
	}

	if (enable_cmdlist) {
		set_reg(mctl_base + MCTL_CTL_DBG, 0xB03A20, 32, 0);
		set_reg(mctl_base + MCTL_CTL_TOP, 0x1, 32, 0);
	} else {
		set_reg(mctl_base + MCTL_CTL_DBG, 0xB13A00, 32, 0);
		if (dpufd->index == PRIMARY_PANEL_IDX)
			set_reg(mctl_base + MCTL_CTL_TOP, 0x2, 32, 0);
		else if (dpufd->index == EXTERNAL_PANEL_IDX)
			set_reg(mctl_base + MCTL_CTL_TOP, 0x3, 32, 0);
		else
			;
	}
}

void hisi_remove_mctl_mutex(struct dpu_fb_data_type *dpufd,
	int mctl_idx, uint32_t cmdlist_idxs)
{
	struct dss_module_reg *dss_module = NULL;
	char __iomem *chn_mutex_base = NULL;
	char __iomem *cmdlist_base = NULL;
	uint32_t offset;
	uint32_t cmdlist_idxs_temp;
	int i;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	dss_module = &(dpufd->dss_module);
	cmdlist_base = dpufd->dss_base + DSS_CMDLIST_OFFSET;

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (dss_module->mctl_ch_used[i] == 1) {
			chn_mutex_base = dss_module->mctl_ch_base[i].chn_mutex_base +
				g_dss_module_ovl_base[mctl_idx][MODULE_MCTL_BASE];
			if (!chn_mutex_base) {
				DPU_FB_ERR("chn_mutex_base is NULL\n");
				return;
			}

			set_reg(chn_mutex_base, 0, 32, 0);
		}
	}

	set_reg(dss_module->mctl_base[mctl_idx] + MCTL_CTL_MUTEX_OV, 0, 32, 0);

	offset = 0x40;
	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1)
			/* start sel */
			set_reg(cmdlist_base + CMDLIST_CH0_CTRL + i * offset, 0x6, 3, 2);

		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}
}

