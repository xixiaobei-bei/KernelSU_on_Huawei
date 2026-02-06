/**
 * @file dp_drv.h
 * @brief Interface for display port driver function
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __DP_DRV_H__
#define __DP_DRV_H__

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/platform_device.h>

#include "dkmd_connector.h"

#define DEV_NAME_DP "dp"
#define DTS_COMP_DP0 "hisilicon,dp0"

#undef pr_fmt
#define pr_fmt(fmt)  "dp: " fmt

struct dp_match_data {
	int (*of_device_setup) (struct platform_device *pdev);
	void (*of_device_release) (struct platform_device *pdev);
};

struct dp_private {
	struct dkmd_connector_info base;
	struct platform_device *pdev;

	int device_initialized;
};

static inline struct dp_private *to_dp_private(struct dkmd_connector_info *info)
{
	return container_of(info, struct dp_private, base);
}

#endif
