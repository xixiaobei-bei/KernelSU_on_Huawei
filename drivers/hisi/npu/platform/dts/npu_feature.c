/*
 * npu_feature.c
 *
 * about npu feature
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
#include "npu_feature.h"

#include <linux/of.h>

#include "npu_log.h"

int devdrv_plat_parse_feature_switch(const struct device_node *module_np, struct devdrv_platform_info *plat_info)
{
	int ret;
	int i;

	ret = of_property_read_u32_array(module_np, "feature",
		(u32 *)(plat_info->dts_info.feature_switch), DEVDRV_FEATURE_MAX_RESOURCE);
	if (ret != 0) {
		NPU_DRV_ERR("read feature from dts failed\n");
		return -1;
	}

	for (i = 0; i < DEVDRV_FEATURE_MAX_RESOURCE; i++)
		NPU_DRV_DEBUG("feature %d switch is %d\n", i, DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, i));

	return 0;
}

void devdrv_plat_switch_on_feature(void)
{
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_AUTO_POWER_DOWN) = 1;
}
void devdrv_plat_switch_off_feature(void)
{
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_AUTO_POWER_DOWN) = 0;
}

