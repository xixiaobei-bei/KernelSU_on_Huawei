/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

typedef struct _tag_ice40
{
    char                           name[HWFPGA_NAME_SIZE];
    hwfpga_intf_t                  intf;
    hwfpga_notify_intf_t          *notify;
} ice40_t;

#define INTF2ICE40(i) container_of(i, ice40_t, intf)

static ice40_t s_ice40;

void ice40_notify_error(uint32_t id);//stub

int ice40_power_on(const hwfpga_intf_t* i)
{
    return ice40_spi_init();
}

int ice40_power_off(const hwfpga_intf_t* i)
{
    return ice40_spi_exit();
}

int ice40_load_firmware(const hwfpga_intf_t* i)
{
    return ice40_spi_load_fw();
}

int ice40_enable(const hwfpga_intf_t* i)
{
    return ice40_spi_enable();
}

int ice40_disable(const hwfpga_intf_t* i)
{
    return ice40_spi_disable();
}

int ice40_init_fun(const hwfpga_intf_t* i)
{
    return ice40_spi_init_fun();
}

int ice40_close_fun(const hwfpga_intf_t* i)
{
    return ice40_spi_close_fun();
}

char const* ice40_get_name(const hwfpga_intf_t* i)
{
    ice40_t* ice40 = NULL;
    ice40 = INTF2ICE40(i);
    return ice40->name;
}

void ice40_notify_error( uint32_t id)
{
    hwfpga_event_t ice40_ev;
    ice40_ev.kind = HWFPGA_INFO_ERROR;
    ice40_ev.data.error.id = id;
    cam_info("%s id = %x",__func__,id);
    hwfpga_intf_notify_error(s_ice40.notify, &ice40_ev);
}

static hwfpga_vtbl_t
s_vtbl_ice40 =
{
    .get_name = ice40_get_name,
    .power_on = ice40_power_on,
    .power_off = ice40_power_off,
    .load_firmware = ice40_load_firmware,
    .enable  = ice40_enable,
    .disable  = ice40_disable,
    .init  = ice40_init_fun,
    .close = ice40_close_fun,
};

static ice40_t
s_ice40 =
{
    .name = "ice40",
    .intf = { .vtbl = &s_vtbl_ice40, },
};

static const struct of_device_id
s_ice40_dt_match[] =
{
    {
        .compatible = "huawei,fpga_ice40",
        .data = &s_ice40.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_fpga_dt_match);

static struct platform_driver
s_ice40_driver =
{
    .driver =
    {
        .name = "huawei,ice40",
        .owner = THIS_MODULE,
        .of_match_table = s_ice40_dt_match,
    },
};

static int32_t
ice40_platform_probe(
        struct platform_device* pdev)
{
    cam_notice("%s enter", __func__);
    return hwfpga_register(pdev, &s_ice40.intf, &s_ice40.notify);
}

static int __init
ice40_init_module(void)
{
    int ret = 0;

    cam_notice("%s enter", __func__);

    ret = platform_driver_probe(&s_ice40_driver,
            ice40_platform_probe);

    return ret;
}

static void __exit
ice40_exit_module(void)
{
    platform_driver_unregister(&s_ice40_driver);
    hwfpga_unregister(&s_ice40.intf);
}

module_init(ice40_init_module);
module_exit(ice40_exit_module);
MODULE_DESCRIPTION("ice40");
MODULE_LICENSE("GPL v2");

