#include <linux/of.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include "npu_dfx.h"
#include "npu_resmem.h"
#include "drv_log.h"
#include "npu_platform.h"
#include "npu_platform_register.h"

#define DEVDRV_DFX_CHANNEL_NAME "channel"
#define DEVDRV_DFX_RESMEM_NAME  "buf_idx"

int devdrv_plat_parse_dfx_desc(struct device_node *module_np,
		struct devdrv_platform_info *plat_info,
		struct devdrv_dfx_desc *dfx_desc)
{
	int ret = 0;
	u8 channel_num = 0;

	channel_num = of_property_count_u32_elems(module_np, DEVDRV_DFX_CHANNEL_NAME);
	if (channel_num > DEVDRV_DFX_CHANNEL_MAX_RESOURCE) {
		devdrv_drv_err("channel_num = %d, out of range.\n", channel_num);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(module_np, DEVDRV_DFX_CHANNEL_NAME,
		(u32 *)(dfx_desc->channels), channel_num);
	if (ret < 0) {
		devdrv_drv_err("parse channels failed.\n");
		return -EINVAL;
	}

	dfx_desc->channel_num = channel_num;

	ret = devdrv_plat_find_resmem_idx(module_np, plat_info, DEVDRV_DFX_RESMEM_NAME,
		&dfx_desc->bufs);
	if (ret < 0) {
		devdrv_drv_err("find resmem idx failed.\n");
		return -EINVAL;
	}

	return 0;
}


