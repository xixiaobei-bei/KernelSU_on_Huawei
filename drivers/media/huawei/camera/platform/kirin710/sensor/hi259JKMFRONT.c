/*
 * hi259JKMFRONT.c
 *
 * driver for hi259JKMFRONT sensor.
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
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

//lint -save -e846 -e866 -e826 -e785 -e838 -e715 -e747 -e774 -e778 -e732 -e731
//lint -save -e514 -e30 -e84 -e64 -e650 -e737 -e31 -e64 -esym(528,*) -esym(753,*)
#define SENSOR_INDEX_2       (2)
#define DELAY_5MS            (5)
#define DELAY_2MS            (2)
#define DELAY_1MS            (1)
#define DELAY_0MS            (0)

#define I2S(i) container_of((i), sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)

static bool s_hi259JKMFRONT_power_on = false; //false for power down, ture for power up
struct mutex hi259JKMFRONT_power_lock;
static struct platform_device *s_pdev = NULL;
static sensor_t *s_sensor = NULL;
static sensor_t s_hi259JKMFRONT;
static struct sensor_power_setting hi259JKMFRONT_power_setting[] = {
     //MCAM1  RESET [GPIO12]
    {
        .seq_type     = SENSOR_RST3,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = DELAY_0MS,
    },

    //MCAM1 PWDN  [GPIO_27]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //SCAM1 PWDN[GPIO28]
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //SCAM1 reset  [GPIO_220]
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_5MS,
    },
    //MCAM0 AFVDD 3.0V
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = DELAY_1MS,
    },

    //SEL=1 SWITCH MIPI to SCAM1
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW, //out down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //SCAM1 IOVDD 1.80V[LDO21]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },

    //SCAM1 AVDD0  2.8V [LDO22]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },

    //SCAM1 MCLK [ISP-CLK2]
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_2,
        .delay = DELAY_2MS,
    },

    //SCAM1 PWDN [GPIO28]
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = DELAY_5MS,
    },

    //SCAM1 RESET  [GPIO_220]
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_HIGH,//pull up
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
};
static struct sensor_power_setting hi259JKMFRONT_power_down_setting[] = {
    //SCAM1 RESET  [GPIO_220]
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,//pull down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //SCAM1 MCLK [ISP-CLK2]
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_2,
        .delay = DELAY_1MS,
    },
    //SCAM1 PWDN [GPIO28]
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = DELAY_1MS,
    },

    //AVDD  2.8V [LDO22]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //IOVDD&DVDD 1.80V[LDO21]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW, //out down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //MCAM0 AFVDD 3.0V
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = DELAY_1MS,
    },
    //MCAM1  RESET [GPIO12]
    {
        .seq_type     = SENSOR_RST3,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = DELAY_1MS,
    },
    //MCAM1 RESET  [GPIO_27]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
};

static char const* hi259JKMFRONT_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return NULL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return NULL;
    }

    return sensor->board_info->name;
}

static int hi259JKMFRONT_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    if(NULL == si)
    {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    ret = hw_sensor_power_up_config(sensor->dev, sensor->board_info);
    if (0 == ret ){
        cam_info("%s. power up config success.", __func__);
    }else{
        cam_err("%s. power up config fail.", __func__);
        return ret;
    }
    if (hw_is_fpga_board()) {
        cam_info("%s powerup by isp on FPGA", __func__);
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

static int hi259JKMFRONT_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    if(NULL == si)
    {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        cam_info("%s poweroff by isp on FPGA", __func__);
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
    hw_sensor_power_down_config(sensor->board_info);
    return ret;
}

static int hi259JKMFRONT_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int hi259JKMFRONT_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int hi259JKMFRONT_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;

    cam_info("%s enter.", __func__);
    if (!si || !data){
        cam_err("%s : si or data is null", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info or name is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("%s name:%s", __func__, sensor->board_info->name);

    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static int hi259JKMFRONT_config_power_on(hwsensor_intf_t* si)
{
    int ret = 0;

    if (NULL == si || NULL == si->vtbl) {
        cam_err("%s : si or si->vtbl is null", __func__);
        return -EINVAL;
    }
    mutex_lock(&hi259JKMFRONT_power_lock);

    if (NULL == si->vtbl->power_up)
    {
        cam_err("%s. si power_up is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&hi259JKMFRONT_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (!s_hi259JKMFRONT_power_on){
        ret = si->vtbl->power_up(si);
        if (0 == ret) {
            s_hi259JKMFRONT_power_on = true;
        } else {
            cam_err("%s. power up fail.", __func__);
        }
    } else {
        cam_err("%s camera has powered on",__func__);
    }

    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&hi259JKMFRONT_power_lock);
    /*lint -e455 +esym(455,*)*/

    return ret;
}

static int hi259JKMFRONT_config_power_off(hwsensor_intf_t* si)
{
    int ret = 0;

    if (NULL == si || NULL == si->vtbl) {
        cam_err("%s : si or si->vtbl is null", __func__);
        return -EINVAL;
    }
    mutex_lock(&hi259JKMFRONT_power_lock);

    if (NULL == si->vtbl->power_down)
    {
        cam_err("%s. si power_down is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&hi259JKMFRONT_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (s_hi259JKMFRONT_power_on){
        ret = si->vtbl->power_down(si);
        if (0 != ret) {
            cam_err("%s. power down fail.", __func__);
        }
        s_hi259JKMFRONT_power_on = false;
    } else {
        cam_err("%s camera has powered off",__func__);
    }
    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&hi259JKMFRONT_power_lock);
    /*lint -e455 +esym(455,*)*/

    return ret;
}

static int hi259JKMFRONT_config_match_id(hwsensor_intf_t* si, void *argp)
{
    int ret = 0;

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->match_id)
    {
        cam_err("%s. si power_up is null", __func__);
        ret = -EINVAL;
    } else {
        ret = si->vtbl->match_id(si,argp);
    }

    return ret;
}

static int hi259JKMFRONT_config(hwsensor_intf_t* si,void  *argp)
{
    struct sensor_cfg_data *data = NULL;
    int ret =0;

    if ((NULL == si) || (NULL == argp) || (NULL == si->vtbl)) {
        cam_err("%s : si, argp or si->vtbl is null", __func__);
        return -EINVAL;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("hi259JKMFRONT cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            ret = hi259JKMFRONT_config_power_on(si);
            break;
        case SEN_CONFIG_POWER_OFF:
            ret = hi259JKMFRONT_config_power_off(si);
            break;
        case SEN_CONFIG_WRITE_REG:
        case SEN_CONFIG_READ_REG:
        case SEN_CONFIG_WRITE_REG_SETTINGS:
        case SEN_CONFIG_READ_REG_SETTINGS:
        case SEN_CONFIG_ENABLE_CSI:
        case SEN_CONFIG_DISABLE_CSI:
            break;
        case SEN_CONFIG_MATCH_ID:
            ret = hi259JKMFRONT_config_match_id(si, argp);
            break;
        case SEN_CONFIG_RESET_HOLD:
            break;
        case SEN_CONFIG_RESET_RELEASE:
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }
    return ret;
}

static hwsensor_vtbl_t s_hi259JKMFRONT_vtbl =
{
    .get_name = hi259JKMFRONT_get_name,
    .config = hi259JKMFRONT_config,
    .power_up = hi259JKMFRONT_power_up,
    .power_down = hi259JKMFRONT_power_down,
    .match_id = hi259JKMFRONT_match_id,
    .csi_enable = hi259JKMFRONT_csi_enable,
    .csi_disable = hi259JKMFRONT_csi_disable,
};
/* individual driver data for each device */

atomic_t volatile hi259JKMFRONT_powered = ATOMIC_INIT(0);

static sensor_t s_hi259JKMFRONT =
{
    .intf = { .vtbl = &s_hi259JKMFRONT_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(hi259JKMFRONT_power_setting),
        .power_setting = hi259JKMFRONT_power_setting,
    },
    .power_down_setting_array = {
        .size = ARRAY_SIZE(hi259JKMFRONT_power_down_setting),
        .power_setting = hi259JKMFRONT_power_down_setting,
    },
    .p_atpowercnt = &hi259JKMFRONT_powered,
};

static const struct of_device_id
s_hi259JKMFRONT_dt_match[] =
{
    {
        .compatible = "huawei,hi259JKMFRONT",
        .data = &s_hi259JKMFRONT.intf,
    },
    { } /* terminate list */
};

MODULE_DEVICE_TABLE(of, s_hi259JKMFRONT_dt_match);
/* platform driver struct */
static int32_t hi259JKMFRONT_platform_probe(struct platform_device* pdev);
static int32_t hi259JKMFRONT_platform_remove(struct platform_device* pdev);
static struct platform_driver
s_hi259JKMFRONT_driver =
{
    .probe = hi259JKMFRONT_platform_probe,
    .remove = hi259JKMFRONT_platform_remove,
    .driver =
    {
        .name = "huawei,hi259JKMFRONT",
        .owner = THIS_MODULE,
        .of_match_table = s_hi259JKMFRONT_dt_match,
    },
};


static int32_t
hi259JKMFRONT_platform_probe(
        struct platform_device* pdev)
{
    int rc = 0;

    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;
    cam_info("enter %s ",__func__);

    if(NULL == pdev)
    {
        cam_err("%s pdev is NULL", __func__);
        return -EINVAL;
    }

    mutex_init(&hi259JKMFRONT_power_lock);
    np = pdev->dev.of_node;
    if (!np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_hi259JKMFRONT_dt_match, np);
    if (!id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    if (NULL == intf) {
        cam_err("%s intf is NULL", __func__);
        return -ENODEV;
    }

    sensor = I2S(intf);
    if(NULL == sensor){
        cam_err("%s sensor is NULL rc %d", __func__, rc);
        return -ENODEV;
    }
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s no dt data rc %d", __func__, rc);
        return -ENODEV;
    }

    sensor->dev = &pdev->dev;

    rc = hwsensor_register(pdev, intf);
    if (rc < 0) {
        cam_err("%s hwsensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_pdev = pdev;

    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc < 0) {
        hwsensor_unregister(s_pdev);
        s_pdev = NULL;
        cam_err("%s rpmsg_sensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_sensor = sensor;

    return rc;
}

static int32_t hi259JKMFRONT_platform_remove(struct platform_device * pdev)
{
    if( NULL != s_sensor)
    {
        rpmsg_sensor_unregister((void*)s_sensor);
        s_sensor = NULL;
    }
    if (NULL != s_pdev) {
        hwsensor_unregister(s_pdev);
        s_pdev = NULL;
    }
    return 0;
}
static int __init hi259JKMFRONT_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_hi259JKMFRONT_driver,
            hi259JKMFRONT_platform_probe);
}

static void __exit hi259JKMFRONT_exit_module(void)
{
    platform_driver_unregister(&s_hi259JKMFRONT_driver);
}

module_init(hi259JKMFRONT_init_module);
module_exit(hi259JKMFRONT_exit_module);
MODULE_DESCRIPTION("hi259JKMFRONT");
MODULE_LICENSE("GPL v2");
//lint -restore
