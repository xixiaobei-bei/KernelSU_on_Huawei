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
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define CTL_RESET_HOLD    (0)
#define CTL_RESET_RELEASE (1)
extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
static struct sensor_power_setting imx318hybird_power_setting [] = {
   //M0 OIS DRV 2.85 [ldo22] //neo pvdd ldo22, vout22 1.1v
    {
        .seq_type = SENSOR_OIS_DRV,
        .config_val = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //AFVDD GPIO 004 ENABLE
    // Power up vcm first, then power up AVDD1
    // XBUCK_AFVDD 2V55---| bucker |---> AVDD1 1V8
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M0+M1 IOVDD 1.8V [ld021]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M0 AVDD0 2.8v [ldo33]
    {
        .seq_type = SENSOR_AVDD0,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M1 AVDD 2.8v [ldo13]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M1 AVDD1 gpio 008 VBUCK3(2v55) -> 1V8
    {
        .seq_type = SENSOR_AVDD1_EN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    // M0 DVDD0 [ldo19] 1v2
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P13V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    // M1 DVDD1 [ldo20] 1v1
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = 1,
    },
    //M0 RESET  [GPIO_013]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //M1 RESET  [GPIO_136]
    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static struct sensor_power_setting imx318hybird_power_setting_clt [] = {
    //pvdd gpio144 1V1
    {
        .seq_type = SENSOR_DVDD0_EN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //AFVDD GPIO 004 ENABLE
    // Power up vcm first, then power up AVDD1
    // XBUCK_AFVDD 2V55---| bucker |---> AVDD1 1V8
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M0+M1 IOVDD 1.8V [ld021]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M0 AVDD0 2.8v [ldo33]
    {
        .seq_type = SENSOR_AVDD0,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M1 AVDD 2.8v [ldo13]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M1 AVDD1 gpio 008 VBUCK3(2v55) -> 1V8
    {
        .seq_type = SENSOR_AVDD1_EN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    // M0 DVDD0 [ldo19] 1v2
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P13V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    // M1 DVDD1 [ldo20] 1v1
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = 1,
    },
    //M0 RESET  [GPIO_013]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //M1 RESET  [GPIO_136]
    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static char const*
imx318hybird_get_name(
        hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

static int
imx318hybird_power_up(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
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

static int
imx318hybird_power_down(
        hwsensor_intf_t* si)
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
    return ret;
}

static int imx318hybird_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int imx318hybird_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int
imx318hybird_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = I2S(si);
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;

    cam_info("%s name:%s", __func__, sensor->board_info->name);

    strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE - 1, sensor->board_info->name, DEVICE_NAME_SIZE - 1);
    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

enum camera_metadata_enum_android_hw_dual_primary_mode{
    ANDROID_HW_DUAL_PRIMARY_FIRST  = 0,
    ANDROID_HW_DUAL_PRIMARY_SECOND = 2,
    ANDROID_HW_DUAL_PRIMARY_BOTH   = 3,
};


static int imx318hybird_do_hw_reset(hwsensor_intf_t* si, int ctl, int id)
{
//lint -save -e826 -e778 -e774 -e747
    sensor_t* sensor = I2S(si);
    hwsensor_board_info_t *b_info;
    int ret;

    b_info = sensor->board_info;
    if (NULL == b_info) {
        cam_warn("%s invalid sensor board info", __func__);
        return 0;
    }
    ret  = gpio_request(b_info->gpios[RESETB].gpio, "imx318reset-0");
    ret |= gpio_request(b_info->gpios[RESETB2].gpio, "imx318reset-1");
    if (ret) {
        cam_err("%s requeset reset pin failed", __func__);
        return ret;
    }
    if(CTL_RESET_HOLD == ctl) {
            ret |= gpio_direction_output(b_info->gpios[RESETB].gpio,CTL_RESET_HOLD);
            ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_HOLD);
            cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_HOLD");
            udelay(2000);
    } else if (CTL_RESET_RELEASE == ctl) {
        if(id == ANDROID_HW_DUAL_PRIMARY_FIRST) {
            ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_RELEASE);
            ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_HOLD);
            cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_HOLD");
        }else if (id == ANDROID_HW_DUAL_PRIMARY_BOTH) {
            ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_RELEASE);
            ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_RELEASE);
            cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_RELEASE");
        }else if (id == ANDROID_HW_DUAL_PRIMARY_SECOND) {
            ret |= gpio_direction_output(b_info->gpios[RESETB2].gpio, CTL_RESET_RELEASE);
            ret |= gpio_direction_output(b_info->gpios[RESETB].gpio, CTL_RESET_HOLD);
            cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_RELEASE");
        }
    }
    gpio_free(b_info->gpios[RESETB].gpio);
    gpio_free(b_info->gpios[RESETB2].gpio);
    if (ret) {
        cam_err("%s set reset pin failed", __func__);
    } else {
        cam_info("%s: set reset state=%d, mode=%d", __func__, ctl, id);
    }
//lint -restore
    return ret;
}


static int
imx318hybird_config(
        hwsensor_intf_t* si,
        void  *argp)
{
    struct sensor_cfg_data *data;
    int ret =0;

	if (NULL == si || NULL == argp){
		cam_err("%s : si or argp is null", __func__);
		return -1;
	}

    data = (struct sensor_cfg_data *)argp;
    cam_debug("imx318hybird cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            ret = si->vtbl->power_up(si);
            break;
        case SEN_CONFIG_POWER_OFF:
            ret = si->vtbl->power_down(si);
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
        case SEN_CONFIG_RESET_HOLD:
            ret = imx318hybird_do_hw_reset(si, CTL_RESET_HOLD, data->mode);
            break;
        case SEN_CONFIG_RESET_RELEASE:
            ret = imx318hybird_do_hw_reset(si, CTL_RESET_RELEASE, data->mode);
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }
    return ret;
}

static hwsensor_vtbl_t s_imx318hybird_vtbl =
{
    .get_name = imx318hybird_get_name,
    .config = imx318hybird_config,
    .power_up = imx318hybird_power_up,
    .power_down = imx318hybird_power_down,
    .match_id = imx318hybird_match_id,
    .csi_enable = imx318hybird_csi_enable,
    .csi_disable = imx318hybird_csi_disable,
};
/* individual driver data for each device */

static sensor_t s_imx318hybird =
{
    .intf = { .vtbl = &s_imx318hybird_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx318hybird_power_setting),
        .power_setting = imx318hybird_power_setting,
    },
};

static sensor_t s_imx318hybird_clt =
{
    .intf = { .vtbl = &s_imx318hybird_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx318hybird_power_setting_clt),
        .power_setting = imx318hybird_power_setting_clt,
    },
};

/* support both imx318hybird & imx318legacydual */
static const struct of_device_id
s_imx318hybird_dt_match[] =
{
    {
        .compatible = "huawei,imx318hybird",
        .data = &s_imx318hybird.intf,
    },
    {
        .compatible = "huawei,imx318hybird_clt",
        .data = &s_imx318hybird_clt.intf,
    },
    { } /* terminate list */
};
MODULE_DEVICE_TABLE(of, s_imx318hybird_dt_match);
/* platform driver struct */
static int32_t imx318hybird_platform_probe(struct platform_device* pdev);
static int32_t imx318hybird_platform_remove(struct platform_device* pdev);
static struct platform_driver
s_imx318hybird_driver =
{
    .probe = imx318hybird_platform_probe,
    .remove = imx318hybird_platform_remove,
    .driver =
    {
        .name = "huawei,imx318hybird",
        .owner = THIS_MODULE,
        .of_match_table = s_imx318hybird_dt_match,
    },
};


static int32_t
imx318hybird_platform_probe(
        struct platform_device* pdev)
{
    int rc = 0;
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *id;
    hwsensor_intf_t *intf;
    sensor_t *sensor;

    cam_info("enter %s gal",__func__);

    if (!np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_imx318hybird_dt_match, np);
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

    rc = hwsensor_register(pdev, intf);
    rc = rpmsg_sensor_register(pdev, (void*)sensor);

    return rc;
}

static int32_t
imx318hybird_platform_remove(
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
    id = of_match_node(s_imx318hybird_dt_match, np);
    if (!id) {
        cam_info("%s none id matched", __func__);
        return 0;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);

    rpmsg_sensor_unregister((void*)&sensor);
    hwsensor_unregister(pdev);
    return 0;
}
static int __init
imx318hybird_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_imx318hybird_driver,
            imx318hybird_platform_probe);
}

static void __exit
imx318hybird_exit_module(void)
{
    platform_driver_unregister(&s_imx318hybird_driver);
}

module_init(imx318hybird_init_module);
module_exit(imx318hybird_exit_module);
MODULE_DESCRIPTION("imx318hybird");
MODULE_LICENSE("GPL v2");
