#include <linux/of.h>
#include "npu_feature.h"
#include "drv_log.h"

int devdrv_plat_parse_feature_switch(
		struct device_node *module_np,
		struct devdrv_platform_info *plat_info)
{
	int ret;
	int i;

	ret = of_property_read_u32_array(module_np, "feature",
		(u32 *)(plat_info->dts_info.feature_switch), DEVDRV_FEATURE_MAX_RESOURCE);
	if (ret != 0)
	{
		devdrv_drv_err("read feature from dts failed.\n");
		return -1;
	}

	for (i = 0; i < DEVDRV_FEATURE_MAX_RESOURCE; i++)
	{
		devdrv_drv_debug("feature %d switch is %d.\n", i,
			DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, i));
	}

	return 0;
}

void devdrv_plat_switch_on_feature(void)
{
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_AUTO_POWER_DOWN) = 1;

}
void devdrv_plat_switch_off_feature(void)
{
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_AUTO_POWER_DOWN) = 0;

}


