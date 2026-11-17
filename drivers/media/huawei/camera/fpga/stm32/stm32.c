/*
 * stm32.c
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
#include "stm32_spi.h"
#include "cam_log.h"

struct stm32_t {
	char name[HWFPGA_NAME_SIZE];
	struct hwfpga_intf_t intf;
	struct hwfpga_notify_intf_t *notify;
};

#define intf_to_stm32(i) container_of(i, struct stm32_t, intf)

static struct stm32_t g_stm32;
static struct platform_device *g_pdev;

int stm32_power_on(const struct hwfpga_intf_t *i)
{
	return stm32_spi_init();
}

int stm32_power_off(const struct hwfpga_intf_t *i)
{
	return stm32_spi_exit();
}

int stm32_load_firmware(const struct hwfpga_intf_t *i)
{
	return stm32_spi_load_fw();
}

int stm32_enable(const struct hwfpga_intf_t *i)
{
	return stm32_spi_enable();
}

int stm32_disable(const struct hwfpga_intf_t *i)
{
	return stm32_spi_disable();
}

int stm32_init_fun(const struct hwfpga_intf_t *i)
{
	return stm32_spi_init_fun();
}

int stm32_close_fun(const struct hwfpga_intf_t *i)
{
	return stm32_spi_close_fun();
}

char const *stm32_get_name(const struct hwfpga_intf_t *i)
{
	struct stm32_t *stm32 = NULL;

	stm32 = intf_to_stm32(i);
	return stm32->name;
}

void stm32_notify_error(uint32_t id)
{
	hwfpga_event_t stm32_ev;

	stm32_ev.kind = HWFPGA_INFO_ERROR;
	stm32_ev.data.error.id = id;
	cam_info("%s id = %x", __func__, id);
	hwfpga_intf_notify_error(g_stm32.notify, &stm32_ev);
}

int stm32_checkdevice(const struct hwfpga_intf_t *i)
{
	return stm32_spi_checkdevice();
}

static struct hwfpga_vtbl_t g_vtbl_stm32 = {
	.get_name      = stm32_get_name,
	.power_on      = stm32_power_on,
	.power_off     = stm32_power_off,
	.load_firmware = stm32_load_firmware,
	.enable        = stm32_enable,
	.disable       = stm32_disable,
	.init          = stm32_init_fun,
	.close         = stm32_close_fun,
	.checkdevice   = stm32_checkdevice,
};

static struct stm32_t g_stm32 = {
	.name = "stm32",
	.intf = { .vtbl = &g_vtbl_stm32, },
};

static const struct of_device_id g_stm32_dt_match[] = {
	{
		.compatible = "huawei,fpga_stm32",
		.data = &g_stm32.intf,
	}, {
	},
};
MODULE_DEVICE_TABLE(of, g_stm32_dt_match);

static struct platform_driver g_stm32_driver = {
	.driver = {
		.name = "huawei,stm32",
		.owner = THIS_MODULE,
		.of_match_table = g_stm32_dt_match,
	},
};

static int32_t stm32_platform_probe(struct platform_device *pdev)
{
	if (!pdev)
		return -1;

	cam_notice("%s enter", __func__);
	g_pdev = pdev;
	return hwfpga_register(pdev, &g_stm32.intf, &g_stm32.notify);
}

static int __init stm32_init_module(void)
{
	int ret;

	cam_notice("%s enter", __func__);
	ret = platform_driver_probe(&g_stm32_driver, stm32_platform_probe);
	return ret;
}

static void __exit stm32_exit_module(void)
{
	platform_driver_unregister(&g_stm32_driver);
	if (g_pdev) {
		hwfpga_unregister(g_pdev);
		g_pdev = NULL;
	}
}

module_init(stm32_init_module);
module_exit(stm32_exit_module);
MODULE_DESCRIPTION("stm32");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

