#ifndef __NPU_DFX_H
#define __NPU_DFX_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int devdrv_plat_parse_dfx_desc(
		struct device_node *module_np,
		struct devdrv_platform_info *plat_info,
		struct devdrv_dfx_desc *dfx_desc);

int devdrv_wait_tscpu_ready_status(void);
#endif
