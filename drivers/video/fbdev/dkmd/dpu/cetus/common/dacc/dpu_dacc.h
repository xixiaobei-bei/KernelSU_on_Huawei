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
#ifndef DPU_DACC_H
#define DPU_DACC_H

#include <linux/types.h>

// this function call by power on workflow
void dpu_dacc_init(char __iomem *dpu_base);
void dpu_dacc_config_scene(char __iomem *dpu_base, uint32_t scene_id, bool enable_cmdlist);

#endif