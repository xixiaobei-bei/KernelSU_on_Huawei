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

int npu_plat_parse_feature_switch(const struct device_node *module_np,
	struct npu_platform_info *plat_info)
{
	int ret;
	int i;

	ret = of_property_read_u32_array(module_np, "feature",
		(u32 *)(plat_info->dts_info.feature_switch),
		NPU_FEATURE_MAX_RESOURCE);
	if (ret != 0) {
		npu_drv_err("read feature from dts failed\n");
		return -1;
	}

	for (i = 0; i < NPU_FEATURE_MAX_RESOURCE; i++)
		npu_drv_debug("feature %d switch is %d\n",
			i, plat_info->dts_info.feature_switch[i]);

	return 0;
}

void npu_plat_switch_on_feature(void)
{
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info failed\n");
		return;
	}
	plat_info->dts_info.feature_switch[NPU_FEATURE_AUTO_POWER_DOWN] = 1;
}

void npu_plat_switch_off_feature(void)
{
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info failed\n");
		return;
	}
	plat_info->dts_info.feature_switch[NPU_FEATURE_AUTO_POWER_DOWN] = 0;
}
