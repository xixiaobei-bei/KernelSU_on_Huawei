/*
 * npu_reg.c
 *
 * about npu reg
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "npu_reg.h"

#include <linux/io.h>
#include <securec.h>
#include "npu_adapter.h"
#include "npu_log.h"

#define NPU_DUMP_REGION_NAME     "dump_region"

static npu_reg_info npu_regs[NPU_REG_MAX_REG] = {
	[NPU_REG_POWER_STATUS] = {"NPU_REG_POWER_STATUS",
		DRV_NPU_POWER_STATUS_REG, NPU_POWER_STATUS_REG_LEN, NULL},
};

u32 *npu_plat_get_reg_vaddr_offset(u32 reg_idx, u32 offset)
{
	struct npu_platform_info *plat_info = NULL;

	if (reg_idx >= NPU_REG_MAX_RESOURCE) {
		npu_drv_err("reg_idx = %x is error\n", reg_idx);
		return NULL;
	}

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_info failed\n");
		return NULL;
	}
	return (u32 *)((u8 *)plat_info->dts_info.reg_vaddr[reg_idx] + offset);
}
EXPORT_SYMBOL(npu_plat_get_reg_vaddr_offset);

int npu_plat_map_reg(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
	int i;
	void __iomem *base = NULL;
	struct npu_mem_desc *mem_desc = NULL;
	int max_regs = NPU_REG_MAX_RESOURCE;

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 0)
		max_regs = NPU_REG_PERICRG_BASE + 1;
	for (i = 0; i < max_regs; i++) {
		mem_desc = &plat_info->dts_info.reg_desc[i];
		npu_drv_debug("resource: base %pK\n",
			(void *)(uintptr_t)(u64)mem_desc->base);
		npu_drv_debug("resource: mem_desc.len %x\n", mem_desc->len);

		if (mem_desc->base != 0 && mem_desc->len != 0) {
			if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
				base = devm_ioremap(&pdev->dev, mem_desc->base, mem_desc->len);
			} else {
				if (i == NPU_REG_TS_SRAM) {
					base = npu_plat_sram_remap(pdev, mem_desc->base,
						mem_desc->len);
				} else {
					base = devm_ioremap(&pdev->dev, mem_desc->base,
						mem_desc->len);
				}
			}

			if (base == NULL) {
				npu_drv_err("platform_get_resource failed i = %d\n", i);
				goto map_platform_reg_fail;
			}
			plat_info->dts_info.reg_vaddr[i] = base;
		} else {
			plat_info->dts_info.reg_vaddr[i] = NULL;
		}
	}

	return 0;
map_platform_reg_fail:
	(void)memset_s(plat_info->dts_info.reg_desc,
		sizeof(plat_info->dts_info.reg_desc),
		0, sizeof(plat_info->dts_info.reg_desc));
	(void)npu_plat_unmap_reg(pdev, plat_info);
	return -1;
}
EXPORT_SYMBOL(npu_plat_map_reg);


int npu_plat_unmap_reg(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
	int i;
	int max_regs = NPU_REG_MAX_RESOURCE;

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 0)
		max_regs = NPU_REG_PERICRG_BASE + 1;

	for (i = 0; i < max_regs; i++) {
		if (plat_info->dts_info.reg_vaddr[i] != NULL) {
			if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 1) {
				devm_iounmap(&pdev->dev, plat_info->dts_info.reg_vaddr[i]);
			} else {
				if (i == NPU_REG_TS_SRAM)
					npu_plat_sram_unmap(pdev, plat_info->dts_info.reg_vaddr[i]);
				else
					devm_iounmap(&pdev->dev, plat_info->dts_info.reg_vaddr[i]);
			}
		}
	}
	return 0;
}
EXPORT_SYMBOL(npu_plat_unmap_reg);

int npu_plat_parse_dump_region_desc(const struct device_node *root,
	struct npu_platform_info *plat_info)
{
	int ret;
	struct npu_mem_desc *mem_desc = &plat_info->dts_info.dump_region_desc[0];

	if (mem_desc == NULL) {
		npu_drv_err("dump_region desc is NULL");
		return -1;
	}
	ret = of_property_read_u32_array(root, NPU_DUMP_REGION_NAME,
		(u32 *)mem_desc,
		NPU_DUMP_REGION_MAX * (sizeof(struct npu_mem_desc) / sizeof(u32)));
	if (ret < 0) {
		npu_drv_err("read dump_regs base failed\n");
		return ret;
	}
	npu_drv_debug("dump region: base=0x%x len=0x%x", mem_desc->base,
		mem_desc->len);
	return 0;
}

int npu_plat_parse_reg_desc(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
	int i;
	struct resource *info = NULL;
	struct npu_mem_desc *mem_desc = NULL;
	int max_regs = NPU_REG_MAX_RESOURCE;

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 0)
		max_regs = NPU_REG_PERICRG_BASE + 1;
	for (i = 0; i < max_regs; i++) {
		info = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (info == NULL || info->end <= info->start) {
			npu_drv_err("info is invalid, i = %d\n", i);
			return -1;
		}
		mem_desc = &plat_info->dts_info.reg_desc[i];
		mem_desc->base = info->start;
		mem_desc->len = info->end - info->start;
	}

	return 0;
}

int npu_plat_ioremap(npu_reg_type reg_type)
{
	npu_reg_info *reg_info = NULL;

	if (reg_type >= NPU_REG_MAX_REG) {
		npu_drv_err("invalid reg_type = %d\n", reg_type);
		return -1;
	}

	reg_info = &npu_regs[reg_type];
	if (reg_info->base == 0 || reg_info->size == 0) {
		npu_drv_err("reg_type = %d is unused\n", reg_type);
		return -1;
	}
	if (reg_info->vaddr)
		return 0;

/*lint -e446 */
	reg_info->vaddr = ioremap_nocache(reg_info->base, reg_info->size);
/*lint +e446 */
	if (reg_info->vaddr == NULL) {
		npu_drv_err("ioremap failed\n");
		return -1;
	}
	return 0;
}

void npu_plat_iounmap(npu_reg_type reg_type)
{
	npu_reg_info *reg_info = NULL;

	if (reg_type >= NPU_REG_MAX_REG) {
		npu_drv_err("invalid reg_type = %d\n", reg_type);
		return;
	}

	reg_info = &npu_regs[reg_type];
	if (reg_info->vaddr) {
		iounmap(reg_info->vaddr);
		reg_info->vaddr = NULL;
	}
}

void __iomem *npu_plat_get_vaddr(npu_reg_type reg_type)
{
	if (reg_type >= NPU_REG_MAX_REG) {
		npu_drv_err("invalid reg_type = %d\n", reg_type);
		return NULL;
	}
	return npu_regs[reg_type].vaddr;
}

void npu_plat_set_npu_power_status(u32 status)
{
	u32 *addr = (u32 *)npu_plat_get_vaddr(NPU_REG_POWER_STATUS);

	if (addr)
		writel(status, addr);
	else
		npu_drv_warn("invalid power status addr\n");
}
