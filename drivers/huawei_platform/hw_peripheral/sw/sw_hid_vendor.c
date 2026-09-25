/*
 * sw_hid_vendor.c
 *
 * single-wire hid vendor
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "sw_core.h"
#include "sw_debug.h"

int sw_drivers_init(void)
{
	int ret;

	ret = sw_bus_init();
	if (ret)
		goto err_bus;

	swhid_init();
	cmrkb_init();
	scmkb_init();

	return SUCCESS;

err_bus:
	sw_bus_exit();
	return ret;
}

void sw_drivers_exit(void)
{
	scmkb_exit();
	cmrkb_exit();
	swhid_exit();
	sw_bus_exit();
}
