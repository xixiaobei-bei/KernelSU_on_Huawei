 /*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * Author:
 * Email:
 * Date:
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

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"
#include "../pmic/hw_pmic.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define CAMERA_LOG_DEBUG

#define BS_DAY    0
extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
static char const* ov8856_get_name(hwsensor_intf_t* si);
static int ov8856_config(hwsensor_intf_t* si, void  *argp);
static int ov8856_power_up(hwsensor_intf_t* si);
static int ov8856_power_down(hwsensor_intf_t* si);
static int ov8856_match_id(hwsensor_intf_t *si, void *data);
static int ov8856_csi_enable(hwsensor_intf_t* si);
static int ov8856_csi_disable(hwsensor_intf_t* si);

static bool s_ov8856_power_on = false;

struct mutex ov8856_power_lock;


static struct sensor_power_setting ov8856_power_setting_hma [] = {
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static hwsensor_vtbl_t
s_ov8856_vtbl =
{
    .get_name = ov8856_get_name,
    .config = ov8856_config,
    .power_up = ov8856_power_up,
    .power_down = ov8856_power_down,
    .match_id = ov8856_match_id,
    .csi_enable = ov8856_csi_enable,
    .csi_disable = ov8856_csi_disable,
};

atomic_t volatile ov8856_powered = ATOMIC_INIT(0);
static sensor_t s_ov8856_hma =
{
    .intf = { .vtbl = &s_ov8856_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(ov8856_power_setting_hma),
            .power_setting = ov8856_power_setting_hma,
    },
    .p_atpowercnt = &ov8856_powered,
};

static const struct of_device_id
s_ov8856_dt_match[] =
{
    {
        .compatible = "huawei,ov8856_hma",
        .data = &s_ov8856_hma.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_ov8856_dt_match);

/* platform driver struct */
static int32_t ov8856_platform_probe(struct platform_device* pdev);
static int32_t ov8856_platform_remove(struct platform_device* pdev);
static struct platform_driver
s_ov8856_driver =
{
    .probe = ov8856_platform_probe,
    .remove = ov8856_platform_remove,
    .driver =
    {
        .name = "huawei,ov8856",
        .owner = THIS_MODULE,
        .of_match_table = s_ov8856_dt_match,
    },
};

static char const*
ov8856_get_name(
        hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

static int
ov8856_power_up(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    if (hw_is_fpga_board()){
        //ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power up sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

static int
ov8856_power_down(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        //ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power down sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power down sensor fail.", __func__);
    }
    return ret;
}

static int ov8856_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int ov8856_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int
ov8856_match_id(
        hwsensor_intf_t *si, void *data)
{
    sensor_t* sensor = I2S(si);
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;
    char * sensor_name[] = {"OV8856_2L"};

    cam_info("%s enter.", __func__);

    cdata->data = SENSOR_INDEX_INVALID;  /*lint !e569 */

    if(!strncmp(sensor->board_info->name,"OV8856_2L",strlen("OV8856_2L")))
    {
        strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE-1, sensor_name[0], strlen(sensor_name[0])+1);
        cdata->data = sensor->board_info->sensor_index;
    }
    else
    {
        strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE-1, sensor->board_info->name, strlen(sensor->board_info->name)+1);
        cdata->data = sensor->board_info->sensor_index;
    }

    if (cdata->data != SENSOR_INDEX_INVALID) /*lint !e650 */
    {
        /*
        hwsensor_writefile(sensor->board_info->sensor_index, cdata->cfg.name);
        */
        cam_info("%s, cdata->cfg.name = %s", __func__,cdata->cfg.name );
    }
    cam_info("%s TODO.  cdata->data=%d", __func__, cdata->data);
    return 0;
}

static int
ov8856_config(
        hwsensor_intf_t* si,void  *argp)
{
    struct sensor_cfg_data *data;
    int ret =0;
    data = (struct sensor_cfg_data *)argp;
    cam_info("ov8856 cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&ov8856_power_lock);
            if (!s_ov8856_power_on) {
                ret = si->vtbl->power_up(si);
                s_ov8856_power_on = true;
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov8856_power_lock);
            /*lint -e455 -esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&ov8856_power_lock);
            if (s_ov8856_power_on) {
                ret = si->vtbl->power_down(si);
                s_ov8856_power_on = false;
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov8856_power_lock);
            /*lint -e455 -esym(455,*)*/
            break;
        case SEN_CONFIG_WRITE_REG:
            break;
        case SEN_CONFIG_READ_REG:
            break;
        case SEN_CONFIG_WRITE_REG_SETTINGS:
            break;
        case SEN_CONFIG_READ_REG_SETTINGS:
            break;
        case SEN_CONFIG_ENABLE_CSI:
            /* ret = si->vtbl->csi_enable(si); */
            break;
        case SEN_CONFIG_DISABLE_CSI:
            /* ret = si->vtbl->csi_disable(si); */
            break;
        case SEN_CONFIG_MATCH_ID:
            ret = si->vtbl->match_id(si,argp);
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }

    return ret;
}

static int32_t
ov8856_platform_probe(
        struct platform_device* pdev)
{
    int rc = 0;
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *id;
    hwsensor_intf_t *intf;
    sensor_t *sensor;

    cam_info("enter %s",__func__);

    if (!np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_ov8856_dt_match, np);
    if (!id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s no dt data", __func__);
        return -ENODEV;
    }
    sensor->dev = &pdev->dev;
    mutex_init(&ov8856_power_lock);

    rc = hwsensor_register(pdev, intf);
    rc = rpmsg_sensor_register(pdev, (void*)sensor);

    return rc;
}

static int32_t
ov8856_platform_remove(
        struct platform_device * pdev)
{
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *id;
    hwsensor_intf_t *intf;
    sensor_t *sensor;

    cam_info("enter %s",__func__);

    if (!np) {
        cam_info("%s of_node is NULL", __func__);
        return 0;
    }
    /* don't use dev->p->driver_data
    * we need to search again */
    id = of_match_node(s_ov8856_dt_match, np);
    if (!id) {
        cam_info("%s none id matched", __func__);
        return 0;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);

    rpmsg_sensor_unregister((void*)&sensor);
    hwsensor_unregister(intf);
    return 0;
}
static int __init
ov8856_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_ov8856_driver,
        ov8856_platform_probe);
}

static void __exit
ov8856_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_ov8856_hma);
    hwsensor_unregister(&s_ov8856_hma.intf);
    platform_driver_unregister(&s_ov8856_driver);
}

module_init(ov8856_init_module);
module_exit(ov8856_exit_module);
MODULE_DESCRIPTION("ov8856");
MODULE_LICENSE("GPL v2");

