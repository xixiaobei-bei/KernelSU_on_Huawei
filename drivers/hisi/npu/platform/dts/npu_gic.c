/*
 * npu_gic.c
 *
 * about npu gic
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
#include "npu_gic.h"

#include <linux/of.h>

#include "npu_log.h"

#define DEVDRV_AICPU_CLUSTER_NAME  "aicpu_cluster"
#define DEVDRV_AICPU_CORE_NAME     "aicpu_core"
#define DEVDRV_TSCPU_CLUSTER_NAME  "tscpu_cluster"
#define DEVDRV_TSCPU_CORE_NAME     "tscpu_core"
#define DEVDRV_GIC0_SPI_BLK_NAME   "gic0_spi_blk"

int devdrv_plat_parse_gic(const struct device_node *module_np, struct devdrv_platform_info *plat_info)
{
	int ret;
	ret = of_property_read_u32(module_np, DEVDRV_AICPU_CLUSTER_NAME, &DEVDRV_PLAT_GET_AICPU_CLUSTER(plat_info));
	if (ret < 0)
		return ret;

	ret = of_property_read_u32(module_np, DEVDRV_AICPU_CORE_NAME, &DEVDRV_PLAT_GET_AICPU_CORE(plat_info));
	if (ret < 0)
		return ret;

	ret = of_property_read_u32(module_np, DEVDRV_TSCPU_CLUSTER_NAME, &DEVDRV_PLAT_GET_TSCPU_CLUSTER(plat_info));
	if (ret < 0)
		return ret;

	ret = of_property_read_u32(module_np, DEVDRV_TSCPU_CORE_NAME, &DEVDRV_PLAT_GET_TSCPU_CORE(plat_info));
	if (ret < 0)
		return ret;

	ret = of_property_read_u32(module_np, DEVDRV_GIC0_SPI_BLK_NAME, &DEVDRV_PLAT_GET_GIC0_SPI_BLK(plat_info));
	if (ret < 0)
		return ret;

	NPU_DRV_DEBUG("aicpu cluster %d core %d," "tscpu cluster %d core %d," "gic0 spi blk %d \n",
		DEVDRV_PLAT_GET_AICPU_CLUSTER(plat_info), DEVDRV_PLAT_GET_AICPU_CORE(plat_info),
		DEVDRV_PLAT_GET_TSCPU_CLUSTER(plat_info), DEVDRV_PLAT_GET_TSCPU_CORE(plat_info),
		DEVDRV_PLAT_GET_GIC0_SPI_BLK(plat_info));

	return 0;
}

