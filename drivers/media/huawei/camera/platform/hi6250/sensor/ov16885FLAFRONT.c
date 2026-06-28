/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * Author:  wx480359
 * Email:
 * Date:    2017-12-19
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

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
struct mutex ov16885FLAFRONT_power_lock;
#define SENSOR_REG_SETTING_DELAY_1_MS 1
#define SENSOR_REG_SETTING_DELAY_5_MS 5
#define SENSOR_REG_SETTING_DELAY_6_MS 6
static hwsensor_vtbl_t s_ov16885FLAFRONT_vtbl;
static bool power_on_status = false;
//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737
int ov16885FLAFRONT_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hw_ov16885FLAFRONT_power_up_setting[] = {
    //MCAM0 AFVDD 3.0V [LDO25]
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MIPI Swith to SCAM [GPIO78]
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //disable MCAM0 reset [GPIO18]
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //disable MCAM1 reset [GPIO076]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable MCAM1 PWDN [GPIO075]
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable SCAM1 reset [GPIO060]
    {
        .seq_type     = SENSOR_RST2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable SCAM1 PWDN [GPIO068]
    {
        .seq_type     = SENSOR_PWDN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //SCAM0 RESET [GPIO17]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_5_MS,
    },
    //SCAM0 IOVDD 1.80V [LDO33]
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM0 AVDD 2.8V [LDO19]
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM0 DVDD 1.2V [LDO20]
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 AVDD 2.80V [LDO13]
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM0 DVDD 1.2V [LDO32]
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //SCAM0 RESET [GPIO17]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_6_MS,
    },
    //SCAM0 CLK
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
};
struct sensor_power_setting hw_ov16885FLAFRONT_power_down_setting[] = {
    //SCAM0 MCLK
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 RESET
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM0 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //SCAM0 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //SCAM0 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM0 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM0 AVDD 2.8V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MIPI Swith to SCAM
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM0 AFVDD 3.0V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //disable SCAM1 PWDN [GPIO068]
    {
        .seq_type     = SENSOR_PWDN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable SCAM1 reset [GPIO060]
    {
        .seq_type     = SENSOR_RST2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable MCAM1 PWDN
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable MCAM1 reset
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable MCAM0 reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
};
atomic_t volatile ov16885FLAFRONT_powered = ATOMIC_INIT(0);
static sensor_t s_ov16885FLAFRONT =
{
    .intf = { .vtbl = &s_ov16885FLAFRONT_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hw_ov16885FLAFRONT_power_up_setting),
            .power_setting = hw_ov16885FLAFRONT_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(hw_ov16885FLAFRONT_power_down_setting),
            .power_setting = hw_ov16885FLAFRONT_power_down_setting,
    },
    .p_atpowercnt = &ov16885FLAFRONT_powered,
};

static const struct of_device_id s_ov16885FLAFRONT_dt_match[] =
{
    {
        .compatible = "huawei,ov16885FLAFRONT",
        .data = &s_ov16885FLAFRONT.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_ov16885FLAFRONT_dt_match);

static struct platform_driver s_ov16885FLAFRONT_driver =
{
    .driver =
    {
        .name = "huawei,ov16885FLAFRONT",
        .owner = THIS_MODULE,
        .of_match_table = s_ov16885FLAFRONT_dt_match,
    },
};

char const* ov16885FLAFRONT_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return NULL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return NULL;
    }
    return sensor->board_info->name;
}

int ov16885FLAFRONT_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }
    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -1;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    ret = hw_sensor_power_up_config(s_ov16885FLAFRONT.dev, sensor->board_info);
    if (0 == ret )
    {
        cam_info("%s. power up sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power up sensor fail.", __func__);
        return ret;
    }

    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
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

int ov16885FLAFRONT_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -1;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
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

int ov16885FLAFRONT_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int ov16885FLAFRONT_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int ov16885FLAFRONT_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;

    if (!si || !data){
        cam_err("%s : si or data is null", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info is NULL.", __func__);
        return -1;
    }
    cdata = (struct sensor_cfg_data *)data;

    cam_info("%s enter.", __func__);

    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static hwsensor_vtbl_t s_ov16885FLAFRONT_vtbl =
{
    .get_name = ov16885FLAFRONT_get_name,
    .config = ov16885FLAFRONT_config,
    .power_up = ov16885FLAFRONT_power_up,
    .power_down = ov16885FLAFRONT_power_down,
    .match_id = ov16885FLAFRONT_match_id,
    .csi_enable = ov16885FLAFRONT_csi_enable,
    .csi_disable = ov16885FLAFRONT_csi_disable,
};

int ov16885FLAFRONT_config(hwsensor_intf_t* si,void  *argp)
{
    int ret =0;
    struct sensor_cfg_data *data = NULL;

    if (NULL == si || NULL == argp){
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("ov16885FLAFRONT cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&ov16885FLAFRONT_power_lock);
            if (false == power_on_status) {
                if(NULL == si->vtbl->power_up){
                    cam_err("%s. si->vtbl->power_up is null.", __func__);
                    mutex_unlock(&ov16885FLAFRONT_power_lock);
                    return -EINVAL;
                }else{
                     ret = si->vtbl->power_up(si);
                     if(0 == ret){
                         power_on_status = true;
                     }
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov16885FLAFRONT_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&ov16885FLAFRONT_power_lock);
            if (true == power_on_status) {
                if(NULL == si->vtbl->power_down){
                    cam_err("%s. si->vtbl->power_down is null.", __func__);
                    mutex_unlock(&ov16885FLAFRONT_power_lock);
                    return -EINVAL;
                }else{
                     ret = si->vtbl->power_down(si);
                     if(0 != ret){
                         cam_err("%s. power_down fail.", __func__);
                     }
                     power_on_status = false;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov16885FLAFRONT_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_MATCH_ID:
            if(NULL == si->vtbl->match_id){
                cam_err("%s. si->vtbl->match_id is null.", __func__);
                ret=-EINVAL;
            }else{
                ret = si->vtbl->match_id(si,argp);
            }
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }
    return ret;
}

static int32_t ov16885FLAFRONT_platform_probe(struct platform_device* pdev)
{
     int rc = 0;
    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }
     cam_info("enter %s",__func__);
     if (pdev->dev.of_node) {
         rc = hw_sensor_get_dt_data(pdev, &s_ov16885FLAFRONT);
         if (rc < 0) {
             cam_err("%s failed line %d\n", __func__, __LINE__);
             goto ov16885FLAFRONT_sensor_probe_fail;
         }
     } else {
         cam_err("%s ov16885FLAFRONT of_node is NULL.\n", __func__);
         goto ov16885FLAFRONT_sensor_probe_fail;
     }
    mutex_init(&ov16885FLAFRONT_power_lock);
    s_ov16885FLAFRONT.dev = &pdev->dev;

    rc = hwsensor_register(pdev, &s_ov16885FLAFRONT.intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto ov16885FLAFRONT_sensor_probe_fail;
    }

    rc = rpmsg_sensor_register(pdev, (void *)&s_ov16885FLAFRONT);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(Sensor2Pdev(s_ov16885FLAFRONT));
        goto ov16885FLAFRONT_sensor_probe_fail;
    }

ov16885FLAFRONT_sensor_probe_fail:
    return rc;
}

static int __init ov16885FLAFRONT_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_ov16885FLAFRONT_driver,ov16885FLAFRONT_platform_probe);
}

static void __exit ov16885FLAFRONT_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_ov16885FLAFRONT);
    hwsensor_unregister(Sensor2Pdev(s_ov16885FLAFRONT));
    platform_driver_unregister(&s_ov16885FLAFRONT_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(ov16885FLAFRONT_init_module);
module_exit(ov16885FLAFRONT_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("ov16885FLAFRONT");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
