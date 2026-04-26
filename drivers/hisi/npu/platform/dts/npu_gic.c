/*
 * npu_gic.c
 *
 * about npu gic
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#define NPU_AICPU_CLUSTER_NAME  "aicpu_cluster"
#define NPU_AICPU_CORE_NAME     "aicpu_core"
#define NPU_TSCPU_CLUSTER_NAME  "tscpu_cluster"
#define NPU_TSCPU_CORE_NAME     "tscpu_core"
#define NPU_GIC0_SPI_BLK_NAME   "gic0_spi_blk"

int npu_plat_parse_gic(const struct device_node *module_np,
	struct npu_platform_info *plat_info)
{
	int ret;

	ret = of_property_read_u32(module_np, NPU_AICPU_CLUSTER_NAME,
		&plat_info->dts_info.aicpu_cluster);
	if (ret < 0)
		return ret;

	ret = of_property_read_u32(module_np, NPU_AICPU_CORE_NAME,
		&plat_info->dts_info.aicpu_core);
	if (ret < 0)
		return ret;

	ret = of_property_read_u32(module_np, NPU_TSCPU_CLUSTER_NAME,
		&plat_info->dts_info.tscpu_cluster);
	if (ret < 0)
		return ret;

	ret = of_property_read_u32(module_np, NPU_TSCPU_CORE_NAME,
		&plat_info->dts_info.tscpu_core);
	if (ret < 0)
		return ret;

	ret = of_property_read_u32(module_np, NPU_GIC0_SPI_BLK_NAME,
		&plat_info->dts_info.gic0_spi_blk_num);
	if (ret < 0)
		return ret;

	npu_drv_debug("aicpu cluster %d core %d, tscpu cluster %d core %d, gic0 spi blk %d\n",
		plat_info->dts_info.aicpu_cluster,
		plat_info->dts_info.aicpu_core,
		plat_info->dts_info.tscpu_cluster,
		plat_info->dts_info.tscpu_core,
		plat_info->dts_info.gic0_spi_blk_num);

	return 0;
}

