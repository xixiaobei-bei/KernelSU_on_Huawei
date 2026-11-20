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
#ifndef DPU_GFX_DEVICE_MGR_H
#define DPU_GFX_DEVICE_MGR_H
#include <linux/types.h>
#include <linux/platform_device.h>

enum {
	FBDEV_ARCH,
	DRMDEV_ARCH
};

void device_mgr_create_gfxdev(struct platform_device *pdev);
void device_mgr_destroy_gfxdev(struct platform_device *pdev);
void device_mgr_create_chrdev(struct platform_device *pdev);
void device_mgr_destroy_chrdev(struct platform_device *pdev);

#endif