#ifndef __NPU_FEATURE_H
#define __NPU_FEATURE_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int devdrv_plat_parse_feature_switch(
		struct device_node *module_np,
		struct devdrv_platform_info *plat_info);

#endif
