/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: provide interface for config jpeg qtable/hufftable etc.
 * Author: lixiuhua
 * Create: 2012-12-22
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JPEGENC_CFG_H__
#define __JPEGENC_CFG_H__

#include <linux/iommu.h>
#include <asm/io.h>

extern void hjpeg_init_hw_param(void __iomem *base_addr,
	uint32_t power_controller, bool smmu_bypass);

#endif /* __JPEGENC_CFG_H__ */