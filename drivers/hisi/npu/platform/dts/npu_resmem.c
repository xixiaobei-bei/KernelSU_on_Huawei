/*
 * npu_resmem.c
 *
 * about npu resmem
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
#include "npu_resmem.h"

#include <linux/of.h>

#include "npu_log.h"


int devdrv_plat_find_resmem_idx(const struct device_node *module_np, struct devdrv_platform_info *plat_info,
                                const char* tag, struct devdrv_mem_desc **result)
{
	int ret;
	u32 index = 0;
	struct devdrv_mem_desc *desc = NULL;
	ret = of_property_read_u32(module_np, tag, &index);
	if (ret < 0) {
		NPU_DRV_ERR("read %s failed\n", tag);
		return ret;
	}

	NPU_DRV_DEBUG("tag %s index %d\n", tag, index);
	if (index <= 0 || index > DEVDRV_RESMEM_MAX_RESOURCE) {
		NPU_DRV_ERR("index %d out of range\n", index);
		return -EINVAL;
	}

	desc = &DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, index - 1);
	if ((desc->base == 0) || (desc->len == 0)) {
		NPU_DRV_ERR("found resmem desc %d NULL: base=%x, len=%x\n", index, desc->base, desc->len);
		return -EINVAL;
	}

	NPU_DRV_DEBUG("found resmem desc %d base %x len %x\n", index, desc->base, desc->len);

	*result = desc;
	return 0;
}

int devdrv_plat_check_resmem_overlap(const struct devdrv_platform_info *plat_info,
                                     int index, u32 base, u32 len, bool is_sec)
{
	int i;
	u32 comp_base;
	u32 comp_len;

	for (i = 0; i < index; i++) {
		if (is_sec) {
			comp_base = DEVDRV_PLAT_GET_RESMEM_S_DESC(plat_info, i).base;
			comp_len = DEVDRV_PLAT_GET_RESMEM_S_DESC(plat_info, i).len;
		} else {
			comp_base = DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, i).base;
			comp_len = DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, i).len;
		}
		if ((comp_base == 0) || (comp_len == 0)) {
			NPU_DRV_DEBUG("from resmem desc %d NULL: base=%x, len=%x\n", i,
				comp_base, comp_len);
			return 0;
		}
		if (((comp_base + comp_len) > base) || ((base + len) < comp_base)) {
			NPU_DRV_ERR("overlap with resmem desc %d: base=%x, len=%x"
				"comp_base=%x, comp_len=%x\n", i, base, len, comp_base, comp_len);
			return -EINVAL;
		}
	}
	return 0;
}

int devdrv_plat_parse_resmem_desc(const struct device_node *root, struct devdrv_platform_info *plat_info)
{
	int desc_count, index, ret;
	u32 base, len;
	ret = of_property_read_u32(root, DEVDRV_RESMEM_BASE_NAME, &base);
	if (ret < 0) {
		NPU_DRV_ERR("read resmem base failed\n");
		return ret;
	}

	desc_count = of_property_count_u32_elems(root, DEVDRV_RESMEM_LEN_NAME);
	if ((desc_count / 2 > DEVDRV_RESMEM_MAX_RESOURCE) || (desc_count <= 0)) {
		NPU_DRV_ERR("desc_count = %d, out of range\n", desc_count);
		return -EINVAL;
	}

	for (index = 0; index < desc_count; index++) {
		ret = of_property_read_u32_index(root, DEVDRV_RESMEM_LEN_NAME, index, &len);
		if (ret < 0) {
			NPU_DRV_ERR("read resmem %d's len failed\n", index);
			return ret;
		}
		ret  = devdrv_plat_check_resmem_overlap(plat_info, index, base, len, false);
		if (ret < 0) {
			NPU_DRV_ERR("resmem %d overlaps \n", index);
			return ret;
		}
		DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, index).base = base;
		DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, index).len = len;
		NPU_DRV_DEBUG("resmem %d: base %x len %x\n", index, base, len);
		base += len;
	}
	return 0;
}

int devdrv_plat_parse_resmem_s_desc(const struct device_node *root, struct devdrv_platform_info *plat_info)
{
	int desc_count;
	int index;
	int ret;
	u32 base;
	u32 len;
	ret = of_property_read_u32(root, DEVDRV_RESMEM_S_BASE_NAME, &base);
	if (ret < 0) {
		NPU_DRV_ERR("read resmem base failed\n");
		return ret;
	}

	desc_count = of_property_count_u32_elems(root, DEVDRV_RESMEM_S_LEN_NAME);
	for (index = 0; index < desc_count; index++) {
		ret = of_property_read_u32_index(root, DEVDRV_RESMEM_S_LEN_NAME, index, &len);
		if (ret < 0) {
			NPU_DRV_ERR("read resmem_s %d's len failed\n", index);
			return ret;
		}
		ret  = devdrv_plat_check_resmem_overlap(plat_info, index, base, len, true);
		if (ret < 0) {
			NPU_DRV_ERR("resmem_s %d overlaps \n", index);
			return ret;
		}
		DEVDRV_PLAT_GET_RESMEM_S_DESC(plat_info, index).base = base;
		DEVDRV_PLAT_GET_RESMEM_S_DESC(plat_info, index).len = len;
		NPU_DRV_DEBUG("resmem_s %d: base %x len %x \n", index, base, len);
		base += len;
	}
	return 0;
}

int devdrv_plat_parse_resmem_usage(struct device_node *module_np, struct devdrv_platform_info *plat_info)
{
	int ret;
	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1) {
		if (plat_info->dts_info.feature_switch[DEVDRV_FEATURE_KERNEL_LOAD_IMG]) {
			ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
				DEVDRV_RESMEM_TSFW_NAME, &DEVDRV_PLAT_GET_TSFW_BUF(plat_info));
			if (ret != 0)
				return ret;
		} else {
			DEVDRV_PLAT_GET_TSFW_BUF(plat_info) = &DEVDRV_PLAT_GET_RESMEM_S_DESC(plat_info, TSCPU_EL3_S);
		}

		ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
			DEVDRV_RESMEM_INFO_NAME, &DEVDRV_PLAT_GET_INFO_BUF(plat_info));
		if (ret != 0)
			return ret;

		ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
			DEVDRV_RESMEM_SQCQ_NAME, &DEVDRV_PLAT_GET_SQCQ_BUF(plat_info));
		if (ret != 0)
			return ret;

		ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
			DEVDRV_RESMEM_HWTS_SQ_NAME, &DEVDRV_PLAT_GET_HWTS_SQ_BUF(plat_info));
		if (ret != 0)
			return ret;

		ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
			DEVDRV_RESMEM_HWTS_CQ_NAME, &DEVDRV_PLAT_GET_HWTS_CQ_BUF(plat_info));
		if (ret != 0)
			return ret;
	} else {
		if (plat_info->dts_info.feature_switch[DEVDRV_FEATURE_KERNEL_LOAD_IMG]) {
			ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
				DEVDRV_RESMEM_TSFW_NAME, &DEVDRV_PLAT_GET_TSFW_BUF(plat_info));
			if (ret != 0)
				return ret;

			ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
				DEVDRV_RESMEM_AIFW_NAME, &DEVDRV_PLAT_GET_AIFW_BUF(plat_info));
			if (ret != 0)
				return ret;
		} else {
			DEVDRV_PLAT_GET_TSFW_BUF(plat_info) = &DEVDRV_PLAT_GET_RESMEM_S_DESC(plat_info, TSCPU_EL3_S);
			DEVDRV_PLAT_GET_AIFW_BUF(plat_info) = &DEVDRV_PLAT_GET_RESMEM_S_DESC(plat_info, AICPU_EL3_S);
		}

		ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
			DEVDRV_RESMEM_SQCQ_NAME, &DEVDRV_PLAT_GET_SQCQ_BUF(plat_info));
		if (ret != 0)
			return ret;

		ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
			DEVDRV_RESMEM_PERSISTENT_TAKS_BUF_NAME,
			&DEVDRV_PLAT_GET_PERSISTENT_TASK_BUF(plat_info));
		if (ret != 0)
			return ret;

		ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
			DEVDRV_RESMEM_CHIP_CFG_NAME, &DEVDRV_PLAT_GET_CHIP_CFG_BUF(plat_info));
	}

	return 0;
}

