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
#include <linux/module.h>
#include <linux/of_device.h>
#include "dkmd_utils.h"
#include "gfxdev_mgr.h"
#include "dpu_comp.h"
#include "dpu_fb.h"
#include "dpu_drm.h"
#include "dpu_chr.h"

static uint32_t g_disp_device_arch = FBDEV_ARCH;

void device_mgr_create_gfxdev(struct platform_device *pdev)
{
	struct composer *comp = NULL;

	if (!pdev) {
		dpu_pr_info("pdev is null!\n");
		return;
	}
	comp = platform_get_drvdata(pdev);

	if (g_disp_device_arch == FBDEV_ARCH)
		fb_device_register(comp);
	else
		drm_device_register(comp);
}
EXPORT_SYMBOL(device_mgr_create_gfxdev);

void device_mgr_destroy_gfxdev(struct platform_device *pdev)
{
	struct composer *comp = NULL;

	if (!pdev) {
		dpu_pr_info("pdev is null!\n");
		return;
	}
	comp = platform_get_drvdata(pdev);

	if (g_disp_device_arch == FBDEV_ARCH)
		fb_device_unregister(comp);
	else
		drm_device_unregister(comp);
}
EXPORT_SYMBOL(device_mgr_destroy_gfxdev);

void device_mgr_create_chrdev(struct platform_device *pdev)
{
	struct composer *comp = NULL;

	if (!pdev) {
		dpu_pr_info("pdev is null!\n");
		return;
	}
	comp = platform_get_drvdata(pdev);

	chr_device_register(comp);
}
EXPORT_SYMBOL(device_mgr_create_chrdev);

void device_mgr_destroy_chrdev(struct platform_device *pdev)
{
	struct composer *comp = NULL;

	if (!pdev) {
		dpu_pr_info("pdev is null!\n");
		return;
	}
	comp = platform_get_drvdata(pdev);

	chr_device_unregister(comp);
}
EXPORT_SYMBOL(device_mgr_destroy_chrdev);

// ============================================================================

static int gfxdev_probe(struct platform_device *pdev)
{
	int ret;

	ret = of_property_read_u32(pdev->dev.of_node, "disp_device_arch", &g_disp_device_arch);
	if (ret) {
		dpu_pr_info("get disp_device_arch failed!\n");
		g_disp_device_arch = FBDEV_ARCH;
	}
	dpu_pr_info("g_disp_device_arch=0x%x", g_disp_device_arch);

	return 0;
}

static int gfxdev_remove(struct platform_device *pdev)
{

	return 0;
}

static const struct of_device_id gfxdev_match_table[] = {
	{
		.compatible = "dkmd,dpu_device",
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, gfxdev_match_table);

static struct platform_driver gfxdev_driver = {
	.probe = gfxdev_probe,
	.remove = gfxdev_remove,
	.driver = {
		.name = "gfxdev",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(gfxdev_match_table),
	},
};

static int __init gfxdev_driver_init(void)
{
	return platform_driver_register(&gfxdev_driver);
}

static void __exit gfxdev_driver_deinit(void)
{
	platform_driver_unregister(&gfxdev_driver);
}

module_init(gfxdev_driver_init);
module_exit(gfxdev_driver_deinit);

MODULE_DESCRIPTION("Display Graphics Driver");
MODULE_LICENSE("GPL");