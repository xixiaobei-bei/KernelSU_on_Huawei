#ifndef __NPU_REG_H
#define __NPU_REG_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int devdrv_plat_unmap_reg(
		struct platform_device *pdev,
		struct devdrv_platform_info *plat_info);

int devdrv_plat_parse_reg_desc(
		struct platform_device *pdev,
		struct devdrv_platform_info *plat_info);

#endif
