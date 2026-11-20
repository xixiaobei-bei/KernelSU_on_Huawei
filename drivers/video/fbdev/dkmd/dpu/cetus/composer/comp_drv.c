/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/genalloc.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <dpu/soc_dpu_define.h>

#include "dpu_utils.h"
#include "comp_drv.h"
#include "comp_dev.h"

static int comp_read_dts(struct device_node *np, struct composer_private *comp)
{
	comp->aclk_gate_dss = of_clk_get(np, 0);
	if (IS_ERR(comp->aclk_gate_dss)) {
		pr_err("scene-%d failed to get aclk_gate_dss!\n", comp->base.scene_id);
		return -ENXIO;
	}

	comp->pclk_gate_dss = of_clk_get(np, 1);
	if (IS_ERR(comp->pclk_gate_dss)) {
		pr_err("scene-%d failed to get pclk_gate_dss!\n", comp->base.scene_id);
		return -ENXIO;
	}

	comp->clk_gate_edc = of_clk_get(np, 2);
	if (IS_ERR(comp->clk_gate_edc)) {
		pr_err("scene-%d failed to get clk_gate_edc!\n", comp->base.scene_id);
		return -ENXIO;
	}

	// TODO: read other data

	return 0;
}

static void comp_init_data(struct composer_private *comp)
{
	dpu_sysfs_init(&comp->attrs);

	// TODO: init other data
	comp->base.on = dpu_comp_on;
	comp->base.off = dpu_comp_off;
	comp->base.create_fence = dpu_comp_on;
	comp->base.present = dpu_comp_present;
}

static int composer_device_setup(struct platform_device *pdev)
{
	int ret;
	struct composer_private *comp = NULL;

	comp = to_composer_private(pdev);
	if (!comp)
		return -EINVAL;

	ret = comp_read_dts(pdev->dev.of_node, comp);
	if (ret) {
		dpu_pr_err("read dts fail");
		return -EINVAL;
	}

	comp->dsssubsys_regulator = devm_regulator_get(&pdev->dev, "regulator_dsssubsys");
	if (IS_ERR(comp->dsssubsys_regulator)) {
		ret = PTR_ERR(comp->dsssubsys_regulator);
		pr_err("dsssubsys_regulator error, ret=%d", ret);
		return ret;
	}

	comp->vivobus_regulator = devm_regulator_get(&pdev->dev, "regulator_vivobus");
	if (IS_ERR(comp->vivobus_regulator)) {
		ret = PTR_ERR(comp->vivobus_regulator);
		pr_err("vivobus_regulator error, ret=%d", ret);
		return ret;
	}

	comp->media1_subsys_regulator = devm_regulator_get(&pdev->dev, "regulator_media_subsys");
	if (IS_ERR(comp->media1_subsys_regulator)) {
		ret = PTR_ERR(comp->media1_subsys_regulator);
		pr_err("media1_subsys_regulator error, ret=%d", ret);
		return ret;
	}

	comp->regulator_smmu_tcu = devm_regulator_get(&pdev->dev, "regulator_smmu_tcu");
	if (IS_ERR(comp->regulator_smmu_tcu)) {
		ret = PTR_ERR(comp->regulator_smmu_tcu);
		pr_err("regulator_smmu_tcu error, ret=%d", ret);
		return ret;
	}

	comp_init_data(comp);

	return 0;
}

static void composer_device_release(struct platform_device *pdev)
{
	struct composer_private *comp = NULL;

	comp = to_composer_private(pdev);
	if (!comp)
		return;
}

static int comp_probe(struct platform_device *pdev)
{
	const struct composer_match_data *data = NULL;
	struct composer_private *comp = NULL;

	dpu_pr_info("comp_probe enter!\n");

	if (!pdev) {
		pr_err("pdev is null!\n");
		return -EINVAL;
	}

	data = of_device_get_match_data(&pdev->dev);
	if (!data) {
		pr_err("get device data failed!\n");
		return -EINVAL;
	}

	comp = devm_kzalloc(&pdev->dev, sizeof(*comp), GFP_KERNEL);
	if (!comp) {
		dpu_pr_err("alloc composer fail");
		return -EINVAL;
	}
	comp->base.scene_id = data->scene_id;
	comp->base.pdev = pdev;

	platform_set_drvdata(pdev, &comp->base);
	if (data->of_device_setup)
		data->of_device_setup(pdev);

	comp_mgr_register_composer(comp);

	return 0;
}

static int comp_remove(struct platform_device *pdev)
{
	const struct composer_match_data *data = NULL;
	struct composer_private *comp = NULL;

	dpu_pr_info("comp_remove enter!\n");

	if (!pdev) {
		pr_err("pdev is null!\n");
		return -EINVAL;
	}

	data = of_device_get_match_data(&pdev->dev);
	if (!data) {
		pr_err("get device data failed!\n");
		return -EINVAL;
	}

	if (data->of_device_release)
		data->of_device_release(pdev);

	comp = to_composer_private(pdev);
	if (!comp)
		return -EINVAL;

	return 0;
}

#define composer_device_match_data(name, ver, setup, release) \
	static struct composer_match_data name = { .scene_id = ver, .of_device_setup = setup, .of_device_release = release }

composer_device_match_data(composer_scene0_info, DPU_SCENE_ONLINE_0, composer_device_setup, composer_device_release);
composer_device_match_data(composer_scene1_info, DPU_SCENE_ONLINE_1, composer_device_setup, composer_device_release);
composer_device_match_data(offline0_info, DPU_SCENE_OFFLINE_0, composer_device_setup, composer_device_release);
composer_device_match_data(offline1_info, DPU_SCENE_OFFLINE_1, composer_device_setup, composer_device_release);
composer_device_match_data(offline2_info, DPU_SCENE_OFFLINE_2, composer_device_setup, composer_device_release);

static const struct of_device_id g_comp_match_table[] = {
	{
		.compatible = "hisilicon,composer_scene0",
		.data = &composer_scene0_info,
	},
	{
		.compatible = "hisilicon,composer_scene1",
		.data = &composer_scene1_info,
	},
	{
		.compatible = "hisilicon,offline0",
		.data = &offline0_info,
	},
	{
		.compatible = "hisilicon,offline1",
		.data = &offline1_info,
	},
	{
		.compatible = "hisilicon,offline2",
		.data = &offline2_info,
	},
	{},
};

static struct platform_driver g_composer_driver = {
	.probe = comp_probe,
	.remove = comp_remove,
	.driver = {
		.name = "dpu_composer",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_comp_match_table),
	}
};

static int __init composer_register(void)
{
	return platform_driver_register(&g_composer_driver);
}

static void __exit composer_unregister(void)
{
	platform_driver_unregister(&g_composer_driver);
}

module_init(composer_register);
module_exit(composer_unregister);

MODULE_AUTHOR("Graphics Display");
MODULE_DESCRIPTION("Composer Module Driver");
MODULE_LICENSE("GPL");
