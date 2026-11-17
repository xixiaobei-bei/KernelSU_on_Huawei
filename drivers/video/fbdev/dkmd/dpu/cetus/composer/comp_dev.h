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
#ifndef COMPOSER_DEV_H
#define COMPOSER_DEV_H

#include <linux/list.h>
#include "comp_drv.h"

int dpu_comp_on(struct platform_device *pdev);
int dpu_comp_off(struct platform_device *pdev);
int dpu_comp_present(struct platform_device *pdev, void *frame);

static inline struct comp_frame* dpu_comp_get_using_frame(struct comp_present_data *present_data)
{
	return &present_data->frames[get_frame_idx(present_data->using_idx)];
}

static inline void dpu_comp_switch_present_index(struct comp_present_data *present_data)
{
	present_data->displayed_idx = present_data->displaying_idx;
	present_data->displaying_idx = present_data->using_idx;
	present_data->using_idx = get_frame_idx(present_data->using_idx + 1);
}

#endif