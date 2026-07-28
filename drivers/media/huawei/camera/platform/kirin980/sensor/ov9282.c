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
//lint -save -e529 -e542 -e429 -e501 -e527 -e559 -e568 -e570 -e563 -e593 -e613 -e668 -e679
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
#include "m25p80_norflash.h"

#define I2S(i) container_of(i, sensor_t, intf)

struct mutex ov9282_power_lock;

static char const* ov9282_get_name(hwsensor_intf_t* si);
static int ov9282_config(hwsensor_intf_t* si, void  *argp);
static int ov9282_power_up(hwsensor_intf_t* si);
static int ov9282_power_down(hwsensor_intf_t* si);
static int ov9282_match_id(hwsensor_intf_t* si, void * data);
static int ov9282_csi_enable(hwsensor_intf_t* si);
static int ov9282_csi_disable(hwsensor_intf_t* si);
static int ov9282_otp_config(hwsensor_intf_t* si, void *argp);
static int ov9282_otp_get(hwsensor_intf_t*si, hwsensor_config_otp_t *otp);
static int ov9282_otp_update(hwsensor_intf_t *si, hwsensor_config_otp_t *otp);
 int ov9282_get_thermal(hwsensor_intf_t *si,void *data);
static ssize_t hw_ov9282thermal_show(struct device *dev,
    struct device_attribute *attr,char *buf)
{
    int rc=0;
    int tmper =0;
    rc = hw_sensor_get_thermal("ir", &tmper);
    if( -1 == rc){
        cam_err("%s ov9282 get thermal fail", __func__);
        return -1;}

    rc = scnprintf(buf, PAGE_SIZE, "%d", tmper);

    return rc;
}

extern int register_camerafs_attr(struct device_attribute *attr);
extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
static struct device_attribute dev_attr_ov9282thermal =
    __ATTR(irthermal, 0440, hw_ov9282thermal_show, NULL);

static struct sensor_power_setting ov9282_power_setting[] = {

    {
        .seq_type = SENSOR_RXDPHY_CLK,
        .delay = 1,
    },

    //IR Camera AVDD 2.8V
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_2,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //IR CAM IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //IR CAM DVDD 1.1V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,//using ISP CLK 0
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
s_ov9282_vtbl =
{
    .get_name = ov9282_get_name,
    .config = ov9282_config,
    .power_up = ov9282_power_up,
    .power_down = ov9282_power_down,
    .match_id = ov9282_match_id,
    .csi_enable = ov9282_csi_enable,
    .csi_disable = ov9282_csi_disable,
    .otp_config = ov9282_otp_config,
    .otp_get = ov9282_otp_get,
    .otp_update = ov9282_otp_update,
    .get_thermal = ov9282_get_thermal,
};

static sensor_t s_ov9282 =
{
    .intf = { .vtbl = &s_ov9282_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(ov9282_power_setting),
            .power_setting = ov9282_power_setting,
     },
};

static const struct of_device_id
s_ov9282_dt_match[] =
{
    {
        .compatible = "huawei,ov9282",
        .data = &s_ov9282.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_ov9282_dt_match);

static struct platform_driver
s_ov9282_driver =
{
    .driver =
    {
        .name = "huawei,ov9282",
        .owner = THIS_MODULE,
        .of_match_table = s_ov9282_dt_match,
    },
};

 int ov9282_get_thermal(hwsensor_intf_t *si,void *data)
{
    int rc = 0;
    sensor_thermal_data *irdata = NULL;
    if (NULL == si || NULL ==data ) {
        cam_err("%s dev is NULL.", __func__);
        return -1;
    }

    irdata = (sensor_thermal_data *)data;
    rc = hw_sensor_get_thermal("ir",&(irdata->data));
    if( -1 == rc){
        cam_err("%s ov9282 get thermal fail", __func__);
        return -1;
    }

    return 0;
}
 

static char const*
ov9282_get_name(
        hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

static int
ov9282_power_up(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    if (0 != m25p_set_spi_cs_value(1)) {
        cam_err("%s. set spi cs high failed.", __func__);
    }
    if (hw_is_fpga_board()){
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
ov9282_power_down(
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
    if (0 != m25p_set_spi_cs_value(0)) {
        cam_err("%s. set spi cs low failed.", __func__);
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

static int ov9282_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int ov9282_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int
ov9282_match_id(
        hwsensor_intf_t* si, void * data)
{

    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;
    if(NULL == si || NULL == data)
    {
        cam_err("%s. si or data is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    cdata = (struct sensor_cfg_data *)data;

    cam_info("%s name:%s", __func__, sensor->board_info->name);

    strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE-1, sensor->board_info->name, strlen(sensor->board_info->name)+1);
    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static int ov9282_otp_update(
    hwsensor_intf_t *si,
    hwsensor_config_otp_t *otp)
{
    int ret = 0;
    if (NULL == si || NULL == otp)
    {
        cam_err("%s : si or otp is null", __func__);
        return -1;
    }

    cam_info("%s enter, type(%d)", __func__, otp->type);

    switch(otp->type)
    {
    case IRSENSOR_DEFAULT_OTP:
        break;
    case IRSENSOR_HOLDER_OTP:
        ret = m25p_set_array_part_content(IRSENSOR_HOLDER_OTP,
            otp->otp_buffer, otp->otp_buffer_size);
        break;
    case IRSENSOR_DEVICE_OTP:
        ret = m25p_set_array_part_content(IRSENSOR_DEVICE_OTP,
            otp->otp_buffer, otp->otp_buffer_size);
        break;
    case IRSENSOR_ENTIRETY_OTP:
        break;
    default:
        cam_err("%s otp type(%d) is error", __func__, otp->type);
        break;
    }

    return ret;
}

static int ov9282_otp_get(
    hwsensor_intf_t *si,
    hwsensor_config_otp_t *otp)
{
    int ret = 0;
    if (NULL == si || NULL == otp)
    {
        cam_err("%s : si or otp is null", __func__);
        return -1;
    }

    cam_info("%s enter, type(%d)", __func__, otp->type);

    switch(otp->type)
    {
    case IRSENSOR_DEFAULT_OTP:
        break;
    case IRSENSOR_HOLDER_OTP:
        break;
    case IRSENSOR_DEVICE_OTP:
        break;
    case IRSENSOR_ENTIRETY_OTP:
        cam_info("%s otp->otp_buffer = 0x%p, otp->otp_buffer_size = 0x%x", __func__, otp->otp_buffer, otp->otp_buffer_size);
        ret = m25p_get_array_part_content(IRSENSOR_ENTIRETY_OTP,
            otp->otp_buffer, otp->otp_buffer_size);
        //stub
        break;
    default:
        cam_err("%s otp type(%d) is error", __func__, otp->type);
        break;
    }

    return ret;
}

static int
ov9282_otp_config(
    hwsensor_intf_t* si,
    void *argp)
{
    hwsensor_config_otp_t *data;
    int ret = 0;

    if (NULL == si || NULL == argp)
    {
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (hwsensor_config_otp_t *)argp;

    cam_info("ov9282 cfgtype = %d", data->cfg_type);
    switch(data->cfg_type) {
    case SENSOR_CONFIG_OTP_GET:
        ret = si->vtbl->otp_get(si, argp);
        break;
    case SENSOR_CONFIG_OTP_UPDATE:
        ret = si->vtbl->otp_update(si, argp);
        break;
    default:
        cam_err("%s cfg-type(%d) is error", __func__, data->cfg_type);
        break;
    }

    return ret;
}

static int
ov9282_config(
        hwsensor_intf_t* si,
        void  *argp)
{
    struct sensor_cfg_data *data;

    int ret =0;
    static bool ov9282_power_on = false;

    if (NULL == si || NULL == argp){
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("ov9282 cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&ov9282_power_lock);
            if (!ov9282_power_on) {
                ret = si->vtbl->power_up(si);
                ov9282_power_on = true;
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov9282_power_lock);
            /*lint -e455 -esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&ov9282_power_lock);
            if (ov9282_power_on) {
                ret = si->vtbl->power_down(si);
                ov9282_power_on = false;
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov9282_power_lock);
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
            //ret = si->vtbl->csi_enable(si);
            break;
        case SEN_CONFIG_DISABLE_CSI:
            //ret = si->vtbl->csi_disable(si);
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
ov9282_platform_probe(
        struct platform_device* pdev)
{
    int rc = 0;
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *id;
    hwsensor_intf_t *intf;
    sensor_t *sensor;

    cam_notice("enter %s",__func__);
    if (!np)
    {
        cam_err("%s ov9282 of_node is NULL.\n", __func__);
        goto ov9282_sensor_probe_fail;
    }

    id = of_match_node(s_ov9282_dt_match, np);
    if (!id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s failed line %d\n", __func__, __LINE__);
        goto ov9282_sensor_probe_fail;
    }

    sensor->dev = &pdev->dev;
    mutex_init(&ov9282_power_lock);

    rc = hwsensor_register(pdev, intf);
    if (rc < 0) {
        cam_err("%s failed line %d\n", __func__, __LINE__);
        goto ov9282_sensor_probe_fail;
    }

    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc < 0) {
        cam_err("%s failed line %d\n", __func__, __LINE__);
        goto ov9282_sensor_probe_fail;
    }

    register_camerafs_attr(&dev_attr_ov9282thermal);

ov9282_sensor_probe_fail:
    return rc;
}

static int __init
ov9282_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_ov9282_driver,
            ov9282_platform_probe);
}

static void __exit
ov9282_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_ov9282);
    hwsensor_unregister(&s_ov9282.intf);
    platform_driver_unregister(&s_ov9282_driver);
}

module_init(ov9282_init_module);
module_exit(ov9282_exit_module);
MODULE_DESCRIPTION("ov9282");
MODULE_LICENSE("GPL v2");

