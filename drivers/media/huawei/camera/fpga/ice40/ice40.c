/*
 * ice40.c
 *
 * camera driver source file
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>

#include "hwcam_intf.h"
#include "../hwfpga.h"
#include "ice40_spi.h"
#include "cam_log.h"

struct ice40_t {
	char                           name[HWFPGA_NAME_SIZE];
	struct hwfpga_intf_t           intf;
	struct hwfpga_notify_intf_t    *notify;
};

#define intf_to_ice40(i) container_of(i, struct ice40_t, intf)

static struct ice40_t g_ice40;
static struct platform_device *g_pdev;

int ice40_power_on(const struct hwfpga_intf_t *i)
{
	return ice40_spi_init();
}

int ice40_power_off(const struct hwfpga_intf_t *i)
{
	return ice40_spi_exit();
}

int ice40_load_firmware(const struct hwfpga_intf_t *i)
{
	return ice40_spi_load_fw();
}

int ice40_enable(const struct hwfpga_intf_t *i)
{
	return ice40_spi_enable();
}

int ice40_disable(const struct hwfpga_intf_t *i)
{
	return ice40_spi_disable();
}

int ice40_init_fun(const struct hwfpga_intf_t *i)
{
	return ice40_spi_init_fun();
}

int ice40_close_fun(const struct hwfpga_intf_t *i)
{
	return ice40_spi_close_fun();
}

char const *ice40_get_name(const struct hwfpga_intf_t *i)
{
	struct ice40_t *ice40 = NULL;

	ice40 = intf_to_ice40(i);
	return ice40->name;
}

void ice40_notify_error(uint32_t id)
{
	hwfpga_event_t ice40_ev;

	ice40_ev.kind = HWFPGA_INFO_ERROR;
	ice40_ev.data.error.id = id;
	cam_info("%s id = %x", __func__, id);
	hwfpga_intf_notify_error(g_ice40.notify, &ice40_ev);
}

int ice40_checkdevice(const struct hwfpga_intf_t *i)
{
	return ice40_spi_checkdevice();
}

static struct hwfpga_vtbl_t g_vtbl_ice40 = {
	.get_name = ice40_get_name,
	.power_on = ice40_power_on,
	.power_off = ice40_power_off,
	.load_firmware = ice40_load_firmware,
	.enable  = ice40_enable,
	.disable  = ice40_disable,
	.init  = ice40_init_fun,
	.close = ice40_close_fun,
	.checkdevice = ice40_checkdevice,
};

static struct ice40_t g_ice40 = {
	.name = "ice40",
	.intf = { .vtbl = &g_vtbl_ice40, },
};

static const struct of_device_id g_ice40_dt_match[] = {
	{
		.compatible = "huawei,fpga_ice40",
		.data = &g_ice40.intf,
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, g_ice40_dt_match);

static struct platform_driver g_ice40_driver = {
	.driver = {
		.name = "huawei,ice40",
		.owner = THIS_MODULE,
		.of_match_table = g_ice40_dt_match,
	},
};

static int32_t ice40_platform_probe(struct platform_device *pdev)
{
	cam_notice("%s enter", __func__);
	g_pdev = pdev;
	return hwfpga_register(pdev, &g_ice40.intf, &g_ice40.notify);
}

static int __init ice40_init_module(void)
{
	int ret;

	cam_notice("%s enter", __func__);

	ret = platform_driver_probe(&g_ice40_driver,
		ice40_platform_probe);

	return ret;
}

static void __exit ice40_exit_module(void)
{
	platform_driver_unregister(&g_ice40_driver);
	if (g_pdev) {
		hwfpga_unregister(g_pdev);
		g_pdev = NULL;
	}
}

module_init(ice40_init_module);
module_exit(ice40_exit_module);
MODULE_DESCRIPTION("ice40");
MODULE_LICENSE("GPL v2");

