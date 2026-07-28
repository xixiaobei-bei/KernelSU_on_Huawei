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
#include "../pmic/hw_pmic.h"
#include "m25p80_norflash.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define OTP_REG_VENDOR      (0x0019)

extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
static char const* ov9286_get_name(hwsensor_intf_t* si);
static int ov9286_config(hwsensor_intf_t* si, void  *argp);
static int ov9286_power_up(hwsensor_intf_t* si);
static int ov9286_power_down(hwsensor_intf_t* si);
static int ov9286_match_id(hwsensor_intf_t* si, void * data);
static int ov9286_csi_enable(hwsensor_intf_t* si);
static int ov9286_csi_disable(hwsensor_intf_t* si);
static int ov9286_otp_config(hwsensor_intf_t* si, void *argp);
static int ov9286_otp_get(hwsensor_intf_t*si, hwsensor_config_otp_t *otp);
static int ov9286_otp_update(hwsensor_intf_t *si, hwsensor_config_otp_t *otp);
 int ov9286_get_thermal(hwsensor_intf_t *si,void *data);
static ssize_t hw_ov9286thermal_show(struct device *dev,
struct device_attribute *attr,char *buf)
{
    int rc=0;
    int tmper =0;
    rc = hw_sensor_get_thermal("ir", &tmper);
    if( -1 == rc){
        cam_err("%s ov9286 get thermal fail", __func__);
        return -1;}

    rc = scnprintf(buf, PAGE_SIZE, "%d", tmper);

    return rc;
}

extern int register_camerafs_attr(struct device_attribute *attr);
static struct device_attribute dev_attr_ov9286thermal =
    __ATTR(irthermal, 0440, hw_ov9286thermal_show, NULL);

static struct sensor_power_setting ov9286_power_setting[] = {

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

    //IR CAM DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P24V,
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
s_ov9286_vtbl =
{
    .get_name = ov9286_get_name,
    .config = ov9286_config,
    .power_up = ov9286_power_up,
    .power_down = ov9286_power_down,
    .match_id = ov9286_match_id,
    .csi_enable = ov9286_csi_enable,
    .csi_disable = ov9286_csi_disable,
    .otp_config = ov9286_otp_config,
    .otp_get = ov9286_otp_get,
    .otp_update = ov9286_otp_update,
    .get_thermal = ov9286_get_thermal,
};

static sensor_t s_ov9286 =
{
    .intf = { .vtbl = &s_ov9286_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(ov9286_power_setting),
            .power_setting = ov9286_power_setting,
     },
};

static const struct of_device_id
s_ov9286_dt_match[] =
{
    {
        .compatible = "huawei,ov9286",
        .data = &s_ov9286.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_ov9286_dt_match);

static struct platform_driver
s_ov9286_driver =
{
    .driver =
    {
        .name = "huawei,ov9286",
        .owner = THIS_MODULE,
        .of_match_table = s_ov9286_dt_match,
    },
};

 int ov9286_get_thermal(hwsensor_intf_t *si,void *data)
{
    int rc = 0;
    sensor_thermal_data *irdata = (sensor_thermal_data *)data;

    if (NULL == si || NULL ==data ) {
        cam_err("%s dev is NULL.", __func__);
        return -1;
    }

    rc = hw_sensor_get_thermal("ir",&(irdata->data));
    if( -1 == rc){
        cam_err("%s ov9286 get thermal fail", __func__);
        return -1;
    }

    return 0;
}

static char const*
ov9286_get_name(
        hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

static int
ov9286_power_up(
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
ov9286_power_down(
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

static int ov9286_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int ov9286_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}
extern int fast_read_nbyte(int addr, u8 *rd_buf, int byte_num);
static int
ov9286_match_id(
        hwsensor_intf_t* si, void * data)
{

    sensor_t* sensor = NULL;
    u8  vendor = 0x88;
    char *name = NULL;
    struct sensor_cfg_data *cdata = NULL;
    if(NULL == si || NULL == data)
    {
        cam_err("%s. si or data is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    cdata = (struct sensor_cfg_data *)data;

    cam_info("%s name:%s", __func__, sensor->board_info->name);
    cdata->factory = 0;//set 0,means vendor name is get from kernel

    fast_read_nbyte(OTP_REG_VENDOR,&vendor,1);
    switch (vendor) {
        case 1:
            name = "OV9286_SUNNY";
            break;
        case 2:
            name = "OV9286_FOXCONN";
            break;
        case 3:
            name = "OV9286_LITEON";
            break;
        case 4:
            name = "OV9286_SEMCO";
            break;
        case 5:
            name = "OV9286_BYD";
            break;
        case 6:
            name = "OV9286_OFILM";
            break;
        case 7:
            name = "OV9286_LGIT";
            break;
        case 8:
            name = "OV9286_SHARP";
            break;
        default:
            name = "OV9286";
            cdata->factory = -1;//set -1,means vendor name is get from isp
            break;
    }
    cam_info("%s vendor = %x name = %s",__func__,vendor,name);
    strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE-1, name, strlen(name)+1);
    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static int ov9286_otp_update(
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

static int ov9286_otp_get(
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
        break;
    default:
        cam_err("%s otp type(%d) is error", __func__, otp->type);
        break;
    }

    return ret;
}

static int
ov9286_otp_config(
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

    cam_info("ov9286 cfgtype = %d", data->cfg_type);
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
ov9286_config(
        hwsensor_intf_t* si,
        void  *argp)
{
    struct sensor_cfg_data *data;

    int ret =0;
    static bool ov9286_power_on = false;

    if (NULL == si || NULL == argp){
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("ov9286 cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            if (!ov9286_power_on) {
                ret = si->vtbl->power_up(si);
                ov9286_power_on = true;
            }
            break;
        case SEN_CONFIG_POWER_OFF:
            if (ov9286_power_on) {
                ret = si->vtbl->power_down(si);
                ov9286_power_on = false;
            }
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
ov9286_platform_probe(
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
        cam_err("%s ov9286 of_node is NULL.\n", __func__);
        goto ov9286_sensor_probe_fail;
    }

    id = of_match_node(s_ov9286_dt_match, np);
    if (!id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc != 0) {
        cam_err("%s failed line %d\n", __func__, __LINE__);
        goto ov9286_sensor_probe_fail;
    }

    sensor->dev = &pdev->dev;

    rc = hwsensor_register(pdev, intf);
    if (rc != 0) {
        cam_err("%s failed line %d\n", __func__, __LINE__);
        goto ov9286_sensor_probe_fail;
    }

    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc != 0) {
        cam_err("%s failed line %d\n", __func__, __LINE__);
        goto ov9286_sensor_probe_fail;
    }

    register_camerafs_attr(&dev_attr_ov9286thermal);

ov9286_sensor_probe_fail:
    cam_err("%s failed. rc =%d\n", __func__, rc);
    return rc;
}

static int __init
ov9286_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_ov9286_driver,
            ov9286_platform_probe);
}

static void __exit
ov9286_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_ov9286);
    hwsensor_unregister(&s_ov9286.intf);
    platform_driver_unregister(&s_ov9286_driver);
}

module_init(ov9286_init_module);
module_exit(ov9286_exit_module);
MODULE_DESCRIPTION("ov9286");
MODULE_LICENSE("GPL v2");

