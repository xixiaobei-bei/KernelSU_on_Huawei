// SPDX-License-Identifier: GPL-2.0
/*
 * Register config for USB
 *
 * Authors: Yu Chen <chenyu56@huawei.com>
 */

#include <linux/of.h>
#include <linux/slab.h>
#include "hisi_usb_reg_cfg.h"
#include "dwc3-hisi.h"

#define REG_CFG_CELL_COUNT 4
#define BIT_MASK_OFFSET 16

struct hisi_usb_reg_cfg *of_get_hisi_usb_reg_cfg_by_name(struct device_node *np,
		const char *prop_name)
{
	struct hisi_usb_reg_cfg *reg_cfg = NULL;
	struct regmap *regmap = NULL;
	struct of_phandle_args args;
	int ret;

	if (!np || !prop_name)
		return NULL;

	ret = of_parse_phandle_with_fixed_args(np, prop_name,
			REG_CFG_CELL_COUNT, 0, &args);
	if (ret) {
		usb_err("%s:get phandle failed %d\n", prop_name, ret);
		return NULL;
	}

	if (args.args_count != REG_CFG_CELL_COUNT) {
		usb_err("%s:args count %d error\n", prop_name, args.args_count);
		goto out;
	}

	regmap = syscon_node_to_regmap(args.np);
	if (IS_ERR(regmap)) {
		usb_err("%s:get regmap error\n", prop_name);
		goto out;
	}

	if (args.args[0] >= REG_cfg_TYPE_MAX) {
		usb_err("%s:reg_cfg_type error\n", prop_name);
		goto out;
	}

	reg_cfg = kzalloc(sizeof(*reg_cfg), GFP_KERNEL);
	if (!reg_cfg) {
		usb_err("%s:alloc reg_cfg failed\n", prop_name);
		goto out;
	}

	reg_cfg->regmap = regmap;
	reg_cfg->cfg_type = args.args[0];
	reg_cfg->offset = args.args[1];
	reg_cfg->value = args.args[2];
	reg_cfg->mask = args.args[3];

out:
	of_node_put(args.np);
	return reg_cfg;
}

void of_remove_hisi_usb_reg_cfg(struct hisi_usb_reg_cfg *reg_cfg)
{
	kfree(reg_cfg);
}

int hisi_usb_reg_write(struct hisi_usb_reg_cfg *reg_cfg)
{
	int ret;

	if (!reg_cfg)
		return -EINVAL;

	switch (reg_cfg->cfg_type) {
	case WRITE_ONLY:
		ret = regmap_write(reg_cfg->regmap, reg_cfg->offset,
				reg_cfg->value);
		break;
	case BIT_MASK:
		ret = regmap_write(reg_cfg->regmap, reg_cfg->offset,
				(reg_cfg->value << BIT_MASK_OFFSET) |
				reg_cfg->value);
		break;
	case READ_WRITE:
		ret = regmap_write_bits(reg_cfg->regmap, reg_cfg->offset,
				reg_cfg->mask, reg_cfg->value);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

/*
 * Return 0 if test fail
 * Return 1 if test success
 * Return nagative value for error
 */
int hisi_usb_reg_test_cfg(struct hisi_usb_reg_cfg *reg_cfg)
{
	unsigned int reg_val = 0;
	int ret;

	if (!reg_cfg)
		return -EINVAL;

	ret = regmap_read(reg_cfg->regmap, reg_cfg->offset, &reg_val);
	if (ret)
		return ret;

	return (reg_val & reg_cfg->mask) == reg_cfg->value;
}
