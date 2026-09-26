/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
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

#include "dpu_cmdlist.h"
#include "dpu_utils.h"

// TODO: waiting for cmdlist
#define dkmd_cmdlist_get_dma_addr(scene_id, value) 0

static struct cmdlist_dm_addr_info dm_tlb_info[] = {
	/* ------- scene_id -------------- DM_addr --------- DM_size ------ */
	{DPU_SCENE_ONLINE_0,  DM_INPUTDATA_ST_ADDR0,  0x1000, }, /* 4K */
	{DPU_SCENE_ONLINE_1,  DM_INPUTDATA_ST_ADDR1,  0x1000, }, /* 4K */
	{DPU_SCENE_ONLINE_2,  DM_INPUTDATA_ST_ADDR2,  0xA00,  }, /* 2.5K */
	{DPU_SCENE_ONLINE_3,  DM_INPUTDATA_ST_ADDR3,  0xA00,  }, /* 2.5K */
	{DPU_SCENE_OFFLINE_0, DM_INPUTDATA_ST_ADDR4,  0xA00,  }, /* 2.5K */
	{DPU_SCENE_OFFLINE_1, DM_INPUTDATA_ST_ADDR5,  0xA00,  }, /* 2.5K */
	{DPU_SCENE_OFFLINE_2, DM_INPUTDATA_ST_ADDR6,  0xA00,  }, /* 2.5K */
	{DPU_SCENE_INITAIL,  DM_DPP_INITAIL_ST_ADDR, 0x800,  }, /* 2K */
	{DPU_SCENE_SECURITY, DM_SECURITY_ST_ADDR,    0x400,  }, /* 1K */
};

void dpu_cmdlist_config_on(char __iomem *dpu_base)
{
	int i;

	// lowpower clk config
	outp32(DPU_CMD_CLK_SEL_ADDR(dpu_base + DPU_CMDLIST_OFFSET), 0xFFFFFFFF);
	outp32(DPU_CMD_CLK_EN_ADDR(dpu_base + DPU_CMDLIST_OFFSET), 0xFFFFFFFF);

	// init
	for (i = DPU_SCENE_ONLINE_0; i < DPU_SCENE_MAX; i++) {
		outp32(DPU_CMD_CMDLIST_CH_INTE_ADDR(dpu_base + DPU_CMDLIST_OFFSET, i), 0xFFFFFFFF);
		outp32(DPU_CMD_CMDLIST_CH_INTC_ADDR(dpu_base + DPU_CMDLIST_OFFSET, i), 0xFFFFFFFF);

		// ch debug
		outp32(DPU_CMD_CMDLIST_CH_DBG_ADDR(dpu_base + DPU_CMDLIST_OFFSET, i), 0);
	}

	set_reg(DPU_CMD_TWO_SAME_FRAME_BYPASS_ADDR(dpu_base + DPU_CMDLIST_OFFSET), 0x1, 1, 0);
}

void dpu_cmdlist_commit(char __iomem *dpu_base, uint32_t scene_id, int32_t cmdlist_id)
{
	dma_addr_t cmdlist_buf_addr;

	dsb(sy);

	set_reg(DPU_CMD_CMDLIST_CH_CTRL_ADDR(dpu_base + DPU_CMDLIST_OFFSET, scene_id), 1, 1, 0);

	// TODO: cmdlist interface
	cmdlist_buf_addr = dkmd_cmdlist_get_dma_addr(scene_id, cmdlist_id);
	outp32(DPU_DM_CMDLIST_ADDR0_ADDR(dpu_base + DACC_OFFSET + dm_tlb_info[scene_id].dm_data_addr), cmdlist_buf_addr);

	dpu_pr_info("cmdlist_buf_addr = 0x%x", cmdlist_buf_addr);
}
EXPORT_SYMBOL(dpu_cmdlist_commit);