#include <linux/of.h>
#include "npu_resmem.h"
#include "drv_log.h"

#define DEVDRV_RESMEM_LEN_NAME  "reserved_memory_len"
#define DEVDRV_RESMEM_BASE_NAME "reserved_memory_base"
#define DEVDRV_RESMEM_TSFW_NAME "ts_fw_buf_idx"
#define DEVDRV_RESMEM_AIFW_NAME "ai_fw_buf_idx"
#define DEVDRV_RESMEM_SQCQ_NAME "sqcq_buf_idx"
#define DEVDRV_RESMEM_PERSISTENT_TAKS_BUF_NAME "persistent_task_buf_idx"

int devdrv_plat_find_resmem_idx(struct device_node *module_np,
		struct devdrv_platform_info *plat_info, const char* tag,
		struct devdrv_mem_desc **result)
{
	int ret = 0;
	u32 index = 0;
	struct devdrv_mem_desc *desc = NULL;

	if ((ret = of_property_read_u32(module_np, tag, &index)) < 0) {
		devdrv_drv_err("read %s failed.\n", tag);
		return ret;
	}

	devdrv_drv_debug("tag %s index %d.\n", tag, index);

	if (index <= 0 || index > DEVDRV_RESMEM_MAX_RESOURCE) {
		devdrv_drv_err("index %d out of range.\n", index);
		return -EINVAL;
	}

	desc = &DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, index-1);
	if ((desc->base == 0) || (desc->len == 0)) {
		devdrv_drv_err("found resmem desc %d NULL: base=%x, len=%x.\n",
			index, desc->base, desc->len);
		return -EINVAL;
	}

	devdrv_drv_debug("found resmem desc %d base %x len %x.\n",
		index, desc->base, desc->len);

	*result = desc;
	return 0;
}

int devdrv_plat_check_resmem_overlap(
	struct devdrv_platform_info *plat_info, int index, u32 base, u32 len)
{
	int i;
	u32 comp_base, comp_len;

	for (i = 0; i < index; i++) {
		comp_base = DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, i).base;
		comp_len = DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, i).len;
		if ((comp_base == 0) || (comp_len == 0)) {
			devdrv_drv_debug("from resmem desc %d NULL: base=%x, len=%x.\n", i,
				comp_base, comp_len);
			return 0;
		}
		if (((comp_base + comp_len) > base) || ((base + len) < comp_base)) {
			devdrv_drv_err("overlap with resmem desc %d: base=%x, len=%x"
				"comp_base=%x, comp_len=%x.\n", i, base, len,
				comp_base, comp_len);
			return -EINVAL;
		}
	}
	return 0;
}

int devdrv_plat_parse_resmem_desc(
		struct device_node *root,
		struct devdrv_platform_info *plat_info)
{
	int desc_count, index, ret;
	u32 base, len;

	if ((ret = of_property_read_u32(root, DEVDRV_RESMEM_BASE_NAME, &base)) < 0) {
		devdrv_drv_err("read resmem base failed.\n");
		return ret;
	}

	desc_count = of_property_count_u32_elems(root, DEVDRV_RESMEM_LEN_NAME);
	if ((desc_count / 2 > DEVDRV_RESMEM_MAX_RESOURCE) ||
		(desc_count <= 0))
	{
		devdrv_drv_err("desc_count = %d, out of range.\n", desc_count);
		return -EINVAL;
	}

	for (index = 0; index < desc_count; index++) {
		if ((ret = of_property_read_u32_index(root, DEVDRV_RESMEM_LEN_NAME,
				index, &len)) < 0) {
			devdrv_drv_err("read resmem %d's len failed.\n", index);
			return ret;
		}
		ret  = devdrv_plat_check_resmem_overlap(plat_info, index, base, len);
		if (ret < 0) {
			devdrv_drv_err("resmem %d overlaps .\n", index);
			return ret;
		}
		DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, index).base = base;
		DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, index).len = len;
		devdrv_drv_debug("resmem %d: base %x len %x.\n", index, base, len);
		base += len;
	}
	return 0;
}

int devdrv_plat_parse_resmem_usage(
		struct device_node *module_np,
		struct devdrv_platform_info *plat_info)
{
	int ret = 0;

	ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
		DEVDRV_RESMEM_TSFW_NAME, &DEVDRV_PLAT_GET_TSFW_BUF(plat_info));
	if (ret != 0) { return ret; }

	ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
		DEVDRV_RESMEM_AIFW_NAME, &DEVDRV_PLAT_GET_AIFW_BUF(plat_info));
	if (ret != 0) { return ret; }

	ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
		DEVDRV_RESMEM_SQCQ_NAME, &DEVDRV_PLAT_GET_SQCQ_BUF(plat_info));
	if (ret != 0) { return ret; }

	ret = devdrv_plat_find_resmem_idx(module_np, plat_info,
		DEVDRV_RESMEM_PERSISTENT_TAKS_BUF_NAME,
		&DEVDRV_PLAT_GET_PERSISTENT_TASK_BUF(plat_info));
	if (ret != 0) { return ret;}

	return 0;
}

