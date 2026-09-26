/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: usb reg config
 * Create: 2019-03-14
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/device.h>
#include <linux/hisi/usb/hisi_usb_reg_cfg.h>
#include <linux/of.h>
#include <linux/slab.h>

#define CHIP_USB_REG_CFG_ARRAY_MAX 16
#define REG_CFG_CELL_COUNT 4
#define BIT_MASK_OFFSET 16

static const char *chip_usb_reg_cells_name = "#chip-usb-reg-cells";

struct chip_usb_reg_cfg *of_get_chip_usb_reg_cfg_by_index(
	struct device_node *np, const char *prop_name, int index)
{
	struct chip_usb_reg_cfg *reg_cfg = NULL;
	struct regmap *regmap = NULL;
	struct of_phandle_args args;
	int ret;

	if (!np || !prop_name)
		return NULL;

	ret = of_parse_phandle_with_args(np, prop_name,
		chip_usb_reg_cells_name, index, &args);
	if (ret) {
		pr_err("%s:get phandle failed %d\n", prop_name, ret);
		return NULL;
	}

	if (args.args_count != REG_CFG_CELL_COUNT) {
		pr_err("%s:args count %d error\n", prop_name, args.args_count);
		goto out;
	}

	regmap = syscon_node_to_regmap(args.np);
	if (IS_ERR(regmap)) {
		pr_err("%s:get regmap error\n", prop_name);
		goto out;
	}

	if (args.args[0] >= REG_CFG_TYPE_MAX) {
		pr_err("%s:reg_cfg_type error\n", prop_name);
		goto out;
	}

	reg_cfg = kzalloc(sizeof(*reg_cfg), GFP_KERNEL);
	if (!reg_cfg) {
		pr_err("%s:alloc reg_cfg failed\n", prop_name);
		goto out;
	}

	reg_cfg->regmap = regmap;
	/* reg_cfg init by dts describe */
	reg_cfg->cfg_type = args.args[0];
	reg_cfg->offset = args.args[1];
	reg_cfg->value = args.args[2];
	reg_cfg->mask = args.args[3];

out:
	of_node_put(args.np);
	return reg_cfg;
}

struct chip_usb_reg_cfg *of_get_chip_usb_reg_cfg(
		struct device_node *np, const char *prop_name)
{
	return of_get_chip_usb_reg_cfg_by_index(np, prop_name, 0);
}

void of_remove_chip_usb_reg_cfg(struct chip_usb_reg_cfg *reg_cfg)
{
	kfree(reg_cfg);
}

int get_chip_usb_reg_cfg_array(struct device *dev, const char *prop_name,
		struct chip_usb_reg_cfg ***reg_cfgs, int *num_cfgs)
{
	struct device_node *np = NULL;
	int elem_count;
	int ret;
	int i;

	if (!dev || !prop_name || !reg_cfgs || !num_cfgs)
		return -EINVAL;

	np = dev->of_node;
	elem_count = of_count_phandle_with_args(np, prop_name,
						chip_usb_reg_cells_name);
	if (elem_count <= 0 || elem_count > CHIP_USB_REG_CFG_ARRAY_MAX)
		return -EINVAL;

	*reg_cfgs = devm_kcalloc(dev, elem_count,
				 sizeof(struct chip_usb_reg_cfg *), GFP_KERNEL);
	if (!(*reg_cfgs))
		return -ENOMEM;

	for (i = 0; i < elem_count; i++) {
		(*reg_cfgs)[i] = of_get_chip_usb_reg_cfg_by_index(np,
								  prop_name,
								  i);
		if (!(*reg_cfgs)[i]) {
			ret = -ENOMEM;
			goto clean;
		}
	}

	*num_cfgs = elem_count;

	return 0;
clean:
	while (i--) {
		of_remove_chip_usb_reg_cfg((*reg_cfgs)[i]);
		(*reg_cfgs)[i] = NULL;
	}
	devm_kfree(dev, *reg_cfgs);
	return ret;
}

void free_chip_usb_reg_cfg_array(struct chip_usb_reg_cfg **reg_cfgs,
				 int num_cfgs)
{
	int i;

	if (!reg_cfgs)
		return;

	for (i = 0; i < num_cfgs; i++)
		of_remove_chip_usb_reg_cfg(reg_cfgs[i]);
}

int chip_usb_reg_write(struct chip_usb_reg_cfg *reg_cfg)
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
			(reg_cfg->mask << BIT_MASK_OFFSET) | reg_cfg->value);
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

int chip_usb_reg_write_array(struct chip_usb_reg_cfg **reg_cfgs, int num_cfgs)
{
	int i;
	int ret;

	if (!reg_cfgs)
		return -EINVAL;

	for (i = 0; i < num_cfgs; i++) {
		ret = chip_usb_reg_write(reg_cfgs[i]);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Return 0 if test fail
 * Return 1 if test success
 * Return nagative value for error
 */
int chip_usb_reg_test_cfg(struct chip_usb_reg_cfg *reg_cfg)
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
