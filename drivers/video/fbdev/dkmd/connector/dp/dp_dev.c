
/**
 * @file dp_dev.c
 * @brief display port device driver
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

#include "dp_dev.h"
#include "dp_drv.h"

static int dp_on(struct platform_device *pdev)
{
	return 0;
}

static int dp_off(struct platform_device *pdev)
{
	return 0;
}

struct dp_dev_private_data dp_dev_data = {
	.base = {
		.on_func = dp_on,
		.off_func = dp_off,
		.ops_handle_func = NULL,
		.next = NULL,
	},
};