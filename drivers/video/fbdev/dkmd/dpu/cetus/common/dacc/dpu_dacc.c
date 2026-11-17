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
#include "dpu_dacc.h"
#include "dpu_utils.h"
#include <dpu/soc_dpu_define.h>

static inline bool is_offline_scene(uint32_t scene_id)
{
	return (scene_id >= DPU_SCENE_OFFLINE_0 && scene_id <= DPU_SCENE_OFFLINE_2);
}

static inline bool is_online_scene(uint32_t scene_id)
{
	return (scene_id >= DPU_SCENE_ONLINE_0 && scene_id <= DPU_SCENE_ONLINE_3);
}

// this function call by power on workflow
void dpu_dacc_init(char __iomem *dpu_base)
{
	// TODO:
}

void dpu_dacc_config_scene(char __iomem *dpu_base, uint32_t scene_id, bool enable_cmdlist)
{
	char __iomem *dmc_base = dpu_base + DACC_OFFSET + DMC_OFFSET;

	set_reg(SOC_DACC_CLK_SEL_REG_ADDR(dmc_base, scene_id), 1, 1, 0);
	set_reg(SOC_DACC_CLK_EN_REG_ADDR(dmc_base, scene_id), 1, 1, 0);
	set_reg(SOC_DACC_CTL_CFG_MODE0_ADDR(dmc_base, scene_id), (uint32_t)enable_cmdlist, 1, 0);

	if (is_offline_scene(scene_id))
		set_reg(SOC_DACC_CTL_SW_START_REG_ADDR(dmc_base, scene_id), 0x1, 1, 0);
	else
		set_reg(SOC_DACC_CTL_ST_FRM_SEL0_REG_ADDR(dmc_base, scene_id), 0, 1, 0);
}
EXPORT_SYMBOL(dpu_dacc_config_scene);