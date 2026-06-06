#ifndef __NPU_IRQ_H
#define __NPU_IRQ_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int devdrv_plat_parse_irq(
		struct platform_device *pdev,
		struct devdrv_platform_info *plat_info);

#endif
