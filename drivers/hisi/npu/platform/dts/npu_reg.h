/*
 * npu_reg.h
 *
 * about npu reg
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __NPU_REG_H
#define __NPU_REG_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int devdrv_plat_unmap_reg(struct platform_device *pdev, struct devdrv_platform_info *plat_info);

int devdrv_plat_parse_reg_desc(struct platform_device *pdev, struct devdrv_platform_info *plat_info);

int devdrv_plat_parse_dump_region_desc(const struct device_node *root, struct devdrv_platform_info *plat_info);

#endif
