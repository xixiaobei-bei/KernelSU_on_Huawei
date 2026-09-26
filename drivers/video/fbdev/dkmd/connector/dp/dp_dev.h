/**
 * @file dp_dev.c
 * @brief dp device driver
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
#ifndef __DP_DEV_H__
#define __DP_DEV_H__

#include "dkmd_connector.h"

struct dp_dev_private_data {
	struct dkmd_connector_data base;
	struct platform_device *pdev;

};

static inline struct dp_dev_private_data *to_dp_dev_private_data(struct dkmd_connector_data *data)
{
	return container_of(data, struct dp_dev_private_data, base);
}

#endif