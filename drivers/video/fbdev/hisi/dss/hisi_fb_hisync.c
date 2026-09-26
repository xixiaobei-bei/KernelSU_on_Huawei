/* Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"

#include "hisi_fb_hisync.h"

void dpufb_hisync_disp_sync_enable(struct dpu_fb_data_type *dpufd)
{
	char __iomem *cmdlist_base = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is null ptr\n");
		return;
	}

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return;
	DPU_FB_INFO("enable hisync feature!\n");

	cmdlist_base = dpufd->dss_base + DSS_CMDLIST_OFFSET;
	set_reg(cmdlist_base + CMD_SYNC_MODE, 0x1, 1, 0);
}

int dpufb_hisync_disp_sync_config(struct dpu_fb_data_type *dpufd)
{
	uint32_t try_times = 0;
	char __iomem *cmdlist_base = NULL;
	uint32_t cycle = 0;
	char __iomem *ldi_base = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is null ptr\n");
		return -1;
	}

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return -1;

	cmdlist_base = dpufd->dss_base + DSS_CMDLIST_OFFSET;
	while ((uint32_t)inp32(cmdlist_base + CMD_CFG_FLAG) & BIT(0)) {
		udelay(1);
		if (++try_times > 10000) {  /* 10ms */
			DPU_FB_ERR("Read cmd_cfg_flag timeout!\n");
			return -1;
		}
	}
	DPU_FB_INFO("wait hisync disp sync succss!\n");

	set_reg(cmdlist_base + CMD_CFG_FLAG, 0x1, 1, 0);

	ldi_base = dpufd->mipi_dsi0_base;
	if (dpufd->panel_info.vsync_delay_time > 0) {
		cycle = dpufd->panel_info.vsync_delay_time * dpufd->panel_info.pxl_clk_rate / 1000;  /* ms to s */
		set_reg(ldi_base + MIPI_VSYNC_DELAY_TIME, cycle - 1, 32, 0);
		set_reg(ldi_base + MIPI_VSYNC_DELAY_CTRL, 0x2, 2, 1);
	} else {
		set_reg(ldi_base + MIPI_VSYNC_DELAY_CTRL, 0x1, 2, 1);
	}
	return 0;
}

#pragma GCC diagnostic pop

