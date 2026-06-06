#ifndef __NPU_RESMEM_H
#define __NPU_RESMEM_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int devdrv_plat_parse_resmem_desc(
		struct device_node *root,
		struct devdrv_platform_info *plat_info);
int devdrv_plat_parse_resmem_usage(
		struct device_node *module_np,
		struct devdrv_platform_info *plat_info);
int devdrv_plat_find_resmem_idx(struct device_node *module_np,
		struct devdrv_platform_info *plat_info, const char* tag,
		struct devdrv_mem_desc **result);

#endif
