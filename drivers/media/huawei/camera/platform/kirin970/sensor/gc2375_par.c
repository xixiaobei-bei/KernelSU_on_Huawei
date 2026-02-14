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

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"
#include <linux/pinctrl/consumer.h>
#include "../pmic/hw_pmic.h"

//lint -save -e846 -e866 -e826 -e785 -e838 -e715 -e747 -e774 -e778 -e732 -e731 -e569 -e650 -e31
#define SENSOR_INDEX_0       (0)
#define DELAY_1MS            (1)
#define DELAY_0MS            (0)
#define DELAY_5MS            (5)
#define I2S(i) container_of((i), sensor_t, intf)

static hwsensor_vtbl_t s_gc2375_vtbl;
static bool power_on_status = false;//false: power off, true:power on
static int gc2375_config(hwsensor_intf_t* si, void  *argp);
struct mutex gc2375_par_power_lock;
static hwsensor_intf_t *s_intf = NULL;
static sensor_t *s_sensor = NULL;

static struct sensor_power_setting gc2375_power_setting[] = {
    //SEL=0 SWITCH MIPI to S1 [GPIO-008]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW, //pull down, for power down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //disable M0 reset [GPIO-013]
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW, //SUSPEND type use real gpio status
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //disable M1 reset [GPIO-136]
    {
        .seq_type = SENSOR_SUSPEND2,
        .config_val = SENSOR_GPIO_LOW, //SUSPEND type use real gpio status
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //S1 PWDN STANDBY [GPIO-014]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_LOW, //out high
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //S1 IOVDD & DVDD 1.80V [PMU-LDO021]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //S1 AVDD 2.8V [PMU-LDO033]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },


    //M0 AFVDD & MIPISW_LDO 3.0V [PMU-LDO25]
    {
        .seq_type = SENSOR_VCM_AVDD,
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //SEL=0 SWITCH MIPI to S1 [GPIO-008]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH, //pull down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //S1 MCLK [ISP-CLK0]
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_0,
        .delay = DELAY_1MS,
    },
    //S1 PWDN NORMAL WORK [GPIO-014]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH, //pull down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_5MS,
    },

    //S1 RESET [GPIO_227]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
};

static struct sensor_power_setting gc2375_power_down_setting[] = {
    //S1 PWDN STANDBY [GPIO-014]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_LOW, //out low
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //SEL=0 SWITCH MIPI to S1 [GPIO-008]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW, //pull down, for power down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //disable M0 reset [GPIO-013]
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW, //SUSPEND type use real gpio status
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //disable M1 reset [GPIO-136]
    {
        .seq_type = SENSOR_SUSPEND2,
        .config_val = SENSOR_GPIO_LOW, //SUSPEND type use real gpio status
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //S1 IOVDD & DVDD 1.80V [PMU-LDO021]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //S1 AVDD 2.8V [PMU-LDO033]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },


    //M0 AFVDD & MIPISW_LDO 3.0V [PMU-LDO25]
    {
        .seq_type = SENSOR_VCM_AVDD,
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //SEL=0 SWITCH MIPI to S1 [GPIO-008]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH, //pull down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //S1 MCLK [ISP-CLK0]
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_0,
        .delay = DELAY_1MS,
    },
    //S1 RESET [GPIO_227]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
    //S1 PWDN NORMAL WORK [GPIO-014]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH, //pull down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_5MS,
    },
};

static char const* gc2375_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;
    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return NULL;
    }
    sensor = I2S(si);

    if (NULL == sensor || NULL == sensor->board_info) {
        cam_err("%s. sensor or sensor->board_info is NULL.", __func__);
        return NULL;
    }

    return sensor->board_info->name;
}

static int gc2375_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }
    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info) {
        cam_err("%s. sensor or sensor->board_info is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }

    if (0 == ret) {
        cam_info("%s. power up sensor success.", __func__);
    } else {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

static int gc2375_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }
    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info) {
        cam_err("%s. sensor or sensor->board_info is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }

    if (0 == ret) {
        cam_info("%s. power down sensor success.", __func__);
    } else {
        cam_err("%s. power down sensor fail.", __func__);
    }

    return ret;
}

static int gc2375_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int gc2375_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int gc2375_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;
    if(NULL == si || NULL == data) {
        cam_err("%s. si or data is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);

    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info or name is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("%s name:%s", __func__, sensor->board_info->name);

    cdata = (struct sensor_cfg_data *)data;
    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static int gc2375_config_power_on(hwsensor_intf_t* si)
{
    int ret = 0;
    mutex_lock(&gc2375_par_power_lock);

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->power_up)
    {
        cam_err("%s. si,si->vtbl or si->vtbl->power_up is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&gc2375_par_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (!power_on_status){
        ret = si->vtbl->power_up(si);
        if (0 == ret) {
            power_on_status = true;
        } else {
            cam_err("%s. power up fail.", __func__);
        }
    } else {
        cam_err("%s camera has powered on",__func__);
    }

    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&gc2375_par_power_lock);
    /*lint -e455 +esym(455,*)*/

    return ret;
}

static int gc2375_config_power_off(hwsensor_intf_t* si)
{
    int ret = 0;
    mutex_lock(&gc2375_par_power_lock);

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->power_down)
    {
        cam_err("%s. si,si->vtbl or si->vtbl->power_down is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&gc2375_par_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (power_on_status){
        ret = si->vtbl->power_down(si);
        if (0 != ret) {
            cam_err("%s. power down fail.", __func__);
        }
        power_on_status = false;
    } else {
        cam_err("%s camera has powered off",__func__);
    }
    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&gc2375_par_power_lock);
    /*lint -e455 +esym(455,*)*/

    return ret;
}

static int gc2375_config_match_id(hwsensor_intf_t* si, void *argp)
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

static int gc2375_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data = NULL;
    int ret = 0;

    if ((NULL == si) || (NULL == argp)) {
        cam_err("%s : si or argp is null", __func__);
        return -EINVAL;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("gc2375 cfgtype = %d",data->cfgtype);
    switch(data->cfgtype) {
        case SEN_CONFIG_POWER_ON:
            ret = gc2375_config_power_on(si);
            break;
        case SEN_CONFIG_POWER_OFF:
            ret = gc2375_config_power_off(si);
            break;
        case SEN_CONFIG_WRITE_REG:
        case SEN_CONFIG_READ_REG:
        case SEN_CONFIG_WRITE_REG_SETTINGS:
        case SEN_CONFIG_READ_REG_SETTINGS:
        case SEN_CONFIG_ENABLE_CSI:
        case SEN_CONFIG_DISABLE_CSI:
            break;
        case SEN_CONFIG_MATCH_ID:
            ret = gc2375_config_match_id(si, argp);
            break;
        default:
            cam_warn("%s cfgtype(%d) is unknow", __func__, data->cfgtype);
            break;
    }

    return ret;
}

static hwsensor_vtbl_t s_gc2375_vtbl =
{
    .get_name = gc2375_get_name,
    .config = gc2375_config,
    .power_up = gc2375_power_up,
    .power_down = gc2375_power_down,
    .match_id = gc2375_match_id,
    .csi_enable = gc2375_csi_enable,
    .csi_disable = gc2375_csi_disable,
};

atomic_t volatile gc2375par_powered = ATOMIC_INIT(0);

static sensor_t s_gc2375 =
{
    .intf = { .vtbl = &s_gc2375_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(gc2375_power_setting),
            .power_setting = gc2375_power_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(gc2375_power_down_setting),
            .power_setting = gc2375_power_down_setting,
     },
    .p_atpowercnt = &gc2375par_powered,
};

static const struct of_device_id s_gc2375_dt_match[] =
{
    {
        .compatible = "huawei,gc2375_par",
        .data = &s_gc2375.intf,
    },
    {

    },/* terminate list */
};

MODULE_DEVICE_TABLE(of, s_gc2375_dt_match);

static int32_t gc2375_platform_probe(struct platform_device* pdev);
static int32_t gc2375_platform_remove(struct platform_device* pdev);

static struct platform_driver s_gc2375_driver =
{
    .probe = gc2375_platform_probe,
    .remove = gc2375_platform_remove,
    .driver =
    {
        .name = "huawei,gc2375",
        .owner = THIS_MODULE,
        .of_match_table = s_gc2375_dt_match,
    },
};

static int32_t gc2375_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;
    cam_info("enter %s ",__func__);

    if (NULL == pdev) {
        cam_err("%s pdev is NULL", __func__);
        return -EINVAL;
    }

    mutex_init(&gc2375_par_power_lock);
    np = pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_gc2375_dt_match, np);
    if (NULL == id) {
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
    s_intf = intf;
    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc < 0) {
        hwsensor_unregister(intf);
        s_intf = NULL;
        cam_err("%s rpmsg_sensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_sensor = sensor;

    return rc;
}

static int32_t gc2375_platform_remove(
    struct platform_device * pdev)
{
    if( NULL != s_sensor) {
        rpmsg_sensor_unregister((void*)s_sensor);
        s_sensor = NULL;
    }
    if (NULL != s_intf) {
        hwsensor_unregister(s_intf);
        s_intf = NULL;
    }
    return 0;
}

static int __init gc2375_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_gc2375_driver,
            gc2375_platform_probe);
}

static void __exit gc2375_exit_module(void)
{
    if (NULL != s_intf) {
        hwsensor_unregister(s_intf);
        s_intf = NULL;
    }

    platform_driver_unregister(&s_gc2375_driver);
}

module_init(gc2375_init_module);
module_exit(gc2375_exit_module);
MODULE_DESCRIPTION("gc2375");
MODULE_LICENSE("GPL v2");
//lint -restore
