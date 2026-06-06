#ifndef __NPU_GIC_H
#define __NPU_GIC_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int devdrv_plat_parse_gic(
		struct device_node *module_np,
		struct devdrv_platform_info *plat_info);

#endif
