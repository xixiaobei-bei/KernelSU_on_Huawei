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
#ifndef DPU_CMDLIST_H
#define DPU_CMDLIST_H

#include <linux/types.h>
#include <dpu/soc_dpu_define.h>

struct cmdlist_dm_addr_info {
	enum SCENE_ID scene_id;
	u_int32_t dm_data_addr;
	u_int32_t dm_data_size;
};

void dpu_cmdlist_config_on(char __iomem *dpu_base);
void dpu_cmdlist_commit(char __iomem *dpu_base, uint32_t scene_id, int32_t cmdlist_id);


#endif