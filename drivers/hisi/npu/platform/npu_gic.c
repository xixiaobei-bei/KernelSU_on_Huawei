#include <linux/of.h>
#include "npu_gic.h"
#include "drv_log.h"

#define DEVDRV_AICPU_CLUSTER_NAME  "aicpu_cluster"
#define DEVDRV_AICPU_CORE_NAME     "aicpu_core"
#define DEVDRV_TSCPU_CLUSTER_NAME  "tscpu_cluster"
#define DEVDRV_TSCPU_CORE_NAME     "tscpu_core"
#define DEVDRV_GIC0_SPI_BLK_NAME   "gic0_spi_blk"

int devdrv_plat_parse_gic(
		struct device_node *module_np,
		struct devdrv_platform_info *plat_info)
{
	int ret = 0;

	if ((ret = of_property_read_u32(module_np, DEVDRV_AICPU_CLUSTER_NAME,
		&DEVDRV_PLAT_GET_AICPU_CLUSTER(plat_info))) < 0) {
		return ret;
	}

	if ((ret = of_property_read_u32(module_np, DEVDRV_AICPU_CORE_NAME,
		&DEVDRV_PLAT_GET_AICPU_CORE(plat_info))) < 0) {
		return ret;
	}

	if ((ret = of_property_read_u32(module_np, DEVDRV_TSCPU_CLUSTER_NAME,
		&DEVDRV_PLAT_GET_TSCPU_CLUSTER(plat_info))) < 0) {
		return ret;
	}

	if ((ret = of_property_read_u32(module_np, DEVDRV_TSCPU_CORE_NAME,
		&DEVDRV_PLAT_GET_TSCPU_CORE(plat_info))) < 0) {
		return ret;
	}

	if ((ret = of_property_read_u32(module_np, DEVDRV_GIC0_SPI_BLK_NAME,
		&DEVDRV_PLAT_GET_GIC0_SPI_BLK(plat_info))) < 0) {
		return ret;
	}

	devdrv_drv_debug("aicpu cluster %d core %d,"
					 "tscpu cluster %d core %d,"
					 "gic0 spi blk %d \n",
					 DEVDRV_PLAT_GET_AICPU_CLUSTER(plat_info),
					 DEVDRV_PLAT_GET_AICPU_CORE(plat_info),
					 DEVDRV_PLAT_GET_TSCPU_CLUSTER(plat_info),
					 DEVDRV_PLAT_GET_TSCPU_CORE(plat_info),
					 DEVDRV_PLAT_GET_GIC0_SPI_BLK(plat_info));

	return 0;
}

