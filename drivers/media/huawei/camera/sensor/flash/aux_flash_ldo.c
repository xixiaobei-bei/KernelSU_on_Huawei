/*
 * aux_flash_ldo.c
 * driver for aux_flash_ldo
 *
 * Copyright (c) 2011-2020 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "hw_subdev.h"
#include "hw_flash.h"
#include "hw_flash_i2c.h"
#include "aux_flash_ldo.h"
#include "hwcam_intf.h"
#include "../../platform/sensor_commom.h"

static struct regulator *g_aux_ldo_supply;

int aux_ldo_remove(void)
{
	if (g_aux_ldo_supply) {
		regulator_put(g_aux_ldo_supply);
		g_aux_ldo_supply = NULL;
	}

	return 0;
}

int aux_ldo_supply_enable(struct hw_flash_ctrl_t *flash_ctrl,
	unsigned int config_val)
{
	int ret;
	struct regulator *supply = NULL;

	if (!flash_ctrl || !flash_ctrl->dev) {
		cam_err("%s: flash_ctrl is NULL\n", __func__);
		return -ENODEV;
	}

	if (g_aux_ldo_supply) {
		cam_info("%s: g_aux_ldo_supply is not NULL\n", __func__);
		aux_ldo_remove();
	}
	supply = regulator_get(flash_ctrl->dev, "flashldo");
	if (IS_ERR(supply)) {
		cam_err("%s: get regulator failed\n", __func__);
		return -EINVAL;
	}
	g_aux_ldo_supply = supply;

	ret = regulator_set_voltage(g_aux_ldo_supply, config_val, config_val);
	if (ret < 0) {
		cam_err("failed to set aux ldo to %d mV, ret = %d",
			config_val, ret);
		aux_ldo_remove();
		return ret;
	}
	ret = regulator_enable(g_aux_ldo_supply);
	if (ret) {
		cam_err("%s: regulator enable failed %d\n", __func__, ret);
		aux_ldo_remove();
		return ret;
	}

	cam_info("%s: regulator enable success\n", __func__);
	return 0;
}
EXPORT_SYMBOL_GPL(aux_ldo_supply_enable);

int aux_ldo_supply_disable(void)
{
	int ret;

	if (!g_aux_ldo_supply) {
		cam_err("%s: g_aux_power_supply is NULL\n", __func__);
		return -ENODEV;
	}

	ret = regulator_disable(g_aux_ldo_supply);
	if (ret)
		cam_err("%s: regulator disable failed %d\n", __func__, ret);
	else
		cam_info("%s: regulator disable success\n", __func__);

	aux_ldo_remove();

	return ret;
}
EXPORT_SYMBOL_GPL(aux_ldo_supply_disable);
