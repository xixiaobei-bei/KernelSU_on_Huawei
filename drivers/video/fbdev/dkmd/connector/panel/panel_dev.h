/**
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

#ifndef __PANEL_DEV_H__
#define __PANEL_DEV_H__

#include "dkmd_connector.h"

extern uint32_t gpio_lcd_vsp_enable;
extern uint32_t gpio_lcd_vsn_enable;
extern uint32_t gpio_lcd_reset;
extern uint32_t gpio_lcd_bl_enable;
extern uint32_t gpio_lcd_tp1v8;

extern struct regulator *vcc_lcdio;
extern struct regulator *vcc_lcdanalog;

struct panel_dev_private_data {
	struct dkmd_connector_data base;
};

struct panel_ops_handle_data {
	char *ops_cmd;
	int (*handle_func)(struct platform_device *pdev, void *desc);
};

static inline struct panel_dev_private_data *to_panel_dev_private_data(struct platform_device *pdev)
{
	struct dkmd_connector_data *pdata = dev_get_platdata(&pdev->dev);

	return container_of(pdata, struct panel_dev_private_data, base);
}

extern struct panel_dev_private_data panel_dev_data;

#endif