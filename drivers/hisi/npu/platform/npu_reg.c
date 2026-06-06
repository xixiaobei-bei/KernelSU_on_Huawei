#include <linux/io.h>

#include "npu_reg.h"
#include "npu_adapter.h"
#include "drv_log.h"

struct devdrv_mem_desc* devdrv_plat_get_reg_desc(u32 reg_idx)
{
	struct devdrv_platform_info *plat_info = NULL;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL)
	{
		devdrv_drv_err("get plat_info failed.\n");
		return NULL;
	}

	return &DEVDRV_PLAT_GET_REG_DESC(plat_info, reg_idx);
}
EXPORT_SYMBOL(devdrv_plat_get_reg_desc);

u32* devdrv_plat_get_reg_vaddr(u32 reg_idx, u32 offset)
{
	struct devdrv_platform_info *plat_info = NULL;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL)
	{
		devdrv_drv_err("get plat_info failed.\n");
		return NULL;
	}
	return (u32 *)((u8*)DEVDRV_PLAT_GET_REG_VADDR(plat_info, reg_idx) + offset);
}
EXPORT_SYMBOL(devdrv_plat_get_reg_vaddr);

int devdrv_plat_unmap_reg(
		struct platform_device *pdev,
		struct devdrv_platform_info *plat_info)
{
	int i;

	for (i = 0; i < DEVDRV_REG_MAX_RESOURCE; i++) {
		if (DEVDRV_PLAT_GET_REG_VADDR(plat_info, i) != NULL) {
			if (i == DEVDRV_REG_TS_SRAM) {
				devdrv_plat_sram_unmap(pdev,
					DEVDRV_PLAT_GET_REG_VADDR(plat_info, i));
			}
			else {
				devm_iounmap(&pdev->dev,
					DEVDRV_PLAT_GET_REG_VADDR(plat_info, i));
			}
		}
	}
	return 0;
}
EXPORT_SYMBOL(devdrv_plat_unmap_reg);

int devdrv_plat_parse_reg_desc(
		struct platform_device *pdev,
		struct devdrv_platform_info *plat_info)
{
	int i;
	void __iomem *base = NULL;
	struct resource *info = NULL;
	struct devdrv_mem_desc* mem_desc = NULL;

	for (i = 0; i < DEVDRV_REG_MAX_RESOURCE; i++) {
		info = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (info == NULL) {
			devdrv_drv_err("platform_get_resource failed i = %d.\n",i);
			return -1;
		}
		mem_desc = &DEVDRV_PLAT_GET_REG_DESC(plat_info, i);
		mem_desc->base = info->start;
		mem_desc->len = info->end - info->start;
		devdrv_drv_debug("resource: base %pK len %x\n",
			(void *)(uintptr_t)(u64)mem_desc->base, mem_desc->len);
		if (i == DEVDRV_REG_TS_SRAM) {
			base = devdrv_plat_sram_remap(pdev, mem_desc->base, mem_desc->len);
		}
		else {
			base = devm_ioremap(&pdev->dev, mem_desc->base, mem_desc->len);
		}
		if (base == NULL) {
			devdrv_drv_err("platform_get_resource failed i = %d.\n",i);
			goto map_platform_reg_fail;
		}
		DEVDRV_PLAT_GET_REG_VADDR(plat_info, i) = base;
	}

	return 0;
map_platform_reg_fail:
	memset(plat_info->dts_info.reg_desc, 0, sizeof(plat_info->dts_info.reg_desc));
	return devdrv_plat_unmap_reg(pdev, plat_info);
}
