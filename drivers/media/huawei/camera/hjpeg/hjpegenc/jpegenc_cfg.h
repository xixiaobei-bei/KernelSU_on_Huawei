/*
 * jpegenc_cfg.h
 *
 * provide interface for config jpeg qtable/hufftable etc.
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * lixiuhua <aimee.li@hisilicon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _INCLUDE_JPEGENC_CFG_H
#define _INCLUDE_JPEGENC_CFG_H

#include <linux/iommu.h>
#include <asm/io.h>

extern void hjpeg_init_hw_param(void __iomem*  base_addr, uint32_t power_controller, bool smmu_bypass);
#endif  // _INCLUDE_HJPEG_COMMON_H
