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
#ifndef DPU_COMPOSER_H
#define DPU_COMPOSER_H
#include <linux/types.h>
#include <linux/platform_device.h>

#include "dkmd_connector.h"
struct composer {
	struct platform_device *pdev;
	struct dkmd_connector_info *connector_info;
	int scene_id;

	int (*on) (struct platform_device *pdev);
	int (*off) (struct platform_device *pdev);

	int (*create_fence) (struct platform_device *pdev);
	int (*present) (struct platform_device *pdev, void *frame);
};

#endif