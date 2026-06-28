/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * Author: 	  wx294303
 * Date:	  2016-11-8
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
#include <securec.h>

#include "hw_flash.h"
#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define GPIO_LOW_STATE       0
#define GPIO_HIGH_STATE      1
#define OV12A10_SUNNY_POS    0
#define OV12A10_LITEON_POS   1
#define OV12A10_FOXCONN_POS  2

int hw_is_fpga_board(void);
//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737

static bool s_ov12a10_power_on = false;
extern struct hw_csi_pad hw_csi_pad;
static hwsensor_vtbl_t s_ov12a10_vtbl;
static char *sensor_dts_name = "OV12A10_VENDOR";

int ov12a10_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hw_ov12a10_power_up_setting[] = {

    //disable sec camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM DVDD1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 AFVDD 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 VCM Enable
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 6,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
struct sensor_power_setting hw_ov12a10_power_down_setting[] = {
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 VCM Enable
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AFVDD 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //SCAM DVDD1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //disable sec camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
};

atomic_t volatile ov12a10_powered = ATOMIC_INIT(0);
struct mutex ov12a10_power_lock;
static sensor_t s_ov12a10 =
{
    .intf = { .vtbl = &s_ov12a10_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hw_ov12a10_power_up_setting),
            .power_setting = hw_ov12a10_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(hw_ov12a10_power_down_setting),
            .power_setting = hw_ov12a10_power_down_setting,
    },
     .p_atpowercnt = &ov12a10_powered,
};

static const struct of_device_id s_ov12a10_dt_match[] =
{
    {
        .compatible = "huawei,ov12a10",
        .data = &s_ov12a10.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_ov12a10_dt_match);

static struct platform_driver s_ov12a10_driver =
{
    .driver =
    {
        .name = "huawei,ov12a10",
        .owner = THIS_MODULE,
        .of_match_table = s_ov12a10_dt_match,
    },
};

char const* ov12a10_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

int ov12a10_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    ret = hw_sensor_power_up_config(s_ov12a10.dev, sensor->board_info);
    if (0 == ret ){
        cam_info("%s. power up config success.", __func__);
    }else{
        cam_err("%s. power up config fail.", __func__);
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

int ov12a10_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
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

int ov12a10_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int ov12a10_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int ov12a10_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;
    uint32_t module_id_0 = 0;
    uint32_t module_id_1 = 0;
    struct pinctrl_state *pinctrl_def  = NULL;
    struct pinctrl_state *pinctrl_idle = NULL;
    struct pinctrl *p = NULL;
    int rc = 0;
    char *sensor_name [] = { "OV12A10_SUNNY", "OV12A10_LITEON", "OV12A10_FOXCONN" };
    if (!si || !data){
        cam_err("%s : si or data is null", __func__);
        return -1;
    }

    sensor = I2S(si);
    cdata  = (struct sensor_cfg_data *)data;
    loge_if_ret(memset_s(cdata->cfg.name, DEVICE_NAME_SIZE, 0, DEVICE_NAME_SIZE));
	
    if (!strncmp(sensor->board_info->name, sensor_dts_name, strlen(sensor_dts_name)))
    {
        p = devm_pinctrl_get(s_ov12a10.dev);
        if (IS_ERR_OR_NULL(p)) {
            cam_err("could not get pinctrl.\n");
            rc = -1;
            goto matchID_exit;
        }

        rc = gpio_request(sensor->board_info->gpios[FSIN].gpio, NULL);
        if(rc < 0) {
            cam_err("%s failed to request gpio[%d]", __func__, sensor->board_info->gpios[FSIN].gpio);
            rc = -1;
            goto matchID_exit;
        }

        cam_info("%s gpio[%d].", __func__, sensor->board_info->gpios[FSIN].gpio);

        pinctrl_def = pinctrl_lookup_state(p, "default");
        if (IS_ERR_OR_NULL(pinctrl_def)) {
            cam_err("could not get defstate.\n");
            rc = -1;
            goto pinctrl_error;
        }

        pinctrl_idle = pinctrl_lookup_state(p, "idle");
        if (IS_ERR_OR_NULL(pinctrl_idle)) {
            pr_err("could not get idle defstate.\n");
            rc = -1;
            goto pinctrl_error;
        }

        /*PULL UP*/
        rc = pinctrl_select_state(p, pinctrl_def);
        if (rc) {
            cam_err("could not set pins to default state.\n");
            rc = -1;
            goto pinctrl_error;
        }

        udelay(10);
        cam_info("%s gpio[%d].", __func__, sensor->board_info->gpios[FSIN].gpio);
        rc = gpio_direction_input(sensor->board_info->gpios[FSIN].gpio);
        if (rc < 0) {
            cam_err("%s failed to config gpio(%d) input.\n",__func__, sensor->board_info->gpios[FSIN].gpio);
            rc = -1;
            goto pinctrl_error;
        }

        module_id_1 = gpio_get_value(sensor->board_info->gpios[FSIN].gpio);

        /*PULL DOWN*/
        rc = pinctrl_select_state(p, pinctrl_idle);
        if (rc) {
            cam_err("could not set pins to idle state.\n");
            rc = -1;
            goto pinctrl_error;
        }

        udelay(10);
        cam_info("%s gpio[%d].", __func__, sensor->board_info->gpios[FSIN].gpio);

        rc = gpio_direction_input(sensor->board_info->gpios[FSIN].gpio);
        if (rc < 0) {
            cam_err("%s failed to config gpio(%d) input.\n",__func__, sensor->board_info->gpios[FSIN].gpio);
            rc = -1;
            goto pinctrl_error;
        }

        module_id_0 = gpio_get_value(sensor->board_info->gpios[FSIN].gpio);

        cam_info("%s module_id_0 %d module_id_1 %d .\n",__func__, module_id_0, module_id_1);
        if((module_id_0 == GPIO_LOW_STATE) && (module_id_1 == GPIO_LOW_STATE)){
            //sunny module
            strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE, sensor_name[OV12A10_SUNNY_POS], 
                strlen(sensor_name[OV12A10_SUNNY_POS]));
            cdata->data = sensor->board_info->sensor_index;
            rc = 0;
        }
        else if((module_id_0 == GPIO_HIGH_STATE) && (module_id_1 == GPIO_HIGH_STATE)){
            //liteon module
            strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE, sensor_name[OV12A10_LITEON_POS], 
                strlen(sensor_name[OV12A10_LITEON_POS]));
            cdata->data = sensor->board_info->sensor_index;
            rc = 0;
        }
        else if((module_id_0 == GPIO_LOW_STATE) && (module_id_1 == GPIO_HIGH_STATE)){
            //foxconn module
            strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE, sensor_name[OV12A10_FOXCONN_POS], 
                strlen(sensor_name[OV12A10_FOXCONN_POS]));
            cdata->data = sensor->board_info->sensor_index;
            rc = 0;
        }
        else{
            strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE, sensor->board_info->name,
                strlen(sensor->board_info->name));
            cdata->data = sensor->board_info->sensor_index;
            cam_err("%s failed to get the module id value.\n",__func__);
            rc = 0;
        }

        gpio_free(sensor->board_info->gpios[FSIN].gpio);
        goto matchID_exit;
    } else {
        strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE, sensor->board_info->name, 
	     strlen(sensor->board_info->name));
        cdata->data = sensor->board_info->sensor_index;
        rc = 0;
        goto matchID_exit;
    }

pinctrl_error:
    gpio_free(sensor->board_info->gpios[FSIN].gpio);
matchID_exit:
    if (cdata->data != SENSOR_INDEX_INVALID) {
        cam_info("%s, cdata->cfg.name = %s", __func__,cdata->cfg.name );
    }

    return rc;
}

static hwsensor_vtbl_t s_ov12a10_vtbl =
{
    .get_name = ov12a10_get_name,
    .config = ov12a10_config,
    .power_up = ov12a10_power_up,
    .power_down = ov12a10_power_down,
    .match_id = ov12a10_match_id,
    .csi_enable = ov12a10_csi_enable,
    .csi_disable = ov12a10_csi_disable,
    /*.sensor_register_attribute = ov12a10_register_attribute,*/
};

int ov12a10_config(hwsensor_intf_t* si,void  *argp)
{
    struct sensor_cfg_data *data;
    int ret =0;

    if (NULL == si || NULL == argp){
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("ov12a10 cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
			mutex_lock(&ov12a10_power_lock);
			if (!s_ov12a10_power_on)
			{
				ret = si->vtbl->power_up(si);
				if (0 == ret)
				{
					s_ov12a10_power_on = true;
				}
			}
			/*lint -e455 -esym(455,*)*/
			mutex_unlock(&ov12a10_power_lock);
			/*lint -e455 +esym(455,*)*/
			break;
        case SEN_CONFIG_POWER_OFF:
			mutex_lock(&ov12a10_power_lock);
			if (s_ov12a10_power_on)
			{
				ret = si->vtbl->power_down(si);
				if (0 != ret)
				{
					cam_err("%s. power_down fail.", __func__);
				}
				s_ov12a10_power_on = false;
			}
			/*lint -e455 -esym(455,*)*/
			mutex_unlock(&ov12a10_power_lock);
			/*lint -e455 +esym(455,*)*/
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
            break;
        case SEN_CONFIG_DISABLE_CSI:
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

static int32_t ov12a10_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    cam_notice("enter %s",__func__);
    if (pdev->dev.of_node) {
        rc = hw_sensor_get_dt_data(pdev, &s_ov12a10);
        if (rc < 0) {
            cam_err("%s failed line %d\n", __func__, __LINE__);
            goto ov12a10_sensor_probe_fail;
        }
    } else {
        cam_err("%s ov12a10 of_node is NULL.\n", __func__);
        goto ov12a10_sensor_probe_fail;
    }

    s_ov12a10.dev = &pdev->dev;
    mutex_init(&ov12a10_power_lock);
    rc = hwsensor_register(pdev, &s_ov12a10.intf);
    rc = rpmsg_sensor_register(pdev, (void*)&s_ov12a10);

ov12a10_sensor_probe_fail:
    return rc;
}

static int __init ov12a10_init_module(void)
{
    cam_notice("enter %s",__func__);
    return platform_driver_probe(&s_ov12a10_driver,
            ov12a10_platform_probe);
}

static void __exit ov12a10_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_ov12a10);
    hwsensor_unregister(Sensor2Pdev(s_ov12a10));
    platform_driver_unregister(&s_ov12a10_driver);
}
//lint -restore
/*lint -e528 -esym(528,*)*/
module_init(ov12a10_init_module);
module_exit(ov12a10_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("ov12a10");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
